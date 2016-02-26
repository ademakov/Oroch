// varint.h
//
// Copyright (c) 2015-2016  Aleksey Demakov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef OROCH_VARINT_H_
#define OROCH_VARINT_H_

#include <cstddef>

#include "common.h"
#include "integer_traits.h"
#include "zigzag.h"

namespace oroch {

//
// Variable byte encoding of integers. Every byte of the encoded data
// represents a 7-bit group from the original integer. The 8th bit is used as
// a continuation mark. The encoded data omits those 7-bit groups that include
// only zero bits and are located at the MSB end of the integer.
//
// This encoding is only good for unsigned integers if they are not too big on
// average.
//
// The codec automatically applies zigzag encoding if used on signed types.
//
template <typename T, typename V = zigzag_codec<T>>
class varint_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using value_codec = V;

	// The number of bytes needed to encode an integer with the given
	// number of significant bits.
	static constexpr size_t
	nbits_space(size_t nbits)
	{
		return (nbits + 6) / 7;
	}

	// The maximum number of bytes needed to encode an integer of the
	// template-specified type.
	static constexpr size_t nbytemax = nbits_space(integer_traits<original_t>::nbits);

	// Get the number of bytes needed to encode a given integer value.
	static size_t
	value_space(original_t src, value_codec vcodec = value_codec())
	{
		unsigned_t value = vcodec.value_encode(src);
		if (value == 0)
			return 1;
		return nbits_space(integer_traits<unsigned_t>::usedcount(value));
	}

	static void
	value_encode(dst_bytes_t &dst, original_t src, value_codec vcodec = value_codec())
	{
		unsigned_t value = vcodec.value_encode(src);
		while (value >= 0x80) {
			*dst++ = byte_t(value | 0x80);
			value >>= 7;
		}
		*dst++ = byte_t(value);
	}

	static original_t
	value_decode(src_bytes_t &src, value_codec vcodec = value_codec())
	{
		unsigned_t value = *src++;
		if ((value & 0x80) != 0) {
			value &= 0x7f;

			unsigned_t byte = *src++;
			if ((byte & 0x80) == 0) {
				value |= byte << 7;
			} else  {
				value |= (byte & 0x7f) << 7;

				byte = *src++;
				if ((byte & 0x80) == 0) {
					value |= byte << 14;
				} else {
					value |= (byte & 0x7f) << 14;

					for (unsigned shift = 21; ; shift += 7) {
						byte = *src++;
						if ((byte & 0x80) == 0) {
							value |= byte << shift;
							break;
						}
						value |= (byte & 0x7f) << shift;
					}
				}
			}
		}
		return vcodec.value_decode(value);
	}

	static void
	value_decode(original_t &dst, src_bytes_t &src, value_codec vcodec = value_codec())
	{
		dst = value_decode(src, vcodec);
	}

	// Get the number of bytes needed to encode a given integer sequence.
	template<typename Iter>
	static size_t
	space(Iter src, Iter const end, value_codec vcodec = value_codec())
	{
		size_t count = 0;
		while (src != end)
			count += value_space(*src++, vcodec);
		return count;
	}

	template<typename Iter>
	static void
	encode(dst_bytes_t &dst, Iter src, Iter const end, value_codec vcodec = value_codec())
	{
		while (src != end)
			value_encode(dst, *src++, vcodec);
	}

	template<typename Iter>
	static void
	decode(Iter dst, Iter const end, src_bytes_t &src, value_codec vcodec = value_codec())
	{
		while (dst != end)
			value_decode(*dst++, src, vcodec);
	}
};

} // namespace oroch

#endif /* OROCH_VARINT_H_ */

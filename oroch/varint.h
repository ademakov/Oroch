// varint.h
//
// Copyright (c) 2015  Aleksey Demakov
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
#include <cstdint>

#include <oroch/integer_traits.h>
#include <oroch/zigzag.h>

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
template <typename T>
class varint_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using value_codec = zigzag_codec<original_t>;

	// The maximum number of bytes needed to encode an integer.
	static constexpr size_t nbytemax = (integer_traits<original_t>::nbits + 6) / 7;

	// The number of bytes needed to encode a given integer.
	static size_t
	space(original_t src)
	{
		size_t count = 1;

		unsigned_t value = value_codec().value_encode(src);
		while (value >= 0x80) {
			value >>= 7;
			count++;
		}

		return count;
	}

	template<typename DstIter>
	static bool
	encode(DstIter &dbegin, DstIter const dend, original_t src)
	{
		DstIter dst = dbegin;
		unsigned_t value = value_codec().value_encode(src);

		while (value >= 0x80) {
			if (dst >= dend)
				return false;
			*dst++ = uint8_t(value | 0x80);
			value >>= 7;
		}

		if (dst >= dend)
			return false;
		*dst++ = uint8_t(value);

		dbegin = dst;
		return true;
	}

	template<typename SrcIter>
	static bool
	decode(original_t &dst, SrcIter &sbegin, SrcIter const send)
	{
		SrcIter src = sbegin;

		if (src >= send)
			return false;
		unsigned_t value = uint8_t(*src++);

		if (int8_t(value) < 0) {
			value &= 0x7f;
			int shift = 7;
			for (;;) {
				if (src >= send)
					return false;
				uint8_t byte = *src++;

				if (int8_t(byte) > 0) {
					value |= unsigned_t(byte) << shift;
					break;
				}

				value |= unsigned_t(byte & 0x7f) << shift;
				shift += 7;
			}
		}

		dst = value_codec().value_decode(value);
		sbegin = src;
		return true;
	}

	template<typename DstIter, typename SrcIter>
	static bool
	encode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		for (; src < send; ++src) {
			if (!encode(dst, dend, *src)) {
				rc = false;
				break;
			}
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}

	template<typename DstIter, typename SrcIter>
	static bool
	decode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		for (; dst < dend; ++dst) {
			if (!decode(*dst, src, send)) {
				rc = false;
				break;
			}
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}
};

} // namespace oroch

#endif /* OROCH_VARINT_H_ */

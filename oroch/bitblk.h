// bitblk.h
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

#ifndef OROCH_BITBLK_H_
#define OROCH_BITBLK_H_

#include <iterator>

#include <oroch/integer_traits.h>
#include <oroch/zigzag.h>

namespace oroch {

//
// Bit-packing of a number of integers in a 16-byte block. Each integer is
// encoded with a fixed bit width.
//
template <typename T>
class bitblk_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	static constexpr size_t block_size = 16;
	static constexpr size_t block_nbits = block_size * 8;

	/* The number of integers that fits in a block. */
	static constexpr size_t
	capacity(size_t nbits)
	{
		return block_nbits / nbits;
	}

	/* The number of blocks to fit given number of integers. */
	static constexpr size_t
	block_number(size_t nbits, size_t nvalues)
	{
		return (nvalues + capacity(nbits) - 1) / capacity(nbits);
	}

	/* The number of bytes to fit given number of integers. */
	static constexpr size_t
	block_volume(size_t nbits, size_t nvalues)
	{
		return block_size * block_number(nbits, nvalues);
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	encode(const size_t nbits,
	       DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       ValueCodec value_codec = ValueCodec())
	{
		size_t size = std::distance(dbegin, dend);
		if (size < block_size)
			return false;

		const size_t mask = (1ul << nbits) - 1;
		const size_t c = capacity(nbits);

		DstIter dst = dbegin;
		SrcIter src = sbegin;
		uint64_t buf[2] = { 0, 0 };

		size_t m = c / 2;
		size_t n = c - m;
		size_t shift = 0;
		while (m--) {
			if (src == send)
				goto done;

			uint64_t value = value_codec.value_encode(*src++);
			buf[0] |= (value & mask) << shift;
			shift += nbits;
		}
		if (shift == 64) {
			shift = 0;
		} else {
			if (src == send)
				goto done;

			size_t nbits1 = 64 - shift;
			size_t nbits2 = nbits - nbits1;
			size_t mask1 = (1ul << nbits1) - 1;
			size_t mask2 = (1ul << nbits2) - 1;

			uint64_t value = value_codec.value_encode(*src++);
			buf[0] |= (value & mask1) << shift;
			buf[1] |= (value >> nbits1) & mask2;
			shift = nbits2;
			n--;
		}
		while (n--) {
			if (src == send)
				goto done;

			uint64_t value = value_codec.value_encode(*src++);
			buf[1] |= (value & mask) << shift;
			shift += nbits;
		}

	done:
		// FIXME: do it in a safer way
		uint64_t *block = reinterpret_cast<uint64_t *>(dbegin);
		block[0] = buf[0];
		block[1] = buf[1];

		dbegin += block_size;
		sbegin = src;
		return true;
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	decode(size_t nbits,
	       DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       ValueCodec value_codec = ValueCodec())
	{
		size_t size = std::distance(sbegin, send);
		if (size < block_size)
			return false;

		const size_t mask = (1ul << nbits) - 1;
		const size_t c = capacity(nbits);

		DstIter dst = dbegin;
		SrcIter src = sbegin;
		uint64_t buf[2];

		// FIXME: do it in a safer way
		uint64_t *block = reinterpret_cast<uint64_t *>(sbegin);
		buf[0] = block[0];
		buf[1] = block[1];

		size_t m = c / 2;
		size_t n = c - m;
		size_t shift = 0;
		while (m--) {
			if (dst == dend)
				goto done;

			uint64_t value = (buf[0] >> shift) & mask;
			*dst++ = value_codec.value_decode(value);
			shift += nbits;
		}
		if (shift == 64) {
			shift = 0;
		} else {
			if (dst == dend)
				goto done;

			size_t nbits1 = 64 - shift;
			size_t nbits2 = nbits - nbits1;
			size_t mask1 = (1ul << nbits1) - 1;
			size_t mask2 = (1ul << nbits2) - 1;

			uint64_t value = 0;
			value |= (buf[0] >> shift) & mask1;
			value |= (buf[1] & mask2) << nbits1;
			*dst++ = value_codec.value_decode(value);
			shift = nbits2;
			n--;
		}
		while (n--) {
			if (dst == dend)
				goto done;

			uint64_t value = (buf[1] >> shift) & mask;
			*dst++ = value_codec.value_decode(value);
			shift += nbits;
		}

	done:
		sbegin += block_size;
		dbegin = dst;
		return true;
	}
};

} // namespace oroch

#endif /* OROCH_BITBLK_H_ */

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

#include <cassert>
#include <iterator>

#include <oroch/config.h>
#include <oroch/integer_traits.h>
#include <oroch/zigzag.h>

namespace oroch {

//
// Bit-packing of a number of integers into a 16-byte block. Each integer is
// encoded with a fixed bit width.
//
template <typename T>
class bitblk_codec
{
public:
	using original_t = T;
	static constexpr size_t block_size = 16;
	static constexpr size_t block_nbits = block_size * 8;

	// Get the number of integers with a given width that fits into
	// a single block.
	static constexpr size_t
	capacity(size_t nbits)
	{
		return block_nbits / nbits;
	}

	// Get the number of blocks required to fit a given number of
	// integers.
	static constexpr size_t
	block_number(size_t nvalues, size_t nbits)
	{
		return (nvalues + capacity(nbits) - 1) / capacity(nbits);
	}

	// Get the number of bytes required to fit a given number of
	// integers.
	static constexpr size_t
	space(size_t nvalues, size_t nbits)
	{
		return block_size * block_number(nvalues, nbits);
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	encode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       const size_t nbits,
	       ValueCodec value_codec = ValueCodec())
	{
		size_t size = std::distance(dbegin, dend);
		if (size < block_size)
			return false;

		const size_t c = capacity(nbits);
		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		DstIter dst = dbegin;
		SrcIter src = sbegin;

		uint64_t u = 0;
		uint64_t v = 0;

		size_t m = c / 2;
		size_t n = c - m;
		size_t shift = 0;
		while (m--) {
			if (src == send)
				goto done;

			uint64_t value = value_codec.value_encode(*src++);
			u |= (value & mask) << shift;
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
			u |= (value & mask1) << shift;
			v |= (value >> nbits1) & mask2;
			shift = nbits2;
			n--;
		}
		while (n--) {
			if (src == send)
				goto done;

			uint64_t value = value_codec.value_encode(*src++);
			v |= (value & mask) << shift;
			shift += nbits;
		}

	done:
		auto addr = std::addressof(*dst);
		uint64_t *block = reinterpret_cast<uint64_t *>(addr);
		block[0] = u;
		block[1] = v;

		dbegin += block_size;
		sbegin = src;
		return true;
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	decode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       const size_t nbits,
	       ValueCodec value_codec = ValueCodec())
	{
		size_t size = std::distance(sbegin, send);
		if (size < block_size)
			return false;

		DstIter dst = dbegin;
		SrcIter src = sbegin;

		auto addr = std::addressof(*src);
		const uint64_t *block = reinterpret_cast<const uint64_t *>(addr);
		uint64_t u = block[0];
		uint64_t v = block[1];

		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		size_t c = capacity(nbits);
		size_t m = c / 2;
		size_t mbits = m * nbits;
		size_t k = std::distance(dst, dend);
		if (c > k) {
			c = k;
			if (m > k)
				m = k;
		}
		size_t n = c - m;
		while (m--) {
			*dst++ = value_codec.value_decode(u & mask);
			u >>= nbits;
		}
		if (n && mbits != 64) {
			size_t r = 64 - mbits;
			uint64_t x = u | (v << r);
			*dst++ = value_codec.value_decode(x & mask);
			v >>= nbits - r;
			n--;
		}
		while (n--) {
			*dst++ = value_codec.value_decode(v & mask);
			v >>= nbits;
		}

		sbegin += block_size;
		dbegin = dst;
		return true;
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static void
	decode(DstIter &dbegin, SrcIter &sbegin,
	       const size_t nbits,
	       ValueCodec value_codec = ValueCodec())
	{
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		auto addr = std::addressof(*src);
		const uint64_t *block = reinterpret_cast<const uint64_t *>(addr);
		uint64_t u = block[0];
		uint64_t v = block[1];

		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		size_t c = capacity(nbits);
		size_t m = c / 2;
		size_t n = c - m;
		size_t mbits = m * nbits;
		while (m--) {
			*dst++ = value_codec.value_decode(u & mask);
			u >>= nbits;
		}
		if (mbits != 64) {
			size_t r = 64 - mbits;
			uint64_t x = u | (v << r);
			*dst++ = value_codec.value_decode(x & mask);
			v >>= nbits - r;
			n--;
		}
		while (n--) {
			*dst++ = value_codec.value_decode(v & mask);
			v >>= nbits;
		}

		sbegin += block_size;
		dbegin = dst;
	}

	template<typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static original_t
	fetch(SrcIter src, const size_t index, const size_t nbits,
	      ValueCodec value_codec = ValueCodec())
	{
		auto addr = std::addressof(*src);
		const uint64_t *block = reinterpret_cast<const uint64_t *>(addr);

		size_t m = capacity(nbits) / 2;

		uint64_t x;
		if (index < m) {
			x = block[0] >> (index * nbits);
		} else {
			size_t mbits = m * nbits;
			if (mbits != 64 && index == m)
				x = (block[0] >> mbits) | (block[1] << (64 - mbits));
			else
				x = block[1] >> (index * nbits - 64);
		}

		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);
		return value_codec.value_decode(x & mask);
	}
};

} // namespace oroch

#endif /* OROCH_BITBLK_H_ */

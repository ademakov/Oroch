// bitpck.h
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

#ifndef OROCH_BITPCK_H_
#define OROCH_BITPCK_H_

#include <iterator>

#include "common.h"
#include "integer_traits.h"
#include "zigzag.h"

namespace oroch {

//
// Bit-packing of a number of integers into a 16-byte block. Each integer is
// encoded with a fixed bit width.
//
// By default the codec applies zigzag encoding if used on signed types.
// This can be replaced by supplying a different value code explicitly.
//
template<typename T, typename V = zigzag_codec<T>>
class bitpck_codec
{
public:
	using original_t = T;
	using value_codec = V;

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

	template<typename Iter>
	static void
	block_encode(dst_bytes_t &dst, Iter &src, Iter const end, const size_t nbits, value_codec &vcodec)
	{
		const size_t c = capacity(nbits);
		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		uint64_t u = 0;
		uint64_t v = 0;

		size_t m = c / 2;
		size_t n = c - m;
		size_t shift = 0;
		while (m--) {
			if (src == end)
				goto done;

			uint64_t value = vcodec.value_encode(*src++);
			u |= (value & mask) << shift;
			shift += nbits;
		}
		if (shift == 64) {
			shift = 0;
		} else {
			if (src == end)
				goto done;

			size_t nbits1 = 64 - shift;
			size_t nbits2 = nbits - nbits1;
			size_t mask1 = (1ul << nbits1) - 1;
			size_t mask2 = (1ul << nbits2) - 1;

			uint64_t value = vcodec.value_encode(*src++);
			u |= (value & mask1) << shift;
			v |= (value >> nbits1) & mask2;
			shift = nbits2;
			n--;
		}
		while (n--) {
			if (src == end)
				goto done;

			uint64_t value = vcodec.value_encode(*src++);
			v |= (value & mask) << shift;
			shift += nbits;
		}

	done:
		uint64_t *block = reinterpret_cast<uint64_t *>(dst);
		block[0] = u;
		block[1] = v;
		dst += block_size;
	}

	template<typename Iter>
	static void
	block_decode(Iter dst, Iter const end, src_bytes_t &src, const size_t nbits, value_codec &vcodec)
	{
		const uint64_t *block = reinterpret_cast<const uint64_t *>(src);
		uint64_t u = block[0];
		uint64_t v = block[1];
		src += block_size;

		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		size_t c = capacity(nbits);
		size_t m = c / 2;
		size_t mbits = m * nbits;
		size_t k = std::distance(dst, end);
		if (c > k) {
			c = k;
			if (m > k)
				m = k;
		}
		size_t n = c - m;
		while (m--) {
			*dst++ = vcodec.value_decode(u & mask);
			u >>= nbits;
		}
		if (n && mbits != 64) {
			size_t r = 64 - mbits;
			uint64_t x = u | (v << r);
			*dst++ = vcodec.value_decode(x & mask);
			v >>= nbits - r;
			n--;
		}
		while (n--) {
			*dst++ = vcodec.value_decode(v & mask);
			v >>= nbits;
		}
	}

	template<typename Iter>
	static void
	block_decode(Iter dst, src_bytes_t &src, const size_t nbits, value_codec &vcodec)
	{
		const uint64_t *block = reinterpret_cast<const uint64_t *>(src);
		uint64_t u = block[0];
		uint64_t v = block[1];
		src += block_size;

		const uint64_t mask = uint64_t(int64_t(-1)) >> (64 - nbits);

		size_t c = capacity(nbits);
		size_t m = c / 2;
		size_t n = c - m;
		size_t mbits = m * nbits;
		while (m--) {
			*dst++ = vcodec.value_decode(u & mask);
			u >>= nbits;
		}
		if (mbits != 64) {
			size_t r = 64 - mbits;
			uint64_t x = u | (v << r);
			*dst++ = vcodec.value_decode(x & mask);
			v >>= nbits - r;
			n--;
		}
		while (n--) {
			*dst++ = vcodec.value_decode(v & mask);
			v >>= nbits;
		}
	}

	static original_t
	block_fetch(src_bytes_t src, const size_t index, const size_t nbits, value_codec &vcodec)
	{
		const uint64_t *block = reinterpret_cast<const uint64_t *>(src);

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
		return vcodec.value_decode(x & mask);
	}

	template<typename Iter>
	static void
	encode(dst_bytes_t &dst, Iter src, Iter end, size_t nbits, value_codec vcodec = value_codec())
	{
		while (src < end)
			block_encode(dst, src, end, nbits, vcodec);
	}

	template<typename Iter>
	static void
	decode(Iter dst, Iter end, src_bytes_t &src, size_t nbits, value_codec vcodec = value_codec())
	{
		size_t c = capacity(nbits);
		for (;;) {
			Iter block_end = dst + c;
			if (block_end > end) {
				if (dst != end)
					block_decode(dst, end, src, nbits, vcodec);
				break;
			}
			block_decode(dst, src, nbits, vcodec);
			dst = block_end;
		}
	}

	static original_t
	fetch(src_bytes_t src, const size_t index, const size_t nbits, value_codec vcodec = value_codec())
	{
		size_t c = capacity(nbits);
		src += (index / c) * block_size;
		return block_fetch(src, index % c, nbits, vcodec);
	}
};

} // namespace oroch

#endif /* OROCH_BITPCK_H_ */

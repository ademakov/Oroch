// bitpck.h
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

#ifndef OROCH_BITPCK_H_
#define OROCH_BITPCK_H_

#include <oroch/bitblk.h>

namespace oroch {

//
// Bit-packing of integers into a sequence of 16-byte blocks.
//
// By default the codec applies zigzag encoding if used on signed types.
// This can be replaced by supplying a different value code explicitly.
//
template<typename T>
class bitpck_codec
{
public:
	using original_t = T;
	using block_codec = bitblk_codec<original_t>;

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	encode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send,
	       size_t nbits, ValueCodec value_codec = ValueCodec())
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (!block_codec::encode(dst, dend, src, send,
						 nbits, value_codec)) {
				rc = false;
				break;
			}
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}

	template<typename DstIter, typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static bool
	decode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send,
	       size_t nbits, ValueCodec value_codec = ValueCodec())
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		size_t ndst = std::distance(dst, dend);
		size_t nsrc = std::distance(src, send);
		size_t ndst_full = ndst / block_codec::capacity(nbits);
		size_t nsrc_full = nsrc / block_codec::block_size;
		size_t n = std::min(ndst_full, nsrc_full);
		while (n--)
			block_codec::decode(dst, src, nbits, value_codec);

		if (dst != dend && src != send) {
			if (!block_codec::decode(dst, dend, src, send,
						 nbits, value_codec))
				rc = false;
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}

	template<typename SrcIter,
		 typename ValueCodec = zigzag_codec<original_t>>
	static original_t
	fetch(SrcIter src, const size_t index, const size_t nbits,
	      ValueCodec value_codec = ValueCodec())
	{
		size_t c = block_codec::capacity(nbits);
		src += (index / c) * block_codec::block_size;
		return block_codec::fetch(src, index % c, nbits,
					  value_codec);
	}
};

} // namespace oroch

#endif /* OROCH_BITPCK_H_ */

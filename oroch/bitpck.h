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

template<typename T>
class bitpck_codec
{
public:
	using original_t = T;
	using value_codec = zigzag_codec<original_t>;
	using block_codec = bitblk_codec<original_t>;

	template<typename DstIter, typename SrcIter>
	static bool
	encode(size_t nbits,
	       DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (!block_codec::encode(nbits, dst, dend, src, send,
						 value_codec())) {
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
	decode(size_t nbits,
	       DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (!block_codec::decode(nbits, dst, dend, src, send,
						 value_codec())) {
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

#endif /* OROCH_BITPCK_H_ */

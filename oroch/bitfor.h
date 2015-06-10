// bitfor.h
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

#ifndef OROCH_BITFOR_H_
#define OROCH_BITFOR_H_

#include <cassert>

#include <oroch/bitblk.h>

namespace oroch {

template<typename T>
class bitfor_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using block_codec = bitblk_codec<original_t>;

	struct parameters
	{
		original_t frame_of_reference;
		size_t nbits;
	};

	class value_codec
	{
	public:
		value_codec(original_t v) : frame_of_reference_(v) {}

		unsigned_t
		value_encode(original_t v)
		{
			return unsigned_t(v - frame_of_reference_);
		}

		original_t
		value_decode(unsigned_t v)
		{
			return original_t(v + frame_of_reference_);
		}

	private:
		original_t frame_of_reference_;
	};

	template<typename DstIter, typename SrcIter>
	static bool
	encode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       const parameters &params)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (!block_codec::encode(dst, dend, src, send, params.nbits,
						 value_codec(params.frame_of_reference))) {
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
	       SrcIter &sbegin, SrcIter const send,
	       const parameters &params)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (!block_codec::decode(dst, dend, src, send, params.nbits,
						 value_codec(params.frame_of_reference))) {
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

#endif /* OROCH_BITFOR_H_ */

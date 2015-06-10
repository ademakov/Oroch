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

#include <oroch/bitpck.h>

namespace oroch {

template<typename T>
class bitfor_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using basic_codec = bitpck_codec<original_t>;
	using block_codec = typename basic_codec::block_codec;

	struct parameters
	{
		parameters(original_t f, size_t n)
		: base(f), nbits(n)
		{
		}

		unsigned_t
		value_encode(original_t v)
		{
			return unsigned_t(v - base);
		}

		original_t
		value_decode(unsigned_t v)
		{
			return original_t(v + base);
		}

		const original_t base;
		const size_t nbits;
	};

	template<typename DstIter, typename SrcIter>
	static bool
	encode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send,
	       const parameters &params)
	{
		return basic_codec::encode(dbegin, dend, sbegin, send,
					   params.nbits, params);
	}

	template<typename DstIter, typename SrcIter>
	static bool
	decode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send,
	       const parameters &params)
	{
		return basic_codec::decode(dbegin, dend, sbegin, send,
					   params.nbits, params);
	}
};

} // namespace oroch

#endif /* OROCH_BITFOR_H_ */

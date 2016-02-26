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

#include "common.h"
#include "bitpck.h"
#include "origin.h"

namespace oroch {

//
// Bit-packing of integers into a sequence of 16-byte blocks with frame of
// reference. Instead of the original values the codec uses their difference
// relative to the provided base value.
//
// The base value must be lower than or equal to the minimum encoded integer.
// This ensures that only unsigned integers get to be bit-packed.
//
template<typename T>
class bitfor_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	struct parameters : public origin_codec<original_t>
	{
		parameters(original_t f, size_t n)
		: origin_codec<original_t>(f), nbits(n)
		{
		}

		const size_t nbits;
	};

	using basic_codec = bitpck_codec<original_t, parameters>;

	template<typename Iter>
	static void
	encode(dst_bytes_t &dst, Iter src, Iter end, const parameters &params)
	{
		basic_codec::encode(dst, src, end, params.nbits, params);
	}

	template<typename Iter>
	static void
	decode(Iter dst, Iter end, src_bytes_t &src, const parameters &params)
	{
		basic_codec::decode(dst, end, src, params.nbits, params);
	}

	static original_t
	fetch(src_bytes_t src, const size_t index, const parameters &params)
	{
		return basic_codec::fetch(src, index, params.nbits, params);
	}
};

} // namespace oroch

#endif /* OROCH_BITFOR_H_ */

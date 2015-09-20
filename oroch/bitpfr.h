// bitpfr.h
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

#ifndef OROCH_BITPFR_H_
#define OROCH_BITPFR_H_

#include <vector>

#include <oroch/bitpck.h>
#include <oroch/origin.h>

namespace oroch {

template<typename T>
class bitpfr_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	struct exceptions
	{
		std::vector<size_t> indices;
		std::vector<unsigned_t> values;

		size_t index = 0;

		void reset() {
			indices.clear();
			values.clear();

			index = 0;
		}
	};

	struct parameters : public origin_codec<original_t>
	{
	public:
		using basic_value_codec = origin_codec<original_t>;

		parameters(original_t f, size_t n, exceptions &x)
		: basic_value_codec(f), nbits(n), mask((1ul << n) - 1), excpts(x)
		{
		}

		unsigned_t
		basic_value_encode(original_t v) const
		{
			return basic_value_codec::value_encode(v);
		}

		unsigned_t
		value_encode(original_t v)
		{
			unsigned_t u = basic_value_encode(v);
			if ((u & ~mask) != 0) {
				excpts.indices.push_back(excpts.index);
				excpts.values.push_back(u >> nbits);
			}
			excpts.index++;
			return u;
		}

		const size_t nbits;
		const unsigned_t mask;
		exceptions &excpts;
	};

	using basic_codec = bitpck_codec<original_t, parameters>;

	template<typename DstIter, typename SrcIter>
	static void
	encode(DstIter &dst, SrcIter &src, SrcIter send, parameters &params)
	{
		basic_codec::encode(dst, src, send, params.nbits, params);
	}

	template<typename DstIter, typename SrcIter>
	static void
	decode(DstIter &dst, DstIter dend, SrcIter &src, const parameters &params)
	{
		DstIter arr = dst;
		basic_codec::decode(dst, dend, src, params.nbits, params);
		for (size_t i = 0; i < params.excpts.indices.size(); i++) {
			size_t idx = params.excpts.indices[i];
			unsigned_t value = params.basic_value_encode(arr[idx]);
			value |= params.excpts.values[i] << params.nbits;
			arr[idx] = params.value_decode(value);
		}
	}
};

} // namespace oroch

#endif /* OROCH_BITPFR_H_ */

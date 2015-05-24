// zigzag_codec.h
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

#ifndef OROCH_ZIGZAG_CODEC_H_
#define OROCH_ZIGZAG_CODEC_H_

#include "integer_traits.h"

#include <cstdint>

namespace oroch {

/*
 * ZigZag encoding of signed integers as described here:
 *
 * https://developers.google.com/protocol-buffers/docs/encoding
 */
template <typename T>
struct zigzag_codec
{
	using signed_t = typename integer_traits<T>::signed_t;
	using unsigned_t = typename integer_traits<T>::unsigned_t;

	static constexpr int sign_shift = integer_traits<T>::nbits - 1;

	static unsigned_t
	encode(signed_t s)
	{
		return (s << 1) ^ (s >> sign_shift);
	}

	static signed_t
	decode(unsigned_t u)
	{
		return (u >> 1) ^ -((signed_t) (u & 1));
	}
};

using zigzag32_codec = zigzag_codec<std::int32_t>;
using zigzag64_codec = zigzag_codec<std::int64_t>;

} // namespace oroch

#endif /* OROCH_ZIGZAG_CODEC_H_ */

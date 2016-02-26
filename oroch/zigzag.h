// zigzag.h
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

#ifndef OROCH_ZIGZAG_H_
#define OROCH_ZIGZAG_H_

#include <type_traits>

#include "integer_traits.h"

namespace oroch {

//
// ZigZag encoding of signed integers as described here:
//
// https://developers.google.com/protocol-buffers/docs/encoding
//
template<typename T>
struct zigzag_codec
{
	using original_t = T;
	using signed_t = typename integer_traits<original_t>::signed_t;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	static constexpr int sign_shift = integer_traits<signed_t>::nbits - 1;

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

	template<typename S = original_t,
		 typename std::enable_if<std::is_signed<S>::value>::type* = nullptr>
	static unsigned_t
	encode_if_signed(original_t v)
	{
		return encode(v);
	}

	template<typename S = original_t,
		 typename std::enable_if<std::is_unsigned<S>::value>::type* = nullptr>
	static unsigned_t
	encode_if_signed(original_t v)
	{
		return v;
	}

	template<typename S = original_t,
		 typename std::enable_if<std::is_signed<S>::value>::type* = nullptr>
	static original_t
	decode_if_signed(unsigned_t v)
	{
		return decode(v);
	}

	template<typename S = original_t,
		 typename std::enable_if<std::is_unsigned<S>::value>::type* = nullptr>
	static original_t
	decode_if_signed(unsigned_t v)
	{
		return v;
	}

	unsigned_t
	value_encode(original_t v) const
	{
		return encode_if_signed(v);
	}

	original_t
	value_decode(unsigned_t v) const
	{
		return decode_if_signed(v);
	}
};

} // namespace oroch

#endif /* OROCH_ZIGZAG_H_ */

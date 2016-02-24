// offset.h
//
// Copyright (c) 2016  Aleksey Demakov
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

#ifndef OROCH_OFFSET_H_
#define OROCH_OFFSET_H_

#include <oroch/integer_traits.h>

namespace oroch {

//
// Encoding of a value as a difference against the previous value with
// an additional constant offset. This might be used e.g. to encode a
// sequence of positions that naturally differ by 1 at least. If the
// offset value is zero then this codec is reduced to a delta codec.
//
template<typename T, T offset>
class offset_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	// If the origin is the first element of the sequence and it is going
	// to be encoded separately then apply the offset to it. If the origin
	// is known from the general sequence properties and the first element
	// is going to be encoded along with other elements, then it could be
	// equal to the origin, so do not apply affet to it.
	offset_codec(original_t origin, bool is_taken_out)
	: origin_{origin}
	{
		if (is_taken_out)
			origin_ += offset;
	}

	unsigned_t
	value_encode(original_t v)
	{
		unsigned_t u = unsigned_t(v - origin_);
		origin_ = v + offset;
		return u;
	}

	original_t
	value_decode(unsigned_t u)
	{
		original_t v = original_t(origin_ + u);
		origin_ = v + offset;
		return v;
	}

private:
	original_t origin_;
};

} // namespace oroch

#endif /* OROCH_OFFSET_H_ */

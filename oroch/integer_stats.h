// integer_codec.h
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

#ifndef OROCH_INTEGER_STATS_H_
#define OROCH_INTEGER_STATS_H_

#include <array>
#include <limits>

#include "common.h"
#include "integer_traits.h"

namespace oroch {

template <typename T>
class integer_stats
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	static constexpr size_t nbits = integer_traits<original_t>::nbits;

	// Collect basic sequence info:
	//  * the number of values;
	//  * the minimum value;
	//  * the maximum value.
	template<typename Iter>
	integer_stats(Iter src, Iter const end)
	{
		for (; src != end; ++src)
			add(*src);
	}

	size_t nvalues() const { return nvalues_; }
	original_t min() const { return minvalue_; }
	original_t max() const { return maxvalue_; }

	size_t original_space() const {
		return nvalues() * sizeof(original_t);
	}

	// Collect info for bit-length histogram of values.
	template<typename Iter>
	void
	build_histogram(Iter src, Iter const end)
	{
		// Build the histogram.
		for (; src != end; ++src)
			stat(*src);
	}

	size_t histogram(std::size_t index) const {
		return histogram_[index];
	}

private:
	void
	add(original_t value)
	{
		nvalues_++;
		if (minvalue_ > value)
			minvalue_ = value;
		if (maxvalue_ < value)
			maxvalue_ = value;
	}

	void
	stat(original_t value)
	{
		unsigned_t delta = value - minvalue_;
		size_t index = integer_traits<unsigned_t>::usedcount(delta);
		histogram_[index]++;
	}

	// The total number of values.
	size_t nvalues_ = 0;

	// The minimum and maximum values in the sequence.
	original_t minvalue_ = std::numeric_limits<original_t>::max();
	original_t maxvalue_ = std::numeric_limits<original_t>::min();

	// The log2 histogram of values.
	std::array<size_t, nbits + 1> histogram_ = {};
};

} // namespace oroch

#endif /* OROCH_INTEGER_STATS_H_ */

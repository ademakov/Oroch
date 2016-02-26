// integer_group.h
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

#ifndef OROCH_INTEGER_GROUP_H_
#define OROCH_INTEGER_GROUP_H_

#include <memory>

#include "common.h"
#include "integer_codec.h"

namespace oroch {

template <typename T>
class integer_group
{
public:
	using original_t = T;
	using codec = integer_codec<original_t>;

	static constexpr size_t alignment = 8;
	static constexpr size_t alignment_mask = alignment - 1;

	template<typename Iter>
	void
	encode(Iter begin, Iter const end)
	{
		typename codec::metadata meta;
		codec::select(meta, begin, end);

		const size_t metaspace = meta.metaspace();
		const size_t dataoffset = (metaspace + alignment_mask) & ~alignment_mask;

		data_.reset(new byte_t[dataoffset + meta.dataspace()]);
		dst_bytes_t meta_bytes = data_.get();
		meta.encode(meta_bytes);

		dst_bytes_t data_bytes = data_.get() + dataoffset;
		codec::encode(data_bytes, begin, end, meta);
	}

	template<typename Iter>
	void
	decode(Iter begin, Iter const end) const
	{
		typename codec::metadata meta;
		src_bytes_t meta_bytes = data_.get();
		src_bytes_t meta_start = meta_bytes;
		meta.decode(meta_bytes);

		const size_t metaspace = std::distance(meta_start, meta_bytes);
		const size_t dataoffset = (metaspace + alignment_mask) & ~alignment_mask;

		src_bytes_t data_bytes = data_.get() + dataoffset;
		codec::decode(begin, end, data_bytes, meta);
	}

	void
	decode(typename codec::metadata& meta) const
	{
		src_bytes_t meta_bytes = data_.get();
		meta.decode(meta_bytes);
	}

protected:
	std::unique_ptr<byte_t[]> data_;
};

} // namespace oroch

#endif /* OROCH_INTEGER_GROUP_H_ */

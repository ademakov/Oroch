// integer_array.h
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

#ifndef OROCH_INTEGER_ARRAY_H_
#define OROCH_INTEGER_ARRAY_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "integer_group.h"

namespace oroch {

constexpr size_t not_found = -1;

namespace detail {

constexpr size_t group_size = 256;

template <typename T>
class array_integer_group : public oroch::integer_group<T>
{
public:
	using super = oroch::integer_group<T>;
	using original_t = typename super::original_t;
	using codec = typename super::codec;

	void
	encode(const original_t *buffer)
	{
		super::encode(buffer, buffer + group_size);
	}

	void
	decode(original_t *buffer) const
	{
		super::decode(buffer, buffer + group_size);
	}

	original_t
	operator[](size_t index) const
	{
		std::array<original_t, group_size> buffer;
		decode(buffer.begin());
		return buffer[index];
	}

	size_t
	find(original_t value) const
	{
		size_t nbits;

		typename codec::metadata meta;
		src_bytes_t meta_bytes = super::data_.get();
		src_bytes_t meta_start = meta_bytes;
		meta.decode(meta_bytes);

		const size_t metaspace = std::distance(meta_start, meta_bytes);
		const size_t dataoffset
			= (metaspace + super::alignment_mask) & ~super::alignment_mask;

		src_bytes_t data_bytes = super::data_.get() + dataoffset;
		switch (meta.value_desc.encoding) {
		case encoding_t::naught:
			if (value == meta.value_desc.origin)
				return 0;
			return not_found;

		case encoding_t::normal:
			for (size_t index = 0; index < group_size; index++) {
				original_t decoded
					= *reinterpret_cast<const original_t *>(data_bytes);
				if (value == decoded)
					return index;
				data_bytes += sizeof(original_t);
			}
			return not_found;

		case encoding_t::varint:
			for (size_t index = 0; index < group_size; index++) {
				original_t decoded;
				varint_codec<original_t>::value_decode(decoded, data_bytes);
				if (value == decoded)
					return index;
			}
			return not_found;

		case encoding_t::bitpck:
			nbits = integer_traits<original_t>::usedcount(
					zigzag_codec<original_t>::encode_if_signed(value));
			if (nbits > meta.value_desc.nbits)
				return not_found;
			break;

		case encoding_t::bitfor:
			nbits = integer_traits<original_t>::usedcount(
					value - meta.value_desc.origin);
			if (nbits > meta.value_desc.nbits)
				return not_found;
			break;

		default:
			break;
		}

		std::array<original_t, group_size> buffer;
		decode(buffer.begin());
		for (size_t index = 0; index < group_size; index++) {
			if (buffer[index] == value)
				return index;
		}

		return not_found;
	}

	void
	info(std::ostream &os)
	{
		typename codec::metadata meta;
		meta.clear();
		super::decode(meta);
		os << meta << std::endl;
	}
};

} // namespace oroch::detail


template <typename T>
class integer_array
{
public:
	using original_t = T;

	bool
	empty() const
	{
		return groups_.empty() && tail_.empty();
	}

	size_t
	size() const
	{
		return groups_.size() * detail::group_size + tail_.size();
	}

	original_t
	at(size_t npos) const
	{
		size_t ngroups = groups_.size();
		size_t group = npos / detail::group_size;
		size_t index = npos % detail::group_size;
		if (group > ngroups || index > tail_.size())
			throw std::out_of_range("array index out of range");

		if (group < ngroups)
			return groups_[group][index];
		else
			return tail_[index];
	}

	original_t
	operator[](size_t npos) const
	{
		size_t ngroups = groups_.size();
		size_t group = npos / detail::group_size;
		size_t index = npos % detail::group_size;

		if (group < ngroups)
			return groups_[group].get(index);
		else
			return tail_[index];
	}

	size_t
	find(original_t value) const
	{
		size_t ngroups = groups_.size();

		for (size_t group = 0; group < ngroups; group++) {
			size_t index = groups_[group].find(value);
			if (index != not_found)
				return group * detail::group_size + index;
		}

		auto it = std::find(tail_.begin(), tail_.end(), value);
		if (it != tail_.end())
			return (ngroups * detail::group_size +
				std::distance(tail_.begin(), it));

		return not_found;
	}

	void
	clear()
	{
		groups_.clear();
		tail_.clear();
	}

	void
	insert(size_t array_index, original_t value)
	{
		size_t ngroups = groups_.size();
		size_t group = array_index / detail::group_size;
		size_t index = array_index % detail::group_size;
		if (group > ngroups || index > tail_.size())
			throw std::out_of_range("array index out of range");

		for (; group < ngroups; group++) {
			std::array<original_t, detail::group_size> buffer;
			groups_[group].decode(buffer.begin());

			original_t save_value = buffer[detail::group_size - 1];
			std::copy_backward(buffer.data() + index,
					   buffer.data() + detail::group_size - 1,
					   buffer.data() + detail::group_size);
			buffer[index] = value;

			groups_[group].encode(buffer.begin());
			value = save_value;
			index = 0;
		}

		tail_.insert(tail_.begin() + index, value);
		if (tail_.size() == detail::group_size) {
			groups_.push_back(detail::array_integer_group<original_t>());
			groups_[group].encode(std::addressof(*tail_.begin()));
			tail_.clear();
		}
	}

	void
	group_info(std::ostream &ostream)
	{
		size_t ngroups = groups_.size();
		for (size_t group = 0; group < ngroups; group++)
			groups_[group].info(ostream);
	}

private:
	// The last array elements (their number varies from 0 to group_size - 1).
	std::vector<original_t> tail_;
	// The packed integer groups. Each group conrains group_size elements.
	std::vector<detail::array_integer_group<original_t>> groups_;
};

} // namespace oroch

#endif /* OROCH_INTEGER_ARRAY_H_ */

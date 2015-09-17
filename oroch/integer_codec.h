// integer_codec.h
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

#ifndef OROCH_INTEGER_CODEC_H_
#define OROCH_INTEGER_CODEC_H_

#include <cassert>
#include <limits>
#include <vector>

#include <oroch/integer_traits.h>
#include <oroch/bitpck.h>
#include <oroch/bitfor.h>
#include <oroch/naught.h>
#include <oroch/normal.h>
#include <oroch/origin.h>
#include <oroch/varint.h>
#include <oroch/zigzag.h>

namespace oroch {

enum encoding_t : uint8_t {
	naught = 0,
	normal = 1,
	varint = 2,
	varfor = 3,
	bitpck = 4,
	bitfor = 5,
};

namespace detail {

template <typename T>
struct encoding_descriptor
{
	using original_t = T;

	// The encoding ID.
	encoding_t encoding;

	// The required amount of memory in bytes for data.
	size_t dataspace;
	// The required amount of memory in bytes for metadata
	// excluding single byte for encoding.
	size_t metaspace;

	// The base value for frame-of-reference encodings.
	original_t origin;

	// The number of bits per integer for bit-packing encodings.
	size_t nbits;

	encoding_descriptor()
	{
		clear();
	}

	void
	clear()
	{
		encoding = encoding_t::normal;
		dataspace = 0;
		metaspace = 0;
		origin = 0;
		nbits = 0;
	}
};

template <typename T>
class encoding_statistics
{
public:
	using original_t = T;

	using varint = varint_codec<original_t>;

	size_t nvalues() const { return nvalues_; }
	size_t normalspace() const { return nvalues() * sizeof(original_t); }

	original_t min() const { return minvalue_; }
	original_t max() const { return maxvalue_; }

	void
	add(original_t value)
	{
		nvalues_++;
		if (minvalue_ > value)
			minvalue_ = value;
		if (maxvalue_ < value)
			maxvalue_ = value;
	}

	template<typename Iter>
	void
	collect(Iter src, Iter const send)
	{
		for (; src != send; ++src)
			add(*src);
	}

private:
	// The total number of values.
	size_t nvalues_ = 0;

	// The minimum and maximum values in the sequence.
	original_t minvalue_ = std::numeric_limits<original_t>::max();
	original_t maxvalue_ = std::numeric_limits<original_t>::min();
};

} // namespace oroch::detail

template <typename T>
struct encoding_metadata
{
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	using varint = varint_codec<original_t>;

	detail::encoding_descriptor<original_t> value_desc;

	detail::encoding_descriptor<original_t> outlier_value_desc;
	detail::encoding_descriptor<size_t> outlier_index_desc;

	std::vector<original_t> outlier_value_vec;
	std::vector<size_t> outlier_index_vec;

	size_t
	dataspace() const
	{
		return value_desc.dataspace;
	}

	size_t
	metaspace() const
	{
		return 1 + value_desc.metaspace;
	}

	void
	clear()
	{
		value_desc.clear();
		outlier_value_desc.clear();
		outlier_index_desc.clear();
		outlier_value_vec.clear();
		outlier_index_vec.clear();
	}

	template<typename DstIter>
	bool
	encode(DstIter &dbegin, DstIter const dend) const
	{
		DstIter dst = dbegin;
		if (dst == dend)
			return false;

		encoding_t encoding = value_desc.encoding;
		*dst++ = encoding;

		switch (encoding) {
		case encoding_t::naught:
		case encoding_t::varfor:
			varint::encode(dst, value_desc.origin);
			break;
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitfor:
			varint::encode(dst, value_desc.origin);
			// no break at the end of case
		case encoding_t::bitpck:
			if (dst == dend)
				return false;
			*dst++ = value_desc.nbits;
			break;
		}

		dbegin = dst;
		return true;
	}

	template<typename SrcIter>
	bool
	decode(SrcIter &sbegin, SrcIter const send)
	{
		SrcIter src = sbegin;
		if (src == send)
			return false;

		encoding_t encoding = static_cast<encoding_t>(*src++);
		value_desc.encoding = encoding;

		switch (encoding) {
		case encoding_t::naught:
		case encoding_t::varfor:
			varint::decode(value_desc.origin, src);
			break;
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitfor:
			varint::decode(value_desc.origin, src);
			// no break at the end of case
		case encoding_t::bitpck:
			if (src == send)
				return false;
			value_desc.nbits = *src++;
			break;
		}

		sbegin = src;
		return true;
	}
};

template <typename T>
class integer_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using metadata = encoding_metadata<original_t>;

	using bitpck = bitpck_codec<original_t>;
	using bitfor = bitfor_codec<original_t>;
	using naught = naught_codec<original_t>;
	using normal = normal_codec<original_t>;
	using varint = varint_codec<original_t, zigzag_codec>;
	using varfor = varint_codec<original_t, origin_codec>;
	using origin = origin_codec<original_t>;
	using zigzag = zigzag_codec<original_t>;

	template<typename SrcIter>
	static void
	select(metadata &meta, SrcIter const sbegin, SrcIter const send)
	{
		//
		// Collect value statistics.
		//

		detail::encoding_statistics<original_t> vstat;
		vstat.collect(sbegin, send);

		//
		// Handle trivial corner cases.
		//

		// An empty sequence.
		if (vstat.nvalues() == 0) {
			meta.value_desc.encoding = encoding_t::normal;
			meta.value_desc.dataspace = 0;
			meta.value_desc.metaspace = 0;
			return;
		}

		// A constant or singular sequence.
		if (vstat.min() == vstat.max()) {
			original_t value = vstat.min();
			meta.value_desc.encoding = encoding_t::naught;
			meta.value_desc.dataspace = 0;
			meta.value_desc.metaspace = varint::value_space(value);
			meta.value_desc.origin = value;
			return;
		}

		//
		// Select the best encoding for the sequence.
		//

		select_best(meta, vstat, sbegin, send);
	}

	template<typename DstIter>
	static bool
	encode_meta(DstIter &dbegin, DstIter const dend, metadata &meta)
	{
		return meta.encode(dbegin, dend);
	}

	template<typename SrcIter>
	static bool
	decode_meta(SrcIter &sbegin, SrcIter const send, metadata &meta)
	{
		return meta.decode(sbegin, send);
	}

	template<typename DstIter, typename SrcIter>
	static bool
	encode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       metadata &meta)
	{
		return encode_basic(dbegin, dend, sbegin, send, meta.value_desc);
	}

	template<typename DstIter, typename SrcIter>
	static bool
	decode(DstIter &dbegin, DstIter const dend,
	       SrcIter &sbegin, SrcIter const send,
	       metadata &meta)
	{
		return decode_basic(dbegin, dend, sbegin, send, meta.value_desc);
	}

private:

	template<typename integer_t>
	static void
	compare(detail::encoding_descriptor<integer_t> &desc,
		encoding_t encoding, size_t metaspace, size_t dataspace,
		integer_t origin, size_t nbits)
	{
		if ((dataspace + metaspace) < (desc.dataspace + desc.metaspace)) {
			desc.encoding = encoding;
			desc.dataspace = dataspace;
			desc.metaspace = metaspace;
			desc.origin = origin;
			desc.nbits = nbits;
		}
	}

	template<typename integer_t, typename SrcIter>
	static void
	select_best(metadata &meta,
		    const detail::encoding_statistics<integer_t> &vstat,
		    SrcIter src, SrcIter const send)
	{
		//
		// Select the best basic encoding.
		//

		select_basic(meta.value_desc, vstat, src, send);
		if (vstat.nvalues() < 4)
			return;

		//
		// TODO:
		//
		// Try it against bit-packing with frame of reference and
		// patching.
		//
	}

	template<typename integer_t, typename SrcIter>
	static void
	select_basic(detail::encoding_descriptor<integer_t> &desc,
		     const detail::encoding_statistics<integer_t> &stat,
		     SrcIter src, SrcIter const send)
	{
		size_t dataspace, metaspace, nbits;

		//
		// The base case is the normal representation.
		//

		desc.encoding = encoding_t::normal;
		desc.dataspace = stat.normalspace();

		//
		// Try it against the bit-packed representation.
		//

		// Find the maximum value to be encoded.
		unsigned_t umax;
		if (std::is_signed<original_t>()) {
			unsigned_t min = zigzag::encode(stat.min());
			unsigned_t max = zigzag::encode(stat.max());
			umax = std::max(min, max);
		} else {
			umax = stat.max();
		}

		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(umax);

		// Account for the memory required to encode all data values.
		dataspace = bitpck::block_codec::space(stat.nvalues(), nbits);

		// Finally try it.
		compare(desc, encoding_t::bitpck, 1, dataspace,
			integer_t(0), nbits);

		//
		// Try it against the bit-packed representation with frame of
		// reference.
		//

		// Find the range of values to be encoded.
		unsigned_t range = stat.max() - stat.min();

		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(range);

		// Account for the memory required to encode all data values.
		dataspace = bitpck::block_codec::space(stat.nvalues(), nbits);
		// The memory required to store the nbits and origin values.
		metaspace = 1 + varint::value_space(stat.min());

		// Finally try it.
		compare(desc, encoding_t::bitfor, metaspace, dataspace,
			stat.min(), nbits);

		//
		// Try it against the varint representation.
		//

		// Count the memory footprint of the two kinds of varints.
		const origin orig(stat.min());
		size_t vispace = 0, vfspace = 0;
		for (; src != send; ++src) {
			original_t val = *src;
			vispace += varint::value_space(val);
			vfspace += varfor::value_space(val, orig);
		}

		// The memory required to store the origin value.
		metaspace = varint::value_space(stat.min());

		compare(desc, encoding_t::varint, 0, vispace,
			integer_t(0), 0);
		compare(desc, encoding_t::varfor, metaspace, vfspace,
			stat.min(), 0);
	}

	template<typename integer_t, typename DstIter, typename SrcIter>
	static bool
	encode_basic(DstIter &dbegin, DstIter const dend,
		     SrcIter &sbegin, SrcIter const send,
		     const detail::encoding_descriptor<integer_t> &desc)
	{
		switch(desc.encoding) {
		case encoding_t::naught:
			return naught::encode(dbegin, dend, sbegin, send);
		case encoding_t::normal:
			return normal::encode(dbegin, dend, sbegin, send);
		case encoding_t::varint:
			return varint::encode(dbegin, sbegin, send);
		case encoding_t::varfor:
			return varfor::encode(dbegin, sbegin, send,
					      origin(desc.origin));
		case encoding_t::bitpck:
			return bitpck::encode(dbegin, dend, sbegin, send, desc.nbits);
		case encoding_t::bitfor:
			typename bitfor::parameters params(desc.origin, desc.nbits);
			return bitfor::encode(dbegin, dend, sbegin, send, params);
		}
		return false;
	}

	template<typename integer_t, typename DstIter, typename SrcIter>
	static bool
	decode_basic(DstIter &dbegin, DstIter const dend,
		     SrcIter &sbegin, SrcIter const send,
		     const detail::encoding_descriptor<integer_t> &desc)
	{
		switch(desc.encoding) {
		case encoding_t::naught:
			return naught::decode(dbegin, dend, sbegin, send, desc.origin);
		case encoding_t::normal:
			return normal::decode(dbegin, dend, sbegin, send);
		case encoding_t::varint:
			return varint::decode(dbegin, sbegin, send);
		case encoding_t::varfor:
			return varfor::decode(dbegin, sbegin, send,
					      origin(desc.origin));
		case encoding_t::bitpck:
			return bitpck::decode(dbegin, dend, sbegin, send, desc.nbits);
		case encoding_t::bitfor:
			typename bitfor::parameters params(desc.origin, desc.nbits);
			return bitfor::decode(dbegin, dend, sbegin, send, params);
		}
		return false;
	}
};

} // namespace oroch

#endif /* OROCH_INTEGER_CODEC_H_ */

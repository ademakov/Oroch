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
#include <oroch/varint.h>
#include <oroch/zigzag.h>

namespace oroch {

enum class encoding_t {
        naught = 0,
        normal = 1,
        varint = 2,
        bitpck = 3,
        bitfor = 4,
};

namespace detail {

template <typename T>
struct encoding_descriptor
{
	using original_t = T;

	// The encoding ID.
	encoding_t encoding;

	// The required amount of memory in bytes.
	size_t space;
	size_t metaspace;

	// The base value for frame-of-reference encodings.
	original_t base;

	// The number of bits per integer for bit-packing encodings.
	size_t nbits;

	void
	clear()
	{
		encoding = encoding_t::normal;
		space = 0;
		metaspace = 0;
		base = 0;
		nbits = 0;
	}
};

template <typename T>
struct encoding_statistics
{
	using original_t = T;

	size_t nvalues = 0;
	original_t minvalue = std::numeric_limits<original_t>::max();
	original_t maxvalue = std::numeric_limits<original_t>::min();

	void
	add(original_t value)
	{
		if (minvalue > value)
			minvalue = value;
		if (maxvalue < value)
			maxvalue = value;
		nvalues++;
	}
};

} // namespace oroch::detail

template <typename T>
struct encoding_metadata
{
	using original_t = T;

	detail::encoding_descriptor<original_t> value_desc;
	detail::encoding_descriptor<original_t> outlier_value_desc;
	detail::encoding_descriptor<size_t> outlier_index_desc;

	std::vector<original_t> outlier_value_vec;
	std::vector<size_t> outlier_index_vec;

	void clear()
	{
		value_desc.clear();
		outlier_value_desc.clear();
		outlier_index_desc.clear();
		outlier_value_vec.clear();
		outlier_index_vec.clear();
	}
};

template <typename T>
class integer_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using statistics = detail::encoding_statistics<original_t>;
	using value_desc = detail::encoding_descriptor<original_t>;
	using metadata = encoding_metadata<original_t>;

	using bitpck = bitpck_codec<original_t>;
	using bitfor = bitfor_codec<original_t>;
	using naught = naught_codec<original_t>;
	using normal = normal_codec<original_t>;
	using varint = varint_codec<original_t>;
	using zigzag = zigzag_codec<original_t>;

	template<typename SrcIter>
	static void
	select(metadata &meta, SrcIter const sbegin, SrcIter const send)
	{
		meta.clear();
		select_basic(meta.value_desc, sbegin, send);
	}

	template<typename DstIter>
	static bool
	encode_meta(DstIter &dbegin, DstIter const dend, metadata &meta)
	{
		DstIter dst = dbegin;

		encoding_t encoding = meta.value_desc.encoding;
		if (dst == dend)
			return false;
		*dst++ = encoding;

		switch (encoding) {
		case encoding_t::naught:
			if (!varint::encode(dst, dend, meta.value_desc.base))
				return false;
			// no break at the end of case
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitfor:
			if (!varint::encode(dst, dend, meta.value_desc.base))
				return false;
			// no break at the end of case
		case encoding_t::bitpck:
			if (dst == dend)
				return false;
			*dst++ = meta.value_desc.nbits;
			break;
		}

		dbegin = dst;
		return true;
	}

	template<typename SrcIter>
	static bool
	deecode_meta(SrcIter &sbegin, SrcIter const send, metadata &meta)
	{
		SrcIter src = sbegin;

		if (src == send)
			return false;
		encoding_t encoding = *src++;
		meta.value_desc.encoding = encoding;

		switch (encoding) {
		case encoding_t::naught:
			if (!varint::decode(meta.value_desc.base, src, send))
				return false;
			// no break at the end of case
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitfor:
			if (!varint::decode(meta.value_desc.base, src, send))
				return false;
			// no break at the end of case
		case encoding_t::bitpck:
			if (src == send)
				return false;
			meta.value_desc.nbits = src++;
			break;
		}

		sbegin = src;
		return true;
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

	template<typename SrcIter>
	static void
	select_basic(value_desc &desc, SrcIter const sbegin, SrcIter const send)
	{
		statistics stat;
		size_t space, metaspace, nbits;

		//
		// Collect value statistics and compute the memory
		// footprint of varint representation.
		//

		space = 0;
		for (SrcIter src = sbegin; src != send; ++src) {
			original_t value = *src;
			space += varint::value_space(value);
			stat.add(value);
		}

		desc.encoding = encoding_t::varint;
		desc.space = space;

		//
		// Compute the memory footprint of normal
		// representation.
		//

		space = normal::space(stat.nvalues);
		if (space <= desc.space) {
			desc.encoding = encoding_t::normal;
			desc.space = space;
		}

		//
		// Check for corner cases.
		//

		if (stat.nvalues == 0)
			return;
		if (stat.minvalue == stat.maxvalue) {
			desc.encoding = encoding_t::naught;
			desc.space = 0;
			desc.metaspace = varint::value_space(stat.minvalue);
			desc.base = stat.minvalue;
			return;
		}

		//
		// Compute the memory footprint of bit-packed
		// representation.
		//

		// Find the maximum value to be encoded.
		unsigned_t umaxvalue;
		if (std::is_signed<original_t>()) {
			unsigned_t minvalue = zigzag::encode(stat.minvalue);
			unsigned_t maxvalue = zigzag::encode(stat.maxvalue);
			umaxvalue = std::max(minvalue, maxvalue);
		} else {
			umaxvalue = stat.maxvalue;
		}

		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(umaxvalue);

		// Account for the memory required to encode all values.
		space = bitpck::block_codec::space(stat.nvalues, nbits);
		// The memory required to encode the nbits value.
		metaspace = 1;
		if ((space + metaspace) < (desc.space + desc.metaspace)) {
			desc.encoding = encoding_t::bitpck;
			desc.space = space;
			desc.metaspace = metaspace;
			desc.nbits = nbits;
		}

		//
		// Compute the memory footprint of bit-packed
		// with frame-of-reference representation.
		//

		// Find the range of values to be encoded.
		unsigned_t range = stat.maxvalue - stat.minvalue;

		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(range);

		// Account for the memory required to encode all values.
		space = bitpck::block_codec::space(stat.nvalues, nbits);
		// The memory required to store the nbits and base values.
		metaspace = 1 + varint::value_space(stat.minvalue);
		if ((space + metaspace) < (desc.space + desc.metaspace)) {
			desc.encoding = encoding_t::bitpck;
			desc.space = space;
			desc.metaspace = metaspace;
			desc.base = stat.minvalue;
			desc.nbits = nbits;
		}
	}

	template<typename DstIter, typename SrcIter>
	static bool
	encode_basic(DstIter &dbegin, DstIter const dend,
		     SrcIter &sbegin, SrcIter const send,
		     value_desc &desc)
	{
		switch(desc.encoding) {
		case encoding_t::naught:
			return naught::encode(dbegin, dend, sbegin, send);
		case encoding_t::normal:
			return normal::encode(dbegin, dend, sbegin, send);
		case encoding_t::varint:
			return varint::encode(dbegin, dend, sbegin, send);
		case encoding_t::bitpck:
			return bitpck::encode(dbegin, dend, sbegin, send, desc.nbits);
		case encoding_t::bitfor:
			typename bitfor::parameters params(desc.base, desc.nbits);
			return bitfor::encode(dbegin, dend, sbegin, send, params);
		}
	}

	template<typename DstIter, typename SrcIter>
	static bool
	decode_basic(DstIter &dbegin, DstIter const dend,
		     SrcIter &sbegin, SrcIter const send,
		     value_desc &desc)
	{
		switch(desc.encoding) {
		case encoding_t::naught:
			return naught::decode(dbegin, dend, sbegin, send, desc.base);
		case encoding_t::normal:
			return normal::decode(dbegin, dend, sbegin, send);
		case encoding_t::varint:
			return varint::decode(dbegin, dend, sbegin, send);
		case encoding_t::bitpck:
			return bitpck::decode(dbegin, dend, sbegin, send, desc.nbits);
		case encoding_t::bitfor:
			typename bitfor::parameters params(desc.base, desc.nbits);
			return bitfor::decode(dbegin, dend, sbegin, send, params);
		}
	}
};

} // namespace oroch

#endif /* OROCH_INTEGER_CODEC_H_ */

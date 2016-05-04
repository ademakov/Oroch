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

#ifndef OROCH_INTEGER_CODEC_H_
#define OROCH_INTEGER_CODEC_H_

#include <array>
#include <cassert>
#include <limits>
#include <ostream>
#include <vector>

#include "common.h"
#include "integer_traits.h"
#include "bitpck.h"
#include "bitfor.h"
#include "bitpfr.h"
#include "naught.h"
#include "normal.h"
#include "offset.h"
#include "origin.h"
#include "varint.h"
#include "zigzag.h"

namespace oroch {

enum encoding_t : byte_t {
	naught = 0,
	normal = 1,
	varint = 2,
	varfor = 3,
	bitpck = 4,
	bitfor = 5,
	bitpfr = 6,
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
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	static constexpr size_t nbits = integer_traits<original_t>::nbits;

	size_t nvalues() const { return nvalues_; }
	size_t normalspace() const { return nvalues() * sizeof(original_t); }

	original_t min() const { return minvalue_; }
	original_t max() const { return maxvalue_; }

	// Collect basic sequence info:
	//  * the number of values;
	//  * the minimum value;
	//  * the maximum value.
	template<typename Iter>
	void
	collect_stats(Iter src, Iter const end)
	{
		for (; src != end; ++src)
			add(*src);
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

} // namespace oroch::detail

template <typename T>
struct encoding_metadata
{
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;

	// Common metadata.
	detail::encoding_descriptor<original_t> value_desc;

	// binpfr metadata.
	size_t noutliers = 0;
	detail::encoding_descriptor<size_t> outlier_index_desc;
	detail::encoding_descriptor<unsigned_t> outlier_value_desc;

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

		noutliers = 0;
		outlier_index_desc.clear();
		outlier_value_desc.clear();
	}

	template<typename integer_t>
	void
	encode_basic(dst_bytes_t &dst, const detail::encoding_descriptor<integer_t> &desc) const
	{
		encoding_t encoding = desc.encoding;
		*dst++ = encoding;

		switch (encoding) {
		case encoding_t::naught:
		case encoding_t::varfor:
			varint_codec<integer_t>::value_encode(dst, desc.origin);
			break;
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitpfr:
		case encoding_t::bitfor:
			varint_codec<integer_t>::value_encode(dst, desc.origin);
			// no break at the end of case
		case encoding_t::bitpck:
			*dst++ = desc.nbits;
			break;
		}
	}

	template<typename integer_t>
	void
	decode_basic(src_bytes_t &src, detail::encoding_descriptor<integer_t> &desc)
	{
		encoding_t encoding = static_cast<encoding_t>(*src++);
		desc.encoding = encoding;

		switch (encoding) {
		case encoding_t::naught:
		case encoding_t::varfor:
			varint_codec<integer_t>::value_decode(desc.origin, src);
			break;
		case encoding_t::normal:
		case encoding_t::varint:
			break;
		case encoding_t::bitpfr:
		case encoding_t::bitfor:
			varint_codec<integer_t>::value_decode(desc.origin, src);
			// no break at the end of case
		case encoding_t::bitpck:
			desc.nbits = *src++;
			break;
		}
	}

	template<typename integer_t>
	void
	encode_extra(dst_bytes_t &dst, const detail::encoding_descriptor<integer_t> &desc) const
	{
		*dst++ = desc.encoding == encoding_t::bitpck ? desc.nbits : 0;
	}

	template<typename integer_t>
	void
	decode_extra(src_bytes_t &src, detail::encoding_descriptor<integer_t> &desc)
	{
		byte_t nbits = static_cast<byte_t>(*src++);
		if (nbits == 0) {
			desc.encoding = encoding_t::varint;
		} else {
			desc.encoding = encoding_t::bitpck;
			desc.nbits = nbits;
		}
	}

	void
	encode(dst_bytes_t &dst) const
	{
		encode_basic(dst, value_desc);
		if (value_desc.encoding == encoding_t::bitpfr) {
			varint_codec<size_t>::value_encode(dst, noutliers);
			encode_extra(dst, outlier_index_desc);
			encode_extra(dst, outlier_value_desc);
		}
	}

	void
	decode(src_bytes_t &src)
	{
		decode_basic(src, value_desc);
		if (value_desc.encoding == encoding_t::bitpfr) {
			noutliers = varint_codec<size_t>::value_decode(src);
			decode_extra(src, outlier_index_desc);
			decode_extra(src, outlier_value_desc);
		}
	}
};

template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const encoding_metadata<T> &meta)
{
	os << "encoding: " << static_cast<int>(meta.value_desc.encoding)
	   << ", origin: " << meta.value_desc.origin
	   << ", nbits: " << meta.value_desc.nbits;
	return os;
}

template <typename T>
class integer_codec
{
public:
	using original_t = T;
	using unsigned_t = typename integer_traits<original_t>::unsigned_t;
	using metadata = encoding_metadata<original_t>;

	template<typename Iter>
	static void
	select(metadata &meta, Iter const src, Iter const end)
	{
		//
		// Collect basic value statistics.
		//

		detail::encoding_statistics<original_t> vstat;
		vstat.collect_stats(src, end);

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
			meta.value_desc.metaspace = varint_codec<original_t>::value_space(value);
			meta.value_desc.origin = value;
			return;
		}

		//
		// Select the best basic encoding for the sequence.
		//

		select_basic(meta.value_desc, vstat, src, end);
		if (vstat.nvalues() < 5)
			return;

		//
		// Compare it against patched bit-packing with a frame of
		// reference.
		//

		// The memory required to store the nbits and origin values.
		size_t basic_metaspace = 1 + varint_codec<original_t>::value_space(vstat.min());

		// Find the range of values to be encoded.
		unsigned_t range = vstat.max() - vstat.min();
		// Find the maximum number of bits per value.
		size_t nbits_max = integer_traits<unsigned_t>::usedcount(range);

		vstat.build_histogram(src, end);
		size_t noutliers = vstat.nvalues() - vstat.histogram(0); // outlier values
		for (size_t nbits = 1; nbits < nbits_max; nbits++) {
			size_t n = vstat.histogram(nbits);
			if (n == 0)
				continue;
			noutliers -= n;

			// The memory required to encode all regular bits.
			size_t basic_dataspace = bitpck_codec<unsigned_t>::space(vstat.nvalues(), nbits);

			// Take into account the outliers number and two nbits values.
			size_t extra_metaspace = 2 + varint_codec<size_t>::value_space(noutliers);

			// The memory required to bit-pack the outliers.
			size_t valpck = bitpck_codec<unsigned_t>::space(noutliers, nbits_max - nbits);

			// The memory required to varint-encode the outliers.
			size_t valvar = 0;
			for (size_t nb = nbits + 1; nb <= nbits_max; nb++) {
				size_t space = varint_codec<size_t>::nbits_space(nb - nbits);
				valvar += space * vstat.histogram(nb);
			}

			// Choose between the two encodings for outliers.
			encoding_t value_encoding;
			size_t value_dataspace;
			if (valpck < valvar) {
				value_encoding = encoding_t::bitpck;
				value_dataspace = valpck;
			} else {
				value_encoding = encoding_t::varint;
				value_dataspace = valvar;
			}

			// Get the very minimum memory required for outlier indices
			// and stop here if the required space is too large.
			size_t indmin = bitpck_codec<size_t>::space(noutliers, 1);
			if (indmin > vstat.nvalues())
				indmin = vstat.nvalues();
			size_t estimate = (basic_metaspace + extra_metaspace
					   + basic_dataspace + value_dataspace + indmin);
			size_t selected = meta.value_desc.dataspace + meta.value_desc.metaspace;
			if (estimate >= selected)
				continue;

			// Compute the really required memory for outlier indices.
			size_t indnbits = 1, indvar = 0;
			offset_codec<size_t, 1, false> index_codec(0);
			for (Iter cur = src; cur < end; cur++) {
				unsigned_t u = (*cur - vstat.min()) >> nbits;
				if (u == 0)
					continue;

				size_t i = cur - src;
				unsigned_t j = index_codec.value_encode(i);
				size_t inb = integer_traits<size_t>::usedcount(j);
				if (indnbits < inb)
					indnbits = inb;
				indvar += varint_codec<size_t>::value_space(j);
			}
			size_t indpck = bitpck_codec<size_t>::space(noutliers, indnbits);

			// Choose between the two encodings for outlier indices.
			encoding_t index_encoding;
			size_t index_dataspace;
			if (indpck < indvar) {
				index_encoding = encoding_t::bitpck;
				index_dataspace = indpck;
			} else {
				index_encoding = encoding_t::varint;
				index_dataspace = indvar;
			}

			size_t required = (basic_metaspace + extra_metaspace
					   + basic_dataspace + value_dataspace + index_dataspace);
			if (required < selected) {
				meta.value_desc.encoding = encoding_t::bitpfr;
				meta.value_desc.origin = vstat.min();
				meta.value_desc.nbits = nbits;

				meta.noutliers = noutliers;
				meta.outlier_value_desc.encoding = value_encoding;
				meta.outlier_value_desc.nbits = nbits_max - nbits;
				meta.outlier_index_desc.encoding = index_encoding;
				meta.outlier_index_desc.nbits = indnbits;

				meta.value_desc.metaspace = basic_metaspace + extra_metaspace;
				meta.value_desc.dataspace = basic_dataspace + value_dataspace + index_dataspace;
			}
		}
	}

	template<typename Iter>
	static void
	encode(dst_bytes_t &dst, Iter src, Iter const end, metadata &meta)
	{
		if (meta.value_desc.encoding == encoding_t::bitpfr)
			encode_bitpfr(dst, src, end, meta);
		else
			encode_basic(dst, src, end, meta.value_desc);
	}

	template<typename Iter>
	static void
	decode(Iter dst, Iter const end, src_bytes_t &src, metadata &meta)
	{
		if (meta.value_desc.encoding == encoding_t::bitpfr)
			decode_bitpfr(dst, end, src, meta);
		else
			decode_basic(dst, end, src, meta.value_desc);
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

	template<typename I, typename Iter>
	static void
	select_basic(detail::encoding_descriptor<I> &desc,
		     const detail::encoding_statistics<I> &stat,
		     Iter src, Iter const end)
	{
		size_t dataspace, metaspace, nbits;

		//
		// The base case is the normal encoding.
		//

		desc.encoding = encoding_t::normal;
		desc.dataspace = stat.normalspace();

		//
		// Compare it against the bit-packed encoding.
		//

		// Find the maximum value to be encoded.
		unsigned_t umax;
		if (std::is_signed<I>()) {
			unsigned_t min = zigzag_codec<I>::encode(stat.min());
			unsigned_t max = zigzag_codec<I>::encode(stat.max());
			umax = std::max(min, max);
		} else {
			umax = stat.max();
		}

		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(umax);

		// Account for the memory required to encode all data values.
		dataspace = bitpck_codec<I>::space(stat.nvalues(), nbits);

		// Finally try it.
		compare(desc, encoding_t::bitpck, 1, dataspace, I{0}, nbits);

		//
		// Compare it against the bit-packed encoding with a frame of
		// reference.
		//

		// Find the range of values to be encoded.
		unsigned_t range = stat.max() - stat.min();
		// Find the number of bits per value.
		nbits = integer_traits<unsigned_t>::usedcount(range);

		// Account for the memory required to encode all data values.
		dataspace = bitpck_codec<I>::space(stat.nvalues(), nbits);
		// The memory required to store the nbits and origin values.
		metaspace = 1 + varint_codec<I>::value_space(stat.min());

		// Finally try it.
		compare(desc, encoding_t::bitfor, metaspace, dataspace, stat.min(), nbits);

		//
		// Compare it against the varint encoding.
		//

		// Count the memory footprint of the two kinds of varints.
		const origin_codec<I> orig(stat.min());
		size_t vispace = 0, vfspace = 0;
		for (; src != end; ++src) {
			original_t val = *src;
			vispace += varint_codec<I, zigzag_codec<I>>::value_space(val);
			vfspace += varint_codec<I, origin_codec<I>>::value_space(val, orig);
		}

		// The memory required to store the origin value.
		metaspace = varint_codec<I>::value_space(stat.min());

		compare(desc, encoding_t::varint, 0, vispace, I(0), 0);
		compare(desc, encoding_t::varfor, metaspace, vfspace, stat.min(), 0);
	}

	template<typename I, typename Iter>
	static void
	encode_basic(dst_bytes_t &dst, Iter src, Iter const end,
		     const detail::encoding_descriptor<I> &desc)
	{
		switch(desc.encoding) {
		case encoding_t::bitpfr:
			throw std::logic_error("not a basic encoding");
		case encoding_t::naught:
			naught_codec<I>::encode(dst, src, end);
			break;
		case encoding_t::normal:
			normal_codec<I>::encode(dst, src, end);
			break;
		case encoding_t::varint:
			varint_codec<I, zigzag_codec<I>>::encode(dst, src, end);
			break;
		case encoding_t::varfor:
			varint_codec<I, origin_codec<I>>::encode(dst, src, end,
								 origin_codec<I>(desc.origin));
			break;
		case encoding_t::bitpck:
			bitpck_codec<I>::encode(dst, src, end, desc.nbits);
			break;
		case encoding_t::bitfor:
			typename bitfor_codec<I>::parameters params(desc.origin, desc.nbits);
			bitfor_codec<I>::encode(dst, src, end, params);
			break;
		}
	}

	template<typename I, typename Iter>
	static void
	decode_basic(Iter dst, Iter const end, src_bytes_t &src,
		     const detail::encoding_descriptor<I> &desc)
	{
		switch(desc.encoding) {
		case encoding_t::bitpfr:
			throw std::logic_error("not a basic encoding");
		case encoding_t::naught:
			naught_codec<I>::decode(dst, end, src, desc.origin);
			break;
		case encoding_t::normal:
			normal_codec<I>::decode(dst, end, src);
			break;
		case encoding_t::varint:
			varint_codec<I, zigzag_codec<I>>::decode(dst, end, src);
			break;
		case encoding_t::varfor:
			varint_codec<I, origin_codec<I>>::decode(dst, end, src,
								 origin_codec<I>(desc.origin));
			break;
		case encoding_t::bitpck:
			bitpck_codec<I>::decode(dst, end, src, desc.nbits);
			break;
		case encoding_t::bitfor:
			typename bitfor_codec<I>::parameters params(desc.origin, desc.nbits);
			bitfor_codec<I>::decode(dst, end, src, params);
			break;
		}
	}

	template<typename Iter>
	static void
	encode_bitpfr(dst_bytes_t &dst, Iter src, Iter const end, metadata &meta)
	{
		typename bitpfr_codec<original_t>::exceptions outliers;

		// Encode the regular values and collect the outliers info.
		typename bitpfr_codec<original_t>::parameters params(
				meta.value_desc.origin,
				meta.value_desc.nbits,
				outliers);
		bitpfr_codec<original_t>::encode(dst, src, end, params);

		// Encode the outliers info.
		encode_basic(dst, outliers.indices.begin(), outliers.indices.end(),
			     meta.outlier_index_desc);
		encode_basic(dst, outliers.values.begin(), outliers.values.end(),
			     meta.outlier_value_desc);
	}

	template<typename Iter>
	static void
	decode_bitpfr(Iter dst, Iter const end, src_bytes_t &src, metadata &meta)
	{
		typename bitpfr_codec<original_t>::exceptions outliers;

		// Prepare the outliers storage.
		outliers.indices.resize(meta.noutliers);
		outliers.values.resize(meta.noutliers);

		// Decode the regular values.
		typename bitpfr_codec<original_t>::parameters params(
				meta.value_desc.origin,
				meta.value_desc.nbits,
				outliers);
		bitpfr_codec<original_t>::decode_basic(dst, end, src, params);

		// Decode the outliers info.
		decode_basic(outliers.indices.begin(), outliers.indices.end(),
			     src, meta.outlier_index_desc);
		decode_basic(outliers.values.begin(), outliers.values.end(),
			     src, meta.outlier_value_desc);

		// Patch the outlier values.
		bitpfr_codec<original_t>::decode_patch(dst, params);
	}
};

} // namespace oroch

#endif /* OROCH_INTEGER_CODEC_H_ */

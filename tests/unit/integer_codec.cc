#include "catch.hpp"

#include <array>
#include <oroch/integer_codec.h>

#define INTS 128

TEST_CASE("integer codec", "[codec]") {
	using codec = oroch::integer_codec<uint32_t>;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;
	for (int n = 0; n <= INTS; n++) {
		for (int i = 0; i < n; i++)
			integers[i] = i;

		auto i_it = integers.begin();
		auto i_end = i_it + n;

		codec::metadata meta;
		codec::select(meta, i_it, i_end);

		INFO("encoding: " << (int) meta.value_desc.encoding <<
		     ", space: " << meta.value_desc.dataspace <<
		     ", origin: " <<  meta.value_desc.origin <<
		     ", nbits: " << meta.value_desc.nbits <<
		     ", size: " << n);

		std::vector<uint8_t> bytes(meta.dataspace());
		oroch::dst_bytes_t d_it = bytes.data();
		codec::encode(d_it, i_it, i_end, meta);

		oroch::src_bytes_t b_it = bytes.data();
		i_it = integers2.begin();
		i_end = i_it + n;
		codec::decode(i_it, i_end, b_it, meta);
		for (int i = 0; i < n; i++)
			REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("integer codec metadata", "[codec]") {
	using codec = oroch::integer_codec<uint32_t>;
	std::array<uint32_t, 4> integers {{ 100, 101, 102, 103 }};
	codec::metadata meta;
	codec::metadata meta2;

	auto i_it = integers.begin();
	codec::select(meta, i_it, integers.end());
	std::vector<uint8_t> bytes(meta.metaspace());
	INFO("encoding: " << (int) meta.value_desc.encoding <<
	     ", mataspace: " << meta.value_desc.metaspace);

	oroch::dst_bytes_t d_it = bytes.data();
	meta.encode(d_it);
	oroch::src_bytes_t b_it = bytes.data();
	meta2.decode(b_it);
	REQUIRE(meta2.value_desc.encoding == meta.value_desc.encoding);
}

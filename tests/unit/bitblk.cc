#include "catch.hpp"

#include <algorithm>
#include <array>
#include <vector>

#include <oroch/bitpck.h>

TEST_CASE("bitblk codec for small values", "[bitblk]") {
	using codec = oroch::bitpck_codec<uint32_t>;
	codec::value_codec value_codec;

	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 10> integers {{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9
	}};
	std::array<uint32_t, 10> integers2;

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 4, value_codec);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 4, value_codec);
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 15));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 3, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 3, value_codec);
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 7));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 2, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 2, value_codec);
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 3));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 1, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 1, value_codec);
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 1));
	}
}

TEST_CASE("bitblk codec for large values", "[bitblk]") {
	using codec = oroch::bitpck_codec<uint32_t>;
	codec::value_codec value_codec;

	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 4> integers {{
		0x000aa000, 0xfff66fff, 0x1248abcd, 0x01020408,
	}};
	std::array<uint32_t, 4> integers2;

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 32, value_codec);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 32, value_codec);
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i]));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 31, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 31, value_codec);
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x7fffffff));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 30, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 30, value_codec);
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x3fffffff));
	}

	d_it = bytes.begin();
	i_it = integers.begin();
	codec::block_encode(d_it, i_it, integers.end(), 29, value_codec);

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::block_decode(i_it, integers2.end(), b_it, 29, value_codec);
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x1fffffff));
	}
}

TEST_CASE("bitblk codec fetch", "[bitblk]") {
	using codec = oroch::bitpck_codec<uint64_t>;
	codec::value_codec value_codec;

	std::array<uint64_t, 128> integers;
	for (int i = 0; i < 128; i++)
		integers[i] = i;

	for (int nbits = 1; nbits <= 64; nbits++) {
		std::array<uint8_t, 16> bytes;
		int nints = codec::capacity(nbits);

		auto b_it = bytes.begin();
		auto i_it = integers.begin();
		codec::block_encode(b_it, i_it, i_it + nints, nbits, value_codec);

		for (int i = 0; i < nints; i++) {
			INFO("nbits: " << nbits << ", index: " << i);
			uint32_t v = i & (uint64_t(int64_t(-1)) >> (64 - nbits));
			REQUIRE(codec::fetch(bytes.begin(), i, nbits) == v);
		}
	}
}

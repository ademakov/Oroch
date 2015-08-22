#include "catch.hpp"

#include <algorithm>
#include <array>
#include <vector>

#include <oroch/bitblk.h>

TEST_CASE("bitblk codec for small values", "[bitblk]") {
	using codec = oroch::bitblk_codec<uint32_t>;

	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 10> integers {{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9
	}};
	std::array<uint32_t, 10> integers2;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 4));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 4));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 15));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 3));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 3));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 7));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 2));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 2));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 3));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 1));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 1));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 1));
	}
}

TEST_CASE("bitblk codec for large values", "[bitblk]") {
	using codec = oroch::bitblk_codec<uint32_t>;

	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 4> integers {{
		0x000aa000, 0xfff66fff, 0x1248abcd, 0x01020408,
	}};
	std::array<uint32_t, 4> integers2;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 32));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 32));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i]));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 31));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 31));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x7fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 30));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 30));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x3fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(codec::encode(b_it, bytes.end(), i_it, integers.end(), 29));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(i_it, integers2.end(), b_it, bytes.end(), 29));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x1fffffff));
	}
}

TEST_CASE("bitblk codec fetch", "[bitblk]") {
	using codec = oroch::bitblk_codec<uint64_t>;

	std::array<uint64_t, 128> integers;
	for (int i = 0; i < 128; i++)
		integers[i] = i;

	for (int nbits = 1; nbits <= 64; nbits++) {
		std::array<uint8_t, 16> bytes;
		int nints = codec::capacity(nbits);

		auto b_it = bytes.begin();
		auto i_it = integers.begin();
		REQUIRE(codec::encode(b_it, bytes.end(), i_it, i_it + nints, nbits));

		for (int i = 0; i < nints; i++) {
			INFO("nbits: " << nbits << ", index: " << i);
			uint32_t v = i & (uint64_t(int64_t(-1)) >> (64 - nbits));
			REQUIRE(codec::fetch(bytes.begin(), i, nbits) == v);
		}
	}
}

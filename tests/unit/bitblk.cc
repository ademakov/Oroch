#include "catch.hpp"

#include <array>
#include <oroch/bitblk.h>

using bitblk32 = oroch::bitblk_codec<uint32_t>;

TEST_CASE("bitblk codec for small values", "[bitblk]") {
	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 10> integers {{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9
	}};
	std::array<uint32_t, 10> integers2;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(bitblk32::encode(4, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(4, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 15));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(3, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(3, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 7));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(2, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(2, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 3));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(1, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(1, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 1));
	}
}

TEST_CASE("bitblk codec for large values", "[bitblk]") {
	std::array<uint8_t, 16> bytes;
	std::array<uint32_t, 4> integers {{
		0x000aa000, 0xfff66fff, 0x1248abcd, 0x01020408,
	}};
	std::array<uint32_t, 4> integers2;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(bitblk32::encode(32, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(32, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i]));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(31, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(31, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x7fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(30, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(30, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x3fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(29, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(29, i_it, integers2.end(), b_it, bytes.end()));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x1fffffff));
	}
}

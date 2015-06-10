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
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 4));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 4));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 15));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 3));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 3));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 7));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 2));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 2));
	for (int i = 0; i < 10; i++) {
		REQUIRE(integers2[i] == (integers[i] & 3));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 1));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 1));
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
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 32));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 32));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i]));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 31));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 31));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x7fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 30));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 30));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x3fffffff));
	}

	b_it = bytes.begin();
	i_it = integers.begin();
	REQUIRE(bitblk32::encode(b_it, bytes.end(), i_it, integers.end(), 29));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(bitblk32::decode(i_it, integers2.end(), b_it, bytes.end(), 29));
	for (int i = 0; i < 4; i++) {
		REQUIRE(integers2[i] == (integers[i] & 0x1fffffff));
	}
}

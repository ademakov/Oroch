#include "catch.hpp"

#include <array>
#include <oroch/bitpck.h>

#define BITS 7
#define INTS 128

TEST_CASE("bitpck codec for unsigned values", "[bitpck]") {
	using codec = oroch::bitpck_codec<uint32_t>;
	std::array<uint8_t, codec::block_codec::block_volume(BITS, INTS)> bytes;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;

	for (int i = 0; i < INTS; i++) {
		integers[i] = i;
	}

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(codec::encode(BITS, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(BITS, i_it, integers2.end(), b_it, bytes.end()));

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitpck codec for signed values", "[bitpck]") {
	using codec = oroch::bitpck_codec<int32_t>;
	std::array<uint8_t, codec::block_codec::block_volume(BITS, INTS)> bytes;
	std::array<int32_t, INTS> integers;
	std::array<int32_t, INTS> integers2;

	for (int i = 0; i < INTS; i++) {
		integers[i] = i - 64;
	}

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	REQUIRE(codec::encode(BITS, b_it, bytes.end(), i_it, integers.end()));

	b_it = bytes.begin();
	i_it = integers2.begin();
	REQUIRE(codec::decode(BITS, i_it, integers2.end(), b_it, bytes.end()));

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

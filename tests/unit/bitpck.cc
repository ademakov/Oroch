#include "catch.hpp"

#include <array>
#include <oroch/bitpck.h>

#define BITS 7
#define INTS 128

TEST_CASE("bitpck codec for unsigned values", "[bitpck]") {
	using codec = oroch::bitpck_codec<uint32_t>;
	std::array<uint8_t, codec::space(INTS, BITS)> bytes;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;

	for (int i = 0; i < INTS; i++) {
		integers[i] = i;
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), BITS);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, BITS);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitpck codec for signed values", "[bitpck]") {
	using codec = oroch::bitpck_codec<int32_t>;
	std::array<uint8_t, codec::space(INTS, BITS)> bytes;
	std::array<int32_t, INTS> integers;
	std::array<int32_t, INTS> integers2;

	for (int i = 0; i < INTS; i++) {
		integers[i] = i - 64;
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), BITS);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, BITS);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitpck codec fetch", "[bitpck]") {
	using codec = oroch::bitpck_codec<uint64_t>;
	std::array<uint8_t, codec::space(INTS, BITS)> bytes;
	std::array<uint32_t, INTS> integers;
	for (int i = 0; i < INTS; i++)
		integers[i] = i;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(b_it, i_it, integers.end(), BITS);

	b_it = bytes.begin();
	for (int i = 0; i < INTS; i++)
		REQUIRE(codec::fetch(bytes.begin(), i, BITS) == i);
}


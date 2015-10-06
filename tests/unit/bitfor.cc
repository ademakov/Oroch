#include "catch.hpp"

#include <array>
#include <oroch/bitfor.h>

#define BITS 7
#define INTS 128

#define FREF 1000

TEST_CASE("bitfor codec for unsigned values", "[bitfor]") {
	using codec = oroch::bitfor_codec<uint32_t>;
	std::array<uint8_t, codec::basic_codec::space(INTS, BITS)> bytes;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;
	codec::parameters params(FREF, BITS);

	for (int i = 0; i < INTS; i++) {
		integers[i] = i + FREF;
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), params);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, params);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitfor codec for signed values", "[bitfor]") {
	using codec = oroch::bitfor_codec<int32_t>;
	std::array<uint8_t, codec::basic_codec::space(INTS, BITS)> bytes;
	std::array<int32_t, INTS> integers;
	std::array<int32_t, INTS> integers2;
	codec::parameters params(-FREF, BITS);

	for (int i = 0; i < INTS; i++) {
		integers[i] = i - FREF;
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), params);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, params);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitfor codec fetch", "[bitfor]") {
	using codec = oroch::bitfor_codec<uint64_t>;
	std::array<uint8_t, codec::basic_codec::space(INTS, BITS)> bytes;
	std::array<uint32_t, INTS> integers;
	codec::parameters params(FREF, BITS);

	for (int i = 0; i < INTS; i++)
		integers[i] = i + FREF;

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(b_it, i_it, integers.end(), params);

	b_it = bytes.begin();
	for (int i = 0; i < INTS; i++)
		REQUIRE(codec::fetch(bytes.begin(), i, params) == (i + FREF));
}

#include "catch.hpp"

#include <array>
#include <oroch/normal.h>

#define INTS 128

TEST_CASE("normal value codec", "[normal]") {
	using codec = oroch::normal_codec<int32_t>;
	std::array<uint8_t, codec::space(INTS)> bytes;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;

	for (int i = 0; i < INTS; i++) {
		integers[i] = i - INTS / 2;
	}

	auto b_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(b_it, i_it, integers.end());

	b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

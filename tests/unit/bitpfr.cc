#include "catch.hpp"

#include <array>
#include <oroch/bitpfr.h>

#define BITS	7
#define SMALL	128
#define LARGE	24
#define INTS	(SMALL + LARGE)

#define FREF	1000

TEST_CASE("bitpfr value codec", "[bitpfr]") {
	using codec = oroch::bitpfr_codec<uint32_t>;
	codec::exceptions excpts;
	codec::parameters params(FREF, BITS, excpts);

	params.value_encode(FREF);
	REQUIRE(excpts.index == 1);
	REQUIRE(excpts.indices.size() == 0);
	REQUIRE(excpts.values.size() == 0);

	params.value_encode(FREF + (1 << BITS));
	REQUIRE(excpts.index == 2);
	REQUIRE(excpts.indices.size() == 1);
	REQUIRE(excpts.indices[0] == 1);
	REQUIRE(excpts.values.size() == 1);
	REQUIRE(excpts.values[0] == 1);
}

TEST_CASE("bitpfr codec for unsigned values", "[bitpfr]") {
	using codec = oroch::bitpfr_codec<uint32_t>;
	std::array<uint8_t, codec::basic_codec::space(INTS, BITS)> bytes;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;
	codec::exceptions excpts;
	codec::parameters params(FREF, BITS, excpts);

	for (int i = 0; i < SMALL; i++) {
		integers[i] = i + FREF;
	}
	for (int i = SMALL; i < INTS; i++) {
		integers[i] = FREF + (1 << (BITS + (i - SMALL)));
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), params);
	REQUIRE(excpts.indices.size() == LARGE);
	REQUIRE(excpts.values.size() == LARGE);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, params);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

TEST_CASE("bitpfr codec for signed values", "[bitpfr]") {
	using codec = oroch::bitpfr_codec<int32_t>;
	std::array<uint8_t, codec::basic_codec::space(INTS, BITS)> bytes;
	std::array<int32_t, INTS> integers;
	std::array<int32_t, INTS> integers2;
	codec::exceptions excpts;
	codec::parameters params(-FREF, BITS, excpts);

	for (int i = 0; i < SMALL; i++) {
		integers[i] = i - FREF;
	}
	for (int i = SMALL; i < INTS; i++) {
		integers[i] = FREF + (1 << (BITS + (i - SMALL)));
	}

	oroch::dst_bytes_t d_it = bytes.begin();
	auto i_it = integers.begin();
	codec::encode(d_it, i_it, integers.end(), params);
	REQUIRE(excpts.indices.size() == LARGE);
	REQUIRE(excpts.values.size() == LARGE);

	oroch::src_bytes_t b_it = bytes.begin();
	i_it = integers2.begin();
	codec::decode(i_it, integers2.end(), b_it, params);

	for (int i = 0; i < INTS; i++) {
		REQUIRE(integers2[i] == integers[i]);
	}
}

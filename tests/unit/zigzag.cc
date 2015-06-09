#include "catch.hpp"

#include <array>
#include <oroch/zigzag.h>

using zigzag32 = oroch::zigzag_codec<int32_t>;
using zigzag64 = oroch::zigzag_codec<int64_t>;

TEST_CASE("zigzag codec for int32_t", "[zigzag]") {
	REQUIRE(zigzag32::encode(0) == 0);
	REQUIRE(zigzag32::encode(-1) == 1);
	REQUIRE(zigzag32::encode(1) == 2);
	REQUIRE(zigzag32::encode(-2) == 3);
	REQUIRE(zigzag32::encode(2) == 4);

	static std::array<int32_t, 9> values = {{
		0, -1, 1, -2, 2,
		std::numeric_limits<std::int32_t>::max() - 1,
		std::numeric_limits<std::int32_t>::min() + 1,
		std::numeric_limits<std::int32_t>::max(),
		std::numeric_limits<std::int32_t>::min(),
	}};

	for (int32_t value : values) {
		uint32_t encoded = zigzag32::encode(value);
		int32_t decoded = zigzag32::decode(encoded);
		REQUIRE(value == decoded);
	}
}

TEST_CASE("zigzag codec for int64_t", "[zigzag]") {
	REQUIRE(zigzag64::encode(0) == 0);
	REQUIRE(zigzag64::encode(-1) == 1);
	REQUIRE(zigzag64::encode(1) == 2);
	REQUIRE(zigzag64::encode(-2) == 3);
	REQUIRE(zigzag64::encode(2) == 4);

	static std::array<int64_t, 13> values = {{
		0, -1, 1, -2, 2,
		std::numeric_limits<std::int32_t>::max() - 1,
		std::numeric_limits<std::int32_t>::min() + 1,
		std::numeric_limits<std::int32_t>::max(),
		std::numeric_limits<std::int32_t>::min(),
		std::numeric_limits<std::int64_t>::max() - 1,
		std::numeric_limits<std::int64_t>::min() + 1,
		std::numeric_limits<std::int64_t>::max(),
		std::numeric_limits<std::int64_t>::min(),
	}};

	for (int32_t value : values) {
		uint32_t encoded = zigzag64::encode(value);
		int32_t decoded = zigzag64::decode(encoded);
		REQUIRE(value == decoded);
	}
}

TEST_CASE("zigzag codec conditional methods", "[zigzag]") {
	REQUIRE(oroch::zigzag_codec<int32_t>().encode_if_signed(1) == 2);
	REQUIRE(oroch::zigzag_codec<uint32_t>().encode_if_signed(1) == 1);
	REQUIRE(oroch::zigzag_codec<int32_t>().decode_if_signed(2) == 1);
	REQUIRE(oroch::zigzag_codec<uint32_t>().decode_if_signed(2) == 2);
}

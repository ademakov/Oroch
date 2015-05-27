#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <oroch/zigzag.h>

using zigzag32 = oroch::zigzag_codec<int32_t>;
using zigzag64 = oroch::zigzag_codec<int64_t>;

TEST_CASE( "zigzag codec for int32_t", "[zigzag]" ) {
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

TEST_CASE( "zigzag codec for int64_t", "[zigzag]" ) {
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

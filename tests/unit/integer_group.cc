#include "catch.hpp"

#include <array>
#include <oroch/integer_group.h>

#define INTS 8

TEST_CASE("integer group", "[group]") {
	oroch::integer_group<uint32_t> group;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;

	for (int t = 0; t < 10000; t++) {
		for (size_t i = 0; i < INTS; i++)
			integers[i] = random() & 0xfff;
		group.encode(integers.begin(), integers.end());
		group.decode(integers2.begin(), integers2.end());
		for (size_t i = 0; i < INTS; i++)
			REQUIRE(integers2[i] == integers[i]);
	}
}

#include "catch.hpp"

#include <oroch/integer_array.h>

using int32_array = oroch::integer_array<int32_t>;

TEST_CASE("small integer array", "[array]")
{
	int32_array array;
	REQUIRE(array.find(0) == oroch::not_found);
	REQUIRE(array.find(1) == oroch::not_found);
	array.insert(0, 0);
	REQUIRE(array.at(0) == 0);
	REQUIRE(array.find(0) == 0);
	array.insert(0, 1);
	REQUIRE(array.at(0) == 1);
	REQUIRE(array.at(1) == 0);
	REQUIRE(array.find(0) == 1);
	REQUIRE(array.find(1) == 0);
}

TEST_CASE("large integer array", "[array]")
{
	const size_t n = 100000;
	int32_array array;
	for (size_t i = 0; i < n; i++)
		array.insert(i, i);
	array.insert(0, -1);
	for (size_t i = 0; i < n; i++)
		REQUIRE(array.find(i) == (i + 1));
	REQUIRE(array.find(-1) == 0);
}

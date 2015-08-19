#include "catch.hpp"

#include <array>
#include <oroch/integer_codec.h>

#define INTS 128

TEST_CASE("integer codec", "[codec]") {
	using codec = oroch::integer_codec<uint32_t>;
	std::array<uint32_t, INTS> integers;
	std::array<uint32_t, INTS> integers2;
	for (int n = 0; n <= INTS; n++) {
		for (int i = 0; i < n; i++)
			integers[i] = i;

		auto i_it = integers.begin();
		auto i_end = i_it + n;

		codec::metadata meta;
		codec::select(meta, i_it, i_end);

		INFO("encoding: " << (int) meta.value_desc.encoding <<
		     ", space: " << meta.value_desc.space <<
		     ", base: " <<  meta.value_desc.base <<
		     ", nbits: " << meta.value_desc.nbits <<
		     ", size: " << n);

		std::vector<uint8_t> bytes(meta.value_desc.space);
		auto b_it = bytes.begin();
		REQUIRE(codec::encode(b_it, bytes.end(), i_it, i_end, meta));

		b_it = bytes.begin();
		i_it = integers2.begin();
		i_end = i_it + n;
		REQUIRE(codec::decode(i_it, i_end, b_it, bytes.end(), meta));
		for (int i = 0; i < n; i++)
			REQUIRE(integers2[i] == integers[i]);
	}
}

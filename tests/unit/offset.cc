#include "catch.hpp"

#include <oroch/offset.h>

TEST_CASE("offset value codec", "[offset]") {
	using codec = oroch::offset_codec<int32_t, 1>;

	codec coder_0(0, false);
	REQUIRE(coder_0.value_encode(1) == 1);
	REQUIRE(coder_0.value_encode(2) == 0);
	REQUIRE(coder_0.value_encode(3) == 0);
	REQUIRE(coder_0.value_encode(5) == 1);

	codec decoder_0(0, false);
	REQUIRE(decoder_0.value_decode(1) == 1);
	REQUIRE(decoder_0.value_decode(0) == 2);
	REQUIRE(decoder_0.value_decode(0) == 3);
	REQUIRE(decoder_0.value_decode(1) == 5);

	codec coder_1(0, true);
	REQUIRE(coder_1.value_encode(1) == 0);
	REQUIRE(coder_1.value_encode(2) == 0);
	REQUIRE(coder_1.value_encode(3) == 0);
	REQUIRE(coder_1.value_encode(5) == 1);

	codec decoder_1(0, true);
	REQUIRE(decoder_1.value_decode(0) == 1);
	REQUIRE(decoder_1.value_decode(0) == 2);
	REQUIRE(decoder_1.value_decode(0) == 3);
	REQUIRE(decoder_1.value_decode(1) == 5);
}

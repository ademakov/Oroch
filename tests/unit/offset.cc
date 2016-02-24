#include "catch.hpp"

#include <oroch/offset.h>

TEST_CASE("offset value codec", "[offset]") {
	using codec_0 = oroch::offset_codec<int32_t, 1, false>;
	using codec_1 = oroch::offset_codec<int32_t, 1, true>;

	codec_0 coder_0(0);
	REQUIRE(coder_0.value_encode(1) == 1);
	REQUIRE(coder_0.value_encode(2) == 0);
	REQUIRE(coder_0.value_encode(3) == 0);
	REQUIRE(coder_0.value_encode(5) == 1);

	codec_0 decoder_0(0);
	REQUIRE(decoder_0.value_decode(1) == 1);
	REQUIRE(decoder_0.value_decode(0) == 2);
	REQUIRE(decoder_0.value_decode(0) == 3);
	REQUIRE(decoder_0.value_decode(1) == 5);

	codec_1 coder_1(0);
	REQUIRE(coder_1.value_encode(1) == 0);
	REQUIRE(coder_1.value_encode(2) == 0);
	REQUIRE(coder_1.value_encode(3) == 0);
	REQUIRE(coder_1.value_encode(5) == 1);

	codec_1 decoder_1(0);
	REQUIRE(decoder_1.value_decode(0) == 1);
	REQUIRE(decoder_1.value_decode(0) == 2);
	REQUIRE(decoder_1.value_decode(0) == 3);
	REQUIRE(decoder_1.value_decode(1) == 5);
}

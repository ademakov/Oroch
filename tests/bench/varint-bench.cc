#include <cstdlib>
#include <chrono>
#include <iostream>
#include <vector>

#include <oroch/varint.h>

#define SIZE	10000000
#define REPEAT	100

static std::vector<uint32_t> v, v2;
static std::vector<uint8_t> x;

int
main()
{
	for (int i = 0; i < SIZE; i++)
		v.push_back(i);

	size_t size = oroch::varint_codec<uint32_t>::space(v.begin(), v.end());

	x.resize(size);
	uint8_t *data = x.data();
	oroch::varint_codec<uint32_t>::encode(data, v.begin(), v.end());

	v2.resize(v.size());

	auto start = std::chrono::high_resolution_clock::now();

	for (int r = 0; r < REPEAT; r++) {
		const uint8_t *data = x.data();
		oroch::varint_codec<uint32_t>::decode(v2.begin(), v2.end(), data);
	}

	auto finish = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "decode time : " << time.count() << " ms" << std::endl;

	return EXIT_SUCCESS;
}

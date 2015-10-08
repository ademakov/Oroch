#include <cstdlib>
#include <chrono>
#include <iostream>
#include <vector>

#include <oroch/integer_group.h>

#define SIZE	1000
#define REPEAT	1000000

static std::vector<int> v, v2;
static oroch::integer_group<int> g;

int
main()
{
	for (int i = 0; i < SIZE; i++)
		v.push_back(1000 + i);
	g.encode(v.begin(), v.end());

	oroch::integer_group<int>::codec::metadata meta;
	g.decode(meta);
	std::cout << meta << std::endl;

	v2.resize(v.size());

	auto start = std::chrono::high_resolution_clock::now();

	for (int r = 0; r < REPEAT; r++)
		g.decode(v2.begin(), v2.end());

	auto finish = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "decode time : " << time.count() << " ms" << std::endl;

	return EXIT_SUCCESS;
}

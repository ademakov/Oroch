#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include <oroch/integer_array.h>

#define SIZE 10000

static std::vector<int> v;
static oroch::integer_array<int> o;

static size_t v_result[2];
static size_t o_result[2];

static void
v_find(int x)
{
	auto iter = std::find(v.begin(), v.end(), x);
	if (iter == v.end())
		v_result[1]++;
	else
		v_result[0]++;
}

static void
o_find(int x)
{
	size_t index = o.find(x);
	if (index == oroch::not_found)
		o_result[1]++;
	else
		o_result[0]++;
}

static void
test(const std::string &title, void (*func)(int x), size_t result[2])
{
	auto start = std::chrono::high_resolution_clock::now();

	for (int r = 0; r < 1000; r++) {
		for (size_t i = 0; i < SIZE; i++)
			(*func)(i - 100);
	}

	auto finish = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << title << ": " << time.count() << ", found: " << result[0]
		  << ", not found: " << result[1] << std::endl;
}

int
main()
{
	for (int i = 0; i < SIZE; i++)
		v.push_back(i - 100);

	for (int i = 0; i < SIZE; i++)
		o.insert(i, v[i]);
	o.group_info(std::cout);

	test("oroch::integer_array", o_find, o_result);
	test("std::vector", v_find, v_result);

	return EXIT_SUCCESS;
}

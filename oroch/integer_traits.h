// integer_traits.h
//
// Copyright (c) 2015  Aleksey Demakov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef OROCH_INTEGER_TRAITS_H_
#define OROCH_INTEGER_TRAITS_H_

#include <type_traits>

namespace oroch {

template <typename T>
struct promote_integer
{
	using type = T;
};

template <>
struct promote_integer<signed char>
{
	using type = int;
};
template <>
struct promote_integer<unsigned char>
{
	using type = int;
};

#if !__LP32__
template <>
struct promote_integer<signed short>
{
	using type = int;
};
template <>
struct promote_integer<unsigned short>
{
	using type = int;
};
#endif

template <typename T>
struct integer_builtins
{
};

template<>
struct integer_builtins<int>
{
	using unsigned_t = unsigned int;

	static int ffs(unsigned_t value) { return __builtin_ffs(value); }
	static int clz(unsigned_t value) { return __builtin_clz(value); }
	static int ctz(unsigned_t value) { return __builtin_ctz(value); }
	static int popcount(unsigned_t value) { return __builtin_popcount(value); }
};

template<>
struct integer_builtins<long int>
{
	using unsigned_t = unsigned long int;

	static int ffs(unsigned_t value) { return __builtin_ffsl(value); }
	static int clz(unsigned_t value) { return __builtin_clzl(value); }
	static int ctz(unsigned_t value) { return __builtin_ctzl(value); }
	static int popcount(unsigned_t value) { return __builtin_popcountl(value); }
};

template<>
struct integer_builtins<long long int>
{
	using unsigned_t = unsigned long long int;

	static int ffs(unsigned_t value) { return __builtin_ffsll(value); }
	static int clz(unsigned_t value) { return __builtin_clzll(value); }
	static int ctz(unsigned_t value) { return __builtin_ctzll(value); }
	static int popcount(unsigned_t value) { return __builtin_popcountll(value); }
};

template <typename T>
struct integer_traits
{
	using signed_t = typename std::make_signed<T>::type;
	using unsigned_t = typename std::make_unsigned<T>::type;
	using promoted_signed_t = typename promote_integer<signed_t>::type;
	using promoted_unsigned_t = typename promote_integer<unsigned_t>::type;

	static constexpr int nbytes = sizeof(signed_t);
	static constexpr int nbits = nbytes * 8;

	static constexpr int promoted_nbytes = sizeof(promoted_signed_t);
	static constexpr int promoted_nbits = promoted_nbytes * 8;

	static int ffs(unsigned_t value)
	{
		return integer_builtins<promoted_signed_t>::ffs(value);
	}
	static int clz(unsigned_t value)
	{
		return integer_builtins<promoted_signed_t>::clz(value);
	}
	static int ctz(unsigned_t value)
	{
		return integer_builtins<promoted_signed_t>::ctz(value);
	}
	static int popcount(unsigned_t value)
	{
		return integer_builtins<promoted_signed_t>::popcount(value);
	}

	static int usedcount(unsigned_t value)
	{
		return value ? promoted_nbits - clz(value) : 0;
	}
};

} // namespace oroch

#endif /* OROCH_INTEGER_TRAITS_H_ */

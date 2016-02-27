# Oroch
A C++ library for integer array compression.

The focus of the library is uniform handling of the different integer types.
The same template-based interface deals with short and long, unsigned and
signed types. Below is a sample of the library use:

```C++
    std::array<int, 6> ints = { 1, 100, 10000, -1, -100, -10000 };
    std::array<size_t, 6> sizes { 1, 100, 10000, 1, 100, 10000 };

    // Get the memory space required to encode the arrays.
    size_t ints_space = oroch::varint_codec<int>::space(ints.begin(), ints.end());
    size_t sizes_space = oroch::varint_codec<size_t>::space(sizes.begin(), sizes.end());
    std::cout << ints_space << "\n" << sizes_space << "\n";

    // Allocate the required memory.
    std::unique_ptr<uint8_t[]> store(new uint8_t[ints_space + sizes_space]);

    // Encode the arrays.
    uint8_t *ptr = store.get();
    oroch::varint_codec<int>::encode(ptr, ints.begin(), ints.end());
    assert(ptr == (store.get() + ints_space));
    oroch::varint_codec<size_t>::encode(ptr, sizes.begin(), sizes.end());
    assert(ptr == (store.get() + ints_space + sizes_space));

```

The output of this sample would be like this:

```
12
8
```

The template mechanism provided by the library automatically applies zigzag
encoding to the int type and only after that uses the varint codec. For the
size_t type the zigzag codec is avoided.

In addition to the varint codec the library also provides bit-packing codecs:

* basic bit-packing codec (in "oroch/bitpck.h"),
* bit-packing with a frame-of-reference technique (in "oroch/bitfor.h"),
* bit-packing with a frame-of-reference and patching (in "oroch/bitpfr.h").

The best choice among these codecs depends on the input data. The library
provides a utility class that compares different codecs against a given input
and selects the best. The class is defined in the "oroch/integer_codec.h"
header. This utility has somewhat complicated interface though. An example
of how to properly use it is provided n the "oroch/integer_group.h" header.

## Comparison

There are already many integer compression libraies available:

* A few different libraries here: https://github.com/lemire
* Another one here: https://github.com/powturbo/TurboPFor

It seems that these libraries are extremely good at what they do. Mostly they
focus on the speed. To this end they limit other features and flexibility.
For instance, some of the libraies handle only 32-bit integers. Or implement
a narrow set of compression algorithms.

The focus of the Oroch library is flexibility and ability to switch to other
compression method by changing just a single line of the code. It is also
realtively small compared to other libraries.

If your project does not need to decode billions of integers per second and
could trade this for smaller and more manageable source code base, then the
Oroch library might be for you.

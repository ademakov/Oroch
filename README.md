# Oroch
A C++ library for integer array compression.

The focus of the library is uniform handling of the different integer types.
The same interface deals with short and long, signed and unsigned types with
the help of template programming.

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

The output of this sample code would be 

```
12
8
```

The template mechanism provided by the library automatically applies zigzag encoding
to the int type and only after that uses varint codec.

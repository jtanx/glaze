# MessagePack Support in Glaze

Glaze now includes full support for MessagePack serialization and deserialization, providing a compact binary alternative to JSON.

## What is MessagePack?

MessagePack is an efficient binary serialization format that is more compact than JSON while remaining fast and simple. It's particularly useful for:

- Network communication where bandwidth is limited
- Storage optimization
- Inter-process communication
- API payloads that don't need human readability

## Features

The Glaze MessagePack implementation provides:

- ✅ **Complete Format Support**: All MessagePack types including integers, floats, strings, arrays, maps, and extension types
- ✅ **Type Safety**: Full C++ type safety with compile-time type checking
- ✅ **Zero Dependencies**: No external MessagePack libraries required
- ✅ **Consistent API**: Same familiar Glaze API patterns as JSON and BEVE
- ✅ **High Performance**: Optimized binary serialization with minimal overhead
- ✅ **Big-Endian Encoding**: Proper byte order handling for cross-platform compatibility

## Basic Usage

### Including the Header

```cpp
#include "glaze/glaze.hpp"
#include "glaze/msgpack.hpp"
```

### Writing (Serialization)

```cpp
struct person {
   std::string name;
   int age;
   std::vector<std::string> hobbies;
};

template <>
struct glz::meta<person> {
   using T = person;
   static constexpr auto value = object(
      "name", &T::name,
      "age", &T::age,
      "hobbies", &T::hobbies
   );
};

person p{"Alice", 25, {"reading", "coding"}};

// Serialize to MessagePack
std::string buffer;
auto ec = glz::write_msgpack(p, buffer);
```

### Reading (Deserialization)

```cpp
// Deserialize from MessagePack
person p2;
auto ec = glz::read_msgpack(p2, buffer);
```

## Supported Types

The MessagePack implementation supports all standard C++ types that Glaze supports:

### Primitive Types
- **Integers**: `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- **Floating Point**: `float`, `double`
- **Boolean**: `bool`
- **Null**: `std::nullptr_t`, `std::nullopt`

### String Types
- `std::string`
- `std::string_view`
- C-style strings (null-terminated char arrays)

### Container Types
- **Arrays**: `std::vector`, `std::array`, `std::deque`, `std::list`
- **Maps**: `std::map`, `std::unordered_map`
- **Optional**: `std::optional`
- **Variant**: `std::variant`

### User-Defined Types
- Structs with Glaze metadata
- Nested structures
- Complex nested containers

## MessagePack Format Details

The implementation follows the [MessagePack specification](https://msgpack.org/):

- **Integers**: Automatically uses the most compact representation (fixint, uint8-64, int8-64)
- **Strings**: Supports fixstr, str8, str16, and str32 formats
- **Arrays**: Supports fixarray, array16, and array32 formats
- **Maps**: Supports fixmap, map16, and map32 formats
- **Big-Endian**: All multi-byte values use network byte order

## Performance Comparison

MessagePack typically provides:
- **8-30% smaller** payload size compared to JSON
- **Faster parsing** due to binary format (no string parsing)
- **Faster serialization** (no string formatting)

Example from our tests:
```
JSON:        125 bytes
MessagePack: 115 bytes (8% reduction)
```

## API Reference

### Write Functions

```cpp
// Write to buffer
template <typename T, typename Buffer>
error_ctx write_msgpack(T&& value, Buffer&& buffer);

// Write and return as string
template <typename T>
expected<std::string, error_ctx> write_msgpack(T&& value);
```

### Read Functions

```cpp
// Read into existing object
template <typename T, typename Buffer>
error_ctx read_msgpack(T&& value, Buffer&& buffer);

// Read and return new object
template <typename T, typename Buffer>
expected<T, error_ctx> read_msgpack(Buffer&& buffer);
```

## Error Handling

MessagePack functions return error codes that can be checked:

```cpp
auto ec = glz::write_msgpack(obj, buffer);
if (ec) {
   std::cerr << "Error: " << glz::format_error(ec, buffer) << std::endl;
}
```

## Limitations

- **Extension Types**: Basic support for extensions is implemented, but custom extension type handlers are not yet supported
- **Timestamp Extension**: While the format byte is defined, automatic timestamp serialization is not yet implemented

## Examples

See `examples/msgpack_example.cpp` for a complete working example demonstrating:
- Serialization of complex nested structures
- Deserialization and round-trip verification
- Size comparison with JSON
- Error handling

## Testing

The implementation includes comprehensive tests covering:
- All primitive types
- Arrays and maps
- Nested structures
- Optional and variant types
- Edge cases (empty containers, large values, unicode strings)
- Round-trip serialization

Run tests with:
```bash
ctest -R msgpack_test
```

## Architecture

The MessagePack implementation follows the same architectural patterns as BEVE:

- `include/glaze/msgpack/header.hpp`: Format constants and utility functions
- `include/glaze/msgpack/write.hpp`: Serialization implementation
- `include/glaze/msgpack/read.hpp`: Deserialization implementation
- `include/glaze/msgpack.hpp`: Main header including all components

## Contributing

The MessagePack implementation is part of Glaze and follows the same contribution guidelines. See the main Glaze documentation for details.

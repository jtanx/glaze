// Glaze Library
// For the license information refer to glaze.hpp

#include <complex>
#include <deque>
#include <list>
#include <map>
#include <numbers>
#include <set>
#include <unordered_map>
#include <vector>

#include "glaze/msgpack/read.hpp"
#include "glaze/msgpack/write.hpp"
#include "ut/ut.hpp"

using namespace ut;

// Test structures
struct my_struct
{
   int i = 287;
   double d = 3.14;
   std::string hello = "Hello World";
   std::array<uint64_t, 3> arr = {1, 2, 3};
};

template <>
struct glz::meta<my_struct>
{
   using T = my_struct;
   static constexpr auto value = object("i", &T::i, //
                                        "d", &T::d, //
                                        "hello", &T::hello, //
                                        "arr", &T::arr //
   );
};

struct nested_struct
{
   int x = 10;
   std::string name = "nested";
};

template <>
struct glz::meta<nested_struct>
{
   using T = nested_struct;
   static constexpr auto value = object("x", &T::x, //
                                        "name", &T::name //
   );
};

struct container_struct
{
   nested_struct nested;
   std::vector<int> vec = {1, 2, 3, 4, 5};
   std::map<std::string, int> map = {{"one", 1}, {"two", 2}};
};

template <>
struct glz::meta<container_struct>
{
   using T = container_struct;
   static constexpr auto value = object("nested", &T::nested, //
                                        "vec", &T::vec, //
                                        "map", &T::map //
   );
};

suite msgpack_basic_tests = [] {
   "msgpack_null"_test = [] {
      std::nullptr_t value = nullptr;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);
      expect(uint8_t(buffer[0]) == glz::msgpack::nil);

      std::nullptr_t read_value;
      expect(!glz::read_msgpack(read_value, buffer));
   };

   "msgpack_bool_true"_test = [] {
      bool value = true;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);
      expect(uint8_t(buffer[0]) == glz::msgpack::true_byte);

      bool read_value = false;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == true);
   };

   "msgpack_bool_false"_test = [] {
      bool value = false;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);
      expect(uint8_t(buffer[0]) == glz::msgpack::false_byte);

      bool read_value = true;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == false);
   };

   "msgpack_positive_fixint"_test = [] {
      uint8_t value = 42;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);
      expect(uint8_t(buffer[0]) == 42);

      uint8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 42);
   };

   "msgpack_negative_fixint"_test = [] {
      int8_t value = -5;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);

      int8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -5);
   };

   "msgpack_uint8"_test = [] {
      uint8_t value = 200;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      uint8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 200);
   };

   "msgpack_uint16"_test = [] {
      uint16_t value = 50000;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      uint16_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 50000);
   };

   "msgpack_uint32"_test = [] {
      uint32_t value = 100000;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      uint32_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 100000);
   };

   "msgpack_uint64"_test = [] {
      uint64_t value = 10000000000ULL;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      uint64_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 10000000000ULL);
   };

   "msgpack_int8"_test = [] {
      int8_t value = -100;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -100);
   };

   "msgpack_int16"_test = [] {
      int16_t value = -20000;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int16_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -20000);
   };

   "msgpack_int32"_test = [] {
      int32_t value = -1000000;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int32_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -1000000);
   };

   "msgpack_int64"_test = [] {
      int64_t value = -10000000000LL;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int64_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -10000000000LL);
   };

   "msgpack_float32"_test = [] {
      float value = 3.14f;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      float read_value = 0.0f;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(std::abs(read_value - 3.14f) < 0.001f);
   };

   "msgpack_float64"_test = [] {
      double value = 3.141592653589793;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      double read_value = 0.0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(std::abs(read_value - 3.141592653589793) < 0.000001);
   };

   "msgpack_fixstr"_test = [] {
      std::string value = "hello";
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::string read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == "hello");
   };

   "msgpack_str8"_test = [] {
      std::string value(40, 'a');
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::string read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_str16"_test = [] {
      std::string value(300, 'x');
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::string read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_empty_string"_test = [] {
      std::string value = "";
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::string read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == "");
   };
};

suite msgpack_array_tests = [] {
   "msgpack_fixarray"_test = [] {
      std::vector<int> value = {1, 2, 3, 4, 5};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::vector<int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_array16"_test = [] {
      std::vector<int> value(100, 42);
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::vector<int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_empty_array"_test = [] {
      std::vector<int> value;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::vector<int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.empty());
   };

   "msgpack_array_of_strings"_test = [] {
      std::vector<std::string> value = {"one", "two", "three"};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::vector<std::string> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_nested_arrays"_test = [] {
      std::vector<std::vector<int>> value = {{1, 2}, {3, 4}, {5, 6}};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::vector<std::vector<int>> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_std_array"_test = [] {
      std::array<int, 3> value = {10, 20, 30};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::array<int, 3> read_value = {0, 0, 0};
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_deque"_test = [] {
      std::deque<int> value = {1, 2, 3, 4};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::deque<int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };
};

suite msgpack_map_tests = [] {
   "msgpack_fixmap"_test = [] {
      std::map<std::string, int> value = {{"one", 1}, {"two", 2}, {"three", 3}};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::map<std::string, int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_empty_map"_test = [] {
      std::map<std::string, int> value;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::map<std::string, int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.empty());
   };

   "msgpack_map_with_int_keys"_test = [] {
      std::map<int, std::string> value = {{1, "one"}, {2, "two"}, {3, "three"}};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::map<int, std::string> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_unordered_map"_test = [] {
      std::unordered_map<std::string, int> value = {{"a", 1}, {"b", 2}, {"c", 3}};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::unordered_map<std::string, int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };

   "msgpack_nested_maps"_test = [] {
      std::map<std::string, std::map<std::string, int>> value = {{"outer1", {{"inner1", 1}}},
                                                                  {"outer2", {{"inner2", 2}}}};
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::map<std::string, std::map<std::string, int>> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };
};

suite msgpack_struct_tests = [] {
   "msgpack_simple_struct"_test = [] {
      my_struct value;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      my_struct read_value;
      read_value.i = 0;
      read_value.d = 0.0;
      read_value.hello = "";
      read_value.arr = {0, 0, 0};

      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.i == 287);
      expect(std::abs(read_value.d - 3.14) < 0.001);
      expect(read_value.hello == "Hello World");
      expect(read_value.arr[0] == 1);
      expect(read_value.arr[1] == 2);
      expect(read_value.arr[2] == 3);
   };

   "msgpack_nested_struct"_test = [] {
      container_struct value;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      container_struct read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.nested.x == 10);
      expect(read_value.nested.name == "nested");
      expect(read_value.vec == value.vec);
      expect(read_value.map == value.map);
   };

   "msgpack_struct_roundtrip"_test = [] {
      my_struct original;
      original.i = 42;
      original.d = 2.718;
      original.hello = "Test String";
      original.arr = {10, 20, 30};

      std::string buffer;
      expect(!glz::write_msgpack(original, buffer));

      my_struct read_value;
      expect(!glz::read_msgpack(read_value, buffer));

      expect(read_value.i == original.i);
      expect(std::abs(read_value.d - original.d) < 0.001);
      expect(read_value.hello == original.hello);
      expect(read_value.arr == original.arr);
   };
};

suite msgpack_optional_tests = [] {
   "msgpack_optional_with_value"_test = [] {
      std::optional<int> value = 42;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::optional<int> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.has_value());
      expect(read_value.value() == 42);
   };

   "msgpack_optional_null"_test = [] {
      std::optional<int> value = std::nullopt;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::optional<int> read_value = 999;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(!read_value.has_value());
   };

   "msgpack_optional_string"_test = [] {
      std::optional<std::string> value = "hello";
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::optional<std::string> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value.has_value());
      expect(read_value.value() == "hello");
   };
};

suite msgpack_variant_tests = [] {
   "msgpack_variant_int"_test = [] {
      std::variant<int, std::string, double> value = 42;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 42);
   };

   "msgpack_variant_string"_test = [] {
      std::variant<int, std::string, double> value = std::string("hello");
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::string read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == "hello");
   };

   "msgpack_variant_double"_test = [] {
      std::variant<int, std::string, double> value = 3.14;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      double read_value = 0.0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(std::abs(read_value - 3.14) < 0.001);
   };
};

suite msgpack_edge_cases = [] {
   "msgpack_max_fixint"_test = [] {
      uint8_t value = 127;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);

      uint8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 127);
   };

   "msgpack_min_negative_fixint"_test = [] {
      int8_t value = -32;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      int8_t read_value = 0;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == -32);
   };

   "msgpack_zero"_test = [] {
      int value = 0;
      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));
      expect(buffer.size() == 1);
      expect(uint8_t(buffer[0]) == 0);

      int read_value = -1;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == 0);
   };

   "msgpack_large_nested_structure"_test = [] {
      std::map<std::string, std::vector<std::map<std::string, int>>> value = {
         {"key1", {{{"a", 1}, {"b", 2}}, {{"c", 3}, {"d", 4}}}}, {"key2", {{{"e", 5}, {"f", 6}}}}};

      std::string buffer;
      expect(!glz::write_msgpack(value, buffer));

      std::map<std::string, std::vector<std::map<std::string, int>>> read_value;
      expect(!glz::read_msgpack(read_value, buffer));
      expect(read_value == value);
   };
};

int main()
{
   // Run all tests
   return 0;
}

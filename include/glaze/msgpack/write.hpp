// Glaze Library
// For the license information refer to glaze.hpp

#pragma once

#include <utility>

#include "glaze/msgpack/header.hpp"
#include "glaze/core/opts.hpp"
#include "glaze/core/reflect.hpp"
#include "glaze/core/seek.hpp"
#include "glaze/core/to.hpp"
#include "glaze/core/write.hpp"
#include "glaze/util/dump.hpp"
#include "glaze/util/for_each.hpp"
#include "glaze/util/variant.hpp"

namespace glz
{
   template <>
   struct serialize<MSGPACK>
   {
      template <auto Opts, class T, is_context Ctx, class B, class IX>
      GLZ_ALWAYS_INLINE static void op(T&& value, Ctx&& ctx, B&& b, IX&& ix)
      {
         to<MSGPACK, std::remove_cvref_t<T>>::template op<Opts>(std::forward<T>(value), std::forward<Ctx>(ctx),
                                                                 std::forward<B>(b), std::forward<IX>(ix));
      }
   };

   // Helper to dump a byte value
   GLZ_ALWAYS_INLINE void dump_msgpack_byte(uint8_t value, auto&& b, auto&& ix)
   {
      if (ix >= b.size()) [[unlikely]] {
         b.resize(2 * (ix + 1));
      }
      b[ix] = value;
      ++ix;
   }

   // Helper to dump multi-byte values in big-endian
   template <typename T>
   GLZ_ALWAYS_INLINE void dump_msgpack_value(T value, auto&& b, auto&& ix)
   {
      using namespace msgpack;
      constexpr auto n = sizeof(T);
      if (const auto k = ix + n; k > b.size()) [[unlikely]] {
         b.resize(2 * k);
      }

      // Use unsigned type for byte order conversion to avoid sign extension issues
      using U = std::conditional_t<std::is_integral_v<T>, std::make_unsigned_t<T>, T>;
      U uval;
      std::memcpy(&uval, &value, n);
      const auto big_endian_value = to_big_endian(uval);
      std::memcpy(&b[ix], &big_endian_value, n);
      ix += n;
   }

   // Null type
   template <always_null_t T>
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&&, is_context auto&&, auto&& b, auto&& ix)
      {
         dump_msgpack_byte(msgpack::nil, b, ix);
      }
   };

   // Boolean type
   template <bool_t T>
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(const bool value, is_context auto&&, auto&& b, auto&& ix)
      {
         dump_msgpack_byte(value ? msgpack::true_byte : msgpack::false_byte, b, ix);
      }
   };

   // Integer types
   template <class T>
      requires(std::integral<T> && !char_t<T> && !bool_t<T>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&&, auto&& b, auto&& ix)
      {
         using namespace msgpack;
         using V = std::decay_t<decltype(value)>;

         if constexpr (std::is_unsigned_v<V>) {
            // Unsigned integer encoding
            if (value <= 0x7f) {
               // Positive fixint
               dump_msgpack_byte(static_cast<uint8_t>(value), b, ix);
            }
            else if (value <= std::numeric_limits<uint8_t>::max()) {
               dump_msgpack_byte(uint8, b, ix);
               dump_msgpack_byte(static_cast<uint8_t>(value), b, ix);
            }
            else if (value <= std::numeric_limits<uint16_t>::max()) {
               dump_msgpack_byte(uint16, b, ix);
               dump_msgpack_value(static_cast<uint16_t>(value), b, ix);
            }
            else if (value <= std::numeric_limits<uint32_t>::max()) {
               dump_msgpack_byte(uint32, b, ix);
               dump_msgpack_value(static_cast<uint32_t>(value), b, ix);
            }
            else {
               dump_msgpack_byte(uint64, b, ix);
               dump_msgpack_value(static_cast<uint64_t>(value), b, ix);
            }
         }
         else {
            // Signed integer encoding
            const auto val = static_cast<int64_t>(value);
            if (val >= -32 && val <= 127) {
               // FixInt range
               if (val >= 0) {
                  dump_msgpack_byte(static_cast<uint8_t>(val), b, ix);
               }
               else {
                  dump_msgpack_byte(static_cast<uint8_t>(val), b, ix); // Negative fixint
               }
            }
            else if (val >= std::numeric_limits<int8_t>::min() && val <= std::numeric_limits<int8_t>::max()) {
               dump_msgpack_byte(int8, b, ix);
               dump_msgpack_byte(static_cast<uint8_t>(static_cast<int8_t>(val)), b, ix);
            }
            else if (val >= std::numeric_limits<int16_t>::min() && val <= std::numeric_limits<int16_t>::max()) {
               dump_msgpack_byte(int16, b, ix);
               dump_msgpack_value(static_cast<int16_t>(val), b, ix);
            }
            else if (val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max()) {
               dump_msgpack_byte(int32, b, ix);
               dump_msgpack_value(static_cast<int32_t>(val), b, ix);
            }
            else {
               dump_msgpack_byte(int64, b, ix);
               dump_msgpack_value(val, b, ix);
            }
         }
      }
   };

   // Floating-point types
   template <class T>
      requires(std::floating_point<T>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&&, auto&& b, auto&& ix)
      {
         using namespace msgpack;
         using V = std::decay_t<decltype(value)>;

         if constexpr (sizeof(V) == 4) {
            dump_msgpack_byte(float32, b, ix);
            uint32_t bits;
            std::memcpy(&bits, &value, 4);
            dump_msgpack_value(bits, b, ix);
         }
         else {
            dump_msgpack_byte(float64, b, ix);
            uint64_t bits;
            std::memcpy(&bits, &value, 8);
            dump_msgpack_value(bits, b, ix);
         }
      }
   };

   // String types
   template <class T>
      requires str_t<T> || char_array_t<T>
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&&, auto&& b, auto&& ix)
      {
         using namespace msgpack;

         const auto n = [&]() -> size_t {
            if constexpr (char_array_t<T>) {
               return std::char_traits<char>::length(value);
            }
            else {
               return value.size();
            }
         }();

         // Write string header
         if (n <= 31) {
            dump_msgpack_byte(fixstr_min | static_cast<uint8_t>(n), b, ix);
         }
         else if (n <= std::numeric_limits<uint8_t>::max()) {
            dump_msgpack_byte(str8, b, ix);
            dump_msgpack_byte(static_cast<uint8_t>(n), b, ix);
         }
         else if (n <= std::numeric_limits<uint16_t>::max()) {
            dump_msgpack_byte(str16, b, ix);
            dump_msgpack_value(static_cast<uint16_t>(n), b, ix);
         }
         else {
            dump_msgpack_byte(str32, b, ix);
            dump_msgpack_value(static_cast<uint32_t>(n), b, ix);
         }

         // Write string data
         if (const auto k = ix + n; k > b.size()) [[unlikely]] {
            b.resize(2 * k);
         }

         if constexpr (char_array_t<T>) {
            std::memcpy(&b[ix], value, n);
         }
         else {
            std::memcpy(&b[ix], value.data(), n);
         }
         ix += n;
      }
   };

   // Array types
   template <class T>
      requires(array_t<T> && !char_array_t<T>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& b, auto&& ix)
      {
         using namespace msgpack;

         const auto n = value.size();

         // Write array header
         if (n <= 15) {
            dump_msgpack_byte(fixarray_min | static_cast<uint8_t>(n), b, ix);
         }
         else if (n <= std::numeric_limits<uint16_t>::max()) {
            dump_msgpack_byte(array16, b, ix);
            dump_msgpack_value(static_cast<uint16_t>(n), b, ix);
         }
         else {
            dump_msgpack_byte(array32, b, ix);
            dump_msgpack_value(static_cast<uint32_t>(n), b, ix);
         }

         // Write array elements
         for (auto&& element : value) {
            serialize<MSGPACK>::op<Opts>(element, ctx, b, ix);
         }
      }
   };

   // Map types
   template <class T>
      requires(writable_map_t<T>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& b, auto&& ix)
      {
         using namespace msgpack;

         const auto n = value.size();

         // Write map header
         if (n <= 15) {
            dump_msgpack_byte(fixmap_min | static_cast<uint8_t>(n), b, ix);
         }
         else if (n <= std::numeric_limits<uint16_t>::max()) {
            dump_msgpack_byte(map16, b, ix);
            dump_msgpack_value(static_cast<uint16_t>(n), b, ix);
         }
         else {
            dump_msgpack_byte(map32, b, ix);
            dump_msgpack_value(static_cast<uint32_t>(n), b, ix);
         }

         // Write map entries
         for (auto&& [key, val] : value) {
            serialize<MSGPACK>::op<Opts>(key, ctx, b, ix);
            serialize<MSGPACK>::op<Opts>(val, ctx, b, ix);
         }
      }
   };

   // Object types (structs with glaze metadata)
   template <class T>
      requires(glaze_object_t<T> || reflectable<T>)
   struct to<MSGPACK, T>
   {
      static constexpr auto N = reflect<T>::size;

      template <auto Opts, size_t I>
      static consteval bool should_skip_field()
      {
         using V = field_t<T, I>;

         if constexpr (std::same_as<V, hidden> || std::same_as<V, skip>) {
            return true;
         }
         else if constexpr (is_member_function_pointer<V>) {
            return !check_write_member_functions(Opts);
         }
         else {
            return false;
         }
      }

      template <auto Opts>
      static consteval size_t count_to_write()
      {
         return []<size_t... I>(std::index_sequence<I...>) {
            return (size_t{} + ... + (should_skip_field<Opts, I>() ? size_t{} : size_t{1}));
         }(std::make_index_sequence<N>{});
      }

      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& b, auto&& ix)
      {
         using namespace msgpack;

         constexpr auto num_fields = count_to_write<Opts>();

         // Write map header
         if constexpr (num_fields <= 15) {
            dump_msgpack_byte(fixmap_min | static_cast<uint8_t>(num_fields), b, ix);
         }
         else if constexpr (num_fields <= std::numeric_limits<uint16_t>::max()) {
            dump_msgpack_byte(map16, b, ix);
            dump_msgpack_value(static_cast<uint16_t>(num_fields), b, ix);
         }
         else {
            dump_msgpack_byte(map32, b, ix);
            dump_msgpack_value(static_cast<uint32_t>(num_fields), b, ix);
         }

         // Write fields
         for_each<N>([&]<size_t I>() {
            if constexpr (should_skip_field<Opts, I>()) {
               return;
            }
            else {
               // Write key
               static constexpr auto key = get<I>(reflect<T>::keys);
               serialize<MSGPACK>::op<Opts>(key, ctx, b, ix);

               // Write value
               if constexpr (reflectable<T>) {
                  serialize<MSGPACK>::op<Opts>(get_member(value, get<I>(to_tie(value))), ctx, b, ix);
               }
               else {
                  serialize<MSGPACK>::op<Opts>(get_member(value, get<I>(reflect<T>::values)), ctx, b, ix);
               }
            }
         });
      }
   };

   // Optional types
   template <class T>
      requires(is_specialization_v<T, std::optional>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& b, auto&& ix)
      {
         if (value) {
            serialize<MSGPACK>::op<Opts>(*value, ctx, b, ix);
         }
         else {
            dump_msgpack_byte(msgpack::nil, b, ix);
         }
      }
   };

   // Variant types
   template <class T>
      requires(is_specialization_v<T, std::variant>)
   struct to<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& b, auto&& ix)
      {
         std::visit(
            [&](auto&& v) {
               serialize<MSGPACK>::op<Opts>(v, ctx, b, ix);
            },
            value);
      }
   };

   // Glaze value wrapper
   template <class T>
      requires(glaze_value_t<T> && !custom_write<T>)
   struct to<MSGPACK, T>
   {
      template <auto Opts, class Value, is_context Ctx, class B, class IX>
      GLZ_ALWAYS_INLINE static void op(Value&& value, Ctx&& ctx, B&& b, IX&& ix)
      {
         using V = std::remove_cvref_t<decltype(get_member(std::declval<Value>(), meta_wrapper_v<T>))>;
         to<MSGPACK, V>::template op<Opts>(get_member(std::forward<Value>(value), meta_wrapper_v<T>),
                                           std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
      }
   };

   // Public API functions
   template <write_supported<MSGPACK> T, class Buffer>
   [[nodiscard]] error_ctx write_msgpack(T&& value, Buffer&& buffer)
   {
      return write<opts{.format = MSGPACK}>(std::forward<T>(value), std::forward<Buffer>(buffer));
   }

   template <auto Opts = opts{}, write_supported<MSGPACK> T>
   [[nodiscard]] glz::expected<std::string, error_ctx> write_msgpack(T&& value)
   {
      auto ret_opts = Opts;
      ret_opts.format = MSGPACK;
      return write<ret_opts>(std::forward<T>(value));
   }
}

// Glaze Library
// For the license information refer to glaze.hpp

#pragma once

#include "glaze/msgpack/header.hpp"
#include "glaze/core/opts.hpp"
#include "glaze/core/read.hpp"
#include "glaze/core/reflect.hpp"
#include "glaze/util/dump.hpp"

namespace glz
{
   template <>
   struct parse<MSGPACK>
   {
      template <auto Opts, class T, is_context Ctx, class It0, class It1>
      GLZ_ALWAYS_INLINE static void op(T&& value, Ctx&& ctx, It0&& it, It1&& end)
      {
         if constexpr (const_value_v<T>) {
            if constexpr (Opts.error_on_const_read) {
               ctx.error = error_code::attempt_const_read;
            }
            else {
               // Skip value
               skip_value_msgpack(std::forward<Ctx>(ctx), std::forward<It0>(it), std::forward<It1>(end));
            }
         }
         else {
            using V = std::remove_cvref_t<T>;
            from<MSGPACK, V>::template op<Opts>(std::forward<T>(value), std::forward<Ctx>(ctx),
                                                 std::forward<It0>(it), std::forward<It1>(end));
         }
      }
   };

   // Helper to skip a MessagePack value
   inline void skip_value_msgpack(is_context auto&& ctx, auto&& it, auto&& end) noexcept
   {
      using namespace msgpack;

      if (invalid_end(ctx, it, end)) {
         return;
      }

      const uint8_t type_byte = static_cast<uint8_t>(*it);
      ++it;

      // Positive fixint
      if (type_byte <= positive_fixint_max) {
         return;
      }
      // Negative fixint
      else if (type_byte >= negative_fixint_min) {
         return;
      }
      // FixMap
      else if (type_byte >= fixmap_min && type_byte <= fixmap_max) {
         const size_t n = type_byte & 0x0f;
         for (size_t i = 0; i < n * 2; ++i) {
            skip_value_msgpack(ctx, it, end);
            if (bool(ctx.error)) return;
         }
      }
      // FixArray
      else if (type_byte >= fixarray_min && type_byte <= fixarray_max) {
         const size_t n = type_byte & 0x0f;
         for (size_t i = 0; i < n; ++i) {
            skip_value_msgpack(ctx, it, end);
            if (bool(ctx.error)) return;
         }
      }
      // FixStr
      else if (type_byte >= fixstr_min && type_byte <= fixstr_max) {
         const size_t n = type_byte & 0x1f;
         it += n;
         if (it > end) {
            ctx.error = error_code::unexpected_end;
         }
      }
      else {
         switch (type_byte) {
         case nil:
         case false_byte:
         case true_byte:
            return;
         case uint8:
         case int8:
            it += 1;
            break;
         case uint16:
         case int16:
            it += 2;
            break;
         case uint32:
         case int32:
         case float32:
            it += 4;
            break;
         case uint64:
         case int64:
         case float64:
            it += 8;
            break;
         case str8:
         case bin8: {
            if (invalid_end(ctx, it, end)) return;
            uint8_t len;
            std::memcpy(&len, it, 1);
            it += 1 + len;
            break;
         }
         case str16:
         case bin16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t len;
            std::memcpy(&len, it, 2);
            len = from_big_endian(len);
            it += 2 + len;
            break;
         }
         case str32:
         case bin32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t len;
            std::memcpy(&len, it, 4);
            len = from_big_endian(len);
            it += 4 + len;
            break;
         }
         case array16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t n;
            std::memcpy(&n, it, 2);
            n = from_big_endian(n);
            it += 2;
            for (size_t i = 0; i < n; ++i) {
               skip_value_msgpack(ctx, it, end);
               if (bool(ctx.error)) return;
            }
            break;
         }
         case array32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t n;
            std::memcpy(&n, it, 4);
            n = from_big_endian(n);
            it += 4;
            for (size_t i = 0; i < n; ++i) {
               skip_value_msgpack(ctx, it, end);
               if (bool(ctx.error)) return;
            }
            break;
         }
         case map16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t n;
            std::memcpy(&n, it, 2);
            n = from_big_endian(n);
            it += 2;
            for (size_t i = 0; i < n * 2; ++i) {
               skip_value_msgpack(ctx, it, end);
               if (bool(ctx.error)) return;
            }
            break;
         }
         case map32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t n;
            std::memcpy(&n, it, 4);
            n = from_big_endian(n);
            it += 4;
            for (size_t i = 0; i < n * 2; ++i) {
               skip_value_msgpack(ctx, it, end);
               if (bool(ctx.error)) return;
            }
            break;
         }
         case fixext1:
            it += 1 + 1; // type + data
            break;
         case fixext2:
            it += 1 + 2;
            break;
         case fixext4:
            it += 1 + 4;
            break;
         case fixext8:
            it += 1 + 8;
            break;
         case fixext16:
            it += 1 + 16;
            break;
         case ext8: {
            if (invalid_end(ctx, it, end)) return;
            uint8_t len;
            std::memcpy(&len, it, 1);
            it += 1 + 1 + len; // length + type + data
            break;
         }
         case ext16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t len;
            std::memcpy(&len, it, 2);
            len = from_big_endian(len);
            it += 2 + 1 + len;
            break;
         }
         case ext32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t len;
            std::memcpy(&len, it, 4);
            len = from_big_endian(len);
            it += 4 + 1 + len;
            break;
         }
         default:
            ctx.error = error_code::syntax_error;
            return;
         }

         if (it > end) {
            ctx.error = error_code::unexpected_end;
         }
      }
   }

   // Null type
   template <always_null_t T>
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&&, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         if (invalid_end(ctx, it, end)) {
            return;
         }
         if (uint8_t(*it) != msgpack::nil) [[unlikely]] {
            ctx.error = error_code::syntax_error;
            return;
         }
         ++it;
      }
   };

   // Boolean type
   template <bool_t T>
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         if (invalid_end(ctx, it, end)) {
            return;
         }
         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         if (type_byte == msgpack::true_byte) {
            value = true;
         }
         else if (type_byte == msgpack::false_byte) {
            value = false;
         }
         else {
            ctx.error = error_code::syntax_error;
         }
      }
   };

   // Integer types
   template <class T>
      requires(std::integral<T> && !char_t<T> && !bool_t<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         using V = std::decay_t<decltype(value)>;

         // Positive fixint
         if (type_byte <= positive_fixint_max) {
            value = static_cast<V>(type_byte);
            return;
         }
         // Negative fixint
         else if (type_byte >= negative_fixint_min) {
            value = static_cast<V>(static_cast<int8_t>(type_byte));
            return;
         }

         switch (type_byte) {
         case uint8: {
            if (invalid_end(ctx, it, end)) return;
            uint8_t val;
            std::memcpy(&val, it, 1);
            it += 1;
            value = static_cast<V>(val);
            break;
         }
         case uint16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t val;
            std::memcpy(&val, it, 2);
            val = from_big_endian(val);
            it += 2;
            value = static_cast<V>(val);
            break;
         }
         case uint32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t val;
            std::memcpy(&val, it, 4);
            val = from_big_endian(val);
            it += 4;
            value = static_cast<V>(val);
            break;
         }
         case uint64: {
            if (it + 8 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint64_t val;
            std::memcpy(&val, it, 8);
            val = from_big_endian(val);
            it += 8;
            value = static_cast<V>(val);
            break;
         }
         case int8: {
            if (invalid_end(ctx, it, end)) return;
            int8_t val;
            std::memcpy(&val, it, 1);
            it += 1;
            value = static_cast<V>(val);
            break;
         }
         case int16: {
            if (it + 2 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint16_t uval;
            std::memcpy(&uval, it, 2);
            uval = from_big_endian(uval);
            int16_t val;
            std::memcpy(&val, &uval, 2);
            it += 2;
            value = static_cast<V>(val);
            break;
         }
         case int32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t uval;
            std::memcpy(&uval, it, 4);
            uval = from_big_endian(uval);
            int32_t val;
            std::memcpy(&val, &uval, 4);
            it += 4;
            value = static_cast<V>(val);
            break;
         }
         case int64: {
            if (it + 8 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint64_t uval;
            std::memcpy(&uval, it, 8);
            uval = from_big_endian(uval);
            int64_t val;
            std::memcpy(&val, &uval, 8);
            it += 8;
            value = static_cast<V>(val);
            break;
         }
         default:
            ctx.error = error_code::syntax_error;
            return;
         }
      }
   };

   // Floating-point types
   template <class T>
      requires(std::floating_point<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         using V = std::decay_t<decltype(value)>;

         switch (type_byte) {
         case float32: {
            if (it + 4 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint32_t bits;
            std::memcpy(&bits, it, 4);
            bits = from_big_endian(bits);
            float f;
            std::memcpy(&f, &bits, 4);
            value = static_cast<V>(f);
            it += 4;
            break;
         }
         case float64: {
            if (it + 8 > end) {
               ctx.error = error_code::unexpected_end;
               return;
            }
            uint64_t bits;
            std::memcpy(&bits, it, 8);
            bits = from_big_endian(bits);
            double d;
            std::memcpy(&d, &bits, 8);
            value = static_cast<V>(d);
            it += 8;
            break;
         }
         default:
            ctx.error = error_code::syntax_error;
            return;
         }
      }
   };

   // String types
   template <class T>
      requires(str_t<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         size_t length = 0;

         // FixStr
         if (type_byte >= fixstr_min && type_byte <= fixstr_max) {
            length = type_byte & 0x1f;
         }
         else {
            switch (type_byte) {
            case str8: {
               if (invalid_end(ctx, it, end)) return;
               uint8_t len;
               std::memcpy(&len, it, 1);
               length = len;
               it += 1;
               break;
            }
            case str16: {
               if (it + 2 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint16_t len;
               std::memcpy(&len, it, 2);
               length = from_big_endian(len);
               it += 2;
               break;
            }
            case str32: {
               if (it + 4 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint32_t len;
               std::memcpy(&len, it, 4);
               length = from_big_endian(len);
               it += 4;
               break;
            }
            default:
               ctx.error = error_code::syntax_error;
               return;
            }
         }

         if (it + length > end) {
            ctx.error = error_code::unexpected_end;
            return;
         }

         if constexpr (requires { value.resize(length); }) {
            value.resize(length);
            std::memcpy(value.data(), it, length);
         }
         else if constexpr (requires { value.assign(it, it + length); }) {
            value.assign(it, it + length);
         }
         else {
            value = T(it, it + length);
         }

         it += length;
      }
   };

   // Array types
   template <class T>
      requires(array_t<T> && !char_array_t<T> && !str_t<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         size_t length = 0;

         // FixArray
         if (type_byte >= fixarray_min && type_byte <= fixarray_max) {
            length = type_byte & 0x0f;
         }
         else {
            switch (type_byte) {
            case array16: {
               if (it + 2 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint16_t len;
               std::memcpy(&len, it, 2);
               length = from_big_endian(len);
               it += 2;
               break;
            }
            case array32: {
               if (it + 4 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint32_t len;
               std::memcpy(&len, it, 4);
               length = from_big_endian(len);
               it += 4;
               break;
            }
            default:
               ctx.error = error_code::syntax_error;
               return;
            }
         }

         // Read elements
         if constexpr (emplace_backable<T>) {
            if constexpr (requires { value.clear(); }) {
               value.clear();
            }
            for (size_t i = 0; i < length; ++i) {
               using V = typename T::value_type;
               V element{};
               parse<MSGPACK>::op<Opts>(element, ctx, it, end);
               if (bool(ctx.error)) return;
               value.emplace_back(std::move(element));
            }
         }
         else if constexpr (accessible<T>) {
            if constexpr (requires { value.resize(length); }) {
               value.resize(length);
            }
            for (size_t i = 0; i < length && i < value.size(); ++i) {
               parse<MSGPACK>::op<Opts>(value[i], ctx, it, end);
               if (bool(ctx.error)) return;
            }
         }
         else {
            ctx.error = error_code::syntax_error;
         }
      }
   };

   // Map types
   template <class T>
      requires(readable_map_t<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         size_t length = 0;

         // FixMap
         if (type_byte >= fixmap_min && type_byte <= fixmap_max) {
            length = type_byte & 0x0f;
         }
         else {
            switch (type_byte) {
            case map16: {
               if (it + 2 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint16_t len;
               std::memcpy(&len, it, 2);
               length = from_big_endian(len);
               it += 2;
               break;
            }
            case map32: {
               if (it + 4 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint32_t len;
               std::memcpy(&len, it, 4);
               length = from_big_endian(len);
               it += 4;
               break;
            }
            default:
               ctx.error = error_code::syntax_error;
               return;
            }
         }

         value.clear();

         for (size_t i = 0; i < length; ++i) {
            using Key = typename T::key_type;
            using Value = typename T::mapped_type;

            Key k{};
            parse<MSGPACK>::op<Opts>(k, ctx, it, end);
            if (bool(ctx.error)) return;

            Value v{};
            parse<MSGPACK>::op<Opts>(v, ctx, it, end);
            if (bool(ctx.error)) return;

            value.emplace(std::move(k), std::move(v));
         }
      }
   };

   // Object types (structs with glaze metadata)
   template <class T>
      requires(glaze_object_t<T> || reflectable<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         using namespace msgpack;

         if (invalid_end(ctx, it, end)) {
            return;
         }

         const uint8_t type_byte = static_cast<uint8_t>(*it);
         ++it;

         size_t length = 0;

         // FixMap
         if (type_byte >= fixmap_min && type_byte <= fixmap_max) {
            length = type_byte & 0x0f;
         }
         else {
            switch (type_byte) {
            case map16: {
               if (it + 2 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint16_t len;
               std::memcpy(&len, it, 2);
               length = from_big_endian(len);
               it += 2;
               break;
            }
            case map32: {
               if (it + 4 > end) {
                  ctx.error = error_code::unexpected_end;
                  return;
               }
               uint32_t len;
               std::memcpy(&len, it, 4);
               length = from_big_endian(len);
               it += 4;
               break;
            }
            default:
               ctx.error = error_code::syntax_error;
               return;
            }
         }

         // Read key-value pairs
         for (size_t i = 0; i < length; ++i) {
            // Read key as string
            std::string key;
            parse<MSGPACK>::op<Opts>(key, ctx, it, end);
            if (bool(ctx.error)) return;

            // Find matching field and deserialize value
            static constexpr auto N = reflect<T>::size;
            bool found = false;

            for_each<N>([&]<size_t I>() {
               if (found || bool(ctx.error)) return;

               static constexpr auto field_key = get<I>(reflect<T>::keys);
               if (key == field_key) {
                  found = true;
                  if constexpr (reflectable<T>) {
                     parse<MSGPACK>::op<Opts>(get_member(value, get<I>(to_tie(value))), ctx, it, end);
                  }
                  else {
                     parse<MSGPACK>::op<Opts>(get_member(value, get<I>(reflect<T>::values)), ctx, it, end);
                  }
               }
            });

            if (!found) {
               // Skip unknown field
               skip_value_msgpack(ctx, it, end);
            }
         }
      }
   };

   // Optional types
   template <class T>
      requires(is_specialization_v<T, std::optional>)
   struct from<MSGPACK, T>
   {
      template <auto Opts>
      GLZ_ALWAYS_INLINE static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) noexcept
      {
         if (invalid_end(ctx, it, end)) {
            return;
         }

         // Peek at type byte
         const uint8_t type_byte = static_cast<uint8_t>(*it);

         if (type_byte == msgpack::nil) {
            ++it;
            value = std::nullopt;
         }
         else {
            using V = typename T::value_type;
            V val{};
            parse<MSGPACK>::op<Opts>(val, ctx, it, end);
            if (!bool(ctx.error)) {
               value = std::move(val);
            }
         }
      }
   };

   // Glaze value wrapper
   template <class T>
      requires(glaze_value_t<T> && !custom_read<T>)
   struct from<MSGPACK, T>
   {
      template <auto Opts, class Value, is_context Ctx, class It0, class It1>
      GLZ_ALWAYS_INLINE static void op(Value&& value, Ctx&& ctx, It0&& it, It1&& end)
      {
         using V = std::decay_t<decltype(get_member(std::declval<Value>(), meta_wrapper_v<T>))>;
         from<MSGPACK, V>::template op<Opts>(get_member(std::forward<Value>(value), meta_wrapper_v<T>),
                                              std::forward<Ctx>(ctx), std::forward<It0>(it), std::forward<It1>(end));
      }
   };

   // Public API functions
   template <read_supported<MSGPACK> T, class Buffer>
   [[nodiscard]] inline error_ctx read_msgpack(T&& value, Buffer&& buffer)
   {
      return read<opts{.format = MSGPACK}>(value, std::forward<Buffer>(buffer));
   }

   template <read_supported<MSGPACK> T, class Buffer>
   [[nodiscard]] inline expected<T, error_ctx> read_msgpack(Buffer&& buffer)
   {
      T value{};
      const auto pe = read<opts{.format = MSGPACK}>(value, std::forward<Buffer>(buffer));
      if (pe) [[unlikely]] {
         return unexpected(pe);
      }
      return value;
   }
}

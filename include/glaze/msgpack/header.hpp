// Glaze Library
// For the license information refer to glaze.hpp

#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "glaze/core/context.hpp"
#include "glaze/util/inline.hpp"

namespace glz::msgpack
{
   // MessagePack format type bytes
   // Positive FixInt: 0x00 - 0x7f
   constexpr uint8_t positive_fixint_min = 0x00;
   constexpr uint8_t positive_fixint_max = 0x7f;

   // FixMap: 0x80 - 0x8f
   constexpr uint8_t fixmap_min = 0x80;
   constexpr uint8_t fixmap_max = 0x8f;

   // FixArray: 0x90 - 0x9f
   constexpr uint8_t fixarray_min = 0x90;
   constexpr uint8_t fixarray_max = 0x9f;

   // FixStr: 0xa0 - 0xbf
   constexpr uint8_t fixstr_min = 0xa0;
   constexpr uint8_t fixstr_max = 0xbf;

   // Nil and Boolean
   constexpr uint8_t nil = 0xc0;
   constexpr uint8_t never_used = 0xc1;
   constexpr uint8_t false_byte = 0xc2;
   constexpr uint8_t true_byte = 0xc3;

   // Binary
   constexpr uint8_t bin8 = 0xc4;
   constexpr uint8_t bin16 = 0xc5;
   constexpr uint8_t bin32 = 0xc6;

   // Extension
   constexpr uint8_t ext8 = 0xc7;
   constexpr uint8_t ext16 = 0xc8;
   constexpr uint8_t ext32 = 0xc9;

   // Float
   constexpr uint8_t float32 = 0xca;
   constexpr uint8_t float64 = 0xcb;

   // Unsigned Integer
   constexpr uint8_t uint8 = 0xcc;
   constexpr uint8_t uint16 = 0xcd;
   constexpr uint8_t uint32 = 0xce;
   constexpr uint8_t uint64 = 0xcf;

   // Signed Integer
   constexpr uint8_t int8 = 0xd0;
   constexpr uint8_t int16 = 0xd1;
   constexpr uint8_t int32 = 0xd2;
   constexpr uint8_t int64 = 0xd3;

   // Fixed Extension
   constexpr uint8_t fixext1 = 0xd4;
   constexpr uint8_t fixext2 = 0xd5;
   constexpr uint8_t fixext4 = 0xd6;
   constexpr uint8_t fixext8 = 0xd7;
   constexpr uint8_t fixext16 = 0xd8;

   // String
   constexpr uint8_t str8 = 0xd9;
   constexpr uint8_t str16 = 0xda;
   constexpr uint8_t str32 = 0xdb;

   // Array
   constexpr uint8_t array16 = 0xdc;
   constexpr uint8_t array32 = 0xdd;

   // Map
   constexpr uint8_t map16 = 0xde;
   constexpr uint8_t map32 = 0xdf;

   // Negative FixInt: 0xe0 - 0xff (-32 to -1)
   constexpr uint8_t negative_fixint_min = 0xe0;
   constexpr uint8_t negative_fixint_max = 0xff;

   // Extension type for timestamp
   constexpr int8_t timestamp_ext_type = -1;

   // Helper functions for byte order conversion (MessagePack uses big-endian)
   template <typename T>
   GLZ_ALWAYS_INLINE constexpr T to_big_endian(T value) noexcept
   {
      if constexpr (std::endian::native == std::endian::big) {
         return value;
      }
      else {
         if constexpr (sizeof(T) == 1) {
            return value;
         }
         else if constexpr (sizeof(T) == 2) {
            return static_cast<T>((value >> 8) | (value << 8));
         }
         else if constexpr (sizeof(T) == 4) {
            return static_cast<T>(((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) |
                                  ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000));
         }
         else if constexpr (sizeof(T) == 8) {
            return static_cast<T>(((value >> 56) & 0x00000000000000FFULL) | ((value >> 40) & 0x000000000000FF00ULL) |
                                  ((value >> 24) & 0x0000000000FF0000ULL) | ((value >> 8) & 0x00000000FF000000ULL) |
                                  ((value << 8) & 0x000000FF00000000ULL) | ((value << 24) & 0x0000FF0000000000ULL) |
                                  ((value << 40) & 0x00FF000000000000ULL) | ((value << 56) & 0xFF00000000000000ULL));
         }
      }
   }

   template <typename T>
   GLZ_ALWAYS_INLINE constexpr T from_big_endian(T value) noexcept
   {
      return to_big_endian(value); // conversion is symmetric
   }
}

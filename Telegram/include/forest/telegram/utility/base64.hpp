#pragma once
#include <algorithm>
#include <iterator>
#include <span>
#include <string_view>

namespace forest::telegram::internal
{
  struct base64
  {
    static constexpr auto alphabet =
      std::string_view ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    static constexpr auto translate = [] {
      auto array = std::array<int, 256> ();
      std::ranges::fill (array, -1);
      for (int i = 0; i < 64; ++i)
        array[alphabet[i]] = i;
      return array;
    }();

    [[nodiscard]] constexpr bool valid (int x) const
    {
      return translate[x] != -1;
    }

    template<class F>
    constexpr void foreach_sextet (std::span<std::byte const> bytes, F callback) const
    {
      unsigned curr = 0;
      unsigned index = 0;

      for (std::byte b : bytes) {
        curr = (curr << 8) | static_cast<unsigned> (b);
        index += 8;

        while (index >= 6) {
          callback (static_cast<std::byte> ((curr >> (index - 6)) & 0x3F));
          index -= 6;
        }
      }

      if (index > 0)
        callback (static_cast<std::byte> ((curr << (6 - index)) & 0x3F));
    }

    template<class F>
    constexpr void foreach_octet (std::string_view base64, F callback) const
    {
      unsigned curr = 0;
      unsigned index = 0;
      for (int b : base64) {
        if (translate[b] == -1)
          return;
        curr = (curr << 6) | translate[b];
        index += 6;
        while (index >= 8) {
          callback (static_cast<std::byte> ((curr >> (index - 8)) & 0xFF));
          index -= 8;
        }
      }
    }

    template<std::output_iterator<char> It>
    constexpr It encode (std::span<std::byte const> bytes, It base64) const
    {
      int len = 0;
      foreach_sextet (bytes, [&] (std::byte curr) {
        *base64++ = alphabet[static_cast<int> (curr)];
        ++len;
      });
      while (len % 4) {
        *base64++ = '=';
        ++len;
      }
      return base64;
    }

    template<std::ranges::contiguous_range Span>
    [[nodiscard]] std::string encode (Span&& span) const
    {
      auto result = std::string ();
      encode (std::as_bytes (std::span (std::as_const (span))), back_inserter (result));
      return result;
    }

    template<std::output_iterator<std::byte> It>
    constexpr It decode (std::string_view base64, It bytes) const
    {
      foreach_octet (base64, [&] (std::byte curr) {
        *bytes++ = curr;
      });
      return bytes;
    }

    template<std::output_iterator<char> It>
      requires (!std::output_iterator<It, std::byte>)
    constexpr It decode (std::string_view base64, It bytes) const
    {
      foreach_octet (base64, [&] (std::byte curr) {
        *bytes++ = static_cast<char> (curr);
      });
      return bytes;
    }

    [[nodiscard]] std::string decode (std::string_view base64) const
    {
      auto result = std::string ();
      decode (base64, back_inserter (result));
      return result;
    }
  };

  constexpr inline struct
  {
    template<class Span>
    [[nodiscard]] std::string operator() (Span&& bytes) const
    {
      return base64 {}.encode (bytes);
    }
  } base64_encode {};

  constexpr inline struct
  {
    [[nodiscard]] std::string operator() (std::string_view encoded) const
    {
      return base64 {}.decode (encoded);
    }
  } base64_decode {};
} // namespace forest::telegram::internal
#pragma once
#include <string_view>
#include <type_traits>

namespace forest::container
{
  template<long Length>
  struct literal_string
  {
    static_assert (Length > 0);

    using size_type = long;
    using difference_type = long;
    using value_type = char;
    using reference = char&;
    using const_reference = char const&;
    using pointer = char*;
    using const_pointer = char const*;
    using iterator = char*;
    using const_iterator = char const*;
    using string_view = std::string_view;

    char _data[Length] {};

    constexpr literal_string () = default;

    template<size_type OtherLength>
      requires (Length >= OtherLength)
    constexpr literal_string (char const (&str)[OtherLength]) noexcept
      : literal_string (string_view (str))
    {}

    constexpr literal_string (char const* str) noexcept
    {
      for (long i = 0; str[i]; ++i)
        _data[i] = str[i];
    }

    constexpr literal_string (std::string_view str) noexcept
      : _data {}
    {
      for (size_type i = 0; i < str.length (); ++i)
        _data[i] = str[i];
    }

    // ---

    [[nodiscard]] constexpr operator string_view () const noexcept
    {
      return string_view (_data, size ());
    }

    [[nodiscard]] constexpr string_view view () const noexcept
    {
      return string_view (_data, size ());
    }

    // ---

    [[nodiscard]] constexpr size_type size () const noexcept
    {
      return Length - (_data[Length - 1] == 0);
    }

    // ---

    [[nodiscard]] constexpr reference operator[] (size_type i) & noexcept
    {
      return _data[i];
    }

    [[nodiscard]] constexpr const_reference operator[] (size_type i) const& noexcept
    {
      return _data[i];
    }

    // ---

    [[nodiscard]] bool operator== (literal_string const&) const = default;
    [[nodiscard]] auto operator<=> (literal_string const&) const = default;
  };

  template<size_t Length>
  literal_string (char const (&)[Length]) -> literal_string<Length>;
} // namespace forest::container

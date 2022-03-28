#pragma once
#include <forest/internal/container/vector.hpp>

namespace forest::internal
{
  template<std::regular Key, std::semiregular Element, long Size>
  class literal_map
  {
  public:
    using key_type = Key;
    using element_type = Element;
    struct value_type;
    using pointer = value_type*;
    using reference = value_type&;
    using size_type = long;
    using iterator = typename vector<value_type>::iterator;
    using const_iterator = typename vector<value_type>::const_iterator;

    struct value_type
    {
      key_type key;
      value_type value;

      template<long I>
        requires (I >= 0 && I < 2)
      [[nodiscard]] friend constexpr auto& get (value_type const& self) noexcept
      {
        if constexpr (I == 0)
          return self.key;
        else
          return self.value;
      }

      template<long I>
        requires (I >= 0 && I < 2)
      [[nodiscard]] friend constexpr auto& get (value_type& self) noexcept
      {
        if constexpr (I == 0)
          return self.key;
        else
          return self.value;
      }

      bool operator== (value_type const&) const = default;
      auto operator<=> (value_type const&) const = default;
    };

    // ---

    std::array<value_type, Size> _elements {};
    long _size {};

    literal_map () = default;

    constexpr literal_map (std::input_iterator auto first, std::input_iterator auto last) noexcept
    {
      for (; first != last; ++first)
        emplace (get<0> (*first), get<1> (*last));
    }

    template<class T>
      requires (!std::same_as<std::remove_cvref_t<T>, literal_map> && std::ranges::input_range<T>)
    constexpr literal_map (T&& rng) noexcept
      : literal_map (std::ranges::begin (rng), std::ranges::end (rng))
    {}

    // ---

    [[nodiscard]] constexpr iterator begin () noexcept
    {
      return _elements.begin ();
    }

    [[nodiscard]] constexpr const_iterator begin () const noexcept
    {
      return _elements.begin ();
    }

    [[nodiscard]] constexpr iterator end () noexcept
    {
      return _elements.begin () + _size;
    }

    [[nodiscard]] constexpr const_iterator end () const noexcept
    {
      return _elements.begin () + _size;
    }

    // ---

    [[nodiscard]] constexpr element_type& at (key_type const& key) noexcept
    {
      return find (key)->value;
    }

    [[nodiscard]] constexpr element_type const& at (key_type const& key) const noexcept
    {
      return find (key)->value;
    }

    // ---

    [[nodiscard]] constexpr bool contains (key_type const& key) const noexcept
    {
      return find (key) != end ();
    }

    // ---

    [[nodiscard]] constexpr iterator find (key_type const& key) noexcept
    {
      return std::ranges::find_if (begin (), end (), [&key] (auto& curr) {
        return curr.key == key;
      });
    }

    [[nodiscard]] constexpr const_iterator find (key_type const& key) const noexcept
    {
      return std::ranges::find_if (begin (), end (), [&key] (auto& curr) {
        return curr.key == key;
      });
    }

    // ---

    template<class... Args>
      requires (std::constructible_from<element_type, Args...>)
    constexpr void emplace (key_type key, Args&&... args) noexcept
    {
      element_type value = element_type (std::forward<Args&&> (args)...);
      _elements[_size++] = value_type {std::move (key), std::move (value)};
    }

    // ---

    [[nodiscard]] constexpr element_type& operator[] (key_type const& key) noexcept
    {
      return at (key);
    }

    [[nodiscard]] constexpr element_type const& operator[] (key_type const& key) const noexcept
    {
      return at (key);
    }
  };
} // namespace forest::internal
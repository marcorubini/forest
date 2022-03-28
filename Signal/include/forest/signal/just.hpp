#pragma once
#include <forest/signal/core.hpp>

namespace forest::signal
{
  template<std::move_constructible T>
  struct just
  {
  private:
    static constexpr bool can_copy_noexcept = std::is_nothrow_copy_constructible_v<T>;
    static constexpr bool can_move_noexcept = std::is_nothrow_move_constructible_v<T>;
    static constexpr bool can_copy = std::copy_constructible<T>;

  public:
    T _value;

    constexpr just (T value) noexcept (can_move_noexcept)
      : _value (std::move (value))
    {}

    [[nodiscard]] constexpr bool has_value () const noexcept
    {
      return true;
    }

    [[nodiscard]] constexpr auto& value () const& noexcept
    {
      return _value;
    }

    [[nodiscard]] constexpr auto& value () & noexcept
    {
      return _value;
    }

    [[nodiscard]] constexpr auto value () && noexcept (can_move_noexcept)
    {
      return std::move (_value);
    }

    [[nodiscard]] constexpr operator T () const& noexcept (can_move_noexcept) requires (can_copy)
    {
      return _value;
    }

    [[nodiscard]] constexpr operator T () && noexcept (can_move_noexcept)
    {
      return std::move (_value);
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto& get () const& noexcept
    {
      return _value;
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto& get () & noexcept
    {
      return _value;
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto get () && noexcept (can_move_noexcept)
    {
      return std::move (_value);
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto& get () const& noexcept
    {
      return _value;
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto& get () & noexcept
    {
      return _value;
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto get () && noexcept (can_move_noexcept)
    {
      return std::move (_value);
    }

    [[nodiscard]] constexpr auto index () const noexcept -> long
    {
      return 0l;
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&std::constructible_from<Other, T const&>)
    [[nodiscard]] constexpr operator Other () const& noexcept (can_copy_noexcept) requires (can_copy)
    {
      return Other (_value);
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&std::constructible_from<Other, T&&>)
    [[nodiscard]] constexpr operator Other () && noexcept (can_move_noexcept)
    {
      return Other (std::move (_value));
    }

    bool operator== (just const&) const = default;
    auto operator<=> (just const&) const = default;
  };
} // namespace forest::signal
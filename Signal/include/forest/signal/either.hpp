#pragma once
#include <variant>

#include <forest/signal/core.hpp>
#include <forest/signal/nothing.hpp>

namespace forest::signal
{
  template<std::move_constructible... Ts>
    requires (sizeof...(Ts) > 0)
  struct either
  {
  private:
    static constexpr long size = sizeof...(Ts);

    template<class T>
    static constexpr bool contains = (std::same_as<Ts, T> || ...);

    static constexpr bool can_copy = (std::copy_constructible<Ts> && ...);
    static constexpr bool can_copy_noexcept = (std::is_nothrow_copy_constructible_v<Ts> && ...);
    static constexpr bool can_move_noexcept = (std::is_nothrow_move_constructible_v<Ts> && ...);

  public:
    std::variant<Ts...> _value;

    template<class Init>
      requires (contains<Init>)
    constexpr either (Init init) noexcept (std::is_nothrow_move_constructible_v<Init>)
      : _value (std::move (init))
    {}

    template<Signal Other>
      requires ((!Nothing<Other>)&&(std::constructible_from<Other, Ts const&>&&...))
    [[nodiscard]] constexpr operator Other () const& noexcept (can_copy_noexcept) requires (can_copy)
    {
      auto visit = [] (auto& value) -> Other {
        return Other (value);
      };
      return std::visit<Other> (visit, _value);
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&(std::constructible_from<Other, Ts&&>&&...))
    [[nodiscard]] constexpr operator Other () && noexcept (can_move_noexcept)
    {
      auto visit = [] (auto& value) -> Other {
        return Other (std::move (value));
      };
      return std::visit (visit, _value);
    }

    [[nodiscard]] constexpr bool has_value () const noexcept
    {
      return true;
    }

    template<class T>
      requires contains<T>
    [[nodiscard]] constexpr auto& get () &
    {
      return std::get<T> (_value);
    }

    template<class T>
      requires contains<T>
    [[nodiscard]] constexpr auto& get () const&
    {
      return std::get<T> (_value);
    }

    template<class T>
      requires contains<T>
    [[nodiscard]] constexpr auto get () &&
    {
      return std::move (std::get<T> (_value));
    }

    template<long I>
      requires (I >= 0 && I < size)
    [[nodiscard]] constexpr auto& get () &
    {
      return std::get<I> (_value);
    }

    template<long I>
      requires (I >= 0 && I < size)
    [[nodiscard]] constexpr auto& get () const&
    {
      return std::get<I> (_value);
    }

    template<long I>
      requires (I >= 0 && I < size)
    [[nodiscard]] constexpr auto get () &&
    {
      return std::move (std::get<I> (_value));
    }

    [[nodiscard]] constexpr auto index () const noexcept -> long
    {
      return static_cast<long> (_value.index ());
    }
  };

} // namespace forest::signal
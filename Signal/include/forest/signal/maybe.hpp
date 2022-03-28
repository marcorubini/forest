#pragma once
#include <optional>
#include <stdexcept>
#include <variant>

#include <forest/meta.hpp>
#include <forest/signal/core.hpp>
#include <forest/signal/nothing.hpp>

namespace forest::signal
{
  template<std::move_constructible... Ts>
    requires (sizeof...(Ts) > 0)
  struct maybe
  {
  private:
    static constexpr long size = sizeof...(Ts);

    template<class T>
    static constexpr bool contains = (std::same_as<Ts, T> || ...);

    static constexpr bool can_copy = (std::copy_constructible<Ts> && ...);
    static constexpr bool can_copy_noexcept = (std::is_nothrow_copy_constructible_v<Ts> && ...);
    static constexpr bool can_move_noexcept = (std::is_nothrow_move_constructible_v<Ts> && ...);

  public:
    std::variant<nothing, Ts...> _value;

    maybe () = default;

    template<class Init>
      requires (contains<Init>)
    constexpr maybe (Init init) noexcept (std::is_nothrow_move_constructible_v<Init>)
      : _value (std::move (init))
    {}

    constexpr maybe (nothing init) noexcept
      : _value ()
    {}

    constexpr maybe (std::nullopt_t init) noexcept
      : _value ()
    {}

  private:
    template<Signal S>
    [[nodiscard]] constexpr auto cast_visitor ()
    {
      return []<class T> (T value) -> S {
        if constexpr (std::same_as<T, nothing> && !Maybe<S>) {
          HEDLEY_UNREACHABLE ();
        } else {
          return S (std::move (value));
        }
      };
    }

  public:
    template<Signal Other>
      requires ((!Nothing<Other>)&&(std::constructible_from<Other, Ts const&>&&...))
    [[nodiscard]] explicit(Either<Other> || Just<Other>) constexpr operator Other () const& //
      noexcept (can_copy_noexcept && (Maybe<Other> || Optional<Other>))                     //
      requires (can_copy)                                                                   //
    {
      if constexpr (Either<Other> || Just<Other>)
        if (!has_value ())
          throw std::logic_error ("Bad cast from 'maybe' with no value.");

      if constexpr (Maybe<Other> || Optional<Other>)
        if (!has_value ())
          return Other ();
      return std::visit (cast_visitor<Other> (), _value);
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&(std::constructible_from<Other, Ts&&>&&...))
    [[nodiscard]] explicit(Either<Other> || Just<Other>) constexpr operator Other () && //
      noexcept (can_move_noexcept && (Maybe<Other> || Optional<Other>))                 //
    {
      if constexpr (Either<Other> || Just<Other>)
        if (!has_value ())
          throw std::logic_error ("Bad cast from 'maybe' with no value.");

      if constexpr (Maybe<Other> || Optional<Other>)
        if (!has_value ())
          return Other ();
      return std::visit (cast_visitor<Other> (), std::move (_value));
    }

    [[nodiscard]] constexpr bool has_value () const noexcept
    {
      return _value.index () != 0;
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
      return std::get<I + 1> (_value);
    }

    template<long I>
      requires (I >= 0 && I < size)
    [[nodiscard]] constexpr auto& get () const&
    {
      return std::get<I + 1> (_value);
    }

    template<long I>
      requires (I >= 0 && I < size)
    [[nodiscard]] constexpr auto get () &&
    {
      return std::move (std::get<I + 1> (_value));
    }

    [[nodiscard]] constexpr auto index () const noexcept -> long
    {
      return static_cast<long> (_value.index ()) - 1;
    }
  };

} // namespace forest::signal
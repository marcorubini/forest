#pragma once
#include <optional>
#include <stdexcept>

#include <forest/signal/core.hpp>
#include <forest/signal/nothing.hpp>

namespace forest::signal
{
  template<std::move_constructible T>
  struct optional
  {
  private:
    static constexpr bool can_copy_noexcept = std::is_nothrow_copy_constructible_v<T>;
    static constexpr bool can_move_noexcept = std::is_nothrow_move_constructible_v<T>;
    static constexpr bool can_copy = std::copy_constructible<T>;

  public:
    std::optional<T> _value;

    optional () = default;

    template<class Init = T>
    explicit(!std::convertible_to<Init&&, T>)                                      //
      constexpr optional (Init value)                                              //
      noexcept (std::is_nothrow_constructible_v<T, Init&&>)                        //
      requires (!std::same_as<Init, optional> && std::constructible_from<T, Init>) //
      : _value (std::move (value))
    {}

    constexpr optional (nothing) noexcept
      : _value ()
    {}

    constexpr optional (std::nullopt_t) noexcept
      : _value ()
    {}

    [[nodiscard]] constexpr bool has_value () const noexcept
    {
      return _value.has_value ();
    }

    [[nodiscard]] constexpr auto& value () const&
    {
      return _value.value ();
    }

    [[nodiscard]] constexpr auto& value () &
    {
      return _value.value ();
    }

    [[nodiscard]] constexpr auto value () &&
    {
      return std::move (_value).value ();
    }

    template<class Other>
    [[nodiscard]] constexpr auto value_or (Other other) const&
    {
      if (has_value ())
        return T (*_value);
      return T (std::move (other));
    }

    template<class Other>
    [[nodiscard]] constexpr auto value_or (Other other) &&
    {
      if (has_value ())
        return T (std::move (*_value));
      return T (std::move (other));
    }

    [[nodiscard]] constexpr auto& operator* () const& noexcept
    {
      return *_value;
    }

    [[nodiscard]] constexpr auto& operator* () & noexcept
    {
      return *_value;
    }

    [[nodiscard]] constexpr auto operator* () && noexcept (can_move_noexcept)
    {
      return std::move (*_value);
    }

    [[nodiscard]] explicit constexpr operator T () const& requires (can_copy)
    {
      return _value.value ();
    }

    [[nodiscard]] explicit constexpr operator T () &&
    {
      return std::move (_value).value ();
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto& get () const&
    {
      return _value.value ();
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto& get () &
    {
      return _value.value ();
    }

    template<long I>
      requires (I == 0)
    [[nodiscard]] constexpr auto get () &&
    {
      return std::move (_value).value ();
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto& get () const&
    {
      return _value.value ();
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto& get () &
    {
      return _value.value ();
    }

    template<std::same_as<T>>
    [[nodiscard]] constexpr auto get () &&
    {
      return std::move (_value).value ();
    }

    [[nodiscard]] constexpr auto index () const noexcept -> long
    {
      return 0l;
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&std::constructible_from<Other, T const&>)
    [[nodiscard]] explicit(Either<Other> || Just<Other>) constexpr operator Other () const& //
      noexcept (can_copy_noexcept && !Just<Other> && !Either<Other>)                        //
      requires (can_copy)                                                                   //
    {
      if constexpr (Just<Other> || Either<Other>)
        if (!has_value ())
          throw std::logic_error ("Bad cast from 'optional' with no value.");

      if constexpr (Maybe<Other> || Optional<Other>)
        if (!has_value ())
          return Other ();
      return Other (*_value);
    }

    template<Signal Other>
      requires ((!Nothing<Other>)&&std::constructible_from<Other, T&&>)
    [[nodiscard]] explicit(Either<Other> || Just<Other>) constexpr operator Other () && //
      noexcept (can_move_noexcept && !Just<Other> && !Either<Other>)                    //
    {
      if constexpr (Just<Other> || Either<Other>)
        if (!has_value ())
          throw std::logic_error ("Bad cast from 'optional' with no value.");

      if constexpr (Maybe<Other> || Optional<Other>)
        if (!has_value ())
          return Other ();
      return Other (std::move (*_value));
    }

    bool operator== (optional const&) const = default;
    auto operator<=> (optional const&) const = default;
  };
} // namespace forest::signal
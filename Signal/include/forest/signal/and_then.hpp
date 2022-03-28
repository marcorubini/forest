#pragma once
#include <forest/signal/concepts.hpp>

namespace forest::signal
{
  template<std::move_constructible... Fs>
    requires (sizeof...(Fs) > 0)
  struct and_then;

  template<std::move_constructible Fn>
  struct and_then<Fn>
  {
    Fn _fn;

    constexpr and_then (Fn fn) noexcept
      : _fn (std::move (fn))
    {}

    template<class Arg>
    [[nodiscard]] constexpr auto operator() (Arg const& arg) const //
      noexcept (false)                                             //
      requires (Invocable<Fn, Arg>)                                //
    {
      return _fn (arg);
    }
  };

  template<std::move_constructible F1, std::move_constructible... Fn>
  struct and_then<F1, Fn...>
  {
    using recurse = and_then<Fn...>;

    F1 _f1;
    and_then<Fn...> _fn;

    constexpr and_then (F1 f1, Fn... fn) noexcept
      : _f1 (std::move (f1))
      , _fn (std::move (fn)...)
    {}

    template<class Arg>
    [[nodiscard]] constexpr auto operator() (Arg const& arg) const                  //
      noexcept (false)                                                              //
      requires (Invocable<F1, Arg>&& Projection<recurse, invoke_result_t<F1, Arg>>) //
    {
      return visit (_fn, _f1 (arg));
    }
  };

  template<class... Fs>
  and_then (Fs...) -> and_then<Fs...>;

} // namespace forest::signal

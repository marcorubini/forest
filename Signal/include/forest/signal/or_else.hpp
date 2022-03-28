#pragma once
#include <forest/signal/concepts.hpp>

namespace forest::signal
{

  template<std::move_constructible... Fns>
  struct or_else;

  template<>
  struct or_else<>
  {
    template<class Arg>
    [[nodiscard]] constexpr auto operator() (Arg&& arg) const
    {
      return nothing {};
    }
  };

  template<std::move_constructible Fn>
  struct or_else<Fn>
  {
    Fn _fn;

    constexpr or_else (Fn fn) noexcept
      : _fn (std::move (fn))
    {}

    template<class Arg>
    [[nodiscard]] constexpr auto operator() (Arg const& arg) const //
      requires (Invocable<Fn, Arg>)                                //
    {
      return _fn (arg);
    }
  };

  template<std::move_constructible F1, std::move_constructible... Fs>
  struct or_else<F1, Fs...>
  {
    using recurse = or_else<Fs...>;

    F1 _f1;
    or_else<Fs...> _fs;

    constexpr or_else (F1 f1, Fs... fs)
      : _f1 (std::move (f1))
      , _fs (std::move (fs)...)
    {}

    template<class Arg>
    [[nodiscard]] constexpr auto operator() (Arg const& arg) const //
      requires (Invocable<F1, Arg>&& Invocable<recurse, Arg>)      //
    {
      using result_type = signal_disjunction_t< //
        invoke_result_t<F1, Arg>,
        invoke_result_t<recurse, Arg>>;

      if (auto curr = _f1 (arg); curr.has_value ())
        if constexpr (!Nothing<decltype (curr)>)
          return result_type (std::move (curr));
      return result_type (_fs (arg));
    }
  };

  template<class... Fs>
  or_else (Fs...) -> or_else<Fs...>;

} // namespace forest::signal
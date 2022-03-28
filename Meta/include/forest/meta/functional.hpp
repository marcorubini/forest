#pragma once
#include <concepts>
#include <functional>
#include <utility>

#include <boost/mp11/algorithm.hpp>
#include <forest/meta/overloaded.hpp>

namespace forest::meta
{
  namespace internal
  {
    template<class First, class Second, class... Args>
    concept Composable = requires (First const first, Second const second, Args&&... args)
    {
      // clang-format off
      { second(first(std::forward<Args>(args)...)) };
      // clang-format on
    };
  } // namespace internal

  template<std::move_constructible... Fns>
    requires (sizeof...(Fns) > 0)
  struct compose;

  template<std::move_constructible Fn>
  struct compose<Fn>
  {
  public:
    Fn _fn;

    constexpr compose (Fn fn) noexcept
      : _fn (std::move (fn))
    {}

    template<class... Args>
    constexpr auto operator() (Args&&... args) const             //
      noexcept (std::is_nothrow_invocable_v<Fn const&, Args...>) //
    {
      return _fn (std::forward<Args> (args)...);
    }
  };

  template<std::move_constructible Fn1, std::move_constructible... Fns>
  struct compose<Fn1, Fns...>
  {
  private:
    using recurse = compose<Fns...>;

    template<class... Args>
    static constexpr bool is_noexcept = std::is_nothrow_invocable_v<Fn1 const&, Args...> //
      && std::is_nothrow_invocable_v<recurse const&, std::invoke_result_t<Fn1 const&, Args...>>;

    template<class... Args>
    static constexpr bool is_callable = std::invocable<Fn1 const&, Args...> //
      && std::invocable<recurse const&, std::invoke_result_t<Fn1 const&, Args...>>;

    template<class... Args>
    using recurse_result_t = std::invoke_result_t<recurse const&, Args...>;

  public:
    Fn1 _fn1;
    compose<Fns...> _fns;

    constexpr compose (Fn1 fn1, Fns... fns) noexcept
      : _fn1 (std::move (fn1), _fns (std::move (fns)...))
    {}

    template<class... Args>
    constexpr auto operator() (Args&&... args) const //
      noexcept (is_noexcept<Args...>)                //
      requires (is_callable<Args...>)                //
    {
      return _fns (_fn1 (std::forward<Args> (args)...));
    }
  };

  template<class... Fns>
  compose (Fns...) -> compose<Fns...>;

  template<std::move_constructible... Fns>
    requires (sizeof...(Fns) > 0)
  struct first_of;

  template<std::move_constructible Fn>
  struct first_of<Fn>
  {
  public:
    Fn _fn;

    constexpr first_of (Fn fn) noexcept
      : _fn (std::move (fn))
    {}

    template<class... Args>
    constexpr auto operator() (Args&&... args) const             //
      noexcept (std::is_nothrow_invocable_v<Fn const&, Args...>) //
      requires (std::invocable<Fn const&, Args...>)              //
    {
      return _fn (std::forward<Args> (args)...);
    }
  };

  template<std::move_constructible F1, std::move_constructible... Fns>
  struct first_of<F1, Fns...>
  {
  private:
    using recurse = first_of<Fns...>;

    template<class... Args>
    static constexpr bool is_noexcept = std::is_nothrow_invocable_v<F1 const&, Args...> //
      || std::is_nothrow_invocable_v<recurse const&, Args...>;

    template<class... Args>
    static constexpr bool is_callable = std::invocable<F1 const&, Args...> //
      || std::invocable<recurse const&, Args...>;

  public:
    F1 _fn1;
    first_of<Fns...> _fns;

    constexpr first_of (F1 fn1, Fns... fns) noexcept
      : _fn1 (std::move (fn1))
      , _fns (std::move (fns)...)
    {}

    template<class... Args>
    constexpr auto operator() (Args&&... args) const //
      noexcept (is_noexcept<Args...>)                //
      requires (is_callable<Args...>)                //
    {
      if constexpr (std::invocable<F1 const&, Args...>) {
        return _fn1 (std::forward<Args> (args)...);
      } else {
        return _fns (std::forward<Args> (args)...);
      }
    }
  };

  template<class... Fns>
  first_of (Fns...) -> first_of<Fns...>;

} // namespace forest::meta
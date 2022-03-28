#pragma once
#include <boost/mp11/algorithm.hpp>
#include <forest/signal/core.hpp>
#include <forest/signal/either.hpp>
#include <forest/signal/just.hpp>
#include <forest/signal/maybe.hpp>
#include <forest/signal/nothing.hpp>
#include <forest/signal/optional.hpp>

namespace forest::signal
{
  template<class T, class S>
  concept Invocable = requires (T const& proj, S const& element)
  {
    // clang-format off
    { proj(element) } -> Signal;
    // clang-format on
  };

  template<class T, class S>
    requires Invocable<T, S>
  using invoke_result_t = decltype (std::declval<T const&> () (std::declval<S const&> ()));

  constexpr inline struct
  {
    template<class T>
    [[nodiscard]] constexpr auto operator() (T const& proj, nothing const&) const //
      noexcept (false)                                                            //
      -> Signal auto                                                              //
    {
      if constexpr (std::invocable<T const&, nothing>) {
        static_assert (Invocable<T, nothing>, "Visit with 'nothing' as argument does not return a Signal.");
        return proj (nothing {});
      } else {
        return nothing {};
      }
    }

    template<class T, class E>
    [[nodiscard]] constexpr auto operator() (T const& proj, just<E> const& signal) const //
      noexcept (false)                                                                   //
      -> Signal auto                                                                     //
      requires (Invocable<T, E>)                                                         //
    {
      return proj (signal.value ());
    }

    template<class T, class E>
    [[nodiscard]] constexpr auto operator() (T const& proj, optional<E> const& signal) const //
      noexcept (false)                                                                       //
      -> Signal auto                                                                         //
      requires (Invocable<T, E>)                                                             //
    {
      constexpr bool handles_nothing = std::invocable<T const&, nothing>;
      constexpr bool handles_nothing_valid = Invocable<T, nothing>;
      static_assert (handles_nothing <= handles_nothing_valid, "Visit with 'nothing' as argument does not return a Signal.");

      if constexpr (handles_nothing) {
        using result_type = signal_union_t<invoke_result_t<T, E>, invoke_result_t<T, nothing>>;
        if (signal.has_value ())
          return result_type (proj (*signal));
        return result_type (proj (nothing {}));
      } else {
        using result_type = signal_union_t<invoke_result_t<T, E>, nothing>;
        if (signal.has_value ())
          return result_type (proj (*signal));
        return result_type (nothing {});
      }
    }

    template<class P, class... Ts>
    [[nodiscard]] constexpr auto operator() (P const& proj, either<Ts...> const& signal) const //
      noexcept (false)                                                                         //
      -> Signal auto                                                                           //
      requires (Invocable<P, Ts>&&...)                                                         //
    {
      using boost::mp11::mp_with_index;
      using result_type = signal_union_t<invoke_result_t<P, Ts>...>;
      return mp_with_index<sizeof...(Ts)> (signal.index (), [&signal, &proj] (auto I) -> result_type {
        return result_type (proj (signal.template get<I> ()));
      });
    }

    template<class P, class... Ts>
    [[nodiscard]] constexpr auto operator() (P const& proj, maybe<Ts...> const& signal) const //
      noexcept (false)                                                                        //
      -> Signal auto                                                                          //
      requires (Invocable<P, Ts>&&...)                                                        //
    {
      using boost::mp11::mp_with_index;
      constexpr bool handles_nothing = std::invocable<P const&, nothing>;
      constexpr bool handles_nothing_valid = Invocable<P, nothing>;
      static_assert (handles_nothing <= handles_nothing_valid, "Visit with 'nothing' as argument does not return a Signal.");

      constexpr long N = sizeof...(Ts);
      if constexpr (handles_nothing) {
        using result_type = signal_union_t<invoke_result_t<P, Ts>..., invoke_result_t<P, nothing>>;
        if (!signal.has_value ())
          return result_type (proj (nothing {}));
        return mp_with_index<N> (signal.index (), [&] (auto I) -> result_type {
          return result_type (proj (signal.template get<I> ()));
        });
      } else {
        using result_type = signal_union_t<invoke_result_t<P, Ts>..., nothing>;
        if (!signal.has_value ())
          return result_type (nothing {});
        return mp_with_index<N> (signal.index (), [&] (auto I) -> result_type {
          return result_type (proj (signal.template get<I> ()));
        });
      }
    }
  } visit {};

  constexpr inline struct
  {
    template<class V, Signal S>
    constexpr void operator() (V&& visitor, S const& signal) const
    {
      auto proj = [&]<class T> (T const& element) {
        if constexpr (std::same_as<T, nothing> && !std::invocable<V, nothing>) {
          return nothing {};
        } else {
          visitor (element);
          return nothing {};
        }
      };
      (void)visit (proj, signal);
    }
  } visit_sink {};

  template<class T, class S>
  concept Projection = Signal<S> && requires (T const& proj, S const& signal)
  {
    // clang-format off
    { visit(proj, signal) } -> Signal;
    // clang-format on
  };

  template<class T, class S>
    requires Projection<T, S>
  using projection_result_t = decltype (visit (std::declval<T const> (), std::declval<S const> ()));

  template<class T, class S>
  concept Sink = Signal<S> && requires (T& sink, S const& signal)
  {
    // clang-format off
    { sink(signal) } -> std::same_as<void>;
    // clang-format on
  };

} // namespace forest::signal
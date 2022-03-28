#pragma once
#include <concepts>

#include <boost/mp11/algorithm.hpp>

namespace forest::signal
{
  // forward declarations

  namespace internal
  {
    template<bool maybe_null, class... Ts>
    struct variant;
  } // namespace internal

  struct nothing;

  template<std::move_constructible... Ts>
    requires (sizeof...(Ts) > 0)
  struct maybe;

  template<std::move_constructible... Ts>
    requires (sizeof...(Ts) > 0)
  struct either;

  template<std::move_constructible T>
  struct just;

  template<std::move_constructible T>
  struct optional;

  // detection traits

  namespace internal
  {
    template<class T>
    constexpr inline bool is_maybe = false;

    template<class... Ts>
    constexpr inline bool is_maybe<maybe<Ts...>> = true;

    template<class T>
    constexpr inline bool is_either = false;

    template<class... Ts>
    constexpr inline bool is_either<either<Ts...>> = true;

    template<class T>
    constexpr inline bool is_just = false;

    template<class T>
    constexpr inline bool is_just<just<T>> = true;

    template<class T>
    constexpr inline bool is_optional = false;

    template<class T>
    constexpr inline bool is_optional<optional<T>> = true;

    template<class T>
    constexpr inline bool is_nothing = false;

    template<>
    constexpr inline bool is_nothing<nothing> = true;
  } // namespace internal

} // namespace forest::signal

namespace forest::signal
{
  template<class T>
  concept Nothing = internal::is_nothing<T>;

  template<class T>
  concept Maybe = internal::is_maybe<T>;

  template<class T>
  concept Either = internal::is_either<T>;

  template<class T>
  concept Just = internal::is_just<T>;

  template<class T>
  concept Optional = internal::is_optional<T>;

  template<class T>
  concept Signal = Nothing<T> || Maybe<T> || Either<T> || Just<T> || Optional<T>;

  // traits

  template<Signal T>
  constexpr inline long num_alternatives = 0;

  template<class... Ts>
  constexpr inline long num_alternatives<either<Ts...>> = sizeof...(Ts);

  template<class... Ts>
  constexpr inline long num_alternatives<maybe<Ts...>> = sizeof...(Ts);

  template<class T>
  constexpr inline long num_alternatives<just<T>> = 1;

  template<class T>
  constexpr inline long num_alternatives<optional<T>> = 1;

  template<>
  constexpr inline long num_alternatives<nothing> = 0;

  // collapse either to just / maybe to optional

  template<Signal T>
  struct collapse : std::type_identity<T>
  {};

  template<class T>
  struct collapse<maybe<T>> : std::type_identity<optional<T>>
  {};

  template<class T>
  struct collapse<either<T>> : std::type_identity<just<T>>
  {};

  template<Signal T>
  using collapse_t = typename collapse<T>::type;

  // concat

  template<Signal T, class... Ts>
  struct concat_alternatives;

  template<Signal T, class... Ts>
  using concat_alternatives_t = typename concat_alternatives<T, Ts...>::type;

  template<class... Ts>
  struct concat_alternatives<nothing, Ts...> : collapse<maybe<Ts...>>
  {};

  template<class T, class... Ts>
  struct concat_alternatives<just<T>, Ts...> : collapse<boost::mp11::mp_set_union<either<T>, either<Ts...>>>
  {};

  template<class T, class... Ts>
  struct concat_alternatives<optional<T>, Ts...> : collapse<boost::mp11::mp_set_union<maybe<T>, maybe<Ts...>>>
  {};

  template<class T, class... Us>
    requires (Maybe<T> || Either<T>)
  struct concat_alternatives<T, Us...> : collapse<boost::mp11::mp_set_union<T, boost::mp11::mp_list<Us...>>>
  {};

  // addition

  template<Signal T, class... Ts>
  struct add_alternatives;

  template<class... Ts>
  struct add_alternatives<nothing, Ts...> : collapse<either<Ts...>>
  {};

  template<class T, class... Us>
    requires (Maybe<T> || Either<T>)
  struct add_alternatives<T, Us...> : collapse<boost::mp11::mp_set_union<T, boost::mp11::mp_list<Us...>>>
  {};

  template<class T, class... Us>
  struct add_alternatives<just<T>, Us...> : collapse<boost::mp11::mp_set_union<either<T>, either<Us...>>>
  {};

  template<class T, class... Us>
  struct add_alternatives<optional<T>, Us...> : collapse<boost::mp11::mp_set_union<maybe<T>, maybe<Us...>>>
  {};

  template<Signal T>
  struct add_nothing;

  template<>
  struct add_nothing<nothing> : std::type_identity<nothing>
  {};

  template<class T>
    requires (Maybe<T> || Optional<T>)
  struct add_nothing<T> : std::type_identity<T>
  {};

  template<class T>
    requires (Either<T> || Just<T>)
  struct add_nothing<T> : collapse<boost::mp11::mp_rename<T, maybe>>
  {};

  template<Signal T, class... Ts>
  using add_alternatives_t = typename add_alternatives<T, Ts...>::type;

  template<Signal T>
  using add_nothing_t = typename add_nothing<T>::type;

  // union

  template<Signal... Ts>
  struct signal_union : std::type_identity<nothing>
  {};

  template<Signal... Ts>
  using signal_union_t = typename signal_union<Ts...>::type;

  template<Signal T>
  struct signal_union<T> : std::type_identity<T>
  {};

  template<Nothing T1, Signal T2>
  struct signal_union<T1, T2> : add_nothing<T2>
  {};

  template<class... Ts, Signal T2>
  struct signal_union<maybe<Ts...>, T2> : add_nothing<concat_alternatives_t<T2, Ts...>>
  {};

  template<class... Ts, Signal T2>
  struct signal_union<either<Ts...>, T2> : concat_alternatives<T2, Ts...>
  {};

  template<class T, Signal T2>
  struct signal_union<just<T>, T2> : concat_alternatives<T2, T>
  {};

  template<class T, Signal T2>
  struct signal_union<optional<T>, T2> : add_nothing<concat_alternatives_t<T2, T>>
  {};

  template<Signal T1, Signal T2, Signal... Ts>
  struct signal_union<T1, T2, Ts...> : signal_union<T1, signal_union_t<T2, Ts...>>
  {};

  // disjunction

  template<Signal... Ts>
  struct signal_disjunction : std::type_identity<nothing>
  {};

  template<Signal... Ts>
  using signal_disjunction_t = typename signal_disjunction<Ts...>::type;

  template<Signal T>
  struct signal_disjunction<T> : std::type_identity<T>
  {};

  template<Signal T1, Signal T2>
  struct signal_disjunction<T1, T2>
  {
    static constexpr auto deduce ()
    {
      if constexpr (Just<T1> || Either<T1> || Nothing<T2>) {
        return std::type_identity<T1> {};
      } else if constexpr (Nothing<T1>) {
        return std::type_identity<T2> {};
      } else {
        static_assert (Maybe<T1> || Optional<T1>);
        static_assert (!Nothing<T2>);

        auto unwrap = []<template<class...> class TT1,
          template<class...>
          class TT2, //
          class... Ts,
          class... Us> (std::type_identity<TT1<Ts...>>, std::type_identity<TT2<Us...>>)
        {
          if constexpr (Maybe<T2> || Optional<T2>) {
            return std::type_identity<add_alternatives_t<T1, Us...>> {};
          } else {
            static_assert (Just<T2> || Either<T2>);
            return std::type_identity<add_alternatives_t<T2, Ts...>> {};
          }
        };

        return unwrap (std::type_identity<T1> {}, std::type_identity<T2> {});
      }

      if constexpr (Nothing<T1>) {
        return std::type_identity<T2> {};
      } else if constexpr (Just<T1> || Either<T1>) {
        return std::type_identity<T1> {};
      }
    }

    using type = typename decltype (deduce ())::type;
  };

  template<Signal T1, Signal T2, Signal... Ts>
  struct signal_disjunction<T1, T2, Ts...> : signal_disjunction<T1, signal_disjunction_t<T2, Ts...>>
  {};

} // namespace forest::signal
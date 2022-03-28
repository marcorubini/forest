#pragma once
#include <algorithm>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>

#include <forest/meta/constant.hpp>
#include <forest/meta/tuple_like.hpp>

namespace forest::meta
{
  template<auto Iterable>
  constexpr inline auto for_each = []<class F> (F callback) {
    using iterable_type = decltype (Iterable);

    constexpr bool is_tuple_like = tuple_like<iterable_type>;
    constexpr bool is_random_access_range = std::ranges::random_access_range<iterable_type>;

    constexpr auto access = []<std::size_t I> (meta::constant<I>) {
      if constexpr (is_tuple_like) {
        return constant<std::get<I> (Iterable)> {};
      } else if constexpr (is_random_access_range) {
        return constant<Iterable[I]> {};
      } else {
        return;
      }
    };

    constexpr auto invoke = [access] (auto& self, auto& callback, auto head, auto... tail) {
      using ret = decltype (callback (access (head)));
      constexpr bool can_stop = std::same_as<ret, bool>;

      if constexpr (can_stop) {
        if (callback (access (head)))
          return true;
      } else {
        callback (access (head));
      }

      if constexpr (sizeof...(tail) > 0)
        return self (self, callback, tail...);
      else if constexpr (can_stop)
        return false;
    };

    constexpr auto invoker = [invoke]<std::size_t... Is> (auto& callback, std::index_sequence<Is...>)
    {
      if constexpr (sizeof...(Is) > 0)
        return invoke (invoke, callback, constant<Is> {}...);
    };

    if constexpr (is_tuple_like) {
      return invoker (callback, std::make_index_sequence<std::tuple_size_v<iterable_type>> {});
    } else if constexpr (is_random_access_range) {
      return invoker (callback, std::make_index_sequence<std::ranges::size (Iterable)> {});
    } else {
      static_assert (is_tuple_like || is_random_access_range);
    }
  };

} // namespace forest::meta
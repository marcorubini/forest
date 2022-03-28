#pragma once
#include <tuple>
#include <type_traits>

namespace forest::internal::meta
{
  template<class T, std::size_t I>
  constexpr inline bool tuple_like_check_element = requires
  {
    // clang-format off
    typename std::tuple_element_t<I, T>;

    requires requires(T& tuple)
    {
      { get<I>(tuple) } -> std::convertible_to<typename std::tuple_element_t<I, T> const&>;
    };
    // clang-format on
  };

  template<class T, std::size_t N>
  constexpr inline bool tuple_like_check = [] () -> bool {
    auto check_indices = []<std::size_t... Is> (std::index_sequence<Is...>)
    {
      return (tuple_like_check_element<T, Is> && ...);
    };

    return check_indices (std::make_index_sequence<N> {});
  }();
} // namespace forest::internal::meta

namespace forest::meta
{
  template<class T>
  constexpr inline bool tuple_like = requires
  {
    // clang-format off
    typename std::tuple_size<T>;
    std::tuple_size<T>::value;
    requires std::tuple_size_v<T> >= 0;
    requires forest::internal::meta::tuple_like_check<T, std::tuple_size_v<T>>;
    // clang-format on
  };

  template<class T>
  concept TupleLike = tuple_like<T>;
} // namespace forest::meta
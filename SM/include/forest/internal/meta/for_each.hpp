#pragma once
#include <algorithm>
#include <span>
#include <tuple>
#include <type_traits>

#include <forest/internal/meta/value_constant.hpp>

namespace forest::internal
{

  template<auto Tuple, long Start = 0, long Count = std::tuple_size_v<decltype (Tuple)> - Start>
  constexpr inline auto consteval_for_each = []<class F> (F callback) {
    using reference_type = decltype (Tuple);
    using tuple_type = std::remove_cvref_t<reference_type>;
    using std::index_sequence;
    using std::make_index_sequence;
    constexpr auto invoke_all = []<auto... Is> (index_sequence<Is...>, F & callback)
    {
      (callback (value_constant<get<Is + Start> (Tuple)> {}), ...);
    };
    invoke_all (make_index_sequence<Count> {}, callback);
  };

  template<std::size_t N, class T, std::size_t M>
  constexpr auto make_array (std::span<T, M> span) -> std::array<std::remove_const_t<T>, N>
  {
    auto result = std::array<std::remove_const_t<T>, N> {};
    std::ranges::copy (span.data (), span.data () + N, result.data ());
    return result;
  }

  template<std::size_t N, class T, std::size_t M>
  constexpr auto make_array (std::array<T, M> array) -> std::array<T, N>
  {
    auto result = std::array<T, N> ();
    std::ranges::copy (array.data (), array.data () + N, result.data ());
    return result;
  }

  template<std::array Range, long Start = 0, long Count = std::ranges::size (Range) - Start>
  constexpr inline auto consteval_for_each_range = []<class F> (F callback) {
    using std::index_sequence;
    using std::make_index_sequence;
    constexpr auto invoke_all = []<auto... Is> (index_sequence<Is...>, F & callback)
    {
      (callback (value_constant<Range[Start + Is]> ()), ...);
    };
    invoke_all (make_index_sequence<Count> {}, callback);
  };
} // namespace forest::internal
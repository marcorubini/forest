#pragma once
#include <forest/meta.hpp>

namespace forest::signal
{
  template<std::move_constructible... Ts>
  struct overload : Ts...
  {
    using Ts::operator()...;

    constexpr overload (Ts... ts)
      : Ts (std::move (ts))...
    {}
  };

  template<class... Ts>
  overload (Ts...) -> overload<Ts...>;
} // namespace forest::signal
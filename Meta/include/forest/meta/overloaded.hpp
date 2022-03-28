#pragma once
#include <utility>

namespace forest::meta
{
  template<class... Overloads>
  struct overloaded : Overloads...
  {
    using Overloads::operator()...;

    constexpr overloaded (Overloads... args)
      : Overloads (std::move (args))...
    {}
  };

  template<class... Ts>
  overloaded (Ts...) -> overloaded<Ts...>;
} // namespace forest::meta
#pragma once
#include <type_traits>

namespace forest::meta
{
  template<auto Value>
  using constant = std::integral_constant<decltype (Value), Value>;
} // namespace forest::meta
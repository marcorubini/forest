#pragma once
#include <type_traits>

namespace forest::internal
{
  template<auto Value>
  using value_constant = std::integral_constant<decltype (Value), Value>;
}
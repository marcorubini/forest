#pragma once
#include <concepts>

namespace forest::meta
{
  template<class T, class U>
  concept SameUnqual = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
}
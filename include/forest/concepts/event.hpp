#pragma once
#include <concepts>

namespace forest
{
  template<class T>
  concept Event = std::copy_constructible<T>;
}

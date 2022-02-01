#pragma once
#include <concepts>

namespace forest
{
  template<class T, class ContextType>
  concept State = requires (T state)
  {
    // clang-format off
    requires std::copy_constructible<T>;

    requires requires (ContextType ctx) {
      { state.on_entry(ctx) };
      { state.on_exit(ctx) };
    };
    // clang-format on
  };
} // namespace forest

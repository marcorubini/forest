#pragma once
#include <forest/signal/core.hpp>

namespace forest::signal
{
  struct nothing
  {
    [[nodiscard]] constexpr bool has_value () const noexcept
    {
      return false;
    }

    bool operator== (nothing const&) const = default;
    auto operator<=> (nothing const&) const = default;
  };
} // namespace forest::signal
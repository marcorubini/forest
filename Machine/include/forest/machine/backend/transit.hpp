#pragma once
#include <variant>

#include <forest/machine/backend/core.hpp>

namespace forest::machine
{
  template<class... Alternatives>
  struct transit_result
  {
    using variant_type = std::variant<transit<void>, transit<Alternatives>...>;
    variant_type _target;

    template<Transit T>
      requires (std::constructible_from<variant_type, T>)
    constexpr transit_result (T init)
      : _target (init)
    {}

    template<class F>
    constexpr decltype (auto) visit (F callback) const
    {
      return std::visit (callback, _target);
    }
  };
} // namespace forest::machine
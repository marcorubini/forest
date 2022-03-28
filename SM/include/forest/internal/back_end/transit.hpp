#pragma once
#include <forest/internal/back_end/base.hpp>
#include <variant>

namespace forest::internal
{
  template<class... Alternatives>
  struct transit_result
  {
    std::variant<transit_none, transit<Alternatives>...> _target;

    template<traits::Transit T>
    constexpr transit_result (T init)
      : _target (init)
    {}

    template<class F>
    constexpr decltype (auto) visit (F callback) const
    {
      auto visitor = [this, &callback]<traits::Transit T> (T) {
        return std::invoke (callback, T {});
      };
      std::visit (visitor, _target);
    }
  };

} // namespace forest::internal
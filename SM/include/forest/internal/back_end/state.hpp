#pragma once
#include <forest/internal/back_end/base.hpp>

namespace forest::internal
{

  // State crtp primary template
  template<class M, class S>
  class state
  {
  public:
    // Fundamental alias that links the state to its machine.
    // machine_type is avaiable from S, and this makes S model the 'State' concept.
    using machine_type = M;

    // Other aliases that are useful for S.
    using state_context = context<M, S>;
    using exact_context = internal::exact_context<M, S>;
    using state_interface = state;

    template<class... Ts>
    using transit_result = internal::transit_result<Ts...>;

    using transit_none = internal::transit_none;

    template<class T>
    using transit = internal::transit<T>;
  };
} // namespace forest::internal
#pragma once
#include <forest/machine/backend/core.hpp>

namespace forest::machine
{
  template<MachineTraits Traits, class State>
  class state_interface
  {
  public:
    using state_context = typename Traits::template context<State>;
    using state_traits = typename Traits::template state<State>;

    template<class... Alternatives>
      requires ((Traits::template contains<Alternatives> || std::same_as<Alternatives, void>)&&...)
    using transit_result = forest::machine::transit_result<Alternatives...>;

    template<class Target>
      requires (Traits::template contains<Target> || std::same_as<Target, void>)
    static constexpr auto transit = forest::machine::transit<Target> {};
  };

  template<MachineTraits Traits, class State>
  class default_state : public state_interface<Traits, State>
  {};
} // namespace forest::machine
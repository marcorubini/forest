#pragma once
#include <forest/machine/backend/core.hpp>

namespace forest::machine
{
  template<MachineTraits Traits, class State>
  class context_interface
  {
  public:
    using machine_reference = typename Traits::reference;

    // members

    machine_reference _machine;

    // constructors

    constexpr context_interface (machine_reference init)
      : _machine (init)
    {}

    // conversion to more-generic context

    template<class OtherState>
      requires (Traits::template is_ancestor<OtherState, State>)
    [[nodiscard]] constexpr operator typename Traits::template context<OtherState> () const
    {
      return {_machine};
    }

    // observers

    template<class Other>
      requires (Traits::template contains<Other>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _machine.template is_active<Other> ();
    }

    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _machine.template is_active<ID> ();
    }

    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr auto& state_cast () const
    {
      return _machine.template state_cast<ID> ();
    }

    template<class OtherState>
      requires (Traits::template contains<OtherState>)
    [[nodiscard]] constexpr auto& state_cast () const
    {
      return _machine.template state_cast<OtherState> ();
    }

    [[nodiscard]] constexpr auto& storage () const
    {
      return _machine.storage ();
    }
  };

  template<MachineTraits Traits, class State>
  class default_context : public context_interface<Traits, State>
  {
    using context_interface<Traits, State>::context_interface;
  };
} // namespace forest::machine
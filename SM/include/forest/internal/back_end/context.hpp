#pragma once
#include <forest/internal/back_end/base.hpp>
#include <forest/internal/meta/dependent.hpp>

namespace forest::internal
{
  // Context primary template
  // This context is avaiable from the state S's on_entry, on_exit, on_reenter, react methods.

  template<class M, class S>
  class context_base
  {
  public:
    M& _machine;

    constexpr context_base (M& init)
      : _machine (init)
    {}
  };

  template<class M, class S>
  class context : public traits::parent_context<M, S>
  {
  public:
    using machine_type = M;
    using state_interface = state<M, S>;
    using typename traits::parent_context<M, S>::context_base;
  };

  template<class M, class S>
    requires (traits::is_root<M, S>)
  class context<M, S> : public context_base<M, S>
  {
  public:
    using machine_type = M;
    using state_interface = state<M, S>;
    using context_base<M, S>::context_base;
  };

  template<class M, class S>
  class exact_context : context_base<M, S>
  {
  public:
    using machine_type = M;
    using state_interface = state<M, S>;
    using context_base<M, S>::context_base;

    constexpr exact_context (std::same_as<context<M, S>> auto init)
      : context_base<M, S> (init._machine)
    {}
  };

} // namespace forest::internal
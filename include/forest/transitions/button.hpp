#pragma once
#include <forest/concepts/context.hpp>
#include <forest/concepts/state.hpp>
#include <forest/events/button_pressed.hpp>
#include <functional>

namespace forest
{
  template<std::copy_constructible Action>
  class button_transition
  {
  private:
    std::string id;
    Action action;

  public:
    button_transition (std::string id, Action action)
      : id (id)
      , action (action)
    {}

    template<Context Ctx, State<Ctx> State>
      requires (std::invocable<Action&, Ctx, State&>)
    bool accepts (Ctx ctx, State& state, events::button_pressed e)
    {
      return e.id == id;
    }

    template<Context Ctx, State<Ctx> State>
      requires (std::invocable<Action&, Ctx, State&>)
    auto operator() (Ctx ctx, State& state, events::button_pressed e)
    {
      return std::invoke (action, ctx, state);
    }
  };

  template<std::copy_constructible Action>
  button_transition (std::string, Action) -> button_transition<Action>;

} // namespace forest

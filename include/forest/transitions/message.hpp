#pragma once
#include <forest/concepts/context.hpp>
#include <forest/concepts/state.hpp>
#include <forest/concepts/transition.hpp>
#include <forest/events/message.hpp>
#include <functional>

namespace forest
{
  template<std::copy_constructible Action>
  class message_transition
  {
  private:
    Action action;

  public:
    message_transition (Action action)
      : action (std::move (action))
    {}

    template<Context Ctx, State<Ctx> S>
      requires (std::invocable<Action, Ctx, S&, std::string>)
    bool accepts (Ctx ctx, S& state, events::message e) const
    {
      return true;
    }

    template<Context Ctx, State<Ctx> S>
      requires (std::invocable<Action, Ctx, S&, std::string>)
    auto operator() (Ctx ctx, S& state, events::message e)
    {
      return std::invoke (action, ctx, state, e.text);
    }
  };

  template<class Action>
  message_transition (Action) -> message_transition<Action>;
} // namespace forest

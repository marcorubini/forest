#pragma once
#include <banana/api.hpp>
#include <forest/concepts/context.hpp>
#include <forest/concepts/transition.hpp>
#include <forest/events/message.hpp>

namespace forest
{
  template<std::copy_constructible Action>
  class command_transition
  {
  private:
    std::string prefix;
    std::string description;
    Action action;

  public:
    command_transition (std::string prefix, std::string description, Action action) //
      : prefix (std::move (prefix))
      , description (std::move (description))
      , action (std::move (action))
    {}

    operator banana::api::bot_command_t () const
    {
      return {prefix.substr (1), description};
    }

    template<Context Ctx, State<Ctx> StateType>
      requires (std::invocable<Action, Ctx, StateType&, std::string>)
    bool accepts (Ctx ctx, StateType&, events::message e) const
    {
      return e.text.starts_with (prefix);
    }

    template<Context Ctx, State<Ctx> StateType>
      requires (std::invocable<Action, Ctx, StateType&, std::string>)
    auto operator() (Ctx ctx, StateType& state, events::message e)
    {
      assert (e.text.starts_with (prefix));
      // TODO: more robust parsing of parameters
      auto param_start = std::min (prefix.length () + 1, e.text.length ());
      return std::invoke (action, ctx, state, e.text.substr (param_start));
    }
  };

  template<class Action>
  command_transition (std::string, std::string, Action) -> command_transition<Action>;

} // namespace forest

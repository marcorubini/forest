#pragma once
#include <concepts>

#include <forest/concepts/context.hpp>
#include <forest/concepts/state.hpp>

namespace forest
{
  template<class T, class ContextType, class StateType, class EventType>
  concept Transition = requires (T transition)
  {
    // clang-format off
    requires std::copy_constructible<T>;
    requires Context<ContextType>;
    requires State<StateType, ContextType>;

    requires requires (ContextType ctx, StateType state, EventType event)
    {
      { transition(ctx, state, event) } -> State<ContextType>;
      { transition.accepts(ctx, state, event) } -> std::same_as<bool>;
    };
    // clang-format on
  };
} // namespace forest

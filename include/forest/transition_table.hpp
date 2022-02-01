#pragma once
#include <forest/concepts/context.hpp>
#include <forest/concepts/event.hpp>
#include <forest/concepts/state.hpp>
#include <forest/concepts/transition.hpp>

#include <optional>
#include <variant>

namespace forest
{
  template<std::copy_constructible GlobalState, std::copy_constructible... Ts>
  class transition_table
  {
  private:
    std::tuple<Ts...> transitions;

  public:
    transition_table () = default;

    constexpr transition_table (Ts... ts)
      : transitions (std::move (ts)...)
    {}

    template<Context ContextType, Event EventType>
    auto trigger (ContextType context, GlobalState& state, EventType event) //
      -> std::optional<GlobalState>
    {
      auto const state_iterator = [&, this]<class CurrState> (CurrState& state) -> std::optional<GlobalState> {
        std::optional<GlobalState> result {};

        auto const transition_iterator = [&, this]<class CurrTransition> () {
          if constexpr (Transition<CurrTransition, ContextType, CurrState, EventType>)
            if (!result.has_value () && std::get<CurrTransition> (transitions).accepts (context, state, event))
              result = std::get<CurrTransition> (transitions) (context, state, event);
        };

        (transition_iterator.template operator()<Ts> (), ...);
        return result;
      };

      return std::visit (state_iterator, state);
    }
  };

  template<std::copy_constructible... States>
  constexpr inline auto make_transition_table = []<std::copy_constructible... Transitions> (Transitions... ts)
  {
    return transition_table<std::variant<States...>, Transitions...> (std::move (ts)...);
  };
} // namespace forest

#pragma once
#include <forest/machine.hpp>
#include <forest/telegram/event.hpp>

namespace forest::telegram
{
  template<machine::MachineTraits Traits, class State>
  class bot_state : public machine::state_interface<Traits, State>
  {
  private:
    using base = machine::state_interface<Traits, State>;

  public:
    template<container::literal_string Prefix, container::literal_string Description>
    using describe_cmd = telegram::command_description<Prefix, Description>;

    template<class... Types>
    using describe_btn = telegram::button_description<Types...>;

    template<CommandDescription T>
    using command_event = telegram::command_event<T>;

    template<ButtonDescription T>
    using button_event = telegram::button_event<T>;

    using message_event = telegram::message_event;

    using message = typename Traits::template context<State>::message;

    template<CommandDescription... Ts>
    using command_list = telegram::command_list<Ts...>;

    template<ButtonDescription... Ts>
    using button_list = telegram::button_list<Ts...>;
  };
} // namespace forest::telegram
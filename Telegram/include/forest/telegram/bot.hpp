#pragma once
#include <forest/machine.hpp>
#include <forest/signal.hpp>
#include <forest/telegram/context.hpp>
#include <forest/telegram/event.hpp>
#include <forest/telegram/state.hpp>
#include <forest/telegram/storage.hpp>

#include <map>
#include <spdlog/spdlog.h>

namespace forest::telegram
{
  template<container::literal_string Text, Driver DriverType = tgbot_driver>
  using describe_machine = machine::describe_machine<Text, bot_storage<DriverType>, bot_context, bot_state>;

  template<machine::MachineTraits Traits>
  class bot
  {
  public:
    using states = typename Traits::list_type;
    using instance = typename Traits::instance;
    static constexpr long num_states = boost::mp11::mp_size<states>::value;
    using storage_type = typename Traits::storage_type;
    using driver_type = typename storage_type::driver_type;
    using driver_reference = typename storage_type::driver_reference;

  private:
    std::map<api_chat_id, instance> _machines;
    driver_reference _driver;

  public:
    bot (driver_reference _driver)
      : _driver (_driver)
    {}

    void set_commands ()
    {
      using commands = boost::mp11::mp_apply<internal::join_commands_t, states>;
      auto list = api_set_commands_action ();

      spdlog::info ("setting commands");
      boost::mp11::mp_for_each<commands> ([&list]<class C> (C) {
        std::string name = std::string (C::name);
        std::string description = std::string (C::description);
        spdlog::debug ("Adding command {}", name);
        list.commands.push_back ({std::move (name), std::move (description)});
      });

      _driver.set_commands (std::move (list));
    }

    template<Event E>
      requires (Traits::template can_react<E>)
    void operator() (E event)
    {
      spdlog::debug ("Handling event of type {} with chat_id {}", meta::type_name<E> (), event.chat_id);
      if (!_machines.contains (event.chat_id)) {
        spdlog::debug ("Creating state machine instance for chat_id {}", event.chat_id);
        _machines.try_emplace (event.chat_id, instance ({_driver, event.chat_id}));
        _machines.at (event.chat_id).start ();
      }
      _machines.at (event.chat_id).react (event);
    }
  };

} // namespace forest::telegram
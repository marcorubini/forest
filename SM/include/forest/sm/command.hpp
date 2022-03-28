#pragma once
#include "tgbot/types/BotCommand.h"
#include <concepts>
#include <string>

#include <hfsm2/machine.hpp>
#include <tgbot/Api.h>

namespace forest
{
  namespace events
  {
    struct command
    {
      std::string prefix;
      std::string parameters;
      std::int64_t id;
    };
  } // namespace events

  template<class T>
  concept Command = requires ()
  {
    // clang-format off
    requires std::default_initializable<T>;
    { T::prefix() } -> std::convertible_to<std::string>;
    { T::description() } -> std::convertible_to<std::string>;
    // clang-format on
  };

  // ===

  template<class Derived, class FSM, Command... Commands>
  class command_handler
  {
  public:
    using command_list = std::tuple<Commands...>;

    template<std::same_as<Derived> D = Derived>
    void react (events::command const& event, typename FSM::FullControl& control)
    {
      static_assert (std::derived_from<Derived, command_handler>);

      bool reacted = false;
      auto const react_to = [&]<Command Current> () {
        if (!reacted) {
          std::string const prefix = Current::prefix ();
          if (prefix == event.prefix) {
            static_cast<D&> (*this).react (Current {}, event, control);
            reacted = true;
          }
        }
      };

      (react_to.template operator()<Commands> (), ...);
    }

    static auto commands () -> std::vector<TgBot::BotCommand::Ptr>
    {
      auto result = std::vector<TgBot::BotCommand::Ptr> ();

      auto push_command = [&result]<class C> () {
        auto cmd = std::make_shared<TgBot::BotCommand> ();
        cmd->command = C::prefix ();
        cmd->description = C::description ();
        result.push_back (std::move (cmd));
      };

      (push_command.template operator()<Commands> (), ...);
      return result;
    }
  };

  template<class T>
  concept CommandHandler = requires ()
  {
    // clang-format off
    { T::commands() } -> std::same_as<std::vector<TgBot::BotCommand::Ptr>>;
    // clang-format on
  };

  template<class... Ts>
  constexpr inline auto collect_commands = [] () {
    auto result = std::vector<TgBot::BotCommand::Ptr> ();

    auto invoke = [&]<class T> () {
      if constexpr (CommandHandler<T>) {
        for (auto cmd : T::commands ())
          result.push_back (std::move (cmd));
      }
    };

    (invoke.template operator()<Ts> (), ...);
    return result;
  };
} // namespace forest
#pragma once
#include <forest/machine.hpp>
#include <forest/telegram/driver.hpp>
#include <forest/telegram/event.hpp>

namespace forest::telegram
{

  // projections

  namespace projections
  {
    template<machine::MachineTraits Machine>
    constexpr inline auto to_command = [] () {
      using boost::mp11::mp_apply;
      using boost::mp11::mp_rename;
      using boost::mp11::mp_transform;
      using boost::mp11::mp_for_each;
      using boost::mp11::mp_size;

      using state_list = typename Machine::list_type;
      using commands = mp_apply<internal::join_commands_t, state_list>;
      using signal_type = mp_rename<mp_transform<command_event, commands>, signal::maybe>;

      constexpr auto split_whitespace = [] (std::string_view text, auto callback) {
        auto pos = text.find (' ');
        while (pos != text.npos) {
          callback (text.substr (0, pos));
          text.remove_prefix (pos);
          text.remove_prefix (text.find_first_not_of (' '));
          pos = text.find (' ');
        }
      };

      return [split_whitespace] (api_new_message_update const& update) -> signal_type {
        if (!update.text.starts_with ("/"))
          return std::nullopt;

        auto const text = std::string_view (update.text).substr (1);
        auto result = signal_type ();
        mp_for_each<commands> ([&]<class C> (C) {
          if (!result.has_value () && text.starts_with (C::name)) {
            auto parameters = std::vector<std::string> ();
            split_whitespace (text, [&] (std::string_view param) {
              parameters.push_back (std::string (param));
            });
            result = command_event<C> {.chat_id = update.chat.id, .parameters = std::move (parameters)};
          }
        });
        return result;
      };
    }();

    template<machine::MachineTraits Machine>
    constexpr inline auto to_button = [] () {
      using boost::mp11::mp_apply;
      using boost::mp11::mp_rename;
      using boost::mp11::mp_transform;
      using boost::mp11::mp_with_index;
      using boost::mp11::mp_size;

      using state_list = typename Machine::list_type;
      using buttons = mp_apply<internal::join_buttons_t, state_list>;
      constexpr long num_buttons = mp_size<buttons>::value;

      if constexpr (num_buttons == 0) {
        return [] (api_callback_query_update const&) -> signal::nothing {
          return {};
        };
      } else {
        using button_events = mp_transform<button_event, buttons>;
        using signal_type = mp_rename<mp_transform<button_event, buttons>, signal::maybe>;

        return [] (api_callback_query_update const& update) -> signal_type {
          spdlog::debug ("Try to parse callback as btn, payload = {}", update.data);

          if (update.data.empty ())
            return std::nullopt;

          return internal::unserialize_button<Machine> (update.data, [&]<class B> (B btn, auto payload) {
            spdlog::debug ("Detected callback {}", meta::type_name<B> ());

            if constexpr (std::same_as<B, std::monostate>) {
              return signal_type (std::nullopt);
            } else {
              auto evt = button_event<B> ();
              evt.chat_id = update.message.chat.id;
              evt.message_id = update.message.id;
              evt.callback_id = update.id;
              evt.payload = std::move (payload);
              return signal_type (std::move (evt));
            }
          });
        };
      }
    }();

    constexpr inline auto to_message = [] (api_new_message_update const& message) -> signal::just<message_event> {
      return message_event {.chat_id = message.chat.id, .message_id = message.id, .text = message.text};
    };

    template<machine::MachineTraits Machine>
    constexpr inline auto to_telegram_event = [] {
      constexpr auto log_message = [] (api_new_message_update const& update) -> signal::nothing {
        spdlog::debug ("new message pipeline: {}", update.text);
        return {};
      };

      constexpr auto new_message_pipeline = signal::or_else ( //
        log_message,
        to_command<Machine>,
        to_message);

      constexpr auto edit_message_pipeline = [] (api_edit_message_update const&) -> signal::nothing {
        return {};
      };

      constexpr auto callback_pipeline = to_button<Machine>;

      return signal::overload (new_message_pipeline, edit_message_pipeline, callback_pipeline);
    }();

  } // namespace projections

} // namespace forest::telegram
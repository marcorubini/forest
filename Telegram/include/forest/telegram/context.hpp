#pragma once
#include <forest/machine.hpp>
#include <forest/telegram/driver.hpp>
#include <forest/telegram/event.hpp>
#include <variant>

namespace forest::telegram
{
  template<machine::MachineTraits Traits, class State>
  struct bot_context : public machine::context_interface<Traits, State>
  {
    using base = machine::context_interface<Traits, State>;
    using base::base;
    using storage_type = typename Traits::storage_type;
    using driver_type = typename storage_type::driver_type;
    using driver_reference = driver_type&;

    struct message
    {
    public:
      std::string text;
      std::optional<api_message_id> reply_to {};
      api_reply_markup reply_markup = api_generic_reply_markup ();
      api_parse_mode parse_mode = api_parse_mode::MARKDOWN;
      bool disable_notification = false;

      template<ButtonDescription Btn>
      void add_button (int row, std::string text, typename Btn::payload_type payload = {}) &
      {
        auto& btn = add_button_helper (row);
        btn.text = std::move (text);
        btn.callback_data = internal::serialize_button<Traits, Btn> (std::move (payload));
      }

    private:
      auto add_button_helper (int row) & -> api_button&
      {
        if (!std::holds_alternative<api_inline_keyboard_reply_markup> (reply_markup))
          reply_markup = api_inline_keyboard_reply_markup ();

        auto& markup = std::get<api_inline_keyboard_reply_markup> (reply_markup);
        while (static_cast<int> (markup.buttons.size ()) <= row)
          markup.buttons.emplace_back ();

        markup.buttons[row].push_back (api_button {});
        return markup.buttons[row].back ();
      }
    };

    // telegram observers

    [[nodiscard]] constexpr auto driver () const -> driver_reference
    {
      return base::storage ().driver ();
    }

    [[nodiscard]] constexpr auto chat_id () const -> api_chat_id
    {
      return base::storage ().chat_id ();
    }

    // actions

    auto send_message (message msg) const -> std::optional<api_message_id>
    {
      spdlog::debug ("Context of chat_id {} is sending message with text '{}'", chat_id (), msg.text);

      auto query = api_send_message_action {.chat_id = chat_id (), //
        .text = std::move (msg.text),
        .reply_to = std::move (msg.reply_to),
        .reply_markup = std::move (msg.reply_markup),
        .parse_mode = std::move (msg.parse_mode),
        .disable_notification = std::move (msg.disable_notification)};
      return driver ().send_message (std::move (query));
    }

    auto edit_message_text (api_message_id id, message msg) const -> std::optional<api_message_id>
    {
      auto query = api_edit_message_text_action {.message_id = id, //
        .new_text = std::move (msg.text),
        .chat_id = chat_id (),
        .new_mode = std::move (msg.parse_mode)};
      return driver ().edit_message_text (std::move (query));
    }

    auto edit_message_markup (api_message_id id, message msg) const -> std::optional<api_message_id>
    {
      auto query = api_edit_message_reply_markup_action {.message_id = id, //
        .chat_id = chat_id (),
        .new_markup = std::move (msg.reply_markup)};
      return driver ().edit_message_markup (std::move (query));
    }

    auto edit_message (api_message_id id, message msg) const -> std::optional<api_message_id>
    {
      auto query = api_edit_message_action {.message_id = id, //
        .chat_id = chat_id (),
        .new_text = std::move (msg.text),
        .new_markup = std::move (msg.reply_markup),
        .new_mode = std::move (msg.parse_mode)};
      return driver ().edit_message_markup (std::move (query));
    }

    auto delete_message (api_message_id message_id) const -> void
    {
      return driver ().delete_message ({chat_id (), message_id});
    }

    auto answer_callback (api_answer_callback_action action) const -> bool
    {
      return driver ().answer_callback (std::move (action));
    }
  };
} // namespace forest::telegram
#pragma once
#include <cassert>
#include <iostream>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <variant>

#include <forest/meta.hpp>
#include <forest/signal.hpp>
#include <spdlog/spdlog.h>
#include <tgbot/Api.h>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>

namespace forest::telegram
{

  // --- telegram ids

  enum class api_chat_id : std::int64_t
  {};

  enum class api_message_id : std::int32_t
  {};

  enum class api_update_id : std::int32_t
  {};

  enum class api_user_id : std::int64_t
  {};

  enum class api_parse_mode : int
  {
    MARKDOWN,
    HTML
  };

  enum class api_chat_type : int
  {
    PRIVATE,
    GROUP,
    SUPERGROUP,
    CHANNEL
  };

  // === basic entities ===
  // ======================

  /** @brief api_user
   */
  struct api_user
  {
    api_user_id id;
    bool is_bot;
    std::string first_name;
    std::string last_name;
    std::string username;
    std::string language_code;
  };

  /** @brief api_file_certificate
   */
  struct api_file_certificate
  {
    std::string filename;
    std::string mime;
  };

  /** @brief api_embedded_certificate
   */
  struct api_embedded_certificate
  {
    std::string data;
  };

  /** @brief api_certificate
   */
  using api_certificate = std::variant<api_file_certificate, api_embedded_certificate>;

  /** @brief api_chat
   */
  struct api_chat
  {
    api_chat_id id;
    api_chat_type type;
    std::string title;
    std::string username;
    std::string first_name;
    std::string last_name;
    std::string description;
  };

  /** @brief api_button
   */
  struct api_button
  {
    std::string text;
    std::string url;
    std::string callback_data;
  };

  /** @brief api_command
   */
  struct api_command
  {
    std::string prefix;
    std::string description;
  };

  // === compound entities ===
  // =========================

  /** @brief api_generic_reply_markup
   */
  struct api_generic_reply_markup
  {};

  /** @brief api_force_reply_markup
   */
  struct api_force_reply_markup
  {
    bool selective = false;
  };

  /** @brief api_inline_keyboard_reply_markup
   */
  struct api_inline_keyboard_reply_markup
  {
    std::vector<std::vector<api_button>> buttons;
  };

  /** @brief api_reply_markup
   */
  using api_reply_markup =
    std::variant<api_generic_reply_markup, api_force_reply_markup, api_inline_keyboard_reply_markup>;

  /** @brief api_message
   */
  struct api_message
  {
    /** @brief id the message id */
    api_message_id id;
    /** @brief from the user that sent the message. Can be empty in channels. */
    std::optional<api_user> from;
    /** @brief sent_unix_time the date the message was sent in unix time. */
    std::int32_t sent_unix_time;
    /** @brief edit_unix_time the date the message was modified in unix time. */
    std::int32_t edit_unix_time;
    /** @brief chat the chat the message belongs to. */
    api_chat chat;
    /** @brief text the message text */
    std::string text;

    struct reply_type
    {
      /** @brief id the replied-to message id. */
      api_message_id id;
      /** @brief from the user that send the replied-to message. Can be empty in channels. */
      std::optional<api_user> from;
      /** @brief sent_unix_time the date the replied-to message was sent in unix time. */
      std::int32_t sent_unix_time;
      /** @brief edit_unix_time the date the replied-to message was modified in unix time. */
      std::int32_t edit_unix_time;
      /** @brief chat the chat the replied-to message belongs to. */
      api_chat chat;
      /** @brief text the replied-to message text. */
      std::string text;
    };

    /** @brief reply_to the message that was replied to. Does not allow deep iteration of replies. */
    std::optional<reply_type> reply_to;
  };

  /** @brief api_callback_query
   */
  struct api_callback_query
  {
    /** @brief id unique identifier of this query. */
    std::string id;
    /** @brief user the sender. */
    api_user user;
    /** @brief message the message with the button that fired the query. */
    api_message message;
    /** @brief data the payload of the callback button. */
    std::string data;
  };

  // === actions ===
  // ===============

  /** @brief api_send_message
   */
  struct api_send_message_action
  {
    api_chat_id chat_id;
    std::string text {};
    std::optional<api_message_id> reply_to {};
    api_reply_markup reply_markup = api_generic_reply_markup {};
    api_parse_mode parse_mode = api_parse_mode::MARKDOWN;
    bool disable_notification = false;
  };

  /** @brief api_edit_message_text
   */
  struct api_edit_message_text_action
  {
    api_chat_id chat_id;
    api_message_id message_id;
    std::string new_text {};
    api_parse_mode new_mode = api_parse_mode::MARKDOWN;
  };

  /** @brief api_edit_message_reply_markup
   */
  struct api_edit_message_reply_markup_action
  {
    api_chat_id chat_id;
    api_message_id message_id;
    api_reply_markup new_markup;
  };

  /** @brief api_edit_message
   */
  struct api_edit_message_action
  {
    api_chat_id chat_id;
    api_message_id message_id;
    std::string new_text {};
    api_parse_mode new_mode = api_parse_mode::MARKDOWN;
    api_reply_markup new_markup = api_generic_reply_markup {};
  };

  /** @brief api_answer_callback
   */
  struct api_answer_callback_action
  {
    std::string callback_id;
    std::string text;
    bool show_alert = false;
  };

  /** @brief api_delete_message
   */
  struct api_delete_message_action
  {
    api_chat_id chat_id;
    api_message_id message_id;
  };

  /** @brief api_set_webhook_action
   */
  struct api_set_webhook_action
  {
    std::string url;
    std::optional<api_certificate> certificate;
    int max_connections = 40;
  };

  // === updates ===
  // ===============

  /** @brief api_new_message_update
   */
  struct api_new_message_update : api_message
  {};

  /** @brief api_edit_message_update
   */
  struct api_edit_message_update : api_message
  {};

  /** @brief api_callback_query_update
   */
  struct api_callback_query_update : api_callback_query
  {};

  /** @brief api_update
   */
  using api_update = signal::either<api_new_message_update, //
    api_edit_message_update,
    api_callback_query_update>;

  // === commands ===
  // ================

  struct api_set_commands_action
  {
    std::vector<api_command> commands;
  };

  template<class T>
  concept Driver = requires (T& bot)
  {
    // clang-format off
    requires requires (api_send_message_action message)
    {
      { bot.send_message(message) } -> std::same_as<std::optional<api_message_id>>;
    };

    requires requires (api_delete_message_action message)
    {
      { bot.delete_message(message) } -> std::same_as<void>;
    };

    requires requires (api_answer_callback_action callback)
    {
      { bot.answer_callback(callback) } -> std::same_as<bool>;
    };

    requires requires (api_edit_message_action edit1, api_edit_message_text_action edit2, api_edit_message_reply_markup_action edit3)
    {
      { bot.edit_message(edit1) } -> std::same_as<std::optional<api_message_id>>;
      { bot.edit_message(edit2) } -> std::same_as<std::optional<api_message_id>>;
      { bot.edit_message(edit3) } -> std::same_as<std::optional<api_message_id>>;
    };

    { bot() } -> std::same_as<api_update>;
    { bot.next() } -> std::same_as<api_update>;
    { bot.poll() } -> std::same_as<int>;
    { bot.has_updates() } -> std::same_as<bool>;

    requires requires (api_set_webhook_action webhook)
    {
      { bot.set_webhook(webhook) } -> std::same_as<void>;
      { bot.delete_webhook() } -> std::same_as<bool>;
    };

    requires requires (api_set_commands_action commands)
    {
      { bot.set_commands(commands) } -> std::same_as<bool>;
    };
    // clang-format on
  };

  struct tgbot_driver
  {
  private:
    TgBot::Bot _tgdriver;
    TgBot::TgLongPoll _poller;
    std::queue<api_update> _updates;

  private:
    [[nodiscard]] auto cast_markup (api_reply_markup const& markup) -> TgBot::GenericReply::Ptr
    {
      auto const visit_generic_rm = [&] (api_generic_reply_markup const&) {
        return std::make_shared<TgBot::GenericReply> ();
      };
      auto const visit_inline_keyboard_rm = [&] (api_inline_keyboard_reply_markup const& rm) {
        auto tgbot_rm = TgBot::InlineKeyboardMarkup ();
        for (auto row : rm.buttons) {
          tgbot_rm.inlineKeyboard.emplace_back ();
          auto& last_row = tgbot_rm.inlineKeyboard.back ();
          for (auto col : row) {
            auto btn = TgBot::InlineKeyboardButton ();
            btn.text = col.text;
            btn.url = col.url;
            btn.callbackData = col.callback_data;
            last_row.push_back (std::make_shared<TgBot::InlineKeyboardButton> (btn));
          }
        }
        return std::make_shared<TgBot::InlineKeyboardMarkup> (std::move (tgbot_rm));
      };
      auto const visit_force_reply_rm = [&] (api_force_reply_markup const& rm) {
        auto tgbot_rm = TgBot::ForceReply {};
        tgbot_rm.selective = rm.selective;
        return std::make_shared<TgBot::ForceReply> (tgbot_rm);
      };
      auto const visitor = meta::overloaded (visit_generic_rm, visit_inline_keyboard_rm, visit_force_reply_rm);
      return std::visit<TgBot::GenericReply::Ptr> (visitor, markup);
    }

    [[nodiscard]] auto cast_parse_mode (api_parse_mode mode) -> std::string
    {
      switch (mode) {
      case api_parse_mode::MARKDOWN:
        return "MARKDOWN";
      case api_parse_mode::HTML:
        return "HTML";
      default:
        assert (mode == api_parse_mode::MARKDOWN || mode == api_parse_mode::HTML);
        HEDLEY_UNREACHABLE_RETURN ("?");
      }
    }

    [[nodiscard]] auto cast_chat (TgBot::Chat const& chat) -> api_chat
    {
      auto result = api_chat ();
      result.description = chat.description;
      result.first_name = chat.firstName;
      result.last_name = chat.lastName;
      result.username = chat.username;
      result.id = static_cast<api_chat_id> (chat.id);
      result.title = chat.title;

      using enum TgBot::Chat::Type;

      switch (chat.type) {
      case Channel:
        result.type = api_chat_type::CHANNEL;
        break;
      case Group:
        result.type = api_chat_type::GROUP;
        break;
      case Supergroup:
        result.type = api_chat_type::SUPERGROUP;
        break;
      case Private:
        result.type = api_chat_type::PRIVATE;
        break;
      default:
        assert (chat.type == Channel || chat.type == Group || chat.type == Supergroup || chat.type == Private);
        break;
      }

      return result;
    }

    [[nodiscard]] auto cast_user (TgBot::User const& user) -> api_user
    {
      auto result = api_user ();
      result.username = user.username;
      result.first_name = user.firstName;
      result.last_name = user.lastName;
      result.id = static_cast<api_user_id> (user.id);
      result.is_bot = user.isBot;
      result.language_code = user.languageCode;
      return result;
    }

    [[nodiscard]] auto cast_message_reply (TgBot::Message const& message) -> api_message::reply_type
    {
      auto result = api_message::reply_type ();
      assert (message.chat);
      result.chat = cast_chat (*message.chat);
      result.id = static_cast<api_message_id> (message.messageId);
      result.sent_unix_time = message.date;
      result.edit_unix_time = message.editDate;
      result.text = message.text;
      if (message.from)
        result.from = cast_user (*message.from);
      return result;
    }

    [[nodiscard]] auto cast_message (TgBot::Message const& message) -> api_message
    {
      auto result = api_message ();
      result.id = static_cast<api_message_id> (message.messageId);
      assert (message.chat);
      result.chat = cast_chat (*message.chat);
      if (message.from)
        result.from = cast_user (*message.from);
      result.sent_unix_time = message.date;
      result.edit_unix_time = message.editDate;
      result.text = message.text;
      if (message.replyToMessage)
        result.reply_to = cast_message_reply (*message.replyToMessage);
      return result;
    }

    [[nodiscard]] auto cast_callback (TgBot::CallbackQuery const& query) -> api_callback_query
    {
      auto result = api_callback_query ();
      result.id = query.id;
      result.data = query.data;
      assert (query.from);
      result.user = cast_user (*query.from);
      assert (query.message);
      result.message = cast_message (*query.message);
      return result;
    }

    [[nodiscard]] auto cast_certificate (api_certificate const certificate) -> TgBot::InputFile::Ptr
    {
      auto const visit_embedded = [] (api_embedded_certificate const& cert) {
        auto in = TgBot::InputFile ();
        in.fileName = "";
        in.mimeType = "";
        in.data = cert.data;
        return std::make_shared<TgBot::InputFile> (in);
      };
      auto const visit_file = [] (api_file_certificate const& cert) {
        auto in = TgBot::InputFile::fromFile (cert.filename, cert.mime);
        return in;
      };
      return std::visit (meta::overloaded (visit_embedded, visit_file), certificate);
    }

    [[nodiscard]] auto cast_command (api_command const cmd) -> TgBot::BotCommand::Ptr
    {
      auto tg_cmd = TgBot::BotCommand ();
      tg_cmd.command = cmd.prefix;
      tg_cmd.description = cmd.description;
      return std::make_shared<TgBot::BotCommand> (tg_cmd);
    }

  public:
    tgbot_driver (std::string const& api_key) //
      : _tgdriver (api_key)
      , _poller (_tgdriver)
    {
      auto const on_message = [this] (TgBot::Message::Ptr message) {
        assert (message);
        if (message->editDate != 0) {
          spdlog::debug ("Driver received message edit.");
          _updates.push (api_edit_message_update {cast_message (*message)});
        } else {
          spdlog::debug ("Driver received new message.");
          _updates.push (api_new_message_update {cast_message (*message)});
        }
      };
      auto const on_callback = [this] (TgBot::CallbackQuery::Ptr query) {
        assert (query);
        spdlog::debug ("Driver received new callback.");
        _updates.push (api_callback_query_update {cast_callback (*query)});
      };
      _tgdriver.getEvents ().onAnyMessage (on_message);
      _tgdriver.getEvents ().onCallbackQuery (on_callback);
    }

    tgbot_driver (tgbot_driver&&) = delete;

    auto poll () -> int
    {
      if (_updates.empty ()) {
        spdlog::debug ("Polling...");
        _poller.start ();
        spdlog::debug ("Polling ended with {} updates", _updates.size ());
      }
      return _updates.size ();
    }

    [[nodiscard]] bool has_updates () const
    {
      return !_updates.empty ();
    }

    [[nodiscard]] auto operator() () -> api_update
    {
      return next ();
    }

    [[nodiscard]] auto next () -> api_update
    {
      if (_updates.empty ())
        throw std::runtime_error ("update queue is empty.");
      auto first = _updates.front ();
      _updates.pop ();
      return first;
    }

    auto send_message (api_send_message_action const message) -> std::optional<api_message_id>
    {
      spdlog::debug ("Driver sending message with text '{}'", message.text);
      auto const reply_to = message.reply_to.has_value () ? *message.reply_to : api_message_id {0};
      auto const reply_markup = cast_markup (message.reply_markup);
      auto const parse_mode = cast_parse_mode (message.parse_mode);

      if (std::holds_alternative<api_inline_keyboard_reply_markup> (message.reply_markup)) {
        auto& markup = std::get<api_inline_keyboard_reply_markup> (message.reply_markup);
        if (markup.buttons.size () > 0) {
          spdlog::debug ("Message has {} button rows.", markup.buttons.size ());
          for (std::size_t i = 0; i < markup.buttons.size (); ++i) {
            spdlog::debug ("Message row {} has {} columns", i, markup.buttons[i].size ());
            for (std::size_t j = 0; j < markup.buttons[i].size (); ++j) {
              auto btn = markup.buttons[i][j];
              spdlog::debug ("Button {} {} has text {} and payload {}", i, j, btn.text, btn.callback_data);
            }
          }
        }
      }

      auto result = _tgdriver.getApi ().sendMessage ( //
        static_cast<std::int64_t> (message.chat_id),
        message.text,
        false,
        static_cast<std::int32_t> (reply_to),
        reply_markup,
        parse_mode,
        message.disable_notification);
      if (result)
        return static_cast<api_message_id> (result->messageId);
      return std::nullopt;
    }

    auto delete_message (api_delete_message_action const message) -> void
    {
      return _tgdriver.getApi ().deleteMessage ( //
        static_cast<std::int64_t> (message.chat_id),
        static_cast<std::int32_t> (message.message_id));
    }

    auto answer_callback (api_answer_callback_action const answer) -> bool
    {
      return _tgdriver.getApi ().answerCallbackQuery ( //
        answer.callback_id,
        answer.text,
        answer.show_alert);
    }

    auto edit_message (api_edit_message_text_action const edit) -> std::optional<api_message_id>
    {
      auto result = _tgdriver.getApi ().editMessageText (edit.new_text, //
        static_cast<std::int64_t> (edit.chat_id),
        static_cast<std::int32_t> (edit.message_id),
        "",
        cast_parse_mode (edit.new_mode));
      if (result)
        return static_cast<api_message_id> (result->messageId);
      return std::nullopt;
    }

    auto edit_message (api_edit_message_reply_markup_action const edit) -> std::optional<api_message_id>
    {
      auto result = _tgdriver.getApi ().editMessageReplyMarkup ( //
        static_cast<std::int64_t> (edit.chat_id),
        static_cast<std::int32_t> (edit.message_id),
        "",
        cast_markup (edit.new_markup));
      if (result)
        return static_cast<api_message_id> (result->messageId);
      return std::nullopt;
    }

    auto edit_message (api_edit_message_action const edit) -> std::optional<api_message_id>
    {
      auto result = _tgdriver.getApi ().editMessageText (edit.new_text, //
        static_cast<std::int64_t> (edit.chat_id),
        static_cast<std::int32_t> (edit.message_id),
        "",
        cast_parse_mode (edit.new_mode),
        false,
        cast_markup (edit.new_markup));
      if (result)
        return static_cast<api_message_id> (result->messageId);
      return std::nullopt;
    }

    auto set_webhook (api_set_webhook_action const action) -> void
    {
      auto certificate = action.certificate ? cast_certificate (*action.certificate) : nullptr;
      _tgdriver.getApi ().setWebhook (action.url, certificate, action.max_connections);
    }

    auto delete_webhook () -> bool
    {
      return _tgdriver.getApi ().deleteWebhook ();
    }

    auto set_commands (api_set_commands_action const action) -> bool
    {
      auto tg_list = std::vector<TgBot::BotCommand::Ptr> ();
      for (auto cmd : action.commands)
        tg_list.push_back (cast_command (std::move (cmd)));
      return _tgdriver.getApi ().setMyCommands (std::move (tg_list));
    }
  };
} // namespace forest::telegram
#pragma once
#include <concepts>
#include <string>

#include <tgbot/Api.h>

namespace forest
{
  namespace events
  {
    struct button
    {
      int index;
      std::string name;
      std::string payload;
      std::string text;
      std::string callback_id;
      std::int64_t message_id;

      button (int index, std::string name, std::string payload, std::string text, std::string callback_id, std::int64_t message_id)
        : index (index)
        , name (std::move (name))
        , payload (std::move (payload))
        , text (std::move (text))
        , callback_id (std::move (callback_id))
        , message_id (message_id)
      {}

      static button from_query (TgBot::CallbackQuery const& query)
      {
        auto stream = std::istringstream (query.data);

        std::string str_index;
        std::string str_name;
        std::string str_payload;
        std::getline (stream, str_index, '|');
        std::getline (stream, str_name, '|');
        std::getline (stream, str_payload, '\n');

        std::string text = query.message ? query.message->text : "";
        std::string id = query.id;
        std::int64_t message_id = query.message ? query.message->messageId : -1;

        try {
          return button {std::stoi (str_index), //
            std::move (str_name),
            std::move (str_payload),
            std::move (text),
            std::move (id),
            message_id};
        } catch (...) {
          return button {-1, //
            std::move (str_name),
            std::move (str_payload),
            std::move (text),
            std::move (id),
            message_id};
        }
      }
    };
  } // namespace events

  template<class T>
  concept Button = requires (int index, std::string text, std::string payload)
  {
    // clang-format off
    requires std::default_initializable<T>;
    { T::mangle(index, payload) } -> std::convertible_to<std::string>;
    { T::mangle(payload) } -> std::convertible_to<std::string>;
    { T::to_bot_button(index, text, payload) } -> std::same_as<TgBot::InlineKeyboardButton>;
    { T::to_bot_button(text, payload) } -> std::same_as<TgBot::InlineKeyboardButton>;
    // clang-format on
  };

  // ===

  template<class Derived>
  class button
  {
  public:
    static auto mangle (int index, std::string payload)
    {
      return std::to_string (index) + "|" + typeid (Derived).name () + "|" + payload;
    }

    static auto mangle (std::string payload)
    {
      return mangle (-1, std::move (payload));
    }

    static auto to_bot_button (int index, std::string text, std::string payload)
    {
      return TgBot::InlineKeyboardButton {.text = std::move (text), .callbackData = mangle (index, std::move (payload))};
    }

    static auto to_bot_button (std::string text, std::string payload)
    {
      return TgBot::InlineKeyboardButton {.text = std::move (text), .callbackData = mangle (std::move (payload))};
    }
  };

} // namespace forest
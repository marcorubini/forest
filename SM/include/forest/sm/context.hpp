#pragma once
#include <concepts>
#include <type_traits>

#include <SQLiteCpp/SQLiteCpp.h>
#include <nlohmann/json.hpp>
#include <tgbot/tgbot.h>

namespace forest
{
  struct any_ref
  {
  private:
    void* _ref;
    std::type_index _type;

  public:
    template<class T>
    constexpr any_ref (T& init)
      : _ref (std::addressof (init))
      , _type (typeid (T))
    {}

    template<class T>
    any_ref (T const&) = delete;

    template<class T>
    T& get ()
    {
      if (typeid (T) != _type)
        throw std::bad_cast ();
      return *static_cast<T*> (_ref);
    }

    template<class T>
    void rebind (T& init)
    {
      _ref = std::addressof (init);
      _type = typeid (T);
    }

    template<class T>
    void rebind (T const&) = delete;
  };

  struct any_cref
  {
  private:
    void const* _ref;
    std::type_index _type;

  public:
    template<class T>
    constexpr any_cref (T const& init)
      : _ref (std::addressof (init))
      , _type (typeid (T))
    {}

    template<class T>
    T const& get ()
    {
      if (typeid (T) != _type)
        throw std::bad_cast ();
      return *static_cast<T const*> (_ref);
    }

    template<class T>
    void rebind (T const& init)
    {
      _ref = std::addressof (init);
      _type = typeid (init);
    }

    template<class T>
    void rebind (T const&&) = delete;
  };

  struct bot_context
  {
  private:
    std::int64_t _chatid;
    TgBot::Bot& _bot;
    SQLite::Database& _database;
    any_ref _global_storage;

  public:
    using markup_type = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>>;

    template<class T>
    bot_context (std::int64_t chatid, TgBot::Bot& bot, SQLite::Database& database, T& _global)
      : _chatid (chatid)
      , _bot (bot)
      , _database (database)
      , _global_storage (_global)
    {}

    auto chat_id () const -> std::int64_t
    {
      return _chatid;
    }

    auto bot () const -> TgBot::Bot&
    {
      return _bot;
    }

    auto database () const -> SQLite::Database&
    {
      return _database;
    }

    template<class T>
    T& global_storage () const
    {
      return _global_storage.get<T> ();
    }

    // ===

    auto send_message (std::string const& text) const -> TgBot::Message::Ptr
    {
      return _bot.getApi ().sendMessage (chat_id (), text);
    }

    auto send_message (std::string const& text, markup_type const& markup) -> TgBot::Message::Ptr
    {
      auto tgmarkup = std::make_shared<TgBot::InlineKeyboardMarkup> ();
      tgmarkup->inlineKeyboard = markup;
      return _bot.getApi ().sendMessage (chat_id (), text, false, 0, tgmarkup);
    }

    auto delete_message (std::int64_t id) const -> void
    {
      _bot.getApi ().deleteMessage (_chatid, id);
    }

    auto edit_message (std::int64_t id, std::string const& text, markup_type const& markup) -> TgBot::Message::Ptr
    {
      auto tgmarkup = std::make_shared<TgBot::InlineKeyboardMarkup> ();
      tgmarkup->inlineKeyboard = markup;
      return _bot.getApi ().editMessageText (text, _chatid, id, "", "", false, tgmarkup);
    }

    auto edit_message (std::int64_t id, std::string const& text) -> TgBot::Message::Ptr
    {
      return _bot.getApi ().editMessageText (text, _chatid, id);
    }

    auto edit_message (std::int64_t id, markup_type const& markup) -> TgBot::Message::Ptr
    {
      auto tgmarkup = std::make_shared<TgBot::InlineKeyboardMarkup> ();
      tgmarkup->inlineKeyboard = markup;
      return _bot.getApi ().editMessageReplyMarkup (_chatid, id, "", tgmarkup);
    }

    // ===

    auto answer_callback (std::string const& id, std::string const& text, bool alert = false) -> void
    {
      _bot.getApi ().answerCallbackQuery (id, text, alert);
    }

    // === database

    int set_value (std::string key, std::string value) const
    {
      static constexpr auto str_set_key =
        "INSERT OR REPLACE INTO sessions(chat_id,kName,kValue) VALUES (?,?,?)";

      auto stm = SQLite::Statement (_database, str_set_key);
      stm.bind (1, _chatid);
      stm.bind (2, key);
      stm.bind (3, value);
      return stm.exec ();
    }

    int set_value (std::string key, nlohmann::json json) const
    {
      return set_value (key, json.dump ());
    }

    std::optional<std::string> get_value (std::string key) const
    {
      static constexpr auto str_set_value = "SELECT kValue FROM sessions WHERE chat_id=? AND kName=?";
      auto stm = SQLite::Statement (_database, str_set_value);
      stm.bind (1, _chatid);
      stm.bind (2, key);
      if (stm.executeStep ()) {
        return stm.getColumn (0).getString ();
      } else {
        return std::nullopt;
      }
    }

    std::optional<nlohmann::json> get_value_json (std::string key) const
    {
      auto result = get_value (key);
      if (!result)
        return std::nullopt;
      return nlohmann::json::parse (*result);
    }
  };

} // namespace forest
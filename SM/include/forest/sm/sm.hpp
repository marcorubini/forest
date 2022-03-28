#pragma once
#include <chrono>
#include <concepts>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>
#include <variant>

#include <SQLiteCpp/SQLiteCpp.h>
#include <hfsm2/machine.hpp>
#include <nlohmann/json.hpp>
#include <tgbot/tgbot.h>

#include <forest/sm/button.hpp>
#include <forest/sm/command.hpp>
#include <forest/sm/context.hpp>
#include <forest/sm/inline_query.hpp>
#include <forest/sm/keyboard.hpp>
#include <forest/sm/message.hpp>

namespace forest
{
  using Machine = hfsm2::MachineT< //
    hfsm2::Config::ContextT<bot_context>>;

  // ===

  template<class FSM, std::default_initializable Storage = std::monostate>
  class bot
  {
  public:
    using fsm_type = FSM;
    using instance_type = typename FSM::Instance;
    using storage_type = Storage;

  private:
    TgBot::Bot _bot;
    SQLite::Database _database;
    std::map<std::int64_t, storage_type> _global_storage;
    std::map<std::int64_t, bot_context> _context;
    std::map<std::int64_t, instance_type> _instances;

    static SQLite::Database create_db (std::string filename)
    {
      SQLite::Database db (std::move (filename), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

      static constexpr auto create_stm = "CREATE TABLE IF NOT EXISTS\n"
                                         "sessions\n"
                                         "(\n"
                                         "chat_id INTEGER,\n"
                                         "kName TEXT not null,\n"
                                         "kValue TEXT null,\n"
                                         "PRIMARY KEY (chat_id, kName)\n"
                                         ")";
      db.exec (create_stm);
      return db;
    }

  public:
    bot (std::string apikey, std::string filename)
      : _bot (std::move (apikey))
      , _database (create_db (std::move (filename)))
    {
      // set commands
      using StateList = typename FSM::StateList;

      auto const set_commands = [&]<class... States, template<class...> class List> (List<States...>)
      {
        auto commands = collect_commands<States...> ();
        std::cerr << "Set " << commands.size () << " commands: " << _bot.getApi ().setMyCommands (commands) << std::endl;
      };

      set_commands (StateList {});

      // set event handlers
      _bot.getEvents ().onAnyMessage ([&] (TgBot::Message::Ptr message) {
        process_event (*message.get ());
      });

      _bot.getEvents ().onCallbackQuery ([&] (TgBot::CallbackQuery::Ptr query) {
        process_event (*query.get ());
      });

      _bot.getEvents ().onInlineQuery ([&] (TgBot::InlineQuery::Ptr query) {
        process_event (*query.get ());
      });

      // delete webhook to allow polling
      _bot.getApi ().deleteWebhook ();
    }

    void process_event (TgBot::Message const& event)
    {
      if (event.chat && event.text.starts_with ("/")) {
        std::int64_t chat_id = event.chat->id;
        std::string text = event.text;
        process_event (chat_id, parse_command (std::move (text), event.messageId));
      } else if (event.chat) {
        std::int64_t chat_id = event.chat->id;
        std::string text = event.text;
        process_event (chat_id, forest::events::message {chat_id, text});
      }
    }

    void process_event (TgBot::CallbackQuery const& event)
    {
      if (event.message && event.message->chat) {
        std::int64_t chat_id = event.message->chat->id;
        process_event (chat_id, parse_button (event));
      }
    }

    void process_event (TgBot::InlineQuery const& event)
    {
      std::cerr << "process_event InlineQuery " << event.id << ", " << event.query << std::endl;
    }

    void process_event (std::int64_t chat_id, auto const& event)
    {
      if (!_instances.contains (chat_id)) {
        _global_storage.try_emplace (chat_id);
        _context.try_emplace (chat_id, chat_id, _bot, _database, _global_storage.at (chat_id));
        _instances.try_emplace (chat_id, _context.at (chat_id));
      }
      _instances.at (chat_id).update ();
      _instances.at (chat_id).react (event);
    }

    auto chat_ids () const -> std::vector<std::int64_t>
    {
      auto result = std::vector<std::int64_t> ();
      for (auto& x : _context)
        result.push_back (x.first);
      return result;
    }

    TgBot::Bot& get_bot ()
    {
      return _bot;
    }

  private:
    events::command parse_command (std::string text, std::int64_t id)
    {
      text = text.substr (1);

      int space_pos = text.find_first_of (" ");
      if (space_pos == std::string::npos)
        space_pos = text.length ();

      std::string prefix = text.substr (0, space_pos);

      int not_space_pos = text.find_first_not_of (" \t", space_pos);
      if (not_space_pos == std::string::npos)
        not_space_pos = text.length ();

      std::string parameters = text.substr (not_space_pos);
      return {.prefix = std::move (prefix), .parameters = std::move (parameters), .id = id};
    }

    events::button parse_button (TgBot::CallbackQuery const& callback)
    {
      return events::button::from_query (callback);
    }
  };

  // ===

  template<class Bot>
  class long_poll_bot
  {
  private:
    TgBot::TgLongPoll _poll_bot;

  public:
    long_poll_bot (Bot& _bot)
      : _poll_bot (_bot.get_bot ())
    {}

    void start ()
    {
      _poll_bot.start ();
    }
  };

  template<class Bot>
  long_poll_bot (Bot&) -> long_poll_bot<Bot>;

  // ===

  template<class Bot>
  class webhook_bot
  {
  private:
    TgBot::TgWebhookTcpServer _webhook_bot;

  public:
    webhook_bot (std::string url, unsigned port, Bot& _bot)
      : _webhook_bot (port, _bot.get_bot ())
    {
      _bot.get_bot ().getApi ().setWebhook (url);
    }

    void start ()
    {
      _webhook_bot.start ();
    }
  };

  template<class Bot>
  webhook_bot (Bot) -> webhook_bot<Bot>;

} // namespace forest
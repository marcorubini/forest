#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <banana/api.hpp>
#include <cassert>
#include <mutex>
#include <nlohmann/json.hpp>

namespace forest
{
  class persistence
  {
  private:
    SQLite::Database db;
    SQLite::Statement stm_get_key;
    SQLite::Statement stm_set_key;
    SQLite::Statement stm_del_key;
    std::mutex mutex;

    static constexpr auto str_get_key = "SELECT kValue FROM sessions WHERE chat_id=? AND kName=?";
    static constexpr auto str_set_key =
      "INSERT OR REPLACE INTO sessions(chat_id,kName,kValue) VALUES (?,?,?)";
    static constexpr auto str_del_key = "DELETE FROM sessions WHERE chat_id=? AND kName=?";
    static constexpr auto str_create_db = "CREATE TABLE IF NOT EXISTS\n"
                                          "sessions\n"
                                          "(\n"
                                          "chat_id INTEGER,\n"
                                          "kName TEXT not null,\n"
                                          "kValue TEXT null,\n"
                                          "PRIMARY KEY (chat_id, kName)\n"
                                          ")";

  public:
    static auto create_database (std::string filename)
    {
      SQLite::Database db (filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
      db.exec (str_create_db);
      return db;
    }

    persistence (std::string filename)
      : db (create_database (filename))
      , stm_get_key (db, str_get_key)
      , stm_set_key (db, str_set_key)
      , stm_del_key (db, str_del_key)
    {}

    auto get_value (banana::integer_t chat_id, std::string kName) -> std::optional<std::string>
    {
      auto guard = std::scoped_lock (mutex);
      stm_get_key.reset ();
      stm_get_key.bind (1, chat_id);
      stm_get_key.bind (2, kName);
      if (stm_get_key.executeStep ()) {
        return stm_get_key.getColumn (0).getString ();
      } else {
        return std::nullopt;
      }
    }

    bool set_value (banana::integer_t chat_id, std::string kName, std::string kValue)
    {
      auto guard = std::scoped_lock (mutex);
      stm_set_key.reset ();
      stm_set_key.bind (1, chat_id);
      stm_set_key.bind (2, kName);

      if (!kValue.empty ())
        stm_set_key.bind (3, kValue);
      return stm_set_key.exec ();
    }

    std::optional<long long> get_value_ll (banana::integer_t chat_id, std::string kName)
    {
      if (auto value = get_value (chat_id, kName); value)
        return std::stoll (value.value ());
      return std::nullopt;
    }

    bool set_value_ll (banana::integer_t chat_id, std::string kName, long long kValue)
    {
      return set_value (chat_id, kName, std::to_string (kValue));
    }

    std::optional<nlohmann::json> get_value_json (banana::integer_t chat_id, std::string kName)
    {
      if (auto value = get_value (chat_id, kName); value)
        return nlohmann::json::parse (value.value ());
      return std::nullopt;
    }

    bool set_value_json (banana::integer_t chat_id, std::string kName, nlohmann::json const& kValue)
    {
      return set_value (chat_id, kName, kValue.dump ());
    }

    bool delete_value (banana::integer_t chat_id, std::string kName)
    {
      auto guard = std::scoped_lock (mutex);
      stm_del_key.reset ();
      stm_del_key.bind (1, chat_id);
      stm_del_key.bind (2, kName);
      return stm_del_key.exec ();
    }
  };

} // namespace forest

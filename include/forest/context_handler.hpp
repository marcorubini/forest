#pragma once
#include <concepts>
#include <map>
#include <string>
#include <vector>

#include <banana/agent/cpr.hpp>
#include <banana/api.hpp>

#include <forest/concepts/context.hpp>
#include <forest/concepts/event.hpp>
#include <forest/concepts/state.hpp>
#include <forest/concepts/transition.hpp>
#include <forest/events/message.hpp>
#include <forest/persistence.hpp>
#include <forest/transition_table.hpp>

namespace forest
{
  template<std::copy_constructible T>
  class context
  {
  public:
    using cache_type = T;
    using cache_reference = T&;

  private:
    banana::integer_t chat_id;
    std::reference_wrapper<T> cache_ref;
    std::reference_wrapper<banana::agent::cpr_async> agent_ref;
    std::reference_wrapper<persistence> persistence_ref;

  public:
    context () = default;

    context (banana::integer_t chat_id, T& cache_ref, banana::agent::cpr_async& agent_ref, persistence& ref)
      : chat_id (chat_id)
      , cache_ref (cache_ref)
      , agent_ref (agent_ref)
      , persistence_ref (ref)
    {}

    auto get_cache () const -> cache_reference
    {
      return cache_ref.get ();
    }

    auto set_cache (cache_type cache) const -> void
    {
      cache_ref.get () = std::move (cache);
    }

    auto send_message (std::string text) const -> void
    {
      banana::api::send_message (agent_ref.get (), {.chat_id = chat_id, .text = std::move (text)});
    }

    // === persistence

    std::optional<std::string> get_value (std::string kName) const
    {
      return persistence_ref.get ().get_value (chat_id, kName);
    }

    bool set_value (std::string kName, std::string kValue) const
    {
      return persistence_ref.get ().set_value (chat_id, kName, kValue);
    }

    std::optional<long long> get_value_ll (std::string kName) const
    {
      return persistence_ref.get ().get_value_ll (chat_id, kName);
    }

    bool set_value_ll (std::string kName, long long kValue) const
    {
      return persistence_ref.get ().set_value_ll (chat_id, kName, kValue);
    }

    std::optional<nlohmann::json> get_value_json (std::string kName) const
    {
      return persistence_ref.get ().get_value_json (chat_id, kName);
    }

    bool set_value_json (std::string kName, nlohmann::json const& json) const
    {
      return persistence_ref.get ().set_value_json (chat_id, kName, json);
    }

    bool delete_value (std::string kName) const
    {
      return persistence_ref.get ().delete_value (chat_id, kName);
    }
  };

  // ---

  template<std::copy_constructible Cache, class Table>
  class context_handler;

  template<std::copy_constructible Cache, class... States, class... Transitions>
  class context_handler<Cache, transition_table<std::variant<States...>, Transitions...>>
  {
  public:
    using cache_type = Cache;
    using table_type = transition_table<std::variant<States...>, Transitions...>;
    using state_type = std::variant<States...>;
    using chat_id_type = banana::integer_t;
    using agent_type = banana::agent::cpr_async;
    using context_type = context<cache_type>;

  private:
    struct context_storage
    {
      cache_type cache;
      table_type table;
      state_type state;
    };

    std::map<chat_id_type, context_storage> context_map;
    std::reference_wrapper<agent_type> agent_ref;
    cache_type cache_init;
    table_type table_init;
    state_type state_init;
    persistence persistent_storage;

  public:
    context_handler (agent_type& agent, cache_type cache, table_type table, state_type state, std::string db_filename)
      : context_map ()
      , agent_ref (agent)
      , cache_init (std::move (cache))
      , table_init (std::move (table))
      , state_init (std::move (state))
      , persistent_storage (db_filename)
    {}

    void handle_update (banana::api::update_t update)
    {
      if (auto message = update.message; message.has_value ()) {
        auto chat_id = message->chat.id;
        auto event = events::message {message->text.value ()};

        if (!context_map.contains (chat_id)) {
          context_storage& storage =
            context_map.emplace (chat_id, context_storage {cache_init, table_init, state_init}).first->second;
          handle_on_entry (get_context (chat_id, storage), storage.state);
        }

        context_storage& storage = context_map.at (chat_id);
        context_type context = get_context (chat_id, storage);

        if (auto new_state = storage.table.trigger (context, storage.state, event); new_state.has_value ()) {
          handle_on_exit (context, storage.state);
          storage.state = new_state.value ();
          handle_on_entry (context, storage.state);
        }
      }
    }

  private:
    void handle_on_entry (context<cache_type> ctx, state_type& state)
    {
      auto const visitor = [ctx] (auto& state) {
        state.on_entry (ctx);
      };
      std::visit (visitor, state);
    }

    void handle_on_exit (context<cache_type> ctx, state_type& state)
    {
      auto const visitor = [ctx] (auto& state) {
        state.on_exit (ctx);
      };
      std::visit (visitor, state);
    }

    context<cache_type> get_context (chat_id_type chat_id, context_storage& storage)
    {
      return context<cache_type> (chat_id, storage.cache, agent_ref.get (), persistent_storage);
    }
  };

  template<std::copy_constructible Cache, class... States, class... Transitions, class StateStart>
  context_handler (banana::agent::cpr_async& agent,
    Cache cache,
    transition_table<std::variant<States...>, Transitions...> table,
    StateStart state,
    std::string) -> context_handler<Cache, transition_table<std::variant<States...>, Transitions...>>;

} // namespace forest

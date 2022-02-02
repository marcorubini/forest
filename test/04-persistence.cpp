#include <forest/forest.hpp>
#include <iostream>
#include <random>
#include <set>

int main (int argc, char** argv)
{
  std::cerr << "Bot start" << std::endl;

  using cache_type = std::monostate;
  using context_type = forest::context<cache_type>;

  struct state_start
  {
    void on_entry (context_type ctx)
    {}
    void on_exit (context_type ctx)
    {}
  };

  auto cmd_config =
    forest::command_transition ("/config", "config unita nome", [] (context_type ctx, state_start& state, std::string params) {
      auto stream = std::istringstream (params);
      std::string misura;
      std::string nome;

      auto misure = std::set<std::string> ({"km", "m", "mi"});

      if (stream >> misura >> nome) {
        if (!misure.contains (misura)) {
          ctx.send_message ("unità di misura non riconosciuta");
          return state_start {};
        }

        if (nome.empty ()) {
          ctx.send_message ("il nome non può essere vuoto");
          return state_start {};
        }

        ctx.set_value ("unita", misura);
        ctx.set_value ("nome", nome);
        ctx.send_message ("configurazione effettuata.");
      } else {
        ctx.send_message ("il comando richiede due parametri, unità di misura e nome.");
      }

      return state_start {};
    });

  auto rng = std::mt19937 (std::random_device () ());
  auto cmd_stampa_misura = forest::command_transition ("/stampaMisura",
    "stampa una misura casuale",
    [&rng] (context_type ctx, state_start& state, std::string params) {
      auto unita = ctx.get_value ("unita");

      if (!unita.has_value ()) {
        unita = "m";
      }

      auto dist = std::uniform_int_distribution<int> (0, 1000000);
      auto misura = dist (rng);

      if (unita.value () == "km")
        misura /= 1000;
      else if (unita.value () == "mi")
        misura /= 1609;

      ctx.send_message (std::to_string (misura) + " " + unita.value ());
      return state_start {};
    });

  // ===

  std::string api = argv[1];
  auto agent = banana::agent::cpr_async (api);
  std::cerr << "agent started with api " << api << std::endl;

  banana::api::set_my_commands (agent, {.commands = {cmd_config, cmd_stampa_misura}});
  std::cerr << "commands set" << std::endl;

  auto table = forest::make_transition_table<state_start> (cmd_config, cmd_stampa_misura);
  std::cerr << "table created" << std::endl;

  try {
    auto handler = forest::context_handler (agent, cache_type {}, table, state_start {}, "db04.db3");
    std::cerr << "handler created" << std::endl;

    banana::integer_t offset = 0;
    while (true) {
      auto allowed = std::vector<std::string> {"message"};
      auto updates = banana::api::get_updates (agent, {.offset = offset, .allowed_updates = allowed}).get ();

      for (banana::api::update_t u : updates) {
        std::cerr << "handling update " << u.update_id << std::endl;
        offset = std::max (offset, u.update_id + 1);
        handler.handle_update (u);
      }
    }
  } catch (std::exception& e) {
    std::cerr << typeid (e).name () << std::endl;
    std::cerr << e.what () << std::endl;
  }
}

#include <cpr/cpr.h>
#include <forest/forest.hpp>
#include <iostream>
#include <optional>
#include <random>
#include <set>
#include <sstream>

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
      /**
       * Accetta come parametri una unità di misura (tra km, m, mi) e un nome di persona.
       * Salva i due parametri in un database persistente.
       */
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
  auto cmd_stampa_misura = forest::command_transition ("/stampamisura",
    "stampa una misura casuale",
    [&rng] (context_type ctx, state_start& state, std::string params) {
      /**
       * Stampa una misura casuale usando l'unità di misura salvata come configurazione.
       */
      auto unita = ctx.get_value ("unita");

      if (!unita.has_value ()) {
        unita = "m";
      }

      auto dist = std::uniform_int_distribution<int> (0, 1000000);
      int misura = dist (rng);

      if (unita.value () == "km")
        misura /= 1000;
      else if (unita.value () == "mi")
        misura /= 1609;

      ctx.send_message (std::to_string (misura) + " " + unita.value ());
      return state_start {};
    });

  auto cmd_gender = forest::command_transition ("/indovinagenere",
    "stampa il genere del nome configurato",
    [] (context_type ctx, state_start& state, std::string params) {
      /**
       * Deduce il genere del nome configurato invocando l'api rest api.genderize.io
       */

      std::optional<std::string> nome = ctx.get_value ("nome");
      if (!nome.has_value ()) {
        ctx.send_message ("nome non configurato.");
        return state_start {};
      }

      std::string rest_url = "https://api.genderize.io/?name=" + nome.value ();
      cpr::Response response = cpr::Get (cpr::Url {rest_url});

      if (response.status_code != 200) {
        std::cerr << "status_code : " << response.status_code << std::endl;
        ctx.send_message ("errore server.");
        return state_start {};
      }

      try {
        std::cerr << "response: " << response.text << std::endl;
        nlohmann::json json = nlohmann::json::parse (response.text);
        std::string gender = json.at ("gender").get<std::string> ();
        double probability = json.at ("probability").get<double> ();

        if (gender == "male")
          gender = "maschio";
        if (gender == "female")
          gender = "femmina";

        std::ostringstream response;
        response << "nome: " << nome.value () << ", genere: " << gender
                 << ", probabilità: " << std::to_string (probability);
        ctx.send_message (response.str ());
        return state_start {};
      } catch (std::exception& e) {
        std::cerr << typeid (e).name () << std::endl;
        std::cerr << e.what () << std::endl;
        ctx.send_message ("errore server.");
        return state_start {};
      }

      return state_start {};
    });

  auto cmd_options =
    forest::command_transition ("/options", "scegli una azione", [] (context_type ctx, state_start& state, std::string params) {
      auto btn1 = forest::button ("btn1", "stampa messaggio");
      auto btn2 = forest::button ("btn2", "stampa misura");
      ctx.send_message ("Seleziona una azione", {{btn1, btn2}});
      std::cerr << "cmd_options" << std::endl;
      return state_start {};
    });

  auto on_press_btn1 = forest::button_transition ("btn1", [] (context_type ctx, state_start& state) {
    ctx.send_message ("hello world");
    return state_start {};
  });

  auto on_press_btn2 = forest::button_transition ("btn2", [=] (context_type ctx, state_start& state) mutable {
    return cmd_stampa_misura (ctx, state, {});
  });

  // ===

  std::string api = argv[1];
  auto agent = banana::agent::cpr_async (api);
  std::cerr << "agent started with api " << api << std::endl;

  bool ok = banana::api::delete_my_commands (agent, {}).get ();
  std::cerr << "delete commands result: " << ok << std::endl;
  ok = banana::api::set_my_commands (agent, {.commands = {cmd_config, cmd_stampa_misura, cmd_gender, cmd_options}})
         .get ();
  std::cerr << "set commands result: " << ok << std::endl;

  auto table = forest::make_transition_table<state_start> (cmd_config, //
    cmd_stampa_misura,
    cmd_gender,
    cmd_options,
    on_press_btn1,
    on_press_btn2);
  std::cerr << "table created" << std::endl;

  try {
    auto handler = forest::context_handler (agent, cache_type {}, table, state_start {}, "db04.db3");
    std::cerr << "handler created" << std::endl;

    banana::integer_t offset = 0;
    while (true) {
      auto allowed = std::vector<std::string> {"message", "callback_query"};
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

#include <forest/forest.hpp>
#include <iostream>

int main (int argc, char** argv)
{
  std::cerr << "Bot start" << std::endl;

  using context_type = forest::context<>;

  struct state_start;
  struct state_ask_name;
  struct state_ask_age;
  struct state_ask_pet_name;
  struct state_ask_pet_age;

  // =========================
  // --- state definitions ---
  // =========================

  struct state_start
  {
    auto on_entry (context_type ctx)
    {}

    auto on_exit (context_type ctx)
    {}
  };

  struct state_ask_name
  {
    auto on_entry (context_type ctx)
    {
      ctx.send_message ("What's your name?");
    }

    auto on_exit (context_type ctx)
    {}
  };

  struct state_ask_age
  {
    std::string name;

    auto on_entry (context_type ctx)
    {
      ctx.send_message ("What's your age?");
    }

    auto on_exit (context_type ctx)
    {}
  };

  struct state_ask_pet_name
  {
    auto on_entry (context_type ctx)
    {
      ctx.send_message ("What's your pet's name?");
    }

    auto on_exit (context_type ctx)
    {}
  };

  struct state_ask_pet_age
  {
    std::string name;

    auto on_entry (context_type ctx)
    {
      ctx.send_message ("What's your pet's age?");
    }

    auto on_exit (context_type ctx)
    {}
  };

  // ==============================
  // --- transition definitions ---
  // ==============================

  auto cmd_dialogue1 =
    // ------------------------ PREFIX ------ DESCRIPTION ----------------------------- INPUT STATE ----
    forest::command_transition ("/dialogue1", "Start dialogue 1", [] (context_type ctx, state_start&) {
      // --- OUTPUT STATE
      return state_ask_name {};
    });

  auto cmd_dialogue2 =
    // ------------------------ PREFIX ------ DESCRIPTION ----------------------------- INPUT STATE ----
    forest::command_transition ("/dialogue2", "Start dialogue 2", [] (context_type ctx, state_start&, std::string params) {
      // --- OUTPUT STATE
      return state_ask_pet_name {};
    });

  auto enter_ask_age_transition =
    // ---------------------------------------------- INPUT STATE ---------- INCOMING MESSAGE
    forest::message_transition ([] (context_type ctx, state_ask_name& state, std::string name, std::string params) {
      // --- OUTPUT STATE
      return state_ask_age {.name = name};
    });

  auto exit_ask_age_transition =
    // ---------------------------------------------- INPUT STATE --------- INCOMING MESSAGE
    forest::message_transition ([] (context_type ctx, state_ask_age& state, std::string age) {
      std::string name = state.name;
      ctx.send_message ("Hi " + name + ", your age is " + age);

      // --- OUTPUT STATE
      return state_start {};
    });

  auto enter_pet_ask_age_transition =
    // ---------------------------------------------- INPUT STATE -------------- INCOMING MESSAGE
    forest::message_transition ([] (context_type ctx, state_ask_pet_name& state, std::string name) {
      // OUTPUT STATE
      return state_ask_pet_age {.name = name};
    });

  auto exit_pet_ask_age_transition =
    // ---------------------------------------------- INPUT STATE ------------- INCOMING MESSAGE
    forest::message_transition ([] (context_type ctx, state_ask_pet_age& state, std::string age) {
      std::string name = state.name;
      ctx.send_message ("Your pet's name is " + name + " and your pet's age is " + age);

      // OUTPUT STATE
      return state_start {};
    });

  // ------------------------------------------

  std::string API = argv[1];
  auto agent = banana::agent::cpr_async (API);

  std::cerr << "Agent started with API " << API << std::endl;

  banana::api::set_my_commands (agent, {.commands = {cmd_dialogue1, cmd_dialogue2}});

  std::cerr << "Commands set" << std::endl;

  auto table = forest::make_transition_table<state_start, //
    state_ask_age,
    state_ask_name,
    state_ask_pet_age,
    state_ask_pet_name> (cmd_dialogue1,
    cmd_dialogue2,
    enter_ask_age_transition, //
    exit_ask_age_transition,
    enter_pet_ask_age_transition,
    exit_pet_ask_age_transition);

  try {
    auto handler = forest::context_handler (agent, {}, table, state_start {}, "db00.db3");

    std::cerr << "Handler constructed" << std::endl;

    banana::integer_t offset = 0;
    while (true) {
      auto allowed = std::vector<std::string> {"message"};
      auto updates = banana::api::get_updates (agent, {.offset = offset, .allowed_updates = allowed}).get ();

      for (banana::api::update_t u : updates) {
        std::cerr << "handle update " << u.update_id << std::endl;
        offset = std::max (offset, u.update_id + 1);
        handler.handle_update (u);
      }
    }
  } catch (std::exception& e) {
    std::cerr << typeid (e).name () << std::endl;
    std::cerr << e.what () << std::endl;
  }
}

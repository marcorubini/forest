#include <forest/forest.hpp>
#include <iostream>
#include <random>

using context_type = forest::context<>;

struct state_start
{
  void on_entry (context_type context)
  {}
  void on_exit (context_type context)
  {}
};

struct transition_start
{
  bool accepts (context_type context, state_start& state, forest::events::message event)
  {
    return event.text == "/start";
  }

  state_start operator() (context_type context, state_start& state, forest::events::message event)
  {
    std::ostringstream response;
    response << "Available commands: \n";
    response << "/roll rolls a dice";
    context.send_message (response.str ());
    return state_start {};
  }
};

struct transition_roll_dice
{
  std::mt19937& random_generator;

  bool accepts (context_type context, state_start& state, forest::events::message event)
  {
    return event.text == "/roll";
  }

  state_start operator() (context_type context, state_start& state, forest::events::message event)
  {
    int dice = random_generator () % 6 + 1;
    context.send_message (std::to_string (dice));
    return state_start {};
  }
};

int main (int argc, char** argv)
{
  std::string api = argv[1];
  banana::agent::cpr_async agent (api);
  banana::api::set_my_commands (agent, {.commands = {{"roll", "roll a dice"}}});

  std::mt19937 random_generator (std::random_device {}());
  auto table = forest::make_transition_table<state_start> ( //
    transition_start {},
    transition_roll_dice {random_generator});

  try {
    auto handler = forest::context_handler (agent, {}, table, state_start {}, "db05.db3");
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
    std::cerr << typeid (e).name () << ": " << e.what () << std::endl;
  }
}

#include <chrono>
#include <thread>

#include <forest/telegram.hpp>

namespace
{
  FOREST_DECLARE_STATE (Root);
  FOREST_DECLARE_STATE (Dialogue1);
  FOREST_DECLARE_STATE (Dialogue2);

  static constexpr auto& machine_description = R"RAW(
    Root
    |-- Dialogue1
    |-- Dialogue2
  )RAW";

  using Machine = forest::telegram::describe_machine<machine_description> //
    ::bind<Root, Dialogue1, Dialogue2>;

  struct Root : Machine::state<Root>
  {
    struct command_d1 : describe_cmd<"dialogue1", "Start dialogue 1">
    {};
    struct command_d2 : describe_cmd<"dialogue2", "Start dialogue 2">
    {};
    using my_commands = command_list<command_d1, command_d2>;

    void start (state_context const& ctx)
    {
      std::cerr << "Started" << std::endl;
    }

    auto react (state_context const& ctx, command_event<command_d1> cmd) -> transit_result<Dialogue1>
    {
      std::cerr << "Handling dialogue1\n";
      return transit<Dialogue1>;
    }

    auto react (state_context const& ctx, command_event<command_d2> cmd) -> transit_result<Dialogue2>
    {
      std::cerr << "Handling dialogue2\n";
      return transit<Dialogue2>;
    }
  };

  struct Dialogue1 : Machine::state<Dialogue1>
  {
    auto react (state_context const& ctx, message_event msg) -> transit_result<>
    {
      ctx.send_message ({.text = msg.text + " 1"});
      return transit<void>;
    }
  };

  struct Dialogue2 : Machine::state<Dialogue2>
  {
    auto react (state_context const& ctx, forest::telegram::message_event msg) -> transit_result<>
    {
      ctx.send_message ({.text = msg.text + " 2"});
      return transit<void>;
    }
  };

} // namespace

int main (int argc, char** argv)
{
  spdlog::set_level (spdlog::level::trace);
  spdlog::info ("test_example started with logging level {}", spdlog::get_level ());
  auto driver = forest::telegram::tgbot_driver ("5193745507:AAHAzxtf4jXLZGDIisk0q2HmVcakkfer_7w");
  spdlog::info ("driver created");

  auto bot = forest::telegram::bot<Machine> (driver);
  spdlog::info ("bot create");

  auto pipeline = forest::telegram::projections::to_telegram_event<Machine>;
  spdlog::info ("pipeline created");

  bot.set_commands ();
  spdlog::info ("commands set");

  while (true) {
    while (driver.has_updates ())
      forest::signal::visit_sink (bot, forest::signal::visit (pipeline, driver.next ()));

    while (driver.poll () == 0)
      std::this_thread::sleep_for (std::chrono::seconds (3));
  }
}

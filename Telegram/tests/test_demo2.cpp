#include "forest/telegram/driver.hpp"
#include <chrono>
#include <thread>

#include <forest/telegram.hpp>

namespace
{
  FOREST_DECLARE_STATE (Root);
  FOREST_DECLARE_STATE (Menu);
  FOREST_DECLARE_STATE (Dialogue1);
  FOREST_DECLARE_STATE (Dialogue2);

  static constexpr auto& machine_description = R"RAW(
    Root
    |-- Menu
    |-- Dialogue1
    |-- Dialogue2
  )RAW";

  using Machine = forest::telegram::describe_machine<machine_description> //
    ::bind<Root, Menu, Dialogue1, Dialogue2>;

  struct Root : Machine::state<Root>
  {
    struct command_d1 : describe_cmd<"dialogue1", "Start dialogue 1">
    {};
    struct command_d2 : describe_cmd<"dialogue2", "Start dialogue 2">
    {};
    struct command_menu : describe_cmd<"menu", "Open menu">
    {};
    using my_commands = command_list<command_menu, command_d1, command_d2>;

    void enter (state_context const& ctx)
    {
      spdlog::debug ("Enter method of Root invoked.");
    }

    auto react (state_context const& ctx, command_event<command_menu> cmd) -> transit_result<Menu>
    {
      spdlog::debug ("Transit to Menu");
      return transit<Menu>;
    }

    auto react (state_context const& ctx, command_event<command_d1> cmd) -> transit_result<Dialogue1>
    {
      spdlog::debug ("Transit to Dialogue1");
      return transit<Dialogue1>;
    }

    auto react (state_context const& ctx, command_event<command_d2> cmd) -> transit_result<Dialogue2>
    {
      spdlog::debug ("Transit to Dialogue2");
      return transit<Dialogue2>;
    }
  };

  struct Menu : Machine::state<Menu>
  {
    forest::telegram::api_message_id my_id;

    struct btn_d1 : describe_btn<>
    {};

    struct btn_d2 : describe_btn<>
    {};

    using my_buttons = button_list<btn_d1, btn_d2>;

    void enter (state_context const& ctx)
    {
      spdlog::debug ("Enter method of Menu invoked.");

      auto msg = message ();
      msg.text = "menu";
      msg.add_button<btn_d1> (0, "Start dialogue 1");
      msg.add_button<btn_d2> (0, "Start dialogue 2");
      my_id = ctx.send_message (msg).value ();
    }

    void exit (state_context const& ctx)
    {
      ctx.delete_message (my_id);
    }

    auto react (state_context const& ctx, button_event<btn_d1> btn) -> transit_result<Dialogue1>
    {
      spdlog::debug ("Button dialogue1 pressed.");
      ctx.answer_callback ({.callback_id = btn.callback_id, .text = "Dialogue1", .show_alert = true});
      return transit<Dialogue1>;
    }

    auto react (state_context const& ctx, button_event<btn_d2> btn) -> transit_result<Dialogue2>
    {
      spdlog::debug ("Button dialogue2 pressed.");
      ctx.answer_callback ({.callback_id = btn.callback_id, .text = "Dialogue2", .show_alert = true});
      return transit<Dialogue2>;
    }
  };

  static_assert (Machine::has_enter_action<Menu>);

  struct Dialogue1 : Machine::state<Dialogue1>
  {
    void enter (state_context const& ctx)
    {
      spdlog::debug ("Enter method of Dialogue1 invoked.");
    }

    auto react (state_context const& ctx, message_event msg) -> transit_result<>
    {
      ctx.send_message ({.text = msg.text + " 1"});
      return transit<void>;
    }
  };

  struct Dialogue2 : Machine::state<Dialogue2>
  {
    void enter (state_context const& ctx)
    {
      spdlog::debug ("Enter method of Dialogue2 invoked.");
    }

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

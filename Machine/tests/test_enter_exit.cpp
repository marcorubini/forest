#include <forest/machine.hpp>

namespace ns_test_enter_exit
{
  FOREST_DECLARE_STATE (Root);
  FOREST_DECLARE_STATE (A);
  FOREST_DECLARE_STATE (B);

  constexpr inline auto& description = R"RAW(
    Root
    |--- A
    |--- B
  )RAW";

  using Machine = forest::machine::describe_machine<description> //
    ::bind<Root, A, B>;

  struct event1
  {
    bool choice;
  };

  struct Root : Machine::state<Root>
  {
    bool active = false;

    void enter (state_context const& ctx)
    {
      active = true;
    }

    void exit (state_context const& ctx)
    {
      active = false;
    }

    auto react (state_context const& ctx, event1 e) -> transit_result<A, B>
    {
      if (e.choice)
        return transit<A> {};
      else
        return transit<B> {};
    }
  };

  struct A : Machine::state<A>
  {
    bool active = false;

    void enter (state_context const& ctx)
    {
      active = true;
    }

    void exit (state_context const& ctx)
    {
      active = false;
    }

    auto react (state_context const& ctx, event1 e) -> transit_result<B>
    {
      return transit<B> {};
    }
  };

  struct B : Machine::state<B>
  {
    bool active = false;

    void enter (state_context const& ctx)
    {
      active = true;
    }

    void exit (state_context const& ctx)
    {
      active = false;
    }

    auto react (state_context const& ctx, event1 e) -> transit_result<A>
    {
      return transit<A> {};
    }
  };

  static_assert (Machine::has_enter_action<Root>);
  static_assert (Machine::has_enter_action<A>);
  static_assert (Machine::has_enter_action<B>);

  static_assert (Machine::has_exit_action<Root>);
  static_assert (Machine::has_exit_action<A>);
  static_assert (Machine::has_exit_action<B>);

} // namespace ns_test_enter_exit

int test_enter_exit (int, char**)
{
  using namespace ns_test_enter_exit;

  auto machine = Machine::instance ();
  machine.start ();

  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  assert (machine.react (event1 {true}));
  assert (machine.is_active<Root> ());
  assert (machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  assert (machine.react (event1 {}));
  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (machine.state_cast<B> ().active);

  machine.stop ();

  assert (!machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());
  assert (!machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  // ---

  machine.start ();

  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  assert (machine.react (event1 {false}));
  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (machine.state_cast<B> ().active);

  assert (machine.react (event1 {}));
  assert (machine.is_active<Root> ());
  assert (machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<Root> ().active);
  assert (machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  machine.stop ();

  assert (!machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());
  assert (!machine.state_cast<Root> ().active);
  assert (!machine.state_cast<A> ().active);
  assert (!machine.state_cast<B> ().active);

  return 0;
}
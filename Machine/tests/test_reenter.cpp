#include <forest/machine.hpp>

namespace ns_test_reenter
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
    int entered = 0;

    inline void enter (state_context const& ctx)
    {
      entered = 1;
    }

    void exit (state_context const& ctx)
    {
      entered = 0;
    }

    void reenter (state_context const& ctx)
    {
      entered += 1;
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
    int enteredA = 0;

    void enter (state_context const& ctx)
    {
      enteredA = 1;
    }

    void exit (state_context const& ctx)
    {
      enteredA = 0;
    }

    void reenter (state_context const& ctx)
    {
      enteredA += 1;
    }
  };

  struct B : Machine::state<B>
  {
    int enteredB = 0;

    void enter (state_context const& ctx)
    {
      enteredB = 1;
    }

    void exit (state_context const& ctx)
    {
      enteredB = 0;
    }

    void reenter (state_context const& ctx)
    {
      enteredB += 1;
    }
  };

  static_assert (Machine::has_enter_action<Root>);
  static_assert (Machine::has_enter_action<A>);
  static_assert (Machine::has_enter_action<B>);

  static_assert (Machine::has_exit_action<Root>);
  static_assert (Machine::has_exit_action<A>);
  static_assert (Machine::has_exit_action<B>);

  static_assert (Machine::has_reenter_action<Root>);
  static_assert (Machine::has_reenter_action<A>);
  static_assert (Machine::has_reenter_action<B>);

} // namespace ns_test_reenter

int test_reenter (int, char**)
{
  using namespace ns_test_reenter;

  auto machine = Machine::instance ();
  assert (!machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());
  assert (machine.state_cast<Root> ().entered == 0);
  assert (machine.state_cast<A> ().enteredA == 0);
  assert (machine.state_cast<B> ().enteredB == 0);

  machine.start ();
  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<Root> ().entered == 1);
  assert (machine.state_cast<A> ().enteredA == 0);
  assert (machine.state_cast<B> ().enteredB == 0);

  assert (machine.react (event1 {true}));
  assert (machine.is_active<Root> ());
  assert (machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<A> ().enteredA == 1);
  assert (machine.state_cast<B> ().enteredB == 0);
  assert (machine.state_cast<Root> ().entered == 1);

  assert (machine.react (event1 {true}));
  assert (machine.is_active<Root> ());
  assert (machine.is_active<A> ());
  assert (!machine.is_active<B> ());

  assert (machine.state_cast<A> ().enteredA == 2);
  assert (machine.state_cast<B> ().enteredB == 0);
  assert (machine.state_cast<Root> ().entered == 1);

  assert (machine.react (event1 {false}));
  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (machine.is_active<B> ());

  assert (machine.state_cast<A> ().enteredA == 0);
  assert (machine.state_cast<B> ().enteredB == 1);
  assert (machine.state_cast<Root> ().entered == 1);

  assert (machine.react (event1 {false}));
  assert (machine.is_active<Root> ());
  assert (!machine.is_active<A> ());
  assert (machine.is_active<B> ());

  assert (machine.state_cast<A> ().enteredA == 0);
  assert (machine.state_cast<B> ().enteredB == 2);
  assert (machine.state_cast<Root> ().entered == 1);

  machine.stop ();

  assert (machine.state_cast<Root> ().entered == 0);
  assert (machine.state_cast<A> ().enteredA == 0);
  assert (machine.state_cast<B> ().enteredB == 0);

  return 0;
}
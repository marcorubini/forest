#include <forest/machine.hpp>

namespace ns_test_region
{
  FOREST_DECLARE_STATE (Root);
  FOREST_DECLARE_STATE (A);
  FOREST_DECLARE_STATE (B);

  constexpr inline auto& description = R"RAW(
    [Root]
     |--- A
     |--- B
  )RAW";

  using Machine = forest::machine::describe_machine<description> //
    ::bind<Root, A, B>;

  struct event1
  {};
  struct event2
  {};

  struct Root : Machine::state<Root>
  {
    int a_counter = 0;
    int b_counter = 0;
  };

  struct A : Machine::state<A>
  {
    auto react (state_context const& ctx, event1) -> transit_result<>
    {
      ctx.state_cast<Root> ().a_counter++;
      return transit<void> {};
    }
  };

  struct B : Machine::state<B>
  {
    auto react (state_context const& ctx, event2) -> transit_result<>
    {
      ctx.state_cast<Root> ().b_counter++;
      return transit<void> {};
    }
  };

} // namespace ns_test_region

int test_region (int, char**)
{
  using namespace ns_test_region;

  auto machine = Machine::instance ();
  machine.start ();

  assert (machine.is_active<Root> ());
  assert (machine.is_active<A> ());
  assert (machine.is_active<B> ());

  assert (machine.react (event1 {}));
  assert (machine.react (event2 {}));

  assert (machine.state_cast<Root> ().a_counter == 1);
  assert (machine.state_cast<Root> ().b_counter == 1);

  machine.stop ();

  assert (machine.is_active<Root> () == false);
  assert (machine.is_active<A> () == false);
  assert (machine.is_active<B> () == false);

  return 0;
}
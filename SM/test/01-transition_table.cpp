#include <cassert>
#include <iostream>

#include <forest/sm/sm.hpp>

struct state1
{};
struct state2
{};
struct state3
{};

struct event1
{};
struct event2
{
  int value;
};

struct transition1
{
  state2 operator() (auto ctx, state1& state, event1) const
  {
    std::cerr << "Transition1" << std::endl;
    return state2 {};
  }
};

struct transition2
{
  bool accepts (auto ctx, state2& state, event2 event) const
  {
    return event.value == 10;
  }

  state3 operator() (auto ctx, state2& state, event2 event) const
  {
    std::cerr << "Transition2" << std::endl;
    return state3 {};
  }
};

struct transition3
{
  void operator() (auto ctx, state3& state, event1 event) const
  {
    std::cerr << "Transition3" << std::endl;
  }
};

int main ()
{
  auto sm = forest::sm::make_state_machine<state1, state2, state3> (transition1 {}, transition2 {}, transition3 {});
  forest::sm::transition_result result = sm.start ();
  assert (result.triggered);
  assert (result.state_changed);

  result = sm.process_event (event1 {});
  assert (result.triggered);
  assert (result.state_changed);
  assert (sm.is<state2> ());

  result = sm.process_event (event2 {});
  assert (!result.triggered);

  result = sm.process_event (event2 {10});
  assert (result.triggered);
  assert (result.state_changed);
  assert (sm.is<state3> ());

  result = sm.process_event (event1 {});
  assert (result.triggered);
  assert (!result.state_changed);
}

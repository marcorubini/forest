#include <cassert>
#include <forest/sm/sm.hpp>
#include <iostream>

int main ()
{
  struct context_type
  {
    using state_machine_type = void;
  };

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
    state2 operator() (context_type, state1& state, event1) const
    {
      std::cerr << "Transition1" << std::endl;
      return state2 {};
    }
  };

  struct transition2
  {
    bool accepts (context_type, state2& state, event2 event) const
    {
      return event.value == 10;
    }

    state3 operator() (context_type, state2& state, event2 event) const
    {
      std::cerr << "Transition2" << std::endl;
      return state3 {};
    }
  };

  struct transition3
  {
    void operator() (context_type, state3& state, event1 event) const
    {
      std::cerr << "Transition3" << std::endl;
    }
  };

  using storage_type = std::variant<state1, state2, state3>;

  auto table = forest::sm::transition_table (transition1 {}, transition2 {}, transition3 {});
  storage_type storage = state1 {};

  forest::sm::transition_result result = forest::sm::trigger_transition (table, //
    context_type {},
    std::get<state1> (storage),
    event1 {},
    storage);
  assert (result.triggered);
  assert (result.state_changed);
  assert (std::holds_alternative<state2> (storage));

  result = forest::sm::trigger_transition (table, //
    context_type {},
    std::get<state2> (storage),
    event2 {},
    storage);
  assert (!result.triggered);

  result = forest::sm::trigger_transition (table, //
    context_type {},
    std::get<state2> (storage),
    event2 {10},
    storage);
  assert (result.triggered);
  assert (result.state_changed);
  assert (std::holds_alternative<state3> (storage));

  result = forest::sm::trigger_transition (table, //
    context_type {},
    std::get<state3> (storage),
    event1 {},
    storage);
  assert (result.triggered);
  assert (!result.state_changed);
}

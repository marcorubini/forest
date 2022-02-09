#include <forest/sm/sm.hpp>

#include <cassert>
#include <iostream>
#include <thread>

struct state1
{
  void on_entry (auto ctx) const
  {
    std::cerr << "on_entry state1\n";
  }

  void on_exit (auto ctx) const
  {
    std::cerr << "on_exit state1\n";
  }
};

struct state2
{
  void on_entry (auto ctx) const
  {
    std::cerr << "on_entry state2\n";
  }

  void on_exit (auto ctx) const
  {
    std::cerr << "on_exit state2\n";
  }

  std::chrono::milliseconds timeout_duration () const
  {
    return std::chrono::milliseconds (5000);
  }
};

struct event1
{};

struct transition1
{
  state2 operator() (auto ctx, state1& state, event1 event) const
  {
    std::cerr << "Transition state1 -> state2\n";
    return state2 {};
  }
};

struct transition2
{
  state1 operator() (auto ctx, state2& state, forest::sm::timeout_event event) const
  {
    std::cerr << "Handling timeout, return to state1\n";
    return state1 {};
  }
};

int main ()
{
  auto sm = forest::sm::make_state_machine<state1, state2> (transition1 {}, transition2 {});
  auto result = sm.start ();
  assert (result.triggered);
  assert (result.state_changed);
  assert (sm.is<state1> ());

  while (true) {
    sm.process_event (event1 {});
    sm.process_timeouts ();
    std::this_thread::sleep_for (std::chrono::milliseconds (3000));
  }
}
#pragma once
#include <forest/internal/container/literal_tree.hpp>
#include <forest/internal/meta/for_each.hpp>

namespace forest::internal
{
  struct state_tree_config
  {
    long num_states;
    long num_arcs;
  };

  struct state_tree_node
  {
    bool is_region;
  };

  template<state_tree_config Conf>
  using state_tree = literal_tree<Conf.num_states, Conf.num_arcs, state_tree_node>;

  template<class T>
  constexpr inline bool is_state_tree = false;

  template<long num_states, long num_arcs>
  constexpr inline bool is_state_tree<literal_tree<num_states, num_arcs, state_tree_node>> = true;

  template<class T>
  concept StateTree = is_state_tree<T>;

} // namespace forest::internal
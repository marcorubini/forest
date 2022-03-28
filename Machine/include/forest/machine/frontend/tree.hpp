#pragma once
#include <string_view>

#include <forest/container.hpp>

namespace forest::machine::internal
{
  struct syntax_tree_node
  {
    std::string_view name;
    bool is_region;
  };

  using syntax_tree = forest::container::tree<syntax_tree_node>;

  struct state_tree_node
  {
    bool is_region;
  };

  template<long num_states, long num_arcs>
  using state_tree = forest::container::literal_tree<num_states, num_arcs, state_tree_node>;

  template<long NStates, long NArcs, long SLen>
  struct named_state_tree
  {
    using string_type = forest::container::literal_string<SLen>;
    using array_type = std::array<string_type, NStates>;

    state_tree<NStates, NArcs> tree;
    array_type names;

    static constexpr long num_states = NStates;
    static constexpr long num_arcs = NArcs;
    static constexpr long string_length = SLen;
  };

} // namespace forest::machine::internal
#pragma once
#include <forest/internal/back_end/base.hpp>
#include <forest/internal/back_end/context.hpp>
#include <forest/internal/back_end/machine.hpp>
#include <forest/internal/back_end/state.hpp>
#include <forest/internal/back_end/transit.hpp>

#include <forest/internal/front_end/parsing.hpp>

namespace forest::internal
{
  template<auto NamedTree, std::move_constructible GS, List SL>
  struct incomplete_machine
  {
    using name_type = typename decltype (NamedTree)::name_type;

    template<name_type StateName>
    static constexpr long find_name = [] () -> long {
      for (long i = 0; i < NamedTree.names.size (); ++i)
        if (NamedTree.names[i] == StateName)
          return i;
      return -1;
    }();

    template<name_type StateName>
    static constexpr bool contains_name = find_name<StateName> != -1;

    template<name_type StateName>
      requires (contains_name<StateName>)
    static constexpr bool unbound = std::same_as<void, boost::mp11::mp_at_c<SL, find_name<StateName>>>;

    template<name_type StateName, class T>
    using modify_list = boost::mp11::mp_replace_at_c<SL, find_name<StateName>, T>;

    template<name_type StateName, class StateType>
      requires (contains_name<StateName>&& unbound<StateName>)
    using bind = typename incomplete_machine<NamedTree, GS, modify_list<StateName, StateType>>::type;

    using type = incomplete_machine;
  };

  template<auto NamedTree, std::move_constructible GS, List SL>
    requires (boost::mp11::mp_contains<SL, void>::value == false)
  struct incomplete_machine<NamedTree, GS, SL>
  {
    using type = machine_description<NamedTree.tree, GS, SL>;
  };

  template<auto Text, class GlobalStorage>
  struct make_machine_helper
  {
    static constexpr auto named_tree = parse_named_state_tree<Text>;
    static constexpr auto named_tree_size = named_tree.tree.num_vertices ();

    using void_list = boost::mp11::mp_repeat_c<boost::mp11::mp_list<void>, named_tree_size>;
    using type = incomplete_machine<named_tree, GlobalStorage, void_list>;
  };

  template<literal_string Text, std::move_constructible GlobalStorage>
  using make_machine = typename make_machine_helper<Text, GlobalStorage>::type;

} // namespace forest::internal
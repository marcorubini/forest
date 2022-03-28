#pragma once
#include <forest/machine/backend/core.hpp>

#include <boost/mp11/algorithm.hpp>

namespace forest::machine::internal
{
  template<auto Tree>
  constexpr inline auto hide = [] () {
    return Tree;
  };

  template<auto NT, class GS, template<class, class> class CC, template<class, class> class CS>
  struct incomplete_binding
  {
    using name_type = forest::container::literal_string<NT.string_length>;
    static constexpr long num_states = NT.num_states;

    template<class State>
    static constexpr name_type
      name = decltype (forest_declare_binding (std::type_identity<State> {})) {}.value.view ();

    template<long Index>
    struct find_pred
    {
      template<class State>
      using fn = forest::meta::constant<name<State> == NT.names[Index]>;
    };

    template<long NameIndex, class StateList>
    using find = boost::mp11::mp_at<StateList, boost::mp11::mp_find_if_q<StateList, find_pred<NameIndex>>>;

    template<class StateList>
    struct reorder_fn
    {
      template<class... Indices>
      using fn = boost::mp11::mp_list<find<Indices {}, StateList>...>;
    };

    template<class StateList>
    using reorder = boost::mp11::mp_apply_q<reorder_fn<StateList>, boost::mp11::mp_iota_c<NT.num_states>>;

    template<class... States>
      requires (sizeof...(States) == num_states)
    using bind = forest::machine::machine_traits<hide<NT.tree>, GS, reorder<boost::mp11::mp_list<States...>>, CC, CS>;
  };

  template<auto NamedTree, class GlobalStorage, template<class, class> class CustomContext, template<class, class> class CustomState>
  using make_machine_helper = incomplete_binding<NamedTree, GlobalStorage, CustomContext, CustomState>;
} // namespace forest::machine::internal

#define FOREST_DECLARE_STATE(NAME)                                                                           \
  struct NAME;                                                                                               \
  constexpr auto forest_declare_binding (std::type_identity<NAME>)->::forest::meta::constant<::forest::container::literal_string (#NAME)>

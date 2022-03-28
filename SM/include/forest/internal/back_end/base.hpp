#pragma once
#include <algorithm>
#include <boost/mp11.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <forest/internal/front_end/state_tree.hpp>
#include <numeric>
#include <variant>

namespace forest::internal
{
  // List concept used to constrain the machine template.

  template<class T>
  concept List = boost::mp11::mp_is_list<T>::value;

  // Forward declared machine template

  template<StateTree auto Tree, std::move_constructible GlobalStorage, List StateList>
  class machine_instance;

  // Forward declared context template

  template<class M, class S>
  class context;

  template<class M, class S>
  class exact_context;

  // Forward declared state template

  template<class M, class S>
  class state;

  // Forward declared transit template

  template<class... Alternatives>
  struct transit_result;

  struct transit_none
  {};

  template<class S>
  struct transit
  {};

  namespace traits
  {
    // Machine traits
    // machine traits work even if the machine type is not complete.

    template<class T>
    constexpr inline bool is_machine = false;

    template<auto Tree, class GlobalStorage, class StateList>
    constexpr inline bool is_machine<machine_instance<Tree, GlobalStorage, StateList>> = true;

    template<class T>
    concept Machine = is_machine<T>;

    template<Machine T>
    constexpr inline auto state_tree = nullptr;

    template<auto Tree, class GlobalStorage, class StateList>
    constexpr inline auto state_tree<machine_instance<Tree, GlobalStorage, StateList>> = Tree;

    namespace internal
    {
      template<Machine T>
      struct machine_state_list_type
      {};

      template<auto Tree, class GlobalStorage, class StateList>
      struct machine_state_list_type<machine_instance<Tree, GlobalStorage, StateList>>
      {
        using type = StateList;
      };
    } // namespace internal

    template<Machine T>
    using state_list = typename internal::machine_state_list_type<T>::type;

    template<Machine T>
    constexpr inline long state_list_size = 0;

    template<auto Tree, class GlobalStorage, class StateList>
    constexpr inline long state_list_size<machine_instance<Tree, GlobalStorage, StateList>> =
      boost::mp11::mp_size<StateList>::value;

    template<Machine M, class S>
    constexpr inline bool contains = boost::mp11::mp_contains<state_list<M>, S>::value;

    template<Machine M, long I>
    constexpr inline bool contains_c = I >= 0 && I < state_list_size<M>;

    template<Machine M, class S>
      requires (contains<M, S>)
    constexpr inline long state_index = boost::mp11::mp_find<state_list<M>, S>::value;

    template<Machine M, long I>
      requires (contains_c<M, I>)
    constexpr inline bool is_region_c = state_tree<M>[I].is_region;

    template<Machine M, class S>
      requires (contains<M, S>)
    constexpr inline bool is_region = is_region_c<M, state_index<M, S>>;

    template<Machine M, long I>
      requires (contains_c<M, I>)
    constexpr inline bool is_root_c = lt_root<state_tree<M>> == I;

    template<Machine M, class S>
      requires (contains<M, S>)
    constexpr inline bool is_root = is_root_c<M, state_index<M, S>>;

    template<Machine M, long I>
      requires (contains_c<M, I>)
    using state_type = typename boost::mp11::mp_at_c<state_list<M>, I>;

    template<Machine M>
    constexpr inline long root_c = lt_root<state_tree<M>>;

    template<Machine M>
    using root = state_type<M, root_c<M>>;

    template<Machine M, long Ancestor, long Descendant>
      requires (contains_c<M, Ancestor>&& contains_c<M, Descendant>)
    constexpr inline bool is_ancestor_c = lt_is_ancestor<state_tree<M>, Ancestor, Descendant>;

    template<Machine M, long Index>
      requires (contains_c<M, Index>)
    constexpr inline long parent_c = lt_parent<state_tree<M>, Index>;

    template<Machine M, long Index>
      requires (contains_c<M, Index>)
    using parent = state_type<M, parent_c<M, Index>>;

    template<Machine M, class State>
      requires (contains<M, State>)
    using parent_context = context<M, parent<M, state_index<M, State>>>;

    template<Machine M, long X, long Y>
      requires (contains_c<M, X>&& contains_c<M, Y>)
    constexpr inline bool is_cross_transition_c = [] {
      constexpr long lca = lt_lca<state_tree<M>, X, Y>;
      if constexpr (X == lca || Y == lca)
        return false;
      return is_region_c<M, lca>;
    }();

    // State traits
    // state traits assume that the state argument is complete

    template<class S>
    concept State = requires
    {
      typename S::machine_type;
      requires std::derived_from<S, state<typename S::machine_type, S>>;
    };

    template<State S>
    using state_machine_t = typename S::machine_type;

    template<State T>
    constexpr inline bool state_has_entry_action = requires (T& state, context<state_machine_t<T>, T> ctx)
    {
      // clang-format off
      { state.on_entry(ctx) };
      // clang-format on
    };

    template<State T>
    constexpr inline bool state_has_exit_action = requires (T& state, context<state_machine_t<T>, T> ctx)
    {
      // clang-format off
      { state.on_exit(ctx) };
      // clang-format on
    };

    template<State T>
    constexpr inline bool state_has_reenter_action = requires (T& state, context<state_machine_t<T>, T> ctx)
    {
      // clang-format off
      { state.on_reenter(ctx) };
      // clang-format on
    };

    // Transit traits
    // transit traits work even if states are not complete

    template<class T>
    constexpr inline bool is_transit_result = false;

    template<class... Alternatives>
    constexpr inline bool is_transit_result<transit_result<Alternatives...>> = true;

    template<class T>
    concept TransitResult = is_transit_result<T>;

    template<class T>
    constexpr inline bool is_transit = false;

    template<class S>
    constexpr inline bool is_transit<transit<S>> = true;

    template<class T>
    concept Transit = is_transit<T> || std::same_as<T, transit_none>;

    template<TransitResult T1, TransitResult T2>
    constexpr inline bool transit_is_subset = false;

    template<class... Args1, class... Args2>
    constexpr inline bool transit_is_subset<transit_result<Args1...>, transit_result<Args2...>> = [] () {
      using list1 = boost::mp11::mp_list<Args1...>;
      using list2 = boost::mp11::mp_list<Args2...>;
      using list_union = boost::mp11::mp_set_union<list2, list1>;
      return std::same_as<list2, list_union>;
    }();

    namespace internal
    {
      template<class T>
      struct transit_target_type;

      template<class T>
      struct transit_target_type<transit<T>> : std::type_identity<T>
      {};

      template<>
      struct transit_target_type<transit_none> : std::type_identity<void>
      {};
    } // namespace internal

    template<Transit T>
    using transit_target_t = typename internal::transit_target_type<T>::type;

    // Reaction traits

    template<State Super, State Leaf, class... Args>
    constexpr inline bool state_has_reaction = requires (Super& state, context<state_machine_t<Super>, Leaf> ctx, Args&&... args)
    {
      // clang-format off
      { state.react(ctx, std::forward<Args>(args)...) } -> TransitResult;
      // clang-format on
    };

    template<Machine M, long LeafIndex, class... Args>
    constexpr inline long hierarchy_find_reaction_handler = [] () -> long {
      using leaf_type = state_type<M, LeafIndex>;
      using context_type = context<M, leaf_type>;
      constexpr long root_index = lt_root<state_tree<M>>;

      long result = -1;
      lt_for_each_in_path<state_tree<M>, LeafIndex, root_index> ([&result] (auto I) {
        using state_type = traits::state_type<M, I>;
        if constexpr (state_has_reaction<state_type, leaf_type, Args...>)
          if (result == -1)
            result = I;
      });

      return result;
    }();

    template<Machine M, long Index, class... Args>
    constexpr inline bool hierarchy_has_reaction_handler = hierarchy_find_reaction_handler<M, Index, Args...> != -1;

    template<Machine M, class... Args>
    constexpr inline auto reaction_sources = [] () {
      constexpr auto tree = traits::state_tree<M>;
      constexpr auto root = traits::root_c<M>;

      constexpr long size = [&] () {
        long result = 0;
        lt_for_each_in_subtree<tree, root> ([&] (auto I) {
          if constexpr (hierarchy_has_reaction_handler<M, I, Args...>)
            result += 1;
        });
        return result;
      }();

      auto result = std::array<long, size> {};
      auto result_dist = std::array<long, size> {};
      auto indices = std::array<long, size> {};
      std::iota (indices.begin (), indices.end (), 0);
      long result_size = 0;

      lt_for_each_in_subtree<tree, root> ([&] (auto I) {
        if constexpr (hierarchy_has_reaction_handler<M, I, Args...>) {
          constexpr long handler = hierarchy_find_reaction_handler<M, I, Args...>;
          result[result_size] = I;
          result_dist[result_size] = lt_depth<tree, I> - lt_depth<tree, handler>;
          result_size++;
        }
      });

      std::ranges::sort (indices, std::ranges::less {}, [&result_dist] (int x) {
        return result_dist[x];
      });

      auto result_copy = result;
      for (long i = 0; i < size; ++i)
        result_copy[i] = result[indices[i]];
      return result_copy;
    }();

  } // namespace traits

} // namespace forest::internal
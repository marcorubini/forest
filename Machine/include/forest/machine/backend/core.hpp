#pragma once
#include <boost/mp11.hpp>
#include <iostream>
#include <tuple>

#include <forest/container.hpp>
#include <forest/meta.hpp>

namespace forest::machine
{
  // Forward declarations

  template<auto Hider, class GS, class SB, template<class Traits, class State> class CC, template<class Traits, class State> class CS>
  class machine_traits;

  namespace internal
  {
    template<class T>
    constexpr inline bool is_machine_traits = false;

    template<auto Hider, class GS, class SB, template<class M, class S> class CC, template<class M, class S> class CS>
    constexpr inline bool is_machine_traits<machine_traits<Hider, GS, SB, CC, CS>> = true;
  } // namespace internal

  template<class T>
  concept MachineTraits = internal::is_machine_traits<T>;

  template<MachineTraits Traits>
  class machine_instance;

  template<MachineTraits Traits>
  class machine_reference;

  namespace internal
  {
    template<class T>
    constexpr inline bool is_machine_instance = false;

    template<class Traits>
    constexpr inline bool is_machine_instance<machine_instance<Traits>> = true;

    template<class T>
    constexpr inline bool is_machine_reference = false;

    template<class Traits>
    constexpr inline bool is_machine_reference<machine_reference<Traits>> = true;
  } // namespace internal

  template<class T>
  concept MachineInstance = internal::is_machine_instance<T>;

  template<class T>
  concept MachineReference = internal::is_machine_reference<T>;

  template<class T>
  concept Machine = MachineInstance<T> || MachineReference<T>;

  template<MachineTraits Traits, class State>
  class default_context;

  template<MachineTraits Traits, class State>
  class context_interface;

  template<MachineTraits, class State>
  class default_state;

  template<MachineTraits Traits, class State>
  class state_interface;

  template<class... Alternatives>
  class transit_result;

  template<class State>
  struct transit
  {
    using type = State;
  };

  namespace internal
  {
    template<class T>
    constexpr inline bool is_transit_result = false;

    template<class... Alternatives>
    constexpr inline bool is_transit_result<transit_result<Alternatives...>> = true;

    template<class T>
    constexpr inline bool is_transit = false;

    template<class T>
    constexpr inline bool is_transit<transit<T>> = true;
  } // namespace internal

  template<class T>
  concept Transit = internal::is_transit<T>;

  template<class T>
  concept TransitResult = internal::is_transit_result<T>;

  // traits definition

  template<auto Hider, class GS, class SB, template<class Traits, class State> class CC, template<class Traits, class State> class CS>
  class machine_traits
  {
  public:
    static constexpr auto tree = Hider ();
    using storage_type = GS;
    using list_type = SB;
    using instance = machine_instance<machine_traits>;
    using reference = machine_reference<machine_traits>;
    static constexpr long binding_size = boost::mp11::mp_size<list_type>::value;

    // basic traits

    template<long ID>
    static constexpr bool contains_c = (ID >= 0 && ID < binding_size);

    template<class State>
    static constexpr bool contains = boost::mp11::mp_contains<list_type, State>::value;

    template<long ID>
      requires (contains_c<ID>)
    using state_type = boost::mp11::mp_at_c<list_type, ID>;

    template<class State>
      requires (contains<State>)
    static constexpr long state_index = boost::mp11::mp_find<list_type, State>::value;

    static constexpr long root_index = tree.root ();
    using root_type = state_type<root_index>;

    // state traits

    template<long ID>
      requires (contains_c<ID>)
    static constexpr bool is_root_c = tree.is_root (ID);

    template<class State>
      requires (contains<State>)
    static constexpr bool is_root = is_root_c<state_index<State>>;

    template<long ID>
      requires (contains_c<ID>)
    static constexpr bool is_region_c = tree[ID].is_region;

    template<class State>
      requires (contains<State>)
    static constexpr bool is_region = is_region_c<state_index<State>>;

    template<long Ancestor, long Descendant>
      requires (contains_c<Ancestor>&& contains_c<Descendant>)
    static constexpr bool is_ancestor_c = tree.is_ancestor (Ancestor, Descendant);

    template<class Ancestor, class Descendant>
      requires (contains<Ancestor>&& contains<Descendant>)
    static constexpr bool is_ancestor = is_ancestor_c<state_index<Ancestor>, state_index<Descendant>>;

    // checked context alias

    template<class State>
      requires (contains<State>)
    using context = CC<machine_traits, State>;

    // checked state traits

    template<class State>
      requires (contains<State>)
    using state = CS<machine_traits, State>;

    // state concepts

    /**
     * @brief True if State has an entry action
     * @tparam State the state to query. Must be bound to Machine
     */
    template<class State>
      requires (contains<State>)
    static constexpr bool has_enter_action = requires (State & state, context<State> const& ctx)
    {
      state.enter (ctx);
    };

    /**
     * @brief True if State has an exit action
     * @tparam State the state to query. Must be bound to Machine
     */
    template<class State>
      requires (contains<State>)
    static constexpr bool has_exit_action = requires (State & state, context<State> const& ctx)
    {
      state.exit (ctx);
    };

    /**
     * @brief True if State has an exit action
     * @tparam State the state to query. Must be bound to Machine
     */
    template<class State>
      requires (contains<State>)
    static constexpr bool has_reenter_action = requires (State & state, context<State> const& ctx)
    {
      state.reenter (ctx);
    };

    /**
     * @brief True if Super has a reaction with the given arguments.
     * @tparam Super the state to query. Must be bound to Machine.
     * @tparam Leaf the current state of the Machine. Must be bound to Machine and descendant of Super.
     * @tparam Args the list of arguments to pass to the reaction.
     *
     * @note
     * This trait is True if Super has a reaction for the given arguments while the machine is in state Leaf.
     * Super mube be an ancestor of Leaf in the Machine hierarchy.
     */
    template<class Super, class Leaf, class... Args>
      requires (contains<Super>&& contains<Leaf>&& is_ancestor<Super, Leaf>)
    static constexpr bool has_reaction_for = requires (Super & super, context<Leaf> const& ctx, Args&&... args)
    {
      // clang-format off
      { super.react(ctx, std::forward<Args>(args)...) } -> TransitResult;
      // clang-format on
    };

    /**
     * @brief True if Super has a reaction guard with the given arguments.
     * @tparam Super the state to query. Must be bound to Machine.
     * @tparam Leaf the current state of the Machine. Must be bound to Machine and descendant of Super.
     * @tparam Args the list of arguments to pass to the reaction.
     */
    template<class Super, class Leaf, class... Args>
      requires (contains<Super>&& contains<Leaf>&& is_ancestor<Super, Leaf>)
    static constexpr bool has_guard_for = requires (Super & super, context<Leaf> const& ctx, Args&&... args)
    {
      // clang-format off
      { super.guard(ctx, std::forward<Args>(args)...) } -> std::same_as<bool>;
      // clang-format on
    };

    /**
     * @brief Finds the set of states that can react to Args when the machine is in CurrState.
     * @tparam CurrState the active state of the Machine.
     * @tparam Args the arguments to react to
     *
     * @note
     * A Machine can be in more than active state, therefore the list returned by this
     * trait isn't exhaustive.
     * Reactants are returned in reverse order of depth. The most derived reactant comes first.
     */
    template<class CurrState, class... Args>
      requires (contains<CurrState>)
    static constexpr auto find_reactants = [] () {
      constexpr long curr_index = state_index<CurrState>;

      constexpr auto root_curr_path = tree.path (curr_index, root_index);
      auto result = root_curr_path;
      result.clear ();

      meta::for_each<root_curr_path> ([&] (auto parent_index) {
        if constexpr (has_reaction_for<state_type<parent_index>, CurrState, Args...>)
          result.push_back (parent_index);
      });

      return result;
    }();

    template<class... Args>
    static constexpr auto find_all_reactants = [] () {
      constexpr long N = binding_size * binding_size;
      auto result = container::literal_vector<container::literal_pair<long, long>, N> ();
      meta::for_each<tree.subtree_inclusive (root_index)> ([&] (auto leaf) {
        using leaf_type = state_type<leaf>;
        meta::for_each<find_reactants<leaf_type, Args...>> ([&] (auto super) {
          using super_type = state_type<super>;
          result.push_back ({leaf, super});
        });
      });

      return result;
    }();

    template<class... Args>
    static constexpr bool can_react = find_all_reactants<Args...>.size () > 0;
  };
}; // namespace forest::machine

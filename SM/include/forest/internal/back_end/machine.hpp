#pragma once
#include <cassert>
#include <forest/internal/back_end/base.hpp>
#include <forest/internal/meta/for_each.hpp>
#include <iostream>

namespace forest::internal
{
  template<StateTree auto Tree, std::move_constructible GlobalStorage, List StateList>
  struct machine_description
  {
    using state_list_type = StateList;
    using state_tuple_type = boost::mp11::mp_rename<state_list_type, std::tuple>;
    using state_bitset_type = std::array<bool, std::tuple_size_v<state_tuple_type>>;
    using global_storage_type = GlobalStorage;
    using instance = machine_instance<Tree, GlobalStorage, StateList>;

    static constexpr long state_list_size = std::tuple_size_v<state_tuple_type>;

    template<class State>
      requires (traits::contains<instance, State>)
    using context = internal::context<instance, State>;

    template<class State>
      requires (traits::contains<instance, State>)
    using state = internal::state<instance, State>;

    template<class Target>
      requires (traits::contains<instance, Target>)
    using transit = internal::transit<Target>;

    using transit_none = internal::transit_none;

    template<class... Targets>
      requires (traits::contains<instance, Targets>&&...)
    using transit_result = internal::transit_result<Targets...>;
  };

  template<StateTree auto Tree, std::move_constructible GlobalStorage, List StateList>
  class machine_instance
  {
  public:
    using state_list_type = StateList;
    using state_tuple_type = boost::mp11::mp_rename<state_list_type, std::tuple>;
    using state_bitset_type = std::array<bool, std::tuple_size_v<state_tuple_type>>;
    using global_storage_type = GlobalStorage;

    static constexpr long state_list_size = std::tuple_size_v<state_tuple_type>;

    template<class State>
      requires (traits::contains<machine_instance, State>)
    using context = internal::context<machine_instance, State>;

    template<class State>
      requires (traits::contains<machine_instance, State>)
    using state = internal::state<machine_instance, State>;

    template<class Target>
      requires (traits::contains<machine_instance, Target>)
    using transit = internal::transit<Target>;

    using transit_none = internal::transit_none;

    template<class... Targets>
      requires (traits::contains<machine_instance, Targets>&&...)
    using transit_result = internal::transit_result<Targets...>;

  private:
    state_tuple_type _state_storage {};
    state_bitset_type _active_bitset {};
    global_storage_type _global_storage;

  public:
    machine_instance () = default;

    // TODO: constructors

    // Observers

    template<long Index>
    [[nodiscard]] constexpr bool is_active () const //
      requires (traits::contains_c<machine_instance, Index>);

    template<class State>
    [[nodiscard]] constexpr bool is_active () const //
      requires (traits::contains<machine_instance, State>);

    template<long Index>
      [[nodiscard]] constexpr context<traits::state_type<machine_instance, Index>> get_context () & //
      requires (traits::contains_c<machine_instance, Index>)
    {
      using state_type = traits::state_type<machine_instance, Index>;
      return context<state_type> {*this};
    }

    // React

    template<class... Args>
    constexpr bool react (Args&&... args) &;

    // Start

    constexpr void start () &
    {
      constexpr long root_index = traits::root_c<machine_instance>;
      // assert: root_index is not active
      assert (!_active_bitset[root_index]);
      this->_enter<root_index> ();
    }

    // Stop

    constexpr void stop () &
    {
      constexpr long root_index = traits::root_c<machine_instance>;
      // assert: root_index is active
      assert (_active_bitset[root_index]);
      this->_deactivate_subtree<root_index> ();
      this->_deactivate_leaf<root_index> ();
    }

    // State cast

    template<long Index>
      requires (traits::contains_c<machine_instance, Index>)
    [[nodiscard]] constexpr auto& state_cast () & //
    {
      return std::get<Index> (_state_storage);
    }

    template<long Index>
      requires (traits::contains_c<machine_instance, Index>)
    [[nodiscard]] constexpr auto const& state_cast () const& //
    {
      return std::get<Index> (_state_storage);
    }

    template<class State>
      requires (traits::contains<machine_instance, State>)
    [[nodiscard]] constexpr State& state_cast () & //
    {
      return std::get<traits::state_index<machine_instance, State>> (_state_storage);
    }

    template<class State>
      requires (traits::contains<machine_instance, State>)
    [[nodiscard]] constexpr State& state_cast () const& //
    {
      return std::get<traits::state_index<machine_instance, State>> (_state_storage);
    }

  private:
    template<long Index>
    constexpr void _deactivate_leaf () &;

    template<long Index>
    constexpr void _deactivate_subtree () &;

    template<long Index>
    constexpr void _reenter () &;

    template<long Index>
    constexpr void _enter () &;

    template<long From, long To>
    constexpr void _transit () &;

    template<long From, class... Args>
    constexpr void _react (Args&&... args) &;

    template<long Index>
    constexpr bool _is_leaf () const;

    template<long Index>
    constexpr void _activate_subregion () &;
  };

  // --- Observers

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  [[nodiscard]] constexpr bool machine_instance<Tree, GS, SL>::is_active () const //
    requires (traits::contains_c<machine_instance, Index>)
  {
    return this->_active_bitset[Index];
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<class State>
  [[nodiscard]] constexpr bool machine_instance<Tree, GS, SL>::is_active () const //
    requires (traits::contains<machine_instance, State>)
  {
    constexpr long index = traits::state_index<machine_instance, State>;
    return this->_active_bitset[index];
  }

  // --- Helpers

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  constexpr void machine_instance<Tree, GS, SL>::_deactivate_leaf () &
  {
    // assert: Index is active.
    // assert: None of Index children is active.
    assert (_active_bitset[Index]);
    lt_for_each_child<Tree, Index> ([this] (auto I) {
      assert (!_active_bitset[I]);
    });

    using state_type = traits::state_type<machine_instance, Index>;
    if constexpr (traits::state_has_exit_action<state_type>) {
      state_type& state = std::get<Index> (_state_storage);
      state.on_exit (this->get_context<Index> ());
    }

    _active_bitset[Index] = false;
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  constexpr void machine_instance<Tree, GS, SL>::_deactivate_subtree () &
  {
    // assert: Index is active
    assert (_active_bitset[Index]);

    lt_for_each_in_subtree<Tree, Index> ([this] (auto curr_index) {
      if constexpr (curr_index != Index) {
        if (_active_bitset[curr_index])
          this->_deactivate_leaf<curr_index> ();
      }
    });
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  constexpr void machine_instance<Tree, GS, SL>::_reenter () &
  {
    // assert: Index is active
    assert (_active_bitset[Index]);

    using state_type = traits::state_type<machine_instance, Index>;
    if constexpr (traits::state_has_reenter_action<state_type>) {
      state_type& state = std::get<Index> (_state_storage);
      state.on_reenter (this->get_context<Index> ());
    }
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  constexpr void machine_instance<Tree, GS, SL>::_enter () &
  {
    // assert: Index is not active
    assert (_active_bitset[Index] == false);

    using state_type = traits::state_type<machine_instance, Index>;
    if constexpr (traits::state_has_entry_action<state_type>) {
      state_type& state = std::get<Index> (_state_storage);
      state.on_entry (this->get_context<Index> ());
    }
    _active_bitset[Index] = true;

    if constexpr (traits::is_region_c<machine_instance, Index>) {
      lt_for_each_child<Tree, Index> ([this] (auto I) {
        if (!_active_bitset[I])
          this->_enter<I> ();
      });
    }
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long From, long To>
  constexpr void machine_instance<Tree, GS, SL>::_transit () & //
  {
    // assert: From is a leaf active state
    assert (_active_bitset[From]);
    assert (this->_is_leaf<From> ());

    static_assert (!traits::is_cross_transition_c<machine_instance, From, To>);

    if constexpr (From == To) {
      this->_reenter<From> ();
      return;
    } else {
      constexpr long lca = lt_lca<Tree, From, To>;

      auto const descend = [this] () {
        lt_for_each_in_path<Tree, lca, To> ([&] (auto I) {
          if constexpr (lca != I) {
            if (!_active_bitset[I])
              this->_enter<I> ();
          }
        });
      };

      if constexpr (lca == From) {
        // only descend
        descend ();
      } else {
        // destruct path to the lca, then descend
        constexpr long before_lca = lt_before_lca<Tree, From, To>;
        this->_deactivate_subtree<before_lca> ();
        this->_deactivate_leaf<before_lca> ();
        descend ();
      }
    }
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Leaf, class... Args>
  constexpr void machine_instance<Tree, GS, SL>::_react (Args&&... args) & //
  {
    constexpr long SuperIndex = traits::hierarchy_find_reaction_handler<machine_instance, Leaf, Args...>;
    using super_type = traits::state_type<machine_instance, SuperIndex>;
    super_type& super = std::get<SuperIndex> (_state_storage);

    traits::TransitResult auto result = super.react (this->get_context<Leaf> (), std::forward<Args> (args)...);

    result.visit ([this]<traits::Transit T> (T) {
      if constexpr (!std::same_as<T, transit_none>) {
        constexpr long target_index = traits::state_index<machine_instance, traits::transit_target_t<T>>;
        this->_transit<Leaf, target_index> ();
      }
    });
  }

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<long Index>
  constexpr bool machine_instance<Tree, GS, SL>::_is_leaf () const
  {
    bool result = _active_bitset[Index];

    lt_for_each_child<Tree, Index> ([this, &result] (auto I) {
      if (_active_bitset[I])
        result = false;
    });

    return result;
  }

  // React

  template<StateTree auto Tree, std::move_constructible GS, List SL>
  template<class... Args>
  constexpr bool machine_instance<Tree, GS, SL>::react (Args&&... args) &
  {
    bool reacted = false;

    constexpr auto possible_sources = traits::reaction_sources<machine_instance, Args...>;

    std::cerr << "Possible sources: ";
    for (long x : possible_sources)
      std::cerr << x << " ";
    std::cerr << "\n";

    consteval_for_each_range<possible_sources> ([&, this] (auto I) {
      if (!reacted && this->_is_leaf<I> ()) {
        reacted = true;
        this->_react<I> (std::forward<Args> (args)...);
      }
    });

    return reacted;
  }

} // namespace forest::internal
#pragma once
#include <forest/machine/backend/context.hpp>
#include <forest/machine/backend/core.hpp>
#include <forest/machine/backend/state.hpp>
#include <forest/machine/backend/transit.hpp>

#include <cassert>
#include <iostream>

namespace forest::machine
{
  template<MachineTraits Traits>
  class machine_reference
  {
  public:
    using list_type = typename Traits::list_type;
    using tuple_type =
      boost::mp11::mp_transform<std::add_lvalue_reference_t, boost::mp11::mp_rename<list_type, std::tuple>>;
    using storage_type = typename Traits::storage_type;
    using bitset_type = std::array<bool, Traits::binding_size>;

    tuple_type _states;
    bitset_type& _active;
    storage_type& _storage;

    constexpr machine_reference (tuple_type _states, bitset_type& _active, storage_type& _storage)
      : _states (_states)
      , _active (_active)
      , _storage (_storage)
    {}

    // observers

    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _active[ID];
    }

    template<class State>
      requires (Traits::template contains<State>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _active[Traits::template state_index<State>];
    }

    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr auto& state_cast () const
    {
      return std::get<ID> (_states);
    }

    template<class State>
      requires (Traits::template contains<State>)
    [[nodiscard]] constexpr auto& state_cast () const
    {
      return std::get<Traits::template state_index<State>> (_states);
    }

    [[nodiscard]] constexpr auto& storage () const
    {
      return _storage;
    }
  };

  template<MachineTraits Traits>
  class machine_instance
  {
  public:
    using tuple_type = boost::mp11::mp_rename<typename Traits::list_type, std::tuple>;
    using bitset_type = std::array<bool, Traits::binding_size>;
    using storage_type = typename Traits::storage_type;
    using reference = machine_reference<Traits>;

    tuple_type _states {};
    bitset_type _active {};
    storage_type _storage;

    // constructors

    machine_instance () = default;

    constexpr machine_instance (storage_type init)
      : _states {}
      , _active {}
      , _storage (std::move (init))
    {}

    // observers

    /**
     * @brief Returns true if the state with index ID is currently active
     * @tparam ID the state index
     */
    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _active[ID];
    }

    /**
     * @brief Returns true if the state State is currently active
     * @tparam State the state to query
     */
    template<class State>
      requires (Traits::template contains<State>)
    [[nodiscard]] constexpr bool is_active () const
    {
      return _active[Traits::template state_index<State>];
    }

    /**
     * @brief Returns a reference to state ID
     * @tparam ID the state index
     */
    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr auto& state_cast ()
    {
      return std::get<ID> (_states);
    }

    /**
     * @brief Returns a reference to State
     * @tparam State
     */
    template<class State>
      requires (Traits::template contains<State>)
    [[nodiscard]] constexpr auto& state_cast ()
    {
      return std::get<Traits::template state_index<State>> (_states);
    }

    // reactions

    template<class... Args>
      requires (Traits::template can_react<Args...>)
    constexpr bool react (Args&&... args)
    {
      return meta::for_each<Traits::template find_all_reactants<Args...>> ([&] (auto pair) {
        constexpr long leaf = pair.value.first;
        constexpr long super = pair.value.second;
        using leaf_type = typename Traits::template state_type<leaf>;
        using super_type = typename Traits::template state_type<super>;

        if (!this->_is_leaf<leaf> () || !this->_check_guard<super_type, leaf_type> (args...))
          return false;

        this->_react<super, leaf> (std::forward<Args> (args)...);
        return true;
      });
    }

    constexpr void start ()
    {
      this->_enter<Traits::root_index> ();
    }

    constexpr void stop ()
    {
      this->_exit_subtree<Traits::root_index> ();
      this->_exit<Traits::root_index> ();
    }

    /**
     * @brief Returns the context object for ID
     * @tparam ID the state index
     */
    template<long ID>
      requires (Traits::template contains_c<ID>)
    [[nodiscard]] constexpr auto get_context ()
    {
      return this->get_context<typename Traits::template state_type<ID>> ();
    }

    /**
     * @brief Returns the context object for State
     * @tparam ID the state index
     */
    template<class State>
      requires (Traits::template contains<State>)
    [[nodiscard]] constexpr auto get_context ()
    {
      using context_type = typename Traits::template context<State>;
      return context_type ({this->_states_ref (), _active, _storage});
    }

  private:
    template<class Super, class Leaf, class... Args>
    [[nodiscard]] constexpr bool _check_guard (Args&&... args)
    {
      if constexpr (Traits::template has_guard_for<Super, Leaf, Args...>) {
        return state_cast<Super> ().guard (get_context<Leaf> (), args...);
      } else {
        return true;
      }
    }

    template<long ID>
    [[nodiscard]] constexpr bool _is_leaf ()
    {
      bool result = this->_active[ID];
      meta::for_each<Traits::tree.children (ID)> ([this, &result] (auto Child) {
        result &= !this->_active[Child];
      });
      return result;
    }

    template<long ID>
    constexpr void _enter ()
    {
      assert (_active[ID] == false);
      using state_type = typename Traits::template state_type<ID>;

      if constexpr (Traits::template has_enter_action<state_type>) {
        this->state_cast<ID> ().enter (this->get_context<ID> ());
      }

      this->_active[ID] = true;

      if constexpr (Traits::template is_region<state_type>) {
        constexpr auto children = Traits::tree.children (ID);
        forest::meta::for_each<children> ([this] (auto C) {
          assert (this->_active[C] == false);
          this->_enter<C> ();
        });
      }
    }

    template<long LCA, long To>
    constexpr void _enter_path ()
    {
      static_assert (Traits::template is_ancestor_c<LCA, To>);

      assert (this->_active[LCA]);
      forest::meta::for_each<Traits::tree.path (LCA, To)> ([this] (auto Curr) {
        if constexpr (Curr != LCA)
          if (!this->_active[Curr])
            this->_enter<Curr> ();
      });
    }

    template<long ID>
    constexpr void _exit ()
    {
      assert (this->_is_leaf<ID> ());
      using state_type = typename Traits::template state_type<ID>;
      if constexpr (Traits::template has_exit_action<state_type>)
        this->state_cast<ID> ().exit (this->get_context<ID> ());

      this->_active[ID] = false;
    }

    template<long ID>
    constexpr void _exit_subtree ()
    {
      assert (this->_active[ID]);

      forest::meta::for_each<Traits::tree.subtree_exclusive (ID)> ([this] (auto Child) {
        if (this->_active[Child])
          this->_exit<Child> ();
      });
    }

    template<long ID>
    constexpr void _reenter ()
    {
      assert (this->_active[ID]);

      using state_type = typename Traits::template state_type<ID>;
      if constexpr (Traits::template has_reenter_action<state_type>)
        this->state_cast<ID> ().reenter (this->get_context<ID> ());
    }

    template<long From, long To>
    constexpr void _transit ()
    {
      assert (this->_is_leaf<From> ());

      if constexpr (From == To) {
        this->_reenter<From> ();
      } else {
        constexpr long lca = Traits::tree.find_lca (From, To);
        if constexpr (lca == From) {
          this->_enter_path<lca, To> ();
        } else {
          static_assert (!Traits::template is_region_c<lca>);

          constexpr long lca_pred = Traits::tree.find_lca_pred (From, To).first;
          this->_exit_subtree<lca_pred> ();
          this->_exit<lca_pred> ();
          this->_enter_path<lca, To> ();
        }
      }
    }

    template<long Super, long Leaf, class... Args>
    constexpr void _react (Args&&... args)
    {
      auto result = this->state_cast<Super> ().react (this->get_context<Leaf> (), std::forward<Args> (args)...);
      result.visit ([this]<Transit T> (T) {
        if constexpr (!std::same_as<T, transit<void>>) {
          using transit_type = typename T::type;
          constexpr long transit_index = Traits::template state_index<transit_type>;
          this->_transit<Leaf, transit_index> ();
        }
      });
    }

    [[nodiscard]] constexpr auto _states_ref ()
    {
      auto apply = [] (auto&... args) {
        return std::forward_as_tuple (args...);
      };
      return std::apply (apply, _states);
    }
  };
} // namespace forest::machine
#pragma once
#include <concepts>
#include <optional>
#include <tuple>
#include <utility>
#include <variant>

namespace forest::sm
{
  // TODO: implement timer methods

  template<class T>
  concept Context = requires (T context)
  {
    typename T::state_machine_type;

    requires std::copy_constructible<T>;
  };

  // todo: implement TimedState

  template<class T>
  concept State = requires (T state)
  {
    requires std::move_constructible<T>;
  };

  template<class T, class C>
  concept StateWithEntryAction = State<T> && Context<C> && requires (T state, C context)
  {
    // clang-format off
    { state.on_entry(context) } -> std::same_as<void>;
    // clang-format on
  };

  template<class T, class C>
  concept StateWithExitAction = State<T> && Context<C> && requires (T state, C context)
  {
    // clang-format off
    { state.on_exit(context) } -> std::same_as<void>;
    // clang-format on
  };

  template<class T>
  concept Event = requires (T event)
  {
    requires std::move_constructible<T>;
  };

  template<class T, class C, class S, class E>
  concept Guard = Context<C> && State<S> && Event<E> && requires (T guard, C context, S state, E event)
  {
    // clang-format off
    { guard(context, state, event) } -> std::same_as<bool>;
    // clang-format on
  };

  template<class T, class C, class S, class E>
  concept InternalTransition = Context<C> && State<S> && Event<E> &&
    requires (T transition, C context, S state, E event)
  {
    // clang-format off
    { transition(context, state, event) } -> std::same_as<void>;
    // clang-format on
  };

  template<class T, class C, class S, class E>
  concept ExternalTransition = Context<C> && State<S> && Event<E> &&
    requires (T transition, C context, S state, E event)
  {
    // clang-format off
    { transition(context, state, event) } -> State;
    // clang-format on
  };

  template<class T, class C, class S, class E>
  concept Transition = InternalTransition<T, C, S, E> || ExternalTransition<T, C, S, E>;

  template<class T, class C, class S, class E>
  concept GuardedTransition = Transition<T, C, S, E> && requires (T transition, C context, S state, E event)
  {
    // clang-format off
    { transition.accepts(context, state, event) } -> std::same_as<bool>;
    // clang-format on
  };

  /**
   * Helper class that adapts a transition and a guard to make a GuardedTransition.
   */
  template<std::move_constructible T, std::move_constructible G>
  class guarded_transition
  {
  private:
    T _base;
    G _guard;

  public:
    constexpr guarded_transition (T base, G guard)
      : _base (std::move (base))
      , _guard (std::move (guard))
    {}

    template<Context C, State S, Event E>
      requires (Guard<G, C, S, E>)
    [[nodiscard]] constexpr bool accepts (C ctx, S& state, E event)
    {
      return _guard (ctx, state, event);
    }

    template<Context C, State S, Event E>
      requires (Transition<T, C, S, E>&& Guard<G, C, S, E>)
    constexpr auto operator() (C ctx, S& state, E event)
    {
      return _base (ctx, state, event);
    }
  };

  template<class T, class G>
  guarded_transition (T, G) -> guarded_transition<T, G>;

  /**
   * Helper type trait returns the result of a transition.
   * If the argument is an InternalTransition, returns void.
   */
  template<class T, Context C, State S, Event E>
    requires (Transition<T, C, S, E>)
  using transition_result_t = std::invoke_result_t<T&, C, S&, E>;

  template<std::move_constructible... Ts>
  struct transition_table
  {
  private:
    using tuple_type = std::tuple<Ts...>;
    tuple_type storage;

  public:
    constexpr transition_table (Ts... init)
      : storage (std::move (init)...)
    {}

    template<std::size_t I>
    friend constexpr std::tuple_element_t<I, tuple_type>& get (transition_table& table)
    {
      return std::get<I> (table.storage);
    }

    template<std::size_t I>
    friend constexpr std::tuple_element_t<I, tuple_type> const& get (transition_table const& table)
    {
      return std::get<I> (table.storage);
    }

    template<std::size_t I>
    friend constexpr std::tuple_element_t<I, tuple_type>&& get (transition_table&& table)
    {
      return std::move (std::get<I> (table.storage));
    }

    template<std::size_t I>
    friend constexpr std::tuple_element_t<I, tuple_type> const&& get (transition_table const&&) = delete;
  };

  template<class... Ts>
  transition_table (Ts...) -> transition_table<Ts...>;

  /**
   * Invokes fn with each member of the transition table as single parameter
   * fn(transition0), fn(transition1), ...
   */
  template<class T, std::move_constructible F>
  constexpr void for_each_transition (T&& table, F fn)
  {
    constexpr std::size_t size = std::tuple_size_v<std::remove_cvref_t<T>>;

    auto const invoke = [&]<std::size_t... Indices> (std::index_sequence<Indices...>)
    {
      using std::get;
      (fn (get<Indices> (table)), ...);
    };

    invoke (std::make_index_sequence<size> ());
  }

  struct transition_result
  {
    bool triggered;
    bool state_changed;
  };

  // TODO: handle TimedState

  template<class T, Context C, State S, Event E, class Storage>
  constexpr transition_result trigger_transition (T&& table, C context, S& state, E event, Storage& storage)
  {
    auto const trigger = [&]<class TransitionType> (TransitionType& transition) -> bool {
      if constexpr (InternalTransition<TransitionType, C, S, E>) {
        // Trigger the transition and don't change the state
        transition (std::move (context), state, std::move (event));
        return false;
      } else {
        using new_state_type = transition_result_t<TransitionType, S, C, E>;

        // Perform the on-exit action before triggering the transition
        if constexpr (StateWithExitAction<S, C>)
          state.on_exit (context);

        // Trigger the transition and store the result
        new_state_type new_state = transition (context, state, std::move (event));

        // Perform the on-entry action
        if constexpr (StateWithEntryAction<new_state_type, C>)
          new_state.on_entry (context);

        // Change the state
        storage = std::move (new_state);
        return true;
      }
    };

    auto const check_guard = [&]<Transition<C, S, E> TransitionType> (TransitionType& transition) {
      if constexpr (GuardedTransition<TransitionType, C, S, E>) {
        return transition.accepts (context, state, event);
      } else {
        return true;
      }
    };

    transition_result result {};
    for_each_transition (table, [&]<class TransitionType> (TransitionType& transition) {
      if constexpr (Transition<TransitionType, C, S, E>) {
        if (!result.triggered && check_guard (transition)) {
          result.triggered = true;
          result.state_changed = trigger (transition);
        }
      }
    });

    return result;
  }

  struct initialization_state
  {};

  struct shutdown_state
  {};

  struct initialization_event
  {};

  struct shutdown_event
  {};

  namespace details
  {
    template<class...>
    struct type_list;
  }

  template<class TransitionList, class StateList>
  class state_machine;

  template<std::move_constructible... Transitions, State... States>
  class state_machine<details::type_list<Transitions...>, details::type_list<States...>>
  {
  private:
    template<class... Ts>
    struct first_type;

    template<class T, class... Ts>
    struct first_type<T, Ts...> : std::type_identity<T>
    {};

    template<class... Ts>
    using first_t = typename first_type<Ts...>::type;

  public:
    static_assert (sizeof...(States) > 0);
    static_assert (sizeof...(Transitions) > 0);

    struct context_type;
    using transition_table = forest::sm::transition_table<Transitions...>;
    using state_type = std::variant<initialization_state, shutdown_state, States...>;

    static constexpr bool has_initialization_transition = //
      (Transition<Transitions, context_type, initialization_state&, initialization_event> || ...);

    template<State S>
    static constexpr bool has_shutdown_transition = //
      (Transition<Transitions, context_type, S&, shutdown_event> || ...);

    constexpr state_machine (Transitions... transitions) //
      : _table (std::move (transitions)...)
      , _state (initialization_state ())
    {}

    /**
     * Returns true if the current state is S
     */
    template<State S>
      requires ((std::same_as<S, States> || ...) //
        || std::same_as<S, initialization_state> //
        || std::same_as<S, shutdown_state>)
    [[nodiscard]] constexpr bool is () const
    {
      return std::holds_alternative<S> (_state);
    }

    /**
     * Returns the current state by reference, assuming it is S.
     */
    template<State S>
      requires ((std::same_as<S, States> || ...) //
        || std::same_as<S, initialization_state> //
        || std::same_as<S, shutdown_state>)
    [[nodiscard]] constexpr S& current_state ()
    {
      return std::get<S> (_state);
    }

    /**
     * Returns the current state by const reference, assuming it is S.
     */
    template<State S>
      requires ((std::same_as<S, States> || ...) //
        || std::same_as<S, initialization_state> //
        || std::same_as<S, shutdown_state>)
    [[nodiscard]] constexpr S const& current_state () const
    {
      return std::get<S> (_state);
    }

    constexpr transition_result start ()
    {
      // TODO: allow custom initialization
      if constexpr (has_initialization_transition) {
        return process_event (initialization_event {});
      } else {
        using first_state = first_t<States...>;
        static_assert (std::default_initializable<first_state>);

        context_type context {*this};
        _state = first_state {};
        if constexpr (StateWithEntryAction<first_state, context_type>)
          current_state<first_state> ().on_entry (context);
        return {.triggered = true, .state_changed = true};
      }
    }

    constexpr transition_result stop () //
    {
      // TODO: allow custom shutdown
      auto const handler = [&]<State S> (S& current) {
        if constexpr (has_shutdown_transition<S>) {
          return process_event (shutdown_event {});
        } else {
          context_type context {*this};
          if constexpr (StateWithExitAction<S, context_type>)
            current.on_exit (context);
          _state = shutdown_state {};
          return transition_result {.triggered = true, .state_changed = true};
        }
      };

      return std::visit (handler, _state);
    }

    template<Event E>
    constexpr transition_result process_event (E event)
    {
      auto const handler = [&]<State S> (S& current) {
        context_type context {*this};
        return trigger_transition (_table, std::move (context), current, std::move (event), _state);
      };

      return std::visit (handler, _state);
    }

  private:
    transition_table _table;
    state_type _state;
  };

  template<std::move_constructible... Transitions, State... States>
  class state_machine<details::type_list<Transitions...>, details::type_list<States...>>::context_type
  {
    using state_machine_type = state_machine;
    std::reference_wrapper<state_machine_type> sm;
  };

} // namespace forest::sm

namespace std
{
  template<class... Ts>
  struct tuple_size<forest::sm::transition_table<Ts...>> : std::tuple_size<std::tuple<Ts...>>
  {};

  template<std::size_t I, class... Ts>
  struct tuple_element<I, forest::sm::transition_table<Ts...>> : std::tuple_element<I, std::tuple<Ts...>>
  {};
} // namespace std

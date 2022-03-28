#pragma once
#include <forest/internal/container/vector.hpp>
#include <forest/internal/meta/for_each.hpp>
#include <iostream>
#include <span>

namespace forest::internal
{
  template<long NumVertices, long NumArcs, std::semiregular NodeData = std::nullptr_t>
  struct literal_tree
  {
  public:
    std::array<long, NumVertices> _parents {};
    std::array<long, NumVertices> _arcs_begin {};
    std::array<long, NumVertices> _arcs_size {};
    std::array<NodeData, NumVertices> _nodes {};
    std::array<long, NumArcs> _arcs {};

    std::array<long, NumVertices * 2> _euler_tour {};
    std::array<long, NumVertices * 2> _euler_tour_start {};
    std::array<long, NumVertices * 2> _euler_tour_end {};

    long _num_arcs {};
    long _num_vertices {};

    literal_tree () = default;

    template<std::input_iterator It1, std::sentinel_for<It1> It2, std::indirectly_regular_unary_invocable<It1> P>
    constexpr literal_tree (It1 first, It2 last, P proj) noexcept
      : literal_tree ()
    {
      // initialize arrays

      std::ranges::fill (_parents, -1);
      std::ranges::fill (_arcs_begin, -1);

      // compute number of arcs

      _num_arcs = std::ranges::distance (first, last);

      // sort arcs

      std::array<std::iter_value_t<std::projected<It1, P>>, NumArcs> temp_arcs {};
      std::ranges::transform (first, last, temp_arcs.begin (), proj);
      std::ranges::sort (temp_arcs.begin (), temp_arcs.begin () + _num_arcs);
      std::ranges::transform (temp_arcs.begin (), temp_arcs.begin () + _num_arcs, _arcs.begin (), [] (auto& curr) {
        return get<1> (curr);
      });

      // compute number of vertices

      for (auto& arc : temp_arcs)
        _num_vertices = std::ranges::max (_num_vertices, static_cast<long> (get<1> (arc) + 1));

      // compute arc intervals

      std::ranges::fill (_arcs_begin, _num_arcs);
      for (long i = 0; i < _num_arcs; ++i) {
        long source = get<0> (temp_arcs[i]);
        long target = get<1> (temp_arcs[i]);

        _parents[target] = source;
        _arcs_size[source] += 1;
        if (_arcs_begin[source] == _num_arcs)
          _arcs_begin[source] = i;
      }

      // compute euler tour
      long et_index = 0;
      auto const make_euler_tour = [&] (auto& self, long curr) -> void {
        _euler_tour[et_index] = curr;
        _euler_tour_start[curr] = et_index;
        ++et_index;

        for (long child : children_of (curr))
          self (self, child);

        _euler_tour[et_index] = curr;
        _euler_tour_end[curr] = et_index;
        ++et_index;
      };
      make_euler_tour (make_euler_tour, root ());
    }

    template<std::input_iterator It1, std::sentinel_for<It1> It2>
    constexpr literal_tree (It1 first, It2 last) noexcept
      : literal_tree (first, last, std::identity {})
    {}

    template<std::ranges::input_range T, std::regular_invocable<std::ranges::range_reference_t<T>> P>
    constexpr literal_tree (T&& rng, P proj)
      : literal_tree (std::ranges::begin (rng), std::ranges::end (rng), std::move (proj))
    {}

    template<std::ranges::input_range T>
    constexpr literal_tree (T&& rng)
      : literal_tree (std::forward<T> (rng), std::identity {})
    {}

    // ---

    [[nodiscard]] constexpr long num_vertices () const noexcept
    {
      return _num_vertices;
    }

    [[nodiscard]] constexpr long num_arcs () const noexcept
    {
      return _num_arcs;
    }

    // ---

    [[nodiscard]] constexpr long root () const noexcept
    {
      for (long i = 0; i < num_vertices (); ++i)
        if (_parents[i] == -1)
          return i;
      return -1;
    }

    // ---

    [[nodiscard]] constexpr long parent_of (long index) const noexcept
    {
      return _parents[index];
    }

    [[nodiscard]] constexpr std::span<long const> children_of (long index) const noexcept
    {
      long begin = _arcs_begin[index];
      long size = _arcs_size[index];
      return std::span (_arcs.data () + begin, size);
    }

    // ---

    [[nodiscard]] constexpr NodeData& operator[] (long index) noexcept
    {
      return _nodes[index];
    }

    [[nodiscard]] constexpr NodeData const& operator[] (long index) const noexcept
    {
      return _nodes[index];
    }

    // ---

    [[nodiscard]] constexpr bool is_ancestor_of (long ancestor, long child) const noexcept
    {
      if (ancestor == child)
        return true;

      while (_parents[child] != -1) {
        child = _parents[child];
        if (child == ancestor)
          return true;
      }

      return false;
    }

    [[nodiscard]] constexpr long depth_of (long index) const noexcept
    {
      long result = 0;
      while (_parents[index] != -1) {
        index = _parents[index];
        result += 1;
      }
      return result;
    }

    [[nodiscard]] constexpr long lowest_common_ancestor (long x, long y) const noexcept
    {
      auto visited = std::array<bool, NumVertices> ();
      auto depth1 = depth_of (x);
      auto depth2 = depth_of (y);
      while (depth1 > depth2) {
        x = _parents[x];
        depth1 -= 1;
      }
      while (depth2 > depth1) {
        y = _parents[y];
        depth2 -= 1;
      }
      while (x != y) {
        x = _parents[x];
        y = _parents[y];
      }
      return x;
    }

    [[nodiscard]] constexpr long before_lowest_common_ancestor (long x, long y) const noexcept
    {
      if (x == y)
        return -1;

      auto lca = lowest_common_ancestor (x, y);
      while (parent_of (x) != lca)
        x = parent_of (x);
      return x;
    }

    [[nodiscard]] constexpr long path_length (long x, long y) const noexcept
    {
      return depth_of (x) + depth_of (y) - 2 * depth_of (lowest_common_ancestor (x, y));
    }

    template<long MaxLength = NumVertices>
    [[nodiscard]] constexpr std::array<long, MaxLength> path (long x, long y) const noexcept
    {
      long result_size = 0;
      auto result = std::array<long, MaxLength> {};
      std::ranges::fill (result, -1);

      auto lca = lowest_common_ancestor (x, y);

      while (x != lca) {
        result[result_size++] = x;
        x = _parents[x];
      }

      result[result_size++] = lca;
      auto mid = result_size;

      while (y != lca) {
        result[result_size++] = y;
        y = _parents[y];
      }

      std::ranges::reverse (result.begin () + mid, result.begin () + result_size);
      return result;
    }

    template<long MaxSize = NumVertices>
    [[nodiscard]] constexpr std::array<long, MaxSize> subtree_of (long x) const noexcept
    {
      long result_size = 0;
      std::array<long, MaxSize> result {};
      std::ranges::fill (result, -1);
      for (long i = _euler_tour_start[x]; i != _euler_tour_end[x]; ++i)
        if (_euler_tour_start[_euler_tour[i]] == i)
          result[result_size++] = _euler_tour[i];
      std::ranges::reverse (result.begin (), result.begin () + result_size);
      return result;
    }

    [[nodiscard]] constexpr long subtree_size (long x) const noexcept
    {
      return (_euler_tour_end[x] - _euler_tour_start[x] + 1) / 2;
    }
  };

  template<class T>
  constexpr inline bool is_literal_tree = false;

  template<long NumV, long NumA, class T>
  constexpr inline bool is_literal_tree<literal_tree<NumV, NumA, T>> = true;

  template<class T>
  concept LiteralTree = is_literal_tree<T>;

  // ---

  template<LiteralTree auto Tree>
  constexpr inline long lt_num_vertices = Tree.num_vertices ();

  template<LiteralTree auto Tree>
  constexpr inline long lt_num_arcs = Tree.num_arcs ();

  // ---

  template<LiteralTree auto Tree, long Query>
  constexpr inline bool lt_contains = Query >= 0 && Query < lt_num_vertices<Tree>;

  template<LiteralTree auto Tree>
  constexpr inline long lt_root = Tree.root ();

  template<LiteralTree auto Tree, long Query>
    requires (lt_contains<Tree, Query>)
  constexpr inline long lt_parent = Tree.parent_of (Query);

  template<LiteralTree auto Tree, long Parent, long Child>
    requires (lt_contains<Tree, Parent>&& lt_contains<Tree, Child>)
  constexpr inline bool ls_is_parent = Tree.parent_of (Child) == Parent;

  template<LiteralTree auto Tree, long Parent, long Child>
    requires (lt_contains<Tree, Parent>&& lt_contains<Tree, Child>)
  constexpr inline bool lt_is_ancestor = Tree.is_ancestor_of (Parent, Child);

  template<LiteralTree auto Tree, long Query>
    requires (lt_contains<Tree, Query>)
  constexpr inline long lt_depth = Tree.depth_of (Query);

  template<LiteralTree auto Tree, long Query1, long Query2>
    requires (lt_contains<Tree, Query1>&& lt_contains<Tree, Query2>)
  constexpr inline long lt_lca = Tree.lowest_common_ancestor (Query1, Query2);

  template<LiteralTree auto Tree, long Query1, long Query2>
    requires (lt_contains<Tree, Query1>&& lt_contains<Tree, Query2>&& Query1 != Query2)
  constexpr inline long lt_before_lca = Tree.before_lowest_common_ancestor (Query1, Query2);

  template<LiteralTree auto Tree, long Query>
    requires (lt_contains<Tree, Query>)
  constexpr inline auto lt_for_each_child = []<class F> (F callback) {
    constexpr auto children = Tree.children_of (Query);
    constexpr auto size = children.size ();
    consteval_for_each_range<make_array<size> (children)> (callback);
  };

  template<LiteralTree auto Tree, long Query>
    requires (lt_contains<Tree, Query>)
  constexpr inline auto lt_for_each_in_subtree = []<class F> (F callback) {
    constexpr auto subtree = Tree.subtree_of (Query);
    constexpr auto size = Tree.subtree_size (Query);
    consteval_for_each_range<make_array<size> (subtree)> (callback);
  };

  template<LiteralTree auto Tree, long From, long To>
    requires (lt_contains<Tree, From>&& lt_contains<Tree, To>)
  constexpr inline auto lt_for_each_in_path = []<class F> (F callback) {
    constexpr auto path = Tree.path (From, To);
    constexpr auto size = Tree.path_length (From, To) + 1;
    consteval_for_each_range<make_array<size> (path)> (callback);
  };

} // namespace forest::internal
#pragma once
#include <forest/container/literal_vector.hpp>
#include <forest/meta/for_each.hpp>
#include <span>

namespace forest::container
{
  template<long NVertices, long NArcs, std::semiregular NodeData>
  struct literal_tree
  {
    using element_type = NodeData;

    struct node
    {
      long parent;
      element_type data;
    };

    struct node_storage
    {
      std::array<node, NVertices> _nodes;

      template<class It, class Sen, class Proj>
      constexpr void init (It first, Sen last, Proj proj) &
      {
        std::ranges::fill (_nodes, node {.parent = -1, .data = {}});

        while (first != last) {
          auto arc = std::invoke (proj, *first);
          auto source = get<0> (arc);
          auto dest = get<1> (arc);
          _nodes[dest].parent = source;
          ++first;
        }
      }

      [[nodiscard]] constexpr node const& operator[] (long i) const
      {
        return _nodes[i];
      }

      [[nodiscard]] constexpr node& operator[] (long i)
      {
        return _nodes[i];
      }

      [[nodiscard]] constexpr long find_root () const
      {
        for (long i = 0; i < NVertices; ++i)
          if (_nodes[i].parent == -1)
            return i;
        return -1;
      }
    };

    struct arc_storage
    {
      std::array<long, NVertices> _first;
      std::array<long, NVertices> _size;
      std::array<long, NArcs> _arcs;

      template<class It, class Sen, class Proj>
      constexpr void init (It first, Sen last, Proj proj) &
      {
        std::ranges::fill (_first, NArcs);
        std::ranges::fill (_size, 0);
        std::ranges::fill (_arcs, -1);

        // sort arcs by (source, destination) and copy destinations to _arcs

        using projected_type = std::iter_value_t<std::projected<It, Proj>>;

        std::array<projected_type, NArcs> temp {};
        std::ranges::transform (first, last, temp.data (), proj);

        constexpr auto compare = [] (projected_type const& lhs, projected_type const& rhs) {
          if (get<0> (lhs) != get<0> (rhs))
            return get<0> (lhs) < get<0> (rhs);
          return get<1> (lhs) < get<1> (rhs);
        };

        std::ranges::sort (temp, compare);

        constexpr auto destination = [] (projected_type const& x) {
          return get<1> (x);
        };

        std::ranges::transform (temp, _arcs.data (), destination);

        // arcs are now initialized and sorted
        // compute arc intervals (for each source, the index of its first arc in _arcs, and the number of arcs)

        for (long i = 0; i < NArcs; ++i) {
          long source = get<0> (temp[i]);
          long dest = get<1> (temp[i]);

          _size[source] += 1;
          if (_first[source] == NArcs)
            _first[source] = i;
        }
      }

      [[nodiscard]] constexpr std::span<long const> operator[] (long i) const
      {
        return std::span (_arcs.data () + _first[i], _size[i]);
      }
    };

    struct tour_storage
    {
      std::array<long, NVertices> _start;
      std::array<long, NVertices> _end;
      std::array<long, NVertices * 2> _tour;

      constexpr void init (node_storage const& nodes, arc_storage const& arcs) &
      {
        recurse (arcs, 0, nodes.find_root ());
      }

      constexpr long recurse (arc_storage const& arcs, long pos, long curr) &
      {
        _tour[pos] = curr;
        _start[curr] = pos;
        ++pos;

        for (long child : arcs[curr])
          pos = recurse (arcs, pos, child);

        _tour[pos] = curr;
        _end[curr] = pos;
        return pos + 1;
      }
    };

    arc_storage _arcs;
    node_storage _nodes;
    tour_storage _tour;

    literal_tree () = default;

    template<std::forward_iterator It, std::sentinel_for<It> Sen, std::indirectly_regular_unary_invocable<It> Proj = std::identity>
    constexpr literal_tree (It first, Sen last, Proj proj = {})
    {
      _arcs.init (first, last, proj);
      _nodes.init (first, last, proj);
      _tour.init (_nodes, _arcs);
    }

    template<std::ranges::forward_range R, std::regular_invocable<std::ranges::range_reference_t<R>> Proj = std::identity>
    constexpr literal_tree (R&& range, Proj proj = {})
      : literal_tree (std::ranges::begin (range), std::ranges::end (range), std::move (proj))
    {}

    // access

    [[nodiscard]] constexpr element_type& operator[] (long i)
    {
      return _nodes[i].data;
    }

    [[nodiscard]] constexpr element_type const& operator[] (long i) const
    {
      return _nodes[i].data;
    }

    // observers

    [[nodiscard]] constexpr long num_vertices () const
    {
      return NVertices;
    }

    [[nodiscard]] constexpr long num_arcs () const
    {
      return NArcs;
    }

    [[nodiscard]] constexpr bool contains (long index) const
    {
      return index >= 0 && index < num_vertices ();
    }

    [[nodiscard]] constexpr long root () const
    {
      return _nodes.find_root ();
    }

    [[nodiscard]] constexpr long parent (long index) const
    {
      return _nodes[index].parent;
    }

    [[nodiscard]] constexpr long is_root (long index) const
    {
      return parent (index) == -1;
    }

    [[nodiscard]] constexpr long is_parent (long parent_idx, long child_idx) const
    {
      return parent (child_idx) == parent_idx;
    }

    [[nodiscard]] constexpr long is_ancestor (long ancestor_idx, long child_idx) const
    {
      if (ancestor_idx == child_idx)
        return true;

      while (child_idx != -1) {
        if (ancestor_idx == child_idx)
          return true;
        child_idx = parent (child_idx);
      }

      return false;
    }

    [[nodiscard]] constexpr long depth (long index) const
    {
      long result = 0;
      while (!is_root (index)) {
        result++;
        index = parent (index);
      }
      return result;
    }

    [[nodiscard]] constexpr long distance (long first, long second) const
    {
      return depth (first) + depth (second) - 2 * depth (find_lca (first, second));
    }

    [[nodiscard]] constexpr auto children (long index) const
    {
      auto result = literal_vector<long, NVertices> ();
      for (long child : _arcs[index])
        result.push_back (child);
      return result;
    }

    [[nodiscard]] constexpr long find_lca (long x, long y) const
    {
      long depth1 = depth (x);
      long depth2 = depth (y);
      while (depth1 > depth2) {
        x = parent (x);
        depth1--;
      }
      while (depth2 > depth1) {
        y = parent (y);
        depth2--;
      }
      while (x != y) {
        x = parent (x);
        y = parent (y);
      }
      return x;
    }

    [[nodiscard]] constexpr std::pair<long, long> find_lca_pred (long x, long y) const
    {
      long lca = find_lca (x, y);
      long pred1 = x;
      long pred2 = y;
      while (pred1 != lca && parent (pred1) != lca)
        pred1 = parent (pred1);
      while (pred2 != lca && parent (pred2) != lca)
        pred2 = parent (pred2);
      return {pred1, pred2};
    }

    [[nodiscard]] constexpr auto path (long x, long y) const
    {
      long const lca = find_lca (x, y);

      auto result = literal_vector<long, NVertices> {};

      while (x != lca) {
        result.push_back (x);
        x = parent (x);
      }

      result.push_back (lca);
      long const mid = result.size ();

      while (y != lca) {
        result.push_back (y);
        y = parent (y);
      }

      std::ranges::reverse (result.begin () + mid, result.end ());
      return result;
    }

    [[nodiscard]] constexpr auto subtree_size_exclusive (long x) const
    {
      long result = 0;
      for (long child : _arcs[x])
        result += subtree_size_exclusive (child) + 1;
      return result;
    }

    [[nodiscard]] constexpr auto subtree_size_inclusive (long x) const
    {
      return subtree_size_exclusive (x) + 1;
    }

    [[nodiscard]] constexpr auto subtree_exclusive (long x) const
    {
      auto result = literal_vector<long, NVertices> {};
      for (long i = _tour._start[x]; i != _tour._end[x]; ++i)
        if (_tour._start[_tour._tour[i]] == i && _tour._tour[i] != x)
          result.push_back (_tour._tour[i]);
      std::ranges::reverse (result);
      return result;
    }

    [[nodiscard]] constexpr auto subtree_inclusive (long x) const
    {
      auto result = subtree_exclusive (x);
      result.push_back (x);
      return result;
    }

    [[nodiscard]] constexpr bool is_cross_arc (long x, long y) const
    {
      long lca = find_lca (x, y);
      return lca != x && lca != y;
    }
  };
} // namespace forest::container

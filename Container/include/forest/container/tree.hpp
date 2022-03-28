#pragma once
#include <concepts>
#include <span>

#include <forest/container/vector.hpp>

namespace forest::container
{
  template<std::semiregular NodeData = std::nullptr_t>
  class tree
  {
  public:
    struct arc_type
    {
      long from;
      long to;

      template<long I, class Self>
        requires (std::same_as<std::remove_cvref_t<Self>, arc_type>&& I >= 0 && I < 2)
      [[nodiscard]] friend constexpr auto& get (Self&& self) noexcept
      {
        if constexpr (I == 0)
          return std::forward<Self> (self).from;
        else
          return std::forward<Self> (self).to;
      }

      bool operator== (arc_type const&) const = default;
      auto operator<=> (arc_type const&) const = default;
    };

    vector<vector<long>> _graph {};
    vector<long> _parents {};
    vector<NodeData> _nodes {};
    vector<arc_type> _arcs;

    // ---

    tree () = default;

    // ---

    [[nodiscard]] constexpr long num_vertices () const noexcept
    {
      return _graph.size ();
    }

    [[nodiscard]] constexpr long num_arcs () const noexcept
    {
      return _arcs.size ();
    }

    // ---

    [[nodiscard]] constexpr std::span<arc_type const> arcs () const noexcept
    {
      return std::span (_arcs.data (), _arcs.size ());
    }

    // ---

    [[nodiscard]] constexpr long parent_of (long i) const noexcept
    {
      return _parents[i];
    }

    [[nodiscard]] constexpr std::span<long const> children_of (size_t i) const noexcept
    {
      return std::span (_graph[i].data (), _graph[i].size ());
    }

    [[nodiscard]] constexpr NodeData& operator[] (long i) noexcept
    {
      return _nodes[i];
    }

    [[nodiscard]] constexpr NodeData const& operator[] (long i) const noexcept
    {
      return _nodes[i];
    }

    // ---

    constexpr long add_node (NodeData data = {}) noexcept
    {
      long id = _graph.size ();
      _graph.emplace_back ();
      _parents.push_back (-1);
      _nodes.push_back (std::move (data));
      return id;
    }

    constexpr void add_arc (long from, long to) noexcept
    {
      _graph[from].push_back (to);
      _parents[to] = from;
      _arcs.push_back (arc_type {from, to});
    }
  };
} // namespace forest::container

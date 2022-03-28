#pragma once
#include <concepts>
#include <forest/container/vector.hpp>

namespace forest::container
{
  template<std::regular Key, std::semiregular Value>
  struct map
  {
    struct node
    {
      Key key;
      Value value;
    };

    using key_type = Key;
    using element_type = Value;
    using value_type = node;
    using size_type = long;
    using difference_type = long;
    using iterator = typename vector<node>::iterator;
    using const_iterator = typename vector<node>::const_iterator;
    using reverse_iterator = typename vector<node>::iterator;
    using const_reverse_iterator = typename vector<node>::const_reverse_iterator;

    vector<node> _nodes;

    // ---------------------------------------------

    map () = default;

    [[nodiscard]] constexpr iterator begin () & noexcept
    {
      return _nodes.begin ();
    }

    [[nodiscard]] constexpr const_iterator begin () const& noexcept
    {
      return _nodes.begin ();
    }

    [[nodiscard]] constexpr iterator end () & noexcept
    {
      return _nodes.end ();
    }

    [[nodiscard]] constexpr const_iterator end () const& noexcept
    {
      return _nodes.end ();
    }

    [[nodiscard]] constexpr size_type size () const noexcept
    {
      return _nodes.size ();
    }

    [[nodiscard]] constexpr bool contains (Key const& key) const noexcept
    {
      return std::ranges::find (_nodes, key, &node::key) != _nodes.end ();
    }

    [[nodiscard]] constexpr element_type& operator[] (key_type const& key) & noexcept
    {
      return std::ranges::find (_nodes, key, &node::key)->value;
    }

    [[nodiscard]] constexpr element_type const& operator[] (key_type const& key) const& noexcept
    {
      return std::ranges::find (_nodes, key, &node::key)->value;
    }

    template<typename... Args>
      requires (std::constructible_from<Value, Args...>)
    constexpr void emplace (Key const& key, Args&&... args) &
    {
      if (contains (key))
        return;

      auto new_key = key;
      auto new_value = Value (std::forward<Args> (args)...);
      auto new_node = node {std::move (new_key), std::move (new_value)};
      _nodes.push_back (std::move (new_node));
    }
  };
} // namespace forest::container

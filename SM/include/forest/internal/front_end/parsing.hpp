#pragma once
#include <forest/internal/container/literal_string.hpp>
#include <forest/internal/container/tree.hpp>
#include <forest/internal/front_end/state_tree.hpp>

namespace forest::internal
{

  // ---

  struct syntax_tree_node
  {
    std::string_view name;
    bool is_region;
  };

  using syntax_tree = tree<syntax_tree_node>;

  // ---

  struct grid_parser
  {
    static constexpr bool is_alpha (char c)
    {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static constexpr bool is_digit (char c)
    {
      return (c >= '0' && c <= '9');
    }

    static constexpr bool is_space (char c)
    {
      return c == ' ' || c == '\t';
    }

    static constexpr std::string_view ltrim (std::string_view text)
    {
      while (!text.empty () && is_space (text.front ()))
        text = text.substr (1);
      return text;
    }

    static constexpr std::string_view rtrim (std::string_view text)
    {
      while (!text.empty () && is_space (text.back ()))
        text = text.substr (0, text.length () - 1);
      return text;
    }

    static constexpr std::string_view trim (std::string_view text)
    {
      text = ltrim (text);
      text = rtrim (text);
      return text;
    }

    static constexpr void split (std::string_view text, char delim, auto callback)
    {
      while (!text.empty ()) {
        auto pos = text.find (delim);
        callback (text.substr (0, pos));
        text.remove_prefix (pos == text.npos ? text.length () : pos + 1);
      }
    };

    static constexpr bool is_state_identifier (std::string_view text)
    {
      text = trim (text);

      if (text.empty ())
        return false;

      auto const valid_character = [] (char c) {
        return is_digit (c) || is_alpha (c);
      };

      if (!std::ranges::all_of (text, valid_character))
        return false;

      if (is_digit (text.front ()))
        return false;

      return true;
    }

    static constexpr bool is_region_identifier (std::string_view text)
    {
      text = trim (text);
      return text.size () > 2   //
        && text.front () == '[' //
        && text.back () == ']'  //
        && is_state_identifier (text.substr (1, text.length () - 2));
    }

    static constexpr bool is_identifier (std::string_view text)
    {
      return is_state_identifier (text) || is_region_identifier (text);
    }

    static constexpr std::string_view get_name (std::string_view identifier)
    {
      identifier = trim (identifier);

      if (is_region_identifier (identifier)) {
        identifier.remove_prefix (1);
        identifier.remove_suffix (1);
      }

      return identifier;
    };

    static constexpr syntax_tree parse (std::string_view text)
    {
      using grid_type = vector<std::string_view>;

      constexpr auto make_grid = [] (std::string_view text) {
        auto grid = grid_type ();
        split (text, '\n', [&grid] (std::string_view line) {
          if (!line.empty ()) {
            grid.push_back (line);
          }
        });
        return grid;
      };

      constexpr auto valid_coord = [] (grid_type const& grid, long row, long col) {
        return row >= 0 && col >= 0 && row < grid.size () && col < grid[row].size ();
      };

      syntax_tree tree;
      grid_type grid = make_grid (text);

      auto recurse = [&tree, &grid, valid_coord] //
        (auto& recurse, long row, long col, std::string_view parent_name, long parent_id) -> void {
        std::string_view curr_identifier = trim (grid[row].substr (col));
        std::string_view curr_name = get_name (curr_identifier);

        long curr_id = tree.add_node ({.name = curr_name, .is_region = is_region_identifier (curr_identifier)});

        if (!parent_name.empty ()) {
          tree.add_arc (parent_id, curr_id);
        }

        // ---

        if (!valid_coord (grid, row + 1, col))
          return;

        col = grid[row + 1].find ('|', col);
        while (valid_coord (grid, row + 1, col) && grid[row + 1][col] == '|') {
          row += 1;
          if (valid_coord (grid, row, col + 1) && grid[row][col + 1] == '-') {
            auto after_dash = grid[row].find_first_not_of ('-', col + 1);
            auto child_identifier = grid[row].substr (after_dash);
            recurse (recurse, row, after_dash, curr_name, curr_id);
          }
        }
      };

      recurse (recurse, 0, 0, {}, -1);
      return tree;
    };
  };

  // ---

  struct named_state_tree_config
  {
    long num_states;
    long num_arcs;
    long string_length;

    constexpr operator state_tree_config () const
    {
      return {.num_states = num_states, .num_arcs = num_arcs};
    }
  };

  template<literal_string Text>
  constexpr inline auto parse_named_state_tree_config = [] {
    syntax_tree tree = grid_parser::parse (Text);
    named_state_tree_config config {};
    config.num_states = tree.num_vertices ();
    config.num_arcs = tree.num_arcs ();

    for (long i = 0; i < config.num_states; ++i)
      config.string_length = std::max (config.string_length, static_cast<long> (tree[i].name.length ()));
    return config;
  }();

  template<named_state_tree_config Conf>
  struct named_state_tree
  {
    using name_type = literal_string<Conf.string_length>;

    state_tree<Conf> tree;
    std::array<name_type, Conf.num_states> names;
  };

  template<literal_string Text>
  constexpr inline auto parse_named_state_tree = [] {
    constexpr auto config = parse_named_state_tree_config<Text>;
    std::array<bool, config.num_states> is_region;

    syntax_tree tree = grid_parser::parse (Text);
    auto result_tree = state_tree<config> (tree.arcs ());
    for (long i = 0; i < config.num_states; ++i)
      result_tree[i].is_region = tree[i].is_region;

    auto result_names = std::array<literal_string<config.string_length>, config.num_states> {};
    for (long i = 0; i < config.num_states; ++i)
      result_names[i] = tree[i].name;

    return named_state_tree<config> {result_tree, result_names};
  }();
} // namespace forest::internal
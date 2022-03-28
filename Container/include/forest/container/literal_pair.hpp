#pragma once
#include <concepts>
#include <utility>

namespace forest::container
{
  template<std::semiregular First, std::semiregular Second>
  class literal_pair
  {
  public:
    using first_type = First;
    using second_type = Second;

    // members

    First first;
    Second second;

    // constructors

    literal_pair () = default;

    constexpr literal_pair (First first, Second second)
      : first (std::move (first))
      , second (std::move (second))
    {}

    // get

    template<long Index>
      requires (Index == 0 || Index == 1)
    [[nodiscard]] friend constexpr auto& get (literal_pair& self)
    {
      if constexpr (Index == 0)
        return self.first;
      else
        return self.second;
    }

    template<long Index>
      requires (Index == 0 || Index == 1)
    [[nodiscard]] friend constexpr auto& get (literal_pair const& self)
    {
      if constexpr (Index == 0)
        return self.first;
      else
        return self.second;
    }
  };

  template<class First, class Second>
  literal_pair (First, Second) -> literal_pair<First, Second>;
} // namespace forest::container

namespace std
{
  template<class First, class Second>
  struct tuple_size<forest::container::literal_pair<First, Second>> : std::integral_constant<std::size_t, 2>
  {};

  template<std::size_t Index, class First, class Second>
  struct tuple_element<Index, forest::container::literal_pair<First, Second>>
    : std::tuple_element<Index, std::pair<First, Second>>
  {};
} // namespace std
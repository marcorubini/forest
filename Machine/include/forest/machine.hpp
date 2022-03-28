#pragma once
#include <forest/machine/backend/machine.hpp>
#include <forest/machine/frontend/binding.hpp>
#include <forest/machine/frontend/parsing.hpp>

namespace forest::machine
{
  template<forest::container::literal_string Text,
    class GlobalStorage = std::monostate,
    template<class, class> class CustomContext = default_context,
    template<class, class> class CustomState = default_state>
  using describe_machine =
    internal::make_machine_helper<internal::parse_tree<Text>, GlobalStorage, CustomContext, CustomState>;
} // namespace forest::machine
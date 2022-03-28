#pragma once
#include <forest/internal/back_end/back_end.hpp>

namespace forest::sm
{
  template<forest::internal::literal_string Text, std::move_constructible GlobalStorage = std::monostate>
  using make_machine = forest::internal::make_machine<Text, GlobalStorage>;
}
#pragma once
#include <string>

namespace forest::events
{
  struct button_pressed
  {
    std::string id;

    button_pressed () = default;

    button_pressed (std::string id)
      : id (id)
    {}
  };
} // namespace forest::events

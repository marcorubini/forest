#pragma once
#include <concepts>
#include <string>

namespace forest
{
  namespace events
  {
    struct message
    {
      std::int64_t id;
      std::string text;
    };
  } // namespace events
} // namespace forest
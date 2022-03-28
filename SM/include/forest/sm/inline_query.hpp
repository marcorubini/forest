#pragma once
#include <string>

namespace forest
{
  namespace events
  {
    struct inline_query
    {
      std::string query_id;
      std::string text;
      std::string offset;
    };
  } // namespace events
} // namespace forest
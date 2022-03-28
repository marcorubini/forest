#pragma once

namespace forest::internal
{
  template<class Discard, class T>
  struct dependent_type
  {
    using type = T;
  };

  template<class Discard, class T>
  using dependent_t = typename dependent_type<Discard, T>::type;
} // namespace forest::internal
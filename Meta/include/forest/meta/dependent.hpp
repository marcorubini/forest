#pragma once

namespace forest::internal::meta
{
  template<class T, class... Discarded>
  struct dependent_type
  {
    using type = T;
  };
} // namespace forest::internal::meta

namespace forest::meta
{
  template<class T, class... Discarded>
  using dependent_t = typename internal::meta::dependent_type<T, Discarded...>::type;
} // namespace forest::meta
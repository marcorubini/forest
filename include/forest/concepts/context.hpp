#pragma once
#include <concepts>
#include <string>

namespace forest
{
  template<class T>
  concept Context = requires (T context)
  {
    // clang-format off
    requires std::copy_constructible<T>;
    typename T::cache_type;
    typename T::cache_reference;

    requires requires (typename T::cache_type cache, std::string message)
    {
      { context.get_cache() } -> std::same_as<typename T::cache_reference>;
      { context.set_cache(std::move(cache)) };
      { context.send_message(message) };
    };
    // clang-format on
  };
} // namespace forest

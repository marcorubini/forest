#pragma once
#include <forest/telegram/driver.hpp>

namespace forest::telegram
{
  template<Driver DriverType>
  struct bot_storage
  {
    using driver_type = DriverType;
    using driver_reference = driver_type&;

    driver_reference _driver;
    api_chat_id _chat_id;

    // ---

    bot_storage (driver_reference driver, api_chat_id chat_id)
      : _driver (driver)
      , _chat_id (chat_id)
    {}

    [[nodiscard]] constexpr auto driver () const -> driver_reference
    {
      return _driver;
    }

    [[nodiscard]] constexpr auto chat_id () const -> api_chat_id
    {
      return _chat_id;
    }
  };
} // namespace forest::telegram
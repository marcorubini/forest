#pragma once
#include <concepts>
#include <memory>
#include <utility>

namespace forest::internal
{
  template<std::move_constructible T>
  class semiregular_box
  {
  private:
    struct empty_type;
    union storage_type;

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using storage = storage_type;

    // ---

    semiregular_box () = default;

    explicit constexpr semiregular_box (value_type init)          //
      noexcept (std::is_nothrow_move_constructible_v<value_type>) //
      : _storage (std::move (init))
    {}

    // ---

    constexpr semiregular_box (semiregular_box const& other)      //
      noexcept (std::is_nothrow_copy_constructible_v<value_type>) //
      requires (std::copy_constructible<value_type>)              //
      : _storage (other.unwrap ())
    {}

    constexpr semiregular_box (semiregular_box&& other)           //
      noexcept (std::is_nothrow_move_constructible_v<value_type>) //
      : _storage (std::move (other).unwrap ())
    {}

    // ---

    semiregular_box& operator= (semiregular_box const& other) & = delete;
    semiregular_box& operator= (semiregular_box&& other) & = delete;

    // ---

    [[nodiscard]] constexpr reference unwrap () & noexcept
    {
      return _storage.value;
    }

    [[nodiscard]] constexpr const_reference unwrap () const& noexcept
    {
      return _storage.value;
    }

    [[nodiscard]] constexpr value_type unwrap () && noexcept (std::is_nothrow_move_constructible_v<value_type>)
    {
      return std::move (_storage).value;
    }

    [[nodiscard]] constexpr reference operator* () & noexcept
    {
      return _storage.value;
    }

    [[nodiscard]] constexpr const_reference operator* () const& noexcept
    {
      return _storage.value;
    }

    [[nodiscard]] constexpr value_type operator* () && noexcept (std::is_nothrow_move_constructible_v<value_type>)
    {
      return std::move (_storage).value;
    }

    // ---

    void emplace (value_type init) &                              //
      noexcept (std::is_nothrow_move_constructible_v<value_type>) //
    {
      std::ranges::destroy_at (std::addressof (_storage.empty));
      std::ranges::construct_at (std::addressof (_storage.value), std::move (init));
    }

    void assign (value_type init) &                               //
      noexcept (std::is_nothrow_move_constructible_v<value_type>) //
    {
      if constexpr (std::movable<value_type>) {
        unwrap () = std::move (init);
      } else {
        reset ();
        emplace (std::move (init));
      }
    }

    // ---

    void reset () &   //
      noexcept (true) //
    {
      std::ranges::destroy_at (std::addressof (_storage.value));
      std::ranges::construct_at (std::addressof (_storage.empty));
    }

    // ---

  private:
    storage _storage;
  };

  template<class T>
  semiregular_box (T) -> semiregular_box<T>;
} // namespace forest::internal
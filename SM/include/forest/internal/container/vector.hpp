#pragma once
#include <algorithm>
#include <memory>

namespace forest::internal
{
  template<std::move_constructible T>
  struct vector_iterator;

  template<std::move_constructible T>
  struct vector_const_iterator;

  template<std::move_constructible T>
  class vector_iterator
  {
  public:
    using size_type = long;
    using difference_type = long;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    pointer _pos {};

    vector_iterator () = default;

    constexpr vector_iterator (pointer _pos) noexcept
      : _pos (_pos)
    {}

    // ---

    constexpr operator vector_const_iterator<T> () const noexcept
    {
      return vector_const_iterator<T> {_pos};
    }

    // ---

    constexpr vector_iterator& operator++ () noexcept
    {
      ++_pos;
      return *this;
    }

    constexpr vector_iterator operator++ (int) noexcept
    {
      auto result = *this;
      ++*this;
      return result;
    }

    constexpr vector_iterator& operator-- () noexcept
    {
      --_pos;
      return *this;
    }

    constexpr vector_iterator operator-- (int) noexcept
    {
      auto result = *this;
      --*this;
      return result;
    }

    // ---

    [[nodiscard]] constexpr difference_type operator- (vector_iterator rhs) const noexcept
    {
      return _pos - rhs._pos;
    }

    [[nodiscard]] friend constexpr vector_iterator operator+ (vector_iterator lhs, difference_type rhs) noexcept
    {
      return {lhs._pos + rhs};
    }

    [[nodiscard]] friend constexpr vector_iterator operator+ (difference_type lhs, vector_iterator rhs) noexcept
    {
      return {lhs + rhs._pos};
    }

    [[nodiscard]] friend constexpr vector_iterator operator- (vector_iterator lhs, difference_type rhs) noexcept
    {
      return {lhs._pos - rhs};
    }

    [[nodiscard]] friend constexpr vector_iterator operator- (difference_type lhs, vector_iterator rhs) noexcept
    {
      return {lhs - rhs._pos};
    }

    // ---

    constexpr vector_iterator& operator+= (difference_type rhs) noexcept
    {
      _pos += rhs;
      return *this;
    }

    constexpr vector_iterator& operator-= (difference_type rhs) noexcept
    {
      _pos -= rhs;
      return *this;
    }

    // ---

    [[nodiscard]] constexpr reference operator* () const noexcept
    {
      return *_pos;
    }

    [[nodiscard]] constexpr reference operator[] (difference_type rhs) const noexcept
    {
      return _pos[rhs];
    }

    [[nodiscard]] constexpr pointer operator-> () const noexcept
    {
      return _pos;
    }

    // ---

    [[nodiscard]] bool operator== (vector_iterator const&) const = default;
    [[nodiscard]] auto operator<=> (vector_iterator const&) const = default;
  };

  template<std::move_constructible T>
  class vector_const_iterator
  {
  public:
    using size_type = long;
    using difference_type = long;
    using value_type = T;
    using pointer = T const*;
    using reference = T const&;

    pointer _pos {};

    vector_const_iterator () = default;

    constexpr vector_const_iterator (pointer _pos) noexcept
      : _pos (_pos)
    {}

    // ---

    constexpr vector_const_iterator& operator++ () noexcept
    {
      ++_pos;
      return *this;
    }

    constexpr vector_const_iterator operator++ (int) noexcept
    {
      auto result = *this;
      ++*this;
      return result;
    }

    constexpr vector_const_iterator& operator-- () noexcept
    {
      --_pos;
      return *this;
    }

    constexpr vector_const_iterator operator-- (int) noexcept
    {
      auto result = *this;
      --*this;
      return result;
    }

    // ---

    [[nodiscard]] constexpr difference_type operator- (vector_const_iterator rhs) const noexcept
    {
      return _pos - rhs._pos;
    }

    [[nodiscard]] friend constexpr vector_const_iterator operator+ (vector_const_iterator lhs, difference_type rhs) noexcept
    {
      return {lhs._pos + rhs};
    }

    [[nodiscard]] friend constexpr vector_const_iterator operator+ (difference_type lhs, vector_const_iterator rhs) noexcept
    {
      return {lhs + rhs._pos};
    }

    [[nodiscard]] friend constexpr vector_const_iterator operator- (vector_const_iterator lhs, difference_type rhs) noexcept
    {
      return {lhs._pos - rhs};
    }

    [[nodiscard]] friend constexpr vector_const_iterator operator- (difference_type lhs, vector_const_iterator rhs) noexcept
    {
      return {lhs - rhs._pos};
    }

    // ---

    constexpr vector_const_iterator& operator+= (difference_type rhs) noexcept
    {
      _pos += rhs;
      return *this;
    }

    constexpr vector_const_iterator& operator-= (difference_type rhs) noexcept
    {
      _pos -= rhs;
      return *this;
    }

    // ---

    [[nodiscard]] constexpr reference operator* () const noexcept
    {
      return *_pos;
    }

    [[nodiscard]] constexpr reference operator[] (difference_type rhs) const noexcept
    {
      return _pos[rhs];
    }

    [[nodiscard]] constexpr pointer operator-> () const noexcept
    {
      return _pos;
    }

    // ---

    [[nodiscard]] bool operator== (vector_const_iterator const&) const = default;
    [[nodiscard]] auto operator<=> (vector_const_iterator const&) const = default;
  };

  // dynamic vector used in consteval functions

  template<std::semiregular T>
  struct vector
  {
    using iterator = vector_iterator<T>;
    using const_iterator = vector_const_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::make_signed_t<std::size_t>;
    using difference_type = std::make_signed_t<std::size_t>;
    using value_type = T;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;

    value_type* _data {};
    size_type _size {};
    size_type _capacity {};

    vector () = default;

    constexpr vector (size_type n) noexcept
      : _data (new T[n] {})
      , _size (n)
      , _capacity (n)
    {}

    constexpr vector (size_type n, value_type const& value) noexcept
      : vector (n)
    {
      for (size_type i = 0; i < n; ++i)
        _data[i] = value;
    }

    constexpr reference push_back (value_type value) & noexcept //
    {
      if (_capacity == _size)
        grow ();

      _data[_size++] = std::move (value);
      return _data[_size - 1];
    }

    template<typename... Args>
      requires (std::constructible_from<value_type, Args...>) //
    constexpr reference emplace_back (Args&&... args) & noexcept
    {
      if (_capacity == _size)
        grow ();

      _data[_size++] = T (std::forward<Args> (args)...);
      return _data[_size - 1];
    }

    constexpr void resize (size_type n) & noexcept
    {
      if (n < _size) {
        _size = n;
      } else if (n > _size) {
        while (_capacity < n)
          grow ();

        _size = n;
      }
    }

    constexpr void resize (size_type n, value_type const& value) & noexcept
    {
      size_type old_size = _size;
      resize (n);

      for (size_type i = old_size; i < n; ++i)
        _data[i] = value;
    }

    [[nodiscard]] constexpr reference operator[] (size_type i) & noexcept
    {
      return _data[i];
    }

    [[nodiscard]] constexpr const_reference operator[] (size_type i) const& noexcept
    {
      return _data[i];
    }

    [[nodiscard]] constexpr reference front () & noexcept
    {
      return _data[0];
    }

    [[nodiscard]] constexpr const_reference front () const& noexcept
    {
      return _data[0];
    }

    [[nodiscard]] constexpr reference back () & noexcept
    {
      return _data[_size - 1];
    }

    [[nodiscard]] constexpr const_reference back () const& noexcept
    {
      return _data[_size - 1];
    }

    constexpr void pop_back () & noexcept
    {
      _size -= 1;
    }

    [[nodiscard]] constexpr iterator begin () & noexcept
    {
      return iterator {_data};
    }

    [[nodiscard]] constexpr const_iterator begin () const& noexcept
    {
      return const_iterator {_data};
    }

    [[nodiscard]] constexpr iterator end () & noexcept
    {
      return iterator {_data + _size};
    }

    [[nodiscard]] constexpr const_iterator end () const& noexcept
    {
      return const_iterator {_data + _size};
    }

    [[nodiscard]] constexpr reverse_iterator rbegin () & noexcept
    {
      return reverse_iterator {end ()};
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin () const& noexcept
    {
      return const_reverse_iterator {end ()};
    }

    [[nodiscard]] constexpr reverse_iterator rend () & noexcept
    {
      return reverse_iterator {begin ()};
    }

    [[nodiscard]] constexpr const_reverse_iterator rend () const& noexcept
    {
      return const_reverse_iterator {begin ()};
    }

    [[nodiscard]] constexpr pointer data () & noexcept
    {
      return _data;
    }

    [[nodiscard]] constexpr const_pointer data () const& noexcept
    {
      return _data;
    }

    [[nodiscard]] constexpr size_type size () const noexcept
    {
      return _size;
    }

    [[nodiscard]] constexpr bool empty () const noexcept
    {
      return _size == 0;
    }

    constexpr vector (vector&& rhs) noexcept
    {
      _data = rhs._data;
      _capacity = rhs._capacity;
      _size = rhs._size;
      rhs._data = nullptr;
      rhs._size = 0;
      rhs._capacity = 0;
    }

    constexpr vector (vector const& rhs) noexcept
    {
      _data = new value_type[rhs._size];

      for (std::size_t i = 0; i < rhs._size; ++i)
        _data[i] = rhs._data[i];

      _size = rhs._size;
      _capacity = _size;
    }

    constexpr vector& operator= (vector&& rhs) noexcept
    {
      std::ranges::destroy_at (this);
      std::ranges::construct_at (this, std::move (rhs));
      return *this;
    }

    constexpr vector& operator= (vector const& rhs) noexcept
    {
      std::ranges::destroy_at (this);
      std::ranges::construct_at (this, rhs);
      return *this;
    }

    constexpr ~vector () noexcept
    {
      delete[] _data;
    }

  private:
    constexpr void grow () & noexcept
    {
      auto new_capacity = _capacity ? _capacity * 2 : 1;
      auto new_data = new T[new_capacity];
      for (size_type i = 0; i < _capacity; ++i)
        new_data[i] = std::move (_data[i]);

      delete[] _data;
      _data = new_data;
      _capacity = new_capacity;
    }
  };

  template<class F>
  static constexpr auto to_array (F) noexcept
  {
    using value_type = typename decltype (F {}())::value_type;
    using size_type = typename decltype (F {}())::size_type;

    constexpr size_type size = F {}().size ();
    auto result = std::array<value_type, size> ();
    auto v = F {}();
    for (size_type i = 0; i < size; ++i)
      result[i] = v[i];
    return result;
  }
} // namespace forest::internal
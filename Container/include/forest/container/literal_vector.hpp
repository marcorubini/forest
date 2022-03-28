#pragma once
#include <algorithm>
#include <array>
#include <concepts>

namespace forest::container
{
  template<std::semiregular T, long MaxN>
  class literal_vector
  {
  public:
    static constexpr long max_capacity = MaxN;

    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;

    using size_type = long;
    using difference_type = long;

    std::array<T, MaxN> _array {};
    long _size {};

    literal_vector () = default;

    constexpr literal_vector (long n)
      : _size (n)
    {}

    // observers

    [[nodiscard]] constexpr size_type size () const
    {
      return _size;
    }

    // access

    [[nodiscard]] constexpr reference operator[] (size_type i)
    {
      return _array[i];
    }

    [[nodiscard]] constexpr const_reference operator[] (size_type i) const
    {
      return _array[i];
    }

    template<long I>
      requires (I >= 0 && I < max_capacity)
    friend constexpr reference get (literal_vector& self)
    {
      return self[I];
    }

    template<long I>
      requires (I >= 0 && I < max_capacity)
    friend constexpr const_reference get (literal_vector const& self)
    {
      return self[I];
    }

    // range

    [[nodiscard]] constexpr iterator begin ()
    {
      return _array.data ();
    }

    [[nodiscard]] constexpr const_iterator begin () const
    {
      return _array.data ();
    }

    [[nodiscard]] constexpr iterator end ()
    {
      return _array.data () + size ();
    }

    [[nodiscard]] constexpr const_iterator end () const
    {
      return _array.data () + size ();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin ()
    {
      return end ();
    }

    [[nodiscard]] constexpr reverse_const_iterator rbegin () const
    {
      return end ();
    }

    [[nodiscard]] constexpr reverse_iterator rend ()
    {
      return begin ();
    }

    [[nodiscard]] constexpr reverse_const_iterator rend () const
    {
      return begin ();
    }

    [[nodiscard]] constexpr pointer data ()
    {
      return _array.data ();
    }

    [[nodiscard]] constexpr const_pointer data () const
    {
      return _array.data ();
    }

    // modify

    constexpr reference push_back (value_type init)
    {
      _array[_size++] = init;
      return _array[_size - 1];
    }

    constexpr void pop_back ()
    {
      _size -= 1;
    }

    constexpr iterator erase (iterator first, iterator last)
    {
      std::ranges::copy (last, end (), first);
      _size -= std::ranges::distance (first, last);
      return first;
    }

    constexpr void clear ()
    {
      _size = 0;
    }
  };
} // namespace forest::container
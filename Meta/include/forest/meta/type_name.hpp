#pragma once
#include <string_view>

namespace forest::meta
{
  template<typename T>
  struct type_name_t
  {
    constexpr static std::string_view fullname_intern ()
    {
#if defined(__clang__) || defined(__GNUC__)
      return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
      return __FUNCSIG__;
#else
#  error "Unsupported compiler"
#endif
    }
    constexpr static std::string_view name ()
    {
      size_t prefix_len = type_name_t<void>::fullname_intern ().find ("void");
      size_t multiple = type_name_t<void>::fullname_intern ().size () - type_name_t<int>::fullname_intern ().size ();
      size_t dummy_len = type_name_t<void>::fullname_intern ().size () - 4 * multiple;
      size_t target_len = (fullname_intern ().size () - dummy_len) / multiple;
      std::string_view rv = fullname_intern ().substr (prefix_len, target_len);
      if (rv.rfind (' ') == rv.npos)
        return rv;
      return rv.substr (rv.rfind (' ') + 1);
    }

    using type = T;
    constexpr static std::string_view value = name ();
  };

  template<class T>
  constexpr std::string_view type_name ()
  {
    return type_name_t<T>::name ();
  }
} // namespace forest::meta
/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id tstring.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/12/31 21:21
 * @uses The tstring for windows(c++20).
 *
 */
#ifndef PF_BASIC_TSTRING_H_
#define PF_BASIC_TSTRING_H_

#include "pf/basic/config.h"

#if OS_WIN_UNUSED

#include <Windows.h>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace pf_basic {

template<class charT>
class basic_tstring {
public:
  using traits_type = typename std::basic_string<charT>::traits_type;
  using value_type = typename std::basic_string<charT>::value_type;
  using allocator_type = typename std::basic_string<charT>::allocator_type;
  using size_type = typename std::basic_string<charT>::size_type;
  using difference_type = typename std::basic_string<charT>::difference_type;
  using reference = typename std::basic_string<charT>::reference;
  using const_reference = typename std::basic_string<charT>::const_reference;
  using pointer = typename std::basic_string<charT>::pointer;
  using const_pointer = typename std::basic_string<charT>::const_pointer;
  using iterator = typename std::basic_string<charT>::iterator;
  using const_iterator = typename std::basic_string<charT>::const_iterator;
  using reverse_iterator = typename std::basic_string<charT>::reverse_iterator;
  using const_reverse_iterator = typename std::basic_string<charT>::const_reverse_iterator;

private:
  template<class T>
  using is_string = std::disjunction<
    std::is_convertible<T, std::basic_string<char>>,
    std::is_convertible<T, std::basic_string<char8_t>>,
    std::is_convertible<T, std::basic_string<char16_t>>,
    std::is_convertible<T, std::basic_string<char32_t>>,
    std::is_convertible<T, std::basic_string<wchar_t>>>;
  template<class T>
  static constexpr bool is_string_v = is_string<T>::value;

  template<class T>
  using is_basic_string = std::conjunction<
    is_string<T>,
    std::negation<std::is_integral<std::remove_pointer_t<std::decay_t<T>>>>>;
  template<class T>
  static constexpr bool is_basic_string_v = is_basic_string<T>::value;

  template<class T>
  class character_of {
  private:
    template<class U, std::enable_if_t<is_basic_string_v<U>, std::nullptr_t> = nullptr>
    static typename std::decay_t<U>::value_type f(U&& s);

    template<class U, std::enable_if_t<std::negation_v<is_basic_string<U>>, std::nullptr_t> = nullptr>
    static std::remove_cvref_t<std::remove_pointer_t<std::decay_t<U>>> f(U&& s);

  public:
    using type = decltype(f(std::declval<T>()));
  };
  template<class T>
  using character_of_t = typename character_of<T>::type;

public:
  template<class dstT, class srcT>
  static std::basic_string<dstT> convert(const std::basic_string<srcT>& src)
  {
    return std::basic_string<dstT>();
  }

  template<>
  static std::string convert(const std::wstring& src)
  {
    auto ret = ::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (!ret) {
      throw std::system_error(GetLastError(), std::generic_category(), "failed to get the buffer size that is needed to store string");
    }

    std::string dst(ret, '\0');
    ret = ::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, dst.data(), dst.size(), nullptr, nullptr);
    if (!ret) {
      throw std::system_error(GetLastError(), std::generic_category(), "failed to get the buffer size that is needed to store string");
    }
    return dst;
  }

  template<>
  static std::wstring convert(const std::string& src)
  {
    auto ret = ::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, nullptr, 0);
    if (!ret) {
      throw std::system_error(GetLastError(), std::generic_category(), "failed to get the buffer size that is needed to store wstring");
    }

    std::wstring dst(ret, L'\0');
    ret = ::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, dst.data(), dst.size());
    if (!ret) {
      throw std::system_error(GetLastError(), std::generic_category(), "failed to translate from string to wstring");
    }
    return dst;
  }

public:
  static constexpr const basic_tstring::size_type npos = std::basic_string<charT>::npos;

public:
  basic_tstring() = default;

  basic_tstring(const basic_tstring& other) = default;

  basic_tstring(basic_tstring&& other) = default;

  basic_tstring(std::initializer_list<charT> character_list)
    : m_string()
    , m_u8string()
    , m_u16string()
    , m_u32string()
    , m_wstring()
  {
    get_cache<charT>().emplace(character_list);
  }

  template <class InputIterator> basic_tstring(InputIterator first, InputIterator last)
    : m_string()
    , m_u8string()
    , m_u16string()
    , m_u32string()
    , m_wstring()
  {
    get_cache<charT>().emplace(std::basic_string<charT>(std::forward<InputIterator>(first), std::forward<InputIterator>(last)));
  }

  template<class T, std::enable_if_t <is_string_v<T>, std::nullptr_t> = nullptr, typename... Args>
  basic_tstring(T arg1, Args&&... args)
    : m_string()
    , m_u8string()
    , m_u16string()
    , m_u32string()
    , m_wstring()
  {
    get_cache<character_of_t<T>>().emplace(std::forward<T>(arg1), std::forward<Args>(args)...);
  }

  template<class T, std::enable_if_t<std::negation_v<is_string<T>>, std::nullptr_t> = nullptr, typename... Args>
  basic_tstring(T arg1, Args&&... args)
    : m_string()
    , m_u8string()
    , m_u16string()
    , m_u32string()
    , m_wstring()
  {
    get_cache<charT>().emplace(std::forward<T>(arg1), std::forward<Args>(args)...);
  }

public:
  basic_tstring& operator=(const basic_tstring& other)
  {
    if (this != &other) {
      m_string = other.m_string;
      m_u8string = other.m_u8string;
      m_u16string = other.m_u16string;
      m_u32string = other.m_u32string;
      m_wstring = other.m_wstring;
    }
    return *this;
  }

  basic_tstring& operator=(basic_tstring&& other)
  {
    if (this != &other) {
      m_string = other.m_string;
      m_u8string = other.m_u8string;
      m_u16string = other.m_u16string;
      m_u32string = other.m_u32string;
      m_wstring = other.m_wstring;
    }
    return *this;
  }

  operator const std::string& () const
  {
    return string();
  }

  operator const std::u8string& () const
  {
    return u8string();
  }

  operator const std::u16string& () const
  {
    return u16string();
  }

  operator const std::u32string& () const
  {
    return u32string();
  }

  operator const std::wstring& () const
  {
    return wstring();
  }

  operator std::string () const
  {
    return string();
  }

  operator std::u8string () const
  {
    return u8string();
  }

  operator std::u16string () const
  {
    return u16string();
  }

  operator std::u32string () const
  {
    return u32string();
  }

  operator std::wstring () const
  {
    return wstring();
  }
  
public:
#define delegate_to_base(method_name)                                                                \
  template<typename... Args> auto method_name(Args... args) const -> decltype(std::declval<std::basic_string<charT>>().method_name(std::forward<Args>(args)...))  \
  {                                                                                \
    return static_cast<const std::basic_string<charT>&>(get_string<charT>()).method_name(std::forward<Args>(args)...);                      \
  }                                                                                \
  template<typename... Args> auto method_name(Args... args) -> decltype(std::declval<std::basic_string<charT>>().method_name(std::forward<Args>(args)...))    \
  {                                                                                \
    clear_caches();                                                                        \
    return get_string<charT>().method_name(std::forward<Args>(args)...);                                            \
  }
  delegate_to_base(append)
  delegate_to_base(assign)
  delegate_to_base(at)
  delegate_to_base(back)
  delegate_to_base(begin)
  delegate_to_base(c_str)
  delegate_to_base(capacity)
  delegate_to_base(cbegin)
  delegate_to_base(cend)
  delegate_to_base(clear)
  delegate_to_base(compare)
  delegate_to_base(copy)
  delegate_to_base(crbegin)
  delegate_to_base(crend)
  delegate_to_base(data)
  delegate_to_base(empty)
  delegate_to_base(end)
  delegate_to_base(ends_with)
  delegate_to_base(erase)
  delegate_to_base(find)
  delegate_to_base(find_first_not_of)
  delegate_to_base(find_first_of)
  delegate_to_base(find_last_nof_of)
  delegate_to_base(find_last_of)
  delegate_to_base(front)
  delegate_to_base(get_allocator)
  delegate_to_base(insert)
  delegate_to_base(length)
  delegate_to_base(max_size)
//  delegate_to_base(operator basic_string_view)
  delegate_to_base(operator+=)
  delegate_to_base(operator[])
  delegate_to_base(pop_back)
  delegate_to_base(push_back)
  delegate_to_base(rbegin)
  delegate_to_base(rend)
  delegate_to_base(replace)
  delegate_to_base(reserve)
  delegate_to_base(resize)
  delegate_to_base(rfind)
  delegate_to_base(shrink_to_fit)
  delegate_to_base(size)
  delegate_to_base(starts_with)
  delegate_to_base(substr)
  delegate_to_base(swap)
#undef delegate_to_base

  const std::string& string() const
  {
    return get_string<char>();
  }

  const std::u8string& u8string() const
  {
    return get_string<char8_t>();
  }

  const std::u16string& u16string() const
  {
    return get_string<char16_t>();
  }

  const std::u32string& u32string() const
  {
    return get_string<char32_t>();
  }

  const std::wstring& wstring() const
  {
    return get_string<wchar_t>();
  }

private:
  template<class T>
  std::optional<std::basic_string<T>>& get_cache() const
  {
    auto& cache = *[&] {
      if constexpr (std::is_same_v<T, char>) {
        return &m_string;
      }
      if constexpr (std::is_same_v<T, char8_t>) {
        return &m_u8string;
      }
      if constexpr (std::is_same_v<T, char16_t>) {
        return &m_u16string;
      }
      if constexpr (std::is_same_v<T, char32_t>) {
        return &m_u32string;
      }
      if constexpr (std::is_same_v<T, wchar_t>) {
        return &m_wstring;
      }
    }();
    return cache;
  }

  template<class T>
  std::basic_string<T>& get_string() const
  {
    std::lock_guard lock(m_mutex);
    return const_cast<basic_tstring*>(this)->get_string<T>();
  }

  template<class T>
  std::basic_string<T>& get_string()
  {
    auto&& cache = get_cache<T>();
    if (!cache) {
      if (m_wstring) {
        cache.emplace(convert<T>(*m_wstring));
      }
      else if (m_string) {
        cache.emplace(convert<T>(*m_string));
      }
      else if (m_u8string) {
        cache.emplace(convert<T>(*m_u8string));
      }
      else if (m_u16string) {
        cache.emplace(convert<T>(*m_u16string));
      }
      else if (m_u32string) {
        cache.emplace(convert<T>(*m_u32string));
      }
      else {
        cache.emplace();
      }
    }
    return *cache;
  }

  void clear_caches()
  {
    // copy a string to a cache of charT type.
    get_string<charT>();

    // clear caches other than charT type.
    if constexpr (std::negation_v<std::is_same<charT, char>>) {
      m_string.reset();
    }
    if constexpr (std::negation_v<std::is_same<charT, char8_t>>) {
      m_u8string.reset();
    }
    if constexpr (std::negation_v<std::is_same<charT, char16_t>>) {
      m_u16string.reset();
    }
    if constexpr (std::negation_v<std::is_same<charT, char32_t>>) {
      m_u32string.reset();
    }
    if constexpr (std::negation_v<std::is_same<charT, wchar_t>>) {
      m_wstring.reset();
    }
  }

private:
  mutable std::optional<std::string> m_string;
  mutable std::optional<std::u8string> m_u8string;
  mutable std::optional<std::u16string> m_u16string;
  mutable std::optional<std::u32string> m_u32string;
  mutable std::optional<std::wstring> m_wstring;
  mutable std::mutex m_mutex;
};

template<class charT, class T>
std::basic_ostream<charT>& operator<<(std::basic_ostream<charT>& os, basic_tstring<T>& str)
{
  return os << static_cast<std::basic_string<charT>>(str);
}

#ifdef _UNICODE
using tstring = basic_tstring<wchar_t>;
#else // _UNICODE
using tstring = basic_tstring<char>;
#endif // _UNICODE

} // namespace pf_basic

#endif // PF_BASIC_TSTRING_H_

#endif // OS_WIN

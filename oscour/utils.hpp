#pragma once
#include <gsl/span>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace oscour
{
template <typename T>
concept bool Arithmetic =
    std::is_arithmetic_v<T>;

template <typename T>
concept bool HasValue = requires {
  T::value;
};
template<typename T>
concept bool NotAPointer =
    !std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename T>
concept bool Any = requires {
    int{};
};

template <typename T>
concept bool MessageArgument = requires {
    NotAPointer<T>;
};


template <typename... Args>
struct dummy
{
};
using span = gsl::span<const char>;

static const constexpr bool is_little_endian
    = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);

template <typename T>
constexpr void to_net(char* p, T x) noexcept
{
  if constexpr(is_little_endian)
  {
    union {
      T i;
      char c[sizeof(T)];
    } u{};

    u.i = x;

    for (int i = 0; i < sizeof(T); i++)
    {
      p[sizeof(T) - i - 1] = u.c[i];
    }
  }
  else
  {
    *reinterpret_cast<T*>(p) = x;
  }
}

template <typename T>
constexpr T from_net(const char* p) noexcept
{
  if constexpr(is_little_endian)
  {
    union {
      T i;
      char c[sizeof(T)];
    } u{};

    for (int i = 0; i < sizeof(T); i++)
    {
      u.c[i] = p[sizeof(T) - i - 1];
    }

    return u.i;
  }
  else
  {
    return *(T*)p;
  }
}

// round up to the next highest multiple of 4. unless x is already a multiple
// of 4
constexpr uint32_t round_up_4(uint32_t x) noexcept
{
  return (x + 3) & ~((uint32_t)0x03);
}

constexpr bool is_multiple_of_4(std::size_t x) noexcept
{
  return (x & ((std::size_t)0x03)) == 0;
}

// return the first 4 byte boundary after the end of a str4
// be careful about calling this version if you don't know whether
// the string is terminated correctly.
constexpr const char* find_str4_end(const char* p) noexcept
{
  if (p[0] == '\0') // special case for SuperCollider integer address pattern
    return p + 4;

  p += 3;

  while (*p)
    p += 4;

  return p + 1;
}

// return the first 4 byte boundary after the end of a str4
// returns 0 if p == end or if the string is unterminated
constexpr const char* find_str4_end(const char* p, const char* end) noexcept
{
  if (p >= end)
    return 0;

  if (p[0] == '\0') // special case for SuperCollider integer address pattern
    return p + 4;

  p += 3;
  end -= 1;

  while (p < end && *p)
    p += 4;

  if (*p)
    return 0;
  else
    return p + 1;
}

constexpr bool is_valid_element_size_value(std::size_t x) noexcept
{
  // Element sizes are specified to be int32_t, and are always rounded up to
  // nearest
  // multiple of 4. Therefore their values can't be greater than 0x7FFFFFFC.
  const constexpr auto bundle_max_size = 0x7FFFFFFC;

  return x >= 0 && x <= bundle_max_size;
}


constexpr bool is_bundle(span dat) noexcept
{
  return (!dat.empty() && dat[0] == '#');
}

constexpr bool is_message(span dat) noexcept
{
  return !is_bundle(dat);
}
}

#pragma once
#include <gsl/span>
#include <stdexcept>
#include <string_view>

namespace oscour
{
template<typename... Args> struct dummy {};
using span = gsl::span<const char>;

static const constexpr bool is_little_endian = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
template<typename T>
inline void from_T(char* p, T x)
{
  if constexpr(is_little_endian)
  {
    union {
      T i;
      char c[sizeof(T)];
    } u;

    u.i = x;

    for(int i = 0; i < sizeof(T); i++)
    {
      p[sizeof(T) - i - 1] = u.c[i];
    }
  }
  else
  {
    *reinterpret_cast<T*>(p) = x;
  }
}

template<typename T>
T to_T(const char* p)
{
  if constexpr(is_little_endian)
  {
    union {
      T i;
      char c[sizeof(T)];
    } u;

    for(int i = 0; i < sizeof(T); i++)
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

enum
{
  OSC_INT32_MAX = 0x7FFFFFFF,

  // Element sizes are specified to be int32_t, and are always rounded up to
  // nearest
  // multiple of 4. Therefore their values can't be greater than 0x7FFFFFFC.
  OSC_BUNDLE_ELEMENT_SIZE_MAX = 0x7FFFFFFC
};

enum ValueTypeSizes
{
  OSC_SIZEOF_INT32 = 4,
  OSC_SIZEOF_UINT32 = 4,
  OSC_SIZEOF_INT64 = 8,
  OSC_SIZEOF_UINT64 = 8
};

// round up to the next highest multiple of 4. unless x is already a multiple
// of 4
inline uint32_t RoundUp4(uint32_t x)
{
  return (x + 3) & ~((uint32_t)0x03);
}

// return the first 4 byte boundary after the end of a str4
// be careful about calling this version if you don't know whether
// the string is terminated correctly.
inline const char* FindStr4End(const char* p)
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
inline const char* FindStr4End(const char* p, const char* end)
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

constexpr inline bool is_valid_element_size_value(std::size_t x)
{
  // sizes may not be negative or exceed OSC_BUNDLE_ELEMENT_SIZE_MAX
  return x >= 0 && x <= OSC_BUNDLE_ELEMENT_SIZE_MAX;
}

constexpr inline bool is_multiple_of_4(std::size_t x)
{
  return (x & ((std::size_t)0x03)) == 0;
}

constexpr inline bool is_bundle(span dat)
{
  return (!dat.empty() && dat[0] == '#');
}

constexpr inline bool is_message(span dat)
{
  return !is_bundle(dat);
}
}

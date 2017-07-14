#pragma once
#include <asio.hpp>
#include <gsl/span>
#include <stdexcept>
#include <string_view>

namespace oscour
{
using span = gsl::span<const char>;
#define OSC_HOST_LITTLE_ENDIAN
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

inline void from_int32(char* p, int32_t x)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    int32_t i;
    char c[4];
  } u;

  u.i = x;

  p[3] = u.c[0];
  p[2] = u.c[1];
  p[1] = u.c[2];
  p[0] = u.c[3];
#else
  *reinterpret_cast<int32_t*>(p) = x;
#endif
}

inline void FromUInt32(char* p, uint32_t x)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    uint32_t i;
    char c[4];
  } u;

  u.i = x;

  p[3] = u.c[0];
  p[2] = u.c[1];
  p[1] = u.c[2];
  p[0] = u.c[3];
#else
  *reinterpret_cast<uint32_t*>(p) = x;
#endif
}

inline void FromInt64(char* p, int64_t x)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    int64_t i;
    char c[8];
  } u;

  u.i = x;

  p[7] = u.c[0];
  p[6] = u.c[1];
  p[5] = u.c[2];
  p[4] = u.c[3];
  p[3] = u.c[4];
  p[2] = u.c[5];
  p[1] = u.c[6];
  p[0] = u.c[7];
#else
  *reinterpret_cast<int64_t*>(p) = x;
#endif
}

inline void FromUInt64(char* p, uint64_t x)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    uint64_t i;
    char c[8];
  } u;

  u.i = x;

  p[7] = u.c[0];
  p[6] = u.c[1];
  p[5] = u.c[2];
  p[4] = u.c[3];
  p[3] = u.c[4];
  p[2] = u.c[5];
  p[1] = u.c[6];
  p[0] = u.c[7];
#else
  *reinterpret_cast<uint64_t*>(p) = x;
#endif
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

inline int32_t to_int32(const char* p)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    int32_t i;
    char c[4];
  } u;

  u.c[0] = p[3];
  u.c[1] = p[2];
  u.c[2] = p[1];
  u.c[3] = p[0];

  return u.i;
#else
  return *(int32_t*)p;
#endif
}

inline uint32_t to_uint32(const char* p)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    uint32_t i;
    char c[4];
  } u;

  u.c[0] = p[3];
  u.c[1] = p[2];
  u.c[2] = p[1];
  u.c[3] = p[0];

  return u.i;
#else
  return *(uint32_t*)p;
#endif
}

inline int64_t to_int64(const char* p)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    int64_t i;
    char c[8];
  } u;

  u.c[0] = p[7];
  u.c[1] = p[6];
  u.c[2] = p[5];
  u.c[3] = p[4];
  u.c[4] = p[3];
  u.c[5] = p[2];
  u.c[6] = p[1];
  u.c[7] = p[0];

  return u.i;
#else
  return *(int64_t*)p;
#endif
}

inline uint64_t to_uint64(const char* p)
{
#ifdef OSC_HOST_LITTLE_ENDIAN
  union {
    uint64_t i;
    char c[8];
  } u;

  u.c[0] = p[7];
  u.c[1] = p[6];
  u.c[2] = p[5];
  u.c[3] = p[4];
  u.c[4] = p[3];
  u.c[5] = p[2];
  u.c[6] = p[1];
  u.c[7] = p[0];

  return u.i;
#else
  return *(uint64_t*)p;
#endif
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

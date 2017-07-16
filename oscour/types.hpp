#pragma once
#include <oscour/exceptions.hpp>
#include <oscour/utils.hpp>

namespace oscour
{
enum type_tag
{
  TRUE_TYPE_TAG = 'T',
  FALSE_TYPE_TAG = 'F',
  NIL_TYPE_TAG = 'N',
  INFINITUM_TYPE_TAG = 'I',
  INT32_TYPE_TAG = 'i',
  FLOAT_TYPE_TAG = 'f',
  CHAR_TYPE_TAG = 'c',
  RGBA_COLOR_TYPE_TAG = 'r',
  MIDI_MESSAGE_TYPE_TAG = 'm',
  INT64_TYPE_TAG = 'h',
  TIME_TAG_TYPE_TAG = 't',
  DOUBLE_TYPE_TAG = 'd',
  STRING_TYPE_TAG = 's',
  SYMBOL_TYPE_TAG = 'S',
  BLOB_TYPE_TAG = 'b',
  ARRAY_BEGIN_TYPE_TAG = '[',
  ARRAY_END_TYPE_TAG = ']'
};

struct begin_bundle
{
  constexpr explicit begin_bundle(uint64_t tag = 1) : timeTag(tag)
  {
  }

  uint64_t timeTag{};
};
struct end_bundle
{
};

struct begin_message
{
  constexpr explicit begin_message(std::string_view mess)
    : addressPattern{mess}
  {
  }
  std::string_view addressPattern{};
};
struct end_message
{
};

struct nil
{
};
struct infinitum
{
};

struct rgba
{
  constexpr rgba() = default;
  constexpr explicit rgba(uint32_t value_) : value(value_)
  {
  }

  constexpr operator uint32_t() const
  {
    return value;
  }

  uint32_t value{};
};

struct midi
{
  constexpr midi() = default;
  constexpr explicit midi(uint32_t value_) : value(value_)
  {
  }

  constexpr operator uint32_t() const
  {
    return value;
  }

  uint32_t value{};
};

struct string
{
  constexpr string() = default;
  constexpr explicit string(const char* value_) : value(value_)
  {
  }

  constexpr operator std::string_view() const
  {
    return value;
  }

  std::string_view value{};
};

struct time_tag
{
  constexpr time_tag() = default;
  constexpr explicit time_tag(uint64_t value_) : value(value_)
  {
  }

  constexpr operator uint64_t() const
  {
    return value;
  }

  uint64_t value{};
};

struct symbol
{
  constexpr symbol() = default;
  constexpr explicit symbol(const char* value_) : value(value_)
  {
  }

  constexpr operator std::string_view() const
  {
    return value;
  }

  std::string_view value{};
};

struct blob
{
  constexpr blob() = default;
  constexpr explicit blob(const void* data_, std::size_t size_)
      : data(data_), size(size_)
  {
  }

  const void* data{};
  std::size_t size{};
};

struct begin_array
{
};
struct end_array
{
};
}

#pragma once
#include <oscour/types.hpp>

namespace oscour
{
template <typename T>
struct osc_type;

template <>
struct osc_type<int32_t>
{
  static const constexpr std::array typetags{INT32_TYPE_TAG};

  static int32_t read(const char* p, char tag)
  {
    return from_net<int32_t>(p);
  }

  static void write(char* p, int32_t v)
  {
    to_net(p, v);
  }
};

template <>
struct osc_type<int64_t>
{
  static const constexpr std::array typetags{INT64_TYPE_TAG};
  static int64_t read(const char* p, char tag)
  {
    return from_net<int64_t>(p);
  }

  static void write(char* p, int64_t v)
  {
    to_net(p, v);
  }
};

template <>
struct osc_type<float>
{
  static const constexpr std::array typetags{FLOAT_TYPE_TAG};
  static float read(const char* p, char tag)
  {
    return from_net<float>(p);
  }

  static void write(char* p, float v)
  {
    to_net(p, v);
  }
};

template <>
struct osc_type<double>
{
  static const constexpr std::array typetags{DOUBLE_TYPE_TAG};
  static double read(const char* p, char tag)
  {
    return from_net<double>(p);
  }

  static void write(char* p, double v)
  {
    to_net(p, v);
  }
};

template <>
struct osc_type<bool>
{
  static const constexpr std::array typetags{TRUE_TYPE_TAG, FALSE_TYPE_TAG};
  static bool read(const char* p, char tag)
  {
    return tag == TRUE_TYPE_TAG;
  }
};

template <>
struct osc_type<char>
{
  static const constexpr std::array typetags{CHAR_TYPE_TAG};

  static char read(const char* p, char tag)
  {
    return (char)from_net<int32_t>(p);
  }

  static void write(char* p, char v)
  {
    to_net(p, (int32_t)v);
  }
};

template <>
struct osc_type<nil>
{
  static const constexpr std::array typetags{NIL_TYPE_TAG};
  static nil read(const char* p, char tag)
  {
    return {};
  }
};

template <>
struct osc_type<infinitum>
{
  static const constexpr std::array typetags{INFINITUM_TYPE_TAG};
  static infinitum read(const char* p, char tag)
  {
    return {};
  }
};

template <>
struct osc_type<time_tag>
{
  static const constexpr std::array typetags{TIME_TAG_TYPE_TAG};
  static time_tag read(const char* p, char tag)
  {
    return time_tag{from_net<uint64_t>(p)};
  }
};

template <>
struct osc_type<rgba>
{
  static const constexpr std::array typetags{RGBA_COLOR_TYPE_TAG};
  static rgba read(const char* p, char tag)
  {
    return rgba{from_net<uint32_t>(p)};
  }
};

template <>
struct osc_type<midi>
{
  static const constexpr std::array typetags{MIDI_MESSAGE_TYPE_TAG};
  static midi read(const char* p, char tag)
  {
    return midi{from_net<uint32_t>(p)};
  }
};

template <>
struct osc_type<string>
{
  static const constexpr std::array typetags{STRING_TYPE_TAG};
  static string read(const char* p, char tag)
  {
    return string{p};
  }
};

template <>
struct osc_type<symbol>
{
  static const constexpr std::array typetags{SYMBOL_TYPE_TAG};
  static symbol read(const char* p, char tag)
  {
    return symbol{p};
  }
};

template <>
struct osc_type<blob>
{
  static const constexpr std::array typetags{BLOB_TYPE_TAG};

  static blob read(const char* p, char tag)
  {
    blob b;
    // read blob size as an unsigned int then validate
    std::size_t sizeResult = (std::size_t)from_net<uint32_t>(p);
    if (!is_valid_element_size_value(sizeResult))
      throw malformed_message("invalid blob size");

    b.size = sizeResult;
    b.data = (void*)(p + sizeof(int32_t));
  }
};

template <>
struct osc_type<begin_array>
{
  static const constexpr std::array typetags{ARRAY_BEGIN_TYPE_TAG};

  static begin_array read(const char* p, char tag)
  {
    return {};
  }
};

template <>
struct osc_type<end_array>
{
  static const constexpr std::array typetags{ARRAY_END_TYPE_TAG};

  static end_array read(const char* p, char tag)
  {
    return {};
  }
};
}

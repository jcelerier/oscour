#pragma once
#include <oscour/types.hpp>
#include <oscour/traits.hpp>
#include <oscour/bundle.hpp>
#include <oscour/exceptions.hpp>
#include <array>
#include <algorithm>

namespace oscour
{
class message_argument_view
{
public:
  message_argument_view() = default;
  message_argument_view(span t, span a) : m_typeTags(t), m_arguments(a)
  {
  }

  friend class message_argument_iterator;

  char type_tag() const
  {
    return m_typeTags[0];
  }

  template<typename Fun>
  void apply(Fun f) {
    switch(type_tag())
    {
      case oscour::type_tag::INT32_TYPE_TAG:
        f(get_unchecked<int32_t>()); break;
      case oscour::type_tag::INT64_TYPE_TAG:
        f(get_unchecked<int64_t>()); break;
      case oscour::type_tag::TRUE_TYPE_TAG:
      case oscour::type_tag::FALSE_TYPE_TAG:
        f(get_unchecked<bool>()); break;
      case oscour::type_tag::NIL_TYPE_TAG:
        f(get_unchecked<nil>()); break;
      case oscour::type_tag::INFINITUM_TYPE_TAG:
        f(get_unchecked<infinitum>()); break;
      case oscour::type_tag::FLOAT_TYPE_TAG:
        f(get_unchecked<float>()); break;
      case oscour::type_tag::DOUBLE_TYPE_TAG:
        f(get_unchecked<double>()); break;
      case oscour::type_tag::CHAR_TYPE_TAG:
        f(get_unchecked<char>()); break;
      case oscour::type_tag::STRING_TYPE_TAG:
        f(get_unchecked<string>()); break;
      case oscour::type_tag::SYMBOL_TYPE_TAG:
        f(get_unchecked<symbol>()); break;
      case oscour::type_tag::RGBA_COLOR_TYPE_TAG:
        f(get_unchecked<rgba>()); break;
      case oscour::type_tag::MIDI_MESSAGE_TYPE_TAG:
        f(get_unchecked<midi>()); break;
      case oscour::type_tag::TIME_TAG_TYPE_TAG:
        f(get_unchecked<time_tag>()); break;
      case oscour::type_tag::BLOB_TYPE_TAG:
        f(get_unchecked<blob>()); break;
      case oscour::type_tag::ARRAY_BEGIN_TYPE_TAG:
        f(get_unchecked<begin_array>()); break;
      case oscour::type_tag::ARRAY_END_TYPE_TAG:
        f(get_unchecked<end_array>()); break;
      default:
        break;
    }
  }

  template<typename T>
  bool is() const {
    return std::find(
          std::begin(osc_type<T>::typetags),
          std::end(osc_type<T>::typetags),
          type_tag()) != std::end(osc_type<T>::typetags);
  }

  template<typename T>
  T get_unchecked() const {
    using type = osc_type<T>;
    return type::read(m_arguments.data(), type_tag());
  }

  template<typename T>
  T get() const
  {
    using type = osc_type<T>;
    if (!m_typeTags.empty())
    {
      if(is<T>())
        return this->get_unchecked<T>();
      else
        throw wrong_argument_type();
    }
    else
    {
      throw missing_argument();
    }
  }

  // Calculate the number of top-level items in the array. Nested arrays count
  // as one item.
  // Only valid at array start. Will throw an std::runtime_error if
  // IsArrayStart() == false.
  std::size_t array_size() const
  {
    // it is only valid to call array_size when the argument is the
    // array start marker
    if (!this->is<begin_array>())
      throw wrong_argument_type();

    std::size_t result = 0;
    unsigned int level = 0;
    const char* typeTag = m_typeTags.data() + 1;

    // iterate through all type tags. note that ReceivedMessage::Init
    // has already checked that the message is well formed.
    while (*typeTag)
    {
      switch (*typeTag++)
      {
        case ARRAY_BEGIN_TYPE_TAG:
          level += 1;
          break;

        case ARRAY_END_TYPE_TAG:
          if (level == 0)
            return result;
          level -= 1;
          break;

        default:
          if (level == 0) // only count items at level 0
            ++result;
      }
    }

    return result;
  }

private:
  span m_typeTags;
  span m_arguments;
};



class message_argument_iterator
{
public:
  message_argument_iterator(span typeTags, span arguments)
      : m_value(typeTags, arguments)
  {
  }

  message_argument_iterator operator++()
  {
    advance();
    return *this;
  }

  message_argument_iterator operator++(int)
  {
    message_argument_iterator old(*this);
    advance();
    return old;
  }

  const message_argument_view& operator*() const
  {
    return m_value;
  }

  const message_argument_view* operator->() const
  {
    return &m_value;
  }

  friend bool operator==(
      const message_argument_iterator& lhs,
      const message_argument_iterator& rhs)
  {
    return lhs.is_equal_to(rhs);
  }

  friend bool operator!=(
      const message_argument_iterator& lhs,
      const message_argument_iterator& rhs)
  {
    return !(lhs == rhs);
  }

private:
  message_argument_view m_value;

  void advance()
  {
    if (m_value.m_typeTags.empty())
      return;

    auto t = m_value.type_tag();
    auto orig = m_value.m_typeTags;
    m_value.m_typeTags = m_value.m_typeTags.subspan(1);
    switch (t)
    {
      case '\0':
        // don't advance past end
        m_value.m_typeTags = orig;
        break;

      case TRUE_TYPE_TAG:
      case FALSE_TYPE_TAG:
      case NIL_TYPE_TAG:
      case INFINITUM_TYPE_TAG:

        // zero length
        break;

      case INT32_TYPE_TAG:
      case FLOAT_TYPE_TAG:
      case CHAR_TYPE_TAG:
      case RGBA_COLOR_TYPE_TAG:
      case MIDI_MESSAGE_TYPE_TAG:
        m_value.m_arguments = m_value.m_arguments.subspan(4);
        break;

      case INT64_TYPE_TAG:
      case TIME_TAG_TYPE_TAG:
      case DOUBLE_TYPE_TAG:
        m_value.m_arguments = m_value.m_arguments.subspan(8);
        break;

      case STRING_TYPE_TAG:
      case SYMBOL_TYPE_TAG:

        // we use the unsafe function FindStr4End(char*) here because all of
        // the arguments have already been validated in
        // ReceivedMessage::Init() below.

        m_value.m_arguments = { FindStr4End(m_value.m_arguments.data()),
                                m_value.m_arguments.data() + m_value.m_arguments.size() };
        break;

      case BLOB_TYPE_TAG:
      {
        // treat blob size as an unsigned int for the purposes of this
        // calculation
        uint32_t blobSize = to_uint32(m_value.m_arguments.data());
        m_value.m_arguments = { m_value.m_arguments.data() + sizeof(int32_t)
                             + RoundUp4(blobSize), m_value.m_arguments.data() + m_value.m_arguments.size()} ;
      }
      break;

      case ARRAY_BEGIN_TYPE_TAG:
      case ARRAY_END_TYPE_TAG:

        //    [ Indicates the beginning of an array. The tags following are for
        //        data in the Array until a close brace tag is reached.
        //    ] Indicates the end of an array.

        // zero length, don't advance argument ptr
        break;

      default: // unknown type tag
        // don't advance
        m_value.m_typeTags = orig;
        break;
    }
  }

  bool is_equal_to(const message_argument_iterator& rhs) const
  {
    return m_value.m_typeTags == rhs.m_value.m_typeTags;
  }
};



class message_view_stream
{
public:
  template<typename T>
  message_view_stream& operator>>(T& rhs)
  {
    if (p_ == end_)
      throw missing_argument();

    rhs = (*p_++).get<T>();
    return *this;
  }

  message_view_stream& operator>>(end_message&)
  {
    if (p_ != end_)
      throw excess_argument();

    return *this;
  }

private:
  friend class message_view;
  message_view_stream(
      const message_argument_iterator& begin,
      const message_argument_iterator& end)
      : p_(begin), end_(end)
  {
  }

  message_argument_iterator p_, end_;
};

class message_view
{
public:
  using const_iterator = message_argument_iterator;
  explicit message_view(span p)
    : m_message{p}
  {
    init(m_message);
  }
  explicit message_view(bundle_element_view b)
    : m_message{b.data()}
  {
    init(m_message);
  }
  span address_pattern() const
  {
    return m_addressPattern;
  }

  // Support for non-standard SuperCollider integer address patterns:
  bool address_pattern_is_uint32() const
  {
    return (m_addressPattern[0] == '\0');
  }
  uint32_t address_pattern_as_uint32() const
  {
    return to_uint32(m_addressPattern.data());
  }

  std::size_t argument_count() const
  {
    return m_typeTags.size();
  }

  span type_tags() const
  {
    return m_typeTags;
  }

  message_argument_iterator begin() const
  {
    return message_argument_iterator(m_typeTags, m_arguments);
  }

  message_argument_iterator end() const
  {
    return message_argument_iterator(span(m_typeTags.data() + size(), std::ptrdiff_t{}), {});
  }

  message_view_stream stream() const
  {
    return message_view_stream(begin(), end());
  }

  std::size_t size() const
  {
    return m_message.size();
  }

private:
  void init(span message)
  {
    const auto size = message.size();
    if (!is_valid_element_size_value(message.size()))
      throw malformed_message("invalid message size");

    if (message.size() == 0)
      throw malformed_message("zero length messages not permitted");

    if (!is_multiple_of_4(message.size()))
      throw malformed_message("message size must be multiple of four");

    const auto begin = message.data();
    const auto end = message.data() + size;

    auto tagBegin = FindStr4End(begin, end);
    if (tagBegin == nullptr)
    {
      // address pattern was not terminated before end
      throw malformed_message("unterminated address pattern");
    }
    else if (m_typeTags.data() + m_typeTags.size() == end)
    {
      // message consists of only the address pattern - no arguments or type
      // tags.
      m_addressPattern = message;
    }
    else
    {
      m_typeTags = {tagBegin, end};
      m_addressPattern = {message.data(), m_typeTags.data()};
      if (m_typeTags[0] != ',')
        throw malformed_message("type tags not present");

      if (m_typeTags[1] == '\0')
      {
        // zero length type tags
        m_typeTags = {};
      }
      else
      {
        // check that all arguments are present and well formed
        auto argBegin = FindStr4End(m_typeTags.data(), end);
        if (argBegin == nullptr)
          throw malformed_message(
              "type tags were not terminated before end of message");

        m_arguments = {argBegin, end};
        m_typeTags = m_typeTags.subspan(1); // advance past initial ','

        const char* typeTag = m_typeTags.data();
        const char* argument = m_arguments.data();
        unsigned int arrayLevel = 0;

        do
        {
          switch (*typeTag)
          {
            case TRUE_TYPE_TAG:
            case FALSE_TYPE_TAG:
            case NIL_TYPE_TAG:
            case INFINITUM_TYPE_TAG:
              // zero length
              break;

            //    [ Indicates the beginning of an array. The tags following are
            //    for
            //        data in the Array until a close brace tag is reached.
            //    ] Indicates the end of an array.
            case ARRAY_BEGIN_TYPE_TAG:
              ++arrayLevel;
              // (zero length argument data)
              break;

            case ARRAY_END_TYPE_TAG:
              --arrayLevel;
              // (zero length argument data)
              break;

            case INT32_TYPE_TAG:
            case FLOAT_TYPE_TAG:
            case CHAR_TYPE_TAG:
            case RGBA_COLOR_TYPE_TAG:
            case MIDI_MESSAGE_TYPE_TAG:

              if (argument == end)
                throw malformed_message("arguments exceed message size");
              argument += 4;
              if (argument > end)
                throw malformed_message("arguments exceed message size");
              break;

            case INT64_TYPE_TAG:
            case TIME_TAG_TYPE_TAG:
            case DOUBLE_TYPE_TAG:

              if (argument == end)
                throw malformed_message("arguments exceed message size");
              argument += 8;
              if (argument > end)
                throw malformed_message("arguments exceed message size");
              break;

            case STRING_TYPE_TAG:
            case SYMBOL_TYPE_TAG:

              if (argument == end)
                throw malformed_message("arguments exceed message size");
              argument = FindStr4End(argument, end);
              if (argument == 0)
                throw malformed_message("unterminated string argument");
              break;

            case BLOB_TYPE_TAG:
            {
              // treat blob size as an unsigned int for the purposes of this
              // calculation
              uint32_t blobSize = to_uint32(argument);
              argument
                  = argument + oscour::OSC_SIZEOF_INT32 + RoundUp4(blobSize);
              if (argument > end)
                malformed_message("arguments exceed message size");
            }
            break;

            default:
              throw malformed_message("unknown type tag");
          }

        } while (*++typeTag != '\0');
        m_typeTags = {m_typeTags.data(), typeTag};

        if (arrayLevel != 0)
          throw malformed_message(
              "array was not terminated before end of message (expected ']' "
              "end of array tag)");
      }
    }
  }

  span m_message;
  span m_addressPattern;
  span m_typeTags;
  span m_arguments;
};
}

inline auto begin(const oscour::message_view& mes)
{
  return mes.begin();
}

inline auto end(const oscour::message_view& mes)
{
  return mes.end();
}

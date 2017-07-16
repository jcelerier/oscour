#pragma once
#include <oscour/exceptions.hpp>
#include <oscour/types.hpp>
#include <oscour/traits.hpp>
#include <string>
#include <string_view>

namespace oscour
{
class outbound_stream
{
public:
  outbound_stream(span sp)
    : outbound_stream{(char*)sp.data(), gsl::narrow_cast<std::size_t>(sp.size())}
  {

  }

  outbound_stream(char* buffer, std::size_t capacity)
      : m_data(buffer)
      , m_end(m_data + capacity)
      , m_typeTagsCurrent(m_end)
      , m_messageCursor(m_data)
      , m_argumentCurrent(m_data)
      , m_elementSizePtr(0)
      , m_messageInProgress(false)
  {
  }

  ~outbound_stream()
  {
  }

  void clear()
  {
    m_typeTagsCurrent = m_end;
    m_messageCursor = m_data;
    m_argumentCurrent = m_data;
    m_elementSizePtr = 0;
    m_messageInProgress = false;
  }

  std::size_t capacity() const
  {
    return m_end - m_data;
  }

  // invariant: size() is valid even while building a message.
  std::size_t size() const
  {
    std::size_t result = m_argumentCurrent - m_data;
    if (is_message_in_progress())
    {
      // account for the length of the type tag string. the total type tag
      // includes an initial comma, plus at least one terminating \0
      result += round_up_4((m_end - m_typeTagsCurrent) + 2);
    }

    return result;
  }

  const char* data() const
  {
    return m_data;
  }

  // indicates that all messages have been closed with a matching EndMessage
  // and all bundles have been closed with a matching EndBundle
  bool is_ready() const
  {
    return (!is_message_in_progress() && !is_bundle_in_progress());
  }

  bool is_message_in_progress() const
  {
    return m_messageInProgress;
  }
  bool is_bundle_in_progress() const
  {
    return (m_elementSizePtr != 0);
  }

  template <
      typename T,
      typename std::enable_if_t<!std::is_same<char, std::remove_const_t<T>>::
                                    value>* = nullptr>
  outbound_stream& operator<<(T* rhs) = delete;

  outbound_stream& operator<<(begin_bundle rhs)
  {
    if (is_message_in_progress())
      throw message_in_progress();

    check_available_bundle_space();

    m_messageCursor = begin_element(m_messageCursor);

    std::memcpy(m_messageCursor, "#bundle\0", 8);
    to_net(m_messageCursor + 8, rhs.timeTag);

    m_messageCursor += 16;
    m_argumentCurrent = m_messageCursor;

    return *this;
  }

  outbound_stream& operator<<(end_bundle)
  {
    if (!is_bundle_in_progress())
      throw bundle_not_in_progress();
    if (is_message_in_progress())
      throw message_in_progress();

    end_element(m_messageCursor);

    return *this;
  }

  void write(std::size_t size)
  {
    to_net(m_messageCursor, size);
  }

  void write(outbound_stream& other)
  {
    to_net(m_messageCursor, (int32_t)other.size());
    m_messageCursor += 4;
    std::memcpy(m_messageCursor, other.data(), other.size());
  }

  outbound_stream& operator<<(begin_message rhs)
  {
    if (is_message_in_progress())
      throw message_in_progress();

    std::size_t rhsLength = std::strlen(rhs.addressPattern.data());
    check_available_message_space(rhsLength);

    m_messageCursor = begin_element(m_messageCursor);

    std::strcpy(m_messageCursor, rhs.addressPattern.data());
    m_messageCursor += rhsLength + 1;

    // zero pad to 4-byte boundary
    std::size_t i = rhsLength + 1;
    while (i & 0x3)
    {
      *m_messageCursor++ = '\0';
      ++i;
    }

    m_argumentCurrent = m_messageCursor;
    m_typeTagsCurrent = m_end;

    m_messageInProgress = true;

    return *this;
  }

  outbound_stream& operator<<(end_message rhs)
  {
    (void)rhs;

    if (!is_message_in_progress())
      throw message_not_in_progress();

    std::size_t typeTagsCount = m_end - m_typeTagsCurrent;

    if (typeTagsCount)
    {

      char* tempTypeTags = (char*)alloca(typeTagsCount);
      std::memcpy(tempTypeTags, m_typeTagsCurrent, typeTagsCount);

      // slot size includes comma and null terminator
      std::size_t typeTagSlotSize = round_up_4(typeTagsCount + 2);

      std::size_t argumentsSize = m_argumentCurrent - m_messageCursor;

      std::memmove(
          m_messageCursor + typeTagSlotSize, m_messageCursor, argumentsSize);

      m_messageCursor[0] = ',';
      // copy type tags in reverse (really forward) order
      for (std::size_t i = 0; i < typeTagsCount; ++i)
        m_messageCursor[i + 1] = tempTypeTags[(typeTagsCount - 1) - i];

      char* p = m_messageCursor + 1 + typeTagsCount;
      for (std::size_t i = 0; i < (typeTagSlotSize - (typeTagsCount + 1)); ++i)
        *p++ = '\0';

      m_typeTagsCurrent = m_end;

      // advance messageCursor_ for next message
      m_messageCursor += typeTagSlotSize + argumentsSize;
    }
    else
    {
      // send an empty type tags string
      std::memcpy(m_messageCursor, ",\0\0\0", 4);

      // advance messageCursor_ for next message
      m_messageCursor += 4;
    }

    m_argumentCurrent = m_messageCursor;

    end_element(m_messageCursor);

    m_messageInProgress = false;

    return *this;
  }

  outbound_stream& operator<<(bool rhs)
  {
    check_available_argument_space(0);

    *(--m_typeTagsCurrent) = (char)((rhs) ? TRUE_TYPE_TAG : FALSE_TYPE_TAG);

    return *this;
  }

  outbound_stream& operator<<(infinitum rhs)
  {
    (void)rhs;
    check_available_argument_space(0);

    *(--m_typeTagsCurrent) = INFINITUM_TYPE_TAG;

    return *this;
  }

  outbound_stream& operator<<(Arithmetic rhs)
  {
    check_available_argument_space(sizeof(rhs));

    *(--m_typeTagsCurrent) = osc_type<decltype(rhs)>::typetags[0];
    to_net(m_argumentCurrent, rhs);
    m_argumentCurrent += sizeof(rhs);

    return *this;
  }

  outbound_stream& operator<<(HasValue rhs)
  {
    check_available_argument_space(sizeof(rhs.value));

    *(--m_typeTagsCurrent) = osc_type<decltype(rhs)>::typetags[0];
    to_net(m_argumentCurrent, rhs.value);
    m_argumentCurrent += sizeof(rhs.value);

    return *this;
  }

  outbound_stream& operator<<(char rhs)
  {
    check_available_argument_space(4);

    *(--m_typeTagsCurrent) = CHAR_TYPE_TAG;
    to_net(m_argumentCurrent, (int32_t)rhs);
    m_argumentCurrent += 4;

    return *this;
  }

  outbound_stream& operator<<(std::string_view rhs)
  {
    check_available_argument_space(round_up_4(rhs.size() + 1));

    *(--m_typeTagsCurrent) = STRING_TYPE_TAG;
    if (!rhs.empty())
      std::memcpy(m_argumentCurrent, rhs.data(), rhs.size());
    m_argumentCurrent += rhs.size();
    *m_argumentCurrent++ = '\0';

    // zero pad to 4-byte boundary
    std::size_t i = rhs.size() + 1;
    while (i & 0x3)
    {
      *m_argumentCurrent++ = '\0';
      ++i;
    }

    return *this;
  }

  outbound_stream& operator<<(const std::string& rhs)
  {
    operator<<(std::string_view(rhs));
    return *this;
  }

  outbound_stream& operator<<(string rhs)
  {
    operator<<(rhs.value);
    return *this;
  }

  template <std::size_t N>
  outbound_stream& operator<<(const char (&ref)[N])
  {
    operator<<(std::string_view(ref, N));
    return *this;
  }

  outbound_stream& operator<<(symbol str)
  {
    const auto rhs = str.value;
    check_available_argument_space(round_up_4(rhs.size() + 1));

    *(--m_typeTagsCurrent) = SYMBOL_TYPE_TAG;
    if (!rhs.empty())
      std::memcpy(m_argumentCurrent, rhs.data(), rhs.size());
    m_argumentCurrent += rhs.size();
    *m_argumentCurrent++ = '\0';

    // zero pad to 4-byte boundary
    std::size_t i = rhs.size() + 1;
    while (i & 0x3)
    {
      *m_argumentCurrent++ = '\0';
      ++i;
    }

    return *this;
  }

  outbound_stream& operator<<(blob rhs)
  {
    check_available_argument_space(4 + round_up_4(rhs.size));

    *(--m_typeTagsCurrent) = BLOB_TYPE_TAG;
    to_net(m_argumentCurrent, rhs.size);
    m_argumentCurrent += 4;

    std::memcpy(m_argumentCurrent, rhs.data, rhs.size);
    m_argumentCurrent += rhs.size;

    // zero pad to 4-byte boundary
    unsigned long i = rhs.size;
    while (i & 0x3)
    {
      *m_argumentCurrent++ = '\0';
      ++i;
    }

    return *this;
  }

  outbound_stream& operator<<(begin_array rhs)
  {
    (void)rhs;
    check_available_argument_space(0);

    *(--m_typeTagsCurrent) = ARRAY_BEGIN_TYPE_TAG;

    return *this;
  }

  outbound_stream& operator<<(end_array rhs)
  {
    (void)rhs;
    check_available_argument_space(0);

    *(--m_typeTagsCurrent) = ARRAY_END_TYPE_TAG;

    return *this;
  }

private:
  char* begin_element(char* beginPtr)
  {
    if (m_elementSizePtr == 0)
    {
      m_elementSizePtr = reinterpret_cast<uint32_t*>(m_data);
      return beginPtr;
    }
    else
    {
      // store an offset to the old element size ptr in the element size slot
      // we store an offset rather than the actual pointer to be 64 bit clean.
      *reinterpret_cast<uint32_t*>(beginPtr)
          = (uint32_t)(reinterpret_cast<char*>(m_elementSizePtr) - m_data);

      m_elementSizePtr = reinterpret_cast<uint32_t*>(beginPtr);

      return beginPtr + 4;
    }
  }

  void end_element(char* endPtr)
  {
    assert(m_elementSizePtr != 0);

    if (m_elementSizePtr == reinterpret_cast<uint32_t*>(m_data))
    {
      m_elementSizePtr = 0;
    }
    else
    {
      // while building an element, an offset to the containing element's
      // size slot is stored in the elements size slot (or a ptr to data_
      // if there is no containing element). We retrieve that here
      uint32_t* previousElementSizePtr
          = reinterpret_cast<uint32_t*>(m_data + *m_elementSizePtr);

      // then we store the element size in the slot. note that the element
      // size does not include the size slot, hence the - 4 below.

      std::ptrdiff_t d = endPtr - reinterpret_cast<char*>(m_elementSizePtr);
      // assert( d >= 4 && d <= 0x7FFFFFFF ); // assume packets smaller than
      // 2Gb

      uint32_t elementSize = static_cast<uint32_t>(d - 4);
      to_net(reinterpret_cast<char*>(m_elementSizePtr), elementSize);

      // finally, we reset the element size ptr to the containing element
      m_elementSizePtr = previousElementSizePtr;
    }
  }

  bool element_size_slot_required() const
  {
    return (m_elementSizePtr != 0);
  }


  void check_available_bundle_space()
  {
    std::size_t required = size() + ((element_size_slot_required()) ? 4 : 0) + 16;

    if (required > capacity())
      throw out_of_memory{};
  }

  void check_available_message_space(std::size_t addressPatternSize)
  {
    // plus 4 for at least four bytes of type tag
    std::size_t required = size() + ((element_size_slot_required()) ? 4 : 0)
                           + round_up_4(addressPatternSize + 1) + 4;

    if (required > capacity())
      throw out_of_memory{};
  }

  void check_available_argument_space(std::size_t argumentLength)
  {
    // plus three for extra type tag, comma and null terminator
    std::size_t required = (m_argumentCurrent - m_data) + argumentLength
                           + round_up_4((m_end - m_typeTagsCurrent) + 3);

    if (required > capacity())
      throw out_of_memory{};
  }

  char* const m_data{};
  char* const m_end{};

  char* m_typeTagsCurrent{}; // stored in reverse order
  char* m_messageCursor{};
  char* m_argumentCurrent{};

  // m_elementSizePtr has two special values: 0 indicates that a bundle
  // isn't open, and m_elementSizePtr==data_ indicates that a bundle is
  // open but that it doesn't have a size slot (ie the outermost bundle)
  uint32_t* m_elementSizePtr{};

  bool m_messageInProgress{};
};



}

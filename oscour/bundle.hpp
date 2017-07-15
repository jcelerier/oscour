#pragma once
#include <oscour/utils.hpp>
#include <oscour/exceptions.hpp>

namespace oscour
{
class bundle_element_view
{
public:
  bundle_element_view(const char* sizePtr)
      : m_data{sizePtr + sizeof(int32_t), to_T<int32_t>(sizePtr)}
  {
  }

  span data() const
  {
    return m_data;
  }

private:
  span m_data;
};

class bundle_view
{
public:
  class iterator
  {
  public:
    iterator(const char* sizePtr)
      : m_value{sizePtr} { }

    iterator operator++() {
      advance();
      return *this;
    }

    iterator operator++(int) {
      iterator old(*this);
      advance();
      return old;
    }

    const bundle_element_view& operator*() const
    { return m_value; }
    const bundle_element_view* operator->() const
    { return &m_value; }

    friend bool operator==( const iterator& lhs, const iterator& rhs)
    { return lhs.m_value.data() == rhs.m_value.data(); }

    friend bool operator!=( const iterator& lhs, const iterator& rhs)
    { return !(lhs == rhs); }

  private:
    void advance() {
      m_value = bundle_element_view{m_value.data().data() + m_value.data().size()};
    }
    bundle_element_view m_value;
  };

  using const_iterator = iterator;

  explicit bundle_view(const span& p) : elementCount_(0)
  {
    init(p);
  }
  explicit bundle_view(const bundle_element_view& e)
      : elementCount_(0)
  {
    init(e.data());
  }

  uint64_t time_tag() const
  {
    return to_T<uint64_t>(timeTag_);
  }

  uint32_t size() const
  {
    return elementCount_;
  }

  const_iterator begin() const
  {
    return const_iterator(timeTag_ + 8);
  }

  const_iterator end() const
  {
    return const_iterator(end_);
  }

private:
  void init(span bundle)
  {
    if (!is_valid_element_size_value(bundle.size()))
      throw malformed_bundle("invalid bundle size");

    if (bundle.size() < 16)
      throw malformed_bundle("packet too short for bundle");

    if (!is_multiple_of_4(bundle.size()))
      throw malformed_bundle("bundle size must be multiple of four");

    if (bundle.subspan(0, 8) != span("#bundle"))
      throw malformed_bundle("bad bundle address pattern");

    end_ = bundle.data() + bundle.size();

    timeTag_ = bundle.data() + 8;

    const char* p = timeTag_ + 8;

    while (p < end_)
    {
      if (p + oscour::OSC_SIZEOF_INT32 > end_)
        throw malformed_bundle("packet too short for elementSize");

      // treat element size as an unsigned int for the purposes of this
      // calculation
      uint32_t elementSize = to_T<uint32_t>(p);
      if ((elementSize & ((uint32_t)0x03)) != 0)
        throw malformed_bundle("bundle element size must be multiple of four");

      p += oscour::OSC_SIZEOF_INT32 + elementSize;
      if (p > end_)
        throw malformed_bundle("packet too short for bundle element");

      ++elementCount_;
    }

    if (p != end_)
      throw malformed_bundle("bundle contents ");
  }
  const char* timeTag_;
  const char* end_;
  uint32_t elementCount_;
};

}

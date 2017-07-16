#pragma once
#include <oscour/outbound_stream.hpp>
#include <array>
#include <algorithm>
#include <new>
#include <variant>
#include <array>
#include <vector>

namespace oscour
{

template <std::size_t N>
using static_message = std::array<char, N>;
using dynamic_message = std::vector<char>;

template<typename T>
constexpr std::size_t argument_size(T)
{ return sizeof(T); }

constexpr std::size_t argument_size(bool)
{ return 0; }

constexpr std::size_t argument_size(std::string_view v)
{ return round_up_4(v.size()); }

template<typename T>
constexpr std::size_t typetag_size(T)
{ return 1; }

template<typename... Args>
constexpr std::size_t message_size(std::string_view addr, const Args&... args)
{
  if(sizeof...(Args) > 0)
  {
    return round_up_4(addr.size())
        + round_up_4(1 + (... + typetag_size(args)))
        + (... + argument_size(args));
  }
  else
  {
    return addr.size();
  }
}

template<std::size_t N = 2048>
class safe_message
{
public:
  using static_t = static_message<N>;
  using dynamic_t = dynamic_message;

  safe_message(std::size_t size)
  {
    if(size > N)
    {
      m_buffer = dynamic_t(size);
    }
  }

  void resize(std::size_t s)
  {
    const auto size = this->size();
    if(s < size)
      return;

    switch(m_buffer.index())
    {
      case 0:
      {
        if(s > N)
        {
          m_buffer = dynamic_t(s);
        }
        break;
      }

      case 1:
      {
        if(s > size)
        {
          m_buffer = dynamic_t(s);
        }
        break;
      }
    }
  }

  char* data() {
    return std::visit([] (auto& vec) { return vec.data(); }, m_buffer);
  }

  std::size_t size() const {
    return std::visit([] (auto& vec) { return vec.size(); }, m_buffer);
  }

  operator oscour::span() {
    return std::visit([] (auto& vec) {
      return oscour::span{vec};
    }, m_buffer);
  }

  void clear() {
    return std::visit([] (auto& vec) { vec.clear(); }, m_buffer);
  }

private:
  std::variant<static_t, dynamic_t> m_buffer;
};

template<typename Buffer = safe_message<>>
class message
{
public:
  message(std::string_view addr, const Any&... args)
    : m_buffer{message_size(addr, args...)}
    , m_stream{oscour::span{m_buffer}}
  {
    m_stream << begin_message{addr};
    make(args...);
    m_stream << end_message{};
  }

  void set(std::string_view addr, const Any&... args)
  {
    m_buffer.resize(message_size(addr, args...));
    m_stream.~outbound_stream();
    new (&m_stream) outbound_stream{oscour::span{m_buffer}};

    m_stream << begin_message{addr};
    make(args...);
    m_stream << end_message{};
  }

  operator oscour::span() {
    return {m_stream.data(), gsl::narrow_cast<std::ptrdiff_t>(m_stream.size())};
  }

private:
  void make(const NotAPointer& arg1, const Any&... args)
  {
    m_stream << arg1;

    if constexpr(sizeof...(args) > 0)
        make(args...);
  }

  Buffer m_buffer;
  outbound_stream m_stream;
};
struct bundle
{
  time_tag tag;
  std::vector<std::variant<bundle, message<>>> data;
};
/*
struct bundle_builder
{
  bundle_builder()
  {
    m_buffer.resize(16384);
    // TODO if a runtime resize is required,
    // how to keep simply the osc outbound stream pointers ?
    // first convert them into distance to zero,
    // then update the stream
    // then reconvert them to the new pointer position
    // also do this for the pointers embedded in the elemnts ?
  }

  void add(gsl::span<std::variant<bundle, message<>>> data)
  {
    start();
    for(const auto& e : data)
    {
      std::visit([&] (const auto& elt) { this->add(elt); }, e);
    }
    finish();
  }

  void add(const bundle& b)
  {
    m_stream << begin_bundle{b.tag};

    for(const auto& e : b.data)
    {
      std::visit([&] (const auto& elt) { this->add(elt); }, e);
    }

    m_stream << end_bundle{};
  }

  void add(const message<Any>& b)
  {
    m_stream.write(b.m_stream);
  }

  void start()
  {
    m_stream << begin_bundle{};
  }

  void add_message(std::string_view addr, const Any&... args)
  {
    m_stream << begin_message{addr};
    make(args...);
    m_stream << end_message{};
  }

  void finish()
  {
    m_stream << end_bundle{};
  }

private:
  dynamic_message m_buffer;
  outbound_stream m_stream;
};
*/

}

#pragma once
#include <oscour/message.hpp>
#include <oscour/bundle.hpp>
#include <boost/lexical_cast.hpp>
#include <type_traits>

namespace oscour
{
template<typename T>
concept bool Arithmetic = std::is_arithmetic_v<T>;

template<typename T> struct yield_false : public std::false_type { };
struct lax_converter
{
  template<typename U>
  void operator()(const nil&, U& u) { u = {}; }
  template<typename U>
  void operator()(const infinitum&, U& u) { u = {}; }

  void operator()(const rgba& t, Arithmetic& u) { u = t.value; }
  void operator()(const midi& t, Arithmetic& u) { u = t.value; }
  void operator()(const time_tag& t, Arithmetic& u) { u = t.value; }
  void operator()(const blob& t, std::string& u) { u = std::string((char*)t.data, t.size); }

  template<typename U>
  void operator()(const string& s, U& u) {
    u = boost::lexical_cast<U>(s.value);
  }

  template<typename U>
  void operator()(const symbol& s, U& u) {
    u = boost::lexical_cast<U>(s.value);
  }

  template<typename T>
  void operator()(const T& t, std::string& u) {
    u = boost::lexical_cast<std::string>(t);
  }

};

class osc_lax_receiver
{
public:
  template<typename Fun>
  void on_message(const std::string& data, Fun f)
  {
    // TODO remove trailing '\0'
    if constexpr(std::is_invocable_v<Fun, const oscour::message_view&>)
      m_simpleHandlers.insert({data, f});
    else
      m_messageHandlers.insert({data, addHandler_impl(f, &Fun::operator())});
  }

  template<typename Fun>
  void on_bundle(Fun f)
  {
    m_bundleHandler = std::move(f);
  }

  void operator()(oscour::span f)
  {
    if(oscour::is_bundle(f))
    {
      handle_bundle(oscour::bundle_view{f});
    }
    else
    {
      handle_message(oscour::message_view{f});
    }
  }

private:
  void handle_bundle(const oscour::bundle_view& b)
  {
    if(m_bundleHandler)
    {
      m_bundleHandler(b);
    }
    else
    {
      for(const auto& bv : b)
      {
        bundle_to_messages_rec(bv);
      }
    }
  }

  void handle_message(const oscour::message_view& m)
  {
    auto pattern = std::string(m.address_pattern().data(), m.address_pattern().size());

    auto it = m_simpleHandlers.find(pattern);
    if(it != m_simpleHandlers.end())
    {
      try { (it->second)(m); } catch(...) { }
    }

    auto [begin, end] = m_messageHandlers.equal_range(pattern);
    for(auto it = begin; it != end; ++it)
    {
      try { (it->second)(m); } catch(...) { }
    }
  }

  void bundle_to_messages_rec(const oscour::bundle_element_view& v)
  {
    auto dat = v.data();
    if(oscour::is_message(dat))
    {
      handle_message(oscour::message_view{dat});
    }
    else
    {
      oscour::bundle_view b{v.data()};
      for(const auto& bv : b)
        bundle_to_messages_rec(bv);
    }
  }

  using iterator = oscour::message_argument_iterator;
  template<typename Fun, typename... Args>
  static auto addHandler_impl_sub(iterator b, iterator e, Fun& f, oscour::dummy<>, Args&&... args)
  {
    if(b == e)
    {
      f(std::forward<Args>(args)...);
    }
    else
    {
      throw;
    }
  }

  template<typename Fun, typename Arg, typename... Args, typename... Args2>
  static auto addHandler_impl_sub(iterator b, iterator e, Fun& f, oscour::dummy<Arg, Args...>, Args2&&... args)
  {
    if(b != e)
    {
      Arg a;
      b->apply([&] (const auto& val) {
        using arg_type = std::remove_reference_t<decltype(val)>;
        if constexpr(std::is_convertible_v<arg_type, Arg>)
          a = val;
        else if constexpr(std::is_invocable_v<lax_converter, const arg_type&, Arg&>)
          lax_converter{}(val, a);
        else
          a = {};
      });

      addHandler_impl_sub(++b, e, f, oscour::dummy<Args...>{}, std::forward<Args2>(args)..., std::move(a));
    }
    else
    {
      throw;
    }
  }

  template<typename Fun, typename... Args>
  static auto addHandler_impl(Fun f, void (Fun::*) (Args...) const)
  {
    return [fun=std::move(f)] (oscour::message_view m) {

      addHandler_impl_sub(m.begin(), m.end(), fun, oscour::dummy<Args...>{});
    };
  }

  template<typename Fun, typename... Args>
  static auto addHandler_impl(Fun f, void (Fun::*) (Args...))
  {
    return [fun=std::move(f)] (oscour::message_view m) {
      addHandler_impl_sub(m.begin(), m.end(), fun, oscour::dummy<Args...>{});
    };
  }


  std::function<void(const oscour::bundle_view&)> m_bundleHandler;
  std::unordered_map<std::string, std::function<void(const oscour::message_view&)>> m_simpleHandlers;
  std::unordered_multimap<std::string, std::function<void(const oscour::message_view&)>> m_messageHandlers;
};
}

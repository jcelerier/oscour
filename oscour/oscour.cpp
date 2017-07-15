#include "oscour.hpp"
#include <iostream>

namespace oscour
{
std::ostream& operator<<(std::ostream& s, nil) {
  s << "nil";
  return s;
}
std::ostream& operator<<(std::ostream& s, infinitum) {
  s << "infinitum";
  return s;
}
std::ostream& operator<<(std::ostream& s, string v) {
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, symbol v) {
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, rgba v) {
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, midi v) {
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, time_tag v) {
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, blob v) {
  s << "blob";
  return s;
}
std::ostream& operator<<(std::ostream& s, begin_array v) {
  s << "[";
  return s;
}
std::ostream& operator<<(std::ostream& s, end_array v) {
  s << "]";
  return s;
}
template<typename... Args> struct dummy {} ;


class osc_strict_receiver
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
    if(begin != end)
    {
      for(auto it = begin; it != end; ++it)
      {
        try { (it->second)(m); } catch(...) { }
      }
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

  template<typename Fun, typename... Args>
  static auto addHandler_impl_sub(oscour::message_view_stream& s, Fun& f, oscour::dummy<>, Args&&... args)
  {
    oscour::end_message m;
    s >> m;

    f(std::forward<Args>(args)...);
  }

  template<typename Fun, typename Arg, typename... Args, typename... Args2>
  static auto addHandler_impl_sub(oscour::message_view_stream& s, Fun& f, oscour::dummy<Arg, Args...>, Args2&&... args)
  {
    Arg a;
    s >> a;

    addHandler_impl_sub(s, f, oscour::dummy<Args...>{}, std::forward<Args2>(args)..., std::move(a));
  }

  template<typename Fun, typename... Args>
  static auto addHandler_impl(Fun f, void (Fun::*) (Args...) const)
  {
    return [fun=std::move(f)] (oscour::message_view m) {
      auto s = m.stream();
      addHandler_impl_sub(s, fun, oscour::dummy<Args...>{});
    };
  }

  template<typename Fun, typename... Args>
  static auto addHandler_impl(Fun f, void (Fun::*) (Args...))
  {
    return [fun=std::move(f)] (oscour::message_view m) {
      auto s = m.stream();
      addHandler_impl_sub(s, fun, oscour::dummy<Args...>{});
    };
  }


  std::function<void(const oscour::bundle_view&)> m_bundleHandler;
  std::unordered_map<std::string, std::function<void(const oscour::message_view&)>> m_simpleHandlers;
  std::unordered_multimap<std::string, std::function<void(const oscour::message_view&)>> m_messageHandlers;
};

class osc_lax_receiver
{

};
}


int main()
{
  oscour::osc_strict_receiver recv;
  recv.on_message("/foo/bar", [] (int f, int c) {
    std::cerr << "case 1: " << f << " " << c;
  });
  recv.on_message("/foo/bar", [] (float f, float c) {
    std::cerr << "case 2: " << f << " " << c;
  });
  recv.on_message("/foo/baa", [] (const oscour::message_view&) {
  });
  oscour::udp_server s(recv, 1234);

  oscour::tcp_server t([](auto f) {}, 5678);
  ::unlink("/tmp/foo");
  oscour::unix_server u([](auto f) {}, "/tmp/foo");
  while (true)
    ;
}

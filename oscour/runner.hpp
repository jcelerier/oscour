#pragma once
#include <thread>
#include <atomic>

namespace oscour
{
template<typename T>
struct sync_runner
{
  T& server;
  sync_runner(T& t): server{t} { }

  void run()
  {
    server.service().run();
  }
  void stop()
  {
    server.service().stop();
  }
};

template<typename T>
struct async_runner
{
  T& server;
  async_runner(T& t): server{t} { }

  ~async_runner()
  {
    stop();
  }

  void run()
  {
    if(m_running)
      return;

    m_running = true;
    m_thread = std::thread {
        [&] {
      try {
        server.service().run();
      } catch(...) {
      }
    }
    };
  }

  void stop()
  {
    m_running = false;
    server.service().stop();
    if(m_thread.joinable())
    {
      m_thread.join();
    }
  }

private:
  std::thread m_thread;
  std::atomic_bool m_running{};
};

}

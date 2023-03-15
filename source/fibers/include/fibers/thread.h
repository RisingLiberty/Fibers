#pragma once

#include <thread>
#include <string>

namespace fiber_test
{
  class Thread
  {
  public:
    // this creates the thread but doesn't start it yet
    Thread(std::string_view name, int index);

    Thread(const Thread&) = delete;
    Thread(Thread&& other);

    // the dtor of thread automatically joins it
    ~Thread();

    // starts executing the thread from the entry point
    template <typename Func>
    void start(Func func)
    {
      std::thread t(func);
      m_thread.swap(t);
    }
    
    std::string_view name() const;
    int index() const;

  private:
    std::string m_name;
    int m_index;
    std::thread m_thread;
  };
}
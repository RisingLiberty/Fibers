#pragma once

#include <vector>
#include <thread>

namespace fiber_test
{
  class ThreadPool
  {
  public:
    ThreadPool();

    int num_threads() const;
    
    template <typename Func>
    void start_thread(size_t threadIdx, Func entrypoint)
    {
      std::thread t(entrypoint);
      m_threads[threadIdx].swap(t);
    }

  private:
    std::vector<std::thread> m_threads;
  };
}
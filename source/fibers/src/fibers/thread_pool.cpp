#include "fibers/thread_pool.h"

namespace fiber_test
{
  ThreadPool::ThreadPool()
  {
    unsigned int max_threads = std::thread::hardware_concurrency();

    // we can only create max_threads - 1 as we already have the main thread
    for (int i = 0; i < max_threads - 1; ++i)
    {
      m_threads.emplace_back();
    }
  }

  int ThreadPool::num_threads() const
  {
    return m_threads.size();
  }
}
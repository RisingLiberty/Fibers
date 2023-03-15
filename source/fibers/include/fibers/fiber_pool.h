#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "fibers/fiber.h"

namespace fiber_test
{
  class FiberPool
  {
  public:
    FiberPool(int numFibers);

    Fiber* new_fiber();
    void return_fiber(Fiber* fiber);
    bool has_fibers() const;

  private:
    std::vector<Fiber> m_all_fibers; // this holds all the fibers
    std::vector<Fiber*> m_not_used_fibers; // this holds the fiber that can be used for jobs
    std::vector<Fiber*> m_used_fibers; // this holds the fibers currently running or put to sleep

    std::mutex m_fiber_access_mutex;
  };
}
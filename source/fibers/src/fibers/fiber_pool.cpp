#include "fibers/fiber_pool.h"
#include <iostream>
#include <Windows.h>

namespace fiber_test
{
  FiberPool::FiberPool(int numFibers)
  {
    for (int i = 0; i < numFibers; ++i)
    {
      std::string name = "Fiber ";
      name += std::to_string(i);
      m_all_fibers.emplace_back(name);
    }

    m_not_used_fibers.reserve(m_all_fibers.size());
    for (Fiber& fiber : m_all_fibers)
    {
      m_not_used_fibers.emplace_back(&fiber);
    }
  }

  Fiber* FiberPool::new_fiber()
  {
    // pop a new fiber from the pool
    std::unique_lock lock(m_fiber_access_mutex);

    Fiber* fiber = m_not_used_fibers.back();
    m_not_used_fibers.pop_back();

    // add this new fiber to the pool of currently in use fibers
    m_used_fibers.push_back(fiber);
    return fiber;
  }

  void FiberPool::return_fiber(Fiber* fiber)
  {
    std::unique_lock lock(m_fiber_access_mutex);
    auto it = std::find(m_used_fibers.cbegin(), m_used_fibers.cend(), fiber);
    m_used_fibers.erase(it);
    m_not_used_fibers.push_back(fiber);
  }

  bool FiberPool::has_fibers() const
  {
    return !m_not_used_fibers.empty();
  }
}
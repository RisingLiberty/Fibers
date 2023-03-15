#include "fibers/thread.h"

namespace fiber_test
{
  Thread::Thread(std::string_view name, int index)
    : m_name(name)
    , m_index(index)
    , m_thread()
  {

  }

  Thread::Thread(Thread&& other)
    : m_name(std::move(other.m_name))
    , m_index(other.m_index)
    , m_thread(std::move(other.m_thread))
  {

  }

  Thread::~Thread()
  {
    if (m_thread.joinable())
    {
      m_thread.join();
    }
  }

  std::string_view Thread::name() const
  {
    return m_name;
  }

  int Thread::index() const
  {
    return m_index;
  }
}
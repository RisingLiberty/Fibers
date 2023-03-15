#include "fibers/job_thread.h"
#include "fibers/job.h"
#include "fibers/fiber.h"
#include "diagnostics/assert.h"

#include <utility>
#include <Windows.h>

namespace fiber_test
{
  JobThread::JobThread(std::string_view name, int index, client_entry clientEntry)
    : Thread(name, index)
    , m_thread_fiber(nullptr)
    , m_job_fiber(nullptr)
    , m_client_entry(clientEntry)
  {
    // start running the thread
    start([this]() {job_thread_entry(); });
  }

  Job* JobThread::start_job_on_fiber(Job* job, Fiber* fiber, Fiber* parentFiber)
  {
    set_fiber(fiber);
    return m_job_fiber->resume(parentFiber, job);
  }

  Job* JobThread::start_job_on_current_fiber(Job* job, Fiber* parentFiber)
  {
    m_job_fiber->set_name(job->name());
    LOG(m_job_fiber->name());
    return m_job_fiber->resume(parentFiber, job);
  }

  Fiber* JobThread::thread_fiber()
  {
    return m_thread_fiber;
  }

  void JobThread::set_fiber(Fiber* fiber)
  {
    REX_ASSERT_X(!has_job_fiber() || m_job_fiber == fiber, "can't overwrite a job fiber, you need to release it first!");
    m_job_fiber = fiber;
  }

  Fiber* JobThread::release_fiber()
  {
    return std::exchange(m_job_fiber, nullptr);
  }

  bool JobThread::has_job_fiber() const
  {
    return m_job_fiber != nullptr;
  }

  JobThread JobThread::convert_this_thread_to_job_thread(std::string_view name, int index)
  {
    return JobThread(name, index);
  }

  JobThread::JobThread(std::string_view name, int index)
    : Thread(name, index)
    , m_thread_fiber(nullptr)
    , m_job_fiber(nullptr)
    , m_client_entry(nullptr)
  {
    std::string fiber_name = "Worker Thread Fiber ";
    fiber_name += std::to_string(index);

    // technically this will leak as it'll never get destroyed.
    // this is however allocated on the main thread
    m_thread_fiber = new Fiber(fiber_name, [](void* param) {return ConvertThreadToFiber(param); }, this);
  }

  JobThread::JobThread(JobThread&& other)
    : Thread(std::move(other))
    , m_thread_fiber(other.m_thread_fiber)
    , m_job_fiber(other.m_job_fiber)
    , m_client_entry(other.m_client_entry)
  {
    other.m_thread_fiber = nullptr;
    other.m_job_fiber = nullptr;
    other.m_client_entry = nullptr;
  }

  JobThread::~JobThread()
  {
    if (m_thread_fiber)
    {
      m_thread_fiber->convert_to_thread();
      delete m_thread_fiber;
    }
  }

  void JobThread::job_thread_entry()
  {
    // update visual studio thread name
    std::wstring thread_name(name().cbegin(), name().cend());
    SetThreadDescription(GetCurrentThread(), thread_name.c_str());

    // create the fiber
    std::string fiber_name = "Worker Thread Fiber ";
    fiber_name += std::to_string(index());
    Fiber fiber = Fiber(fiber_name, [](void* param) {return ConvertThreadToFiber(param); }, this);
    m_thread_fiber = &fiber;
    m_client_entry(this);
    m_thread_fiber = nullptr;
  }
}
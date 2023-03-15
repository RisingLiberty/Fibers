#include "fibers/fiber.h"
#include "fibers/job_thread.h"
#include "fibers/job_queue.h"
#include "fibers/job.h"

#include <Windows.h>

namespace fiber_test
{
  Fiber::Fiber(const std::string& name)
    : m_name(name)
    , m_fiber_data(std::make_unique<FiberData>())
    , m_client_entry(nullptr)
    , m_fiber(name, fiber_entry, m_fiber_data.get())
    , m_job_thread(nullptr)
  {
    m_fiber_data->this_fiber = this;
  }
  Fiber::Fiber(const std::string& name, void* (*fiberCreator)(void*), JobThread* jobThread)
    : m_name(name)
    , m_fiber_data(std::make_unique<FiberData>())
    , m_client_entry(nullptr)
    , m_fiber(fiberCreator(m_fiber_data.get()))
    , m_job_thread(jobThread)
  {
    m_fiber_data->this_fiber = this;
    m_fiber_data->thread = jobThread;
  }

  Fiber::Fiber(Fiber&& other)
    : m_name(std::move(other.m_name))
    , m_fiber_data(std::move(other.m_fiber_data))
    , m_client_entry(std::move(other.m_client_entry))
    , m_fiber(std::move(other.m_fiber))
    , m_job_thread(other.m_job_thread)
  {
    m_fiber_data->this_fiber = this; // make sure we reset the Fiber point in the FiberData
  }
  
  Fiber::~Fiber() = default;

  void Fiber::convert_to_thread()
  {
    m_fiber.reset();
    REX_ASSERT_X(ConvertFiberToThread() != 0, "Failed to convert fiber back to thread");
  }

  void Fiber::set_name(std::string_view name)
  {
    m_name = name;
  }

  std::string_view Fiber::name() const
  {
    return m_name;
  }

  Job* Fiber::resume(Fiber* parentFiber, Job* job)
  {
    FiberData* this_data = static_cast<FiberData*>(GetFiberData());

    REX_ASSERT_X(this_data == parentFiber->m_fiber_data.get(), "Invalid fiber data");

    m_fiber_data->job = job;
    m_fiber_data->parent_fiber = parentFiber;
    switch_to_me();

    return this_data->job;
  }

  void Fiber::resume_parent()
  {
    FiberData* this_data = static_cast<FiberData*>(GetFiberData());

    REX_ASSERT_X(this_data == m_fiber_data.get(), "Invalid fiber data");

    // transfer job back to the parent
    m_fiber_data->parent_fiber->m_fiber_data->job = this_data->job;
    m_fiber_data->parent_fiber->switch_to_me();
  }

  void Fiber::set_thread(JobThread* thread)
  {
    m_job_thread = thread;
    m_fiber_data->thread = thread;
  }

  void Fiber::set_parent_fiber(Fiber* parentFiber)
  {
    m_fiber_data->parent_fiber = parentFiber;
  }

  void Fiber::set_entry_point(FiberHandle::entry_point_func entryFunc)
  {
    m_client_entry = entryFunc;
  }

  void Fiber::set_param(void* param)
  {
    m_fiber_data->param = param;
  }

  JobThread* Fiber::thread()
  {
    return m_job_thread;
  }

  FiberHandle& Fiber::handle()
  {
    return m_fiber;
  }

  void Fiber::switch_to_me()
  {
    FiberData* this_data = static_cast<FiberData*>(GetFiberData());

    LOG("[" << GetCurrentThreadId() << "] leaving " << this_data->this_fiber->name());
    LOG("[" << GetCurrentThreadId() << "] at address " << this_data->this_fiber);

    // transfer job ownership back to the parent, this is also the point when the previous job gets destroyed
    SwitchToFiber(handle().get());

    LOG("[" << GetCurrentThreadId() << "] entered " << this_data->this_fiber->name());
    LOG("[" << GetCurrentThreadId() << "] at address " << this_data->this_fiber);
  }

  void __stdcall Fiber::fiber_entry(void* param)
  {
    FiberData* fiber_data = static_cast<FiberData*>(param);

    LOG("[" << GetCurrentThreadId() << "] entered " << fiber_data->this_fiber->name());
    LOG("[" << GetCurrentThreadId() << "] at address " << fiber_data->this_fiber);

    while (true)
    {
      LOG("About to run " << fiber_data->job->name());

      fiber_data->job->run(fiber_data);
      fiber_data->job->on_finish();

      // return back to thread's fiber that spawned this one
      fiber_data->this_fiber->resume_parent();
    }
  }
}
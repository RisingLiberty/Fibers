#pragma once

#include "diagnostics/assert.h"
#include "fibers/fiber_handle.h"

namespace fiber_test
{
  class Fiber;
  class JobThread;
  class Job;

  struct FiberData
  {
    void* param = nullptr; // user provided param
    Fiber* this_fiber = nullptr; // pointer pointing to the fiber that uses this FiberData
    Fiber* parent_fiber = nullptr; // pointer to parent fiber that spawned this one
    Job* job = nullptr;
    JobThread* thread = nullptr;
  };

  class Fiber
  {
  public:
    Fiber(const std::string& name); // constructed from scratch
    Fiber(const std::string& name, void* (*fiberCreator)(void*), JobThread* jobThread); // converted from a thread
    Fiber(const Fiber&) = delete;
    Fiber(Fiber&& other);
    ~Fiber();

    Fiber& operator=(const Fiber&) = delete;
    Fiber& operator=(Fiber&&) = delete;

    void convert_to_thread();

    void set_name(std::string_view name);

    std::string_view name() const;
    Job* resume(Fiber* parentFiber, Job* job);
    void resume_parent();

    void set_thread(JobThread* thread);
    void set_parent_fiber(Fiber* fiber);
    void set_entry_point(FiberHandle::entry_point_func entryFunc);
    void set_param(void* param);

    JobThread* thread();

    FiberHandle& handle();

  private:
    void switch_to_me();
    static void __stdcall fiber_entry(void* param);

  private:
    std::string m_name;
    std::unique_ptr<FiberData> m_fiber_data; // needs to be on the heap as this would become invalid when reallocating the fiber
    FiberHandle::entry_point_func m_client_entry;
    FiberHandle m_fiber;
    JobThread* m_job_thread;
  };
}
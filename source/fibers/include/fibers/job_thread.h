#pragma once

#include "fibers/thread.h"
#include "fibers/fiber.h"

#include <functional>

namespace fiber_test
{
  class Fiber;

  class JobThread : public Thread
  {
  public:
    using client_entry = std::function<void(JobThread*)>;

    JobThread(std::string_view name, int index, client_entry clientEntry);
    JobThread(const JobThread&) = delete;
    JobThread(JobThread&& other);
    ~JobThread();

    Job* start_job_on_fiber(Job* job, Fiber* fiber, Fiber* parentFiber);
    Job* start_job_on_current_fiber(Job* job, Fiber* parentFiber);

    Fiber* thread_fiber();

    void set_fiber(Fiber* fiber);

    Fiber* release_fiber();
    bool has_job_fiber() const;

    static JobThread convert_this_thread_to_job_thread(std::string_view name, int index);

  private:
    JobThread(std::string_view name, int index);

    void job_thread_entry();

  private:
    Fiber* m_job_fiber;
    Fiber* m_thread_fiber;
    client_entry m_client_entry;
  };
}
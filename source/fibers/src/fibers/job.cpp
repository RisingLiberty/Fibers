#include "fibers/job.h"

#include "fibers/fiber.h"
#include "fibers/job_queue.h"

namespace fiber_test
{
  //
  // Counter
  //

  Counter::Counter(int initVal)
    : m_value(initVal)
  {}

  int Counter::val() const
  {
    return m_value.load();
  }

  void Counter::dec()
  {
    m_value--;
  }

  //
  // JobDecl
  //

  JobDecl::JobDecl()
    : entry_point(nullptr)
    , param(nullptr)
    , job_name(nullptr)
  {}

  JobDecl::JobDecl(job_entry_func entryFunc, void* param, const char* name)
    : entry_point(entryFunc)
    , param(param)
    , job_name(name)
  {}

  //
  // Job
  //

  Job::Job()
    : m_entry_point(nullptr)
    , m_param(nullptr)
    , m_counter(nullptr)
    , m_fiber(nullptr)
    , m_name(nullptr)
  {}

  Job::Job(JobDecl decl, Counter& counter)
    : m_entry_point(decl.entry_point)
    , m_param(decl.param)
    , m_counter(&counter)
    , m_fiber(nullptr)
    , m_name(decl.job_name)
  {}

  void Job::run(FiberData* fiberData)
  {
    m_entry_point(m_param);
  }

  void Job::set_fiber(Fiber* fiber)
  {
    m_fiber = fiber;
  }

  void Job::on_finish()
  {
    m_counter->dec();
  }

  JobDecl::job_entry_func Job::entry_func() const
  {
    return m_entry_point;
  }

  void* Job::param() const
  {
    return m_param;
  }

  Fiber* Job::fiber()
  {
    return m_fiber;
  }

  const char* Job::name() const
  {
    return m_name;
  }

  void readd_job(Job job)
  {
    Counter counter;
    JobDecl job_decl;
    job_decl.entry_point = job.entry_func();
    job_decl.param = job.param();
    job_queue().add_jobs(&job_decl, 1, counter);
  }

  void wait_for_counter(Counter& counter, int expectedValue)
  {
    // the current fibers need to be put to sleep.
    // we push it to a wait list with it's wake up condition (counter == expectedValue)
    // after which we switch the fiber to the next job
    job_queue().sleep_current_job(counter, expectedValue);
  }

  void wait_for_counter_and_free(Counter& counter, int expectedValue)
  {
    // the current fibers need to be put to sleep.
    // we push it to a wait list with it's wake up condition (counter == expectedValue)
    // after which we switch the fiber to the next job
    job_queue().sleep_current_job(counter, expectedValue);
    job_queue().free_jobs(counter);
  }
}
#include "fibers/sleeping_job_fiber.h"
#include "fibers/fiber.h"
#include "fibers/job_thread.h"

namespace fiber_test
{
  SleepingJobFiber::SleepingJobFiber(Fiber* fiber, Job* job, Counter* counter, int expectedValue)
    : m_fiber(fiber)
    , m_job(job)
    , m_counter(counter)
    , m_expected_value(expectedValue)
  {}

  bool SleepingJobFiber::can_wakeup() const
  {
    return m_counter->val() == m_expected_value;
  }

  Fiber* SleepingJobFiber::fiber()
  {
    return m_fiber;
  }

  Job* SleepingJobFiber::job()
  {
    return m_job;
  }

  Job* SleepingJobFiber::wakeup(JobThread* thread)
  {
    m_fiber->set_thread(thread);
    m_fiber->set_parent_fiber(thread->thread_fiber());
    return m_fiber->resume(thread->thread_fiber(), m_job);
  }
}
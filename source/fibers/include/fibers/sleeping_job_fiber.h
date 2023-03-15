#pragma once

#include "fibers/job.h"

#include <memory>

namespace fiber_test
{
  class JobThread;

  class SleepingJobFiber
  {
  public:
    SleepingJobFiber(Fiber* fiber, Job* job, Counter* counter, int expectedValue);

    bool can_wakeup() const;
    Job* wakeup(JobThread* thread);

    Fiber* fiber();
    Job* job();

  private:
    Fiber* m_fiber;
    Job* m_job;
    Counter* m_counter;
    int m_expected_value;
  };
}
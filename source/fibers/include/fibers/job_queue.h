#pragma once

#include <vector>
#include <atomic>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <optional>

#include "fibers/thread_pool.h"
#include "fibers/fiber_pool.h"
#include "fibers/job.h"
#include "fibers/sleeping_job_fiber.h"
#include "fibers/job_thread.h"

namespace fiber_test
{
  class JobQueue
  {
  public:
    JobQueue();

    void start();
    void stop();
    void sleep_current_job(Counter& counter, int expectedValue);
    void wait_to_wakeup();

    Job* pull_job();
    void add_jobs(const JobDecl* decl, size_t count, Counter& counter);
    void free_jobs(Counter& counter);
    void readd_job();
    void wait_till_done();

  private:
    bool has_jobs();
    std::optional<SleepingJobFiber> try_getting_sleeping_job_to_wakeup();
    void job_thread_entry(JobThread* jobThread);

  private:
    FiberPool m_fiber_pool;

    std::vector<Job*> m_jobs_to_run; // all jobs that need to get executed
    std::vector<SleepingJobFiber> m_sleeping_jobs; // jobs that were put to sleep because their counter wasn't 0

    std::mutex m_job_access_mutex; // mutex to safely access our jobs
    std::mutex m_sleeping_job_access_mutex; // mutex to safely access our jobs

    std::vector<JobThread> m_job_threads;

    std::unordered_map<Counter*, std::vector<std::unique_ptr<Job>>> m_counter_to_jobs;

    bool m_process_jobs;
  };

  JobQueue& job_queue();

  template <size_t Size>
  void run_jobs(const std::array<JobDecl, Size>& jobs, Counter& counter)
  {
    // add the jobs to the job queue to be scheduled later
    // all jobs added will decrement the counter provided here on finish
    job_queue().add_jobs(jobs.data(), jobs.size(), counter);
  }

  void run_job(const JobDecl& decl, Counter& counter);
}

#define JOB_ENTRY_POINT(x) void x(void* param)

#include "fibers/job_queue.h"

#include <algorithm>
#include <Windows.h>

namespace fiber_test
{
  JobQueue::JobQueue()
    : m_fiber_pool(150)
    , m_jobs_to_run()
    , m_sleeping_jobs()
    , m_process_jobs(true)
  {}

  void JobQueue::start()
  {
    const int num_job_threads = std::thread::hardware_concurrency() - 1;
    m_job_threads.reserve(num_job_threads);
    std::string name;
    for (int i = 0; i < num_job_threads; ++i)
    {
      name = "Worker Thread ";
      name += std::to_string(i);
      m_job_threads.emplace_back(name, i, [this](JobThread* thread) { job_thread_entry(thread); });

    }

    name = "Worker Thread ";
    name += std::to_string(m_job_threads.size());

    JobThread job_thread = JobThread::convert_this_thread_to_job_thread(name, m_job_threads.size());
    job_thread_entry(&job_thread);
  }

  void JobQueue::stop()
  {
    m_process_jobs = false;
  }

  void JobQueue::sleep_current_job(Counter& counter, int expectedValue)
  {
    // this code is always run from a fiber that's executing a job
    // therefore we can safely get the fiber data and cast it to the FiberData struct
    // this holds all the information we need to properly put the current fiber to sleep
    // and go back to the parent fiber to continue executing other jobs

    FiberData* fiber_data = static_cast<FiberData*>(GetFiberData());

    // first, add the fiber to the waitlist
    Fiber* fiber = fiber_data->this_fiber;
    Job* job = fiber_data->job;

    LOG("Sleeping " << job->name() << "\nOn fiber " << fiber->name());

    Fiber* sleeping_fiber = nullptr;
    {
      std::unique_lock lock(m_sleeping_job_access_mutex);
      m_sleeping_jobs.emplace_back(fiber, job, &counter, expectedValue); // this moves ownership of the job to the SleepingJobFiber
      // second release fiber from the current thread
      sleeping_fiber = fiber_data->thread->release_fiber();
      REX_ASSERT_X(sleeping_fiber != nullptr, "fiber to put to sleep is nullptr");
    }

    sleeping_fiber->resume_parent();

    // third, wait for it to wake up again
    //wait_to_wakeup();

    // if the code comes back here, it means that the counter reached 0
    // and we picked up where we left so we can continue executing the rest of this task
  }

  void JobQueue::wait_to_wakeup()
  {
    // resume the parent fiber so it can continue processing other jobs
    FiberData* fiber_data = static_cast<FiberData*>(GetFiberData());
    Fiber* fiber = fiber_data->this_fiber;
    fiber->resume_parent();
  }

  void JobQueue::add_jobs(const JobDecl* decl, size_t count, Counter& counter)
  {
    std::unique_lock lock(m_job_access_mutex);

    auto it = m_counter_to_jobs.find(&counter);
    if (it == m_counter_to_jobs.end())
    {
      std::vector<std::unique_ptr<Job>>& jobs = m_counter_to_jobs[&counter];
      jobs.resize(count);
      for (int i = 0; i < count; ++i)
      {
        jobs[i] = std::make_unique<Job>(decl[i], counter);
      }
    }

    std::vector<std::unique_ptr<Job>>& jobs = m_counter_to_jobs[&counter];
    const int num_current_jobs = m_jobs_to_run.size();
    m_jobs_to_run.resize(num_current_jobs + count);
    for (int i = 0; i < count; ++i)
    {
      int jobs_to_run_idx = i + num_current_jobs;
      m_jobs_to_run[jobs_to_run_idx] = jobs[i].get();
    }
  }

  void JobQueue::free_jobs(Counter& counter)
  {
    std::unique_lock lock(m_job_access_mutex);

    auto it = m_counter_to_jobs.find(&counter);
    m_counter_to_jobs.erase(it);
  }

  void JobQueue::readd_job()
  {
    std::unique_lock lock(m_job_access_mutex);

    FiberData* fiber_data = static_cast<FiberData*>(GetFiberData());
    m_jobs_to_run.push_back(fiber_data->job);
  }

  void JobQueue::wait_till_done()
  {
    while (m_process_jobs)
    {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1ms);
    }
  }

  Job* JobQueue::pull_job()
  {
    std::unique_lock lock(m_job_access_mutex);

    if (m_jobs_to_run.empty())
    {
      return nullptr;
    }

    Job* job = m_jobs_to_run.front();
    m_jobs_to_run.erase(m_jobs_to_run.cbegin());

    return job;
  }

  bool JobQueue::has_jobs()
  {
    std::unique_lock lock(m_job_access_mutex);
    return !m_counter_to_jobs.empty();
  }

  std::optional<SleepingJobFiber> JobQueue::try_getting_sleeping_job_to_wakeup()
  {
    std::unique_lock lock(m_sleeping_job_access_mutex);
    
    auto first = m_sleeping_jobs.begin();
    auto end = m_sleeping_jobs.end();
    for (auto it = first; it != end; ++it)
    {
      if (it->can_wakeup())
      {
        SleepingJobFiber sleeping_job_fiber = *it;
        m_sleeping_jobs.erase(it);
        return sleeping_job_fiber;
      }
    }

    return std::nullopt;
  }

  void JobQueue::job_thread_entry(JobThread* jobThread)
  {
    // this is the mainloop, should only stop on exit when the job system should shut down
    // the mainloop does what's described in fiber_test.cpp run function
    // 1) pull a new fiber
    //   - checking if any waiting fiber can be woken up
    // 2) run job on fiber
    // 3) if you need to wait, sleep fiber
    // 4) go back to step 1
    while (m_process_jobs)
    {
      // Step 1: pull fiber into working thread
      
      // Step 1.1: check if we can run a sleeping fiber
      std::optional<SleepingJobFiber> sleeping_job_fiber = try_getting_sleeping_job_to_wakeup();
      if (sleeping_job_fiber.has_value())
      {
        // if the thread still has a fiber
        // release it as we need to pick up the one of the sleeping job
        if (jobThread->has_job_fiber())
        {
          Fiber* fiber_to_release = jobThread->release_fiber();
          m_fiber_pool.return_fiber(fiber_to_release);
        }

        LOG("waking up sleeping job " << sleeping_job_fiber->job()->name());

        // restart the sleeping fiber
        jobThread->set_fiber(sleeping_job_fiber->fiber());
        sleeping_job_fiber->wakeup(jobThread);
      }
      else if (Job* job = pull_job())
      {
        LOG("pulling new job " << job->name());

        if (jobThread->has_job_fiber() == false)
        {
          LOG("running on new fiber");
          Fiber* fiber = m_fiber_pool.new_fiber();
          fiber->set_name(job->name());
          fiber->set_thread(jobThread);
          jobThread->start_job_on_fiber(job, fiber, jobThread->thread_fiber());
        }
        else
        {
          LOG("running on existing fiber ");
          jobThread->start_job_on_current_fiber(job, jobThread->thread_fiber());
        }
      }
      else
      {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
      }
    }
  }

  JobQueue& job_queue()
  {
    static JobQueue job_queue;
    return job_queue;
  }

  void run_job(const JobDecl& decl, Counter& counter)
  {
    job_queue().add_jobs(&decl, 1, counter);
  }
}
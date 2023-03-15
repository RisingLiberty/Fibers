#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

#include "fibers/fiber.h"
#include "fibers/fiber_pool.h"
#include "fibers/job_queue.h"

namespace fiber_test
{
  class Object
  {
  public:
    Object()
      : m_x(s_id++)
    {}

  private:
    static int s_id;
    int m_x;
  };
  int Object::s_id = 0;

  std::array<Object, 10> g_objects;

  JOB_ENTRY_POINT(animate_object)
  {
    Object* p = static_cast<Object*>(param);

    static std::mutex cout_mutex;
    std::unique_lock lock(cout_mutex);

    static std::atomic<int> x = 0;
    LOG2("Animating object " << x.load());
    x++;
  }

  JOB_ENTRY_POINT(animate_all_objects)
  {
    LOG("Animating all objects");

    std::array<JobDecl, g_objects.size()> jobs;

    for (int i = 0; i < jobs.size(); ++i)
    {
      jobs[i] = JobDecl(animate_object, &g_objects[i], "animate_object");
    }

    Counter job_counter(jobs.size());
    run_jobs(jobs, job_counter);
    wait_for_counter_and_free(job_counter, 0);

    LOG("animated all objects!");
  }

  JOB_ENTRY_POINT(start_jobs)
  {
    int* tries = static_cast<int*>(param);

    while (*tries > 0)
    {
      LOG("running start jobs");

      JobDecl job(animate_all_objects, nullptr, "animate_all_objects");
      Counter job_counter(1);
      run_job(job, job_counter);
      wait_for_counter(job_counter, 0);
      --* tries;
    }

    job_queue().stop();
  }
}

int main(int argc, char* argv[])
{
  // https://youtu.be/HIVBhKj7gQU?t=634
  // 1) pull new fiber into the worker thread        <-------+
  //  - checks all waiting fibers if any can continue        |
  // 2) job can always add new jobs, added to the end        |
  // 3) if you need to wait -> move fiber to a waitlist      |
  // 4) go back to step 1 -----------------------------------+

  // test job to run
  // in an actual engine, pass in a pointer to something that can verify
  // if the game should stop or not

  int x = 10;
  fiber_test::JobDecl job(fiber_test::start_jobs, &x, "start_jobs");
  fiber_test::Counter job_counter(10);
  fiber_test::run_job(job, job_counter);

  // start the job systems, fomr this point on, we don't have a main thread anymore
  // and everything will be handled by the job system.
  fiber_test::job_queue().start();

  while (x > 0)
  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
  }
  fiber_test::job_queue().wait_till_done();

  // this should only get reached if the initial job has finished processing
  std::cout << "done!\n";

  std::cin.get();

  return 0;
}

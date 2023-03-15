#pragma once

#include <atomic>

namespace fiber_test
{
  class Fiber;

  class Counter
  {
  public:
    Counter(int initVal = 0);

    int val() const;
    void dec();

  private:
    std::atomic<int> m_value;
  };

  struct JobDecl
  {
  public:
    using job_entry_func = void (*)(void*);

    JobDecl();
    JobDecl(job_entry_func entryFunc, void* param, const char* name);

  public:
    job_entry_func entry_point;
    void* param;
    const char* job_name;
  };

  class FiberData;

  class Job
  {
  public:
    Job();
    Job(JobDecl decl, Counter& counter);

    void run(FiberData* fiberData);
    void set_fiber(Fiber* fiber);
    void on_finish();

    JobDecl::job_entry_func entry_func() const;
    void* param() const;
    Fiber* fiber();
    const char* name() const;

  private:
    JobDecl::job_entry_func m_entry_point;
    void* m_param;
    Counter* m_counter;
    Fiber* m_fiber;
    const char* m_name;
  };

  void wait_for_counter(Counter& counter, int expectedValue);
  void wait_for_counter_and_free(Counter& counter, int expectedValue);
}
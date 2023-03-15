#pragma once

#include <iostream>
#include <mutex>

#define REX_ASSERT(msg) std::cout << "Assert: " << msg << "\n"; __debugbreak();
#define REX_ASSERT_X(cond, msg)                                                                                                                                                                                                                       \
  [&]()                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                      \
    if(!(cond))                                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                    \
      REX_ASSERT(msg);                                                                                                                                                                                                                        \
      return true;                                                                                                                                                                                                                                       \
    }                                                                                                                                                                                                                                                    \
    return false;                                                                                                                                                                                                                                        \
  }()

extern std::mutex log_mutex;
#define LOG(msg) //{ std::unique_lock lock(log_mutex); std::cout << msg << "\n"; }
#define LOG2(msg) { std::unique_lock lock(log_mutex); std::cout << msg << "\n"; }

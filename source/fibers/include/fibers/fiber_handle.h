#pragma once

#include <string_view>

namespace fiber_test
{
  class FiberHandle
  {
  public:
    using entry_point_func = void(__stdcall*)(void*);

    FiberHandle(void* handle = nullptr);
    FiberHandle(std::string_view name, entry_point_func entryPoint, void* param);
    FiberHandle(const FiberHandle&) = delete;
    FiberHandle(FiberHandle&& other);
    ~FiberHandle();

    FiberHandle& operator=(const FiberHandle&) = delete;
    FiberHandle& operator=(FiberHandle&&) = delete;

    void reset();
    void* get();

  private:
    void* m_handle;

    static constexpr void* InvalidFiberValue = reinterpret_cast<void*>(0x0000000000001e00); // on Windows x64, returned by GetCurrentFiber
  };
}
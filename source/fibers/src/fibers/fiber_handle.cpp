#include "fibers/fiber_handle.h"
#include "diagnostics/assert.h"

#include <Windows.h>

namespace fiber_test
{
  FiberHandle::FiberHandle(void* handle)
    : m_handle(handle)
  {
    REX_ASSERT_X(m_handle != nullptr, "Failed to create fiber handle");
  }

  FiberHandle::FiberHandle(std::string_view name, entry_point_func entryPoint, void* param)
    : m_handle(nullptr)
  {
    m_handle = CreateFiber(0, entryPoint, param);
    REX_ASSERT_X(m_handle != nullptr, "Failed to create fiber " << name);
  }
  FiberHandle::FiberHandle(FiberHandle&& other)
    : m_handle(other.m_handle)
  {
    other.m_handle = nullptr;
  }
  FiberHandle::~FiberHandle()
  {
    if (m_handle)
    {
      DeleteFiber(m_handle);
    }
  }

  void FiberHandle::reset()
  {
    m_handle = nullptr;
  }

  void* FiberHandle::get()
  {
    return m_handle;
  }

}
#ifdef __clang__
  #include <clib/exec_protos.h>
#else
  #include <proto/exec.h>
#endif

#include <exec/memory.h>

#include "MemoryPool.h"

MemoryPool::MemoryPool()
  : m_pPoolHeader(CreatePool(MEMF_PUBLIC|MEMF_CLEAR, 50000, 25000))
{
  if(m_pPoolHeader == NULL)
  {
    throw "Failed to create the memory pool.";
  }
}

MemoryPool::~MemoryPool()
{
  if(m_pPoolHeader != NULL)
  {
    DeletePool(m_pPoolHeader);
    m_pPoolHeader = NULL;
  }
}

APTR MemoryPool::Header()
{
  return m_pPoolHeader;
}

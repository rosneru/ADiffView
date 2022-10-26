#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <exec/types.h>

/**
 * Simple abstraction of an AmigaOS3.0+ memory pool. Just done to be
 * able to use memory pools in the RAII way.
 *
 * @author Uwe Rosner
 * @date 22/07/2020
 */
class MemoryPool
{
public:
  MemoryPool();
  virtual ~MemoryPool();

  APTR Header();
private:
  APTR m_pPoolHeader;
};

#endif

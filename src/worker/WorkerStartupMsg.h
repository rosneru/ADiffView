#ifndef WORKER_STARTUP_MSG
#define WORKER_STARTUP_MSG

#include <exec/ports.h>

class WorkerBase;

struct WorkerStartupMsg : public Message 
{
  class WorkerBase* that;
};

#endif

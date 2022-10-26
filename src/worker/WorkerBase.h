#ifndef WORKER_BASE_H
#define WORKER_BASE_H

#include <dos/dosextens.h>
#include <exec/ports.h>

#include "WorkerStartupMsg.h"

/**
 * This is an abstract class to provide background worker functionality
 * e.g. for time-consuming operations.
 *
 * @author Uwe Rosner
 * @date 24/01/2019
 */
class WorkerBase
{
public:

  WorkerBase();
  virtual ~WorkerBase();

  /**
   * Creates and starts the background process.
   */
  bool Run();

protected:
  WorkerStartupMsg* m_pStartupMsg;
  struct MsgPort* m_pReplyPort;

  /**
   * Method to perform the time consuming background work.
   * Must be implemented by inheriting classes.
   */
  virtual void doWork() = 0;

private:
  struct Process* m_pBackgrProcess;
  bool m_IsExitRequested;

  static void startup();
};

#endif

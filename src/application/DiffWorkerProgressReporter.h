#ifndef DIFF_WORKER_PROGRESS_REPORTER_H
#define DIFF_WORKER_PROGRESS_REPORTER_H

#include "ProgressReporter.h"

/**
 * Overrides the default ProgressReporter for DiffWorker
 *
 * @author Uwe Rosner
 * @date 22/07/2020
 */
class DiffWorkerProgressReporter : public ProgressReporter
{
public:
  DiffWorkerProgressReporter(struct MsgPort* pProgressPort, 
                             struct MsgPort*& pReplyPort);

  virtual void SetValue(int progress);

  /**
   * Reset the internal offset. Should be done before starting the
   * progress again.
   */
  void Reset();

private:
  int m_ProgressOffset;
};

#endif

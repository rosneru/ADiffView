#ifndef STOP_WATCH_H
#define STOP_WATCH_H

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>
#include <devices/timer.h>
#include <exec/ports.h>
#include <exec/io.h>
/**
 * Class for an easy to use stop watch using the timer.device
 *
 * @author Uwe Rosner
 * @date 21/04/2018
 */
class StopWatch
{
public:
  /**
   * Creates the stopwatch, initializes the needed MessagePort and
   * opens the timer.device
   */
  StopWatch();

  /**
   * Destroys the the stopwatch and disposed all opened resources
   */
  virtual ~StopWatch();

  /**
   * Starts or re-starts the stop watch.
   */
  void Start();

  /**
   * Picks the elapsed time in milli seconds.
   *
   * @param bKeepStartPoint When true, the stop watch is not reset when
   * picking. That means that the former start time will be used at next
   * pick again.
   *
   * @returns
   * Elapsed time in milliseconds or -1 if an error occurred.
   */
  long Pick(bool bKeepStartPoint = false);

private:
  struct MsgPort* m_pMsgPort;
  struct timerequest* m_pTimeRequest;

  ULONG m_ClocksPerSecond;
  struct EClockVal m_StartClock;
  struct EClockVal m_StopClock;
};

#endif

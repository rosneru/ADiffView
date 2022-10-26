#ifdef __clang__
  #include <clib/exec_protos.h>
#else
  #include <proto/exec.h>
#endif


#include "StopWatch.h"

// Base address of timer device; has to be global
struct TimerBase* TimerBase = NULL;

StopWatch::StopWatch()
  : m_pMsgPort(CreateMsgPort()),
    m_pTimeRequest((struct timerequest*) CreateIORequest(m_pMsgPort, 
                                                         sizeof(struct timerequest))),
    m_ClocksPerSecond(0),
    m_StartClock(),
    m_StopClock()                                                         
{
  if(m_pMsgPort == NULL)
  {
    throw "Failed to create MessagePort for StopWatch.";
  }

  if(m_pTimeRequest == NULL)
  {
    DeleteMsgPort(m_pMsgPort);
    m_pMsgPort = NULL;

    throw "Failed to create time request for StopWatch.";
  }

  // Open the timer.device
  BYTE res = OpenDevice(TIMERNAME,
                        UNIT_ECLOCK,
                        (struct IORequest*) m_pTimeRequest,
                        TR_GETSYSTIME);
  if(res != NULL)
  {
    DeleteIORequest(m_pTimeRequest);
    m_pTimeRequest = NULL;
    DeleteMsgPort(m_pMsgPort);
    m_pMsgPort = NULL;

    throw "Failed to open the timer.device for StopWatch.";
  }

  m_pTimeRequest->tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

  // Setting the timer base
  TimerBase = (struct TimerBase*)m_pTimeRequest->tr_node.io_Device;
}

StopWatch::~StopWatch()
{
  if(TimerBase != NULL)
  {
    CloseDevice((struct IORequest*)m_pTimeRequest);
    TimerBase = NULL;
  }

  if(m_pTimeRequest != NULL)
  {
    DeleteIORequest(m_pTimeRequest);
    m_pTimeRequest = NULL;
  }

  if(m_pMsgPort != NULL)
  {
    DeleteMsgPort(m_pMsgPort);
    m_pMsgPort = NULL;
  }
}


void StopWatch::Start()
{
  // Getting and storing current eclock value
  m_ClocksPerSecond = ReadEClock(&m_StartClock);
}


long StopWatch::Pick(bool bKeepStartPoint)
{
  if(m_ClocksPerSecond == 0)
  {
    // Not started yet
    return -1;
  }

  // Read the eclock value again
  ReadEClock(&m_StopClock);

  // Calculate the elapsed time in milliseconds
  long long millisecs = m_StopClock.ev_lo - m_StartClock.ev_lo;
  millisecs *= 1000;
  millisecs /= m_ClocksPerSecond;

  if(bKeepStartPoint == false)
  {
    ReadEClock(&m_StartClock);
  }

  return static_cast<LONG>(millisecs);
}

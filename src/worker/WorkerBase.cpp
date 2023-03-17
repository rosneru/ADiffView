#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
#endif

#include <dos/dosextens.h>
#include <dos/dostags.h>

#include "WorkerBase.h"
#include "ProgressMessage.h"

WorkerBase::WorkerBase()
  : m_pStartupMsg(NULL),
    m_pReplyPort(NULL),
    m_pBackgrProcess(NULL),
    m_IsExitRequested(false)
{
}


WorkerBase::~WorkerBase()
{
  if (m_pStartupMsg != NULL)
  {
    FreeVec(m_pStartupMsg);
    m_pStartupMsg = NULL;
  }
}


bool WorkerBase::run()
{
  if (m_pStartupMsg != NULL)
  {
    FreeVec(m_pStartupMsg);
    m_pStartupMsg = NULL;
  }

  // Create the background process
  m_pBackgrProcess = CreateNewProcTags(NP_Name, (ULONG)"WorkerBase",
                                       NP_Entry, (ULONG)&startup,
                                       NP_StackSize, 30000,
                                       TAG_DONE);

  if (m_pBackgrProcess == NULL)
  {
    // Failed to create process
    return false;
  }

  m_pStartupMsg = (struct WorkerStartupMsg*)
    AllocVec(sizeof(struct WorkerStartupMsg), MEMF_CLEAR|MEMF_PUBLIC);

  if (m_pStartupMsg == NULL)
  {
    // Failed to alloc memory for process startup msg
    return false;
  }

  m_pStartupMsg->that = this;
  PutMsg(&m_pBackgrProcess->pr_MsgPort, m_pStartupMsg);

  return true;
}


void WorkerBase::startup()
{
  struct Process* pProcess = (struct Process*) FindTask(NULL);
  if(pProcess == NULL)
  {
    // Error in process startup: Can't find own task
    return;
  }

  // Wait for start signal
  WaitPort(&pProcess->pr_MsgPort);

  struct WorkerStartupMsg* pStartupMsg = (struct WorkerStartupMsg*)
    GetMsg(&pProcess->pr_MsgPort);

  if ((pStartupMsg == NULL) || (pStartupMsg->that == NULL))
  {
    // Need pStartupMsg and ist 'that' pointer
    return;
  }

  // Create the reply port for this process. It is used to receive
  // answers from main for the sent progress messages.
  if ((pStartupMsg->that->m_pReplyPort = CreateMsgPort()) == NULL)
  {
    // Failed to create message port
    return;
  }

  pStartupMsg->that->doWork();

  DeleteMsgPort(pStartupMsg->that->m_pReplyPort);
  pStartupMsg->that->m_pReplyPort = NULL;
}

#ifdef __clang__
  #include <clib/exec_protos.h>
#else
  #include <proto/exec.h>
#endif

#include "ADiffViewPorts.h"


ADiffViewPorts::ADiffViewPorts()
  : m_pMsgPortIDCMP(NULL),
    m_pMsgPortProgress(NULL),
    m_pMsgPortWorkbench(NULL)
{
    // Create a message port for all windows (IDCMP) messages
  m_pMsgPortIDCMP = CreateMsgPort();
  if(m_pMsgPortIDCMP == NULL)
  {
    throw "Failed to create the windows message port.";
  }

  // Create a message port for progress notification
  m_pMsgPortProgress = CreateMsgPort();
  if(m_pMsgPortProgress == NULL)
  {
    DeleteMsgPort(m_pMsgPortIDCMP);
    throw "Failed to create the progress message port.";
  }

    // Create a message port for the Workbench app messages
    m_pMsgPortWorkbench = CreateMsgPort();
    if(m_pMsgPortWorkbench == NULL)
    {
      DeleteMsgPort(m_pMsgPortProgress);
      DeleteMsgPort(m_pMsgPortIDCMP);
      throw "Failed to create the app message port.";
    }
}


ADiffViewPorts::~ADiffViewPorts()
{
  if(m_pMsgPortWorkbench != NULL)
  {
    DeleteMsgPort(m_pMsgPortWorkbench);
    m_pMsgPortWorkbench = NULL;
  }

  if(m_pMsgPortProgress != NULL)
  {
    DeleteMsgPort(m_pMsgPortProgress);
    m_pMsgPortProgress = NULL;
  }

  if(m_pMsgPortIDCMP != NULL)
  {
    DeleteMsgPort(m_pMsgPortIDCMP);
    m_pMsgPortIDCMP = NULL;
  }
}


struct MsgPort* ADiffViewPorts::Idcmp() const
{
  return m_pMsgPortIDCMP;
}

struct MsgPort* ADiffViewPorts::Progress() const
{
  return m_pMsgPortProgress;
}

struct MsgPort* ADiffViewPorts::Workbench() const
{
  return m_pMsgPortWorkbench;
}

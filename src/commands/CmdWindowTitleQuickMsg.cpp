#ifdef __clang__
  #include <clib/exec_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/exec.h>
  #include <proto/intuition.h>
#endif

#include <intuition/intuition.h>

#include "CmdWindowTitleQuickMsg.h"


CmdWindowTitleQuickMsg::CmdWindowTitleQuickMsg(std::vector<WindowBase*>* pAllWindowsVector)
  : CommandBase(pAllWindowsVector),
    m_Message("Medium is the message")
{
}

CmdWindowTitleQuickMsg::~CmdWindowTitleQuickMsg()
{

}

void CmdWindowTitleQuickMsg::setTitleMessage(const std::string message)
{
  m_Message = message;
}

void CmdWindowTitleQuickMsg::Execute(struct Window* pActiveWindow)
{
  if(m_OriginalWindowTitle.length() < 1)
  {
    m_OriginalWindowTitle = (char*)pActiveWindow->Title;
  }

  if(m_Message.length() < 1)
  {
    m_ConcatenatedWindowTitle = m_OriginalWindowTitle;
  }
  else
  {
    m_ConcatenatedWindowTitle = m_OriginalWindowTitle + " - " + m_Message;
  }

  SetWindowTitles(pActiveWindow, m_ConcatenatedWindowTitle.c_str(), (STRPTR) ~0);
}

#include "CmdQuit.h"


CmdQuit::CmdQuit(std::vector<WindowBase*>* pAllWindowsVector,
                 bool& exitAllowed, 
                 bool& exitRequested)
  : CommandBase(pAllWindowsVector),
    m_IsExitAllowed(exitAllowed),
    m_IsExitRequested(exitRequested)
{
}

CmdQuit::~CmdQuit()
{

}

void CmdQuit::Execute(struct Window* pActiveWindow)
{
  if(m_IsExitAllowed)
  {
    m_IsExitRequested = true;
  }
}

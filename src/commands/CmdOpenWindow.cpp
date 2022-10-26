#include "CmdOpenWindow.h"

CmdOpenWindow::CmdOpenWindow(std::vector<WindowBase*>* pAllWindowsVector,
                             WindowBase& window)
  : CommandBase(pAllWindowsVector),
    m_Window(window)
{

}

CmdOpenWindow::~CmdOpenWindow()
{
}

void CmdOpenWindow::Execute(struct Window* pActiveWindow) 
{
  if(m_Window.isOpen())
  {
    return;
  }

  
  if(m_Window.open() == true)
  {
    DisableInAllWindowMenus();
  }
}

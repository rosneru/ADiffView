#include "CmdChangeTabulatorWidth.h"

CmdChangeTabulatorWidth::CmdChangeTabulatorWidth(std::vector<WindowBase*>* pAllWindowsVector,
                                                 size_t tabulatorWidth,
                                                 DiffWindow& diffWindow,
                                                 DiffWorker& diffWorker)
  : CommandBase(pAllWindowsVector),
    m_TabulatorWidth(tabulatorWidth),
    m_DiffWindow(diffWindow),
    m_DiffWorker(diffWorker)
{

}


CmdChangeTabulatorWidth::~CmdChangeTabulatorWidth()
{
}

void CmdChangeTabulatorWidth::Execute(struct Window* pActiveWindow) 
{
  DiffDocument* pDocument = m_DiffWorker.getDiffDocument();
  if(pDocument == NULL)
  {
    return;
  }
  
  pDocument->setTabulatorSize(m_TabulatorWidth);
  m_DiffWindow.setTabulatorSize(m_TabulatorWidth);
}

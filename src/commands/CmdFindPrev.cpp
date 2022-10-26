#include "CmdFindPrev.h"

CmdFindPrev::CmdFindPrev(std::vector<WindowBase*>* pAllWindowsVector,
                         TextFinder& textFinder)
  : CommandBase(pAllWindowsVector),
    m_TextFinder(textFinder)
{

}

CmdFindPrev::~CmdFindPrev()
{

}

void CmdFindPrev::Execute(struct Window* pActiveWindow)
{
  m_TextFinder.displayPrevResult();
}

#include "CmdFindNext.h"

CmdFindNext::CmdFindNext(std::vector<WindowBase*>* pAllWindowsVector,
                         TextFinder& textFinder)
  : CommandBase(pAllWindowsVector),
    m_TextFinder(textFinder)
{

}

CmdFindNext::~CmdFindNext()
{

}

void CmdFindNext::Execute(struct Window* pActiveWindow)
{
  m_TextFinder.displayNextResult();
}

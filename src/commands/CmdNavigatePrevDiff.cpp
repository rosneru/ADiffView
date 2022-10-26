#include "CmdNavigatePrevDiff.h"

CmdNavigatePrevDiff::CmdNavigatePrevDiff(std::vector<WindowBase*>* pAllWindowsVector,
                                         DiffWorker& diffWorker,
                                         DiffWindow& diffWindow,
                                         CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg)
  : CmdNavigateBase(pAllWindowsVector, diffWorker, diffWindow, cmdWindowTitleQuickMsg)
{

}

CmdNavigatePrevDiff::~CmdNavigatePrevDiff()
{

}

void CmdNavigatePrevDiff::Execute(struct Window* pActiveWindow)
{
  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    return;
  }

  // Get lineId of next diff block
  long lineId = -1;
  bool hasScrolled = false;
  do
  {
    if(m_FormerLineId < 0)
    {
      lineId = getPrevResult(pLeftTextArea->getY() 
                            + pLeftTextArea->getMaxVisibleLines());
    }
    else if(pLeftTextArea->isLineVisible(m_FormerLineId))
    {
      // The former result is currently displayed. So getting the 
      // next result after that former result, not from the window 
      // top line.
      lineId = getPrevResult();
    }
    else
    {
      lineId = getPrevResult(pLeftTextArea->getY());
    }

    if(lineId < 0)
    {
      signalNoResultFound("No previous difference");
      return;
    }

    m_FormerLineId = lineId;
    hasScrolled = m_DiffWindow.scrollToPage(0, lineId, 1, 1);
  } 
  while (hasScrolled == false); // Repeatedly getting the next diff block
                                // until the view scrolled once.

  // Clear the window title message that may have been displayed previously
  m_CmdWindowTitleQuickMsg.setTitleMessage("");
  m_CmdWindowTitleQuickMsg.Execute(m_DiffWindow.getIntuiWindow());
}

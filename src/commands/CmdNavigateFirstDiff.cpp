#include "CmdNavigateFirstDiff.h"

CmdNavigateFirstDiff::CmdNavigateFirstDiff(std::vector<WindowBase*>* pAllWindowsVector,
                                           DiffWorker& diffWorker,
                                           DiffWindow& diffWindow,
                                           CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg)
  : CmdNavigateBase(pAllWindowsVector, diffWorker, diffWindow, cmdWindowTitleQuickMsg)
{

}

CmdNavigateFirstDiff::~CmdNavigateFirstDiff()
{

}

void CmdNavigateFirstDiff::Execute(struct Window* pActiveWindow)
{
  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    return;
  }

  // Get lineId of first diff block (The next one starting from line 0).
  long lineId = getNextResult(0);
  if(lineId < 0)
  {
    // Should not be possible, a first diff block *should* always exist
    signalNoResultFound("No first difference");
    return;
  }

  m_FormerLineId = lineId;
  m_DiffWindow.scrollToPage(0, lineId, 1, 1);

  // Clear the window title message that may have been displayed previously
  m_CmdWindowTitleQuickMsg.setTitleMessage("");
  m_CmdWindowTitleQuickMsg.Execute(m_DiffWindow.getIntuiWindow());
}

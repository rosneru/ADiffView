#include "CmdNavigateLastDiff.h"

CmdNavigateLastDiff::CmdNavigateLastDiff(std::vector<WindowBase*>* pAllWindowsVector,
                                         DiffWorker& diffWorker,
                                         DiffWindow& diffWindow,
                                         CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg)
  : CmdNavigateBase(pAllWindowsVector, diffWorker, diffWindow, cmdWindowTitleQuickMsg)
{

}

CmdNavigateLastDiff::~CmdNavigateLastDiff()
{

}

void CmdNavigateLastDiff::Execute(struct Window* pActiveWindow)
{
  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    return;
  }

  // As we are about to use the DiffDocument pointer below we first have
  // to get the current one and check if it maybe (still) is NULL.
  applyDocumentChanged();
  if(m_pDiffDocument == NULL)
  {
    return;
  }

  // Get lineId of last diff block (The one before the document end).
  long lineId = getPrevResult(m_pDiffDocument->getNumLines() - 1);
  if(lineId < 0)
  {
    // Should not be possible, a first diff block *should* always exist
    signalNoResultFound("No last difference");
    return;
  }

  m_FormerLineId = lineId;
  m_DiffWindow.scrollToPage(0, lineId, 1, 1);

  // Clear the window title message that may have been displayed previously
  m_CmdWindowTitleQuickMsg.setTitleMessage("");
  m_CmdWindowTitleQuickMsg.Execute(m_DiffWindow.getIntuiWindow());
}

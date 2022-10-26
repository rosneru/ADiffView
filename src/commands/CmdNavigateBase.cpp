#ifdef __clang__
  #include <clib/intuition_protos.h>
#else
  #include <proto/intuition.h>
#endif

#include "CmdNavigateBase.h"

CmdNavigateBase::CmdNavigateBase(std::vector<WindowBase*>* pAllWindowsVector,
                                 DiffWorker& diffWorker,
                                 DiffWindow& diffWindow,
                                 CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg)
  : CommandBase(pAllWindowsVector),
    m_DiffWorker(diffWorker),
    m_DiffWindow(diffWindow),
    m_CmdWindowTitleQuickMsg(cmdWindowTitleQuickMsg),
    m_pDiffDocument(NULL),
    m_FormerLineId(-1)
{

}


long CmdNavigateBase::getPrevResult()
{
  applyDocumentChanged();
  if((m_pDiffDocument == NULL) || 
     (m_pDiffDocument->getDiffIndices().size() < 1))
  {
    return -1;
  }

  if(m_DiffIndicesIterator == m_pDiffDocument->getDiffIndices().begin())
  {
    return -1;
  }

  m_DiffIndicesIterator--;
  return (*m_DiffIndicesIterator);
}


long CmdNavigateBase::getPrevResult(size_t startLineId)
{
  applyDocumentChanged();
  if((m_pDiffDocument == NULL) || 
     (m_pDiffDocument->getDiffIndices().size() < 1))
  {
    return -1;
  }

  std::list<size_t>::const_iterator itBeforeStart= 
    m_pDiffDocument->getDiffIndices().begin();
  itBeforeStart--;

  m_DiffIndicesIterator = m_pDiffDocument->getDiffIndices().end();
  m_DiffIndicesIterator--;

  // Forward iterator to the first result after or equal startLineId
  while((*m_DiffIndicesIterator) >= startLineId)
  {
    m_DiffIndicesIterator--;
    if(m_DiffIndicesIterator == itBeforeStart)
    {
      // Avoid overflow: back to last valid item
      m_DiffIndicesIterator = m_pDiffDocument->getDiffIndices().begin();
      return -1;
    }
  }

  return (*m_DiffIndicesIterator);
}


long CmdNavigateBase::getNextResult()
{
  applyDocumentChanged();
  if((m_pDiffDocument == NULL) || 
     (m_pDiffDocument->getDiffIndices().size() < 1))
  {
    return -1;
  }

  m_DiffIndicesIterator++;

  if(m_DiffIndicesIterator == m_pDiffDocument->getDiffIndices().end())
  {
    // Avoid overflow: back to last valid item
    m_DiffIndicesIterator--;
    return -1;
  }

  return (*m_DiffIndicesIterator);
}


long CmdNavigateBase::getNextResult(size_t startLineId)
{
  applyDocumentChanged();
  if((m_pDiffDocument == NULL) || 
     (m_pDiffDocument->getDiffIndices().size() < 1))
  {
    return -1;
  }

  m_DiffIndicesIterator = m_pDiffDocument->getDiffIndices().begin();

  // Forward iterator to the first result after or equal startLineId
  while((*m_DiffIndicesIterator) < startLineId)
  {
    m_DiffIndicesIterator++;
    if(m_DiffIndicesIterator == m_pDiffDocument->getDiffIndices().end())
    {
      // Avoid overflow: back to last valid item
      m_DiffIndicesIterator = m_pDiffDocument->getDiffIndices().begin();
    }
  }

  return (*m_DiffIndicesIterator);
}


void CmdNavigateBase::signalNoResultFound(const char* pTitleMessage)
{
  // Flash the display
  DisplayBeep(m_DiffWindow.getScreen().getIntuiScreen());

  // Set a text message into window title
  m_CmdWindowTitleQuickMsg.setTitleMessage(pTitleMessage);
  m_CmdWindowTitleQuickMsg.Execute(m_DiffWindow.getIntuiWindow());
}


void CmdNavigateBase::applyDocumentChanged()
{
  if(m_pDiffDocument != m_DiffWorker.getDiffDocument())
  {
    m_FormerLineId = -1;
    m_pDiffDocument = m_DiffWorker.getDiffDocument();
    if(m_pDiffDocument == NULL)
    {
      return;
    }

    m_DiffIndicesIterator = m_pDiffDocument->getDiffIndices().begin();
  }
}

#include "SelectableDiffFile.h"

SelectableDiffFile::SelectableDiffFile(const DiffFileBase& diffFile)
  : m_DiffFile(diffFile),
    m_SearchResultSelection(diffFile.getLines()),
    m_DynamicSelection(diffFile.getLines()),
    m_pCurrentSelection(&m_SearchResultSelection)
{

}

const DiffLine* SelectableDiffFile::operator[](unsigned long index) const
{
  if(index >= m_DiffFile.getNumLines())
  {
    return NULL;
  }

  return m_DiffFile[index];
}

unsigned long SelectableDiffFile::getNumLines() const
{
  return m_DiffFile.getNumLines();
}

void SelectableDiffFile::addSearchResultSelectionBlock(unsigned long lineId,
                                                  unsigned long fromColumn,
                                                  unsigned long toColumn)
{
  m_SearchResultSelection.addBlock(lineId, fromColumn, toColumn);
}


void SelectableDiffFile::startDynamicSelection(unsigned long lineId,
                                               unsigned long columnId)
{
  m_DynamicSelection.startDynamicSelection(lineId, columnId);
}

void SelectableDiffFile::updateDynamicSelection(unsigned long lineId,
                                                unsigned long columnId)
{
  m_DynamicSelection.updateDynamicSelection(lineId, columnId);
}

const std::list<long>& SelectableDiffFile::getUpdatedLineIds()
{
  return m_pCurrentSelection->getUpdatedLineIds();
}

void SelectableDiffFile::clearUpdatedLineIds()
{
  m_pCurrentSelection->clearUpdatedLineIds();
}

void SelectableDiffFile::clearSearchResultSelection()
{
  m_SearchResultSelection.clear();
}

void SelectableDiffFile::clearDynamicSelection()
{
  m_DynamicSelection.clear();
}

void SelectableDiffFile::activateDynamicSelection()
{
  if(m_pCurrentSelection == &m_DynamicSelection)
  {
    return;
  }

  m_pCurrentSelection = &m_DynamicSelection;

  // TODO rethink , reactivate
  // // Add the formerly selected lines to UpdatedLineIds to get them
  // // redrawn (as selection cleared) if necessary
  // addToUpdatedLines(m_SearchResultSelection.getSelectionLines());

  return;
}

void SelectableDiffFile::activateSearchResultSelection(long pageTopLineId,
                                                       long pageBottomLineId)
{
  if(m_pCurrentSelection == &m_SearchResultSelection)
  {
    return;
  }

  m_pCurrentSelection = &m_SearchResultSelection;

  // TODO rethink , reactivate
  // // Add the formerly selected lines to UpdatedLineIds to get them
  // // redrawn (as selection cleared) if necessary
  // addToUpdatedLines(m_DynamicSelection.getSelectionLines());


  // // Add the lineIds of the selected lines of the current page to
  // // updated lines collection
  // const std::list<TextSelectionLine*>* pSelectedLines = m_SearchResultSelection.getSelectionLines();
  // std::list<TextSelectionLine*>::const_iterator it;
  // for(it = pSelectedLines->begin(); it != pSelectedLines->end(); it++)
  // {
  //   long lineId = (*it)->getLineId();
  //   if(lineId < pageTopLineId)
  //   {
  //     continue;
  //   }

  //   if(lineId >= pageBottomLineId)
  //   {
  //     break;
  //   }

  //   m_SearchResultSelection.addUpdatedLine(lineId);
  // }

  return;
}

bool SelectableDiffFile::isPointInSelection(unsigned long lineId,
                                            unsigned long columnId) const
{
  return m_pCurrentSelection->isSelected(lineId, columnId);
}

long SelectableDiffFile::getNumNormalChars(unsigned long lineId, 
                                           unsigned long columnId)
{
  if(lineId >= m_DiffFile.getNumLines())
  {
    return 0;
  }

  if(columnId > m_DiffFile[lineId]->getNumChars())
  {
    return 0;
  }

  long numMarkedChars = m_pCurrentSelection->getNumMarkedChars(lineId, columnId);
  if(numMarkedChars > 0)
  {
    return 0;
  }

  long nextSelStart = m_pCurrentSelection->getNextSelectionStart(lineId, columnId);
  if(nextSelStart > 0)
  {
    return nextSelStart - columnId;
  }
  
  return m_DiffFile[lineId]->getNumChars() - columnId;
}


long SelectableDiffFile::getNumMarkedChars(unsigned long lineId, 
                                           unsigned long columnId)
{
  return m_pCurrentSelection->getNumMarkedChars(lineId, columnId);
}

void SelectableDiffFile::addToUpdatedLines(const std::list<TextSelectionLine*>* pLineCollection)
{
  if(pLineCollection == NULL)
  {
    return;
  }

  std::vector<long> formerSelectedLineIds;
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = pLineCollection->begin(); it != pLineCollection->end(); it++)
  {
    formerSelectedLineIds.push_back((*it)->getLineId());
  }
  
  m_pCurrentSelection->addUpdatedLines(formerSelectedLineIds);
}

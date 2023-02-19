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

void SelectableDiffFile::activateDynamicSelection(long pageTopLineId,
                                                  long pageBottomLineId)
{
  if(m_pCurrentSelection == &m_DynamicSelection)
  {
    return;
  }

  m_pCurrentSelection = &m_DynamicSelection;

  // Add the formerly selected lines to UpdatedLineIds to get them
  // redrawn if necessary, e.g. when selection is cleared.
  // For the algorithm see file
  //   "(WDB) (DEV) (ALGORITHM) (AMIGA) ADiffView - Text selection updated lines.xlsx"
  if(m_SearchResultSelection.getSelectionLines()->size() < 1)
  {
    return;
  }

  long lowestSelectedLineId = m_SearchResultSelection.getSelectionLines()->front()->getLineId();
  long highestSelectedLineId = m_SearchResultSelection.getSelectionLines()->back()->getLineId();
  long from = std::max(lowestSelectedLineId, pageTopLineId);
  long to = std::min(highestSelectedLineId, pageBottomLineId);
  if(from <= to)
  {
    std::vector<long> formerSelectedLineIds;
    for(long i = from; i <= to; i++)
    {
      if(m_SearchResultSelection.isLineSelected(i))
      {
        formerSelectedLineIds.push_back(i);
      }
    }

    m_DynamicSelection.addUpdatedLines(formerSelectedLineIds);
  }

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

  // Add the formerly selected lines to UpdatedLineIds to get them
  // redrawn if necessary, e.g. when selection is cleared.
  // For the algorithm see file
  //   "(WDB) (DEV) (ALGORITHM) (AMIGA) ADiffView - Text selection updated lines.xlsx"
  long from = std::max(m_DynamicSelection.getMinLineId(), pageTopLineId);
  long to = std::min(m_DynamicSelection.getMaxLineId(), pageBottomLineId);
  if(from <= to)
  {
    std::vector<long> formerSelectedLineIds;
    for(long i = from; i <= to; i++)
    {
      formerSelectedLineIds.push_back(i);
    }

    m_SearchResultSelection.addUpdatedLines(formerSelectedLineIds);
  }

  // Add the lineIds of the selected lines of the current page to
  // updated lines collection
  const std::list<TextSelectionLine*>* pSelectedLines = m_SearchResultSelection.getSelectionLines();
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = pSelectedLines->begin(); it != pSelectedLines->end(); it++)
  {
    long lineId = (*it)->getLineId();
    if(lineId < pageTopLineId)
    {
      continue;
    }

    if(lineId >= pageBottomLineId)
    {
      break;
    }

    m_SearchResultSelection.addUpdatedLine(lineId);
  }

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

const DynamicSelection& SelectableDiffFile::getDynamicSelection()
{
  return m_DynamicSelection;
}

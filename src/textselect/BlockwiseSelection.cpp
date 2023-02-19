#include <algorithm>
#include <stddef.h>
#include "BlockwiseSelection.h"

BlockwiseSelection::BlockwiseSelection(const std::vector<DiffLine*>& textLines)
  : SelectionBase(textLines)
{
}

BlockwiseSelection::~BlockwiseSelection()
{
  clear();
}

void BlockwiseSelection::addBlock(unsigned long lineId, 
                                  unsigned long fromColumn, 
                                  unsigned long toColumn)
{
  TextSelectionLine* pSelectionLine = findSelectionLine(lineId);
  if(pSelectionLine != NULL)
  {
    pSelectionLine->addBlock(fromColumn, toColumn);
  }
  else
  {
    pSelectionLine = new TextSelectionLine(lineId, fromColumn, toColumn);
    
    // Now inserting this new TextSelectionLine sorted ascending by its
    // LineId(). NOTE: Iterating here is done backwards assuming that
    // addBlock() itself is mostly called with ascending line numbers
    // (e.g. as it is done in TextSearch); and so it is faster.
    std::list<TextSelectionLine*>::reverse_iterator it;
    for(it = m_SelectionLines.rbegin(); it != m_SelectionLines.rend(); it++)
    {
      if((*it)->getLineId() < lineId)
      {
        m_SelectionLines.insert(it.base(), pSelectionLine);
        m_UpdatedLineIds.push_front(lineId);
        return;
      }
    }

    // List is empty
    m_SelectionLines.push_front(pSelectionLine);
  }
}


void BlockwiseSelection::clear()
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    m_UpdatedLineIds.push_back((*it)->getLineId());
    delete *it;
  }

  m_SelectionLines.clear();
}

bool BlockwiseSelection::isSelected(unsigned long lineId, unsigned long columnId) const
{
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    if((*it)->getLineId() == lineId)
    {
      return (*it)->isSelected(columnId);
    }
    else if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectionLines is sorted by lineId
      return false;
    }
  }

  return false;
}

bool BlockwiseSelection::isLineSelected(unsigned long lineId) const
{
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectionLines is sorted by lineId
      return 0;
    }
    if((*it)->getLineId() == lineId)
    {
      return true;
    }
  }

  return false;
}

long BlockwiseSelection::getNumMarkedChars(unsigned long lineId, 
                                           unsigned long columnId) const
{
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectionLines is sorted by lineId
      return 0;
    }

    if((*it)->getLineId() == lineId)
    {
      long numMarkedChars = (*it)->getNumMarkedChars(columnId);
      if(numMarkedChars > 0)
      {
        return numMarkedChars;
      }
    }
  }

  return 0;
}


long BlockwiseSelection::getNextSelectionStart(unsigned long lineId, 
                                               unsigned long columnId) const
{
  std::list<TextSelectionLine*>::const_iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectionLines is sorted by lineId
      return -1;
    }

    if((*it)->getLineId() == lineId)
    {
      return (*it)->getNextSelectionStart(columnId);
    }
  }

  return -1;
}

const std::list<TextSelectionLine*>* BlockwiseSelection::getSelectionLines() const
{
  return &m_SelectionLines;
}

TextSelectionLine* BlockwiseSelection::findSelectionLine(unsigned long lineId)
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      return NULL;
    }

    if((*it)->getLineId() == lineId)
    {
      return *it;
    }
  }

  return NULL;
}

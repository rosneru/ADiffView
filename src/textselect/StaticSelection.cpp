#include <algorithm>
#include <stddef.h>
#include "StaticSelection.h"

StaticSelection::StaticSelection(const std::vector<DiffLine*>& textLines)
  : SelectionBase(textLines)
{
}

StaticSelection::~StaticSelection()
{
  clear();
}

void StaticSelection::addBlock(long lineId, long fromColumn, long toColumn)
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


void StaticSelection::clear()
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    m_UpdatedLineIds.push_back((*it)->getLineId());
    delete *it;
  }

  m_SelectionLines.clear();
}

bool StaticSelection::isSelected(long lineId, long columnId) const
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

bool StaticSelection::isLineSelected(long lineId) const
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

long StaticSelection::getNumMarkedChars(long lineId, long columnId) const
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


long StaticSelection::getNextSelectionStart(long lineId, long columnId) const
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

const std::list<TextSelectionLine*>* StaticSelection::getSelectionLines() const
{
  return &m_SelectionLines;
}

TextSelectionLine* StaticSelection::findSelectionLine(long lineId)
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

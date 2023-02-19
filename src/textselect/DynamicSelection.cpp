#include <algorithm>
#include <stddef.h>
#include "DynamicSelection.h"

DynamicSelection::DynamicSelection(const std::vector<DiffLine*>& textLines)
  : SelectionBase(textLines),
    m_StartLineId(-1),
    m_StartColumnId(-1),
    m_MinLineId(-1),
    m_MinLineColumnId(-1),
    m_MaxLineId(-1),
    m_MaxLineColumnId(-1)
{
}

DynamicSelection::~DynamicSelection()
{
}

void DynamicSelection::clear()
{
  if(m_MinLineId >= 0)
  {
    for(long i = m_MinLineId; i <= m_MaxLineId; i++)
    {
      m_UpdatedLineIds.push_front(i);
    }
  }

  m_StartLineId = -1;
  m_StartColumnId = -1;
  m_MinLineId = -1;
  m_MinLineColumnId = -1;
  m_MaxLineId = -1;
  m_MaxLineColumnId = -1;
}

void DynamicSelection::startDynamicSelection(unsigned long lineId,
                                             unsigned long columnId)
{
  if(lineId > (m_TextLines.size() - 1))
  {
    return;
  }

  clear();

  m_StartLineId = lineId;
  m_MinLineId = lineId;
  m_MaxLineId = lineId;
  
  unsigned long lastColumn = m_TextLines[lineId]->getNumChars();
  m_StartColumnId = std::min(columnId, lastColumn);
  m_MinLineColumnId = m_StartColumnId;
  m_MaxLineColumnId = m_StartColumnId;

  // TODO Before clearing the selected lines above add all line ids from
  // there to the m_UpdatedLineIds
  m_UpdatedLineIds.push_back(lineId);
}

void DynamicSelection::updateDynamicSelection(unsigned long lineId,
                                              unsigned long columnId)
{
  long maxAllowedColumnId = m_TextLines[lineId]->getNumChars();
  if(columnId > maxAllowedColumnId)
  {
    columnId = maxAllowedColumnId;
  }

  if(lineId < m_MinLineId                                       // Extend selection upward
  || (m_MinLineId < m_StartLineId && lineId <= m_StartLineId)   // Reduce upward selection
  && m_MinLineId != lineId)                                     // LineId  not changed
  {
    long fromLineId = std::min((long)lineId, m_MinLineId);
    long toLineId = std::max((long)lineId, m_MinLineId);
    for(long i = fromLineId; i <= toLineId; i++)
    {
      m_UpdatedLineIds.push_front(i);
    }

    m_MinLineId = lineId;
    m_MinLineColumnId = columnId;
    m_MaxLineId = m_StartLineId;
    m_MaxLineColumnId = m_StartColumnId;
    return;
  }

  if(lineId > m_MaxLineId                                       // Extend selection downward
  || (m_MaxLineId > m_StartLineId && lineId >= m_StartLineId)   // Reduce downward selection
  && m_MaxLineId != lineId)                                     // LineId  not changed
  {
    long fromLineId = std::min((long)lineId, m_MaxLineId);
    long toLineId = std::max((long)lineId, m_MaxLineId);
    for(long i = fromLineId; i <= toLineId; i++)
    {
      m_UpdatedLineIds.push_front(i);
    }

    m_MaxLineId = lineId;
    m_MaxLineColumnId = columnId;
    m_MinLineId = m_StartLineId;
    m_MinLineColumnId = m_StartColumnId;
    return;
  }

  if(m_MinLineId == m_MaxLineId)
  {
    // long fromColumnId = std::min(m_MinLineColumnId, m_StartColumnId);
    // long toColumnId = std::max(m_MinLineColumnId, m_StartColumnId);
    // fromColumnId = std::min(m_MaxLineColumnId, m_StartColumnId);
    // toColumnId = std::max(m_MaxLineColumnId, m_StartColumnId);
    // // if(columnId < fromColumnId || columnId > toColumnId)
    if(columnId != m_MinLineColumnId)
    {
      m_MinLineColumnId = columnId;
      m_MaxLineColumnId = columnId;
      m_UpdatedLineIds.push_front(lineId);
    }

    return;
  }

  if(lineId == m_MinLineId)
  {
    if(columnId == m_MinLineColumnId)
    {
      return;
    }

    m_MinLineColumnId = columnId;
    m_UpdatedLineIds.push_front(lineId);
    return;
  }

  if(lineId == m_MaxLineId)
  {
    if(columnId == m_MaxLineColumnId)
    {
      return;
    }

    m_MaxLineColumnId = columnId;
    m_UpdatedLineIds.push_front(lineId);
    return;
  }

}

bool DynamicSelection::isSelected(unsigned long lineId, unsigned long columnId) const
{
  if(lineId < m_MinLineId || lineId > m_MaxLineId)
  {
    return false;
  }

  if(lineId == m_MinLineId)
  {
    if(m_MinLineId == m_MaxLineId)
    {
      long from = std::min(m_MinLineColumnId, m_StartColumnId);
      long to = std::max(m_MinLineColumnId, m_StartColumnId);
      if(columnId >= from && columnId <= to)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    if(columnId > m_MaxLineColumnId
    && columnId <= m_TextLines[lineId]->getNumChars())
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  if(lineId == m_MaxLineId)
  {
    if(columnId <= m_MaxLineColumnId)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  if(columnId <= m_TextLines[lineId]->getNumChars())
  {
    return true;
  }

  return false;
}

long DynamicSelection::getNumMarkedChars(unsigned long lineId, 
                                         unsigned long columnId) const
{
  if(lineId < m_MinLineId || lineId > m_MaxLineId)
  {
    return 0;
  }
  
  if(m_MinLineId == m_MaxLineId)  // Requested line is the only, singular selected line
  {
    long fromColumn = std::min(m_MinLineColumnId, m_StartColumnId);
    long toColumn = std::max(m_MaxLineColumnId, m_StartColumnId);
    if(columnId < fromColumn || columnId > toColumn)
    {
      return 0;
    }

    return toColumn - columnId;
  }

  if(lineId == m_MinLineId)
  {
    if(columnId < m_MinLineColumnId)
    {
      return 0;
    }

    return m_TextLines[lineId]->getNumChars() - columnId;
  }

  if(lineId == m_MaxLineId)
  {
    if(columnId > m_MaxLineColumnId)
    {
      return 0;
    }

    return m_MaxLineColumnId - columnId;
  }

  // For the lines in the middle (no start or end line) return the 
  // number of selected characters from the requested column
  unsigned long lineTextLength = m_TextLines[lineId]->getNumChars();
  if(columnId > lineTextLength)
  {
    // Requested column exceeds line length
    return -1;
  }

  return lineTextLength - columnId;
}


long DynamicSelection::getNextSelectionStart(unsigned long lineId, 
                                             unsigned long columnId) const
{
  if(lineId < m_MinLineId || lineId > m_MaxLineId)
  {
    return -1;
  }

  if(m_MinLineId == m_MaxLineId) // Requested line is the one and only selected line
  {
    if(columnId > m_MinLineColumnId)
    {
      return -1;
    }

    if(columnId > m_MinLineColumnId)
    {
      return 0;
    }

    return std::min(m_MinLineColumnId, m_StartColumnId) - columnId;
  }

  if(lineId == m_MinLineId)
  {
    if(columnId > m_MinLineColumnId)
    {
      return -1;
    }

    if(columnId > m_MinLineColumnId)
    {
      return 0;
    }

    return m_MinLineColumnId - columnId;
  }

  if(lineId == m_MaxLineId)
  {
    if(columnId > m_MaxLineColumnId)
    {
      return -1;
    }

    return 0;
  }

  return 0;
}



long DynamicSelection::getMinLineId() const
{
  return m_MinLineId;
}

long DynamicSelection::getMaxLineId() const
{
  return m_MaxLineId;
}

long DynamicSelection::getNumTotalSelectedChars() const
{
  long totalSelectedChars = 0;
  for(long i = m_MinLineId; i <= m_MaxLineId; i++)
  {
    if(i == m_MinLineId || i == m_MaxLineId)
    {
      long lineSelectionStartColumn = getNextSelectionStart(i, 0);
      totalSelectedChars += getNumMarkedChars(i, lineSelectionStartColumn);
    }
    else
    {
      totalSelectedChars += m_TextLines[i]->getNumChars();
    }
  }

  return totalSelectedChars;
}

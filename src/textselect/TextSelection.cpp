#include <algorithm>
#include <stddef.h>
#include "TextSelection.h"

TextSelection::TextSelection(const std::vector<DiffLine*>& textLines)
  : m_TextLines(textLines),
    m_UpdateDirection(UD_NONE)
{
}

TextSelection::~TextSelection()
{
  clear();
}

void TextSelection::startDynamicSelection(unsigned long lineId,
                                          unsigned long columnId)
{
  m_UpdatedLineIds.clear();
  clear();
  m_UpdateDirection = TextSelection::UD_NONE;
  m_StartLineId = lineId;
  m_StartColumnId = columnId;
  m_SelectionLines.push_back(new TextSelectionLine(lineId, columnId, columnId));

  // TODO Before clearing the selected lines above add all line ids from
  // there to the m_UpdatedLineIds
  m_UpdatedLineIds.push_back(lineId);

  m_UpdatedLineIds.sort();
  m_UpdatedLineIds.unique();
}

void TextSelection::updateDynamicSelection(unsigned long lineId,
                                           unsigned long columnId)
{
  UpdateDirection newUpdateDirection;
  unsigned long lineIdIns;
  unsigned long lastColumn, fromColumn, toColumn; 
  m_UpdatedLineIds.clear();

  // Check if update is on a line which already contains a selection
  TextSelectionLine* pLineToUpdate = findSelectionLine(lineId);
  if(pLineToUpdate != NULL)
  {
    TextSelectionRange* pSelectedBlock = pLineToUpdate->getFirstSelectedBlock();
    if(pSelectedBlock != NULL)
    {
      if(lineId == m_StartLineId)
      {
        fromColumn = std::min(columnId, m_StartColumnId);
        toColumn = std::max(columnId, m_StartColumnId);
        if(pSelectedBlock->getFromColumn() != fromColumn 
        || pSelectedBlock->getToColumn() != toColumn)
        {
          pSelectedBlock->setFromColumn(fromColumn);
          pSelectedBlock->setToColumn(toColumn);
          m_UpdatedLineIds.push_back(lineId);
        }
      }
      else if(lineId < m_StartLineId)
      {
        if(pSelectedBlock->getFromColumn() != columnId)
        {
          pSelectedBlock->setFromColumn(columnId);
          m_UpdatedLineIds.push_back(lineId);
        }
      }
      else if(lineId > m_StartLineId)
      {
        if(pSelectedBlock->getToColumn() != columnId)
        {
          pSelectedBlock->setToColumn(columnId);
          m_UpdatedLineIds.push_back(lineId);
        }
      }
    }
  }

  newUpdateDirection = calcUpdateDirection(lineId);
  if(newUpdateDirection != TextSelection::UD_NONE)
  {
    m_UpdateDirection = newUpdateDirection;
  }

  switch(newUpdateDirection)
  {
    case TextSelection::UD_NONE:
      break;
    case TextSelection::UD_START_UPWARD:
      clear();

      // Select start line from start char to last column
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectionLines.push_back(new TextSelectionLine(lineId, columnId, lastColumn));
      m_UpdatedLineIds.push_back(lineId);

      // Add the fully selected lines between selection start line and target lineId
      for(lineIdIns = lineId + 1; lineIdIns < m_StartLineId; lineIdIns++)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectionLines.push_back(new TextSelectionLine(lineIdIns, 0, lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

      m_SelectionLines.push_back(new TextSelectionLine(m_StartLineId, 0, m_StartColumnId));
      m_UpdatedLineIds.push_back(m_StartLineId);
      break;
    case TextSelection::UD_APPEND_UPWARD:
      // Remove the former, only partly selected top line
      clearFirstSelectionLine();

      // Add the fully selected lines from former to new top line
      for(lineIdIns = m_LowestLineId; lineIdIns > lineId; lineIdIns--)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectionLines.push_front(new TextSelectionLine(lineIdIns, 0, lastColumn));
        m_UpdatedLineIds.push_front(lineIdIns);
      }

      // Add the new top line
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectionLines.push_front(new TextSelectionLine(lineId, columnId, lastColumn));
      m_UpdatedLineIds.push_front(lineId);
      break;
    case TextSelection::UD_REDUCE_TOP:
      // Remove reduced top lines including the still fully selected new
      // top line
      for(unsigned int i = m_LowestLineId; i <= lineId; i++)
      {
        clearFirstSelectionLine();
      }

      // Re-add the new top line but this time only partly selected
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectionLines.push_front(new TextSelectionLine(lineId, columnId, lastColumn));

      // m_UpdatedLineIds.push_back(lineId);
      
      // NOTE: The line above is commented out because there is no need
      // to add lineId to m_UpdatedLineIds. It has been done already in
      // clearFirstSelectionLine().
      break;
    case TextSelection::UD_STOP_UPWARD:
      clear();
      fromColumn = std::min(columnId, m_StartColumnId);
      toColumn = std::max(columnId, m_StartColumnId);
      m_SelectionLines.push_front(new TextSelectionLine(lineId, fromColumn, toColumn));
      break;
    case TextSelection::UD_START_DOWNWARD:
      clear();

      // Select start line from column 0 to start char
      lastColumn = m_TextLines[m_StartLineId]->getNumChars() - 1;
      m_SelectionLines.push_front(new TextSelectionLine(m_StartLineId, m_StartColumnId, lastColumn));

      // Add the fully selected lines between selection start line and target lineId
      for(lineIdIns = m_StartLineId + 1; lineIdIns < lineId; lineIdIns--)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectionLines.push_back(new TextSelectionLine(lineIdIns, 0, lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

       // Also add the new bottom line
      m_SelectionLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      m_UpdatedLineIds.push_back(lineId);
      break;
    case TextSelection::UD_APPEND_DOWNWARD:
      // Remove the former bottom line
      clearLastSelectionLine();

      // Add the fully selected lines from former to new bottom line
      for(lineIdIns = m_HighestLineId; lineIdIns < lineId; lineIdIns++)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectionLines.push_back(new TextSelectionLine(lineIdIns, 0, lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

      // Also add the new bottom line
      m_SelectionLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      m_UpdatedLineIds.push_back(lineId);  
      break;
    case TextSelection::UD_REDUCE_BOTTOM:
      // Remove reduced bottom lines including the still fully selected
      // new bottom line
      for(unsigned int i = m_HighestLineId; i >= lineId; i--)
      {
        clearLastSelectionLine();
      }

      // Re-add the new bottom line but this time only partly selected
      m_SelectionLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      break;
    case TextSelection::UD_STOP_DOWNWARD:
      clear();
      fromColumn = std::min(columnId, m_StartColumnId);
      toColumn = std::max(columnId, m_StartColumnId);
      m_SelectionLines.push_front(new TextSelectionLine(lineId, fromColumn, toColumn));
      break;
  }

  m_UpdatedLineIds.sort();
  m_UpdatedLineIds.unique();
}

void TextSelection::addBlock(unsigned long lineId, 
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
    
    // Now inserting this new TextSelectionLine sorted acending by its
    // LineId(). NOTE: Iterating here is done backwards assuming that
    // addBlock() itself is mostly called with ascending line numbers
    // (e.g. as it is done in TextSearch); and so it is faster.
    std::list<TextSelectionLine*>::reverse_iterator it;
    for(it = m_SelectionLines.rbegin(); it != m_SelectionLines.rend(); it++)
    {
      if((*it)->getLineId() < lineId)
      {
        m_SelectionLines.insert(it.base(), pSelectionLine);
        return;
      }
    }

    // List is empty
    m_SelectionLines.push_front(pSelectionLine);
  }
}


void TextSelection::clear()
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectionLines.begin(); it != m_SelectionLines.end(); it++)
  {
    m_UpdatedLineIds.push_back((*it)->getLineId());
    delete *it;
  }

  m_SelectionLines.clear();
}

bool TextSelection::isSelected(unsigned long lineId, unsigned long columnId) const
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

long TextSelection::getNumMarkedChars(unsigned long lineId, 
                                      unsigned long columnId)
{
  std::list<TextSelectionLine*>::iterator it;
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


long TextSelection::getNextSelectionStart(unsigned long lineId, 
                                          unsigned long columnId)
{
  std::list<TextSelectionLine*>::iterator it;
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

const std::list<int>& TextSelection::getUpdatedLineIds() const
{
  return m_UpdatedLineIds;
}

const std::list<TextSelectionLine*>* TextSelection::getSelectionLines() const 
{
  return &m_SelectionLines;
}

void TextSelection::clearUpdatedLineIds()
{
  m_UpdatedLineIds.clear();
}

void TextSelection::clearFirstSelectionLine()
{
  // Remove first TextSelectionLine
  std::list<TextSelectionLine*>::iterator it = m_SelectionLines.begin();
  m_UpdatedLineIds.push_back((*it)->getLineId());
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectionLines.pop_front();
}

void TextSelection::clearLastSelectionLine()
{
  // Remove last TextSelectionLine
  std::list<TextSelectionLine*>::iterator it = m_SelectionLines.end();
  --it;
  m_UpdatedLineIds.push_back((*it)->getLineId());
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectionLines.pop_back();
}

TextSelection::UpdateDirection TextSelection::calcUpdateDirection(
  unsigned long lineId)
{
  unsigned long numLinesSelected = m_SelectionLines.size();
  m_LowestLineId = m_SelectionLines.front()->getLineId();
  m_HighestLineId = m_SelectionLines.back()->getLineId();

  if(numLinesSelected == 1)
  {
    if(m_StartLineId > lineId)
    {
      return TextSelection::UD_START_UPWARD;
    }
    else if(m_StartLineId < lineId)
    {
      return TextSelection::UD_START_DOWNWARD;
    }
  }
  else if(numLinesSelected > 1)
  {
    if(lineId < m_LowestLineId)
    {
      if(m_HighestLineId > m_StartLineId)
      {
        return TextSelection::UD_START_UPWARD;
      }
      else
      {
        return TextSelection::UD_APPEND_UPWARD;
      }
    }
    else if(lineId > m_HighestLineId)
    {
      if(m_LowestLineId < m_StartLineId)
      {
        return TextSelection::UD_START_DOWNWARD;
      }
      else
      {
        return TextSelection::UD_APPEND_DOWNWARD;
      }
    }
    else if(lineId < m_StartLineId)
    {
      if(m_UpdateDirection == TextSelection::UD_START_UPWARD ||
         m_UpdateDirection == TextSelection::UD_APPEND_UPWARD)
      {
        if(lineId == m_LowestLineId)
        {
          return TextSelection::UD_NONE;
        }
      }
      return TextSelection::UD_REDUCE_TOP;
    }
    else if(lineId > m_StartLineId)
    {
      if(m_UpdateDirection == TextSelection::UD_START_DOWNWARD ||
         m_UpdateDirection == TextSelection::UD_APPEND_DOWNWARD)
      {
        if(lineId == m_HighestLineId)
        {
          return TextSelection::UD_NONE;
        }
      }
      return TextSelection::UD_REDUCE_BOTTOM;
    }
    else if(lineId == m_HighestLineId)
    {
      return TextSelection::UD_STOP_UPWARD;
    }
    else if(lineId == m_LowestLineId)
    {
      return TextSelection::UD_STOP_DOWNWARD;
    }
  }

  return TextSelection::UD_NONE;
}

TextSelectionLine* TextSelection::findSelectionLine(unsigned long lineId)
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

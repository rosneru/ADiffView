#include <algorithm>
#include <stddef.h>
#include "TextSelection.h"

TextSelection::TextSelection(const std::vector<DiffLine*>& textLines)
  : m_TextLines(textLines),
    m_UpdateDirection(NONE)
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
  m_UpdateDirection = TextSelection::NONE;
  m_SelectionStartLine = lineId;
  m_SelectionStartColumn = columnId;
  addBlock(lineId, columnId, columnId);

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
  unsigned long oldTopLastColumn, oldBottomLastColumn;
  m_UpdatedLineIds.clear();

  // Check if update is on a line which already contains a selection
  TextSelectionLine* pLineToUpdate = findSelectionLine(lineId);
  if(pLineToUpdate != NULL)
  {
    TextSelectionRange* pSelectedBlock = pLineToUpdate->getFirstSelectedBlock();
    if(pSelectedBlock != NULL)
    {
      if(columnId > pSelectedBlock->getToColumn())
      {
        pSelectedBlock->setToColumn(columnId);
        m_UpdatedLineIds.push_back(lineId);
        m_UpdatedLineIds.sort();
        m_UpdatedLineIds.unique();
        return;
      }
      else if(columnId < pSelectedBlock->getFromColumn())
      {
        pSelectedBlock->setFromColumn(columnId);
        m_UpdatedLineIds.push_back(lineId);
        m_UpdatedLineIds.sort();
        m_UpdatedLineIds.unique();
        return;
      }
    }
  }

  newUpdateDirection = calcUpdateDirection(lineId);
  if(newUpdateDirection != TextSelection::NONE)
  {
    m_UpdateDirection = newUpdateDirection;
  }

  switch(newUpdateDirection)
  {
    case TextSelection::NONE:
      break;
    case TextSelection::START_UPWARD:
      clear();

      // Select start line from start char to last column
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_back(new TextSelectionLine(lineId, columnId, lastColumn));
      m_UpdatedLineIds.push_back(lineId);

      // Add the fully selected lines between selection start line and target lineId
      for(lineIdIns = lineId + 1; lineIdIns < m_SelectionStartLine; lineIdIns++)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectedLines.push_back(new TextSelectionLine(lineIdIns,
                                                        0,
                                                        lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

      m_SelectedLines.push_back(new TextSelectionLine(m_SelectionStartLine,
                                                      0,
                                                      m_SelectionStartColumn));
      m_UpdatedLineIds.push_back(m_SelectionStartLine);
      break;
    case TextSelection::APPEND_UPWARD:
      // Remove the former, only partly selected top line
      clearFirstSelectionLine();

      // Add the fully selected lines from former to new top line
      for(lineIdIns = m_LowestLineId;
          lineIdIns > lineId;
          lineIdIns--)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectedLines.push_front(new TextSelectionLine(lineIdIns,
                                                         0,
                                                         lastColumn));
        m_UpdatedLineIds.push_front(lineIdIns);
      }

      // Add the new top line
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_front(new TextSelectionLine(lineId, columnId, lastColumn));
      m_UpdatedLineIds.push_front(lineId);
      break;
    case TextSelection::REDUCE_TOP:
      // Remove reduced top lines including the still fully selected new
      // top line
      for(unsigned int i = m_LowestLineId; i <= lineId; i++)
      {
        clearFirstSelectionLine();
      }

      // Re-add the new top line but this time only partly selected
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_front(new TextSelectionLine(lineId, columnId, lastColumn));

      // m_UpdatedLineIds.push_back(lineId);
      
      // NOTE: The line above is commented out because there is no need
      // to add lineId to m_UpdatedLineIds. It has been done already in
      // clearFirstSelectionLine().
      break;
    case TextSelection::STOP_UPWARD:
      clear();
      fromColumn = std::min(columnId, m_SelectionStartColumn);
      toColumn = std::max(columnId, m_SelectionStartColumn);
      m_SelectedLines.push_front(new TextSelectionLine(lineId, fromColumn, toColumn));
      break;
    case TextSelection::START_DOWNWARD:
      clear();

      // Select start line from column 0 to start char
      lastColumn = m_TextLines[m_SelectionStartLine]->getNumChars() - 1;
      m_SelectedLines.push_front(new TextSelectionLine(m_SelectionStartLine,
                                                       m_SelectionStartColumn,
                                                       lastColumn));

      // Add the fully selected lines between selection start line and target lineId
      for(lineIdIns = m_SelectionStartLine + 1;
          lineIdIns < lineId;
          lineIdIns--)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectedLines.push_back(new TextSelectionLine(lineIdIns,
                                                        0,
                                                        lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

      m_SelectedLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      m_UpdatedLineIds.push_back(lineId);
      break;
    case TextSelection::APPEND_DOWNWARD:
      // Remove the former bottom line
      clearLastSelectionLine();

      // Add the fully selected lines from former to new bottom line
      for(lineIdIns = m_HighestLineId;
          lineIdIns < lineId;
          lineIdIns++)
      {
        lastColumn = m_TextLines[lineIdIns]->getNumChars() - 1;
        m_SelectedLines.push_back(new TextSelectionLine(lineIdIns, 0, lastColumn));
        m_UpdatedLineIds.push_back(lineIdIns);
      }

      // Also add the new bottom line
      m_SelectedLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      m_UpdatedLineIds.push_back(lineId);  
      break;
    case TextSelection::REDUCE_BOTTOM:
      // Remove reduced bottom lines including the still fully selected
      // new bottom line
      for(unsigned int i = m_HighestLineId; i >= lineId; i--)
      {
        clearLastSelectionLine();
      }

      // Re-add the new bottom line but this time only partly selected
      m_SelectedLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      break;
    case TextSelection::STOP_DOWNWARD:
      clear();
      fromColumn = std::min(columnId, m_SelectionStartColumn);
      toColumn = std::max(columnId, m_SelectionStartColumn);
      m_SelectedLines.push_front(new TextSelectionLine(lineId, fromColumn, toColumn));
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
    m_SelectedLines.push_back(pSelectionLine);

    // TODO sort m_SelectedLines by lineId
  }
}


void TextSelection::clear()
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectedLines.begin(); it != m_SelectedLines.end(); it++)
  {
    m_UpdatedLineIds.push_back((*it)->getLineId());
    delete *it;
  }

  m_SelectedLines.clear();
}

long TextSelection::getNumMarkedChars(unsigned long lineId, 
                                      unsigned long columnId)
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectedLines.begin(); it != m_SelectedLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectedLines is sorted by lineId
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
  for(it = m_SelectedLines.begin(); it != m_SelectedLines.end(); it++)
  {
    if((*it)->getLineId() > lineId)
    {
      // TODO This only works when m_SelectedLines is sorted by lineId
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

void TextSelection::clearFirstSelectionLine()
{
  // Remove first TextSelectionLine
  std::list<TextSelectionLine*>::iterator it = m_SelectedLines.begin();
  m_UpdatedLineIds.push_back((*it)->getLineId());
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectedLines.pop_front();
}

void TextSelection::clearLastSelectionLine()
{
  // Remove last TextSelectionLine
  std::list<TextSelectionLine*>::iterator it = m_SelectedLines.end();
  --it;
  m_UpdatedLineIds.push_back((*it)->getLineId());
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectedLines.pop_back();
}

TextSelection::UpdateDirection TextSelection::calcUpdateDirection(
  unsigned long lineId)
{
  unsigned long numLinesSelected = m_SelectedLines.size();
  m_LowestLineId = m_SelectedLines.front()->getLineId();
  m_HighestLineId = m_SelectedLines.back()->getLineId();

  if(numLinesSelected == 1)
  {
    if(m_SelectionStartLine > lineId)
    {
      return TextSelection::START_UPWARD;
    }
    else if(m_SelectionStartLine < lineId)
    {
      return TextSelection::START_DOWNWARD;
    }
  }
  else if(numLinesSelected > 1)
  {
    if(lineId < m_LowestLineId)
    {
      if(m_HighestLineId > m_SelectionStartLine)
      {
        return TextSelection::START_UPWARD;
      }
      else
      {
        return TextSelection::APPEND_UPWARD;
      }
    }
    else if(lineId > m_HighestLineId)
    {
      if(m_LowestLineId < m_SelectionStartLine)
      {
        return TextSelection::START_DOWNWARD;
      }
      else
      {
        return TextSelection::APPEND_DOWNWARD;
      }
    }
    else if(lineId < m_SelectionStartLine)
    {
      if(m_UpdateDirection == TextSelection::START_UPWARD ||
         m_UpdateDirection == TextSelection::APPEND_UPWARD)
      {
        if(lineId == m_LowestLineId)
        {
          return TextSelection::NONE;
        }
      }
      return TextSelection::REDUCE_TOP;
    }
    else if(lineId > m_SelectionStartLine)
    {
      if(m_UpdateDirection == TextSelection::START_DOWNWARD ||
         m_UpdateDirection == TextSelection::APPEND_DOWNWARD)
      {
        if(lineId == m_HighestLineId)
        {
          return TextSelection::NONE;
        }
      }
      return TextSelection::REDUCE_BOTTOM;
    }
    else if(lineId == m_HighestLineId)
    {
      return TextSelection::STOP_UPWARD;
    }
    else if(lineId == m_LowestLineId)
    {
      return TextSelection::STOP_DOWNWARD;
    }
  }

  return TextSelection::NONE;
}

TextSelectionLine* TextSelection::findSelectionLine(unsigned long lineId)
{
  std::list<TextSelectionLine*>::iterator it;
  for(it = m_SelectedLines.begin(); it != m_SelectedLines.end(); it++)
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

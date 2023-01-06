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
  clear();
  m_UpdateDirection = TextSelection::NONE;
  m_SelectionStartLine = lineId;
  m_SelectionStartColumn = columnId;
  addBlock(lineId, columnId, columnId);
  m_UpdatedLineIds.clear();

  // TODO Before clearing the selected lines above add all line ids from
  // there to the m_UpdatedLineIds
  m_UpdatedLineIds.push_back(lineId);
}

void TextSelection::updateDynamicSelection(unsigned long lineId,
                                           unsigned long columnId)
{
  UpdateDirection newUpdateDirection;
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
        return;
      }
      else if(columnId < pSelectedBlock->getFromColumn())
      {
        pSelectedBlock->setFromColumn(columnId);
        m_UpdatedLineIds.push_back(lineId);
        return;
      }
    }
  }

  lineId = limitLineId(lineId);
  newUpdateDirection = calcUpdateDirection(lineId);
  switch(newUpdateDirection)
  {
    case TextSelection::NONE:
      break;
    case TextSelection::START_UPWARD:
      clear();
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_front(
        new TextSelectionLine(lineId, columnId, lastColumn));
      m_SelectedLines.push_back(
        new TextSelectionLine(m_SelectionStartLine, 0, m_SelectionStartColumn));
      m_UpdatedLineIds.push_back(lineId);
      m_UpdatedLineIds.push_back(m_SelectionStartLine);
      break;
    case TextSelection::APPEND_UPWARD:
      oldTopLastColumn = m_TextLines[m_LowestLineId]->getNumChars() - 1;

      // Remove the partly selected former top line from selection and
      // add it as a whole
      clearFirstSelectionLine();
      m_SelectedLines.push_front(
        new TextSelectionLine(m_LowestLineId, 0, oldTopLastColumn));

      // Also add the new top line
      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_front(
        new TextSelectionLine(lineId, columnId, lastColumn));
      
      m_UpdatedLineIds.push_back(lineId);
      m_UpdatedLineIds.push_back(m_LowestLineId);
      break;
    case TextSelection::REDUCE_TOP:
      clearFirstSelectionLine();
      clearFirstSelectionLine();

      lastColumn = m_TextLines[lineId]->getNumChars() - 1;
      m_SelectedLines.push_front(
        new TextSelectionLine(lineId, columnId, lastColumn));

      m_UpdatedLineIds.push_back(m_LowestLineId);
      m_UpdatedLineIds.push_back(lineId);
      break;
    case TextSelection::STOP_UPWARD:
      clear();
      fromColumn = std::min(columnId, m_SelectionStartColumn);
      toColumn = std::max(columnId, m_SelectionStartColumn);
      m_SelectedLines.push_front(
        new TextSelectionLine(lineId, fromColumn, toColumn));

      m_UpdatedLineIds.push_back(m_LowestLineId);
      m_UpdatedLineIds.push_back(lineId);
      break;
    case TextSelection::START_DOWNWARD:
      clear();
      lastColumn = m_TextLines[m_SelectionStartLine]->getNumChars() - 1;
      m_SelectedLines.push_front(new TextSelectionLine(m_SelectionStartLine,
                                                       m_SelectionStartColumn,
                                                       lastColumn));
      m_SelectedLines.push_back(new TextSelectionLine(lineId, 0, columnId));

      m_UpdatedLineIds.push_back(m_SelectionStartLine);
      m_UpdatedLineIds.push_back(lineId);
      break;
    case TextSelection::APPEND_DOWNWARD:
      oldBottomLastColumn = m_TextLines[m_HighestLineId]->getNumChars() - 1;

      // Remove the former bottom line from selection and add it as a
      // whole
      clearLastSelectionLine();
      m_SelectedLines.push_back(
        new TextSelectionLine(m_HighestLineId, 0, oldBottomLastColumn));

      // Also add the new bottom line
      m_SelectedLines.push_back(
        new TextSelectionLine(lineId, 0, columnId));

      m_UpdatedLineIds.push_back(m_HighestLineId);
      m_UpdatedLineIds.push_back(lineId);  
      break;
    case TextSelection::REDUCE_BOTTOM:
      clearLastSelectionLine();
      clearLastSelectionLine();
      m_SelectedLines.push_back(new TextSelectionLine(lineId, 0, columnId));
      m_UpdatedLineIds.push_back(lineId);
      m_UpdatedLineIds.push_back(m_HighestLineId);
      break;
    case TextSelection::STOP_DOWNWARD:
      clear();
      fromColumn = std::min(columnId, m_SelectionStartColumn);
      toColumn = std::max(columnId, m_SelectionStartColumn);
      m_SelectedLines.push_front(
        new TextSelectionLine(lineId, fromColumn, toColumn));

      m_UpdatedLineIds.push_back(lineId);
      m_UpdatedLineIds.push_back(m_HighestLineId);
      break;
  }
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
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectedLines.pop_front();
}

void TextSelection::clearLastSelectionLine()
{
  // Remove last TextSelectionLine
  std::list<TextSelectionLine*>::iterator it = m_SelectedLines.end();
  delete *it;

  // And remove its item from the listz of selected lines
  m_SelectedLines.pop_back();
}

unsigned long TextSelection::limitLineId(unsigned long lineId)
{
  m_LowestLineId = m_SelectedLines.front()->getLineId();
  m_HighestLineId = m_SelectedLines.back()->getLineId();

  if(m_UpdateDirection == TextSelection::NONE ||
     m_UpdateDirection == TextSelection::STOP_UPWARD ||
     m_UpdateDirection == TextSelection::STOP_DOWNWARD)
  {
    if(lineId < m_SelectionStartLine)
    {
      return m_SelectionStartLine - 1;
    }
    else if(lineId > m_SelectionStartLine)
    {
      return m_SelectionStartLine + 1;
    }
  }
  else if(m_UpdateDirection == TextSelection::START_UPWARD ||
          m_UpdateDirection == TextSelection::APPEND_UPWARD)
  {
    if(lineId < m_LowestLineId)
    {
      return m_LowestLineId - 1;
    }
    else if(lineId > m_LowestLineId)  // direction change
    {
      return m_LowestLineId + 1;
    }
  }
  else if(m_UpdateDirection == TextSelection::REDUCE_TOP)
  {
    return m_LowestLineId + 1;
  }
  else if (m_UpdateDirection == TextSelection::START_DOWNWARD ||
      m_UpdateDirection == TextSelection::APPEND_DOWNWARD)
  {
    if(lineId > m_HighestLineId)
    {
      return m_HighestLineId + 1;
    }
    else if(lineId < m_HighestLineId) // direction change
    {
      return m_HighestLineId - 1;
    }
  }
  else if(m_UpdateDirection == TextSelection::REDUCE_BOTTOM)
  {
    return m_HighestLineId - 1;
  }

  return lineId;
}

TextSelection::UpdateDirection TextSelection::calcUpdateDirection(
  unsigned long lineId)
{
  unsigned long numLinesSelected = m_SelectedLines.size();
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
      return TextSelection::APPEND_UPWARD;
    }
    else if(lineId > m_HighestLineId)
    {
      return TextSelection::APPEND_DOWNWARD;
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

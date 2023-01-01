#include <stddef.h>
#include "TextSelection.h"

TextSelection::TextSelection(const std::vector<DiffLine*>& textLines)
  : m_TextLines(textLines)
{
}

TextSelection::~TextSelection()
{
  clear();
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
      return 0;
    }

    if((*it)->getLineId() == lineId)
    {
      return (*it)->getNextSelectionStart(columnId);
    }
  }

  return 0;
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
    unsigned long lowest_line_id = m_SelectedLines.front()->getLineId();
    unsigned long highest_line_id = m_SelectedLines.back()->getLineId();
    if(lineId < lowest_line_id)
    {
      return TextSelection::APPEND_UPWARD;
    }
    else if(lineId > highest_line_id)
    {
      return TextSelection::APPEND_DOWNWARD;
    }
    else if(lineId < m_SelectionStartLine)
    {
      return TextSelection::REDUCE_TOP;
    }
    else if(lineId > m_SelectionStartLine)
    {
      return TextSelection::REDUCE_BOTTOM;
    }
    else if(lineId == highest_line_id)
    {
      return TextSelection::STOP_UPWARD;
    }
    else if(lineId == lowest_line_id)
    {
      return TextSelection::STOP_DOWNWARD;
    }
  }

  return TextSelection::UNKNOWN;
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

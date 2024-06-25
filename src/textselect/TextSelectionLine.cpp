#include <stddef.h>

#include "TextSelectionLine.h"

TextSelectionLine::TextSelectionLine(long lineId, long fromColumn, long toColumn)
  : m_LineId(lineId)
{
  addBlock(fromColumn, toColumn);
}

TextSelectionLine::~TextSelectionLine()
{
  std::vector<TextSelectionRange*>::iterator it;
  for(it = m_SelectedBlocks.begin(); it != m_SelectedBlocks.end(); it++)
  {
    delete *it;
  }
}

void TextSelectionLine::addBlock(long fromColumn, long toColumn)
{
  TextSelectionRange* pBlock = new TextSelectionRange(fromColumn, toColumn);
  m_SelectedBlocks.push_back(pBlock);

  // TODO check if adding this block can be achived by extending an existing one

  // TODO ensure that m_SelectedBlocks is sorted by fromColumn
}

bool TextSelectionLine::isSelected(long columnId) const
{
  std::vector<TextSelectionRange*>::const_iterator it;
  for(it = m_SelectedBlocks.begin(); it != m_SelectedBlocks.end(); it++)
  {
    if((*it)->getNumMarkedChars(columnId) > 0)
    {
      return true;
    }
  }

  return false;
}

TextSelectionRange* TextSelectionLine::getFirstSelectedBlock()
{
  if(m_SelectedBlocks.size() < 1)
  {
    return NULL;
  }

  return m_SelectedBlocks[0];
}

long TextSelectionLine::getLineId() const
{
  return m_LineId;
}

long TextSelectionLine::getNumMarkedChars(long columnId)
{
  std::vector<TextSelectionRange*>::iterator it;
  for(it = m_SelectedBlocks.begin(); it != m_SelectedBlocks.end(); it++)
  {
    long numMarkedChars = (*it)->getNumMarkedChars(columnId);
    if(numMarkedChars > 0)
    {
      return numMarkedChars;
    }
  }

  return 0;
}

long TextSelectionLine::getNextSelectionStart(long columnId)
{
  std::vector<TextSelectionRange*>::iterator it;
  for(it = m_SelectedBlocks.begin(); it != m_SelectedBlocks.end(); it++)
  {
    long fromColumn = (*it)->getFromColumn();
    if(fromColumn >= columnId)
    {
      return fromColumn - columnId;
    }
  }

  return -1;
}

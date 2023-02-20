#include "TextSelectionRange.h"

TextSelectionRange::TextSelectionRange(long fromColumn, long toColumn)
  : m_FromColumn(fromColumn),
    m_ToColumn(toColumn)
{

}

long TextSelectionRange::getFromColumn() const
{
  return m_FromColumn;
}

long TextSelectionRange::getToColumn() const
{
  return m_ToColumn;
}

void TextSelectionRange::setFromColumn(long fromColumn)
{
  m_FromColumn = fromColumn;
}

void TextSelectionRange::setToColumn(long toColumn)
{
  m_ToColumn = toColumn;
}

long TextSelectionRange::getNumMarkedChars(long columnId)
{
  if(columnId < m_FromColumn)
  {
    return 0;
  }

  if(columnId > m_ToColumn)
  {
    return 0;
  }

  return m_ToColumn - columnId + 1;
}

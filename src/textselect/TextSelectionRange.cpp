#include "TextSelectionRange.h"

TextSelectionRange::TextSelectionRange(unsigned long fromColumn,
                                       unsigned long toColumn)
  : m_FromColumn(fromColumn),
    m_ToColumn(toColumn)
{

}

unsigned long TextSelectionRange::getFromColumn() const
{
  return m_FromColumn;
}

unsigned long TextSelectionRange::getToColumn() const
{
  return m_ToColumn;
}

long TextSelectionRange::getNumMarkedChars(unsigned long columnId)
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

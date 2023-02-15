#include <algorithm>
#include <stddef.h>
#include "SelectionBase.h"

SelectionBase::SelectionBase(const std::vector<DiffLine*>& textLines)
  : m_TextLines(textLines)
{
}

SelectionBase::~SelectionBase()
{
}

const std::list<long>& SelectionBase::getUpdatedLineIds()
{
  m_UpdatedLineIds.sort();
  m_UpdatedLineIds.unique();

  return m_UpdatedLineIds;
}

void SelectionBase::addUpdatedLine(long lineId)
{
  m_UpdatedLineIds.push_back(lineId);
}

void SelectionBase::addUpdatedLines(const std::vector<long>& linesRange)
{
  std::list<long>::iterator it = m_UpdatedLineIds.end();
  m_UpdatedLineIds.insert(it, linesRange.begin(), linesRange.end());
}

void SelectionBase::clearUpdatedLineIds()
{
  m_UpdatedLineIds.clear();
}

#include "SelectableDiffFile.h"

SelectableDiffFile::SelectableDiffFile(const DiffFileBase& diffFile)
  : m_DiffFile(diffFile),
    m_Selection(diffFile.getLines())
{

}

const DiffLine* SelectableDiffFile::operator[](unsigned long index) const
{
  if(index >= m_DiffFile.getNumLines())
  {
    return NULL;
  }

  return m_DiffFile[index];
}

unsigned long SelectableDiffFile::getNumLines() const
{
  return m_DiffFile.getNumLines();
}

void SelectableDiffFile::addSelection(unsigned long lineId, 
                                      unsigned long fromColumn, 
                                      unsigned long toColumn)
{
  m_Selection.addBlock(lineId, fromColumn, toColumn);
}

void SelectableDiffFile::clearSelection()
{
  m_Selection.clear();
}

long SelectableDiffFile::getNumNormalChars(unsigned long lineId, 
                                           unsigned long columnId)
{
  if(lineId >= m_DiffFile.getNumLines())
  {
    return 0;
  }

  if(columnId > m_DiffFile[lineId]->getNumChars())
  {
    return 0;
  }

  long numMarkedChars = m_Selection.getNumMarkedChars(lineId, columnId);
  if(numMarkedChars > 0)
  {
    return 0;
  }

  long nextSelStart = m_Selection.getNextSelectionStart(lineId, columnId);
  if(nextSelStart > 0)
  {
    return nextSelStart - columnId;
  }
  
  return m_DiffFile[lineId]->getNumChars() - columnId;
}


long SelectableDiffFile::getNumMarkedChars(unsigned long lineId, 
                                           unsigned long columnId)
{
  return m_Selection.getNumMarkedChars(lineId, columnId);
}

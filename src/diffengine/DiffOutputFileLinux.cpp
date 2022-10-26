
#include <string.h>


#include "DiffOutputFileLinux.h"

DiffOutputFileLinux::DiffOutputFileLinux(const DiffInputFileBase& diffInputFile)
  : DiffOutputFileBase(diffInputFile)
{
}


DiffOutputFileLinux::~DiffOutputFileLinux()
{
  std::vector<DiffLine*>::iterator it;
  for(it = m_Lines.begin(); it != m_Lines.end(); it++)
  {
    delete *it;
  }
}


long DiffOutputFileLinux::addLine(const char* pText,
                                  DiffLine::LineState lineState,
                                  const char* pFormattedLineNumber)
{
  DiffLine* pDiffLine = new DiffLine(pText, lineState, pFormattedLineNumber);
  m_Lines.push_back(pDiffLine);

  return m_Lines.size() - 1;
}

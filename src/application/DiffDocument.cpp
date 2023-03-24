#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
#endif

#include <exec/memory.h>

#include "AmigaFile.h"
#include "DiffDocument.h"
#include "DiffEngine.h"

DiffDocument::DiffDocument(const char* pLeftFilePath,
                           const char* pRightFilePath,
                           bool& isCancelRequested,
                           StopWatch& stopWatch,
                           ProgressReporter& progress,
                           bool lineNumbersEnabled,
                           bool ignoreTrailingSpaces,
                           bool countByLines)
  : m_LeftSrcFile(m_Pool.Header(),
                  isCancelRequested,
                  progress,
                  "Loading left file",
                  pLeftFilePath,
                  lineNumbersEnabled,
                  ignoreTrailingSpaces),
    m_RightSrcFile(m_Pool.Header(),
                   isCancelRequested,
                   progress,
                   "Loading right file",
                   pRightFilePath,
                   lineNumbersEnabled,
                   ignoreTrailingSpaces),
    m_LeftDiffFile(m_LeftSrcFile, m_Pool.Header()),
    m_RightDiffFile(m_RightSrcFile, m_Pool.Header()),
    m_DiffEngine(m_LeftSrcFile,
                 m_RightSrcFile,
                 m_LeftDiffFile,
                 m_RightDiffFile,
                 progress,
                 "Comparing the files",
                 isCancelRequested,
                 m_DiffIndices,
                 countByLines),
    m_LineNumbersEnabled(lineNumbersEnabled),
    m_DiffTime(0),
    m_MaxLineLength(0)
{
  m_LeftFileName = pLeftFilePath;
  m_RightFileName = pRightFilePath;  

}

DiffDocument::~DiffDocument()
{

}

bool DiffDocument::startCompare()
{
  return m_DiffEngine.startCompare();
}

void DiffDocument::setTabulatorSize(unsigned long tabulatorSize)
{
  // Determine the max. line length in chars. Can either be found in the
  // left or in the right file.
  unsigned long numChars;
  unsigned long renderColumn;
  for(size_t i = 0; i < m_LeftDiffFile.getNumLines(); i++)
  {
    numChars = m_LeftDiffFile[i]->getNumChars();
    renderColumn = m_LeftDiffFile[i]->getRenderColumn(numChars, tabulatorSize);
    if(renderColumn > m_MaxLineLength)
    {
      m_MaxLineLength = renderColumn;
    }
  }

  for(size_t i = 0; i < m_RightDiffFile.getNumLines(); i++)
  {
    numChars = m_RightDiffFile[i]->getNumChars();
    renderColumn = m_RightDiffFile[i]->getRenderColumn(numChars, tabulatorSize);
    if(renderColumn > m_MaxLineLength)
    {
      m_MaxLineLength = renderColumn;
    }
  }
}

void DiffDocument::setDiffTime(long diffTime)
{
  m_DiffTime = diffTime;
}

const char* DiffDocument::getLeftFileName() const
{
  return m_LeftFileName.c_str();
}

const char* DiffDocument::getRightFileName() const
{
  return m_RightFileName.c_str();
}

long DiffDocument::getElapsedDiffTime() const
{
  return m_DiffTime;
}

size_t DiffDocument::getNumDifferences() const
{
  return m_DiffEngine.getNumDifferences();
}

size_t DiffDocument::getNumAdded() const
{
  return m_DiffEngine.getNumAdded();
}

size_t DiffDocument::getNumChanged() const
{
  return m_DiffEngine.getNumChanged();
}

size_t DiffDocument::getNumDeleted() const
{
  return m_DiffEngine.getNumDeleted();
}


size_t DiffDocument::getNumLines() const
{

  // Note: Right diff file should have the same number of lines
  return m_LeftDiffFile.getNumLines();
}

size_t DiffDocument::getMaxLineLength() const
{
  return m_MaxLineLength;
}


const DiffOutputFileBase& DiffDocument::getLeftDiffFile() const
{
  return m_LeftDiffFile;
}


const DiffOutputFileBase& DiffDocument::getRightDiffFile() const
{
    return m_RightDiffFile;
}

const std::list<size_t>& DiffDocument::getDiffIndices() const
{
  return m_DiffIndices;
}


bool DiffDocument::areLineNumbersEnabled() const
{
  return m_LineNumbersEnabled;
}

bool DiffDocument::hasLeftFileDateChanged() const
{
  AmigaFile fileNow(m_LeftFileName.c_str());
  const struct DateStamp* pFileDateNow = fileNow.getDate();
  LONG compareResult = CompareDates(pFileDateNow, m_LeftSrcFile.getFileDate());
  return compareResult != 0;
}

bool DiffDocument::hasRightFileDateChanged() const
{
  AmigaFile fileNow(m_RightFileName.c_str());
  const struct DateStamp* pFileDateNow = fileNow.getDate();
  LONG compareResult = CompareDates(pFileDateNow, m_RightSrcFile.getFileDate());
  return compareResult != 0;
}

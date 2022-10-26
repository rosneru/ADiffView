#ifndef DIFF_DOCUMENT_H
#define DIFF_DOCUMENT_H

#include <list>
#include <string>

#include "DiffInputFileAmiga.h"
#include "DiffOutputFileAmiga.h"
#include "DiffEngine.h"
#include "DiffLine.h"
#include "MemoryPool.h"
#include "ProgressReporter.h"
#include "StopWatch.h"

/**
 * The document which holds the result of the compare operation and can
 * be displayed in a DiffWindow.
 * 
 * @author Uwe Rosner
 * @date 24/09/2018
 */
class DiffDocument
{
public:
  /**
   * Creating the document immediately starts comparing the files.
   */
  DiffDocument(const char* pLeftFilePath,
               const char* pRightFilePath,
               bool& isCancelRequested,
               StopWatch& stopWatch,
               ProgressReporter& progress,
               bool lineNumbersEnabled,
               bool ignoreTrailingSpaces,
               bool countByLines);

  virtual ~DiffDocument();

  /**
   * Start comparing pLeftFilePath and pRightFilePath
   */
  bool startCompare();

  /**
   * Set the tabulator size used to render this document.
   *
   * Internally this only calculates the max line length which is only
   * needed for to render the document properly. So this maybe should be
   * part of the view, not of this data object.
   *
   * TODO: Move to view.
   */
  void setTabulatorSize(unsigned long tabulatorSize);

  void setDiffTime(long diffTime);

  const char* getLeftFileName() const;
  const char* getRightFileName() const;
  
  long getElapsedDiffTime() const;

  size_t getNumDifferences() const;
  size_t getNumAdded() const;
  size_t getNumChanged() const;
  size_t getNumDeleted() const;

  size_t getNumLines() const;

  size_t getMaxLineLength() const;

  const DiffOutputFileBase& getLeftDiffFile() const;
  const DiffOutputFileBase& getRightDiffFile() const;

  const std::list<size_t>& getDiffIndices() const;

  bool areLineNumbersEnabled() const;

private:
  std::string m_LeftFileName;
  std::string m_RightFileName;
  MemoryPool m_Pool; // Ensure the pool is created before the DiffFiles
  DiffInputFileAmiga m_LeftSrcFile;
  DiffInputFileAmiga m_RightSrcFile;
  DiffOutputFileAmiga m_LeftDiffFile;
  DiffOutputFileAmiga m_RightDiffFile;

  std::list<size_t> m_DiffIndices;

  DiffEngine m_DiffEngine;

  bool m_LineNumbersEnabled;
  long m_DiffTime;
  size_t m_MaxLineLength;
};

#endif

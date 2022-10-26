#ifndef DIFF_OUTPUT_FILE_BASE_H
#define DIFF_OUTPUT_FILE_BASE_H

#include <string>

#include "DiffInputFileBase.h"

class DiffOutputFileBase : public DiffFileBase
{
public:
  DiffOutputFileBase(const DiffInputFileBase& diffInputFile);
  virtual ~DiffOutputFileBase();

  /**
   * Adds a DiffLine to file using given string and line state.
   * 
   * Returns the index where the line was inserted or -1 on error.
   */
  virtual long addLine(const char* pText, 
                       DiffLine::LineState lineState,
                       const char* pFormattedLineNumber) = 0;


  void addEmptyLine();

private:
  const char* m_pEmptyLine;

  /**
   * A string filled with spaces to the same text width as a line number
   * would be.
   */
  const std::string m_EmptyLineNumber;
};

#endif

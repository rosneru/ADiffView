#ifndef DIFF_OUTPUT_FILE_AMIGA_H
#define DIFF_OUTPUT_FILE_AMIGA_H

#include <exec/types.h>

#include "DiffOutputFileBase.h"
#include "ProgressReporter.h"

/**
 * Extends the DiffFilePartition::PreProcess with Amiga file loading
 * functionality
 *
 *
 * @author Uwe Rosner
 * @date 09/11/2018
 */
class DiffOutputFileAmiga : public DiffOutputFileBase
{
public:
  DiffOutputFileAmiga(const DiffInputFileBase& diffInputFile, 
                      APTR pPoolHeader);
  virtual ~DiffOutputFileAmiga();

  /**
   * Adds a DiffLine to file using given string and line state.
   * 
   * Returns the index where the line was inserted or -1 on error.
   */
  long addLine(const char* pText,
                 DiffLine::LineState lineState, 
                 const char* pFormattedLineNumber);

private:
  APTR m_pPoolHeader;
};

#endif // DIFF_FILE_PARTITION_AMIGA_H

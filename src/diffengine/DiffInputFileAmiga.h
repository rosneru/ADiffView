#ifndef DIFF_INPUT_FILE_AMIGA_H
#define DIFF_INPUT_FILE_AMIGA_H

#include <exec/types.h>

#include "DiffInputFileBase.h"
#include "ProgressReporter.h"

/**
 * Extends the DiffFilePartition::PreProcess with Amiga file loading
 * functionality
 *
 *
 * @author Uwe Rosner
 * @date 18/07/2020
 */
class DiffInputFileAmiga : public DiffInputFileBase
{
public:
  DiffInputFileAmiga(APTR pPoolHeader, 
                     bool& isCancelRequested, 
                     ProgressReporter& progress,
                     const char* pProgressDescription,
                     const char* pFileName,
                     bool lineNumbersEnabled);

  virtual ~DiffInputFileAmiga();


private:
  APTR m_pPoolHeader;
  char* m_pFileBuffer;

  void collectLineNumbers(size_t maxNumLines);
};

#endif // DIFF_FILE_PARTITION_AMIGA_H

#ifndef DIFF_OUTPUT_FILE_LINUX_H
#define DIFF_OUTPUT_FILE_LINUX_H

#include "DiffOutputFileBase.h"
#include "ProgressReporter.h"

/**
 *
 *
 * @author Uwe Rosner
 * @date 09/11/2018
 */
class DiffOutputFileLinux : public DiffOutputFileBase
{
public:
  DiffOutputFileLinux(const DiffInputFileBase& diffInputFile);
  virtual ~DiffOutputFileLinux();

  long addLine(const char* pText,
               DiffLine::LineState lineState, 
               const char* pFormattedLineNumber);

private:
  DiffLine* createDiffLine();
};

#endif

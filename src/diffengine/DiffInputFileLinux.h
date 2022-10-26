#ifndef DIFF_INPUT_FILE_LINUX_H
#define DIFF_INPUT_FILE_LINUX_H


#include "DiffInputFileBase.h"
#include "ProgressReporter.h"

/**
 *
 * @author Uwe Rosner
 * @date 18/07/2020
 */
class DiffInputFileLinux : public DiffInputFileBase
{
public:
  DiffInputFileLinux(bool& isCancelRequested, 
                     const char* pFileName,
                     bool lineNumbersEnabled,
                     bool ignoreTrailingSpaces = false);

  virtual ~DiffInputFileLinux();


private:
  char* m_pLineNumberBuffer;
  void collectLineNumbers(size_t maxNumLines);
};

#endif

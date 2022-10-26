#ifndef PROGRESS_REPORTER_LINUX_H
#define PROGRESS_REPORTER_LINUX_H

/**
 * This is an empty stub of the ProgressReporter to be able to build the
 * tests in Linux.
 *
 * @author Uwe Rosner
 * @date 03/08/2020
 */
class ProgressReporter
{
public:
  ProgressReporter();

  virtual void SetValue(int progress);
  void SetDescription(const char* pProgressDescription);

};

#endif

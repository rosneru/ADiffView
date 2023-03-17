#ifndef CMD_COMPARE_H
#define CMD_COMPARE_H

#include "DiffWorker.h"
#include "CommandBase.h"

/**
 * Command to start the DiffWorker that in a background task compares 
 * the files.
 *
 * On execute this command will perform the diff using the given
 * DiffWorker.
 *
 * @author Uwe Rosner
 * @date 26/10/2018
 */
class CmdCompare : public CommandBase
{
public:
  CmdCompare(std::vector<WindowBase*>* pAllWindowsVector,
             DiffWorker& diffWorker);

  virtual ~CmdCompare();
  virtual void Execute(struct Window* pActiveWindow);

private:
  DiffWorker& m_DiffWorker;
};

#endif

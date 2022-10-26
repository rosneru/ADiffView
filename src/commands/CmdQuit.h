#ifndef CMD_QUIT_H
#define CMD_QUIT_H

#include "CommandBase.h"

/**
 * Command for exiting the application. It takes an boolean reference
 * as second parameter. When Execute is called this reference variable
 * will be set to true so that the application in its loop can react
 * and quit.
 *
 * @author Uwe Rosner
 * @date 24/09/2018
 */
class CmdQuit : public CommandBase
{
public:
  CmdQuit(std::vector<WindowBase*>* pAllWindowsVector,
          bool& exitAllowed, 
          bool& exitRequested);
  
  virtual ~CmdQuit();

  virtual void Execute(struct Window* pActiveWindow);

private:
  bool& m_IsExitAllowed;
  bool& m_IsExitRequested;
};

#endif // CMD_QUIT_H

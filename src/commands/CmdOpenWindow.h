#ifndef CMD_OPEN_WINDOW_H
#define CMD_OPEN_WINDOW_H

#include "MenuBase.h"
#include "CommandBase.h"

/**
 * Command for opening a window.
 *
 * On Execute this will open the provided window and attach the provided
 * menu to it
 *
 * @author Uwe Rosner
 * @date 24/10/2018
 */
class CmdOpenWindow : public CommandBase
{
public:
  CmdOpenWindow(std::vector<WindowBase*>* pAllWindowsVector,
                WindowBase& window);

  virtual ~CmdOpenWindow();
  virtual void Execute(struct Window* pActiveWindow);

private:
  WindowBase& m_Window;
};

#endif // CMD_OPEN_WINDOW_H

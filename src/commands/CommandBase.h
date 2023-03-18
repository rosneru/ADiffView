#ifndef COMMAND_H
#define COMMAND_H

#include <intuition/intuition.h>

#include <vector>
#include "WindowBase.h"

#ifdef Execute
  #undef Execute
#endif

/**
 * Base class of all commands. Is an abstract class because of the
 * abstract Execute() method.
 *
 * Derived concrete command objects are mainly wired to the app menu.
 * On menu pick event the Execute() method is called.
 *
 * @author Uwe Rosner
 * @date 24/09/2018
 */
class CommandBase
{
public:
  CommandBase(std::vector<WindowBase*>* pAllWindowsVector);
  virtual ~CommandBase();
  virtual void Execute(struct Window* pActiveWindow) = 0;

  void EnableInAllWindowMenus() const;
  void DisableInAllWindowMenus() const;

protected:
  static std::vector<WindowBase*>* m_pAllWindowsVector;

  CommandBase();

  void enableBusyPointerForAllWindows();
  void disableBusyPointerForAllWindows();

private:
    struct Requester m_SleepRequester;
};

#endif

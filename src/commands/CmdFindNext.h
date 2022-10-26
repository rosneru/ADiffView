#ifndef CMD_FIND_NEXT_H
#define CMD_FIND_NEXT_H

#include "TextFinder.h"
#include "CommandBase.h"

/**
 * Command to display the next search result (Next from the current
 * y-scroll position of the text in DiffWindow)
 *
 * @author Uwe Rosner
 * @date 12/06/2021
 */
class CmdFindNext : public CommandBase
{
public:
  CmdFindNext(std::vector<WindowBase*>* pAllWindowsVector,
              TextFinder& textFinder);

  virtual ~CmdFindNext();
  virtual void Execute(struct Window* pActiveWindow);

private:
  TextFinder& m_TextFinder;
};

#endif

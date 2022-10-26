#ifndef CMD_NAVIGATE_NEXT_DIFF_H
#define CMD_NAVIGATE_NEXT_DIFF_H

#include "DiffWindow.h"
#include "CmdNavigateBase.h"
#include "CmdWindowTitleQuickMsg.h"

/**
 * Command to display ('jump to') the next diff block in DiffWindow.
 * Next diff block is the next one after current scroll position of the
 * text in DiffWindow.
 *
 * @author Uwe Rosner
 * @date 14/12/2019
 */
class CmdNavigateNextDiff : public CmdNavigateBase
{
public:
  CmdNavigateNextDiff(std::vector<WindowBase*>* pAllWindowsVector,
                      DiffWorker& diffWorker,
                      DiffWindow& diffWindow,
                      CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg);

  virtual ~CmdNavigateNextDiff();
  virtual void Execute(struct Window* pActiveWindow);
};

#endif

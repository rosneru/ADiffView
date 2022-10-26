#ifndef CMD_NAV_PREV_DIFF_H
#define CMD_NAV_PREV_DIFF_H

#include "DiffWindow.h"
#include "CmdNavigateBase.h"
#include "CmdWindowTitleQuickMsg.h"

/**
 * Command to display ('jump to') the previous diff block in DiffWindow.
 * Previous diff block is the next one before current scroll position of
 * the text in DiffWindow.
 *
 * @author Uwe Rosner
 * @date 14/12/2019
 */
class CmdNavigatePrevDiff : public CmdNavigateBase
{
public:
  CmdNavigatePrevDiff(std::vector<WindowBase*>* pAllWindowsVector,
                      DiffWorker& diffWorker,
                      DiffWindow& diffWindow,
                      CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg);

  virtual ~CmdNavigatePrevDiff();
  virtual void Execute(struct Window* pActiveWindow);

};

#endif

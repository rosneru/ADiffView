#ifndef CMD_NAV_LAST_DIFF_H
#define CMD_NAV_LAST_DIFF_H

#include "DiffWindow.h"
#include "CmdNavigateBase.h"
#include "CmdWindowTitleQuickMsg.h"

/**
 * Command to display ('jump to') the last diff block in DiffWindow.
 *
 * @author Uwe Rosner
 * @date 17/06/2021
 */
class CmdNavigateLastDiff : public CmdNavigateBase
{
public:
  CmdNavigateLastDiff(std::vector<WindowBase*>* pAllWindowsVector,
                      DiffWorker& diffWorker,
                      DiffWindow& diffWindow,
                      CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg);

  virtual ~CmdNavigateLastDiff();
  virtual void Execute(struct Window* pActiveWindow);

};

#endif

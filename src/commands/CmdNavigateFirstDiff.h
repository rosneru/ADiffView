#ifndef CMD_NAV_FIRST_DIFF_H
#define CMD_NAV_FIRST_DIFF_H

#include "DiffWindow.h"
#include "CmdNavigateBase.h"
#include "CmdWindowTitleQuickMsg.h"

/**
 * Command to display ('jump to') the first diff block in DiffWindow.
 *
 * @author Uwe Rosner
 * @date 17/06/2021
 */
class CmdNavigateFirstDiff : public CmdNavigateBase
{
public:
  CmdNavigateFirstDiff(std::vector<WindowBase*>* pAllWindowsVector,
                       DiffWorker& diffWorker,
                       DiffWindow& diffWindow,
                       CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg);

  virtual ~CmdNavigateFirstDiff();
  virtual void Execute(struct Window* pActiveWindow);

};

#endif

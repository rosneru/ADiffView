#ifndef MENU_DIFF_WINDOW_H
#define MENU_DIFF_WINDOW_H

#include "ADiffViewArgs.h"
#include "CommandBase.h"
#include "MenuBase.h"

class MenuDiffWindow : public MenuBase
{
public:
  MenuDiffWindow(CommandBase* pCmdOpenFilesWindow,
                 CommandBase* pCmdAboutRequester,
                 CommandBase* pCmdQuit,
                 CommandBase* pCmdCopySelection,
                 CommandBase* pCmdNavigateFirstDiff,
                 CommandBase* pCmdNavigatePrevDiff,
                 CommandBase* pCmdNavigateNextDiff,
                 CommandBase* pCmdNavigateLastDiff,
                 CommandBase* pCmdOpenSearchWindow,
                 CommandBase* pCmdFindNext,
                 CommandBase* pCmdFindPrev,
                 CommandBase* pCmdTabWidth2,
                 CommandBase* pCmdTabWidth4,
                 CommandBase* pCmdTabWidth8,
                 CommandBase* pCmdTabWidthCustom,
                 const ADiffViewArgs& args);

private:
  char m_CustomTabWidth[32];
};

#endif

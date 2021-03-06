#ifdef __clang__
  #include <clib/gadtools_protos.h>
#else
  #include <proto/gadtools.h>
#endif

#include "MenuDiffWindow.h"

MenuDiffWindow::MenuDiffWindow(CommandBase* pCmdOpenFilesWindow,
                               CommandBase* pCmdAboutRequester,
                               CommandBase* pCmdQuit,
                               CommandBase* pCmdNavigateFirstDiff,
                               CommandBase* pCmdNavigatePrevDiff,
                               CommandBase* pCmdNavigateNextDiff,
                               CommandBase* pCmdNavigateLastDiff,
                               CommandBase* pCmdOpenSearchWindow,
                               CommandBase* pCmdFindNext,
                               CommandBase* pCmdFindPrev)
{
  struct NewMenu newMenu[] = 
  {
    { NM_TITLE,   "Project",                 0 , 0, 0, 0 },
    {   NM_ITEM,    "Open...",              "o", 0, 0, pCmdOpenFilesWindow },
    {   NM_ITEM,    "About...",              0 , 0, 0, pCmdAboutRequester },
    {   NM_ITEM,    NM_BARLABEL,             0 , 0, 0, 0 },
    {   NM_ITEM,    "Quit",                 "q", 0, 0, pCmdQuit },
    { NM_TITLE,   "Navigate",                0 , 0, 0, 0 },
    {   NM_ITEM,    "First difference",     "1", 0, 0, pCmdNavigateFirstDiff },
    {   NM_ITEM,    "Previous difference",  "2", 0, 0, pCmdNavigatePrevDiff },
    {   NM_ITEM,    "Next difference",      "3", 0, 0, pCmdNavigateNextDiff },
    {   NM_ITEM,    "Last difference",      "4", 0, 0, pCmdNavigateLastDiff },
    {   NM_ITEM,    NM_BARLABEL,             0 , 0, 0, 0 },
    {   NM_ITEM,    "Find...",              "f", 0, 0, pCmdOpenSearchWindow },
    {   NM_ITEM,    "Find prev",            "p", 0, 0, pCmdFindPrev },
    {   NM_ITEM,    "Find next",            "n", 0, 0, pCmdFindNext },
    { NM_END,     NULL,                      0 , 0, 0, 0 },
  };

  m_pMenu = CreateMenus(newMenu, TAG_DONE);
  if(m_pMenu == NULL)
  {
    throw "Failed to create the menu for DiffWindow.";
  }
}

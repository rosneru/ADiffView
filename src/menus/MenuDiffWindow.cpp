#ifdef __clang__
  #include <clib/gadtools_protos.h>
#else
  #include <proto/gadtools.h>
#endif

#include <stdio.h>

#include "MenuDiffWindow.h"

MenuDiffWindow::MenuDiffWindow(CommandBase* pCmdOpenFilesWindow,
                               CommandBase* pCmdAboutRequester,
                               CommandBase* pCmdQuit,
                               CommandBase* pCmdCopySelection,
                               CommandBase* pCmdEditLeftFile,
                               CommandBase* pCmdEditRightFile,
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
                               const ADiffViewArgs& args)
  :   m_pCmdCopySelection(pCmdCopySelection),
      m_pCmdFindNext(pCmdFindNext),
      m_pCmdFindPrev(pCmdFindPrev)
{
  // Create the text for the custom tab width menu entry
  sprintf(m_CustomTabWidth, "Custom: %ld", args.getTabSize());

  // define 4 variables for the "Tabulator width" menu and set one of
  // them to CHECKED depending on the current tab size (read from args).
  // Set the others to 0.
  UWORD is2CheckedValue = args.getTabSize() == 2 ? CHECKED : 0;
  UWORD is4CheckedValue = args.getTabSize() == 4 ? CHECKED : 0;
  UWORD is8CheckedValue = args.getTabSize() == 8 ? CHECKED : 0;
  UWORD isCustomCheckedValue = 0;
  if((is2CheckedValue + is4CheckedValue + is8CheckedValue) == 0)
  {
    isCustomCheckedValue = CHECKED;
  }

  is2CheckedValue = is2CheckedValue|CHECKIT|MENUTOGGLE;
  is4CheckedValue = is4CheckedValue|CHECKIT|MENUTOGGLE;
  is8CheckedValue = is8CheckedValue|CHECKIT|MENUTOGGLE;
  isCustomCheckedValue = isCustomCheckedValue|CHECKIT|MENUTOGGLE;

  // The finalizing NewMenu entry will be added separately
  struct NewMenu newMenuFinalizingTag = {NM_END, NULL, 0 , 0, 0, 0};

  // Create an array of NewMenu structs to define the menu layout
  struct NewMenu newMenu[] = 
  {
    { NM_TITLE,   "Project",                    0 , 0, 0, 0 },
    {   NM_ITEM,    "Open...",                 "o", 0, 0, pCmdOpenFilesWindow },
    {   NM_ITEM,    "About...",                 0 , 0, 0, pCmdAboutRequester },
    {   NM_ITEM,    NM_BARLABEL,                0 , 0, 0, 0 },
    {   NM_ITEM,    "Quit",                    "q", 0, 0, pCmdQuit },
    { NM_TITLE,   "Edit",                       0 , 0, 0, 0 },
    {   NM_ITEM,    "Copy",                    "c", 0, 0, pCmdCopySelection },
    {   NM_ITEM,    NM_BARLABEL,                0 , 0, 0, 0 },
    {   NM_ITEM,    "Edit left file",          "l", 0, 0, pCmdEditLeftFile },
    {   NM_ITEM,    "Edit right file",         "r", 0, 0, pCmdEditRightFile },
    { NM_TITLE,   "Navigation",                 0 , 0, 0, 0 },
    {   NM_ITEM,    "First difference",        "1", 0, 0, pCmdNavigateFirstDiff },
    {   NM_ITEM,    "Previous difference",     "2", 0, 0, pCmdNavigatePrevDiff },
    {   NM_ITEM,    "Next difference",         "3", 0, 0, pCmdNavigateNextDiff },
    {   NM_ITEM,    "Last difference",         "4", 0, 0, pCmdNavigateLastDiff },
    {   NM_ITEM,    NM_BARLABEL,                0 , 0, 0, 0 },
    {   NM_ITEM,    "Find...",                 "f", 0, 0, pCmdOpenSearchWindow },
    {   NM_ITEM,    "Find prev",               "p", 0, 0, pCmdFindPrev },
    {   NM_ITEM,    "Find next",               "n", 0, 0, pCmdFindNext },
    { NM_TITLE,   "View",                       0 , 0, 0, 0 },
    {   NM_ITEM,    "Tabulator width",          0, 0, 0, 0 },
    {     NM_SUB,    "2",                       0, is2CheckedValue,      ~1, pCmdTabWidth2 },
    {     NM_SUB,    "4",                       0, is4CheckedValue,      ~2, pCmdTabWidth4 },
    {     NM_SUB,    "8",                       0, is8CheckedValue,      ~4, pCmdTabWidth8 },
    {     NM_SUB,    (STRPTR)m_CustomTabWidth,  0, isCustomCheckedValue, ~8, pCmdTabWidthCustom },
    newMenuFinalizingTag
  };

  // Get size of the array of NewMenu structs
  size_t numNewMenuItems = sizeof(newMenu) / sizeof(newMenu[0]);

  /** When the custom value is not checked at menu creation time
   * (Application start) then it is not used and the appropriate field
   * is not needed. It is removed from the NewMenu array by setting the
   * NewMenu finalizing line into this array item. 
   *
   * NOTE: This is a dirty hack as it won't work when items are added
   * below the m_CustomTabWidth line.
   * 
   * TODO: Find a better solution.
   */
  if(isCustomCheckedValue != CHECKED)
  {
    newMenu[numNewMenuItems - 2] = newMenuFinalizingTag;
  }

  m_pMenu = CreateMenus(newMenu, TAG_DONE);
  if(m_pMenu == NULL)
  {
    throw "Failed to create the menu for DiffWindow.";
  }
}

CommandBase* MenuDiffWindow::getCmdCopySelection()
{
  return m_pCmdCopySelection;
}

CommandBase* MenuDiffWindow::getCmdFindNext()
{
  return m_pCmdFindNext;
}

CommandBase* MenuDiffWindow::getCmdFindPrev()
{
  return m_pCmdFindPrev;
}

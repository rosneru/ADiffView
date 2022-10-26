/* Application.h                                                       */
/*-------------------------------------------------------------------*/
/* Main application class                                            */
/*                                                                   */
/* Uwe Rosner                                                        */
/* Created on 23/09/2018                                             */
/*-------------------------------------------------------------------*/
#ifndef APPLICATION
#define APPLICATION

#include <workbench/workbench.h>

#include <string>
#include <vector>

#include "ADiffViewArgs.h"
#include "ADiffViewPens.h"
#include "ADiffViewPorts.h"
#include "ADiffViewSettings.h"
#include "MenuFilesWindow.h"
#include "MenuDiffWindow.h"
#include "OpenClonedWorkbenchScreen.h"
#include "OpenJoinedPublicScreen.h"
#include "CmdChangeTabulatorWidth.h"
#include "CmdCloseWindow.h"
#include "CmdDiff.h"
#include "CmdFindPrev.h"
#include "CmdFindNext.h"
#include "CmdNavigateFirstDiff.h"
#include "CmdNavigateLastDiff.h"
#include "CmdNavigateNextDiff.h"
#include "CmdNavigatePrevDiff.h"
#include "CmdOpenWindow.h"
#include "CmdQuit.h"
#include "CmdRequester.h"
#include "CmdWindowTitleQuickMsg.h"
#include "TextFinder.h"
#include "DiffWindow.h"
#include "DiffWorker.h"
#include "FilesWindow.h"
#include "ProgressWindow.h"
#include "SearchWindow.h"

class Application
{
public:
  Application(ScreenBase& screen, 
              const ADiffViewArgs& args, 
              ADiffViewSettings& settings);
  virtual ~Application();

  void Run();

private:
  ScreenBase& m_Screen;
  const ADiffViewArgs& m_Args;
  ADiffViewSettings& m_Settings;

  std::string m_LeftFilePath;
  std::string m_RightFilePath;

  bool m_IsCancelRequested;
  bool m_IsExitRequested;
  bool m_IsExitAllowed;
  bool m_IsAppWindow;
  bool m_IsAppIcon;

  ADiffViewPens m_Pens;
  MenuDiffWindow m_DiffWindowMenu;
  MenuFilesWindow m_FilesWindowMenu;
  ADiffViewPorts m_Ports;
  DiffWorker m_DiffWorker;
  DiffWindow m_DiffWindow;
  FilesWindow m_FilesWindow;
  ProgressWindow m_ProgressWindow;
  SearchWindow m_SearchWindow;
  TextFinder m_TextFinder;
  CmdDiff m_CmdDiff;
  CmdFindPrev m_CmdFindPrev;
  CmdFindNext m_CmdFindNext;
  CmdNavigateFirstDiff m_CmdNavigateFirstDiff;
  CmdNavigatePrevDiff m_CmdNavigatePrevDiff;
  CmdNavigateNextDiff m_CmdNavigateNextDiff;
  CmdNavigateLastDiff m_CmdNavigateLastDiff;
  CmdQuit m_CmdQuit;
  CmdOpenWindow m_CmdOpenFilesWindow;
  CmdCloseWindow m_CmdCloseFilesWindow;
  CmdOpenWindow m_CmdOpenSearchWindow;
  CmdCloseWindow m_CmdCloseSearchWindow;
  CmdRequester m_CmdAboutRequester;
  CmdWindowTitleQuickMsg m_CmdWindowTitleQuickMsg;
  CmdChangeTabulatorWidth m_CmdTabWidth2;
  CmdChangeTabulatorWidth m_CmdTabWidth4;
  CmdChangeTabulatorWidth m_CmdTabWidth8;
  CmdChangeTabulatorWidth m_CmdTabWidthCustom;
  std::string m_AboutMsg;

  std::vector<WindowBase*> m_AllWindowsList;

  struct AppIcon* m_pAppIcon;

  /**
   * Handling messages from Intuition
   */
  void intuiEventLoop();

  void handleAppWindowMessages();
  void handleProgressMessages();
  void handleIdcmpMessages();
};


#endif

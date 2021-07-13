#ifdef __clang__
  #include <clib/gadtools_protos.h>
  #include <clib/icon_protos.h>
  #include <clib/intuition_protos.h>
  #include <clib/wb_protos.h>
#else
  #include <proto/gadtools.h>
  #include <proto/icon.h>
  #include <proto/intuition.h>
  #include <proto/wb.h>
#endif

#include <libraries/gadtools.h>

#include <stdio.h>

#include "ADiffView_rev.h"
#include "CommandBase.h"
#include "ProgressMessage.h"

#include "Application.h"

Application::Application(ScreenBase& screen, 
                         ADiffViewArgs& args, 
                         ADiffViewSettings& settings)
  : m_Screen(screen),
    m_Args(args),
    m_Settings(settings),
    m_LeftFilePath(args.LeftFile()),   // copy (not reference) to member
    m_RightFilePath(args.RightFile()), // copy (not reference) to member
    m_IsCancelRequested(false),
    m_IsExitRequested(false),
    m_IsExitAllowed(true),
    m_IsAppWindow(m_Args.PubScreenName() == "Workbench"),
    m_IsAppIcon(!m_IsAppWindow && !m_Args.NoAppIcon()),
    m_Pens(screen, settings),
    m_DiffWindowMenu(&m_CmdOpenFilesWindow, 
                     &m_CmdAboutRequester, 
                     &m_CmdQuit,
                     &m_CmdNavigateFirstDiff,
                     &m_CmdNavigatePrevDiff,
                     &m_CmdNavigateNextDiff,
                     &m_CmdNavigateLastDiff,
                     &m_CmdOpenSearchWindow,
                     &m_CmdFindNext,
                     &m_CmdFindPrev),
    m_FilesWindowMenu(&m_CmdOpenFilesWindow, 
                      &m_CmdAboutRequester, 
                      &m_CmdQuit),
    m_DiffWorker(m_LeftFilePath,
                 m_RightFilePath,
                 m_DiffWindow,
                 m_ProgressWindow,
                 m_CmdOpenFilesWindow,
                 m_CmdCloseFilesWindow,
                 m_Ports.Progress(),
                 m_IsCancelRequested,
                 m_IsExitAllowed,
                 m_Args.ShowLineNumbers(),
                 m_Args.getTabSize()),
    m_DiffWindow(screen,
                 m_Pens,
                 m_Ports.Idcmp(),
                 &m_DiffWindowMenu,
                 m_Args.getTabSize()),
    m_FilesWindow(m_AllWindowsList,
                  screen,
                  m_Ports.Idcmp(),
                  m_LeftFilePath,
                  m_RightFilePath,
                  m_CmdDiff,
                  m_CmdCloseFilesWindow,
                  &m_FilesWindowMenu),
    m_ProgressWindow(screen,
                     m_Pens,
                     m_Ports.Idcmp(),
                     m_IsCancelRequested,
                     NULL),
    m_SearchWindow(m_AllWindowsList,
                   screen,
                   m_Ports.Idcmp(),
                   m_TextFinder,
                   m_CmdCloseSearchWindow),
    m_TextFinder(m_DiffWorker, m_DiffWindow, m_Args.getTabSize()),
    m_CmdDiff(&m_AllWindowsList, m_DiffWorker),
    m_CmdFindPrev(&m_AllWindowsList, m_TextFinder),
    m_CmdFindNext(&m_AllWindowsList, m_TextFinder),
    m_CmdNavigateFirstDiff(&m_AllWindowsList, m_DiffWorker, m_DiffWindow),
    m_CmdNavigatePrevDiff(&m_AllWindowsList, m_DiffWorker, m_DiffWindow),
    m_CmdNavigateNextDiff(&m_AllWindowsList, m_DiffWorker, m_DiffWindow),
    m_CmdNavigateLastDiff(&m_AllWindowsList, m_DiffWorker, m_DiffWindow),
    m_CmdQuit(&m_AllWindowsList, m_IsExitAllowed, m_IsExitRequested),
    m_CmdOpenFilesWindow(&m_AllWindowsList, m_FilesWindow),
    m_CmdCloseFilesWindow(&m_AllWindowsList, m_CmdOpenFilesWindow, m_FilesWindow),
    m_CmdOpenSearchWindow(&m_AllWindowsList, m_SearchWindow),
    m_CmdCloseSearchWindow(&m_AllWindowsList, m_CmdOpenSearchWindow, m_SearchWindow),
    m_CmdAboutRequester(&m_AllWindowsList, m_AboutMsg, "About", "Ok"),
    m_pAppIcon(NULL)
{
  //
  // Note: VERSTAG above has been created with bumprev and is defined
  // in included file ADiffView_rev.h. It contains the program name
  // and version. Referencing it also means that the c:version command will
  // work for ADiffView.
  //

  // Create the message to be displayed in the about dialog
  m_AboutMsg = VERSTAG + 7;   // Skip the first 7 chars of pVersTag
                              // which is only "\0$VER: "
  m_AboutMsg += "\n\n";
  m_AboutMsg += "Copyright(c) 2021 Uwe Rosner (u.rosner@ymail.com)";
  m_AboutMsg += "\n\n";
  m_AboutMsg += "This release of ADiffView may be freely distributed.\n";
  m_AboutMsg += "It may not be commercially distributed without the\n";
  m_AboutMsg += "explicit permission of the author.\n";

  // Add all windows to the array
  m_AllWindowsList.push_back(&m_DiffWindow);
  m_AllWindowsList.push_back(&m_FilesWindow);
  m_AllWindowsList.push_back(&m_ProgressWindow);
  m_AllWindowsList.push_back(&m_SearchWindow);

  // Create a MessagePort for Workbench app messages if needed
  if(m_IsAppWindow || m_IsAppIcon)
  {
    if(m_IsAppWindow)
    {
      // FilesWindow should be an AppWindow
      m_FilesWindow.enableAppWindow(m_Ports.Workbench(), 0L);
    }

    if(m_IsAppIcon && m_Args.getDiscObject() != NULL)
    {
      m_pAppIcon = AddAppIcon(0L,
                              0L,
                              (UBYTE*)"ADiffView",
                              m_Ports.Workbench(),
                              0,
                              m_Args.getDiscObject(),
                              TAG_DONE);
    }
  }


  m_FilesWindow.setMenu(&m_FilesWindowMenu);
  m_DiffWindow.setMenu(&m_DiffWindowMenu);

  if((m_LeftFilePath.length() > 0) &&
     (m_RightFilePath.length() > 0) &&
     (m_Args.DontAsk() == true))
  {
    //
    // The DONTASK argument has been set and left and right file are
    // also given: Start the diff immediately
    //
    m_CmdDiff.Execute(NULL);
  }
  else
  {
    //
    // Fallback / default: Open the FilesWindow
    //
    // Passing a pointer to the command "Open files" to disable that
    // menu item at opening.
    m_CmdOpenFilesWindow.Execute(NULL);
  }
}


Application::~Application()
{
  if(m_pAppIcon != NULL)
  {
    RemoveAppIcon(m_pAppIcon);
    m_pAppIcon = NULL;
  }

  // Ensure that all windows are closed
  std::vector<WindowBase*>::iterator it;
  for(it = m_AllWindowsList.begin(); it != m_AllWindowsList.end(); it++)
  {
    (*it)->close();
  }
}


void Application::Run()
{
  //
  // The main event loop
  //
  intuiEventLoop();
}


void Application::intuiEventLoop()
{
  ULONG sigIDCMP = (1ul << m_Ports.Idcmp()->mp_SigBit);
  ULONG sigProgress = (1ul << m_Ports.Progress()->mp_SigBit);

  ULONG sigAppWin = 0;
  if(m_IsAppWindow || m_IsAppIcon)
  {
    sigAppWin = (1ul << m_Ports.Workbench()->mp_SigBit);
  }

  ULONG signals = sigIDCMP | sigProgress | sigAppWin;

  do
  {
    const ULONG received = Wait(signals);

    if(received & sigAppWin)
    {
      handleAppWindowMessages();
    }

    if(received & sigProgress)
    {
      handleProgressMessages();
    }

    if(received & sigIDCMP)
    {
      handleIdcmpMessages();
    }

    // Check all screens for any open windows
    if(m_Screen.NumOpenWindows() < 1)
    {
      // Exitting as there are no windows open anymore.
      m_CmdQuit.Execute(NULL); // Only sets m_IsExitRequested to true.
    }
  }
  while(!m_IsExitRequested);
}


void Application::handleAppWindowMessages()
{
  struct AppMessage* pAppMsg = NULL;
  while((pAppMsg = (struct AppMessage*)
    GetMsg(m_Ports.Workbench())) != NULL)
  {
    // When the files window is open it will handle the app message
    m_FilesWindow.HandleAppMessage(pAppMsg);

    // But when its an AppIcon message, always bring the screen to front
    if(m_IsAppIcon)
    {
      m_Screen.ToFront();
    }

    // All messages must be replied
    ReplyMsg((struct Message*)pAppMsg);
  }
}


void Application::handleProgressMessages()
{
  struct ProgressMessage* pProgressMsg = NULL;
  while((pProgressMsg = (struct ProgressMessage*)
    GetMsg(m_Ports.Progress())) != NULL)
  {
    m_ProgressWindow.HandleProgress(pProgressMsg);

    // All messages must be replied
    ReplyMsg(pProgressMsg);
  }
}


void Application::handleIdcmpMessages()
{
  struct IntuiMessage* pIdcmpMsg = NULL;
  while ((pIdcmpMsg = GT_GetIMsg(m_Ports.Idcmp())) != NULL)
  {
    // Get all data we need from message
    ULONG msgClass = pIdcmpMsg->Class;
    UWORD msgCode = pIdcmpMsg->Code;
    APTR msgIAddress = pIdcmpMsg->IAddress;
    struct Window* pMsgWindow = pIdcmpMsg->IDCMPWindow;

    // When we're through with a message, reply
    GT_ReplyIMsg(pIdcmpMsg);

    if(msgClass == IDCMP_MENUPICK)
    {
      //
      // getMenu-pick messages are handled here
      //
      UWORD menuNumber = msgCode;
      struct MenuItem* pSelectedItem = NULL;

      // Create an array of all menus to be searched for the item
      MenuBase* pMenus[] = {&m_FilesWindowMenu, &m_DiffWindowMenu};

      // Iterate all those menus, trying to find the item
      for(size_t i = 0; i < (sizeof pMenus / sizeof pMenus[0]); i++)
      {
        pSelectedItem = ItemAddress(pMenus[i]->IntuiMenu(), menuNumber);
        if(pSelectedItem != NULL)
        {
          // Item found
          break;
        }
      }

      if(pSelectedItem != NULL)
      {
        // Getting the user data from selected menu item
        APTR pUserData = GTMENUITEM_USERDATA(pSelectedItem);
        if(pUserData != NULL)
        {
          // The menu user_data contains a pointer to a Command
          CommandBase* pSelectedCommand = static_cast<CommandBase*>(pUserData);

          // Execute this command
          pSelectedCommand->Execute(pMsgWindow);
        }
      }

      return;
    }

    // All other idcmp messages are handled in the appropriate window
    std::vector<WindowBase*>::iterator it;
    for(it = m_AllWindowsList.begin(); it != m_AllWindowsList.end(); it++)
    {
      if(pMsgWindow == (*it)->getIntuiWindow())
      {
        (*it)->handleIDCMP(msgClass, msgCode, msgIAddress);
      }
    }
  }
}

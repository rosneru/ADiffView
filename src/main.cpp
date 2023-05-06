/**
 * ADiffView
 *
 * A simple graphical viewer for Differences in ASCII text files.
 *
 * Running on AMIGA computers with at least OS 3.0 / v39.
 *
 * DEVELOPMENT, TESTING, DEBUGGING, BUILD
 *     NDK for AmigaOS 3.1
 *     StormC 4
 *
 * CROSS DEVELOPMENT
 *     Linux
 *     Amiga OS toolchain from https://github.com/bebbo
 *     Visual Studio Code
 *
 * Author: Uwe Rosner
 *    https://github.com/rosneru
 */


#define INTUI_V36_NAMES_ONLY


#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
#endif

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuitionbase.h>
#include <workbench/startup.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "ADiffViewArgs.h"
#include "ADiffViewSettings.h"
#include "ADiffView_rev.h"
#include "Application.h"
#include "OpenClonedWorkbenchScreen.h"
#include "OpenJoinedPublicScreen.h"
#include "MessageBox.h"
#include "OpenScreenBase.h"


extern struct IntuitionBase* IntuitionBase;


/**
 * CLI entry point
 */
int main(int argc, char **argv)
{
  MessageBox request;

  // Check if the OS version is at least v39 / OS 3.0; return otherwise
  if(IntuitionBase->LibNode.lib_Version < 39)
  {
    request.Show("This program needs at least OS 3.0 / v39 to run.",
                 "Ok");
    return RETURN_FAIL;
  }

  ScreenBase* pScreen = NULL;
  Application* pApplication = NULL;

  ULONG exitCode = RETURN_OK;

  try
  {
    // Parse the command line or Workbench start arguments
    ADiffViewArgs args(argc, argv);

    // Create a settings instance
    ADiffViewSettings settings;

    // Create (and open) the screen depending on args
    if(args.getPubScreenName().length() > 0)
    {
      // Use a given public screen
      pScreen = new OpenJoinedPublicScreen(settings, args.getPubScreenName().c_str());
    }
    else
    {
      // Clone the Workbench screen onto an own public screen but with
      // only 8 colors
      pScreen = new OpenClonedWorkbenchScreen(settings, VERS, "ADIFFVIEW", 3);
      const OpenClonedWorkbenchScreen* pPublicScreen = 
        static_cast<const OpenClonedWorkbenchScreen*>(pScreen);
      if(pPublicScreen != NULL)
      {
        args.setPubScreenName(pPublicScreen->getPubScreenName());
      }
    }

    // Create and run the application
    pApplication = new Application(*pScreen, args, settings);
    pApplication->Run();
  }
  catch(const char* pMsg)
  {
    std::string msgString = pMsg;
    request.Show("ADiffView internal error",
                 pMsg,
                 "Exit");

    exitCode = RETURN_FAIL;
  }

  if(pApplication != NULL)
  {
    delete pApplication;
  }

  if(pScreen != NULL)
  {
    bool isScreenClosed = false;
    while(!isScreenClosed)
    {
      isScreenClosed = pScreen->close();
      if(isScreenClosed)
      {
        break;
      }

      request.Show("Failed to close screen",
                     "ADiffView failed to close its public screen.\n\n"
                     "Please close all windows on ADiffView screen and "
                     "try again to close it.",
                   "Close screen");
    };

    delete pScreen;
  }
  
  return exitCode;
}


/**
 * Workbench entry point.
 */
void wbmain(struct WBStartup* wb)
{
  // Call the CLI entry point with argc=0
  main(0, (char **) wb);
}

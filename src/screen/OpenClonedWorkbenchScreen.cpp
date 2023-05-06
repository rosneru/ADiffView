#ifdef __clang__
  #include <clib/gadtools_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/gadtools.h>
  #include <proto/intuition.h>
#endif

#include <stdio.h>
#include "OpenClonedWorkbenchScreen.h"

OpenClonedWorkbenchScreen::OpenClonedWorkbenchScreen(const char* pTitle,
                                                     const char* pPubScreenName,
                                                     short depth,
                                                     const ULONG* pColors32Array)
  : m_pTitle(pTitle),
    m_Depth(depth)
{
  // Trying to find a public screen name that isn't already in use
  char numBuf[3];
  Screen* pScreenAlreadyInUse;
  for(int i=1; i < 100; i++)
  {
    sprintf(numBuf, "%d", i);
    m_PubScreenName = pPubScreenName;
    m_PubScreenName += ".";
    m_PubScreenName += numBuf;
    pScreenAlreadyInUse = LockPubScreen(m_PubScreenName.c_str());
    if(pScreenAlreadyInUse == NULL)
    {
      // A public screen with this name could not be locked. So this
      // name is free to be used for this public screen.
      break;
    }

    UnlockPubScreen(NULL, pScreenAlreadyInUse);
    if(i == 99)
    {
      throw "Failed to find an unused name for public screen.";
    }
  }
  

  //
  // Opening a nearly-copy of the Workbench screen
  //
  // This is only an approximate copy, as it'll have 8 colors instead
  // of how many the Workbench screen might have.
  //
  m_pIntuiScreen = OpenScreenTags(NULL,
                                  SA_LikeWorkbench, TRUE,
                                  SA_Type, CUSTOMSCREEN,
                                  SA_Depth, (ULONG)m_Depth,
                                  SA_SharePens,TRUE,
                                  SA_Title, (ULONG)m_pTitle,
                                  SA_Colors32, (ULONG)pColors32Array,
                                  SA_PubName, (ULONG)m_PubScreenName.c_str(),
                                  SA_OffScreenDragging, TRUE,
                                  TAG_DONE);

  if(m_pIntuiScreen == NULL)
  {
    throw "Failed to open the screen.";
  }

  m_pDrawInfo = GetScreenDrawInfo(m_pIntuiScreen);
  if(m_pDrawInfo == NULL)
  {
    CloseScreen(m_pIntuiScreen);
    throw "Failed to get the screen draw info";
  }

  // Get visual info from screen
  m_pVisualInfo = (APTR) GetVisualInfo(m_pIntuiScreen, TAG_DONE);
  if(m_pVisualInfo == NULL)
  {
    FreeScreenDrawInfo(m_pIntuiScreen, m_pDrawInfo);
    CloseScreen(m_pIntuiScreen);
    throw "Failed to get the visual info.";
  }

  PubScreenStatus(m_pIntuiScreen, 0);
}


OpenClonedWorkbenchScreen::~OpenClonedWorkbenchScreen()
{
  close();
}

bool OpenClonedWorkbenchScreen::close()
{
  if(m_pVisualInfo != NULL)
  {
    FreeVisualInfo(m_pVisualInfo);
    m_pVisualInfo = NULL;
  }

  if(m_pDrawInfo != NULL && m_pIntuiScreen != NULL)
  {
    FreeScreenDrawInfo(m_pIntuiScreen, m_pDrawInfo);
    m_pDrawInfo = NULL;
  }

  if(m_pIntuiScreen != NULL)
  {
    if(CloseScreen(m_pIntuiScreen) == FALSE)
    {
      return false;
    }

    m_pIntuiScreen = NULL;
  }

  return true;
}

const char* OpenClonedWorkbenchScreen::getPubScreenName() const
{
  return m_PubScreenName.c_str();
}

#ifdef __clang__
  #include <clib/gadtools_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/gadtools.h>
  #include <proto/intuition.h>
#endif

#include "OpenClonedWorkbenchScreen.h"

OpenClonedWorkbenchScreen::OpenClonedWorkbenchScreen(const ADiffViewSettings& settings,
                                                     const char* pTitle,
                                                     const char* pPubScreenName,
                                                     short depth)
  : ScreenBase(settings),
    m_pTitle(pTitle),
    m_Depth(depth)
{
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
                                  SA_Colors32, (ULONG)m_Settings.GetColorArray(),
                                  SA_PubName, (ULONG)pPubScreenName,
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

#ifdef __clang__
  #include <clib/gadtools_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/gadtools.h>
  #include <proto/intuition.h>
#endif

#include "OpenJoinedPublicScreen.h"

OpenJoinedPublicScreen::OpenJoinedPublicScreen(const ADiffViewSettings& settings,
                                               const char* pPubScreenName)
  : ScreenBase(settings)
{
  m_pIntuiScreen = LockPubScreen(pPubScreenName);

  if(m_pIntuiScreen == NULL)
  {
    throw "Failed to lock public screen.";
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
}

OpenJoinedPublicScreen::~OpenJoinedPublicScreen()
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
    UnlockPubScreen(NULL, m_pIntuiScreen);
    m_pIntuiScreen = NULL;
  }
}

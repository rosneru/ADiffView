#ifdef __clang__
  #include <clib/gadtools_protos.h>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/gadtools.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
#endif

#include <graphics/text.h>
#include <intuition/intuitionbase.h>

#include "OpenScreenBase.h"

extern struct IntuitionBase* IntuitionBase;

ScreenBase::ScreenBase()
  : m_pIntuiScreen(NULL),
    m_pDrawInfo(NULL),
    m_NumWindowsOpen(0),
    m_pVisualInfo(NULL)
{
}

ScreenBase::~ScreenBase()
{

}


struct Screen* ScreenBase::getIntuiScreen()
{
  return m_pIntuiScreen;
}

struct DrawInfo* ScreenBase::getIntuiDrawInfo()
{
  return m_pDrawInfo;
}

struct TextAttr* ScreenBase::getIntuiTextAttr()
{
  if(m_pIntuiScreen == NULL)
  {
    return 0;
  }

  return m_pIntuiScreen->Font;
}

APTR ScreenBase::getGadtoolsVisualInfo()
{
  return m_pVisualInfo;
}

size_t ScreenBase::getNumOpenWindows() const
{
  return m_NumWindowsOpen;
}

void ScreenBase::increaseNumOpenWindows()
{
  m_NumWindowsOpen++;
}

void ScreenBase::decreaseNumOpenWindows()
{
  if(m_NumWindowsOpen > 0)
  {
    m_NumWindowsOpen--;
  }
}

void ScreenBase::toFront() const
{
  ScreenToFront(m_pIntuiScreen);
}

bool ScreenBase::isHiresMode() const
{
  if(m_pIntuiScreen == NULL)
  {
    return false;
  }

  if(m_pIntuiScreen->Flags & SCREENHIRES)
  {
    return true;
  }

  return false;
}

bool ScreenBase::arePixelsSquare() const
{
  if(m_pIntuiScreen == NULL)
  {
    return false;
  }
  
  LONG modeId = GetVPModeID(&m_pIntuiScreen->ViewPort);
  if(modeId == INVALID_ID)
  {
    return false;  // TODO or true..or throw?
  }

  DisplayInfo dispInfo;

  DisplayInfoHandle dh = FindDisplayInfo(modeId);
  GetDisplayInfoData(dh,
                     (APTR)&dispInfo,
                     sizeof(struct DisplayInfo),
                     DTAG_DISP,
                     INVALID_ID);

  WORD xAspect = dispInfo.Resolution.x;
  WORD yAspect = dispInfo.Resolution.y;

  return xAspect == yAspect;
}

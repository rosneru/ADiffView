#ifdef __clang__
  #include <clib/graphics_protos.h>
#else
  #include <proto/graphics.h>
#endif

#include "ADiffViewPens.h"


ADiffViewPens::ADiffViewPens(ScreenBase& screenBase,
                             const ADiffViewSettings& settings)
  : m_ScreenBase(screenBase),
    m_Settings(settings),
    m_RedPen(-1),
    m_YellowPen(-1),
    m_GreenPen(-1),
    m_GrayPen(-1)
{
  if(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap == NULL)
  {
    throw "Failed to create pens. Missing ColorMap in ViewPort.";
  }

  const ULONG* pColRed = m_Settings.getColorRedArray();
  const ULONG* pColGreen = m_Settings.getColorGreenArray();
  const ULONG* pColYellow = m_Settings.getColorYellowArray();
  const ULONG* pColGray = m_Settings.getColorGrayArray();

  // Find the best pens for the needed colors
  m_RedPen = ObtainBestPen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap,
    pColRed[0], pColRed[1], pColRed[2],
    OBP_FailIfBad, FALSE,
    OBP_Precision, PRECISION_EXACT,
    TAG_DONE);

  m_GreenPen = ObtainBestPen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap,
    pColGreen[0], pColGreen[1], pColGreen[2],
    OBP_FailIfBad, FALSE,
    OBP_Precision, PRECISION_EXACT,
    TAG_DONE);

  m_YellowPen = ObtainBestPen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap,
    pColYellow[0], pColYellow[1], pColYellow[2],
    OBP_FailIfBad, FALSE,
    OBP_Precision, PRECISION_EXACT,
    TAG_DONE);

  m_GrayPen = ObtainBestPen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap,
    pColGray[0], pColGray[1], pColGray[2],
    OBP_FailIfBad, FALSE,
    OBP_Precision, PRECISION_EXACT,
    TAG_DONE);
}

ADiffViewPens::~ADiffViewPens()
{
  ReleasePen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap, m_RedPen);
  ReleasePen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap, m_YellowPen);
  ReleasePen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap, m_GreenPen);
  ReleasePen(m_ScreenBase.getIntuiScreen()->ViewPort.ColorMap, m_GrayPen);
}


LONG ADiffViewPens::Background() const
{
  return m_ScreenBase.getIntuiDrawInfo()->dri_Pens[BACKGROUNDPEN];
}


LONG ADiffViewPens::NormalText()  const
{
  return m_ScreenBase.getIntuiDrawInfo()->dri_Pens[TEXTPEN];
}


LONG ADiffViewPens::HighlightedText()  const
{
  return m_ScreenBase.getIntuiDrawInfo()->dri_Pens[HIGHLIGHTTEXTPEN];
}


LONG ADiffViewPens::Fill()  const
{
  return m_ScreenBase.getIntuiDrawInfo()->dri_Pens[FILLPEN];
}


LONG ADiffViewPens::Red() const
{
  if(m_RedPen < 0)
  {
    return 0;
  }

  return m_RedPen;
}


LONG ADiffViewPens::Yellow() const
{
  if(m_YellowPen < 0)
  {
    return 0;
  }

  return m_YellowPen;
}


LONG ADiffViewPens::Green() const
{
  if(m_GreenPen < 0)
  {
    return 0;
  }

  return m_GreenPen;
}


LONG ADiffViewPens::Gray() const
{
  if(m_GrayPen < 0)
  {
    return 0;
  }

  return m_GrayPen;
}


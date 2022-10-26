#ifdef __clang__
  #include <clib/graphics_protos.h>
#else
  #include <proto/graphics.h>
#endif

#include "DiffWindowRastports.h"


DiffWindowRastports::DiffWindowRastports(struct Window* pIntuiWindow,
                                         const ADiffViewPens& pens)
{
  SetAPen(pIntuiWindow->RPort, pens.NormalText());

  m_RPortWindow = *pIntuiWindow->RPort;

  m_RPortAPenBackgr = *pIntuiWindow->RPort;
  SetAPen(&m_RPortAPenBackgr, pens.Background());

  m_RPortLineNum = *pIntuiWindow->RPort;
  SetBPen(&m_RPortLineNum, pens.Gray());

  m_RPortTextDefault = *pIntuiWindow->RPort;
  SetBPen(&m_RPortTextDefault, pens.Background());

  m_RPortTextSelected = *pIntuiWindow->RPort;
  SetAPen(&m_RPortTextSelected, pens.NormalText());
  SetBPen(&m_RPortTextSelected, pens.HighlightedText());

  m_RPortTextRedBG = *pIntuiWindow->RPort;
  SetBPen(&m_RPortTextRedBG, pens.Red());

  m_RPortTextGreenBG = *pIntuiWindow->RPort;
  SetBPen(&m_RPortTextGreenBG, pens.Green());

  m_RPortTextYellowBG = *pIntuiWindow->RPort;
  SetBPen(&m_RPortTextYellowBG, pens.Yellow());
}


DiffWindowRastports::~DiffWindowRastports()
{

}

struct RastPort* DiffWindowRastports::Window()
{
  return &m_RPortWindow;
}

struct RastPort* DiffWindowRastports::APenBackgr()
{
  return &m_RPortAPenBackgr;
}

struct RastPort* DiffWindowRastports::getLineNumText()
{
  return &m_RPortLineNum;
}

struct RastPort* DiffWindowRastports::TextDefault()
{
  return &m_RPortTextDefault;
}

struct RastPort* DiffWindowRastports::TextSelected()
{
  return &m_RPortTextSelected;
}

struct RastPort* DiffWindowRastports::TextRedBG()
{
  return &m_RPortTextRedBG;
}

struct RastPort* DiffWindowRastports::TextGreenBG()
{
  return &m_RPortTextGreenBG;
}

struct RastPort* DiffWindowRastports::TextYellowBG()
{
  return &m_RPortTextYellowBG;
}

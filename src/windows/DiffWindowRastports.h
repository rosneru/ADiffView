#ifndef DIFF_WINDOW_RASTPORTS_H
#define DIFF_WINDOW_RASTPORTS_H


#include <graphics/rastport.h>
#include <intuition/intuition.h>

#include "ADiffViewPens.h"

/**
 * Different Rastports for different use cases.
 *
 * @author Uwe Rosner
 * @date 01/09/2020
 */
class DiffWindowRastports
{
public:
  DiffWindowRastports(struct Window* pIntuiWindow, 
                      const ADiffViewPens& pens);

  virtual ~DiffWindowRastports();

  struct RastPort* Window();
  struct RastPort* APenBackgr();
  struct RastPort* getLineNumText();
  struct RastPort* TextDefault();
  struct RastPort* TextSelected();
  struct RastPort* TextRedBG();
  struct RastPort* TextGreenBG();
  struct RastPort* TextYellowBG();

private:
  struct RastPort m_RPortWindow;
  struct RastPort m_RPortAPenBackgr;
  struct RastPort m_RPortLineNum;
  struct RastPort m_RPortTextDefault;
  struct RastPort m_RPortTextSelected;
  struct RastPort m_RPortTextRedBG;
  struct RastPort m_RPortTextGreenBG;
  struct RastPort m_RPortTextYellowBG;
};

#endif

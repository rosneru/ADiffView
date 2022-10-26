#ifndef A_DIFF_VIEW_PENS_H
#define A_DIFF_VIEW_PENS_H

#include "OpenScreenBase.h"
#include "ADiffViewSettings.h"

/**
 * Class for the pens used by ADiffView
 *
 * @author Uwe Rosner
 * @date 18/11/2018
 */
class ADiffViewPens
{
public:
  ADiffViewPens(ScreenBase& screenBase,
                const ADiffViewSettings& settings);

  virtual ~ADiffViewPens();

  LONG Background() const;
  LONG NormalText() const;
  LONG HighlightedText() const;
  LONG Fill() const;

  LONG Red() const;
  LONG Yellow() const;
  LONG Green() const;
  LONG Gray() const;


private:
  ScreenBase& m_ScreenBase;
  const ADiffViewSettings& m_Settings;

  LONG m_RedPen;
  LONG m_YellowPen;
  LONG m_GreenPen;
  LONG m_GrayPen;

};

#endif

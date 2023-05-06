#ifndef APPSCREEN
#define APPSCREEN

#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

#include "ADiffViewSettings.h"

/**
 * An Amiga screen to be used for an application. This class is
 * abstract, so only a derived class can be instanciated. 
 *
 * Right after instanciaten the screen is open. Destroy the instance to
 * close the screen.
 *
 * @author Uwe Rosner
 * @date 23/09/2018
 */
class ScreenBase
{
public:
  ScreenBase();
  virtual ~ScreenBase();

  virtual bool close() = 0;

  /**
   * Returns the intuition screen structure or NULL if screen is not open
   */
  struct Screen* getIntuiScreen();

  /**
   * Returns the intuition screen draw info or NULL if it hadn't been
   * acquired successfully
   */
  struct DrawInfo* getIntuiDrawInfo();

  /**
   * Returns the TextAttr of the screen
   */
  struct TextAttr* getIntuiTextAttr();

  /**
   * Returns the VisualInfo needed for GadTools menus and Gadgets
   */
  APTR getGadtoolsVisualInfo();

  /**
   * Returns the number of windows currently open on this screen.
   */
  size_t getNumOpenWindows() const;

  /**
   * TODO Find something better than to manually handle this number
   */
  void increaseNumOpenWindows();

  /**
   * TODO Find something better than to manually handle this number
   */
  void decreaseNumOpenWindows();

  /**
   * Brings the screen to the front of the display.
   */
  void toFront() const;

  /**
   * Returns true if the screen is in a Hires mode which probably means
   * it has non-square pixels.
   * 
   * NOTE: If the screen is interlaced the pixels are square, 'though.
   */
  bool isHiresMode() const;

  /**
   * Returns true if (according to the display database) the pixels of
   * this screen are square.
   */
  bool arePixelsSquare() const;

protected:
  struct Screen* m_pIntuiScreen;
  struct DrawInfo* m_pDrawInfo;
  struct TextFont* m_pTextFont;

  size_t m_NumWindowsOpen;

  APTR m_pVisualInfo;
};


#endif

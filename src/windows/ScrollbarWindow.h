#ifndef SCROLBARL_WINDOW_H
#define SCROLBARL_WINDOW_H

#include <exec/ports.h>
#include <intuition/screens.h>

#include "OpenScreenBase.h"
#include "WindowBase.h"

/**
 * Class a window that contains vertical and horizontal scrollbars in
 * its border.
 *
 * @author Uwe Rosner
 * @date 10/12/2018
 */
class ScrollbarWindow : public WindowBase
{
public:
  ScrollbarWindow(ScreenBase& screen,
                  struct MsgPort* pIdcmpMsgPort,
                  MenuBase* pMenu);

  virtual ~ScrollbarWindow();


  /**
   * Opens the window.
   *
   * @returns
   * false if opening fails
   */
  virtual bool open(InitialPosition initialPos = WindowBase::IP_Center);

  /**
   * @implements abstract method of base class WindowBase.
   */
  virtual void handleIDCMP(ULONG msgClass,
                           UWORD msgCode,
                           APTR pItemAddress);

protected:
  long m_InnerWindowRight;  ///> X-position of the right-most pixel before the scrollbar
  long m_InnerWindowBottom; ///> Y-position of the bottom-most pixel before the scrollbar


  /**
   * This handles the x-changes triggered by the horizontal scrollbar
   * of the window. It is called from the handleIDCMP method of this
   * class.
   *
   * Derived classes must implement the method and scroll the content to
   * the given x-position.
   */
  virtual void handleXChange(size_t newX) = 0;

  /**
   * This handles the y-changes triggered by the vertical scrollbar of
   * the window. Is called by the method HandleIDCMP() of this class.
   *
   * Derived classes must implement the method and scroll the content to
   * the given y-position.
   */
  virtual void handleYChange(size_t newY) = 0;

  /**
   * This handles the increase by 1 of the x-position triggered by the
   * cursor-right-key or the right-arrow-gadget. It is called from the
   * handleIDCMP method of this class.
   *
   * Derived classes must implement the method and perform the scrolling
   * to x+numChars.
   *
   * @param numChars Amount to increase the x-position by
   *
   * @param bTriggeredByScrollPot If the call is triggered by moving the
   * scrollbar pot: true If the call is triggered by other sources:
   * false
   */
  virtual void handleXIncrease(size_t numChars,
                               bool bTriggeredByScrollPot = false) = 0;

  /**
   * This handles the decrease by 1 of the x-position triggered by the
   * cursor-left-key or the left-arrow-gadget. It is called from the
   * handleIDCMP method of this class.
   *
   * Derived classes must implement the method and perform the scrolling
   * to x-numChars.
   *
   * @param numChars
   * Amount to decrease the x-position by
   *
   * @param bTriggeredByScrollPot
   * If the call is triggered by moving the scrollbar pot: true
   * If the call is triggered by other sources: false
   */
  virtual void handleXDecrease(size_t numChars,
                               bool bTriggeredByScrollPot = false) = 0;

  /**
   * This handles the increase by 1 of the y-position triggered by the
   * cursor-down-key or the down-arrow-gadget. It is called from the
   * handleIDCMP method of this class.
   *
   * Derived classes must implement the method and perform the scrolling
   * to y+numLines.
   *
   * @param numLines
   * Amount to increase the y-position by
   *
   * @param bTriggeredByScrollPot
   * If the call is triggered by moving the scrollbar pot: true
   * If the call is triggered by other sources: false
   */
  virtual void handleYIncrease(size_t numLines,
                               bool bTriggeredByScrollPot = false) = 0;

  /**
   * This handles the decrease by 1 of the y-position triggered by the
   * cursor-up-key or the up-arrow-gadget. It is called from the
   * handleIDCMP method of this class.
   *
   * Derived classes must implement the method and perform the scrolling
   * to y-numLines.
   *
   * @param numLines
   * Amount to decrease the y-position by
   *
   * @param bTriggeredByScrollPot
   * If the call is triggered by moving the scrollbar pot: true
   * If the call is triggered by other sources: false
   */
  virtual void handleYDecrease(size_t numLines,
                               bool bTriggeredByScrollPot = false) = 0;

  /**
   * Calculates some inner window sizes which is needed after window
   * opening and after resizing.
   */
  virtual void calcSizes();

  /**
   * Setting a new top position to the x-scrollbar pot.
   */
  void setXScrollTop(int left);


  /**
   * Setting a new top position to the y-scrollbar pot.
   */
  void setYScrollTop(int top);

  /**
   * Setting the pot size of the x-scrollbar.
   *
   * @param maxVisibleChars
   * Number of max visible chars / points / etc in window. Has to be
   * updated e.g. after the window resizes.
   *
   * @param totalChars
   * Number of total visible chars / points / etc in document.
   */
  void setXScrollPotSize(int maxVisibleChars, int totalChars);

  /**
   * Setting the pot size of the y-scrollbar.
   *
   * @param maxVisibleLines
   * Number of max visible lines / points / etc in window. Has to be
   * updated e.g. after the window resizes.
   *
   * @param totalLines
   * Number of total visible lines / points / etc in document.
   */
  void setYScrollPotSize(int maxVisibleLines, int totalLines);

private:
  int m_MaxVisibleLines;

  /**
   * IDs to help to interpret the events of this window's BOOPSI system
   * gadgets.
   */
  enum GadgetId
  {
    GID_PropX,
    GID_PropY,
    GID_ArrowUp,
    GID_ArrowDown,
    GID_ArrowLeft,
    GID_ArrowRight,
  };

  ULONG m_SizeGadWidth;
  ULONG m_SizeGadHeight;

  struct Image* m_pImgLeftArrow;      ///> h-scrollbar left button image
  struct Image* m_pImgRightArrow;     ///> h-scrollbar right button image
  struct Image* m_pImgUpArrow;        ///> v-scrollbar up button image
  struct Image* m_pImgDownArrow;      ///> v-scrollbar down button image

  struct Gadget* m_pGadPropX;         ///> horizontal scrollbar
  struct Gadget* m_pGadPropY;         ///> vertical scrollbar
  struct Gadget* m_pGadBtnLeftArrow;  ///> h-scrollbar left button
  struct Gadget* m_pGadBtnRightArrow; ///> h-scrollbar right button
  struct Gadget* m_pGadBtnUpArrow;    ///> v-scrollbar up button
  struct Gadget* m_pGadBtnDownArrow;  ///> v-scrollbar down button

  void cleanup();

  /**
   * Browses through all IDCMP_IDCMPUPDATE messages for the given
   * proportional gadget (BOOPSI) which are addressed to this window.
   * It deletes each of this messages by replying it to intuition.
   * Finally the given value argument variable is set to the last
   * found top value.
   *
   * This should be used on proportional gadgets to get only the latest
   * top value and delete all previous messages of this kind. Usefull
   * on slow systems to not get flooded by messages for this gadget.
   *
   * @param gadgetId
   * Id of the BOOPSI gadget to browse for messages
   *
   * @param lastVal
   * If one or more IDCMP_IDCMPUPDATE messages are found while browsing,
   * the tag value of the latest found message is written to this given
   * argument. If no appropriate message is found the argument variable
   * will not be overwritten.
   */
  void extractLatestPropGadTopValue(GadgetId gadgetId, size_t& lastVal);
};


#endif

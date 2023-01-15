#ifndef DIFF_WINDOW_H
#define DIFF_WINDOW_H

#include <string>

#include <exec/ports.h>
#include <intuition/screens.h>

#include "ADiffViewPens.h"
#include "DiffDocument.h"
#include "DiffLine.h"
#include "DiffWindowRastports.h"
#include "DiffWindowTextArea.h"
#include "OpenScreenBase.h"
#include "ScrollbarWindow.h"

/**
 * Class for a diff window object. Can be created multiple times.
 *
 * @author Uwe Rosner
 * @date 16/11/2018
 */
class DiffWindow : public ScrollbarWindow
{
public:
  DiffWindow(ScreenBase& screen,
             const ADiffViewPens& pens,
             struct MsgPort* pIdcmpMsgPort,
             MenuBase* pMenu,
             ULONG tabSize);

  virtual ~DiffWindow();


  /**
   * Open the window.
   *
   * @returns
   * false if opening fails
   */
  virtual bool open(InitialPosition initialPos = WindowBase::IP_Center);


  /**
   * Set the diff document to be displayed in DiffWindow.
   */
  bool setDocument(DiffDocument* pDiffDocument);

  /**
   * Sets the given tabulator size and redraws the document if
   * necessary.
   */
  void setTabulatorSize(ULONG tabSize);

  /**
   * Re-renders the documents without clearing the area before
   * If a lineId is given, only this line is rendered.
   */
  void renderDocuments(long long justThisLineId = -1);


  /**
   * Clears all selections and stop the dynamic selection mode if there
   * is any
   */
  void clearAndStopSelection();

  /**
   * Scroll the window to display given line at top.
   */
  void scrollTopTo(size_t top);
  
  /**
   * Scroll the window to display given column at left.
   */
  void scrollLeftTo(size_t left);

  /**
   * Scrolls until the the given page of (charId, lineId), (numLines x
   * numChars) is visible in current display. Doesn't scroll if given
   * page is already visible.
   *
   *  @returns true if scrolling has been done.
   */
  bool scrollToPage(size_t charId, 
                    size_t lineId, 
                    size_t numChars, 
                    size_t numLines);

  bool isHorizontallyVisible(size_t startCharId) const;
  bool isVerticallyVisible(size_t startLineId) const;

  /**
   * Return the left text area
   */
  DiffWindowTextArea* getLeftTextArea() const;

  /**
   * Return the right text area
   */
  DiffWindowTextArea* getRightTextArea() const;


  /**
   * @extends method of base class ScrollbarWindow.
   */
  virtual void handleIDCMP(const struct IntuiMessage* pMsg);

protected:
  /**
   * Recalculate the window content and redraw completely according to
   * new dimension (m_Width, m_Height).
   *
   * This should be called from the application if the signal
   * IDCMP_NEWSIZE for this window is received.
   */
  void performResize();

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleXChange(size_t newX);

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleYChange(size_t newY);

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleXIncrease(size_t numChars, bool bTriggeredByScrollPot);

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleXDecrease(size_t numChars, bool bTriggeredByScrollPot);

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleYIncrease(size_t numLines, bool bTriggeredByScrollPot);

  /**
   * @implements abstract method of base class ScrollbarWindow.
   */
  void handleYDecrease(size_t numLines, bool bTriggeredByScrollPot);

private:
  const ADiffViewPens& m_Pens;

  ULONG m_TabSize;

  DiffWindowRastports* m_pRPorts;
  DiffDocument* m_pDocument;
  char m_EmptyChar;

  struct Gadget* m_pLastParentGadget;
  struct Gadget* m_pGadtoolsContext;
  struct Gadget* m_pGadTxtLeftFile;
  struct Gadget* m_pGadTxtRightFile;

  long m_NumDifferences;      ///> Number of differences

  ULONG m_IndentX;  ///> X-distance of the text areas to window borders
  ULONG m_IndentY;  ///> Y-distance of the text areas to window borders
  ULONG m_TextAreasWidth;
  ULONG m_TextAreasHeight;

  DiffWindowTextArea* m_pLeftTextArea;
  DiffWindowTextArea* m_pRightTextArea;

  enum SelectionMode
  {
    SM_NONE = 0,
    SM_SELECTION_LEFT_STARTED,
    SM_SELECTION_LEFT_FINISHED,
    SM_SELECTION_RIGHT_STARTED,
    SM_SELECTION_RIGHT_FINISHED,
  };

  SelectionMode m_SelectionMode;

  char m_ChangedText[20];
  char m_AddedText[20];
  char m_DeletedText[20];
  char m_StatusBarText[60];

  struct TextAttr m_TextAttr;

  //
  // The next two are called from HandleIDCMP() to get that method not
  // overblown.
  //
  void handleGadgetEvent(struct Gadget* pGadget);
  void handleVanillaKey(UWORD code);
  void handleMouseButtons(const struct IntuiMessage* pMsg);
  void handleMouseMove(const struct IntuiMessage* pMsg);

  void cleanup();

  bool createGadgets();

  /**
   * Calculates some inner window sizes which is needed after window
   * opening and after resizing.
   */
  void calcSizes();

  /**
   * Resize the gadgets. Should be called after the window size has
   * changed. As gadget-resizing or re-positioning is not supported with
   * Gadtools, the gadgets are re-created completely.
   */
  void resizeGadgets();


  /**
   * Paint the basic window content: the 2 BevelBoxes for the original
   * (left) and changed (right) diff file, the Document names and the
   * state bar at the bottom.
   */
  void paintWindowDecoration();


  /**
   * Display a status bar with statistical information and a legend
   * below the two text areas
   */
  void paintStatusBar();
};


#endif

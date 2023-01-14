#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/gadtools_protos.h>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
  #include <clib/utility_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/gadtools.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
  #include <proto/utility.h>
#endif

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <libraries/gadtools.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "MessageBox.h"
#include "DiffWindow.h"



DiffWindow::DiffWindow(ScreenBase& screen,
                       const ADiffViewPens& pens,
                       struct MsgPort* pIdcmpMsgPort,
                       MenuBase* pMenu,
                       ULONG tabSize)
  : ScrollbarWindow(screen, pIdcmpMsgPort, pMenu),
    m_Pens(pens),
    m_TabSize(tabSize),
    m_pRPorts(NULL),
    m_pDocument(NULL),
    m_EmptyChar('\0'),
    m_pLastParentGadget(NULL),
    m_pGadtoolsContext(NULL),
    m_pGadTxtLeftFile(NULL),
    m_pGadTxtRightFile(NULL),
    m_NumDifferences(0),
    m_IndentX(5),
    m_IndentY(0),
    m_TextAreasWidth(0),
    m_TextAreasHeight(0),
    m_pLeftTextArea(NULL),
    m_pRightTextArea(NULL),
    m_SelectionMode(DiffWindow::SM_NONE)
{
  // If parent window already defined gadgets, we store the last of
  // these gadgets and the count of defined gadgets. They are needed
  // for dynamically re-creating this window's gadgets at window
  // resizing etc.
  m_pLastParentGadget = getLastGadget();

  // Create this window's gadgets
  if(createGadgets() == false)
  {
    cleanup();
    throw "DiffWindow: Failed to create gadgets.";
  }

  // Set the default title
  setTitle("ADiffView");

  // Setting the window flags
  addFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DEPTHGADGET |     // Add a depth gadget
           WFLG_SIZEGADGET |      // Add a size gadget
           WFLG_GIMMEZEROZERO |
           WFLG_REPORTMOUSE);   // Different layers for border and content

  // Setting the IDCMP messages we want to receive for this window
  addIDCMP(IDCMP_MENUPICK |       // Inform about menu selection
           IDCMP_CLOSEWINDOW |    // Inform about click on close gadget
           IDCMP_NEWSIZE |        // Inform about resizing
           IDCMP_MOUSEMOVE |      // Inform about mouse movement
           IDCMP_MOUSEBUTTONS);   // Inform about mouse buttons
}


DiffWindow::~DiffWindow()
{
  cleanup();
}


void DiffWindow::performResize()
{
  if(!isOpen())
  {
    return;
  }

  // TODO check if this test and m_Width/Height assignment can be removed.
  if(((ULONG)m_pWindow->Width) == m_Width &&
     ((ULONG)m_pWindow->Height) == m_Height)
  {
    // nothing changed
    return;
  }

  m_Width = m_pWindow->Width;
  m_Height = m_pWindow->Height;

  // Clear the window completely
  SetRast(m_pRPorts->Window(), m_Pens.Background());

  // Calculate some values which have to calculated after window
  // opening and after resizing
  calcSizes();

  // Resize gadgets to fit into new window size
  resizeGadgets();

  if((m_pLeftTextArea == NULL) || (m_pRightTextArea == NULL))
  {
    return;
  }

  // Set location and size of the left text area
  m_pLeftTextArea->setPosition(m_IndentX, m_IndentY);
  m_pLeftTextArea->setSize(m_TextAreasWidth, m_TextAreasHeight);

  // Set location and size of the right text area
  m_pRightTextArea->setPosition(m_IndentX + m_TextAreasWidth, m_IndentY);
  m_pRightTextArea->setSize(m_TextAreasWidth, m_TextAreasHeight);

  // Paint the content of the two documents (from current y-position,
  //not from start)
  m_pLeftTextArea->renderPage();
  m_pRightTextArea->renderPage();

  if(m_pDocument == NULL)
  {
    return;
  }

  // Set x-scroll-gadget's pot size in relation of new window size
  setXScrollPotSize(m_pLeftTextArea->getMaxVisibleChars(),
                    m_pDocument->getMaxLineLength());

  // Set y-scroll-gadget's pot size in relation of new window size
  setYScrollPotSize(m_pLeftTextArea->getMaxVisibleLines(),
                    m_pDocument->getNumLines());


  // Paint the status bar
  paintStatusBar();

  // Paint the window decoration
  paintWindowDecoration();
}


bool DiffWindow::open(InitialPosition initialPos)
{
  //
  // Open the window (and the screen if it isn't open yet)
  //
  if(!ScrollbarWindow::open(initialPos))
  {
    return false;
  }


  if(m_pRPorts != NULL)
  {
    delete m_pRPorts;
    m_pRPorts = NULL;
  }

  m_pRPorts = new DiffWindowRastports(m_pWindow, m_Pens);
  m_pDocument = NULL;

  //
  // Calculate some initial values. It's possible that they are needed
  // in the initialize() method which is called from WindowBase::Open()
  // shortly before opening, so the Open() call is done afterwards.
  //
  m_IndentY = 2 *  m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;

  m_TextAttr.ta_Name = m_Screen.getIntuiDrawInfo()->dri_Font->tf_Message.mn_Node.ln_Name;
  m_TextAttr.ta_YSize = m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;
  m_TextAttr.ta_Style = m_Screen.getIntuiDrawInfo()->dri_Font->tf_Style;
  m_TextAttr.ta_Flags = m_Screen.getIntuiDrawInfo()->dri_Font->tf_Flags;


  // Calculate some sizes which are only calculable with window already
  // open
  calcSizes();

  // Paint the window decoration
  paintWindowDecoration();

  return true;
}


bool DiffWindow::setDocument(DiffDocument* pDiffDocument)
{
  if(pDiffDocument == NULL)
  {
    return false;
  }

  m_pDocument = pDiffDocument;

  m_NumDifferences = pDiffDocument->getNumAdded()
                   + pDiffDocument->getNumChanged()
                   + pDiffDocument->getNumDeleted();

  sprintf(m_StatusBarText,
          "Diff performed in %ld ms. Total changes: %ld   |   ",
          pDiffDocument->getElapsedDiffTime(),
          m_NumDifferences);

  sprintf(m_AddedText,
          "%zu Added",
          pDiffDocument->getNumAdded());

  sprintf(m_ChangedText,
          "%zu Changed",
          pDiffDocument->getNumChanged());

  sprintf(m_DeletedText,
          "%zu Deleted",
          pDiffDocument->getNumDeleted());

  if(!isOpen())
  {
    return true;
  }

  // Clear the window completely
  SetRast(m_pRPorts->Window(), m_Pens.Background());

  // Display the document file names in the gadgets
  GT_SetGadgetAttrs(m_pGadTxtLeftFile,
                    m_pWindow,
                    NULL,
                    GTTX_Text, (ULONG)m_pDocument->getLeftFileName(),
                    TAG_DONE);

  GT_SetGadgetAttrs(m_pGadTxtRightFile,
                    m_pWindow,
                    NULL,
                    GTTX_Text, (ULONG)m_pDocument->getRightFileName(),
                    TAG_DONE);


  // Ensure that the gadgets are re-drawn
  RefreshGList(m_pGadtoolsContext, m_pWindow, NULL, -1);

  calcSizes();

  // Remove diff text areas if still existing
  if(m_pLeftTextArea != NULL)
  {
    delete m_pLeftTextArea;
    m_pLeftTextArea = NULL;
  }

  if(m_pRightTextArea != NULL)
  {
    delete m_pRightTextArea;
    m_pRightTextArea = NULL;
  }

  m_pLeftTextArea = new DiffWindowTextArea(pDiffDocument->getLeftDiffFile(),
                                        m_pRPorts,
                                        m_pTextFont,
                                        pDiffDocument->areLineNumbersEnabled(),
                                        pDiffDocument->getMaxLineLength(),
                                        m_TabSize,
                                        m_pWindow->WScreen->WBorLeft,
                                        m_pWindow->WScreen->WBorTop);

  m_pRightTextArea = new DiffWindowTextArea(pDiffDocument->getRightDiffFile(),
                                        m_pRPorts,
                                        m_pTextFont,
                                        pDiffDocument->areLineNumbersEnabled(),
                                        pDiffDocument->getMaxLineLength(),
                                        m_TabSize,
                                        m_pWindow->WScreen->WBorLeft,
                                        m_pWindow->WScreen->WBorTop);

  m_SelectionMode = DiffWindow::SM_NONE;

  // Set location and size of the left text area
  m_pLeftTextArea->setPosition(m_IndentX, m_IndentY);
  m_pLeftTextArea->setSize(m_TextAreasWidth, m_TextAreasHeight);

  // Set location and size of the right text area
  m_pRightTextArea->setPosition(m_IndentX + m_TextAreasWidth, m_IndentY);
  m_pRightTextArea->setSize(m_TextAreasWidth, m_TextAreasHeight);

  // Paint the content of the two documents (from start)
  m_pLeftTextArea->printPageAt(0, 0);
  m_pRightTextArea->printPageAt(0, 0);

  // With the calculated sizes the gadgets must be re-sized/positioned
  resizeGadgets();

  // Paint the window decoration
  paintWindowDecoration();

  // Paint the status bar
  paintStatusBar();

  // Set x-scroll-gadget's pot size in relation of new window size
  setXScrollPotSize(m_pLeftTextArea->getMaxVisibleChars(),
                    m_pDocument->getMaxLineLength());

  // Set y-scroll-gadget's pot size in relation of new window size
  setYScrollPotSize(m_pLeftTextArea->getMaxVisibleLines(),
                    m_pDocument->getNumLines());

  // Set both scroll gadgets to zero position
  setXScrollTop(0);
  setYScrollTop(0);

  return true;
}


void DiffWindow::setTabulatorSize(ULONG tabSize)
{
  if(tabSize == m_TabSize)
  {
    return;
  }

  m_TabSize = tabSize;

  m_pLeftTextArea->setTabulatorSize(m_TabSize);
  m_pRightTextArea->setTabulatorSize(m_TabSize);
  m_pLeftTextArea->clearText();
  m_pRightTextArea->clearText();
  renderDocuments();
}


void DiffWindow::renderDocuments(long long justThisLineId)
{
  if(justThisLineId < 0)
  {
    m_pLeftTextArea->renderPage();
    m_pRightTextArea->renderPage();
  }
  else
  {
    m_pLeftTextArea->renderIndexedLine(justThisLineId);
    m_pRightTextArea->renderIndexedLine(justThisLineId);
  }
  
}


void DiffWindow::scrollTopTo(size_t top)
{
  // Scroll y to next diff
  handleYChange(top);

  // Set scrollbar to new y position
  // NOTE: y is equal in left and right text area, so it doesn't 
  //       matter which one to get.
  setYScrollTop(m_pLeftTextArea->getY());
}

void DiffWindow::scrollLeftTo(size_t left)
{
  // Scroll y to next diff
  handleXChange(left);

  // Set scrollbar to new y position
  // NOTE: x is equal in left and right text area, so it doesn't 
  //       matter which one to get.
  setXScrollTop(m_pLeftTextArea->getX());
}


bool DiffWindow::scrollToPage(size_t charId, 
                              size_t lineId, 
                              size_t numChars, 
                              size_t numLines)
{
  if((m_pLeftTextArea == NULL) || (m_pRightTextArea == NULL))
  {
    return false;
  }

  bool hasScrolled = false;

  if(!isHorizontallyVisible(charId))
  {
    scrollLeftTo(charId);
    hasScrolled = true;
  }

  if(!isVerticallyVisible(lineId))
  {
    scrollTopTo(lineId);
    hasScrolled = true;
  }

  return hasScrolled;
}


bool DiffWindow::isHorizontallyVisible(size_t startCharId) const
{
  if(startCharId < m_pLeftTextArea->getX())
  {
    return false;
  }

  size_t rightMostCharId = m_pLeftTextArea->getX() 
                         + m_pLeftTextArea->getMaxVisibleChars()
                         - 1;

  if(startCharId > rightMostCharId)
  {
    return false;
  }

  return true;
}

bool DiffWindow::isVerticallyVisible(size_t startLineId) const
{
  if(startLineId < m_pLeftTextArea->getY())
  {
    return false;
  }

  size_t bottomMostLineId = m_pLeftTextArea->getY()
                          + m_pLeftTextArea->getMaxVisibleLines()
                          - 1;

  if(startLineId > bottomMostLineId)
  {
    return false;
  }

  return true;
}


DiffWindowTextArea* DiffWindow::getLeftTextArea() const
{
  return m_pLeftTextArea;
}

DiffWindowTextArea* DiffWindow::getRightTextArea() const
{
  return m_pRightTextArea;
}

void DiffWindow::handleIDCMP(const struct IntuiMessage* pMsg)
{
  ScrollbarWindow::handleIDCMP(pMsg);

  switch (pMsg->Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) pMsg->IAddress;
      handleGadgetEvent(pGadget);
      break;
    }

    case IDCMP_VANILLAKEY:
    {
      handleVanillaKey(pMsg->Code);
      break;
    }

    case IDCMP_NEWSIZE:
    {
      performResize();
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      close();
      break;
    }

    case IDCMP_MOUSEBUTTONS:
    {
      handleMouseButtons(pMsg);
    }

    case IDCMP_MOUSEMOVE:
    {
      handleMouseMove(pMsg);
    }
  }
}


void DiffWindow::handleXChange(size_t newX)
{
  m_pLeftTextArea->scrollLeftToColumn(newX);
  m_pRightTextArea->scrollLeftToColumn(newX);
}


void DiffWindow::handleYChange(size_t newY)
{
  m_pLeftTextArea->scrollTopToRow(newY);
  m_pRightTextArea->scrollTopToRow(newY);
}


void DiffWindow::handleXIncrease(size_t numChars,
                           bool bTriggeredByScrollPot)
{
  m_pLeftTextArea->scrollLeft(numChars);
  m_pRightTextArea->scrollLeft(numChars);

  if(!bTriggeredByScrollPot)
  {
    // Y-position-decrease was not triggered by the scrollbar pot
    // directly. So the pot top position must be set manually.
    setXScrollTop(m_pLeftTextArea->getX());
  }
}


void DiffWindow::handleXDecrease(size_t numChars,
                                 bool bTriggeredByScrollPot)
{
  m_pLeftTextArea->scrollRight(numChars);
  m_pRightTextArea->scrollRight(numChars);

  if(!bTriggeredByScrollPot)
  {
    // Y-position-decrease was not triggered by the scrollbar pot
    // directly. So the pot top position must be set manually.
    setXScrollTop(m_pLeftTextArea->getX());
  }
}


void DiffWindow::handleYIncrease(size_t numLines,
                                 bool bTriggeredByScrollPot)
{
  m_pLeftTextArea->scrollUp(numLines);
  m_pRightTextArea->scrollUp(numLines);

  if(!bTriggeredByScrollPot)
  {
    // Y-position-decrease was not triggered by the scrollbar pot
    // directly. So the pot top position must be set manually.
    setYScrollTop(m_pLeftTextArea->getY());
  }
}


void DiffWindow::handleYDecrease(size_t numLines,
                                 bool bTriggeredByScrollPot)
{
  m_pLeftTextArea->scrollDown(numLines);
  m_pRightTextArea->scrollDown(numLines);

  if(!bTriggeredByScrollPot)
  {
    // Y-position-decrease was not triggered by the scrollbar pot
    // directly. So the pot top position must be set manually.
    setYScrollTop(m_pLeftTextArea->getY());
  }
}


void DiffWindow::cleanup()
{
  if(m_pLeftTextArea != NULL)
  {
    delete m_pLeftTextArea;
    m_pLeftTextArea = NULL;
  }

  if(m_pRightTextArea != NULL)
  {
    delete m_pRightTextArea;
    m_pRightTextArea = NULL;
  }

  close();

  if(m_pRPorts != NULL)
  {
    delete m_pRPorts;
    m_pRPorts = NULL;
  }

  if(m_pGadtoolsContext != NULL)
  {
    FreeGadgets(m_pGadtoolsContext);
    m_pGadtoolsContext = NULL;
  }

  m_pGadTxtLeftFile = NULL;
  m_pGadTxtRightFile = NULL;
}


bool DiffWindow::createGadgets()
{
  WORD fontHeight =  m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;

  // Default location and sizes for the two string gadgets
  WORD gadWidth = m_TextAreasWidth;
  WORD gadHeight = fontHeight + 4;
  WORD gadTop = m_IndentY - fontHeight - 4;
  WORD gad1Left = m_IndentX;
  WORD gad2Left = m_IndentX + gadWidth;

  // If the text areas already exist overwrite the defaults with the actual
  if((m_pLeftTextArea != NULL) && (m_pRightTextArea == NULL))
  {
    gadWidth = m_pLeftTextArea->getWidth();
    gadTop = m_pLeftTextArea->getTop() - fontHeight - 4;
    gad1Left = m_pLeftTextArea->getLeft();
    gad2Left = m_pRightTextArea->getLeft();
  }


  struct Gadget* pFakeGad = NULL;
  pFakeGad = (struct Gadget*) CreateContext(&m_pGadtoolsContext);
  if(pFakeGad == NULL)
  {
    return false;
  }

  if(m_pGadtoolsContext == NULL)
  {
    // This is the first call of createGadgets() in this session
    if(m_pLastParentGadget == NULL)
    {
      // Parent has no gadgets created: Set this context as first gadget
      setFirstGadget(m_pGadtoolsContext);
    }
    else
    {
      // Parent contains already gadgets: link this context as successor
      // of parents last gadget
      m_pLastParentGadget->NextGadget = m_pGadtoolsContext;
    }
  }

  struct NewGadget newGadget;
  newGadget.ng_TextAttr   = m_Screen.getIntuiTextAttr();
  newGadget.ng_VisualInfo = m_Screen.getGadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = gad1Left;
  newGadget.ng_TopEdge    = gadTop;
  newGadget.ng_Width      = gadWidth;
  newGadget.ng_Height     = gadHeight;
  newGadget.ng_Flags      = PLACETEXT_RIGHT | NG_HIGHLABEL;
  newGadget.ng_GadgetText = NULL;

  const char* pFileName = (m_pDocument == NULL) ? &m_EmptyChar
                                                : m_pDocument->getLeftFileName();

  m_pGadTxtLeftFile = CreateGadget(TEXT_KIND,
                                   pFakeGad,
                                   &newGadget,
                                   GTTX_Border, TRUE,
                                   GTTX_Text, (ULONG)pFileName,
                                   TAG_DONE);
  if(m_pGadTxtLeftFile == NULL)
  {
    return false;
  }

  newGadget.ng_LeftEdge = gad2Left;
  pFileName = (m_pDocument == NULL) ? &m_EmptyChar
                                    : m_pDocument->getRightFileName();

  m_pGadTxtRightFile = CreateGadget(TEXT_KIND,
                                    m_pGadTxtLeftFile,
                                    &newGadget,
                                    GTTX_Border, TRUE,
                                    GTTX_Text, (ULONG)pFileName,
                                    TAG_DONE);
  if(m_pGadTxtRightFile == NULL)
  {
    return false;
  }

  return true;
}


void DiffWindow::handleGadgetEvent(struct Gadget* pGadget)
{
  if(pGadget == NULL)
  {
    return;
  }

  // Currently nothing to handle
}


void DiffWindow::handleVanillaKey(UWORD code)
{
  // Currently nothing to handle
}


void DiffWindow::handleMouseButtons(const struct IntuiMessage* pMsg)
{
  switch(pMsg->Code)
  {
    case SELECTDOWN:
    {
      if(m_pLeftTextArea == NULL || m_pRightTextArea == NULL)
      {
        return;
      }

      if(m_SelectionMode == SM_SELECTION_LEFT_FINISHED
      && m_pLeftTextArea->isPointInSelection(pMsg->MouseX, pMsg->MouseY))
      {
        m_SelectionMode = SM_NONE;
        m_pLeftTextArea->clearSelection();
        m_pLeftTextArea->renderSelectionUpdatedLines();
      }
      else if(m_SelectionMode == SM_SELECTION_RIGHT_FINISHED
           && m_pRightTextArea->isPointInSelection(pMsg->MouseX, pMsg->MouseY))
      {
        m_SelectionMode = SM_NONE;
        m_pRightTextArea->clearSelection();
        m_pRightTextArea->renderSelectionUpdatedLines();
      }
      else if(m_pLeftTextArea->isPointInTextArea(pMsg->MouseX, pMsg->MouseY))
      {
        m_SelectionMode = SM_SELECTION_LEFT_STARTED;

        m_pRightTextArea->clearSelection();
        m_pRightTextArea->renderSelectionUpdatedLines();
        m_pLeftTextArea->startSelection(pMsg->MouseX, pMsg->MouseY);
      }
      else if(m_pRightTextArea->isPointInTextArea(pMsg->MouseX, pMsg->MouseY))
      {
        m_SelectionMode = SM_SELECTION_RIGHT_STARTED;

        m_pLeftTextArea->clearSelection();
        m_pLeftTextArea->renderSelectionUpdatedLines();
        m_pRightTextArea->startSelection(pMsg->MouseX, pMsg->MouseY);
      }
      else
      {
        m_SelectionMode = SM_NONE;
      }
      break;
    }

    case SELECTUP:
    {
      if(m_SelectionMode == SM_SELECTION_LEFT_STARTED)
      {
        m_SelectionMode = SM_SELECTION_LEFT_FINISHED;
      }
      else if(m_SelectionMode == SM_SELECTION_RIGHT_STARTED)
      {
        m_SelectionMode = SM_SELECTION_LEFT_FINISHED;
      }
      else
      {
        m_SelectionMode = SM_NONE;
      }
      break;
    }
  }
}


void DiffWindow::handleMouseMove(const struct IntuiMessage* pMsg)
{
  switch(m_SelectionMode)
  {
    case DiffWindow::SM_SELECTION_LEFT_STARTED:
    {
      m_pLeftTextArea->updateSelection(pMsg->MouseX, pMsg->MouseY);
      break;
    }
    case DiffWindow::SM_SELECTION_RIGHT_STARTED:
    {
      m_pRightTextArea->updateSelection(pMsg->MouseX, pMsg->MouseY);
      break;
    }
  }
}


void DiffWindow::calcSizes()
{
  // Calculate the current m_InnerWindowRight and m_InnerWindowBottom
  ScrollbarWindow::calcSizes();

  // Calculate the
  m_TextAreasWidth = m_InnerWindowRight - m_IndentX - m_IndentX;
  m_TextAreasWidth /= 2;

  // Pre-calc text areas height. Will later be limited to int multiples.
  m_TextAreasHeight = m_InnerWindowBottom - m_IndentY - m_IndentY;
}


void DiffWindow::resizeGadgets()
{
  if(m_pGadtoolsContext == NULL)
  {
    // No gadgets created
    return;
  }

  // Save a copy of the soon to be obsolete GadTools context
  struct Gadget* pOldContext = m_pGadtoolsContext;

  // Detach this windows gadgets from the ones defined in parent window
  if(m_pLastParentGadget != NULL)
  {
    m_pLastParentGadget->NextGadget = NULL;
  }

  // Remove these gadgets from the windows gadget list
  RemoveGList(m_pWindow, m_pGadtoolsContext, -1);

  // Clear the area on which the new gadgets will be drawn
  RectFill(m_pRPorts->APenBackgr(),
           0,
           0,
           m_InnerWindowRight,
           m_pLeftTextArea->getTop() - 3);

  // Re-create the gadgets with new position and size
  // TODO handle failure.
  createGadgets();

  AddGList(m_pWindow, m_pGadtoolsContext, (UWORD)~0, -1, NULL);
  RefreshGList(m_pGadtoolsContext, m_pWindow, NULL, -1);
  FreeGadgets(pOldContext);

  // Perform the initial refresh of all the GadTools. Mandatory.
  GT_RefreshWindow(m_pWindow, NULL);

  if(m_pDocument == NULL)
  {
    return;
  }

  // Display the document file names in the gadgets
  GT_SetGadgetAttrs(m_pGadTxtLeftFile,
                    m_pWindow,
                    NULL,
                    GTTX_Text, (ULONG)m_pDocument->getLeftFileName(),
                    TAG_DONE);

  GT_SetGadgetAttrs(m_pGadTxtRightFile,
                    m_pWindow,
                    NULL,
                    GTTX_Text, (ULONG)m_pDocument->getRightFileName(),
                    TAG_DONE);
}


void DiffWindow::paintWindowDecoration()
{
  if((m_pLeftTextArea == NULL) || (m_pRightTextArea == NULL))
  {
    return;
  }

  // Create borders for the two text areas
  DrawBevelBox(m_pRPorts->Window(),
               m_pLeftTextArea->getLeft(),
               m_pLeftTextArea->getTop(),
               m_pLeftTextArea->getWidth(),
               m_pLeftTextArea->getHeight(),
               GT_VisualInfo, (ULONG)m_Screen.getGadtoolsVisualInfo(),
               GTBB_Recessed, TRUE,
               TAG_DONE);

  DrawBevelBox(m_pRPorts->Window(),
               m_pRightTextArea->getLeft(),
               m_pRightTextArea->getTop(),
               m_pRightTextArea->getWidth(),
               m_pRightTextArea->getHeight(),
               GT_VisualInfo, (ULONG)m_Screen.getGadtoolsVisualInfo(),
               GTBB_Recessed, TRUE,
               TAG_DONE);
}


void DiffWindow::paintStatusBar()
{
  if(!isOpen() || (m_pLeftTextArea == NULL))
  {
    return;
  }

  int top = m_pLeftTextArea->getTop() + m_pLeftTextArea->getHeight() + m_InnerWindowBottom;
  top /= 2;
  top -= m_Screen.getIntuiDrawInfo()->dri_Font->tf_Baseline;
  top++;

  int left = m_pLeftTextArea->getLeft() + 2;

  // Clear the status bar area
  RectFill(m_pRPorts->APenBackgr(),
           0,
           top,
           m_InnerWindowRight,
           m_InnerWindowBottom);


  struct IntuiText intuiText;
  intuiText.FrontPen  = m_Pens.NormalText();
  intuiText.BackPen   = m_Pens.Background();
  intuiText.DrawMode  = JAM2;
  intuiText.ITextFont = &m_TextAttr;
  intuiText.NextText  = NULL;

  intuiText.TopEdge   = top;
  intuiText.LeftEdge  = left;
  intuiText.IText = (UBYTE*) m_StatusBarText;
  PrintIText(m_pRPorts->Window(), &intuiText, 0, 0);

  left += IntuiTextLength(&intuiText);
  intuiText.LeftEdge = left;
  intuiText.BackPen = m_Pens.Green();
  intuiText.IText = (UBYTE*) m_AddedText;
  PrintIText(m_pRPorts->Window(), &intuiText, 0, 0);

  left += IntuiTextLength(&intuiText) + 5;
  intuiText.LeftEdge = left;
  intuiText.BackPen = m_Pens.Yellow();
  intuiText.IText = (UBYTE*) m_ChangedText;
  PrintIText(m_pRPorts->Window(), &intuiText, 0, 0);

  left += IntuiTextLength(&intuiText) + 5;
  intuiText.LeftEdge = left;
  intuiText.BackPen = m_Pens.Red();
  intuiText.IText = (UBYTE*) m_DeletedText;
  PrintIText(m_pRPorts->Window(), &intuiText, 0, 0);
}

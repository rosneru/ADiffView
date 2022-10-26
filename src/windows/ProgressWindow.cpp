#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/gadtools_protos.h>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/gadtools.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
#endif

#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <libraries/gadtools.h>

#include <string.h>
#include <stdio.h>

#include "ProgressMessage.h"
#include "ProgressWindow.h"


ProgressWindow::ProgressWindow(ScreenBase& screen,
                               const ADiffViewPens& pens,
                               struct MsgPort* pIdcmpMsgPort,
                               bool& isCancelRequested,
                               MenuBase* pMenu)
  : WindowBase(screen, pIdcmpMsgPort, pMenu),
    m_Pens(pens),
    m_IsCancelRequested(isCancelRequested),
    m_NewGadget(),
    m_pGadtoolsContext(NULL),
    m_pGadBtnStop(NULL),
    m_pTextZero("0%"),
    m_pTextHundred("100%")
{
  const char* pErrMsg = "ProgressWindow: Failed to create gadgets.";

  m_Width = 230;
  ULONG buttonWidth = 60;

  ULONG xOffset = 6;
  ULONG yOffset = 6;

  struct Screen* pScr = m_Screen.getIntuiScreen();
  if(m_Screen.isHiresMode())
  {
    // When in hires mode the pixels are none-square. They are twice
    // as high as wide, and this is corrected here.
    yOffset /= 2;
  }

  m_OuterRect.set(pScr->WBorLeft + xOffset,
                  pScr->WBorTop + pScr->BarHeight + yOffset,
                  m_Width - pScr->WBorRight - 1 - xOffset,
                  pScr->WBorTop + pScr->BarHeight + yOffset + 3 * m_pTextFont->tf_YSize);

  m_TextZeroWidth = TextLength(&pScr->RastPort, m_pTextZero, 2);
  m_TextHundredWidth = TextLength(&pScr->RastPort, m_pTextHundred, 4);

  m_ProgressRect.set(m_OuterRect.getLeft() + m_TextZeroWidth + 2 * xOffset,
                     m_OuterRect.getTop() + m_pTextFont->tf_YSize,
                     m_OuterRect.getRight() - m_TextHundredWidth - 2 * xOffset,
                     m_OuterRect.getTop() + 2 * m_pTextFont->tf_YSize);

  //
  // Setting up the gadgets
  //

  // Create GadTools context data
  m_pGadtoolsContext = (struct Gadget*) CreateContext(&m_pGadtoolsContext);
  if(m_pGadtoolsContext == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Setting the first gadget of the gadget list for the window
  setFirstGadget(m_pGadtoolsContext);

  // Declare the basic gadget structure and fill with basic values
  m_NewGadget.ng_TextAttr   = m_Screen.getIntuiTextAttr();
  m_NewGadget.ng_VisualInfo = m_Screen.getGadtoolsVisualInfo();

  // Creating the 'Stop' button gadget
  m_NewGadget.ng_LeftEdge   = m_Width / 2 - buttonWidth / 2;
  m_NewGadget.ng_TopEdge    = m_OuterRect.getBottom() + yOffset;
  m_NewGadget.ng_Width      = buttonWidth;
  m_NewGadget.ng_Height     = m_pTextFont->tf_YSize + 2 * yOffset;
  m_NewGadget.ng_GadgetID   = GID_BtnStop;
  m_NewGadget.ng_GadgetText = (UBYTE*) "Stop";
  m_NewGadget.ng_Flags      = 0;

  m_Height = m_NewGadget.ng_TopEdge + m_NewGadget.ng_Height + yOffset + pScr->WBorBottom;

  // Setting window title
  setTitle("Progress window");

  // Setting the window flags
  addFlags(WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget


  // Setting the IDCMP messages we want to receive for this window
  addIDCMP(IDCMP_MENUPICK |       // Get msg when menu selected
           IDCMP_VANILLAKEY |     // Get msg when RAW key pressed
           IDCMP_RAWKEY |         // Get msg when printable key pressed
           IDCMP_REFRESHWINDOW |  // Get msg when must refreshing
           IDCMP_GADGETUP);       // Get msg when gadgets changed
}


ProgressWindow::~ProgressWindow()
{
  cleanup();
}


void ProgressWindow::cleanup()
{
  close();

  if(m_pGadtoolsContext != NULL)
  {
    FreeGadgets(m_pGadtoolsContext);
    m_pGadtoolsContext = NULL;
    m_pGadBtnStop = NULL;
  }
}


void ProgressWindow::Refresh()
{
// TODO
//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool ProgressWindow::open(InitialPosition initialPos)
{
  if(!WindowBase::open(initialPos))
  {
    return false;
  }

  // Enable the Stop button in case it has been disabled
  GT_SetGadgetAttrs(m_pGadBtnStop, getIntuiWindow(), NULL,
                    GA_Disabled, FALSE,
                    TAG_DONE);

  // Set the windows rastport to fill with a pattern for the structured
  // background
  UWORD areafillPattern[2] = {0xaaaa, 0x5555};
  m_pWindow->RPort->AreaPtrn = areafillPattern;
  m_pWindow->RPort->AreaPtSz = 1;

  struct Screen* pScreen = m_Screen.getIntuiScreen();

  // Fill the structured background
  SetDrMd(m_pWindow->RPort, JAM1);
  SetAPen(m_pWindow->RPort, m_Pens.HighlightedText());
  RectFill(m_pWindow->RPort,
           pScreen->WBorLeft,
           pScreen->WBorTop + pScreen->BarHeight - 1,
           m_pWindow->Width - pScreen->WBorRight - 1,
           m_pWindow->Height - pScreen->WBorBottom - 1);

  // Now set the windows RastPort to fill without a pattern
  m_pWindow->RPort->AreaPtrn = NULL;

  // Draw the outer gray area and bevel box
  SetAPen(m_pWindow->RPort, m_Pens.Background());
  RectFill(m_pWindow->RPort,
           m_OuterRect.getLeft(),
           m_OuterRect.getTop(),
           m_OuterRect.getRight() - 1,
           m_OuterRect.getBottom() - 1);

  DrawBevelBox(m_pWindow->RPort,
               m_OuterRect.getLeft(),
               m_OuterRect.getTop(),
               m_OuterRect.getWidth(),
               m_OuterRect.getHeight(),
               GT_VisualInfo, (ULONG)m_Screen.getGadtoolsVisualInfo(),
               GTBB_Recessed, TRUE,
               TAG_DONE);

  // Draw a bevel box around the area where the progress bar will be
  DrawBevelBox(m_pWindow->RPort,
               m_ProgressRect.getLeft() - 2,
               m_ProgressRect.getTop() - 1,
               m_ProgressRect.getWidth() + 4,
               m_ProgressRect.getHeight() + 2,
               GT_VisualInfo, (ULONG)m_Screen.getGadtoolsVisualInfo(),
               GTBB_Recessed, TRUE,
               TAG_DONE);

  // Draw the '0%' and '100%' texts
  SetAPen(m_pWindow->RPort,m_Pens.NormalText());

  Move(m_pWindow->RPort,
       (m_OuterRect.getLeft() + m_ProgressRect.getLeft()) / 2 - m_TextZeroWidth / 2,
       m_ProgressRect.getTop() + m_pTextFont->tf_Baseline + 1);

  Text(m_pWindow->RPort, m_pTextZero, strlen(m_pTextZero));

  Move(m_pWindow->RPort,
       (m_OuterRect.getRight() + m_ProgressRect.getRight()) / 2 - m_TextHundredWidth / 2,
       m_ProgressRect.getTop() + m_pTextFont->tf_Baseline + 1);

  Text(m_pWindow->RPort, m_pTextHundred, strlen(m_pTextHundred));

  // Create the stop gadget. This is done here and not in the
  // constructor because it has to sit above the structure background.
  m_pGadBtnStop = CreateGadget(BUTTON_KIND,
                               m_pGadtoolsContext, &m_NewGadget,
                               TAG_DONE);
  if(m_pGadBtnStop == NULL)
  {
    return false;
  }

  RefreshGList(m_pGadtoolsContext, m_pWindow, NULL, -1);

  // Perform the initial refresh of all the GadTools. Mandatory.
  GT_RefreshWindow(m_pWindow, NULL);

  return true;
}


void ProgressWindow::handleIDCMP(ULONG msgClass,
                                 UWORD msgCode,
                                 APTR pItemAddress)
{
  if(!isOpen())
  {
    return;
  }

  switch (msgClass)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) pItemAddress;
      switch(pGadget->GadgetID)
      {
        case GID_BtnStop:
        {
          // Set the flag which will stop background process as soon as
          // possible
          m_IsCancelRequested = true;

          // Disable the Stop button
          GT_SetGadgetAttrs(m_pGadBtnStop, getIntuiWindow(), NULL,
                            GA_Disabled, TRUE,
                            TAG_DONE);
          break;
        }
      }

      break;
    }

    case IDCMP_REFRESHWINDOW:
    {
      // This handling is REQUIRED with GadTools
      GT_BeginRefresh(getIntuiWindow());
      GT_EndRefresh(getIntuiWindow(), TRUE);
      break;
    }
  }
}


void ProgressWindow::HandleProgress(struct ProgressMessage* pProgrMsg)
{
  if(!isOpen())
  {
    return;
  }

  if(pProgrMsg == NULL)
  {
    return;
  }

  if(pProgrMsg->progress < 0)
  {
    m_ProgressDescr = "";
    return;
  }

  int progressWidth = 1;
  if(pProgrMsg->progress > 0)
  {
    progressWidth = (m_ProgressRect.getWidth() - 2) * pProgrMsg->progress / 100;
  }

  // Set color to <blue> for painting the progress bar
  SetAPen(m_pWindow->RPort, m_Pens.Fill());

  // Fill the progress bar area
  RectFill(m_pWindow->RPort,
           m_ProgressRect.getLeft(),
           m_ProgressRect.getTop(),
           m_ProgressRect.getLeft() + progressWidth,
           m_ProgressRect.getBottom());

  // Set color to <background> for painting the grey background of the
  // yet uncovered area of the progress bar
  SetAPen(m_pWindow->RPort,m_Pens.Background());

  // NOTE: The following condition is a workaround because std::string
  // currently has no != overload
  if(!(m_ProgressDescr == pProgrMsg->pDescription))
  {
    // Update progress description as it has changed
    m_ProgressDescr = pProgrMsg->pDescription;

    // Update the description in window title
    SetWindowTitles(m_pWindow, m_ProgressDescr.c_str(), (STRPTR)~0);
  }
}
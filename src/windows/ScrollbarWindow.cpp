#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
  #include <clib/utility_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
  #include <proto/utility.h>
#endif

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>

#include <string.h>

#include "ScrollbarWindow.h"

ScrollbarWindow::ScrollbarWindow(ScreenBase& screen,
                                 struct MsgPort* pIdcmpMsgPort,
                                 MenuBase* pMenu)
  : WindowBase(screen, pIdcmpMsgPort, pMenu),
    m_InnerWindowRight(0),
    m_InnerWindowBottom(0),
    m_MaxVisibleLines(0),
    m_SizeGadWidth(18),
    m_SizeGadHeight(10),
    m_pImgLeftArrow(NULL),
    m_pImgRightArrow(NULL),
    m_pImgUpArrow(NULL),
    m_pImgDownArrow(NULL),
    m_pGadPropX(NULL),
    m_pGadPropY(NULL),
    m_pGadBtnLeftArrow(NULL),
    m_pGadBtnRightArrow(NULL),
    m_pGadBtnUpArrow(NULL),
    m_pGadBtnDownArrow(NULL)
{
  const char* pErrMsg = "ScrollbarWindow: NewObj() failed.";

  //
  // Create scroll bars and gadgets for the window. They will be
  // attached to the window at opening time
  //
  ULONG width = 0;
  ULONG height = 0;

  // Getting the width and height of the current system size gadget
  struct Image* pSizeImage = createImageObj(SIZEIMAGE, 
                                            m_SizeGadWidth, 
                                            m_SizeGadHeight);

  // As the size image is only needed for getting the width and height 
  // of the size gadget it can be disposed right now
  if(pSizeImage != NULL)
  {
    DisposeObject(pSizeImage);
    pSizeImage = NULL;
  }

  // Create the arrow down image and getting its width and height
  m_pImgDownArrow = createImageObj(DOWNIMAGE, width, height);
  if(m_pImgDownArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the arrow down gadget
  m_pGadBtnDownArrow = (struct Gadget*)
    NewObject(NULL, BUTTONGCLASS,
              GA_ID, GID_ArrowDown,
              GA_RelRight, -width+1,
              GA_RelBottom, -m_SizeGadHeight-height+1,
              GA_Width, width,
              GA_Height, height,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_RightBorder, TRUE,
              GA_Image, (ULONG)m_pImgDownArrow,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);
  if(m_pGadBtnDownArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the arrow up image and getting its width and height
  m_pImgUpArrow = createImageObj(UPIMAGE, width, height);
  if(m_pImgUpArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the arrow down gadget
  m_pGadBtnUpArrow = (struct Gadget*)
    NewObject(NULL, BUTTONGCLASS,
              GA_Previous, (ULONG)m_pGadBtnDownArrow,
              GA_ID, GID_ArrowUp,
              GA_RelRight, -width+1,
              GA_RelBottom, -m_SizeGadHeight-height-height+1,
              GA_Width, width,
              GA_Height, height,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_RightBorder, TRUE,
              GA_Image, (ULONG)m_pImgUpArrow,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);
  if(m_pGadBtnUpArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the vertical proportional gadget / slider
  ULONG barHeight = m_Screen.getIntuiScreen()->BarHeight;
  ULONG wborTop = m_Screen.getIntuiScreen()->WBorTop;
  ULONG wborBottom = m_Screen.getIntuiScreen()->WBorBottom;

	m_pGadPropY = (struct Gadget*)
    NewObject(NULL, PROPGCLASS,
              GA_Previous, (ULONG)m_pGadBtnUpArrow,
              GA_ID, GID_PropY,
              GA_RelRight, -m_SizeGadWidth+4,
              GA_Top, barHeight + 2,
              GA_Width, m_SizeGadWidth-6,
              GA_RelHeight, - m_SizeGadHeight - height - height
                            - barHeight - wborTop - wborBottom,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_RightBorder, TRUE,
              PGA_Freedom, FREEVERT,
              PGA_Borderless, TRUE,
              PGA_NewLook, TRUE,
              PGA_Total, 100,
              PGA_Top, 0, // TODO remove??
              PGA_Visible, 100,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);

  // Create the arrow left image and getting its width and height
  m_pImgRightArrow = createImageObj(RIGHTIMAGE,
                                      width,
                                      height);
  if(m_pImgRightArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }
  
  // Create the arrow right gadget
  m_pGadBtnRightArrow = (struct Gadget*)
    NewObject(NULL, BUTTONGCLASS,
              GA_Previous, (ULONG)m_pGadPropY,
              GA_ID, GID_ArrowRight,
              GA_RelRight, -m_SizeGadWidth-width+1,
              GA_RelBottom, -height+1,
              GA_Width, width,
              GA_Height, height,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_BottomBorder, TRUE,
              GA_Image, (ULONG)m_pImgRightArrow,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);
  if(m_pGadBtnRightArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the arrow left image and getting its width and height
  m_pImgLeftArrow = createImageObj(LEFTIMAGE, width, height);
  if(m_pImgLeftArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the arrow left gadget
  m_pGadBtnLeftArrow = (struct Gadget*)
    NewObject(NULL, BUTTONGCLASS,
              GA_Previous, (ULONG)m_pGadBtnRightArrow,
              GA_ID, GID_ArrowLeft,
              GA_RelRight, -m_SizeGadWidth-width-width+1,
              GA_RelBottom, -height+1,
              GA_Width, width,
              GA_Height, height,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_BottomBorder, TRUE,
              GA_Image, (ULONG)m_pImgLeftArrow,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);
  if(m_pGadBtnLeftArrow == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the horizontal proportional gadget / slider
  BYTE wborLeft = m_Screen.getIntuiScreen()->WBorLeft;
  BYTE wborRight = m_Screen.getIntuiScreen()->WBorRight;

  m_pGadPropX = (struct Gadget*)
    NewObject(NULL, PROPGCLASS,
              GA_Previous, (ULONG)m_pGadBtnLeftArrow,
              GA_ID, GID_PropX,
              GA_Left, (ULONG)m_Screen.getIntuiScreen()->WBorLeft,
              GA_RelBottom, -m_SizeGadHeight+3,
              GA_RelWidth, - m_SizeGadWidth - width - width
                           - wborLeft - wborRight,
              GA_Height, m_SizeGadHeight-4,
              GA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
              GA_GZZGadget, TRUE,
              GA_BottomBorder, TRUE,
              PGA_Freedom, FREEHORIZ,
              PGA_Borderless, TRUE,
              PGA_NewLook, TRUE,
              PGA_Total, 100,
              //PGA_Left, 0,  // TODO remove??
              PGA_Visible, 100,
              ICA_TARGET, ICTARGET_IDCMP,
              TAG_DONE, 0);
  if(m_pGadPropX == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Set the default title
  setTitle("ScrollbarWindow");

  // Setting the IDCMP messages we want to receive for this window
  addIDCMP(IDCMP_VANILLAKEY |     // Inform about normal key presses
           IDCMP_RAWKEY |         // Inform about RAW key presses
           IDCMP_IDCMPUPDATE);    // Inform about BOOPSI gadget updates

  // Setting the first gadget of the gadget list for the window
  setFirstGadget(m_pGadBtnDownArrow);
}

ScrollbarWindow::~ScrollbarWindow()
{
  cleanup();
}


bool ScrollbarWindow::open(InitialPosition initialPos)
{
  if(!WindowBase::open(initialPos))
  {
    return false;
  }

  return true;
}


void ScrollbarWindow::handleIDCMP(const struct IntuiMessage* pMsg)
{
  if(!isOpen())
  {
    return;
  }

  switch (pMsg->Class)
  {
    case IDCMP_IDCMPUPDATE:
    {
      ULONG tagData = GetTagData(GA_ID, 0, (struct TagItem *)pMsg->IAddress);
      switch(tagData)
      {
        case ScrollbarWindow::GID_PropX:
        {
          // Get the top value of the prop gadget from the tag data of
          // the message
          size_t newX = GetTagData(PGA_Top, 0, (struct TagItem *) pMsg->IAddress);

          // If there are other GID_PropX addressed to this window,
          // delete them and get the top value from the latest found
          // message.
          extractLatestPropGadTopValue(GID_PropX, newX);

          // Pass the latest found newX value to the childs which have
          // to implement this abstract method
          handleXChange(newX);

          break;
        }

        case ScrollbarWindow::GID_PropY:
        {
          // Get the top value of the prop gadget from the tag data of
          // the message
          size_t newY = GetTagData(PGA_Top,
                                   0,
                                   (struct TagItem *) pMsg->IAddress);

          // If there are other GID_PropY addressed to this window,
          // delete them and get the top value from the latest found
          // message.
          extractLatestPropGadTopValue(GID_PropY, newY);

          // Pass the latest found newY value to the childs which have
          // to implement this abstract method
          handleYChange(newY);

          break;
        }

        case ScrollbarWindow::GID_ArrowLeft:
        {
          handleXDecrease(2);
          break;
        }

        case ScrollbarWindow::GID_ArrowRight:
        {
          handleXIncrease(2);
          break;
        }

        case ScrollbarWindow::GID_ArrowUp:
        {
          handleYDecrease(2);
          break;
        }

        case ScrollbarWindow::GID_ArrowDown:
        {
          handleYIncrease(2);
          break;
        }

      }
      break;
    }

    case IDCMP_RAWKEY:
    {
      if(pMsg->Code == CURSORLEFT)
      {
        handleXDecrease(2);
      }
      else if(pMsg->Code == CURSORRIGHT)
      {
        handleXIncrease(2);
      }
      else if(pMsg->Code == CURSORDOWN)
      {
        handleYIncrease(2);
      }
      else if(pMsg->Code == CURSORUP)
      {
        handleYDecrease(2);
      }
      break;
    }
    case IDCMP_VANILLAKEY:
    {
      if(pMsg->Code == 32) // SPACE
      {
        handleYIncrease(m_MaxVisibleLines);
      }
      else if(pMsg->Code == 8) // BACKSPACE
      {
        handleYDecrease(m_MaxVisibleLines);
      }


      break;
    }
  }
}


void ScrollbarWindow::calcSizes()
{
  // Calculate the inner window dimensions
  m_InnerWindowRight = m_pWindow->Width
                     - m_Screen.getIntuiScreen()->WBorLeft
                     - m_SizeGadWidth;

  m_InnerWindowBottom = m_pWindow->Height //- m_Screen.getIntuiScreen()->BarHeight
                      - m_pWindow->BorderTop
                      - m_SizeGadHeight;
}


void ScrollbarWindow::setXScrollTop(int left)
{
  if(m_pGadPropX == NULL)
  {
    return;
  }

  if(left < 0)
  {
    return;
  }

  SetGadgetAttrs(m_pGadPropX, m_pWindow, NULL,
                 PGA_Top, left,
                 TAG_DONE);
}


void ScrollbarWindow::setYScrollTop(int top)
{
  if(m_pGadPropY == NULL)
  {
    return;
  }

  if(top < 0)
  {
    return;
  }

  SetGadgetAttrs(m_pGadPropY, m_pWindow, NULL,
                 PGA_Top, top,
                 TAG_DONE);
}


void ScrollbarWindow::setXScrollPotSize(int maxVisibleChars,
                                        int totalChars)
{
  if(m_pGadPropX == NULL)
  {
    return;
  }

  if(totalChars < 0)
  {
    totalChars = maxVisibleChars;
  }

  // Set the size of x-scrollbar "potentiometer"
  SetGadgetAttrs(m_pGadPropX,
                 m_pWindow,
                 NULL,
                 PGA_Total, totalChars,
                 PGA_Visible, maxVisibleChars,
                 TAG_DONE);

}


void ScrollbarWindow::setYScrollPotSize(int maxVisibleLines,
                                        int totalLines)
{
  if(m_pGadPropY == NULL)
  {
    return;
  }

  m_MaxVisibleLines = maxVisibleLines;

  if(totalLines < 0)
  {
    totalLines = maxVisibleLines;
  }

  // Set the size of y-scrollbar "potentiometer"
  SetGadgetAttrs(m_pGadPropY,
                 m_pWindow,
                 NULL,
                 PGA_Total, totalLines,
                 PGA_Visible, maxVisibleLines,
                 TAG_DONE);

}


void ScrollbarWindow::cleanup()
{
  if(m_pGadBtnLeftArrow != NULL)
  {
    DisposeObject(m_pGadBtnLeftArrow);
    m_pGadBtnLeftArrow = NULL;
  }

  if(m_pImgLeftArrow != NULL)
  {
    DisposeObject(m_pImgLeftArrow);
    m_pImgLeftArrow = NULL;
  }

  if(m_pGadBtnRightArrow != NULL)
  {
    DisposeObject(m_pGadBtnRightArrow);
    m_pGadBtnRightArrow = NULL;
  }

  if(m_pImgRightArrow != NULL)
  {
    DisposeObject(m_pImgRightArrow);
    m_pImgRightArrow = NULL;
  }

  if(m_pGadPropX != NULL)
  {
    DisposeObject(m_pGadPropX);
    m_pGadPropX = NULL;
  }

  if(m_pGadBtnDownArrow != NULL)
  {
    DisposeObject(m_pGadBtnDownArrow);
    m_pGadBtnDownArrow = NULL;
  }

  if(m_pImgDownArrow != NULL)
  {
    DisposeObject(m_pImgDownArrow);
    m_pImgDownArrow = NULL;
  }

  if(m_pGadBtnUpArrow != NULL)
  {
    DisposeObject(m_pGadBtnUpArrow);
    m_pGadBtnUpArrow = NULL;
  }

  if(m_pImgUpArrow != NULL)
  {
    DisposeObject(m_pImgUpArrow);
    m_pImgUpArrow = NULL;
  }

  if(m_pGadPropY != NULL)
  {
    DisposeObject(m_pGadPropY);
    m_pGadPropY = NULL;
  }
}


void ScrollbarWindow::extractLatestPropGadTopValue(GadgetId gadgetId,
                                                   size_t& lastVal)
{
  Forbid();

  struct MsgPort* pIdcmpMsgPort = m_pWindow->UserPort;

  struct IntuiMessage* pMessage;
  pMessage = (struct IntuiMessage*) pIdcmpMsgPort->mp_MsgList.lh_Head;

  struct Node* pSuccessor;

  while((pSuccessor = pMessage->ExecMessage.mn_Node.ln_Succ) != NULL)
  {
    if(pMessage->IDCMPWindow == m_pWindow &&
        pMessage->Class == IDCMP_IDCMPUPDATE)
    {
      ULONG msgTagData = GetTagData(GA_ID, 0,
        (struct TagItem *)pMessage->IAddress);

      if(msgTagData == gadgetId)
      {
        lastVal = GetTagData(PGA_Top, 0, (struct TagItem*)
          pMessage->IAddress);

        // Intuition is about to free this message. Make sure
        // that we have politely sent it back.
        Remove((struct Node*) pMessage);
        ReplyMsg((struct Message*) pMessage);
      }
    }

    pMessage = (struct IntuiMessage*) pSuccessor;
  }

  // Enable multi tasking
  Permit();
}


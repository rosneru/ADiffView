#ifdef __clang__
  #include <clib/exec_protos.h>
  #include <clib/gadtools_protos.h>>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
  #include <clib/wb_protos.h>
#else
  #include <proto/exec.h>
  #include <proto/gadtools.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
  #include <proto/wb.h>
#endif

#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>

#include <string.h>

#include "WindowBase.h"

extern struct GfxBase* GfxBase;

WindowBase::WindowBase(ScreenBase& screen,
                       struct MsgPort* pIdcmpMsgPort,
                       MenuBase* pMenu)
  : m_Screen(screen),
    m_pIdcmpMsgPort(pIdcmpMsgPort),
    m_pWindow(NULL),
    m_bBorderless(false),
    m_Left(0),
    m_Top(0),
    m_Width(0),
    m_Height(0),
    m_pMenu(pMenu),
    m_pTextFont(((struct GfxBase *)GfxBase)->DefaultFont),
    m_bIsFixed(false),
    m_InitialPosition(IP_Center),
    m_WindowFlags(0),
    m_WindowIdcmp(0),
    m_pFirstGadget(NULL),
    m_pAppWindowPort(NULL),
    m_pAppWindow(NULL),
    m_AppWindowId(0)
{
}


WindowBase::~WindowBase()
{
  close();
}

void WindowBase::performResize()
{
  // Resizable childs should override this and re-paint their contents
  // according the new size.
}

bool WindowBase::open(InitialPosition initialPos)
{
  m_InitialPosition = initialPos;

  //
  // Initial validations
  //

  if(isOpen())
  {
    // Not opening the window if it is already open
    // TODO Alternatively: bring window to front and return true;
    return true;
  }

  //
  // Calculating window size etc in dependency of screen dimensions
  //
  int screenWidth = m_Screen.getIntuiScreen()->Width;
  int screenHeight = m_Screen.getIntuiScreen()->Height;
  int screenBarHeight = m_Screen.getIntuiScreen()->BarHeight;

  switch(m_InitialPosition)
  {
    case IP_Center:
      if(m_Width == 0)
      {
        // Initial window width not set in initialize()
        m_Width = screenWidth * 60 / 100;
      }

      if(m_Height == 0)
      {
        // Initial window height not set in initialize()
        m_Height = screenHeight * 60 / 100;
      }

      m_Top = screenHeight / 2 - m_Height / 2;
      m_Left = screenWidth / 2 - m_Width / 2;
      break;

    case IP_Fill:
      m_Width = screenWidth;
      m_Height = screenHeight - screenBarHeight - 1;
      m_Top = screenBarHeight + 1;
      m_Left = 0;
      break;

    case IP_Left:
      m_Width = screenWidth / 2;
      m_Height = screenHeight - screenBarHeight - 1;
      m_Top = screenBarHeight + 1;
      m_Left = 0;
      break;

    case IP_Right:
      m_Width = screenWidth / 2;
      m_Height = screenHeight - screenBarHeight - 1;
      m_Top = screenBarHeight + 1;
      m_Left = m_Width;
      break;

    case IP_Explicit:
      // Nothing to do
      break;
  }

  if(!m_bIsFixed)
  {
    // Add a dragbar -> make window movable
    m_WindowFlags |= WFLG_DRAGBAR;
  }

  // Define a minimum height that won't crash ADiffViewwhen 
  // performResize() is called in DiffWindow. 
  // TODO: This is just a quick fix. Find something better.B
  ULONG minHeight = 8 * m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;

  // Open the window
  m_pWindow = OpenWindowTags(NULL,
                             WA_SmartRefresh, TRUE,
                             WA_Left, m_Left,
                             WA_Top, m_Top,
                             WA_Width, m_Width,
                             WA_Height, m_Height,
                             WA_Title, (ULONG) m_Title.c_str(),
                             WA_Activate, TRUE,
                             WA_PubScreen, (ULONG) m_Screen.getIntuiScreen(),
                             WA_Flags, m_WindowFlags,
                             WA_MinWidth, 230,
                             WA_MinHeight, minHeight,
                             WA_MaxWidth, ~0,
                             WA_MaxHeight, ~0,
                             WA_NewLookMenus, TRUE,
                             WA_Borderless, m_bBorderless,
                             WA_Gadgets, (ULONG)m_pFirstGadget,
                             TAG_DONE);

  if(!isOpen())
  {
    // Opening failed
    return false;
  }

  m_Screen.increaseNumOpenWindows();

  // The window uses this message port which can be the same as used by
  // other windows
  m_pWindow->UserPort = m_pIdcmpMsgPort;

  ModifyIDCMP(m_pWindow, m_WindowIdcmp);

  // Create an AppWindow if requested
  if(m_pAppWindowPort != NULL)
  {
    m_pAppWindow = AddAppWindow(m_AppWindowId,
                                0,
                                m_pWindow,
                                m_pAppWindowPort,
                                TAG_DONE, 0);
  }

  if(m_pMenu == NULL)
  {
    // Opening successful, but at the moment there's no menu set to
    //the window
    return true;
  }

  // Create the menu if; returns true if already done
  if(m_pMenu->Layout(m_Screen) == true)
  {
    // Attach the menu to the window
    m_pMenu->AttachToWindow(m_pWindow);
  }

  return true;
}


void WindowBase::close()
{
  if(!isOpen())
  {
    return;
  }

  if(m_pMenu != NULL)
  {
    m_pMenu->DetachFromWindow(m_pWindow);
  }

  if(m_pAppWindow != NULL)
  {
    RemoveAppWindow(m_pAppWindow);
    m_pAppWindow = NULL;
  }

  closeWindowSafely();
  m_Screen.decreaseNumOpenWindows();
}


bool WindowBase::isOpen() const
{
  return m_pWindow != NULL;
}


const char* WindowBase::getTitle() const
{
  return m_Title.c_str();
}


void WindowBase::setTitle(const char* pNewTitle)
{
  if(pNewTitle == NULL)
  {
    return;
  }

  m_Title = pNewTitle;

  if(!isOpen())
  {
    // Window is not open, so we don't change its title dynamically
    return;
  }

  // Call intuition function to set the window title Note the ~0 inverts
  // the value and is a value of -1. That means the screen title remains
  // unchanged.
  SetWindowTitles(m_pWindow, m_Title.c_str(), (STRPTR) ~0);
}


void WindowBase::setInitialDimension(ULONG left,
                                     ULONG top,
                                     ULONG width,
                                     ULONG height)
{
  if(isOpen())
  {
    return;
  }

  m_InitialPosition = WindowBase::IP_Explicit;
  m_Left = left;
  m_Top = top;
  m_Width = width;
  m_Height = height;
}


void WindowBase::setFixed(bool bFixWindow)
{
  if(isOpen())
  {
    return;
  }

  m_bIsFixed = bFixWindow;
}


void WindowBase::setBorderless(bool bBorderless)
{
  if(isOpen())
  {
    return;
  }

  m_bBorderless = bBorderless;
}


void WindowBase::enableAppWindow(struct MsgPort* pAppWindowPort,
                                 ULONG appWindowId)
{
  if(isOpen())
  {
    return;
  }

  m_pAppWindowPort = pAppWindowPort;
  m_AppWindowId = appWindowId;
}


struct Window* WindowBase::getIntuiWindow()
{
  return m_pWindow;
}


void WindowBase::setMenu(MenuBase* pMenu)
{
  if(pMenu == NULL)
  {
    // Given menu is NULL: do nothing
    return;
  }

  if(m_pMenu != NULL)
  {
    // There is already a menu set on this window : do nothing
    return;
  }

  m_pMenu = pMenu;


  if(!isOpen())
  {
    // The window isn't open yet: don't attach the menu now
    return;
  }

  // Create the menu; returns true if already done
  if(m_pMenu->Layout(m_Screen) == true)
  {
    // Attach the menu to the window
    m_pMenu->AttachToWindow(m_pWindow);
  }

  return;
}

MenuBase* WindowBase::getMenu()
{
  return m_pMenu;
}


ScreenBase& WindowBase::getScreen()
{
  return m_Screen;
}

size_t WindowBase::maxArrayTextLength(const char** ppArrayOfTexts, 
                                      size_t arrayNumItems)
{
  Screen* pIntuiScreen = m_Screen.getIntuiScreen();
  if(pIntuiScreen == NULL)
  {
    return 0;
  }

  size_t maxLength = 0;
  for(size_t i = 0; i < arrayNumItems; i++)
  {
    const char* pTxt = ppArrayOfTexts[i];
    if(pTxt != NULL)
    {
      size_t txtWidth = TextLength(&pIntuiScreen->RastPort, pTxt, strlen(pTxt));
      if(txtWidth > maxLength)
      {
        maxLength = txtWidth;
      }
    }
  }

  return maxLength;
}



void WindowBase::setStringGadgetText(struct Gadget* pGadget,
                                      const char* pText)
{
  if(!isOpen() || (pGadget == NULL) || (pText == NULL))
  {
    return;
  }

  GT_SetGadgetAttrs(pGadget, m_pWindow, NULL,
                    GTST_String, (ULONG)pText,
                    TAG_DONE);
}

STRPTR WindowBase::getStringGadgetText(struct Gadget* pGadget)
{
  if(!isOpen() || pGadget == NULL)
  {
    return NULL;
  }

  long pTextPointerStorage;
  long numProcessed;

  numProcessed  = GT_GetGadgetAttrs(pGadget, m_pWindow, NULL,
                                    GTST_String, (ULONG)&pTextPointerStorage,
                                    TAG_DONE);
  if(numProcessed != 1)
  {
    return NULL;
  }

  return (STRPTR)pTextPointerStorage;
}



void WindowBase::addFlags(ULONG flags)
{
  m_WindowFlags |= flags;
}


void WindowBase::addIDCMP(ULONG idcmp)
{
  m_WindowIdcmp |= idcmp;
}


void WindowBase::setFirstGadget(struct Gadget* pFirstGadget)
{
  if(m_pFirstGadget != NULL)
  {
    return;
  }

  m_pFirstGadget = pFirstGadget;
}


struct Gadget* WindowBase::getFirstGadget()
{
  return m_pFirstGadget;
}


struct Gadget* WindowBase::getLastGadget()
{
  if(m_pFirstGadget == NULL)
  {
    return NULL;
  }

  struct Gadget* pGadget = m_pFirstGadget->NextGadget;
  while(pGadget != NULL)
  {
    if(pGadget->NextGadget == NULL)
    {
      break;
    }

    pGadget = pGadget->NextGadget;
  }

  return pGadget;

}


struct Image* WindowBase::createImageObj(ULONG sysImageId,
                                         ULONG& width,
                                         ULONG& height)
{
  struct Image* pImage;
  pImage = (struct Image*) NewObject(NULL, 
                                     SYSICLASS,
                                     SYSIA_Which, sysImageId,
                                     SYSIA_Size, SYSISIZE_MEDRES,
                                     SYSIA_DrawInfo, (ULONG)m_Screen.getIntuiDrawInfo(),
                                     TAG_DONE);

  if(pImage != NULL)
  {
    GetAttr(IA_Width, pImage, &width);
    GetAttr(IA_Height, pImage, &height);
  }

  return pImage;
}


void WindowBase::closeWindowSafely()
{
  if(!isOpen())
  {
    return;
  }

  // We forbid here to keep out of race conditions with Intuition.
  Forbid();

  // Send back any messages for this window  that have not yet been
  // processed.
  stripIntuiMessages();

  // Clear UserPort so Intuition will not free it.
  m_pWindow->UserPort = NULL;

  // Tell intuition to stop sending more messages
  ModifyIDCMP(m_pWindow, 0L);

  // Turn multitasking back on
  Permit();

  CloseWindow(m_pWindow);
  m_pWindow = NULL;
}


void WindowBase::stripIntuiMessages()
{
  struct MsgPort* pIdcmpMsgPort = m_pWindow->UserPort;
  struct Node* pSuccessor;
  struct IntuiMessage* pMessage;

  pMessage = (struct IntuiMessage*) pIdcmpMsgPort->mp_MsgList.lh_Head;

  while((pSuccessor = pMessage->ExecMessage.mn_Node.ln_Succ) != NULL)
  {
    if(pMessage->IDCMPWindow == m_pWindow)
    {
      // Intuition is about to free this message. Make sure that we
      // have politely sent it back.
      Remove((struct Node*) pMessage);
      ReplyMsg((struct Message*) pMessage);
    }

    pMessage = (struct IntuiMessage*) pSuccessor;
  }
}

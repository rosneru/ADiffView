#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}
#include "AslFileRequest.h"
#include "OpenFilesWindow.h"

OpenFilesWindow::OpenFilesWindow(AppScreen& p_AppScreen,
    struct MsgPort* p_pMsgPort, AmigaDiffFacade& p_DiffFacade)
  : WindowBase(p_AppScreen, p_pMsgPort),
    m_DiffFacade(p_DiffFacade),
    m_pGadgetList(NULL),
    m_pLeftFileStringGadget(NULL),
    m_pRightFileStringGadget(NULL),
    m_pSelectLeftFileButton(NULL),
    m_pSelectRightFileButton(NULL),
    m_pDiffButton(NULL),
    m_pCancelButton(NULL)
{
  // We moved this into the constructor
  // From the destructor. That'll be hilarious.
  if(m_pGadgetList != NULL)
  {
    FreeGadgets(m_pGadgetList);
    m_pGadgetList = NULL;
    m_pLeftFileStringGadget = NULL;
    m_pRightFileStringGadget = NULL;
    m_pSelectLeftFileButton = NULL;
    m_pSelectRightFileButton = NULL;
    m_pDiffButton = NULL;
    m_pCancelButton = NULL;
  }
}

OpenFilesWindow::~OpenFilesWindow()
{
  Close();
}

OpenFilesWindow::NewMethod()
{
  // This method is completely new and shiny.
}


void OpenFilesWindow::Refresh()
{

//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool OpenFilesWindow::Open(APTR p_pUserDataMenuItemToDisable)
{
  if(WindowBase::Open(p_pUserDataMenuItemToDisable) == false)
  {
    return false;
  }

  // Set the Diff button to an initial enabled / disabled state
  setDiffButtonState();

  setStringGadgetText(m_pLeftFileStringGadget, m_DiffFacade.LeftFilePath());
  setStringGadgetText(m_pRightFileStringGadget, m_DiffFacade.RightFilePath());

  return true;
}

void OpenFilesWindow::HandleIdcmp(ULONG p_Class, UWORD p_Code, APTR p_IAddress)
{
  if(!IsOpen())
  {
    return;
  }

  switch (p_Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) p_IAddress;
      if(pGadget->GadgetID == GID_LeftFileButton)
      {
        // Button "..." to select left file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string leftFilePath = m_DiffFacade.LeftFilePath();
        if(selectFile(leftFilePath, "Select left (original) file"))
        {
          setStringGadgetText(m_pLeftFileStringGadget, leftFilePath);
          m_DiffFacade.SetLeftFilePath(leftFilePath.c_str());
          setDiffButtonState();
        }
      }
      else if(pGadget->GadgetID == GID_RightFileButton)
      {
        // Button "..." to select right file was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // Open an ASL file request to let the user select the file
        std::string rightFilePath = m_DiffFacade.RightFilePath();
        if(selectFile(rightFilePath, "Select right (changed) file"))
        {
          setStringGadgetText(m_pRightFileStringGadget, rightFilePath);
          m_DiffFacade.SetRightFilePath(rightFilePath.c_str());
          setDiffButtonState();

        }
      }
      else if(pGadget->GadgetID == GID_CancelButton)
      {
        // Button "Cancel" was clicked
        Close();
      }
      else if(pGadget->GadgetID == GID_DiffButton)
      {
        // Button "Diff" was clicked

        // Read latest string gadgets contents before continue
        readStringGadgetsText();

        // If now one of the texts is empty, do not perform the Diff
        if(m_DiffFacade.LeftFilePath().length() == 0 ||
           m_DiffFacade.RightFilePath().length() == 0)
        {
          return;
        }

        // Disable all gadgets while diff is performing
        disableAll();

        // Perform the diff
        if(m_DiffFacade.Diff() == true)
        {
          // Diff was successful. Left and right diff windows should
          // be open now, so this window can be closed
          enableAll();
          Close();
        }
        else
        {
          // TODO FileRequest to inform the user about diff error
          enableAll();
        }
      }
      else if(pGadget->GadgetID == GID_LeftFileString)
      {
        // Text in left file string gadget was changed
        readStringGadgetsText();
      }
      else if(pGadget->GadgetID == GID_RightFileString)
      {
        // Text in right file string gadget was changed
        readStringGadgetsText();
      }
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      if(!m_bFileRequestOpen)
      {
        Close();
      }
      break;
    }
  }
}

void OpenFilesWindow::initialize()
{
  //
  // Calculate some basic values
  //
  m_WinWidth = (WORD)m_AppScreen.IntuiScreen()->Width / 2;
  m_FontHeight = m_AppScreen.IntuiDrawInfo()->dri_Font->tf_YSize;
  WORD barHeight = m_AppScreen.IntuiScreen()->WBorTop + m_FontHeight + 2;

  WORD hSpace = 10;
  WORD vSpace = 10;
  WORD top = barHeight + vSpace;
  WORD left = hSpace;
  WORD right = m_WinWidth - hSpace;
  WORD buttonWidth = 60;
  WORD buttonHeight = m_FontHeight + 6;
  WORD selectButtonWidth = TextLength(
    &m_AppScreen.IntuiScreen()->RastPort, "...", 3) + 8;
  WORD stringGadgetWidth = right - left - hSpace / 2 - selectButtonWidth;
  WORD selectButtonLeft = right - selectButtonWidth;

  //
  // Setting up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pContext;
  pContext = (struct Gadget*) CreateContext(&m_pGadgetList);
  if(pContext == NULL)
  {
    return;
  }

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  // Line 1  contains  a label
  newGadget.ng_TextAttr   = m_AppScreen.GfxTextAttr();
  newGadget.ng_VisualInfo = m_AppScreen.GadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
    pContext, &newGadget, TAG_END);

  // Line 2 contains a string gadget and selection button for the
  // filename of the left file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_LeftFileString;
  newGadget.ng_Flags      = 0;

  m_pLeftFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_LeftFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectLeftFileButton = CreateGadget(BUTTON_KIND,
    m_pLeftFileStringGadget, &newGadget, TAG_END);

  // Line 3  contains a label
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonHeight + vSpace;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = m_FontHeight;
  newGadget.ng_GadgetText = (UBYTE*) "Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND, m_pSelectLeftFileButton,
    &newGadget, TAG_END);

  // Line 4 contains a string gadget and selection button for the
  // filename of the right file

  // Creating the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += m_FontHeight + 2;
  newGadget.ng_Width      = stringGadgetWidth;
  newGadget.ng_Height     = buttonHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_RightFileString;
  newGadget.ng_Flags      = 0;

  m_pRightFileStringGadget = CreateGadget(STRING_KIND,
    pLabelGadget, &newGadget, TAG_END);

  // Creating the Select button
  newGadget.ng_LeftEdge   = selectButtonLeft;
  newGadget.ng_Width      = selectButtonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "...";
  newGadget.ng_GadgetID   = GID_RightFileButton;
  newGadget.ng_Flags      = 0;

  m_pSelectRightFileButton = CreateGadget(BUTTON_KIND,
    m_pRightFileStringGadget, &newGadget, TAG_END);

  // Line 5 conatins the buttons Diff and Cancel

  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Diff";
  newGadget.ng_GadgetID   = GID_DiffButton;

  m_pDiffButton = CreateGadget(BUTTON_KIND,
    m_pSelectRightFileButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Creating the Cancel button
  newGadget.ng_LeftEdge   = right - buttonWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Cancel";
  newGadget.ng_GadgetID   = GID_CancelButton;

  m_pCancelButton = CreateGadget(BUTTON_KIND,
    m_pDiffButton, &newGadget, GT_Underscore, '_', TAG_END);

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_WinHeight = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  SetTitle("Open the files to diff");

  // Setting the window flags
  setFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  setIDCMP(IDCMP_MENUPICK |       // Inform us about menu selection
           IDCMP_VANILLAKEY |     // Inform us about RAW key press
           IDCMP_RAWKEY |         // Inform us about printable key press
           IDCMP_CLOSEWINDOW |    // Inform us about click on close gadget
           IDCMP_REFRESHWINDOW |  // Inform us when refreshing is necessary
           BUTTONIDCMP);          // Inform us about GadTools button events

  // Setting the first gadget of the gadet list for the window
  setFirstGadget(m_pGadgetList);

  m_bInitialized = true;
}


bool OpenFilesWindow::selectFile(std::string& p_FilePath,
  const std::string& p_RequestTitle)
{
  if(m_bFileRequestOpen)
  {
    // Shouldn't be possible, but anyhow
    return false;
  }

  m_bFileRequestOpen = true;
  disableAll();

  AslFileRequest request(IntuiWindow());
  std::string selectedFile = request.SelectFile(p_RequestTitle, p_FilePath);

  if(selectedFile.length() == 0)
  {
    // Cancelled or nothing selected
    m_bFileRequestOpen = false;
    enableAll();
    return false;
  }

  // Note: This will copy selectedFile to the target of p_FilePath
  //       (the variable p_FilePath points to).
  //       @see  std::string& operator=(const std::string& p_Other);
  //
  //       The reference itself is not rebound to p_FilePath as by
  //       design refereces can't be rebound after initialization.
  //
  //       Because of the copying p_FilePath will 'point to' the valid
  //       copied value of selectedFile after leaving the method scope.
  p_FilePath = selectedFile;

  enableAll();
  m_bFileRequestOpen = false;
  return true;
}

void OpenFilesWindow::enableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, FALSE,
    TAG_END);

  setDiffButtonState();

  // TODO enable menu item "Quit"
}

void OpenFilesWindow::disableAll()
{
  GT_SetGadgetAttrs(m_pLeftFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pRightFileStringGadget, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectLeftFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pSelectRightFileButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pCancelButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(m_pDiffButton, IntuiWindow(), NULL,
    GA_Disabled, TRUE,
    TAG_END);

  // TODO disable menu item "Quit"
}


void OpenFilesWindow::setDiffButtonState()
{
  if(!IsOpen() || m_pDiffButton == NULL)
  {
    return;
  }

  if(m_DiffFacade.LeftFilePath().length() > 0 &&
     m_DiffFacade.RightFilePath().length() > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pDiffButton, m_pWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);

  }
}

void OpenFilesWindow::setStringGadgetText(struct Gadget* p_pGadget,
  const std::string& p_Text)
{
  if(!IsOpen() || p_pGadget == NULL)
  {
    return;
  }

  GT_SetGadgetAttrs(p_pGadget, m_pWindow, NULL,
    GTST_String, p_Text.c_str(),
    TAG_END);
}

void OpenFilesWindow::readStringGadgetsText()
{
  // Set the changed string gadget text as left file path
  UBYTE* pBuf = ((struct StringInfo*)
    m_pLeftFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetLeftFilePath((const char*)pBuf);

  // Set the changed string gadget text as right file path
  pBuf = ((struct StringInfo*)
    m_pRightFileStringGadget->SpecialInfo)->Buffer;

  m_DiffFacade.SetRightFilePath((const char*)pBuf);

  // Enable the 'Diff' button when both string gadgets contain text.
  // If not not: disable it.
  setDiffButtonState();

}


#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

#include "AppMenu.h"


AppMenu::AppMenu(AppScreen& p_Screen)
  : m_Screen(p_Screen),
    m_pMenu(NULL)
{

}

AppMenu::~AppMenu()
{
  Dispose();
}

bool AppMenu::Create(struct NewMenu* p_pMenuDefinition)
{
  if(m_Screen.GadtoolsVisualInfo() == NULL)
  {
    // Without VisualInfo the menu can't be created
    return false;
  }

  // Menu building step 1: Create the menu
  m_pMenu = CreateMenus(p_pMenuDefinition, TAG_END);
  if(m_pMenu == NULL)
  {
    // Menu creation failed
    return false;
  }

  // Menu building step 2: Outlaying the menu
  if(LayoutMenus(m_pMenu, m_Screen.GadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_END) == FALSE)
  {
    // Outlaying the menu failed
    Dispose();
    return false;
  }

  return true;
}

void AppMenu::Dispose()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool AppMenu::AttachToWindow(struct Window* p_pWindow)
{
  if(SetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

bool AppMenu::UpdateInWindow(struct Window* p_pWindow)
{
  if(ResetMenuStrip(p_pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}

void AppMenu::DetachFromWindow(struct Window* p_pWindow)
{
  ClearMenuStrip(p_pWindow);
}

struct Menu* AppMenu::IntuiMenu()
{
  return m_pMenu;
}


void AppMenu::DisableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToDisable)
{
  if(p_pUserDataMenuItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(p_pWindow, menuNumber);
}

void AppMenu::EnableMenuItem(struct Window* p_pWindow,
  APTR p_pUserDataMenuItemToEnable)
{
  if(p_pUserDataMenuItemToEnable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(p_pUserDataMenuItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(p_pWindow, menuNumber);
}

struct MenuItem* AppMenu::findItemByUserData(APTR p_pUserDataToFind, WORD& p_FoundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  p_FoundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == p_pUserDataToFind)
      {
        p_FoundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}

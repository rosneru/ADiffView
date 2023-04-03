#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
  #include <clib/gadtools_protos.h>
  #include <clib/graphics_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
  #include <proto/gadtools.h>
  #include <proto/graphics.h>
  #include <proto/intuition.h>
#endif

#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <libraries/gadtools.h>
#include <workbench/startup.h>

#include <string.h>
#include <stdio.h>
#include <vector>

#include "FilesWindow.h"


FilesWindow::FilesWindow(std::vector<WindowBase*>& windowArray,
                         ScreenBase& screen,
                         struct MsgPort* pIdcmpMsgPort,
                         std::string& leftFilePath,
                         std::string& rightFilePath,
                         CommandBase& cmdDiff,
                         CommandBase& cmdCloseFilesWindow,
                         MenuBase* pMenu)
  : WindowBase(screen, pIdcmpMsgPort, pMenu),
    m_LeftFilePath(leftFilePath),
    m_RightFilePath(rightFilePath),
    m_CmdDiff(cmdDiff),
    m_CmdCloseFilesWindow(cmdCloseFilesWindow),
    m_CmdSelectLeftFile(&windowArray, "Select left (original) file"),
    m_CmdSelectRightFile(&windowArray, "Select right (changed) file"),
    m_MaxPathLength(1024),
    m_pGadtoolsContext(NULL),
    m_pGadStrLeftFile(NULL),
    m_pGadStrRightFile(NULL),
    m_pGadBtnSelectLeft(NULL),
    m_pGadBtnSelectRight(NULL),
    m_pGadBtnDiff(NULL),
    m_pGadBtnSwap(NULL),
    m_pGadBtnClear(NULL),
    m_pGadBtnCancel(NULL)
{
  const char* pErrMsg = "FilesWindow: Failed to create gadgets.";

  //
  // Calculate some basic values
  //
  struct Screen* pIntuiScreen = m_Screen.getIntuiScreen();
  UWORD fontHeight = m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;

  WORD hSpace = 10;
  WORD vSpace = 10;

  //
  // Default button dimensions
  //

  // Extra space by which a button must be wider than its text to look good
  const int btnExtraHSpace = 8;

  // Set the same width for all of the bottom buttons row according to
  // the longest button text
  const char* btnTexts[]  = {"Compare", "Swap", "Clear", "Cancel"};
  size_t numBottomButtons = sizeof(btnTexts) / (sizeof btnTexts[0]);
  WORD buttonsWidth = maxArrayTextLength(btnTexts, numBottomButtons);
  buttonsWidth += btnExtraHSpace;

  // Buttons height
  WORD buttonsHeight = fontHeight + 6;

  // as default the window's width should be half of the screen's width
  m_Width = (WORD)pIntuiScreen->Width / 2;

  // But it must be at least as wide as needed
  ULONG neededWidth = numBottomButtons * buttonsWidth
                    + (numBottomButtons + 1) * hSpace
                    + pIntuiScreen->WBorLeft
                    + pIntuiScreen->WBorRight;

  if(m_Width < neededWidth)
  {
    m_Width = neededWidth;
  }

  if(screen.getIntuiScreen()->Width < 721)
  {
    m_Width = screen.getIntuiScreen()->Width  * 80 / 100;
  }

  WORD barHeight = pIntuiScreen->WBorTop + fontHeight + 2;

  WORD top = barHeight + vSpace;
  WORD left = pIntuiScreen->WBorLeft + hSpace;
  WORD right = m_Width - pIntuiScreen->WBorRight - hSpace;

  WORD btnSelectWidth = TextLength(&pIntuiScreen->RastPort, "...", 3)
                      + btnExtraHSpace;

  WORD btnSelectLeft = right - btnSelectWidth;

  WORD stringGadWidth = right - left - hSpace / 2 - btnSelectWidth;

  //
  // Set up the gadgets
  //

  // Create a place for GadTools context data
  struct Gadget* pFakeGad;
  pFakeGad = (struct Gadget*) CreateContext(&m_pGadtoolsContext);
  if(pFakeGad == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Set the first gadget of the gadget list for the window
  setFirstGadget(m_pGadtoolsContext);

  // Declare the basic gadget structure
  struct NewGadget newGadget;

  //
  // Row 1:  contains  a label
  //
  newGadget.ng_TextAttr   = m_Screen.getIntuiTextAttr();
  newGadget.ng_VisualInfo = m_Screen.getGadtoolsVisualInfo();
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = stringGadWidth;
  newGadget.ng_Height     = fontHeight;
  newGadget.ng_GadgetText = "_Left file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  struct Gadget* pLabelGadget = CreateGadget(TEXT_KIND,
                                             pFakeGad,
                                             &newGadget,
                                             GT_Underscore, '_',
                                             TAG_DONE);
  if(pLabelGadget == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  //
  // Row 2: contains a string gadget and selection button for the
  // filename of the left file
  //

  // Create the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += fontHeight + 2;
  newGadget.ng_Width      = stringGadWidth;
  newGadget.ng_Height     = buttonsHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_StrLeftFile;
  newGadget.ng_Flags      = 0;

  m_pGadStrLeftFile = CreateGadget(STRING_KIND,
                                   pLabelGadget,
                                   &newGadget,
                                   GTST_MaxChars, m_MaxPathLength,
                                   TAG_DONE);
  if(m_pGadStrLeftFile == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the Select button
  newGadget.ng_LeftEdge   = btnSelectLeft;
  newGadget.ng_Width      = btnSelectWidth;
  newGadget.ng_GadgetText = "...";
  newGadget.ng_GadgetID   = GID_BtnLeftFile;
  newGadget.ng_Flags      = 0;

  m_pGadBtnSelectLeft = CreateGadget(BUTTON_KIND,
                                     m_pGadStrLeftFile,
                                     &newGadget,
                                     TAG_DONE);
  if(m_pGadBtnSelectLeft == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  //
  // Row 3:  contains a label
  //
  newGadget.ng_LeftEdge   = left + 2;
  newGadget.ng_TopEdge    += buttonsHeight + vSpace;
  newGadget.ng_Width      = stringGadWidth;
  newGadget.ng_Height     = fontHeight;
  newGadget.ng_GadgetText = "_Right file";
  newGadget.ng_Flags = PLACETEXT_RIGHT | PLACETEXT_LEFT | NG_HIGHLABEL;

  pLabelGadget = CreateGadget(TEXT_KIND,
                              m_pGadBtnSelectLeft,
                              &newGadget,
                              GT_Underscore, '_',
                              TAG_DONE);
  if(pLabelGadget == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  //
  // Row 4: contains a string gadget and selection button for the
  // filename of the right file
  //

  // Create the string gadget
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += fontHeight + 2;
  newGadget.ng_Width      = stringGadWidth;
  newGadget.ng_Height     = buttonsHeight;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_GadgetID   = GID_StrRightFile;
  newGadget.ng_Flags      = 0;

  m_pGadStrRightFile = CreateGadget(STRING_KIND,
                                    pLabelGadget,
                                    &newGadget,
                                    GTST_MaxChars, m_MaxPathLength,
                                    TAG_DONE);
  if(m_pGadStrRightFile == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the Select button
  newGadget.ng_LeftEdge   = btnSelectLeft;
  newGadget.ng_Width      = btnSelectWidth;
  newGadget.ng_GadgetText = "...";
  newGadget.ng_GadgetID   = GID_BtnRightFile;
  newGadget.ng_Flags      = 0;

  m_pGadBtnSelectRight = CreateGadget(BUTTON_KIND,
                                      m_pGadStrRightFile,
                                      &newGadget,
                                      TAG_DONE);

  //
  // Row 5: contains the buttons Compare, Swap, Clear and Cancel
  //

  // Calculate the 'field width' of each of the four bottom buttons
  // 1) Calculate the free / uncovered area between all 4 buttons
  ULONG bottomButtonDistances = m_Width - pIntuiScreen->WBorLeft
                                      - pIntuiScreen->WBorRight
                                      - 2 * hSpace
                                      - 4 * buttonsWidth;

  // 2) Between 4 buttons there are 3 free areas
  bottomButtonDistances /= 3;


  // Creating the Diff button
  newGadget.ng_LeftEdge   = left;
  newGadget.ng_TopEdge    += buttonsHeight + vSpace + vSpace;
  newGadget.ng_Width      = buttonsWidth;
  newGadget.ng_GadgetText = "Compare";
  newGadget.ng_GadgetID   = GID_BtnDiff;

  m_pGadBtnDiff = CreateGadget(BUTTON_KIND,
                               m_pGadBtnSelectRight,
                               &newGadget,
                               GT_Underscore, '_',
                               TAG_DONE);
  if(m_pGadBtnDiff == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Creating the Swap button
  newGadget.ng_LeftEdge   += buttonsWidth + bottomButtonDistances;
  newGadget.ng_GadgetText = "_Swap";
  newGadget.ng_GadgetID   = GID_BtnSwap;

  m_pGadBtnSwap = CreateGadget(BUTTON_KIND,
                               m_pGadBtnDiff,
                               &newGadget,
                               GT_Underscore, '_',
                               TAG_DONE);
  if(m_pGadBtnSwap == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the Clear button
  newGadget.ng_LeftEdge   += buttonsWidth + bottomButtonDistances;
  newGadget.ng_GadgetText = "Cl_ear";
  newGadget.ng_GadgetID   = GID_BtnClear;

  m_pGadBtnClear = CreateGadget(BUTTON_KIND,
                                m_pGadBtnSwap,
                                &newGadget,
                                GT_Underscore, '_',
                                TAG_DONE);
  if(m_pGadBtnClear == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Creating the Cancel button
  newGadget.ng_LeftEdge   += buttonsWidth + bottomButtonDistances;
  newGadget.ng_GadgetText = "Cancel";
  newGadget.ng_GadgetID   = GID_BtnCancel;

  m_pGadBtnCancel = CreateGadget(BUTTON_KIND,
                                 m_pGadBtnClear,
                                 &newGadget,
                                 GT_Underscore, '_',
                                 TAG_DONE);
  if(m_pGadBtnCancel == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_Height = newGadget.ng_TopEdge + newGadget.ng_Height + vSpace;

  // Setting window title
  setTitle("Open the files to compare");

  // Setting the window flags
  addFlags(WFLG_CLOSEGADGET |     // Add a close gadget
           WFLG_DRAGBAR |         // Add a drag gadget
           WFLG_DEPTHGADGET);     // Add a depth gadget

  // Setting the IDCMP messages we want to receive for this window
  addIDCMP(IDCMP_MENUPICK |       // Get msg when menu selected
           IDCMP_VANILLAKEY |     // Get msg when RAW key pressed
           IDCMP_CLOSEWINDOW |    // Get msg when close gadget clicked
           IDCMP_REFRESHWINDOW |  // Get msg when must refresh; needed by GadTools
           IDCMP_GADGETUP);       // Get msg when gadgets state changed
}


FilesWindow::~FilesWindow()
{
  cleanup();
}


void FilesWindow::Refresh()
{
// TODO
//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool FilesWindow::open(InitialPosition initialPos)
{
  if(!WindowBase::open(initialPos))
  {
    return false;
  }

  // Perform the initial refresh of all the GadTools. Mandatory.
  GT_RefreshWindow(m_pWindow, NULL);

  setStringGadgetText(m_pGadStrLeftFile, m_LeftFilePath.c_str());
  setStringGadgetText(m_pGadStrRightFile, m_RightFilePath.c_str());

  // Enable or disable the 'Diff' and 'Swap' buttons depending on some
  // conditions
  checkEnableButtons();

  return true;
}


void FilesWindow::handleIDCMP(const struct IntuiMessage* pMsg)
{
  if(!isOpen())
  {
    return;
  }

  switch (pMsg->Class)
  {
    case IDCMP_GADGETUP:
    {
      struct Gadget* pGadget = (struct Gadget*) pMsg->IAddress;
      handleGadgetEvent(pGadget);
      break;
    }

    case IDCMP_REFRESHWINDOW:
    {
      // This handling is REQUIRED with GadTools
      GT_BeginRefresh(m_pWindow);
      GT_EndRefresh(m_pWindow, TRUE);
      break;
    }

    case IDCMP_CLOSEWINDOW:
    {
      m_CmdCloseFilesWindow.Execute(NULL);
      break;
    }

    case IDCMP_VANILLAKEY:
    {
      handleVanillaKey(pMsg->Code);
      break;
    }
  }
}


void FilesWindow::HandleAppMessage(struct AppMessage* pAppMsg)
{
  if(!isOpen())
  {
    return;
  }

  // Alloc temporary buf memory
  int bufLen = 2048;  // TODO How to get rid of this fixed maximum?
  STRPTR pBuf = (STRPTR) AllocVec(bufLen, MEMF_ANY);
  if(pBuf == NULL)
  {
    return;
  }

  struct WBArg* pWbArg = pAppMsg->am_ArgList;
  struct Gadget* pStrGadget = NULL;

  int i = 0;
  while((pStrGadget = getFirstEmptyStringGadget()) != NULL)
  {
    if(i > 1)
    {
      // Max 2 args can be received, as there are only 2 string gadgets
      break;
    }

    if(i >= pAppMsg->am_NumArgs)
    {
      // No more WbArgs
      break;
    }

    if(NameFromLock(pWbArg[i].wa_Lock, pBuf, bufLen) != 0)
    {
      if(AddPart(pBuf,(STRPTR) pWbArg[i].wa_Name, bufLen))
      {
        setStringGadgetText(pStrGadget, pBuf);
      }
    }

    i++;
  }

  FreeVec(pBuf);
  checkEnableButtons();
}


void FilesWindow::handleGadgetEvent(struct Gadget* pGadget)
{
  if(pGadget == NULL)
  {
    return;
  }

  switch(pGadget->GadgetID)
  {
    case GID_StrLeftFile:
    case GID_StrRightFile:
      checkEnableButtons();
      break;

    case GID_BtnLeftFile:  // Select left file
      selectLeftFile();
      break;

    case GID_BtnRightFile: // Select right file
      selectRightFile();
      break;

    case GID_BtnSwap:      // Swap left and right file
      swapFiles();
      break;

    case GID_BtnClear:     // Clear the string gadgets
      clear();
      break;

    case GID_BtnDiff:      // Compare the files and display the diff
      compare();
      break;

    case GID_BtnCancel:
      m_CmdCloseFilesWindow.Execute(NULL);
      break;
  }
}


void FilesWindow::handleVanillaKey(UWORD code)
{
  switch(code)
  {
    case 'l': // Select left file
    case 'L':
      selectLeftFile();
      break;

    case 'r': // Select right file
    case 'R':
      selectRightFile();
      break;

    case 's': // Swap left and right file
    case 'S':
    {
      // Allow only if Swap button is enabled
      long disabled;
      long numProcessed;
      numProcessed  = GT_GetGadgetAttrs(m_pGadBtnSwap, m_pWindow, NULL,
                                        GA_Disabled, (ULONG)&disabled,
                                        TAG_DONE);

      if((numProcessed != 1) || (disabled == 1))
      {
        return;
      }

      // Button is enabled, perform its action
      swapFiles();
      break;
    }

    case 'e': // Clear the string gadgets
    case 'E':
    {
      // Allow only if Clear button is enabled
      long disabled;
      long numProcessed;
      numProcessed  = GT_GetGadgetAttrs(m_pGadBtnClear, m_pWindow, NULL,
                                        GA_Disabled, (ULONG)&disabled,
                                        TAG_DONE);

      if((numProcessed != 1) || (disabled == 1))
      {
        return;
      }

      // Button is enabled, perform its action
      clear();
      break;
    }

    case 0xD:// <RETURN> Compare the files and display the diff
    {
      // Allow only if Diff button is enabled
      long disabled;
      long numProcessed;
      numProcessed  = GT_GetGadgetAttrs(m_pGadBtnDiff, m_pWindow, NULL,
                                        GA_Disabled, (ULONG)&disabled,
                                        TAG_DONE);

      if((numProcessed != 1) || (disabled == 1))
      {
        return;
      }

      // Button is enabled, perform its action
      compare();
      break;
    }

    case 0x1B: // <ESC> Cancel
      m_CmdCloseFilesWindow.Execute(NULL);
      break;

  }
}


void FilesWindow::selectLeftFile()
{
  // Read latest string gadgets contents before continue
  STRPTR pLeftStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  STRPTR pRightStrGadgetText = getStringGadgetText(m_pGadStrRightFile);

  if((pLeftStrGadgetText == NULL) || (pRightStrGadgetText == NULL))
  {
    checkEnableButtons();
    return;
  }

  if(strlen(pLeftStrGadgetText) == 0)
  {
    // Left file path is empty, so use the path of the right file for
    // pre-selection (regardless if that also is empty)
    m_CmdSelectLeftFile.SetInitialFilePath(pRightStrGadgetText);

    // Do not use the file name 'though
    m_CmdSelectLeftFile.SetPreselectPathOnly(true);
  }
  else
  {
    m_CmdSelectLeftFile.SetInitialFilePath(pLeftStrGadgetText);
    m_CmdSelectLeftFile.SetPreselectPathOnly(false);
  }

  m_CmdSelectLeftFile.Execute(m_pWindow);

  if(m_CmdSelectLeftFile.SelectedFile().length() == 0)
  {
    checkEnableButtons();
    return;
  }

  setStringGadgetText(m_pGadStrLeftFile,
                      m_CmdSelectLeftFile.SelectedFile().c_str());

  checkEnableButtons();
}


void FilesWindow::selectRightFile()
{

  // Read latest string gadgets contents before continue
  STRPTR pLeftStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  STRPTR pRightStrGadgetText = getStringGadgetText(m_pGadStrRightFile);

  if((pLeftStrGadgetText == NULL) || (pRightStrGadgetText == NULL))
  {
    checkEnableButtons();
    return;
  }

  if(strlen(pRightStrGadgetText) == 0)
  {
    // Right file path is empty, so use the path of the left file for
    // pre-selection (regardless if that also is empty)
    m_CmdSelectRightFile.SetInitialFilePath(pLeftStrGadgetText);

    // Do not use the file name 'though
    m_CmdSelectRightFile.SetPreselectPathOnly(true);
  }
  else
  {
    m_CmdSelectRightFile.SetInitialFilePath(pRightStrGadgetText);
    m_CmdSelectRightFile.SetPreselectPathOnly(false);
  }

  m_CmdSelectRightFile.Execute(m_pWindow);

  if(m_CmdSelectRightFile.SelectedFile().length() == 0)
  {
    checkEnableButtons();
    return;
  }

  setStringGadgetText(m_pGadStrRightFile,
                      m_CmdSelectRightFile.SelectedFile().c_str());

  checkEnableButtons();
}


void FilesWindow::swapFiles()
{
  STRPTR pLeftStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  STRPTR pRightStrGadgetText = getStringGadgetText(m_pGadStrRightFile);

  if((pLeftStrGadgetText == NULL) || (pRightStrGadgetText == NULL))
  {
    return;
  }

  std::string formerLeftFilePath = pLeftStrGadgetText;

  setStringGadgetText(m_pGadStrLeftFile, pRightStrGadgetText);
  setStringGadgetText(m_pGadStrRightFile, formerLeftFilePath.c_str());
}


void FilesWindow::compare()
{
  if(!isOpen() || (m_pGadBtnDiff == NULL) || (m_pGadBtnSwap == NULL))
  {
    return;
  }

  STRPTR pLeftStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  STRPTR pRightStrGadgetText = getStringGadgetText(m_pGadStrRightFile);

  if((pLeftStrGadgetText == NULL) || (pRightStrGadgetText == NULL))
  {
    return;
  }

  m_LeftFilePath = pLeftStrGadgetText;
  m_RightFilePath = pRightStrGadgetText;

  // Perform the diff
  m_CmdDiff.Execute(NULL);
}


void FilesWindow::clear()
{
  setStringGadgetText(m_pGadStrLeftFile, "");
  setStringGadgetText(m_pGadStrRightFile, "");
  checkEnableButtons();
}


void FilesWindow::cleanup()
{
  if(m_pGadtoolsContext != NULL)
  {
    FreeGadgets(m_pGadtoolsContext);
  }

  m_pGadtoolsContext = NULL;
  m_pGadStrLeftFile = NULL;
  m_pGadStrRightFile = NULL;
  m_pGadBtnSelectLeft = NULL;
  m_pGadBtnSelectRight = NULL;
  m_pGadBtnDiff = NULL;
  m_pGadBtnSwap = NULL;
  m_pGadBtnCancel = NULL;
}

void FilesWindow::checkEnableButtons()
{
  if(!isOpen() || (m_pGadBtnDiff == NULL) || (m_pGadBtnSwap == NULL))
  {
    return;
  }

  STRPTR pLeftStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  STRPTR pRightStrGadgetText = getStringGadgetText(m_pGadStrRightFile);

  if((pLeftStrGadgetText == NULL) || (pRightStrGadgetText == NULL))
  {
    return;
  }

  if((strlen(pLeftStrGadgetText)) > 0
   &&(strlen(pRightStrGadgetText)) > 0)
  {
    // Enable "Diff" button
    GT_SetGadgetAttrs(m_pGadBtnDiff, m_pWindow, NULL,
                      GA_Disabled, FALSE,
                      TAG_DONE);
  }
  else
  {
    // Disable "Diff" button
    GT_SetGadgetAttrs(m_pGadBtnDiff, m_pWindow, NULL,
                      GA_Disabled, TRUE,
                      TAG_DONE);
  }

  if((strlen(pLeftStrGadgetText)) > 0
   ||(strlen(pRightStrGadgetText)) > 0)
  {
    // Enable "Swap" button
    GT_SetGadgetAttrs(m_pGadBtnSwap, m_pWindow, NULL,
                      GA_Disabled, FALSE,
                      TAG_DONE);

    // Enable "Clear" button
    GT_SetGadgetAttrs(m_pGadBtnClear, m_pWindow, NULL,
                      GA_Disabled, FALSE,
                      TAG_DONE);
  }
  else
  {
    // Disable "Swap" button
    GT_SetGadgetAttrs(m_pGadBtnSwap, m_pWindow, NULL,
                      GA_Disabled, TRUE,
                      TAG_DONE);

    // Disable "Clear" button
    GT_SetGadgetAttrs(m_pGadBtnClear, m_pWindow, NULL,
                      GA_Disabled, TRUE,
                      TAG_DONE);
  }
}


struct Gadget* FilesWindow::getFirstEmptyStringGadget()
{
  STRPTR pStrGadgetText = getStringGadgetText(m_pGadStrLeftFile);
  if((pStrGadgetText == NULL) || (strlen(pStrGadgetText) == 0))
  {
    return m_pGadStrLeftFile;
  }

  pStrGadgetText = getStringGadgetText(m_pGadStrRightFile);
  if((pStrGadgetText == NULL) || (strlen(pStrGadgetText) == 0))
  {
    return m_pGadStrRightFile;
  }

  return NULL;
}

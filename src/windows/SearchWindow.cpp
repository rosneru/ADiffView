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

#include "SearchWindow.h"


/**
 * Array of string constants for 'Location' cycle gadget
 */
const char* g_GadCycLocationLabels [5] = {"Both files", "Left file", "Right file", "Differences", NULL};


SearchWindow::SearchWindow(std::vector<WindowBase*>& windowArray,
                         ScreenBase& screen,
                         struct MsgPort* pIdcmpMsgPort,
                         TextFinder& TextFinder,
                         CommandBase& cmdCloseSearchWindow)
  : WindowBase(screen, pIdcmpMsgPort, NULL),
    m_TextFinder(TextFinder),
    m_CmdCloseSearchWindow(cmdCloseSearchWindow),
    m_NumLocationLabels(sizeof(g_GadCycLocationLabels) /
                        sizeof(g_GadCycLocationLabels[0])
                        - 1), // Skip trailing NULL item
    m_pGadtoolsContext(NULL),
    m_pGadStrSearchText(NULL),
    m_pGadCycLocation(NULL),
    m_pGadCbxIgnoreCase(NULL),
    m_pGadBtnFind(NULL),
    m_pGadBtnFromStart(NULL),
    m_pGadBtnBackwards(NULL)
{     
  const char* pErrMsg = "SearchWindow: Failed to create gadgets.";

  //
  // Calculate some basic values
  //
  struct Screen* pIntuiScreen = m_Screen.getIntuiScreen();
  UWORD fontHeight = m_Screen.getIntuiDrawInfo()->dri_Font->tf_YSize;

  ULONG doScaleCheckboxes = FALSE;
  if(m_Screen.arePixelsSquare())
  {
    doScaleCheckboxes = TRUE;
  }

  WORD hSpace = 10;
  WORD vSpace = 6;

  //
  // Default button dimensions
  //

  // Extra space by which a button must be wider than its text to look good
  const int btnExtraHSpace = 8;

  // Set the same width for all of the bottom buttons row according to
  // the longest button text
  const char* btnTexts[]  = {"Find", "From start", "Backwards"};
  size_t numBottomButtons = sizeof(btnTexts) / (sizeof btnTexts[0]);
  WORD buttonsWidth = maxArrayTextLength(btnTexts, numBottomButtons);
  buttonsWidth += btnExtraHSpace;

  // Buttons height
  WORD buttonsHeight = fontHeight + 6;

  // as default the window's width should be half of the screen's width
  m_Width = (WORD)pIntuiScreen->Width / 2;
  if(m_Width < 400)
  {
    //  but at least 400
    m_Width = 400;
  }

  WORD barHeight = pIntuiScreen->WBorTop + fontHeight + 2;

  WORD top = barHeight + vSpace;
  WORD left = pIntuiScreen->WBorLeft + hSpace;
  WORD right = m_Width - pIntuiScreen->WBorRight - hSpace - hSpace;

  // Set the labelWidth to the longest label text
  const char* labelTexts[]  = 
  { // TODO change
    "Search for", "Location", "Ignore case", "Start from"
  };

  size_t arraySize = sizeof(labelTexts) / (sizeof labelTexts[0]);
  WORD labelsWidth = maxArrayTextLength(labelTexts, arraySize);

  WORD contentWidth = right - left - labelsWidth;

  // Both cycle gadgets should be the same width. Calculate this width 
  // by measuring the longest text width

  arraySize = sizeof(g_GadCycLocationLabels) / sizeof(g_GadCycLocationLabels[0]);
  WORD cycWidth = maxArrayTextLength(g_GadCycLocationLabels, arraySize);
  cycWidth += 3 * hSpace;

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

  // Create the string gadget for the search text
  newGadget.ng_TextAttr   = m_Screen.getIntuiTextAttr();
  newGadget.ng_VisualInfo = m_Screen.getGadtoolsVisualInfo();
  newGadget.ng_Flags      = NG_HIGHLABEL;
  newGadget.ng_LeftEdge   = labelsWidth + left + hSpace;
  newGadget.ng_TopEdge    = top;
  newGadget.ng_Width      = contentWidth;
  newGadget.ng_Height     = fontHeight + 5;
  newGadget.ng_GadgetText = (UBYTE*) "Search _for";
  newGadget.ng_GadgetID   = GID_StrSearchText;

  m_pGadStrSearchText = CreateGadget(STRING_KIND,
                                     pFakeGad,
                                     &newGadget,
                                     GT_Underscore, '_',
                                     TAG_DONE);
  if(m_pGadStrSearchText == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  newGadget.ng_TopEdge    += buttonsHeight + vSpace;
  newGadget.ng_Width      = cycWidth;
  newGadget.ng_GadgetText = (UBYTE*) "_Location";
  newGadget.ng_GadgetID   = GID_CycLocation;

  m_pGadCycLocation = CreateGadget(CYCLE_KIND,
                                   m_pGadStrSearchText,
                                   &newGadget,
                                   GT_Underscore, '_',
                                   GTCY_Labels, (ULONG)g_GadCycLocationLabels,
                                   TAG_DONE);

  if(m_pGadCycLocation == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the 'Ignore case' checkbox
  newGadget.ng_LeftEdge   += (cycWidth + hSpace + hSpace + labelsWidth);
  newGadget.ng_TopEdge    ++; // Manually center text vertically as
                              // GadTools doesn't do it

  newGadget.ng_Width = newGadget.ng_Height + 3;
  newGadget.ng_GadgetText = (UBYTE*) "Ignore _case";
  newGadget.ng_GadgetID   = GID_CbxIgnoreCase;

  m_pGadCbxIgnoreCase = CreateGadget(CHECKBOX_KIND,
                                     m_pGadCycLocation,
                                     &newGadget,
                                     GT_Underscore, '_',
                                     GTCB_Scaled, doScaleCheckboxes,
                                     GTCB_Checked, FALSE,
                                     TAG_DONE);

  if(m_pGadCbxIgnoreCase == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Create the 'Find' button
  newGadget.ng_TopEdge    += buttonsHeight + vSpace + vSpace;
  newGadget.ng_LeftEdge   = left + hSpace;
  newGadget.ng_Width      = buttonsWidth;
  newGadget.ng_GadgetText = (UBYTE*) "Find";
  newGadget.ng_GadgetID   = GID_BtnFind;

  m_pGadBtnFind = CreateGadget(BUTTON_KIND,
                               m_pGadCbxIgnoreCase,
                               &newGadget,
                               TAG_DONE);
  if(m_pGadBtnFind == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Calculate the distance between the left edges of the buttons in
  // this line
  LONG btnsLeftEdgeDist = buttonsWidth;
  if((newGadget.ng_LeftEdge + btnsLeftEdgeDist) < (labelsWidth + left + hSpace))
  {
    // Hope, this h-distance is enough. But it is visually a good idea
    // to start the button on a vertical line with the gadgets above.
    btnsLeftEdgeDist = labelsWidth + left + hSpace - newGadget.ng_LeftEdge;
  }
  else
  {
    // Add a h-distance to previous button
    // newGadget.ng_LeftEdge += 10;
    btnsLeftEdgeDist += 10;
  }
  
  // Create the 'From start' button
  newGadget.ng_LeftEdge += btnsLeftEdgeDist;

  newGadget.ng_GadgetText = (UBYTE*) "From _start";
  newGadget.ng_GadgetID   = GID_BtnFromStart;

  m_pGadBtnFromStart = CreateGadget(BUTTON_KIND,
                                   m_pGadBtnFind,
                                   &newGadget,
                                   GT_Underscore, '_',
                                   TAG_DONE);

  if(m_pGadBtnFromStart == NULL)
  {
    cleanup();
    throw pErrMsg;
  }


  // Create the 'Backwards' button
  newGadget.ng_LeftEdge  += btnsLeftEdgeDist;
  newGadget.ng_GadgetText = (UBYTE*) "_Backwards";
  newGadget.ng_GadgetID   = GID_BtnBackwards;

  m_pGadBtnBackwards = CreateGadget(BUTTON_KIND,
                                    m_pGadBtnFromStart,
                                    &newGadget,
                                    GT_Underscore, '_',
                                    TAG_DONE);

  if(m_pGadBtnBackwards == NULL)
  {
    cleanup();
    throw pErrMsg;
  }

  // Adjust the window height depending on the y-Pos and height of the
  // last gadget
  m_Height = newGadget.ng_TopEdge 
          + newGadget.ng_Height 
          + (3 * pIntuiScreen->WBorBottom);

  // Setting window title
  setTitle("Find");

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


SearchWindow::~SearchWindow()
{
  cleanup();
}


void SearchWindow::Refresh()
{
// TODO
//  BeginRefresh(m_pWindow);
//  EndRefresh(m_pWindow, TRUE);
}

bool SearchWindow::open(InitialPosition initialPos)
{
  //
  // Open the window
  //
  if(!WindowBase::open(initialPos))
  {
    return false;
  }

  // Perform the initial refresh of all the GadTools. Mandatory.
  GT_RefreshWindow(m_pWindow, NULL);

  /**
   * Apply the options from search command
   *
   * NOTE: This can and should be done before opening the window to
   * avoid possible visible changing of the cycle boxes when the window
   * already is open
   *
   * NOTE: No check of gadget pointers needed. If there would be
   * something wrong the constructor had already thrown an exception and
   * this wouldn't be executed.
   */

  setStringGadgetText(m_pGadStrSearchText, m_TextFinder.getSearchText());

  if(m_TextFinder.isCaseIgnored())
  {
    GT_SetGadgetAttrs(m_pGadCbxIgnoreCase, m_pWindow, NULL,
                      GTCB_Checked, TRUE,
                      TAG_DONE);
  }
  else
  {
    GT_SetGadgetAttrs(m_pGadCbxIgnoreCase, m_pWindow, NULL,
                      GTCB_Checked, FALSE,
                      TAG_DONE);
  }

  switch(m_TextFinder.getLocation())
  {
    case SL_BothFiles:
    {
      GT_SetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                        GTCY_Active, 0, // TODO Change manual value; use map?
                        TAG_DONE);
      break;
    }

    case SL_LeftFile:
    {
      GT_SetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                        GTCY_Active, 1, // TODO Change manual value; use map?
                        TAG_DONE);
      break;
    }

    case SL_RightFile:
    {
      GT_SetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                        GTCY_Active, 2, // TODO Change manual value; use map?
                        TAG_DONE);
      break;
    }

    case SL_Differences:
    {
      GT_SetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                        GTCY_Active, 3, // TODO Change manual value; use map?
                        TAG_DONE);
      break;
    }
  }




  // Also when opened for the first time, the search text gadget should
  // be activated
  if(strlen(m_TextFinder.getSearchText()) < 1)
  {
    setFindButtonsEnabled(false);
    ActivateGadget(m_pGadStrSearchText, m_pWindow, NULL);
  }
  else
  {
    setFindButtonsEnabled(true);
  }

  return true;
}


void SearchWindow::handleIDCMP(const struct IntuiMessage* pMsg)
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
      m_CmdCloseSearchWindow.Execute(NULL);
      break;
    }

    case IDCMP_VANILLAKEY:
    {
      handleVanillaKey(pMsg->Code);
      break;
    }
  }
}

void SearchWindow::find()
{
  STRPTR pTextToFind = applyChangedSearchText();
  if(pTextToFind == NULL)
  {
    return;
  }

  // Set the user-typed text to find in search command: This performs
  // the search if pTextToFind differs from the text which may already
  // have been set in TextFinder.
  m_TextFinder.setSearchText(pTextToFind);

  // Jump to the next result and close the search window. 
  // (Or just close the search window if no next result exists but the 
  //  current search result still is displayed)
  if((m_TextFinder.displayNextResult(false, true) == true) 
  || (m_TextFinder.isFormerResultDisplayed()))
  {
    m_CmdCloseSearchWindow.Execute(NULL);
  }
  else
  {
    // Again 'display' the next result but this time only to get the
    // 'Not found' error message shown in window title.
    m_TextFinder.displayNextResult();
  }
}

void SearchWindow::findFromStart()
{
  STRPTR pTextToFind = applyChangedSearchText();
  if(pTextToFind == NULL)
  {
    return;
  }

  // Set the user-typed text to find in search command: This performs
  // the search if pTextToFind differs from the text which (maybe) is
  // already set in  TextFinder.
  m_TextFinder.setSearchText(pTextToFind);

  // Jump to the result
  if(m_TextFinder.displayFirstResult() == true)
  {
    m_CmdCloseSearchWindow.Execute(NULL);
  }
}

void SearchWindow::findBackwards()
{
  STRPTR pTextToFind = applyChangedSearchText();
  if(pTextToFind == NULL)
  {
    return;
  }

  // Set the user-typed text to find in search command: This performs
  // the search if pTextToFind differs from the text which (maybe) is
  // already set in  TextFinder.
  m_TextFinder.setSearchText(pTextToFind);

  // Jump to the result
  if(m_TextFinder.displayLastResult() == true)
  {
    m_CmdCloseSearchWindow.Execute(NULL);
  }
}

void SearchWindow::handleGadgetEvent(struct Gadget* pGadget)
{
  if(pGadget == NULL)
  {
    return;
  }

  switch(pGadget->GadgetID)
  {
    case GID_StrSearchText:
    {
      STRPTR pTextToFind = applyChangedSearchText();
      if(pTextToFind == NULL)
      {
        return;
      }

      // Set the user-typed text to find in search command
      m_TextFinder.setSearchText(pTextToFind);
      break;
    }

    case GID_CycLocation:
    {
      applyChangedLocation();
      break;
    }

    case GID_CbxIgnoreCase:
    {
      applyChangedCase();
      break;
    }

    case GID_BtnFind:
    {
      find();
      break;
    }

    case GID_BtnFromStart:
    {
      findFromStart();
      break;
    }

    case GID_BtnBackwards:
    {
      findBackwards();
      break;
    } 
  }
}


void SearchWindow::handleVanillaKey(UWORD code)
{
  switch(code)
  {
    case 'b':
    case 'B':
    {
      findBackwards();
      break;
    }

    case 'c':
    case 'C':
    {
      toggleCaseGadget();
      applyChangedCase();
      break;
    }

    case 'f':
    case 'F':
    {
      ActivateGadget(m_pGadStrSearchText, m_pWindow, NULL);
      break;
    }

    case 'l':
    case 'L':
    {
      toggleLocationGadget();
      applyChangedLocation();
      break;
    }

    case 's':
    case 'S':
    {
      findFromStart();
      break;
    }

    case 0xD:// <RETURN> Compare the files and display the diff
    {
      find();
      break;
    }

    case 0x1B: // <ESC> Cancel
    {
      m_CmdCloseSearchWindow.Execute(NULL);
      break;
    }

  }
}


void SearchWindow::toggleLocationGadget()
{
  // Get the current index value from 'location' cycle gadget
  ULONG currentId = 0;
  if(GT_GetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                      GTCY_Active, (ULONG)&currentId,
                      TAG_DONE) != 1)
  {
    return;
  }

  // Increase the id; start from 0 when over max
  currentId++;
  if(currentId >= m_NumLocationLabels)
  {
    currentId = 0;
  }

  // Set the new index value in gadget
  GT_SetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                    GTCY_Active, currentId,
                    TAG_DONE);
}


void SearchWindow::toggleCaseGadget()
{
  ULONG isChecked = 0;
  if(GT_GetGadgetAttrs(m_pGadCbxIgnoreCase, m_pWindow, NULL,
                       GTCB_Checked, (ULONG)&isChecked,
                       TAG_DONE) != 1)
  {
    return;
  }

  if(isChecked == 0)
  {
    isChecked = 1;
  }
  else
  {
    isChecked = 0;
  }

  // Set the new checked value in gadget
  GT_SetGadgetAttrs(m_pGadCbxIgnoreCase, m_pWindow, NULL,
                    GTCB_Checked, isChecked,
                    TAG_DONE);
}

void SearchWindow::setFindButtonsEnabled(bool enabled)
{
  BOOL disabled;
  if(enabled == true)
  {
    disabled = FALSE;
  }
  else
  {
    disabled = TRUE;
  }

  GT_SetGadgetAttrs(m_pGadBtnFind, m_pWindow, NULL,
                    GA_Disabled, disabled,
                    TAG_DONE);

  GT_SetGadgetAttrs(m_pGadBtnFromStart, m_pWindow, NULL,
                    GA_Disabled, disabled,
                    TAG_DONE);

  GT_SetGadgetAttrs(m_pGadBtnBackwards, m_pWindow, NULL,
                    GA_Disabled, disabled,
                    TAG_DONE);

}

void SearchWindow::applyChangedLocation()
{
  // Get the current index value from 'location' cycle gadget
  ULONG currentId = 0;
  if(GT_GetGadgetAttrs(m_pGadCycLocation, m_pWindow, NULL,
                      GTCY_Active, (ULONG)&currentId,
                      TAG_DONE) != 1)
  {
    return;
  }

  m_TextFinder.setLocation((SearchLocation)currentId);
  applyChangedSearchText(); // because this enables/disables the buttons
}


void SearchWindow::applyChangedCase()
{
  // Get the current state of 'is case ignored' checkbox gadget
  ULONG isChecked = FALSE;
  if(GT_GetGadgetAttrs(m_pGadCbxIgnoreCase, m_pWindow, NULL,
                      GTCB_Checked, (ULONG)&isChecked,
                      TAG_DONE) != 1)
  {
    return;
  }

  m_TextFinder.setCaseIgnored(isChecked == TRUE);
  applyChangedSearchText(); // because this enables/disables the buttons
}


STRPTR SearchWindow::applyChangedSearchText()
{
  STRPTR pTextToFind = getStringGadgetText(m_pGadStrSearchText);
  if((pTextToFind == NULL) || (strlen(pTextToFind) < 1))
  {
    setFindButtonsEnabled(false);
    return NULL;
  }

  setFindButtonsEnabled(true);

  return pTextToFind;
}

void SearchWindow::cleanup()
{
  if(m_pGadtoolsContext != NULL)
  {
    FreeGadgets(m_pGadtoolsContext);
  }

  m_pGadtoolsContext = NULL;
  m_pGadStrSearchText = NULL;
  m_pGadCycLocation = NULL;
  m_pGadCbxIgnoreCase = NULL;
  m_pGadBtnFind = NULL;
}

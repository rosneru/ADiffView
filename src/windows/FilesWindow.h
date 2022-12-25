#ifndef FILES_WINDOW_H
#define FILES_WINDOW_H

#include <string>
#include <vector>

#include <exec/ports.h>
#include <intuition/screens.h>
#include <workbench/workbench.h>

#include "OpenScreenBase.h"
#include "CommandBase.h"
#include "CmdFileRequester.h"
#include "WindowBase.h"

/**
 * Class for the window to open the two files on which the diff will
 * be performed.
 *
 *
 * @author Uwe Rosner
 * @date 21/10/2018
 */
class FilesWindow : public WindowBase
{
public:
  FilesWindow(std::vector<WindowBase*>& windowArray,
              ScreenBase& screen,
              struct MsgPort* pIdcmpMsgPort,
              std::string& leftFilePath,
              std::string& rightFilePath,
              CommandBase& cmdDiff,
              CommandBase& cmdCloseFilesWindow,
              MenuBase* pMenu);

  virtual ~FilesWindow();

  void Refresh();

  /**
   * Opening the window.
   *
   * @returns
   * When ok: true, false if opening fails
   */
  bool open(InitialPosition initialPos = WindowBase::IP_Center);

  /**
   * Handles given IDCMP event.
   *
   * @returns
   * If this event was handled: true; if it was not handled: false..
   */
  void handleIDCMP(const struct IntuiMessage* pMsg);

  /**
   * Handles the given progress event.
   *
   * Prints the current pDescription text of given pProgrMsg and
   * draws a progress bar representing the actual percentual value.
   */
  void HandleAppMessage(struct AppMessage* pAppMsg);

private:
  std::string& m_LeftFilePath;
  std::string& m_RightFilePath;

  CommandBase& m_CmdDiff;
  CommandBase& m_CmdCloseFilesWindow;

  CmdFileRequester m_CmdSelectLeftFile;
  CmdFileRequester m_CmdSelectRightFile;

  const ULONG m_MaxPathLength;

  /**
   * IDs to help to interpret the events of this window's Gadtools
   * gadgets.
   */
  enum GadgetId
  {
    GID_StrLeftFile,
    GID_StrRightFile,
    GID_BtnLeftFile,
    GID_BtnRightFile,
    GID_BtnDiff,
    GID_BtnSwap,
    GID_BtnClear,
    GID_BtnCancel,
  };

  struct Gadget* m_pGadtoolsContext;
  struct Gadget* m_pGadStrLeftFile;
  struct Gadget* m_pGadStrRightFile;
  struct Gadget* m_pGadBtnSelectLeft;
  struct Gadget* m_pGadBtnSelectRight;
  struct Gadget* m_pGadBtnDiff;
  struct Gadget* m_pGadBtnSwap;
  struct Gadget* m_pGadBtnClear;
  struct Gadget* m_pGadBtnCancel;

  //
  // The next two are called from HandleIDCMP() to get that method not
  // overblown.
  //
  void handleGadgetEvent(struct Gadget* pGadget);
  void handleVanillaKey(UWORD code);

  //
  // These next methods are the main functions of this window and are
  // mapped to the butons and keys.
  //
  void selectLeftFile();
  void selectRightFile();
  void swapFiles();
  void compare();
  void clear();

  void cleanup();

  /**
   * Enables or disables the 'Diff' and 'Swap' buttons depending on
   * some conditions.
   */
  void checkEnableButtons();

  /**
   * Returns the first string gadget of
   *   {m_pGadStrLeftFile, m_pGadStrRightFile}
   * which contains currently no text.
   *
   * Returns NULL if both string gadgets contain text
   */
  struct Gadget* getFirstEmptyStringGadget();
};


#endif

#ifndef CMD_COPY_SELECTION_H
#define CMD_COPY_SELECTION_H

#include "DiffWindow.h"
#include "Clipboard.h"
#include "CommandBase.h"

/**
 * A command to copy the selected text of DiffWindwo into the Clipboard
 *
 * @author Uwe Rosner
 * @date 20/01/2023
 */
class CmdCopySelection : public CommandBase
{
public:
  CmdCopySelection(std::vector<WindowBase*>* pAllWindowsVector,
                   const DiffWindow& diffWindow);

  virtual ~CmdCopySelection();
  virtual void Execute(struct Window* pActiveWindow);

private:
  const DiffWindow& m_DiffWindow;
  Clipboard m_Clipboard;
};

#endif

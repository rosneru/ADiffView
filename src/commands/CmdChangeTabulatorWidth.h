#ifndef CMD_CHANGE_TABULATOR_WIDTH_H
#define CMD_CHANGE_TABULATOR_WIDTH_H

#include "MenuBase.h"
#include "CommandBase.h"
#include "DiffWindow.h"
#include "DiffWorker.h"

/**
 * Command for changing the tabulator width.
 *
 * It will redisplay the DiffDocument in DiffWindow with the new
 * tabulator width.
 *
 * @author Uwe Rosner
 * @date 16/11/2021
 */
class CmdChangeTabulatorWidth : public CommandBase
{
public:
  CmdChangeTabulatorWidth(std::vector<WindowBase*>* pAllWindowsVector,
                          size_t tabulatorWidth,
                          DiffWindow& diffWindow,
                          DiffWorker& diffWorker);

  virtual ~CmdChangeTabulatorWidth();
  virtual void Execute(struct Window* pActiveWindow);

private:
  const size_t m_TabulatorWidth;
  DiffWindow& m_DiffWindow;
  DiffWorker& m_DiffWorker;
};

#endif

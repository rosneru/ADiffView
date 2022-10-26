#ifndef CMD_WINDOW_TITLE_QUICK_MSG_H
#define CMD_WINDOW_TITLE_QUICK_MSG_H

#include <string>

#include "CommandBase.h"

/**
 * Command to display a message in the title of a window for a short
 * time.
 *
 * @author Uwe Rosner
 * @date 28/11/2021
 */
class CmdWindowTitleQuickMsg : public CommandBase
{
public:
  CmdWindowTitleQuickMsg(std::vector<WindowBase*>* pAllWindowsVector);
  virtual ~CmdWindowTitleQuickMsg();

  virtual void Execute(struct Window* pActiveWindow);

  void setTitleMessage(const std::string message);

private:
  std::string m_Message;
  std::string m_OriginalWindowTitle;
  std::string m_ConcatenatedWindowTitle;
};

#endif

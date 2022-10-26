#ifndef MENU_FILES_WINDOW_H
#define MENU_FILES_WINDOW_H

#include "CommandBase.h"
#include "MenuBase.h"

class MenuFilesWindow : public MenuBase
{
public:
  MenuFilesWindow(CommandBase* pCmdOpenFilesWindow,
                  CommandBase* pCmdAboutRequester,
                  CommandBase* pCmdQuit);
};

#endif

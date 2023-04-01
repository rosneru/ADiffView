#ifndef CMD_EDIT_FILE_H
#define CMD_EDIT_FILE_H

#include "ADiffViewArgs.h"
#include "CommandBase.h"

/**
 * Command for
 *
 * @author Uwe Rosner
 * @date 17/03/2023
 */
class CmdEditFile : public CommandBase
{
public:
  CmdEditFile(std::vector<WindowBase*>* pAllWindowsVector,
              const ADiffViewArgs& args,
              const std::string& pathToFile);
  
  virtual ~CmdEditFile();

  virtual void Execute(struct Window* pActiveWindow);

private:
  const ADiffViewArgs& m_Args;
  const std::string& m_PathToFile;
};

#endif

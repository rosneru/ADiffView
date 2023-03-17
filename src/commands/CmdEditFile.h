#ifndef CMD_EDIT_FILE_H
#define CMD_EDIT_FILE_H

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
              const std::string& pathToFile,
              const std::string& pathToEditor);
  
  virtual ~CmdEditFile();

  virtual void Execute(struct Window* pActiveWindow);

private:
  const std::string& m_PathToFile;
  const std::string& m_PathToEditor;
};

#endif

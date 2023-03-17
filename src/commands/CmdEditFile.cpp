#include "CmdEditFile.h"


CmdEditFile::CmdEditFile(std::vector<WindowBase*>* pAllWindowsVector,
                         const std::string& pathToFile,
                         const std::string& pathToEditor)
  : CommandBase(pAllWindowsVector),
    m_PathToFile(pathToFile),
    m_PathToEditor(pathToEditor)
{
}

CmdEditFile::~CmdEditFile()
{

}

void CmdEditFile::Execute(struct Window* pActiveWindow)
{

}

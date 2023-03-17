#ifdef __clang__
  #include <clib/dos_protos.h>
#else
  #include <proto/dos.h>
#endif

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
    std::string systemCall = m_PathToEditor;
    systemCall += " ";
    systemCall += m_PathToFile;

    SystemTagList(systemCall.c_str(), TAG_DONE);
}

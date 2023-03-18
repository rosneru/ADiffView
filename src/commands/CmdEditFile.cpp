#ifdef __clang__
  #include <clib/dos_protos.h>
#else
  #include <proto/dos.h>
#endif

#include "CmdRequester.h"
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

    if(SystemTagList(systemCall.c_str(), TAG_DONE) != 0)
    {
      std::string msg = "Starting the editor with the following command failed:\n";
      msg += systemCall.c_str();
      
      CmdRequester cmdRequester(m_pAllWindowsVector, msg, "Error", "Ok");
      cmdRequester.Execute(pActiveWindow);
    }
}

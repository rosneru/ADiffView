#ifdef __clang__
  #include <clib/dos_protos.h>
#else
  #include <proto/dos.h>
#endif

#include "CmdRequester.h"
#include "CmdEditFile.h"


CmdEditFile::CmdEditFile(std::vector<WindowBase*>* pAllWindowsVector,
                         const ADiffViewArgs& args,
                         const std::string& pathToFile)
  : CommandBase(pAllWindowsVector),
    m_Args(args),
    m_PathToFile(pathToFile)
{

}

CmdEditFile::~CmdEditFile()
{

}

void CmdEditFile::Execute(struct Window* pActiveWindow)
{
  std::string systemCall = "Run ";
  systemCall += m_Args.getEditorPath();
  systemCall += " \"";
  systemCall += m_PathToFile;
  systemCall += "\"";

  if(m_Args.isEditorOnPubScreen())
  {
    systemCall += " PUBSCREEN ";
    systemCall += m_Args.getPubScreenName();
  }

  if(SystemTagList(systemCall.c_str(), TAG_DONE) != 0)
  {
    std::string msg = "Starting the editor with the following command failed:\n";
    msg += systemCall.c_str();
    
    CmdRequester cmdRequester(m_pAllWindowsVector, msg, "Error", "Ok");
    cmdRequester.Execute(pActiveWindow);
  }
}

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
  std::string systemCall = m_Args.getEditorPath();
  systemCall += " \"";
  systemCall += m_PathToFile;
  systemCall += "\"";

  if(m_Args.isEditorOnPubScreen())
  {
    systemCall += " PUBSCREEN ";
    systemCall += m_Args.getPubScreenName();
  }

  // Disable the "About..." item (this command) in all menus
  DisableInAllWindowMenus();

  // Set the busy pointer for all windows
  enableBusyPointerForAllWindows();

  if(SystemTagList(systemCall.c_str(), TAG_DONE) != 0)
  {
    std::string msg = "Starting the editor with the following command failed:\n";
    msg += systemCall.c_str();
    
    CmdRequester cmdRequester(m_pAllWindowsVector, msg, "Error", "Ok");
    cmdRequester.Execute(pActiveWindow);
  }

  // Set the normal pointer for all windows
  disableBusyPointerForAllWindows();

  // Enable the "About..." item (this command) in all menus
  EnableInAllWindowMenus();
}

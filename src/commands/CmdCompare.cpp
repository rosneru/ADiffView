#include "CmdCompare.h"

CmdCompare::CmdCompare(std::vector<WindowBase*>* pAllWindowsVector,
                 DiffWorker& diffWorker)
  : CommandBase(pAllWindowsVector),
    m_DiffWorker(diffWorker)
{

}

CmdCompare::~CmdCompare()
{

}

void CmdCompare::Execute(struct Window* pActiveWindow)
{
  // NOTE: Only use one of the methods below to start the diff calculation.

  // Run() starts the diff as asynchronous background process
  m_DiffWorker.run();

  // compare() starts the diff blocking inside the ui task
  // m_DiffWorker.compare();
}

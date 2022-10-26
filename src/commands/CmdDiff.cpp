#include "CmdDiff.h"

CmdDiff::CmdDiff(std::vector<WindowBase*>* pAllWindowsVector,
                 DiffWorker& diffWorker)
  : CommandBase(pAllWindowsVector),
    m_DiffWorker(diffWorker)
{

}

CmdDiff::~CmdDiff()
{

}

void CmdDiff::Execute(struct Window* pActiveWindow)
{
  // NOTE: Only use one of the methods below to start the diff calculation.

  // Run() starts the diff as asynchronous background process
  m_DiffWorker.Run();

  // Diff() starts the diff blocking inside the ui task
  // m_DiffWorker.Diff();
}

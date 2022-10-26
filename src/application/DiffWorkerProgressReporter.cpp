#include "DiffWorkerProgressReporter.h"


DiffWorkerProgressReporter::DiffWorkerProgressReporter(struct MsgPort* pProgressPort, 
                                                       struct MsgPort*& pReplyPort)
  : ProgressReporter(pProgressPort, pReplyPort),
    m_ProgressOffset(0)
{

}

void DiffWorkerProgressReporter::SetValue(int progress)
{
  //
  // Reporting the 3 stages of diff-progress (preprocessing left file,
  // preprocessing right file, performing the diff as 0..33%, 33%..66%
  // and 66%..100%.
  //

  if(progress == 100)
  {
    if(m_ProgressOffset == 0)
    {
      m_ProgressOffset = 33;
      progress = 0;
    }
    else if(m_ProgressOffset == 33)
    {
      m_ProgressOffset = 66;
      progress = 0;
    }
    else if(m_ProgressOffset == 66)
    {
      m_ProgressOffset = 0;
      progress = -1;
    }
  }
  else
  {
    progress = progress / 3 + 1;
  }

  ProgressReporter::SetValue(m_ProgressOffset + progress);
}

void DiffWorkerProgressReporter::Reset()
{
  m_ProgressOffset = 0;
}

#ifdef __clang__
  #include <clib/exec_protos.h>
#else
  #include <proto/exec.h>
#endif

#include "MessageBox.h"
#include "DiffWorker.h"

DiffWorker::DiffWorker(std::string& leftFilePath,
                       std::string& rightFilePath,
                       DiffWindow& diffWindow,
                       ProgressWindow& progressWindow,
                       CmdOpenWindow& cmdOpenFilesWindow,
                       CmdCloseWindow& cmdCloseFilesWindow,
                       MsgPort* pProgressPort,
                       bool& isCancelRequested,
                       bool& isExitAllowed,
                       bool bShowLineNumbers,
                       unsigned long tabSize)
  : m_LeftSrcFilePath(leftFilePath),
    m_RightSrcFilePath(rightFilePath),
    m_Progress(pProgressPort, m_pReplyPort),
    m_DiffWindow(diffWindow),
    m_ProgressWindow(progressWindow),
    m_CmdOpenFilesWindow(cmdOpenFilesWindow),
    m_CmdCloseFilesWindow(cmdCloseFilesWindow),
    m_IsCancelRequested(isCancelRequested),
    m_IsExitAllowed(isExitAllowed),
    m_bShowLineNumbers(bShowLineNumbers),
    m_TabSize(tabSize),
    m_pDiffDocument(NULL)
{
}

DiffWorker::~DiffWorker()
{
  if(m_pDiffDocument != NULL)
  {
    delete m_pDiffDocument;
    m_pDiffDocument = NULL;
  }
}


bool DiffWorker::Diff()
{
  m_IsCancelRequested = false;
  m_IsExitAllowed = false;
  m_Progress.Reset();

  //
  // Close FilesWindow, open ProgressWindow etc
  //
  m_ProgressWindow.open();
  m_CmdCloseFilesWindow.Execute(NULL);
  m_DiffWindow.close();


  //
  // Some basic tests
  //
  MessageBox request(m_ProgressWindow.getIntuiWindow());
  if(m_LeftSrcFilePath.length() == 0)
  {
    request.Show("Error: Left file name not set.",
                 "Ok");

    m_CmdOpenFilesWindow.Execute(NULL);
    m_ProgressWindow.close();

    m_IsExitAllowed = true;
    return false;
  }

  if(m_RightSrcFilePath.length() == 0)
  {
    request.Show("Error: Right file name not set.",
                 "Ok");

    m_CmdOpenFilesWindow.Execute(NULL);
    m_ProgressWindow.close();

    m_IsExitAllowed = true;
    return false;
  }

  if(m_pDiffDocument != NULL)
  {
    delete m_pDiffDocument;
    m_pDiffDocument = NULL;
  }

  try
  {
    m_StopWatch.Start();

    // NOTE: Creating the document starts the file compare
    m_pDiffDocument = new DiffDocument(m_LeftSrcFilePath.c_str(),
                                       m_RightSrcFilePath.c_str(),
                                       m_IsCancelRequested,
                                       m_StopWatch,
                                       m_Progress,
                                       m_bShowLineNumbers,
                                       m_TabSize);

    if(m_pDiffDocument->getNumDifferences() < 1)
    {
      throw "No differences found: the files are equal.";
    }
  }
  catch(const char* pError)
  {
    if(!m_IsCancelRequested)
    {
      request.Show(pError, "Ok");
    }

    m_CmdOpenFilesWindow.Execute(NULL);
    m_ProgressWindow.close();

    m_Progress.SetValue(100);

    m_IsExitAllowed = true;
    return false;
  }
  catch (std::bad_alloc& exception) 
  { 
    if(!m_IsCancelRequested)
    {
      request.Show(exception.what(), "Ok");
    }
  } 

  m_DiffWindow.open(WindowBase::IP_Fill);
  m_DiffWindow.setDocument(m_pDiffDocument);

  m_ProgressWindow.close();

  m_IsExitAllowed = true;
  return true;
}


void DiffWorker::doWork()
{
  Diff();
}

const DiffDocument* DiffWorker::getDiffDocument() const
{
  return m_pDiffDocument;
}

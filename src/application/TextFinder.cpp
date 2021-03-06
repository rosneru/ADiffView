#ifdef __clang__
  #include <clib/intuition_protos.h>
#else
  #include <proto/intuition.h>
#endif

#include <string.h>

#include "DiffFileSearchResult.h"
#include "TextFinder.h"


TextFinder::TextFinder(const DiffWorker& diffWorker,
                       DiffWindow& diffWindow,
                       unsigned long tabSize)
  : m_DiffWorker(diffWorker),
    m_DiffWindow(diffWindow),
    m_TabSize(tabSize),
    m_pDiffDocument(NULL),
    m_pSearchEngine(NULL),
    m_pNewSearchEngine(NULL),
    m_pFormerResult(NULL)
{
}

TextFinder::~TextFinder()
{
  if(m_pFormerResult != NULL)
  {
    delete m_pFormerResult;
    m_pFormerResult = NULL;
  }

  if(m_pNewSearchEngine != NULL)
  {
    delete m_pNewSearchEngine;
    m_pNewSearchEngine = NULL;
  }
  
  if(m_pSearchEngine != NULL)
  {
    delete m_pSearchEngine;
    m_pSearchEngine = NULL;
  }

}


bool TextFinder::displayFirstResult()
{
  if((m_pSearchEngine == NULL) && (m_pNewSearchEngine == NULL))
  {
    return false;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    // No left text area available - should not be possible
    return false;
  }

  // Perform a new search if the document has changed
  applyDocumentChanged();

  // Apply a new search engine if one exists
  applyNewSearchEngine();

  /**
   * Get the first search result (the next one starting from line id 0)
   */
  DiffFileSearchResult* pResult = m_pSearchEngine->getNextResult(0);
  if(pResult == NULL)
  {
    signalNoResultFound();
    return false;
  }

  unmarkFormerResult();

  markNewResult(pResult);

  scrollToNewResult(pResult);

  return true;
}

bool TextFinder::displayLastResult()
{
  if((m_pSearchEngine == NULL) && (m_pNewSearchEngine == NULL))
  {
    return false;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    // No left text area available - should not be possible
    return false;
  }

  // Perform a new search if the document has changed
  applyDocumentChanged();

  // Apply a new search engine if one exists
  applyNewSearchEngine();

  /**
   * Get the last search result (search backwards from last line id)
   */
  DiffFileSearchResult* pResult = 
    m_pSearchEngine->getPrevResult(m_pDiffDocument->getNumLines() - 1);

  if(pResult == NULL)
  {
    signalNoResultFound();
    return false;
  }

  unmarkFormerResult();

  markNewResult(pResult);

  scrollToNewResult(pResult);

  return true;
}


bool TextFinder::displayNextResult()
{
  if((m_pSearchEngine == NULL) && (m_pNewSearchEngine == NULL))
  {
    return false;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    // No left text area available - should not be possible
    return false;
  }
  
  // Perform a new search if the document has changed
  applyDocumentChanged();

  // Apply a new search engine if one exists and remember the last
  // search result of the old search engine
  applyNewSearchEngine();


  DiffFileSearchResult* pResult = NULL;
  if(m_pFormerResult == NULL || 
     !pLeftTextArea->isLineVisible(m_pFormerResult->getLineId()))
  {
    // No former search result exists or it is currently not displayed
    // in window. So get the next search result from current window top
    // line id.
    pResult = m_pSearchEngine->getNextResult(pLeftTextArea->getY());
  }
  else
  {
    // The former result is currently displayed. So getting the 
    // next result after that former result, not from the window 
    // top line.
    pResult = m_pSearchEngine->getNextResult();
  }

  if(pResult == NULL)
  {
    signalNoResultFound();
    return false;
  }

  unmarkFormerResult();

  markNewResult(pResult);

  scrollToNewResult(pResult);

  return true;
}


bool TextFinder::displayPrevResult()
{
  if((m_pSearchEngine == NULL) && (m_pNewSearchEngine == NULL))
  {
    return false;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  if(pLeftTextArea == NULL)
  {
    // No left text area available - should not be possible
    return false;
  }
  
  // Perform a new search if the document has changed
  applyDocumentChanged();

  // Apply a new search engine if one exists and remember the last
  // search result of the old search engine
  applyNewSearchEngine();


  DiffFileSearchResult* pResult = NULL;
  if(m_pFormerResult == NULL || 
     !pLeftTextArea->isLineVisible(m_pFormerResult->getLineId()))
  {
    // No former search result exists or it is currently not displayed
    // in window. So get the previous search result from current window
    // bottom line id.
    pResult = m_pSearchEngine->getPrevResult(pLeftTextArea->getY() 
                                             + pLeftTextArea->getMaxVisibleLines());
  }
  else
  {
    // The former result is currently displayed. So getting the 
    // previous result above that former result, not from the window 
    // top line.
    pResult = m_pSearchEngine->getPrevResult();
  }

  if(pResult == NULL)
  {
    signalNoResultFound();
    return false;
  }

  unmarkFormerResult();

  markNewResult(pResult);

  scrollToNewResult(pResult);

  return true;
}


void TextFinder::applyDocumentChanged()
{
  if(hasDiffDocumentChanged() == false)
  {
    return;
  }

  // Meanwhile the user 'opened' another diff document
  if(m_pSearchEngine != NULL)
  {
    m_pNewSearchEngine = createNewSearchEngine(m_pSearchEngine->getSearchString().c_str(),
                                                m_pSearchEngine->isCaseIgnored(),
                                                m_pSearchEngine->getLocation());
  }
}


void TextFinder::applyNewSearchEngine()
{
  if(m_pNewSearchEngine == NULL)
  {
    return;
  }

  unmarkFormerResult();

  m_pFormerResult = NULL;

  // Old search engine is not needed anymore as the new one will be
  // taken
  delete m_pSearchEngine;

  // The new search engine now becomes the current one
  m_pSearchEngine = m_pNewSearchEngine;

  // And is not 'the new search engine' anymore
  m_pNewSearchEngine = NULL;
}

void TextFinder::signalNoResultFound()
{
  DisplayBeep(m_DiffWindow.getScreen().IntuiScreen());
}

void TextFinder::unmarkFormerResult()
{
  // Clear the former search result visually
  if(m_pFormerResult == NULL)
  {
    return;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  DiffWindowTextArea* pRightTextArea = m_DiffWindow.getRightTextArea();
  if( (pLeftTextArea == NULL) || (pLeftTextArea == NULL) )
  {
    return;
  }

  // Clear all result selections
  pLeftTextArea->clearSelection();
  pRightTextArea->clearSelection();

  // Re-render the line line with the former search result to visually
  // remove the selection
  m_DiffWindow.renderDocuments(m_pFormerResult->getLineId());
}

void TextFinder::markNewResult(DiffFileSearchResult* pResult)
{
  // Clear the former search result visually
  if(pResult == NULL)
  {
    return;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  DiffWindowTextArea* pRightTextArea = m_DiffWindow.getRightTextArea();
  if( (pLeftTextArea == NULL) || (pLeftTextArea == NULL) )
  {
    return;
  }

  size_t searchStringLength = m_pSearchEngine->getSearchString().length();
  int stopCharId = pResult->getCharId() + searchStringLength - 1;

  if(pResult->getLocation() == DiffFileSearchResult::LeftFile)
  {
    pLeftTextArea->addSelection(pResult->getLineId(),
                                pResult->getCharId(),
                                stopCharId);
  }
  else if(pResult->getLocation() == DiffFileSearchResult::RightFile)
  {
    pRightTextArea->addSelection(pResult->getLineId(),
                                 pResult->getCharId(),
                                 stopCharId);
  }

  // Remember newResult as formerResult
  if(m_pFormerResult != NULL)
  {
    delete m_pFormerResult;
  }

  m_pFormerResult = new DiffFileSearchResult(pResult->getLocation(),
                                             pResult->getLineId(),
                                             pResult->getCharId());
}

void TextFinder::scrollToNewResult(DiffFileSearchResult* pResult)
{
  if((pResult == NULL) || (m_pDiffDocument == NULL) || (m_pSearchEngine == NULL))
  {
    return;
  }

  // Get the DiffLine which contains the result  
  const DiffLine* pResultLine = NULL;
  if(pResult->getLocation() == DiffFileSearchResult::LeftFile)
  {
    pResultLine = m_pDiffDocument->getLeftDiffFile()[pResult->getLineId()];
  }
  else if(pResult->getLocation() == DiffFileSearchResult::RightFile)
  {
    pResultLine = m_pDiffDocument->getRightDiffFile()[pResult->getLineId()];
  }
  else
  {
    return;
  }

  if(pResultLine == NULL)
  {
    return;
  }

  // Get the postition (column) of the search string in original text line
  size_t srcTextColumn = pResult->getCharId();

  // Get the position of search string in rendered line (with TABulators
  // calculated in). TODO: Parametrize TAB_WIDTH
  size_t resultingTextColumn = pResultLine->getRenderColumn(srcTextColumn,
                                                            m_TabSize);

  // If necessary scroll the window to have the result visible
  size_t searchStringLength = m_pSearchEngine->getSearchString().length();
  bool hasScrolled = m_DiffWindow.scrollToPage(resultingTextColumn,
                                               pResult->getLineId(),
                                               searchStringLength,
                                               1);

  if(hasScrolled)
  {
    // Re-render complete document
    m_DiffWindow.renderDocuments();
  }
  else
  {
    // Re-render only line with the result
    m_DiffWindow.renderDocuments(pResult->getLineId());
  }
}


DiffFileSearchEngine* TextFinder::createNewSearchEngine(const char* pSearchText,
                                                        bool isCaseIgnored,
                                                        SearchLocation location)
{
  DiffFileSearchEngine* pNewSearchEngine = NULL;

  if((pSearchText == NULL) || (strlen(pSearchText) < 1))
  {
    return NULL;
  }

  DiffWindowTextArea* pLeftTextArea = m_DiffWindow.getLeftTextArea();
  DiffWindowTextArea* pRightTextArea = m_DiffWindow.getRightTextArea();
  if((pLeftTextArea == NULL) || (pRightTextArea == NULL))
  {
    return NULL;
  }


  // Get current document
  m_pDiffDocument = m_DiffWorker.getDiffDocument();
  if(m_pDiffDocument == NULL)
  {
    return NULL;
  }

  // This already performes the search of all occurrences of pSearchText
  // in one or both files (dependent on location parameter) and can take
  // some time.
  //
  // TODO: Consider to create a task.
  pNewSearchEngine = new DiffFileSearchEngine(m_pDiffDocument->getLeftDiffFile(),
                                              m_pDiffDocument->getRightDiffFile(),
                                              pSearchText,
                                              isCaseIgnored,
                                              location);

  return pNewSearchEngine;
}


const char* TextFinder::getSearchText() const
{
  if(m_pSearchEngine == NULL)
  {
    return ""; // Default
  }

  return m_pSearchEngine->getSearchString().c_str();
}

void TextFinder::setSearchText(const char* pSearchText)
{
  if(m_pNewSearchEngine != NULL)
  {
    if(m_pNewSearchEngine->getSearchString() == pSearchText)
    {
      // Search text hasn't changed
      return;
    }

    // Apply changed search text. But there's already a new created
    // search engine with some other search parameter changed. So we
    // create another new search engine and take the other parameters
    // from the already existing new search engine.
    DiffFileSearchEngine* pTmpSearchEngine = NULL;
    pTmpSearchEngine = createNewSearchEngine(pSearchText,
                                             m_pNewSearchEngine->isCaseIgnored(),
                                             m_pNewSearchEngine->getLocation());

    delete m_pNewSearchEngine;
    m_pNewSearchEngine = pTmpSearchEngine;
  }
  else if(m_pSearchEngine != NULL)
  {
    if(m_pSearchEngine->getSearchString() == pSearchText)
    {
      // Search text hasn't changed
      return;
    }

    // Apply changed search text. A new search engine is created which
    // takes all other parameters from the current search engine. from
    // the already existing new search engine.
    m_pNewSearchEngine = createNewSearchEngine(pSearchText,
                                               m_pSearchEngine->isCaseIgnored(),
                                               m_pSearchEngine->getLocation());
  }
  else
  {
    // No search engine exists
    m_pNewSearchEngine = createNewSearchEngine(pSearchText,
                                               isCaseIgnored(),
                                               getLocation());
  }
}


bool TextFinder::isCaseIgnored() const
{
  if(m_pSearchEngine == NULL)
  {
    return true;  // Default
  }

  return m_pSearchEngine->isCaseIgnored();
}


void TextFinder::setCaseIgnored(bool isCaseIgnored)
{
  if(m_pNewSearchEngine != NULL)
  {
    if(m_pNewSearchEngine->isCaseIgnored() == isCaseIgnored)
    {
      // Case ignored flag hasn't changed
      return;
    }

    // Apply changed isCaseIgnored parameter. But there's already a new
    // created search engine with some other search parameter changed.
    // So we create another new search engine and take the other
    // parameters from the already existing new search engine.
    DiffFileSearchEngine* pTmpSearchEngine = NULL;
    pTmpSearchEngine = createNewSearchEngine(m_pNewSearchEngine->getSearchString().c_str(),
                                             isCaseIgnored,
                                             m_pNewSearchEngine->getLocation());

    delete m_pNewSearchEngine;
    m_pNewSearchEngine = pTmpSearchEngine;
  }
  else if(m_pSearchEngine != NULL)
  {
    if(m_pSearchEngine->isCaseIgnored() == isCaseIgnored)
    {
      // Case ignored flag hasn't changed
      return;
    }

    // Apply changed isCaseIgnored parameter. A new search engine is
    // created which takes all other parameters from the current search
    // engine. from the already existing new search engine.
    m_pNewSearchEngine = createNewSearchEngine(m_pSearchEngine->getSearchString().c_str(),
                                               isCaseIgnored,
                                               m_pSearchEngine->getLocation());
  }
  else
  {
    // No search engine exists
    m_pNewSearchEngine = createNewSearchEngine(getSearchText(),
                                               isCaseIgnored,
                                               getLocation());
  }
}


SearchLocation TextFinder::getLocation() const
{
  if(m_pSearchEngine == NULL)
  {
    return SL_BothFiles;  // Default
  }

  return m_pSearchEngine->getLocation();
}

void TextFinder::setLocation(SearchLocation location)
{
  if(m_pNewSearchEngine != NULL)
  {
    if(m_pNewSearchEngine->getLocation() == location)
    {
      // Location hasn't changed
      return;
    }

    // Apply changed location parameter. But there's already a new
    // created search engine with some other search parameter changed.
    // So we create another new search engine and take the other
    // parameters from the already existing new search engine.
    DiffFileSearchEngine* pTmpSearchEngine = NULL;
    pTmpSearchEngine = createNewSearchEngine(m_pNewSearchEngine->getSearchString().c_str(),
                                             m_pNewSearchEngine->isCaseIgnored(),
                                             location);

    delete m_pNewSearchEngine;
    m_pNewSearchEngine = pTmpSearchEngine;
  }
  else if(m_pSearchEngine != NULL)
  {
    if(m_pSearchEngine->getLocation() == location)
    {
      // Location hasn't changed
      return;
    }

    // Apply changed location parameter. A new search engine is created
    // which takes all other parameters from the current search engine.
    // from the already existing new search engine.
    m_pNewSearchEngine = createNewSearchEngine(m_pSearchEngine->getSearchString().c_str(),
                                               m_pSearchEngine->isCaseIgnored(),
                                               location);
  }
  else
  {
    // No search engine exists
    m_pNewSearchEngine = createNewSearchEngine(getSearchText(),
                                               isCaseIgnored(),
                                               location);
  }
}


bool TextFinder::hasDiffDocumentChanged() const
{
  const DiffDocument* pWorkerDiffDoc = m_DiffWorker.getDiffDocument();
  return m_pDiffDocument != pWorkerDiffDoc;
}

#ifndef TEXT_FINDER_H
#define TEXT_FINDER_H

#include <string>
#include "CmdWindowTitleQuickMsg.h"
#include "DiffDocument.h"
#include "DiffFileSearchEngine.h"
#include "DiffWindow.h"
#include "DiffWorker.h"
#include "SearchOptions.h"



/**
 * This class exposes methods to set search parameters (The 
 * 'Search text', the 'isCaseIgnored' property and the 'location' 
 * property). 
 * 
 * Whenever such a parameter is set and its value changed from the 
 * formerly set one a new search is performed in the DiffDocument which 
 * is currently opened in DiffWorker.
 * 
 * Additionally, methods are provided to navigate between search results
 * in the DiffWindow when it is open.
 *
 * @author Uwe Rosner
 * @date 12/04/2021
 */
class TextFinder
{
public:
  TextFinder(DiffWorker& diffWorker,
             DiffWindow& diffWindow,
             unsigned long tabSize,
             CmdWindowTitleQuickMsg& cmdWindowTitleQuickMsg);

  virtual ~TextFinder();

  bool displayFirstResult(bool doSignalIfNoResultFound = true);
  bool displayLastResult(bool doSignalIfNoResultFound = true);
  bool displayNextResult(bool doSignalIfNoResultFound = true);
  bool displayPrevResult(bool doSignalIfNoResultFound = true);

  bool isFormerResultDisplayed() const;

  /**
   * Get the current 'Text to search for'
   */
  const char* getSearchText() const;

  /**
   * Set the 'Text to search for'. If this differs from the text
   * formerly set, a new search is performed.
   */
  void setSearchText(const char* pSearchText);

  /**
   * Get the current 'Is case ignored' property.
   */
  bool isCaseIgnored() const;

  /**
   * Set the 'Is case ignored'. If the new value differs from the
   * formerly set value, a new search is performed.
   */
  void setCaseIgnored(bool isCaseIgnored);

  /**
   * Get the current 'Search location' property.
   */
  SearchLocation getLocation() const;

  /**
   * Set the 'Search location'. If the new value differs from the
   * formerly set value, a new search is performed.
   */
  void setLocation(SearchLocation location);

private:
  DiffWorker& m_DiffWorker;
  DiffWindow& m_DiffWindow;
  unsigned long m_TabSize;
  CmdWindowTitleQuickMsg& m_CmdWindowTitleQuickMsg;

  DiffDocument* m_pDiffDocument;
  DiffFileSearchEngine* m_pSearchEngine;
  DiffFileSearchEngine* m_pNewSearchEngine;

  DiffFileSearchResult* m_pFormerResult;

  void applyDocumentChanged();
  void applyNewSearchEngine();
  void signalNoResultFound();
  void unmarkFormerResult();
  void markNewResult(DiffFileSearchResult* pResult);
  void scrollToNewResult(DiffFileSearchResult* pResult);

  DiffFileSearchEngine* createNewSearchEngine(const char* pSearchText, 
                                              bool isCaseIgnored, 
                                              SearchLocation location);

  bool hasDiffDocumentChanged() const;
};

#endif

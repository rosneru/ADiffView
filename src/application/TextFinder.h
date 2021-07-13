#ifndef TEXT_FINDER_H
#define TEXT_FINDER_H

#include <string>
#include "DiffDocument.h"
#include "DiffFileSearchEngine.h"
#include "DiffWindow.h"
#include "DiffWorker.h"
#include "SearchOptions.h"



/**
 * Command to find for text in diff result window.
 *
 * @author Uwe Rosner
 * @date 12/04/2021
 */
class TextFinder
{
public:
  TextFinder(const DiffWorker& diffWorker,
             DiffWindow& diffWindow,
             unsigned long tabSize);

  virtual ~TextFinder();

  bool displayFirstResult();
  bool displayLastResult();
  bool displayNextResult();
  bool displayPrevResult();

  const char* getSearchText() const;
  void setSearchText(const char* pSearchText);

  bool isCaseIgnored() const;
  void setCaseIgnored(bool isCaseIgnored);

  SearchLocation getLocation() const;
  void setLocation(SearchLocation location);

private:
  const DiffWorker& m_DiffWorker;
  DiffWindow& m_DiffWindow;
  unsigned long m_TabSize;

  const DiffDocument* m_pDiffDocument;
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

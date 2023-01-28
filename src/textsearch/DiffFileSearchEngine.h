#ifndef DIFF_FILE_SEARCH_ENGINE_H
#define DIFF_FILE_SEARCH_ENGINE_H

#include <string>
#include <vector>

#include "DiffFileBase.h"
#include "DiffFileSearchResult.h"
#include "SearchOptions.h"

/**
 * A text search engine that operates on two DiffFiles.
 *
 * @author Uwe Rosner
 * @date 10/04/2021
 */
class DiffFileSearchEngine
{
public:
  DiffFileSearchEngine(const DiffFileBase& leftFile, 
                       const DiffFileBase& rightFile, 
                       const char* pSearchString,
                       bool isCaseIgnored,
                       SearchLocation location);
  virtual ~DiffFileSearchEngine();

  /**
   * Get the collection of all search results
   */
  const std::vector<DiffFileSearchResult*>& getResults() const;

  /**
   * Returns the number of search results
   */
  size_t getNumResults();

  /**
   * Returns the current result (that one the iterator points on)
   */  
  DiffFileSearchResult* getCurrentResult();

  /**
   * Returns the previous result (relative to the current one) or NULL if
   * there is none.
   */
  DiffFileSearchResult* getPrevResult();

  /**
   * Returns the next result above the given line id or NULL if there is
   * none.
   */
  DiffFileSearchResult* getPrevResult(size_t startLineId);
  
  /**
   * Returns the next result (from the current one) or NULL if there is
   * none.
   */
  DiffFileSearchResult* getNextResult();

  /**
   * Returns the next result after the given line id or NULL if there is
   * none.
   */
  DiffFileSearchResult* getNextResult(size_t startLineId);

  const std::string& getSearchString() const;
  bool isCaseIgnored() const;
  SearchLocation getLocation() const;

private:
  const DiffFileBase& m_LeftFile;
  const DiffFileBase& m_RightFile;
  std::string m_SearchString;
  bool m_IsCaseIgnored;
  SearchLocation m_Location;

  std::vector<DiffFileSearchResult*> m_Results;
  std::vector<DiffFileSearchResult*>::iterator m_ResultsIterator;

  void find();

  char* findSubStr(const char* pHaystack, const char* pNeedle);
};

#endif

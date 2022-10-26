#ifndef DIFF_FILE_SEARCH_ENGINE_STEADILY_H
#define DIFF_FILE_SEARCH_ENGINE_STEADILY_H

#include <string>
#include <vector>

#include "DiffFileBase.h"
#include "DiffFileSearchResult.h"

/**
 * A text search engine that operates on two DiffFiles.
 *
 * @author Uwe Rosner
 * @date 10/04/2021
 */
class DiffFileSearchEngineSteadily
{
public:
  DiffFileSearchEngineSteadily(const DiffFileBase& leftFile, 
                               const DiffFileBase& rightFile, 
                               const char* pSearchString);
  virtual ~DiffFileSearchEngineSteadily();

  size_t getNumResults();
  DiffFileSearchResult* getFirstResult(size_t startLineId);
  DiffFileSearchResult* getPrevResult();
  DiffFileSearchResult* getNextResult();

private:
  const DiffFileBase& m_LeftFile;
  const DiffFileBase& m_RightFile;
  std::string m_SearchString;
  std::vector<DiffFileSearchResult*> m_Results;
  std::vector<DiffFileSearchResult*>::iterator m_ResultsIterator;

  void find();
  

};

#endif

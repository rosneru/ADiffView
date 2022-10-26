#include <string.h>
#include "DiffFileSearchEngineSteadily.h"

DiffFileSearchEngineSteadily::DiffFileSearchEngineSteadily(const DiffFileBase& leftFile, 
                       const DiffFileBase& rightFile, 
                       const char* pSearchString)
  : m_LeftFile(leftFile),
    m_RightFile(rightFile),
    m_SearchString(pSearchString)
{
  find();
  m_ResultsIterator = m_Results.begin();
}

DiffFileSearchEngineSteadily::~DiffFileSearchEngineSteadily()
{
  std::vector<DiffFileSearchResult*>::iterator it;
  for(it = m_Results.begin(); it != m_Results.end(); it++)
  {
    delete *it;
  }
}


size_t DiffFileSearchEngineSteadily::getNumResults()
{
  return m_Results.size();
}

DiffFileSearchResult* DiffFileSearchEngineSteadily::getFirstResult(size_t startLineId)
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }

  m_ResultsIterator = m_Results.begin();

  // Forward iterator to the first result after or equal startLineId
  while((*m_ResultsIterator)->getLineId() < startLineId)
  {
    m_ResultsIterator++;
    if(m_ResultsIterator == m_Results.end())
    {
      // Avoid overflow: back to last valid item
      m_ResultsIterator = m_Results.begin();
    }
  }

  return (*m_ResultsIterator);
}

DiffFileSearchResult* DiffFileSearchEngineSteadily::getPrevResult()
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }
  
  if(m_ResultsIterator == m_Results.begin())
  {
    return NULL;
  }

  m_ResultsIterator--;
  return (*m_ResultsIterator);
}

DiffFileSearchResult* DiffFileSearchEngineSteadily::getNextResult()
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }

  m_ResultsIterator++;

  if(m_ResultsIterator == m_Results.end())
  {
    // Avoid overflow: back to last valid item
    m_ResultsIterator--;
    return NULL;
  }

  return (*m_ResultsIterator);
}



void DiffFileSearchEngineSteadily::find()
{
  DiffFileSearchResult* pResult;
  const char* pFoundAtPos;
  const char* pSearchStart;

  for(size_t lineId = 0; lineId < m_LeftFile.getNumLines(); lineId++)
  {
    // Try to find pStrToSearch in line of left file
    pSearchStart = m_LeftFile[lineId]->getText();
    while ((pFoundAtPos = strstr(pSearchStart, m_SearchString.c_str())) != NULL)
    {
      size_t charId = pFoundAtPos - m_LeftFile[lineId]->getText();
      pResult = new DiffFileSearchResult(DiffFileSearchResult::LeftFile, lineId, charId);
      m_Results.push_back(pResult);
      pSearchStart = pFoundAtPos + 1;
    }

    // Try to find pStrToSearch in line of right file
    pSearchStart = m_RightFile[lineId]->getText();
    while ((pFoundAtPos = strstr(pSearchStart, m_SearchString.c_str())) != NULL)
    {
      size_t charId = pFoundAtPos - m_RightFile[lineId]->getText();
      pResult = new DiffFileSearchResult(DiffFileSearchResult::RightFile, lineId, charId);
      m_Results.push_back(pResult);
      pSearchStart = pFoundAtPos + 1;
    }
  }
}

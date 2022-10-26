#include <string.h>
#include "DiffFileSearchEngine.h"

DiffFileSearchEngine::DiffFileSearchEngine(const DiffFileBase& leftFile, 
                                           const DiffFileBase& rightFile, 
                                           const char* pSearchString,
                                           bool isCaseIgnored,
                                           SearchLocation location)
  : m_LeftFile(leftFile),
    m_RightFile(rightFile),
    m_SearchString(pSearchString),
    m_IsCaseIgnored(isCaseIgnored),
    m_Location(location)
{
  find();
  m_ResultsIterator = m_Results.begin();
  m_ResultsIterator--;
}

DiffFileSearchEngine::~DiffFileSearchEngine()
{
  std::vector<DiffFileSearchResult*>::iterator it;
  for(it = m_Results.begin(); it != m_Results.end(); it++)
  {
    delete *it;
  }
}


size_t DiffFileSearchEngine::getNumResults()
{
  return m_Results.size();
}


DiffFileSearchResult* DiffFileSearchEngine::getCurrentResult()
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }

  if(m_ResultsIterator == (m_Results.begin() - 1))
  {
    return NULL;
  }

  return (*m_ResultsIterator);
}


DiffFileSearchResult* DiffFileSearchEngine::getPrevResult()
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }

  if(m_ResultsIterator == (m_Results.begin() - 1))
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

DiffFileSearchResult* DiffFileSearchEngine::getPrevResult(size_t startLineId)
{
  if(m_Results.size() < 1)
  {
    return NULL;
  }

  if(startLineId >= m_LeftFile.getNumLines())
  {
    // Avoid limit overrun
    startLineId = m_LeftFile.getNumLines() -1;
  }

  m_ResultsIterator = m_Results.end() - 1;

  // Forward iterator to the first result after or equal startLineId
  while((*m_ResultsIterator)->getLineId() >= startLineId)
  {
    m_ResultsIterator--;
    if(m_ResultsIterator == (m_Results.begin() - 1))
    {
      // Avoid overflow: back to last valid item
      m_ResultsIterator = m_Results.begin();
      return NULL;
    }
  }

  return (*m_ResultsIterator);
}

DiffFileSearchResult* DiffFileSearchEngine::getNextResult()
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

DiffFileSearchResult* DiffFileSearchEngine::getNextResult(size_t startLineId)
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
      m_ResultsIterator--;
      return NULL;
    }
  }

  return (*m_ResultsIterator);
}


const std::string& DiffFileSearchEngine::getSearchString() const
{
  return m_SearchString;
}


bool DiffFileSearchEngine::isCaseIgnored() const
{
  return m_IsCaseIgnored;
}


SearchLocation DiffFileSearchEngine::getLocation() const
{
  return m_Location;
}


void DiffFileSearchEngine::find()
{
  if(m_SearchString.length() < 1)
  {
    return;
  }

  DiffFileSearchResult* pResult;
  const char* pFoundAtPos;
  const char* pSearchStart;

  for(size_t lineId = 0; lineId < m_LeftFile.getNumLines(); lineId++)
  {
    // Depending on the location settings try to find m_SearchString in
    // the left file
    if((m_Location == SL_BothFiles) 
    || (m_Location == SL_LeftFile)
    || ((m_Location == SL_Differences) && (m_LeftFile[lineId]->getState() != DiffLine::Normal)))
    {
      pSearchStart = m_LeftFile[lineId]->getText();
      while ((pFoundAtPos = findSubStr(pSearchStart, m_SearchString.c_str())) != NULL)
      {
        size_t charId = pFoundAtPos - m_LeftFile[lineId]->getText();
        pResult = new DiffFileSearchResult(DiffFileSearchResult::LeftFile, lineId, charId);
        m_Results.push_back(pResult);
        pSearchStart = pFoundAtPos + 1;
      }
    }

    // Depending on the location settings try to find m_SearchString in
    // the right file
    if((m_Location == SL_BothFiles) 
    || (m_Location == SL_RightFile)
    || ((m_Location == SL_Differences) && (m_RightFile[lineId]->getState() != DiffLine::Normal)))
    {
      pSearchStart = m_RightFile[lineId]->getText();
      while ((pFoundAtPos = findSubStr(pSearchStart, m_SearchString.c_str())) != NULL)
      {
        size_t charId = pFoundAtPos - m_RightFile[lineId]->getText();
        pResult = new DiffFileSearchResult(DiffFileSearchResult::RightFile, lineId, charId);
        m_Results.push_back(pResult);
        pSearchStart = pFoundAtPos + 1;
      }
    }
  }
}

char* myStrCaseStr(const char* pHaystack, const char* pNeedle)
{ 
  const char *c1,*c2;

  do 
  {
    c1 = pHaystack; 
    c2 = pNeedle;

    while(tolower(*c1) && tolower(*c1)==tolower(*c2)) 
    {
      c1++; c2++;
    }

    if (!*c2)
    {
      return (char *)pHaystack;
    }
  } 
  while(*pHaystack++);
  
  return (char *)0;
}


char* DiffFileSearchEngine::findSubStr(const char* pHaystack, 
                                       const char* pNeedle)
{
  if(m_IsCaseIgnored)
  {
    return (char*)myStrCaseStr(pHaystack, pNeedle);
  }
  else
  {
    return (char*)strstr(pHaystack, pNeedle);
  }
}

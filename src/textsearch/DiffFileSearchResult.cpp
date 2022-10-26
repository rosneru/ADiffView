#include "DiffFileSearchResult.h"

DiffFileSearchResult::DiffFileSearchResult(Location location, 
                                           size_t lineId, 
                                           size_t charId)
  : m_Location(location),
    m_LineId(lineId),
    m_CharId(charId)
{

}

DiffFileSearchResult::~DiffFileSearchResult()
{

}

bool DiffFileSearchResult::equals(DiffFileSearchResult* pOther)
{
  if(m_Location != pOther->m_Location)
  {
    return false;
  }

  if(m_LineId != pOther->m_LineId)
  {
    return false;
  }

  if(m_CharId != pOther->m_CharId)
  {
    return false;
  }

  return true;
}


bool DiffFileSearchResult::isBefore(DiffFileSearchResult* pOther)
{
  if(m_LineId < pOther->m_LineId)
  {
    return true;
  }

  if(m_LineId == pOther->m_LineId)
  {
    if(m_Location < pOther->m_Location)
    {
      return true;
    }
    else if(m_Location == pOther->m_Location)
    {
      if(m_CharId < pOther->m_CharId)
      {
        return true;
      }
    }
  }
  
  return false;
}


DiffFileSearchResult::Location DiffFileSearchResult::getLocation() const
{
  return m_Location;
}


size_t DiffFileSearchResult::getLineId() const
{
  return m_LineId;
}


size_t DiffFileSearchResult::getCharId() const
{
  return m_CharId;
}

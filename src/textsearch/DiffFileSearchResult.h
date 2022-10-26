#ifndef DIFF_FILE_SEARCH_RESULT_H
#define DIFF_FILE_SEARCH_RESULT_H

#include <stddef.h>

/**
 * The search result of DiffFileSearchEngine.
 *
 * @author Uwe Rosner
 * @date 10/04/2021
 */
class DiffFileSearchResult
{
public:
  enum Location
  {
    None = 0,
    LeftFile,
    RightFile
  };

  DiffFileSearchResult(Location location, size_t lineId, size_t charId);
  virtual ~DiffFileSearchResult();

  bool equals(DiffFileSearchResult* pOther);
  bool isBefore(DiffFileSearchResult* pOther);

  Location getLocation() const;
  size_t getLineId() const;
  size_t getCharId() const;

private:
  Location m_Location;
  size_t m_LineId;
  size_t m_CharId;
};

#endif

#ifndef SELECTION_BASE_H
#define SELECTION_BASE_H

#include <list>
#include <vector>
#include "DiffLine.h"
#include "TextSelectionLine.h"


/**
 * A base class for the different selection methods.
 *
 * @author Uwe Rosner
 * @date 14/02/2023
 */
class SelectionBase
{
public:
  SelectionBase(const std::vector<DiffLine*>& textLines);
  virtual ~SelectionBase();

  /**
   * Clears the selection completely.
   */
  virtual void clear() = 0;

  /**
   * Returns true if given line id and column id is part of this text
   * selection:
   */
  virtual bool isSelected(long lineId, long columnId) const = 0;

  /**
   * Returns the number of selected characters from given column on
   * given line.
   */
  virtual long getNumMarkedChars(long lineId, long columnId) const = 0;

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  virtual long getNextSelectionStart(long lineId, long columnId) const = 0;

  const std::list<long>& getUpdatedLineIds();
  void addUpdatedLine(long lineId);
  void addUpdatedLines(const std::vector<long>& linesRange);
  void clearUpdatedLineIds();

protected:
  const std::vector<DiffLine*>& m_TextLines;
  std::list<long> m_UpdatedLineIds;
};

#endif

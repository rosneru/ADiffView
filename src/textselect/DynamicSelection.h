#ifndef DYNAMIC_SELECTION_H
#define DYNAMIC_SELECTION_H

#include <list>
#include <vector>
#include "DiffLine.h"
#include "SelectionBase.h"
#include "TextSelectionLine.h"


/**
 * A selection object for one continuous selection block that can be
 * updated dynamically, for example when selecting text with the mouse.
 *
 * @author Uwe Rosner
 * @date 04/01/2023
 */
class DynamicSelection : public SelectionBase
{
public:
  DynamicSelection(const std::vector<DiffLine*>& textLines);
  virtual ~DynamicSelection();

  /**
   * Clears the selection completely.
   */
  void clear();

  /**
   * Start a selection at given line and column
   */
  void startSelection(long lineId, long columnId);

  /**
   * Update the selection to given line and column.
   */
  void updateSelection(long lineId, long columnId);

  /**
   * Returns true if given line id and column id is part of this text
   * selection:
   */
  bool isSelected(long lineId, long columnId) const;

  /**
   * Returns the number of selected characters from given column on
   * given line.
   */
  long getNumMarkedChars(long lineId, long columnId) const;

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(long lineId, long columnId) const;

  /**
   * Returns the id of the lowest selected line
   */
  long getMinLineId() const;

  /**
   * Returns the id of the highest selected line
   */
  long getMaxLineId() const;

  /**
   * Returns the total number of selected chars in this selection block
   * taking into account the actual text lines and their lengths given
   * in the constructor.
   */
  long getNumTotalSelectedChars() const;

private:
  long m_StartLineId;
  long m_StartColumnId;

  long m_MinLineId;
  long m_MinLineColumnId;
  long m_MaxLineId;
  long m_MaxLineColumnId;
};

#endif

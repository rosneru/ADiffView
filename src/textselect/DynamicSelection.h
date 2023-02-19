#ifndef DYNAMIC_SELECTION_H
#define DYNAMIC_SELECTION_H

#include <list>
#include <vector>
#include "DiffLine.h"
#include "SelectionBase.h"
#include "TextSelectionLine.h"


/**
 * A selection object for one continuous selection block that can be
 * updated dynamically, e.g. with the mouse.
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
   * Start a dynamic selection at given line and column
   */
  void startDynamicSelection(unsigned long lineId, unsigned long columnId);

  /**
   * Update a dynamic selection to given line and column.
   *
   * NOTE: If the new lineId is more than one line above or below the
   * current 'selection target point' it is limited one line distance.
   * So, in some cases this method must be called multiple times with
   * the same lineId to reach the requested lineId.
   */
  void updateDynamicSelection(unsigned long lineId, unsigned long columnId);

  /**
   * Returns true if given line id and column id is part of this text
   * selection:
   */
  bool isSelected(unsigned long lineId, unsigned long columnId) const;

  /**
   * Returns the number of selected characters from given column on
   * given line.
   */
  long getNumMarkedChars(unsigned long lineId, unsigned long columnId) const;

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(unsigned long lineId, unsigned long columnId) const;

  long getMinLineId() const;
  long getMaxLineId() const;
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

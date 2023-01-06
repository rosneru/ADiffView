#ifndef TEXT_SELECTION_H
#define TEXT_SELECTION_H

#include <list>
#include <vector>
#include "DiffLine.h"
#include "TextSelectionLine.h"

/**
 * A collection of lines which contain selected ranges of text.
 *
 * @author Uwe Rosner
 * @date 22/12/2020
 */
class TextSelection
{
public:
  TextSelection(const std::vector<DiffLine*>& textLines);
  virtual ~TextSelection();


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
   * Adds a selection block (range) to a line.
   * 
   * NOTE: Do not mix this with dynamic selection
   */
  void addBlock(unsigned long lineId, 
                unsigned long fromColumn, 
                unsigned long toColumn);

  void clear();

  long getNumMarkedChars(unsigned long lineId, unsigned long columnId);

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(unsigned long lineId, unsigned long columnId);

  const std::list<int>& getUpdatedLineIds() const;

private:
  const std::vector<DiffLine*>& m_TextLines;
  std::list<TextSelectionLine*> m_SelectedLines;
  std::list<int> m_UpdatedLineIds;

  enum UpdateDirection
  {
    START_UPWARD = 0,
    APPEND_UPWARD,
    REDUCE_TOP,
    STOP_UPWARD,
    START_DOWNWARD,
    APPEND_DOWNWARD,
    REDUCE_BOTTOM,
    STOP_DOWNWARD,
    NONE,
  };

  unsigned long m_SelectionStartLine;
  unsigned long m_SelectionStartColumn;
  unsigned long m_LowestLineId;
  unsigned long m_HighestLineId;

  UpdateDirection m_UpdateDirection;

  void clearFirstSelectionLine();
  void clearLastSelectionLine();

  unsigned long limitLineId(unsigned long lineId);
  UpdateDirection calcUpdateDirection(unsigned long lineId);

  TextSelectionLine* findSelectionLine(unsigned long lineId);
};

#endif

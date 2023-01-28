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

  /**
   * Clears the selection completely.
   */
  void clear();

  /**
   * Returns true if given line id and column id is part of this text
   * selection:
   */
  bool isSelected(unsigned long lineId, unsigned long columnId) const;

  /**
   * Returns the number of selected characters from given column on
   * given line.
   */
  long getNumMarkedChars(unsigned long lineId, unsigned long columnId);

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(unsigned long lineId, unsigned long columnId);

  /**
   * Returns a list<int> of the lineIds whoese selection state was
   * changed during last dynamic selection operation.
   */
  const std::list<int>& getUpdatedLineIds();
  
  /**
   * Returns the selection lines 
   */
  const std::list<TextSelectionLine*>* getSelectionLines() const;

  void clearUpdatedLineIds();

private:
  const std::vector<DiffLine*>& m_TextLines;
  std::list<TextSelectionLine*> m_SelectionLines;
  std::list<int> m_UpdatedLineIds;

  enum UpdateDirection
  {
    UD_START_UPWARD = 0,
    UD_APPEND_UPWARD,
    UD_REDUCE_TOP,
    UD_STOP_UPWARD,
    UD_START_DOWNWARD,
    UD_APPEND_DOWNWARD,
    UD_REDUCE_BOTTOM,
    UD_STOP_DOWNWARD,
    UD_NONE,
  };

  unsigned long m_StartLineId;    ///< Line where the dynamic selection started
  unsigned long m_StartColumnId;  ///< Column where the dynamic selection started
  unsigned long m_LowestLineId;   ///< Lowest line in dynamic selection block
  unsigned long m_HighestLineId;  ///< Highest line in dynamic selection block

  UpdateDirection m_UpdateDirection;

  void clearFirstSelectionLine();
  void clearLastSelectionLine();

  UpdateDirection calcUpdateDirection(unsigned long lineId);

  TextSelectionLine* findSelectionLine(unsigned long lineId);
};

#endif

#ifndef BLOCKWISE_SELECTION_H
#define BLOCKWISE_SELECTION_H

#include <list>
#include <vector>
#include "DiffLine.h"
#include "SelectionBase.h"


/**
 * A collection of lines which contain selected ranges of text.
 *
 * @author Uwe Rosner
 * @date 22/12/2020
 */
class BlockwiseSelection : public SelectionBase
{
public:
  BlockwiseSelection(const std::vector<DiffLine*>& textLines);
  virtual ~BlockwiseSelection();

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

  const std::list<TextSelectionLine*>* getSelectionLines() const;

private:
  std::list<TextSelectionLine*> m_SelectionLines;

  TextSelectionLine* findSelectionLine(unsigned long lineId);
};

#endif

#ifndef TEXT_SELECTION_LINE_H
#define TEXT_SELECTION_LINE_H

#include <vector>
#include "TextSelectionRange.h"

/**
 * A line with some ranges of selected text.
 *
 * @author Uwe Rosner
 * @date 20/12/2020
 */
class TextSelectionLine
{
public:
  TextSelectionLine(long lineId, long fromColumn, long toColumn);

  virtual ~TextSelectionLine();

  void addBlock(long fromColumn, long toColumn);

  /**
   * Returns true if given line id and column id is part of this text
   * selection:
   */
  bool isSelected(long columnId) const;

  TextSelectionRange* getFirstSelectedBlock();
  
  long getLineId() const;

  long getNumMarkedChars(long columnId);

  /**
   * Returns the columnId of the next selection start after given
   * column. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(long columnId);


private:
  std::vector<TextSelectionRange*> m_SelectedBlocks;
  long m_LineId;
};


#endif

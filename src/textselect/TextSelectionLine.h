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
  TextSelectionLine(unsigned long lineId,
                    unsigned long fromColumn, 
                    unsigned long toColumn);

  virtual ~TextSelectionLine();

  void addBlock(unsigned long fromColumn, unsigned long toColumn);

  unsigned long getLineId() const;

  long getNumMarkedChars(unsigned long columnId);

  /**
   * Returns the columnId of the next selection start after given
   * column. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(unsigned long columnId);

private:
  std::vector<TextSelectionRange*> m_SelectedBlocks;
  unsigned long m_LineId;
};


#endif

#ifndef TEXT_SELECTION_H
#define TEXT_SELECTION_H

#include <vector>
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
  TextSelection();
  virtual ~TextSelection();
  void add(unsigned long lineId, 
           unsigned long fromColumn, 
           unsigned long toColumn);

  void clear();

  long getNumMarkedChars(unsigned long lineId, unsigned long columnId);

  /**
   * Returns the columnId of the next selection start after given column
   * on given line. If there is no next selection start -1 is returned.
   */
  long getNextSelectionStart(unsigned long lineId, unsigned long columnId);

private:
  std::vector<TextSelectionLine*> m_SelectedLines;

  TextSelectionLine* findSelectionLine(unsigned long lineId);
};

#endif

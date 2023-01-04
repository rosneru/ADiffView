#ifndef TEXT_SELECTION_BLOCK_H
#define TEXT_SELECTION_BLOCK_H

/**
 * A range of selected text on a line.
 *
 * @author Uwe Rosner
 * @date 03/01/2021
 */
class TextSelectionRange
{
public:
  TextSelectionRange(unsigned long fromColumn, 
                     unsigned long toColumn);

  unsigned long getFromColumn() const;
  unsigned long getToColumn() const;

  void setFromColumn(unsigned long fromColumn);
  void setToColumn(unsigned long toColumn);

  long getNumMarkedChars(unsigned long columnId);

private:
  unsigned long m_FromColumn;
  unsigned long m_ToColumn;
};

#endif

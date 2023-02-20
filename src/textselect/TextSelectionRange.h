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
  TextSelectionRange(long fromColumn, long toColumn);

  long getFromColumn() const;
  long getToColumn() const;

  void setFromColumn(long fromColumn);
  void setToColumn(long toColumn);

  long getNumMarkedChars(long columnId);

private:
  long m_FromColumn;
  long m_ToColumn;
};

#endif

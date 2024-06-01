#ifndef DIFF_LINE_H
#define DIFF_LINE_H

#include <stdlib.h>

/**
 * Used to hold parse results of (the part of) a text line. Is needed
 * for rendering tab stops. 
 *
 * When for example 'numRemainingSpaces' is > 0 then the text position
 * is inside a tabulator cell  and 'numRemainingSpaces' spaces must be
 * rendered until the tabulator cell is rendered properly.
 *
 * Or, when 'numRemainingChars' is > 0 then the text position is in
 * normal text and 'numRemainingChars' can be rendered without having to
 * deal with a tabulator.
 *
 * The method getTextPositionInfo(..) can be used to get this info for a
 * given portion of a text line.
 *
 * NOTE: In the result of getTextPositionInfo(..) there is always one of
 * the both fields '0'. If both fields are '0', eol was reached.
 */
typedef struct
{
  size_t numRemainingSpaces;
  size_t numRemainingChars;
  size_t srcTextColumn;
} TextPositionInfo;

/**
 * Represents a line in a Diff which contains of Text and a LineState.
 *
 * It originates from Stephane Rodriguez open diff implementation:
 * https://www.codeproject.com/Articles/3666/Diff-tool
 *
 * @author Uwe Rosner
 * @date 28/09/2018
 */
class DiffLine
{
public:
  enum LineState
  {
    Normal = 0,
    Changed,
    Added,
    Deleted,
    Undefined
  };

  /**
   * Creates a new DiffLine with the given text.
   *
   * When `doSkipLeadingSpaces` is set to true, the whole `pText` is
   * applied but the the spaces / tabs until the first non-whitespace
   * character are skipped for token generation.
   *
   * IMPORTANT: This constructor sets the `IsLinked` field to false so
   * that this `DiffLine` is considered as 'owner' of the given text and
   * line number.
   */
  DiffLine(const char* pText, bool doSkipLeadingSpaces = false);

  /**
   * Creates a new DiffLine with given text and state.
   *
   * IMPORTANT: This constructor sets the `IsLinked` field to true so
   * that another DiffLine is considered as owner of the given text and
   * is responsible to free it after use.
   */
  DiffLine(const char* pText,
           LineState state,
           const char* pLineNumberText);


  virtual ~DiffLine();

  /**
   * Returns the text of the diff line
   */
  const char* getText() const;


  /**
   * Returns the line length in number of chars.
   */
  unsigned long getNumChars() const;

  /**
   * Return the state of the this line
   */
  LineState getState() const;

  /**
   * Set the state of this line.
   */
  void setState(LineState state);

  /**
   * Return the LineNumber as a digit-formatted text.
   *
   * E.g. when highest line number is three-digits long, the returned
   * value will be something like "  1", " 82" or "123".
   *
   * Returns NULL if collecting the line numbers wasn't enabled in the
   * corresponding src DiffFile before PreProcess().
   */
  const char* getLineNumText() const;

  /**
   *  Set the digit-formatted number for this line.
   */
  void setLineNumText(const char* pLineNum);

  /**
   * Returns the token which has been calculated as a numerical
   * representation of this lines text context
   */
  unsigned long getToken() const;

  /**
   * Fills the given TextPositionInfo with the information if on desired
   * resultingTextColumn is normal text to render or if there are some
   * (white)spaces to render to fulfill a tabulator cell width.
   */
  void getTextPositionInfo(TextPositionInfo* pInfo,
                           unsigned long resultingTextColumn, 
                           unsigned long tabSize) const;

  /**
   * Returns the column position on which the given `documentColumn`
   * would be rendered, respecting all previous TABulator chars.
   */
  unsigned long getRenderColumn(unsigned long documentColumn,
                                unsigned long tabSize) const;

  /**
   * Returns the column position of given `renderColumn` in the original document,
   * respecting all previous TABulator chars.
   */
  unsigned long getDocumentColumn(unsigned long requestedRenderColumn,
                                  unsigned long tabSize) const;
protected:
  const char* m_Text;
  const unsigned long m_TextLength;
  LineState m_State;
  const char* m_pLineNumberText;
  unsigned long m_Token;
  bool m_bIsLinked;
};

#endif

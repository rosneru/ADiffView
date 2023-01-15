#ifndef DIFF_WINDOW_TEXT_AREA_H
#define DIFF_WINDOW_TEXT_AREA_H

#include <exec/types.h>
#include <graphics/text.h>
#include <intuition/intuition.h>

#include "DiffOutputFileBase.h"
#include "DiffLine.h"
#include "DiffWindowRastports.h"
#include "Rect.h"
#include "SelectableDiffFile.h"

#include <vector>


/**
 * A text area which can display DiffOutputFiles. These files are the
 * result of a text compare operation and contain informations about
 * inserted, deleted and changed lines. This text area can display these
 * documents with or without line numbers. The background color of the
 * rendered text lines is set to reflect normal, unchanged text or one
 * the states mentioned above.
 * 
 * @author Uwe Rosner
 * @date 12/08/2020
 */
class DiffWindowTextArea : public Rect
{
public:
  DiffWindowTextArea(const DiffOutputFileBase& diffFile, 
                     DiffWindowRastports*& pRPorts,
                     TextFont* pTextFont,
                     bool lineNumbersEnabled,
                     ULONG maxNumChars,
                     ULONG tabSize,
                     ULONG WBorLeft,
                     ULONG WBorTop);

  virtual ~DiffWindowTextArea();

  ULONG getX() const;
  ULONG getY() const;

  bool isScrolledToTop() const;
  bool isScrolledToBottom() const;
  bool isLineVisible(ULONG lineId) const;

  ULONG getMaxVisibleChars() const;
  ULONG getMaxVisibleLines() const;

  /**
   * Set the dimensions of this text area.
   */
  void setSize(ULONG width, ULONG height);

  /**
   * Set the tabulator size
   */
  void setTabulatorSize(ULONG tabulatorSize);

  /**
   * Scroll the text up or down to diplay the given (by rowId) line on
   * first position top.
   */
  void scrollTopToRow(ULONG rowId);

  /**
   * Scroll the text left or right to diplay the given (by column) line
   * on first position left.
   */
  void scrollLeftToColumn(ULONG columId);

  /**
   * Scroll the text left by numChars chars and fills the gap at right
   * with the following chars
   *
   * @returns Number of lines scrolled. Can be smaller than expected
   * when last char of longest line is displayed.
   */
  ULONG scrollLeft(ULONG numChars);

  /**
   * Scroll the text right by numChars chars and fills the gap at left
   * with the previous chars
   *
   * @returns Number of lines scrolled. Can be smaller than expected
   * when first char of text reached.
   */
  ULONG scrollRight(ULONG numChars);

  /**
   * Scroll the text up by numLines lines and fills the gap at bottom
   * with the next lines.
   *
   * @returns Number of lines scrolled. Can be smaller than expected
   * when end of text reached.
   */
  ULONG scrollUp(ULONG numLines);

  /**
   * Scroll the text down by numLines lines and fills the gap at top
   * with the previous lines
   *
   * @returns Number of lines scrolled. Can be smaller than expected
   * when start of text reached.
   */
  ULONG scrollDown(ULONG numLines);

  /**
   * Clear the area except the line numbers area
   */
  void clearText();

  /**
   * Clear the area with the line numbers area
   */
  void clearTextAndLineNumbers();

  /**
   * Print the diff file content in the text area at given position. The
   * content is scrolled until the desired posion is visible.
   */
  void printPageAt(ULONG left, ULONG top);

  /**
   * Print the diff file content in the text area.
   *
   * @param dontPrintLineNumbers Per default the line numbers are
   * printed. When this optional parameter is set to true they are not
   * printed.
   */
  void renderPage(bool dontPrintLineNumbers = false);

  void renderIndexedLine(ULONG lineId);

  void renderSelectionUpdatedLines();


  /**
   * Adds a text selection to the DiffFile which is displayed in this 
   * text area.
   * 
   * @param lineId Id of the line the selection is added
   * @param fromColumn Id of the column on which the selection starts
   * @param toColumn Id of the column on which the selection ends
   */
  void addSelection(ULONG lineId, ULONG fromColumn, ULONG toColumn);

  void startSelection(WORD mouseX, WORD mouseY);
  void updateSelection(WORD mouseX, WORD mouseY);

  void clearSelection();

  /**
   * Returns true if the given point is inside the text area.
   */
  bool isPointInTextArea(unsigned long pointX, unsigned long pointY) const;

  /**
   * Returns true if the given point is inside the text area
   */
  bool isPointInSelection(unsigned long pointX, unsigned long pointY);


private:
  SelectableDiffFile m_DiffFile;
  DiffWindowRastports*& m_pRPorts;
  bool m_AreLineNumbersEnabled;
  ULONG m_LongestLineChars; ///> Number of chars of the longest line of DiffFile.

  TextPositionInfo m_PositionInfo;  ///> Destination to calculate current column informations
  ULONG m_TabSize;         ///> Number of spaces of each tabulator
  ULONG m_WBorLeft;
  ULONG m_WBorTop;

  char* m_pLineOfSpaces;    ///> A text containing m_AreaMaxChars spaces, initialized in setSize()

  UWORD m_FontWidth_pix;    ///> Width of the rastport text font
  UWORD m_FontHeight_pix;   ///> Height of the rastport text font
  UWORD m_FontBaseline_pix; ///> Baseline (from top) of the rastport text font


  ULONG m_AreaMaxChars; ///> Max number of chars that fits in at current size
  ULONG m_AreaMaxLines; ///> Max number of lines that fits in at current size

  ULONG m_X;  ///> Position of left text column in display (> 0 when x scroll)
  ULONG m_Y;  ///> Position of top line in display (> 0 when y scroll)

  UWORD m_LineNumsWidth_chars;
  UWORD m_LineNumsWidth_pix;

  Rect m_HScrollRect;     ///> Horizontal scroll region
  Rect m_VScrollRect;     ///> Vertical scroll region

  /**
   * Contains the line id of the text to which the mouse points. 
   * NOTE: @ref calcMouseInTextPosition() must be called before.
   */
  long m_MouseTextColumn;

  /**
   * Contains the column id of the text to which the mouse points. 
   * NOTE: @ref calcMouseInTextPosition() must be called before.
   */
  long m_MouseTextLine;

  /**
   * Print a diff line.
   * @param lineTop TopEdge y-position in pixels for the line
   *
   * @param numCharLimit When set to a value > 0 max. up to (according
   * to scroll m_X) numCharLimit chars are printed to the left of the
   * display area. When < 0, max. up to numCharLimit chars are printed
   * to the right of the display area.
   */
  void renderLine(ULONG lineId, 
                  bool doDisplayLineNumbers, 
                  long lineTop, 
                  long numCharLimit = 0);

  /**
   * Calculate how many chars of given DiffLine must be print 
   * considering the current scroll position m_X, the current width of
   * the text area and the startIndex.
   */
  ULONG calcNumRenderChars(const DiffLine* pDiffLine, int count, int startIndex);

  void calcMouseInTextPosition(WORD mouseX, WORD mouseY);


  /**
   * Returns the appropriate rastport for a DiffLine with given
   * state to be printed in.
   */
  RastPort* diffStateToRastPort(DiffLine::LineState state);
};

#endif

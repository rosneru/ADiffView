#include <math.h>
#include <string.h>

#include "DiffWindowTextAreaMock.h"


void EraseRect(RastPort* rp, SHORT xmin, SHORT ymin, SHORT xmax, SHORT ymax)
{

}


void ScrollRasterBF(RastPort* rp, WORD dx, WORD dy, WORD xmin, WORD ymin, WORD xmax, WORD ymax)
{

}


void Move(RastPort* rp, SHORT x, SHORT y)
{

}

void Text(RastPort* rp, const char* string, WORD length)
{

}

void Text(RastPort* rp, const char* string, WORD length, char* pRenderBuf, const char* pRPortMarker)
{
  strcat(pRenderBuf, pRPortMarker);
  strncat(pRenderBuf, string, length);
}

DiffWindowTextAreaMock::DiffWindowTextAreaMock(const DiffOutputFileBase& diffFile,
                                               bool lineNumbersEnabled,
                                               ULONG maxNumChars,
                                               ULONG tabSize)
  : m_DiffFile(diffFile),
    m_AreLineNumbersEnabled(lineNumbersEnabled),
    m_LongestLineChars(maxNumChars),
    m_TabSize(tabSize),
    m_pLineOfSpaces(NULL),
    m_FontWidth_pix(8),
    m_FontHeight_pix(8),
    m_FontBaseline_pix(2),
    m_AreaMaxChars(0),
    m_AreaMaxLines(0),
    m_X(0),
    m_Y(0),
    m_LineNumsWidth_chars(0),
    m_LineNumsWidth_pix(0)
{
  if (lineNumbersEnabled)
  {
    const DiffLine* pLine = diffFile[0];
    const char* pLineNum = pLine->getLineNumText();

    m_LineNumsWidth_chars = strlen(pLineNum);
    m_LineNumsWidth_pix = m_LineNumsWidth_chars * m_FontWidth_pix;
  }
}


DiffWindowTextAreaMock::~DiffWindowTextAreaMock()
{
  if (m_pLineOfSpaces != NULL)
  {
    delete[] m_pLineOfSpaces;
    m_pLineOfSpaces = NULL;
  }
}


ULONG DiffWindowTextAreaMock::getX() const
{
  return m_X;
}


ULONG DiffWindowTextAreaMock::getY() const
{
  return m_Y;
}


bool DiffWindowTextAreaMock::isScrolledToTop() const
{
  return m_Y < 1;
}


bool DiffWindowTextAreaMock::isScrolledToBottom() const
{
  return (m_Y + m_AreaMaxLines) >= m_DiffFile.getNumLines();
}


bool DiffWindowTextAreaMock::isLineVisible(ULONG lineId) const
{
  return (lineId >= m_Y) && (lineId < (m_Y + m_AreaMaxLines));
}


ULONG DiffWindowTextAreaMock::getMaxVisibleChars() const
{
  return m_AreaMaxChars;
}


ULONG DiffWindowTextAreaMock::getMaxVisibleLines() const
{
  return m_AreaMaxLines;
}


void DiffWindowTextAreaMock::setSize(ULONG width, ULONG height)
{
  m_AreaMaxLines = (height - 4) / m_FontHeight_pix;
  m_AreaMaxChars = (width - 7 - m_LineNumsWidth_pix) / m_FontWidth_pix;

  // Create a full line of spaces, to be used for tab rendering
  if (m_pLineOfSpaces != NULL)
  {
    delete[] m_pLineOfSpaces;
    m_pLineOfSpaces = NULL;
  }

  m_pLineOfSpaces = new char[m_AreaMaxChars + 1];
  memset(m_pLineOfSpaces, ' ', m_AreaMaxChars);
  m_pLineOfSpaces[m_AreaMaxChars] = '\0';

  // Limit height to whole lines
  height = m_AreaMaxLines * m_FontHeight_pix + 3;

  // Set the outer rect of this text area to new width and height
  Rect::setSize(width, height);

  /**
   * NOTE: The TextArea 'is' a Rect by itself but it additionally
   * contains two other rects which are needed for scrolling. The reason
   * two other rects are needed is:
   *
   *   The 'Left edge' values of the scrolling area depend on if the
   *   scroll is done horizontally or vertically. On vertical scroll the
   *   block containing line numbers is scrolled too. On horizontal
   *   scroll, it is not.
   */

  ULONG maxTextWidth_pix = m_AreaMaxChars * m_FontWidth_pix;
  maxTextWidth_pix += m_LineNumsWidth_pix;

  m_HScrollRect.set(getLeft() + m_LineNumsWidth_pix + 2, getTop() + 1,
                    getLeft() + maxTextWidth_pix + 2,
                    getTop() + getHeight() - 3);

  m_VScrollRect.set(getLeft() + 2, getTop() + 1, getLeft() + maxTextWidth_pix + 2,
                    getTop() + getHeight() - 3);
}


void DiffWindowTextAreaMock::addSelection(ULONG lineId, 
                                          ULONG fromColumn, 
                                          ULONG toColumn)
{
  m_DiffFile.addSelection(lineId, fromColumn, toColumn);
}


void DiffWindowTextAreaMock::clearSelection()
{
  m_DiffFile.clearSelection();
}


void DiffWindowTextAreaMock::scrollTopToRow(ULONG rowId)
{
  // Prevent to scroll below the last line
  ULONG yLimit = m_DiffFile.getNumLines() - m_AreaMaxLines;
  if ((yLimit > 0) && (rowId > yLimit))
  {
    rowId = yLimit;
  }

  int delta = rowId - m_Y;
  if (delta == 0)
  {
    return;
  }

  int deltaAbs = abs(delta);

  int deltaLimit = m_AreaMaxLines - 2;
  // int deltaLimit = 10;

  //
  // Scroll small amounts (1/2 text area height) line by line
  //
  if (deltaAbs < deltaLimit)
  {
    if (delta > 0)
    {
      scrollUp(deltaAbs);
    }
    else if (delta < 0)
    {
      scrollDown(deltaAbs);
    }

    return;
  }

  //
  // Scroll bigger amounts by re-painting the whole page at the new
  // y-position
  //
  m_Y = rowId;

  EraseRect(&m_RPWindow, m_VScrollRect.getLeft(), m_VScrollRect.getTop(),
            m_VScrollRect.getRight(), m_VScrollRect.getBottom());

  renderPage();
}


void DiffWindowTextAreaMock::scrollLeftToColumn(ULONG columId)
{
  long delta = columId - m_X;
  if (delta == 0)
  {
    return;
  }

  int deltaAbs = abs(delta);

  // int deltaLimit = m_MaxTextAreaChars / 2;
  int deltaLimit = 10;

  //
  // Scroll small amounts (1/2 text area width) line by line
  //
  if (deltaAbs < deltaLimit)
  {
    if (delta > 0)
    {
      scrollLeft(deltaAbs);
    }
    else if (delta < 0)
    {
      scrollRight(deltaAbs);
    }

    return;
  }

  //
  // Scroll bigger amounts by re-painting the whole page at the new
  // x-position
  //
  m_X = columId;

  // Clear the area except the line numbers area
  EraseRect(&m_RPWindow, m_HScrollRect.getLeft(), m_HScrollRect.getTop(),
            m_HScrollRect.getRight(), m_HScrollRect.getBottom());

  // NOTE Commented out for debugging
  // renderPage(true);
}


ULONG DiffWindowTextAreaMock::scrollLeft(ULONG numChars)
{
  if (numChars < 1)
  {
    // Noting to do
    return 0;
  }

  if (numChars > m_AreaMaxChars)
  {
    numChars = m_AreaMaxChars;
  }

  if (m_LongestLineChars < m_AreaMaxChars)
  {
    // Do not move the scroll area left if all the text fits into
    // the window
    return 0;
  }

  if ((m_X + m_AreaMaxChars) == m_LongestLineChars)
  {
    // Do not move the scroll area left if text already at rightmost
    // position
    return 0;
  }

  if ((m_X + m_AreaMaxChars + numChars) > m_LongestLineChars)
  {
    // Limit the scrolling to only scroll only as many chars as necessary
    numChars = m_LongestLineChars - (m_X + m_AreaMaxChars);
  }

  // Move text area content left by n * the width of one char
  ScrollRasterBF(&m_RPWindow, numChars * m_FontWidth_pix, 0,
                 m_HScrollRect.getLeft(), m_HScrollRect.getTop(),
                 m_HScrollRect.getRight(), m_HScrollRect.getBottom());

  // Fill the gap with the following chars
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;

    // NOTE Commented out for debug
    // renderLine(lineId, false, lineTopEdge, -numChars);
  }

  // NOTE Commented out for debug
  // m_X += numChars;
  return numChars;
}


ULONG DiffWindowTextAreaMock::scrollRight(ULONG numChars)
{
  if (numChars < 1)
  {
    // Nothing to do
    return 0;
  }

  if (m_X < 1)
  {
    // Do not move the text area right if text is already at leftmost position
    return 0;
  }

  if (numChars > m_X)
  {
    // Limit the scrolling to only scroll only as many chars as necessary
    numChars = m_X;
  }

  if (numChars > m_AreaMaxChars)
  {
    numChars = m_AreaMaxChars;
  }

  // Move text area content right by n * the height of one text line
  ScrollRasterBF(&m_RPWindow,
                 -numChars * m_FontWidth_pix, // n * width
                 0, m_HScrollRect.getLeft(), m_HScrollRect.getTop(),
                 m_HScrollRect.getRight(), m_HScrollRect.getBottom());

  // fill the gap with the previous chars
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;

    // NOTE: Commented out for debug
    // renderLine(lineId, false, lineTopEdge, numChars);
  }

  // NOTE: Commented out for debug
  // m_X -= numChars;
  return numChars;
}


ULONG DiffWindowTextAreaMock::scrollUp(ULONG numLines)
{
  if (numLines < 1)
  {
    // Noting to do
    return 0;
  }

  if (m_DiffFile.getNumLines() <= m_AreaMaxLines)
  {
    // All text fits into the window. Don't scroll.
    return 0;
  }

  if (numLines > m_AreaMaxLines)
  {
    numLines = m_AreaMaxLines;
  }

  if ((m_Y + m_AreaMaxLines + numLines) > m_DiffFile.getNumLines())
  {
    // Limit the scrolling to not exceed number of lines in file
    numLines = m_DiffFile.getNumLines() - m_Y - m_AreaMaxLines;
  }

  // Move each text area upward by n * the height of one text line
  ScrollRasterBF(&m_RPWindow, 0, numLines * m_FontHeight_pix,
                 m_VScrollRect.getLeft(), m_VScrollRect.getTop(),
                 m_VScrollRect.getRight(), m_VScrollRect.getBottom());

  // Fill the now empty lines at the bottom with the next lines
  for (ULONG i = 0; i < numLines; i++)
  {
    ULONG lineId = m_Y + m_AreaMaxLines + i;
    WORD lineTopEdge = (m_AreaMaxLines - numLines + i) * m_FontHeight_pix;
    renderLine(lineId, true, lineTopEdge, 0, NULL);
  }

  m_Y += numLines;

  return numLines;
}


ULONG DiffWindowTextAreaMock::scrollDown(ULONG numLines)
{
  if (numLines < 1 || m_Y < 1)
  {
    // Nothing to do
    return 0;
  }

  if (numLines > m_Y)
  {
    // Limit to not exceed / go below lineId 0
    numLines = m_Y;
  }

  if (numLines > m_AreaMaxLines)
  {
    // Limit to scroll maximum one page.
    // TODO Why is this needed here?
    numLines = m_AreaMaxLines;
  }


  // Move each text area downward by n * the height of one text line
  ScrollRasterBF(&m_RPWindow, 0,
                 -numLines * m_FontHeight_pix, // n * height
                 m_VScrollRect.getLeft(), m_VScrollRect.getTop(),
                 m_VScrollRect.getRight(), m_VScrollRect.getBottom());

  // Fill the now empty lines at the top with the previous text lines
  for (ULONG i = 0; i < numLines; i++)
  {
    ULONG lineId = m_Y - numLines + i;
    WORD lineTopEdge = i * m_FontHeight_pix;
    renderLine(lineId, true, lineTopEdge, 0, NULL);
  }

  m_Y -= numLines;

  return numLines;
}


void DiffWindowTextAreaMock::printPageAt(ULONG left, ULONG top)
{
  m_X = left;
  m_Y = top;

  renderPage();
}

void DiffWindowTextAreaMock::renderPage(bool dontPrintLineNumbers)
{
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
    renderLine(lineId, !dontPrintLineNumbers, lineTopEdge, 0, NULL);
  }
}


void DiffWindowTextAreaMock::renderIndexedLine(ULONG lineId)
{
  WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
  renderLine(lineId, true, lineTopEdge, 0, NULL);
}

ULONG DiffWindowTextAreaMock::renderLine(ULONG lineId,
                                    bool doDisplayLineNumbers,
                                    long lineTop,
                                    long numCharLimit,
                                    char* pBuf)
{
  pBuf[0] = '\0'; // For unit tests
  const char* pRPortMarkerDiffState = "{NRMCOL}";
  const char* pRPortMarkerSelection = "{SELCOL}";
  const char* pRPortMarker;


  ULONG numTxPosInfoCalls = 0;
  const DiffLine* pLine = m_DiffFile[lineId];
  if (pLine == NULL)
  {
    return numTxPosInfoCalls;
  }

  /**
   * Render the line numbers
   */

  if (doDisplayLineNumbers && m_AreLineNumbersEnabled)
  {
    // Move rastport cursor to start of line numbers block
    Move(&m_RPLineNumText, m_VScrollRect.getLeft(),
         getTop() + lineTop + m_FontBaseline_pix + 1);

    // Get the text or set to empty spaces when there is none
    const char* pLineNum = pLine->getLineNumText();

    // Print line's original line number
    Text(&m_RPLineNumText, pLineNum, m_LineNumsWidth_chars);
  }

  /**
   * Render the line
   */

  ULONG srcTextStartColumn;
  ULONG currentDisplayColumn;
  ULONG maxRemainingCharsToRender;
  if (numCharLimit < 0)
  {
    // Only render 'numCharLimit' chars  at the right of the lines
    // visible area
    maxRemainingCharsToRender = -numCharLimit;

    srcTextStartColumn = m_AreaMaxChars + m_X;
    currentDisplayColumn = m_AreaMaxChars - maxRemainingCharsToRender;
  }
  else if (numCharLimit > 0)
  {
    // Only render 'numCharLimit' chars at the left of the lines visible
    // area
    maxRemainingCharsToRender = numCharLimit;
    srcTextStartColumn = m_X - maxRemainingCharsToRender;
    currentDisplayColumn = 0;
  }
  else
  {
    // Default, no scrolling
    maxRemainingCharsToRender = m_AreaMaxChars;
    srcTextStartColumn = m_X;
    currentDisplayColumn = 0;
  }

  // Get the text position info of resulting text column. This also
  // calculates the srcTextColumn which is needed next.
  ULONG resultingTextColumn = srcTextStartColumn;

  pLine->getTextPositionInfo(&m_PositionInfo, resultingTextColumn, m_TabSize);
  numTxPosInfoCalls++;
  
  while (maxRemainingCharsToRender > 0 &&
          (m_PositionInfo.numRemainingChars > 0 ||
          m_PositionInfo.numRemainingSpaces > 0))
  {
    // Get the RastPort (with the appropriate color) to render the
    // next block of marked / not marked text in the line.
    ULONG numCharsInBlock;
    RastPort* pRPort;
    if ((numCharsInBlock = m_DiffFile.getNumNormalChars(
            lineId, m_PositionInfo.srcTextColumn)) > 0)
    {
      // The RastPort of the normal, not marked text depends on the
      // diff state of the line.
      pRPort = diffStateToRastPort(pLine->getState());
      pRPortMarker = pRPortMarkerDiffState;
    }
    else if ((numCharsInBlock = m_DiffFile.getNumMarkedChars(
                lineId, m_PositionInfo.srcTextColumn)) > 0)
    {
      pRPort = &m_RPTextSelected;
      pRPortMarker = pRPortMarkerSelection;
    }
    else
    {
      // Line finished
      return numTxPosInfoCalls;
    }

    ULONG numNextCharsToRender;
    ULONG numSrcCharsIncreased;
    const char* pTextToPrint;
    bool hasMarkedNormalBlockLimitReached = false;
    bool hasNumCharsBeenLimited = false;

    if (m_PositionInfo.numRemainingChars > 0)
    {
      // Set the text print pointer to te next char to be rendered
      numNextCharsToRender = m_PositionInfo.numRemainingChars;
      pTextToPrint = pLine->getText() + m_PositionInfo.srcTextColumn;

      // Check if block limit {marked test|normal text} reached
      // NOTE: Only checked and limited when chars are printed. When
      //       remainingSpaces are printed they are not limited 
      //       because they all belong to the same tabulator-block.
      if (numNextCharsToRender > numCharsInBlock)
      {
        numNextCharsToRender = numCharsInBlock;
        hasMarkedNormalBlockLimitReached = true;
      }

      numSrcCharsIncreased = numNextCharsToRender;
    }
    else
    {
      // Here m_PositionInfo.numRemainingSpaces is greater than 0.
      
      // Set the text print pointer to the line of spaces
      pTextToPrint = m_pLineOfSpaces;

      // And apply the number of spaces to print. (They fill the 
      // remaining space of a tabulator column)
      numNextCharsToRender = m_PositionInfo.numRemainingSpaces;

      numSrcCharsIncreased = 1;
    }

    // Check if max remaining chars limit reached
    if (numNextCharsToRender > maxRemainingCharsToRender)
    {
      numNextCharsToRender = maxRemainingCharsToRender;
      hasNumCharsBeenLimited = true;
    }

    /**
     * Move rastport cursor to render position and render the text
     */
    Move(pRPort, m_HScrollRect.getLeft() + m_FontWidth_pix * currentDisplayColumn,
         getTop() + lineTop + m_FontBaseline_pix + 1);

    Text(pRPort, pTextToPrint, numNextCharsToRender, pBuf, pRPortMarker);

    /*
      * After-rendering checks
      */
    if (hasNumCharsBeenLimited)
    {
      // Line finished
      return numTxPosInfoCalls;
    }
    
    if (hasMarkedNormalBlockLimitReached)
    {
      maxRemainingCharsToRender -= numNextCharsToRender;
      currentDisplayColumn += numNextCharsToRender;
      resultingTextColumn += numNextCharsToRender;

      if(maxRemainingCharsToRender < 1)
      {
        // Line finished
        return numTxPosInfoCalls;
      }

      pLine->getTextPositionInfo(&m_PositionInfo, resultingTextColumn, m_TabSize);
      numTxPosInfoCalls++;

      // Block {normal text|marked text} finished
      continue;
    }

    if(m_PositionInfo.srcTextColumn + numSrcCharsIncreased >= pLine->getNumChars())
    {
      // Line finished
      return numTxPosInfoCalls;
    }

    maxRemainingCharsToRender -= numNextCharsToRender;
    currentDisplayColumn += numNextCharsToRender;
    resultingTextColumn += numNextCharsToRender;

    pLine->getTextPositionInfo(&m_PositionInfo, resultingTextColumn, m_TabSize);
    numTxPosInfoCalls++;
  }


  return numTxPosInfoCalls;
}


ULONG DiffWindowTextAreaMock::calcNumPrintChars(const DiffLine* pDiffLine, 
                                            int count, 
                                            int startIndex)
{
  ULONG numCharsToPrint = 0;

  if (count > 0)
  {
    numCharsToPrint = count;
  }
  else
  {
    // Determine how many characters would be print theoretically
    numCharsToPrint = pDiffLine->getNumChars() - m_X;
  }

  if (numCharsToPrint > m_AreaMaxChars)
  {
    // Limit the number of printed chars to fit into the text area
    numCharsToPrint = m_AreaMaxChars;
  }

  if ((startIndex > -1) && (numCharsToPrint + startIndex > pDiffLine->getNumChars()))
  {
    // Limit the number of printed chars to line length
    numCharsToPrint = pDiffLine->getNumChars() - startIndex;
  }

  return numCharsToPrint;
}


RastPort* DiffWindowTextAreaMock::diffStateToRastPort(DiffLine::LineState state)
{
  if (state == DiffLine::Added)
  {
    return &m_RPGreen;
  }
  else if (state == DiffLine::Changed)
  {
    return &m_RPYellow;
  }
  else if (state == DiffLine::Deleted)
  {
    return &m_RPRed;
  }
  else
  {
    return &m_RPDefault;
  }
}

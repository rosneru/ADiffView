#ifdef __clang__
  #include <clib/graphics_protos.h>
#else
  #include <proto/graphics.h>
#endif

#include <math.h>
#include <string.h>

#include "DiffWindowTextArea.h"


DiffWindowTextArea::DiffWindowTextArea(const DiffOutputFileBase& diffFile,
                                       DiffWindowRastports*& pRPorts,
                                       TextFont* pTextFont,
                                       bool lineNumbersEnabled,
                                       ULONG maxNumChars,
                                       ULONG tabSize,
                                       ULONG WBorLeft,
                                       ULONG WBorTop)
  : m_DiffFile(diffFile),
    m_pRPorts(pRPorts),
    m_AreLineNumbersEnabled(lineNumbersEnabled),
    m_LongestLineChars(maxNumChars),
    m_TabSize(tabSize),
    m_WBorLeft(WBorLeft),
    m_WBorTop(WBorTop),
    m_pLineOfSpaces(NULL),
    m_FontWidth_pix(pTextFont->tf_XSize),
    m_FontHeight_pix(pTextFont->tf_YSize),
    m_FontBaseline_pix(pTextFont->tf_Baseline),
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


DiffWindowTextArea::~DiffWindowTextArea()
{
  if (m_pLineOfSpaces != NULL)
  {
    delete[] m_pLineOfSpaces;
    m_pLineOfSpaces = NULL;
  }
}


ULONG DiffWindowTextArea::getX() const
{
  return m_X;
}


ULONG DiffWindowTextArea::getY() const
{
  return m_Y;
}


bool DiffWindowTextArea::isScrolledToTop() const
{
  return m_Y < 1;
}


bool DiffWindowTextArea::isScrolledToBottom() const
{
  return (m_Y + m_AreaMaxLines) >= m_DiffFile.getNumLines();
}


bool DiffWindowTextArea::isLineVisible(ULONG lineId) const
{
  return (lineId >= m_Y) && (lineId < (m_Y + m_AreaMaxLines));
}


ULONG DiffWindowTextArea::getMaxVisibleChars() const
{
  return m_AreaMaxChars;
}


ULONG DiffWindowTextArea::getMaxVisibleLines() const
{
  return m_AreaMaxLines;
}


void DiffWindowTextArea::setSize(ULONG width, ULONG height)
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


void DiffWindowTextArea::setTabulatorSize(ULONG tabulatorSize)
{
  m_TabSize = tabulatorSize;
}


void DiffWindowTextArea::addSearchResultSelection(ULONG lineId, 
                                      ULONG fromColumn, 
                                      ULONG toColumn)
{
  m_DiffFile.addSearchResultSelectionBlock(lineId, fromColumn, toColumn);
}

void DiffWindowTextArea::calcMouseInTextPosition(WORD mouseX, WORD mouseY)
{
    m_MouseTextColumn = ((long)mouseX - (long)m_HScrollRect.getLeft()) / m_FontWidth_pix;
    m_MouseTextColumn += m_X;
    if(m_MouseTextColumn < 0)
    {
      m_MouseTextColumn = 0;
    }

    m_MouseTextLine = ((long)mouseY - (long)m_HScrollRect.getTop()) / m_FontHeight_pix;
    if(m_MouseTextLine < 0)
    {
      m_MouseTextLine = 0;
    }
    m_MouseTextLine += m_Y;
    if(m_MouseTextLine < 0)
    {
      m_MouseTextLine = 0;
    }
}

void DiffWindowTextArea::startDynamicSelection(WORD mouseX, WORD mouseY)
{
  calcMouseInTextPosition(mouseX, mouseY);

  const DiffLine* pLine = m_DiffFile[m_MouseTextLine];
  if (pLine == NULL)
  {
    return;
  }

  ULONG documentColumn = pLine->getDocumentColumn(m_MouseTextColumn, m_TabSize);

  m_DiffFile.startDynamicSelection(m_MouseTextLine, documentColumn);
  renderSelectionUpdatedLines();
}


DiffWindowTextArea::ScrollRequest DiffWindowTextArea::updateDynamicSelection(
  WORD mouseX, WORD mouseY)
{
  DiffWindowTextArea::ScrollRequest result = SR_NONE;
  calcMouseInTextPosition(mouseX, mouseY);

  const DiffLine* pLine = m_DiffFile[m_MouseTextLine];
  if (pLine == NULL)
  {
    return SR_NONE;
  }

  ULONG documentColumn = pLine->getDocumentColumn(m_MouseTextColumn, m_TabSize);

  long bottomLine = m_Y + m_AreaMaxLines - 1;
  long rightmostColumn = m_X + m_AreaMaxChars;

  if(m_MouseTextLine > bottomLine)
  {
    result = SR_UP;
  }
  else if(((ULONG)m_MouseTextLine <= m_Y) && m_Y > 0)
  {
    result = SR_DOWN;
  }
  else if(m_MouseTextColumn > rightmostColumn 
       && m_X + m_AreaMaxChars < m_DiffFile[m_MouseTextLine]->getNumChars())
  {
    result = SR_LEFT;
  }
  else if((ULONG)m_MouseTextColumn < m_X)
  {
    result = SR_RIGHT;
  }

  m_DiffFile.updateDynamicSelection(m_MouseTextLine, documentColumn);
  renderSelectionUpdatedLines();

  return result;
}

void DiffWindowTextArea::clearSearchResultSelection()
{
  m_DiffFile.clearSearchResultSelection();
}


void DiffWindowTextArea::clearDynamicSelection()
{
  m_DiffFile.clearDynamicSelection();
}

void DiffWindowTextArea::activateSearchResultSelection()
{
  m_DiffFile.activateSearchResultSelection(m_Y, m_Y + m_AreaMaxLines);
}

void DiffWindowTextArea::activateDynamicSelection()
{
  m_DiffFile.activateDynamicSelection(m_Y, m_Y + m_AreaMaxLines);
}

bool DiffWindowTextArea::isPointInTextArea(unsigned long pointX,
                                           unsigned long pointY) const
{
  return m_HScrollRect.isPointInside(pointX, pointY);
}

bool DiffWindowTextArea::isPointInSelection(unsigned long pointX,
                                            unsigned long pointY)
{
  calcMouseInTextPosition(pointX, pointY);
  return m_DiffFile.isPointInSelection(m_MouseTextLine, m_MouseTextColumn);
}

const SelectableDiffFile* DiffWindowTextArea::getSelectionDocument() const
{
  return &m_DiffFile;
}

void DiffWindowTextArea::scrollTopToRow(ULONG rowId)
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

  clearTextAndLineNumbers();
  renderPage();
}


void DiffWindowTextArea::scrollLeftToColumn(ULONG columId)
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

  clearText();
  renderPage(true);
}


ULONG DiffWindowTextArea::scrollLeft(ULONG numChars)
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
  ScrollRasterBF(m_pRPorts->Window(),
                 numChars * m_FontWidth_pix,
                 0,
                 m_HScrollRect.getLeft(),
                 m_HScrollRect.getTop(),
                 m_HScrollRect.getRight() - 1,
                 m_HScrollRect.getBottom());

  // Fill the gap with the following chars
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
    renderLine(lineId, false, lineTopEdge, -numChars);
  }

  m_X += numChars;
  return numChars;
}


ULONG DiffWindowTextArea::scrollRight(ULONG numChars)
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
  ScrollRasterBF(m_pRPorts->Window(),
                 -numChars * m_FontWidth_pix, // n * width
                 0,
                 m_HScrollRect.getLeft(),
                 m_HScrollRect.getTop(),
                 m_HScrollRect.getRight() - 1,
                 m_HScrollRect.getBottom());

  // fill the gap with the previous chars
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
    renderLine(lineId, false, lineTopEdge, numChars);
  }

  m_X -= numChars;
  return numChars;
}


ULONG DiffWindowTextArea::scrollUp(ULONG numLines)
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
  ScrollRasterBF(m_pRPorts->Window(),
                 0,
                 numLines * m_FontHeight_pix,
                 m_VScrollRect.getLeft(),
                 m_VScrollRect.getTop(),
                 m_VScrollRect.getRight(),
                 m_VScrollRect.getBottom());

  // Fill the now empty lines at the bottom with the next lines
  for (ULONG i = 0; i < numLines; i++)
  {
    ULONG lineId = m_Y + m_AreaMaxLines + i;
    WORD lineTopEdge = (m_AreaMaxLines - numLines + i) * m_FontHeight_pix;
    renderLine(lineId, true, lineTopEdge);
  }

  m_Y += numLines;

  return numLines;
}


ULONG DiffWindowTextArea::scrollDown(ULONG numLines)
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
  ScrollRasterBF(m_pRPorts->Window(),
                 0,
                 -numLines * m_FontHeight_pix, // n * height
                 m_VScrollRect.getLeft(),
                 m_VScrollRect.getTop(),
                 m_VScrollRect.getRight(),
                 m_VScrollRect.getBottom());

  // Fill the now empty lines at the top with the previous text lines
  for (ULONG i = 0; i < numLines; i++)
  {
    ULONG lineId = m_Y - numLines + i;
    WORD lineTopEdge = i * m_FontHeight_pix;
    renderLine(lineId, true, lineTopEdge);
  }

  m_Y -= numLines;

  return numLines;
}


void DiffWindowTextArea::clearText()
{
  EraseRect(m_pRPorts->Window(), m_HScrollRect.getLeft(), m_HScrollRect.getTop(),
            m_HScrollRect.getRight(), m_HScrollRect.getBottom());
}


void DiffWindowTextArea::clearTextAndLineNumbers()
{
  EraseRect(m_pRPorts->Window(), m_VScrollRect.getLeft(), m_VScrollRect.getTop(),
            m_VScrollRect.getRight(), m_VScrollRect.getBottom());
}


void DiffWindowTextArea::renderPageAt(ULONG left, ULONG top)
{
  m_X = left;
  m_Y = top;

  renderPage();
}

void DiffWindowTextArea::renderPage(bool dontPrintLineNumbers)
{
  for (ULONG lineId = m_Y; lineId < m_Y + m_AreaMaxLines; lineId++)
  {
    WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
    renderLine(lineId, !dontPrintLineNumbers, lineTopEdge);
  }
}

void DiffWindowTextArea::renderIndexedLine(ULONG lineId)
{
  WORD lineTopEdge = (lineId - m_Y) * m_FontHeight_pix;
  renderLine(lineId, true, lineTopEdge);
}

void DiffWindowTextArea::renderSelectionUpdatedLines()
{
  // Render all lines whose selection has changed
  ULONG bottomLine = m_Y + m_AreaMaxLines;
  const std::list<long>& updatedLines = m_DiffFile.getUpdatedLineIds();
  std::list<long>::const_iterator it;
  for(it = updatedLines.begin(); it != updatedLines.end(); it++)
  {
    ULONG lineId = *it;
    if(lineId < m_Y)
    {
      continue;
    }

    if(lineId >= bottomLine)
    {
      break;
    }

    renderIndexedLine(*it);
  }

  m_DiffFile.clearUpdatedLineIds();
}

void DiffWindowTextArea::renderLine(ULONG lineId,
                                    bool doDisplayLineNumbers,
                                    long lineTop,
                                    long numCharLimit)
{
  const DiffLine* pLine = m_DiffFile[lineId];
  if (pLine == NULL)
  {
    return;
  }

  /**
   * Render the line numbers
   */

  if (doDisplayLineNumbers && m_AreLineNumbersEnabled)
  {
    // Move rastport cursor to start of line numbers block
    Move(m_pRPorts->getLineNumText(), m_VScrollRect.getLeft(),
         getTop() + lineTop + m_FontBaseline_pix + 1);

    // Get the text or set to empty spaces when there is none
    const char* pLineNum = pLine->getLineNumText();

    // Print line's original line number
    Text(m_pRPorts->getLineNumText(), pLineNum, m_LineNumsWidth_chars);
  }

  /**
   * Render the line
   */

  ULONG srcTextStartColumn;
  ULONG currentDisplayColumn;
  ULONG maxRemainingCharsToRender;
  if (numCharLimit < 0)
  {
    // Only render 'numCharLimit' chars  at the right of the text area
    // (Done when rendering after scrolling the content to left)
    maxRemainingCharsToRender = -numCharLimit;

    srcTextStartColumn = m_AreaMaxChars + m_X;
    currentDisplayColumn = m_AreaMaxChars - maxRemainingCharsToRender;
  }
  else if (numCharLimit > 0)
  {
    // Only render 'numCharLimit' chars  at the left of the text area
    // (Done when rendering after scrolling the content to right)
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

  while (maxRemainingCharsToRender > 0
  && (m_PositionInfo.numRemainingChars > 0 || m_PositionInfo.numRemainingSpaces > 0))
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
    }
    else if ((numCharsInBlock = m_DiffFile.getNumMarkedChars(
                lineId, m_PositionInfo.srcTextColumn)) > 0)
    {
      pRPort = m_pRPorts->TextSelected();
    }
    else
    {
      // Line finished
      return;
    }

    LONG numNextCharsToRender;
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
    Move(pRPort,
         m_HScrollRect.getLeft() + m_FontWidth_pix * currentDisplayColumn,
         getTop() + lineTop + m_FontBaseline_pix + 1);

    Text(pRPort, pTextToPrint, numNextCharsToRender);

    /**
     * After-rendering checks
     */
    if (hasNumCharsBeenLimited)
    {
      // Line finished
      return;
    }
    
    if (hasMarkedNormalBlockLimitReached)
    {
      maxRemainingCharsToRender -= numNextCharsToRender;
      currentDisplayColumn += numNextCharsToRender;
      resultingTextColumn += numNextCharsToRender;

      if(maxRemainingCharsToRender < 1)
      {
        // Line finished
        return;
      }

      // Block {normal text|marked text} finished; continue on top of loop
      pLine->getTextPositionInfo(&m_PositionInfo, resultingTextColumn, m_TabSize);
      continue;
    }

    if(m_PositionInfo.srcTextColumn + numSrcCharsIncreased >= pLine->getNumChars())
    {
      // Line finished
      return;
    }

    maxRemainingCharsToRender -= numNextCharsToRender;
    currentDisplayColumn += numNextCharsToRender;
    resultingTextColumn += numNextCharsToRender;

    pLine->getTextPositionInfo(&m_PositionInfo, resultingTextColumn, m_TabSize);
  }
}


ULONG DiffWindowTextArea::calcNumRenderChars(const DiffLine* pDiffLine, 
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


RastPort* DiffWindowTextArea::diffStateToRastPort(DiffLine::LineState state)
{
  if (state == DiffLine::Added)
  {
    return m_pRPorts->TextGreenBG();
  }
  else if (state == DiffLine::Changed)
  {
    return m_pRPorts->TextYellowBG();
  }
  else if (state == DiffLine::Deleted)
  {
    return m_pRPorts->TextRedBG();
  }
  else
  {
    return m_pRPorts->TextDefault();
  }
}

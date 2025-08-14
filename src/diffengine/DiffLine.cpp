#include <string.h>
#include "DiffLine.h"

DiffLine::DiffLine(const char* pText, bool doSkipLeadingSpaces)
  : m_Text(pText),
    m_TextLength(strlen(pText)),
    m_State(Normal),
    m_pLineNumberText(NULL),
    m_Token(0),
    m_bIsLinked(false)
{
  const char* pBuf = pText;
  size_t i = 0;
  if(doSkipLeadingSpaces)
  {
    while(*pBuf == ' ' || *pBuf == '\t')
    {
      i++;
      pBuf++;
    }
  }

  for(i = i; i < m_TextLength; i++)
  {
    m_Token += 2 * m_Token + *(pBuf++); // (George V. Reilly hint)
  }
}

DiffLine::DiffLine(const char* pText,
                   LineState state,
                   const char* pLineNumberText)
  : m_Text(pText),
    m_TextLength(strlen(pText)),
    m_State(state),
    m_pLineNumberText(pLineNumberText),
    m_Token(0),
    m_bIsLinked(true)
{
}

DiffLine::~DiffLine()
{
}

const char* DiffLine::getText() const
{
  return m_Text;
}

unsigned long DiffLine::getNumChars() const
{
  return m_TextLength;
}

DiffLine::LineState DiffLine::getState() const
{
  return m_State;
}

const char*DiffLine::getLineNumText() const
{
  return m_pLineNumberText;
}

void DiffLine::setLineNumText(const char* pFormattedLineNum)
{
  m_pLineNumberText = pFormattedLineNum;
}

void DiffLine::setState(DiffLine::LineState state)
{
  m_State = state;
}

unsigned long DiffLine::getToken() const
{
  return m_Token;
}



void DiffLine::getTextPositionInfo(TextPositionInfo* pInfo,
                                   unsigned long resultingTextColumn,
                                   unsigned long tabSize) const
{
  unsigned long accumulatedColumn = 0;

  for (pInfo->srcTextColumn = 0; pInfo->srcTextColumn < m_TextLength; ++pInfo->srcTextColumn)
  {
    char c = m_Text[pInfo->srcTextColumn];
    unsigned long nextColumn;

    if (c == '\t')
    {
      nextColumn = accumulatedColumn + (tabSize - (accumulatedColumn % tabSize));
    }
    else
    {
      nextColumn = accumulatedColumn + 1;
    }

    if (accumulatedColumn > resultingTextColumn)
    {
      // Wir sind mitten in einem Tabulatorblock
      pInfo->srcTextColumn--;
      pInfo->numRemainingChars = 0;
      pInfo->numRemainingSpaces = tabSize - (resultingTextColumn % tabSize);
      return;
    }
    else if (accumulatedColumn == resultingTextColumn)
    {
      if (c == '\t')
      {
        // Direkt am Anfang eines Tabulatorblocks
        pInfo->numRemainingChars = 0;
        pInfo->numRemainingSpaces = tabSize - (accumulatedColumn % tabSize);
      }
      else
      {
        // Normales Zeichen, zähle bis zum nächsten Tabulator
        size_t i = pInfo->srcTextColumn;
        while (i < m_TextLength && m_Text[i] != '\t')
        {
          ++i;
        }
        pInfo->numRemainingChars = i - pInfo->srcTextColumn;
        pInfo->numRemainingSpaces = 0;
      }
      return;
    }

    accumulatedColumn = nextColumn;
  }

  // EOL erreicht
  pInfo->numRemainingChars = 0;
  pInfo->numRemainingSpaces = 0;
}

unsigned long DiffLine::getRenderColumn(unsigned long documentColumn,
                                        unsigned long tabSize) const
{
  if (documentColumn > m_TextLength)
  {
    return 0;
  }

  unsigned long renderColumn = 0;

  for (unsigned long i = 0; i < documentColumn && i < m_TextLength; ++i)
  {
    if (m_Text[i] == '\t')
    {
      renderColumn += tabSize - (renderColumn % tabSize);
    }
    else
    {
      ++renderColumn;
    }
  }

  return renderColumn;
}

unsigned long DiffLine::getDocumentColumn(unsigned long requestedRenderColumn,
                                          unsigned long tabSize) const
{
  unsigned long actualRenderColumn = 0;
  bool isTabBorder = false;
  for(unsigned long documentColumn = 0; documentColumn < m_TextLength; documentColumn++)
  {
    if(m_Text[documentColumn] == '\t')
    {
      unsigned long tabRemainingChars = (tabSize - (documentColumn % tabSize));
      while (tabRemainingChars-- > 0)
      {
        if(actualRenderColumn == requestedRenderColumn)
        {
          return documentColumn;
        }
        actualRenderColumn++;
      }
      
      isTabBorder = true;
    }

    if(actualRenderColumn == requestedRenderColumn)
    {
      if(isTabBorder)
      {
        return documentColumn + 1;
      }
      else
      {
        return documentColumn;
      }
    }

    actualRenderColumn++;
  }

  return m_TextLength;
}

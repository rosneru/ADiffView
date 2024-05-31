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
  unsigned long i, accumulatedColumn, tabIndent;

  accumulatedColumn = 0;

  // Parse each character of input text
  for(pInfo->srcTextColumn = 0; pInfo->srcTextColumn < m_TextLength; pInfo->srcTextColumn++)
  {
    if(accumulatedColumn >= resultingTextColumn)
    {
      tabIndent = tabSize - (unsigned long)(resultingTextColumn % tabSize);

      if(accumulatedColumn > resultingTextColumn)
      {
        // In midst of / among a tabulator block
        pInfo->srcTextColumn--;
        pInfo->numRemainingChars = 0;
        pInfo->numRemainingSpaces = tabIndent;
      }
      else
      {
        if(m_Text[pInfo->srcTextColumn] == '\t')
        {
          // Directly on the start of a tabulator block
          pInfo->numRemainingChars = 0;
          pInfo->numRemainingSpaces = tabIndent;
        }
        else
        {
          // A printable character, no tabulator block
          
          // Check how many chars / spaces until next tab position or eol
          for(i = pInfo->srcTextColumn; i < m_TextLength; i++)
          {
            if(m_Text[i] == '\t')
            {
              break;
            }
          }

          pInfo->numRemainingChars = i - pInfo->srcTextColumn;
          pInfo->numRemainingSpaces = 0;
        }
      }

      return;
    }

    if(m_Text[pInfo->srcTextColumn] == '\t')
    {
      // Increase actual result column by current position tabulator indent
      accumulatedColumn += (size_t)( tabSize - (accumulatedColumn % tabSize));
    }
    else
    {
      // Increase actual result column by one
      accumulatedColumn++;
    }
  }

  pInfo->numRemainingChars = 0;
  pInfo->numRemainingSpaces = 0;
}

unsigned long DiffLine::getRenderColumn(unsigned long originalColumn,
                                        unsigned long tabSize) const
{
  if(originalColumn > m_TextLength)
  {
    return 0;
  }

  unsigned long renderColumn = 0;
  for(unsigned long i = 0; i < m_TextLength; i++)
  {
    if(i == originalColumn)
    {
      return renderColumn;
    }

    if(m_Text[i] == '\t')
    {
      unsigned long  indent = tabSize - (renderColumn % tabSize);
      renderColumn += indent;
    }
    else
    {
      renderColumn++;
    }
  }

  return renderColumn;
}

#include <list>

#include "SelectableDiffFile.h"
#include "TextSelectionLine.h"
#include "TextSelectionRange.h"
#include "CmdCopySelection.h"

CmdCopySelection::CmdCopySelection(std::vector<WindowBase*>* pAllWindowsVector,
                                   const DiffWindow& diffWindow)
  : CommandBase(pAllWindowsVector),
    m_DiffWindow(diffWindow)
{
}

CmdCopySelection::~CmdCopySelection()
{
}

void CmdCopySelection::Execute(Window* pActiveWindow)
{
  const SelectableDiffFile* pDiffFile = m_DiffWindow.getSelectionDocument();
  if(pDiffFile == NULL)
  {
    return;
  }

  const std::list<TextSelectionLine*>* pSelectionLines = pDiffFile->getSelectionLines();
  if(pSelectionLines == NULL)
  {
    return;
  }

  if(pSelectionLines->size() < 1)
  {
    // At least one line must be selected
    return;
  }

  std::list<TextSelectionLine*>::const_iterator itLastItem = pSelectionLines->end();
  itLastItem--;

  ULONG lastLineId = (*itLastItem)->getLineId();

  // First pass: get number of total chars to write.
  //
  // NOTE: Fully selected lines (all but the bottom one) report one char
  // more on getNumMarkedChars(). This is needed anyhow here for the
  // additional line feed char that is written for those lines.)
  std::list<TextSelectionLine*>::const_iterator it;
  ULONG totalChars = 0;
  for(it = pSelectionLines->begin(); it != pSelectionLines->end(); it++)
  {
    TextSelectionLine*  pLine = *it;
    TextSelectionRange* pRange = pLine->getFirstSelectedBlock();
    totalChars += pRange->getNumMarkedChars(pRange->getFromColumn());
  }

  m_Clipboard.prepareMultilineWrite(totalChars - 1);

  for(it = pSelectionLines->begin(); it != pSelectionLines->end(); it++)
  {
    TextSelectionLine*  pLine = *it;
    TextSelectionRange* pRange = pLine->getFirstSelectedBlock();
    const char* pLineFullText = (*pDiffFile)[pLine->getLineId()]->getText();


    ULONG fromColumn = pRange->getFromColumn();
    const char* pLineTextStart = pLineFullText + fromColumn;
    bool doWriteLineFeed = it != itLastItem;
    ULONG numCharsToWrite = pRange->getNumMarkedChars(fromColumn);
    if(it != itLastItem)
    {
      numCharsToWrite--;
    }

    m_Clipboard.performMultilineWrite(pLineTextStart, numCharsToWrite, doWriteLineFeed);
    // printf("[FAIL] %.*s\n", pRange->getNumMarkedChars(fromColumn), pLineTextStart);
  }

  m_Clipboard.finishMultilineWrite();
}

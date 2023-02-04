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

  std::list<TextSelectionLine*>::const_iterator it;
  ULONG totalChars = 0;
  for(it = pSelectionLines->begin(); it != pSelectionLines->end(); it++)
  {
    TextSelectionLine*  pLine = *it;
    TextSelectionRange* pRange = pLine->getFirstSelectedBlock();
    totalChars += pRange->getNumMarkedChars(pRange->getFromColumn());
  }

  // For every selection line except the last one a '\n' is appended
  // totalChars += (pSelectionLines->size() - 1);

  m_Clipboard.prepareMultilineWrite(totalChars);

  for(it = pSelectionLines->begin(); it != pSelectionLines->end(); it++)
  {
    TextSelectionLine*  pLine = *it;
    TextSelectionRange* pRange = pLine->getFirstSelectedBlock();
    const char* pLineFullText = (*pDiffFile)[pLine->getLineId()]->getText();


    ULONG fromColumn = pRange->getFromColumn();
    const char* pLineTextStart = pLineFullText + fromColumn;
    bool doWriteLineFeed = it != itLastItem;
    m_Clipboard.performMultilineWrite(pLineTextStart, pRange->getNumMarkedChars(fromColumn), doWriteLineFeed);
  }

  m_Clipboard.finishMultilineWrite();
}

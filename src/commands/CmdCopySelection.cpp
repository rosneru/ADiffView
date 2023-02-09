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
    
    int lineNumChars = (*pDiffFile)[pLine->getLineId()]->getNumChars();
    int selectedNumChars = pRange->getNumMarkedChars(pRange->getFromColumn());
    if(lineNumChars != selectedNumChars)
    {
      printf("lineId %d: lineNumChars = %d, selectedNumChars = %d \n", pLine->getLineId(),
                                                                      lineNumChars,
                                                                      selectedNumChars);
    }
  }

  // Increase the number of total chars because for every selection line
  // except the last one a '\n' is appended
  totalChars += (pSelectionLines->size() - 1);

  // If the last selection line is empty, decrease the number of total
  // chars by one
  it = pSelectionLines->end();
  it--;
  ULONG numCharsLastSelectionLine = (*pDiffFile)[(*it)->getLineId()]->getNumChars();
  if(numCharsLastSelectionLine < 1)
  {
    totalChars--;
  }

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
    // printf("[FAIL] %.*s\n", pRange->getNumMarkedChars(fromColumn), pLineTextStart);
  }

  m_Clipboard.finishMultilineWrite();
}

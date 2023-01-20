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

  printf("a\n");
  const std::list<TextSelectionLine*>* pSelectionLines = pDiffFile->getSelectionLines();
  if(pSelectionLines == NULL)
  {
    return;
  }
  printf("b\n");
  printf("%lu\n", pSelectionLines->size());
  // for(std::list<TextSelectionLine*>::const_iterator it = selectionLines.begin(); it != selectionLines.end(); it++)
  // {
  //   printf("xyeah\n");
  //   // TextSelectionLine*  pLine = *it;
  //   // TextSelectionRange* pRange = pLine->getFirstSelectedBlock();
  //   // const char* pLineFullText = (*pDiffFile)[pLine->getLineId()]->getText();

  //   // ULONG fromColumn = pRange->getFromColumn();
  //   // const char* pLineTextStart = pLineFullText + fromColumn;
  //   // printf("%.*s\n", pRange->getNumMarkedChars(fromColumn), pLineTextStart);
  // }
}

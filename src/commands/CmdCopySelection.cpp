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

  const DynamicSelection& selection = pDiffFile->getDynamicSelection();
  long totalChars = selection.getNumTotalSelectedChars();
  if(totalChars == 0)
  {
    return;
  }

  long numSelectedLines = selection.getMaxLineId() - selection.getMinLineId() + 1;

  // Increase total chars by the number of selected lines -1 because all
  // but the last line get a newline attached (Newlines are not part of
  // the textLines collection)
  totalChars += (numSelectedLines - 1);

  m_Clipboard.prepareMultilineWrite(totalChars);

  long fromLineId = selection.getMinLineId();
  long toLineId = selection.getMaxLineId();
  for(long i = fromLineId; i <= toLineId; i++)
  {
    const char* pLineTextStart = NULL;
    long numCharsToWrite  = 0;
    bool doWriteLineFeed = true;

    if(i == fromLineId)
    {
      long startColumn = selection.getNextSelectionStart(i, 0);
      numCharsToWrite = (*pDiffFile)[i]->getNumChars() - startColumn;
      pLineTextStart = (*pDiffFile)[i]->getText() + startColumn;
    }
    else if (i == toLineId)
    {
      numCharsToWrite = selection.getNumMarkedChars(i, 0);
      pLineTextStart = (*pDiffFile)[i]->getText();
      doWriteLineFeed = false;
    }
    else
    {
      numCharsToWrite = (*pDiffFile)[i]->getNumChars();
      pLineTextStart = (*pDiffFile)[i]->getText();
    }

    m_Clipboard.performMultilineWrite(pLineTextStart, numCharsToWrite, doWriteLineFeed);
  }

  m_Clipboard.finishMultilineWrite();
}

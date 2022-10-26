#include <string.h>
#include <stdio.h>

#include "DiffInputFileLinux.h"

DiffInputFileLinux::DiffInputFileLinux(bool& isCancelRequested, 
                                       const char* pFileName,
                                       bool lineNumbersEnabled,
                                       bool ignoreTrailingSpaces)
  : DiffInputFileBase(isCancelRequested),
    m_pLineNumberBuffer(NULL)
{
  FILE* pFile = fopen(pFileName, "rb");
  if(pFile == NULL)
  {
    throw "Failed to open file.";
  }

  // Get the file size
  fseek (pFile, 0, SEEK_END);
  m_FileBytes = ftell (pFile);
  fseek (pFile, 0, SEEK_SET);

  if(m_FileBytes < 1)
  {
    fclose(pFile);
    throw "Empty file.";
  }

  // Read the whole file
  m_pFileBuffer = new char[m_FileBytes + 1];
  size_t bytesRead = fread(m_pFileBuffer, 1, m_FileBytes, pFile);
  if(bytesRead != m_FileBytes)
  {
    delete[] m_pFileBuffer;
    m_pFileBuffer = NULL;

    fclose(pFile);
    throw "Read wrong number of bytes from file.";
  }

  fclose(pFile);

  // When last line isn't finished with newline, add a newline at its
  // end and increase the file byte size by 1 (m_pFileBuffer was
  // allocated  one byte larger for this purpose)
  if(m_pFileBuffer[m_FileBytes-1] != '\n')
  {
    m_pFileBuffer[m_FileBytes] = '\n';
    m_FileBytes++;
  }

  // Pre-processing if necessary
  if(ignoreTrailingSpaces)
  {
    removeTrailingSpaces();
  }

  // Create DiffLine objects for each line in file buffer
  char* pLineStart = m_pFileBuffer;
  for(size_t i = 0; i < m_FileBytes; i++)
  {
    if(m_pFileBuffer[i] == '\n')
    {
      // Finalize the current line
      m_pFileBuffer[i] = 0;

      // Create DiffLine from curren line
      DiffLine* pDiffLine = new DiffLine(pLineStart);

      // Append DiffLine to list
      m_Lines.push_back(pDiffLine);

      // Next line starts after current line and thin finalizing '\0'
      pLineStart = m_pFileBuffer + i + 1;
    }
    else if(m_pFileBuffer[i] == 0)
    {
      // Skip leading zeros (the ones at the line start)
      pLineStart++;
    }
  }

  if(lineNumbersEnabled)
  {
    int digits = numDigits(getNumLines());
    size_t lineNumberBufSize = (digits + 2) * getNumLines();
    m_pLineNumberBuffer = new char[lineNumberBufSize];

    collectLineNumbers(getNumLines());
  }
}


DiffInputFileLinux::~DiffInputFileLinux()
{
  std::vector<DiffLine*>::iterator it;
  for(it = m_Lines.begin(); it != m_Lines.end(); it++)
  {
    delete *it;
  }

  if(m_pLineNumberBuffer != NULL)
  {
    delete[] m_pLineNumberBuffer;
    m_pLineNumberBuffer = NULL;
  }

  if(m_pFileBuffer != NULL)
  {
    delete[] m_pFileBuffer;
    m_pFileBuffer = NULL;
  }
}


void DiffInputFileLinux::collectLineNumbers(size_t maxNumLines)
{
  int digits = numDigits(maxNumLines);

  for(size_t i = 0; i < getNumLines(); i++)
  {
    size_t bufIdx = i * (digits + 2);
    char* pLineNumber = m_pLineNumberBuffer + bufIdx;

    sprintf(pLineNumber, "%*d ", digits, (i + 1));

    DiffLine* pLine = (*this)[i];
    pLine->setLineNumText(pLineNumber);
  }
}

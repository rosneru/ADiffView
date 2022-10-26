#include "DiffInputFileBase.h"

DiffInputFileBase::DiffInputFileBase(bool& isCancelRequested)
  : m_IsCancelRequested(isCancelRequested),
    m_pFileBuffer(NULL),
    m_FileBytes(0)
{

}

DiffInputFileBase::~DiffInputFileBase()
{

}


void DiffInputFileBase::removeTrailingSpaces()
{
    long i = m_FileBytes;
    bool isLineEnd = false;
    while(--i >= 0)
    {
      if(!isLineEnd)
      {
        // A newline could be a hint that this is a line ending
        if(m_pFileBuffer[i] == '\n')
        {
          // But only if the character before it is not also an '\n'
          if((i > 0) && (m_pFileBuffer[i - 1] != '\n'))
          {
            isLineEnd = true;
            m_pFileBuffer[i] = 0;
          }
        }
      }
      else if(m_pFileBuffer[i] == ' ' || m_pFileBuffer[i] == '\t')
      {
        // Overwrite whitespace with 0
        m_pFileBuffer[i] = 0;
      }
      else
      {
        isLineEnd = false;
        m_pFileBuffer[i + 1] = '\n';
      }
    }
}
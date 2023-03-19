#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
#endif

#include <stddef.h>

#include "AmigaFile.h"

AmigaFile::AmigaFile(const char* pFileName, ULONG accessMode)
  : MAX_LINE_LENGTH(1024), // TODO A better solution needed?
    m_pLineBuf((STRPTR) AllocVec(MAX_LINE_LENGTH, MEMF_ANY|MEMF_CLEAR)),
    m_FileDescriptor(0),
    m_pFib((struct FileInfoBlock*) AllocVec((sizeof(struct FileInfoBlock)), MEMF_ANY|MEMF_CLEAR))
{
  BPTR pLock = Lock(pPath, ACCESS_READ);
  if(pLock != 0)
  {
    freeDirContent(pFibNodesList);
    return NULL;
  }



  m_pLineBuf = ;
  m_FileDescriptor = Open(pFileName, accessMode);
  if(m_FileDescriptor == 0)
  {
    throw "Failed to open file.";
  }
}


AmigaFile::~AmigaFile()
{
  if(m_pLineBuf != NULL)
  {
    FreeVec(m_pLineBuf);
    m_pLineBuf = NULL;
  }

  if(m_FileDescriptor != 0)
  {
    Close(m_FileDescriptor);
    m_FileDescriptor = 0;
  }
}


ULONG AmigaFile::countLines()
{
  ULONG numLines = 0;
  size_t readBufSize = MAX_LINE_LENGTH - 1; // -1 => Workaround for a
                                            // bug in AmigaOS v36/37

  // Rewind reading pointer to start of file
  Seek(m_FileDescriptor, 0, OFFSET_BEGINNING);

  // Reading all lines and increment counter
  while(FGets(m_FileDescriptor, m_pLineBuf, readBufSize) != NULL)
  {
    numLines++;
  }

  // Rewind reading pointer to start of file again
  Seek(m_FileDescriptor, 0, OFFSET_BEGINNING);

  return numLines;
}


char* AmigaFile::readLine()
{
  ULONG readBufSize = MAX_LINE_LENGTH - 1; // -1 => Workaround for a OS v36 failure

  if(FGets(m_FileDescriptor, m_pLineBuf, readBufSize) == NULL)
  {
    return NULL;
  }

  for(size_t i = 0; i < readBufSize; i++)
  {
    if((m_pLineBuf[i] == '\r') ||
       (m_pLineBuf[i] == '\n'))
    {
      m_pLineBuf[i] = 0;
      break;
    }
  }

  return m_pLineBuf;
}


ULONG AmigaFile::getByteSize()
{
  Seek(m_FileDescriptor, 0, OFFSET_END);
  ULONG size = Seek(m_FileDescriptor, 0, OFFSET_BEGINING);

  return size;
}


bool AmigaFile::readFile(void* pBuffer, size_t bufferSize)
{
  LONG bytesRead = Read(m_FileDescriptor, pBuffer, bufferSize);
  return bytesRead == static_cast<LONG>(bufferSize);
}

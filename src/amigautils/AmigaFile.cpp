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
    m_pFib((struct FileInfoBlock*) AllocVec((sizeof(struct FileInfoBlock)), 
                                            MEMF_ANY|MEMF_CLEAR))
{
  if(m_pLineBuf == NULL)
  {
    cleanup();
    throw "Failed to alloc line buffer memory in AmigaFile class.";
  }

  if(m_pFib == NULL)
  {
    cleanup();
    throw "Failed to fib memory in AmigaFile class.";
  }

  BPTR pLock = Lock(pFileName, ACCESS_READ);
  if(pLock != 0)
  {
    if(Examine(pLock, m_pFib) == DOSFALSE)
    {
      UnLock(pLock);
      cleanup();
      throw "Failed to examine in AmigaFile class.";
    }
  }

  UnLock(pLock);

  m_FileDescriptor = Open(pFileName, accessMode);
  if(m_FileDescriptor == 0)
  {
    cleanup();
    throw "Failed to open file.";
  }
}


AmigaFile::~AmigaFile()
{
  cleanup();
}

bool AmigaFile::isOlderThan(const AmigaFile& other)
{
  LONG diff = CompareDates(&m_pFib->fib_Date, &other.m_pFib->fib_Date);
  return diff > 0;
}

void AmigaFile::cleanup()
{
  if(m_FileDescriptor != 0)
  {
    Close(m_FileDescriptor);
    m_FileDescriptor = 0;
  }

  if(m_pFib != NULL)
  {
    FreeVec(m_pFib);
  }

  if(m_pLineBuf != NULL)
  {
    FreeVec(m_pLineBuf);
  }
}

char* AmigaFile::readLine()
{
  ULONG readBufSize = MAX_LINE_LENGTH - 1; // -1 => Workaround for a OS v36 failure

  if(FGets(m_FileDescriptor, m_pLineBuf, readBufSize) == NULL)
  {
    return NULL;
  }

  for(ULONG i = 0; i < readBufSize; i++)
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

bool AmigaFile::readFile(void* pBuffer, ULONG bufferSize)
{
  LONG bytesRead = Read(m_FileDescriptor, pBuffer, bufferSize);
  return bytesRead == static_cast<LONG>(bufferSize);
}

ULONG AmigaFile::countLines()
{
  ULONG numLines = 0;
  ULONG readBufSize = MAX_LINE_LENGTH - 1; // -1 => Workaround for a
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

ULONG AmigaFile::getByteSize() const
{
  ULONG size = (ULONG)m_pFib->fib_Size;
  return size;
}

const struct DateStamp* AmigaFile::getDate() const
{
  return &m_pFib->fib_Date;
}

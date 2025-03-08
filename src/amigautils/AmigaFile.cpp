#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
#endif

#include <dos/dosextens.h>

#include "AmigaFile.h"

AmigaFile::AmigaFile(const char* pPath, ULONG accessMode)
  : MAX_LINE_LENGTH(1024), // TODO A better solution needed?
    m_pLineBuf((STRPTR) AllocVec(MAX_LINE_LENGTH, MEMF_ANY|MEMF_CLEAR)),
    m_OriginalCurrentDirLock(0),
    m_FileLock(0),
    m_FileDescriptor(0),
    m_pFib((struct FileInfoBlock*) AllocVec((sizeof(struct FileInfoBlock)), 
                                            MEMF_ANY|MEMF_CLEAR))
{
  if(m_pLineBuf == NULL)
  {
    cleanup();
    throw "Failed to open file. (AllocVec for line buffer)";
  }

  if(m_pFib == NULL)
  {
    cleanup();
    throw "Failed to open file. (AllocVec for fib)";
  }
  
  struct Process* pProcess;
  if(!(pProcess = (struct Process *)FindTask(NULL)))
  {
    cleanup();
    throw "Failed to open file. (FindTask)";
  }

  m_OriginalCurrentDirLock = pProcess->pr_CurrentDir;

  // Lock the file (Will be released when this AmigaFile object is
  // destroyed in AmigaFile::cleanup)
  m_FileLock = lockFromLongName(pPath);
  if(!m_FileLock)
  {
    cleanup();
    throw "Failed to open file.";
  }

  if(DOSFALSE == Examine(m_FileLock, m_pFib))
  {
    cleanup();
    throw "Failed to open file. (Examine)";
  }

  // Enter the directory of the file
  BPTR pFileDirLock = ParentDir(m_FileLock);
  CurrentDir(pFileDirLock);
  
  // Open the file relatively, only by its name. This should work as
  // the file dir has made current dir in the step above.
  STRPTR pName = FilePart(pPath);
  m_FileDescriptor = Open(pName, accessMode);

  // Change current dir to the formerly, original one and release lock
  // to the file dir.
  CurrentDir(m_OriginalCurrentDirLock);
  UnLock(pFileDirLock);

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

  if(m_FileLock)
  {
    UnLock(m_FileLock);
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

BPTR AmigaFile::lockFromLongName(const char* name)
{
  LONG pos = 0;
  BPTR resultLock = 0, lock = 0;
  BPTR oldLock = -1L;     // Never a valid lock
  char buffer[108 + 32];  // Long enough for a component and a device name

  do
  {
    pos = SplitName(name,'/', buffer, pos, sizeof(buffer));
    if (pos < 0)
    {
      // No separator found, call now Lock
      resultLock = Lock(buffer, SHARED_LOCK);
      break;
    }
    else
    {
      // Lock the partial path so far and abort on error If two slashes
      // are next to each other, go to the parent directory.
      if (!(lock = Lock(*buffer ? buffer : "/", SHARED_LOCK)))
      {
        break;
      }

      // Rotate directories
      lock = CurrentDir(lock);

      // Unlock previous directory, keep the old directory
      if (oldLock >= 0)
      {
        UnLock(lock);
      }
      else
      {
        oldLock = lock;
      }
    }
  } while (1);

  // Restore the current directory if any. None of the functions touch
  // IoErr().
  if (oldLock >= 0)
  {
    UnLock(CurrentDir(oldLock));
  }

  return resultLock;
}

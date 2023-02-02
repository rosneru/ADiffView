#ifdef __clang__
  #include <clib/exec_protos.h>
#else
  #include <proto/exec.h>
#endif

#include <devices/clipboard.h>

#include "Clipboard.h"



Clipboard::Clipboard()
  : m_pMsgPort(CreateMsgPort()),
    m_pIOClipReq(NULL)
{
  if(m_pMsgPort == NULL)
  {
    throw "Failed to create MessagePort for Clipboard.";
  }

  m_pIOClipReq = (struct IOClipReq*) CreateIORequest(m_pMsgPort,
                                                     sizeof(struct IOClipReq));
  if(m_pIOClipReq == NULL)
  {
    DeleteMsgPort(m_pMsgPort);
    m_pMsgPort = NULL;

    throw "Failed to create IORequest for Clipboard device.";
  }

  // Open the timer.device
  BYTE res = OpenDevice("clipboard.device",
                        0,
                        (struct IORequest*) m_pIOClipReq,
                        0L);
  if(res != NULL)
  {
    DeleteIORequest(m_pIOClipReq);
    m_pIOClipReq = NULL;
    DeleteMsgPort(m_pMsgPort);
    m_pMsgPort = NULL;

    throw "Failed to open the clipboard.device for Clipboard.";
  }
}

Clipboard::~Clipboard()
{
  CloseDevice((struct IORequest*)m_pIOClipReq);
  DeleteIORequest(m_pIOClipReq);
  DeleteMsgPort(m_pMsgPort);
}

bool Clipboard::writeFText(const char* string, ULONG slen)
{
  bool isOdd = (slen & 1) == true;
  ULONG length = isOdd ? slen + 1 : slen;

  // Initial set up Offset, Error, and ClipId
  m_pIOClipReq->io_Offset = 0;
  m_pIOClipReq->io_Error = 0;
  m_pIOClipReq->io_ClipID = 0;

  // Create the IFF header information
  writeLong((long*)"FORM");
  length += 12L;                  // + "[size]FTXTCHRS"
  writeLong((long*)&length);      // total length
  writeLong((long*)"FTXT");
  writeLong((long*)"CHRS");
  writeLong((long*)&slen);        // string length

  // Write string
  m_pIOClipReq->io_Data = (STRPTR)string;
  m_pIOClipReq->io_Length = slen;
  m_pIOClipReq->io_Command = CMD_WRITE;

  DoIO((struct IORequest*)m_pIOClipReq);

  // Pad if needed
  if(isOdd)
  {
    m_pIOClipReq->io_Data = (STRPTR)"";
    m_pIOClipReq->io_Length = 1L;

    DoIO((struct IORequest*)m_pIOClipReq);
  }

  // Tell the clipboard we are done writing
  m_pIOClipReq->io_Command = CMD_UPDATE;
  DoIO((struct IORequest*)m_pIOClipReq);

  // Check if io_Error was set by any of the preceding IO requests
  if(m_pIOClipReq->io_Error == TRUE)
  {
    return false;
  }

  return true;
}

bool Clipboard::prepareMultilineWrite(ULONG slen)
{
  return false;
}

bool Clipboard::performMultilineWrite(const char* string, ULONG slen, bool doAppendNewline)
{
  return false;
}

bool Clipboard::finishMultilineWrite()
{
  return false;
}


bool Clipboard::writeLong(long* pLongData)
{
  m_pIOClipReq->io_Data = (STRPTR)pLongData;
  m_pIOClipReq->io_Length = 4L;
  m_pIOClipReq->io_Command = CMD_WRITE;
  
  DoIO((struct IORequest*)m_pIOClipReq);

  if(m_pIOClipReq->io_Actual == 4)
  {
    if(m_pIOClipReq->io_Error == FALSE)
    {
      return true;
    }
  }

  return false;
}

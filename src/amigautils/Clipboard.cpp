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

bool Clipboard::writeFText(const char* pText, ULONG textLength)
{
  if(prepareMultilineWrite(textLength) == false)
  {
    return false;
  }

  if(performMultilineWrite(pText, textLength, false) == false)
  {
    return false;
  }

  if(finishMultilineWrite() == false)
  {
    return false;
  }

  return true;
}

bool Clipboard::prepareMultilineWrite(ULONG textLength)
{
  m_IsOddLength = (textLength & 1) == true;
  ULONG totalLength = m_IsOddLength ? textLength + 1 : textLength;

  // Initial set up Offset, Error, and ClipId
  m_pIOClipReq->io_Offset = 0;
  m_pIOClipReq->io_Error = 0;
  m_pIOClipReq->io_ClipID = 0;
  m_pIOClipReq->io_Command = CMD_WRITE;

  // Create the IFF header information
  writeLong((long*)"FORM");
  totalLength += 12L;               // + "[size]FTXTCHRS" TODO: Check what if size exceeds 9999?? Then it must be 13?
  writeLong((long*)&totalLength);   // length with remaining header (All after FORM)
  writeLong((long*)"FTXT");
  writeLong((long*)"CHRS");
  writeLong((long*)&textLength);

  // Check if any of these `writeLong` failed.
  if(m_pIOClipReq->io_Error == TRUE)
  {
    return false;
  }

  return true;
}

bool Clipboard::performMultilineWrite(const char* pText, ULONG textLength, bool doAppendNewline)
{
  // Write string
  m_pIOClipReq->io_Data = (STRPTR)pText;
  m_pIOClipReq->io_Length = textLength;

  DoIO((struct IORequest*)m_pIOClipReq);
  if(m_pIOClipReq->io_Error == TRUE)
  {
    return false;
  }

  if(doAppendNewline)
  {
    m_pIOClipReq->io_Data = (STRPTR)"\n";
    m_pIOClipReq->io_Length = 1L;
    DoIO((struct IORequest*)m_pIOClipReq);
    if(m_pIOClipReq->io_Error == TRUE)
    {
      return false;
    }
  }

  return true;
}

bool Clipboard::finishMultilineWrite()
{
  // Pad if needed
  if(m_IsOddLength)
  {
    m_pIOClipReq->io_Data = (STRPTR)"";
    m_pIOClipReq->io_Length = 1L;

    DoIO((struct IORequest*)m_pIOClipReq);
    if(m_pIOClipReq->io_Error == TRUE)
    {
      return false;
    }
  }

  // Tell the clipboard we are done writing
  m_pIOClipReq->io_Command = CMD_UPDATE;
  DoIO((struct IORequest*)m_pIOClipReq);
  if(m_pIOClipReq->io_Error == TRUE)
  {
    return false;
  }

  return true;
}


bool Clipboard::writeLong(long* pLongData)
{
  m_pIOClipReq->io_Data = (STRPTR)pLongData;
  m_pIOClipReq->io_Length = 4L;
  m_pIOClipReq->io_Command = CMD_WRITE;
  
  DoIO((struct IORequest*)m_pIOClipReq);

  if(m_pIOClipReq->io_Actual != 4)
  {
    return false;
  }

  if(m_pIOClipReq->io_Error == TRUE)
  {
    return false;
  }

  return true;
}

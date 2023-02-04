#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <devices/clipboard.h>
#include <exec/ports.h>
#include <exec/io.h>

/**
 * Class for easy Clipboard access
 *
 * @author Uwe Rosner
 * @date 30/01/2023
 */
class Clipboard
{
public:
  Clipboard();
  virtual ~Clipboard();

  bool writeFText(const char* pText, ULONG textLength);

  bool prepareMultilineWrite(ULONG textLength);
  bool performMultilineWrite(const char* pText, ULONG textLength, bool doAppendNewline);
  bool finishMultilineWrite();

private:
  struct MsgPort* m_pMsgPort;
  struct IOClipReq* m_pIOClipReq;
  bool m_IsOddLength;

  bool writeLong(long* pLongData);
};

#endif

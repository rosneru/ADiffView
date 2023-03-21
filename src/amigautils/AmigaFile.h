#ifndef AMIGA_FILE_H
#define AMIGA_FILE_H

#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/dos.h>

/**
 * Represents an open file. On creation an exception is thrown if the
 * file can't be opened.
 *
 * @author Uwe Rosner
 * @date 18/10/2018
 */
class AmigaFile
{
public:
  /**
   * Creates an open file with given name and given access mode.
   *
   * @param pFileName Name of the file to open.
   * @param accessMode One of MODE_NEWFILE, MODE_READWRITE or
   *        MODE_OLDFILE. DMODE_OLDFILE.
   */
  AmigaFile(const char* pFileName, ULONG accessMode = MODE_OLDFILE);
  
  /**
   * Destroys the file object.
   */
  virtual ~AmigaFile();

  /**
   * Reads the next line of the file into an internal line buffer and
   * returns the buffer address.
   *
   * NOTE: On each call the internal buffer is overwritten. You must
   *       take care and preserve the read line if necessary.
   */
  char* readLine();

  /**
   * Reads the whole file into the given buffer.
   */
  bool readFile(void* pBuffer, ULONG bufferSize);

  /**
   * Counts the number of lines in file. The file has to be opened
   * before calling this method.
   *
   * NOTE After this call the file reading pointer will be reset to
   *      the start of the file.
   *
   * @returns
   * Number of lines in file. A value of '0' can mean that the file
   * isn't opened or that the number of lines = 0. Check the Open()
   * result to be sure.
   */
  ULONG countLines();

  /**
   * Gets the file size in bytes.
   *
   * NOTE: Changes the file handle position! After this operation the
   * file handle points to the start of the file.
   *
   * @returns
   * File size in bytes. A value of '0' can mean that the file isn't
   * opened or that the file size = 0. Check the Open() result to be
   * sure.
   */
  ULONG getByteSize() const;

  /**
   * Returns the DateStamp of the file
   */
  const struct DateStamp * getDate() const;

private:
  const ULONG MAX_LINE_LENGTH;
  STRPTR m_pLineBuf;
  BPTR m_FileDescriptor;
  struct FileInfoBlock* m_pFib;

  void cleanup();
};

#endif

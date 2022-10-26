#ifndef SELECTABLE_DIFF_FILE_H
#define SELECTABLE_DIFF_FILE_H

#include "DiffFileBase.h"
#include "TextSelection.h"

/**
 * Adds the ability to select [0; n] blocks of text to an existing
 * DiffFile.
 *
 * @author Uwe Rosner
 * @date 30/12/2020
 */
class SelectableDiffFile
{
public:
  SelectableDiffFile(const DiffFileBase& diffFile);

  const DiffLine* operator[](unsigned long index) const;

  unsigned long getNumLines() const;

  void addSelection(unsigned long lineId, 
                    unsigned long fromColumn, 
                    unsigned long toColumn);

  void clearSelection();

  long getNumNormalChars(unsigned long lineId, unsigned long columnId);
  long getNumMarkedChars(unsigned long lineId, unsigned long columnId);

private:
  const DiffFileBase& m_DiffFile;
  TextSelection m_Selection;

};

#endif

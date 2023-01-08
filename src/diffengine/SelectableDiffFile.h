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

  /**
   * Start a dynamic selection at given line and column
   */
  void startDynamicSelection(unsigned long lineId, unsigned long columnId);

  /**
   * Update a dynamic selection to given line and column.
   *
   * NOTE: If the new lineId is more than one line above or below the
   * current 'selection target point' it is limited one line distance.
   * So, in some cases this method must be called multiple times with
   * the same lineId to reach the requested lineId.
   */
  void updateDynamicSelection(unsigned long lineId, unsigned long columnId);

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

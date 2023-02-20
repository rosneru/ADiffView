#ifndef SELECTABLE_DIFF_FILE_H
#define SELECTABLE_DIFF_FILE_H

#include "DiffFileBase.h"
#include "SelectionBase.h"
#include "StaticSelection.h"
#include "DynamicSelection.h"

/**
 * Adds the ability text selection to an existing DiffFile.
 *
 * Contains two TextSelection objects:
 * 1) One to be used for search results where the matches can be added 
 *    successively by calling \see addSearchResultSelectionBlock
 * 2) The other one is a dynamic selection which can first started with 
 *    \see startDynamicSelection and the updated with
 *    \see updateDynamicSelection
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

  void addSearchResultSelectionBlock(unsigned long lineId, 
                                     unsigned long fromColumn, 
                                     unsigned long toColumn);

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


  void clearSearchResultSelection();

  void clearDynamicSelection();

  /**
   * Activates the dynamic selection mode.
   */
  void activateDynamicSelection(long pageTopLineId, long pageBottomLineId);

  /**
   * Activates the search result selection mode.
   */
  void activateSearchResultSelection(long pageTopLineId, long pageBottomLineId);

  /**
   * Returns a list with the ids of the lines whose selection was
   * changed during last dynamic selection operation.
   */
  const std::list<long>& getUpdatedLineIds();

  void clearUpdatedLineIds();

  bool isPointInSelection(unsigned long lineId, unsigned long columnId) const;

  long getNumNormalChars(unsigned long lineId, unsigned long columnId);
  long getNumMarkedChars(unsigned long lineId, unsigned long columnId);

  const DynamicSelection& getDynamicSelection() const;

private:
  const DiffFileBase& m_DiffFile;
  StaticSelection m_SearchResultSelection;
  DynamicSelection m_DynamicSelection;
  SelectionBase* m_pCurrentSelection;
};

#endif

#ifndef SEARCH_OPTIONS_H
#define SEARCH_OPTIONS_H

enum SearchLocation
{
  /**
   * Search in left and right file
   */
  SL_BothFiles,

  /**
   * Search in left file
   */
  SL_LeftFile,

  /**
   * Search in right file
   */
  SL_RightFile,

  /**
   * Search only in difference lines (ADDED, DELETED and CHANGED) of
   * left and right file
   */
  SL_Differences
};


enum SearchDirection
{
  SD_Downward,
  SD_Upward,
};

#endif

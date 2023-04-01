#ifndef ADIFFVIEW_ARGS_H
#define ADIFFVIEW_ARGS_H

#include <workbench/workbench.h>

#include <string>


/**
 * Class to parse and store the command line / workbench arguments
 * retrieved at the start of ADiffView.
 *
 * @author Uwe Rosner
 * @date 06/12/2019
 */
class ADiffViewArgs
{
public:

  /**
   * Creates the options object. On creation it extracts the arguments
   * which on program start optionally can be provided via Workbench
   * or CLI.
   *
   * @param argc
   * The argc variable from main().
   *
   * @param argv
   * The argv array from main().
   *
   */
  ADiffViewArgs(int argc, char **argv);

  virtual ~ADiffViewArgs();

  /**
   * Path to left file
   */
  const std::string& getLeftFile() const;

  /**
   * Path to right file
   */
  const std::string& getRightFile() const;

  /**
   * When started from workbench this returns the DiscObject associated
   * with the application icon. Returns NULL when started from Shell or
   * when it failed to get the disc object.
   */
  DiskObject* getDiscObject() const;

  /**
   * Path to a text editor
   */
  const std::string& getEditorPath() const;

  /**
   * Name of the public screen to run ADiffView on
   * If empty a own screen will be opened (Workbench clone, 8 colors)
   */
  const std::string& getPubScreenName() const;
  
  /**
   * Set the name of the public screen ADiffView will open by itself
   */
  void setPubScreenName(const char* pName);

  /**
   * Returns the tab size specified by startup argument or the default
   * value.
   */
  long getTabSize() const;

  /**
   * If true, the number of differences reported is counted per lines,
   * not per 'groups of difference blocks' as it is the default.
   *
   * Defaults to false.
   */
  bool isCountByLines() const;

  /**
   * If true, the diff will be performed immediately without waiting
   * for the user to click the "Diff" button in OpenFilesWindow.
   *
   * This only works when both files, left and right are also passed
   * as arguments.
   *
   * Defaults to false.
   */
  bool isDontAsk() const;

  /**
   * If true the editor to edit files will be started on the same screen
   * as ADiffView by starting it with the PUBSCREEN argument.
   */
  bool isEditorOnPubScreen() const;

  /**
   * If true, whitespaces on line endings are not taken into account
   * when the diff is being calculated.
   *
   * Defaults to false.
   */
  bool isIgnoreTrailingSpaces() const;

  /**
   * If true, no AppIcon for file drag'n'drop is installed on the
   * Workbench
   */
  bool isNoAppIcon() const;

  /**
   * If true, the line numbers will be shown in the diff result window.
   *
   * Defaults to true
   */
  bool isShowLineNumbers() const;


private:
  int m_ArgC;
  char** m_pArgV;

  struct DiskObject* m_pDiskObject;
  std::string m_PubScreenName;
  std::string m_LeftFilePath;
  std::string m_RightFilePath;
  std::string m_EditorPath;
  bool m_bCountByLines;
  bool m_bDontAsk;
  bool m_bEditorOnPubscreen;
  bool m_bIgnoreTrailingSpaces;
  bool m_bNoAppIcon;
  bool m_bShowLineNumbers;
  long m_TabSize;

  void readWorkbenchArgs();
  void readCommandLineArgs();

  /**
   * Uses FindTooltype to find and return the value of given name.
   * Returns NULL if tooltype not found.
   *
   * Is basically just a wrapper around FindTooltype with the proper
   * types needed. Helps to avoid some casts and STRPTR compiler
   * warnings in readWorkbenchArgs()
   */
  char* toolTypeValue(const STRPTR* ppTooltypeArray,
                      const char* pTooltypeName);
};

#endif

#ifdef __clang__
  #include <clib/dos_protos.h>
  #include <clib/exec_protos.h>
  #include <clib/icon_protos.h>
#else
  #include <proto/dos.h>
  #include <proto/exec.h>
  #include <proto/icon.h>
#endif

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <stdlib.h>

#include "ADiffViewArgs.h"


ADiffViewArgs::ADiffViewArgs(int argc, char **argv)
  : m_ArgC(argc),
    m_pArgV(argv),
    m_pDiskObject(NULL),
    m_EditorPath("Ed"),
    m_bCountByLines(false),
    m_bDontAsk(false),
    m_bEditorOnPubscreen(false),
    m_bIgnoreTrailingSpaces(false),
    m_bNoAppIcon(false),
    m_bShowLineNumbers(true),
    m_TabSize(8)
{
  if(argc == 0)
  {
    //
    // Started from Workbench
    //
    readWorkbenchArgs();

  }
  else
  {
    //
    // Started from CLI
    //
    readCommandLineArgs();
  }
}


ADiffViewArgs::~ADiffViewArgs()
{
  if(m_pDiskObject != NULL)
  {
    FreeDiskObject(m_pDiskObject);
    m_pDiskObject = NULL;
  }
}

const std::string& ADiffViewArgs::getLeftFile() const
{
  return m_LeftFilePath;
}


const std::string& ADiffViewArgs::getRightFile() const
{
  return m_RightFilePath;
}

DiskObject* ADiffViewArgs::getDiscObject() const
{
  return m_pDiskObject;
}

const std::string& ADiffViewArgs::getEditorPath() const
{
  return m_EditorPath;
}

const std::string& ADiffViewArgs::getPubScreenName() const
{
  return m_PubScreenName;
}

void ADiffViewArgs::setPubScreenName(const char* pName)
{
  m_PubScreenName = pName;
}

long ADiffViewArgs::getTabSize() const
{
  return m_TabSize;
}

bool ADiffViewArgs::isCountByLines() const
{
  return m_bCountByLines;
}

bool ADiffViewArgs::isDontAsk() const
{
  return m_bDontAsk;
}

bool ADiffViewArgs::isEditorOnPubScreen() const
{
  return m_bEditorOnPubscreen;
}

bool ADiffViewArgs::isIgnoreTrailingSpaces() const
{
  return m_bIgnoreTrailingSpaces;
}

bool ADiffViewArgs::isNoAppIcon() const
{
  return m_bNoAppIcon;
}

bool ADiffViewArgs::isShowLineNumbers() const
{
  return m_bShowLineNumbers;
}

void ADiffViewArgs::readWorkbenchArgs()
{
  int bufLen = 2048;  // TODO How to get rid of this fixed maximum?
  STRPTR pBuf = (STRPTR) AllocVec(bufLen, MEMF_ANY);
  if(pBuf == NULL)
  {
    return;
  }

  struct WBStartup* pWbStartup = (struct WBStartup*) m_pArgV;
  struct WBArg* pWbArg = pWbStartup->sm_ArgList;
  for(int i=0; i < pWbStartup->sm_NumArgs; i++)
  {
    if((pWbArg[i].wa_Lock != 0))  // TODO check. Was 'NULL' before.
    {
      if(i == 0)
      {
        //
        // The first pWbArg is the application icon itself. Getting
        // the PUBSCREEN tooltype from it
        //

        // Make the directory of the icon to the current dir (cd)
        BPTR oldDir = CurrentDir(pWbArg[i].wa_Lock);

        m_pDiskObject = GetDiskObjectNew((STRPTR) pWbArg[i].wa_Name);

        if(m_pDiskObject != NULL)
        {
          const STRPTR* ppTooltypeArray = m_pDiskObject->do_ToolTypes;

          char* pValue = toolTypeValue(ppTooltypeArray, "PUBSCREEN");
          if(pValue != NULL)
          {
            m_PubScreenName = pValue;
          }

          pValue = toolTypeValue(ppTooltypeArray, "EDITOR");
          if(pValue != NULL)
          {
            m_EditorPath = pValue;
          }

          if(toolTypeValue(ppTooltypeArray, "EDITORONPUBSCREEN") != NULL)
          {
            m_bEditorOnPubscreen = true;
          }

          if(toolTypeValue(ppTooltypeArray, "DONOTASK") != NULL)
          {
            m_bDontAsk = true;
          }

          if(toolTypeValue(ppTooltypeArray, "NOAPPICON") != NULL)
          {
            m_bNoAppIcon = true;
          }

          if(toolTypeValue(ppTooltypeArray, "NOLINENUMBERS") != NULL)
          {
             m_bShowLineNumbers = false;
          }

          if(toolTypeValue(ppTooltypeArray, "IGNORETRAILINGSPACES") != NULL)
          {
             m_bIgnoreTrailingSpaces = true;
          }

          if(toolTypeValue(ppTooltypeArray, "COUNTBYLINES") != NULL)
          {
             m_bCountByLines = true;
          }

          if((pValue = toolTypeValue(ppTooltypeArray, "TABSIZE")) != NULL)
          {
            long parsedTabSize = strtol(pValue, NULL, 10);
            if(parsedTabSize > 0)
            {
              m_TabSize = parsedTabSize;
            }
          }
        }

        // Change back to the formerly current directory
        CurrentDir(oldDir);
      }
      else if(i < 3)
      {
        if(NameFromLock(pWbArg[i].wa_Lock, pBuf, bufLen) != 0)
        {
          if(AddPart(pBuf,(STRPTR) pWbArg[i].wa_Name, bufLen))
          {

            if(i == 1)
            {
              m_LeftFilePath = pBuf;
            }
            else
            {
              m_RightFilePath = pBuf;
            }
          }
        }
      }
      else
      {
        // We only need the filenames of the first 2 selected icons
        break;
      }
    }
  }

  FreeVec(pBuf);
}

void ADiffViewArgs::readCommandLineArgs()
{
    // Reading the command line arguments
    const char argTempl[] = "FILES/M,PUBSCREEN/K,EDITOR/K,EDITORONPUBSCREEN/S,DONOTASK/S,NOAPPICON/S,NOLINENUMBERS/S,IGNORETRAILINGSPACES/S,COUNTBYLINES/S,TABSIZE/K/N";
    LONG args[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    struct RDArgs* pReadArgs = ReadArgs(argTempl, args, NULL);
    if(pReadArgs == NULL)
    {
      return;
    }

    if(args[0] != 0)
    {
      // args[0] contains a array of pointers to the passed FILEs.
      STRPTR* pFiles = (STRPTR*) args[0];

      // Even if there are passed more than two FILE arguments, only
      // the first two of them are taken and stored as left and right 
      // file path.
      if(pFiles[0] != NULL)
      {
        m_LeftFilePath = pFiles[0];
      }

      if(pFiles[1] != NULL)
      {
        m_RightFilePath = pFiles[1];
      }
    }

    if(args[1] != 0)
    {
      m_PubScreenName = (const char*) args[1];
    }

    if(args[2] != 0)
    {
      m_EditorPath = (const char*) args[2];
    }

    if(args[3] != 0)
    {
      m_bEditorOnPubscreen = true;
    }

    if(args[4] != 0)
    {
      m_bDontAsk = true;
    }

    if(args[5] != 0)
    {
      m_bNoAppIcon = true;
    }

    if(args[6] != 0)
    {
      m_bShowLineNumbers = false;
    }

    if(args[7] != 0)
    {
      m_bIgnoreTrailingSpaces = true;
    }

    if(args[8] != 0)
    {
      m_bCountByLines = true;
    }

    if(args[9] != 0)
    {
      LONG parsedTabSize = *((LONG*)args[9]);
      if(parsedTabSize > 0)
      {
        m_TabSize = parsedTabSize;
      }
    }

    FreeArgs(pReadArgs);
}


char* ADiffViewArgs::toolTypeValue(const STRPTR* ppTooltypeArray, 
                                   const char* pTooltypeName)
{
  return (char*)FindToolType((CONST_STRPTR*)ppTooltypeArray, (STRPTR)pTooltypeName);
}

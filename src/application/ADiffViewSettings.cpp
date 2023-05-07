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


#include "ADiffViewSettings.h"


/**
 * The default color set
 */
ADiffViewSettings::ColorArray ADiffViewSettings::m_sColors =
{
  0x00040004, // 0x0004 - Load 3 colors, starting from 0x004
  0xf3f3f3f3, 0xb5b5b5b5, 0xb9b9b9b9, // red
  0xc1c1c1c1, 0xfefefefe, 0xbdbdbdbd, // green
  0xfcfcfcfc, 0xffffffff, 0xbbbbbbbb, // yellow
  0x83838383, 0x83838383, 0x83838383,
  0x00000000  // Termination
};



ADiffViewSettings::ADiffViewSettings()
  : m_ColorArray(m_sColors)
{

}


ADiffViewSettings::~ADiffViewSettings()
{

}


bool ADiffViewSettings::Load()
{
  return true;
}

const ULONG* ADiffViewSettings::getColorArray() const
{
  return m_ColorArray.elem;
}


const ULONG* ADiffViewSettings::getColorRedArray() const
{
  return m_ColorArray.elem + 1;
}


const ULONG* ADiffViewSettings::getColorGreenArray() const
{
  return m_ColorArray.elem + 4;
}


const ULONG* ADiffViewSettings::getColorYellowArray() const
{
  return m_ColorArray.elem + 7;
}


const ULONG* ADiffViewSettings::getColorGrayArray() const
{
  return m_ColorArray.elem + 10;
}

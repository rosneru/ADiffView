#ifdef __clang__
  #include <clib/exec_protos.h>
  #include <clib/gadtools_protos.h>
  #include <clib/intuition_protos.h>
#else
  #include <proto/exec.h>
  #include <proto/gadtools.h>
  #include <proto/intuition.h>
#endif

#include <exec/memory.h>

#include "MenuBase.h"


MenuBase::MenuBase()
  : m_pMenu(NULL),
  m_IsLayoutDone(false)
{

}


MenuBase::~MenuBase()
{
  if(m_pMenu != NULL)
  {
    FreeMenus(m_pMenu);
    m_pMenu = NULL;
  }
}

bool MenuBase::Layout(ScreenBase& screen)
{
  if(m_IsLayoutDone)
  {
    return true;
  }

  if(screen.getGadtoolsVisualInfo() == NULL)
  {
    // No VisualInfo available to create the menu
    return false;
  }

  // Create the layout for the menu
  if(LayoutMenus(m_pMenu, screen.getGadtoolsVisualInfo(),
                 GTMN_NewLookMenus, TRUE, // Ignored before v39
                 TAG_DONE) == FALSE)
  {
    // Failed to create the layout for the menu
    return false;
  }

  m_IsLayoutDone = true;
  return true;
}



bool MenuBase::AttachToWindow(struct Window* pWindow)
{
  if(SetMenuStrip(pWindow, m_pMenu) == FALSE)
  {
    // Binding the menu to given window failed
    return false;
  }

  return true;
}




void MenuBase::DetachFromWindow(struct Window* pWindow)
{
  ClearMenuStrip(pWindow);
}

struct Menu* MenuBase::IntuiMenu()
{
  return m_pMenu;
}


void MenuBase::DisableMenuItem(struct Window* pWindow,
                               APTR pUserDatMenuBaseItemToDisable)
{
  if(pUserDatMenuBaseItemToDisable == NULL)
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(pUserDatMenuBaseItemToDisable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OffMenu(pWindow, menuNumber);
}


void MenuBase::EnableMenuItem(struct Window* pWindow,
                              APTR pUserDatMenuBaseItemToEnable)
{
  if((pWindow == NULL) || (pUserDatMenuBaseItemToEnable == NULL))
  {
    return;
  }

  WORD menuNumber = 0;
  struct MenuItem* pFoundItem = findItemByUserData(pUserDatMenuBaseItemToEnable, menuNumber);
  if(pFoundItem == NULL)
  {
    return;
  }

  OnMenu(pWindow, menuNumber);
}


struct MenuItem* MenuBase::findItemByUserData(APTR pUserDataToFind,
                                             WORD& foundMenuNumber)
{
  if(m_pMenu == NULL)
  {
    return NULL;
  }

  foundMenuNumber = 0;

  struct Menu* pMenu = m_pMenu;
  struct MenuItem* pItem = pMenu->FirstItem;
  if(pItem == NULL)
  {
    return NULL;
  }

  int iMenu = 0;
  int iItem = 0;

  do
  {
    do
    {
      APTR pUserData = GTMENUITEM_USERDATA(pItem);
      if(pUserData == pUserDataToFind)
      {
        foundMenuNumber = FULLMENUNUM(iMenu, iItem, 0);
        return pItem;
      }

      pItem = pItem->NextItem;
      iItem++;
    }
    while(pItem != NULL);

    pMenu = pMenu->NextMenu;
    if(pMenu != NULL)
    {
      pItem = pMenu->FirstItem;
      iItem = 0;
      iMenu++;
    }
  }
  while(pItem != NULL);

  return NULL;
}

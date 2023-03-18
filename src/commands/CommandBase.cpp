#ifdef __clang__
  #include <clib/intuition_protos.h>
#else
  #include <proto/intuition.h>
#endif

#include "CommandBase.h"

std::vector<WindowBase*>* CommandBase::m_pAllWindowsVector(NULL);

CommandBase::CommandBase(std::vector<WindowBase*>* pAllWindowsVector)
  : m_SleepRequester()
{
  m_pAllWindowsVector = pAllWindowsVector;
}

CommandBase::~CommandBase()
{

}


void CommandBase::EnableInAllWindowMenus() const
{
  for(size_t i = 0; i < m_pAllWindowsVector->size(); i++)
  {
    struct Window* pIntuiWindow = (*m_pAllWindowsVector)[i]->getIntuiWindow();
    MenuBase* pMenu = (*m_pAllWindowsVector)[i]->getMenu();

    if(pIntuiWindow != NULL && pMenu != NULL)
    {
      pMenu->EnableMenuItem((*m_pAllWindowsVector)[i]->getIntuiWindow(), (APTR)this);
    }
  }
}


void CommandBase::DisableInAllWindowMenus() const
{
  for(size_t i = 0; i < m_pAllWindowsVector->size(); i++)
  {
    struct Window* pIntuiWindow = (*m_pAllWindowsVector)[i]->getIntuiWindow();
    MenuBase* pMenu = (*m_pAllWindowsVector)[i]->getMenu();

    if(pIntuiWindow != NULL && pMenu != NULL)
    {
      pMenu->DisableMenuItem((*m_pAllWindowsVector)[i]->getIntuiWindow(), (APTR)this);
    }
  }
}


void CommandBase::disableBusyPointerForAllWindows()
{
  for(size_t i = 0; i < m_pAllWindowsVector->size(); i++)
  {
    struct Window* pIntuiWindow = (*m_pAllWindowsVector)[i]->getIntuiWindow();

    if(pIntuiWindow != NULL && (*m_pAllWindowsVector)[i]->isOpen())
    {
      EndRequest(&m_SleepRequester, pIntuiWindow);
      SetWindowPointer(pIntuiWindow, WA_BusyPointer, FALSE, TAG_DONE);
    }
  }
}

void CommandBase::enableBusyPointerForAllWindows()
{  
  InitRequester(&m_SleepRequester);

  for(size_t i = 0; i < m_pAllWindowsVector->size(); i++)
  {
    struct Window* pIntuiWindow = (*m_pAllWindowsVector)[i]->getIntuiWindow();

    if(pIntuiWindow != NULL)
    {
      Request(&m_SleepRequester, pIntuiWindow);
      SetWindowPointer(pIntuiWindow, WA_BusyPointer, TRUE, TAG_DONE);
    }
  }
}

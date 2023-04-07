#ifndef OPEN_CLONED_WORKBENCH_SCREEN_H
#define OPEN_CLONED_WORKBENCH_SCREEN_H

#include "OpenScreenBase.h"

class OpenClonedWorkbenchScreen : public ScreenBase
{
public:
  OpenClonedWorkbenchScreen(const ADiffViewSettings& settings,
                            const char* pTitle,
                            const char* pPubScreenName,
                            short depth);

  virtual ~OpenClonedWorkbenchScreen();

  virtual bool close();

private:
  const char* m_pTitle;
  short m_Depth;
};

#endif

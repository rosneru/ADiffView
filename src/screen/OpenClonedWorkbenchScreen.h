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

  bool Open();
  void Close();

private:
  const char* m_pTitle;
  short m_Depth;
};

#endif

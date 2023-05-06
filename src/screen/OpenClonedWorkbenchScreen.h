#ifndef OPEN_CLONED_WORKBENCH_SCREEN_H
#define OPEN_CLONED_WORKBENCH_SCREEN_H

#include <string>
#include "OpenScreenBase.h"

class OpenClonedWorkbenchScreen : public ScreenBase
{
public:
  /**
   * Open a public screen with given settings.
   *
   * \param pPubScreenName Start of the public screen name. At screen
   * opening there's a number from 1..99 attached. Get the final public
   * screen name after opening with getPubScreenName().
   */
  OpenClonedWorkbenchScreen(const char* pTitle,
                            const char* pPubScreenName,
                            short depth,
                            const ULONG* pColors32Array);

  virtual ~OpenClonedWorkbenchScreen();

  virtual bool close();

  const char* getPubScreenName() const;

private:
  std::string m_PubScreenName;
  const char* m_pTitle;
  short m_Depth;
};

#endif

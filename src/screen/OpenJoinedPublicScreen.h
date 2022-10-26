#ifndef JOINED_PUBLIC_SCREEN_H
#define JOINED_PUBLIC_SCREEN_H

#include "OpenScreenBase.h"

class OpenJoinedPublicScreen : public ScreenBase
{
public:
  OpenJoinedPublicScreen(const ADiffViewSettings& settings,
                         const char* pPubScreenName);

  virtual ~OpenJoinedPublicScreen();

};

#endif

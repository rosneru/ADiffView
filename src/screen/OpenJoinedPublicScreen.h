#ifndef JOINED_PUBLIC_SCREEN_H
#define JOINED_PUBLIC_SCREEN_H

#include "OpenScreenBase.h"

class OpenJoinedPublicScreen : public ScreenBase
{
public:
  OpenJoinedPublicScreen(const char* pPubScreenName);

  virtual bool close();

  virtual ~OpenJoinedPublicScreen();

};

#endif

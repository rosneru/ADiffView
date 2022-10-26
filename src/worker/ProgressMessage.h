#ifndef PROGRESS_MESSAGE_H
#define PROGRESS_MESSAGE_H

#include <exec/ports.h>

struct ProgressMessage : public Message
{
  long progress;
  const char *pDescription;
};


#endif

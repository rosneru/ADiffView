#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <intuition/intuition.h>


/**
 * Displays a requester with a message to the user.
 *
 * @author Uwe Rosner
 * @date 10/02/2019
 */
class MessageBox
{
public:
  /**
   * Creates a Requester on given window. If window is NULL the
   * requester is created on the Workbench.
   */
  MessageBox(struct Window* pWindow = NULL);
  virtual ~MessageBox();

  LONG Show(const char* pWindowTitle,
            const char* pMessage,
            const char* pButtonText);


  LONG Show(const char* pMessage,
            const char* pButtonText);

private:
    struct Window* m_pWindow;
    struct EasyStruct m_EasyStruct;
};

#endif

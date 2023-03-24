#ifdef __clang__
  #include <clib/intuition_protos.h>
#else
  #include <proto/intuition.h>
#endif

#include "MessageBox.h"

MessageBox::MessageBox(struct Window* pWindow)
  : m_pWindow(pWindow)
{

  m_EasyStruct.es_StructSize = sizeof(EasyStruct);
  m_EasyStruct.es_Flags = 0;
  m_EasyStruct.es_Title = NULL;
}

MessageBox::~MessageBox()
{

}

LONG MessageBox::Show(const char* pWindowTitle,
                      const char* pMessage,
                      const char* pButtonText)
{
  m_EasyStruct.es_Title = (UBYTE*) pWindowTitle;
  m_EasyStruct.es_TextFormat = (UBYTE*)pMessage;
  m_EasyStruct.es_GadgetFormat = (UBYTE*)pButtonText;

  return EasyRequest(m_pWindow, &m_EasyStruct, NULL);
}


LONG MessageBox::Show(const char* pMessage,
                      const char* pButtonText)
{
  m_EasyStruct.es_Title = NULL;
  m_EasyStruct.es_TextFormat = (UBYTE*)pMessage;
  m_EasyStruct.es_GadgetFormat = (UBYTE*)pButtonText;

  return EasyRequest(m_pWindow, &m_EasyStruct, NULL);
}

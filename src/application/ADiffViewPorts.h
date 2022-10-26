#ifndef ADIFFVIEW_PORTS_H
#define ADIFFVIEW_PORTS_H

#include <exec/ports.h>

/**
 * Class for all MessagePorts needed by ADiffView.
 *
 * @author Uwe Rosner
 * @date 04/08/2020
 */
class ADiffViewPorts
{
public:
  ADiffViewPorts();
  virtual ~ADiffViewPorts();

  struct MsgPort* Idcmp() const;
  struct MsgPort* Progress() const;
  struct MsgPort* Workbench() const;

private:
  struct MsgPort* m_pMsgPortIDCMP;
  struct MsgPort* m_pMsgPortProgress;
  struct MsgPort* m_pMsgPortWorkbench;
};

#endif

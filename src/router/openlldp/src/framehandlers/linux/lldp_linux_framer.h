#ifndef __LLDP_LINUX_FRAMER_H__
#define __LLDP_LINUX_FRAMER_H__
#include "lldp_port.h"

int socketInitializeLLDP();
void socketCleanupLLDP();
static int _getmac(struct lldp_port *lldp_port);
static int _getip(struct lldp_port *lldp_port);
int refreshInterfaceData(struct lldp_port *lldp_port);

#endif /* __LLDP_LINUX_FRAMER_H__ */

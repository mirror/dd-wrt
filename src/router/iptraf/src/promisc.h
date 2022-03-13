#ifndef IPTRAF_NG_PROMISC_H
#define IPTRAF_NG_PROMISC_H

#include "list.h"

void promisc_enable(int sock, struct list_head *promisc, const char *device_name);
void promisc_disable(int sock, struct list_head *promisc);

#endif	/* IPTRAF_NG_PROMISC_H */

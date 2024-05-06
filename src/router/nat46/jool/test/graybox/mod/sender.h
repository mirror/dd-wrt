#ifndef _GRAYBOX_MOD_SENDER_H
#define _GRAYBOX_MOD_SENDER_H

#include <linux/types.h>

int sender_send(char *pkt_name, void *pkt, size_t pkt_len);

#endif

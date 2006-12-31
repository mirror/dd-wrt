/* arp.h - ARP state machine */
 
/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef ARP_H
#define ARP_H

#include <stdint.h>
#include <linux/atmarp.h>

#include "atmd.h"
#include "atmarp.h"
#include "atmarpd.h"
#include "table.h"


void discard_vccs(ENTRY *entry);
void discard_entry(ENTRY *entry);
void vcc_detach(ENTRY *entry);
void need_ip(int itf_num,uint32_t ip);
void query_ip(const UN_CTX *ctx,uint32_t ip);
void incoming_arp(VCC *vcc,struct atmarphdr *hdr,int len);
int arp_ioctl(struct atmarp_req *req);
void vcc_connected(VCC *vcc);
void vcc_failed(VCC *vcc);
void disconnect_vcc(VCC *vcc);
void incoming_call(VCC *vcc);
void incoming_unidirectional(VCC *vcc);

#endif

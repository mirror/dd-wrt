#ifndef __RFLOW_H__
#define __RFLOW_H__

#include "headers.h"
#include "psrc.h"
#include "psrc-pcap.h"

#define RFLOW_VERSION_STRING "1.00"
#define RFLOW_COPYRIGHT " Copyright by Nikki Chumakov\nbased on ipcapd code by Lev Walkin vlm@lionet.info"

int reopen_packet_source_ring(packet_source_t *psrc, int loop);
int process_packet_sources(packet_source_t *sources);

void process_packet_data(packet_source_t *ps,
      const unsigned char *packet, int caplen);

int if_stat(FILE *, char *iface);
void system_uptime(FILE *); /* Print the uptime */

void rflow_usage(void);

int cfgread(const char *);

extern double self_started;

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

int ifst_preopen(void); /* Pre-open files inaccessible after chroot() */

int display_internal_averages(FILE *, const char *ifname);

#endif /* __RFLOW_H__ */

#ifndef	__PSRC_RING__
#define	__PSRC_RING__

#include "ring.h"

int init_packet_source_ring(packet_source_t *ps);

void *process_ring(void *);

void print_stats_ring(FILE *, packet_source_t *ps);

#endif	/* __PSRC_RING__ */

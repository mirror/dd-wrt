#ifndef	__PSRC_DYNAMIC__
#define	__PSRC_DYNAMIC__

int init_packet_source_dynamic(packet_source_t *ps);

void *process_dynamic(void *);

void print_stats_dynamic(FILE *, packet_source_t *ps);

#endif	/* __PSRC_DYNAMIC__ */

#ifndef	__PSRC_PCAP__
#define	__PSRC_PCAP__

int init_packet_source_pcap(packet_source_t *ps);

void *process_pcap(void *);

void print_stats_pcap(FILE *, packet_source_t *ps);

#endif	/* __PSRC_PCAP__ */

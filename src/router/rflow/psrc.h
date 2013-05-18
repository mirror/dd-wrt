/*-
 * import from ipcapd
 * $Id: psrc.h,v 1.8 2004/05/12 05:16:52 vlm Exp $
 */
#ifndef	__PSRC_H__
#define	__PSRC_H__

#include "headers.h"
#include "genhash.h"
// #include "psrc-ring.h"

typedef struct packet_source_s {

	enum {
		IFACE_UNKNOWN,
	        IFACE_DYNAMIC,
	        IFACE_PCAP,
	} iface_type;
		
	enum {
		PST_INVALID,	/* Not initialized */
		PST_EMBRYONIC,	/* Interface name is set */
		PST_READY,	/* Ready for operation */
	} state;

	/*
	 * Interface properties.
	 */
	char	ifName[IFNAMSIZ];
	int	ifIndex;	/* Assigned internal (SNMP) index */

	unsigned int dlt;	/* Data Link Type */
	char *custom_filter;	/* pcap filter */
	enum {
		IFLAG_NONE	= 0x00,
		IFLAG_INONLY	= 0x01,	/* Incoming only */
		IFLAG_PROMISC	= 0x02,	/* Enable promiscuous mode */
		IFLAG_NF_SAMPLED= 0x04,	/* Enable NetFlow sampling mode */
		IFLAG_NF_DISABLE= 0x08,	/* Disable NetFlow */
		IFLAG_RSH_EXTRA	= 0x10,	/* Enable RSH extra info */
		IFLAG_LARGE_CAP = 0x20,	/* Large capture length (BPF/PCAP) */
		IFLAGS_STRICT	= 0x80,	/* Do not alter flags (internal) */
	} iflags;

	/*
	 * Declare source-dependent interface descriptors.
	 */

	union {
#ifdef	PSRC_pcap
		struct {
			pcap_t *dev;
			pthread_mutex_t dev_mutex;
		} pcap;
#endif
    
		struct {
			genhash_t		*already_got;
		} dynamic;
	} iface;

	int	fd;	/* File descriptor, if applicable */
	char	*buf;
	size_t	bufsize;

	void	*(*process_ptr)(void *);  /* pointer to capture function */
	void	(*print_stats)(FILE *, struct packet_source_s *);

	/*
	 * Run-time variables.
	 */
	pthread_t 	thid;

	/*
	 * Statistics
	 */
	long long bytes_prev;    /* Bytes per previous second, estimated */
	long long bytes_cur;     /* Current bytecount */
	long long packets_prev;  /* Packets per previous second, est. */
	long long packets_cur;   /* Current packets count */

	long long bytes_lp;	/* Bytes per long period */
	long long bps_lp;	/* Bytes per second per long period */

	long long packets_lp;	/* Packets per long period */
	long long pps_lp;	/* Bytes per second per long period */

	double avg_period;	/* Averaging period, seconds */

	unsigned int sample_count;	/* NetFlow sampling-mode counter */

	/*
	 * Internal stuff.
	 */
	struct packet_source_s *next;
} packet_source_t;

packet_source_t *create_packet_source(char *ifname, int iflags, char *filter);
int init_packet_source(packet_source_t *, int retry_mode);
void destroy_packet_source(packet_source_t *ps);

const char *IFNameBySource(void *psrc);

#endif	/* __PSRC_H__ */

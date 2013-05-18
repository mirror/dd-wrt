#include "rflow.h"
// #include "sf_lite.h"
#include "opt.h"
#include "psrc.h"
#include "psrc-pcap.h"

static int ifIndexes;	/* Ever-increasing interface name */

packet_source_t *
create_packet_source(char *ifname, int iflags, char *filter) {
	packet_source_t *ps;

	/*
	 * Create a new interface structure.
	 */
	ps = calloc(1, sizeof *ps);
	if(ps == NULL) return NULL;

	ps->fd = -1;	/* Packet source file descriptor */

	/*
	 * Fill-up the common parameters.
	 */
	strncpy( ps->ifName, ifname, sizeof(ps->ifName) );
	ps->ifName[sizeof(ps->ifName) - 1] = 0;
	ps->iflags = iflags;
	ps->custom_filter = filter;

	ps->avg_period = 300;   /* 5 minutes */
	ps->bytes_lp = -1;	/* Initial value */
	ps->packets_lp = -1;	/* Initial value */

	ps->state = PST_EMBRYONIC;


	/*
	 * Initialize packet source descriptors.
	 */
#if 0
	if(strchr(ifname, '*')) {
		/*
		 * The interface does not exist, so it might be dynamic.
		 */
		fprintf(stderr, "[DYNAMIC] ");
		ps->process_ptr = process_dynamic;
		ps->print_stats = print_stats_dynamic;
		ps->iface_type = IFACE_DYNAMIC;
		ps->iface.dynamic.already_got = genhash_new(cmpf_string,
			hashf_string,
			NULL, (void (*)(void *))destroy_packet_source);
		if(ps->iface.dynamic.already_got == NULL)
			goto failure;
	} 
  else 
#endif
  {
		ps->process_ptr = process_pcap;
		ps->print_stats = print_stats_pcap;

    		ps->iface_type = IFACE_PCAP;
		ps->iface.pcap.dev = NULL;
		if(pthread_mutex_init(&ps->iface.pcap.dev_mutex, NULL) == -1)
			goto failure;
		
	}

	ps->ifIndex = ++ifIndexes;

	return ps;

failure:
	free(ps);
	return NULL;
}


int
init_packet_source(packet_source_t *ps, int retry_mode) {

	return init_packet_source_pcap(ps);
}

void
destroy_packet_source(packet_source_t *ps) {
	if(ps == NULL) return;

	if(ps->buf)
		free(ps->buf);

	if(ps->fd != -1)
		close(ps->fd);

	if(ps->iface.pcap.dev)
		pcap_close(ps->iface.pcap.dev);
	pthread_mutex_destroy(&ps->iface.pcap.dev_mutex);

	free(ps);
}

const char *
IFNameBySource(void *psrc) {
	packet_source_t *ps = psrc;
	if(ps) {
		switch(ps->iface_type) {
		default:
			return ps->ifName;
		}
	} else {
		return "<?>";
	}
}

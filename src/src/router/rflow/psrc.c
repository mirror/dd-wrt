#include "rflow.h"
// #include "sf_lite.h"
#include "opt.h"
#include "psrc.h"
#include "psrc-ring.h"

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
		ps->process_ptr = process_ring;
		ps->print_stats = print_stats_ring;
		
    		ps->iface_type = IFACE_RING;
		ps->iface.ring.dev = NULL;
		if(pthread_mutex_init(&ps->iface.ring.dev_mutex, NULL) == -1)
			goto failure;
	}

	switch(ps->iface_type) {
	case IFACE_DYNAMIC:
		break;
	default:
		ps->ifIndex = ++ifIndexes;
	}

	return ps;

failure:
	free(ps);
	return NULL;
}


int
init_packet_source(packet_source_t *ps, int retry_mode) {

	switch(ps->iface_type) {
#if 0
	case IFACE_DYNAMIC:
		return init_packet_source_dynamic(ps);
#endif
	default:
		return init_packet_source_ring(ps);
	}
}

void
destroy_packet_source(packet_source_t *ps) {
	if(ps == NULL) return;

	if(ps->buf)
		free(ps->buf);

	if(ps->fd != -1)
		close(ps->fd);

	switch(ps->iface_type) {
#if 0
	case IFACE_DYNAMIC:
		genhash_destroy(ps->iface.dynamic.already_got);
		break;
#endif
	case IFACE_RING:
		if(ps->iface.ring.dev)
			ring_close(ps->iface.ring.dev);
		pthread_mutex_destroy(&ps->iface.ring.dev_mutex);
		break;
	default:
		break;
	}

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

#include "rflow.h"
#include "cfgvar.h"
#include "opt.h"
#include "psrc.h"
#include "psrc-ring.h"
#include "pcap-bpf.h"

#define RING_ERRBUF_SIZE 1024

// #warning "Ignore sent" interface feature is not supported by RING. Minor warning.

static int apply_filter(ring_t *dev, char *filter);

int
reopen_packet_source_ring(packet_source_t *ps, int loop) {
	char errbuf[RING_ERRBUF_SIZE];
	ring_t *dev;
	int snaplen;

	assert(ps->state != PST_INVALID);	/* Embryonic or Ready */
	assert(ps->iface_type == IFACE_RING);	/* Don't cook crap */

	pthread_mutex_lock(&ps->iface.ring.dev_mutex);
	ps->state = PST_EMBRYONIC;
	if(ps->iface.ring.dev) {
		ring_close(ps->iface.ring.dev);
		ps->iface.ring.dev = NULL;
	}
	pthread_mutex_unlock(&ps->iface.ring.dev_mutex);

	/* Prepare capture length value */
	if((ps->iflags & IFLAG_LARGE_CAP)) {
		snaplen = 96;
	} else {
		snaplen = 68;
	}

	while(1) {
		dev = ring_open(ps->ifName, snaplen,
			(ps->iflags & IFLAG_PROMISC)?1:0, errbuf);
		if(dev == NULL) {
			if(loop) {
				sleep(1);
				continue;
			} else {
				fprintf(stderr, "[%s] ", errbuf);
				return -1;
			}
		}

		/* Get device type */
		ps->dlt = ring_datalink(dev);

fprintf (stderr, "apply filter...\n");
		if( apply_filter(dev, ps->custom_filter) ) {
			ring_close(dev);

			errno = ENODEV;
			if(loop) {
				sleep(10);
				continue;
			} else {
				fprintf(stderr,
					"[Warning: Can't initialize filter!] ");
				return -1;
			}
		} else {
			ps->iface.ring.dev = dev;
			ps->state = PST_READY;
			return 0;
		}
	}
}

int
init_packet_source_ring(packet_source_t *ps) {

  if(ps->iface_type != IFACE_RING) return -1;

	if(ps->iflags & IFLAG_INONLY) {
		printf("%s: Input-only feature not supported by RING.\n",
			ps->ifName);
		errno = EPERM;
		return -1;
	}

	if(reopen_packet_source_ring(ps, 0))
		return -1;

	/* Complain about unknown devices. */
	switch(ps->dlt) {
#ifdef	DLT_LOOP
	case DLT_LOOP:
#endif	/* DLT_LOOP */
	case DLT_NULL:    /* Loopback */
	case DLT_RAW:     /* Some PPP implementations, etc. */
	case DLT_EN10MB:  /* Generic Ethernet-compatible */
	case DLT_PPP:     /* Point-to point interface */
#ifdef	DLT_C_HDLC
	case DLT_C_HDLC:  /* BSD/OS Cisco HDLC */
#endif
	case DLT_IEEE802: /* Token Ring */
#ifdef	DLT_LINUX_SLL
	case DLT_LINUX_SLL:	/* fake header for Linux cooked socket */
#endif
		break;
	default:
		fprintf(stderr, "[Unknown interface type] ");
		ring_close(ps->iface.ring.dev);
		ps->iface.ring.dev = NULL;
		errno = ENODEV;
		return -1;
	};

	return 0;
}

static int
apply_filter(ring_t *dev, char *filter) {
	struct bpf_program fp;

	if(filter == NULL)
		filter = "ip";
#if 0
	if( pcap_compile(dev, &fp, filter, 1, -1) )
		return -1;

	if( pcap_setfilter(dev, &fp) )
		return -1;
#endif
	return 0;
}



void
print_stats_ring(FILE *f, packet_source_t *ps) {
	struct ring_stat pstat;
	int ret;

	assert(ps->iface_type == IFACE_RING);

	pthread_mutex_lock(&ps->iface.ring.dev_mutex);
	if(ps->state == PST_EMBRYONIC) {
		fprintf(f,	"Interface %s: DOWN\n", ps->ifName);
		pthread_mutex_unlock(&ps->iface.ring.dev_mutex);
		return;
	}
	ret = ring_stats(ps->iface.ring.dev, &pstat);
	pthread_mutex_unlock(&ps->iface.ring.dev_mutex);

	if(ret) {
		fprintf(f, "Interface %s: %s\n",
			ps->ifName, strerror(errno));
		return;
	}

	fprintf(f, "Interface %s: received %u", ps->ifName, pstat.ps_recv);

	fprintf(f, ", %.0f m average %lld bytes/sec, %lld pkts/sec",
		ps->avg_period / 60,
		ps->bps_lp,
		ps->pps_lp
	);

	if(pstat.ps_ifdrop)
		fprintf(f, ", dropped %u", pstat.ps_drop);

	fprintf(f, "\n");
}

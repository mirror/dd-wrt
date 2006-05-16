#include "rflow.h"
#include "opt.h"
#include "psrc-ring.h"

#include "ring.h"

static void
ring_callback(void *psp, const struct ring_pkthdr *ph, const unsigned char *packet) {
	return process_packet_data((packet_source_t *)psp,
			packet, ph->caplen);
}

void *
process_ring(void *psp) {
	packet_source_t *ps = psp;
	int pdr;
	struct pollfd pfd;
	sigset_t set, oset;

	assert(ps->iface_type == IFACE_RING);
	
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	
	while(1) {

		if(signoff_now)
			break;

		if(ps->state != PST_READY || ps->iface.ring.dev == NULL) {
			int ret;
			sigprocmask(SIG_UNBLOCK, &set, &oset);
			ret = reopen_packet_source_ring(ps, 1);
			if(ret) {
				sleep(1);
				continue;
			}
			sigprocmask(SIG_SETMASK, &oset, NULL);
			assert(ps->state == PST_READY);
		}

// fprintf (stderr, "process ring_read_packet\n");
		sigprocmask(SIG_UNBLOCK, &set, &oset);
		pdr = ring_read_packet(ps->iface.ring.dev, ring_callback, (void *)ps);
		sigprocmask(SIG_SETMASK, &oset, NULL);

		/* Timeout */
		if(pdr == 0) {
#if 0
			pfd.fd = ring_fileno(ps->iface.ring.dev);
			pfd.events = POLLIN;
			sigprocmask(SIG_UNBLOCK, &set, &oset);
			poll(&pfd, 1, -1);
			sigprocmask(SIG_SETMASK, &oset, NULL);
#endif
			continue;
		}

		/* Device error */
		if(pdr == -1) {
			/* Request to re-initialize */
			ps->state = PST_EMBRYONIC;
			sleep(1);	/* Reinitialize after a timeout */
			continue;
		}

	}

	return NULL;
}

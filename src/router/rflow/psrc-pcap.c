/*-
 * Copyright (c) 2001, 2002 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: psrc-pcap.c,v 1.13 2004/07/27 09:49:00 vlm Exp $
 */

#include "rflow.h"
#include "cfgvar.h"
#include "opt.h"


#warning "Ignore sent" interface feature is not supported by PCAP. Minor warning.

static int apply_filter(pcap_t *dev, char *filter);

int
reopen_packet_source_pcap(packet_source_t *ps, int loop) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *dev;
	int snaplen;

	assert(ps->state != PST_INVALID);	/* Embryonic or Ready */
	assert(ps->iface_type == IFACE_PCAP);	/* Don't cook crap */

	pthread_mutex_lock(&ps->iface.pcap.dev_mutex);
	ps->state = PST_EMBRYONIC;
	if(ps->iface.pcap.dev) {
		pcap_close(ps->iface.pcap.dev);
		ps->iface.pcap.dev = NULL;
	}
	pthread_mutex_unlock(&ps->iface.pcap.dev_mutex);

	/* Prepare capture length value */
	if((ps->iflags & IFLAG_LARGE_CAP)) {
		snaplen = 96;
	} else {
		snaplen = 68;
	}

	for(;;) {
		dev = pcap_open_live(IFNameBySource(ps), snaplen,
			(ps->iflags & IFLAG_PROMISC)?1:0, 500, errbuf);
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
		ps->dlt = pcap_datalink(dev);

		if( apply_filter(dev, ps->custom_filter) ) {
			pcap_close(dev);

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
			ps->iface.pcap.dev = dev;
			ps->state = PST_READY;
			return 0;
		}
	}
}

int
init_packet_source_pcap(packet_source_t *ps) {

        if(ps->iface_type != IFACE_PCAP) return -1;

	if(ps->iflags & IFLAG_INONLY) {
		printf("%s: Input-only feature not supported by PCAP.\n",
			IFNameBySource(ps));
		errno = EPERM;
		return -1;
	}

	if(reopen_packet_source_pcap(ps, 0))
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
		pcap_close(ps->iface.pcap.dev);
		ps->iface.pcap.dev = NULL;
		errno = ENODEV;
		return -1;
	};

	return 0;
}

static int
apply_filter(pcap_t *dev, char *filter) {
	struct bpf_program fp;

	if(filter == NULL)
		filter = "ip";

	if( pcap_compile(dev, &fp, filter, 1, -1) )
		return -1;

	if( pcap_setfilter(dev, &fp) )
		return -1;

	return 0;
}



void
print_stats_pcap(FILE *f, packet_source_t *ps) {
	struct pcap_stat pstat;
	int ret;

	assert(ps->iface_type == IFACE_PCAP);

	pthread_mutex_lock(&ps->iface.pcap.dev_mutex);
	if(ps->state == PST_EMBRYONIC) {
		fprintf(f,	"Interface %s: DOWN\n", IFNameBySource(ps));
		pthread_mutex_unlock(&ps->iface.pcap.dev_mutex);
		return;
	}
	ret = pcap_stats(ps->iface.pcap.dev, &pstat);
	pthread_mutex_unlock(&ps->iface.pcap.dev_mutex);

	if(ret) {
		fprintf(f, "Interface %s: %s\n",
			IFNameBySource(ps), strerror(errno));
		return;
	}

	fprintf(f, "Interface %s: received %u",
		IFNameBySource(ps), pstat.ps_recv);

	fprintf(f, ", %.0f m average %lld bytes/sec, %lld pkts/sec",
		ps->avg_period / 60,
		ps->bps_lp,
		ps->pps_lp
	);

	if(pstat.ps_ifdrop)
		fprintf(f, ", dropped %u", pstat.ps_drop);

	fprintf(f, "\n");
}

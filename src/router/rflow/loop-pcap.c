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
 * $Id: loop-pcap.c,v 1.8 2004/09/17 05:37:58 vlm Exp $
 */

#include "rflow.h"
#include "opt.h"

static void
pcap_callback(unsigned char *psp, const struct pcap_pkthdr *ph, const unsigned char *packet) {
	return process_packet_data((packet_source_t *)psp,
			packet, ph->caplen);
}

int
reopen_packet_source_pcap(packet_source_t *ps, int loop);

void *
process_pcap(void *psp) {
	packet_source_t *ps = psp;
	int pdr;
	struct pollfd pfd;

	assert(ps->iface_type == IFACE_PCAP);
	
	for(;;) {

		if(signoff_now)
			break;

		if(ps->state != PST_READY || ps->iface.pcap.dev == NULL) {
			int ret;
			ret = reopen_packet_source_pcap(ps, 1);
			if(ret) {
				sleep(1);
				continue;
			}
			assert(ps->state == PST_READY);
		}

		pdr = pcap_dispatch(ps->iface.pcap.dev, 0,
			pcap_callback, (unsigned char *)ps);

		/* Timeout */
		if(pdr == 0) {
			pfd.fd = pcap_fileno(ps->iface.pcap.dev);
			pfd.events = POLLIN;
			poll(&pfd, 1, -1);
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

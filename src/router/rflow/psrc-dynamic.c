/*-
 * Copyright (c) 2004 Lev Walkin <vlm@lionet.info>.
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
 * $Id: psrc-dynamic.c,v 1.2 2004/07/27 09:49:00 vlm Exp $
 */

#include "rflow.h"
#include "opt.h"
#include "cfgvar.h"
#include <assert.h>

int
init_packet_source_dynamic(packet_source_t *ps) {
	int fd = -1;

	assert(ps->state != PST_INVALID);	/* Embryonic or Ready */
	assert(ps->iface_type == IFACE_DYNAMIC);	/* Don't cook crap */

	/*
	 * Open routing socket.
	 */
	fd = ps->fd;
	if(fd != -1) {
		ps->state = PST_READY;
		return 0;
	} else {
		ps->state = PST_EMBRYONIC;
	}

#ifdef	PF_ROUTE
	fd = socket(PF_ROUTE, SOCK_RAW, AF_INET);
#endif
	if(fd == -1) {
		fd = open("/proc/net/dev", O_RDONLY);
	}
	if(fd == -1)
		return -1;
	
	/*
	 * Finish initialization of the structure.
	 */
	ps->fd = fd;
	ps->state = PST_READY;

	return 0;
}

void
print_stats_dynamic(FILE *f, packet_source_t *ps) {

	assert(ps->iface_type == IFACE_DYNAMIC);

	fprintf(f, "Interface %s: dynamic, forked %d\n",
		IFNameBySource(ps), genhash_count(ps->iface.dynamic.already_got));
}


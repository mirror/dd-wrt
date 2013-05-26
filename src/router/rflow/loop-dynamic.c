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
 * $Id: loop-dynamic.c,v 1.6 2004/09/17 05:37:58 vlm Exp $
 */

#include "rflow.h"
#include "cfgvar.h"
#include "ifs_list.h"
#include "opt.h"

/*
 * Try to open interfaces matching wildcard.
 */
static void interface_wildcard_check(packet_source_t *ps);
static void open_clone(packet_source_t *ps, char *ifname);

void *
process_dynamic(void *psp) {
#ifdef	IPCAD_DYNAMIC_ROUTING_SOCKET
	struct rt_msghdr *rtm;
	char buf[(sizeof(*rtm) + 63) & ~63];
	ssize_t skip_unknown = 0;
	ssize_t len = 0;
	int ret;
#endif
	packet_source_t *ps = psp;
	int fd = ps->fd;
	struct pollfd pfd;

	assert(ps->iface_type == IFACE_DYNAMIC);

	/*
	 * Open everything which matches wildcard.
	 */
	interface_wildcard_check(ps);

	pfd.fd = fd;
	pfd.events = POLLIN;

	for(;;) {

		if(signoff_now)
			break;

#ifdef	IPCAD_DYNAMIC_ROUTING_SOCKET
		ret = poll(&pfd, 1, 1000);
		switch(ret) {
		case 1:
			if(pfd.revents & POLLIN)
				break;
			/* Fall through */
		case -1:
		default:
			continue;
		case 0:
			goto check;
		}
		len = read(fd, buf,
			skip_unknown ? MIN(skip_unknown, (int)sizeof(buf)) : 4);
		assert(len >= 0);
		if(skip_unknown) {
			assert(len <= (int)skip_unknown);
			skip_unknown -= len;
			continue;
		}
		/* This is local host, for socket's sake! */
		assert(len == 4);
		rtm = (void *)buf;
		skip_unknown = rtm->rtm_msglen - len;
		if(skip_unknown + len > (int)sizeof(buf))
			continue;	/* Too invalid message */
		if(skip_unknown <= 0) {
			fprintf(stderr, "Disabling permanently\n");
			break;
		}
		if(rtm->rtm_version != RTM_VERSION) {
			fprintf(stderr, "Cannot decode version %d "
				"routing messages, please recompile ipcad\n",
				rtm->rtm_version);
			continue;
		}
		/* Looking for interface announcements */
		if(rtm->rtm_type != RTM_NEWADDR
		&& rtm->rtm_type != RTM_ADD) {
			continue;
		}

check:
#else
		sleep(1);
#endif

		interface_wildcard_check(ps);
	}

	return 0;
}

static void
interface_wildcard_check(packet_source_t *ps) {
	slist *ifaces = get_interface_names();
	size_t i;

	if(ifaces == NULL || ifaces->count == 0) {
		sfree(ifaces);
		return;
	}

	for(i = 0; i < ifaces->count; i++) {
		if(genhash_get(ps->iface.dynamic.already_got,
				ifaces->list[i])) {
			/*
			 * This interface is already initialized by me.
			 */
			continue;
		}
		if(fnmatch(ps->ifName, ifaces->list[i], 0) == 0) {
			open_clone(ps, ifaces->list[i]);
		}
	}

	sfree(ifaces);
}

static void
open_clone(packet_source_t *ps, char *ifname) {
	packet_source_t *nps;

	nps = cfg_add_iface(ifname,
			ps->iflags | IFLAGS_STRICT,
			ps->custom_filter);
	if(nps == NULL) {
		return;
	}

	if(genhash_add(ps->iface.dynamic.already_got, nps->ifName, ps)) {
		destroy_packet_source(nps);
		return;
	}

	if(pthread_create(&nps->thid, NULL, nps->process_ptr, nps)) {
		/* Abandon addition to hash. Will do next time */
		/* genhash_del() implies destroy_packet_source()! */
		genhash_del(ps->iface.dynamic.already_got, nps->ifName);
	}
}


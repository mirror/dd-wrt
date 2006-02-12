/*-
 * Copyright (c) 2001, 2002, 2003 Lev Walkin <vlm@lionet.info>.
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
 * $Id: cfgvar.c,v 1.33 2004/05/06 15:53:29 vlm Exp $
 */

#include "rflow.h"
// #include "sf_lite.h"
#include "cfgvar.h"
#include "disp.h"
#include "opt.h"


/*
 * Run-time global variables
 */
int agr_portmap[65536];	/* Ports aggregation table */
config_t static_config;
config_t *conf = &static_config;

/*
 * Configure and initialize the specified packet source (interface).
 */
packet_source_t *
cfg_add_iface(char *iface, int iflags, char *filter) {
	packet_source_t *ps;
	int ret;

	fprintf(stderr, "Opening %s... ", iface);

	if(!(iflags & IFLAGS_STRICT) && conf->capture_ports)
		/* Inherit current capture_ports setting */
		iflags |= IFLAG_RSH_EXTRA | IFLAG_LARGE_CAP;
	if(!(iflags & IFLAG_NF_DISABLE))
		iflags |= IFLAG_LARGE_CAP;

	if((iflags & IFLAG_LARGE_CAP)) fprintf(stderr, "[LCap] ");
	if((iflags & IFLAG_RSH_EXTRA)) fprintf(stderr, "[ERSH] ");
	if((iflags & IFLAG_NF_DISABLE)) fprintf(stderr, "[!NF] ");

	/*
	 * Create a structure describing the packet source.
	 */
	ps = create_packet_source(iface, iflags, filter);
	if(ps == NULL) {
		perror("Can't construct");
		return NULL;
	}

	/*
	 * Initialize the packet source.
	 */
	ret = init_packet_source(ps, 0);
	if(ret == -1) {
		switch(errno) {
		case ENETDOWN:
			/*
			 * Use this device even if it is DOWN now.
			 */
			fprintf(stderr, "[DOWN, yet available] ");
			break;
		case ENODEV:
		case ENXIO:
			/*
			 * Use this device even if it is not configured now.
			 */
			fprintf(stderr, "[NODEV, yet configured] ");
			break;
		default:
			perror("Can't initialize");
			destroy_packet_source(ps);
			return NULL;
		}
		assert(ps->state == PST_EMBRYONIC);
	}

	/*
	 * Add the packet source into the list.
	 */
	pthread_mutex_lock(&packet_sources_list_lock);
	ps->next = conf->packet_sources_head;
	conf->packet_sources_head = ps;
	pthread_mutex_unlock(&packet_sources_list_lock);

	fprintf(stderr, "Initialized as %d\n", ps->ifIndex);

	return ps;
}

struct rsh_entries {
	char *username;
	unsigned int addr;
	int privlevel;
	struct rsh_entries *next;
};

static struct rsh_entries *rshes = NULL;

int
cfg_add_rsh_host(char *ru, char *rh, int privlevel) {
	struct  in_addr ia;
	struct rsh_entries *res;
	
	if(!ru || !rh || inet_pton(AF_INET, rh, &ia) == -1) {
		fprintf(stderr,
			"Invalid [user@]<host-ip> specification: %s\n", rh);
		exit(EX_DATAERR);
	}

	if((res = rshes)) {
		while(res->next)
			res = res->next;
		res = res->next = (struct rsh_entries *)malloc(sizeof(*res));
	} else {
		res = rshes = (struct rsh_entries *)malloc(sizeof(*res));
	}

	if(!res) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EX_OSERR);
	}

	res->username = strdup(ru);
	res->addr = ia.s_addr;
	res->privlevel = privlevel;
	res->next = NULL;

	if(!res->username) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EX_OSERR);
	}

	return 0;
}

/* 0: None, 1: view only, 2: default, 3: admin  */
int
cfg_check_rsh(char *ru, struct in_addr *ia) {
	struct rsh_entries *res;

	if(!rshes)	/* Allow world readable mode if no rules defined. */
		return 1;

	for(res = rshes; res; res=res->next) {
		if(res->addr != ia->s_addr)
			continue;
		if(!*res->username)
			return res->privlevel;
		if(strcmp(res->username, ru) == 0)
			return res->privlevel;
	}

	return 0;
}

int
cfg_add_aggregate_ports_table(int from, int to, int into) {
	int i;

	if(from < 0 || from > 65535
	|| to < 0 || to > 65535) {
		fprintf(stderr, "%d..%d->%d: Values out of range 0..65535\n",
			from, to, into);
		errno = ERANGE;
		return -1;
	}

	if(to < from) {
		int tmp = from;
		from = to;
		to = tmp;
	}

	for(i = from; i <= to; i++)
		agr_portmap[i] = into;

	printf("Aggregate ports %d..%d into %d\n",
		from, to, into);

	return 0;
}


int
cfg_add_atable2(unsigned int ip, unsigned int mask, unsigned int strip) {
	struct atable *at;

	if(!(at = conf->atable)) {
		at = conf->atable = malloc(sizeof *at);
	} else {
		while(at->next)
			at = at->next;
		at = at->next = malloc(sizeof *at);
	}

	if(!at) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EX_OSERR);
	}

	at->ip.s_addr = ip;
	at->mask.s_addr = mask;
	at->strip.s_addr = strip;
	at->next = NULL;

	if(strip == (unsigned int)-1) {
		at->strip_bits = 32;
	} else {
		u_int32_t __n = strip;
		__n = (__n & 0x55555555) + ((__n & 0xaaaaaaaa) >> 1);
		__n = (__n & 0x33333333) + ((__n & 0xcccccccc) >> 2);
		__n = (__n & 0x0f0f0f0f) + ((__n & 0xf0f0f0f0) >> 4);
		__n = (__n & 0x00ff00ff) + ((__n & 0xff00ff00) >> 8);
		__n = (__n & 0x0000ffff) + ((__n & 0xffff0000) >> 16);
		at->strip_bits = __n;
	}

	printf("Aggregate network ");
	print_ip(stdout, at->ip);
	printf("/");
	print_ip(stdout, at->mask);
	printf(" -> ");
	print_ip(stdout, at->strip);
	printf("\n");

	return 0;
}

int
cfg_add_atable(char *sip, char *smask, char *sstrip) {
	unsigned int ip;
	struct in_addr  mask;
	struct in_addr strip;
	unsigned long t;

	t = strchr(sstrip, '.')?1:0;
	if(t && inet_pton (AF_INET, sstrip, &strip)<0) return -1;
	if(!t) {
		errno=0;
		t = strtoul(sstrip, NULL, 10);
		if((t > 32) || errno) return -1;
		if(!t) strip.s_addr = 0;
		else strip.s_addr = htonl(0xffffffff << (32 - t));
	}

	if(inet_pton(AF_INET, sip, &ip) < 0)
		return -1;

	t = strchr(smask, '.')?1:0;
	if(t && inet_pton(AF_INET, smask, &mask) < 0) return -1;
	if(!t) {
		errno=0;
		t = strtoul(smask, NULL, 10);
		if((t > 32) || errno) return -1;
		if(!t) mask.s_addr = 0;
		else mask.s_addr = htonl(0xffffffff << (32 - t));
	}

	return cfg_add_atable2(ip, mask.s_addr, strip.s_addr);
}


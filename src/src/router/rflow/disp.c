/*-
 * Copyright (c) 2001, 2004 Lev Walkin <vlm@lionet.info>.
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
 * $Id: disp.c,v 1.31 2004/04/17 22:31:36 vlm Exp $
 */

#include "rflow.h"
#include "cfgvar.h"
#include "storage.h"
#include "opt.h"
#include "disp.h"

#define	C(f)	(unsigned char)((f) & 0xff)

void
print_ip(FILE *f, struct in_addr ip) {
	ip.s_addr = ntohl(ip.s_addr);
	fprintf(f, "%d.%d.%d.%d",
		C(ip.s_addr >> 24),
		C(ip.s_addr >> 16),
		C(ip.s_addr >> 8),
		C(ip.s_addr >> 0)
	);
}

void
print_aligned_ip(FILE *f, struct in_addr ip) {
	int z;

	ip.s_addr = ntohl(ip.s_addr);

	z = fprintf(f, " %d.%d.%d.%d",
		C(ip.s_addr >> 24),
		C(ip.s_addr >> 16),
		C(ip.s_addr >> 8),
		C(ip.s_addr >> 0)
	);

#define ROOM_FOR_IPADDR	17
	while(z++ < ROOM_FOR_IPADDR)
		putc(' ', f);
}


int
display(FILE *f, disp_storage_e dstore) {
	flow_t *flows;
	int entries;
	time_t storage_created_tstamp;
	time_t stalled;
	long long ex_packets;
	long long ex_bytes;
	flow_storage_t *storage;
	int ret;

	switch(dstore) {
#if 0
	case DS_ACTIVE:		storage = &active_storage; break;
	case DS_CHECKPOINT:	storage = &checkpoint_storage; break;
#endif
	case DS_NETFLOW:	storage = &netflow_storage; break;
	default:
		errno = EINVAL;
		return -1;
	}

	/*
	 * Perform quick operation on the locked table.
	 */
	lock_storage(storage);
		stalled = storage->first_miss;
		ex_packets = storage->missed_packets;
		ex_bytes = storage->missed_bytes;
		storage_created_tstamp = storage->create_time;

		ret = get_flow_table(storage, &flows, &entries);
	unlock_storage(storage);

	if(ret == -1) {
		fprintf(f, "Memory allocation error.\n");
		return -1;
	}

	if(dstore == DS_NETFLOW) {
		putc('\n', f);
#if 0
		fprintf(f, "IP Flow Switching Cache, %ld bytes\n",
			(long)entries * sizeof(*active_storage.buckets[0]));
#endif
		fprintf(f, "  %ld flows, %lld flow alloc failures\n",
			(long)entries, ex_packets);
		fprintf(f, "  Active flows timeout in %ld minutes\n",
			(long)(conf->netflow_timeout_active / 60));
		fprintf(f, "  Inactive flows timeout in %ld seconds\n",
			(long)conf->netflow_timeout_inactive);
	}

	/*
	 * Print out the table contents.
	 */
	dump_flow_table(f, flows, entries, dstore);

	if(flows) free(flows);

	if(dstore == DS_NETFLOW)
		return 0;	/* Further info is not necessary */

	if(storage_created_tstamp) {
		time_t now = time(NULL);
		time_t age = (now - storage_created_tstamp) / 60;
		
		fprintf(f, "Accounting data age is %5ld\n", (long)age);
		fprintf(f, "Accounting data age exact %ld\n",
			(long)now - storage_created_tstamp);
		fprintf(f, "Accounting data saved %ld\n", (long)now);
	}

	if(ex_packets)
		fprintf(f,
			"Accounting threshold exceeded for %llu packets "
			"and %llu bytes\n", ex_packets, ex_bytes);

	if(stalled)
		fprintf(f, "Information incomplete since %s", ctime(&stalled));

	if(dstore == DS_ACTIVE)
		rflow_show_stats(f);

	return 0;
}

void
dump_flow_table(FILE *f, flow_t *flow, int entries, disp_storage_e mode) {

	/*
	 * Print out the header.
	 */
	if(mode == DS_NETFLOW) {
		fprintf(f,
			"\nSrcIf         SrcIPaddress"
			"    DstIf         DstIPaddress"
			"    Pr SrcP DstP  Pkts\n");
		for(; entries > 0; entries--, flow++) {
			int sport = flow->src_port;
			int dport = flow->dst_port;
			if(sport == -1) sport = dport = 0;
			if(flow->ifName[0])
				fprintf(f, "%-13s", flow->ifName);
			else
				fprintf(f, "%-13s",
					IFNameBySource(flow->ifSource));
			print_aligned_ip(f, flow->src);
			fprintf(f, "%-13s", "<?>");
			print_aligned_ip(f, flow->dst);
			fprintf(f, "%02x %04x %04x %5ld",
				flow->ip_p, sport, dport, (long)flow->packets);
			putc('\n', f);
		}
		putc('\n', f);
		return;
	} else if(conf->capture_ports)
		fprintf(f,
			"\n   Source           Destination"
			"    Packets        Bytes"
			"  SrcPt DstPt Proto   IF\n");
	else
		fprintf(f,
			"\n   Source           Destination"
			"              Packets               Bytes"
			"\n");

	/*
	 * Print the entries.
	 */
	for(; entries > 0; entries--, flow++) {
		print_aligned_ip(f, flow->src);
		print_aligned_ip(f, flow->dst);
		fprintf(f,
			conf->capture_ports
			?"%8lu %12qu"
			:"%18lu %19qu",
			(unsigned long)flow->packets, flow->bytes);
		if(conf->capture_ports) {
			if(flow->src_port == -1)
				fprintf(f, " %6s %5s", "-", "-");
			else
				fprintf(f, " %6d %5d",
					flow->src_port, flow->dst_port);
			fprintf(f, " %5d ", flow->ip_p);
			if(flow->ifName[0])
				fprintf(f, "%4s", flow->ifName);
			else
				fprintf(f, "%4s", IFNameBySource(flow->ifSource));
		}
		putc('\n', f);
	}

	putc('\n', f);
}

void
rflow_show_stats(FILE *f) {
	packet_source_t *ps;
#if AFLOW
	size_t active_entries;
#endif
	size_t netflow_entries;
	size_t used_memory;

	/* Reasonably atomic operations, no locking required */
#if AFLOW
	active_entries = active_storage.entries;
#endif
	netflow_entries = netflow_storage.entries;
#if AFLOW
	used_memory = active_entries * sizeof(*active_storage.buckets[0]);
#else
	used_memory = netflow_entries * sizeof(*netflow_storage.buckets[0]);
#endif

	for(ps = conf->packet_sources_head; ps; ps = ps->next) {
		ps->print_stats(f, ps);
	}

#if AFLOW
	fprintf(f, "Flow entries made: %ld\n", (long)active_entries);
#endif
  fprintf(f, "NetFlow cached flows: %ld\n", (long)netflow_entries);

	if(conf->memsize)   
		fprintf(f, "Memory usage: %ld%% (%ld from %ld)\n",
			(long)(used_memory * 100 / conf->memsize),
			(long)used_memory, (long)conf->memsize);
	else
		fprintf(f, "Memory usage: %ld kbytes.\n",
			(long)(used_memory >> 10));

#if AFLOW
	fprintf(f, "Free slots for rsh clients: %ld\n", (long)max_clients);
#endif

	ipcad_uptime(f);
	system_uptime(f);
}

int
display_internal_averages(FILE *f, const char *ifname) {
	packet_source_t *ps;

	/* Find the interface */
	for(ps = conf->packet_sources_head; ps; ps = ps->next) {
		if(strcmp(ps->ifName, ifname) == 0)
			break;
	}

	if(!ps)
		return -1;

	fprintf(f, "  %.0f minute average rate %lld bits/sec, %lld packets/sec\n",  
		ps->avg_period / 60,
		ps->bps_lp * 8,
		ps->pps_lp 
	);

	return 0;
}

void
show_version(FILE *f) {
	fprintf(f,
		RFLOW_VERSION_STRING
		RFLOW_COPYRIGHT "\n"
		"\n"
	);

	ipcad_uptime(f);
	system_uptime(f);
}


void
ipcad_uptime(FILE *f) {
	fprintf(f, "RFLOW uptime is");
	display_uptime(f, time(NULL) - (time_t)self_started);
}


void
display_uptime(FILE *f, time_t uptime) {
	int days, hrs, mins, secs;

	days = uptime / 86400;
	uptime %= 86400;
	hrs = uptime / 3600;
	uptime %= 3600;
	mins = uptime / 60;
	secs = uptime % 60;

	if(days > 0)
		fprintf(f, " %d day%s", days, days>1?"s":"");
	if(hrs > 0 && mins > 0)
		fprintf(f, " %2d:%02d", hrs, mins);
	else if(hrs > 0)
		fprintf(f, " %d hour%s,", hrs, hrs>1?"s":"");
	else if(mins > 0)
		fprintf(f, " %d minute%s", mins, mins>1?"s":"");
	else
		fprintf(f, " %d second%s", secs, secs>1?"s":"");
	fprintf(f, "\n");

}


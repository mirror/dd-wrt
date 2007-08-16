#include "rflow.h"
#include "cfgvar.h"
#include "storage.h"
#include "servers.h"
#include "opt.h"
#include "nflow.h"

static long
get_time() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec;
}

static void process_netflow_cache(double now, server *srv, int force_flush);
static void flow_records_send(server *srv);	/* Send UDP block */
static void flow_export(double now, server *srv, flow_el_t *el);

/*
 * This thread opens a UDP channel to the configured destination
 * and checks the NetFlow tables periodically, flushing out expired entries.
 */
void *
netflow_exporter(void *srvp) {
	server *srv = srvp;
	sigset_t set, oset;
	long time_to = 0;
	long now;
	int sockfd;
	sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd == -1) {
		fprintf(stderr, "Can't create socket for %s.\n",
			srv->name);
		return NULL;
	}

	if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK)) {
		close(sockfd);
		fprintf(stderr, "Can't make socket non-blocking for %s.\n",
			srv->name);
		return NULL;
	}

	srv->sockfd = sockfd;

	/* Flag for parent thread */
	srv->started_ok = 1;

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	/*
	 * Process the NetFlow cache periodically,
	 * flushing expired entries.
	 */
	while(1) {
		long tdiff;

		if(signoff_now)
			break;

		now = get_time();
		tdiff = time_to - now;
		if(tdiff > 0) {
			sigprocmask(SIG_UNBLOCK, &set, &oset);
			sleep(tdiff);
			sigprocmask(SIG_SETMASK, &oset, NULL);
		} else {
			time_to = now + 1;	/* Almost one second */
			process_netflow_cache(now, srv, 0);
		}
	}

	/*************************************************************
	 * We were interrupted when the ipcad was signalled to exit.
	 * Force the full table flush to the destination.
	 */

	/*
	 * Disable non-blocking mode to try to avoid packet drops
	 * due to the write buffer shortage
	 * (network-induced drops are still there, though).
	 */
	(void)fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) & ~O_NONBLOCK);

	/* Force flush */
	process_netflow_cache(get_time(), srv, 1);

	return NULL;
}

/*
 * Process the NetFlow cache storage and emit UDP datagrams.
 */
static void
process_netflow_cache(double now, server *srv, int force_flush) {
	flow_el_t *el, *pel, *nel;
	int removed;

	lock_storage(&netflow_storage);

	/*
	 * Scan the NetFlow cache in search for expired entries.
	 */
	for(pel = 0, el = netflow_storage.head, removed = 0; el;
			pel = removed?pel:el, el = nel) {
		nel = el->hash_next;	/* Next element */
		removed = 0;	/* Not removed anything yet */

		if((now - el->flow.seen_last)  < conf->netflow_timeout_inactive
		&& (now - el->flow.seen_first) < conf->netflow_timeout_active
		&& !force_flush)
			continue;

		/*
		 * Okay, now got the entry that must be expired.
		 */
		if(pel)	pel->hash_next = el->hash_next;
		else	netflow_storage.head = el->hash_next;

		/*
		 * Remove from the hash bucket also.
		 */
		if(el->bucket_prev) {
			if((el->bucket_prev->bucket_next = el->bucket_next))
				el->bucket_next->bucket_prev = el->bucket_prev;
		} else {
			flow_el_t **bucket;
			bucket = &netflow_storage.buckets
				[el->hash_value % netflow_storage.numbuckets];
			if((*bucket = el->bucket_next))
				el->bucket_next->bucket_prev = NULL;
		}
		netflow_storage.entries--;

		flow_export(now, srv, el);

		free(el);
		removed = 1;	/* Removed element, leave pel intact! */
	}

	unlock_storage(&netflow_storage);

	/* Flush the partially full NetFlow output buffer. */
	flow_records_send(srv);
}


static void
flow_export(double now, server *srv, flow_el_t *el) {
	struct NetFlow_VersionDescriptor *vd = &NFVers[conf->netflow_version];
	NFv5_Header *nfh = (void *)srv->buf;
	NFv5_Record *nfr;

	/*
	 * Initialize header, if not yet initialized.
	 */
	if(srv->buf_off == 0) {
		u_int32_t sysuptime_ms = (now - self_started) * 1000;
		memset(nfh, 0, vd->header_size);

		nfh->Version = htons(conf->netflow_version);
		nfh->SysUptime = htonl(sysuptime_ms);
		nfh->unix_secs = now;
		nfh->unix_secs = htonl(nfh->unix_secs);
		nfh->unix_nsecs = (now - (long)now) * 1000000000;
		nfh->unix_nsecs = htonl(nfh->unix_nsecs);
		nfh->flow_sequence = htonl(netflow_storage.flows_count);
		if(conf->netflow_packet_interval > 1) {
			assert((conf->netflow_packet_interval & ~0x3FFF) == 0);
			nfh->sampling_interval = (0x01 << 14)
				| conf->netflow_packet_interval;
			nfh->sampling_interval = htons(nfh->sampling_interval);
		}

		srv->buf_off = vd->header_size;
	}

	assert(sizeof(*nfr) == 48);
	assert(vd->record_size == 48);
	assert(nfh->Count < vd->max_records);

	/*
	 * Fill the record.
	 */
	nfr = (void *)srv->buf + srv->buf_off;
	nfr->srcaddr	= el->flow.src.s_addr;
	nfr->dstaddr	= el->flow.dst.s_addr;
	nfr->nexthop	= 0;	/* AHEZ */
	if(el->flow.ifSource) {
		packet_source_t *ps = el->flow.ifSource;
		nfr->input	= htons(ps->ifIndex);
	} else {
		nfr->input	= 0;
	}
	nfr->output	= 0;	/* Can't distinguish between I and O ;( */
	nfr->dPkts	= htonl(el->flow.packets);
	nfr->dOctets	= htonl(el->flow.bytes);
	nfr->First	= (el->flow.seen_first - self_started) * 1000;
	nfr->First	= htonl(nfr->First);
	nfr->Last	= (el->flow.seen_last - self_started) * 1000;
	nfr->Last	= htonl(nfr->Last);
	if(el->flow.ip_p == IPPROTO_ICMP) {
		/* Ports contain ICMP Type/Code, must ignore */
		nfr->srcport = 0;
		nfr->dstport = 0;
	} else {
		nfr->srcport = htons(el->flow.src_port);
		nfr->dstport = htons(el->flow.dst_port);
	}
	memset(&nfr->pad1, 0, vd->record_size - offsetof(NFv5_Record, pad1));
	nfr->prot	= el->flow.ip_p;
	nfr->tos	= el->flow.ip_tos;
	*((u_int8_t *)nfr + vd->tcp_flags_offset) = el->flow.tcp_flags;
	if(vd->ip_masks_offset) {
		u_int8_t *mask = (u_int8_t *)nfr + vd->ip_masks_offset;
		mask[0] = el->flow.src_mask;
		mask[1] = el->flow.dst_mask;
	}

	/*
	 * Move to the next record slot.
	 */
	srv->buf_off += vd->record_size;
	assert(srv->buf_off < sizeof(srv->buf));
	netflow_storage.flows_count++;	/* Got another flow... */
	nfh->Count++;	/* htons() in flow_records_send() */
	if(nfh->Count == vd->max_records) {
		/*
		 * Maximum number of records is already hanging there.
		 * Send out the packet.
		 */
		flow_records_send(srv);
		assert(srv->buf_off == 0);
	}
}

/*
 * Write the prepared NetFlow record into the destination socket.
 */
static void
flow_records_send(server *srv) {
	NFv5_Header *nfh = (void *)srv->buf;
	ssize_t ret;

	if(srv->buf_off == 0)
		return;
	assert(srv->buf_off <= sizeof(srv->buf));

	nfh->Count = htons(nfh->Count);

	ret = sendto(srv->sockfd, srv->buf, srv->buf_off, 0,
		(struct sockaddr *)&srv->addr, sizeof(srv->addr));
	if(ret == -1) {
		int l;
		socklen_t slen = sizeof(l);

		switch(errno) {
		case EINVAL:
		case EBADF:
		case EFAULT:
			assert(!"unreachable");
		case ENOBUFS:
			break;
		case EHOSTUNREACH:
		case ECONNREFUSED:
		case EHOSTDOWN:
		case ENETDOWN:
		default:
			/* Clear pending error */
			getsockopt(srv->sockfd, SOL_SOCKET, SO_ERROR,
				&l, &slen);
			break;
		}
	}

	srv->buf_off = 0;
}


/*
 * packet.c	Generic packet manipulation functions.
 *
 * Version:	$Id: 0f870f514eeb88d5f2ca348ecd9c01c77411abda $
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000-2006  The FreeRADIUS server project
 */

RCSID("$Id: 0f870f514eeb88d5f2ca348ecd9c01c77411abda $")

#include	<freeradius-devel/libradius.h>

#ifdef WITH_UDPFROMTO
#include	<freeradius-devel/udpfromto.h>
#endif

#include <fcntl.h>

/*
 *	See if two packets are identical.
 *
 *	Note that we do NOT compare the authentication vectors.
 *	That's because if the authentication vector is different,
 *	it means that the NAS has given up on the earlier request.
 */
int fr_packet_cmp(RADIUS_PACKET const *a, RADIUS_PACKET const *b)
{
	int rcode;

	if (a->sockfd < b->sockfd) return -1;
	if (a->sockfd > b->sockfd) return +1;

	/*
	 *	IDs should be spread effectively randomly
	 */
	if (a->id < b->id) return -1;
	if (a->id > b->id) return +1;

#ifdef WITH_TCP
	/*
	 *	TCP sockets have (by definition) the same src/dst
	 *	IP/port combos.  We we can just ignore those fields.
	 */
	if (a->proto == IPPROTO_TCP) {
		return 0;
	}
#endif

	/*
	 *	RADIUS can have unconnected UDP sockets, in which case
	 *	the socket FD is the same, but the src/dst IP/ports
	 *	may be different.
	 */

	/*
	 *	Source ports are pretty much random.
	 */
	rcode = (int) a->src_port - (int) b->src_port;
	if (rcode != 0) return rcode;

	/*
	 *	Usually many client IPs, and few server IPs
	 */
	rcode = fr_ipaddr_cmp(&a->src_ipaddr, &b->src_ipaddr);
	if (rcode != 0) return rcode;

	/*
	 *	One socket can receive packets for multiple
	 *	destination IPs, so we check that before checking the
	 *	file descriptor.
	 */
	rcode = fr_ipaddr_cmp(&a->dst_ipaddr, &b->dst_ipaddr);
	if (rcode != 0) return rcode;

	/*
	 *	At this point, the order of comparing socket FDs
	 *	and/or destination ports doesn't matter.  One of those
	 *	fields will make the socket unique, and the other is
	 *	pretty much redundant.
	 */
	rcode = (int) a->dst_port - (int) b->dst_port;
	return rcode;
}

int fr_inaddr_any(fr_ipaddr_t *ipaddr)
{

	if (ipaddr->af == AF_INET) {
		if (ipaddr->ipaddr.ip4addr.s_addr == INADDR_ANY) {
			return 1;
		}

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	} else if (ipaddr->af == AF_INET6) {
		if (IN6_IS_ADDR_UNSPECIFIED(&(ipaddr->ipaddr.ip6addr))) {
			return 1;
		}
#endif

	} else {
		fr_strerror_printf("Unknown address family");
		return -1;
	}

	return 0;
}


/*
 *	Create a fake "request" from a reply, for later lookup.
 */
void fr_request_from_reply(RADIUS_PACKET *request,
			   RADIUS_PACKET const *reply)
{
	request->sockfd = reply->sockfd;
	request->id = reply->id;
#ifdef WITH_TCP
	request->proto = reply->proto;

#ifdef WITH_RADIUSV11
	if (reply->radiusv11) {
		memcpy(request->vector, reply->vector, sizeof(request->vector));
	}
#endif
#endif
	request->src_port = reply->dst_port;
	request->dst_port = reply->src_port;
	request->src_ipaddr = reply->dst_ipaddr;
	request->dst_ipaddr = reply->src_ipaddr;
}

/*
 *	Open a socket on the given IP and port.
 */
int fr_socket(fr_ipaddr_t *ipaddr, uint16_t port)
{
	int sockfd;
	struct sockaddr_storage salocal;
	socklen_t	salen;

	sockfd = socket(ipaddr->af, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fr_strerror_printf("cannot open socket: %s", fr_syserror(errno));
		return sockfd;
	}

#ifdef WITH_UDPFROMTO
	/*
	 *	Initialize udpfromto for all sockets.
	 */
	if (udpfromto_init(sockfd) != 0) {
		close(sockfd);
		fr_strerror_printf("cannot initialize udpfromto: %s", fr_syserror(errno));
		return -1;
	}
#endif

	if (!fr_ipaddr2sockaddr(ipaddr, port, &salocal, &salen)) {
		return sockfd;
	}

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	if (ipaddr->af == AF_INET6) {
		/*
		 *	Listening on '::' does NOT get you IPv4 to
		 *	IPv6 mapping.  You've got to listen on an IPv4
		 *	address, too.  This makes the rest of the server
		 *	design a little simpler.
		 */
#ifdef IPV6_V6ONLY

		if (IN6_IS_ADDR_UNSPECIFIED(&ipaddr->ipaddr.ip6addr)) {
			int on = 1;

			if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY,
				       (char *)&on, sizeof(on)) < 0) {
				close(sockfd);
				fr_strerror_printf("Failed setting sockopt "
						   "IPPROTO_IPV6 - IPV6_V6ONLY"
						   ": %s", fr_syserror(errno));
				return -1;
			}
		}
#endif /* IPV6_V6ONLY */
	}
#endif /* HAVE_STRUCT_SOCKADDR_IN6 */

#if (defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)) || defined(IP_DONTFRAG)
	if (ipaddr->af == AF_INET) {
		int flag;

#if defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)

		/*
		 *	Disable PMTU discovery.  On Linux, this
		 *	also makes sure that the "don't fragment"
		 *	flag is zero.
		 */
		flag = IP_PMTUDISC_DONT;
		if (setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER,
			       &flag, sizeof(flag)) < 0) {
			close(sockfd);
			fr_strerror_printf("Failed setting sockopt "
					   "IPPROTO_IP - IP_MTU_DISCOVER: %s",
					   fr_syserror(errno));
			return -1;
		}
#endif

#if defined(IP_DONTFRAG)
		/*
		 *	Ensure that the "don't fragment" flag is zero.
		 */
		flag = 0;
		if (setsockopt(sockfd, IPPROTO_IP, IP_DONTFRAG,
			   &flag, sizeof(flag)) < 0) {
			close(sockfd);
			fr_strerror_printf("Failed setting sockopt "
					   "IPPROTO_IP - IP_DONTFRAG: %s",
					   fr_syserror(errno));
			return -1;
		}
#endif
	}
#endif

	if (bind(sockfd, (struct sockaddr *) &salocal, salen) < 0) {
		close(sockfd);
		fr_strerror_printf("cannot bind socket: %s", fr_syserror(errno));
		return -1;
	}

	return sockfd;
}


/*
 *	We need to keep track of the socket & it's IP/port.
 */
typedef struct fr_packet_socket_t {
	int		sockfd;
	void		*ctx;

	uint32_t	num_outgoing;

	int		src_any;
	fr_ipaddr_t	src_ipaddr;
	uint16_t	src_port;

	int		dst_any;
	fr_ipaddr_t	dst_ipaddr;
	uint16_t	dst_port;

	bool		dont_use;

#ifdef WITH_TCP
	int		proto;
#endif

#ifdef WITH_RADIUSV11
	bool		radiusv11;

	uint32_t	counter;
#endif

	uint8_t		id[32];
} fr_packet_socket_t;


#define FNV_MAGIC_PRIME (0x01000193)
#define MAX_SOCKETS (1024)
#define SOCKOFFSET_MASK (MAX_SOCKETS - 1)
#define SOCK2OFFSET(sockfd) ((sockfd * FNV_MAGIC_PRIME) & SOCKOFFSET_MASK)

/*
 *	Structure defining a list of packets (incoming or outgoing)
 *	that should be managed.
 */
struct fr_packet_list_t {
	rbtree_t	*tree;

	int		alloc_id;
	uint32_t	num_outgoing;
	int		last_recv;
	int		num_sockets;

	fr_packet_socket_t sockets[MAX_SOCKETS];
};


/*
 *	Ugh.  Doing this on every sent/received packet is not nice.
 */
static fr_packet_socket_t *fr_socket_find(fr_packet_list_t *pl,
					  int sockfd)
{
	int i, start;

	i = start = SOCK2OFFSET(sockfd);

	do {			/* make this hack slightly more efficient */
		if (pl->sockets[i].sockfd == sockfd) return &pl->sockets[i];

		i = (i + 1) & SOCKOFFSET_MASK;
	} while (i != start);

	return NULL;
}

bool fr_packet_list_socket_freeze(fr_packet_list_t *pl, int sockfd)
{
	fr_packet_socket_t *ps;

	if (!pl) {
		fr_strerror_printf("Invalid argument");
		return false;
	}

	ps = fr_socket_find(pl, sockfd);
	if (!ps) {
		fr_strerror_printf("No such socket");
		return false;
	}

	ps->dont_use = true;
	return true;
}

bool fr_packet_list_socket_thaw(fr_packet_list_t *pl, int sockfd)
{
	fr_packet_socket_t *ps;

	if (!pl) return false;

	ps = fr_socket_find(pl, sockfd);
	if (!ps) return false;

	ps->dont_use = false;
	return true;
}


bool fr_packet_list_socket_del(fr_packet_list_t *pl, int sockfd)
{
	fr_packet_socket_t *ps;

	if (!pl) return false;

	ps = fr_socket_find(pl, sockfd);
	if (!ps) return false;

	if (ps->num_outgoing != 0) {
		fr_strerror_printf("socket is still in use");
		return false;
	}

	ps->sockfd = -1;
	pl->num_sockets--;

	return true;
}


bool fr_packet_list_socket_add(fr_packet_list_t *pl, int sockfd, int proto,
#ifdef WITH_RADIUSV11
			       bool radiusv11,
#endif
			       fr_ipaddr_t *dst_ipaddr, uint16_t dst_port,
			       void *ctx)
{
	int i, start;
	struct sockaddr_storage	src;
	socklen_t		sizeof_src;
	fr_packet_socket_t	*ps;

	if (!pl || !dst_ipaddr || (dst_ipaddr->af == AF_UNSPEC)) {
		fr_strerror_printf("Invalid argument");
		return false;
	}

	if (pl->num_sockets >= MAX_SOCKETS) {
		fr_strerror_printf("Too many open sockets");
		return false;
	}

#ifndef WITH_TCP
	if (proto != IPPROTO_UDP) {
		fr_strerror_printf("only UDP is supported");
		return false;
	}
#endif

	ps = NULL;
	i = start = SOCK2OFFSET(sockfd);

	do {
		if (pl->sockets[i].sockfd == -1) {
			ps =  &pl->sockets[i];
			break;
		}

		i = (i + 1) & SOCKOFFSET_MASK;
	} while (i != start);

	if (!ps) {
		fr_strerror_printf("All socket entries are full");
		return false;
	}

	memset(ps, 0, sizeof(*ps));
	ps->ctx = ctx;
#ifdef WITH_TCP
	ps->proto = proto;
#ifdef WITH_RADIUSV11
	ps->radiusv11 = radiusv11;

	/*
	 *	Start our packet counter at a random 64-bit number.
	 */
	if (radiusv11) {
		ps->counter = fr_rand();
	}
#endif
#endif

	/*
	 *	Get address family, etc. first, so we know if we
	 *	need to do udpfromto.
	 *
	 *	FIXME: udpfromto also does this, but it's not
	 *	a critical problem.
	 */
	sizeof_src = sizeof(src);
	memset(&src, 0, sizeof_src);
	if (getsockname(sockfd, (struct sockaddr *) &src,
			&sizeof_src) < 0) {
		fr_strerror_printf("%s", fr_syserror(errno));
		return false;
	}

	if (!fr_sockaddr2ipaddr(&src, sizeof_src, &ps->src_ipaddr,
				&ps->src_port)) {
		fr_strerror_printf("Failed to get IP");
		return false;
	}

	ps->dst_ipaddr = *dst_ipaddr;
	ps->dst_port = dst_port;

	ps->src_any = fr_inaddr_any(&ps->src_ipaddr);
	if (ps->src_any < 0) return false;

	ps->dst_any = fr_inaddr_any(&ps->dst_ipaddr);
	if (ps->dst_any < 0) return false;

	/*
	 *	As the last step before returning.
	 */
	ps->sockfd = sockfd;
	pl->num_sockets++;

	return true;
}

static int packet_entry_cmp(void const *one, void const *two)
{
	RADIUS_PACKET const * const *a = one;
	RADIUS_PACKET const * const *b = two;

	return fr_packet_cmp(*a, *b);
}

void fr_packet_list_free(fr_packet_list_t *pl)
{
	if (!pl) return;

	rbtree_free(pl->tree);
	free(pl);
}


/*
 *	Caller is responsible for managing the packet entries.
 */
fr_packet_list_t *fr_packet_list_create(int alloc_id)
{
	int i;
	fr_packet_list_t	*pl;

	pl = malloc(sizeof(*pl));
	if (!pl) return NULL;
	memset(pl, 0, sizeof(*pl));

	pl->tree = rbtree_create(NULL, packet_entry_cmp, NULL, 0);
	if (!pl->tree) {
		fr_packet_list_free(pl);
		return NULL;
	}

	for (i = 0; i < MAX_SOCKETS; i++) {
		pl->sockets[i].sockfd = -1;
	}

	pl->alloc_id = alloc_id;

	return pl;
}


/*
 *	If pl->alloc_id is set, then fr_packet_list_id_alloc() MUST
 *	be called before inserting the packet into the list!
 */
bool fr_packet_list_insert(fr_packet_list_t *pl,
			    RADIUS_PACKET **request_p)
{
	if (!pl || !request_p || !*request_p) return 0;

	VERIFY_PACKET(*request_p);

	return rbtree_insert(pl->tree, request_p);
}

RADIUS_PACKET **fr_packet_list_find(fr_packet_list_t *pl,
				      RADIUS_PACKET *request)
{
	if (!pl || !request) return 0;

	VERIFY_PACKET(request);

	return rbtree_finddata(pl->tree, &request);
}


/*
 *	This presumes that the reply has dst_ipaddr && dst_port set up
 *	correctly (i.e. real IP, or "*").
 */
RADIUS_PACKET **fr_packet_list_find_byreply(fr_packet_list_t *pl,
					      RADIUS_PACKET *reply)
{
	RADIUS_PACKET my_request, *request;
	fr_packet_socket_t *ps;

	if (!pl || !reply) return NULL;

	VERIFY_PACKET(reply);

	ps = fr_socket_find(pl, reply->sockfd);
	if (!ps) return NULL;

	/*
	 *	Initialize request from reply, AND from the source
	 *	IP & port of this socket.  The client may have bound
	 *	the socket to 0, in which case it's some random port,
	 *	that is NOT in the original request->src_port.
	 */
	my_request.sockfd = reply->sockfd;
	my_request.id = reply->id;

#ifdef WITH_TCP
	/*
	 *	TCP sockets are always bound to the correct src/dst IP/port
	 */
	if (ps->proto == IPPROTO_TCP) {
		reply->dst_ipaddr = ps->src_ipaddr;
		reply->dst_port = ps->src_port;
		reply->src_ipaddr = ps->dst_ipaddr;
		reply->src_port = ps->dst_port;

		my_request.src_ipaddr = ps->src_ipaddr;
		my_request.src_port = ps->src_port;
		my_request.dst_ipaddr = ps->dst_ipaddr;
		my_request.dst_port = ps->dst_port;

	} else
#endif
	{
		if (ps->src_any) {
			my_request.src_ipaddr = ps->src_ipaddr;
		} else {
			my_request.src_ipaddr = reply->dst_ipaddr;
		}
		my_request.src_port = ps->src_port;

		my_request.dst_ipaddr = reply->src_ipaddr;
		my_request.dst_port = reply->src_port;
	}

#ifdef WITH_TCP
	my_request.proto = reply->proto;
#endif

#ifdef WITH_RADIUSV11
	my_request.radiusv11 = reply->radiusv11;
#endif

	request = &my_request;

	return rbtree_finddata(pl->tree, &request);
}


bool fr_packet_list_yank(fr_packet_list_t *pl, RADIUS_PACKET *request)
{
	rbnode_t *node;

	if (!pl || !request) return false;

	VERIFY_PACKET(request);

	node = rbtree_find(pl->tree, &request);
	if (!node) return false;

	rbtree_delete(pl->tree, node);
	return true;
}

uint32_t fr_packet_list_num_elements(fr_packet_list_t *pl)
{
	if (!pl) return 0;

	return rbtree_num_elements(pl->tree);
}


/*
 *	1 == ID was allocated & assigned
 *	0 == couldn't allocate ID.
 *
 *	Note that this ALSO assigns a socket to use, and updates
 *	packet->request->src_ipaddr && packet->request->src_port
 *
 *	In multi-threaded systems, the calls to id_alloc && id_free
 *	should be protected by a mutex.  This does NOT have to be
 *	the same mutex as the one protecting the insert/find/yank
 *	calls!
 *
 *	We assume that the packet has dst_ipaddr && dst_port
 *	already initialized.  We will use those to find an
 *	outgoing socket.  The request MAY also have src_ipaddr set.
 *
 *	We also assume that the sender doesn't care which protocol
 *	should be used.
 */
bool fr_packet_list_id_alloc(fr_packet_list_t *pl, int proto,
			    RADIUS_PACKET **request_p, void **pctx)
{
	int i, j, k, fd, id, start_i, start_j, start_k;
	int src_any = 0;
	fr_packet_socket_t *ps= NULL;
	RADIUS_PACKET *request = *request_p;

	VERIFY_PACKET(request);

	if ((request->dst_ipaddr.af == AF_UNSPEC) ||
	    (request->dst_port == 0)) {
		fr_strerror_printf("No destination address/port specified");
		return false;
	}

#ifndef WITH_TCP
	if ((proto != 0) && (proto != IPPROTO_UDP)) {
		fr_strerror_printf("Invalid destination protocol");
		return false;
	}
#endif

	/*
	 *	Special case: unspec == "don't care"
	 */
	if (request->src_ipaddr.af == AF_UNSPEC) {
		memset(&request->src_ipaddr, 0, sizeof(request->src_ipaddr));
		request->src_ipaddr.af = request->dst_ipaddr.af;
	}

	src_any = fr_inaddr_any(&request->src_ipaddr);
	if (src_any < 0) {
		fr_strerror_printf("Can't check src_ipaddr");
		return false;
	}

	/*
	 *	MUST specify a destination address.
	 */
	if (fr_inaddr_any(&request->dst_ipaddr) != 0) {
		fr_strerror_printf("Must specify a dst_ipaddr");
		return false;
	}

	/*
	 *	FIXME: Go to an LRU system.  This prevents ID re-use
	 *	for as long as possible.  The main problem with that
	 *	approach is that it requires us to populate the
	 *	LRU/FIFO when we add a new socket, or a new destination,
	 *	which can be expensive.
	 *
	 *	The LRU can be avoided if the caller takes care to free
	 *	Id's only when all responses have been received, OR after
	 *	a timeout.
	 *
	 *	Right now, the random approach is almost OK... it's
	 *	brute-force over all of the available ID's, BUT using
	 *	random numbers for everything spreads the load a bit.
	 *
	 *	The old method had a hash lookup on allocation AND
	 *	on free.  The new method has brute-force on allocation,
	 *	and near-zero cost on free.
	 */

	id = fd = -1;
	if (request->id >= 0 && request->id < 256)
		id = request->id;
	start_i = fr_rand() & SOCKOFFSET_MASK;

#define ID_i ((i + start_i) & SOCKOFFSET_MASK)
	for (i = 0; i < MAX_SOCKETS; i++) {
		if (pl->sockets[ID_i].sockfd == -1) continue; /* paranoia */

		ps = &(pl->sockets[ID_i]);

		/*
		 *	This socket is marked as "don't use for new
		 *	packets".  But we can still receive packets
		 *	that are outstanding.
		 */
		if (ps->dont_use) continue;

#ifdef WITH_TCP
		if (ps->proto != proto) continue;
#endif

		/*
		 *	Address families don't match, skip it.
		 */
		if (ps->src_ipaddr.af != request->dst_ipaddr.af) continue;

		/*
		 *	MUST match dst port, if we have one.
		 */
		if ((ps->dst_port != 0) &&
		    (ps->dst_port != request->dst_port)) continue;

		/*
		 *	MUST match requested src port, if one has been given.
		 */
		if ((request->src_port != 0) &&
		    (ps->src_port != request->src_port)) continue;

		/*
		 *	We don't care about the source IP, but this
		 *	socket is link local, and the requested
		 *	destination is not link local.  Ignore it.
		 */
		if (src_any && (ps->src_ipaddr.af == AF_INET) &&
		    (((ps->src_ipaddr.ipaddr.ip4addr.s_addr >> 24) & 0xff) == 127) &&
		    (((request->dst_ipaddr.ipaddr.ip4addr.s_addr >> 24) & 0xff) != 127)) continue;

		/*
		 *	We're sourcing from *, and they asked for a
		 *	specific source address: ignore it.
		 */
		if (ps->src_any && !src_any) continue;

		/*
		 *	We're sourcing from a specific IP, and they
		 *	asked for a source IP that isn't us: ignore
		 *	it.
		 */
		if (!ps->src_any && !src_any &&
		    (fr_ipaddr_cmp(&request->src_ipaddr,
				   &ps->src_ipaddr) != 0)) continue;

		/*
		 *	UDP sockets are allowed to match
		 *	destination IPs exactly, OR a socket
		 *	with destination * is allowed to match
		 *	any requested destination.
		 *
		 *	TCP sockets must match the destination
		 *	exactly.  They *always* have dst_any=0,
		 *	so the first check always matches.
		 */
		if (!ps->dst_any &&
		    (fr_ipaddr_cmp(&request->dst_ipaddr,
				   &ps->dst_ipaddr) != 0)) continue;

#ifdef WITH_RADIUSV11
		/*
		 *	RADIUSV11 matches on src/dst IP/port, but
		 *	doesn't care about num_outgoing or ID allocation.
		 */
		if (ps->radiusv11) {
			request->radiusv11 = true;

			fd = ps->sockfd;

			/*
			 *	Increment the counter.  It has to be
			 *	unique, but its exact value doesn't
			 *	matter.  i.e. the receiver doesn't
			 *	care if it's a counter or a random
			 *	number.
			 */
			ps->counter++;
			id = 0;
			memcpy(request->vector, &ps->counter, sizeof(ps->counter));
			memset(request->vector + sizeof(ps->counter), 0, sizeof(request->vector) - sizeof(ps->counter));
			break;
		}
#endif

		/*
		 *	All IDs are allocated: ignore it.
		 */
		if (ps->num_outgoing == 256) continue;

		/*
		 *	Otherwise, this socket is OK to use.
		 */

		/*
		 *	An explicit ID was requested
		 */

		if (id != -1) {
			if  ((ps->id[(id >> 3) & 0x1f] & (1 << (id & 0x07))) != 0) continue;

			ps->id[(id >> 3) & 0x1f] |= (1 << (id & 0x07));
			fd = i;
			break;
		}

		/*
		 *	Look for a free Id, starting from a random number.
		 */
		start_j = fr_rand() & 0x1f;
#define ID_j ((j + start_j) & 0x1f)
		for (j = 0; j < 32; j++) {
			if (ps->id[ID_j] == 0xff) continue;


			start_k = fr_rand() & 0x07;
#define ID_k ((k + start_k) & 0x07)
			for (k = 0; k < 8; k++) {
				if ((ps->id[ID_j] & (1 << ID_k)) != 0) continue;

				ps->id[ID_j] |= (1 << ID_k);
				id = (ID_j * 8) + ID_k;
				fd = i;
				break;
			}
			if (fd >= 0) break;
		}
#undef ID_i
#undef ID_j
#undef ID_k
		if (fd >= 0) break;
	}

	/*
	 *	Ask the caller to allocate a new ID.
	 */
	if (fd < 0) {
		fr_strerror_printf("Failed finding socket, caller must allocate a new one");
		return false;
	}

	/*
	 *	Set the ID, source IP, and source port.
	 */
	request->id = id;
#ifdef WITH_RADIUSV11
	if (ps->radiusv11) request->id = ps->counter;
#endif

	request->sockfd = ps->sockfd;
	request->src_ipaddr = ps->src_ipaddr;
	request->src_port = ps->src_port;

	/*
	 *	If we managed to insert it, we're done.
	 */
	if (fr_packet_list_insert(pl, request_p)) {
		if (pctx) *pctx = ps->ctx;
		ps->num_outgoing++;
		pl->num_outgoing++;
		return true;
	}

	/*
	 *	Mark the ID as free.  This is the one line from
	 *	id_free() that we care about here.
	 */
	ps->id[(id >> 3) & 0x1f] &= ~(1 << (id & 0x07));

	request->id = -1;
	request->sockfd = -1;
	request->src_ipaddr.af = AF_UNSPEC;
	request->src_port = 0;

	return false;
}

/*
 *	Should be called AFTER yanking it from the list, so that
 *	any newly inserted entries don't collide with this one.
 */
bool fr_packet_list_id_free(fr_packet_list_t *pl,
			    RADIUS_PACKET *request, bool yank)
{
	fr_packet_socket_t *ps;

	if (!pl || !request) return false;

	VERIFY_PACKET(request);

	if (yank && !fr_packet_list_yank(pl, request)) return false;

	ps = fr_socket_find(pl, request->sockfd);
	if (!ps) return false;

#ifdef WITH_RADIUSV11
	/*
	 *	RADIUSV11 packets don't track individual IDs.
	 */
	if (!ps->radiusv11)
#endif
		ps->id[(request->id >> 3) & 0x1f] &= ~(1 << (request->id & 0x07));

	ps->num_outgoing--;
	pl->num_outgoing--;

	request->id = -1;
	request->src_ipaddr.af = AF_UNSPEC; /* id_alloc checks this */
	request->src_port = 0;

	return true;
}

/*
 *	We always walk RBTREE_DELETE_ORDER, which is like RBTREE_IN_ORDER, except that
 *	<0 means error, stop
 *	0  means OK, continue
 *	1  means delete current node and stop
 *	2  means delete current node and continue
 */
int fr_packet_list_walk(fr_packet_list_t *pl, void *ctx, rb_walker_t callback)
{
	if (!pl || !callback) return 0;

	return rbtree_walk(pl->tree, RBTREE_DELETE_ORDER, callback, ctx);
}

int fr_packet_list_fd_set(fr_packet_list_t *pl, fd_set *set)
{
	int i, maxfd;

	if (!pl || !set) return 0;

	maxfd = -1;

	for (i = 0; i < MAX_SOCKETS; i++) {
		if (pl->sockets[i].sockfd == -1) continue;
		FD_SET(pl->sockets[i].sockfd, set);
		if (pl->sockets[i].sockfd > maxfd) {
			maxfd = pl->sockets[i].sockfd;
		}
	}

	if (maxfd < 0) return -1;

	return maxfd + 1;
}

/*
 *	Round-robins the receivers, without priority.
 *
 *	FIXME: Add sockfd, if -1, do round-robin, else do sockfd
 *		IF in fdset.
 */
RADIUS_PACKET *fr_packet_list_recv(fr_packet_list_t *pl, fd_set *set)
{
	int start;
	RADIUS_PACKET *packet;

	if (!pl || !set) return NULL;

	start = pl->last_recv;
	do {
		start++;
		start &= SOCKOFFSET_MASK;

		if (pl->sockets[start].sockfd == -1) continue;

		if (!FD_ISSET(pl->sockets[start].sockfd, set)) continue;

#ifdef WITH_TCP
		if (pl->sockets[start].proto == IPPROTO_TCP) {
			packet = fr_tcp_recv(pl->sockets[start].sockfd, 0);
			if (!packet) {
				fr_strerror_printf("TCP connection has been closed");
				return NULL;
			}

			/*
			 *	We always know src/dst ip/port for TCP
			 *	sockets.  So just fill them in.  Since
			 *	we read the packet from the TCP
			 *	socket, we invert src/dst.
			 */
			packet->dst_ipaddr = pl->sockets[start].src_ipaddr;
			packet->dst_port = pl->sockets[start].src_port;
			packet->src_ipaddr = pl->sockets[start].dst_ipaddr;
			packet->src_port = pl->sockets[start].dst_port;

		} else
#endif

		/*
		 *	Rely on rad_recv() to fill in the required
		 *	fields.
		 */
		packet = rad_recv(NULL, pl->sockets[start].sockfd, 0);
		if (!packet) continue;

		/*
		 *	Call fr_packet_list_find_byreply().  If it
		 *	doesn't find anything, discard the reply.
		 */

		pl->last_recv = start;
#ifdef WITH_TCP
		packet->proto = pl->sockets[start].proto;
#endif
		return packet;
	} while (start != pl->last_recv);

	return NULL;
}

uint32_t fr_packet_list_num_incoming(fr_packet_list_t *pl)
{
	uint32_t num_elements;

	if (!pl) return 0;

	num_elements = rbtree_num_elements(pl->tree);
	if (num_elements < pl->num_outgoing) return 0; /* panic! */

	return num_elements - pl->num_outgoing;
}

uint32_t fr_packet_list_num_outgoing(fr_packet_list_t *pl)
{
	if (!pl) return 0;

	return pl->num_outgoing;
}

/*
 *	Debug the packet if requested.
 */
void fr_packet_header_print(FILE *fp, RADIUS_PACKET *packet, bool received)
{
	char src_ipaddr[128];
	char dst_ipaddr[128];

	if (!fp) return;
	if (!packet) return;

	/*
	 *	Client-specific debugging re-prints the input
	 *	packet into the client log.
	 *
	 *	This really belongs in a utility library
	 */
	if (is_radius_code(packet->code)) {
		fprintf(fp, "%s %s Id %i from %s%s%s:%u to %s%s%s:%u length %zu\n",
		        received ? "Received" : "Sent",
		        fr_packet_codes[packet->code],
		        packet->id,
		        packet->src_ipaddr.af == AF_INET6 ? "[" : "",
		        inet_ntop(packet->src_ipaddr.af,
				  &packet->src_ipaddr.ipaddr,
				  src_ipaddr, sizeof(src_ipaddr)),
			packet->src_ipaddr.af == AF_INET6 ? "]" : "",
		        packet->src_port,
		        packet->dst_ipaddr.af == AF_INET6 ? "[" : "",
		        inet_ntop(packet->dst_ipaddr.af,
				  &packet->dst_ipaddr.ipaddr,
				  dst_ipaddr, sizeof(dst_ipaddr)),
		        packet->dst_ipaddr.af == AF_INET6 ? "]" : "",
		        packet->dst_port,
		        packet->data_len);
	} else {
		fprintf(fp, "%s code %u Id %i from %s%s%s:%u to %s%s%s:%u length %zu\n",
		        received ? "Received" : "Sent",
		        packet->code,
		        packet->id,
		        packet->src_ipaddr.af == AF_INET6 ? "[" : "",
		        inet_ntop(packet->src_ipaddr.af,
				  &packet->src_ipaddr.ipaddr,
				  src_ipaddr, sizeof(src_ipaddr)),
		        packet->src_ipaddr.af == AF_INET6 ? "]" : "",
		        packet->src_port,
		        packet->dst_ipaddr.af == AF_INET6 ? "[" : "",
		        inet_ntop(packet->dst_ipaddr.af,
				  &packet->dst_ipaddr.ipaddr,
				  dst_ipaddr, sizeof(dst_ipaddr)),
		        packet->dst_ipaddr.af == AF_INET6 ? "]" : "",
		        packet->dst_port,
		        packet->data_len);
	}
}


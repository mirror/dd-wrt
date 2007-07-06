/*
 * in_distribution.c     Distribution Input
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <net/if.h>

#include <bmon/bmon.h>
#include <bmon/input.h>
#include <bmon/node.h>
#include <bmon/item.h>
#include <bmon/distribution.h>
#include <bmon/utils.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

static int recv_fd = -1;
static char *c_port = "2048";
static int c_port_int = 2048;
static char *c_ip = NULL;
static int c_ipv6 = 0;
static int c_max_read = 10;
static int c_bufsize = 8192;
static int c_debug = 0;
static int c_multicast = 0;
static int c_bind = 1;
static char *c_iface = NULL;
static char *buf;

static int join_multicast4(int fd, struct sockaddr_in *addr, const char *iface)
{
	struct ip_mreq mreq;
	struct ifreq ifreq;
	unsigned char loop = 0;

	memcpy(&mreq.imr_multiaddr, &addr->sin_addr, sizeof(struct sockaddr_in));
	strncpy(ifreq.ifr_name, iface, IFNAMSIZ);

	if (iface) {
		if (ioctl(fd, SIOCGIFADDR, &ifreq) < 0)
			return -1;

		memcpy(&mreq.imr_interface,
			&((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
			sizeof(struct sockaddr_in));
	} else
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		return -1;

	return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
}

static int join_multicast6(int fd, struct sockaddr_in6 *addr, const char *iface)
{
	struct ipv6_mreq mreq6;
	unsigned char loop = 0;

	memcpy(&mreq6.ipv6mr_multiaddr, &addr->sin6_addr, sizeof(struct sockaddr_in6));

	if (iface) {
		return -1; /* XXX: unsupported atm */
	} else
		mreq6.ipv6mr_interface = 0;

	if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) < 0)
		return -1;

	return setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop));
}

static int join_multicast(int fd, struct sockaddr *sa, const char *iface)
{
	switch (sa->sa_family) {
		case AF_INET:
			return join_multicast4(fd, (struct sockaddr_in *) sa, iface);

		case AF_INET6:
			return join_multicast6(fd, (struct sockaddr_in6 *) sa, iface);
	}

	return -1;
}

static int distribution_probe(void)
{
	if (c_ip) {
		int err;
		char s[INET6_ADDRSTRLEN];
		struct addrinfo hints = {
			.ai_socktype = SOCK_DGRAM,
			.ai_family = c_ipv6 ? PF_INET6 : PF_INET,
		};
		struct addrinfo *res = NULL, *t;

		if (c_ipv6 && !strcmp(c_ip, "224.0.0.1"))
			c_ip = "ff01::1";

		if ((err = getaddrinfo(c_ip, c_port, &hints, &res)) < 0)
			quit("getaddrinfo failed: %s\n", gai_strerror(err));

		for (t = res; t; t = t->ai_next) {
			if (c_debug) {
				const char *x = xinet_ntop(t->ai_addr, s, sizeof(s));
				fprintf(stderr, "Trying %s...", x ? x : "null");
			}
			recv_fd = socket(t->ai_family, t->ai_socktype, 0);

			if (recv_fd < 0) {
				if (c_debug)
					fprintf(stderr, "socket() failed: %s\n", strerror(errno));
				continue;
			}

			if (c_multicast) {
				if (join_multicast(recv_fd, t->ai_addr, c_iface) < 0)
					continue;

				if (!c_bind)
					goto skip_bind;
			}

			if (bind(recv_fd, t->ai_addr, t->ai_addrlen) < 0) {
				if (c_debug)
					fprintf(stderr, "bind() failed: %s\n", strerror(errno));
				continue;
			}
skip_bind:

			if (c_debug)
				fprintf(stderr, "OK\n");

			goto ok;
		}

		fprintf(stderr, "Could not create and connect a datagram " \
			"socket, tried:\n");

		for (t = res; t; t = t->ai_next) {
			const char *x = xinet_ntop(t->ai_addr, s, sizeof(s));
			fprintf(stderr, "\t%s\n", x ? x : "null");
		}

		quit("Last error message was: %s\n", strerror(errno));
	} else {
		if (c_ipv6) {
			struct sockaddr_in6 addr = {
				.sin6_family = AF_INET6,
				.sin6_addr = IN6ADDR_ANY_INIT,
				.sin6_port = htons(c_port_int),
			};

			recv_fd = socket(AF_INET6, SOCK_DGRAM, 0);

			if (recv_fd < 0)
				goto try_4;

			if (bind(recv_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
				goto try_4;

			goto ok;
		}
try_4:
		recv_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (recv_fd < 0)
			quit("socket creation failed: %s\n", strerror(errno));
		{
			struct sockaddr_in addr = {
				.sin_family = AF_INET,
				.sin_port = htons(c_port_int),
			};

			/* Guess what, NetBSD is fucked up so this can't
			 * be in the initializer */
			addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(recv_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
				quit("bind failed: %s\n", strerror(errno));
		}

		goto ok;
	}

ok:
	{
		int flags;
		
		if ((flags = fcntl(recv_fd, F_GETFL)) < 0)
			quit("fcntl failed: %s\n", strerror(errno));
		
		if (fcntl(recv_fd, F_SETFL, flags | O_NONBLOCK) < 0)
			quit("fcntl failed: %s\n", strerror(errno));

	}

	return 1;
}

static int process_item(struct distr_msg_hdr *hdr, const char *nodename,
			struct distr_msg_item *intf, char *from)
{
	char *intfname;
	int remaining, offset;
	char *desc = NULL;
	uint32_t handle = 0;
	int rx_usage = -1, tx_usage = -1;
	int parent = 0, level = 0, link = 0, index = 0;

	item_t *local_intf, *parent_intf;
	node_t *remote_node;

	intfname = ((char *) intf) + sizeof(*intf);

	if (c_debug)
		fprintf(stderr, "Processing interface %s (offset: %d " \
			"optslen: %d namelen: %d hdrsize: %lu)\n",
			intfname ? intfname : "null", ntohs(intf->i_offset),
			intf->i_optslen, intf->i_namelen, (unsigned long) sizeof(*intf));

	if (intf->i_namelen < 4 || intf->i_namelen > IFNAME_MAX) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (invalid namelen %d)\n",
				intf->i_namelen);
		return -1;
	}

	if ('\0' == *intfname) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (empty linkname)\n");
		return -1;
	}

	index = ntohs(intf->i_index);

	if (intf->i_optslen) {
		offset = sizeof(*intf) + intf->i_namelen;
		remaining = intf->i_optslen;

		while (remaining > 0) {
			struct distr_msg_ifopt *opt;

			opt = (struct distr_msg_ifopt *) (((char *) intf) + offset);

			if (opt->io_len > (remaining - sizeof(*opt))) {
				if (c_debug)
					fprintf(stderr, "Discarding malformed packet (invalid opt len)\n");
				return -1;
			}

			switch (opt->io_type) {
				case IFOPT_HANDLE:
					if (opt->io_len != sizeof(uint32_t)) {
						if (c_debug)
							fprintf(stderr, "Discarding malformed packet " \
								"(invalid opt len for handle)\n");
						return -1;
					}

					handle = ntohl(*(uint32_t *) (((char *) opt) + sizeof (*opt)));
					break;

				case IFOPT_PARENT:
					parent = ntohs(opt->io_pad);
					break;

				case IFOPT_LEVEL:
					level = ntohs(opt->io_pad);
					break;

				case IFOPT_LINK:
					link = ntohs(opt->io_pad);
					break;

				case IFOPT_RX_USAGE:
					if (opt->io_len != sizeof(uint32_t)) {
						if (c_debug)
							fprintf(stderr, "Discarding malformed packet " \
								"(invalid opt len for rx usage)\n");
						return -1;
					}

					rx_usage = ntohl(*(uint32_t *) (((char *)opt) + sizeof (*opt)));
					break;

				case IFOPT_TX_USAGE:
					if (opt->io_len != sizeof(uint32_t)) {
						if (c_debug)
							fprintf(stderr, "Discarding malformed packet " \
								"(invalid opt len for tx usage)\n");
						return -1;
					}

					tx_usage = ntohl(*(uint32_t *) (((char *)opt) + sizeof (*opt)));
					break;

				case IFOPT_DESC:
					if (opt->io_len <= 0) {
						if (c_debug)
							fprintf(stderr, "Discarding malformed packet " \
								"(invalid opt len for description)\n");
						return -1;
					}

					desc = ((char *) opt) + sizeof(*opt);
					break;
			}

			remaining -= (sizeof(*opt) + opt->io_len);
			offset += (sizeof(*opt) + opt->io_len);
		}
	
		if (remaining < 0)
			if (c_debug)
				fprintf(stderr, "Leftover from options: %d\n", abs(remaining));
	}

	remote_node = lookup_node(nodename, 1);

	if (NULL == remote_node) {
		if (c_debug)
			fprintf(stderr, "Could not create node entry for remote node\n");
		return -1;
	}

	if (remote_node->n_from)
		xfree((void *) remote_node->n_from);
	remote_node->n_from = strdup(from);

	if (ntohs(intf->i_flags) & IF_IS_CHILD) {
		parent_intf = get_item(remote_node, parent);
		if (parent_intf == NULL) {
			if (c_debug)
				fprintf(stderr, "Could not find parent interface for remote interface\n");
			return -1;
		}
	} else
		parent_intf = NULL;

	local_intf = lookup_item(remote_node, intfname, handle, parent_intf);
	if (local_intf == NULL) {
		if (c_debug)
			fprintf(stderr, "Could not crate interface for remote interface\n");
		return -1;
	}

	if (local_intf->i_flags & ITEM_FLAG_LOCAL) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet " \
					"(about to overwrite a local item)\n");
		return -1;
	}

	local_intf->i_major_attr = BYTES;
	local_intf->i_minor_attr = PACKETS;

	local_intf->i_rx_usage = rx_usage;
	local_intf->i_tx_usage = tx_usage;

	if (desc) {
		if (local_intf->i_desc && strcmp(local_intf->i_desc, desc)) {
			free(local_intf->i_desc);
			local_intf->i_desc = NULL;
		}

		if (local_intf->i_desc == NULL)
			local_intf->i_desc = strdup(desc);
	}
		
	offset = sizeof(*intf) + intf->i_optslen + intf->i_namelen;
	remaining = ntohs(intf->i_offset) - offset;

	while (remaining > 0) {
		struct distr_msg_attr *attr;
		uint64_t rx, tx;
		int type;
		attr = (struct distr_msg_attr *) (((char *) intf) + offset);

		type = ntohs(attr->a_type);
		rx = xntohll(attr->a_rx);
		tx = xntohll(attr->a_tx);

		if (c_debug)
			fprintf(stderr, "Attribute type %d %" PRIu64 " %" PRIu64 "\n",
				type, rx, tx);

		if (type >= ATTR_MAX)
			goto skip;

		if (1) {
			int aflags = ntohs(attr->a_flags);
			int flags = (aflags & ATTR_RX_PROVIDED ? RX_PROVIDED : 0) |
				    (aflags & ATTR_TX_PROVIDED ? TX_PROVIDED : 0);

			update_attr(local_intf, type, rx, tx, flags);
		}

skip:
		remaining -= sizeof(*attr);
		offset += sizeof(*attr);
	}

	if (ntohs(intf->i_flags) & IF_IS_CHILD)
		local_intf->i_flags |= ITEM_FLAG_IS_CHILD;
	local_intf->i_level = level;
	local_intf->i_link = link;

	if (1) {
		timestamp_t ts = {
			.tv_sec = ntohl(hdr->h_ts_sec),
			.tv_usec = ntohl(hdr->h_ts_usec)
		};

		notify_update(local_intf, &ts);
	}

	increase_lifetime(local_intf, 1);

	if (remaining < 0)
		if (c_debug)
			fprintf(stderr, "Leftover from attributes: %d\n", abs(remaining));

	return 0;
}

static void process_group(struct distr_msg_hdr *hdr, const char *nodename,
			  struct distr_msg_grp *grp, char *from)
{
	int remaining, offset;
	int grpoffset;
	
	if (c_debug)
		fprintf(stderr, "Processing group (type:%d offset:%d)\n",
			ntohs(grp->g_type), ntohs(grp->g_offset));

	if (ntohs(grp->g_type) != BMON_GRP_IF) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (invalid group type)\n");
		return;
	}

	grpoffset = ntohs(grp->g_offset);

	if (grpoffset < sizeof(*grp) || grpoffset > ntohs(hdr->h_len)) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (invalid group offset)\n");
		return;
	}

	offset = sizeof(*grp);
	remaining = grpoffset - offset;

	while (remaining > 0) {
		struct distr_msg_item *intf = (struct distr_msg_item *) (((char *) grp) + offset);
		int ioff = ntohs(intf->i_offset);

		if (ioff < (sizeof(*intf) + 4) || ioff > ntohs(hdr->h_len)) {
			if (c_debug)
				fprintf(stderr, "Discarding malformed packet (interface offset too short)\n");
			return;
		}

		if (ioff > remaining) {
			if (c_debug)
				fprintf(stderr, "Discarding malformed packet (unexpected group end)\n");
			return;
		}

		if (process_item(hdr, nodename, intf, from) < 0)
			return;

		remaining -= ioff;
		offset += ioff;
	}

	if (remaining < 0)
		if (c_debug)
			fprintf(stderr, "Leftover from group: %d\n", abs(remaining));
}

static void process_msg(struct distr_msg_hdr *hdr, char *from)
{
	char *nodename;
	struct distr_msg_grp *group;
	
	if (c_debug)
		fprintf(stderr, "Processing message from %s (len=%d)\n",
			from, ntohs(hdr->h_len));

	nodename = ((char *) hdr) + sizeof(*hdr);
	group = (struct distr_msg_grp *) (((char *) hdr) + hdr->h_offset);

	if ('\0' == *nodename) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (empty nodename)\n");
		return;
	}
		

	process_group(hdr, nodename, group, from);
}

static void process_data(char *buf, int len, char *from)
{
	struct distr_msg_hdr *hdr = (struct distr_msg_hdr *) buf;

	if (len < sizeof(*hdr)) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (hdrcheck)\n");
		return;
	}

	if (hdr->h_magic != BMON_MAGIC) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (magic mismatch)\n");
		return;
	}

	if (hdr->h_ver != BMON_VERSION) {
		if (c_debug)
			fprintf(stderr, "Discarding incompatible packet (version mismatch)\n");
		return;
	}

	if (ntohs(hdr->h_len) < len) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (packet size mismatch)\n");
		return;
	}

	if (hdr->h_offset < (sizeof(*hdr) + 4)) {
		if (c_debug)
			fprintf(stderr, "Discarding malformed packet (offset too short)\n");
		return;
	}

	process_msg(hdr, from);
}

static void distribution_read(void)
{
	int i, n;
	struct sockaddr_in6 addr;
	socklen_t len = sizeof(addr);
	char addrstr[INET6_ADDRSTRLEN];

	memset(&addr, 0, sizeof(addr));
	memset(buf, 0, c_bufsize);

	for (i = 0; i < c_max_read; i++) {
		n = recvfrom(recv_fd, buf, c_bufsize, 0,
			(struct sockaddr *) &addr, &len);

		if (n < 0) {
			if (EAGAIN == errno)
				return;
			else
				quit("recvfrom failed: %s\n", strerror(errno));
		}

		if (addr.sin6_family == AF_INET) {
			struct sockaddr_in *in4 = (struct sockaddr_in *) &addr;
			inet_ntop(AF_INET, (void *) &in4->sin_addr,
				addrstr, len);
		} else if (addr.sin6_family == AF_INET6) {
			inet_ntop(AF_INET6, (void *) &addr.sin6_addr,
				addrstr, len);
		}

		if (c_debug)
			fprintf(stderr, "Read %d bytes from %s\n", n, addrstr);

		process_data(buf, n, addrstr);
	}

	return;
}

static void distribution_init(void)
{
	buf = xcalloc(1, c_bufsize);
}

static void print_help(void)
{
	printf(
	"Distribution - Collects statistics from other nodes\n" \
	"\n" \
	"  Collects statistics from other nodes using the distribution\n" \
	"  secondary output method (-O distribution).\n" \
	"\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    ip=ADDR            Only process messages from this address (default: none)\n" \
	"    port=NUM           Port the messages are comming from (default: 2048)\n" \
	"    ipv6               Prefer IPv6 when creating sockets\n" \
	"    multicast[=ADDR]   Use multicast to collect statistics\n" \
	"    intf=NAME          Bind multicast socket to given interface\n" \
	"    nobind             Don't bind, receive multicast and unicast messages\n" \
	"    max_read=NUM       Max. reads during one read interval (default: 10)\n" \
	"    bufsize=NUM        Size of receive buffer (default: 8192)\n" \
	"    debug              Print verbose message for debugging\n" \
	"    help               Print this help text\n");
}

static void distribution_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "port") && attrs->value) {
			c_port = attrs->value;
			c_port_int = strtol(c_port, NULL, 0);
		} else if (!strcasecmp(attrs->type, "ip") && attrs->value)
			c_ip = attrs->value;
		else if (!strcasecmp(attrs->type, "ipv6"))
			c_ipv6 = 1;
		else if (!strcasecmp(attrs->type, "max_read") && attrs->value)
			c_max_read = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "bufsize") && attrs->value)
			c_bufsize = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "debug"))
			c_debug = 1;
		else if (!strcasecmp(attrs->type, "multicast")) {
			if (attrs->value)
				c_ip = attrs->value;
			else
				c_ip = "224.0.0.1";
		} else if (!strcasecmp(attrs->type, "nobind"))
			c_bind = 0;
		else if (!strcasecmp(attrs->type, "intf") && attrs->value)
			c_iface = attrs->value;
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static struct input_module distribution_ops = {
	.im_name = "distribution",
	.im_set_opts = distribution_set_opts,
	.im_read = distribution_read,
	.im_probe = distribution_probe,
	.im_init = distribution_init,
};

static void __init do_distribution_init(void)
{
	register_secondary_input_module(&distribution_ops);
}

/*
 * bmon_distr.c            Bandwidth Monitor
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

#include <bmon/bmon.h>
#include <bmon/output.h>
#include <bmon/node.h>
#include <bmon/item.h>
#include <bmon/input.h>
#include <bmon/distribution.h>
#include <bmon/utils.h>

#include <netdb.h>

static int send_fd;
static int c_ipv6;
static char *c_ip = "224.0.0.1";
static char *c_port = "2048";
static int c_forward = 0;
static int c_errignore = 0;
static int c_debug = 0;
static int c_send_all = 15;
static int send_all_rem = 1;

static void * build_opts(item_t *intf, size_t *size)
{
	void *buf;
	size_t optsize = 0, off = 0;
	struct distr_msg_ifopt *ip;

	if (intf->i_handle)
		optsize += sizeof(*ip) + 4;

	if (intf->i_parent)
		optsize += sizeof(*ip);

	if (intf->i_link)
		optsize += sizeof(*ip);

	if (intf->i_level)
		optsize += sizeof(*ip);

	if (intf->i_rx_usage >= 0)
		optsize += sizeof(*ip) + 4;

	if (intf->i_tx_usage >= 0)
		optsize += sizeof(*ip) + 4;

	if (intf->i_desc)
		optsize += sizeof(*ip) + ((strlen(intf->i_desc)+4) & ~3);

	if (0 == optsize) {
		*size = 0;
		return NULL;
	}

	buf = xcalloc(1, optsize);

	if (intf->i_handle) {
		ip = buf + off;
		ip->io_type = IFOPT_HANDLE;
		ip->io_len = 4;
		*((uint32_t *) (buf + off + sizeof(*ip)))  = htonl(intf->i_handle);
		off += sizeof(*ip) + 4;
	}

	if (intf->i_parent) {
		ip = buf + off;
		ip->io_type = IFOPT_PARENT;
		ip->io_len = 0;
		ip->io_pad = htons(intf->i_parent);
		off += sizeof(*ip);
	}

	if (intf->i_link) {
		ip = buf + off;
		ip->io_type = IFOPT_LINK;
		ip->io_len = 0;
		ip->io_pad = htons(intf->i_link);
		off += sizeof(*ip);
	}

	if (intf->i_level) {
		ip = buf + off;
		ip->io_type = IFOPT_LEVEL;
		ip->io_len = 0;
		ip->io_pad = htons(intf->i_level);
		off += sizeof(*ip);
	}

	if (intf->i_rx_usage >= 0) {
		ip = buf + off;
		ip->io_type = IFOPT_RX_USAGE;
		ip->io_len = 4;
		*((uint32_t *) (buf+off+sizeof(*ip))) = htonl(intf->i_rx_usage);
		off += sizeof(*ip) + 4;
	}

	if (intf->i_tx_usage >= 0) {
		ip = buf + off;
		ip->io_type = IFOPT_TX_USAGE;
		ip->io_len = 4;
		*((uint32_t *) (buf+off+sizeof(*ip))) = htonl(intf->i_tx_usage);
		off += sizeof(*ip) + 4;
	}

	if (intf->i_desc) {
		ip = buf + off;
		ip->io_type = IFOPT_DESC;
		ip->io_len = (strlen(intf->i_desc)+4) & ~3;
		strcpy(buf+off+sizeof(*ip), intf->i_desc);
		off += sizeof(*ip) + ip->io_len;
	}

	*size = optsize;
	return buf;
}

static inline int worth_sending(stat_attr_t *a)
{
	if (send_all_rem == 0)
		return 1;

	if (!(a->a_flags & ATTR_FLAG_RX_ENABLED) &&
	    !(a->a_flags & ATTR_FLAG_TX_ENABLED)) {
		if (c_debug)
			fprintf(stderr, "Attribute %s not worth sending, no data\n",
			    type2name(a->a_type));
		return 0;
	}

	if (!attr_get_rx(a) && !attr_get_tx(a)) {
		if (c_debug)
			fprintf(stderr, "Attribute %s not worth sending, still 0\n",
			    type2name(a->a_type));
		return 0;
	}
	
	if (a->a_last_distribution.tv_sec == a->a_updated.tv_sec) {
		if (a->a_last_distribution.tv_usec == a->a_updated.tv_usec) {
			if (c_debug)
				fprintf(stderr, "Attribute %s not worth sending, no update\n",
				    type2name(a->a_type));
			return 0;
		}
	}

	return 1;
}

static void * build_item_msg(item_t *intf, size_t *size)
{
	int i, off = 0;
	void *buf, *opts;
	size_t msgsize = 0, namelen, nattrs = 0, optsize;
	struct distr_msg_item *ip;
	struct distr_msg_attr *ap;

	namelen = (strlen(intf->i_name) + 5) & ~3; /* 5 because of \0 */
	opts = build_opts(intf, &optsize);

	for (i = 0; i < ATTR_HASH_MAX; i++) {
		stat_attr_t *a;
		for (a = intf->i_attrs[i]; a; a = a->a_next)
			if (worth_sending(a))
				nattrs++;
	}
	
	msgsize = sizeof(*ip) + namelen + optsize + (nattrs * sizeof(*ap));

	ip = buf = xcalloc(1, msgsize);

	ip->i_index = htons(intf->i_index);
	ip->i_offset = htons(msgsize);
	ip->i_namelen = namelen;
	ip->i_optslen = optsize;
	ip->i_flags = htons((intf->i_flags & ITEM_FLAG_IS_CHILD) ? IF_IS_CHILD : 0);

	off = sizeof(*ip);
	memcpy(buf + off, intf->i_name, strlen(intf->i_name));
	off += namelen;

	if (opts) {
		memcpy(buf + off, opts, optsize);
		off += optsize;
	}

	for (i = 0; i < ATTR_HASH_MAX; i++) {
		stat_attr_t *a;
		for (a = intf->i_attrs[i]; a; a = a->a_next) {
			if (worth_sending(a)) {
				struct distr_msg_attr am = {
					.a_type = htons(a->a_type),
					.a_rx = xhtonll(attr_get_rx(a)),
					.a_tx = xhtonll(attr_get_tx(a)),
					.a_flags = htons(
					    (a->a_flags & ATTR_FLAG_RX_ENABLED ? ATTR_RX_PROVIDED : 0) |
					    (a->a_flags & ATTR_FLAG_TX_ENABLED ? ATTR_TX_PROVIDED : 0)),
				};

				COPY_TS(&a->a_last_distribution, &a->a_updated);
				memcpy(buf + off, &am, sizeof(am));
				off += sizeof(am);
			}
		}
	}

	*size = msgsize;
	return buf;
}

static void * build_item_group(node_t *node, size_t *size)
{
	size_t grpsize = sizeof(struct distr_msg_grp);
	struct distr_msg_grp *gp;
	void *group;
	int i;

	gp = group = xcalloc(1, grpsize);
	
	for (i = 0; i < node->n_nitems; i++) {
		if (node->n_items[i].i_name[0]) {
			size_t size;
			void *im = build_item_msg(&node->n_items[i], &size);
			int goff = grpsize;

			grpsize += size;
			gp = group = xrealloc(group, grpsize);
			memcpy(group + goff, im, size);

			xfree(im);
		}
	}

	gp->g_type = htons(BMON_GRP_IF);
	gp->g_offset = htons(grpsize);

	*size = grpsize;
	return group;
}

static void distribute_node(node_t *node, void *arg)
{
	void *buf;
	struct distr_msg_hdr *hdr;
	size_t grpsize;
	void *grp = build_item_group(node, &grpsize);
	size_t nodenamelen = (strlen(node->n_name) + 5) & ~3; /* 5 because of \0 */
	size_t msgsize = sizeof(*hdr) + nodenamelen + grpsize;

	hdr = buf = xcalloc(1, msgsize);

	hdr->h_magic = BMON_MAGIC;
	hdr->h_ver = BMON_VERSION;
	hdr->h_offset = sizeof(*hdr) + nodenamelen;
	hdr->h_len = htons(msgsize);
	hdr->h_ts_sec = htonl(rtiming.rt_last_read.tv_sec);
	hdr->h_ts_usec = htonl(rtiming.rt_last_read.tv_usec);
	memcpy(buf + sizeof(*hdr), node->n_name, strlen(node->n_name));
	memcpy(buf + sizeof(*hdr) + nodenamelen, grp, grpsize);

	if (send(send_fd, buf, msgsize, 0) < 0) {
		if (!c_errignore)
			quit("send() failed: %s\n", strerror(errno));
		else if (c_debug)
			fprintf(stderr, "Ignoring error %s\n", strerror(errno));
	}
	
	xfree(grp);
	xfree(buf);
}

static void distribute_nodes(void)
{
	send_all_rem--;
	
	if (c_forward)
		foreach_node(distribute_node, NULL);
	else
		distribute_node(get_local_node(), NULL);

	if (send_all_rem <= 0)
		send_all_rem = c_send_all;
}

static void print_module_help(void)
{
	printf(
	"Distribution of statistic over a network\n" \
	"\n" \
	"  Sends all local statistics to the specified ip or to the\n" \
	"  multicast address all-hosts if none was specified. The\n" \
	"  protocol is optimized for size, therefore only counters\n" \
	"  that have changed will get distributed. The complete list\n" \
	"  of attribute will be distributed once in a while to make\n" \
	"  clients also list zero counters.\n" \
	"\n" \
	"  You may want to set the option `errignore' while in unicast\n" \
	"  mode to prevent send() from failing if the destination has\n" \
	"  gone down for a while (f.e. due to a reboot).\n" \
	"\n" \
	"  Remotely collected statistics can be distributed in forwarding\n" \
	"  mode. However, you must make sure to not create loops as there\n" \
	"  is no duplicate check on the receiving side and thus would\n" \
	"  result in statistic corruption.\n" \
	"\n"
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    ip=ADDR          Destination address (default: 224.0.0.1/ff01::1)\n" \
	"    port=NUM         Destination port (default: 2048)\n" \
	"    ipv6             Prefer IPv6\n" \
	"    forward          Forwarding mode, also distribute non-local nodes\n" \
	"    errignore        Ignore ICMP error messages while sending\n" \
	"    debug            Verbose output for debugging\n" \
	"    sendall          Send interval of complete attribute list (default: 15)\n" \
	"    help             Print this help text\n");
}

static void distribution_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "ip") && attrs->value)
			c_ip = attrs->value;
		else if (!strcasecmp(attrs->type, "port") && attrs->value)
			c_port = attrs->value;
		else if (!strcasecmp(attrs->type, "ipv6"))
			c_ipv6 = 1;
		else if (!strcasecmp(attrs->type, "forward"))
			c_forward = 1;
		else if (!strcasecmp(attrs->type, "errignore"))
			c_errignore = 1;
		else if (!strcasecmp(attrs->type, "debug"))
			c_debug = 1;
		else if (!strcasecmp(attrs->type, "sendall")) {
			if (attrs->value)
				c_send_all = strtol(attrs->value, NULL, 0);
			else
				c_send_all = 1;
		} else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static int distribution_probe(void)
{
	int err;
	struct addrinfo *t, *res = NULL;
	struct addrinfo hints = {
		.ai_socktype = SOCK_DGRAM,
		.ai_family = c_ipv6 ? PF_INET6 : PF_INET,
	};
	char s[INET6_ADDRSTRLEN];

	if (c_ipv6 && !strcmp(c_ip, "224.0.0.1"))
		c_ip = "ff01::1";
	
	if ((err = getaddrinfo(c_ip, c_port, &hints, &res)) < 0)
		quit("getaddrinfo failed: %s\n", gai_strerror(err));
	
	
	for (t = res; t; t = t->ai_next) {
		if (c_debug) {
			const char *x = xinet_ntop(t->ai_addr, s, sizeof(s));
			fprintf(stderr, "Trying %s...", x ? x : "null");
		}
		send_fd = socket(t->ai_family, t->ai_socktype, 0);

		if (send_fd < 0) {
			if (c_debug)
				fprintf(stderr, "socket() failed: %s\n", strerror(errno));
			continue;
		}

		if (connect(send_fd, t->ai_addr, t->ai_addrlen) < 0) {
			if (c_debug)
				fprintf(stderr, "connect() failed: %s\n", strerror(errno));
			continue;
		}

		if (c_debug)
			fprintf(stderr, "OK\n");
		return 1;
	}

	fprintf(stderr, "Could not create and connect a datagram " \
		"socket, tried:\n");

	for (t = res; t; t = t->ai_next) {
		const char *x = xinet_ntop(t->ai_addr, s, sizeof(s));
		fprintf(stderr, "\t%s\n", x ? x : "null");
	}

	quit("Last error message was: %s\n", strerror(errno));
	return 0;
}

static struct output_module distribution_ops = {
	.om_name = "distribution",
	.om_draw = distribute_nodes,
	.om_set_opts = distribution_set_opts,
	.om_probe = distribution_probe,
};

static void __init do_distribution_init(void)
{
	register_secondary_output_module(&distribution_ops);
}

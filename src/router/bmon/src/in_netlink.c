/*
 * in_netlink.c            rtnetlink input (Linux)
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
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
#include <bmon/input.h>
#include <bmon/node.h>
#include <bmon/item.h>
#include <bmon/conf.h>
#include <bmon/utils.h>

#if defined HAVE_NL && defined SYS_LINUX

static int c_notc = 0;
static const char *c_mapfile;

struct hmap {
	char *id;
	char *name;
	struct hmap *next;
};

static struct hmap *map;

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/helpers.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/filter.h>

#include <net/if.h>

static struct nl_handle nl_h = NL_INIT_HANDLE();
static struct nl_cache link_cache = RTNL_INIT_LINK_CACHE();
static struct nl_cache *qdisc_cache;
static struct nl_cache *class_cache;

struct xdata {
	item_t *intf;
	struct rtnl_link *link;
	int level;
	item_t *parent;
};

static void read_map(const char *file)
{
	FILE *f;
	char buf[1024];
	int line = 0;

	f = fopen(file, "r");
	if (f == NULL)
		quit("Could not open mapfile %s: %s\n", file, strerror(errno));

	while (fgets(buf, sizeof(buf), f)) {
		struct hmap *m;
		char *p, *id, *name;
		line++;
		if (*buf == '#' || *buf == '\r' || *buf == '\n')
			continue;

		for (p = buf; *p == ' ' || *p == '\t'; p++);
		if (*p == '\0')
			quit("%s: Parse error at line %d: Missing handle\n",
			    file, line);
		id = p;

		for (; *p != ' ' && *p != '\t' && *p != '\0'; p++);
		if (*p == '\0')
			quit("%s: Parse error at line %d: Missing name\n",
			    file, line);
		*p = '\0';

		for (++p; *p == ' ' || *p == '\t'; p++);
		name = p;
		for (; *p != ' ' && *p != '\t' && *p != '\0' &&
		       *p != '\r' && *p != '\n'; p++);
		*p = '\0';

		if (strlen(id) <= 0 || strlen(name) <= 0)
			quit("%s: Parse error at line %d: Invalid id or handle\n",
			    file, line);

		m = xcalloc(1, sizeof(*m));
		m->id = strdup(id);
		m->name = strdup(name);
		m->next = map;
		map = m;
	}

	fclose(f);
}

static const char *lookup_map(const char *handle)
{
	struct hmap *m;

	for (m = map; m; m = m->next)
		if (!strcmp(handle, m->id))
			return m->name;

	return handle;
}

static void handle_qdisc(struct nl_common *, void *);

#if 0
static void handle_filter(struct nl_common *c, void *arg)
{
	struct rtnl_filter *filter = (struct rtnl_filter *) c;
	struct xdata *x = arg;
	item_t *intf;
	char name[IFNAME_MAX];

	printf("HAHAH!\n");

	if (filter->f_handle) {
		const char *h = lookup_map(nl_handle2str(filter->f_handle));
		snprintf(name, sizeof(name), "f:%s %s", filter->f_kind, h);
	} else
		snprintf(name, sizeof(name), "f:%s", filter->f_kind);

	intf = lookup_item(get_local_node(), name, filter->f_handle, x->parent);

	if (intf == NULL)
		return;

	intf->i_link = x->intf->i_index;
	intf->i_flags |= ITEM_FLAG_IS_CHILD;
	intf->i_level = x->level;

	update_attr(intf, PACKETS, 0, 0, 0);
	update_attr(intf, BYTES, 0, 0, 0);

	notify_update(intf, NULL);
	increase_lifetime(intf, 1);
}
#endif

static void handle_class(struct nl_common *c, void *arg)
{
	struct rtnl_class *class = (struct rtnl_class *) c;
	struct rtnl_qdisc *leaf;
	struct xdata *x = arg;
	item_t *intf;
	char name[IFNAME_MAX];
	struct xdata xn = {
		.intf = x->intf,
		.link = x->link,
		.level = x->level + 1,
	};

	if (class->c_handle) {
		const char *h = lookup_map(nl_handle2str(class->c_handle));
		snprintf(name, sizeof(name), "c:%s(%s)", class->c_kind, h);
	} else
		snprintf(name, sizeof(name), "c:%s", class->c_kind);

	intf = lookup_item(get_local_node(), name, class->c_handle, x->parent);

	if (intf == NULL)
		return;

	xn.parent = intf;

	intf->i_link = x->intf->i_index;
	intf->i_flags |= ITEM_FLAG_IS_CHILD;
	intf->i_level = x->level;
	intf->i_major_attr = BYTES;
	intf->i_minor_attr = PACKETS;

	update_attr(intf, PACKETS, 0, class->c_stats.tcs_basic.packets, TX_PROVIDED);
	update_attr(intf, BYTES, 0, class->c_stats.tcs_basic.bytes, TX_PROVIDED);
	update_attr(intf, DROP, 0, class->c_stats.tcs_queue.drops, TX_PROVIDED);
	update_attr(intf, OVERLIMITS, 0, class->c_stats.tcs_queue.overlimits, TX_PROVIDED);
	update_attr(intf, BPS, 0, class->c_stats.tcs_rate_est.bps, TX_PROVIDED);
	update_attr(intf, PPS, 0, class->c_stats.tcs_rate_est.pps, TX_PROVIDED);
	update_attr(intf, QLEN, 0, class->c_stats.tcs_queue.qlen, TX_PROVIDED);
	update_attr(intf, BACKLOG, 0, class->c_stats.tcs_queue.backlog, TX_PROVIDED);
	update_attr(intf, REQUEUES, 0, class->c_stats.tcs_queue.requeues, TX_PROVIDED);

	notify_update(intf, NULL);
	increase_lifetime(intf, 1);

	if ((leaf = rtnl_class_leaf_qdisc(class, qdisc_cache)))
		handle_qdisc((struct nl_common *) leaf, &xn);
	rtnl_class_foreach_child(class, class_cache, &handle_class, &xn);
#if 0
	rtnl_class_foreach_filter_nocache(&nl_h, class, &handle_filter, &xn);
#endif
}

static void handle_qdisc(struct nl_common *c, void *arg)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) c;
	struct xdata *x = arg;
	item_t *intf;
	char name[IFNAME_MAX];
	struct xdata xn = {
		.intf = x->intf,
		.link = x->link,
		.level = x->level + 1,
	};

	if (qdisc->q_handle) {
		const char *h = lookup_map(nl_handle2str(qdisc->q_handle));
		snprintf(name, sizeof(name), "q:%s(%s)", qdisc->q_kind, h);
	} else
		snprintf(name, sizeof(name), "q:%s", qdisc->q_kind);

	intf = lookup_item(get_local_node(), name, qdisc->q_handle, x->parent);
	if (intf == NULL)
		return;

	xn.parent = intf;

	intf->i_link = x->intf->i_index;
	intf->i_flags |= ITEM_FLAG_IS_CHILD;
	intf->i_level = x->level;
	intf->i_major_attr = BYTES;
	intf->i_minor_attr = PACKETS;

	if (qdisc->q_handle == 0xffff00) {
		update_attr(intf, BYTES, qdisc->q_stats.tcs_basic.bytes, 0, RX_PROVIDED);
		update_attr(intf, PACKETS, qdisc->q_stats.tcs_basic.packets, 0, RX_PROVIDED);
		update_attr(intf, DROP, qdisc->q_stats.tcs_queue.drops, 0, RX_PROVIDED);
		update_attr(intf, OVERLIMITS, qdisc->q_stats.tcs_queue.overlimits, 0, RX_PROVIDED);
		update_attr(intf, BPS, qdisc->q_stats.tcs_rate_est.bps, 0, RX_PROVIDED);
		update_attr(intf, PPS, qdisc->q_stats.tcs_rate_est.pps, 0, RX_PROVIDED);
		update_attr(intf, QLEN, qdisc->q_stats.tcs_queue.qlen, 0, RX_PROVIDED);
		update_attr(intf, BACKLOG, qdisc->q_stats.tcs_queue.backlog, 0, RX_PROVIDED);
		update_attr(intf, REQUEUES, qdisc->q_stats.tcs_queue.requeues, 0, RX_PROVIDED);
	} else {
		update_attr(intf, BYTES, 0, qdisc->q_stats.tcs_basic.bytes, TX_PROVIDED);
		update_attr(intf, PACKETS, 0, qdisc->q_stats.tcs_basic.packets, TX_PROVIDED);
		update_attr(intf, DROP, 0, qdisc->q_stats.tcs_queue.drops, TX_PROVIDED);
		update_attr(intf, OVERLIMITS, 0, qdisc->q_stats.tcs_queue.overlimits, TX_PROVIDED);
		update_attr(intf, BPS, 0, qdisc->q_stats.tcs_rate_est.bps, TX_PROVIDED);
		update_attr(intf, PPS, 0, qdisc->q_stats.tcs_rate_est.pps, TX_PROVIDED);
		update_attr(intf, QLEN, 0, qdisc->q_stats.tcs_queue.qlen, TX_PROVIDED);
		update_attr(intf, BACKLOG, 0, qdisc->q_stats.tcs_queue.backlog, TX_PROVIDED);
		update_attr(intf, REQUEUES, 0, qdisc->q_stats.tcs_queue.requeues, TX_PROVIDED);
	}

	notify_update(intf, NULL);
	increase_lifetime(intf, 1);

	rtnl_qdisc_foreach_child(qdisc, class_cache, &handle_class, &xn);
#if 0
	rtnl_qdisc_foreach_filter_nocache(&nl_h, qdisc, &handle_filter, &xn);
#endif
}

static void handle_tc(item_t *intf, struct rtnl_link *link)
{
	struct rtnl_qdisc *qdisc;
	struct xdata x = {
		.intf = intf,
		.link = link,
		.level = 1,
		.parent = intf,
	};

	class_cache = rtnl_class_build_cache(&nl_h, link->l_index);
	if (class_cache == NULL)
		return;

	if ((qdisc = rtnl_qdisc_get_root(qdisc_cache, link->l_index)))
		handle_qdisc((struct nl_common *) qdisc, &x);
	else if ((qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, link->l_index, 0)))
		handle_qdisc((struct nl_common *) qdisc, &x);

	if ((qdisc = rtnl_qdisc_get_ingress(qdisc_cache, link->l_index)))
		handle_qdisc((struct nl_common *) qdisc, &x);

	nl_cache_destroy_and_free(class_cache);
}

static void do_link(struct nl_common *item, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *) item;
	struct rtnl_lstats *st;
	item_t *intf;

	if (!link->l_name[0])
		return;

	if (get_show_only_running() && !(link->l_flags & IFF_UP))
		return;

	intf = lookup_item(get_local_node(), link->l_name, 0, 0);

	if (NULL == intf)
		return;

	intf->i_major_attr = BYTES;
	intf->i_minor_attr = PACKETS;

	st = &link->l_stats;

	update_attr(intf, BYTES, st->ls_rx.bytes, st->ls_tx.bytes,
		    RX_PROVIDED | TX_PROVIDED);
	update_attr(intf, PACKETS, st->ls_rx.packets, st->ls_tx.packets,
		    RX_PROVIDED | TX_PROVIDED);

	update_attr(intf, ERRORS, st->ls_rx.errors, st->ls_tx.errors,
		RX_PROVIDED | TX_PROVIDED);

	update_attr(intf, DROP, st->ls_rx.dropped, st->ls_tx.dropped,
		RX_PROVIDED | TX_PROVIDED);

	update_attr(intf, FIFO, st->ls_rx_fifo_errors,
		st->ls_tx_fifo_errors, RX_PROVIDED | TX_PROVIDED);

	update_attr(intf, COMPRESSED, st->ls_rx.compressed,
		st->ls_tx.compressed, RX_PROVIDED | TX_PROVIDED);

	update_attr(intf, MULTICAST, st->ls_rx.multicast, 0, RX_PROVIDED);
	update_attr(intf, COLLISIONS, 0, st->ls_tx_collisions, TX_PROVIDED);
	update_attr(intf, LENGTH_ERRORS, st->ls_rx_length_errors, 0, RX_PROVIDED);
	update_attr(intf, OVER_ERRORS, st->ls_rx_over_errors, 0, RX_PROVIDED);
	update_attr(intf, CRC_ERRORS, st->ls_rx_crc_errors, 0, RX_PROVIDED);
	update_attr(intf, FRAME, st->ls_rx_frame_errors, 0, RX_PROVIDED);
	update_attr(intf, MISSED_ERRORS, st->ls_rx_missed_errors, 0, RX_PROVIDED);
	update_attr(intf, ABORTED_ERRORS, 0, st->ls_tx_aborted_errors, TX_PROVIDED);
	update_attr(intf, HEARTBEAT_ERRORS, 0, st->ls_tx_heartbeat_errors,
		TX_PROVIDED);
	update_attr(intf, WINDOW_ERRORS, 0, st->ls_tx_window_errors, TX_PROVIDED);
	update_attr(intf, CARRIER_ERRORS, 0, st->ls_tx_carrier_errors, TX_PROVIDED);

	if (!c_notc)
		handle_tc(intf, link);
	
	notify_update(intf, NULL);
	increase_lifetime(intf, 1);
}

static void netlink_read(void)
{
	if (nl_cache_update(&nl_h, &link_cache) < 0)
		quit("%s\n", nl_geterror());

	if (!c_notc) {
		if (qdisc_cache == NULL) {
			qdisc_cache = rtnl_qdisc_build_cache(&nl_h);
			if (qdisc_cache == NULL)
				c_notc = 1;
		} else {
			if (nl_cache_update(&nl_h, qdisc_cache) < 0)
				c_notc = 1;
		}
	}

	nl_cache_foreach(&link_cache, do_link, NULL);
}

static void netlink_shutdown(void)
{
	nl_close(&nl_h);
}

static void netlink_do_init(void)
{
	if (nl_connect(&nl_h, NETLINK_ROUTE) < 0)
		quit("%s\n", nl_geterror());

	nl_use_default_handlers(&nl_h);
}

static int netlink_probe(void)
{
	if (nl_connect(&nl_h, NETLINK_ROUTE) < 0)
		return 0;
	
	if (nl_cache_update(&nl_h, &link_cache) < 0) {
		nl_close(&nl_h);
		return 0;
	}

	nl_cache_destroy(&link_cache);
	nl_close(&nl_h);

	if (c_mapfile)
		read_map(c_mapfile);
	
	return 1;
}

static void print_help(void)
{
	printf(
	"netlink - Netlink statistic collector for Linux\n" \
	"\n" \
	"  Powerful statistic collector for Linux using netlink sockets\n" \
	"  to collect link and traffic control statistics.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    notc           Do not collect traffic control statistics\n" \
	"    map=FILE       Translate handles to map using a mapfile\n" \
	"\n" \
	"  Map File format:\n" \
	"    # comment\n" \
	"    <handle> <name> # midline comment\n");
}

static void netlink_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "notc"))
			c_notc = 1;
		else if (!strcasecmp(attrs->type, "map"))
			c_mapfile = attrs->value;
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static struct input_module netlink_ops = {
	.im_name     = "netlink",
	.im_read     = netlink_read,
	.im_shutdown = netlink_shutdown,
	.im_set_opts = netlink_set_opts,
	.im_probe = netlink_probe,
	.im_init = netlink_do_init,
};

static void __init netlink_init(void)
{
	register_input_module(&netlink_ops);
}

#endif

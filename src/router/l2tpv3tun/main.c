/*
 * L2TP control utility.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	James Chapman <jchapman@katalix.com>
 *
 */

/* Implementation note:-
 *
 * This code is written for easy import into the ip utility of the
 * iproute2 package. However, iproute2 uses libnl instead of
 * libnetlink, so this code is maintained separately for now.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
//#include <linux/if.h>
//#include <linux/if_arp.h>
//#include <linux/ip.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/ctrl.h>
#include <netlink/utils.h>

#include <linux/genetlink.h>
#include <linux/l2tp.h>

#include "utils.h"

#ifndef L2TP_CMD_ROOT
#define L2TP_CMD_ROOT "ip l2tp"
#endif

enum {
	L2TP_ADD,
	L2TP_CHG,
	L2TP_DEL,
	L2TP_GET
};

struct l2tp_parm {
	uint32_t tunnel_id;
	uint32_t peer_tunnel_id;
	uint32_t session_id;
	uint32_t peer_session_id;
	uint32_t offset;
	uint32_t peer_offset;
	enum l2tp_encap_type encap;
	uint16_t local_udp_port;
	uint16_t peer_udp_port;
	int cookie_len;
	uint8_t cookie[8];
	int peer_cookie_len;
	uint8_t peer_cookie[8];
	struct in_addr local_ip;
	struct in_addr peer_ip;

	uint16_t pw_type;
	uint16_t mtu;
	unsigned int udp_csum:1;
	unsigned int recv_seq:1;
	unsigned int send_seq:1;
	unsigned int lns_mode:1;
	unsigned int data_seq:2;
	unsigned int tunnel:1;
	unsigned int session:1;
	int reorder_timeout;
	char *ifname;
};

struct l2tp_stats {
	uint64_t data_rx_packets;
	uint64_t data_rx_bytes;
	uint64_t data_rx_errors;
	uint64_t data_rx_oos_packets;
	uint64_t data_rx_oos_discards;
	uint64_t data_tx_packets;
	uint64_t data_tx_bytes;
	uint64_t data_tx_errors;
};

struct l2tp_data {
	struct l2tp_parm config;
	struct l2tp_stats stats;
};

/* netlink socket */
static struct nl_sock *nl_sock;
static int nl_family;

/*****************************************************************************
 * Netlink actions
 *****************************************************************************/

static int create_tunnel(struct l2tp_parm *p)
{
	struct nl_msg *msg;
	int result = 0;

	msg = nlmsg_alloc();

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, NLM_F_REQUEST,
		    L2TP_CMD_TUNNEL_CREATE, L2TP_GENL_VERSION);

	nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->tunnel_id);
	nla_put_u32(msg, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	nla_put_u8(msg, L2TP_ATTR_PROTO_VERSION, 3);
	nla_put_u16(msg, L2TP_ATTR_ENCAP_TYPE, p->encap);

	nla_put_u32(msg, L2TP_ATTR_IP_SADDR, p->local_ip.s_addr);
	nla_put_u32(msg, L2TP_ATTR_IP_DADDR, p->peer_ip.s_addr);
	if (p->encap == L2TP_ENCAPTYPE_UDP) {
		nla_put_u16(msg, L2TP_ATTR_UDP_SPORT, p->local_udp_port);
		nla_put_u16(msg, L2TP_ATTR_UDP_DPORT, p->peer_udp_port);
	}

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_wait_for_ack(nl_sock);
	if (result > 0) {
		result = 0;
	}

	return result;
}

static int delete_tunnel(struct l2tp_parm *p)
{
	struct nl_msg *msg;
	int result;

	msg = nlmsg_alloc();

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, NLM_F_REQUEST,
		    L2TP_CMD_TUNNEL_DELETE, L2TP_GENL_VERSION);

	nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->tunnel_id);

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_wait_for_ack(nl_sock);
	if (result > 0) {
		result = 0;
	}

	return result;
}

static int create_session(struct l2tp_parm *p)
{
	struct nl_msg *msg;
	int result = 0;

	msg = nlmsg_alloc();

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, NLM_F_REQUEST,
		    L2TP_CMD_SESSION_CREATE, L2TP_GENL_VERSION);

	nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->tunnel_id);
	nla_put_u32(msg, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	nla_put_u32(msg, L2TP_ATTR_SESSION_ID, p->session_id);
	nla_put_u32(msg, L2TP_ATTR_PEER_SESSION_ID, p->peer_session_id);
	nla_put_u16(msg, L2TP_ATTR_PW_TYPE, p->pw_type);
	if (p->mtu) {
		nla_put_u16(msg, L2TP_ATTR_MTU, p->mtu);
	}
	nla_put_u8(msg, L2TP_ATTR_RECV_SEQ, p->recv_seq);
	nla_put_u8(msg, L2TP_ATTR_SEND_SEQ, p->send_seq);
	nla_put_u8(msg, L2TP_ATTR_LNS_MODE, p->lns_mode);
	if (p->data_seq) {
		nla_put_u8(msg, L2TP_ATTR_DATA_SEQ, p->data_seq);
	}
	if (p->reorder_timeout) {
		nla_put_msecs(msg, L2TP_ATTR_RECV_TIMEOUT, p->reorder_timeout);
	}
	if (p->offset) {
		nla_put_u16(msg, L2TP_ATTR_OFFSET, p->offset);
	}
	if (p->cookie_len) {
		nla_put(msg, L2TP_ATTR_COOKIE, p->cookie_len, p->cookie);
	}
	if (p->peer_cookie_len) {
		nla_put(msg, L2TP_ATTR_PEER_COOKIE, p->peer_cookie_len, p->peer_cookie);
	}
	if (p->ifname && p->ifname[0]) {
		nla_put_string(msg, L2TP_ATTR_IFNAME, p->ifname);
	}

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_wait_for_ack(nl_sock);
	if (result > 0) {
		result = 0;
	}

	return result;
}

static int delete_session(struct l2tp_parm *p)
{
	struct nl_msg *msg;
	int result = 0;

	msg = nlmsg_alloc();

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, NLM_F_REQUEST,
		    L2TP_CMD_SESSION_DELETE, L2TP_GENL_VERSION);

	nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->tunnel_id);
	nla_put_u32(msg, L2TP_ATTR_SESSION_ID, p->session_id);

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_wait_for_ack(nl_sock);
	if (result > 0) {
		result = 0;
	}

	return result;
}

static void print_cookie(char *name, uint8_t *cookie, int len)
{
	printf("  %s %02x%02x%02x%02x", name,
	       cookie[0], cookie[1],
	       cookie[2], cookie[3]);
	if (len == 8)
		printf("%02x%02x%02x%02x",
		       cookie[4], cookie[5],
		       cookie[6], cookie[7]);
}

static void print_tunnel(struct l2tp_data *data)
{
	struct l2tp_parm *p = &data->config;

	printf("Tunnel %u, encap %s\n",
	       p->tunnel_id,
	       p->encap == L2TP_ENCAPTYPE_UDP ? "UDP" :
	       p->encap == L2TP_ENCAPTYPE_IP ? "IP" : "??");
	printf("  From %s ", inet_ntoa(p->local_ip));
	printf("to %s\n", inet_ntoa(p->peer_ip));
	printf("  Peer tunnel %u\n",
	       p->peer_tunnel_id);

	if (p->encap == L2TP_ENCAPTYPE_UDP)
		printf("  UDP source / dest ports: %hu/%hu\n",
		       p->local_udp_port, p->peer_udp_port);
}

static void print_session(struct l2tp_data *data)
{
	struct l2tp_parm *p = &data->config;

	printf("Session %u in tunnel %u\n",
	       p->session_id, p->tunnel_id);
	printf("  Peer session %u, tunnel %u\n",
	       p->peer_session_id, p->peer_tunnel_id);

	if (p->ifname != NULL) {
		printf("  interface name: %s\n", p->ifname);
	}
	printf("  offset %u, peer offset %u\n",
	       p->offset, p->peer_offset);
	if (p->cookie_len > 0)
		print_cookie("cookie", p->cookie, p->cookie_len);
	if (p->peer_cookie_len > 0)
		print_cookie("peer cookie", p->peer_cookie, p->peer_cookie_len);

	if (p->reorder_timeout != 0) {
		printf("  reorder timeout: %u\n", p->reorder_timeout);
	}
}

static int nl_get_response(struct nl_msg *msg, void *arg)
{
	struct l2tp_data *data = arg;
	struct l2tp_parm *p = &data->config;
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[L2TP_ATTR_MAX + 1];
	struct nlattr *nla_stats;
	int result = 0;

	/* Validate message and parse attributes */
	genlmsg_parse(nlh, 0, attrs, L2TP_ATTR_MAX, NULL);
	if (nlh->nlmsg_type == NLMSG_ERROR) {
		result = -EBADMSG;
		goto out;
	}

	if (attrs[L2TP_ATTR_PW_TYPE])
		p->pw_type = nla_get_u16(attrs[L2TP_ATTR_PW_TYPE]);
	if (attrs[L2TP_ATTR_ENCAP_TYPE])
		p->encap = nla_get_u16(attrs[L2TP_ATTR_ENCAP_TYPE]);
	if (attrs[L2TP_ATTR_OFFSET])
		p->offset = nla_get_u16(attrs[L2TP_ATTR_OFFSET]);
	if (attrs[L2TP_ATTR_DATA_SEQ])
		p->data_seq = nla_get_u16(attrs[L2TP_ATTR_DATA_SEQ]);
	if (attrs[L2TP_ATTR_CONN_ID])
		p->tunnel_id = nla_get_u32(attrs[L2TP_ATTR_CONN_ID]);
	if (attrs[L2TP_ATTR_PEER_CONN_ID])
		p->peer_tunnel_id = nla_get_u32(attrs[L2TP_ATTR_PEER_CONN_ID]);
	if (attrs[L2TP_ATTR_SESSION_ID])
		p->session_id = nla_get_u32(attrs[L2TP_ATTR_SESSION_ID]);
	if (attrs[L2TP_ATTR_PEER_SESSION_ID])
		p->peer_session_id = nla_get_u32(attrs[L2TP_ATTR_PEER_SESSION_ID]);
	if (attrs[L2TP_ATTR_UDP_CSUM])
		p->udp_csum = nla_get_u8(attrs[L2TP_ATTR_UDP_CSUM]);
	if (attrs[L2TP_ATTR_COOKIE]) {
		nla_memcpy(&p->cookie[0], attrs[L2TP_ATTR_COOKIE], sizeof(p->cookie));
		p->cookie_len = nla_len(attrs[L2TP_ATTR_COOKIE]);
	}
	if (attrs[L2TP_ATTR_PEER_COOKIE]) {
		nla_memcpy(&p->peer_cookie[0], attrs[L2TP_ATTR_PEER_COOKIE], sizeof(p->peer_cookie));
		p->peer_cookie_len = nla_len(attrs[L2TP_ATTR_PEER_COOKIE]);
	}
	if (attrs[L2TP_ATTR_RECV_SEQ])
		p->recv_seq = nla_get_u8(attrs[L2TP_ATTR_RECV_SEQ]);
	if (attrs[L2TP_ATTR_SEND_SEQ])
		p->send_seq = nla_get_u8(attrs[L2TP_ATTR_SEND_SEQ]);
	if (attrs[L2TP_ATTR_RECV_TIMEOUT])
		p->reorder_timeout = nla_get_msecs(attrs[L2TP_ATTR_RECV_TIMEOUT]);
	if (attrs[L2TP_ATTR_IP_SADDR])
		p->local_ip.s_addr = nla_get_u32(attrs[L2TP_ATTR_IP_SADDR]);
	if (attrs[L2TP_ATTR_IP_DADDR])
		p->peer_ip.s_addr = nla_get_u32(attrs[L2TP_ATTR_IP_DADDR]);
	if (attrs[L2TP_ATTR_UDP_SPORT])
		p->local_udp_port = nla_get_u16(attrs[L2TP_ATTR_UDP_SPORT]);
	if (attrs[L2TP_ATTR_UDP_DPORT])
		p->peer_udp_port = nla_get_u16(attrs[L2TP_ATTR_UDP_DPORT]);
	if (attrs[L2TP_ATTR_MTU])
		p->mtu = nla_get_u16(attrs[L2TP_ATTR_MTU]);
	if (attrs[L2TP_ATTR_IFNAME])
		p->ifname = nla_get_string(attrs[L2TP_ATTR_IFNAME]);

	nla_stats = attrs[L2TP_ATTR_STATS];
	if (nla_stats) {
		struct nlattr *tb[L2TP_ATTR_STATS_MAX + 1];

		result = nla_parse_nested(tb, L2TP_ATTR_STATS_MAX, nla_stats, NULL);
		if (result < 0)
			goto out;

		if (tb[L2TP_ATTR_TX_PACKETS])
			data->stats.data_tx_packets = nla_get_u64(tb[L2TP_ATTR_TX_PACKETS]);
		if (tb[L2TP_ATTR_TX_BYTES])
			data->stats.data_tx_bytes = nla_get_u64(tb[L2TP_ATTR_TX_BYTES]);
		if (tb[L2TP_ATTR_TX_ERRORS])
			data->stats.data_tx_errors = nla_get_u64(tb[L2TP_ATTR_TX_ERRORS]);
		if (tb[L2TP_ATTR_RX_PACKETS])
			data->stats.data_rx_packets = nla_get_u64(tb[L2TP_ATTR_RX_PACKETS]);
		if (tb[L2TP_ATTR_RX_BYTES])
			data->stats.data_rx_bytes = nla_get_u64(tb[L2TP_ATTR_RX_BYTES]);
		if (tb[L2TP_ATTR_RX_ERRORS])
			data->stats.data_rx_errors = nla_get_u64(tb[L2TP_ATTR_RX_ERRORS]);
		if (tb[L2TP_ATTR_RX_SEQ_DISCARDS])
			data->stats.data_rx_oos_discards = nla_get_u64(tb[L2TP_ATTR_RX_SEQ_DISCARDS]);
		if (tb[L2TP_ATTR_RX_OOS_PACKETS])
			data->stats.data_rx_oos_packets = nla_get_u64(tb[L2TP_ATTR_RX_OOS_PACKETS]);
	}

	result = 0;

out:
	return result;
}

static int nl_session_get_response(struct nl_msg *msg, void *arg)
{
	int ret = nl_get_response(msg, arg);

	if (ret == 0)
		print_session(arg);

	return ret;
}

static int nl_tunnel_get_response(struct nl_msg *msg, void *arg)
{
	int ret = nl_get_response(msg, arg);

	if (ret == 0)
		print_tunnel(arg);

	return ret;
}

static int get_session(struct l2tp_data *p)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int result = -EPROTONOSUPPORT;
	enum nl_cb_kind cb_kind = NL_CB_DEFAULT;
	int flags = NLM_F_DUMP;

	if (nl_family <= 0) {
		goto out;
	}

	cb = nl_cb_alloc(cb_kind);
	if (!cb) {
		goto out;
	}
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, nl_session_get_response, p);

	msg = nlmsg_alloc();

	if (p->config.tunnel_id && p->config.session_id)
		flags = NLM_F_REQUEST;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, flags,
		    L2TP_CMD_SESSION_GET, L2TP_GENL_VERSION);

	if (p->config.tunnel_id && p->config.session_id) {
		nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->config.tunnel_id);
		nla_put_u32(msg, L2TP_ATTR_SESSION_ID, p->config.session_id);
	}

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_recvmsgs(nl_sock, cb);
	if (result > 0) {
		result = nl_wait_for_ack(nl_sock);
	}

	if (result > 0) {
		result = 0;
	}

	nl_cb_put(cb);

out:
	return result;
}

static int get_tunnel(struct l2tp_data *p)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int result = -EPROTONOSUPPORT;
	enum nl_cb_kind cb_kind = NL_CB_DEFAULT;
	int flags = NLM_F_DUMP;

	if (nl_family <= 0) {
		goto out;
	}

	cb = nl_cb_alloc(cb_kind);
	if (!cb) {
		goto out;
	}
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, nl_tunnel_get_response, p);

	msg = nlmsg_alloc();

	if (p->config.tunnel_id)
		flags = NLM_F_REQUEST;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, nl_family, 0, flags,
		    L2TP_CMD_TUNNEL_GET, L2TP_GENL_VERSION);

	if (p->config.tunnel_id)
		nla_put_u32(msg, L2TP_ATTR_CONN_ID, p->config.tunnel_id);

	nl_send_auto_complete(nl_sock, msg);

	nlmsg_free(msg);

	result = nl_recvmsgs(nl_sock, cb);
	if (result > 0) {
		result = nl_wait_for_ack(nl_sock);
	}

	if (result > 0) {
		result = 0;
	}

	nl_cb_put(cb);

out:
	return result;
}

/*****************************************************************************
 * Command parser
 *****************************************************************************/

static int hex(char ch)
{
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	return -1;
}

static int hex2mem(char *buf, uint8_t *mem, int count)
{
	int i, j;
	int c;

	for (i = 0, j = 0; i < count; i++, j += 2) {
		c = hex(buf[j]);
		if (c < 0)
			goto err;

		mem[i] = c << 4;

		c = hex(buf[j + 1]);
		if (c < 0)
			goto err;

		mem[i] |= c;
	}

	return 0;

err:
	return -1;
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: %s add tunnel\n", L2TP_CMD_ROOT);
	fprintf(stderr, "          remote ADDR local ADDR\n");
	fprintf(stderr, "          tunnel_id ID peer_tunnel_id ID\n");
	fprintf(stderr, "          [ encap { ip | udp } ]\n");
	fprintf(stderr, "          [ udp_sport PORT ] [ udp_dport PORT ]\n");
	fprintf(stderr, "       %s add session\n", L2TP_CMD_ROOT);
	fprintf(stderr, "          tunnel_id ID\n");
	fprintf(stderr, "          session_id ID peer_session_id ID\n");
	fprintf(stderr, "          [ cookie HEXSTR ] [ peer_cookie HEXSTR ]\n");
	fprintf(stderr, "          [ offset OFFSET ] [ peer_offset OFFSET ]\n");
	fprintf(stderr, "          [ ifname IFNAME ]\n");
	fprintf(stderr, "       %s del tunnel tunnel_id ID\n", L2TP_CMD_ROOT);
	fprintf(stderr, "       %s del session tunnel_id ID session_id ID\n", L2TP_CMD_ROOT);
	fprintf(stderr, "       %s show tunnel [ tunnel_id ID ]\n", L2TP_CMD_ROOT);
	fprintf(stderr, "       %s show session [ tunnel_id ID ] [ session_id ID ]\n", L2TP_CMD_ROOT);
	fprintf(stderr, "\n");
	fprintf(stderr, "Where: NAME   := STRING\n");
	fprintf(stderr, "       ADDR   := { IP_ADDRESS | any }\n");
	fprintf(stderr, "       PORT   := { 0..65535 }\n");
	fprintf(stderr, "       ID     := { 1..4294967295 }\n");
	fprintf(stderr, "       HEXSTR := { 8 or 16 hex digits (4 / 8 bytes) }\n\n");
	fprintf(stderr, "Developed for the OpenL2TP project.\n");
	fprintf(stderr, "http://www.openl2tp.org\n");
	exit(-1);
}

static int parse_args(int argc, char **argv, int cmd, struct l2tp_parm *p)
{
	memset(p, 0, sizeof(*p));

	if (argc == 0)
		usage();

	while (argc > 0) {
		if (strcmp(*argv, "encap") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ip") == 0) {
				p->encap = L2TP_ENCAPTYPE_IP;
			} else if (strcmp(*argv, "udp") == 0) {
				p->encap = L2TP_ENCAPTYPE_UDP;
			} else {
				fprintf(stderr, "Unknown tunnel encapsulation.\n");
				exit(-1);
			}
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			p->peer_ip.s_addr = get_addr32(*argv);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			p->local_ip.s_addr = get_addr32(*argv);
		} else if ((strcmp(*argv, "tunnel_id") == 0) ||
			   (strcmp(*argv, "tid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->tunnel_id = uval;
		} else if ((strcmp(*argv, "peer_tunnel_id") == 0) ||
			   (strcmp(*argv, "ptid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_tunnel_id = uval;
		} else if ((strcmp(*argv, "session_id") == 0) ||
			   (strcmp(*argv, "sid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->session_id = uval;
		} else if ((strcmp(*argv, "peer_session_id") == 0) ||
			   (strcmp(*argv, "psid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_session_id = uval;
		} else if (strcmp(*argv, "udp_sport") == 0) {
			__u16 uval;
			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->local_udp_port = uval;
		} else if (strcmp(*argv, "udp_dport") == 0) {
			__u16 uval;
			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->peer_udp_port = uval;
		} else if (strcmp(*argv, "offset") == 0) {
			__u8 uval;
			NEXT_ARG();
			if (get_u8(&uval, *argv, 0))
				invarg("invalid offset\n", *argv);
			p->offset = uval;
		} else if (strcmp(*argv, "peer_offset") == 0) {
			__u8 uval;
			NEXT_ARG();
			if (get_u8(&uval, *argv, 0))
				invarg("invalid offset\n", *argv);
			p->peer_offset = uval;
		} else if (strcmp(*argv, "cookie") == 0) {
			int slen;
			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->cookie_len = slen / 2;
			if (hex2mem(*argv, p->cookie, p->cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "peer_cookie") == 0) {
			int slen;
			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->peer_cookie_len = slen / 2;
			if (hex2mem(*argv, p->peer_cookie, p->peer_cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "ifname") == 0) {
			NEXT_ARG();
			p->ifname = *argv;
		} else if (strcmp(*argv, "tunnel") == 0) {
			p->tunnel = 1;
		} else if (strcmp(*argv, "session") == 0) {
			p->session = 1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			fprintf(stderr, "Unknown command: %s\n", *argv);
			usage();
		}

		argc--; argv++;
	}

	return 0;
}


static int do_add(int argc, char **argv)
{
	struct l2tp_parm p;
	int ret = 0;

	if (parse_args(argc, argv, L2TP_ADD, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if (p.tunnel_id == 0)
		missarg("tunnel_id");

	/* session_id and peer_session_id must be provided for sessions */
	if ((p.session) && (p.peer_session_id == 0))
		missarg("peer_session_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	/* peer_tunnel_id is needed for tunnels */
	if ((p.tunnel) && (p.peer_tunnel_id == 0))
		missarg("peer_tunnel_id");

	if (p.tunnel) {
		if (p.local_ip.s_addr == 0)
			missarg("local");

		if (p.peer_ip.s_addr == 0)
			missarg("remote");

		if (p.encap == L2TP_ENCAPTYPE_UDP) {
			if (p.local_udp_port == 0)
				missarg("udp_sport");
			if (p.peer_udp_port == 0)
				missarg("udp_dport");
		}

		ret = create_tunnel(&p);
	}

	if (p.session) {
		/* Only ethernet pseudowires supported */
		p.pw_type = L2TP_PWTYPE_ETH;

		ret = create_session(&p);
	}

	return ret;
}

static int do_del(int argc, char **argv)
{
	struct l2tp_parm p;

	if (parse_args(argc, argv, L2TP_DEL, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if ((p.tunnel) && (p.tunnel_id == 0))
		missarg("tunnel_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	if (p.session_id)
		return delete_session(&p);
	else
		return delete_tunnel(&p);

	return -1;
}

static int do_show(int argc, char **argv)
{
	struct l2tp_data data;
	struct l2tp_parm *p = &data.config;

	if (parse_args(argc, argv, L2TP_GET, p) < 0)
		return -1;

	if (!p->tunnel && !p->session)
		missarg("tunnel or session");

	if (p->session)
		get_session(&data);
	else
		get_tunnel(&data);

	return 0;
}

int do_ipl2tp(int argc, char **argv)
{
	nl_sock = nl_socket_alloc();
	if (!nl_sock) {
		perror("nl_socket_alloc");
		return 1;
	}

	if (nl_connect(nl_sock, NETLINK_GENERIC) < 0) {
		perror("nl_connect");
		return 1;
	}

	nl_family = genl_ctrl_resolve(nl_sock, L2TP_GENL_NAME);
	if (nl_family < 0) {
		fprintf(stderr, "L2TP netlink support unavailable.\n");
		return 1;
	}

	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return do_add(argc-1, argv+1);
		if (matches(*argv, "del") == 0)
			return do_del(argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return do_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();

		fprintf(stderr, "Command \"%s\" is unknown, try \"%s help\".\n", *argv, L2TP_CMD_ROOT);
		exit(-1);
	}

	usage();
}

int main(int argc, char **argv)
{
	return do_ipl2tp(argc - 1, argv + 1);
}

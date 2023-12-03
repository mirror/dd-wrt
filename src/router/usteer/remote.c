/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <libubox/vlist.h>
#include <libubox/avl-cmp.h>
#include <libubox/usock.h>
#include "usteer.h"
#include "remote.h"
#include "node.h"

static uint32_t local_id;
static struct uloop_fd remote_fd;
static struct uloop_timeout remote_timer;
static struct uloop_timeout reload_timer;

static struct blob_buf buf;
static uint32_t msg_seq;

struct interface {
	struct vlist_node node;
	int ifindex;
};

static void
interfaces_update_cb(struct vlist_tree *tree,
		     struct vlist_node *node_new,
		     struct vlist_node *node_old);

static int remote_host_cmp(const void *k1, const void *k2, void *ptr)
{
	unsigned long v1 = (unsigned long) k1;
	unsigned long v2 = (unsigned long) k2;

	return v2 - v1;
}

static VLIST_TREE(interfaces, avl_strcmp, interfaces_update_cb, true, true);
LIST_HEAD(remote_nodes);
AVL_TREE(remote_hosts, remote_host_cmp, false, NULL);

static const char *
interface_name(struct interface *iface)
{
	return iface->node.avl.key;
}

static void
interface_check(struct interface *iface)
{
	iface->ifindex = if_nametoindex(interface_name(iface));
	uloop_timeout_set(&reload_timer, 1);
}

static void
interface_init(struct interface *iface)
{
	interface_check(iface);
}

static void
interface_free(struct interface *iface)
{
	avl_delete(&interfaces.avl, &iface->node.avl);
	free(iface);
}

static void
interfaces_update_cb(struct vlist_tree *tree,
		     struct vlist_node *node_new,
		     struct vlist_node *node_old)
{
	struct interface *iface;

	if (node_new && node_old) {
		iface = container_of(node_new, struct interface, node);
		free(iface);
		iface = container_of(node_old, struct interface, node);
		interface_check(iface);
	} else if (node_old) {
		iface = container_of(node_old, struct interface, node);
		interface_free(iface);
	} else {
		iface = container_of(node_new, struct interface, node);
		interface_init(iface);
	}
}

void usteer_interface_add(const char *name)
{
	struct interface *iface;
	char *name_buf;

	iface = calloc_a(sizeof(*iface), &name_buf, strlen(name) + 1);
	strcpy(name_buf, name);
	vlist_add(&interfaces, &iface->node, name_buf);
}

void config_set_interfaces(struct blob_attr *data)
{
	struct blob_attr *cur;
	int rem;

	if (!data)
		return;

	if (!blobmsg_check_attr_list(data, BLOBMSG_TYPE_STRING))
		return;

	vlist_update(&interfaces);
	blobmsg_for_each_attr(cur, data, rem) {
		usteer_interface_add(blobmsg_data(cur));
	}
	vlist_flush(&interfaces);
}

void config_get_interfaces(struct blob_buf *buf)
{
	struct interface *iface;
	void *c;

	c = blobmsg_open_array(buf, "interfaces");
	vlist_for_each_element(&interfaces, iface, node) {
		blobmsg_add_string(buf, NULL, interface_name(iface));
	}
	blobmsg_close_array(buf, c);
}

static void
interface_add_station(struct usteer_remote_node *node, struct blob_attr *data)
{
	struct sta *sta;
	struct sta_info *si, *local_si;
	struct apmsg_sta msg;
	struct usteer_node *local_node;
	bool create;
	bool connect_change;

	if (!parse_apmsg_sta(&msg, data)) {
		MSG(DEBUG, "Cannot parse station in message\n");
		return;
	}

	if (msg.timeout <= 0) {
		MSG(DEBUG, "Refuse to add an already expired station entry\n");
		return;
	}

	sta = usteer_sta_get(msg.addr, true);
	if (!sta)
		return;

	si = usteer_sta_info_get(sta, &node->node, &create);
	if (!si)
		return;

	connect_change = si->connected != msg.connected;
	si->connected = msg.connected;
	si->signal = msg.signal;
	si->seen = current_time - msg.seen;
	si->last_connected = current_time - msg.last_connected;

	/* Check if client roamed to this foreign node */
	if ((connect_change || create) && si->connected == STA_CONNECTED) {
		for_each_local_node(local_node) {
			local_si = usteer_sta_info_get(sta, local_node, NULL);
			if (!local_si)
				continue;

			if (current_time - local_si->last_connected < config.roam_process_timeout) {
				node->node.roam_events.target++;
				break;
			}
		}
	}

	usteer_sta_info_update_timeout(si, msg.timeout);
}

static void
remote_node_free(struct usteer_remote_node *node)
{
	struct usteer_remote_host *host = node->host;

	list_del(&node->list);
	list_del(&node->host_list);
	usteer_sta_node_cleanup(&node->node);
	usteer_measurement_report_node_cleanup(&node->node);
	free(node);

	if (!list_empty(&host->nodes))
		return;

	avl_delete(&remote_hosts, &host->avl);
	free(host->addr);
	free(host);
}

static struct usteer_remote_host *
interface_get_host(const char *addr, unsigned long id)
{
	struct usteer_remote_host *host;

	host = avl_find_element(&remote_hosts, (void *)id, host, avl);
	if (host)
		goto out;

	host = calloc(1, sizeof(*host));
	host->avl.key = (void *)id;
	INIT_LIST_HEAD(&host->nodes);
	avl_insert(&remote_hosts, &host->avl);

out:
	if (host->addr && !strcmp(host->addr, addr))
		return host;

	free(host->addr);
	host->addr = strdup(addr);

	return host;
}

static struct usteer_remote_node *
interface_get_node(struct usteer_remote_host *host, const char *name)
{
	struct usteer_remote_node *node;
	int addr_len = strlen(host->addr);
	char *buf;

	list_for_each_entry(node, &host->nodes, host_list)
		if (!strcmp(node->name, name))
			return node;

	node = calloc_a(sizeof(*node), &buf, addr_len + 1 + strlen(name) + 1);
	node->node.type = NODE_TYPE_REMOTE;
	node->node.created = current_time;

	sprintf(buf, "%s#%s", host->addr, name);
	node->node.avl.key = buf;
	node->name = buf + addr_len + 1;
	node->host = host;
	INIT_LIST_HEAD(&node->node.sta_info);
	INIT_LIST_HEAD(&node->node.measurements);

	list_add_tail(&node->list, &remote_nodes);
	list_add_tail(&node->host_list, &host->nodes);

	return node;
}

static void
interface_add_node(struct usteer_remote_host *host, struct blob_attr *data)
{
	struct usteer_remote_node *node;
	struct apmsg_node msg;
	struct blob_attr *cur;
	int rem;

	if (!parse_apmsg_node(&msg, data)) {
		MSG(DEBUG, "Cannot parse node in message\n");
		return;
	}

	node = interface_get_node(host, msg.name);
	node->check = 0;
	node->node.freq = msg.freq;
	node->node.channel = msg.channel;
	node->node.op_class = msg.op_class;
	node->node.n_assoc = msg.n_assoc;
	node->node.max_assoc = msg.max_assoc;
	node->node.noise = msg.noise;
	node->node.load = msg.load;

	memcpy(node->node.bssid, msg.bssid, sizeof(node->node.bssid));

	snprintf(node->node.ssid, sizeof(node->node.ssid), "%s", msg.ssid);
	usteer_node_set_blob(&node->node.rrm_nr, msg.rrm_nr);
	usteer_node_set_blob(&node->node.node_info, msg.node_info);

	blob_for_each_attr(cur, msg.stations, rem)
		interface_add_station(node, cur);
}

static void
interface_recv_msg(struct interface *iface, char *addr_str, void *buf, int len)
{
	struct usteer_remote_host *host;
	struct blob_attr *data = buf;
	struct apmsg msg;
	struct blob_attr *cur;
	int rem;

	if (config.local_mode)
		return;

	if (blob_pad_len(data) != len) {
		MSG(DEBUG, "Invalid message length (header: %d, real: %d)\n", blob_pad_len(data), len);
		return;
	}

	if (!parse_apmsg(&msg, data)) {
		MSG(DEBUG, "Missing fields in message\n");
		return;
	}

	if (msg.id == local_id)
		return;

	MSG(NETWORK, "Received message on %s (id=%08x->%08x seq=%d len=%d)\n",
		interface_name(iface), msg.id, local_id, msg.seq, len);

	host = interface_get_host(addr_str, msg.id);
	usteer_node_set_blob(&host->host_info, msg.host_info);

	blob_for_each_attr(cur, msg.nodes, rem)
		interface_add_node(host, cur);
}

static struct interface *
interface_find_by_ifindex(int index)
{
	struct interface *iface;

	vlist_for_each_element(&interfaces, iface, node) {
		if (iface->ifindex == index)
			return iface;
	}

	return NULL;
}

static void
interface_recv_v4(struct uloop_fd *u, unsigned int events)
{
	static char buf[APMGR_BUFLEN];
	static char cmsg_buf[( CMSG_SPACE(sizeof(struct in_pktinfo)) + sizeof(int)) + 1];
	static struct sockaddr_in sin;
	char addr_str[INET_ADDRSTRLEN];
	static struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	static struct msghdr msg = {
		.msg_name = &sin,
		.msg_namelen = sizeof(sin),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsg_buf,
		.msg_controllen = sizeof(cmsg_buf),
	};
	struct cmsghdr *cmsg;
	int len;

	do {
		struct in_pktinfo *pkti = NULL;
		struct interface *iface;

		len = recvmsg(u->fd, &msg, 0);
		if (len < 0) {
			switch (errno) {
			case EAGAIN:
				return;
			case EINTR:
				continue;
			default:
				perror("recvmsg");
				uloop_fd_delete(u);
				return;
			}
		}

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_type != IP_PKTINFO)
				continue;

			pkti = (struct in_pktinfo *) CMSG_DATA(cmsg);
		}

		if (!pkti) {
			MSG(DEBUG, "Received packet without ifindex\n");
			continue;
		}

		iface = interface_find_by_ifindex(pkti->ipi_ifindex);
		if (!iface) {
			MSG(DEBUG, "Received packet from unconfigured interface %d\n", pkti->ipi_ifindex);
			continue;
		}

		inet_ntop(AF_INET, &sin.sin_addr, addr_str, sizeof(addr_str));

		interface_recv_msg(iface, addr_str, buf, len);
	} while (1);
}


static void interface_recv_v6(struct uloop_fd *u, unsigned int events){
	static char buf[APMGR_BUFLEN];
	static char cmsg_buf[( CMSG_SPACE(sizeof(struct in6_pktinfo)) + sizeof(int)) + 1];
	static struct sockaddr_in6 sin;
	static struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	static struct msghdr msg = {
		.msg_name = &sin,
		.msg_namelen = sizeof(sin),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsg_buf,
		.msg_controllen = sizeof(cmsg_buf),
	};
	struct cmsghdr *cmsg;
	char addr_str[INET6_ADDRSTRLEN];
	int len;

	do {
		struct in6_pktinfo *pkti = NULL;
		struct interface *iface;

		len = recvmsg(u->fd, &msg, 0);
		if (len < 0) {
			switch (errno) {
			case EAGAIN:
				return;
			case EINTR:
				continue;
			default:
				perror("recvmsg");
				uloop_fd_delete(u);
				return;
			}
		}

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_type != IPV6_PKTINFO)
				continue;

			pkti = (struct in6_pktinfo *) CMSG_DATA(cmsg);
		}

		if (!pkti) {
			MSG(DEBUG, "Received packet without ifindex\n");
			continue;
		}

		iface = interface_find_by_ifindex(pkti->ipi6_ifindex);
		if (!iface) {
			MSG(DEBUG, "Received packet from unconfigured interface %d\n", pkti->ipi6_ifindex);
			continue;
		}

		inet_ntop(AF_INET6, &sin.sin6_addr, addr_str, sizeof(addr_str));
		if (sin.sin6_addr.s6_addr[0] == 0) {
			/* IPv4 mapped address. Ignore. */
			continue;
		}

		interface_recv_msg(iface, addr_str, buf, len);
	} while (1);
}

static void interface_send_msg_v4(struct interface *iface, struct blob_attr *data)
{
	static size_t cmsg_data[( CMSG_SPACE(sizeof(struct in_pktinfo)) / sizeof(size_t)) + 1];
	static struct sockaddr_in a;
	static struct iovec iov;
	static struct msghdr m = {
		.msg_name = (struct sockaddr *) &a,
		.msg_namelen = sizeof(a),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsg_data,
		.msg_controllen = CMSG_LEN(sizeof(struct in_pktinfo)),
	};
	struct in_pktinfo *pkti;
	struct cmsghdr *cmsg;

	a.sin_family = AF_INET;
	a.sin_port = htons(APMGR_PORT);
	a.sin_addr.s_addr = ~0;

	memset(cmsg_data, 0, sizeof(cmsg_data));
	cmsg = CMSG_FIRSTHDR(&m);
	cmsg->cmsg_len = m.msg_controllen;
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type = IP_PKTINFO;

	pkti = (struct in_pktinfo *) CMSG_DATA(cmsg);
	pkti->ipi_ifindex = iface->ifindex;

	iov.iov_base = data;
	iov.iov_len = blob_pad_len(data);

	if (sendmsg(remote_fd.fd, &m, 0) < 0)
		perror("sendmsg");
}


static void interface_send_msg_v6(struct interface *iface, struct blob_attr *data) {
	static struct sockaddr_in6 groupSock = {};

	groupSock.sin6_family = AF_INET6;
	inet_pton(AF_INET6, APMGR_V6_MCAST_GROUP, &groupSock.sin6_addr);
	groupSock.sin6_port = htons(APMGR_PORT);

	setsockopt(remote_fd.fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &iface->ifindex, sizeof(iface->ifindex));

	if (sendto(remote_fd.fd, data, blob_pad_len(data), 0, (const struct sockaddr *)&groupSock, sizeof(groupSock)) < 0)
		perror("sendmsg");
}

static void interface_send_msg(struct interface *iface, struct blob_attr *data){
	if (config.ipv6) {
		interface_send_msg_v6(iface, data);
	} else {
		interface_send_msg_v4(iface, data);
	}
}

static void usteer_send_sta_info(struct sta_info *sta)
{
	int seen = current_time - sta->seen;
	int last_connected = !!sta->connected ? 0 : current_time - sta->last_connected;
	void *c;

	c = blob_nest_start(&buf, 0);
	blob_put(&buf, APMSG_STA_ADDR, sta->sta->addr, 6);
	blob_put_int8(&buf, APMSG_STA_CONNECTED, !!sta->connected);
	blob_put_int32(&buf, APMSG_STA_SIGNAL, sta->signal);
	blob_put_int32(&buf, APMSG_STA_SEEN, seen);
	blob_put_int32(&buf, APMSG_STA_LAST_CONNECTED, last_connected);
	blob_put_int32(&buf, APMSG_STA_TIMEOUT, config.local_sta_timeout - seen);
	blob_nest_end(&buf, c);
}

static void usteer_send_node(struct usteer_node *node, struct sta_info *sta)
{
	void *c, *s, *r;

	c = blob_nest_start(&buf, 0);

	blob_put_string(&buf, APMSG_NODE_NAME, usteer_node_name(node));
	blob_put_string(&buf, APMSG_NODE_SSID, node->ssid);
	blob_put_int32(&buf, APMSG_NODE_FREQ, node->freq);
	blob_put_int32(&buf, APMSG_NODE_NOISE, node->noise);
	blob_put_int32(&buf, APMSG_NODE_LOAD, node->load);
	blob_put_int32(&buf, APMSG_NODE_N_ASSOC, node->n_assoc);
	blob_put_int32(&buf, APMSG_NODE_MAX_ASSOC, node->max_assoc);
	blob_put_int32(&buf, APMSG_NODE_OP_CLASS, node->op_class);
	blob_put_int32(&buf, APMSG_NODE_CHANNEL, node->channel);
	blob_put_int32(&buf, APMSG_NODE_N, node->n);
	blob_put_int32(&buf, APMSG_NODE_VHT, node->vht);
	blob_put_int32(&buf, APMSG_NODE_HE, node->he);
	blob_put_int32(&buf, APMSG_NODE_CW, node->cw);
	blob_put(&buf, APMSG_NODE_BSSID, node->bssid, sizeof(node->bssid));
	if (node->rrm_nr) {
		r = blob_nest_start(&buf, APMSG_NODE_RRM_NR);
		blobmsg_add_field(&buf, BLOBMSG_TYPE_ARRAY, "",
				  blobmsg_data(node->rrm_nr),
				  blobmsg_data_len(node->rrm_nr));
		blob_nest_end(&buf, r);
	}

	if (node->node_info)
		blob_put(&buf, APMSG_NODE_NODE_INFO,
			 blob_data(node->node_info),
			 blob_len(node->node_info));

	s = blob_nest_start(&buf, APMSG_NODE_STATIONS);

	if (sta) {
		usteer_send_sta_info(sta);
	} else {
		list_for_each_entry(sta, &node->sta_info, node_list)
			usteer_send_sta_info(sta);
	}

	blob_nest_end(&buf, s);

	blob_nest_end(&buf, c);
}

static void
usteer_check_timeout(void)
{
	struct usteer_remote_node *node, *tmp;
	int timeout = config.remote_node_timeout;

	list_for_each_entry_safe(node, tmp, &remote_nodes, list) {
		if (config.local_mode || node->check++ > timeout)
			remote_node_free(node);
	}
}

static void *
usteer_update_init(void)
{
	blob_buf_init(&buf, 0);
	blob_put_int32(&buf, APMSG_ID, local_id);
	blob_put_int32(&buf, APMSG_SEQ, ++msg_seq);
	if (host_info_blob)
		blob_put(&buf, APMSG_HOST_INFO,
			 blob_data(host_info_blob),
			 blob_len(host_info_blob));

	return blob_nest_start(&buf, APMSG_NODES);
}

static void
usteer_update_send(void *c)
{
	struct interface *iface;

	blob_nest_end(&buf, c);

	vlist_for_each_element(&interfaces, iface, node)
		interface_send_msg(iface, buf.head);
}

void
usteer_send_sta_update(struct sta_info *si)
{
	void *c = usteer_update_init();
	usteer_send_node(si->node, si);
	usteer_update_send(c);
}

static void
usteer_send_update_timer(struct uloop_timeout *t)
{
	struct usteer_node *node;
	void *c;

	usteer_update_time();
	uloop_timeout_set(t, config.remote_update_interval);

	if (!config.local_mode &&
	    (!avl_is_empty(&local_nodes) || host_info_blob)) {
		c = usteer_update_init();
		for_each_local_node(node)
			usteer_send_node(node, NULL);

		usteer_update_send(c);
	}
	usteer_check_timeout();
}

static int
usteer_init_local_id(void)
{
	FILE *f;

	f = fopen("/dev/urandom", "r");
	if (!f) {
		perror("fopen(/dev/urandom)");
		return -1;
	}

	if (fread(&local_id, sizeof(local_id), 1, f) < 1) {
		fclose(f);
		return -1;
	}

	fclose(f);
	return 0;
}

static int usteer_create_v4_socket() {
	int yes = 1;
	int fd;

	fd = usock(USOCK_UDP | USOCK_SERVER | USOCK_NONBLOCK |
		   USOCK_NUMERIC | USOCK_IPV4ONLY,
		   "0.0.0.0", APMGR_PORT_STR);
	if (fd < 0) {
		perror("usock");
		return - 1;
	}

	if (setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &yes, sizeof(yes)) < 0)
		perror("setsockopt(IP_PKTINFO)");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0)
		perror("setsockopt(SO_BROADCAST)");

	return fd;
}


static int usteer_create_v6_socket() {
	struct interface *iface;
	struct ipv6_mreq group;
	int yes = 1;
	int fd;

	fd = usock(USOCK_UDP | USOCK_SERVER | USOCK_NONBLOCK |
		   USOCK_NUMERIC | USOCK_IPV6ONLY,
		   "::", APMGR_PORT_STR);
	if (fd < 0) {
		perror("usock");
		return fd;
	}

	if (!inet_pton(AF_INET6, APMGR_V6_MCAST_GROUP, &group.ipv6mr_multiaddr.s6_addr))
		perror("inet_pton(AF_INET6)");

	/* Membership has to be added for every interface we listen on. */
	vlist_for_each_element(&interfaces, iface, node) {
		group.ipv6mr_interface = iface->ifindex;
		if(setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&group, sizeof group) < 0)
			perror("setsockopt(IPV6_ADD_MEMBERSHIP)");
	}

	if (setsockopt(fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &yes, sizeof(yes)) < 0)
		perror("setsockopt(IPV6_RECVPKTINFO)");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0)
		perror("setsockopt(SO_BROADCAST)");

	return fd;
}

static void usteer_reload_timer(struct uloop_timeout *t) {
	/* Remove uloop descriptor */
	if (remote_fd.fd && remote_fd.registered) {
		uloop_fd_delete(&remote_fd);
		close(remote_fd.fd);
	}

	if (config.ipv6) {
		remote_fd.fd = usteer_create_v6_socket();
		remote_fd.cb = interface_recv_v6;
	} else {
		remote_fd.fd = usteer_create_v4_socket();
		remote_fd.cb = interface_recv_v4;
	}

	if (remote_fd.fd < 0)
		return;

	uloop_fd_add(&remote_fd, ULOOP_READ);
}

int usteer_interface_init(void)
{
	if (usteer_init_local_id())
		return -1;

	remote_timer.cb = usteer_send_update_timer;
	remote_timer.cb(&remote_timer);

	reload_timer.cb = usteer_reload_timer;
	reload_timer.cb(&reload_timer);

	return 0;
}

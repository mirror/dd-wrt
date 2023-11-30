// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */


#include <netinet/ether.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <net/ethernet.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>
#include <sys/syscall.h>
#include <errno.h>
#include <net/if.h>
#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <time.h>

#include "main.h"
#include "functions.h"
#include "bat-hosts.h"
#include "sys.h"
#include "debug.h"
#include "netlink.h"

#define PATH_BUFF_LEN 400

static struct timespec start_time;
static char *host_name;
char *line_ptr = NULL;

void start_timer(void)
{
	clock_gettime(CLOCK_MONOTONIC, &start_time);
}

double end_timer(void)
{
	struct timespec end_time, diff;

	clock_gettime(CLOCK_MONOTONIC, &end_time);
	diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
	diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;

	if (diff.tv_nsec < 0) {
		diff.tv_sec--;
		diff.tv_nsec += 1000000000;
	}

	return (((double)diff.tv_sec * 1000) + ((double)diff.tv_nsec / 1000000));
}

char *ether_ntoa_long(const struct ether_addr *addr)
{
	static char asc[18];

	sprintf(asc, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr->ether_addr_octet[0], addr->ether_addr_octet[1],
		addr->ether_addr_octet[2], addr->ether_addr_octet[3],
		addr->ether_addr_octet[4], addr->ether_addr_octet[5]);

	return asc;
}

char *get_name_by_macaddr(struct ether_addr *mac_addr, int read_opt)
{
	struct bat_host *bat_host = NULL;

	if (read_opt & USE_BAT_HOSTS)
		bat_host = bat_hosts_find_by_mac((char *)mac_addr);

	if (!bat_host)
		host_name = ether_ntoa_long((struct ether_addr *)mac_addr);
	else
		host_name = bat_host->name;

	return host_name;
}

char *get_name_by_macstr(char *mac_str, int read_opt)
{
	struct ether_addr *mac_addr;

	mac_addr = ether_aton(mac_str);
	if (!mac_addr)
		return mac_str;

	return get_name_by_macaddr(mac_addr, read_opt);
}

int file_exists(const char *fpath)
{
	struct stat st;

	return stat(fpath, &st) == 0;
}

static void file_open_problem_dbg(const char *full_path)
{
	if (!file_exists(module_ver_path)) {
		fprintf(stderr, "Error - batman-adv module has not been loaded\n");
		return;
	}

	fprintf(stderr, "Error - can't open file '%s': %s\n", full_path, strerror(errno));
	fprintf(stderr, "The option you called seems not to be compiled into your batman-adv kernel module.\n");
	fprintf(stderr, "Consult the README if you wish to learn more about compiling options into batman-adv.\n");
}

static bool ether_addr_valid(const uint8_t *addr)
{
	/* no multicast address */
	if (addr[0] & 0x01)
		return false;

	/* no zero address */
	if ((addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]) == 0)
		return false;

	return true;
}

int read_file(const char *full_path, int read_opt)
{
	int res = EXIT_FAILURE;
	size_t len = 0;
	FILE *fp = NULL;

	fp = fopen(full_path, "r");
	if (!fp) {
		if (!(read_opt & SILENCE_ERRORS))
			file_open_problem_dbg(full_path);

		return res;
	}

	while (getline(&line_ptr, &len, fp) != -1) {
		/* the buffer will be handled elsewhere */
		if (read_opt & USE_READ_BUFF)
			break;

		printf("%s", line_ptr);
	}

	if (line_ptr)
		res = EXIT_SUCCESS;

	fclose(fp);
	return res;
}

struct ether_addr *translate_mac(struct state *state,
				 const struct ether_addr *mac)
{
	struct ether_addr in_mac;
	static struct ether_addr out_mac;
	struct ether_addr *mac_result;

	/* input mac has to be copied because it could be in the shared
	 * ether_aton buffer
	 */
	memcpy(&in_mac, mac, sizeof(in_mac));
	memcpy(&out_mac, mac, sizeof(out_mac));
	mac_result = &out_mac;

	if (!ether_addr_valid(in_mac.ether_addr_octet))
		return mac_result;

	translate_mac_netlink(state, &in_mac, mac_result);

	return mac_result;
}

int get_algoname(struct state *state, unsigned int mesh_ifindex,
		 char *algoname, size_t algoname_len)
{
	return get_algoname_netlink(state, mesh_ifindex, algoname,
				    algoname_len);
}

static int resolve_l3addr(int ai_family, const char *asc, void *l3addr)
{
	int ret;
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *inet4;
	struct sockaddr_in6 *inet6;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = ai_family;
	ret = getaddrinfo(asc, NULL, &hints, &res);
	if (ret)
		return -EADDRNOTAVAIL;

	if (res) {
		switch (ai_family) {
		case AF_INET:
			inet4 = (struct sockaddr_in *)res->ai_addr;
			memcpy(l3addr, &inet4->sin_addr.s_addr,
			       sizeof(inet4->sin_addr.s_addr));
			break;
		case AF_INET6:
			inet6 = (struct sockaddr_in6 *)res->ai_addr;
			memcpy(l3addr, &inet6->sin6_addr.s6_addr,
			       sizeof(inet6->sin6_addr.s6_addr));
			break;
		default:
			ret = -EINVAL;
		}
	}

	freeaddrinfo(res);
	return ret;
}

static void request_mac_resolve(int ai_family, const void *l3addr)
{
	const struct sockaddr *sockaddr;
	struct sockaddr_in inet4;
	struct sockaddr_in6 inet6;
	size_t sockaddr_len;
	int sock;
	char t = 0;

	sock = socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
		return;

	switch (ai_family) {
	case AF_INET:
		memset(&inet4, 0, sizeof(inet4));
		inet4.sin_family = ai_family;
		inet4.sin_port = htons(9);
		memcpy(&inet4.sin_addr.s_addr, l3addr,
		       sizeof(inet4.sin_addr.s_addr));
		sockaddr = (const struct sockaddr *)&inet4;
		sockaddr_len = sizeof(inet4);
		break;
	case AF_INET6:
		memset(&inet6, 0, sizeof(inet6));
		inet6.sin6_family = ai_family;
		inet6.sin6_port = htons(9);
		memcpy(&inet6.sin6_addr.s6_addr, l3addr,
		       sizeof(inet6.sin6_addr.s6_addr));
		sockaddr = (const struct sockaddr *)&inet6;
		sockaddr_len = sizeof(inet6);
		break;
	default:
		close(sock);
		return;
	}

	sendto(sock, &t, sizeof(t), 0, sockaddr, sockaddr_len);
	close(sock);
}

struct resolve_mac_nl_arg {
	int ai_family;
	const void *l3addr;
	struct ether_addr *mac_result;
	int found;
};

static struct nla_policy neigh_policy[NDA_MAX+1] = {
	[NDA_CACHEINFO] = { .minlen = sizeof(struct nda_cacheinfo) },
	[NDA_PROBES]    = { .type = NLA_U32 },
};

static int resolve_mac_from_parse(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NDA_MAX + 1];
	struct ndmsg *nm;
	int ret;
	int l3_len;
	struct resolve_mac_nl_arg *nl_arg = arg;
	uint8_t *mac;
	uint8_t *l3addr;

	nm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*nm), tb, NDA_MAX,
			  neigh_policy);
	if (ret < 0)
		goto err;

	if (nl_arg->ai_family != nm->ndm_family)
		goto err;

	switch (nl_arg->ai_family) {
	case AF_INET:
		l3_len = 4;
		break;
	case AF_INET6:
		l3_len = 16;
		break;
	default:
		l3_len = 0;
	}

	if (l3_len == 0)
		goto err;

	if (!tb[NDA_LLADDR] || !tb[NDA_DST])
		goto err;

	if (nla_len(tb[NDA_LLADDR]) != ETH_ALEN)
		goto err;

	if (nla_len(tb[NDA_DST]) != l3_len)
		goto err;

	mac = nla_data(tb[NDA_LLADDR]);
	l3addr = nla_data(tb[NDA_DST]);

	if (!ether_addr_valid(mac))
		goto err;

	if (memcmp(nl_arg->l3addr, l3addr, l3_len) == 0) {
		memcpy(nl_arg->mac_result, mac, ETH_ALEN);
		nl_arg->found = 1;
	}

err:
	return NL_OK;
}

static struct ether_addr *resolve_mac_from_cache(int ai_family,
						 const void *l3addr)
{
	struct nl_sock *sock;
	struct ether_addr *mac_result = NULL;
	static struct ether_addr mac_tmp;
	int ret;
	struct rtgenmsg gmsg = {
		.rtgen_family = ai_family,
	};
	struct nl_cb *cb = NULL;
	struct resolve_mac_nl_arg arg = {
		.ai_family = ai_family,
		.l3addr = l3addr,
		.mac_result = &mac_tmp,
		.found = 0,
	};

	sock = nl_socket_alloc();
	if (!sock)
		goto err;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0)
		goto err;

	ret = nl_send_simple(sock, RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP,
			     &gmsg, sizeof(gmsg));
	if (ret < 0)
		goto err;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, resolve_mac_from_parse, &arg);
	ret = nl_recvmsgs(sock, cb);
	if (ret < 0)
		goto err;

	if (arg.found)
		mac_result = &mac_tmp;

err:
	if (cb)
		nl_cb_put(cb);
	if (sock)
		nl_socket_free(sock);

	return mac_result;
}

static struct ether_addr *resolve_mac_from_addr(int ai_family, const char *asc)
{
	uint8_t ipv4_addr[4];
	uint8_t ipv6_addr[16];
	void *l3addr;
	int ret;
	int retries = 5;
	struct ether_addr *mac_result = NULL;

	switch (ai_family) {
	case AF_INET:
		l3addr = ipv4_addr;
		break;
	case AF_INET6:
		l3addr = ipv6_addr;
		break;
	default:
		return NULL;
	}

	ret = resolve_l3addr(ai_family, asc, l3addr);
	if (ret < 0)
		return NULL;

	while (retries-- && !mac_result) {
		mac_result = resolve_mac_from_cache(ai_family, l3addr);
		if (!mac_result) {
			request_mac_resolve(ai_family, l3addr);
			usleep(200000);
		}
	}

	return mac_result;
}

struct ether_addr *resolve_mac(const char *asc)
{
	struct ether_addr *mac_result = NULL;
	static const int ai_families[] = {AF_INET, AF_INET6};
	size_t i;

	mac_result = ether_aton(asc);
	if (mac_result)
		goto out;

	for (i = 0; i < sizeof(ai_families) / sizeof(*ai_families); i++) {
		mac_result = resolve_mac_from_addr(ai_families[i], asc);
		if (mac_result)
			goto out;
	}

out:
	return mac_result;
}

int query_rtnl_link(int ifindex, nl_recvmsg_msg_cb_t func, void *arg)
{
	struct ifinfomsg rt_hdr = {
		.ifi_family = IFLA_UNSPEC,
	};
	struct nl_sock *sock;
	struct nl_msg *msg;
	struct nl_cb *cb;
	int err = 0;
	int ret;

	sock = nl_socket_alloc();
	if (!sock)
		return -ENOMEM;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, func, arg);

	msg = nlmsg_alloc_simple(RTM_GETLINK, NLM_F_REQUEST | NLM_F_DUMP);
	if (!msg) {
		err = -ENOMEM;
		goto err_free_cb;
	}

	ret = nlmsg_append(msg, &rt_hdr, sizeof(rt_hdr), NLMSG_ALIGNTO);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_u32(msg, IFLA_MASTER, ifindex);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto err_free_msg;

	nl_recvmsgs(sock, cb);

err_free_msg:
	nlmsg_free(msg);
err_free_cb:
	nl_cb_put(cb);
err_free_sock:
	nl_socket_free(sock);

	return err;
}

static int ack_errno_handler(struct sockaddr_nl *nla __maybe_unused,
			     struct nlmsgerr *nlerr,
			     void *arg)
{
	int *err = arg;

	*err = nlerr->error;

	return NL_STOP;
}

static int ack_wait_handler(struct nl_msg *msg __maybe_unused,
			    void *arg __maybe_unused)
{
	return NL_STOP;
}

int netlink_simple_request(struct nl_msg *msg)
{
	struct nl_sock *sock;
	struct nl_cb *cb;
	int err = 0;
	int ret;

	sock = nl_socket_alloc();
	if (!sock)
		return -ENOMEM;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	nl_cb_err(cb, NL_CB_CUSTOM, ack_errno_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_wait_handler, NULL);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto err_free_cb;

	// ack_errno_handler sets err on errors
	err = 0;
	nl_recvmsgs(sock, cb);

err_free_cb:
	nl_cb_put(cb);
err_free_sock:
	nl_socket_free(sock);

	return err;
}

struct rtnl_link_iface_data {
	uint8_t kind_found:1;
	uint8_t master_found:1;
	uint8_t link_found:1;
	uint8_t vid_found:1;
	char kind[IF_NAMESIZE];
	unsigned int master;
	unsigned int link;
	uint16_t vid;
};

static int query_rtnl_link_single_parse(struct nl_msg *msg, void *arg)
{
	static struct nla_policy link_policy[IFLA_MAX + 1] = {
		[IFLA_LINKINFO] = { .type = NLA_NESTED },
		[IFLA_MASTER] = { .type = NLA_U32 },
		[IFLA_LINK] = { .type = NLA_U32 },
	};
	static struct nla_policy link_info_policy[IFLA_INFO_MAX + 1] = {
		[IFLA_INFO_KIND] = { .type = NLA_STRING },
		[IFLA_INFO_DATA] = { .type = NLA_NESTED },
	};
	static struct nla_policy vlan_policy[IFLA_VLAN_MAX + 1] = {
		[IFLA_VLAN_ID] = { .type = NLA_U16 },
	};

	struct rtnl_link_iface_data *link_data = arg;
	struct nlattr *li[IFLA_INFO_MAX + 1];
	struct nlattr *vi[IFLA_VLAN_MAX + 1];
	struct nlmsghdr *n = nlmsg_hdr(msg);
	struct nlattr *tb[IFLA_MAX + 1];
	char *type;
	int ret;

	if (!nlmsg_valid_hdr(n, sizeof(struct ifinfomsg)))
		return NL_OK;

	ret = nlmsg_parse(n, sizeof(struct ifinfomsg), tb, IFLA_MAX,
			  link_policy);
	if (ret < 0)
		return NL_OK;

	if (tb[IFLA_MASTER]) {
		link_data->master = nla_get_u32(tb[IFLA_MASTER]);
		link_data->master_found = true;
	}

	if (tb[IFLA_LINK]) {
		link_data->link = nla_get_u32(tb[IFLA_LINK]);
		link_data->link_found = true;
	}

	/* parse subattributes linkinfo */
	if (!tb[IFLA_LINKINFO])
		return NL_OK;

	ret = nla_parse_nested(li, IFLA_INFO_MAX, tb[IFLA_LINKINFO],
			       link_info_policy);
	if (ret < 0)
		return NL_OK;

	if (li[IFLA_INFO_KIND]) {
		type = nla_get_string(li[IFLA_INFO_KIND]);
		strncpy(link_data->kind, type, sizeof(link_data->kind));
		link_data->kind[sizeof(link_data->kind) - 1] = '\0';

		link_data->kind_found = true;
	}

	if (!li[IFLA_INFO_DATA])
		return NL_OK;

	ret = nla_parse_nested(vi, IFLA_VLAN_MAX, li[IFLA_INFO_DATA],
			       vlan_policy);
	if (ret < 0)
		return NL_OK;

	if (vi[IFLA_VLAN_ID]) {
		link_data->vid = nla_get_u16(vi[IFLA_VLAN_ID]);
		link_data->vid_found = true;
	}

	return NL_STOP;
}

static int query_rtnl_link_single(int mesh_ifindex,
				  struct rtnl_link_iface_data *link_data)
{
	struct ifinfomsg ifinfo = {
		.ifi_family = AF_UNSPEC,
		.ifi_index = mesh_ifindex,
	};
	struct nl_cb *cb = NULL;
	struct nl_sock *sock;
	int ret;

	link_data->kind_found = false;
	link_data->master_found = false;
	link_data->link_found = false;
	link_data->vid_found = false;

	sock = nl_socket_alloc();
	if (!sock)
		return -1;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0)
		goto free_sock;

	ret = nl_send_simple(sock, RTM_GETLINK, NLM_F_REQUEST,
			     &ifinfo, sizeof(ifinfo));
	if (ret < 0)
		goto free_sock;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto free_sock;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, query_rtnl_link_single_parse,
		  link_data);
	nl_recvmsgs(sock, cb);

	nl_cb_put(cb);
free_sock:
	nl_socket_free(sock);

	return 0;
}

int translate_vlan_iface(struct state *state, const char *vlandev)
{
	struct rtnl_link_iface_data link_data;
	unsigned int arg_ifindex;

	arg_ifindex = if_nametoindex(vlandev);
	if (arg_ifindex == 0)
		return -ENODEV;

	query_rtnl_link_single(arg_ifindex, &link_data);
	if (!link_data.vid_found)
		return -ENODEV;

	if (!link_data.link_found)
		return -EINVAL;

	if (!link_data.kind_found)
		return -EINVAL;

	if (strcmp(link_data.kind, "vlan") != 0)
		return -EINVAL;

	if (!if_indextoname(link_data.link, state->mesh_iface))
		return -ENODEV;

	state->vid = link_data.vid;
	state->selector = SP_VLAN;

	return 0;
}

int translate_mesh_iface_vlan(struct state *state, const char *vlandev)
{
	int ret;

	ret = translate_vlan_iface(state, vlandev);
	if (ret < 0)
		goto fallback_meshif;

	return 0;

fallback_meshif:
	/* if there is no vid then the argument must be the
	 * mesh interface
	 */
	snprintf(state->mesh_iface, sizeof(state->mesh_iface), "%s", vlandev);
	state->selector = SP_NONE_OR_MESHIF;

	return 0;
}

int translate_vid(struct state *state, const char *vidstr)
{
	unsigned long vid;
	char *endptr;

	if (vidstr[0] == '\0') {
		fprintf(stderr, "Error - unparsable vid\n");
		return -EINVAL;
	}

	vid = strtoul(vidstr, &endptr, 0);
	if (!endptr || *endptr != '\0') {
		fprintf(stderr, "Error - unparsable vid\n");
		return -EINVAL;
	}

	if (vid > 4095) {
		fprintf(stderr, "Error - too large vid (max 4095)\n");
		return -ERANGE;
	}

	/* get mesh interface and overwrite vid afterwards */
	state->vid = vid;
	state->selector = SP_VLAN;

	return 0;
}

int translate_hard_iface(struct state *state, const char *hardif)
{
	struct rtnl_link_iface_data link_data;
	unsigned int arg_ifindex;

	arg_ifindex = if_nametoindex(hardif);
	if (arg_ifindex == 0)
		return -ENODEV;

	query_rtnl_link_single(arg_ifindex, &link_data);
	if (!link_data.master_found)
		return -ENOLINK;

	if (!if_indextoname(link_data.master, state->mesh_iface))
		return -ENOLINK;

	state->hif = arg_ifindex;
	state->selector = SP_HARDIF;

	return 0;
}

static int check_mesh_iface_netlink(unsigned int ifindex)
{
	struct rtnl_link_iface_data link_data;

	query_rtnl_link_single(ifindex, &link_data);
	if (!link_data.kind_found)
		return -1;

	if (strcmp(link_data.kind, "batadv") != 0)
		return -1;

	return 0;
}

int guess_netdev_type(const char *netdev, enum selector_prefix *type)
{
	struct rtnl_link_iface_data link_data;
	unsigned int netdev_ifindex;

	netdev_ifindex = if_nametoindex(netdev);
	if (netdev_ifindex == 0)
		return -ENODEV;

	query_rtnl_link_single(netdev_ifindex, &link_data);

	if (link_data.kind_found && strcmp(link_data.kind, "batadv") == 0) {
		*type = SP_MESHIF;
		return 0;
	}

	if (link_data.master_found &&
	    check_mesh_iface_netlink(link_data.master) >= 0) {
		*type = SP_HARDIF;
		return 0;
	}

	if (link_data.kind_found && strcmp(link_data.kind, "vlan") == 0) {
		*type = SP_VLAN;
		return 0;
	}

	return -EINVAL;
}

int check_mesh_iface(struct state *state)
{
	state->mesh_ifindex = if_nametoindex(state->mesh_iface);
	if (state->mesh_ifindex == 0)
		return -1;

	return check_mesh_iface_netlink(state->mesh_ifindex);
}

int check_mesh_iface_ownership(struct state *state, char *hard_iface)
{
	struct rtnl_link_iface_data link_data;
	unsigned int hardif_index;

	hardif_index = if_nametoindex(hard_iface);
	if (hardif_index == 0)
		return EXIT_FAILURE;

	query_rtnl_link_single(hardif_index, &link_data);
	if (!link_data.master_found)
		return EXIT_FAILURE;

	if (state->mesh_ifindex != link_data.master)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int get_random_bytes_syscall(void *buf __maybe_unused,
				    size_t buflen __maybe_unused)
{
#ifdef SYS_getrandom
	return syscall(SYS_getrandom, buf, buflen, 0);
#else
	return -EOPNOTSUPP;
#endif
}

static int get_random_bytes_urandom(void *buf, size_t buflen)
{
	int fd;
	ssize_t r;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -EOPNOTSUPP;

	r = read(fd, buf, buflen);
	close(fd);
	if (r < 0)
		return -EOPNOTSUPP;

	if ((size_t)r != buflen)
		return -EOPNOTSUPP;

	return 0;
}

static int get_random_bytes_fallback(void *buf, size_t buflen)
{
	struct timespec now;
	static int initialized = 0;
	size_t i;
	uint8_t *bufc = buf;

	/* this is not a good source for randomness */
	if (!initialized) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		srand(now.tv_sec ^ now.tv_nsec);
		initialized = 1;
	}

	for (i = 0; i < buflen; i++)
		bufc[i] = rand() & 0xff;

	return 0;
}

void get_random_bytes(void *buf, size_t buflen)
{
	int ret;

	ret = get_random_bytes_syscall(buf, buflen);
	if (ret != -EOPNOTSUPP)
		return;

	ret = get_random_bytes_urandom(buf, buflen);
	if (ret != -EOPNOTSUPP)
		return;

	get_random_bytes_fallback(buf, buflen);
}

void check_root_or_die(const char *cmd)
{
	if (geteuid() != 0) {
		fprintf(stderr, "Error - you must be root to run '%s' !\n", cmd);
		exit(EXIT_FAILURE);
	}
}

int parse_bool(const char *val, bool *res)
{
	if (strcasecmp(val, "0") == 0 ||
	    strcasecmp(val, "disable") == 0 ||
	    strcasecmp(val, "disabled") == 0) {
		*res = false;
		return 0;
	} else if (strcasecmp(val, "1") == 0 ||
		   strcasecmp(val, "enable") == 0 ||
		   strcasecmp(val, "enabled") == 0) {
		*res = true;
		return 0;
	}

	return -EINVAL;
}

bool parse_throughput(char *buff, const char *description, uint32_t *throughput)
{
	enum batadv_bandwidth_units bw_unit_type = BATADV_BW_UNIT_KBIT;
	uint64_t lthroughput;
	char *tmp_ptr;
	char *endptr;

	if (strlen(buff) > 4) {
		tmp_ptr = buff + strlen(buff) - 4;

		if (strncasecmp(tmp_ptr, "mbit", 4) == 0)
			bw_unit_type = BATADV_BW_UNIT_MBIT;

		if (strncasecmp(tmp_ptr, "kbit", 4) == 0 ||
		    bw_unit_type == BATADV_BW_UNIT_MBIT)
			*tmp_ptr = '\0';
	}

	lthroughput = strtoull(buff, &endptr, 10);
	if (!endptr || *endptr != '\0') {
		fprintf(stderr, "Invalid throughput speed for %s: %s\n",
			description, buff);
		return false;
	}

	switch (bw_unit_type) {
	case BATADV_BW_UNIT_MBIT:
		/* prevent overflow */
		if (UINT64_MAX / 10 < lthroughput) {
			fprintf(stderr,
				"Throughput speed for %s too large: %s\n",
				description, buff);
			return false;
		}

		lthroughput *= 10;
		break;
	case BATADV_BW_UNIT_KBIT:
	default:
		lthroughput = lthroughput / 100;
		break;
	}

	if (lthroughput > UINT32_MAX) {
		fprintf(stderr, "Throughput speed for %s too large: %s\n",
			description, buff);
		return false;
	}

	*throughput = lthroughput;

	return true;
}

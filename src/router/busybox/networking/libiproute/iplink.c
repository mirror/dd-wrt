/* vi: set sw=4 ts=4: */
/*
 * Authors: Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 * 			Patrick McHardy <kaber@trash.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#include <net/if.h>
/*#include <net/if_packet.h> - not needed? */
#include <netpacket/packet.h>
#include <netinet/if_ether.h>

#include <linux/if_vlan.h>
#include "ip_common.h"  /* #include "libbb.h" is inside */
#include "rt_names.h"
#include "utils.h"

#undef  ETH_P_8021AD
#define ETH_P_8021AD            0x88A8
#undef  VLAN_FLAG_REORDER_HDR
#define VLAN_FLAG_REORDER_HDR   0x1
#undef  VLAN_FLAG_GVRP
#define VLAN_FLAG_GVRP          0x2
#undef  VLAN_FLAG_LOOSE_BINDING
#define VLAN_FLAG_LOOSE_BINDING 0x4
#undef  VLAN_FLAG_MVRP
#define VLAN_FLAG_MVRP          0x8
#undef  IFLA_VLAN_PROTOCOL
#define IFLA_VLAN_PROTOCOL      5

#ifndef IFLA_LINKINFO
# define IFLA_LINKINFO 18
# define IFLA_INFO_KIND 1
# define IFLA_INFO_DATA 2
#endif

#ifndef IFLA_VLAN_MAX
# define IFLA_VLAN_ID 1
# define IFLA_VLAN_FLAGS 2
struct ifla_vlan_flags {
	uint32_t	flags;
	uint32_t	mask;
};
#endif

/* taken from linux/sockios.h */
#define SIOCSIFNAME  0x8923  /* set interface name */

#if 0
# define dbg(...) bb_error_msg(__VA_ARGS__)
#else
# define dbg(...) ((void)0)
#endif


#define str_on_off "on\0""off\0"

/* Exits on error */
static int get_ctl_fd(void)
{
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	return xsocket(PF_INET6, SOCK_DGRAM, 0);
}

/* Exits on error */
static void do_chflags(char *dev, uint32_t flags, uint32_t mask)
{
	struct ifreq ifr;
	int fd;

	strncpy_IFNAMSIZ(ifr.ifr_name, dev);
	fd = get_ctl_fd();
	xioctl(fd, SIOCGIFFLAGS, &ifr);
	if ((ifr.ifr_flags ^ flags) & mask) {
		ifr.ifr_flags &= ~mask;
		ifr.ifr_flags |= mask & flags;
		xioctl(fd, SIOCSIFFLAGS, &ifr);
	}
	close(fd);
}

/* Exits on error */
static void do_changename(char *dev, char *newdev)
{
	struct ifreq ifr;
	int fd;

	strncpy_IFNAMSIZ(ifr.ifr_name, dev);
	strncpy_IFNAMSIZ(ifr.ifr_newname, newdev);
	fd = get_ctl_fd();
	xioctl(fd, SIOCSIFNAME, &ifr);
	close(fd);
}

/* Exits on error */
static void set_qlen(char *dev, int qlen)
{
	struct ifreq ifr;
	int s;

	s = get_ctl_fd();
	memset(&ifr, 0, sizeof(ifr));
	strncpy_IFNAMSIZ(ifr.ifr_name, dev);
	ifr.ifr_qlen = qlen;
	xioctl(s, SIOCSIFTXQLEN, &ifr);
	close(s);
}

/* Exits on error */
static void set_mtu(char *dev, int mtu)
{
	struct ifreq ifr;
	int s;

	s = get_ctl_fd();
	memset(&ifr, 0, sizeof(ifr));
	strncpy_IFNAMSIZ(ifr.ifr_name, dev);
	ifr.ifr_mtu = mtu;
	xioctl(s, SIOCSIFMTU, &ifr);
	close(s);
}

/* Exits on error */
static void set_master(char *dev, int master)
{
	struct rtnl_handle rth;
	struct {
		struct nlmsghdr  n;
		struct ifinfomsg i;
		char             buf[1024];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_NEWLINK;
	req.i.ifi_family = preferred_family;

	xrtnl_open(&rth);
	req.i.ifi_index = xll_name_to_index(dev);
	//printf("master %i for %i\n", master, req.i.ifi_index);
	addattr_l(&req.n, sizeof(req), IFLA_MASTER, &master, 4);
	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		xfunc_die();
}

/* Exits on error */
static void set_netns(char *dev, int netns)
{
	struct rtnl_handle rth;
	struct {
		struct nlmsghdr  n;
		struct ifinfomsg i;
		char             buf[1024];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_NEWLINK;
	req.i.ifi_family = preferred_family;

	xrtnl_open(&rth);
	req.i.ifi_index = xll_name_to_index(dev);
	//printf("netns %i for %i\n", netns, req.i.ifi_index);
	addattr_l(&req.n, sizeof(req), IFLA_NET_NS_PID, &netns, 4);
	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		xfunc_die();
}

/* Exits on error */
static int get_address(char *dev, int *htype)
{
	struct ifreq ifr;
	struct sockaddr_ll me;
	int s;

	s = xsocket(PF_PACKET, SOCK_DGRAM, 0);

	/*memset(&ifr, 0, sizeof(ifr)); - SIOCGIFINDEX does not need to clear all */
	strncpy_IFNAMSIZ(ifr.ifr_name, dev);
	xioctl(s, SIOCGIFINDEX, &ifr);

	memset(&me, 0, sizeof(me));
	me.sll_family = AF_PACKET;
	me.sll_ifindex = ifr.ifr_ifindex;
	me.sll_protocol = htons(ETH_P_LOOP);
	xbind(s, (struct sockaddr*)&me, sizeof(me));
	bb_getsockname(s, (struct sockaddr*)&me, sizeof(me));
	//never happens:
	//if (getsockname(s, (struct sockaddr*)&me, &alen) == -1)
	//	bb_perror_msg_and_die("getsockname");
	close(s);
	*htype = me.sll_hatype;
	return me.sll_halen;
}

/* Exits on error */
static void parse_address(char *dev, int hatype, int halen, char *lla, struct ifreq *ifr)
{
	int alen;

	memset(ifr, 0, sizeof(*ifr));
	strncpy_IFNAMSIZ(ifr->ifr_name, dev);
	ifr->ifr_hwaddr.sa_family = hatype;

	alen = hatype == 1/*ARPHRD_ETHER*/ ? 14/*ETH_HLEN*/ : 19/*INFINIBAND_HLEN*/;
	alen = ll_addr_a2n((unsigned char *)(ifr->ifr_hwaddr.sa_data), alen, lla);
	if (alen < 0)
		exit(EXIT_FAILURE);
	if (alen != halen) {
		bb_error_msg_and_die("wrong address (%s) length: expected %d bytes", lla, halen);
	}
}

/* Exits on error */
static void set_address(struct ifreq *ifr, int brd)
{
	int s;

	s = get_ctl_fd();
	if (brd)
		xioctl(s, SIOCSIFHWBROADCAST, ifr);
	else
		xioctl(s, SIOCSIFHWADDR, ifr);
	close(s);
}


static void die_must_be_on_off(const char *msg) NORETURN;
static void die_must_be_on_off(const char *msg)
{
	bb_error_msg_and_die("argument of \"%s\" must be \"on\" or \"off\"", msg);
}

/* Return value becomes exitcode. It's okay to not return at all */
static int do_set(char **argv)
{
	char *dev = NULL;
	uint32_t mask = 0;
	uint32_t flags = 0;
	int qlen = -1;
	int mtu = -1;
	int master = -1;
	int netns = -1;
	char *newaddr = NULL;
	char *newbrd = NULL;
	struct ifreq ifr0, ifr1;
	char *newname = NULL;
	int htype, halen;
	/* If you add stuff here, update iplink_full_usage */
	static const char keywords[] ALIGN1 =
		"up\0""down\0""name\0""mtu\0""qlen\0""multicast\0"
		"arp\0""promisc\0""address\0""netns\0"
		"master\0""nomaster\0"
		"dev\0" /* must be last */;
	enum { ARG_up = 0, ARG_down, ARG_name, ARG_mtu, ARG_qlen, ARG_multicast,
		ARG_arp, ARG_promisc, ARG_addr, ARG_netns,
		ARG_master, ARG_nomaster,
		ARG_dev };
	enum { PARM_on = 0, PARM_off };
	smalluint key;

	while (*argv) {
		/* substring search ensures that e.g. "addr" and "address"
		 * are both accepted */
		key = index_in_substrings(keywords, *argv);
		if (key == ARG_up) {
			mask |= IFF_UP;
			flags |= IFF_UP;
		} else if (key == ARG_down) {
			mask |= IFF_UP;
			flags &= ~IFF_UP;
		} else if (key == ARG_name) {
			NEXT_ARG();
			newname = *argv;
		} else if (key == ARG_mtu) {
			NEXT_ARG();
			if (mtu != -1)
				duparg("mtu", *argv);
			mtu = get_unsigned(*argv, "mtu");
		} else if (key == ARG_qlen) {
//TODO: txqueuelen, txqlen are synonyms to qlen
			NEXT_ARG();
			if (qlen != -1)
				duparg("qlen", *argv);
			qlen = get_unsigned(*argv, "qlen");
		} else if (key == ARG_addr) {
			NEXT_ARG();
			newaddr = *argv;
		} else if (key == ARG_master) {
			NEXT_ARG();
			master = xll_name_to_index(*argv);
		} else if (key == ARG_nomaster) {
			master = 0;
		} else if (key == ARG_netns) {
			NEXT_ARG();
			netns = get_unsigned(*argv, "netns");
		} else if (key >= ARG_dev) {
			/* ^^^^^^ ">=" here results in "dev IFACE" treated as default */
			if (key == ARG_dev) {
				NEXT_ARG();
			}
			if (dev)
				duparg2("dev", *argv);
			dev = *argv;
		} else {
			/* "on|off" options */
			int param;
			NEXT_ARG();
			param = index_in_strings(str_on_off, *argv);
			if (key == ARG_multicast) {
				if (param < 0)
					die_must_be_on_off("multicast");
				mask |= IFF_MULTICAST;
				if (param == PARM_on)
					flags |= IFF_MULTICAST;
				else
					flags &= ~IFF_MULTICAST;
			} else if (key == ARG_arp) {
				if (param < 0)
					die_must_be_on_off("arp");
				mask |= IFF_NOARP;
				if (param == PARM_on)
					flags &= ~IFF_NOARP;
				else
					flags |= IFF_NOARP;
			} else if (key == ARG_promisc) {
				if (param < 0)
					die_must_be_on_off("promisc");
				mask |= IFF_PROMISC;
				if (param == PARM_on)
					flags |= IFF_PROMISC;
				else
					flags &= ~IFF_PROMISC;
			}
		}

/* Other keywords recognized by iproute2-3.12.0: */
#if 0
		} else if (matches(*argv, "broadcast") == 0 ||
				strcmp(*argv, "brd") == 0) {
			NEXT_ARG();
			len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			if (len < 0)
				return -1;
			addattr_l(&req->n, sizeof(*req), IFLA_BROADCAST, abuf, len);
                } else if (strcmp(*argv, "netns") == 0) {
                        NEXT_ARG();
                        if (netns != -1)
                                duparg("netns", *argv);
			if ((netns = get_netns_fd(*argv)) >= 0)
				addattr_l(&req->n, sizeof(*req), IFLA_NET_NS_FD, &netns, 4);
			else if (get_integer(&netns, *argv, 0) == 0)
				addattr_l(&req->n, sizeof(*req), IFLA_NET_NS_PID, &netns, 4);
			else
                                invarg_1_to_2(*argv, "netns");
		} else if (strcmp(*argv, "allmulticast") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_ALLMULTI;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_ALLMULTI;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_ALLMULTI;
			} else
				return on_off("allmulticast", *argv);
		} else if (strcmp(*argv, "trailers") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_NOTRAILERS;
			if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags |= IFF_NOTRAILERS;
			} else if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags &= ~IFF_NOTRAILERS;
			} else
				return on_off("trailers", *argv);
		} else if (strcmp(*argv, "vf") == 0) {
			struct rtattr *vflist;
			NEXT_ARG();
			if (get_integer(&vf,  *argv, 0)) {
				invarg_1_to_2(*argv, "vf");
			}
			vflist = addattr_nest(&req->n, sizeof(*req),
					      IFLA_VFINFO_LIST);
			len = iplink_parse_vf(vf, &argc, &argv, req);
			if (len < 0)
				return -1;
			addattr_nest_end(&req->n, vflist);
		} else if (matches(*argv, "master") == 0) {
			int ifindex;
			NEXT_ARG();
			ifindex = xll_name_to_index(*argv);
			if (!ifindex)
				invarg_1_to_2(*argv, "master");
			addattr_l(&req->n, sizeof(*req), IFLA_MASTER,
				  &ifindex, 4);
		} else if (matches(*argv, "nomaster") == 0) {
			int ifindex = 0;
			addattr_l(&req->n, sizeof(*req), IFLA_MASTER,
				  &ifindex, 4);
		} else if (matches(*argv, "dynamic") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_DYNAMIC;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_DYNAMIC;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_DYNAMIC;
			} else
				return on_off("dynamic", *argv);
		} else if (matches(*argv, "alias") == 0) {
			NEXT_ARG();
			addattr_l(&req->n, sizeof(*req), IFLA_IFALIAS,
				  *argv, strlen(*argv));
			argc--; argv++;
			break;
		} else if (strcmp(*argv, "group") == 0) {
			NEXT_ARG();
			if (*group != -1)
				duparg("group", *argv);
			if (rtnl_group_a2n(group, *argv))
				invarg_1_to_2(*argv, "group");
		} else if (strcmp(*argv, "mode") == 0) {
			int mode;
			NEXT_ARG();
			mode = get_link_mode(*argv);
			if (mode < 0)
				invarg_1_to_2(*argv, "mode");
			addattr8(&req->n, sizeof(*req), IFLA_LINKMODE, mode);
		} else if (strcmp(*argv, "state") == 0) {
			int state;
			NEXT_ARG();
			state = get_operstate(*argv);
			if (state < 0)
				invarg_1_to_2(*argv, "state");
			addattr8(&req->n, sizeof(*req), IFLA_OPERSTATE, state);
		} else if (matches(*argv, "numtxqueues") == 0) {
			NEXT_ARG();
			if (numtxqueues != -1)
				duparg("numtxqueues", *argv);
			if (get_integer(&numtxqueues, *argv, 0))
				invarg_1_to_2(*argv, "numtxqueues");
			addattr_l(&req->n, sizeof(*req), IFLA_NUM_TX_QUEUES,
				  &numtxqueues, 4);
		} else if (matches(*argv, "numrxqueues") == 0) {
			NEXT_ARG();
			if (numrxqueues != -1)
				duparg("numrxqueues", *argv);
			if (get_integer(&numrxqueues, *argv, 0))
				invarg_1_to_2(*argv, "numrxqueues");
			addattr_l(&req->n, sizeof(*req), IFLA_NUM_RX_QUEUES,
				  &numrxqueues, 4);
		}
#endif

		argv++;
	}

	if (!dev) {
		bb_error_msg_and_die(bb_msg_requires_arg, "\"dev\"");
	}

	if (newaddr || newbrd) {
		halen = get_address(dev, &htype);
		if (newaddr) {
			parse_address(dev, htype, halen, newaddr, &ifr0);
			set_address(&ifr0, 0);
		}
		if (newbrd) {
			parse_address(dev, htype, halen, newbrd, &ifr1);
			set_address(&ifr1, 1);
		}
	}

	if (newname && strcmp(dev, newname)) {
		do_changename(dev, newname);
		dev = newname;
	}
	if (qlen != -1) {
		set_qlen(dev, qlen);
	}
	if (mtu != -1) {
		set_mtu(dev, mtu);
	}
	if (master != -1) {
		set_master(dev, master);
	}
	if (netns != -1) {
		set_netns(dev, netns);
	}
	if (mask)
		do_chflags(dev, flags, mask);
	return 0;
}

static int ipaddr_list_link(char **argv)
{
	preferred_family = AF_PACKET;
	return ipaddr_list_or_flush(argv, 0);
}

#ifndef IFLA_VLAN_QOS_MAX
struct ifla_vlan_qos_mapping {
	__u32 from;
	__u32 to;
};

enum {
	IFLA_VLAN_QOS_UNSPEC,
	IFLA_VLAN_QOS_MAPPING,
	__IFLA_VLAN_QOS_MAX
};
#endif
static __u32 parse_mapping_num(char *key)
{
	return get_u32(key, "vlan number");
}

static void parse_mapping_gen(char ***argvp,
		      __u32 (*key_cb)(char *key),
		      void (*mapping_cb)(__u32 key, char *value, void *data),
		      void *mapping_cb_data)
{
	char **argv = *argvp;

	while (*argv) {
		char *colon = strchr(*argv, ':');
		__u32 key;

		if (!colon)
			break;
		*colon = '\0';

		key = key_cb(*argv);	
		mapping_cb(key, colon + 1, mapping_cb_data);

		argv++;
	}

	*argvp = argv;
}

static void parse_mapping(char ***argvp,
		  void  (*mapping_cb)(__u32 key, char *value, void *data),
		  void *mapping_cb_data)
{
	parse_mapping_gen(argvp, parse_mapping_num,
					 mapping_cb, mapping_cb_data);
}

static void parse_qos_mapping(__u32 key, char *value, void *data)
{
	struct nlmsghdr *n = data;
	struct ifla_vlan_qos_mapping m = {
		.from = key,
	};

	m.to = get_u32(value, "to");

	addattr_l(n, 1024, IFLA_VLAN_QOS_MAPPING, &m, sizeof(m));
}

#ifndef NLMSG_TAIL
#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))
#endif

static struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type)
{
	struct rtattr *nest = NLMSG_TAIL(n);

	addattr_l(n, maxlen, type, NULL, 0);
	return nest;
}

static int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest)
{
	nest->rta_len = (void *)NLMSG_TAIL(n) - (void *)nest;
	return n->nlmsg_len;
}

static void vlan_parse_qos_map(char ***argvp, struct nlmsghdr *n,
			      int attrtype)
{
	struct rtattr *tail;

	tail = addattr_nest(n, 1024, attrtype);

	parse_mapping(argvp, &parse_qos_mapping, n);

	addattr_nest_end(n, tail);
}

static void vlan_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{
	static const char keywords[] ALIGN1 =
		"id\0"
		"protocol\0"
		"reorder_hdr\0"
		"gvrp\0"
		"mvrp\0"
		"loose_binding\0"
		"ingress-qos-map\0"
		"egress-qos-map\0"
	;
	static const char protocols[] ALIGN1 =
		"802.1q\0"
		"802.1ad\0"
	;
	enum {
		ARG_id = 0,
		ARG_protocol,
		ARG_reorder_hdr,
		ARG_gvrp,
		ARG_mvrp,
		ARG_loose_binding,
		ARG_ingress_qos_map,
		ARG_egress_qos_map,
	};
	enum {
		PROTO_8021Q = 0,
		PROTO_8021AD,
	};
	enum {
		PARM_on = 0,
		PARM_off
	};
	int arg;
	uint16_t id, proto;
	struct ifla_vlan_flags flags = {};

	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type vlan");

		NEXT_ARG();
		if (arg == ARG_id) {
			id = get_u16(*argv, "id");
			addattr_l(n, size, IFLA_VLAN_ID, &id, sizeof(id));
		} else if (arg == ARG_protocol) {
			arg = index_in_substrings(protocols, str_tolower(*argv));
			if (arg == PROTO_8021Q)
				proto = htons(ETH_P_8021Q);
			else if (arg == PROTO_8021AD)
				proto = htons(ETH_P_8021AD);
			else
				bb_error_msg_and_die("unknown VLAN encapsulation protocol '%s'",
								     *argv);
			addattr_l(n, size, IFLA_VLAN_PROTOCOL, &proto, sizeof(proto));
		} else if (arg == ARG_ingress_qos_map) {
			vlan_parse_qos_map(&argv, n, IFLA_VLAN_INGRESS_QOS);
		} else if (arg == ARG_egress_qos_map) {
			vlan_parse_qos_map(&argv, n, IFLA_VLAN_EGRESS_QOS);
		} else {
			int param = index_in_strings(str_on_off, *argv);
			if (param < 0)
				die_must_be_on_off(nth_string(keywords, arg));

			if (arg == ARG_reorder_hdr) {
				flags.mask |= VLAN_FLAG_REORDER_HDR;
				flags.flags &= ~VLAN_FLAG_REORDER_HDR;
				if (param == PARM_on)
					flags.flags |= VLAN_FLAG_REORDER_HDR;
			} else if (arg == ARG_gvrp) {
				flags.mask |= VLAN_FLAG_GVRP;
				flags.flags &= ~VLAN_FLAG_GVRP;
				if (param == PARM_on)
					flags.flags |= VLAN_FLAG_GVRP;
			} else if (arg == ARG_mvrp) {
				flags.mask |= VLAN_FLAG_MVRP;
				flags.flags &= ~VLAN_FLAG_MVRP;
				if (param == PARM_on)
					flags.flags |= VLAN_FLAG_MVRP;
			} else if (arg == ARG_loose_binding) {
				flags.mask |= VLAN_FLAG_LOOSE_BINDING;
				flags.flags &= ~VLAN_FLAG_LOOSE_BINDING;
				if (param == PARM_on)
					flags.flags |= VLAN_FLAG_LOOSE_BINDING;
		}
		argv++;
	}

	if (flags.mask)
		addattr_l(n, size, IFLA_VLAN_FLAGS, &flags, sizeof(flags));
}

static void vrf_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{
/* IFLA_VRF_TABLE is an enum, not a define -
 * can't test "defined(IFLA_VRF_TABLE)".
 */
#if !defined(IFLA_VRF_MAX)
# define IFLA_VRF_TABLE 1
#endif
	uint32_t table;

	if (strcmp(*argv, "table") != 0)
		invarg_1_to_2(*argv, "type vrf");

	NEXT_ARG();
	table = get_u32(*argv, "table");
	addattr_l(n, size, IFLA_VRF_TABLE, &table, sizeof(table));
}

/* VXLAN section */
enum {
	IFLA_VXLAN_UNSPEC,
	IFLA_VXLAN_ID,
	IFLA_VXLAN_GROUP,	/* group or remote address */
	IFLA_VXLAN_LINK,
	IFLA_VXLAN_LOCAL,
	IFLA_VXLAN_TTL,
	IFLA_VXLAN_TOS,
	IFLA_VXLAN_LEARNING,
	IFLA_VXLAN_AGEING,
	IFLA_VXLAN_LIMIT,
	IFLA_VXLAN_PORT_RANGE,	/* source port */
	IFLA_VXLAN_PROXY,
	IFLA_VXLAN_RSC,
	IFLA_VXLAN_L2MISS,
	IFLA_VXLAN_L3MISS,
	IFLA_VXLAN_PORT,	/* destination port */
	IFLA_VXLAN_GROUP6,
	IFLA_VXLAN_LOCAL6,
	IFLA_VXLAN_UDP_CSUM,
	IFLA_VXLAN_UDP_ZERO_CSUM6_TX,
	IFLA_VXLAN_UDP_ZERO_CSUM6_RX,
	IFLA_VXLAN_REMCSUM_TX,
	IFLA_VXLAN_REMCSUM_RX,
	IFLA_VXLAN_GBP,
	IFLA_VXLAN_REMCSUM_NOPARTIAL,
	IFLA_VXLAN_COLLECT_METADATA,
	IFLA_VXLAN_LABEL,
	IFLA_VXLAN_GPE,
	IFLA_VXLAN_TTL_INHERIT,
	IFLA_VXLAN_DF,
	IFLA_VXLAN_VNIFILTER, /* only applicable with COLLECT_METADATA mode */
	__IFLA_VXLAN_MAX
};
#define IFLA_VXLAN_MAX	(__IFLA_VXLAN_MAX - 1)

struct ifla_vxlan_port_range {
	uint16_t	low;
	uint16_t	high;
};

enum ifla_vxlan_df {
	VXLAN_DF_UNSET = 0,
	VXLAN_DF_SET,
	VXLAN_DF_INHERIT,
	__VXLAN_DF_END,
	VXLAN_DF_MAX = __VXLAN_DF_END - 1,
};


#define VXLAN_ATTRSET(attrs, type) (((attrs) & (1L << (type))) != 0)

static void check_duparg(uint64_t *attrs, int type, const char *key,
			 const char *argv)
{
	if (!VXLAN_ATTRSET(*attrs, type)) {
		*attrs |= (1L << type);
		return;
	}
	duparg2(key, argv);
}

/* This uses a non-standard parsing (ie not inet_aton, or inet_pton)
 * because of legacy choice to parse 10.8 as 10.8.0.0 not 10.0.0.8
 */

static inline bool is_addrtype_inet(const inet_prefix *p)
{
	return p->flags & ADDRTYPE_INET;
}

static inline bool is_addrtype_inet_unspec(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_UNSPEC) == ADDRTYPE_INET_UNSPEC;
}

static inline bool is_addrtype_inet_multi(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_MULTI) == ADDRTYPE_INET_MULTI;
}

static inline bool is_addrtype_inet_not_unspec(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_UNSPEC) == ADDRTYPE_INET;
}

static inline bool is_addrtype_inet_not_multi(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_MULTI) == ADDRTYPE_INET;
}


static void nodev(const char *dev)
{
	bb_error_msg_and_die("Cannot find device \"%s\"", dev);
}

#ifndef LABEL_MAX_MASK
#define     LABEL_MAX_MASK          0xFFFFFU
#endif

static void invarg(const char *msg, const char *arg)
{
	bb_error_msg_and_die("Error: argument \"%s\" is wrong: %s", arg, msg);
}

static int addattr(struct nlmsghdr *n, int maxlen, int type)
{
	return addattr_l(n, maxlen, type, NULL, 0);
}

static int addattr16(struct nlmsghdr *n, int maxlen, int type, uint16_t data)
{
	return addattr_l(n, maxlen, type, &data, sizeof(uint16_t));
}


static int get_be16(const char *arg, const char *msg)
{
	return htons(get_u16(arg, msg));
}

static void print_explain(void)
{
	bb_simple_error_msg_and_die(
		"Usage: ... vxlan id VNI\n"
		"		[ { group | remote } IP_ADDRESS ]\n"
		"		[ local ADDR ]\n"
		"		[ ttl TTL ]\n"
		"		[ tos TOS ]\n"
		"		[ df DF ]\n"
		"		[ flowlabel LABEL ]\n"
		"		[ dev PHYS_DEV ]\n"
		"		[ dstport PORT ]\n"
		"		[ srcport MIN MAX ]\n"
		"		[ [no]learning ]\n"
		"		[ [no]proxy ]\n"
		"		[ [no]rsc ]\n"
		"		[ [no]l2miss ]\n"
		"		[ [no]l3miss ]\n"
		"		[ ageing SECONDS ]\n"
		"		[ maxaddress NUMBER ]\n"
		"		[ [no]udpcsum ]\n"
		"		[ [no]udp6zerocsumtx ]\n"
		"		[ [no]udp6zerocsumrx ]\n"
		"		[ [no]remcsumtx ] [ [no]remcsumrx ]\n"
		"		[ [no]external ] [ gbp ] [ gpe ]\n"
		"		[ [no]vnifilter ]\n"
		"\n"
		"Where:	VNI	:= 0-16777215\n"
		"	ADDR	:= { IP_ADDRESS | any }\n"
		"	TOS	:= { NUMBER | inherit }\n"
		"	TTL	:= { 1..255 | auto | inherit }\n"
		"	DF	:= { unset | set | inherit }\n"
		"	LABEL := 0-1048575");
}

static void explain(void)
{
	print_explain();
}

static int vxlan_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{
	inet_prefix saddr, daddr;
	int arg;
	uint32_t vni = 0;
	uint8_t learning = 1;
	uint8_t vnifilter = 0;
	uint16_t dstport = 0;
	uint8_t metadata = 0;
	uint64_t attrs = 0;
	bool set_op = (n->nlmsg_type == RTM_NEWLINK &&
		       !(n->nlmsg_flags & NLM_F_CREATE));
	bool selected_family = false;

	static const char keywords[] ALIGN1 =
		"id\0"
		"vni\0"
		"group\0"
		"remote\0"
		"local\0"
		"dev\0"
		"ttl\0"
		"hoplimit\0"
		"tos\0"
		"dsfield\0"
		"df\0"
		"label\0"
		"flowlabel\0"
		"ageing\0"
		"maxaddress\0"
		"port\0"
		"srcport\0"
		"dstport\0"
		"learning\0"
		"nolearning\0"
		"proxy\0"
		"noproxy\0"
		"rsc\0"
		"norsc\0"
		"l2miss\0"
		"nol2miss\0"
		"l3miss\0"
		"nol3miss\0"
		"udpcsum\0"
		"noudpcsum\0"
		"udp6zerocsumtx\0"
		"noudp6zerocsumtx\0"
		"udp6zerocsumrx\0"
		"noudp6zerocsumrx\0"
		"remcsumtx\0"
		"noremcsumtx\0"
		"remcsumrx\0"
		"noremcsumrx\0"
		"external\0"
		"noexternal\0"
		"gbp\0"
		"gpe\0"
		"vnifilter\0"
		"novnifilter\0"
		"help\0"
	;

	enum {
		ARG_id = 0,
		ARG_vni,
		ARG_group,
		ARG_remote,
		ARG_local,
		ARG_dev,
		ARG_ttl,
		ARG_hoplimit,
		ARG_tos,
		ARG_dsfield,
		ARG_df,
		ARG_label,
		ARG_flowlabel,
		ARG_ageing,
		ARG_maxaddress,
		ARG_port,
		ARG_srcport,
		ARG_dstport,
		ARG_learning,
		ARG_nolearning,
		ARG_proxy,
		ARG_noproxy,
		ARG_rsc,
		ARG_norsc,
		ARG_l2miss,
		ARG_nol2miss,
		ARG_l3miss,
		ARG_nol3miss,
		ARG_udpcsum,
		ARG_noudpcsum,
		ARG_udp6zerocsumtx,
		ARG_noudp6zerocsumtx,
		ARG_udp6zerocsumrx,
		ARG_noudp6zerocsumrx,
		ARG_remcsumtx,
		ARG_noremcsumtx,
		ARG_remcsumrx,
		ARG_noremcsumrx,
		ARG_external,
		ARG_noexternal,
		ARG_gbp,
		ARG_gpe,
		ARG_vnifilter,
		ARG_novnifilter,
		ARG_help,
	};

	saddr.family = daddr.family = AF_UNSPEC;

	inet_prefix_reset(&saddr);
	inet_prefix_reset(&daddr);

	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type vxlan");
		if (arg == ARG_id || arg == ARG_vni) {
			/* We will add ID attribute outside of the loop since we
			 * need to consider metadata information as well.
			 */
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_ID, "id", *argv);
			vni = get_u32(*argv, "id");
			if (vni >= 1u << 24)
				invarg("invalid id", *argv);
		} else if (arg == ARG_group) {
			if (is_addrtype_inet_not_multi(&daddr)) {
				bb_simple_error_msg_and_die("vxlan: both group and remote cannot be specified");
			}
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_GROUP, "group", *argv);
			get_addr(&daddr, *argv, saddr.family);
			if (!is_addrtype_inet_multi(&daddr))
				invarg("invalid group address", *argv);
		} else if (arg == ARG_remote) {
			if (is_addrtype_inet_multi(&daddr)) {
				bb_simple_error_msg_and_die("vxlan: both group and remote cannot be specified");
			}
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_GROUP, "remote", *argv);
			get_addr(&daddr, *argv, saddr.family);
			if (!is_addrtype_inet_not_multi(&daddr))
				invarg("invalid remote address", *argv);
		} else if (arg == ARG_local) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LOCAL, "local", *argv);
			get_addr(&saddr, *argv, daddr.family);
			if (!is_addrtype_inet_not_multi(&saddr))
				invarg("invalid local address", *argv);
		} else if (arg == ARG_dev) {
			unsigned int link;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LINK, "dev", *argv);
			link = xll_name_to_index(*argv);
			if (!link)
				nodev(*argv);
			addattr32(n, size, IFLA_VXLAN_LINK, link);
		} else if (arg == ARG_ttl || arg == ARG_hoplimit) {
			unsigned int uval;
			__u8 ttl = 0;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_TTL, "ttl", *argv);
			if (strcmp(*argv, "inherit") == 0) {
				addattr(n, size, IFLA_VXLAN_TTL_INHERIT);
			} else if (strcmp(*argv, "auto") == 0) {
				addattr8(n, size, IFLA_VXLAN_TTL, ttl);
			} else {
				uval = get_unsigned(*argv, "ttl");
				if (uval > 255)
					invarg("TTL must be <= 255", *argv);
				ttl = uval;
				addattr8(n, size, IFLA_VXLAN_TTL, ttl);
			}
		} else if (arg == ARG_tos || arg == ARG_dsfield) {
			uint32_t uval;
			__u8 tos;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_TOS, "tos", *argv);
			if (strcmp(*argv, "inherit") != 0) {
				if (rtnl_dsfield_a2n(&uval, *argv))
					invarg("bad TOS value", *argv);
				tos = uval;
			} else
				tos = 1;
			addattr8(n, size, IFLA_VXLAN_TOS, tos);
		} else if (arg == ARG_df) {
			enum ifla_vxlan_df df;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_DF, "df", *argv);
			if (strcmp(*argv, "unset") == 0)
				df = VXLAN_DF_UNSET;
			else if (strcmp(*argv, "set") == 0)
				df = VXLAN_DF_SET;
			else if (strcmp(*argv, "inherit") == 0)
				df = VXLAN_DF_INHERIT;
			else
				invarg("DF must be 'unset', 'set' or 'inherit'",
				       *argv);

			addattr8(n, size, IFLA_VXLAN_DF, df);
		} else if (arg == ARG_label || arg == ARG_flowlabel) {
			uint32_t uval;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LABEL, "flowlabel",
				     *argv);
			uval = get_u32(*argv, "flowlabel");
			if ((uval & ~LABEL_MAX_MASK))
				invarg("invalid flowlabel", *argv);
			addattr32(n, size, IFLA_VXLAN_LABEL, htonl(uval));
		} else if (arg == ARG_ageing) {
			uint32_t age;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_AGEING, "ageing",
				     *argv);
			if (strcmp(*argv, "none") == 0)
				age = 0;
			else 
			    age = get_u32(*argv, "ageing timer");
			addattr32(n, size, IFLA_VXLAN_AGEING, age);
		} else if (arg == ARG_maxaddress) {
			uint32_t maxaddr;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LIMIT,
				     "maxaddress", *argv);
			if (strcmp(*argv, "unlimited") == 0)
				maxaddr = 0;
			else maxaddr = get_u32(*argv, "max addresses");
			addattr32(n, size, IFLA_VXLAN_LIMIT, maxaddr);
		} else if (arg == ARG_port || arg == ARG_srcport) {
			struct ifla_vxlan_port_range range = { 0, 0 };

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_PORT_RANGE, "srcport",
				     *argv);
			range.low = get_be16(*argv, "min port");
			NEXT_ARG();
			range.high = get_be16(*argv, "max port");
			if (range.low || range.high) {
				addattr_l(n, size, IFLA_VXLAN_PORT_RANGE,
					  &range, sizeof(range));
			}
		} else if (arg == ARG_dstport) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_PORT, "dstport", *argv);
			dstport = get_u16(*argv, "dst port");
		} else if (arg == ARG_nolearning) {
			check_duparg(&attrs, IFLA_VXLAN_LEARNING, *argv, *argv);
			learning = 0;
		} else if (arg == ARG_learning) {
			check_duparg(&attrs, IFLA_VXLAN_LEARNING, *argv, *argv);
			learning = 1;
		} else if (arg == ARG_noproxy) {
			check_duparg(&attrs, IFLA_VXLAN_PROXY, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_PROXY, 0);
		} else if (arg == ARG_proxy) {
			check_duparg(&attrs, IFLA_VXLAN_PROXY, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_PROXY, 1);
		} else if (arg == ARG_norsc) {
			check_duparg(&attrs, IFLA_VXLAN_RSC, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_RSC, 0);
		} else if (arg == ARG_rsc) {
			check_duparg(&attrs, IFLA_VXLAN_RSC, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_RSC, 1);
		} else if (arg == ARG_nol2miss) {
			check_duparg(&attrs, IFLA_VXLAN_L2MISS, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_L2MISS, 0);
		} else if (arg == ARG_l2miss) {
			check_duparg(&attrs, IFLA_VXLAN_L2MISS, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_L2MISS, 1);
		} else if (arg == ARG_nol3miss) {
			check_duparg(&attrs, IFLA_VXLAN_L3MISS, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_L3MISS, 0);
		} else if (arg == ARG_l3miss) {
			check_duparg(&attrs, IFLA_VXLAN_L3MISS, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_L3MISS, 1);
		} else if (arg == ARG_udpcsum) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_CSUM, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_CSUM, 1);
		} else if (arg == ARG_noudpcsum) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_CSUM, *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_CSUM, 0);
		} else if (arg == ARG_udp6zerocsumtx) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_ZERO_CSUM6_TX, 1);
		} else if (arg == ARG_noudp6zerocsumtx) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_ZERO_CSUM6_TX, 0);
		} else if (arg == ARG_udp6zerocsumrx) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_ZERO_CSUM6_RX, 1);
		} else if (arg == ARG_noudp6zerocsumrx) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_UDP_ZERO_CSUM6_RX, 0);
		} else if (arg == ARG_remcsumtx) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_TX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_REMCSUM_TX, 1);
		} else if (arg == ARG_noremcsumtx) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_TX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_REMCSUM_TX, 0);
		} else if (arg == ARG_remcsumrx) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_RX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_REMCSUM_RX, 1);
		} else if (arg == ARG_noremcsumrx) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_RX,
				     *argv, *argv);
			addattr8(n, size, IFLA_VXLAN_REMCSUM_RX, 0);
		} else if (arg == ARG_external) {
			check_duparg(&attrs, IFLA_VXLAN_COLLECT_METADATA,
				     *argv, *argv);
			metadata = 1;
			learning = 0;
			/* we will add LEARNING attribute outside of the loop */
			addattr8(n, size, IFLA_VXLAN_COLLECT_METADATA,
				 metadata);
		} else if (arg == ARG_noexternal) {
			check_duparg(&attrs, IFLA_VXLAN_COLLECT_METADATA,
				     *argv, *argv);
			metadata = 0;
			addattr8(n, size, IFLA_VXLAN_COLLECT_METADATA,
				 metadata);
		} else if (arg == ARG_vnifilter) {
			check_duparg(&attrs, IFLA_VXLAN_VNIFILTER,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_VNIFILTER, 1);
			vnifilter = 1;
		} else if (arg == ARG_novnifilter) {
			check_duparg(&attrs, IFLA_VXLAN_VNIFILTER,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_VNIFILTER, 0);
		} else if (arg == ARG_gbp) {
			check_duparg(&attrs, IFLA_VXLAN_GBP, *argv, *argv);
			addattr_l(n, size, IFLA_VXLAN_GBP, NULL, 0);
		} else if (arg == ARG_gpe) {
			check_duparg(&attrs, IFLA_VXLAN_GPE, *argv, *argv);
			addattr_l(n, size, IFLA_VXLAN_GPE, NULL, 0);
		} else if (arg == ARG_help) {
			explain();
		} else {
			bb_error_msg_and_die("vxlan: unknown command \"%s\"?", *argv);
		}
		argv++;
	}

	if (!metadata && vnifilter) {
		bb_simple_error_msg_and_die("vxlan: vnifilter is valid only when 'external' is set");
	}

	if (metadata && VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID)) {
		bb_simple_error_msg_and_die("vxlan: both 'external' and vni cannot be specified");
	}

	if (!metadata && !vnifilter && !VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID) && !set_op) {
		bb_simple_error_msg_and_die("vxlan: missing virtual network identifier");
	}

	if (is_addrtype_inet_multi(&daddr) &&
	    !VXLAN_ATTRSET(attrs, IFLA_VXLAN_LINK)) {
		bb_simple_error_msg_and_die("vxlan: 'group' requires 'dev' to be specified");
	}

	if (!VXLAN_ATTRSET(attrs, IFLA_VXLAN_PORT) &&
	    VXLAN_ATTRSET(attrs, IFLA_VXLAN_GPE)) {
		dstport = 4790;
	}

	if (VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID))
		addattr32(n, size, IFLA_VXLAN_ID, vni);

	if (is_addrtype_inet(&saddr)) {
		int type = (saddr.family == AF_INET) ? IFLA_VXLAN_LOCAL
						     : IFLA_VXLAN_LOCAL6;
		addattr_l(n, size, type, saddr.data, saddr.bytelen);
		selected_family = true;
	}

	if (is_addrtype_inet(&daddr)) {
		int type = (daddr.family == AF_INET) ? IFLA_VXLAN_GROUP
						     : IFLA_VXLAN_GROUP6;
		addattr_l(n, size, type, daddr.data, daddr.bytelen);
		selected_family = true;
	}

	if (!selected_family) {
		if (preferred_family == AF_INET) {
			get_addr(&daddr, "default", AF_INET);
			addattr_l(n, size, IFLA_VXLAN_GROUP,
				  daddr.data, daddr.bytelen);
		} else if (preferred_family == AF_INET6) {
			get_addr(&daddr, "default", AF_INET6);
			addattr_l(n, size, IFLA_VXLAN_GROUP6,
				  daddr.data, daddr.bytelen);
		}
	}

	if (!set_op || VXLAN_ATTRSET(attrs, IFLA_VXLAN_LEARNING))
		addattr8(n, size, IFLA_VXLAN_LEARNING, learning);

	if (dstport)
		addattr16(n, size, IFLA_VXLAN_PORT, htons(dstport));

	return 0;
}



/* Return value becomes exitcode. It's okay to not return at all */
static int do_add_or_delete(char **argv, const unsigned rtm)
{
	static const char keywords[] ALIGN1 =
		"link\0""name\0""type\0""dev\0""address\0";
	enum {
		ARG_link,
		ARG_name,
		ARG_type,
		ARG_dev,
		ARG_address,
	};
	struct rtnl_handle rth;
	struct {
		struct nlmsghdr  n;
		struct ifinfomsg i;
		char             buf[1024];
	} req;
	smalluint arg;
	char *name_str = NULL;
	char *link_str = NULL;
	char *type_str = NULL;
	char *dev_str = NULL;
	char *address_str = NULL;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = rtm;
	req.i.ifi_family = preferred_family;
	if (rtm == RTM_NEWLINK)
		req.n.nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;

	/* NB: update iplink_full_usage if you extend this code */

	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg == ARG_type) {
			NEXT_ARG();
			type_str = *argv++;
			dbg("type_str:'%s'", type_str);
			break;
		}
		if (arg == ARG_link) {
			NEXT_ARG();
			link_str = *argv;
			dbg("link_str:'%s'", link_str);
		} else if (arg == ARG_name) {
			NEXT_ARG();
			name_str = *argv;
			dbg("name_str:'%s'", name_str);
		} else if (arg == ARG_address) {
			NEXT_ARG();
			address_str = *argv;
			dbg("address_str:'%s'", address_str);
		} else {
			if (arg == ARG_dev) {
				if (dev_str)
					duparg(*argv, "dev");
				NEXT_ARG();
			}
			dev_str = *argv;
			dbg("dev_str:'%s'", dev_str);
		}
		argv++;
	}
	xrtnl_open(&rth);
	ll_init_map(&rth);
	if (type_str && rtm == RTM_NEWLINK) {
		struct rtattr *linkinfo = NLMSG_TAIL(&req.n);

		addattr_l(&req.n, sizeof(req), IFLA_LINKINFO, NULL, 0);
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, type_str,
				strlen(type_str));

		if (*argv) {
			struct rtattr *data = NLMSG_TAIL(&req.n);
			addattr_l(&req.n, sizeof(req), IFLA_INFO_DATA, NULL, 0);

			if (strcmp(type_str, "vlan") == 0)
				vlan_parse_opt(argv, &req.n, sizeof(req));
			else if (strcmp(type_str, "vrf") == 0)
				vrf_parse_opt(argv, &req.n, sizeof(req));
#if ENABLE_VXLAN
			else if (strcmp(type_str, "vxlan") == 0)
				vxlan_parse_opt(argv, &req.n, sizeof(req));
#endif
			data->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)data;
		}

		linkinfo->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)linkinfo;
	}
	/* Allow "ip link add dev" and "ip link add name" */
	if (!name_str)
		name_str = dev_str;
	else if (!dev_str)
		dev_str = name_str;
	/* else if (!strcmp(name_str, dev_str))
		name_str = dev_str; */

	if (rtm != RTM_NEWLINK) {
		if (!dev_str)
			return 1; /* Need a device to delete */
		req.i.ifi_index = xll_name_to_index(dev_str);
	} else {
		if (link_str) {
			int idx = xll_name_to_index(link_str);
			addattr_l(&req.n, sizeof(req), IFLA_LINK, &idx, 4);
		}
		if (address_str) {
			unsigned char abuf[32];
			int len = ll_addr_a2n(abuf, sizeof(abuf), address_str);
			dbg("address len:%d", len);
			if (len < 0)
				return -1;
			addattr_l(&req.n, sizeof(req), IFLA_ADDRESS, abuf, len);
		}
	}
	if (name_str) {
		const size_t name_len = strlen(name_str) + 1;
		if (name_len < 2 || name_len > IFNAMSIZ)
			invarg_1_to_2(name_str, "name");
		addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name_str, name_len);
	}
	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		return 2;
	return 0;
}

/* Return value becomes exitcode. It's okay to not return at all */
int FAST_FUNC do_iplink(char **argv)
{
	static const char keywords[] ALIGN1 =
		"add\0""delete\0""set\0""show\0""lst\0""list\0";

	xfunc_error_retval = 2; //TODO: move up to "ip"? Is it the common rule for all "ip" tools?
	if (*argv) {
		int key = index_in_substrings(keywords, *argv);
		if (key < 0) /* invalid argument */
			invarg_1_to_2(*argv, applet_name);
		argv++;
		if (key <= 1) /* add/delete */
			return do_add_or_delete(argv, key ? RTM_DELLINK : RTM_NEWLINK);
		if (key == 2) /* set */
			return do_set(argv);
	}
	/* show, lst, list */
	return ipaddr_list_link(argv);
}

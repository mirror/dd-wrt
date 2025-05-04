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
#include <netinet/ether.h>

#include <linux/if_vlan.h>
#if ENABLE_FEATURE_IP_LINK_CAN
# include <linux/can/netlink.h>
#endif
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

#ifndef NLMSG_TAIL
#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))
#endif

#ifndef IFLA_LINKINFO
# define IFLA_LINKINFO 18
# define IFLA_INFO_KIND 1
# define IFLA_INFO_DATA 2
#endif

#ifndef IFLA_VLAN_MAX
# define IFLA_VLAN_ID 1
# define IFLA_VLAN_FLAGS 2
# define IFLA_VLAN_EGRESS_QOS 3
# define IFLA_VLAN_INGRESS_QOS 4

#define IFLA_VLAN_MAX	(__IFLA_VLAN_MAX - 1)

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

enum {
	PARM_on = 0,
	PARM_off
};

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
		exit_FAILURE();
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

#if ENABLE_FEATURE_IP_LINK_CAN
static uint32_t get_float_1000(char *arg, const char *errmsg)
{
	uint32_t ret;
	double d;
	char *ptr;

	errno = 0;
//TODO: needs setlocale(LC_NUMERIC, "C")?
	d = strtod(arg, &ptr);
	if (errno || ptr == arg || *ptr
	 || d > (0xFFFFFFFFU / 1000) || d < 0
	) {
		invarg_1_to_2(arg, errmsg); /* does not return */
	}
	ret = d * 1000;

	return ret;
}

static void do_set_can(char *dev, char **argv)
{
	struct can_bittiming bt = {}, dbt = {};
	struct can_ctrlmode cm = {};
	char *keyword;
	static const char keywords[] ALIGN1 =
		"bitrate\0""sample-point\0""tq\0"
		"prop-seg\0""phase-seg1\0""phase-seg2\0""sjw\0"
		"dbitrate\0""dsample-point\0""dtq\0"
		"dprop-seg\0""dphase-seg1\0""dphase-seg2\0""dsjw\0"
		"loopback\0""listen-only\0""triple-sampling\0"
		"one-shot\0""berr-reporting\0"
		"fd\0""fd-non-iso\0""presume-ack\0"
		"cc-len8-dlc\0""restart\0""restart-ms\0"
		"termination\0";
	enum { ARG_bitrate = 0, ARG_sample_point, ARG_tq,
		ARG_prop_seg, ARG_phase_seg1, ARG_phase_seg2, ARG_sjw,
		ARG_dbitrate, ARG_dsample_point, ARG_dtq,
		ARG_dprop_seg, ARG_dphase_seg1, ARG_dphase_seg2, ARG_dsjw,
		ARG_loopback, ARG_listen_only, ARG_triple_sampling,
		ARG_one_shot, ARG_berr_reporting,
		ARG_fd, ARG_fd_non_iso, ARG_presume_ack,
		ARG_cc_len8_dlc, ARG_restart, ARG_restart_ms,
		ARG_termination };
	struct rtnl_handle rth;
	struct {
		struct nlmsghdr  n;
		struct ifinfomsg i;
		char buf[1024];
	} req;
	size_t dev_len;
	struct rtattr *linkinfo, *data;
	smalluint key, param;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_NEWLINK;
	req.i.ifi_family = preferred_family;
	xrtnl_open(&rth);
	req.i.ifi_index = xll_name_to_index(dev);
	dev_len = strlen(dev);
	if (dev_len < 2 || dev_len > IFNAMSIZ)
		invarg_1_to_2(dev, "dev");

	addattr_l(&req.n, sizeof(req), IFLA_IFNAME, dev, dev_len);
	linkinfo = NLMSG_TAIL(&req.n);
	addattr_l(&req.n, sizeof(req), IFLA_LINKINFO, NULL, 0);
	addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, (void *)"can",
		  strlen("can"));
	data = NLMSG_TAIL(&req.n);
	addattr_l(&req.n, sizeof(req), IFLA_INFO_DATA, NULL, 0);
	while (*argv) {
		key = index_in_substrings(keywords, *argv);
		keyword = *argv;
		//printf("%s: key: %d, *argv: %s\n", __func__, key, *argv);
		switch (key) {
		case ARG_bitrate:
		case ARG_tq:
		case ARG_prop_seg:
		case ARG_phase_seg1:
		case ARG_phase_seg2:
		case ARG_sjw: {
			uint32_t *val;
			NEXT_ARG();
			if (key == ARG_bitrate)
				val = &bt.bitrate;
			else if (key == ARG_tq)
				val = &bt.tq;
			else if (key == ARG_prop_seg)
				val = &bt.prop_seg;
			else if (key == ARG_phase_seg1)
				val = &bt.phase_seg1;
			else if (key == ARG_phase_seg2)
				val = &bt.phase_seg2;
			else
				val = &bt.sjw;

			*val = get_u32(*argv, keyword);
			break;
		}
		case ARG_sample_point: {
			NEXT_ARG();
			bt.sample_point = get_float_1000(*argv, keyword);
			break;
		}
		case ARG_dbitrate:
		case ARG_dtq:
		case ARG_dprop_seg:
		case ARG_dphase_seg1:
		case ARG_dphase_seg2:
		case ARG_dsjw: {
			uint32_t *val;
			NEXT_ARG();
			if (key == ARG_dbitrate)
				val = &dbt.bitrate;
			else if (key == ARG_dtq)
				val = &dbt.tq;
			else if (key == ARG_dprop_seg)
				val = &dbt.prop_seg;
			else if (key == ARG_dphase_seg1)
				val = &dbt.phase_seg1;
			else if (key == ARG_dphase_seg2)
				val = &dbt.phase_seg2;
			else
				val = &dbt.sjw;

			*val = get_u32(*argv, keyword);
			break;
		}
		case ARG_dsample_point: {
			NEXT_ARG();
			dbt.sample_point = get_float_1000(*argv, keyword);
			break;
		}
		case ARG_loopback:
		case ARG_listen_only:
		case ARG_triple_sampling:
		case ARG_one_shot:
		case ARG_berr_reporting:
		case ARG_fd:
		case ARG_fd_non_iso:
		case ARG_presume_ack:
		case ARG_cc_len8_dlc: {
			uint32_t flag = 0;

			NEXT_ARG();
			param = index_in_strings(str_on_off, *argv);
			if (param < 0)
				die_must_be_on_off(keyword);

			if (key == ARG_loopback)
				flag = CAN_CTRLMODE_LOOPBACK;
			else if (key == ARG_listen_only)
				flag = CAN_CTRLMODE_LISTENONLY;
			else if (key == ARG_triple_sampling)
				flag = CAN_CTRLMODE_3_SAMPLES;
			else if (key == ARG_one_shot)
				flag = CAN_CTRLMODE_ONE_SHOT;
			else if (key == ARG_berr_reporting)
				flag = CAN_CTRLMODE_BERR_REPORTING;
			else if (key == ARG_fd)
				flag = CAN_CTRLMODE_FD;
			else if (key == ARG_fd_non_iso)
				flag = CAN_CTRLMODE_FD_NON_ISO;
			else if (key == ARG_presume_ack)
				flag = CAN_CTRLMODE_PRESUME_ACK;
			else
#if defined(CAN_CTRLMODE_CC_LEN8_DLC)
				flag = CAN_CTRLMODE_CC_LEN8_DLC;
#else
				die_must_be_on_off(keyword);
#endif
			cm.mask |= flag;
			if (param == PARM_on)
				cm.flags |= flag;

			break;
		}
		case ARG_restart: {
			uint32_t val = 1;
			/*NEXT_ARG(); - WRONG? */
			addattr_l(&req.n, sizeof(req), IFLA_CAN_RESTART, &val, sizeof(val));
			break;
		}
		case ARG_restart_ms: {
			uint32_t val;
			NEXT_ARG();
			val = get_u32(*argv, keyword);
			addattr_l(&req.n, sizeof(req), IFLA_CAN_RESTART_MS, &val, sizeof(val));
			break;
		}
		case ARG_termination: {
			uint16_t val;
			NEXT_ARG();
			val = get_u16(*argv, keyword);
			addattr_l(&req.n, sizeof(req), IFLA_CAN_TERMINATION, &val, sizeof(val));
			break;
		}
		default:
			break;
		}
		argv++;
	}

	if (bt.bitrate || bt.tq)
		addattr_l(&req.n, sizeof(req), IFLA_CAN_BITTIMING, &bt, sizeof(bt));

	if (cm.mask)
		addattr_l(&req.n, sizeof(req), IFLA_CAN_CTRLMODE, &cm, sizeof(cm));

	data->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)data;
	linkinfo->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)linkinfo;

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		xfunc_die();
}

static void set_type(char *type, char *dev, char **argv)
{
/* When we have more than just "type can ARGS" supported, maybe:
	static const char keywords[] ALIGN1 = ""
		IF_FEATURE_IP_LINK_CAN("can\0")
		;
	typedef void FAST_FUNC(*ip_type_set_func_ptr_t)(char*, char**);
	static const ip_type_set_func_ptr_t funcs[] ALIGN_PTR = {
		IF_FEATURE_IP_LINK_CAN(do_set_can,)
	};
	ip_type_set_func_ptr_t func;
	int key;

	key = index_in_substrings(keywords, type);
	if (key < 0)
		invarg_1_to_2(type, "type");
	func = funcs[key];
	func(dev, argv);
*/
	if (strcmp(type, "can") != 0)
		invarg_1_to_2(type, "type");
	do_set_can(dev, argv);
}
#endif

/* Return value becomes exitcode. It's okay to not return at all */
static int do_set(char **argv)
{
	char *dev = NULL;
#if ENABLE_FEATURE_IP_LINK_CAN
	char *type = NULL;
#endif
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
#if ENABLE_FEATURE_IP_LINK_CAN
		"type\0"
#endif
		"dev\0" /* must be last */;
	enum { ARG_up = 0, ARG_down, ARG_name, ARG_mtu, ARG_qlen, ARG_multicast,
		ARG_arp, ARG_promisc, ARG_addr, ARG_netns,
		ARG_master, ARG_nomaster,
#if ENABLE_FEATURE_IP_LINK_CAN
		ARG_type,
#endif
		ARG_dev };
	smalluint key;

	while (*argv) {
		/* substring search ensures that e.g. "addr" and "address"
		 * are both accepted */
		key = index_in_substrings(keywords, *argv);
		//printf("%s: key: %d, *argv: %s\n", __func__, key, *argv);
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
#if ENABLE_FEATURE_IP_LINK_CAN
		} else if (key == ARG_type) {
			NEXT_ARG();
			type = *argv;
			argv++;
			break;
#endif
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
#if ENABLE_FEATURE_IP_LINK_CAN
	if (type)
		set_type(type, dev, argv);
#endif
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
			continue;
		} else if (arg == ARG_egress_qos_map) {
			vlan_parse_qos_map(&argv, n, IFLA_VLAN_EGRESS_QOS);
			continue;
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

#ifndef IFLA_DSA_MAX
enum {
	IFLA_DSA_UNSPEC,
	IFLA_DSA_CONDUIT,
	/* Deprecated, use IFLA_DSA_CONDUIT instead */
	IFLA_DSA_MASTER = IFLA_DSA_CONDUIT,
	__IFLA_DSA_MAX,
};

#define IFLA_DSA_MAX	(__IFLA_DSA_MAX - 1)
#endif

static void invarg(const char *msg, const char *arg)
{
	bb_error_msg_and_die("Error: argument \"%s\" is wrong: %s", arg, msg);
}


static void dsa_explain(void)
{
	bb_simple_error_msg_and_die("Usage: ... dsa [ conduit DEVICE ]\n");
}

static void dsa_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{

	uint32_t table;
	while(*argv) {
		if (strcmp(*argv, "conduit") == 0 ||
		    strcmp(*argv, "master") == 0) {
			uint32_t ifindex;

			NEXT_ARG();
			ifindex = xll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
			addattr_l(n, 1024, IFLA_DSA_MASTER, &ifindex, 4);
		} else if (strcmp(*argv, "help") == 0) {
			dsa_explain();
			return;
		} else {
			bb_error_msg_and_die("dsa: unknown command \"%s\"?", *argv);
		}
		argv++;
	}
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

/* IPVLAN section */
enum {
	IFLA_IPVLAN_UNSPEC,
	IFLA_IPVLAN_MODE,
	IFLA_IPVLAN_FLAGS,
	__IFLA_IPVLAN_MAX
};

#define IFLA_IPVLAN_MAX (__IFLA_IPVLAN_MAX - 1)

enum ipvlan_mode {
	IPVLAN_MODE_L2 = 0,
	IPVLAN_MODE_L3,
	IPVLAN_MODE_L3S,
	IPVLAN_MODE_MAX
};

#define IPVLAN_F_PRIVATE	0x01
#define IPVLAN_F_VEPA		0x02

/* MACVLAN section */
enum {
	IFLA_COMPAT_MACVLAN_UNSPEC,
	IFLA_COMPAT_MACVLAN_MODE,
	IFLA_COMPAT_MACVLAN_FLAGS,
	IFLA_COMPAT_MACVLAN_MACADDR_MODE,
	IFLA_COMPAT_MACVLAN_MACADDR,
	IFLA_COMPAT_MACVLAN_MACADDR_DATA,
	IFLA_COMPAT_MACVLAN_MACADDR_COUNT,
	IFLA_COMPAT_MACVLAN_BC_QUEUE_LEN,
	IFLA_COMPAT_MACVLAN_BC_QUEUE_LEN_USED,
	IFLA_COMPAT_MACVLAN_BC_CUTOFF,
	__IFLA_COMPAT_MACVLAN_MAX,
};

#define IFLA_COMPAT_MACVLAN_MAX (__IFLA_COMPAT_MACVLAN_MAX - 1)

enum compat_macvlan_mode {
	COMPAT_MACVLAN_MODE_PRIVATE = 1, /* don't talk to other macvlans */
	COMPAT_MACVLAN_MODE_VEPA    = 2, /* talk to other ports through ext bridge */
	COMPAT_MACVLAN_MODE_BRIDGE  = 4, /* talk to bridge ports directly */
	COMPAT_MACVLAN_MODE_PASSTHRU = 8,/* take over the underlying device */
	COMPAT_MACVLAN_MODE_SOURCE  = 16,/* use source MAC address list to assign */
};

enum compat_macvlan_macaddr_mode {
	COMPAT_MACVLAN_MACADDR_ADD,
	COMPAT_MACVLAN_MACADDR_DEL,
	COMPAT_MACVLAN_MACADDR_FLUSH,
	COMPAT_MACVLAN_MACADDR_SET,
};

#define COMPAT_MACVLAN_FLAG_NOPROMISC	1
#define COMPAT_MACVLAN_FLAG_NODST	2 /* skip dst macvlan if matching src macvlan */



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

static void vxlan_print_explain(void)
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


static void ipvlan_print_explain(void)
{
	bb_simple_error_msg_and_die(
		"Usage: ...ipvlan [ mode MODE ] [ FLAGS ]\n"
		"\n"
		"MODE: l3 | l3s | l2\n"
		"FLAGS: bridge | private | vepa\n"
		"(first values are the defaults if nothing is specified).\n");
}

static void macvlan_explain(void)
{
	bb_simple_error_msg_and_die(
		"Usage: ... macvlan mode MODE [flag MODE_FLAG] MODE_OPTS [bcqueuelen BC_QUEUE_LEN] [bclim BCLIM]\n"
		"\n"
		"MODE: private | vepa | bridge | passthru | source\n"
		"MODE_FLAG: null | nopromisc | nodst\n"
		"MODE_OPTS: for mode \"source\":\n"
		"\tmacaddr { { add | del } <macaddr> | set [ <macaddr> [ <macaddr>  ... ] ] | flush }\n"
		"BC_QUEUE_LEN: Length of the rx queue for broadcast/multicast: [0-4294967295]\n"
		"BCLIM: Threshold for broadcast queueing: 32-bit integer\n");
}

static int get_s32(__s32 *val, const char *arg, int base)
{
	long res;
	char *ptr;

	errno = 0;

	if (!arg || !*arg)
		return -1;
	res = strtol(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr)
		return -1;
	if ((res == LONG_MIN || res == LONG_MAX) && errno == ERANGE)
		return -1;
	if (res > INT32_MAX || res < INT32_MIN)
		return -1;

	*val = res;
	return 0;
}

static int macvlan_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{

	uint32_t mode = 0;
	uint16_t flags = 0;
	uint32_t mac_mode = 0;
	int has_flags = 0;
	unsigned char mac[ETH_ALEN];
	struct rtattr *nmac;
	int arg;

	static const char keywords[] ALIGN1 =
		"mode\0"
		"flag\0"
		"macaddr\0"
		"nopromisc\0"
		"nodst\0"
		"bcqueuelen\0"
		"bclim\0"
		"help\0"
	;

	enum {
		ARG_mode = 0,
		ARG_flag,
		ARG_macaddr,
		ARG_nopromisc,
		ARG_nodst,
		ARG_bcqueuelen,
		ARG_bclim,
		ARG_help,
	};


	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type macvlan");
		if (arg == ARG_mode) {
			NEXT_ARG();

			if (strcmp(*argv, "private") == 0)
				mode = COMPAT_MACVLAN_MODE_PRIVATE;
			else if (strcmp(*argv, "vepa") == 0)
				mode = COMPAT_MACVLAN_MODE_VEPA;
			else if (strcmp(*argv, "bridge") == 0)
				mode = COMPAT_MACVLAN_MODE_BRIDGE;
			else if (strcmp(*argv, "passthru") == 0)
				mode = COMPAT_MACVLAN_MODE_PASSTHRU;
			else if (strcmp(*argv, "source") == 0)
				mode = COMPAT_MACVLAN_MODE_SOURCE;
			else
				bb_error_msg_and_die("macvlan: argument of \"mode\" must be \"private\", \"vepa\", \"bridge\", \"passthru\" or \"source\", not \"%s\"\n", *argv);
			
			addattr16(n, 1024, IFLA_IPVLAN_MODE, mode);
		} else if (arg == ARG_flag) {
			NEXT_ARG();

			if (strcmp(*argv, "nopromisc") == 0)
				flags |= COMPAT_MACVLAN_FLAG_NOPROMISC;
			else if (strcmp(*argv, "nodst") == 0)
				flags |= COMPAT_MACVLAN_FLAG_NODST;
			else if (strcmp(*argv, "null") == 0)
				flags |= 0;
			else
				bb_error_msg_and_die("macvlan: argument of \"flag\" must be \"nopromisc\", \"nodst\" or \"null\", not \"%s\"\n", *argv);
			has_flags = 1;
		} else if (arg == ARG_macaddr) {
			NEXT_ARG();

			if (strcmp(*argv, "add") == 0) {
				mac_mode = COMPAT_MACVLAN_MACADDR_ADD;
			} else if (strcmp(*argv, "del") == 0) {
				mac_mode = COMPAT_MACVLAN_MACADDR_DEL;
			} else if (strcmp(*argv, "set") == 0) {
				mac_mode = COMPAT_MACVLAN_MACADDR_SET;
			} else if (strcmp(*argv, "flush") == 0) {
				mac_mode = COMPAT_MACVLAN_MACADDR_FLUSH;
			} else {
				macvlan_explain();
			}

			addattr32(n, 1024, IFLA_COMPAT_MACVLAN_MACADDR_MODE, mac_mode);

			if (mac_mode == COMPAT_MACVLAN_MACADDR_ADD ||
			    mac_mode == COMPAT_MACVLAN_MACADDR_DEL) {
				NEXT_ARG();

				if (ll_addr_a2n(mac, sizeof(mac),
						*argv) != ETH_ALEN)
					return -1;

				addattr_l(n, 1024, IFLA_COMPAT_MACVLAN_MACADDR, &mac,
					  ETH_ALEN);
			}

			if (mac_mode == COMPAT_MACVLAN_MACADDR_SET) {
				nmac = addattr_nest(n, 1024,
						    IFLA_COMPAT_MACVLAN_MACADDR_DATA);
				while (*(argv+1)) {
					NEXT_ARG();

					if (ll_addr_a2n(mac, sizeof(mac),
							*argv) != ETH_ALEN) {
						PREV_ARG();
						break;
					}

					addattr_l(n, 1024, IFLA_COMPAT_MACVLAN_MACADDR,
						  &mac, ETH_ALEN);
				}
				addattr_nest_end(n, nmac);
			}
		} else if (arg == ARG_nopromisc) {
			flags |= COMPAT_MACVLAN_FLAG_NOPROMISC;
			has_flags = 1;
		} else if (arg == ARG_nodst) {
			flags |= COMPAT_MACVLAN_FLAG_NODST;
			has_flags = 1;
		} else if (arg == ARG_bcqueuelen) {
			uint32_t bc_queue_len;
			NEXT_ARG();

			bc_queue_len = get_u32(*argv, "bcqueuelen");
			
			addattr32(n, 1024, IFLA_COMPAT_MACVLAN_BC_QUEUE_LEN, bc_queue_len);
		} else if (arg == ARG_bclim) {
			int32_t bclim;
			NEXT_ARG();

			if (get_s32(&bclim, *argv, 0)) {
				bb_error_msg_and_die(
	    			"macvlan: illegal value for \"bclim\": \"%s\"\n", *argv);
			}
			addattr_l(n, 1024, IFLA_COMPAT_MACVLAN_BC_CUTOFF,
				  &bclim, sizeof(bclim));
		} else if (arg == ARG_help) {
			macvlan_explain();
		} else {
			bb_error_msg_and_die("macvlan: unknown command \"%s\"?", *argv);
		}
		argv++;
	}
	addattr16(n, 1024, IFLA_IPVLAN_FLAGS, flags);
	return 0;
}


static int ipvlan_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{

	uint16_t flags = 0;
	bool mflag_given = false;
	int arg;

	static const char keywords[] ALIGN1 =
		"mode\0"
		"private\0"
		"vepa\0"
		"bridge\0"
		"help\0"
	;

	enum {
		ARG_mode = 0,
		ARG_private,
		ARG_vepa,
		ARG_bridge,
		ARG_help,
	};


	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type ipvlan");
		if (arg == ARG_mode) {
			uint16_t mode = 0;
			NEXT_ARG();
			if (strcmp(*argv, "l2") == 0)
				mode = IPVLAN_MODE_L2;
			else if (strcmp(*argv, "l3") == 0)
				mode = IPVLAN_MODE_L3;
			else if (strcmp(*argv, "l3s") == 0)
				mode = IPVLAN_MODE_L3S;
			else {
				bb_simple_error_msg_and_die("ipvlan: argument of \"mode\" must be either \"l2\", \"l3\" or \"l3s\"\n");
			}
			addattr16(n, 1024, IFLA_IPVLAN_MODE, mode);
		} else if (arg == ARG_private && !mflag_given) {
			flags |= IPVLAN_F_PRIVATE;
			mflag_given = true;
		} else if (arg == ARG_vepa && !mflag_given) {
			flags |= IPVLAN_F_VEPA;
			mflag_given = true;
		} else if (arg == ARG_bridge && !mflag_given) {
			mflag_given = true;
		} else if (arg == ARG_help) {
			ipvlan_print_explain();
		} else {
			bb_error_msg_and_die("ipvlan: unknown command \"%s\"?", *argv);
		}
		argv++;
	}
	addattr16(n, 1024, IFLA_IPVLAN_FLAGS, flags);
	return 0;
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
			vxlan_print_explain();
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
#ifndef IFLA_BR_MAX
/* Bridge section */

enum {
	IFLA_BR_UNSPEC,
	IFLA_BR_FORWARD_DELAY,
	IFLA_BR_HELLO_TIME,
	IFLA_BR_MAX_AGE,
	IFLA_BR_AGEING_TIME,
	IFLA_BR_STP_STATE,
	IFLA_BR_PRIORITY,
	IFLA_BR_VLAN_FILTERING,
	IFLA_BR_VLAN_PROTOCOL,
	IFLA_BR_GROUP_FWD_MASK,
	IFLA_BR_ROOT_ID,
	IFLA_BR_BRIDGE_ID,
	IFLA_BR_ROOT_PORT,
	IFLA_BR_ROOT_PATH_COST,
	IFLA_BR_TOPOLOGY_CHANGE,
	IFLA_BR_TOPOLOGY_CHANGE_DETECTED,
	IFLA_BR_HELLO_TIMER,
	IFLA_BR_TCN_TIMER,
	IFLA_BR_TOPOLOGY_CHANGE_TIMER,
	IFLA_BR_GC_TIMER,
	IFLA_BR_GROUP_ADDR,
	IFLA_BR_FDB_FLUSH,
	IFLA_BR_MCAST_ROUTER,
	IFLA_BR_MCAST_SNOOPING,
	IFLA_BR_MCAST_QUERY_USE_IFADDR,
	IFLA_BR_MCAST_QUERIER,
	IFLA_BR_MCAST_HASH_ELASTICITY,
	IFLA_BR_MCAST_HASH_MAX,
	IFLA_BR_MCAST_LAST_MEMBER_CNT,
	IFLA_BR_MCAST_STARTUP_QUERY_CNT,
	IFLA_BR_MCAST_LAST_MEMBER_INTVL,
	IFLA_BR_MCAST_MEMBERSHIP_INTVL,
	IFLA_BR_MCAST_QUERIER_INTVL,
	IFLA_BR_MCAST_QUERY_INTVL,
	IFLA_BR_MCAST_QUERY_RESPONSE_INTVL,
	IFLA_BR_MCAST_STARTUP_QUERY_INTVL,
	IFLA_BR_NF_CALL_IPTABLES,
	IFLA_BR_NF_CALL_IP6TABLES,
	IFLA_BR_NF_CALL_ARPTABLES,
	IFLA_BR_VLAN_DEFAULT_PVID,
	IFLA_BR_PAD,
	IFLA_BR_VLAN_STATS_ENABLED,
	IFLA_BR_MCAST_STATS_ENABLED,
	IFLA_BR_MCAST_IGMP_VERSION,
	IFLA_BR_MCAST_MLD_VERSION,
	IFLA_BR_VLAN_STATS_PER_PORT,
	IFLA_BR_MULTI_BOOLOPT,
	IFLA_BR_MCAST_QUERIER_STATE,
	IFLA_BR_FDB_N_LEARNED,
	IFLA_BR_FDB_MAX_LEARNED,
	__IFLA_BR_MAX,
};

#define IFLA_BR_MAX	(__IFLA_BR_MAX - 1)

struct ifla_bridge_id {
	__u8	prio[2];
	__u8	addr[6]; /* ETH_ALEN */
};

enum {
	BRIDGE_MODE_UNSPEC,
	BRIDGE_MODE_HAIRPIN,
};

enum {
	IFLA_BRPORT_UNSPEC,
	IFLA_BRPORT_STATE,	/* Spanning tree state     */
	IFLA_BRPORT_PRIORITY,	/* "             priority  */
	IFLA_BRPORT_COST,	/* "             cost      */
	IFLA_BRPORT_MODE,	/* mode (hairpin)          */
	IFLA_BRPORT_GUARD,	/* bpdu guard              */
	IFLA_BRPORT_PROTECT,	/* root port protection    */
	IFLA_BRPORT_FAST_LEAVE,	/* multicast fast leave    */
	IFLA_BRPORT_LEARNING,	/* mac learning */
	IFLA_BRPORT_UNICAST_FLOOD, /* flood unicast traffic */
	IFLA_BRPORT_PROXYARP,	/* proxy ARP */
	IFLA_BRPORT_LEARNING_SYNC, /* mac learning sync from device */
	IFLA_BRPORT_PROXYARP_WIFI, /* proxy ARP for Wi-Fi */
	IFLA_BRPORT_ROOT_ID,	/* designated root */
	IFLA_BRPORT_BRIDGE_ID,	/* designated bridge */
	IFLA_BRPORT_DESIGNATED_PORT,
	IFLA_BRPORT_DESIGNATED_COST,
	IFLA_BRPORT_ID,
	IFLA_BRPORT_NO,
	IFLA_BRPORT_TOPOLOGY_CHANGE_ACK,
	IFLA_BRPORT_CONFIG_PENDING,
	IFLA_BRPORT_MESSAGE_AGE_TIMER,
	IFLA_BRPORT_FORWARD_DELAY_TIMER,
	IFLA_BRPORT_HOLD_TIMER,
	IFLA_BRPORT_FLUSH,
	IFLA_BRPORT_MULTICAST_ROUTER,
	IFLA_BRPORT_PAD,
	IFLA_BRPORT_MCAST_FLOOD,
	IFLA_BRPORT_MCAST_TO_UCAST,
	IFLA_BRPORT_VLAN_TUNNEL,
	IFLA_BRPORT_BCAST_FLOOD,
	IFLA_BRPORT_GROUP_FWD_MASK,
	IFLA_BRPORT_NEIGH_SUPPRESS,
	IFLA_BRPORT_ISOLATED,
	IFLA_BRPORT_BACKUP_PORT,
	IFLA_BRPORT_MRP_RING_OPEN,
	IFLA_BRPORT_MRP_IN_OPEN,
	IFLA_BRPORT_MCAST_EHT_HOSTS_LIMIT,
	IFLA_BRPORT_MCAST_EHT_HOSTS_CNT,
	IFLA_BRPORT_LOCKED,
	IFLA_BRPORT_MAB,
	IFLA_BRPORT_MCAST_N_GROUPS,
	IFLA_BRPORT_MCAST_MAX_GROUPS,
	IFLA_BRPORT_NEIGH_VLAN_SUPPRESS,
	__IFLA_BRPORT_MAX
};
#define IFLA_BRPORT_MAX (__IFLA_BRPORT_MAX - 1)

/* bridge boolean options
 * BR_BOOLOPT_NO_LL_LEARN - disable learning from link-local packets
 * BR_BOOLOPT_MCAST_VLAN_SNOOPING - control vlan multicast snooping
 *
 * IMPORTANT: if adding a new option do not forget to handle
 *            it in br_boolopt_toggle/get and bridge sysfs
 */
enum br_boolopt_id {
	BR_BOOLOPT_NO_LL_LEARN,
	BR_BOOLOPT_MCAST_VLAN_SNOOPING,
	BR_BOOLOPT_MST_ENABLE,
	BR_BOOLOPT_MAX
};

/* struct br_boolopt_multi - change multiple bridge boolean options
 *
 * @optval: new option values (bit per option)
 * @optmask: options to change (bit per option)
 */
struct br_boolopt_multi {
	__u32 optval;
	__u32 optmask;
};

enum {
	BRIDGE_QUERIER_UNSPEC,
	BRIDGE_QUERIER_IP_ADDRESS,
	BRIDGE_QUERIER_IP_PORT,
	BRIDGE_QUERIER_IP_OTHER_TIMER,
	BRIDGE_QUERIER_PAD,
	BRIDGE_QUERIER_IPV6_ADDRESS,
	BRIDGE_QUERIER_IPV6_PORT,
	BRIDGE_QUERIER_IPV6_OTHER_TIMER,
	__BRIDGE_QUERIER_MAX
};
#define BRIDGE_QUERIER_MAX (__BRIDGE_QUERIER_MAX - 1)

#endif

static void bridge_explain(void)
{
	bb_simple_error_msg_and_die(
		"Usage: ... bridge [ fdb_flush ]\n"
		"		  [ forward_delay FORWARD_DELAY ]\n"
		"		  [ hello_time HELLO_TIME ]\n"
		"		  [ max_age MAX_AGE ]\n"
		"		  [ ageing_time AGEING_TIME ]\n"
		"		  [ stp_state STP_STATE ]\n"
		"		  [ mst_enabled MST_ENABLED ]\n"
		"		  [ priority PRIORITY ]\n"
		"		  [ group_fwd_mask MASK ]\n"
		"		  [ group_address ADDRESS ]\n"
		"		  [ no_linklocal_learn NO_LINKLOCAL_LEARN ]\n"
		"		  [ fdb_max_learned FDB_MAX_LEARNED ]\n"
		"		  [ vlan_filtering VLAN_FILTERING ]\n"
		"		  [ vlan_protocol VLAN_PROTOCOL ]\n"
		"		  [ vlan_default_pvid VLAN_DEFAULT_PVID ]\n"
		"		  [ vlan_stats_enabled VLAN_STATS_ENABLED ]\n"
		"		  [ vlan_stats_per_port VLAN_STATS_PER_PORT ]\n"
		"		  [ mcast_snooping MULTICAST_SNOOPING ]\n"
		"		  [ mcast_vlan_snooping MULTICAST_VLAN_SNOOPING ]\n"
		"		  [ mcast_router MULTICAST_ROUTER ]\n"
		"		  [ mcast_query_use_ifaddr MCAST_QUERY_USE_IFADDR ]\n"
		"		  [ mcast_querier MULTICAST_QUERIER ]\n"
		"		  [ mcast_hash_elasticity HASH_ELASTICITY ]\n"
		"		  [ mcast_hash_max HASH_MAX ]\n"
		"		  [ mcast_last_member_count LAST_MEMBER_COUNT ]\n"
		"		  [ mcast_startup_query_count STARTUP_QUERY_COUNT ]\n"
		"		  [ mcast_last_member_interval LAST_MEMBER_INTERVAL ]\n"
		"		  [ mcast_membership_interval MEMBERSHIP_INTERVAL ]\n"
		"		  [ mcast_querier_interval QUERIER_INTERVAL ]\n"
		"		  [ mcast_query_interval QUERY_INTERVAL ]\n"
		"		  [ mcast_query_response_interval QUERY_RESPONSE_INTERVAL ]\n"
		"		  [ mcast_startup_query_interval STARTUP_QUERY_INTERVAL ]\n"
		"		  [ mcast_stats_enabled MCAST_STATS_ENABLED ]\n"
		"		  [ mcast_igmp_version IGMP_VERSION ]\n"
		"		  [ mcast_mld_version MLD_VERSION ]\n"
		"		  [ nf_call_iptables NF_CALL_IPTABLES ]\n"
		"		  [ nf_call_ip6tables NF_CALL_IP6TABLES ]\n"
		"		  [ nf_call_arptables NF_CALL_ARPTABLES ]\n"
		"\n"
		"Where: VLAN_PROTOCOL := { 802.1Q | 802.1ad }\n"
	);
}

static void bridge_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size) {

	int arg;
	struct br_boolopt_multi bm = {};
	uint32_t val;

	static const char keywords[] ALIGN1 =
		"forward_delay\0"
		"hello_time\0"
		"max_age\0"
		"ageing_time\0"
		"stp_state\0"
		"mst_enabled\0"
		"priority\0"
		"vlan_filtering\0"
		"vlan_protocol\0"
		"group_fwd_mask\0"
		"group_address\0"
		"no_linklocal_learn\0"
		"fdb_max_learned\0"
		"fdb_flush\0"
		"vlan_default_pvid\0"
		"vlan_stats_enabled\0"
		"vlan_stats_per_port\0"
		"mcast_router\0"
		"mcast_snooping\0"
		"mcast_vlan_snooping\0"
		"mcast_query_use_ifaddr\0"
		"mcast_querier\0"
		"mcast_hash_elasticity\0"
		"mcast_hash_max\0"
		"mcast_last_member_count\0"
		"mcast_startup_query_count\0"
		"mcast_last_member_interval\0"
		"mcast_membership_interval\0"
		"mcast_querier_interval\0"
		"mcast_query_interval\0"
		"mcast_query_response_interval\0"
		"mcast_startup_query_interval\0"
		"mcast_stats_enabled\0"
		"mcast_igmp_version\0"
		"mcast_mld_version\0"
		"nf_call_iptables\0"
		"nf_call_ip6tables\0"
		"nf_call_arptables\0"
		"help\0"
	;

	enum {
		ARG_forward_delay = 0,
		ARG_hello_time,
		ARG_max_age,
		ARG_ageing_time,
		ARG_stp_state,
		ARG_mst_enabled,
		ARG_priority,
		ARG_vlan_filtering,
		ARG_vlan_protocol,
		ARG_group_fwd_mask,
		ARG_group_address,
		ARG_no_linklocal_learn,
		ARG_fdb_max_learned,
		ARG_fdb_flush,
		ARG_vlan_default_pvid,
		ARG_vlan_stats_enabled,
		ARG_vlan_stats_per_port,
		ARG_mcast_router,
		ARG_mcast_snooping,
		ARG_mcast_vlan_snooping,
		ARG_mcast_query_use_ifaddr,
		ARG_mcast_querier,
		ARG_mcast_hash_elasticity,
		ARG_mcast_hash_max,
		ARG_mcast_last_member_count,
		ARG_mcast_startup_query_count,
		ARG_mcast_last_member_interval,
		ARG_mcast_membership_interval,
		ARG_mcast_querier_interval,
		ARG_mcast_query_interval,
		ARG_mcast_query_response_interval,
		ARG_mcast_startup_query_interval,
		ARG_mcast_stats_enabled,
		ARG_mcast_igmp_version,
		ARG_mcast_mld_version,
		ARG_nf_call_iptables,
		ARG_nf_call_ip6tables,
		ARG_nf_call_arptables,
		ARG_help,
	};

	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type bridge");
		if (arg == ARG_forward_delay) {
			NEXT_ARG();
			val = get_u32(*argv, "invalid forward_delay");

			addattr32(n, 1024, IFLA_BR_FORWARD_DELAY, val);
		} else if (arg == ARG_hello_time) {
			NEXT_ARG();
			val = get_u32(*argv, "invalid hello_time");

			addattr32(n, 1024, IFLA_BR_HELLO_TIME, val);
		} else if (arg == ARG_max_age) {
			NEXT_ARG();
			
			val = get_u32(*argv, "invalid max_age");

			addattr32(n, 1024, IFLA_BR_MAX_AGE, val);
		} else if (arg == ARG_ageing_time) {
			NEXT_ARG();
			
			val = get_u32(*argv, "invalid ageing_time");

			addattr32(n, 1024, IFLA_BR_AGEING_TIME, val);
		} else if (arg == ARG_stp_state) {
			NEXT_ARG();
			val = get_u32(*argv, "invalid stp_state");

			addattr32(n, 1024, IFLA_BR_STP_STATE, val);
		} else if (arg == ARG_mst_enabled) {
			uint32_t mst_bit = 1 << BR_BOOLOPT_MST_ENABLE;
			uint8_t mst_enabled;

			NEXT_ARG();
			mst_enabled = get_u8(*argv, "invalid mst_enabled");
			bm.optmask |= mst_bit;
			if (mst_enabled)
				bm.optval |= mst_bit;
			else
				bm.optval &= ~mst_bit;
		} else if (arg == ARG_fdb_max_learned) {
			uint32_t fdb_max_learned;

			NEXT_ARG();
			fdb_max_learned = get_u32(*argv, "invalid fdb_max_learned");

			addattr32(n, 1024, IFLA_BR_FDB_MAX_LEARNED, fdb_max_learned);
		} else if (arg == ARG_priority) {
			uint16_t prio;

			NEXT_ARG();
			prio = get_u16(*argv, "invalid priority");

			addattr16(n, 1024, IFLA_BR_PRIORITY, prio);
		} else if (arg == ARG_vlan_filtering) {
			uint8_t vlan_filter;

			NEXT_ARG();
			vlan_filter = get_u8(*argv, "invalid vlan_filtering");

			addattr8(n, 1024, IFLA_BR_VLAN_FILTERING, vlan_filter);
		} else if (arg == ARG_vlan_protocol) {
			uint16_t vlan_proto;

			NEXT_ARG();
			if (ll_proto_a2n(&vlan_proto, *argv))
				invarg("invalid vlan_protocol", *argv);

			addattr16(n, 1024, IFLA_BR_VLAN_PROTOCOL, vlan_proto);
		} else if (arg == ARG_group_fwd_mask) {
			uint16_t fwd_mask;

			NEXT_ARG();
			
				fwd_mask = get_u16(*argv,"invalid group_fwd_mask");

			addattr16(n, 1024, IFLA_BR_GROUP_FWD_MASK, fwd_mask);
		} else if (arg == ARG_group_address) {
			unsigned char llabuf[32];
			int len;

			NEXT_ARG();
			len = ll_addr_a2n(llabuf, sizeof(llabuf), *argv);
			if (len < 0)
				return;
			addattr_l(n, 1024, IFLA_BR_GROUP_ADDR, llabuf, len);
		} else if (arg == ARG_no_linklocal_learn) {
			uint32_t no_ll_learn_bit = 1 << BR_BOOLOPT_NO_LL_LEARN;
			uint8_t no_ll_learn;

			NEXT_ARG();
				no_ll_learn = get_u8(*argv,"invalid no_linklocal_learn");
			bm.optmask |= 1 << BR_BOOLOPT_NO_LL_LEARN;
			if (no_ll_learn)
				bm.optval |= no_ll_learn_bit;
			else
				bm.optval &= ~no_ll_learn_bit;
		} else if (arg == ARG_fdb_flush) {
			addattr(n, 1024, IFLA_BR_FDB_FLUSH);
		} else if (arg == ARG_vlan_default_pvid) {
			uint16_t default_pvid;

			NEXT_ARG();
				default_pvid = get_u16(*argv,"invalid vlan_default_pvid");

			addattr16(n, 1024, IFLA_BR_VLAN_DEFAULT_PVID,
				  default_pvid);
		} else if (arg == ARG_vlan_stats_enabled) {
			uint8_t vlan_stats_enabled;

			NEXT_ARG();
			
			vlan_stats_enabled = get_u8(*argv,"invalid vlan_stats_enabled");
			addattr8(n, 1024, IFLA_BR_VLAN_STATS_ENABLED,
				  vlan_stats_enabled);
		} else if (arg == ARG_vlan_stats_per_port) {
			uint8_t vlan_stats_per_port;

			NEXT_ARG();
			val = get_u8(*argv,"invalid vlan_stats_per_port");
			addattr8(n, 1024, IFLA_BR_VLAN_STATS_PER_PORT,
				 vlan_stats_per_port);
		} else if (arg == ARG_mcast_router) {
			uint8_t mcast_router;

			NEXT_ARG();
			mcast_router = get_u8(*argv,"invalid mcast_router");

			addattr8(n, 1024, IFLA_BR_MCAST_ROUTER, mcast_router);
		} else if (arg == ARG_mcast_snooping) {
			uint8_t mcast_snoop;

			NEXT_ARG();
			mcast_snoop = get_u8(*argv,"invalid mcast_snooping");

			addattr8(n, 1024, IFLA_BR_MCAST_SNOOPING, mcast_snoop);
		} else if (arg == ARG_mcast_vlan_snooping) {
			uint32_t mcvl_bit = 1 << BR_BOOLOPT_MCAST_VLAN_SNOOPING;
			uint8_t mcast_vlan_snooping;

			NEXT_ARG();
			mcast_vlan_snooping = get_u8(*argv,"invalid mcast_vlan_snooping");
			bm.optmask |= 1 << BR_BOOLOPT_MCAST_VLAN_SNOOPING;
			if (mcast_vlan_snooping)
				bm.optval |= mcvl_bit;
			else
				bm.optval &= ~mcvl_bit;
		} else if (arg == ARG_mcast_query_use_ifaddr) {
			uint8_t mcast_qui;

			NEXT_ARG();
			mcast_qui = get_u8(*argv,"invalid mcast_query_use_ifaddr");

			addattr8(n, 1024, IFLA_BR_MCAST_QUERY_USE_IFADDR,
				 mcast_qui);
		} else if (arg == ARG_mcast_querier) {
			uint8_t mcast_querier;

			NEXT_ARG();
			mcast_querier = get_u8(*argv,"invalid mcast_querier");

			addattr8(n, 1024, IFLA_BR_MCAST_QUERIER, mcast_querier);
		} else if (arg == ARG_mcast_hash_elasticity) {
			uint32_t mcast_hash_el;

			NEXT_ARG();
			mcast_hash_el = get_u32(*argv,"invalid mcast_hash_elasticity");

			addattr32(n, 1024, IFLA_BR_MCAST_HASH_ELASTICITY,
				  mcast_hash_el);
		} else if (arg == ARG_mcast_hash_max) {
			uint32_t mcast_hash_max;

			NEXT_ARG();
			mcast_hash_max = get_u32(*argv,"invalid mcast_hash_max");

			addattr32(n, 1024, IFLA_BR_MCAST_HASH_MAX,
				  mcast_hash_max);
		} else if (arg == ARG_mcast_last_member_count) {
			uint32_t mcast_lmc;

			NEXT_ARG();
			mcast_lmc = get_u32(*argv,"invalid mcast_last_member_count");

			addattr32(n, 1024, IFLA_BR_MCAST_LAST_MEMBER_CNT,
				  mcast_lmc);
		} else if (arg == ARG_mcast_startup_query_count) {
			uint32_t mcast_sqc;

			NEXT_ARG();
			mcast_sqc = get_u32(*argv,"invalid mcast_startup_query_count");

			addattr32(n, 1024, IFLA_BR_MCAST_STARTUP_QUERY_CNT,
				  mcast_sqc);
		} else if (arg == ARG_mcast_last_member_interval) {
			__u64 mcast_last_member_intvl;

			NEXT_ARG();
			mcast_last_member_intvl = get_u64(*argv,"invalid mcast_last_member_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_LAST_MEMBER_INTVL,
				  mcast_last_member_intvl);
		} else if (arg == ARG_mcast_membership_interval) {
			__u64 mcast_membership_intvl;

			NEXT_ARG();
			mcast_membership_intvl = get_u64(*argv,"invalid mcast_membership_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_MEMBERSHIP_INTVL,
				  mcast_membership_intvl);
		} else if (arg == ARG_mcast_querier_interval) {
			__u64 mcast_querier_intvl;

			NEXT_ARG();
			mcast_querier_intvl = get_u64(*argv,"invalid mcast_querier_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_QUERIER_INTVL,
				  mcast_querier_intvl);
		} else if (arg == ARG_mcast_query_interval) {
			__u64 mcast_query_intvl;

			NEXT_ARG();
			mcast_query_intvl = get_u64(*argv,"invalid mcast_query_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_QUERY_INTVL,
				  mcast_query_intvl);
		} else if (arg == ARG_mcast_query_response_interval) {
			__u64 mcast_query_resp_intvl;

			NEXT_ARG();
			mcast_query_resp_intvl = get_u64(*argv,"invalid mcast_query_response_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_QUERY_RESPONSE_INTVL,
				  mcast_query_resp_intvl);
		} else if (arg == ARG_mcast_startup_query_interval) {
			__u64 mcast_startup_query_intvl;

			NEXT_ARG();
			mcast_startup_query_intvl = get_u64(*argv,"invalid mcast_startup_query_interval");

			addattr64(n, 1024, IFLA_BR_MCAST_STARTUP_QUERY_INTVL,
				  mcast_startup_query_intvl);
		} else if (arg == ARG_mcast_stats_enabled) {
			uint8_t mcast_stats_enabled;

			NEXT_ARG();
			mcast_stats_enabled = get_u8(*argv,"invalid mcast_stats_enabled");
			addattr8(n, 1024, IFLA_BR_MCAST_STATS_ENABLED,
				  mcast_stats_enabled);
		} else if (arg == ARG_mcast_igmp_version) {
			uint8_t igmp_version;

			NEXT_ARG();
			igmp_version = get_u8(*argv,"invalid mcast_igmp_version");
			addattr8(n, 1024, IFLA_BR_MCAST_IGMP_VERSION,
				  igmp_version);
		} else if (arg == ARG_mcast_mld_version) {
			uint8_t mld_version;

			NEXT_ARG();
			mld_version = get_u8(*argv,"invalid mcast_mld_version");
			addattr8(n, 1024, IFLA_BR_MCAST_MLD_VERSION,
				  mld_version);
		} else if (arg == ARG_nf_call_iptables) {
			uint8_t nf_call_ipt;

			NEXT_ARG();
			nf_call_ipt = get_u8(*argv,"invalid nf_call_iptables");

			addattr8(n, 1024, IFLA_BR_NF_CALL_IPTABLES,
				 nf_call_ipt);
		} else if (arg == ARG_nf_call_ip6tables) {
			uint8_t nf_call_ip6t;

			NEXT_ARG();
			nf_call_ip6t = get_u8(*argv,"invalid nf_call_ip6tables");

			addattr8(n, 1024, IFLA_BR_NF_CALL_IP6TABLES,
				 nf_call_ip6t);
		} else if (arg == ARG_nf_call_arptables) {
			uint8_t nf_call_arpt;

			NEXT_ARG();
			nf_call_arpt = get_u8(*argv,"invalid nf_call_arptables");

			addattr8(n, 1024, IFLA_BR_NF_CALL_ARPTABLES,
				 nf_call_arpt);
		} else if (arg == ARG_help) {
			bridge_explain();
		} else {
			bb_error_msg_and_die("bridge: unknown command \"%s\"?", *argv);
		}
		argv++;
	}

	if (bm.optmask)
		addattr_l(n, 1024, IFLA_BR_MULTI_BOOLOPT,
			  &bm, sizeof(bm));
	return;
}

static void bridge_slave_explain(void)
{
	bb_simple_error_msg_and_die(
		"Usage: ... bridge_slave [ fdb_flush ]\n"
		"			[ state STATE ]\n"
		"			[ priority PRIO ]\n"
		"			[ cost COST ]\n"
		"			[ guard {on | off} ]\n"
		"			[ hairpin {on | off} ]\n"
		"			[ fastleave {on | off} ]\n"
		"			[ root_block {on | off} ]\n"
		"			[ learning {on | off} ]\n"
		"			[ flood {on | off} ]\n"
		"			[ proxy_arp {on | off} ]\n"
		"			[ proxy_arp_wifi {on | off} ]\n"
		"			[ mcast_router MULTICAST_ROUTER ]\n"
		"			[ mcast_fast_leave {on | off} ]\n"
		"			[ mcast_flood {on | off} ]\n"
		"			[ bcast_flood {on | off} ]\n"
		"			[ mcast_to_unicast {on | off} ]\n"
		"			[ group_fwd_mask MASK ]\n"
		"			[ neigh_suppress {on | off} ]\n"
		"			[ neigh_vlan_suppress {on | off} ]\n"
		"			[ vlan_tunnel {on | off} ]\n"
		"			[ isolated {on | off} ]\n"
		"			[ locked {on | off} ]\n"
		"			[ mab {on | off} ]\n"
		"			[ backup_port DEVICE ] [ nobackup_port ]\n"
	);
}

static bool matches(const char *prefix, const char *string)
{
	if (!*prefix)
		return true;
	while (*string && *prefix == *string) {
		prefix++;
		string++;
	}

	return !!*prefix;
}

static int parse_one_of(const char *msg, const char *realval, const char * const *list,
		 size_t len, int *p_err)
{
	int i;

	for (i = 0; i < len; i++) {
		if (list[i] && matches(realval, list[i]) == 0) {
			*p_err = 0;
			return i;
		}
	}


	fprintf(stderr, "Error: argument of \"%s\" must be one of ", msg);
	for (i = 0; i < len; i++)
		if (list[i])
			fprintf(stderr, "\"%s\", ", list[i]);
	fprintf(stderr, "not \"%s\"\n", realval);
	*p_err = -EINVAL;
	return 0;
}

static bool parse_on_off(const char *msg, const char *realval, int *p_err)
{
	static const char * const values_on_off[] = { "off", "on" };

	return parse_one_of(msg, realval, values_on_off, ARRAY_SIZE(values_on_off), p_err);
}


static void bridge_slave_parse_on_off(const char *arg_name, const char *arg_val,
				      struct nlmsghdr *n, int type)
{
	int ret;
	__u8 val = parse_on_off(arg_name, arg_val, &ret);

	if (ret)
		exit(1);
	addattr8(n, 1024, type, val);
}

static void bridge_slave_parse_opt(char **argv, struct nlmsghdr *n, unsigned int size)
{
	uint8_t state;
	uint16_t priority;
	uint32_t cost;
	int arg;

	static const char keywords[] ALIGN1 =
		"fdb_flush\0"
		"state\0"
		"priority\0"
		"cost\0"
		"hairpin\0"
		"guard\0"
		"root_block\0"
		"fastleave\0"
		"learning\0"
		"flood\0"
		"mcast_flood\0"
		"bcast_flood\0"
		"mcast_to_unicast\0"
		"proxy_arp\0"
		"proxy_arp_wifi\0"
		"mcast_router\0"
		"mcast_fast_leave\0"
		"neigh_suppress\0"
		"neigh_vlan_suppress\0"
		"group_fwd_mask\0"
		"vlan_tunnel\0"
		"isolated\0"
		"locked\0"
		"mab\0"
		"backup_port\0"
		"nobackup_port\0"
		"help\0"
	;

	enum {
		ARG_fdb_flush = 0,
		ARG_state,
		ARG_priority,
		ARG_cost,
		ARG_hairpin,
		ARG_guard,
		ARG_root_block,
		ARG_fastleave,
		ARG_learning,
		ARG_flood,
		ARG_mcast_flood,
		ARG_bcast_flood,
		ARG_mcast_to_unicast,
		ARG_proxy_arp,
		ARG_proxy_arp_wifi,
		ARG_mcast_router,
		ARG_mcast_fast_leave,
		ARG_neigh_suppress,
		ARG_neigh_vlan_suppress,
		ARG_group_fwd_mask,
		ARG_vlan_tunnel,
		ARG_isolated,
		ARG_locked,
		ARG_mab,
		ARG_backup_port,
		ARG_nobackup_port,
		ARG_help,
	};

	while (*argv) {
		arg = index_in_substrings(keywords, *argv);
		if (arg < 0)
			invarg_1_to_2(*argv, "type bridge_slave");

		if (arg == ARG_fdb_flush) {
			addattr(n, 1024, IFLA_BRPORT_FLUSH);
		} else if (arg == ARG_state) {
			NEXT_ARG();
			state = get_u8(*argv,"state is invalid");
			addattr8(n, 1024, IFLA_BRPORT_STATE, state);
		} else if (arg == ARG_priority) {
			NEXT_ARG();
			priority = get_u16(*argv, "priority is invalid");
			addattr16(n, 1024, IFLA_BRPORT_PRIORITY, priority);
		} else if (arg == ARG_cost) {
			NEXT_ARG();
			cost = get_u32(*argv, "cost is invalid");
			addattr32(n, 1024, IFLA_BRPORT_COST, cost);
		} else if (arg == ARG_hairpin) {
			NEXT_ARG();
			bridge_slave_parse_on_off("hairpin", *argv, n,
						  IFLA_BRPORT_MODE);
		} else if (arg == ARG_guard) {
			NEXT_ARG();
			bridge_slave_parse_on_off("guard", *argv, n,
						  IFLA_BRPORT_GUARD);
		} else if (arg == ARG_root_block) {
			NEXT_ARG();
			bridge_slave_parse_on_off("root_block", *argv, n,
						  IFLA_BRPORT_PROTECT);
		} else if (arg == ARG_fastleave) {
			NEXT_ARG();
			bridge_slave_parse_on_off("fastleave", *argv, n,
						  IFLA_BRPORT_FAST_LEAVE);
		} else if (arg == ARG_learning) {
			NEXT_ARG();
			bridge_slave_parse_on_off("learning", *argv, n,
						  IFLA_BRPORT_LEARNING);
		} else if (arg == ARG_flood) {
			NEXT_ARG();
			bridge_slave_parse_on_off("flood", *argv, n,
						  IFLA_BRPORT_UNICAST_FLOOD);
		} else if (arg == ARG_mcast_flood) {
			NEXT_ARG();
			bridge_slave_parse_on_off("mcast_flood", *argv, n,
						  IFLA_BRPORT_MCAST_FLOOD);
		} else if (arg == ARG_bcast_flood) {
			NEXT_ARG();
			bridge_slave_parse_on_off("bcast_flood", *argv, n,
						  IFLA_BRPORT_BCAST_FLOOD);
		} else if (arg == ARG_mcast_to_unicast) {
			NEXT_ARG();
			bridge_slave_parse_on_off("mcast_to_unicast", *argv, n,
						  IFLA_BRPORT_MCAST_TO_UCAST);
		} else if (arg == ARG_proxy_arp) {
			NEXT_ARG();
			bridge_slave_parse_on_off("proxy_arp", *argv, n,
						  IFLA_BRPORT_PROXYARP);
		} else if (arg == ARG_proxy_arp_wifi) {
			NEXT_ARG();
			bridge_slave_parse_on_off("proxy_arp_wifi", *argv, n,
						  IFLA_BRPORT_PROXYARP_WIFI);
		} else if (arg == ARG_mcast_router) {
			uint8_t mcast_router;

			NEXT_ARG();
			mcast_router = get_u8(*argv, "invalid mcast_router");
			addattr8(n, 1024, IFLA_BRPORT_MULTICAST_ROUTER,
				 mcast_router);
		} else if (arg == ARG_mcast_fast_leave) {
			NEXT_ARG();
			bridge_slave_parse_on_off("mcast_fast_leave", *argv, n,
						  IFLA_BRPORT_FAST_LEAVE);
		} else if (arg == ARG_neigh_suppress) {
			NEXT_ARG();
			bridge_slave_parse_on_off("neigh_suppress", *argv, n,
						  IFLA_BRPORT_NEIGH_SUPPRESS);
		} else if (arg == ARG_neigh_vlan_suppress) {
			NEXT_ARG();
			bridge_slave_parse_on_off("neigh_vlan_suppress", *argv,
						  n, IFLA_BRPORT_NEIGH_VLAN_SUPPRESS);
		} else if (arg == ARG_group_fwd_mask) {
			uint16_t mask;

			NEXT_ARG();
			mask = get_u16(*argv, "invalid group_fwd_mask");
			addattr16(n, 1024, IFLA_BRPORT_GROUP_FWD_MASK, mask);
		} else if (arg == ARG_vlan_tunnel) {
			NEXT_ARG();
			bridge_slave_parse_on_off("vlan_tunnel", *argv, n,
						  IFLA_BRPORT_VLAN_TUNNEL);
		} else if (arg == ARG_isolated) {
			NEXT_ARG();
			bridge_slave_parse_on_off("isolated", *argv, n,
						  IFLA_BRPORT_ISOLATED);
		} else if (arg == ARG_locked) {
			NEXT_ARG();
			bridge_slave_parse_on_off("locked", *argv, n,
						  IFLA_BRPORT_LOCKED);
		} else if (arg == ARG_mab) {
			NEXT_ARG();
			bridge_slave_parse_on_off("mab", *argv, n,
						  IFLA_BRPORT_MAB);
		} else if (arg == ARG_backup_port) {
			int ifindex;

			NEXT_ARG();
			ifindex = xll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
			addattr32(n, 1024, IFLA_BRPORT_BACKUP_PORT, ifindex);
		} else if (arg == ARG_nobackup_port) {
			addattr32(n, 1024, IFLA_BRPORT_BACKUP_PORT, 0);
		} else if (arg == ARG_help) {
			bridge_slave_explain();
			return;
		} else {
			bb_error_msg_and_die("bridge_slave: unknown option \"%s\"?\n",
				*argv);
			return;
		}
		argv++;
	}

	return;
}



/* Return value becomes exitcode. It's okay to not return at all */
static int do_add_or_delete(char **argv, const unsigned rtm, unsigned int flags, int set)
{
	char **oldargv = argv;
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
	req.n.nlmsg_flags = NLM_F_REQUEST | flags;
	req.n.nlmsg_type = rtm;
	req.i.ifi_family = preferred_family;

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

		if (set && strcmp(type_str, "dsa") && strcmp(type_str, "bridge") && strcmp(type_str, "bridge_slave")) {
			return do_set(oldargv);
		}

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
#if ENABLE_IPVLAN
			else if (strcmp(type_str, "ipvlan") == 0)
				ipvlan_parse_opt(argv, &req.n, sizeof(req));
#endif
#if ENABLE_MACVLAN
			else if (strcmp(type_str, "macvlan") == 0)
				macvlan_parse_opt(argv, &req.n, sizeof(req));
#endif
#if ENABLE_DSA
			else if (strcmp(type_str, "dsa") == 0)
				dsa_parse_opt(argv, &req.n, sizeof(req));
#endif
#if ENABLE_BRIDGE
			else if (strcmp(type_str, "bridge") == 0)
				bridge_parse_opt(argv, &req.n, sizeof(req));
			else if (strcmp(type_str, "bridge_slave") == 0)
				bridge_slave_parse_opt(argv, &req.n, sizeof(req));
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
		if (key <= 2) /* add/delete */
			return do_add_or_delete(argv, key == 1 ? RTM_DELLINK : RTM_NEWLINK, key == 0 ? NLM_F_CREATE|NLM_F_EXCL : 0, key == 2);
	}
	/* show, lst, list */
	return ipaddr_list_link(argv);
}

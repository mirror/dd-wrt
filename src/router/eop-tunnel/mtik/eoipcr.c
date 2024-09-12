#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_tunnel.h>

#include "libnetlink.h"

static int print_link(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	struct ndmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *ifattr[IFLA_MAX + 1];
	struct rtattr *ifinfo[IFLA_INFO_MAX + 1];
	struct rtattr *ifgreo[IFLA_GRE_MAX + 1];
	const char *ifname;
	const char *kind;
	int link = 0;
	int tunid = 0;
	struct in_addr sip, dip;
	uint8_t ttl = 0, tos = 0;
	struct ifinfomsg *ifi;

	sip.s_addr = dip.s_addr = htonl(0);

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK) {
		fprintf(stderr, "Not RTM_xxxLINK: %08x %08x %08x\n", n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	ifi = (void *)r;

	parse_rtattr(ifattr, IFLA_MAX, IFLA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));
	if (ifattr[IFLA_IFNAME])
		ifname = rta_getattr_str(ifattr[IFLA_IFNAME]);
	else
		ifname = "";
	if (ifattr[IFLA_LINKINFO])
		parse_rtattr(ifinfo, IFLA_INFO_MAX, (void *)rta_getattr_str(ifattr[IFLA_LINKINFO]), ifattr[IFLA_LINKINFO]->rta_len);
	else
		memset(ifinfo, 0, sizeof ifinfo);
	if (ifinfo[IFLA_INFO_KIND])
		kind = rta_getattr_str(ifinfo[IFLA_INFO_KIND]);
	else
		kind = "";
	if (!strcmp(kind, "eoip") && ifinfo[IFLA_INFO_DATA]) {
		char ts[IFNAMSIZ], td[IFNAMSIZ];

		parse_rtattr(ifgreo, IFLA_GRE_MAX, (void *)rta_getattr_str(ifinfo[IFLA_INFO_DATA]),
			     ifinfo[IFLA_INFO_DATA]->rta_len);
		if (ifgreo[IFLA_GRE_LINK])
			link = rta_getattr_u32(ifgreo[IFLA_GRE_LINK]);
		if (ifgreo[IFLA_GRE_TOS])
			tos = rta_getattr_u8(ifgreo[IFLA_GRE_TOS]);
		if (ifgreo[IFLA_GRE_TTL])
			ttl = rta_getattr_u8(ifgreo[IFLA_GRE_TTL]);
		if (ifgreo[IFLA_GRE_LOCAL])
			sip.s_addr = rta_getattr_u32(ifgreo[IFLA_GRE_LOCAL]);
		if (ifgreo[IFLA_GRE_REMOTE])
			dip.s_addr = rta_getattr_u32(ifgreo[IFLA_GRE_REMOTE]);
		if (ifgreo[IFLA_GRE_IKEY])
			tunid = rta_getattr_u32(ifgreo[IFLA_GRE_IKEY]);
		strcpy(ts, inet_ntoa(sip));
		strcpy(td, inet_ntoa(dip));
		printf("%d: %s@%d: link/%s %s remote %s tunnel-id %d ttl %d tos %d\n", ifi->ifi_index, ifname, link, kind, ts, td,
		       tunid, ttl, tos);
	}

	return 0;
}

static void list(void)
{
	struct rtnl_handle rth;

	if (rtnl_open(&rth, 0)) {
		perror("Cannot open rtnetlink");
		return;
	}
	if (rtnl_wilddump_request(&rth, AF_UNSPEC, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		return;
	}

	if (rtnl_dump_filter(&rth, print_link, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return;
	}
	rtnl_close(&rth);
}

static int rtnetlink_request(struct nlmsghdr *msg, int buflen, struct sockaddr_nl *adr)
{
	int rsk;
	int n;

	/* Use a private socket to avoid having to keep state for a sequence number. */
	rsk = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rsk < 0)
		return -1;
	n = sendto(rsk, msg, buflen, 0, (struct sockaddr *)adr, sizeof(struct sockaddr_nl));
	if (errno)
		perror("in send");
	close(rsk);
	if (n < 0)
		return -1;
	return 0;
}

static int eoip_add(int excl, char *name, uint16_t tunnelid, uint32_t sip, uint32_t dip, uint32_t link, uint8_t ttl, uint8_t tos)
{
	struct {
		struct nlmsghdr msg;
		struct ifinfomsg ifi;
		struct rtattr a_name;
		char ifname[IFNAMSIZ];
		struct rtattr a_lnfo;
		struct rtattr a_kind;
		char kind[8];
		struct rtattr a_data;
		struct rtattr a_ikey;
		uint32_t ikey;
		struct rtattr a_sa;
		uint32_t sa;
		struct rtattr a_da;
		uint32_t da;
		struct rtattr a_link;
		uint32_t link;
		struct rtattr a_ttl;
		uint8_t ttl;
		uint8_t ttlpad[3];
		struct rtattr a_tos;
		uint8_t tos;
		uint8_t tospad[3];
		uint8_t dummy[0];
	} req = {
		.msg = {
			.nlmsg_len = 0,	// fix me later
		.nlmsg_type = RTM_NEWLINK,.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | (excl ? NLM_F_EXCL : 0),},.ifi = {
		.ifi_family = AF_UNSPEC,.ifi_index = 0,},.a_name = {
		.rta_len = IFNAMSIZ + sizeof(struct rtattr),.rta_type = IFLA_IFNAME,},.ifname = "",.a_lnfo = {
			.rta_len = 0,	// fix me later
		.rta_type = IFLA_LINKINFO,},.a_kind = {
		.rta_len = 8 + sizeof(struct rtattr),.rta_type = IFLA_INFO_KIND,},.kind = "eoip",.a_data = {
			.rta_len = 0,	// fix me later
		.rta_type = IFLA_INFO_DATA,},.a_ikey = {
		.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_IKEY,},.ikey = 4321,.a_sa = {
		.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_LOCAL,},.sa = htonl(0),.a_da = {
		.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_REMOTE,},.da = htonl(0),.a_link = {
		.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_LINK,},.link = 0,.a_ttl = {
		.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_TTL,},.ttl = 0,.a_tos = {
	.rta_len = 4 + sizeof(struct rtattr),.rta_type = IFLA_GRE_TOS,},.tos = 0,};
	struct sockaddr_nl adr = {
		.nl_family = AF_NETLINK,
	};

	req.msg.nlmsg_len = NLMSG_LENGTH(sizeof(req)) - sizeof(req.msg);

	req.a_name.rta_len = (char *)&req.a_lnfo - (char *)&req.a_name;
	req.a_lnfo.rta_len = (char *)&req.dummy - (char *)&req.a_lnfo;
	req.a_kind.rta_len = (char *)&req.a_data - (char *)&req.a_kind;
	req.a_data.rta_len = (char *)&req.dummy - (char *)&req.a_data;
	req.a_ikey.rta_len = (char *)&req.a_sa - (char *)&req.a_ikey;
	req.a_sa.rta_len = (char *)&req.a_da - (char *)&req.a_sa;
	req.a_da.rta_len = (char *)&req.a_link - (char *)&req.a_da;
	req.a_link.rta_len = (char *)&req.a_ttl - (char *)&req.a_link;
	req.a_ttl.rta_len = (char *)&req.a_tos - (char *)&req.a_ttl;
	req.a_tos.rta_len = (char *)&req.dummy - (char *)&req.a_tos;

	req.sa = sip;
	req.da = dip;
	strcpy(req.ifname, name);
	req.ikey = tunnelid;
	req.link = link;
	req.ttl = ttl;
	req.tos = tos;
	if (0) {
		unsigned char *p = (unsigned char *)&req;
		int l = (char *)&req.dummy - (char *)&req;
		int i;

		printf("req size: %d\n", req.msg.nlmsg_len);
		printf("name size: %d\n", req.a_name.rta_len);
		printf("lnfo size: %d\n", req.a_lnfo.rta_len);
		printf("kind size: %d\n", req.a_kind.rta_len);
		printf("data size: %d\n", req.a_data.rta_len);
		printf("ikey size: %d\n", req.a_ikey.rta_len);
		printf("sadr size: %d\n", req.a_sa.rta_len);
		printf("dadr size: %d\n", req.a_da.rta_len);

		printf("packet size: %d, data dump:", l);

		for (i = 0; i < l; i++)
			printf("%s%02x", (!(i % 16)) ? "\n" : " ", p[i]);
		fflush(stdout);
	}

	if (rtnetlink_request(&req.msg, sizeof(req), &adr) < 0) {
		perror("error in netlink request");
		return -1;
	}
	return 0;
}

typedef enum {
	E_AMBIGUOUS = -1,
	E_NONE = 0,
	C_LIST,
	C_ADD,
	C_SET,
	P_LOCAL,
	P_REMOTE,
	P_TUNNELID,
	P_TTL,
	P_TOS,
	P_LINK,
	P_NAME,
} e_cmd;

typedef struct {
	char *cmd;
	e_cmd cid;
} s_cmd;

static s_cmd cmds[] = {
	{
		.cmd = "list",
		.cid = C_LIST,
	},
	{
		.cmd = "show",
		.cid = C_LIST,
	},
	{
		.cmd = "add",
		.cid = C_ADD,
	},
	{
		.cmd = "new",
		.cid = C_ADD,
	},
	{
		.cmd = "set",
		.cid = C_SET,
	},
	{
		.cmd = "change",
		.cid = C_SET,
	},
	{
		.cmd = NULL,
		.cid = 0,
	},
};

static s_cmd prms[] = {
	{
		.cmd = "local",
		.cid = P_LOCAL,
	},
	{
		.cmd = "remote",
		.cid = P_REMOTE,
	},
	{
		.cmd = "tunnel-id",
		.cid = P_TUNNELID,
	},
	{
		.cmd = "tid",
		.cid = P_TUNNELID,
	},
	{
		.cmd = "ttl",
		.cid = P_TTL,
	},
	{
		.cmd = "tos",
		.cid = P_TOS,
	},
	{
		.cmd = "link",
		.cid = P_LINK,
	},
	{
		.cmd = "name",
		.cid = P_NAME,
	},
	{
		.cmd = NULL,
		.cid = 0,
	},
};

static e_cmd find_cmd(s_cmd *l, char *pcmd)
{
	int cnt = 0;
	int len = 0;
	e_cmd match = E_NONE;

	while (l && pcmd && l->cmd) {
		char *ha = l->cmd;
		char *ne = pcmd;
		int clen = 0;

		while (*ha && *ne && *ha == *ne)
			ha++, ne++, clen++;

		if (!*ne) {
			if (clen && clen > len) {
				cnt = 0;
				len = clen;
				match = l->cid;
			}
			if (clen && clen == len)
				cnt++;
		}
		l++;
	}
	if (cnt == 1)
		return match;
	return cnt ? E_AMBIGUOUS : E_NONE;
}

static void usage(char *me)
{
	printf("usage:\n"
	       "\t%s add [name <name>] tunnel-id <id> [local <ip>] remote <ip> [ttl <ttl>] [tos <tos>] [link <ifindex|ifname>]\n"
	       "\t%s set  name <name>  tunnel-id <id> [local <ip>] remote <ip> [ttl <ttl>] [tos <tos>] [link <ifindex|ifname>]\n"
	       "\t%s list\n",
	       me, me, me);
}

int main(int argc, char **argv)
{
	if (argc >= 1) {
		e_cmd c = (argc == 1) ? C_LIST : find_cmd(cmds, argv[1]);
		int excl = 1;

		switch (c) {
dafeutl:
		default:
			usage(argv[0]);
			return 0;
		case C_LIST:
			if (argc > 2)
				goto dafeutl;
			list();
			return 0;
		case C_SET:
			excl = 0;
		case C_ADD: {
			char ifname[IFNAMSIZ + 1] = "";
			uint32_t sip = htonl(0), dip = htonl(0);
			uint32_t tid = ~0;
			uint8_t ttl = 0, tos = 0;
			uint32_t link = 0;
			int i = 2;

			while (i < argc) {
				e_cmd p = find_cmd(prms, argv[i]);
				int noarg = !((i + 1) < argc);
				struct in_addr iad;

				switch (p) {
				default:
					break;
				case E_NONE:
					printf("unknown option: %s\n", argv[i]);
					return 0;
				case E_AMBIGUOUS:
					printf("option is ambiguous: %s\n", argv[i]);
					return 0;
				}

				if (noarg) {
					printf("option: %s requires an argument\n", argv[i]);
					return 0;
				}

				switch (p) {
				default:
					printf("BUG!\n");
					return 0;
				case P_NAME:
					ifname[(sizeof ifname) - 1] = 0;
					strncpy(ifname, argv[i + 1], IFNAMSIZ);
					break;
				case P_TTL:
					ttl = atoi(argv[i + 1]);
					break;
				case P_TOS:
					tos = atoi(argv[i + 1]);
					break;
				case P_LINK:
					// convert ifname to ifinidex, also support numeric arg
					link = if_nametoindex(argv[i + 1]);
					if (!link)
						link = atoi(argv[i + 1]);
					if (!link) {
						printf("invald interface name/index: %s\n", argv[i + 1]);
						return 0;
					}
					break;
				case P_TUNNELID:
					tid = atoi(argv[i + 1]);
					break;
				case P_LOCAL:
					if (!inet_aton(argv[i + 1], &iad)) {
						printf("invald ip address: %s\n", argv[i + 1]);
						return 0;
					}
					sip = iad.s_addr;
					break;
				case P_REMOTE:
					if (!inet_aton(argv[i + 1], &iad)) {
						printf("invald ip address: %s\n", argv[i + 1]);
						return 0;
					}
					dip = iad.s_addr;
					break;
				}
				i += 2;
			}
			if (tid > 0xffff) {
				if (tid == ~0)
					printf("tunnel-id is mandatory parameter\n");
				else
					printf("invalid tunnel-id value: %d\n", tid);
				return 0;
			}
			// tunnel id is in host byte order, addresses are in net byte order
			eoip_add(excl, ifname, tid, sip, dip, link, ttl, tos);
			return 0;
		}
		}
	}
	return 0;
}

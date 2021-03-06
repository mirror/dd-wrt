// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/types.h>
#include <linux/if_bridge.h>
#include <net/if.h>
#include <errno.h>

#include "libnetlink.h"
#include "list.h"
#include "cfm_netlink.h"

static struct rtnl_handle rth = { .fd = -1 };

static LIST_HEAD(mrp_rings);

struct mrp_ring {
	struct list_head list;
	uint32_t ifindex;
	uint32_t ring_id;
	uint32_t p_ifindex;
	uint32_t s_ifindex;
};

struct request {
	struct nlmsghdr		n;
	struct ifinfomsg	ifm;
	char			buf[1024];
};

static void cfm_nl_bridge_prepare(uint32_t ifindex, int cmd, struct request *req,
				  struct rtattr **afspec, struct rtattr **af,
				  struct rtattr **af_sub, int attr)
{
	req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req->n.nlmsg_flags = NLM_F_REQUEST;
	req->n.nlmsg_type = cmd;
	req->ifm.ifi_family = PF_BRIDGE;

	req->ifm.ifi_index = ifindex;

	*afspec = addattr_nest(&req->n, sizeof(*req), IFLA_AF_SPEC);
	addattr16(&req->n, sizeof(*req), IFLA_BRIDGE_FLAGS, BRIDGE_FLAGS_SELF);

	*af = addattr_nest(&req->n, sizeof(*req),
			   IFLA_BRIDGE_CFM | NLA_F_NESTED);
	*af_sub = addattr_nest(&req->n, sizeof(*req),
			       attr | NLA_F_NESTED);
}

static int cfm_nl_terminate(struct request *req, struct rtattr *afspec,
			    struct rtattr *af, struct rtattr *af_sub)
{
	int err;

	addattr_nest_end(&req->n, af_sub);
	addattr_nest_end(&req->n, af);
	addattr_nest_end(&req->n, afspec);

	err = rtnl_talk(&rth, &req->n, NULL);
	if (err) {
		printf("cfm_nl_terminate: rtnl_talk failed\n");
		return err;
	}

	return 0;
}

static char *rta_getattr_mac(const struct rtattr *rta)
{
	static char buf_ret[100];
	unsigned char mac[6];

	memcpy(&mac, RTA_DATA(rta), 6);
	snprintf(buf_ret, sizeof(buf_ret), "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return buf_ret;
}

static char *rta_getattr_short_name(const struct rtattr *rta)
{
	static char buf_ret[100];
	char *maid;
	int length = 0, maid_idx = 0;

	memset(buf_ret, 0, sizeof(buf_ret));

	maid = RTA_DATA(rta);

	maid_idx = 3;
	length = maid[2];
	if (maid[0] != 1) {
		length = maid[4];
		maid_idx = 4 + maid[1];
	}

	if (length <= sizeof(buf_ret))
		memcpy(buf_ret, &maid[maid_idx], length);

	return buf_ret;
}

static char *rta_getattr_domain_name(const struct rtattr *rta)
{
	static char buf_ret[100];
	char *maid;
	int length = 0;

	memset(buf_ret, 0, sizeof(buf_ret));

	maid = RTA_DATA(rta);

	if (maid[0] != 1) {
		length = maid[1];
		if (length <= sizeof(buf_ret))
			memcpy(buf_ret, &maid[2], length);
	}

	return buf_ret;
}

static char *int_domain(uint32_t value)
{
	switch (value) {
	case BR_CFM_PORT: return "port";
	case BR_CFM_VLAN: return "vlan";
	}
	return "undef";
}

static char *int_direction(uint32_t value)
{
	switch (value) {
	case BR_CFM_MEP_DIRECTION_DOWN: return "down";
	case BR_CFM_MEP_DIRECTION_UP: return "up";
	}
	return "undef";
}

static char *int_interval(uint32_t value)
{
	switch (value) {
	case BR_CFM_CCM_INTERVAL_3_3_MS: return "3ms3";
	case BR_CFM_CCM_INTERVAL_10_MS: return "10ms";
	case BR_CFM_CCM_INTERVAL_100_MS: return "100ms";
	case BR_CFM_CCM_INTERVAL_1_SEC: return "1s";
	case BR_CFM_CCM_INTERVAL_10_SEC: return "10s";
	case BR_CFM_CCM_INTERVAL_1_MIN: return "1m";
	case BR_CFM_CCM_INTERVAL_10_MIN: return "10m";
	}
	return "undef";
}

static int addattrmac(struct nlmsghdr *n, int maxlen, int type, struct mac_addr *mac)
{
//printf("addattrmac sizeof(*mac) %lu  rta_len %lu  mac %02X-%02X-%02X-%02X-%02X-%02X\n", sizeof(*mac), RTA_LENGTH(sizeof(mac->addr)), mac->addr[0], mac->addr[1], mac->addr[2], mac->addr[3], mac->addr[4], mac->addr[5]);
	return addattr_l(n, maxlen, type, mac->addr, sizeof(mac->addr));
}

static int addattrmaid(struct nlmsghdr *n, int maxlen, int type, struct maid_data *maid)
{
//printf("addattrmaid sizeof(*maid) %lu  rta_len %lu  len %u  maid %02X-%02X-%02X-%s\n", sizeof(*maid), RTA_LENGTH(sizeof(maid->data)), len, maid->data[0], maid->data[1], maid->data[2], &maid->data[3]);
	return addattr_l(n, maxlen, type, maid->data, sizeof(maid->data));
}

static int cfm_mep_config_show(struct nlmsghdr *n, void *arg)
{
	struct rtattr *aftb[IFLA_BRIDGE_MAX + 1];
	struct rtattr *infotb[IFLA_BRIDGE_CFM_MEP_CREATE_MAX + 1];
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX + 1];
	int len = n->nlmsg_len;
	struct rtattr *i, *list;
	int rem;
	uint32_t instance;
	char ifname[IF_NAMESIZE];

	memset(ifname, 0, IF_NAMESIZE);

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "Message too short!\n");
		return -1;
	}

	if (ifi->ifi_family != AF_BRIDGE)
		return 0;

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);
	if (!tb[IFLA_AF_SPEC])
		return 0;

	parse_rtattr_flags(aftb, IFLA_BRIDGE_MAX, RTA_DATA(tb[IFLA_AF_SPEC]), RTA_PAYLOAD(tb[IFLA_AF_SPEC]), NLA_F_NESTED);
	if (!aftb[IFLA_BRIDGE_CFM])
		return 0;

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP create:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_MEP_CREATE_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_MEP_CREATE_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_MEP_CREATE_INSTANCE]) {
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CREATE_INSTANCE]));
			printf("    Domain %s\n", int_domain(rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CREATE_DOMAIN])));
			printf("    Direction %s\n", int_direction(rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CREATE_DIRECTION])));
			printf("    Port %s\n", if_indextoname(rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CREATE_IFINDEX]), ifname));
		}
		printf("\n");
	}

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP config:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_MEP_CONFIG_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_MEP_CONFIG_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_MEP_CONFIG_INSTANCE]) {
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CONFIG_INSTANCE]));
			printf("    Unicast_mac %s\n", rta_getattr_mac(infotb[IFLA_BRIDGE_CFM_MEP_CONFIG_UNICAST_MAC]));
			printf("    Mdlevel %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CONFIG_MDLEVEL]));
			printf("    Mepid %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_CONFIG_MEPID]));
		}
		printf("\n");
	}

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP cc_config:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_CONFIG_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_CONFIG_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_CC_CONFIG_INSTANCE]) {
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CONFIG_INSTANCE]));
			printf("    Enable %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CONFIG_ENABLE]));
			printf("    Interval %s\n", int_interval(rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CONFIG_EXP_INTERVAL])));
			printf("    Domain-name %s\n",
				rta_getattr_domain_name(infotb[IFLA_BRIDGE_CFM_CC_CONFIG_EXP_MAID]));
			printf("    Short-name %s\n",
				rta_getattr_short_name(infotb[IFLA_BRIDGE_CFM_CC_CONFIG_EXP_MAID]));
		}
		printf("\n");
	}

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP cc_peer_config:");
	instance = 0xFFFFFFFF;
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_PEER_MEP_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_PEER_MEP_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_CC_PEER_MEP_INSTANCE]) {
			if (instance != rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_MEP_INSTANCE])) {
				instance = rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_MEP_INSTANCE]);
				printf("\n");
				printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_MEP_INSTANCE]));
				printf("    Peer-mep");
			}
			printf(" %u", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_MEPID]));
		}
	}
	printf("\n\n");

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP cc_ccm_tx_config:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_CCM_TX_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_CCM_TX_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_INSTANCE]) {
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_INSTANCE]));
			printf("    Dmac %s\n", rta_getattr_mac(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_DMAC]));
			printf("    sequence %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_SEQ_NO_UPDATE]));
			printf("    period %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_PERIOD]));
			printf("    iftlv %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_IF_TLV]));
			printf("    iftlv-value %u\n", rta_getattr_u8(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_IF_TLV_VALUE]));
			printf("    porttlv %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_PORT_TLV]));
			printf("    porttlv-value %u\n", rta_getattr_u8(infotb[IFLA_BRIDGE_CFM_CC_CCM_TX_PORT_TLV_VALUE]));
		}
		printf("\n");
	}

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP cc_rdi_config:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_RDI_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_RDI_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);

		if (infotb[IFLA_BRIDGE_CFM_CC_RDI_INSTANCE]) {
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_RDI_INSTANCE]));
			printf("    Rdi %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_RDI_RDI]));
		}
		printf("\n");
	}

	return 0;
}

static int cfm_mep_status_show(struct nlmsghdr *n, void *arg)
{
	struct rtattr *aftb[IFLA_BRIDGE_MAX + 1];
	struct rtattr *infotb[IFLA_BRIDGE_CFM_MEP_STATUS_MAX + 1];
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX + 1];
	int len = n->nlmsg_len;
	struct rtattr *i, *list;
	int rem;
	uint32_t instance;
	char ifname[IF_NAMESIZE];

	memset(ifname, 0, IF_NAMESIZE);

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "Message too short!\n");
		return -1;
	}

	if (ifi->ifi_family != AF_BRIDGE)
		return 0;

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);
	if (!tb[IFLA_AF_SPEC])
		return 0;

	parse_rtattr_flags(aftb, IFLA_BRIDGE_MAX, RTA_DATA(tb[IFLA_AF_SPEC]), RTA_PAYLOAD(tb[IFLA_AF_SPEC]), NLA_F_NESTED);
	if (!aftb[IFLA_BRIDGE_CFM])
		return 0;

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM MEP status:\n");
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_MEP_STATUS_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_MEP_STATUS_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);
		if (!infotb[IFLA_BRIDGE_CFM_MEP_STATUS_INSTANCE])
			continue;

		printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_STATUS_INSTANCE]));
		printf("    Opcode unexp seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_STATUS_OPCODE_UNEXP_SEEN]));
		printf("    Version unexp seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_STATUS_VERSION_UNEXP_SEEN]));
		printf("    Rx level low seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_MEP_STATUS_RX_LEVEL_LOW_SEEN]));
		printf("\n");
	}

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM CC peer status:\n");
	instance = 0xFFFFFFFF;
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_PEER_STATUS_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_PEER_STATUS_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);
		if (!infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE])
			continue;

		if (instance != rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE])) {
			instance = rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE]);
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE]));
		}
		printf("    Peer-mep %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_PEER_MEPID]));
		printf("        CCM defect %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_CCM_DEFECT]));
		printf("        Rdi %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_RDI]));
		printf("        Port tlv %u\n", rta_getattr_u8(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_PORT_TLV_VALUE]));
		printf("        If tlv %u\n", rta_getattr_u8(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_IF_TLV_VALUE]));
		printf("        CCM seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_SEEN]));
		printf("        Tlv seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_TLV_SEEN]));
		printf("        Seq unexp seen %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_SEQ_UNEXP_SEEN]));
		printf("\n");
	}

	return 0;
}

int cfm_offload_init(void)
{
	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return EXIT_FAILURE;
	}

	return 0;
}

void cfm_offload_uninit(void)
{
	rtnl_close(&rth);
}

int cfm_offload_mep_create(uint32_t br_ifindex, uint32_t instance, uint32_t domain, uint32_t direction, uint32_t ifindex)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec,
			      &af, &af_sub, IFLA_BRIDGE_CFM_MEP_CREATE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CREATE_INSTANCE,
		  instance);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CREATE_DOMAIN,
		  domain);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CREATE_DIRECTION,
		  direction);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CREATE_IFINDEX,
		  ifindex);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_mep_delete(uint32_t br_ifindex, uint32_t instance)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec, &af,
			      &af_sub, IFLA_BRIDGE_CFM_MEP_DELETE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_DELETE_INSTANCE,
		  instance);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_mep_config(uint32_t br_ifindex, uint32_t instance, struct mac_addr *mac,
			   uint32_t level, uint32_t mepid)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec,
			      &af, &af_sub, IFLA_BRIDGE_CFM_MEP_CONFIG);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CONFIG_INSTANCE,
		  instance);
	addattrmac(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CONFIG_UNICAST_MAC,
		   mac);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CONFIG_MDLEVEL,
		  level);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_MEP_CONFIG_MEPID,
		  mepid);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_cc_config(uint32_t br_ifindex, uint32_t instance, uint32_t enable,
			  uint32_t interval, struct maid_data *maid)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec,
			      &af, &af_sub, IFLA_BRIDGE_CFM_CC_CONFIG);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CONFIG_INSTANCE,
		  instance);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CONFIG_ENABLE,
		  enable);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CONFIG_EXP_INTERVAL,
		  interval);
	addattrmaid(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CONFIG_EXP_MAID,
		    maid);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_cc_peer(uint32_t br_ifindex, uint32_t instance, uint32_t remove, uint32_t mepid)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };
	int attr;

	attr = (remove) ? IFLA_BRIDGE_CFM_CC_PEER_MEP_REMOVE : IFLA_BRIDGE_CFM_CC_PEER_MEP_ADD;

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec, &af,
			      &af_sub, attr);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_PEER_MEP_INSTANCE,
		  instance);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_PEER_MEPID,
		  mepid);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_cc_rdi(uint32_t br_ifindex, uint32_t instance, uint32_t rdi)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec, &af,
			      &af_sub, IFLA_BRIDGE_CFM_CC_RDI);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_RDI_INSTANCE,
		  instance);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_RDI_RDI,
		  rdi);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_cc_ccm_tx(uint32_t br_ifindex, uint32_t instance,
			  struct mac_addr *dmac, uint32_t sequence, uint32_t period, uint32_t iftlv,
			  uint8_t iftlv_value, uint32_t porttlv, uint8_t porttlv_value)
{
	struct rtattr *afspec, *af, *af_sub;
	struct request req = { 0 };

	cfm_nl_bridge_prepare(br_ifindex, RTM_SETLINK, &req, &afspec,
			      &af, &af_sub, IFLA_BRIDGE_CFM_CC_CCM_TX);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_INSTANCE,
		  instance);
	addattrmac(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_DMAC,
		   dmac);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_SEQ_NO_UPDATE,
		  sequence);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_PERIOD,
		  period);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_IF_TLV,
		  iftlv);
	addattr8(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_IF_TLV_VALUE,
		  iftlv_value);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_PORT_TLV,
		  porttlv);
	addattr8(&req.n, sizeof(req), IFLA_BRIDGE_CFM_CC_CCM_TX_PORT_TLV_VALUE,
		  porttlv_value);

	return cfm_nl_terminate(&req, afspec, af, af_sub);
}

int cfm_offload_mep_config_show(uint32_t br_ifindex)
{
	int err;

	err = rtnl_linkdump_req_filter(&rth, PF_BRIDGE, RTEXT_FILTER_CFM_CONFIG);
	if (err < 0) {
		fprintf(stderr, "Cannot rtnl_linkdump_req_filter\n");
		return err;
	}

	return rtnl_dump_filter(&rth, cfm_mep_config_show, NULL);
}

int cfm_offload_mep_status_show(uint32_t br_ifindex)
{
	int err;

	err = rtnl_linkdump_req_filter(&rth, PF_BRIDGE, RTEXT_FILTER_CFM_STATUS);
	if (err < 0) {
		fprintf(stderr, "Cannot rtnl_linkdump_req_filter\n");
		return err;
	}

	return rtnl_dump_filter(&rth, cfm_mep_status_show, NULL);
}

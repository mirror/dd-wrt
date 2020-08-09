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

#include "state_machine.h"
#include "utils.h"
#include "libnetlink.h"

struct rtnl_handle rth = { .fd = -1 };

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

static void mrp_nl_bridge_prepare(uint32_t ifindex, int cmd, struct request *req,
				  struct rtattr **afspec, struct rtattr **afmrp,
				  struct rtattr **af_submrp, int mrp_attr)
{
	req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req->n.nlmsg_flags = NLM_F_REQUEST;
	req->n.nlmsg_type = cmd;
	req->ifm.ifi_family = PF_BRIDGE;

	req->ifm.ifi_index = ifindex;

	*afspec = addattr_nest(&req->n, sizeof(*req), IFLA_AF_SPEC);
	addattr16(&req->n, sizeof(*req), IFLA_BRIDGE_FLAGS, BRIDGE_FLAGS_SELF);

	*afmrp = addattr_nest(&req->n, sizeof(*req),
			      IFLA_BRIDGE_MRP | NLA_F_NESTED);
	*af_submrp = addattr_nest(&req->n, sizeof(*req),
				  mrp_attr | NLA_F_NESTED);
}

static void mrp_nl_port_prepare(struct mrp_port *port, int cmd,
				struct request *req, struct rtattr **afspec,
				struct rtattr **afmrp,
				struct rtattr **af_submrp, int mrp_attr)
{
	req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req->n.nlmsg_flags = NLM_F_REQUEST;
	req->n.nlmsg_type = cmd;
	req->ifm.ifi_family = PF_BRIDGE;

	req->ifm.ifi_index = port->ifindex;

	*afspec = addattr_nest(&req->n, sizeof(*req), IFLA_AF_SPEC);
	*afmrp = addattr_nest(&req->n, sizeof(*req),
			      IFLA_BRIDGE_MRP | NLA_F_NESTED);
	*af_submrp = addattr_nest(&req->n, sizeof(*req),
				  mrp_attr | NLA_F_NESTED);
}

static int mrp_nl_terminate(struct request *req, struct rtattr *afspec,
			    struct rtattr *afmrp, struct rtattr *af_submrp)
{
	int err;

	addattr_nest_end(&req->n, af_submrp);
	addattr_nest_end(&req->n, afmrp);
	addattr_nest_end(&req->n, afspec);

	err = rtnl_talk(&rth, &req->n, NULL);
	if (err)
		return err;

	return 0;
}

static int get_bridges(struct nlmsghdr *n, void *arg)
{
	struct rtattr *aftb[IFLA_BRIDGE_MAX + 1];
	struct rtattr *mrp_infotb[IFLA_BRIDGE_MRP_INFO_MAX + 1];
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX + 1];
	struct mrp_ring *mrp_ring;
	int len = n->nlmsg_len;
	struct rtattr *i, *list;
	int rem;

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

	parse_rtattr_nested(aftb, IFLA_BRIDGE_MAX, tb[IFLA_AF_SPEC]);
	if (!aftb[IFLA_BRIDGE_MRP])
		return 0;

	list = aftb[IFLA_BRIDGE_MRP];
	rem = RTA_PAYLOAD(list);
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != IFLA_BRIDGE_MRP_INFO)
			continue;

		parse_rtattr_nested(mrp_infotb, IFLA_BRIDGE_MRP_INFO_MAX, i);
		if (!mrp_infotb[IFLA_BRIDGE_MRP_INFO_RING_ID])
			return 0;

		mrp_ring = malloc(sizeof(*mrp_ring));
		memset(mrp_ring, 0x0, sizeof(*mrp_ring));
		mrp_ring->ifindex = rta_getattr_u32(tb[IFLA_MASTER]);
		mrp_ring->ring_id = rta_getattr_u32(mrp_infotb[IFLA_BRIDGE_MRP_INFO_RING_ID]);
		mrp_ring->p_ifindex = rta_getattr_u32(mrp_infotb[IFLA_BRIDGE_MRP_INFO_P_IFINDEX]);
		mrp_ring->s_ifindex = rta_getattr_u32(mrp_infotb[IFLA_BRIDGE_MRP_INFO_S_IFINDEX]);
		list_add_tail(&mrp_ring->list, &mrp_rings);
	}

	return 0;
}

static int mrp_offload_clear(void)
{
	struct mrp_ring *mrp_ring;
	int err;

	err = rtnl_linkdump_req_filter(&rth, PF_BRIDGE, RTEXT_FILTER_MRP);
	if (err < 0) {
		fprintf(stderr, "Cannot rtnl_linkdump_req_filter\n");
		return err;
	}

	rtnl_dump_filter(&rth, get_bridges, NULL);

	list_for_each_entry(mrp_ring, &mrp_rings, list) {
		struct rtattr *afspec, *afmrp, *af_submrp;
		struct request req = { 0 };

		mrp_nl_bridge_prepare(mrp_ring->ifindex, RTM_DELLINK, &req,
				      &afspec, &afmrp, &af_submrp,
				      IFLA_BRIDGE_MRP_INSTANCE);

		addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_RING_ID,
			  mrp_ring->ring_id);
		addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_P_IFINDEX,
			  mrp_ring->p_ifindex);
		addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_S_IFINDEX,
			  mrp_ring->s_ifindex);

		mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
	}

	return 0;
}

int mrp_offload_init(void)
{
	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return EXIT_FAILURE;
	}

	mrp_offload_clear();
	return 0;
}

void mrp_offload_uninit(void)
{
	rtnl_close(&rth);
}

int mrp_offload_add(struct mrp *mrp, struct mrp_port *p, struct mrp_port *s,
		    uint16_t prio)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec,
			      &afmrp, &af_submrp, IFLA_BRIDGE_MRP_INSTANCE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_RING_ID,
		  mrp->ring_nr);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_P_IFINDEX,
		  p->ifindex);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_S_IFINDEX,
		  s->ifindex);
	addattr16(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_PRIO, prio);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_del(struct mrp *mrp)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_DELLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_INSTANCE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_RING_ID,
		  mrp->ring_nr);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_P_IFINDEX,
		  mrp->p_port->ifindex);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_INSTANCE_S_IFINDEX,
		  mrp->s_port->ifindex);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_port_offload_set_state(struct mrp_port *p,
			       enum br_mrp_port_state_type state)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	p->state = state;

	mrp_nl_port_prepare(p, RTM_SETLINK, &req, &afspec, &afmrp,
			    &af_submrp, IFLA_BRIDGE_MRP_PORT_STATE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_PORT_STATE_STATE, state);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_port_offload_set_role(struct mrp_port *p,
			      enum br_mrp_port_role_type role)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	p->role = role;

	mrp_nl_port_prepare(p, RTM_SETLINK, &req, &afspec, &afmrp,
			    &af_submrp, IFLA_BRIDGE_MRP_PORT_ROLE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_PORT_ROLE_ROLE, role);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_set_ring_state(struct mrp *mrp,
			       enum br_mrp_ring_state_type state)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_RING_STATE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_RING_STATE_RING_ID,
		  mrp->ring_nr);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_RING_STATE_STATE,
		  state);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_set_ring_role(struct mrp *mrp, enum br_mrp_ring_role_type role)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp->ring_role = role;

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_RING_ROLE);

	if (mrp->mra_support)
		role = BR_MRP_RING_ROLE_MRA;

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_RING_ROLE_RING_ID,
		  mrp->ring_nr);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_RING_ROLE_ROLE,
		  role);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_send_ring_test(struct mrp *mrp, uint32_t interval, uint32_t max,
			       uint32_t period)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_START_TEST);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_TEST_RING_ID,
		  mrp->ring_nr);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_TEST_INTERVAL,
		  interval);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_TEST_MAX_MISS,
		  max);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_TEST_PERIOD,
		  period);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_TEST_MONITOR,
		  mrp->test_monitor);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_set_in_state(struct mrp *mrp,
			     enum br_mrp_in_state_type state)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_IN_STATE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_STATE_IN_ID,
		  mrp->in_id);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_STATE_STATE,
		  state);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_set_in_role(struct mrp *mrp, enum br_mrp_in_role_type role)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp->in_role = role;

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_IN_ROLE);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_ROLE_RING_ID,
		  mrp->ring_nr);
	addattr16(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_ROLE_IN_ID,
		  mrp->in_id);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_ROLE_I_IFINDEX,
		  mrp->i_port->ifindex);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_IN_ROLE_ROLE,
		  role);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_send_in_test(struct mrp *mrp, uint32_t interval, uint32_t max,
			      uint32_t period)
{
	struct rtattr *afspec, *afmrp, *af_submrp;
	struct request req = { 0 };

	mrp_nl_bridge_prepare(mrp->ifindex, RTM_SETLINK, &req, &afspec, &afmrp,
			      &af_submrp, IFLA_BRIDGE_MRP_START_IN_TEST);

	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_IN_TEST_IN_ID,
		  mrp->in_id);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_IN_TEST_INTERVAL,
		  interval);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_IN_TEST_MAX_MISS,
		  max);
	addattr32(&req.n, sizeof(req), IFLA_BRIDGE_MRP_START_IN_TEST_PERIOD,
		  period);

	return mrp_nl_terminate(&req, afspec, afmrp, af_submrp);
}

int mrp_offload_flush(struct mrp *mrp)
{
	struct request req = { 0 };
	struct rtattr *protinfo;

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_SETLINK;
	req.ifm.ifi_family = PF_BRIDGE;

	protinfo = addattr_nest(&req.n, sizeof(req),
				IFLA_PROTINFO | NLA_F_NESTED);
	addattr(&req.n, 1024, IFLA_BRPORT_FLUSH);

	addattr_nest_end(&req.n, protinfo);

	req.ifm.ifi_index = mrp->p_port->ifindex;
	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	req.ifm.ifi_index = mrp->s_port->ifindex;
	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	if (!mrp->i_port)
		return 0;

	req.ifm.ifi_index = mrp->i_port->ifindex;
	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	return 0;
}


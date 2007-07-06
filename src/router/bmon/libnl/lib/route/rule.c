/*
 * rule.c          rtnetlink rules layer
 *
 * Copyright (c) 2003-2005 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/**
 * @ingroup rtnl
 * @defgroup rule Rules
 *
 * Module to access and modify routing rules.
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/rule.h>
#include <netlink/route/rtattr.h>
#include <inttypes.h>

static int rule_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			   void *arg)
{
	struct rtnl_rule rule = RTNL_INIT_RULE();
	struct rtmsg *r = NLMSG_DATA(n);
	struct rtattr *tb[RTA_MAX+1];
	struct nl_parser_param *pp = arg;
	size_t len;
	int err;

	if (n->nlmsg_type != RTM_NEWRULE)
		return P_IGNORE;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
	if (len < 0)
		return nl_error(EINVAL, "netlink message too short to be " \
					"a rule message");

	r = NLMSG_DATA(n);
	err = nl_parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	if (err < 0)
		return err;

	rule.r_family = r->rtm_family;
	rule.r_type = r->rtm_type;
	rule.r_dsfield = r->rtm_tos;
	rule.r_scope = r->rtm_scope;
	rule.r_proto = r->rtm_protocol;
	rule.r_mask = (RULE_HAS_FAMILY | RULE_HAS_TYPE | RULE_HAS_DSFIELD |
		       RULE_HAS_SCOPE | RULE_HAS_PROTO);

	if (r->rtm_table) {
		rule.r_table = r->rtm_table;
		rule.r_mask |= RULE_HAS_TABLE;
	}

	if (tb[RTA_PRIORITY]) {
		if ((err = NL_COPY_DATA(rule.r_prio, tb[RTA_PRIORITY])) < 0)
			return err;
		rule.r_mask |= RULE_HAS_PRIO;
	}

	if (tb[RTA_SRC]) {
		nl_copy_addr(&rule.r_src, tb[RTA_SRC]);
		rule.r_src.a_family = r->rtm_family;
		rule.r_mask |= RULE_HAS_SRC;
	}

	if (tb[RTA_DST]) {
		nl_copy_addr(&rule.r_src, tb[RTA_DST]);
		rule.r_dst.a_family = r->rtm_family;
		rule.r_mask |= RULE_HAS_DST;
	}

	if (tb[RTA_PROTOINFO]) {
		if ((err = NL_COPY_DATA(rule.r_fwmark, tb[RTA_PROTOINFO])) < 0)
			return err;
		rule.r_mask |= RULE_HAS_FWMARK;
	}

	if (tb[RTA_IIF]) {
		if (RTA_PAYLOAD(tb[RTA_IIF]) >= (sizeof(rule.r_iif) - 1))
			return nl_error(EINVAL, "rule iif TLV length " \
					"exceeds local link name limit");

		memcpy(rule.r_iif, RTA_DATA(tb[RTA_IIF]),
		       RTA_PAYLOAD(tb[RTA_IIF]));
		rule.r_iif[sizeof(rule.r_iif) - 1] = '\0';
		rule.r_mask |= RULE_HAS_IIF;
	}

	if (tb[RTA_FLOW]) {
		if ((err = NL_COPY_DATA(rule.r_realms, tb[RTA_FLOW])) < 0)
			return err;
		rule.r_mask |= RULE_HAS_REALMS;
	}

	err = pp->pp_cb((struct nl_common *) &rule, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int rule_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETRULE, AF_UNSPEC, NLM_F_DUMP);
}

static int rule_dump_brief(struct nl_cache *c, struct nl_common *o, FILE *fd,
			   struct nl_dump_params *params)
{
	struct rtnl_rule *r = (struct rtnl_rule *) o;
	char addr[INET6_ADDRSTRLEN+5], realms[32], type[32];

	dp_new_line(fd, params, 0);

	if (r->r_mask & RULE_HAS_PRIO)
		fprintf(fd, "%d:\t", r->r_prio);
	else
		fprintf(fd, "0:\t");

	if (r->r_mask & RULE_HAS_SRC)
		fprintf(fd, "from %s ",
			nl_addr2str_r(&r->r_src, addr, sizeof(addr)));
	else if (r->r_mask & RULE_HAS_SRC_LEN && r->r_src_len)
		fprintf(fd, "from 0/%d ", r->r_src_len);
	else
		fprintf(fd, "from all ");

	if (r->r_mask & RULE_HAS_DST)
		fprintf(fd, "to %s ",
			nl_addr2str_r(&r->r_dst, addr, sizeof(addr)));
	else if (r->r_mask & RULE_HAS_DST_LEN && r->r_dst_len)
		fprintf(fd, "to 0/%d ", r->r_dst_len);
	else
		fprintf(fd, "to all ");

	if (r->r_mask & RULE_HAS_DSFIELD)
		fprintf(fd, "tos %d ", r->r_dsfield);

	if (r->r_mask & RULE_HAS_FWMARK)
		fprintf(fd, "fwmark %" PRIx64 , r->r_fwmark);

	if (r->r_mask & RULE_HAS_IIF)
		fprintf(fd, "iif %s ", r->r_iif);

	if (r->r_mask & RULE_HAS_TABLE)
		fprintf(fd, "lookup %d ", r->r_table);

	if (r->r_mask & RULE_HAS_REALMS)
		fprintf(fd, "realms %s ",
			rtnl_realms2str_r(r->r_realms, realms, sizeof(realms)));

	fprintf(fd, "%s\n", nl_rtntype2str_r(r->r_type, type, sizeof(type)));

	return 1;
}

static int rule_filter(struct nl_common *obj, struct nl_common *filter)
{
	struct rtnl_rule *o = (struct rtnl_rule *) obj;
	struct rtnl_rule *f = (struct rtnl_rule *) filter;

	if (obj->ce_type != RTNL_RULE || filter->ce_type != RTNL_RULE)
		return 0;

#define REQ(F) (f->r_mask & RULE_HAS_##F)
#define AVAIL(F) (o->r_mask & RULE_HAS_##F)
	if ((REQ(FAMILY)    &&
	      (!AVAIL(FAMILY)    || o->r_family != f->r_family))	||
	    (REQ(SCOPE)     &&
	      (!AVAIL(SCOPE)     || o->r_scope != f->r_scope))		||
	    (REQ(TABLE)     &&
	      (!AVAIL(TABLE)     || o->r_table != f->r_table))		||
	    (REQ(REALMS)    &&
	      (!AVAIL(REALMS)    || o->r_realms != f->r_realms))	||
	    (REQ(DSFIELD)   &&
	      (!AVAIL(DSFIELD)   || o->r_dsfield != f->r_dsfield))	||
	    (REQ(TYPE)      &&
	      (!AVAIL(TYPE)      || o->r_type != f->r_type))		||
	    (REQ(PRIO)      &&
	      (!AVAIL(PRIO)      || o->r_prio != f->r_prio))		||
	    (REQ(PROTO)     &&
	      (!AVAIL(PROTO)     || o->r_proto != f->r_proto))		||
	    (REQ(FWMARK)    &&
	      (!AVAIL(FWMARK)    || o->r_fwmark != f->r_fwmark))	||
	    (REQ(SRC_LEN)   &&
	      (!AVAIL(SRC_LEN)   || o->r_src_len != f->r_src_len))	||
	    (REQ(DST_LEN)   &&
	      (!AVAIL(DST_LEN)   || o->r_dst_len != f->r_dst_len))	||
	    (REQ(SRC)       &&
	      (!AVAIL(SRC)       || nl_addrcmp(&o->r_src, &f->r_src)))	||
	    (REQ(IIF)       &&
	      (!AVAIL(IIF)       || strcmp(o->r_iif, f->r_iif)))	||
	    (REQ(DST)       &&
	      (!AVAIL(DST)       || nl_addrcmp(&o->r_dst, &f->r_dst))))
		return 0;
#undef REQ
#undef AVAIL

	return 1;
}

/**
 * @name General API
 * @{
 */

/**
 * Build a rule cache including all rules currently configured in the kernel.
 * @arg handle		netlink handle
 *
 * Allocates a new rule cache, initializes it properly and updates it
 * to include all rules currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The new cache or NULL if an error occured.
 */
struct nl_cache * rtnl_rule_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_RULE;
	cache->c_type_size = sizeof(struct rtnl_rule);
	cache->c_ops = &rtnl_rule_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Dump rule attributes
 * @arg rule		rule to dump
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_rule_dump(struct rtnl_rule *rule, FILE *fd,
		    struct nl_dump_params *params)
{
	dump_from_ops((struct nl_common *) rule, fd, params, &rtnl_rule_ops);
}

/** @} */

/**
 * @name Rule Addition/Deletion
 * @{
 */

static struct nl_msg *build_rule_msg(struct rtnl_rule *tmpl, int cmd, int flags)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = cmd,
		.nlmsg_flags = flags,
	};
	struct rtmsg rtm = {
		.rtm_protocol = RTPROT_BOOT,
		.rtm_scope = RT_SCOPE_UNIVERSE,
		.rtm_type = RTN_UNSPEC
	};

	if (cmd == RTM_NEWRULE)
		rtm.rtm_type = RTN_UNICAST;
		
	if (tmpl->r_mask & RULE_HAS_FAMILY)
		rtm.rtm_family = tmpl->r_family;

	if (tmpl->r_mask & RULE_HAS_TABLE)
		rtm.rtm_table = tmpl->r_table;

	if (tmpl->r_mask & RULE_HAS_DSFIELD)
		rtm.rtm_tos = tmpl->r_dsfield;

	if (tmpl->r_mask & RULE_HAS_TYPE)
		rtm.rtm_type = tmpl->r_type;

	if (tmpl->r_mask & RULE_HAS_SRC_LEN)
		rtm.rtm_src_len = tmpl->r_src_len;

	if (tmpl->r_mask & RULE_HAS_DST_LEN)
		rtm.rtm_dst_len = tmpl->r_dst_len;

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &rtm, sizeof(rtm));

	if (tmpl->r_mask & RULE_HAS_SRC)
		nl_msg_append_tlv(m, RTA_SRC, &tmpl->r_src.a_addr,
				  tmpl->r_src.a_len);

	if (tmpl->r_mask & RULE_HAS_DST)
		nl_msg_append_tlv(m, RTA_DST, &tmpl->r_dst.a_addr,
				  tmpl->r_dst.a_len);

	if (tmpl->r_mask & RULE_HAS_PRIO)
		nl_msg_append_tlv(m, RTA_PRIORITY, &tmpl->r_prio,
				  sizeof(tmpl->r_prio));

	if (tmpl->r_mask & RULE_HAS_FWMARK)
		nl_msg_append_tlv(m, RTA_PROTOINFO, &tmpl->r_fwmark,
				  sizeof(uint32_t));

	if (tmpl->r_mask & RULE_HAS_REALMS)
		nl_msg_append_tlv(m, RTA_FLOW, &tmpl->r_realms,
				  sizeof(tmpl->r_realms));

	if (tmpl->r_mask & RULE_HAS_IIF)
		nl_msg_append_tlv(m, RTA_IIF, tmpl->r_iif,
				  strlen(tmpl->r_iif) + 1);




	return m;
}

/**
 * Build netlink request message to add a new rule
 * @arg tmpl		template with data of new rule
 *
 * Builds a new netlink message requesting a addition of a new
 * rule. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a tmpl must contain the attributes of the new
 * address set via \c rtnl_rule_set_* functions.
 * 
 * @return The netlink message
 */
struct nl_msg * rtnl_rule_build_add_request(struct rtnl_rule *tmpl)
{
	return build_rule_msg(tmpl, RTM_NEWRULE, NLM_F_CREATE|NLM_F_EXCL);
}

/**
 * Add a new rule
 * @arg handle		netlink handle
 * @arg tmpl		template with requested changes
 *
 * Builds a netlink message by calling rtnl_rule_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_rule_add(struct nl_handle *handle, struct rtnl_rule *tmpl)
{
	int err;
	struct nl_msg *m = rtnl_rule_build_add_request(tmpl);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete a rule
 * @arg rule		rule to delete
 *
 * Builds a new netlink message requesting a deletion of a rule.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a rule must point to an existing
 * address.
 *
 * @return The netlink message
 */
struct nl_msg * rtnl_rule_build_delete_request(struct rtnl_rule *rule)
{
	return build_rule_msg(rule, RTM_DELRULE, 0);
}

/**
 * Delete a rule
 * @arg handle		netlink handle
 * @arg rule		rule to delete
 *
 * Builds a netlink message by calling rtnl_rule_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_rule_delete(struct nl_handle *handle, struct rtnl_rule *rule)
{
	int err;
	struct nl_msg *m = rtnl_rule_build_delete_request(rule);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Realms Translations
 * @{
 */

char * rtnl_realms2str_r(uint32_t realms, char *buf, size_t len)
{
	int from = RTNL_REALM_FROM(realms);
	int to = RTNL_REALM_TO(realms);

	snprintf(buf, len, "%d/%d", from, to);

	return buf;
}

char * rtnl_realms2str(uint32_t realms)
{
	static char buf[64];
	return rtnl_realms2str_r(realms, buf, sizeof(buf));
}

/** @} */

/**
 * @name Attribute Modifications
 * @{
 */

/**
 * Set the address family of a rule to the specified value
 * @arg rule		rule to change
 * @arg family		new address family
 */
void rtnl_rule_set_family(struct rtnl_rule *rule, int family)
{
	rule->r_family = family;
	rule->r_mask |= RULE_HAS_FAMILY;
}

/**
 * Set the priority of a rule to the specified value
 * @arg rule		rule to change
 * @arg prio		new priority
 */
void rtnl_rule_set_prio(struct rtnl_rule *rule, int prio)
{
	rule->r_prio = prio;
	rule->r_mask |= RULE_HAS_PRIO;
}

/**
 * Set the firewall mark of a rule to the specified value
 * @arg rule		rule to change
 * @arg fwmark		new firewall mark
 */
void rtnl_rule_set_fwmark(struct rtnl_rule *rule, uint64_t fwmark)
{
	rule->r_fwmark = fwmark;
	rule->r_mask |= RULE_HAS_FWMARK;
}

/**
 * Set the table of a rule to the specified value
 * @arg rule		rule to change
 * @arg table		new table
 */
void rtnl_rule_set_table(struct rtnl_rule *rule, int table)
{
	rule->r_table = table;
	rule->r_mask |= RULE_HAS_TABLE;
}

/**
 * Set the dsfield of a rule to the specified value
 * @arg rule		rule to change
 * @arg dsfield		new dsfield value
 */
void rtnl_rule_set_dsfield(struct rtnl_rule *rule, int dsfield)
{
	rule->r_dsfield = dsfield;
	rule->r_mask |= RULE_HAS_DSFIELD;
}

/**
 * Set the source address length of a rule to the specified value
 * @arg rule		rule to change
 * @arg len		new source address length
 */
void rtnl_rule_set_src_len(struct rtnl_rule *rule, int len)
{
	rule->r_src_len = len;
	if (rule->r_mask & RULE_HAS_SRC)
		rule->r_src.a_prefix = len;
	rule->r_mask |= RULE_HAS_SRC_LEN;
}

/**
 * Set the destination address length of a rule to the specified value
 * @arg rule		rule to change
 * @arg len		new destination address length
 */
void rtnl_rule_set_dst_len(struct rtnl_rule *rule, int len)
{
	rule->r_dst_len = len;
	if (rule->r_mask & RULE_HAS_DST)
		rule->r_dst.a_prefix = len;
	rule->r_mask |= RULE_HAS_DST_LEN;
}

/**
 * Set the source address of a rule
 * @arg rule		rule to change
 * @arg src		new source address
 *
 * Assigns the new source address to the specified rule handle. The
 * address is validated against the address family if set already via
 * either rtnl_rule_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_set_src(struct rtnl_rule *rule, struct nl_addr *src)
{
	if (rule->r_mask & RULE_HAS_FAMILY) {
		if (src->a_family != rule->r_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		rule->r_family = src->a_family;
		
	memcpy(&rule->r_src, src, sizeof(rule->r_src));
	rule->r_src_len = rule->r_src.a_prefix;
	rule->r_mask |= (RULE_HAS_SRC | RULE_HAS_FAMILY | RULE_HAS_SRC_LEN);
	return 0;
}

/**
 * Set the source address of a rule
 * @arg rule		rule to change
 * @arg src		new source address as string
 *
 * Translates the specified address to a binary format and assigns
 * it as the new source address by calling rtnl_rule_set_src().
 *
 * @see rtnl_rule_set_src()
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_set_src_str(struct rtnl_rule *rule, const char *src)
{
	int err;
	struct nl_addr a = {0};
	int hint = rule->r_mask & RULE_HAS_FAMILY ? rule->r_family : AF_UNSPEC;
	
	err = nl_str2addr(src, &a, hint);
	if (err < 0)
		return err;

	return rtnl_rule_set_src(rule, &a);
}

/**
 * Set the destination address of a rule
 * @arg rule		rule to change
 * @arg dst		new destination address
 *
 * Assigns the new destination address to the specified rule handle. The
 * address is validated against the address family if set already via
 * either rtnl_rule_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_set_dst(struct rtnl_rule *rule, struct nl_addr *dst)
{
	if (rule->r_mask & RULE_HAS_FAMILY) {
		if (dst->a_family != rule->r_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		rule->r_family = dst->a_family;
		
	memcpy(&rule->r_dst, dst, sizeof(rule->r_dst));
	rule->r_dst_len = rule->r_dst.a_prefix;
	rule->r_mask |= (RULE_HAS_DST | RULE_HAS_FAMILY | RULE_HAS_DST_LEN);
	return 0;
}

/**
 * Set the destination address of a rule
 * @arg rule		rule to change
 * @arg dst		new destination address as string
 *
 * Translates the specified address to a binary format and assigns
 * it as the new destination address by calling rtnl_rule_set_dst().
 *
 * @see rtnl_rule_set_dst()
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_set_dst_str(struct rtnl_rule *rule, const char *dst)
{
	int err;
	struct nl_addr a = {0};
	int hint = rule->r_mask & RULE_HAS_FAMILY ? rule->r_family : AF_UNSPEC;
	
	err = nl_str2addr(dst, &a, hint);
	if (err < 0)
		return err;

	return rtnl_rule_set_dst(rule, &a);
}

/** @} */

struct nl_cache_ops rtnl_rule_ops = {
	.co_request_update	= rule_request_update,
	.co_msg_parser		= rule_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= rule_dump_brief,
	.co_filter		= rule_filter,
};

/** @} */

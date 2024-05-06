#ifndef SRC_USR_NL_ATTRIBUTE_H_
#define SRC_USR_NL_ATTRIBUTE_H_

#include <netlink/attr.h>
#include "common/config.h"
#include "usr/util/result.h"
#include "usr/nl/session.h"

#define foreach_entry(pos, ghdr, rem) \
	nla_for_each_attr( \
		pos, \
		genlmsg_attrdata(ghdr, sizeof(struct joolnlhdr)), \
		genlmsg_attrlen(ghdr, sizeof(struct joolnlhdr)), \
		rem \
	)

struct jool_result jnla_parse_msg(struct nl_msg *msg, struct nlattr *tb[],
		int maxtype, struct nla_policy *policy,
		bool validate_mandatories);
struct jool_result jnla_parse_nested(struct nlattr *result[], int maxtype,
		struct nlattr *root, struct nla_policy *policy);
struct jool_result jnla_validate_list(struct nlattr *head, int len,
		char const *what, struct nla_policy *policy);

struct nlattr *jnla_nest_start(struct nl_msg *msg, int attrtype);

void nla_get_addr6(struct nlattr const *attr, struct in6_addr *addr);
void nla_get_addr4(struct nlattr const *attr, struct in_addr *addr);
struct jool_result nla_get_prefix6(struct nlattr *attr, struct ipv6_prefix *out);
struct jool_result nla_get_prefix4(struct nlattr *attr, struct ipv4_prefix *out);
struct jool_result nla_get_taddr6(struct nlattr *attr, struct ipv6_transport_addr *out);
struct jool_result nla_get_taddr4(struct nlattr *attr, struct ipv4_transport_addr *out);
struct jool_result nla_get_eam(struct nlattr *attr, struct eamt_entry *out);
struct jool_result nla_get_pool4(struct nlattr *attr, struct pool4_entry *out);
struct jool_result nla_get_bib(struct nlattr *attr, struct bib_entry *out);
struct jool_result nla_get_session(struct nlattr *attr, struct session_entry_usr *out);
struct jool_result nla_get_plateaus(struct nlattr *attr, struct mtu_plateaus *out);

/*
 * Implementation notes:
 *
 * Looking at the NLA_PUT macro, it would seem the Netlink API is pretty
 * confident that the nla_put* functions will never return anything other than
 * "success" (non-negative, but in practice always 0) and "packet too small"
 * (negative, in practice -NLE_NOMEM).
 *
 * This seems short-sighted to me, but I've decided to inherit the assumption
 * for the following reasons (in increasing order of importance):
 *
 * 1. This module is essentially an extension of the Netlink API.
 * 2. I don't want to clutter the code even more with error switches all over
 *    the place.
 * 3. The Netlink contracts don't actually state that -NLE_NOMEM is the error
 *    code reserved for "packet too small," so those switches would be naive
 *    anyway. (In truth, NLE_NOMEM was an odd choice. It seems it should be
 *    NLE_MSGSIZE...)
 *
 * Therefore, don't bother over-analyzing the result code of these functions.
 * Until libnl decides to change its API, `>= 0` means success, `< 0` means
 * packet too small. No other outcomes.
 */

int nla_put_prefix6(struct nl_msg *msg, int attrtype, struct ipv6_prefix const *prefix);
int nla_put_prefix4(struct nl_msg *msg, int attrtype, struct ipv4_prefix const *prefix);
int nla_put_plateaus(struct nl_msg *msg, int attrtype, struct mtu_plateaus const *plateaus);
int nla_put_eam(struct nl_msg *msg, int attrtype, struct eamt_entry const *entry);
int nla_put_pool4(struct nl_msg *msg, int attrtype, struct pool4_entry const *entry);
int nla_put_bib(struct nl_msg *msg, int attrtype, struct bib_entry const *entry);
int nla_put_bib_attrs(struct nl_msg *msg, int attrtype,
		struct ipv6_transport_addr const *addr6,
		struct ipv4_transport_addr const *addr4,
		l4_protocol proto,
		bool is_static);
int nla_put_session(struct nl_msg *msg, int attrtype, struct session_entry_usr const *entry);

#endif /* SRC_USR_NL_ATTRIBUTE_H_ */

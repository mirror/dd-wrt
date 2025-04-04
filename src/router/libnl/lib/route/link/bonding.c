/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2011-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup bonding Bonding
 *
 * @details
 * \b Link Type Name: "bond"
 *
 * @route_doc{link_bonding, Bonding Documentation}
 * @{
 */

#include "nl-default.h"

#include <netlink/netlink.h>
#include <netlink/route/link/bonding.h>

#include "nl-route.h"
#include "link-api.h"

#define BOND_HAS_MODE		(1 << 0)
#define BOND_HAS_ACTIVE_SLAVE	(1 << 1)

struct bond_info {
	uint8_t bn_mode;
	uint32_t ifindex;
	uint32_t bn_mask;
};

static int bond_info_alloc(struct rtnl_link *link)
{
	struct bond_info *bn;

	if (link->l_info)
		memset(link->l_info, 0, sizeof(*bn));
	else {
		bn = calloc(1, sizeof(*bn));
		if (!bn)
			return -NLE_NOMEM;

		link->l_info = bn;
	}

	return 0;
}

static void bond_info_free(struct rtnl_link *link)
{
	_nl_clear_free(&link->l_info);
}

static int bond_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct bond_info *bn = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;
	if (bn->bn_mask & BOND_HAS_MODE)
		NLA_PUT_U8(msg, IFLA_BOND_MODE, bn->bn_mode);

	if (bn->bn_mask & BOND_HAS_ACTIVE_SLAVE)
		NLA_PUT_U32(msg, IFLA_BOND_ACTIVE_SLAVE, bn->ifindex);

	nla_nest_end(msg, data);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, data);
	return -NLE_MSGSIZE;
}

static struct rtnl_link_info_ops bonding_info_ops = {
	.io_name		= "bond",
	.io_alloc		= bond_info_alloc,
	.io_put_attrs		= bond_put_attrs,
	.io_free		= bond_info_free,
};

#define IS_BOND_INFO_ASSERT(link)                                                    \
	do {                                                                         \
		if (link->l_info_ops != &bonding_info_ops) {                         \
			APPBUG("Link is not a bond link. Set type \"bond\" first."); \
		}                                                                    \
	} while (0)

/**
 * Set active slave for bond
 * @arg link            Link object of type bond
 * @arg active          ifindex of active slave to set
 *
 * @return void
 */
void rtnl_link_bond_set_activeslave(struct rtnl_link *link, int active_slave)
{
	struct bond_info *bn = link->l_info;

	IS_BOND_INFO_ASSERT(link);

	bn->ifindex = active_slave;

	bn->bn_mask |= BOND_HAS_ACTIVE_SLAVE;
}

/**
 * Set bond mode
 * @arg link            Link object of type bond
 * @arg mode            bond mode to set
 *
 * @return void
 */
void rtnl_link_bond_set_mode(struct rtnl_link *link, uint8_t mode)
{
	struct bond_info *bn = link->l_info;

	IS_BOND_INFO_ASSERT(link);

	bn->bn_mode = mode;

	bn->bn_mask |= BOND_HAS_MODE;
}

/**
 * Allocate link object of type bond
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_bond_alloc(void)
{
	struct rtnl_link *link;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if (rtnl_link_set_type(link, "bond") < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Create a new kernel bonding device
 * @arg sock		netlink socket
 * @arg name		name of bonding device or NULL
 * @arg opts		bonding options (currently unused)
 *
 * Creates a new bonding device in the kernel. If no name is
 * provided, the kernel will automatically pick a name of the
 * form "type%d" (e.g. bond0, vlan1, etc.)
 *
 * The \a opts argument is currently unused. In the future, it
 * may be used to carry additional bonding options to be set
 * when creating the bonding device.
 *
 * @note When letting the kernel assign a name, it will become
 *       difficult to retrieve the interface afterwards because
 *       you have to guess the name the kernel has chosen. It is
 *       therefore not recommended to not provide a device name.
 *
 * @see rtnl_link_bond_enslave()
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_bond_add(struct nl_sock *sock, const char *name,
		       struct rtnl_link *opts)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_bond_alloc()))
		return -NLE_NOMEM;

	if (!name && opts)
		name = rtnl_link_get_name(opts);

	if (name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sock, link, NLM_F_CREATE);

	rtnl_link_put(link);

	return err;
}

/**
 * Add a link to a bond (enslave)
 * @arg sock		netlink socket
 * @arg master		ifindex of bonding master
 * @arg slave		ifindex of slave link to add to bond
 *
 * This function is identical to rtnl_link_bond_enslave() except that
 * it takes interface indices instead of rtnl_link objcets.
 *
 * @see rtnl_link_bond_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_enslave_ifindex(struct nl_sock *sock, int master,
				   int slave)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_bond_alloc()))
		return -NLE_NOMEM;

	rtnl_link_set_ifindex(link, slave);
	rtnl_link_set_master(link, master);
	
	if ((err = rtnl_link_change(sock, link, link, 0)) < 0)
		goto errout;

	rtnl_link_put(link);

	/*
	 * Due to the kernel not signaling whether this opertion is
	 * supported or not, we will retrieve the attribute to see  if the
	 * request was successful. If the master assigned remains unchanged
	 * we will return NLE_OPNOTSUPP to allow performing backwards
	 * compatibility of some sort.
	 */
	if ((err = rtnl_link_get_kernel(sock, slave, NULL, &link)) < 0)
		return err;

	if (rtnl_link_get_master(link) != master)
		err = -NLE_OPNOTSUPP;

errout:
	rtnl_link_put(link);

	return err;
}

/**
 * Add a link to a bond (enslave)
 * @arg sock		netlink socket
 * @arg master		bonding master
 * @arg slave		slave link to add to bond
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message adding the slave to
 * the master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_bond_enslave_ifindex()
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_enslave(struct nl_sock *sock, struct rtnl_link *master,
			   struct rtnl_link *slave)
{
	return rtnl_link_bond_enslave_ifindex(sock,
				rtnl_link_get_ifindex(master),
				rtnl_link_get_ifindex(slave));
}

/**
 * Release a link from a bond
 * @arg sock		netlink socket
 * @arg slave		slave link to be released
 *
 * This function is identical to rtnl_link_bond_release() except that
 * it takes an interface index instead of a rtnl_link object.
 *
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_release_ifindex(struct nl_sock *sock, int slave)
{
	return rtnl_link_bond_enslave_ifindex(sock, 0, slave);
}

/**
 * Release a link from a bond
 * @arg sock		netlink socket
 * @arg slave		slave link to be released
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message releasing the slave from
 * its master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_bond_release_ifindex()
 * @see rtnl_link_bond_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_release(struct nl_sock *sock, struct rtnl_link *slave)
{
	return rtnl_link_bond_release_ifindex(sock,
				rtnl_link_get_ifindex(slave));
}

static void _nl_init bonding_init(void)
{
	rtnl_link_register_info(&bonding_info_ops);
}

static void _nl_exit bonding_exit(void)
{
	rtnl_link_unregister_info(&bonding_info_ops);
}

/** @} */

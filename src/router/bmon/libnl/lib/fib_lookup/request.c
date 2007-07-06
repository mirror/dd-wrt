/*
 * lib/fib_lookup/request.c	FIB Lookup Request
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup fib_lookup
 * @defgroup flreq Request
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/fib_lookup/request.h>

/** @cond SKIP */
#define REQUEST_ATTR_ADDR	0x01
#define REQUEST_ATTR_FWMARK	0x02
#define REQUEST_ATTR_TOS	0x04
#define REQUEST_ATTR_SCOPE	0x08
#define REQUEST_ATTR_TABLE	0x10
/** @endcond */

/**
 * @name Lookup Request Creation/Deletion
 * @{
 */

/**
 * Allocate and initialize new lookup request object.
 * @note Free the memory after usage using flnl_request_put() or
 *       flnl_request_free().
 * @return Newly allocated lookup request object or NULL if an error occured.
 */
struct flnl_request *flnl_request_alloc(void)
{
	struct flnl_request *req;

	req = calloc(1, sizeof(*req));
	if (req)
		req->lr_refcnt = 1;
	else
		nl_errno(ENOMEM);

	return req;
}

/**
 * Request undestroyable reference of lookup request object.
 * @arg req		Lookup request object.
 * @return Lookup request object of which the reference was given.
 */
struct flnl_request *flnl_request_get(struct flnl_request *req)
{
	req->lr_refcnt++;

	return req;
}

/**
 * Give back reference of lookup request object.
 * @arg req		Lookup request object to be given back.
 * 
 * Decrements the reference counter and destroys the object if the
 * last reference was given back.
 */
void flnl_request_put(struct flnl_request *req)
{
	if (!req)
		return;

	if (req->lr_refcnt <= 1)
		flnl_request_free(req);
	else
		req->lr_refcnt--;
}

/**
 * Free lookup request object.
 * @arg req		Lookup request object to be freed.
 */
void flnl_request_free(struct flnl_request *req)
{
	if (!req)
		return;

	if (req->lr_refcnt != 1)
		BUG();

	free(req);
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set firewall mark of lookup request object.
 * @arg req		Lookup request object.
 * @arg fwmark		Firewall mark.
 */
void flnl_request_set_fwmark(struct flnl_request *req, uint64_t fwmark)
{
	req->lr_fwmark = fwmark;
	req->lr_mask |= REQUEST_ATTR_FWMARK;
}

/**
 * Get firewall mark of lookup request object.
 * @arg req		Lookup request object.
 * @return Firewall mark or UINT_LEAST64_MAX if not available.
 */
uint64_t flnl_request_get_fwmark(struct flnl_request *req)
{
	if (req->lr_mask & REQUEST_ATTR_FWMARK)
		return req->lr_fwmark;
	else
		return UINT_LEAST64_MAX;
}

/**
 * Set Type of Service of lookup request object.
 * @arg req		Lookup request object.
 * @arg tos		Type of Service.
 */
void flnl_request_set_tos(struct flnl_request *req, int tos)
{
	req->lr_tos = tos;
	req->lr_mask |= REQUEST_ATTR_TOS;
}

/**
 * Get Type of Service of lookup request object.
 * @arg req		Lookup request object.
 * @return Type of Service or -1 if not available.
 */
int flnl_request_get_tos(struct flnl_request *req)
{
	if (req->lr_mask & REQUEST_ATTR_TOS)
		return req->lr_tos;
	else
		return -1;
}

/**
 * Set Scope of lookup request object.
 * @arg req		Lookup request oject.
 * @arg scope		Routing scope.
 */
void flnl_request_set_scope(struct flnl_request *req, int scope)
{
	req->lr_scope = scope;
	req->lr_mask |= REQUEST_ATTR_SCOPE;
}

/**
 * Get scope of lookup request object.
 * @arg req		Lookup request object.
 * @return Scope or -1 if not available.
 */
int flnl_request_get_scope(struct flnl_request *req)
{
	if (req->lr_mask & REQUEST_ATTR_SCOPE)
		return req->lr_scope;
	else
		return -1;
}

/**
 * Set routing table of lookup request object.
 * @arg req		Lookup request object.
 * @arg table		Routing table.
 */
void flnl_request_set_table(struct flnl_request *req, int table)
{
	req->lr_table = table;
	req->lr_mask |= REQUEST_ATTR_TABLE;
}

/**
 * Get routing table of lookup request object.
 * @arg req		Lookup request object.
 * @return Routing table or -1 if not available.
 */
int flnl_request_get_table(struct flnl_request *req)
{
	if (req->lr_mask & REQUEST_ATTR_TABLE)
		return req->lr_table;
	else
		return -1;
}

/**
 * Set destination address of lookup request object.
 * @arg req		Lookup request object.
 * @arg addr		IPv4 destination address.
 */
int flnl_request_set_addr(struct flnl_request *req, struct nl_addr *addr)
{
	if (addr->a_family != AF_INET)
		return nl_error(EINVAL, "Address must be an IPv4 address");

	if (req->lr_addr)
		nl_addr_put(req->lr_addr);

	nl_addr_get(addr);
	req->lr_addr = addr;

	req->lr_mask |= REQUEST_ATTR_ADDR;

	return 0;
}

/**
 * Get destination address of lookup request object.
 * @arg req		Lookup request object.
 * @return Destination address or NULL if not available.
 */
struct nl_addr *flnl_request_get_addr(struct flnl_request *req)
{
	if (req->lr_mask & REQUEST_ATTR_ADDR)
		return req->lr_addr;
	else
		return NULL;
}

/** @} */

/**
 * @name Miscellaneous
 * @{
 */

/**
 * Compares two lookup request objects.
 * @arg a		Lookup request object.
 * @arg b		Another lookup request object.
 *
 * @return Integer less than, equal to or greather than zero if \c is found,
 *         respectively to be less than, to, or be greater than \c b.
 */
int flnl_request_cmp(struct flnl_request *a, struct flnl_request *b)
{
#define REQ(F) (a->lr_mask & REQUEST_ATTR_##F)
#define AVAIL(F) (b->lr_mask & REQUEST_ATTR_##F)
#define F_CUS(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define F_INT(F, N) (REQ(F) && (!AVAIL(F) || (a->N != b->N)))

	if (F_INT(FWMARK,	lr_fwmark)				||
	    F_INT(TOS,		lr_tos)					||
	    F_INT(SCOPE,	lr_scope)				||
	    F_INT(TABLE,	lr_table)				||
	    F_CUS(ADDR,		nl_addr_cmp(a->lr_addr, b->lr_addr)))
		return 0;
#undef REQ
#undef AVAIL
#undef F_CUS
#undef F_INT
	return 1;
}

/** @} */

/** @} */

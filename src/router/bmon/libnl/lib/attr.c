/*
 * lib/attr.c		Netlink Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <linux/socket.h>

/**
 * @ingroup nl
 * @defgroup attr Attributes
 * Netlink Attributes Construction/Parsing Interface
 * @par 0) Introduction
 * Netlink attributes are chained together following each other:
 * @code
 *    <------- nla_total_size(payload) ------->
 *    <---- nla_attr_size(payload) ----->
 *   +----------+- - -+- - - - - - - - - +- - -+-------- - -
 *   |  Header  | Pad |     Payload      | Pad |  Header
 *   +----------+- - -+- - - - - - - - - +- - -+-------- - -
 *                     <- nla_len(nla) ->      ^
 *   nla_data(nla)----^                        |
 *   nla_next(nla)-----------------------------'
 * @endcode
 *
 * @par
 * The attribute header and payload must be aligned properly:
 * @code
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 * @endcode
 *
 * @par Nested TLVs:
 * Nested TLVs are an array of TLVs nested into another TLV. This can be useful
 * to allow subsystems to have their own formatting rules without the need to
 * make the underlying layer be aware of it. It can also be useful to transfer
 * arrays, lists and flattened trees.
 * \code
 *  <-------------------- NLA_ALIGN(...) ------------------->
 * +---------------+- - - - - - - - - - - - - - - - - -+- - -+
 * |               |+---------+---------+- - -+-------+|     |
 * |  TLV Header   ||  TLV 1  |  TLV 2  |     | TLV n || Pad |
 * |               |+---------+---------+- - -+-------+|     |
 * +---------------+- - - - - - - - - - - - - - - - - -+- - -+
 *                  <--------- nla_data(nla) --------->
 * \endcode
 *
 * @par 1) Constructing a message with attributes
 * @code
 * int param1 = 10;
 * char *param2 = "parameter text";
 * struct nlmsghdr hdr = {
 * 	.nlmsg_type = MY_ACTION,
 * };
 * struct nl_msg *m = nlmsg_build(&hdr);
 * nla_put_u32(m, 1, param1);
 * nla_put_string(m, 2, param2);
 * 
 * nl_send_auto_complete(handle, nl_msg_get(m));
 * nlmsg_free(m);
 * @endcode
 *
 * @par 2) Constructing nested attributes
 * @code
 * struct nl_msg * nested_config(void)
 * {
 * 	int a = 5, int b = 10;
 * 	struct nl_msg *n = nlmsg_build(NULL);
 * 	nla_put_u32(n, 10, a);
 * 	nla_put_u32(n, 20, b);
 * 	return n;
 * }
 *
 * ...
 * struct nl_msg *m = nlmsg_build(&hdr);
 * struct nl_msg *nest = nested_config();
 * nla_put_nested(m, 1, nest);
 *
 * nl_send_auto_complete(handle, nl_msg_get(m));
 * nlmsg_free(nest);
 * nlmsg_free(m);
 * @endcode
 * @{
 */

/**
 * @name Size Calculations
 * @{
 */

/**
 * length of attribute not including padding
 * @arg payload		length of payload
 */
int nla_attr_size(int payload)
{
	return NLA_HDRLEN + payload;
}

/**
 * total length of attribute including padding
 * @arg payload		length of payload
 */
int nla_total_size(int payload)
{
	return NLA_ALIGN(nla_attr_size(payload));
}

/**
 * length of padding at the tail of the attribute
 * @arg payload		length of payload
 */
int nla_padlen(int payload)
{
	return nla_total_size(payload) - nla_attr_size(payload);
}

/** @} */

/**
 * @name Payload Access
 * @{
 */

/**
 * head of payload
 * @arg nla		netlink attribute
 */
void *nla_data(const struct nlattr *nla)
{
	return (char *) nla + NLA_HDRLEN;
}

/**
 * length of payload
 * @arg nla		netlink attribute
 */
int nla_len(const struct nlattr *nla)
{
	return nla->nla_len - NLA_HDRLEN;
}

/** @} */

/**
 * @name Attribute Parsing
 * @{
 */

/**
 * check if the netlink attribute fits into the remaining bytes
 * @arg nla		netlink attribute
 * @arg remaining	number of bytes remaining in attribute stream
 */
int nla_ok(const struct nlattr *nla, int remaining)
{
	return remaining >= sizeof(*nla) &&
	       nla->nla_len >= sizeof(*nla) &&
	       nla->nla_len <= remaining;
}

/**
 * next netlink attribte in attribute stream
 * @arg nla		netlink attribute
 * @arg remaining	number of bytes remaining in attribute stream
 *
 * @return the next netlink attribute in the attribute stream and
 * decrements remaining by the size of the current attribute.
 */
struct nlattr *nla_next(const struct nlattr *nla, int *remaining)
{
	int totlen = NLA_ALIGN(nla->nla_len);

	*remaining -= totlen;
	return (struct nlattr *) ((char *) nla + totlen);
}

static uint16_t nla_attr_minlen[NLA_TYPE_MAX+1] = {
	[NLA_U8]	= sizeof(uint8_t),
	[NLA_U16]	= sizeof(uint16_t),
	[NLA_U32]	= sizeof(uint32_t),
	[NLA_U64]	= sizeof(uint64_t),
	[NLA_STRING]	= 1,
	[NLA_NESTED]	= NLA_HDRLEN,
};

static int validate_nla(struct nlattr *nla, int maxtype,
			struct nla_policy *policy)
{
	struct nla_policy *pt;
	int minlen = 0;

	if (nla->nla_type <= 0 || nla->nla_type > maxtype)
		return 0;

	pt = &policy[nla->nla_type];

	if (pt->type > NLA_TYPE_MAX)
		BUG();

	if (pt->minlen)
		minlen = pt->minlen;
	else if (pt->type != NLA_UNSPEC)
		minlen = nla_attr_minlen[pt->type];

	if (pt->type == NLA_FLAG && nla_len(nla) > 0)
		return -ERANGE;

	if (nla_len(nla) < minlen)
		return -ERANGE;

	if (pt->maxlen && nla_len(nla) > pt->maxlen)
		return -ERANGE;

	if (pt->type == NLA_STRING) {
		char *data = nla_data(nla);
		if (data[nla_len(nla) - 1] != '\0')
			return -EINVAL;
	}

	return 0;
}


/**
 * Parse a stream of attributes into a tb buffer
 * @arg tb		destination array with maxtype+1 elements
 * @arg maxtype		maximum attribute type to be expected
 * @arg head		head of attribute stream
 * @arg len		length of attribute stream
 * @arg policy		validation policy
 *
 * Parses a stream of attributes and stores a pointer to each attribute in
 * the tb array accessable via the attribute type. Attributes with a type
 * exceeding maxtype will be silently ignored for backwards compatibility
 * reasons. policy may be set to NULL if no validation is required.
 *
 * @return 0 on success or a negative error code.
 */
int nla_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
	      struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	memset(tb, 0, sizeof(struct nlattr *) * (maxtype + 1));

	nla_for_each_attr(nla, head, len, rem) {
		uint16_t type = nla->nla_type;

		if (type == 0) {
			fprintf(stderr, "Illegal nla->nla_type == 0\n");
			continue;
		}

		if (type <= maxtype) {
			if (policy) {
				err = validate_nla(nla, maxtype, policy);
				if (err < 0)
					goto errout;
			}

			tb[type] = nla;
		}
	}

	if (rem > 0)
		fprintf(stderr, "netlink: %d bytes leftover after parsing "
		       "attributes.\n", rem);

	err = 0;
errout:
	return err;
}


/**
 * parse nested attributes
 * @arg tb		destination array with maxtype+1 elements
 * @arg maxtype		maximum attribute type to be expected
 * @arg nla		attribute containing the nested attributes
 * @arg policy		validation policy
 *
 * @see nla_parse()
 */
int nla_parse_nested(struct nlattr *tb[], int maxtype, struct nlattr *nla,
		     struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, nla_data(nla), nla_len(nla), policy);
}

/**
 * Validate a stream of attributes
 * @arg head		head of attribute stream
 * @arg len		length of attribute stream
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 *
 * Validates all attributes in the specified attribute stream
 * against the specified policy. Attributes with a type exceeding
 * maxtype will be ignored. See documenation of struct nla_policy
 * for more details.
 *
 * @return 0 on success or a negative error code.
 */
int nla_validate(struct nlattr *head, int len, int maxtype,
		 struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	nla_for_each_attr(nla, head, len, rem) {
		err = validate_nla(nla, maxtype, policy);
		if (err < 0)
			goto errout;
	}

	err = 0;
errout:
	return err;
}

/**
 * Find a specific attribute in a stream of attributes
 * @arg head		head of attribute stream
 * @arg len		length of attribute stream
 * @arg attrtype	type of attribute to look for
 *
 * @return the first attribute in the stream matching the specified type.
 */
struct nlattr *nla_find(struct nlattr *head, int len, int attrtype)
{
	struct nlattr *nla;
	int rem;

	nla_for_each_attr(nla, head, len, rem)
		if (nla->nla_type == attrtype)
			return nla;

	return NULL;
}

/** @} */

/**
 * @name Utilities
 * @{
 */

/**
 * Copy a netlink attribute into another memory area
 * @arg dest		where to copy to memcpy
 * @arg src		netlink attribute to copy from
 * @arg count		size of the destination area
 *
 * Note: The number of bytes copied is limited by the length of
 *       attribute's payload. memcpy
 *
 * @return the number of bytes copied.
 */
int nla_memcpy(void *dest, struct nlattr *src, int count)
{
	int minlen;

	if (!src)
		return 0;
	
	minlen = min_t(int, count, nla_len(src));
	memcpy(dest, nla_data(src), minlen);

	return minlen;
}

/**
 * Copy string attribute payload into a sized buffer
 * @arg dst		where to copy the string to
 * @arg nla		attribute to copy the string from
 * @arg dstsize		size of destination buffer
 *
 * Copies at most dstsize - 1 bytes into the destination buffer.
 * The result is always a valid NUL-terminated string. Unlike
 * strlcpy the destination buffer is always padded out.
 *
 * @return the length of the source buffer.
 */
size_t nla_strlcpy(char *dst, const struct nlattr *nla, size_t dstsize)
{
	size_t srclen = nla_len(nla);
	char *src = nla_data(nla);

	if (srclen > 0 && src[srclen - 1] == '\0')
		srclen--;

	if (dstsize > 0) {
		size_t len = (srclen >= dstsize) ? dstsize - 1 : srclen;

		memset(dst, 0, dstsize);
		memcpy(dst, src, len);
	}

	return srclen;
}

/**
 * Compare an attribute with sized memory area
 * @arg nla		netlink attribute
 * @arg data		memory area
 * @arg size		size of memory area
 */
int nla_memcmp(const struct nlattr *nla, const void *data,
			     size_t size)
{
	int d = nla_len(nla) - size;

	if (d == 0)
		d = memcmp(nla_data(nla), data, size);

	return d;
}

/**
 * Compare a string attribute against a string
 * @arg nla		netlink string attribute
 * @arg str		another string
 */
int nla_strcmp(const struct nlattr *nla, const char *str)
{
	int len = strlen(str) + 1;
	int d = nla_len(nla) - len;

	if (d == 0)
		d = memcmp(nla_data(nla), str, len);

	return d;
}

/** @} */

/**
 * @name Attribute Construction
 * @{
 */

/**
 * reserve room for attribute on the skb
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg attrlen		length of attribute payload
 *
 * Adds a netlink attribute header to a netlink message and reserves
 * room for the payload but does not copy it.
 */
struct nlattr *nla_reserve(struct nl_msg *n, int attrtype, int attrlen)
{
	struct nlattr *nla;
	int tlen;
	
	tlen = NLMSG_ALIGN(n->nm_nlh->nlmsg_len) + nla_total_size(attrlen);

	n->nm_nlh = realloc(n->nm_nlh, tlen);
	if (!n->nm_nlh) {
		nl_errno(ENOMEM);
		return NULL;
	}

	nla = (struct nlattr *) nlmsg_tail(n->nm_nlh);
	nla->nla_type = attrtype;
	nla->nla_len = nla_attr_size(attrlen);

	memset((unsigned char *) nla + nla->nla_len, 0, nla_padlen(attrlen));
	n->nm_nlh->nlmsg_len = tlen;

	return nla;
}

/**
 * Add a netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg attrlen		length of attribute payload
 * @arg data		head of attribute payload
 *
 * @return -1 if the tailroom of the skb is insufficient to store
 * the attribute header and payload.
 */
int nla_put(struct nl_msg *n, int attrtype, int attrlen, const void *data)
{
	struct nlattr *nla;

	nla = nla_reserve(n, attrtype, attrlen);
	if (!nla)
		return nl_errno(ENOMEM);

	memcpy(nla_data(nla), data, attrlen);

	return 0;
}

/**
 * Add a nested netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg nested		netlink attribute to nest
 *
 * @return -1 if the tailroom of the skb is insufficient to store
 * the attribute header and payload.
 */
int nla_put_nested(struct nl_msg *n, int attrtype, struct nl_msg *nested)
{
	return nla_put(n, attrtype, nlmsg_len(nested->nm_nlh),
		       nlmsg_data(nested->nm_nlh));
}

/**
 * Add a u16 netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg value		numeric value
 */
int nla_put_u8(struct nl_msg *n, int attrtype, uint8_t value)
{
	return nla_put(n, attrtype, sizeof(uint8_t), &value);
}

/**
 * Add a u16 netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg value		numeric value
 */
int nla_put_u16(struct nl_msg *n, int attrtype, uint16_t value)
{
	return nla_put(n, attrtype, sizeof(uint16_t), &value);
}

/**
 * Add a u32 netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg value		numeric value
 */
int nla_put_u32(struct nl_msg *n, int attrtype, uint32_t value)
{
	return nla_put(n, attrtype, sizeof(uint32_t), &value);
}

/**
 * Add a u64 netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg value		numeric value
 */
int nla_put_u64(struct nl_msg *n, int attrtype, uint64_t value)
{
	return nla_put(n, attrtype, sizeof(uint64_t), &value);
}

/**
 * Add a string netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg str		NUL terminated string
 */
int nla_put_string(struct nl_msg *n, int attrtype, const char *str)
{
	return nla_put(n, attrtype, strlen(str) + 1, str);
}

/**
 * Add a flag netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 */
int nla_put_flag(struct nl_msg *n, int attrtype)
{
	return nla_put(n, attrtype, 0, NULL);
}

/**
 * Add a msecs netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg msecs 		number of msecs
 */
int nla_put_msecs(struct nl_msg *n, int attrtype, unsigned long msecs)
{
	return nla_put_u64(n, attrtype, msecs);
}

/**
 * Add an abstract data netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg data		abstract data
 */
int nla_put_data(struct nl_msg *n, int attrtype, struct nl_data *data)
{
	return nla_put(n, attrtype, nl_data_get_size(data),
		       nl_data_get(data));
}

/**
 * Add an abstract address netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg addr		abstract address
 */
int nla_put_addr(struct nl_msg *n, int attrtype, struct nl_addr *addr)
{
	return nla_put(n, attrtype, nl_addr_get_len(addr),
		       nl_addr_get_binary_addr(addr));
}

/** @} */

/**
 * @name Attribute Nesting
 * @{
 */

/**
 * Start a new level of nested attributes
 * @arg n		netlink message
 * @arg attrtype	attribute type of container
 *
 * @return the container attribute
 */
struct nlattr *nla_nest_start(struct nl_msg *n, int attrtype)
{
	struct nlattr *start = (struct nlattr *) nlmsg_tail(n->nm_nlh);

	if (nla_put(n, attrtype, 0, NULL) < 0)
		return NULL;

	return start;
}

/**
 * Finalize nesting of attributes
 * @arg n		netlink message
 * @arg start		container attribute
 *
 * Corrects the container attribute header to include the all
 * appeneded attributes.
 *
 * @return the total data length of the skb.
 */
int nla_nest_end(struct nl_msg *n, struct nlattr *start)
{
	start->nla_len = (unsigned char *) nlmsg_tail(n->nm_nlh) -
				(unsigned char *) start;
	return 0;
}

/** @} */

/**
 * @name Attribute Reading
 * @{
 */

/**
 * Return payload of u32 attribute
 * @arg nla		u32 netlink attribute
 */
uint32_t nla_get_u32(struct nlattr *nla)
{
	return *(uint32_t *) nla_data(nla);
}

/**
 * Return payload of u16 attribute
 * @arg nla		u16 netlink attribute
 */
uint16_t nla_get_u16(struct nlattr *nla)
{
	return *(uint16_t *) nla_data(nla);
}

/**
 * Return payload of u8 attribute
 * @arg nla		u8 netlink attribute
 */
uint8_t nla_get_u8(struct nlattr *nla)
{
	return *(uint8_t *) nla_data(nla);
}

/**
 * Return payload of u64 attribute
 * @arg nla		u64 netlink attribute
 */
uint64_t nla_get_u64(struct nlattr *nla)
{
	uint64_t tmp;

	nla_memcpy(&tmp, nla, sizeof(tmp));

	return tmp;
}

/**
 * Return payload of flag attribute
 * @arg nla		flag netlink attribute
 */
int nla_get_flag(struct nlattr *nla)
{
	return !!nla;
}

/**
 * Return payload of msecs attribute
 * @arg nla		msecs netlink attribute
 *
 * @return the number of milliseconds.
 */
unsigned long nla_get_msecs(struct nlattr *nla)
{
	return nla_get_u64(nla);
}

/**
 * Return payload of address attribute
 * @arg nla		address netlink attribute
 * @arg family		address family
 *
 * @return Newly allocated address handle or NULL
 */
struct nl_addr *nla_get_addr(struct nlattr *nla, int family)
{
	return nl_addr_build(family, nla_data(nla), nla_len(nla));
}

/**
 * Return payload of abstract data attribute
 * @arg nla		abstract data netlink attribute
 *
 * @return Newly allocated abstract data handle or NULL
 */
struct nl_data *nla_get_data(struct nlattr *nla)
{
	return nl_data_alloc(nla_data(nla), nla_len(nla));
}

/** @} */

/** @} */

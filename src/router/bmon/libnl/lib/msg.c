/*
 * lib/msg.c		Netlink Messages Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup nl
 * @defgroup msg Messages
 * Netlink Message Construction/Parsing Interface
 * 
 * The following information is partly extracted from RFC3549
 * (ftp://ftp.rfc-editor.org/in-notes/rfc3549.txt)
 *
 * @par Message Format
 * Netlink messages consist of a byte stream with one or multiple
 * Netlink headers and an associated payload.  If the payload is too big
 * to fit into a single message it, can be split over multiple Netlink
 * messages, collectively called a multipart message.  For multipart
 * messages, the first and all following headers have the \c NLM_F_MULTI
 * Netlink header flag set, except for the last header which has the
 * Netlink header type \c NLMSG_DONE.
 *
 * @par
 * The Netlink message header (\link nlmsghdr struct nlmsghdr\endlink) is shown below.
 * @code   
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Length                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |            Type              |           Flags              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Sequence Number                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Process ID (PID)                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * @par
 * The netlink message header and payload must be aligned properly:
 * @code
 *  <------- NLMSG_ALIGN(hlen) ------> <---- NLMSG_ALIGN(len) --->
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * |           Header           | Pad |       Payload       | Pad |
 * |      struct nlmsghdr       |     |                     |     |
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * @endcode
 * @par
 * Message Format:
 * @code
 *    <--- nlmsg_total_size(payload)  --->
 *    <-- nlmsg_msg_size(payload) ->
 *   +----------+- - -+-------------+- - -+-------- - -
 *   | nlmsghdr | Pad |   Payload   | Pad | nlmsghdr
 *   +----------+- - -+-------------+- - -+-------- - -
 *   nlmsg_data(nlh)---^                   ^
 *   nlmsg_next(nlh)-----------------------+
 * @endcode
 * @par
 * The payload may consist of arbitary data but may have strict
 * alignment and formatting rules depening on the specific netlink
 * families.
 * @par
 * @code
 *    <---------------------- nlmsg_len(nlh) --------------------->
 *    <------ hdrlen ------>       <- nlmsg_attrlen(nlh, hdrlen) ->
 *   +----------------------+- - -+--------------------------------+
 *   |     Family Header    | Pad |           Attributes           |
 *   +----------------------+- - -+--------------------------------+
 *   nlmsg_attrdata(nlh, hdrlen)---^
 * @endcode
 * @par The ACK Netlink Message
 * This message is actually used to denote both an ACK and a NACK.
 * Typically, the direction is from FEC to CPC (in response to an ACK
 * request message).  However, the CPC should be able to send ACKs back
 * to FEC when requested.
 * @code
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Netlink message header                  |
 * |                       type = NLMSG_ERROR                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Error code                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       OLD Netlink message header              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * @par 1) Creating a new netlink message
 * @code
 * // The most common way to start creating a message is by providing an
 * // defined netlink header to nlmsg_build():
 * struct nlmsghdr hdr = {
 * 	.nlmsg_type = MY_TYPE,
 * 	.nlmsg_flags = MY_FLAGS,
 * };
 * struct nl_msg *msg = nlmsg_build(&hdr);
 *
 * // For simple usages where only the message type and flags is of
 * // interenst a shortcut can be taken:
 * struct nl_msg *msg = nlmsg_build_simple(MY_TYPE, MY_FLAGS);
 *
 * // When using a headerless message for creating nested attributes
 * // the header is not required and nlmsg_build_no_hdr() may be used:
 * struct nl_msg *msg = nlmsg_build_no_hdr();
 *
 * // The header can later be retrieved with nlmsg_hdr() and changed again:
 * nlmsg_hdr(msg)->nlmsg_flags |= YET_ANOTHER_FLAG;
 * @endcode
 *
 * @par 2) Appending data to the message
 * @code
 * // Payload may be added to the message via nlmsg_append(). The fourth
 * // parameter specifies whether to pad up to NLMSG_ALIGN to make sure
 * // that a possible further data block is properly aligned.
 * nlmsg_append(msg, &mydata, sizeof(mydata), 0);
 * @endcode
 *
 * @par 3) Cleaning up message construction
 * @code
 * // After successful use of the message, the memory must be freed
 * // using nlmsg_free()
 * nlmsg_free(msg);
 * @endcode
 * 
 * @par Example 2 (Parsing messages):
 * @code
 * int n;
 * unsigned char *buf;
 * struct nlmsghdr *hdr;
 *
 * n = nl_recv(handle, NULL, &buf);
 * 
 * hdr = (struct nlmsghdr *) buf;
 * while (nlmsg_ok(hdr, n)) {
 * 	// Process message here...
 * 	hdr = nlmsg_next(hdr, &n);
 * }
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/cache.h>
#include <netlink/attr.h>
#include <linux/socket.h>

/**
 * @name Size Calculations
 * @{
 */

/**
 * length of netlink message not including padding
 * @arg payload		length of message payload
 */
int nlmsg_msg_size(int payload)
{
	return NLMSG_HDRLEN + payload;
}

/**
 * length of netlink message including padding
 * @arg payload		length of message payload
 */
int nlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(nlmsg_msg_size(payload));
}

/**
 * length of padding at the message's tail
 * @arg payload		length of message payload
 */
int nlmsg_padlen(int payload)
{
	return nlmsg_total_size(payload) - nlmsg_msg_size(payload);
}

/** @} */

/**
 * @name Payload Access
 * @{
 */

/**
 * head of message payload
 * @arg nlh		netlink messsage header
 */
void *nlmsg_data(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_HDRLEN;
}

void *nlmsg_tail(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_ALIGN(nlh->nlmsg_len);
}

/**
 * length of message payload
 * @arg nlh		netlink message header
 */
int nlmsg_len(const struct nlmsghdr *nlh)
{
	return nlh->nlmsg_len - NLMSG_HDRLEN;
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

/**
 * head of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
struct nlattr *nlmsg_attrdata(const struct nlmsghdr *nlh, int hdrlen)
{
	unsigned char *data = nlmsg_data(nlh);
	return (struct nlattr *) (data + NLMSG_ALIGN(hdrlen));
}

/**
 * length of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
int nlmsg_attrlen(const struct nlmsghdr *nlh, int hdrlen)
{
	return nlmsg_len(nlh) - NLMSG_ALIGN(hdrlen);
}

/** @} */

/**
 * @name Message Parsing
 * @{
 */

/**
 * check if the netlink message fits into the remaining bytes
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 */
int nlmsg_ok(const struct nlmsghdr *nlh, int remaining)
{
	return (remaining >= sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len <= remaining);
}

/**
 * next netlink message in message stream
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 *
 * @returns the next netlink message in the message stream and
 * decrements remaining by the size of the current message.
 */
struct nlmsghdr *nlmsg_next(struct nlmsghdr *nlh, int *remaining)
{
	int totlen = NLMSG_ALIGN(nlh->nlmsg_len);

	*remaining -= totlen;

	return (struct nlmsghdr *) ((unsigned char *) nlh + totlen);
}

/**
 * parse attributes of a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 * @arg tb		destination array with maxtype+1 elements
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 *
 * See nla_parse()
 */
int nlmsg_parse(struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[],
		int maxtype, struct nla_policy *policy)
{
	if (nlh->nlmsg_len < nlmsg_msg_size(hdrlen))
		return nl_errno(EINVAL);

	return nla_parse(tb, maxtype, nlmsg_attrdata(nlh, hdrlen),
			 nlmsg_attrlen(nlh, hdrlen), policy);
}

/**
 * nlmsg_find_attr - find a specific attribute in a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of familiy specific header
 * @arg attrtype	type of attribute to look for
 *
 * Returns the first attribute which matches the specified type.
 */
struct nlattr *nlmsg_find_attr(struct nlmsghdr *nlh, int hdrlen, int attrtype)
{
	return nla_find(nlmsg_attrdata(nlh, hdrlen),
			nlmsg_attrlen(nlh, hdrlen), attrtype);
}

/**
 * nlmsg_validate - validate a netlink message including attributes
 * @arg nlh		netlinket message header
 * @arg hdrlen		length of familiy specific header
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 */
int nlmsg_validate(struct nlmsghdr *nlh, int hdrlen, int maxtype,
		   struct nla_policy *policy)
{
	if (nlh->nlmsg_len < nlmsg_msg_size(hdrlen))
		return nl_errno(EINVAL);

	return nla_validate(nlmsg_attrdata(nlh, hdrlen),
			    nlmsg_attrlen(nlh, hdrlen), maxtype, policy);
}

/** @} */

/**
 * @name Message Building/Access
 * @{
 */

struct nl_msg *nlmsg_new(void)
{
	struct nl_msg *nm;

	nm = calloc(1, sizeof(*nm));
	if (!nm)
		goto errout;

	nm->nm_nlh = calloc(1, nlmsg_msg_size(0));
	if (!nm->nm_nlh)
		goto errout;

	nm->nm_nlh->nlmsg_len = nlmsg_msg_size(0);
	return nm;
errout:
	free(nm);
	nl_errno(ENOMEM);
	return NULL;
}


/**
 * Build a new netlink message
 * @arg hdr		Netlink message header template
 *
 * Builds a new netlink message with a tailroom for the netlink
 * message header. If \a hdr is not NULL it will be used as a
 * template for the netlink message header, otherwise the header
 * is left blank.
 * 
 * @return Newly allocated netlink message or NULL
 */ 
struct nl_msg *nlmsg_build(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = nlmsg_new();
	if (nm && hdr) {
		int size = nm->nm_nlh->nlmsg_len;
		memcpy(nm->nm_nlh, hdr, sizeof(*hdr));
		nm->nm_nlh->nlmsg_len = size;
	}

	return nm;
}

struct nl_msg *nlmsg_build_simple(int nlmsgtype, int flags)
{
	struct nlmsghdr nlh = {
		.nlmsg_type = nlmsgtype,
		.nlmsg_flags = flags,
	};

	return nlmsg_build(&nlh);
}

struct nl_msg *nlmsg_build_no_hdr(void)
{
	return nlmsg_build(NULL);
}

struct nl_msg *nlmsg_convert(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = calloc(1, sizeof(struct nl_msg));
	if (!nm)
		goto errout;

	nm->nm_nlh = calloc(1, NLMSG_ALIGN(hdr->nlmsg_len));
	if (!nm->nm_nlh)
		goto errout;

	memcpy(nm->nm_nlh, hdr, NLMSG_ALIGN(hdr->nlmsg_len));

	return nm;
errout:
	free(nm);
	nl_errno(ENOMEM);
	return NULL;
}

/**
 * Append raw data to a netlink message
 * @arg n		netlink message
 * @arg data		data to add
 * @arg len		length of data
 * @arg pad		add padding at the end?
 *
 * Extends the netlink message as needed and appends the data of given
 * length to the message. The length of the message is not aligned to
 * anything. The caller is responsible to provide a length and
 * evtentually padded data to fullfil any alignment requirements.
 *
 * @return 0 on success or a negative error code
 * @attention Appending of improperly aligned raw data may result in
 *            a corrupt message. It is left to you to add the right
 *            amount of data to have the message aligned to NLMSG_ALIGNTO
 *            in the end.
 */
int nlmsg_append(struct nl_msg *n, void *data, size_t len, int pad)
{
	void *tmp;

	if (pad)
		len = NLMSG_ALIGN(len);

	tmp = realloc(n->nm_nlh, n->nm_nlh->nlmsg_len + len);
	if (!tmp)
		return nl_errno(ENOMEM);

	n->nm_nlh = tmp;
	memcpy((void *) n->nm_nlh + n->nm_nlh->nlmsg_len, data, len);
	n->nm_nlh->nlmsg_len += len;

	return 0;
}


/**
 * nlmsg_put - Add a netlink message header
 * @arg n		netlink message
 * @arg pid		netlink process id
 * @arg seq		sequence number of message
 * @arg type		message type
 * @arg payload		length of message payload
 * @arg flags		message flags
 */
struct nlmsghdr *nlmsg_put(struct nl_msg *n, uint32_t pid, uint32_t seq,
			   int type, int payload, int flags)
{
	struct nlmsghdr *nlh;

	if (n->nm_nlh->nlmsg_len < NLMSG_HDRLEN)
		BUG();

	nlh = (struct nlmsghdr *) n->nm_nlh;
	nlh->nlmsg_type = type;
	nlh->nlmsg_len = nlmsg_msg_size(payload);
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_pid = pid;
	nlh->nlmsg_seq = seq;

	memset((unsigned char *) nlmsg_data(nlh) + payload, 0,
	       nlmsg_padlen(payload));

	return nlh;
}

/**
 * Return actual netlink message
 * @arg n		netlink message
 * 
 * Returns the actual netlink message casted to the type of the netlink
 * message header.
 * 
 * @return A pointer to the netlink message.
 */
struct nlmsghdr *nlmsg_hdr(struct nl_msg *n)
{
	return n->nm_nlh;
}

/**
 * Free a netlink message
 * @arg n		netlink message
 *
 * Destroys a netlink message and frees up all used memory.
 *
 * @pre The message must be unused.
 */
void nlmsg_free(struct nl_msg *n)
{
	if (!n)
		return;

	free(n->nm_nlh);
	free(n);
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

void nlmsg_set_proto(struct nl_msg *msg, int protocol)
{
	msg->nm_protocol = protocol;
}

int nlmsg_get_proto(struct nl_msg *msg)
{
	return msg->nm_protocol;
}

void nlmsg_set_src(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_src, addr, sizeof(*addr));
}

struct sockaddr_nl *nlmsg_get_src(struct nl_msg *msg)
{
	return &msg->nm_src;
}

void nlmsg_set_dst(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_dst, addr, sizeof(*addr));
}

struct sockaddr_nl *nlmsg_get_dst(struct nl_msg *msg)
{
	return &msg->nm_dst;
}

void nlmsg_set_creds(struct nl_msg *msg, struct ucred *creds)
{
	memcpy(&msg->nm_creds, creds, sizeof(*creds));
	msg->nm_flags |= NL_MSG_CRED_PRESENT;
}

struct ucred *nlmsg_get_creds(struct nl_msg *msg)
{
	if (msg->nm_flags & NL_MSG_CRED_PRESENT)
		return &msg->nm_creds;
	return NULL;
}

/** @} */

/**
 * @name Netlink Message Type Translations
 * @{
 */

static struct trans_tbl nl_msgtypes[] = {
	__ADD(NLMSG_NOOP,NOOP)
	__ADD(NLMSG_ERROR,ERROR)
	__ADD(NLMSG_DONE,DONE)
	__ADD(NLMSG_OVERRUN,OVERRUN)
};

/**
 * Convert netlink message type number to character string.
 * @arg type		Netlink message type.
 * @arg buf		Destination buffer.
 * @arg size		Size of destination buffer.
 *
 * Converts a netlink message type number to a character string and stores
 * it in the provided buffer.
 *
 * @return The destination buffer or the type encoded in hexidecimal form
 * 	   if no match was found.
 */
char *nl_nlmsgtype2str(int type, char *buf, size_t size)
{
	return __type2str(type, buf, size, nl_msgtypes,
			  ARRAY_SIZE(nl_msgtypes));
}

/**
 * Convert character string to netlink message type.
 * @arg name		Name of netlink message type.
 *
 * Converts the provided character string specifying a netlink message type
 * into the corresponding numeric value
 *
 * @return Numeric netlink message type or a negative value
 * 	   if no match was found.
 */
int nl_str2nlmsgtype(const char *name)
{
	return __str2type(name, nl_msgtypes, ARRAY_SIZE(nl_msgtypes));
}

/** @} */

/**
 * @name Netlink Message Flags Translations
 * @{
 */

/**
 * Translate netlink message flags into a character string (Reentrant).
 * @arg flags		netlink message flags
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Translates netlink message flags into a character string and stores
 * it in the provided buffer.
 *
 * @return The destination buffer
 */
char *nl_nlmsg_flags2str(int flags, char *buf, size_t len)
{
	memset(buf, 0, len);

#define PRINT_FLAG(f) \
	if (flags & NLM_F_##f) { \
		flags &= ~NLM_F_##f; \
		strncat(buf, #f, len - strlen(buf) - 1); \
		if (flags) \
			strncat(buf, ",", len - strlen(buf) - 1); \
	}
	
	PRINT_FLAG(REQUEST);
	PRINT_FLAG(MULTI);
	PRINT_FLAG(ACK);
	PRINT_FLAG(ECHO);
	PRINT_FLAG(ROOT);
	PRINT_FLAG(MATCH);
	PRINT_FLAG(ATOMIC);
	PRINT_FLAG(REPLACE);
	PRINT_FLAG(EXCL);
	PRINT_FLAG(CREATE);
	PRINT_FLAG(APPEND);

	if (flags) {
		char s[32];
		snprintf(s, sizeof(s), "0x%x", flags);
		strncat(buf, s, len - strlen(buf) - 1);
	}
#undef PRINT_FLAG

	return buf;
}

/** @} */

/**
 * @name Direct Parsing
 * @{
 */

/** @cond SKIP */
struct dp_xdata {
	void (*cb)(struct nl_object *, void *);
	void *arg;
};
/** @endcond */

static int parse_cb(struct nl_object *obj, struct nl_parser_param *p)
{
	struct dp_xdata *x = p->pp_arg;

	x->cb(obj, x->arg);
	nl_object_put(obj);
	return 0;
}

int nl_msg_parse(struct nl_msg *msg, void (*cb)(struct nl_object *, void *),
		 void *arg)
{
	struct nl_cache_ops *ops;
	struct nl_parser_param p = {
		.pp_cb = parse_cb
	};
	struct dp_xdata x = {
		.cb = cb,
		.arg = arg,
	};

	ops = nl_cache_mngt_associate(nlmsg_get_proto(msg),
				      nlmsg_hdr(msg)->nlmsg_type);
	if (ops == NULL)
		return nl_error(ENOENT, "Unknown message type %d",
				nlmsg_hdr(msg)->nlmsg_type);
	p.pp_arg = &x;

	return nl_cache_parse(ops, NULL, nlmsg_hdr(msg), &p);
}

/** @} */

/** @} */

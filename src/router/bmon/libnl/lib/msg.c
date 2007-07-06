/*
 * lib/msg.c		Netlink Message Helpers
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/**
 * @ingroup nl
 * @defgroup msg Netlink Messages
 *
 * Provides functionatilty to build and parse netlink messages.
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
 *  <---------- hlen ---------->       <------- len ------->
 *  <------------------ NLMSG_LENGTH(len) ----------------->
 *  <---------------------- NLMSG_SPACE(len) -------------------->
 * @endcode
 * @par
 * The payload may consist of arbitary data but may have strict
 * alignment and formatting rules depening on the specific netlink
 * families.
 * @par
 * @code
 * NLMSG_TAIL(nlh)---------------------v
 * +----------+- - -+- - - - - -+- - -+----------+- - -+- - - - - -
 * |  Header  | Pad |  Payload  | Pad |  Header  | Pad |  Payload
 * +----------+- - -+- - - - - -+- - -+----------+- - -+- - - - - -
 * NLMSG_DATA(nlh)---^                 ^
 * NLMSG_NEXT(nlh, len)----------------'
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
 * @par Example 1 (Building a netlink message):
 * @code
 * struct nlmsghdr hdr = {
 * 	.nlmsg_type = MY_TYPE,
 * };
 * struct mystruct mydata = {
 * 	.a = 10,
 * 	.b = 20,
 * 	.c = 400
 * };
 * struct nl_msg *msg = nlmsg_build(&hdr);
 * nl_msg_append_raw(msg, &mydata, sizeof(mydata));
 * nl_send_auto_complete(handle, nl_msg_get(msg));
 * nl_msg_free(msg);
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
 * while (NLMSG_OK(hdr, n)) {
 * 	// Process message here...
 * 	hdr = NLMSG_NEXT(hdr, n);
 * }
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <linux/socket.h>

/**
 * @name Message Building/Access
 * @{
 */

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
struct nl_msg * nl_msg_build(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = calloc(1, sizeof(struct nl_msg));
	if (nm == NULL)
		return NULL;

	nm->nmsg = calloc(1, NLMSG_LENGTH(0));
	if (nm->nmsg == NULL) {
		free(nm);
		return NULL;
	}

	if (hdr)
		memcpy(nm->nmsg, hdr, sizeof(*hdr));

	nm->nmsg->nlmsg_len = NLMSG_LENGTH(0);
	return nm;
}

/**
 * Append raw data to a netlink message
 * @arg n		netlink message
 * @arg data		data to add
 * @arg len		length of data
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
int nl_msg_append_raw(struct nl_msg *n, void *data, size_t len)
{
	void *tmp;

	tmp = realloc(n->nmsg, n->nmsg->nlmsg_len + len);

	if (tmp == NULL)
		return -ENOMEM;

	n->nmsg = tmp;
	memcpy((void *) n->nmsg + n->nmsg->nlmsg_len, data, len);
	n->nmsg->nlmsg_len += len;

	return 0;
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
struct nlmsghdr * nl_msg_get(struct nl_msg *n)
{
	return n->nmsg;
}

/**
 * Return payload of a netlink message
 * @arg n		netlink message
 * @return A pointer to the start of the payload.
 */
void * nl_msg_payload(struct nl_msg *n)
{
	return NLMSG_DATA(n->nmsg);
}

/**
 * Return length of payload of a netlink message
 * @arg n		netlink message
 * @return The length of the payload in bytes.
 */
size_t nl_msg_payloadlen(struct nl_msg *n)
{
	return NLMSG_PAYLOAD(n->nmsg, 0);
}

/**
 * Free a netlink message
 * @arg n		netlink message
 *
 * Destroys a netlink message and frees up all used memory.
 *
 * @pre The message must be unused.
 */
void nl_msg_free(struct nl_msg *n)
{
	if (n->nmsg) {
		free(n->nmsg);
		n->nmsg = NULL;
	}

	free(n);
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
 * Translate a netlink message type number into a character string (Reentrant).
 * @arg type		netlink message type
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Translates a netlink message type number into a character string and stores
 * it in the provided buffer.
 *
 * @return The destination buffer or the type encoded in hex if no match was found.
 */
char * nl_nlmsgtype2str_r(int type, char *buf, size_t len)
{
	return __type2str_r(type, buf, len, nl_msgtypes,
	    ARRAY_SIZE(nl_msgtypes));
}

/**
 * Translate a netlink message type number into a character string
 * @arg type		netlink message type
 *
 * Translates a netlink message type number into a character string and stores
 * it in a static buffer.
 *
 * @return A static buffer or the type encoded in hex if no match was found.
 * @attention This funnction is NOT thread safe.
 */
char * nl_nlmsgtype2str(int type)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(type, buf, sizeof(buf), nl_msgtypes,
	    ARRAY_SIZE(nl_msgtypes));
}

/**
 * Translate a character string into a netlink message type
 * @arg name		netlink message type name
 *
 * Translates the provided character string specifying a netlink message type
 * into the corresponding numeric value
 *
 * @return Netlink message type number or a negative value.
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
char * nl_nlmsg_flags2str_r(int flags, char *buf, size_t len)
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

/**
 * Transtlate netlink message flags into a character string.
 * @arg flags		netlink message flags
 *
 * Translates netlink message flags into a character string and stores
 * it in a static buffer.
 *
 * @return A static buffer
 * @attention This funnction is NOT thread safe.
 */
char * nl_nlmsg_flags2str(int flags)
{
	static char buf[128];
	memset(buf, 0, sizeof(buf));
	return nl_nlmsg_flags2str_r(flags, buf, sizeof(buf));
}

/** @} */


/** @} */

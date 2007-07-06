#ifndef __LINUX_NETLINK_H
#define __LINUX_NETLINK_H

/**
 * @ingroup nl
 * @name Netlink Families
 * @{
 */

#define NETLINK_ROUTE		0	/* Routing/device hook				*/
#define NETLINK_SKIP		1	/* Reserved for ENskip  			*/
#define NETLINK_USERSOCK	2	/* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL	3	/* Firewalling hook				*/
#define NETLINK_TCPDIAG		4	/* TCP socket monitoring			*/
#define NETLINK_NFLOG		5	/* netfilter/iptables ULOG */
#define NETLINK_XFRM		6	/* ipsec */
#define NETLINK_SELINUX		7	/* SELinux event notifications */
#define NETLINK_ARPD		8
#define NETLINK_AUDIT		9	/* auditing */
#define NETLINK_ROUTE6		11	/* af_inet6 route comm channel */
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	/* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT	15	/* Kernel messages to userspace */
#define NETLINK_TAPBASE		16	/* 16 to 31 are ethertap */

#define MAX_LINKS 32		

/** \} */

/**
 * Netlink socket address
 * @ingroup nl
 */
struct sockaddr_nl
{
	/** socket family (== AF_NETLINK) */
	sa_family_t     nl_family;

	/** Padding (unused) */
	unsigned short  nl_pad;

	/** Unique process ID  */
	uint32_t        nl_pid;

	/** Multicast group subscriptions */
	uint32_t        nl_groups;
};

/**
 * Netlink message header
 * @ingroup msg
 */
struct nlmsghdr
{
	/**
	 * Length of message including header.
	 */
	uint32_t	nlmsg_len;

	/**
	 * Message type (content type)
	 */
	uint16_t	nlmsg_type;

	/**
	 * Additional flags
	 */
	uint16_t	nlmsg_flags;

	/**
	 * Sequence number
	 */
	uint32_t	nlmsg_seq;

	/**
	 * The PID of the proccess sending the message.
	 */
	uint32_t	nlmsg_pid;
};

/**
 * @ingroup msg
 * @name Standard message flags
 * @anchor nlmsgflags
 * @{
 */

/** Must be set on all request messages (typically from user space to
 * kernel space). */
#define NLM_F_REQUEST		1

/** Indicates the message is part of a multipart message terminated
 * by NLMSG_DONE. */
#define NLM_F_MULTI		2

/** Request for an acknowledgment on success. */
#define NLM_F_ACK		4

/** Echo this request */
#define NLM_F_ECHO		8

/** @} */

/**
 * @ingroup msg
 * @name Additional message flags for GET requests
 * @{
 */

/** Return the complete table instead of a single entry. */
#define NLM_F_ROOT	0x100

/** Return all entries matching criteria passed in message content. */
#define NLM_F_MATCH	0x200

/** Return an atomic snapshot of the table being referenced. This
 * may require special privileges because it has the potential to
 * interrupt service in the FE for a longer time. */
#define NLM_F_ATOMIC	0x400

/** Dump all entries */
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/** @} */

/**
 * @ingroup msg
 * @name Additional messsage flags for NEW requests
 * @{
 */

/** Replace existing matching config object with this request. */
#define NLM_F_REPLACE	0x100

/** Don't replace the config object if it already exists. */
#define NLM_F_EXCL	0x200

/** Create config object if it doesn't already exist. */
#define NLM_F_CREATE	0x400

/** Add to the end of the object list. */
#define NLM_F_APPEND	0x800

/** @} */

/**
 * @ingroup msg
 * @name Low Level Message Macros
 * @{
 */

/** Required alignment for netlink message header and payload. */
#define NLMSG_ALIGNTO	4

/**
 * Extend a length to be properly aligned to NLMSG_ALIGNTO.
 * @arg len		original length
 */
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )

/**
 * Length of message without additional padding to payload.
 * @arg len		length of payload
 */
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(sizeof(struct nlmsghdr)))

/**
 * Real length of message including all paddings
 * @arg len		length of payload
 */
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))

/**
 * Next message in byte stream.
 * @arg nlh		netlink message header
 * @arg len		current remaining length of byte stream (writeable)
 *
 * Decreases \a len by the length of the current length and returns
 * a pointer to the next message.
 * 
 * @pre \a len must contain remaining length of message byte stream.
 * @post \a len will contain new remaining length without the length
 *       of the current message pointed to by \a nlh.
 */
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + \
				  NLMSG_ALIGN((nlh)->nlmsg_len)))

/**
 * Validate a possible netlink message against current byte stream.
 * @arg nlh		netlink message header
 * @arg len		remaining length of byte stream
 *
 * Validates the netlink mesasge pointed to by \a nlh and checks
 * wether it fits into the remaining tail of the byte stream.
 */
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))

/**
 * Pointer to message payload
 * @arg nlh		netlink message header
 */
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))

/**
 * Length of payload
 * @arg nlh		netlink message header
 * @arg len		Length of payload prefixed (aligned)
 */
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

/** @} */

/**
 * @ingroup msg
 * @name Message types
 * @note Individual netlink families specify more message types, e.g.,
 * \c NETLINK_ROUTE service specifies several types, such as \c RTM_NEWLINK
 * \c RTM_DELLINK, \c RTM_GETLINK, \c RTM_NEWADDR, \c RTM_DELADDR,
 * \c RTM_NEWROUTE, \c RTM_DELROUTE, etc.
 * @{
 */

/** No operation, message must be ignored */
#define NLMSG_NOOP	1
/** The message signals an error and the payload contains a nlmsgerr
 * structure. This can be looked at as a NACK and typically it is
 * from FEC to CPC. */
#define NLMSG_ERROR	2
/** Message terminates a multipart message. */
#define NLMSG_DONE	3
/** The message signals that data got lost */
#define NLMSG_OVERRUN	4

/** @} */

/**
 * Netlink error message
 * @ingroup msg
 */
struct nlmsgerr
{
	/** Error code (errno number) */
	int		error;

	/** Original netlink message causing the error */
	struct nlmsghdr	msg;
};

#endif	/* __LINUX_NETLINK_H */

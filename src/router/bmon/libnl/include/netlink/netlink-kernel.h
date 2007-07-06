#ifndef __LINUX_NETLINK_H
#define __LINUX_NETLINK_H

#define NETLINK_ROUTE		0	/* Routing/device hook */
#define NETLINK_W1		1	/* 1-wire subsystem */
#define NETLINK_USERSOCK	2	/* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL	3	/* Firewalling hook		*/
#define NETLINK_INET_DIAG	4	/* INET socket monitoring */
#define NETLINK_TCPDIAG		NETLINK_INET_DIAG
#define NETLINK_NFLOG		5	/* netfilter/iptables ULOG */
#define NETLINK_XFRM		6	/* ipsec */
#define NETLINK_SELINUX		7	/* SELinux event notifications */
#define NETLINK_ISCSI		8	/* Open-iSCSI */
#define NETLINK_AUDIT		9	/* auditing */
#define NETLINK_FIB_LOOKUP	10
#define NETLINK_CONNECTOR	11
#define NETLINK_NETFILTER	12
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	/* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT	15	/* Kernel messages to userspace */
#define NETLINK_GENERIC		16

#define MAX_LINKS 32		

/**
 * Netlink socket address
 * @ingroup nl
 */
struct sockaddr_nl
{
	/** socket family (AF_NETLINK) */
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
	 * Message flags
	 */
	uint16_t	nlmsg_flags;

	/**
	 * Sequence number
	 */
	uint32_t	nlmsg_seq;

	/**
	 * Netlink PID of the proccess sending the message.
	 */
	uint32_t	nlmsg_pid;
};

/**
 * @name Standard message flags
 * @{
 */

/**
 * Must be set on all request messages (typically from user space to
 * kernel space).
 * @ingroup msg
 */
#define NLM_F_REQUEST		1

/**
 * Indicates the message is part of a multipart message terminated
 * by NLMSG_DONE.
 */
#define NLM_F_MULTI		2

/**
 * Request for an acknowledgment on success.
 */
#define NLM_F_ACK		4

/**
 * Echo this request
 */
#define NLM_F_ECHO		8

/** @} */

/**
 * @name Additional message flags for GET requests
 * @{
 */

/**
 * Return the complete table instead of a single entry.
 * @ingroup msg
 */
#define NLM_F_ROOT	0x100

/**
 * Return all entries matching criteria passed in message content.
 */
#define NLM_F_MATCH	0x200

/**
 * Return an atomic snapshot of the table being referenced. This
 * may require special privileges because it has the potential to
 * interrupt service in the FE for a longer time.
 */
#define NLM_F_ATOMIC	0x400

/**
 * Dump all entries
 */
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/** @} */

/**
 * @name Additional messsage flags for NEW requests
 * @{
 */

/**
 * Replace existing matching config object with this request.
 * @ingroup msg
 */
#define NLM_F_REPLACE	0x100

/**
 * Don't replace the config object if it already exists.
 */
#define NLM_F_EXCL	0x200

/**
 * Create config object if it doesn't already exist.
 */
#define NLM_F_CREATE	0x400

/**
 * Add to the end of the object list.
 */
#define NLM_F_APPEND	0x800

/** @} */

#define NLMSG_ALIGNTO	4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

/**
 * @name Standard Message types
 * @{
 */

/**
 * No operation, message must be ignored
 * @ingroup msg
 */
#define NLMSG_NOOP		0x1

/**
 * The message signals an error and the payload contains a nlmsgerr
 * structure. This can be looked at as a NACK and typically it is
 * from FEC to CPC.
 */
#define NLMSG_ERROR		0x2

/**
 * Message terminates a multipart message.
 */
#define NLMSG_DONE		0x3

/**
 * The message signals that data got lost
 */
#define NLMSG_OVERRUN		0x4

/**
 * Lower limit of reserved message types
 */
#define NLMSG_MIN_TYPE		0x10

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

#define NETLINK_ADD_MEMBERSHIP	1
#define NETLINK_DROP_MEMBERSHIP	2
#define NETLINK_PKTINFO		3

struct nl_pktinfo
{
	__u32	group;
};

/**
 * Netlink Attribute
 * @ingroup attr
 */
struct nlattr
{
	/** Attribute length */
	__u16           nla_len;

	/** Attribute type */
	__u16           nla_type;
};

#define NLA_ALIGNTO		4
#define NLA_ALIGN(len)		(((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN		((int) NLA_ALIGN(sizeof(struct nlattr)))

#endif	/* __LINUX_NETLINK_H */

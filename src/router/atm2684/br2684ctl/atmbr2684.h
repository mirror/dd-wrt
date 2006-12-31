#ifndef _LINUX_ATMBR2684_H
#define _LINUX_ATMBR2684_H

#include <linux/atm.h>
//#include <linux/if.h>		/* For IFNAMSIZ */
#define IFNAMSIZ 16
#define T2A_PVC		  1	/* address is PVC */
#define SOL_SOCKET	0xffff
#define SO_SNDBUF	0x1001	/* Send buffer size. */
#define SOL_IP		0
#define SOL_TCP		6
#define SOL_UDP		17
#define SOL_IPV6	41
#define SOL_ICMPV6	58
#define SOL_RAW		255
#define SOL_IPX		256
#define SOL_AX25	257
#define SOL_ATALK	258
#define SOL_NETROM	259
#define SOL_ROSE	260
#define SOL_DECNET	261
#define	SOL_X25		262
#define SOL_PACKET	263
#define SOL_ATM		264	/* ATM layer (cell level) */
#define SOL_AAL		265	/* ATM Adaption Layer (packet level) */
#define SOL_IRDA        266
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_LOCAL	AF_LOCAL
#define PF_INET		AF_INET
#define PF_AX25		AF_AX25
#define PF_IPX		AF_IPX
#define PF_APPLETALK	AF_APPLETALK
#define	PF_NETROM	AF_NETROM
#define PF_BRIDGE	AF_BRIDGE
#define PF_ATMPVC	AF_ATMPVC
#define PF_X25		AF_X25
#define PF_INET6	AF_INET6
#define PF_ROSE		AF_ROSE
#define PF_DECnet	AF_DECnet
#define PF_NETBEUI	AF_NETBEUI
#define PF_SECURITY	AF_SECURITY
#define PF_KEY		AF_KEY
#define PF_NETLINK	AF_NETLINK
#define PF_ROUTE	AF_ROUTE
#define PF_PACKET	AF_PACKET
#define PF_ASH		AF_ASH
#define PF_ECONET	AF_ECONET
#define PF_ATMSVC	AF_ATMSVC
#define PF_SNA		AF_SNA
#define PF_IRDA		AF_IRDA
#define PF_PPPOX	AF_PPPOX
#define PF_MAX		AF_MAX
#define AF_UNSPEC	0
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_AX25		3	/* Amateur Radio AX.25 		*/
#define AF_IPX		4	/* Novell IPX 			*/
#define AF_APPLETALK	5	/* AppleTalk DDP 		*/
#define AF_NETROM	6	/* Amateur Radio NET/ROM 	*/
#define AF_BRIDGE	7	/* Multiprotocol bridge 	*/
#define AF_ATMPVC	8	/* ATM PVCs			*/
#define AF_X25		9	/* Reserved for X.25 project 	*/
#define AF_INET6	10	/* IP version 6			*/
#define AF_ROSE		11	/* Amateur Radio X.25 PLP	*/
#define AF_DECnet	12	/* Reserved for DECnet project	*/
#define AF_NETBEUI	13	/* Reserved for 802.2LLC project*/
#define AF_SECURITY	14	/* Security callback pseudo AF */
#define AF_KEY		15      /* PF_KEY key management API */
#define AF_NETLINK	16
#define AF_ROUTE	AF_NETLINK /* Alias to emulate 4.4BSD */
#define AF_PACKET	17	/* Packet family		*/
#define AF_ASH		18	/* Ash				*/
#define AF_ECONET	19	/* Acorn Econet			*/
#define AF_ATMSVC	20	/* ATM SVCs			*/
#define AF_SNA		22	/* Linux SNA Project (nutters!) */
#define AF_IRDA		23	/* IRDA sockets			*/
#define AF_PPPOX	24	/* PPPoX sockets		*/
#define AF_MAX		32	/* For now.. */
#define SOCK_DGRAM	1		/* datagram (conn.less) socket	*/
#define SOCK_STREAM	2		/* stream (connection) socket	*/
#define SOCK_RAW	3		/* raw socket			*/
#define SOCK_RDM	4		/* reliably-delivered message	*/
#define SOCK_SEQPACKET	5		/* sequential packet socket	*/
#define SOCK_PACKET	10		/* linux specific way of	*/



/*
 * Type of media we're bridging (ethernet, token ring, etc)  Currently only
 * ethernet is supported
 */
#define BR2684_MEDIA_ETHERNET	(0)	/* 802.3 */
#define BR2684_MEDIA_802_4	(1)	/* 802.4 */
#define BR2684_MEDIA_TR		(2)	/* 802.5 - token ring */
#define BR2684_MEDIA_FDDI	(3)
#define BR2684_MEDIA_802_6	(4)	/* 802.6 */

/*
 * Is there FCS inbound on this VC?  This currently isn't supported.
 */
#define BR2684_FCSIN_NO		(0)
#define BR2684_FCSIN_IGNORE	(1)
#define BR2684_FCSIN_VERIFY	(2)

/*
 * Is there FCS outbound on this VC?  This currently isn't supported.
 */
#define BR2684_FCSOUT_NO	(0)
#define BR2684_FCSOUT_SENDZERO	(1)
#define BR2684_FCSOUT_GENERATE	(2)

/*
 * Does this VC include LLC encapsulation?
 */
#define BR2684_ENCAPS_VC	(0)	/* VC-mux */
#define BR2684_ENCAPS_LLC	(1)
#define BR2684_ENCAPS_AUTODETECT (2)	/* Unsuported */

/*
 * This is for the ATM_NEWBACKENDIF call - these are like socket families:
 * the first element of the structure is the backend number and the rest
 * is per-backend specific
 */
struct atm_newif_br2684 {
	atm_backend_t	backend_num;	/* ATM_BACKEND_BR2684 */
	int		media;		/* BR2684_MEDIA_* */
	char		ifname[IFNAMSIZ];
	int		mtu;
};

/*
 * This structure is used to specify a br2684 interface - either by a
 * positive integer (returned by ATM_NEWBACKENDIF) or the interfaces name
 */
#define BR2684_FIND_BYNOTHING	(0)
#define BR2684_FIND_BYNUM	(1)
#define BR2684_FIND_BYIFNAME	(2)
struct br2684_if_spec {
	int method;			/* BR2684_FIND_* */
	union {
		char		ifname[IFNAMSIZ];
		int		devnum;
	} spec;
};

/*
 * This is for the ATM_SETBACKEND call - these are like socket families:
 * the first element of the structure is the backend number and the rest
 * is per-backend specific
 */
struct atm_backend_br2684 {
	atm_backend_t	backend_num;	/* ATM_BACKEND_BR2684 */
	struct br2684_if_spec ifspec;
	int	fcs_in;		/* BR2684_FCSIN_* */
	int	fcs_out;	/* BR2684_FCSOUT_* */
	int	fcs_auto;	/* 1: fcs_{in,out} disabled if no FCS rx'ed */
	int	encaps;		/* BR2684_ENCAPS_* */
	int	has_vpiid;	/* 1: use vpn_id - Unsupported */
	unsigned char	vpn_id[7];
	//__u8	vpn_id[7];
	int	send_padding;	/* unsupported */
	int	min_size;	/* we will pad smaller packets than this */
        int     flag;           /* brcm -- protocol filter flag */
};

/*
 * The BR2684_SETFILT ioctl is an experimental mechanism for folks
 * terminating a large number of IP-only vcc's.  When netfilter allows
 * efficient per-if in/out filters, this support will be removed
 */
struct br2684_filter {
	unsigned int	prefix;		/* network byte order */
	unsigned int	netmask;	/* 0 = disable filter */
	//__u32	prefix;		/* network byte order */
	//__u32	netmask;	/* 0 = disable filter */
};

struct br2684_filter_set {
	struct br2684_if_spec ifspec;
	struct br2684_filter filter;
};

#define BR2684_SETFILT	_IOW( 'a', ATMIOC_BACKEND + 0, \
				struct br2684_filter_set)
#define BR2684_DEL	253

#endif /* _LINUX_ATMBR2684_H */

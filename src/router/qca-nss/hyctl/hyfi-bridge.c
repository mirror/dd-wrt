/*
 * Copyright (c) 2012, 2014-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*
 * Copyright (c) 2010 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * 2012 Qualcomm Atheros, Inc.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include "hyfi-bridge.h"

#ifdef SON_MEMORY_DEBUG

#include "qca-son-mem-debug.h"
#undef QCA_MOD_INPUT
#define QCA_MOD_INPUT QCA_MOD_LIBHYFI_BRIDGE

#include "son-mem-debug.h"

#endif /* SON_MEMORY_DEBUG */


/* Use this internal structure for caching table entry size and commands */
typedef struct
{
	size_t size;
	u_int32_t get_command;
	u_int32_t set_command;
    u_int32_t netlink_key;

} bridgeTableParams_t;

static const bridgeTableParams_t tableParams[] =
{
		{ sizeof( struct __hatbl_entry ), HYFI_GET_HA_TABLE, HYFI_ADD_HATBL_ENTRIES, NETLINK_QCA_HYFI },
		{ sizeof( struct __hdtbl_entry ), HYFI_GET_HD_TABLE, HYFI_ADD_HDTBL_ENTRIES, NETLINK_QCA_HYFI },
		{ sizeof( struct __hfdb_entry ), HYFI_GET_FDB, ~0, NETLINK_QCA_HYFI },
		{ sizeof( struct __mc_mdb_entry ), HYFI_GET_MC_MDB, ~0, NETLINK_QCA_MC },
		{ sizeof( struct __mc_param_acl_rule ), HYFI_GET_MC_ACL, ~0, NETLINK_QCA_MC },
		{ sizeof( struct __mc_encaptbl_entry ), ~0, HYFI_SET_MC_PSW_ENCAP, NETLINK_QCA_MC },
		{ sizeof( struct __mc_floodtbl_entry ), ~0, HYFI_SET_MC_PSW_FLOOD, NETLINK_QCA_MC },
		{ sizeof( struct __mc_param_router_port), HYFI_GET_MC_ROUTER_PORT, ~0, NETLINK_QCA_MC},
};

/* print debug information */
const char *hyctl_status_debug[] = {"HYFI_STATUS_SUCCESS",
                                    "HYFI_STATUS_NOT_SUPPORTED",
                                    "HYFI_STATUS_RESOURCES",
                                    "HYFI_STATUS_INVALID_PARAMETER",
                                    "HYFI_STATUS_BUFFER_OVERFLOW",
                                    "HYFI_STATUS_FAILURE",
                                    "HYFI_STATUS_NOT_FOUND"};


//#define DEBUG_HY_NETLINK

/*-F- netlink_msg --
 */
int32_t netlink_msg(int32_t msg_type, u_int8_t *data, int32_t hymsgdatalen, int32_t netlink_key)
{
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    socklen_t fromlen;
    int32_t ret = HYFI_STATUS_FAILURE;
    int32_t sock_fd;
    struct __hyctl_msg_header *msgheader;
    static pid_t myPid = 0;

    /* Do it only once per context, save a system call */
    if(!myPid)
    	myPid = getpid();

    do {
        sock_fd = socket(AF_NETLINK, SOCK_RAW, netlink_key);
        if (sock_fd <0) {
#ifdef DEBUG_HY_NETLINK
            printf("netlink socket create failed\n");
#endif
            break;
        }

        /* Set nonblock. */
        if (fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK)) {
#ifdef DEBUG_HY_NETLINK
            perror("fcntl():");
#endif
            break;
        }

        fromlen = sizeof(src_addr);
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = myPid;  /* self pid */
        src_addr.nl_groups = 0;  /* not in mcast groups */

        bind(sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr));
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0;   /* For Linux Kernel */
        dest_addr.nl_groups = 0; /* unicast */
        nlh=(struct nlmsghdr *)data;
        /* Fill the netlink message header */
        nlh->nlmsg_type = msg_type;
        nlh->nlmsg_len = NLMSG_SPACE(HYFI_MSG_HDRLEN+hymsgdatalen);
        nlh->nlmsg_pid = myPid;  /* self pid */
        nlh->nlmsg_flags = 0;

        int optval;

        optval = nlh->nlmsg_len;
        if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUFFORCE,
                       &optval, sizeof(optval))) {
            perror("Setsockopt SO_SNDBUF: ");
            break;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUFFORCE,
                       &optval, sizeof(optval))) {
            perror("Setsockopt SO_RCVBUF: ");
            break;
        }

        if (sendto(sock_fd,
                     (void*)nlh,
                     nlh->nlmsg_len,
                     0,
                     (struct sockaddr *)&dest_addr,
                     sizeof(struct sockaddr_nl)) <= 0) {
#ifdef DEBUG_HY_NETLINK
            perror("netlink socket send failed\n");
#endif
            break;
        }

        struct pollfd pollfd = {
		    sock_fd,
		    POLLIN,
		    0
	    };

        if (poll(&pollfd, 1, 2000) <= 0) { /* timeout:2s */
#ifdef DEBUG_HY_NETLINK
            perror("poll():");
#endif
            break;
        }

        if (recvfrom(sock_fd,
                       (void*)nlh,
                       NLMSG_SPACE(HYFI_MSG_HDRLEN+hymsgdatalen),
                       MSG_WAITALL,
                       (struct sockaddr *)&src_addr,
                       &fromlen) <= 0) {
#ifdef DEBUG_HY_NETLINK
            perror("netlink socket receive failed\n");
#endif
            break;
        }
        msgheader = (struct __hyctl_msg_header *)NLMSG_DATA(nlh);

        ret = msgheader->status;

#ifdef DEBUG_HY_NETLINK
        if (ret !=HYFI_STATUS_SUCCESS)
            printf("netlink socket status failed %d\n", ret);
#endif

    } while (0);

    if (sock_fd >0)
        close(sock_fd);

    return ret;
}

void *bridgeAllocTableBuf( int32_t Size, const char *BridgeName )
{
    u_int8_t *data = malloc( HYFI_BRIDGE_MESSAGE_SIZE( Size ) );

    if (data == NULL) {
        return NULL;
    }

    bridgeInitBuf( data, Size + HYFI_BRIDGE_MESSAGE_SIZE(0), BridgeName );

    /* Provide a pointer to the calling application without the header */
    return (void *)( data + HYFI_BRIDGE_MESSAGE_SIZE(0) );
}

void bridgeFreeTableBuf( void *Buf )
{
	if( Buf )
	{
		/* Walking back, restoring the original pointer */
	    u_int8_t *data = (u_int8_t *)(Buf) - HYFI_BRIDGE_MESSAGE_SIZE(0);
		free(data);
	}
}

void bridgeInitBuf( void *Buf, size_t Size, const char *BridgeName )
{
	struct __hyctl_msg_header *hymsghdr;

    /* Clear the buffer and initialize bridge name and buffer size */
	memset( Buf, 0, Size );
    hymsghdr = NLMSG_DATA( Buf );

    if (BridgeName) {
      if (strlcpy(hymsghdr->if_name, BridgeName,
              sizeof(hymsghdr->if_name)) >= sizeof(hymsghdr->if_name)) {
          return;
      }
    }

    hymsghdr->buf_len = Size - HYFI_BRIDGE_MESSAGE_SIZE(0);
}

int32_t bridgeAttach(const char* BridgeName)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_ATTACH_BRIDGE, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeDetach --
 */
int32_t bridgeDetach(const char* BridgeName)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_DETACH_BRIDGE, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

int32_t getBridge(const char* BridgeName)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_GET_BRIDGE, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeSetBridgeMode --
 */
int32_t bridgeSetBridgeMode(const char* BridgeName, int32_t Mode)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( u_int32_t) ) ];
    int32_t *p;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    if (Mode != !HYFI_BRIDGE_MODE_RELAY_OVERRIDE &&
		Mode != HYFI_BRIDGE_MODE_RELAY_OVERRIDE)
        return -1;

    p = HYFI_MSG_DATA(nlmsgbuf);
    *p = Mode;

    retval = netlink_msg(HYFI_SET_BRIDGE_MODE, nlmsgbuf, sizeof(u_int32_t), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeSetBridgeForwardingMode --
 */
int32_t bridgeSetForwardingMode(const char* BridgeName, int32_t Mode)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( u_int32_t) ) ];
    int32_t *p;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    if (Mode != HYFI_BRIDGE_FWMODE_APS &&
		Mode != HYFI_BRIDGE_FWMODE_NO_HYBRID_TABLES &&
		Mode != HYFI_BRIDGE_FWMODE_MCAST_ONLY)
        return -1;

    p = HYFI_MSG_DATA(nlmsgbuf);
    *p = Mode;

    retval = netlink_msg(HYFI_SET_BRIDGE_FWMODE, nlmsgbuf, sizeof(u_int32_t), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeSetIFGroup --
 */
int32_t bridgeSetIFGroup(const char* BridgeName, const char* InterfaceName, int32_t GroupID, int32_t GroupType)
{

    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __brport_group) ) ];
    int32_t ifindex;
    struct __brport_group *p;

    if (!(ifindex = if_nametoindex(InterfaceName))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    p->ifindex = ifindex;
    p->group_num = GroupID;
    p->group_type = GroupType;

    retval = netlink_msg(HYFI_SET_BRPORT_GROUP, nlmsgbuf, sizeof(struct __brport_group), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;

}

int32_t bridgeTableAction( const char* BridgeName, hybridgeTable_e TableType, int32_t* NumEntries, void* TableEntry, hybridgeTableAction_e TableAction )
{
    int32_t retval;
    void *nlmsgbuf = NULL;
    struct __hyctl_msg_header *hymsghdr;
    size_t entrySize;
    u_int32_t action;
    u_int32_t size;

    /* Sanity check */
	if( TableType >= HYFI_BRIDGE_TABLE_LAST || !TableEntry )
	{
    	printf( "%s: Invalid request\n", __FUNCTION__ );
		return -1;
	}

    /* Get the message header pointer. */
    nlmsgbuf = (u_int8_t *)(TableEntry) - HYFI_BRIDGE_MESSAGE_SIZE(0);

    /* Get the table's entry size and get command */
    entrySize = tableParams[ TableType ].size;
    if( TableAction == HYFI_BRIDGE_ACTION_GET )
        action = tableParams[ TableType ].get_command;
    else
        action = tableParams[ TableType ].set_command;

    /* Get the pointer to the hybrid message header */
    hymsghdr = NLMSG_DATA(nlmsgbuf);

    size = *NumEntries * entrySize;

    /* Sanity check, make sure the buffer is large enough */
    if( size > hymsghdr->buf_len )
	{
    	printf( "%s: Buffer too small (requested %d, allocated %d)\n", __FUNCTION__, size, hymsghdr->buf_len );
		return -1;
	}

    /* Get the table data from the hybrid bridge */
    retval = netlink_msg( action, nlmsgbuf, size, tableParams[ TableType ].netlink_key );

    if ( retval == HYFI_STATUS_SUCCESS)
    {
		*NumEntries = hymsghdr->bytes_written/entrySize;
		return 0;
    }
    else
    {
        printf( "%s: netlink failed, error: %s \n", __FUNCTION__, hyctl_status_debug[retval] );
        *NumEntries = hymsghdr->bytes_needed/entrySize;
    }

    return -1;
}

#ifndef DISABLE_APS_HOOKS
/*-F- bridgeAddHATableEntries --
 */
int32_t bridgeAddHATableEntries(const char* BridgeName,  int32_t Hash,
u_int8_t* MAC, u_int8_t* ID, const char* InterfaceName, int32_t TrafficClass, u_int32_t Priority)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hatbl_entry) ) ];
    struct __hatbl_entry *pentry;
    int32_t ifindex;

    if (!(ifindex = if_nametoindex(InterfaceName))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    pentry->hash = Hash;
    memcpy(pentry->da, MAC, ETH_ALEN);
    memcpy(pentry->id, ID, ETH_ALEN);
    pentry->port_list[ 0 ].port = ifindex;
    pentry->sub_class = TrafficClass;
    pentry->priority = Priority;

    retval = netlink_msg(HYFI_ADD_HATBL_ENTRIES, nlmsgbuf, sizeof(struct __hatbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

int32_t bridgeToggleLocalBit( const char* BridgeName, const struct __hatbl_entry *entry )
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hatbl_entry) ) ];
    struct __hatbl_entry *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy( pentry, entry, sizeof( struct __hatbl_entry ) );

    pentry->local = !pentry->local;

    retval = netlink_msg(HYFI_UPDATE_HATBL_ENTRY, nlmsgbuf, sizeof(struct __hatbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetHATableEntries --
 */
int32_t bridgeSetHATableEntries(const char* BridgeName,int32_t Hash,
u_int8_t* MAC, const char* InterfaceName, int32_t TrafficClass, u_int32_t Priority)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hatbl_entry) ) ];
    struct __hatbl_entry *pentry;
    int32_t ifindex;

    if (!(ifindex = if_nametoindex(InterfaceName))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    pentry->hash = Hash;
    memcpy(pentry->da, MAC, ETH_ALEN);
    pentry->port_list[ 0 ].port = ifindex;
    pentry->sub_class = TrafficClass;
    pentry->priority = Priority;
    
    retval = netlink_msg(HYFI_UPDATE_HATBL_ENTRIES, nlmsgbuf, sizeof(struct __hatbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetHATableAggrEntry --
 */
int32_t bridgeSetHATableAggrEntry(const char* BridgeName,int32_t Hash,
u_int8_t* MAC, hyfiBridgeAggrEntry_t *aggrData, int32_t TrafficClass, u_int32_t Priority)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hatbl_entry) ) ];
    struct __hatbl_entry *pentry;
    int32_t ifindex;
    u_int32_t i;

    if( !aggrData->interfaceName[ 0 ] )
        return -1;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    pentry->hash = Hash;
    memcpy(pentry->da, MAC, ETH_ALEN);

    for( i = 0; i < HYFI_AGGR_MAX_IFACE; i++ )
    {
        if( !aggrData->interfaceName[ i ] )
            break;

        if (!(ifindex = if_nametoindex(aggrData->interfaceName[ i ]))) {
            continue;
        }

        pentry->port_list[ i ].port = ifindex;
        pentry->port_list[ i ].quota = aggrData->quota[ i ];
    }
    pentry->sub_class = TrafficClass;
    pentry->priority = Priority;
    pentry->aggr_entry = pentry->port_list[ 0 ].port ? 1 : 0;

    retval = netlink_msg(HYFI_UPDATE_HATBL_ENTRIES, nlmsgbuf, sizeof(struct __hatbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}
/*-F- bridgeDelHATableEntries --
 */
#define INVALID_INTERFACE   "0000"
int32_t bridgeDelHATableEntries(const char* BridgeName,int32_t Hash,
u_int8_t* MAC, int32_t TrafficClass, u_int32_t Priority  )
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hatbl_entry) ) ];
    struct __hatbl_entry *pentry;
    int32_t ifindex = if_nametoindex(INVALID_INTERFACE);

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    pentry->hash = Hash;
    memcpy(pentry->da, MAC, ETH_ALEN);
    pentry->port_list[ 0 ].port = ifindex;
    pentry->sub_class = TrafficClass;
    pentry->priority = Priority;

    retval = netlink_msg(HYFI_UPDATE_HATBL_ENTRIES, nlmsgbuf, sizeof(struct __hatbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}


/*-F- bridgeAddHDTableEntries --
 */
int32_t bridgeAddHDTableEntries(const char* BridgeName, const u_int8_t* MAC,
const u_int8_t* ID, const char* InterfaceUDP, const char* InterfaceOther, int32_t static_entry)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hdtbl_entry) ) ];
    struct __hdtbl_entry *pentry;
    int32_t udp_port, other_port;

    if (!(udp_port = if_nametoindex(InterfaceUDP))) {
        return -1;
    }

    if (!(other_port = if_nametoindex(InterfaceOther))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(pentry->mac_addr, MAC, ETH_ALEN);
    memcpy(pentry->id, ID, ETH_ALEN);
    pentry->udp_port = udp_port;
    pentry->other_port = other_port;
    pentry->static_entry = static_entry;

    retval = netlink_msg(HYFI_ADD_HDTBL_ENTRIES, nlmsgbuf, sizeof(struct __hdtbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetHDTableEntries --
 */
int32_t bridgeSetHDTableEntries(const char* BridgeName, u_int8_t* MAC,
u_int8_t* ID, const char* InterfaceUDP, const char* InterfaceOther)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hdtbl_entry) ) ];
    struct __hdtbl_entry *pentry;
    int32_t udp_port, other_port;

    if (!(udp_port = if_nametoindex(InterfaceUDP))) {
        return -1;
    }

    if (!(other_port = if_nametoindex(InterfaceOther))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(pentry->mac_addr, MAC, ETH_ALEN);
    memcpy(pentry->id, ID, ETH_ALEN);
    pentry->udp_port = udp_port;
    pentry->other_port = other_port;

    retval = netlink_msg(HYFI_UPDATE_HDTBL_ENTRIES, nlmsgbuf, sizeof(struct __hdtbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeDelHDTableEntriesByMAC --
 */
int32_t bridgeDelHDTableEntriesByMAC(const char* BridgeName, const u_int8_t* MAC)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hdtbl_entry) ) ];
    struct __hdtbl_entry *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(pentry->mac_addr, MAC, ETH_ALEN);

    retval = netlink_msg(HYFI_DEL_HDTBL_ENTRIES, nlmsgbuf, sizeof(struct __hdtbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeDelHDTableEntriesByID --
 */
int32_t bridgeDelHDTableEntriesByID(const char* BridgeName, u_int8_t* ID)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __hdtbl_entry) ) ];
    struct __hdtbl_entry *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(pentry->id, ID, ETH_ALEN);

    retval = netlink_msg(HYFI_DEL_HDTBL_ENTRIES_BYID, nlmsgbuf, sizeof(struct __hdtbl_entry), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}


/*-F- bridgeFlushHATable --
 */
int32_t bridgeFlushHATable(const char* BridgeName)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_FLUSH_HATBL, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeFlushHDTable --
 */
int32_t bridgeFlushHDTable(const char* BridgeName)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_FLUSH_HDTBL, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}


/*-F- bridgeSetHATableAgingParams --
 */
int32_t bridgeSetHATableAgingParams(const char* BridgeName, u_int32_t AgingTime  )
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof(struct __aging_param) ) ];
    struct __aging_param *param;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    param = HYFI_MSG_DATA(nlmsgbuf);
    param->aging_time  = AgingTime;

    retval = netlink_msg(HYFI_SET_HATBL_AGING_PARAM, nlmsgbuf, sizeof(struct __aging_param), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}
#endif

/*-F- bridgeSetIFBroadcast --
 */
int32_t bridgeSetIFBroadcast(const char* BridgeName, const char* InterfaceName, u_int32_t BroadcastEnable )
{

    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __brport_group) ) ];
    int32_t ifindex;
    struct __brport_group *p;

    if (!(ifindex = if_nametoindex(InterfaceName))) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    p->ifindex = ifindex;
    p->bcast_enable = BroadcastEnable ? 1 : 0;

    retval = netlink_msg(HYFI_SET_BRPORT_BCAST, nlmsgbuf, sizeof(struct __brport_group), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeSetIFType --
 */
int32_t bridgeSetIFType(const char* BridgeName, const char* InterfaceName, enum __hyInterfaceType InterfaceType )
{

    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __brport_group) ) ];
    int32_t ifindex;
    struct __brport_group *p;

    if (!(ifindex = if_nametoindex(InterfaceName))) {
        return -1;
    }

    if (InterfaceType >= __hyInterface_NumberOfChildInterfaces) {
        return -1;
    }

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    p->ifindex = ifindex;
    p->port_type = InterfaceType;

    retval = netlink_msg(HYFI_SET_BRPORT_TYPE, nlmsgbuf, sizeof(struct __brport_group), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}
/*-F- bridgeSetTcpSP --
 */
int32_t bridgeSetTcpSP(const char* BridgeName, u_int32_t TcpSPEnable )
{

    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( u_int32_t) ) ];
    u_int32_t *p;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    *p = TcpSPEnable;

    retval = netlink_msg(HYFI_SET_BRIDGE_TCP_SP, nlmsgbuf, sizeof( u_int32_t), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeGetLANPortNumber --
 */
int32_t bridgeGetLANPortNumber(const char* BridgeName, const char* MAC, u_int32_t vlanid )
{
    int32_t retval;
    struct __switchport_index *pentry;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( struct __switchport_index) ) ];
    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(pentry->mac_addr, MAC, ETH_ALEN);
    pentry->vlanid = vlanid;
    pentry->portid = 0;

    retval = netlink_msg(HYFI_GET_SWITCH_PORT_ID, nlmsgbuf, sizeof(struct __switchport_index), NETLINK_QCA_HYFI);
    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeGetVersionCompatibility --
 */
int32_t bridgeGetVersionCompatibility(const char* BridgeName, char* app_ver)
{
    int32_t retval;
    char *data;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( strlen(app_ver)+1 ) ];
    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    data = HYFI_MSG_DATA(nlmsgbuf);
    strlcpy(data, app_ver, strlen(app_ver)+1);

    retval = netlink_msg(HYFI_VERSION_COMPATIBILITY_CHECK, nlmsgbuf, strlen(app_ver)+1, NETLINK_QCA_HYFI);

    if (retval == HYFI_STATUS_SUCCESS)
        return 0;
    else if (retval == HYFI_STATUS_FAILURE)
        return 1;
    else
        return -1;
}

/*-F- bridgeGetFourAddressIface -- Get the list of wds_ext ifaces attached to the given bridge
 *
 * BridgeName [in] - bridge on which wds_ext list is searched
 * iface_list [out] - data pointer to list of wds_ext ifaces
 *
 */
int32_t bridgeGetFourAddressIface(const char* BridgeName, void *iface_list) {
    struct WdsExt_iflist *list;
    int32_t retval;
    u_int32_t len=sizeof(struct WdsExt_iflist);

    u_int8_t nlmsgbuf[HYFI_BRIDGE_MESSAGE_SIZE(sizeof(struct WdsExt_iflist))];

    bridgeInitBuf(nlmsgbuf, sizeof(nlmsgbuf), BridgeName);
    list = HYFI_MSG_DATA(nlmsgbuf);

    retval = netlink_msg(HYFI_GET_WDS_EXT_IFACE_LIST, nlmsgbuf, len, NETLINK_QCA_HYFI);
    memcpy(iface_list, list, len);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetEnablePathSwitchParams --
 */
int32_t bridgeSetPathSwitchParam(const char* BridgeName, struct __path_switch_param *PathswitchParam)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof(struct __path_switch_param) ) ];
    struct __path_switch_param *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy( pentry, PathswitchParam, sizeof( struct __path_switch_param ) );

    retval = netlink_msg(HYFI_SET_PATHSWITCH_PARAM, nlmsgbuf, sizeof(struct __path_switch_param), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetPathSwitchAdvancedParam --
 */
int32_t bridgeSetPathSwitchAdvancedParam(const char* BridgeName, u_int32_t type, u_int32_t data )
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof(u_int32_t) ) ];
    struct __path_switch_param *pentry;

    if( type < HYFI_PSW_FIRST || type >= HYFI_PSW_LAST )
        return -1;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);
    memcpy( pentry, &data, sizeof( u_int32_t ) );

    retval = netlink_msg(type, nlmsgbuf, sizeof(u_int32_t), NETLINK_QCA_HYFI);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}


int32_t bridgeSetPathSwitchSwitchEndMarkerTimeout( const char* BridgeName, u_int32_t to )
{
    return bridgeSetPathSwitchAdvancedParam( BridgeName, HYFI_SET_PSW_MSE_TIMEOUT, to );
}

/*-F- bridgeSetEventInfo --
 */
int32_t bridgeSetEventInfo(const char* BridgeName, u_int32_t Pid, u_int32_t Cmd, u_int32_t netlinkKey)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( u_int32_t) ) ];
    u_int32_t *p;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    *p = Pid;

    retval = netlink_msg(Cmd, nlmsgbuf, sizeof( u_int32_t), netlinkKey);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

/*-F- bridgeSetSnoopingParam --
 */
int32_t bridgeSetSnoopingParam(const char* BridgeName, int Cmd, void *MCParam, u_int32_t ParamLen)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE(ParamLen) ];
    void *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);

    memcpy(pentry, MCParam, ParamLen);

    retval = netlink_msg(Cmd, nlmsgbuf, ParamLen, NETLINK_QCA_MC);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeGetSnoopingParam --
 */
int32_t bridgeGetSnoopingParam(const char* BridgeName, int Cmd, void *MCParam, u_int32_t ParamLen)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE(ParamLen) ];
    void *pentry;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    pentry = HYFI_MSG_DATA(nlmsgbuf);

    retval = netlink_msg(Cmd, nlmsgbuf, ParamLen, NETLINK_QCA_MC);

    memcpy(MCParam, pentry, ParamLen);

    if ( retval != HYFI_STATUS_SUCCESS)
        return -1;

    return 0;
}

/*-F- bridgeSetBridgeMapTrafficSeparationMode --
 */
int32_t bridgeSetBridgeMapTrafficSeparationMode(const char* BridgeName, int32_t TSEnabled)
{
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( sizeof( u_int32_t) ) ];
    int32_t *p;

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    p = HYFI_MSG_DATA(nlmsgbuf);
    *p = TSEnabled;

    retval = netlink_msg(HYFI_SET_BRIDGE_TS_MODE, nlmsgbuf, sizeof(u_int32_t), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

int32_t bridgeSetServicePriortizationRuleSet(const char* BridgeName, struct __sp_rule* rule) {
    int32_t retval;
    u_int8_t nlmsgbuf[HYFI_BRIDGE_MESSAGE_SIZE(sizeof(struct __sp_rule))];
    struct __sp_rule* dst_rule;

    bridgeInitBuf(nlmsgbuf, sizeof(nlmsgbuf), BridgeName);

    dst_rule = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(dst_rule, rule, sizeof(struct __sp_rule));

    retval = netlink_msg(HYFI_SET_SP_RULE, nlmsgbuf, sizeof(struct __sp_rule), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

int32_t bridgeFlushServicePriortizationRules(const char* BridgeName) {
    int32_t retval;
    u_int8_t nlmsgbuf[ HYFI_BRIDGE_MESSAGE_SIZE( 0 ) ];

    bridgeInitBuf( nlmsgbuf, sizeof(nlmsgbuf), BridgeName );

    retval = netlink_msg(HYFI_FLUSH_SP_RULES, nlmsgbuf,  0, NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}

int32_t bridgeSetMSCSRule(const char* BridgeName, struct __mscs_rule* rule) {
    int32_t retval;
    u_int8_t nlmsgbuf[HYFI_BRIDGE_MESSAGE_SIZE(sizeof(struct __mscs_rule))];
    struct __mscs_rule* dst_rule;

    bridgeInitBuf(nlmsgbuf, sizeof(nlmsgbuf), BridgeName);

    dst_rule = HYFI_MSG_DATA(nlmsgbuf);
    memcpy(dst_rule, rule, sizeof(struct __mscs_rule));

    retval = netlink_msg(HYFI_SET_MSCS_RULE, nlmsgbuf, sizeof(struct __mscs_rule), NETLINK_QCA_HYFI);

    if (retval != HYFI_STATUS_SUCCESS)
        return -1;
    else
        return 0;
}


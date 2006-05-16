
/* Sanitised ipx.h for net-tools. */

#ifndef _IPX_H_
#define _IPX_H_

#define IPX_NODE_LEN	6
#define IPX_MTU		576

struct sockaddr_ipx {
#if LINUX_VERSION_CODE > 131328	/* 2.1.0 or later */
    sa_family_t sipx_family;
#else
    short sipx_family;
#endif
    unsigned short sipx_port;
    unsigned int sipx_network;
    unsigned char sipx_node[IPX_NODE_LEN];
    unsigned char sipx_type;
    unsigned char sipx_zero;	/* 16 byte fill */
};

#define IPX_FRAME_NONE		0
#define IPX_FRAME_SNAP		1
#define IPX_FRAME_8022		2
#define IPX_FRAME_ETHERII	3
#define IPX_FRAME_8023		4
#define IPX_FRAME_TR_8022	5

#endif

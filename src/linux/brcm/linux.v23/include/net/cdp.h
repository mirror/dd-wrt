/********************************************************************

	Based on documentation from Cisco, detailing the CDP packet format,
 	which is available from:
	http://www.cisco.com/univercd/cc/td/doc/product/lan/trsrb/frames.htm#xtocid842812

	Portions Copyright (c) 2001 Chris Crowther and tom burkart
	The authors admit no liability nor provide any warranty for this
	software.  This material is provided "AS-IS" and at no charge.

	This software is released under the the GNU Public Licence (GPL).

 *******************************************************************/

#ifndef	_NET_CDP_H
#define	_NET_CDP_H

/* timer values */
#define CDP_POLL		HZ*5	/* poll the neighbor list every 5
					 * seconds for expired entires */

/* the packet types */
#define CDP_TYPE_DEVICEID	0x0001
#define CDP_TYPE_ADDRESS 	0x0002
#define CDP_TYPE_PORTID		0x0003
#define CDP_TYPE_CAPABILITIES	0x0004
#define CDP_TYPE_VERSION	0x0005
#define CDP_TYPE_PLATFORM	0x0006
#define CDP_TYPE_IPPREFIX	0x0007

/* the capability masks */
#define CDP_CAPABILITY_L3R	0x01	/* a layer 3 router */
#define CDP_CAPABILITY_L2TB	0x02	/* a layer 2 transparent bridge */
#define CDP_CAPABILITY_L2SRB	0x04	/* a layer 2 source-route bridge */
#define CDP_CAPABILITY_L2SW	0x08	/* a layer 2 switch (non-spanning tree) */
#define CDP_CAPABILITY_L3TXRX	0x10	/* a layer 3 (non routing) host */
#define CDP_CAPABILITY_IGRP	0x20	/* does not forward IGMP Packets to non-routers */
#define CDP_CAPABILITY_L1	0x40	/* a layer 1 repeater */

/* the actual neighbor entry */
struct s_cdp_neighbor {
	unsigned char *remote_ethernet;	/* Remote MAC */
	char *local_iface;		/* Device we saw the packet on*/
	struct timeval timestamp;	/* Time packet arrived */
	unsigned char cdp_proto_ver;	/* Version of CDP */
	unsigned char cdp_ttl;		/* Holdtime */
	unsigned short cdp_checksum;	/* Standard IP packet checksum */
	unsigned char *cdp_version;	/* Software version on neighbor */
	unsigned char *cdp_deviceID;	/* Remote device identification */
	unsigned char *cdp_address;	/* copy of the address info from the
					 * packet */
	unsigned long cdp_capabilities;	/* Capabilities */
	unsigned char *cdp_platform;	/* Model name */
	unsigned char *cdp_portID;	/* Remote device port */
	unsigned char *cdp_prefix;	/* IP address prefix */

	struct s_cdp_neighbor *next;
	struct s_cdp_neighbor *prev;
};

/* head struct */
struct s_cdp_neighbors {
	struct s_cdp_neighbor *head;
	struct s_cdp_neighbor *foot;
};

#endif


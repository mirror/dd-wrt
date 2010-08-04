/*******************************************************************************
 *
 *   Copyright (c) 2008 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 *   Contact Information:
 *   Technology Support <tech@starsemi.com>
 *   Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ********************************************************************************/

#ifndef SWITCH_API_H_K
#define SWITCH_API_H_K


#ifndef __KERNEL__
typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;
typedef int s32;
#else

#include <linux/types.h>

#endif


#define CAVM_OK 0
#define CAVM_ERR 1
#define CAVM_NOT_FOUND 2
#define CAVM_FOUND 3
#define CAVM_FAIL -1 // use minus 

#define MAC_PORT0 0
#define MAC_PORT1 1
#define MAC_PORT2 2
#define CPU_PORT 3

typedef enum 
{


	CNS3XXX_ARL_TABLE_LOOKUP,
	CNS3XXX_ARL_TABLE_ADD,
	CNS3XXX_ARL_TABLE_DEL,
	CNS3XXX_ARL_TABLE_SEARCH,
	CNS3XXX_ARL_TABLE_SEARCH_AGAIN,
	CNS3XXX_ARL_IS_TABLE_END,
	CNS3XXX_ARL_TABLE_FLUSH,

	CNS3XXX_VLAN_TABLE_LOOKUP,
	CNS3XXX_VLAN_TABLE_ADD,
	CNS3XXX_VLAN_TABLE_DEL,
	CNS3XXX_VLAN_TABLE_READ,

	CNS3XXX_SKEW_SET,
	CNS3XXX_SKEW_GET,

	CNS3XXX_BRIDGE_SET,
	CNS3XXX_BRIDGE_GET,

	CNS3XXX_PORT_NEIGHBOR_SET,
	CNS3XXX_PORT_NEIGHBOR_GET,

	CNS3XXX_HOL_PREVENT_SET,
	CNS3XXX_HOL_PREVENT_GET,

	CNS3XXX_TC_SET, // traffic class, for 1, 2, 4, traffic class
	CNS3XXX_TC_GET,

	CNS3XXX_PRI_CTRL_SET,
	CNS3XXX_PRI_CTRL_GET,

	CNS3XXX_DMA_RING_CTRL_SET,
	CNS3XXX_DMA_RING_CTRL_GET,

	CNS3XXX_PRI_IP_DSCP_SET,
	CNS3XXX_PRI_IP_DSCP_GET,

	CNS3XXX_ETYPE_SET,
	CNS3XXX_ETYPE_GET,

	CNS3XXX_UDP_RANGE_SET,
	CNS3XXX_UDP_RANGE_GET,

	CNS3XXX_ARP_REQUEST_SET,
	CNS3XXX_ARP_REQUEST_GET,

	CNS3XXX_RATE_LIMIT_SET,
	CNS3XXX_RATE_LIMIT_GET,

	CNS3XXX_QUEUE_WEIGHT_SET,
	CNS3XXX_QUEUE_WEIGHT_GET,

	CNS3XXX_FC_RLS_SET,
	CNS3XXX_FC_RLS_GET,

	CNS3XXX_FC_SET_SET,
	CNS3XXX_FC_SET_GET,

	CNS3XXX_SARL_RLS_SET,
	CNS3XXX_SARL_RLS_GET,

	CNS3XXX_SARL_SET_SET,
	CNS3XXX_SARL_SET_GET,

	CNS3XXX_SARL_OQ_SET,
	CNS3XXX_SARL_OQ_GET,

	CNS3XXX_SARL_ENABLE_SET,
	CNS3XXX_SARL_ENABLE_GET,

	CNS3XXX_FC_SET,
	CNS3XXX_FC_GET,

	CNS3XXX_IVL_SET,
	CNS3XXX_IVL_GET,

	CNS3XXX_WAN_PORT_SET,
	CNS3XXX_WAN_PORT_GET,

	CNS3XXX_PVID_GET,
	CNS3XXX_PVID_SET,

	CNS3XXX_QA_GET, // queue allocate
	CNS3XXX_QA_SET,

	CNS3XXX_PACKET_MAX_LEN_GET, // set maximun frame length.
	CNS3XXX_PACKET_MAX_LEN_SET,

	CNS3XXX_BCM53115M_REG_READ,
	CNS3XXX_BCM53115M_REG_WRITE,

	CNS3XXX_RXRING_STATUS,
	CNS3XXX_TXRING_STATUS,

	CNS3XXX_DUMP_MIB_COUNTER,

	CNS3XXX_REG_READ,
	CNS3XXX_REG_WRITE,

}CNS3XXXIoctlCmd;

typedef struct
{
	u8 vlan_index;
	u8 valid;
	u16 vid;
	u8 wan_side;
	u8 etag_pmap;
	u8 mb_pmap;
	//u8 my_mac[6];
	u8 *my_mac;
}VLANTableEntry; // for vlan table function

typedef struct
{
	u16 vid;
	u8 pmap;
	//u8 mac[6];
	u8 *mac;
	u8 age_field;
	u8 vlan_mac;
	u8 filter;
}ARLTableEntry; // for arl table function


typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	ARLTableEntry entry;
}CNS3XXXARLTableEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	VLANTableEntry entry;
}CNS3XXXVLANTableEntry; // for ioctl VLAN table ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	u8 enable;
}CNS3XXXHOLPreventControl; 


typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char which_port; // 0, 1, 2, 3 (cpu port)
	unsigned char type; // 0: C-Neighbor, 1: S-Neighbor
}CNS3XXXPortNeighborControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char type; // 0: C-Component, 1: S-Component
}CNS3XXXBridgeControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char tc; // traffic class, for 1, 2, 4, traffic class
}CNS3XXXTrafficClassControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char which_port; // 0, 1, 2, 3 (cpu port)
	unsigned int val;
	unsigned char port_pri;
	unsigned char udp_pri_en;
	unsigned char dscp_pri_en;
	unsigned char vlan_pri_en;
	unsigned char ether_pri_en;
}CNS3XXXPriCtrlControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char ts_double_ring_en;
	unsigned char fs_double_ring_en;
	unsigned char fs_pkt_allocate;
}CNS3XXXDmaRingCtrlControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned int ip_dscp_num; // 0 ~ 63
	unsigned char pri; // 3 bits
}CNS3XXXPriIpDscpControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned int etype_num;
	unsigned int val;
	unsigned int pri;
}CNS3XXXEtypeControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned int udp_range_num;
	unsigned short int port_start;
	unsigned short int port_end;
}CNS3XXXUdpRangeEtypeControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char val; // 0: boradcast forward, 1: redirect to the CPU
}CNS3XXXArpRequestControl;

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char which_port; // 0, 1, 2, 3 (port 0 extra dma)
	unsigned char band_width;
	unsigned char base_rate;
	
}CNS3XXXRateLimitEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char which_port; // 0, 1, 2, 3 (port 0 extra dma)
	unsigned char sch_mode;
	unsigned char q0_w;
	unsigned char q1_w;
	unsigned char q2_w;
	unsigned char q3_w;
}CNS3XXXQueueWeightEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned int val;
	unsigned char tc; // 0-3
	unsigned char gyr; // 0 (green), 1(yellow), 2(red)
}CNS3XXXSARLEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char port; // 0, 1, 2, 3 (cpu port)
	unsigned char fc_en; // 0(rx/tx disable), 1(rx enable), 2(tx enable), 3(rx/tx enable)
}CNS3XXXFCEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char enable; // enable: 1 -> IVL, enable: 0 -> SVL
}CNS3XXXIVLEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char wan_port;
}CNS3XXXWANPortEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char which_port;
	unsigned int pvid;
}CNS3XXXPVIDEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char qa; // queue allocate
}CNS3XXXQAEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	unsigned char max_len; // maximum frame length
}CNS3XXXMaxLenEntry; // for ioctl arl ...

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	u8 page;
	u8 offset;
	u32 u32_val;
	u16 u16_val;
	u8 u8_val;
	u8 data_len;

}CNS3XXXBCM53115M; 

typedef struct 
{
	CNS3XXXIoctlCmd cmd;
	u32 mib[52];
	u16 mib_len;
}CNS3XXXMIBCounter;

#if 0
typedef struct 
{
	CNS3XXXIoctlCmd cmd;
        TXRing *tx_ring;
        RXRing *rx_ring;
}CNS3XXXRingStatus; 
#endif


#endif

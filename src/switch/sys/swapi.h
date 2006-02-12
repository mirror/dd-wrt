/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Switch API calls compatible with Robo, Strata, and XGS
 * device driver API's and SDK's.
 *
 * This module provides a high-level programming abstraction
 * for programming Layer-2 ethernet switch ASIC's.
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef __BCM_SWAPI_H
#define __BCM_SWAPI_H

#include <etc53xx.h>
#include <if_robo.h>
#include <nvports.h>
extern robo_driver_t *robo_driver;

/*
 * Port bitmap used throughout the BCM53xx/56xx switch chip family
 */
#define DEFAULT_WAN_VLAN 2
#define DEFAULT_LAN_VLAN 1
/*
 * Port bitmap used throughout the BCM53xx/56xx switch chip family
 */
#define BCM_MIN_PORT 1
#define BCM_MAX_PORT 9
#define BCM_MII_ARL_UC_PORT 9 /* note ARL table UC port same for 5380 & 5365 */
#ifdef BCM5380
#define BCM_MII_PORT 9
#define BCM_NUM_PORTS 8
#else
#define BCM_MII_PORT 6
#define BCM_NUM_PORTS 5
#endif
#define BCM_WAN_PORT BCM_MIN_PORT
#define BCM_LAN_MIN_PORT (BCM_MIN_PORT+1)
#define BCM_LAN_MAX_PORT BCM_NUM_PORTS
#define BCM_DEF_WAN_VLAN 6
#define BCM_DEF_LAN_VLAN 1
#define BRIDGE_GROUP_MAC_ADDR {0x01,0x80,0xc2,0x00,0x00,0x00}
#define IGMP_MAC_ADDR {0x01,0x00,0x5e,0x00,0x00,0x00}
#define PBMP_NONE		0
#define PBMP_PORT(port)		(1U << (port))
#define PBMP_RANGE(lp, hp)	(PBMP_PORT((hp) + 1) - PBMP_PORT(lp))
#define PBMP_MEMBER(bmp, port)	(((bmp) & PBMP_PORT(port)) != 0)
#define PBMP_INVALID		0xffffffff

typedef int                     bcm_port_t;     /* Port number */
typedef unsigned int            bcm_pbmp_t;     /* Port bitmap (1<<portno)*/
typedef int                     bcm_vlan_t;     /* VLAN ID integer */
typedef int                     bcm_trunk_t;    /* Trunk ID integer */

/*
 * MAC and IP addresses
 */

typedef uint8   bcm_mac_t[6];
typedef uint32  bcm_ip_t;


/*
 * Switch Port configuration structure.
 */
typedef struct bcm_port_info_s{
    int    link;              /* link or not*/
    int    porten_tx;	      /* port enable ==> tx*/
    int    porten_rx;	      /* port enable ==> rx*/
    int    anmode;	          /* AN */
    int    speed;             /*  Initial speed*/
    int    dpx;               /* Dpx bit*/
    int    flow;              /* Flow control */
    int    autocast;	      /* Autocast enable/disable */
    int    portmirror;	      /* port mirror */
    int    stp_state;         /* Spanning Tree state */
} bcm_port_info_t;

/*
 * API return values
 */
#define BCM_RET_SUCCESS          0
#define BCM_RET_FAIL            -1
#define BCM_RET_TIMEOUT         -2
#define BCM_RET_INVALID_VID     -3
#define BCM_RET_ARL_TABLE_FULL  -4
#define BCM_RET_SEMA_UNAVAIL    -5

/*
 * busy wait timeout values
 */
#define BCM_TIMEOUT_VAL 100

/*
 * semaphore timeout values
 */
#define BCM_SEM_TIMEOUT_VAL 1000

/*
 * API init/deinit calls.
 */
extern void *bcm_api_init(void);
extern void bcm_api_deinit(void);

/*
 * get robo device type call
 */
extern int bcm_get_robo_devtype(void);
extern int bcm_is_robo(void);


/*
 * semaphore calls.
 */
extern int bcm_get_sema(void);
extern int bcm_rel_sema(void);

/*
 * port/interface map calls.
 */
extern int bcm_add_port_interface(uint port, uint iface);
typedef int (*BCM_GET_PORT_FROM_INTERFACE)(uint iface, uint *port);
extern int bcm_get_port_from_interface(uint iface, uint *port);

/*
 * read/write reg calls
 */
extern int bcm_write_reg(int unit, uint8 page, uint8 addr, uint8 *bufptr, uint len);
typedef int (*BCM_WRITE_REG)(int unit, uint8 page, uint8 addr, uint8 *bufptr, uint len);
extern int bcm_read_reg(int unit, uint8 page, uint8 addr, uint8 *bufptr, uint len);
extern int bcm_write_reg_no_sema(int unit, uint8 page, uint8 addr, uint8 *bufptr, uint len);
extern int bcm_read_reg_no_sema(int unit, uint8 page, uint8 addr, uint8 *bufptr, uint len);

/*
 * Strata/XGS compatible Port API calls.
 */
extern int bcm_port_speed_get(bcm_port_t port, int *speed);
extern int bcm_port_speed_set(bcm_port_t port, int speed);
extern int bcm_port_autoneg_get(bcm_port_t port, int *autoneg);
extern int bcm_port_autoneg_set(bcm_port_t port, int autoneg);
extern int bcm_port_restart_autoneg(bcm_port_t port);
extern int bcm_port_duplex_get(bcm_port_t port, int *duplex);
extern int bcm_port_duplex_set(bcm_port_t port, int duplex);
extern int bcm_port_pause_get(bcm_port_t port,
			      int *pause_tx, int *pause_rx);
extern int bcm_port_pause_set(bcm_port_t port,
			      int pause_tx, int pause_rx);
extern int bcm_port_loopback_set(bcm_port_t port, int loopback);
extern int bcm_port_loopback_get(bcm_port_t port, int *loopback);
extern int bcm_port_enable_set(bcm_port_t port, int enable);
extern int bcm_port_enable_get(bcm_port_t port, int *enable);
extern int bcm_port_stp_get(bcm_port_t port, int *state);
typedef int (*BCM_PORT_STP_SET)(bcm_port_t port, int state);
extern int bcm_port_stp_set(bcm_port_t port, int state);
extern int bcm_port_link_status_get(bcm_port_t port, int *up);
extern int bcm_port_info_get(bcm_port_t port,
			     bcm_port_info_t* portinfo);
extern int bcm_port_poweroff_get(bcm_port_t port, int *poweroff);
extern int bcm_port_poweroff_set(bcm_port_t port, int poweroff);
typedef struct _ROBO_MII_AN_ADVERT_STRUC bcm_an_advert_t;
extern int bcm_port_advert_get(bcm_port_t port,
			     bcm_an_advert_t* an_advert);
extern int bcm_port_advert_set(bcm_port_t port,
			     bcm_an_advert_t* an_advert);
extern int bcm_port_advert_remote_get(bcm_port_t port,
			     bcm_an_advert_t* an_advert);
extern int bcm_set_port_attributes(PORT_ATTRIBS *port_attribs, uint portid);

/* Spanning Tree port states */
typedef enum bcm_port_stp_e {
    BCM_PORT_STP_NONE    = 0,
    BCM_PORT_STP_DISABLE = 1,
    BCM_PORT_STP_BLOCK   = 2,
    BCM_PORT_STP_LISTEN  = 3,
    BCM_PORT_STP_LEARN   = 4,
    BCM_PORT_STP_FORWARD = 5,
} bcm_port_stp_t;


/*      SNMP MIB Statistics API, retrieve counters from Switch H/W ASIC.
 *	The following enumerated types designate the SNMP variables as
 *	defined by the RFCs.
 */

typedef enum bcm_stat_val_e {
#define ROBO_MIB_NOT_DEFINED 256
#define ROBO_MIB_SUM1 257
#define ROBO_MIB_SUM2 258
#define ROBO_MIB_SUM3 259
#define ROBO_MIB_SUM4 260
#define ROBO_MIB_SUM5 261
#define ROBO_MIB_SUM6 262
#define ROBO_MIB_SUM7 263
#define ROBO_MIB_SUM8 264
#define ROBO_MIB_SUM9 265
#define ROBO_MIB_SUM1_REGS  {ROBO_MIB_RX_MC_PKTS, ROBO_MIB_RX_BC_PKTS}
#define ROBO_MIB_SUM2_REGS  {ROBO_MIB_RX_ALIGNMENT_ERRORS, ROBO_MIB_RX_FCS_ERRORS,  \
                            ROBO_MIB_RX_FRAGMENTS, ROBO_MIB_RX_OVER_SIZE_PKTS,  \
                            ROBO_MIB_RX_JABBERS}
#define ROBO_MIB_SUM3_REGS  {ROBO_MIB_TX_MC_PKTS, ROBO_MIB_TX_BC_PKTS}
#define ROBO_MIB_SUM4_REGS  {ROBO_MIB_EXCESS_COLLISIONS, ROBO_MIB_TX_LATE_COLLISIONS}
#define ROBO_MIB_SUM5_REGS  {ROBO_MIB_RX_UC_PKTS, ROBO_MIB_RX_MC_PKTS,   \
                            ROBO_MIB_RX_BC_PKTS}
#define ROBO_MIB_SUM6_REGS  {ROBO_MIB_TX_UC_PKTS, ROBO_MIB_TX_MC_PKTS,   \
                            ROBO_MIB_TX_BC_PKTS}
#define ROBO_MIB_SUM7_REGS  {ROBO_MIB_RX_DROP_PKTS, ROBO_MIB_TX_DROP_PKTS}
#define ROBO_MIB_SUM8_REGS  {ROBO_MIB_RX_UC_PKTS, ROBO_MIB_RX_MC_PKTS,   \
                            ROBO_MIB_RX_BC_PKTS, ROBO_MIB_RX_ALIGNMENT_ERRORS, \
                            ROBO_MIB_RX_FCS_ERRORS,ROBO_MIB_RX_FRAGMENTS, \
                            ROBO_MIB_RX_OVER_SIZE_PKTS, ROBO_MIB_RX_JABBERS}
#define ROBO_MIB_SUM9_REGS  {ROBO_MIB_RX_ALIGNMENT_ERRORS,/* CRC ERRORS? */}
    /*** RFC 1213 ***/

    snmpIfInOctets = ROBO_MIB_RX_OCTETS,
    snmpIfInUcastPkts = ROBO_MIB_RX_UC_PKTS,
    snmpIfInNUcastPkts = ROBO_MIB_SUM1,
    snmpIfInDiscards = ROBO_MIB_RX_DROP_PKTS,
    snmpIfInErrors = ROBO_MIB_SUM2,
    snmpIfInUnknownProtos = ROBO_MIB_NOT_DEFINED,
    snmpIfOutOctets = ROBO_MIB_TX_OCTETS,
    snmpIfOutUcastPkts = ROBO_MIB_TX_UC_PKTS,
    snmpIfOutNUcastPkts = ROBO_MIB_SUM3,
    snmpIfOutDiscards = ROBO_MIB_TX_DROP_PKTS,
    snmpIfOutErrors = ROBO_MIB_SUM4,
    snmpIpForwDatagrams = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIpInReceives = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIpInDiscards = ROBO_MIB_NOT_DEFINED,  /* ? */

    /*** RFC 1493 ***/

    snmpDot1dBasePortDelayExceededDiscards = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpDot1dBasePortMtuExceededDiscards = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpDot1dTpPortInFrames = ROBO_MIB_SUM5,
    snmpDot1dTpPortOutFrames = ROBO_MIB_SUM6,
    snmpDot1dPortInDiscards = ROBO_MIB_RX_DROP_PKTS,

    /*** RFC 1757 (EtherStat) ***/

    snmpEtherStatsDropEvents = ROBO_MIB_SUM7,
    snmpEtherStatsMulticastPkts = ROBO_MIB_TX_MC_PKTS,
    snmpEtherStatsBroadcastPkts = ROBO_MIB_TX_BC_PKTS,
    snmpEtherStatsUndersizePkts = ROBO_MIB_RX_UNDER_SIZE_PKTS,
    snmpEtherStatsFragments = ROBO_MIB_RX_FRAGMENTS,
    snmpEtherStatsPkts64Octets = ROBO_MIB_RX_PKTS_64,
    snmpEtherStatsPkts65to127Octets = ROBO_MIB_RX_PKTS_65_TO_127,
    snmpEtherStatsPkts128to255Octets = ROBO_MIB_RX_PKTS_128_TO_255,
    snmpEtherStatsPkts256to511Octets = ROBO_MIB_RX_PKTS_256_TO_511,
    snmpEtherStatsPkts512to1023Octets = ROBO_MIB_RX_PKTS_512_TO_1023,
    snmpEtherStatsPkts1024to1518Octets = ROBO_MIB_RX_PKTS_1024_TO_1522,
    snmpEtherStatsOversizePkts = ROBO_MIB_RX_OVER_SIZE_PKTS,
    snmpEtherStatsJabbers = ROBO_MIB_RX_OVER_SIZE_PKTS,
    snmpEtherStatsOctets = ROBO_MIB_RX_OCTETS,
    snmpEtherStatsPkts = ROBO_MIB_SUM8,
    snmpEtherStatsCollisions = ROBO_MIB_TX_COLLISIONS,
    snmpEtherStatsCRCAlignErrors = ROBO_MIB_SUM9,
    snmpEtherStatsTXNoErrors = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpEtherStatsRXNoErrors = ROBO_MIB_NOT_DEFINED,  /* ? */

    /*** RFC 2665 ***/

    snmpDot3StatsAlignmentErrors = ROBO_MIB_RX_ALIGNMENT_ERRORS,
    snmpDot3StatsFCSErrors = ROBO_MIB_RX_FCS_ERRORS,
    snmpDot3StatsSingleCollisionFrames = ROBO_MIB_TX_SINGLE_COLLISIONS,
    snmpDot3StatsMultipleCollisionFrames = ROBO_MIB_TX_MULTI_COLLISIONS,
    snmpDot3StatsSQETTestErrors = ROBO_MIB_NOT_DEFINED,
    snmpDot3StatsDeferredTransmissions = ROBO_MIB_TX_DEFER_TX,
    snmpDot3StatsLateCollisions = ROBO_MIB_TX_LATE_COLLISIONS,
    snmpDot3StatsExcessiveCollisions = ROBO_MIB_EXCESS_COLLISIONS,
    snmpDot3StatsInternalMacTransmitErrors = ROBO_MIB_TX_DROP_PKTS,
    snmpDot3StatsCarrierSenseErrors = ROBO_MIB_NOT_DEFINED,
    snmpDot3StatsFrameTooLongs = ROBO_MIB_RX_OVER_SIZE_PKTS,
    snmpDot3StatsInternalMacReceiveErrors = ROBO_MIB_RX_DROP_PKTS,
    snmpDot3StatsSymbolErrors = ROBO_MIB_RX_SYMBOL_ERROR,

    /*** RFC 2233 ***/

    snmpIfHCInOctets = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCInUcastPkts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCInMulticastPkts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCInBroadcastPkts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCOutOctets = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCOutUcastPkts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCOutMulticastPkts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpIfHCOutBroadcastPckts = ROBO_MIB_NOT_DEFINED,  /* ? */

    /*** Additional Broadcom stats ***/

    snmpBcmIPMCBridgedPckts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpBcmIPMCRoutedPckts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpBcmIPMCInDroppedPckts = ROBO_MIB_NOT_DEFINED,  /* ? */
    snmpBcmIPMCOutDroppedPckts = ROBO_MIB_NOT_DEFINED,  /* ? */

    snmpValCount
} bcm_stat_val_t;

extern int bcm_stat_init(int unit);

extern char *bcm_stat_name(bcm_stat_val_t type);

extern int bcm_stat_clear(bcm_port_t port);
extern int bcm_stat_clear_all(int unit);

extern int bcm_stat_get(bcm_port_t port, 
			bcm_stat_val_t type, uint64 *value);

extern int bcm_stat_get32(bcm_port_t port, 
			  bcm_stat_val_t type, uint32 *value);



/*
 * 802.1Q and Port Based VLAN support API's
 */
extern int bcm_vlan_enable(int unit, int enable);

extern int bcm_vlan_create(int unit, bcm_vlan_t vid);

extern int bcm_vlan_destroy(int unit, bcm_vlan_t vid);
extern int bcm_vlan_destroy_all(int unit);

extern int bcm_vlan_port_add(int unit,
			     bcm_vlan_t vid,
			     bcm_pbmp_t pbmp, bcm_pbmp_t ubmp, int addDefaultTag);

extern int bcm_vlan_port_remove(int unit,
				bcm_vlan_t vid,
				bcm_pbmp_t pbmp);

extern int bcm_vlan_port_get(int unit,
			     bcm_vlan_t vid,
			     bcm_pbmp_t *pbmp, bcm_pbmp_t *ubmp);

/* Native VLAN Support (the VLAN ID the switch should tag an untagged frame
 * with as the packet ingresses the switch fabric).
 */

extern int bcm_vlan_default_get(int unit, bcm_vlan_t *vid_ptr);
extern int bcm_vlan_default_set(int unit, bcm_vlan_t vid);


/*
 * Layer-2 Support
 * Device-independent structures for representing an L2 address
 */

#define BCM_L2_COS_SRC_PRI	0x00000001  /* src COS has prio over dst COS */
#define	BCM_L2_DISCARD_SRC	0x00000002
#define	BCM_L2_DISCARD_DST	0x00000004
#define	BCM_L2_COPY_TO_CPU	0x00000008
#define	BCM_L2_L3LOOKUP		0x00000010
#define BCM_L2_STATIC		0x00000020
#define BCM_L2_HIT		0x00000040
#define BCM_L2_TRUNK_MEMBER	0x00000080
#define BCM_L2_MCAST		0x00000100

typedef struct bcm_l2_addr_s {
    uint32              flags;          /* BCM_L2_XXX flags */
    bcm_mac_t           mac;            /* 802.3 MAC address */
    bcm_vlan_t          vid;            /* VLAN identifier */
    int                 port;           /* Zero-based port number */
    int                 unit;           /* Unit on which port occurs */
    int                 cpu;            /* CPU reference for higher layers */
    bcm_trunk_t         tgid;           /* Trunk group ID */
    bcm_pbmp_t          pbmp;           /* port bitmap for multicast l2 addrs */
} bcm_l2_addr_t;


/* API structures for dump operations */
typedef struct bcm_arl_table_entry_s {
  bcm_mac_t                 MACaddr;
  int                       port;
  int                       staticAddr;
  int                       vid;
  int                       highPrio;
  int                       age;
  int                       arl_addr;
} bcm_arl_table_entry_t;

typedef struct bcm_vlan_table_entry_s {
  ROBO_VLAN_READ_WRITE_STRUC vlan_entry;
  int                        vlan_addr;
  int                        vlan_index;
  int                        vlan_id;
} bcm_vlan_table_entry_t;


/*
 * dump valid entries of ARL table
 */
extern int bcm_l2_addr_dumptable(int unit, bcm_arl_table_entry_t *table, int *numEntries);

/*
 * dump valid entries of VLAN table
 */
extern int bcm_vlan_dumptable(int unit, bcm_vlan_table_entry_t *table, int *numEntries);

/*
 * clear all entries of ARL table
 */
int
bcm_l2_addr_cleartable(int unit);

/*
 * Initialize a bcm_l2_addr_t to a specified MAC address and VLAN,
 * zeroing all other fields.
 */
extern void bcm_l2_addr_init(bcm_l2_addr_t *l2addr,
			     bcm_mac_t mac_addr, bcm_vlan_t vid);

/*
 * Add address to L2 table
 */
extern int bcm_l2_addr_add(int unit, bcm_l2_addr_t *l2addr);

/*
 * Bit swapping routines
 */
void bcm_bitswap(bcm_mac_t *macAddr);
uint bcm_bitswap_vlan(uint vlan);
extern void bcm_byteswap(bcm_mac_t *macAddr);

/*
 * Remove address(es) from L2 table
 */
#define BCM_L2_REMOVE_STATIC	0x00000001

extern int bcm_l2_addr_remove(int unit,
			      bcm_mac_t mac_addr, bcm_vlan_t vid);
extern int bcm_l2_addr_remove_by_port(int unit,
				      bcm_pbmp_t pbmp, uint32 flags);
extern int bcm_l2_addr_remove_by_vlan(int unit,
				      bcm_vlan_t vid, uint32 flags);

/*
 * Look up address in L2 table
 */
extern int bcm_l2_addr_get(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
			   bcm_l2_addr_t *l2addr);


/*
 * Dump CAM table
 */
#define CAM_ENTRIES 64

typedef struct cam_table_s {
    uint8   addr[CAM_ENTRIES][6];
    uint    valid[CAM_ENTRIES];
} cam_table_t;

extern int bcm_cam_dumptable(cam_table_t *cam_table);

/*
 * MIB Autocast
 */
extern int bcm_update_MIB_AC_stats(uint port);
extern int brcm_get_mibac_reg(int port, int offset, int size, uint8 *regval);
extern void brcm_get_mibac_stats(int port, int *count, ROBO_MIB_AC_STRUCT *stats);

/* Mirroring API */

#define BCM_MIRROR_DISABLE  0
#define BCM_MIRROR_ENABLE   1

extern int bcm_mirror_mode(int unit, int mode);
extern int bcm_mirror_to_set(int unit, bcm_port_t port);
extern int bcm_mirror_to_get(int unit, bcm_port_t *port);
extern int bcm_mirror_ingress_set(int unit, bcm_port_t port, int val);
extern int bcm_mirror_ingress_get(int unit, bcm_port_t port, int *val);
extern int bcm_mirror_egress_set(int unit, bcm_port_t port, int val);
extern int bcm_mirror_egress_get(int unit, bcm_port_t port, int *val);
extern int bcm_mirror_to_pbmp_set(int unit, bcm_port_t port, bcm_pbmp_t pbmp);
extern int bcm_mirror_to_pbmp_get(int unit, bcm_port_t port, bcm_pbmp_t *pbmp);

/* initialize linkscan */
extern void bcm_linkscan_init(robo_info_t *robo, int milliseconds);

/* register/unregister brcm tag device */
extern int bcm_reg_brcmtag_dev(char *base_name, char *suffix, uint port);
extern int bcm_unreg_brcmtag_dev(char *dev_name);

/* macro to get port from interface name */
#define bcm_get_port(ifname,vlan) do {  \
  char *curptr = NULL;                  \
  uint iface;                           \
  if ((curptr = strrchr(ifname,'.'))) { \
    if (!sscanf(++curptr,"%d",&iface))  \
      *(vlan) = 0;                      \
    else \
      if (pbcm_get_port_from_interface != NULL) \
        pbcm_get_port_from_interface(iface,vlan); \
  } else                                \
    *(vlan) = 0;                        \
  } while (0);

/* macros to allow for accessing API from user or kernel mode */
/* note that in user mode the 'handle' argument is the module file descriptor */
/* but in kernel mode, it's the robo device handle */
#ifdef __KERNEL__
#define ROBO_RREG(unit,page,addr,bufptr,len) (*(robo_driver->robo_read))(robo,unit,page, \
		addr,bufptr,len)
#define ROBO_WREG(unit,page,addr,bufptr,len) (*(robo_driver->robo_write))(robo,unit,page, \
		addr,bufptr,len)
#else
#define ROBO_RREG(runit,rpage,raddr,bufptr,rlen) bcm_read_reg_no_sema(runit,  \
        rpage, raddr, bufptr, rlen);
#define ROBO_WREG(runit,rpage,raddr,bufptr,rlen) bcm_write_reg_no_sema(runit,  \
        rpage, raddr, bufptr, rlen);
#endif

/* Port types */
#define ROBO_PORT_TYPE_NONE    (0)
#define ROBO_PORT_TYPE_FE      (1<<0)
#define ROBO_PORT_TYPE_MII     (1<<1)
#define ROBO_PORT_TYPE_SMP     (1<<2)
#define ROBO_PORT_TYPE_GE      (1<<3)

void bcm_get_vaddr(bcm_vlan_t vid, uint *vaddr, uint *vindex);
void bcm_get_vid(bcm_vlan_t *vid, uint vaddr, uint vindex);

/* Port Monitor (linkscan) support 
 * Linkscan tasks, checks link on all switch ports
 * and indicates link up/down event to netif associated
 * with each port/vlan.
 */
typedef
struct robo_ls_data_s{
    robo_info_t* robo;
    int milliseconds;
    unsigned int active_mask;
} robo_ls_data_t;

#ifdef _CFE_
void bcm_linkscan(unsigned long dp);
#endif 
void bcm_linkscan_init(robo_info_t *robo, int milliseconds);


/* Port map support */
int bcm_get_num_ports(void);

typedef struct robo_port_map_s {
    int cid;        /* Chip ID: zero for single switch designs */
    uint32 flags;   /* descriptor */
} robo_port_map_t;

#define NUM_PORTS (sizeof(robo_port_map)/sizeof(robo_port_map[0]))
#define NUM_SWITCH_PORTS NUM_PORTS-1 /* w/o MII */
#define hwport(x) (x-1)              /* where 'x' is sw port */
#define swport(x) (x+1)              /* where 'x' is hw port */
#define portcap(x) (robo_port_map[(x)].flags) /* where 'x' is hw port */
#define portcid(x) (robo_port_map[(x)].cid)   /* where 'x' is hw port */
#define PBMP_PORT(port)    (1U << (port))     /* where 'port' is hw port */
#define IF_PORT_EXISTS(x) (x<NUM_PORTS && portcap(x)) /* 'x' is hw port */


#endif /* !__BCM_SWAPI_H */

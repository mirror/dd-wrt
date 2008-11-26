#include <Copyright.h>

/********************************************************************************
* msApiDefs.h
*
* DESCRIPTION:
*       API definitions for QuarterDeck Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiDefs_h
#define __msApiDefs_h

#ifdef __cplusplus
extern "C" {
#endif
#include <msApiTypes.h>
/* Debug function   */

#ifdef DEBUG_QD
#define DBG_INFO(x) gtDbgPrint x
#else
#define DBG_INFO(x);
#endif /* DEBUG_QD */

typedef GT_U32 GT_SEM;

#define ETHERNET_HEADER_SIZE	GT_ETHERNET_HEADER_SIZE
#define IS_MULTICAST_MAC		GT_IS_MULTICAST_MAC
#define IS_BROADCAST_MAC		GT_IS_BROADCAST_MAC

#define GT_INVALID_PORT			0xFF
#define GT_INVALID_PORT_VEC		0xFFFFFFFF

/*
 *   Logical Port value based on a Port
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |0|  reserved                                   |    port       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *   The following macros should be used to extract specific info
 *   from a Logical Port index
 */
typedef GT_U32 GT_LPORT;
typedef GT_U32 GT_ETYPE;


/* Define the different device type that may exist in system        */
typedef enum
{
    GT_88E6021  = 0x021,
    GT_88E6051  = 0x051,
    GT_88E6052  = 0x052,
    GT_88E6060  = 0x060,
    GT_88E6063	= 0x153,
    GT_FH_VPN	= 0xF53,
    GT_FF_EG	= 0xF91,
    GT_FF_HG	= 0xF93,
	GT_88E6083	= 0x083,

	GT_88E6181	= 0x1A0,

	GT_88E6153	= 0x1A1,
	GT_88E6183	= 0x1A3,

	GT_88E6093	= 0x093,
	GT_88E6092	= 0x097,
	GT_88E6045	= 0x094,
	GT_88E6095	= 0x095,

	GT_88E6152	= 0x1A4,
	GT_88E6155	= 0x1A5,
	GT_88E6182	= 0x1A6,
	GT_88E6185	= 0x1A7,

	GT_88E6121	= 0x104,
	GT_88E6122	= 0x105,
	GT_88E6131	= 0x106,
	GT_88E6108	= 0x107,

	GT_88E6031	= 0x031,
	GT_88E6035	= 0x035,
	GT_88E6055	= 0x055,
	GT_88E6061	= 0x061,
	GT_88E6065	= 0x065,

	GT_88E6046	= 0x048,
	GT_88E6047	= 0x049,
	GT_88E6085	= 0x04A,
	GT_88E6096	= 0x098,
	GT_88E6097	= 0x099,

	GT_88E6123	= 0x121,
	GT_88E6125	= 0x125,
	GT_88E6140	= 0x141,
	GT_88E6161	= 0x161,
	GT_88E6165	= 0x165

}GT_DEVICE;


/* Definition for the revision number of the device        */
typedef enum
{
	GT_REV_0 = 0,
	GT_REV_1,
	GT_REV_2,
	GT_REV_3
}GT_DEVICE_REV;


/* ToDo: No Used */
typedef enum
{
    INTR_MODE_DISABLE =0,
    INTR_MODE_ENABLE
}INTERRUPT_MODE;

/* Definition for the Port Speed */
typedef enum
{
	PORT_SPEED_10_MBPS = 0,
	PORT_SPEED_100_MBPS = 1,
	PORT_SPEED_200_MBPS = 2,	/* valid only if device support */
	PORT_SPEED_1000_MBPS = 2 ,	/* valid only if device support */
	PORT_SPEED_UNKNOWN = 3
} GT_PORT_SPEED_MODE;

/* Definition for the forced Port Speed */
typedef enum
{
	PORT_FORCE_SPEED_10_MBPS = 0,
	PORT_FORCE_SPEED_100_MBPS = 1,
	PORT_FORCE_SPEED_200_MBPS = 2,	/* valid only if device support */
	PORT_FORCE_SPEED_1000_MBPS = 2,	/* valid only if device support */
	PORT_DO_NOT_FORCE_SPEED =3
} GT_PORT_FORCED_SPEED_MODE;

/* Definition for the forced Port Duplex mode */
typedef enum
{
	PORT_DO_NOT_FORCE_DUPLEX,
	PORT_FORCE_FULL_DUPLEX,
	PORT_FORCE_HALF_DUPLEX
} GT_PORT_FORCED_DUPLEX_MODE;

/* Definition for the forced Port Link */
typedef enum
{
	PORT_DO_NOT_FORCE_LINK,
	PORT_FORCE_LINK_UP,
	PORT_FORCE_LINK_DOWN
} GT_PORT_FORCED_LINK_MODE;

/* Definition for the forced flow control mode */
typedef enum
{
	PORT_DO_NOT_FORCE_FC,
	PORT_FORCE_FC_ENABLED,
	PORT_FORCE_FC_DISABLED
} GT_PORT_FORCED_FC_MODE;

/* Definition for the PPU state */
typedef enum
{
	PPU_STATE_DISABLED_AT_RESET,
	PPU_STATE_ACTIVE,
	PPU_STATE_DISABLED_AFTER_RESET,
	PPU_STATE_POLLING
} GT_PPU_STATE;


/*
 * Typedef: enum GT_PORT_CONFIG_MODE
 *
 * Description: Defines port's interface type configuration mode determined at
 *				reset. This definition may not represent the device under use.
 *				Please refer to the device datasheet for detailed information.
 *
 */
typedef enum
{
	PORTCFG_GMII_125MHZ = 0,		/* Px_GTXCLK = 125MHz, 1000BASE */
	PORTCFG_FD_MII_0MHZ = 1,		/* Px_GTXCLK = 0 MHz, Power Save */
	PORTCFG_FDHD_MII_25MHZ = 2,		/* Px_GTXCLK = 25MHz, 100BASE */
	PORTCFG_FDHD_MII_2_5MHZ = 3,	/* Px_GTXCLK = 2.5MHz, 10BASE */
	PORTCFG_FD_SERDES = 4,			/* Default value */
	PORTCFG_FD_1000BASE_X = 5,
	PORTCFG_MGMII = 6,				/* duplex, speed determined by the PPU */
	PORTCFG_DISABLED = 7
} GT_PORT_CONFIG_MODE;


typedef enum
{
	GT_SA_FILTERING_DISABLE = 0,
	GT_SA_DROP_ON_LOCK,
	GT_SA_DROP_ON_UNLOCK,
	GT_SA_DROP_TO_CPU
} GT_SA_FILTERING;


/* Definition for the Ingree/Egress Frame Mode */
typedef enum
{
	GT_FRAME_MODE_NORMAL = 0,	/* Normal Network */
	GT_FRAME_MODE_DSA,			/* Distributed Switch Architecture */
	GT_FRAME_MODE_PROVIDER,		/* Provider */
	GT_FRAME_MODE_ETHER_TYPE_DSA	/* Ether Type DSA */
} GT_FRAME_MODE;

/*
 * Typedef: enum GT_JUMBO_MODE
 *
 * Description: Defines Jumbo Frame Size allowed to be tx and rx
 *
 * Fields:
 *      GT_JUMBO_MODE_1522 - Rx and Tx frames with max byte of 1522.
 *      GT_JUMBO_MODE_2048 - Rx and Tx frames with max byte of 2048.
 *      GT_JUMBO_MODE_10240 - Rx and Tx frames with max byte of 10240.
 *
 */
typedef enum
{
    GT_JUMBO_MODE_1522 = 0,
	GT_JUMBO_MODE_2048,
	GT_JUMBO_MODE_10240
} GT_JUMBO_MODE;


/*
 * Typedef: enum GT_PRI_OVERRIDE
 *
 * Description: Defines the priority override
 *
 * Fields:
 *      PRI_OVERRIDE_NONE - Normal frame priority processing occurs.
 *		PRI_OVERRIDE_FRAME_QUEUE -
 *			Both frame and queue overrides take place on the frame.
 *      PRI_OVERRIDE_FRAME -
 *			Overwite the frame's FPri (frame priority).
 *			If the frame egresses tagged, the priority in the frame will be
 *			the overwritten priority value.
 *		PRI_OVERRIDE_QUEUE -
 *			Overwite the frame's QPri (queue priority).
 *			QPri is used internally to map the frame to one of the egress
 *			queues inside the switch.
 *
 */
typedef enum
{
    PRI_OVERRIDE_NONE = 0,
    PRI_OVERRIDE_FRAME_QUEUE,
	PRI_OVERRIDE_FRAME,
	PRI_OVERRIDE_QUEUE
} GT_PRI_OVERRIDE;


/*
 * Typedef: enum GT_FRAME_POLICY
 *
 * Description: Defines the policy of the frame
 *
 * Fields:
 *      FRAME_POLICY_NONE - Normal frame switching
 *      FRAME_POLICY_MIRROR - Mirror(copy) frame to the MirrorDest port
 *      FRAME_POLICY_TRAP - Trap(re-direct) frame to the CPUDest port
 *      FRAME_POLICY_DISCARD - Discard(filter) the frame
 *
 */
typedef enum
{
    FRAME_POLICY_NONE = 0,
	FRAME_POLICY_MIRROR,
	FRAME_POLICY_TRAP,
    FRAME_POLICY_DISCARD
} GT_FRAME_POLICY;


/*
 * Typedef: enum GT_POLICY_TYPE
 *
 * Description: Defines the policy type
 *
 * Fields:
 *      POLICY_TYPE_DA 	- Policy based on Destination Address
 *      POLICY_TYPE_SA 	- Policy based on Source Address
 *      POLICY_TYPE_VTU 	- Policy based on VID
 *		POLICY_TYPE_ETYPE	- based on Ether Type of a frame
 *		POLICY_TYPE_PPPoE	- Policy for the frame with Ether Type of 0x8863
 *		POLICY_TYPE_VBAS	- Policy for the frame with Ether Type of 0x8200
 *		POLICY_TYPE_OPT82	- Policy for the frame with DHCP Option 82
 *		POLICY_TYPE_UDP	- Policy for the frame with Broadcast IPv4 UDP or
 *						Multicast IPv6 UDP
 */
typedef enum
{
	POLICY_TYPE_DA,
	POLICY_TYPE_SA,
	POLICY_TYPE_VTU,
	POLICY_TYPE_ETYPE,
	POLICY_TYPE_PPPoE,
	POLICY_TYPE_VBAS,
	POLICY_TYPE_OPT82,
	POLICY_TYPE_UDP
} GT_POLICY_TYPE;


/*
 * Typedef: enum GT_PRI_OVERRIDE_FTYPE
 *
 * Description: Definition of the frame type for priority override
 *
 * Fields:
 *		FTYPE_DSA_TO_CPU_BPDU -
 *			Used on multicast DSA To_CPU frames with a Code of 0x0 (BPDU/MGMT).
 *			Not used on non-DSA Control frames.
 *		FTYPE_DSA_TO_CPU_F2R -
 *			Used on DSA To_CPU frames with a Code of 0x1 (Frame to Register
 *			Reply). Not used on non-DSA Control frames.
 *		FTYPE_DSA_TO_CPU_IGMP -
 *			Used on DSA To_CPU frames with a Code of 0x2 (IGMP/MLD Trap)
 *			and on non-DSA Control frames that are IGMP or MLD trapped
 *		FTYPE_DSA_TO_CPU_TRAP -
 *			Used on DSA To_CPU frames with a Code of 0x3 (Policy Trap) and
 *			on non-DSA Control frames that are Policy Trapped
 *		FTYPE_DSA_TO_CPU_ARP -
 *			Used on DSA To_CPU frames with a Code of 0x4 (ARP Mirror) and
 *			on non-DSA Control frames that are ARP Mirrored (see gprtSetARPtoCPU API).
 *		FTYPE_DSA_TO_CPU_MIRROR -
 *			Used on DSA To_CPU frames with a Code of 0x5 (Policy Mirror) and
 *			on non-DSA Control frames that are Policy Mirrored (see gprtSetPolicy API).
 *		FTYPE_DSA_TO_CPU_RESERVED -
 *			Used on DSA To_CPU frames with a Code of 0x6 (Reserved). Not
 *			used on non-DSA Control frames.
 *		FTYPE_DSA_TO_CPU_UCAST_MGMT -
 *			Used on unicast DSA To_CPU frames with a Code of 0x0 (unicast
 *			MGMT). Not used on non-DSA Control frames.
 *		FTYPE_DSA_FROM_CPU -
 *			Used on DSA From_CPU frames. Not used on non-DSA Control frame
 *		FTYPE_DSA_CROSS_CHIP_FC -
 *			Used on DSA Cross Chip Flow Control frames (To_Sniffer Flow
 *			Control). Not used on non-DSA Control frames.
 *		FTYPE_DSA_CROSS_CHIP_EGRESS_MON -
 *			Used on DSA Cross Chip Egress Monitor frames (To_Sniffer Tx).
 *			Not used on non-DSA Control frames.
 *		FTYPE_DSA_CROSS_CHIP_INGRESS_MON -
 *			Used on DSA Cross Chip Ingress Monitor frames (To_Sniffer Rx).
 *			Not used on non-DSA Control frames.
 *		FTYPE_PORT_ETYPE_MATCH -
 *			Used on normal network ports (see gprtSetFrameMode API)
 *			on frames whose Ethertype matches the port's PortEType register.
 *			Not used on non-DSA Control frames.
 *		FTYPE_BCAST_NON_DSA_CONTROL -
 *			Used on Non-DSA Control frames that contain a Broadcast
 *			destination address. Not used on DSA Control frames.
 */
typedef enum
{
	FTYPE_DSA_TO_CPU_BPDU = 0,
	FTYPE_DSA_TO_CPU_F2R,
	FTYPE_DSA_TO_CPU_IGMP,
	FTYPE_DSA_TO_CPU_TRAP,
	FTYPE_DSA_TO_CPU_ARP,
	FTYPE_DSA_TO_CPU_MIRROR,
	FTYPE_DSA_TO_CPU_RESERVED,
	FTYPE_DSA_TO_CPU_UCAST_MGMT,
	FTYPE_DSA_FROM_CPU,
	FTYPE_DSA_CROSS_CHIP_FC,
	FTYPE_DSA_CROSS_CHIP_EGRESS_MON,
	FTYPE_DSA_CROSS_CHIP_INGRESS_MON,
	FTYPE_PORT_ETYPE_MATCH,
	FTYPE_BCAST_NON_DSA_CONTROL
} GT_PRI_OVERRIDE_FTYPE;


/*
 * Typedef: struct GT_QPRI_TBL_ENTRY
 *
 * Description: This structure is used for the entry of Queue Priority Override
 *				Table.
 *
 * Fields:
 *      qPriEn    - GT_TRUE to enable Queue Priority, GT_FALSE otherwise
 *		qPriority - priority to be overridden ( 0 ~ 3 ) only if qPriEn is GT_TRUE
 *					When qPriEn is GT_FALSE, qPriority should be ignored.
 */
typedef struct
{
	GT_BOOL		qPriEn;
	GT_U32		qPriority;
}GT_QPRI_TBL_ENTRY;



/* Maximam number of ports a switch may have. */
#define MAX_SWITCH_PORTS    11
#define VERSION_MAX_LEN 30
#define MAX_QOS_WEIGHTS		128

/*
 * Typedef: struct GT_QoS_WEIGHT
 *
 * Description: This structure is used for Programmable Round Robin Weights.
 *
 * Fields:
 *      len    - length of the valid queue data
 *		queue  - upto 128 queue data
 */
typedef struct
{
	GT_U32		len;
	GT_U8		queue[MAX_QOS_WEIGHTS];
}GT_QoS_WEIGHT;



/*
 * Typedef: struct GT_VERSION
 *
 * Description: This struct holds the package version.
 *
 * Fields:
 *      version - string array holding the version.
 *
 */
typedef struct
{
    GT_U8   version[VERSION_MAX_LEN];
}GT_VERSION;


/*
 * typedef: struct GT_RMU
 *
 * Description: This struct holds the Remote Management Unit mode.
 *
 * Fields:
 *		rmuEn	- enable or disable RMU
 *		port	- logical port where RMU is enabled
 */
typedef struct
{
	GT_BOOL		rmuEn;
	GT_LPORT	port;
} GT_RMU;



/*
 * Typedef:
 *
 * Description: Defines the different sizes of the Mac address table.
 *
 * Fields:
 *      ATU_SIZE_256    -   256 entries Mac address table.
 *      ATU_SIZE_512    -   512 entries Mac address table.
 *      ATU_SIZE_1024   -   1024 entries Mac address table.
 *      ATU_SIZE_2048   -   2048 entries Mac address table.
 *      ATU_SIZE_4096   -   4096 entries Mac address table.
 *      ATU_SIZE_8192   -   8192 entries Mac address table.
 *
 */
typedef enum
{
    ATU_SIZE_256,
    ATU_SIZE_512,
    ATU_SIZE_1024,
    ATU_SIZE_2048,
    ATU_SIZE_4096,
	ATU_SIZE_8192
}ATU_SIZE;


/*
 * typedef: enum GT_PORT_STP_STATE
 *
 * Description: Enumeration of the port spanning tree state.
 *
 * Enumerations:
 *   GT_PORT_DISABLE    - port is disabled.
 *   GT_PORT_BLOCKING   - port is in blocking/listening state.
 *   GT_PORT_LEARNING   - port is in learning state.
 *   GT_PORT_FORWARDING - port is in forwarding state.
 */
typedef enum
{
    GT_PORT_DISABLE = 0,
    GT_PORT_BLOCKING,
    GT_PORT_LEARNING,
    GT_PORT_FORWARDING
} GT_PORT_STP_STATE;


/*
 * typedef: enum GT_EGRESS_MODE
 *
 * Description: Enumeration of the port egress mode.
 *
 * Enumerations:
 *   GT_UNMODIFY_EGRESS - frames are transmited unmodified.
 *   GT_TAGGED_EGRESS   - all frames are transmited tagged.
 *   GT_UNTAGGED_EGRESS - all frames are transmited untagged.
 *   GT_ADD_TAG         - always add a tag. (or double tag)
 */
typedef enum
{
    GT_UNMODIFY_EGRESS = 0,
    GT_UNTAGGED_EGRESS,
    GT_TAGGED_EGRESS,
    GT_ADD_TAG
} GT_EGRESS_MODE;

/*  typedef: enum GT_DOT1Q_MODE */

typedef enum
{
	GT_DISABLE = 0,
    GT_FALLBACK,
	GT_CHECK,
	GT_SECURE
} GT_DOT1Q_MODE;


/* typedef: enum GT_SW_MODE */

typedef enum
{
	GT_CPU_ATTATCHED = 0, /* ports come up disabled */
	GT_BACKOFF,           /* EEPROM attac mode with old half duplex backoff mode */
	GT_STAND_ALONE,       /* ports come up enabled, ignore EEPROM */
    GT_EEPROM_ATTATCHED   /* EEPROM defined prot states */
} GT_SW_MODE;


/*
 * Typedef: enum GT_ATU_OPERARION
 *
 * Description: Defines the different ATU and VTU operations
 *
 * Fields:
 *      FLUSH_ALL           - Flush all entries.
 *      FLUSH_UNLOCKED      - Flush all unlocked entries in ATU.
 *      LOAD_PURGE_ENTRY    - Load / Purge entry.
 *      GET_NEXT_ENTRY      - Get next ATU or VTU  entry.
 *      FLUSH_ALL_IN_DB     - Flush all entries in a particular DBNum.
 *      FLUSH_UNLOCKED_IN_DB - Flush all unlocked entries in a particular DBNum.
 *      SERVICE_VIOLATONS   - sevice violations of VTU
 *
 */
typedef enum
{
    FLUSH_ALL = 1,	/* for both atu and vtu */
    FLUSH_UNLOCKED,	/* for atu only */
    LOAD_PURGE_ENTRY,	/* for both atu and vtu */
    GET_NEXT_ENTRY,	/* for both atu and vtu */
	FLUSH_ALL_IN_DB,	/* for atu only */
    FLUSH_UNLOCKED_IN_DB,	/* for atu only */
    SERVICE_VIOLATIONS 	/* for vtu only */
} GT_ATU_OPERATION, GT_VTU_OPERATION;


/*
 * typedef: enum GT_FLUSH_CMD
 *
 * Description: Enumeration of the address translation unit flush operation.
 *
 * Enumerations:
 *   GT_FLUSH_ALL       - flush all entries.
 *   GT_FLUSH_ALL_UNBLK - flush all unblocked (or unlocked).
 *   GT_FLUSH_ALL_UNLOCKED - flush all unblocked (or unlocked).
 */
typedef enum
{
    GT_FLUSH_ALL       = 1,
    GT_FLUSH_ALL_UNBLK = 2,
    GT_FLUSH_ALL_UNLOCKED = 2
}GT_FLUSH_CMD;

/*
 * typedef: enum GT_MOVE_CMD
 *
 * Description: Enumeration of the address translation unit move or remove operation.
 *	 When destination port is set to 0xF, Remove operation occurs.
 *
 * Enumerations:
 *   GT_MOVE_ALL       - move all entries.
 *   GT_MOVE_ALL_UNBLK - move all unblocked (or unlocked).
 *   GT_MOVE_ALL_UNLOCKED - move all unblocked (or unlocked).
 */
typedef enum
{
    GT_MOVE_ALL       = 1,
    GT_MOVE_ALL_UNBLK = 2,
    GT_MOVE_ALL_UNLOCKED = 2
}GT_MOVE_CMD;


/*
 * typedef: enum GT_ATU_UC_STATE
 *
 * Description:
 *      Enumeration of the address translation unit entry state of unicast
 *      entris.
 *
 * Enumerations:
 *      GT_UC_INVALID   - invalid entry.
 *      GT_UC_DYNAMIC   - unicast dynamic entry.
 *		GT_UC_NO_PRI_TO_CPU_STATIC_NRL
 *				- static unicast entry that will be forwarded to CPU without
 *					forcing priority and without rate limiting.
 *		GT_UC_TO_CPU_STATIC_NRL
 *				- static unicast entry that will be forwarded to CPU without
 *					rate limiting.
 *      GT_UC_NO_PRI_STATIC_NRL
 *				- static unicast entry without forcing priority and without.
 *					rate limiting.
 *      GT_UC_STATIC_NRL    - static unicast entry without rate limiting.
 *		GT_UC_NO_PRI_TO_CPU_STATIC
 *				- static unicast entry that will be forwarded to CPU without
 *					forcing priority.
 *		GT_UC_TO_CPU_STATIC - static unicast entry that will be forwarded to CPU.
 *      GT_UC_NO_PRI_STATIC - static unicast entry without forcing priority.
 *      GT_UC_STATIC    - static unicast entry.
 *
 * Note: Please refer to the device datasheet for detailed unicast entry states
 *		that are supported by the device.
 */
typedef enum
{
    GT_UC_INVALID      = 0,
    GT_UC_DYNAMIC      = 0x1,

    GT_UC_NO_PRI_TO_CPU_STATIC_NRL	= 0x8,
    GT_UC_TO_CPU_STATIC_NRL			= 0x9,
    GT_UC_NO_PRI_STATIC_NRL			= 0xA,
    GT_UC_STATIC_NRL 				= 0xB,

    GT_UC_NO_PRI_TO_CPU_STATIC	= 0xC,
    GT_UC_TO_CPU_STATIC 		= 0xD,
    GT_UC_NO_PRI_STATIC 		= 0xE,
    GT_UC_STATIC  				= 0xF
} GT_ATU_UC_STATE;


/*
 * typedef: enum GT_ATU_MC_STATE
 *
 * Description:
 *      Enumeration of the address translation unit entry state of multicast
 *      entris.
 *
 * Enumerations:
 *      GT_MC_INVALID         - invalid entry.
 *      GT_MC_MGM_STATIC      - static multicast management entries.
 *      GT_MC_STATIC          - static multicast regular entris.
 *      GT_MC_PRIO_MGM_STATIC - static multicast management entries with
 *                              priority.
 *      GT_MC_PRIO_STATIC     - static multicast regular entris with priority.
 *      GT_MC_PRIO_STATIC_UNLIMITED_RATE - static multicast regular entris with priority
 *											and without rate limiting.
 *      GT_MC_MGM_STATIC_UNLIMITED_RATE	 - static multicast management entries without
 *											rate limiting.
 *      GT_MC_STATIC_UNLIMITED_RATE      - static multicast regular entris without
 *											rate limiting.
 *      GT_MC_PRIO_MGM_STATIC_UNLIMITED_RATE - static multicast management entries with
 *                              priority and without rate limiting.
 *
 * Note: Please refer to the device datasheet for detailed multicast entry states
 *		that are supported by the device.
 */
typedef enum
{
    GT_MC_INVALID         = 0,
    GT_MC_MGM_STATIC_UNLIMITED_RATE = 0x4,
    GT_MC_STATIC_UNLIMITED_RATE	= 0x5,
    GT_MC_MGM_STATIC      = 0x6,
    GT_MC_STATIC          = 0x7,
    GT_MC_PRIO_MGM_STATIC_UNLIMITED_RATE = 0xC,
    GT_MC_PRIO_STATIC_UNLIMITED_RATE	= 0xD,
    GT_MC_PRIO_MGM_STATIC = 0xE,
    GT_MC_PRIO_STATIC     = 0xF
} GT_ATU_MC_STATE;


/*
 *  typedef: struct GT_ATU_EXT_PRI
 *
 *  Description:
 *		Extanded priority information for the address tarnslaton unit entry.
 *
 *		macFPri data is used to override the frame priority on any frame associated
 *		with this MAC, if the useMacFPri is GT_TRUE and the port's SA and/or
 *		DA FPriOverride are enabled. SA Frame Priority Overrides can only occur on
 *		MAC addresses that are Static or where the Port is Locked, and where the port
 *		is mapped source port for the MAC address.
 *
 *		macQPri data is used to override the queue priority on any frame associated
 *		with this MAC, if the EntryState indicates Queue Priority Override and the
 *		port's SA and/or DA QPriOverride are enabled.
 *
 *  Fields:
 *      useMacFPri - Use MAC frame priority override. When this is GT_TRUE,
 *					 MacFPri data can be used to override the frame priority on
 *					 any frame associated with this MAC.
 *      macFPri    - MAC frame priority data (0 ~ 7).
 *      macQPri    - MAC queue priority data (0 ~ 3).
 *
 *  Comment:
 *      Please refer to the device datasheet to find out if this feature is supported.
 *		When this structure is implemented, the followings are the devices supporting
 *		this feature:
 *			88E6065, 88E6035, and 88E6055 support all extanded priority data.
 *			88E6061 and 88E6031 support only macQPri data
 */
typedef struct
{
	GT_BOOL			useMacFPri;
	GT_U8 			macFPri;
	GT_U8 			macQPri;
} GT_ATU_EXT_PRI;


/*
 *  typedef: struct GT_ATU_ENTRY
 *
 *  Description: address tarnslaton unit Entry
 *
 *  Fields:
 *      macAddr    - mac address
 *      trunkMember- GT_TRUE if entry belongs to a Trunk. This field will be
 *					 ignored if device does not support Trunk.
 *      portVec    - port Vector.
 *					 If trunkMember field is GT_TRUE, this value represents trunk ID.
 *      prio       - entry priority.
 *      entryState - the entry state.
 *		DBNum	   - ATU MAC Address Database number. If multiple address
 *					 databases are not being used, DBNum should be zero.
 *					 If multiple address databases are being used, this value
 *					 should be set to the desired address database number.
 *		exPrio     - extanded priority information. If device support extanded
 *					 priority, prio field should be ignored.
 *
 *  Comment:
 *      The entryState union Type is determine according to the Mac Type.
 */
typedef struct
{
	GT_ETHERADDR 	macAddr;
	GT_BOOL			trunkMember;
	GT_U32			portVec;
	GT_U8			prio;
	GT_U8			reserved;
	GT_U16			DBNum;
	union
	{
		GT_ATU_UC_STATE ucEntryState;
		GT_ATU_MC_STATE mcEntryState;
	} entryState;
	GT_ATU_EXT_PRI	exPrio;
} GT_ATU_ENTRY;


/*
 *  typedef: struct GT_VTU_DATA
 *
 *  Description: VLAN  tarnslaton unit Data Register
 *
 *  Fields:
 *      memberTagP - Membership and Egress Tagging
 *                   memberTagP[0] is for Port 0, MemberTagP[1] is for port 1, and so on
 *
 *  Comment:
 * 	MAX_SWITCH_PORTS is 10 for Octane.
 * 	In the case of FullSail, there are 3 ports. So, the rest 7 is ignored in memeberTagP
 */
typedef struct
{
    GT_U8     memberTagP[MAX_SWITCH_PORTS];
    GT_U8     portStateP[MAX_SWITCH_PORTS];
} GT_VTU_DATA;

/*
 *  definition for MEMBER_TAG
 */
#define MEMBER_EGRESS_UNMODIFIED	0
#define NOT_A_MEMBER				1
#define MEMBER_EGRESS_UNTAGGED		2
#define MEMBER_EGRESS_TAGGED		3

/*
 *  typedef: struct GT_VTU_EXT_INFO
 *
 *  Description:
 *		Extanded VTU Entry information for Priority Override and Non Rate Limit.
 *		Frame Priority is used to as the tag's PRI bits if the frame egresses
 *		the switch tagged. The egresss queue the frame is switch into is not
 *		modified by the Frame Priority Override.
 *		Queue Priority is used to determine the egress queue the frame is
 *		switched into. If the frame egresses tagged, the priority in the frame
 *		will not be modified by a Queue Priority Override.
 *		NonRateLimit for VID is used to indicate any frames associated with this
 *		VID are to bypass ingress and egress rate limiting, if the ingress
 *		port's VID NRateLimit is also enabled.
 *
 *  Fields:
 *      useVIDFPri - Use VID frame priority override. When this is GT_TRUE and
 *					 VIDFPriOverride of the ingress port of the frame is enabled,
 *					 vidFPri data is used to override the frame priority on
 *					 any frame associated with this VID.
 *      vidFPri    - VID frame priority data (0 ~ 7).
 *      useVIDQPri - Use VID queue priority override. When this is GT_TRUE and
 *					 VIDQPriOverride of the ingress port of the frame is enabled,
 *					 vidQPri data can be used to override the queue priority on
 *					 any frame associated with this VID.
 *      vidQPri    - VID queue priority data (0 ~ 3).
 *      vidNRateLimit - bypass rate ingress and egress limiting
 *
 *  Comment:
 *      Please refer to the device datasheet to find out if this feature is supported.
 *		When this structure is implemented, the followings are the devices supporting
 *		this feature:
 *			88E6065, 88E6035, and 88E6055 support all data.
 *			88E6061 and 88E6031 support only vidNRateLimit.
 */
typedef struct
{
	GT_BOOL			useVIDFPri;
	GT_U8 			vidFPri;
	GT_BOOL			useVIDQPri;
	GT_U8 			vidQPri;
	GT_BOOL			vidNRateLimit;
} GT_VTU_EXT_INFO;


/*
 *  typedef: struct GT_VTU_ENTRY
 *
 *  Description: VLAN  tarnslaton unit Entry
 *		Each field in the structure is device specific, i.e., some fields may not
 *		be supported by the used switch device. In such case, those fields are
 *		ignored by the DSDT driver. Please refer to the datasheet for the list of
 *		supported fields.
 *
 *  Fields:
 *      DBNum      - database number or FID (forwarding information database)
 *      vid        - VLAN ID
 *      vtuData    - VTU data
 *		vidPriOverride - override the priority on any frame associated with this VID
 *		vidPriority - VID Priority bits (0 ~ 7)
 *		sid	       - 802.1s Port State Database ID
 *		vidPolicy  - indicate that the frame with this VID uses VID Policy
 *		             (see gprtSetPolicy API).
 *		vidExInfo  - extanded information for VTU entry. If the device supports extanded
 *					 information, vidPriorityOverride and vidPriority values are
 *					 ignored.
 */
typedef struct
{
    GT_U16        DBNum;
    GT_U16        vid;
    GT_VTU_DATA   vtuData;
	GT_BOOL		  vidPriOverride;
	GT_U8		  vidPriority;
	GT_U8		  sid;
	GT_BOOL		  vidPolicy;
	GT_VTU_EXT_INFO	vidExInfo;
} GT_VTU_ENTRY;


/*
 * Typedef: enum GT_STU_OPERARION
 *
 * Description: Defines the STU operations
 *
 * Fields:
 *      LOAD_PURGE_STU_ENTRY    - Load / Purge entry.
 *      GET_NEXT_STU_ENTRY      - Get next STU  entry.
 *
 */
typedef enum
{
    LOAD_PURGE_STU_ENTRY = 5,
    GET_NEXT_STU_ENTRY =6
} GT_STU_OPERATION;


/*
 *  typedef: struct GT_STU_ENTRY
 *
 *  Description: 802.1s Port State Information Database (STU) Entry
 *
 *  Fields:
 *      sid       - STU ID
 *		portState - Per VLAN Port States for each port.
 */
typedef struct
{
    GT_U16				sid;
    GT_PORT_STP_STATE	portState[MAX_SWITCH_PORTS];
} GT_STU_ENTRY;


/*
 *  typedef: struct GT_VTU_INT_STATUS
 *
 *  Description: VLAN tarnslaton unit interrupt status
 *
 *  Fields:
 *      intCause  - VTU Interrupt Cause
 *					GT_VTU_FULL_VIOLATION,GT_MEMEBER_VIOLATION,or
 *					GT_MISS_VIOLATION
 *      SPID      - source port number
 * 					if intCause is GT_VTU_FULL_VIOLATION, it means nothing
 *      vid       - VLAN ID
 * 					if intCause is GT_VTU_FULL_VIOLATION, it means nothing
 */
typedef struct
{
    GT_U16   vtuIntCause;
    GT_U8    spid;
    GT_U16   vid;
} GT_VTU_INT_STATUS;

/*
 *  typedef: struct GT_ATU_INT_STATUS
 *
 *  Description: VLAN tarnslaton unit interrupt status
 *
 *  Fields:
 *      intCause  - ATU Interrupt Cause
 *					GT_FULL_VIOLATION,GT_MEMEBER_VIOLATION,
 *					GT_MISS_VIOLATION, GT_AGE_VIOLATION, or
 *					GT_AGE_OUT_VIOLATION
 *      SPID      - source port number
 * 					if intCause is GT_FULL_VIOLATION, it means nothing
 *      DBNum     - DB Num (or FID)
 * 					if intCause is GT_FULL_VIOLATION, it means nothing
 *		macAddr	  - MAC Address
 */
typedef struct
{
    GT_U16   atuIntCause;
    GT_U8    spid;
    GT_U8    dbNum;
    GT_ETHERADDR  macAddr;
} GT_ATU_INT_STATUS;

/*
* Definition for VTU interrupt
*/
#define GT_MEMBER_VIOLATION		0x4
#define GT_MISS_VIOLATION		0x2
#define GT_VTU_FULL_VIOLATION	0x1
/*
* Definitions for ATU interrupt in Gigabit switch are the same as
* the ones for VTU interrupt. Here we just redefine the FULL_VIOLATION for
* both VTU and ATU.
*/
#define GT_FULL_VIOLATION		0x1

#define GT_AGE_VIOLATION		0x8
#define GT_AGE_OUT_VIOLATION	0x10


/*
 * Typedef: enum GT_PVT_OPERATION
 *
 * Description: Defines the PVT (Cross Chip Port VLAN Table) Operation type
 *
 * Fields:
 *      PVT_INITIALIZE - Initialize all resources to the inital state
 *      PVT_WRITE      - Write to the selected PVT entry
 *      PVT_READ       - Read from the selected PVT entry
 */
typedef enum
{
	PVT_INITIALIZE 	= 0x1,
	PVT_WRITE		= 0x3,
	PVT_READ		= 0x4
} GT_PVT_OPERATION;


/*
 *  typedef: struct GT_PVT_OP_DATA
 *
 *  Description: data required by PVT (Cross Chip Port VLAN Table) Operation
 *
 *  Fields:
 *      pvtAddr - pointer to the desired entry of PVT
 *      pvtData - Cross Chip Port VLAN data for the entry pointed by pvtAddr
 */
typedef struct
{
	GT_U32	pvtAddr;
	GT_U32	pvtData;
} GT_PVT_OP_DATA;


/*
 *  typedef: enum GT_PIRL_FC_DEASSERT
 *
 *  Description: Enumeration of the port flow control de-assertion mode on PIRL.
 *
 *  Enumerations:
 *      GT_PIRL_FC_DEASSERT_EMPTY -
 *				De-assert when the ingress rate resource has become empty
 *		GT_PIRL_FC_DEASSERT_CBS_LIMIT -
 *				De-assert when the ingress rate resource has enough room as
 *				specified by the CBSLimit.
 */
typedef enum
{
    GT_PIRL_FC_DEASSERT_EMPTY = 0,
    GT_PIRL_FC_DEASSERT_CBS_LIMIT
} GT_PIRL_FC_DEASSERT;


/*
 *  typedef: enum GT_PIRL_ELIMIT_MODE
 *
 *  Description: Enumeration of the port egress rate limit counting mode.
 *
 *  Enumerations:
 *      GT_PIRL_ELIMIT_FRAME -
 *				Count the number of frames
 *      GT_PIRL_ELIMIT_LAYER1 -
 *				Count all Layer 1 bytes:
 *				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      GT_PIRL_ELIMIT_LAYER2 -
 *				Count all Layer 2 bytes: Frame's DA to CRC
 *      GT_PIRL_ELIMIT_LAYER3 -
 *				Count all Layer 3 bytes:
 *				Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum
{
    GT_PIRL_ELIMIT_FRAME = 0,
    GT_PIRL_ELIMIT_LAYER1,
    GT_PIRL_ELIMIT_LAYER2,
    GT_PIRL_ELIMIT_LAYER3
} GT_PIRL_ELIMIT_MODE;


/* typedef: enum GT_RATE_LIMIT_MODE
 * The ingress limit mode in the rate control register (0xA)
 */

typedef enum
{
	GT_LIMT_ALL = 0, 		/* limit and count all frames */
	GT_LIMIT_FLOOD,      	/* limit and count Broadcast, Multicast and flooded unicast frames */
	GT_LIMIT_BRDCST_MLTCST,	/* limit and count Broadcast and Multicast frames */
	GT_LIMIT_BRDCST   		/* limit and count Broadcast frames */
} GT_RATE_LIMIT_MODE;

/* typedef: enum GT_PRI0_RATE
 * The ingress data rate limit for priority 0 frames
 */

typedef enum
{
	GT_NO_LIMIT = 0, 	/* Not limited   */
	GT_128K,      		/* 128K bits/sec */
	GT_256K,      		/* 256K bits/sec */
	GT_512K,      		/* 512 bits/sec */
	GT_1M,      		/* 1M  bits/sec */
	GT_2M,      		/* 2M  bits/sec */
	GT_4M,      		/* 4M  bits/sec */
	GT_8M,      		/* 8M  bits/sec */
	GT_16M,      		/* 16M  bits/sec, Note: supported only by Gigabit Ethernet Switch */
	GT_32M,      		/* 32M  bits/sec, Note: supported only by Gigabit Ethernet Switch */
	GT_64M,      		/* 64M  bits/sec, Note: supported only by Gigabit Ethernet Switch */
	GT_128M,      		/* 128M  bits/sec, Note: supported only by Gigabit Ethernet Switch */
	GT_256M      		/* 256M  bits/sec, Note: supported only by Gigabit Ethernet Switch */
} GT_PRI0_RATE,GT_EGRESS_RATE;


/*
 * Typedef: union GT_ERATE_TYPE
 *
 * Description: Egress Rate
 *
 * Fields:
 *      definedRate - GT_EGRESS_RATE enum type should be used on the following devices:
 *						88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183, 88E6093
 *						88E6095, 88E6185, 88E6108, 88E6065, 88E6061, and their variations.
 *      kbRate      - rate in kbps that should be used on the following devices:
 *						88E6097, 88E6096 with count mode of non frame, such as
 *									ALL_LAYER1, ALL_LAYER2, and ALL_LAYER3
 *						64kbps ~ 1Mbps    : increments of 64kbps,
 *						1Mbps ~ 100Mbps   : increments of 1Mbps, and
 *						100Mbps ~ 1000Mbps: increments of 10Mbps
 *						Therefore, the valid values are:
 *							64, 128, 192, 256, 320, 384,..., 960,
 *							1000, 2000, 3000, 4000, ..., 100000,
 *							110000, 120000, 130000, ..., 1000000.
 *      fRate 		- frame per second that should be used on the following devices:
 *						88E6097, 88E6096 with count mode of frame (GT_PIRL_COUNT_FRAME)
 */
typedef union
{
	GT_EGRESS_RATE	definedRate;
	GT_U32			kbRate;
	GT_U32			fRate;
} GT_ERATE_TYPE;

/*
 * Formula for Rate Limit of Gigabit Switch family and Enhanced FastEthernet Switch
 */
#define GT_GET_RATE_LIMIT(_kbps)	\
		((_kbps)?(8000000 / (28 * (_kbps))):0)
#define GT_GET_RATE_LIMIT2(_kbps)	\
		((_kbps)?(8000000 / (32 * (_kbps)) + (8000000 % (32 * (_kbps))?1:0)):0)
#define GT_GET_RATE_LIMIT3(_kbps)	\
		((_kbps)?(8000000 / (40 * (_kbps)) + (8000000 % (40 * (_kbps))?1:0)):0)

#define MAX_RATE_LIMIT		256000	/* unit of Kbps */
#define MIN_RATE_LIMIT		65		/* unit of Kbps */


#define GT_GET_RATE_LIMIT_PER_FRAME(_frames, _dec)	\
		((_frames)?(1000000000 / (32 * (_frames)) + (1000000000 % (32 * (_frames))?1:0)):0)

#define GT_GET_RATE_LIMIT_PER_BYTE(_kbps, _dec)	\
		((_kbps)?((8000000*(_dec)) / (32 * (_kbps)) + ((8000000*(_dec)) % (32 * (_kbps))?1:0)):0)

/*
 * typedef: enum GT_BURST_SIZE
 * The ingress data rate limit burst size windows selection
 */

typedef enum
{
	GT_BURST_SIZE_12K = 0, 	/* 12K byte burst size */
	GT_BURST_SIZE_24K,		/* 24K byte burst size */
	GT_BURST_SIZE_48K,		/* 48K byte burst size */
	GT_BURST_SIZE_96K 	 	/* 96K byte burst size */
} GT_BURST_SIZE;

/*
 * typedef: enum GT_BURST_RATE
 * The ingress data rate limit based on burst size
 */

typedef enum
{
	GT_BURST_NO_LIMIT = 0, 	/* Not limited   */
	GT_BURST_64K,  		/* 64K bits/sec */
	GT_BURST_128K,  	/* 128K bits/sec */
	GT_BURST_256K,  	/* 256K bits/sec */
	GT_BURST_384K,  	/* 384K bits/sec */
	GT_BURST_512K,  	/* 512 bits/sec */
	GT_BURST_640K,  	/* 640K bits/sec */
	GT_BURST_768K,  	/* 768K bits/sec */
	GT_BURST_896K,  	/* 896K bits/sec */
	GT_BURST_1M,    	/* 1M  bits/sec */
	GT_BURST_1500K,  	/* 1.5M bits/sec */
	GT_BURST_2M,    	/* 2M  bits/sec */
	GT_BURST_4M,    	/* 4M  bits/sec */
	GT_BURST_8M,   		/* 8M  bits/sec */
	GT_BURST_16M,  		/* 16M  bits/sec */
	GT_BURST_32M,  		/* 32M  bits/sec */
	GT_BURST_64M,  		/* 64M  bits/sec */
	GT_BURST_128M, 		/* 128M  bits/sec */
	GT_BURST_256M  		/* 256M  bits/sec */
} GT_BURST_RATE;

/*
 * Formula for burst based Rate Limit
 */
#define GT_GET_BURST_RATE_LIMIT(_bsize,_kbps)	\
		((_kbps)?(((_bsize)+1)*8000000 / (32 * (_kbps)) + 		\
				(((_bsize)+1)*8000000 % (32 * (_kbps))?1:0))	\
				:0)

/*
 * Typedef: enum GT_PIRL_OPERATION
 *
 * Description: Defines the PIRL (Port Ingress Rate Limit) Operation type
 *
 * Fields:
 *      PIRL_INIT_ALL_RESOURCE - Initialize all resources to the inital state
 *      PIRL_INIT_RESOURCE     - Initialize selected resources to the inital state
 *      PIRL_WRITE_RESOURCE    - Write to the selected resource/register
 *      PIRL_READ_RESOURCE     - Read from the selected resource/register
 */
typedef enum
{
	PIRL_INIT_ALL_RESOURCE 	= 0x1,
	PIRL_INIT_RESOURCE		= 0x2,
	PIRL_WRITE_RESOURCE		= 0x3,
	PIRL_READ_RESOURCE		= 0x4
} GT_PIRL_OPERATION, GT_PIRL2_OPERATION;


/*
 *  typedef: struct GT_PIRL_OP_DATA
 *
 *  Description: data required by PIRL Operation
 *
 *  Fields:
 *      irlUnit   - Ingress Rate Limit Unit that defines one of IRL resources.
 *      irlReg    - Ingress Rate Limit Register.
 *      irlData   - Ingress Rate Limit Data.
 */
typedef struct
{
	GT_U32	irlUnit;
	GT_U32	irlReg;
	GT_U32	irlData;
} GT_PIRL_OP_DATA;

/*
 *  typedef: struct GT_PIRL2_OP_DATA
 *
 *  Description: data required by PIRL Operation
 *
 *  Fields:
 *      irlPort   - Ingress Rate Limiting port (physical port number).
 *      irlRes    - Ingress Rate Limit Resource.
 *      irlReg    - Ingress Rate Limit Register.
 *      irlData   - Ingress Rate Limit Data.
 */
typedef struct
{
	GT_U32	irlPort;
	GT_U32	irlRes;
	GT_U32	irlReg;
	GT_U32	irlData;
} GT_PIRL2_OP_DATA;

/*
 * Typedef: enum GT_PIRL_ACTION
 *
 * Description: Defines the Action that should be taken when
 *		there there are not enough tokens to accept the entire incoming frame
 *
 * Fields:
 *		PIRL_ACTION_ACCEPT - accept the frame
 *		PIRL_ACTION_USE_LIMIT_ACTION - use ESB Limit Action
 */
typedef enum
{
	PIRL_ACTION_USE_LIMIT_ACTION = 0x0,
	PIRL_ACTION_ACCEPT 	= 0x1
} GT_PIRL_ACTION;

/*
 * Typedef: enum GT_ESB_LIMIT_ACTION
 *
 * Description: Defines the ESB Limit Action that should be taken when
 *		the incoming port information rate exceeds the EBS_Limit.
 *
 * Fields:
 *		ESB_LIMIT_ACTION_DROP - drop packets
 *		ESB_LIMIT_ACTION_FC   - send flow control packet
 */
typedef enum
{
	ESB_LIMIT_ACTION_DROP 	= 0x0,
	ESB_LIMIT_ACTION_FC		= 0x1
} GT_ESB_LIMIT_ACTION;


/*
 * Typedef: enum GT_BUCKET_RATE_TYPE
 *
 * Description: Defines the Bucket Rate Type
 *
 * Fields:
 *		BUCKET_TYPE_TRAFFIC_BASED	- bucket is traffic type based
 *		BUCKET_TYPE_RATE_BASED		- bucket is rate based
 */
typedef enum
{
	BUCKET_TYPE_TRAFFIC_BASED	= 0x0,
	BUCKET_TYPE_RATE_BASED		= 0x1
} GT_BUCKET_RATE_TYPE;

/*
 * Definition for GT_BUCKET_TYPE_TRAFFIC_BASED
 *
 * Description: Defines the Traffic Type that is used when Bucket Rate Type
 *		is traffic type based (BUCKET_TYPE_TRAFFIC_BASED).
 *		Please refer to the device datasheet in order to check which traffic
 *		types are supported.
 *
 * Definition:
 *		BUCKET_TRAFFIC_UNKNOWN_UNICAST	- unknown unicast frame
 *		BUCKET_TRAFFIC_UNKNOWN_MULTICAST- unknown multicast frame
 *		BUCKET_TRAFFIC_BROADCAST		- broadcast frame
 *		BUCKET_TRAFFIC_MULTICAST		- multicast frame
 *		BUCKET_TRAFFIC_UNICAST			- unicast frame
 *		BUCKET_TRAFFIC_MGMT_FRAME		- management frame
 *		BUCKET_TRAFFIC_ARP				- arp frame
 *		BUCKET_TRAFFIC_TCP_DATA			- TCP Data
 *		BUCKET_TRAFFIC_TCP_CTRL			- TCP Ctrl (if any of the TCP Flags[5:0] are set)
 *		BUCKET_TRAFFIC_UDP				- UDP
 *		BUCKET_TRAFFIC_NON_TCPUDP		- covers IGMP,ICMP,GRE,IGRP,L2TP
 *		BUCKET_TRAFFIC_IMS				- Ingress Monitor Source
 *		BUCKET_TRAFFIC_POLICY_MIRROR	- Policy Mirror
 *		BUCKET_TRAFFIC_PLICY_TRAP		- Policy Trap
 */
#define BUCKET_TRAFFIC_UNKNOWN_UNICAST  	0x01
#define BUCKET_TRAFFIC_UNKNOWN_MULTICAST	0x02
#define BUCKET_TRAFFIC_BROADCAST			0x04
#define BUCKET_TRAFFIC_MULTICAST			0x08
#define BUCKET_TRAFFIC_UNICAST				0x10
#define BUCKET_TRAFFIC_MGMT_FRAME			0x20
#define BUCKET_TRAFFIC_ARP					0x40
#define BUCKET_TRAFFIC_TCP_DATA				0x100
#define	BUCKET_TRAFFIC_TCP_CTRL				0x200
#define	BUCKET_TRAFFIC_UDP					0x400
#define	BUCKET_TRAFFIC_NON_TCPUDP			0x800
#define	BUCKET_TRAFFIC_IMS					0x1000
#define	BUCKET_TRAFFIC_POLICY_MIRROR		0x2000
#define	BUCKET_TRAFFIC_PLICY_TRAP			0x4000

/*
 *  typedef: enum GT_PIRL_COUNT_MODE
 *
 *  Description: Enumeration of the port egress rate limit counting mode.
 *
 *  Enumerations:
 *      GT_PIRL_COUNT_ALL_LAYER1 -
 *				Count all Layer 1 bytes:
 *				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      GT_PIRL_COUNT_ALL_LAYER2 -
 *				Count all Layer 2 bytes: Frame's DA to CRC
 *      GT_PIRL_COUNT_ALL_LAYER3 -
 *				Count all Layer 3 bytes:
 *				Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum
{
    GT_PIRL_COUNT_ALL_LAYER1 = 0,
    GT_PIRL_COUNT_ALL_LAYER2,
    GT_PIRL_COUNT_ALL_LAYER3
} GT_PIRL_COUNT_MODE;

/*
 *  typedef: enum GT_PIRL2_COUNT_MODE
 *
 *  Description: Enumeration of the port egress rate limit counting mode.
 *
 *  Enumerations:
 *      GT_PIRL2_COUNT_FRAME -
 *				Count the number of frames
 *      GT_PIRL2_COUNT_ALL_LAYER1 -
 *				Count all Layer 1 bytes:
 *				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      GT_PIRL2_COUNT_ALL_LAYER2 -
 *				Count all Layer 2 bytes: Frame's DA to CRC
 *      GT_PIRL2_COUNT_ALL_LAYER3 -
 *				Count all Layer 3 bytes:
 *				Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum
{
	GT_PIRL2_COUNT_FRAME = 0,
    GT_PIRL2_COUNT_ALL_LAYER1,
    GT_PIRL2_COUNT_ALL_LAYER2,
    GT_PIRL2_COUNT_ALL_LAYER3
} GT_PIRL2_COUNT_MODE;



/*
 *  typedef: struct GT_PIRL_RESOURCE
 *
 *  Description: data structure that represents a PIRL Resource
 *
 *  Fields:
 *      accountQConf    - account discarded frames due to queue congestion
 *      accountFiltered - account filtered frames
 *		ebsLimitAction  - action should be taken when the incoming rate exceeds
 *						  the ebsLimit.
 *								ESB_LIMIT_ACTION_DROP - drop packets
 *								ESB_LIMIT_ACTION_FC   - send flow control packet
 *		ebsLimit        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
 *		cbsLimit		- Committed BUrst Size limit (expected to be 2kBytes)
 *		bktRateFactor   - bucket rate factor = bucketDecrement/updateInterval,
 *						  where updateInterval indicates the rate at which the
 *						  bucket needs to be updated with tokens, or 1/CIR,
 *						  where CIR is the committed information rate in kbps.
 *						  bucketDecrement indicates the amount of tokens that
 *						  need to be removed per each bucket decrement.
 *		bktIncrement    - the amount of tokens that need to be added for each
 *						  byte of packet information.
 *		bktRateType		- bucket is either rate based or traffic type based.
 *								BUCKET_TYPE_RATE_BASED, or
 *								BUCKET_TYPE_TRAFFIC_BASED
 *		bktTypeMask		- used if bktRateType is BUCKET_TYPE_TRAFFIC_BASED.
 *						  any combination of the following definitions:
 *								BUCKET_TRAFFIC_UNKNOWN_UNICAST,
 *								BUCKET_TRAFFIC_UNKNOWN_MULTICAST,
 *								BUCKET_TRAFFIC_BROADCAST,
 *								BUCKET_TRAFFIC_MULTICAST,
 *								BUCKET_TRAFFIC_UNICAST,
 *								BUCKET_TRAFFIC_MGMT_FRAME, and
 *								BUCKET_TRAFFIC_ARP
 *		byteTobeCounted	- bytes to be counted for accounting
 *								GT_PIRL_COUNT_ALL_LAYER1,
 *								GT_PIRL_COUNT_ALL_LAYER2, or
 *								GT_PIRL_COUNT_ALL_LAYER3
 *
 */
typedef struct
{
	GT_BOOL		accountQConf;
	GT_BOOL		accountFiltered;
	GT_ESB_LIMIT_ACTION ebsLimitAction;
	GT_U32		ebsLimit;
	GT_U32		cbsLimit;
	GT_U32		bktRateFactor;
	GT_U32		bktIncrement;
	GT_BUCKET_RATE_TYPE	bktRateType;
	GT_U32		bktTypeMask;
	GT_PIRL_COUNT_MODE	byteTobeCounted;
} GT_PIRL_RESOURCE;

/*
 *  typedef: struct GT_PIRL_CUSTOM_RATE_LIMIT
 *
 *  Description: The parameters that decides Ingress Rate Limit vary depending on
 *				the application. Since DSDT driver cannot cover all the cases,
 *				this structure is provided for the custom parameter setting.
 *				However, in most cases, user may ingore this structure by setting
 *				isValid to GT_FALSE. If Ingress Rate Limit is too much off from
 *				the expected rate, please contact FAE and gets the correct ebsLimit,
 *				cbsLimit,bktIncrement, and bktRateFactor value and use this structure
 *				to do custom parameter setting.
 *
 *		isValid         - If GT_TRUE, the paramers in this structure are used
 *						  to program PIRL Resource's Rate Limit. And ingressRate
 *						  in GT_PIRL_BUCKET_DATA structure are ignored.
 *						  If GT_FALSE, ingressRate in GT_PIRL_BUCKET_DATA structure
 *						  is used for Resource's Rate Limit.
 *		ebsLimit        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
 *		cbsLimit		- Committed Burst Size limit (expected to be 2kBytes)
 *		bktIncrement    - the amount of tokens that need to be added for each
 *						  byte of packet information.
 *		bktRateFactor   - bucket rate factor = bucketDecrement/updateInterval,
 *						  where updateInterval indicates the rate at which the
 *						  bucket needs to be updated with tokens, or 1/CIR,
 *						  where CIR is the committed information rate in kbps.
 *						  bucketDecrement indicates the amount of tokens that
 *						  need to be removed per each bucket decrement.
*/
typedef struct
{
	GT_BOOL		isValid;
	GT_U32		ebsLimit;
	GT_U32		cbsLimit;
	GT_U32		bktIncrement;
	GT_U32		bktRateFactor;
} GT_PIRL_CUSTOM_RATE_LIMIT;

/*
 *  typedef: struct GT_PIRL_BUCKET_DATA
 *
 *  Description: data structure for PIRL Bucket programing that is resource based
 *
 *  Fields:
 *		ingressRate   	- commited ingress rate in kbps.
 *						  64kbps ~ 1Mbps    : increments of 64kbps,
 *						  1Mbps ~ 100Mbps   : increments of 1Mbps, and
 *						  100Mbps ~ 200Mbps : increments of 10Mbps
 *						  Therefore, the valid values are:
 *								64, 128, 192, 256, 320, 384,..., 960,
 *								1000, 2000, 3000, 4000, ..., 100000,
 *								110000, 120000, 130000, ..., 200000.
 *		customSetup   	- custom ingress rate parameter setup. please refer to
 *						  GT_PIRL_CUSTOM_RATE_LIMIT structure.
 *      accountQConf    - account discarded frames due to queue congestion
 *      accountFiltered - account filtered frames
 *		esbLimitAction 	- action should be taken when the incoming rate exceeds
 *						  the limit.
 *								ESB_LIMIT_ACTION_DROP - drop packets
 *								ESB_LIMIT_ACTION_FC   - send flow control packet
 *		fcDeassertMode	- port flow control de-assertion mode when limitAction is
 *						  set to ESB_LIMIT_ACTION_FC.
 *						  fcDeassertMode[0] for port 0, fcDeassertMode[1] for
 *						  port 1, etc. If port x does not share the bucket,
 *						  fcDeassertMode[x] data will be ignored.
 *								GT_PIRL_FC_DEASSERT_EMPTY -
 *									De-assert when the ingress rate resource has
 *									become empty.
 *								GT_PIRL_FC_DEASSERT_CBS_LIMIT -
 *									De-assert when the ingress rate resource has
 *									enough room as specified by the CBSLimit.
 *		bktRateType		- bucket is either rate based or traffic type based.
 *								BUCKET_TYPE_RATE_BASED, or
 *								BUCKET_TYPE_TRAFFIC_BASED
 *		bktTypeMask		- used if bktRateType is BUCKET_TYPE_TRAFFIC_BASED.
 *						  any combination of the following definitions:
 *								BUCKET_TRAFFIC_UNKNOWN_UNICAST,
 *								BUCKET_TRAFFIC_UNKNOWN_MULTICAST,
 *								BUCKET_TRAFFIC_BROADCAST,
 *								BUCKET_TRAFFIC_MULTICAST,
 *								BUCKET_TRAFFIC_UNICAST,
 *								BUCKET_TRAFFIC_MGMT_FRAME, and
 *								BUCKET_TRAFFIC_ARP
 *		byteTobeCounted	- bytes to be counted for accounting
 *								GT_PIRL_COUNT_ALL_LAYER1,
 *								GT_PIRL_COUNT_ALL_LAYER2, or
 *								GT_PIRL_COUNT_ALL_LAYER3
 *
 */
typedef struct
{
	GT_U32		ingressRate;
	GT_PIRL_CUSTOM_RATE_LIMIT customSetup;
	GT_BOOL		accountQConf;
	GT_BOOL		accountFiltered;
	GT_ESB_LIMIT_ACTION ebsLimitAction;
	GT_PIRL_FC_DEASSERT fcDeassertMode[MAX_SWITCH_PORTS];
	GT_BUCKET_RATE_TYPE	bktRateType;
	GT_U32		bktTypeMask;
	GT_PIRL_COUNT_MODE	byteTobeCounted;
} GT_PIRL_DATA;


/*
 *  typedef: struct GT_PIRL2_RESOURCE
 *
 *  Description: data structure that represents a PIRL Resource
 *
 *  Fields:
 *      accountQConf    - account discarded frames due to queue congestion
 *      accountFiltered - account filtered frames
 *      mgmtNrlEn 		- exclude management frame from ingress rate limiting calculation
 *      saNrlEn 		- exclude from ingress rate limiting calculation if the SA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *      daNrlEn 		- exclude from ingress rate limiting calculation if the DA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *		samplingMode	- sample one out of so many frames/bytes for a stream of frames
 *		actionMode		- action should be taken when there are not enough tokens
 *						  to accept the entire incoming frame
 *								PIRL_ACTION_ACCEPT - accept the frame
 *								PIRL_ACTION_USE_LIMIT_ACTION - use limitAction
 *		ebsLimitAction  - action should be taken when the incoming rate exceeds
 *						  the ebsLimit.
 *								ESB_LIMIT_ACTION_DROP - drop packets
 *								ESB_LIMIT_ACTION_FC   - send flow control packet
 *		ebsLimit        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
 *		cbsLimit		- Committed BUrst Size limit (expected to be 2kBytes)
 *		bktRateFactor   - bucket rate factor = bucketDecrement/updateInterval,
 *						  where updateInterval indicates the rate at which the
 *						  bucket needs to be updated with tokens, or 1/CIR,
 *						  where CIR is the committed information rate in kbps.
 *						  bucketDecrement indicates the amount of tokens that
 *						  need to be removed per each bucket decrement.
 *		bktIncrement    - the amount of tokens that need to be added for each
 *						  byte of packet information.
 *		fcDeassertMode	- flow control de-assertion mode when limitAction is
 *						  set to ESB_LIMIT_ACTION_FC.
 *								GT_PIRL_FC_DEASSERT_EMPTY -
 *									De-assert when the ingress rate resource has
 *									become empty.
 *								GT_PIRL_FC_DEASSERT_CBS_LIMIT -
 *									De-assert when the ingress rate resource has
 *									enough room as specified by the CBSLimit.
 *		bktRateType		- bucket is either rate based or traffic type based.
 *								BUCKET_TYPE_RATE_BASED, or
 *								BUCKET_TYPE_TRAFFIC_BASED
 *      priORpt         - determine the incoming frames that get rate limited using
 *						  this ingress rate resource.
 *						  		GT_TRUE - typeMask OR priMask
 *						  		GT_FALSE - typeMask AND priMask
 *		priMask         - priority bit mask that each bit indicates one of the four
 *						  queue priorities. Setting each one of these bits indicates
 *						  that this particular rate resource is slated to account for
 *						  incoming frames with the enabled bits' priority.
 *		bktTypeMask		- used if bktRateType is BUCKET_TYPE_TRAFFIC_BASED.
 *						  any combination of the following definitions:
 *								BUCKET_TRAFFIC_UNKNOWN_UNICAST,
 *								BUCKET_TRAFFIC_UNKNOWN_MULTICAST,
 *								BUCKET_TRAFFIC_BROADCAST,
 *								BUCKET_TRAFFIC_MULTICAST,
 *								BUCKET_TRAFFIC_UNICAST,
 *								BUCKET_TRAFFIC_MGMT_FRAME,
 *								BUCKET_TRAFFIC_ARP,
 *								BUCKET_TRAFFIC_TCP_DATA,
 *								BUCKET_TRAFFIC_TCP_CTRL,
 *								BUCKET_TRAFFIC_UDP,
 *								BUCKET_TRAFFIC_NON_TCPUDP,
 *								BUCKET_TRAFFIC_IMS,
 *								BUCKET_TRAFFIC_POLICY_MIRROR, and
 *								BUCKET_TRAFFIC_PLICY_TRAP
 *		byteTobeCounted	- bytes to be counted for accounting
 *								GT_PIRL2_COUNT_FRAME,
 *								GT_PIRL2_COUNT_ALL_LAYER1,
 *								GT_PIRL2_COUNT_ALL_LAYER2, or
 *								GT_PIRL2_COUNT_ALL_LAYER3
 *
 */
typedef struct
{
	GT_BOOL		accountQConf;
	GT_BOOL		accountFiltered;
	GT_BOOL		mgmtNrlEn;
	GT_BOOL		saNrlEn;
	GT_BOOL		daNrlEn;
	GT_BOOL		samplingMode;
	GT_PIRL_ACTION	actionMode;
	GT_ESB_LIMIT_ACTION ebsLimitAction;
	GT_U32		ebsLimit;
	GT_U32		cbsLimit;
	GT_U32		bktRateFactor;
	GT_U32		bktIncrement;
	GT_PIRL_FC_DEASSERT fcDeassertMode;
	GT_BUCKET_RATE_TYPE	bktRateType;
	GT_BOOL		priORpt;
	GT_U32		priMask;
	GT_U32		bktTypeMask;
	GT_PIRL2_COUNT_MODE	byteTobeCounted;
} GT_PIRL2_RESOURCE;


/*
 *  typedef: struct GT_PIRL2_BUCKET_DATA
 *
 *  Description: data structure for PIRL2 Bucket programing that is port based.
 *
 *  Fields:
 *		ingressRate   	- commited ingress rate in kbps.
 *						  64kbps ~ 1Mbps    : increments of 64kbps,
 *						  1Mbps ~ 100Mbps   : increments of 1Mbps, and
 *						  100Mbps ~ 200Mbps : increments of 10Mbps
 *						  Therefore, the valid values are:
 *								64, 128, 192, 256, 320, 384,..., 960,
 *								1000, 2000, 3000, 4000, ..., 100000,
 *								110000, 120000, 130000, ..., 200000.
 *		customSetup   	- custom ingress rate parameter setup. please refer to
 *						  GT_PIRL_CUSTOM_RATE_LIMIT structure.
 *      accountQConf    - account discarded frames due to queue congestion
 *      accountFiltered - account filtered frames
 *      mgmtNrlEn 		- exclude management frame from ingress rate limiting calculation
 *      saNrlEn 		- exclude from ingress rate limiting calculation if the SA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *      daNrlEn 		- exclude from ingress rate limiting calculation if the DA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *		samplingMode	- sample one out of so many frames/bytes for a stream of frames
 *		actionMode		- action should be taken when there are not enough tokens
 *						  to accept the entire incoming frame
 *								PIRL_ACTION_ACCEPT - accept the frame
 *								PIRL_ACTION_USE_LIMIT_ACTION - use limitAction
 *		ebsLimitAction 	- action should be taken when the incoming rate exceeds
 *						  the limit.
 *								ESB_LIMIT_ACTION_DROP - drop packets
 *								ESB_LIMIT_ACTION_FC   - send flow control packet
 *		fcDeassertMode	- flow control de-assertion mode when limitAction is
 *						  set to ESB_LIMIT_ACTION_FC.
 *								GT_PIRL_FC_DEASSERT_EMPTY -
 *									De-assert when the ingress rate resource has
 *									become empty.
 *								GT_PIRL_FC_DEASSERT_CBS_LIMIT -
 *									De-assert when the ingress rate resource has
 *									enough room as specified by the CBSLimit.
 *		bktRateType		- bucket is either rate based or traffic type based.
 *								BUCKET_TYPE_RATE_BASED, or
 *								BUCKET_TYPE_TRAFFIC_BASED
 *      priORpt         - determine the incoming frames that get rate limited using
 *						  this ingress rate resource.
 *						  		GT_TRUE - typeMask OR priMask
 *						  		GT_FALSE - typeMask AND priMask
 *		priMask         - priority bit mask that each bit indicates one of the four
 *						  queue priorities. Setting each one of these bits indicates
 *						  that this particular rate resource is slated to account for
 *						  incoming frames with the enabled bits' priority.
 *		bktTypeMask		- used if bktRateType is BUCKET_TYPE_TRAFFIC_BASED.
 *						  any combination of the following definitions:
 *								BUCKET_TRAFFIC_UNKNOWN_UNICAST,
 *								BUCKET_TRAFFIC_UNKNOWN_MULTICAST,
 *								BUCKET_TRAFFIC_BROADCAST,
 *								BUCKET_TRAFFIC_MULTICAST,
 *								BUCKET_TRAFFIC_UNICAST,
 *								BUCKET_TRAFFIC_MGMT_FRAME,
 *								BUCKET_TRAFFIC_ARP,
 *								BUCKET_TRAFFIC_TCP_DATA,
 *								BUCKET_TRAFFIC_TCP_CTRL,
 *								BUCKET_TRAFFIC_UDP,
 *								BUCKET_TRAFFIC_NON_TCPUDP,
 *								BUCKET_TRAFFIC_IMS,
 *								BUCKET_TRAFFIC_POLICY_MIRROR, and
 *								BUCKET_TRAFFIC_PLICY_TRAP
 *		byteTobeCounted	- bytes to be counted for accounting
 *								GT_PIRL2_COUNT_FRAME,
 *								GT_PIRL2_COUNT_ALL_LAYER1,
 *								GT_PIRL2_COUNT_ALL_LAYER2, or
 *								GT_PIRL2_COUNT_ALL_LAYER3
 *
 */
typedef struct
{
	GT_U32		ingressRate;
	GT_PIRL_CUSTOM_RATE_LIMIT customSetup;
	GT_BOOL		accountQConf;
	GT_BOOL		accountFiltered;
	GT_BOOL		mgmtNrlEn;
	GT_BOOL		saNrlEn;
	GT_BOOL		daNrlEn;
	GT_BOOL		samplingMode;
	GT_PIRL_ACTION	actionMode;
	GT_ESB_LIMIT_ACTION ebsLimitAction;
	GT_PIRL_FC_DEASSERT fcDeassertMode;
	GT_BUCKET_RATE_TYPE	bktRateType;
	GT_BOOL		priORpt;
	GT_U32		priMask;
	GT_U32		bktTypeMask;
	GT_PIRL2_COUNT_MODE	byteTobeCounted;
} GT_PIRL2_DATA;

#define MAX_PTP_CONSECUTIVE_READ	4

/*
 * Typedef: enum GT_PTP_OPERATION
 *
 * Description: Defines the PTP (Precise Time Protocol) Operation type
 *
 * Fields:
 *      PTP_WRITE_DATA 			- Write data to the PTP register
 *      PTP_READ_DATA			- Read data from PTP register
 *      PTP_READ_MULTIPLE_DATA	- Read multiple data from PTP register
 */
typedef enum
{
	PTP_WRITE_DATA 			= 0x3,
	PTP_READ_DATA  			= 0x4,
	PTP_READ_MULTIPLE_DATA	= 0x6,
} GT_PTP_OPERATION;


/*
 *  typedef: struct GT_PTP_OP_DATA
 *
 *  Description: data required by PTP Operation
 *
 *  Fields:
 *      ptpPort		- physical port of the device
 *      ptpAddr 	- register address
 *      ptpData 	- data for ptp register.
 *      ptpMultiData- used for multiple read operation.
 *      nData 		- number of data to be read on multiple read operation.
 */
typedef struct
{
	GT_U32	ptpPort;
	GT_U32	ptpAddr;
	GT_U32	ptpData;
	GT_U32	ptpMultiData[MAX_PTP_CONSECUTIVE_READ];
	GT_U32	nData;
} GT_PTP_OP_DATA;


/*
 *  typedef: struct GT_PTP_CONFIG
 *
 *  Description: PTP configuration parameters
 *
 *  Fields:
 *      ptpEType	- PTP Ether Type
 *      msgIdTSEn 	- Message IDs that needs time stamp
 *      tsArrPtr 	- Time stamp arrival time counter pointer (either Arr0Time or Arr1Time)
 *      ptpArrIntEn	- PTP port arrival interrupt enable
 *      ptpDepIntEn	- PTP port departure interrupt enable
 *      transSpec 	- Transport Specific bits in PTP common header
 *      msgIdStartBit 	- Message ID starting bit in the PTP common header
 *      disTSOverwrite 	- disable time stamp counter overwriting until the corresponding
 *						  timer valid bit is cleared.
 */
typedef struct
{
	GT_U32	ptpEType;
	GT_U32	msgIdTSEn;
	GT_U32	tsArrPtr;
	GT_U32	ptpArrIntEn;
	GT_U32	ptpDepIntEn;
	GT_U32	transSpec;
	GT_U32	msgIdStartBit;
	GT_BOOL	disTSOverwrite;
} GT_PTP_CONFIG;


/*
 * Typedef: enum GT_PTP_TIME
 *
 * Description: Defines the PTP Time to be read
 *
 * Fields:
 *      PTP_WRITE_DATA 			- Write data to the PTP register
 *      PTP_READ_DATA			- Read data from PTP register
 *      PTP_READ_MULTIPLE_DATA	- Read multiple data from PTP register
 */
typedef enum
{
	PTP_ARR0_TIME = 0x0,
	PTP_ARR1_TIME = 0x1,
	PTP_DEP_TIME = 0x2
} GT_PTP_TIME;


/*
 * Typedef: enum GT_PTP_INT_STATUS
 *
 * Description: Defines the PTP Port interrupt status for time stamp
 *
 * Fields:
 *      PTP_INT_NORMAL		- No error condition occurred
 *      PTP_INT_OVERWRITE 	- PTP logic has to process more than one PTP frame
 *						  		that needs time stamping before the current read.
 *								Only the latest one is saved.
 *      PTP_INT_DROP	  	- PTP logic has to process more than one PTP frame
 *						  		that needs time stamping before the current read.
 *								Only the oldest one is saved.
 *
 */
typedef enum
{
	PTP_INT_NORMAL 		= 0x0,
	PTP_INT_OVERWRITE 	= 0x1,
	PTP_INT_DROP 		= 0x2
} GT_PTP_INT_STATUS;


/*
 *  typedef: struct GT_PTP_TS_STATUS
 *
 *  Description: PTP port status of time stamp
 *
 *  Fields:
 *      isValid		- time stamp is valid
 *      status		- time stamp error status
 *      timeStamped	- time stamp value of a PTP frame that needs to be time stamped
 *      ptpSeqId	- sequence ID of the frame whose time stamp information has been captured
 */
typedef struct
{
	GT_BOOL	isValid;
	GT_U32	timeStamped;
	GT_U32	ptpSeqId;
	GT_PTP_INT_STATUS	status;
} GT_PTP_TS_STATUS;


#ifdef CONFIG_AVB_FPGA

typedef enum
{
	PTP_CLOCK_SRC_AD_DEVICE = 0,	/* PTP Clock source is from A/D device */
	PTP_CLOCK_SRC_FPGA				/* PTP Clock source is from Cesium FPGA */
} GT_PTP_CLOCK_SRC;

typedef enum
{
	PTP_P9_MODE_GMII = 0, 	/* Port 9 uses GMII connect to 88E1111 */
	PTP_P9_MODE_MII,		/* Port 9 uses MII connect to 88E1111 */
	PTP_P9_MODE_MII_CONNECTOR,		/* Port 9 connect to MII connector */
	PTP_P9_MODE_JUMPER		/* Use Jumper setup */
} GT_PTP_P9_MODE;

typedef enum
{
	GT_PTP_SIGN_NEGATIVE = 0,	/* apply Minus sign to the Duty Cycle */
	GT_PTP_SIGN_PLUS			/* apply Plus sign to the Duty Cycle */
} GT_PTP_SIGN;

typedef struct
{
	GT_PTP_SIGN	adjSign;	/* determine the plus/minus sign of the duty cycle adj */
	GT_U32	cycleStep;		/* number of steps which will be applied in adjusting the duty cycle high time
								of the 8KHz clock cycle.
								valid values are 0 ~ 7 */
	GT_U32	cycleInterval;	/* define the interval of clock cycles for which a duty cycle adj will occur */
	GT_U32	cycleAdjust;	/* define the number of 8KHz clock cycles for which duty cycle adj will occur
								within each PTP clock clycle interval.
								Note that (cycleAdjust <= cycleInterval) for proper operation */
} GT_PTP_CLOCK_ADJUSTMENT;

#endif


/*
 * typedef: enum GT_EVENT_TYPE
 *
 * Description: Enumeration of the available hardware driven events.
 *
 * Enumerations:
 *   GT_AVB_INT    - AVB Interrupt Enable
 *   GT_DEVICE_INT - Device Interrupt (GT_DEVICE_INT_TYPE) Enable
 *   GT_STATS_DONE - Statistics Operation Done interrrupt Enable
 *   GT_VTU_PROB - VLAN Problem/Violation Interrupt Enable
 *   GT_VTU_DONE - VALN Table Operation Done Interrupt Enable
 *   GT_ATU_PROB - ATU Problem/Violation Interrupt Enable, for Gigabit Switch
 *   GT_ATU_FULL - ATU full interrupt enable, for Fast Ethernet Switch
 *   GT_ATU_DONE - ATU Done interrupt enable.
 *   GT_PHY_INT  - PHY interrupt enable, for Fast Ethernet Switch
 *   GT_EE_INT   - EEPROM Done interrupt enable.
 */
#define GT_AVB_INT           	0x100
#define GT_DEVICE_INT           0x80
#define GT_STATS_DONE           0x40
#define GT_VTU_PROB             0x20
#define GT_VTU_DONE             0x10
#define GT_ATU_PROB 		0x8
#define GT_ATU_FULL 		0x8
#define GT_ATU_DONE			0x4
#define GT_PHY_INTERRUPT	0x2		/* Device may not support PHY Int. Please refer to datasheet. */
#define GT_EE_INTERRUPT		0x1

#define GT_INT_MASK			\
		(GT_AVB_INT | GT_DEVICE_INT | GT_STATS_DONE | GT_VTU_PROB | GT_VTU_DONE | GT_ATU_FULL | 	\
		GT_ATU_DONE | GT_PHY_INTERRUPT | GT_EE_INTERRUPT)
#define GT_NO_INTERNAL_PHY_INT_MASK		\
		(GT_AVB_INT | GT_DEVICE_INT | GT_STATS_DONE | GT_VTU_PROB | GT_VTU_DONE | GT_ATU_PROB | 	\
		GT_ATU_DONE | GT_EE_INTERRUPT)


/*
 *  typedef: struct GT_DEV_EVENT
 *
 *  Description: Device interrupt status
 *
 *  Fields:
 *      event     - Device Interrupts to be enabled
 *					GT_DEV_INT_WATCHDOG, GT_DEV_INT_JAMLIMIT,
 *					GT_DEV_INT_DUPLEX_MISMATCH, and/or GT_DEV_INT_SERDES_LINK
 *      portList  - SERDES port list where GT_DEV_INT_SERDES_LINK interrupt needs
 *					to be asserted. It's in vector format, Bit 10 is for port 10,
 *					Bit 9 is for port 9, etc.
 *					valid only if GT_DEV_INT_SERDES_LINK bit is set.
 *      phyList   - Phy list where GT_DEV_INT_PHY interrupt needs to be asserted.
 *					It's in vector format, Bit 0 is for port 0,
 *					Bit 1 is for port 1, etc.
 *					valid only if GT_DEV_INT_PHY bit is set.
 */
typedef struct
{
    GT_U32		event;
	GT_U32		portList;
	GT_U32		phyList;
} GT_DEV_EVENT;


/*
 *  typedef: struct GT_DEV_INT_STATUS
 *
 *  Description: Device interrupt status
 *
 *  Fields:
 *      intCause  - Device Interrupt Cause
 *					GT_DEV_INT_WATCHDOG, GT_DEV_INT_JAMLIMIT,
 *					GT_DEV_INT_DUPLEX_MISMATCH, and/or GT_DEV_INT_SERDES_LINK
 *		port	  - logical port where GT_DEV_INT_DUPLEX_MISMATCH occurred.
 *					valid only if GT_DEV_INT_DUPLEX_MISMATCH is set.
 *      linkInt   - SERDES port list where GT_DEV_INT_SERDES_LINK interrupt is
 *					asserted. It's in vector format, Bit 10 is for port 10,
 *					Bit 9 is for port 9, etc.
 *					valid only if GT_DEV_INT_SERDES_LINK bit is set.
 *					These bits are only valid of the port that is in 1000Base-X mode.
 */
typedef struct
{
    GT_U32		devIntCause;
	GT_LPORT	port;
    GT_U32		linkInt;
    GT_U32		phyInt;
} GT_DEV_INT_STATUS;


/*
* GT_DEVICE_INT
*
* Description: Enumeration of Device interrupt
*	GT_DEV_INT_WATCHDOG		- WatchDog event interrupt (WatchDog event can be
*							  configured with gwdSetEvent API)
*	GT_DEV_INT_JAMLIMIT		- any of the ports detect an Ingress Jam Limit violation
*							  (gprtSetPauseLimitIn API)
*	GT_DEV_INT_DUPLEX_MISMATCH	- any of the ports detect a duplex mismatch
*							  (i.e., the local port is in half duplex mode while
*							  the link partner is in full duplex mode)
*	GT_DEV_INT_SERDES_LINK	- SERDES link chage interrupt.
*							  An interrupt occurs when a SERDES port changes link
*							  status (link up or link down)
*/

#define GT_DEV_INT_WATCHDOG			0x8
#define GT_DEV_INT_JAMLIMIT			0x4
#define GT_DEV_INT_DUPLEX_MISMATCH	0x2
#define GT_DEV_INT_SERDES_LINK		0x1
#define GT_DEV_INT_PHY				0x10

/*
* GT_WATCHDOG_EVENT
*
* Description: Enumeration of WatchDog event
*		GT_WD_QC  - Queue Controller Watch Dog enable.
*					When enabled, the QC's watch dog circuit checks for link
*					list errors and any errors found in the QC.
*		GT_WD_EGRESS - Egress Watch Dog enable.
*					When enabled, each port's egress circuit checks for problems
*					between the port and the Queue Controller.
*		GT_WD_FORCE - Force a Watch Dog event.
*/

#define GT_WD_QC		0x1
#define GT_WD_EGRESS	0x2
#define GT_WD_FORCE		0x4


/*
* typedef: struct GT_WD_EVENT_HISTORY
*
* Description: WatchDog Event History (cleared only by a hardware reset)
*		wdEvent   - When it's set to GT_TRUE, some enabled Watch Dog event occurred.
*					The following events are possible:
*						QC WatchDog Event (GT_WD_QC)
*						Egress WatchDog Event (GT_WD_EGRESS)
*						Forced WatchDog Event (GT_WD_FORCE)
*		egressEvent-If any port's egress logic detects an egress watch dog issue,
*					this field is set to GT_TRUE, regardless of the enabling GT_WD_EGRESS
*					event.
*/
typedef struct
{
	GT_BOOL	wdEvent;
	GT_BOOL egressEvent;
} GT_WD_EVENT_HISTORY;


/*
* typedef: enum GT_PHY_INT
*
* Description: Enumeration of PHY interrupt
*/

#define GT_SPEED_CHANGED 		0x4000
#define GT_DUPLEX_CHANGED		0x2000
#define GT_PAGE_RECEIVED		0x1000
#define GT_AUTO_NEG_COMPLETED	0x800
#define GT_LINK_STATUS_CHANGED	0x400
#define GT_SYMBOL_ERROR			0x200
#define GT_FALSE_CARRIER		0x100
#define GT_FIFO_FLOW			0x80
#define GT_CROSSOVER_CHANGED	0x40
#define GT_POLARITY_CHANGED		0x2
#define GT_JABBER				0x1

#define GT_AUTO_NEG_ERROR		0x8000
#define GT_DOWNSHIFT_DETECT		0x20
#define GT_ENERGY_DETECT		0x10

/*
* typedef: enum GT_PHY_AUTO_MODE
*
* Description: Enumeration of Autonegotiation mode.
*	Auto for both speed and duplex.
*	Auto for speed only and Full duplex.
*	Auto for speed only and Half duplex. (1000Mbps is not supported)
*	Auto for duplex only and speed 1000Mbps.
*	Auto for duplex only and speed 100Mbps.
*	Auto for duplex only and speed 10Mbps.
*	1000Mbps Full duplex.
*	100Mbps Full duplex.
*	100Mbps Half duplex.
*	10Mbps Full duplex.
*	10Mbps Half duplex.
*/

typedef enum
{
	SPEED_AUTO_DUPLEX_AUTO,
	SPEED_1000_DUPLEX_AUTO,
	SPEED_100_DUPLEX_AUTO,
	SPEED_10_DUPLEX_AUTO,
	SPEED_AUTO_DUPLEX_FULL,
	SPEED_AUTO_DUPLEX_HALF,
	SPEED_1000_DUPLEX_FULL,
	SPEED_1000_DUPLEX_HALF,
	SPEED_100_DUPLEX_FULL,
	SPEED_100_DUPLEX_HALF,
	SPEED_10_DUPLEX_FULL,
	SPEED_10_DUPLEX_HALF
}GT_PHY_AUTO_MODE;


/*
* typedef: enum GT_PHY_PAUSE_MODE
*
* Description: Enumeration of Pause Mode in the Phy.
*
* Enumerations:
*	GT_PHY_NO_PAUSE		- disable pause
*	GT_PHY_PAUSE		- support pause
*	GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*	GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*/
typedef enum
{
	GT_PHY_NO_PAUSE = 0,
	GT_PHY_PAUSE,
	GT_PHY_ASYMMETRIC_PAUSE,
	GT_PHY_BOTH_PAUSE
} GT_PHY_PAUSE_MODE;


/*
* typedef: enum GT_PHY_SPEED
*
* Description: Enumeration of Phy Speed
*
* Enumerations:
*	PHY_SPEED_10_MBPS   - 10Mbps
*	PHY_SPEED_100_MBPS	- 100Mbps
*	PHY_SPEED_1000_MBPS - 1000Mbps
*/
typedef enum
{
	PHY_SPEED_10_MBPS,
	PHY_SPEED_100_MBPS,
	PHY_SPEED_1000_MBPS
} GT_PHY_SPEED;


/*
* typedef: enum GT_SERDES_MODE
*
* Description: Enumeration of Serdes mode
*
* Enumerations:
*	PHY_SERDES_100FX	 - 100 FX
*	PHY_SERDES_1000X	 - 1000 X
*	PHY_SERDES_SGMII_PHY - SGMII PHY
*	PHY_SERDES_SGMII_MAC - SGMII MAC
*/
typedef enum
{
	PHY_SERDES_100FX = 0,
	PHY_SERDES_1000X,
	PHY_SERDES_SGMII_PHY,
	PHY_SERDES_SGMII_MAC
} GT_SERDES_MODE;


/*
* typedef: enum GT_EDETECT_MODE
*
* Description: Enumeration of Energy Detect mode
*
* Enumerations:
*	GT_EDETECT_OFF		- Energy Detect disabled
*	GT_EDETECT_SENSE_PULSE	- Energy Detect enabled with sense and pulse
*	GT_EDETECT_SENSE	- Energy Detect enabled only with sense
*/
typedef enum
{
	GT_EDETECT_OFF = 0,
	GT_EDETECT_SENSE_PULSE,
	GT_EDETECT_SENSE
} GT_EDETECT_MODE;

/*
 * typedef: enum GT_INGRESS_MODE
 *
 * Description: Enumeration of the port ingress mode.
 *
 * Enumerations:
 *   GT_UNMODIFY_INGRESS - frames are receive unmodified.
 *   GT_TRAILER_INGRESS  - all frames are received with trailer.
 *   GT_UNTAGGED_INGRESS  - remove tag on receive (for double tagging).
 *   GT_CPUPORT_INGRESS - no trailer. used to identify the CPU port for IGMP/MLD Snooping
 */
typedef enum
{
    GT_UNMODIFY_INGRESS = 0,  /* 0x00 */
    GT_TRAILER_INGRESS,       /* 0x01 */
    GT_UNTAGGED_INGRESS,      /* 0x10 */
    GT_CPUPORT_INGRESS        /* 0x11 */
} GT_INGRESS_MODE;


/*
 * typedef: enum GT_EGRESS_FLOOD
 *
 * Description: Enumeration of the port ingress mode.
 *
 * Enumerations:
 *   GT_BLOCK_EGRESS_UNKNOWN - do not egress frame with unknown DA
 *   GT_BLOCK_EGRESS_UNKNOWN_MULTICAST - do not egress frame with unknown multicast DA
 *   GT_BLOCK_EGRESS_UNKNOWN_UNIICAST - do not egress frame with unknown unicast DA
 *   GT_BLOCK_EGRESS_NONE - egress all frames with unknown DA
 */
typedef enum
{
    GT_BLOCK_EGRESS_UNKNOWN = 0,
    GT_BLOCK_EGRESS_UNKNOWN_MULTICAST,
    GT_BLOCK_EGRESS_UNKNOWN_UNICAST,
    GT_BLOCK_EGRESS_NONE
} GT_EGRESS_FLOOD;


/*
 *  typedef: enum GT_MC_RATE
 *
 *  Description: Enumeration of the port ingress mode.
 *
 *  Enumerations:
 *      GT_MC_3_PERCENT_RL   - multicast rate is limited to 3 percent.
 *      GT_MC_6_PERCENT_RL   - multicast rate is limited to 6 percent.
 *      GT_MC_12_PERCENT_RL  - multicast rate is limited to 12 percent.
 *      GT_MC_100_PERCENT_RL - unlimited multicast rate.
 */
typedef enum
{
    GT_MC_3_PERCENT_RL = 0,
    GT_MC_6_PERCENT_RL,
    GT_MC_12_PERCENT_RL,
    GT_MC_100_PERCENT_RL,
} GT_MC_RATE;


/*
 *  typedef: enum GT_INGRESS_RATE_MODE
 *
 *  Description: Enumeration of the port ingress rate limit mode.
 *
 *  Enumerations:
 *      GT_RATE_PRI_BASE   - Priority based rate limiting
 *		GT_RATE_BURST_BASE - Burst Size based rate limiting
 */
typedef enum
{
    GT_RATE_PRI_BASE = 0,
    GT_RATE_BURST_BASE
} GT_INGRESS_RATE_MODE;


/*
 *  typedef: enum GT_PORT_SCHED_MODE
 *
 *  Description: Enumeration of port scheduling mode
 *
 *  Fields:
 *	 	GT_PORT_SCHED_WEIGHTED_RRB - use 8,4,2,1 weighted fair scheduling
 *	 	GT_PORT_SCHED_STRICT_PRI - use a strict priority scheme
 *	 	GT_PORT_SCHED_STRICT_PRI3 - use a strict for priority 3 and weighted
 *									round robin for the priority 2,1,and 0
 *	 	GT_PORT_SCHED_STRICT_PRI2_3 - use a strict for priority 2,3 and weighted
 *									round robin for the priority 1,and 0
 *
 *  Comment:
 */
typedef enum
{
    GT_PORT_SCHED_WEIGHTED_RRB = 0,
    GT_PORT_SCHED_STRICT_PRI,
    GT_PORT_SCHED_STRICT_PRI3,
    GT_PORT_SCHED_STRICT_PRI2_3
} GT_PORT_SCHED_MODE;


/*
 *  typedef: struct GT_PORT_STAT
 *
 *  Description: port statistic struct.
 *
 *  Fields:
 *      rxCtr   - port receive counter.
 *      txCtr   - port transmit counter.
 *      dropped - dropped frame counter.
 *
 *  Comment:
 *		dropped frame counter is supported by only limited devices.
 *		At this moment, 88E6061/88E6065 are the devices supporting
 *		dropped frame counter.
 */
typedef struct
{
    GT_U16  rxCtr;
    GT_U16  txCtr;
    GT_U16  dropped;
} GT_PORT_STAT;

/*
 *  typedef: struct GT_PORT_STAT2
 *
 *  Description: port statistic struct.
 *
 *  Fields:
 *      inDiscardLo - InDiscards Low Frame Counter
 *      inDiscardHi - InDiscards High Frame Counter
 *      inFiltered  - InFiltered Frame Counter
 *      outFiltered - OutFiltered Frame Counter
 *
 *  Comment:
 */
typedef struct
{
    GT_U16  inDiscardLo;
    GT_U16  inDiscardHi;
    GT_U16  inFiltered;
    GT_U16  outFiltered;
} GT_PORT_STAT2;


/*
 **  typedef: struct GT_PORT_Q_COUNTERS
 **
 **  Description: port queue statistic struct.
 **
 **  Fields:
 **      OutQ_Size - port egress queue size coi
 **      Rsv_Size  - ingress reserved e counter
 **
 **/
typedef struct
{
    GT_U16  OutQ_Size;
    GT_U16  Rsv_Size;
} GT_PORT_Q_STAT;

/*
 * typedef: enum GT_CTR_MODE
 *
 * Description: Enumeration of the port counters mode.
 *
 * Enumerations:
 *   GT_CTR_ALL    - In this mode the counters counts Rx receive and transmit
 *                   frames.
 *   GT_CTR_ERRORS - In this mode the counters counts Rx Errors and collisions.
 */
typedef enum
{
    GT_CTR_ALL = 0,
    GT_CTR_ERRORS,
} GT_CTR_MODE;

typedef struct _GT_QD_DEV GT_QD_DEV;

/*
 * semaphore related definitions.
 * User Applications may register Semaphore functions using following definitions
 */
typedef enum
{
	GT_SEM_EMPTY,
	GT_SEM_FULL
} GT_SEM_BEGIN_STATE;

typedef GT_SEM (*FGT_SEM_CREATE)(
						GT_SEM_BEGIN_STATE state);
typedef GT_STATUS (*FGT_SEM_DELETE)(
						GT_SEM semId);
typedef GT_STATUS (*FGT_SEM_TAKE)(
						GT_SEM semId, GT_U32 timOut);
typedef GT_STATUS (*FGT_SEM_GIVE)(
						GT_SEM semId);

typedef struct
{
	FGT_SEM_CREATE	semCreate; 	/* create semapore */
	FGT_SEM_DELETE	semDelete; 	/* delete the semapore */
	FGT_SEM_TAKE	semTake;	/* try to get a semapore */
	FGT_SEM_GIVE	semGive;	/* return semaphore */
}GT_SEM_ROUTINES;

/*
 * definitions for registering MII access functions.
 *
*/
typedef GT_BOOL (*FGT_READ_MII)(
						GT_QD_DEV*   dev,
						unsigned int phyAddr,
						unsigned int miiReg,
						unsigned int* value);
typedef GT_BOOL (*FGT_WRITE_MII)(
						GT_QD_DEV*   dev,
						unsigned int phyAddr,
						unsigned int miiReg,
						unsigned int value);
typedef GT_BOOL (*FGT_INT_HANDLER)(
						GT_QD_DEV*   dev,
						GT_U16*);

typedef struct _BSP_FUNCTIONS
{
	FGT_READ_MII 	readMii;	/* read MII Registers */
	FGT_WRITE_MII 	writeMii;	/* write MII Registers */
	FGT_SEM_CREATE	semCreate; 	/* create semapore */
	FGT_SEM_DELETE	semDelete; 	/* delete the semapore */
	FGT_SEM_TAKE	semTake;	/* try to get a semapore */
	FGT_SEM_GIVE	semGive;	/* return semaphore */
}BSP_FUNCTIONS;


/*
 *	Type definition for MIB counter operation
*/
typedef enum
{
	STATS_FLUSH_ALL,		/* Flush all counters for all ports */
	STATS_FLUSH_PORT,		/* Flush all counters for a port */
	STATS_READ_COUNTER,		/* Read a specific counter from a port */
	STATS_READ_REALTIME_COUNTER,	/* Read a realtime counter from a port */
	STATS_READ_ALL			/* Read all counters from a port */

} GT_STATS_OPERATION;

typedef struct _GT_STATS_COUNTER_SET
{
	GT_U32	InUnicasts;
	GT_U32	InBroadcasts;
	GT_U32	InPause;
	GT_U32	InMulticasts;
	GT_U32	InFCSErr;
	GT_U32	AlignErr;
	GT_U32	InGoodOctets;
	GT_U32	InBadOctets;
	GT_U32	Undersize;
	GT_U32	Fragments;
	GT_U32	In64Octets;		/* 64 Octets */
	GT_U32	In127Octets;	/* 65 to 127 Octets */
	GT_U32	In255Octets;	/* 128 to 255 Octets */
	GT_U32	In511Octets;	/* 256 to 511 Octets */
	GT_U32	In1023Octets;	/* 512 to 1023 Octets */
	GT_U32	InMaxOctets;	/* 1024 to Max Octets */
	GT_U32	Jabber;
	GT_U32	Oversize;
	GT_U32	InDiscards;
	GT_U32	Filtered;
	GT_U32	OutUnicasts;
	GT_U32	OutBroadcasts;
	GT_U32	OutPause;
	GT_U32	OutMulticasts;
	GT_U32	OutFCSErr;
	GT_U32	OutGoodOctets;
	GT_U32	Out64Octets;	/* 64 Octets */
	GT_U32	Out127Octets;	/* 65 to 127 Octets */
	GT_U32	Out255Octets;	/* 128 to 255 Octets */
	GT_U32	Out511Octets;	/* 256 to 511 Octets */
	GT_U32	Out1023Octets;	/* 512 to 1023 Octets */
	GT_U32	OutMaxOctets;	/* 1024 to Max Octets */
	GT_U32	Collisions;
	GT_U32	Late;
	GT_U32	Excessive;
	GT_U32	Multiple;
	GT_U32	Single;
	GT_U32	Deferred;
	GT_U32	OutDiscards;

} GT_STATS_COUNTER_SET;


typedef enum
{
	STATS_InUnicasts = 0,
	STATS_InBroadcasts,
	STATS_InPause,
	STATS_InMulticasts,
	STATS_InFCSErr,
	STATS_AlignErr,
	STATS_InGoodOctets,
	STATS_InBadOctets,
	STATS_Undersize,
	STATS_Fragments,
	STATS_In64Octets,
	STATS_In127Octets,
	STATS_In255Octets,
	STATS_In511Octets,
	STATS_In1023Octets,
	STATS_InMaxOctets,
	STATS_Jabber,
	STATS_Oversize,
	STATS_InDiscards,
	STATS_Filtered,
	STATS_OutUnicasts,
	STATS_OutBroadcasts,
	STATS_OutPause,
	STATS_OutMulticasts,
	STATS_OutFCSErr,
	STATS_OutGoodOctets,
	STATS_Out64Octets,
	STATS_Out127Octets,
	STATS_Out255Octets,
	STATS_Out511Octets,
	STATS_Out1023Octets,
	STATS_OutMaxOctets,
	STATS_Collisions,
	STATS_Late,
	STATS_Excessive,
	STATS_Multiple,
	STATS_Single,
	STATS_Deferred,
	STATS_OutDiscards

} GT_STATS_COUNTERS;
/*
 * typedef: enum GT_HISTOGRAM_MODE
 *
 * Description: Enumeration of the histogram counters mode.
 *
 * Enumerations:
 *   GT_COUNT_RX_ONLY - In this mode, Rx Histogram Counters are counted.
 *   GT_COUNT_TX_ONLY - In this mode, Tx Histogram Counters are counted.
 *   GT_COUNT_RX_TX   - In this mode, Rx and Tx Histogram Counters are counted.
 */
typedef enum
{
    GT_COUNT_RX_ONLY = 0,
    GT_COUNT_TX_ONLY,
	GT_COUNT_RX_TX
} GT_HISTOGRAM_MODE;

/*
	Counter set 2 is used by 88E6183
*/
typedef struct _GT_STATS_COUNTER_SET2
{
	GT_U32	InGoodOctetsHi;
	GT_U32	InGoodOctetsLo;
	GT_U32	InBadOctets;
	GT_U32	OutDiscards;
	GT_U32	InGoodFrames;
	GT_U32	InBadFrames;
	GT_U32	InBroadcasts;
	GT_U32	InMulticasts;
	/*
		Histogram Counters : Rx Only, Tx Only, or both Rx and Tx
		(refer to Histogram Mode)
	*/
	GT_U32	Octets64;		/* 64 Octets */
	GT_U32	Octets127;		/* 65 to 127 Octets */
	GT_U32	Octets255;		/* 128 to 255 Octets */
	GT_U32	Octets511;		/* 256 to 511 Octets */
	GT_U32	Octets1023;		/* 512 to 1023 Octets */
	GT_U32	OctetsMax;		/* 1024 to Max Octets */
	GT_U32	OutOctetsHi;
	GT_U32	OutOctetsLo;
	GT_U32	OutFrames;
	GT_U32	Excessive;
	GT_U32	OutMulticasts;
	GT_U32	OutBroadcasts;
	GT_U32	InBadMACCtrl;

	GT_U32	OutPause;
	GT_U32	InPause;
	GT_U32	InDiscards;
	GT_U32	Undersize;
	GT_U32	Fragments;
	GT_U32	Oversize;
	GT_U32	Jabber;
	GT_U32	MACRcvErr;
	GT_U32	InFCSErr;
	GT_U32	Collisions;
	GT_U32	Late;

} GT_STATS_COUNTER_SET2;


typedef enum
{
	STATS2_InGoodOctetsHi = 0,
	STATS2_InGoodOctetsLo,
	STATS2_InBadOctets,

	STATS2_OutDiscards,
	STATS2_InGoodFrames,
	STATS2_InBadFrames,
	STATS2_InBroadcasts,
	STATS2_InMulticasts,
	STATS2_64Octets,
	STATS2_127Octets,
	STATS2_255Octets,
	STATS2_511Octets,
	STATS2_1023Octets,
	STATS2_MaxOctets,
	STATS2_OutOctetsHi,
	STATS2_OutOctetsLo,
	STATS2_OutFrames,
	STATS2_Excessive,
	STATS2_OutMulticasts,
	STATS2_OutBroadcasts,
	STATS2_InBadMACCtrl,
	STATS2_OutPause,
	STATS2_InPause,
	STATS2_InDiscards,
	STATS2_Undersize,
	STATS2_Fragments,
	STATS2_Oversize,
	STATS2_Jabber,
	STATS2_MACRcvErr,
	STATS2_InFCSErr,
	STATS2_Collisions,
	STATS2_Late

} GT_STATS_COUNTERS2;

/*
	Counter set 3 is used by 88E6093 and 88E6065
*/
typedef struct _GT_STATS_COUNTER_SET3
{
	GT_U32	InGoodOctetsLo;	/* offset 0 */
	GT_U32	InGoodOctetsHi;	/* offset 1, not supported by 88E6065 */
	GT_U32	InBadOctets;		/* offset 2 */
	GT_U32	OutFCSErr;			/* offset 3 */
	GT_U32	InUnicasts;			/* offset 4 */
	GT_U32	Deferred;			/* offset 5 */
	GT_U32	InBroadcasts;		/* offset 6 */
	GT_U32	InMulticasts;		/* offset 7 */
	/*
		Histogram Counters : Rx Only, Tx Only, or both Rx and Tx
		(refer to Histogram Mode)
	*/
	GT_U32	Octets64;		/* 64 Octets, offset 8 */
	GT_U32	Octets127;		/* 65 to 127 Octets, offset 9 */
	GT_U32	Octets255;		/* 128 to 255 Octets, offset 10 */
	GT_U32	Octets511;		/* 256 to 511 Octets, offset 11 */
	GT_U32	Octets1023;		/* 512 to 1023 Octets, offset 12 */
	GT_U32	OctetsMax;		/* 1024 to Max Octets, offset 13 */
	GT_U32	OutOctetsLo;	/* offset 14 */
	GT_U32	OutOctetsHi;	/* offset 15, not supported by 88E6065 */
	GT_U32	OutUnicasts;	/* offset 16 */
	GT_U32	Excessive;		/* offset 17 */
	GT_U32	OutMulticasts;	/* offset 18 */
	GT_U32	OutBroadcasts;	/* offset 19 */
	GT_U32	Single;			/* offset 20 */

	GT_U32	OutPause;		/* offset 21 */
	GT_U32	InPause;			/* offset 22 */
	GT_U32	Multiple;		/* offset 23 */
	GT_U32	Undersize;		/* offset 24 */
	GT_U32	Fragments;		/* offset 25 */
	GT_U32	Oversize;		/* offset 26 */
	GT_U32	Jabber;			/* offset 27 */
	GT_U32	InMACRcvErr;	/* offset 28 */
	GT_U32	InFCSErr;		/* offset 29 */
	GT_U32	Collisions;		/* offset 30 */
	GT_U32	Late;				/* offset 31 */

} GT_STATS_COUNTER_SET3;


typedef enum
{
	STATS3_InGoodOctetsLo = 0,
	STATS3_InGoodOctetsHi,
	STATS3_InBadOctets,

	STATS3_OutFCSErr,
	STATS3_InUnicasts,
	STATS3_Deferred,			/* offset 5 */
	STATS3_InBroadcasts,
	STATS3_InMulticasts,
	STATS3_64Octets,
	STATS3_127Octets,
	STATS3_255Octets,			/* offset 10 */
	STATS3_511Octets,
	STATS3_1023Octets,
	STATS3_MaxOctets,
	STATS3_OutOctetsLo,
	STATS3_OutOctetsHi,
	STATS3_OutUnicasts,		/* offset 16 */
	STATS3_Excessive,
	STATS3_OutMulticasts,
	STATS3_OutBroadcasts,
	STATS3_Single,
	STATS3_OutPause,
	STATS3_InPause,
	STATS3_Multiple,
	STATS3_Undersize,			/* offset 24 */
	STATS3_Fragments,
	STATS3_Oversize,
	STATS3_Jabber,
	STATS3_InMACRcvErr,
	STATS3_InFCSErr,
	STATS3_Collisions,
	STATS3_Late					/* offset 31 */

} GT_STATS_COUNTERS3;



/*
 * typedef: struct GT_1000T_MASTER_SLAVE
 *
 * Description: 1000Base-T Master/Slave Configuration
 *
 * Fields:
 *      autoConfig   - GT_TRUE for auto-config, GT_FALSE for manual setup.
 *      masterPrefer - GT_TRUE if Master configuration is preferred.
 *
 */
typedef struct _GT_1000T_MASTER_SLAVE
{
	GT_BOOL	autoConfig;
	GT_BOOL masterPrefer;
} GT_1000T_MASTER_SLAVE;


#define GT_MDI_PAIR_NUM     	4	/* (1,2),(3,6),(4,5),(7,8) */
#define GT_CHANNEL_PAIR_NUM     2	/* (channel A,B),(channel C,D) */


/*
 * typedef: enum GT_PHY_LINK_STATUS
 *
 * Description: Enumeration of Link Status
 *
 * Enumerations:
 *		GT_PHY_LINK_OFF		- No Link
 *		GT_PHY_LINK_COPPER	- Link on Copper
 *		GT_PHY_LINK_FIBER	- Link on Fiber
 */
typedef enum
{
    GT_PHY_LINK_OFF = 0,
    GT_PHY_LINK_COPPER = 1,
    GT_PHY_LINK_FIBER = 2
} GT_PHY_LINK_STATUS;


/* Definition for packet generator */

/* Payload */
typedef enum
{
    GT_PG_PAYLOAD_RANDOM = 0,	/* Pseudo-random */
    GT_PG_PAYLOAD_5AA5		/* 5A,A5,5A,A5,... */
} GT_PG_PAYLOAD;

/* Length */
typedef enum
{
    GT_PG_LENGTH_64 = 0,		/* 64 bytes */
    GT_PG_LENGTH_1514
} GT_PG_LENGTH;

/* Error */
typedef enum
{
    GT_PG_TX_NORMAL = 0,		/* No Error */
    GT_PG_TX_ERROR			/* Tx packets with CRC error and Symbol error */
} GT_PG_TX;

/* Structure for packet generator */
typedef struct
{
    GT_PG_PAYLOAD  payload;
    GT_PG_LENGTH   length;
    GT_PG_TX       tx;
} GT_PG;


/*
 * typedef: enum GT_TEST_STATUS
 *
 * Description: Enumeration of VCT test status
 *
 * Enumerations:
 *      GT_TEST_FAIL    - virtual cable test failed.
 *      GT_NORMAL_CABLE - normal cable.
 *      GT_IMPEDANCE_MISMATCH - impedance mismatch.
 *      GT_OPEN_CABLE   - open in cable.
 *      GT_SHORT_CABLE  - short in cable.
 *
 */
typedef enum
{
    GT_TEST_FAIL,
    GT_NORMAL_CABLE,
	GT_IMPEDANCE_MISMATCH,
    GT_OPEN_CABLE,
    GT_SHORT_CABLE,
} GT_TEST_STATUS;


/*
 * typedef: enum GT_NORMAL_CABLE_LEN
 *
 * Description: Enumeration for normal cable length
 *
 * Enumerations:
 *      GT_LESS_THAN_50M - cable length less than 50 meter.
 *      GT_50M_80M       - cable length between 50 - 80 meter.
 *      GT_80M_110M      - cable length between 80 - 110 meter.
 *      GT_110M_140M     - cable length between 110 - 140 meter.
 *      GT_MORE_THAN_140 - cable length more than 140 meter.
 *      GT_UNKNOWN_LEN   - unknown length.
 *
 */
typedef enum
{
    GT_LESS_THAN_50M,
    GT_50M_80M,
    GT_80M_110M,
    GT_110M_140M,
    GT_MORE_THAN_140,
    GT_UNKNOWN_LEN,

} GT_NORMAL_CABLE_LEN;


/*
 * typedef: enum GT_CABLE_LEN
 *
 * Description: Enumeration cable length
 *
 * Enumerations:
 *      normCableLen - cable lenght for normal cable.
 *      errCableLen  - for cable failure the estimate fault distance in meters.
 *
 */
typedef union
{
    GT_NORMAL_CABLE_LEN normCableLen;
    GT_U8               errCableLen;

} GT_CABLE_LEN;

/*
 * typedef: struct GT_CABLE_STATUS
 *
 * Description: virtual cable diagnostic status per MDI pair.
 *
 * Fields:
 *      cableStatus - VCT cable status.
 *      cableLen    - VCT cable length.
 *	phyType	    - type of phy (100M phy or Gigabit phy)
 */
typedef struct
{
    GT_TEST_STATUS  cableStatus[GT_MDI_PAIR_NUM];
    GT_CABLE_LEN    cableLen[GT_MDI_PAIR_NUM];
    GT_U16	    phyType;

} GT_CABLE_STATUS;


/*
 * typedef: enum GT_CABLE_TYPE
 *
 * Description: Enumeration of Cable Type
 *
 * Enumerations:
 *		GT_STRAIGHT_CABLE	_ straight cable
 *      GT_CROSSOVER_CABLE 	- crossover cable
 */
typedef enum
{
    GT_STRAIGHT_CABLE,
	GT_CROSSOVER_CABLE

} GT_CABLE_TYPE;

/*
 * typedef: enum GT_POLARITY_STATUS
 *
 * Description: Enumeration of polarity status
 *
 * Enumerations:
 *		GT_POSITIVE	- positive polarity
 *      GT_NEGATIVE	- negative polarity
 */
typedef enum
{
    GT_POSITIVE,
	GT_NEGATIVE

} GT_POLARITY_STATUS;


/*
 * typedef: struct GT_1000BT_EXTENDED_STATUS
 *
 * Description: Currently the 1000Base-T PCS can determine the cable polarity
 * 		on pairs A,B,C,D; crossover on pairs A,B and C,D; and skew among
 *		the pares. These status enhance the capability of the virtual cable tester
 *
 * Fields:
 *      isValid		- GT_TRUE if this structure have valid information,
 * 					  GT_FALSE otherwise.
 *					  It is valid only if 1000BASE-T Link is up.
 *      pairSwap    - GT_CROSSOVER_CABLE, if the cable is crossover,
 *					  GT_STRAIGHT_CABLE, otherwise
 *		pairPolarity- GT_POSITIVE, if polarity is positive,
 *					  GT_NEGATIVE, otherwise
 *		pairSkew	- pair skew in units of ns
 */
typedef struct
{
	GT_BOOL				isValid;
    GT_CABLE_TYPE		pairSwap[GT_CHANNEL_PAIR_NUM];
    GT_POLARITY_STATUS	pairPolarity[GT_MDI_PAIR_NUM];
    GT_U32			    pairSkew[GT_MDI_PAIR_NUM];

} GT_1000BT_EXTENDED_STATUS;


/*
 * if isGigPhy in GT_CABLE_STATUS is not GT_TRUE, cableStatus and cableLen
 * will have only 2 pairs available.
 * One is RX Pair and the other is TX Pair.
 */
#define MDI_RX_PAIR		0	/* cableStatus[0] or cableLen[0] */
#define MDI_TX_PAIR		1	/* cableStatus[1] or cableLen[1] */

/* definition for Phy Type */
#define PHY_100M		0 /* 10/100M phy, E3082 or E3083 */
#define PHY_1000M		1 /* Gigabit phy, the rest phys */
#define PHY_10000M		2 /* 10 Gigabit phy, unused */
#define PHY_1000M_B		3 /* Gigabit phy which needs work-around */
#define PHY_1000M_MP	4 /* Gigabit phy with multiple page mode */


/* definition for Trunking */
#define IS_TRUNK_ID_VALID(_dev, _id)	(((_id) < 16) ? 1 : 0)


/* definition for device scan mode */
#define SMI_AUTO_SCAN_MODE		0	/* Scan 0 or 0x10 base address to find the QD */
#define SMI_MANUAL_MODE			1	/* Use QD located at manually defined base addr */
#define SMI_MULTI_ADDR_MODE		2	/* Use QD at base addr and use indirect access */
typedef struct
{
	GT_U32	scanMode;	/* check definition for device scan mode */
	GT_U32	baseAddr;	/* meaningful if scanMode is not SMI_AUTO_SCAN_MODE */
} GT_SCAN_MODE;


#define GT_SKIP_INIT_SETUP	0x736b6970

/*
 * Typedef: struct GT_SYS_CONFIG
 *
 * Description: System configuration Parameters struct.
 *
 * Fields:
 *	devNum		- Switch Device Number
 *  cpuPortNum  - The physical port used to connect the device to CPU.
 *                This is the port to which packets destined to CPU are
 *                forwarded.
 *  initPorts   - Whether to initialize the ports state.
 *                GT_FALSE    - leave in default state.
 *                GT_TRUE     - Initialize to Forwarding state.
 *  skipInitSetup - skip init setup, if value is GT_SKIP_INIT_SETUP
 *                  perform init setup, otherwise
 *	                Initializing port state is not affected by this variable.
 *	BSPFunctions	- Group of BSP specific functions.
 *				SMI Read/Write and Semaphore Related functions.
 */
typedef struct
{
    GT_U8         devNum;
    GT_U8         cpuPortNum;
    GT_BOOL       initPorts;
    BSP_FUNCTIONS BSPFunctions;
	GT_SCAN_MODE  mode;
    GT_U32        skipInitSetup;
}GT_SYS_CONFIG;



/*
 * Typedef: struct GT_QD_DEV
 *
 * Description: Includes Tapi layer switch configuration data.
 *
 * Fields:
 *   deviceId       - The device type identifier.
 *   revision       - The device revision number.
 *   baseRegAddr    - Switch Base Register address.
 *   numOfPorts     - Number of active ports.
 *   maxPorts       - max ports. This field is only for driver's use.
 *   cpuPortNum     - Logical port number whose physical port is connected to the CPU.
 *   maxPhyNum      - max configurable Phy address.
 *   stpMode        - current switch STP mode (0 none, 1 en, 2 dis)
 *   accessMode	    - shows how to find and access the device.
 *   phyAddr        - SMI address used to access Switch registers(only for SMI_MULTI_ADDR_MODE).
 *   validPortVec   - valid port list in vector format
 *   validPhyVec	- valid phy list in vector format
 *   devGroup	    - the device group
 *   devName	    - name of the device
 *   devStorage	    - driver internal use (hold various temp information)
 *   multiAddrSem   - Semaphore for Accessing SMI Device
 *   atuRegsSem     - Semaphore for ATU access
 *   vtuRegsSem     - Semaphore for VTU access
 *   statsRegsSem   - Semaphore for RMON counter access
 *   pirlRegsSem    - Semaphore for PIRL Resource access
 *   ptpRegsSem     - Semaphore for PTP Resource access
 *   tblRegsSem     - Semaphore for various Table Resource access,
 *                    such as Trunk Tables and Device Table
 *   phyRegsSem     - Semaphore for PHY Device access
 *   fgtReadMii     - platform specific SMI register Read function
 *   fgtWriteMii    - platform specific SMI register Write function
 *   semCreate      - function to create semapore
 *   semDelete      - function to delete the semapore
 *   semTake        - function to get a semapore
 *   semGive        - function to return semaphore
 *   appData        - application data that user may use
 */
struct _GT_QD_DEV
{
    GT_DEVICE   deviceId;
    GT_LPORT    cpuPortNum;
    GT_U8       revision;
    GT_U8		devNum;
    GT_U8		devEnabled;
    GT_U8       baseRegAddr;
    GT_U8       numOfPorts;
	GT_U8		maxPorts;
    GT_U8       maxPhyNum;
    GT_U8		stpMode;
	GT_U8		accessMode;
	GT_U8		phyAddr;
	GT_U16		validPortVec;
	GT_U16		validPhyVec;
	GT_U16		devGroup;
    GT_U32		devName;
    GT_U32		devStorage;
    GT_SEM		multiAddrSem;
    GT_SEM		atuRegsSem;
    GT_SEM		vtuRegsSem;
    GT_SEM		statsRegsSem;
    GT_SEM		pirlRegsSem;
    GT_SEM		ptpRegsSem;
    GT_SEM		tblRegsSem;
    GT_SEM		phyRegsSem;

    FGT_READ_MII 	fgtReadMii;
    FGT_WRITE_MII 	fgtWriteMii;

    FGT_SEM_CREATE	semCreate; 	/* create semaphore */
    FGT_SEM_DELETE	semDelete; 	/* delete the semaphore */
    FGT_SEM_TAKE	semTake;	/* try to get a semaphore */
    FGT_SEM_GIVE	semGive;	/* return semaphore */
    void*		appData;

};


#ifdef __cplusplus
}
#endif

#endif /* __msApi_h */

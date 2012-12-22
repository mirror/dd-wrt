#include "RTL8366RB_DRIVER/rtl8368s_types.h"
#include "rtl8366_phy.h"

int32 rtl8368s_getAsicReg(uint32 reg, uint32 *val);
int32 rtl8368s_setAsicReg(uint32 reg, uint32 value);
int32 rtl8366rb_setPort4RGMIIControl(uint32 enabled);

/* Port ability */
#define RTL8368S_PORT_ABILITY_BASE			0x0010

/* port vlan control register */
#define RTL8368S_PORT_VLAN_CTRL_BASE		0x0063

/*port linking status*/
#define RTL8368S_PORT_LINK_STATUS_BASE		0x0014
#define RTL8368S_PORT_STATUS_SPEED_OFF		0
#define RTL8368S_PORT_STATUS_SPEED_MSK		(0x3<<RTL8368S_PORT_STATUS_SPEED_OFF)
#define RTL8368S_PORT_STATUS_DUPLEX_OFF		2
#define RTL8368S_PORT_STATUS_DUPLEX_MSK		(0x1<<RTL8368S_PORT_STATUS_DUPLEX_OFF)
#define RTL8368S_PORT_STATUS_LINK_OFF		4
#define RTL8368S_PORT_STATUS_LINK_MSK		(0x1<<RTL8368S_PORT_STATUS_LINK_OFF)
#define RTL8368S_PORT_STATUS_TXPAUSE_OFF	5
#define RTL8368S_PORT_STATUS_TXPAUSE_MSK	(0x1<<RTL8368S_PORT_STATUS_TXPAUSE_OFF)
#define RTL8368S_PORT_STATUS_RXPAUSE_OFF	6
#define RTL8368S_PORT_STATUS_RXPAUSE_MSK	(0x1<<RTL8368S_PORT_STATUS_RXPAUSE_OFF)
#define RTL8368S_PORT_STATUS_AN_OFF			7
#define RTL8368S_PORT_STATUS_AN_MSK			(0x1<<RTL8368S_PORT_STATUS_AN_OFF)

/*internal control*/
#define RTL8368S_RESET_CONTROL_REG			0x0100
#define RTL8368S_RESET_QUEUE_OFF			2

#define RTL8368S_CHIP_ID_REG				0x0509

/*MAC control*/
#define RTL8368S_MAC_FORCE_CTRL_REG			0x0F11


/* PHY registers control */
#define RTL8368S_PHY_ACCESS_CTRL_REG		0x8000
#define RTL8368S_PHY_ACCESS_DATA_REG		0x8002

#define RTL8368S_PHY_CTRL_READ				1
#define RTL8368S_PHY_CTRL_WRITE				0

#define RTL8368S_PHY_REG_MASK				0x1f
#define RTL8368S_PHY_PAGE_OFFSET			5
#define RTL8368S_PHY_PAGE_MASK				(0xF<<RTL8368S_PHY_PAGE_OFFSET)
#define RTL8368S_PHY_NO_OFFSET				9
#define RTL8368S_PHY_NO_MASK				(0x1f<<RTL8368S_PHY_NO_OFFSET)

#define RTL8366RB_PHY_NO_MAX                4
#define RTL8366RB_PHY_PAGE_MAX              7
#define RTL8366RB_PHY_ADDR_MAX              31


#if 0
/* enum for port ID */
enum PORTID
{
	PORT0 =  0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT6,	
	PORT7,	
	PORT_MAX
};

/* enum for port ability speed */
enum PORTABILITYSPEED
{
	SPD_10M_H = 1,
	SPD_10M_F,
	SPD_100M_H,
	SPD_100M_F,
	SPD_1000M_F
};

/* enum for port current link speed */
enum PORTLINKSPEED
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum MACLINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum PORTLINKDUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

//#define CPU_PORT 1
//#define LED_SET 1
#define GET_INFO 1
#define MIB_COUNTER 1
#define CFG_SP1000 1
#endif

#ifdef CPU_PORT
/* cpu port control reg */
#define RTL8368S_CPU_CTRL_REG				0x0061
#define RTL8368S_CPU_PORTS_OFF				0
#define RTL8368S_CPU_PORTS_MSK				(0xff<<RTL8368S_CPU_PORTS_OFF)
#define RTL8368S_CPU_INSTAG_OFF				15
#define RTL8368S_CPU_INSTAG_MSK				(0x1<<RTL8368S_CPU_INSTAG_OFF)
#endif

#ifdef LED_SET
/* LED registers*/
#define RTL8368S_LED_BLINK_REG				0x0430
#define RTL8368S_LED_BLINKRATE_OFF			0
#define RTL8368S_LED_BLINKRATE_MSK			0x0007
#define RTL8368S_LED_INDICATED_CONF_REG		0x0431
#define RTL8368S_LED_0_1_FORCE_REG			0x0432
#define RTL8368S_LED_2_3_FORCE_REG			0x0433
#define RTL8368S_LED_0_FORCE_MASK			0x003F
#define RTL8368S_LED_1_FORCE_MASK			0x0FC0
#define RTL8368S_LED_2_FORCE_MASK			0x003F
#define RTL8368S_LED_3_FORCE_MASK			0x0FC0
#define RTL8368S_LED_1_FORCE_OFF			6
#define RTL8368S_LED_3_FORCE_OFF			6

#define RTL8368S_LED_GROUP_MAX				4

enum RTL8368S_LEDCONF{

	LEDCONF_LEDOFF=0, 		
	LEDCONF_DUPCOL,		
	LEDCONF_LINK_ACT,		
	LEDCONF_SPD1000,		
	LEDCONF_SPD100,		
	LEDCONF_SPD10,			
	LEDCONF_SPD1000ACT,	
	LEDCONF_SPD100ACT,	
	LEDCONF_SPD10ACT,		
	LEDCONF_SPD10010ACT,  
	LEDCONF_FIBER,			
	LEDCONF_FAULT,			
	LEDCONF_LINKRX,		
	LEDCONF_LINKTX,		
	LEDCONF_MASTER,		
	LEDCONF_LEDFORCE,		
};
#endif

#ifdef MIB_COUNTER
/* MIBs control */
#define RTL8368S_MIB_COUTER_BASE			0x1000
#define RTL8368S_MIB_COUTER_PORT_OFFSET		0x50
#define RTL8368S_MIB_DOT1DTPLEARNDISCARD	0x1280

#define RTL8368S_MIB_CTRL_REG				0x13F0

#define RTL8368S_MIB_CTRL_USER_MSK			0x0FFC
#define RTL8368S_MIB_CTRL_BUSY_MSK			0x1
#define RTL8368S_MIB_CTRL_BUSY_OFF			0
#define RTL8368S_MIB_CTRL_RESET_MSK			0x1
#define RTL8368S_MIB_CTRL_RESET_OFF			1
#define RTL8368S_MIB_CTRL_GLOBAL_RESET_OFF	11
#define RTL8368S_MIB_CTRL_GLOBAL_RESET_MSK	(0x1<<RTL8368S_MIB_CTRL_GLOBAL_RESET_OFF)
#define RTL8368S_MIB_CTRL_PORT_RESET_OFF	2
#define RTL8368S_MIB_CTRL_PORT_RESET_MSK	(0xff<<RTL8368S_MIB_CTRL_PORT_RESET_OFF)
#define RTL8368S_MIB_CTRL_QM_RESET_OFF		10
#define RTL8368S_MIB_CTRL_QM_RESET_MSK		(0x1<<RTL8368S_MIB_CTRL_QM_RESET_OFF)

/* Interrupt control address */
#define RTL8368S_INTERRUPT_CONTROL_REG			0x0440
#define RTL8368S_INTERRUPT_POLARITY_OFF			0
#define RTL8368S_INTERRUPT_POLARITY_MSK			0x0001
#define RTL8368S_INTERRUPT_P4_RGMII_LED_OFF		2
#define RTL8368S_INTERRUPT_P4_RGMII_LED_MSK		0x0004
#define RTL8368S_INTERRUPT_MASK_REG				0x0441
#define RTL8368S_INTERRUPT_LINKCHANGE_OFF		0
#define RTL8368S_INTERRUPT_LINKCHANGE_MSK		(0xff<<RTL8368S_INTERRUPT_LINKCHANGE_OFF)
#define RTL8368S_INTERRUPT_ACLEXCEED_OFF		8
#define RTL8368S_INTERRUPT_ACLEXCEED_MSK		(0x1<<RTL8368S_INTERRUPT_ACLEXCEED_OFF)
#define RTL8368S_INTERRUPT_STORMEXCEED_OFF		9
#define RTL8368S_INTERRUPT_STORMEXCEED_MSK		(0x1<<RTL8368S_INTERRUPT_STORMEXCEED_OFF)
#define RTL8368S_INTERRUPT_P4_FIBER_INT_OFF		12
#define RTL8368S_INTERRUPT_P4_FIBER_INT_MSK		(0x1<<RTL8368S_INTERRUPT_P4_FIBER_INT_OFF)
#define RTL8368S_INTERRUPT_P4_UTP_INT_OFF		13
#define RTL8368S_INTERRUPT_P4_UTP_INT_MSK		(0x1<<RTL8368S_INTERRUPT_P4_UTP_INT_OFF)
#define RTL8368S_INTERRUPT_STATUS_REG			0x0442
#define RTL8368S_INTERRUPT_STATUS_MSK			0xffff

#if 0
enum RTL8368S_MIBCOUNTER{

	IfInOctets = 0,
	EtherStatsOctets,
	EtherStatsUnderSizePkts,
	EtherStatsFragments,
	EtherStatsPkts64Octets,
	EtherStatsPkts65to127Octets,
	EtherStatsPkts128to255Octets,
	EtherStatsPkts256to511Octets,
	EtherStatsPkts512to1023Octets,
	EtherStatsPkts1024to1518Octets,
	EtherOversizeStats,
	EtherStatsJabbers,
	IfInUcastPkts,
	EtherStatsMulticastPkts,
	EtherStatsBroadcastPkts,
	EtherStatsDropEvents,
	Dot3StatsFCSErrors,
	Dot3StatsSymbolErrors,
	Dot3InPauseFrames,
	Dot3ControlInUnknownOpcodes,
	IfOutOctets,
	Dot3StatsSingleCollisionFrames,
	Dot3StatMultipleCollisionFrames,
	Dot3sDeferredTransmissions,
	Dot3StatsLateCollisions,
	EtherStatsCollisions,
	Dot3StatsExcessiveCollisions,
	Dot3OutPauseFrames,
	Dot1dBasePortDelayExceededDiscards,
	Dot1dTpPortInDiscards,
	IfOutUcastPkts,
	IfOutMulticastPkts,
	IfOutBroadcastPkts,
	OutOampduPkts,
	InOampduPkts,
	PktgenPkts,
	/*Device only */	
	Dot1dTpLearnEntryDiscardFlag,
	RTL8368S_MIBS_NUMBER,
	
};	
#endif
#endif

BOOL rtl8366rb_phy_is_link_alive(int phyUnit);
int rtl8366rb_phy_is_up(int unit);
int rtl8366rb_phy_is_fdx(int unit);
int rtl8366rb_phy_speed(int unit);
BOOL rtl8366rb_phy_setup(int unit);
/* 2008/11/28 BUFFALO: fix WAN portlink down issue */
int rtl8366rb_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed);

typedef struct 
{
    uint32 vid;
    uint32 mbrmsk;
    uint32 untagmsk;
    uint32 fid;
}
vlanConfig_t;

typedef struct
{
    ipaddr_t      sip;
    ipaddr_t      dip;
    ether_addr_t  mac;
    uint16        fid;
    uint16        mbr;
    uint16        block;
    uint16        spa;
    uint16        age;
    uint16        auth;
    uint16        swst;
    uint16        ipmulti;
}
l2_entry;

typedef struct
{
    enum MACLINKMODE force;
    enum PORTLINKSPEED speed;
    enum PORTLINKDUPLEXMODE duplex;
    uint32 link;
    uint32 txPause;
    uint32 rxPause;
}
macConfig_t;

int rtl_chip_type_select(void);

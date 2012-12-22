#include "RTL8366S_DRIVER/rtl8366s_types.h"
#include "rtl8366_phy.h"

/* Port ability */
#define RTL8366S_PORT_ABILITY_BASE			0x0011

/* port vlan control register */
#define RTL8366S_PORT_VLAN_CTRL_BASE		0x0058

/*port linking status*/
#define RTL8366S_PORT_LINK_STATUS_BASE		0x0060
#define RTL8366S_PORT_STATUS_SPEED_BIT		0
#define RTL8366S_PORT_STATUS_SPEED_MSK		0x0003
#define RTL8366S_PORT_STATUS_DUPLEX_BIT		2
#define RTL8366S_PORT_STATUS_DUPLEX_MSK		0x0004
#define RTL8366S_PORT_STATUS_LINK_BIT		4
#define RTL8366S_PORT_STATUS_LINK_MSK		0x0010
#define RTL8366S_PORT_STATUS_TXPAUSE_BIT	5
#define RTL8366S_PORT_STATUS_TXPAUSE_MSK	0x0020
#define RTL8366S_PORT_STATUS_RXPAUSE_BIT	6
#define RTL8366S_PORT_STATUS_RXPAUSE_MSK	0x0040
#define RTL8366S_PORT_STATUS_AN_BIT			7
#define RTL8366S_PORT_STATUS_AN_MSK			0x0080

/*internal control*/
#define RTL8366S_RESET_CONTROL_REG			0x0100
#define RTL8366S_RESET_QUEUE_BIT			2

#define RTL8366S_CHIP_ID_REG				0x0105

/*MAC control*/
#define RTL8366S_MAC_FORCE_CTRL0_REG		0x0F04
#define RTL8366S_MAC_FORCE_CTRL1_REG		0x0F05


/* PHY registers control */
#define RTL8366S_PHY_ACCESS_CTRL_REG		0x8028
#define RTL8366S_PHY_ACCESS_DATA_REG		0x8029

#define RTL8366S_PHY_CTRL_READ				1
#define RTL8366S_PHY_CTRL_WRITE				0

#define RTL8366S_PHY_REG_MASK				0x1F
#define RTL8366S_PHY_PAGE_OFFSET			5
#define RTL8366S_PHY_PAGE_MASK				(0x7<<5)
#define RTL8366S_PHY_NO_OFFSET				9
#define RTL8366S_PHY_NO_MASK				(0x1F<<9)

#define RTL8366S_PHY_NO_MAX					4
#define RTL8366S_PHY_PAGE_MAX				7
#define RTL8366S_PHY_ADDR_MAX				31


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
#define RTL8366S_CPU_CTRL_REG				0x004F
#define RTL8366S_CPU_DRP_BIT				14
#define RTL8366S_CPU_DRP_MSK				0x4000
#define RTL8366S_CPU_INSTAG_BIT				15
#define RTL8366S_CPU_INSTAG_MSK				0x8000
#endif

#ifdef LED_SET
/* LED registers*/
#define RTL8366S_LED_BLINK_REG					0x420
#define RTL8366S_LED_BLINKRATE_BIT				0
#define RTL8366S_LED_BLINKRATE_MSK				0x0007
#define RTL8366S_LED_INDICATED_CONF_REG			0x421
#define RTL8366S_LED_0_1_FORCE_REG				0x422
#define RTL8366S_LED_2_3_FORCE_REG				0x423

#define RTL8366S_LED_GROUP_MAX				4
enum RTL8366S_LEDCONF
{
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
#define RTL8366S_MIB_COUTER_BASE			0x1000
#define RTL8366S_MIB_COUTER_PORT_OFFSET		0x0040
#define RTL8366S_MIB_COUTER_2_BASE			0x1180
#define RTL8366S_MIB_COUTER2_PORT_OFFSET	0x0008
#define RTL8366S_MIB_DOT1DTPLEARNDISCARD	0x11B0

#define RTL8366S_MIB_CTRL_REG				0x11F0

#define RTL8366S_MIB_CTRL_USER_MSK			0x01FF
#define RTL8366S_MIB_CTRL_BUSY_MSK			0x0001
#define RTL8366S_MIB_CTRL_RESET_MSK			0x0002

#define RTL8366S_MIB_CTRL_GLOBAL_RESET_MSK	0x0004
#define RTL8366S_MIB_CTRL_PORT_RESET_BIT	0x0003
#define RTL8366S_MIB_CTRL_PORT_RESET_MSK	0x01FC

#if 0
enum RTL8366S_MIBCOUNTER{

	IfInOctets = 0,
	EtherStatsOctets,
	EtherStatsUnderSizePkts,
	EtherFregament,
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
	/*Device only */	
	Dot1dTpLearnEntryDiscardFlag,
	RTL8366S_MIBS_NUMBER,	
};	
#endif
#endif

#if 0
#define BOOL    unsigned char
#define TRUE    1
#define FALSE   0
#define SUCCESS 0
#define FAILED  1

typedef struct phyAbility_s
{
	uint16_t	AutoNegotiation:1;/*PHY register 0.12 setting for auto-negotiation process*/
	uint16_t	Half_10:1;		/*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
	uint16_t	Full_10:1;		/*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
	uint16_t	Half_100:1;		/*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
	uint16_t	Full_100:1;		/*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
	uint16_t	Full_1000:1;	/*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
	uint16_t	FC:1;			/*PHY register 4.10 setting for flow control capability*/
	uint16_t	AsyFC:1;		/*PHY register 4.11 setting for  asymmetric flow control capability*/
} phyAbility_t;
#endif

BOOL rtl8366sr_phy_is_link_alive(int phyUnit);
int rtl8366sr_phy_is_up(int unit);
int rtl8366sr_phy_is_fdx(int unit);
int rtl8366sr_phy_speed(int unit);
BOOL rtl8366sr_phy_setup(int unit);
/* 2008/11/28 BUFFALO: fix WAN portlink down issue */
int rtl8366sr_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed);


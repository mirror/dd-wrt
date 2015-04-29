#ifndef	_RTL_8366_PHY_H_
#define	_RTL_8366_PHY_H_

/*
 * Only the commonly implemented MII registers and its bit definition are defined here.
 */
//#define PHY_CONTROL_REG						0
//#define PHY_STATUS_REG						1
//#define PHY_AN_ADVERTISEMENT_REG			4
//#define PHY_AN_LINKPARTNER_REG				5
//#define PHY_1000_BASET_CONTROL_REG			9
//#define PHY_1000_BASET_STATUS_REG			10
#define	MII_CONTROL_REG		0
#define	MII_STATUS_REG		1
#define	MII_PHY_ID0			2
#define	MII_PHY_ID1			3
#define	MII_LOCAL_CAP		4
#define	MII_REMOTE_CAP		5
#define	MII_EXT_AUTONEG		6
#define	MII_LOCAL_NEXT_PAGE	7
#define	MII_REMOTE_NEXT_PAGE	8
#define	MII_GIGA_CONTROL	9
#define	MII_GIGA_STATUS		10
#define	MII_EXT_STATUS_REG	15

// Control register
#define	MII_CONTROL_RESET	15
#define	MII_CONTROL_LOOPBACK	14
#define	MII_CONTROL_100MBPS	13
#define	MII_CONTROL_AUTONEG	12
#define	MII_CONTROL_POWERDOWN	11
#define	MII_CONTROL_ISOLATE	10
#define	MII_CONTROL_RENEG	9
#define	MII_CONTROL_FULLDUPLEX	8
#define	MII_CONTROL_COLL_TEST	7
#define	MII_CONTROL_1000MBPS	6

// Status/Extended status register
#define	MII_STATUS_100_T4	15	// Basic status
#define	MII_STATUS_100_TX_FULL	14
#define	MII_STATUS_100_TX_HALF	13
#define	MII_STATUS_10_TX_FULL	12
#define	MII_STATUS_10_TX_HALF	11
#define	MII_STATUS_100_T2_FULL	10
#define	MII_STATUS_100_T2_HALF	9
#define	MII_STATUS_EXTENDED	8
#define	MII_STATUS_RESERVED	7
#define	MII_STATUS_NO_PREAMBLE	6
#define	MII_STATUS_AUTONEG_DONE	5
#define	MII_STATUS_REMOTE_FAULT	4
#define	MII_STATUS_AUTONEG_ABLE	3
#define	MII_STATUS_LINK_UP	2
#define	MII_STATUS_JABBER	1
#define	MII_STATUS_CAPABILITY	0
#define	MII_GIGA_CONTROL_FULL	9
#define	MII_GIGA_CONTROL_HALF	8
#define	MII_GIGA_STATUS_FULL	11
#define	MII_GIGA_STATUS_HALF	10
#define	MII_STATUS_1000_X_FULL	15	// Extendedn status
#define	MII_STATUS_1000_X_HALF	14
#define	MII_STATUS_1000_T_FULL	13
#define	MII_STATUS_1000_T_HALF	12

// Local/Remmote capability register
#define	MII_CAP_NEXT_PAGE	15
#define	MII_CAP_ACKNOWLEDGE	14	// Remote only
#define	MII_CAP_REMOTE_FAULT	13
#define	MII_CAP_RESERVED	12
#define	MII_CAP_ASYMM_PAUSE	11
#define	MII_CAP_SYMM_PAUSE	10
#define	MII_CAP_100BASE_T4	9
#define	MII_CAP_100BASE_TX_FULL	8
#define	MII_CAP_100BASE_TX	7
#define	MII_CAP_10BASE_TX_FULL	6
#define	MII_CAP_10BASE_TX	5
#define	MII_CAP_IEEE_802_3	0x0001

#define	MII_LINK_MODE_MASK	0x1f	// 100Base-T4, 100Base-TX and 10Base-TX

#define REALTEK_RTL8366_CHIP_ID0    0x001C
#define REALTEK_RTL8366_CHIP_ID1    0xC940
#define REALTEK_RTL8366_CHIP_ID1_MP 0xC960

#define REALTEK_MIN_PORT_ID 0
#define REALTEK_MAX_PORT_ID 5
#define REALTEK_MIN_PHY_ID REALTEK_MIN_PORT_ID
#define REALTEK_MAX_PHY_ID 4
#define REALTEK_CPU_PORT_ID REALTEK_MAX_PORT_ID
#define REALTEK_PHY_PORT_MASK ((1<<(REALTEK_MAX_PHY_ID+1)) - (1<<REALTEK_MIN_PHY_ID))
#define REALTEK_CPU_PORT_MASK (1<<REALTEK_CPU_PORT_ID)
#define REALTEK_ALL_PORT_MASK (REALTEK_PHY_PORT_MASK | REALTEK_CPU_PORT_MASK)

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
}
phyAbility_t;

#endif	//	_RTL_8366_PHY_H_

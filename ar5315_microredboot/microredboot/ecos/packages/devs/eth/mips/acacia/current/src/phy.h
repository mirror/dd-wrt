#ifndef _PHY_H_
#define _PHY_H_

#include "acacia_mac.h"
#include "types.h"

/* MII Management Interface Registers and bit values */

#define IDT_ETH_REG_BASE     0xB8058000
#define IDT_MIIMCFG_REG      IDT_ETH_REG_BASE + 0x0220
#define IDT_MIIMCMD_REG      IDT_ETH_REG_BASE + 0x0224
#define IDT_MIIMADDR_REG     IDT_ETH_REG_BASE + 0x0228
#define IDT_MIIMWTD_REG      IDT_ETH_REG_BASE + 0x022c
#define IDT_MIIMRDD_REG      IDT_ETH_REG_BASE + 0x0230
#define IDT_MIIMIND_REG      IDT_ETH_REG_BASE + 0x0234

#define IDT_ETHMCP_REG       IDT_ETH_REG_BASE + 0x0028
#define IDT_ETHMCP_VAL	     0

#define MIICFG_RESET_VAL     0x00008000

#define MIICMD_SET_SCAN()						\
	{								\
		*((volatile U32 *)IDT_MIIMCMD_REG) = MIIMCMD_SCN;	\
	}

#define MIICMD_RESET_SCAN()						\
	{								\
		*((volatile U32 *)IDT_MIIMCMD_REG) &= ~MIIMCMD_SCN;	\
	}

#define MIIM_RESET()							\
	{								\
		*((volatile U32 *)IDT_MIIMCFG_REG) = MIICFG_RESET_VAL;	\
	}

#define MIIM_CLEAR_RESET()						\
	{								\
		*((volatile U32 *)IDT_MIIMCFG_REG) &= ~MIICFG_RESET_VAL;\
	}

#define PORT0_NAME "WAN"
#define PORT1_NAME "LAN"

/* PHY Device used on the board : LXT972 
 * Register Definitions 
 */

#define PHY0_ADDR              0x1
#define PHY1_ADDR              0x2

#define PHY_10MBS               0
#define PHY_100MBS              1
#define PHY_AUTO_SPEED          2

#define PHY_HALF_DPX            0
#define PHY_FULL_DPX            1
#define PHY_AUTO_DPX            2

#define PHY_CONTROL_REG         0x0

#define PHY_RESET_VAL           0x8000
#define PHY_LOOP_EN             0x4000
#define PHY_SPEED_SEL           0x2000
#define PHY_AUTONEG_EN          0x1000
#define PHY_PWR_DOWN            0x0800
#define PHY_ISOLATE             0x0400
#define PHY_RESTART_AN          0x0200
#define PHY_DPLX_MODE           0x0100
#define PHY_COL_TEST            0x0080
#define PHY_SPD_SELECT          0x0040

#define PHY_NEGOTIATE           (PHY_AUTONEG_EN | PHY_RESTART_AN)

#define PHY_STATUS1_REG         0x1

#define PHY_10BASE_T4           0x8000
#define PHY_100BASEX_FULL       0x4000
#define PHY_100BASEX_HALF       0x2000
#define PHY_10BASE_FULL         0x1000
#define PHY_10BASE_HALF         0x0800
#define PHY_10BASET2_HALF       0x0400
#define PHY_10BASET2_FULL       0x0200
#define PHY_EXT_STATUS          0x0100
#define PHY_MF_PREAM_SUPPRESS   0x0040
#define PHY_AUTONEG_DONE        0x0020
#define PHY_REMOTE_FAULT        0x0010
#define PHY_AUTONEG_ABLE        0x0008
#define PHY_LINK_STATUS         0x0004
#define PHY_JABBER_DETECT       0x0002
#define PHY_EXTEND_ABLE         0x0001

#define PHY_PHY_ID_1_REG        0x2
#define PHY_PHY_ID_2_REG        0x3
#define PHY_AUTONEG_ADV_REG     0x4

#define PHY_MULTIPAGE_ABLE      0x8000
#define PHY_ASYM_PAUSE          0x0800
#define PHY_PAUSE_EN            0x0400
#define PHY_100BASETX_FULL      0x0100 
#define PHY_100BASETX_ABLE      0x0080
#define PHY_10BASET_FULL        0x0040
#define PHY_10BASET_ABLE        0x0020
#define PHY_IEEE802_3_VAL       0x0001

#define PHY_AUTO_NEG_DEFAULT			\
	(					\
	 PHY_100BASETX_FULL |			\
	 PHY_100BASETX_ABLE |			\
	 PHY_10BASET_FULL   |			\
	 PHY_10BASET_ABLE   |			\
	 PHY_IEEE802_3_VAL			\
	 ) 

#define PHY_AUTONEG_LNK_ABILITY_REG  0x5
#define PHY_AUTONEG_EXPANSION_REG    0x6

#define PHY_BASE_PAGE             0x0020 
#define PHY_PAR_DETECT_FAULT      0x0010
#define PHY_LNK_PARTNER           0x0008
#define PHY_NXT_PAG_ABLE          0x0004
#define PHY_PAGE_RCVD             0x0002
#define PHY_LNK_PARTNER_ANABLE    0x0001

#define PHY_AUTONEG_LNK_PAGE_XMT_REG 0x7
#define PHY_AUTONEG_LNK_NXT_PAGE_REG 0x8

#define PHY_NEXT_PAGE         0x8000
#define PHY_MESSAGE_PAGE      0x2000
#define PHY_ACK2              0x1000
#define PHY_TOGGLE            0x0800 

#define PHY_PORT_CONFIG_REG   0x10

#define PHY_FORCE_LNK_PASS    0x4000
#define PHY_XMT_DISABLE       0x2000
#define PHY_BYPASS_SCRAMBLER  0x1000
#define PHY_JABBER            0x0400 
#define PHY_SQE               0x0200
#define PHY_TP_LOOPBACK       0x0100
#define PHY_CRS_SELECT        0x0080
#define PHY_PRE_EN            0x0020
#define PHY_ALT_NXT_PAGE      0x0002

#define PHY_STATUS2_REG       0x11
#define PHY_STAT2_100         0x4000
#define PHY_STAT2_LINK        0x0400
#define PHY_STAT2_FULL_DPLX   0x0200
#define PHY_STAT2_AN_ABLE     0x0100
#define PHY_STAT2_AN_CMPLT    0x0080

#define PHY_INTRPT_ENABLE_REG 0x12

#define PHY_INTRPT_STATUS_REG 0x13

#define PHY_LED_CONFIG_REG    0x14
#define PHY_LED_VAL           0x1232
#define PHY_LED_VAL2          0x1242

#define PHY_XMT_CONTROL_REG   0x1E

#define PHY_SPEED_10          10000000
#define PHY_SPEED_100         100000000

#define PHY_RXCOUNT_REG       0x12

/*MII management configuration */

#define MIIMCFG_RSV(i)           ((i&0x3)<<2)
#define MIIMCFG_R                (1<<15)

/*MII management command */

#define MIIMCMD_RD               (1<<0)
#define MIIMCMD_SCN              (1<<1)

/*MII management address */

#define MIIMADDR_REGADDR(i)      ((i&0x1f)<<0)
#define MIIMADDR_PHYADDR(i)      ((i&0x1f)<<8)

/*MII management indicators */

#define MIIMIND_BSY              (1<<0)
#define MIIMIND_SCN              (1<<1)
#define MIIMIND_NV               (1<<2)

void idt32438PhyRegReadAll( int phyAddr); 
void idt32438PhyInit(void);
int idt32438PhyAutoNegotiationComplete(acacia_MAC_t *MACInfo);
int idt32438PhyRegPoll(U32 v, U32 timeout);
int idt32438PhyRegRead 
(
int         phyAddr,  /* phy address */
int         regAddr,  /* reg address */
int*       retVal     /* return value */
);

int idt32438PhyRegWrite
(
int     phyAddr,      /* phy address */
int     regAddr,      /* reg address */
int     writeData     /* data to be written */
);

#endif //  _PHY_H_

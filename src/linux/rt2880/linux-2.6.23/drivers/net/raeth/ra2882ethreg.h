#ifndef RA2882ETHREG_H
#define RA2882ETHREG_H

#include <linux/mii.h>		// for struct mii_if_info in ra2882ethreg.h
#include <linux/version.h>	/* check linux version for 2.4 and 2.6 compatibility */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif
#include "raether.h"


#define MAX_PACKET_SIZE	1514
#define	MIN_PACKET_SIZE 60

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#define BIT(x)	((1 << x))
#define RT2880_BIT(x)	((1 << x))
#define ETHER_ADDR_LEN  6

/*  Phy Vender ID list */

#define EV_MARVELL_PHY_ID0 0x0141  
#define EV_MARVELL_PHY_ID1 0x0CC2  
#define EV_MARVELL_PHY_SENAO_ID1 0x0E11  

/*
     FE_INT_STATUS
*/
#define RT2880_CNT_PPE_AF       RT2880_BIT(31)     
#define RT2880_CNT_GDM_AF       RT2880_BIT(29)
#define RT2880_PSE_P2_FC	RT2880_BIT(26)
#define RT2880_PSE_BUF_DROP     RT2880_BIT(24)
#define RT2880_GDM_OTHER_DROP	RT2880_BIT(23)
#define RT2880_PSE_P1_FC        RT2880_BIT(22)
#define RT2880_PSE_P0_FC        RT2880_BIT(21)
#define RT2880_PSE_FQ_EMPTY     RT2880_BIT(20)
#define RT2880_GE1_STA_CHG      RT2880_BIT(18)
#define RT2880_TX_COHERENT      RT2880_BIT(17)
#define RT2880_RX_COHERENT      RT2880_BIT(16)

#define RT2880_TX_DONE_INT3     RT2880_BIT(11)
#define RT2880_TX_DONE_INT2     RT2880_BIT(10)
#define RT2880_TX_DONE_INT1     RT2880_BIT(9)
#define RT2880_TX_DONE_INT0     RT2880_BIT(8)
#define RT2880_RX_DONE_INT0     RT2880_BIT(2)
#define RT2880_TX_DLY_INT       RT2880_BIT(1)
#define RT2880_RX_DLY_INT       RT2880_BIT(0)

#define RX_BUF_ALLOC_SIZE	2000
#define FASTPATH_HEADROOM   	64

#define ETHER_BUFFER_ALIGN	32		///// Align on a cache line

#define ETHER_ALIGNED_RX_SKB_ADDR(addr) \
        ((((unsigned long)(addr) + ETHER_BUFFER_ALIGN - 1) & \
        ~(ETHER_BUFFER_ALIGN - 1)) - (unsigned long)(addr))

#ifdef CONFIG_PSEUDO_SUPPORT
typedef struct _PSEUDO_ADAPTER {
    struct net_device *RaethDev;
    struct net_device *PseudoDev;
} PSEUDO_ADAPTER, PPSEUDO_ADAPTER;

#define MAX_PSEUDO_ENTRY               1
#endif


/* Register Map, Ref to Frame Engine Design Spec, chap2 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define RA2882ETH_BASE  RALINK_FRAME_ENGINE_BASE
#else
#if defined(CONFIG_RALINK_RT2880_SHUTTLE)
#define RA2882ETH_BASE  (0xA0310000)
#elif defined(CONFIG_RALINK_RT2880_MP) 
#define RA2882ETH_BASE  (0xA0400000)
#else
#error Please Choice Chip Version (Shuttle/MP)
#endif
#endif

/* Register Categories Definition */
#define RAFRAMEENGINE_OFFSET	0x0000
#define RAGDMA_OFFSET		0x0020
#define RAPSE_OFFSET		0x0040
#define RAGDMA2_OFFSET		0x0060
#define RACDMA_OFFSET		0x0080
#define RAPDMA_OFFSET		0x0100
#define RAPPE_OFFSET		0x0200
#define RACMTABLE_OFFSET	0x0400
#define RAPOLICYTABLE_OFFSET	0x1000


/* Register Map Detail */

/* 1. Frame Engine Global Registers */
#define MDIO_ACCESS		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x00)
#define MDIO_CFG 		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x04)
#define FE_GLO_CFG		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x08)
#define FE_RST_GL		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x0C)
#define FE_INT_STATUS		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x10)
#define FE_INT_ENABLE		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x14)
#define MDIO_CFG2		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x18) //Original:FC_DROP_STA
#define FOC_TS_T		(RA2882ETH_BASE+RAFRAMEENGINE_OFFSET+0x1C)


/* 2. GDMA Registers */
#define	GDMA1_FWD_CFG		(RA2882ETH_BASE+RAGDMA_OFFSET+0x00)
#define GDMA1_SCH_CFG		(RA2882ETH_BASE+RAGDMA_OFFSET+0x04)
#define GDMA1_SHPR_CFG		(RA2882ETH_BASE+RAGDMA_OFFSET+0x08)
#define GDMA1_MAC_ADRL		(RA2882ETH_BASE+RAGDMA_OFFSET+0x0C)
#define GDMA1_MAC_ADRH		(RA2882ETH_BASE+RAGDMA_OFFSET+0x10)

#define	GDMA2_FWD_CFG		(RA2882ETH_BASE+RAGDMA2_OFFSET+0x00)
#define GDMA2_SCH_CFG		(RA2882ETH_BASE+RAGDMA2_OFFSET+0x04)
#define GDMA2_SHPR_CFG		(RA2882ETH_BASE+RAGDMA2_OFFSET+0x08)
#define GDMA2_MAC_ADRL		(RA2882ETH_BASE+RAGDMA2_OFFSET+0x0C)
#define GDMA2_MAC_ADRH		(RA2882ETH_BASE+RAGDMA2_OFFSET+0x10)

/* 3. PSE */
#define PSE_FQ_CFG		(RA2882ETH_BASE+RAPSE_OFFSET+0x00)
#define CDMA_FC_CFG		(RA2882ETH_BASE+RAPSE_OFFSET+0x04)
#define GDMA1_FC_CFG		(RA2882ETH_BASE+RAPSE_OFFSET+0x08)
#define GDMA2_FC_CFG		(RA2882ETH_BASE+RAPSE_OFFSET+0x0C)

#define PDMA_FC_CFG		(RA2882ETH_BASE+0x1f0)

/* 4. CDMA */
#define CDMA_CSG_CFG		(RA2882ETH_BASE+RACDMA_OFFSET+0x00)
#define CDMA_SCH_CFG		(RA2882ETH_BASE+RACDMA_OFFSET+0x04)
/* skip ppoe sid and vlan id definition */


/* 5. PDMA */
#define PDMA_GLO_CFG		(RA2882ETH_BASE+RAPDMA_OFFSET+0x00)
#define PDMA_RST_CFG		(RA2882ETH_BASE+RAPDMA_OFFSET+0x04)
#define PDMA_SCH_CFG		(RA2882ETH_BASE+RAPDMA_OFFSET+0x08)

#define DLY_INT_CFG		(RA2882ETH_BASE+RAPDMA_OFFSET+0x0C)

#define TX_BASE_PTR0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x10)
#define TX_MAX_CNT0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x14)
#define TX_CTX_IDX0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x18)
#define TX_DTX_IDX0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x1C)

#define TX_BASE_PTR1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x20)
#define TX_MAX_CNT1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x24)
#define TX_CTX_IDX1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x28)
#define TX_DTX_IDX1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x2C)

#define TX_BASE_PTR2		(RA2882ETH_BASE+RAPDMA_OFFSET+0x40)
#define TX_MAX_CNT2		(RA2882ETH_BASE+RAPDMA_OFFSET+0x44)
#define TX_CTX_IDX2		(RA2882ETH_BASE+RAPDMA_OFFSET+0x48)
#define TX_DTX_IDX2		(RA2882ETH_BASE+RAPDMA_OFFSET+0x4C)

#define TX_BASE_PTR3		(RA2882ETH_BASE+RAPDMA_OFFSET+0x50)
#define TX_MAX_CNT3		(RA2882ETH_BASE+RAPDMA_OFFSET+0x54)
#define TX_CTX_IDX3		(RA2882ETH_BASE+RAPDMA_OFFSET+0x58)
#define TX_DTX_IDX3		(RA2882ETH_BASE+RAPDMA_OFFSET+0x5C)

#define RX_BASE_PTR0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x30)
#define RX_MAX_CNT0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x34)
#define RX_CALC_IDX0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x38)
#define RX_DRX_IDX0		(RA2882ETH_BASE+RAPDMA_OFFSET+0x3C)

#define RX_BASE_PTR1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x40)
#define RX_MAX_CNT1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x44)
#define RX_CALC_IDX1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x48)
#define RX_DRX_IDX1		(RA2882ETH_BASE+RAPDMA_OFFSET+0x4C)

#define DELAY_INT_INIT_NAPI	0x840f8514
#define DELAY_INT_INIT		0x84048404
#define FE_INT_DLY_INIT		(RT2880_TX_DLY_INT | RT2880_RX_DLY_INT)
#define FE_INT_ALL		(RT2880_PSE_P2_FC | RT2880_PSE_BUF_DROP | \
                                 RT2880_GDM_OTHER_DROP | RT2880_PSE_P1_FC | \
                                 RT2880_PSE_P0_FC | RT2880_PSE_FQ_EMPTY |   \
                                 RT2880_TX_COHERENT | RT2880_RX_COHERENT |  \
				 RT2880_TX_DONE_INT3 | RT2880_TX_DONE_INT2 | \
				 RT2880_TX_DONE_INT1 | RT2880_TX_DONE_INT0 | \
                                 RT2880_RX_DONE_INT0 | RT2880_GE1_STA_CHG )


/* 6. PPE, 168 bytes */
#define PPE_GLO_CFG		(RA2882ETH_BASE+RAPPE_OFFSET+0x00)
#define PPE_STD_GA_H		(RA2882ETH_BASE+RAPPE_OFFSET+0x04)
#define PPE_STD_GA_L		(RA2882ETH_BASE+RAPPE_OFFSET+0x08)
#define PPE_ETD_GA_H		(RA2882ETH_BASE+RAPPE_OFFSET+0x0C)
#define PPE_ETD_GA_L		(RA2882ETH_BASE+RAPPE_OFFSET+0x10) /* orginal name EXT */
#define PPE_FLOW_SET		(RA2882ETH_BASE+RAPPE_OFFSET+0x14)
#define PPE_PRE_ACL		(RA2882ETH_BASE+RAPPE_OFFSET+0x18) /* Pre-ACL Table */
#define PPE_PRE_MTR		(RA2882ETH_BASE+RAPPE_OFFSET+0x1C) /* Pre-Meter Table */
#define PRE_AC			(RA2882ETH_BASE+RAPPE_OFFSET+0x20) /* Pre-Account Table */
#define PRE_POST_MTR		(RA2882ETH_BASE+RAPPE_OFFSET+0x24) /* Post-Meter Table */
#define PRE_POST_AC		(RA2882ETH_BASE+RAPPE_OFFSET+0x28) /* Post-Account Table */
#define PPE_POL_CFG		(RA2882ETH_BASE+RAPPE_OFFSET+0x2C) /* Policy engine configuration */

#define PPE_FOE_CFG		(RA2882ETH_BASE+RAPPE_OFFSET+0x30) /* Flow offload engine configuration */
#define PPE_FOE_BASE		(RA2882ETH_BASE+RAPPE_OFFSET+0x34) /* Flow offload engine talbe base address */
#define PPE_FOE_USE		(RA2882ETH_BASE+RAPPE_OFFSET+0x38) /* Flow offload engine talbe used */
#define PPE_FOE_BNDR		(RA2882ETH_BASE+RAPPE_OFFSET+0x3C) /* Flow offload engine bind rate */
#define PPE_FOE_LMT1		(RA2882ETH_BASE+RAPPE_OFFSET+0x40) /* Flow offload engine build entry limit p1 */
#define PPE_FOE_LMT2		(RA2882ETH_BASE+RAPPE_OFFSET+0x44) /* Flow offload engine build entry limit p2 */
#define PPE_FOE_KA		(RA2882ETH_BASE+RAPPE_OFFSET+0x48) /* Flow offload engine keep alive config */
#define PPE_FOE_UNB_AGE		(RA2882ETH_BASE+RAPPE_OFFSET+0x4C) /* Flow offload engine unbind entry age out */
#define PPE_FOE_BND_AGE1	(RA2882ETH_BASE+RAPPE_OFFSET+0x50) /* Flow offload engine stamp p1 */
#define PPE_FOE_BND_AGE2	(RA2882ETH_BASE+RAPPE_OFFSET+0x54) /* Flow offload engine stamp p2 */

#define CPU_PORT_CFG		(RA2882ETH_BASE+RAPPE_OFFSET+0x58) /* CPU port configuration */
#define GE_PORT_CFG		(RA2882ETH_BASE+RAPPE_OFFSET+0x5C) /* Giga ehternet port config */

#define DSCP0_7_MAP_UP		(RA2882ETH_BASE+RAPPE_OFFSET+0x60) /* DSCP0~7 mapping to user priority */
#define DSCP8_15_MAP_UP		(RA2882ETH_BASE+RAPPE_OFFSET+0x64) /* ... mapping to user priority */
#define DSCP16_23_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x68) /* ... mapping to user priority */
#define DSCP24_31_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x6C) /* ... mapping to user priority */
#define DSCP32_39_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x70) /* ... mapping to user priority */
#define DSCP40_47_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x74) /* ... mapping to user priority */
#define DSCP48_55_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x78) /* ... mapping to user priority */
#define DSCP56_63_MAP_UP	(RA2882ETH_BASE+RAPPE_OFFSET+0x7C) /* ... mapping to user priority */
#define AUTO_UP_CFG1		(RA2882ETH_BASE+RAPPE_OFFSET+0x80) /* Auto user priority config p1 */
#define AUTO_UP_CFG2		(RA2882ETH_BAS:+RAPPE_OFFSET+0x84) /* Auto user priority config p2 */
#define UP_RES			(RA2882ETH_BASE+RAPPE_OFFSET+0x88) /* User priority resolution */
#define UP_MAP_VPRI		(RA2882ETH_BASE+RAPPE_OFFSET+0x8C) /* UP mapping to VLAN priority tag */
#define UP0_3_MAP_IDSCP		(RA2882ETH_BASE+RAPPE_OFFSET+0x90) /* UP 0~3 mapping to in-profile DSCP val */
#define UP4_7_MAP_IDSCP		(RA2882ETH_BASE+RAPPE_OFFSET+0x94) /* ... mapping to in-profile DSCP val */
#define UP0_3_MAP_ODSCP		(RA2882ETH_BASE+RAPPE_OFFSET+0x98) /* ... mapping to out-profile DSCP val */
#define UP4_7_MAP_ODSCP		(RA2882ETH_BASE+RAPPE_OFFSET+0x9C) /* ... mapping to out-profile DSCP val */
#define UP_MAP_AC		(RA2882ETH_BASE+RAPPE_OFFSET+0xA0) /* UP mapping to Access category */


/* 7. Counter and Meter Table */

#define PPE_AC_BCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x000) /* PPE Accounting Group 0 Byte Cnt */
#define PPE_AC_PCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x004) /* PPE Accounting Group 0 Packet Cnt */
/* 0 ~ 63 */

#define PPE_MTR_CNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x200) /* 0 ~ 63 */
/* skip... */
#define PPE_MTR_CNT63		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x2FC)

#define GDMA_TX_GBCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x300) /* Transmit good byte cnt for GEport */
#define GDMA_TX_GPCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x304) /* Transmit good pkt cnt for GEport */
#define GDMA_TX_SKIPCNT0	(RA2882ETH_BASE+RACMTABLE_OFFSET+0x308) /* Transmit skip cnt for GEport */
#define GDMA_TX_COLCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x30C) /* Transmit collision cnt for GEport */

/* update these address mapping to fit data sheet v0.26, by bobtseng, 2007.6.14 */
#define GDMA_RX_GBCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x320)
#define GDMA_RX_GPCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x324)
#define GDMA_RX_OERCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x328)
#define GDMA_RX_FERCNT0 	(RA2882ETH_BASE+RACMTABLE_OFFSET+0x32C)
#define GDMA_RX_SERCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x330)
#define GDMA_RX_LERCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x334)
#define GDMA_RX_CERCNT0		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x338)
#define GDMA_RX_FCCNT1		(RA2882ETH_BASE+RACMTABLE_OFFSET+0x33C)

/* 8. PPE Policy Table */
#define PT_Rule0_L		(RA2882ETH_BASE+RAPOLICYTABLE_OFFSET+0x000) /* Policy table rule bit 31:0 for rule xxx */
#define PT_Rule0_H		(RA2882ETH_BASE+RAPOLICYTABLE_OFFSET+0x004) /* Policy table rule bit 63:32 for rule xxx */
/* ... */
#define PT_Rule511_L		(RA2882ETH_BASE+RAPOLICYTABLE_OFFSET+0xFF8)
#define PT_Rule511_H		(RA2882ETH_BASE+RAPOLICYTABLE_OFFSET+0xFFC)

// PHYS_TO_K1
#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)

typedef unsigned int RA2880_REG;

#define sysRegRead(phys)        \
        (*(volatile RA2880_REG *)PHYS_TO_K1(phys))

#define sysRegWrite(phys, val)  \
        ((*(volatile RA2880_REG *)PHYS_TO_K1(phys)) = (val))

#define u_long	unsigned long
#define u32	unsigned int
#define u16	unsigned short

#define RT2880_BIT(x)              ((1 << x))


/* ====================================== */
#define RT2880_GDM1_DISPAD       RT2880_BIT(18)
#define RT2880_GDM1_DISCRC       RT2880_BIT(17)

//GDMA1 uni-cast frames destination port
#define RT2880_GDM1_ICS_EN   	   (0x1 << 22)
#define RT2880_GDM1_TCS_EN   	   (0x1 << 21)
#define RT2880_GDM1_UCS_EN   	   (0x1 << 20)
#define RT2880_GDM1_JMB_EN   	   (0x1 << 19)
#define RT2880_GDM1_STRPCRC   	   (0x1 << 16)
#define RT2880_GDM1_UFRC_P_CPU     (0 << 12)
#define RT2880_GDM1_UFRC_P_GDMA1   (1 << 12)
#define RT2880_GDM1_UFRC_P_PPE     (6 << 12)

//GDMA1 broad-cast MAC address frames
#define RT2880_GDM1_BFRC_P_CPU     (0 << 8)
#define RT2880_GDM1_BFRC_P_GDMA1   (1 << 8)
#define RT2880_GDM1_BFRC_P_PPE     (6 << 8)

//GDMA1 multi-cast MAC address frames
#define RT2880_GDM1_MFRC_P_CPU     (0 << 4)
#define RT2880_GDM1_MFRC_P_GDMA1   (1 << 4)
#define RT2880_GDM1_MFRC_P_PPE     (6 << 4)

//GDMA1 other MAC address frames destination port
#define RT2880_GDM1_OFRC_P_CPU     (0 << 0)
#define RT2880_GDM1_OFRC_P_GDMA1   (1 << 0)
#define RT2880_GDM1_OFRC_P_PPE     (6 << 0)

#define RT2880_ICS_GEN_EN          (1 << 2)
#define RT2880_UCS_GEN_EN          (1 << 1)
#define RT2880_TCS_GEN_EN          (1 << 0)

// MDIO_CFG	bit
#define RT2880_MDIO_CFG_GP1_FC_TX	(1 << 11)
#define RT2880_MDIO_CFG_GP1_FC_RX	(1 << 10)

/* ====================================== */
/* ====================================== */
#define RT2880_GP1_LNK_DWN     RT2880_BIT(9) 
#define RT2880_GP1_AN_FAIL     RT2880_BIT(8) 
/* ====================================== */
/* ====================================== */
#define RT2880_PSE_RESET       RT2880_BIT(0)
/* ====================================== */
#define RT2880_PST_DRX_IDX0       RT2880_BIT(16)
#define RT2880_PST_DTX_IDX3       RT2880_BIT(3)
#define RT2880_PST_DTX_IDX2       RT2880_BIT(2)
#define RT2880_PST_DTX_IDX1       RT2880_BIT(1)
#define RT2880_PST_DTX_IDX0       RT2880_BIT(0)

#define RT2880_TX_WB_DDONE       RT2880_BIT(6)
#define RT2880_RX_DMA_BUSY       RT2880_BIT(3)
#define RT2880_TX_DMA_BUSY       RT2880_BIT(1)
#define RT2880_RX_DMA_EN         RT2880_BIT(2)
#define RT2880_TX_DMA_EN         RT2880_BIT(0)

#define PDMA_BT_SIZE_4DWORDS     (0<<4)
#define PDMA_BT_SIZE_8DWORDS     (1<<4)
#define PDMA_BT_SIZE_16DWORDS    (2<<4)

/* Register bits.
 */

#define MACCFG_RXEN		(1<<2)
#define MACCFG_TXEN		(1<<3)
#define MACCFG_PROMISC		(1<<18)
#define MACCFG_RXMCAST		(1<<19)
#define MACCFG_FDUPLEX		(1<<20)
#define MACCFG_PORTSEL		(1<<27)
#define MACCFG_HBEATDIS		(1<<28)


#define DMACTL_SR		(1<<1)	/* Start/Stop Receive */
#define DMACTL_ST		(1<<13)	/* Start/Stop Transmission Command */

#define DMACFG_SWR		(1<<0)	/* Software Reset */
#define DMACFG_BURST32		(32<<8)

#define DMASTAT_TS		0x00700000	/* Transmit Process State */
#define DMASTAT_RS		0x000e0000	/* Receive Process State */

#define MACCFG_INIT		0 //(MACCFG_FDUPLEX) // | MACCFG_PORTSEL)



/* Descriptor bits.
 */
#define R_OWN		0x80000000	/* Own Bit */
#define RD_RER		0x02000000	/* Receive End Of Ring */
#define RD_LS		0x00000100	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
#define RD_CHAIN	0x01000000	/* Chained */

/* Word 0 */
#define T_OWN		0x80000000	/* Own Bit */
#define TD_ES		0x00008000	/* Error Summary */

/* Word 1 */
#define TD_LS		0x40000000	/* Last Segment */
#define TD_FS		0x20000000	/* First Segment */
#define TD_TER		0x08000000	/* Transmit End Of Ring */
#define TD_CHAIN	0x01000000	/* Chained */


#define TD_SET		0x08000000	/* Setup Packet */


#define POLL_DEMAND 1

#define RSTCTL	(0x34)
#define RSTCTL_RSTENET1	(1<<19)
#define RSTCTL_RSTENET2	(1<<20)

#define INIT_VALUE_OF_RT2883_PSE_FQ_CFG		0xff908000
#define INIT_VALUE_OF_RT2880_PSE_FQFC_CFG	0x80504000
#define INIT_VALUE_OF_ICPLUS_PHY_INIT_VALUE	0x1001BC01

// Define Whole FE Reset Register
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define FE_RESET        (RALINK_SYSCTL_BASE + 0x34)
#define FE_RESET_BIT    RALINK_FE_RST
#else
#define FE_RESET        0xA0300034
#define FE_RESET_BIT    BIT(18)
#endif

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------
typedef struct _PDMA_RXD_INFO1_  PDMA_RXD_INFO1_T;

struct _PDMA_RXD_INFO1_
{
    unsigned int    PDP0;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO2_    PDMA_RXD_INFO2_T;

struct _PDMA_RXD_INFO2_
{
    unsigned int    PLEN1                 : 14;
    unsigned int    LS1                   : 1;
    unsigned int    UN_USED               : 1;
    unsigned int    PLEN0                 : 14;
    unsigned int    LS0                   : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO3_  PDMA_RXD_INFO3_T;

struct _PDMA_RXD_INFO3_
{
    unsigned int    UN_USE1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO4_    PDMA_RXD_INFO4_T;

struct _PDMA_RXD_INFO4_
{

    unsigned int    FOE_Entry           : 14;
    unsigned int    FVLD                : 1;
    unsigned int    UN_USE1             : 1;
    unsigned int    AI                  : 8;
    unsigned int    SP                  : 3;
    unsigned int    AIS                 : 1;
    unsigned int    L4F                 : 1;
    unsigned int    IPF                  : 1;
    unsigned int    L4FVLD_bit           : 1;
    unsigned int    IPFVLD_bit           : 1;
};


struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
};

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO1_  PDMA_TXD_INFO1_T;

struct _PDMA_TXD_INFO1_
{
    unsigned int    SDP0;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO2_    PDMA_TXD_INFO2_T;

struct _PDMA_TXD_INFO2_
{
    unsigned int    SDL1                  : 14;
    unsigned int    LS1_bit               : 1;
    unsigned int    BURST_bit             : 1;
    unsigned int    SDL0                  : 14;
    unsigned int    LS0_bit               : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO3_  PDMA_TXD_INFO3_T;

struct _PDMA_TXD_INFO3_
{
    unsigned int    SDP1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO4_    PDMA_TXD_INFO4_T;

struct _PDMA_TXD_INFO4_
{

    unsigned int    VIDX                : 4;
    unsigned int    VPRI                : 3;
    unsigned int    INSV                : 1;
    unsigned int    SIDX                : 4;
    unsigned int    INSP                : 1;
    unsigned int    RXIF            	: 1; /* RT2880E used by hw_nat to identity RX interface */
    unsigned int    UN_USE3             : 2;
    unsigned int    QN                  : 3;
    unsigned int    UN_USE2             : 5;
    unsigned int    PN                  : 3;
    unsigned int    UN_USE1             : 2;
    unsigned int    TCO                 : 1;
    unsigned int    UCO			: 1;
    unsigned int    ICO		        : 1;
};


struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#define PHY_Enable_Auto_Nego		0x1000
#define PHY_Restart_Auto_Nego		0x0200

/* PHY_STAT_REG = 1; */
#define PHY_Auto_Neco_Comp	0x0020
#define PHY_Link_Status		0x0004

/* PHY_AUTO_NEGO_REG = 4; */
#define PHY_Cap_10_Half  0x0020
#define PHY_Cap_10_Full  0x0040
#define	PHY_Cap_100_Half 0x0080
#define	PHY_Cap_100_Full 0x0100

/* proc definition */

#define CDMA_OQ_STA	(RA2882ETH_BASE+RAPSE_OFFSET+0x4c)
#define GDMA1_OQ_STA	(RA2882ETH_BASE+RAPSE_OFFSET+0x50)
#define PPE_OQ_STA	(RA2882ETH_BASE+RAPSE_OFFSET+0x54)
#define PSE_IQ_STA	(RA2882ETH_BASE+RAPSE_OFFSET+0x58)

#define PROCREG_CONTROL_FILE      "/var/run/procreg_control"

#define PROCREG_DIR		"rt2880"
#define PROCREG_GMAC		"gmac"
#define PROCREG_CP0		"cp0"
#define PROCREG_RAQOS		"qos"
#define PROCREG_READ_VAL	"regread_value"
#define PROCREG_WRITE_VAL	"regwrite_value"
#define PROCREG_ADDR	  	"reg_addr"
#define PROCREG_CTL		"procreg_control"

struct rt2880_reg_op_data {
  char	name[64];
  unsigned int reg_addr;
  unsigned int op;
  unsigned int reg_value;
};        

typedef struct MACInfo_s
{
        int ivec;       /* interrupt vector */
        unsigned char   macaddr[6];
} MAC_INFO;

typedef struct end_device
{
    unsigned int        ppeEbl;
    int                 enetUnit;       /* enet unit number */
    int                 isLAN;          /* 0-->WAN; 1-->LAN */
    int                 isRequestedUp;  /* desired state of UP & RUNNING */

    unsigned int        tx_cpu_owner_idx0;
    unsigned int        rx_cpu_owner_idx0;
    unsigned int        fe_int_status;
#if defined (CONFIG_RAETH_QOS)
    unsigned int        tx0_full, tx1_full, tx2_full, tx3_full, tx_full;
    unsigned int	phy_tx_ring0, phy_tx_ring1, phy_tx_ring2, phy_tx_ring3;
#else
    unsigned int        tx_full;
    unsigned int	phy_rx_ring, phy_tx_ring0;
#endif

    struct              tasklet_struct     rx_tasklet;
    struct              tasklet_struct     tx_tasklet;

#if defined(CONFIG_RAETH_QOS)
    struct		sk_buff *	   skb_free[NUM_TX_RINGS][NUM_TX_DESC];
    unsigned int	free_idx[NUM_TX_RINGS];
#else
    struct		sk_buff*	   skb_free[NUM_TX_DESC];
    unsigned int	free_idx;
#endif

    struct              MACInfo_s          *MACInfo;
    struct              net_device_stats stat;  /* The new statistics table. */
    spinlock_t          page_lock;              /* Page register locks */
    struct PDMA_txdesc *tx_ring0, *tx_ring1, *tx_ring2, *tx_ring3;
#ifdef CONFIG_RAETH_NAPI
    atomic_t irq_sem;
#endif
#ifdef CONFIG_PSEUDO_SUPPORT
    struct net_device *PseudoDev;
    unsigned int isPseudo;
#endif
#if defined (CONFIG_ETHTOOL) && ( defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_RT_3052_ESW) )
	struct mii_if_info	mii_info;
#endif
} END_DEVICE, *pEND_DEVICE;

#define RAETH_VERSION	"v2.00"

#endif

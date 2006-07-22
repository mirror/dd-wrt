/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software is copyrighted by and is the sole property of
 * VIA Networking Technologies, Inc. This software may only be used
 * in accordance with the corresponding license agreement. Any unauthorized
 * use, duplication, transmission, distribution, or disclosure of this
 * software is expressly forbidden.
 *
 * This software is provided by VIA Networking Technologies, Inc. "as is"
 * and any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose
 * are disclaimed. In no event shall VIA Networking Technologies, Inc.
 * be liable for any direct, indirect, incidental, special, exemplary, or
 * consequential damages.
 *
 *
 * File: rhine_hw.h
 *
 * Purpose: Rhine series MAC register supporting header file.
 *
 * Author: Chuang Liang-Shing, AJ Jiang, Guard Kuo
 *
 * Date: Jan 28, 2005
 *
 *
 */
 
#ifndef __RHINE_MAC_H__
#define __RHINE_MAC_H__

#include "osdep.h"

/* Registers in the MAC */

#define MAC_REG_PAR         0x00        /* physical address */
#define MAC_REG_RCR         0x06        
#define MAC_REG_TCR         0x07        
#define MAC_REG_CR0         0x08        
#define MAC_REG_CR1         0x09        
#define MAC_REG_TQWK        0x0A
#define MAC_REG_ISR         0x0C        
#define MAC_REG_IMR         0x0E        
#define MAC_REG_MAR         0x10        
#define MAC_REG_MCAM        0x10        
#define MAC_REG_VCAM        0x16        
#define MAC_REG_CUR_RD_ADDR 0x18        
#define MAC_REG_CUR_TD_ADDR 0x1C        
#define MAC_REG_RX_DMA_PTR  0x60        
#define MAC_REG_MPHY        0x6C        
#define MAC_REG_MIISR       0x6D        
#define MAC_REG_BCR0        0x6E        
#define MAC_REG_BCR1        0x6F        
#define MAC_REG_MIICR       0x70        
#define MAC_REG_MIIAD       0x71        
#define MAC_REG_MIIDATA     0x72        
#define MAC_REG_EECSR       0x74        
#define MAC_REG_TEST        0x75        
#define MAC_REG_CFGA        0x78        
#define MAC_REG_CFGB        0x79        
#define MAC_REG_CFGC        0x7A        
#define MAC_REG_CFGD        0x7B        
#define MAC_REG_CNTR_MPA    0x7C        
#define MAC_REG_CNTR_CRC    0x7E        

/* Registers for VT6102 */
#define MAC_REG_GFTEST      0x54    
#define MAC_REG_RFTCMD      0x55    
#define MAC_REG_TFTCMD      0x56    
#define MAC_REG_GFSTATUS    0x57    
#define MAC_REG_BNRY        0x58    
#define MAC_REG_CURR        0x5A    
#define MAC_REG_FIFO_DATA   0x5C    
#define MAC_REG_CUR_RXDMA   0x60    
#define MAC_REG_CUR_TXDMA   0x64    

#define MAC_REG_MISC_CR0    0x80    
#define MAC_REG_MISC_CR1    0x81    
#define MAC_REG_PMCPORT     0x82    
#define MAC_REG_STICKHW     0x83    
#define MAC_REG_MISR        0x84    
#define MAC_REG_MIMR        0x86    
#define MAC_REG_CAMMSK      0x88    
#define MAC_REG_BPMA        0x8C    
#define MAC_REG_BPMD        0x8F    
#define MAC_REG_BPCMD       0x90    
#define MAC_REG_BPIN_DATA   0x91    
#define MAC_REG_CAMC        0x92    
#define MAC_REG_CAMADD      0x93    
#define MAC_REG_EECHKSUM    0x93    
#define MAC_REG_SUS_MII_AD  0x94    
#define MAC_REG_MIBCR       0x94
#define MAC_REG_MIBDATA     0x96    
#define MAC_REG_SUS_PHY_ID  0x96    
#define MAC_REG_MIBPORT     0x96    
#define MAC_REG_MIBDAT      0x97    
#define MAC_REG_FLOWCR0     0x98
#define MAC_REG_FLOWCR1     0x99
#define MAC_REG_PAUSE_TIMER 0x9A    
#define MAC_REG_SOFT_TIMER0 0x9C    
#define MAC_REG_SOFT_TIMER1 0x9E    
#define MAC_REG_WOLCR_SET   0xA0    
#define MAC_REG_PWCFG_SET   0xA1    
#define MAC_REG_TSTREG_SET  0xA2    
#define MAC_REG_WOLCG_SET   0xA3    
#define MAC_REG_WOLCR_CLR   0xA4    
#define MAC_REG_PWCFG_CLR   0xA5    
#define MAC_REG_TSTREG_CLR  0xA6    
#define MAC_REG_WOLCG_CLR   0xA7    
#define MAC_REG_PWRCSR_SET  0xA8    
#define MAC_REG_PWRCSR1_SET 0xA9
#define MAC_REG_PWRCSR_CLR  0xAC    
#define MAC_REG_PWRCSR1_CLR 0xAD
#define MAC_REG_PATRN_CRC0  0xB0    
#define MAC_REG_PATRN_CRC1  0xB4    
#define MAC_REG_PATRN_CRC2  0xB8    
#define MAC_REG_PATRN_CRC3  0xBC    
#define MAC_REG_BYTEMSK0_0  0xC0    
#define MAC_REG_BYTEMSK0_1  0xC4    
#define MAC_REG_BYTEMSK0_2  0xC8    
#define MAC_REG_BYTEMSK0_3  0xCC    
#define MAC_REG_BYTEMSK1_0  0xD0    
#define MAC_REG_BYTEMSK1_1  0xD4    
#define MAC_REG_BYTEMSK1_2  0xD8    
#define MAC_REG_BYTEMSK1_3  0xDC    
#define MAC_REG_BYTEMSK2_0  0xE0    
#define MAC_REG_BYTEMSK2_1  0xE4    
#define MAC_REG_BYTEMSK2_2  0xE8    
#define MAC_REG_BYTEMSK2_3  0xEC    
#define MAC_REG_BYTEMSK3_0  0xF0    
#define MAC_REG_BYTEMSK3_1  0xF4    
#define MAC_REG_BYTEMSK3_2  0xF8    
#define MAC_REG_BYTEMSK3_3  0xFC    

#define BYTE_REG_BITS_ON(hw,x,p)   do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p))|(x)),(p));} while (0)
#define WORD_REG_BITS_ON(hw,x,p)   do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p))|(x)),(p));} while (0)
#define DWORD_REG_BITS_ON(hw,x,p)  do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p))|(x)),(p));} while (0)

#define BYTE_REG_BITS_IS_ON(hw,x,p) (CSR_READ_1(hw,(p)) & (x))
#define WORD_REG_BITS_IS_ON(hw,x,p) (CSR_READ_2(hw,(p)) & (x))
#define DWORD_REG_BITS_IS_ON(hw,x,p) (CSR_READ_4(hw,(p)) & (x))

#define BYTE_REG_BITS_OFF(hw,x,p)  do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p)) & (~(x))),(p));} while (0)
#define WORD_REG_BITS_OFF(hw,x,p)  do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p)) & (~(x))),(p));} while (0)
#define DWORD_REG_BITS_OFF(hw,x,p) do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p)) & (~(x))),(p));} while (0)

#define BYTE_REG_BITS_SET(hw,x,m,p)    do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p)) & (~(m))) |(x),(p));} while (0)
#define WORD_REG_BITS_SET(hw,x,m,p)    do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p)) & (~(m))) |(x),(p));} while (0)
#define DWORD_REG_BITS_SET(hw,x,m,p)   do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p)) & (~(m)))|(x),(p));}  while (0)



/* Bits in the RCR register */

#define RCR_RRFT2           0x80        
#define RCR_RRFT1           0x40        
#define RCR_RRFT0           0x20        
#define RCR_PROM            0x10        
#define RCR_AB              0x08        
#define RCR_AM              0x04        
#define RCR_AR              0x02        
#define RCR_SEP             0x01        


/* Bits in the TCR register */

#define TCR_RTSF            0x80        
#define TCR_RTFT1           0x40        
#define TCR_RTFT0           0x20        
#define TCR_RTGOPT          0x10        
#define TCR_OFSET           0x08        
#define TCR_LB1             0x04        /* loopback[1] */ 
#define TCR_LB0             0x02        /* loopback[0] */ 
#define TCR_PQEN            0x01


/* Bits in the CR0 register */

#define CR0_RDMD            0x40        /* rx descriptor polling demand */ 
#define CR0_TDMD            0x20        /* tx descriptor polling demand */ 
#define CR0_TXON            0x10        
#define CR0_RXON            0x08        
#define CR0_STOP            0x04        /* stop MAC, default = 1 */ 
#define CR0_STRT            0x02        /* start MAC */ 
#define CR0_INIT            0x01        /* start init process */ 

#define CR0_SFRST           0x8000      /* software reset */ 
#define CR0_RDMD1           0x4000      
#define CR0_TDMD1           0x2000      
#define CR0_KEYPAG          0x1000      
#define CR0_DPOLL           0x0800      /* disable rx/tx auto polling */ 
#define CR0_FDX             0x0400      /* full duplex mode */ 
#define CR0_ETEN            0x0200      /* early tx mode */ 
#define CR0_EREN            0x0100      /* early rx mode */ 

/* Bits in the CR1 register */

#define CR1_SFRST           0x80        /* software reset */ 
#define CR1_RDMD1           0x40        
#define CR1_TDMD1           0x20        
#define CR1_REAUTO          0x10        /* for VT6105 */ 
#define CR1_KEYPAG          0x10        
#define CR1_DPOLL           0x08        /* disable rx/tx auto polling */ 
#define CR1_FDX             0x04        /* full duplex mode */ 
#define CR1_DISAU           0x02        /* for VT6105 */ 
#define CR1_ETEN            0x02        /* early tx mode */ 
#define CR1_EREN            0x01        /* early rx mode */ 

/* Bits in the IMR register */

#define IMR_GENM            0x8000      
#define IMR_SRCM            0x4000      
#define IMR_ABTM            0x2000      
#define IMR_NORBFM          0x1000      
#define IMR_PKTRAM          0x0800      
#define IMR_OVFM            0x0400      
#define IMR_ETM             0x0200      
#define IMR_ERM             0x0100      
#define IMR_CNTM            0x0080      
#define IMR_BEM             0x0040      
#define IMR_RUM             0x0020      
#define IMR_TUM             0x0010      
#define IMR_TXEM            0x0008      
#define IMR_RXEM            0x0004      
#define IMR_PTXM            0x0002      
#define IMR_PRXM            0x0001      

#define IMR_TDWBRAI         0x00080000UL 
#define IMR_TM1_INT         0x00020000UL 
#define IMR_TM0_INT         0x00010000UL 

/* Bits in the ISR (MISR) register */

#define ISR_GENI            0x00008000UL    /*  for 6102 */ 
#define ISR_SRCI            0x00004000UL    
#define ISR_ABTI            0x00002000UL    
#define ISR_NORBF           0x00001000UL    
#define ISR_PKTRA           0x00000800UL    
#define ISR_OVFI            0x00000400UL    
#define ISR_UDFI            0x00000200UL    /*  for 6102 */ 
#define ISR_ERI             0x00000100UL    
#define ISR_CNT             0x00000080UL    
#define ISR_BE              0x00000040UL    
#define ISR_RU              0x00000020UL    
#define ISR_TU              0x00000010UL    
#define ISR_TXE             0x00000008UL    
#define ISR_RXE             0x00000004UL    
#define ISR_PTX             0x00000002UL    
#define ISR_PRX             0x00000001UL    
/* Bits in MISR */
#define ISR_PWEINT          0x00800000UL    /* power event report in test mode */ 
#define ISR_UDPINT_CLR      0x00400000UL    /* userdefined, host driven interrupt.clear */ 
#define ISR_UDPINT_SET      0x00200000UL    /* userdefined, host driven interrupt.Set */ 
#define ISR_SSRCI           0x00100000UL    /* suspend well mii polling status change interrupt */ 
#define ISR_TDWBRAI         0x00080000UL    /* TD WB queue race */ 
#define ISR_PHYINT          0x00040000UL    /* PHY state change interrupt, active by
                                               PHYINTEN (misc.cr[9]) in normal mode */
#define ISR_TM1_INT         0x00020000UL    
#define ISR_TM0_INT         0x00010000UL    



/* Bits in the MIISR register */

#define MIISR_N_FDX         0x04
#define MIISR_LNKFL         0x02        
#define MIISR_SPEED         0x01        


/* Bits in the MIICR register */

#define MIICR_MAUTO         0x80        
#define MIICR_RCMD          0x40        
#define MIICR_WCMD          0x20        
#define MIICR_MDPM          0x10        
#define MIICR_MOUT          0x08        
#define MIICR_MDO           0x04        
#define MIICR_MDI           0x02        
#define MIICR_MDC           0x01        


/* Bits in the MIIAD register */

#define MIIAD_MIDLE         0x80        
#define MIIAD_MSRCEN        0x40        
#define MIIAD_MDONE         0x20        
#define MIIAD_MAD0          0x01

/* Bits in the MIBCR register */

#define MIBCR_MIBEN         0x10
#define MIBCR_MIBHALF       0x20
#define MIBCR_MIBINC        0x40
#define MIBCR_MIBRTN        0x80

/* Bits in the EECSR register */

#define EECSR_EEPR          0x80        /* eeprom programed status, 73h means programed */ 
#define EECSR_EMBP          0x40        /* eeprom embeded programming */ 
#define EECSR_AUTOLD        0x20        /* eeprom content reload */ 
#define EECSR_DPM           0x10        /* eeprom direct programming */ 
#define EECSR_CS            0x08        /* eeprom CS pin */ 
#define EECSR_SK            0x04        /* eeprom SK pin */ 
#define EECSR_DI            0x02        /* eeprom DI pin */ 
#define EECSR_DO            0x01        /* eeprom DO pin */ 


/* Bits in the BCR0 register */

#define BCR0_BOOT_MASK      ((BYTE) 0xC0)
#define BCR0_BOOT_INT19     ((BYTE) 0x00)
#define BCR0_BOOT_INT18     ((BYTE) 0x40)
#define BCR0_BOOT_LOCAL     ((BYTE) 0x80)
#define BCR0_BOOT_BEV       ((BYTE) 0xC0)

#define BCR0_MED2           0x80        
#define BCR0_LED100M        0x40        
#define BCR0_CRFT2          0x20        
#define BCR0_CRFT1          0x10        
#define BCR0_CRFT0          0x08        
#define BCR0_DMAL2          0x04        
#define BCR0_DMAL1          0x02        
#define BCR0_DMAL0          0x01        



/* Bits in the BCR1 register */

#define BCR1_MED1           0x80        /* for VT6102 */ 
#define BCR1_MED0           0x40        /* for VT6102 */ 
#define BCR1_VIDFR          0x80        /* for VT6105 */ 
#define BCR1_TXQNOBK        0x40        /* for VT6105 */ 
#define BCR1_CTSF           0x20        
#define BCR1_CTFT1          0x10        
#define BCR1_CTFT0          0x08        
#define BCR1_POT2           0x04        
#define BCR1_POT1           0x02        
#define BCR1_POT0           0x01        


/* Bits in the CFGA register */

#define CFGA_EELOAD         0x80        /* enable eeprom embeded and direct programming */ 
#define CFGA_LED0S0         0x01        

/* Bits in the CFGB register */

#define CFGB_QPKTDIS        0x80        
#define CFGB_MRLDIS         0x20        


/* Bits in the CFGC register */

#define CFGC_BOOT_RPL       ((BYTE) 0x80) /*Boot method selection for VT3106 */
#define CFGC_MEDEN          0x80        
#define CFGC_BROPT          0x40        
#define CFGC_DLYEN          0x20        
#define CFGC_DTSEL          0x10        
#define CFGC_BTSEL          0x08        
#define CFGC_BPS2           0x04        /* bootrom select[2] */ 
#define CFGC_BPS1           0x02        /* bootrom select[1] */ 
#define CFGC_BPS0           0x01        /* bootrom select[0] */ 

typedef enum {
    RHINE_VLAN_ID_CAM=0,
    RHINE_MULTICAST_CAM
} RHINE_CAM_TYPE, *PRHINE_CAM_TYPE;


/* Bits in the CAMCR register */


#define CAMC_CAMRD          0x08
#define CAMC_CAMWR          0x04
#define CAMC_VCAMSL         0x02
#define CAMC_CAMEN          0x01

/* Bits in the CFGD register */

#define CFGD_GPIOEN         0x80        
#define CFGD_DIAG           0x40        
#define CFGD_MAGIC          0x10        
#define CFGD_CRADOM         0x08        
#define CFGD_CAP            0x04        
#define CFGD_MBA            0x02        
#define CFGD_BAKOPT         0x01        

/* for VT6102 */

/* Bits in STICKHW */
#define STICKHW_LEGWOLEN        0x0080  /* status for software reference */ 
#define STICKHW_LEGACY_WOLSR    0x0008
#define STICKHW_LEGACY_WOLEN    0x0004
#define STICKHW_DS1_SHADOW      0x0002  /* R/W by software/cfg cycle */ 
#define STICKHW_DS0_SHADOW      0x0001  /* suspend well DS write port */ 

/* Bits in MISC.CR0 */
#define MISC_CR0_TM0US          0x20
#define MISC_CR0_FDXTFEN        0x10    /* Full-duplex flow control TX enable */ 
#define MISC_CR0_FDXRFEN        0x08    /* Full-duplex flow control RX enable */ 
#define MISC_CR0_HDXFEN         0x04    /* Half-duplex flow control enable */ 
#define MISC_CR0_TIMER0_SUSPEND 0x02
#define MISC_CR0_TIMER0_EN      0x01

/* Bits in MISC.CR1 */
#define MISC_CR1_FORSRST        0x40
#define MISC_CR1_VAUXJMP        0x20
#define MISC_CR1_PHYINT         0x02
#define MISC_CR1_TIMER1_EN      0x01

/* Bits in BPCMD */
#define BPCMD_BPDNE             0x80
#define BPCMD_EBPWR             0x02
#define BPCMD_EBPRD             0x01

/* Bits in MISR */
#define MISR_PWEINT             0x80    /* power event report in test mode */ 
#define MISR_UDPINT_CLR         0x40    /* userdefined, host driven interrupt.clear */ 
#define MISR_UDPINT_SET         0x20    /* userdefined, host driven interrupt.Set */ 
#define MISR_SSRCI              0x10    /* suspend well mii polling status change interrupt */ 
#define MISR_TDWBRAI            0x08    /* TD WB queue race */ 
#define MISR_PHYINT             0x04    /* PHY state change interrupt, active by
                                           PHYINTEN (misc.cr[9]) in normal mode */
#define MISR_TM1_INT            0x02
#define MISR_TM0_INT            0x01

/* Bits in WOLCR */
#define WOLCR_LNKOFF_EN         0x80    /* link off detected enable */ 
#define WOLCR_LNKON_EN          0x40    /* link on detected enable */ 
#define WOLCR_MAGPKT_EN         0x20    /* magic packet filter enable */ 
#define WOLCR_UNICAST_EN        0x10    /* unicast filter enable */ 
#define WOLCR_MSWOLEN3          0x08    /* enable pattern match filtering */ 
#define WOLCR_MSWOLEN2          0x04
#define WOLCR_MAGPKT_SEC_EN     0x02    /* Magice packet with a password */ 
#define WOLCR_MSWOLEN1          0x02
#define WOLCR_MSWOLEN0          0x01
#define WOLCR_ARP_EN            0x01    /* arp */ 

/* Bits in PWCFG */
#define PWCFG_SMIITIME          0x80    /* internal MII I/F timing */ 
#define PWCFG_PCISTICK          0x40    /* PCI sticky R/W enable */ 
#define PWCFG_WOLTYPE           0x20    /* pulse(1) or button (0) */ 
#define PWCFG_LEGCY_WOL         0x10
#define PWCFG_PMCSR_PME_SR      0x08
#define PWCFG_PMCSR_PME_EN      0x04    /* control by PCISTICK */ 
#define PWCFG_LEGACY_WOLSR      0x02    /* Legacy WOL_SR shadow */ 
#define PWCFG_LEGACY_WOLEN      0x01    /* Legacy WOL_EN shadow */ 

/* Bits in TestReg */
#define TSTREG_SGENWATCH        0x08
#define TSTREG_SMCSNOOP         0x04
#define TSTREG_SMACTEST         0x02
#define TSTREG_SNORMAL          0x01

/* Bits in WOLCFG */
#define WOLCFG_PME_OVR          0x80    /* for legacy use, force PMEEN always */ 
#define WOLCFG_SFDX             0x40    /* full duplex while in WOL mode */ 
#define WOLCFG_SAM              0x20    /* accept multicast case reset, default=0 */ 
#define WOLCFG_SAB              0x10    /* accept broadcast case reset, default=0 */ 
#define WOLCFG_SMIIACC          0x08    /* ?? */ 
#define WOLCFG_SMIIOPT          0x04    /* MIIOPT to extend clock in suspendwell */ 
#define WOLCFG_PTNPAG           0x04
#define WOLCFG_SSRCEN           0x02    /* suspend well mii status change enable */ 
#define WOLCFG_PHYINTEN         0x01    /* 0:PHYINT trigger enable, 1:use internal MII
                                           to report status change */

/* Ethernet address filter type */
#define PKT_TYPE_NONE               0x0000  /* turn off receiver */ 
#define PKT_TYPE_DIRECTED           0x0001  /* obselete, directed address is always accepted */ 
#define PKT_TYPE_MULTICAST          0x0002
#define PKT_TYPE_ALL_MULTICAST      0x0004
#define PKT_TYPE_BROADCAST          0x0008
#define PKT_TYPE_PROMISCUOUS        0x0020
#define PKT_TYPE_LONG               0x2000  /* NOTE.... the definition of LONG is >2048 bytes in our chip */ 
#define PKT_TYPE_RUNT               0x4000
#define PKT_TYPE_ERROR              0x8000  /* accept error packets, e.g. CRC error */ 


/* Loopback mode */
#define MAC_LB_NONE         0x00        
#define MAC_LB_INTERNAL     0x01        
#define MAC_LB_PHY          0x02        /* MII or Internal-10BaseT loopback */ 


/* Forced media type */
#define FORCED_MEDIA_NONE       0x00    
#define FORCED_MEDIA_AUTO       0x01    
#define FORCED_MEDIA_100M_HALF  0x02    /* hub card */ 
#define FORCED_MEDIA_100M_FULL  0x03    /* fiber channel */ 
#define FORCED_MEDIA_10M_HALF   0x04    
#define FORCED_MEDIA_10M_FULL   0x05    




/* Registers in the PCI configuration space */

#define PCI_REG_COMMAND     0x04        
#define PCI_REG_MODE0       0x50        
#define PCI_REG_FIFOTST     0x51        
#define PCI_REG_MODE2       0x52        
#define PCI_REG_MODE3       0x53        
#define PCI_REG_DELAY_TIMER 0x54        
#define PCI_REG_FIFOCMD     0x56        
#define PCI_REG_FIFOSTA     0x57        
#define PCI_REG_BNRY        0x58        
#define PCI_REG_CURR        0x5A        
#define PCI_REG_FIFO_DATA   0x5C        


/* Registers for power management (offset) */
#define PCI_REG_PM_BASE     0x40        

#define PM_CAP_ID           0x00        
#define PM_NEXT_ITEM_PTR    0x01        
#define PM_PMC0             0x02        
#define PM_PMC1         0x03        
#define PM_PMCSR0       0x04        
#define PM_PMCSR1       0x05        
#define PM_CSR_BSE      0x06        
#define PM_DATA         0x07        


/* Bits in the COMMAND register */

#define COMMAND_BUSM        0x04        


/* Bits in the MODE0 register */

#define MODE0_QPKTDS        0x80        


/* Bits in the MODE2 register */

#define MODE2_PCEROPT       0x80        /* VT6102 only */ 
#define MODE2_DISABT        0x40        
#define MODE2_MRDPL         0x08        /* VT6107A1 and above */ 
#define MODE2_MODE10T       0x02        


/* Bits in the MODE3 register */

#define MODE3_XONOPT        0x80
#define MODE3_TPACEN        0x40
#define MODE3_BACKOPT       0x20
#define MODE3_DLTSEL        0x10
#define MODE3_MIIDMY        0x08
#define MODE3_MIION         0x04


/* Bits in the RSR0 register */

#define RSR0_BUFF           0x80
#define RSR0_FRAG           0x40
#define RSR0_SERR           0x40
#define RSR0_RUNT           0x20
#define RSR0_LONG           0x10
#define RSR0_FOV            0x08
#define RSR0_FAE            0x04
#define RSR0_CRC            0x02
#define RSR0_RERR           0x01


/* Bits in the RSR1 register */

#define RSR1_RXOK           0x8000      /* rx OK */ 
#define RSR1_VIDHIT         0x4000      /* VID Hit */ 
#define RSR1_MAR            0x2000      /* MAC accept multicast address packet */ 
#define RSR1_BAR            0x1000      /* MAC accept broadcast address packet */ 
#define RSR1_PHY            0x0800      /* MAC accept physical address packet */ 
#define RSR1_CHN            0x0400      /* chain buffer, always = 1 */ 
#define RSR1_STP            0x0200      /* start of packet */ 
#define RSR1_EDP            0x0100      /* end of packet */ 


#define RSR_OWN             0x80000000

#define PQSTS_RXLERR        0x800000
#define PQSTS_SNPTAG        0x400000
#define PQSTS_IPOK          0x200000        /*IP Checkusm validatiaon ok */
#define PQSTS_TUOK          0x100000        /*TCP/UDP Checkusm validatiaon ok */ 
#define PQSTS_IPKT          0x080000        /*Received an IP packet */ 
#define PQSTS_TCPKT         0x040000        /*Received a TCP packet */ 
#define PQSTS_UDPKT         0x020000        /*Received a UDP packet */ 
#define PQSTS_TAG           0x010000        /*Received a tagged packet */ 


/* Bits in the TSR0 register */

#define TSR0_CDH            0x80        /* AQE test fail (CD heartbeat) */
#define TSR0_COLS           0x10        /* experience collision in this transmit event */
#define TSR0_NCR3           0x08        /* collision retry counter[3] */
#define TSR0_NCR2           0x04        /* collision retry counter[2] */
#define TSR0_NCR1           0x02        /* collision retry counter[1] */
#define TSR0_NCR0           0x01        /* collision retry counter[0] */
#define TSR0_NCR            (TSR0_NCR0|TSR0_NCR1|TSR0_NCR2|TSR0_NCR3)

/* Bits in the TSR1 register */

#define TSR1_TERR           0x8000      
#define TSR1_JAB            0x4000      /* jabber condition occured */
#define TSR1_SERR           0x2000      
#define TSR1_TBUFF          0x1000      
#define TSR1_UDF            0x0800      
#define TSR1_CRS            0x0400      
#define TSR1_OWC            0x0200      /* late collision */
#define TSR1_ABT            0x0100      


/* Bits in the TCR register */

#define TCR_IC              0x800000    /* assert interrupt immediately
                                           while descriptor has been send complete */
#define TCR_EDP             0x400000    /* end of packet */
#define TCR_STP             0x200000    /* start of packet */
#define TCR_TCPCK           0x100000    /* request TCP checksum calculation. */
#define TCR_UDPCK           0x080000    /* request UDP checksum calculation. */
#define TCR_IPCK            0x040000    /* request TCP checksum calculation. */
#define TCR_TAG             0x020000    /* Do insert tag */
#define TCR_CRC             0x010000    /* disable CRC generation */

#define TSR_OWN             0x80000000
#define TCR_CHAIN           0x8000

/* Bits in the FlowCR1 register */

#define FLOWCR1_XHITH1      0x80
#define FLOWCR1_XHITH0      0x40
#define FLOWCR1_XLTH1       0x20
#define FLOWCR1_XLTH0       0x10
#define FLOWCR1_XONEN       0x08
#define FLOWCR1_FDXTFCEN    0x04
#define FLOWCR1_FDXRFCEN    0x02
#define FLOWCR1_HDXFCEN     0x01

/*
 * General constants that are fun to know.
 *
 * vendor ID
 */
#define VENDORID        0x1106

/*
 * device IDs.
 */
#define DEVICEID_3043   0x3043
#define DEVICEID_3065   0x3065
#define DEVICEID_3106   0x3106
#define DEVICEID_3053   0x3053

/*
// Revision ID
 */
#define REV_ID_VT86C100A_E  0x04
#define REV_ID_VT3071_A     0x20
#define REV_ID_VT3071_B     0x21
#define REV_ID_VT6102_A     0x40
#define REV_ID_VT6102_C     0x42
#define REV_ID_VT6105_A0    0x80
#define REV_ID_VT6105_B0    0x83
#define REV_ID_VT6105_LOM   0x8A
#define REV_ID_VT6107_A0    0x8C
#define REV_ID_VT6107_A1    0x8D
#define REV_ID_VT6105M_A0   0x90
#define REV_ID_VT6105M_B1   0x94

/*
// IMR Mask [4.39]
 */
#define IMR_MASK_VALUE          0x000BD7FFUL                                        /* initial value of IMR */
#define IMR_MASK_EXCEPT_PRX     (IMR_MASK_VALUE & (~(DWORD)IMR_PRXM))               /* Except PRXM */
#define IMR_MASK_EXCEPT_PTX     (IMR_MASK_VALUE & (~(DWORD)IMR_PTXM))               /* Except PTXM */
#define IMR_MASK_EXCEPT_PTXPRX  (IMR_MASK_VALUE & (~(DWORD)(IMR_PRXM | IMR_PTXM)))  /* Except PTXM & PRXM */


typedef enum _speed_opt {
    SPD_DPX_AUTO     = 0,
    SPD_DPX_100_HALF = 1,
    SPD_DPX_100_FULL = 2,
    SPD_DPX_10_HALF  = 3,
    SPD_DPX_10_FULL  = 4
} SPD_DPX_OPT, *PSPD_DPX_OPT;

#define     RHINE_WOL_MAGIC             0x00000000UL
#define     RHINE_WOL_PHY               0x00000001UL
#define     RHINE_WOL_ARP               0x00000002UL
#define     RHINE_WOL_UCAST             0x00000004UL
#define     RHINE_WOL_BCAST             0x00000010UL
#define     RHINE_WOL_MCAST             0x00000020UL
#define     RHINE_WOL_MAGIC_SEC         0x00000040UL

/* flags for options */
#define     RHINE_FLAGS_TAGGING         0x00000001UL
#define     RHINE_FLAGS_TX_CSUM         0x00000002UL
#define     RHINE_FLAGS_RX_CSUM         0x00000004UL
#define     RHINE_FLAGS_IP_ALIGN        0x00000008UL
#define     RHINE_FLAGS_VAL_PKT_LEN     0x00000010UL

/* flags for driver status */
#define     RHINE_FLAGS_OPENED          0x00010000UL
#define     RHINE_FLAGS_WOL_ENABLED     0x00080000UL
/*flags for capbilities*/
#define     RHINE_FLAGS_TX_ALIGN        0x01000000UL
#define     RHINE_FLAGS_HAVE_CAM        0x02000000UL
#define     RHINE_FLAGS_FLOW_CTRL       0x04000000UL

/* flags for MII status  */
#define     RHINE_LINK_FAIL             0x00000001UL
#define     RHINE_SPEED_10              0x00000002UL
#define     RHINE_SPEED_100             0x00000004UL
#define     RHINE_SPEED_1000            0x00000008UL
#define     RHINE_DUPLEX_FULL           0x00000010UL
#define     RHINE_AUTONEG_ENABLE        0x00000020UL
#define     RHINE_FORCED_BY_EEPROM      0x00000040UL
/* for rhine_set_media_duplex  */
#define     RHINE_LINK_CHANGE           0x00000001UL
#define     RHINE_LINK_UNCHANGE         0x00000002UL


/*
// Registers in the MII (offset unit is WORD)
 */
#define MII_REG_BMCR        0x00        /* physical address */
#define MII_REG_BMSR        0x01        
#define MII_REG_PHYID1      0x02        /* OUI */
#define MII_REG_PHYID2      0x03        /* OUI + Module ID + REV ID */
#define MII_REG_ANAR        0x04        
#define MII_REG_ANLPAR      0x05        
#define MII_REG_MODCFG      0x10
/* NS, MYSON only  */
#define MII_REG_PCR         0x17        
/* ESI only  */
#define MII_REG_PCSR        0x17        

/*
// Bits in the BMCR register
*/
#define BMCR_RESET          0x8000      
#define BMCR_LBK            0x4000      
#define BMCR_SPEED          0x2000      
#define BMCR_AUTO           0x1000      
#define BMCR_PD             0x0800      
#define BMCR_ISO            0x0400      
#define BMCR_REAUTO         0x0200      
#define BMCR_FDX            0x0100      

/*
// Bits in the BMSR register
*/
#define BMSR_AUTOCM         0x0020      
#define BMSR_LNK            0x0004      

/*
// Bits in the ANAR register
*/
#define ANAR_ASMDIR         0x0800      /* Asymmetric PAUSE support */
#define ANAR_PAUSE          0x0400      /* Symmetric PAUSE Support */
#define ANAR_T4             0x0200      
#define ANAR_TXFD           0x0100      
#define ANAR_TX             0x0080      
#define ANAR_10FD           0x0040      
#define ANAR_10             0x0020      

/*
// Bits in the ANLPAR register
*/
#define ANLPAR_ASMDIR       0x0800      /* Asymmetric PAUSE support */
#define ANLPAR_PAUSE        0x0400      /* Symmetric PAUSE Support */
#define ANLPAR_T4           0x0200
#define ANLPAR_TXFD         0x0100
#define ANLPAR_TX           0x0080
#define ANLPAR_10FD         0x0040
#define ANLPAR_10           0x0020

#define IS_PHY_VT6103(p)        (( ((p)->dwPHYId & 0xFFFFFFF0UL) ==0x010108F20UL)\
                                && ( ((p)->dwPHYId & 0xFUL) >4) )
#define W_MAX_TIMEOUT       0x3fff


/*
 * Rhine TX/RX list structure.
 */
#define FET_RXSTAT_RXLEN            0x07FF0000
#define FET_RXSTAT_RXLEN_EXT        0x78000000

#define FET_RXBYTES(x)      ((x & FET_RXSTAT_RXLEN) >> 16)
#define FET_RXSTAT (RSR1_STP|RSR1_EDP|RSR_OWN)

#define FET_RXCTL_BUFLEN        0x000007ff
#define FET_RXCTL_BUFLEN_EXT    0x00007800
#define FET_RXCTL_CHAIN         0x00008000
#define FET_RXCTL (FET_RXCTL_CHAIN|FET_RXCTL_BUFLEN)

#define FET_TXSTAT_PQMASK       0x7FFF0000

#define FET_TXCTL_BUFLEN        0x000007ff
#define FET_TXCTL_BUFLEN_EXT    0x00007800

typedef enum {
    OWNED_BY_HOST=0,
    OWNED_BY_NIC=1
} RHINE_OWNER_TYPE, *PRHINE_OWNER_TYPE;

typedef struct __rx_desc {
    volatile U32                rdesc0;
    volatile U32                rdesc1;
    volatile U32                buff_addr;
    volatile U32                next_desc;
    volatile U32                Reserved[4]; /* 16 bytes */
} __attribute__ ((__packed__))
RX_DESC, *PRX_DESC;



typedef struct _tx_desc {
    volatile U32                tdesc0;
    volatile U32                tdesc1;
    volatile U32                buff_addr;
    volatile U32                next_desc;
    volatile U32                Reserved[4]; /* 16 bytes */
} __attribute__ ((__packed__))
TX_DESC, *PTX_DESC;


#define ADD_ONE_WITH_WRAP_AROUND(uVar, uModulo) {   \
    if ((uVar) >= ((uModulo) - 1))                  \
        (uVar) = 0;                                 \
    else                                            \
        (uVar)++;                                   \
}


/* Support Adapter Name */
#define RHINE_FULL_DRV_NAM      "VIA Rhine Family Fast Ethernet Adapter Driver"
#define RHINE_II_DRV_NAME       "VIA Rhine II Fast Ethernet Adapter"
#define RHINE_III_DRV_NAME      "VIA Rhine III Fast Ethernet Adapter"
#define RHINE_IIIM_DRV_NAME     "VIA Rhine III Management Adapter"
#define RHINE_86C100A_DRV_NAME  "VIA VT86C100A Rhine Fast Ethernet Adapte"


typedef enum  _chip_type{
    VT86C100A = 1,
    VT6102 = 2,
    VT6105 = 3,
    VT6105M = 4
} CHIP_TYPE, *PCHIP_TYPE;

typedef struct _pci_device{
    U16 vendor;
    U16 device;
    U16 subvendor;
    U16 subsystem;
    CHIP_TYPE type;
} PCI_DEVICE_TABLE, *PPCI_DEVICE_TABLE;

/* Preventing driver from accessing system varialbe directly */
typedef struct __rhine_stats {
    int tx_packets;
    U32 tx_bytes;
    int tx_errors;
    int tx_dropped;
    int tx_fifo_errors;
    int tx_aborted_errors;
    int tx_carrier_errors;
    int tx_window_errors;
    int tx_heartbeat_errors;
    int collisions;
    int rx_errors;
    int rx_dropped;
    int rx_crc_errors;
    int rx_frame_errors;
    int rx_fifo_errors;
    int rx_packets;
} RHINE_STATS, *PRHINE_STATS;


typedef struct __rhine_opt {
    int         nRxDescs;       /* Number of RX descriptors */
    int         nTxDescs;       /* Number of TX descriptors */
    SPD_DPX_OPT spd_dpx;        /* Media link mode */
    int         vid;            /* vlan id */
    int         tx_thresh;      /* Tx Fifo threshold */
    int         rx_thresh;      /* Rx fifo threshold */
    int         DMA_length;     /* DMA length */
    int         flow_cntl;
    int         wol_opts;       /* Wake on lan options */
    int         td_int_count;
    int         int_works;
    int         rx_bandwidth_hi;
    int         rx_bandwidth_lo;
    int         rx_bandwidth_en;
    U32         flags;
} OPTIONS, *POPTIONS;

struct rhine_hw {
    long                        memaddr;
    long                        ioaddr;
    U32                         io_size;
    PU8                         hw_addr;

    U32                         dwPHYId;
    U8                          byRevId;
    U16                         SubSystemID;
    U16                         SubVendorID;


#define AVAIL_TD(hw,q)   ((hw)->sOpts.nTxDescs-((hw)->iTDUsed[(q)]))
    int                         nTxQueues;
    volatile int                iTDUsed[TX_QUEUE_NO];
    int                         aiCurrTDIdx[TX_QUEUE_NO];
    int                         aiTailTDIdx[TX_QUEUE_NO];
    PTX_DESC                    apTDRings[TX_QUEUE_NO];

    int                         iCurrRDIdx;
    PRX_DESC                    aRDRing;


    OPTIONS                     sOpts;

    U32                         IntMask;
    U32                         flags;

    unsigned int                rx_buf_sz;
    int                         multicast_limit;

    RHINE_STATS                 stats;

    /* operation-system-specific structure */
    struct em_osdep             *back;
};

/* export function */
void rhine_clearISR(struct rhine_hw *hw);
U32 rhine_ReadISR(struct rhine_hw *hw);
void rhine_WriteISR(U32 status,struct rhine_hw *hw);
void rhine_disable_int(struct rhine_hw *hw);
void rhine_enable_int(struct rhine_hw *hw, U32 IsrStatus);

void rhine_enable_mmio(struct rhine_hw *hw);
void rhine_reload_eeprom(struct rhine_hw *hw);
void rhine_wol_reset(struct rhine_hw *hw);
void rhine_set_tx_thresh(struct rhine_hw *hw, int tx_thresh);
void rhine_set_rx_thresh(struct rhine_hw *hw, int rx_thresh);
void rhine_set_DMA_length(struct rhine_hw *hw, int DMA_length);
void rhine_init_flow_control_register(struct rhine_hw *hw, U16 RxDescs);
U32 rhine_check_media_mode(struct rhine_hw *hw);
void rhine_set_duplex(struct rhine_hw *hw, BOOL bFlag);
BOOL rhine_soft_reset(struct rhine_hw *hw);
void rhine_get_cam_mask(struct rhine_hw* hw, PU32 pMask, RHINE_CAM_TYPE cam_type);
void rhine_set_cam_mask(struct rhine_hw* hw, U32 mask, RHINE_CAM_TYPE cam_type);
void rhine_get_cam(struct rhine_hw *hw, int idx, PU8 addr, RHINE_CAM_TYPE cam_type);
void rhine_set_cam(struct rhine_hw *hw, int idx, PU8 addr, RHINE_CAM_TYPE cam_type);


void mii_set_auto_on(struct rhine_hw *hw);
void mii_set_auto_off(struct rhine_hw *hw);
BOOL rhine_mii_read(struct rhine_hw *hw, U8 byIdx, PU16 pdata);
BOOL rhine_mii_write (struct rhine_hw *hw, BYTE byMiiAddr, WORD wData);
void rhine_disable_mii_auto_poll(struct rhine_hw *hw);
void rhine_enable_mii_auto_poll(struct rhine_hw *hw);
void rhine_get_phy_id(struct rhine_hw *hw);
BOOL rhine_is_full_duplex (struct rhine_hw *hw);
void enable_flow_control_ability(struct rhine_hw *hw);
int rhine_set_media_mode(struct rhine_hw *hw, POPTIONS option);
void rhine_print_link_status(U32 status);
void rhine_set_promiscuous(struct rhine_hw* hw);
void rhine_set_all_multicast(struct rhine_hw* hw);

BOOL rhine_td_own_bit_on(PTX_DESC pTD);
void rhine_set_td_own(PTX_DESC pTD);
void rhine_set_tx_buf_sz(PTX_DESC pTD, U16 size);
U16 rhine_get_tx_buf_sz(PTX_DESC pTD);
BOOL rhine_rd_own_bit_on( PRX_DESC pRD );
void rhine_set_rd_own( PRX_DESC pRD);
U16 rhine_get_rx_frame_length( PRX_DESC pRD);
void rhine_set_rx_buf_sz(PRX_DESC pRD, U16 size);


#define MII_REG_BITS_ON(x,i,hw) do {\
    U16 w;\
    rhine_mii_read((hw),(i),&(w));\
    (w)|=(x);\
    rhine_mii_write((hw),(i),(w));\
} while (0)

#define MII_REG_BITS_OFF(x,i,hw) do {\
    U16 w;\
    rhine_mii_read((hw),(i),&(w));\
    (w)&=(~(x));\
    rhine_mii_write((hw),(i),(w));\
} while (0)

#define MII_REG_BITS_IS_ON(x,i,hw) ({\
    U16 w;\
    rhine_mii_read((hw),(i),&(w));\
    ((BOOL) ((w) & (x)));})



#endif

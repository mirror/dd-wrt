/*
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416reg.h#2 $
 */
#ifndef _DEV_ATH_AR5416REG_H
#define _DEV_ATH_AR5416REG_H

/* Xcode hardcodes the paths */
#ifdef AH5416_SUPPORT_XCODE
#include <ar5212reg.h>
#else
#include "ar5212/ar5212reg.h"
#endif

enum {
    HAL_RESET_POWER_ON,
    HAL_RESET_WARM,
    HAL_RESET_COLD,
};

/* Allows the use of NDIS ini file without editing. The systems group
 * produces changes for the NDIS driver only 
 */
 #define A_UINT32 u_int32_t


#define AR_SREV                 0x4020 // mac silicon rev (expanded from 8 bits to 16 bits for Sowl)
#define AR_SREV_ID              0x000000FF /* Mask to read SREV info */
#define AR_SREV_VERSION         0x000000F0 /* Mask for Chip version */
#define AR_SREV_VERSION_S       4          /* Mask to shift Major Rev Info */
#define AR_SREV_REVISION_N        0x00000007 /* Mask for Chip revision level */
/* Sowl extension to SREV. AR_SREV_ID must be 0xFF */
#define AR_SREV_ID2                 0xFFFFFFFF /* Mask to read SREV info */
#define AR_SREV_VERSION2        	0xFFFC0000 /* Mask for Chip version */
#define AR_SREV_VERSION2_S          18         /* Mask to shift Major Rev Info */
#define AR_SREV_TYPE2        	    0x0003F000 /* Mask for Chip type */
#define AR_SREV_TYPE2_S             12         /* Mask to shift Major Rev Info */
#define AR_SREV_TYPE2_CHAIN		    0x00001000 /* chain mode (1 = 3 chains, 0 = 2 chains) */
#define AR_SREV_TYPE2_HOST_MODE		0x00002000 /* host mode (1 = PCI, 0 = PCIe) */
#define AR_SREV_REVISION2        	0x00000F00
#define AR_SREV_REVISION2_S     	8

#define AR_SREV_VERSION_OWL_PCI     0xD
#define AR_SREV_VERSION_OWL_PCIE    0xC
#define AR_SREV_VERSION_HOWL                  0x014

#define AR_SREV_REVISION_OWL_10     0      /* Owl 1.0 */
#define AR_SREV_REVISION_OWL_20     1      /* Owl 2.0/2.1 */
#define AR_SREV_REVISION_OWL_22     2      /* Owl 2.2 */
#define AR_SREV_VERSION_SOWL        0x1f
#define AR_SREV_REVISION_SOWL_10    0      /* Sowl 1.0 */
#define AR_SREV_REVISION_SOWL_11    1      /* Sowl 1.1 */





#define AR_RTC_BASE             0x00020000

// RTC registers
#define AR_RTC_RC               0x7000     /* reset control */
#define AR_RTC_RC_M             0x00000003
#define AR_RTC_RC_MAC_WARM      0x00000001
#define AR_RTC_RC_MAC_COLD      0x00000002
/* Sowl */
#define AR_RTC_SOWL_PLL_DIV	    0x000003ff
#define AR_RTC_SOWL_PLL_DIV_S   0
#define AR_RTC_SOWL_PLL_REFDIV  0x00003C00
#define AR_RTC_SOWL_PLL_REFDIV_S 10
#define AR_RTC_SOWL_PLL_CLKSEL	0x0000C000
#define AR_RTC_SOWL_PLL_CLKSEL_S 14
/* Owl */
#define AR_RTC_PLL_CONTROL      0x7014
#define AR_RTC_PLL_DIV          0x0000001f
#define AR_RTC_PLL_DIV_S        0
#define AR_RTC_PLL_DIV2         0x00000020
#define AR_RTC_PLL_REFDIV_5     0x000000c0
#define AR_RTC_PLL_CLKSEL_S     8
#define AR_RTC_PLL_CLKSEL       0x00000300

#define AR_RTC_RESET            0x7040      /* reset RTC */


#define AR_RTC_SLEEP_CLK            0x7048
#define AR_RTC_FORCE_DERIVED_CLK    0x2

#define AR_RTC_FORCE_WAKE           0x704c      /* control MAC force wake */
#define AR_RTC_FORCE_WAKE_EN        0x00000001  /* enable force wake */
#define AR_RTC_FORCE_WAKE_ON_INT    0x00000002  /* auto-wake on MAC interrupt */

#define AR_RTC_INTR_CAUSE       0x7050          /* RTC interrupt cause/clear */
#define AR_RTC_INTR_ENABLE      0x7054          /* RTC interrupt enable */
#define AR_RTC_INTR_MASK        0x7058          /* RTC interrupt mask */

#define AR_RTC_DERIVED_CLK      (AR_RTC_BASE + 0x0038)  /* RTC derived clock */
#define AR_RTC_DERIVED_CLK_PERIOD 0x0000fffe
#define AR_RTC_DERIVED_CLK_PERIOD_S 1

#define AR_SREV_OWL_20_OR_LATER(_ah) ((AH_PRIVATE((_ah))->ah_macVersion >= AR_SREV_VERSION_SOWL) || \
				(AH_PRIVATE((_ah))->ah_macRev >= AR_SREV_REVISION_OWL_20))
#define AR_SREV_OWL_22_OR_LATER(_ah) ((AH_PRIVATE((_ah))->ah_macVersion >= AR_SREV_VERSION_SOWL) || \
				(AH_PRIVATE((_ah))->ah_macRev >= AR_SREV_REVISION_OWL_22))

#ifdef AH_SUPPORT_AR9000
#define AR_SREV_SOWL_10_OR_LATER(_ah) ((AH_PRIVATE((_ah))->ah_macVersion >= AR_SREV_VERSION_SOWL))
#define AR_SREV_SOWL(_ah) ((AH_PRIVATE((_ah))->ah_macVersion == AR_SREV_VERSION_SOWL))
#define AR_SREV_SOWL_10(_ah) (AR_SREV_SOWL(_ah) && (AH_PRIVATE((_ah))->ah_macRev == AR_SREV_REVISION_SOWL_10))
#define AR_SREV_SOWL_11(_ah) (AR_SREV_SOWL(_ah) && (AH_PRIVATE((_ah))->ah_macRev == AR_SREV_REVISION_SOWL_11))
#define AR_SREV_HOWL(ah) ((AH_PRIVATE(ah)->ah_macVersion) == AR_SREV_VERSION_HOWL)
#else
#define AR_SREV_SOWL_10_OR_LATER(_ah) (0)
#define AR_SREV_SOWL(_ah) (0)
#define AR_SREV_SOWL_10(_ah) (0)
#define AR_SREV_SOWL_11(_ah) (0)
#define AR_SREV_HOWL(_ah) (0)
#endif

#define ATH_SREV_REV_HOWL_NO_MBSSID_AGGR        0xb0  /* Howl MAC revision that does not support Multiple BSSID Aggregation (Howl 1.1,1.2,1.3)*/
#define ATH_SREV_REV_HOWL_MASK                  0xF8 /* Howl MAC revision MASK to find the Howl 1.1,1.2,1.3 */



/* Test macro for owl 1.0 */
#define IS_5416V1(_ah)  ((_ah)->ah_macRev == AR_SREV_REVISION_OWL_10)  
//#define IS_5416_EMU_EEP(_ah)  ((AH_PRIVATE(_ah)->ah_devid == AR5416_DEVID_EMU_PCI) ||  (AH_PRIVATE(_ah)->ah_devid == AR5416_DEVID_EMU_PCIE))
#define IS_5416_EMU_EEP(_ah)	0

#define AR_SREV_5416_V20_OR_LATER(_ah) (AR_SREV_HOWL((_ah)) || AR_SREV_OWL_20_OR_LATER(_ah))
#define AR_SREV_5416_V22_OR_LATER(_ah)	(AR_SREV_HOWL((_ah)) || AR_SREV_OWL_22_OR_LATER(_ah)) 

#define IS_5416V2(_ah)		AR_SREV_5416_V20_OR_LATER(_ah)
#define IS_5416V2_2(_ah)	AR_SREV_5416_V22_OR_LATER(_ah)


/* Eeprom size */
//#define AR5416_EEPROM_MAX 0x400

/* Additional register definitions for AR5416 */

#define AR_MIRT              0x0020 // Mac Interrupt rate threshold register
#define AR_MIRT_VAL          0x0000ffff // in uS
#define AR_MIRT_VAL_S        16

#define AR_TIMT              0x0028 // Mac Tx Interrupt mitigation threshold
#define AR_TIMT_LAST         0x0000ffff // Last packet threshold
#define AR_TIMT_LAST_S       0
#define AR_TIMT_FIRST        0xffff0000 // First packet threshold
#define AR_TIMT_FIRST_S      16

#define AR_RIMT              0x002C // Mac Rx Interrupt mitigation threshold
#define AR_RIMT_LAST         0x0000ffff // Last packet threshold
#define AR_RIMT_LAST_S       0
#define AR_RIMT_FIRST        0xffff0000 // First packet threshold
#define AR_RIMT_FIRST_S      16

/* Interrupt Mask Registers */
#ifdef AR5416_INT_MITIGATION
#define AR_IMR_TXMINTR       0x00080000 // Maximum interrupt transmit rate
#define AR_IMR_RXMINTR       0x01000000 // Maximum interrupt receive rate
#define AR_IMR_TXINTM        0x40000000 // Tx interrupt after mitigation
#define AR_IMR_RXINTM        0x80000000 // Rx interrupt after mitigation
#endif

#define AR_RXFIFO_CFG          0x8114
#define AR_OBS                 0x4080

/* MAC tx DMA size config  */
#define AR_TXCFG_DMASZ_MASK  0x00000003
#define AR_TXCFG_DMASZ_4B    0
#define AR_TXCFG_DMASZ_8B    1
#define AR_TXCFG_DMASZ_16B   2
#define AR_TXCFG_DMASZ_32B   3
#define AR_TXCFG_DMASZ_64B   4
#define AR_TXCFG_DMASZ_128B  5
#define AR_TXCFG_DMASZ_256B  6
#define AR_TXCFG_DMASZ_512B  7

/* MAC rx DMA size config  */
#define AR_RXCFG_DMASZ_MASK  0x00000007
#define AR_RXCFG_DMASZ_4B    0
#define AR_RXCFG_DMASZ_8B    1
#define AR_RXCFG_DMASZ_16B   2
#define AR_RXCFG_DMASZ_32B   3
#define AR_RXCFG_DMASZ_64B   4
#define AR_RXCFG_DMASZ_128B  5
#define AR_RXCFG_DMASZ_256B  6
#define AR_RXCFG_DMASZ_512B  7

/* MAC Led registers */
#define AR_MAC_LED                  0x1f04 /* LED control */
#define AR_MAC_LED_BLINK_SLOW       0x00000008 /* LED slowest blink rate mode */
#define AR_MAC_LED_BLINK_THRESH_SEL 0x00000070 /* LED blink threshold select */
#define AR_MAC_LED_MODE             0x00000380 /* LED mode select */
#define AR_MAC_LED_MODE_S           7
#define AR_MAC_LED_MODE_PROP        0 /* Blink prop to filtered tx/rx */
#define AR_MAC_LED_MODE_RPROP       1 /* Blink prop to unfiltered tx/rx */
#define AR_MAC_LED_MODE_SPLIT       2 /* Blink power for tx/net for rx */
#define AR_MAC_LED_MODE_RAND        3 /* Blink randomly */
#define AR_MAC_LED_MODE_POWON       5 /* Power LED on (s/w control) */
#define AR_MAC_LED_MODE_NETON       6 /* Network LED on (s/w control) */
#define AR_MAC_LED_ASSOC_S	    10
#define AR_MAC_LED_ASSOC	    0x00000c00
#define AR_MAC_LED_ASSOC_NONE       0x00000000 /* STA is not associated or trying */
#define AR_MAC_LED_ASSOC_ACTIVE     0x00000400 /* STA is associated */
#define AR_MAC_LED_ASSOC_PENDING    0x00000800 /* STA is trying to associate */

// AR9280: rf long shift registers
#define AR_AN_RF2G1_CH0         0x7810
#define AR_AN_RF2G1_CH0_OB      0x03800000
#define AR_AN_RF2G1_CH0_OB_S    23
#define AR_AN_RF2G1_CH0_DB      0x1C000000
#define AR_AN_RF2G1_CH0_DB_S    26

#define AR_AN_RF5G1_CH0         0x7818
#define AR_AN_RF5G1_CH0_OB5     0x00070000
#define AR_AN_RF5G1_CH0_OB5_S   16
#define AR_AN_RF5G1_CH0_DB5     0x00380000
#define AR_AN_RF5G1_CH0_DB5_S   19

#define AR_AN_RF2G1_CH1         0x7834
#define AR_AN_RF2G1_CH1_OB      0x03800000
#define AR_AN_RF2G1_CH1_OB_S    23
#define AR_AN_RF2G1_CH1_DB      0x1C000000
#define AR_AN_RF2G1_CH1_DB_S    26

#define AR_AN_RF5G1_CH1         0x783C
#define AR_AN_RF5G1_CH1_OB5     0x00070000
#define AR_AN_RF5G1_CH1_OB5_S   16
#define AR_AN_RF5G1_CH1_DB5     0x00380000
#define AR_AN_RF5G1_CH1_DB5_S   19

#define AR_AN_TOP2                  0x7894
#define AR_AN_TOP2_XPABIAS_LVL      0xC0000000
#define AR_AN_TOP2_XPABIAS_LVL_S    30
#define AR_AN_TOP2_LOCALBIAS        0x00200000
#define AR_AN_TOP2_LOCALBIAS_S      21
#define AR_AN_TOP2_PWDCLKIND        0x00400000
#define AR_AN_TOP2_PWDCLKIND_S      22

#define AR_AN_SYNTH9            0x7868
#define AR_AN_SYNTH9_REFDIVA    0xf8000000
#define AR_AN_SYNTH9_REFDIVA_S  27



/* MAC PCU Registers */
#define AR_STA_ID1_PRESERVE_SEQNUM 0x20000000 /* Don't replace seq num */
#define AR_PCU_TXBUF_CTRL               0x8340
#define AR_PCU_TXBUF_CTRL_SIZE_MASK     0x7FF
#define AR_PCU_TXBUF_CTRL_USABLE_SIZE   0x700

/*Extended PCU DIAG_SW control fields */
#define AR_DIAG_DUAL_CHAIN_INFO     0x01000000 // dual chain channel info
#define AR_DIAG_RX_ABORT            0x02000000 //  abort rx
#define AR_DIAG_SATURATE_CYCLE_CNT  0x04000000 // saturate cycle cnts (no shift)
#define AR_DIAG_OBS_PT_SEL2         0x08000000 // Mask for observation point sel
#define AR_DIAG_RX_CLEAR_CTL_LOW    0x10000000 // force rx_clear (ctl) low (i.e. busy)
#define AR_DIAG_RX_CLEAR_EXT_LOW    0x20000000 // force rx_clear (ext) low (i.e. busy)

#define AR_AHB_MODE             0x4024 // ahb mode for dma
#define AR_AHB_EXACT_WR_EN      0x00000000 // write exact bytes
#define AR_AHB_BUF_WR_EN        0x00000001 // buffer write upto cacheline
#define AR_AHB_EXACT_RD_EN      0x00000000 // read exact bytes
#define AR_AHB_CACHELINE_RD_EN  0x00000002 // read upto end of cacheline
#define AR_AHB_PREFETCH_RD_EN   0x00000004 // prefetch upto page boundary
#define AR_AHB_PAGE_SIZE_1K     0x00000000 // set page-size as 1k
#define AR_AHB_PAGE_SIZE_2K     0x00000008 // set page-size as 2k
#define AR_AHB_PAGE_SIZE_4K     0x00000010 // set page-size as 4k

#define AR_RAD5133_SREV_MAJOR  0xc0 /* Fowl: 2+5G/3x3 */
#define AR_RAD2133_SREV_MAJOR  0xd0 /* Fowl: 2G/3x3   */
#define AR_RAD5122_SREV_MAJOR  0xe0 /* Fowl: 5G/2x2   */
#define AR_RAD2122_SREV_MAJOR  0xf0 /* Fowl: 2+5G/2x2 */

#define AR_TXOP_X          0x81ec                // txop for legacy non-qos
#define AR_TXOP_X_VAL      0x000000FF

/* Beacon registers */

/* TSF Registers */
#define AR_RESET_TSF        0x8020
#define AR_RESET_TSF_ONCE   0x01000000 // reset tsf once ; self-clears bit

/* Sleep Registers */
#define AR_SLP32_MODE                  0x8244
#define AR_SLP32_HALF_CLK_LATENCY      0x000FFFFF    // rising <-> falling edge
#define AR_SLP32_ENA                   0x00100000
#define AR_SLP32_TSF_WRITE_STATUS      0x00200000    // tsf update in progress

#define AR_SLP32_WAKE              0x8248
#define AR_SLP32_WAKE_XTL_TIME     0x0000FFFF    // time to wake crystal

#define AR_SLP32_INC               0x824c
#define AR_SLP32_TST_INC           0x000FFFFF

#define AR_SLP_CNT         0x8250    // 32kHz cycles for which mac is asleep
#define AR_SLP_CYCLE_CNT   0x8254    // absolute number of 32kHz cycles

#define AR_SLP_MIB_CTRL    0x8258
#define AR_SLP_MIB_CLEAR   0x00000001    // clear pending
#define AR_SLP_MIB_PENDING 0x00000002    // clear counters

/* on-demand subfields */
#define AR_TXOP_0_3    0x81f0                    // txop for various tid's
#define AR_TXOP_4_7    0x81f4
#define AR_TXOP_8_11   0x81f8
#define AR_TXOP_12_15  0x81fc

/* Interrupt Mask Registers */
#define AR_IMR_S2_CST          0x00400000 /* Carrier sense timeout */
#define AR_IMR_S2_GTT          0x00800000 /* Global transmit timeout */

/* DMA & PCI Registers in PCI space (usable during sleep)*/
#define AR_RC_AHB            0x00000001 // ahb reset
#define AR_RC_APB            0x00000002 // apb reset
#define AR_RC_HOSTIF         0x00000100 // host interface reset

/* RTC registers */
#define AR_RTC_RC               0x7000     /* reset control */
#define AR_RTC_RC_M             0x00000003
#define AR_RTC_RC_MAC_WARM      0x00000001
#define AR_RTC_RC_MAC_COLD      0x00000002
#define AR_RTC_PLL_CONTROL      0x7014
#define AR_RTC_PLL_DIV          0x0000001f
#define AR_RTC_PLL_DIV_S        0
#define AR_RTC_PLL_DIV2         0x00000020
#define AR_RTC_PLL_REFDIV_5     0x000000c0

#define AR_RTC_RESET            0x7040      /* RTC reset register */
#define AR_RTC_RESET_EN         0x00000001  /* Reset RTC bit */

#define AR_RTC_STATUS           0x7044      /* system sleep status */
#define AR_RTC_PM_STATUS_M	0x0000000f  /* Pwr Mgmt Status is the last 4 bits */
#define AR_RTC_STATUS_M		0x0000003f  /* RTC Status is the last 6 bits */
#define AR_RTC_STATUS_SHUTDOWN  0x00000001
#define AR_RTC_STATUS_ON        0x00000002
#define AR_RTC_STATUS_SLEEP     0x00000004
#define AR_RTC_STATUS_WAKEUP    0x00000008
#define AR_RTC_STATUS_COLDRESET	0x00000010 /* Not currently used */
#define AR_RTC_STATUS_PLLCHANGE	0x00000020 /* Not currently used */

#define AR_RTC_SLEEP_CLK            0x7048
#define AR_RTC_FORCE_DERIVED_CLK    0x2

#define AR_RTC_FORCE_WAKE           0x704c      /* control MAC force wake */
#define AR_RTC_FORCE_WAKE_EN        0x00000001  /* enable force wake */
#define AR_RTC_FORCE_WAKE_ON_INT    0x00000002  /* auto-wake on MAC interrupt */

#define AR_RTC_INTR_CAUSE       0x7050          /* RTC interrupt cause/clear */
#define AR_RTC_INTR_ENABLE      0x7054          /* RTC interrupt enable */
#define AR_RTC_INTR_MASK        0x7058          /* RTC interrupt mask */

#define AR_RTC_PLL_CLKSEL	0x00000300
#define AR_RTC_PLL_CLKSEL_S	8


#define AR_OBS_BUS_CTRL     0x8068  // select a bus for observation
#define AR_OBS_BUS_SEL_1    0x00040000
#define AR_OBS_BUS_SEL_2    0x00080000
#define AR_OBS_BUS_SEL_3    0x000C0000
#define AR_OBS_BUS_SEL_4    0x08040000
#define AR_OBS_BUS_SEL_5    0x08080000

#define AR_OBS_BUS_1               0x806c // mac debug observation bus
#define AR_OBS_BUS_1_PCU           0x00000001
#define AR_OBS_BUS_1_RX_END        0x00000002
#define AR_OBS_BUS_1_RX_WEP        0x00000004
#define AR_OBS_BUS_1_RX_BEACON     0x00000008
#define AR_OBS_BUS_1_RX_FILTER     0x00000010
#define AR_OBS_BUS_1_TX_HCF        0x00000020
#define AR_OBS_BUS_1_QUIET_TIME    0x00000040
#define AR_OBS_BUS_1_CHAN_IDLE     0x00000080
#define AR_OBS_BUS_1_TX_HOLD       0x00000100
#define AR_OBS_BUS_1_TX_FRAME      0x00000200
#define AR_OBS_BUS_1_RX_FRAME      0x00000400
#define AR_OBS_BUS_1_RX_CLEAR      0x00000800
#define AR_OBS_BUS_1_WEP_STATE     0x0003F000
#define AR_OBS_BUS_1_WEP_STATE_S   12
#define AR_OBS_BUS_1_RX_STATE      0x01F00000
#define AR_OBS_BUS_1_RX_STATE_S    20
#define AR_OBS_BUS_1_TX_STATE      0x7E000000
#define AR_OBS_BUS_1_TX_STATE_S    25


/* Sleep control */
#define AR5416_SLEEP1_CAB_TIMEOUT   0xFFE00000 // Cab timeout(TU) mask
#define AR5416_SLEEP1_CAB_TIMEOUT_S 22         // Cab timeout(TU) shift

#define AR5416_SLEEP2_BEACON_TIMEOUT    0xFFE00000 // Beacon timeout(TU) mask
#define AR5416_SLEEP2_BEACON_TIMEOUT_S  22         // Beacon timeout(TU) shift

/* generic timers based on tsf - all uS */
#define AR_NEXT_TBTT_TIMER                  0x8200
#define AR_NEXT_DMA_BEACON_ALERT            0x8204
#define AR_NEXT_SWBA                        0x8208
#define AR_NEXT_CFP                         0x8208
#define AR_NEXT_HCF                         0x820C
#define AR_NEXT_TIM                         0x8210
#define AR_NEXT_DTIM                        0x8214
#define AR_NEXT_QUIET_TIMER                 0x8218
#define AR_NEXT_NDP_TIMER                   0x821C

#define AR5416_BEACON_PERIOD                    0x8220
#define AR_DMA_BEACON_PERIOD                0x8224
#define AR_SWBA_PERIOD                      0x8228
#define AR_HCF_PERIOD                       0x822C
#define AR_TIM_PERIOD                       0x8230
#define AR_DTIM_PERIOD                      0x8234
#define AR_QUIET_PERIOD                     0x8238
#define AR_NDP_PERIOD                       0x823C

#define AR_TIMER_MODE                       0x8240
#define AR_TBTT_TIMER_EN                    0x00000001
#define AR_DBA_TIMER_EN                     0x00000002
#define AR_SWBA_TIMER_EN                    0x00000004
#define AR_HCF_TIMER_EN                     0x00000008
#define AR_TIM_TIMER_EN                     0x00000010
#define AR_DTIM_TIMER_EN                    0x00000020
#define AR_QUIET_TIMER_EN                   0x00000040
#define AR_NDP_TIMER_EN                     0x00000080
#define AR_TIMER_OVERFLOW_INDEX             0x00000700
#define AR_TIMER_OVERFLOW_INDEX_S           8
#define AR_TIMER_THRESH                     0xFFFFF000
#define AR_TIMER_THRESH_S                   12

/* PCU Misc modes */
#define AR_PCU_FORCE_BSSID_MATCH   0x00000001    // force bssid to match
#define AR_PCU_MIC_NEW_LOC_ENA     0x00000004    // tx/rx mic key are together
#define AR_PCU_TX_ADD_TSF          0x00000008    // add tx_tsf + int_tsf
#define AR_PCU_CCK_SIFS_MODE       0x00000010    // assume 11b sifs programmed
#define AR_PCU_RX_ANT_UPDT         0x00000800    // KC_RX_ANT_UPDATE
#define AR_PCU_TXOP_TBTT_LIMIT_ENA 0x00001000    // enforce txop / tbtt
#define AR_PCU_MISS_BCN_IN_SLEEP   0x00004000    // count bmiss's when sleeping
#define AR_PCU_BUG_12306_FIX_ENA   0x00020000    // use rx_clear to count sifs
#define AR_PCU_FORCE_QUIET_COLL    0x00040000    // kill xmit for channel change
#define AR_PCU_TBTT_PROTECT        0x00200000    // no xmit upto tbtt + 20 uS
#define AR_PCU_CLEAR_VMF           0x01000000    // clear vmf mode (fast cc)
#define AR_PCU_CLEAR_BA_VALID      0x04000000    // clear ba state

/* rx and cal chain masks */
#define AR_SELFGEN_MASK         0x832c

#define AR_GTXTO	0x0064 // MAC global transmit timeout
#define AR_GTXTO_TIMEOUT_COUNTER    0x0000FFFF  // Mask for timeout counter (in TUs)
#define AR_GTXTO_TIMEOUT_LIMIT      0xFFFF0000  // Mask for timeout limit (in  TUs)
#define AR_GTXTO_TIMEOUT_LIMIT_S    16      // Shift for timeout limit

#define AR_GTTM		0x0068 // MAC global transmit timeout mode
#define AR_GTTM_USEC          0x00000001 // usec strobe
#define AR_GTTM_IGNORE_IDLE   0x00000002 // ignore channel idle
#define AR_GTTM_RESET_IDLE    0x00000004 // reset counter on channel idle low
#define AR_GTTM_CST_USEC      0x00000008 // CST usec strobe

#define AR_CST         0x006C // MAC carrier sense timeout
#define AR_CST_TIMEOUT_COUNTER    0x0000FFFF  // Mask for timeout counter (in TUs)
#define AR_CST_TIMEOUT_LIMIT      0xFFFF0000  // Mask for timeout limit (in  TUs)
#define AR_CST_TIMEOUT_LIMIT_S    16      // Shift for timeout limit

/* Interrupts */
#define AR_INTR_SPURIOUS 	    	0xffffffff
#define AR_ISR_S2_CST			0x00400000 // Carrier sense timeout
#define AR_ISR_S2_GTT          		0x00800000 // Global transmit timeout
#define AR_INTR_RTC_IRQ             	0x00000001 // rtc in shutdown state
#define AR_INTR_MAC_IRQ             	0x00000002 // pending mac interrupt
#define AR_INTR_EEP_PROT_ACCESS     	0x00000004 // eeprom protected area access
#define AR_INTR_MAC_AWAKE           	0x00020000 // mac is awake
#define AR_INTR_MAC_ASLEEP          	0x00040000 // mac is asleep
#define AR_INTR_SYNC_CAUSE_CLR  	0x4028 // clear interrupt
#define AR_INTR_SYNC_CAUSE      	0x4028 // check pending interrupts
#define AR_INTR_SYNC_ENABLE     	0x402c // enable interrupts
#define AR_INTR_ASYNC_MASK      	0x4030 // asynchronous interrupt mask
#define AR_INTR_SYNC_MASK       	0x4034 // synchronous interrupt mask
#define AR_INTR_ASYNC_CAUSE     	0x4038 // check pending interrupts
#define AR_INTR_ASYNC_ENABLE    	0x403c // enable interrupts

/* Encryption Keytable */
#define	AR_KEYTABLE_0		0x8800	/* MAC Key Cache */
#define	AR_KEYTABLE(_n)		(AR_KEYTABLE_0 + ((_n)*32))
#define	AR_KEYTABLE_TYPE(_n)	(AR_KEYTABLE(_n) + 20)	/* key type */
#define	AR_KEYTABLE_TYPE_40	0x00000000	/* WEP 40 bit key */
#define	AR_KEYTABLE_TYPE_104	0x00000001	/* WEP 104 bit key */
#define	AR_KEYTABLE_TYPE_128	0x00000003	/* WEP 128 bit key */
#define	AR_KEYTABLE_TYPE_TKIP	0x00000004	/* TKIP and Michael */
#define	AR_KEYTABLE_TYPE_AES	0x00000005	/* AES/OCB 128 bit key */
#define	AR_KEYTABLE_TYPE_CCM	0x00000006	/* AES/CCM 128 bit key */
#define	AR_KEYTABLE_TYPE_CLR	0x00000007	/* no encryption */

/* GPIO Interrupt */
#define AR_INTR_GPIO                	0x3FF00000 // gpio interrupted
#define AR_INTR_GPIO_S              	20
/* GPIOs */
#define AR_GPIO_IN           0x4048 // GPIO input register

#define AR_GPIO_INTR_OUT     0x404c // GPIO output register
#define AR_GPIO_OUT_CTRL     0x000003FF // 0 = out, 1 = in
#define AR_GPIO_OUT_VAL      0x000FFC00
#define AR_GPIO_OUT_VAL_S    10
#define AR_GPIO_INTR_CTRL    0x3FF00000
#define AR_GPIO_INTR_CTRL_S  20

#define AR928X_GPIO_IN_VAL                       0x000FFC00
#define AR928X_GPIO_IN_VAL_S                     10

#define AR_2040_MODE                0x8318
#define AR_2040_JOINED_RX_CLEAR 0x00000001   // use ctl + ext rx_clear for cca

/* Additional cycle counter. See also AR_CCCNT */
#define AR_EXTRCCNT         0x8328   // extension channel rx clear count
                     // counts number of cycles rx_clear (ext) is low (i.e. busy)
                     // when the MAC is not actively transmitting/receiving

#define AR_SELFGEN_MASK         0x832c

/* Eeprom defines */
#define AR_EEPROM_STATUS_DATA               0x407c
#define AR_EEPROM_STATUS_DATA_VAL           0x0000ffff
#define AR_EEPROM_STATUS_DATA_VAL_S         0
#define AR_EEPROM_STATUS_DATA_BUSY          0x00010000
#define AR_EEPROM_STATUS_DATA_BUSY_ACCESS   0x00020000
#define AR_EEPROM_STATUS_DATA_PROT_ACCESS   0x00040000
#define AR_EEPROM_STATUS_DATA_ABSENT_ACCESS 0x00080000

/* PCIe defines */
#define AR5416_PCIE_SERDES		0x4040
#define AR5416_PCIE_SERDES2		0x4044
#define AR5416_PCIE_PM_CTRL		0x4014

/*
 * synchronous interrupt signals
 */
enum {
    AR_INTR_SYNC_RTC_IRQ                = 0x00000001,
    AR_INTR_SYNC_MAC_IRQ                = 0x00000002,
    AR_INTR_SYNC_EEPROM_ILLEGAL_ACCESS  = 0x00000004,
    AR_INTR_SYNC_APB_TIMEOUT            = 0x00000008,
    AR_INTR_SYNC_PCI_MODE_CONFLICT      = 0x00000010,
    AR_INTR_SYNC_HOST1_FATAL            = 0x00000020,
    AR_INTR_SYNC_HOST1_PERR             = 0x00000040,
    AR_INTR_SYNC_TRCV_FIFO_PERR         = 0x00000080,
    AR_INTR_SYNC_RADM_CPL_EP            = 0x00000100,
    AR_INTR_SYNC_RADM_CPL_DLLP_ABORT    = 0x00000200,
    AR_INTR_SYNC_RADM_CPL_DLP_ABORT     = 0x00000400,
    AR_INTR_SYNC_RADM_CPL_ECRC_ERR      = 0x00000800,
    AR_INTR_SYNC_RADM_CPL_TIMEOUT       = 0x00001000,
    AR_INTR_SYNC_LOCAL_TIMEOUT          = 0x00002000,
    AR_INTR_SYNC_PM_ACCESS              = 0x00004000,
    AR_INTR_SYNC_MAC_AWAKE              = 0x00008000,
    AR_INTR_SYNC_MAC_ASLEEP             = 0x00010000,
    AR_INTR_SYNC_MAC_SLEEP_ACCESS       = 0x00020000,
    AR_INTR_SYNC_ALL                    = 0x0003FFFF,
};

#ifdef AH_SUPPORT_AR5416FPGA
/* XXX - AR5416 Emulation only
 * XXX - TODO - remove when emulation complete
 */
#define AR_EMU              0x0070 // MAC - special emulation only register
#define AR_EMU_RATETHROT    0x00000001 // rate throttling (enabled = 1)
#define AR_EMU_CTL          0x00000002 // ctl channel busy (busy = 1)
#define AR_EMU_EXT          0x00000004 // ext channel busy (busy = 1)
#define AR_EMU_HALF_RATE    0x00000080 // run at half-rate for encryption
#define AR_EMU_VERSION      0xFFFFFF00 // Mask for version (read only)
#define AR_EMU_VERSION_S    8      // Shift for timeout limit

#endif /* AH_SUPPORT_AR5416FPGA */

#endif /* _DEV_ATH_AR5416REG_H */

//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-12-03
// Purpose:      
// Description:  Hardware definitions for AR5312/AR2313 WiSoC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * ar531xreg.h Register definitions for Atheros AR5311 and AR5312 chipsets.
 *   - WLAN registers are listed in
 *         hal/ar5211/ar5211Reg.h
 *         hal/ar5212/ar5212Reg.h
 *   - Ethernet registers are listed in ar531xenet.h
 *   - Standard UART is 16550 compatible.
 */

#ifndef _AR531XREG_H
#define _AR531XREG_H

#ident "ACI $Header: //depot/sw/releases/linuxsrc/src/redboot_cobra/patches/ecos/ecos-2.0/packages/hal/mips/ar5312/current/include/ar531xreg.h#3 $"

/*
 * Address map
 */
#define AR531X_SDRAM0   0x00000000
#define AR531X_SDRAM1   0x08000000
#define AR531X_WLAN0    0xb8000000
#define AR531X_ENET0    0xb8100000
#define AR531X_ENET1    0xb8200000
#define AR531X_SDRAMCTL 0xb8300000
#define AR531X_FLASHCTL 0xb8400000
#define AR531X_WLAN1    0xb8500000              /* (ar5212) */
#define AR531X_APBBASE  0xbc000000
#define AR531X_FLASH0   0x1e000000
#define AR531X_FLASHBT  0x1fc00000              /* ro boot alias to FLASH0 */
#define AR531X_FLASH1   AR531X_SDRAM1           /* board stuff option */
#define AR531X_FLASH2   0x10000000

/*
 * ARM SDRAM Controller
 *   - No read or write buffers are included.
 */
#define AR531X_MEM_CFG0 (AR531X_SDRAMCTL + 0x00)
#define AR531X_MEM_CFG1 (AR531X_SDRAMCTL + 0x04)
#define AR531X_MEM_REF  (AR531X_SDRAMCTL + 0x08)        /* 16 bit value */

#define MEM_CFG0_F0     0x00000002      /* bank 0: 256Mb support */
#define MEM_CFG0_T0     0x00000004      /* bank 0: chip width */
#define MEM_CFG0_B0     0x00000008      /* bank 0: 2 vs 4 bank */
#define MEM_CFG0_F1     0x00000020      /* bank 1: 256Mb support */
#define MEM_CFG0_T1     0x00000040      /* bank 1: chip width */
#define MEM_CFG0_B1     0x00000080      /* bank 1: 2 vs 4 bank */
                                        /* bank 2 and 3 are not supported */
#define MEM_CFG0_E      0x00020000      /* SDRAM clock control */
#define MEM_CFG0_C      0x00040000      /* SDRAM clock enable */
#define MEM_CFG0_X      0x00080000      /* bus width (0 == 32b) */
#define MEM_CFG0_CAS    0x00300000      /* CAS latency (1-3) */
#define MEM_CFG0_C1     0x00100000
#define MEM_CFG0_C2     0x00200000
#define MEM_CFG0_C3     0x00300000
#define MEM_CFG0_R      0x00c00000      /* RAS to CAS latency (1-3) */
#define MEM_CFG0_R1     0x00400000
#define MEM_CFG0_R2     0x00800000
#define MEM_CFG0_R3     0x00c00000
#define MEM_CFG0_A      0x01000000      /* AHB auto pre-charge */

#define MEM_CFG1_I      0x00000001      /* memory init control */
#define MEM_CFG1_M      0x00000002      /* memory init control */
#define MEM_CFG1_R      0x00000004      /* read buffer enable (unused) */
#define MEM_CFG1_W      0x00000008      /* write buffer enable (unused) */
#define MEM_CFG1_B      0x00000010      /* SDRAM engine busy */
#define MEM_CFG1_AC0    0x00000700      /* bank 0: SDRAM addr check (added) */
#define MEM_CFG1_AC_2   0               /* AC of 2MB */
#define MEM_CFG1_AC_4   1               /* AC of 4MB */
#define MEM_CFG1_AC_8   2               /* AC of 8MB */
#define MEM_CFG1_AC_16  3               /* AC of 16MB */
#define MEM_CFG1_AC_32  4               /* AC of 32MB */
#define MEM_CFG1_AC_64  5               /* AC of 64MB */
#define MEM_CFG1_AC_128 6               /* AC of 128MB */
#define MEM_CFG1_AC0_S  8
#define MEM_CFG1_E0     0x00000800      /* bank 0: enable */
#define MEM_CFG1_AC1    0x00007000      /* bank 1: SDRAM addr check (added) */
#define MEM_CFG1_AC1_S  12
#define MEM_CFG1_E1     0x00008000      /* bank 1: enable */

/*
 * ARM Flash Controller
 *   - supports 3 flash banks with either x8 or x16 devices.
 */
#define AR531X_FLASHCTL0        (AR531X_FLASHCTL + 0x00)
#define AR531X_FLASHCTL1        (AR531X_FLASHCTL + 0x04)
#define AR531X_FLASHCTL2        (AR531X_FLASHCTL + 0x08)

#define FLASHCTL_IDCY   0x0000000f      /* Idle cycle turn around time */
#define FLASHCTL_IDCY_S 0
#define FLASHCTL_WST1   0x000003e0      /* Wait state 1 */
#define FLASHCTL_WST1_S 5
#define FLASHCTL_RBLE   0x00000400      /* Read byte lane enable */
#define FLASHCTL_WST2   0x0000f800      /* Wait state 2 */
#define FLASHCTL_WST2_S 11
#define FLASHCTL_AC     0x00070000      /* Flash address check (added) */
#define FLASHCTL_AC_S   16
#define FLASHCTL_AC_128K 0x00000000
#define FLASHCTL_AC_256K 0x00010000
#define FLASHCTL_AC_512K 0x00020000
#define FLASHCTL_AC_1M   0x00030000
#define FLASHCTL_AC_2M   0x00040000
#define FLASHCTL_AC_4M   0x00050000
#define FLASHCTL_AC_8M   0x00060000
#define FLASHCTL_AC_RES  0x00070000     /* 16MB is not supported */
#define FLASHCTL_E      0x00080000      /* Flash bank enable (added) */
#define FLASHCTL_BUSERR 0x01000000      /* Bus transfer error status flag */
#define FLASHCTL_WPERR  0x02000000      /* Write protect error status flag */
#define FLASHCTL_WP     0x04000000      /* Write protect */
#define FLASHCTL_BM     0x08000000      /* Burst mode */
#define FLASHCTL_MW     0x30000000      /* Memory width */
#define FLASHCTL_MWx8   0x00000000      /* Memory width x8 */
#define FLASHCTL_MWx16  0x10000000      /* Memory width x16 */
#define FLASHCTL_MWx32  0x20000000      /* Memory width x32 (not supported) */
#define FLASHCTL_ATNR   0x00000000      /* Access type == no retry */
#define FLASHCTL_ATR    0x80000000      /* Access type == retry every */
#define FLASHCTL_ATR4   0xc0000000      /* Access type == retry every 4 */

/*
 * APB Address Map
 */
#define AR531X_UART0    (AR531X_APBBASE + 0x0003)       /* high speed uart */
#define AR531X_UART1    (AR531X_APBBASE + 0x1000)       /* ar531x only */
#define AR531X_GPIO     (AR531X_APBBASE + 0x2000)
#define AR531X_RESETTMR (AR531X_APBBASE + 0x3000)
#define AR531X_APB2AHB  (AR531X_APBBASE + 0x4000)

#define AR531X_GPIO_DO      (AR531X_GPIO + 0x00)        /* backwards */
#define AR531X_GPIO_DI      (AR531X_GPIO + 0x04)
#define AR531X_GPIO_CR      (AR531X_GPIO + 0x08)

#define AR531X_NUM_GPIO 8
#define GPIO_CR_M(x)    (1 << (x))                      /* mask for i/o */
#define GPIO_CR_O(x)    (0 << (x))                      /* output */
#define GPIO_CR_I(x)    (1 << (x))                      /* input */
#define GPIO_CR_INT(x)  (1 << ((x)+8))                  /* interrupt enable */
#define GPIO_CR_UART(x) (1 << ((x)+16))                 /* uart multiplex */

/*
 * Timers
 */
#define AR531X_TIMER    (AR531X_RESETTMR + 0x0000)      /* count down timer */
#define AR531X_RELOAD   (AR531X_RESETTMR + 0x0004)      /* count reload value */
#define AR531X_WDC      (AR531X_RESETTMR + 0x0008)
#define AR531X_WD       (AR531X_RESETTMR + 0x000c)

#define WDC_RESET       0x00000002                      /* reset on watchdog */
#define WDC_NMI         0x00000001                      /* NMI on watchdog */

/*
 * APB Interrupt control
 */
#define AR531X_ISR      (AR531X_RESETTMR + 0x0010)
#define AR531X_IMR      (AR531X_RESETTMR + 0x0014)
#define AR531X_GISR     (AR531X_RESETTMR + 0x0018)      /* global intr status */

#define ISR_TIMER       0x0001
#define ISR_AHBPROC     0x0002                  /* AHB proc error */
#define ISR_AHBDMA      0x0004                  /* AHB DMA error */
#define ISR_GPIO        0x0008
#define ISR_UART0       0x0010                  /* high speed UART */
#define ISR_UART0DMA    0x0020                  /* UART DMA (ar5312) */
#define ISR_WD          0x0040                  /* watchdog */
#define ISR_LOCAL       0x0080                  /* local interrupt */

#define IMR_TIMER       ISR_TIMER
#define IMR_AHBPROC     ISR_AHBPROC
#define IMR_AHBDMA      ISR_AHBDMA
#define IMR_GPIO        ISR_GPIO
#define IMR_UART0       ISR_UART0
#define IMR_UART0DMA    ISR_UART0DMA            /* (ar5312) */
#define IMR_WD          ISR_WD
#define IMR_LOCAL       0x0080

#define GISR_WLAN0      0x0001
#define GISR_ENET0      0x0002
#define GISR_ENET1      0x0004
#define GISR_WLAN1      0x0008                  /* (ar5212) */
#define GISR_MISC       0x0010
#define GISR_TIMER      0x0020

/*
 * UART0 DMA engine
 */
#define AR531X_TXADDR0  (AR531X_APB2AHB + 0x0000)
#define AR531X_TXADDR1  (AR531X_APB2AHB + 0x0004)
#define AR531X_TXCTRL0  (AR531X_APB2AHB + 0x0008)
#define AR531X_TXCTRL1  (AR531X_APB2AHB + 0x000c)
#define AR531X_TXSTAT0  (AR531X_APB2AHB + 0x0010)
#define AR531X_TXSTAT1  (AR531X_APB2AHB + 0x0014)
#define AR531X_RXADDR0  (AR531X_APB2AHB + 0x0020)
#define AR531X_RXADDR1  (AR531X_APB2AHB + 0x0024)
#define AR531X_RXCTRL0  (AR531X_APB2AHB + 0x0028)
#define AR531X_RXCTRL1  (AR531X_APB2AHB + 0x002c)
#define AR531X_RXSTAT0  (AR531X_APB2AHB + 0x0030)
#define AR531X_RXSTAT1  (AR531X_APB2AHB + 0x0034)
#define AR531X_TIMEOUT  (AR531X_APB2AHB + 0x0038)
#define AR531X_DMAISR   (AR531X_APB2AHB + 0x0040)
#define AR531X_DMAIMR   (AR531X_APB2AHB + 0x0044)

#define TXCTRL_SIZE     0x00ffff
#define TXCTRL_SIZE_S   0
#define TXCTRL_OWN      0x010000
#define TXCTRL_READY    0x020000

#define TXSTAT_DONE     0x000001
#define TXSTAT_OK       0x000002
#define TXSTAT_AHBERR   0x000004
#define TXSTAT_COUNT    0x0ffff0
#define TXSTAT_COUNT_S  4

#define RXCTRL_SIZE     0x00ffff
#define RXCTRL_SIZE_S   0
#define RXCTRL_OWN      0x010000
#define RXCTRL_READY    0x020000

#define RXSTAT_DONE     0x000001
#define RXSTAT_OK       0x000002
#define RXSTAT_AHBERR   0x000004
#define RXSTAT_TIMEOUT  0x000008
#define RXSTAT_COUNT    0x0ffff0
#define RXSTAT_COUNT_S  4

#define DMAISR_TXOK     0x0001
#define DMAISR_TXERR    0x0002
#define DMAISR_RXOK     0x0004
#define DMAISR_RXERR    0x0008
#define DMAISR_RXTIM    0x0010

#define DMAIMR_TXOK     DMAISR_TXOK
#define DMAIMR_TXERR    DMAISR_TXERR
#define DMAIMR_RXOK     DMAISR_RXOK
#define DMAIMR_RXERR    DMAISR_RXERR
#define DMAIMR_RXTIM    DMAISR_RXTIM

/*
 * Reset Register
 */
#define AR531X_RESET    (AR531X_RESETTMR + 0x0020)
#define RESET_SYSTEM         0x00000001      /* cold reset full system */
#define RESET_PROC           0x00000002      /* cold reset MIPS core */
#define RESET_WLAN0          0x00000004      /* cold reset WLAN MAC and BB */
#define RESET_EPHY0          0x00000008      /* cold reset ENET0 phy */
#define RESET_EPHY1          0x00000010      /* cold reset ENET1 phy */
#define RESET_ENET0          0x00000020      /* cold reset ENET0 mac */
#define RESET_ENET1          0x00000040      /* cold reset ENET1 mac */
#define RESET_UART0          0x00000100      /* cold reset UART0 (high speed) */

#define RESET_WLAN1          0x00000200      /* cold reset WLAN MAC/BB */

#define RESET_APB            0x00000400      /* cold reset APB (ar5312) */
#define RESET_WARM_PROC      0x00001000      /* warm reset MIPS core */
#define RESET_WARM_WLAN0_MAC 0x00002000      /* warm reset WLAN0 MAC */
#define RESET_WARM_WLAN0_BB  0x00004000      /* warm reset WLAN0 BaseBand */
#define RESET_NMI            0x00010000      /* send an NMI to the processor */
#define RESET_WARM_WLAN1_MAC 0x00020000      /* warm reset WLAN1 mac */
#define RESET_WARM_WLAN1_BB  0x00040000      /* warm reset WLAN1 baseband */
#define RESET_LOCAL_BUS      0x00080000      /* reset local bus */
#define RESET_WDOG           0x00100000      /* last reset was a watchdog */

/*
 * Config Register
 */
#define AR531X_CONFIG   (AR531X_RESETTMR + 0x0030)
#define CONFIG_DEBUG    0x0008          /* Enable debug performance counters */
#define CONFIG_MERGE    0x0004          /* Enable SysAD merge */
#define CONFIG_WLANSWAP 0x0002          /* DMA data is swapped to WLAN(s) */
#define CONFIG_BIG      0x0001          /* big endian (default) */

/*
 * Interface Debug
 */
#define AR531X_FLASHDBG (AR531X_RESETTMR + 0x0040)
#define AR531X_MIIDBG   (AR531X_RESETTMR + 0x0044)

/*
 * CPU Performance Counters
 */
#define AR531X_PERFMSK0 (AR531X_RESETTMR + 0x0050)
#define AR531X_PERFMSK1 (AR531X_RESETTMR + 0x0054)
#define AR531X_PERFCNT0 (AR531X_RESETTMR + 0x0058)
#define AR531X_PERFCNT1 (AR531X_RESETTMR + 0x005c)

#define PERF_DATAHIT    0x0001  /* Count Data Cache Hits */
#define PERF_DATAMISS   0x0002  /* Count Data Cache Misses */
#define PERF_INSTHIT    0x0004  /* Count Instruction Cache Hits */
#define PERF_INSTMISS   0x0008  /* Count Instruction Cache Misses */
#define PERF_ACTIVE     0x0010  /* Count Active Processor Cycles */
#define PERF_WBHIT      0x0020  /* Count CPU Write Buffer Hits */
#define PERF_WBMISS     0x0040  /* Count CPU Write Buffer Misses */

#define PERF_EB_ARDY    0x0001  /* Count EB_ARdy signal */
#define PERF_EB_AVALID  0x0002  /* Count EB_AValid signal */
#define PERF_EB_WDRDY   0x0004  /* Count EB_WDRdy signal */
#define PERF_EB_RDVAL   0x0008  /* Count EB_RdVal signal */
#define PERF_VRADDR     0x0010  /* Count valid read address cycles */
#define PERF_VWADDR     0x0020  /* Count valid write address cycles */
#define PERF_VWDATA     0x0040  /* Count valid write data cycles */

/*
 * Clock Control
 * AR5311 supports just AR531X_CLOCKCTL.
 * AR5312 supports AR531X_CLOCKCTL, AR531X_CLOCKCTL1, and AR531X_CLOCKCTL2.
 */
#define AR531X_CLOCKCTL  (AR531X_RESETTMR + 0x0060)
#define AR531X_CLOCKCTL1 (AR531X_RESETTMR + 0x0064)
#define AR531X_CLOCKCTL2 (AR531X_RESETTMR + 0x0068)

/* Scratch register holds CPU clock rate (setup by bootrom) */
#define AR5312_SCRATCH   (AR531X_RESETTMR + 0x006c)

/*
 * Values for AR531X_CLOCKCTL1
 *
 * The AR531X_CLOCKCTL1 register is loaded based on the speed of
 * our incoming clock.  Currently, all valid configurations
 * for an AR5312 use an ar5112 radio clocked at 40MHz.  Until
 * there are other configurations available, we'll hardcode
 * this 40MHz assumption.
 */
#define AR531X_INPUT_CLOCK                  40000000
#define AR531X_CLOCKCTL1_IN40_OUT160MHZ 0x0405 # 40MHz in, 160Mhz out
#define AR531X_CLOCKCTL1_IN40_OUT180MHZ 0x0915 # 40MHz in, 180Mhz out
#define AR531X_CLOCKCTL1_IN40_OUT200MHZ 0x1935 # 40MHz in, 200Mhz out
#define AR531X_CLOCKCTL1_IN40_OUT220MHZ 0x0b15 # 40MHz in, 220Mhz out
#define AR531X_CLOCKCTL1_IN40_OUT240MHZ 0x0605 # 40MHz in, 240Mhz out

/* Board-dependent selections */
#define AR531X_CLOCKCTL1_SELECTION CYGNUM_CLOCKCTL1_SELECTION
#define AR531X_CPU_CLOCK_RATE CYGNUM_CPU_CLOCK_RATE

/* Used by asm_cninit for early use of the UART */
#define AR531X_UART_CLOCK_RATE             (AR531X_CPU_CLOCK_RATE / 4)

/* Used by romSizeMemory to set SDRAM Memory Refresh */
#define AR531X_SDRAM_CLOCK_RATE            (AR531X_CPU_CLOCK_RATE / 2)

/* Used by rom_reboot to set the watchdog timer */
#define AR531X_WATCHDOG_CLOCK_RATE         (AR531X_CPU_CLOCK_RATE / 4)

/* The various flavors of Wireless System on a Chip that are handled */
#define AR531X_FLAVOR_AR5312 1 /* "FREEDOM" */
#define AR531X_FLAVOR_AR2312 2 /* "CASPER" */
#define AR531X_FLAVOR_AR2313 3 /* "VIPER" */

/* Bit fields for AR531X_CLOCKCTL1
 *
 * Formula to calculate CPU frequency:
 *   (InputClock / Predivider) * Multiplier * (1 + Doubler)
 */
#if (CYGNUM_WISOC_FLAVOR == AR531X_FLAVOR_AR5312) || (CYGNUM_WISOC_FLAVOR == AR531X_FLAVOR_AR2312)
#define AR531X_CLOCKCTL1_PREDIVIDE_MASK    0x00000030
#define AR531X_CLOCKCTL1_PREDIVIDE_SHIFT            4
#define AR531X_CLOCKCTL1_MULTIPLIER_MASK   0x00001f00
#define AR531X_CLOCKCTL1_MULTIPLIER_SHIFT           8
#define AR531X_CLOCKCTL1_DOUBLER_MASK      0x00010000
#endif

#if CYGNUM_WISOC_FLAVOR == AR531X_FLAVOR_AR2313
#define AR531X_CLOCKCTL1_PREDIVIDE_MASK    0x00003000
#define AR531X_CLOCKCTL1_PREDIVIDE_SHIFT           12
#define AR531X_CLOCKCTL1_MULTIPLIER_MASK   0x001f0000
#define AR531X_CLOCKCTL1_MULTIPLIER_SHIFT          16
#define AR531X_CLOCKCTL1_DOUBLER_MASK      0x00000000
#endif

#if (CYGNUM_WISOC_FLAVOR == AR531X_FLAVOR_AR2312) || (CYGNUM_WISOC_FLAVOR == AR531X_FLAVOR_AR2313)
#define TWISTED_ENET_MACS 1
#endif

/* Bit fields for AR531X_CLOCKCTL2 */
#define AR531X_CLOCKCTL2_WANT_RESET        0x00000001 /* reset with new vals */
#define AR531X_CLOCKCTL2_WANT_DIV2         0x00000010 /* request /2 clock */
#define AR531X_CLOCKCTL2_WANT_DIV4         0x00000020 /* request /4 clock */
#define AR531X_CLOCKCTL2_WANT_PLL_BYPASS   0x00000080 /* request PLL bypass */
#define AR531X_CLOCKCTL2_STATUS_DIV2       0x10000000 /* have /2 clock */
#define AR531X_CLOCKCTL2_STATUS_DIV4       0x20000000 /* have /4 clock */
#define AR531X_CLOCKCTL2_STATUS_PLL_BYPASS 0x80000000 /* PLL is bypassed */

/*
 * The UART computes baud rate as:
 *   baud = clock / (16 * divisor)
 * where divisor is specified as a High Byte (DLM) and a Low Byte (DLL).
 */
#define DESIRED_BAUD_RATE                        9600

#define AR531X_NS16550_DLM_VALUE \
        ((((AR531X_UART_CLOCK_RATE/DESIRED_BAUD_RATE)/16) >> 8) & 0xff)

#define AR531X_NS16550_DLL_VALUE \
        (((AR531X_UART_CLOCK_RATE/DESIRED_BAUD_RATE)/16) & 0xff)


/*
 * SDRAM Memory Refresh (MEM_REF) value is computed as:
 *    15.625us * SDRAM_CLOCK_RATE (in MHZ)
 */
#define DESIRED_MEMORY_REFRESH_NSECS 15625
#define AR531X_SDRAM_MEMORY_REFRESH_VALUE \
        ((DESIRED_MEMORY_REFRESH_NSECS * AR531X_SDRAM_CLOCK_RATE/1000000) / 1000 )
/*
 * The WATCHDOG value is computed as
 *  10 seconds * AR531X_WATCHDOG_CLOCK_RATE
 */
#define DESIRED_WATCHDOG_SECONDS       10
#define AR531X_WATCHDOG_TIME \
        (DESIRED_WATCHDOG_SECONDS * AR531X_WATCHDOG_CLOCK_RATE)


/* Values for AR531X_CLOCKCTL */
#define CLOCKCTL_ETH0   0x0004  /* enable eth0 clock */
#define CLOCKCTL_ETH1   0x0008  /* enable eth1 clock */
#define CLOCKCTL_UART0  0x0010  /* enable UART0 external clock */

/*
 * AHB Error Reporting.
 * PROCADDR and DMAADDR are Read-And-Clear registers.
 */
#define AR531X_PROCADDR (AR531X_RESETTMR + 0x0070)
#define AR531X_PROCERR  (AR531X_RESETTMR + 0x0074)
#define AR531X_DMAADDR  (AR531X_RESETTMR + 0x0078)
#define AR531X_DMAERR   (AR531X_RESETTMR + 0x007c)

#define PROCERR_HTRANS          0x00000003
#define PROCERR_HTRANS_S        0
#define PROCERR_HRESP           0x0000000c
#define PROCERR_HRESP_S         2
#define PROCERR_HWRITE          0x00000010
#define PROCERR_HBURST          0x000000e0
#define PROCERR_HBURST_S        5
#define PROCERR_HSIZE           0x00000700
#define PROCERR_HSIZE_S         8
#define PROCERR_HPROT           0x00000800
#define PROCERR_VALID           0x80000000      /* error values are latched */

#define DMAERR_HTRANS           0x00000003
#define DMAERR_HTRANS_S         0
#define DMAERR_HRESP            0x0000000c
#define DMAERR_HRESP_S          2
#define DMAERR_HWRITE           0x00000010
#define DMAERR_HBURST           0x000000e0
#define DMAERR_HBURST_S         5
#define DMAERR_HSIZE            0x00000700
#define DMAERR_HSIZE_S          8
#define DMAERR_HMASTER          0x00007800
#define DMAERR_HMASTER_S        11
#define DMAERR_VALID            0x80000000      /* error values are latched */

/*
 * Interface Enable
 */
#define AR531X_ENABLE   (AR531X_RESETTMR + 0x0080)
#define ENABLE_WLAN0              0x0001
#define ENABLE_ENET0              0x0002
#define ENABLE_ENET1              0x0004
#define ENABLE_UART_AND_WLAN1_PIO 0x0008        /* UART, and WLAN1 PIOs */
#define ENABLE_WLAN1_DMA          0x0010        /* WLAN1 DMAs */
#define ENABLE_WLAN1 (ENABLE_UART_AND_WLAN1_PIO | ENABLE_WLAN1_DMA)

/*
 * Revision Register - Initial value is 0x3010 (WMAC 3.0, AR531X 1.0).
 */
#define AR531X_REV      (AR531X_RESETTMR + 0x0090)
#define REV_WMAC_MAJ    0xf000
#define REV_WMAC_MAJ_S  12
#define REV_WMAC_MIN    0x0f00
#define REV_WMAC_MIN_S  8
#define REV_MAJ         0x00f0
#define REV_MAJ_S       4
#define REV_MIN         0x000f
#define REV_MIN_S       0
#define REV_CHIP        (REV_MAJ|REV_MIN)

/* Major revision numbers, bits 7..4 of Revision ID register */
#define REV_MAJ_AR5311          0x01
#define REV_MAJ_AR5312          0x04

/* Minor revision numbers, bits 3..0 of Revision ID register */
#define REV_MIN_AR5312_DUAL     0x0     /* Dual WLAN version */
#define REV_MIN_AR5312_SINGLE   0x1     /* Single WLAN version */

/*
 * Local Bus Config Register
 *
 * ENET1 and Local Bus share pins, so only one can be used at a time.
 * Bit 0 decides which:
 *      0--> ENET1
 *      1--> Local Bus (default)
 *
 * Bit 1 selects external local clock on GPIO_0 (default 0)
 *
 * Bit 2 indicates that the local bus is synchronous (default 0)
 */
#define AR5312_LOCAL_BUS_CONFIG         (AR531X_RESETTMR + 0x00c0)
#define AR5312_LBCONFIG_LOCAL_BUS       0x00000001
#define AR5312_LBCONFIG_CLOCK_GPIO0     0x00000002
#define AR5312_LBCONFIG_LBSYNCHRONOUS   0x00000004

#define AR5312_LBCONFIG_DEFAULT         0x00000000

/*
 * Interrupt routing from IO to the processor IP bits
 */
#define AR531X_INTR_WLAN0       SR_IBIT3
#define AR531X_INTR_ENET0       SR_IBIT4
#define AR531X_INTR_ENET1       SR_IBIT5
#define AR531X_INTR_WLAN1       SR_IBIT6                /* (ar5312) */
#define AR531X_INTR_MISCIO      SR_IBIT7
#define AR531X_INTR_COMPARE     SR_IBIT8

/*
 * Applicable "PCICFG" bits for WLAN(s).  Assoc status and LED mode.
 */
#define AR531X_PCICFG           (AR531X_RESETTMR + 0x00b0)
#define ASSOC_STATUS_M          0x00000003
#define ASSOC_STATUS_NONE       0
#define ASSOC_STATUS_PENDING    1
#define ASSOC_STATUS_ASSOCIATED 2
#define LED_MODE_M              0x0000001c
#define LED_BLINK_THRESHOLD_M   0x000000e0
#define LED_SLOW_BLINK_MODE     0x00000100

#endif

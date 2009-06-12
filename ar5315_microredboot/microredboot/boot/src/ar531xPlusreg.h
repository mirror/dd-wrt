/*
 *  Copyright (c) 2001-2002 Atheros Communications, Inc., All Rights Reserved
 */

/*
 * Add support for Cobra
 *
 * AR531XPLUSreg.h Register definitions for Atheros AR5311 and AR5312 chipsets.
 *   - WLAN registers are listed in
 *         hal/ar5211/ar5211Reg.h
 *         hal/ar5212/ar5212Reg.h
 *   - Ethernet registers are listed in ar531xenet.h
 *   - Standard UART is 16550 compatible.
 */

#ifndef _AR531XPLUSREG_H
#define _AR531XPLUSREG_H

#include "ramconfig.h"

#ident "ACI $Header: //depot/sw/branches/mBSSID_dev/src/ap/os/vxworks/target/config/ar531xPlus/ar531xPlusreg.h#1 $"

/*
 * Address map
 */
#define AR531XPLUS_SDRAM0           0x00000000      /* DRAM */
#define AR531XPLUS_SPI_READ         0x08000000      /* SPI FLASH */
#define AR531XPLUS_WLAN0            0xB0000000      /* Wireless MMR */
#define AR531XPLUS_PCI              0xB0100000      /* PCI MMR */
#define AR531XPLUS_SDRAMCTL         0xB0300000      /* SDRAM MMR */
#define AR531XPLUS_LOCAL            0xB0400000      /* LOCAL BUS MMR */
#define AR531XPLUS_ENET0            0xB0500000      /* ETHERNET MMR */
#define AR531XPLUS_DSLBASE          0xB1000000      /* RESET CONTROL MMR */
#define AR531XPLUS_UART0            0xB1100000      /* UART MMR */
#define AR531XPLUS_SPI              0xB1300000      /* SPI FLASH MMR */
#define AR531XPLUS_FLASHBT          0xBfc00000      /* ro boot alias to FLASH */
#define AR531XPLUS_RAM1             0x40000000      /* ram alias */
#define AR531XPLUS_PCIEXT           0x80000000      /* pci external */
#define AR531XPLUS_RAM2             0xc0000000      /* ram alias */
#define AR531XPLUS_RAM3             0xe0000000      /* ram alias */

/*
 * Reset Register
 */
#define AR531XPLUS_COLD_RESET       (AR531XPLUS_DSLBASE + 0x0000)

/* Cold Reset */
#define RESET_COLD_AHB              0x00000001
#define RESET_COLD_APB              0x00000002
#define RESET_COLD_CPU              0x00000004
#define RESET_COLD_CPUWARM          0x00000008
#define RESET_SYSTEM                (RESET_COLD_CPU | RESET_COLD_AHB | RESET_COLD_CPUWARM)      /* full system */

/* Warm Reset */

#define AR531XPLUS_RESET            (AR531XPLUS_DSLBASE + 0x0004)

#define RESET_WARM_WLAN0_MAC        0x00000001      /* warm reset WLAN0 MAC */
#define RESET_WARM_WLAN0_BB         0x00000002      /* warm reset WLAN0 BaseBand */
#define RESET_MPEGTS_RSVD           0x00000004      /* warm reset MPEG-TS */
#define RESET_PCIDMA                0x00000008      /* warm reset PCI ahb/dma */
#define RESET_MEMCTL                0x00000010      /* warm reset memory controller */
#define RESET_LOCAL                 0x00000020      /* warm reset local bus */
#define RESET_I2C_RSVD              0x00000040      /* warm reset I2C bus */
#define RESET_SPI                   0x00000080      /* warm reset SPI interface */
#define RESET_UART0                 0x00000100      /* warm reset UART0 */
#define RESET_IR_RSVD               0x00000200      /* warm reset IR interface */
#define RESET_EPHY0                 0x00000400      /* cold reset ENET0 phy */
#define RESET_ENET0                 0x00000800      /* cold reset ENET0 mac */

/*
 * AHB master arbitration control
 */
#define AR531XPLUS_AHB_ARB_CTL      (AR531XPLUS_DSLBASE + 0x0008)

#define ARB_CPU                     0x00000001      /* CPU, default */
#define ARB_WLAN                    0x00000002      /* WLAN */
#define ARB_MPEGTS_RSVD             0x00000004      /* MPEG-TS */
#define ARB_LOCAL                   0x00000008      /* LOCAL */
#define ARB_PCI                     0x00000010      /* PCI */
#define ARB_ETHERNET                0x00000020      /* Ethernet */
#define ARB_RETRY                   0x00000100      /* retry policy, debug only */

/*
 * Config Register
 */
#define AR531XPLUS_ENDIAN_CTL       (AR531XPLUS_DSLBASE + 0x000c)

#define CONFIG_AHB                  0x00000001      /* EC - AHB bridge endianess */
#define CONFIG_WLAN                 0x00000002      /* WLAN byteswap */
#define CONFIG_MPEGTS_RSVD          0x00000004      /* MPEG-TS byteswap */
#define CONFIG_PCI                  0x00000008      /* PCI byteswap */
#define CONFIG_MEMCTL               0x00000010      /* Memory controller endianess */
#define CONFIG_LOCAL                0x00000020      /* Local bus byteswap */
#define CONFIG_ETHERNET             0x00000040      /* Ethernet byteswap */

#define CONFIG_MERGE                0x00000200      /* CPU write buffer merge */
#define CONFIG_CPU                  0x00000400      /* CPU big endian */
#define CONFIG_PCIAHB               0x00000800
#define CONFIG_PCIAHB_BRIDGE        0x00001000
#define CONFIG_SPI                  0x00008000      /* SPI byteswap */
#define CONFIG_CPU_DRAM             0x00010000
#define CONFIG_CPU_PCI              0x00020000
#define CONFIG_CPU_MMR              0x00040000
#define CONFIG_BIG                  0x00000400      


/*
 * NMI control
 */
#define AR531XPLUS_NMI_CTL          (AR531XPLUS_DSLBASE + 0x0010)

#define NMI_EN  1

/*
 * Revision Register - Initial value is 0x3010 (WMAC 3.0, AR531X 1.0).
 */
#define AR531XPLUS_SREV             (AR531XPLUS_DSLBASE + 0x0014)

#define REV_MAJ                     0x00f0
#define REV_MAJ_S                   4
#define REV_MIN                     0x000f
#define REV_MIN_S                   0
#define REV_CHIP                    (REV_MAJ|REV_MIN)

/*
 * Interface Enable
 */
#define AR531XPLUS_IF_CTL           (AR531XPLUS_DSLBASE + 0x0018)

#define IF_MASK                     0x00000007
#define IF_DISABLED                 0
#define IF_PCI                      1
#define IF_TS_LOCAL                 2
#define IF_ALL                      3   /* only for emulation with separate pins */
#define IF_LOCAL_HOST               0x00000008
#define IF_PCI_HOST                 0x00000010
#define IF_PCI_INTR                 0x00000020
#define IF_PCI_CLK_MASK             0x00030000
#define IF_PCI_CLK_INPUT            0 
#define IF_PCI_CLK_OUTPUT_LOW       1
#define IF_PCI_CLK_OUTPUT_CLK       2
#define IF_PCI_CLK_OUTPUT_HIGH      3
#define IF_PCI_CLK_SHIFT            16 
 
                
/* Major revision numbers, bits 7..4 of Revision ID register */
#define REV_MAJ_AR5311              0x01
#define REV_MAJ_AR5312              0x04
#define REV_MAJ_AR5315              0x0B

/*
 * APB Interrupt control
 */

#define AR531XPLUS_ISR              (AR531XPLUS_DSLBASE + 0x0020)
#define AR531XPLUS_IMR              (AR531XPLUS_DSLBASE + 0x0024)
#define AR531XPLUS_GISR             (AR531XPLUS_DSLBASE + 0x0028)

#define ISR_UART0                   0x0001           /* high speed UART */
#define ISR_I2C_RSVD                0x0002           /* I2C bus */
#define ISR_SPI                     0x0004           /* SPI bus */
#define ISR_AHB                     0x0008           /* AHB error */
#define ISR_APB                     0x0010           /* APB error */
#define ISR_TIMER                   0x0020           /* timer */
#define ISR_GPIO                    0x0040           /* GPIO */
#define ISR_WD                      0x0080           /* watchdog */
#define ISR_IR_RSVD                 0x0100           /* IR */
                                
#define IMR_UART0                   ISR_UART0
#define IMR_I2C_RSVD                ISR_I2C_RSVD
#define IMR_SPI                     ISR_SPI
#define IMR_AHB                     ISR_AHB
#define IMR_APB                     ISR_APB
#define IMR_TIMER                   ISR_TIMER
#define IMR_GPIO                    ISR_GPIO
#define IMR_WD                      ISR_WD
#define IMR_IR_RSVD                 ISR_IR_RSVD

#define GISR_MISC                   0x0001
#define GISR_WLAN0                  0x0002
#define GISR_MPEGTS_RSVD            0x0004
#define GISR_LOCALPCI               0x0008
#define GISR_WMACPOLL               0x0010
#define GISR_TIMER                  0x0020
#define GISR_ETHERNET               0x0040

/*
 * Interrupt routing from IO to the processor IP bits
 * Define our inter mask and level
 */
#define AR531XPLUS_INTR_MISCIO      SR_IBIT3
#define AR531XPLUS_INTR_WLAN0       SR_IBIT4
#define AR531XPLUS_INTR_ENET0       SR_IBIT5
#define AR531XPLUS_INTR_LOCALPCI    SR_IBIT6
#define AR531XPLUS_INTR_WMACPOLL    SR_IBIT7
#define AR531XPLUS_INTR_COMPARE     SR_IBIT8

/*
 * Timers
 */
#define AR531XPLUS_TIMER            (AR531XPLUS_DSLBASE + 0x0030)
#define AR531XPLUS_RELOAD           (AR531XPLUS_DSLBASE + 0x0034)
#define AR531XPLUS_WD               (AR531XPLUS_DSLBASE + 0x0038)
#define AR531XPLUS_WDC              (AR531XPLUS_DSLBASE + 0x003c)

#define WDC_NMI                     0x00000001     /* NMI on watchdog */
#define WDC_RESET                   0x00000002     /* reset on watchdog */
#define WDC_AHB_INTR                0x00000004     /* Watchdog AHB Interrupt */

/*
 * Cobra-specific memory controller parameters
 */
#define AR531XPLUS_RST_MEMCTL       (AR531XPLUS_DSLBASE + 0x0040)

#define RST_MEMCTL_EXT_FB           0x00000008

/*
 * Interface Debug
 */
#define AR531X_FLASHDBG             (AR531X_RESETTMR + 0x0040)
#define AR531X_MIIDBG               (AR531X_RESETTMR + 0x0044)


/*
 * CPU Performance Counters
 */
#define AR531XPLUS_PERFCNT0         (AR531XPLUS_DSLBASE + 0x0048)
#define AR531XPLUS_PERFCNT1         (AR531XPLUS_DSLBASE + 0x004c)

#define PERF_DATAHIT                0x0001  /* Count Data Cache Hits */
#define PERF_DATAMISS               0x0002  /* Count Data Cache Misses */
#define PERF_INSTHIT                0x0004  /* Count Instruction Cache Hits */
#define PERF_INSTMISS               0x0008  /* Count Instruction Cache Misses */
#define PERF_ACTIVE                 0x0010  /* Count Active Processor Cycles */
#define PERF_WBHIT                  0x0020  /* Count CPU Write Buffer Hits */
#define PERF_WBMISS                 0x0040  /* Count CPU Write Buffer Misses */
                                
#define PERF_EB_ARDY                0x0001  /* Count EB_ARdy signal */
#define PERF_EB_AVALID              0x0002  /* Count EB_AValid signal */
#define PERF_EB_WDRDY               0x0004  /* Count EB_WDRdy signal */
#define PERF_EB_RDVAL               0x0008  /* Count EB_RdVal signal */
#define PERF_VRADDR                 0x0010  /* Count valid read address cycles */
#define PERF_VWADDR                 0x0020  /* Count valid write address cycles */
#define PERF_VWDATA                 0x0040  /* Count valid write data cycles */

/*
 * AHB Error Reporting.
 */
#define AR531XPLUS_AHB_ERR0         (AR531XPLUS_DSLBASE + 0x0050)  /* error  */
#define AR531XPLUS_AHB_ERR1         (AR531XPLUS_DSLBASE + 0x0054)  /* haddr  */
#define AR531XPLUS_AHB_ERR2         (AR531XPLUS_DSLBASE + 0x0058)  /* hwdata */
#define AR531XPLUS_AHB_ERR3         (AR531XPLUS_DSLBASE + 0x005c)  /* hrdata */
#define AR531XPLUS_AHB_ERR4         (AR531XPLUS_DSLBASE + 0x0060)  /* status */

#define AHB_ERROR_DET               1   /* AHB Error has been detected,          */
                                        /* write 1 to clear all bits in ERR0     */
#define AHB_ERROR_OVR               2   /* AHB Error overflow has been detected  */
#define AHB_ERROR_WDT               4   /* AHB Error due to wdt instead of hresp */

#define PROCERR_HMAST               0x0000000f
#define PROCERR_HMAST_DFLT          0
#define PROCERR_HMAST_WMAC          1
#define PROCERR_HMAST_ENET          2
#define PROCERR_HMAST_PCIENDPT      3
#define PROCERR_HMAST_LOCAL         4
#define PROCERR_HMAST_CPU           5
#define PROCERR_HMAST_PCITGT        6
                                    
#define PROCERR_HMAST_S             0
#define PROCERR_HWRITE              0x00000010
#define PROCERR_HSIZE               0x00000060
#define PROCERR_HSIZE_S             5
#define PROCERR_HTRANS              0x00000180
#define PROCERR_HTRANS_S            7
#define PROCERR_HBURST              0x00000e00
#define PROCERR_HBURST_S            9



/*
 * Clock Control
 */
#define AR531XPLUS_PLLC_CTL         (AR531XPLUS_DSLBASE + 0x0064)
#define AR531XPLUS_PLLV_CTL         (AR531XPLUS_DSLBASE + 0x0068)
#define AR531XPLUS_CPUCLK           (AR531XPLUS_DSLBASE + 0x006c)
#define AR531XPLUS_AMBACLK          (AR531XPLUS_DSLBASE + 0x0070)
#define AR531XPLUS_SYNCCLK          (AR531XPLUS_DSLBASE + 0x0074)
#define AR531XPLUS_DSL_SLEEP_CTL    (AR531XPLUS_DSLBASE + 0x0080)
#define AR531XPLUS_DSL_SLEEP_DUR    (AR531XPLUS_DSLBASE + 0x0084)

/* PLLc Control fields */
#define PLLC_REF_DIV_M              0x00000003
#define PLLC_REF_DIV_S              0
#define PLLC_FDBACK_DIV_M           0x0000007C
#define PLLC_FDBACK_DIV_S           2
#define PLLC_ADD_FDBACK_DIV_M       0x00000080
#define PLLC_ADD_FDBACK_DIV_S       7
#define PLLC_CLKC_DIV_M             0x0001c000
#define PLLC_CLKC_DIV_S             14
#define PLLC_CLKM_DIV_M             0x00700000
#define PLLC_CLKM_DIV_S             20

/* CPU CLK Control fields */
#define CPUCLK_CLK_SEL_M            0x00000003
#define CPUCLK_CLK_SEL_S            0
#define CPUCLK_CLK_DIV_M            0x0000000c
#define CPUCLK_CLK_DIV_S            2

/* AMBA CLK Control fields */
#define AMBACLK_CLK_SEL_M           0x00000003
#define AMBACLK_CLK_SEL_S           0
#define AMBACLK_CLK_DIV_M           0x0000000c
#define AMBACLK_CLK_DIV_S           2

#if defined(COBRA_EMUL)
#define AR531XPLUS_AMBA_CLOCK_RATE  20000000
#define AR531XPLUS_CPU_CLOCK_RATE   40000000
#else
#if defined(DEFAULT_PLL)
#define AR531XPLUS_AMBA_CLOCK_RATE  40000000
#define AR531XPLUS_CPU_CLOCK_RATE   40000000
#else
#define AR531XPLUS_AMBA_CLOCK_RATE  92000000
#define AR531XPLUS_CPU_CLOCK_RATE   184000000
#endif /* ! DEFAULT_PLL */
#endif /* ! COBRA_EMUL */

#define AR531XPLUS_UART_CLOCK_RATE  AR531XPLUS_AMBA_CLOCK_RATE

#if !PRJ_BUILD
#define AR531XPLUS_SDRAM_CLOCK_RATE AR531XPLUS_AMBA_CLOCK_RATE
#endif /* ! PRJ_BUILD */            

#define AR531XPLUS_WATCHDOG_CLOCK_RATE  AR531XPLUS_AMBA_CLOCK_RATE

/*
 * The UART computes baud rate as:
 *   baud = clock / (16 * divisor)
 * where divisor is specified as a High Byte (DLM) and a Low Byte (DLL).
 */
#define DESIRED_BAUD_RATE           9600

/*
 * The WATCHDOG value is computed as
 *  10 seconds * AR531X_WATCHDOG_CLOCK_RATE
 */
#define DESIRED_WATCHDOG_SECONDS    10
#define AR531XPLUS_WATCHDOG_TIME \
        (DESIRED_WATCHDOG_SECONDS * AR531XPLUS_WATCHDOG_CLOCK_RATE)


#define CLOCKCTL_UART0  0x0010  /* enable UART0 external clock */

/*
 * GPIO
 */

#define AR531XPLUS_GPIO_DI          (AR531XPLUS_DSLBASE + 0x0088)
#define AR531XPLUS_GPIO_DO          (AR531XPLUS_DSLBASE + 0x0090)
#define AR531XPLUS_GPIO_CR          (AR531XPLUS_DSLBASE + 0x0098)
#define AR531XPLUS_GPIO_INT         (AR531XPLUS_DSLBASE + 0x00a0)

#define GPIO_CR_M(x)                (1 << (x))                  /* mask for i/o */
#define GPIO_CR_O(x)                (1 << (x))                  /* output */
#define GPIO_CR_I(x)                (0 << (x))                  /* input */

#define GPIO_INT(x,Y)               ((x) << (8 * (Y)))          /* interrupt enable */
#define GPIO_INT_M(Y)               ((0x3F) << (8 * (Y)))       /* mask for int */
#define GPIO_INT_LVL(x,Y)           ((x) << (8 * (Y) + 6))      /* interrupt level */
#define GPIO_INT_LVL_M(Y)           ((0x3) << (8 * (Y) + 6))    /* mask for int level */

#define AR531XPLUS_NUM_GPIO         22
#define AR531XPLUS_RESET_GPIO       5
    
/* 
 *  PCI Clock Control
 */     
 
#define AR531XPLUS_PCICLK           (AR531XPLUS_DSLBASE + 0x00a4)

#define PCICLK_INPUT_M              0x3
#define PCICLK_INPUT_S              0
                         
#define PCICLK_PLLC_CLKM            0
#define PCICLK_PLLC_CLKM1           1
#define PCICLK_PLLC_CLKC            2
#define PCICLK_REF_CLK              3 

#define PCICLK_DIV_M                0xc
#define PCICLK_DIV_S                2
                         
#define PCICLK_IN_FREQ              0
#define PCICLK_IN_FREQ_DIV_6        1
#define PCICLK_IN_FREQ_DIV_8        2
#define PCICLK_IN_FREQ_DIV_10       3 

#define AR531XPLUS_SCRATCH0         (AR531XPLUS_DSLBASE + 0x00a8)
#define AR531XPLUS_SCRATCH1         (AR531XPLUS_DSLBASE + 0x00ac)

/*
 * Observation Control Register
 */
#define AR531XPLUS_OCR              (AR531XPLUS_DSLBASE + 0x00b0)
#define OCR_LED0_GPIO1              0x00008000
#define OCR_LED1_GPIO2              0x00010000

/* 
 *  General Clock Control
 */     
 
#define AR531XPLUS_MISCCLK          (AR531XPLUS_DSLBASE + 0x00b4)
#define MISCCLK_PLLBYPASS_EN        0x00000001
#define MISCCLK_PROCREFCLK          0x00000002

/*
 * SDRAM Controller
 *   - No read or write buffers are included.
 */
#define AR531XPLUS_MEM_CFG          (AR531XPLUS_SDRAMCTL + 0x00)
#define AR531XPLUS_MEM_STMG0R       (AR531XPLUS_SDRAMCTL + 0x04)
#define AR531XPLUS_MEM_CTRL         (AR531XPLUS_SDRAMCTL + 0x0c)
#define AR531XPLUS_MEM_REF          (AR531XPLUS_SDRAMCTL + 0x10)

#define SDRAM_DATA_WIDTH_M          0x00006000
#define SDRAM_DATA_WIDTH_S          13

#define SDRAM_COL_WIDTH_M           0x00001E00
#define SDRAM_COL_WIDTH_S           9

#define SDRAM_ROW_WIDTH_M           0x000001E0
#define SDRAM_ROW_WIDTH_S           5

#define SDRAM_BANKADDR_BITS_M       0x00000018
#define SDRAM_BANKADDR_BITS_S       3


/*
 * SDRAM Memory Refresh (MEM_REF) value is computed as:
 * MEMCTL_SREFR = (Tr * hclk_freq) / R
 * where Tr is max. time of refresh of any single row
 * R is number of rows in the DRAM
 * For most 133MHz SDRAM parts, Tr=64ms, R=4096 or 8192
 */
#if defined(COBRA_EMUL)
#define AR531XPLUS_SDRAM_MEMORY_REFRESH_VALUE  0x96
#else 
#if defined(DEFAULT_PLL)
#define AR531XPLUS_SDRAM_MEMORY_REFRESH_VALUE  0x200
#else
#define AR531XPLUS_SDRAM_MEMORY_REFRESH_VALUE  0x61a
#endif /* ! DEFAULT_PLL */
#endif 

#if defined(AR531XPLUS)

#define AR531XPLUS_SDRAM_DDR_SDRAM      0   /* Not DDR SDRAM */
#define AR531XPLUS_SDRAM_DATA_WIDTH     16  /* bits */   
#if RAM_SIZE == 0x2000000
#define AR531XPLUS_SDRAM_COL_WIDTH      9
#define AR531XPLUS_SDRAM_ROW_WIDTH      13
#elif RAM_SIZE == 0x1000000
#define AR531XPLUS_SDRAM_COL_WIDTH      9
#define AR531XPLUS_SDRAM_ROW_WIDTH      12
#else
#define AR531XPLUS_SDRAM_COL_WIDTH      8
#define AR531XPLUS_SDRAM_ROW_WIDTH      12
#endif

#else

#define AR531XPLUS_SDRAM_DDR_SDRAM      0   /* Not DDR SDRAM */
#define AR531XPLUS_SDRAM_DATA_WIDTH     16
#if RAM_SIZE == 0x2000000
#define AR531XPLUS_SDRAM_COL_WIDTH      9
#define AR531XPLUS_SDRAM_ROW_WIDTH      13
#elif RAM_SIZE == 0x1000000
#define AR531XPLUS_SDRAM_COL_WIDTH      9
#define AR531XPLUS_SDRAM_ROW_WIDTH      12
#else
#define AR531XPLUS_SDRAM_COL_WIDTH      8
#define AR531XPLUS_SDRAM_ROW_WIDTH      12
#endif
#endif /* ! AR531XPLUS */

/*
 * SPI Flash Interface Registers
 */

#define AR531XPLUS_SPI_CTL      (AR531XPLUS_SPI + 0x00)
#define AR531XPLUS_SPI_OPCODE   (AR531XPLUS_SPI + 0x04)
#define AR531XPLUS_SPI_DATA     (AR531XPLUS_SPI + 0x08)

#define SPI_CTL_START           0x00000100
#define SPI_CTL_BUSY            0x00010000
#define SPI_CTL_TXCNT_MASK      0x0000000f
#define SPI_CTL_RXCNT_MASK      0x000000f0
#define SPI_CTL_TX_RX_CNT_MASK  0x000000ff
#define SPI_CTL_SIZE_MASK       0x00060000

#define SPI_CTL_CLK_SEL_MASK    0x03000000
#define SPI_OPCODE_MASK         0x000000ff

/* 
 * PCI-MAC Configuration registers 
 */
#define PCI_MAC_RC              (AR531XPLUS_PCI + 0x4000) 
#define PCI_MAC_SCR             (AR531XPLUS_PCI + 0x4004)
#define PCI_MAC_INTPEND         (AR531XPLUS_PCI + 0x4008)
#define PCI_MAC_SFR             (AR531XPLUS_PCI + 0x400C)

#define PCI_MAC_PCICFG          (AR531XPLUS_PCI + 0x4010)
#define ASSOC_STATUS_M          0x00000060
#define ASSOC_STATUS_NONE       0
#define ASSOC_STATUS_PENDING    0x20
#define ASSOC_STATUS_ASSOCIATED 0x40

#define LED_MODE_M              0x000e0000
#define LED_BLINK_THRESHOLD_M   0x00700000
#define LED_SLOW_BLINK_MODE     0x00800000

#define PCI_MAC_SREV            (AR531XPLUS_PCI + 0x4020)

#define PCI_MAC_RC_MAC          0x00000001
#define PCI_MAC_RC_BB           0x00000002

#define PCI_MAC_SCR_SLMODE_M    0x00030000
#define PCI_MAC_SCR_SLMODE_S    16        
#define PCI_MAC_SCR_SLM_FWAKE   0         
#define PCI_MAC_SCR_SLM_FSLEEP  1         
#define PCI_MAC_SCR_SLM_NORMAL  2         

#define PCI_MAC_SFR_SLEEP       0x00000001

#define PCI_MAC_PCICFG_SPWR_DN  0x00010000


/* 
 * WLAN MAC Configuration Registers 
 */
 
#define AR531XPLUS_WLANPHY_BASE (AR531XPLUS_WLAN0 + 0x9800)

/*
 * PCI Bus Interface Registers
 */
#define AR531XPLUS_PCI_1MS_REG      (AR531XPLUS_PCI + 0x0008)
#define AR531XPLUS_PCI_1MS_MASK     0x3FFFF         /* # of AHB clk cycles in 1ms */

#define AR531XPLUS_PCI_MISC_CONFIG  (AR531XPLUS_PCI + 0x000c)
#define AR531XPLUS_PCIMISC_TXD_EN   0x00000001      /* Enable TXD for fragments */
#define AR531XPLUS_PCIMISC_CFG_SEL  0x00000002      /* mem or config cycles */
#define AR531XPLUS_PCIMISC_GIG_MASK 0x0000000C      /* bits 31-30 for pci req */
#define AR531XPLUS_PCIMISC_RST_MODE 0x00000030
#define AR531XPLUS_PCIRST_INPUT     0x00000000      /* 4:5=0 rst is input */
#define AR531XPLUS_PCIRST_LOW       0x00000010      /* 4:5=1 rst to GND */
#define AR531XPLUS_PCIRST_HIGH      0x00000020      /* 4:5=2 rst to VDD */
#define AR531XPLUS_PCIGRANT_EN      0x00000000      /* 6:7=0 early grant en */
#define AR531XPLUS_PCIGRANT_FRAME   0x00000040      /* 6:7=1 grant waits 4 frame */
#define AR531XPLUS_PCIGRANT_IDLE    0x00000080      /* 6:7=2 grant waits 4 idle */
#define AR531XPLUS_PCIGRANT_GAP     0x00000000      /* 6:7=2 grant waits 4 idle */
#define AR531XPLUS_PCICACHE_DIS     0x00001000      /* PCI external access cache disable */

#define AR531XPLUS_PCI_OUT_TSTAMP   (AR531XPLUS_PCI + 0x0010)

#define AR531XPLUS_PCI_UNCACHE_CFG  (AR531XPLUS_PCI + 0x0014)

#define AR531XPLUS_PCI_IN_EN        (AR531XPLUS_PCI + 0x0100)
#define AR531XPLUS_PCI_IN_EN0       0x01            /* Enable chain 0 */
#define AR531XPLUS_PCI_IN_EN1       0x02            /* Enable chain 1 */
#define AR531XPLUS_PCI_IN_EN2       0x04            /* Enable chain 2 */
#define AR531XPLUS_PCI_IN_EN3       0x08            /* Enable chain 3 */

#define AR531XPLUS_PCI_IN_DIS       (AR531XPLUS_PCI + 0x0104)
#define AR531XPLUS_PCI_IN_DIS0      0x01            /* Disable chain 0 */
#define AR531XPLUS_PCI_IN_DIS1      0x02            /* Disable chain 1 */
#define AR531XPLUS_PCI_IN_DIS2      0x04            /* Disable chain 2 */
#define AR531XPLUS_PCI_IN_DIS3      0x08            /* Disable chain 3 */

#define AR531XPLUS_PCI_IN_PTR       (AR531XPLUS_PCI + 0x0200)

#define AR531XPLUS_PCI_OUT_EN       (AR531XPLUS_PCI + 0x0400)
#define AR531XPLUS_PCI_OUT_EN0      0x01            /* Enable chain 0 */

#define AR531XPLUS_PCI_OUT_DIS      (AR531XPLUS_PCI + 0x0404)
#define AR531XPLUS_PCI_OUT_DIS0     0x01            /* Disable chain 0 */

#define AR531XPLUS_PCI_OUT_PTR      (AR531XPLUS_PCI + 0x0408)

#define AR531XPLUS_PCI_INT_STATUS   (AR531XPLUS_PCI + 0x0500)   /* write one to clr */
#define AR531XPLUS_PCI_TXINT        0x00000001      /* Desc In Completed */
#define AR531XPLUS_PCI_TXOK         0x00000002      /* Desc In OK */
#define AR531XPLUS_PCI_TXERR        0x00000004      /* Desc In ERR */
#define AR531XPLUS_PCI_TXEOL        0x00000008      /* Desc In End-of-List */
#define AR531XPLUS_PCI_RXINT        0x00000010      /* Desc Out Completed */
#define AR531XPLUS_PCI_RXOK         0x00000020      /* Desc Out OK */
#define AR531XPLUS_PCI_RXERR        0x00000040      /* Desc Out ERR */
#define AR531XPLUS_PCI_RXEOL        0x00000080      /* Desc Out EOL */
#define AR531XPLUS_PCI_TXOOD        0x00000200      /* Desc In Out-of-Desc */
#define AR531XPLUS_PCI_MASK         0x0000FFFF      /* Desc Mask */
#define AR531XPLUS_PCI_EXT_INT      0x02000000      
#define AR531XPLUS_PCI_ABORT_INT    0x04000000      

#define AR531XPLUS_PCI_INT_MASK     (AR531XPLUS_PCI + 0x0504)   /* same as INT_STATUS */

#define AR531XPLUS_PCI_INTEN_REG    (AR531XPLUS_PCI + 0x0508)
#define AR531XPLUS_PCI_INT_DISABLE  0x00            /* disable pci interrupts */
#define AR531XPLUS_PCI_INT_ENABLE   0x01            /* enable pci interrupts */

#define AR531XPLUS_PCI_HOST_IN_EN   (AR531XPLUS_PCI + 0x0800)
#define AR531XPLUS_PCI_HOST_IN_DIS  (AR531XPLUS_PCI + 0x0804)
#define AR531XPLUS_PCI_HOST_IN_PTR  (AR531XPLUS_PCI + 0x0810)
#define AR531XPLUS_PCI_HOST_OUT_EN  (AR531XPLUS_PCI + 0x0900)
#define AR531XPLUS_PCI_HOST_OUT_DIS (AR531XPLUS_PCI + 0x0904)
#define AR531XPLUS_PCI_HOST_OUT_PTR (AR531XPLUS_PCI + 0x0908)


/*
 * Local Bus Interface Registers
 */
#define AR531XPLUS_LB_CONFIG        (AR531XPLUS_LOCAL + 0x0000)
#define AR531XPLUS_LBCONF_OE        0x00000001      /* =1 OE is low-true */
#define AR531XPLUS_LBCONF_CS0       0x00000002      /* =1 first CS is low-true */
#define AR531XPLUS_LBCONF_CS1       0x00000004      /* =1 2nd CS is low-true */
#define AR531XPLUS_LBCONF_RDY       0x00000008      /* =1 RDY is low-true */
#define AR531XPLUS_LBCONF_WE        0x00000010      /* =1 Write En is low-true */
#define AR531XPLUS_LBCONF_WAIT      0x00000020      /* =1 WAIT is low-true */
#define AR531XPLUS_LBCONF_ADS       0x00000040      /* =1 Adr Strobe is low-true */
#define AR531XPLUS_LBCONF_MOT       0x00000080      /* =0 Intel, =1 Motorola */
#define AR531XPLUS_LBCONF_8CS       0x00000100      /* =1 8 bits CS, 0= 16bits */
#define AR531XPLUS_LBCONF_8DS       0x00000200      /* =1 8 bits Data S, 0=16bits */
#define AR531XPLUS_LBCONF_ADS_EN    0x00000400      /* =1 Enable ADS */
#define AR531XPLUS_LBCONF_ADR_OE    0x00000800      /* =1 Adr cap on OE, WE or DS */
#define AR531XPLUS_LBCONF_ADDT_MUX  0x00001000      /* =1 Adr and Data share bus */
#define AR531XPLUS_LBCONF_DATA_OE   0x00002000      /* =1 Data cap on OE, WE, DS */
#define AR531XPLUS_LBCONF_16DATA    0x00004000      /* =1 Data is 16 bits wide */
#define AR531XPLUS_LBCONF_SWAPDT    0x00008000      /* =1 Byte swap data */
#define AR531XPLUS_LBCONF_SYNC      0x00010000      /* =1 Bus synchronous to clk */
#define AR531XPLUS_LBCONF_INT       0x00020000      /* =1 Intr is low true */
#define AR531XPLUS_LBCONF_INT_CTR0  0x00000000      /* GND high-Z, Vdd is high-Z */
#define AR531XPLUS_LBCONF_INT_CTR1  0x00040000      /* GND drive, Vdd is high-Z */
#define AR531XPLUS_LBCONF_INT_CTR2  0x00080000      /* GND high-Z, Vdd drive */
#define AR531XPLUS_LBCONF_INT_CTR3  0x000C0000      /* GND drive, Vdd drive */
#define AR531XPLUS_LBCONF_RDY_WAIT  0x00100000      /* =1 RDY is negative of WAIT */
#define AR531XPLUS_LBCONF_INT_PULSE 0x00200000      /* =1 Interrupt is a pulse */
#define AR531XPLUS_LBCONF_ENABLE    0x00400000      /* =1 Falcon respond to LB */

#define AR531XPLUS_LB_CLKSEL        (AR531XPLUS_LOCAL + 0x0004)
#define AR531XPLUS_LBCLK_EXT        0x0001          /* use external clk for lb */

#define AR531XPLUS_LB_1MS           (AR531XPLUS_LOCAL + 0x0008)
#define AR531XPLUS_LB1MS_MASK       0x3FFFF         /* # of AHB clk cycles in 1ms */

#define AR531XPLUS_LB_MISCCFG       (AR531XPLUS_LOCAL + 0x000C)
#define AR531XPLUS_LBM_TXD_EN       0x00000001      /* Enable TXD for fragments */
#define AR531XPLUS_LBM_RX_INTEN     0x00000002      /* Enable LB ints on RX ready */
#define AR531XPLUS_LBM_MBOXWR_INTEN 0x00000004      /* Enable LB ints on mbox wr */
#define AR531XPLUS_LBM_MBOXRD_INTEN 0x00000008      /* Enable LB ints on mbox rd */
#define AR531XPLUS_LMB_DESCSWAP_EN  0x00000010      /* Byte swap desc enable */
#define AR531XPLUS_LBM_TIMEOUT_MASK 0x00FFFF80
#define AR531XPLUS_LBM_TIMEOUT_SHFT 7
#define AR531XPLUS_LBM_PORTMUX      0x07000000


#define AR531XPLUS_LB_RXTSOFF       (AR531XPLUS_LOCAL + 0x0010)

#define AR531XPLUS_LB_TX_CHAIN_EN   (AR531XPLUS_LOCAL + 0x0100)
#define AR531XPLUS_LB_TXEN_0        0x01
#define AR531XPLUS_LB_TXEN_1        0x02
#define AR531XPLUS_LB_TXEN_2        0x04
#define AR531XPLUS_LB_TXEN_3        0x08

#define AR531XPLUS_LB_TX_CHAIN_DIS  (AR531XPLUS_LOCAL + 0x0104)
#define AR531XPLUS_LB_TX_DESC_PTR   (AR531XPLUS_LOCAL + 0x0200)

#define AR531XPLUS_LB_RX_CHAIN_EN   (AR531XPLUS_LOCAL + 0x0400)
#define AR531XPLUS_LB_RXEN          0x01

#define AR531XPLUS_LB_RX_CHAIN_DIS  (AR531XPLUS_LOCAL + 0x0404)
#define AR531XPLUS_LB_RX_DESC_PTR   (AR531XPLUS_LOCAL + 0x0408)

#define AR531XPLUS_LB_INT_STATUS    (AR531XPLUS_LOCAL + 0x0500)
#define AR531XPLUS_INT_TX_DESC      0x0001
#define AR531XPLUS_INT_TX_OK        0x0002
#define AR531XPLUS_INT_TX_ERR       0x0004
#define AR531XPLUS_INT_TX_EOF       0x0008
#define AR531XPLUS_INT_RX_DESC      0x0010
#define AR531XPLUS_INT_RX_OK        0x0020
#define AR531XPLUS_INT_RX_ERR       0x0040
#define AR531XPLUS_INT_RX_EOF       0x0080
#define AR531XPLUS_INT_TX_TRUNC     0x0100
#define AR531XPLUS_INT_TX_STARVE    0x0200
#define AR531XPLUS_INT_LB_TIMEOUT   0x0400
#define AR531XPLUS_INT_LB_ERR       0x0800
#define AR531XPLUS_INT_MBOX_WR      0x1000
#define AR531XPLUS_INT_MBOX_RD      0x2000

/* Bit definitions for INT MASK are the same as INT_STATUS */
#define AR531XPLUS_LB_INT_MASK      (AR531XPLUS_LOCAL + 0x0504)

#define AR531XPLUS_LB_INT_EN        (AR531XPLUS_LOCAL + 0x0508)
#define AR531XPLUS_LB_MBOX          (AR531XPLUS_LOCAL + 0x0600)



/*
 * IR Interface Registers
 */
#define AR531XPLUS_IR_PKTDATA                   (AR531XPLUS_IR + 0x0000)

#define AR531XPLUS_IR_PKTLEN                    (AR531XPLUS_IR + 0x07fc) /* 0 - 63 */

#define AR531XPLUS_IR_CONTROL                   (AR531XPLUS_IR + 0x0800)
#define AR531XPLUS_IRCTL_TX                     0x00000000  /* use as tranmitter */
#define AR531XPLUS_IRCTL_RX                     0x00000001  /* use as receiver   */
#define AR531XPLUS_IRCTL_SAMPLECLK_MASK         0x00003ffe  /* Sample clk divisor mask */
#define AR531XPLUS_IRCTL_SAMPLECLK_SHFT                  1
#define AR531XPLUS_IRCTL_OUTPUTCLK_MASK         0x03ffc000  /* Output clk divisor mask */
#define AR531XPLUS_IRCTL_OUTPUTCLK_SHFT                 14

#define AR531XPLUS_IR_STATUS                    (AR531XPLUS_IR + 0x0804)
#define AR531XPLUS_IRSTS_RX                     0x00000001  /* receive in progress */
#define AR531XPLUS_IRSTS_TX                     0x00000002  /* transmit in progress */

#define AR531XPLUS_IR_CONFIG                    (AR531XPLUS_IR + 0x0808)
#define AR531XPLUS_IRCFG_INVIN                  0x00000001  /* invert input polarity */
#define AR531XPLUS_IRCFG_INVOUT                 0x00000002  /* invert output polarity */
#define AR531XPLUS_IRCFG_SEQ_START_WIN_SEL      0x00000004  /* 1 => 28, 0 => 7 */
#define AR531XPLUS_IRCFG_SEQ_START_THRESH       0x000000f0  /*  */
#define AR531XPLUS_IRCFG_SEQ_END_UNIT_SEL       0x00000100  /*  */
#define AR531XPLUS_IRCFG_SEQ_END_UNIT_THRESH    0x00007e00  /*  */
#define AR531XPLUS_IRCFG_SEQ_END_WIN_SEL        0x00008000  /*  */
#define AR531XPLUS_IRCFG_SEQ_END_WIN_THRESH     0x001f0000  /*  */
#define AR531XPLUS_IRCFG_NUM_BACKOFF_WORDS      0x01e00000  /*  */

/*
 * PCI memory constants: Memory area 1 and 2 are the same size -
 * (twice the PCI_TLB_PAGE_SIZE). The definition of
 * CPU_TO_PCI_MEM_SIZE is coupled with the TLB setup routine
 * sysLib.c/sysTlbInit(), in that it assumes that 2 pages of size
 * PCI_TLB_PAGE_SIZE are set up in the TLB for each PCI memory space.
 */
 
#define CPU_TO_PCI_MEM_BASE1    0xE0000000
#define CPU_TO_PCI_MEM_SIZE1    (2*PCI_TLB_PAGE_SIZE)
 

/* TLB attributes for PCI transactions */

#define PCI_MMU_PAGEMASK        0x00003FFF
#define MMU_PAGE_UNCACHED       0x00000010
#define MMU_PAGE_DIRTY          0x00000004
#define MMU_PAGE_VALID          0x00000002
#define MMU_PAGE_GLOBAL         0x00000001
#define PCI_MMU_PAGEATTRIB      (MMU_PAGE_UNCACHED|MMU_PAGE_DIRTY|\
                                 MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define PCI_MEMORY_SPACE1_VIRT  0xE0000000      /* Used for non-prefet  mem   */
#define PCI_MEMORY_SPACE1_PHYS  0x80000000
#define PCI_TLB_PAGE_SIZE       0x01000000
#define TLB_HI_MASK             0xFFFFE000
#define TLB_LO_MASK             0x3FFFFFFF
#define PAGEMASK_SHIFT          11
#define TLB_LO_SHIFT            6

#define PCI_MAX_LATENCY         0xFFF           /* Max PCI latency            */

#define HOST_PCI_DEV_ID         3
#define HOST_PCI_MBAR0          0x10000000
#define HOST_PCI_MBAR1          0x20000000
#define HOST_PCI_MBAR2          0x30000000

#define HOST_PCI_SDRAM_BASEADDR HOST_PCI_MBAR1
#define PCI_DEVICE_MEM_SPACE    0x800000
/*
 * Access macros for mips soc registers.
 */
#define sysRegRead(phys)        \
        (*(volatile unsigned int *)PHYS_TO_K1(phys))
#define sysRegWrite(phys,val)   \
        ((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))

#endif

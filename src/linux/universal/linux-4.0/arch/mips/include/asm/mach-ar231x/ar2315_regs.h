/*
 * Register definitions for AR2315+
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006-2008 Felix Fietkau <nbd@openwrt.org>
 */

#ifndef __AR2315_REG_H
#define __AR2315_REG_H

/*
 * IRQs
 */
#define AR2315_IRQ_MISC_INTRS   MIPS_CPU_IRQ_BASE+2 /* C0_CAUSE: 0x0400 */
#define AR2315_IRQ_WLAN0_INTRS  MIPS_CPU_IRQ_BASE+3 /* C0_CAUSE: 0x0800 */
#define AR2315_IRQ_ENET0_INTRS  MIPS_CPU_IRQ_BASE+4 /* C0_CAUSE: 0x1000 */
#define AR2315_IRQ_LCBUS_PCI    MIPS_CPU_IRQ_BASE+5 /* C0_CAUSE: 0x2000 */
#define AR2315_IRQ_WLAN0_POLL   MIPS_CPU_IRQ_BASE+6 /* C0_CAUSE: 0x4000 */

/*
 * Address map
 */
#define AR2315_SPI_READ         0x08000000      /* SPI FLASH */
#define AR2315_WLAN0            0x10000000      /* Wireless MMR */
#define AR2315_PCI              0x10100000      /* PCI MMR */
#define AR2315_SDRAMCTL         0x10300000      /* SDRAM MMR */
#define AR2315_LOCAL            0x10400000      /* LOCAL BUS MMR */
#define AR2315_ENET0            0x10500000      /* ETHERNET MMR */
#define AR2315_DSLBASE          0x11000000      /* RESET CONTROL MMR */
#define AR2315_UART0            0x11100003      /* UART MMR */
#define AR2315_SPI              0x11300000      /* SPI FLASH MMR */
#define AR2315_PCIEXT           0x80000000      /* pci external */

/*
 * Reset Register
 */
#define AR2315_COLD_RESET       (AR2315_DSLBASE + 0x0000)

#define AR2315_RESET_COLD_AHB              0x00000001
#define AR2315_RESET_COLD_APB              0x00000002
#define AR2315_RESET_COLD_CPU              0x00000004
#define AR2315_RESET_COLD_CPUWARM          0x00000008
#define AR2315_RESET_SYSTEM                (RESET_COLD_CPU | RESET_COLD_APB | RESET_COLD_AHB)      /* full system */
#define AR2317_RESET_SYSTEM                0x00000010


#define AR2315_RESET            (AR2315_DSLBASE + 0x0004)

#define AR2315_RESET_WARM_WLAN0_MAC        0x00000001      /* warm reset WLAN0 MAC */
#define AR2315_RESET_WARM_WLAN0_BB         0x00000002      /* warm reset WLAN0 BaseBand */
#define AR2315_RESET_MPEGTS_RSVD           0x00000004      /* warm reset MPEG-TS */
#define AR2315_RESET_PCIDMA                0x00000008      /* warm reset PCI ahb/dma */
#define AR2315_RESET_MEMCTL                0x00000010      /* warm reset memory controller */
#define AR2315_RESET_LOCAL                 0x00000020      /* warm reset local bus */
#define AR2315_RESET_I2C_RSVD              0x00000040      /* warm reset I2C bus */
#define AR2315_RESET_SPI                   0x00000080      /* warm reset SPI interface */
#define AR2315_RESET_UART0                 0x00000100      /* warm reset UART0 */
#define AR2315_RESET_IR_RSVD               0x00000200      /* warm reset IR interface */
#define AR2315_RESET_EPHY0                 0x00000400      /* cold reset ENET0 phy */
#define AR2315_RESET_ENET0                 0x00000800      /* cold reset ENET0 mac */

/*
 * AHB master arbitration control
 */
#define AR2315_AHB_ARB_CTL      (AR2315_DSLBASE + 0x0008)

#define AR2315_ARB_CPU                     0x00000001      /* CPU, default */
#define AR2315_ARB_WLAN                    0x00000002      /* WLAN */
#define AR2315_ARB_MPEGTS_RSVD             0x00000004      /* MPEG-TS */
#define AR2315_ARB_LOCAL                   0x00000008      /* LOCAL */
#define AR2315_ARB_PCI                     0x00000010      /* PCI */
#define AR2315_ARB_ETHERNET                0x00000020      /* Ethernet */
#define AR2315_ARB_RETRY                   0x00000100      /* retry policy, debug only */

/*
 * Config Register
 */
#define AR2315_ENDIAN_CTL       (AR2315_DSLBASE + 0x000c)

#define AR2315_CONFIG_AHB                  0x00000001      /* EC - AHB bridge endianess */
#define AR2315_CONFIG_WLAN                 0x00000002      /* WLAN byteswap */
#define AR2315_CONFIG_MPEGTS_RSVD          0x00000004      /* MPEG-TS byteswap */
#define AR2315_CONFIG_PCI                  0x00000008      /* PCI byteswap */
#define AR2315_CONFIG_MEMCTL               0x00000010      /* Memory controller endianess */
#define AR2315_CONFIG_LOCAL                0x00000020      /* Local bus byteswap */
#define AR2315_CONFIG_ETHERNET             0x00000040      /* Ethernet byteswap */

#define AR2315_CONFIG_MERGE                0x00000200      /* CPU write buffer merge */
#define AR2315_CONFIG_CPU                  0x00000400      /* CPU big endian */
#define AR2315_CONFIG_PCIAHB               0x00000800
#define AR2315_CONFIG_PCIAHB_BRIDGE        0x00001000
#define AR2315_CONFIG_SPI                  0x00008000      /* SPI byteswap */
#define AR2315_CONFIG_CPU_DRAM             0x00010000
#define AR2315_CONFIG_CPU_PCI              0x00020000
#define AR2315_CONFIG_CPU_MMR              0x00040000
#define AR2315_CONFIG_BIG                  0x00000400


/*
 * NMI control
 */
#define AR2315_NMI_CTL          (AR2315_DSLBASE + 0x0010)

#define AR2315_NMI_EN  1

/*
 * Revision Register - Initial value is 0x3010 (WMAC 3.0, AR531X 1.0).
 */
#define AR2315_SREV             (AR2315_DSLBASE + 0x0014)

#define AR2315_REV_MAJ                     0x00f0
#define AR2315_REV_MAJ_S                   4
#define AR2315_REV_MIN                     0x000f
#define AR2315_REV_MIN_S                   0
#define AR2315_REV_CHIP                    (AR2315_REV_MAJ|AR2315_REV_MIN)

/*
 * Interface Enable
 */
#define AR2315_IF_CTL           (AR2315_DSLBASE + 0x0018)

#define AR2315_IF_MASK                     0x00000007
#define AR2315_IF_DISABLED                 0
#define AR2315_IF_PCI                      1
#define AR2315_IF_TS_LOCAL                 2
#define AR2315_IF_ALL                      3   /* only for emulation with separate pins */
#define AR2315_IF_LOCAL_HOST               0x00000008
#define AR2315_IF_PCI_HOST                 0x00000010
#define AR2315_IF_PCI_INTR                 0x00000020
#define AR2315_IF_PCI_CLK_MASK             0x00030000
#define AR2315_IF_PCI_CLK_INPUT            0
#define AR2315_IF_PCI_CLK_OUTPUT_LOW       1
#define AR2315_IF_PCI_CLK_OUTPUT_CLK       2
#define AR2315_IF_PCI_CLK_OUTPUT_HIGH      3
#define AR2315_IF_PCI_CLK_SHIFT            16

/*
 * APB Interrupt control
 */

#define AR2315_ISR              (AR2315_DSLBASE + 0x0020)
#define AR2315_IMR              (AR2315_DSLBASE + 0x0024)
#define AR2315_GISR             (AR2315_DSLBASE + 0x0028)

#define AR2315_ISR_UART0                   0x0001           /* high speed UART */
#define AR2315_ISR_I2C_RSVD                0x0002           /* I2C bus */
#define AR2315_ISR_SPI                     0x0004           /* SPI bus */
#define AR2315_ISR_AHB                     0x0008           /* AHB error */
#define AR2315_ISR_APB                     0x0010           /* APB error */
#define AR2315_ISR_TIMER                   0x0020           /* timer */
#define AR2315_ISR_GPIO                    0x0040           /* GPIO */
#define AR2315_ISR_WD                      0x0080           /* watchdog */
#define AR2315_ISR_IR_RSVD                 0x0100           /* IR */

#define AR2315_GISR_MISC                   0x0001
#define AR2315_GISR_WLAN0                  0x0002
#define AR2315_GISR_MPEGTS_RSVD            0x0004
#define AR2315_GISR_LOCALPCI               0x0008
#define AR2315_GISR_WMACPOLL               0x0010
#define AR2315_GISR_TIMER                  0x0020
#define AR2315_GISR_ETHERNET               0x0040

/*
 * Interrupt routing from IO to the processor IP bits
 * Define our inter mask and level
 */
#define AR2315_INTR_MISCIO      SR_IBIT3
#define AR2315_INTR_WLAN0       SR_IBIT4
#define AR2315_INTR_ENET0       SR_IBIT5
#define AR2315_INTR_LOCALPCI    SR_IBIT6
#define AR2315_INTR_WMACPOLL    SR_IBIT7
#define AR2315_INTR_COMPARE     SR_IBIT8

/*
 * Timers
 */
#define AR2315_TIMER            (AR2315_DSLBASE + 0x0030)
#define AR2315_RELOAD           (AR2315_DSLBASE + 0x0034)
#define AR2315_WD               (AR2315_DSLBASE + 0x0038)
#define AR2315_WDC              (AR2315_DSLBASE + 0x003c)

#define AR2315_WDC_IGNORE_EXPIRATION       0x00000000
#define AR2315_WDC_NMI                     0x00000001               /* NMI on watchdog */
#define AR2315_WDC_RESET                   0x00000002               /* reset on watchdog */

/*
 * CPU Performance Counters
 */
#define AR2315_PERFCNT0         (AR2315_DSLBASE + 0x0048)
#define AR2315_PERFCNT1         (AR2315_DSLBASE + 0x004c)

#define AR2315_PERF0_DATAHIT                0x0001  /* Count Data Cache Hits */
#define AR2315_PERF0_DATAMISS               0x0002  /* Count Data Cache Misses */
#define AR2315_PERF0_INSTHIT                0x0004  /* Count Instruction Cache Hits */
#define AR2315_PERF0_INSTMISS               0x0008  /* Count Instruction Cache Misses */
#define AR2315_PERF0_ACTIVE                 0x0010  /* Count Active Processor Cycles */
#define AR2315_PERF0_WBHIT                  0x0020  /* Count CPU Write Buffer Hits */
#define AR2315_PERF0_WBMISS                 0x0040  /* Count CPU Write Buffer Misses */

#define AR2315_PERF1_EB_ARDY                0x0001  /* Count EB_ARdy signal */
#define AR2315_PERF1_EB_AVALID              0x0002  /* Count EB_AValid signal */
#define AR2315_PERF1_EB_WDRDY               0x0004  /* Count EB_WDRdy signal */
#define AR2315_PERF1_EB_RDVAL               0x0008  /* Count EB_RdVal signal */
#define AR2315_PERF1_VRADDR                 0x0010  /* Count valid read address cycles */
#define AR2315_PERF1_VWADDR                 0x0020  /* Count valid write address cycles */
#define AR2315_PERF1_VWDATA                 0x0040  /* Count valid write data cycles */

/*
 * AHB Error Reporting.
 */
#define AR2315_AHB_ERR0         (AR2315_DSLBASE + 0x0050)  /* error  */
#define AR2315_AHB_ERR1         (AR2315_DSLBASE + 0x0054)  /* haddr  */
#define AR2315_AHB_ERR2         (AR2315_DSLBASE + 0x0058)  /* hwdata */
#define AR2315_AHB_ERR3         (AR2315_DSLBASE + 0x005c)  /* hrdata */
#define AR2315_AHB_ERR4         (AR2315_DSLBASE + 0x0060)  /* status */

#define AHB_ERROR_DET               1   /* AHB Error has been detected,          */
                                        /* write 1 to clear all bits in ERR0     */
#define AHB_ERROR_OVR               2   /* AHB Error overflow has been detected  */
#define AHB_ERROR_WDT               4   /* AHB Error due to wdt instead of hresp */

#define AR2315_PROCERR_HMAST               0x0000000f
#define AR2315_PROCERR_HMAST_DFLT          0
#define AR2315_PROCERR_HMAST_WMAC          1
#define AR2315_PROCERR_HMAST_ENET          2
#define AR2315_PROCERR_HMAST_PCIENDPT      3
#define AR2315_PROCERR_HMAST_LOCAL         4
#define AR2315_PROCERR_HMAST_CPU           5
#define AR2315_PROCERR_HMAST_PCITGT        6

#define AR2315_PROCERR_HMAST_S             0
#define AR2315_PROCERR_HWRITE              0x00000010
#define AR2315_PROCERR_HSIZE               0x00000060
#define AR2315_PROCERR_HSIZE_S             5
#define AR2315_PROCERR_HTRANS              0x00000180
#define AR2315_PROCERR_HTRANS_S            7
#define AR2315_PROCERR_HBURST              0x00000e00
#define AR2315_PROCERR_HBURST_S            9

/*
 * Clock Control
 */
#define AR2315_PLLC_CTL         (AR2315_DSLBASE + 0x0064)
#define AR2315_PLLV_CTL         (AR2315_DSLBASE + 0x0068)
#define AR2315_CPUCLK           (AR2315_DSLBASE + 0x006c)
#define AR2315_AMBACLK          (AR2315_DSLBASE + 0x0070)
#define AR2315_SYNCCLK          (AR2315_DSLBASE + 0x0074)
#define AR2315_DSL_SLEEP_CTL    (AR2315_DSLBASE + 0x0080)
#define AR2315_DSL_SLEEP_DUR    (AR2315_DSLBASE + 0x0084)

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

/*
 * GPIO
 */
#define AR2315_GPIO_DI          (AR2315_DSLBASE + 0x0088)
#define AR2315_GPIO_DO          (AR2315_DSLBASE + 0x0090)
#define AR2315_GPIO_CR          (AR2315_DSLBASE + 0x0098)
#define AR2315_GPIO_INT         (AR2315_DSLBASE + 0x00a0)

#define AR2315_GPIO_CR_M(x)                (1 << (x))                  /* mask for i/o */
#define AR2315_GPIO_CR_O(x)                (1 << (x))                  /* output */
#define AR2315_GPIO_CR_I(x)                (0)                         /* input */

#define AR2315_GPIO_INT_S(x)               (x)                         /* interrupt enable */
#define AR2315_GPIO_INT_M                  (0x3F)                      /* mask for int */
#define AR2315_GPIO_INT_LVL(x)             ((x) << 6)                  /* interrupt level */
#define AR2315_GPIO_INT_LVL_M              ((0x3) << 6)                /* mask for int level */

#define AR2315_GPIO_INT_MAX_Y				1   /* Maximum value of Y for AR5313_GPIO_INT_* macros */
#define AR2315_GPIO_INT_LVL_OFF				0   /* Triggerring off */
#define AR2315_GPIO_INT_LVL_LOW				1   /* Low Level Triggered */
#define AR2315_GPIO_INT_LVL_HIGH			2   /* High Level Triggered */
#define AR2315_GPIO_INT_LVL_EDGE			3   /* Edge Triggered */

#define AR2315_RESET_GPIO       5
#define AR2315_NUM_GPIO         22

/*
 *  PCI Clock Control
 */
#define AR2315_PCICLK           (AR2315_DSLBASE + 0x00a4)

#define AR2315_PCICLK_INPUT_M              0x3
#define AR2315_PCICLK_INPUT_S              0

#define AR2315_PCICLK_PLLC_CLKM            0
#define AR2315_PCICLK_PLLC_CLKM1           1
#define AR2315_PCICLK_PLLC_CLKC            2
#define AR2315_PCICLK_REF_CLK              3

#define AR2315_PCICLK_DIV_M                0xc
#define AR2315_PCICLK_DIV_S                2

#define AR2315_PCICLK_IN_FREQ              0
#define AR2315_PCICLK_IN_FREQ_DIV_6        1
#define AR2315_PCICLK_IN_FREQ_DIV_8        2
#define AR2315_PCICLK_IN_FREQ_DIV_10       3

/*
 * Observation Control Register
 */
#define AR2315_OCR              (AR2315_DSLBASE + 0x00b0)
#define OCR_GPIO0_IRIN              0x0040
#define OCR_GPIO1_IROUT             0x0080
#define OCR_GPIO3_RXCLR             0x0200

/*
 *  General Clock Control
 */

#define AR2315_MISCCLK          (AR2315_DSLBASE + 0x00b4)
#define MISCCLK_PLLBYPASS_EN        0x00000001
#define MISCCLK_PROCREFCLK          0x00000002

/*
 * SDRAM Controller
 *   - No read or write buffers are included.
 */
#define AR2315_MEM_CFG          (AR2315_SDRAMCTL + 0x00)
#define AR2315_MEM_CTRL         (AR2315_SDRAMCTL + 0x0c)
#define AR2315_MEM_REF          (AR2315_SDRAMCTL + 0x10)

#define SDRAM_DATA_WIDTH_M          0x00006000
#define SDRAM_DATA_WIDTH_S          13

#define SDRAM_COL_WIDTH_M           0x00001E00
#define SDRAM_COL_WIDTH_S           9

#define SDRAM_ROW_WIDTH_M           0x000001E0
#define SDRAM_ROW_WIDTH_S           5

#define SDRAM_BANKADDR_BITS_M       0x00000018
#define SDRAM_BANKADDR_BITS_S       3

/*
 * SPI Flash Interface Registers
 */

#define AR2315_SPI_CTL      (AR2315_SPI + 0x00)
#define AR2315_SPI_OPCODE   (AR2315_SPI + 0x04)
#define AR2315_SPI_DATA     (AR2315_SPI + 0x08)

#define SPI_CTL_START           0x00000100
#define SPI_CTL_BUSY            0x00010000
#define SPI_CTL_TXCNT_MASK      0x0000000f
#define SPI_CTL_RXCNT_MASK      0x000000f0
#define SPI_CTL_TX_RX_CNT_MASK  0x000000ff
#define SPI_CTL_SIZE_MASK       0x00060000

#define SPI_CTL_CLK_SEL_MASK    0x03000000
#define SPI_OPCODE_MASK         0x000000ff

/*
 * PCI Bus Interface Registers
 */
#define AR2315_PCI_1MS_REG      (AR2315_PCI + 0x0008)
#define AR2315_PCI_1MS_MASK     0x3FFFF         /* # of AHB clk cycles in 1ms */

#define AR2315_PCI_MISC_CONFIG  (AR2315_PCI + 0x000c)
#define AR2315_PCIMISC_TXD_EN   0x00000001      /* Enable TXD for fragments */
#define AR2315_PCIMISC_CFG_SEL  0x00000002      /* mem or config cycles */
#define AR2315_PCIMISC_GIG_MASK 0x0000000C      /* bits 31-30 for pci req */
#define AR2315_PCIMISC_RST_MODE 0x00000030
#define AR2315_PCIRST_INPUT     0x00000000      /* 4:5=0 rst is input */
#define AR2315_PCIRST_LOW       0x00000010      /* 4:5=1 rst to GND */
#define AR2315_PCIRST_HIGH      0x00000020      /* 4:5=2 rst to VDD */
#define AR2315_PCIGRANT_EN      0x00000000      /* 6:7=0 early grant en */
#define AR2315_PCIGRANT_FRAME   0x00000040      /* 6:7=1 grant waits 4 frame */
#define AR2315_PCIGRANT_IDLE    0x00000080      /* 6:7=2 grant waits 4 idle */
#define AR2315_PCIGRANT_GAP     0x00000000      /* 6:7=2 grant waits 4 idle */
#define AR2315_PCICACHE_DIS     0x00001000      /* PCI external access cache disable */

#define AR2315_PCI_OUT_TSTAMP   (AR2315_PCI + 0x0010)

#define AR2315_PCI_UNCACHE_CFG  (AR2315_PCI + 0x0014)

#define AR2315_PCI_IN_EN        (AR2315_PCI + 0x0100)
#define AR2315_PCI_IN_EN0       0x01            /* Enable chain 0 */
#define AR2315_PCI_IN_EN1       0x02            /* Enable chain 1 */
#define AR2315_PCI_IN_EN2       0x04            /* Enable chain 2 */
#define AR2315_PCI_IN_EN3       0x08            /* Enable chain 3 */

#define AR2315_PCI_IN_DIS       (AR2315_PCI + 0x0104)
#define AR2315_PCI_IN_DIS0      0x01            /* Disable chain 0 */
#define AR2315_PCI_IN_DIS1      0x02            /* Disable chain 1 */
#define AR2315_PCI_IN_DIS2      0x04            /* Disable chain 2 */
#define AR2315_PCI_IN_DIS3      0x08            /* Disable chain 3 */

#define AR2315_PCI_IN_PTR       (AR2315_PCI + 0x0200)

#define AR2315_PCI_OUT_EN       (AR2315_PCI + 0x0400)
#define AR2315_PCI_OUT_EN0      0x01            /* Enable chain 0 */

#define AR2315_PCI_OUT_DIS      (AR2315_PCI + 0x0404)
#define AR2315_PCI_OUT_DIS0     0x01            /* Disable chain 0 */

#define AR2315_PCI_OUT_PTR      (AR2315_PCI + 0x0408)

#define AR2315_PCI_INT_STATUS   (AR2315_PCI + 0x0500)   /* write one to clr */
#define AR2315_PCI_TXINT        0x00000001      /* Desc In Completed */
#define AR2315_PCI_TXOK         0x00000002      /* Desc In OK */
#define AR2315_PCI_TXERR        0x00000004      /* Desc In ERR */
#define AR2315_PCI_TXEOL        0x00000008      /* Desc In End-of-List */
#define AR2315_PCI_RXINT        0x00000010      /* Desc Out Completed */
#define AR2315_PCI_RXOK         0x00000020      /* Desc Out OK */
#define AR2315_PCI_RXERR        0x00000040      /* Desc Out ERR */
#define AR2315_PCI_RXEOL        0x00000080      /* Desc Out EOL */
#define AR2315_PCI_TXOOD        0x00000200      /* Desc In Out-of-Desc */
#define AR2315_PCI_MASK         0x0000FFFF      /* Desc Mask */
#define AR2315_PCI_EXT_INT      0x02000000
#define AR2315_PCI_ABORT_INT    0x04000000

#define AR2315_PCI_INT_MASK     (AR2315_PCI + 0x0504)   /* same as INT_STATUS */

#define AR2315_PCI_INTEN_REG    (AR2315_PCI + 0x0508)
#define AR2315_PCI_INT_DISABLE  0x00            /* disable pci interrupts */
#define AR2315_PCI_INT_ENABLE   0x01            /* enable pci interrupts */

#define AR2315_PCI_HOST_IN_EN   (AR2315_PCI + 0x0800)
#define AR2315_PCI_HOST_IN_DIS  (AR2315_PCI + 0x0804)
#define AR2315_PCI_HOST_IN_PTR  (AR2315_PCI + 0x0810)
#define AR2315_PCI_HOST_OUT_EN  (AR2315_PCI + 0x0900)
#define AR2315_PCI_HOST_OUT_DIS (AR2315_PCI + 0x0904)
#define AR2315_PCI_HOST_OUT_PTR (AR2315_PCI + 0x0908)


/*
 * Local Bus Interface Registers
 */
#define AR2315_LB_CONFIG        (AR2315_LOCAL + 0x0000)
#define AR2315_LBCONF_OE        0x00000001      /* =1 OE is low-true */
#define AR2315_LBCONF_CS0       0x00000002      /* =1 first CS is low-true */
#define AR2315_LBCONF_CS1       0x00000004      /* =1 2nd CS is low-true */
#define AR2315_LBCONF_RDY       0x00000008      /* =1 RDY is low-true */
#define AR2315_LBCONF_WE        0x00000010      /* =1 Write En is low-true */
#define AR2315_LBCONF_WAIT      0x00000020      /* =1 WAIT is low-true */
#define AR2315_LBCONF_ADS       0x00000040      /* =1 Adr Strobe is low-true */
#define AR2315_LBCONF_MOT       0x00000080      /* =0 Intel, =1 Motorola */
#define AR2315_LBCONF_8CS       0x00000100      /* =1 8 bits CS, 0= 16bits */
#define AR2315_LBCONF_8DS       0x00000200      /* =1 8 bits Data S, 0=16bits */
#define AR2315_LBCONF_ADS_EN    0x00000400      /* =1 Enable ADS */
#define AR2315_LBCONF_ADR_OE    0x00000800      /* =1 Adr cap on OE, WE or DS */
#define AR2315_LBCONF_ADDT_MUX  0x00001000      /* =1 Adr and Data share bus */
#define AR2315_LBCONF_DATA_OE   0x00002000      /* =1 Data cap on OE, WE, DS */
#define AR2315_LBCONF_16DATA    0x00004000      /* =1 Data is 16 bits wide */
#define AR2315_LBCONF_SWAPDT    0x00008000      /* =1 Byte swap data */
#define AR2315_LBCONF_SYNC      0x00010000      /* =1 Bus synchronous to clk */
#define AR2315_LBCONF_INT       0x00020000      /* =1 Intr is low true */
#define AR2315_LBCONF_INT_CTR0  0x00000000      /* GND high-Z, Vdd is high-Z */
#define AR2315_LBCONF_INT_CTR1  0x00040000      /* GND drive, Vdd is high-Z */
#define AR2315_LBCONF_INT_CTR2  0x00080000      /* GND high-Z, Vdd drive */
#define AR2315_LBCONF_INT_CTR3  0x000C0000      /* GND drive, Vdd drive */
#define AR2315_LBCONF_RDY_WAIT  0x00100000      /* =1 RDY is negative of WAIT */
#define AR2315_LBCONF_INT_PULSE 0x00200000      /* =1 Interrupt is a pulse */
#define AR2315_LBCONF_ENABLE    0x00400000      /* =1 Falcon respond to LB */

#define AR2315_LB_CLKSEL        (AR2315_LOCAL + 0x0004)
#define AR2315_LBCLK_EXT        0x0001          /* use external clk for lb */

#define AR2315_LB_1MS           (AR2315_LOCAL + 0x0008)
#define AR2315_LB1MS_MASK       0x3FFFF         /* # of AHB clk cycles in 1ms */

#define AR2315_LB_MISCCFG       (AR2315_LOCAL + 0x000C)
#define AR2315_LBM_TXD_EN       0x00000001      /* Enable TXD for fragments */
#define AR2315_LBM_RX_INTEN     0x00000002      /* Enable LB ints on RX ready */
#define AR2315_LBM_MBOXWR_INTEN 0x00000004      /* Enable LB ints on mbox wr */
#define AR2315_LBM_MBOXRD_INTEN 0x00000008      /* Enable LB ints on mbox rd */
#define AR2315_LMB_DESCSWAP_EN  0x00000010      /* Byte swap desc enable */
#define AR2315_LBM_TIMEOUT_MASK 0x00FFFF80
#define AR2315_LBM_TIMEOUT_SHFT 7
#define AR2315_LBM_PORTMUX      0x07000000


#define AR2315_LB_RXTSOFF       (AR2315_LOCAL + 0x0010)

#define AR2315_LB_TX_CHAIN_EN   (AR2315_LOCAL + 0x0100)
#define AR2315_LB_TXEN_0        0x01
#define AR2315_LB_TXEN_1        0x02
#define AR2315_LB_TXEN_2        0x04
#define AR2315_LB_TXEN_3        0x08

#define AR2315_LB_TX_CHAIN_DIS  (AR2315_LOCAL + 0x0104)
#define AR2315_LB_TX_DESC_PTR   (AR2315_LOCAL + 0x0200)

#define AR2315_LB_RX_CHAIN_EN   (AR2315_LOCAL + 0x0400)
#define AR2315_LB_RXEN          0x01

#define AR2315_LB_RX_CHAIN_DIS  (AR2315_LOCAL + 0x0404)
#define AR2315_LB_RX_DESC_PTR   (AR2315_LOCAL + 0x0408)

#define AR2315_LB_INT_STATUS    (AR2315_LOCAL + 0x0500)
#define AR2315_INT_TX_DESC      0x0001
#define AR2315_INT_TX_OK        0x0002
#define AR2315_INT_TX_ERR       0x0004
#define AR2315_INT_TX_EOF       0x0008
#define AR2315_INT_RX_DESC      0x0010
#define AR2315_INT_RX_OK        0x0020
#define AR2315_INT_RX_ERR       0x0040
#define AR2315_INT_RX_EOF       0x0080
#define AR2315_INT_TX_TRUNC     0x0100
#define AR2315_INT_TX_STARVE    0x0200
#define AR2315_INT_LB_TIMEOUT   0x0400
#define AR2315_INT_LB_ERR       0x0800
#define AR2315_INT_MBOX_WR      0x1000
#define AR2315_INT_MBOX_RD      0x2000

/* Bit definitions for INT MASK are the same as INT_STATUS */
#define AR2315_LB_INT_MASK      (AR2315_LOCAL + 0x0504)

#define AR2315_LB_INT_EN        (AR2315_LOCAL + 0x0508)
#define AR2315_LB_MBOX          (AR2315_LOCAL + 0x0600)

/*
 * IR Interface Registers
 */
#define AR2315_IR_PKTDATA                   (AR2315_IR + 0x0000)

#define AR2315_IR_PKTLEN                    (AR2315_IR + 0x07fc) /* 0 - 63 */

#define AR2315_IR_CONTROL                   (AR2315_IR + 0x0800)
#define AR2315_IRCTL_TX                     0x00000000  /* use as tranmitter */
#define AR2315_IRCTL_RX                     0x00000001  /* use as receiver   */
#define AR2315_IRCTL_SAMPLECLK_MASK         0x00003ffe  /* Sample clk divisor mask */
#define AR2315_IRCTL_SAMPLECLK_SHFT                  1
#define AR2315_IRCTL_OUTPUTCLK_MASK         0x03ffc000  /* Output clk divisor mask */
#define AR2315_IRCTL_OUTPUTCLK_SHFT                 14

#define AR2315_IR_STATUS                    (AR2315_IR + 0x0804)
#define AR2315_IRSTS_RX                     0x00000001  /* receive in progress */
#define AR2315_IRSTS_TX                     0x00000002  /* transmit in progress */

#define AR2315_IR_CONFIG                    (AR2315_IR + 0x0808)
#define AR2315_IRCFG_INVIN                  0x00000001  /* invert input polarity */
#define AR2315_IRCFG_INVOUT                 0x00000002  /* invert output polarity */
#define AR2315_IRCFG_SEQ_START_WIN_SEL      0x00000004  /* 1 => 28, 0 => 7 */
#define AR2315_IRCFG_SEQ_START_THRESH       0x000000f0  /*  */
#define AR2315_IRCFG_SEQ_END_UNIT_SEL       0x00000100  /*  */
#define AR2315_IRCFG_SEQ_END_UNIT_THRESH    0x00007e00  /*  */
#define AR2315_IRCFG_SEQ_END_WIN_SEL        0x00008000  /*  */
#define AR2315_IRCFG_SEQ_END_WIN_THRESH     0x001f0000  /*  */
#define AR2315_IRCFG_NUM_BACKOFF_WORDS      0x01e00000  /*  */

#define HOST_PCI_DEV_ID         3
#define HOST_PCI_MBAR0          0x10000000
#define HOST_PCI_MBAR1          0x20000000
#define HOST_PCI_MBAR2          0x30000000

#define HOST_PCI_SDRAM_BASEADDR HOST_PCI_MBAR1
#define PCI_DEVICE_MEM_SPACE    0x800000

#endif /* __AR2315_REG_H */

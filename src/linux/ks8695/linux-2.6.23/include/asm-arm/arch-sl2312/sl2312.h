#ifndef __sl2312_h
#define __sl2312_h

/****************************************************************************
 * Copyright  Storlink Corp 2002-2003.  All rights reserved.                *
 *--------------------------------------------------------------------------*
 * Name:board.s                                                             *
 * Description:  SL231x specfic define                                      *
 * Author: Plus Chen                                                        *
 * Version: 0.9 Create
 ****************************************************************************/

/*
  CPE address map;

               +====================================================
    0x00000000 | FLASH
    0x0FFFFFFF |
               |====================================================
    0x10000000 | SDRAM
    0x1FFFFFFF |
               |====================================================
    0x20000000 | Global Registers        0x20000000-0x20FFFFFF
               | EMAC and DMA            0x21000000-0x21FFFFFF
               | UART Module             0x22000000-0x22FFFFFF
               | Timer Module            0x23000000-0x23FFFFFF
               | Interrupt Module        0x24000000-0x24FFFFFF
               | RTC Module              0x25000000-0x25FFFFFF
               | LPC Host Controller     0x26000000-0x26FFFFFF
               | LPC Peripherial IO      0x27000000-0x27FFFFFF
               | WatchDog Timer          0x28000000-0x28FFFFFF
    0x2FFFFFFF | Reserved                0x29000000-0x29FFFFFF
               |=====================================================
    0x30000000 | PCI IO, Configuration Registers
    0x3FFFFFFF |
               |=====================================================
    0x40000000 | PCI Memory
    0x4FFFFFFF |
               |=====================================================
    0x50000000 | Ethernet MAC and DMA    0x50000000-0x50FFFFFF
               | Security and DMA        0x51000000-0x51FFFFFF
               | IDE Channel 0 Register  0x52000000-0x527FFFFF
               | IDE Channel 1 Register  0x52800000-0x52FFFFFF
               | USB Register            0x53000000-0x53FFFFFF
               | Flash Controller        0x54000000-0x54FFFFFF
               | DRAM Controller         0x55000000-0x55FFFFFF
    0x5FFFFFFF | Reserved                0x56000000-0x5FFFFFFF
               |=====================================================
    0x60000000 | Reserved
    0x6FFFFFFF |
               |=====================================================
    0x70000000 | FLASH shadow Memory
    0x7FFFFFFF |
               |=====================================================
    0x80000000 | Big Endian of memory    0x00000000-0x7FFFFFFF
    0xFFFFFFFF |
               +=====================================================
*/



/*-------------------------------------------------------------------------------
 Memory Map definitions
-------------------------------------------------------------------------------- */
#define TEST		1
#if 0

static inline int GETCPUID()
{
       int cpuid;
      __asm__(
"mrc p8, 0, r0, c0, c0, 0\n"
"mov %0, r0"
       :"=r"(cpuid)
       :
       :"r0");
       return (cpuid & 0x07);
}
#endif
#define SL2312_SRAM_BASE                0x70000000       //  SRAM base after remap
#define SL2312_DRAM_BASE                0x00000000       //  DRAM base after remap
#define SL2312_RAM_BASE                 0x10000000       //  RAM code base before remap
#define SL2312_FLASH_BASE         	    0x30000000
#define SL2312_ROM_BASE                 0x30000000
#define SL2312_GLOBAL_BASE              0x40000000
#define SL2312_WAQTCHDOG_BASE           0x41000000
#define SL2312_UART_BASE                0x42000000
#define SL2312_TIMER_BASE               0x43000000
#define SL2312_LCD_BASE                 0x44000000
#define SL2312_RTC_BASE                 0x45000000
#define SL2312_SATA_BASE                0x46000000
#define SL2312_LPC_HOST_BASE            0x47000000
#define SL2312_LPC_IO_BASE              0x47800000
// #define SL2312_INTERRUPT_BASE           0x48000000
#define SL2312_INTERRUPT0_BASE          0x48000000
#define SL2312_INTERRUPT1_BASE          0x49000000
//#define SL2312_INTERRUPT_BASE		((getcpuid()==0)?SL2312_INTERRUPT0_BASE:SL2312_INTERRUPT1_BASE)
#define SL2312_INTERRUPT_BASE		    0x48000000
#define SL2312_SSP_CTRL_BASE            0x4A000000
#define SL2312_POWER_CTRL_BASE          0x4B000000
#define SL2312_CIR_BASE                 0x4C000000
#define SL2312_GPIO_BASE                0x4D000000
#define SL2312_GPIO_BASE1               0x4E000000
#define SL2312_GPIO_BASE2               0x4F000000
#define SL2312_PCI_IO_BASE              0x50000000
#define SL2312_PCI_MEM_BASE             0x58000000
#ifdef  CONFIG_NET_SL351X
#define SL2312_TOE_BASE                 0x60000000
#define SL2312_GMAC0_BASE               0x6000A000
#define SL2312_GMAC1_BASE               0x6000E000
#else
#define SL2312_GMAC0_BASE               0x60000000
#define SL2312_GMAC1_BASE               0x61000000
#endif
#define SL2312_SECURITY_BASE            0x62000000
#define SL2312_IDE0_BASE                0x63000000
#define SL2312_IDE1_BASE		        0x63400000
#define SL2312_RAID_BASE                0x64000000
#define SL2312_FLASH_CTRL_BASE          0x65000000
#define SL2312_DRAM_CTRL_BASE           0x66000000
#define SL2312_GENERAL_DMA_BASE         0x67000000
#define SL2312_USB_BASE                 0x68000000
#define SL2312_USB0_BASE                0x68000000
#define SL2312_USB1_BASE                0x69000000
#define SL2312_FLASH_SHADOW             0x30000000
#define SL2312_BIG_ENDIAN_BASE			0x80000000

#ifdef CONFIG_GEMINI_IPI
#define CPU_1_MEM_BASE			0x4000000				// 64 MB
#define CPU_1_DATA_OFFSET		0x4000000-0x300000		// Offset 61 MB
#endif

#define SL2312_TIMER1_BASE              SL2312_TIMER_BASE
#define SL2312_TIMER2_BASE              (SL2312_TIMER_BASE + 0x10)
#define SL2312_TIMER3_BASE              (SL2312_TIMER_BASE + 0x20)

#define SL2312_PCI_DMA_MEM1_BASE		0x00000000
#define SL2312_PCI_DMA_MEM2_BASE		0x00000000
#define SL2312_PCI_DMA_MEM3_BASE		0x00000000
#define SL2312_PCI_DMA_MEM1_SIZE		7
#define SL2312_PCI_DMA_MEM2_SIZE		6
#define SL2312_PCI_DMA_MEM3_SIZE		6

/*-------------------------------------------------------------------------------
 Global Module
---------------------------------------------------------------------------------*/
#define GLOBAL_ID                       0x00
#define GLOBAL_CHIP_ID                  0x002311
#define GLOBAL_CHIP_REV                 0xA0
#define GLOBAL_STATUS                   0x04
#define GLOBAL_CONTROL                  0x1C
#define GLOBAL_REMAP_BIT                0x01
#define GLOBAL_RESET_REG		0x0C
#define GLOBAL_MISC_REG					0x30
#define PFLASH_SHARE_BIT				0x02

#define GLOBAL_RESET		(1<<31)
#define RESET_CPU1			(1<<30)
#define RESET_SATA1			(1<<27)
#define RESET_SATA0			(1<<26)
#define RESET_CIR			(1<<25)
#define RESET_EXT_DEV		(1<<24)
#define RESET_WD			(1<<23)
#define RESET_GPIO2			(1<<22)
#define RESET_GPIO1			(1<<21)
#define RESET_GPIO0			(1<<20)
#define RESET_SSP			(1<<19)
#define RESET_UART			(1<<18)
#define RESET_TIMER			(1<<17)
#define RESET_RTC			(1<<16)
#define RESET_INT0			(1<<15)
#define RESET_INT1			(1<<14)
#define RESET_LCD			(1<<13)
#define RESET_LPC			(1<<12)
#define RESET_APB			(1<<11)
#define RESET_DMA			(1<<10)
#define RESET_USB1			(1<<9 )
#define RESET_USB0			(1<<8 )
#define RESET_PCI			(1<<7 )
#define RESET_GMAC1			(1<<6 )
#define RESET_GMAC0			(1<<5 )
#define RESET_IPSEC			(1<<4 )
#define RESET_RAID			(1<<3 )
#define RESET_IDE			(1<<2 )
#define RESET_FLASH			(1<<1 )
#define RESET_DRAM			(1<<0 )








/*-------------------------------------------------------------------------------
 DRAM Module
---------------------------------------------------------------------------------*/
#define DRAM_SIZE_32M                   0x2000000
#define DRAM_SIZE_64M                   0x4000000
#define DRAM_SIZE_128M                  0x8000000

#define DRAM_SIZE                       DRAM_SIZE_128M

#define DRAM_SDRMR                      0x00
#define SDRMR_DISABLE_DLL               0x80010000

/*------------------------------------------------------------------------------
 Share Pin Flag
--------------------------------------------------------------------------------*/
#ifdef CONFIG_SL2312_SHARE_PIN
#define FLASH_SHARE_BIT                    0
#define UART_SHARE_BIT                     1
#define EMAC_SHARE_BIT                     2
#define IDE_RW_SHARE_BIT                   3
#define IDE_CMD_SHARE_BIT                  4
#endif
/*-------------------------------------------------------------------------------
 System Clock
---------------------------------------------------------------------------------*/

#ifndef SYS_CLK
#ifdef CONFIG_SL3516_ASIC
#define SYS_CLK                         150000000
#else
#define SYS_CLK                     	20000000
#endif
#endif

#define AHB_CLK                     	SYS_CLK
#define MAX_TIMER                   	3
#ifndef APB_CLK
#ifdef CONFIG_SL3516_ASIC
#define APB_CLK                     	(SYS_CLK / 6)
#else
#define APB_CLK				SYS_CLK
#endif
#endif

#ifdef CONFIG_SL3516_ASIC
#define UART_CLK                        48000000	// 30000000 for GeminiA chip, else 48000000
#else
#define UART_CLK			48000000
#endif

#define SL2312_BAUD_115200              (UART_CLK / 1843200)
#define SL2312_BAUD_57600               (UART_CLK / 921600)
#define SL2312_BAUD_38400		        (UART_CLK / 614400)
#define SL2312_BAUD_19200               (UART_CLK / 307200)
#define SL2312_BAUD_14400               (UART_CLK / 230400)
#define SL2312_BAUD_9600                (UART_CLK / 153600)

#endif



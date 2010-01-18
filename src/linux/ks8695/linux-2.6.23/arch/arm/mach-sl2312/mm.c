/*
 *  linux/arch/arm/mach-epxa10db/mm.c
 *
 *  MM routines for Altera'a Epxa10db board
 *
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/sizes.h>

#include <asm/mach/map.h>

/* Page table mapping for I/O region */
static struct map_desc sl2312_io_desc[] __initdata = {
#ifdef CONFIG_GEMINI_IPI
{__phys_to_virt(CPU_1_MEM_BASE),         __phys_to_pfn(CPU_1_MEM_BASE),        SZ_64M,  MT_MEMORY},
#endif
{IO_ADDRESS(SL2312_SRAM_BASE),         __phys_to_pfn(SL2312_SRAM_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_DRAM_CTRL_BASE),    __phys_to_pfn(SL2312_DRAM_CTRL_BASE),   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GLOBAL_BASE),       __phys_to_pfn(SL2312_GLOBAL_BASE),      SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_WAQTCHDOG_BASE),    __phys_to_pfn(SL2312_WAQTCHDOG_BASE),   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_UART_BASE),         __phys_to_pfn(SL2312_UART_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_TIMER_BASE),        __phys_to_pfn(SL2312_TIMER_BASE),       SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_LCD_BASE),          __phys_to_pfn(SL2312_LCD_BASE),         SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_RTC_BASE),          __phys_to_pfn(SL2312_RTC_BASE),         SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_SATA_BASE),         __phys_to_pfn(SL2312_SATA_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_LPC_HOST_BASE),     __phys_to_pfn(SL2312_LPC_HOST_BASE),    SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_LPC_IO_BASE),       __phys_to_pfn(SL2312_LPC_IO_BASE),      SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_INTERRUPT_BASE),    __phys_to_pfn(SL2312_INTERRUPT_BASE),   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_INTERRUPT1_BASE),   __phys_to_pfn(SL2312_INTERRUPT1_BASE),  SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_SSP_CTRL_BASE),     __phys_to_pfn(SL2312_SSP_CTRL_BASE),    SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_POWER_CTRL_BASE),   __phys_to_pfn(SL2312_POWER_CTRL_BASE),  SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_CIR_BASE),          __phys_to_pfn(SL2312_CIR_BASE),         SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GPIO_BASE),         __phys_to_pfn(SL2312_GPIO_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GPIO_BASE1),        __phys_to_pfn(SL2312_GPIO_BASE1),       SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GPIO_BASE2),        __phys_to_pfn(SL2312_GPIO_BASE2),       SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_PCI_IO_BASE),	   __phys_to_pfn(SL2312_PCI_IO_BASE),      SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_PCI_MEM_BASE),	   __phys_to_pfn(SL2312_PCI_MEM_BASE),	   SZ_512K,  MT_DEVICE},
#ifdef CONFIG_NET_SL351X
{IO_ADDRESS(SL2312_TOE_BASE),    	__phys_to_pfn(SL2312_TOE_BASE)       ,   SZ_512K,  MT_DEVICE},
#endif
{IO_ADDRESS(SL2312_GMAC0_BASE),	       __phys_to_pfn(SL2312_GMAC0_BASE),	   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GMAC1_BASE),	       __phys_to_pfn(SL2312_GMAC1_BASE),	   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_SECURITY_BASE),     __phys_to_pfn(SL2312_SECURITY_BASE),    SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_IDE0_BASE),         __phys_to_pfn(SL2312_IDE0_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_IDE1_BASE),         __phys_to_pfn(SL2312_IDE1_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_RAID_BASE),         __phys_to_pfn(SL2312_RAID_BASE),        SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_FLASH_CTRL_BASE),   __phys_to_pfn(SL2312_FLASH_CTRL_BASE),  SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_DRAM_CTRL_BASE),    __phys_to_pfn(SL2312_DRAM_CTRL_BASE),   SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_GENERAL_DMA_BASE),  __phys_to_pfn(SL2312_GENERAL_DMA_BASE), SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_USB0_BASE),         __phys_to_pfn(SL2312_USB_BASE),         SZ_512K,  MT_DEVICE},
{IO_ADDRESS(SL2312_USB1_BASE),         __phys_to_pfn(SL2312_USB1_BASE),        SZ_512K,  MT_DEVICE},
{FLASH_VADDR(SL2312_FLASH_BASE),       __phys_to_pfn(SL2312_FLASH_BASE),       SZ_16M,    MT_DEVICE},
};

void __init sl2312_map_io(void)
{
	iotable_init(sl2312_io_desc, ARRAY_SIZE(sl2312_io_desc));
}

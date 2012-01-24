
/*
 * Prom setup file for ar7240
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/serial_reg.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include "ar7240.h"

int __ath_flash_size;


void __init prom_init(void)
{

    printk ("flash_size passed from bootloader = %d\n", (int)fw_arg3);
    __ath_flash_size = fw_arg3;


    mips_machtype  = MACH_ATHEROS_AR7240;

}

void __init prom_free_prom_memory(void)
{
}





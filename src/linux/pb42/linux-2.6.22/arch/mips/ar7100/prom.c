
/*
 * Prom setup file for ar7100
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include "ar7100.h"

#define sysRegRead(phys)	\
	(*(volatile u32 *)KSEG1ADDR(phys))

void __init prom_init(void)
{

	/* 
	 * if user passes kernel args, ignore the default one 
	 */

	mips_machgroup = MACH_GROUP_AR7100;
	mips_machtype  = MACH_ATHEROS_AP81;

}

void __init prom_free_prom_memory(void)
{
}

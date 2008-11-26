/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *  Copyright (C) 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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

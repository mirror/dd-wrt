/*
 * prom.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
 * mainly based on Atheros LSDK Code, some code taken from OpenWrt and ATH79 tree
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

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

#include <asm/mach-ar71xx/ar71xx.h>
#include <asm/mach-ar71xx/ar933x_uart.h>

void __init prom_init(void)
{
	mips_machtype = MACH_ATHEROS_AR7240;
}

void __init prom_free_prom_memory(void)
{
}

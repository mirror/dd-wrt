/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2005 infineon
 *   Copyright (C) 2007 John Crispin <blogic@openwrt.org>
 *
 */
#ifndef _IFXMIPS_VPE_H__
#define _IFXMIPS_VPE_H__

/* For the explanation of the APIs please refer the section "MT APRP Kernel
 * Programming" in AR9 SW Architecture Specification
 */
int32_t vpe1_sw_start(void* sw_start_addr, uint32_t tcmask, uint32_t flags);
int32_t vpe1_sw_stop(uint32_t flags);
uint32_t vpe1_get_load_addr (uint32_t flags);
uint32_t vpe1_get_max_mem (uint32_t flags);

int32_t vpe1_set_boot_param(char *field, char *value, char flags);
int32_t vpe1_get_boot_param(char *field, char **value, char flags);

/* Watchdog APIs */
extern unsigned long vpe1_wdog_ctr;
extern unsigned long vpe1_wdog_timeout;

unsigned long vpe1_sw_wdog_start(unsigned long);
unsigned long vpe1_sw_wdog_stop(unsigned long);

typedef int (*VPE_SW_WDOG_RESET)(unsigned long wdog_cleared_ok_count);
int32_t vpe1_sw_wdog_register_reset_handler(VPE_SW_WDOG_RESET reset_fn);

#endif

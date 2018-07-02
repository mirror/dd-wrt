/*
<:copyright-BRCM:2015:GPL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/sched.h>
#include <linux/sizes.h>

#ifndef CONFIG_MIPS
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#endif

#include <bcm_map_part.h>
#include <board.h>
#if !defined(CONFIG_BCM96858)
#include <pmc_drv.h>
#endif

extern unsigned long memsize;
extern bool is_memory_reserved;
extern int rsvd_mem_cnt;
extern reserve_mem_t reserve_mem[TOTAL_RESERVE_MEM_NUM];
extern unsigned long reserved_mem_total;
extern void check_if_rootfs_is_set(char *cmdline);
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
extern void bl_xparms_setup(const unsigned char* blparms, unsigned int size);
#endif

#define BRCM_BLPARMS_PROP "brcm_blparms"

static char* dt_scan_mem_str[] = {
#if IS_ENABLED(CONFIG_BCM_ADSL)
    ADSL_BASE_ADDR_STR,
#endif
#if IS_ENABLED(CONFIG_BCM_RDPA)
    PARAM1_BASE_ADDR_STR,
    PARAM2_BASE_ADDR_STR,
#endif
#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
    DHD_BASE_ADDR_STR,
    DHD_BASE_ADDR_STR_1,
    DHD_BASE_ADDR_STR_2,
#endif
#if defined(CONFIG_BCM960333)
    PLC_BASE_ADDR_STR,
#endif
    NULL
};

#ifdef CONFIG_OF
extern const void *__init of_get_flat_dt_prop(unsigned long node, const char *name,
                                       int *size);

static int __init dt_get_memory_prop(unsigned long node, uint64_t* base, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;

    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return -1;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
        *base = dt_mem_next_cell(dt_root_addr_cells, &reg);
        *size = dt_mem_next_cell(dt_root_size_cells, &reg);
    }

    return 0;
}

/* This function scan through the device tree blob for any system related node.
 * data such as memory size, reserved memory, blparms and etc. Device related 
 * node should be handled by the corresponding device driver.
 */
int __init bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    uint64_t base, size;
    int i = 0, blsize = 0;
    char rsvd_mem_str[64];
    uint8_t* bcm_blparms_buf = (uint8_t*)bcm_get_blparms();
    uint8_t* blparms = NULL;

    if ( node == of_get_flat_dt_root() ) { /* root node */ 
         blparms = (uint8_t*)of_get_flat_dt_prop(node, BRCM_BLPARMS_PROP, &blsize);
         if( blparms ) {
            memcpy(bcm_blparms_buf, blparms, blsize);
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
            bl_xparms_setup(bcm_blparms_buf, blsize);
#endif
        }
    }

    if ( strncmp(uname, "memory", 6) == 0 ) {
        const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
        if( type != NULL && strcmp(type, "memory") == 0 && dt_get_memory_prop(node, &base, &size) == 0 ) {
            memsize = size;
        }
    }

    while( dt_scan_mem_str[i] ) {
        sprintf(rsvd_mem_str, "%s%s", DT_RSVD_PREFIX_STR, dt_scan_mem_str[i]);
        if ( strcmp(uname, rsvd_mem_str) == 0 ) {
            if( dt_get_memory_prop(node, &base, &size) == 0 ) {
                if( base == 0 || size == 0 ) {// this entry from dtb is not valid, ignore it
                    printk("Error:incomplete rsvd mem entry base %lld size %lld for %s\n", base, size, rsvd_mem_str); 
                    break;
                }
                strcpy(reserve_mem[rsvd_mem_cnt].name, dt_scan_mem_str[i]);
                reserve_mem[rsvd_mem_cnt].phys_addr = (uint32_t)base;
                reserve_mem[rsvd_mem_cnt].size = (uint32_t)size;
                reserve_mem[rsvd_mem_cnt].mapped = of_get_flat_dt_prop(node, "no-map", NULL) == NULL;
                is_memory_reserved = true;
		reserved_mem_total += size;
                rsvd_mem_cnt++;
            }
            break;
        }
        i++;
    }

    return 0;
}

void __init bcm_scan_fdt(void)
{
        memset(reserve_mem, 0x0, sizeof(reserve_mem_t)*TOTAL_RESERVE_MEM_NUM);
	of_scan_flat_dt(bcm_early_scan_dt, NULL);
}

EXPORT_SYMBOL(bcm_scan_fdt);
#endif

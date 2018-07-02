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
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <bcm_map_part.h>
#include <board.h>
#include <pmc_drv.h>

extern int rsvd_mem_cnt;
extern reserve_mem_t reserve_mem[TOTAL_RESERVE_MEM_NUM];
extern void check_if_rootfs_is_set(char *cmdline);
extern int bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data);

#ifdef CONFIG_OF

static int __init bcm_create_reserved_memory_mapping(void)
{
    int i;
    void* virt;
    uint32_t phys, size;

    for(i = 0; i < rsvd_mem_cnt; i++ ) {
        if (reserve_mem[i].mapped == 0) {
            phys = reserve_mem[i].phys_addr;
            virt = phys_to_virt(phys);
            size = reserve_mem[i].size;

            create_pgd_mapping(&init_mm, (phys_addr_t)phys, (unsigned long)virt, size,  __pgprot(PROT_NORMAL_NC));
            reserve_mem[i].mapped = 1;
            printk("creating mapping for reserved memory phys 0x%08x virt 0x%p size 0x%08x for %s\n", phys, virt, (uint32_t)size, reserve_mem[i].name);
        } else {
            printk("Do not need to create mapping for reserved memory phys "
                   "0x%08x size 0x%08x for %s\n", reserve_mem[i].phys_addr,
                   reserve_mem[i].size, reserve_mem[i].name);
        }
    }

    return 0;
}

#endif

static void bcm_sys_restart(enum reboot_mode reboot_mode, const char *cmd)
{
#if !defined(CONFIG_BRCM_IKOS)
    kerSysSoftReset();
#endif
}

static int __init bcm_arch_early_init(void)
{
    /* replace with PSCI in future release */
    arm_pm_restart = bcm_sys_restart;

    memset((void*)bcm_get_blparms(), 0x0, bcm_get_blparms_size());
#ifdef CONFIG_OF
    memset(reserve_mem, 0x0, sizeof(reserve_mem_t)*TOTAL_RESERVE_MEM_NUM);
    of_scan_flat_dt(bcm_early_scan_dt, NULL);

    bcm_create_reserved_memory_mapping();
#endif
    check_if_rootfs_is_set(boot_command_line);

#if !defined(CONFIG_BRCM_IKOS)
    pmc_init();

    kerSysEarlyFlashInit();
    kerSysFlashInit();
#endif

    return 0;
}

early_initcall(bcm_arch_early_init);

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
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <bcm_mm.h>

#ifdef CONFIG_BCM_CFE_XARGS_EARLY
#define BLPARMS_SIZE CONFIG_BCM_CFE_XARGS_EARLY_SIZE 
#else
#define BLPARMS_SIZE 1024 
#endif

unsigned char g_blparms_buf[BLPARMS_SIZE];
unsigned long memsize = SZ_16M;
reserve_mem_t reserve_mem[TOTAL_RESERVE_MEM_NUM];
unsigned long reserved_mem_total = 0;
int rsvd_mem_cnt = 0;
bool is_memory_reserved = false;
bool is_rootfs_set = false;


/***************************************************************************
 * C++ New and delete operator functions
 ***************************************************************************/

/* void *operator new(unsigned int sz) */
void *_Znwj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* void *operator new[](unsigned int sz)*/
void *_Znaj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* placement new operator */
/* void *operator new (unsigned int size, void *ptr) */
void *ZnwjPv(unsigned int size, void *ptr)
{
    return ptr;
}

/* void operator delete(void *m) */
void _ZdlPv(void *m)
{
    kfree(m);
}

/* void operator delete[](void *m) */
void _ZdaPv(void *m)
{
    kfree(m);
}
EXPORT_SYMBOL(_Znwj);
EXPORT_SYMBOL(_Znaj);
EXPORT_SYMBOL(ZnwjPv);
EXPORT_SYMBOL(_ZdlPv);
EXPORT_SYMBOL(_ZdaPv);


void __init check_if_rootfs_is_set(char *cmdline)
{
    char *cmd_ptr;

    cmd_ptr = strstr(cmdline, "root=");
    if (cmd_ptr != NULL)
        is_rootfs_set = true;
}

const unsigned char* bcm_get_blparms(void)
{
    return g_blparms_buf;
}

EXPORT_SYMBOL(bcm_get_blparms);

unsigned int  bcm_get_blparms_size(void)
{
    return BLPARMS_SIZE;
}

EXPORT_SYMBOL(bcm_get_blparms_size);

bool kerSysIsRootfsSet(void)
{
    return is_rootfs_set;
}
EXPORT_SYMBOL(kerSysIsRootfsSet);

unsigned long getMemorySize(void)
{
    return memsize;
}
EXPORT_SYMBOL(getMemorySize);

#if IS_ENABLED(CONFIG_BCM_ADSL)

/***************************************************************************
 * Function Name: kerSysGetDslPhyMemory
 * Description  : return the start address of the reserved DSL SDRAM. The memory
 * 		  is reserved in the arch dependent setup.c
 * Returns      : physical address of the reserved DSL SDRAM
 ***************************************************************************/
void *kerSysGetDslPhyMemory(void)
{
    void* virt = NULL;
    uint32_t size, phys;

    if( BcmMemReserveGetByName(ADSL_BASE_ADDR_STR, &virt, &size) == 0 ) {
#if defined(CONFIG_ARM64) || defined(CONFIG_ARM)
        phys = virt_to_phys(virt);
#else
        phys = VIRT_TO_PHYS(virt);
#endif
        return (void *)phys;
    } else
        return NULL;
}

EXPORT_SYMBOL(kerSysGetDslPhyMemory);
#endif

int BcmMemReserveGetByName(char *name, void **addr, unsigned int *size)
{
    int i;

    *addr = NULL;
    *size = 0;

    if (is_memory_reserved == false)
        return -1;

    for (i = 0; i < rsvd_mem_cnt; i++) {
        if (strcmp(name, reserve_mem[i].name) == 0) {
#if defined(CONFIG_ARM64) || defined(CONFIG_ARM)
            *addr = phys_to_virt(reserve_mem[i].phys_addr);
#else
            *addr = (void*)PHYS_TO_UNCACHED(reserve_mem[i].phys_addr);
#endif
            *size =  reserve_mem[i].size;
            return 0;
        }
    }

    return -1;
}
EXPORT_SYMBOL(BcmMemReserveGetByName);

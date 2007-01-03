/*
<:copyright-gpl 
 Copyright 2004 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/*
 * prom.c: PROM library initialization code.
 *
 */
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <bcm_map_part.h>
#include <board.h>
#include "boardparms.h"
#include "softdsl/AdslCoreDefs.h"


extern int  do_syslog(int, char *, int);
extern void __init InitNvramInfo( void );
extern void kerSysFlashInit( void );
extern unsigned long get_nvram_start_addr(void);

void __init create_root_nfs_cmdline( char *cmdline );
UINT32 __init calculateCpuSpeed(void);

/*
 * Defines representing the memory chunks allocated for the DSP module. These defines will be set during
 * the build process.
 */
#define DSP_CORE_SIZE 0
#define DSP_INIT_SIZE 0

/* Pointers to memory buffers allocated for the DSP module */
void *dsp_core;
void *dsp_init;
EXPORT_SYMBOL(dsp_core);
EXPORT_SYMBOL(dsp_init);

void __init allocDspModBuffers(void);

const char *get_system_type(void)
{
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
    return( pNvramData->szBoardId );
}

unsigned long getMemorySize(void)
{
    unsigned long size;
    unsigned long memCfg;

    size = 1;
    memCfg = MEMC->Config;
    /* Number of column bits */
    size <<= (((memCfg & MEMC_COL_MASK) >> MEMC_COL_SHFT) + 8);
    /* Plus number of row bits */
    size <<= (((memCfg & MEMC_ROW_MASK) >> MEMC_ROW_SHFT) + 11);
    /* Plus bus width */
    if (((memCfg & MEMC_WIDTH_MASK) >> MEMC_WIDTH_SHFT) == MEMC_32BIT_BUS)
        size <<= 2;
    else
        size <<= 1;

    /* Plus number of banks */
    size <<= 2;

    return( size );
}

/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */
void __init prom_init(void)
{

    kerSysFlashInit();
    do_syslog(8, NULL, 8);

    printk( "%s prom init\n", get_system_type() );

    PERF->IrqMask = 0;

    arcs_cmdline[0] = '\0';

#if defined(CONFIG_ROOT_NFS)
    create_root_nfs_cmdline( arcs_cmdline );
#elif defined(CONFIG_ROOT_FLASHFS)
    strcpy(arcs_cmdline, CONFIG_ROOT_FLASHFS);
    strcat(arcs_cmdline, " ");
#endif
//    strcat(arcs_cmdline, CONFIG_CMDLINE);

    add_memory_region(0, (getMemorySize() - ADSL_SDRAM_IMAGE_SIZE), BOOT_MEM_RAM);

    /* Count register increments every other clock */
//    mips_hpt_frequency = calculateCpuSpeed() / 2;
//    r4k_interval = mips_hpt_frequency / HZ;

    mips_machgroup = MACH_GROUP_BRCM;
    mips_machtype = MACH_BCM963XX;
}

/* --------------------------------------------------------------------------
    Name: prom_free_prom_memory
Abstract: 
 -------------------------------------------------------------------------- */
void __init prom_free_prom_memory(void)
{

}


#if defined(CONFIG_ROOT_NFS)
/* This function reads in a line that looks something like this:
 *
 *
 * CFE bootline=bcmEnet(0,0)host:vmlinux e=192.169.0.100:ffffff00 h=192.169.0.1
 *
 *
 * and retuns in the cmdline parameter some that looks like this:
 *
 * CONFIG_CMDLINE="root=/dev/nfs nfsroot=192.168.0.1:/opt/targets/96345R/fs
 * ip=192.168.0.100:192.168.0.1::255.255.255.0::eth0:off rw"
 */
#define BOOT_LINE_ADDR   0x0
#define HEXDIGIT(d) ((d >= '0' && d <= '9') ? (d - '0') : ((d | 0x20) - 'W'))
#define HEXBYTE(b)  (HEXDIGIT((b)[0]) << 4) + HEXDIGIT((b)[1])

void __init create_root_nfs_cmdline( char *cmdline )
{
    char root_nfs_cl[] = "root=/dev/nfs nfsroot=%s:" CONFIG_ROOT_NFS_DIR
        " ip=%s:%s::%s::eth0:off rw";

    char *localip = NULL;
    char *hostip = NULL;
    char mask[16] = "";
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
    char bootline[128] = "";
    char *p = bootline;

    memcpy(bootline, pNvramData->szBootline, sizeof(bootline));

    while( *p )
    {
        if( p[0] == 'e' && p[1] == '=' )
        {
            /* Found local ip address */
            p += 2;
            localip = p;
            while( *p && *p != ' ' && *p != ':' )
                p++;
            if( *p == ':' )
            {
                /* Found network mask (eg FFFFFF00 */
                *p++ = '\0';
                sprintf( mask, "%u.%u.%u.%u", HEXBYTE(p), HEXBYTE(p + 2),
                HEXBYTE(p + 4), HEXBYTE(p + 6) );
                p += 4;
            }
            else if( *p == ' ' )
                *p++ = '\0';
        }
        else if( p[0] == 'h' && p[1] == '=' )
        {
            /* Found host ip address */
            p += 2;
            hostip = p;
            while( *p && *p != ' ' )
                p++;
            if( *p == ' ' )
                    *p++ = '\0';
        }
        else 
            p++;
    }

    if( localip && hostip ) 
        sprintf( cmdline, root_nfs_cl, hostip, localip, hostip, mask );
}
#endif

/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM6358 CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
#if defined(CONFIG_BCM96338)
UINT32 __init calculateCpuSpeed(void)
{
    return 240000000;
}
#endif

#if defined(CONFIG_BCM96348)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;

    UINT32 pllStrap = PERF->PllStrap;
    int n1 = (pllStrap & PLL_N1_MASK) >> PLL_N1_SHFT;
    int n2 = (pllStrap & PLL_N2_MASK) >> PLL_N2_SHFT;
    int m1cpu = (pllStrap & PLL_M1_CPU_MASK) >> PLL_M1_CPU_SHFT;

    cpu_speed = (16 * (n1 + 1) * (n2 + 2) / (m1cpu + 1)) * 1000000;

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96358)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;

    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;
    int numerator;

    cpu_speed = 64000000 / ((pllConfig & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);
    numerator = (((pllConfig & MIPSDDR_N2_MASK) >> MIPSDDR_N2_SHFT) * ((pllConfig & MIPSDDR_N1_MASK) >> MIPSDDR_N1_SHFT));
    cpu_speed = (cpu_speed * numerator) / 4;

    return cpu_speed;
}
#endif

/*
*****************************************************************************
** FUNCTION:   allocDspModBuffers
**
** PURPOSE:    Allocates buffers for the init and core sections of the DSP
**             module. This module is special since it has to be allocated
**             in the 0x800.. memory range which is not mapped by the TLB.
**
** PARAMETERS: None
** RETURNS:    Nothing
*****************************************************************************
*/
void __init allocDspModBuffers(void)
{
#if defined(CONFIG_BCM96358) && defined(CONFIG_BCM_ENDPOINT_MODULE)
    printk("Allocating memory for DSP module core and initialization code\n");

	dsp_core = (void*)((DSP_CORE_SIZE > 0) ? alloc_bootmem((unsigned long)DSP_CORE_SIZE) : NULL);
	dsp_init = (void*)((DSP_INIT_SIZE > 0) ? alloc_bootmem((unsigned long)DSP_INIT_SIZE) : NULL);

	printk("Allocated DSP module memory - CORE=0x%x SIZE=%d, INIT=0x%x SIZE=%d\n", 
	       (unsigned int)dsp_core, DSP_CORE_SIZE, (unsigned int)dsp_init , DSP_INIT_SIZE);
#endif
}

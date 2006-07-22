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
extern void serial_init(void);
extern void __init InitNvramInfo( void );
extern void kerSysFlashInit( void );
extern unsigned long get_nvram_start_addr(void);
void __init create_root_nfs_cmdline( char *cmdline );

#if defined(CONFIG_BCM96338)
#define CPU_CLOCK                   240000000
#define MACH_BCM                    MACH_BCM96338
#endif
#if defined(CONFIG_BCM96345)
#define CPU_CLOCK                   140000000
#define MACH_BCM                    MACH_BCM96345
#endif
#if defined(CONFIG_BCM96348)
void __init calculateCpuSpeed(void);
static unsigned long cpu_speed;
#define CPU_CLOCK                   cpu_speed
#define MACH_BCM                    MACH_BCM96348
#endif

const char *get_system_type(void)
{
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();

    return( pNvramData->szBoardId );
}

unsigned long getMemorySize(void)
{
    unsigned long ulSdramType = BOARD_SDRAM_TYPE;

    unsigned long ulSdramSize;

    switch( ulSdramType )
    {
    case BP_MEMORY_16MB_1_CHIP:
    case BP_MEMORY_16MB_2_CHIP:
        ulSdramSize = 16 * 1024 * 1024;
        break;
    case BP_MEMORY_32MB_1_CHIP:
    case BP_MEMORY_32MB_2_CHIP:
        ulSdramSize = 32 * 1024 * 1024;
        break;
    case BP_MEMORY_64MB_2_CHIP:
        ulSdramSize = 64 * 1024 * 1024;
        break;
    default:
        ulSdramSize = 8 * 1024 * 1024;
        break;
    }

    return ulSdramSize;
}

/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */
void __init prom_init(void)
{
    extern ulong r4k_interval;

    serial_init();

    kerSysFlashInit();

    do_syslog(8, NULL, 8);

    printk( "%s prom init\n", get_system_type() );

    PERF->IrqMask = 0;

    arcs_cmdline[0] = '\0';

#if defined(CONFIG_ROOT_NFS)
    create_root_nfs_cmdline( arcs_cmdline );
#elif defined(CONFIG_ROOT_FLASHFS)
    strcpy(arcs_cmdline, CONFIG_ROOT_FLASHFS);
#endif

    add_memory_region(0, (getMemorySize() - ADSL_SDRAM_IMAGE_SIZE), BOOT_MEM_RAM);

#if defined(CONFIG_BCM96348)
    calculateCpuSpeed();
#endif
    /* Count register increments every other clock */
    r4k_interval = CPU_CLOCK / HZ / 2;
    mips_hpt_frequency = CPU_CLOCK / 2;

    mips_machgroup = MACH_GROUP_BRCM;
    mips_machtype = MACH_BCM;
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
extern unsigned long get_nvram_start_addr(void);

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

#if defined(CONFIG_BCM96348)
/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM6348 CPU speed by reading the PLL strap register
    *      and applying the following formula:
    *      cpu_clk = (.25 * 64MHz freq) * (N1 + 1) * (N2 + 2) / (M1_CPU + 1)
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
void __init calculateCpuSpeed(void)
{
    UINT32 pllStrap = PERF->PllStrap;
    int n1 = (pllStrap & PLL_N1_MASK) >> PLL_N1_SHFT;
    int n2 = (pllStrap & PLL_N2_MASK) >> PLL_N2_SHFT;
    int m1cpu = (pllStrap & PLL_M1_CPU_MASK) >> PLL_M1_CPU_SHFT;

    cpu_speed = (16 * (n1 + 1) * (n2 + 2) / (m1cpu + 1)) * 1000000;
}
#endif


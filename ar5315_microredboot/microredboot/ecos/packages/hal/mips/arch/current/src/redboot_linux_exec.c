//==========================================================================
//
//      redboot_linux_exec.c
//
//      RedBoot exec command for Linux booting
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2004 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    t@keshi.org
// Contributors: t@keshi.org, jskov, dwmw2
// Date:         2001-07-09
// Purpose:      RedBoot exec command for Linux booting
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <redboot.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#define xstr(s) str(s)
#define str(s...) #s

typedef struct
{
	char *name;
	char *val;
} t_env_var;

struct parmblock {
	t_env_var memsize;
	t_env_var modetty0;
	t_env_var ethaddr;
	t_env_var env_end;
	char *argv[2];
	char text[0];
};

static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute an image", 
            "[-b <argv addr>] [-c \"kernel command line\"] [-w <timeout>]\n"
	    "        [<entry point>]",
            do_exec
    );

static void 
do_exec(int argc, char *argv[])
{
    cyg_uint32 entry = (cyg_uint32)entry_address?:CYGDAT_REDBOOT_MIPS_LINUX_BOOT_ENTRY;
    cyg_uint32 base_addr = CYGDAT_REDBOOT_MIPS_LINUX_BOOT_ARGV_ADDR;
    char *cmd_line = xstr(CYGDAT_REDBOOT_MIPS_LINUX_BOOT_COMMAND_LINE);
    bool base_addr_set, cmd_line_set, wait_time_set;
    int wait_time, res;
    char line[8];
    
    struct option_info opts[3];
    char *pcmd;
    struct parmblock *pb;
    void (*tt)(int a, char **b, void *c);
    int oldints;
    hal_virtual_comm_table_t *__chan;
    int baud;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, &base_addr_set, "base address");
    init_opts(&opts[1], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[2], 'c', true, OPTION_ARG_TYPE_STR, 
              (void **)&cmd_line, &cmd_line_set, "kernel command line");
    
    if (!scan_opts(argc, argv, 1, opts, 3, (void *)&entry, 
                   OPTION_ARG_TYPE_NUM, "entry address"))
        return;
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }

    tt = (void *)entry;

    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    baud = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_GETBAUD);

    diag_printf("Now booting linux kernel:\n");
    diag_printf(" Base address 0x%08x Entry 0x%08x\n", base_addr, entry);
    diag_printf(" Cmdline : %s\n", cmd_line);

    if (wait_time_set) {
        diag_printf("About to start execution at %p - abort with ^C within %d seconds\n",
                    (void *)entry, wait_time);
        res = _rb_gets(line, sizeof(line), wait_time*1000);
        if (res == _GETS_CTRLC) {
            return;
        }
    }
    
    HAL_DISABLE_INTERRUPTS(oldints);

#ifdef CYGPKG_IO_ETH_DRIVERS
    eth_drv_stop();
#endif

    pb = (struct parmblock *)base_addr;
    pcmd = pb->text;

    pb->memsize.name = pcmd;
    pcmd += diag_sprintf(pcmd, "memsize");
    pb->memsize.val = ++pcmd;
    pcmd += diag_sprintf(pcmd, "0x%08x", (ram_end - ram_start + 0xFFFFF) & ~0xFFFFF);

    pb->modetty0.name = ++pcmd;
    pcmd += diag_sprintf(pcmd, "modetty0");
    pb->modetty0.val = ++pcmd;
    pcmd += diag_sprintf(pcmd, "%d,n,8,1,hw", baud);

#ifdef CYGPKG_REDBOOT_NETWORKING
    pb->ethaddr.name = ++pcmd;
    pcmd += diag_sprintf(pcmd, "ethaddr");
    pb->ethaddr.val = ++pcmd;
    pcmd += diag_sprintf(pcmd, "%02x.%02x.%02x.%02x.%02x.%02x",
                         __local_enet_addr[0], __local_enet_addr[1],
                         __local_enet_addr[2], __local_enet_addr[3],
                         __local_enet_addr[4], __local_enet_addr[5]);
    pb->env_end.name = NULL;
    pb->env_end.val = NULL;
#else
    pb->ethaddr.name = NULL;
    pb->ethaddr.val = NULL;
#endif

    /* Point argv[0] at a handy `\0` */
    pb->argv[0] = pcmd;
    pb->argv[1] = ++pcmd;
    
    strcpy(pcmd, cmd_line);
    
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();

    tt(2, pb->argv, pb);
}

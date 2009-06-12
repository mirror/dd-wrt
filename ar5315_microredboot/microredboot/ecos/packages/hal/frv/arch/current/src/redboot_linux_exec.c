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
// Date:         2003-11-13
// Purpose:      RedBoot exec command for Linux booting, from MIPS arch
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

static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute an image", 
            "[-c \"kernel command line\"] [-w <timeout>]\n"
	    "        [<entry point>]",
            do_exec
    );

static void 
do_exec(int argc, char *argv[])
{
    cyg_uint32 entry = (cyg_uint32)entry_address?:CYGDAT_REDBOOT_FRV_LINUX_BOOT_ENTRY;
    char *cmd_line = xstr(CYGDAT_REDBOOT_FRV_LINUX_BOOT_COMMAND_LINE);
    bool cmd_line_set, wait_time_set;
    int wait_time, res;
    char line[8];
    
    struct option_info opts[3];
    void (*linux_boot)(unsigned long, char *);
    int oldints;
    hal_virtual_comm_table_t *__chan;
    int baud;

    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_STR, 
              (void **)&cmd_line, &cmd_line_set, "kernel command line");
    
    if (!scan_opts(argc, argv, 1, opts, 2, (void *)&entry, 
                   OPTION_ARG_TYPE_NUM, "entry address"))
        return;

    linux_boot = (void *)entry;

    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    baud = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_GETBAUD);

    diag_printf("Now booting linux kernel:\n");
    diag_printf(" Entry 0x%08x\n", entry);
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
   
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();

    linux_boot(0xdead1eaf, cmd_line);
}

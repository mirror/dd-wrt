//==========================================================================
//
//      redboot_linux_exec.c
//
//      AM33 Linux specific RedBoot exec command
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
// Author(s):    dhowells
// Contributors: gthomas, jskov, msalter
// Date:         2001-07-19
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

// Exported CLI function(s)

// The exec command works like the go command in YAMON - that is,
// providing callee with argc, argv, and env. In addition to the
// information provided by the user, 

static void do_exec(int argc, char *argv[]);

static char exec_usage[] =
            "[-w timeout] [-c \"kernel command line\"] [<entry_point>]";

RedBoot_cmd("exec", 
            "Execute an image - with MMU off", 
            exec_usage,
            do_exec
	    );

typedef void (*code_fun)(void) __attribute__((noreturn));

static void 
do_exec(int argc, char *argv[])
{
    unsigned long oldints;
    bool wait_time_set;
    int  wait_time, res;
    bool cmd_line_set;
    struct option_info opts[4];
    code_fun entry;
    char line[8];
    char *cmd_line;
    int num_options;

    entry = (code_fun)entry_address;  // Default from last 'load' operation
    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_STR, 
              (void **)&cmd_line, (bool *)&cmd_line_set, "kernel command line");
    num_options = 2;

    if (!scan_opts(argc, argv, 1, opts, num_options, (void *)&entry, 
                   OPTION_ARG_TYPE_NUM, "starting address"))
    {
        return;
    }
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }
    if (cmd_line_set) {
	memcpy((char*)CYGHWR_REDBOOT_AM33_LINUX_CMD_ADDRESS,"cmdline:",8);
        strncpy((char*)CYGHWR_REDBOOT_AM33_LINUX_CMD_ADDRESS+8,cmd_line,256);
	*(char*)(CYGHWR_REDBOOT_AM33_LINUX_CMD_ADDRESS+8+256) = 0;
    }
    else {
	*(char*)(CYGHWR_REDBOOT_AM33_LINUX_CMD_ADDRESS+256) = 0;
    }

    if (wait_time_set) {
        diag_printf("About to start execution at %p - abort with ^C within %d seconds\n",
                    (void *)entry, wait_time);
        res = _rb_gets(line, sizeof(line), wait_time*1000);
        if (res == _GETS_CTRLC) {
            return;
        }
    }

#ifdef CYGPKG_IO_ETH_DRIVERS
    eth_drv_stop();
#endif

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    (*entry)();
}

// EOF redboot_linux_exec.c

//==========================================================================
//
//      redboot_linux_exec.c
//
//      RedBoot command to boot Linux
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
//####OTHERCOPYRIGHTBEGIN####
//
//  The structure definitions below are taken from include/asm-/redboot.h in
//  the Linux kernel, Copyright (c) 2002, 2003 Gary Thomas, Copyright (c) 1997 Dan Malek. 
//  Their presence here is for the express purpose of communication with the Linux 
//  kernel being booted and is considered 'fair use' by the original author and
//  are included with their permission.
//
//####OTHERCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: gthomas,msalter
// Date:         2002-01-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_REDBOOT_NETWORKING
#include <net/net.h>
#endif

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/redboot_linux_exec.h>

//=========================================================================

// Exported CLI function(s)
static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute a Linux image - with MMU off", 
            "[-w timeout]\n"
            "        [-c \"kernel command line\"] [<entry_point>]",
            do_exec
    );

//
// Execute a Linux kernel - this is a RedBoot CLI command
//
static void 
do_exec(int argc, char *argv[])
{
    unsigned long entry;
    bool wait_time_set, cmd_line_set;
    int  wait_time;
    char *cmd_line;
    char *cline;
    struct option_info opts[2];
    hal_virtual_comm_table_t *__chan;
    int baud_rate;

    bd_t *board_info;
    CYG_INTERRUPT_STATE oldints;
    unsigned long sp = CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE;
    
    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void *)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_STR, 
              (void *)&cmd_line, (bool *)&cmd_line_set, "kernel command line");
    entry = entry_address;  // Default from last 'load' operation
    if (!scan_opts(argc, argv, 1, opts, 2, (void *)&entry, OPTION_ARG_TYPE_NUM, 
                   "[physical] starting address")) {
        return;
    }
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }

    // Determine baud rate on current console
    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    baud_rate = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_GETBAUD);
    if (baud_rate <= 0) {
        baud_rate = CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD;
    }

    // Make a little space at the top of the stack, and align to
    // 64-bit boundary.
    sp = (sp-128) & ~7;  // The Linux boot code uses this space for FIFOs
    
    // Copy the commandline onto the stack, and set the SP to just below it.
    if (cmd_line_set) {
	int len,i;

	// get length of string
	for( len = 0; cmd_line[len] != '\0'; len++ );

	// decrement sp by length of string and align to
	// word boundary.
	sp = (sp-(len+1)) & ~3;

	// assign this SP value to command line start
	cline = (char *)sp;

	// copy command line over.
	for( i = 0; i < len; i++ )
	    cline[i] = cmd_line[i];
	cline[len] = '\0';

    } else {
        cline = (char *)NULL;
    }
    
    // Set up parameter struct at top of stack
    sp = sp-sizeof(bd_t);
    board_info = (bd_t *)sp;
    memset(board_info, sizeof(*board_info), 0);
    
    board_info->bi_tag		= 0x42444944;
    board_info->bi_size		= sizeof(board_info);
    board_info->bi_revision	= 1;
    board_info->bi_bdate	= 0x06012002;
    board_info->bi_memstart	= CYGMEM_REGION_ram;
    board_info->bi_memsize	= CYGMEM_REGION_ram_SIZE;
    board_info->bi_baudrate     = baud_rate;
    board_info->bi_cmdline      = cline;
#ifdef CYGPKG_REDBOOT_NETWORKING
    memcpy(board_info->bi_enetaddr, __local_enet_addr, sizeof(enet_addr_t));
#endif
    // Call platform specific code to fill in the platform/architecture specific details
    plf_redboot_linux_exec(board_info);

    // adjust SP to 64 byte boundary, and leave a little space
    // between it and the commandline for PowerPC calling
    // conventions.
	
    sp = (sp-64)&~63;

    if (wait_time_set) {
        int script_timeout_ms = wait_time * 1000;
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
        unsigned char *hold_script = script;
        script = (unsigned char *)0;
#endif
        diag_printf("About to start execution at %p - abort with ^C within %d seconds\n",
                    (void *)entry, wait_time);
        while (script_timeout_ms >= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT) {
            int res;
            char line[80];
            res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
            if (res == _GETS_CTRLC) {
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
                script = hold_script;  // Re-enable script
#endif
                return;
            }
            script_timeout_ms -= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT;
        }
    }

#ifdef CYGPKG_IO_ETH_DRIVERS
    eth_drv_stop();
#endif

    // Disable interrupts
    HAL_DISABLE_INTERRUPTS(oldints);

    // Put the caches to sleep.
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();

//    diag_printf("entry %08x, sp %08x, info %08x, cmd line %08x, baud %d\n",
//		entry, sp, board_info, cline, baud_rate);
//    breakpoint();
    
    // Call into Linux
    __asm__ volatile (        
	               // Start by disabling MMU - the mappings are
	               // 1-1 so this should not cause any problems
	               "mfmsr	3\n"
		       "li      4,0xFFFFFFCF\n"
		       "and	3,3,4\n"
		       "sync\n"
		       "mtmsr	3\n"
		       "sync\n"

		       // Now set up parameters to jump into linux

		       "mtlr	%0\n"		// set entry address in LR
		       "mr	1,%1\n"		// set stack pointer
		       "mr	3,%2\n"		// set board info in R3
		       "mr	4,%3\n"		// set command line in R4
		       "blr          \n"	// jump into linux
		       :
		       : "r"(entry),"r"(sp),"r"(board_info),"r"(cline)
		       : "r3", "r4"
	             
	             );
}

//=========================================================================
// EOF redboot_linux_exec.c

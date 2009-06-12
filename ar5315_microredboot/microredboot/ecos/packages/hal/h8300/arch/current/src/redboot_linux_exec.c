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
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-05-28
// Purpose:      RedBoot exec command for uClinux booting
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <redboot.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#define xstr(s) str(s)
#define str(s...) #s

#if defined(CYGDAT_REDBOOT_H8300_LINUX_COMMAND_START)
static void 
do_exec(int argc, char *argv[])
{
    cyg_uint32 entry = CYGDAT_REDBOOT_H8300_LINUX_BOOT_ENTRY;
    cyg_uint32 command_addr = CYGDAT_REDBOOT_H8300_LINUX_COMMAND_START;
    char *cmd_line = xstr( CYGDAT_REDBOOT_H8300_LINUX_BOOT_COMMAND_LINE );
    
    bool command_addr_set,command_line_set;

    struct option_info opts[2];
    char *pcmd;
    int oldints;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              &command_addr, &command_addr_set, "command line address");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_STR, 
              &cmd_line, &command_line_set, "kernel command line");
    
    if (!scan_opts(argc, argv, 1, opts, 2, (void *)&entry, 
                   OPTION_ARG_TYPE_NUM, "entry address"))
	    return ;
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }
  
    diag_printf("Now booting linux kernel:\n");
    diag_printf(" Entry Address 0x%08x\n", entry);
    diag_printf(" Cmdline : %s\n", cmd_line);

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();

    pcmd = (char *)command_addr;
    while ((*pcmd++ = *cmd_line++));

    asm ("jmp @%0" : : "r" (entry));
}

RedBoot_cmd("exec", 
            "Execute an image", 
            "[-b <command line addr>] [-c \"kernel command line\"]\n"
	    "        [<entry point>]",
            do_exec
    );
#endif

static void
do_set_mem(int argc, char *argv[])
{
    struct option_info opts[5];
    unsigned long base, data;
    bool base_set, data_set,len_set = 0;
    static char _size = 1;
    bool set_32bit, set_16bit, set_8bit;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              &base, (bool *)&base_set, "base address");
    init_opts(&opts[1], 'd', true, OPTION_ARG_TYPE_NUM, 
              &data, (bool *)&data_set, "write_data");
    init_opts(&opts[2], '4', false, OPTION_ARG_TYPE_FLG,
              &set_32bit, (bool *)0, "dump 32 bit units");
    init_opts(&opts[3], '2', false, OPTION_ARG_TYPE_FLG,
              &set_16bit, (bool *)0, "dump 16 bit units");
    init_opts(&opts[4], '1', false, OPTION_ARG_TYPE_FLG,
              &set_8bit, (bool *)0, "dump 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 5, 0, 0, "")) {
        return;
    }
    if (!base_set) {
      diag_printf("illigal base\n");
      return ;
    }

    if (!data_set) {
      diag_printf("illigal data\n");
      return ;
    }

    if (set_32bit) {
      _size = 4;
    } else if (set_16bit) {
      _size = 2;
    } else if (set_8bit) {
      _size = 1;
    }

    if (!len_set) {
        _size = 4;
    }
    diag_printf("%d %x = %x\n",_size,base,data);
    switch( _size ) {
    case 1:
      *(unsigned char *)base=data;
      break;
    case 2:
      *(unsigned short *)base=data;
      break;
    case 4:
      *(unsigned long *)base=data;
      break;
    }
}

RedBoot_cmd("set",
	    "Set Memory",
	    "-b address -[1|2|4] -d data",
	    do_set_mem
    );


//==========================================================================
//
//      mcmp.c
//
//      RedBoot memory compare (mcmp) routine
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2002-08-06
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

RedBoot_cmd("mcmp", 
            "Compare two blocks of memory",
            "-s <location> -d <location> -l <length> [-1|-2|-4]",
            do_mcmp
    );

void
do_mcmp(int argc, char *argv[])
{
    // Fill a region of memory with a pattern
    struct option_info opts[6];
    unsigned long src_base, dst_base;
    long len;
    bool src_base_set, dst_base_set, len_set;
    bool set_32bit, set_16bit, set_8bit;

    init_opts(&opts[0], 's', true, OPTION_ARG_TYPE_NUM, 
              (void *)&src_base, (bool *)&src_base_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void *)&len, (bool *)&len_set, "length");
    init_opts(&opts[2], 'd', true, OPTION_ARG_TYPE_NUM, 
              (void *)&dst_base, (bool *)&dst_base_set, "base address");
    init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[4], '2', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[5], '1', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_8bit, (bool *)0, "fill 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }
    if (!src_base_set || !dst_base_set || !len_set) {
        diag_printf("usage: mcmp -s <addr> -d <addr> -l <length> [-1|-2|-4]\n");
        return;
    }
    // No checks here    
    if (set_8bit) {
        // Compare 8 bits at a time
        while ((len -= sizeof(cyg_uint8)) >= 0) {
            if (*((cyg_uint8 *)src_base)++ != *((cyg_uint8 *)dst_base)++) {
                ((cyg_uint8 *)src_base)--;
                ((cyg_uint8 *)dst_base)--;
                diag_printf("Buffers don't match - %p=0x%02x, %p=0x%02x\n",
                            src_base, *((cyg_uint8 *)src_base),
                            dst_base, *((cyg_uint8 *)dst_base));
                return;
            }
        }
    } else if (set_16bit) {
        // Compare 16 bits at a time
        while ((len -= sizeof(cyg_uint16)) >= 0) {
            if (*((cyg_uint16 *)src_base)++ != *((cyg_uint16 *)dst_base)++) {
                ((cyg_uint16 *)src_base)--;
                ((cyg_uint16 *)dst_base)--;
                diag_printf("Buffers don't match - %p=0x%04x, %p=0x%04x\n",
                            src_base, *((cyg_uint16 *)src_base),
                            dst_base, *((cyg_uint16 *)dst_base));
                return;
            }
        }
    } else {
        // Default - 32 bits
        while ((len -= sizeof(cyg_uint32)) >= 0) {
            if (*((cyg_uint32 *)src_base)++ != *((cyg_uint32 *)dst_base)++) {
                ((cyg_uint32 *)src_base)--;
                ((cyg_uint32 *)dst_base)--;
                diag_printf("Buffers don't match - %p=0x%08x, %p=0x%08x\n",
                            src_base, *((cyg_uint32 *)src_base),
                            dst_base, *((cyg_uint32 *)dst_base));
                return;
            }
        }
    }
}

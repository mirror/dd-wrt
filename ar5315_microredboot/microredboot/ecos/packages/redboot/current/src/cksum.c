//==========================================================================
//
//      cksum.c
//
//      RedBoot POSIX CRC calculation/command
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
// Date:         2001-01-31
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

// Compute a CRC, using the POSIX 1003 definition

RedBoot_cmd("cksum", 
            "Compute a 32bit checksum [POSIX algorithm] for a range of memory", 
            "-b <location> -l <length>",
            do_cksum
    );

void
do_cksum(int argc, char *argv[])
{
    struct option_info opts[2];
    unsigned long base, len, crc;
    bool base_set, len_set;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&base, (bool *)&base_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void *)&len, (bool *)&len_set, "length");
    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
        return;
    }
    if (!base_set || !len_set) {
	if (load_address >= (CYG_ADDRESS)ram_start &&
	    load_address_end < (CYG_ADDRESS)ram_end &&
	    load_address < load_address_end) {
	    base = load_address;
	    len = load_address_end - load_address;
            diag_printf("Computing cksum for area %p-%p\n",
                        base, load_address_end);
	} else {
	    diag_printf("usage: cksum -b <addr> -l <length>\n");
	    return;
	}
    }
    crc = cyg_posix_crc32((unsigned char *)base, len);
    diag_printf("POSIX cksum = %lu %lu (0x%08lx 0x%08lx)\n", crc, len, crc, len);
}


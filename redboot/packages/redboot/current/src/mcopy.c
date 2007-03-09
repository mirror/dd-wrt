//==========================================================================
//
//      mcopy.c
//
//      RedBoot memory copy (mcopy) routine
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2003-07-01
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

static void
do_mcopy(int argc, char *argv[])
{
    struct option_info opts[6];
    unsigned long src, dst, len, end;
    bool src_set, dst_set, len_set;
    bool sz32, sz16, sz8;
    int incr = 1;

    init_opts(&opts[0], 's', true, OPTION_ARG_TYPE_NUM, 
              (void *)&src, (bool *)&src_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void *)&len, (bool *)&len_set, "length");
    init_opts(&opts[2], 'd', true, OPTION_ARG_TYPE_NUM, 
              (void *)&dst, (bool *)&dst_set, "base address");
    init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&sz32, (bool *)0, "copy 32 bit data");
    init_opts(&opts[4], '2', false, OPTION_ARG_TYPE_FLG,
              (void *)&sz16, (bool *)0, "copy 16 bit data");
    init_opts(&opts[5], '1', false, OPTION_ARG_TYPE_FLG,
              (void *)&sz8, (bool *)0, "copy 8 bit data");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }

    // Must have src, dst, len. No more than one size specifier.
    if (!src_set || !dst_set || !len_set || (sz32 + sz16 + sz8) > 1) {
        diag_printf("usage: mcopy -s <addr> -d <addr> -l <length> [-1|-2|-4]\n");
        return;
    }

    // adjust incr and len for data size
    if (sz16) {
	len = (len + 1) & ~1;
	incr = 2;
    } else if (sz32 || !sz8) {
	len = (len + 3) & ~3;
	incr = 4;
    }

    end = src + len;

    // If overlapping areas, must copy backwards.
    if (dst > src && dst < (src + len)) {
	end = src - incr;
	src += (len - incr);
	dst += (len - incr);
	incr = -incr;
    }

    if (sz8) {
	while (src != end) {
	    *(cyg_uint8 *)dst = *(cyg_uint8 *)src;
	    src += incr;
	    dst += incr;
	}
    } else if (sz16) {
	while (src != end) {
	    *(cyg_uint16 *)dst = *(cyg_uint16 *)src;
	    src += incr;
	    dst += incr;
	}
    } else {
        // Default - 32 bits
	while (src != end) {
	    *(cyg_uint32 *)dst = *(cyg_uint32 *)src;
	    src += incr;
	    dst += incr;
	}
    }
}


RedBoot_cmd("mcopy", 
            "Copy memory from one address to another",
            "-s <location> -d <location> -l <length> [-1|-2|-4]",
            do_mcopy
    );

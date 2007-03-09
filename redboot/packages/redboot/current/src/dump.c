//==========================================================================
//
//      dump.c
//
//      RedBoot dump support
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

RedBoot_cmd("dump", 
            "Display (hex dump) a range of memory", 
            "-b <location> [-l <length>] [-s] [-1|2|4]",
            do_dump 
    );
RedBoot_cmd("x", 
            "Display (hex dump) a range of memory", 
            "-b <location> [-l <length>] [-s] [-1|2|4]",
            do_x
    );

void
do_dump(int argc, char *argv[])
{
    struct option_info opts[6];
    unsigned long base, len;
    bool base_set, len_set;
    static unsigned long _base, _len;
    static char _size = 1;
    bool srec_dump, set_32bit, set_16bit, set_8bit;
    int i, n, off, cksum;
    cyg_uint8 ch;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&base, (bool *)&base_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void *)&len, (bool *)&len_set, "length");
    init_opts(&opts[2], 's', false, OPTION_ARG_TYPE_FLG, 
              (void *)&srec_dump, 0, "dump data using Morotola S-records");
    init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "dump 32 bit units");
    init_opts(&opts[4], '2', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_16bit, (bool *)0, "dump 16 bit units");
    init_opts(&opts[5], '1', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_8bit, (bool *)0, "dump 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }
    if (!base_set) {
        if (_base == 0) {
            diag_printf("Dump what [location]?\n");
            return;
        }
        base = _base;
        if (!len_set) {
            len = _len;
            len_set = true;
        }
    }

    if (set_32bit) {
      _size = 4;
    } else if (set_16bit) {
      _size = 2;
    } else if (set_8bit) {
      _size = 1;
    }

    if (!len_set) {
        len = 32;
    }
    if (srec_dump) {
        off = 0;
        while (off < len) {
            n = (len > 16) ? 16 : len;
            cksum = n+5;
            diag_printf("S3%02X%08X", n+5, off+base);
            for (i = 0;  i < 4;  i++) {
                cksum += (((base+off)>>(i*8)) & 0xFF);
            }
            for (i = 0;  i < n;  i++) {
                ch = *(cyg_uint8 *)(base+off+i);
                diag_printf("%02X", ch);
                cksum += ch;
            }
            diag_printf("%02X\n", ~cksum & 0xFF);
            off += n;
        }
    } else {
        switch( _size ) {
        case 1:
            diag_dump_buf((void *)base, len);
            break;
        case 2:
            diag_dump_buf_16bit((void *)base, len);
            break;
        case 4:
            diag_dump_buf_32bit((void *)base, len);
            break;
        }
    }
    _base = base + len;
    _len = len;
}

// Simple alias for the dump command
void
do_x(int argc, char *argv[])
{
    do_dump(argc, argv);
}

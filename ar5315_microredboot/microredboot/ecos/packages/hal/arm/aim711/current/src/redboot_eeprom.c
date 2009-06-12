//==========================================================================
//
//      redboot_eeprom.c
//
//      RedBoot command to read and write eeprom content of the
//      ARM Industrial Module AIM 711
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    rcassebohm
// Contributors: rcassebohm
// Date:         2003-11-10
// Purpose:      
// Description:  
//              
// RedBoot command to read and write eeprom content of the
// ARM Industrial Module AIM 711
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <redboot.h>

#include <cyg/hal/hal_io.h>

// Exported CLI function(s)
static void do_eeprom_read(int argc, char *argv[]);
RedBoot_cmd("eeprom_read", 
            "Read eeprom content", 
            "-b <location> -o <eeprom offset> -l <length> [-d dump]",
            do_eeprom_read
    );

static void do_eeprom_write(int argc, char *argv[]);
RedBoot_cmd("eeprom_write", 
            "Write eeprom content", 
            "[-b <location>] -o <eeprom offset> [-l <length>]",
            do_eeprom_write
    );

static void 
do_eeprom_read(int argc, char *argv[])
{
    bool base_addr_set, offset_set, length_set, dump;
    unsigned long base_addr, offset, length;
    struct option_info opts[4];
    cyg_uint8 *buf;
    int ret;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, (bool *)&base_addr_set, "location");
    init_opts(&opts[1], 'o', true, OPTION_ARG_TYPE_NUM, 
              (void **)&offset, (bool *)&offset_set, "eeprom offset");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[3], 'd', true, OPTION_ARG_TYPE_FLG, 
              (void **)&dump, 0, "dump data");
    if (!scan_opts(argc, argv, 1, opts, 4, 0, 0, "")) {
        return;
    }

    if (!base_addr_set || !offset_set || !length_set)
    {
        diag_printf("usage: eeprom_read -b <location> -o <eeprom offset>"
                " -l <length> [-d dump]\n");
        return;
    }

    buf = (cyg_uint8 *)base_addr;
    ret = hal_aim711_eeprom_read(buf, offset, length);
    if (ret < 0)
    {
        diag_printf("Error while trying to read eeprom content\n");
        return;
    }
 
    diag_printf("Read %d bytes of eeprom content\n", ret);

    if (dump)
        diag_dump_buf((void *)buf, ret);
}

static void 
do_eeprom_write(int argc, char *argv[])
{
    bool base_addr_set, offset_set, length_set;
    unsigned long base_addr, offset, length;
    struct option_info opts[3];
    cyg_uint8 *buf;
    int ret;

    base_addr = load_address;
    length = load_address_end - load_address;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, (bool *)&base_addr_set, "location");
    init_opts(&opts[1], 'o', true, OPTION_ARG_TYPE_NUM, 
              (void **)&offset, (bool *)&offset_set, "eeprom offset");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    if (!scan_opts(argc, argv, 1, opts, 3, 0, 0, "")) {
        return;
    }

    if (!offset_set)
    {
        diag_printf("usage: eeprom_write [-b <location>] -o <eeprom offset>"
                " [-l <length>]\n");
        return;
    }

    buf = (cyg_uint8 *)base_addr;
    ret = hal_aim711_eeprom_write(buf, offset, length);
    if (ret < 0)
    {
        diag_printf("Error while trying to write eeprom content\n");
        return;
    }
 
    diag_printf("Written %d bytes of eeprom content\n", ret);
}
      
// EOF redboot_eeprom.c

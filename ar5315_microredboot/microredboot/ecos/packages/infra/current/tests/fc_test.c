//==========================================================================
//
//      fc_test.c
//
//      Test/demonstration of using RedBoot 'fconfig' from eCos
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003, 2004 Gary Thomas
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
// Date:         2003-12-22
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

//
// Demonstration of how to use virtual vector interfaces to access/modify
// persistent data stored by 'fconfig' command in RedBoot.
//
// Note: there is currently no support for adding new keys using this
// mechanism.  Only existing key/value pairs may be updated.
//
#include <pkgconf/hal.h>
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>

void
main(void)
{
    struct cyg_fconfig fc;
    char key[64];
    int port;

    diag_printf("fconfig test started\n");
    fc.offset = 0;
    fc.key = key;
    fc.keylen = sizeof(key);
    while (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_NEXT, &fc)) {
        diag_printf("  Offset: %d, key: '%s', type: %d\n", fc.offset, fc.key, fc.type);
        fc.keylen = sizeof(key);
    }
    // Try and update a data value
    fc.key = "gdb_port";
    fc.val = &port;
    fc.type = CYGNUM_FLASH_CFG_TYPE_CONFIG_INT;
    if (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_GET, &fc)) {
        diag_printf("gdb_port = %d\n", port);
        port++;
        if (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_SET, &fc)) {
            if (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_GET, &fc)) {
                diag_printf("now = %d\n", port);
            } else {
                diag_printf("Can't re-fetch 'gdb_port'\n");
                exit(1);
            }
            port--;
            if (!CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_SET, &fc)) {
                diag_printf("Can't update 'gdb_port'\n");
                exit(1);
            }
        } else {
            diag_printf("Can't update 'gdb_port'\n");
            exit(1);
        }
    } else {
        diag_printf("Fetch 'gdb_port' failed\n");
        exit(1);
    }
    diag_printf("... done\n");
    exit(1);
}

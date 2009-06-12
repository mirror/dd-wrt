//==========================================================================
//
//        redboot_module.c
//
//        ARM E7T board RedBoot module wrapper
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):     gthomas
// Contributors:  gthomas, jskov
// Date:          2001-03-19
// Description:   AEB-2 FLASH module for RedBoot
//####DESCRIPTIONEND####

//
// This is the module 'wrapper' for RedBoot
//

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_stub.h>

// ARM AEB-2 module stuff

#ifdef CYGPKG_REDBOOT

#include <redboot.h>

#ifndef CHECKSUM
#define CHECKSUM 0x0
#endif

extern char __exception_handlers, __rom_data_end;

const char __title[] = "RedBoot";
const char __help[] = "RedBoot              " __DATE__;

struct ModuleHeader {
    cyg_uint32    magic;
    cyg_uint16    flags;
    cyg_uint8     major;
    cyg_uint8     minor;
    cyg_uint32    checksum;
    cyg_uint32    ro_base;
    cyg_uint32    ro_limit;
    cyg_uint32    rw_base;
    cyg_uint32    zi_base; 
    cyg_uint32    zi_limit;
    cyg_uint32    self;
    cyg_uint32    start;
    cyg_uint32    init;
    cyg_uint32    final;
    cyg_uint32    service;
    cyg_uint32    title;
    cyg_uint32    help;
    cyg_uint32    cmdtbl;
    cyg_uint32    swi_base;
    cyg_uint32    swi_handler;
};

const static struct ModuleHeader __hdr = {
    0x4D484944,                     // MHID
    2,                              // flags = auto start
    1,                              // major
    0,                              // minor
    CHECKSUM,                       // checksum
    (cyg_uint32) &__exception_handlers,         // start of module (read-only) image
    (cyg_uint32) &__rom_data_end,    // end of image
    0,                              // r/w base - unused
    0,                              // bss base - unused
    0,                              // bss limit - unused
    (cyg_uint32) &__hdr,            // self (for module identification)
    (cyg_uint32) &__exception_handlers,         // startup 
    0,                              // init - unused
    0,                              // final - unused
    0,                              // service - unused
    (cyg_uint32) &__title,          // title
    (cyg_uint32) &__help,           // help string
    0,                              // command table - unused
    0,                              // SWI table - unused
    0                               // SWI handler - unused
};

static void
__dummy(void *p)
{
}

void __dummy_init(void)
{
    __dummy((void*)&__hdr);
}

_RedBoot_init(__dummy_init, RedBoot_INIT_LAST);


#else
#error "Stand-alone RedBoot only"
#endif

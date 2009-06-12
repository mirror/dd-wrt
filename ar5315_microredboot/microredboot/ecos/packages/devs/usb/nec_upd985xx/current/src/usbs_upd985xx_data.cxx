//==========================================================================
//
//      usbs_nec_upd9850x.c
//
//      Static data for the NEC uPD9850x USB device driver
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
// Author(s):    bartv
// Contributors: bartv
// Date:         2001-05-22
//
// This file contains various objects that should go into extras.o
// rather than libtarget.a, e.g. devtab entries that would normally
// be eliminated by the selective linking.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/diag.h>
#include <cyg/io/devtab.h>
#include <cyg/io/usb/usbs_upd985xx.h>
#include <pkgconf/devs_usb_upd985xx.h>

// ----------------------------------------------------------------------------
// Initialization. The goal here is to call usbs_upd985xx_init()
// early on during system startup, to take care of things like
// registering interrupt handlers etc. which are best done
// during system init.
//
// If the endpoint 0 devtab entry is available then its init()
// function can be used to take care of this. However the devtab
// entries are optional so an alternative mechanism must be
// provided. Unfortunately although it is possible to give
// a C function the constructor attribute, it cannot be given
// an initpri attribute. Instead it is necessary to define a
// dummy C++ class.

extern "C" void usbs_upd985xx_init(void);

#ifndef CYGVAR_DEVS_USB_UPD985XX_EP0_DEVTAB_ENTRY
class usbs_upd985xx_initialization {
  public:
    usbs_upd985xx_initialization() {
        usbs_upd985xx_init();
    }
};

static usbs_upd985xx_initialization usbs_upd985xx_init_object CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO);
#endif

// ----------------------------------------------------------------------------
// The devtab entries. Each of these is optional, many applications
// will want to use the lower-level API rather than go via
// open/read/write/ioctl.

#ifdef CYGVAR_DEVS_USB_UPD985XX_EP0_DEVTAB_ENTRY

// For endpoint 0 the only legal operations are get_config() and
// set_config(), and these are provided by the common package.

static bool
usbs_upd985xx_devtab_ep0_init(struct cyg_devtab_entry* tab)
{
    CYG_UNUSED_PARAM(struct cyg_devtab_entry*, tab);
    usbs_upd985xx_init();
    return true;
}

static CHAR_DEVIO_TABLE(usbs_upd985xx_ep0_devtab_functions,
                        &cyg_devio_cwrite,
                        &cyg_devio_cread,
                        &cyg_devio_select,
                        &usbs_devtab_get_config,
                        &usbs_devtab_set_config);

static CHAR_DEVTAB_ENTRY(usbs_upd985xx_ep0_devtab_entry,
                         CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "0c",
                         0,
                         &usbs_upd985xx_ep0_devtab_functions,
                         &usbs_upd985xx_devtab_ep0_init,
                         0,
                         (void*) &usbs_upd985xx_ep0);
#endif

// ----------------------------------------------------------------------------
// Common routines for ep3, ep4 and ep5
#if defined(CYGVAR_DEVS_USB_UPD985XX_EP3_DEVTAB_ENTRY) || \
    defined(CYGVAR_DEVS_USB_UPD985XX_EP4_DEVTAB_ENTRY) || \
    defined(CYGVAR_DEVS_USB_UPD985XX_EP5_DEVTAB_ENTRY)
static bool
usbs_upd985xx_devtab_dummy_init(struct cyg_devtab_entry* tab)
{
    CYG_UNUSED_PARAM(struct cyg_devtab_entry*, tab);
    return true;
}
#endif

// ----------------------------------------------------------------------------
// ep3 devtab entry. This can only be used for slave->host, so only
// the cwrite() function makes sense. The same function table can be
// used for ep5.

#if defined(CYGVAR_DEVS_USB_UPD985XX_EP3_DEVTAB_ENTRY) || \
    defined(CYGVAR_DEVS_USB_UPD985XX_EP5_DEVTAB_ENTRY)
static CHAR_DEVIO_TABLE(usbs_upd985xx_ep35_devtab_functions,
                        &usbs_devtab_cwrite,
                        &cyg_devio_cread,
                        &cyg_devio_select,
                        &usbs_devtab_get_config,
                        &usbs_devtab_set_config);

# if defined(CYGVAR_DEVS_USB_UPD985XX_EP3_DEVTAB_ENTRY)
static CHAR_DEVTAB_ENTRY(usbs_upd985xx_ep3_devtab_entry,
                         CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "3w",
                         0,
                         &usbs_upd985xx_ep35_devtab_functions,
                         &usbs_upd985xx_devtab_dummy_init,
                         0,
                         (void*) &usbs_upd985xx_ep3);

# endif

# if defined(CYGVAR_DEVS_USB_UPD985XX_EP5_DEVTAB_ENTRY)
static CHAR_DEVTAB_ENTRY(usbs_upd985xx_ep5_devtab_entry,
                         CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "5w",
                         0,
                         &usbs_upd985xx_ep35_devtab_functions,
                         &usbs_upd985xx_devtab_dummy_init,
                         0,
                         (void*) &usbs_upd985xx_ep5);
# endif
#endif

// ----------------------------------------------------------------------------
// ep4 devtab entry. This can only be used for host->slave, so only the
// cread() function makes sense.

#ifdef CYGVAR_DEVS_USB_UPD985XX_EP4_DEVTAB_ENTRY

static CHAR_DEVIO_TABLE(usbs_upd985xx_ep4_devtab_functions,
                        &cyg_devio_cwrite,
                        &usbs_devtab_cread,
                        &cyg_devio_select,
                        &usbs_devtab_get_config,
                        &usbs_devtab_set_config);

static CHAR_DEVTAB_ENTRY(usbs_upd985xx_ep4_devtab_entry,
                         CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "4r",
                         0,
                         &usbs_upd985xx_ep4_devtab_functions,
                         &usbs_upd985xx_devtab_dummy_init,
                         0,
                         (void*) &usbs_upd985xx_ep4);
#endif


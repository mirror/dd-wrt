#ifndef CYGONCE_USBS_UPD985XX_H
# define CYGONCE_USBS_UPD985XX_H
//==========================================================================
//
//      include/usbs_upd985xx.h
//
//      The interface exported by the NEC uPD985xx USB device driver
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
// Date:         2000-05-22
// Purpose:
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/io/usb/usbs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The NEC UPD985xx family comes with on-chip USB slave support. This
 * provides seven endpoints. Endpoint 0 can only be used for control
 * messages. Endpoints 1 and 2 can only be used for isochronous
 * transfers, and are not supported at this time. Endpoints 3 and 4
 * are for bulk transfers, although endpoint 3 is normally disabled
 * and endpoint 5 is used for bulk transfers instead. Endpoints 5
 * and 6 are normally used for interrupt transfers, but endpoint 5 can
 * also be used bulk transfers. Endpoint 6 is not currently supported.
 */
extern usbs_control_endpoint    usbs_upd985xx_ep0;
extern usbs_tx_endpoint         usbs_upd985xx_ep3;
extern usbs_rx_endpoint         usbs_upd985xx_ep4;
extern usbs_tx_endpoint         usbs_upd985xx_ep5;

#ifdef __cplusplus
} /* extern "C" { */
#endif


#endif /* CYGONCE_USBS_UPD985XX_H */

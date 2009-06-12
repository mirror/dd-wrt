#ifndef CYGONCE_USBS_SA11X0_H
# define CYGONCE_USBS_SA11X0_H
//==========================================================================
//
//      include/usbs_sa11x0.h
//
//      The interface exported by the SA11X0 USB device driver
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
// Date:         2000-10-04
// Purpose:
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/io/usb/usbs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The SA11x0 family comes with on-chip USB slave support. This
 * provides three endpoints. Endpoint 0 can only be used for control
 * messages. Endpoints 1 and 2 can only be used for bulk transfers,
 * host->slave for endpoint 1 and slave->host for endpoint 2.
 */
extern usbs_control_endpoint    usbs_sa11x0_ep0;
extern usbs_rx_endpoint         usbs_sa11x0_ep1;
extern usbs_tx_endpoint         usbs_sa11x0_ep2;
    
#ifdef __cplusplus
} /* extern "C" { */
#endif


#endif /* CYGONCE_USBS_SA11X0_H */

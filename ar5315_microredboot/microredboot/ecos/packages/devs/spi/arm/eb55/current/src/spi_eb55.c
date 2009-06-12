//==========================================================================
//
//      spi_eb55.c
//
//      Atmel AT91EB55 SPI devices 
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
// Author(s):     Savin Zlobec <savin@elatec.si> 
// Date:          2004-08-27
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/io/spi.h>
#include <cyg/io/spi_at91.h>

// -------------------------------------------------------------------------
// AT91EB55 SPI exported devices 

// AT45DB321B DataFlash
static cyg_spi_at91_device_t spi_dataflash_dev0 CYG_SPI_DEVICE_ON_BUS(0) = 
{
    .spi_device.spi_bus = &cyg_spi_at91_bus.spi_bus,

    .dev_num     = 0,       // Device number
    .cl_pol      = 1,       // Clock polarity (0 or 1)
    .cl_pha      = 0,       // Clock phase (0 or 1)
    .cl_brate    = 8192000, // Clock baud rate
    .cs_up_udly  = 1,       // Delay in usec between CS up and transfer start
    .cs_dw_udly  = 1,       // Delay in usec between transfer end and CS down
    .tr_bt_udly  = 1        // Delay in usec between two transfers
};

cyg_spi_device *cyg_spi_dataflash_dev0 = &spi_dataflash_dev0.spi_device;

// -------------------------------------------------------------------------
// EOF spi_eb55.c

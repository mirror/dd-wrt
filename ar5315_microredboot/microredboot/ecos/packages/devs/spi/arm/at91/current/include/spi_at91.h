#ifndef CYGONCE_DEVS_SPI_ARM_AT91_H
#define CYGONCE_DEVS_SPI_ARM_AT91_H
//==========================================================================
//
//      spi_at91.h
//
//      Atmel AT91 (ARM) SPI driver defines
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
// Date:          2004-08-25
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/spi.h>

//-----------------------------------------------------------------------------
// AT91 SPI BUS

typedef struct cyg_spi_at91_bus_s
{
    // ---- Upper layer data ----

    cyg_spi_bus   spi_bus;                  // Upper layer SPI bus data

    // ---- Lower layer data ----
     
    cyg_interrupt     spi_interrupt;        // SPI interrupt object
    cyg_handle_t      spi_interrupt_handle; // SPI interrupt handle
    cyg_drv_mutex_t   transfer_mx;          // Transfer mutex
    cyg_drv_cond_t    transfer_cond;        // Transfer condition
    cyg_bool          transfer_end;         // Transfer end flag
    cyg_bool          cs_up;                // Chip Select up flag       
} cyg_spi_at91_bus_t;

//-----------------------------------------------------------------------------
// AT91 SPI DEVICE

typedef struct cyg_spi_at91_device_s
{
    // ---- Upper layer data ----

    cyg_spi_device spi_device;  // Upper layer SPI device data

    // ---- Lower layer data (configurable) ----

    cyg_uint8  dev_num;         // Device number
    cyg_uint8  cl_pol;          // Clock polarity (0 or 1)
    cyg_uint8  cl_pha;          // Clock phase    (0 or 1)
    cyg_uint32 cl_brate;        // Clock baud rate
    cyg_uint16 cs_up_udly;      // Delay in us between CS up and transfer start
    cyg_uint16 cs_dw_udly;      // Delay in us between transfer end and CS down
    cyg_uint16 tr_bt_udly;      // Delay in us between two transfers

    // ---- Lower layer data (internal) ----

    cyg_bool   init;            // Is device initialized
    cyg_uint8  cl_scbr;         // Value of SCBR (SPI clock) reg field
    cyg_uint8  cl_div32;        // Divide SPI master clock by 32
} cyg_spi_at91_device_t;

//-----------------------------------------------------------------------------
// AT91 SPI exported bus

externC cyg_spi_at91_bus_t cyg_spi_at91_bus;

//-----------------------------------------------------------------------------

#endif // CYGONCE_DEVS_SPI_ARM_AT91_H 

//-----------------------------------------------------------------------------
// End of spi_at91.h

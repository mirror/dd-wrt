#ifndef CYGONCE_IO_SPI_H
#define CYGONCE_IO_SPI_H

//=============================================================================
//
//      spi.h
//
//      Generic API for accessing devices on an SPI bus
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric Limited
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//####DESCRIPTIONBEGIN####
//
// Author(s): 	bartv
// Date:	    2004-04-23
//####DESCRIPTIONEND####
//=============================================================================

#include <pkgconf/infra.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/hal/hal_io.h>

typedef struct cyg_spi_device {
    struct cyg_spi_bus*     spi_bus;
} cyg_spi_device;

typedef struct cyg_spi_bus {
    cyg_drv_mutex_t         spi_lock;
#ifdef CYGDBG_USE_ASSERTS    
    cyg_spi_device*         spi_current_device;
#endif
    void                    (*spi_transaction_begin)(cyg_spi_device*);
    void                    (*spi_transaction_transfer)(cyg_spi_device*, cyg_bool, cyg_uint32, const cyg_uint8*, cyg_uint8*, cyg_bool);
    void                    (*spi_transaction_tick)(cyg_spi_device*, cyg_bool, cyg_uint32);
    void                    (*spi_transaction_end)(cyg_spi_device*);
    int                     (*spi_get_config)(cyg_spi_device*, cyg_uint32, void*, cyg_uint32*);
    int                     (*spi_set_config)(cyg_spi_device*, cyg_uint32, const void*, cyg_uint32*);
} cyg_spi_bus;

#ifdef CYGDBG_USE_ASSERTS
# define CYG_SPI_BUS_COMMON_INIT(_bus_)                                 \
    CYG_MACRO_START                                                     \
    cyg_drv_mutex_init( & ((_bus_)->spi_lock));                         \
    (_bus_)->spi_current_device = (cyg_spi_device*)0;                   \
    CYG_MACRO_END
#else
# define CYG_SPI_BUS_COMMON_INIT(_bus_)                                 \
    CYG_MACRO_START                                                     \
    cyg_drv_mutex_init( & ((_bus_)->spi_lock));                         \
    CYG_MACRO_END
#endif

// All devices should be in a per-bus table
#define CYG_SPI_DEFINE_BUS_TABLE(_type_, _which_)                                       \
    CYG_HAL_TABLE_BEGIN(cyg_spi_bus_ ## _which_ ## _devs, spibus_ ## _which_);          \
    CYG_HAL_TABLE_END(cyg_spi_bus_ ## _which_ ## _devs_end, spibus_ ## _which_);        \
    extern _type_ cyg_spi_bus_## _which_ ## _devs[], cyg_spi_bus_## _which_ ## _devs_end

#define CYG_SPI_DEVICE_ON_BUS(_which_)  CYG_HAL_TABLE_ENTRY( spibus_ ## _which_)

// Keys for use with the get_config() and set_config() operations.
#define CYG_IO_GET_CONFIG_SPI_CLOCKRATE     0x00000800
#define CYG_IO_SET_CONFIG_SPI_CLOCKRATE     0x00000880

// The simple I/O operations.
externC void        cyg_spi_transfer(cyg_spi_device*, cyg_bool, cyg_uint32, const cyg_uint8*, cyg_uint8*);
externC void        cyg_spi_tick(cyg_spi_device*, cyg_bool, cyg_uint32);
externC int         cyg_spi_get_config(cyg_spi_device*, cyg_uint32, void*, cyg_uint32*);
externC int         cyg_spi_set_config(cyg_spi_device*, cyg_uint32, const void*, cyg_uint32*);

// Support for more complicated transactions.
externC void        cyg_spi_transaction_begin(cyg_spi_device*);
externC cyg_bool    cyg_spi_transaction_begin_nb(cyg_spi_device*);
externC void        cyg_spi_transaction_transfer(cyg_spi_device*, cyg_bool, cyg_uint32, const cyg_uint8*, cyg_uint8*, cyg_bool);
externC void        cyg_spi_transaction_tick(cyg_spi_device*, cyg_bool, cyg_uint32);
externC void        cyg_spi_transaction_end(cyg_spi_device*);

// Allow the HAL to export named devices, without introducing circular
// dependencies in the header files.
#ifdef HAL_SPI_EXPORTED_DEVICES
  HAL_SPI_EXPORTED_DEVICES
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_IO_SPI_H
// End of spi.h

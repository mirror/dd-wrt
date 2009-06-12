//==========================================================================
//
//      spi.c
//
//      Generic API for accessing devices attached to an SPI bus
//
//==========================================================================
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
//==========================================================================
//###DESCRIPTIONBEGIN####
//
// Author(s):     bartv
// Date:          2004-04-23
//
//###DESCRIPTIONEND####
//========================================================================

#include <pkgconf/infra.h>
#include <pkgconf/io_spi.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/io/spi.h>

// The simple transfer function just calls the transaction functions
// in order. The chip select should be dropped at the end of this
// transfer.
void
cyg_spi_transfer(cyg_spi_device* device, cyg_bool polled, cyg_uint32 count, const cyg_uint8* tx_data, cyg_uint8* rx_data)
{
    cyg_spi_transaction_begin(device);
    cyg_spi_transaction_transfer(device, polled, count, tx_data, rx_data, 1);
    cyg_spi_transaction_end(device);
}

// Tick is similarly simple.
void
cyg_spi_tick(cyg_spi_device* device, cyg_bool polled, cyg_uint32 count)
{
    cyg_spi_transaction_begin(device);
    cyg_spi_transaction_tick(device, polled, count);
    cyg_spi_transaction_end(device);
}

// get_config() and set_config() requires locking the bus, then accessing the
// bus-specific function via the bus structure. Only locking is needed, not
// setting up the bus for a transfer to this device, so the transaction begin
// function is inappropriate.
int
cyg_spi_get_config(cyg_spi_device* device, cyg_uint32 key, void* buf, cyg_uint32* len)
{
    int             result;
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_CHECK_FUNC_PTR(bus->spi_get_config, "SPI bus driver has not provided a get_config function");
    
    while (!cyg_drv_mutex_lock(&(bus->spi_lock)));
    result = (*(bus->spi_get_config))(device, key, buf, len);
    cyg_drv_mutex_unlock(&(bus->spi_lock));

    return result;
}

// set_config is basically just a clone of the above, but using a different
// bus-specific operation.
int
cyg_spi_set_config(cyg_spi_device* device, cyg_uint32 key, const void* buf, cyg_uint32* len)
{
    int             result;
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_CHECK_FUNC_PTR(bus->spi_set_config, "SPI bus driver has not provided a set_config function");
    
    while (!cyg_drv_mutex_lock(&(bus->spi_lock)));
    result = (*(bus->spi_set_config))(device, key, buf, len);
    cyg_drv_mutex_unlock(&(bus->spi_lock));

    return result;
}

// transaction-begin involves getting sole access to the bus, then calling
// the bus driver's begin function to set up the hardware appropriately for
// the target device.
void
cyg_spi_transaction_begin(cyg_spi_device* device)
{
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_CHECK_FUNC_PTR(bus->spi_transaction_begin, "SPI device has not provided a transaction_begin function");

    while (!cyg_drv_mutex_lock(&(bus->spi_lock)));
#ifdef CYGDBG_USE_ASSERTS    
    bus->spi_current_device = device;
#endif
    
    // This thread now has exclusive access to the bus
    (*(bus->spi_transaction_begin))(device);

    // All done. Return with the bus still locked.
}

// A variant of the above which returns immediately if some other thread
// is using the bus.
cyg_bool
cyg_spi_transaction_begin_nb(cyg_spi_device* device)
{
    cyg_bool        result  = false;
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_CHECK_FUNC_PTR(bus->spi_transaction_begin, "SPI device has not provided a transaction_begin function");

    if (cyg_drv_mutex_trylock(&(bus->spi_lock))) {
#ifdef CYGDBG_USE_ASSERTS        
        bus->spi_current_device = device;
#endif        
        (*(bus->spi_transaction_begin))(device);
        result = true;
    }
    
    return result;
}

void
cyg_spi_transaction_transfer(cyg_spi_device* device, cyg_bool polled,
                             cyg_uint32 count, const cyg_uint8* tx_data, cyg_uint8* rx_data,
                             cyg_bool drop_cs)
{
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_ASSERT(bus->spi_current_device == device, "SPI transfer requested without claiming the bus");
    CYG_CHECK_FUNC_PTR(bus->spi_transaction_transfer, "SPI device has not provided a transfer function");
    (*(bus->spi_transaction_transfer))(device, polled, count, tx_data, rx_data, drop_cs);
}

void
cyg_spi_transaction_tick(cyg_spi_device* device, cyg_bool polled, cyg_uint32 count)
{
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_ASSERT(bus->spi_current_device == device, "SPI ticks requested without claiming the bus");
    CYG_CHECK_FUNC_PTR(bus->spi_transaction_tick, "SPI device has not provided a tick function");

    (*(bus->spi_transaction_tick))(device, polled, count);
}

void
cyg_spi_transaction_end(cyg_spi_device* device)
{
    cyg_spi_bus*    bus;
    CYG_CHECK_DATA_PTR(device, "valid SPI device pointer required");

    bus = device->spi_bus;
    CYG_CHECK_DATA_PTR(bus, "SPI device does not point at a valid bus structure");
    CYG_ASSERT(bus->spi_current_device == device, "SPI transfer requested without claiming the bus");
    CYG_CHECK_FUNC_PTR(bus->spi_transaction_end, "SPI device has not provided an end function");

    // First, call the bus' end function.
    (*(bus->spi_transaction_end))(device);

    // Then release the SPI bus for other threads.
    cyg_drv_mutex_unlock(&(bus->spi_lock));
}

//=============================================================================
//
//      plf_stub.c
//
//      Platform specific code for GDB stub support.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, jskov (based on the old tx39 hal_stub.c)
// Contributors:tmichals
// Date:        1999-02-12
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================
#include <pkgconf/hal.h>
#include <pkgconf/system.h>

#include CYGBLD_HAL_PLATFORM_H
#include <cyg/hal/drv_api.h>          // Cache handling
#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling

#include <cyg/hal/hal_io.h>             // HAL IO macros
#include <cyg/hal/hal_diag.h>           // diag output. FIXME
#include <cyg/hal/idt32334sio.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_intr.h>



#define PIO_DIRCNTL_REG         0xb8000604

void hal_IDT32334_init_serial_baud( int baud )
{
	cyg_uint16 baud_divisor;
	cyg_uint8 _lcr, _ier;
	cyg_addrword_t port = CMA_SER_16550_BASE_A;
	cyg_uint32  status;


	HAL_READ_UINT32 ( PIO_DIRCNTL_REG+4, status);
	status |= 0xf0;
	HAL_WRITE_UINT32 ( PIO_DIRCNTL_REG+4, status);


	HAL_READ_UINT32 (PIO_DIRCNTL_REG, status);
	status &= 0xffffff0f;
	status |= 0x50;
	HAL_WRITE_UINT32(PIO_DIRCNTL_REG, status);

	baud_divisor = (CYGHWR_HAL_MIPS_CPU_FREQ_ACTUAL) / (16 * baud);


	// Set databits, stopbits and parity.
	_lcr = LCR_WL8 | LCR_SB1 | LCR_PN;

	HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

	// Set baud rate.
	_lcr |= LCR_DL;
	HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);
	HAL_WRITE_UINT8(port+SER_16550_DLM, baud_divisor >> 8);
	HAL_WRITE_UINT8(port+SER_16550_DLL, baud_divisor & 0xff);
	_lcr &= ~LCR_DL;
	HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

	HAL_WRITE_UINT8(port+SER_16550_FCR, 0x1);

	HAL_WRITE_UINT8(port+SER_16550_MSR, 0x30);



}


#ifndef CYGNUM_IO_SERIAL_IDT32334_SERIAL0_BAUD
# define	CYGNUM_IO_SERIAL_IDT32334_SERIAL0_BAUD 115200
#endif

// Initialize the current serial port.
void hal_IDT32334_init_serial( void )
{
	 hal_IDT32334_init_serial_baud(  CYGNUM_IO_SERIAL_IDT32334_SERIAL0_BAUD);
}

// Write C to the current serial port.
void hal_IDT32334_put_char( int c )
{
	cyg_uint8 _lsr;
	cyg_addrword_t port =CMA_SER_16550_BASE_A;

	do
	{
		HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
	}while ( !(_lsr & SIO_LSR_THRE)  && (_lsr));

	if ((_lsr & SIO_LSR_THRE) || (!_lsr))  
		HAL_WRITE_UINT8(port+SER_16550_THR, c);
}

// Read one character from the current serial port.
int hal_IDT32334_get_char( void )
{
	unsigned char c;
	cyg_uint8 _lsr;
	cyg_addrword_t port = CMA_SER_16550_BASE_A;

	do {
		HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
	} while ((_lsr & SIO_LSR_DR) == 0);

	HAL_READ_UINT8(port+SER_16550_RBR, c);
	return c;

}


int hal_IDT32334_chk_char()
{
	unsigned char c;
	cyg_uint8 _lsr;
	cyg_addrword_t port = CMA_SER_16550_BASE_A;

	HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
	return (_lsr & SIO_LSR_DR)?1:0;

}
//#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
//-----------------------------------------------------------------------------
// End of plf_stub.c

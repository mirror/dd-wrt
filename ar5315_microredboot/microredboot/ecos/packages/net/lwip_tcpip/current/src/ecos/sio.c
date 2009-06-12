//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric 
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

/* Serial operations for SLIP */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/netif.h"

#include <cyg/io/io.h>
#include <cyg/io/config_keys.h>
#include <cyg/infra/diag.h>

static cyg_io_handle_t ser;

static int len;

void
sio_send(char c,void * dev)
{
	len = 1;
	cyg_io_write(*(cyg_io_handle_t*)dev, &c, &len);
}

char
sio_recv(void * dev)
{
	char c;
	len = 1;
	cyg_io_read(*(cyg_io_handle_t *)dev, &c, &len);
	return c;			
}

int
sio_write(void *dev, char *b, int size)
{
	int len = size;
	cyg_io_write(*(cyg_io_handle_t*)dev, b, &len);
	return len;
}
		
int
sio_read(void *dev, char *b, int size)
{
	int len = size;
	cyg_io_read(*(cyg_io_handle_t*)dev, b, &len);
	
	return len;
}

void * 
sio_open(int devnum)
{
	int res;
	cyg_uint32 nb = 0, len = 4;
	
#if LWIP_SLIP
	#define SIODEV SLIP_DEV
#elif PPP_SUPPORT
	#define SIODEV PPP_DEV
#endif
	res = cyg_io_lookup(SIODEV, &ser);
	if (res != ENOERR)
		diag_printf("Cannot open %s\n", SIODEV);

	res = cyg_io_set_config(ser, CYG_IO_SET_CONFIG_READ_BLOCKING, &nb, &len);
	res = cyg_io_set_config(ser, CYG_IO_SET_CONFIG_WRITE_BLOCKING, &nb, &len);
	len = 4;
	nb = 0xFFFFFFFF;
	res = cyg_io_set_config(ser, CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_METHOD, &nb, &len);
	if (res != ENOERR) 
			diag_printf("set_config flow_control returned an error\n");
	return &ser; 
}

void 
sio_read_abort(void * dev)
{
   diag_printf("Abort called\n");
}

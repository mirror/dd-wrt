#ifndef CYGONCE_SNTP_SNTP_H
#define CYGONCE_SNTP_SNTP_H
//=============================================================================
//
//      sntp.h
//
//      SNTP - Simple Network Time Protocol
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Andrew Lunn
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
// Author(s):   andrew.lunn
// Contributors:
// Date:        2003-02-11
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/net_sntp.h>
#include <cyg/infra/cyg_type.h>

// Multicast address used by IPv6
#define IN6ADDR_NTP_MULTICAST \
	{{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01 }}}

/* Call this function to start the SNTP Client */
__externC void 
cyg_sntp_start(void);

#ifdef CYGPKG_NET_SNTP_UNICAST
__externC void cyg_sntp_set_servers(struct sockaddr *server_list, cyg_uint32 num_servers);
#endif

#endif

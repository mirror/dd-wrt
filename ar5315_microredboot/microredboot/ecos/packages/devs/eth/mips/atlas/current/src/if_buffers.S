// #========================================================================
// #
// #    if_buffers.S
// #
// #    Declare a chunk of 4Kbyte aligned memory for use by bus-mastering
// #    PCI ethernet device.
// #
// #========================================================================
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
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     msalter
// # Contributors:  msalter
// # Date:          2000-11-03
// # Purpose:       
// # Description:   This file defines a chunk of 2Kbyte aligned memory for use
// #                by bus-mastering PCI ethernet device.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

#include "saa9730.h"

	.bss
	.p2align(11)
	.globl cyg_io_atlas_2kbuffers
cyg_io_atlas_2kbuffers:
	.rept (SAA9730_RXPKTS_PER_BUFFER + SAA9730_TXPKTS_PER_BUFFER) \
		  * SAA9730_BUFFERS * SAA9730_PACKET_SIZE
	.byte 0
	.endr

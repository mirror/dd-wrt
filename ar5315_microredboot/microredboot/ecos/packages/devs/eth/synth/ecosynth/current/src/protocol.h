#ifndef CYGONCE_DEVS_ETH_ECOSYNTH_PROTOCOL_H
#define CYGONCE_DEVS_ETH_ECOSYNTH_PROTOCOL_H

//=============================================================================
//
//      protocol.h
//
//      Protocol between target-side and host-side ethernet support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Alternative licenses for eCos may be arranged by contacting the
// copyright holder(s).
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contributors:bartv
// Date:        2002-08-08
// Purpose:     Protocol definitions.
// Usage:       #include "protocol.h"
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Transmit: no arguments, up to 1514 bytes of outgoing data, no reply expected.
#define SYNTH_ETH_TX            0x01
// Receive: no arguments or data, the return code indicates whether or not
// there are more packets to follow, and the return data is up to 1514 bytes.
#define SYNTH_ETH_RX            0x02
// Start: one argument, non-zero -> promiscuous mode. No reply.
#define SYNTH_ETH_START         0x03
// Stop: no arguments or reply.
#define SYNTH_ETH_STOP          0x04
// Settings: no arguments or data. The return code holds the interrupt vector.
// The return data is six bytes of MAC address, plus a single byte indicating
// whether or not multicasting is supported. 
#define SYNTH_ETH_GETPARAMS     0x05
// Enable/disable multicasting. One argument, enable/disable.
#define SYNTH_ETH_MULTIALL      0x06

#endif  // CYGONCE_DEVS_ETH_ECOSYNTH_PROTOCOL.H

#ifndef CYGONCE_HAL_SYNTH_PROTOCOL_H
#define CYGONCE_HAL_SYNTH_PROTOCOL_H

//=============================================================================
//
//      synth_protocol.h
//
//      Generic protocol between eCos and the I/O auxiliary.
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
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contributors:bartv
// Date:        2002-08-05
// Purpose:     Protocol definitions.
// Description: This header file defines the protocol used between the
//              synthetic target HAL and the I/O auxiliary. The relevant
//              code in the latter is implemented in Tcl (making
//              use of the "binary" command) so this header file is
//              not actually used on the host-side. Instead the protocol
//              is defined in terms of byte arrays.
//
//              The header file is not exported to any higher-level packages
//              that wish to communicate with the auxiliary. Instead those
//              packages are expected to use the functions synth_auxiliary_xchgmsg()
//              and synth_auxiliary_instantiate().
//
// Usage:       #include "synth_protocol.h"
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Messages from the eCos synthetic target application to the I/O
// auxiliary are exactly 24 bytes long, consisting of six 32-bit
// little-endian integers. Inside the auxiliary they will be decoded
// using the Tcl binary scan command and the format i6. The top bit
// of RXLEN is overloaded to indicate whether or not a reply is expected
// at all.
#define SYNTH_REQUEST_LENGTH            24
#define SYNTH_REQUEST_DEVID_OFFSET       0
#define SYNTH_REQUEST_REQUEST_OFFSET     4
#define SYNTH_REQUEST_ARG1_OFFSET        8
#define SYNTH_REQUEST_ARG2_OFFSET       12
#define SYNTH_REQUEST_TXLEN_OFFSET      16
#define SYNTH_REQUEST_RXLEN_OFFSET      20

// And the response. This consists of two 32-bit little-endian integers,
// a result code and an actual rx_len field.
#define SYNTH_REPLY_LENGTH               8
#define SYNTH_REPLY_RESULT_OFFSET        0
#define SYNTH_REPLY_RXLEN_OFFSET         4

// Device 0 is special, it is for control messages with the auxiliary
// itself - for example, to instantiate a device.
#define SYNTH_DEV_AUXILIARY     0

// Requests intended directly for the auxiliary.

// Instantiate a device. arg1 and arg2 are ignored. The tx buffer
// holds a string for the given device. The rx buffer will be for a
// single integer, the device id or -1.
#define SYNTH_AUXREQ_INSTANTIATE                0x01

// Second-stage initialization, once all eCos device drivers have been
// activated.
#define SYNTH_AUXREQ_CONSTRUCTORS_DONE          0x02

// Get the current mask of pending interrupts. arg1 and arg2 are
// ignored, and there is no tx buffer. The reply code holds the irq
// pending mask. Normally there is no additional reply data, but if
// rx_len is non-zero then that indicates that the auxiliary has been
// asked to exit.
#define SYNTH_AUXREQ_GET_IRQPENDING             0x03

// Versioning. The core protocol cannot be changed without breaking
// lots of code. However it is still a good idea to allow the eCos
// application to verify that the host-side is the right version, in
// case new requests are added. arg1 and arg2 are ignored, there is no
// tx buffer or reply data, and the reply code holds the version.
#define SYNTH_AUXREQ_GET_VERSION                0x04

// The version has to be kept in synch with ecosynth.tcl
#define SYNTH_AUXILIARY_PROTOCOL_VERSION        0x01

// The console device is also provided by the architectural package,
// but only implements one function (write some output) so there is no
// need for any function codes.

#endif  // CYGONCE_HAL_SYNTH_PROTOCOL.H

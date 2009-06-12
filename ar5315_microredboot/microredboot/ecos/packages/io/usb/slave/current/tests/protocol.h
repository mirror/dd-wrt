//=================================================================
//
//        protocol.h
//
//        USB testing - host<->target protocol
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
// This header file is shared between target and host, and serves to
// define certain aspects of the protocol used between the two such
// as request codes.
//
// Author(s):     bartv
// Date:          2001-07-04
//####DESCRIPTIONEND####
//==========================================================================

// The largest control packet that will be sent or expected.
#define USBTEST_MAX_CONTROL_DATA        255

// The largest error message that can be sent.
#define USBTEST_MAX_MESSAGE             254

// The largest bulk transfer that will be sent or expected. Because of
// the use of the USB devfs support in the Linux kernel this is
// currently limited to a single page, i.e. 4096 bytes. To allow for
// padding, it is actually reduced to a slightly smaller size of 4090
// bytes. This should still be sufficient to test most interesting
// boundary conditions, apart from the transition to >64K. 
//
// A small amount of additional buffer space should be allocated by
// both host and target to allow for padding and possibly cache
// alignment. All other protocols involve smaller transfers than this,
// <= 64 bytes for interrupt transfers, <= 1023 for isochronous.
#define USBTEST_MAX_BULK_DATA           (4096)
#define USBTEST_MAX_BULK_DATA_EXTRA     1024

// The maximum number of tests that can be run concurrently. Each
// needs a separate thread, stack, and buffer so there are memory
// consumption implications.
#define USBTEST_MAX_CONCURRENT_TESTS    8

// Allow the host to find out the number of endpoints supported on
// this target. The theoretical maximum number of endpoints is 91
// (endpoint 0 control, endpoint 1-15 for both IN and OUT bulk, iso
// and interrupt) so a single byte response will suffice. The value
// and index fields are not used.
#define USBTEST_MAX_ENDPOINTS           91
#define USBTEST_ENDPOINT_COUNT          0x001

// Get hold of additional information about a specific entry in the
// array of endpoint details. The index field in the request
// identifies the entry of interest. The reply information is as per
// the usbs_testing_endpoint structure, and consists of:
//   1) one byte, the endpoint type (control, bulk, ...)
//   2) one byte, the endpoint number (as opposed to the array index number)
//   3) one byte for direction, USB_DIR_IN or USB_DIR_OUT
//   4) one byte for max_in_padding, usually 0
//   5) four bytes for min_size, 32-bit little-endian integer
//   6) four bytes for max_size, 32-bit little-endian integer
//   7) an additional n bytes for the devtab name, max ~240 bytes
//      although usually far less.
#define USBTEST_ENDPOINT_DETAILS        0x002

// Report pass or failure. The host will send a string of up to
// MAX_CONTROL_DATA bytes. The value and index fields are not used.
#define USBTEST_PASS                    0x003
#define USBTEST_PASS_EXIT               0x004
#define USBTEST_FAIL                    0x005
#define USBTEST_FAIL_EXIT               0x006

// Synchronise. One problem with the current eCos USB API is that
// there is no way to have a delayed response to a control message.
// Any such support would be tricky, there are significant differences
// in the hardware implementations and also timing constraints that
// need to be satisfied. Instead the entire response to any control
// request has to be prepared at DSR level. Usually this does not
// cause any problems, e.g. for handling the standard control
// messages, but for USB testing it may not be possible to handle a
// request entirely at DSR level - yet the next full request should
// not come in until the current one has been handled at thread-level.
// To work around this there is support for a synchronization control
// message. The return value is a single byte, 1 if the target is
// ready for new requests, 0 if there is a still a request being
// processed. The host can then perform some polling.
#define USBTEST_SYNCH                   0x007

// Abort. There is no easy way to get both host and target back to a
// known state, so abort the current test run.
#define USBTEST_ABORT                   0x008

// Cancel the current batch of tests. Something has gone wrong at the
// Tcl level, so any tests already prepared must be abandoned. No
// additional data is required.
#define USBTEST_CANCEL                  0x009

// Start the current batch of tests. No additional data is involved
// or expected.
#define USBTEST_START                   0x00A

// Has the current batch of tests finished? The host side polls the
// target at regular intervals for this information.
#define USBTEST_FINISHED                0x00B

// Set the test-terminated flag. Something has gone wrong, probably a
// timeout.
#define USBTEST_SET_TERMINATED          0x00C

// Get hold of recovery information for thread i in the target, where
// the index field of the request identifies the thread. The result
// is zero-bytes if the specified test has already finished, otherwise
// a recovery structure.
#define USBTEST_GET_RECOVERY            0x00D

// The target should perform a recovery action to unlock a thread
// on the host. The request holds a recovery structure.
#define USBTEST_PERFORM_RECOVERY        0x00E

// Collect the test result. The result is a single byte that indicates
// pass or fail, optionally followed by a failure message.
#define USBTEST_GET_RESULT              0x00F

// The current batch of tests has completed. Perform any final clean-ups.
#define USBTEST_BATCH_DONE              0x010

// Set the verbosity level on the target-side
#define USBTEST_VERBOSE                 0x011

// Perform endpoint initialization to ensure host and target
// can actually communicate over a given endpoint
#define USBTEST_INIT_CONTROL            0x012
#define USBTEST_INIT_BULK_IN            0x013
#define USBTEST_INIT_BULK_OUT           0x014
#define USBTEST_INIT_ISO_IN             0x015
#define USBTEST_INIT_ISO_OUT            0x016
#define USBTEST_INIT_INTERRUPT_IN       0x017
#define USBTEST_INIT_INTERRUPT_OUT      0x018


// A standard bulk test. The data consists of a UsbTest_Bulk
// structure, suitably packed.
#define USBTEST_TEST_BULK               0x040

// A control-IN test. The host will send reserved control messages with
// an appropriate length field, and the target should return that data.
#define USBTEST_TEST_CONTROL_IN         0x041

// Sub-protocols for reserved control messages, supporting test operations
// other than control-IN.
#define USBTEST_RESERVED_CONTROL_IN     0x01

// Work around a problem with control messages that involve additional
// data from host to target. This problem is not yet well-understood.
// The workaround involves sending multiple control packets with
// up to four bytes encoded in the index and value fields.
#define USBTEST_CONTROL_DATA1           0x0F1
#define USBTEST_CONTROL_DATA2           0x0F2
#define USBTEST_CONTROL_DATA3           0x0F3
#define USBTEST_CONTROL_DATA4           0x0F4


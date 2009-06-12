// ====================================================================
//
//      ser_arm_aim711_s3c4510.inl
//
//      ARM Industrial Module AIM 711 Serial I/O Interface Module 
//      (interrupt driven) for use with s3c4510 driver.
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars.Lindqvist@combitechsystems.com
// Contributors: jlarmour
// Date:         2001-10-19
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

#include <pkgconf/hal.h>        // Value CYGNUM_HAL_ARM_S3C4510_CLOCK_SPEED needed
#include <cyg/hal/hal_intr.h>

#define CYGNUM_HAL_ARM_S3C4510_CLOCK_SPEED CYGNUM_HAL_CPUCLOCK

#ifdef CYGPKG_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0
static s3c4510_serial_info s3c4510_serial_info0 = {0x07FFd000, 
                                           CYGNUM_HAL_INTERRUPT_UART0_TX,
                                           CYGNUM_HAL_INTERRUPT_UART0_RX};
#if CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_BUFSIZE > 0
static unsigned char s3c4510_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_BUFSIZE];
static unsigned char s3c4510_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(s3c4510_serial_channel0,
                                       s3c4510_serial_funs, 
                                       s3c4510_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &s3c4510_serial_out_buf0[0], sizeof(s3c4510_serial_out_buf0),
                                       &s3c4510_serial_in_buf0[0], sizeof(s3c4510_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(s3c4510_serial_channel0,
                      s3c4510_serial_funs, 
                      s3c4510_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(s3c4510_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             s3c4510_serial_init, 
             s3c4510_serial_lookup,     // Serial driver may need initializing
             &s3c4510_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1
static s3c4510_serial_info s3c4510_serial_info1 = {0x07FFe000,
                                           CYGNUM_HAL_INTERRUPT_UART1_TX,
                                           CYGNUM_HAL_INTERRUPT_UART1_RX};
#if CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_BUFSIZE > 0
static unsigned char s3c4510_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_BUFSIZE];
static unsigned char s3c4510_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(s3c4510_serial_channel1,
                                       s3c4510_serial_funs, 
                                       s3c4510_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &s3c4510_serial_out_buf1[0], sizeof(s3c4510_serial_out_buf1),
                                       &s3c4510_serial_in_buf1[0], sizeof(s3c4510_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(s3c4510_serial_channel1,
                      s3c4510_serial_funs, 
                      s3c4510_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(s3c4510_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             s3c4510_serial_init, 
             s3c4510_serial_lookup,     // Serial driver may need initializing
             &s3c4510_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AIM711_S3C4510_SERIAL1

// EOF ser_arm_aim711_s3c4510.inl

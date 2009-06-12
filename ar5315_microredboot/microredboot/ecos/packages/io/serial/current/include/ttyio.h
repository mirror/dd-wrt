#ifndef CYGONCE_TTYIO_H
#define CYGONCE_TTYIO_H
// ====================================================================
//
//      ttyio.h
//
//      Device I/O 
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
// Author(s):   gthomas
// Contributors:        gthomas
// Date:        1999-02-04
// Purpose:     Special support for tty I/O devices
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// This file contains the user-level visible I/O interfaces

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/io/serialio.h>
#include <cyg/io/config_keys.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    cyg_uint32         tty_out_flags;
    cyg_uint32         tty_in_flags;
} cyg_tty_info_t;

// TTY flags - used to control behaviour when sending data to tty
#define CYG_TTY_OUT_FLAGS_CRLF  0x0001  // Map '\n' => '\r\n' on output

#define CYG_TTY_OUT_FLAGS_DEFAULT (CYG_TTY_OUT_FLAGS_CRLF)

// TTY flags - used to control behaviour when receiving data from tty
#define CYG_TTY_IN_FLAGS_CR      0x0001  // Map '\r' => '\n' on input
#define CYG_TTY_IN_FLAGS_CRLF    0x0002  // Map '\r\n' => '\n' on input
#define CYG_TTY_IN_FLAGS_ECHO    0x0004  // Echo characters as processed
#define CYG_TTY_IN_FLAGS_BINARY  0x0008  // No input processing

#define CYG_TTY_IN_FLAGS_DEFAULT (CYG_TTY_IN_FLAGS_CR|CYG_TTY_IN_FLAGS_ECHO)

#ifdef __cplusplus
}
#endif

#endif  /* CYGONCE_TTYIO_H */
/* EOF ttyio.h */

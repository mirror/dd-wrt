#ifndef CYGONCE_INFRA_DIAG_H
#define CYGONCE_INFRA_DIAG_H

/*=============================================================================
//
//      diag.h
//
//      Diagnostic Routines for Infra Development
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):   nickg
// Contributors:        nickg, gthomas
// Date:        1998-03-02
// Purpose:     Diagnostic Routines for Infra Development
// Description: Diagnostic routines for use during infra development.
// Usage:       #include <cyg/infra/diag.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================*/

#include <pkgconf/infra.h>
#include <cyg/infra/cyg_type.h>
#include <stdarg.h>

/*---------------------------------------------------------------------------*/
/* Diagnostic routines                                                       */

externC void diag_init(void);         /* Initialize, call before any others*/

externC void diag_write_char(char c); /* Write single char to output       */

externC void diag_write_string(const char *psz); /* Write zero terminated string */

externC void diag_write_dec( cyg_int32 n);    /* Write decimal value       */

externC void diag_write_hex( cyg_uint32 n);   /* Write hexadecimal value   */

externC void diag_dump_buf(void *buf, CYG_ADDRWORD len);
externC void diag_dump_buf_32bit(void *buf, CYG_ADDRWORD len);
externC void diag_dump_buf_16bit(void *buf, CYG_ADDRWORD len);
typedef int __printf_fun(const char *fmt, ...);
externC void diag_vdump_buf_with_offset(__printf_fun *pf,
                                        cyg_uint8     *p, 
                                        CYG_ADDRWORD   s, 
                                        cyg_uint8     *base);
externC void diag_dump_buf_with_offset(cyg_uint8     *p, 
                                       CYG_ADDRWORD   s, 
                                       cyg_uint8     *base);

externC void diag_dump_buf_with_offset_32bit(cyg_uint32 *p, 
                                             CYG_ADDRWORD     s, 
                                             cyg_uint32      *base);

externC void diag_dump_buf_with_offset_16bit(cyg_uint16 *p, 
                                             CYG_ADDRWORD     s, 
                                             cyg_uint16      *base);

externC int  diag_printf( const char *fmt, ... );  /* Formatted print      */

externC void diag_init_putc(void (*putc)(char c, void **param));
externC int  diag_sprintf(char *buf, const char *fmt, ...);
externC int  diag_snprintf(char *buf, size_t len, const char *fmt, ...);
externC int  diag_vsprintf(char *buf, const char *fmt, va_list ap);
externC int  diag_vprintf(const char *fmt, va_list ap);


/*---------------------------------------------------------------------------*/
/* Internal Diagnostic MACROS                                                */

#define DIAG_DEVICE_START_SYNC()
#define DIAG_DEVICE_END_SYNC()

/*---------------------------------------------------------------------------*/
#endif /* CYGONCE_INFRA_DIAG_H */
/* EOF diag.h */

#ifndef CYGONCE_INFRA_DIAG_H
#define CYGONCE_INFRA_DIAG_H

/*=============================================================================
//
//      diag.h
//
//      Diagnostic Routines for Infra Development
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
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

#ifdef CYGDBG_INFRA_DIAG_PRINTF_USE_VARARG
#include <stdarg.h>
#endif

#ifdef CYGDBG_INFRA_DIAG_USE_DEVICE
#include <cyg/io/io_diag.h>
#endif

/*---------------------------------------------------------------------------*/
/* Diagnostic routines                                                       */

externC void diag_init(void);         /* Initialize, call before any others*/

externC void diag_write_char(char c); /* Write single char to output       */

externC void diag_write_string(const char *psz); /* Write zero terminated string */

externC void diag_write_dec( cyg_int32 n);    /* Write decimal value       */

externC void diag_write_hex( cyg_uint32 n);   /* Write hexadecimal value   */

externC void diag_dump_buf(void *buf, CYG_ADDRWORD len);

#ifdef CYGDBG_INFRA_DIAG_PRINTF_USE_VARARG

externC void diag_printf( const char *fmt, ... );  /* Formatted print      */

#else

// This function deliberately has a K&R prototype to avoid having to use
// varargs, or pad arglists or anything grody like that.

#warning CYGDBG_INFRA_DIAG_PRINTF_USE_VARARG not enabled
#warning Expect a "function declaration isn't a prototype" warning

externC void diag_printf(/* const char *fmt, CYG_ADDRWORD, CYG_ADDRWORD,
                         CYG_ADDRWORD, CYG_ADDRWORD, CYG_ADDRWORD,
                         CYG_ADDRWORD, CYG_ADDRWORD, CYG_ADDRWORD */);

#endif

/*---------------------------------------------------------------------------*/
/* Internal Diagnostic MACROS                                                */

#ifdef CYGDBG_INFRA_DIAG_USE_DEVICE
#define DIAG_DEVICE_START_SYNC()        diag_device_start_sync()
#define DIAG_DEVICE_END_SYNC()          diag_device_end_sync()
#else
#define DIAG_DEVICE_START_SYNC()
#define DIAG_DEVICE_END_SYNC()
#endif

/*---------------------------------------------------------------------------*/
#endif /* CYGONCE_INFRA_DIAG_H */
/* EOF diag.h */

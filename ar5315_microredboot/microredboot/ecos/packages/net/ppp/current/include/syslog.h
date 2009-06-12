#ifndef CYGONCE_PPP_SYSLOG_H
#define CYGONCE_PPP_SYSLOG_H
// ====================================================================
//
//      syslog.h
//
//      PPP syslog stub
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 eCosCentric Ltd.
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
// copyright holder.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2003-06-01
// Purpose:             Syslog
// Description:         This header contains definitions for using syslog
//                      in the PPP code.
//
//####DESCRIPTIONEND####
//
// ====================================================================

#include <cyg/infra/diag.h>

#include <cyg/ppp/names.h>

// ====================================================================
// Syslog functions
//
// We ignore setlogmask() but syslog() is a simple stub that goes to
// diag_printf(). 

#define setlogmask(x)

void syslog( int level, char *fmt, ... );

// ====================================================================
// Log level flags
//
// These are duplicates of the defines in sys/param.h.

#define LOG_ERR      0x0001
#define LOG_WARNING  0x0002
#define LOG_NOTICE   0x0004
#define LOG_INFO     0x0008
#define LOG_DEBUG    0x0010
#define LOG_MDEBUG   0x0020
#define LOG_IOCTL    0x0040
#define LOG_ADDR     0x0100
#define LOG_FAIL     0x0200
#define LOG_INIT     0x0080
#define LOG_EMERG    0x4000
#define LOG_CRIT     0x8000

// ====================================================================
// Debug facility
//
// Switch these defines over to enable some debugging messages

//#define db_printf diag_printf
#define db_printf(fmt, ... )

// ====================================================================
#endif // CYGONCE_PPP_SYSLOG_H

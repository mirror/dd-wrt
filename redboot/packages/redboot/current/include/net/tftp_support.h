//==========================================================================
//
//      net/tftp_support.h
//
//      Stand-alone TFTP support for RedBoot
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _TFTP_SUPPORT_H_
#define _TFTP_SUPPORT_H_

#include "net.h"

/*
 * File transfer modes
 */
#define TFTP_NETASCII   0              // Text files
#define TFTP_OCTET      1              // Binary files

/*
 * Errors
 */
#define	TFTP_ENOTFOUND   1   /* file not found */
#define	TFTP_EACCESS     2   /* access violation */
#define	TFTP_ENOSPACE    3   /* disk full or allocation exceeded */
#define	TFTP_EBADOP      4   /* illegal TFTP operation */
#define	TFTP_EBADID      5   /* unknown transfer ID */
#define	TFTP_EEXISTS     6   /* file already exists */
#define	TFTP_ENOUSER     7   /* no such user */
#define TFTP_TIMEOUT     8   /* operation timed out */
#define TFTP_NETERR      9   /* some sort of network error */
#define TFTP_INVALID    10   /* invalid parameter */
#define TFTP_PROTOCOL   11   /* protocol violation */
#define TFTP_TOOLARGE   12   /* file is larger than buffer */

/*
 * Client support
 */

extern int   tftp_stream_open(connection_info_t *info, int *err);
extern int   tftp_stream_read(char *buf, int len, int *err);
extern void  tftp_stream_close(int *err);
extern char *tftp_error(int err);

#define TFTP_TIMEOUT_PERIOD 5
#define TFTP_TIMEOUT_MAX   15
#define TFTP_RETRIES_MAX    5

#define TFTP_PORT           69

extern getc_io_funcs_t tftp_io;

#endif // _TFTP_SUPPORT_H_

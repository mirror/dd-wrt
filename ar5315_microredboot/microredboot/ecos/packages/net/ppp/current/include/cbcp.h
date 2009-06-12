//==========================================================================
//
//      include/cbcp.h
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

#ifndef CBCP_H
#define CBCP_H

typedef struct cbcp_state {
    int    us_unit;	/* Interface unit number */
    u_char us_id;		/* Current id */
    u_char us_allowed;
    int    us_type;
    char   *us_number;    /* Telefone Number */
} cbcp_state;

extern cbcp_state cbcp[];

extern struct protent cbcp_protent;

#define CBCP_MINLEN 4

#define CBCP_REQ    1
#define CBCP_RESP   2
#define CBCP_ACK    3

#define CB_CONF_NO     1
#define CB_CONF_USER   2
#define CB_CONF_ADMIN  3
#define CB_CONF_LIST   4
#endif

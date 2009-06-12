#ifndef CYGONCE_PPP_PPP_H
#define CYGONCE_PPP_PPP_H

//==========================================================================
//
//      ppp.h
//
//      PPP system interface
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003, 2004 eCosCentric Ltd.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holder.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:        2002-05-22
// Purpose:     PPP interface
// Description: PPP sybsystem interface definitions
// Usage:       #include "cyg/ppp/ppp.h"
//              ...
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/ppp.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/io/serialio.h>

#define MAXNAMELEN 256
#define MAXSECRETLEN 256

// -------------------------------------------------------------------------
// PPP instance

typedef CYG_ADDRWORD cyg_ppp_handle_t;

// -------------------------------------------------------------------------
/* PPP statistics */

typedef struct {
	int auth_failures;        /* PAP or CHAP failures */
	int no_proto;             /* No network protocol running */
	int idle_timeout;         /* Idle timer expired */
	int connect_time_expired; /* Max connect time expired */
	int loopback;             /* Loopback detected */
	int no_response;          /* Peer not responding */
} cyg_ppp_stats_t;

extern cyg_ppp_stats_t cyg_ppp_stats; /* PPP statistics */

// -------------------------------------------------------------------------

typedef struct
{
    unsigned int
        debug           : 1,            /* Debug flag */
        kdebugflag      : 5,	        /* Tell kernel to print debug messages */
        default_route   : 1,            /* Set up default route via PPP link */
        modem           : 1,            /* Use modem control lines */
        flowctl         : 2,            /* Flow control, see below */
        refuse_pap      : 1,	        /* Don't wanna auth. ourselves with PAP */
	refuse_chap     : 1	        /* Don't wanna auth. ourselves with CHAP */
        ;

    cyg_serial_baud_rate_t      baud;   /* serial line baud rate */
    
    int	        idle_time_limit;        /* Shut down link if idle for this long */
    int         maxconnect;             /* Maximum connect time (seconds) */

    cyg_uint32  our_address;            /* Our IP address */
    cyg_uint32  his_address;            /* His IP address */

    char        **script;               /* CHAT connection script */
    
    char	user[MAXNAMELEN];       /* Our name for authenticating ourselves */
    char	passwd[MAXSECRETLEN];   /* Password for PAP and secret for CHAP */
                          
} cyg_ppp_options_t;

// -------------------------------------------------------------------------
/* Values for flowctl field */

#define CYG_PPP_FLOWCTL_DEFAULT         0       /* Default to current setting        */
#define CYG_PPP_FLOWCTL_NONE            1       /* No flow control - not recommended */
#define CYG_PPP_FLOWCTL_HARDWARE        2       /* Hardware flow control - CTS/RTS   */
#define CYG_PPP_FLOWCTL_SOFTWARE        3       /* Software flow control - XON/XOFF  */

// -------------------------------------------------------------------------

externC cyg_int32 cyg_ppp_options_init( cyg_ppp_options_t *options );

// -------------------------------------------------------------------------

externC cyg_ppp_handle_t cyg_ppp_up( const char *devnam,
                                     const cyg_ppp_options_t *options );

// -------------------------------------------------------------------------

externC cyg_int32 cyg_ppp_down( const cyg_ppp_handle_t handle );


// -------------------------------------------------------------------------

externC cyg_int32 cyg_ppp_wait_up( cyg_ppp_handle_t handle );

externC void cyg_ppp_wait_down( cyg_ppp_handle_t handle );

// -------------------------------------------------------------------------

externC cyg_int32 cyg_ppp_chat( const char *devname,
                                const char *script[] );


// -------------------------------------------------------------------------
#endif // CYGONCE_PPP_PPP_H multiple inclusion protection
// EOF ppp.h

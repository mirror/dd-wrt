#ifndef CYGONCE_PPP_PPP_IO_H
#define CYGONCE_PPP_PPP_IO_H
// ====================================================================
//
//      ppp_io.h
//
//      PPP IO interface
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
// Purpose:             Internal PPP interfaces
// Description:         This header describes the interfaces between
//                      the PPP daemon system dependent code in sys-ecos.c
//                      and the PPP line-discipline-derived code in ppp_io.c.
//
//####DESCRIPTIONEND####
//
// ====================================================================

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/io_fileio.h>

#include <sys/uio.h>

#include <cyg/io/io.h>
#include <cyg/io/serial.h>
#include <cyg/io/serial.h>

#include <cyg/kernel/kapi.h>

#include <cyg/ppp/names.h>

#include <cyg/ppp/ppp.h>

// ====================================================================
// Fake TTY structure
//
// This is passed into the line discipline code and pretends to be a
// tty device. In actual fact we use it to store all the state for the
// connection.

struct tty
{
    // IO device handle.
    cyg_io_handle_t             t_handle;

    // Pointer to softc structure. This is the only field that the
    // ex-BSD code actually accesses.
    void                        *t_sc;

    // Pointer to user's options
    const cyg_ppp_options_t     *options;

    // Parameters for PPPD thread
    cyg_handle_t                pppd_thread;
    volatile int                pppd_wakeup;
    volatile cyg_bool_t         pppd_thread_running;

    // Parameters for transmit thread
    cyg_sem_t                   tx_sem;
    cyg_handle_t                tx_thread;
    volatile cyg_bool_t         tx_thread_running;

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    // Serial line control
    cyg_serial_line_status_callback_t serial_callbacks;
    volatile cyg_uint32         carrier_detected;
#endif
    
    // Alarm for implementing wait_input() timeout.
    cyg_handle_t                alarm;
    cyg_alarm                   alarm_obj;
    
};

externC struct tty ppp_tty;

// ====================================================================
// This is called at the correct time in the PPPD to set up the
// options from the user's request. This has to be done after the
// protocols etc. have intitialized.

externC void cyg_ppp_options_install( const cyg_ppp_options_t *options );

// ====================================================================
// Definition of serial callback function. Used for carrier detection
// etc.

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
externC void cyg_ppp_serial_callback( cyg_serial_line_status_t *s,
                                      CYG_ADDRWORD priv );
#endif

// ====================================================================
// These are all renamed versions of the PPP line discipline
// functions. The PPPD code calls these directly to control the PPP
// driver and to send and receive control packets.

extern int	cyg_ppp_pppopen __P((struct tty *tp));

extern int	cyg_ppp_ppptioctl __P((struct tty *tp, u_long cmd, caddr_t data, int flag));
                                       
extern int	cyg_ppp_pppinput __P((int c, struct tty *tp));

extern int	cyg_ppp_pppwrite __P((struct tty *tp, struct uio *uio, int flag));

extern int	cyg_ppp_pppread __P((struct tty *tp, struct uio *uio, int flag));

extern int	cyg_ppp_pppclose __P((struct tty *tp, int flag));

extern int	cyg_ppp_pppcheck __P((struct tty *tp));

// ====================================================================
// Start PPP transmission. This is called from the TX thread to cause
// any pending packets to be sent.

extern int	cyg_ppp_pppstart __P((struct tty *tp));

// ====================================================================
#endif // CYGONCE_PPP_PPP_IO_H

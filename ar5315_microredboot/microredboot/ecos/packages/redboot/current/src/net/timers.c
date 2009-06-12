//==========================================================================
//
//      net/timers.c
//
//      Stand-alone networking support for RedBoot
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

#include <net/net.h>

static timer_t *tmr_list;


/*
 * Set a timer. Caller is responsible for providing the timer_t struct.
 */
void
__timer_set(timer_t *t, unsigned long delay,
	    tmr_handler_t handler, void *user_data)
{
    timer_t *p;

    t->delay = delay;
    t->start = MS_TICKS();
    t->handler = handler;
    t->user_data = user_data;

    for (p = tmr_list; p; p = p->next)
	if (p == t) {
	    return;
	}

    t->next = tmr_list;
    tmr_list = t;
}


/*
 * Remove a given timer from timer list.
 */
void
__timer_cancel(timer_t *t)
{
    timer_t *prev, *p;

    for (prev = NULL, p = tmr_list; p; prev = p, p = p->next)
	if (p == t) {
	    if (prev)
		prev->next = p->next;
	    else
		tmr_list = p->next;
	    return;
	}
}


/*
 * Poll timer list for timer expirations.
 */
void
__timer_poll(void)
{
    timer_t *prev, *t;

    prev = NULL;
    t = tmr_list;
    while (t) {
	if ((MS_TICKS_DELAY() - t->start) >= t->delay) {

	    /* remove it before calling handler */
	    if (prev)
		prev->next = t->next;
	    else
		tmr_list = t->next;
	    /* now, call the handler */
	    t->handler(t->user_data);
	    
	    /*
	     * handler may be time consuming, so start
	     * from beginning of list.
	     */
	    prev = NULL;
	    t = tmr_list;
	} else {
	    prev = t;
	    t = t->next;
	}
    }
}

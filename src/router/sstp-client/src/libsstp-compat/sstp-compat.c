/*!
 * @brief Provide compability layer for sstp-client and other libraries
 *
 * @file sstp-comapt.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <sstp-compat.h>

#ifndef HAVE_LIBEVENT2

event_st *event_new(event_base_st *base, int sock, short fl, 
    event_fn cb, void *arg)
{
    event_st *event = calloc(1, sizeof(event_st));
    if (event)
    {
        event_set(event, sock, fl, cb, arg);
        event_base_set(base, event);
    }

    return event;
}


void event_free(event_st *event)
{
    free(event);
}

#endif

/*!
 * @brief Provide compability layer for sstp-client and other libraries
 *
 * @file sstp-comapt.h
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

#ifndef __SSTP_COMPAT_H__
#define __SSTP_COMPAT_H__

#include <event.h>
#include <openssl/ssl.h>

#if HAVE_LIBEVENT2
#include <event2/dns.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#else
#include <event.h>
#endif


typedef struct event_base event_base_st;
typedef struct event event_st;
typedef void (*event_fn)(int, short, void *);

#ifndef HAVE_LIBEVENT2

/*! 
 * @brief provide a dummy function for missing event_new of libevent 1.4
 */
event_st *event_new(event_base_st *base, int sock, short fl, 
    event_fn cb, void *arg);


/*! 
 * @brief provide a dummy function for missing event_free of libevent 1.4
 */
void event_free(event_st *event);

#endif  /* #ifndef HAVE_LIBEVENT2 */



#endif	/* #ifndef __SSTP_COMMON_H__ */

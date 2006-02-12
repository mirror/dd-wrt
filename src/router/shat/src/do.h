/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: do.h,v 1.4 2005/02/26 23:37:11 jordan Exp $
 */

#ifndef __SHAT_DO_H
#define __SHAT_DO_H


typedef union _do {
# ifdef __SHAT_DO_PRIVATE
    struct {
        int   length, max_fd;
        void (*idle) (void*); /* action to be taken on idle looping */
        void           *data; /* to be passed to the idle fn */
        int  default_timeout; /* default timeout (used vy the rescheduler) */
        int          timeout; /* currently active timeout */
        time_t           due; /* idle timer scheduler, next run */
        struct timeval    tv; /* idle time scheduler timeout */
        struct timeval  *tvp; /* ... to be used in the select loop */
    } def ;
    struct {
        int            rfd  ; /* read file handle to listen on */
        void  (*cb) (void*) ; /* action to take on incoming data */
        void         *data  ; /* to be passed to cb on activation */
    } set ;
    /* open end ... */
# else
    char any ;
# endif
} DO ;

extern DO  *do_init    (void(*idle)(void*),void*data) ;
extern DO  *do_action  (DO*,int fd,void(*action)(void*),void*data) ;
extern int  do_loop    (DO*,int (*eval)(int), int tmo);
extern void do_resched (DO*,int);
extern void do_destroy (DO*);

#endif /* __SHAT_DO_H */

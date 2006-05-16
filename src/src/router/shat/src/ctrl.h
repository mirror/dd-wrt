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
 * $Id: ctrl.h,v 1.10 2005/05/05 14:25:41 jordan Exp $
 */

#ifndef __SHAT_CTRL_H
#define __SHAT_CTRL_H
#ifndef DISCARD_CMD_SOCKET

#include <netinet/in.h>
#include <netinet/ether.h>
#include <pwd.h>

// /* defined in util.h */
// #define CTRL_REQUEST_PATH  "/var/run/shat/server"
// #define CTRL_IOBUF_SIZE    2000
// #define CTRL_REQUEST_LOG   0x10
// #define CTRL_REPLY_LOG     0x20

typedef struct _ctrl {
# ifdef __CTRL_PRIVATE
    int cmd_fd ;

    struct sockaddr_un un;              /* the socket path */
    unsigned    length_un;              /* length */

    char iobuf [CTRL_IOBUF_SIZE];       /* response data buffer */
    unsigned        ioinx;              /* append to io puffer */
    char   iobuf_reply_ok;              /* flag that a reply is wanted */
    char    *reply_socket;              /* reply socket path */
    
    /* call back functions */
    unsigned          dim;              /* number of handlers */
    struct _service {
        char                name [19];  /* service by name */
        char                 use_ctrl;  /* ignore fn_state and use CTRL* */
        void                *fn_state;
        void (*fn) (void*,int,const char**);  /* call back hook */
    } service [1] ;
# else
    char any ;
# endif
} CTRL ;

extern CTRL *ctrl_init (gid_t gid);
extern CTRL *ctrl_add  (CTRL*,const char*,
                        void(*)(void*,int,const char**),void*);
extern void  ctrl_handler (CTRL*);
extern void  ctrl_usage     (void *,int,const char**);
extern void  ctrl_verbosity (void *,int,const char**);
extern int   ctrl_reply   (CTRL*, const char *format, ...);
extern void  ctrl_destroy (CTRL*);
extern int   ctrl_fd      (CTRL*);

#endif /* DISCARD_CMD_SOCKET */
#endif /* __SHAT_CTRL_H */

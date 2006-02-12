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
 * $Id: ctrl.c,v 1.8 2005/05/05 14:25:41 jordan Exp $
 */

#ifndef DISCARD_CMD_SOCKET

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/un.h>

#include "util.h"

#define __CTRL_PRIVATE
#include "ctrl.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */

#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

#define DIM(a) (sizeof (a) / sizeof (a [0]))

/* ----------------------------------------------------------------------- *
 * Private
 * ----------------------------------------------------------------------- */

/* example for a communication handler */
void usage_handler (void *ctrl, int argc, const char **argv) {
    CTRL *ct = ctrl;
    char **p;
    int n ;

    /* prepend by a first argument "?" */
    p = (char **)xmalloc ((argc + 1) * sizeof (*argv)) ;
    memcpy (p + 1, argv, argc * sizeof (*argv)) ;
    p [0] = "?";
        
    for (n = 0; n < ct->dim; n++) {
        if (ct->service [n].fn != ctrl_usage) { /* dont call yourself */
            struct _service *svc = ct->service + n ;
            p [1] = svc->name ;
            (*svc->fn) (svc->use_ctrl ? ct : svc->fn_state,
                        argc + 1,
                        (const char**)p);
        }
    }

    free (p);
}

/* ----------------------------------------------------------------------- *
 * Public
 * ----------------------------------------------------------------------- */

/* execute request */
void ctrl_handler (CTRL *ct) {
    int argc, argi, n;
    char buf [2048], *argv [20], *p;
    struct sockaddr_un sun ;
    socklen_t slen = sizeof (sun) ;

    /* read request into buffer - leave one byte before input starts */
    if ((n = recvfrom (ct->cmd_fd, buf + 1, sizeof (buf) - 2, MSG_TRUNC,
                       (struct sockaddr*)&sun, &slen)) < 0) {
        logger (LOG_ERR, "command: Error reading from %s: ",
                ct->un.sun_path, strerror (errno));
        return ;
    }
    if (n == 0) return ;
    buf [n+1] = '\0'; /* starting at buf+1, zero terminated - easy parsing */

#   ifdef CTRL_REQUEST_LOG
    if (verbosity & CTRL_REQUEST_LOG)
        logger (LOG_DEBUG, "command request: \"%s\"", buf+1);
#   endif

    /* parse buffer, split into space separated arguments */
    for (argc = 0, p = buf + 1; p && argc < DIM (argv); argc ++) {
        while (*p == ' ') p ++ ;
        argv [argc] = p ;
        p [-1] = '\0' ; /* set delimiter */
        p = strchr (p, ' ');
    }
    if (argc == 0) return ;

    /* check whether we got a reply socket */
    if (argv [0] [0] == '/') {
        if (-- argc == 0) return ;    /* there is nothing we can do, here */
        ct->reply_socket = argv [0];  /* publish the reply socket path */
        argi = 1 ;                    /* first request argument */
    }
    else {
        ct->reply_socket = 0 ;
        argi = 0 ;                    /* first request argument */
    }

    /* setup for output */
    ct->ioinx          = 0 ;
    ct->iobuf_reply_ok = 0 ;

    /* find handler by its registered name and run it */
    for (n = 0; n < ct->dim; n++) {
        if (strcmp (ct->service [n].name, argv [argi]) == 0) {
            struct _service *svc = ct->service + n ;

            /* call hander */
            (*svc->fn)
                (svc->use_ctrl ? ct : svc->fn_state,
                 argc,
                 (const char**)argv + argi);

            /* send back collected output */
            goto reply_to_sender;
        }
     }

    /* no handler: invoke the usage handler if we can send back data */
    if (ct->reply_socket == 0) return ;

    ctrl_reply (ct, "-ERR: unknown command \"%s\"\n", argv [argi]);
    usage_handler (ct, argc, (const char**)argv + argi);

 reply_to_sender:
    if (ct->iobuf_reply_ok == 0) return ;

    /* send the command buf to server */
    strncpy (sun.sun_path, ct->reply_socket, UNIX_PATH_MAX);
    sun.sun_path [UNIX_PATH_MAX-1] = '\0' ;
    slen =
        offsetof (struct sockaddr_un, sun_path) +
        strlen (sun.sun_path) + 1;

#   ifdef CTRL_REPLY_LOG
    if (verbosity & CTRL_REPLY_LOG)
        logger (LOG_DEBUG, "command reply: \"%s\"", ct->iobuf);
#   endif

    if (sendto (ct->cmd_fd, ct->iobuf, ct->ioinx, 0,
                (struct sockaddr*)&sun, slen) < 0) {
        ct->iobuf [ct->ioinx - 1] = '\0';
        logger (LOG_ERR, "command: Cannot reply to %s: %s",
                ct->reply_socket, strerror (errno));
    }
}


/* collect response data - returns the number chars left in the buffer */
int ctrl_reply (CTRL *ct, const char *format, ...) {

    int left = sizeof (ct->iobuf)  - ct->ioinx;

    if (ct->reply_socket == 0) {           /* no reply expected */
        errno = EPIPE ;                    /* Broken pipe */
        return -1 ;
    }
 
    if (format) {

        va_list ap;
        va_start (ap, format);

        if (left > 0) {
        
            int n = vsnprintf (ct->iobuf + ct->ioinx, /* append data here */
                               left,       /* not more than this many */
                               format,     /* format */
                               ap);        /* these arguments */

            if (n < 0)                     /* error occured */
                left = -1 ;
            else if (n > left) {           /* output was truncated */
                left      = -1 ;
                ct->ioinx = sizeof (ct->iobuf) ;
                errno     = ENOSPC ;       /* No space left on device */
            }
            else {
                left      -= n ;           /* bookkeeping ... */
                ct->ioinx += n ;
            }
        }
        else {                             /* flag overflow */
            left   = -1 ;
            errno  = E2BIG ;               /*  Argument list too long */
        }

        va_end (ap);
    }

    ct->iobuf_reply_ok = 1 ;              /* remember that we got in, here */
    return left ;
}



/* example for some easy communication handlers */
void ctrl_usage (void *ctrl, int argc, const char **argv) {
    CTRL *ct = ctrl;

    if (ctrl_reply (ct, "+OK\n") >= 0)
        usage_handler (ct, argc, argv);
}


void ctrl_verbosity (void *ctrl, int argc, const char **argv) {
    CTRL *ct = ctrl;
    const char *me = argv [0];
    unsigned old ;

    /* check for a help line request */
    if (me [0] == '?' && !me [1]) { me = argv [1]; goto help; }

    old = verbosity ;

    if (argc > 1) {
        unsigned noise ;
        if (integer (&noise, argv [1]) == 0)
            goto usage ;
        verbosity = noise ;
    }

    ctrl_reply (ct, "+OK: %#x", old) ;
    return ;

usage:
    ctrl_reply (ct, "-ERR: usage: ");
help:
    ctrl_reply (ct, "%s [verbosity] -- shows/sets noise level\n", me);
}


/* ----------------------------------------------------------------------- *
 * Constructor/Destructor
 * ----------------------------------------------------------------------- */

CTRL *ctrl_init (gid_t gid) {
    CTRL ct ;
    char *dup = xstrdup (CTRL_REQUEST_PATH) ;
    char *dir = dirname (dup) ; /* dirname modifies its argument */
        
    ct.dim = 0;
    mkdir (dir, 0700); /* may fail when existing, already */
    if (gid) {
        chown (dir, -1, gid);
        chmod (dir, 0750);
    }
    free (dup);
    
    if ((ct.cmd_fd = socket (PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        logger (LOG_ERR, "Cannot create command socket: %s",
                strerror (errno));
        return 0;
    }

    ct.un.sun_family = AF_UNIX ;
    strncpy (ct.un.sun_path, CTRL_REQUEST_PATH, UNIX_PATH_MAX);
    ct.un.sun_path [UNIX_PATH_MAX-1] = '\0' ;
    unlink (ct.un.sun_path);

    /* The length of the address is the offset of the start of the filename,
       plus its length, plus one for the terminating null byte. */
    ct.length_un =
        offsetof (struct sockaddr_un, sun_path) +
        strlen (ct.un.sun_path) + 1;
    if (bind (ct.cmd_fd, (struct sockaddr*)&ct.un, ct.length_un) < 0) {
        logger (LOG_ERR, "Cannot bind command socket to %s: ", 
                ct.un.sun_path, strerror (errno));
        return 0;
    }

    if (gid) {
        chown (ct.un.sun_path, -1, gid);
        chmod (ct.un.sun_path, 0770);
    }
    
    return xmemdup (&ct);
}


void ctrl_destroy (CTRL *ct) {
    close (ct->cmd_fd);
    unlink (ct->un.sun_path);
    free (ct) ;
}


int ctrl_fd (CTRL *ct) {
    return ct->cmd_fd ;
}


CTRL *ctrl_add (CTRL                          *ct,
                const char                  *name,
                void (*handler)(void*,int,const char**),
                void               *handler_state) {

    if (ct && name && handler) {
        struct _service *svc;
        int use_ctrl = (ct == handler_state) ;
        
        /* get space for an additional entry */
        ct = (CTRL*) xrealloc
            (ct, sizeof (CTRL) + ct->dim * sizeof (struct _service)) ;

        /* populate new entry */
        svc = ct->service + (ct->dim ++);
        
        strncpy (svc->name, name, sizeof (svc->name));
        svc->name [sizeof (svc->name) - 1] = 0;
        svc->fn       = handler ;

        /* there is no fixup procedure needed when the CTRL ptr has
           changed due to a memory reallocation by xrealloc() */

        if ((svc->use_ctrl = use_ctrl) == 0)
            svc->fn_state = handler_state ;
    }

    return ct ;
}

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
#endif /* DISCARD_CMD_SOCKET */

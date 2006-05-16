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
 * $Id: shatc.c,v 1.4 2005/04/30 11:38:23 jordan Exp $
 */

/* simple stupid example of a command line client */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <signal.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "util.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */
static char *client = 0;

#define REPLY_SOCKET_TMPL "/tmp/replyto-%s-%x"

/* ----------------------------------------------------------------------- *
 * MAIN
 * ----------------------------------------------------------------------- */

#ifdef DISCARD_CMD_SOCKET

int main () {
    fprintf (stderr,
             "Communication support was disabled by compiler option!\n");
    exit (2);
}

#else

static const
struct option long_options [] = {
    {"help", 0, 0, 'h'},
    {"root", 1, 0, 'r'},
    {0, 0, 0, 0}
};
static const
char short_options [] = "r:h";
   
static char
*append (const char *p) {
    static char    *str ;
    static unsigned str_len ;

    if (str_len == 0)
        return str = strcpy
            ((char*)xrealloc (str, (str_len = strlen (p) + 1)), p);

    str = strcat ((char*)xrealloc (str, (str_len += strlen (p) + 1)), " ");
    return strcat (str, p);
}

static
void tmo_exit (int unused) {
    if (client) unlink (client);
    logger (LOG_ERR, "TIMEOUT", progname);
    exit (2);
}

static
void signal_exit (int unused) { 
    if (client) unlink (client);
    exit (2);
}

static
void usage (void) {
    fprintf
        (stderr,
         "Usage: %s [options] <cmd> [<args> ...]\n\n"
         "Options: -r, --root=<PREFIX> server runs in a chroot environment\n"
         "         -h, --help          print this message\n",
         progname);
    exit (2);
}

int main (int argc, char *argv[]) {
    int fd, un_size, n, c, option_inx ;
    struct sockaddr_un un ;
    char *p, *q, buf [1024], *root = "";
    
    /* progname ::= only the last path segment */
    if ((progname = strrchr (argv [0], '/')) == 0)
        progname = argv [0] ;
    else
        ++ progname ;

    while ((c = getopt_long (argc, argv, short_options, 
                             long_options, &option_inx)) != -1)
        switch (c) {
        case 'r':
            root = optarg ;
            continue ;
        case 'h':
        case '?':
            usage ();
        }

    if (argc <= optind)
        usage ();

    /* create reply socket */
    if ((fd = socket (PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        logger (LOG_ERR, "Cannot create command socket: %s",
                strerror (errno));
        exit (2);
    }

    /* assemble the reply command socket path */
    client = (char*) xmalloc
        (sizeof (REPLY_SOCKET_TMPL) + strlen (progname) + 30);
    sprintf (client, REPLY_SOCKET_TMPL, progname, getpid ());
    if (strlen (client) > UNIX_PATH_MAX-1) client [UNIX_PATH_MAX-1] = '\0' ;
  
    /* make an argument string: /reply/socket <cmd> <args> ... */
    p = append (client) ;
    for (n = optind; n < argc; n ++)
        p = append (argv [n]);

    /* prepend the /reply/socket path by the root prefix */
    q = (char*) xmalloc (strlen (root) + strlen (client) + 1);
    client = strcat (strcpy (q, root), client);

    /* make directory unless present */
    q = xstrdup (client); /* dirname changes the arg, so duplicate  */
    mkdir (dirname (q), 0700);
    free (q);

    /* prepare for clean up */
    signal (SIGINT,  signal_exit);
    signal (SIGHUP,  signal_exit);
    signal (SIGTERM, signal_exit);
    signal (SIGUSR1, signal_exit);
    signal (SIGUSR2, signal_exit);

    /* bind to the reply socket */
    unlink (client);
    un.sun_family = AF_UNIX ;
    strcpy (un.sun_path, client);
    un_size = sizeof (un.sun_family) + strlen (un.sun_path);
    if (bind (fd, (struct sockaddr*)&un, un_size) < 0) {
        logger (LOG_ERR, "Cannot bind command socket to %s: %s", 
                un.sun_path, strerror (errno));
        exit (2);
    }

    /* send the command buf to server */
    strncpy (un.sun_path, CTRL_REQUEST_PATH, UNIX_PATH_MAX);
    un.sun_path [UNIX_PATH_MAX-1] = '\0' ;
    un_size =
        offsetof (struct sockaddr_un, sun_path) +
        strlen (un.sun_path) + 1;
    if (sendto (fd, p, strlen (p), 0, (struct sockaddr*)&un, un_size) < 0) {
        logger (LOG_ERR, "Cannot send command via %s: %s",
                un.sun_path, strerror (errno));
        unlink (client);
        exit (2);
    }

    /* receive answer */
    signal (SIGALRM,   tmo_exit);
    alarm (20);
    n = recvfrom (fd, buf, sizeof (buf) -1, 0, 0, 0);
    alarm (0);
    if (n < 0) {
        logger (LOG_ERR, "Error reading from %s: ",
                un.sun_path, strerror (errno));
        unlink (client);
        exit (2) ;
    }
    buf [n] = '\0'; /* zero terminated - makes parsing easier */
    unlink (client);

    printf ("%s\n", buf);

    exit (buf [0] == '-' ? 1 : 0) ;
}

#endif /* DISCARD_CMD_SOCKET */

/* ------------------------------------------------------------------------ *
 * End
 * ------------------------------------------------------------------------ */

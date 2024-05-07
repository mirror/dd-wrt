/*
 *  $Id: libnet_socket.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_socket.c - raw sockets routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"

int
libnet_open_raw_sock(int prot)
{
    int fd;
    int one = 1;
#if (__svr4__)
    void *oneptr = &one;
#else
    int *oneptr = &one;
#endif  /* __svr4__ */

    fd = socket(AF_INET, SOCK_RAW, prot);
    if (fd == -1)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_CRITICAL,
            "libnet_open_raw_sock: SOCK_RAW allocation failed: %s\n",
            strerror(errno));
#endif
        return (-1);
    }
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, oneptr, sizeof(one)) == -1)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_CRITICAL,
                "libnet_open_raw_sock: set IP_HDRINCL failed: %s\n",
                strerror(errno));
#endif
        return (-1);
    }
#if (__linux__)
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, oneptr, sizeof(one)) == -1)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_CRITICAL,
                "libnet_open_raw_sock: set SO_BROADCAST failed: %s\n",
                strerror(errno));
#endif
        return (-1);
    }
#endif  /* __linux__ */
    return (fd);
}


int
libnet_close_raw_sock(int fd)
{
    return (close(fd));
}
/* EOF */

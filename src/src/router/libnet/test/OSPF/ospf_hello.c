/*
 *  $Id: ospf_hello.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  ospf_hello.c - OSPF Hello packet builder
 *
 *  Copyright (c) 1999 Andrew Reiter <areiter@bindview.com>
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
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

void	help(char *);

int
main(int argc, char **argv)
{
    int sock, warn, len;
    u_char *pack, *to, *from, *neighbor;
    u_long src, dst, addaid, addrid, nbr,  auth[2];

    if (argc != 4) 
	help(argv[0]);

    from = argv[1];
    to = argv[2];
    neighbor = argv[3];

    len = LIBNET_OSPF_H + LIBNET_HELLO_H + LIBNET_IP_H + LIBNET_AUTH_H;
    pack = (u_char *)malloc(len);

    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1) {
	perror("socket");
   	free(pack);
	exit(-1);
    }


    src = libnet_name_resolve(from, 0);
    dst = libnet_name_resolve(to, 0);
    nbr = libnet_name_resolve(neighbor, 0);

    addrid = 0x23696969;
    addaid = 0xc0ffee00;	/* GENERIC : FAKE : ETC */

    libnet_build_ip(LIBNET_OSPF_H + LIBNET_AUTH_H + LIBNET_HELLO_H, 0x00,
                101, IP_DF, 254, IPPROTO_OSPF, src, dst, NULL, 0, pack);

    auth[0] = 0;
    auth[1] = 0;

    libnet_build_ospf(LIBNET_HELLO_H + LIBNET_AUTH_H, LIBNET_OSPF_HELLO,
                addrid, addaid, LIBNET_OSPF_AUTH_NULL, NULL, 0, pack +
                LIBNET_IP_H); 
  
    LIBNET_OSPF_AUTHCPY(pack + LIBNET_IP_H + LIBNET_OSPF_H, auth);

    libnet_build_ospf_hello(0xffffffff, 2, 0x00, 0x00, 30, src, src, nbr,
                NULL, 0, pack + LIBNET_IP_H + LIBNET_OSPF_H + LIBNET_AUTH_H);

    libnet_do_checksum(pack, IPPROTO_OSPF, len);

    warn = libnet_write_ip(sock, pack, len);
    if (warn != len) {
  	printf("Error sending: %d bytes written : %s\n", warn, strerror(errno));
	free(pack);
	exit(warn);
    } else {
	printf("%d bytes written\n", warn); 
    }
    free(pack);
    return (0);
}

void
help(char *pname)
{
    printf("Usage: %s <source ip> <dest. ip> <neighbor>\n", pname);
    printf("[Use x.x.x.x format]\n");
    exit(0);
}

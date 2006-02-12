/*
 *  $Id: ospf_request.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  ospf_request.c - generic OSPF LSR packet builder
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

void 	help(char *);

int
main(int argc, char **argv)
{
    int warn, sock, len;
    u_char *pack;
    char *from, *to, *adrtr;
    u_long src, dest, advrouter, addaid, addrid;
    u_char auth[2]; 

    if (argc != 4)
	help(argv[0]);

    from = argv[1];
    to = argv[2];
    adrtr = argv[3]; 

    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1) {
	perror("socket");
	exit(-1);
    }

    src = libnet_name_resolve(from, 0);
    dest = libnet_name_resolve(to, 0);
    advrouter = libnet_name_resolve(adrtr, 0);

    addaid = 0xc0ffeeb0;
    addrid = 0xb00f00d0;

    len = LIBNET_IP_H + LIBNET_OSPF_H + LIBNET_AUTH_H + LIBNET_LSR_H;
    pack = (u_char *)malloc(len);
    
    libnet_build_ip(LIBNET_OSPF_H + LIBNET_AUTH_H + LIBNET_LSR_H, 0x00,
            (u_short)rand(), IP_DF, 0xff, IPPROTO_OSPF, src, dest, NULL,
            0, pack);

    memset(auth, 0, sizeof(auth));

    libnet_build_ospf(LIBNET_AUTH_H + LIBNET_LSR_H, LIBNET_OSPF_LSR,
            addrid, addaid, LIBNET_OSPF_AUTH_NULL, NULL, 0, pack +
            LIBNET_IP_H);

    LIBNET_OSPF_AUTHCPY(pack + LIBNET_IP_H + LIBNET_OSPF_H, auth);

    libnet_build_ospf_lsr(LIBNET_LS_TYPE_RTR, 0xffffffff, advrouter, NULL, 0, 
			pack + LIBNET_IP_H + LIBNET_OSPF_H + LIBNET_AUTH_H);

    libnet_do_checksum(pack, IPPROTO_OSPF, len);

    warn = libnet_write_ip(sock, pack, len); 
    if (warn == -1) {
	printf("Error writing packet to wire\n");
	free(pack);
    } else {
	printf("%d bytes written\n", warn);
    }

    free(pack);
    return (0);
}

void
help(char *pname)
{
    printf("Usage: %s <source ip> <dest. ip> <router ip>\n", pname);
    printf("[Use x.x.x.x format]\n");
    exit(0);
}

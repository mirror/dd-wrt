/*
 *  $Id: ospf_lsa.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  ospf_lsa.c - OSPF LSA packet builder
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
    int sock, warn, len;
    u_char *pack;
    char *from, *to;
    u_long src, dst, addrid, addaid, auth[2];

    if (argc != 3) 
	help(argv[0]);

    len = LIBNET_IP_H + LIBNET_OSPF_H + LIBNET_AUTH_H + LIBNET_LSA_H +
                LIBNET_LS_NET_LEN;
    pack = (u_char *)malloc(len);

    from = argv[1];
    to = argv[2];

    src = libnet_name_resolve(from, 0);
    dst = libnet_name_resolve(to, 0);

    addrid = 0xff00ff00;
    addaid = 0xd00dd00d;

    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1) {
	perror("socket");
	free(pack);
	exit(sock);
    }

    libnet_build_ip(LIBNET_OSPF_H + LIBNET_AUTH_H + LIBNET_LSA_H +
                LIBNET_LS_NET_LEN,
			0x00, 
			(u_short)rand(), 
			IP_DF,
			0xfe,
			IPPROTO_OSPF,
			src,
			dst,
			NULL,
			0,
			pack);

    libnet_build_ospf(LIBNET_AUTH_H + LIBNET_LSA_H + LIBNET_LS_NET_LEN,
			LIBNET_OSPF_LSA,
			addrid,
			addaid,
			LIBNET_OSPF_AUTH_NULL,
			NULL,
			0,
			pack + LIBNET_IP_H);

    memset(auth, 0, sizeof(auth));
    LIBNET_OSPF_AUTHCPY(pack + LIBNET_OSPF_H + LIBNET_IP_H, auth);

    libnet_build_ospf_lsa(40, 
			  0x00, 
			  LIBNET_LS_TYPE_NET,
			  addrid,
			  src, 
			  0xf0f0f00f,
			  LIBNET_LS_NET_LEN,
			  NULL,
			  0,
			  pack + LIBNET_AUTH_H + LIBNET_OSPF_H + LIBNET_IP_H);

    libnet_build_ospf_lsa_net(0xffffff00, 
				0xc0ffee00, 
				NULL,
				0,
				pack + LIBNET_LSA_H + LIBNET_AUTH_H +
                                LIBNET_OSPF_H + LIBNET_IP_H);

    libnet_do_checksum(pack, IPPROTO_OSPF, len);
    libnet_do_checksum(pack + LIBNET_IP_H + LIBNET_OSPF_H + LIBNET_AUTH_H,
                    IPPROTO_OSPF_LSA, LIBNET_LS_NET_LEN + LIBNET_LSA_H);


    warn = libnet_write_ip(sock, pack, len);
    if (warn == -1) {
	printf("Error writing packet to the wire\n");
	free(pack);
	exit(warn);
    }

    printf("%d bytes written\n", warn);
    free(pack);
    return (0);
}

void
help(char *pname)
{
    printf("Usage: %s <source ip> <dest. ip>\n", pname);
    printf("[Use x.x.x.x format]\n");
    exit(0);
}

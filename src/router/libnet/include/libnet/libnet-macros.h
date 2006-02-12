/*
 *  $Id: libnet-macros.h,v 1.1 2004/04/27 01:30:54 dyang Exp $
 *
 *  libnet-macros.h - Network routine library macro header file
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

#ifndef __LIBNET_MACROS_H
#define __LIBNET_MACROS_H

/*
 *  for libnet_hostlookup
 */
#define LIBNET_DONT_RESOLVE 0
#define LIBNET_RESOLVE      1

/*
 *  prand constants
 */
#define LIBNET_PR2          0
#define LIBNET_PR8          1
#define LIBNET_PR16         2
#define LIBNET_PRu16        3
#define LIBNET_PR32         4
#define LIBNET_PRu32        5
#define LIBNET_PRAND_MAX    0xffffffff

/*
 *  Concession to legacy naming scheme
 */
#define PR2         LIBNET_PR2
#define PR8         LIBNET_PR8
#define PR16        LIBNET_PR16
#define PRu16       LIBNET_PRu16
#define PR32        LIBNET_PR32
#define PRu32       LIBNET_PRu32
#define PRAND_MAX   LIBNET_PRAND_MAX

/*
 *  Misc packet sizes for malloc
 */
#define LIBNET_PACKET       LIBNET_IP_H + LIBNET_TCP_H  /* IP, UDP or TCP */
#define LIBNET_OPTS         0x34            /* options! options! options! */
#define LIBNET_MAX_PACKET   0xffff          /* as big as we can get */


/*
 *  Error handling constants.
 */
#define LIBNET_ERR_WARNING  1
#define LIBNET_ERR_CRITICAL 2
#define LIBNET_ERR_FATAL    3

/*
 *  Concession to legacy naming scheme
 */
#define LN_ERR_WARNING  LIBNET_ERR_WARNING
#define LN_ERR_CRITICAL LIBNET_ERR_CRITICAL
#define LN_ERR_FATAL    LIBNET_ERR_FATAL


#define LIBNET_ERRBUF_SIZE  256


/*
 *  Some BSD variants have this endianess problem.
 */
#if (LIBNET_BSD_BYTE_SWAP)
#define FIX(n)      ntohs(n)
#define UNFIX(n)    htons(n)
#else
#define FIX(n)      (n)
#define UNFIX(n)    (n)
#endif

/*
 *  Arena stuff
 */
#define LIBNET_GET_ARENA_SIZE(a)               (a.size)
#define LIBNET_GET_ARENA_REMAINING_BYTES(a)    (a.size - a.current)


/*
 *  Checksum stuff
 */
#define LIBNET_CKSUM_CARRY(x) \
    (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))


/*
 *  OSPF stuff
 */
#define LIBNET_OSPF_AUTHCPY(x,y)  memcpy((u_char *)x, (u_char *)y, sizeof(y))
#define LIBNET_OSPF_CKSUMBUF(x,y) memcpy((u_char *)x, (u_char *)y, sizeof(y))  

/*
 *  Not all systems have IFF_LOOPBACK
 *  Used by if_addr.c
 */
#ifdef IFF_LOOPBACK
#define LIBNET_ISLOOPBACK(p) ((p)->ifr_flags & IFF_LOOPBACK)
#else
#define LIBNET_ISLOOPBACK(p) (strcmp((p)->ifr_name, "lo0") == 0)
#endif

#define LIBNET_PRINT_ETH_ADDR(e) \
{ \
    int i = 0; \
    for (i = 0; i < 6; i++) \
    { \
        printf("%x", e.ether_addr_octet[i]); \
        if (i != 5) \
        { \
            printf(":"); \
        } \
    } \
    printf("\n"); \
}

#endif  /* __LIBNET_MACROS_H */

/* EOF */

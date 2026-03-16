/*
 * Copyright (c) 2012
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

/*
 * The uh_dport/uh_sport values are only available on Linux when
 * _BSD_SOURCE is set and _XOPEN_SOURCE/_XOPEN_SOURCE_EXTENDED
 * is not set.
 *
 * Functions to access these fields are placed in this file,
 * making this the only file where _XOPEN_SOURCE/_XOPEN_SOURCE_EXTENDED
 * are not set.
 */

#if HAVE_LINUX_BUGS
#undef _XOPEN_SOURCE
#undef _XOPEN_SOURCE_EXTENDED
#endif /* HAVE_LINUX_BUGS */

#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

static const char rcsid[] =
"$Id: udp_port.c,v 1.3 2012/08/08 13:17:05 michaels Exp $";

in_port_t *udphdr_uh_dport(struct udphdr *udp);
in_port_t *udphdr_uh_sport(struct udphdr *udp);
uint16_t *udphdr_uh_ulen(struct udphdr *udp);
uint16_t *udphdr_uh_sum(struct udphdr *udp);

in_port_t *
udphdr_uh_dport(struct udphdr *udp)
{

   return &udp->uh_dport;
}

in_port_t *
udphdr_uh_sport(struct udphdr *udp)
{

   return &udp->uh_sport;
}

uint16_t *
udphdr_uh_ulen(struct udphdr *udp)
{

   return &udp->uh_ulen;
}

uint16_t *
udphdr_uh_sum(struct udphdr *udp)
{

   return &udp->uh_sum;
}

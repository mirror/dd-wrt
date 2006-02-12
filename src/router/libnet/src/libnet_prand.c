/*
 *  $Id: libnet_prand.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_prand.c - pseudo-random number generation
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
libnet_seed_prand()
{
    struct timeval seed;

    if (gettimeofday(&seed, NULL) == -1)
    {
        perror("seed_rand: cannot gettimeofday");
        return (-1);
    }

    /*
     *  More entropy then just seeding with time(2).
     */
    srandom((unsigned)(seed.tv_sec ^ seed.tv_usec));
    return (1);
}


u_long
libnet_get_prand(int mod)
{
    u_long n;

    n = random();

    switch (mod)
    {
        case LIBNET_PR2:
            return (n % 0x2);           /* 0 - 1 */
        case LIBNET_PR8:
            return (n % 0xff);          /* 0 - 255 */
        case LIBNET_PR16:
            return (n % 0x7fff);        /* 0 - 32767 */
        case LIBNET_PRu16:
            return (n % 0xffff);        /* 0 - 65535 */
        case LIBNET_PR32:
            return (n % 0x7fffffff);    /* 0 - 2147483647 */
        case LIBNET_PRu32:
            return (n);                 /* 0 - 4294967295 */
    }
    return (0);                         /* NOTTREACHED */
}

/* EOF */

/*
 *  $Id: prand_trials.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  prand_trials.c - psuedorandom number generation
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
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

int
main(int argc, char **argv)
{

    int i, j;

    printf("Psuedorandom number generation\n");
    printf("For each trial, 1000 numbers will be generated\n");
    libnet_seed_prand();

    printf("\n\nPress return for trial 1 (0 - 1)\n\n");
    getc(stdin);
    for (i = 1000; i; i--)
    {
        printf("%ld ", libnet_get_prand(LIBNET_PR2));
    }

    printf("\n\nPress return for trial 2 (0 - 255)\n\n");
    getc(stdin);
    for (i = 1000; i; i--)
    {
        printf("%3ld ", libnet_get_prand(LIBNET_PR8));
    }

    printf("\n\nPress return for trial 3 (0 - 32767)\n\n");
    getc(stdin);
    for (j = 13, i = 1000; i; i--, j--)
    {
        if (!j)
        {
            printf("\n");
            j = 13;
        }
        printf("%5ld ", libnet_get_prand(LIBNET_PR16));
    }

    printf("\n\nPress return for trial 4 (0 - 65535)\n\n");
    getc(stdin);
    for (j = 13, i = 1000; i; i--, j--)
    {
        if (!j)
        {
            printf("\n");
            j = 13;
        }
        printf("%5ld ", libnet_get_prand(LIBNET_PRu16));
    }

    printf("\n\nPress return for trial 5 (0 - 2147483647)\n\n");
    getc(stdin);
    for (j = 7, i = 1000; i; i--, j--)
    {
        if (!j)
        {
            printf("\n");
            j = 7;
        }
        printf("%10ld ", libnet_get_prand(LIBNET_PR32));
    }

    printf("\n\nPress return for trial 6 (0 - 4294967295)\n\n");
    getc(stdin);
    for (j = 7, i = 1000; i; i--, j--)
    {
        if (!j)
        {
            printf("\n");
            j = 7;
        }
        printf("%10ld ", libnet_get_prand(LIBNET_PRu32));
    }

    printf("\nCompleted\n");
    return (EXIT_SUCCESS);
}

/* EOF */

/*
 *  $Id: libnet_error.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_error.c - error handling code
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
#include <stdarg.h>

char *
ll_strerror(int errnum)
{
#ifdef HAVE_STRERROR
    return (strerror(errnum));
#else
    extern int sys_nerr;
    static char ebuf[20];

    if ((u_int)errnum < sys_nerr)
    {
        return ((char *)sys_errlist[errnum]);
    }
    sprintf(ebuf, "Unknown error: %d", errnum);
    return (ebuf);
#endif
}


void
libnet_error(int severity, char *msg, ...)
{
    va_list ap;
    char buf[BUFSIZ];

    va_start(ap, msg);
    vsnprintf(buf, sizeof(buf) - 1, msg, ap);

    switch (severity)
    {
        case LIBNET_ERR_WARNING:
            fprintf(stderr, "Warning: ");
            break;
        case LIBNET_ERR_CRITICAL:
            fprintf(stderr, "Critical: ");
            break;
        case LIBNET_ERR_FATAL:
            fprintf(stderr, "Fatal: ");
            break;
    }
    fprintf(stderr, "%s", buf);
    va_end(ap);

    if (severity == LIBNET_ERR_FATAL)
    {
        exit(EXIT_FAILURE);
    }
}

/* EOF */

/*
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998
 *	The Regents of the University of California.  All rights reserved.
 *
 * Some portions Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Some portions Copyright (C) 2010-2013 Sourcefire, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _SFBPF_INT_H
#define _SFBPF_INT_H

#ifdef HAVE_VISIBILITY
#  define DAQ_SO_PUBLIC  __attribute__ ((visibility("default")))
#  define DAQ_SO_PRIVATE __attribute__ ((visibility("hidden")))
#else
#  define DAQ_SO_PUBLIC
#  define DAQ_SO_PRIVATE
#endif

#include "sfbpf.h"
#include "sf-redefines.h"

#define PCAP_ERRBUF_SIZE 256

#ifndef strlcpy
#define strlcpy(x, y, z) \
    (strncpy((x), (y), (z)), \
     ((z) <= 0 ? 0 : ((x)[(z) - 1] = '\0')), \
     strlen((y)))
#endif

int yylex(void);
int sfbpf_strcasecmp(const char *s1, const char *s2);

#define SFBPF_NETMASK_UNKNOWN        0xffffffff

#endif /* _SFBPF_INT_H */

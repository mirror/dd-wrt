
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004-2009, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _COMMON_AUTOBUF_H
#define _COMMON_AUTOBUF_H

struct autobuf;

#include "defs.h"
#include <stdarg.h>
#include <time.h>

#define ROUND_UP_TO_POWER_OF_2(val, pow2) (((val) + (pow2) - 1) & ~((pow2) - 1))

#define AUTOBUFCHUNK	4096
struct autobuf {
  int size;
  int len;
  char *buf;
};

int abuf_init (struct autobuf * autobuf, int initial_size);
void abuf_free (struct autobuf * autobuf);
int abuf_vappendf (struct autobuf *autobuf, const char *fmt, va_list ap) __attribute__ ((format(printf, 2, 0)));
int abuf_appendf (struct autobuf * autobuf, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
int abuf_puts (struct autobuf * autobuf, const char *s);
int abuf_strftime (struct autobuf * autobuf, const char *format, const struct tm * tm);
int abuf_memcpy (struct autobuf * autobuf, const void *p, const unsigned int len);
int abuf_memcpy_prefix (struct autobuf *autobuf, const void *p, const unsigned int len);
int abuf_pull (struct autobuf * autobuf, int len);
#endif

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */

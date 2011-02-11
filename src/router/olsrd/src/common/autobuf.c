
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

#include "common/autobuf.h"
#include "defs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


static int autobuf_enlarge(struct autobuf *autobuf, int new_size);


int
abuf_init(struct autobuf *autobuf, int initial_size)
{
  autobuf->len = 0;
  if (initial_size <= 0) {
    autobuf->size = 0;
    autobuf->buf = NULL;
    return 0;
  }
  autobuf->size = ROUND_UP_TO_POWER_OF_2(initial_size, AUTOBUFCHUNK);
  autobuf->buf = calloc(autobuf->size, 1);
  if (autobuf->buf == NULL) {
    autobuf->size = 0;
    return -1;
  }
  *autobuf->buf = '\0';
  return 0;
}

void
abuf_free(struct autobuf *autobuf)
{
  free(autobuf->buf);
  autobuf->buf = NULL;
  autobuf->len = 0;
  autobuf->size = 0;
}

static int
autobuf_enlarge(struct autobuf *autobuf, int new_size)
{
  new_size++;
  if (new_size > autobuf->size) {
    char *p;
    int roundUpSize = ROUND_UP_TO_POWER_OF_2(new_size, AUTOBUFCHUNK);
    p = realloc(autobuf->buf, roundUpSize);
    if (p == NULL) {
#ifdef WIN32
      WSASetLastError(ENOMEM);
#else
      errno = ENOMEM;
#endif
      return -1;
    }
    autobuf->buf = p;

    memset(&autobuf->buf[autobuf->size], 0, roundUpSize - autobuf->size);
    autobuf->size = roundUpSize;
  }
  return 0;
}

int
abuf_vappendf(struct autobuf *autobuf, const char *format, va_list ap)
{
  int rc;
  int min_size;
  va_list ap2;
  va_copy(ap2, ap);
  rc = vsnprintf(autobuf->buf + autobuf->len, autobuf->size - autobuf->len, format, ap);
  va_end(ap);
  min_size = autobuf->len + rc;
  if (min_size >= autobuf->size) {
    if (autobuf_enlarge(autobuf, min_size) < 0) {
      autobuf->buf[autobuf->len] = '\0';
      return -1;
    }
    vsnprintf(autobuf->buf + autobuf->len, autobuf->size - autobuf->len, format, ap2);
  }
  va_end(ap2);
  autobuf->len = min_size;
  return 0;
}

int
abuf_appendf(struct autobuf *autobuf, const char *fmt, ...)
{
  int rv;
  va_list ap;
  va_start(ap, fmt);
  rv = abuf_vappendf(autobuf, fmt, ap);
  va_end(ap);
  return rv;
}

int
abuf_puts(struct autobuf *autobuf, const char *s)
{
  int len; 

  if (NULL == s) return 0;
  len = strlen(s);
  if (autobuf_enlarge(autobuf, autobuf->len + len + 1) < 0) {
    return -1;
  }
  strcpy(autobuf->buf + autobuf->len, s);
  autobuf->len += len;
  return len;
}

int
abuf_strftime(struct autobuf *autobuf, const char *format, const struct tm *tm)
{
  int rc = strftime(autobuf->buf + autobuf->len, autobuf->size - autobuf->len, format, tm);
  if (rc == 0) {
    /* we had an error! Probably the buffer too small. So we add some bytes. */
    if (autobuf_enlarge(autobuf, autobuf->size + AUTOBUFCHUNK) < 0) {
      autobuf->buf[autobuf->len] = '\0';
      return -1;
    }
    rc = strftime(autobuf->buf + autobuf->len, autobuf->size - autobuf->len, format, tm);
  }
  autobuf->len += rc;
  return rc;
}

int
abuf_memcpy(struct autobuf *autobuf, const void *p, const unsigned int len)
{
  if (autobuf_enlarge(autobuf, autobuf->len + len) < 0) {
    return -1;
  }
  memcpy(autobuf->buf + autobuf->len, p, len);
  autobuf->len += len;
  return len;
}

int
abuf_memcpy_prefix(struct autobuf *autobuf, const void *p, const unsigned int len)
{
  if (autobuf_enlarge(autobuf, autobuf->len + len) < 0) {
    return -1;
  }
  memmove(&autobuf->buf[len], autobuf->buf, autobuf->len);
  memcpy(autobuf->buf, p, len);
  autobuf->len += len;
  return len;
}

int
abuf_pull(struct autobuf * autobuf, int len) {
  char *p;
  size_t newsize;

  if (len != autobuf->len) {
    memmove(autobuf->buf, &autobuf->buf[len], autobuf->len - len);
  }
  autobuf->len -= len;

  newsize = ROUND_UP_TO_POWER_OF_2(autobuf->len + 1, AUTOBUFCHUNK);
  p = realloc(autobuf->buf, newsize);
  if (p == NULL) {
#ifdef WIN32
    WSASetLastError(ENOMEM);
#else
    errno = ENOMEM;
#endif
    return -1;
  }
  autobuf->buf = p;
  autobuf->size = newsize;
  return 0;
}
/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */


/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * Copyright (c) 2004, Thomas Lopatic (thomas@olsr.org)
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

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static unsigned int debugMask = 0;

void
error(const char *form, ...)
{
  va_list args;

  va_start(args, form);
  vfprintf(stderr, form, args);
  va_end(args);
}

void
debug(int facility, const char *form, ...)
{
  va_list args;

  if ((debugMask & facility) == 0)
    return;

  va_start(args, form);
  vfprintf(stderr, form, args);
  va_end(args);
}

char *
strdupAdd(const char *string, int add)
{
  char *res = allocMem(strlen(string) + 1 + add);

  strcpy(res, string);
  return res;
}

char *
myStrdup(const char *string)
{
  return strdupAdd(string, 0);
}

void
chomp(char *line, int len)
{
  while (len-- > 0) {
    if (line[len] != 10 && line[len] != 13)
      break;

    line[len] = 0;
  }
}

char *
intToString(char *buff, unsigned int val)
{
  int i;

  buff[9] = 0;

  for (i = 8; i >= 0; i--) {
    buff[i] = (char)(val % 10 + '0');

    val /= 10;

    if (val == 0)
      break;
  }

  return buff + i;
}

int
stringToInt(unsigned int *val, const char *buff)
{
  *val = 0;

  while (*buff != 0) {
    if (*buff < '0' || *buff > '9')
      return -1;

    else
      *val = *val * 10 + *buff - '0';

    buff++;
  }

  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

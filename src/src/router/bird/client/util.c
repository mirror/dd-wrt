/*
 *	BIRD Client -- Utility Functions
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "nest/bird.h"
#include "lib/string.h"
#include "client/client.h"

/* Client versions of logging functions */

static void
vlog(char *msg, va_list args)
{
  char buf[1024];

  if (bvsnprintf(buf, sizeof(buf)-1, msg, args) < 0)
    bsprintf(buf + sizeof(buf) - 100, " ... <too long>");
  fputs(buf, stderr);
  fputc('\n', stderr);
}

void
bug(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  cleanup();
  fputs("Internal error: ", stderr);
  vlog(msg, args);
  vfprintf(stderr, msg, args);
  exit(1);
}

void
die(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  cleanup();
  vlog(msg, args);
  exit(1);
}

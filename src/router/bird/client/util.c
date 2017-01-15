/*
 *	BIRD Client -- Utility Functions
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */
#ifdef NEED_PRINF
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "nest/bird.h"
#include "lib/string.h"
#include "client/client.h"

/* Client versions of logging functions */

static void
vlog(const char *msg, va_list args)
{
  char buf[1024];

  int n = vsnprintf(buf, sizeof(buf), msg, args);
  if (n < 0)
    snprintf(buf, sizeof(buf), "???");
  else if (n >= (int) sizeof(buf))
    snprintf(buf + sizeof(buf) - 100, 100, " ... <too long>");
  fputs(buf, stderr);
  fputc('\n', stderr);
}

void
bug(const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  cleanup();
  fputs("Internal error: ", stderr);
  vlog(msg, args);
  vfprintf(stderr, msg, args);
  va_end(args);
  exit(1);
}

void
die(const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  cleanup();
  vlog(msg, args);
  va_end(args);
  exit(1);
}
#endif
/* ISC license. */

#include <string.h>

#include "mdevd-internal.h"

char const *mdevd_uevent_getvar (struct uevent_s const *event, char const *var)
{
  size_t varlen = strlen(var) ;
  unsigned short i = 1 ;
  for (; i < event->varn ; i++)
    if (!strncmp(var, event->buf + event->vars[i], varlen) && event->buf[event->vars[i] + varlen] == '=')
      break ;
  return i < event->varn ? event->buf + event->vars[i] + varlen + 1 : 0 ;
}


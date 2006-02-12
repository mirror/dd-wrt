/*
 *	BIRD -- Password handling
 *
 *	(c) 1999 Pavel Machek <pavel@ucw.cz>
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "nest/password.h"
#include "lib/string.h"

struct password_item *last_password_item = NULL;

struct password_item *
password_find(list *l)
{
  struct password_item *pi;

  if (l)
  {
    WALK_LIST(pi, *l)
    {
      if ((pi->genfrom < now) && (pi->gento > now))
        return pi;
    }
  }
  return NULL;
}

void password_cpy(char *dst, char *src, int size)
{
  bzero(dst, size);
  memcpy(dst, src, (strlen(src) < (unsigned) size ? strlen(src) : (unsigned) size));
}


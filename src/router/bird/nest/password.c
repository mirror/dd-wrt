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
#include "lib/mac.h"

struct password_item *last_password_item = NULL;

struct password_item *
password_find(list *l, int first_fit)
{
  struct password_item *pi;
  struct password_item *pf = NULL;

  if (l)
  {
    WALK_LIST(pi, *l)
    {
      if ((pi->genfrom < now_real) && (pi->gento > now_real))
      {
	if (first_fit)
	  return pi;

	if (!pf || pf->genfrom < pi->genfrom)
	  pf = pi;
      }
    }
  }
  return pf;
}

struct password_item *
password_find_by_id(list *l, uint id)
{
  struct password_item *pi;

  if (!l)
    return NULL;

  WALK_LIST(pi, *l)
    if ((pi->id == id) && (pi->accfrom <= now_real) && (now_real < pi->accto))
      return pi;

  return NULL;
}

struct password_item *
password_find_by_value(list *l, char *pass, uint size)
{
  struct password_item *pi;

  if (!l)
    return NULL;

  WALK_LIST(pi, *l)
    if (password_verify(pi, pass, size) && (pi->accfrom <= now_real) && (now_real < pi->accto))
      return pi;

  return NULL;
}

uint
max_mac_length(list *l)
{
  struct password_item *pi;
  uint val = 0;

  if (!l)
    return 0;

  WALK_LIST(pi, *l)
    val = MAX(val, mac_type_length(pi->alg));

  return val;
}

/* BGP Extended Communities Attribute
 * Copyright (C) 2000 Kunihiro Ishiguro <kunihiro@zebra.org>
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include "hash.h"
#include "memory.h"
#include "prefix.h"
#include "command.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_ecommunity.h"

/* For parse Extended Community attribute tupple. */
struct ecommunity_as
{
  as_t as;
  u_int32_t val;
};

struct ecommunity_ip
{
  struct in_addr ip;
  u_int16_t val;
};

/* Hash of community attribute. */
struct Hash *ecomhash;

struct ecommunity *
ecommunity_new ()
{
  struct ecommunity *new;

  new = XMALLOC (MTYPE_ECOMMUNITY, sizeof (struct ecommunity));
  memset (new, 0, sizeof (struct ecommunity));
  return new;
}

void
ecommunity_free (struct ecommunity *ecom)
{
  if (ecom->val)
    XFREE (MTYPE_ECOMMUNITY_VAL, ecom->val);
  XFREE (MTYPE_ECOMMUNITY, ecom);
}

struct ecommunity *
ecommunity_parse (char *pnt, u_short length)
{
  struct ecommunity tmp;
  struct ecommunity *find;
  struct ecommunity *new;

  if (length % 8)
    return NULL;

  tmp.size = length / 8;
  tmp.val = pnt;

  find = (struct ecommunity *) hash_search (ecomhash, &tmp);
  if (find)
    {
      find->refcnt++;
      return find;
    }

  new = ecommunity_new ();

  new->refcnt = 1;
  new->size = length / 8;
  new->val = XMALLOC (MTYPE_ECOMMUNITY_VAL, length);
  memcpy (new->val, pnt, length);

  hash_push (ecomhash, new);

  return new;
}

struct ecommunity *
ecommunity_dup (struct ecommunity *ecom)
{
  struct ecommunity *new;

  new = XMALLOC (MTYPE_ECOMMUNITY, sizeof (struct ecommunity));
  memset (new, 0, sizeof (struct ecommunity));
  new->size = ecom->size;
  if (new->size)
    {
      new->val = XMALLOC (MTYPE_ECOMMUNITY_VAL, ecom->size * 8);
      memcpy (new->val, ecom->val, ecom->size * 8);
    }
  else
    new->val = NULL;
  return new;
}

struct ecommunity *
ecommunity_merge (struct ecommunity *ecom1, struct ecommunity *ecom2)
{
  if (ecom1->val)
    ecom1->val = XREALLOC (MTYPE_ECOMMUNITY_VAL, ecom1->val, 
			   (ecom1->size + ecom2->size) * 8);
  else
    ecom1->val = XMALLOC (MTYPE_ECOMMUNITY_VAL,
			  (ecom1->size + ecom2->size) * 8);

  memcpy (ecom1->val + (ecom1->size * 8), ecom2->val, ecom2->size * 8);
  ecom1->size += ecom2->size;

  return ecom1;
}

struct ecommunity *
ecommunity_intern (struct ecommunity *ecom)
{
  struct ecommunity *find;

  assert (ecom->refcnt == 0);

  find = (struct ecommunity *) hash_search (ecomhash, ecom);
  if (find)
    {
      ecommunity_free (ecom);
      find->refcnt++;
      return find;
    }

  ecom->refcnt = 1;
  hash_push (ecomhash, ecom);
  return ecom;
}

void
ecommunity_unintern (struct ecommunity *ecom)
{
  if (ecom->refcnt)
    ecom->refcnt--;

  if (ecom->refcnt == 0)
    {
      struct ecommunity *ret;
  
      ret = (struct ecommunity *) hash_pull (ecomhash, ecom);
      assert (ret != NULL);

      ecommunity_free (ecom);
    }
}

unsigned int
ecommunity_hash_make (struct ecommunity *ecom)
{
  int c;
  unsigned int key;
  unsigned char *pnt;

  key = 0;
  pnt = (unsigned char *)ecom->val;
  
  for (c = 0; c < ecom->size * 8; c++)
    key += pnt[c];

  return key %= HASHTABSIZE;
}

int
ecommunity_cmp (struct ecommunity *ecom1, struct ecommunity *ecom2)
{
  if (ecom1->size == ecom2->size)
    if (memcmp (ecom1->val, ecom2->val, ecom1->size * 8) == 0)
      return 1;
  return 0;
}

/* Initialize Extended Comminities related hash. */
void
ecommunity_init ()
{
  ecomhash = hash_new (HASHTABSIZE);
  ecomhash->hash_key = ecommunity_hash_make;
  ecomhash->hash_cmp = ecommunity_cmp;
}

/* Extended Communities token enum. */
enum ecommunity_token
{
  ecommunity_token_as,
  ecommunity_token_ip,
  ecommunity_token_unknown
};

/* Get next Extended Communities token from the string. */
u_char *
ecommunity_gettoken (char *buf, enum ecommunity_token *token,
		     struct ecommunity_as *eas, struct ecommunity_ip *eip)
{
  int dot = 0;
  int separator = 0;
  u_int32_t community_low = 0;
  u_int32_t community_high = 0;
  char *p = buf;
  struct in_addr ip;
  char ipstr[INET_ADDRSTRLEN + 1];
  int ret;

  /* Skip white space. */
  while (isspace ((int) *p))
    p++;

  /* Check the end of the line. */
  if (*p == '\0')
    return NULL;

  /* Community value. */
  if (isdigit ((int) *p)) 
    {
      while (isdigit ((int) *p) || *p == ':' || *p == '.') 
	{
	  if (*p == ':') 
	    {
	      if (separator)
		{
		  *token = ecommunity_token_unknown;
		  return p;
		}
	      else
		{
		  separator = 1;

		  if (dot)
		    {
		      if ((p - buf) > INET_ADDRSTRLEN)
			{
			  *token = ecommunity_token_unknown;
			  return p;
			}
		      memset (ipstr, 0, INET_ADDRSTRLEN + 1);
		      memcpy (ipstr, buf, p - buf);
		      ret = inet_aton (ipstr, &ip);
		      if (ret == 0)
			{
			  *token = ecommunity_token_unknown;
			  return p;
			}
		    }
		  else
		    community_high = community_low;

		  community_low = 0;
		}
	    }
	  else if (*p == '.')
	    {
	      if (separator)
		{
		  *token = ecommunity_token_unknown;
		  return p;
		}

	      dot++;

	      if (dot > 4)
		{
		  *token = ecommunity_token_unknown;
		  return p;
		}
	    }
	  else
	    {
	      community_low *= 10;
	      community_low += (*p - '0');
	    }
	  p++;
	}

      if (dot)
	{
	  eip->ip = ip;
	  eip->val = community_low;
	  *token = ecommunity_token_ip;
	}
      else
	{
	  eas->as = community_high;
	  eas->val = community_low;
	  *token = ecommunity_token_as;
	}
      return p;
    }

  *token = ecommunity_token_unknown;
  return p;
}

void
ecommunity_add_val (struct ecommunity *ecom, int type, 
		    enum ecommunity_token token, 
		    struct ecommunity_as *eas, struct ecommunity_ip *eip)
{
  u_char *pnt;

  ecom->size++;

  if (ecom->val)
    ecom->val = XREALLOC (MTYPE_ECOMMUNITY_VAL, ecom->val, ecom_length (ecom));
  else
    ecom->val = XMALLOC (MTYPE_ECOMMUNITY_VAL, ecom_length (ecom));

  pnt = ecom->val + ((ecom->size - 1) * 8);

  if (token == ecommunity_token_as)
    *pnt = ECOMMUNITY_ENCODE_AS;
  else if (token == ecommunity_token_ip)
    *pnt = ECOMMUNITY_ENCODE_IP;
  pnt++;

  if (type == ECOMMUNITY_ROUTE_TARGET)
    *pnt = ECOMMUNITY_ROUTE_TARGET;
  else if (type == ECOMMUNITY_SITE_ORIGIN)
    *pnt = ECOMMUNITY_SITE_ORIGIN;
  pnt++;

  if (token == ecommunity_token_as)
    {
      *pnt++ = (u_char) (eas->as >> 8);
      *pnt++ = (u_char) (eas->as);
      *pnt++ = (u_char) (eas->val >> 24);
      *pnt++ = (u_char) (eas->val >> 16);
      *pnt++ = (u_char) (eas->val >> 8);
      *pnt++ = (u_char) (eas->val);
    }
  else if (token == ecommunity_token_ip)
    {
      memcpy (pnt, &eip->ip, 4);
      pnt += 4;
      *pnt++ = (u_char) (eip->val >> 8);
      *pnt++ = (u_char) (eip->val);
    }
}

struct ecommunity *
ecommunity_str2com (int type, char *str)
{
  struct ecommunity *ecom = NULL;
  struct ecommunity_as eas;
  struct ecommunity_ip eip;
  enum ecommunity_token token;

  while ((str = ecommunity_gettoken (str, &token, &eas, &eip)))
    {
      switch (token)
	{
	case ecommunity_token_as:
	case ecommunity_token_ip:
	  if (ecom == NULL)
	    ecom = ecommunity_new ();
	  ecommunity_add_val (ecom, type, token, &eas, &eip);
	  break;
	case ecommunity_token_unknown:
	default:
	  if (ecom)
	    ecommunity_free (ecom);
	  return NULL;
	  break;
	}
    }
  return ecom;
}

void
ecommunity_print (struct ecommunity *ecom)
{
  int i;
  u_char *pnt;
  struct ecommunity_as eas;
  struct ecommunity_ip eip;
  int encode = 0;
  int type = 0;

  for (i = 0; i < ecom->size; i++)
    {
      pnt = ecom->val + (i * 8);

      /* High-order octet of type. */
      if (*pnt == ECOMMUNITY_ENCODE_AS)
	encode = ECOMMUNITY_ENCODE_AS;
      else if (*pnt == ECOMMUNITY_ENCODE_IP)
	encode = ECOMMUNITY_ENCODE_IP;
      pnt++;
      
      /* Low-order octet of type. */
      if (*pnt == ECOMMUNITY_ROUTE_TARGET)
	{
	  if (type != ECOMMUNITY_ROUTE_TARGET)
	    printf (" RT:");
	  type = ECOMMUNITY_ROUTE_TARGET;
	}
      else if (*pnt == ECOMMUNITY_SITE_ORIGIN)
	{
	  if (type != ECOMMUNITY_SITE_ORIGIN)
	    printf (" SOO:");
	  type = ECOMMUNITY_SITE_ORIGIN;
	}
      pnt++;

      if (encode == ECOMMUNITY_ENCODE_AS)
	{
	  eas.as = (*pnt++ << 8);
	  eas.as |= (*pnt++);

	  eas.val = (*pnt++ << 24);
	  eas.val |= (*pnt++ << 16);
	  eas.val |= (*pnt++ << 8);
	  eas.val |= (*pnt++);

	  printf ("%d:%d", eas.as, eas.val);
	}
      else if (encode == ECOMMUNITY_ENCODE_IP)
	{
	  memcpy (&eip.ip, pnt, 4);
	  pnt += 4;
	  eip.val = (*pnt++ << 8);
	  eip.val |= (*pnt++);

	  printf ("%s:%d", inet_ntoa (eip.ip), eip.val);
	}
    }

  printf ("\n");
}

void
ecommunity_vty_out (struct vty *vty, struct ecommunity *ecom)
{
  int i;
  u_char *pnt;
  struct ecommunity_as eas;
  struct ecommunity_ip eip;
  int encode = 0;
  int type = 0;

  for (i = 0; i < ecom->size; i++)
    {
      pnt = ecom->val + (i * 8);

      /* High-order octet of type. */
      if (*pnt == ECOMMUNITY_ENCODE_AS)
	encode = ECOMMUNITY_ENCODE_AS;
      else if (*pnt == ECOMMUNITY_ENCODE_IP)
	encode = ECOMMUNITY_ENCODE_IP;
      pnt++;
      
      /* Low-order octet of type. */
      if (*pnt == ECOMMUNITY_ROUTE_TARGET)
	{
	  if (type != ECOMMUNITY_ROUTE_TARGET)
	    vty_out (vty, " RT:");
	  type = ECOMMUNITY_ROUTE_TARGET;
	}
      else if (*pnt == ECOMMUNITY_SITE_ORIGIN)
	{
	  if (type != ECOMMUNITY_SITE_ORIGIN)
	    vty_out (vty, " SOO:");
	  type = ECOMMUNITY_SITE_ORIGIN;
	}
      pnt++;

      if (encode == ECOMMUNITY_ENCODE_AS)
	{
	  eas.as = (*pnt++ << 8);
	  eas.as |= (*pnt++);

	  eas.val = (*pnt++ << 24);
	  eas.val |= (*pnt++ << 16);
	  eas.val |= (*pnt++ << 8);
	  eas.val |= (*pnt++);

	  vty_out (vty, "%d:%d", eas.as, eas.val);
	}
      else if (encode == ECOMMUNITY_ENCODE_IP)
	{
	  memcpy (&eip.ip, pnt, 4);
	  pnt += 4;
	  eip.val = (*pnt++ << 8);
	  eip.val |= (*pnt++);

	  vty_out (vty, "%s:%d", inet_ntoa (eip.ip), eip.val);
	}
    }
}

#ifdef TEST
void
ecommunity_test ()
{
  struct ecommunity *com1;
  struct ecommunity *com2;

  com1 = ecommunity_str2com (ECOMMUNITY_ROUTE_TARGET, "65535:1");
  com2 = ecommunity_str2com (ECOMMUNITY_SITE_ORIGIN, "1.1.1.1:1");

  ecommunity_print (com1);
  ecommunity_print (com2);

  /*
  if (com2 == NULL)
    printf ("errror\n");
  else
    ecommunity_print (com2);
  */

  /* printf ("%s\n", ecommunity_print (com1)); */
  {
    int i;

    u_char *pnt = com2->val;

    for (i = 0; i < 8; i ++)
      {
	printf ("[%d] %d\n", i, pnt[i]);
      }
  }
}
#endif /* TEST */

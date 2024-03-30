/* This file is part of pound
 * Copyright (C) 2020-2024 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "pound.h"
#include "json.h"

#define json_2nrealloc mem2nrealloc
void (*json_memabrt) (void) = NULL;

char const *
json_strerror (int ec)
{
  static char *json_errstr[] = {
    [JSON_E_NOERR] = "No error",
    [JSON_E_NOMEM] = "Not enough memory",
    [JSON_E_BADTOK] = "Unrecognized token",
    [JSON_E_BADDELIM] = "Bad delimiter",
    [JSON_E_BADSTRING] = "Malformed string"
  };
  if (ec < 0 || ec >= sizeof (json_errstr) / sizeof (json_errstr[0]))
    return "Unknown error";
  return json_errstr[ec];
}

static void
json_format_writez (struct json_format *fmt, char const *str)
{
  size_t len = strlen (str);
  fmt->write (fmt->data, str, len);
}

static void
json_format_writec (struct json_format *fmt, char c)
{
  fmt->write (fmt->data, &c, 1);
}

static void
json_format_indent (struct json_format *fmt, size_t level)
{
  level *= fmt->indent;
  while (level--)
    json_format_writec (fmt, ' ');
}

static void
json_format_delim (struct json_format *fmt, size_t level)
{
  json_format_writec (fmt, ',');
  if (fmt->indent)
    {
      json_format_writec (fmt, '\n');
      json_format_indent (fmt, level);
    }
  else
    json_format_writec (fmt, ' ');
}

/* General-purpose */
struct json_value *
json_value_create (int type)
{
  struct json_value *obj = calloc (1, sizeof (*obj));
  if (obj)
    obj->type = type;
  else if (json_memabrt)
    json_memabrt ();
  return obj;
}

typedef void (*json_format_fun) (struct json_format *, struct json_value *,
				 size_t);
typedef void (*json_free_fun) (struct json_value *);
typedef int (*json_copy_fun) (struct json_value *, struct json_value **);

static int json_copy_generic (struct json_value *, struct json_value **);
static int json_copy_string (struct json_value *, struct json_value **);
static int json_copy_array (struct json_value *, struct json_value **);
static int json_copy_object (struct json_value *, struct json_value **);

static void json_format_null (struct json_format *, struct json_value *,
			      size_t level);
static void json_format_bool (struct json_format *, struct json_value *,
			      size_t level);
static void json_format_number (struct json_format *, struct json_value *,
				size_t level);
static void json_format_integer (struct json_format *, struct json_value *,
				 size_t level);
static void json_format_string (struct json_format *, struct json_value *,
				size_t level);
static void json_format_array (struct json_format *, struct json_value *,
			       size_t level);
static void json_format_object (struct json_format *, struct json_value *,
				size_t level);


static void json_free_string (struct json_value *);
static void json_free_array (struct json_value *);
static void json_free_object (struct json_value *);

static struct json_value_meth
{
  json_format_fun format_fun;
  json_free_fun free_fun;
  json_copy_fun copy_fun;
} json_value_meth[] =
{
  [json_null] = { json_format_null, NULL, json_copy_generic },
  [json_bool] = { json_format_bool, NULL, json_copy_generic },
  [json_number] = { json_format_number, NULL, json_copy_generic },
  [json_integer] = { json_format_integer, NULL, json_copy_generic },
  [json_string] = { json_format_string, json_free_string, json_copy_string },
  [json_array] = { json_format_array, json_free_array, json_copy_array },
  [json_object] = { json_format_object, json_free_object, json_copy_object }
};

static inline int
is_valid_type (int type)
{
  return type >= 0
    && type < sizeof (json_value_meth) / sizeof (json_value_meth[0]);
}

static inline int
is_valid_value (struct json_value *obj)
{
  return obj && is_valid_type (obj->type);
}

void
json_value_free (struct json_value *obj)
{
  if (is_valid_value (obj))
    {
      if (json_value_meth[obj->type].free_fun)
	json_value_meth[obj->type].free_fun (obj);
      free (obj);
    }
}

int
json_value_copy (struct json_value *val, struct json_value **new_val)
{
  if (is_valid_value (val))
    return json_value_meth[val->type].copy_fun (val, new_val);
  else
    {
      errno = EINVAL;
      return -1;
    }
}

int
json_value_format (struct json_value *obj, struct json_format *fmt,
		   size_t level)
{
  ++level;
  if (!obj)
    {
      json_format_null (fmt, obj, level);
      return 0;
    }
  if (is_valid_value (obj))
    {
      json_value_meth[obj->type].format_fun (fmt, obj, level);
      return 0;
    }
  errno = EINVAL;
  return -1;
}

static int
json_copy_generic (struct json_value *val, struct json_value **ret_val)
{
  struct json_value *p;

  if (!val)
    {
      p = json_new_null ();
      if (!p)
	return -1;
    }
  else if (is_valid_type (val->type))
    {
      p = json_value_create (val->type);
      if (p)
	memcpy (&p->v, &val->v, sizeof (p->v));
    }
  else
    {
      errno = EINVAL;
    }

  if (!p)
    return -1;

  *ret_val = p;
  return 0;
}

/* Null value */
struct json_value *
json_new_null (void)
{
  return json_value_create (json_null);
}

static void
json_format_null (struct json_format *fmt, struct json_value *val,
		  size_t level)
{
  json_format_writez (fmt, "null");
}

/* Bool value */
struct json_value *
json_new_bool (int b)
{
  struct json_value *j = json_value_create (json_bool);
  if (j)
    j->v.b = b;
  return j;
}

static void
json_format_bool (struct json_format *fmt, struct json_value *val,
		  size_t level)
{
  json_format_writez (fmt, val->v.b ? "true" : "false");
}

/* Number value */
struct json_value *
json_new_number (double n)
{
  struct json_value *j = json_value_create (json_number);
  if (j)
    j->v.n = n;
  return j;
}

static void
json_format_number (struct json_format *fmt, struct json_value *val,
		    size_t level)
{
  char buffer[128];		//FIXME
  if (fmt->precision == -1)
    snprintf (buffer, sizeof buffer, "%e", val->v.n);
  else
    snprintf (buffer, sizeof buffer, "%.*f", fmt->precision, val->v.n);
  json_format_writez (fmt, buffer);
}

struct json_value *
json_new_integer (long n)
{
  struct json_value *j = json_value_create (json_integer);
  if (j)
    j->v.n = n;
  return j;
}

static void
json_format_integer (struct json_format *fmt, struct json_value *val,
		     size_t level)
{
  char buffer[128];		//FIXME
  snprintf (buffer, sizeof buffer, "%.*f", 0, val->v.n);
  json_format_writez (fmt, buffer);
}

/* String value */
struct json_value *
json_new_string (char const *str)
{
  struct json_value *j = json_value_create (json_string);
  if (j)
    {
      j->v.s = strdup (str);
      if (!j->v.s)
	{
	  free (j);
	  j = NULL;
	}
    }
  return j;
}

static void
json_free_string (struct json_value *val)
{
  free (val->v.s);
}

static int
json_copy_string (struct json_value *val, struct json_value **ret_val)
{
  struct json_value *newval = json_value_create (json_string);
  if (!newval)
    return -1;
  if ((newval->v.s = strdup (val->v.s)) == NULL)
    {
      free (newval);
      return -1;
    }
  *ret_val = newval;
  return 0;
}

enum { ESCAPE, UNESCAPE };

static int
escape (char c, char *o, int un)
{
  static char transtab[] = "\\\\\"\"b\bf\fn\nr\rt\t//";
  char *p;

  for (p = transtab; p[2]; p += 2)
    {
      if (p[!un] == c)
	{
	  *o = p[un];
	  return 0;
	}
    }
  return -1;
}

static void
json_format_escape (struct json_format *fmt, char const *s)
{
  json_format_writec (fmt, '"');
  for (; *s; s++)
    {
      char c;
      if (!escape (*s, &c, ESCAPE))
	{
	  json_format_writec (fmt, '\\');
	  json_format_writec (fmt, c);
	}
      else
	json_format_writec (fmt, *s);
    }
  json_format_writec (fmt, '"');
}

static void
json_format_string (struct json_format *fmt, struct json_value *val,
		    size_t level)
{
  json_format_escape (fmt, val->v.s);
}

/* Array value */
struct json_value *
json_new_array (void)
{
  struct json_value *j = json_value_create (json_array);
  if (j)
    {
      j->v.a = malloc (sizeof (*j->v.a));
      if (j->v.a)
	{
	  j->v.a->oc = 0;
	  j->v.a->on = 0;
	  j->v.a->ov = NULL;
	}
      else if (json_memabrt)
	json_memabrt ();
      else
	{
	  free (j);
	  j = NULL;
	}
    }
  return j;
}

static void
json_free_array (struct json_value *val)
{
  size_t i;

  for (i = 0; i < val->v.a->oc; i++)
    json_value_free (val->v.a->ov[i]);
  free (val->v.a->ov);
  free (val->v.a);
}

static int
json_copy_array (struct json_value *val, struct json_value **ret_val)
{
  struct json_value *newval;
  size_t i;

  newval = json_new_array ();
  if (!newval)
    return -1;
  if (!(newval->v.a->ov = calloc (val->v.a->oc, sizeof (val->v.a->ov[0]))))
    {
      free (newval);
      if (json_memabrt)
	json_memabrt ();
      return -1;
    }
  newval->v.a->oc = newval->v.a->on = val->v.a->oc;
  for (i = 0; i < val->v.a->oc; i++)
    {
      if (json_value_copy (val->v.a->ov[i], &newval->v.a->ov[i]))
	{
	  newval->v.a->oc = i;
	  json_value_free (newval);
	  return -1;
	}
    }
  *ret_val = newval;
  return 0;
}

static int
json_array_expand (struct json_value *jv)
{
  size_t i;
  size_t n = jv->v.a->on;
  struct json_value **p = json_2nrealloc (jv->v.a->ov,
					  &jv->v.a->on,
					  sizeof (jv->v.a->ov[0]));
  if (!p)
    {
      if (json_memabrt)
	json_memabrt ();
      return -1;
    }
  jv->v.a->ov = p;
  for (i = jv->v.a->on; i < n; i++)
    jv->v.a->ov[i] = NULL;
  return 0;
}

int
json_array_insert (struct json_value *jv, size_t idx, struct json_value *v)
{
  if (jv == NULL || jv->type != json_array || v == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if (jv->v.a->oc <= idx)
    {
      while (jv->v.a->on <= idx)
	{
	  if (json_array_expand (jv))
	    return -1;
	}
      jv->v.a->oc = idx + 1;
    }
  jv->v.a->ov[idx] = v;
  return 0;
}

int
json_array_append (struct json_value *jv, struct json_value *v)
{
  if (jv == NULL || jv->type != json_array || v == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  return json_array_insert (jv, jv->v.a->oc, v);
}

int
json_array_set (struct json_value *jv, size_t idx, struct json_value *v)
{
  if (jv == NULL || jv->type != json_array || v == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  if (idx >= json_array_length (jv))
    {
      errno = ENOENT;
      return -1;
    }
  jv->v.a->ov[idx] = v;
  return 0;
}

int
json_array_get (struct json_value *jv, size_t idx, struct json_value **retval)
{
  if (jv == NULL || jv->type != json_array || retval == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  if (idx >= json_array_length (jv))
    {
      errno = ENOENT;
      return -1;
    }
  *retval = jv->v.a->ov[idx];
  return 0;
}

static void
json_format_array (struct json_format *fmt, struct json_value *obj,
		   size_t level)
{
  size_t i;

  json_format_writec (fmt, '[');
  if (obj->v.a->oc)
    {
      if (fmt->indent)
	json_format_writec (fmt, '\n');
      for (i = 0; i < obj->v.a->oc; i++)
	{
	  (i ? json_format_delim : json_format_indent) (fmt, level);
	  json_value_format (obj->v.a->ov[i], fmt, level);
	}
      if (fmt->indent)
	{
	  json_format_writec (fmt, '\n');
	  json_format_indent (fmt, level - 1);
	}
    }
  json_format_writec (fmt, ']');
}

/* Object value */
struct json_value *
json_new_object (void)
{
  struct json_value *jv = json_value_create (json_object);
  if (jv && !(jv->v.o = calloc (1, sizeof (*jv->v.o))))
    {
      if (json_memabrt)
	json_memabrt ();
      free (jv);
      return NULL;
    }
  return jv;
}

static void
json_free_object (struct json_value *val)
{
  struct json_pair *p;

  p = SLIST_FIRST (&val->v.o->pair_head);
  while (p)
    {
      struct json_pair *next = SLIST_NEXT (p, next);
      free (p->k);
      json_value_free (p->v);
      free (p);
      p = next;
    }
  free (val->v.o);
}

static int
json_copy_object (struct json_value *val, struct json_value **ret_val)
{
  struct json_value *newval;
  struct json_pair *p;

  if (!(newval = json_new_object ()))
    return -1;
  SLIST_FOREACH (p, &val->v.o->pair_head, next)
    {
      struct json_value *elt = NULL;

      if (json_value_copy (p->v, &elt) || json_object_set (newval, p->k, elt))
	{
	  json_value_free (elt);
	  json_value_free (newval);
	  return -1;
	}
    }
  *ret_val = newval;
  return 0;
}

static int
json_object_lookup_or_install (struct json_object *obj, char const *name,
			       int install, struct json_pair **retval)
{
  struct json_pair *l, *m;
  size_t i, n, count;

  if (obj->pair_count == 0)
    l = NULL;
  else
    {
      l = SLIST_FIRST (&obj->pair_head);

      if (strcmp (l->k, name) > 0)
	{
	  l = NULL;
	}
      else if (strcmp (SLIST_LAST (&obj->pair_head)->k, name) < 0)
	{
	  l = SLIST_LAST (&obj->pair_head);
	}
      else
	{
	  count = obj->pair_count;
	  while (count)
	    {
	      int c;

	      n = count / 2;
	      for (m = l, i = 0; i < n; m = SLIST_NEXT (m, next), i++)
		;

	      c = strcmp (m->k, name);
	      if (c == 0)
		{
		  *retval = m;
		  return 0;
		}
	      else if (n == 0)
		{
		  break;
		}
	      else if (c < 0)
		{
		  l = m;
		  count -= n;
		}
	      else
		{
		  count = n;
		}
	    }
	}
    }

  if (!install)
    {
      errno = ENOENT;
      return -1;
    }

  m = malloc (sizeof (*m));
  if (!m)
    {
      if (json_memabrt)
	json_memabrt ();
      return -1;
    }
  m->next = NULL;
  m->k = strdup (name);
  if (!m->k)
    {
      if (json_memabrt)
	json_memabrt ();
      free (m);
      return -1;
    }
  m->v = NULL;

  if (!l)
    {
      SLIST_INSERT_HEAD (m, &obj->pair_head, next);
    }
  else
    {
      struct json_pair *p;

      while ((p = SLIST_NEXT (l, next)) != NULL && strcmp (p->k, name) < 0)
	l = p;

      SLIST_INSERT_AFTER (&obj->pair_head, l, m, next);
    }
  obj->pair_count++;

  *retval = m;
  return 0;
}

int
json_object_set (struct json_value *obj, char const *name,
		 struct json_value *val)
{
  struct json_pair *p;
  int res;

  if (obj == NULL || obj->type != json_object || val == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  res = json_object_lookup_or_install (obj->v.o, name, 1, &p);
  if (res)
    return res;

  json_value_free (p->v);
  p->v = val;
  return 0;
}

int
json_object_get (struct json_value *obj, char const *name,
		 struct json_value **retval)
{
  struct json_pair *p;
  int res;

  if (obj == NULL || obj->type != json_object || name == NULL || retval == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  res = json_object_lookup_or_install (obj->v.o, name, 0, &p);
  if (res)
    return res;
  *retval = p->v;
  return 0;
}

static void
json_format_object (struct json_format *fmt, struct json_value *obj,
		    size_t level)
{
  struct json_object *op = obj->v.o;
  struct json_pair *p;
  json_format_writec (fmt, '{');
  if (op->pair_count)
    {
      if (fmt->indent)
	json_format_writec (fmt, '\n');
      SLIST_FOREACH (p, &op->pair_head, next)
	{
	  (p == SLIST_FIRST (&op->pair_head)
	   ? json_format_indent : json_format_delim) (fmt, level);
	  json_format_escape (fmt, p->k);
	  json_format_writec (fmt, ':');
	  if (fmt->indent)
	    json_format_writec (fmt, ' ');
	  json_value_format (p->v, fmt, level);
	}
      if (fmt->indent)
	{
	  json_format_writec (fmt, '\n');
	  json_format_indent (fmt, level - 1);
	}
    }
  json_format_writec (fmt, '}');
}

/* Removes from OBJ all pairs for which the predicate function PRED
   returns 1.
 */
int
json_object_filter (struct json_value *obj,
		    int (*pred) (char const *, struct json_value *, void *),
		    void *data)
{
  struct json_object *op;
  struct json_pair *p, *prev;

  if (obj == NULL || obj->type != json_object || pred == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  op = obj->v.o;
  if (SLIST_EMPTY (&op->pair_head))
    return 0;

  prev = NULL;
  for (p = SLIST_FIRST (&op->pair_head); p;)
    {
      struct json_pair *next = p->next;
      if (pred (p->k, p->v, data))
	{
	  if (prev)
	    SLIST_NEXT (prev, next) = next;
	  else
	    SLIST_FIRST (&op->pair_head) = next;
	  if (!next)
	    SLIST_LAST (&op->pair_head) = prev;
	  free (p->k);
	  json_value_free (p->v);
	}
      else
	prev = p;
      p = next;
    }
  return 0;
}

/* Parser */
#define ISSPACE(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')
#define ISHEX(c) (strchr("0123456789abcdefABCDEF", c) != NULL)
#define SKIPWS(s) while (*(s) && ISSPACE(*(s))) (s)++;

struct j_context
{
  struct j_context *next;
  struct json_value *obj;
  char *key;
};

static inline int
j_context_type (struct j_context *ctx)
{
  return ctx ? ctx->obj->type : json_null;
}

static int
j_context_push (struct j_context **ctx, int type)
{
  struct j_context *cur = malloc (sizeof (*cur));

  if (!cur)
    {
      if (json_memabrt)
	json_memabrt ();
      return JSON_E_NOMEM;
    }

  switch (type)
    {
    case json_array:
      cur->obj = json_new_array ();
      break;
    case json_object:
      cur->obj = json_new_object ();
      break;
    default:
      abort ();
    }
  if (!cur->obj)
    {
      free (cur);
      return JSON_E_NOMEM;
    }
  cur->next = *ctx;
  cur->key = NULL;
  *ctx = cur;
  return JSON_E_NOERR;
}

static struct json_value *
j_context_pop (struct j_context **ctx)
{
  struct json_value *val;
  struct j_context *next;

  if (!*ctx)
    return NULL;
  val = (*ctx)->obj;
  free ((*ctx)->key);
  next = (*ctx)->next;
  free (*ctx);
  *ctx = next;

  return val;
}

static int
utf8_wctomb (char const *u, char r[6])
{
  unsigned int wc = strtoul (u, NULL, 16);
  int count;

  if (wc < 0x80)
    count = 1;
  else if (wc < 0x800)
    count = 2;
  else if (wc < 0x10000)
    count = 3;
  else if (wc < 0x200000)
    count = 4;
  else if (wc < 0x4000000)
    count = 5;
  else if (wc <= 0x7fffffff)
    count = 6;
  else
    return -1;

  switch (count)
    {
      /* Note: code falls through cases! */
    case 6:
      r[5] = 0x80 | (wc & 0x3f);
      wc = wc >> 6;
      wc |= 0x4000000;
    case 5:
      r[4] = 0x80 | (wc & 0x3f);
      wc = wc >> 6;
      wc |= 0x200000;
    case 4:
      r[3] = 0x80 | (wc & 0x3f);
      wc = wc >> 6;
      wc |= 0x10000;
    case 3:
      r[2] = 0x80 | (wc & 0x3f);
      wc = wc >> 6;
      wc |= 0x800;
    case 2:
      r[1] = 0x80 | (wc & 0x3f);
      wc = wc >> 6;
      wc |= 0xc0;
    case 1:
      r[0] = wc;
    }
  return count;
}

static int
j_get_text (char const *input, char **retval, char const **endp)
{
  size_t len;
  char const *p;
  char *q;
  char *str;

  len = 1;
  ++input;
  for (p = input; *p != '"'; p++)
    {
      if (*p == '\\')
	{
	  if (ISHEX (p[1]) && ISHEX (p[2]) && ISHEX (p[3]) && ISHEX (p[4]))
	    {
	      char r[6];
	      int n = utf8_wctomb (p, r);
	      if (n == -1)
		{
		  *endp = p + 1;
		  return JSON_E_BADSTRING;
		}
	      p += 5;
	      len += n;
	      continue;
	    }
	  else
	    {
	      p++;
	    }
	}
      if (*p == 0)
	return JSON_E_BADSTRING;
      len++;
    }

  str = malloc (len);
  if (!str)
    {
      if (json_memabrt)
	json_memabrt ();
      return JSON_E_NOMEM;
    }

  p = input;
  q = str;
  while (*p != '"')
    {
      if (*p == '\\')
	{
	  p++;
	  if (escape (*p, q, UNESCAPE) == 0)
	    {
	      q++;
	      p++;
	    }
	  else if (ISHEX (p[0])
		   && ISHEX (p[1]) && ISHEX (p[2]) && ISHEX (p[3]))
	    {
	      char r[6];
	      int n = utf8_wctomb (p, r);
	      memcpy (q, r, n);
	      p += 4;
	      q += n;
	    }
	  else
	    {
	      *q++ = *p++;
	    }
	}
      else
	{
	  *q++ = *p++;
	}
    }
  *q++ = 0;
  *endp = p + 1;
  *retval = str;
  return 0;
}

int
json_parse_string (char const *input, struct json_value **retval, char **endp)
{
  struct j_context *ctx = NULL;
  struct json_value *val;
  int ecode = JSON_E_NOERR;
  char *str;

  while (1)
    {
      val = NULL;

      SKIPWS (input);
      switch (*input)
	{
	case '[':
	  j_context_push (&ctx, json_array);
	  ++input;
	  continue;

	case ']':
	  if (j_context_type (ctx) == json_array)
	    {
	      val = j_context_pop (&ctx);
	    }
	  else
	    {
	      ecode = JSON_E_BADTOK;
	      goto err;
	    }
	  ++input;
	  break;

	case '{':
	  j_context_push (&ctx, json_object);
	  ++input;
	  continue;

	case '}':
	  if (j_context_type (ctx) == json_object)
	    {
	      val = j_context_pop (&ctx);
	    }
	  else
	    {
	      ecode = JSON_E_BADTOK;
	      goto err;
	    }
	  ++input;
	  break;

	case '"':
	  ecode = j_get_text (input, &str, &input);
	  if (ecode != JSON_E_NOERR)
	    goto err;
	  if (j_context_type (ctx) == json_object && !ctx->key)
	    {
	      ctx->key = str;

	      SKIPWS (input);
	      if (*input == ':')
		{
		  ++input;
		  continue;
		}

	      ecode = JSON_E_BADDELIM;
	      goto err;
	    }
	  else
	    {
	      val = json_value_create (json_string);
	      if (!val)
		goto err;
	      val->v.s = str;
	    }
	  break;

	case '-':
	case '+':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  {
	    char *p;
	    double d = strtod (input, &p);
	    if ((input[0] == '-' && d == -HUGE_VAL) || d == HUGE_VAL)
	      {
		ecode = JSON_E_BADTOK;
		goto err;
	      }
	    val = json_new_number (d);
	    if (!val)
	      {
		ecode = JSON_E_NOMEM;
		goto err;
	      }
	    input = p;
	    break;
	  }

	default:
	  if (strncmp (input, "null", 4) == 0)
	    {
	      val = json_new_null ();
	      input += 4;
	    }
	  else if (strncmp (input, "true", 4) == 0)
	    {
	      val = json_new_bool (1);
	      input += 4;
	    }
	  else if (strncmp (input, "false", 5) == 0)
	    {
	      val = json_new_bool (0);
	      input += 5;
	    }
	  else
	    {
	      ecode = JSON_E_BADTOK;
	      goto err;
	    }
	  if (!val)
	    {
	      ecode = JSON_E_NOMEM;
	      goto err;
	    }
	}

      if (val)
	{
	  if (ctx)
	    {
	      int rc;

	      switch (j_context_type (ctx))
		{
		case json_array:
		  rc = json_array_append (ctx->obj, val);
		  break;

		case json_object:
		  if (!ctx->key)
		    {
		      json_value_free (val);
		      ecode = JSON_E_BADTOK;
		      goto err;
		    }
		  rc = json_object_set (ctx->obj, ctx->key, val);
		  free (ctx->key);
		  ctx->key = NULL;
		  break;

		default:
		  abort ();
		}
	      if (rc)
		{
		  json_value_free (val);
		  ecode = JSON_E_NOMEM;
		  goto err;
		}
	    }
	  else
	    {
	      *retval = val;
	      *endp = (char *) input;
	      return 0;
	    }
	}

      SKIPWS (input);
      if (*input == ',')
	{
	  ++input;
	}
      else if (*input == ']' || *input == '}')
	{
	  continue;
	}
      else
	{
	  ecode = JSON_E_BADDELIM;
	  goto err;
	}
    }

err:
  while (j_context_pop (&ctx))
    ;
  *endp = (char *) input;
  return ecode;
}

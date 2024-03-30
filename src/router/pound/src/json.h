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
#ifndef POUND_JSON_H
# define POUND_JSON_H

#include <stdlib.h>
#include "list.h"

enum json_value_type
{
  json_null,
  json_bool,
  json_number,
  json_integer,
  json_string,
  json_array,
  json_object
};

struct json_value;

struct json_array
{
  size_t oc;
  struct json_value **ov;
  size_t on;
};

struct json_object
{
  SLIST_HEAD (,json_pair) pair_head;
  size_t pair_count;
};

struct json_value
{
  enum json_value_type type;
  union
  {
    int b;			/* json_bool */
    double n;			/* json_number, json_integer */
    char *s;			/* json_string */
    struct json_array *a;	/* json_array */
    struct json_object *o;	/* json_object */
  } v;
};

struct json_pair
{
  char *k;
  struct json_value *v;
  SLIST_ENTRY (json_pair) next;
};

struct json_format
{
  size_t indent;
  int precision;
  void (*write) (void *, char const *, size_t);
  void *data;
};

int json_value_format (struct json_value *obj, struct json_format *fmt,
		       size_t level);
void json_value_free (struct json_value *);
int json_value_copy (struct json_value *val, struct json_value **new_val);

char const *json_strerror (int ec);

void *json_2nrealloc (void *p, size_t * pn, size_t s);

struct json_value *json_new_null (void);
struct json_value *json_new_bool (int b);
struct json_value *json_new_number (double n);
struct json_value *json_new_integer (long n);
struct json_value *json_new_string (char const *str);

struct json_value *json_new_object (void);
int json_object_set (struct json_value *obj, char const *name,
		     struct json_value *val);
int json_object_get (struct json_value *obj, char const *name,
		     struct json_value **retval);
int json_object_filter (struct json_value *obj,
			int (*pred) (char const *, struct json_value *,
				     void *), void *data);

struct json_value *json_new_array (void);
static inline size_t
json_array_length (struct json_value *j)
{
  return j->v.a->oc;
}

int json_array_insert (struct json_value *j, size_t idx,
		       struct json_value *v);
int json_array_append (struct json_value *j, struct json_value *v);
int json_array_set (struct json_value *j, size_t idx, struct json_value *v);
int json_array_get (struct json_value *j, size_t idx,
		    struct json_value **retval);


enum
{
  JSON_E_NOERR,
  JSON_E_NOMEM,
  JSON_E_BADTOK,
  JSON_E_BADDELIM,
  JSON_E_BADSTRING
};

int json_parse_string (char const *input, struct json_value **retval,
		       char **endp);

extern void (*json_memabrt) (void);

#endif

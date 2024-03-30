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

#include "pound.h"

void *
mem2nrealloc (void *p, size_t *pn, size_t s)
{
  size_t n = *pn;
  char *newp;

  if (!p)
    {
      if (!n)
	{
	  /* The approximate size to use for initial small allocation
	     requests, when the invoking code  specifies an old size of zero.
	     64 bytes is the largest "small" request for the
	     GNU C library malloc.  */
	  enum { DEFAULT_MXFAST = 64 };

	  n = DEFAULT_MXFAST / s;
	  n += !n;
	}
    }
  else
    {
      /* Set N = ceil (1.5 * N) so that progress is made if N == 1.
	 Check for overflow, so that N * S stays in size_t range.
	 The check is slightly conservative, but an exact check isn't
	 worth the trouble.  */
      if ((size_t) -1 / 3 * 2 / s <= n)
	{
	  errno = ENOMEM;
	  return NULL;
	}
      n += (n + 1) / 2;
    }

  newp = realloc (p, n * s);
  if (!newp)
    return NULL;
  *pn = n;
  return newp;
}

void *
xmalloc (size_t s)
{
  void *p = malloc (s);
  if (p == NULL)
    xnomem ();
  return p;
}

void *
xcalloc (size_t nmemb, size_t size)
{
  void *p = calloc (nmemb, size);
  if (!p)
    xnomem ();
  return p;
}

void *
xrealloc (void *p, size_t s)
{
  if ((p = realloc (p, s)) == NULL)
    xnomem ();
  return p;
}

void *
x2nrealloc (void *p, size_t *pn, size_t s)
{
  if ((p = mem2nrealloc (p, pn, s)) == NULL)
    xnomem ();
  return p;
}

char *
xstrdup (char const *s)
{
  char *p = strdup (s);
  if (!p)
    xnomem ();
  return p;
}

char *
xstrndup (const char *s, size_t n)
{
  char *p = strndup (s, n);
  if (!p)
    xnomem ();
  return p;
}

void
stringbuf_init (struct stringbuf *sb, void (*nomem) (void))
{
  memset (sb, 0, sizeof (*sb));
  sb->nomem = nomem;
}

void
stringbuf_reset (struct stringbuf *sb)
{
  sb->len = 0;
}

void
stringbuf_free (struct stringbuf *sb)
{
  free (sb->base);
}

static int
stringbuf_expand (struct stringbuf *sb, size_t len)
{
  while (sb->len + len > sb->size)
    {
      char *p = mem2nrealloc (sb->base, &sb->size, 1);
      if (p == NULL)
	{
	  if (sb->nomem)
	    sb->nomem ();
	  sb->err = 1;
	  return -1;
	}
      sb->base = p;
    }
  sb->err = 0;
  return 0;
}

int
stringbuf_truncate (struct stringbuf *sb, size_t len)
{
  if (len > sb->size)
    return stringbuf_expand (sb, len - sb->size);
  sb->len = len;
  return 0;
}

int
stringbuf_add_char (struct stringbuf *sb, int c)
{
  if (stringbuf_expand (sb, 1))
    return -1;
  sb->base[sb->len++] = c;
  return 0;
}

int
stringbuf_add (struct stringbuf *sb, char const *str, size_t len)
{
  if (stringbuf_expand (sb, len))
    return -1;
  memcpy (sb->base + sb->len, str, len);
  sb->len += len;
  return 0;
}

int
stringbuf_add_string (struct stringbuf *sb, char const *str)
{
  return stringbuf_add (sb, str, strlen (str));
}

char *
stringbuf_set (struct stringbuf *sb, int c, size_t n)
{
  size_t start = sb->len;

  if (stringbuf_expand (sb, n))
    return NULL;
  memset (sb->base + sb->len, c, n);
  sb->len += n;
  return sb->base + start;
}

char *
stringbuf_finish (struct stringbuf *sb)
{
  if (stringbuf_err (sb) || stringbuf_add_char (sb, 0))
    return NULL;
  return sb->base;
}

int
stringbuf_vprintf (struct stringbuf *sb, char const *fmt, va_list ap)
{
  for (;;)
    {
      size_t bufsize = sb->size - sb->len;
      ssize_t n;
      va_list aq;

      va_copy (aq, ap);
      n = vsnprintf (sb->base + sb->len, bufsize, fmt, aq);
      va_end (aq);

      if (n < 0 || n >= bufsize || !memchr (sb->base + sb->len, '\0', n + 1))
	{
	  char *p = mem2nrealloc (sb->base, &sb->size, 1);
	  if (p == NULL)
	    {
	      if (sb->nomem)
		sb->nomem ();
	      sb->err = 1;
	      return -1;
	    }
	  sb->base = p;
	}
      else
	{
	  sb->len += n;
	  break;
	}
    }
  return 0;
}

int
stringbuf_printf (struct stringbuf *sb, char const *fmt, ...)
{
  va_list ap;
  int rc;

  va_start (ap, fmt);
  rc = stringbuf_vprintf (sb, fmt, ap);
  va_end (ap);
  return rc;
}

int
stringbuf_strftime (struct stringbuf *sb, char const *fmt, const struct tm *tm)
{
  for (;;)
    {
      size_t bufsize = sb->size - sb->len;
      size_t n;

      n = strftime (sb->base + sb->len, bufsize, fmt, tm);
      if (n > 0)
	{
	  sb->len += n;
	  break;
	}
      else
	{
	  char *p = mem2nrealloc (sb->base, &sb->size, 1);
	  if (p == NULL)
	    {
	      if (sb->nomem)
		sb->nomem ();
	      sb->err = 1;
	      return -1;
	    }
	  sb->base = p;
	}
    }
  return 0;
}

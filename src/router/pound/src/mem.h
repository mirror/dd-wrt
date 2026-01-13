/* This file is part of pound
 * Copyright (C) 2020-2025 Sergey Poznyakoff
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

#include <stdarg.h>

#define ATTR_RETURNS_NONNULL __attribute__ ((__returns_nonnull__))

void *mem2nrealloc (void *p, size_t *pn, size_t s);
void xnomem (void);
void *xmalloc (size_t s) ATTR_RETURNS_NONNULL;
void *xcalloc (size_t nmemb, size_t size) ATTR_RETURNS_NONNULL;
#define xzalloc(s) xcalloc(1, s)
#define XZALLOC(v) (v = xzalloc (sizeof ((v)[0])))

void *xrealloc (void *p, size_t s) ATTR_RETURNS_NONNULL;
void *x2nrealloc (void *p, size_t *pn, size_t s) ATTR_RETURNS_NONNULL;
char *xstrdup (char const *s) ATTR_RETURNS_NONNULL;
char *xstrndup (const char *s, size_t n) ATTR_RETURNS_NONNULL;

struct stringbuf
{
  char *base;                     /* Buffer storage. */
  size_t size;                    /* Size of buf. */
  size_t len;                     /* Actually used length in buf. */
  void (*nomem) (void);           /* Out of memory handler. */
  int err;                        /* Error indicator */
};

void stringbuf_init (struct stringbuf *sb, void (*nomem) (void));
void stringbuf_reset (struct stringbuf *sb);
int stringbuf_truncate (struct stringbuf *sb, size_t len);
char *stringbuf_finish (struct stringbuf *sb);
void stringbuf_free (struct stringbuf *sb);
int stringbuf_add (struct stringbuf *sb, char const *str, size_t len);
int stringbuf_add_char (struct stringbuf *sb, int c);
int stringbuf_add_string (struct stringbuf *sb, char const *str);
int stringbuf_vprintf (struct stringbuf *sb, char const *fmt, va_list ap);
int stringbuf_printf (struct stringbuf *sb, char const *fmt, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));
char *stringbuf_set (struct stringbuf *sb, int c, size_t n);
struct tm;
int stringbuf_strftime (struct stringbuf *sb, char const *fmt,
			const struct tm *tm);

static inline int
stringbuf_err (struct stringbuf *sb)
{
  return sb->err;
}

static inline char *stringbuf_value (struct stringbuf *sb)
{
  return sb->base;
}

static inline size_t stringbuf_len (struct stringbuf *sb)
{
  return sb->len;
}

static inline void stringbuf_consume (struct stringbuf *sb, size_t len)
{
  if (len < sb->len)
    {
      memmove (sb->base, sb->base + len, sb->len - len);
      sb->len -= len;
    }
  else
    sb->len = 0;
}

extern void xnomem (void);
extern void lognomem (void);

static inline void xstringbuf_init (struct stringbuf *sb)
{
  stringbuf_init (sb, xnomem);
}

static inline void stringbuf_init_log (struct stringbuf *sb)
{
  stringbuf_init (sb, lognomem);
}

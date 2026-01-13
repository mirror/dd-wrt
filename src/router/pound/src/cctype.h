/* This file is part of Pound.
   Copyright (C) 2025 Sergey Poznyakoff.
 
   Pound is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.
 
   Pound is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with Pound.  If not, see <http://www.gnu.org/licenses/>. */

#include <stddef.h>

#define CCTYPE_ALPHA   0x0001
#define CCTYPE_DIGIT   0x0002
#define CCTYPE_BLANK   0x0004
#define CCTYPE_CNTRL   0x0008
#define CCTYPE_GRAPH   0x0010
#define CCTYPE_LOWER   0x0020
#define CCTYPE_UPPER   0x0040
#define CCTYPE_PRINT   0x0080
#define CCTYPE_PUNCT   0x0100
#define CCTYPE_SPACE   0x0200
#define CCTYPE_XLETR   0x0400
#define CCTYPE_UNRSRV  0x0800 /* RFC 3986 section 2.2 unreserved characters */
#define CC_TAB_MAX     128

extern int cc_tab[CC_TAB_MAX];

static inline int
c_isascii (int c)
{
  return (unsigned)c < CC_TAB_MAX;
}

static inline int
c_is_class (int c, int class)
{
  return c_isascii(c) && (cc_tab[(unsigned)c] & class);
}

#define c_isalpha(c)  c_is_class (c, CCTYPE_ALPHA)
#define c_iscntrl(c)  c_is_class (c, CCTYPE_CNTRL)
#define c_isdigit(c)  c_is_class (c, CCTYPE_DIGIT)
#define c_isgraph(c)  c_is_class (c, CCTYPE_GRAPH)
#define c_islower(c)  c_is_class (c, CCTYPE_LOWER)
#define c_isprint(c)  c_is_class (c, CCTYPE_PRINT)
#define c_ispunct(c)  c_is_class (c, CCTYPE_PUNCT)
#define c_isspace(c)  c_is_class (c, CCTYPE_SPACE)
#define c_isupper(c)  c_is_class (c, CCTYPE_UPPER)
#define c_isxdigit(c) c_is_class (c, CCTYPE_DIGIT|CCTYPE_XLETR)
#define c_isalnum(c)  c_is_class (c, CCTYPE_ALPHA|CCTYPE_DIGIT)
#define c_isblank(c)  c_is_class (c, CCTYPE_BLANK)

static inline int
c_tolower (int c)
{
  return c_isupper (c) ? c - 'A' + 'a' : c;
}

static inline int
c_toupper (int c)
{
  return c_islower (c) ? c - 'a' + 'A' : c;
}

int c_strcasecmp (char const *a, char const *b);
int c_strncasecmp (char const *a, char const *b, size_t n);
size_t c_memspn (char const *str, int class, size_t len);
size_t c_memcspn (char const *str, int class, size_t len);
size_t c_memrspn (char const *str, int class, size_t len);
size_t c_memrcspn (char const *str, int class, size_t len);

size_t c_trimrws (char const *str, size_t len);
char *c_trimlws (char const *str, size_t *plen);
char *c_trimws (char const *str, size_t *plen);




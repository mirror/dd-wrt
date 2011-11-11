/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2008-2011 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* String manipulation functions
 * $Id: str.c,v 1.11 2011/05/23 21:22:24 castaglia Exp $
 */

#include "conf.h"

/* Maximum number of replacements that we will do in a given string. */
#define PR_STR_MAX_REPLACEMENTS			128

char *sreplace(pool *p, char *s, ...) {
  va_list args;
  char *m, *r, *src, *cp;
  char *matches[PR_STR_MAX_REPLACEMENTS+1], *replaces[PR_STR_MAX_REPLACEMENTS+1];
  char buf[PR_TUNABLE_PATH_MAX] = {'\0'}, *pbuf = NULL;
  size_t nmatches = 0, rlen = 0;
  int blen = 0;

  if (p == NULL ||
      s == NULL) {
    errno = EINVAL;
    return NULL;
  }

  src = s;
  cp = buf;
  *cp = '\0';

  memset(matches, 0, sizeof(matches));
  memset(replaces, 0, sizeof(replaces));

  blen = strlen(src) + 1;

  va_start(args, s);

  while ((m = va_arg(args, char *)) != NULL &&
         nmatches < PR_STR_MAX_REPLACEMENTS) {
    char *tmp = NULL;
    int count = 0;

    r = va_arg(args, char *);
    if (r == NULL) {
      break;
    }

    /* Increase the length of the needed buffer by the difference between
     * the given match and replacement strings, multiplied by the number
     * of times the match string occurs in the source string.
     */
    tmp = strstr(s, m);
    while (tmp) {
      pr_signals_handle();
      count++;
      if (count > 8) {
        /* More than eight instances of the same escape on the same line?
         * Give me a break.
         */
        return s;
      }

      /* Be sure to increment the pointer returned by strstr(3), to
       * advance past the beginning of the substring for which we are
       * looking.  Otherwise, we just loop endlessly, seeing the same
       * value for tmp over and over.
       */
      tmp += strlen(m);
      tmp = strstr(tmp, m);
    }

    /* We are only concerned about match/replacement strings that actually
     * occur in the given string.
     */
    if (count) {
      blen += count * (strlen(r) - strlen(m));
      if (blen < 0) {
        /* Integer overflow. In order to overflow this, somebody must be
         * doing something very strange. The possibility still exists that
         * we might not catch this overflow in extreme corner cases, but
         * massive amounts of data (gigabytes) would need to be in s to
         * trigger this, easily larger than any buffer we might use.
         */
        return s;
      }
      matches[nmatches] = m;
      replaces[nmatches++] = r;
    }
  }

  va_end(args);

  /* If there are no matches, then there is nothing to replace. */
  if (nmatches == 0) {
    return s;
  }

  /* Try to handle large buffer situations (i.e. escaping of PR_TUNABLE_PATH_MAX
   * (>2048) correctly, but do not allow very big buffer sizes, that may
   * be dangerous (BUFSIZ may be defined in stdio.h) in some library
   * functions.
   */
#ifndef BUFSIZ
# define BUFSIZ 8192
#endif

  if (blen >= BUFSIZ) {
    errno = ENOSPC;
    return NULL;
  }

  cp = pbuf = (char *) pcalloc(p, ++blen);

  while (*src) {
    char **mptr, **rptr;

    for (mptr = matches, rptr = replaces; *mptr; mptr++, rptr++) {
      size_t mlen;

      mlen = strlen(*mptr);
      rlen = strlen(*rptr);

      if (strncmp(src, *mptr, mlen) == 0) {
        sstrncpy(cp, *rptr, blen - strlen(pbuf));

        if (((cp + rlen) - pbuf + 1) > blen) {
          pr_log_pri(PR_LOG_ERR,
            "WARNING: attempt to overflow internal ProFTPD buffers");
          cp = pbuf;

          cp += (blen - 1);
          goto done;

        } else {
          cp += rlen;
        }
	
        src += mlen;
        break;
      }
    }

    if (!*mptr) {
      if ((cp - pbuf + 1) >= blen) {
        pr_log_pri(PR_LOG_ERR,
          "WARNING: attempt to overflow internal ProFTPD buffers");
        cp = pbuf;

        cp += (blen - 1);
        goto done;
      }

      *cp++ = *src++;
    }
  }

 done:
  *cp = '\0';

  return pbuf;
}

/* "safe" strcat, saves room for NUL at end of dst, and refuses to copy more
 * than "n" bytes.
 */
char *sstrcat(char *dst, const char *src, size_t n) {
  register char *d;

  if (!dst || !src || n == 0) {
    errno = EINVAL;
    return NULL;
  }

  for (d = dst; *d && n > 1; d++, n--) ;

  while (n-- > 1 && *src)
    *d++ = *src++;

  *d = 0;
  return dst;
}

char *pstrdup(pool *p, const char *str) {
  char *res;
  size_t len;

  if (!p || !str) {
    errno = EINVAL;
    return NULL;
  }

  len = strlen(str) + 1;

  res = palloc(p, len);
  sstrncpy(res, str, len);
  return res;
}

char *pstrndup(pool *p, const char *str, size_t n) {
  char *res;

  if (!p || !str) {
    errno = EINVAL;
    return NULL;
  }

  res = palloc(p, n + 1);
  sstrncpy(res, str, n + 1);
  return res;
}

char *pdircat(pool *p, ...) {
  char *argp, *res;
  char last;

  int count = 0;
  size_t len = 0;
  va_list ap;

  if (p == NULL) {
    errno = EINVAL;
    return NULL;
  }

  va_start(ap, p);

  last = 0;

  while ((res = va_arg(ap, char *)) != NULL) {
    /* If the first argument is "", we have to account for a leading /
     * which must be added.
     */
    if (!count++ && !*res)
      len++;

    else if (last && last != '/' && *res != '/')
      len++;

    else if (last && last == '/' && *res == '/')
      len--;

    len += strlen(res);
    last = (*res ? res[strlen(res) - 1] : 0);
  }

  va_end(ap);
  res = (char *) pcalloc(p, len + 1);

  va_start(ap, p);

  last = 0;

  while ((argp = va_arg(ap, char *)) != NULL) {
    if (last && last == '/' && *argp == '/')
      argp++;

    else if (last && last != '/' && *argp != '/')
      sstrcat(res, "/", len + 1);

    sstrcat(res, argp, len + 1);
    last = (*res ? res[strlen(res) - 1] : 0);
  }

  va_end(ap);

  return res;
}

char *pstrcat(pool *p, ...) {
  char *argp, *res;

  size_t len = 0;
  va_list ap;

  if (p == NULL) {
    errno = EINVAL;
    return NULL;
  }

  va_start(ap, p);

  while ((res = va_arg(ap, char *)) != NULL)
    len += strlen(res);

  va_end(ap);

  res = pcalloc(p, len + 1);

  va_start(ap, p);

  while ((argp = va_arg(ap, char *)) != NULL)
    sstrcat(res, argp, len + 1);

  va_end(ap);

  return res;
}

char *pr_str_strip(pool *p, char *str) {
  char c, *dupstr, *start, *finish;
 
  if (!p || !str) {
    errno = EINVAL;
    return NULL;
  }
 
  /* First, find the non-whitespace start of the given string */
  for (start = str; isspace((int) *start); start++);
 
  /* Now, find the non-whitespace end of the given string */
  for (finish = &str[strlen(str)-1]; isspace((int) *finish); finish--);

  /* finish is now pointing to a non-whitespace character.  So advance one
   * character forward, and set that to NUL.
   */
  c = *++finish;
  *finish = '\0';

  /* The space-stripped string is, then, everything from start to finish. */
  dupstr = pstrdup(p, start);

  /* Restore the given string buffer contents. */
  *finish = c;

  return dupstr;
}

char *pr_str_strip_end(char *s, char *ch) {
  size_t len;

  if (s == NULL ||
      ch == NULL) {
    errno = EINVAL;
    return NULL;
  }

  len = strlen(s);

  while (len && strchr(ch, *(s+len - 1))) {
    pr_signals_handle();

    *(s+len - 1) = '\0';
    len--;
  }

  return s;
}

char *pr_str_get_word(char **cp, int flags) {
  char *res, *dst;
  char quote_mode = 0;

  if (cp == NULL ||
     !*cp ||
     !**cp) {
    errno = EINVAL;
    return NULL;
  }

  if (!(flags & PR_STR_FL_PRESERVE_WHITESPACE)) {
    while (**cp && isspace((int) **cp))
      (*cp)++;
  }

  if (!**cp)
    return NULL;

  res = dst = *cp;

  if (!(flags & PR_STR_FL_PRESERVE_COMMENTS)) {
    /* Stop processing at start of an inline comment. */
    if (**cp == '#')
      return NULL;
  }

  if (**cp == '\"') {
    quote_mode++;
    (*cp)++;
  }

  while (**cp && (quote_mode ? (**cp != '\"') : !isspace((int) **cp))) {
    if (**cp == '\\' && quote_mode) {

      /* Escaped char */
      if (*((*cp)+1))
        *dst = *(++(*cp));
    }

    *dst++ = **cp;
    ++(*cp);
  }

  if (**cp)
    (*cp)++;
  *dst = '\0';

  return res;
}

/* get_token tokenizes a string, increments the src pointer to the next
 * non-separator in the string.  If the src string is empty or NULL, the next
 * token returned is NULL.
 */
char *pr_str_get_token(char **s, char *sep) {
  char *res;

  if (s == NULL ||
      *s == NULL ||
      **s == '\0' ||
      sep == NULL) {
    errno = EINVAL;
    return NULL;
  }

  res = *s;

  while (**s && !strchr(sep, **s)) {
    (*s)++;
  }

  if (**s)
    *(*s)++ = '\0';

  return res;
}

int pr_str_is_boolean(const char *str) {
  if (str == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (strncasecmp(str, "on", 3) == 0) {
    return TRUE;
  }

  if (strncasecmp(str, "off", 4) == 0) {
    return FALSE;
  }

  if (strncasecmp(str, "yes", 4) == 0) {
    return TRUE;
  }
 
  if (strncasecmp(str, "no", 3) == 0) {
    return FALSE;
  }

  if (strncasecmp(str, "true", 5) == 0) {
    return TRUE;
  }

  if (strncasecmp(str, "false", 6) == 0) {
    return FALSE;
  }

  if (strncasecmp(str, "1", 2) == 0) {
    return TRUE;
  }

  if (strncasecmp(str, "0", 2) == 0) {
    return FALSE;
  }

  errno = EINVAL;
  return -1;
}

/* Return true if str contains any of the glob(7) characters. */
int pr_str_is_fnmatch(const char *str) {
  int have_bracket = 0;

  while (*str) {
    switch (*str) {
      case '?':
      case '*':
        return TRUE;

      case '\\':
        /* If the next character is NUL, we've reached the end of the string. */
        if (*(str+1) == '\0')
          return FALSE;

        /* Skip past the escaped character, i.e. the next character. */
        str++;
        break;

      case '[':
        have_bracket++;
        break;

      case ']':
        if (have_bracket)
          return TRUE;
        break;

      default:
        break;
    }

    str++;
  }

  return FALSE;
}


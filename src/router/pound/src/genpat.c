/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2023-2025 Sergey Poznyakoff
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
#include <regex.h>
#include <string.h>
#include <assert.h>

static struct genpat_defn const *genpat_vtab[] = {
  [GENPAT_POSIX]   = &posix_genpat_defn,
  [GENPAT_PCRE]    = PCRE_REGEX_DEFN,
  [GENPAT_PREFIX]  = &prefix_genpat_defn,
  [GENPAT_SUFFIX]  = &suffix_genpat_defn,
  [GENPAT_CONTAIN] = &contain_genpat_defn,
  [GENPAT_EXACT]   = &exact_genpat_defn
};
#define GENPAT_MAX (sizeof (genpat_vtab) / sizeof (genpat_vtab[0]))


int
genpat_compile (GENPAT *retval, int pat_type, const char *pattern, int pflags)
{
  GENPAT gp;
  struct genpat_defn const *vtab;
  int rc;

  assert (pat_type >= 0 && pat_type < GENPAT_MAX);
  if ((vtab = genpat_vtab[pat_type]) == NULL)
    {
      *retval = NULL;
      errno = ENOSYS;
      return -1;
    }

  XZALLOC (gp);
  gp->vtab = vtab;
  *retval = gp;
  if ((rc = vtab->gp_init (gp)) != 0)
    return rc;
  return vtab->gp_compile (gp->data, pattern, pflags);
}

#define GENPAT_ASSERT(p) \
  assert (p != NULL); \
  assert (p->vtab != NULL);

char const *
genpat_error (GENPAT p, size_t *off)
{
  GENPAT_ASSERT (p);
  return p->vtab->gp_error (p->data, off);
}

int
genpat_match (GENPAT p, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  GENPAT_ASSERT (p);
  return p->vtab->gp_exec (p->data, subj, n, prm);
}

size_t
genpat_nsub (GENPAT p)
{
  GENPAT_ASSERT (p);
  return p->vtab->gp_nsub (p->data);
}

void
genpat_free (GENPAT p)
{
  GENPAT_ASSERT (p);
  p->vtab->gp_free (p->data);
  free (p);
}

struct posix_pattern
{
  regex_t re;
  char *errmsg;
};

static int
posix_pattern_init (GENPAT p)
{
  p->data = xzalloc (sizeof (struct posix_pattern));
  return 0;
}

static int
posix_pattern_compile (void *gp_data, const char *pattern, int pflags)
{
  struct posix_pattern *pat = gp_data;
  int flags = REG_EXTENDED;
  int rc;

  if (pflags & GENPAT_ICASE)
    flags |= REG_ICASE;
  if (pflags & GENPAT_MULTILINE)
    flags |= REG_NEWLINE;

  if ((rc = regcomp (&pat->re, pattern, flags)) != 0)
    {
      char errbuf[128];
      regerror (rc, &pat->re, errbuf, sizeof (errbuf));
      pat->errmsg = xstrdup (errbuf);
    }
  return rc;
}

static char const *
posix_pattern_error (void *gp_data, size_t *off)
{
  struct posix_pattern *pat = gp_data;
  *off = 0;
  return pat->errmsg;
}

static int
posix_pattern_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct posix_pattern *pat = gp_data;
  int rc;
  regmatch_t *rm = NULL;

  if (n > 0)
    {
      rm = calloc (n, sizeof (rm[0]));
      if (rm == NULL)
	return -1;
      if (n > pat->re.re_nsub + 1)
	n = pat->re.re_nsub + 1;
    }

  if ((rc = regexec (&pat->re, subj, n, rm, 0)) == 0)
    {
      size_t i;
      for (i = 0; i < n; i++)
	{
	  prm[i].rm_so = rm[i].rm_so;
	  prm[i].rm_eo = rm[i].rm_eo;
	}
    }
  free (rm);
  return rc == REG_NOMATCH;
}

static size_t
posix_pattern_nsub (void *gp_data)
{
  struct posix_pattern *pat = gp_data;
  return pat->re.re_nsub + 1;
}

static void
posix_pattern_free (void *gp_data)
{
  if (gp_data)
    {
      struct posix_pattern *pat = gp_data;
      regfree (&pat->re);
      free (pat->errmsg);
      free (pat);
    }
}

struct genpat_defn posix_genpat_defn =
  {
    .gp_init = posix_pattern_init,
    .gp_compile = posix_pattern_compile,
    .gp_error = posix_pattern_error,
    .gp_exec = posix_pattern_exec,
    .gp_nsub = posix_pattern_nsub,
    .gp_free = posix_pattern_free
  };

/* Substring matches */
struct substr_pattern
{
  char *pattern;      /* Substring. */
  size_t len;         /* Length of the pattern. */
  int ci;             /* Is it case-insensitive. */
};

static int
substr_init (GENPAT p)
{
  p->data = xzalloc (sizeof (struct substr_pattern));
  return 0;
}

static void
substr_free (void *gp_data)
{
  if (gp_data)
    {
      struct substr_pattern *pat = gp_data;
      free (pat->pattern);
      free (pat);
    }
}

static int
substr_compile (void *gp_data, const char *pattern, int pflags)
{
  struct substr_pattern *pat = gp_data;
  pat->ci = pflags & GENPAT_ICASE;
  //FIXME: error out on GENPAT_MULTILINE
  pat->pattern = xstrdup (pattern);
  pat->len = strlen (pat->pattern);
  return 0;
}

static char const *
substr_error (void *gp_data, size_t *off)
{
  *off = 0;
  return "no error";
}

static size_t
substr_num_submatch (void *gp_data)
{
  return 0;
}

/*
 * Implementations of the substring matching.
 */

/* Exact match. */
static int
exact_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct substr_pattern *pat = gp_data;
  size_t slen = strlen (subj);

  if (slen != pat->len)
    return 1;
  return (pat->ci ? c_strcasecmp : strcmp) (subj, pat->pattern);
}

/* Prefix match (-beg). */
static int
prefix_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct substr_pattern *pat = gp_data;
  size_t slen = strlen (subj);

  if (slen < pat->len)
    return 1;
  return (pat->ci ? c_strncasecmp : strncmp) (subj, pat->pattern, pat->len);
}

/* Suffix match (-end). */
static int
suffix_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct substr_pattern *pat = gp_data;
  size_t slen = strlen (subj);

  if (slen < pat->len)
    return 1;
  return (pat->ci ? c_strncasecmp : strncmp) (subj + slen - pat->len,
					      pat->pattern, pat->len);
}

/* "Regex" definitions for the above. */
struct genpat_defn prefix_genpat_defn =
  {
    .gp_init = substr_init,
    .gp_compile = substr_compile,
    .gp_error = substr_error,
    .gp_exec = prefix_exec,
    .gp_nsub = substr_num_submatch,
    .gp_free = substr_free
  };

struct genpat_defn suffix_genpat_defn =
  {
    .gp_init = substr_init,
    .gp_compile = substr_compile,
    .gp_error = substr_error,
    .gp_exec = suffix_exec,
    .gp_nsub = substr_num_submatch,
    .gp_free = substr_free
  };

struct genpat_defn exact_genpat_defn =
  {
    .gp_init = substr_init,
    .gp_compile = substr_compile,
    .gp_error = substr_error,
    .gp_exec = exact_exec,
    .gp_nsub = substr_num_submatch,
    .gp_free = substr_free
  };

/*
 * Substring match (-contain).
 * Implements Boyer–Moore string search algorithm.
 */

/* Upper-case equivalents of ASCII symbols. */
static unsigned char casemap[] = {
  '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
  '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
  '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
  '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
     ' ',    '!',    '"',    '#',    '$',    '%',    '&',   '\'',
     '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
     '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
     '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
     'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
     'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
     'X',    'Y',    'Z',    '[',   '\\',    ']',    '^',    '_',
     '`',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
     'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
     'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
     'X',    'Y',    'Z',    '{',    '|',    '}',    '~', '\177',
  '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
  '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
  '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
  '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
  '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
  '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
  '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
  '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
  '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
  '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
  '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
  '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
  '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
  '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
  '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
  '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377'
};

struct strstr_pattern
{
  unsigned char *pattern;          /* Search pattern. */
  size_t patlen;                   /* Length of the pattern. */
  int ci;                          /* Case-insensitivity flag. */
  int bad_char_delta[UCHAR_MAX];   /* Table of shifts for bad character rule. */
  int *good_pfx_delta;             /* Table of shifts for good prefix rule. */
};

#define UC(sp,c) ((sp)->ci ? casemap[(unsigned char)c] : (unsigned char) c)

/*
 * Compute shift amounts for bad character rule.
 * Each table element bad_char_delta[C] contains the distance between the last
 * character in the pattern and the rightmost occurrence of character C
 * in the pattern.
 *
 * If C doesn't occur in the pattern, the value is pattern length - 1.
 *
 * The table will be indexed by characters from the subject string, therefore
 * case mapping is applied to its indices.
 */
static void
fill_bad_char_delta (struct strstr_pattern *sp)
{
  int i;

  for (i = 0; i < UCHAR_MAX; i++)
    sp->bad_char_delta[UC (sp, i)] = sp->patlen - 1;
  for (i = 0; i < sp->patlen; i++)
    sp->bad_char_delta[UC (sp, sp->pattern[i])] = sp->patlen - 1 - i;
}

/*
 * Return true if text starting at str[pos] is the prefix of str,
 * i.e. str[pos..len] == str[0..len-pos].
 * For case-insensitive patterns, this uses the fact that the pattern
 * has already been converted to upper-case.
 */
static int
is_prefix (unsigned char *str, int len, int pos)
{
  return memcmp (str, str + pos, len - pos) != 0;
}

/*
 * Return the biggest possible N, such that
 *   str[pos..pos+N] == str[len-N..len]
 */
static int
longest_suffix_length (unsigned char *str, int len, int pos)
{
  int i;

  for (i = 0; i < pos; i++)
    if (str[pos - i] != str[len - 1 - i])
      break;

  return i;
}

/*
 * Compute shift amounts for good suffix rule.
 *
 * The Good suffix rule applies if a substring in subject string matches
 * a suffix of pattern P[i,patlen-1] and this substring is the largest
 * such substring for the given alignment.
 * 
 * There are two cases:
 * 
 * 1. The substring occurs elsewhere in pattern.  In this case the algorithm
 * selects the rightmost occurrence of the substring such that it does not
 * form the suffix of the pattern and the character to the left of it is not
 * the same as the character to the left of the found substring.
 *
 * 2. The substring does not occur elsewhere in pattern.  In that case,
 * shift the left end of the pattern to the right past the left end of
 * the found substring, so that a prefix of the shifted pattern is also
 * a suffix of the substring.  If such shift is not possible, then shift
 * pattern patlen characters to the right.
 */
static void
fill_good_pfx_delta (struct strstr_pattern *sp)
{
  int i;
  int j = 1;

  sp->good_pfx_delta = xcalloc (sp->patlen, sizeof sp->good_pfx_delta[0]);

  for (i = sp->patlen - 1; i >= 0; i--)
    {
      if (is_prefix (sp->pattern, sp->patlen, i + 1))
	j = i + 1;
      sp->good_pfx_delta[i] = j + sp->patlen - 1 - i;
    }

  for (i = 0; i < sp->patlen - 1; i++)
    {
      int suflen = longest_suffix_length (sp->pattern, sp->patlen, i);
      if (sp->pattern[i - suflen] != sp->pattern[sp->patlen - 1 - suflen])
	sp->good_pfx_delta[sp->patlen - 1 - suflen] = sp->patlen - 1 - i + suflen;
    }
}

int
strstr_init (GENPAT p)
{
  p->data = xzalloc (sizeof (struct strstr_pattern));
  return 0;
}

static void
strstr_free (void *gp_data)
{
  if (gp_data)
    {
      struct strstr_pattern *sp = gp_data;
      free (sp->pattern);
      free (sp->good_pfx_delta);
      free (sp);
    }
}

int
strstr_compile (void *gp_data, const char *pattern, int pflags)
{
  struct strstr_pattern *sp = gp_data;
  int i;

  sp->ci = pflags & GENPAT_ICASE;
  sp->patlen = strlen (pattern);
  sp->pattern = xmalloc (sp->patlen + 1);
  for (i = 0; i < sp->patlen; i++)
    sp->pattern[i] = UC (sp, pattern[i]);
  sp->pattern[i] = 0;

  fill_bad_char_delta (sp);
  fill_good_pfx_delta (sp);
  return 0;
}

#define max(a,b) ((a)>(b)?(a):(b))

int
strstr_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct strstr_pattern *sp = gp_data;
  int i;
  int subjlen;

  if (sp->patlen == 0)
    return 0;

  subjlen = strlen (subj);
  i = sp->patlen - 1;
  while (i < subjlen)
    {
      int j;

      for (j = sp->patlen - 1; j >= 0 && UC (sp, subj[i]) == sp->pattern[j]; i--, j--)
	;

      if (j < 0)
	return 0;

      i += max (sp->bad_char_delta[UC (sp, subj[i])], sp->good_pfx_delta[j]);
    }
  return 1;
}

struct genpat_defn contain_genpat_defn =
  {
    .gp_init = strstr_init,
    .gp_compile = strstr_compile,
    .gp_error = substr_error,
    .gp_exec = strstr_exec,
    .gp_nsub = substr_num_submatch,
    .gp_free = strstr_free
  };

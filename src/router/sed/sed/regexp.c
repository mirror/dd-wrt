/*  GNU SED, a batch stream editor.
    Copyright (C) 1999-2022 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <https://www.gnu.org/licenses/>. */

#include "sed.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xalloc.h"

#ifdef gettext_noop
# define N_(String) gettext_noop(String)
#else
# define N_(String) (String)
#endif

extern bool use_extended_syntax_p;

static const char errors[] =
  "no previous regular expression\0"
  "cannot specify modifiers on empty regexp";

#define NO_REGEX (errors)
#define BAD_MODIF (NO_REGEX + sizeof(N_("no previous regular expression")))


void
dfaerror (char const *mesg)
{
  panic ("%s", mesg);
}

void
dfawarn (char const *mesg)
{
  if (!getenv ("POSIXLY_CORRECT"))
    dfaerror (mesg);
}


static void
compile_regex_1 (struct regex *new_regex, int needed_sub)
{
  const char *error;
  int syntax = ((extended_regexp_flags & REG_EXTENDED)
                 ? RE_SYNTAX_POSIX_EXTENDED
                 : RE_SYNTAX_POSIX_BASIC);

  syntax &= ~RE_DOT_NOT_NULL;
  syntax |= RE_NO_POSIX_BACKTRACKING;

  switch (posixicity)
    {
    case POSIXLY_EXTENDED:
      syntax &= ~RE_UNMATCHED_RIGHT_PAREN_ORD;
      break;
    case POSIXLY_CORRECT:
      syntax |= RE_UNMATCHED_RIGHT_PAREN_ORD;
      break;
    case POSIXLY_BASIC:
      syntax |= RE_UNMATCHED_RIGHT_PAREN_ORD | RE_NO_GNU_OPS;
      if (!(extended_regexp_flags & REG_EXTENDED))
        syntax |= RE_LIMITED_OPS;
      break;
    }

  if (new_regex->flags & REG_ICASE)
    syntax |= RE_ICASE;
  else
    new_regex->pattern.fastmap = malloc (1 << (sizeof (char) * 8));
  syntax |= needed_sub ? 0 : RE_NO_SUB;

  /* If REG_NEWLINE is set, newlines are treated differently.  */
  if (new_regex->flags & REG_NEWLINE)
    {
      /* REG_NEWLINE implies neither . nor [^...] match newline.  */
      syntax &= ~RE_DOT_NEWLINE;
      syntax |= RE_HAT_LISTS_NOT_NEWLINE;
    }

  re_set_syntax (syntax);
  error = re_compile_pattern (new_regex->re, new_regex->sz,
                              &new_regex->pattern);
  new_regex->pattern.newline_anchor =
    buffer_delimiter == '\n' && (new_regex->flags & REG_NEWLINE) != 0;

  new_regex->pattern.translate = NULL;
#ifndef RE_ICASE
  if (new_regex->flags & REG_ICASE)
    {
      static char translate[1 << (sizeof (char) * 8)];
      int i;
      for (i = 0; i < sizeof (translate) / sizeof (char); i++)
        translate[i] = tolower (i);

      new_regex->pattern.translate = translate;
    }
#endif

  if (error)
    bad_prog (error);

  /* Just to be sure, I mark this as not POSIXLY_CORRECT behavior */
  if (needed_sub
      && new_regex->pattern.re_nsub < needed_sub - 1
      && posixicity == POSIXLY_EXTENDED)
    {
      char buf[200];
      sprintf (buf, _("invalid reference \\%d on `s' command's RHS"),
              needed_sub - 1);
      bad_prog (buf);
    }

  int dfaopts = buffer_delimiter == '\n' ? 0 : DFA_EOL_NUL;
  new_regex->dfa = dfaalloc ();
  dfasyntax (new_regex->dfa, &localeinfo, syntax, dfaopts);
  dfacomp (new_regex->re, new_regex->sz, new_regex->dfa, 1);

  /* The patterns which consist of only ^ or $ often appear in
     substitution, but regex and dfa are not good at them, as regex does
     not build fastmap, and as all in buffer must be scanned for $.  So
     we mark them to handle manually.  */
  if (new_regex->sz == 1)
    {
      if (new_regex->re[0] == '^')
        new_regex->begline = true;
      if (new_regex->re[0] == '$')
        new_regex->endline = true;
    }
}

struct regex *
compile_regex (struct buffer *b, int flags, int needed_sub)
{
  struct regex *new_regex;
  size_t re_len;

  /* // matches the last RE */
  if (size_buffer (b) == 0)
    {
      if (flags > 0)
        bad_prog (_(BAD_MODIF));
      return NULL;
    }

  re_len = size_buffer (b);
  new_regex = xzalloc (sizeof (struct regex) + re_len - 1);
  new_regex->flags = flags;
  memcpy (new_regex->re, get_buffer (b), re_len);

  /* GNU regex does not process \t & co. */
  new_regex->sz = normalize_text (new_regex->re, re_len, TEXT_REGEX);

  compile_regex_1 (new_regex, needed_sub);
  return new_regex;
}

int
match_regex (struct regex *regex, char *buf, size_t buflen,
            size_t buf_start_offset, struct re_registers *regarray,
            int regsize)
{
  int ret;
  static struct regex *regex_last;

  /* printf ("Matching from %d/%d\n", buf_start_offset, buflen); */

  /* Keep track of the last regexp matched. */
  if (!regex)
    {
      regex = regex_last;
      if (!regex_last)
        bad_prog (_(NO_REGEX));
    }
  else
    regex_last = regex;

  /* gnulib's re_search uses signed-int as length */
  if (buflen >= INT_MAX)
    panic (_("regex input buffer length larger than INT_MAX"));

  if (regex->pattern.no_sub && regsize)
    {
      /* Re-compiling an existing regex, free the previously allocated
         structures.  */
      if (regex->dfa)
        {
          dfafree (regex->dfa);
          free (regex->dfa);
          regex->dfa = NULL;
        }
      regfree (&regex->pattern);

      compile_regex_1 (regex, regsize);
    }

  regex->pattern.regs_allocated = REGS_REALLOCATE;

  /* Optimized handling for '^' and '$' patterns */
  if (regex->begline || regex->endline)
    {
      size_t offset;

      if (regex->endline)
        {
          const char *p = NULL;

          if (regex->flags & REG_NEWLINE)
            p = memchr (buf + buf_start_offset, buffer_delimiter,
                        buflen - buf_start_offset);

          offset = p ? p - buf : buflen;
        }
      else if (buf_start_offset == 0)
        /* begline anchor, starting at beginning of the buffer. */
        offset = 0;
      else if (!(regex->flags & REG_NEWLINE))
        /* begline anchor, starting in the middle of the text buffer,
           and multiline regex is not specified - will never match.
           Example: seq 2 | sed 'N;s/^/X/g' */
        return 0;
      else if (buf[buf_start_offset - 1] == buffer_delimiter)
        /* begline anchor, starting in the middle of the text buffer,
           with multiline match, and the current character
           is the line delimiter - start here.
           Example: seq 2 | sed 'N;s/^/X/mg' */
        offset = buf_start_offset;
      else
        {
          /* begline anchor, starting in the middle of the search buffer,
             all previous optimizions didn't work: search
             for the next line delimiter character in the buffer,
             and start from there if found. */
          const char *p = memchr (buf + buf_start_offset, buffer_delimiter,
                                  buflen - buf_start_offset);

          if (p == NULL)
            return 0;

          offset = p - buf + 1;
        }

      if (regsize)
        {
          size_t i;

          if (!regarray->start)
            {
              regarray->start = XCALLOC (1, regoff_t);
              regarray->end = XCALLOC (1, regoff_t);
              regarray->num_regs = 1;
            }

          regarray->start[0] = offset;
          regarray->end[0] = offset;

          for (i = 1 ; i < regarray->num_regs; ++i)
            regarray->start[i] = regarray->end[i] = -1;
        }

      return 1;
    }

  if (buf_start_offset == 0)
    {
      struct dfa *superset = dfasuperset (regex->dfa);

      if (superset && !dfaexec (superset, buf, buf + buflen, true, NULL, NULL))
        return 0;

      if ((!regsize && (regex->flags & REG_NEWLINE))
          || (!superset && dfaisfast (regex->dfa)))
        {
          bool backref = false;

          if (!dfaexec (regex->dfa, buf, buf + buflen, true, NULL, &backref))
            return 0;

          if (!regsize && (regex->flags & REG_NEWLINE) && !backref)
            return 1;
        }
    }

  /* If the buffer delimiter is not newline character, we cannot use
     newline_anchor flag of regex.  So do it line-by-line, and add offset
     value to results.  */
  if ((regex->flags & REG_NEWLINE) && buffer_delimiter != '\n')
    {
      const char *beg, *end;
      const char *start;

      beg = buf;

      if (buf_start_offset > 0)
        {
          const char *eol = memrchr (buf, buffer_delimiter, buf_start_offset);

          if (eol != NULL)
            beg = eol + 1;
        }

      start = buf + buf_start_offset;

      for (;;)
        {
          end = memchr (beg, buffer_delimiter, buf + buflen - beg);

          if (end == NULL)
            end = buf + buflen;

          ret = re_search (&regex->pattern, beg, end - beg,
                           start - beg, end - start,
                           regsize ? regarray : NULL);

          if (ret > -1)
            {
              size_t i;

              ret += beg - buf;

              if (regsize)
                {
                  for (i = 0; i < regarray->num_regs; ++i)
                    {
                      if (regarray->start[i] > -1)
                        regarray->start[i] += beg - buf;
                      if (regarray->end[i] > -1)
                        regarray->end[i] += beg - buf;
                    }
                }

              break;
            }

          if (end == buf + buflen)
            break;

          beg = start = end + 1;
        }
    }
  else
    ret = re_search (&regex->pattern, buf, buflen, buf_start_offset,
                     buflen - buf_start_offset,
                     regsize ? regarray : NULL);

  return (ret > -1);
}


#ifdef lint
void
release_regex (struct regex *regex)
{
  if (regex->dfa)
    {
      dfafree (regex->dfa);
      free (regex->dfa);
      regex->dfa = NULL;
    }
  regfree (&regex->pattern);
  free (regex);
}
#endif /* lint */

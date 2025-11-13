/*  GNU SED, a batch stream editor.
    Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

/* Written by Assaf Gordon.  */

/* debug.c: debugging functions */

#include "sed.h"
#include "basicdefs.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>

/* indentation level when printing the program */
static int block_level = 0;


void
debug_print_char (char c)
{
  if (ISPRINT (c) && c != '\\')
    {
      putchar (c);
      return;
    }

  putchar ('\\');
  switch (c)
    {
    case '\a':
      putchar ('a');
      break;
    case '\f':
      putchar ('f');
      break;
    case '\r':
      putchar ('r');
      break;
    case '\t':
      putchar ('t');
      break;
    case '\v':
      putchar ('v');
      break;
    case '\n':
      putchar ('n');
      break;
    case '\\':
      putchar ('\\');
      break;

    default:
      printf ("o%03o", (unsigned int) c);
    }
}

static void
debug_print_regex_pattern (const char *pat, size_t len)
{
  const char *p = pat;
  while (len--)
    {
      if (*p == '/')
        fputs ("\\/", stdout);
      else
        debug_print_char (*p);
      ++p;
    }
}

static void
debug_print_regex_flags (const struct regex *r, bool addr)
{
  if (!r)
    return;

#ifdef REG_PERL
  if (r->flags & REG_DOTALL)    /* REG_PERL */
    putchar ('s');
  if (r->flags & REG_EXTENDED)  /* REG_PERL */
    putchar ('x');
#endif

  if (r->flags & REG_ICASE)
    putchar (addr ? 'I' : 'i');
  if (r->flags & REG_NEWLINE)
    putchar (addr ? 'M' : 'm');
}

static void
debug_print_regex (const struct regex *r)
{
  if (!r)
    {
      /* Previous Regex */
      fputs ("//", stdout);
      return;
    }

  putchar ('/');
  debug_print_regex_pattern (r->re, r->sz);
  putchar ('/');
}

static void
debug_print_addr (const struct addr *a)
{
  if (!a)
    return;
  switch (a->addr_type)
    {
    case ADDR_IS_NULL:
      fputs ("[ADDR-NULL]", stdout);
      break;
    case ADDR_IS_REGEX:
      debug_print_regex (a->addr_regex);
      debug_print_regex_flags (a->addr_regex, true);
      break;
    case ADDR_IS_NUM:
      printf ("%lu", a->addr_number);
      break;
    case ADDR_IS_NUM_MOD:
      printf ("%lu~%lu", a->addr_number, a->addr_step);
      break;
    case ADDR_IS_STEP:
      printf ("+%lu", a->addr_step);
      break;
    case ADDR_IS_STEP_MOD:
      printf ("~%lu", a->addr_step);
      break;
    case ADDR_IS_LAST:
      putchar ('$');
      break;
    }
}

static void
debug_print_subst_replacement (const struct replacement *r)
{
  enum replacement_types last_repl_type = REPL_ASIS;

  if (!r)
    return;

  const struct replacement *p = r;
  while (p)
    {
      if (p->repl_type != last_repl_type)
        {
          /* Special GNU replacements \E\U\u\L\l should be printed
             BEFORE the 'prefix' .... the 'prefix' refers to being
             before the backreference.  */
          putchar ('\\');
          if (p->repl_type == 0)
            putchar ('E');
          else if (p->repl_type == REPL_UPPERCASE)
            putchar ('U');
          else if (p->repl_type == REPL_LOWERCASE)
            putchar ('L');
          else if ((p->repl_type & REPL_MODIFIERS) == REPL_UPPERCASE_FIRST)
            putchar ('u');
          else if ((p->repl_type & REPL_MODIFIERS) == REPL_LOWERCASE_FIRST)
            putchar ('l');

          last_repl_type = p->repl_type;
        }

      if (p->prefix_length)
        fwrite (p->prefix, 1, p->prefix_length, stdout);

      if (p->subst_id != -1)
        {
          if (p->subst_id == 0)
            putchar ('&');
          else
            printf ("\\%d", p->subst_id);
        }

      p = p->next;
    }
}

static void
debug_print_output_file (const struct output *o)
{
  if (!o)
    return;

  fputs (o->name, stdout);
}

static void
debug_print_subst (const struct subst *s)
{
  if (!s)
    return;

  debug_print_regex (s->regx);
  debug_print_subst_replacement (s->replacement);
  putchar ('/');

  debug_print_regex_flags (s->regx, false);

  if (s->global)
    putchar ('g');
  if (s->eval)
    putchar ('e');
  if (s->print)
    putchar ('p');
  if (s->numb)
    printf ("%lu", s->numb);
  if (s->outf)
    {
      putchar ('w');
      debug_print_output_file (s->outf);
    }
}

static void
debug_print_translation (const struct sed_cmd *sc)
{
  unsigned int i;

  if (mb_cur_max > 1)
    {
      /* multibyte translation */
      putchar ('/');
      for (i = 0; sc->x.translatemb[2 * i] != NULL; i++)
        fputs (sc->x.translatemb[2 * i], stdout);
      putchar ('/');
      for (i = 0; sc->x.translatemb[2 * i] != NULL; i++)
        fputs (sc->x.translatemb[2 * i + 1], stdout);
      putchar ('/');
    }
  else
    {
      /* unibyte translation */
      putchar ('/');
      for (i = 0; i < 256; ++i)
        if (sc->x.translate[i] != (unsigned char) i)
          putchar ((unsigned char) i);
      putchar ('/');
      for (i = 0; i < 256; ++i)
        if (sc->x.translate[i] != (unsigned char) i)
          putchar (sc->x.translate[i]);
      putchar ('/');
    }
}

static void
debug_print_function (const struct vector *program, const struct sed_cmd *sc)
{
  if (!sc)
    return;

  putchar (sc->cmd);

  switch (sc->cmd)              /* LCOV_EXCL_BR */
    {
    case '=':
      break;

    case ':':
      printf ("%s", sc->x.label_name);
      break;

    case '{':
      break;

    case '}':
      break;

    case '#':                  /* LCOV_EXCL_LINE */
      /* should not happen - discarded during compilation.  */
      assert (0);               /* LCOV_EXCL_LINE */

    case 'a':
    case 'c':
    case 'i':
      fputs ("\\", stdout);
      if (sc->x.cmd_txt.text_length)
        fwrite (sc->x.cmd_txt.text, 1, sc->x.cmd_txt.text_length, stdout);
      break;

    case 'b':
    case 't':
    case 'T':
      {
        if (sc->x.jump_index < program->v_length)
          {
            const char *label_name = program->v[sc->x.jump_index].x.label_name;
            if (label_name)
              printf (" %s", label_name);
          }
      }
      break;

    case 'D':
      break;

    case 'd':
      break;

    case 'e':
      putchar (' ');
      fwrite (sc->x.cmd_txt.text, 1, sc->x.cmd_txt.text_length, stdout);
      break;

    case 'F':
      break;

    case 'g':
      break;

    case 'G':
      break;

    case 'h':
      break;

    case 'H':
      break;

      /* 'i' is lumped above with 'a' and 'c' */

    case 'L':
    case 'l':
    case 'q':
    case 'Q':
      if (sc->x.int_arg != -1)
        printf (" %d", sc->x.int_arg);
      break;

    case 'n':
      break;

    case 'N':
      break;

    case 'P':
      break;

    case 'p':
      break;

      /* 'q','Q' are lumped above with 'L' and 'l' */

    case 'r':
      putchar (' ');
      fputs (sc->x.readcmd.fname, stdout);
      break;

    case 'R':
      putchar (' ');
      fputs (sc->x.inf->name, stdout);
      break;

    case 's':
      debug_print_subst (sc->x.cmd_subst);
      break;

      /* 't','T' are lumped above with 'b' */

    case 'v':                  /* LCOV_EXCL_LINE */
      /* should not happen - handled during compilation then discarded. */
      assert (0);               /* LCOV_EXCL_LINE */

    case 'W':
      debug_print_output_file (sc->x.outf);
      break;

    case 'w':
      debug_print_output_file (sc->x.outf);
      break;

    case 'x':
      break;

    case 'y':
      debug_print_translation (sc);
      break;

    case 'z':
      break;

    default:                   /* LCOV_EXCL_LINE */
      /* should not happen - unless missed a sed command. */
      assert (0);               /* LCOV_EXCL_LINE */
    }
}

void
debug_print_command (const struct vector *program, const struct sed_cmd *sc)
{
  bool addr_bang;
  if (!program)
    return;

  if (sc->cmd == '}')
    --block_level;

  for (int j = 0; j < block_level; ++j)
    fputs ("  ", stdout);

  debug_print_addr (sc->a1);
  if (sc->a2)
    putchar (',');
  debug_print_addr (sc->a2);

  addr_bang = sc->addr_bang;
  /* Implmentation detail: GNU Sed implements beginning of block
     by negating the matched address and jumping if there's no match.  */
  if (sc->cmd == '{')
    addr_bang = !addr_bang;
  if (addr_bang)
    putchar ('!');

  if (sc->a1 || sc->a2)
    putchar (' ');

  debug_print_function (program, sc);

  putchar ('\n');

  if (sc->cmd == '{')
    ++block_level;
}

void
debug_print_program (const struct vector *program)
{
  if (!program)
    return;

  block_level = 1;
  puts ("SED PROGRAM:");
  for (size_t i = 0; i < program->v_length; ++i)
    debug_print_command (program, &program->v[i]);
  block_level = 0;
}

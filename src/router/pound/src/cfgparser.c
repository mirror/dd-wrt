/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2024-2025 Sergey Poznyakoff
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

#include "config.h"
#include <stddef.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <syslog.h>
#include <glob.h>
#include "list.h"
#include "mem.h"
#include "cfgparser.h"
#include "cctype.h"

extern char const *progname;

/*
 * Scanner
 */

/*
 * Return a static string describing the given type.
 * Note, that in addition to the types defined above, this function returns
 * meaningful description for all possible ASCII characters.
 */
char const *
token_type_str (unsigned type)
{
  static char buf[6];
  switch (type)
    {
    case T_IDENT:
      return "identifier";

    case T_STRING:
      return "quoted string";

    case T_NUMBER:
      return "number";

    case T_LITERAL:
      return "literal";

    case '\n':
      return "end of line";

    case '\t':
      return "'\\t'";

    case '\\':
      return "'\\'";

    case '\"':
      return "'\"'";
    }

  if (c_isprint (type))
    {
      buf[0] = buf[2] = '\'';
      buf[1] = type;
      buf[3] = 0;
    }
  else if (c_iscntrl (type))
    {
      buf[0] = '^';
      buf[1] = type ^ 0100;
      buf[2] = 0;
    }
  else
    {
      buf[5] = 0;
      buf[4] = (type & 7) + '0';
      type >>= 3;
      buf[3] = (type & 7) + '0';
      type >>= 3;
      buf[2] = (type & 7) + '0';
      buf[1] = '0';
      buf[0] = '\\';
    }
  return buf;
}

size_t
token_mask_str (TOKENMASK mask, char *buf, size_t size)
{
  unsigned i = 0;
  char *q = buf, *end = buf + size - 1;

  for (i = T__BASE; i < T__END; i++)
    {
      if (mask & T_BIT (i))
	{
	  char const *s;

	  mask &= ~T_BIT (i);
	  if (q > buf)
	    {
	      if (mask)
		{
		  if (end - q <= 2)
		    break;
		  *q++ = ',';
		  *q++ = ' ';
		}
	      else
		{
		  if (end - q <= 4)
		    break;
		  strcpy (q, " or ");
		  q += 4;
		}
	    }
	  s = token_type_str (i);
	  while (*s && q < end)
	    {
	      *q++ = *s++;
	    }
	}
    }
  *q = 0;
  return q - buf;
}

int
kw_to_tok (struct kwtab *kwt, char const *name, int ci, int *retval)
{
  for (; kwt->name; kwt++)
    if ((ci ? c_strcasecmp : strcmp) (kwt->name, name) == 0)
      {
	*retval = kwt->tok;
	return 0;
      }
  return -1;
}

char const *
kw_to_str (struct kwtab *kwt, int t)
{
  for (; kwt->name; kwt++)
    if (kwt->tok == t)
      break;
  return kwt->name;
}

static void
stderr_error_msg (char const *msg)
{
  if (progname)
    fprintf (stderr, "%s: ", progname);
  fputs (msg, stderr);
  fputc ('\n', stderr);
}

void (*cfg_error_msg) (char const *msg) = stderr_error_msg;

void
stringbuf_format_locus_point (struct stringbuf *sb,
			      struct locus_point const *loc)
{
  stringbuf_printf (sb, "%s:%d", string_ptr (loc->filename), loc->line);
  if (loc->col)
    stringbuf_printf (sb, ".%d", loc->col);
}

static int
same_file (struct locus_point const *a, struct locus_point const *b)
{
  return a->filename == b->filename
	 || (a->filename && b->filename &&
	     strcmp (string_ptr (a->filename), string_ptr (b->filename)) == 0);
}

void
stringbuf_format_locus_range (struct stringbuf *sb,
			      struct locus_range const *range)
{
  stringbuf_format_locus_point (sb, &range->beg);
  if (range->end.filename)
    {
      if (!same_file (&range->beg, &range->end))
	{
	  stringbuf_add_char (sb, '-');
	  stringbuf_format_locus_point (sb, &range->end);
	}
      else if (range->beg.line != range->end.line)
	{
	  stringbuf_add_char (sb, '-');
	  stringbuf_printf (sb, "%d", range->end.line);
	  if (range->end.col)
	    stringbuf_printf (sb, ".%d", range->end.col);
	}
      else if (range->beg.col && range->beg.col != range->end.col)
	{
	  stringbuf_add_char (sb, '-');
	  stringbuf_printf (sb, "%d", range->end.col);
	}
    }
}

void
vconf_error_at_locus_range (struct locus_range const *loc,
			    char const *fmt, va_list ap)
{
  struct stringbuf sb;

  xstringbuf_init (&sb);
  if (loc)
    {
      stringbuf_format_locus_range (&sb, loc);
      stringbuf_add_string (&sb, ": ");
    }
  stringbuf_vprintf (&sb, fmt, ap);
  cfg_error_msg (sb.base);
  stringbuf_free (&sb);
}

void
conf_error_at_locus_range (struct locus_range const *loc, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vconf_error_at_locus_range (loc, fmt, ap);
  va_end (ap);
}

void
vconf_error_at_locus_point (struct locus_point const *loc,
			    char const *fmt, va_list ap)
{
  struct stringbuf sb;

  xstringbuf_init (&sb);
  if (loc)
    {
      stringbuf_format_locus_point (&sb, loc);
      stringbuf_add_string (&sb, ": ");
    }
  stringbuf_vprintf (&sb, fmt, ap);
  cfg_error_msg (sb.base);
  stringbuf_free (&sb);
}

void
conf_error_at_locus_point (struct locus_point const *loc, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vconf_error_at_locus_point (loc, fmt, ap);
  va_end (ap);
}


typedef DLIST_HEAD (, workdir) WORKDIR_HEAD;

static WORKDIR_HEAD workdir_head = DLIST_HEAD_INITIALIZER (workdir_head);

static char *
xgetcwd (void)
{
  char *buf = NULL;
  size_t size = 0;

  for (;;)
    {
      buf = x2nrealloc (buf, &size, 1);
      if (getcwd (buf, size) != NULL)
	break;
      if (errno != ERANGE)
	{
	  conf_error ("getcwd: %s", strerror (errno));
	  exit (1);
	}
    }
  return buf;
}

WORKDIR *
workdir_get (char const *name)
{
  WORKDIR *wp;
  char *cwd = NULL;
  int fd;

  if (name == NULL)
    {
      cwd = xgetcwd ();
      name = cwd;
    }

  DLIST_FOREACH (wp, &workdir_head, link)
    if (strcmp (wp->name, name) == 0)
      {
	wp->refcount++;
	free (cwd);
	return wp;
      }

  if (cwd)
    fd = AT_FDCWD;
  else if ((fd = open (name, O_RDONLY | O_NONBLOCK | O_DIRECTORY)) == -1)
    {
      int ec = errno;
      free (cwd);
      errno = ec;
      return NULL;
    }

  wp = xzalloc (sizeof (*wp) + strlen (name));
  strcpy (wp->name, name);
  wp->refcount = 1;
  wp->fd = fd;
  DLIST_PUSH (&workdir_head, wp, link);
  free (cwd);
  return wp;
}

static int
workdir_free (WORKDIR *wd)
{
  if (!wd)
    return 0;
  if (wd->refcount == 0)
    {
      DLIST_REMOVE (&workdir_head, wd, link);
      if (wd->fd != AT_FDCWD)
	close (wd->fd);
      free (wd);
      return 0;
    }
  return 1;
}

static int
workdir_cleanup (int keepwd)
{
  WORKDIR *wd, *tmp;
  int cwd = -1;
  DLIST_FOREACH_SAFE (wd, tmp, &workdir_head, link)
    {
      if (workdir_free (wd))
	{
	  if (wd->fd == AT_FDCWD && keepwd)
	    {
	      if (cwd == -1)
		{
		  int fd = openat (wd->fd, ".",
				   O_RDONLY | O_NONBLOCK | O_DIRECTORY);
		  if (fd == -1)
		    {
		      conf_error ("can't open current working directory: %s",
				  strerror (errno));
		      return -1;
		    }
		  cwd = fd;
		}
	      wd->fd = cwd;
	    }
	}
    }
  return 0;
}

char const *include_dir = SYSCONFDIR;
WORKDIR *include_wd;

WORKDIR *
get_include_wd_at_locus_range (struct locus_range const *locus)
{
  if (!include_wd)
    {
      include_wd = workdir_get (include_dir);
      if (!include_wd)
	conf_error_at_locus_range (locus,
				   "can't open include directory %s: %s",
				   include_dir,
				   strerror (errno));
    }
  return include_wd;
}

FILE *
fopen_wd (WORKDIR *wd, const char *filename)
{
  int fd;
  int dirfd = AT_FDCWD;

  if (!wd)
    wd = include_wd;
  if (wd)
    dirfd = wd->fd;
  if ((fd = openat (dirfd, filename, O_RDONLY)) == -1)
    return NULL;
  return fdopen (fd, "r");
}

FILE *
fopen_include (const char *filename)
{
  WORKDIR *wd = get_include_wd ();
  if (!wd)
    return NULL;
  return fopen_wd (wd, filename);
}

char *
filename_resolve (const char *filename)
{
  char *ret;
  if (filename[0] == '/')
    ret = xstrdup (filename);
  else
    {
      WORKDIR *wd = get_include_wd ();
      if (!wd)
	return NULL;
      ret = xmalloc (strlen (wd->name) + strlen (filename) + 2);
      strcat (strcat (strcpy (ret, wd->name), "/"), filename);
      workdir_unref (wd);
    }
  return ret;
}

void
fopen_error (int pri, int ec, WORKDIR *wd, const char *filename,
	     struct locus_range const *loc)
{
  if (filename[0] == '/' || wd == NULL)
    conf_error_at_locus_range (loc, "can't open %s: %s",
			       filename, strerror (ec));
  else
    conf_error_at_locus_range (loc, "can't open %s/%s: %s",
			       wd->name, filename, strerror (ec));
}

/*
 * Input scanner.
 */
struct input_dir
{
  struct input_dir *prev;
  WORKDIR *wd;
  glob_t *glob;
  size_t idx;
};

static struct input_dir *input_dir_tos;

static inline WORKDIR *
input_workdir (void)
{
  return input_dir_tos
	   ? input_dir_tos->wd
	   : get_include_wd_at_locus_range (last_token_locus_range ());
}

static void
input_dir_push (WORKDIR *wd, glob_t *glob)
{
  struct input_dir *dir;
  XZALLOC (dir);
  dir->wd = workdir_ref (wd);
  dir->glob = glob;
  dir->idx = 0;
  dir->prev = input_dir_tos;
  input_dir_tos = dir;
}

static void
input_dir_pop (void)
{
  struct input_dir *dir = input_dir_tos;
  input_dir_tos = dir->prev;
  workdir_unref (dir->wd);
  globfree (dir->glob);
  free (dir->glob);
  free (dir);
}

static char const *
input_dir_next_name (void)
{
  if (input_dir_tos)
    {
      while (input_dir_tos->idx < input_dir_tos->glob->gl_pathc)
	{
	  char const *name = input_dir_tos->glob->gl_pathv[input_dir_tos->idx++];
	  if (name[strlen(name)-1] != '/')
	    return name;
	}
      input_dir_pop ();
    }
  return NULL;
}

#define MAX_PUTBACK 3

/* Input stream */
struct cfginput
{
  struct cfginput *prev;          /* Previous input in stack. */

  FILE *file;                     /* Input file. */
  ino_t ino;
  dev_t devno;

  struct locus_point locus;       /* Current location */
  int prev_col;                   /* Last column in the previous line. */
  struct token token;             /* Current token. */
  struct token putback[MAX_PUTBACK]; /* Putback space */
  int putback_index;              /* Index of the next free slot in putback */

  /* Input buffer: */
  struct stringbuf buf;
};

static struct cfginput *
input_close (struct cfginput *input)
{
  struct cfginput *prev = NULL;
  if (input)
    {
      prev = input->prev;
      locus_range_unref (&input->token.locus);
      locus_point_unref (&input->locus);
      fclose (input->file);
      stringbuf_free (&input->buf);
      free (input);
    }
  return prev;
}

static struct cfginput *
input_open (char const *filename, struct stat *st)
{
  struct cfginput *input;

  XZALLOC (input);
  if ((input->file = fopen_include (filename)) == 0)
    {
      conf_error ("can't open %s: %s", filename, strerror (errno));
      free (input);
      return NULL;
    }
  input->ino = st->st_ino;
  input->devno = st->st_dev;
  locus_point_init (&input->locus, filename,
		    (include_wd != NULL && include_wd->fd != AT_FDCWD)
		    ? include_wd->name : NULL);
  return input;
}

static inline int
input_getc (struct cfginput *input)
{
  int c = fgetc (input->file);
  if (c == '\n')
    {
      input->locus.line++;
      input->prev_col = input->locus.col;
      input->locus.col = 0;
    }
  else if (c == '\t')//FIXME
    input->locus.col += 8;
  else if (c != EOF)
    input->locus.col++;
  return c;
}

static void
input_ungetc (struct cfginput *input, int c)
{
  if (c != EOF)
    {
      ungetc (c, input->file);
      if (c == '\n')
	{
	  input->locus.line--;
	  input->locus.col = input->prev_col;
	}
      else
	input->locus.col--;
    }
}

#define is_ident_start(c) (c_isalpha (c) || c == '_')
#define is_ident_cont(c) (is_ident_start (c) || c_isdigit (c))

static struct token *input_unput (struct cfginput *input);

int
input_gettkn (struct cfginput *input, struct token **tok)
{
  int c;

  stringbuf_reset (&input->buf);

  if (input->putback_index > 0)
    {
      *tok = input_unput (input);
      return input->token.type;
    }

  for (;;)
    {
      c = input_getc (input);

      if (c == EOF)
	{
	  input->token.type = c;
	  break;
	}

      if (c == '#')
	{
	  while ((c = input_getc (input)) != '\n')
	    if (c == EOF)
	      {
		input->token.type = c;
		goto end;
	      }
	  /* return newline */
	}

      if (c == '\n')
	{
	  locus_point_copy (&input->token.locus.beg, &input->locus);
	  input->token.locus.beg.line--;
	  input->token.locus.beg.col = input->prev_col;
	  input->token.type = c;
	  break;
	}

      if (c_isspace (c))
	continue;

      locus_point_copy (&input->token.locus.beg, &input->locus);
      if (c == '"')
	{
	  while ((c = input_getc (input)) != '"')
	    {
	      if (c == '\\')
		{
		  c = input_getc (input);
		  switch (c)
		    {
		    case '"':
		    case '\\':
		      break;

		    case EOF:
		      conf_error_at_locus_point (&input->locus,
						 "end of file in quoted string");
		      input->token.type = T_ERROR;
		      goto end;

		    case '\n':
		      conf_error_at_locus_point (&input->locus,
						 "end of line in quoted string");
		      input->token.type = T_ERROR;
		      goto end;

		    default:
		      conf_error_at_locus_point (&input->locus,
						 "unrecognized escape character");
		    }
		}
	      stringbuf_add_char (&input->buf, c);
	    }
	  input->token.type = T_STRING;
	  input->token.str = stringbuf_finish (&input->buf);
	  break;
	}

      if (is_ident_start (c))
	{
	  do
	    {
	      stringbuf_add_char (&input->buf, c);
	    }
	  while ((c = input_getc (input)) != EOF && is_ident_cont (c));
	  if (c == EOF || c_isspace (c))
	    {
	      input_ungetc (input, c);
	      input->token.type = T_IDENT;
	      input->token.str = stringbuf_finish (&input->buf);
	      break;
	    }
	  /* It is a literal */
	}

      if (c_isdigit (c))
	input->token.type = T_NUMBER;
      else
	input->token.type = T_LITERAL;

      do
	{
	  stringbuf_add_char (&input->buf, c);
	  if (!c_isdigit (c))
	    input->token.type = T_LITERAL;
	}
      while ((c = input_getc (input)) != EOF && !c_isspace (c));

      input_ungetc (input, c);
      input->token.str = stringbuf_finish (&input->buf);
      break;
    }
 end:
  locus_point_copy (&input->token.locus.end, &input->locus);
  *tok = &input->token;
  return input->token.type;
}

static void
input_putback (struct cfginput *input, struct token *tok)
{
  assert (input->putback_index < MAX_PUTBACK);
  locus_range_ref (&tok->locus);
  input->putback[input->putback_index] = *tok;
  if (tok->type >= T__BASE && tok->type < T__END)
    input->putback[input->putback_index].str = tok->str;
  else
    input->putback[input->putback_index].str = NULL;
  input->putback_index++;
}

static struct token *
input_unput (struct cfginput *input)
{
  if (input->putback_index > 0)
    {
      locus_range_unref (&input->token.locus);
      input->token = input->putback[--input->putback_index];
      return &input->token;
    }
  return NULL;
}


struct cfginput *cur_input;

static inline struct token *
cur_token (void)
{
  return &cur_input->token;
}

struct locus_range const *
last_token_locus_range (void)
{
  if (cur_input)
    return &cur_token()->locus;
  else
    return NULL;
}

static int
globat (int wd, const char *restrict pattern, int flags,
	int (*errfunc)(const char *epath, int eerrno),
	glob_t *restrict pglob)
{
  int curfd;
  int ret;

  if (wd == AT_FDCWD)
    curfd = AT_FDCWD;
  else
    {
      curfd = openat (AT_FDCWD, ".", O_DIRECTORY | O_RDONLY | O_NDELAY);
      if (curfd == -1)
	return GLOB_ABORTED;

      if (fchdir (wd))
	{
	  close (curfd);
	  return GLOB_ABORTED;
	}
    }

  ret = glob (pattern, flags, errfunc, pglob);

  if (curfd != AT_FDCWD)
    {
      if (fchdir (curfd))
	{
	  int ec = errno;
	  globfree (pglob);
	  close (curfd);
	  errno = ec;
	  return -1;
	}
      close (curfd);
    }
  return ret;
}

static int
push_input (WORKDIR *wd, const char *filename)
{
  struct stat st;
  struct cfginput *input;

  if (fstatat (wd->fd, filename, &st, 0))
    {
      if (errno == ENOENT)
	{
	  int rc;
	  glob_t *glob;
	  XZALLOC (glob);
	  rc = globat (wd->fd, filename, GLOB_ERR | GLOB_MARK, NULL, glob);
	  if (rc == 0)
	    {
	      input_dir_push (wd, glob);
	      return push_input (input_workdir (), input_dir_next_name ());
	    }
	  errno = ENOENT;
	}

      if (filename[0] == '/')
	conf_error ("can't stat %s: %s", filename, strerror (errno));
      else
	conf_error ("can't stat %s/%s: %s", wd->name, filename,
		    strerror (errno));
      return -1;
    }

  for (input = cur_input; input; input = input->prev)
    {
      if (input->ino == st.st_ino && input->devno == st.st_dev)
	{
	  if (input->prev)
	    {
	      conf_error ("%s already included", filename);
	      conf_error_at_locus_point (&input->prev->locus,
					 "here is the location of original inclusion");
	    }
	  else
	    {
	      conf_error ("%s already included (at top level)", filename);
	    }
	  return -1;
	}
    }

  if ((input = input_open (filename, &st)) == NULL)
    return -1;

  input->prev = cur_input;
  cur_input = input;

  return 0;
}

static void
pop_input (void)
{
  char const *name;
  cur_input = input_close (cur_input);
  if ((name = input_dir_next_name ()) != NULL)
    push_input (input_workdir (), name);
}

static int
gettkn (struct token **tok)
{
  int t = EOF;

  while (cur_input && (t = input_gettkn (cur_input, tok)) == EOF)
    pop_input ();
  return t;
}

struct token *
gettkn_expect_mask (int expect)
{
  struct token *tok;
  int type = gettkn (&tok);

  if (type == EOF)
    {
      conf_error ("%s", "unexpected end of file");
      tok = NULL;
    }
  else if (type == T_ERROR)
    {
      /* error message already issued */
      tok = NULL;
    }
  else if (expect == 0)
    /* any token is accepted */;
  else if (!T_MASK_ISSET (expect, type))
    {
      char buf[MAX_TOKEN_BUF_SIZE];
      token_mask_str (expect, buf, sizeof (buf));
      conf_error ("expected %s, but found %s", buf,
		  token_type_str (tok->type));
      tok = NULL;
    }
  return tok;
}

struct token *
gettkn_any (void)
{
  return gettkn_expect_mask (T_ANY);
}

struct token *
gettkn_expect (int type)
{
  return gettkn_expect_mask (T_BIT (type));
}

void
putback_tkn (struct token *tok)
{
  input_putback (cur_input, tok ? tok : cur_token ());
}

void
putback_synth (int type, char const *str, struct locus_range *loc)
{
  struct token tok;
  if (str)
    {
      stringbuf_reset (&cur_input->buf);
      stringbuf_add_string (&cur_input->buf, str);
    }
  tok.type = type;
  tok.str = stringbuf_finish (&cur_input->buf);
  locus_range_init (&tok.locus);
  if (loc)
    locus_range_copy (&tok.locus, loc);
  putback_tkn (&tok);
}

/*
 * Read from the input all material up to "End" (case-insensitive) on a
 * line by itself.  Leave the material in input->buf.  Return last character
 * read.
 */
int
cfg_read_to_end (struct cfginput *input, char **ptr)
{
  int c;
  struct locus_range range;
  struct token *tok;

  range.beg = input->locus;

  stringbuf_reset (&input->buf);

  /* Drain putback */
  while ((tok = input_unput (input)) != NULL)
    {
      stringbuf_add_string (&input->buf, tok->str);
      free (tok->str);
      if (input->putback_index > 0)
	stringbuf_add_char (&input->buf, ' ');
    }

  for (;;)
    {
      c = input_getc (input);
      if (c == EOF)
	{
	  range.end = input->locus;
	  conf_error_at_locus_range (&range, "%s",
				     "unexpected end of file");
	  break;
	}
      if (c == '\n')
	{
	  char *start = stringbuf_value (&input->buf);
	  char *end = start + stringbuf_len (&input->buf);
	  char *line;
	  size_t linelen, len;

	  for (line = end - 1; line > start; line--)
	    {
	      if (*line == '\n')
		{
		  ++line;
		  break;
		}
	    }

	  len = linelen = end - line;
	  line = c_trimws (line, &len);

	  if (len == 3 && c_strncasecmp (line, "end", 3) == 0)
	    {
	      stringbuf_truncate (&input->buf, stringbuf_len (&input->buf) -
				  linelen);
	      break;
	    }
	}
      stringbuf_add_char (&input->buf, c);
    }
  *ptr = stringbuf_finish (&input->buf);
  return c;
}

/*
 * Find in TAB an entry describing the keyword NAME.  If the keyword is an
 * alias to another one, return the aliased keyword, and place in *REF a
 * pointer to the entry describing the alias.  Otherwise, initialize *REF to
 * NULL.
 *
 * Instead of returning a pointer to the TAB entry itself, copy it to *BUF
 * first and return BUF.
 */
static CFGPARSER_TABLE *
parser_find (CFGPARSER_TABLE *tab, char const *name, CFGPARSER_TABLE *buf,
	     CFGPARSER_TABLE **ref)
{
  CFGPARSER_TABLE *p;

  *ref = NULL;
  p = tab;
  if (p->type == KWT_TOPLEVEL)
    p++;
  for (; p->name; p++)
    {
      if (p->type == KWT_TOPLEVEL)
	continue;
      else if (p->type == KWT_TABREF)
	{
	  CFGPARSER_TABLE *result = parser_find (p->ref, name, buf, ref);
	  if (result)
	    return result;
	}
      else if (p->type == KWT_SOFTREF)
	{
	  CFGPARSER_TABLE *result = parser_find (p->ref, name, buf, ref);
	  if (result)
	    {
	      result->data = p->data;
	      result->off = p->off;
	      return result;
	    }
	}
      else if (p->type == KWT_WILDCARD || c_strcasecmp (p->name, name) == 0)
	{
	  *ref = p;
	  if (p->type == KWT_ALIAS)
	    {
	      while (p > tab && p->type == KWT_ALIAS)
		p--;
	      assert (p->type == KWT_REG);
	    }
	  *buf = *p;
	  return buf;
	}
    }
  return NULL;
}

CFGPARSER_TABLE cfg_global_parsetab[] = {
  {
    .name = "Include",
    .parser = cfg_parse_include
  },
  { NULL }
};

int
cfgparser0 (CFGPARSER_TABLE *ptab, void *call_data, void *section_data,
	    int single_statement,
	    enum deprecation_mode handle_deprecated,
	    struct locus_range *retrange)
{
  struct token *tok;

  locus_point_copy (&retrange->beg, &last_token_locus_range ()->beg);
  locus_point_init (&retrange->end, NULL, NULL);

  for (;;)
    {
      int type = gettkn (&tok);

      if (type == EOF)
	{
	  if (ptab[0].type == KWT_TOPLEVEL)
	    goto end;
	  else
	    {
	      conf_error_at_locus_point (&retrange->beg,
					 "unexpected end of file");
	      return CFGPARSER_FAIL;
	    }
	}
      else if (type == T_ERROR)
	return CFGPARSER_FAIL;

      if (retrange)
	locus_point_copy (&retrange->end, &last_token_locus_range ()->end);

      if (tok->type == T_IDENT)
	{
	  CFGPARSER_TABLE buf, *ref, *ent = parser_find (ptab, tok->str,
							 &buf, &ref);

	  if (!single_statement && ent == NULL)
	    ent = parser_find (cfg_global_parsetab, tok->str, &buf, &ref);

	  if (ref && ref->deprecated)
	    {
	      switch (handle_deprecated)
		{
		case DEPREC_OK:
		  break;

		case DEPREC_WARN:
		  if (ent->message)
		    conf_error ("warning: deprecated statement, %s",
				ref->message);
		  else
		    conf_error ("warning: deprecated statement,"
				" use \"%s\" instead", ent->name);
		  break;

		case DEPREC_ERR:
		  if (ent->message)
		    conf_error ("deprecated statement, %s", ref->message);
		  else
		    conf_error ("deprecated statement,"
				" use \"%s\" instead", ent->name);
		  return CFGPARSER_FAIL;
		}
	    }

	  if (ent)
	    {
	      void *data = ent->data ? ent->data : call_data;

	      switch (ent->parser ((char*)data + ent->off, section_data))
		{
		case CFGPARSER_OK:
		  type = gettkn (&tok);
		  if (type == T_ERROR)
		    return CFGPARSER_FAIL;
		  if (type != '\n' && type != EOF)
		    {
		      conf_error ("unexpected %s", token_type_str (type));
		      return CFGPARSER_FAIL;
		    }
		  if (single_statement)
		    return CFGPARSER_OK_NONL;
		  break;

		case CFGPARSER_OK_NONL:
		  if (single_statement)
		    return CFGPARSER_OK_NONL;
		  continue;

		case CFGPARSER_FAIL:
		  return CFGPARSER_FAIL;

		case CFGPARSER_END:
		  goto end;
		}
	    }
	  else
	    {
	      conf_error_at_locus_range (&tok->locus, "unrecognized keyword");
	      return CFGPARSER_FAIL;
	    }
	}
      else if (tok->type == '\n')
	continue;
      else
	conf_error_at_locus_range (&tok->locus, "syntax error");
    }
 end:
  return CFGPARSER_OK;
}

int
cfgparser (CFGPARSER_TABLE *ptab, void *call_data, void *section_data,
	   int single_statement,
	   enum deprecation_mode handle_deprecated,
	   struct locus_range *retrange)
{
  struct locus_range range = LOCUS_RANGE_INITIALIZER;
  int rc = cfgparser0 (ptab, call_data, section_data, single_statement,
		       handle_deprecated, &range);
  if (retrange)
    locus_range_copy (retrange, &range);
  locus_range_unref (&range);
  return rc;
}

int
cfgparser_open (char const *filename, char const *wd)
{
  int rc;

  if ((include_wd = workdir_get (wd)) == NULL)
    {
      conf_error ("can't open cwd: %s", strerror (errno));
      return -1;
    }
  rc = push_input (include_wd, filename);

  /* Make sure an attempt to open include_dir will be made if needed. */
  workdir_unref (include_wd);
  include_wd = NULL;

  return rc;
}

int
cfgparser_finish (int keepwd)
{
  workdir_unref (include_wd);
  if (include_wd && include_wd->refcount == 0)
    include_wd = NULL;
  /* Remove unreferenced wd's and resolve CWD */
  return workdir_cleanup (keepwd);
}

int
cfgparser_parse (char const *filename, char const *wd,
		 CFGPARSER_TABLE *tab, void *data,
		 enum deprecation_mode handle_deprecated, int keepwd)
{
  int rc;

  if (cfgparser_open (filename, wd))
    return -1;
  rc = cfgparser_loop (tab, data, data, handle_deprecated, NULL);
  if (rc == 0)
    {
      if (cur_input)
	rc = -1;
    }
  if (cfgparser_finish (keepwd))
    rc = -1;
  return rc;
}

int
cfg_parse_includedir (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  WORKDIR *wd;
  if (!tok)
    return CFGPARSER_FAIL;
  if ((wd = workdir_get (tok->str)) == NULL)
    {
      conf_error ("can't open directory %s: %s", tok->str, strerror (errno));
      return CFGPARSER_FAIL;
    }
  workdir_free (include_wd);
  include_wd = wd;
  return CFGPARSER_OK;
}

int
cfg_parse_include (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return CFGPARSER_FAIL;
  if (push_input (get_include_wd_at_locus_range (&tok->locus), tok->str))
    return CFGPARSER_FAIL;
  return CFGPARSER_OK_NONL;
}

int
cfg_parse_end (void *call_data, void *section_data)
{
  return CFGPARSER_END;
}

int
cfg_int_set_one (void *call_data, void *section_data)
{
  *(int*)call_data = 1;
  return CFGPARSER_OK;
}

int
cfg_assign_string (void *call_data, void *section_data)
{
  char *s;
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return CFGPARSER_FAIL;
  s = xstrdup (tok->str);
  *(char**)call_data = s;
  return CFGPARSER_OK;
}

int
cfg_assign_string_from_file (void *call_data, void *section_data)
{
  struct stat st;
  char *s;
  FILE *fp;
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return CFGPARSER_FAIL;
  if ((fp = fopen_include (tok->str)) == NULL)
    {
      fopen_error (LOG_ERR, errno, include_wd, tok->str, &tok->locus);
      return CFGPARSER_FAIL;
    }
  if (fstat (fileno (fp), &st))
    {
      conf_error ("can't stat %s: %s", tok->str, strerror (errno));
      return CFGPARSER_FAIL;
    }
  if (!S_ISREG (st.st_mode))
    {
      conf_error ("%s: not a regular file", tok->str);
      return CFGPARSER_FAIL;
    }
  if (st.st_size == 0)
    {
      conf_error ("%s: empty file", tok->str);
      return CFGPARSER_FAIL;
    }
  // FIXME: Check st_size upper bound?
  s = xmalloc (st.st_size + 1);
  if (fread (s, st.st_size, 1, fp) != 1)
    {
      conf_error ("%s: read error: %s", tok->str, strerror (errno));
      return CFGPARSER_FAIL;
    }
  s[st.st_size] = 0;
  fclose (fp);
  *(char**)call_data = s;
  return CFGPARSER_OK;
}

int
cfg_assign_bool (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect_mask (T_UNQ);

  if (!tok)
    return CFGPARSER_FAIL;

  if (strcmp (tok->str, "1") == 0 ||
      strcmp (tok->str, "yes") == 0 ||
      strcmp (tok->str, "true") == 0 ||
      strcmp (tok->str, "on") == 0)
    *(int *)call_data = 1;
  else if (strcmp (tok->str, "0") == 0 ||
	   strcmp (tok->str, "no") == 0 ||
	   strcmp (tok->str, "false") == 0 ||
	   strcmp (tok->str, "off") == 0)
    *(int *)call_data = 0;
  else
    {
      conf_error ("%s", "not a boolean value");
      conf_error ("valid booleans are: %s for true value, and %s for false value",
		  "1, yes, true, on",
		  "0, no, false, off");
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

int
cfg_assign_unsigned_min (unsigned *dst, unsigned minval, int quiet)
{
  unsigned long n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return CFGPARSER_FAIL;

  errno = 0;
  n = strtoul (tok->str, &p, 10);
  if (errno || *p || n > UINT_MAX)
    {
      conf_error ("%s", "bad unsigned number");
      return CFGPARSER_FAIL;
    }
  if (n < minval)
    {
      if (quiet)
	n = minval;
      else
	{
	  conf_error ("value out of allowed range (>= %lu)", minval);
	  return CFGPARSER_FAIL;
	}
    }
  *dst = n;
  return 0;
}

int
cfg_assign_unsigned (void *call_data, void *section_data)
{
  return cfg_assign_unsigned_min (call_data, 0, 0);
}

int
cfg_assign_int (void *call_data, void *section_data)
{
  long n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return CFGPARSER_FAIL;

  errno = 0;
  n = strtol (tok->str, &p, 10);
  if (errno || *p || n < INT_MIN || n > INT_MAX)
    {
      conf_error ("%s", "bad integer number");
      return CFGPARSER_FAIL;
    }
  *(int *)call_data = n;
  return 0;
}

int
cfg_assign_int_range (int *dst, int min, int max)
{
  int n;
  int rc;

  if ((rc = cfg_assign_int (&n, NULL)) != CFGPARSER_OK)
    return rc;

  if ((min >= 0 && n < min) || (max > 0 && n > max))
    {
      if (min < 0)
	conf_error ("value out of allowed range (<= %d)", max);
      else if (max < 0)
	conf_error ("value out of allowed range (>= %d)", min);
      else
	conf_error ("value out of allowed range (%d..%d)", min, max);
      return CFGPARSER_FAIL;
    }
  *dst = n;
  return CFGPARSER_OK;
}

int
cfg_assign_mode (void *call_data, void *section_data)
{
  long n;
  char *end;
  struct token *tok = gettkn_expect (T_NUMBER);

  errno = 0;
  n = strtoul (tok->str, &end, 8);
  if (errno || *end || n > 0777)
    {
      conf_error_at_locus_range (&tok->locus, "%s", "invalid file mode");
      return CFGPARSER_FAIL;
    }
  *(mode_t*)call_data = n;
  return CFGPARSER_OK;
}

int
cfg_assign_int_enum (int *dst, struct token *tok, struct kwtab *kwtab,
		     char *what)
{
  if (tok == NULL)
    return CFGPARSER_FAIL;

  if (kw_to_tok (kwtab, tok->str, 0, dst))
    {
      conf_error ("unrecognized %s", what);
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static struct kwtab facility_table[] = {
  { "auth", LOG_AUTH },
#ifdef  LOG_AUTHPRIV
  { "authpriv", LOG_AUTHPRIV },
#endif
  { "cron", LOG_CRON },
  { "daemon", LOG_DAEMON },
#ifdef  LOG_FTP
  { "ftp", LOG_FTP },
#endif
  { "kern", LOG_KERN },
  { "lpr", LOG_LPR },
  { "mail", LOG_MAIL },
  { "news", LOG_NEWS },
  { "syslog", LOG_SYSLOG },
  { "user", LOG_USER },
  { "uucp", LOG_UUCP },
  { "local0", LOG_LOCAL0 },
  { "local1", LOG_LOCAL1 },
  { "local2", LOG_LOCAL2 },
  { "local3", LOG_LOCAL3 },
  { "local4", LOG_LOCAL4 },
  { "local5", LOG_LOCAL5 },
  { "local6", LOG_LOCAL6 },
  { "local7", LOG_LOCAL7 },
  { NULL }
};

int
cfg_assign_log_facility (void *call_data, void *section_data)
{
  int n;
  struct token *tok = gettkn_expect_mask (T_UNQ);

  if (!tok)
    return CFGPARSER_FAIL;

  if (strcmp (tok->str, "-") == 0)
    n = -1;
  else if (kw_to_tok (facility_table, tok->str, 1, &n) != 0)
    {
      conf_error ("%s", "unknown log facility name");
      return CFGPARSER_FAIL;
    }
  *(int*)call_data = n;

  return CFGPARSER_OK;
}

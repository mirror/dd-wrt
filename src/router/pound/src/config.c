/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2024 Sergey Poznyakoff
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
#include "extern.h"
#include <openssl/x509v3.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>


/*
 * Scanner
 */

/* Token types: */
enum
  {
    T__BASE = 256,
    T_IDENT = T__BASE, /* Identifier */
    T_NUMBER,          /* Decimal number */
    T_STRING,          /* Quoted string */
    T_LITERAL,         /* Unquoted literal */
    T__END,
    T_ERROR = T__END,  /* Erroneous or malformed token */
  };

typedef unsigned TOKENMASK;

#define T_BIT(t) ((TOKENMASK)1<<((t)-T__BASE))
#define T_MASK_ISSET(m,t) ((m) & T_BIT(t))
#define T_ANY 0 /* any token, including newline */
/* Unquoted character sequence */
#define T_UNQ (T_BIT (T_IDENT) | T_BIT (T_NUMBER) | T_BIT (T_LITERAL))

/* Token structure */
struct token
{
  int type;
  char *str;
  struct locus_range locus;
};

/*
 * Return a static string describing the given type.
 * Note, that in addition to the types defined above, this function returns
 * meaningful description for all possible ASCII characters.
 */
static char const *
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

  if (isprint (type))
    {
      buf[0] = buf[2] = '\'';
      buf[1] = type;
      buf[3] = 0;
    }
  else if (iscntrl (type))
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

static size_t
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

/*
 * Buffer size for token buffer used as input to token_mask_str.  This takes
 * into account only T_.* types above, as returned by token_type_str.
 *
 * Be sure to update this constant if you change anything above.
 */
#define MAX_TOKEN_BUF_SIZE 45


struct kwtab
{
  char const *name;
  int tok;
};

static int
kw_to_tok (struct kwtab *kwt, char const *name, int ci, int *retval)
{
  for (; kwt->name; kwt++)
    if ((ci ? strcasecmp : strcmp) (kwt->name, name) == 0)
      {
	*retval = kwt->tok;
	return 0;
      }
  return -1;
}

static char const *
kw_to_str (struct kwtab *kwt, int t)
{
  for (; kwt->name; kwt++)
    if (kwt->tok == t)
      break;
  return kwt->name;
}

#define MAX_PUTBACK 3

/* Input stream */
struct input
{
  struct input *prev;             /* Previous input in stack. */

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

static void
stringbuf_format_locus_point (struct stringbuf *sb, struct locus_point const *loc)
{
  stringbuf_printf (sb, "%s:%d", loc->filename, loc->line);
  if (loc->col)
    stringbuf_printf (sb, ".%d", loc->col);
}

static int
same_file (struct locus_point const *a, struct locus_point const *b)
{
  return a->filename == b->filename
	 || (a->filename && b->filename && strcmp (a->filename, b->filename) == 0);
}

static void
stringbuf_format_locus_range (struct stringbuf *sb, struct locus_range const *range)
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

static void
vconf_error_at_locus_range (struct locus_range const *loc, char const *fmt, va_list ap)
{
  struct stringbuf sb;

  xstringbuf_init (&sb);
  if (loc)
    {
      stringbuf_format_locus_range (&sb, loc);
      stringbuf_add_string (&sb, ": ");
    }
  stringbuf_vprintf (&sb, fmt, ap);
  logmsg (LOG_ERR, "%s", sb.base);
  stringbuf_free (&sb);
}

static void
conf_error_at_locus_range (struct locus_range const *loc, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vconf_error_at_locus_range (loc, fmt, ap);
  va_end (ap);
}

static void
vconf_error_at_locus_point (struct locus_point const *loc, char const *fmt, va_list ap)
{
  struct stringbuf sb;

  xstringbuf_init (&sb);
  if (loc)
    {
      stringbuf_format_locus_point (&sb, loc);
      stringbuf_add_string (&sb, ": ");
    }
  stringbuf_vprintf (&sb, fmt, ap);
  logmsg (LOG_ERR, "%s", sb.base);
  stringbuf_free (&sb);
}

static void
conf_error_at_locus_point (struct locus_point const *loc, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vconf_error_at_locus_point (loc, fmt, ap);
  va_end (ap);
}

static void
regcomp_error_at_locus_range (struct locus_range const *loc, int rc, regex_t *rx,
			      char const *expr)
{
  char errbuf[512];
  regerror (rc, rx, errbuf, sizeof (errbuf));
  conf_error_at_locus_range (loc, "%s", errbuf);
  if (expr)
    conf_error_at_locus_range (loc, "regular expression: %s", expr);
}

static void
openssl_error_at_locus_range (struct locus_range const *loc,
			      char const *filename, char const *msg)
{
  unsigned long n = ERR_get_error ();
  if (filename)
    conf_error_at_locus_range (loc, "%s: %s: %s", filename, msg,
			       ERR_error_string (n, NULL));
  else
    conf_error_at_locus_range (loc, "%s: %s", msg, ERR_error_string (n, NULL));

  if ((n = ERR_get_error ()) != 0)
    {
      do
	{
	  conf_error_at_locus_range (loc, "%s", ERR_error_string (n, NULL));
	}
      while ((n = ERR_get_error ()) != 0);
    }
}

struct name_list
{
  struct name_list *next;
  char name[1];
};

static struct name_list *name_list;

static char const *
pathname_alloc (char const *dir, char const *name)
{
  struct name_list *np;
  size_t dirlen = 0;

  /* Ignore the directory if the filename is absolute. */
  if (name[0] == '/')
    dir = NULL;

  if (dir)
    dirlen = strlen (dir) + 1;

  np = xmalloc (sizeof (*np) + dirlen + strlen (name));
  if (dir)
    {
      strcpy (np->name, dir);
      np->name[dirlen-1] = '/';
    }
  strcpy (np->name + dirlen, name);
  np->next = name_list;
  name_list = np;
  return np->name;
}

//FIXME
#if 0
static char const *
name_alloc (char const *name)
{
  struct name_list *np;
  np = xmalloc (sizeof (*np) + strlen (name));
  strcpy (np->name, name);
  np->next = name_list;
  name_list = np;
  return np->name;
}

static void
name_list_free (void)
{
  while (name_list)
    {
      struct name_list *next = name_list->next;
      free (name_list);
      name_list = next;
    }
}
#endif

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
	  logmsg (LOG_CRIT, "getcwd: %s", strerror (errno));
	  exit (1);
	}
    }
  return buf;
}

static WORKDIR *
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
      free (cwd);
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

static inline WORKDIR *
workdir_ref (WORKDIR *wd)
{
  wd->refcount++;
  return wd;
}

static inline void
workdir_unref (WORKDIR *wd)
{
  assert (wd->refcount > 0);
  wd->refcount--;
}

static void
workdir_free (WORKDIR *wd)
{
  if (wd->refcount == 0)
    {
      DLIST_REMOVE (&workdir_head, wd, link);
      if (wd->fd != AT_FDCWD)
	close (wd->fd);
      free (wd);
    }
}

static void
workdir_cleanup (void)
{
  WORKDIR *wp, *tmp;
  DLIST_FOREACH_SAFE (wp, tmp, &workdir_head, link)
    {
      workdir_free (wp);
    }
}

static char const *include_dir = SYSCONFDIR;
static WORKDIR *include_wd;

static void
close_include_dir (void)
{
  if (include_wd)
    {
      workdir_free (include_wd);
      include_wd = NULL;
    }
}

static WORKDIR *
open_include_dir (char const *dir)
{
  close_include_dir ();
  return include_wd = workdir_get (dir);
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

static FILE *
fopen_include (const char *filename)
{
  return fopen_wd (include_wd, filename);
}

void
fopen_error (int pri, int ec, WORKDIR *wd, const char *filename,
	     struct locus_range *loc)
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
static struct input *
input_close (struct input *input)
{
  struct input *prev = NULL;
  if (input)
    {
      prev = input->prev;
      fclose (input->file);
      stringbuf_free (&input->buf);
      free (input);
    }
  return prev;
}

static struct input *
input_open (char const *filename, struct stat *st)
{
  struct input *input;

  input = xmalloc (sizeof (*input));
  memset (input, 0, sizeof (*input));
  if ((input->file = fopen_include (filename)) == 0)
    {
      logmsg (LOG_ERR, "can't open %s: %s", filename, strerror (errno));
      free (input);
      return NULL;
    }
  input->ino = st->st_ino;
  input->devno = st->st_dev;
  if (include_wd == NULL || include_wd->fd == AT_FDCWD)
    input->locus.filename = xstrdup (filename);
  else
    input->locus.filename = pathname_alloc (include_wd->name, filename);
  input->locus.line = 1;
  input->locus.col = 0;
  return input;
}

static inline int
input_getc (struct input *input)
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
input_ungetc (struct input *input, int c)
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

#define is_ident_start(c) (isalpha (c) || c == '_')
#define is_ident_cont(c) (is_ident_start (c) || isdigit (c))

int
input_gettkn (struct input *input, struct token **tok)
{
  int c;

  stringbuf_reset (&input->buf);

  if (input->putback_index > 0)
    {
      input->token = input->putback[--input->putback_index];
      if (input->token.str != NULL)
	{
	  stringbuf_add_string (&input->buf, input->token.str);
	  free (input->token.str);
	  input->token.str = stringbuf_finish (&input->buf);
	}
      *tok = &input->token;
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
	  input->token.locus.beg = input->locus;
	  input->token.locus.beg.line--;
	  input->token.locus.beg.col = input->prev_col;
	  input->token.type = c;
	  break;
	}

      if (isspace (c))
	continue;

      input->token.locus.beg = input->locus;
      if (c == '"')
	{
	  while ((c = input_getc (input)) != '"')
	    {
	      if (c == '\\')
		{
		  c = input_getc (input);
		  if (!(c == EOF || c == '"' || c == '\\'))
		    {
		      conf_error_at_locus_point (&input->locus,
						 "unrecognized escape character");
		    }
		}
	      if (c == EOF)
		{
		  conf_error_at_locus_point (&input->locus,
					     "end of file in quoted string");
		  input->token.type = T_ERROR;
		  goto end;
		}
	      if (c == '\n')
		{
		  conf_error_at_locus_point (&input->locus,
					     "end of line in quoted string");
		  input->token.type = T_ERROR;
		  goto end;
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
	  if (c == EOF || isspace (c))
	    {
	      input_ungetc (input, c);
	      input->token.type = T_IDENT;
	      input->token.str = stringbuf_finish (&input->buf);
	      break;
	    }
	  /* It is a literal */
	}

      if (isdigit (c))
	input->token.type = T_NUMBER;
      else
	input->token.type = T_LITERAL;

      do
	{
	  stringbuf_add_char (&input->buf, c);
	  if (!isdigit (c))
	    input->token.type = T_LITERAL;
	}
      while ((c = input_getc (input)) != EOF && !isspace (c));

      input_ungetc (input, c);
      input->token.str = stringbuf_finish (&input->buf);
      break;
    }
 end:
  input->token.locus.end = input->locus;
  *tok = &input->token;
  return input->token.type;
}

static void
input_putback (struct input *input, struct token *tok)
{
  assert (input->putback_index < MAX_PUTBACK);
  input->putback[input->putback_index] = *tok;
  if (tok->type >= T__BASE && tok->type < T__END)
    input->putback[input->putback_index].str = xstrdup (tok->str);
  else
    input->putback[input->putback_index].str = NULL;
  input->putback_index++;
}


struct input *cur_input;

static inline struct token *
cur_token (void)
{
  return &cur_input->token;
}

static struct locus_range *
last_token_locus_range (void)
{
  if (cur_input)
    return &cur_token()->locus;
  else
    return NULL;
}

#define conf_error(fmt, ...) \
  conf_error_at_locus_range (last_token_locus_range (), fmt, __VA_ARGS__)

#define conf_regcomp_error(rc, rx, expr) \
  regcomp_error_at_locus_range (last_token_locus_range (), rc, rx, expr)

#define conf_openssl_error(file, msg)				\
  openssl_error_at_locus_range (last_token_locus_range (), file, msg)

static int
push_input (const char *filename)
{
  struct stat st;
  struct input *input;

  if (fstatat (include_wd->fd, filename, &st, 0))
    {
      if (include_wd->fd == AT_FDCWD)
	conf_error ("can't stat %s: %s", filename, strerror (errno));
      else
	conf_error ("can't stat %s/%s: %s", include_wd->name, filename,
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
	      conf_error_at_locus_point (&input->prev->locus, "here is the place of original inclusion");
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
  cur_input = input_close (cur_input);
}

static int
gettkn (struct token **tok)
{
  int t;

  while (cur_input && (t = input_gettkn (cur_input, tok)) == EOF)
    pop_input ();
  return t;
}

static struct token *
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
      conf_error ("expected %s, but found %s", buf, token_type_str (tok->type));
      tok = NULL;
    }
  return tok;
}

static struct token *
gettkn_any (void)
{
  return gettkn_expect_mask (T_ANY);
}

static struct token *
gettkn_expect (int type)
{
  return gettkn_expect_mask (T_BIT (type));
}

static void
putback_tkn (struct token *tok)
{
  input_putback (cur_input, tok ? tok : cur_token ());
}

enum
  {
    PARSER_OK,
    PARSER_OK_NONL,
    PARSER_FAIL,
    PARSER_END
  };

typedef int (*PARSER) (void *, void *);

typedef struct parser_table
{
  char *name;
  PARSER parser;
  void *data;
  size_t off;
} PARSER_TABLE;

static PARSER_TABLE *
parser_find (PARSER_TABLE *tab, char const *name)
{
  for (; tab->name; tab++)
    if (strcasecmp (tab->name, name) == 0)
      return tab;
  return NULL;
}

static int parse_include (void *call_data, void *section_data);

static PARSER_TABLE global_parsetab[] = {
  { "Include", parse_include },
  { NULL }
};

static int
parse_statement (PARSER_TABLE *ptab, void *call_data, void *section_data,
		 int single_statement, struct locus_range *retrange)
{
  struct token *tok;

  if (retrange)
    {
      retrange->beg = last_token_locus_range ()->beg;
    }

  for (;;)
    {
      int type = gettkn (&tok);

      if (type == EOF)
	{
	  if (retrange)
	    {
	      conf_error_at_locus_point (&retrange->beg, "unexpected end of file");
	      return PARSER_FAIL;
	    }
	  goto end;
	}
      else if (type == T_ERROR)
	return PARSER_FAIL;

      if (retrange)
	{
	  retrange->end = last_token_locus_range ()->end;
	}

      if (tok->type == T_IDENT)
	{
	  PARSER_TABLE *ent = parser_find (ptab, tok->str);

	  if (!single_statement && ent == NULL)
	    ent = parser_find (global_parsetab, tok->str);

	  if (ent)
	    {
	      void *data = ent->data ? ent->data : call_data;
	      switch (ent->parser ((char*)data + ent->off, section_data))
		{
		case PARSER_OK:
		  type = gettkn (&tok);
		  if (type == T_ERROR)
		    return PARSER_FAIL;
		  if (type != '\n' && type != EOF)
		    {
		      conf_error ("unexpected %s", token_type_str (type));
		      return PARSER_FAIL;
		    }
		  if (single_statement)
		    return PARSER_OK_NONL;
		  break;

		case PARSER_OK_NONL:
		  continue;

		case PARSER_FAIL:
		  return PARSER_FAIL;

		case PARSER_END:
		  goto end;
		}
	    }
	  else
	    {
	      conf_error_at_locus_range (&tok->locus, "unrecognized keyword");
	      return PARSER_FAIL;
	    }
	}
      else if (tok->type == '\n')
	continue;
      else
	conf_error_at_locus_range (&tok->locus, "syntax error");
    }
 end:
  return PARSER_OK;
}

static int
parser_loop (PARSER_TABLE *ptab, void *call_data, void *section_data,
	     struct locus_range *retrange)
{
  return parse_statement (ptab, call_data, section_data, 0, retrange);
}

/*
 * Named backends
 */
typedef struct named_backend
{
  char *name;
  struct locus_range locus;
  int priority;
  int disabled;
  struct be_regular bereg;
  SLIST_ENTRY (named_backend) link;
} NAMED_BACKEND;

#define HT_TYPE NAMED_BACKEND
#include "ht.h"

typedef struct named_backend_table
{
  NAMED_BACKEND_HASH *hash;
  SLIST_HEAD(,named_backend) head;
} NAMED_BACKEND_TABLE;

static void
named_backend_table_init (NAMED_BACKEND_TABLE *tab)
{
  tab->hash = NAMED_BACKEND_HASH_NEW ();
  SLIST_INIT (&tab->head);
}

static void
named_backend_table_free (NAMED_BACKEND_TABLE *tab)
{
  NAMED_BACKEND_HASH_FREE (tab->hash);
  while (!SLIST_EMPTY (&tab->head))
    {
      NAMED_BACKEND *ent = SLIST_FIRST (&tab->head);
      SLIST_SHIFT (&tab->head, link);
      free (ent);
    }
}

static NAMED_BACKEND *
named_backend_insert (NAMED_BACKEND_TABLE *tab, char const *name,
		      struct locus_range const *locus,
		      BACKEND *be)
{
  NAMED_BACKEND *bp, *old;

  bp = xmalloc (sizeof (*bp) + strlen (name) + 1);
  bp->name = (char*) (bp + 1);
  strcpy (bp->name, name);
  bp->locus = *locus;
  bp->priority = be->priority;
  bp->disabled = be->disabled;
  bp->bereg = be->v.reg;
  if ((old = NAMED_BACKEND_INSERT (tab->hash, bp)) != NULL)
    {
      free (bp);
      return old;
    }
  SLIST_PUSH (&tab->head, bp, link);
  return NULL;
}

static NAMED_BACKEND *
named_backend_retrieve (NAMED_BACKEND_TABLE *tab, char const *name)
{
  NAMED_BACKEND key;

  key.name = (char*) name;
  return NAMED_BACKEND_RETRIEVE (tab->hash, &key);
}

typedef struct
{
  int log_level;
  int facility;
  unsigned clnt_to;
  unsigned be_to;
  unsigned ws_to;
  unsigned be_connto;
  unsigned ignore_case;
  int header_options;
  BALANCER balancer;
  NAMED_BACKEND_TABLE named_backend_table;
} POUND_DEFAULTS;

static int
parse_includedir (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
  if (open_include_dir (tok->str) == NULL)
    {
      conf_error ("can't open directory %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static int
parse_include (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
  if (push_input (tok->str))
    return PARSER_FAIL;
  return PARSER_OK_NONL;
}

static int
parse_end (void *call_data, void *section_data)
{
  return PARSER_END;
}

static int
int_set_one (void *call_data, void *section_data)
{
  *(int*)call_data = 1;
  return PARSER_OK;
}

static int
assign_string (void *call_data, void *section_data)
{
  char *s;
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
  s = xstrdup (tok->str);
  *(char**)call_data = s;
  return PARSER_OK;
}

static int
assign_string_from_file (void *call_data, void *section_data)
{
  struct stat st;
  char *s;
  FILE *fp;
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
  if ((fp = fopen_include (tok->str)) == NULL)
    {
      fopen_error (LOG_ERR, errno, include_wd, tok->str, &tok->locus);
      return PARSER_FAIL;
    }
  if (fstat (fileno (fp), &st))
    {
      conf_error ("can't stat %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  // FIXME: Check st_size bounds.
  s = xmalloc (st.st_size + 1);
  if (fread (s, st.st_size, 1, fp) != 1)
    {
      conf_error ("%s: read error: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  s[st.st_size] = 0;
  fclose (fp);
  *(char**)call_data = s;
  return PARSER_OK;
}

static int
assign_bool (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect_mask (T_UNQ);

  if (!tok)
    return PARSER_FAIL;

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
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static int
assign_unsigned (void *call_data, void *section_data)
{
  unsigned long n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return PARSER_FAIL;

  errno = 0;
  n = strtoul (tok->str, &p, 10);
  if (errno || *p || n > UINT_MAX)
    {
      conf_error ("%s", "bad unsigned number");
      return PARSER_FAIL;
    }
  *(unsigned *)call_data = n;
  return 0;
}

static int
assign_int (void *call_data, void *section_data)
{
  long n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return PARSER_FAIL;

  errno = 0;
  n = strtol (tok->str, &p, 10);
  if (errno || *p || n < INT_MIN || n > INT_MAX)
    {
      conf_error ("%s", "bad integer number");
      return PARSER_FAIL;
    }
  *(int *)call_data = n;
  return 0;
}

static int
assign_int_range (int *dst, int min, int max)
{
  int n;
  int rc;

  if ((rc = assign_int (&n, NULL)) != PARSER_OK)
    return rc;

  if (!(min <= n && n <= max))
    {
      conf_error ("value out of allowed range (%d..%d)", min, max);
      return PARSER_FAIL;
    }
  *dst = n;
  return PARSER_OK;
}

static int
assign_mode (void *call_data, void *section_data)
{
  long n;
  char *end;
  struct token *tok = gettkn_expect (T_NUMBER);

  errno = 0;
  n = strtoul (tok->str, &end, 8);
  if (errno || *end || n > 0777)
    {
      conf_error_at_locus_range (&tok->locus, "%s", "invalid file mode");
      return PARSER_FAIL;
    }
  *(mode_t*)call_data = n;
  return PARSER_OK;
}

static int
assign_CONTENT_LENGTH (void *call_data, void *section_data)
{
  CONTENT_LENGTH n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return PARSER_FAIL;

  if (strtoclen (tok->str, 10, &n, &p) || *p)
    {
      conf_error ("%s", "bad long number");
      return PARSER_FAIL;
    }
  *(CONTENT_LENGTH *)call_data = n;
  return 0;
}

#define assign_timeout assign_unsigned

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

static int
assign_log_facility (void *call_data, void *section_data)
{
  int n;
  struct token *tok = gettkn_expect_mask (T_UNQ);

  if (!tok)
    return PARSER_FAIL;

  if (strcmp (tok->str, "-") == 0)
    n = -1;
  else if (kw_to_tok (facility_table, tok->str, 1, &n) != 0)
    {
      conf_error ("%s", "unknown log facility name");
      return PARSER_FAIL;
    }
  *(int*)call_data = n;

  return PARSER_OK;
}
/*
 * The ai_flags in the struct addrinfo is not used, unless in hints.
 * Therefore it is reused to mark which parts of address have been
 * initialized.
 */
#define ADDRINFO_SET_ADDRESS(addr) ((addr)->ai_flags = AI_NUMERICHOST)
#define ADDRINFO_HAS_ADDRESS(addr) ((addr)->ai_flags & AI_NUMERICHOST)
#define ADDRINFO_SET_PORT(addr) ((addr)->ai_flags |= AI_NUMERICSERV)
#define ADDRINFO_HAS_PORT(addr) ((addr)->ai_flags & AI_NUMERICSERV)

static int
assign_address_internal (struct addrinfo *addr, struct token *tok)
{
  if (!tok)
    return PARSER_FAIL;

  if (tok->type != T_IDENT && tok->type != T_LITERAL && tok->type != T_STRING)
    {
      conf_error_at_locus_range (&tok->locus,
				 "expected hostname or IP address, but found %s",
				 token_type_str (tok->type));
      return PARSER_FAIL;
    }
  if (get_host (tok->str, addr, PF_UNSPEC))
    {
      /* if we can't resolve it assume this is a UNIX domain socket */
      struct sockaddr_un *sun;
      size_t len = strlen (tok->str);
      if (len > UNIX_PATH_MAX)
	{
	  conf_error_at_locus_range (&tok->locus,
				     "%s", "UNIX path name too long");
	  return PARSER_FAIL;
	}

      len += offsetof (struct sockaddr_un, sun_path) + 1;
      sun = xmalloc (len);
      sun->sun_family = AF_UNIX;
      strcpy (sun->sun_path, tok->str);

      addr->ai_socktype = SOCK_STREAM;
      addr->ai_family = AF_UNIX;
      addr->ai_protocol = 0;
      addr->ai_addr = (struct sockaddr *) sun;
      addr->ai_addrlen = len;
    }
  ADDRINFO_SET_ADDRESS (addr);
  return PARSER_OK;
}

static int
assign_address (void *call_data, void *section_data)
{
  struct addrinfo *addr = call_data;

  if (ADDRINFO_HAS_ADDRESS (addr))
    {
      conf_error ("%s", "Duplicate Address statement");
      return PARSER_FAIL;
    }

  return assign_address_internal (call_data, gettkn_any ());
}

static int
assign_port_internal (struct addrinfo *addr, struct token *tok)
{
  struct addrinfo hints, *res;
  int rc;

  if (!tok)
    return PARSER_FAIL;

  if (tok->type != T_IDENT && tok->type != T_NUMBER)
    {
      conf_error_at_locus_range (&tok->locus,
				 "expected port number or service name, but found %s",
				 token_type_str (tok->type));
      return PARSER_FAIL;
    }

  if (!(addr->ai_family == AF_INET || addr->ai_family == AF_INET6))
    {
      conf_error_at_locus_range (&tok->locus, "Port is not applicable to this address family");
      return PARSER_FAIL;
    }

  memset (&hints, 0, sizeof(hints));
  hints.ai_flags = feature_is_set (FEATURE_DNS) ? 0 : AI_NUMERICHOST;
  hints.ai_family = addr->ai_family;
  hints.ai_socktype = addr->ai_socktype;
  hints.ai_protocol = addr->ai_protocol;
  rc = getaddrinfo (NULL, tok->str, &hints, &res);
  if (rc != 0)
    {
      conf_error_at_locus_range (&tok->locus,
				 "bad port number: %s", gai_strerror (rc));
      return PARSER_FAIL;
    }

  switch (addr->ai_family)
    {
    case AF_INET:
      ((struct sockaddr_in *)addr->ai_addr)->sin_port =
	((struct sockaddr_in *)res->ai_addr)->sin_port;
      break;

    case AF_INET6:
      ((struct sockaddr_in6 *)addr->ai_addr)->sin6_port =
	((struct sockaddr_in6 *)res->ai_addr)->sin6_port;
      break;

    default:
      conf_error_at_locus_range (&tok->locus, "%s",
				 "Port is supported only for INET/INET6 back-ends");
      return PARSER_FAIL;
    }
  freeaddrinfo (res);
  ADDRINFO_SET_PORT (addr);

  return PARSER_OK;
}

static int
assign_port (void *call_data, void *section_data)
{
  struct addrinfo *addr = call_data;

  if (ADDRINFO_HAS_PORT (addr))
    {
      conf_error ("%s", "Duplicate port statement");
      return PARSER_FAIL;
    }
  if (!(ADDRINFO_HAS_ADDRESS (addr)))
    {
      conf_error ("%s", "Address statement should precede Port");
      return PARSER_FAIL;
    }

  return assign_port_internal (call_data, gettkn_any ());
}

/*
 * ACL support
 */

/* Max. number of bytes in an inet address (suitable for both v4 and v6) */
#define MAX_INADDR_BYTES 16

typedef struct cidr
{
  int family;                           /* Address family */
  int len;                              /* Address length */
  unsigned char addr[MAX_INADDR_BYTES]; /* Network address */
  unsigned char mask[MAX_INADDR_BYTES]; /* Address mask */
  SLIST_ENTRY (cidr) next;              /* Link to next CIDR */
} CIDR;

/* Create a new ACL. */
static ACL *
new_acl (char const *name)
{
  ACL *acl;

  XZALLOC (acl);
  if (name)
    acl->name = xstrdup (name);
  else
    acl->name = NULL;
  SLIST_INIT (&acl->head);

  return acl;
}

/* Match cidr against inet address ap/len.  Return 0 on match, 1 otherwise. */
static int
cidr_match (CIDR *cidr, unsigned char *ap, size_t len)
{
  size_t i;

  if (cidr->len == len)
    {
      for (i = 0; i < len; i++)
	{
	  if (cidr->addr[i] != (ap[i] & cidr->mask[i]))
	    return 1;
	}
    }
  return 0;
}

/*
 * Split the inet address of SA to address pointer and length, suitable
 * for use with the above functions.  Store pointer in RET_PTR.  Return
 * address length in bytes, or -1 if SA has invalid address family.
 */
static int
sockaddr_bytes (struct sockaddr *sa, unsigned char **ret_ptr)
{
  switch (sa->sa_family)
    {
    case AF_INET:
      *ret_ptr = (unsigned char *) &(((struct sockaddr_in*)sa)->sin_addr.s_addr);
      return 4;

    case AF_INET6:
      *ret_ptr = (unsigned char *) &(((struct sockaddr_in6*)sa)->sin6_addr);
      return 16;

    default:
      break;
    }
  return -1;
}

/*
 * Match sockaddr SA against ACL.  Return 0 if it matches, 1 if it does not
 * and -1 on error (invalid address family).
 */
int
acl_match (ACL *acl, struct sockaddr *sa)
{
  CIDR *cidr;
  unsigned char *ap;
  size_t len;

  if ((len = sockaddr_bytes (sa, &ap)) == -1)
    return -1;

  SLIST_FOREACH (cidr, &acl->head, next)
    {
      if (cidr->family == sa->sa_family && cidr_match (cidr, ap, len) == 0)
	return 0;
    }

  return 1;
}

static void
masklen_to_netmask (unsigned char *buf, size_t len, size_t masklen)
{
  int i, cnt;

  cnt = masklen / 8;
  for (i = 0; i < cnt; i++)
    buf[i] = 0xff;
  if (i == MAX_INADDR_BYTES)
    return;
  cnt = 8 - masklen % 8;
  buf[i++] = (0xff >> cnt) << cnt;
  for (; i < MAX_INADDR_BYTES; i++)
    buf[i] = 0;
}

/* Parse CIDR at the current point of the input. */
static int
parse_cidr (ACL *acl)
{
  struct token *tok;
  char *mask;
  struct addrinfo hints, *res;
  unsigned long masklen;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if ((mask = strchr (tok->str, '/')) != NULL)
    {
      char *p;

      *mask++ = 0;

      errno = 0;
      masklen = strtoul (mask, &p, 10);
      if (errno || *p)
	{
	  conf_error ("%s", "invalid netmask");
	  return PARSER_FAIL;
	}
    }

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_flags = AI_NUMERICHOST;

  if ((rc = getaddrinfo (tok->str, NULL, &hints, &res)) == 0)
    {
      CIDR *cidr;
      int len, i;
      unsigned char *p;

      if ((len = sockaddr_bytes (res->ai_addr, &p)) == -1)
	{
	  conf_error ("%s", "unsupported address family");
	  return PARSER_FAIL;
	}
      XZALLOC (cidr);
      cidr->family = res->ai_family;
      cidr->len = len;
      memcpy (cidr->addr, p, len);
      if (!mask)
	masklen = len * 8;
      masklen_to_netmask (cidr->mask, cidr->len, masklen);
      /* Fix-up network address, just in case */
      for (i = 0; i < len; i++)
	cidr->addr[i] &= cidr->mask[i];
      SLIST_PUSH (&acl->head, cidr, next);
      freeaddrinfo (res);
    }
  else
    {
      conf_error ("%s", "invalid IP address: %s", gai_strerror (rc));
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

/*
 * List of named ACLs.
 * There shouldn't be many of them, so it's perhaps no use in implementing
 * more sophisticated data structures than a mere singly-linked list.
 */
static ACL_HEAD acl_list = SLIST_HEAD_INITIALIZER (acl_list);

/*
 * Return a pointer to the named ACL, or NULL if no ACL with such name is
 * found.
 */
static ACL *
acl_by_name (char const *name)
{
  ACL *acl;
  SLIST_FOREACH (acl, &acl_list, next)
    {
      if (strcmp (acl->name, name) == 0)
	break;
    }
  return acl;
}

/*
 * Parse ACL definition.
 * On entry, input must be positioned on the next token after ACL ["name"].
 */
static int
parse_acl (ACL *acl)
{
  struct token *tok;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type != '\n')
    {
      conf_error ("expected newline, but found %s", token_type_str (tok->type));
      return PARSER_FAIL;
    }

  for (;;)
    {
      int rc;
      if ((tok = gettkn_any ()) == NULL)
	return PARSER_FAIL;
      if (tok->type == '\n')
	continue;
      if (tok->type == T_IDENT)
	{
	  if (strcasecmp (tok->str, "end") == 0)
	    break;
	  if (strcasecmp (tok->str, "include") == 0)
	    {
	      if ((rc = parse_include (NULL, NULL)) == PARSER_FAIL)
		return rc;
	      continue;
	    }
	  conf_error ("expected CIDR, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return PARSER_FAIL;
	}
      putback_tkn (tok);
      if ((rc = parse_cidr (acl)) != PARSER_OK)
	return rc;
    }
  return PARSER_OK;
}

/*
 * Parse a named ACL.
 * Input is positioned after the "ACL" keyword.
 */
static int
parse_named_acl (void *call_data, void *section_data)
{
  ACL *acl;
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if (acl_by_name (tok->str))
    {
      conf_error ("%s", "ACL with that name already defined");
      return PARSER_FAIL;
    }

  acl = new_acl (tok->str);
  SLIST_PUSH (&acl_list, acl, next);

  return parse_acl (acl);
}

/*
 * Parse ACL reference.  Two forms are accepted:
 * ACL "name"
 *   References a named ACL.
 * ACL "\n" ... End
 *   Creates and references an unnamed ACL.
 */
static int
parse_acl_ref (ACL **ret_acl)
{
  struct token *tok;
  ACL *acl;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type == '\n')
    {
      putback_tkn (tok);
      acl = new_acl (NULL);
      *ret_acl = acl;
      return parse_acl (acl);
    }
  else if (tok->type == T_STRING)
    {
      if ((acl = acl_by_name (tok->str)) == NULL)
	{
	  conf_error ("no such ACL: %s", tok->str);
	  return PARSER_FAIL;
	}
      *ret_acl = acl;
    }
  else
    {
      conf_error ("expected ACL name or definition, but found %s",
		  token_type_str (tok->type));
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static int
assign_acl (void *call_data, void *section_data)
{
  return parse_acl_ref (call_data);
}

static int
parse_ECDHCurve (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
#if SET_DH_AUTO == 0 && !defined OPENSSL_NO_ECDH
  if (set_ECDHCurve (tok->str) == 0)
    {
      conf_error ("%s", "ECDHCurve: invalid curve name");
      return PARSER_FAIL;
    }
#else
  conf_error ("%s", "statement ignored");
#endif
  return PARSER_OK;
}

static int
parse_SSLEngine (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;
#if HAVE_OPENSSL_ENGINE_H && OPENSSL_VERSION_MAJOR < 3
  ENGINE *e;

  if (!(e = ENGINE_by_id (tok->str)))
    {
      conf_error ("%s", "unrecognized engine");
      return PARSER_FAIL;
    }

  if (!ENGINE_init (e))
    {
      ENGINE_free (e);
      conf_error ("%s", "could not init engine");
      return PARSER_FAIL;
    }

  if (!ENGINE_set_default (e, ENGINE_METHOD_ALL))
    {
      ENGINE_free (e);
      conf_error ("%s", "could not set all defaults");
    }

  ENGINE_finish (e);
  ENGINE_free (e);
#else
  conf_error ("%s", "statement ignored");
#endif

  return PARSER_OK;
}

static int
backend_parse_https (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct stringbuf sb;

  if ((be->v.reg.ctx = SSL_CTX_new (SSLv23_client_method ())) == NULL)
    {
      conf_openssl_error (NULL, "SSL_CTX_new");
      return PARSER_FAIL;
    }

  SSL_CTX_set_app_data (be->v.reg.ctx, be);
  SSL_CTX_set_verify (be->v.reg.ctx, SSL_VERIFY_NONE, NULL);
  SSL_CTX_set_mode (be->v.reg.ctx, SSL_MODE_AUTO_RETRY);
#ifdef SSL_MODE_SEND_FALLBACK_SCSV
  SSL_CTX_set_mode (be->v.reg.ctx, SSL_MODE_SEND_FALLBACK_SCSV);
#endif
  SSL_CTX_set_options (be->v.reg.ctx, SSL_OP_ALL);
#ifdef  SSL_OP_NO_COMPRESSION
  SSL_CTX_set_options (be->v.reg.ctx, SSL_OP_NO_COMPRESSION);
#endif
  SSL_CTX_clear_options (be->v.reg.ctx,
			 SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);
  SSL_CTX_clear_options (be->v.reg.ctx, SSL_OP_LEGACY_SERVER_CONNECT);

  xstringbuf_init (&sb);
  stringbuf_printf (&sb, "%d-Pound-%ld", getpid (), random ());
  SSL_CTX_set_session_id_context (be->v.reg.ctx,
				  (unsigned char *) stringbuf_value (&sb),
				  stringbuf_len (&sb));
  stringbuf_free (&sb);

  POUND_SSL_CTX_init (be->v.reg.ctx);

  return PARSER_OK;
}

static int
backend_parse_cert (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->v.reg.ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if (SSL_CTX_use_certificate_chain_file (be->v.reg.ctx, tok->str) != 1)
    {
      conf_openssl_error (tok->str, "SSL_CTX_use_certificate_chain_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_use_PrivateKey_file (be->v.reg.ctx, tok->str, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error (tok->str, "SSL_CTX_use_PrivateKey_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (be->v.reg.ctx) != 1)
    {
      conf_openssl_error (tok->str, "SSL_CTX_check_private_key failed");
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

static int
backend_assign_ciphers (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->v.reg.ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  SSL_CTX_set_cipher_list (be->v.reg.ctx, tok->str);
  return PARSER_OK;
}

static int
backend_parse_servername (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->v.reg.ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  be->v.reg.servername = xstrdup (tok->str);

  return PARSER_OK;
}

static int
backend_assign_priority (void *call_data, void *section_data)
{
  return assign_int_range (call_data, 0, 9);
}

static int
set_proto_opt (int *opt)
{
  struct token *tok;
  int n;

  static struct kwtab kwtab[] = {
    { "SSLv2", SSL_OP_NO_SSLv2 },
    { "SSLv3", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 },
#ifdef SSL_OP_NO_TLSv1
    { "TLSv1", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 },
#endif
#ifdef SSL_OP_NO_TLSv1_1
    { "TLSv1_1", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
		 SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 },
#endif
#ifdef SSL_OP_NO_TLSv1_2
    { "TLSv1_2", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
		 SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 |
		 SSL_OP_NO_TLSv1_2 },
#endif
    { NULL }
  };

  if ((tok = gettkn_expect (T_IDENT)) == NULL)
    return PARSER_FAIL;

  if (kw_to_tok (kwtab, tok->str, 0, &n))
    {
      conf_error ("%s", "unrecognized protocol name");
      return PARSER_FAIL;
    }

  *opt |= n;

  return PARSER_OK;
}

static int
disable_proto (void *call_data, void *section_data)
{
  SSL_CTX *ctx = *(SSL_CTX**) call_data;
  int n = 0;

  if (ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if (set_proto_opt (&n) != PARSER_OK)
    return PARSER_FAIL;

  SSL_CTX_set_options (ctx, n);

  return PARSER_OK;
}

static PARSER_TABLE backend_parsetab[] = {
  { "End",       parse_end },
  { "Address",   assign_address, NULL, offsetof (BACKEND, v.reg.addr) },
  { "Port",      assign_port,    NULL, offsetof (BACKEND, v.reg.addr) },
  { "Priority",  backend_assign_priority, NULL, offsetof (BACKEND, priority) },
  { "TimeOut",   assign_timeout, NULL, offsetof (BACKEND, v.reg.to) },
  { "WSTimeOut", assign_timeout, NULL, offsetof (BACKEND, v.reg.ws_to) },
  { "ConnTO",    assign_timeout, NULL, offsetof (BACKEND, v.reg.conn_to) },
  { "HTTPS",     backend_parse_https },
  { "Cert",      backend_parse_cert },
  { "Ciphers",   backend_assign_ciphers },
  { "Disable",   disable_proto,  NULL, offsetof (BACKEND, v.reg.ctx) },
  { "Disabled",  assign_bool,    NULL, offsetof (BACKEND, disabled) },
  { "ServerName",backend_parse_servername, NULL },
  { NULL }
};

static PARSER_TABLE use_backend_parsetab[] = {
  { "End",       parse_end },
  { "Priority",  backend_assign_priority, NULL, offsetof (BACKEND, priority) },
  { "Disabled",  assign_bool,    NULL, offsetof (BACKEND, disabled) },
  { NULL }
};

static PARSER_TABLE emergency_parsetab[] = {
  { "End", parse_end },
  { "Address", assign_address, NULL, offsetof (BACKEND, v.reg.addr) },
  { "Port", assign_port, NULL, offsetof (BACKEND, v.reg.addr) },
  { "TimeOut", assign_timeout, NULL, offsetof (BACKEND, v.reg.to) },
  { "WSTimeOut", assign_timeout, NULL, offsetof (BACKEND, v.reg.ws_to) },
  { "ConnTO", assign_timeout, NULL, offsetof (BACKEND, v.reg.conn_to) },
  { "HTTPS", backend_parse_https },
  { "Cert", backend_parse_cert },
  { "Ciphers", backend_assign_ciphers },
  { "Disable", disable_proto, NULL, offsetof (BACKEND, v.reg.ctx) },
  { NULL }
};

static int
check_addrinfo (struct addrinfo const *addr, struct locus_range const *range, char const *name)
{
  if (ADDRINFO_HAS_ADDRESS (addr))
    {
      if (!ADDRINFO_HAS_PORT (addr) &&
	  (addr->ai_family == AF_INET || addr->ai_family == AF_INET6))
	{
	  conf_error_at_locus_range (range, "%s missing Port declaration", name);
	  return PARSER_FAIL;
	}
    }
  else
    {
      conf_error_at_locus_range (range, "%s missing Address declaration", name);
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static char *
format_locus_str (struct locus_range *rp)
{
  struct stringbuf sb;

  xstringbuf_init (&sb);
  stringbuf_format_locus_range (&sb, rp);
  return stringbuf_finish (&sb);
}

static BACKEND *
parse_backend_internal (PARSER_TABLE *table, POUND_DEFAULTS *dfl,
			struct locus_point *beg)
{
  BACKEND *be;
  struct locus_range range;

  XZALLOC (be);
  be->be_type = BE_BACKEND;
  be->v.reg.addr.ai_socktype = SOCK_STREAM;
  be->v.reg.to = dfl->be_to;
  be->v.reg.conn_to = dfl->be_connto;
  be->v.reg.ws_to = dfl->ws_to;
  be->v.reg.alive = 1;
  memset (&be->v.reg.addr, 0, sizeof (be->v.reg.addr));
  be->priority = 5;
  pthread_mutex_init (&be->mut, NULL);

  if (parser_loop (table, be, dfl, &range))
    return NULL;
  if (beg)
    range.beg = *beg;
  if (check_addrinfo (&be->v.reg.addr, &range, "Backend") != PARSER_OK)
    return NULL;
  be->locus = format_locus_str (&range);

  return be;
}

static int
parse_backend (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  BACKEND *be;
  struct token *tok;
  struct locus_point beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type == T_STRING)
    {
      struct locus_range range;

      range.beg = beg;

      XZALLOC (be);
      be->be_type = BE_BACKEND_REF;
      be->v.be_name = xstrdup (tok->str);
      be->priority = -1;
      be->disabled = -1;
      pthread_mutex_init (&be->mut, NULL);

      if (parser_loop (use_backend_parsetab, be, section_data, &range))
	return PARSER_FAIL;
      be->locus = format_locus_str (&tok->locus);
    }
  else
    {
      putback_tkn (tok);
      be = parse_backend_internal (backend_parsetab, section_data, &beg);
      if (!be)
	return PARSER_FAIL;
    }

  SLIST_PUSH (head, be, next);

  return PARSER_OK;
}

static int
parse_use_backend (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  BACKEND *be;
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  XZALLOC (be);
  be->be_type = BE_BACKEND_REF;
  be->v.be_name = xstrdup (tok->str);
  be->locus = format_locus_str (&tok->locus);
  be->priority = 5;
  pthread_mutex_init (&be->mut, NULL);

  SLIST_PUSH (head, be, next);

  return PARSER_OK;
}

static int
parse_emergency (void *call_data, void *section_data)
{
  BACKEND **res_ptr = call_data;
  BACKEND *be;
  POUND_DEFAULTS dfl = *(POUND_DEFAULTS*)section_data;

  dfl.be_to = 120;
  dfl.be_connto = 120;
  dfl.ws_to = 120;

  be = parse_backend_internal (emergency_parsetab, &dfl, NULL);
  if (!be)
    return PARSER_FAIL;

  *res_ptr = be;

  return PARSER_OK;
}

static int
parse_metrics (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  BACKEND *be;

  XZALLOC (be);
  be->be_type = BE_METRICS;
  be->priority = 1;
  pthread_mutex_init (&be->mut, NULL);
  SLIST_PUSH (head, be, next);
  return PARSER_OK;
}

static SERVICE_COND *
service_cond_append (SERVICE_COND *cond, int type)
{
  SERVICE_COND *sc;

  assert (cond->type == COND_BOOL);
  XZALLOC (sc);
  service_cond_init (sc, type);
  SLIST_PUSH (&cond->bool.head, sc, next);

  return sc;
}

static void
stringbuf_escape_regex (struct stringbuf *sb, char const *p)
{
  while (*p)
    {
      size_t len = strcspn (p, "\\[]{}().*+?");
      if (len > 0)
	stringbuf_add (sb, p, len);
      p += len;
      if (*p)
	{
	  stringbuf_add_char (sb, '\\');
	  stringbuf_add_char (sb, *p);
	  p++;
	}
    }
}

enum match_mode
  {
    MATCH_EXACT,
    MATCH_RE,
    MATCH_BEG,
    MATCH_END,
    MATCH__MAX
  };

static int
parse_match_mode (int *mode, int *re_flags, int *from_file)
{
  struct token *tok;

  enum
  {
    MATCH_ICASE = MATCH__MAX,
    MATCH_CASE,
    MATCH_FILE
  };

  static struct kwtab optab[] = {
    { "-re",    MATCH_RE },
    { "-exact", MATCH_EXACT },
    { "-beg",   MATCH_BEG },
    { "-end",   MATCH_END },
    { "-icase", MATCH_ICASE },
    { "-case",  MATCH_CASE },
    { "-file",  MATCH_FILE },
    { NULL }
  };

  if (from_file)
    *from_file = 0;

  for (;;)
    {
      int n;

      if ((tok = gettkn_expect_mask (T_BIT (T_STRING) | T_BIT (T_LITERAL))) == NULL)
	return PARSER_FAIL;

      if (tok->type == T_STRING)
	break;

      if (kw_to_tok (optab, tok->str, 0, &n))
	{
	  conf_error ("unexpected token: %s", tok->str);
	  return PARSER_FAIL;
	}

      switch (n)
	{
	case MATCH_CASE:
	  *re_flags &= ~REG_ICASE;
	  break;

	case MATCH_ICASE:
	  *re_flags |= REG_ICASE;
	  break;

	case MATCH_FILE:
	  if (from_file)
	    *from_file = 1;
	  else
	    {
	      conf_error ("unexpected token: %s", tok->str);
	      return PARSER_FAIL;
	    }
	  break;

	default:
	  *mode = n;
	}
    }
  putback_tkn (tok);
  return PARSER_OK;
}

static char *
build_regex (struct stringbuf *sb, int mode, char const *expr, char const *pfx)
{
  switch (mode)
    {
    case MATCH_EXACT:
      stringbuf_add_char (sb, '^');
      if (pfx)
	stringbuf_add_string (sb, pfx);
      stringbuf_escape_regex (sb, expr);
      stringbuf_add_char (sb, '$');
      break;

    case MATCH_RE:
      if (pfx)
	{
	  stringbuf_add_string (sb, pfx);
	  if (expr[0] == '^')
	    expr++;
	}
      stringbuf_add_string (sb, expr);
      break;

    case MATCH_BEG:
      stringbuf_add_char (sb, '^');
      if (pfx)
	stringbuf_add_string (sb, pfx);
      stringbuf_escape_regex (sb, expr);
      stringbuf_add_string (sb, ".*");
      break;

    case MATCH_END:
      stringbuf_add_string (sb, ".*");
      if (pfx)
	stringbuf_add_string (sb, pfx);
      stringbuf_escape_regex (sb, expr);
      stringbuf_add_char (sb, '$');
      break;

    default:
      abort ();
    }
  return stringbuf_finish (sb);
}

static int
parse_regex_compat (regex_t *regex, int flags)
{
  struct token *tok;
  int mode = MATCH_RE;
  char *p;
  int rc;
  struct stringbuf sb;

  if (parse_match_mode (&mode, &flags, NULL))
    return PARSER_FAIL;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  xstringbuf_init (&sb);
  p = build_regex (&sb, mode, tok->str, NULL);
  rc = regcomp (regex, p, flags);
  stringbuf_free (&sb);
  if (rc)
    {
      conf_regcomp_error (rc, regex, NULL);
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

STRING_REF *
string_ref_alloc (char const *str)
{
  STRING_REF *ref = xmalloc (sizeof (*ref) + strlen (str));
  ref->refcount = 1;
  strcpy (ref->value, str);
  return ref;
}

STRING_REF *
string_ref_incr (STRING_REF *ref)
{
  if (ref)
    ref->refcount++;
  return ref;
}

void
string_ref_free (STRING_REF *ref)
{
  if (ref && --ref->refcount == 0)
    free (ref);
}

static int
parse_cond_matcher_0 (SERVICE_COND *top_cond, enum service_cond_type type,
		      int mode, int flags, char const *string)
{
  struct token *tok;
  int rc;
  struct stringbuf sb;
  SERVICE_COND *cond;
  static char const host_pfx[] = "Host:[[:space:]]*";
  int from_file;
  char *expr;

  if (parse_match_mode (&mode, &flags, &from_file))
    return PARSER_FAIL;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  xstringbuf_init (&sb);
  if (from_file)
    {
      FILE *fp;
      char *p;
      char buf[MAXBUF];
      STRING_REF *ref = NULL;

      if ((fp = fopen_include (tok->str)) == NULL)
	{
	  fopen_error (LOG_ERR, errno, include_wd, tok->str, &tok->locus);
	  return PARSER_FAIL;
	}

      cond = service_cond_append (top_cond, COND_BOOL);
      cond->bool.op = BOOL_OR;

      switch (type)
	{
	case COND_QUERY_PARAM:
	case COND_STRING_MATCH:
	  ref = string_ref_alloc (string);
	  break;
	default:
	  break;
	}

      while ((p = fgets (buf, sizeof buf, fp)) != NULL)
	{
	  int rc;
	  size_t len;
	  SERVICE_COND *hc;

	  p += strspn (p, " \t");
	  for (len = strlen (p);
	       len > 0 && (p[len-1] == ' ' || p[len-1] == '\t'|| p[len-1] == '\n'); len--)
	    ;
	  if (len == 0 || *p == '#')
	    continue;
	  p[len] = 0;

	  stringbuf_reset (&sb);
	  expr = build_regex (&sb, mode, p, type == COND_HOST ? host_pfx : NULL);
	  hc = service_cond_append (cond, type);
	  rc = regcomp (&hc->re, expr, flags);
	  if (rc)
	    {
	      conf_regcomp_error (rc, &hc->re, NULL);
	      return PARSER_FAIL;
	    }
	  switch (type)
	    {
	    case COND_QUERY_PARAM:
	    case COND_STRING_MATCH:
	      memmove (&hc->sm.re, &hc->re, sizeof (hc->sm.re));
	      hc->sm.string = string_ref_incr (ref);
	      break;

	    default:
	      break;
	    }
	}
      string_ref_free (ref);
      fclose (fp);
    }
  else
    {
      cond = service_cond_append (top_cond, type);
      expr = build_regex (&sb, mode, tok->str, type == COND_HOST ? host_pfx : NULL);
      rc = regcomp (&cond->re, expr, flags);
      if (rc)
	{
	  conf_regcomp_error (rc, &cond->re, NULL);
	  return PARSER_FAIL;
	}
      switch (type)
	{
	case COND_QUERY_PARAM:
	case COND_STRING_MATCH:
	  memmove (&cond->sm.re, &cond->re, sizeof (cond->sm.re));
	  cond->sm.string = string_ref_alloc (string);
	  break;

	default:
	  break;
	}
    }
  stringbuf_free (&sb);

  return PARSER_OK;
}

static int
parse_cond_matcher (SERVICE_COND *top_cond, enum service_cond_type type,
		    int mode, int flags, char const *string)
{
  int rc;
  char *string_copy;
  if (string)
    string_copy = xstrdup (string);
  else
    string_copy = NULL;
  rc = parse_cond_matcher_0 (top_cond, type, mode, flags, string_copy);
  free (string_copy);
  return rc;
}

static int
parse_cond_acl (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_ACL);
  return parse_acl_ref (&cond->acl);
}

static int
parse_cond_url_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_URL, MATCH_RE,
			     REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0),
			     NULL);
}

static int
parse_cond_path_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_PATH, MATCH_RE,
			     REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0),
			     NULL);
}

static int
parse_cond_query_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_QUERY, MATCH_RE,
			     REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0),
			     NULL);
}

static int
parse_cond_query_param_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *top_cond = call_data;
  POUND_DEFAULTS *dfl = section_data;
  int flags = REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0);
  struct token *tok;
  char *string;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  string = xstrdup (tok->str);
  rc = parse_cond_matcher (top_cond, COND_QUERY_PARAM, MATCH_RE, flags,
			   string);
  free (string);
  return rc;
}

static int
parse_cond_string_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *top_cond = call_data;
  POUND_DEFAULTS *dfl = section_data;
  int flags = REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0);
  struct token *tok;
  char *string;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  string = xstrdup (tok->str);
  rc = parse_cond_matcher (top_cond, COND_STRING_MATCH, MATCH_RE, flags,
			   string);
  free (string);
  return rc;
}

static int
parse_cond_hdr_matcher (void *call_data, void *section_data)
{
  return parse_cond_matcher (call_data, COND_HDR, MATCH_RE,
			     REG_NEWLINE | REG_EXTENDED | REG_ICASE,
			     NULL);
}

static int
parse_cond_head_deny_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BOOL);
  cond->bool.op = BOOL_NOT;
  return parse_cond_matcher (cond, COND_HDR, MATCH_RE,
			     REG_NEWLINE | REG_EXTENDED | REG_ICASE,
			     NULL);
}

static int
parse_cond_host (void *call_data, void *section_data)
{
  return parse_cond_matcher (call_data, COND_HOST, MATCH_EXACT,
			     REG_EXTENDED | REG_ICASE, NULL);
}

static int
parse_cond_basic_auth (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BASIC_AUTH);
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  cond->pwfile.locus = tok->locus;
  cond->pwfile.filename = xstrdup (tok->str);
  return PARSER_OK;
}

static int
parse_redirect_backend (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  struct token *tok;
  int code = 302;
  BACKEND *be;
  regmatch_t matches[5];
  struct locus_range range;

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type == T_NUMBER)
    {
      int n = atoi (tok->str);
      switch (n)
	{
	case 301:
	case 302:
	case 303:
	case 307:
	case 308:
	  code = n;
	  break;

	default:
	  conf_error ("%s", "invalid status code");
	  return PARSER_FAIL;
	}

      if ((tok = gettkn_any ()) == NULL)
	return PARSER_FAIL;
    }

  range.end = last_token_locus_range ()->end;

  if (tok->type != T_STRING)
    {
      conf_error ("expected %s, but found %s", token_type_str (T_STRING), token_type_str (tok->type));
      return PARSER_FAIL;
    }

  XZALLOC (be);
  be->locus = format_locus_str (&range);
  be->be_type = BE_REDIRECT;
  be->priority = 1;
  pthread_mutex_init (&be->mut, NULL);

  be->v.redirect.status = code;
  be->v.redirect.url = xstrdup (tok->str);

  if (regexec (&LOCATION, be->v.redirect.url, 4, matches, 0))
    {
      conf_error ("%s", "Redirect bad URL");
      return PARSER_FAIL;
    }

  if ((be->v.redirect.has_uri = matches[3].rm_eo - matches[3].rm_so) == 1)
    /* the path is a single '/', so remove it */
    be->v.redirect.url[matches[3].rm_so] = '\0';

  SLIST_PUSH (head, be, next);

  return PARSER_OK;
}

static int
parse_error_backend (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  struct token *tok;
  int n, status;
  char *text = NULL;
  BACKEND *be;
  int rc;
  struct locus_range range;

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_expect (T_NUMBER)) == NULL)
    return PARSER_FAIL;

  n = atoi (tok->str);
  if ((status = http_status_to_pound (n)) == -1)
    {
      conf_error ("%s", "unsupported status code");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type == T_STRING)
    {
      putback_tkn (tok);
      if ((rc = assign_string_from_file (&text, section_data)) == PARSER_FAIL)
	return rc;
    }
  else if (tok->type == '\n')
    rc = PARSER_OK_NONL;
  else
    {
      conf_error ("%s", "string or newline expected");
      return PARSER_FAIL;
    }

  range.end = last_token_locus_range ()->end;

  XZALLOC (be);
  be->locus = format_locus_str (&range);
  be->be_type = BE_ERROR;
  be->priority = 1;
  pthread_mutex_init (&be->mut, NULL);

  be->v.error.status = status;
  be->v.error.text = text;

  SLIST_PUSH (head, be, next);

  return rc;
}

struct service_session
{
  int type;
  char *id;
  unsigned ttl;
};

static struct kwtab sess_type_tab[] = {
  { "IP", SESS_IP },
  { "COOKIE", SESS_COOKIE },
  { "URL", SESS_URL },
  { "PARM", SESS_PARM },
  { "BASIC", SESS_BASIC },
  { "HEADER", SESS_HEADER },
  { NULL }
};

char const *
sess_type_to_str (int type)
{
  if (type == SESS_NONE)
    return "NONE";
  return kw_to_str (sess_type_tab, type);
}

static int
session_type_parser (void *call_data, void *section_data)
{
  SERVICE *svc = call_data;
  struct token *tok;
  int n;

  if ((tok = gettkn_expect (T_IDENT)) == NULL)
    return PARSER_FAIL;

  if (kw_to_tok (sess_type_tab, tok->str, 1, &n))
    {
      conf_error ("%s", "Unknown Session type");
      return PARSER_FAIL;
    }
  svc->sess_type = n;

  return PARSER_OK;
}

static PARSER_TABLE session_parsetab[] = {
  { "End", parse_end },
  { "Type", session_type_parser },
  { "TTL", assign_timeout, NULL, offsetof (SERVICE, sess_ttl) },
  { "ID", assign_string, NULL, offsetof (SERVICE, sess_id) },
  { NULL }
};

static int
parse_session (void *call_data, void *section_data)
{
  SERVICE *svc = call_data;
  struct locus_range range;

  if (parser_loop (session_parsetab, svc, section_data, &range))
    return PARSER_FAIL;

  if (svc->sess_type == SESS_NONE)
    {
      conf_error_at_locus_range (&range, "Session type not defined");
      return PARSER_FAIL;
    }

  if (svc->sess_ttl == 0)
    {
      conf_error_at_locus_range (&range, "Session TTL not defined");
      return PARSER_FAIL;
    }

  switch (svc->sess_type)
    {
    case SESS_COOKIE:
    case SESS_URL:
    case SESS_HEADER:
      if (svc->sess_id == NULL)
	{
	  conf_error ("%s", "Session ID not defined");
	  return PARSER_FAIL;
	}
      break;

    default:
      break;
    }

  return PARSER_OK;
}

static int
assign_dfl_ignore_case (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return assign_bool (&dfl->ignore_case, NULL);
}

static int parse_cond (int op, SERVICE_COND *cond, void *section_data);

static int
parse_match (void *call_data, void *section_data)
{
  struct token *tok;
  int op = BOOL_AND;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;
  if (tok->type == T_IDENT)
    {
      if (strcasecmp (tok->str, "and") == 0)
	op = BOOL_AND;
      else if (strcasecmp (tok->str, "or") == 0)
	op = BOOL_OR;
      else
	{
	  conf_error ("expected AND or OR, but found %s", tok->str);
	  return PARSER_FAIL;
	}
    }
  else
    putback_tkn (tok);

  return parse_cond (op, call_data, section_data);
}

static int parse_not_cond (void *call_data, void *section_data);

#define MATCH_CONDITIONS(data, off)				\
  { "ACL", parse_cond_acl, data, off },				\
  { "URL", parse_cond_url_matcher, data, off },			\
  { "Path", parse_cond_path_matcher, data, off },		\
  { "Query", parse_cond_query_matcher, data, off },		\
  { "QueryParam", parse_cond_query_param_matcher, data, off },	\
  { "Header", parse_cond_hdr_matcher, data, off },		\
  { "Host", parse_cond_host, data, off },			\
  { "BasicAuth", parse_cond_basic_auth, data, off },		\
  { "StringMatch", parse_cond_string_matcher, data, off },	\
  { "Match", parse_match, data, off },				\
  { "NOT", parse_not_cond, data, off },				\
    /* compatibility keywords */				\
  { "HeadRequire", parse_cond_hdr_matcher, data, off },         \
  { "HeadDeny", parse_cond_head_deny_matcher, data, off }

static PARSER_TABLE negate_parsetab[] = {
  MATCH_CONDITIONS (NULL, 0),
  { NULL }
};

static int
parse_not_cond (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BOOL);
  cond->bool.op = BOOL_NOT;
  return parse_statement (negate_parsetab, cond, section_data, 1, NULL);
}

static PARSER_TABLE logcon_parsetab[] = {
  { "End", parse_end },
  MATCH_CONDITIONS (NULL, 0),
  { NULL }
};

static int
parse_cond (int op, SERVICE_COND *cond, void *section_data)
{
  SERVICE_COND *subcond = service_cond_append (cond, COND_BOOL);
  struct locus_range range;

  subcond->bool.op = op;
  return parser_loop (logcon_parsetab, subcond, section_data, &range);
}

static int parse_else (void *call_data, void *section_data);
static int parse_rewrite (void *call_data, void *section_data);
static int parse_set_header (void *call_data, void *section_data);
static int parse_delete_header (void *call_data, void *section_data);
static int parse_set_url (void *call_data, void *section_data);
static int parse_set_path (void *call_data, void *section_data);
static int parse_set_query (void *call_data, void *section_data);
static int parse_set_query_param (void *call_data, void *section_data);
static int parse_sub_rewrite (void *call_data, void *section_data);

#define REWRITE_OPS(data, off)						\
  { "SetHeader", parse_set_header, data, off },				\
  { "DeleteHeader", parse_delete_header, data, off },			\
  { "SetURL", parse_set_url, data, off },				\
  { "SetPath", parse_set_path, data, off },				\
  { "SetQuery", parse_set_query, data, off },				\
  { "SetQueryParam", parse_set_query_param, data, off }

static PARSER_TABLE rewrite_rule_parsetab[] = {
  { "End", parse_end },
  { "Rewrite", parse_sub_rewrite, NULL, offsetof (REWRITE_RULE, ophead) },
  { "Else", parse_else, NULL, offsetof (REWRITE_RULE, iffalse) },
  MATCH_CONDITIONS (NULL, offsetof (REWRITE_RULE, cond)),
  REWRITE_OPS (NULL, offsetof (REWRITE_RULE, ophead)),
  { NULL }
};

static int
parse_end_else (void *call_data, void *section_data)
{
  struct token nl = { '\n' };
  putback_tkn (NULL);
  putback_tkn (&nl);
  return PARSER_END;
}

static PARSER_TABLE else_rule_parsetab[] = {
  { "End", parse_end_else },
  { "Rewrite", parse_sub_rewrite, NULL, offsetof (REWRITE_RULE, ophead) },
  { "Else", parse_else, NULL, offsetof (REWRITE_RULE, iffalse) },
  MATCH_CONDITIONS (NULL, offsetof (REWRITE_RULE, cond)),
  REWRITE_OPS (NULL, offsetof (REWRITE_RULE, ophead)),
  { NULL }
};

static REWRITE_OP *
rewrite_op_alloc (REWRITE_OP_HEAD *head, enum rewrite_type type)
{
  REWRITE_OP *op;

  XZALLOC (op);
  op->type = type;
  SLIST_PUSH (head, op, next);

  return op;
}

static int
parse_rewrite_op (REWRITE_OP_HEAD *head, enum rewrite_type type)
{
  REWRITE_OP *op = rewrite_op_alloc (head, type);
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  op->v.str = xstrdup (tok->str);
  return PARSER_OK;
}

static int
parse_delete_header (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_HDR_DEL);
  POUND_DEFAULTS *dfl = section_data;

  XZALLOC (op->v.hdrdel);
  return parse_regex_compat (&op->v.hdrdel->pat,
			     REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0));
}

static int
parse_set_header (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_HDR_SET);
}

static int
parse_set_url (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_URL_SET);
}

static int
parse_set_path (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_PATH_SET);
}

static int
parse_set_query (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_QUERY_SET);
}

static int
parse_set_query_param (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_QUERY_PARAM_SET);
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  op->v.qp.name = xstrdup (tok->str);

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  op->v.qp.value = xstrdup (tok->str);

  return PARSER_OK;

}

static REWRITE_RULE *
rewrite_rule_alloc (REWRITE_RULE_HEAD *head)
{
  REWRITE_RULE *rule;

  XZALLOC (rule);
  service_cond_init (&rule->cond, COND_BOOL);
  SLIST_INIT (&rule->ophead);

  if (head)
    SLIST_PUSH (head, rule, next);

  return rule;
}

static int
parse_else (void *call_data, void *section_data)
{
  REWRITE_RULE *rule = rewrite_rule_alloc (NULL);
  *(REWRITE_RULE**)call_data = rule;
  return parser_loop (else_rule_parsetab, rule, section_data, NULL);
}

static int
parse_sub_rewrite (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_REWRITE_RULE);
  op->v.rule = rewrite_rule_alloc (NULL);
  return parser_loop (rewrite_rule_parsetab, op->v.rule, section_data, NULL);
}

#define MATCH_RESPONSE_CONDITIONS(data, off)			\
  { "Header", parse_cond_hdr_matcher, data, off },		\
  { "StringMatch", parse_cond_string_matcher, data, off },	\
  { "Match", parse_match, data, off },				\
  { "NOT", parse_not_cond, data, off }

#define REWRITE_RESPONSE_OPS(data, off)						\
  { "SetHeader", parse_set_header, data, off },				\
  { "DeleteHeader", parse_delete_header, data, off }

static int parse_response_else (void *call_data, void *section_data);
static int parse_response_sub_rewrite (void *call_data, void *section_data);

static PARSER_TABLE response_rewrite_rule_parsetab[] = {
  { "End", parse_end },
  { "Rewrite", parse_response_sub_rewrite, NULL, offsetof (REWRITE_RULE, ophead) },
  { "Else", parse_response_else, NULL, offsetof (REWRITE_RULE, iffalse) },
  MATCH_RESPONSE_CONDITIONS (NULL, offsetof (REWRITE_RULE, cond)),
  REWRITE_RESPONSE_OPS (NULL, offsetof (REWRITE_RULE, ophead)),
  { NULL }
};

static PARSER_TABLE response_else_rule_parsetab[] = {
  { "End", parse_end_else },
  { "Rewrite", parse_response_sub_rewrite, NULL, offsetof (REWRITE_RULE, ophead) },
  { "Else", parse_else, NULL, offsetof (REWRITE_RULE, iffalse) },
  MATCH_RESPONSE_CONDITIONS (NULL, offsetof (REWRITE_RULE, cond)),
  REWRITE_RESPONSE_OPS (NULL, offsetof (REWRITE_RULE, ophead)),
  { NULL }
};

static int
parse_response_else (void *call_data, void *section_data)
{
  REWRITE_RULE *rule = rewrite_rule_alloc (NULL);
  *(REWRITE_RULE**)call_data = rule;
  return parser_loop (response_else_rule_parsetab, rule, section_data, NULL);
}

static int
parse_response_sub_rewrite (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_REWRITE_RULE);
  op->v.rule = rewrite_rule_alloc (NULL);
  return parser_loop (response_rewrite_rule_parsetab, op->v.rule, section_data, NULL);
}

static int
parse_rewrite (void *call_data, void *section_data)
{
  struct token *tok;
  PARSER_TABLE *table;
  REWRITE_RULE_HEAD *rw = call_data, *head;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;
  if (tok->type == T_IDENT)
    {
      if (strcasecmp (tok->str, "response") == 0)
	{
	  table = response_rewrite_rule_parsetab;
	  head = &rw[REWRITE_RESPONSE];
	}
      else if (strcasecmp (tok->str, "request") == 0)
	{
	  table = rewrite_rule_parsetab;
	  head = &rw[REWRITE_REQUEST];
	}
      else
	{
	  conf_error ("expected response, request, or newline, but found %s",
		      token_type_str (tok->type));
	  return PARSER_FAIL;
	}
    }
  else
    {
      putback_tkn (tok);
      table = rewrite_rule_parsetab;
      head = &rw[REWRITE_REQUEST];
    }
  return parser_loop (table, rewrite_rule_alloc (head), section_data, NULL);
}

static REWRITE_RULE *
rewrite_rule_last_uncond (REWRITE_RULE_HEAD *head)
{
  if (!SLIST_EMPTY (head))
    {
      REWRITE_RULE *rw = SLIST_LAST (head);
      if (rw->cond.type == COND_BOOL && SLIST_EMPTY (&rw->cond.bool.head))
	return rw;
    }

  return rewrite_rule_alloc (head);
}

#define __cat2__(a,b) a ## b
#define SETFN_NAME(part)			\
  __cat2__(parse_,part)
#define SETFN_SVC_NAME(part)			\
  __cat2__(parse_svc_,part)
#define SETFN_SVC_DECL(part)					     \
  static int							     \
  SETFN_SVC_NAME(part) (void *call_data, void *section_data)	     \
  {								     \
    REWRITE_RULE *rule = rewrite_rule_last_uncond (call_data);	     \
    return SETFN_NAME(part) (&rule->ophead, section_data);	     \
  }

SETFN_SVC_DECL (set_url)
SETFN_SVC_DECL (set_path)
SETFN_SVC_DECL (set_query)
SETFN_SVC_DECL (set_query_param)
SETFN_SVC_DECL (set_header)
SETFN_SVC_DECL (delete_header)

/*
 * Support for backward-compatible HeaderRemove and HeadRemove directives.
 */
static int
parse_header_remove (void *call_data, void *section_data)
{
  REWRITE_RULE *rule = rewrite_rule_last_uncond (call_data);
  REWRITE_OP *op = rewrite_op_alloc (&rule->ophead, REWRITE_HDR_DEL);
  XZALLOC (op->v.hdrdel);
  return parse_regex_compat (&op->v.hdrdel->pat,
			     REG_EXTENDED | REG_ICASE | REG_NEWLINE);
}

static int
parse_balancer (void *call_data, void *section_data)
{
  BALANCER *t = call_data;
  struct token *tok;

  if ((tok = gettkn_expect_mask (T_UNQ)) == NULL)
    return PARSER_FAIL;
  if (strcasecmp (tok->str, "random") == 0)
    *t = BALANCER_RANDOM;
  else if (strcasecmp (tok->str, "iwrr") == 0)
    *t = BALANCER_IWRR;
  else
    {
      conf_error ("unsupported balancing strategy: %s", tok->str);
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static int
parse_log_suppress (void *call_data, void *section_data)
{
  int *result_ptr = call_data;
  struct token *tok;
  int n;
  int result = 0;
  int type;
  static struct kwtab status_table[] = {
    { "all",      STATUS_MASK (100) | STATUS_MASK (200) |
		  STATUS_MASK (300) | STATUS_MASK (400) | STATUS_MASK (500) },
    { "info",     STATUS_MASK (100) },
    { "success",  STATUS_MASK (200) },
    { "redirect", STATUS_MASK (300) },
    { "clterr",   STATUS_MASK (400) },
    { "srverr",   STATUS_MASK (500) },
    { NULL }
  };

  if ((tok = gettkn_expect_mask (T_UNQ)) == NULL)
    return PARSER_FAIL;

  do
    {
      if (strlen (tok->str) == 1 && isdigit (tok->str[0]))
	{
	  n = tok->str[0] - '0';
	  if (n <= 0 || n >= sizeof (status_table) / sizeof (status_table[0]))
	    {
	      conf_error ("%s", "unsupported status mask");
	      return PARSER_FAIL;
	    }
	  n = STATUS_MASK (n * 100);
	}
      else if (kw_to_tok (status_table, tok->str, 1, &n) != 0)
	{
	  conf_error ("%s", "unsupported status mask");
	  return PARSER_FAIL;
	}
      result |= n;
    }
  while ((type = gettkn (&tok)) != EOF && type != T_ERROR &&
	 T_MASK_ISSET (T_UNQ, type));

  if (type == T_ERROR)
    return PARSER_FAIL;
  if (type == EOF)
    {
      conf_error ("%s", "unexpected end of file");
      return PARSER_FAIL;
    }

  putback_tkn (tok);

  *result_ptr = result;

  return PARSER_OK;
}

static PARSER_TABLE service_parsetab[] = {
  { "End", parse_end },

  MATCH_CONDITIONS (NULL, offsetof (SERVICE, cond)),

  { "Rewrite", parse_rewrite, NULL, offsetof (SERVICE, rewrite) },
  { "SetHeader", SETFN_SVC_NAME (set_header), NULL, offsetof (SERVICE, rewrite) },
  { "DeleteHeader", SETFN_SVC_NAME (delete_header), NULL, offsetof (SERVICE, rewrite) },
  { "SetURL", SETFN_SVC_NAME (set_url), NULL, offsetof (SERVICE, rewrite) },
  { "SetPath", SETFN_SVC_NAME (set_path), NULL, offsetof (SERVICE, rewrite) },
  { "SetQuery", SETFN_SVC_NAME (set_query), NULL, offsetof (SERVICE, rewrite) },
  { "SetQueryParam", SETFN_SVC_NAME (set_query_param), NULL, offsetof (SERVICE, rewrite) },

  { "IgnoreCase", assign_dfl_ignore_case },
  { "Disabled", assign_bool, NULL, offsetof (SERVICE, disabled) },
  { "Redirect", parse_redirect_backend, NULL, offsetof (SERVICE, backends) },
  { "Error", parse_error_backend, NULL, offsetof (SERVICE, backends) },
  { "Backend", parse_backend, NULL, offsetof (SERVICE, backends) },
  { "UseBackend", parse_use_backend, NULL, offsetof (SERVICE, backends) },
  { "Emergency", parse_emergency, NULL, offsetof (SERVICE, emergency) },
  { "Metrics", parse_metrics, NULL, offsetof (SERVICE, backends) },
  { "Session", parse_session },
  { "Balancer", parse_balancer, NULL, offsetof (SERVICE, balancer) },
  { "ForwardedHeader", assign_string, NULL, offsetof (SERVICE, forwarded_header) },
  { "TrustedIP", assign_acl, NULL, offsetof (SERVICE, trusted_ips) },
  { "LogSuppress", parse_log_suppress, NULL, offsetof (SERVICE, log_suppress_mask) },
  { NULL }
};

static int
find_service_ident (SERVICE_HEAD *svc_head, char const *name)
{
  SERVICE *svc;
  SLIST_FOREACH (svc, svc_head, next)
    {
      if (svc->name && strcmp (svc->name, name) == 0)
	return 1;
    }
  return 0;
}

static int
parse_service (void *call_data, void *section_data)
{
  SERVICE_HEAD *head = call_data;
  POUND_DEFAULTS *dfl = (POUND_DEFAULTS*) section_data;
  struct token *tok;
  SERVICE *svc;
  struct locus_range range;

  XZALLOC (svc);
  service_cond_init (&svc->cond, COND_BOOL);
  SLIST_INIT (&svc->backends);

  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, NULL);
  svc->balancer = dfl->balancer;

  tok = gettkn_any ();

  if (!tok)
    return PARSER_FAIL;

  if (tok->type == T_STRING)
    {
      if (find_service_ident (head, tok->str))
	{
	  conf_error ("%s", "service name is not unique");
	  return PARSER_FAIL;
	}
      svc->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  if ((svc->sessions = session_table_new ()) == NULL)
    {
      conf_error ("%s", "session_table_new failed");
      return -1;
    }

  if (parser_loop (service_parsetab, svc, dfl, &range))
    return PARSER_FAIL;
  else
    {
      BACKEND *be;

      if ((be = SLIST_FIRST (&svc->backends)) == NULL)
	{
	  conf_error_at_locus_range (&range, "warning: no backends defined");
	}
      else
	{
	  int be_class = 0;
#         define BE_MASK(n) (1<<(n))
#         define  BX_(x)  ((x) - (((x)>>1)&0x77777777)			\
			   - (((x)>>2)&0x33333333)			\
			   - (((x)>>3)&0x11111111))
#         define BITCOUNT(x)     (((BX_(x)+(BX_(x)>>4)) & 0x0F0F0F0F) % 255)
	  int n = 0;

	  SLIST_FOREACH (be, &svc->backends, next)
	    {
	      n++;
	      be_class |= BE_MASK (be->be_type);
	      be->service = svc;
	      if (!be->disabled)
		{
		  svc->tot_pri += be->priority;
		  if (svc->max_pri < be->priority)
		    svc->max_pri = be->priority;
		}
	      svc->abs_pri += be->priority;
	    }

	  if (n > 1)
	    {
	      if (be_class & ~(BE_MASK (BE_BACKEND) | BE_MASK (BE_REDIRECT)))
		{
		  conf_error_at_locus_range (&range,
			  "%s",
			  BITCOUNT (be_class) == 1
			    ? "multiple backends of this type are not allowed"
			    : "service mixes backends of different types");
		  return PARSER_FAIL;
		}

	       if (be_class & BE_MASK (BE_REDIRECT))
		{
		  conf_error_at_locus_range (&range,
			  "warning: %s",
			  (be_class & BE_MASK (BE_BACKEND))
			     ? "service mixes regular and redirect backends"
			     : "service uses multiple redirect backends");
		  conf_error_at_locus_range (&range,
			  "see section \"DEPRECATED FEATURES\" in pound(8)");
		}
	    }
	}

      service_lb_init (svc);

      SLIST_PUSH (head, svc, next);
    }
  svc->locus = format_locus_str (&range);
  return PARSER_OK;
}

static int
parse_acme (void *call_data, void *section_data)
{
  SERVICE_HEAD *head = call_data;
  SERVICE *svc;
  BACKEND *be;
  SERVICE_COND *cond;
  struct token *tok;
  struct stat st;
  int rc;
  static char re_acme[] = "^/\\.well-known/acme-challenge/(.+)";
  int fd;
  struct locus_range range;

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if (stat (tok->str, &st))
    {
      conf_error ("can't stat %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  if (!S_ISDIR (st.st_mode))
    {
      conf_error ("%s is not a directory: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  if ((fd = open (tok->str, O_RDONLY | O_NONBLOCK | O_DIRECTORY)) == -1)
    {
      conf_error ("can't open directory %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }

  /* Create service */
  XZALLOC (svc);
  service_cond_init (&svc->cond, COND_BOOL);
  SLIST_INIT (&svc->backends);

  /* Create a URL matcher */
  cond = service_cond_append (&svc->cond, COND_URL);
  rc = regcomp (&cond->re, re_acme, REG_EXTENDED);
  if (rc)
    {
      conf_regcomp_error (rc, &cond->re, NULL);
      return PARSER_FAIL;
    }

  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, NULL);

  range.end = last_token_locus_range ()->beg;
  svc->locus = format_locus_str (&range);

  svc->tot_pri = 1;
  svc->abs_pri = 1;
  svc->max_pri = 1;

  /* Create ACME backend */
  XZALLOC (be);
  be->be_type = BE_ACME;
  be->priority = 1;
  pthread_mutex_init (&be->mut, NULL);

  be->v.acme.wd = fd;

  /* Register backend in service */
  SLIST_PUSH (&svc->backends, be, next);

  /* Register service in the listener */
  SLIST_PUSH (head, svc, next);

  return PARSER_OK;
}


static int
listener_parse_xhttp (void *call_data, void *section_data)
{
  return assign_int_range (call_data, 0, 3);
}

static int
listener_parse_checkurl (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  POUND_DEFAULTS *dfl = section_data;
  struct token *tok;
  int rc;

  if (lst->has_pat)
    {
      conf_error ("%s", "CheckURL multiple pattern");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  rc = regcomp (&lst->url_pat, tok->str,
		REG_NEWLINE | REG_EXTENDED |
		(dfl->ignore_case ? REG_ICASE : 0));
  if (rc)
    {
      conf_regcomp_error (rc, &lst->url_pat, NULL);
      return PARSER_FAIL;
    }
  lst->has_pat = 1;

  return PARSER_OK;
}

static int
read_fd (int fd)
{
  struct msghdr msg;
  struct iovec iov[1];
  char base[1];
  union
  {
    struct cmsghdr cm;
    char control[CMSG_SPACE (sizeof (int))];
  } control_un;
  struct cmsghdr *cmptr;

  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof (control_un.control);

  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  iov[0].iov_base = base;
  iov[0].iov_len = sizeof (base);

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  if (recvmsg (fd, &msg, 0) > 0)
    {
      if ((cmptr = CMSG_FIRSTHDR (&msg)) != NULL
	  && cmptr->cmsg_len == CMSG_LEN (sizeof (int))
	  && cmptr->cmsg_level == SOL_SOCKET
	  && cmptr->cmsg_type == SCM_RIGHTS)
	return *((int*) CMSG_DATA (cmptr));
    }
  return -1;
}

static int
listener_parse_socket_from (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct sockaddr_storage ss;
  socklen_t sslen = sizeof (ss);
  struct token *tok;
  struct addrinfo addr;
  int sfd, fd;

  if (ADDRINFO_HAS_ADDRESS (&lst->addr))
    {
      conf_error ("%s", "Duplicate Address or SocketFrom statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  memset (&addr, 0, sizeof (addr));
  if (assign_address_internal (&addr, tok) != PARSER_OK)
    return PARSER_FAIL;

  if ((sfd = socket (PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      conf_error ("socket: %s", strerror (errno));
      return PARSER_FAIL;
    }

  if (connect (sfd, addr.ai_addr, addr.ai_addrlen) < 0)
    {
      conf_error ("connect %s: %s",
		  ((struct sockaddr_un*)addr.ai_addr)->sun_path,
		  strerror (errno));
      return PARSER_FAIL;
    }

  fd = read_fd (sfd);

  if (fd == -1)
    {
      conf_error ("can't get socket: %s", strerror (errno));
      return PARSER_FAIL;
    }

  if (getsockname (fd, (struct sockaddr*) &ss, &sslen) == -1)
    {
      conf_error ("can't get socket address: %s", strerror (errno));
      return PARSER_FAIL;
    }

  free (lst->addr.ai_addr);
  lst->addr.ai_addr = xmalloc (sslen);
  memcpy (lst->addr.ai_addr, &ss, sslen);
  lst->addr.ai_addrlen = sslen;
  lst->addr.ai_family = ss.ss_family;
  ADDRINFO_SET_ADDRESS (&lst->addr);
  ADDRINFO_SET_PORT (&lst->addr);

  {
    struct stringbuf sb;
    char tmp[MAX_ADDR_BUFSIZE];

    xstringbuf_init (&sb);
    stringbuf_format_locus_range (&sb, &tok->locus);
    stringbuf_add_string (&sb, ": obtained address ");
    stringbuf_add_string (&sb, addr2str (tmp, sizeof (tmp), &lst->addr, 0));
    logmsg (LOG_DEBUG, "%s", stringbuf_finish (&sb));
    stringbuf_free (&sb);
  }

  lst->sock = fd;

  return PARSER_OK;
}

static int
parse_rewritelocation (void *call_data, void *section_data)
{
  return assign_int_range (call_data, 0, 2);
}

struct canned_log_format
{
  char *name;
  char *fmt;
};

static struct canned_log_format canned_log_format[] = {
  /* 0 - not used */
  { "null", "" },
  /* 1 - regular logging */
  { "regular", "%a %r - %>s" },
  /* 2 - extended logging (show chosen backend server as well) */
  { "extended", "%a %r - %>s (%{Host}i/%{service}N -> %{backend}N) %{f}T sec" },
  /* 3 - Apache-like format (Combined Log Format with Virtual Host) */
  { "vhost_combined", "%{Host}I %a - %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\"" },
  /* 4 - same as 3 but without the virtual host information */
  { "combined", "%a - %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\"" },
  /* 5 - same as 3 but with information about the Service and Backend used */
  { "detailed", "%{Host}I %a - %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\" (%{service}N -> %{backend}N) %{f}T sec" },
};
static int max_canned_log_format =
  sizeof (canned_log_format) / sizeof (canned_log_format[0]);

struct log_format_data
{
  struct locus_range *locus;
  int fn;
  int fatal;
};

void
log_format_diag (void *data, int fatal, char const *msg, int off)
{
  struct log_format_data *ld = data;
  if (ld->fn == -1)
    {
      struct locus_range loc = *ld->locus;
      loc.beg.col += off;
      loc.end = loc.beg;
      conf_error_at_locus_range (&loc, "%s", msg);
    }
  else
    {
      conf_error_at_locus_range (ld->locus, "INTERNAL ERROR: error compiling built-in format %d", ld->fn);
      conf_error_at_locus_range (ld->locus, "%s: near %s", msg,
				 canned_log_format[ld->fn].fmt + off);
      conf_error_at_locus_range (ld->locus, "please report");
    }
  ld->fatal = fatal;
}

static void
compile_canned_formats (void)
{
  struct log_format_data ld;
  int i;

  ld.locus = NULL;
  ld.fatal = 0;

  for (i = 0; i < max_canned_log_format; i++)
    {
      ld.fn = i;
      if (http_log_format_compile (canned_log_format[i].name,
				   canned_log_format[i].fmt,
				   log_format_diag, &ld) == -1 || ld.fatal)
	exit (1);
    }
}

static int
parse_log_level (void *call_data, void *section_data)
{
  int log_level;
  int *log_level_ptr = call_data;
  struct token *tok = gettkn_expect_mask (T_BIT (T_STRING) | T_BIT (T_NUMBER));
  if (!tok)
    return PARSER_FAIL;

  if (tok->type == T_STRING)
    {
      log_level = http_log_format_find (tok->str);
      if (log_level == -1)
	{
	  conf_error ("undefined format: %s", tok->str);
	  return PARSER_FAIL;
	}
    }
  else
    {
      char *p;
      long n;

      errno = 0;
      n = strtol (tok->str, &p, 10);
      if (errno || *p || n < 0 || n > INT_MAX)
	{
	  conf_error ("%s", "unsupported log level number");
	  return PARSER_FAIL;
	}
      if (http_log_format_check (n))
	{
	  conf_error ("%s", "undefined log level");
	  return PARSER_FAIL;
	}
      log_level = n;
    }
  *log_level_ptr = log_level;
  return PARSER_OK;
}

static int
parse_log_format (void *call_data, void *section_data)
{
  struct token *tok;
  char *name;
  struct log_format_data ld;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;
  name = strdup (tok->str);
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    {
      free (name);
      return PARSER_FAIL;
    }

  ld.locus = &tok->locus;
  ld.fn = -1;
  ld.fatal = 0;

  if (http_log_format_compile (name, tok->str, log_format_diag, &ld) == -1 ||
      ld.fatal)
    rc = PARSER_FAIL;
  else
    rc = PARSER_OK;
  free (name);
  return rc;
}

static int
parse_header_options (void *call_data, void *section_data)
{
  int *opt = call_data;
  int n;
  struct token *tok;
  static struct kwtab options[] = {
    { "forwarded", HDROPT_FORWARDED_HEADERS },
    { "ssl",       HDROPT_SSL_HEADERS },
    { "all",       HDROPT_FORWARDED_HEADERS|HDROPT_SSL_HEADERS },
    { NULL }
  };

  for (;;)
    {
      char *name;
      int neg;

      if ((tok = gettkn_any ()) == NULL)
	return PARSER_FAIL;
      if (tok->type == '\n')
	break;
      if (!(tok->type == T_IDENT || tok->type == T_LITERAL))
	{
	  conf_error ("unexpected %s", token_type_str (tok->type));
	  return PARSER_FAIL;
	}

      name = tok->str;
      if (strcasecmp (name, "none") == 0)
	*opt = 0;
      else
	{
	  if (strncasecmp (name, "no-", 3) == 0)
	    {
	      neg = 1;
	      name += 3;
	    }
	  else
	    neg = 0;

	  if (kw_to_tok (options, name, 1, &n))
	    {
	      conf_error ("%s", "unknown option");
	      return PARSER_FAIL;
	    }

	  if (neg)
	    *opt &= ~n;
	  else
	    *opt |= n;
	}
    }

  return PARSER_OK_NONL;
}

static PARSER_TABLE http_parsetab[] = {
  { "End", parse_end },
  { "Address", assign_address, NULL, offsetof (LISTENER, addr) },
  { "Port", assign_port, NULL, offsetof (LISTENER, addr) },
  { "SocketFrom", listener_parse_socket_from },
  { "xHTTP", listener_parse_xhttp, NULL, offsetof (LISTENER, verb) },
  { "Client", assign_timeout, NULL, offsetof (LISTENER, to) },
  { "CheckURL", listener_parse_checkurl },
  { "Err400", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_BAD_REQUEST]) },
  { "Err401", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_UNAUTHORIZED]) },
  { "Err403", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_FORBIDDEN]) },
  { "Err404", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_FOUND]) },
  { "Err413", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_PAYLOAD_TOO_LARGE]) },
  { "Err414", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_URI_TOO_LONG]) },
  { "Err500", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_INTERNAL_SERVER_ERROR]) },
  { "Err501", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_IMPLEMENTED]) },
  { "Err503", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_SERVICE_UNAVAILABLE]) },
  { "MaxRequest", assign_CONTENT_LENGTH, NULL, offsetof (LISTENER, max_req) },

  { "Rewrite", parse_rewrite, NULL, offsetof (LISTENER, rewrite) },
  { "SetHeader", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "DeleteHeader", SETFN_SVC_NAME (delete_header), NULL, offsetof (LISTENER, rewrite) },
  { "SetURL", SETFN_SVC_NAME (set_url), NULL, offsetof (LISTENER, rewrite) },
  { "SetPath", SETFN_SVC_NAME (set_path), NULL, offsetof (LISTENER, rewrite) },
  { "SetQuery", SETFN_SVC_NAME (set_query), NULL, offsetof (LISTENER, rewrite) },
  { "SetQueryParam", SETFN_SVC_NAME (set_query_param), NULL, offsetof (LISTENER, rewrite) },

  { "HeaderOption", parse_header_options, NULL, offsetof (LISTENER, header_options) },

  /* Backward compatibility */
  { "HeaderAdd", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "AddHeader", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "HeaderRemove", parse_header_remove, NULL, offsetof (LISTENER, rewrite) },
  { "HeadRemove", parse_header_remove, NULL, offsetof (LISTENER, rewrite) },

  { "RewriteLocation", parse_rewritelocation, NULL, offsetof (LISTENER, rewr_loc) },
  { "RewriteDestination", assign_bool, NULL, offsetof (LISTENER, rewr_dest) },
  { "LogLevel", parse_log_level, NULL, offsetof (LISTENER, log_level) },
  { "Service", parse_service, NULL, offsetof (LISTENER, services) },
  { "ACME", parse_acme, NULL, offsetof (LISTENER, services) },
  { "ForwardedHeader", assign_string, NULL, offsetof (LISTENER, forwarded_header) },
  { "TrustedIP", assign_acl, NULL, offsetof (LISTENER, trusted_ips) },
{ NULL }
};

static LISTENER *
listener_alloc (POUND_DEFAULTS *dfl)
{
  LISTENER *lst;

  XZALLOC (lst);

  lst->mode = 0600;
  lst->sock = -1;
  lst->to = dfl->clnt_to;
  lst->rewr_loc = 1;
  lst->log_level = dfl->log_level;
  lst->verb = 0;
  lst->header_options = dfl->header_options;
  SLIST_INIT (&lst->rewrite[REWRITE_REQUEST]);
  SLIST_INIT (&lst->rewrite[REWRITE_RESPONSE]);
  SLIST_INIT (&lst->services);
  SLIST_INIT (&lst->ctx_head);
  return lst;
}

static int
find_listener_ident (LISTENER_HEAD *list_head, char const *name)
{
  LISTENER *lstn;
  SLIST_FOREACH (lstn, list_head, next)
    {
      if (lstn->name && strcmp (lstn->name, name) == 0)
	return 1;
    }
  return 0;
}

static int
parse_listen_http (void *call_data, void *section_data)
{
  LISTENER *lst;
  LISTENER_HEAD *list_head = call_data;
  POUND_DEFAULTS *dfl = section_data;
  struct locus_range range;
  struct token *tok;

  if ((lst = listener_alloc (dfl)) == NULL)
    return PARSER_FAIL;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;
  else if (tok->type == T_STRING)
    {
      if (find_listener_ident (list_head, tok->str))
	{
	  conf_error ("%s", "listener name is not unique");
	  return PARSER_FAIL;
	}
      lst->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  if (parser_loop (http_parsetab, lst, section_data, &range))
    return PARSER_FAIL;

  if (check_addrinfo (&lst->addr, &range, "ListenHTTP") != PARSER_OK)
    return PARSER_FAIL;

  lst->locus = format_locus_str (&range);

  SLIST_PUSH (list_head, lst, next);
  return PARSER_OK;
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
# define general_name_string(n) \
	xstrndup ((char*)ASN1_STRING_get0_data (n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#else
# define general_name_string(n) \
	xstrndup ((char*)ASN1_STRING_data(n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#endif

static void
get_subjectaltnames (X509 *x509, POUND_CTX *pc, size_t san_max)
{
  STACK_OF (GENERAL_NAME) * san_stack =
    (STACK_OF (GENERAL_NAME) *) X509_get_ext_d2i (x509, NID_subject_alt_name,
						  NULL, NULL);
  char **result;

  if (san_stack == NULL)
    return;
  while (sk_GENERAL_NAME_num (san_stack) > 0)
    {
      GENERAL_NAME *name = sk_GENERAL_NAME_pop (san_stack);
      switch (name->type)
	{
	case GEN_DNS:
	  if (pc->subjectAltNameCount == san_max)
	    pc->subjectAltNames = x2nrealloc (pc->subjectAltNames,
					      &san_max,
					      sizeof (pc->subjectAltNames[0]));
	  pc->subjectAltNames[pc->subjectAltNameCount++] = general_name_string (name);
	  break;

	default:
	  logmsg (LOG_INFO, "unsupported subjectAltName type encountered: %i",
		  name->type);
	}
      GENERAL_NAME_free (name);
    }

  sk_GENERAL_NAME_pop_free (san_stack, GENERAL_NAME_free);
  if (pc->subjectAltNameCount
      && (result = realloc (pc->subjectAltNames,
			    pc->subjectAltNameCount * sizeof (pc->subjectAltNames[0]))) != NULL)
    pc->subjectAltNames = result;
}

static int
load_cert (char const *filename, LISTENER *lst)
{
  POUND_CTX *pc;

  XZALLOC (pc);

  if ((pc->ctx = SSL_CTX_new (SSLv23_server_method ())) == NULL)
    {
      conf_openssl_error (NULL, "SSL_CTX_new");
      return PARSER_FAIL;
    }

  if (SSL_CTX_use_certificate_chain_file (pc->ctx, filename) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_certificate_chain_file");
      return PARSER_FAIL;
    }
  if (SSL_CTX_use_PrivateKey_file (pc->ctx, filename, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_PrivateKey_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (pc->ctx) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_check_private_key");
      return PARSER_FAIL;
    }

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
  {
    /* we have support for SNI */
    FILE *fcert;
    X509 *x509;
    X509_NAME *xname = NULL;
    int i;
    size_t san_max;

    if ((fcert = fopen (filename, "r")) == NULL)
      {
	conf_error ("%s: could not open certificate file: %s", filename,
		    strerror (errno));
	return PARSER_FAIL;
      }

    x509 = PEM_read_X509 (fcert, NULL, NULL, NULL);
    fclose (fcert);

    if (!x509)
      {
	conf_error ("%s: could not get certificate subject", filename);
	return PARSER_FAIL;
      }

    pc->subjectAltNameCount = 0;
    pc->subjectAltNames = NULL;
    san_max = 0;

    /* Extract server name */
    xname = X509_get_subject_name (x509);
    for (i = -1;
	 (i = X509_NAME_get_index_by_NID (xname, NID_commonName, i)) != -1;)
      {
	X509_NAME_ENTRY *entry = X509_NAME_get_entry (xname, i);
	ASN1_STRING *value;
	char *str = NULL;
	value = X509_NAME_ENTRY_get_data (entry);
	if (ASN1_STRING_to_UTF8 ((unsigned char **)&str, value) >= 0)
	  {
	    if (pc->server_name == NULL)
	      pc->server_name = str;
	    else
	      {
		if (pc->subjectAltNameCount == san_max)
		  pc->subjectAltNames = x2nrealloc (pc->subjectAltNames,
						    &san_max,
						    sizeof (pc->subjectAltNames[0]));
		pc->subjectAltNames[pc->subjectAltNameCount++] = str;
	      }
	  }
      }

    get_subjectaltnames (x509, pc, san_max);
    X509_free (x509);

    if (pc->server_name == NULL)
      {
	conf_error ("%s: no CN in certificate subject name", filename);
	return PARSER_FAIL;
      }
  }
#else
  if (res->ctx)
    conf_error ("%s: multiple certificates not supported", filename);
#endif
  SLIST_PUSH (&lst->ctx_head, pc, next);

  return PARSER_OK;
}

static int
https_parse_cert (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  struct stat st;

  if (lst->has_other)
    {
      conf_error ("%s", "Cert directives MUST precede other SSL-specific directives");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if (stat (tok->str, &st))
    {
      conf_error ("%s: stat error: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }

  if (S_ISREG (st.st_mode))
    return load_cert (tok->str, lst);

  if (S_ISDIR (st.st_mode))
    {
      DIR *dp;
      struct dirent *ent;
      struct stringbuf namebuf;
      size_t dirlen;
      int rc = PARSER_OK;

      dirlen = strlen (tok->str);
      while (dirlen > 0 && tok->str[dirlen-1] == '/')
	dirlen--;

      xstringbuf_init (&namebuf);
      stringbuf_add (&namebuf, tok->str, dirlen);
      stringbuf_add_char (&namebuf, '/');
      dirlen++;

      dp = opendir (tok->str);
      if (dp == NULL)
	{
	  conf_error ("%s: error opening directory: %s", tok->str,
		      strerror (errno));
	  stringbuf_free (&namebuf);
	  return PARSER_FAIL;
	}

      while ((ent = readdir (dp)) != NULL)
	{
	  char *filename;

	  if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0)
	    continue;

	  stringbuf_add_string (&namebuf, ent->d_name);
	  filename = stringbuf_finish (&namebuf);
	  if (stat (filename, &st))
	    {
	      conf_error ("%s: stat error: %s", filename, strerror (errno));
	    }
	  else if (S_ISREG (st.st_mode))
	    {
	      if ((rc = load_cert (filename, lst)) != PARSER_OK)
		break;
	    }
	  else
	    conf_error ("warning: ignoring %s: not a regular file", filename);
	  stringbuf_truncate (&namebuf, dirlen);
	}
      closedir (dp);
      stringbuf_free (&namebuf);
      return rc;
    }

  conf_error ("%s: not a regular file or directory", tok->str);
  return PARSER_FAIL;
}

static int
verify_OK (int pre_ok, X509_STORE_CTX * ctx)
{
  return 1;
}

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
static int
SNI_server_name (SSL *ssl, int *dummy, POUND_CTX_HEAD *ctx_head)
{
  const char *server_name;
  POUND_CTX *pc;

  if ((server_name = SSL_get_servername (ssl, TLSEXT_NAMETYPE_host_name)) == NULL)
    return SSL_TLSEXT_ERR_NOACK;

  /* logmsg(LOG_DEBUG, "Received SSL SNI Header for servername %s", servername); */

  SSL_set_SSL_CTX (ssl, NULL);
  SLIST_FOREACH (pc, ctx_head, next)
    {
      if (fnmatch (pc->server_name, server_name, 0) == 0)
	{
	  /* logmsg(LOG_DEBUG, "Found cert for %s", servername); */
	  SSL_set_SSL_CTX (ssl, pc->ctx);
	  return SSL_TLSEXT_ERR_OK;
	}
      else if (pc->subjectAltNameCount > 0 && pc->subjectAltNames != NULL)
	{
	  int i;

	  for (i = 0; i < pc->subjectAltNameCount; i++)
	    {
	      if (fnmatch ((char *) pc->subjectAltNames[i], server_name, 0) ==
		  0)
		{
		  SSL_set_SSL_CTX (ssl, pc->ctx);
		  return SSL_TLSEXT_ERR_OK;
		}
	    }
	}
    }

  /* logmsg(LOG_DEBUG, "No match for %s, default used", server_name); */
  SSL_set_SSL_CTX (ssl, SLIST_FIRST (ctx_head)->ctx);
  return SSL_TLSEXT_ERR_OK;
}
#endif

static int
https_parse_client_cert (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  int depth;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "ClientCert may only be used after Cert");
      return PARSER_FAIL;
    }
  lst->has_other = 1;

  if (assign_int_range (&lst->clnt_check, 0, 3) != PARSER_OK)
    return PARSER_FAIL;

  if (lst->clnt_check > 0 && assign_int (&depth, NULL) != PARSER_OK)
    return PARSER_FAIL;

  switch (lst->clnt_check)
    {
    case 0:
      /* don't ask */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	SSL_CTX_set_verify (pc->ctx, SSL_VERIFY_NONE, NULL);
      break;

    case 1:
      /* ask but OK if no client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_CLIENT_ONCE, NULL);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;

    case 2:
      /* ask and fail if no client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;

    case 3:
      /* ask but do not verify client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_CLIENT_ONCE, verify_OK);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;
    }
  return PARSER_OK;
}

static int
https_parse_disable (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  return set_proto_opt (&lst->ssl_op_enable);
}

static int
https_parse_ciphers (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "Ciphers may only be used after Cert");
      return PARSER_FAIL;
    }
  lst->has_other = 1;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    SSL_CTX_set_cipher_list (pc->ctx, tok->str);

  return PARSER_OK;
}

static int
https_parse_honor_cipher_order (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  int bv;

  if (assign_bool (&bv, NULL) != PARSER_OK)
    return PARSER_FAIL;

  if (bv)
    {
      lst->ssl_op_enable |= SSL_OP_CIPHER_SERVER_PREFERENCE;
      lst->ssl_op_disable &= ~SSL_OP_CIPHER_SERVER_PREFERENCE;
    }
  else
    {
      lst->ssl_op_disable |= SSL_OP_CIPHER_SERVER_PREFERENCE;
      lst->ssl_op_enable &= ~SSL_OP_CIPHER_SERVER_PREFERENCE;
    }

  return PARSER_OK;
}

static int
https_parse_allow_client_renegotiation (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;

  if (assign_int_range (&lst->allow_client_reneg, 0, 2) != PARSER_OK)
    return PARSER_FAIL;

  if (lst->allow_client_reneg == 2)
    {
      lst->ssl_op_enable |= SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
      lst->ssl_op_disable &= ~SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
    }
  else
    {
      lst->ssl_op_disable |= SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
      lst->ssl_op_enable &= ~SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
    }

  return PARSER_OK;
}

static int
https_parse_calist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  STACK_OF (X509_NAME) *cert_names;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "CAList may only be used after Cert");
      return PARSER_FAIL;
    }
  lst->has_other = 1;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if ((cert_names = SSL_load_client_CA_file (tok->str)) == NULL)
    {
      conf_openssl_error (NULL, "SSL_load_client_CA_file");
      return PARSER_FAIL;
    }

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    SSL_CTX_set_client_CA_list (pc->ctx, cert_names);

  return PARSER_OK;
}

static int
https_parse_verifylist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "VerifyList may only be used after Cert");
      return PARSER_FAIL;
    }
  lst->has_other = 1;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    if (SSL_CTX_load_verify_locations (pc->ctx, tok->str, NULL) != 1)
      {
	conf_openssl_error (NULL, "SSL_CTX_load_verify_locations");
	return PARSER_FAIL;
      }

  return PARSER_OK;
}

static int
https_parse_crlist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  X509_STORE *store;
  X509_LOOKUP *lookup;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "CRlist may only be used after Cert");
      return PARSER_FAIL;
    }
  lst->has_other = 1;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    {
      store = SSL_CTX_get_cert_store (pc->ctx);
      if ((lookup = X509_STORE_add_lookup (store, X509_LOOKUP_file ())) == NULL)
	{
	  conf_openssl_error (NULL, "X509_STORE_add_lookup");
	  return PARSER_FAIL;
	}

      if (X509_load_crl_file (lookup, tok->str, X509_FILETYPE_PEM) != 1)
	{
	  conf_openssl_error (tok->str, "X509_load_crl_file failed");
	  return PARSER_FAIL;
	}

      X509_STORE_set_flags (store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
    }

  return PARSER_OK;
}

static int
https_parse_nohttps11 (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  return assign_int_range (&lst->noHTTPS11, 0, 2);
}

static PARSER_TABLE https_parsetab[] = {
  { "End", parse_end },
  { "Address", assign_address, NULL, offsetof (LISTENER, addr) },
  { "Port", assign_port, NULL, offsetof (LISTENER, addr) },
  { "SocketFrom", listener_parse_socket_from },
  { "xHTTP", listener_parse_xhttp, NULL, offsetof (LISTENER, verb) },
  { "Client", assign_timeout, NULL, offsetof (LISTENER, to) },
  { "CheckURL", listener_parse_checkurl },
  { "Err400", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_BAD_REQUEST]) },
  { "Err401", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_UNAUTHORIZED]) },
  { "Err403", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_FORBIDDEN]) },
  { "Err404", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_FOUND]) },
  { "Err413", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_PAYLOAD_TOO_LARGE]) },
  { "Err414", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_URI_TOO_LONG]) },
  { "Err500", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_INTERNAL_SERVER_ERROR]) },
  { "Err501", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_IMPLEMENTED]) },
  { "Err503", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_SERVICE_UNAVAILABLE]) },
  { "MaxRequest", assign_CONTENT_LENGTH, NULL, offsetof (LISTENER, max_req) },

  { "Rewrite", parse_rewrite, NULL, offsetof (LISTENER, rewrite) },
  { "SetHeader", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "DeleteHeader", SETFN_SVC_NAME (delete_header), NULL, offsetof (LISTENER, rewrite) },
  { "SetURL", SETFN_SVC_NAME (set_url), NULL, offsetof (LISTENER, rewrite) },
  { "SetPath", SETFN_SVC_NAME (set_path), NULL, offsetof (LISTENER, rewrite) },
  { "SetQuery", SETFN_SVC_NAME (set_query), NULL, offsetof (LISTENER, rewrite) },
  { "SetQueryParam", SETFN_SVC_NAME (set_query_param), NULL, offsetof (LISTENER, rewrite) },

  { "HeaderOption", parse_header_options, NULL, offsetof (LISTENER, header_options) },

  /* Backward compatibility */
  { "HeaderAdd", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "AddHeader", SETFN_SVC_NAME (set_header), NULL, offsetof (LISTENER, rewrite) },
  { "HeaderRemove", parse_header_remove, NULL, offsetof (LISTENER, rewrite) },
  { "HeadRemove", parse_header_remove, NULL, offsetof (LISTENER, rewrite) },

  { "RewriteLocation", parse_rewritelocation, NULL, offsetof (LISTENER, rewr_loc) },
  { "RewriteDestination", assign_bool, NULL, offsetof (LISTENER, rewr_dest) },
  { "LogLevel", parse_log_level, NULL, offsetof (LISTENER, log_level) },
  { "ForwardedHeader", assign_string, NULL, offsetof (LISTENER, forwarded_header) },
  { "TrustedIP", assign_acl, NULL, offsetof (LISTENER, trusted_ips) },
  { "Service", parse_service, NULL, offsetof (LISTENER, services) },
  { "Cert", https_parse_cert },
  { "ClientCert", https_parse_client_cert },
  { "Disable", https_parse_disable },
  { "Ciphers", https_parse_ciphers },
  { "SSLHonorCipherOrder", https_parse_honor_cipher_order },
  { "SSLAllowClientRenegotiation", https_parse_allow_client_renegotiation },
  { "CAlist", https_parse_calist },
  { "VerifyList", https_parse_verifylist },
  { "CRLlist", https_parse_crlist },
  { "NoHTTPS11", https_parse_nohttps11 },
  { NULL }
};

static int
parse_listen_https (void *call_data, void *section_data)
{
  LISTENER *lst;
  LISTENER_HEAD *list_head = call_data;
  POUND_DEFAULTS *dfl = section_data;
  struct locus_range range;
  POUND_CTX *pc;
  struct stringbuf sb;
  struct token *tok;

  if ((lst = listener_alloc (dfl)) == NULL)
    return PARSER_FAIL;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;
  else if (tok->type == T_STRING)
    {
      if (find_listener_ident (list_head, tok->str))
	{
	  conf_error ("%s", "listener name is not unique");
	  return PARSER_FAIL;
	}
      lst->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  lst->ssl_op_enable = SSL_OP_ALL;
#ifdef  SSL_OP_NO_COMPRESSION
  lst->ssl_op_enable |= SSL_OP_NO_COMPRESSION;
#endif
  lst->ssl_op_disable =
    SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION | SSL_OP_LEGACY_SERVER_CONNECT |
    SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;

  if (parser_loop (https_parsetab, lst, section_data, &range))
    return PARSER_FAIL;

  if (check_addrinfo (&lst->addr, &range, "ListenHTTPS") != PARSER_OK)
    return PARSER_FAIL;

  lst->locus = format_locus_str (&range);

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error_at_locus_range (&range, "Cert statement is missing");
      return PARSER_FAIL;
    }

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
  if (!SLIST_EMPTY (&lst->ctx_head))
    {
      SSL_CTX *ctx = SLIST_FIRST (&lst->ctx_head)->ctx;
      if (!SSL_CTX_set_tlsext_servername_callback (ctx, SNI_server_name)
	  || !SSL_CTX_set_tlsext_servername_arg (ctx, &lst->ctx_head))
	{
	  conf_openssl_error (NULL, "can't set SNI callback");
	  return PARSER_FAIL;
	}
    }
#endif

  xstringbuf_init (&sb);
  SLIST_FOREACH (pc, &lst->ctx_head, next)
    {
      SSL_CTX_set_app_data (pc->ctx, lst);
      SSL_CTX_set_mode (pc->ctx, SSL_MODE_AUTO_RETRY);
      SSL_CTX_set_options (pc->ctx, lst->ssl_op_enable);
      SSL_CTX_clear_options (pc->ctx, lst->ssl_op_disable);
      stringbuf_reset (&sb);
      stringbuf_printf (&sb, "%d-Pound-%ld", getpid (), random ());
      SSL_CTX_set_session_id_context (pc->ctx, (unsigned char *) sb.base,
				      sb.len);
      POUND_SSL_CTX_init (pc->ctx);
      SSL_CTX_set_info_callback (pc->ctx, SSLINFO_callback);
    }
  stringbuf_free (&sb);

  SLIST_PUSH (list_head, lst, next);
  return PARSER_OK;
}

static int
parse_threads_compat (void *call_data, void *section_data)
{
  int rc;
  unsigned n;

  if ((rc = assign_unsigned (&n, section_data)) != PARSER_OK)
    return rc;

  worker_min_count = worker_max_count = n;

  return PARSER_OK;
}

static int
parse_control_socket (void *call_data, void *section_data)
{
  struct addrinfo *addr = call_data;
  struct token *tok;
  struct sockaddr_un *sun;
  size_t len;

  /* Get socket address */
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  len = strlen (tok->str);
  if (len > UNIX_PATH_MAX)
    {
      conf_error_at_locus_range (&tok->locus,
				 "%s", "UNIX path name too long");
      return PARSER_FAIL;
    }

  len += offsetof (struct sockaddr_un, sun_path) + 1;
  sun = xmalloc (len);
  sun->sun_family = AF_UNIX;
  strcpy (sun->sun_path, tok->str);
  unlink_at_exit (sun->sun_path);

  addr->ai_socktype = SOCK_STREAM;
  addr->ai_family = AF_UNIX;
  addr->ai_protocol = 0;
  addr->ai_addr = (struct sockaddr *) sun;
  addr->ai_addrlen = len;

  return PARSER_OK;
}

static PARSER_TABLE control_parsetab[] = {
  { "End",         parse_end },
  { "Socket",      parse_control_socket, NULL, offsetof (LISTENER, addr) },
  { "ChangeOwner", assign_bool, NULL, offsetof (LISTENER, chowner) },
  { "Mode",        assign_mode, NULL, offsetof (LISTENER, mode) },
  { NULL }
};

static int
parse_control (void *call_data, void *section_data)
{
  struct token *tok;
  LISTENER *lst;
  SERVICE *svc;
  BACKEND *be;
  int rc;
  struct locus_range range;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;
  lst = listener_alloc (section_data);
  switch (tok->type)
    {
    case '\n':
      rc = parser_loop (control_parsetab, lst, section_data, &range);
      if (rc == PARSER_OK)
	{
	  if (lst->addr.ai_addrlen == 0)
	    {
	      conf_error_at_locus_range (&range, "%s",
					 "Socket statement is missing");
	      rc = PARSER_FAIL;
	    }
	}
      break;

    case T_STRING:
      range.beg = last_token_locus_range ()->beg;
      putback_tkn (tok);
      rc = parse_control_socket (&lst->addr, section_data);
      range.end = last_token_locus_range ()->end;
      break;

    default:
      conf_error ("expected string or newline, but found %s", token_type_str (tok->type));
      rc = PARSER_FAIL;
    }

  if (rc != PARSER_OK)
    return PARSER_FAIL;

  lst->verb = 1; /* Need PUT and DELETE methods */
  lst->locus = format_locus_str (&range);
  /* Register listener in the global listener list */
  SLIST_PUSH (&listeners, lst, next);

  /* Create service */
  XZALLOC (svc);
  lst->locus = format_locus_str (&range);
  SLIST_INIT (&svc->backends);
  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, NULL);
  svc->tot_pri = 1;
  svc->abs_pri = 1;
  svc->max_pri = 1;
  /* Register service in the listener */
  SLIST_PUSH (&lst->services, svc, next);

  /* Create backend */
  XZALLOC (be);
  be->locus = format_locus_str (&range);
  be->be_type = BE_CONTROL;
  be->priority = 1;
  pthread_mutex_init (&be->mut, NULL);
  /* Register backend in service */
  SLIST_PUSH (&svc->backends, be, next);

  return PARSER_OK;
}

static int
parse_named_backend (void *call_data, void *section_data)
{
  NAMED_BACKEND_TABLE *tab = call_data;
  struct token *tok;
  BACKEND *be;
  struct locus_range range;
  NAMED_BACKEND *olddef;
  char *name;

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  name = xstrdup (tok->str);

  be = parse_backend_internal (backend_parsetab, section_data, NULL);
  if (!be)
    return PARSER_FAIL;
  range.end = last_token_locus_range ()->end;
  if (check_addrinfo (&be->v.reg.addr, &range, "Backend") != PARSER_OK)
    return PARSER_FAIL;

  olddef = named_backend_insert (tab, name, &range, be);
  free (name);
  pthread_mutex_destroy (&be->mut);
  free (be->locus);
  free (be);
  // FIXME: free address on failure only.

  if (olddef)
    {
      conf_error_at_locus_range (&range, "redefinition of named backend %s",
				 olddef->name);
      conf_error_at_locus_range (&olddef->locus,
				 "original definition was here");
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

static int
parse_combine_headers (void *call_data, void *section_data)
{
  struct token *tok;

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type != '\n')
    {
      conf_error ("expected newline, but found %s", token_type_str (tok->type));
      return PARSER_FAIL;
    }

  for (;;)
    {
      int rc;
      if ((tok = gettkn_any ()) == NULL)
	return PARSER_FAIL;
      if (tok->type == '\n')
	continue;
      if (tok->type == T_IDENT)
	{
	  if (strcasecmp (tok->str, "end") == 0)
	    break;
	  if (strcasecmp (tok->str, "include") == 0)
	    {
	      if ((rc = parse_include (NULL, NULL)) == PARSER_FAIL)
		return rc;
	      continue;
	    }
	  conf_error ("expected quoted string, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return PARSER_FAIL;
	}
      if (tok->type == T_STRING)
	combinable_header_add (tok->str);
      else
	{
	  conf_error ("expected quoted string, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return PARSER_FAIL;
	}
    }
  return PARSER_OK;
}

static PARSER_TABLE top_level_parsetab[] = {
  { "IncludeDir", parse_includedir },
  { "User", assign_string, &user },
  { "Group", assign_string, &group },
  { "RootJail", assign_string, &root_jail },
  { "Daemon", assign_bool, &daemonize },
  { "Supervisor", assign_bool, &enable_supervisor },
  { "WorkerMinCount", assign_unsigned, &worker_min_count },
  { "WorkerMaxCount", assign_unsigned, &worker_max_count },
  { "Threads", parse_threads_compat },
  { "WorkerIdleTimeout", assign_timeout, &worker_idle_timeout },
  { "Grace", assign_timeout, &grace },
  { "LogFacility", assign_log_facility, NULL, offsetof (POUND_DEFAULTS, facility) },
  { "LogLevel", parse_log_level, NULL, offsetof (POUND_DEFAULTS, log_level) },
  { "LogFormat", parse_log_format },
  { "Alive", assign_timeout, &alive_to },
  { "Client", assign_timeout, NULL, offsetof (POUND_DEFAULTS, clnt_to) },
  { "TimeOut", assign_timeout, NULL, offsetof (POUND_DEFAULTS, be_to) },
  { "WSTimeOut", assign_timeout, NULL, offsetof (POUND_DEFAULTS, ws_to) },
  { "ConnTO", assign_timeout, NULL, offsetof (POUND_DEFAULTS, be_connto) },
  { "IgnoreCase", assign_bool, NULL, offsetof (POUND_DEFAULTS, ignore_case) },
  { "Balancer", parse_balancer, NULL, offsetof (POUND_DEFAULTS, balancer) },
  { "HeaderOption", parse_header_options, NULL, offsetof (POUND_DEFAULTS, header_options) },
  { "ECDHCurve", parse_ECDHCurve },
  { "SSLEngine", parse_SSLEngine },
  { "Control", parse_control },
  { "Anonymise", int_set_one, &anonymise },
  { "Anonymize", int_set_one, &anonymise },
  { "Service", parse_service, &services },
  { "Backend", parse_named_backend, NULL, offsetof (POUND_DEFAULTS, named_backend_table) },
  { "ListenHTTP", parse_listen_http, &listeners },
  { "ListenHTTPS", parse_listen_https, &listeners },
  { "ACL", parse_named_acl, NULL },
  { "PidFile", assign_string, &pid_name },
  { "BackendStats", assign_bool, &enable_backend_stats },
  { "ForwardedHeader", assign_string, &forwarded_header },
  { "TrustedIP", assign_acl, &trusted_ips },
  { "CombineHeaders", parse_combine_headers },
  { NULL }
};

static int
resolve_backend_ref (BACKEND *be, void *data)
{
  if (be->be_type == BE_BACKEND_REF)
    {
      NAMED_BACKEND_TABLE *tab = data;
      NAMED_BACKEND *nb;

      nb = named_backend_retrieve (tab, be->v.be_name);
      if (!nb)
	{
	  logmsg (LOG_ERR, "%s: named backend %s is not declared",
		  be->locus, be->v.be_name);
	  return -1;
	}
      free (be->v.be_name);
      be->be_type = BE_BACKEND;
      be->v.reg = nb->bereg;
      if (be->priority == -1)
	be->priority = nb->priority;
      if (be->disabled == -1)
	be->disabled = nb->disabled;
    }
  return 0;
}

/*
 * Fix-up password file structures for use in restricted chroot
 * environment.
 */
static int
cond_pass_file_fixup (SERVICE_COND *cond)
{
  int rc = 0;

  switch (cond->type)
    {
    case COND_BASIC_AUTH:
      if (cond->pwfile.filename[0] == '/')
	{
	  if (root_jail)
	    {
	      /* Split file name into directory and base name, */
	      char *p = strrchr (cond->pwfile.filename, '/');
	      if (p != NULL)
		{
		  char *dir = cond->pwfile.filename;
		  *p++ = 0;
		  cond->pwfile.filename = xstrdup (p);
		  if ((cond->pwfile.wd = workdir_get (dir)) == NULL)
		    {
		      conf_error_at_locus_range (&cond->pwfile.locus,
						 "can't open directory %s: %s",
						 dir,
						 strerror (errno));
		      free (dir);
		      rc = -1;
		      break;
		    }
		  free (dir);
		}
	    }
	}
      else
	cond->pwfile.wd = workdir_ref (include_wd);
      break;

    case COND_BOOL:
      {
	SERVICE_COND *subcond;
	SLIST_FOREACH (subcond, &cond->bool.head, next)
	  {
	    if ((rc = cond_pass_file_fixup (subcond)) != 0)
	      break;
	  }
      }
      break;

    default:
      break;
    }
  return rc;
}

static int
rule_pass_file_fixup (REWRITE_RULE *rule)
{
  int rc = 0;
  do
    {
      if ((rc = cond_pass_file_fixup (&rule->cond)) != 0)
	break;
    }
  while ((rule = rule->iffalse) != NULL);
  return rc;
}

static int
pass_file_fixup (REWRITE_RULE_HEAD *head)
{
  REWRITE_RULE *rule;
  int rc = 0;

  SLIST_FOREACH (rule, head, next)
    {
      if ((rc = rule_pass_file_fixup (rule)) != 0)
	break;
    }
  return rc;
}

static int
service_pass_file_fixup (SERVICE *svc, void *data)
{
  if (cond_pass_file_fixup (&svc->cond))
    return -1;
  return pass_file_fixup (&svc->rewrite[REWRITE_REQUEST]);
}

static int
listener_pass_file_fixup (LISTENER *lstn, void *data)
{
  return pass_file_fixup (&lstn->rewrite[REWRITE_REQUEST]);
}

int
parse_config_file (char const *file, int nosyslog)
{
  int res = -1;
  POUND_DEFAULTS pound_defaults = {
    .log_level = 1,
    .facility = LOG_DAEMON,
    .clnt_to = 10,
    .be_to = 15,
    .ws_to = 600,
    .be_connto = 15,
    .ignore_case = 0,
    .header_options = HDROPT_FORWARDED_HEADERS | HDROPT_SSL_HEADERS,
    .balancer = BALANCER_RANDOM
  };

  named_backend_table_init (&pound_defaults.named_backend_table);
  compile_canned_formats ();
  open_include_dir (NULL);
  res = push_input (file);
  workdir_unref (include_wd);
  if (res == 0)
    {
      open_include_dir (include_dir);
      res = parser_loop (top_level_parsetab, &pound_defaults, &pound_defaults, NULL);
      if (res == 0)
	{
	  if (cur_input)
	    exit (1);
	  if (foreach_backend (resolve_backend_ref,
			       &pound_defaults.named_backend_table))
	    exit (1);
	  if (worker_min_count > worker_max_count)
	    abend ("WorkerMinCount is greater than WorkerMaxCount");
	  if (!nosyslog)
	    log_facility = pound_defaults.facility;

	  if (root_jail || daemonize)
	    {
	      if (foreach_listener (listener_pass_file_fixup, NULL)
		  || foreach_service (service_pass_file_fixup, NULL))
		exit (1);
	      if (include_wd->refcount > 0 && include_wd->fd == AT_FDCWD)
		{
		  int fd = openat (include_wd->fd, ".",
				   O_RDONLY | O_NONBLOCK | O_DIRECTORY);
		  if (fd == -1)
		    {
		      logmsg (LOG_CRIT,
			      "can't open current working directory: %s",
			      strerror (errno));
		      exit (1);
		    }
		  include_wd->fd = fd;
		}
	    }
	  workdir_unref (include_wd);
	}
    }
  named_backend_table_free (&pound_defaults.named_backend_table);
  if (include_wd->refcount == 0)
    include_wd = NULL;
  workdir_cleanup ();
  return res;
}

enum
  {
    F_OFF,
    F_ON,
    F_DFL
  };

struct pound_feature
{
  char *name;
  char *descr;
  int enabled;
  void (*setfn) (int, char const *);
};

static void
set_include_dir (int enabled, char const *val)
{
  if (enabled)
    {
      if (val && (*val == 0 || strcmp (val, ".") == 0))
	val = NULL;
      include_dir = val;
    }
  else
    include_dir = NULL;
}

static struct pound_feature feature[] = {
  [FEATURE_DNS] = {
    .name = "dns",
    .descr = "resolve host names found in configuration file (default)",
    .enabled = F_ON
  },
  [FEATURE_INCLUDE_DIR] = {
    .name = "include-dir",
    .descr = "include file directory",
    .enabled = F_DFL,
    .setfn = set_include_dir
  },
  { NULL }
};

int
feature_is_set (int f)
{
  return feature[f].enabled;
}

static int
feature_set (char const *name)
{
  int i, enabled = F_ON;
  size_t len;
  char *val;

  if ((val = strchr (name, '=')) != NULL)
    {
      len = val - name;
      val++;
    }
  else
    len = strlen (name);

  if (val == NULL && strncmp (name, "no-", 3) == 0)
    {
      name += 3;
      len -= 3;
      enabled = F_OFF;
    }

  if (*name)
    {
      for (i = 0; feature[i].name; i++)
	{
	  if (strlen (feature[i].name) == len &&
	      memcmp (feature[i].name, name, len) == 0)
	    {
	      if (feature[i].setfn)
		feature[i].setfn (enabled, val);
	      else if (val)
		break;
	      feature[i].enabled = enabled;
	      return 0;
	    }
	}
    }
  return -1;
}

struct string_value pound_settings[] = {
  { "Configuration file",  STRING_CONSTANT, { .s_const = POUND_CONF } },
  { "Include directory",   STRING_CONSTANT, { .s_const = SYSCONFDIR } },
  { "PID file",   STRING_CONSTANT,  { .s_const = POUND_PID } },
  { "Buffer size",STRING_INT, { .s_int = MAXBUF } },
#if ! SET_DH_AUTO
  { "DH bits",         STRING_INT, { .s_int = DH_LEN } },
  { "RSA regeneration interval", STRING_INT, { .s_int = T_RSA_KEYS } },
#endif
  { NULL }
};

void
print_help (void)
{
  int i;

  printf ("usage: %s [-FVcehv] [-W [no-]FEATURE] [-f FILE] [-p FILE]\n", progname);
  printf ("HTTP/HTTPS reverse-proxy and load-balancer\n");
  printf ("\nOptions are:\n\n");
  printf ("   -c               check configuration file syntax and exit\n");
  printf ("   -e               print errors on stderr (implies -F)\n");
  printf ("   -F               remain in foreground after startup\n");
  printf ("   -f FILE          read configuration from FILE\n");
  printf ("                    (default: %s)\n", POUND_CONF);
  printf ("   -p FILE          write PID to FILE\n");
  printf ("                    (default: %s)\n", POUND_PID);
  printf ("   -V               print program version, compilation settings, and exit\n");
  printf ("   -v               print log messages to stdout/stderr during startup\n");
  printf ("   -W [no-]FEATURE  enable or disable optional feature\n");
  printf ("\n");
  printf ("FEATUREs are:\n");
  for (i = 0; feature[i].name; i++)
    printf ("   %-16s %s\n", feature[i].name, feature[i].descr);
  printf ("\n");
  printf ("Report bugs and suggestions to <%s>\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_URL
  printf ("%s home page: <%s>\n", PACKAGE_NAME, PACKAGE_URL);
#endif
}

void
config_parse (int argc, char **argv)
{
  int c;
  int check_only = 0;
  char *conf_name = POUND_CONF;
  char *pid_file_option = NULL;
  int foreground_option = 0;
  int stderr_option = 0;

  set_progname (argv[0]);

  while ((c = getopt (argc, argv, "ceFf:hp:VvW:")) > 0)
    switch (c)
      {
      case 'c':
	check_only = 1;
	break;

      case 'e':
	stderr_option = foreground_option = 1;
	break;

      case 'F':
	foreground_option = 1;
	break;

      case 'f':
	conf_name = optarg;
	break;

      case 'h':
	print_help ();
	exit (0);

      case 'p':
	pid_file_option = optarg;
	break;

      case 'V':
	print_version (pound_settings);
	exit (0);

      case 'v':
	print_log = 1;
	break;

      case 'W':
	if (feature_set (optarg))
	  {
	    logmsg (LOG_ERR, "invalid feature name: %s", optarg);
	    exit (1);
	  }
	break;

      default:
	exit (1);
      }

  if (optind < argc)
    {
      logmsg (LOG_ERR, "unknown extra arguments (%s...)", argv[optind]);
      exit (1);
    }

  if (parse_config_file (conf_name, stderr_option))
    exit (1);

  if (check_only)
    {
      logmsg (LOG_INFO, "Config file %s is OK", conf_name);
      exit (0);
    }

  if (SLIST_EMPTY (&listeners))
    abend ("no listeners defined");

  if (pid_file_option)
    pid_name = pid_file_option;

  if (foreground_option)
    daemonize = 0;

  if (daemonize)
    {
      if (log_facility == -1)
	log_facility = LOG_DAEMON;
    }
}

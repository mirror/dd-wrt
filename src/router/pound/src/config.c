/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2022 Sergey Poznyakoff
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

char *progname;

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
#define T_ANY 0 /* any token, inlcuding newline */
/* Unquoted character sequence */
#define T_UNQ (T_BIT (T_IDENT) | T_BIT (T_NUMBER) | T_BIT(T_LITERAL))

/* Locations in the source file */
struct locus_point
{
  char const *filename;
  int line;
  int col;
};

struct locus_range
{
  struct locus_point beg, end;
};

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
  int ready;                      /* Token already parsed and put back */

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

  stringbuf_init (&sb);
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

  stringbuf_init (&sb);
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
openssl_error_at_locus_range (struct locus_range const *loc, char const *msg)
{
  unsigned long n = ERR_get_error ();
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
  if ((input->file = fopen (filename, "r")) == 0)
    {
      logmsg (LOG_ERR, "can't open %s: %s", filename, strerror (errno));
      free (input);
      return NULL;
    }
  input->ino = st->st_ino;
  input->devno = st->st_dev;
  input->locus.filename = name_alloc (filename);
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

  if (input->ready)
    {
      input->ready = 0;
      *tok = &input->token;
      return input->token.type;
    }

  stringbuf_reset (&input->buf);
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
	  stringbuf_add_char (&input->buf, 0);
	  input->token.type = T_STRING;
	  input->token.str = input->buf.base;
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
	      stringbuf_add_char (&input->buf, 0);
	      input->token.type = T_IDENT;
	      input->token.str = input->buf.base;
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
      stringbuf_add_char (&input->buf, 0);
      input->token.str = input->buf.base;
      break;
    }
 end:
  input->token.locus.end = input->locus;
  *tok = &input->token;
  return input->token.type;
}

static void
input_putback (struct input *input)
{
  assert (input->ready == 0);
  input->ready = 1;
}


struct input *cur_input;

static struct locus_range *
last_token_locus_range (void)
{
  if (cur_input)
    return &cur_input->token.locus;
  else
    return NULL;
}

#define conf_error(fmt, ...) \
  conf_error_at_locus_range (last_token_locus_range (), fmt, __VA_ARGS__)

#define conf_regcomp_error(rc, rx, expr) \
  regcomp_error_at_locus_range (last_token_locus_range (), rc, rx, expr)

#define conf_openssl_error(msg) \
  openssl_error_at_locus_range (last_token_locus_range (), msg)

static int
push_input (const char *filename)
{
  struct stat st;
  struct input *input;

  if (stat (filename, &st))
    {
      conf_error ("can't stat %s: %s", filename, strerror (errno));
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
putback_tkn (void)
{
  input_putback (cur_input);
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


typedef struct
{
  int log_level;
  int facility;
  unsigned clnt_to;
  unsigned be_to;
  unsigned ws_to;
  unsigned be_connto;
  unsigned ignore_case;
} POUND_DEFAULTS;

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
  if (stat (tok->str, &st))
    {
      conf_error ("can't stat %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
  // FIXME: Check st_size bounds.
  s = xmalloc (st.st_size + 1);
  if ((fp = fopen (tok->str, "r")) == NULL)
    {
      conf_error ("can't open %s: %s", tok->str, strerror (errno));
      return PARSER_FAIL;
    }
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
assign_LONG (void *call_data, void *section_data)
{
  LONG n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return PARSER_FAIL;

  errno = 0;
  n = STRTOL (tok->str, &p, 10);
  if (errno || *p)
    {
      conf_error ("%s", "bad long number");
      return PARSER_FAIL;
    }
  *(LONG *)call_data = n;
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
 * There shouldn't be many of them so it's perhaps no use in implementing
 * more sophisticated data structures than a mere singly-linked list.
 */
static ACL_HEAD acl_list = SLIST_HEAD_INITIALIZER (acl_list);

/*
 * Return a pointer to the named ACL or NULL if no ACL with such name is found.
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
      if (tok->type == T_IDENT && strcasecmp (tok->str, "end") == 0)
	break;
      putback_tkn ();
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
      putback_tkn ();
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

  if ((be->ctx = SSL_CTX_new (SSLv23_client_method ())) == NULL)
    {
      conf_openssl_error ("SSL_CTX_new");
      return PARSER_FAIL;
    }

  SSL_CTX_set_app_data (be->ctx, be);
  SSL_CTX_set_verify (be->ctx, SSL_VERIFY_NONE, NULL);
  SSL_CTX_set_mode (be->ctx, SSL_MODE_AUTO_RETRY);
#ifdef SSL_MODE_SEND_FALLBACK_SCSV
  SSL_CTX_set_mode (be->ctx, SSL_MODE_SEND_FALLBACK_SCSV);
#endif
  SSL_CTX_set_options (be->ctx, SSL_OP_ALL);
#ifdef  SSL_OP_NO_COMPRESSION
  SSL_CTX_set_options (be->ctx, SSL_OP_NO_COMPRESSION);
#endif
  SSL_CTX_clear_options (be->ctx,
			 SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);
  SSL_CTX_clear_options (be->ctx, SSL_OP_LEGACY_SERVER_CONNECT);

  stringbuf_init (&sb);
  stringbuf_printf (&sb, "%d-Pound-%ld", getpid (), random ());
  SSL_CTX_set_session_id_context (be->ctx, (unsigned char *) sb.base, sb.len);
  stringbuf_free (&sb);

  POUND_SSL_CTX_init (be->ctx);

  return PARSER_OK;
}

static int
backend_parse_cert (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  if (SSL_CTX_use_certificate_chain_file (be->ctx, tok->str) != 1)
    {
      conf_openssl_error ("SSL_CTX_use_certificate_chain_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_use_PrivateKey_file (be->ctx, tok->str, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error ("SSL_CTX_use_PrivateKey_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (be->ctx) != 1)
    {
      conf_openssl_error ("SSL_CTX_check_private_key failed");
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

static int
backend_assign_ciphers (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  SSL_CTX_set_cipher_list (be->ctx, tok->str);
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
  SSL_CTX *ctx = call_data;
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
  {
    .name = "End",
    .parser = parse_end
  },
  {
    .name = "Address",
    .parser = assign_address,
    .off = offsetof (BACKEND, addr)
  },
  {
    .name = "Port",
    .parser = assign_port,
    .off = offsetof (BACKEND, addr)
  },
  {
    .name = "Priority",
    .parser = backend_assign_priority,
    .off = offsetof (BACKEND, priority)
  },
  {
    .name = "TimeOut",
    .parser = assign_timeout,
    .off = offsetof (BACKEND, to)
  },
  {
    .name = "WSTimeOut",
    .parser = assign_timeout,
    .off = offsetof (BACKEND, ws_to)
  },
  {
    .name = "ConnTO",
    .parser = assign_timeout,
    .off = offsetof (BACKEND, conn_to)
  },
  {
    .name = "HTTPS",
    .parser = backend_parse_https
  },
  {
    .name = "Cert",
    .parser = backend_parse_cert
  },
  {
    .name = "Ciphers",
    .parser = backend_assign_ciphers
  },
  {
    .name = "Disable",
    .parser = disable_proto,
    .off = offsetof (BACKEND, ctx)
  },
  {
    .name = "Disabled",
    .parser = assign_bool,
    .off = offsetof (BACKEND, disabled)
  },
  { NULL }
};

static PARSER_TABLE emergency_parsetab[] = {
  { "End", parse_end },
  { "Address", assign_address, NULL, offsetof (BACKEND, addr) },
  { "Port", assign_port, NULL, offsetof (BACKEND, addr) },
  { "TimeOut", assign_timeout, NULL, offsetof (BACKEND, to) },
  { "WSTimeOut", assign_timeout, NULL, offsetof (BACKEND, ws_to) },
  { "ConnTO", assign_timeout, NULL, offsetof (BACKEND, conn_to) },
  { "HTTPS", backend_parse_https },
  { "Cert", backend_parse_cert },
  { "Ciphers", backend_assign_ciphers },
  { "Disable", disable_proto, NULL, offsetof (BACKEND, ctx) },
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

static BACKEND *
parse_backend_internal (PARSER_TABLE *table, POUND_DEFAULTS *dfl)
{
  BACKEND *be;
  struct locus_range range;

  XZALLOC (be);
  be->be_type = BE_BACKEND;
  be->addr.ai_socktype = SOCK_STREAM;
  be->to = dfl->be_to;
  be->conn_to = dfl->be_connto;
  be->ws_to = dfl->ws_to;
  be->alive = 1;
  memset (&be->addr, 0, sizeof (be->addr));
  be->priority = 5;
  be->url = NULL;
  pthread_mutex_init (&be->mut, NULL);

  if (parser_loop (table, be, dfl, &range))
    return NULL;

  if (check_addrinfo (&be->addr, &range, "Backend") != PARSER_OK)
    return NULL;

  return be;
}

static int
parse_backend (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  BACKEND *be;

  be = parse_backend_internal (backend_parsetab, section_data);
  if (!be)
    return PARSER_FAIL;

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

  be = parse_backend_internal (emergency_parsetab, &dfl);
  if (!be)
    return PARSER_FAIL;

  *res_ptr = be;

  return PARSER_OK;
}

static int
parse_regex (regex_t *re, int flags)
{
  int rc;

  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;

  rc = regcomp (re, tok->str, flags);
  if (rc)
    {
      conf_regcomp_error (rc, re, NULL);
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

static int
assign_matcher (void *call_data, void *section_data)
{
  MATCHER_HEAD *head = call_data;
  MATCHER *m;
  int rc;

  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return PARSER_FAIL;

  XZALLOC (m);
  rc = regcomp (&m->pat, tok->str, REG_ICASE | REG_NEWLINE | REG_EXTENDED);
  if (rc)
    {
      conf_regcomp_error (rc, &m->pat, NULL);
      return PARSER_FAIL;
    }
  SLIST_PUSH (head, m, next);

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
  SERVICE_COND *cond = service_cond_append (call_data, COND_URL);
  int flags = REG_NEWLINE | REG_EXTENDED | (dfl->ignore_case ? REG_ICASE : 0);
  return parse_regex (&cond->re, flags);
}

static int
parse_cond_hdr_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_HDR);
  return parse_regex (&cond->re, REG_NEWLINE | REG_EXTENDED | REG_ICASE);
}

static int
parse_cond_head_deny_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BOOL);
  cond->bool.op = BOOL_NOT;
  cond = service_cond_append (cond, COND_HDR);
  return parse_regex (&cond->re, REG_NEWLINE | REG_EXTENDED | REG_ICASE);
}

static int
parse_cond_host (void *call_data, void *section_data)
{
  struct token *tok;
  struct stringbuf sb;
  char *p;
  int rc;
  SERVICE_COND *cond = service_cond_append (call_data, COND_HDR);

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  stringbuf_init (&sb);
  stringbuf_add_string (&sb, "Host:[[:space:]]*");
  p = tok->str;
  while (*p)
    {
      size_t len = strcspn (p, "\\[]{}().*+?");
      if (len > 0)
	stringbuf_add (&sb, p, len);
      p += len;
      if (*p)
	{
	  stringbuf_add_char (&sb, '\\');
	  stringbuf_add_char (&sb, *p);
	  p++;
	}
    }

  p = stringbuf_finish (&sb);

  rc = regcomp (&cond->re, p, REG_EXTENDED | REG_ICASE);
  stringbuf_free (&sb);
  if (rc)
    {
      conf_regcomp_error (rc, &cond->re, NULL);
      return PARSER_FAIL;
    }

  return PARSER_OK;
}

static int
assign_redirect (void *call_data, void *section_data)
{
  BACKEND_HEAD *head = call_data;
  struct token *tok;
  int code = 302;
  BACKEND *be;
  regmatch_t matches[5];

  if ((tok = gettkn_any ()) == NULL)
    return PARSER_FAIL;

  if (tok->type == T_NUMBER)
    {
      int n = atoi (tok->str);
      if (n == 301 || n == 302 || n == 307)
	code = n;
      else
	{
	  conf_error ("%s", "invalid status code");
	  return PARSER_FAIL;
	}

      if ((tok = gettkn_any ()) == NULL)
	return PARSER_FAIL;
    }

  if (tok->type != T_STRING)
    {
      conf_error ("expected %s, but found %s", token_type_str (T_STRING), token_type_str (tok->type));
      return PARSER_FAIL;
    }

  XZALLOC (be);
  be->be_type = BE_REDIRECT;
  be->redir_code = code;
  be->priority = 1;
  be->alive = 1;
  pthread_mutex_init (&be->mut, NULL);
  be->url = xstrdup (tok->str);

  if (regexec (&LOCATION, be->url, 4, matches, 0))
    {
      conf_error ("%s", "Redirect bad URL");
      return PARSER_FAIL;
    }

  if ((be->redir_req = matches[3].rm_eo - matches[3].rm_so) == 1)
    /* the path is a single '/', so remove it */
    be->url[matches[3].rm_so] = '\0';

  SLIST_PUSH (head, be, next);

  return PARSER_OK;
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
  struct service_session *sp = call_data;
  struct token *tok;
  int n;

  if ((tok = gettkn_expect (T_IDENT)) == NULL)
    return PARSER_FAIL;

  if (kw_to_tok (sess_type_tab, tok->str, 1, &n))
    {
      conf_error ("%s", "Unknown Session type");
      return PARSER_FAIL;
    }
  sp->type = n;

  return PARSER_OK;
}

static PARSER_TABLE session_parsetab[] = {
  { "End", parse_end },
  { "Type", session_type_parser },
  { "TTL", assign_timeout, NULL, offsetof (struct service_session, ttl) },
  { "ID", assign_string, NULL, offsetof (struct service_session, id) },
  { NULL }
};

static int
xregcomp (regex_t *rx, char const *expr, int flags)
{
  int rc;

  rc = regcomp (rx, expr, flags);
  if (rc)
    {
      conf_regcomp_error (rc, rx, expr);
      return PARSER_FAIL;
    }
  return PARSER_OK;
}

static int
parse_session (void *call_data, void *section_data)
{
  SERVICE *svc = call_data;
  struct service_session sess;
  struct stringbuf sb;
  struct locus_range range;

  memset (&sess, 0, sizeof (sess));
  if (parser_loop (session_parsetab, &sess, section_data, &range))
    return PARSER_FAIL;

  if (sess.type == SESS_NONE)
    {
      conf_error_at_locus_range (&range, "Session type not defined");
      return PARSER_FAIL;
    }

  if (sess.ttl == 0)
    {
      conf_error_at_locus_range (&range, "Session TTL not defined");
      return PARSER_FAIL;
    }

  if ((sess.type == SESS_COOKIE || sess.type == SESS_URL
       || sess.type == SESS_HEADER) && sess.id == NULL)
    {
      conf_error ("%s", "Session ID not defined");
      return PARSER_FAIL;
    }

  stringbuf_init (&sb);
  switch (sess.type)
    {
    case SESS_COOKIE:
      stringbuf_printf (&sb, "Cookie[^:]*:.*[ \t]%s=", sess.id);
      if (xregcomp (&svc->sess_start, sb.base, REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;

      if (xregcomp (&svc->sess_pat, "([^;]*)", REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      break;

    case SESS_URL:
      stringbuf_printf (&sb, "[?&]%s=", sess.id);
      if (xregcomp (&svc->sess_start, sb.base, REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      if (xregcomp (&svc->sess_pat, "([^&;#]*)", REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      break;

    case SESS_PARM:
      if (xregcomp (&svc->sess_start, ";", REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      if (xregcomp (&svc->sess_pat, "([^?]*)", REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      break;

    case SESS_BASIC:
      if (xregcomp (&svc->sess_start, "Authorization:[ \t]*Basic[ \t]*",
		    REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      if (xregcomp (&svc->sess_pat, "([^ \t]*)",
		    REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      break;

    case SESS_HEADER:
      stringbuf_printf (&sb, "%s:[ \t]*", sess.id);
      if (xregcomp (&svc->sess_start, sb.base,
		    REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      if (xregcomp (&svc->sess_pat, "([^ \t]*)",
		    REG_ICASE | REG_NEWLINE | REG_EXTENDED) != PARSER_OK)
	return PARSER_FAIL;
      break;
    }

  svc->sess_ttl = sess.ttl;
  svc->sess_type = sess.type;

  free (sess.id);
  stringbuf_free (&sb);

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
parse_and_cond (void *call_data, void *section_data)
{
  return parse_cond (BOOL_AND, call_data, section_data);
}

static int
parse_or_cond (void *call_data, void *section_data)
{
  return parse_cond (BOOL_OR, call_data, section_data);
}

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
    putback_tkn ();

  return parse_cond (op, call_data, section_data);
}

static int parse_not_cond (void *call_data, void *section_data);

static PARSER_TABLE negate_parsetab[] = {
  { "ACL", parse_cond_acl },
  { "URL", parse_cond_url_matcher },
  { "Header", parse_cond_hdr_matcher },
  { "HeadRequire", parse_cond_hdr_matcher },    /* compatibility keyword */
  { "HeadDeny", parse_cond_head_deny_matcher }, /* compatibility keyword */
  { "Host", parse_cond_host },
  { "Match", parse_match },
  { "AND", parse_and_cond, },
  { "OR", parse_or_cond, },
  { "NOT", parse_not_cond, },
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
  { "ACL", parse_cond_acl },
  { "URL", parse_cond_url_matcher },
  { "Header", parse_cond_hdr_matcher },
  { "HeadRequire", parse_cond_hdr_matcher },    /* compatibility keyword */
  { "HeadDeny", parse_cond_head_deny_matcher }, /* compatibility keyword */
  { "Host", parse_cond_host },
  { "Match", parse_match },
  { "AND", parse_and_cond, },
  { "OR", parse_or_cond, },
  { "NOT", parse_not_cond, },
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

static PARSER_TABLE service_parsetab[] = {
  { "End", parse_end },
  { "ACL", parse_cond_acl, NULL, offsetof (SERVICE, cond) },
  { "URL", parse_cond_url_matcher, NULL, offsetof (SERVICE, cond) },
  { "Header", parse_cond_hdr_matcher, NULL, offsetof (SERVICE, cond) },
  /* compatibility keyword: */
  { "HeadRequire", parse_cond_hdr_matcher, NULL, offsetof (SERVICE, cond) },
  /* compatibility keyword: */
  { "HeadDeny", parse_cond_head_deny_matcher, NULL, offsetof (SERVICE, cond) },
  { "Host", parse_cond_host, NULL, offsetof (SERVICE, cond) },
  { "Match", parse_match, NULL, offsetof (SERVICE, cond) },
  { "AND", parse_and_cond, NULL, offsetof (SERVICE, cond) },
  { "OR", parse_or_cond, NULL, offsetof (SERVICE, cond) },
  { "NOT", parse_not_cond, NULL, offsetof (SERVICE, cond) },
  { "IgnoreCase", assign_dfl_ignore_case },
  { "Disabled", assign_bool, NULL, offsetof (SERVICE, disabled) },
  { "Redirect", assign_redirect, NULL, offsetof (SERVICE, backends) },
  { "Backend", parse_backend, NULL, offsetof (SERVICE, backends) },
  { "Emergency", parse_emergency, NULL, offsetof (SERVICE, emergency) },
  { "Session", parse_session },
  { NULL }
};

static int
parse_service (void *call_data, void *section_data)
{
  SERVICE_HEAD *head = call_data;
  POUND_DEFAULTS dfl = *(POUND_DEFAULTS*) section_data;
  struct token *tok;
  SERVICE *svc;
  struct locus_range range;

  XZALLOC (svc);
  service_cond_init (&svc->cond, COND_BOOL);
  SLIST_INIT (&svc->backends);

  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, NULL);

  tok = gettkn_any ();

  if (!tok)
    return PARSER_FAIL;

  if (tok->type == T_STRING)
    {
      if (strlen (tok->str) > sizeof (svc->name) - 1)
	{
	  conf_error ("%s", "service name too long: truncated");
	}
      strncpy (svc->name, tok->str, sizeof (svc->name) - 1);
    }
  else
    putback_tkn ();

  if ((svc->sessions = session_table_new ()) == NULL)
    {
      conf_error ("%s", "session_table_new failed");
      return -1;
    }

  if (parser_loop (service_parsetab, svc, &dfl, &range))
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
	  SLIST_FOREACH (be, &svc->backends, next)
	    {
	      be->service = svc;
	      if (!be->disabled)
		svc->tot_pri += be->priority;
	      svc->abs_pri += be->priority;
	    }
	}

      SLIST_PUSH (head, svc, next);
    }
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
  size_t len;
  int rc;
  static char re_acme[] = "^/\\.well-known/acme-challenge/(.+)";
  static char suf_acme[] = "/$1";
  static size_t suf_acme_size = sizeof (suf_acme) - 1;

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

  svc->tot_pri = 1;
  svc->abs_pri = 1;

  /* Create ACME backend */
  XZALLOC (be);
  be->be_type = BE_ACME;
  be->priority = 1;
  be->alive = 1;
  pthread_mutex_init (&be->mut, NULL);

  len = strlen (tok->str);
  if (tok->str[len-1] == '/')
    len--;

  be->url = xmalloc (len + suf_acme_size + 1);
  memcpy (be->url, tok->str, len);
  strcpy (be->url + len, suf_acme);

  /* Register backend in service */
  SLIST_PUSH (&svc->backends, be, next);

  /* Register service in the listener */
  SLIST_PUSH (head, svc, next);

  return PARSER_OK;
}


static int
listener_parse_xhttp (void *call_data, void *section_data)
{
  return assign_int_range (call_data, 0, 4);
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

    stringbuf_init (&sb);
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

static int
append_string_line (void *call_data, void *section_data)
{
  char **dst = call_data;
  char *s = *dst;
  size_t len = s ? strlen (s) : 0;
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  s = xrealloc (s, len + strlen (tok->str) + 3);
  if (len == 0)
    strcpy (s, tok->str);
  else
    {
      strcpy (s + len, "\r\n");
      strcpy (s + len + 2, tok->str);
    }
  *dst = s;

  return PARSER_OK;
}

static int
parse_log_level (void *call_data, void *section_data)
{
  return assign_int_range (call_data, 0, 5);
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
  { "Err404", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_FOUND]) },
  { "Err413", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_PAYLOAD_TOO_LARGE]) },
  { "Err414", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_URI_TOO_LONG]) },
  { "Err500", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_INTERNAL_SERVER_ERROR]) },
  { "Err501", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_IMPLEMENTED]) },
  { "Err503", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_SERVICE_UNAVAILABLE]) },
  { "MaxRequest", assign_LONG, NULL, offsetof (LISTENER, max_req) },
  { "HeaderRemove", assign_matcher, NULL, offsetof (LISTENER, head_off) },
  { "HeadRemove", assign_matcher, NULL, offsetof (LISTENER, head_off) },
  { "RewriteLocation", parse_rewritelocation, NULL, offsetof (LISTENER, rewr_loc) },
  { "RewriteDestination", assign_bool, NULL, offsetof (LISTENER, rewr_dest) },
  { "LogLevel", parse_log_level, NULL, offsetof (LISTENER, log_level) },
  { "HeaderAdd", append_string_line, NULL, offsetof (LISTENER, add_head) },
  { "AddHeader", append_string_line, NULL, offsetof (LISTENER, add_head) },
  { "Service", parse_service, NULL, offsetof (LISTENER, services) },
  { "ACME", parse_acme, NULL, offsetof (LISTENER, services) },
  { NULL }
};

static LISTENER *
listener_alloc (POUND_DEFAULTS *dfl)
{
  LISTENER *lst;

  XZALLOC (lst);

  lst->sock = -1;
  lst->to = dfl->clnt_to;
  lst->rewr_loc = 1;
  lst->log_level = dfl->log_level;
  lst->verb = 0;
  SLIST_INIT (&lst->head_off);
  SLIST_INIT (&lst->services);
  SLIST_INIT (&lst->ctx_head);
  return lst;
}

static int
parse_listen_http (void *call_data, void *section_data)
{
  LISTENER *lst;
  LISTENER_HEAD *list_head = call_data;
  POUND_DEFAULTS *dfl = section_data;
  struct locus_range range;

  if ((lst = listener_alloc (dfl)) == NULL)
    return PARSER_FAIL;

  if (parser_loop (http_parsetab, lst, section_data, &range))
    return PARSER_FAIL;

  if (check_addrinfo (&lst->addr, &range, "ListenHTTP") != PARSER_OK)
    return PARSER_FAIL;

  SLIST_PUSH (list_head, lst, next);
  return PARSER_OK;
}

static int
is_class (int c, char *cls)
{
  int k;

  if (*cls == 0)
    return 0;
  if (c == *cls)
    return 1;
  cls++;
  while ((k = *cls++) != 0)
    {
      if (k == '-' && cls[0] != 0)
	{
	  if (cls[-2] <= c && c <= cls[0])
	    return 1;
	  cls++;
	}
      else if (c == k)
	return 1;
    }
  return 0;
}

static char *
extract_cn (char const *str, size_t *plen)
{
  while (*str)
    {
      if ((str[0] == 'c' || str[0] == 'C') && (str[1] == 'n' || str[1] == 'N') && str[2] == '=')
	{
	  size_t i;
	  str += 3;
	  for (i = 0; str[i] && is_class (str[i], "-*.A-Za-z0-9"); i++)
	    ;
	  if (str[i] == 0)
	    {
	      *plen = i;
	      return (char*) str;
	    }
	  str += i;
	}
      str++;
    }
  return NULL;
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
# define general_name_string(n) \
	(unsigned char*) \
	xstrndup ((char*)ASN1_STRING_get0_data (n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#else
# define general_name_string(n) \
	(unsigned char*) \
	xstrndup ((char*)ASN1_STRING_data(n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#endif

unsigned char **
get_subjectaltnames (X509 * x509, unsigned int *count)
{
  unsigned int local_count;
  unsigned char **result;
  STACK_OF (GENERAL_NAME) * san_stack =
    (STACK_OF (GENERAL_NAME) *) X509_get_ext_d2i (x509, NID_subject_alt_name,
						  NULL, NULL);
  unsigned char *temp[sk_GENERAL_NAME_num (san_stack)];
  GENERAL_NAME *name;
  int i;

  local_count = 0;
  result = NULL;
  name = NULL;
  *count = 0;
  if (san_stack == NULL)
    return NULL;
  while (sk_GENERAL_NAME_num (san_stack) > 0)
    {
      name = sk_GENERAL_NAME_pop (san_stack);
      switch (name->type)
	{
	case GEN_DNS:
	  temp[local_count++] = general_name_string (name);
	  break;

	default:
	  logmsg (LOG_INFO, "unsupported subjectAltName type encountered: %i",
		  name->type);
	}
      GENERAL_NAME_free (name);
    }

  result = xcalloc (local_count, sizeof (unsigned char *));
  for (i = 0; i < local_count; i++)
    result[i] = temp[i];
  *count = local_count;

  sk_GENERAL_NAME_pop_free (san_stack, GENERAL_NAME_free);

  return result;
}

static int
https_parse_cert (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  POUND_CTX *pc;

  if (lst->has_other)
    {
      conf_error ("%s", "Cert directives MUST precede other SSL-specific directives");
      return PARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  XZALLOC (pc);

  if ((pc->ctx = SSL_CTX_new (SSLv23_server_method ())) == NULL)
    {
      conf_openssl_error ("SSL_CTX_new");
      return PARSER_FAIL;
    }

  if (SSL_CTX_use_certificate_chain_file (pc->ctx, tok->str) != 1)
    {
      conf_openssl_error ("SSL_CTX_use_certificate_chain_file");
      return PARSER_FAIL;
    }
  if (SSL_CTX_use_PrivateKey_file (pc->ctx, tok->str, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error ("SSL_CTX_use_PrivateKey_file");
      return PARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (pc->ctx) != 1)
    {
      conf_openssl_error ("SSL_CTX_check_private_key");
      return PARSER_FAIL;
    }

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
  {
    /* we have support for SNI */
    FILE *fcert;
    char server_name[MAXBUF];
    X509 *x509;
    char *cnp;
    size_t cnl;

    if ((fcert = fopen (tok->str, "r")) == NULL)
      {
	conf_error ("%s", "ListenHTTPS: could not open certificate file");
	return PARSER_FAIL;
      }

    x509 = PEM_read_X509 (fcert, NULL, NULL, NULL);
    fclose (fcert);

    if (!x509)
      {
	conf_error ("%s", "could not get certificate subject");
	return PARSER_FAIL;
      }

    memset (server_name, '\0', MAXBUF);
    X509_NAME_oneline (X509_get_subject_name (x509), server_name,
		       sizeof (server_name) - 1);
    pc->subjectAltNameCount = 0;
    pc->subjectAltNames = NULL;
    pc->subjectAltNames = get_subjectaltnames (x509, &pc->subjectAltNameCount);
    X509_free (x509);

    if ((cnp = extract_cn (server_name, &cnl)) == NULL)
      {
	conf_error ("no CN in certificate subject name (%s)\n", server_name);
	return PARSER_FAIL;
      }
    pc->server_name = xmalloc (cnl + 1);
    memcpy (pc->server_name, cnp, cnl);
    pc->server_name[cnl] = 0;
  }
#else
  if (res->ctx)
    conf_error ("%s", "multiple certificates not supported");
#endif
  SLIST_PUSH (&lst->ctx_head, pc, next);

  return PARSER_OK;
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
      conf_openssl_error ("SSL_load_client_CA_file");
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
	conf_openssl_error ("SSL_CTX_load_verify_locations");
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
	  conf_openssl_error ("X509_STORE_add_lookup");
	  return PARSER_FAIL;
	}

      if (X509_load_crl_file (lookup, tok->str, X509_FILETYPE_PEM) != 1)
	{
	  conf_openssl_error ("X509_load_crl_file failed");
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
  { "Err404", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_FOUND]) },
  { "Err413", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_PAYLOAD_TOO_LARGE]) },
  { "Err414", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_URI_TOO_LONG]) },
  { "Err500", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_INTERNAL_SERVER_ERROR]) },
  { "Err501", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_NOT_IMPLEMENTED]) },
  { "Err503", assign_string_from_file, NULL, offsetof (LISTENER, http_err[HTTP_STATUS_SERVICE_UNAVAILABLE]) },
  { "MaxRequest", assign_LONG, NULL, offsetof (LISTENER, max_req) },
  { "HeaderRemove", assign_matcher, NULL, offsetof (LISTENER, head_off) },
  { "HeadRemove", assign_matcher, NULL, offsetof (LISTENER, head_off) },
  { "RewriteLocation", parse_rewritelocation, NULL, offsetof (LISTENER, rewr_loc) },
  { "RewriteDestination", assign_bool, NULL, offsetof (LISTENER, rewr_dest) },
  { "LogLevel", parse_log_level, NULL, offsetof (LISTENER, log_level) },
  { "HeaderAdd", append_string_line, NULL, offsetof (LISTENER, add_head) },
  { "AddHeader", append_string_line, NULL, offsetof (LISTENER, add_head) },
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

  if ((lst = listener_alloc (dfl)) == NULL)
    return PARSER_FAIL;

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
	  conf_openssl_error ("can't set SNI callback");
	  return PARSER_FAIL;
	}
    }
#endif

  stringbuf_init (&sb);
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
parse_control_global (void *call_data, void *section_data)
{
  struct token *tok;
  LISTENER *lst;
  SERVICE *svc;
  BACKEND *be;
  struct sockaddr_un *sun;
  size_t len;

  /* Get socket address */
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return PARSER_FAIL;

  /* Create listener for that address */
  lst = listener_alloc (section_data);

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
  pound_atexit (unlink_file, sun->sun_path);

  lst->addr.ai_socktype = SOCK_STREAM;
  lst->addr.ai_family = AF_UNIX;
  lst->addr.ai_protocol = 0;
  lst->addr.ai_addr = (struct sockaddr *) sun;
  lst->addr.ai_addrlen = len;

  lst->verb = 1; /* Need PUT and DELETE methods */
  /* Register listener in the global listener list */
  SLIST_PUSH (&listeners, lst, next);

  /* Create service */
  XZALLOC (svc);
  SLIST_INIT (&svc->backends);
  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, NULL);
  svc->tot_pri = 1;
  svc->abs_pri = 1;
  /* Register service in the listener */
  SLIST_PUSH (&lst->services, svc, next);

  /* Create backend */
  XZALLOC (be);
  be->be_type = BE_CONTROL;
  be->priority = 1;
  be->alive = 1;
  pthread_mutex_init (&be->mut, NULL);
  /* Register backend in service */
  SLIST_PUSH (&svc->backends, be, next);

  return PARSER_OK;
}

static PARSER_TABLE top_level_parsetab[] = {
  { "Include", parse_include },
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
  { "Alive", assign_timeout, &alive_to },
  { "Client", assign_timeout, NULL, offsetof (POUND_DEFAULTS, clnt_to) },
  { "TimeOut", assign_timeout, NULL, offsetof (POUND_DEFAULTS, be_to) },
  { "WSTimeOut", assign_timeout, NULL, offsetof (POUND_DEFAULTS, ws_to) },
  { "ConnTO", assign_timeout, NULL, offsetof (POUND_DEFAULTS, be_connto) },
  { "IgnoreCase", assign_bool, NULL, offsetof (POUND_DEFAULTS, ignore_case) },
  { "ECDHCurve", parse_ECDHCurve },
  { "SSLEngine", parse_SSLEngine },
  { "Control", parse_control_global },
  { "Anonymise", int_set_one, &anonymise },
  { "Anonymize", int_set_one, &anonymise },
  { "Service", parse_service, &services },
  { "ListenHTTP", parse_listen_http, &listeners },
  { "ListenHTTPS", parse_listen_https, &listeners },
  { "ACL", parse_named_acl, NULL },
  { "PidFile", assign_string, &pid_name },
  { NULL }
};

int
parse_config_file (char const *file)
{
  int res = -1;
  POUND_DEFAULTS pound_defaults = {
    .facility = LOG_DAEMON,
    .log_level = 1,
    .clnt_to = 10,
    .be_to = 15,
    .ws_to = 600,
    .be_connto = 15,
    .ignore_case = 0
  };

  if (push_input (file) == 0)
    {
      res = parser_loop (top_level_parsetab, &pound_defaults, &pound_defaults, NULL);
      if (res == 0)
	{
	  if (cur_input)
	    exit (1);
	  if (worker_min_count > worker_max_count)
	    {
	      logmsg (LOG_ERR, "WorkerMinCount is greater than WorkerMaxCount");
	      exit (1);
	    }
	  log_facility = pound_defaults.facility;
	}
    }
  return res;
}

enum {
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

static struct pound_feature feature[] = {
  [FEATURE_DNS] = {
    .name = "dns",
    .descr = "resolve host names found in configuration file (default)",
    .enabled = F_ON
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

enum string_value_type
  {
    STRING_CONSTANT,
    STRING_INT,
    STRING_VARIABLE,
    STRING_FUNCTION,
    STRING_PRINTER
  };

struct string_value
{
  char const *kw;
  enum string_value_type type;
  union
  {
    char *s_const;
    char **s_var;
    int s_int;
    char const *(*s_func) (void);
    void (*s_print) (FILE *);
  } data;
};

#define VALUE_COLUMN 28

static void
print_string_values (struct string_value *values, FILE *fp)
{
  struct string_value *p;
  char const *val;

  for (p = values; p->kw; p++)
    {
      int n = fprintf (fp, "%s:", p->kw);
      if (n < VALUE_COLUMN)
	fprintf (fp, "%*s", VALUE_COLUMN-n, "");

      switch (p->type)
	{
	case STRING_CONSTANT:
	  val = p->data.s_const;
	  break;

	case STRING_INT:
	  fprintf (fp, "%d\n", p->data.s_int);
	  continue;

	case STRING_VARIABLE:
	  val = *p->data.s_var;
	  break;

	case STRING_FUNCTION:
	  val = p->data.s_func ();
	  break;

	case STRING_PRINTER:
	  p->data.s_print (fp);
	  fputc ('\n', fp);
	  continue;
	}

      fprintf (fp, "%s\n", val);
    }
}

static char const *
supervisor_status (void)
{
#if SUPERVISOR
  return "enabled";
#else
  return "disabled";
#endif
}

struct string_value pound_settings[] = {
  { "Configuration file",  STRING_CONSTANT, { .s_const = POUND_CONF } },
  { "PID file",   STRING_CONSTANT,  { .s_const = POUND_PID } },
  { "Supervisor", STRING_FUNCTION, { .s_func = supervisor_status } },
  { "Buffer size",STRING_INT, { .s_int = MAXBUF } },
#if ! SET_DH_AUTO
  { "DH bits",         STRING_INT, { .s_int = DH_LEN } },
  { "RSA regeneration interval", STRING_INT, { .s_int = T_RSA_KEYS } },
#endif
  { NULL }
};

static int copyright_year = 2022;
void
print_version (void)
{
  printf ("%s (%s) %s\n", progname, PACKAGE_NAME, PACKAGE_VERSION);
  printf ("Copyright (C) 2002-2010 Apsis GmbH\n");
  printf ("Copyright (C) 2018-%d Sergey Poznyakoff\n", copyright_year);
  printf ("\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
");
  printf ("\nBuilt-in defaults:\n\n");
  print_string_values (pound_settings, stdout);
}

void
print_help (void)
{
  int i;

  printf ("usage: %s [-Vchv] [-W [no-]FEATURE] [-f FILE] [-p FILE]\n", progname);
  printf ("HTTP/HTTPS reverse-proxy and load-balancer\n");
  printf ("\nOptions are:\n\n");
  printf ("   -c               check configuration file syntax and exit\n");
  printf ("   -f FILE          read configuration from FILE\n");
  printf ("                    (default: %s)\n", POUND_CONF);
  printf ("   -p FILE          write PID to FILE\n");
  printf ("                    (default: %s)\n", POUND_PID);
  printf ("   -V               print program version, compilation settings, and exit\n");
  printf ("   -v               verbose mode\n");
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

  if ((progname = strrchr (argv[0], '/')) != NULL)
    progname++;
  else
    progname = argv[0];
  while ((c = getopt (argc, argv, "cf:hp:VvW:")) > 0)
    switch (c)
      {
      case 'c':
	check_only = 1;
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
	print_version ();
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

  if (parse_config_file (conf_name))
    exit (1);
  name_list_free ();

  if (check_only)
    {
      logmsg (LOG_INFO, "Config file %s is OK", conf_name);
      exit (0);
    }

  if (SLIST_EMPTY (&listeners))
    {
      logmsg (LOG_ERR, "no listeners defined");
      exit (1);
    }

  if (pid_file_option)
    pid_name = pid_file_option;
}

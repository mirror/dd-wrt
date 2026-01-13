/* This file is part of pound
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
#include <stdarg.h>
#include <stddef.h>

typedef struct
{
  unsigned refcnt;
  char str[1];
} STRING;

static inline STRING *
string_ref (STRING *s)
{
  if (s) s->refcnt++;
  return s;
}

static inline STRING *
string_unref (STRING *sp)
{
  if (sp && --sp->refcnt == 0)
    {
      free (sp);
      return NULL;
    }
  return sp;
}

static inline STRING *
string_alloc (size_t n)
{
  return string_ref (xcalloc (1, sizeof (STRING) + n));
}

static inline STRING *
string_ninit (char const *s, size_t n)
{
  STRING *str = string_alloc (n);
  memcpy (str->str, s, n);
  str->str[n] = 0;
  return str;
}

static inline STRING *
string_init (char const *s)
{
  return s ? string_ninit (s, strlen (s)) : NULL;
}

static inline char const *
string_ptr (STRING *s)
{
  return s ? s->str : NULL;
}

/* Locations in the source file */
struct locus_point
{
  STRING *filename;
  int line;
  int col;
};

#define LOCUS_POINT_INITIALIZER { NULL, 1, 0 }

struct locus_range
{
  struct locus_point beg, end;
};

#define LOCUS_RANGE_INITIALIZER \
  { LOCUS_POINT_INITIALIZER, LOCUS_POINT_INITIALIZER }


static inline void
locus_point_init (struct locus_point *pt, char const *filename,
		  char const *dir)
{
  if (filename)
    {
      size_t dlen = (dir && filename[0] != '/') ? strlen (dir) : 0;
      char *p;
      pt->filename = string_alloc (strlen (filename) +
				   (dlen ? (dlen + 1) : 0));
      p = pt->filename->str;
      if (dlen > 0)
	{
	  memcpy (p, dir, dlen);
	  p += dlen;
	  *p++ = '/';
	}
      strcpy (p, filename);
    }
  else
    pt->filename = NULL;
  pt->line = 1;
  pt->col = 0;
}

static inline void
locus_point_ref (struct locus_point *pt)
{
  string_ref (pt->filename);
}

static inline void
locus_point_unref (struct locus_point *pt)
{
  string_unref (pt->filename);
  pt->filename = NULL;
}

static inline void
locus_point_copy (struct locus_point *dst, struct locus_point const *src)
{
  if (dst->filename != src->filename)
    string_unref (dst->filename);
  *dst = *src;
  string_ref (dst->filename);
}

static inline void
locus_range_init (struct locus_range *rng)
{
  locus_point_init (&rng->beg, NULL, NULL);
  locus_point_init (&rng->end, NULL, NULL);
}

static inline void
locus_range_copy (struct locus_range *dst, struct locus_range const *src)
{
  locus_point_copy (&dst->beg, &src->beg);
  locus_point_copy (&dst->end, &src->end);
}

static inline void
locus_range_ref (struct locus_range *r)
{
  locus_point_ref (&r->beg);
  locus_point_ref (&r->end);
}

static inline void
locus_range_unref (struct locus_range *r)
{
  locus_point_unref (&r->beg);
  locus_point_unref (&r->end);
}

struct locus_range const *last_token_locus_range (void);

typedef struct workdir
{
  DLIST_ENTRY (workdir) link;
  int refcount;
  int fd;
  char name[1];
} WORKDIR;

static inline WORKDIR *
workdir_ref (WORKDIR *wd)
{
  wd->refcount++;
  return wd;
}

static inline void
workdir_unref (WORKDIR *wd)
{
  if (wd)
    {
      wd->refcount--;
    }
}

WORKDIR *workdir_get (char const *name);
WORKDIR *get_include_wd_at_locus_range (struct locus_range const *locus);
static inline WORKDIR *get_include_wd (void)
{
  return get_include_wd_at_locus_range (last_token_locus_range ());
}

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

/*
 * Buffer size for token buffer used as input to token_mask_str.  This takes
 * into account only T_.* types above, as returned by token_type_str.
 *
 * Be sure to update this constant if you change anything above.
 */
#define MAX_TOKEN_BUF_SIZE 45

/* Token structure */
struct token
{
  int type;
  char *str;
  struct locus_range locus;
};


/*
 * Token manipulation functions.
 */
char const *token_type_str (unsigned type);
size_t token_mask_str (TOKENMASK mask, char *buf, size_t size);

/*
 * Keyword table and lookups it in.
 */
struct kwtab
{
  char const *name;
  int tok;
};

int kw_to_tok (struct kwtab *kwt, char const *name, int ci, int *retval);
char const *kw_to_str (struct kwtab *kwt, int t);

/*
 * Locus formatting.
 */
struct stringbuf;
void stringbuf_format_locus_point (struct stringbuf *sb,
				   struct locus_point const *loc);
void stringbuf_format_locus_range (struct stringbuf *sb,
				   struct locus_range const *range);
void vconf_error_at_locus_range (struct locus_range const *loc,
				 char const *fmt, va_list ap);
void conf_error_at_locus_range (struct locus_range const *loc,
				char const *fmt, ...);
void vconf_error_at_locus_point (struct locus_point const *loc,
				 char const *fmt, va_list ap);
void conf_error_at_locus_point (struct locus_point const *loc,
				char const *fmt, ...);

#define conf_error(fmt, ...)						\
  conf_error_at_locus_range (last_token_locus_range (), fmt, __VA_ARGS__)

struct token *gettkn_expect_mask (int expect);
struct token *gettkn_expect (int type);
struct token *gettkn_any (void);
void putback_tkn (struct token *tok);
void putback_synth (int type, char const *str, struct locus_range *loc);

enum
  {
    CFGPARSER_OK,
    CFGPARSER_OK_NONL,
    CFGPARSER_FAIL,
    CFGPARSER_END
  };

typedef int (*CFGPARSER) (void *, void *);

enum keyword_type
  {
    KWT_REG,          /* Regular keyword */
    KWT_ALIAS,        /* Alias to another keyword */
    KWT_TABREF,       /* Reference to another table */
    KWT_SOFTREF,      /* Same as above, but overrides data/off pair of it. */
    KWT_TOPLEVEL,     /* Can appear only in the first entry of the table.
			 Remaining fields are ignored.
			 Indicates that it is a top-level table, i.e. the
			 input ends with an EOF. */
    KWT_WILDCARD      /* Match any keyword.  Used in poundctl to skip
			 statements of no interest. */
  };

typedef struct cfg_parser_table
{
  char *name;        /* The keyword. */
  CFGPARSER parser;  /* Parser function. */
  void *data;        /* Data pointer to pass to parser in its first
			parameter. */
  size_t off;        /* Offset data by this number of bytes before passing. */

  enum keyword_type type;  /* Entry type. */

  /* For KWT_TABREF & KWT_SOFTREF */
  struct cfg_parser_table *ref;

  /* For deprecated statements: */
  int deprecated;    /* Whether the statement is deprecated. */
  char *message;     /* Deprecation message. For KWT_ALIAS it can be NULL,
			in which case a default message will be generated. */
} CFGPARSER_TABLE;

enum deprecation_mode
  {
    DEPREC_OK,
    DEPREC_WARN,
    DEPREC_ERR
  };

int cfgparser (CFGPARSER_TABLE *ptab,
	       void *call_data, void *section_data,
	       int single_statement,
	       enum deprecation_mode handle_deprecated,
	       struct locus_range *retrange);

static inline int
cfgparser_loop (CFGPARSER_TABLE *ptab,
		void *call_data, void *section_data,
		enum deprecation_mode handle_deprecated,
		struct locus_range *retrange)
{
  return cfgparser (ptab, call_data, section_data, 0, handle_deprecated, retrange);
}

int cfg_parse_end (void *call_data, void *section_data);
int cfg_parse_include (void *call_data, void *section_data);
int cfg_parse_includedir (void *call_data, void *section_data);
int cfg_int_set_one (void *call_data, void *section_data);
int cfg_assign_string (void *call_data, void *section_data);
int cfg_assign_string_from_file (void *call_data, void *section_data);
int cfg_assign_bool (void *call_data, void *section_data);
int cfg_assign_unsigned (void *call_data, void *section_data);
int cfg_assign_int (void *call_data, void *section_data);
int cfg_assign_mode (void *call_data, void *section_data);

int cfg_assign_unsigned_min (unsigned *dst, unsigned minval, int quiet);
int cfg_assign_int_range (int *dst, int min, int max);
int cfg_assign_int_enum (int *dst, struct token *tok, struct kwtab *kwtab,
			 char *what);
int cfg_assign_log_facility (void *call_data, void *section_data);

#define cfg_assign_timeout cfg_assign_unsigned

int cfgparser_open (char const *filename, char const *wd);
int cfgparser_finish (int keepwd);
int cfgparser_parse (char const *filename, char const *wd,
		     CFGPARSER_TABLE *tab,
		     void *section_data,
		     enum deprecation_mode handle_deprecated, int keepwd);
struct cfginput;

int cfg_read_to_end (struct cfginput *input, char **ptr);

extern struct cfginput *cur_input;
extern void (*cfg_error_msg) (char const *msg);
extern char const *include_dir;
extern WORKDIR *include_wd;
extern CFGPARSER_TABLE cfg_global_parsetab[];

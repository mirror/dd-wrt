/*
 *	BIRD -- Configuration Lexer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Lexical analyzer
 *
 * The lexical analyzer used for configuration files and CLI commands
 * is generated using the |flex| tool accompanied by a couple of
 * functions maintaining the hash tables containing information about
 * symbols and keywords.
 *
 * Each symbol is represented by a &symbol structure containing name
 * of the symbol, its lexical scope, symbol class (%SYM_PROTO for a
 * name of a protocol, %SYM_CONSTANT for a constant etc.) and class
 * dependent data.  When an unknown symbol is encountered, it's
 * automatically added to the symbol table with class %SYM_VOID.
 *
 * The keyword tables are generated from the grammar templates
 * using the |gen_keywords.m4| script.
 */

%{
#undef REJECT     /* Avoid name clashes */

#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <libgen.h>
#include <glob.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PARSER 1

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "filter/filter.h"
#include "conf/conf.h"
#include "conf/cf-parse.tab.h"
#include "lib/string.h"

struct keyword {
  byte *name;
  int value;
  struct keyword *next;
};

#include "conf/keywords.h"

#define KW_HASH_SIZE 64
static struct keyword *kw_hash[KW_HASH_SIZE];
static int kw_hash_inited;

#define SYM_HASH_SIZE 128

struct sym_scope {
  struct sym_scope *next;		/* Next on scope stack */
  struct symbol *name;			/* Name of this scope */
  int active;				/* Currently entered */
};
static struct sym_scope *conf_this_scope;

static int cf_hash(byte *c);
static inline struct symbol * cf_get_sym(byte *c, uint h0);

linpool *cfg_mem;

int (*cf_read_hook)(byte *buf, unsigned int max, int fd);
struct include_file_stack *ifs;
static struct include_file_stack *ifs_head;

#define MAX_INCLUDE_DEPTH 8

#define YY_INPUT(buf,result,max) result = cf_read_hook(buf, max, ifs->fd);
#define YY_NO_UNPUT
#define YY_FATAL_ERROR(msg) cf_error(msg)

static void cf_include(char *arg, int alen);
static int check_eof(void);

%}

%option noyywrap
%option noinput
%option nounput
%option noreject

%x COMMENT CCOMM CLI

ALPHA [a-zA-Z_]
DIGIT [0-9]
XIGIT [0-9a-fA-F]
ALNUM [a-zA-Z_0-9]
WHITE [ \t]
include   ^{WHITE}*include{WHITE}*\".*\"{WHITE}*;

%%
{include} {
  char *start, *end;

  if (!ifs->depth)
    cf_error("Include not allowed in CLI");

  start = strchr(yytext, '"');
  start++;

  end = strchr(start, '"');
  *end = 0;

  if (start == end)
    cf_error("Include with empty argument");

  cf_include(start, end-start);
}

{DIGIT}+\.{DIGIT}+\.{DIGIT}+\.{DIGIT}+ {
  ip4_addr a;
  if (!ip4_pton(yytext, &a))
    cf_error("Invalid IPv4 address %s", yytext);

#ifdef IPV6
  cf_lval.i32 = ip4_to_u32(a);
  return RTRID;
#else
  cf_lval.a = ipa_from_ip4(a);
  return IPA;
#endif
}

({XIGIT}*::|({XIGIT}*:){3,})({XIGIT}*|{DIGIT}+\.{DIGIT}+\.{DIGIT}+\.{DIGIT}+) {
#ifdef IPV6
  if (ipa_pton(yytext, &cf_lval.a))
    return IPA;
  cf_error("Invalid IPv6 address %s", yytext);
#else
  cf_error("This is an IPv4 router, therefore IPv6 addresses are not supported");
#endif
}

0x{XIGIT}+ {
  char *e;
  unsigned long int l;
  errno = 0;
  l = strtoul(yytext+2, &e, 16);
  if (e && *e || errno == ERANGE || (unsigned long int)(unsigned int) l != l)
    cf_error("Number out of range");
  cf_lval.i = l;
  return NUM;
}

{DIGIT}+ {
  char *e;
  unsigned long int l;
  errno = 0;
  l = strtoul(yytext, &e, 10);
  if (e && *e || errno == ERANGE || (unsigned long int)(unsigned int) l != l)
    cf_error("Number out of range");
  cf_lval.i = l;
  return NUM;
}

else: {
  /* Hack to distinguish if..else from else: in case */
  return ELSECOL;
}

({ALPHA}{ALNUM}*|[']({ALNUM}|[-]|[\.]|[:])*[']) {
  if(*yytext == '\'') {
    yytext[yyleng-1] = 0;
    yytext++;
  }
  unsigned int h = cf_hash(yytext);
  struct keyword *k = kw_hash[h & (KW_HASH_SIZE-1)];
  while (k)
    {
      if (!strcmp(k->name, yytext))
	{
	  if (k->value > 0)
	    return k->value;
	  else
	    {
	      cf_lval.i = -k->value;
	      return ENUM;
	    }
	}
      k=k->next;
    }
  cf_lval.s = cf_get_sym(yytext, h);
  return SYM;
}

<CLI>(.|\n) {
  BEGIN(INITIAL);
  return CLI_MARKER;
}

\.\. {
  return DDOT;
}

[={}:;,.()+*/%<>~\[\]?!\|-] {
  return yytext[0];
}

["][^"\n]*["] {
  yytext[yyleng-1] = 0;
  cf_lval.t = cfg_strdup(yytext+1);
  return TEXT;
}

["][^"\n]*\n	cf_error("Unterminated string");

<INITIAL,COMMENT><<EOF>>	{ if (check_eof()) return END; }

{WHITE}+

\n	ifs->lino++;

#	BEGIN(COMMENT);

\/\*	BEGIN(CCOMM);

.	cf_error("Unknown character");

<COMMENT>\n {
  ifs->lino++;
  BEGIN(INITIAL);
}

<COMMENT>.

<CCOMM>\*\/	BEGIN(INITIAL);
<CCOMM>\n	ifs->lino++;
<CCOMM>\/\*	cf_error("Comment nesting not supported");
<CCOMM><<EOF>>	cf_error("Unterminated comment");
<CCOMM>.

\!\= return NEQ;
\!\~ return NMA;
\<\= return LEQ;
\>\= return GEQ;
\&\& return AND;
\|\| return OR;

\[\= return PO;
\=\] return PC;

%%

static int
cf_hash(byte *c)
{
  unsigned int h = 13;

  while (*c)
    h = (h * 37) + *c++;
  return h;
}


/*
 * IFS stack - it contains structures needed for recursive processing
 * of include in config files. On the top of the stack is a structure
 * for currently processed file. Other structures are either for
 * active files interrupted because of include directive (these have
 * fd and flex buffer) or for inactive files scheduled to be processed
 * later (when parent requested including of several files by wildcard
 * match - these do not have fd and flex buffer yet).
 *
 * FIXME: Most of these ifs and include functions are really sysdep/unix.
 */

static struct include_file_stack *
push_ifs(struct include_file_stack *old)
{
  struct include_file_stack *ret;
  ret = cfg_allocz(sizeof(struct include_file_stack));
  ret->lino = 1;
  ret->prev = old;
  return ret;
}

static struct include_file_stack *
pop_ifs(struct include_file_stack *old)
{
 yy_delete_buffer(old->buffer);
 close(old->fd);
 return old->prev;
}

static void
enter_ifs(struct include_file_stack *new)
{
  if (!new->buffer)
    {
      new->fd = open(new->file_name, O_RDONLY);
      if (new->fd < 0)
        {
          ifs = ifs->up;
	  cf_error("Unable to open included file %s: %m", new->file_name);
        }

      new->buffer = yy_create_buffer(NULL, YY_BUF_SIZE);
    }

  yy_switch_to_buffer(new->buffer);
}

/**
 * cf_lex_unwind - unwind lexer state during error
 *
 * cf_lex_unwind() frees the internal state on IFS stack when the lexical
 * analyzer is terminated by cf_error().
 */
void
cf_lex_unwind(void)
{
  struct include_file_stack *n;

  for (n = ifs; n != ifs_head; n = n->prev)
    {
      /* Memory is freed automatically */
      if (n->buffer)
	yy_delete_buffer(n->buffer);
      if (n->fd)
        close(n->fd);
    }

  ifs = ifs_head;
}

static void
cf_include(char *arg, int alen)
{
  struct include_file_stack *base_ifs = ifs;
  int new_depth, rv, i;
  char *patt;
  glob_t g = {};

  new_depth = ifs->depth + 1;
  if (new_depth > MAX_INCLUDE_DEPTH)
    cf_error("Max include depth reached");

  /* expand arg to properly handle relative filenames */
  if (*arg != '/')
    {
      int dlen = strlen(ifs->file_name);
      char *dir = alloca(dlen + 1);
      patt = alloca(dlen + alen + 2);
      memcpy(dir, ifs->file_name, dlen + 1);
      sprintf(patt, "%s/%s", dirname(dir), arg);
    }
  else
    patt = arg;

  /* Skip globbing if there are no wildcards, mainly to get proper
     response when the included config file is missing */
  if (!strpbrk(arg, "?*["))
    {
      ifs = push_ifs(ifs);
      ifs->file_name = cfg_strdup(patt);
      ifs->depth = new_depth;
      ifs->up = base_ifs;
      enter_ifs(ifs);
      return;
    }

  /* Expand the pattern */
  rv = glob(patt, GLOB_ERR | GLOB_NOESCAPE, NULL, &g);
  if (rv == GLOB_ABORTED)
    cf_error("Unable to match pattern %s: %m", patt);
  if ((rv != 0) || (g.gl_pathc <= 0))
    return;

  /*
   * Now we put all found files to ifs stack in reverse order, they
   * will be activated and processed in order as ifs stack is popped
   * by pop_ifs() and enter_ifs() in check_eof().
   */
  for(i = g.gl_pathc - 1; i >= 0; i--)
    {
      char *fname = g.gl_pathv[i];
      struct stat fs;

      if (stat(fname, &fs) < 0)
	{
	  globfree(&g);
	  cf_error("Unable to stat included file %s: %m", fname);
	}

      if (fs.st_mode & S_IFDIR)
        continue;

      /* Prepare new stack item */
      ifs = push_ifs(ifs);
      ifs->file_name = cfg_strdup(fname);
      ifs->depth = new_depth;
      ifs->up = base_ifs;
    }

  globfree(&g);
  enter_ifs(ifs);
}

static int
check_eof(void)
{
  if (ifs == ifs_head)
    {
      /* EOF in main config file */
      ifs->lino = 1; /* Why this? */
      return 1;
    }

  ifs = pop_ifs(ifs);
  enter_ifs(ifs);
  return 0;
}

static struct symbol *
cf_new_sym(byte *c, uint h0)
{
  uint h = h0 & (SYM_HASH_SIZE-1);
  struct symbol *s, **ht;
  int l;

  if (!new_config->sym_hash)
    new_config->sym_hash = cfg_allocz(SYM_HASH_SIZE * sizeof(struct keyword *));
  ht = new_config->sym_hash;
  l = strlen(c);
  if (l > SYM_MAX_LEN)
    cf_error("Symbol too long");
  s = cfg_alloc(sizeof(struct symbol) + l);
  s->next = ht[h];
  ht[h] = s;
  s->scope = conf_this_scope;
  s->class = SYM_VOID;
  s->def = NULL;
  s->aux = 0;
  strcpy(s->name, c);
  return s;
}

static struct symbol *
cf_find_sym(struct config *cfg, byte *c, uint h0)
{
  uint h = h0 & (SYM_HASH_SIZE-1);
  struct symbol *s, **ht;

  if (ht = cfg->sym_hash)
    {
      for(s = ht[h]; s; s=s->next)
	if (!strcmp(s->name, c) && s->scope->active)
	  return s;
    }
  if (ht = cfg->sym_fallback)
    {
      /* We know only top-level scope is active */
      for(s = ht[h]; s; s=s->next)
	if (!strcmp(s->name, c) && s->scope->active)
	  return s;
    }

  return NULL;
}

static inline struct symbol *
cf_get_sym(byte *c, uint h0)
{
  return cf_find_sym(new_config, c, h0) ?: cf_new_sym(c, h0);
}

/**
 * cf_find_symbol - find a symbol by name
 * @cfg: specificed config
 * @c: symbol name
 *
 * This functions searches the symbol table in the config @cfg for a symbol of
 * given name. First it examines the current scope, then the second recent one
 * and so on until it either finds the symbol and returns a pointer to its
 * &symbol structure or reaches the end of the scope chain and returns %NULL to
 * signify no match.
 */
struct symbol *
cf_find_symbol(struct config *cfg, byte *c)
{
  return cf_find_sym(cfg, c, cf_hash(c));
}

/**
 * cf_get_symbol - get a symbol by name
 * @c: symbol name
 *
 * This functions searches the symbol table of the currently parsed config
 * (@new_config) for a symbol of given name. It returns either the already
 * existing symbol or a newly allocated undefined (%SYM_VOID) symbol if no
 * existing symbol is found.
 */
struct symbol *
cf_get_symbol(byte *c)
{
  return cf_get_sym(c, cf_hash(c));
}

struct symbol *
cf_default_name(char *template, int *counter)
{
  char buf[SYM_MAX_LEN];
  struct symbol *s;
  char *perc = strchr(template, '%');

  for(;;)
    {
      bsprintf(buf, template, ++(*counter));
      s = cf_get_sym(buf, cf_hash(buf));
      if (s->class == SYM_VOID)
	return s;
      if (!perc)
	break;
    }
  cf_error("Unable to generate default name");
}

/**
 * cf_define_symbol - define meaning of a symbol
 * @sym: symbol to be defined
 * @type: symbol class to assign
 * @def: class dependent data
 *
 * Defines new meaning of a symbol. If the symbol is an undefined
 * one (%SYM_VOID), it's just re-defined to the new type. If it's defined
 * in different scope, a new symbol in current scope is created and the
 * meaning is assigned to it. If it's already defined in the current scope,
 * an error is reported via cf_error().
 *
 * Result: Pointer to the newly defined symbol. If we are in the top-level
 * scope, it's the same @sym as passed to the function.
 */
struct symbol *
cf_define_symbol(struct symbol *sym, int type, void *def)
{
  if (sym->class)
    {
      if (sym->scope == conf_this_scope)
	cf_error("Symbol already defined");
      sym = cf_new_sym(sym->name, cf_hash(sym->name));
    }
  sym->class = type;
  sym->def = def;
  return sym;
}

static void
cf_lex_init_kh(void)
{
  struct keyword *k;

  for(k=keyword_list; k->name; k++)
    {
      unsigned h = cf_hash(k->name) & (KW_HASH_SIZE-1);
      k->next = kw_hash[h];
      kw_hash[h] = k;
    }
  kw_hash_inited = 1;
}

/**
 * cf_lex_init - initialize the lexer
 * @is_cli: true if we're going to parse CLI command, false for configuration
 * @c: configuration structure
 *
 * cf_lex_init() initializes the lexical analyzer and prepares it for
 * parsing of a new input.
 */
void
cf_lex_init(int is_cli, struct config *c)
{
  if (!kw_hash_inited)
    cf_lex_init_kh();

  ifs_head = ifs = push_ifs(NULL);
  if (!is_cli)
    {
      ifs->file_name = c->file_name;
      ifs->fd = c->file_fd;
      ifs->depth = 1;
    }

  yyrestart(NULL);
  ifs->buffer = YY_CURRENT_BUFFER;

  if (is_cli)
    BEGIN(CLI);
  else
    BEGIN(INITIAL);

  conf_this_scope = cfg_allocz(sizeof(struct sym_scope));
  conf_this_scope->active = 1;
}

/**
 * cf_push_scope - enter new scope
 * @sym: symbol representing scope name
 *
 * If we want to enter a new scope to process declarations inside
 * a nested block, we can just call cf_push_scope() to push a new
 * scope onto the scope stack which will cause all new symbols to be
 * defined in this scope and all existing symbols to be sought for
 * in all scopes stored on the stack.
 */
void
cf_push_scope(struct symbol *sym)
{
  struct sym_scope *s = cfg_alloc(sizeof(struct sym_scope));

  s->next = conf_this_scope;
  conf_this_scope = s;
  s->active = 1;
  s->name = sym;
}

/**
 * cf_pop_scope - leave a scope
 *
 * cf_pop_scope() pops the topmost scope from the scope stack,
 * leaving all its symbols in the symbol table, but making them
 * invisible to the rest of the config.
 */
void
cf_pop_scope(void)
{
  conf_this_scope->active = 0;
  conf_this_scope = conf_this_scope->next;
  ASSERT(conf_this_scope);
}

struct symbol *
cf_walk_symbols(struct config *cf, struct symbol *sym, int *pos)
{
  for(;;)
    {
      if (!sym)
	{
	  if (*pos >= SYM_HASH_SIZE)
	    return NULL;
	  sym = cf->sym_hash[(*pos)++];
	}
      else
	sym = sym->next;
      if (sym && sym->scope->active)
	return sym;
    }
}

/**
 * cf_symbol_class_name - get name of a symbol class
 * @sym: symbol
 *
 * This function returns a string representing the class
 * of the given symbol.
 */
char *
cf_symbol_class_name(struct symbol *sym)
{
  if (cf_symbol_is_constant(sym))
    return "constant";

  switch (sym->class)
    {
    case SYM_VOID:
      return "undefined";
    case SYM_PROTO:
      return "protocol";
    case SYM_TEMPLATE:
      return "protocol template";
    case SYM_FUNCTION:
      return "function";
    case SYM_FILTER:
      return "filter";
    case SYM_TABLE:
      return "routing table";
    case SYM_ROA:
      return "ROA table";
    default:
      return "unknown type";
    }
}


/**
 * DOC: Parser
 *
 * Both the configuration and CLI commands are analyzed using a syntax
 * driven parser generated by the |bison| tool from a grammar which
 * is constructed from information gathered from grammar snippets by
 * the |gen_parser.m4| script.
 *
 * Grammar snippets are files (usually with extension |.Y|) contributed
 * by various BIRD modules in order to provide information about syntax of their
 * configuration and their CLI commands. Each snipped consists of several
 * sections, each of them starting with a special keyword: |CF_HDR| for
 * a list of |#include| directives needed by the C code, |CF_DEFINES|
 * for a list of C declarations, |CF_DECLS| for |bison| declarations
 * including keyword definitions specified as |CF_KEYWORDS|, |CF_GRAMMAR|
 * for the grammar rules, |CF_CODE| for auxiliary C code and finally
 * |CF_END| at the end of the snippet.
 *
 * To create references between the snippets, it's possible to define
 * multi-part rules by utilizing the |CF_ADDTO| macro which adds a new
 * alternative to a multi-part rule.
 *
 * CLI commands are defined using a |CF_CLI| macro. Its parameters are:
 * the list of keywords determining the command, the list of parameters,
 * help text for the parameters and help text for the command.
 *
 * Values of |enum| filter types can be defined using |CF_ENUM| with
 * the following parameters: name of filter type, prefix common for all
 * literals of this type and names of all the possible values.
 */

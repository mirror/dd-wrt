/*
 *	BIRD -- Configuration Parser Top
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#define PARSER 1

#include "nest/bird.h"
#include "conf/conf.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/timer.h"
#include "lib/string.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "filter/filter.h"

/* FIXME: Turn on YYERROR_VERBOSE and work around lots of bison bugs? */

CF_DEFINES

static void
check_u16(unsigned val)
{
  if (val > 0xFFFF)
    cf_error("Value %d out of range (0-65535)", val);
}

CF_DECLS

%union {
  int i;
  u32 i32;
  ip_addr a;
  struct symbol *s;
  char *t;
  struct rtable_config *r;
  struct f_inst *x;
  struct filter *f;
  struct f_tree *e;
  struct f_trie *trie;
  struct f_val v;
  struct f_path_mask *h;
  struct password_item *p;
  struct rt_show_data *ra;
  struct roa_show_data *ro;
  struct sym_show_data *sd;
  struct lsadb_show_data *ld;
  struct iface *iface;
  struct roa_table *rot;
  void *g;
  bird_clock_t time;
  struct prefix px;
  struct proto_spec ps;
  struct timeformat *tf;
}

%token END CLI_MARKER INVALID_TOKEN ELSECOL DDOT
%token GEQ LEQ NEQ AND OR
%token PO PC
%token <i> NUM ENUM
%token <i32> RTRID
%token <a> IPA
%token <s> SYM
%token <t> TEXT
%type <iface> ipa_scope

%type <i> expr bool pxlen
%type <i32> expr_us
%type <time> datetime
%type <a> ipa
%type <px> prefix prefix_or_ipa
%type <t> text
%type <t> text_or_none

%nonassoc PREFIX_DUMMY
%left AND OR
%nonassoc '=' '<' '>' '~' GEQ LEQ NEQ NMA PO PC
%left '+' '-'
%left '*' '/' '%'
%left '!'
%nonassoc '.'

CF_KEYWORDS(DEFINE, ON, OFF, YES, NO, S, MS, US, PORT)

CF_GRAMMAR

/* Basic config file structure */

config: conf_entries END { return 0; }
 | CLI_MARKER cli_cmd { return 0; }
 ;

conf_entries:
   /* EMPTY */
 | conf_entries conf
 ;

CF_ADDTO(conf, ';')


/* Constant expressions */

CF_ADDTO(conf, definition)
definition:
   DEFINE SYM '=' term ';' {
     struct f_val *val = cfg_alloc(sizeof(struct f_val));
     *val = f_eval($4, cfg_mem);
     if (val->type == T_RETURN) cf_error("Runtime error");
     cf_define_symbol($2, SYM_CONSTANT | val->type, val);
   }
 ;

expr:
   NUM
 | '(' term ')' { $$ = f_eval_int($2); }
 | SYM {
     if ($1->class != (SYM_CONSTANT | T_INT)) cf_error("Number expected");
     $$ = SYM_VAL($1).i; }
 ;


expr_us:
   expr S  { $$ = (u32) $1 * 1000000; }
 | expr MS { $$ = (u32) $1 * 1000; }
 | expr US { $$ = (u32) $1 * 1; }
 ;

/* expr_u16: expr { check_u16($1); $$ = $1; }; */

/* Switches */

bool:
   expr { $$ = !!$1; }
 | ON { $$ = 1; }
 | YES { $$ = 1; }
 | OFF { $$ = 0; }
 | NO { $$ = 0; }
 | /* Silence means agreement */ { $$ = 1; }
 ;

/* Addresses, prefixes and netmasks */

ipa:
   IPA
 | SYM {
     if ($1->class != (SYM_CONSTANT | T_IP)) cf_error("IP address expected");
     $$ = SYM_VAL($1).px.ip;
   }
 ;

ipa_scope:
   /* empty */ { $$ = NULL; }
 | '%' SYM { $$ = if_get_by_name($2->name); }
 ;

prefix:
   ipa pxlen {
     if (!ip_is_prefix($1, $2)) cf_error("Invalid prefix");
     $$.addr = $1; $$.len = $2;
   }
 ;

prefix_or_ipa:
   prefix
 | ipa { $$.addr = $1; $$.len = BITS_PER_IP_ADDRESS; }
 ;

pxlen:
   '/' expr {
     if ($2 < 0 || $2 > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", $2);
     $$ = $2;
   }
 | ':' ipa {
     $$ = ipa_masklen($2);
     if ($$ < 0) cf_error("Invalid netmask %I", $2);
   }
 ;

datetime:
   TEXT {
     $$ = tm_parse_datetime($1);
     if (!$$)
       cf_error("Invalid date and time");
   }
 ;

text:
   TEXT
 | SYM {
     if ($1->class != (SYM_CONSTANT | T_STRING)) cf_error("String expected");
     $$ = SYM_VAL($1).s;
   }
 ;

text_or_none:
   TEXT { $$ = $1; }
 |      { $$ = NULL; }
 ;

CF_CODE

CF_END

/*
 *	BIRD -- Configuration Parser Top
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

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
  void *g;
  bird_clock_t time;
  struct prefix px;
  struct proto_spec ps;
  struct timeformat *tf;
}

%token END CLI_MARKER INVALID_TOKEN
%token GEQ LEQ NEQ AND OR
%token PO PC
%token <i> NUM ENUM
%token <i32> RTRID
%token <a> IPA
%token <s> SYM
%token <t> TEXT

%type <i> expr bool pxlen
%type <time> datetime
%type <a> ipa
%type <px> prefix prefix_or_ipa
%type <t> text_or_none

%nonassoc PREFIX_DUMMY
%left AND OR
%nonassoc '=' '<' '>' '~' '.' GEQ LEQ NEQ PO PC
%left '+' '-'
%left '*' '/' '%'
%left '!'

CF_KEYWORDS(DEFINE, ON, OFF, YES, NO)

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

expr:
   NUM
 | '(' term ')' { $$ = f_eval_int($2); }
 | SYM { if ($1->class != SYM_NUMBER) cf_error("Number expected"); else $$ = $1->aux; }
 ;

CF_ADDTO(conf, definition)
definition:
   DEFINE SYM '=' expr ';' {
     cf_define_symbol($2, SYM_NUMBER, NULL);
     $2->aux = $4;
   }
 | DEFINE SYM '=' IPA ';' {
     cf_define_symbol($2, SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)$2->def = $4;
   }
 ;

/* Switches */

bool:
   expr {$$ = !!$1; }
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
     if ($1->class != SYM_IPA) cf_error("IP address expected");
     $$ = *(ip_addr *)$1->def;
   }
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
     $$ = ipa_mklen($2);
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

text_or_none:
   TEXT { $$ = $1; }
 |      { $$ = NULL; }
 ;

CF_CODE

CF_END

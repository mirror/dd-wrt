/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         cf_parse
#define yylex           cf_lex
#define yyerror         cf_error
#define yylval          cf_lval
#define yychar          cf_char
#define yydebug         cf_debug
#define yynerrs         cf_nerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "cf-parse.y"

/* Headers from ../../conf/confbase.Y */

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

/* Defines from ../../conf/confbase.Y */

static void
check_u16(unsigned val)
{
  if (val > 0xFFFF)
    cf_error("Value %d out of range (0-65535)", val);
}

/* Headers from ../../sysdep/unix/config.Y */

#include "lib/unix.h"
#include <stdio.h>

/* Headers from ../../sysdep/unix/krt.Y */

#include "lib/krt.h"

/* Defines from ../../sysdep/unix/krt.Y */

#define THIS_KRT ((struct krt_config *) this_proto)
#define THIS_KIF ((struct kif_config *) this_proto)

/* Headers from ../../sysdep/linux/netlink.Y */

/* Headers from ../../nest/config.Y */

#include "nest/rt-dev.h"
#include "nest/password.h"
#include "nest/cmds.h"
#include "lib/lists.h"

/* Defines from ../../nest/config.Y */

static struct proto_config *this_proto;
static struct iface_patt *this_ipatt;
static struct iface_patt_node *this_ipn;
static struct roa_table_config *this_roa_table;
static list *this_p_list;
static struct password_item *this_p_item;
static int password_id;

static inline void
reset_passwords(void)
{
  this_p_list = NULL;
}

static inline list *
get_passwords(void)
{
  list *rv = this_p_list;
  this_p_list = NULL;
  return rv;
}

#define DIRECT_CFG ((struct rt_dev_config *) this_proto)

/* Headers from ../../filter/config.Y */

/* Defines from ../../filter/config.Y */

#define P(a,b) ((a << 8) | b)

static inline u32 pair(u32 a, u32 b) { return (a << 16) | b; }
static inline u32 pair_a(u32 p) { return p >> 16; }
static inline u32 pair_b(u32 p) { return p & 0xFFFF; }


/*
 * Sets and their items are during parsing handled as lists, linked
 * through left ptr. The first item in a list also contains a pointer
 * to the last item in a list (right ptr). For convenience, even items
 * are handled as one-item lists. Lists are merged by f_merge_items().
 */

static inline struct f_tree *
f_new_item(struct f_val from, struct f_val to)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from = from;
  t->to = to;
  return t;
}

static inline struct f_tree *
f_merge_items(struct f_tree *a, struct f_tree *b)
{
  if (!a) return b;
  a->right->left = b;
  a->right = b->right;
  b->right = NULL;
  return a;
}

static inline struct f_tree *
f_new_pair_item(int fa, int ta, int fb, int tb)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_PAIR;
  t->from.val.i = pair(fa, fb);
  t->to.val.i = pair(ta, tb);
  return t;
}

static inline struct f_tree *
f_new_pair_set(int fa, int ta, int fb, int tb)
{
  struct f_tree *lst = NULL;
  int i;

  if ((fa == ta) || ((fb == 0) && (tb == 0xFFFF)))
    return f_new_pair_item(fa, ta, fb, tb);
  
  if ((ta < fa) || (tb < fb))
    cf_error( "From value cannot be higher that To value in pair sets");

  for (i = fa; i <= ta; i++)
    lst = f_merge_items(lst, f_new_pair_item(i, i, fb, tb));

  return lst;
}

#define EC_ALL 0xFFFFFFFF

static struct f_tree *
f_new_ec_item(u32 kind, u32 ipv4_used, u32 key, u32 vf, u32 vt)
{
  u64 fm, to;

  if (ipv4_used || (key >= 0x10000)) {
    check_u16(vf);
    if (vt == EC_ALL)
      vt = 0xFFFF;
    else
      check_u16(vt);
  }

  if (kind == EC_GENERIC) {
    fm = ec_generic(key, vf);
    to = ec_generic(key, vt);
  }
  else if (ipv4_used) {
    fm = ec_ip4(kind, key, vf);
    to = ec_ip4(kind, key, vt);
  }
  else if (key < 0x10000) {
    fm = ec_as2(kind, key, vf);
    to = ec_as2(kind, key, vt);
  }
  else {
    fm = ec_as4(kind, key, vf);
    to = ec_as4(kind, key, vt);
  }

  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_EC;
  t->from.val.ec = fm;
  t->to.val.ec = to;
  return t;
}

static inline struct f_inst *
f_generate_empty(struct f_inst *dyn)
{ 
  struct f_inst *e = f_new_inst();
  e->code = 'E';

  switch (dyn->aux & EAF_TYPE_MASK) {
    case EAF_TYPE_AS_PATH:
      e->aux = T_PATH;
      break;
    case EAF_TYPE_INT_SET:
      e->aux = T_CLIST;
      break;
    case EAF_TYPE_EC_SET:
      e->aux = T_ECLIST;
      break;
    default:
      cf_error("Can't empty that attribute");
  }

  dyn->code = P('e','S');
  dyn->a1.p = e;
  return dyn;
}


static inline struct f_inst *
f_generate_dpair(struct f_inst *t1, struct f_inst *t2)
{
  struct f_inst *rv;

  if ((t1->code == 'c') && (t2->code == 'c')) {
    if ((t1->aux != T_INT) || (t2->aux != T_INT))
      cf_error( "Can't operate with value of non-integer type in pair constructor");

    check_u16(t1->a2.i);
    check_u16(t2->a2.i);

    rv = f_new_inst();
    rv->code = 'c';
    rv->aux = T_PAIR;
    rv->a2.i = pair(t1->a2.i, t2->a2.i);
  }
  else {
    rv = f_new_inst();
    rv->code = P('m', 'p');
    rv->a1.p = t1;
    rv->a2.p = t2;
  }

  return rv;
}

static inline struct f_inst *
f_generate_ec(u16 kind, struct f_inst *tk, struct f_inst *tv)
{
  struct f_inst *rv;
  int c1 = 0, c2 = 0, ipv4_used = 0;
  u32 key = 0, val2 = 0;

  if (tk->code == 'c') {
    c1 = 1;

    if (tk->aux == T_INT) {
      ipv4_used = 0; key = tk->a2.i;
    }
    else if (tk->aux == T_QUAD) {
      ipv4_used = 1; key = tk->a2.i;
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }

#ifndef IPV6
  /* IP->Quad implicit conversion */
  else if (tk->code == 'C') {
    c1 = 1;
    struct f_val *val = tk->a1.p;
    if (val->type == T_IP) {
      ipv4_used = 1; key = ipa_to_u32(val->val.px.ip);
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }
#endif

  if (tv->code == 'c') {
    if (tv->aux != T_INT)
      cf_error("Can't operate with value of non-integer type in EC constructor");
    c2 = 1;
    val2 = tv->a2.i;
  }

  if (c1 && c2) {
    u64 ec;
  
    if (kind == EC_GENERIC) {
      ec = ec_generic(key, val2);
    }
    else if (ipv4_used) {
      check_u16(val2);
      ec = ec_ip4(kind, key, val2);
    }
    else if (key < 0x10000) {
      ec = ec_as2(kind, key, val2);
    }
    else {
      check_u16(val2);
      ec = ec_as4(kind, key, val2);
    }

    NEW_F_VAL;
    rv = f_new_inst();
    rv->code = 'C';
    rv->a1.p = val;    
    val->type = T_EC;
    val->val.ec = ec;
  }
  else {
    rv = f_new_inst();
    rv->code = P('m','c');
    rv->aux = kind;
    rv->a1.p = tk;
    rv->a2.p = tv;
  }

  return rv;
}



/* Headers from ../../proto/bgp/config.Y */

#include "proto/bgp/bgp.h"

/* Defines from ../../proto/bgp/config.Y */

#define BGP_CFG ((struct bgp_config *) this_proto)

/* Headers from ../../proto/ospf/config.Y */

#include "proto/ospf/ospf.h"

/* Defines from ../../proto/ospf/config.Y */

#define OSPF_CFG ((struct ospf_config *) this_proto)
#define OSPF_PATT ((struct ospf_iface_patt *) this_ipatt)

static struct ospf_area_config *this_area;
static struct nbma_node *this_nbma;
static list *this_nets;
static struct area_net_config *this_pref;
static struct ospf_stubnet_config *this_stubnet; 

#ifdef OSPFv2
static void
ospf_iface_finish(void)
{
  struct ospf_iface_patt *ip = OSPF_PATT;

  if (ip->deadint == 0)
    ip->deadint = ip->deadc * ip->helloint;

  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");
}
#endif

#ifdef OSPFv3
static void
ospf_iface_finish(void)
{
  struct ospf_iface_patt *ip = OSPF_PATT;

  if (ip->deadint == 0)
    ip->deadint = ip->deadc * ip->helloint;

  if ((ip->autype != OSPF_AUTH_NONE) || (get_passwords() != NULL))
    cf_error("Authentication not supported in OSPFv3");
}
#endif

static void
ospf_area_finish(void)
{
  if ((this_area->areaid == 0) && (this_area->type != OPT_E))
    cf_error("Backbone area cannot be stub/NSSA");

  if (this_area->summary && (this_area->type == OPT_E))
    cf_error("Only Stub/NSSA areas can use summary propagation");

  if (this_area->default_nssa && ((this_area->type != OPT_N) || ! this_area->summary))
    cf_error("Only NSSA areas with summary propagation can use NSSA default route");

  if ((this_area->default_cost & LSA_EXT_EBIT) && ! this_area->default_nssa)
    cf_error("Only NSSA default route can use type 2 metric");
}

static void
ospf_proto_finish(void)
{
  struct ospf_config *cf = OSPF_CFG;

  if (EMPTY_LIST(cf->area_list))
    cf_error( "No configured areas in OSPF");

  int areano = 0;
  int backbone = 0;
  struct ospf_area_config *ac;
  WALK_LIST(ac, cf->area_list)
  {
    areano++;
    if (ac->areaid == 0)
     backbone = 1;
  }
  cf->abr = areano > 1;

  if (cf->abr && !backbone)
  {
    struct ospf_area_config *ac = cfg_allocz(sizeof(struct ospf_area_config));
    add_head(&cf->area_list, NODE ac);
    init_list(&ac->patt_list);
    init_list(&ac->net_list);
    init_list(&ac->enet_list);
    init_list(&ac->stubnet_list);
  }

  if (!cf->abr && !EMPTY_LIST(cf->vlink_list))
    cf_error( "Vlinks cannot be used on single area router");
}

static inline void
check_defcost(int cost)
{
  if ((cost <= 0) || (cost >= LSINFINITY))
   cf_error("Default cost must be in range 1-%d", LSINFINITY-1);
}

static inline void
set_instance_id(unsigned id)
{
#ifdef OSPFv3
  OSPF_PATT->instance_id = id;
#else
  cf_error("Instance ID requires OSPFv3");
#endif
}

/* Headers from ../../proto/pipe/config.Y */

#include "proto/pipe/pipe.h"

/* Defines from ../../proto/pipe/config.Y */

#define PIPE_CFG ((struct pipe_config *) this_proto)

/* Headers from ../../proto/rip/config.Y */

#include "proto/rip/rip.h"
#include "nest/iface.h"

/* Defines from ../../proto/rip/config.Y */

#define RIP_CFG ((struct rip_proto_config *) this_proto)
#define RIP_IPATT ((struct rip_patt *) this_ipatt)

/* Headers from ../../proto/static/config.Y */

#include "proto/static/static.h"

/* Defines from ../../proto/static/config.Y */

#define STATIC_CFG ((struct static_config *) this_proto)
static struct static_route *this_srt, *this_srt_nh, *last_srt_nh;



/* Line 268 of yacc.c  */
#line 546 "cf-parse.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 258,
     CLI_MARKER = 259,
     INVALID_TOKEN = 260,
     ELSECOL = 261,
     DDOT = 262,
     GEQ = 263,
     LEQ = 264,
     NEQ = 265,
     AND = 266,
     OR = 267,
     PO = 268,
     PC = 269,
     NUM = 270,
     ENUM = 271,
     RTRID = 272,
     IPA = 273,
     SYM = 274,
     TEXT = 275,
     PREFIX_DUMMY = 276,
     DEFINE = 277,
     ON = 278,
     OFF = 279,
     YES = 280,
     NO = 281,
     LOG = 282,
     SYSLOG = 283,
     ALL = 284,
     DEBUG = 285,
     TRACE = 286,
     INFO = 287,
     REMOTE = 288,
     WARNING = 289,
     ERROR = 290,
     AUTH = 291,
     FATAL = 292,
     BUG = 293,
     STDERR = 294,
     SOFT = 295,
     TIMEFORMAT = 296,
     ISO = 297,
     SHORT = 298,
     LONG = 299,
     BASE = 300,
     NAME = 301,
     CONFIGURE = 302,
     DOWN = 303,
     KERNEL = 304,
     PERSIST = 305,
     SCAN = 306,
     TIME = 307,
     LEARN = 308,
     DEVICE = 309,
     ROUTES = 310,
     KRT_SOURCE = 311,
     KRT_METRIC = 312,
     ASYNC = 313,
     TABLE = 314,
     KRT_PREFSRC = 315,
     KRT_REALM = 316,
     ROUTER = 317,
     ID = 318,
     PROTOCOL = 319,
     TEMPLATE = 320,
     PREFERENCE = 321,
     DISABLED = 322,
     DIRECT = 323,
     INTERFACE = 324,
     IMPORT = 325,
     EXPORT = 326,
     FILTER = 327,
     NONE = 328,
     STATES = 329,
     FILTERS = 330,
     LIMIT = 331,
     ACTION = 332,
     WARN = 333,
     BLOCK = 334,
     RESTART = 335,
     DISABLE = 336,
     PASSWORD = 337,
     FROM = 338,
     PASSIVE = 339,
     TO = 340,
     EVENTS = 341,
     PACKETS = 342,
     PROTOCOLS = 343,
     INTERFACES = 344,
     PRIMARY = 345,
     STATS = 346,
     COUNT = 347,
     FOR = 348,
     COMMANDS = 349,
     PREEXPORT = 350,
     GENERATE = 351,
     ROA = 352,
     MAX = 353,
     FLUSH = 354,
     LISTEN = 355,
     BGP = 356,
     V6ONLY = 357,
     DUAL = 358,
     ADDRESS = 359,
     PORT = 360,
     PASSWORDS = 361,
     DESCRIPTION = 362,
     SORTED = 363,
     RELOAD = 364,
     IN = 365,
     OUT = 366,
     MRTDUMP = 367,
     MESSAGES = 368,
     RESTRICT = 369,
     MEMORY = 370,
     IGP_METRIC = 371,
     SHOW = 372,
     STATUS = 373,
     SUMMARY = 374,
     ROUTE = 375,
     SYMBOLS = 376,
     ADD = 377,
     DELETE = 378,
     DUMP = 379,
     RESOURCES = 380,
     SOCKETS = 381,
     NEIGHBORS = 382,
     ATTRIBUTES = 383,
     ECHO = 384,
     ENABLE = 385,
     FUNCTION = 386,
     PRINT = 387,
     PRINTN = 388,
     UNSET = 389,
     RETURN = 390,
     ACCEPT = 391,
     REJECT = 392,
     QUITBIRD = 393,
     INT = 394,
     BOOL = 395,
     IP = 396,
     PREFIX = 397,
     PAIR = 398,
     QUAD = 399,
     EC = 400,
     SET = 401,
     STRING = 402,
     BGPMASK = 403,
     BGPPATH = 404,
     CLIST = 405,
     ECLIST = 406,
     IF = 407,
     THEN = 408,
     ELSE = 409,
     CASE = 410,
     TRUE = 411,
     FALSE = 412,
     RT = 413,
     RO = 414,
     UNKNOWN = 415,
     GENERIC = 416,
     GW = 417,
     NET = 418,
     MASK = 419,
     PROTO = 420,
     SOURCE = 421,
     SCOPE = 422,
     CAST = 423,
     DEST = 424,
     LEN = 425,
     DEFINED = 426,
     CONTAINS = 427,
     RESET = 428,
     PREPEND = 429,
     FIRST = 430,
     LAST = 431,
     MATCH = 432,
     ROA_CHECK = 433,
     EMPTY = 434,
     WHERE = 435,
     EVAL = 436,
     LOCAL = 437,
     NEIGHBOR = 438,
     AS = 439,
     HOLD = 440,
     CONNECT = 441,
     RETRY = 442,
     KEEPALIVE = 443,
     MULTIHOP = 444,
     STARTUP = 445,
     VIA = 446,
     NEXT = 447,
     HOP = 448,
     SELF = 449,
     DEFAULT = 450,
     PATH = 451,
     METRIC = 452,
     START = 453,
     DELAY = 454,
     FORGET = 455,
     WAIT = 456,
     AFTER = 457,
     BGP_PATH = 458,
     BGP_LOCAL_PREF = 459,
     BGP_MED = 460,
     BGP_ORIGIN = 461,
     BGP_NEXT_HOP = 462,
     BGP_ATOMIC_AGGR = 463,
     BGP_AGGREGATOR = 464,
     BGP_COMMUNITY = 465,
     BGP_EXT_COMMUNITY = 466,
     RR = 467,
     RS = 468,
     CLIENT = 469,
     CLUSTER = 470,
     AS4 = 471,
     ADVERTISE = 472,
     IPV4 = 473,
     CAPABILITIES = 474,
     PREFER = 475,
     OLDER = 476,
     MISSING = 477,
     LLADDR = 478,
     DROP = 479,
     IGNORE = 480,
     REFRESH = 481,
     INTERPRET = 482,
     COMMUNITIES = 483,
     BGP_ORIGINATOR_ID = 484,
     BGP_CLUSTER_LIST = 485,
     IGP = 486,
     GATEWAY = 487,
     RECURSIVE = 488,
     MED = 489,
     TTL = 490,
     SECURITY = 491,
     DETERMINISTIC = 492,
     SECONDARY = 493,
     OSPF = 494,
     AREA = 495,
     OSPF_METRIC1 = 496,
     OSPF_METRIC2 = 497,
     OSPF_TAG = 498,
     OSPF_ROUTER_ID = 499,
     RFC1583COMPAT = 500,
     STUB = 501,
     TICK = 502,
     COST = 503,
     COST2 = 504,
     RETRANSMIT = 505,
     HELLO = 506,
     TRANSMIT = 507,
     PRIORITY = 508,
     DEAD = 509,
     TYPE = 510,
     BROADCAST = 511,
     BCAST = 512,
     NONBROADCAST = 513,
     NBMA = 514,
     POINTOPOINT = 515,
     PTP = 516,
     POINTOMULTIPOINT = 517,
     PTMP = 518,
     SIMPLE = 519,
     AUTHENTICATION = 520,
     STRICT = 521,
     CRYPTOGRAPHIC = 522,
     ELIGIBLE = 523,
     POLL = 524,
     NETWORKS = 525,
     HIDDEN = 526,
     VIRTUAL = 527,
     CHECK = 528,
     LINK = 529,
     RX = 530,
     BUFFER = 531,
     LARGE = 532,
     NORMAL = 533,
     STUBNET = 534,
     TAG = 535,
     EXTERNAL = 536,
     LSADB = 537,
     ECMP = 538,
     WEIGHT = 539,
     NSSA = 540,
     TRANSLATOR = 541,
     STABILITY = 542,
     GLOBAL = 543,
     LSID = 544,
     INSTANCE = 545,
     REAL = 546,
     TOPOLOGY = 547,
     STATE = 548,
     PIPE = 549,
     PEER = 550,
     MODE = 551,
     OPAQUE = 552,
     TRANSPARENT = 553,
     RIP = 554,
     INFINITY = 555,
     PERIOD = 556,
     GARBAGE = 557,
     TIMEOUT = 558,
     MULTICAST = 559,
     QUIET = 560,
     NOLISTEN = 561,
     VERSION1 = 562,
     PLAINTEXT = 563,
     MD5 = 564,
     HONOR = 565,
     NEVER = 566,
     ALWAYS = 567,
     RIP_METRIC = 568,
     RIP_TAG = 569,
     STATIC = 570,
     PROHIBIT = 571,
     MULTIPATH = 572
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 469 "cf-parse.y"

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



/* Line 293 of yacc.c  */
#line 928 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 940 "cf-parse.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  61
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2570

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  339
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  228
/* YYNRULES -- Number of rules.  */
#define YYNRULES  741
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1393

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   572

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    30,     2,     2,
     329,   330,    28,    26,   335,    27,    32,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   332,   331,
      23,    22,    24,   336,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   337,     2,   338,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   333,     2,   334,    25,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    13,    15,    19,    21,
      27,    33,    35,    37,    39,    41,    43,    44,    46,    48,
      49,    52,    55,    57,    59,    62,    65,    67,    69,    70,
      75,    78,    79,    81,    84,    86,    88,    92,    94,    98,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   121,
     125,   127,   129,   131,   133,   136,   141,   145,   149,   153,
     157,   162,   165,   166,   168,   171,   174,   178,   181,   185,
     188,   192,   196,   200,   205,   207,   209,   211,   216,   217,
     220,   223,   226,   228,   230,   231,   233,   237,   241,   242,
     251,   253,   258,   260,   262,   263,   265,   269,   270,   273,
     276,   279,   282,   285,   288,   292,   296,   299,   303,   306,
     309,   311,   313,   315,   316,   319,   322,   325,   328,   331,
     333,   337,   341,   342,   344,   346,   349,   350,   352,   356,
     358,   362,   365,   369,   373,   377,   378,   382,   384,   386,
     390,   392,   396,   398,   400,   402,   404,   406,   408,   410,
     412,   416,   418,   422,   424,   426,   431,   433,   434,   438,
     443,   445,   448,   449,   455,   461,   467,   473,   478,   482,
     486,   491,   497,   499,   500,   504,   509,   514,   515,   518,
     522,   526,   530,   533,   536,   539,   543,   547,   550,   553,
     555,   557,   562,   563,   567,   571,   575,   576,   578,   580,
     585,   586,   589,   592,   595,   598,   601,   604,   607,   608,
     611,   621,   631,   636,   640,   644,   648,   652,   656,   660,
     664,   669,   671,   673,   675,   676,   678,   682,   686,   690,
     694,   699,   704,   709,   714,   717,   719,   721,   723,   725,
     726,   728,   729,   734,   737,   739,   741,   743,   745,   747,
     749,   751,   753,   755,   757,   759,   761,   764,   767,   768,
     772,   774,   778,   780,   782,   784,   787,   791,   794,   799,
     800,   806,   807,   809,   811,   814,   816,   820,   822,   824,
     826,   828,   830,   832,   836,   838,   840,   842,   844,   846,
     850,   852,   858,   870,   872,   874,   876,   879,   881,   889,
     899,   907,   909,   911,   913,   917,   919,   921,   923,   927,
     929,   933,   935,   939,   943,   945,   948,   951,   958,   960,
     964,   965,   970,   974,   976,   980,   984,   988,   991,   994,
     997,  1000,  1001,  1004,  1007,  1008,  1010,  1012,  1014,  1016,
    1018,  1020,  1022,  1026,  1030,  1032,  1034,  1040,  1048,  1049,
    1054,  1056,  1058,  1060,  1062,  1064,  1066,  1068,  1070,  1072,
    1076,  1080,  1084,  1088,  1092,  1096,  1100,  1104,  1108,  1112,
    1116,  1120,  1124,  1128,  1131,  1136,  1138,  1140,  1142,  1144,
    1147,  1150,  1154,  1158,  1165,  1169,  1173,  1177,  1181,  1187,
    1194,  1201,  1208,  1215,  1220,  1229,  1234,  1236,  1238,  1240,
    1242,  1244,  1246,  1248,  1249,  1251,  1255,  1257,  1261,  1262,
    1264,  1269,  1276,  1281,  1285,  1291,  1297,  1302,  1309,  1313,
    1316,  1322,  1328,  1337,  1346,  1355,  1364,  1367,  1371,  1375,
    1381,  1388,  1396,  1403,  1408,  1413,  1419,  1426,  1433,  1439,
    1443,  1448,  1454,  1460,  1466,  1472,  1477,  1482,  1488,  1494,
    1500,  1506,  1512,  1518,  1524,  1530,  1537,  1544,  1553,  1560,
    1567,  1573,  1578,  1584,  1589,  1595,  1600,  1606,  1611,  1617,
    1623,  1626,  1630,  1634,  1636,  1639,  1642,  1647,  1650,  1652,
    1655,  1660,  1661,  1665,  1668,  1670,  1673,  1677,  1681,  1685,
    1689,  1692,  1696,  1697,  1703,  1704,  1710,  1713,  1716,  1718,
    1723,  1725,  1727,  1728,  1732,  1735,  1738,  1741,  1747,  1750,
    1751,  1755,  1756,  1759,  1762,  1766,  1769,  1772,  1776,  1779,
    1782,  1785,  1787,  1791,  1794,  1797,  1800,  1803,  1806,  1809,
    1813,  1816,  1819,  1822,  1825,  1828,  1831,  1834,  1837,  1841,
    1845,  1848,  1852,  1855,  1859,  1863,  1868,  1871,  1874,  1877,
    1881,  1885,  1889,  1891,  1892,  1895,  1899,  1901,  1902,  1904,
    1907,  1908,  1911,  1913,  1915,  1918,  1922,  1923,  1924,  1927,
    1928,  1932,  1933,  1937,  1942,  1944,  1945,  1950,  1957,  1964,
    1971,  1979,  1986,  1994,  2000,  2001,  2004,  2008,  2011,  2015,
    2019,  2022,  2026,  2029,  2032,  2036,  2040,  2046,  2051,  2056,
    2059,  2063,  2067,  2072,  2077,  2082,  2088,  2094,  2099,  2103,
    2108,  2113,  2118,  2123,  2125,  2127,  2129,  2131,  2133,  2135,
    2137,  2139,  2140,  2143,  2146,  2147,  2151,  2152,  2156,  2157,
    2161,  2164,  2168,  2172,  2178,  2184,  2188,  2191,  2195,  2199,
    2201,  2204,  2209,  2213,  2217,  2221,  2224,  2227,  2230,  2235,
    2237,  2239,  2241,  2243,  2245,  2247,  2249,  2251,  2253,  2255,
    2257,  2259,  2261,  2263,  2265,  2267,  2269,  2271,  2273,  2275,
    2277,  2279,  2281,  2283,  2285,  2287,  2289,  2291,  2293,  2295,
    2297,  2299,  2301,  2303,  2305,  2307,  2309,  2311,  2313,  2315,
    2317,  2319,  2321,  2323,  2325,  2327,  2329,  2331,  2333,  2335,
    2337,  2339,  2341,  2343,  2345,  2348,  2351,  2354,  2357,  2360,
    2363,  2366,  2369,  2373,  2377,  2381,  2385,  2389,  2393,  2397,
    2399,  2401,  2403,  2405,  2407,  2409,  2411,  2413,  2415,  2417,
    2419,  2421,  2423,  2425,  2427,  2429,  2431,  2433,  2435,  2437,
    2439,  2441
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     340,     0,    -1,   341,     3,    -1,     4,   562,    -1,    -1,
     341,   561,    -1,    15,    -1,   329,   493,   330,    -1,    19,
      -1,    33,    19,    22,   342,   331,    -1,    33,    19,    22,
      18,   331,    -1,   342,    -1,    34,    -1,    36,    -1,    35,
      -1,    37,    -1,    -1,    18,    -1,    19,    -1,    -1,    30,
      19,    -1,   345,   349,    -1,   347,    -1,   345,    -1,    29,
     342,    -1,   332,   345,    -1,    20,    -1,    20,    -1,    -1,
      38,   354,   355,   331,    -1,    57,    20,    -1,    -1,    20,
      -1,    39,   353,    -1,    50,    -1,    40,    -1,   333,   356,
     334,    -1,   357,    -1,   356,   335,   357,    -1,    41,    -1,
      42,    -1,    43,    -1,    44,    -1,    45,    -1,    46,    -1,
      47,    -1,    48,    -1,    49,    -1,   123,    99,   401,   331,
      -1,   123,    20,   331,    -1,   131,    -1,    75,    -1,    56,
      -1,    38,    -1,   359,    20,    -1,   359,    20,   342,    20,
      -1,   359,    53,    54,    -1,   359,    53,    55,    -1,    52,
     360,   331,    -1,    58,   365,     3,    -1,    58,    51,   365,
       3,    -1,    59,     3,    -1,    -1,    20,    -1,   381,    60,
      -1,    61,   344,    -1,    62,    63,   342,    -1,    64,   344,
      -1,    65,    66,   344,    -1,   381,    65,    -1,    62,    63,
     342,    -1,   101,   351,   348,    -1,    60,    70,   342,    -1,
      73,    74,   372,   331,    -1,    15,    -1,    17,    -1,    18,
      -1,   111,   112,   374,   331,    -1,    -1,   374,   375,    -1,
     115,   345,    -1,   116,   342,    -1,   113,    -1,   114,    -1,
      -1,   119,    -1,    70,    19,   376,    -1,   108,    70,    19,
      -1,    -1,   379,   108,   347,   109,    15,   195,    15,   331,
      -1,   378,    -1,   378,   333,   379,   334,    -1,    75,    -1,
      76,    -1,    -1,    19,    -1,    19,    94,    19,    -1,    -1,
      77,   342,    -1,    78,   344,    -1,    41,   398,    -1,   123,
     401,    -1,    81,   384,    -1,    82,   384,    -1,    81,    87,
     386,    -1,    82,    87,   386,    -1,    70,   387,    -1,    73,
      74,   372,    -1,   118,    20,    -1,    83,   457,    -1,   458,
      -1,    40,    -1,    84,    -1,    -1,    88,    89,    -1,    88,
      90,    -1,    88,    91,    -1,    88,    92,    -1,   342,   385,
      -1,    19,    -1,    41,    99,   398,    -1,    41,   105,   342,
      -1,    -1,    20,    -1,   348,    -1,    20,   348,    -1,    -1,
      27,    -1,   389,   391,   390,    -1,   392,    -1,   393,   335,
     392,    -1,   381,    79,    -1,   394,   382,   333,    -1,   395,
     383,   331,    -1,   395,   397,   331,    -1,    -1,    80,   396,
     393,    -1,    40,    -1,    35,    -1,   333,   399,   334,    -1,
     400,    -1,   399,   335,   400,    -1,    85,    -1,    66,    -1,
      86,    -1,   100,    -1,    97,    -1,    98,    -1,    40,    -1,
      35,    -1,   333,   402,   334,    -1,   403,    -1,   402,   335,
     403,    -1,    85,    -1,   124,    -1,   117,   333,   405,   334,
      -1,   406,    -1,    -1,   406,   331,   405,    -1,   407,   333,
     408,   334,    -1,   407,    -1,    93,    20,    -1,    -1,   107,
      94,   350,   331,   408,    -1,   107,    96,   350,   331,   408,
      -1,   147,    94,   350,   331,   408,    -1,   147,    96,   350,
     331,   408,    -1,    74,   342,   331,   408,    -1,   128,   129,
       3,    -1,   128,   126,     3,    -1,   128,    99,   448,     3,
      -1,   128,    99,    40,   448,     3,    -1,    19,    -1,    -1,
     128,   100,     3,    -1,   128,   100,   130,     3,    -1,   128,
     131,   417,     3,    -1,    -1,   417,   347,    -1,   417,   104,
     348,    -1,   417,    70,    19,    -1,   417,    83,   457,    -1,
     417,   458,    -1,   417,    40,    -1,   417,   101,    -1,   417,
     418,    19,    -1,   417,    75,    19,    -1,   417,   102,    -1,
     417,   103,    -1,   106,    -1,    82,    -1,   128,   108,   420,
       3,    -1,    -1,   420,   421,   347,    -1,   420,   195,    15,
      -1,   420,    70,    19,    -1,    -1,   121,    -1,   104,    -1,
     128,   132,   423,     3,    -1,    -1,   423,    70,    -1,   423,
     142,    -1,   423,    83,    -1,   423,    75,    -1,   423,    76,
      -1,   423,   108,    -1,   423,    19,    -1,    -1,    70,    19,
      -1,   133,   108,   347,   109,    15,   195,    15,   424,     3,
      -1,   134,   108,   347,   109,    15,   195,    15,   424,     3,
      -1,   110,   108,   424,     3,    -1,   135,   136,     3,    -1,
     135,   137,     3,    -1,   135,   100,     3,    -1,   135,   138,
       3,    -1,   135,   139,     3,    -1,   135,    66,     3,    -1,
     135,    99,     3,    -1,   140,   436,   437,     3,    -1,    40,
      -1,    35,    -1,    15,    -1,    -1,    15,    -1,    92,   447,
       3,    -1,   141,   447,     3,    -1,    91,   447,     3,    -1,
     120,   447,     3,    -1,   120,   121,   447,     3,    -1,   120,
     122,   447,     3,    -1,    41,   447,   398,     3,    -1,   123,
     447,   401,     3,    -1,   125,     3,    -1,    19,    -1,    40,
      -1,    20,    -1,    19,    -1,    -1,    20,    -1,    -1,    83,
      19,   450,   456,    -1,   192,   493,    -1,   150,    -1,   151,
      -1,   152,    -1,   153,    -1,   154,    -1,   155,    -1,   156,
      -1,   158,    -1,   159,    -1,   160,    -1,   161,    -1,   162,
      -1,   452,   157,    -1,   452,    19,    -1,    -1,   453,   331,
     454,    -1,   453,    -1,   453,   331,   455,    -1,   460,    -1,
      19,    -1,   456,    -1,   191,   493,    -1,   329,   455,   330,
      -1,   329,   330,    -1,   454,   333,   463,   334,    -1,    -1,
     142,    19,   462,   459,   460,    -1,    -1,   464,    -1,   499,
      -1,   464,   499,    -1,   499,    -1,   333,   463,   334,    -1,
      18,    -1,   342,    -1,    17,    -1,   466,    -1,    16,    -1,
      15,    -1,   329,   493,   330,    -1,    17,    -1,   466,    -1,
      16,    -1,   493,    -1,   469,    -1,   469,     7,   469,    -1,
      28,    -1,   329,   470,   335,   470,   330,    -1,   329,   470,
     335,   470,   330,     7,   329,   469,   335,   469,   330,    -1,
     493,    -1,   169,    -1,   170,    -1,   171,    15,    -1,   172,
      -1,   329,   473,   335,   472,   335,   472,   330,    -1,   329,
     473,   335,   472,   335,   472,     7,   472,   330,    -1,   329,
     473,   335,   472,   335,    28,   330,    -1,   471,    -1,   474,
      -1,   467,    -1,   467,     7,   467,    -1,   471,    -1,   474,
      -1,   468,    -1,   468,     7,   468,    -1,   475,    -1,   477,
     335,   475,    -1,   476,    -1,   478,   335,   476,    -1,    18,
      29,    15,    -1,   479,    -1,   479,    26,    -1,   479,    27,
      -1,   479,   333,    15,   335,    15,   334,    -1,   480,    -1,
     481,   335,   480,    -1,    -1,   482,   478,   332,   463,    -1,
     482,     6,   463,    -1,   491,    -1,   329,   493,   330,    -1,
      13,   485,    14,    -1,    29,   486,    29,    -1,    15,   485,
      -1,    28,   485,    -1,   336,   485,    -1,   483,   485,    -1,
      -1,    15,   486,    -1,   336,   486,    -1,    -1,    15,    -1,
     167,    -1,   168,    -1,    20,    -1,   466,    -1,   479,    -1,
      17,    -1,   337,   477,   338,    -1,   337,   481,   338,    -1,
      16,    -1,   484,    -1,   329,   493,   335,   493,   330,    -1,
     329,   473,   335,   493,   335,   493,   330,    -1,    -1,    19,
     329,   498,   330,    -1,    19,    -1,    94,    -1,   173,    -1,
     174,    -1,   176,    -1,   177,    -1,   178,    -1,   179,    -1,
     180,    -1,   329,   493,   330,    -1,   493,    26,   493,    -1,
     493,    27,   493,    -1,   493,    28,   493,    -1,   493,    29,
     493,    -1,   493,    11,   493,    -1,   493,    12,   493,    -1,
     493,    22,   493,    -1,   493,    10,   493,    -1,   493,    23,
     493,    -1,   493,     9,   493,    -1,   493,    24,   493,    -1,
     493,     8,   493,    -1,   493,    25,   493,    -1,    31,   493,
      -1,   182,   329,   493,   330,    -1,   491,    -1,   487,    -1,
     488,    -1,    77,    -1,   489,   492,    -1,   489,   566,    -1,
     493,    32,   152,    -1,   493,    32,   181,    -1,   493,    32,
     175,   329,   493,   330,    -1,   493,    32,   186,    -1,   493,
      32,   187,    -1,    26,   190,    26,    -1,    27,   190,    27,
      -1,    27,    27,   190,    27,    27,    -1,   185,   329,   493,
     335,   493,   330,    -1,   133,   329,   493,   335,   493,   330,
      -1,   134,   329,   493,   335,   493,   330,    -1,    83,   329,
     493,   335,   493,   330,    -1,   189,   329,    19,   330,    -1,
     189,   329,    19,   335,   493,   335,   493,   330,    -1,    19,
     329,   498,   330,    -1,   149,    -1,   147,    -1,   148,    -1,
      46,    -1,   143,    -1,   144,    -1,   493,    -1,    -1,   495,
      -1,   495,   335,   496,    -1,   493,    -1,   493,   335,   497,
      -1,    -1,   497,    -1,   163,   493,   164,   465,    -1,   163,
     493,   164,   465,   165,   465,    -1,    19,    22,   493,   331,
      -1,   146,   493,   331,    -1,   489,   566,    22,   493,   331,
      -1,   489,   492,    22,   493,   331,    -1,    77,    22,   493,
     331,    -1,   145,   329,   489,   566,   330,   331,    -1,   494,
     496,   331,    -1,   490,   331,    -1,   166,   493,   333,   482,
     334,    -1,   489,   566,    32,   190,   331,    -1,   489,   566,
      32,   185,   329,   493,   330,   331,    -1,   489,   566,    32,
     133,   329,   493,   330,   331,    -1,   489,   566,    32,   134,
     329,   493,   330,   331,    -1,   489,   566,    32,    83,   329,
     493,   330,   331,    -1,   381,   112,    -1,   500,   382,   333,
      -1,   501,   383,   331,    -1,   501,   193,   195,   342,   331,
      -1,   501,   193,   345,   195,   342,   331,    -1,   501,   194,
     345,   346,   195,   342,   331,    -1,   501,   223,   226,    74,
     372,   331,    -1,   501,   223,   225,   331,    -1,   501,   224,
     225,   331,    -1,   501,   196,    63,   342,   331,    -1,   501,
     201,   196,    63,   342,   331,    -1,   501,   197,   198,    63,
     342,   331,    -1,   501,   199,    63,   342,   331,    -1,   501,
     200,   331,    -1,   501,   200,   342,   331,    -1,   501,   203,
     204,   205,   331,    -1,   501,   233,   234,   205,   331,    -1,
     501,   233,   234,   235,   331,    -1,   501,   233,   234,   236,
     331,    -1,   501,   243,    79,   331,    -1,   501,   243,   244,
     331,    -1,   501,   207,   208,   344,   331,    -1,   501,   245,
     208,   344,   331,    -1,   501,   242,   208,   344,   331,    -1,
     501,   231,   232,   344,   331,    -1,   501,   248,   245,   344,
     331,    -1,   501,   206,   216,   342,   331,    -1,   501,   206,
     215,   342,   331,    -1,   501,   177,   115,   345,   331,    -1,
     501,   209,   210,    63,   342,   331,    -1,   501,    46,   211,
      63,   342,   331,    -1,   501,    46,   212,    63,   342,   335,
     342,   331,    -1,   501,    92,   213,    46,   344,   331,    -1,
     501,   141,   131,   237,   344,   331,    -1,   501,   141,   227,
     344,   331,    -1,   501,   230,   344,   331,    -1,   501,   228,
     229,   344,   331,    -1,   501,    93,    20,   331,    -1,   501,
     131,    87,   342,   331,    -1,   501,    95,   344,   331,    -1,
     501,   238,   239,   344,   331,    -1,   501,   249,   344,   331,
      -1,   501,   242,    70,   387,   331,    -1,   501,   246,   247,
     344,   331,    -1,   381,   250,    -1,   502,   382,   333,    -1,
     503,   504,   331,    -1,   383,    -1,   256,   344,    -1,   294,
     344,    -1,   294,   344,    87,   342,    -1,   258,   342,    -1,
     506,    -1,   251,   372,    -1,   505,   333,   507,   334,    -1,
      -1,   507,   508,   331,    -1,   257,   344,    -1,   296,    -1,
     130,   344,    -1,   206,   296,   344,    -1,   206,   259,   342,
      -1,   206,   260,   342,    -1,   257,   259,   342,    -1,   297,
     344,    -1,   297,   298,   342,    -1,    -1,   281,   509,   333,
     520,   334,    -1,    -1,   292,   510,   333,   520,   334,    -1,
     290,   511,    -1,    80,   532,    -1,   515,    -1,   512,   333,
     513,   334,    -1,   512,    -1,   347,    -1,    -1,   513,   514,
     331,    -1,   282,   344,    -1,   130,   344,    -1,   259,   342,
      -1,   518,   529,   333,   516,   334,    -1,   518,   529,    -1,
      -1,   516,   517,   331,    -1,    -1,   262,   342,    -1,   261,
     342,    -1,   263,   210,   342,    -1,   212,   342,    -1,   265,
     342,    -1,   265,   103,   342,    -1,   276,    84,    -1,   276,
     275,    -1,   276,   278,    -1,   404,    -1,   283,   285,   372,
      -1,   259,   342,    -1,   262,   342,    -1,   280,   342,    -1,
     261,   342,    -1,   212,   342,    -1,   265,   342,    -1,   265,
     103,   342,    -1,   266,   267,    -1,   266,   268,    -1,   266,
     269,    -1,   266,   270,    -1,   266,   271,    -1,   266,   272,
      -1,   266,   273,    -1,   266,   274,    -1,   302,   267,   344,
      -1,   263,   210,   342,    -1,   264,   342,    -1,   277,   269,
     344,    -1,   257,   344,    -1,   284,   285,   344,    -1,   294,
     295,   342,    -1,   138,   333,   524,   334,    -1,   276,    84,
      -1,   276,   275,    -1,   276,   278,    -1,   286,   287,   288,
      -1,   286,   287,   289,    -1,   286,   287,   342,    -1,   404,
      -1,    -1,   520,   521,    -1,   522,   523,   331,    -1,   347,
      -1,    -1,   282,    -1,   291,   342,    -1,    -1,   524,   525,
      -1,   526,    -1,   527,    -1,    18,   331,    -1,    18,   279,
     331,    -1,    -1,    -1,   301,   342,    -1,    -1,   530,   519,
     331,    -1,    -1,   333,   530,   334,    -1,   528,   393,   529,
     531,    -1,    20,    -1,    -1,   128,   250,   413,     3,    -1,
     128,   250,   138,   413,   533,     3,    -1,   128,   250,    80,
     413,   533,     3,    -1,   128,   250,   303,   413,   533,     3,
      -1,   128,   250,   303,    40,   413,   533,     3,    -1,   128,
     250,   304,   413,   533,     3,    -1,   128,   250,   304,    40,
     413,   533,     3,    -1,   128,   250,   293,   542,     3,    -1,
      -1,   542,   299,    -1,   542,   251,   372,    -1,   542,   285,
      -1,   542,   266,    15,    -1,   542,   300,   372,    -1,   542,
     205,    -1,   542,    73,   372,    -1,   542,    19,    -1,   381,
     305,    -1,   543,   382,   333,    -1,   544,   383,   331,    -1,
     544,   306,    70,    19,   331,    -1,   544,   307,   308,   331,
      -1,   544,   307,   309,   331,    -1,   381,   310,    -1,   545,
     382,   333,    -1,   546,   383,   331,    -1,   546,   311,   342,
     331,    -1,   546,   116,   342,   331,    -1,   546,   312,   342,
     331,    -1,   546,   313,    63,   342,   331,    -1,   546,   314,
      63,   342,   331,    -1,   546,   276,   547,   331,    -1,   546,
     404,   331,    -1,   546,   321,   323,   331,    -1,   546,   321,
     194,   331,    -1,   546,   321,   322,   331,    -1,   546,    80,
     553,   331,    -1,   319,    -1,   320,    -1,    84,    -1,   267,
      -1,   315,    -1,   316,    -1,   317,    -1,   318,    -1,    -1,
     208,   342,    -1,   307,   548,    -1,    -1,   550,   549,   331,
      -1,    -1,   333,   550,   334,    -1,    -1,   552,   393,   551,
      -1,   381,   326,    -1,   554,   382,   333,    -1,   555,   383,
     331,    -1,   555,   284,   285,   344,   331,    -1,   555,   242,
      70,   387,   331,    -1,   555,   559,   331,    -1,   131,   347,
      -1,   202,   345,   346,    -1,   557,   295,   342,    -1,   557,
      -1,   558,   557,    -1,   556,   202,   345,   346,    -1,   556,
     202,    20,    -1,   556,   328,   558,    -1,   556,   244,   345,
      -1,   556,   235,    -1,   556,   148,    -1,   556,   327,    -1,
     128,   326,   413,     3,    -1,   331,    -1,   343,    -1,   352,
      -1,   358,    -1,   361,    -1,   371,    -1,   373,    -1,   377,
      -1,   380,    -1,   563,    -1,   388,    -1,   449,    -1,   451,
      -1,   461,    -1,   362,    -1,   363,    -1,   364,    -1,   409,
      -1,   410,    -1,   411,    -1,   412,    -1,   414,    -1,   415,
      -1,   416,    -1,   419,    -1,   422,    -1,   425,    -1,   426,
      -1,   427,    -1,   428,    -1,   429,    -1,   430,    -1,   431,
      -1,   432,    -1,   433,    -1,   434,    -1,   435,    -1,   438,
      -1,   439,    -1,   440,    -1,   441,    -1,   442,    -1,   443,
      -1,   444,    -1,   445,    -1,   446,    -1,   534,    -1,   535,
      -1,   536,    -1,   537,    -1,   538,    -1,   539,    -1,   540,
      -1,   541,    -1,   560,    -1,   564,   334,    -1,   565,   334,
      -1,   395,   334,    -1,   501,   334,    -1,   503,   334,    -1,
     544,   334,    -1,   546,   334,    -1,   555,   334,    -1,   366,
     382,   333,    -1,   564,   383,   331,    -1,   564,   367,   331,
      -1,   564,   370,   331,    -1,   368,   382,   333,    -1,   565,
     383,   331,    -1,   565,   369,   331,    -1,    67,    -1,    68,
      -1,    71,    -1,    72,    -1,   127,    -1,     5,    -1,   217,
      -1,   214,    -1,   218,    -1,   216,    -1,   215,    -1,   219,
      -1,   220,    -1,   221,    -1,   240,    -1,   241,    -1,   222,
      -1,   252,    -1,   253,    -1,   254,    -1,   255,    -1,   324,
      -1,   325,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   643,   643,   644,   647,   649,   656,   657,   658,   664,
     668,   677,   678,   679,   680,   681,   682,   688,   689,   696,
     697,   701,   708,   709,   713,   717,   724,   732,   733,   739,
     748,   749,   753,   758,   759,   763,   764,   768,   769,   773,
     774,   775,   776,   777,   778,   779,   780,   781,   787,   788,
     797,   798,   799,   800,   803,   804,   805,   806,   810,   817,
     820,   823,   827,   828,   836,   841,   842,   846,   853,   859,
     864,   868,   882,   895,   901,   902,   903,   914,   916,   918,
     922,   923,   924,   925,   932,   933,   937,   945,   949,   951,
     957,   958,   965,   966,   970,   976,   980,   990,   992,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1009,
    1010,  1011,  1012,  1016,  1017,  1018,  1019,  1020,  1024,  1033,
    1041,  1042,  1050,  1058,  1059,  1060,  1064,  1065,  1069,  1074,
    1075,  1082,  1089,  1090,  1091,  1095,  1103,  1109,  1110,  1111,
    1115,  1116,  1120,  1121,  1122,  1123,  1124,  1125,  1131,  1132,
    1133,  1137,  1138,  1142,  1143,  1149,  1150,  1153,  1155,  1159,
    1160,  1164,  1182,  1183,  1184,  1185,  1186,  1187,  1195,  1198,
    1201,  1204,  1208,  1209,  1212,  1215,  1219,  1223,  1229,  1235,
    1242,  1247,  1252,  1257,  1261,  1265,  1275,  1283,  1287,  1294,
    1295,  1300,  1304,  1311,  1318,  1322,  1330,  1331,  1332,  1337,
    1341,  1344,  1345,  1346,  1347,  1348,  1349,  1350,  1355,  1360,
    1368,  1375,  1382,  1390,  1392,  1394,  1396,  1398,  1400,  1402,
    1405,  1411,  1412,  1413,  1417,  1418,  1424,  1426,  1428,  1430,
    1432,  1434,  1438,  1442,  1445,  1449,  1450,  1451,  1455,  1456,
    1457,  1465,  1465,  1475,  1479,  1480,  1481,  1482,  1483,  1484,
    1485,  1486,  1487,  1488,  1489,  1490,  1491,  1512,  1523,  1524,
    1531,  1532,  1539,  1548,  1552,  1556,  1580,  1581,  1585,  1598,
    1598,  1614,  1615,  1618,  1619,  1623,  1626,  1635,  1648,  1649,
    1650,  1651,  1655,  1656,  1657,  1658,  1659,  1663,  1666,  1667,
    1668,  1672,  1675,  1684,  1687,  1688,  1689,  1690,  1694,  1695,
    1696,  1700,  1701,  1702,  1703,  1707,  1708,  1709,  1710,  1714,
    1715,  1719,  1720,  1724,  1731,  1732,  1733,  1734,  1741,  1742,
    1745,  1746,  1753,  1765,  1766,  1770,  1771,  1775,  1776,  1777,
    1778,  1779,  1783,  1784,  1785,  1789,  1790,  1791,  1792,  1793,
    1794,  1795,  1796,  1797,  1798,  1799,  1803,  1804,  1814,  1818,
    1841,  1877,  1879,  1880,  1881,  1882,  1883,  1884,  1885,  1889,
    1890,  1891,  1892,  1893,  1894,  1895,  1896,  1897,  1898,  1899,
    1900,  1901,  1902,  1903,  1904,  1906,  1907,  1908,  1910,  1912,
    1914,  1916,  1917,  1918,  1919,  1920,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1938,  1939,  1944,  1967,  1968,  1969,  1970,
    1971,  1972,  1976,  1979,  1980,  1981,  1989,  1996,  2005,  2006,
    2010,  2016,  2026,  2035,  2041,  2046,  2053,  2058,  2064,  2065,
    2066,  2074,  2075,  2076,  2077,  2078,  2084,  2105,  2106,  2107,
    2108,  2109,  2119,  2120,  2121,  2122,  2123,  2124,  2125,  2126,
    2127,  2128,  2129,  2130,  2131,  2132,  2133,  2134,  2135,  2136,
    2137,  2138,  2139,  2140,  2141,  2142,  2143,  2144,  2145,  2146,
    2147,  2148,  2149,  2150,  2151,  2156,  2157,  2158,  2159,  2160,
    2171,  2181,  2182,  2186,  2187,  2188,  2189,  2190,  2191,  2194,
    2209,  2212,  2214,  2218,  2219,  2220,  2221,  2222,  2223,  2224,
    2225,  2226,  2227,  2227,  2228,  2228,  2229,  2230,  2231,  2235,
    2236,  2240,  2248,  2250,  2254,  2255,  2256,  2260,  2261,  2264,
    2266,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,  2277,
    2278,  2279,  2282,  2304,  2305,  2306,  2307,  2308,  2309,  2310,
    2311,  2312,  2313,  2314,  2315,  2316,  2317,  2318,  2319,  2320,
    2321,  2322,  2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,
    2331,  2332,  2333,  2336,  2338,  2341,  2343,  2352,  2354,  2355,
    2358,  2360,  2364,  2365,  2367,  2376,  2387,  2407,  2409,  2412,
    2414,  2417,  2419,  2423,  2427,  2428,  2433,  2436,  2439,  2444,
    2447,  2452,  2455,  2459,  2463,  2466,  2467,  2468,  2469,  2470,
    2471,  2472,  2473,  2479,  2486,  2487,  2488,  2493,  2494,  2500,
    2507,  2508,  2509,  2510,  2511,  2512,  2513,  2514,  2515,  2516,
    2517,  2518,  2519,  2523,  2524,  2525,  2530,  2531,  2532,  2533,
    2534,  2537,  2538,  2539,  2542,  2544,  2547,  2549,  2553,  2562,
    2569,  2576,  2577,  2578,  2579,  2580,  2583,  2592,  2600,  2607,
    2608,  2612,  2617,  2623,  2626,  2630,  2631,  2632,  2635,  2642,
    2642,  2642,  2642,  2642,  2642,  2642,  2642,  2642,  2642,  2642,
    2642,  2642,  2642,  2643,  2643,  2643,  2643,  2643,  2643,  2643,
    2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,
    2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,
    2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,  2643,
    2643,  2643,  2643,  2643,  2644,  2644,  2644,  2644,  2644,  2644,
    2644,  2644,  2645,  2645,  2645,  2645,  2646,  2646,  2646,  2647,
    2647,  2647,  2647,  2647,  2648,  2648,  2649,  2650,  2651,  2652,
    2653,  2654,  2655,  2656,  2657,  2658,  2659,  2659,  2659,  2659,
    2659,  2659
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "CLI_MARKER", "INVALID_TOKEN",
  "ELSECOL", "DDOT", "GEQ", "LEQ", "NEQ", "AND", "OR", "PO", "PC", "NUM",
  "ENUM", "RTRID", "IPA", "SYM", "TEXT", "PREFIX_DUMMY", "'='", "'<'",
  "'>'", "'~'", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "'.'", "DEFINE",
  "ON", "OFF", "YES", "NO", "LOG", "SYSLOG", "ALL", "DEBUG", "TRACE",
  "INFO", "REMOTE", "WARNING", "ERROR", "AUTH", "FATAL", "BUG", "STDERR",
  "SOFT", "TIMEFORMAT", "ISO", "SHORT", "LONG", "BASE", "NAME",
  "CONFIGURE", "DOWN", "KERNEL", "PERSIST", "SCAN", "TIME", "LEARN",
  "DEVICE", "ROUTES", "KRT_SOURCE", "KRT_METRIC", "ASYNC", "TABLE",
  "KRT_PREFSRC", "KRT_REALM", "ROUTER", "ID", "PROTOCOL", "TEMPLATE",
  "PREFERENCE", "DISABLED", "DIRECT", "INTERFACE", "IMPORT", "EXPORT",
  "FILTER", "NONE", "STATES", "FILTERS", "LIMIT", "ACTION", "WARN",
  "BLOCK", "RESTART", "DISABLE", "PASSWORD", "FROM", "PASSIVE", "TO",
  "EVENTS", "PACKETS", "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS",
  "COUNT", "FOR", "COMMANDS", "PREEXPORT", "GENERATE", "ROA", "MAX",
  "FLUSH", "LISTEN", "BGP", "V6ONLY", "DUAL", "ADDRESS", "PORT",
  "PASSWORDS", "DESCRIPTION", "SORTED", "RELOAD", "IN", "OUT", "MRTDUMP",
  "MESSAGES", "RESTRICT", "MEMORY", "IGP_METRIC", "SHOW", "STATUS",
  "SUMMARY", "ROUTE", "SYMBOLS", "ADD", "DELETE", "DUMP", "RESOURCES",
  "SOCKETS", "NEIGHBORS", "ATTRIBUTES", "ECHO", "ENABLE", "FUNCTION",
  "PRINT", "PRINTN", "UNSET", "RETURN", "ACCEPT", "REJECT", "QUITBIRD",
  "INT", "BOOL", "IP", "PREFIX", "PAIR", "QUAD", "EC", "SET", "STRING",
  "BGPMASK", "BGPPATH", "CLIST", "ECLIST", "IF", "THEN", "ELSE", "CASE",
  "TRUE", "FALSE", "RT", "RO", "UNKNOWN", "GENERIC", "GW", "NET", "MASK",
  "PROTO", "SOURCE", "SCOPE", "CAST", "DEST", "LEN", "DEFINED", "CONTAINS",
  "RESET", "PREPEND", "FIRST", "LAST", "MATCH", "ROA_CHECK", "EMPTY",
  "WHERE", "EVAL", "LOCAL", "NEIGHBOR", "AS", "HOLD", "CONNECT", "RETRY",
  "KEEPALIVE", "MULTIHOP", "STARTUP", "VIA", "NEXT", "HOP", "SELF",
  "DEFAULT", "PATH", "METRIC", "START", "DELAY", "FORGET", "WAIT", "AFTER",
  "BGP_PATH", "BGP_LOCAL_PREF", "BGP_MED", "BGP_ORIGIN", "BGP_NEXT_HOP",
  "BGP_ATOMIC_AGGR", "BGP_AGGREGATOR", "BGP_COMMUNITY",
  "BGP_EXT_COMMUNITY", "RR", "RS", "CLIENT", "CLUSTER", "AS4", "ADVERTISE",
  "IPV4", "CAPABILITIES", "PREFER", "OLDER", "MISSING", "LLADDR", "DROP",
  "IGNORE", "REFRESH", "INTERPRET", "COMMUNITIES", "BGP_ORIGINATOR_ID",
  "BGP_CLUSTER_LIST", "IGP", "GATEWAY", "RECURSIVE", "MED", "TTL",
  "SECURITY", "DETERMINISTIC", "SECONDARY", "OSPF", "AREA", "OSPF_METRIC1",
  "OSPF_METRIC2", "OSPF_TAG", "OSPF_ROUTER_ID", "RFC1583COMPAT", "STUB",
  "TICK", "COST", "COST2", "RETRANSMIT", "HELLO", "TRANSMIT", "PRIORITY",
  "DEAD", "TYPE", "BROADCAST", "BCAST", "NONBROADCAST", "NBMA",
  "POINTOPOINT", "PTP", "POINTOMULTIPOINT", "PTMP", "SIMPLE",
  "AUTHENTICATION", "STRICT", "CRYPTOGRAPHIC", "ELIGIBLE", "POLL",
  "NETWORKS", "HIDDEN", "VIRTUAL", "CHECK", "LINK", "RX", "BUFFER",
  "LARGE", "NORMAL", "STUBNET", "TAG", "EXTERNAL", "LSADB", "ECMP",
  "WEIGHT", "NSSA", "TRANSLATOR", "STABILITY", "GLOBAL", "LSID",
  "INSTANCE", "REAL", "TOPOLOGY", "STATE", "PIPE", "PEER", "MODE",
  "OPAQUE", "TRANSPARENT", "RIP", "INFINITY", "PERIOD", "GARBAGE",
  "TIMEOUT", "MULTICAST", "QUIET", "NOLISTEN", "VERSION1", "PLAINTEXT",
  "MD5", "HONOR", "NEVER", "ALWAYS", "RIP_METRIC", "RIP_TAG", "STATIC",
  "PROHIBIT", "MULTIPATH", "'('", "')'", "';'", "':'", "'{'", "'}'", "','",
  "'?'", "'['", "']'", "$accept", "config", "conf_entries", "expr",
  "definition", "bool", "ipa", "ipa_scope", "prefix", "prefix_or_ipa",
  "pxlen", "datetime", "text_or_none", "log_config", "syslog_name",
  "log_file", "log_mask", "log_mask_list", "log_cat", "mrtdump_base",
  "timeformat_which", "timeformat_spec", "timeformat_base",
  "cmd_CONFIGURE", "cmd_CONFIGURE_SOFT", "cmd_DOWN", "cfg_name",
  "kern_proto_start", "kern_item", "kif_proto_start", "kif_item",
  "nl_item", "rtrid", "idval", "listen", "listen_opts", "listen_opt",
  "tab_sorted", "newtab", "roa_table_start", "roa_table_opts", "roa_table",
  "proto_start", "proto_name", "proto_item", "imexport", "limit_action",
  "limit_spec", "rtable", "debug_default", "iface_patt_node_init",
  "iface_patt_node_body", "iface_negate", "iface_patt_node",
  "iface_patt_list", "dev_proto_start", "dev_proto", "dev_iface_init",
  "dev_iface_patt", "debug_mask", "debug_list", "debug_flag",
  "mrtdump_mask", "mrtdump_list", "mrtdump_flag", "password_list",
  "password_items", "password_item", "password_item_begin",
  "password_item_params", "cmd_SHOW_STATUS", "cmd_SHOW_MEMORY",
  "cmd_SHOW_PROTOCOLS", "cmd_SHOW_PROTOCOLS_ALL", "optsym",
  "cmd_SHOW_INTERFACES", "cmd_SHOW_INTERFACES_SUMMARY", "cmd_SHOW_ROUTE",
  "r_args", "export_or_preexport", "cmd_SHOW_ROA", "roa_args", "roa_mode",
  "cmd_SHOW_SYMBOLS", "sym_args", "roa_table_arg", "cmd_ADD_ROA",
  "cmd_DELETE_ROA", "cmd_FLUSH_ROA", "cmd_DUMP_RESOURCES",
  "cmd_DUMP_SOCKETS", "cmd_DUMP_INTERFACES", "cmd_DUMP_NEIGHBORS",
  "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES", "cmd_DUMP_PROTOCOLS",
  "cmd_ECHO", "echo_mask", "echo_size", "cmd_DISABLE", "cmd_ENABLE",
  "cmd_RESTART", "cmd_RELOAD", "cmd_RELOAD_IN", "cmd_RELOAD_OUT",
  "cmd_DEBUG", "cmd_MRTDUMP", "cmd_RESTRICT", "proto_patt", "proto_patt2",
  "filter_def", "$@1", "filter_eval", "type", "one_decl", "decls",
  "declsn", "filter_body", "filter", "where_filter", "function_params",
  "function_body", "function_def", "$@2", "cmds", "cmds_int", "block",
  "fipa", "set_atom", "switch_atom", "pair_expr", "pair_atom", "pair_item",
  "ec_expr", "ec_kind", "ec_item", "set_item", "switch_item", "set_items",
  "switch_items", "fprefix_s", "fprefix", "fprefix_set", "switch_body",
  "bgp_path_expr", "bgp_path", "bgp_path_tail1", "bgp_path_tail2",
  "constant", "constructor", "rtadot", "function_call", "symbol",
  "static_attr", "term", "break_command", "print_one", "print_list",
  "var_listn", "var_list", "cmd", "bgp_proto_start", "bgp_proto",
  "ospf_proto_start", "ospf_proto", "ospf_proto_item", "ospf_area_start",
  "ospf_area", "ospf_area_opts", "ospf_area_item", "$@3", "$@4",
  "ospf_stubnet", "ospf_stubnet_start", "ospf_stubnet_opts",
  "ospf_stubnet_item", "ospf_vlink", "ospf_vlink_opts", "ospf_vlink_item",
  "ospf_vlink_start", "ospf_iface_item", "pref_list", "pref_item",
  "pref_base", "pref_opt", "ipa_list", "ipa_item", "ipa_el", "ipa_ne",
  "ospf_iface_start", "ospf_instance_id", "ospf_iface_opts",
  "ospf_iface_opt_list", "ospf_iface", "opttext", "cmd_SHOW_OSPF",
  "cmd_SHOW_OSPF_NEIGHBORS", "cmd_SHOW_OSPF_INTERFACE",
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_TOPOLOGY_ALL",
  "cmd_SHOW_OSPF_STATE", "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB",
  "lsadb_args", "pipe_proto_start", "pipe_proto", "rip_cfg_start",
  "rip_cfg", "rip_auth", "rip_mode", "rip_iface_item", "rip_iface_opts",
  "rip_iface_opt_list", "rip_iface_init", "rip_iface",
  "static_proto_start", "static_proto", "stat_route0", "stat_multipath1",
  "stat_multipath", "stat_route", "cmd_SHOW_STATIC", "conf", "cli_cmd",
  "proto", "kern_proto", "kif_proto", "dynamic_attr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    61,    60,    62,   126,    43,    45,    42,    47,
      37,    33,    46,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,   530,   531,   532,   533,
     534,   535,   536,   537,   538,   539,   540,   541,   542,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,    40,
      41,    59,    58,   123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   339,   340,   340,   341,   341,   342,   342,   342,   343,
     343,   344,   344,   344,   344,   344,   344,   345,   345,   346,
     346,   347,   348,   348,   349,   349,   350,   351,   351,   352,
     353,   353,   354,   354,   354,   355,   355,   356,   356,   357,
     357,   357,   357,   357,   357,   357,   357,   357,   358,   358,
     359,   359,   359,   359,   360,   360,   360,   360,   361,   362,
     363,   364,   365,   365,   366,   367,   367,   367,   367,   368,
     369,   369,   370,   371,   372,   372,   372,   373,   374,   374,
     375,   375,   375,   375,   376,   376,   377,   378,   379,   379,
     380,   380,   381,   381,   382,   382,   382,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   383,   383,   383,   384,
     384,   384,   384,   385,   385,   385,   385,   385,   386,   387,
     388,   388,   389,   390,   390,   390,   391,   391,   392,   393,
     393,   394,   395,   395,   395,   396,   397,   398,   398,   398,
     399,   399,   400,   400,   400,   400,   400,   400,   401,   401,
     401,   402,   402,   403,   403,   404,   404,   405,   405,   406,
     406,   407,   408,   408,   408,   408,   408,   408,   409,   410,
     411,   412,   413,   413,   414,   415,   416,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   418,
     418,   419,   420,   420,   420,   420,   421,   421,   421,   422,
     423,   423,   423,   423,   423,   423,   423,   423,   424,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   436,   436,   437,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   447,   447,   448,   448,
     448,   450,   449,   451,   452,   452,   452,   452,   452,   452,
     452,   452,   452,   452,   452,   452,   452,   453,   454,   454,
     455,   455,   456,   457,   457,   458,   459,   459,   460,   462,
     461,   463,   463,   464,   464,   465,   465,   466,   467,   467,
     467,   467,   468,   468,   468,   468,   468,   469,   470,   470,
     470,   471,   471,   472,   473,   473,   473,   473,   474,   474,
     474,   475,   475,   475,   475,   476,   476,   476,   476,   477,
     477,   478,   478,   479,   480,   480,   480,   480,   481,   481,
     482,   482,   482,   483,   483,   484,   484,   485,   485,   485,
     485,   485,   486,   486,   486,   487,   487,   487,   487,   487,
     487,   487,   487,   487,   487,   487,   488,   488,   489,   490,
     491,   492,   492,   492,   492,   492,   492,   492,   492,   493,
     493,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     493,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     493,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     493,   493,   493,   493,   493,   493,   494,   494,   494,   494,
     494,   494,   495,   496,   496,   496,   497,   497,   498,   498,
     499,   499,   499,   499,   499,   499,   499,   499,   499,   499,
     499,   499,   499,   499,   499,   499,   500,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     502,   503,   503,   504,   504,   504,   504,   504,   504,   505,
     506,   507,   507,   508,   508,   508,   508,   508,   508,   508,
     508,   508,   509,   508,   510,   508,   508,   508,   508,   511,
     511,   512,   513,   513,   514,   514,   514,   515,   515,   516,
     516,   517,   517,   517,   517,   517,   517,   517,   517,   517,
     517,   517,   518,   519,   519,   519,   519,   519,   519,   519,
     519,   519,   519,   519,   519,   519,   519,   519,   519,   519,
     519,   519,   519,   519,   519,   519,   519,   519,   519,   519,
     519,   519,   519,   520,   520,   521,   522,   523,   523,   523,
     524,   524,   525,   525,   526,   527,   528,   529,   529,   530,
     530,   531,   531,   532,   533,   533,   534,   535,   536,   537,
     538,   539,   540,   541,   542,   542,   542,   542,   542,   542,
     542,   542,   542,   543,   544,   544,   544,   544,   544,   545,
     546,   546,   546,   546,   546,   546,   546,   546,   546,   546,
     546,   546,   546,   547,   547,   547,   548,   548,   548,   548,
     548,   549,   549,   549,   550,   550,   551,   551,   552,   553,
     554,   555,   555,   555,   555,   555,   556,   557,   557,   558,
     558,   559,   559,   559,   559,   559,   559,   559,   560,   561,
     561,   561,   561,   561,   561,   561,   561,   561,   561,   561,
     561,   561,   561,   562,   562,   562,   562,   562,   562,   562,
     562,   562,   562,   562,   562,   562,   562,   562,   562,   562,
     562,   562,   562,   562,   562,   562,   562,   562,   562,   562,
     562,   562,   562,   562,   562,   562,   562,   562,   562,   562,
     562,   562,   562,   562,   563,   563,   563,   563,   563,   563,
     563,   563,   564,   564,   564,   564,   565,   565,   565,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     3,     1,     5,
       5,     1,     1,     1,     1,     1,     0,     1,     1,     0,
       2,     2,     1,     1,     2,     2,     1,     1,     0,     4,
       2,     0,     1,     2,     1,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     3,
       1,     1,     1,     1,     2,     4,     3,     3,     3,     3,
       4,     2,     0,     1,     2,     2,     3,     2,     3,     2,
       3,     3,     3,     4,     1,     1,     1,     4,     0,     2,
       2,     2,     1,     1,     0,     1,     3,     3,     0,     8,
       1,     4,     1,     1,     0,     1,     3,     0,     2,     2,
       2,     2,     2,     2,     3,     3,     2,     3,     2,     2,
       1,     1,     1,     0,     2,     2,     2,     2,     2,     1,
       3,     3,     0,     1,     1,     2,     0,     1,     3,     1,
       3,     2,     3,     3,     3,     0,     3,     1,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     4,     1,     0,     3,     4,
       1,     2,     0,     5,     5,     5,     5,     4,     3,     3,
       4,     5,     1,     0,     3,     4,     4,     0,     2,     3,
       3,     3,     2,     2,     2,     3,     3,     2,     2,     1,
       1,     4,     0,     3,     3,     3,     0,     1,     1,     4,
       0,     2,     2,     2,     2,     2,     2,     2,     0,     2,
       9,     9,     4,     3,     3,     3,     3,     3,     3,     3,
       4,     1,     1,     1,     0,     1,     3,     3,     3,     3,
       4,     4,     4,     4,     2,     1,     1,     1,     1,     0,
       1,     0,     4,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     0,     3,
       1,     3,     1,     1,     1,     2,     3,     2,     4,     0,
       5,     0,     1,     1,     2,     1,     3,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     5,    11,     1,     1,     1,     2,     1,     7,     9,
       7,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       3,     1,     3,     3,     1,     2,     2,     6,     1,     3,
       0,     4,     3,     1,     3,     3,     3,     2,     2,     2,
       2,     0,     2,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     1,     1,     5,     7,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     4,     1,     1,     1,     1,     2,
       2,     3,     3,     6,     3,     3,     3,     3,     5,     6,
       6,     6,     6,     4,     8,     4,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     3,     1,     3,     0,     1,
       4,     6,     4,     3,     5,     5,     4,     6,     3,     2,
       5,     5,     8,     8,     8,     8,     2,     3,     3,     5,
       6,     7,     6,     4,     4,     5,     6,     6,     5,     3,
       4,     5,     5,     5,     5,     4,     4,     5,     5,     5,
       5,     5,     5,     5,     5,     6,     6,     8,     6,     6,
       5,     4,     5,     4,     5,     4,     5,     4,     5,     5,
       2,     3,     3,     1,     2,     2,     4,     2,     1,     2,
       4,     0,     3,     2,     1,     2,     3,     3,     3,     3,
       2,     3,     0,     5,     0,     5,     2,     2,     1,     4,
       1,     1,     0,     3,     2,     2,     2,     5,     2,     0,
       3,     0,     2,     2,     3,     2,     2,     3,     2,     2,
       2,     1,     3,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     3,
       2,     3,     2,     3,     3,     4,     2,     2,     2,     3,
       3,     3,     1,     0,     2,     3,     1,     0,     1,     2,
       0,     2,     1,     1,     2,     3,     0,     0,     2,     0,
       3,     0,     3,     4,     1,     0,     4,     6,     6,     6,
       7,     6,     7,     5,     0,     2,     3,     2,     3,     3,
       2,     3,     2,     2,     3,     3,     5,     4,     4,     2,
       3,     3,     4,     4,     4,     5,     5,     4,     3,     4,
       4,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     2,     0,     3,     0,     3,     0,     3,
       2,     3,     3,     5,     5,     3,     2,     3,     3,     1,
       2,     4,     3,     3,     3,     2,     2,     2,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    62,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   682,   683,
     684,   685,   686,   687,   688,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
       3,     1,     2,     0,     0,     0,     0,     0,     0,    92,
      93,     0,     0,     0,     0,     0,   348,   649,   650,   651,
     652,   653,    94,    94,   654,   655,   656,    90,   657,     0,
     659,    94,    97,   660,   661,   662,    94,    97,    94,    97,
      94,    97,    94,    97,    94,    97,     5,   658,    97,    97,
     235,   237,   236,     0,    63,    62,     0,    61,     0,     0,
     208,     0,     0,     0,     0,   234,   239,     0,   192,     0,
       0,   177,   200,   173,   173,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   223,   222,   221,   224,     0,     0,
      32,    31,    34,     0,     0,     0,    53,    52,    51,    50,
       0,     0,    84,     0,   241,     0,    78,     0,     0,   269,
     331,   335,   344,   341,   277,   350,   338,     0,     0,   334,
     348,   378,     0,     0,     0,   336,   337,     0,     0,     0,
     348,     0,   339,   340,   345,   376,   377,     0,   375,   243,
      95,     0,     0,    88,    64,    69,   131,   426,   470,   593,
     599,   630,     0,     0,     0,     0,     0,    16,   135,     0,
       0,     0,     0,   706,     0,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,    16,   707,     0,
       0,     0,    16,     0,    16,   708,   473,     0,     0,   478,
       0,     0,     0,   709,     0,     0,   628,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   710,     0,     0,   156,
     160,     0,     0,     0,     0,   711,     0,     0,     0,     0,
      16,     0,    16,     0,   704,     0,     0,     0,     0,    28,
     705,     0,     0,   138,   137,     0,     0,     0,    59,   228,
     226,     0,     0,     0,     0,   229,   149,   148,     0,     0,
     238,   240,   239,     0,   174,     0,   196,   169,   168,     0,
       0,   172,   173,   173,   584,   173,   173,     0,     0,    17,
      18,     0,     0,     0,   218,   219,   215,   213,   214,   216,
     217,   225,     0,   227,     0,     0,    33,    35,     0,     0,
     120,     6,     8,   348,   121,    54,     0,    58,    85,    86,
      74,    75,    76,     0,   258,    87,     0,    49,     0,     0,
     331,   350,   331,   348,   331,   331,     0,   323,     0,   348,
       0,     0,     0,   334,   334,     0,   373,   348,   348,   348,
     348,   348,     0,   294,   295,     0,   297,     0,     0,   281,
     279,   348,   278,   280,   303,   301,   302,   309,     0,   314,
     318,     0,   724,   719,   720,   721,   722,   351,   723,   352,
     353,   354,   355,   356,   357,   358,   726,   729,   728,   725,
     727,   730,   731,   732,   735,   733,   734,   736,   737,   738,
     739,   740,   741,   379,   380,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,     0,     0,
     712,   716,     0,   132,   100,   119,   106,     0,    98,    12,
      14,    13,    15,    11,    99,   122,   111,   258,   112,     0,
     348,   102,   110,     0,   103,   108,   101,   133,   134,   427,
       0,     0,     0,     0,     0,     0,     0,    16,     0,     0,
       0,    19,     0,     0,     0,   439,     0,     0,     0,     0,
       0,    16,     0,     0,     0,     0,    16,     0,    16,     0,
      16,     0,    16,     0,     0,    16,    16,    16,     0,   428,
     471,   479,   474,   477,   475,   472,   481,   594,     0,     0,
       0,   595,   600,   122,     0,   161,     0,   157,   615,   613,
     614,     0,     0,     0,     0,     0,     0,     0,     0,   601,
     608,   162,   631,   636,     0,    16,   632,   646,     0,   645,
       0,   647,     0,   635,     0,    65,     0,    67,    16,   714,
     715,   713,     0,    27,     0,   718,   717,   143,   142,   144,
     146,   147,   145,     0,   140,   232,    60,   209,   212,   230,
     231,   153,   154,     0,   151,   233,     0,   170,   175,   191,
       0,   198,   197,     0,     0,   176,   183,     0,     0,   190,
     258,   184,   187,   188,     0,   189,   178,     0,   182,   199,
     207,   201,   204,   205,   203,   206,   202,   575,   575,     0,
     173,   575,   173,   575,   576,   648,     0,     0,    21,     0,
       0,   220,     0,     0,    30,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,    37,    29,     0,     0,    56,
      57,    73,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,     0,     0,     0,   242,   262,    82,
      83,     0,     0,    77,    79,    48,     0,   258,   327,   328,
       0,   329,   330,   325,   313,   406,   409,     0,   386,     0,
     387,   332,   333,   326,     0,     0,     0,     0,     0,     0,
     296,   348,   359,   348,   290,   288,     0,     0,   287,     0,
       0,   342,   315,   316,     0,     0,   343,   371,   369,   367,
     364,   365,   366,   368,   370,   372,   360,   361,   362,   363,
     381,     0,   382,   384,   385,    96,     0,    91,   107,   126,
     129,   136,   263,   264,   109,   113,   104,   265,   105,     0,
       0,    16,   463,   465,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   440,     0,     0,     0,     0,
       0,     0,   433,     0,   434,     0,   461,     0,     0,     0,
       0,     0,     0,     0,   445,   446,     0,     0,     0,   467,
       0,     0,     0,   597,   598,   626,   612,   603,     0,     0,
     607,   602,   604,     0,     0,   610,   611,   609,     0,     0,
       0,     0,     0,     0,   642,    19,   644,     0,   639,   643,
      72,    66,    68,    70,    23,    22,    71,   139,     0,   150,
       0,   171,   195,   194,   193,   180,   186,   181,   179,   185,
     574,     0,     0,   583,   592,     0,   590,     0,     0,   587,
     585,     0,   575,     0,   575,     0,    24,    25,     0,     0,
      10,     9,    36,     0,     7,    55,   257,   256,   258,   348,
      80,    81,   267,   260,     0,   270,   324,   348,   395,     0,
     348,   348,   348,   374,   348,   393,   348,     0,     0,   348,
     348,   348,   277,   304,   310,     0,     0,   319,   348,     0,
     127,     0,   122,     0,   118,     0,     0,     0,   464,     0,
     460,   454,   429,     0,    20,     0,   435,     0,   438,     0,
     441,   453,   452,   447,     0,     0,   462,   450,   442,   443,
     444,   466,   468,   449,   448,   469,   451,   476,   566,    16,
       0,    16,   492,     0,     0,   494,   484,    16,   480,     0,
     498,   567,   596,   624,   629,   155,   157,   605,   606,     0,
       0,     0,     0,     0,   159,   634,   633,   641,    19,     0,
     640,   141,   152,   578,   577,   591,   586,   588,   589,     0,
     579,     0,   581,     0,     0,    38,   259,     0,   399,     0,
     400,   401,     0,   348,   397,   398,   396,   348,   348,     0,
     348,     0,     0,   348,   273,     0,   266,   407,   388,     0,
       0,     0,     0,     0,   348,   346,   289,   287,     0,     0,
     293,     0,     0,     0,   123,   124,   128,   130,   114,   115,
     116,   117,   456,     0,   458,   459,   430,     0,   437,   436,
     455,   432,   122,   497,   485,     0,     0,    16,     0,   483,
       0,     0,   501,   496,   500,     0,     0,   490,   482,     0,
     508,   621,   158,   162,    26,     0,     0,     0,     0,   637,
     638,   580,   582,   208,   208,   348,   348,   348,   348,     0,
       0,     0,   268,   274,     0,     0,   419,   402,   404,     0,
     261,   392,   390,   391,   389,   348,     0,   291,   348,     0,
     383,     0,   125,     0,   431,   567,   487,   488,   486,   489,
     553,   522,   502,   553,   491,   568,   509,     0,     0,   627,
       0,   167,   162,   162,   162,   162,     0,     0,     0,     0,
       0,     0,   413,   348,   320,   348,   348,     0,   348,   418,
       0,   347,     0,     0,     0,   317,     0,   457,   571,     0,
       0,     0,   511,   622,   616,   617,   618,   619,   620,   623,
     625,   163,   164,   165,   166,   210,   211,   412,   349,   416,
       0,   348,   410,   275,     0,     0,     0,     0,     0,     0,
       0,     0,   405,   394,   348,   300,   348,   298,     0,   569,
     573,   493,   556,   554,   557,    16,     0,    16,   499,     0,
     495,     0,     0,     0,     0,     0,     0,   507,   521,     0,
       0,     0,   348,   348,   282,   286,   284,   348,   420,   285,
     307,   305,   306,   311,     0,   415,   414,   348,   348,   348,
     348,   421,     0,     0,    89,     0,   558,     0,     0,   505,
     506,   504,   503,   515,   513,   512,     0,     0,   516,   518,
     519,   520,   510,   417,   276,   411,   322,   287,     0,   348,
       0,     0,     0,     0,     0,   348,   299,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   572,   552,     0,   559,   555,   514,
     517,   283,   348,   308,   321,   312,     0,     0,     0,     0,
       0,   560,   527,   542,   523,   526,   524,     0,   540,     0,
     528,   530,   531,   532,   533,   534,   535,   536,   537,   546,
     547,   548,    16,   525,    16,     0,     0,    16,   570,     0,
     425,   423,   424,   422,   292,     0,   539,   529,   541,   543,
     549,   550,   551,   544,   538,     0,   545,   561,   562,   563,
       0,   564,   565
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   493,    78,   494,   351,   801,   865,   866,
     668,  1105,   604,    79,   366,   153,   369,   684,   685,    80,
     160,   161,    81,    19,    20,    21,   116,    82,   305,    83,
     311,   306,    84,   383,    85,   386,   714,   379,    86,    87,
     482,    88,    89,   201,   224,   501,   944,   786,   486,    90,
     779,  1066,   941,   780,   781,    91,    92,   495,   225,   316,
     613,   614,   329,   623,   624,   288,   838,   289,   290,   851,
      22,    23,    24,    25,   347,    26,    27,    28,   339,   647,
      29,   336,   634,    30,   340,   322,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,   147,   362,    42,
      43,    44,    45,    46,    47,    48,    49,    50,   113,   333,
      93,   384,    94,   704,   705,   706,   914,   783,   784,   502,
     717,   708,    95,   389,  1039,  1040,  1212,   192,   424,  1260,
     745,   746,   425,  1059,   747,   426,   427,  1263,   428,  1264,
     193,   430,   431,  1214,   395,   194,   396,   405,   195,   196,
     197,  1042,   198,   463,  1057,  1043,  1128,  1129,   726,   727,
    1044,    96,    97,    98,    99,   267,   268,   269,   831,   989,
    1090,  1095,  1093,  1094,  1190,  1239,   990,  1192,  1249,   991,
    1326,  1189,  1233,  1234,  1278,  1375,  1387,  1388,  1389,  1082,
    1100,  1275,  1230,  1083,   881,    51,    52,    53,    54,    55,
      56,    57,    58,   659,   100,   101,   102,   103,   571,  1199,
    1160,  1101,   994,   563,   564,   104,   105,   297,   858,   859,
     298,    59,   106,    60,   107,   108,   109,   464
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1155
static const yytype_int16 yypact[] =
{
      67,  2348,    63,   166,   452,   259,   110,   452,   452,    62,
     456,   452,   125,   606,   143,   179,   773,   347,   452, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155,   121,   393,   138,   494,   250,   159, -1155,
   -1155,   256,   257,   241,   210,   324,  1248, -1155, -1155, -1155,
   -1155, -1155,   356,   356, -1155, -1155, -1155,    48, -1155,   725,
   -1155,   356,  2203, -1155, -1155, -1155,   356,  2109,   356,  1954,
     356,   621,   356,  1941,   356,  2143, -1155, -1155,  1869,  2034,
   -1155, -1155, -1155,   -13, -1155,   365,   430, -1155,   449,   455,
     378,   452,   452,   488,    75, -1155,   468,    83, -1155,   490,
     508, -1155, -1155,   160,   539,   481,   481,   510,   558,   601,
     633,   645,   647,   650, -1155, -1155, -1155,   553,   661,   572,
   -1155,   609, -1155,    -5,   -13,   252, -1155, -1155, -1155, -1155,
     157,   345,   562,   615, -1155,   671, -1155,   362,    75, -1155,
       0, -1155, -1155, -1155,   666,   368, -1155,   511,    24,     2,
    1248, -1155,   386,   402,   405, -1155, -1155,   407,   419,   435,
     781,    59, -1155, -1155, -1155, -1155, -1155,  1906, -1155,  2482,
     613,   389,   447, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155,   451,   -13,   721,   714,   252,   108, -1155,   472,
     525,   769,    75, -1155,   460,   462,   469,   399,   582,   785,
     108,   719,   -72,   694,   265,   481,   750,   616,   757,    10,
     625,   618,   406,   617,   614,   401,   598,   600,   108,   602,
     596,   593,    -8,    -9,   630,   586,   595,   108, -1155,   512,
     514,   615,   108,   252,   108, -1155, -1155,   518,   520, -1155,
     526,   782,   337, -1155,   534,   533, -1155,   831,   252,   536,
     -38,   252,   252,   807,   811,   -29, -1155,   540,   546, -1155,
     545,   547,   481,   809,   599, -1155,   550,   226,   560,   823,
     108,   835,   108,   841, -1155,   587,   589,   594,   850,   897,
   -1155,   604,   607, -1155, -1155,   797,   919,   921, -1155, -1155,
   -1155,   912,   934,   936,   937, -1155, -1155, -1155,   167,   938,
   -1155, -1155,   641,   939, -1155,   940,   151, -1155, -1155,  1101,
     889, -1155,   539,   539, -1155,   282,   367,   942,   943, -1155,
   -1155,    -6,   845,   847, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155,   957, -1155,   238,   948, -1155, -1155,  1413,   638,
   -1155, -1155, -1155,  1248, -1155,   252,   619, -1155, -1155, -1155,
   -1155, -1155, -1155,   640,  1892, -1155,    78, -1155,   642,   648,
       0, -1155,     0,  1248,     0,     0,   960, -1155,   961,   972,
     953,   790,   954,     2,     2,   955,   950,  1248,  1248,  1248,
    1248,  1248,   964, -1155, -1155,   971, -1155,   658,   236, -1155,
   -1155,   734, -1155, -1155,   987, -1155, -1155, -1155,   171,    17,
   -1155,   214, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155,  1248,  1248,  1248,  1248,  1248,
    1248,  1248,  1248,  1248,  1248,  1248,  1248,  1248,   453,   981,
   -1155, -1155,   -56, -1155, -1155, -1155, -1155,   615, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155,  2295, -1155,   252,
    1248, -1155, -1155,   252, -1155, -1155, -1155, -1155, -1155, -1155,
     941,   944,   956,   675,   677,   252,   772,   108,   481,   252,
     816,   988,   252,   958,   252, -1155,   688,   959,   815,   252,
     252,   108,   969,   702,   973,   703,   108,   707,   108,   200,
     108,   721,   108,   712,   722,   108,   108,   108,   723, -1155,
   -1155, -1155, -1155, -1155,   965, -1155, -1155, -1155,  1026,   726,
     727, -1155, -1155, -1155,   729, -1155,   730,   963, -1155, -1155,
   -1155,   738,   742,   746,   252,   252,   751,   752,   754, -1155,
   -1155,   310, -1155, -1155,   721,   108, -1155, -1155,   565, -1155,
     481, -1155,   877, -1155,   252, -1155,   252, -1155,   108, -1155,
   -1155, -1155,   252, -1155,   481, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155,   408, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155,   411, -1155, -1155,  1078, -1155, -1155, -1155,
    1067, -1155, -1155,  1072,   481, -1155, -1155,  1069,  1073, -1155,
    2295, -1155, -1155, -1155,   481, -1155, -1155,  1075, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155,  1076,  1076,   576,
     539,  1076,   539,  1076, -1155, -1155,   252,   481, -1155,  1080,
    1082, -1155,   767,   770, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155,   443, -1155, -1155,   109,  1083, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155,    65,   776,   775, -1155, -1155, -1155,
   -1155,   481,   252, -1155, -1155, -1155,  1762,  1892, -1155, -1155,
    1491, -1155, -1155, -1155, -1155,   457, -1155,   779, -1155,  1085,
   -1155, -1155, -1155, -1155,   493,   519,   564,  1539,   591,   111,
   -1155,  1248, -1155,  1248, -1155,  1107,   780,   786,   109,    90,
     133, -1155, -1155, -1155,  1102,  1098, -1155,  1295,  1295,  1295,
    1375,  1375,  1295,  1295,  1295,  1295,   445,   445,   950,   950,
   -1155,   793, -1155, -1155, -1155, -1155,   481, -1155, -1155,  1096,
   -1155,   789, -1155, -1155, -1155,  1038, -1155,  2482, -1155,   252,
     252,   108, -1155, -1155,   796,   108,   798,   801,   802,   252,
    1109,   947,   805,   252,   806, -1155,   252,   812,   814,   817,
     818,   252, -1155,   615, -1155,   820, -1155,   821,   822,   824,
     837,   839,   842,   843, -1155, -1155,   844,   846,   849, -1155,
     252,  2039,   854, -1155, -1155,    85, -1155, -1155,   804,   856,
   -1155, -1155, -1155,   857,   858, -1155, -1155, -1155,   252,   331,
     335,   813,   859,   867, -1155,   988, -1155,   481,   871,   877,
   -1155, -1155, -1155, -1155,    -6, -1155, -1155, -1155,   797, -1155,
     167, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155,  1169,  1175, -1155, -1155,   615, -1155,   615,  1184, -1155,
   -1155,   615,  1076,  1197,  1076,  1198, -1155, -1155,  1011,  1013,
   -1155, -1155, -1155,  1413, -1155, -1155, -1155, -1155,  1892,  1150,
   -1155, -1155, -1155,   878,   880, -1155, -1155,  1248, -1155,  1185,
    1248,  1248,  1248, -1155,  1248, -1155,  1248,   660,  1564,  1248,
     997,  1248, -1155, -1155, -1155,   876,   666, -1155,  1248,  1116,
   -1155,   748, -1155,   629, -1155,   884,   881,   895, -1155,   898,
   -1155, -1155, -1155,   901, -1155,   252, -1155,   903, -1155,   906,
   -1155, -1155, -1155, -1155,   913,   914, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,   108,
      52,    54, -1155,   962,   481, -1155, -1155,   161, -1155,   915,
   -1155,   927, -1155, -1155, -1155, -1155,   963, -1155, -1155,   917,
    1233,  1233,  1233,  1233, -1155, -1155, -1155, -1155,   988,   252,
     871, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,  1252,
   -1155,  1254, -1155,  1243,  1244, -1155, -1155,    -4, -1155,  1238,
   -1155, -1155,   933,  1248, -1155, -1155, -1155,  1248,  1248,   935,
      15,  1906,   949,  1204, -1155,  1892, -1155, -1155, -1155,  1589,
    1614,  1646,  1671,   701,  1248, -1155, -1155,  2482,   952,   951,
    2482,  1255,  1696,  1257,   481, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155,   252, -1155, -1155, -1155,   976, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155,   252,   252,   108,   252, -1155,
     945,   615, -1155, -1155,   975,   977,   252, -1155, -1155,   252,
     978,   -34, -1155,   310, -1155,   983,   984,   998,   999, -1155,
   -1155, -1155, -1155,   378,   378,  1248,   972,  1248, -1155,   747,
    2406,    71, -1155, -1155,  1261,   273, -1155,  2482,   993,  1001,
   -1155, -1155, -1155, -1155, -1155,  1248,  1726,  1277,  1223,  1002,
   -1155,  1090, -1155,  1004, -1155,    68, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155,   252,   340, -1155,
    1008, -1155,   310,   310,   310,   310,  1285,  1286,  1336,   982,
    1416,  1922, -1155,  1820, -1155,  1248,  1248,   231,  1204, -1155,
    1751, -1155,  1012,  1003,     7, -1155,  1275, -1155,  1007,    18,
     -58,    22,   624, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
    1019,  1150,  1126, -1155,    32,  1441,  1466,  1014,  1021,  1022,
    1023,  1024, -1155, -1155,  1248, -1155,  1248, -1155,  1035, -1155,
   -1155, -1155, -1155, -1155,    33,   108,   252,   108, -1155,  1036,
   -1155,   252,   252,   252,  1132,   251,    28, -1155, -1155,  1039,
    1056,  1020,  1820,    39, -1155, -1155, -1155,   734, -1155, -1155,
    1346, -1155, -1155, -1155,   163, -1155, -1155,  1248,  1248,  1248,
    1248, -1155,  1034,  1058, -1155,  2118, -1155,   252,  1061, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155,   252,   252, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155,  1776,    50,    39,
     211,  1801,  1826,  1851,  1876,  1248, -1155,  1062,   252,   108,
     252,   252,   252,  1186,   252,   277,  1106,    41,  1125,   252,
    1121,  1122,  1115,  1144, -1155, -1155,  1086, -1155, -1155, -1155,
   -1155, -1155,  1248, -1155, -1155, -1155,  1087,  1088,  1089,  1091,
    1084, -1155, -1155, -1155, -1155, -1155, -1155,   252, -1155,   252,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155,   108, -1155,   108,   235,   252,   108, -1155,  1776,
   -1155, -1155, -1155, -1155, -1155,     6, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155,   -41, -1155, -1155, -1155, -1155,
    1092, -1155, -1155
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1155, -1155, -1155,  -152, -1155,   -91,  -233,  -802,  -124,  -634,
   -1155,  -464, -1155, -1155, -1155, -1155, -1155, -1155,   528, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155,  1298, -1155, -1155, -1155,
   -1155, -1155, -1155,  -256, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155,   946,   967,  1201, -1155,   926,  -417, -1155,
   -1155, -1155, -1155,   492,  -556, -1155, -1155, -1155, -1155,    21,
   -1155,   567,   -66, -1155,   566,  -989,   436,  -558, -1155,  -521,
   -1155, -1155, -1155, -1155,  -126, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155,  -331, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,   808,  1114,
   -1155, -1155, -1155, -1155,  -703,   563,   427,  1063,   840,  1140,
   -1155,   764, -1155, -1155,  -923, -1155,   230,  -187,   736,   185,
    -903,   556, -1154, -1037,  1297, -1092,   755,   196, -1155, -1155,
    -185,   749, -1155, -1155, -1155, -1155,   285,   383, -1155, -1155,
    -893, -1155,    20,   465,   -76, -1155, -1155,   319,   590,   392,
   -1020, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155,   357, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
     364, -1155, -1155, -1155,  -359, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155, -1155,   652, -1155,
   -1155, -1155, -1155, -1155, -1155, -1155, -1155,  -999
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -409
static const yytype_int16 yytable[] =
{
     199,   520,   521,   374,   423,   551,   429,   835,   348,   839,
     878,   352,   353,   913,  1226,   390,  1041,   403,  1115,   391,
    1123,  -272,   313,   666,  1385,   371,  1056,   314,   392,   372,
    -272,  -272,  -272,  -272,  1027,   367,   349,   350,  1253,   422,
     349,   350,  1125,   752,   753,  -271,   568,  1254,  1255,  1256,
     932,   401,   776,  1007,  -271,  -271,  -271,  -271,  1027,   516,
    1261,  1028,   541,    61,   488,  1254,  1255,  1256,   932,   371,
     543,     1,  1235,   372,   371,   419,   420,   174,   372,   465,
     466,   467,   468,   469,   906,  1028,   334,   526,   489,   490,
     491,   492,  1029,   470,   471,   472,   473,   474,   475,   476,
     477,  1184,   388,   478,   406,   371,   419,   420,   932,   372,
     326,   553,  1289,   117,   418,   327,  1029,   465,   466,   467,
     468,   469,  1262,   371,   822,  1359,   566,   372,   125,   572,
     573,   470,   471,   472,   473,   474,   475,   476,   477,   514,
     149,   478,   489,   490,   491,   492,  1261,  1041,   371,   419,
     420,   932,   372,  1213,   629,   517,   506,   537,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,   576,   548,   852,   583,    62,
     120,   552,  1210,   554,  1157,   370,   371,   375,  1037,   341,
     372,  1038,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1273,
     397,   709,   710,   711,   712,   489,   490,   491,   492,    63,
     542,  1236,  1037,  1248,    64,  1038,  1109,    65,  1262,   595,
     376,   597,   673,   335,   402,   646,   657,   658,    66,   661,
     663,   630,   907,   688,  1237,  1171,  1254,  1255,  1256,   932,
     167,   778,  1213,   163,   484,   544,    67,   154,  1390,    68,
     342,    69,    70,   155,   465,   466,   467,   468,   469,    71,
     371,   135,   621,   371,   372,   631,   672,   372,   470,   471,
     472,   473,   474,   475,   476,   477,   371,   371,   478,   162,
     372,   372,   632,  1158,    72,   164,  1238,    73,   777,   114,
    1041,   569,   570,   349,   350,   797,  1325,   136,  1251,    74,
    1391,   622,   371,   577,   578,  1176,   372,   687,   343,   882,
    1159,   341,   893,  1290,   895,  1177,  1291,  1065,    75,   168,
     115,  1085,  1086,  1088,  1217,  1276,  1360,   720,  1041,  1361,
     315,  1272,   660,   725,  1277,  1116,   667,   165,   368,   393,
    1296,   734,   735,   736,   737,   738,   394,  1227,   404,   373,
    1386,   525,   913,   169,  -272,   748,   633,   785,  1087,  -272,
     754,   785,  1231,   166,  1287,   855,  1240,   856,    76,  1041,
    1041,  1257,   144,   794,  1218,  1219,  1258,   798,  -271,  1099,
     802,   864,   804,  -271,   587,   200,  1334,   808,   809,  1332,
    1349,   203,   145,   373,   848,   114,   341,   146,   421,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,  1340,   942,  1174,   818,  1041,   662,   328,   713,
     397,   864,   397,   150,   397,   397,  1220,   849,   993,   373,
     942,  1221,   843,   844,   787,  1000,   796,  1001,   588,  1002,
    1142,  1003,   151,   318,   897,   819,   820,   373,   839,   904,
     810,   925,   860,   152,   861,   815,   926,   817,   321,   821,
     863,   823,   319,   344,   826,   827,   828,   850,   320,  1096,
     519,   589,   421,   345,   346,   465,   466,   467,   468,   469,
     590,   110,   111,   476,   477,   110,   111,   478,   910,   470,
     471,   472,   473,   474,   475,   476,   477,   330,   331,   478,
     373,   325,   112,   337,   853,  1299,   112,    77,  1300,   349,
     350,   465,   466,   467,   468,   469,   750,   862,   332,   751,
     874,   338,   496,   354,   896,   470,   471,   472,   473,   474,
     475,   476,   477,  1380,  1381,   478,  1145,   465,   466,   467,
     468,   469,   156,  1019,   892,  1021,   894,  1106,  1107,  1108,
    1257,   470,   471,   472,   473,   474,   475,   476,   477,   755,
     157,   478,   756,   591,   592,   497,   498,   965,   341,   499,
     911,   355,   423,   423,   373,   496,   742,   373,   361,   158,
     429,   743,   465,   466,   467,   468,   469,   121,   122,   883,
     373,   373,  1161,   349,   350,   854,   470,   471,   472,   473,
     474,   475,   476,   477,   364,   884,   478,   422,   422,   465,
     466,   467,   468,   469,   356,   770,   373,  1194,   497,   498,
     510,   511,   503,   470,   471,   472,   473,   474,   475,   476,
     477,   529,   530,   478,  1008,   159,   533,   534,   771,  1015,
     380,  1016,   381,   382,   772,  1018,   357,   945,   946,   773,
     774,  1201,  1202,  1203,  1204,   559,   560,   953,   358,   885,
     359,   957,   939,   360,   959,  1195,  1196,  1197,  1198,   964,
     330,   331,   213,   500,   363,   927,   365,   928,   465,   466,
     467,   468,   469,   689,   690,   718,   377,   719,   977,   721,
     722,   378,   470,   471,   472,   473,   474,   475,   476,   477,
     385,   214,   478,   387,   215,   398,   999,   399,   216,   217,
     947,   400,   219,   220,   949,   126,   127,   479,   864,   465,
     466,   467,   468,   469,   128,   407,   500,   277,  1068,  1069,
    1070,  1071,   480,   470,   471,   472,   473,   474,   475,   476,
     477,   408,   129,   478,   409,   130,   410,   131,   132,   221,
     485,   279,   867,   868,   222,   869,   870,   170,   411,   171,
     172,   173,   174,   175,   176,   465,   466,   467,   468,   469,
     177,   178,   744,   179,   412,   180,   349,   350,  1064,   470,
     471,   472,   473,   474,   475,   476,   477,   902,   903,   478,
     481,   886,  1166,  1167,   483,   204,   731,   732,   487,   505,
     205,   507,   917,   508,   170,   512,   171,   172,   173,   174,
     175,   176,   509,  1077,   206,   513,   515,   177,   178,   518,
     179,   181,   180,   522,   523,   118,   119,   182,   123,   124,
     524,   527,   528,   535,   532,   531,   148,   887,   920,   536,
     539,   864,   540,   546,   538,  1151,  1241,   207,   545,   137,
     547,   725,   888,   549,  1049,  1050,  1051,   550,  1052,   555,
    1053,   565,   558,   556,   921,  1060,   133,  1110,   181,   557,
    1092,   889,  1062,   607,   182,   561,   562,   183,   184,   567,
     574,   579,   138,   139,   575,   890,   891,   580,   581,   584,
     582,   586,   608,   609,   585,  1242,  1243,  1244,  1084,  1245,
    1089,   593,   649,   594,   610,   611,  1097,   612,   596,   922,
    1246,   185,   186,   413,   414,   415,   416,   598,   650,   140,
     141,   142,   143,   602,   183,   184,   187,   603,   599,   188,
     600,  1143,   615,   189,   616,   601,   924,   271,   272,   323,
     324,   617,   134,  1146,  1147,   605,  1149,   618,   606,   619,
     620,   625,   627,   628,  1154,   664,   665,  1155,   185,   186,
     413,   414,   415,   416,   669,   273,   670,  1119,  1247,   651,
     671,  1120,  1121,   187,   652,   653,   188,  1127,   674,   686,
     189,   691,   654,   715,   723,   208,   724,   716,  1136,   728,
     729,   730,   478,   739,   733,   170,   740,   171,   172,   173,
     174,   175,   176,   741,   749,  1054,  1148,   655,   177,   178,
     775,   179,   791,   180,   789,  1193,   792,   790,   793,   795,
     170,   799,   171,   172,   173,   174,   175,   176,   800,   805,
     807,   803,   806,   177,   178,   744,   179,  1259,   180,   202,
     209,   656,   811,   812,   814,   210,  1135,   212,   816,  1168,
     725,  1170,   226,   824,   260,   832,   270,   813,   275,   181,
     291,   211,   830,   825,   829,   182,   277,   833,   834,  1180,
     836,   837,  1060,   190,   259,  1232,   266,  1232,   274,   840,
     287,   191,   296,   841,   181,   307,   312,   842,  1172,   857,
     182,   871,   845,   846,  1280,   847,   872,   873,   875,  1283,
    1284,  1285,   876,  1288,   879,   898,   880,   899,   900,  1215,
    1216,   901,  1127,   905,   635,   183,   184,   908,   909,   918,
     190,  1259,   919,  1259,   929,   930,   936,   935,   191,   349,
     350,   931,   938,   940,   942,  1327,   943,   948,   954,   950,
     183,   184,   951,   952,  1329,  1330,   956,   958,   995,   185,
     186,   636,   955,   960,  1279,   961,  1281,  1004,   962,   963,
    1060,   966,   967,   968,   187,   969,  1342,   188,  1344,  1345,
    1346,   189,  1348,  1350,   185,   186,  1009,  1363,   970,  1027,
     971,   637,  1013,   972,   973,   974,   638,   975,  1014,   187,
     976,  1297,   188,   639,   640,   992,   189,   996,   997,   998,
    1005,  1301,  1302,  1303,  1304,  1376,  1028,  1377,  1006,  1017,
    1020,  1022,   641,   642,   643,   644,  1023,   645,  1024,  1045,
    1046,  1061,  1048,  1382,  1383,  1072,  1073,   170,  1343,   171,
     172,   173,   174,   175,   176,  1063,  1074,  1029,  1099,  1075,
     177,   178,  1076,   179,  1078,   180,   170,  1079,   171,   172,
     173,   174,   175,   176,  1080,  1081,  1098,  1091,  1103,   177,
     178,  1183,   179,  1104,   180,  1111,  1369,  1112,  1113,  1114,
    1117,   170,  1118,   171,   172,   173,   174,   175,   176,  1122,
    1139,  1378,  1141,  1379,   177,   178,  1384,   179,  1150,   180,
    1126,   181,  1137,  1175,  1182,  1186,  1138,   182,  1205,  1206,
    1228,  1252,   500,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
     181,   190,  -408,  -409,  -409,  -409,   182,  1144,  1152,   191,
    1153,  1156,  1208,  1037,  1162,  1163,  1038,  -409,  -409,  -409,
    -409,   474,   475,   476,   477,   181,   190,   478,  1178,  1164,
    1165,   182,  1179,  1225,   191,  1187,  1185,   183,   184,  1200,
    1229,  1224,  1286,  1267,   465,   466,   467,   468,   469,  1250,
    1268,  1269,  1270,  1298,  1294,  1271,   183,   184,   470,   471,
     472,   473,   474,   475,   476,   477,  1274,  1282,   478,  1305,
    1292,   185,   186,  1351,  1352,  1353,  1354,  1355,  1356,  1357,
    1358,   183,   184,   465,   466,   467,   187,  1293,  1306,   188,
     185,   186,  1328,   189,  1362,  1341,  1347,   470,   471,   472,
     473,   474,   475,   476,   477,   187,  1364,   478,   188,  1365,
    1366,  1367,   189,   317,  1374,   185,   186,  1368,  1370,  1371,
    1372,   504,  1373,  1392,   465,   466,   467,   468,   469,   788,
     187,  1025,  1102,   188,  1067,  1011,  1012,   189,   470,   471,
     472,   473,   474,   475,   476,   477,   626,   707,   478,   465,
     466,   467,   468,   469,   675,   676,   677,   678,   679,   680,
     681,   682,   683,   470,   471,   472,   473,   474,   475,   476,
     477,  1026,  1130,   478,   465,   466,   467,   468,   469,   648,
     877,   915,  1295,  1333,  -271,   933,  1058,   417,   470,   471,
     472,   473,   474,   475,   476,   477,  1335,  1222,   478,   465,
     466,   467,   468,   469,   937,   934,  1124,  1047,  1169,  1188,
    1191,  1010,     0,   470,   471,   472,   473,   474,   475,   476,
     477,     0,     0,   478,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   190,     0,  -403,     0,     0,     0,     0,
       0,   191,     0,     0,     0,     0,     0,   465,   466,   467,
     468,   469,   190,     0,     0,     0,     0,     0,     0,     0,
     191,   470,   471,   472,   473,   474,   475,   476,   477,     0,
       0,   478,   465,   466,   467,   468,   469,   190,     0,     0,
       0,     0,     0,     0,     0,   191,   470,   471,   472,   473,
     474,   475,   476,   477,     0,     0,   478,   465,   466,   467,
     468,   469,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   470,   471,   472,   473,   474,   475,   476,   477,     0,
       0,   478,   465,   466,   467,   468,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   470,   471,   472,   473,
     474,   475,   476,   477,     0,     0,   478,     0,     0,     0,
       0,     0,     0,     0,   465,   466,   467,   468,   469,     0,
       0,     0,     0,     0,     0,     0,     0,  1207,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,   465,
     466,   467,   468,   469,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   470,   471,   472,   473,   474,   475,   476,
     477,     0,     0,   478,   465,   466,   467,   468,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,     0,
       0,     0,     0,     0,   465,   466,   467,   468,   469,     0,
       0,     0,     0,     0,     0,     0,     0,  1209,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,   465,
     466,   467,   468,   469,     0,     0,     0,     0,     0,     0,
       0,     0,  1265,   470,   471,   472,   473,   474,   475,   476,
     477,     0,     0,   478,   465,   466,   467,   468,   469,     0,
       0,     0,     0,     0,     0,     0,     0,  1266,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,   465,
     466,   467,   468,   469,     0,     0,     0,     0,     0,     0,
       0,   916,     0,   470,   471,   472,   473,   474,   475,   476,
     477,     0,     0,   478,   465,   466,   467,   468,   469,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,   465,
     466,   467,   468,   469,     0,     0,  1028,     0,     0,   923,
       0,     0,     0,   470,   471,   472,   473,   474,   475,   476,
     477,     0,     0,   478,   465,   466,   467,   468,   469,     0,
       0,     0,     0,     0,  1055,     0,     0,  1029,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,     0,
     213,   432,   692,   693,   694,   695,   696,   697,   698,  1131,
     699,   700,   701,   702,   703,     0,     0,   432,     0,   299,
     300,   301,     0,   302,   303,     0,     0,     0,     0,   214,
       0,     0,   215,     0,  1132,     0,   216,   217,     0,     0,
     219,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
       0,     0,     0,   433,   434,     0,  1133,   435,   436,     0,
       0,     0,   213,  1037,     0,     0,  1038,   221,     0,   433,
     434,     0,   222,   435,   436,   213,     0,     0,     0,     0,
     437,  1134,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,     0,   215,     0,     0,     0,   216,   217,
       0,   276,   219,   220,   214,     0,  1140,   215,     0,     0,
       0,   216,   217,   438,   277,   219,   220,     0,     0,     0,
       0,     0,   692,   693,   694,   695,   696,   697,   698,   438,
     699,   700,   701,   702,   703,     0,  1181,   278,   279,   221,
       0,     0,     0,     0,   222,     0,     0,     0,     0,     0,
       0,     0,   221,     0,     0,   213,     0,   222,     0,   439,
     440,  1223,   441,   442,   443,   444,   445,     0,     0,     0,
       0,     0,   912,     0,     0,     0,   308,     0,     0,     0,
       0,     0,     0,     0,   214,     0,  1331,   215,     0,     0,
       0,   216,   217,     0,     0,   219,   220,     0,     0,   978,
     446,   447,   448,   449,   450,   451,   452,   453,   454,     0,
       0,  1336,     0,     0,     0,   309,   446,   447,   448,   449,
     450,   451,   452,   453,   454,     0,   455,   456,     0,     0,
     213,     0,   221,  1211,     0,   227,  1337,   222,   457,   458,
     459,   460,   455,   456,     0,     0,     0,     0,     0,   979,
       0,     0,     0,     0,   457,   458,   459,   460,     0,   214,
       0,  1338,   215,     0,   213,     0,   216,   217,     0,     0,
     219,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   228,   229,   304,   230,   261,  1339,     0,     0,     0,
     262,   277,   263,   214,     0,     0,   215,   280,     0,     0,
     216,   217,     0,     0,   219,   220,     0,   221,     0,     0,
     461,   462,   222,     0,     0,   279,     0,     0,     0,     0,
     231,     0,     0,     0,   213,   980,   461,   462,   264,     0,
     232,     0,   281,   282,   283,   284,  1307,     0,     0,     0,
       0,   221,   285,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,   214,   292,   286,   215,     0,     0,     0,
     216,   217,     0,   218,   219,   220,   233,     0,   265,     0,
       0,     0,     0,     0,     0,     0,   981,     0,     0,     0,
       0,     0,   234,   235,     0,   236,   237,     0,   238,   239,
     240,     0,   241,     0,   782,   242,   243,     0,   244,     0,
     982,   221,   983,     0,     0,     0,   222,     0,     0,   984,
    1308,   985,   245,   246,     0,   986,   987,   247,     0,   248,
     249,     0,   250,     0,     0,     0,     0,   251,     0,     0,
       0,   252,   253,     0,   254,   255,     0,   256,   257,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   310,     0,
       0,     0,     0,   988,     0,  1309,     0,  1310,     0,  1311,
    1312,  1313,  1314,  1315,  1316,   293,     0,     0,     0,     4,
       0,     0,     0,     0,  1317,  1318,     0,     0,  1319,     0,
       0,     0,  1320,     0,  1321,     0,     5,     6,     0,     0,
       0,     0,  1322,     0,   465,   466,   467,   468,   469,     0,
    1323,     0,     0,     0,     0,     0,     0,   294,   470,   471,
     472,   473,   474,   475,   476,   477,     0,     0,   478,     7,
       8,     0,     0,   258,     0,   692,   693,   694,   695,   696,
     697,   698,  1324,   699,   700,   701,   702,   703,     9,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,    11,     0,    12,     0,     0,    13,   295,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,    18,
     465,   466,   467,   468,   469,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   470,   471,   472,   473,   474,   475,
     476,   477,     0,     0,   478,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1173
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1155))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-409))

static const yytype_int16 yycheck[] =
{
      76,   234,   235,   155,   191,   261,   191,   563,   134,   567,
     644,   135,   136,   716,     7,    15,   909,    15,    22,    19,
    1040,     6,    35,    29,    18,    15,   929,    40,    28,    19,
      15,    16,    17,    18,    19,    40,    18,    19,     6,   191,
      18,    19,  1041,    26,    27,     6,    84,    15,    16,    17,
      18,    27,   108,   855,    15,    16,    17,    18,    19,   131,
    1214,    46,    70,     0,   216,    15,    16,    17,    18,    15,
      79,     4,   130,    19,    15,    16,    17,    18,    19,     8,
       9,    10,    11,    12,    19,    46,     3,   239,    34,    35,
      36,    37,    77,    22,    23,    24,    25,    26,    27,    28,
      29,  1138,   168,    32,   180,    15,    16,    17,    18,    19,
      35,   263,    84,     3,   190,    40,    77,     8,     9,    10,
      11,    12,  1214,    15,   541,    84,   278,    19,     3,   281,
     282,    22,    23,    24,    25,    26,    27,    28,    29,   230,
      19,    32,    34,    35,    36,    37,  1300,  1040,    15,    16,
      17,    18,    19,  1173,     3,   227,   222,   248,   143,   144,
     145,   146,   147,   148,   149,   194,   257,   584,   292,     3,
     108,   262,  1171,   264,   208,   154,    15,    20,   163,    19,
      19,   166,   143,   144,   145,   146,   147,   148,   149,  1226,
     170,   113,   114,   115,   116,    34,    35,    36,    37,    33,
     208,   259,   163,  1192,    38,   166,  1008,    41,  1300,   300,
      53,   302,   364,   130,   190,   339,   342,   343,    52,   345,
     346,    70,   157,   375,   282,  1118,    15,    16,    17,    18,
      20,   487,  1252,    74,   213,   244,    70,    99,   279,    73,
      80,    75,    76,   105,     8,     9,    10,    11,    12,    83,
      15,   108,    85,    15,    19,   104,    18,    19,    22,    23,
      24,    25,    26,    27,    28,    29,    15,    15,    32,    19,
      19,    19,   121,   307,   108,    19,   334,   111,   334,    20,
    1173,   319,   320,    18,    19,   518,  1275,   108,  1211,   123,
     331,   124,    15,   322,   323,    22,    19,   373,   138,   658,
     334,    19,   661,   275,   663,    32,   278,   941,   142,    99,
      51,   259,   260,   259,    83,   282,   275,   393,  1211,   278,
     333,  1224,    40,   399,   291,   329,   332,    70,   333,   329,
    1253,   407,   408,   409,   410,   411,   336,   330,   336,   329,
     334,   331,  1045,    19,   329,   421,   195,   499,   296,   334,
     333,   503,   334,   112,   103,   588,   334,   590,   192,  1252,
    1253,   329,    15,   515,   133,   134,   334,   519,   329,   301,
     522,   604,   524,   334,   148,    19,  1299,   529,   530,   329,
     103,   333,    35,   329,    74,    20,    19,    40,   329,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,  1305,   335,   333,   205,  1299,    40,   333,   331,
     390,   644,   392,    20,   394,   395,   185,   107,   333,   329,
     335,   190,   574,   575,   500,    94,   517,    96,   202,    94,
    1064,    96,    39,     3,   667,   235,   236,   329,   996,   330,
     531,   330,   594,    50,   596,   536,   335,   538,    70,   540,
     602,   542,     3,   293,   545,   546,   547,   147,     3,   298,
     195,   235,   329,   303,   304,     8,     9,    10,    11,    12,
     244,    19,    20,    28,    29,    19,    20,    32,   711,    22,
      23,    24,    25,    26,    27,    28,    29,    19,    20,    32,
     329,     3,    40,     3,   585,   332,    40,   331,   335,    18,
      19,     8,     9,    10,    11,    12,   335,   598,    40,   338,
     634,     3,    40,     3,   666,    22,    23,    24,    25,    26,
      27,    28,    29,   288,   289,    32,  1082,     8,     9,    10,
      11,    12,    38,   892,   660,   894,   662,  1001,  1002,  1003,
     329,    22,    23,    24,    25,    26,    27,    28,    29,   335,
      56,    32,   338,   327,   328,    83,    84,   813,    19,    87,
     712,     3,   749,   750,   329,    40,   330,   329,    15,    75,
     755,   335,     8,     9,    10,    11,    12,   121,   122,     3,
     329,   329,  1103,    18,    19,    20,    22,    23,    24,    25,
      26,    27,    28,    29,    22,    19,    32,   749,   750,     8,
       9,    10,    11,    12,     3,   152,   329,   267,    83,    84,
     211,   212,    87,    22,    23,    24,    25,    26,    27,    28,
      29,   215,   216,    32,   857,   131,   225,   226,   175,   885,
      15,   887,    17,    18,   181,   891,     3,   789,   790,   186,
     187,  1162,  1163,  1164,  1165,   308,   309,   799,     3,    73,
       3,   803,   776,     3,   806,   315,   316,   317,   318,   811,
      19,    20,    41,   191,     3,   741,    57,   743,     8,     9,
      10,    11,    12,    54,    55,   390,   331,   392,   830,   394,
     395,   119,    22,    23,    24,    25,    26,    27,    28,    29,
      19,    70,    32,   331,    73,    29,   848,   329,    77,    78,
     791,   190,    81,    82,   795,    99,   100,    94,   941,     8,
       9,    10,    11,    12,   108,   329,   191,    93,    89,    90,
      91,    92,   333,    22,    23,    24,    25,    26,    27,    28,
      29,   329,   126,    32,   329,   129,   329,   131,   132,   118,
      19,   117,   334,   335,   123,   334,   335,    13,   329,    15,
      16,    17,    18,    19,    20,     8,     9,    10,    11,    12,
      26,    27,    28,    29,   329,    31,    18,    19,    20,    22,
      23,    24,    25,    26,    27,    28,    29,   334,   335,    32,
     333,   205,  1113,  1114,   333,    60,   403,   404,    74,    20,
      65,   331,   335,   331,    13,   213,    15,    16,    17,    18,
      19,    20,   333,   955,    79,    20,    87,    26,    27,   115,
      29,    77,    31,    63,   198,     7,     8,    83,    10,    11,
      63,   196,   204,   225,   210,   208,    18,   251,   335,   229,
     234,  1064,   239,   247,   232,  1091,   212,   112,   208,    66,
     245,   917,   266,   331,   920,   921,   922,   333,   924,   331,
     926,    20,    70,   333,   335,   931,   250,  1009,    77,   333,
     984,   285,   938,    66,    83,   331,   333,   133,   134,   333,
      63,   331,    99,   100,    63,   299,   300,   331,   333,    70,
     333,   331,    85,    86,   285,   261,   262,   263,   979,   265,
     981,   331,     3,    70,    97,    98,   987,   100,    63,   335,
     276,   167,   168,   169,   170,   171,   172,    66,    19,   136,
     137,   138,   139,    63,   133,   134,   182,    20,   331,   185,
     331,  1073,     3,   189,     3,   331,   335,   306,   307,   121,
     122,    19,   326,  1085,  1086,   331,  1088,     3,   331,     3,
       3,     3,     3,     3,  1096,     3,     3,  1099,   167,   168,
     169,   170,   171,   172,   109,   334,   109,  1033,   334,    70,
       3,  1037,  1038,   182,    75,    76,   185,  1043,    20,   331,
     189,   331,    83,   331,    14,   250,    15,   329,  1054,    26,
     190,    27,    32,    19,    29,    13,    15,    15,    16,    17,
      18,    19,    20,   335,     7,   335,  1087,   108,    26,    27,
      19,    29,    46,    31,    63,  1157,   331,    63,   331,   237,
      13,   195,    15,    16,    17,    18,    19,    20,    30,   331,
     205,    63,    63,    26,    27,    28,    29,  1214,    31,    83,
     305,   142,    63,   331,   331,   310,   335,    91,   331,  1115,
    1116,  1117,    96,   331,    98,    19,   100,    74,   102,    77,
     104,   326,    87,   331,   331,    83,    93,   331,   331,  1135,
     331,   331,  1138,   329,    97,  1189,    99,  1191,   101,   331,
     103,   337,   105,   331,    77,   108,   109,   331,   331,   202,
      83,     3,   331,   331,  1236,   331,    19,    15,    19,  1241,
    1242,  1243,    19,  1245,    19,    15,    20,    15,   331,  1175,
    1176,   331,  1178,    20,     3,   133,   134,   331,   333,   330,
     329,  1298,    27,  1300,     7,   335,    18,    15,   337,    18,
      19,   335,   329,    27,   335,  1277,    88,   331,    19,   331,
     133,   134,   331,   331,  1286,  1287,   331,   331,   334,   167,
     168,    40,   195,   331,  1235,   331,  1237,   334,   331,   331,
    1226,   331,   331,   331,   182,   331,  1308,   185,  1310,  1311,
    1312,   189,  1314,  1315,   167,   168,   295,  1319,   331,    19,
     331,    70,     3,   331,   331,   331,    75,   331,     3,   182,
     331,  1257,   185,    82,    83,   331,   189,   331,   331,   331,
     331,  1267,  1268,  1269,  1270,  1347,    46,  1349,   331,    15,
       3,     3,   101,   102,   103,   104,   195,   106,   195,   331,
     330,   335,    27,  1365,  1366,   331,   335,    13,  1309,    15,
      16,    17,    18,    19,    20,   109,   331,    77,   301,   331,
      26,    27,   331,    29,   331,    31,    13,   331,    15,    16,
      17,    18,    19,    20,   331,   331,   331,   285,   331,    26,
      27,    28,    29,    20,    31,     3,  1332,     3,    15,    15,
      22,    13,   329,    15,    16,    17,    18,    19,    20,   334,
      15,  1362,    15,  1364,    26,    27,  1367,    29,   333,    31,
     331,    77,   330,    22,     7,   195,   335,    83,     3,     3,
      15,   165,   191,   143,   144,   145,   146,   147,   148,   149,
      77,   329,   330,     8,     9,    10,    83,   331,   333,   337,
     333,   333,   330,   163,   331,   331,   166,    22,    23,    24,
      25,    26,    27,    28,    29,    77,   329,    32,   335,   331,
     331,    83,   331,   330,   337,   331,   334,   133,   134,   331,
     333,   329,   210,   329,     8,     9,    10,    11,    12,   330,
     329,   329,   329,     7,   334,   331,   133,   134,    22,    23,
      24,    25,    26,    27,    28,    29,   331,   331,    32,   335,
     331,   167,   168,   267,   268,   269,   270,   271,   272,   273,
     274,   133,   134,     8,     9,    10,   182,   331,   330,   185,
     167,   168,   331,   189,   269,   333,   210,    22,    23,    24,
      25,    26,    27,    28,    29,   182,   285,    32,   185,   287,
     295,   267,   189,   115,   330,   167,   168,   331,   331,   331,
     331,   220,   331,   331,     8,     9,    10,    11,    12,   503,
     182,   903,   996,   185,   942,   868,   870,   189,    22,    23,
      24,    25,    26,    27,    28,    29,   332,   384,    32,     8,
       9,    10,    11,    12,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    22,    23,    24,    25,    26,    27,    28,
      29,   908,  1045,    32,     8,     9,    10,    11,    12,   339,
     640,   717,  1252,  1298,   334,   749,   930,   190,    22,    23,
      24,    25,    26,    27,    28,    29,  1300,  1178,    32,     8,
       9,    10,    11,    12,   755,   750,  1041,   917,  1116,  1145,
    1153,   859,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   329,    -1,   331,    -1,    -1,    -1,    -1,
      -1,   337,    -1,    -1,    -1,    -1,    -1,     8,     9,    10,
      11,    12,   329,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     337,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,     8,     9,    10,    11,    12,   329,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   337,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   331,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   331,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   331,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   331,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   330,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,     8,
       9,    10,    11,    12,    -1,    -1,    46,    -1,    -1,   330,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,   330,    -1,    -1,    77,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    -1,
      41,     5,   150,   151,   152,   153,   154,   155,   156,   330,
     158,   159,   160,   161,   162,    -1,    -1,     5,    -1,    60,
      61,    62,    -1,    64,    65,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    73,    -1,   330,    -1,    77,    78,    -1,    -1,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    67,    68,    -1,   330,    71,    72,    -1,
      -1,    -1,    41,   163,    -1,    -1,   166,   118,    -1,    67,
      68,    -1,   123,    71,    72,    41,    -1,    -1,    -1,    -1,
      94,   330,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,    78,
      -1,    80,    81,    82,    70,    -1,   330,    73,    -1,    -1,
      -1,    77,    78,   127,    93,    81,    82,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   127,
     158,   159,   160,   161,   162,    -1,   330,   116,   117,   118,
      -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,    -1,    -1,    41,    -1,   123,    -1,   173,
     174,   330,   176,   177,   178,   179,   180,    -1,    -1,    -1,
      -1,    -1,   330,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,   330,    73,    -1,    -1,
      -1,    77,    78,    -1,    -1,    81,    82,    -1,    -1,    80,
     214,   215,   216,   217,   218,   219,   220,   221,   222,    -1,
      -1,   330,    -1,    -1,    -1,   101,   214,   215,   216,   217,
     218,   219,   220,   221,   222,    -1,   240,   241,    -1,    -1,
      41,    -1,   118,   333,    -1,    46,   330,   123,   252,   253,
     254,   255,   240,   241,    -1,    -1,    -1,    -1,    -1,   130,
      -1,    -1,    -1,    -1,   252,   253,   254,   255,    -1,    70,
      -1,   330,    73,    -1,    41,    -1,    77,    78,    -1,    -1,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,   334,    95,   251,   330,    -1,    -1,    -1,
     256,    93,   258,    70,    -1,    -1,    73,   276,    -1,    -1,
      77,    78,    -1,    -1,    81,    82,    -1,   118,    -1,    -1,
     324,   325,   123,    -1,    -1,   117,    -1,    -1,    -1,    -1,
     131,    -1,    -1,    -1,    41,   206,   324,   325,   294,    -1,
     141,    -1,   311,   312,   313,   314,   138,    -1,    -1,    -1,
      -1,   118,   321,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    70,   131,   334,    73,    -1,    -1,    -1,
      77,    78,    -1,    80,    81,    82,   177,    -1,   334,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   257,    -1,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
     201,    -1,   203,    -1,    19,   206,   207,    -1,   209,    -1,
     281,   118,   283,    -1,    -1,    -1,   123,    -1,    -1,   290,
     212,   292,   223,   224,    -1,   296,   297,   228,    -1,   230,
     231,    -1,   233,    -1,    -1,    -1,    -1,   238,    -1,    -1,
      -1,   242,   243,    -1,   245,   246,    -1,   248,   249,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   334,    -1,
      -1,    -1,    -1,   334,    -1,   257,    -1,   259,    -1,   261,
     262,   263,   264,   265,   266,   242,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    -1,   276,   277,    -1,    -1,   280,    -1,
      -1,    -1,   284,    -1,   286,    -1,    58,    59,    -1,    -1,
      -1,    -1,   294,    -1,     8,     9,    10,    11,    12,    -1,
     302,    -1,    -1,    -1,    -1,    -1,    -1,   284,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    91,
      92,    -1,    -1,   334,    -1,   150,   151,   152,   153,   154,
     155,   156,   334,   158,   159,   160,   161,   162,   110,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,   123,    -1,   125,    -1,    -1,   128,   334,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,   140,   141,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   334,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   340,   341,    41,    58,    59,    91,    92,   110,
     120,   123,   125,   128,   133,   134,   135,   140,   141,   362,
     363,   364,   409,   410,   411,   412,   414,   415,   416,   419,
     422,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   534,   535,   536,   537,   538,   539,   540,   541,   560,
     562,     0,     3,    33,    38,    41,    52,    70,    73,    75,
      76,    83,   108,   111,   123,   142,   192,   331,   343,   352,
     358,   361,   366,   368,   371,   373,   377,   378,   380,   381,
     388,   394,   395,   449,   451,   461,   500,   501,   502,   503,
     543,   544,   545,   546,   554,   555,   561,   563,   564,   565,
      19,    20,    40,   447,    20,    51,   365,     3,   447,   447,
     108,   121,   122,   447,   447,     3,    99,   100,   108,   126,
     129,   131,   132,   250,   326,   108,   108,    66,    99,   100,
     136,   137,   138,   139,    15,    35,    40,   436,   447,    19,
      20,    39,    50,   354,    99,   105,    38,    56,    75,   131,
     359,   360,    19,    74,    19,    70,   112,    20,    99,    19,
      13,    15,    16,    17,    18,    19,    20,    26,    27,    29,
      31,    77,    83,   133,   134,   167,   168,   182,   185,   189,
     329,   337,   466,   479,   484,   487,   488,   489,   491,   493,
      19,   382,   382,   333,    60,    65,    79,   112,   250,   305,
     310,   326,   382,    41,    70,    73,    77,    78,    80,    81,
      82,   118,   123,   334,   383,   397,   382,    46,    92,    93,
      95,   131,   141,   177,   193,   194,   196,   197,   199,   200,
     201,   203,   206,   207,   209,   223,   224,   228,   230,   231,
     233,   238,   242,   243,   245,   246,   248,   249,   334,   383,
     382,   251,   256,   258,   294,   334,   383,   504,   505,   506,
     382,   306,   307,   334,   383,   382,    80,    93,   116,   117,
     276,   311,   312,   313,   314,   321,   334,   383,   404,   406,
     407,   382,   131,   242,   284,   334,   383,   556,   559,    60,
      61,    62,    64,    65,   334,   367,   370,   383,    62,   101,
     334,   369,   383,    35,    40,   333,   398,   365,     3,     3,
       3,    70,   424,   447,   447,     3,    35,    40,   333,   401,
      19,    20,    40,   448,     3,   130,   420,     3,     3,   417,
     423,    19,    80,   138,   293,   303,   304,   413,   413,    18,
      19,   345,   347,   347,     3,     3,     3,     3,     3,     3,
       3,    15,   437,     3,    22,    57,   353,    40,   333,   355,
     398,    15,    19,   329,   342,    20,    53,   331,   119,   376,
      15,    17,    18,   372,   450,    19,   374,   331,   401,   462,
      15,    19,    28,   329,   336,   483,   485,   491,    29,   329,
     190,    27,   190,    15,   336,   486,   493,   329,   329,   329,
     329,   329,   329,   169,   170,   171,   172,   473,   493,    16,
      17,   329,   342,   466,   467,   471,   474,   475,   477,   479,
     480,   481,     5,    67,    68,    71,    72,    94,   127,   173,
     174,   176,   177,   178,   179,   180,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   240,   241,   252,   253,   254,
     255,   324,   325,   492,   566,     8,     9,    10,    11,    12,
      22,    23,    24,    25,    26,    27,    28,    29,    32,    94,
     333,   333,   379,   333,   398,    19,   387,    74,   342,    34,
      35,    36,    37,   342,   344,   396,    40,    83,    84,    87,
     191,   384,   458,    87,   384,    20,   401,   331,   331,   333,
     211,   212,   213,    20,   344,    87,   131,   227,   115,   195,
     345,   345,    63,   198,    63,   331,   342,   196,   204,   215,
     216,   208,   210,   225,   226,   225,   229,   344,   232,   234,
     239,    70,   208,    79,   244,   208,   247,   245,   344,   331,
     333,   372,   344,   342,   344,   331,   333,   333,    70,   308,
     309,   331,   333,   552,   553,    20,   342,   333,    84,   319,
     320,   547,   342,   342,    63,    63,   194,   322,   323,   331,
     331,   333,   333,   347,    70,   285,   331,   148,   202,   235,
     244,   327,   328,   331,    70,   344,    63,   344,    66,   331,
     331,   331,    63,    20,   351,   331,   331,    66,    85,    86,
      97,    98,   100,   399,   400,     3,     3,    19,     3,     3,
       3,    85,   124,   402,   403,     3,   448,     3,     3,     3,
      70,   104,   121,   195,   421,     3,    40,    70,    75,    82,
      83,   101,   102,   103,   104,   106,   347,   418,   458,     3,
      19,    70,    75,    76,    83,   108,   142,   413,   413,   542,
      40,   413,    40,   413,     3,     3,    29,   332,   349,   109,
     109,     3,    18,   342,    20,    41,    42,    43,    44,    45,
      46,    47,    48,    49,   356,   357,   331,   493,   342,    54,
      55,   331,   150,   151,   152,   153,   154,   155,   156,   158,
     159,   160,   161,   162,   452,   453,   454,   456,   460,   113,
     114,   115,   116,   331,   375,   331,   329,   459,   485,   485,
     493,   485,   485,    14,    15,   493,   497,   498,    26,   190,
      27,   486,   486,    29,   493,   493,   493,   493,   493,    19,
      15,   335,   330,   335,    28,   469,   470,   473,   493,     7,
     335,   338,    26,    27,   333,   335,   338,   493,   493,   493,
     493,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     152,   175,   181,   186,   187,    19,   108,   334,   372,   389,
     392,   393,    19,   456,   457,   342,   386,   493,   386,    63,
      63,    46,   331,   331,   342,   237,   344,   345,   342,   195,
      30,   346,   342,    63,   342,   331,    63,   205,   342,   342,
     344,    63,   331,    74,   331,   344,   331,   344,   205,   235,
     236,   344,   387,   344,   331,   331,   344,   344,   344,   331,
      87,   507,    19,   331,   331,   393,   331,   331,   405,   406,
     331,   331,   331,   342,   342,   331,   331,   331,    74,   107,
     147,   408,   387,   344,    20,   345,   345,   202,   557,   558,
     342,   342,   344,   342,   345,   347,   348,   334,   335,   334,
     335,     3,    19,    15,   347,    19,    19,   457,   348,    19,
      20,   533,   533,     3,    19,    73,   205,   251,   266,   285,
     299,   300,   413,   533,   413,   533,   342,   345,    15,    15,
     331,   331,   334,   335,   330,    20,    19,   157,   331,   333,
     345,   342,   330,   453,   455,   460,   330,   335,   330,    27,
     335,   335,   335,   330,   335,   330,   335,   493,   493,     7,
     335,   335,    18,   467,   475,    15,    18,   480,   329,   347,
      27,   391,   335,    88,   385,   342,   342,   344,   331,   344,
     331,   331,   331,   342,    19,   195,   331,   342,   331,   342,
     331,   331,   331,   331,   342,   372,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   342,    80,   130,
     206,   257,   281,   283,   290,   292,   296,   297,   334,   508,
     515,   518,   331,   333,   551,   334,   331,   331,   331,   342,
      94,    96,    94,    96,   334,   331,   331,   346,   345,   295,
     557,   400,   403,     3,     3,   372,   372,    15,   372,   533,
       3,   533,     3,   195,   195,   357,   454,    19,    46,    77,
     143,   144,   145,   146,   147,   148,   149,   163,   166,   463,
     464,   489,   490,   494,   499,   331,   330,   497,    27,   493,
     493,   493,   493,   493,   335,   330,   469,   493,   470,   472,
     493,   335,   493,   109,    20,   348,   390,   392,    89,    90,
      91,    92,   331,   335,   331,   331,   331,   342,   331,   331,
     331,   331,   528,   532,   344,   259,   260,   296,   259,   344,
     509,   285,   347,   511,   512,   510,   298,   344,   331,   301,
     529,   550,   405,   331,    20,   350,   350,   350,   350,   346,
     342,     3,     3,    15,    15,    22,   329,    22,   329,   493,
     493,   493,   334,   499,   492,   566,   331,   493,   495,   496,
     455,   330,   330,   330,   330,   335,   493,   330,   335,    15,
     330,    15,   348,   342,   331,   393,   342,   342,   344,   342,
     333,   372,   333,   333,   342,   342,   333,   208,   307,   334,
     549,   408,   331,   331,   331,   331,   424,   424,   493,   498,
     493,   489,   331,   164,   333,    22,    22,    32,   335,   331,
     493,   330,     7,    28,   472,   334,   195,   331,   529,   520,
     513,   520,   516,   342,   267,   315,   316,   317,   318,   548,
     331,   408,   408,   408,   408,     3,     3,   331,   330,   331,
     566,   333,   465,   499,   482,   493,   493,    83,   133,   134,
     185,   190,   496,   330,   329,   330,     7,   330,    15,   333,
     531,   334,   347,   521,   522,   130,   259,   282,   334,   514,
     334,   212,   261,   262,   263,   265,   276,   334,   404,   517,
     330,   463,   165,     6,    15,    16,    17,   329,   334,   466,
     468,   471,   474,   476,   478,   331,   331,   329,   329,   329,
     329,   331,   469,   472,   331,   530,   282,   291,   523,   344,
     342,   344,   331,   342,   342,   342,   210,   103,   342,    84,
     275,   278,   331,   331,   334,   465,   463,   493,     7,   332,
     335,   493,   493,   493,   493,   335,   330,   138,   212,   257,
     259,   261,   262,   263,   264,   265,   266,   276,   277,   280,
     284,   286,   294,   302,   334,   404,   519,   342,   331,   342,
     342,   330,   329,   468,   463,   476,   330,   330,   330,   330,
     469,   333,   342,   344,   342,   342,   342,   210,   342,   103,
     342,   267,   268,   269,   270,   271,   272,   273,   274,    84,
     275,   278,   269,   342,   285,   287,   295,   267,   331,   493,
     331,   331,   331,   331,   330,   524,   342,   342,   344,   344,
     288,   289,   342,   342,   344,    18,   334,   525,   526,   527,
     279,   331,   331
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 643 "cf-parse.y"
    { return 0; }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 644 "cf-parse.y"
    { return 0; }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 657 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 658 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 664 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 668 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 677 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 678 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 679 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 680 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 681 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 682 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 689 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 696 "cf-parse.y"
    { (yyval.iface) = NULL; }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 697 "cf-parse.y"
    { (yyval.iface) = if_get_by_name((yyvsp[(2) - (2)].s)->name); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 701 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 709 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 713 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 717 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 724 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 732 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 733 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 739 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 748 "cf-parse.y"
    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 749 "cf-parse.y"
    { (yyval.t) = bird_name; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 753 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 758 "cf-parse.y"
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[(2) - (2)].t); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 759 "cf-parse.y"
    { (yyval.g) = stderr; }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 763 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 764 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 768 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 769 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 773 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 774 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 775 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 776 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 777 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 778 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 779 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 780 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 781 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 787 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 788 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 797 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 798 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 799 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 800 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 803 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 804 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 805 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 806 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 818 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 821 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 824 "cf-parse.y"
    { cmd_shutdown(); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 827 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 836 "cf-parse.y"
    { this_proto = krt_init_config((yyvsp[(1) - (2)].i)); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 841 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 842 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 846 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[(2) - (2)].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[(2) - (2)].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 853 "cf-parse.y"
    { THIS_KRT->devroutes = (yyvsp[(3) - (3)].i); }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 859 "cf-parse.y"
    { this_proto = kif_init_config((yyvsp[(1) - (2)].i)); }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 864 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 868 "cf-parse.y"
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[(2) - (3)].t);
     kpi->prefix = (yyvsp[(3) - (3)].px).addr;
     kpi->pxlen = (yyvsp[(3) - (3)].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 882 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->sys.table_id = (yyvsp[(3) - (3)].i);
   }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 895 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 901 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 903 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[(1) - (1)].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 922 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 923 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 924 "cf-parse.y"
    { new_config->listen_bgp_flags = 0; }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 925 "cf-parse.y"
    { new_config->listen_bgp_flags = 1; }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 932 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 933 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 937 "cf-parse.y"
    {
   struct rtable_config *cf;
   cf = rt_new_table((yyvsp[(2) - (3)].s));
   cf->sorted = (yyvsp[(3) - (3)].i);
   }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 945 "cf-parse.y"
    {
  this_roa_table = roa_new_table_config((yyvsp[(3) - (3)].s));
}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 951 "cf-parse.y"
    {
     roa_add_item_config(this_roa_table, (yyvsp[(3) - (8)].px).addr, (yyvsp[(3) - (8)].px).len, (yyvsp[(5) - (8)].i), (yyvsp[(7) - (8)].i));
   }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 965 "cf-parse.y"
    { (yyval.i) = SYM_PROTO; }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 966 "cf-parse.y"
    { (yyval.i) = SYM_TEMPLATE; }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 970 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = this_proto->class;
     s->def = this_proto;
     this_proto->name = s->name;
     }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 976 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 980 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].s)->class != SYM_TEMPLATE) && ((yyvsp[(3) - (3)].s)->class != SYM_PROTO)) cf_error("Template or protocol name expected");

     cf_define_symbol((yyvsp[(1) - (3)].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[(1) - (3)].s)->name;

     proto_copy_config(this_proto, (yyvsp[(3) - (3)].s)->def);
   }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 992 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 996 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 997 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 998 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 999 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1000 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1001 "cf-parse.y"
    { this_proto->in_limit = (yyvsp[(3) - (3)].g); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1002 "cf-parse.y"
    { this_proto->out_limit = (yyvsp[(3) - (3)].g); }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1003 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1004 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1005 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1009 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1011 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1012 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1016 "cf-parse.y"
    { (yyval.i) = PLA_DISABLE; }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1017 "cf-parse.y"
    { (yyval.i) = PLA_WARN; }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1018 "cf-parse.y"
    { (yyval.i) = PLA_BLOCK; }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1019 "cf-parse.y"
    { (yyval.i) = PLA_RESTART; }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1020 "cf-parse.y"
    { (yyval.i) = PLA_DISABLE; }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1024 "cf-parse.y"
    {
     struct proto_limit *l = cfg_allocz(sizeof(struct proto_limit));
     l->limit = (yyvsp[(1) - (2)].i);
     l->action = (yyvsp[(2) - (2)].i);
     (yyval.g) = l;
   }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1033 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1041 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1042 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1050 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1058 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1059 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1060 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1064 "cf-parse.y"
    { this_ipn->positive = 1; }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1065 "cf-parse.y"
    { this_ipn->positive = 0; }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1082 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_device, sizeof(struct rt_dev_config), (yyvsp[(1) - (2)].i));
     init_list(&DIRECT_CFG->iface_list);
   }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1095 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&DIRECT_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1109 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1110 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1111 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1116 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1120 "cf-parse.y"
    { (yyval.i) = D_STATES; }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1121 "cf-parse.y"
    { (yyval.i) = D_ROUTES; }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1122 "cf-parse.y"
    { (yyval.i) = D_FILTERS; }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1123 "cf-parse.y"
    { (yyval.i) = D_IFACES; }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1124 "cf-parse.y"
    { (yyval.i) = D_EVENTS; }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1125 "cf-parse.y"
    { (yyval.i) = D_PACKETS; }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1131 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1132 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1133 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1138 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1142 "cf-parse.y"
    { (yyval.i) = MD_STATES; }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1143 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1164 "cf-parse.y"
    {
     if (!this_p_list) {
     	this_p_list = cfg_alloc(sizeof(list));
     	init_list(this_p_list);
        password_id = 1;
     }
     this_p_item = cfg_alloc(sizeof (struct password_item));
     this_p_item->password = (yyvsp[(2) - (2)].t);
     this_p_item->genfrom = 0;
     this_p_item->gento = TIME_INFINITY;
     this_p_item->accfrom = 0;
     this_p_item->accto = TIME_INFINITY;
     this_p_item->id = password_id++;
     add_tail(this_p_list, &this_p_item->n);
   }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1182 "cf-parse.y"
    { }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1183 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 1184 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1185 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1186 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1187 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1196 "cf-parse.y"
    { cmd_show_status(); }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1199 "cf-parse.y"
    { cmd_show_memory(); }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1202 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1205 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1209 "cf-parse.y"
    { (yyval.s) = NULL; }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1213 "cf-parse.y"
    { if_show(); }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1216 "cf-parse.y"
    { if_show_summary(); }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1220 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1223 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1229 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1235 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1242 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1247 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1252 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1257 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1261 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1265 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->export_mode) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->export_mode = (yyvsp[(2) - (3)].i);
     (yyval.ra)->primary_only = 1;
     (yyval.ra)->export_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1275 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1283 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1287 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1294 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1295 "cf-parse.y"
    { (yyval.i) = 2; }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1301 "cf-parse.y"
    { roa_show((yyvsp[(3) - (4)].ro)); }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1304 "cf-parse.y"
    {
     (yyval.ro) = cfg_allocz(sizeof(struct roa_show_data));
     (yyval.ro)->mode = ROA_SHOW_ALL;
     (yyval.ro)->table = roa_table_default;
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
   }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1311 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     if ((yyval.ro)->mode != ROA_SHOW_ALL) cf_error("Only one prefix expected");
     (yyval.ro)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ro)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ro)->mode = (yyvsp[(2) - (3)].i);
   }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1318 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     (yyval.ro)->asn = (yyvsp[(3) - (3)].i);
   }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1322 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     if ((yyvsp[(3) - (3)].s)->class != SYM_ROA) cf_error("%s is not a ROA table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ro)->table = ((struct roa_table_config *)(yyvsp[(3) - (3)].s)->def)->table;
   }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1330 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_PX; }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1331 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_IN; }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1332 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_FOR; }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1338 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].sd)); }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1341 "cf-parse.y"
    {
     (yyval.sd) = cfg_allocz(sizeof(struct sym_show_data));
   }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1344 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_TABLE; }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1345 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_FUNCTION; }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1346 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_FILTER; }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1347 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_PROTO; }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1348 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_TEMPLATE; }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1349 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_ROA; }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1350 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->sym = (yyvsp[(2) - (2)].s); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1355 "cf-parse.y"
    { 
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
     (yyval.rot) = roa_table_default;
   }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1360 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].s)->class != SYM_ROA)
       cf_error("%s is not a ROA table", (yyvsp[(2) - (2)].s)->name);
     (yyval.rot) = ((struct roa_table_config *)(yyvsp[(2) - (2)].s)->def)->table;
   }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1369 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_add_item((yyvsp[(8) - (9)].rot), (yyvsp[(3) - (9)].px).addr, (yyvsp[(3) - (9)].px).len, (yyvsp[(5) - (9)].i), (yyvsp[(7) - (9)].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1376 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_delete_item((yyvsp[(8) - (9)].rot), (yyvsp[(3) - (9)].px).addr, (yyvsp[(3) - (9)].px).len, (yyvsp[(5) - (9)].i), (yyvsp[(7) - (9)].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1383 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_flush((yyvsp[(3) - (4)].rot), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1391 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1393 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1395 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1397 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); }
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1399 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1401 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1403 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1405 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1411 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1412 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1417 "cf-parse.y"
    { (yyval.i) = 4096; }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1418 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1425 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1427 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1429 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1431 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1433 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1435 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1439 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1443 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1446 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1449 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1450 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1451 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1455 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1456 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1457 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1465 "cf-parse.y"
    { (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[(2) - (2)].s) ); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1466 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s)->def = (yyvsp[(4) - (4)].f);
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1475 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1479 "cf-parse.y"
    { (yyval.i) = T_INT; }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1480 "cf-parse.y"
    { (yyval.i) = T_BOOL; }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1481 "cf-parse.y"
    { (yyval.i) = T_IP; }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1482 "cf-parse.y"
    { (yyval.i) = T_PREFIX; }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1483 "cf-parse.y"
    { (yyval.i) = T_PAIR; }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1484 "cf-parse.y"
    { (yyval.i) = T_QUAD; }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1485 "cf-parse.y"
    { (yyval.i) = T_EC; }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1486 "cf-parse.y"
    { (yyval.i) = T_STRING; }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1487 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1488 "cf-parse.y"
    { (yyval.i) = T_PATH; }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1489 "cf-parse.y"
    { (yyval.i) = T_CLIST; }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1490 "cf-parse.y"
    { (yyval.i) = T_ECLIST; }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1491 "cf-parse.y"
    { 
	switch ((yyvsp[(1) - (2)].i)) {
	  case T_INT:
	  case T_PAIR:
	  case T_QUAD:
	  case T_EC:
	  case T_IP:
	       (yyval.i) = T_SET;
	       break;

	  case T_PREFIX:
	       (yyval.i) = T_PREFIX_SET;
	    break;

	  default:
		cf_error( "You can't create sets of this type." );
	}
   }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1512 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1523 "cf-parse.y"
    { (yyval.s) = NULL; }
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1524 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   }
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1531 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1532 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   }
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1539 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   }
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1548 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1556 "cf-parse.y"
    {
     /* Construct 'IF term THEN ACCEPT; REJECT;' */
     struct filter *f = cfg_alloc(sizeof(struct filter));
     struct f_inst *i, *acc, *rej;
     acc = f_new_inst();		/* ACCEPT */
     acc->code = P('p',',');
     acc->a1.p = NULL;
     acc->a2.i = F_ACCEPT;
     rej = f_new_inst();		/* REJECT */
     rej->code = P('p',',');
     rej->a1.p = NULL;
     rej->a2.i = F_REJECT;
     i = f_new_inst();			/* IF */
     i->code = '?';
     i->a1.p = (yyvsp[(2) - (2)].x);
     i->a2.p = acc;
     i->next = rej;
     f->name = NULL;
     f->root = i;
     (yyval.f) = f;
  }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1580 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1581 "cf-parse.y"
    { (yyval.s)=NULL; }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1585 "cf-parse.y"
    {
     if ((yyvsp[(1) - (4)].s)) {
       /* Prepend instruction to clear local variables */
       (yyval.x) = f_new_inst();
       (yyval.x)->code = P('c','v');
       (yyval.x)->a1.p = (yyvsp[(1) - (4)].s);
       (yyval.x)->next = (yyvsp[(3) - (4)].x);
     } else
       (yyval.x) = (yyvsp[(3) - (4)].x);
   }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1598 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1601 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   }
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1614 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1615 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1618 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1619 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); }
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1623 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1626 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1635 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); }
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1648 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); }
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1649 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1650 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1651 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); }
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1655 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); }
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1656 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = f_eval_int((yyvsp[(2) - (3)].x)); }
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1657 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1658 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1659 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); }
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1663 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(1) - (1)].x)); check_u16((yyval.i)); }
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1666 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (1)].i), (yyvsp[(1) - (1)].i)); }
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1667 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i)); }
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1668 "cf-parse.y"
    { (yyval.i32) = 0xFFFF; }
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1672 "cf-parse.y"
    {
     (yyval.e) = f_new_pair_set(pair_a((yyvsp[(2) - (5)].i32)), pair_b((yyvsp[(2) - (5)].i32)), pair_a((yyvsp[(4) - (5)].i32)), pair_b((yyvsp[(4) - (5)].i32)));
   }
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1675 "cf-parse.y"
    {
     /* Hack: $2 and $4 should be pair_expr, but that would cause shift/reduce conflict */
     if ((pair_a((yyvsp[(2) - (11)].i32)) != pair_b((yyvsp[(2) - (11)].i32))) || (pair_a((yyvsp[(4) - (11)].i32)) != pair_b((yyvsp[(4) - (11)].i32))))
       cf_error("syntax error");
     (yyval.e) = f_new_pair_item(pair_b((yyvsp[(2) - (11)].i32)), (yyvsp[(8) - (11)].i), pair_b((yyvsp[(4) - (11)].i32)), (yyvsp[(10) - (11)].i)); 
   }
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1684 "cf-parse.y"
    { (yyval.i32) = f_eval_int((yyvsp[(1) - (1)].x)); }
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1687 "cf-parse.y"
    { (yyval.i) = EC_RT; }
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1688 "cf-parse.y"
    { (yyval.i) = EC_RO; }
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1689 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (2)].i); }
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1690 "cf-parse.y"
    { (yyval.i) = EC_GENERIC; }
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1694 "cf-parse.y"
    { (yyval.e) = f_new_ec_item((yyvsp[(2) - (7)].i), 0, (yyvsp[(4) - (7)].i32), (yyvsp[(6) - (7)].i32), (yyvsp[(6) - (7)].i32)); }
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1695 "cf-parse.y"
    { (yyval.e) = f_new_ec_item((yyvsp[(2) - (9)].i), 0, (yyvsp[(4) - (9)].i32), (yyvsp[(6) - (9)].i32), (yyvsp[(8) - (9)].i32)); }
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1696 "cf-parse.y"
    {  (yyval.e) = f_new_ec_item((yyvsp[(2) - (7)].i), 0, (yyvsp[(4) - (7)].i32), 0, EC_ALL); }
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1702 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); }
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1703 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); }
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1709 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1710 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); }
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1715 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); }
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1720 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); }
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1724 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   }
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1731 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1732 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; }
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1733 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; }
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1734 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   }
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1741 "cf-parse.y"
    { (yyval.trie) = f_new_trie(cfg_mem); trie_add_fprefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); }
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1742 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_fprefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); }
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1745 "cf-parse.y"
    { (yyval.e) = NULL; }
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1746 "cf-parse.y"
    {
     /* Fill data fields */
     struct f_tree *t;
     for (t = (yyvsp[(2) - (4)].e); t; t = t->left)
       t->data = (yyvsp[(4) - (4)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (4)].e), (yyvsp[(2) - (4)].e));
   }
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1753 "cf-parse.y"
    { 
     struct f_tree *t = f_new_tree();
     t->from.type = t->to.type = T_VOID;
     t->right = t;
     t->data = (yyvsp[(3) - (3)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), t);
 }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1765 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1766 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1770 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); }
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1771 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); }
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1775 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); }
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1776 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1777 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; }
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1778 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); }
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1779 "cf-parse.y"
    { (yyval.h) = NULL; }
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1783 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); }
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1784 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; }
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1785 "cf-parse.y"
    { (yyval.h) = NULL; }
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1789 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); }
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1790 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  }
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1791 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  }
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1792 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); }
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1793 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); }
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1794 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); }
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1795 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1796 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); }
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1797 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); }
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1798 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; }
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1799 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; }
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1803 "cf-parse.y"
    { (yyval.x) = f_generate_dpair((yyvsp[(2) - (5)].x), (yyvsp[(4) - (5)].x)); }
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1804 "cf-parse.y"
    { (yyval.x) = f_generate_ec((yyvsp[(2) - (7)].i), (yyvsp[(4) - (7)].x), (yyvsp[(6) - (7)].x)); }
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1818 "cf-parse.y"
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[(3) - (4)].x);
     if ((yyvsp[(1) - (4)].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[(1) - (4)].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[(1) - (4)].s)->def;
     sym = (yyvsp[(1) - (4)].s)->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[(1) - (4)].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1841 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     switch ((yyvsp[(1) - (1)].s)->class) {
       case SYM_NUMBER:
	(yyval.x) = f_new_inst();
	(yyval.x)->code = 'c'; 
	(yyval.x)->aux = T_INT; 
	(yyval.x)->a2.i = (yyvsp[(1) - (1)].s)->aux;
	break;
       case SYM_IPA:
	{ NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; val->type = T_IP; val->val.px.ip = * (ip_addr *) ((yyvsp[(1) - (1)].s)->def); }
	break;
       case SYM_VARIABLE | T_BOOL:
       case SYM_VARIABLE | T_INT:
       case SYM_VARIABLE | T_PAIR:
       case SYM_VARIABLE | T_QUAD:
       case SYM_VARIABLE | T_EC:
       case SYM_VARIABLE | T_STRING:
       case SYM_VARIABLE | T_IP:
       case SYM_VARIABLE | T_PREFIX:
       case SYM_VARIABLE | T_PREFIX_SET:
       case SYM_VARIABLE | T_SET:
       case SYM_VARIABLE | T_PATH:
       case SYM_VARIABLE | T_PATH_MASK:
       case SYM_VARIABLE | T_CLIST:
       case SYM_VARIABLE | T_ECLIST:
	 (yyval.x)->code = 'V';
	 (yyval.x)->a1.p = (yyvsp[(1) - (1)].s)->def;
	 (yyval.x)->a2.p = (yyvsp[(1) - (1)].s)->name;
	 break;
       default:
	 cf_error("%s: variable expected.", (yyvsp[(1) - (1)].s)->name );
     }
   }
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1877 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; }
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1879 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; }
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1880 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ }
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1881 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ }
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1882 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); }
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1883 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; }
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1884 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); }
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1885 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest);   (yyval.x)->a1.i = 1; }
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1889 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); }
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1890 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1891 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1892 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1893 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1894 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1895 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1896 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1897 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1898 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1899 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1900 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); }
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1901 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); }
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1902 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1903 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); }
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1904 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); }
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1906 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1907 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1908 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1910 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; }
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1912 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; }
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1914 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); }
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1916 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; }
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1917 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); }
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1919 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1920 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1930 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; }
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1931 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; }
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1932 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_ECLIST; }
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1933 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1934 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1935 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1936 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'f'; }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1938 "cf-parse.y"
    { (yyval.x) = f_generate_roa_check((yyvsp[(3) - (4)].s), NULL, NULL); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1939 "cf-parse.y"
    { (yyval.x) = f_generate_roa_check((yyvsp[(3) - (8)].s), (yyvsp[(5) - (8)].x), (yyvsp[(7) - (8)].x)); }
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1944 "cf-parse.y"
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[(3) - (4)].x);
     if ((yyvsp[(1) - (4)].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[(1) - (4)].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[(1) - (4)].s)->def;
     sym = (yyvsp[(1) - (4)].s)->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[(1) - (4)].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1967 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; }
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1968 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; }
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1969 "cf-parse.y"
    { (yyval.i) = F_REJECT; }
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1970 "cf-parse.y"
    { (yyval.i) = F_ERROR; }
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1971 "cf-parse.y"
    { (yyval.i) = F_NOP; }
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1972 "cf-parse.y"
    { (yyval.i) = F_NONL; }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1976 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; }
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1979 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1980 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1981 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1989 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   }
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1996 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 2005 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 2006 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 2010 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   }
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 2016 "cf-parse.y"
    {
     struct f_inst *i = f_new_inst();
     i->code = '?';
     i->a1.p = (yyvsp[(2) - (6)].x);
     i->a2.p = (yyvsp[(4) - (6)].x);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = i;
     (yyval.x)->a2.p = (yyvsp[(6) - (6)].x);
   }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 2026 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll set value\n" );
     if (((yyvsp[(1) - (4)].s)->class & ~T_MASK) != SYM_VARIABLE)
       cf_error( "You may set only variables." );
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = (yyvsp[(1) - (4)].s);
     (yyval.x)->a2.p = (yyvsp[(3) - (4)].x);
   }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 2035 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 2041 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 2046 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 2053 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 2058 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 2064 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 2065 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 2066 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 2074 "cf-parse.y"
    { (yyval.x) = f_generate_empty((yyvsp[(2) - (5)].x)); }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 2075 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 2076 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 2077 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 2078 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'f', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 2084 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_bgp, sizeof(struct bgp_config), (yyvsp[(1) - (2)].i));
     BGP_CFG->hold_time = 240;
     BGP_CFG->connect_retry_time = 120;
     BGP_CFG->initial_hold_time = 240;
     BGP_CFG->compare_path_lengths = 1;
     BGP_CFG->igp_metric = 1;
     BGP_CFG->start_delay_time = 5;
     BGP_CFG->error_amnesia_time = 300;
     BGP_CFG->error_delay_time_min = 60;
     BGP_CFG->error_delay_time_max = 300;
     BGP_CFG->enable_refresh = 1;
     BGP_CFG->enable_as4 = 1;
     BGP_CFG->capabilities = 2;
     BGP_CFG->advertise_ipv4 = 1;
     BGP_CFG->interpret_communities = 1;
     BGP_CFG->default_local_pref = 100;
 }
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 2107 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); }
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 2108 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(3) - (6)].a); BGP_CFG->local_as = (yyvsp[(5) - (6)].i); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 2109 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip))
       cf_error("Only one neighbor per BGP instance is allowed");
     if (!ipa_has_link_scope((yyvsp[(3) - (7)].a)) != !(yyvsp[(4) - (7)].iface))
       cf_error("Link-local address and interface scope must be used together");

     BGP_CFG->remote_ip = (yyvsp[(3) - (7)].a);
     BGP_CFG->iface = (yyvsp[(4) - (7)].iface);
     BGP_CFG->remote_as = (yyvsp[(6) - (7)].i);
   }
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 2119 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 2120 "cf-parse.y"
    { BGP_CFG->rr_client = 1; }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 2121 "cf-parse.y"
    { BGP_CFG->rs_client = 1; }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 2122 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 2123 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 2124 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 2125 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 2126 "cf-parse.y"
    { BGP_CFG->multihop = 64; }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 2127 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (4)].i); if (((yyvsp[(3) - (4)].i)<1) || ((yyvsp[(3) - (4)].i)>255)) cf_error("Multihop must be in range 1-255"); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 2128 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 2129 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 2130 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; }
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 2131 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; }
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 2132 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_DIRECT; }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 2133 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_RECURSIVE; }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 2134 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 2135 "cf-parse.y"
    { BGP_CFG->med_metric = (yyvsp[(4) - (5)].i); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 2136 "cf-parse.y"
    { BGP_CFG->igp_metric = (yyvsp[(4) - (5)].i); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 2137 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 2138 "cf-parse.y"
    { BGP_CFG->deterministic_med = (yyvsp[(4) - (5)].i); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 2139 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 2140 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 2141 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 2142 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 2143 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 2144 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); }
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 2145 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); }
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 2146 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); }
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 2147 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 2148 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 2149 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 2150 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 2151 "cf-parse.y"
    {
     this_proto->in_limit = cfg_allocz(sizeof(struct proto_limit));
     this_proto->in_limit->limit = (yyvsp[(4) - (5)].i);
     this_proto->in_limit->action = PLA_RESTART;
   }
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 2156 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); }
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 2157 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); }
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 2158 "cf-parse.y"
    { BGP_CFG->secondary = (yyvsp[(3) - (4)].i); }
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 2159 "cf-parse.y"
    { BGP_CFG->igp_table = (yyvsp[(4) - (5)].r); }
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 2160 "cf-parse.y"
    { BGP_CFG->ttl_security = (yyvsp[(4) - (5)].i); }
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 2171 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config), (yyvsp[(1) - (2)].i));
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  }
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 2187 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); }
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 2188 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (2)].i) ? DEFAULT_ECMP_LIMIT : 0; }
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 2189 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (4)].i) ? (yyvsp[(4) - (4)].i) : 0; if ((yyvsp[(4) - (4)].i) < 0) cf_error("ECMP limit cannot be negative"); }
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 2190 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i); if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); }
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2194 "cf-parse.y"
    {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = (yyvsp[(2) - (2)].i32);
  this_area->default_cost = DEFAULT_STUB_COST;
  this_area->type = OPT_E;
  this_area->transint = DEFAULT_TRANSINT;

  init_list(&this_area->patt_list);
  init_list(&this_area->net_list);
  init_list(&this_area->enet_list);
  init_list(&this_area->stubnet_list);
 }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2209 "cf-parse.y"
    { ospf_area_finish(); }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2218 "cf-parse.y"
    { this_area->type = (yyvsp[(2) - (2)].i) ? 0 : OPT_E; /* We should remove the option */ }
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2219 "cf-parse.y"
    { this_area->type = OPT_N; }
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2220 "cf-parse.y"
    { this_area->summary = (yyvsp[(2) - (2)].i); }
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2221 "cf-parse.y"
    { this_area->default_nssa = (yyvsp[(3) - (3)].i); }
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2222 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i); check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2223 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i) | LSA_EXT_EBIT; check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2224 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i);  check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2225 "cf-parse.y"
    { this_area->translator = (yyvsp[(2) - (2)].i); }
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2226 "cf-parse.y"
    { this_area->transint = (yyvsp[(3) - (3)].i); }
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2227 "cf-parse.y"
    { this_nets = &this_area->net_list; }
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2228 "cf-parse.y"
    { this_nets = &this_area->enet_list; }
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2240 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   }
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2254 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); }
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2255 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); }
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2256 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); }
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2260 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2261 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2270 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2271 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); }
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2272 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2273 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; }
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2274 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); }
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2275 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); }
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2276 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2277 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2278 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2283 "cf-parse.y"
    {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&OSPF_CFG->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->voa = this_area->areaid;
  OSPF_PATT->vid = (yyvsp[(3) - (3)].i32);
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->waitint = WAIT_DMH*HELLOINT_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->deadint = 0;
  OSPF_PATT->type = OSPF_IT_VLINK;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 }
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2304 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); }
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2305 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2306 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); }
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2307 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); }
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2308 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; }
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2309 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); }
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2310 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); }
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2311 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2312 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2313 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2314 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2315 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; }
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2316 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; }
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2317 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2318 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2319 "cf-parse.y"
    { OSPF_PATT->real_bcast = (yyvsp[(3) - (3)].i); if (OSPF_VERSION != 2) cf_error("Real broadcast option requires OSPFv2"); }
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2320 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2321 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); }
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2322 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; }
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2323 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; }
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2324 "cf-parse.y"
    { OSPF_PATT->check_link = (yyvsp[(3) - (3)].i); }
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2325 "cf-parse.y"
    { OSPF_PATT->ecmp_weight = (yyvsp[(3) - (3)].i) - 1; if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("ECMP weight must be in range 1-256"); }
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2327 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2328 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2329 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2330 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; }
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2331 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; }
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2332 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) || ((yyvsp[(3) - (3)].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); }
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2344 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(this_nets, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (1)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (1)].px).len;
 }
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2354 "cf-parse.y"
    { this_pref->hidden = 1; }
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2355 "cf-parse.y"
    { this_pref->tag = (yyvsp[(2) - (2)].i); }
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2368 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 }
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2377 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 }
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2387 "cf-parse.y"
    {
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->cost = COST_D;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->pollint = POLLINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->priority = PRIORITY_D;
  OSPF_PATT->waitint = WAIT_DMH*HELLOINT_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->deadint = 0;
  OSPF_PATT->type = OSPF_IT_UNDEF;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 }
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2409 "cf-parse.y"
    { set_instance_id((yyvsp[(2) - (2)].i)); }
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2423 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2428 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2434 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); }
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2437 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); }
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2440 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); }
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2445 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); }
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2448 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); }
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2453 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); }
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2456 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); }
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2460 "cf-parse.y"
    { ospf_sh_lsadb((yyvsp[(4) - (5)].ld)); }
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2463 "cf-parse.y"
    {
     (yyval.ld) = cfg_allocz(sizeof(struct lsadb_show_data));
   }
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2466 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->scope = LSA_SCOPE_AS; }
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2467 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->scope = LSA_SCOPE_AREA; (yyval.ld)->area = (yyvsp[(3) - (3)].i32) ;}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2468 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->scope = 1; /* hack, 0 is no filter */ }
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2469 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->type = (yyvsp[(3) - (3)].i); }
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2470 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->lsid = (yyvsp[(3) - (3)].i32); }
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2471 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->router = SH_ROUTER_SELF; }
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2472 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->router = (yyvsp[(3) - (3)].i32); }
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2473 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->name = (yyvsp[(2) - (2)].s); }
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2479 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config), (yyvsp[(1) - (2)].i));
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  }
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2488 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   }
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2493 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; }
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2494 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; }
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2500 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config), (yyvsp[(1) - (2)].i));
     rip_init_config(RIP_CFG);
   }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2509 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2510 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); }
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2511 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); }
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2512 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); }
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2513 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); }
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2514 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); }
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2516 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; }
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2517 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; }
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2518 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; }
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2523 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; }
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2524 "cf-parse.y"
    { (yyval.i)=AT_MD5; }
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2525 "cf-parse.y"
    { (yyval.i)=AT_NONE; }
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2530 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; }
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2531 "cf-parse.y"
    { (yyval.i)=0; }
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2532 "cf-parse.y"
    { (yyval.i)=IM_QUIET; }
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2533 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; }
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2534 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; }
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2538 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); }
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2539 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); }
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2553 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   }
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2569 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config), (yyvsp[(1) - (2)].i));
     static_init_config((struct static_config *) this_proto);
  }
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2578 "cf-parse.y"
    { STATIC_CFG->check_link = (yyvsp[(4) - (5)].i); }
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2579 "cf-parse.y"
    { STATIC_CFG->igp_table = (yyvsp[(4) - (5)].r); }
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2583 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  }
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2592 "cf-parse.y"
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[(2) - (3)].a);
     this_srt_nh->via_if = (yyvsp[(3) - (3)].iface);
     this_srt_nh->if_name = (void *) this_srt; /* really */
   }
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2600 "cf-parse.y"
    {
     this_srt_nh->masklen = (yyvsp[(3) - (3)].i) - 1; /* really */
     if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("Weight must be in range 1-256"); 
   }
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2607 "cf-parse.y"
    { this_srt->mp_next = this_srt_nh; }
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2608 "cf-parse.y"
    { last_srt_nh->mp_next = this_srt_nh; }
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2612 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (4)].a);
      this_srt->via_if = (yyvsp[(4) - (4)].iface);
   }
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2617 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   }
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2623 "cf-parse.y"
    {
      this_srt->dest = RTD_MULTIPATH;
   }
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2626 "cf-parse.y"
    {
      this_srt->dest = RTDX_RECURSIVE;
      this_srt->via = (yyvsp[(3) - (3)].a);
   }
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2630 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; }
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2631 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; }
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2632 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; }
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2636 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); }
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2644 "cf-parse.y"
    { bgp_check_config(BGP_CFG); }
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2644 "cf-parse.y"
    { ospf_proto_finish(); }
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2644 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); }
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2647 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_SOURCE); }
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2647 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_METRIC); }
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2647 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); }
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2647 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); }
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2648 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); }
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2648 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2649 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); }
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2650 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); }
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2651 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); }
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2652 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); }
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2653 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); }
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2654 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); }
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2655 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); }
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2656 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); }
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2657 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); }
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2658 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); }
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_EC_SET, T_ECLIST, EA_CODE(EAP_BGP, BA_EXT_COMMUNITY)); }
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); }
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); }
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); }
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); }
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); }
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2659 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); }
    break;



/* Line 1806 of yacc.c  */
#line 8513 "cf-parse.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 2661 "cf-parse.y"

/* C Code from ../../conf/confbase.Y */

/* C Code from ../../sysdep/unix/config.Y */

/* C Code from ../../sysdep/unix/krt.Y */

/* C Code from ../../sysdep/linux/netlink.Y */

/* C Code from ../../nest/config.Y */

/* C Code from ../../proto/bgp/config.Y */

/* C Code from ../../proto/ospf/config.Y */

/* C Code from ../../proto/pipe/config.Y */

/* C Code from ../../proto/rip/config.Y */

/* C Code from ../../proto/static/config.Y */



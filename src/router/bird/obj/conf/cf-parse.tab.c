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

/* Headers from ../../sysdep/linux/netlink/netlink.Y */

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
   cf_error("Default cost must be in range 1-%d", LSINFINITY);
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
#line 536 "cf-parse.tab.c"

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
     ASYNC = 311,
     TABLE = 312,
     KRT_PREFSRC = 313,
     KRT_REALM = 314,
     ROUTER = 315,
     ID = 316,
     PROTOCOL = 317,
     TEMPLATE = 318,
     PREFERENCE = 319,
     DISABLED = 320,
     DIRECT = 321,
     INTERFACE = 322,
     IMPORT = 323,
     EXPORT = 324,
     FILTER = 325,
     NONE = 326,
     STATES = 327,
     FILTERS = 328,
     PASSWORD = 329,
     FROM = 330,
     PASSIVE = 331,
     TO = 332,
     EVENTS = 333,
     PACKETS = 334,
     PROTOCOLS = 335,
     INTERFACES = 336,
     PRIMARY = 337,
     STATS = 338,
     COUNT = 339,
     FOR = 340,
     COMMANDS = 341,
     PREEXPORT = 342,
     GENERATE = 343,
     ROA = 344,
     MAX = 345,
     FLUSH = 346,
     LISTEN = 347,
     BGP = 348,
     V6ONLY = 349,
     DUAL = 350,
     ADDRESS = 351,
     PORT = 352,
     PASSWORDS = 353,
     DESCRIPTION = 354,
     RELOAD = 355,
     IN = 356,
     OUT = 357,
     MRTDUMP = 358,
     MESSAGES = 359,
     RESTRICT = 360,
     MEMORY = 361,
     IGP_METRIC = 362,
     SHOW = 363,
     STATUS = 364,
     SUMMARY = 365,
     ROUTE = 366,
     SYMBOLS = 367,
     ADD = 368,
     DELETE = 369,
     DUMP = 370,
     RESOURCES = 371,
     SOCKETS = 372,
     NEIGHBORS = 373,
     ATTRIBUTES = 374,
     ECHO = 375,
     DISABLE = 376,
     ENABLE = 377,
     RESTART = 378,
     FUNCTION = 379,
     PRINT = 380,
     PRINTN = 381,
     UNSET = 382,
     RETURN = 383,
     ACCEPT = 384,
     REJECT = 385,
     QUITBIRD = 386,
     INT = 387,
     BOOL = 388,
     IP = 389,
     PREFIX = 390,
     PAIR = 391,
     QUAD = 392,
     EC = 393,
     SET = 394,
     STRING = 395,
     BGPMASK = 396,
     BGPPATH = 397,
     CLIST = 398,
     ECLIST = 399,
     IF = 400,
     THEN = 401,
     ELSE = 402,
     CASE = 403,
     TRUE = 404,
     FALSE = 405,
     RT = 406,
     RO = 407,
     UNKNOWN = 408,
     GENERIC = 409,
     GW = 410,
     NET = 411,
     MASK = 412,
     PROTO = 413,
     SOURCE = 414,
     SCOPE = 415,
     CAST = 416,
     DEST = 417,
     LEN = 418,
     DEFINED = 419,
     CONTAINS = 420,
     RESET = 421,
     PREPEND = 422,
     FIRST = 423,
     LAST = 424,
     MATCH = 425,
     ROA_CHECK = 426,
     EMPTY = 427,
     WHERE = 428,
     EVAL = 429,
     LOCAL = 430,
     NEIGHBOR = 431,
     AS = 432,
     HOLD = 433,
     CONNECT = 434,
     RETRY = 435,
     KEEPALIVE = 436,
     MULTIHOP = 437,
     STARTUP = 438,
     VIA = 439,
     NEXT = 440,
     HOP = 441,
     SELF = 442,
     DEFAULT = 443,
     PATH = 444,
     METRIC = 445,
     START = 446,
     DELAY = 447,
     FORGET = 448,
     WAIT = 449,
     AFTER = 450,
     BGP_PATH = 451,
     BGP_LOCAL_PREF = 452,
     BGP_MED = 453,
     BGP_ORIGIN = 454,
     BGP_NEXT_HOP = 455,
     BGP_ATOMIC_AGGR = 456,
     BGP_AGGREGATOR = 457,
     BGP_COMMUNITY = 458,
     BGP_EXT_COMMUNITY = 459,
     RR = 460,
     RS = 461,
     CLIENT = 462,
     CLUSTER = 463,
     AS4 = 464,
     ADVERTISE = 465,
     IPV4 = 466,
     CAPABILITIES = 467,
     LIMIT = 468,
     PREFER = 469,
     OLDER = 470,
     MISSING = 471,
     LLADDR = 472,
     DROP = 473,
     IGNORE = 474,
     REFRESH = 475,
     INTERPRET = 476,
     COMMUNITIES = 477,
     BGP_ORIGINATOR_ID = 478,
     BGP_CLUSTER_LIST = 479,
     IGP = 480,
     GATEWAY = 481,
     RECURSIVE = 482,
     MED = 483,
     TTL = 484,
     SECURITY = 485,
     DETERMINISTIC = 486,
     OSPF = 487,
     AREA = 488,
     OSPF_METRIC1 = 489,
     OSPF_METRIC2 = 490,
     OSPF_TAG = 491,
     OSPF_ROUTER_ID = 492,
     RFC1583COMPAT = 493,
     STUB = 494,
     TICK = 495,
     COST = 496,
     COST2 = 497,
     RETRANSMIT = 498,
     HELLO = 499,
     TRANSMIT = 500,
     PRIORITY = 501,
     DEAD = 502,
     TYPE = 503,
     BROADCAST = 504,
     BCAST = 505,
     NONBROADCAST = 506,
     NBMA = 507,
     POINTOPOINT = 508,
     PTP = 509,
     POINTOMULTIPOINT = 510,
     PTMP = 511,
     SIMPLE = 512,
     AUTHENTICATION = 513,
     STRICT = 514,
     CRYPTOGRAPHIC = 515,
     ELIGIBLE = 516,
     POLL = 517,
     NETWORKS = 518,
     HIDDEN = 519,
     VIRTUAL = 520,
     CHECK = 521,
     LINK = 522,
     RX = 523,
     BUFFER = 524,
     LARGE = 525,
     NORMAL = 526,
     STUBNET = 527,
     TAG = 528,
     EXTERNAL = 529,
     LSADB = 530,
     ECMP = 531,
     WEIGHT = 532,
     NSSA = 533,
     TRANSLATOR = 534,
     STABILITY = 535,
     GLOBAL = 536,
     LSID = 537,
     TOPOLOGY = 538,
     STATE = 539,
     PIPE = 540,
     PEER = 541,
     MODE = 542,
     OPAQUE = 543,
     TRANSPARENT = 544,
     RIP = 545,
     INFINITY = 546,
     PERIOD = 547,
     GARBAGE = 548,
     TIMEOUT = 549,
     MULTICAST = 550,
     QUIET = 551,
     NOLISTEN = 552,
     VERSION1 = 553,
     PLAINTEXT = 554,
     MD5 = 555,
     HONOR = 556,
     NEVER = 557,
     ALWAYS = 558,
     RIP_METRIC = 559,
     RIP_TAG = 560,
     STATIC = 561,
     PROHIBIT = 562,
     MULTIPATH = 563
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 459 "cf-parse.y"

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
#line 909 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 921 "cf-parse.tab.c"

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
#define YYLAST   2461

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  330
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  224
/* YYNRULES -- Number of rules.  */
#define YYNRULES  725
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1368

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   563

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    30,     2,     2,
     320,   321,    28,    26,   326,    27,    32,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   323,   322,
      23,    22,    24,   327,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   328,     2,   329,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   324,     2,   325,    25,     2,     2,     2,
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
     316,   317,   318,   319
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
     220,   223,   226,   228,   230,   233,   237,   238,   247,   249,
     254,   256,   258,   259,   261,   265,   266,   269,   272,   275,
     278,   281,   284,   287,   291,   294,   297,   299,   301,   303,
     305,   309,   313,   314,   316,   318,   321,   322,   324,   328,
     330,   334,   337,   341,   345,   349,   350,   354,   356,   358,
     362,   364,   368,   370,   372,   374,   376,   378,   380,   382,
     384,   388,   390,   394,   396,   398,   403,   405,   406,   410,
     415,   417,   420,   421,   427,   433,   439,   445,   450,   454,
     458,   463,   469,   471,   472,   476,   481,   486,   487,   490,
     494,   498,   502,   505,   508,   511,   515,   519,   522,   525,
     527,   529,   534,   535,   539,   543,   547,   548,   550,   552,
     557,   558,   561,   564,   567,   570,   573,   576,   579,   580,
     583,   593,   603,   608,   612,   616,   620,   624,   628,   632,
     636,   641,   643,   645,   647,   648,   650,   654,   658,   662,
     666,   671,   676,   681,   686,   689,   691,   693,   695,   697,
     698,   700,   701,   706,   709,   711,   713,   715,   717,   719,
     721,   723,   725,   727,   729,   731,   733,   736,   739,   740,
     744,   746,   750,   752,   754,   756,   759,   763,   766,   771,
     772,   778,   779,   781,   783,   786,   788,   792,   794,   796,
     798,   800,   802,   804,   808,   810,   812,   814,   816,   818,
     822,   824,   830,   842,   844,   846,   848,   851,   853,   861,
     871,   879,   881,   883,   885,   889,   891,   893,   895,   899,
     901,   905,   907,   911,   915,   917,   920,   923,   930,   932,
     936,   937,   942,   946,   948,   952,   956,   960,   963,   966,
     969,   972,   973,   976,   979,   980,   982,   984,   986,   988,
     990,   992,   994,   998,  1002,  1004,  1006,  1012,  1020,  1021,
    1026,  1028,  1030,  1032,  1034,  1036,  1038,  1040,  1042,  1044,
    1048,  1052,  1056,  1060,  1064,  1068,  1072,  1076,  1080,  1084,
    1088,  1092,  1096,  1100,  1103,  1108,  1110,  1112,  1114,  1116,
    1119,  1122,  1126,  1130,  1137,  1141,  1145,  1149,  1153,  1159,
    1166,  1173,  1180,  1187,  1192,  1201,  1206,  1208,  1210,  1212,
    1214,  1216,  1218,  1220,  1221,  1223,  1227,  1229,  1233,  1234,
    1236,  1241,  1248,  1253,  1257,  1263,  1269,  1274,  1281,  1285,
    1288,  1294,  1300,  1309,  1318,  1327,  1336,  1339,  1343,  1347,
    1353,  1360,  1368,  1375,  1380,  1385,  1391,  1398,  1405,  1411,
    1415,  1420,  1426,  1432,  1438,  1444,  1449,  1454,  1460,  1466,
    1472,  1478,  1484,  1490,  1496,  1502,  1509,  1516,  1525,  1532,
    1539,  1545,  1550,  1556,  1561,  1567,  1572,  1578,  1584,  1590,
    1593,  1597,  1601,  1603,  1606,  1609,  1614,  1617,  1619,  1622,
    1627,  1628,  1632,  1635,  1637,  1640,  1644,  1648,  1652,  1656,
    1659,  1663,  1664,  1670,  1671,  1677,  1680,  1683,  1685,  1690,
    1692,  1694,  1695,  1699,  1702,  1705,  1708,  1713,  1715,  1716,
    1720,  1721,  1724,  1727,  1731,  1734,  1737,  1741,  1744,  1747,
    1750,  1752,  1756,  1759,  1762,  1765,  1768,  1771,  1774,  1778,
    1781,  1784,  1787,  1790,  1793,  1796,  1799,  1802,  1806,  1809,
    1813,  1816,  1820,  1824,  1829,  1832,  1835,  1838,  1842,  1846,
    1850,  1852,  1853,  1856,  1860,  1862,  1863,  1865,  1868,  1869,
    1872,  1874,  1876,  1879,  1883,  1884,  1885,  1889,  1890,  1894,
    1898,  1900,  1901,  1906,  1913,  1920,  1927,  1935,  1942,  1950,
    1956,  1957,  1960,  1964,  1967,  1971,  1975,  1978,  1982,  1985,
    1988,  1992,  1996,  2002,  2007,  2012,  2015,  2019,  2023,  2028,
    2033,  2038,  2044,  2050,  2055,  2059,  2064,  2069,  2074,  2079,
    2081,  2083,  2085,  2087,  2089,  2091,  2093,  2095,  2096,  2099,
    2102,  2103,  2107,  2108,  2112,  2113,  2117,  2120,  2124,  2128,
    2134,  2140,  2144,  2147,  2151,  2155,  2157,  2160,  2165,  2169,
    2173,  2177,  2180,  2183,  2186,  2191,  2193,  2195,  2197,  2199,
    2201,  2203,  2205,  2207,  2209,  2211,  2213,  2215,  2217,  2219,
    2221,  2223,  2225,  2227,  2229,  2231,  2233,  2235,  2237,  2239,
    2241,  2243,  2245,  2247,  2249,  2251,  2253,  2255,  2257,  2259,
    2261,  2263,  2265,  2267,  2269,  2271,  2273,  2275,  2277,  2279,
    2281,  2283,  2285,  2287,  2289,  2291,  2293,  2295,  2297,  2299,
    2301,  2304,  2307,  2310,  2313,  2316,  2319,  2322,  2325,  2329,
    2333,  2337,  2341,  2345,  2349,  2353,  2355,  2357,  2359,  2361,
    2363,  2365,  2367,  2369,  2371,  2373,  2375,  2377,  2379,  2381,
    2383,  2385,  2387,  2389,  2391,  2393
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     331,     0,    -1,   332,     3,    -1,     4,   549,    -1,    -1,
     332,   548,    -1,    15,    -1,   320,   481,   321,    -1,    19,
      -1,    33,    19,    22,   333,   322,    -1,    33,    19,    22,
      18,   322,    -1,   333,    -1,    34,    -1,    36,    -1,    35,
      -1,    37,    -1,    -1,    18,    -1,    19,    -1,    -1,    30,
      19,    -1,   336,   340,    -1,   338,    -1,   336,    -1,    29,
     333,    -1,   323,   336,    -1,    20,    -1,    20,    -1,    -1,
      38,   345,   346,   322,    -1,    57,    20,    -1,    -1,    20,
      -1,    39,   344,    -1,    50,    -1,    40,    -1,   324,   347,
     325,    -1,   348,    -1,   347,   326,   348,    -1,    41,    -1,
      42,    -1,    43,    -1,    44,    -1,    45,    -1,    46,    -1,
      47,    -1,    48,    -1,    49,    -1,   114,    91,   389,   322,
      -1,   114,    20,   322,    -1,   122,    -1,    73,    -1,    56,
      -1,    38,    -1,   350,    20,    -1,   350,    20,   333,    20,
      -1,   350,    53,    54,    -1,   350,    53,    55,    -1,    52,
     351,   322,    -1,    58,   356,     3,    -1,    58,    51,   356,
       3,    -1,    59,     3,    -1,    -1,    20,    -1,   371,    60,
      -1,    61,   335,    -1,    62,    63,   333,    -1,    64,   335,
      -1,    65,    66,   335,    -1,   371,    65,    -1,    62,    63,
     333,    -1,    93,   342,   339,    -1,    60,    68,   333,    -1,
      71,    72,   363,   322,    -1,    15,    -1,    17,    -1,    18,
      -1,   103,   104,   365,   322,    -1,    -1,   365,   366,    -1,
     107,   336,    -1,   108,   333,    -1,   105,    -1,   106,    -1,
      68,    19,    -1,   100,    68,    19,    -1,    -1,   369,   100,
     338,   101,    15,   188,    15,   322,    -1,   368,    -1,   368,
     324,   369,   325,    -1,    73,    -1,    74,    -1,    -1,    19,
      -1,    19,    86,    19,    -1,    -1,    75,   333,    -1,    76,
     335,    -1,    41,   386,    -1,   114,   389,    -1,    79,   374,
      -1,    80,   374,    -1,    68,   375,    -1,    71,    72,   363,
      -1,   110,    20,    -1,    81,   445,    -1,   446,    -1,    40,
      -1,    82,    -1,    19,    -1,    41,    91,   386,    -1,    41,
      97,   333,    -1,    -1,    20,    -1,   339,    -1,    20,   339,
      -1,    -1,    27,    -1,   377,   379,   378,    -1,   380,    -1,
     381,   326,   380,    -1,   371,    77,    -1,   382,   372,   324,
      -1,   383,   373,   322,    -1,   383,   385,   322,    -1,    -1,
      78,   384,   381,    -1,    40,    -1,    35,    -1,   324,   387,
     325,    -1,   388,    -1,   387,   326,   388,    -1,    83,    -1,
      66,    -1,    84,    -1,    92,    -1,    89,    -1,    90,    -1,
      40,    -1,    35,    -1,   324,   390,   325,    -1,   391,    -1,
     390,   326,   391,    -1,    83,    -1,   115,    -1,   109,   324,
     393,   325,    -1,   394,    -1,    -1,   394,   322,   393,    -1,
     395,   324,   396,   325,    -1,   395,    -1,    85,    20,    -1,
      -1,    99,    86,   341,   322,   396,    -1,    99,    88,   341,
     322,   396,    -1,   140,    86,   341,   322,   396,    -1,   140,
      88,   341,   322,   396,    -1,    72,   333,   322,   396,    -1,
     119,   120,     3,    -1,   119,   117,     3,    -1,   119,    91,
     436,     3,    -1,   119,    91,    40,   436,     3,    -1,    19,
      -1,    -1,   119,    92,     3,    -1,   119,    92,   121,     3,
      -1,   119,   122,   405,     3,    -1,    -1,   405,   338,    -1,
     405,    96,   339,    -1,   405,    68,    19,    -1,   405,    81,
     445,    -1,   405,   446,    -1,   405,    40,    -1,   405,    93,
      -1,   405,   406,    19,    -1,   405,    73,    19,    -1,   405,
      94,    -1,   405,    95,    -1,    98,    -1,    80,    -1,   119,
     100,   408,     3,    -1,    -1,   408,   409,   338,    -1,   408,
     188,    15,    -1,   408,    68,    19,    -1,    -1,   112,    -1,
      96,    -1,   119,   123,   411,     3,    -1,    -1,   411,    68,
      -1,   411,   135,    -1,   411,    81,    -1,   411,    73,    -1,
     411,    74,    -1,   411,   100,    -1,   411,    19,    -1,    -1,
      68,    19,    -1,   124,   100,   338,   101,    15,   188,    15,
     412,     3,    -1,   125,   100,   338,   101,    15,   188,    15,
     412,     3,    -1,   102,   100,   412,     3,    -1,   126,   127,
       3,    -1,   126,   128,     3,    -1,   126,    92,     3,    -1,
     126,   129,     3,    -1,   126,   130,     3,    -1,   126,    66,
       3,    -1,   126,    91,     3,    -1,   131,   424,   425,     3,
      -1,    40,    -1,    35,    -1,    15,    -1,    -1,    15,    -1,
     132,   435,     3,    -1,   133,   435,     3,    -1,   134,   435,
       3,    -1,   111,   435,     3,    -1,   111,   112,   435,     3,
      -1,   111,   113,   435,     3,    -1,    41,   435,   386,     3,
      -1,   114,   435,   389,     3,    -1,   116,     3,    -1,    19,
      -1,    40,    -1,    20,    -1,    19,    -1,    -1,    20,    -1,
      -1,    81,    19,   438,   444,    -1,   185,   481,    -1,   143,
      -1,   144,    -1,   145,    -1,   146,    -1,   147,    -1,   148,
      -1,   149,    -1,   151,    -1,   152,    -1,   153,    -1,   154,
      -1,   155,    -1,   440,   150,    -1,   440,    19,    -1,    -1,
     441,   322,   442,    -1,   441,    -1,   441,   322,   443,    -1,
     448,    -1,    19,    -1,   444,    -1,   184,   481,    -1,   320,
     443,   321,    -1,   320,   321,    -1,   442,   324,   451,   325,
      -1,    -1,   135,    19,   450,   447,   448,    -1,    -1,   452,
      -1,   487,    -1,   452,   487,    -1,   487,    -1,   324,   451,
     325,    -1,    18,    -1,   333,    -1,    17,    -1,   454,    -1,
      16,    -1,    15,    -1,   320,   481,   321,    -1,    17,    -1,
     454,    -1,    16,    -1,   481,    -1,   457,    -1,   457,     7,
     457,    -1,    28,    -1,   320,   458,   326,   458,   321,    -1,
     320,   458,   326,   458,   321,     7,   320,   457,   326,   457,
     321,    -1,   481,    -1,   162,    -1,   163,    -1,   164,    15,
      -1,   165,    -1,   320,   461,   326,   460,   326,   460,   321,
      -1,   320,   461,   326,   460,   326,   460,     7,   460,   321,
      -1,   320,   461,   326,   460,   326,    28,   321,    -1,   459,
      -1,   462,    -1,   455,    -1,   455,     7,   455,    -1,   459,
      -1,   462,    -1,   456,    -1,   456,     7,   456,    -1,   463,
      -1,   465,   326,   463,    -1,   464,    -1,   466,   326,   464,
      -1,    18,    29,    15,    -1,   467,    -1,   467,    26,    -1,
     467,    27,    -1,   467,   324,    15,   326,    15,   325,    -1,
     468,    -1,   469,   326,   468,    -1,    -1,   470,   466,   323,
     451,    -1,   470,     6,   451,    -1,   479,    -1,   320,   481,
     321,    -1,    13,   473,    14,    -1,    29,   474,    29,    -1,
      15,   473,    -1,    28,   473,    -1,   327,   473,    -1,   471,
     473,    -1,    -1,    15,   474,    -1,   327,   474,    -1,    -1,
      15,    -1,   160,    -1,   161,    -1,    20,    -1,   454,    -1,
     467,    -1,    17,    -1,   328,   465,   329,    -1,   328,   469,
     329,    -1,    16,    -1,   472,    -1,   320,   481,   326,   481,
     321,    -1,   320,   461,   326,   481,   326,   481,   321,    -1,
      -1,    19,   320,   486,   321,    -1,    19,    -1,    86,    -1,
     166,    -1,   167,    -1,   169,    -1,   170,    -1,   171,    -1,
     172,    -1,   173,    -1,   320,   481,   321,    -1,   481,    26,
     481,    -1,   481,    27,   481,    -1,   481,    28,   481,    -1,
     481,    29,   481,    -1,   481,    11,   481,    -1,   481,    12,
     481,    -1,   481,    22,   481,    -1,   481,    10,   481,    -1,
     481,    23,   481,    -1,   481,     9,   481,    -1,   481,    24,
     481,    -1,   481,     8,   481,    -1,   481,    25,   481,    -1,
      31,   481,    -1,   175,   320,   481,   321,    -1,   479,    -1,
     475,    -1,   476,    -1,    75,    -1,   477,   480,    -1,   477,
     553,    -1,   481,    32,   145,    -1,   481,    32,   174,    -1,
     481,    32,   168,   320,   481,   321,    -1,   481,    32,   179,
      -1,   481,    32,   180,    -1,    26,   183,    26,    -1,    27,
     183,    27,    -1,    27,    27,   183,    27,    27,    -1,   178,
     320,   481,   326,   481,   321,    -1,   124,   320,   481,   326,
     481,   321,    -1,   125,   320,   481,   326,   481,   321,    -1,
      81,   320,   481,   326,   481,   321,    -1,   182,   320,    19,
     321,    -1,   182,   320,    19,   326,   481,   326,   481,   321,
      -1,    19,   320,   486,   321,    -1,   142,    -1,   140,    -1,
     141,    -1,    46,    -1,   136,    -1,   137,    -1,   481,    -1,
      -1,   483,    -1,   483,   326,   484,    -1,   481,    -1,   481,
     326,   485,    -1,    -1,   485,    -1,   156,   481,   157,   453,
      -1,   156,   481,   157,   453,   158,   453,    -1,    19,    22,
     481,   322,    -1,   139,   481,   322,    -1,   477,   553,    22,
     481,   322,    -1,   477,   480,    22,   481,   322,    -1,    75,
      22,   481,   322,    -1,   138,   320,   477,   553,   321,   322,
      -1,   482,   484,   322,    -1,   478,   322,    -1,   159,   481,
     324,   470,   325,    -1,   477,   553,    32,   183,   322,    -1,
     477,   553,    32,   178,   320,   481,   321,   322,    -1,   477,
     553,    32,   124,   320,   481,   321,   322,    -1,   477,   553,
      32,   125,   320,   481,   321,   322,    -1,   477,   553,    32,
      81,   320,   481,   321,   322,    -1,   371,   104,    -1,   488,
     372,   324,    -1,   489,   373,   322,    -1,   489,   186,   188,
     333,   322,    -1,   489,   186,   336,   188,   333,   322,    -1,
     489,   187,   336,   337,   188,   333,   322,    -1,   489,   216,
     219,    72,   363,   322,    -1,   489,   216,   218,   322,    -1,
     489,   217,   218,   322,    -1,   489,   189,    63,   333,   322,
      -1,   489,   194,   189,    63,   333,   322,    -1,   489,   190,
     191,    63,   333,   322,    -1,   489,   192,    63,   333,   322,
      -1,   489,   193,   322,    -1,   489,   193,   333,   322,    -1,
     489,   196,   197,   198,   322,    -1,   489,   227,   228,   198,
     322,    -1,   489,   227,   228,   229,   322,    -1,   489,   227,
     228,   230,   322,    -1,   489,   237,    77,   322,    -1,   489,
     237,   238,   322,    -1,   489,   200,   201,   335,   322,    -1,
     489,   239,   201,   335,   322,    -1,   489,   236,   201,   335,
     322,    -1,   489,   225,   226,   335,   322,    -1,   489,   242,
     239,   335,   322,    -1,   489,   199,   209,   333,   322,    -1,
     489,   199,   208,   333,   322,    -1,   489,   170,   107,   336,
     322,    -1,   489,   202,   203,    63,   333,   322,    -1,   489,
      46,   204,    63,   333,   322,    -1,   489,    46,   205,    63,
     333,   326,   333,   322,    -1,   489,   132,   206,    46,   335,
     322,    -1,   489,   133,   122,   231,   335,   322,    -1,   489,
     133,   220,   335,   322,    -1,   489,   223,   335,   322,    -1,
     489,   221,   222,   335,   322,    -1,   489,    85,    20,   322,
      -1,   489,   122,   224,   333,   322,    -1,   489,    87,   335,
     322,    -1,   489,   232,   233,   335,   322,    -1,   489,   236,
      68,   375,   322,    -1,   489,   240,   241,   335,   322,    -1,
     371,   243,    -1,   490,   372,   324,    -1,   491,   492,   322,
      -1,   373,    -1,   249,   335,    -1,   287,   335,    -1,   287,
     335,   224,   333,    -1,   251,   333,    -1,   494,    -1,   244,
     363,    -1,   493,   324,   495,   325,    -1,    -1,   495,   496,
     322,    -1,   250,   335,    -1,   289,    -1,   121,   335,    -1,
     199,   289,   335,    -1,   199,   252,   333,    -1,   199,   253,
     333,    -1,   250,   252,   333,    -1,   290,   335,    -1,   290,
     291,   333,    -1,    -1,   274,   497,   324,   508,   325,    -1,
      -1,   285,   498,   324,   508,   325,    -1,   283,   499,    -1,
      78,   519,    -1,   503,    -1,   500,   324,   501,   325,    -1,
     500,    -1,   338,    -1,    -1,   501,   502,   322,    -1,   275,
     335,    -1,   121,   335,    -1,   252,   333,    -1,   506,   324,
     504,   325,    -1,   506,    -1,    -1,   504,   505,   322,    -1,
      -1,   255,   333,    -1,   254,   333,    -1,   256,   203,   333,
      -1,   205,   333,    -1,   258,   333,    -1,   258,    95,   333,
      -1,   269,    82,    -1,   269,   268,    -1,   269,   271,    -1,
     392,    -1,   276,   278,   363,    -1,   252,   333,    -1,   255,
     333,    -1,   273,   333,    -1,   254,   333,    -1,   205,   333,
      -1,   258,   333,    -1,   258,    95,   333,    -1,   259,   260,
      -1,   259,   261,    -1,   259,   262,    -1,   259,   263,    -1,
     259,   264,    -1,   259,   265,    -1,   259,   266,    -1,   259,
     267,    -1,   256,   203,   333,    -1,   257,   333,    -1,   270,
     262,   335,    -1,   250,   335,    -1,   277,   278,   335,    -1,
     287,   288,   333,    -1,   129,   324,   512,   325,    -1,   269,
      82,    -1,   269,   268,    -1,   269,   271,    -1,   279,   280,
     281,    -1,   279,   280,   282,    -1,   279,   280,   333,    -1,
     392,    -1,    -1,   508,   509,    -1,   510,   511,   322,    -1,
     338,    -1,    -1,   275,    -1,   284,   333,    -1,    -1,   512,
     513,    -1,   514,    -1,   515,    -1,    18,   322,    -1,    18,
     272,   322,    -1,    -1,    -1,   517,   507,   322,    -1,    -1,
     324,   517,   325,    -1,   516,   381,   518,    -1,    20,    -1,
      -1,   119,   243,   401,     3,    -1,   119,   243,   129,   401,
     520,     3,    -1,   119,   243,    78,   401,   520,     3,    -1,
     119,   243,   294,   401,   520,     3,    -1,   119,   243,   294,
      40,   401,   520,     3,    -1,   119,   243,   295,   401,   520,
       3,    -1,   119,   243,   295,    40,   401,   520,     3,    -1,
     119,   243,   286,   529,     3,    -1,    -1,   529,   292,    -1,
     529,   244,   363,    -1,   529,   278,    -1,   529,   259,    15,
      -1,   529,   293,   363,    -1,   529,   198,    -1,   529,    71,
     363,    -1,   529,    19,    -1,   371,   296,    -1,   530,   372,
     324,    -1,   531,   373,   322,    -1,   531,   297,    68,    19,
     322,    -1,   531,   298,   299,   322,    -1,   531,   298,   300,
     322,    -1,   371,   301,    -1,   532,   372,   324,    -1,   533,
     373,   322,    -1,   533,   302,   333,   322,    -1,   533,   108,
     333,   322,    -1,   533,   303,   333,   322,    -1,   533,   304,
      63,   333,   322,    -1,   533,   305,    63,   333,   322,    -1,
     533,   269,   534,   322,    -1,   533,   392,   322,    -1,   533,
     312,   314,   322,    -1,   533,   312,   187,   322,    -1,   533,
     312,   313,   322,    -1,   533,    78,   540,   322,    -1,   310,
      -1,   311,    -1,    82,    -1,   260,    -1,   306,    -1,   307,
      -1,   308,    -1,   309,    -1,    -1,   201,   333,    -1,   298,
     535,    -1,    -1,   537,   536,   322,    -1,    -1,   324,   537,
     325,    -1,    -1,   539,   381,   538,    -1,   371,   317,    -1,
     541,   372,   324,    -1,   542,   373,   322,    -1,   542,   277,
     278,   335,   322,    -1,   542,   236,    68,   375,   322,    -1,
     542,   546,   322,    -1,   122,   338,    -1,   195,   336,   337,
      -1,   544,   288,   333,    -1,   544,    -1,   545,   544,    -1,
     543,   195,   336,   337,    -1,   543,   195,    20,    -1,   543,
     319,   545,    -1,   543,   238,   336,    -1,   543,   229,    -1,
     543,   141,    -1,   543,   318,    -1,   119,   317,   401,     3,
      -1,   322,    -1,   334,    -1,   343,    -1,   349,    -1,   352,
      -1,   362,    -1,   364,    -1,   367,    -1,   370,    -1,   550,
      -1,   376,    -1,   437,    -1,   439,    -1,   449,    -1,   353,
      -1,   354,    -1,   355,    -1,   397,    -1,   398,    -1,   399,
      -1,   400,    -1,   402,    -1,   403,    -1,   404,    -1,   407,
      -1,   410,    -1,   413,    -1,   414,    -1,   415,    -1,   416,
      -1,   417,    -1,   418,    -1,   419,    -1,   420,    -1,   421,
      -1,   422,    -1,   423,    -1,   426,    -1,   427,    -1,   428,
      -1,   429,    -1,   430,    -1,   431,    -1,   432,    -1,   433,
      -1,   434,    -1,   521,    -1,   522,    -1,   523,    -1,   524,
      -1,   525,    -1,   526,    -1,   527,    -1,   528,    -1,   547,
      -1,   551,   325,    -1,   552,   325,    -1,   383,   325,    -1,
     489,   325,    -1,   491,   325,    -1,   531,   325,    -1,   533,
     325,    -1,   542,   325,    -1,   357,   372,   324,    -1,   551,
     373,   322,    -1,   551,   358,   322,    -1,   551,   361,   322,
      -1,   359,   372,   324,    -1,   552,   373,   322,    -1,   552,
     360,   322,    -1,    69,    -1,    70,    -1,   118,    -1,     5,
      -1,   210,    -1,   207,    -1,   211,    -1,   209,    -1,   208,
      -1,   212,    -1,   213,    -1,   214,    -1,   234,    -1,   235,
      -1,   215,    -1,   245,    -1,   246,    -1,   247,    -1,   248,
      -1,   315,    -1,   316,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   633,   633,   634,   637,   639,   646,   647,   648,   654,
     658,   667,   668,   669,   670,   671,   672,   678,   679,   686,
     687,   691,   698,   699,   703,   707,   714,   722,   723,   729,
     738,   739,   743,   748,   749,   753,   754,   758,   759,   763,
     764,   765,   766,   767,   768,   769,   770,   771,   777,   778,
     787,   788,   789,   790,   793,   794,   795,   796,   800,   807,
     810,   813,   817,   818,   826,   841,   842,   846,   853,   859,
     871,   875,   888,   901,   907,   908,   909,   920,   922,   924,
     928,   929,   930,   931,   938,   944,   948,   950,   956,   957,
     964,   965,   969,   975,   979,   989,   991,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1006,  1007,  1008,  1009,  1013,
    1021,  1022,  1030,  1038,  1039,  1040,  1044,  1045,  1049,  1054,
    1055,  1062,  1069,  1070,  1071,  1075,  1083,  1089,  1090,  1091,
    1095,  1096,  1100,  1101,  1102,  1103,  1104,  1105,  1111,  1112,
    1113,  1117,  1118,  1122,  1123,  1129,  1130,  1133,  1135,  1139,
    1140,  1144,  1162,  1163,  1164,  1165,  1166,  1167,  1175,  1178,
    1181,  1184,  1188,  1189,  1192,  1195,  1199,  1203,  1209,  1215,
    1222,  1227,  1232,  1237,  1241,  1245,  1255,  1263,  1267,  1274,
    1275,  1280,  1284,  1291,  1298,  1302,  1310,  1311,  1312,  1317,
    1321,  1324,  1325,  1326,  1327,  1328,  1329,  1330,  1335,  1340,
    1348,  1355,  1362,  1370,  1372,  1374,  1376,  1378,  1380,  1382,
    1385,  1391,  1392,  1393,  1397,  1398,  1404,  1406,  1408,  1410,
    1412,  1414,  1418,  1422,  1425,  1429,  1430,  1431,  1435,  1436,
    1437,  1445,  1445,  1455,  1459,  1460,  1461,  1462,  1463,  1464,
    1465,  1466,  1467,  1468,  1469,  1470,  1471,  1492,  1503,  1504,
    1511,  1512,  1519,  1528,  1532,  1536,  1560,  1561,  1565,  1578,
    1578,  1594,  1595,  1598,  1599,  1603,  1606,  1615,  1628,  1629,
    1630,  1631,  1635,  1636,  1637,  1638,  1639,  1643,  1646,  1647,
    1648,  1652,  1655,  1664,  1667,  1668,  1669,  1670,  1674,  1675,
    1676,  1680,  1681,  1682,  1683,  1687,  1688,  1689,  1690,  1694,
    1695,  1699,  1700,  1704,  1711,  1712,  1713,  1714,  1721,  1722,
    1725,  1726,  1733,  1745,  1746,  1750,  1751,  1755,  1756,  1757,
    1758,  1759,  1763,  1764,  1765,  1769,  1770,  1771,  1772,  1773,
    1774,  1775,  1776,  1777,  1778,  1779,  1783,  1784,  1794,  1798,
    1821,  1857,  1859,  1860,  1861,  1862,  1863,  1864,  1865,  1869,
    1870,  1871,  1872,  1873,  1874,  1875,  1876,  1877,  1878,  1879,
    1880,  1881,  1882,  1883,  1884,  1886,  1887,  1888,  1890,  1892,
    1894,  1896,  1897,  1898,  1899,  1900,  1910,  1911,  1912,  1913,
    1914,  1915,  1916,  1918,  1919,  1924,  1947,  1948,  1949,  1950,
    1951,  1952,  1956,  1959,  1960,  1961,  1969,  1976,  1985,  1986,
    1990,  1996,  2006,  2015,  2021,  2026,  2033,  2038,  2044,  2045,
    2046,  2054,  2055,  2056,  2057,  2058,  2064,  2085,  2086,  2087,
    2088,  2089,  2099,  2100,  2101,  2102,  2103,  2104,  2105,  2106,
    2107,  2108,  2109,  2110,  2111,  2112,  2113,  2114,  2115,  2116,
    2117,  2118,  2119,  2120,  2121,  2122,  2123,  2124,  2125,  2126,
    2127,  2128,  2129,  2130,  2131,  2132,  2133,  2134,  2135,  2146,
    2156,  2157,  2161,  2162,  2163,  2164,  2165,  2166,  2169,  2184,
    2187,  2189,  2193,  2194,  2195,  2196,  2197,  2198,  2199,  2200,
    2201,  2202,  2202,  2203,  2203,  2204,  2205,  2206,  2210,  2211,
    2215,  2223,  2225,  2229,  2230,  2231,  2235,  2236,  2239,  2241,
    2244,  2245,  2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,
    2254,  2257,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2310,  2312,  2315,  2317,  2326,  2328,  2329,  2332,  2334,
    2338,  2339,  2341,  2350,  2361,  2381,  2383,  2386,  2388,  2392,
    2396,  2397,  2402,  2405,  2408,  2413,  2416,  2421,  2424,  2428,
    2432,  2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2448,
    2455,  2456,  2457,  2462,  2463,  2469,  2476,  2477,  2478,  2479,
    2480,  2481,  2482,  2483,  2484,  2485,  2486,  2487,  2488,  2492,
    2493,  2494,  2499,  2500,  2501,  2502,  2503,  2506,  2507,  2508,
    2511,  2513,  2516,  2518,  2522,  2531,  2538,  2545,  2546,  2547,
    2548,  2549,  2552,  2561,  2569,  2576,  2577,  2581,  2586,  2592,
    2595,  2599,  2600,  2601,  2604,  2611,  2611,  2611,  2611,  2611,
    2611,  2611,  2611,  2611,  2611,  2611,  2611,  2611,  2611,  2612,
    2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,
    2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,
    2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,
    2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,  2612,
    2613,  2613,  2613,  2613,  2613,  2613,  2613,  2613,  2614,  2614,
    2614,  2614,  2615,  2615,  2615,  2616,  2616,  2616,  2617,  2617,
    2618,  2619,  2620,  2621,  2622,  2623,  2624,  2625,  2626,  2627,
    2628,  2628,  2628,  2628,  2628,  2628
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
  "DEVICE", "ROUTES", "ASYNC", "TABLE", "KRT_PREFSRC", "KRT_REALM",
  "ROUTER", "ID", "PROTOCOL", "TEMPLATE", "PREFERENCE", "DISABLED",
  "DIRECT", "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE", "STATES",
  "FILTERS", "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS",
  "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR",
  "COMMANDS", "PREEXPORT", "GENERATE", "ROA", "MAX", "FLUSH", "LISTEN",
  "BGP", "V6ONLY", "DUAL", "ADDRESS", "PORT", "PASSWORDS", "DESCRIPTION",
  "RELOAD", "IN", "OUT", "MRTDUMP", "MESSAGES", "RESTRICT", "MEMORY",
  "IGP_METRIC", "SHOW", "STATUS", "SUMMARY", "ROUTE", "SYMBOLS", "ADD",
  "DELETE", "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS", "ATTRIBUTES",
  "ECHO", "DISABLE", "ENABLE", "RESTART", "FUNCTION", "PRINT", "PRINTN",
  "UNSET", "RETURN", "ACCEPT", "REJECT", "QUITBIRD", "INT", "BOOL", "IP",
  "PREFIX", "PAIR", "QUAD", "EC", "SET", "STRING", "BGPMASK", "BGPPATH",
  "CLIST", "ECLIST", "IF", "THEN", "ELSE", "CASE", "TRUE", "FALSE", "RT",
  "RO", "UNKNOWN", "GENERIC", "GW", "NET", "MASK", "PROTO", "SOURCE",
  "SCOPE", "CAST", "DEST", "LEN", "DEFINED", "CONTAINS", "RESET",
  "PREPEND", "FIRST", "LAST", "MATCH", "ROA_CHECK", "EMPTY", "WHERE",
  "EVAL", "LOCAL", "NEIGHBOR", "AS", "HOLD", "CONNECT", "RETRY",
  "KEEPALIVE", "MULTIHOP", "STARTUP", "VIA", "NEXT", "HOP", "SELF",
  "DEFAULT", "PATH", "METRIC", "START", "DELAY", "FORGET", "WAIT", "AFTER",
  "BGP_PATH", "BGP_LOCAL_PREF", "BGP_MED", "BGP_ORIGIN", "BGP_NEXT_HOP",
  "BGP_ATOMIC_AGGR", "BGP_AGGREGATOR", "BGP_COMMUNITY",
  "BGP_EXT_COMMUNITY", "RR", "RS", "CLIENT", "CLUSTER", "AS4", "ADVERTISE",
  "IPV4", "CAPABILITIES", "LIMIT", "PREFER", "OLDER", "MISSING", "LLADDR",
  "DROP", "IGNORE", "REFRESH", "INTERPRET", "COMMUNITIES",
  "BGP_ORIGINATOR_ID", "BGP_CLUSTER_LIST", "IGP", "GATEWAY", "RECURSIVE",
  "MED", "TTL", "SECURITY", "DETERMINISTIC", "OSPF", "AREA",
  "OSPF_METRIC1", "OSPF_METRIC2", "OSPF_TAG", "OSPF_ROUTER_ID",
  "RFC1583COMPAT", "STUB", "TICK", "COST", "COST2", "RETRANSMIT", "HELLO",
  "TRANSMIT", "PRIORITY", "DEAD", "TYPE", "BROADCAST", "BCAST",
  "NONBROADCAST", "NBMA", "POINTOPOINT", "PTP", "POINTOMULTIPOINT", "PTMP",
  "SIMPLE", "AUTHENTICATION", "STRICT", "CRYPTOGRAPHIC", "ELIGIBLE",
  "POLL", "NETWORKS", "HIDDEN", "VIRTUAL", "CHECK", "LINK", "RX", "BUFFER",
  "LARGE", "NORMAL", "STUBNET", "TAG", "EXTERNAL", "LSADB", "ECMP",
  "WEIGHT", "NSSA", "TRANSLATOR", "STABILITY", "GLOBAL", "LSID",
  "TOPOLOGY", "STATE", "PIPE", "PEER", "MODE", "OPAQUE", "TRANSPARENT",
  "RIP", "INFINITY", "PERIOD", "GARBAGE", "TIMEOUT", "MULTICAST", "QUIET",
  "NOLISTEN", "VERSION1", "PLAINTEXT", "MD5", "HONOR", "NEVER", "ALWAYS",
  "RIP_METRIC", "RIP_TAG", "STATIC", "PROHIBIT", "MULTIPATH", "'('", "')'",
  "';'", "':'", "'{'", "'}'", "','", "'?'", "'['", "']'", "$accept",
  "config", "conf_entries", "expr", "definition", "bool", "ipa",
  "ipa_scope", "prefix", "prefix_or_ipa", "pxlen", "datetime",
  "text_or_none", "log_config", "syslog_name", "log_file", "log_mask",
  "log_mask_list", "log_cat", "mrtdump_base", "timeformat_which",
  "timeformat_spec", "timeformat_base", "cmd_CONFIGURE",
  "cmd_CONFIGURE_SOFT", "cmd_DOWN", "cfg_name", "kern_proto_start",
  "kern_item", "kif_proto_start", "kif_item", "nl_item", "rtrid", "idval",
  "listen", "listen_opts", "listen_opt", "newtab", "roa_table_start",
  "roa_table_opts", "roa_table", "proto_start", "proto_name", "proto_item",
  "imexport", "rtable", "debug_default", "iface_patt_node_init",
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
  "ospf_iface_start", "ospf_iface_opts", "ospf_iface_opt_list",
  "ospf_iface", "opttext", "cmd_SHOW_OSPF", "cmd_SHOW_OSPF_NEIGHBORS",
  "cmd_SHOW_OSPF_INTERFACE", "cmd_SHOW_OSPF_TOPOLOGY",
  "cmd_SHOW_OSPF_TOPOLOGY_ALL", "cmd_SHOW_OSPF_STATE",
  "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB", "lsadb_args",
  "pipe_proto_start", "pipe_proto", "rip_cfg_start", "rip_cfg", "rip_auth",
  "rip_mode", "rip_iface_item", "rip_iface_opts", "rip_iface_opt_list",
  "rip_iface_init", "rip_iface", "static_proto_start", "static_proto",
  "stat_route0", "stat_multipath1", "stat_multipath", "stat_route",
  "cmd_SHOW_STATIC", "conf", "cli_cmd", "proto", "kern_proto", "kif_proto",
  "dynamic_attr", 0
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
      40,    41,    59,    58,   123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   330,   331,   331,   332,   332,   333,   333,   333,   334,
     334,   335,   335,   335,   335,   335,   335,   336,   336,   337,
     337,   338,   339,   339,   340,   340,   341,   342,   342,   343,
     344,   344,   345,   345,   345,   346,   346,   347,   347,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   349,   349,
     350,   350,   350,   350,   351,   351,   351,   351,   352,   353,
     354,   355,   356,   356,   357,   358,   358,   358,   358,   359,
     360,   360,   361,   362,   363,   363,   363,   364,   365,   365,
     366,   366,   366,   366,   367,   368,   369,   369,   370,   370,
     371,   371,   372,   372,   372,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   374,   374,   374,   374,   375,
     376,   376,   377,   378,   378,   378,   379,   379,   380,   381,
     381,   382,   383,   383,   383,   384,   385,   386,   386,   386,
     387,   387,   388,   388,   388,   388,   388,   388,   389,   389,
     389,   390,   390,   391,   391,   392,   392,   393,   393,   394,
     394,   395,   396,   396,   396,   396,   396,   396,   397,   398,
     399,   400,   401,   401,   402,   403,   404,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   406,
     406,   407,   408,   408,   408,   408,   409,   409,   409,   410,
     411,   411,   411,   411,   411,   411,   411,   411,   412,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   424,   424,   425,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   435,   435,   436,   436,
     436,   438,   437,   439,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   441,   442,   442,
     443,   443,   444,   445,   445,   446,   447,   447,   448,   450,
     449,   451,   451,   452,   452,   453,   453,   454,   455,   455,
     455,   455,   456,   456,   456,   456,   456,   457,   458,   458,
     458,   459,   459,   460,   461,   461,   461,   461,   462,   462,
     462,   463,   463,   463,   463,   464,   464,   464,   464,   465,
     465,   466,   466,   467,   468,   468,   468,   468,   469,   469,
     470,   470,   470,   471,   471,   472,   472,   473,   473,   473,
     473,   473,   474,   474,   474,   475,   475,   475,   475,   475,
     475,   475,   475,   475,   475,   475,   476,   476,   477,   478,
     479,   480,   480,   480,   480,   480,   480,   480,   480,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   481,   481,   481,   481,   482,   482,   482,   482,
     482,   482,   483,   484,   484,   484,   485,   485,   486,   486,
     487,   487,   487,   487,   487,   487,   487,   487,   487,   487,
     487,   487,   487,   487,   487,   487,   488,   489,   489,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   490,
     491,   491,   492,   492,   492,   492,   492,   492,   493,   494,
     495,   495,   496,   496,   496,   496,   496,   496,   496,   496,
     496,   497,   496,   498,   496,   496,   496,   496,   499,   499,
     500,   501,   501,   502,   502,   502,   503,   503,   504,   504,
     505,   505,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   506,   507,   507,   507,   507,   507,   507,   507,   507,
     507,   507,   507,   507,   507,   507,   507,   507,   507,   507,
     507,   507,   507,   507,   507,   507,   507,   507,   507,   507,
     507,   508,   508,   509,   510,   511,   511,   511,   512,   512,
     513,   513,   514,   515,   516,   517,   517,   518,   518,   519,
     520,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   529,   529,   529,   529,   529,   529,   529,   529,   530,
     531,   531,   531,   531,   531,   532,   533,   533,   533,   533,
     533,   533,   533,   533,   533,   533,   533,   533,   533,   534,
     534,   534,   535,   535,   535,   535,   535,   536,   536,   536,
     537,   537,   538,   538,   539,   540,   541,   542,   542,   542,
     542,   542,   543,   544,   544,   545,   545,   546,   546,   546,
     546,   546,   546,   546,   547,   548,   548,   548,   548,   548,
     548,   548,   548,   548,   548,   548,   548,   548,   548,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     550,   550,   550,   550,   550,   550,   550,   550,   551,   551,
     551,   551,   552,   552,   552,   553,   553,   553,   553,   553,
     553,   553,   553,   553,   553,   553,   553,   553,   553,   553,
     553,   553,   553,   553,   553,   553
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
       2,     2,     1,     1,     2,     3,     0,     8,     1,     4,
       1,     1,     0,     1,     3,     0,     2,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     1,     1,     1,     1,
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
       5,     4,     5,     4,     5,     4,     5,     5,     5,     2,
       3,     3,     1,     2,     2,     4,     2,     1,     2,     4,
       0,     3,     2,     1,     2,     3,     3,     3,     3,     2,
       3,     0,     5,     0,     5,     2,     2,     1,     4,     1,
       1,     0,     3,     2,     2,     2,     4,     1,     0,     3,
       0,     2,     2,     3,     2,     2,     3,     2,     2,     2,
       1,     3,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     2,     3,
       2,     3,     3,     4,     2,     2,     2,     3,     3,     3,
       1,     0,     2,     3,     1,     0,     1,     2,     0,     2,
       1,     1,     2,     3,     0,     0,     3,     0,     3,     3,
       1,     0,     4,     6,     6,     6,     7,     6,     7,     5,
       0,     2,     3,     2,     3,     3,     2,     3,     2,     2,
       3,     3,     5,     4,     4,     2,     3,     3,     4,     4,
       4,     5,     5,     4,     3,     4,     4,     4,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     2,     2,
       0,     3,     0,     3,     0,     3,     2,     3,     3,     5,
       5,     3,     2,     3,     3,     1,     2,     4,     3,     3,
       3,     2,     2,     2,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    62,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   649,
     650,   651,   652,   653,   654,   655,   656,   657,   658,   659,
     660,   661,   662,   663,   664,   665,   666,   667,   668,   669,
     670,   671,   672,   673,   674,   675,   676,   677,   678,   679,
     680,   681,   682,   683,   684,   685,   686,   687,   688,   689,
       3,     1,     2,     0,     0,     0,     0,     0,     0,    90,
      91,     0,     0,     0,     0,     0,   338,   635,   636,   637,
     638,   639,    92,    92,   640,   641,   642,    88,   643,     0,
     645,    92,    95,   646,   647,   648,    92,    95,    92,    95,
      92,    95,    92,    95,    92,    95,     5,   644,    95,    95,
     225,   227,   226,     0,    63,    62,     0,    61,   198,     0,
       0,     0,     0,   224,   229,     0,   182,     0,     0,   167,
     190,   163,   163,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   212,   211,   214,     0,     0,     0,     0,
      32,    31,    34,     0,     0,     0,    53,    52,    51,    50,
       0,     0,    84,     0,   231,     0,    78,     0,     0,   259,
     321,   325,   334,   331,   267,   340,   328,     0,     0,   324,
     338,   368,     0,     0,     0,   326,   327,     0,     0,     0,
     338,     0,   329,   330,   335,   366,   367,     0,   365,   233,
      93,     0,     0,    86,    64,    69,   121,   416,   459,   579,
     585,   616,     0,     0,     0,     0,     0,    16,   125,     0,
       0,     0,     0,   692,     0,     0,     0,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,   693,     0,     0,
       0,    16,     0,    16,   694,   462,     0,     0,   467,     0,
       0,     0,   695,     0,     0,   614,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   696,     0,     0,   146,   150,
       0,     0,     0,     0,   697,     0,     0,     0,     0,    16,
       0,    16,     0,   690,     0,     0,     0,     0,    28,   691,
       0,     0,   128,   127,     0,     0,     0,    59,     0,     0,
       0,     0,   219,   139,   138,     0,     0,   228,   230,   229,
       0,   164,     0,   186,   159,   158,     0,     0,   162,   163,
     163,   570,   163,   163,     0,     0,    17,    18,     0,     0,
       0,   208,   209,   205,   203,   204,   206,   207,   215,     0,
     216,   217,   218,     0,     0,    33,    35,     0,     0,   110,
       6,     8,   338,   111,    54,     0,    58,    74,    75,    76,
       0,   248,    85,     0,    49,     0,     0,   321,   340,   321,
     338,   321,   321,     0,   313,     0,   338,     0,     0,     0,
     324,   324,     0,   363,   338,   338,   338,   338,   338,     0,
     284,   285,     0,   287,     0,     0,   271,   269,   338,   268,
     270,   293,   291,   292,   299,     0,   304,   308,     0,   708,
     705,   706,   341,   707,   342,   343,   344,   345,   346,   347,
     348,   710,   713,   712,   709,   711,   714,   715,   716,   719,
     717,   718,   720,   721,   722,   723,   724,   725,   369,   370,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,     0,     0,   698,   702,     0,   122,    98,
     109,   102,     0,    96,    12,    14,    13,    15,    11,    97,
     112,   107,   248,   108,   338,   100,   106,   101,   104,    99,
     123,   124,   417,     0,     0,     0,     0,     0,     0,     0,
      16,     0,     0,     0,    19,     0,     0,     0,   429,     0,
       0,     0,     0,     0,    16,     0,     0,     0,     0,    16,
       0,    16,     0,    16,     0,    16,     0,     0,    16,    16,
      16,   418,   460,   468,   463,   466,   464,   461,   470,   580,
       0,     0,     0,   581,   586,   112,     0,   151,     0,   147,
     601,   599,   600,     0,     0,     0,     0,     0,     0,     0,
       0,   587,   594,   152,   617,   622,     0,    16,   618,   632,
       0,   631,     0,   633,     0,   621,     0,    65,     0,    67,
      16,   700,   701,   699,     0,    27,     0,   704,   703,   133,
     132,   134,   136,   137,   135,     0,   130,   222,    60,   199,
     202,   220,   221,   143,   144,     0,   141,   223,     0,   160,
     165,   181,     0,   188,   187,     0,     0,   166,   173,     0,
       0,   180,   248,   174,   177,   178,     0,   179,   168,     0,
     172,   189,   197,   191,   194,   195,   193,   196,   192,   561,
     561,     0,   163,   561,   163,   561,   562,   634,     0,     0,
      21,     0,     0,   210,     0,     0,    30,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    37,    29,     0,
       0,    56,    57,    73,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,     0,     0,     0,   232,
     252,    82,    83,     0,     0,    77,    79,    48,     0,   248,
     317,   318,     0,   319,   320,   315,   303,   396,   399,     0,
     376,     0,   377,   322,   323,   316,     0,     0,     0,     0,
       0,     0,   286,   338,   349,   338,   280,   278,     0,     0,
     277,     0,     0,   332,   305,   306,     0,     0,   333,   361,
     359,   357,   354,   355,   356,   358,   360,   362,   350,   351,
     352,   353,   371,     0,   372,   374,   375,    94,     0,    89,
     103,   116,   119,   126,   253,   254,   105,   255,     0,     0,
     453,   455,     0,    16,    16,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   430,     0,     0,     0,     0,     0,
       0,   423,     0,   424,     0,   451,     0,     0,     0,     0,
       0,     0,     0,   435,   436,     0,     0,     0,     0,     0,
       0,   583,   584,   612,   598,   589,     0,     0,   593,   588,
     590,     0,     0,   596,   597,   595,     0,     0,     0,     0,
       0,     0,   628,    19,   630,     0,   625,   629,    72,    66,
      68,    70,    23,    22,    71,   129,     0,   140,     0,   161,
     185,   184,   183,   170,   176,   171,   169,   175,   560,     0,
       0,   569,   578,     0,   576,     0,     0,   573,   571,     0,
     561,     0,   561,     0,    24,    25,     0,     0,    10,     9,
      36,     0,     7,    55,   247,   246,   248,   338,    80,    81,
     257,   250,     0,   260,   314,   338,   385,     0,   338,   338,
     338,   364,   338,   383,   338,     0,     0,   338,   338,   338,
     267,   294,   300,     0,     0,   309,   338,     0,   117,     0,
     112,     0,     0,   454,     0,     0,   450,   444,   419,     0,
      20,     0,   425,     0,   428,     0,   431,   443,   442,   437,
       0,     0,   452,   440,   432,   433,   434,   456,   457,   439,
     438,   458,   441,   465,   554,    16,     0,    16,   481,     0,
       0,   483,   473,    16,   469,     0,   487,   497,   582,   610,
     615,   145,   147,   591,   592,     0,     0,     0,     0,     0,
     149,   620,   619,   627,    19,     0,   626,   131,   142,   564,
     563,   577,   572,   574,   575,     0,   565,     0,   567,     0,
       0,    38,   249,     0,   389,     0,   390,   391,     0,   338,
     387,   388,   386,   338,   338,     0,   338,     0,     0,   338,
     263,     0,   256,   397,   378,     0,     0,     0,     0,     0,
     338,   336,   279,   277,     0,     0,   283,     0,     0,     0,
     113,   114,   118,   120,   446,     0,   448,   449,   420,     0,
     427,   426,   445,   422,   112,   486,   474,     0,     0,    16,
       0,   472,     0,     0,   490,   485,   489,     0,     0,   479,
     471,   498,   607,   148,   152,    26,     0,     0,     0,     0,
     623,   624,   566,   568,   198,   198,   338,   338,   338,   338,
       0,     0,     0,   258,   264,     0,     0,   409,   392,   394,
       0,   251,   382,   380,   381,   379,   338,     0,   281,   338,
       0,   373,     0,   115,     0,   421,   557,   476,   477,   475,
     478,   541,   511,   491,   541,   480,   500,     0,     0,   613,
       0,   157,   152,   152,   152,   152,     0,     0,     0,     0,
       0,     0,   403,   338,   310,   338,   338,     0,   338,   408,
       0,   337,     0,     0,     0,   307,     0,   447,   555,   559,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   496,
     510,     0,   608,   602,   603,   604,   605,   606,   609,   611,
     153,   154,   155,   156,   200,   201,   402,   339,   406,     0,
     338,   400,   265,     0,     0,     0,     0,     0,     0,     0,
       0,   395,   384,   338,   290,   338,   288,     0,     0,   482,
     544,   542,   545,    16,     0,    16,   488,     0,   484,   504,
     502,   501,     0,     0,   505,   507,   508,   509,   499,     0,
       0,   338,   338,   272,   276,   274,   338,   410,   275,   297,
     295,   296,   301,     0,   405,   404,   338,   338,   338,   338,
     411,     0,     0,    87,     0,     0,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     558,   540,     0,   546,     0,     0,   494,   495,   493,   492,
     503,   506,   407,   266,   401,   312,   277,     0,   338,     0,
       0,     0,     0,     0,   338,   289,   548,   516,   530,   512,
     515,   513,     0,   528,     0,   517,   519,   520,   521,   522,
     523,   524,   525,   526,   534,   535,   536,    16,   514,    16,
       0,     0,   556,   547,   543,   273,   338,   298,   311,   302,
       0,     0,     0,     0,     0,     0,   527,   518,   529,   531,
     537,   538,   539,   532,     0,   415,   413,   414,   412,   282,
       0,   533,   549,   550,   551,     0,   552,   553
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   488,    78,   489,   348,   790,   853,   854,
     660,  1086,   596,    79,   365,   153,   368,   676,   677,    80,
     160,   161,    81,    19,    20,    21,   116,    82,   304,    83,
     310,   305,    84,   380,    85,   383,   706,    86,    87,   477,
      88,    89,   201,   224,   495,   481,    90,   771,  1052,   929,
     772,   773,    91,    92,   490,   225,   315,   605,   606,   326,
     615,   616,   287,   826,   288,   289,   839,    22,    23,    24,
      25,   344,    26,    27,    28,   336,   639,    29,   333,   626,
      30,   337,   319,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,   145,   359,    42,    43,    44,    45,
      46,    47,    48,    49,    50,   113,   330,    93,   381,    94,
     696,   697,   698,   902,   775,   776,   496,   709,   700,    95,
     386,  1025,  1026,  1201,   192,   421,  1249,   737,   738,   422,
    1045,   739,   423,   424,  1252,   425,  1253,   193,   427,   428,
    1203,   392,   194,   393,   402,   195,   196,   197,  1028,   198,
     458,  1043,  1029,  1109,  1110,   718,   719,  1030,    96,    97,
      98,    99,   266,   267,   268,   819,   975,  1072,  1077,  1075,
    1076,  1171,  1227,   976,  1136,  1181,   977,  1282,  1170,  1221,
    1222,  1285,  1345,  1362,  1363,  1364,  1064,  1218,  1169,  1065,
     869,    51,    52,    53,    54,    55,    56,    57,    58,   651,
     100,   101,   102,   103,   563,  1188,  1140,  1082,   980,   555,
     556,   104,   105,   296,   846,   847,   297,    59,   106,    60,
     107,   108,   109,   459
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1152
static const yytype_int16 yypact[] =
{
     102,  2245,   132,    15,   403,   247,   196,   103,   535,   403,
     226,  1702,   154,   216,   771,   367,   403,   403,   403, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152,   361,   333,   267,   389,   407,   288, -1152,
   -1152,   413,   379,   370,   110,   498,  1315, -1152, -1152, -1152,
   -1152, -1152,   508,   508, -1152, -1152, -1152,   207, -1152,   640,
   -1152,   508,  2090, -1152, -1152, -1152,   508,  1907,   508,  1829,
     508,  1927,   508,  1977,   508,  1901, -1152, -1152,  1182,  2042,
   -1152, -1152, -1152,    33, -1152,   533,   558, -1152,   500,   403,
     403,   628,    57, -1152,   579,   130, -1152,   637,   647, -1152,
   -1152,   482,   639,   259,   259,   670,   686,   700,   712,   718,
     729,   736, -1152, -1152, -1152,   728,   750,   753,   754,   738,
   -1152,   705, -1152,    54,    33,     5, -1152, -1152, -1152, -1152,
     122,   442, -1152,   653, -1152,   747, -1152,   447,    57, -1152,
       7, -1152, -1152, -1152,   749,   460, -1152,   599,    19,    13,
    1315, -1152,   463,   466,   470, -1152, -1152,   471,   474,   478,
     732,   138, -1152, -1152, -1152, -1152, -1152,  1717, -1152,  1849,
     698,   475,   476, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152,   477,    33,   768,   734,     5,   161, -1152,    26,
      26,   791,    57, -1152,   490,   492,   493,   381,   800,   161,
     598,   615,    56,   717,    36,   259,   763,   644,   764,    32,
     641,   631,   278,   630,   638,   319,   621,   618,   161,   619,
     616,   613,     3,    10,   657,   607,   620, -1152,   538,   537,
     653,   161,     5,   161, -1152, -1152,   542,   549, -1152,   550,
     811,   302, -1152,   563,   565, -1152,   870,     5,   567,    35,
       5,     5,   839,   840,   -21, -1152,   582,   584, -1152,   587,
     588,   259,   845,   642, -1152,   586,   303,   593,   848,   161,
     854,   161,   852, -1152,   601,   603,   604,   856,   907, -1152,
     606,   609, -1152, -1152,   713,   930,   931, -1152,   916,   936,
     937,   939, -1152, -1152, -1152,   166,   941, -1152, -1152,   597,
     943, -1152,   946,    81, -1152, -1152,  2122,   926, -1152,   639,
     639, -1152,   107,   284,   947,   948, -1152, -1152,    -4,   851,
     853, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,   952,
   -1152, -1152, -1152,    84,   938, -1152, -1152,  1069,   634, -1152,
   -1152, -1152,  1315, -1152,     5,   580, -1152, -1152, -1152, -1152,
     649,  2265, -1152,   115, -1152,   651,   655,     7, -1152,     7,
    1315,     7,     7,   945, -1152,   963,  1077,   961,   777,   935,
      13,    13,   959,   958,  1315,  1315,  1315,  1315,  1315,   973,
   -1152, -1152,   978, -1152,   669,    53, -1152, -1152,   610, -1152,
   -1152,   990, -1152, -1152, -1152,   171,    17, -1152,   268, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
    1315,  1315,  1315,  1315,  1315,  1315,  1315,  1315,  1315,  1315,
    1315,  1315,  1315,   636,   982, -1152, -1152,   -73, -1152, -1152,
   -1152, -1152,   653, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152,  2123, -1152,  1315, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152,   940,   942,   680,   688,     5,   962,   778,
     161,   259,     5,   823,   984,     5,   953,     5, -1152,   693,
     955,   821,     5,     5,   161,   960,   702,   965,   711,   161,
     719,   161,   285,   161,   768,   161,   720,   722,   161,   161,
     161, -1152, -1152, -1152, -1152, -1152,   822, -1152, -1152, -1152,
    1016,   723,   726, -1152, -1152, -1152,   737, -1152,   740,   966,
   -1152, -1152, -1152,   741,   742,   744,     5,     5,   759,   761,
     766, -1152, -1152,   298, -1152, -1152,   768,   161, -1152, -1152,
     526, -1152,   259, -1152,   863, -1152,     5, -1152,     5, -1152,
     161, -1152, -1152, -1152,     5, -1152,   259, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152,   330, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152,   357, -1152, -1152,  1072, -1152,
   -1152, -1152,  1057, -1152, -1152,  1070,   259, -1152, -1152,  1082,
    1086, -1152,  2123, -1152, -1152, -1152,   259, -1152, -1152,  1101,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,  1071,
    1071,   806,   639,  1071,   639,  1071, -1152, -1152,     5,   259,
   -1152,  1074,  1109, -1152,   805,   815, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152,   398, -1152, -1152,  1045,
    1108, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152,    76,   818,   817, -1152,
   -1152, -1152, -1152,   259,     5, -1152, -1152, -1152,  2028,  2265,
   -1152, -1152,  1258, -1152, -1152, -1152, -1152,   162, -1152,   825,
   -1152,  1121, -1152, -1152, -1152, -1152,   233,   262,   441,  1352,
     467,   113, -1152,  1315, -1152,  1315, -1152,  1142,   824,   827,
    1045,   190,   394, -1152, -1152, -1152,  1136,  1137, -1152,  2364,
    2364,  2364,  2375,  2375,  2364,  2364,  2364,  2364,   511,   511,
     958,   958, -1152,   834, -1152, -1152, -1152, -1152,   259, -1152,
   -1152,  1129, -1152,   831, -1152, -1152, -1152,  1849,     5,     5,
   -1152, -1152,   837,   161,   161,   838,   841,   842,     5,  1143,
     979,   847,     5,   857, -1152,     5,   864,   865,   866,   867,
       5, -1152,   653, -1152,   868, -1152,   871,   872,   873,   874,
     876,   877,   878, -1152, -1152,   882,   896,   898,     5,   687,
     900, -1152, -1152,    91, -1152, -1152,   836,   902, -1152, -1152,
   -1152,   912,   914, -1152, -1152, -1152,     5,   416,   484,   908,
     917,   923, -1152,   984, -1152,   259,   904,   863, -1152, -1152,
   -1152, -1152,    -4, -1152, -1152, -1152,   713, -1152,   166, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,  1245,
    1246, -1152, -1152,   653, -1152,   653,  1236, -1152, -1152,   653,
    1071,  1251,  1071,  1253, -1152, -1152,  1075,  1076, -1152, -1152,
   -1152,  1069, -1152, -1152, -1152, -1152,  2265,  1726, -1152, -1152,
   -1152,   950,   954, -1152, -1152,  1315, -1152,  1247,  1315,  1315,
    1315, -1152,  1315, -1152,  1315,   497,  1398,  1315,  1116,  1315,
   -1152, -1152, -1152,   967,   749, -1152,  1315,  1172, -1152,   668,
   -1152,   956,   969, -1152,   975,   977, -1152, -1152, -1152,   980,
   -1152,     5, -1152,   981, -1152,   991, -1152, -1152, -1152, -1152,
     994,   995, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152,   161,   282,    86, -1152,  1010,
     259, -1152, -1152,   109, -1152,  1000, -1152,   976, -1152, -1152,
   -1152, -1152,   966, -1152, -1152,  1002,  1269,  1269,  1269,  1269,
   -1152, -1152, -1152, -1152,   984,     5,   904, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152,  1298, -1152,  1301, -1152,  1291,
    1310, -1152, -1152,    -5, -1152,  1304, -1152, -1152,  1007,  1315,
   -1152, -1152, -1152,  1315,  1315,  1004,    23,  1717,  1014,  1190,
   -1152,  2265, -1152, -1152, -1152,  1437,  1476,  1505,  1530,   555,
    1315, -1152, -1152,  1849,  1017,  1011,  1849,  1324,  1558,  1325,
     259, -1152, -1152, -1152, -1152,     5, -1152, -1152, -1152,  1021,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152,     5,     5,   161,
       5, -1152,  1023,   653, -1152, -1152,  1024,  1025,     5, -1152,
   -1152, -1152,   100, -1152,   298, -1152,  1030,  1031,  1032,  1033,
   -1152, -1152, -1152, -1152,   500,   500,  1315,  1077,  1315, -1152,
     652,  2304,   581, -1152, -1152,  1323,   439, -1152,  1849,  1043,
    1034, -1152, -1152, -1152, -1152, -1152,  1315,  1599,  1350,  1292,
    1046, -1152,  1170, -1152,  1037, -1152,   250, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152,   464,     5,   148, -1152,
    1048, -1152,   298,   298,   298,   298,  1379,  1380,   684,  1064,
     843,  1598, -1152,  1460, -1152,  1315,  1315,   335,  1190, -1152,
    1628, -1152,  1066,  1067,    12, -1152,  1372, -1152, -1152, -1152,
      14,    30,    31,     5,     5,     5,  1186,   249,   165, -1152,
   -1152,  1073, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,  1078,
    1726,  1233, -1152,   121,   957,  1203,  1080,  1081,  1083,  1084,
    1089, -1152, -1152,  1315, -1152,  1315, -1152,  1090,  1986, -1152,
   -1152, -1152,   -92,   161,     5,   161, -1152,  1091, -1152, -1152,
   -1152, -1152,     5,     5, -1152, -1152, -1152, -1152, -1152,  1092,
    1068,  1460,    94, -1152, -1152, -1152,   610, -1152, -1152,  1385,
   -1152, -1152, -1152,   289, -1152, -1152,  1315,  1315,  1315,  1315,
   -1152,  1093,  1094, -1152,  1104,     5,   161,     5,     5,     5,
    1191,     5,   280,   911,   265,  1140,     5,  1151,  1138,  1144,
   -1152, -1152,  1111, -1152,     5,  1112, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152,  1653,    42,    94,   222,
    1678,  1703,  1728,  1753,  1315, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152,     5, -1152,     5, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152,   161, -1152,   161,
     384,     5, -1152, -1152, -1152, -1152,  1315, -1152, -1152, -1152,
    1113,  1115,  1119,  1120,  1110,    -3, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152,  1653, -1152, -1152, -1152, -1152, -1152,
    -181, -1152, -1152, -1152, -1152,  1128, -1152, -1152
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1152, -1152, -1152,  -146, -1152,  -227,  -231,  -813,  -123,  -620,
   -1152,  -258, -1152, -1152, -1152, -1152, -1152, -1152,   547, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152,  1328, -1152, -1152, -1152,
   -1152, -1152, -1152,  -255, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152,   934,   779,  1231,  -409, -1152, -1152, -1152, -1152,
     524,  -548, -1152, -1152, -1152, -1152,   146, -1152,   600,    44,
   -1152,   614, -1099,   473,  -547, -1152,  -417, -1152, -1152, -1152,
   -1152,  -124, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152,  -358, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152,   635,  1139, -1152, -1152, -1152,
   -1152,  -695,   561,   427,  1096,   846,  1135, -1152,   772, -1152,
   -1152,  -925, -1152,   232,  -190,   739,   185,  -903,   571, -1151,
    -908,  1293,  -868,   752,   192, -1152, -1152,  -185,   745, -1152,
   -1152, -1152, -1152,   310,   341, -1152, -1152,  -874, -1152,    93,
     468,   -76, -1152, -1152,   338,   617,   412,  -995, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,   377, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
    -332, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152, -1152,   672, -1152, -1152, -1152, -1152, -1152,
   -1152, -1152, -1152,  -937
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -399
static const yytype_int16 yytable[] =
{
     199,   420,   506,   513,   514,   543,   426,   823,   345,   373,
     349,   350,   827,   901,  1042,  1360,   866,  1096,    62,  1215,
     370,   530,   387,  1027,   371,   658,   388,   768,   400,  -262,
     993,  1104,   346,   347,   544,   389,   546,  1180,  -262,  -262,
    -262,  -262,  1013,   744,   745,   419,   398,   370,    63,   346,
     347,   371,  1250,    64,   346,   347,    65,  1243,  1244,  1245,
     920,   460,   461,   462,   463,   464,   491,    66,   312,  1014,
     483,   534,   587,   313,   589,   465,   466,   467,   468,   469,
     470,   471,   472,    67,   621,   473,    68,   536,    69,    70,
    1106,  1365,   323,   519,   366,   894,    71,   324,  1015,   370,
    -261,   370,   664,   371,   403,   371,     1,   492,   493,  -261,
    -261,  -261,  -261,  1013,   415,    72,   545,   560,    73,  1281,
     484,   485,   486,   487,   370,   811,   338,  1242,   371,    74,
     167,   558,    61,   331,   564,   565,  1243,  1244,  1245,   920,
    1014,  1366,   374,   484,   485,   486,   487,   652,  1250,   622,
      75,  1223,  1027,   370,   416,   417,   174,   371,  1202,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,   568,   840,   575,  1015,
     460,   461,   462,   463,   464,   375,   370,   623,   509,  1023,
     371,  1090,  1024,  1283,   465,   466,   467,   468,   469,   470,
     471,   472,  1284,   624,   473,   484,   485,   486,   487,   117,
      76,   168,   399,   118,   535,   370,   416,   417,   920,   371,
     494,  1164,   385,   638,  1199,   649,   650,   665,   653,   655,
     701,   702,   703,   704,   512,  1151,   895,   770,   680,   123,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1243,  1244,  1245,
     920,   460,   461,   462,   463,   464,  1202,  1235,   537,   613,
    1023,   332,   769,  1024,   133,   465,   466,   467,   468,   469,
     470,   471,   472,   394,   370,   473,   499,   114,   371,   625,
     460,   461,   462,   463,   464,  1240,   510,   346,   347,  1027,
     786,   614,  1224,   785,   465,   466,   467,   468,   469,   470,
     471,   472,   569,   570,   473,   370,   679,   799,   115,   371,
     369,  1137,   804,   338,   806,  1225,   810,  1262,   812,  1051,
    1261,   815,   816,   817,   712,  1097,   134,  1295,   870,   659,
     717,   881,  1361,   883,   654,   372,  1027,   390,   726,   727,
     728,   729,   730,  1216,   391,  1251,   901,    77,  1070,  1219,
     401,   746,   740,  -262,  1233,   561,   562,  1324,  -262,   843,
     841,   844,   372,   150,   518,  1226,  1228,   314,   154,   479,
     163,   782,  1336,   850,   155,   852,   787,  1027,  1027,   791,
     836,   793,   151,  1338,   734,  1314,   797,   798,   367,   735,
     149,   325,   142,   152,   749,   750,   751,   752,   753,   754,
     755,   756,   757,   758,   759,   760,   761,   837,  1138,   370,
    1078,  1344,   143,   371,   372,   852,   372,   144,  1183,   370,
     416,   417,   920,   371,  -261,   979,  1206,   930,   777,  -261,
     831,   832,   110,   111,  1027,  1139,   162,   156,   885,   372,
    1123,  1251,   164,  1236,   913,   827,  1237,   705,   838,   914,
     848,  1246,   849,   112,   579,   157,  1247,   165,   851,   460,
     461,   462,   463,   464,  1184,  1185,  1186,  1187,   418,  1207,
    1208,  1156,   158,   465,   466,   467,   468,   469,   470,   471,
     472,  1157,   898,   473,   166,   460,   461,   462,   463,   464,
     394,   372,   394,   807,   394,   394,   522,   523,   905,   465,
     466,   467,   468,   469,   470,   471,   472,   742,   580,   473,
     743,   338,   986,   862,   987,   460,   461,   462,   463,   464,
     372,   159,   884,  1209,   808,   809,  1126,   169,  1210,   465,
     466,   467,   468,   469,   470,   471,   472,   200,   880,   473,
     882,   203,   581,  1325,  1067,  1068,  1326,   526,   527,   471,
     472,   582,  1246,   473,   346,   347,   842,   951,  1005,   276,
    1007,   420,   420,   114,   110,   111,   934,   935,   899,   908,
     339,   317,   426,   460,   461,   462,   463,   464,   318,   372,
     988,  1069,   989,   278,  1168,   112,   930,   465,   466,   467,
     468,   469,   470,   471,   472,   503,   504,   473,   909,   460,
     461,   462,   463,   464,   747,   419,   419,   748,   327,   328,
     372,   551,   552,   465,   466,   467,   468,   469,   470,   471,
     472,   340,  1298,   473,   994,  1299,   327,   328,  1001,   329,
    1002,   583,   584,   170,  1004,   171,   172,   173,   174,   175,
     176,   322,   931,   932,   681,   682,   177,   178,   736,   179,
     334,   180,   939,   121,   122,   927,   943,   119,   120,   945,
     335,   146,   147,   148,   950,   855,   856,   915,   338,   916,
     460,   461,   462,   463,   464,  1350,  1351,  1141,   377,  1173,
     378,   379,   963,   351,   465,   466,   467,   468,   469,   470,
     471,   472,   857,   858,   473,   181,   346,   347,  1050,   352,
     985,   182,   460,   461,   462,   463,   464,   710,   852,   711,
     204,   713,   714,   353,   372,   205,   465,   466,   467,   468,
     469,   470,   471,   472,   418,   354,   473,   206,  1174,  1175,
    1176,   355,  1177,   890,   891,  1190,  1191,  1192,  1193,  1087,
    1088,  1089,   356,  1178,   183,   184,  1146,  1147,  1066,   357,
    1071,   723,   724,   358,   207,   170,  1079,   171,   172,   173,
     174,   175,   176,   360,   320,   321,   361,   362,   177,   178,
     363,   179,   364,   180,   376,   964,   382,   910,   341,   384,
     185,   186,   410,   411,   412,   413,   342,   343,   395,   599,
     396,   762,   397,   404,   474,   187,   405,   480,   188,  1179,
     406,   407,   189,   912,   408,  1059,   600,   601,   409,   475,
     476,   478,   602,   603,   763,   604,   482,   181,   965,   871,
     764,   498,   500,   182,   501,   765,   766,   502,  1132,   852,
     505,   508,   507,  1040,   511,   872,   515,   517,   521,   717,
     520,   524,  1035,  1036,  1037,   516,  1038,   135,  1039,   528,
     529,   525,  1129,  1046,   532,   531,   533,  1074,   539,  1091,
    1048,   460,   461,   462,   463,   464,   183,   184,   538,   540,
     541,   542,   136,   137,   547,   465,   466,   467,   468,   469,
     470,   471,   472,   548,   549,   473,   258,   873,   265,   550,
     273,  1116,   286,   208,   295,   553,   966,   306,   311,   554,
     557,   559,   185,   186,   410,   411,   412,   413,   138,   139,
     140,   141,   566,   567,   571,  1154,   572,   187,   578,  1124,
     188,   573,   574,   576,   189,   585,   586,   588,   590,   594,
     577,  1127,  1128,   591,  1130,   592,   593,   595,   597,   641,
     190,   598,  1135,   607,   608,   609,   209,   967,   191,   610,
     611,   210,   612,  1100,   617,   642,   619,  1101,  1102,   620,
     656,   657,   661,  1108,   662,   663,   678,   211,   666,   715,
     721,   968,   722,   969,  1117,   460,   461,   462,   463,   464,
     970,   683,   971,   707,  1152,   708,   972,   973,   716,   465,
     466,   467,   468,   469,   470,   471,   472,   720,   725,   473,
     473,  1182,   731,   732,   643,   733,  1286,   741,  1288,   644,
     645,   767,   780,   778,   874,   779,  1196,   646,   783,   784,
     781,   788,   974,  1248,   789,   794,   792,   202,   795,   796,
    1148,   717,  1150,   800,   801,   212,   647,  1229,  1230,  1231,
     226,  1234,   259,   803,   269,   820,   274,   802,   290,  1308,
    1160,   805,   813,  1046,   814,   821,   818,  1220,   822,  1220,
     875,   276,   190,   460,   461,   462,   463,   464,   845,   824,
     191,   648,   825,   828,   829,   876,   830,   465,   466,   467,
     468,   469,   470,   471,   472,   859,   860,   473,  1287,  1204,
    1205,   833,  1108,   834,   877,   861,  1290,  1291,   835,   886,
     170,   868,   171,   172,   173,   174,   175,   176,   878,   879,
    1348,   863,  1349,   177,   178,   864,   179,  1248,   180,  1248,
     667,   668,   669,   670,   671,   672,   673,   674,   675,  1307,
     867,  1309,  1310,  1311,   887,  1313,  1315,   888,   893,   170,
    1328,   171,   172,   173,   174,   175,   176,   889,  1333,  1046,
     896,   897,   177,   178,   736,   179,   906,   180,   907,   917,
     918,   923,   181,   919,   926,   924,   928,   930,   182,   933,
     936,   981,   940,   937,   938,  1198,  1346,   941,  1347,   942,
    1296,  1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,   944,
    1300,  1301,  1302,  1303,  1352,  1353,   946,   947,   948,   949,
     952,   181,   995,   953,   954,   955,   956,   182,   957,   958,
     959,   183,   184,   170,   960,   171,   172,   173,   174,   175,
     176,   460,   461,   462,   463,   464,   177,   178,   961,   179,
     962,   180,   978,   213,   982,   465,   466,   467,   468,   469,
     470,   471,   472,   990,   983,   473,   984,   185,   186,   991,
     183,   184,   298,   299,   300,   992,   301,   302,   999,  1000,
     214,  1003,   187,   215,  1006,   188,  1008,   216,   217,   189,
    1354,   219,   220,  1009,  1010,   181,   460,   461,   462,   463,
     464,   182,  1031,  1049,  1034,  1032,   185,   186,  1054,  1254,
     465,   466,   467,   468,   469,   470,   471,   472,  1073,  1085,
     473,   187,   221,  1047,   188,  1055,   222,  1056,   189,  1057,
    1081,  1092,  1058,  1060,  1093,   170,  1094,   171,   172,   173,
     174,   175,   176,  1061,   183,   184,  1062,  1063,   177,   178,
    1163,   179,  1080,   180,  1084,  1095,  1098,  1099,   170,  1103,
     171,   172,   173,   174,   175,   176,  1107,  1119,  1118,  1120,
    1122,   177,   178,  1125,   179,  1155,   180,  1131,  1133,  1134,
     185,   186,  1142,  1143,  1144,  1145,  1159,  1162,  1166,  1167,
     460,   461,   462,   463,   464,   187,   892,   181,   188,  1158,
    1189,  1165,   189,   182,   465,   466,   467,   468,   469,   470,
     471,   472,  1194,  1195,   473,  1197,  1213,  1217,  1214,  1232,
     181,  1241,  1297,  1293,  1312,  1238,   182,   190,  -398,  1239,
    1256,  1257,  1327,  1258,  1259,   191,   460,   461,   462,   463,
     464,  1260,  1263,  1289,  1292,  1305,   183,   184,  1330,  1304,
     465,   466,   467,   468,   469,   470,   471,   472,  1306,  1329,
     473,  1359,  1331,  1332,  1334,  1355,   190,  1356,  1011,   183,
     184,  1357,  1358,   316,   191,   460,   461,   462,   463,   464,
    1367,   497,   185,   186,  1053,  1083,   997,  1012,  1111,   465,
     466,   467,   468,   469,   470,   471,   472,   187,   618,   473,
     188,   640,   998,  1294,   189,   185,   186,   699,   865,  1013,
     921,   903,  1337,   414,   460,   461,   462,   463,   464,  1044,
     187,  1339,   925,   188,   922,  1105,  1211,   189,   465,   466,
     467,   468,   469,   470,   471,   472,  1014,   303,   473,  1149,
     190,  1172,  -393,   460,   461,   462,   463,   464,   191,   996,
       0,     0,  1033,     0,     0,  1255,     0,   465,   466,   467,
     468,   469,   470,   471,   472,  1015,     0,   473,   460,   461,
     462,   463,   464,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   465,   466,   467,   468,   469,   470,   471,   472,
       0,     0,   473,     0,     0,     0,   460,   461,   462,   463,
     464,     0,     0,     0,     0,     0,     0,     0,     0,   904,
     465,   466,   467,   468,   469,   470,   471,   472,     0,     0,
     473,     0,     0,     0,     0,     0,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,   429,     0,     0,     0,   460,   461,   462,
     463,   464,   190,     0,     0,     0,  1023,     0,     0,  1024,
     191,   465,   466,   467,   468,   469,   470,   471,   472,     0,
       0,   473,     0,     0,     0,   190,   460,   461,   462,   463,
     464,     0,     0,   191,     0,     0,     0,     0,     0,     0,
     465,   466,   467,   468,   469,   470,   471,   472,     0,     0,
     473,   460,   461,   462,   463,   464,     0,   430,   431,     0,
       0,     0,     0,   911,     0,   465,   466,   467,   468,   469,
     470,   471,   472,     0,     0,   473,   460,   461,   462,   463,
     464,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     465,   466,   467,   468,   469,   470,   471,   472,     0,     0,
     473,   460,   461,   462,   463,   464,   433,     0,     0,  1041,
       0,     0,   429,     0,     0,   465,   466,   467,   468,   469,
     470,   471,   472,     0,     0,   473,   460,   461,   462,   463,
     464,     0,     0,     0,     0,  1013,     0,     0,     0,     0,
     465,   466,   467,   468,   469,   470,   471,   472,  1112,     0,
     473,   460,   461,   462,   463,   464,     0,     0,     0,     0,
       0,     0,  1014,     0,     0,   465,   466,   467,   468,   469,
     470,   471,   472,     0,  1200,   473,   430,   431,     0,     0,
       0,     0,     0,   124,   125,     0,     0,  1113,     0,     0,
       0,  1015,   126,   432,     0,   441,   442,   443,   444,   445,
     446,   447,   448,   449,     0,     0,     0,     0,     0,   127,
       0,     0,   128,     0,   129,   130,  1114,     0,     0,     0,
       0,     0,   450,   451,     0,   433,     0,     0,     0,     0,
       0,     0,     0,   452,   453,   454,   455,     0,     0,     0,
       0,  1115,     0,     0,     0,     0,     0,   460,   461,   462,
     463,   464,  1016,  1017,  1018,  1019,  1020,  1021,  1022,     0,
     213,   465,   466,   467,   468,   469,   470,   471,   472,  1121,
       0,   473,  1023,   434,   435,  1024,   436,   437,   438,   439,
     440,     0,     0,     0,     0,     0,     0,   214,     0,     0,
     215,     0,     0,     0,   216,   217,     0,     0,   219,   220,
       0,     0,     0,   456,   457,     0,     0,     0,     0,     0,
    1161,     0,     0,     0,   441,   442,   443,   444,   445,   446,
     447,   448,   449,     0,     0,     0,     0,     0,     0,   221,
       0,     0,   213,   222,     0,   131,     0,     0,   213,  1212,
       0,   450,   451,   227,     0,     0,     0,     0,     0,     0,
       0,     0,   452,   453,   454,   455,     0,     0,   213,   214,
       0,     0,   215,     0,  1335,   214,   216,   217,   215,     0,
     219,   220,   216,   217,     0,     0,   219,   220,     0,     0,
       0,     0,   228,     0,   229,   214,     0,     0,   215,  1340,
       0,     0,   216,   217,     0,     0,   219,   220,     0,     0,
       0,   221,     0,     0,     0,   222,     0,   221,   213,   132,
       0,   222,     0,   291,  1341,     0,     0,     0,     0,   230,
       0,     0,   456,   457,     0,     0,     0,   221,     0,   231,
     232,   222,     0,     0,     0,   214,     0,     0,   215,  1342,
       0,  -261,   216,   217,     0,   275,   219,   220,     0,     0,
       0,     0,   276,     0,     0,     0,     0,     0,     0,     0,
       0,   276,     0,   260,  1343,     0,     0,   233,   261,     0,
     262,     0,     0,   213,     0,   277,   278,   221,     0,     0,
       0,   222,     0,   234,   235,   278,   236,   237,     0,   238,
     239,   240,     0,   241,   307,     0,   242,   243,     0,   244,
     214,     0,     0,   215,     0,  1264,   263,   216,   217,     0,
       0,   219,   220,   245,   246,   627,     0,     0,   247,     0,
     248,   213,   249,     0,   250,   308,     0,   292,     0,   251,
     346,   347,   774,   252,   253,     0,   254,   255,     0,   256,
       0,     0,   221,     0,   264,     0,   222,     0,   214,     0,
       0,   215,   628,     0,     0,   216,   217,     0,   218,   219,
     220,   684,   685,   686,   687,   688,   689,   690,   293,   691,
     692,   693,   694,   695,     0,     0,     0,     0,     0,     0,
     629,  1265,     0,     0,     0,   630,     0,     0,     0,     0,
     221,     0,   631,   632,   222,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   633,   634,   635,   636,     0,
     637,     0,     0,     0,   270,   271,   294,     0,     0,     0,
       0,     0,   257,     0,     0,     0,  1266,     0,  1267,     0,
    1268,  1269,  1270,  1271,  1272,  1273,   279,     0,     0,     0,
       0,     0,   272,     0,     0,  1274,  1275,     0,     0,  1276,
       0,     0,     0,  1277,     0,  1278,   684,   685,   686,   687,
     688,   689,   690,  1279,   691,   692,   693,   694,   695,   280,
     281,   282,   283,     0,     0,     0,     4,     0,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   285,     5,     6,     0,   494,     0,     0,     0,
       0,  1280,   460,   461,   462,   463,   464,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   465,   466,   467,   468,
     469,   470,   471,   472,     0,     0,   473,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     0,   900,
       0,     0,     0,     0,     0,     0,     8,     0,     0,     9,
       0,    10,     0,     0,    11,     0,     0,   309,     0,    12,
      13,    14,  -399,  -399,  -399,     0,    15,    16,    17,    18,
       0,     0,     0,   460,   461,   462,  -399,  -399,  -399,  -399,
     469,   470,   471,   472,     0,     0,   473,   465,   466,   467,
     468,   469,   470,   471,   472,     0,     0,   473,   684,   685,
     686,   687,   688,   689,   690,   223,   691,   692,   693,   694,
     695,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1153
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1152))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-399))

static const yytype_int16 yycheck[] =
{
      76,   191,   229,   234,   235,   260,   191,   555,   132,   155,
     133,   134,   559,   708,   917,    18,   636,    22,     3,     7,
      15,   248,    15,   897,    19,    29,    19,   100,    15,     6,
     843,  1026,    18,    19,   261,    28,   263,  1136,    15,    16,
      17,    18,    19,    26,    27,   191,    27,    15,    33,    18,
      19,    19,  1203,    38,    18,    19,    41,    15,    16,    17,
      18,     8,     9,    10,    11,    12,    40,    52,    35,    46,
     216,    68,   299,    40,   301,    22,    23,    24,    25,    26,
      27,    28,    29,    68,     3,    32,    71,    77,    73,    74,
    1027,   272,    35,   239,    40,    19,    81,    40,    75,    15,
       6,    15,    18,    19,   180,    19,     4,    81,    82,    15,
      16,    17,    18,    19,   190,   100,   262,    82,   103,  1218,
      34,    35,    36,    37,    15,   534,    19,     6,    19,   114,
      20,   277,     0,     3,   280,   281,    15,    16,    17,    18,
      46,   322,    20,    34,    35,    36,    37,    40,  1299,    68,
     135,   121,  1026,    15,    16,    17,    18,    19,  1153,   136,
     137,   138,   139,   140,   141,   142,   187,   576,   291,    75,
       8,     9,    10,    11,    12,    53,    15,    96,   122,   156,
      19,   994,   159,   275,    22,    23,    24,    25,    26,    27,
      28,    29,   284,   112,    32,    34,    35,    36,    37,     3,
     185,    91,   183,   100,   201,    15,    16,    17,    18,    19,
     184,  1119,   168,   336,  1151,   339,   340,   363,   342,   343,
     105,   106,   107,   108,   188,  1099,   150,   482,   374,     3,
     136,   137,   138,   139,   140,   141,   142,    15,    16,    17,
      18,     8,     9,    10,    11,    12,  1241,    82,   238,    83,
     156,   121,   325,   159,   100,    22,    23,    24,    25,    26,
      27,    28,    29,   170,    15,    32,   222,    20,    19,   188,
       8,     9,    10,    11,    12,  1200,   220,    18,    19,  1153,
     511,   115,   252,   510,    22,    23,    24,    25,    26,    27,
      28,    29,   313,   314,    32,    15,   372,   524,    51,    19,
     154,   201,   529,    19,   531,   275,   533,  1215,   535,   929,
    1213,   538,   539,   540,   390,   320,   100,  1242,   650,   323,
     396,   653,   325,   655,    40,   320,  1200,   320,   404,   405,
     406,   407,   408,   321,   327,  1203,  1031,   322,   252,   325,
     327,   324,   418,   320,    95,   310,   311,    82,   325,   580,
     577,   582,   320,    20,   322,   325,   325,   324,    91,   213,
      72,   507,   320,   590,    97,   596,   512,  1241,  1242,   515,
      72,   517,    39,  1298,   321,    95,   522,   523,   324,   326,
      19,   324,    15,    50,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,    99,   298,    15,
     291,  1304,    35,    19,   320,   636,   320,    40,   260,    15,
      16,    17,    18,    19,   320,   324,    81,   326,   494,   325,
     566,   567,    19,    20,  1298,   325,    19,    38,   659,   320,
    1050,  1299,    19,   268,   321,   982,   271,   322,   140,   326,
     586,   320,   588,    40,   141,    56,   325,    68,   594,     8,
       9,    10,    11,    12,   306,   307,   308,   309,   320,   124,
     125,    22,    73,    22,    23,    24,    25,    26,    27,    28,
      29,    32,   703,    32,   104,     8,     9,    10,    11,    12,
     387,   320,   389,   198,   391,   392,   208,   209,   326,    22,
      23,    24,    25,    26,    27,    28,    29,   326,   195,    32,
     329,    19,    86,   626,    88,     8,     9,    10,    11,    12,
     320,   122,   658,   178,   229,   230,  1064,    19,   183,    22,
      23,    24,    25,    26,    27,    28,    29,    19,   652,    32,
     654,   324,   229,   268,   252,   253,   271,   218,   219,    28,
      29,   238,   320,    32,    18,    19,    20,   802,   880,    85,
     882,   741,   742,    20,    19,    20,   783,   784,   704,   326,
      78,     3,   747,     8,     9,    10,    11,    12,    68,   320,
      86,   289,    88,   109,   324,    40,   326,    22,    23,    24,
      25,    26,    27,    28,    29,   204,   205,    32,   326,     8,
       9,    10,    11,    12,   326,   741,   742,   329,    19,    20,
     320,   299,   300,    22,    23,    24,    25,    26,    27,    28,
      29,   129,   323,    32,   845,   326,    19,    20,   873,    40,
     875,   318,   319,    13,   879,    15,    16,    17,    18,    19,
      20,     3,   778,   779,    54,    55,    26,    27,    28,    29,
       3,    31,   788,     8,     9,   768,   792,   112,   113,   795,
       3,    16,    17,    18,   800,   325,   326,   733,    19,   735,
       8,     9,    10,    11,    12,   281,   282,  1084,    15,   205,
      17,    18,   818,     3,    22,    23,    24,    25,    26,    27,
      28,    29,   325,   326,    32,    75,    18,    19,    20,     3,
     836,    81,     8,     9,    10,    11,    12,   387,   929,   389,
      60,   391,   392,     3,   320,    65,    22,    23,    24,    25,
      26,    27,    28,    29,   320,     3,    32,    77,   254,   255,
     256,     3,   258,   325,   326,  1142,  1143,  1144,  1145,   987,
     988,   989,     3,   269,   124,   125,  1094,  1095,   965,     3,
     967,   400,   401,    15,   104,    13,   973,    15,    16,    17,
      18,    19,    20,     3,   119,   120,     3,     3,    26,    27,
      22,    29,    57,    31,   322,    78,    19,   326,   286,   322,
     160,   161,   162,   163,   164,   165,   294,   295,    29,    66,
     320,   145,   183,   320,    86,   175,   320,    19,   178,   325,
     320,   320,   182,   326,   320,   941,    83,    84,   320,   324,
     324,   324,    89,    90,   168,    92,    72,    75,   121,     3,
     174,    20,   322,    81,   322,   179,   180,   324,  1073,  1050,
      20,   206,   224,   326,   107,    19,    63,    63,   197,   905,
     189,   201,   908,   909,   910,   191,   912,    66,   914,   218,
     222,   203,  1069,   919,   228,   226,   233,   970,   241,   995,
     926,     8,     9,    10,    11,    12,   124,   125,   201,   239,
     322,   324,    91,    92,   322,    22,    23,    24,    25,    26,
      27,    28,    29,   324,   324,    32,    97,    71,    99,    68,
     101,   326,   103,   243,   105,   322,   199,   108,   109,   324,
      20,   324,   160,   161,   162,   163,   164,   165,   127,   128,
     129,   130,    63,    63,   322,   324,   322,   175,   322,  1055,
     178,   324,   324,    68,   182,   322,    68,    63,    66,    63,
     278,  1067,  1068,   322,  1070,   322,   322,    20,   322,     3,
     320,   322,  1078,     3,     3,    19,   296,   250,   328,     3,
       3,   301,     3,  1019,     3,    19,     3,  1023,  1024,     3,
       3,     3,   101,  1029,   101,     3,   322,   317,    20,    14,
     183,   274,    27,   276,  1040,     8,     9,    10,    11,    12,
     283,   322,   285,   322,   322,   320,   289,   290,    15,    22,
      23,    24,    25,    26,    27,    28,    29,    26,    29,    32,
      32,  1137,    19,    15,    68,   326,  1223,     7,  1225,    73,
      74,    19,   322,    63,   198,    63,   322,    81,    46,   231,
     322,   188,   325,  1203,    30,   322,    63,    83,    63,   198,
    1096,  1097,  1098,    63,   322,    91,   100,  1173,  1174,  1175,
      96,  1177,    98,   322,   100,    19,   102,    72,   104,  1266,
    1116,   322,   322,  1119,   322,   322,   224,  1170,   322,  1172,
     244,    85,   320,     8,     9,    10,    11,    12,   195,   322,
     328,   135,   322,   322,   322,   259,   322,    22,    23,    24,
      25,    26,    27,    28,    29,     3,    19,    32,  1224,  1155,
    1156,   322,  1158,   322,   278,    15,  1232,  1233,   322,    15,
      13,    20,    15,    16,    17,    18,    19,    20,   292,   293,
    1327,    19,  1329,    26,    27,    19,    29,  1297,    31,  1299,
      41,    42,    43,    44,    45,    46,    47,    48,    49,  1265,
      19,  1267,  1268,  1269,    15,  1271,  1272,   322,    20,    13,
    1276,    15,    16,    17,    18,    19,    20,   322,  1284,  1215,
     322,   324,    26,    27,    28,    29,   321,    31,    27,     7,
     326,    15,    75,   326,   320,    18,    27,   326,    81,   322,
     322,   325,    19,   322,   322,   322,  1312,   188,  1314,   322,
    1246,   260,   261,   262,   263,   264,   265,   266,   267,   322,
    1256,  1257,  1258,  1259,  1330,  1331,   322,   322,   322,   322,
     322,    75,   288,   322,   322,   322,   322,    81,   322,   322,
     322,   124,   125,    13,   322,    15,    16,    17,    18,    19,
      20,     8,     9,    10,    11,    12,    26,    27,   322,    29,
     322,    31,   322,    41,   322,    22,    23,    24,    25,    26,
      27,    28,    29,   325,   322,    32,   322,   160,   161,   322,
     124,   125,    60,    61,    62,   322,    64,    65,     3,     3,
      68,    15,   175,    71,     3,   178,     3,    75,    76,   182,
    1336,    79,    80,   188,   188,    75,     8,     9,    10,    11,
      12,    81,   322,   101,    27,   321,   160,   161,   322,   322,
      22,    23,    24,    25,    26,    27,    28,    29,   278,    20,
      32,   175,   110,   326,   178,   326,   114,   322,   182,   322,
     324,     3,   322,   322,     3,    13,    15,    15,    16,    17,
      18,    19,    20,   322,   124,   125,   322,   322,    26,    27,
      28,    29,   322,    31,   322,    15,    22,   320,    13,   325,
      15,    16,    17,    18,    19,    20,   322,   326,   321,    15,
      15,    26,    27,   322,    29,    22,    31,   324,   324,   324,
     160,   161,   322,   322,   322,   322,   322,     7,   188,   322,
       8,     9,    10,    11,    12,   175,   321,    75,   178,   326,
     322,   325,   182,    81,    22,    23,    24,    25,    26,    27,
      28,    29,     3,     3,    32,   321,   320,    15,   321,   203,
      75,   158,     7,   325,   203,   322,    81,   320,   321,   321,
     320,   320,   262,   320,   320,   328,     8,     9,    10,    11,
      12,   322,   322,   322,   322,   321,   124,   125,   280,   326,
      22,    23,    24,    25,    26,    27,    28,    29,   324,   278,
      32,   321,   288,   322,   322,   322,   320,   322,   891,   124,
     125,   322,   322,   115,   328,     8,     9,    10,    11,    12,
     322,   220,   160,   161,   930,   982,   856,   896,  1031,    22,
      23,    24,    25,    26,    27,    28,    29,   175,   329,    32,
     178,   336,   858,  1241,   182,   160,   161,   381,   632,    19,
     741,   709,  1297,   190,     8,     9,    10,    11,    12,   918,
     175,  1299,   747,   178,   742,  1027,  1158,   182,    22,    23,
      24,    25,    26,    27,    28,    29,    46,   325,    32,  1097,
     320,  1134,   322,     8,     9,    10,    11,    12,   328,   847,
      -1,    -1,   905,    -1,    -1,   322,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    75,    -1,    32,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   321,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,
     140,   141,   142,     5,    -1,    -1,    -1,     8,     9,    10,
      11,    12,   320,    -1,    -1,    -1,   156,    -1,    -1,   159,
     328,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    -1,   320,     8,     9,    10,    11,
      12,    -1,    -1,   328,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,     8,     9,    10,    11,    12,    -1,    69,    70,    -1,
      -1,    -1,    -1,   321,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,     8,     9,    10,    11,    12,   118,    -1,    -1,   321,
      -1,    -1,     5,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,   321,    -1,
      32,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,   324,    32,    69,    70,    -1,    -1,
      -1,    -1,    -1,    91,    92,    -1,    -1,   321,    -1,    -1,
      -1,    75,   100,    86,    -1,   207,   208,   209,   210,   211,
     212,   213,   214,   215,    -1,    -1,    -1,    -1,    -1,   117,
      -1,    -1,   120,    -1,   122,   123,   321,    -1,    -1,    -1,
      -1,    -1,   234,   235,    -1,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   245,   246,   247,   248,    -1,    -1,    -1,
      -1,   321,    -1,    -1,    -1,    -1,    -1,     8,     9,    10,
      11,    12,   136,   137,   138,   139,   140,   141,   142,    -1,
      41,    22,    23,    24,    25,    26,    27,    28,    29,   321,
      -1,    32,   156,   166,   167,   159,   169,   170,   171,   172,
     173,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      71,    -1,    -1,    -1,    75,    76,    -1,    -1,    79,    80,
      -1,    -1,    -1,   315,   316,    -1,    -1,    -1,    -1,    -1,
     321,    -1,    -1,    -1,   207,   208,   209,   210,   211,   212,
     213,   214,   215,    -1,    -1,    -1,    -1,    -1,    -1,   110,
      -1,    -1,    41,   114,    -1,   243,    -1,    -1,    41,   321,
      -1,   234,   235,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   245,   246,   247,   248,    -1,    -1,    41,    68,
      -1,    -1,    71,    -1,   321,    68,    75,    76,    71,    -1,
      79,    80,    75,    76,    -1,    -1,    79,    80,    -1,    -1,
      -1,    -1,    85,    -1,    87,    68,    -1,    -1,    71,   321,
      -1,    -1,    75,    76,    -1,    -1,    79,    80,    -1,    -1,
      -1,   110,    -1,    -1,    -1,   114,    -1,   110,    41,   317,
      -1,   114,    -1,   122,   321,    -1,    -1,    -1,    -1,   122,
      -1,    -1,   315,   316,    -1,    -1,    -1,   110,    -1,   132,
     133,   114,    -1,    -1,    -1,    68,    -1,    -1,    71,   321,
      -1,   325,    75,    76,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    85,    -1,   244,   321,    -1,    -1,   170,   249,    -1,
     251,    -1,    -1,    41,    -1,   108,   109,   110,    -1,    -1,
      -1,   114,    -1,   186,   187,   109,   189,   190,    -1,   192,
     193,   194,    -1,   196,    62,    -1,   199,   200,    -1,   202,
      68,    -1,    -1,    71,    -1,   129,   287,    75,    76,    -1,
      -1,    79,    80,   216,   217,     3,    -1,    -1,   221,    -1,
     223,    41,   225,    -1,   227,    93,    -1,   236,    -1,   232,
      18,    19,    19,   236,   237,    -1,   239,   240,    -1,   242,
      -1,    -1,   110,    -1,   325,    -1,   114,    -1,    68,    -1,
      -1,    71,    40,    -1,    -1,    75,    76,    -1,    78,    79,
      80,   143,   144,   145,   146,   147,   148,   149,   277,   151,
     152,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,
      68,   205,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
     110,    -1,    80,    81,   114,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    -1,    -1,    -1,   297,   298,   325,    -1,    -1,    -1,
      -1,    -1,   325,    -1,    -1,    -1,   250,    -1,   252,    -1,
     254,   255,   256,   257,   258,   259,   269,    -1,    -1,    -1,
      -1,    -1,   325,    -1,    -1,   269,   270,    -1,    -1,   273,
      -1,    -1,    -1,   277,    -1,   279,   143,   144,   145,   146,
     147,   148,   149,   287,   151,   152,   153,   154,   155,   302,
     303,   304,   305,    -1,    -1,    -1,    41,    -1,    -1,   312,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   325,    58,    59,    -1,   184,    -1,    -1,    -1,
      -1,   325,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,   321,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,
      -1,   116,    -1,    -1,   119,    -1,    -1,   325,    -1,   124,
     125,   126,     8,     9,    10,    -1,   131,   132,   133,   134,
      -1,    -1,    -1,     8,     9,    10,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,   143,   144,
     145,   146,   147,   148,   149,   325,   151,   152,   153,   154,
     155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   331,   332,    41,    58,    59,   102,   111,   114,
     116,   119,   124,   125,   126,   131,   132,   133,   134,   353,
     354,   355,   397,   398,   399,   400,   402,   403,   404,   407,
     410,   413,   414,   415,   416,   417,   418,   419,   420,   421,
     422,   423,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   521,   522,   523,   524,   525,   526,   527,   528,   547,
     549,     0,     3,    33,    38,    41,    52,    68,    71,    73,
      74,    81,   100,   103,   114,   135,   185,   322,   334,   343,
     349,   352,   357,   359,   362,   364,   367,   368,   370,   371,
     376,   382,   383,   437,   439,   449,   488,   489,   490,   491,
     530,   531,   532,   533,   541,   542,   548,   550,   551,   552,
      19,    20,    40,   435,    20,    51,   356,     3,   100,   112,
     113,   435,   435,     3,    91,    92,   100,   117,   120,   122,
     123,   243,   317,   100,   100,    66,    91,    92,   127,   128,
     129,   130,    15,    35,    40,   424,   435,   435,   435,    19,
      20,    39,    50,   345,    91,    97,    38,    56,    73,   122,
     350,   351,    19,    72,    19,    68,   104,    20,    91,    19,
      13,    15,    16,    17,    18,    19,    20,    26,    27,    29,
      31,    75,    81,   124,   125,   160,   161,   175,   178,   182,
     320,   328,   454,   467,   472,   475,   476,   477,   479,   481,
      19,   372,   372,   324,    60,    65,    77,   104,   243,   296,
     301,   317,   372,    41,    68,    71,    75,    76,    78,    79,
      80,   110,   114,   325,   373,   385,   372,    46,    85,    87,
     122,   132,   133,   170,   186,   187,   189,   190,   192,   193,
     194,   196,   199,   200,   202,   216,   217,   221,   223,   225,
     227,   232,   236,   237,   239,   240,   242,   325,   373,   372,
     244,   249,   251,   287,   325,   373,   492,   493,   494,   372,
     297,   298,   325,   373,   372,    78,    85,   108,   109,   269,
     302,   303,   304,   305,   312,   325,   373,   392,   394,   395,
     372,   122,   236,   277,   325,   373,   543,   546,    60,    61,
      62,    64,    65,   325,   358,   361,   373,    62,    93,   325,
     360,   373,    35,    40,   324,   386,   356,     3,    68,   412,
     435,   435,     3,    35,    40,   324,   389,    19,    20,    40,
     436,     3,   121,   408,     3,     3,   405,   411,    19,    78,
     129,   286,   294,   295,   401,   401,    18,    19,   336,   338,
     338,     3,     3,     3,     3,     3,     3,     3,    15,   425,
       3,     3,     3,    22,    57,   344,    40,   324,   346,   386,
      15,    19,   320,   333,    20,    53,   322,    15,    17,    18,
     363,   438,    19,   365,   322,   389,   450,    15,    19,    28,
     320,   327,   471,   473,   479,    29,   320,   183,    27,   183,
      15,   327,   474,   481,   320,   320,   320,   320,   320,   320,
     162,   163,   164,   165,   461,   481,    16,    17,   320,   333,
     454,   455,   459,   462,   463,   465,   467,   468,   469,     5,
      69,    70,    86,   118,   166,   167,   169,   170,   171,   172,
     173,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     234,   235,   245,   246,   247,   248,   315,   316,   480,   553,
       8,     9,    10,    11,    12,    22,    23,    24,    25,    26,
      27,    28,    29,    32,    86,   324,   324,   369,   324,   386,
      19,   375,    72,   333,    34,    35,    36,    37,   333,   335,
     384,    40,    81,    82,   184,   374,   446,   374,    20,   389,
     322,   322,   324,   204,   205,    20,   335,   224,   206,   122,
     220,   107,   188,   336,   336,    63,   191,    63,   322,   333,
     189,   197,   208,   209,   201,   203,   218,   219,   218,   222,
     335,   226,   228,   233,    68,   201,    77,   238,   201,   241,
     239,   322,   324,   363,   335,   333,   335,   322,   324,   324,
      68,   299,   300,   322,   324,   539,   540,    20,   333,   324,
      82,   310,   311,   534,   333,   333,    63,    63,   187,   313,
     314,   322,   322,   324,   324,   338,    68,   278,   322,   141,
     195,   229,   238,   318,   319,   322,    68,   335,    63,   335,
      66,   322,   322,   322,    63,    20,   342,   322,   322,    66,
      83,    84,    89,    90,    92,   387,   388,     3,     3,    19,
       3,     3,     3,    83,   115,   390,   391,     3,   436,     3,
       3,     3,    68,    96,   112,   188,   409,     3,    40,    68,
      73,    80,    81,    93,    94,    95,    96,    98,   338,   406,
     446,     3,    19,    68,    73,    74,    81,   100,   135,   401,
     401,   529,    40,   401,    40,   401,     3,     3,    29,   323,
     340,   101,   101,     3,    18,   333,    20,    41,    42,    43,
      44,    45,    46,    47,    48,    49,   347,   348,   322,   481,
     333,    54,    55,   322,   143,   144,   145,   146,   147,   148,
     149,   151,   152,   153,   154,   155,   440,   441,   442,   444,
     448,   105,   106,   107,   108,   322,   366,   322,   320,   447,
     473,   473,   481,   473,   473,    14,    15,   481,   485,   486,
      26,   183,    27,   474,   474,    29,   481,   481,   481,   481,
     481,    19,    15,   326,   321,   326,    28,   457,   458,   461,
     481,     7,   326,   329,    26,    27,   324,   326,   329,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   145,   168,   174,   179,   180,    19,   100,   325,
     363,   377,   380,   381,    19,   444,   445,   481,    63,    63,
     322,   322,   333,    46,   231,   335,   336,   333,   188,    30,
     337,   333,    63,   333,   322,    63,   198,   333,   333,   335,
      63,   322,    72,   322,   335,   322,   335,   198,   229,   230,
     335,   375,   335,   322,   322,   335,   335,   335,   224,   495,
      19,   322,   322,   381,   322,   322,   393,   394,   322,   322,
     322,   333,   333,   322,   322,   322,    72,    99,   140,   396,
     375,   335,    20,   336,   336,   195,   544,   545,   333,   333,
     335,   333,   336,   338,   339,   325,   326,   325,   326,     3,
      19,    15,   338,    19,    19,   445,   339,    19,    20,   520,
     520,     3,    19,    71,   198,   244,   259,   278,   292,   293,
     401,   520,   401,   520,   333,   336,    15,    15,   322,   322,
     325,   326,   321,    20,    19,   150,   322,   324,   336,   333,
     321,   441,   443,   448,   321,   326,   321,    27,   326,   326,
     326,   321,   326,   321,   326,   481,   481,     7,   326,   326,
      18,   455,   463,    15,    18,   468,   320,   338,    27,   379,
     326,   333,   333,   322,   335,   335,   322,   322,   322,   333,
      19,   188,   322,   333,   322,   333,   322,   322,   322,   322,
     333,   363,   322,   322,   322,   322,   322,   322,   322,   322,
     322,   322,   322,   333,    78,   121,   199,   250,   274,   276,
     283,   285,   289,   290,   325,   496,   503,   506,   322,   324,
     538,   325,   322,   322,   322,   333,    86,    88,    86,    88,
     325,   322,   322,   337,   336,   288,   544,   388,   391,     3,
       3,   363,   363,    15,   363,   520,     3,   520,     3,   188,
     188,   348,   442,    19,    46,    75,   136,   137,   138,   139,
     140,   141,   142,   156,   159,   451,   452,   477,   478,   482,
     487,   322,   321,   485,    27,   481,   481,   481,   481,   481,
     326,   321,   457,   481,   458,   460,   481,   326,   481,   101,
      20,   339,   378,   380,   322,   326,   322,   322,   322,   333,
     322,   322,   322,   322,   516,   519,   335,   252,   253,   289,
     252,   335,   497,   278,   338,   499,   500,   498,   291,   335,
     322,   324,   537,   393,   322,    20,   341,   341,   341,   341,
     337,   333,     3,     3,    15,    15,    22,   320,    22,   320,
     481,   481,   481,   325,   487,   480,   553,   322,   481,   483,
     484,   443,   321,   321,   321,   321,   326,   481,   321,   326,
      15,   321,    15,   339,   333,   322,   381,   333,   333,   335,
     333,   324,   363,   324,   324,   333,   504,   201,   298,   325,
     536,   396,   322,   322,   322,   322,   412,   412,   481,   486,
     481,   477,   322,   157,   324,    22,    22,    32,   326,   322,
     481,   321,     7,    28,   460,   325,   188,   322,   324,   518,
     508,   501,   508,   205,   254,   255,   256,   258,   269,   325,
     392,   505,   333,   260,   306,   307,   308,   309,   535,   322,
     396,   396,   396,   396,     3,     3,   322,   321,   322,   553,
     324,   453,   487,   470,   481,   481,    81,   124,   125,   178,
     183,   484,   321,   320,   321,     7,   321,    15,   517,   325,
     338,   509,   510,   121,   252,   275,   325,   502,   325,   333,
     333,   333,   203,    95,   333,    82,   268,   271,   322,   321,
     451,   158,     6,    15,    16,    17,   320,   325,   454,   456,
     459,   462,   464,   466,   322,   322,   320,   320,   320,   320,
     322,   457,   460,   322,   129,   205,   250,   252,   254,   255,
     256,   257,   258,   259,   269,   270,   273,   277,   279,   287,
     325,   392,   507,   275,   284,   511,   335,   333,   335,   322,
     333,   333,   322,   325,   453,   451,   481,     7,   323,   326,
     481,   481,   481,   481,   326,   321,   324,   333,   335,   333,
     333,   333,   203,   333,    95,   333,   260,   261,   262,   263,
     264,   265,   266,   267,    82,   268,   271,   262,   333,   278,
     280,   288,   322,   333,   322,   321,   320,   456,   451,   464,
     321,   321,   321,   321,   457,   512,   333,   333,   335,   335,
     281,   282,   333,   333,   481,   322,   322,   322,   322,   321,
      18,   325,   513,   514,   515,   272,   322,   322
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
#line 633 "cf-parse.y"
    { return 0; }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 634 "cf-parse.y"
    { return 0; }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 647 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 648 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 654 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 658 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 667 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 668 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 669 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 670 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 671 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 672 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 679 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 686 "cf-parse.y"
    { (yyval.iface) = NULL; }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 687 "cf-parse.y"
    { (yyval.iface) = if_get_by_name((yyvsp[(2) - (2)].s)->name); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 691 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 699 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 703 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 707 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 714 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 722 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 723 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 729 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 738 "cf-parse.y"
    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 739 "cf-parse.y"
    { (yyval.t) = bird_name; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 743 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 748 "cf-parse.y"
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[(2) - (2)].t); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 749 "cf-parse.y"
    { (yyval.g) = stderr; }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 753 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 754 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 758 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 759 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 763 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 764 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 765 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 766 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 767 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 768 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 769 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 770 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 771 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 777 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 778 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 787 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 788 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 789 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 790 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 793 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 794 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 795 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 796 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 808 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 811 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 814 "cf-parse.y"
    { cmd_shutdown(); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 817 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 826 "cf-parse.y"
    {
#ifndef CONFIG_MULTIPLE_TABLES
     if (cf_krt)
       cf_error("Kernel protocol already defined");
#endif
     cf_krt = this_proto = proto_config_new(&proto_unix_kernel, sizeof(struct krt_config), (yyvsp[(1) - (2)].i));
     THIS_KRT->scan_time = 60;
     THIS_KRT->learn = THIS_KRT->persist = 0;
     krt_scan_construct(THIS_KRT);
     krt_set_construct(THIS_KRT);
   }
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
    {
     if (cf_kif)
       cf_error("Kernel device protocol already defined");
     cf_kif = this_proto = proto_config_new(&proto_unix_iface, sizeof(struct kif_config), (yyvsp[(1) - (2)].i));
     THIS_KIF->scan_time = 60;
     init_list(&THIS_KIF->primary);
     krt_if_construct(THIS_KIF);
   }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 871 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 875 "cf-parse.y"
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
#line 888 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 901 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 907 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 909 "cf-parse.y"
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
#line 928 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 929 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 930 "cf-parse.y"
    { new_config->listen_bgp_flags = 0; }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 931 "cf-parse.y"
    { new_config->listen_bgp_flags = 1; }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 938 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 944 "cf-parse.y"
    {
  this_roa_table = roa_new_table_config((yyvsp[(3) - (3)].s));
}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 950 "cf-parse.y"
    {
     roa_add_item_config(this_roa_table, (yyvsp[(3) - (8)].px).addr, (yyvsp[(3) - (8)].px).len, (yyvsp[(5) - (8)].i), (yyvsp[(7) - (8)].i));
   }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 964 "cf-parse.y"
    { (yyval.i) = SYM_PROTO; }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 965 "cf-parse.y"
    { (yyval.i) = SYM_TEMPLATE; }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 969 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = this_proto->class;
     s->def = this_proto;
     this_proto->name = s->name;
     }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 975 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 979 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].s)->class != SYM_TEMPLATE) && ((yyvsp[(3) - (3)].s)->class != SYM_PROTO)) cf_error("Template or protocol name expected");

     cf_define_symbol((yyvsp[(1) - (3)].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[(1) - (3)].s)->name;

     proto_copy_config(this_proto, (yyvsp[(3) - (3)].s)->def);
   }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 991 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 995 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 996 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 997 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 998 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 999 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1000 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1001 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1002 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1006 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1008 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1009 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1013 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 1021 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1022 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1030 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1038 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1039 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1040 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1044 "cf-parse.y"
    { this_ipn->positive = 1; }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1045 "cf-parse.y"
    { this_ipn->positive = 0; }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1062 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_device, sizeof(struct rt_dev_config), (yyvsp[(1) - (2)].i));
     init_list(&DIRECT_CFG->iface_list);
   }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1075 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&DIRECT_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1089 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1090 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1091 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1096 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1100 "cf-parse.y"
    { (yyval.i) = D_STATES; }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1101 "cf-parse.y"
    { (yyval.i) = D_ROUTES; }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1102 "cf-parse.y"
    { (yyval.i) = D_FILTERS; }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1103 "cf-parse.y"
    { (yyval.i) = D_IFACES; }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1104 "cf-parse.y"
    { (yyval.i) = D_EVENTS; }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1105 "cf-parse.y"
    { (yyval.i) = D_PACKETS; }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1111 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1112 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1113 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1118 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1122 "cf-parse.y"
    { (yyval.i) = MD_STATES; }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1123 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1144 "cf-parse.y"
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

  case 152:

/* Line 1806 of yacc.c  */
#line 1162 "cf-parse.y"
    { }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1163 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1164 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1165 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1166 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1167 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1176 "cf-parse.y"
    { cmd_show_status(); }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1179 "cf-parse.y"
    { cmd_show_memory(); }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1182 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1185 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1189 "cf-parse.y"
    { (yyval.s) = NULL; }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 1193 "cf-parse.y"
    { if_show(); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1196 "cf-parse.y"
    { if_show_summary(); }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1200 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1203 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1209 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1215 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1222 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1227 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1232 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1237 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1241 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1245 "cf-parse.y"
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

  case 176:

/* Line 1806 of yacc.c  */
#line 1255 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1263 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1267 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1274 "cf-parse.y"
    { (yyval.i) = 1; }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1275 "cf-parse.y"
    { (yyval.i) = 2; }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1281 "cf-parse.y"
    { roa_show((yyvsp[(3) - (4)].ro)); }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1284 "cf-parse.y"
    {
     (yyval.ro) = cfg_allocz(sizeof(struct roa_show_data));
     (yyval.ro)->mode = ROA_SHOW_ALL;
     (yyval.ro)->table = roa_table_default;
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
   }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1291 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     if ((yyval.ro)->mode != ROA_SHOW_ALL) cf_error("Only one prefix expected");
     (yyval.ro)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ro)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ro)->mode = (yyvsp[(2) - (3)].i);
   }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1298 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     (yyval.ro)->asn = (yyvsp[(3) - (3)].i);
   }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1302 "cf-parse.y"
    {
     (yyval.ro) = (yyvsp[(1) - (3)].ro);
     if ((yyvsp[(3) - (3)].s)->class != SYM_ROA) cf_error("%s is not a ROA table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ro)->table = ((struct roa_table_config *)(yyvsp[(3) - (3)].s)->def)->table;
   }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1310 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_PX; }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1311 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_IN; }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1312 "cf-parse.y"
    { (yyval.i) = ROA_SHOW_FOR; }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1318 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].sd)); }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1321 "cf-parse.y"
    {
     (yyval.sd) = cfg_allocz(sizeof(struct sym_show_data));
   }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1324 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_TABLE; }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1325 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_FUNCTION; }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1326 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_FILTER; }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1327 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_PROTO; }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1328 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_TEMPLATE; }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1329 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->type = SYM_ROA; }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1330 "cf-parse.y"
    { (yyval.sd) = (yyvsp[(1) - (2)].sd); (yyval.sd)->sym = (yyvsp[(2) - (2)].s); }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1335 "cf-parse.y"
    { 
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
     (yyval.rot) = roa_table_default;
   }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1340 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].s)->class != SYM_ROA)
       cf_error("%s is not a ROA table", (yyvsp[(2) - (2)].s)->name);
     (yyval.rot) = ((struct roa_table_config *)(yyvsp[(2) - (2)].s)->def)->table;
   }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1349 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_add_item((yyvsp[(8) - (9)].rot), (yyvsp[(3) - (9)].px).addr, (yyvsp[(3) - (9)].px).len, (yyvsp[(5) - (9)].i), (yyvsp[(7) - (9)].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1356 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_delete_item((yyvsp[(8) - (9)].rot), (yyvsp[(3) - (9)].px).addr, (yyvsp[(3) - (9)].px).len, (yyvsp[(5) - (9)].i), (yyvsp[(7) - (9)].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1363 "cf-parse.y"
    {
  if (! cli_access_restricted())
    { roa_flush((yyvsp[(3) - (4)].rot), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1371 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1373 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1375 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1377 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1379 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1381 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1383 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1385 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1391 "cf-parse.y"
    { (yyval.i) = ~0; }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1392 "cf-parse.y"
    { (yyval.i) = 0; }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1397 "cf-parse.y"
    { (yyval.i) = 4096; }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1398 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1405 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); }
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1407 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1409 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1411 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1413 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1415 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1419 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1423 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1426 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1429 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1430 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1431 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1435 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1436 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1437 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1445 "cf-parse.y"
    { (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[(2) - (2)].s) ); }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1446 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s)->def = (yyvsp[(4) - (4)].f);
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1455 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1459 "cf-parse.y"
    { (yyval.i) = T_INT; }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1460 "cf-parse.y"
    { (yyval.i) = T_BOOL; }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1461 "cf-parse.y"
    { (yyval.i) = T_IP; }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1462 "cf-parse.y"
    { (yyval.i) = T_PREFIX; }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1463 "cf-parse.y"
    { (yyval.i) = T_PAIR; }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1464 "cf-parse.y"
    { (yyval.i) = T_QUAD; }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1465 "cf-parse.y"
    { (yyval.i) = T_EC; }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1466 "cf-parse.y"
    { (yyval.i) = T_STRING; }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1467 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1468 "cf-parse.y"
    { (yyval.i) = T_PATH; }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1469 "cf-parse.y"
    { (yyval.i) = T_CLIST; }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1470 "cf-parse.y"
    { (yyval.i) = T_ECLIST; }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1471 "cf-parse.y"
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

  case 247:

/* Line 1806 of yacc.c  */
#line 1492 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1503 "cf-parse.y"
    { (yyval.s) = NULL; }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1504 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1511 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1512 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1519 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1528 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1536 "cf-parse.y"
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

  case 256:

/* Line 1806 of yacc.c  */
#line 1560 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1561 "cf-parse.y"
    { (yyval.s)=NULL; }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1565 "cf-parse.y"
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

  case 259:

/* Line 1806 of yacc.c  */
#line 1578 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   }
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1581 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1594 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1595 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; }
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1598 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); }
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1599 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1603 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1606 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1615 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1628 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1629 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1630 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1631 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); }
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1635 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1636 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = f_eval_int((yyvsp[(2) - (3)].x)); }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1637 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1638 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1639 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1643 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(1) - (1)].x)); check_u16((yyval.i)); }
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1646 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (1)].i), (yyvsp[(1) - (1)].i)); }
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1647 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i)); }
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1648 "cf-parse.y"
    { (yyval.i32) = 0xFFFF; }
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1652 "cf-parse.y"
    {
     (yyval.e) = f_new_pair_set(pair_a((yyvsp[(2) - (5)].i32)), pair_b((yyvsp[(2) - (5)].i32)), pair_a((yyvsp[(4) - (5)].i32)), pair_b((yyvsp[(4) - (5)].i32)));
   }
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1655 "cf-parse.y"
    {
     /* Hack: $2 and $4 should be pair_expr, but that would cause shift/reduce conflict */
     if ((pair_a((yyvsp[(2) - (11)].i32)) != pair_b((yyvsp[(2) - (11)].i32))) || (pair_a((yyvsp[(4) - (11)].i32)) != pair_b((yyvsp[(4) - (11)].i32))))
       cf_error("syntax error");
     (yyval.e) = f_new_pair_item(pair_b((yyvsp[(2) - (11)].i32)), (yyvsp[(8) - (11)].i), pair_b((yyvsp[(4) - (11)].i32)), (yyvsp[(10) - (11)].i)); 
   }
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1664 "cf-parse.y"
    { (yyval.i32) = f_eval_int((yyvsp[(1) - (1)].x)); }
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1667 "cf-parse.y"
    { (yyval.i) = EC_RT; }
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1668 "cf-parse.y"
    { (yyval.i) = EC_RO; }
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1669 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (2)].i); }
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1670 "cf-parse.y"
    { (yyval.i) = EC_GENERIC; }
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1674 "cf-parse.y"
    { (yyval.e) = f_new_ec_item((yyvsp[(2) - (7)].i), 0, (yyvsp[(4) - (7)].i32), (yyvsp[(6) - (7)].i32), (yyvsp[(6) - (7)].i32)); }
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1675 "cf-parse.y"
    { (yyval.e) = f_new_ec_item((yyvsp[(2) - (9)].i), 0, (yyvsp[(4) - (9)].i32), (yyvsp[(6) - (9)].i32), (yyvsp[(8) - (9)].i32)); }
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1676 "cf-parse.y"
    {  (yyval.e) = f_new_ec_item((yyvsp[(2) - (7)].i), 0, (yyvsp[(4) - (7)].i32), 0, EC_ALL); }
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1682 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); }
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1683 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); }
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1689 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); }
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1690 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); }
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1695 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); }
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1700 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); }
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1704 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   }
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1711 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); }
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1712 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; }
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1713 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; }
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1714 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1721 "cf-parse.y"
    { (yyval.trie) = f_new_trie(cfg_mem); trie_add_fprefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); }
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1722 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_fprefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); }
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1725 "cf-parse.y"
    { (yyval.e) = NULL; }
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1726 "cf-parse.y"
    {
     /* Fill data fields */
     struct f_tree *t;
     for (t = (yyvsp[(2) - (4)].e); t; t = t->left)
       t->data = (yyvsp[(4) - (4)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (4)].e), (yyvsp[(2) - (4)].e));
   }
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1733 "cf-parse.y"
    { 
     struct f_tree *t = f_new_tree();
     t->from.type = t->to.type = T_VOID;
     t->right = t;
     t->data = (yyvsp[(3) - (3)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), t);
 }
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1745 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1746 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); }
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1750 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); }
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1751 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); }
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1755 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); }
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1756 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; }
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1757 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; }
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1758 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); }
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1759 "cf-parse.y"
    { (yyval.h) = NULL; }
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1763 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1764 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; }
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1765 "cf-parse.y"
    { (yyval.h) = NULL; }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1769 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); }
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1770 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  }
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1771 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  }
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1772 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1773 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); }
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1774 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); }
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1775 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); }
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1776 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); }
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1777 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); }
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1778 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; }
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1779 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; }
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1783 "cf-parse.y"
    { (yyval.x) = f_generate_dpair((yyvsp[(2) - (5)].x), (yyvsp[(4) - (5)].x)); }
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1784 "cf-parse.y"
    { (yyval.x) = f_generate_ec((yyvsp[(2) - (7)].i), (yyvsp[(4) - (7)].x), (yyvsp[(6) - (7)].x)); }
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1798 "cf-parse.y"
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

  case 340:

/* Line 1806 of yacc.c  */
#line 1821 "cf-parse.y"
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

  case 341:

/* Line 1806 of yacc.c  */
#line 1857 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; }
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1859 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; }
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1860 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ }
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1861 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ }
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1862 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); }
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1863 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; }
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1864 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); }
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1865 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); }
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1869 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); }
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1870 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1871 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1872 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1873 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1874 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1875 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1876 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1877 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1878 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1879 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1880 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); }
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1881 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); }
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1882 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); }
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1883 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); }
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1884 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); }
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1886 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1887 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1888 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1890 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; }
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1892 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; }
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1894 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); }
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1896 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; }
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1897 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1898 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); }
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1899 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1900 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); }
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1910 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; }
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1911 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; }
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1912 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_ECLIST; }
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1913 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); }
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1914 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; }
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1915 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; }
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1916 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'f'; }
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_generate_roa_check((yyvsp[(3) - (4)].s), NULL, NULL); }
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1919 "cf-parse.y"
    { (yyval.x) = f_generate_roa_check((yyvsp[(3) - (8)].s), (yyvsp[(5) - (8)].x), (yyvsp[(7) - (8)].x)); }
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1924 "cf-parse.y"
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

  case 386:

/* Line 1806 of yacc.c  */
#line 1947 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; }
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1948 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; }
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1949 "cf-parse.y"
    { (yyval.i) = F_REJECT; }
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1950 "cf-parse.y"
    { (yyval.i) = F_ERROR; }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1951 "cf-parse.y"
    { (yyval.i) = F_NOP; }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1952 "cf-parse.y"
    { (yyval.i) = F_NONL; }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1956 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1959 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1960 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1961 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   }
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1969 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   }
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1976 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   }
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1985 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1986 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); }
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1990 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   }
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1996 "cf-parse.y"
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

  case 402:

/* Line 1806 of yacc.c  */
#line 2006 "cf-parse.y"
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

  case 403:

/* Line 1806 of yacc.c  */
#line 2015 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 2021 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   }
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 2026 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 2033 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   }
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 2038 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 2044 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); }
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 2045 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); }
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 2046 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   }
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 2054 "cf-parse.y"
    { (yyval.x) = f_generate_empty((yyvsp[(2) - (5)].x)); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 2055 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 2056 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 2057 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 2058 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'f', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 2064 "cf-parse.y"
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

  case 419:

/* Line 1806 of yacc.c  */
#line 2087 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 2088 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(3) - (6)].a); BGP_CFG->local_as = (yyvsp[(5) - (6)].i); }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 2089 "cf-parse.y"
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

  case 422:

/* Line 1806 of yacc.c  */
#line 2099 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 2100 "cf-parse.y"
    { BGP_CFG->rr_client = 1; }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 2101 "cf-parse.y"
    { BGP_CFG->rs_client = 1; }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 2102 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 2103 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); }
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 2104 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); }
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 2105 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); }
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 2106 "cf-parse.y"
    { BGP_CFG->multihop = 64; }
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 2107 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (4)].i); if (((yyvsp[(3) - (4)].i)<1) || ((yyvsp[(3) - (4)].i)>255)) cf_error("Multihop must be in range 1-255"); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 2108 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; }
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 2109 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 2110 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 2111 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 2112 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_DIRECT; }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 2113 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_RECURSIVE; }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 2114 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 2115 "cf-parse.y"
    { BGP_CFG->med_metric = (yyvsp[(4) - (5)].i); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 2116 "cf-parse.y"
    { BGP_CFG->igp_metric = (yyvsp[(4) - (5)].i); }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 2117 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 2118 "cf-parse.y"
    { BGP_CFG->deterministic_med = (yyvsp[(4) - (5)].i); }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 2119 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 2120 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); }
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 2121 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); }
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 2122 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 2123 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 2124 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 2125 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 2126 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 2127 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 2128 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 2129 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 2130 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 2131 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 2132 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 2133 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 2134 "cf-parse.y"
    { BGP_CFG->igp_table = (yyvsp[(4) - (5)].r); }
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 2135 "cf-parse.y"
    { BGP_CFG->ttl_security = (yyvsp[(4) - (5)].i); }
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 2146 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config), (yyvsp[(1) - (2)].i));
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 2162 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 2163 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (2)].i) ? DEFAULT_ECMP_LIMIT : 0; }
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 2164 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (4)].i) ? (yyvsp[(4) - (4)].i) : 0; if ((yyvsp[(4) - (4)].i) < 0) cf_error("ECMP limit cannot be negative"); }
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 2165 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i); if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); }
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 2169 "cf-parse.y"
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

  case 469:

/* Line 1806 of yacc.c  */
#line 2184 "cf-parse.y"
    { ospf_area_finish(); }
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 2193 "cf-parse.y"
    { this_area->type = (yyvsp[(2) - (2)].i) ? 0 : OPT_E; /* We should remove the option */ }
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 2194 "cf-parse.y"
    { this_area->type = OPT_N; }
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 2195 "cf-parse.y"
    { this_area->summary = (yyvsp[(2) - (2)].i); }
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 2196 "cf-parse.y"
    { this_area->default_nssa = (yyvsp[(3) - (3)].i); }
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 2197 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i); check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 2198 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i) | LSA_EXT_EBIT; check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 2199 "cf-parse.y"
    { this_area->default_cost = (yyvsp[(3) - (3)].i);  check_defcost((yyvsp[(3) - (3)].i)); }
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2200 "cf-parse.y"
    { this_area->translator = (yyvsp[(2) - (2)].i); }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2201 "cf-parse.y"
    { this_area->transint = (yyvsp[(3) - (3)].i); }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2202 "cf-parse.y"
    { this_nets = &this_area->net_list; }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2203 "cf-parse.y"
    { this_nets = &this_area->enet_list; }
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2215 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   }
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2229 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); }
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2230 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); }
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2231 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); }
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2235 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2236 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2245 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2246 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); }
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2247 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2248 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; }
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2249 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); }
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2250 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); }
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2251 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2252 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2253 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2258 "cf-parse.y"
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

  case 512:

/* Line 1806 of yacc.c  */
#line 2279 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); }
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2280 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2281 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); }
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2282 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); }
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2283 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; }
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2284 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); }
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2285 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); }
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2286 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2287 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2288 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2289 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2290 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; }
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2291 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; }
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2292 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2293 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2294 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2295 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); }
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2296 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; }
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2297 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; }
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2298 "cf-parse.y"
    { OSPF_PATT->check_link = (yyvsp[(3) - (3)].i); }
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2299 "cf-parse.y"
    { OSPF_PATT->ecmp_weight = (yyvsp[(3) - (3)].i) - 1; if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("ECMP weight must be in range 1-256"); }
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2301 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2302 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2303 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2304 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; }
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2305 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; }
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2306 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) || ((yyvsp[(3) - (3)].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); }
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2318 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(this_nets, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (1)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (1)].px).len;
 }
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2328 "cf-parse.y"
    { this_pref->hidden = 1; }
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2329 "cf-parse.y"
    { this_pref->tag = (yyvsp[(2) - (2)].i); }
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2342 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 }
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2351 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 }
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2361 "cf-parse.y"
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

  case 559:

/* Line 1806 of yacc.c  */
#line 2392 "cf-parse.y"
    { ospf_iface_finish(); }
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2397 "cf-parse.y"
    { (yyval.t) = NULL; }
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2403 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); }
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2406 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); }
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2409 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); }
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2414 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); }
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2417 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); }
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2422 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); }
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2425 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); }
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2429 "cf-parse.y"
    { ospf_sh_lsadb((yyvsp[(4) - (5)].ld)); }
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2432 "cf-parse.y"
    {
     (yyval.ld) = cfg_allocz(sizeof(struct lsadb_show_data));
   }
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2435 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->scope = LSA_SCOPE_AS; }
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2436 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->scope = LSA_SCOPE_AREA; (yyval.ld)->area = (yyvsp[(3) - (3)].i32) ;}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2437 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->scope = 1; /* hack, 0 is no filter */ }
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2438 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->type = (yyvsp[(3) - (3)].i); }
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2439 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->lsid = (yyvsp[(3) - (3)].i32); }
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2440 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->router = SH_ROUTER_SELF; }
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2441 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (3)].ld); (yyval.ld)->router = (yyvsp[(3) - (3)].i32); }
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2442 "cf-parse.y"
    { (yyval.ld) = (yyvsp[(1) - (2)].ld); (yyval.ld)->name = (yyvsp[(2) - (2)].s); }
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2448 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config), (yyvsp[(1) - (2)].i));
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  }
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2457 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   }
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2462 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; }
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2463 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; }
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2469 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config), (yyvsp[(1) - (2)].i));
     rip_init_config(RIP_CFG);
   }
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2478 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); }
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2479 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); }
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2480 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); }
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2481 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); }
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2482 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); }
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2483 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); }
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2485 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; }
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2486 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; }
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2487 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; }
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2492 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; }
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2493 "cf-parse.y"
    { (yyval.i)=AT_MD5; }
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2494 "cf-parse.y"
    { (yyval.i)=AT_NONE; }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2499 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2500 "cf-parse.y"
    { (yyval.i)=0; }
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2501 "cf-parse.y"
    { (yyval.i)=IM_QUIET; }
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2502 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; }
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2503 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; }
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2507 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); }
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2508 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); }
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2522 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   }
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2538 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config), (yyvsp[(1) - (2)].i));
     static_init_config((struct static_config *) this_proto);
  }
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2547 "cf-parse.y"
    { STATIC_CFG->check_link = (yyvsp[(4) - (5)].i); }
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2548 "cf-parse.y"
    { STATIC_CFG->igp_table = (yyvsp[(4) - (5)].r); }
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2552 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  }
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2561 "cf-parse.y"
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[(2) - (3)].a);
     this_srt_nh->via_if = (yyvsp[(3) - (3)].iface);
     this_srt_nh->if_name = (void *) this_srt; /* really */
   }
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2569 "cf-parse.y"
    {
     this_srt_nh->masklen = (yyvsp[(3) - (3)].i) - 1; /* really */
     if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("Weight must be in range 1-256"); 
   }
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2576 "cf-parse.y"
    { this_srt->mp_next = this_srt_nh; }
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2577 "cf-parse.y"
    { last_srt_nh->mp_next = this_srt_nh; }
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2581 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (4)].a);
      this_srt->via_if = (yyvsp[(4) - (4)].iface);
   }
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2586 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   }
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2592 "cf-parse.y"
    {
      this_srt->dest = RTD_MULTIPATH;
   }
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2595 "cf-parse.y"
    {
      this_srt->dest = RTDX_RECURSIVE;
      this_srt->via = (yyvsp[(3) - (3)].a);
   }
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2599 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; }
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2600 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; }
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2601 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; }
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2605 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); }
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2613 "cf-parse.y"
    { bgp_check_config(BGP_CFG); }
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2613 "cf-parse.y"
    { ospf_proto_finish(); }
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2613 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); }
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2616 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); }
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2616 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); }
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2617 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); }
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2617 "cf-parse.y"
    { (yyval.x) = NULL; }
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2618 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); }
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2619 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); }
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2620 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); }
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2621 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); }
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2622 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); }
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2623 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); }
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2624 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); }
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2625 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); }
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2626 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); }
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2627 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); }
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_EC_SET, T_ECLIST, EA_CODE(EAP_BGP, BA_EXT_COMMUNITY)); }
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); }
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); }
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); }
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); }
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); }
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2628 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); }
    break;



/* Line 1806 of yacc.c  */
#line 8347 "cf-parse.tab.c"
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
#line 2630 "cf-parse.y"

/* C Code from ../../conf/confbase.Y */

/* C Code from ../../sysdep/unix/config.Y */

/* C Code from ../../sysdep/unix/krt.Y */

/* C Code from ../../sysdep/linux/netlink/netlink.Y */

/* C Code from ../../nest/config.Y */

/* C Code from ../../proto/bgp/config.Y */

/* C Code from ../../proto/ospf/config.Y */

/* C Code from ../../proto/pipe/config.Y */

/* C Code from ../../proto/rip/config.Y */

/* C Code from ../../proto/static/config.Y */



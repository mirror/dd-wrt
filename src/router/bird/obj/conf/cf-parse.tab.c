
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

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

/* Line 189 of yacc.c  */
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
  if ((this_area->areaid == 0) && (this_area->stub != 0))
    cf_error( "Backbone area cannot be stub");
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
    init_list(&ac->stubnet_list);
  }

  if (!cf->abr && !EMPTY_LIST(cf->vlink_list))
    cf_error( "No configured areas in OSPF");
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



/* Line 189 of yacc.c  */
#line 347 "cf-parse.tab.c"

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
     PREFERENCE = 318,
     DISABLED = 319,
     DIRECT = 320,
     INTERFACE = 321,
     IMPORT = 322,
     EXPORT = 323,
     FILTER = 324,
     NONE = 325,
     STATES = 326,
     FILTERS = 327,
     PASSWORD = 328,
     FROM = 329,
     PASSIVE = 330,
     TO = 331,
     EVENTS = 332,
     PACKETS = 333,
     PROTOCOLS = 334,
     INTERFACES = 335,
     PRIMARY = 336,
     STATS = 337,
     COUNT = 338,
     FOR = 339,
     COMMANDS = 340,
     PREEXPORT = 341,
     GENERATE = 342,
     LISTEN = 343,
     BGP = 344,
     V6ONLY = 345,
     DUAL = 346,
     ADDRESS = 347,
     PORT = 348,
     PASSWORDS = 349,
     DESCRIPTION = 350,
     RELOAD = 351,
     IN = 352,
     OUT = 353,
     MRTDUMP = 354,
     MESSAGES = 355,
     RESTRICT = 356,
     MEMORY = 357,
     IGP_METRIC = 358,
     SHOW = 359,
     STATUS = 360,
     SUMMARY = 361,
     ROUTE = 362,
     SYMBOLS = 363,
     DUMP = 364,
     RESOURCES = 365,
     SOCKETS = 366,
     NEIGHBORS = 367,
     ATTRIBUTES = 368,
     ECHO = 369,
     DISABLE = 370,
     ENABLE = 371,
     RESTART = 372,
     FUNCTION = 373,
     PRINT = 374,
     PRINTN = 375,
     UNSET = 376,
     RETURN = 377,
     ACCEPT = 378,
     REJECT = 379,
     QUITBIRD = 380,
     INT = 381,
     BOOL = 382,
     IP = 383,
     PREFIX = 384,
     PAIR = 385,
     QUAD = 386,
     SET = 387,
     STRING = 388,
     BGPMASK = 389,
     BGPPATH = 390,
     CLIST = 391,
     IF = 392,
     THEN = 393,
     ELSE = 394,
     CASE = 395,
     TRUE = 396,
     FALSE = 397,
     GW = 398,
     NET = 399,
     MASK = 400,
     PROTO = 401,
     SOURCE = 402,
     SCOPE = 403,
     CAST = 404,
     DEST = 405,
     LEN = 406,
     DEFINED = 407,
     ADD = 408,
     DELETE = 409,
     CONTAINS = 410,
     RESET = 411,
     PREPEND = 412,
     FIRST = 413,
     LAST = 414,
     MATCH = 415,
     EMPTY = 416,
     WHERE = 417,
     EVAL = 418,
     LOCAL = 419,
     NEIGHBOR = 420,
     AS = 421,
     HOLD = 422,
     CONNECT = 423,
     RETRY = 424,
     KEEPALIVE = 425,
     MULTIHOP = 426,
     STARTUP = 427,
     VIA = 428,
     NEXT = 429,
     HOP = 430,
     SELF = 431,
     DEFAULT = 432,
     PATH = 433,
     METRIC = 434,
     START = 435,
     DELAY = 436,
     FORGET = 437,
     WAIT = 438,
     AFTER = 439,
     BGP_PATH = 440,
     BGP_LOCAL_PREF = 441,
     BGP_MED = 442,
     BGP_ORIGIN = 443,
     BGP_NEXT_HOP = 444,
     BGP_ATOMIC_AGGR = 445,
     BGP_AGGREGATOR = 446,
     BGP_COMMUNITY = 447,
     RR = 448,
     RS = 449,
     CLIENT = 450,
     CLUSTER = 451,
     AS4 = 452,
     ADVERTISE = 453,
     IPV4 = 454,
     CAPABILITIES = 455,
     LIMIT = 456,
     PREFER = 457,
     OLDER = 458,
     MISSING = 459,
     LLADDR = 460,
     DROP = 461,
     IGNORE = 462,
     REFRESH = 463,
     INTERPRET = 464,
     COMMUNITIES = 465,
     BGP_ORIGINATOR_ID = 466,
     BGP_CLUSTER_LIST = 467,
     IGP = 468,
     GATEWAY = 469,
     RECURSIVE = 470,
     MED = 471,
     OSPF = 472,
     AREA = 473,
     OSPF_METRIC1 = 474,
     OSPF_METRIC2 = 475,
     OSPF_TAG = 476,
     OSPF_ROUTER_ID = 477,
     RFC1583COMPAT = 478,
     STUB = 479,
     TICK = 480,
     COST = 481,
     RETRANSMIT = 482,
     HELLO = 483,
     TRANSMIT = 484,
     PRIORITY = 485,
     DEAD = 486,
     TYPE = 487,
     BROADCAST = 488,
     BCAST = 489,
     NONBROADCAST = 490,
     NBMA = 491,
     POINTOPOINT = 492,
     PTP = 493,
     POINTOMULTIPOINT = 494,
     PTMP = 495,
     SIMPLE = 496,
     AUTHENTICATION = 497,
     STRICT = 498,
     CRYPTOGRAPHIC = 499,
     ELIGIBLE = 500,
     POLL = 501,
     NETWORKS = 502,
     HIDDEN = 503,
     VIRTUAL = 504,
     CHECK = 505,
     LINK = 506,
     RX = 507,
     BUFFER = 508,
     LARGE = 509,
     NORMAL = 510,
     STUBNET = 511,
     LSADB = 512,
     ECMP = 513,
     WEIGHT = 514,
     TOPOLOGY = 515,
     STATE = 516,
     PIPE = 517,
     PEER = 518,
     MODE = 519,
     OPAQUE = 520,
     TRANSPARENT = 521,
     RIP = 522,
     INFINITY = 523,
     PERIOD = 524,
     GARBAGE = 525,
     TIMEOUT = 526,
     MULTICAST = 527,
     QUIET = 528,
     NOLISTEN = 529,
     VERSION1 = 530,
     PLAINTEXT = 531,
     MD5 = 532,
     HONOR = 533,
     NEVER = 534,
     ALWAYS = 535,
     RIP_METRIC = 536,
     RIP_TAG = 537,
     STATIC = 538,
     PROHIBIT = 539,
     MULTIPATH = 540
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 268 "cf-parse.y"

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



/* Line 214 of yacc.c  */
#line 692 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 704 "cf-parse.tab.c"

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
# if YYENABLE_NLS
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
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

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2027

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  307
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  206
/* YYNRULES -- Number of rules.  */
#define YYNRULES  649
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1212

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   540

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    30,     2,     2,
     297,   298,    28,    26,   303,    27,    32,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   300,   299,
      23,    22,    24,   304,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   305,     2,   306,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   301,     2,   302,    25,     2,     2,     2,
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
     296
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    13,    15,    19,    21,
      27,    33,    35,    37,    39,    41,    43,    44,    46,    48,
      51,    53,    55,    58,    61,    63,    65,    66,    71,    74,
      75,    77,    80,    82,    84,    88,    90,    94,    96,    98,
     100,   102,   104,   106,   108,   110,   112,   117,   121,   123,
     125,   127,   129,   132,   137,   141,   145,   149,   153,   158,
     161,   162,   164,   167,   170,   174,   177,   181,   184,   188,
     192,   196,   201,   203,   205,   207,   212,   213,   216,   219,
     222,   224,   226,   229,   231,   232,   234,   235,   238,   241,
     244,   247,   250,   253,   256,   260,   263,   266,   268,   270,
     272,   274,   278,   282,   283,   285,   287,   290,   291,   293,
     297,   299,   303,   306,   310,   314,   318,   319,   323,   325,
     327,   331,   333,   337,   339,   341,   343,   345,   347,   349,
     351,   353,   357,   359,   363,   365,   367,   372,   374,   375,
     379,   384,   386,   389,   390,   396,   402,   408,   414,   419,
     423,   427,   432,   438,   440,   441,   445,   450,   455,   456,
     459,   463,   467,   471,   474,   477,   480,   484,   488,   491,
     494,   496,   498,   503,   507,   511,   515,   519,   523,   527,
     531,   536,   538,   540,   542,   543,   545,   549,   553,   557,
     561,   566,   571,   576,   581,   584,   586,   588,   590,   592,
     593,   595,   596,   601,   604,   606,   608,   610,   612,   614,
     616,   618,   620,   622,   624,   627,   630,   631,   635,   637,
     641,   643,   645,   647,   650,   654,   657,   662,   663,   669,
     670,   672,   674,   677,   679,   683,   685,   687,   689,   691,
     693,   695,   699,   701,   703,   705,   707,   709,   713,   715,
     721,   733,   735,   737,   741,   743,   745,   749,   751,   755,
     757,   761,   765,   767,   770,   773,   780,   782,   786,   787,
     792,   796,   798,   802,   806,   810,   813,   816,   819,   822,
     823,   826,   829,   830,   836,   838,   840,   842,   844,   846,
     848,   850,   854,   858,   860,   862,   863,   868,   870,   872,
     874,   876,   878,   880,   882,   884,   886,   890,   894,   898,
     902,   906,   910,   914,   918,   922,   926,   930,   934,   938,
     942,   945,   950,   952,   954,   956,   958,   961,   964,   968,
     972,   979,   983,   987,   991,   995,  1002,  1009,  1016,  1023,
    1028,  1030,  1032,  1034,  1036,  1038,  1040,  1042,  1043,  1045,
    1049,  1051,  1055,  1056,  1058,  1063,  1070,  1075,  1079,  1085,
    1091,  1096,  1103,  1107,  1110,  1116,  1122,  1131,  1140,  1149,
    1158,  1161,  1165,  1169,  1175,  1182,  1189,  1196,  1201,  1206,
    1212,  1219,  1226,  1232,  1236,  1241,  1247,  1253,  1259,  1265,
    1270,  1275,  1281,  1287,  1293,  1299,  1305,  1311,  1317,  1324,
    1331,  1340,  1347,  1354,  1360,  1365,  1371,  1376,  1382,  1387,
    1393,  1399,  1402,  1406,  1410,  1412,  1415,  1418,  1423,  1426,
    1428,  1431,  1436,  1437,  1441,  1445,  1448,  1453,  1456,  1459,
    1461,  1466,  1468,  1470,  1471,  1475,  1478,  1481,  1484,  1489,
    1491,  1492,  1496,  1497,  1500,  1503,  1507,  1510,  1513,  1517,
    1520,  1523,  1526,  1528,  1532,  1535,  1538,  1541,  1544,  1547,
    1550,  1554,  1557,  1560,  1563,  1566,  1569,  1572,  1575,  1578,
    1582,  1585,  1589,  1592,  1596,  1600,  1605,  1608,  1611,  1614,
    1618,  1622,  1626,  1628,  1629,  1632,  1634,  1636,  1639,  1643,
    1644,  1647,  1649,  1651,  1654,  1658,  1659,  1660,  1664,  1665,
    1669,  1673,  1675,  1676,  1681,  1688,  1695,  1702,  1710,  1717,
    1725,  1732,  1735,  1739,  1743,  1749,  1754,  1759,  1762,  1766,
    1770,  1775,  1780,  1785,  1791,  1797,  1802,  1806,  1811,  1816,
    1821,  1826,  1828,  1830,  1832,  1834,  1836,  1838,  1840,  1842,
    1843,  1846,  1849,  1850,  1854,  1855,  1859,  1860,  1864,  1867,
    1871,  1875,  1881,  1885,  1888,  1891,  1895,  1897,  1900,  1904,
    1908,  1912,  1915,  1918,  1921,  1926,  1928,  1930,  1932,  1934,
    1936,  1938,  1940,  1942,  1944,  1946,  1948,  1950,  1952,  1954,
    1956,  1958,  1960,  1962,  1964,  1966,  1968,  1970,  1972,  1974,
    1976,  1978,  1980,  1982,  1984,  1986,  1988,  1990,  1992,  1994,
    1996,  1998,  2000,  2002,  2004,  2006,  2008,  2010,  2012,  2014,
    2016,  2018,  2020,  2022,  2024,  2026,  2029,  2032,  2035,  2038,
    2041,  2044,  2047,  2050,  2054,  2058,  2062,  2066,  2070,  2074,
    2078,  2080,  2082,  2084,  2086,  2088,  2090,  2092,  2094,  2096,
    2098,  2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,  2116
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     308,     0,    -1,   309,     3,    -1,     4,   508,    -1,    -1,
     309,   507,    -1,    15,    -1,   297,   443,   298,    -1,    19,
      -1,    33,    19,    22,   310,   299,    -1,    33,    19,    22,
      18,   299,    -1,   310,    -1,    34,    -1,    36,    -1,    35,
      -1,    37,    -1,    -1,    18,    -1,    19,    -1,   313,   316,
      -1,   314,    -1,   313,    -1,    29,   310,    -1,   300,   313,
      -1,    20,    -1,    20,    -1,    -1,    38,   321,   322,   299,
      -1,    57,    20,    -1,    -1,    20,    -1,    39,   320,    -1,
      50,    -1,    40,    -1,   301,   323,   302,    -1,   324,    -1,
     323,   303,   324,    -1,    41,    -1,    42,    -1,    43,    -1,
      44,    -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,
      49,    -1,   110,    90,   362,   299,    -1,   110,    20,   299,
      -1,   118,    -1,    73,    -1,    56,    -1,    38,    -1,   326,
      20,    -1,   326,    20,   310,    20,    -1,   326,    53,    54,
      -1,   326,    53,    55,    -1,    52,   327,   299,    -1,    58,
     332,     3,    -1,    58,    51,   332,     3,    -1,    59,     3,
      -1,    -1,    20,    -1,   344,    60,    -1,    61,   312,    -1,
      62,    63,   310,    -1,    64,   312,    -1,    65,    66,   312,
      -1,   344,    65,    -1,    62,    63,   310,    -1,    92,   318,
     315,    -1,    60,    68,   310,    -1,    71,    72,   339,   299,
      -1,    15,    -1,    17,    -1,    18,    -1,    99,   100,   341,
     299,    -1,    -1,   341,   342,    -1,   103,   313,    -1,   104,
     310,    -1,   101,    -1,   102,    -1,    68,    19,    -1,    73,
      -1,    -1,    19,    -1,    -1,    74,   310,    -1,    75,   312,
      -1,    41,   359,    -1,   110,   362,    -1,    78,   347,    -1,
      79,   347,    -1,    68,   348,    -1,    71,    72,   339,    -1,
     106,    20,    -1,    80,   410,    -1,   411,    -1,    40,    -1,
      81,    -1,    19,    -1,    41,    90,   359,    -1,    41,    96,
     310,    -1,    -1,    20,    -1,   315,    -1,    20,   315,    -1,
      -1,    27,    -1,   350,   352,   351,    -1,   353,    -1,   354,
     303,   353,    -1,   344,    76,    -1,   355,   345,   301,    -1,
     356,   346,   299,    -1,   356,   358,   299,    -1,    -1,    77,
     357,   354,    -1,    40,    -1,    35,    -1,   301,   360,   302,
      -1,   361,    -1,   360,   303,   361,    -1,    82,    -1,    66,
      -1,    83,    -1,    91,    -1,    88,    -1,    89,    -1,    40,
      -1,    35,    -1,   301,   363,   302,    -1,   364,    -1,   363,
     303,   364,    -1,    82,    -1,   111,    -1,   105,   301,   366,
     302,    -1,   367,    -1,    -1,   367,   299,   366,    -1,   368,
     301,   369,   302,    -1,   368,    -1,    84,    20,    -1,    -1,
      98,    85,   317,   299,   369,    -1,    98,    87,   317,   299,
     369,    -1,   134,    85,   317,   299,   369,    -1,   134,    87,
     317,   299,   369,    -1,    72,   310,   299,   369,    -1,   115,
     116,     3,    -1,   115,   113,     3,    -1,   115,    90,   401,
       3,    -1,   115,    90,    40,   401,     3,    -1,    19,    -1,
      -1,   115,    91,     3,    -1,   115,    91,   117,     3,    -1,
     115,   118,   378,     3,    -1,    -1,   378,   314,    -1,   378,
      95,   315,    -1,   378,    68,    19,    -1,   378,    80,   410,
      -1,   378,   411,    -1,   378,    40,    -1,   378,    92,    -1,
     378,   379,    19,    -1,   378,    73,    19,    -1,   378,    93,
      -1,   378,    94,    -1,    97,    -1,    79,    -1,   115,   119,
     374,     3,    -1,   120,   121,     3,    -1,   120,   122,     3,
      -1,   120,    91,     3,    -1,   120,   123,     3,    -1,   120,
     124,     3,    -1,   120,    66,     3,    -1,   120,    90,     3,
      -1,   125,   389,   390,     3,    -1,    40,    -1,    35,    -1,
      15,    -1,    -1,    15,    -1,   126,   400,     3,    -1,   127,
     400,     3,    -1,   128,   400,     3,    -1,   107,   400,     3,
      -1,   107,   108,   400,     3,    -1,   107,   109,   400,     3,
      -1,    41,   400,   359,     3,    -1,   110,   400,   362,     3,
      -1,   112,     3,    -1,    19,    -1,    40,    -1,    20,    -1,
      19,    -1,    -1,    20,    -1,    -1,    80,    19,   403,   409,
      -1,   174,   443,    -1,   137,    -1,   138,    -1,   139,    -1,
     140,    -1,   141,    -1,   142,    -1,   144,    -1,   145,    -1,
     146,    -1,   147,    -1,   405,   143,    -1,   405,    19,    -1,
      -1,   406,   299,   407,    -1,   406,    -1,   408,   299,   406,
      -1,   413,    -1,    19,    -1,   409,    -1,   173,   443,    -1,
     297,   408,   298,    -1,   297,   298,    -1,   407,   301,   416,
     302,    -1,    -1,   129,    19,   415,   412,   413,    -1,    -1,
     417,    -1,   449,    -1,   417,   449,    -1,   449,    -1,   301,
     416,   302,    -1,    18,    -1,   310,    -1,    17,    -1,   419,
      -1,    16,    -1,    15,    -1,   297,   443,   298,    -1,    17,
      -1,   419,    -1,    16,    -1,   443,    -1,   422,    -1,   422,
       7,   422,    -1,    28,    -1,   297,   423,   303,   423,   298,
      -1,   297,   423,   303,   423,   298,     7,   297,   422,   303,
     422,   298,    -1,   424,    -1,   420,    -1,   420,     7,   420,
      -1,   424,    -1,   421,    -1,   421,     7,   421,    -1,   425,
      -1,   427,   303,   425,    -1,   426,    -1,   428,   303,   426,
      -1,    18,    29,    15,    -1,   429,    -1,   429,    26,    -1,
     429,    27,    -1,   429,   301,    15,   303,    15,   302,    -1,
     430,    -1,   431,   303,   430,    -1,    -1,   432,   428,   300,
     416,    -1,   432,     6,   416,    -1,   441,    -1,   297,   443,
     298,    -1,    13,   435,    14,    -1,    29,   436,    29,    -1,
      15,   435,    -1,    28,   435,    -1,   304,   435,    -1,   433,
     435,    -1,    -1,    15,   436,    -1,   304,   436,    -1,    -1,
     297,   443,   303,   443,   298,    -1,    15,    -1,   152,    -1,
     153,    -1,    20,    -1,   419,    -1,   429,    -1,    17,    -1,
     305,   427,   306,    -1,   305,   431,   306,    -1,    16,    -1,
     434,    -1,    -1,    19,   297,   448,   298,    -1,    19,    -1,
      85,    -1,   154,    -1,   155,    -1,   157,    -1,   158,    -1,
     159,    -1,   160,    -1,   161,    -1,   297,   443,   298,    -1,
     443,    26,   443,    -1,   443,    27,   443,    -1,   443,    28,
     443,    -1,   443,    29,   443,    -1,   443,    11,   443,    -1,
     443,    12,   443,    -1,   443,    22,   443,    -1,   443,    10,
     443,    -1,   443,    23,   443,    -1,   443,     9,   443,    -1,
     443,    24,   443,    -1,   443,     8,   443,    -1,   443,    25,
     443,    -1,    31,   443,    -1,   163,   297,   443,   298,    -1,
     441,    -1,   438,    -1,   437,    -1,    74,    -1,   439,   442,
      -1,   439,   512,    -1,   443,    32,   139,    -1,   443,    32,
     162,    -1,   443,    32,   156,   297,   443,   298,    -1,   443,
      32,   169,    -1,   443,    32,   170,    -1,    26,   172,    26,
      -1,    27,   172,    27,    -1,   168,   297,   443,   303,   443,
     298,    -1,   164,   297,   443,   303,   443,   298,    -1,   165,
     297,   443,   303,   443,   298,    -1,    80,   297,   443,   303,
     443,   298,    -1,    19,   297,   448,   298,    -1,   136,    -1,
     134,    -1,   135,    -1,    46,    -1,   130,    -1,   131,    -1,
     443,    -1,    -1,   445,    -1,   445,   303,   446,    -1,   443,
      -1,   443,   303,   447,    -1,    -1,   447,    -1,   148,   443,
     149,   418,    -1,   148,   443,   149,   418,   150,   418,    -1,
      19,    22,   443,   299,    -1,   133,   443,   299,    -1,   439,
     512,    22,   443,   299,    -1,   439,   442,    22,   443,   299,
      -1,    74,    22,   443,   299,    -1,   132,   297,   439,   512,
     298,   299,    -1,   444,   446,   299,    -1,   440,   299,    -1,
     151,   443,   301,   432,   302,    -1,   439,   512,    32,   172,
     299,    -1,   439,   512,    32,   168,   297,   443,   298,   299,
      -1,   439,   512,    32,   164,   297,   443,   298,   299,    -1,
     439,   512,    32,   165,   297,   443,   298,   299,    -1,   439,
     512,    32,    80,   297,   443,   298,   299,    -1,   344,   100,
      -1,   450,   345,   301,    -1,   451,   346,   299,    -1,   451,
     175,   177,   310,   299,    -1,   451,   175,   313,   177,   310,
     299,    -1,   451,   176,   313,   177,   310,   299,    -1,   451,
     204,   207,    72,   339,   299,    -1,   451,   204,   206,   299,
      -1,   451,   205,   206,   299,    -1,   451,   178,    63,   310,
     299,    -1,   451,   183,   178,    63,   310,   299,    -1,   451,
     179,   180,    63,   310,   299,    -1,   451,   181,    63,   310,
     299,    -1,   451,   182,   299,    -1,   451,   182,   310,   299,
      -1,   451,   185,   186,   187,   299,    -1,   451,   215,   216,
     187,   299,    -1,   451,   215,   216,   217,   299,    -1,   451,
     215,   216,   218,   299,    -1,   451,   225,    76,   299,    -1,
     451,   225,   226,   299,    -1,   451,   189,   190,   312,   299,
      -1,   451,   227,   190,   312,   299,    -1,   451,   224,   190,
     312,   299,    -1,   451,   213,   214,   312,   299,    -1,   451,
     188,   198,   310,   299,    -1,   451,   188,   197,   310,   299,
      -1,   451,   158,   103,   313,   299,    -1,   451,   191,   192,
      63,   310,   299,    -1,   451,    46,   193,    63,   310,   299,
      -1,   451,    46,   194,    63,   310,   303,   310,   299,    -1,
     451,   126,   195,    46,   312,   299,    -1,   451,   127,   118,
     219,   312,   299,    -1,   451,   127,   208,   312,   299,    -1,
     451,   211,   312,   299,    -1,   451,   209,   210,   312,   299,
      -1,   451,    84,    20,   299,    -1,   451,   118,   212,   310,
     299,    -1,   451,    86,   312,   299,    -1,   451,   220,   221,
     312,   299,    -1,   451,   224,    68,   348,   299,    -1,   344,
     228,    -1,   452,   345,   301,    -1,   453,   454,   299,    -1,
     346,    -1,   234,   312,    -1,   269,   312,    -1,   269,   312,
     212,   310,    -1,   236,   310,    -1,   456,    -1,   229,   339,
      -1,   455,   301,   457,   302,    -1,    -1,   457,   458,   299,
      -1,   235,   237,   310,    -1,   235,   312,    -1,   258,   301,
     468,   302,    -1,   267,   459,    -1,    77,   479,    -1,   463,
      -1,   460,   301,   461,   302,    -1,   460,    -1,   314,    -1,
      -1,   461,   462,   299,    -1,   259,   312,    -1,   117,   312,
      -1,   237,   310,    -1,   466,   301,   464,   302,    -1,   466,
      -1,    -1,   464,   465,   299,    -1,    -1,   239,   310,    -1,
     238,   310,    -1,   240,   192,   310,    -1,   194,   310,    -1,
     242,   310,    -1,   242,    94,   310,    -1,   253,    81,    -1,
     253,   252,    -1,   253,   255,    -1,   365,    -1,   260,   262,
     339,    -1,   237,   310,    -1,   239,   310,    -1,   257,   310,
      -1,   238,   310,    -1,   194,   310,    -1,   242,   310,    -1,
     242,    94,   310,    -1,   243,   244,    -1,   243,   245,    -1,
     243,   246,    -1,   243,   247,    -1,   243,   248,    -1,   243,
     249,    -1,   243,   250,    -1,   243,   251,    -1,   240,   192,
     310,    -1,   241,   310,    -1,   254,   246,   312,    -1,   235,
     312,    -1,   261,   262,   312,    -1,   269,   270,   310,    -1,
     123,   301,   472,   302,    -1,   253,    81,    -1,   253,   252,
      -1,   253,   255,    -1,   263,   264,   265,    -1,   263,   264,
     266,    -1,   263,   264,   310,    -1,   365,    -1,    -1,   468,
     469,    -1,   470,    -1,   471,    -1,   314,   299,    -1,   314,
     259,   299,    -1,    -1,   472,   473,    -1,   474,    -1,   475,
      -1,    18,   299,    -1,    18,   256,   299,    -1,    -1,    -1,
     477,   467,   299,    -1,    -1,   301,   477,   302,    -1,   476,
     354,   478,    -1,    20,    -1,    -1,   115,   228,   374,     3,
      -1,   115,   228,   123,   374,   480,     3,    -1,   115,   228,
      77,   374,   480,     3,    -1,   115,   228,   271,   374,   480,
       3,    -1,   115,   228,   271,    40,   374,   480,     3,    -1,
     115,   228,   272,   374,   480,     3,    -1,   115,   228,   272,
      40,   374,   480,     3,    -1,   115,   228,   268,   374,   480,
       3,    -1,   344,   273,    -1,   489,   345,   301,    -1,   490,
     346,   299,    -1,   490,   274,    68,    19,   299,    -1,   490,
     275,   276,   299,    -1,   490,   275,   277,   299,    -1,   344,
     278,    -1,   491,   345,   301,    -1,   492,   346,   299,    -1,
     492,   279,   310,   299,    -1,   492,   104,   310,   299,    -1,
     492,   280,   310,   299,    -1,   492,   281,    63,   310,   299,
      -1,   492,   282,    63,   310,   299,    -1,   492,   253,   493,
     299,    -1,   492,   365,   299,    -1,   492,   289,   291,   299,
      -1,   492,   289,   176,   299,    -1,   492,   289,   290,   299,
      -1,   492,    77,   499,   299,    -1,   287,    -1,   288,    -1,
      81,    -1,   244,    -1,   283,    -1,   284,    -1,   285,    -1,
     286,    -1,    -1,   190,   310,    -1,   275,   494,    -1,    -1,
     496,   495,   299,    -1,    -1,   301,   496,   302,    -1,    -1,
     498,   354,   497,    -1,   344,   294,    -1,   500,   345,   301,
      -1,   501,   346,   299,    -1,   501,   261,   262,   312,   299,
      -1,   501,   505,   299,    -1,   118,   314,    -1,   184,   313,
      -1,   503,   270,   310,    -1,   503,    -1,   504,   503,    -1,
     502,   184,   313,    -1,   502,   184,    20,    -1,   502,   296,
     504,    -1,   502,   217,    -1,   502,   135,    -1,   502,   295,
      -1,   115,   294,   374,     3,    -1,   299,    -1,   311,    -1,
     319,    -1,   325,    -1,   328,    -1,   338,    -1,   340,    -1,
     343,    -1,   509,    -1,   349,    -1,   402,    -1,   404,    -1,
     414,    -1,   329,    -1,   330,    -1,   331,    -1,   370,    -1,
     371,    -1,   372,    -1,   373,    -1,   375,    -1,   376,    -1,
     377,    -1,   380,    -1,   381,    -1,   382,    -1,   383,    -1,
     384,    -1,   385,    -1,   386,    -1,   387,    -1,   388,    -1,
     391,    -1,   392,    -1,   393,    -1,   394,    -1,   395,    -1,
     396,    -1,   397,    -1,   398,    -1,   399,    -1,   481,    -1,
     482,    -1,   483,    -1,   484,    -1,   485,    -1,   486,    -1,
     487,    -1,   488,    -1,   506,    -1,   510,   302,    -1,   511,
     302,    -1,   356,   302,    -1,   451,   302,    -1,   453,   302,
      -1,   490,   302,    -1,   492,   302,    -1,   501,   302,    -1,
     333,   345,   301,    -1,   510,   346,   299,    -1,   510,   334,
     299,    -1,   510,   337,   299,    -1,   335,   345,   301,    -1,
     511,   346,   299,    -1,   511,   336,   299,    -1,    69,    -1,
      70,    -1,   114,    -1,     5,    -1,   199,    -1,   196,    -1,
     200,    -1,   198,    -1,   197,    -1,   201,    -1,   202,    -1,
     203,    -1,   222,    -1,   223,    -1,   230,    -1,   231,    -1,
     232,    -1,   233,    -1,   292,    -1,   293,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   428,   428,   429,   432,   434,   441,   442,   443,   449,
     453,   462,   463,   464,   465,   466,   467,   473,   474,   481,
     488,   489,   493,   497,   504,   512,   513,   519,   528,   529,
     533,   538,   539,   543,   544,   548,   549,   553,   554,   555,
     556,   557,   558,   559,   560,   561,   567,   568,   577,   578,
     579,   580,   583,   584,   585,   586,   590,   597,   600,   603,
     607,   608,   616,   632,   633,   637,   644,   650,   663,   667,
     680,   693,   699,   700,   701,   712,   714,   716,   720,   721,
     722,   723,   730,   738,   742,   748,   754,   756,   760,   761,
     762,   763,   764,   765,   766,   767,   771,   772,   773,   774,
     778,   786,   787,   795,   803,   804,   805,   809,   810,   814,
     819,   820,   827,   836,   837,   838,   842,   851,   857,   858,
     859,   863,   864,   868,   869,   870,   871,   872,   873,   879,
     880,   881,   885,   886,   890,   891,   897,   898,   901,   903,
     907,   908,   912,   930,   931,   932,   933,   934,   935,   943,
     946,   949,   952,   956,   957,   960,   963,   966,   970,   976,
     982,   989,   994,   999,  1004,  1008,  1012,  1022,  1030,  1034,
    1041,  1042,  1045,  1049,  1051,  1053,  1055,  1057,  1059,  1061,
    1064,  1070,  1071,  1072,  1076,  1077,  1083,  1085,  1087,  1089,
    1091,  1093,  1097,  1101,  1104,  1108,  1109,  1110,  1114,  1115,
    1116,  1124,  1124,  1134,  1138,  1139,  1140,  1141,  1142,  1143,
    1144,  1145,  1146,  1147,  1148,  1168,  1179,  1180,  1187,  1188,
    1195,  1204,  1208,  1212,  1236,  1237,  1241,  1254,  1254,  1270,
    1271,  1274,  1275,  1279,  1282,  1291,  1304,  1305,  1306,  1307,
    1311,  1312,  1313,  1314,  1315,  1319,  1322,  1323,  1324,  1328,
    1331,  1340,  1341,  1342,  1346,  1347,  1348,  1352,  1353,  1357,
    1358,  1362,  1369,  1370,  1371,  1372,  1379,  1380,  1383,  1384,
    1391,  1403,  1404,  1408,  1409,  1413,  1414,  1415,  1416,  1417,
    1421,  1422,  1423,  1427,  1441,  1442,  1443,  1444,  1445,  1446,
    1447,  1448,  1449,  1450,  1451,  1461,  1465,  1488,  1522,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1534,  1535,  1536,  1537,
    1538,  1539,  1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,
    1548,  1549,  1551,  1552,  1553,  1555,  1557,  1559,  1561,  1562,
    1563,  1564,  1565,  1575,  1576,  1577,  1578,  1579,  1580,  1585,
    1608,  1609,  1610,  1611,  1612,  1613,  1617,  1620,  1621,  1622,
    1631,  1638,  1647,  1648,  1652,  1658,  1668,  1677,  1683,  1688,
    1695,  1700,  1706,  1707,  1708,  1716,  1718,  1719,  1720,  1721,
    1727,  1749,  1750,  1751,  1752,  1753,  1759,  1760,  1761,  1762,
    1763,  1764,  1765,  1766,  1767,  1768,  1769,  1770,  1771,  1772,
    1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,  1781,  1782,
    1783,  1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,
    1793,  1803,  1814,  1815,  1819,  1820,  1821,  1822,  1823,  1824,
    1827,  1838,  1841,  1843,  1847,  1848,  1849,  1850,  1851,  1852,
    1856,  1857,  1861,  1869,  1871,  1875,  1876,  1877,  1881,  1882,
    1885,  1887,  1890,  1891,  1892,  1893,  1894,  1895,  1896,  1897,
    1898,  1899,  1900,  1903,  1925,  1926,  1927,  1928,  1929,  1930,
    1931,  1932,  1933,  1934,  1935,  1936,  1937,  1938,  1939,  1940,
    1941,  1942,  1943,  1944,  1945,  1946,  1947,  1948,  1949,  1950,
    1951,  1952,  1953,  1956,  1958,  1962,  1963,  1965,  1974,  1984,
    1986,  1990,  1991,  1993,  2002,  2013,  2033,  2035,  2038,  2040,
    2044,  2048,  2049,  2053,  2056,  2059,  2064,  2067,  2072,  2075,
    2078,  2084,  2092,  2093,  2094,  2099,  2100,  2106,  2113,  2114,
    2115,  2116,  2117,  2118,  2119,  2120,  2121,  2122,  2123,  2124,
    2125,  2129,  2130,  2131,  2136,  2137,  2138,  2139,  2140,  2143,
    2144,  2145,  2148,  2150,  2153,  2155,  2159,  2168,  2175,  2182,
    2183,  2184,  2185,  2188,  2197,  2204,  2211,  2212,  2216,  2220,
    2226,  2229,  2230,  2231,  2234,  2241,  2241,  2241,  2241,  2241,
    2241,  2241,  2241,  2241,  2241,  2241,  2241,  2241,  2242,  2242,
    2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,
    2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,
    2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,  2242,
    2242,  2242,  2242,  2242,  2242,  2243,  2243,  2243,  2243,  2243,
    2243,  2243,  2243,  2244,  2244,  2244,  2244,  2245,  2245,  2245,
    2246,  2246,  2246,  2247,  2247,  2248,  2249,  2250,  2251,  2252,
    2253,  2254,  2255,  2256,  2257,  2257,  2257,  2257,  2257,  2257
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
  "ROUTER", "ID", "PROTOCOL", "PREFERENCE", "DISABLED", "DIRECT",
  "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE", "STATES", "FILTERS",
  "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS", "PROTOCOLS",
  "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR", "COMMANDS",
  "PREEXPORT", "GENERATE", "LISTEN", "BGP", "V6ONLY", "DUAL", "ADDRESS",
  "PORT", "PASSWORDS", "DESCRIPTION", "RELOAD", "IN", "OUT", "MRTDUMP",
  "MESSAGES", "RESTRICT", "MEMORY", "IGP_METRIC", "SHOW", "STATUS",
  "SUMMARY", "ROUTE", "SYMBOLS", "DUMP", "RESOURCES", "SOCKETS",
  "NEIGHBORS", "ATTRIBUTES", "ECHO", "DISABLE", "ENABLE", "RESTART",
  "FUNCTION", "PRINT", "PRINTN", "UNSET", "RETURN", "ACCEPT", "REJECT",
  "QUITBIRD", "INT", "BOOL", "IP", "PREFIX", "PAIR", "QUAD", "SET",
  "STRING", "BGPMASK", "BGPPATH", "CLIST", "IF", "THEN", "ELSE", "CASE",
  "TRUE", "FALSE", "GW", "NET", "MASK", "PROTO", "SOURCE", "SCOPE", "CAST",
  "DEST", "LEN", "DEFINED", "ADD", "DELETE", "CONTAINS", "RESET",
  "PREPEND", "FIRST", "LAST", "MATCH", "EMPTY", "WHERE", "EVAL", "LOCAL",
  "NEIGHBOR", "AS", "HOLD", "CONNECT", "RETRY", "KEEPALIVE", "MULTIHOP",
  "STARTUP", "VIA", "NEXT", "HOP", "SELF", "DEFAULT", "PATH", "METRIC",
  "START", "DELAY", "FORGET", "WAIT", "AFTER", "BGP_PATH",
  "BGP_LOCAL_PREF", "BGP_MED", "BGP_ORIGIN", "BGP_NEXT_HOP",
  "BGP_ATOMIC_AGGR", "BGP_AGGREGATOR", "BGP_COMMUNITY", "RR", "RS",
  "CLIENT", "CLUSTER", "AS4", "ADVERTISE", "IPV4", "CAPABILITIES", "LIMIT",
  "PREFER", "OLDER", "MISSING", "LLADDR", "DROP", "IGNORE", "REFRESH",
  "INTERPRET", "COMMUNITIES", "BGP_ORIGINATOR_ID", "BGP_CLUSTER_LIST",
  "IGP", "GATEWAY", "RECURSIVE", "MED", "OSPF", "AREA", "OSPF_METRIC1",
  "OSPF_METRIC2", "OSPF_TAG", "OSPF_ROUTER_ID", "RFC1583COMPAT", "STUB",
  "TICK", "COST", "RETRANSMIT", "HELLO", "TRANSMIT", "PRIORITY", "DEAD",
  "TYPE", "BROADCAST", "BCAST", "NONBROADCAST", "NBMA", "POINTOPOINT",
  "PTP", "POINTOMULTIPOINT", "PTMP", "SIMPLE", "AUTHENTICATION", "STRICT",
  "CRYPTOGRAPHIC", "ELIGIBLE", "POLL", "NETWORKS", "HIDDEN", "VIRTUAL",
  "CHECK", "LINK", "RX", "BUFFER", "LARGE", "NORMAL", "STUBNET", "LSADB",
  "ECMP", "WEIGHT", "TOPOLOGY", "STATE", "PIPE", "PEER", "MODE", "OPAQUE",
  "TRANSPARENT", "RIP", "INFINITY", "PERIOD", "GARBAGE", "TIMEOUT",
  "MULTICAST", "QUIET", "NOLISTEN", "VERSION1", "PLAINTEXT", "MD5",
  "HONOR", "NEVER", "ALWAYS", "RIP_METRIC", "RIP_TAG", "STATIC",
  "PROHIBIT", "MULTIPATH", "'('", "')'", "';'", "':'", "'{'", "'}'", "','",
  "'?'", "'['", "']'", "$accept", "config", "conf_entries", "expr",
  "definition", "bool", "ipa", "prefix", "prefix_or_ipa", "pxlen",
  "datetime", "text_or_none", "log_config", "syslog_name", "log_file",
  "log_mask", "log_mask_list", "log_cat", "mrtdump_base",
  "timeformat_which", "timeformat_spec", "timeformat_base",
  "cmd_CONFIGURE", "cmd_CONFIGURE_SOFT", "cmd_DOWN", "cfg_name",
  "kern_proto_start", "kern_item", "kif_proto_start", "kif_item",
  "nl_item", "rtrid", "idval", "listen", "listen_opts", "listen_opt",
  "newtab", "proto_start", "proto_name", "proto_item", "imexport",
  "rtable", "debug_default", "iface_patt_node_init",
  "iface_patt_node_body", "iface_negate", "iface_patt_node",
  "iface_patt_list", "dev_proto_start", "dev_proto", "dev_iface_init",
  "dev_iface_patt", "debug_mask", "debug_list", "debug_flag",
  "mrtdump_mask", "mrtdump_list", "mrtdump_flag", "password_list",
  "password_items", "password_item", "password_item_begin",
  "password_item_params", "cmd_SHOW_STATUS", "cmd_SHOW_MEMORY",
  "cmd_SHOW_PROTOCOLS", "cmd_SHOW_PROTOCOLS_ALL", "optsym",
  "cmd_SHOW_INTERFACES", "cmd_SHOW_INTERFACES_SUMMARY", "cmd_SHOW_ROUTE",
  "r_args", "export_or_preexport", "cmd_SHOW_SYMBOLS",
  "cmd_DUMP_RESOURCES", "cmd_DUMP_SOCKETS", "cmd_DUMP_INTERFACES",
  "cmd_DUMP_NEIGHBORS", "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES",
  "cmd_DUMP_PROTOCOLS", "cmd_ECHO", "echo_mask", "echo_size",
  "cmd_DISABLE", "cmd_ENABLE", "cmd_RESTART", "cmd_RELOAD",
  "cmd_RELOAD_IN", "cmd_RELOAD_OUT", "cmd_DEBUG", "cmd_MRTDUMP",
  "cmd_RESTRICT", "proto_patt", "proto_patt2", "filter_def", "$@1",
  "filter_eval", "type", "one_decl", "decls", "declsn", "filter_body",
  "filter", "where_filter", "function_params", "function_body",
  "function_def", "$@2", "cmds", "cmds_int", "block", "fipa", "set_atom",
  "switch_atom", "pair_expr", "pair_atom", "pair_item", "set_item",
  "switch_item", "set_items", "switch_items", "fprefix_s", "fprefix",
  "fprefix_set", "switch_body", "bgp_path_expr", "bgp_path",
  "bgp_path_tail1", "bgp_path_tail2", "dpair", "constant", "rtadot",
  "function_call", "symbol", "static_attr", "term", "break_command",
  "print_one", "print_list", "var_listn", "var_list", "cmd",
  "bgp_proto_start", "bgp_proto", "ospf_proto_start", "ospf_proto",
  "ospf_proto_item", "ospf_area_start", "ospf_area", "ospf_area_opts",
  "ospf_area_item", "ospf_stubnet", "ospf_stubnet_start",
  "ospf_stubnet_opts", "ospf_stubnet_item", "ospf_vlink",
  "ospf_vlink_opts", "ospf_vlink_item", "ospf_vlink_start",
  "ospf_iface_item", "pref_list", "pref_item", "pref_el", "pref_hid",
  "ipa_list", "ipa_item", "ipa_el", "ipa_ne", "ospf_iface_start",
  "ospf_iface_opts", "ospf_iface_opt_list", "ospf_iface", "opttext",
  "cmd_SHOW_OSPF", "cmd_SHOW_OSPF_NEIGHBORS", "cmd_SHOW_OSPF_INTERFACE",
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_TOPOLOGY_ALL",
  "cmd_SHOW_OSPF_STATE", "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB",
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
     534,   535,   536,   537,   538,   539,   540,    40,    41,    59,
      58,   123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   307,   308,   308,   309,   309,   310,   310,   310,   311,
     311,   312,   312,   312,   312,   312,   312,   313,   313,   314,
     315,   315,   316,   316,   317,   318,   318,   319,   320,   320,
     321,   321,   321,   322,   322,   323,   323,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   325,   325,   326,   326,
     326,   326,   327,   327,   327,   327,   328,   329,   330,   331,
     332,   332,   333,   334,   334,   334,   334,   335,   336,   336,
     337,   338,   339,   339,   339,   340,   341,   341,   342,   342,
     342,   342,   343,   344,   345,   345,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   347,   347,   347,   347,
     348,   349,   349,   350,   351,   351,   351,   352,   352,   353,
     354,   354,   355,   356,   356,   356,   357,   358,   359,   359,
     359,   360,   360,   361,   361,   361,   361,   361,   361,   362,
     362,   362,   363,   363,   364,   364,   365,   365,   366,   366,
     367,   367,   368,   369,   369,   369,   369,   369,   369,   370,
     371,   372,   373,   374,   374,   375,   376,   377,   378,   378,
     378,   378,   378,   378,   378,   378,   378,   378,   378,   378,
     379,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   389,   389,   390,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   400,   400,   401,   401,
     401,   403,   402,   404,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   406,   407,   407,   408,   408,
     409,   410,   410,   411,   412,   412,   413,   415,   414,   416,
     416,   417,   417,   418,   418,   419,   420,   420,   420,   420,
     421,   421,   421,   421,   421,   422,   423,   423,   423,   424,
     424,   425,   425,   425,   426,   426,   426,   427,   427,   428,
     428,   429,   430,   430,   430,   430,   431,   431,   432,   432,
     432,   433,   433,   434,   434,   435,   435,   435,   435,   435,
     436,   436,   436,   437,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   439,   440,   441,   442,   442,
     442,   442,   442,   442,   442,   442,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     444,   444,   444,   444,   444,   444,   445,   446,   446,   446,
     447,   447,   448,   448,   449,   449,   449,   449,   449,   449,
     449,   449,   449,   449,   449,   449,   449,   449,   449,   449,
     450,   451,   451,   451,   451,   451,   451,   451,   451,   451,
     451,   451,   451,   451,   451,   451,   451,   451,   451,   451,
     451,   451,   451,   451,   451,   451,   451,   451,   451,   451,
     451,   451,   451,   451,   451,   451,   451,   451,   451,   451,
     451,   452,   453,   453,   454,   454,   454,   454,   454,   454,
     455,   456,   457,   457,   458,   458,   458,   458,   458,   458,
     459,   459,   460,   461,   461,   462,   462,   462,   463,   463,
     464,   464,   465,   465,   465,   465,   465,   465,   465,   465,
     465,   465,   465,   466,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   468,   468,   469,   469,   470,   471,   472,
     472,   473,   473,   474,   475,   476,   477,   477,   478,   478,
     479,   480,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   490,   490,   490,   490,   491,   492,   492,
     492,   492,   492,   492,   492,   492,   492,   492,   492,   492,
     492,   493,   493,   493,   494,   494,   494,   494,   494,   495,
     495,   495,   496,   496,   497,   497,   498,   499,   500,   501,
     501,   501,   501,   502,   503,   503,   504,   504,   505,   505,
     505,   505,   505,   505,   506,   507,   507,   507,   507,   507,
     507,   507,   507,   507,   507,   507,   507,   507,   508,   508,
     508,   508,   508,   508,   508,   508,   508,   508,   508,   508,
     508,   508,   508,   508,   508,   508,   508,   508,   508,   508,
     508,   508,   508,   508,   508,   508,   508,   508,   508,   508,
     508,   508,   508,   508,   508,   509,   509,   509,   509,   509,
     509,   509,   509,   510,   510,   510,   510,   511,   511,   511,
     512,   512,   512,   512,   512,   512,   512,   512,   512,   512,
     512,   512,   512,   512,   512,   512,   512,   512,   512,   512
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     3,     1,     5,
       5,     1,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     2,     2,     1,     1,     0,     4,     2,     0,
       1,     2,     1,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     3,     1,     1,
       1,     1,     2,     4,     3,     3,     3,     3,     4,     2,
       0,     1,     2,     2,     3,     2,     3,     2,     3,     3,
       3,     4,     1,     1,     1,     4,     0,     2,     2,     2,
       1,     1,     2,     1,     0,     1,     0,     2,     2,     2,
       2,     2,     2,     2,     3,     2,     2,     1,     1,     1,
       1,     3,     3,     0,     1,     1,     2,     0,     1,     3,
       1,     3,     2,     3,     3,     3,     0,     3,     1,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     4,     1,     0,     3,
       4,     1,     2,     0,     5,     5,     5,     5,     4,     3,
       3,     4,     5,     1,     0,     3,     4,     4,     0,     2,
       3,     3,     3,     2,     2,     2,     3,     3,     2,     2,
       1,     1,     4,     3,     3,     3,     3,     3,     3,     3,
       4,     1,     1,     1,     0,     1,     3,     3,     3,     3,
       4,     4,     4,     4,     2,     1,     1,     1,     1,     0,
       1,     0,     4,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     0,     3,     1,     3,
       1,     1,     1,     2,     3,     2,     4,     0,     5,     0,
       1,     1,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     3,     1,     5,
      11,     1,     1,     3,     1,     1,     3,     1,     3,     1,
       3,     3,     1,     2,     2,     6,     1,     3,     0,     4,
       3,     1,     3,     3,     3,     2,     2,     2,     2,     0,
       2,     2,     0,     5,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     0,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     4,     1,     1,     1,     1,     2,     2,     3,     3,
       6,     3,     3,     3,     3,     6,     6,     6,     6,     4,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     3,
       1,     3,     0,     1,     4,     6,     4,     3,     5,     5,
       4,     6,     3,     2,     5,     5,     8,     8,     8,     8,
       2,     3,     3,     5,     6,     6,     6,     4,     4,     5,
       6,     6,     5,     3,     4,     5,     5,     5,     5,     4,
       4,     5,     5,     5,     5,     5,     5,     5,     6,     6,
       8,     6,     6,     5,     4,     5,     4,     5,     4,     5,
       5,     2,     3,     3,     1,     2,     2,     4,     2,     1,
       2,     4,     0,     3,     3,     2,     4,     2,     2,     1,
       4,     1,     1,     0,     3,     2,     2,     2,     4,     1,
       0,     3,     0,     2,     2,     3,     2,     2,     3,     2,
       2,     2,     1,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       2,     3,     2,     3,     3,     4,     2,     2,     2,     3,
       3,     3,     1,     0,     2,     1,     1,     2,     3,     0,
       2,     1,     1,     2,     3,     0,     0,     3,     0,     3,
       3,     1,     0,     4,     6,     6,     6,     7,     6,     7,
       6,     2,     3,     3,     5,     4,     4,     2,     3,     3,
       4,     4,     4,     5,     5,     4,     3,     4,     4,     4,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       2,     2,     0,     3,     0,     3,     0,     3,     2,     3,
       3,     5,     3,     2,     2,     3,     1,     2,     3,     3,
       3,     2,     2,     2,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   578,   579,   580,   581,
     582,   583,   584,   585,   586,   587,   588,   589,   590,   591,
     592,   593,   594,   595,   596,   597,   598,   599,   600,   601,
     602,   603,   604,   605,   606,   607,   608,   609,   610,   611,
     612,   613,   614,     3,     1,     2,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   295,   565,   566,
     567,   568,   569,    84,    84,   570,   571,   572,     0,   574,
      84,    86,   575,   576,   577,    84,    86,    84,    86,    84,
      86,    84,    86,    84,    86,     5,   573,    86,    86,   195,
     197,   196,     0,    61,    60,     0,    59,     0,     0,     0,
       0,   194,   199,     0,     0,     0,   158,   154,   154,   154,
       0,     0,     0,     0,     0,     0,     0,   183,   182,   181,
     184,     0,     0,     0,     0,    30,    29,    32,     0,     0,
       0,    51,    50,    49,    48,     0,     0,    82,     0,   201,
      76,     0,     0,   227,   279,   284,   293,   290,   235,   297,
     287,     0,     0,   282,   295,   325,     0,   285,   286,     0,
       0,     0,     0,   295,     0,   288,   289,   294,   324,   323,
       0,   322,   203,    85,     0,     0,    62,    67,   112,   370,
     411,   511,   517,   548,     0,     0,     0,     0,     0,    16,
     116,     0,     0,     0,     0,   617,     0,     0,     0,     0,
       0,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,   618,     0,     0,
       0,    16,     0,    16,   619,   414,     0,     0,   419,     0,
       0,     0,   620,     0,     0,   546,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   621,     0,     0,   137,   141,
       0,     0,     0,   622,     0,     0,     0,     0,    16,     0,
      16,     0,   615,     0,     0,     0,     0,    26,   616,     0,
       0,   119,   118,     0,     0,     0,    57,     0,     0,   189,
     130,   129,     0,     0,   198,   200,   199,     0,   155,     0,
     150,   149,     0,   153,     0,   154,   154,   154,   154,   154,
       0,     0,   178,   179,   175,   173,   174,   176,   177,   185,
       0,   186,   187,   188,     0,     0,    31,    33,     0,     0,
     101,     6,     8,   295,   102,    52,     0,    56,    72,    73,
      74,     0,   216,     0,    47,     0,     0,   279,   297,   279,
     295,   279,   279,     0,   271,     0,   295,     0,     0,   282,
     282,     0,   320,   295,   295,   295,   295,   295,     0,   239,
     237,   295,   236,   238,   252,   251,   257,     0,   262,   266,
       0,   633,   630,   631,   298,   632,   299,   300,   301,   302,
     303,   304,   305,   635,   638,   637,   634,   636,   639,   640,
     641,   642,   643,   644,   645,   646,   647,   648,   649,   326,
     327,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,     0,   623,   627,   113,    89,   100,
      93,     0,    87,    12,    14,    13,    15,    11,    88,   103,
      98,   216,    99,   295,    91,    97,    92,    95,    90,   114,
     115,   371,     0,     0,     0,     0,     0,     0,     0,    16,
       0,    17,    18,     0,     0,     0,     0,     0,     0,   383,
       0,     0,     0,     0,     0,    16,     0,     0,     0,     0,
      16,     0,    16,     0,    16,     0,    16,     0,     0,    16,
     372,   412,   420,   415,   418,   416,   413,   422,   512,     0,
       0,     0,   513,   518,   103,     0,   142,     0,   138,   533,
     531,   532,     0,     0,     0,     0,     0,     0,     0,     0,
     519,   526,   143,   549,     0,   553,    16,   550,   562,     0,
     561,   563,     0,   552,     0,    63,     0,    65,    16,   625,
     626,   624,     0,    25,     0,   629,   628,   124,   123,   125,
     127,   128,   126,     0,   121,   192,    58,   190,   191,   134,
     135,     0,   132,   193,     0,   151,   156,   157,   164,     0,
       0,   171,   216,   165,   168,   169,     0,   170,   159,     0,
     163,   172,   502,   502,   502,   154,   502,   154,   502,   503,
     564,   180,     0,     0,    28,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,    35,    27,     0,     0,    54,
      55,    71,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,     0,     0,     0,   202,   220,    80,    81,     0,
       0,    75,    77,    46,     0,   216,   275,   276,     0,   277,
     278,   273,   261,   350,   353,     0,   333,   334,   280,   281,
     274,     0,     0,     0,     0,     0,   306,   295,   248,   246,
       0,   245,     0,     0,   291,   263,   264,     0,     0,   292,
     318,   316,   314,   311,   312,   313,   315,   317,   319,   307,
     308,   309,   310,   328,     0,   329,   331,   332,    94,   107,
     110,   117,   221,   222,    96,   223,     0,     0,   406,   408,
       0,    16,    16,     0,     0,     0,     0,     0,     0,     0,
       0,   384,     0,     0,     0,     0,     0,     0,   377,     0,
     378,     0,   404,     0,     0,     0,     0,     0,     0,     0,
     389,   390,     0,     0,     0,     0,   515,   516,   544,   530,
     521,     0,     0,   525,   520,   522,     0,     0,   528,   529,
     527,     0,     0,     0,     0,     0,     0,    19,     0,   559,
     558,     0,   556,   560,    70,    64,    66,    68,    21,    20,
      69,   120,     0,   131,     0,   152,   161,   167,   162,   160,
     166,   501,     0,     0,     0,   502,     0,   502,     0,    10,
       9,    34,     0,     7,    53,   215,   214,   216,   295,    78,
      79,   225,   218,     0,   228,   272,   295,   339,   295,   321,
     295,   295,   295,     0,   295,   295,   235,   253,   258,     0,
       0,   267,   295,   108,     0,   103,     0,     0,   407,     0,
       0,   403,   397,   373,     0,     0,   379,     0,   382,     0,
     385,   396,   395,   391,     0,     0,   405,   394,   386,   387,
     388,   409,   410,   393,   392,   417,   495,    16,     0,     0,
       0,   421,     0,   429,   439,   514,   542,   547,   136,   138,
     523,   524,     0,     0,     0,     0,     0,   140,    22,    23,
     551,   554,     0,   557,   122,   133,   505,   504,   510,     0,
     506,     0,   508,    36,   217,     0,   343,     0,   344,   345,
       0,   295,   341,   342,   340,   295,   295,     0,   295,     0,
       0,   295,   231,   224,     0,   351,     0,     0,     0,     0,
     283,   247,   245,     0,     0,     0,   104,   105,   109,   111,
     399,     0,   401,   402,   374,   375,   381,   380,   398,   376,
     103,   428,     0,   425,   483,     0,   432,   427,   431,   423,
     440,   539,   139,   143,    24,     0,     0,     0,     0,   555,
     507,   509,   295,   295,   295,   295,     0,     0,     0,   226,
     232,     0,     0,   363,   346,   348,     0,   219,   338,   336,
     337,   335,   249,     0,   330,   106,     0,   498,   424,     0,
     453,   433,   442,     0,     0,   545,     0,   148,   143,   143,
     143,   143,     0,     0,     0,     0,   357,   295,   268,   295,
     295,     0,   295,   362,     0,   265,   400,   496,   500,   426,
       0,   484,   485,   486,     0,     0,     0,     0,     0,     0,
       0,   438,   452,     0,   540,   534,   535,   536,   537,   538,
     541,   543,   144,   145,   146,   147,   356,   296,   360,     0,
     295,   354,   233,     0,     0,     0,     0,     0,     0,     0,
       0,   349,   295,     0,     0,   487,    16,     0,    16,   430,
       0,   446,   444,   443,     0,     0,   447,   449,   450,   451,
     441,     0,     0,   295,   295,   240,   244,   242,   295,   364,
     243,   255,   254,   259,     0,   359,   358,   295,   295,   295,
     295,   365,     0,     0,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
     482,     0,   488,   436,   437,   435,   434,   445,   448,   361,
     234,   355,   270,   245,     0,   295,     0,     0,     0,     0,
       0,   295,   489,   458,   472,   454,   457,   455,     0,   470,
       0,   459,   461,   462,   463,   464,   465,   466,   467,   468,
     476,   477,   478,    16,   456,    16,     0,     0,   497,   241,
     295,   256,   269,   260,     0,     0,     0,     0,     0,     0,
     469,   460,   471,   473,   479,   480,   481,   474,     0,   369,
     367,   368,   366,   250,     0,   475,   490,   491,   492,     0,
     493,   494
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   447,    69,   448,   534,   779,   780,   767,
     965,   554,    70,   336,   138,   339,   614,   615,    71,   145,
     146,    72,    16,    17,    18,   105,    73,   283,    74,   289,
     284,    75,   351,    76,   353,   642,    77,    78,   184,   206,
     454,   440,    79,   699,   938,   834,   700,   701,    80,    81,
     449,   207,   294,   563,   564,   303,   571,   572,   267,   751,
     268,   269,   764,    19,    20,    21,    22,   314,    23,    24,
      25,   312,   589,    26,    27,    28,    29,    30,    31,    32,
      33,    34,   130,   330,    35,    36,    37,    38,    39,    40,
      41,    42,    43,   102,   307,    82,   352,    83,   632,   633,
     634,   813,   703,   704,   455,   645,   636,    84,   356,   917,
     918,  1061,   175,   384,  1101,   669,   670,   385,   386,  1103,
     387,  1104,   176,   389,   390,  1063,   362,   177,   363,   371,
     178,   179,   180,   920,   181,   419,   932,   921,   985,   986,
     654,   655,   922,    85,    86,    87,    88,   246,   247,   248,
     744,   872,   957,   958,  1034,  1080,   873,  1002,  1043,   874,
    1131,   999,  1031,  1032,  1033,  1189,  1206,  1207,  1208,   950,
    1073,  1028,   951,   792,    44,    45,    46,    47,    48,    49,
      50,    51,    89,    90,    91,    92,   522,  1050,  1006,   961,
     877,   514,   515,    93,    94,   275,   772,   773,   276,    52,
      95,    53,    96,    97,    98,   420
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -950
static const yytype_int16 yypact[] =
{
      40,  1824,    90,   223,   584,   239,    91,   385,   584,   141,
     830,   710,   467,   584,   584,   584,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,   236,   420,   -20,   400,
     255,   208,  -950,   321,   160,   297,   329,   848,  -950,  -950,
    -950,  -950,  -950,   361,   361,  -950,  -950,  -950,   704,  -950,
     361,  1541,  -950,  -950,  -950,   361,  1653,   361,  1524,   361,
     688,   361,  1431,   361,  1448,  -950,  -950,   627,  1450,  -950,
    -950,  -950,    45,  -950,   303,   388,  -950,   584,   584,   393,
      88,  -950,   612,   102,   407,   426,  -950,   384,   209,   384,
     430,   468,   475,   498,   530,   537,   545,  -950,  -950,  -950,
     408,   555,   557,   569,   556,  -950,   548,  -950,   -22,    45,
      50,  -950,  -950,  -950,  -950,   224,   331,  -950,   443,  -950,
    -950,   335,    88,  -950,    27,  -950,  -950,  -950,   573,   344,
    -950,   471,   473,     0,   848,  -950,   353,  -950,  -950,   364,
     382,   387,   389,   848,    85,  -950,  -950,  -950,  -950,  -950,
    1426,  -950,  1948,  -950,   395,   416,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,   431,    45,   671,   636,    50,    74,
    -950,   403,   403,   699,    88,  -950,   423,   445,   441,   -55,
     725,    74,   534,   552,   -57,   645,    95,   418,   687,   577,
     695,    -2,   582,   575,   424,   580,   579,   314,   571,   565,
      74,   564,   563,   560,    28,   -48,   592,  -950,   487,   488,
     443,    74,    50,    74,  -950,  -950,   491,   490,  -950,   494,
     719,   350,  -950,   493,   495,  -950,   777,    50,   501,   -54,
      50,    50,   736,   740,  -131,  -950,   516,   517,  -950,   519,
     520,   418,   561,  -950,   525,   -14,   526,   759,    74,   765,
      74,   769,  -950,   538,   539,   540,   773,   820,  -950,   542,
     544,  -950,  -950,   892,   842,   847,  -950,   849,   852,  -950,
    -950,  -950,   -47,   853,  -950,  -950,   634,   869,  -950,   870,
    -950,  -950,  1726,  -950,   875,   384,   384,   384,   302,   401,
     877,   879,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
     881,  -950,  -950,  -950,    73,   831,  -950,  -950,   961,   589,
    -950,  -950,  -950,   848,  -950,    50,   621,  -950,  -950,  -950,
    -950,   593,  1858,    84,  -950,   594,   597,    27,  -950,    27,
     848,    27,    27,   886,  -950,   876,   620,   897,   874,     0,
       0,   873,   871,   848,   848,   848,   848,   848,   108,  -950,
    -950,   654,  -950,  -950,   917,  -950,  -950,    70,    -1,  -950,
      87,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,   848,   848,   848,   848,   848,   848,   848,   848,   848,
     848,   848,   848,   848,   486,  -950,  -950,  -950,  -950,  -950,
    -950,   443,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  1776,  -950,   848,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,   863,   864,   631,   632,    50,   888,   714,    74,
     418,  -950,  -950,    50,   758,   762,    50,   878,    50,  -950,
     643,   882,   757,    50,    50,    74,   884,   651,   885,   655,
      74,   657,    74,   237,    74,   671,    74,   661,   662,    74,
    -950,  -950,  -950,  -950,  -950,   752,  -950,  -950,  -950,   946,
     668,   670,  -950,  -950,  -950,   673,  -950,   677,   900,  -950,
    -950,  -950,   689,   690,   693,    50,    50,   696,   697,   698,
    -950,  -950,   323,  -950,    25,  -950,    74,  -950,  -950,   574,
    -950,  -950,   786,  -950,    50,  -950,    50,  -950,    74,  -950,
    -950,  -950,    50,  -950,   418,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,   375,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,   413,  -950,  -950,   984,  -950,  -950,  -950,  -950,   975,
     995,  -950,  1776,  -950,  -950,  -950,   418,  -950,  -950,  1001,
    -950,  -950,  1003,  1003,  1003,   384,  1003,   384,  1003,  -950,
    -950,  -950,   726,   728,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,   433,  -950,  -950,  1107,  1004,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,    59,   729,   737,  -950,  -950,  -950,  -950,   418,
      50,  -950,  -950,  -950,  1432,  1858,  -950,  -950,  1132,  -950,
    -950,  -950,  -950,   188,  -950,   731,  -950,  -950,  -950,  -950,
    -950,   390,  1165,   440,   502,   527,  -950,   848,  -950,  1023,
     734,  1107,   131,   138,  -950,  -950,  -950,  1029,  1021,  -950,
    1959,  1959,  1959,  1984,  1984,  1959,  1959,  1959,  1959,   447,
     447,   871,   871,  -950,   748,  -950,  -950,  -950,  -950,  1027,
    -950,   754,  -950,  -950,  -950,  1948,    50,    50,  -950,  -950,
     756,    74,    74,   761,   763,   764,    50,    50,   771,    50,
     772,  -950,    50,   774,   775,   776,   788,    50,  -950,   443,
    -950,   789,  -950,   804,   813,   821,   822,   823,   824,   826,
    -950,  -950,   827,    50,   172,   828,  -950,  -950,   143,  -950,
    -950,   770,   829,  -950,  -950,  -950,   838,   839,  -950,  -950,
    -950,    50,   421,   460,   844,    50,   418,  -950,   850,  -950,
    -950,   418,   794,   786,  -950,  -950,  -950,  -950,    25,  -950,
    -950,  -950,   892,  -950,   -47,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  1073,  1144,  1145,  1003,  1147,  1003,  1148,  -950,
    -950,  -950,   961,  -950,  -950,  -950,  -950,  1858,   106,  -950,
    -950,  -950,  -950,   405,  -950,  -950,   848,  -950,   848,  -950,
     848,   848,   848,  1190,   848,   654,  -950,  -950,  -950,   859,
     573,  -950,   848,  -950,   639,  -950,   866,   860,  -950,   867,
     868,  -950,  -950,  -950,   872,   880,  -950,   883,  -950,   896,
    -950,  -950,  -950,  -950,   904,   905,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,    47,   895,   890,
     418,  -950,   906,  -950,   907,  -950,  -950,  -950,  -950,   900,
    -950,  -950,   908,  1149,  1149,  1149,  1149,  -950,  -950,  -950,
    -950,  -950,    50,   794,  -950,  -950,  -950,  -950,  -950,  1167,
    -950,  1169,  -950,  -950,  -950,    16,  -950,  1146,  -950,  -950,
     909,   848,  -950,  -950,  -950,   848,   848,   918,    33,  1426,
     910,   694,  -950,  -950,  1858,  -950,  1215,  1240,  1265,  1290,
    -950,  -950,  1948,   912,  1163,  1316,   418,  -950,  -950,  -950,
    -950,    50,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,    50,  -950,  -950,   443,  -950,  -950,   920,  -950,
    -950,    76,  -950,   323,  -950,   929,   930,   931,   932,  -950,
    -950,  -950,   848,   620,   848,  -950,   587,  1878,   559,  -950,
    -950,  1158,   241,  -950,  1948,   933,   934,  -950,  -950,  -950,
    -950,  -950,  1174,   943,  -950,  -950,   935,   185,  -950,     3,
    -950,  -950,   304,    50,   455,  -950,   936,  -950,   323,   323,
     323,   323,   887,   913,  1024,  1475,  -950,   678,  -950,   848,
     848,   351,   694,  -950,   949,  -950,  -950,  -950,  -950,  -950,
      29,  -950,  -950,  -950,    10,    50,    50,    50,   991,    18,
      -7,  -950,  -950,   954,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,   956,
     106,  1034,  -950,    14,  1057,  1082,   958,   959,   960,   962,
     971,  -950,   848,  1547,   972,  -950,    74,    50,    74,  -950,
     979,  -950,  -950,  -950,    50,    50,  -950,  -950,  -950,  -950,
    -950,   980,   978,   678,    41,  -950,  -950,  -950,   654,  -950,
    -950,  1178,  -950,  -950,    36,  -950,  -950,   848,   848,   848,
     848,  -950,   955,   981,    50,    74,    50,    50,    50,  1040,
      50,    48,   851,   120,  1014,    50,   999,  1017,  1013,  -950,
    -950,   985,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  1341,   206,    41,   235,  1366,  1391,  1416,
    1442,   848,  -950,  -950,  -950,  -950,  -950,  -950,    50,  -950,
      50,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,    74,  -950,    74,    53,    50,  -950,  -950,
     848,  -950,  -950,  -950,   986,   987,   996,   997,  1005,    -4,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  1341,  -950,
    -950,  -950,  -950,  -950,  -158,  -950,  -950,  -950,  -950,  1006,
    -950,  -950
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -950,  -950,  -950,   -99,  -950,  -207,  -205,  -269,  -567,  -950,
    -221,  -950,  -950,  -950,  -950,  -950,  -950,   504,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  1200,  -950,  -950,  -950,  -950,
    -950,  -950,  -237,  -950,  -950,  -950,  -950,  -950,   796,  1387,
    1105,   814,  -950,  -950,  -950,  -950,   476,  -508,  -950,  -950,
    -950,  -950,   132,  -950,   528,   -75,  -950,   524,  -949,   442,
    -511,  -950,  -446,  -950,  -950,  -950,  -950,  -110,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,   482,  1025,  -950,  -950,  -950,  -950,  -604,
     513,  -950,   977,   750,  1018,  -950,   691,  -950,  -950,  -915,
    -950,   240,  -173,   663,   190,  -814,   512,  -820,   674,   200,
    -950,  -950,  -169,   676,  -950,  -950,  -950,  -950,   218,   357,
    -950,  -950,  -792,  -950,  -130,   436,   -67,  -950,  -950,   336,
     541,   386,  -823,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -403,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,   588,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -833
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -353
static const yytype_int16 yytable[] =
{
     182,   383,   535,   502,   465,   388,   748,   752,   320,   321,
     931,   474,   475,   341,  1204,   369,   919,   342,   337,   789,
    1094,   471,   472,   491,   364,   675,   676,   519,   497,  1095,
    1096,  1097,   826,   341,   503,   569,   505,   342,   972,  -230,
     812,   344,   357,   588,     1,   527,   358,  -229,  -230,  -230,
    -230,  -230,   905,  1042,   765,   359,  -229,  -229,  -229,  -229,
     905,   468,   341,   341,   570,   341,   342,   342,   341,   342,
     139,   545,   342,   547,  1087,   382,   140,   355,   805,   906,
     291,   443,   444,   445,   446,   292,   982,   906,   341,   341,
      54,   602,   342,   342,   106,   980,   495,   372,  1209,   442,
     341,   379,   380,   158,   342,   308,   378,   907,   443,   444,
     445,   446,  1085,   471,   472,   907,   421,   422,   423,   424,
     425,   538,   480,   300,  1130,   905,   919,  1076,   301,   458,
     426,   427,   428,   429,   430,   431,   432,   433,   462,   463,
     434,  1210,  1160,   504,   111,  1092,   341,   379,   380,   826,
     342,   469,   906,   341,   379,   380,   826,   342,   517,   528,
     529,   523,   524,   908,   909,   910,   911,   912,   913,   914,
     539,   908,   909,   910,   911,   912,   913,   914,   498,  1142,
     907,   915,  1059,  1015,   916,   637,   638,   639,   640,   915,
     793,   794,   916,   796,  1062,   798,   421,   422,   423,   424,
     425,  1170,   806,   540,   698,   592,   593,   594,   596,   598,
     426,   427,   428,   429,   430,   431,   432,   433,   496,   309,
     434,  1095,  1096,  1097,   826,   919,    55,   364,   313,   364,
    1182,   364,   364,   520,   521,   603,   908,   909,   910,   911,
     912,   913,   914,  1102,   345,  1088,   618,  1077,  1089,   866,
    1095,  1096,  1097,   826,   915,   134,    56,   916,  1112,   103,
     150,    57,   713,  1020,    58,   714,  1003,   937,   919,  1078,
    1062,   340,   473,  1021,   147,    59,   617,   346,   726,   338,
     148,   541,   542,   731,   952,   733,   315,   737,  1074,   739,
     104,    60,   742,   648,    61,   343,    62,   479,  1205,   653,
     677,   919,   919,    63,   370,  1029,   661,   662,   663,   664,
     665,  1098,  1079,   973,   671,   343,  1099,   151,  1194,  1195,
     987,   313,    64,   103,   360,   766,  1102,   438,  1075,   768,
    -230,   361,   316,    65,   770,  -230,  1145,  1188,  -229,  1146,
     149,   776,   595,  -229,   343,   343,   293,   343,   153,   778,
     343,  1004,    66,   919,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   710,   752,   995,
     343,   343,  1171,   673,   715,  1172,   674,   718,  1005,   720,
     183,   778,   381,   641,   724,   725,   705,   152,   256,   302,
     678,   296,   899,   679,   901,   761,   299,    67,   421,   422,
     423,   424,   425,   313,    99,   100,   666,   867,  -229,   258,
     310,   667,   426,   427,   428,   429,   430,   431,   432,   433,
     313,   762,   434,   329,   734,   101,   756,   757,   343,   311,
     868,  1066,   869,   322,   809,   381,   471,   472,   141,   870,
     135,   597,   997,   450,   876,   774,   835,   775,   421,   422,
     423,   424,   425,   777,   735,   736,   142,   763,   348,   136,
     349,   350,   426,   427,   428,   429,   430,   431,   432,   433,
     137,   323,   434,   143,   871,   432,   433,   317,   324,   434,
     318,   319,   127,   451,   452,   795,  1027,   797,   835,   109,
     110,   816,   855,   107,   108,   131,   132,   133,  1035,   383,
     383,   325,   128,  1180,   839,   840,   883,   129,   884,   388,
     421,   422,   423,   424,   425,  1067,  1068,  1007,   144,  1069,
     487,   488,    68,  1070,   426,   427,   428,   429,   430,   431,
     432,   433,  1098,   326,   434,   421,   422,   423,   424,   425,
     327,   810,  1036,  1037,  1038,   885,  1039,   886,   328,   426,
     427,   428,   429,   430,   431,   432,   433,  1040,   331,   434,
     332,   889,  1052,  1053,  1054,  1055,   891,   421,   422,   423,
     424,   425,   333,   382,   382,   646,   453,   647,   334,   649,
     650,   426,   427,   428,   429,   430,   431,   432,   433,   297,
     298,   434,   471,   472,   769,   421,   422,   423,   424,   425,
     823,   956,   365,    99,   100,   335,  1041,   836,   837,   426,
     427,   428,   429,   430,   431,   432,   433,   844,   845,   434,
     847,   483,   484,   849,   101,   693,   510,   511,   854,   778,
     347,   304,   305,   154,   354,   155,   156,   157,   158,   159,
     160,   366,   694,   367,   865,   368,   161,   162,   695,   163,
     373,   164,   306,   304,   305,   696,   697,   471,   472,   936,
     953,   374,   882,   966,   967,   968,   888,   154,   195,   155,
     156,   157,   158,   159,   160,   619,   620,   781,   782,   375,
     161,   162,   668,   163,   376,   164,   377,   277,   278,   279,
     439,   280,   281,   818,   165,   196,   435,   905,   197,  1045,
     166,   198,   199,   923,   924,   201,   202,   154,   441,   155,
     156,   157,   158,   159,   160,   783,   784,   436,  1000,   457,
     161,   162,   459,   163,   906,   164,   658,   659,   165,   195,
    1030,   778,   437,   203,   166,   801,   802,   204,  1046,  1047,
    1048,  1049,   461,   820,   460,   464,   466,   467,   470,   653,
     476,   926,   907,   927,   928,   929,   196,   477,   478,   197,
     481,   482,   198,   199,   186,   935,   201,   202,   165,   187,
     485,   486,   167,   168,   166,   490,   120,   489,   492,   493,
     188,   494,   499,   169,   170,   171,   500,   509,   172,   501,
     506,   507,   512,   969,   203,   508,   513,   516,   204,   525,
     121,   122,   518,   526,   189,   821,   167,   168,   908,   909,
     910,   911,   912,   913,   914,   530,   531,   169,   170,   171,
     532,   533,   172,   536,   537,   543,   915,   544,   546,   916,
     822,   123,   124,   125,   126,   548,   552,   549,   550,   551,
     553,   555,   996,   556,   976,   565,   167,   168,   977,   978,
     566,   604,   567,   998,   984,   568,   573,   169,   170,   171,
    1018,   154,   172,   155,   156,   157,   158,   159,   160,  1133,
     185,  1135,   575,   576,   161,   162,   194,   163,   591,   164,
     599,   208,   600,   239,   601,   249,  1016,   254,   616,   270,
    1100,   652,   621,   643,   644,   421,   422,   423,   424,   425,
     651,   657,   660,   434,  1044,  1012,   653,  1014,  1154,   426,
     427,   428,   429,   430,   431,   432,   433,   173,  -352,   434,
     112,   113,   165,   656,   672,   174,   706,   707,   166,   282,
     708,   709,   190,   712,   711,   716,  1081,  1082,  1083,   717,
    1086,   719,   721,   114,   723,   722,   115,   727,   116,   117,
     728,   173,  1064,  1065,   730,   984,   732,   729,   557,   174,
     740,   741,   250,   251,   743,   745,  1192,   746,  1193,   747,
     771,  1100,   749,  1100,   558,   559,   750,   191,  1134,  1060,
     560,   561,   192,   562,   256,  1137,  1138,   785,   753,   754,
     252,   173,   755,  -347,   786,   758,   759,   760,   193,   174,
     167,   168,   605,   606,   607,   608,   609,   610,   611,   612,
     613,   169,   170,   171,   787,  1153,   172,  1155,  1156,  1157,
     790,  1159,  1161,   791,   804,   799,  1174,   800,   807,   817,
     824,  1143,   421,   422,   423,   424,   425,   825,   808,   830,
    1147,  1148,  1149,  1150,   829,   832,   426,   427,   428,   429,
     430,   431,   432,   433,   833,   838,   434,   835,   118,  1190,
     841,  1191,   842,   843,   892,   421,   422,   423,   424,   425,
     846,   848,   878,   850,   851,   852,   896,  1196,  1197,   426,
     427,   428,   429,   430,   431,   432,   433,   853,   856,   434,
     421,   422,   423,   424,   425,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,   857,   426,   427,   428,   429,   430,   431,
     432,   433,   858,  1198,   434,   421,   422,   423,   424,   425,
     859,   860,   861,   862,   119,   863,   864,   875,   879,   426,
     427,   428,   429,   430,   431,   432,   433,   880,   881,   434,
     421,   422,   423,   424,   425,   173,   887,   897,   898,   890,
     900,   902,   955,   174,   426,   427,   428,   429,   430,   431,
     432,   433,   934,   941,   434,   940,   942,   943,   974,   964,
     970,   944,   971,   421,   422,   423,   424,   425,   993,   945,
    1019,  1024,   946,  1084,  1093,  1144,  1056,   426,   427,   428,
     429,   430,   431,   432,   433,   947,   954,   434,   421,   422,
     423,   424,   425,   948,   949,   959,   975,   963,   960,   983,
     992,  1057,   426,   427,   428,   429,   430,   431,   432,   433,
     979,  1001,   434,   421,   422,   423,   424,   425,  1008,  1009,
    1010,  1011,  1158,  1023,  1026,  1051,  1022,   426,   427,   428,
     429,   430,   431,   432,   433,  1025,  1072,   434,   421,   422,
     423,   424,   425,  1090,  1091,  1107,  1108,  1109,  1151,  1110,
    1173,  1175,   426,   427,   428,   429,   430,   431,   432,   433,
    1111,  1132,   434,   421,   422,   423,   424,   425,  1136,  1139,
    1140,  1176,  1152,  1177,  1178,  1199,  1200,   426,   427,   428,
     429,   430,   431,   432,   433,  1201,  1202,   434,   421,   422,
     423,   424,   425,  1203,   295,  1211,   903,   456,   895,   738,
     894,   939,   426,   427,   428,   429,   430,   431,   432,   433,
     904,   962,   434,  1058,   421,   422,   423,   424,   425,   635,
     590,   574,   788,  1141,  1181,   827,   814,   933,   426,   427,
     428,   429,   430,   431,   432,   433,  1183,   828,   434,   421,
     422,   423,   424,   425,   831,   981,  1105,   925,  1071,  1013,
       0,   893,     0,   426,   427,   428,   429,   430,   431,   432,
     433,     0,     0,   434,   421,   422,   423,   424,   425,     0,
       0,  1106,     0,     0,     0,     0,     0,     0,   426,   427,
     428,   429,   430,   431,   432,   433,     0,     0,   434,   421,
     422,   423,   424,   425,     0,   803,     0,     0,     0,     0,
       0,     0,     0,   426,   427,   428,   429,   430,   431,   432,
     433,     0,     0,   434,   421,   422,   423,   424,   425,     0,
     815,   391,     0,     0,     0,     0,     0,     0,   426,   427,
     428,   429,   430,   431,   432,   433,     0,     0,   434,     0,
     421,   422,   423,   424,   425,     0,     0,     0,     0,     0,
       0,     0,     0,   819,   426,   427,   428,   429,   430,   431,
     432,   433,   195,   238,   434,   245,     0,   253,     0,   266,
     391,   274,     0,     0,   285,   290,     0,     0,   930,   195,
       0,   195,     0,     0,     0,   392,   393,     0,     0,   196,
       0,     0,   197,     0,     0,   198,   199,     0,   255,   201,
     202,   394,   286,   988,     0,   256,   196,     0,   196,   197,
       0,   197,   198,   199,   198,   199,   201,   202,   201,   202,
       0,     0,     0,     0,     0,   257,   258,   203,   989,     0,
     395,   204,   287,     0,   392,   393,     0,     0,     0,     0,
       0,     0,     0,     0,   203,     0,   203,     0,   204,     0,
     204,     0,     0,   990,     0,   195,   271,     0,     0,   622,
     623,   624,   625,   626,   627,     0,   628,   629,   630,   631,
     396,   397,   195,   398,   399,   400,   401,   402,   991,   395,
       0,     0,   196,     0,     0,   197,     0,     0,   198,   199,
       0,     0,   201,   202,     0,     0,     0,     0,     0,   196,
       0,     0,   197,     0,   994,   198,   199,     0,   200,   201,
     202,     0,   403,   404,   405,   406,   407,   408,   409,   410,
     203,   256,     0,     0,   204,     0,     0,     0,     0,  1179,
       0,     0,     0,     0,     0,     0,     0,   203,   411,   412,
       0,   204,   258,     0,     0,     0,   413,   414,   415,   416,
       0,     0,     0,     0,  1184,     0,     0,     0,     0,     0,
    1113,   403,   404,   405,   406,   407,   408,   409,   410,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,  1185,
       0,     0,     0,     0,   195,     0,     0,   411,   412,   209,
       0,     0,     0,     0,     0,   413,   414,   415,   416,   272,
     260,   261,   262,   263,  1186,     0,     0,     0,   417,   418,
     264,   196,     0,     0,   197,     0,     0,   198,   199,   577,
     811,   201,   202,   265,     0,     0,     0,   210,     0,   211,
    1187,  1114,     0,     0,   471,   472,     0,     0,     0,     0,
     273,     0,   288,   240,     0,     0,     0,     0,   241,   203,
     242,     0,     0,   204,     0,     0,   578,   417,   418,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,   213,
     214,     0,  1115,     0,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,     0,     0,   243,   579,   702,     0,     0,     0,   580,
    1123,  1124,     0,     0,  1125,   581,   582,     0,  1126,     0,
    1127,   215,     0,     0,     0,     0,  1128,     0,   583,   584,
     585,   586,     0,   587,     0,     0,   244,     0,   216,   217,
       0,   218,   219,     0,   220,   221,   222,     0,   223,     0,
       0,   224,   225,   205,   226,     0,     0,     0,     0,  1129,
       0,     0,     0,     0,     0,     0,     0,   227,   228,     0,
       0,     0,   229,     0,   230,     4,   231,     0,   232,     0,
       0,     0,     0,   233,     0,     0,     0,   234,   235,     0,
     236,     0,     5,     6,     0,     0,   421,   422,   423,   424,
     425,     0,     0,     0,     0,     0,     0,     0,     0,   453,
     426,   427,   428,   429,   430,   431,   432,   433,     0,     0,
     434,     0,     0,   622,   623,   624,   625,   626,   627,     0,
     628,   629,   630,   631,     0,     0,     0,     0,     0,     0,
       0,     7,     0,     0,     8,     0,     9,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
      13,    14,    15,     0,     0,   237,   421,   422,   423,   424,
     425,     0,     0,     0,     0,     0,     0,  -353,  -353,  -353,
     426,   427,   428,   429,   430,   431,   432,   433,     0,     0,
     434,  -353,  -353,  -353,  -353,   430,   431,   432,   433,     0,
       0,   434,   421,   422,   423,   622,   623,   624,   625,   626,
     627,     0,   628,   629,   630,   631,   426,   427,   428,   429,
     430,   431,   432,   433,     0,     0,   434,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1017
};

static const yytype_int16 yycheck[] =
{
      67,   174,   271,   240,   211,   174,   514,   518,   118,   119,
     824,   216,   217,    15,    18,    15,   808,    19,    40,   586,
       6,    18,    19,   230,   154,    26,    27,    81,    76,    15,
      16,    17,    18,    15,   241,    82,   243,    19,    22,     6,
     644,   140,    15,   312,     4,   176,    19,     6,    15,    16,
      17,    18,    19,  1002,    29,    28,    15,    16,    17,    18,
      19,   118,    15,    15,   111,    15,    19,    19,    15,    19,
      90,   278,    19,   280,    81,   174,    96,   152,    19,    46,
      35,    34,    35,    36,    37,    40,   919,    46,    15,    15,
       0,    18,    19,    19,     3,   918,    68,   164,   256,   198,
      15,    16,    17,    18,    19,     3,   173,    74,    34,    35,
      36,    37,    94,    18,    19,    74,     8,     9,    10,    11,
      12,   135,   221,    35,  1073,    19,   918,   117,    40,   204,
      22,    23,    24,    25,    26,    27,    28,    29,   193,   194,
      32,   299,    94,   242,     3,  1060,    15,    16,    17,    18,
      19,   208,    46,    15,    16,    17,    18,    19,   257,   290,
     291,   260,   261,   130,   131,   132,   133,   134,   135,   136,
     184,   130,   131,   132,   133,   134,   135,   136,   226,  1094,
      74,   148,  1015,   975,   151,   101,   102,   103,   104,   148,
     593,   594,   151,   596,  1017,   598,     8,     9,    10,    11,
      12,    81,   143,   217,   441,   315,   316,   317,   318,   319,
      22,    23,    24,    25,    26,    27,    28,    29,   190,   117,
      32,    15,    16,    17,    18,  1017,     3,   357,    19,   359,
    1145,   361,   362,   287,   288,   334,   130,   131,   132,   133,
     134,   135,   136,  1063,    20,   252,   345,   237,   255,    77,
      15,    16,    17,    18,   148,    19,    33,   151,  1072,    20,
     100,    38,   469,    22,    41,   470,   190,   834,  1060,   259,
    1093,   139,   177,    32,    19,    52,   343,    53,   485,   301,
      72,   295,   296,   490,   237,   492,    77,   494,   259,   496,
      51,    68,   499,   360,    71,   297,    73,   299,   302,   366,
     301,  1093,  1094,    80,   304,   302,   373,   374,   375,   376,
     377,   297,   302,   297,   381,   297,   302,    20,   265,   266,
     924,    19,    99,    20,   297,   300,  1146,   195,   299,   536,
     297,   304,   123,   110,   539,   302,   300,  1151,   297,   303,
      19,   548,    40,   302,   297,   297,   301,   297,    19,   554,
     297,   275,   129,  1145,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   466,   879,   936,
     297,   297,   252,   303,   473,   255,   306,   476,   302,   478,
      19,   586,   297,   299,   483,   484,   453,    90,    84,   301,
     303,     3,   795,   306,   797,    72,     3,   174,     8,     9,
      10,    11,    12,    19,    19,    20,   298,   235,   302,   105,
       3,   303,    22,    23,    24,    25,    26,    27,    28,    29,
      19,    98,    32,    15,   187,    40,   525,   526,   297,     3,
     258,    80,   260,     3,   639,   297,    18,    19,    38,   267,
      20,    40,   950,    40,   301,   544,   303,   546,     8,     9,
      10,    11,    12,   552,   217,   218,    56,   134,    15,    39,
      17,    18,    22,    23,    24,    25,    26,    27,    28,    29,
      50,     3,    32,    73,   302,    28,    29,   268,     3,    32,
     271,   272,    15,    80,    81,   595,   301,   597,   303,     7,
       8,   303,   729,   108,   109,    13,    14,    15,   194,   672,
     673,     3,    35,   297,   711,   712,    85,    40,    87,   678,
       8,     9,    10,    11,    12,   164,   165,   963,   118,   168,
     206,   207,   299,   172,    22,    23,    24,    25,    26,    27,
      28,    29,   297,     3,    32,     8,     9,    10,    11,    12,
       3,   640,   238,   239,   240,    85,   242,    87,     3,    22,
      23,    24,    25,    26,    27,    28,    29,   253,     3,    32,
       3,   766,  1008,  1009,  1010,  1011,   771,     8,     9,    10,
      11,    12,     3,   672,   673,   357,   173,   359,    22,   361,
     362,    22,    23,    24,    25,    26,    27,    28,    29,   107,
     108,    32,    18,    19,    20,     8,     9,    10,    11,    12,
     667,   870,    29,    19,    20,    57,   302,   706,   707,    22,
      23,    24,    25,    26,    27,    28,    29,   716,   717,    32,
     719,   197,   198,   722,    40,   139,   276,   277,   727,   834,
     299,    19,    20,    13,   299,    15,    16,    17,    18,    19,
      20,   297,   156,   172,   743,   172,    26,    27,   162,    29,
     297,    31,    40,    19,    20,   169,   170,    18,    19,    20,
     867,   297,   761,   884,   885,   886,   765,    13,    41,    15,
      16,    17,    18,    19,    20,    54,    55,   302,   303,   297,
      26,    27,    28,    29,   297,    31,   297,    60,    61,    62,
      19,    64,    65,   303,    74,    68,   301,    19,    71,   244,
      80,    74,    75,   298,   299,    78,    79,    13,    72,    15,
      16,    17,    18,    19,    20,   302,   303,   301,   955,    20,
      26,    27,   299,    29,    46,    31,   369,   370,    74,    41,
     999,   936,   301,   106,    80,   302,   303,   110,   283,   284,
     285,   286,   301,   303,   299,    20,   212,   195,   103,   816,
      63,   818,    74,   820,   821,   822,    68,   180,    63,    71,
     178,   186,    74,    75,    60,   832,    78,    79,    74,    65,
     190,   192,   152,   153,    80,   210,    66,   206,   214,   216,
      76,   221,   190,   163,   164,   165,   299,    68,   168,   301,
     299,   301,   299,   892,   106,   301,   301,    20,   110,    63,
      90,    91,   301,    63,   100,   303,   152,   153,   130,   131,
     132,   133,   134,   135,   136,   299,   299,   163,   164,   165,
     301,   301,   168,   262,   299,   299,   148,    68,    63,   151,
     303,   121,   122,   123,   124,    66,    63,   299,   299,   299,
      20,   299,   941,   299,   911,     3,   152,   153,   915,   916,
       3,    20,     3,   952,   921,     3,     3,   163,   164,   165,
     301,    13,   168,    15,    16,    17,    18,    19,    20,  1076,
      74,  1078,     3,     3,    26,    27,    80,    29,     3,    31,
       3,    85,     3,    87,     3,    89,   299,    91,   299,    93,
    1063,    15,   299,   299,   297,     8,     9,    10,    11,    12,
      14,    27,    29,    32,  1003,   972,   973,   974,  1115,    22,
      23,    24,    25,    26,    27,    28,    29,   297,   298,    32,
      90,    91,    74,    26,     7,   305,    63,    63,    80,   302,
     299,   299,   228,   219,    46,   177,  1035,  1036,  1037,   177,
    1039,    63,   299,   113,   187,    63,   116,    63,   118,   119,
     299,   297,  1019,  1020,   299,  1022,   299,    72,    66,   305,
     299,   299,   274,   275,   212,    19,  1173,   299,  1175,   299,
     184,  1144,   299,  1146,    82,    83,   299,   273,  1077,   301,
      88,    89,   278,    91,    84,  1084,  1085,     3,   299,   299,
     302,   297,   299,   299,    19,   299,   299,   299,   294,   305,
     152,   153,    41,    42,    43,    44,    45,    46,    47,    48,
      49,   163,   164,   165,    19,  1114,   168,  1116,  1117,  1118,
      19,  1120,  1121,    20,    20,   299,  1125,   299,   299,   298,
       7,  1098,     8,     9,    10,    11,    12,   303,   301,    18,
    1107,  1108,  1109,  1110,    15,   297,    22,    23,    24,    25,
      26,    27,    28,    29,    27,   299,    32,   303,   228,  1158,
     299,  1160,   299,   299,   270,     8,     9,    10,    11,    12,
     299,   299,   302,   299,   299,   299,     3,  1176,  1177,    22,
      23,    24,    25,    26,    27,    28,    29,   299,   299,    32,
       8,     9,    10,    11,    12,   244,   245,   246,   247,   248,
     249,   250,   251,   299,    22,    23,    24,    25,    26,    27,
      28,    29,   299,  1180,    32,     8,     9,    10,    11,    12,
     299,   299,   299,   299,   294,   299,   299,   299,   299,    22,
      23,    24,    25,    26,    27,    28,    29,   299,   299,    32,
       8,     9,    10,    11,    12,   297,   302,     3,     3,   299,
       3,     3,   262,   305,    22,    23,    24,    25,    26,    27,
      28,    29,   303,   303,    32,   299,   299,   299,    22,    20,
       3,   299,     3,     8,     9,    10,    11,    12,    15,   299,
      22,     7,   299,   192,   150,     7,   299,    22,    23,    24,
      25,    26,    27,    28,    29,   299,   301,    32,     8,     9,
      10,    11,    12,   299,   299,   299,   297,   299,   301,   299,
     298,   298,    22,    23,    24,    25,    26,    27,    28,    29,
     302,   301,    32,     8,     9,    10,    11,    12,   299,   299,
     299,   299,   192,   299,   299,   299,   303,    22,    23,    24,
      25,    26,    27,    28,    29,   302,   297,    32,     8,     9,
      10,    11,    12,   299,   298,   297,   297,   297,   303,   297,
     246,   262,    22,    23,    24,    25,    26,    27,    28,    29,
     299,   299,    32,     8,     9,    10,    11,    12,   299,   299,
     302,   264,   301,   270,   299,   299,   299,    22,    23,    24,
      25,    26,    27,    28,    29,   299,   299,    32,     8,     9,
      10,    11,    12,   298,   104,   299,   802,   202,   784,   495,
     782,   835,    22,    23,    24,    25,    26,    27,    28,    29,
     807,   879,    32,   299,     8,     9,    10,    11,    12,   352,
     312,   306,   582,  1093,  1144,   672,   645,   825,    22,    23,
      24,    25,    26,    27,    28,    29,  1146,   673,    32,     8,
       9,    10,    11,    12,   678,   919,   299,   816,  1022,   973,
      -1,   773,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    -1,
      -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,     8,
       9,    10,    11,    12,    -1,   298,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,     8,     9,    10,    11,    12,    -1,
     298,     5,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   298,    22,    23,    24,    25,    26,    27,
      28,    29,    41,    86,    32,    88,    -1,    90,    -1,    92,
       5,    94,    -1,    -1,    97,    98,    -1,    -1,   298,    41,
      -1,    41,    -1,    -1,    -1,    69,    70,    -1,    -1,    68,
      -1,    -1,    71,    -1,    -1,    74,    75,    -1,    77,    78,
      79,    85,    62,   298,    -1,    84,    68,    -1,    68,    71,
      -1,    71,    74,    75,    74,    75,    78,    79,    78,    79,
      -1,    -1,    -1,    -1,    -1,   104,   105,   106,   298,    -1,
     114,   110,    92,    -1,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,   106,    -1,   110,    -1,
     110,    -1,    -1,   298,    -1,    41,   118,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,   144,   145,   146,   147,
     154,   155,    41,   157,   158,   159,   160,   161,   298,   114,
      -1,    -1,    68,    -1,    -1,    71,    -1,    -1,    74,    75,
      -1,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    71,    -1,   298,    74,    75,    -1,    77,    78,
      79,    -1,   196,   197,   198,   199,   200,   201,   202,   203,
     106,    84,    -1,    -1,   110,    -1,    -1,    -1,    -1,   298,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   222,   223,
      -1,   110,   105,    -1,    -1,    -1,   230,   231,   232,   233,
      -1,    -1,    -1,    -1,   298,    -1,    -1,    -1,    -1,    -1,
     123,   196,   197,   198,   199,   200,   201,   202,   203,    -1,
      -1,    -1,    -1,    -1,   253,    -1,    -1,    -1,    -1,   298,
      -1,    -1,    -1,    -1,    41,    -1,    -1,   222,   223,    46,
      -1,    -1,    -1,    -1,    -1,   230,   231,   232,   233,   261,
     279,   280,   281,   282,   298,    -1,    -1,    -1,   292,   293,
     289,    68,    -1,    -1,    71,    -1,    -1,    74,    75,     3,
     298,    78,    79,   302,    -1,    -1,    -1,    84,    -1,    86,
     298,   194,    -1,    -1,    18,    19,    -1,    -1,    -1,    -1,
     302,    -1,   302,   229,    -1,    -1,    -1,    -1,   234,   106,
     236,    -1,    -1,   110,    -1,    -1,    40,   292,   293,    -1,
      -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,
     127,    -1,   235,    -1,   237,   238,   239,   240,   241,   242,
     243,    -1,    -1,   269,    68,    19,    -1,    -1,    -1,    73,
     253,   254,    -1,    -1,   257,    79,    80,    -1,   261,    -1,
     263,   158,    -1,    -1,    -1,    -1,   269,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    -1,   302,    -1,   175,   176,
      -1,   178,   179,    -1,   181,   182,   183,    -1,   185,    -1,
      -1,   188,   189,   302,   191,    -1,    -1,    -1,    -1,   302,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,   205,    -1,
      -1,    -1,   209,    -1,   211,    41,   213,    -1,   215,    -1,
      -1,    -1,    -1,   220,    -1,    -1,    -1,   224,   225,    -1,
     227,    -1,    58,    59,    -1,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,    -1,   110,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,   125,
     126,   127,   128,    -1,    -1,   302,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,     8,     9,    10,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,     8,     9,    10,   137,   138,   139,   140,   141,
     142,    -1,   144,   145,   146,   147,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   149
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   308,   309,    41,    58,    59,   107,   110,   112,
     115,   120,   125,   126,   127,   128,   329,   330,   331,   370,
     371,   372,   373,   375,   376,   377,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   481,   482,   483,   484,   485,   486,
     487,   488,   506,   508,     0,     3,    33,    38,    41,    52,
      68,    71,    73,    80,    99,   110,   129,   174,   299,   311,
     319,   325,   328,   333,   335,   338,   340,   343,   344,   349,
     355,   356,   402,   404,   414,   450,   451,   452,   453,   489,
     490,   491,   492,   500,   501,   507,   509,   510,   511,    19,
      20,    40,   400,    20,    51,   332,     3,   108,   109,   400,
     400,     3,    90,    91,   113,   116,   118,   119,   228,   294,
      66,    90,    91,   121,   122,   123,   124,    15,    35,    40,
     389,   400,   400,   400,    19,    20,    39,    50,   321,    90,
      96,    38,    56,    73,   118,   326,   327,    19,    72,    19,
     100,    20,    90,    19,    13,    15,    16,    17,    18,    19,
      20,    26,    27,    29,    31,    74,    80,   152,   153,   163,
     164,   165,   168,   297,   305,   419,   429,   434,   437,   438,
     439,   441,   443,    19,   345,   345,    60,    65,    76,   100,
     228,   273,   278,   294,   345,    41,    68,    71,    74,    75,
      77,    78,    79,   106,   110,   302,   346,   358,   345,    46,
      84,    86,   118,   126,   127,   158,   175,   176,   178,   179,
     181,   182,   183,   185,   188,   189,   191,   204,   205,   209,
     211,   213,   215,   220,   224,   225,   227,   302,   346,   345,
     229,   234,   236,   269,   302,   346,   454,   455,   456,   345,
     274,   275,   302,   346,   345,    77,    84,   104,   105,   253,
     279,   280,   281,   282,   289,   302,   346,   365,   367,   368,
     345,   118,   261,   302,   346,   502,   505,    60,    61,    62,
      64,    65,   302,   334,   337,   346,    62,    92,   302,   336,
     346,    35,    40,   301,   359,   332,     3,   400,   400,     3,
      35,    40,   301,   362,    19,    20,    40,   401,     3,   117,
       3,     3,   378,    19,   374,    77,   123,   268,   271,   272,
     374,   374,     3,     3,     3,     3,     3,     3,     3,    15,
     390,     3,     3,     3,    22,    57,   320,    40,   301,   322,
     359,    15,    19,   297,   310,    20,    53,   299,    15,    17,
      18,   339,   403,   341,   299,   362,   415,    15,    19,    28,
     297,   304,   433,   435,   441,    29,   297,   172,   172,    15,
     304,   436,   443,   297,   297,   297,   297,   297,   443,    16,
      17,   297,   310,   419,   420,   424,   425,   427,   429,   430,
     431,     5,    69,    70,    85,   114,   154,   155,   157,   158,
     159,   160,   161,   196,   197,   198,   199,   200,   201,   202,
     203,   222,   223,   230,   231,   232,   233,   292,   293,   442,
     512,     8,     9,    10,    11,    12,    22,    23,    24,    25,
      26,    27,    28,    29,    32,   301,   301,   301,   359,    19,
     348,    72,   310,    34,    35,    36,    37,   310,   312,   357,
      40,    80,    81,   173,   347,   411,   347,    20,   362,   299,
     299,   301,   193,   194,    20,   312,   212,   195,   118,   208,
     103,    18,    19,   177,   313,   313,    63,   180,    63,   299,
     310,   178,   186,   197,   198,   190,   192,   206,   207,   206,
     210,   312,   214,   216,   221,    68,   190,    76,   226,   190,
     299,   301,   339,   312,   310,   312,   299,   301,   301,    68,
     276,   277,   299,   301,   498,   499,    20,   310,   301,    81,
     287,   288,   493,   310,   310,    63,    63,   176,   290,   291,
     299,   299,   301,   301,   313,   314,   262,   299,   135,   184,
     217,   295,   296,   299,    68,   312,    63,   312,    66,   299,
     299,   299,    63,    20,   318,   299,   299,    66,    82,    83,
      88,    89,    91,   360,   361,     3,     3,     3,     3,    82,
     111,   363,   364,     3,   401,     3,     3,     3,    40,    68,
      73,    79,    80,    92,    93,    94,    95,    97,   314,   379,
     411,     3,   374,   374,   374,    40,   374,    40,   374,     3,
       3,     3,    18,   310,    20,    41,    42,    43,    44,    45,
      46,    47,    48,    49,   323,   324,   299,   443,   310,    54,
      55,   299,   137,   138,   139,   140,   141,   142,   144,   145,
     146,   147,   405,   406,   407,   409,   413,   101,   102,   103,
     104,   299,   342,   299,   297,   412,   435,   435,   443,   435,
     435,    14,    15,   443,   447,   448,    26,    27,   436,   436,
      29,   443,   443,   443,   443,   443,   298,   303,    28,   422,
     423,   443,     7,   303,   306,    26,    27,   301,   303,   306,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   139,   156,   162,   169,   170,   339,   350,
     353,   354,    19,   409,   410,   443,    63,    63,   299,   299,
     310,    46,   219,   312,   313,   310,   177,   177,   310,    63,
     310,   299,    63,   187,   310,   310,   312,    63,   299,    72,
     299,   312,   299,   312,   187,   217,   218,   312,   348,   312,
     299,   299,   312,   212,   457,    19,   299,   299,   354,   299,
     299,   366,   367,   299,   299,   299,   310,   310,   299,   299,
     299,    72,    98,   134,   369,    29,   300,   316,   312,    20,
     313,   184,   503,   504,   310,   310,   312,   310,   313,   314,
     315,   302,   303,   302,   303,     3,    19,    19,   410,   315,
      19,    20,   480,   480,   480,   374,   480,   374,   480,   299,
     299,   302,   303,   298,    20,    19,   143,   299,   301,   313,
     310,   298,   406,   408,   413,   298,   303,   298,   303,   298,
     303,   303,   303,   443,     7,   303,    18,   420,   425,    15,
      18,   430,   297,    27,   352,   303,   310,   310,   299,   312,
     312,   299,   299,   299,   310,   310,   299,   310,   299,   310,
     299,   299,   299,   299,   310,   339,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   310,    77,   235,   258,   260,
     267,   302,   458,   463,   466,   299,   301,   497,   302,   299,
     299,   299,   310,    85,    87,    85,    87,   302,   310,   313,
     299,   313,   270,   503,   361,   364,     3,     3,     3,   480,
       3,   480,     3,   324,   407,    19,    46,    74,   130,   131,
     132,   133,   134,   135,   136,   148,   151,   416,   417,   439,
     440,   444,   449,   298,   299,   447,   443,   443,   443,   443,
     298,   422,   443,   423,   303,   443,    20,   315,   351,   353,
     299,   303,   299,   299,   299,   299,   299,   299,   299,   299,
     476,   479,   237,   312,   301,   262,   314,   459,   460,   299,
     301,   496,   366,   299,    20,   317,   317,   317,   317,   310,
       3,     3,    22,   297,    22,   297,   443,   443,   443,   302,
     449,   442,   512,   299,   443,   445,   446,   406,   298,   298,
     298,   298,   298,    15,   298,   315,   310,   354,   310,   468,
     339,   301,   464,   190,   275,   302,   495,   369,   299,   299,
     299,   299,   443,   448,   443,   439,   299,   149,   301,    22,
      22,    32,   303,   299,     7,   302,   299,   301,   478,   302,
     314,   469,   470,   471,   461,   194,   238,   239,   240,   242,
     253,   302,   365,   465,   310,   244,   283,   284,   285,   286,
     494,   299,   369,   369,   369,   369,   299,   298,   299,   512,
     301,   418,   449,   432,   443,   443,    80,   164,   165,   168,
     172,   446,   297,   477,   259,   299,   117,   237,   259,   302,
     462,   310,   310,   310,   192,    94,   310,    81,   252,   255,
     299,   298,   416,   150,     6,    15,    16,    17,   297,   302,
     419,   421,   424,   426,   428,   299,   299,   297,   297,   297,
     297,   299,   422,   123,   194,   235,   237,   238,   239,   240,
     241,   242,   243,   253,   254,   257,   261,   263,   269,   302,
     365,   467,   299,   312,   310,   312,   299,   310,   310,   299,
     302,   418,   416,   443,     7,   300,   303,   443,   443,   443,
     443,   303,   301,   310,   312,   310,   310,   310,   192,   310,
      94,   310,   244,   245,   246,   247,   248,   249,   250,   251,
      81,   252,   255,   246,   310,   262,   264,   270,   299,   298,
     297,   421,   416,   426,   298,   298,   298,   298,   422,   472,
     310,   310,   312,   312,   265,   266,   310,   310,   443,   299,
     299,   299,   299,   298,    18,   302,   473,   474,   475,   256,
     299,   299
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
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
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



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
  if (yyn == YYPACT_NINF)
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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

/* Line 1455 of yacc.c  */
#line 428 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 429 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 442 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 443 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 449 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 453 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 462 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 463 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 464 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 465 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 466 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 467 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 474 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 481 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 489 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 493 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 497 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 504 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 512 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 513 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 519 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 528 "cf-parse.y"
    { (yyval.t) = (yyvsp[(2) - (2)].t); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 529 "cf-parse.y"
    { (yyval.t) = bird_name; ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 533 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 538 "cf-parse.y"
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[(2) - (2)].t); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 539 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 543 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 544 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 548 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 549 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 553 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 554 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 555 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 556 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 557 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 558 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 559 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 560 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 561 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 567 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 568 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 577 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 578 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 579 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 580 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 583 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 584 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 585 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 586 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 598 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 601 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 604 "cf-parse.y"
    { cmd_shutdown(); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 607 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 616 "cf-parse.y"
    {
#ifndef CONFIG_MULTIPLE_TABLES
     if (cf_krt)
       cf_error("Kernel protocol already defined");
#endif
     cf_krt = this_proto = proto_config_new(&proto_unix_kernel, sizeof(struct krt_config));
     this_proto->preference = DEF_PREF_INHERITED;
     THIS_KRT->scan_time = 60;
     THIS_KRT->learn = THIS_KRT->persist = 0;
     krt_scan_construct(THIS_KRT);
     krt_set_construct(THIS_KRT);
   ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 632 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 633 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 637 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[(2) - (2)].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[(2) - (2)].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 644 "cf-parse.y"
    { THIS_KRT->devroutes = (yyvsp[(3) - (3)].i); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 650 "cf-parse.y"
    {
     if (cf_kif)
       cf_error("Kernel device protocol already defined");
     cf_kif = this_proto = proto_config_new(&proto_unix_iface, sizeof(struct kif_config));
     this_proto->preference = DEF_PREF_DIRECT;
     THIS_KIF->scan_time = 60;
     init_list(&THIS_KIF->primary);
     krt_if_construct(THIS_KIF);
   ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 663 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 667 "cf-parse.y"
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[(2) - (3)].t);
     kpi->prefix = (yyvsp[(3) - (3)].px).addr;
     kpi->pxlen = (yyvsp[(3) - (3)].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 680 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 693 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 699 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 701 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[(1) - (1)].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 720 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 721 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 722 "cf-parse.y"
    { new_config->listen_bgp_flags = 0; ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 723 "cf-parse.y"
    { new_config->listen_bgp_flags = 1; ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 730 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 742 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 748 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 756 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 760 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 761 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 762 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 763 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 764 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 765 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 766 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 767 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 771 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 773 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 774 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 778 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 786 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 787 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 795 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 803 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 804 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 805 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 809 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 810 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 827 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 842 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 857 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 858 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 859 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 864 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 868 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 869 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 870 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 871 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 872 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 873 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 879 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 880 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 881 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 886 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 890 "cf-parse.y"
    { (yyval.i) = MD_STATES; ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 891 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 912 "cf-parse.y"
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
   ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 930 "cf-parse.y"
    { ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 931 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 932 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 933 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 934 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 935 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 944 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 947 "cf-parse.y"
    { cmd_show_memory(); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 950 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 953 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 957 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 961 "cf-parse.y"
    { if_show(); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 964 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 967 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 970 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 976 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 982 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 989 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 994 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 999 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1004 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1008 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1012 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->export_mode) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->export_mode = (yyvsp[(2) - (3)].i);
     (yyval.ra)->primary_only = 1;
     (yyval.ra)->export_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1022 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1030 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1034 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1041 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1042 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1046 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1050 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1052 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1054 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1056 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1058 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1060 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1062 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1064 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1070 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1071 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1076 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1077 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1084 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1086 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1088 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1090 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1092 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1094 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1098 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1102 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1105 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1108 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1109 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1110 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1114 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1115 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1116 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1124 "cf-parse.y"
    { (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1125 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s)->def = (yyvsp[(4) - (4)].f);
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1134 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1138 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1139 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1140 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1141 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1142 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1143 "cf-parse.y"
    { (yyval.i) = T_QUAD; ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1144 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1145 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1146 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1147 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1148 "cf-parse.y"
    { 
	switch ((yyvsp[(1) - (2)].i)) {
	  case T_INT:
	  case T_PAIR:
	  case T_QUAD:
	  case T_IP:
	       (yyval.i) = T_SET;
	       break;

	  case T_PREFIX:
	       (yyval.i) = T_PREFIX_SET;
	    break;

	  default:
		cf_error( "You can't create sets of this type." );
	}
   ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1168 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1179 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1180 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1187 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1188 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1195 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1204 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1212 "cf-parse.y"
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
  ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1236 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1237 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1241 "cf-parse.y"
    {
     if ((yyvsp[(1) - (4)].s)) {
       /* Prepend instruction to clear local variables */
       (yyval.x) = f_new_inst();
       (yyval.x)->code = P('c','v');
       (yyval.x)->a1.p = (yyvsp[(1) - (4)].s);
       (yyval.x)->next = (yyvsp[(3) - (4)].x);
     } else
       (yyval.x) = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1254 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1257 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1270 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1271 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1274 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1275 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1279 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1282 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1291 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1304 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1305 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1306 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1307 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1311 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1312 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1313 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1314 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1315 "cf-parse.y"
    { (yyval.v).type = pair_a((yyvsp[(1) - (1)].i)); (yyval.v).val.i = pair_b((yyvsp[(1) - (1)].i)); ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1319 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(1) - (1)].x)); check_u16((yyval.i)); ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1322 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (1)].i), (yyvsp[(1) - (1)].i)); ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1323 "cf-parse.y"
    { (yyval.i32) = pair((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i)); ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1324 "cf-parse.y"
    { (yyval.i32) = 0xFFFF; ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1328 "cf-parse.y"
    {
     (yyval.e) = f_new_pair_set(pair_a((yyvsp[(2) - (5)].i32)), pair_b((yyvsp[(2) - (5)].i32)), pair_a((yyvsp[(4) - (5)].i32)), pair_b((yyvsp[(4) - (5)].i32)));
   ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1331 "cf-parse.y"
    {
     /* Hack: $2 and $4 should be pair_expr, but that would cause shift/reduce conflict */
     if ((pair_a((yyvsp[(2) - (11)].i32)) != pair_b((yyvsp[(2) - (11)].i32))) || (pair_a((yyvsp[(4) - (11)].i32)) != pair_b((yyvsp[(4) - (11)].i32))))
       cf_error("syntax error");
     (yyval.e) = f_new_pair_item(pair_b((yyvsp[(2) - (11)].i32)), pair_b((yyvsp[(4) - (11)].i32)), (yyvsp[(8) - (11)].i), (yyvsp[(10) - (11)].i)); 
   ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1341 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1342 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1347 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (1)].v), (yyvsp[(1) - (1)].v)); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1348 "cf-parse.y"
    { (yyval.e) = f_new_item((yyvsp[(1) - (3)].v), (yyvsp[(3) - (3)].v)); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1353 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1358 "cf-parse.y"
    { (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), (yyvsp[(3) - (3)].e)); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1362 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1369 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1370 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1371 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1372 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1379 "cf-parse.y"
    { (yyval.trie) = f_new_trie(cfg_mem); trie_add_fprefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1380 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_fprefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1383 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1384 "cf-parse.y"
    {
     /* Fill data fields */
     struct f_tree *t;
     for (t = (yyvsp[(2) - (4)].e); t; t = t->left)
       t->data = (yyvsp[(4) - (4)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (4)].e), (yyvsp[(2) - (4)].e));
   ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1391 "cf-parse.y"
    { 
     struct f_tree *t = f_new_tree();
     t->from.type = t->to.type = T_VOID;
     t->right = t;
     t->data = (yyvsp[(3) - (3)].x);
     (yyval.e) = f_merge_items((yyvsp[(1) - (3)].e), t);
 ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1403 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1404 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1408 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1409 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1413 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1414 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1415 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1416 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1417 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1421 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1422 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1423 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1427 "cf-parse.y"
    {
        if (((yyvsp[(2) - (5)].x)->code == 'c') && ((yyvsp[(4) - (5)].x)->code == 'c'))
          { 
            if (((yyvsp[(2) - (5)].x)->aux != T_INT) || ((yyvsp[(4) - (5)].x)->aux != T_INT))
              cf_error( "Can't operate with value of non-integer type in pair constructor" );
	    check_u16((yyvsp[(2) - (5)].x)->a2.i); check_u16((yyvsp[(4) - (5)].x)->a2.i);
            (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PAIR;  (yyval.x)->a2.i = pair((yyvsp[(2) - (5)].x)->a2.i, (yyvsp[(4) - (5)].x)->a2.i);
          }
	else
	  { (yyval.x) = f_new_inst(); (yyval.x)->code = P('m', 'p'); (yyval.x)->a1.p = (yyvsp[(2) - (5)].x); (yyval.x)->a2.p = (yyvsp[(4) - (5)].x); }
    ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1441 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1442 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1443 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1444 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1445 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1446 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1447 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1448 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1449 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1450 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1451 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1465 "cf-parse.y"
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
   ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1488 "cf-parse.y"
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
       case SYM_VARIABLE | T_STRING:
       case SYM_VARIABLE | T_IP:
       case SYM_VARIABLE | T_PREFIX:
       case SYM_VARIABLE | T_PREFIX_SET:
       case SYM_VARIABLE | T_SET:
       case SYM_VARIABLE | T_PATH:
       case SYM_VARIABLE | T_PATH_MASK:
       case SYM_VARIABLE | T_CLIST:
	 (yyval.x)->code = 'V';
	 (yyval.x)->a1.p = (yyvsp[(1) - (1)].s)->def;
	 (yyval.x)->a2.p = (yyvsp[(1) - (1)].s)->name;
	 break;
       default:
	 cf_error("%s: variable expected.", (yyvsp[(1) - (1)].s)->name );
     }
   ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1522 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1524 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1525 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1526 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1527 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1528 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1529 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1530 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1534 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1535 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1536 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1537 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1538 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1539 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1540 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1541 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1542 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1543 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1544 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1545 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1546 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1547 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1548 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1549 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1551 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1552 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1553 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1555 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1557 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1559 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1561 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1562 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1563 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1564 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1565 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1575 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1576 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1577 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1578 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1579 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1580 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'f'; ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1585 "cf-parse.y"
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
   ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1608 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1609 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1610 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1611 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1612 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1613 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1617 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1620 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1621 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1622 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1631 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1638 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1647 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1648 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1652 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1658 "cf-parse.y"
    {
     struct f_inst *i = f_new_inst();
     i->code = '?';
     i->a1.p = (yyvsp[(2) - (6)].x);
     i->a2.p = (yyvsp[(4) - (6)].x);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = i;
     (yyval.x)->a2.p = (yyvsp[(6) - (6)].x);
   ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1668 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll set value\n" );
     if (((yyvsp[(1) - (4)].s)->class & ~T_MASK) != SYM_VARIABLE)
       cf_error( "You may set only variables." );
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = (yyvsp[(1) - (4)].s);
     (yyval.x)->a2.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1677 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1683 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1688 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1695 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1700 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1706 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1707 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1708 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1717 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1718 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1719 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1720 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1721 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'f', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1727 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_bgp, sizeof(struct bgp_config));
     this_proto->preference = DEF_PREF_BGP;
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
 ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1751 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1752 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(3) - (6)].a); BGP_CFG->local_as = (yyvsp[(5) - (6)].i); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1753 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1759 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1760 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1761 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1762 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1763 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1764 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1765 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1766 "cf-parse.y"
    { BGP_CFG->multihop = 64; ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1767 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (4)].i); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1768 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1769 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1770 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1771 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1772 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_DIRECT; ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1773 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_RECURSIVE; ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1774 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1775 "cf-parse.y"
    { BGP_CFG->med_metric = (yyvsp[(4) - (5)].i); ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1776 "cf-parse.y"
    { BGP_CFG->igp_metric = (yyvsp[(4) - (5)].i); ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1777 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1778 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1779 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1780 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1781 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1782 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1783 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1784 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1785 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1786 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1787 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1788 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1789 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1790 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1791 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1792 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1793 "cf-parse.y"
    { BGP_CFG->igp_table = (yyvsp[(4) - (5)].r); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1803 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1820 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1821 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (2)].i) ? DEFAULT_ECMP_LIMIT : 0; ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1822 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (4)].i) ? (yyvsp[(4) - (4)].i) : 0; if ((yyvsp[(4) - (4)].i) < 0) cf_error("ECMP limit cannot be negative"); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1823 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i); if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1827 "cf-parse.y"
    {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = (yyvsp[(2) - (2)].i32);
  this_area->stub = 0;
  init_list(&this_area->patt_list);
  init_list(&this_area->net_list);
  init_list(&this_area->stubnet_list);
 ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1838 "cf-parse.y"
    { ospf_area_finish(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1847 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1848 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1861 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1875 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1876 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1877 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1881 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1891 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1892 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1893 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1894 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1895 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1896 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1897 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1898 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1899 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1904 "cf-parse.y"
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
 ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1925 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1926 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1927 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1928 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1929 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1930 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1931 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1932 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1933 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1934 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1935 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1936 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1937 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1938 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1939 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1940 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1941 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1942 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1943 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1944 "cf-parse.y"
    { OSPF_PATT->check_link = (yyvsp[(3) - (3)].i); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1945 "cf-parse.y"
    { OSPF_PATT->ecmp_weight = (yyvsp[(3) - (3)].i) - 1; if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("ECMP weight must be in range 1-256"); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1947 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1948 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1949 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1950 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1951 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1952 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) || ((yyvsp[(3) - (3)].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1966 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1975 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1994 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2003 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2013 "cf-parse.y"
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
 ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2044 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2049 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2054 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2057 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2060 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2065 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2068 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2073 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2076 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2079 "cf-parse.y"
    { ospf_sh_lsadb(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf)); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2084 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2094 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2099 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2100 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2106 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2115 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2116 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2117 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2118 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2119 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2120 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2122 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2123 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2124 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2129 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2130 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2131 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2136 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2137 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2138 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2139 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2140 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2144 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2145 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2159 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2175 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2184 "cf-parse.y"
    { STATIC_CFG->check_link = (yyvsp[(4) - (5)].i); ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2188 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2197 "cf-parse.y"
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[(2) - (2)].a);
     this_srt_nh->if_name = (void *) this_srt; /* really */
   ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2204 "cf-parse.y"
    {
     this_srt_nh->masklen = (yyvsp[(3) - (3)].i) - 1; /* really */
     if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("Weight must be in range 1-256"); 
   ;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2211 "cf-parse.y"
    { this_srt->mp_next = this_srt_nh; ;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2212 "cf-parse.y"
    { last_srt_nh->mp_next = this_srt_nh; ;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2216 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2220 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2226 "cf-parse.y"
    {
      this_srt->dest = RTD_MULTIPATH;
   ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2229 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2230 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2231 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2235 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2243 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2243 "cf-parse.y"
    { ospf_proto_finish(); ;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2243 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2246 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); ;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2246 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); ;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2247 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); ;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2247 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2248 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2249 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2250 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2251 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2252 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2253 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2254 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2255 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2256 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); ;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); ;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2257 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;



/* Line 1455 of yacc.c  */
#line 7379 "cf-parse.tab.c"
      default: break;
    }
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
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
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
      if (yyn != YYPACT_NINF)
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
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
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



/* Line 1675 of yacc.c  */
#line 2259 "cf-parse.y"

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



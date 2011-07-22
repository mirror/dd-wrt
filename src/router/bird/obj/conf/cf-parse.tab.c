
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

#define P(a,b) ((a<<8) | b)

static int make_pair(int i1, int i2)
{
  unsigned u1 = i1;
  unsigned u2 = i2;

  if ((u1 > 0xFFFF) || (u2 > 0xFFFF))
    cf_error( "Can't operate with value out of bounds in pair constructor");

  return (u1 << 16) | u2;
}

struct f_tree *f_generate_rev_wcard(int from, int to, int expr)
{
	struct f_tree *ret = NULL, *last = NULL;

	while (from <= to) {
		ret = f_new_tree(); 
		ret->from.type = ret->to.type = T_PAIR;
		ret->from.val.i = ret->to.val.i = make_pair(from, expr); 
		ret->left = last;

		from++; last = ret;
	}
	return ret;
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
#line 303 "cf-parse.tab.c"

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
     GEQ = 262,
     LEQ = 263,
     NEQ = 264,
     AND = 265,
     OR = 266,
     PO = 267,
     PC = 268,
     NUM = 269,
     ENUM = 270,
     RTRID = 271,
     IPA = 272,
     SYM = 273,
     TEXT = 274,
     PREFIX_DUMMY = 275,
     DEFINE = 276,
     ON = 277,
     OFF = 278,
     YES = 279,
     NO = 280,
     LOG = 281,
     SYSLOG = 282,
     ALL = 283,
     DEBUG = 284,
     TRACE = 285,
     INFO = 286,
     REMOTE = 287,
     WARNING = 288,
     ERROR = 289,
     AUTH = 290,
     FATAL = 291,
     BUG = 292,
     STDERR = 293,
     SOFT = 294,
     TIMEFORMAT = 295,
     ISO = 296,
     SHORT = 297,
     LONG = 298,
     BASE = 299,
     NAME = 300,
     CONFIGURE = 301,
     DOWN = 302,
     KERNEL = 303,
     PERSIST = 304,
     SCAN = 305,
     TIME = 306,
     LEARN = 307,
     DEVICE = 308,
     ROUTES = 309,
     ASYNC = 310,
     TABLE = 311,
     KRT_PREFSRC = 312,
     KRT_REALM = 313,
     ROUTER = 314,
     ID = 315,
     PROTOCOL = 316,
     PREFERENCE = 317,
     DISABLED = 318,
     DIRECT = 319,
     INTERFACE = 320,
     IMPORT = 321,
     EXPORT = 322,
     FILTER = 323,
     NONE = 324,
     STATES = 325,
     FILTERS = 326,
     PASSWORD = 327,
     FROM = 328,
     PASSIVE = 329,
     TO = 330,
     EVENTS = 331,
     PACKETS = 332,
     PROTOCOLS = 333,
     INTERFACES = 334,
     PRIMARY = 335,
     STATS = 336,
     COUNT = 337,
     FOR = 338,
     COMMANDS = 339,
     PREEXPORT = 340,
     GENERATE = 341,
     LISTEN = 342,
     BGP = 343,
     V6ONLY = 344,
     DUAL = 345,
     ADDRESS = 346,
     PORT = 347,
     PASSWORDS = 348,
     DESCRIPTION = 349,
     RELOAD = 350,
     IN = 351,
     OUT = 352,
     MRTDUMP = 353,
     MESSAGES = 354,
     RESTRICT = 355,
     MEMORY = 356,
     IGP_METRIC = 357,
     SHOW = 358,
     STATUS = 359,
     SUMMARY = 360,
     ROUTE = 361,
     SYMBOLS = 362,
     DUMP = 363,
     RESOURCES = 364,
     SOCKETS = 365,
     NEIGHBORS = 366,
     ATTRIBUTES = 367,
     ECHO = 368,
     DISABLE = 369,
     ENABLE = 370,
     RESTART = 371,
     FUNCTION = 372,
     PRINT = 373,
     PRINTN = 374,
     UNSET = 375,
     RETURN = 376,
     ACCEPT = 377,
     REJECT = 378,
     QUITBIRD = 379,
     INT = 380,
     BOOL = 381,
     IP = 382,
     PREFIX = 383,
     PAIR = 384,
     QUAD = 385,
     SET = 386,
     STRING = 387,
     BGPMASK = 388,
     BGPPATH = 389,
     CLIST = 390,
     IF = 391,
     THEN = 392,
     ELSE = 393,
     CASE = 394,
     TRUE = 395,
     FALSE = 396,
     GW = 397,
     NET = 398,
     MASK = 399,
     PROTO = 400,
     SOURCE = 401,
     SCOPE = 402,
     CAST = 403,
     DEST = 404,
     LEN = 405,
     DEFINED = 406,
     ADD = 407,
     DELETE = 408,
     CONTAINS = 409,
     RESET = 410,
     PREPEND = 411,
     FIRST = 412,
     LAST = 413,
     MATCH = 414,
     EMPTY = 415,
     WHERE = 416,
     EVAL = 417,
     LOCAL = 418,
     NEIGHBOR = 419,
     AS = 420,
     HOLD = 421,
     CONNECT = 422,
     RETRY = 423,
     KEEPALIVE = 424,
     MULTIHOP = 425,
     STARTUP = 426,
     VIA = 427,
     NEXT = 428,
     HOP = 429,
     SELF = 430,
     DEFAULT = 431,
     PATH = 432,
     METRIC = 433,
     START = 434,
     DELAY = 435,
     FORGET = 436,
     WAIT = 437,
     AFTER = 438,
     BGP_PATH = 439,
     BGP_LOCAL_PREF = 440,
     BGP_MED = 441,
     BGP_ORIGIN = 442,
     BGP_NEXT_HOP = 443,
     BGP_ATOMIC_AGGR = 444,
     BGP_AGGREGATOR = 445,
     BGP_COMMUNITY = 446,
     RR = 447,
     RS = 448,
     CLIENT = 449,
     CLUSTER = 450,
     AS4 = 451,
     ADVERTISE = 452,
     IPV4 = 453,
     CAPABILITIES = 454,
     LIMIT = 455,
     PREFER = 456,
     OLDER = 457,
     MISSING = 458,
     LLADDR = 459,
     DROP = 460,
     IGNORE = 461,
     REFRESH = 462,
     INTERPRET = 463,
     COMMUNITIES = 464,
     BGP_ORIGINATOR_ID = 465,
     BGP_CLUSTER_LIST = 466,
     IGP = 467,
     GATEWAY = 468,
     RECURSIVE = 469,
     MED = 470,
     OSPF = 471,
     AREA = 472,
     OSPF_METRIC1 = 473,
     OSPF_METRIC2 = 474,
     OSPF_TAG = 475,
     OSPF_ROUTER_ID = 476,
     RFC1583COMPAT = 477,
     STUB = 478,
     TICK = 479,
     COST = 480,
     RETRANSMIT = 481,
     HELLO = 482,
     TRANSMIT = 483,
     PRIORITY = 484,
     DEAD = 485,
     TYPE = 486,
     BROADCAST = 487,
     BCAST = 488,
     NONBROADCAST = 489,
     NBMA = 490,
     POINTOPOINT = 491,
     PTP = 492,
     POINTOMULTIPOINT = 493,
     PTMP = 494,
     SIMPLE = 495,
     AUTHENTICATION = 496,
     STRICT = 497,
     CRYPTOGRAPHIC = 498,
     ELIGIBLE = 499,
     POLL = 500,
     NETWORKS = 501,
     HIDDEN = 502,
     VIRTUAL = 503,
     CHECK = 504,
     LINK = 505,
     RX = 506,
     BUFFER = 507,
     LARGE = 508,
     NORMAL = 509,
     STUBNET = 510,
     LSADB = 511,
     ECMP = 512,
     WEIGHT = 513,
     TOPOLOGY = 514,
     STATE = 515,
     PIPE = 516,
     PEER = 517,
     MODE = 518,
     OPAQUE = 519,
     TRANSPARENT = 520,
     RIP = 521,
     INFINITY = 522,
     PERIOD = 523,
     GARBAGE = 524,
     TIMEOUT = 525,
     MULTICAST = 526,
     QUIET = 527,
     NOLISTEN = 528,
     VERSION1 = 529,
     PLAINTEXT = 530,
     MD5 = 531,
     HONOR = 532,
     NEVER = 533,
     ALWAYS = 534,
     RIP_METRIC = 535,
     RIP_TAG = 536,
     STATIC = 537,
     PROHIBIT = 538,
     MULTIPATH = 539
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 224 "cf-parse.y"

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
#line 647 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 659 "cf-parse.tab.c"

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
#define YYLAST   1947

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  306
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  200
/* YYNRULES -- Number of rules.  */
#define YYNRULES  636
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1197

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   539

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,     2,    29,     2,     2,
     296,   297,    27,    25,   302,    26,    31,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   299,   298,
      22,    21,    23,   303,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   304,     2,   305,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   300,     2,   301,    24,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    32,    33,    34,    35,
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
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295
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
     693,   699,   708,   721,   727,   733,   739,   741,   746,   748,
     752,   756,   758,   761,   764,   771,   773,   777,   778,   783,
     787,   789,   793,   797,   801,   804,   807,   810,   813,   814,
     817,   820,   821,   827,   829,   831,   833,   835,   837,   839,
     841,   845,   849,   851,   853,   854,   859,   861,   863,   865,
     867,   869,   871,   873,   875,   877,   881,   885,   889,   893,
     897,   901,   905,   909,   913,   917,   921,   925,   929,   933,
     936,   941,   943,   945,   947,   949,   952,   955,   959,   963,
     970,   974,   978,   982,   986,   993,  1000,  1007,  1012,  1014,
    1016,  1018,  1020,  1022,  1024,  1026,  1027,  1029,  1033,  1035,
    1039,  1040,  1042,  1047,  1054,  1059,  1063,  1069,  1075,  1080,
    1087,  1091,  1094,  1100,  1106,  1115,  1124,  1133,  1136,  1140,
    1144,  1150,  1157,  1164,  1171,  1176,  1181,  1187,  1194,  1201,
    1207,  1211,  1216,  1222,  1228,  1234,  1240,  1245,  1250,  1256,
    1262,  1268,  1274,  1280,  1286,  1292,  1299,  1306,  1315,  1322,
    1329,  1335,  1340,  1346,  1351,  1357,  1362,  1368,  1374,  1377,
    1381,  1385,  1387,  1390,  1393,  1398,  1401,  1403,  1406,  1411,
    1412,  1416,  1420,  1423,  1428,  1431,  1434,  1436,  1441,  1443,
    1445,  1446,  1450,  1453,  1456,  1459,  1464,  1466,  1467,  1471,
    1472,  1475,  1478,  1482,  1485,  1488,  1492,  1495,  1498,  1501,
    1503,  1507,  1510,  1513,  1516,  1519,  1522,  1525,  1529,  1532,
    1535,  1538,  1541,  1544,  1547,  1550,  1553,  1557,  1560,  1564,
    1567,  1571,  1575,  1580,  1583,  1586,  1589,  1593,  1597,  1601,
    1603,  1604,  1607,  1609,  1611,  1614,  1618,  1619,  1622,  1624,
    1626,  1629,  1633,  1634,  1635,  1639,  1640,  1644,  1648,  1650,
    1651,  1656,  1663,  1670,  1677,  1685,  1692,  1700,  1707,  1710,
    1714,  1718,  1724,  1729,  1734,  1737,  1741,  1745,  1750,  1755,
    1760,  1766,  1772,  1777,  1781,  1786,  1791,  1796,  1801,  1803,
    1805,  1807,  1809,  1811,  1813,  1815,  1817,  1818,  1821,  1824,
    1825,  1829,  1830,  1834,  1835,  1839,  1842,  1846,  1850,  1856,
    1860,  1863,  1866,  1870,  1872,  1875,  1879,  1883,  1887,  1890,
    1893,  1896,  1901,  1903,  1905,  1907,  1909,  1911,  1913,  1915,
    1917,  1919,  1921,  1923,  1925,  1927,  1929,  1931,  1933,  1935,
    1937,  1939,  1941,  1943,  1945,  1947,  1949,  1951,  1953,  1955,
    1957,  1959,  1961,  1963,  1965,  1967,  1969,  1971,  1973,  1975,
    1977,  1979,  1981,  1983,  1985,  1987,  1989,  1991,  1993,  1995,
    1997,  1999,  2001,  2004,  2007,  2010,  2013,  2016,  2019,  2022,
    2025,  2029,  2033,  2037,  2041,  2045,  2049,  2053,  2055,  2057,
    2059,  2061,  2063,  2065,  2067,  2069,  2071,  2073,  2075,  2077,
    2079,  2081,  2083,  2085,  2087,  2089,  2091
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     307,     0,    -1,   308,     3,    -1,     4,   501,    -1,    -1,
     308,   500,    -1,    14,    -1,   296,   436,   297,    -1,    18,
      -1,    32,    18,    21,   309,   298,    -1,    32,    18,    21,
      17,   298,    -1,   309,    -1,    33,    -1,    35,    -1,    34,
      -1,    36,    -1,    -1,    17,    -1,    18,    -1,   312,   315,
      -1,   313,    -1,   312,    -1,    28,   309,    -1,   299,   312,
      -1,    19,    -1,    19,    -1,    -1,    37,   320,   321,   298,
      -1,    56,    19,    -1,    -1,    19,    -1,    38,   319,    -1,
      49,    -1,    39,    -1,   300,   322,   301,    -1,   323,    -1,
     322,   302,   323,    -1,    40,    -1,    41,    -1,    42,    -1,
      43,    -1,    44,    -1,    45,    -1,    46,    -1,    47,    -1,
      48,    -1,   109,    89,   361,   298,    -1,   109,    19,   298,
      -1,   117,    -1,    72,    -1,    55,    -1,    37,    -1,   325,
      19,    -1,   325,    19,   309,    19,    -1,   325,    52,    53,
      -1,   325,    52,    54,    -1,    51,   326,   298,    -1,    57,
     331,     3,    -1,    57,    50,   331,     3,    -1,    58,     3,
      -1,    -1,    19,    -1,   343,    59,    -1,    60,   311,    -1,
      61,    62,   309,    -1,    63,   311,    -1,    64,    65,   311,
      -1,   343,    64,    -1,    61,    62,   309,    -1,    91,   317,
     314,    -1,    59,    67,   309,    -1,    70,    71,   338,   298,
      -1,    14,    -1,    16,    -1,    17,    -1,    98,    99,   340,
     298,    -1,    -1,   340,   341,    -1,   102,   312,    -1,   103,
     309,    -1,   100,    -1,   101,    -1,    67,    18,    -1,    72,
      -1,    -1,    18,    -1,    -1,    73,   309,    -1,    74,   311,
      -1,    40,   358,    -1,   109,   361,    -1,    77,   346,    -1,
      78,   346,    -1,    67,   347,    -1,    70,    71,   338,    -1,
     105,    19,    -1,    79,   409,    -1,   410,    -1,    39,    -1,
      80,    -1,    18,    -1,    40,    89,   358,    -1,    40,    95,
     309,    -1,    -1,    19,    -1,   314,    -1,    19,   314,    -1,
      -1,    26,    -1,   349,   351,   350,    -1,   352,    -1,   353,
     302,   352,    -1,   343,    75,    -1,   354,   344,   300,    -1,
     355,   345,   298,    -1,   355,   357,   298,    -1,    -1,    76,
     356,   353,    -1,    39,    -1,    34,    -1,   300,   359,   301,
      -1,   360,    -1,   359,   302,   360,    -1,    81,    -1,    65,
      -1,    82,    -1,    90,    -1,    87,    -1,    88,    -1,    39,
      -1,    34,    -1,   300,   362,   301,    -1,   363,    -1,   362,
     302,   363,    -1,    81,    -1,   110,    -1,   104,   300,   365,
     301,    -1,   366,    -1,    -1,   366,   298,   365,    -1,   367,
     300,   368,   301,    -1,   367,    -1,    83,    19,    -1,    -1,
      97,    84,   316,   298,   368,    -1,    97,    86,   316,   298,
     368,    -1,   133,    84,   316,   298,   368,    -1,   133,    86,
     316,   298,   368,    -1,    71,   309,   298,   368,    -1,   114,
     115,     3,    -1,   114,   112,     3,    -1,   114,    89,   400,
       3,    -1,   114,    89,    39,   400,     3,    -1,    18,    -1,
      -1,   114,    90,     3,    -1,   114,    90,   116,     3,    -1,
     114,   117,   377,     3,    -1,    -1,   377,   313,    -1,   377,
      94,   314,    -1,   377,    67,    18,    -1,   377,    79,   409,
      -1,   377,   410,    -1,   377,    39,    -1,   377,    91,    -1,
     377,   378,    18,    -1,   377,    72,    18,    -1,   377,    92,
      -1,   377,    93,    -1,    96,    -1,    78,    -1,   114,   118,
     373,     3,    -1,   119,   120,     3,    -1,   119,   121,     3,
      -1,   119,    90,     3,    -1,   119,   122,     3,    -1,   119,
     123,     3,    -1,   119,    65,     3,    -1,   119,    89,     3,
      -1,   124,   388,   389,     3,    -1,    39,    -1,    34,    -1,
      14,    -1,    -1,    14,    -1,   125,   399,     3,    -1,   126,
     399,     3,    -1,   127,   399,     3,    -1,   106,   399,     3,
      -1,   106,   107,   399,     3,    -1,   106,   108,   399,     3,
      -1,    40,   399,   358,     3,    -1,   109,   399,   361,     3,
      -1,   111,     3,    -1,    18,    -1,    39,    -1,    19,    -1,
      18,    -1,    -1,    19,    -1,    -1,    79,    18,   402,   408,
      -1,   173,   436,    -1,   136,    -1,   137,    -1,   138,    -1,
     139,    -1,   140,    -1,   141,    -1,   143,    -1,   144,    -1,
     145,    -1,   146,    -1,   404,   142,    -1,   404,    18,    -1,
      -1,   405,   298,   406,    -1,   405,    -1,   407,   298,   405,
      -1,   412,    -1,    18,    -1,   408,    -1,   172,   436,    -1,
     296,   407,   297,    -1,   296,   297,    -1,   406,   300,   415,
     301,    -1,    -1,   128,    18,   414,   411,   412,    -1,    -1,
     416,    -1,   442,    -1,   416,   442,    -1,   442,    -1,   300,
     415,   301,    -1,    17,    -1,   309,    -1,    16,    -1,   418,
      -1,    15,    -1,   296,   309,   302,   309,   297,    -1,   296,
     309,   302,   309,    31,    31,   309,   297,    -1,   296,   309,
     302,   309,   297,    31,    31,   296,   309,   302,   309,   297,
      -1,   296,    27,   302,    27,   297,    -1,   296,   309,   302,
      27,   297,    -1,   296,    27,   302,   309,   297,    -1,   419,
      -1,   419,    31,    31,   419,    -1,   420,    -1,   421,   302,
     420,    -1,    17,    28,    14,    -1,   422,    -1,   422,    25,
      -1,   422,    26,    -1,   422,   300,    14,   302,    14,   301,
      -1,   423,    -1,   424,   302,   423,    -1,    -1,   425,   420,
     299,   415,    -1,   425,     6,   415,    -1,   434,    -1,   296,
     436,   297,    -1,    12,   428,    13,    -1,    28,   429,    28,
      -1,    14,   428,    -1,    27,   428,    -1,   303,   428,    -1,
     426,   428,    -1,    -1,    14,   429,    -1,   303,   429,    -1,
      -1,   296,   436,   302,   436,   297,    -1,    14,    -1,   151,
      -1,   152,    -1,    19,    -1,   418,    -1,   422,    -1,    16,
      -1,   304,   421,   305,    -1,   304,   424,   305,    -1,    15,
      -1,   427,    -1,    -1,    18,   296,   441,   297,    -1,    18,
      -1,    84,    -1,   153,    -1,   154,    -1,   156,    -1,   157,
      -1,   158,    -1,   159,    -1,   160,    -1,   296,   436,   297,
      -1,   436,    25,   436,    -1,   436,    26,   436,    -1,   436,
      27,   436,    -1,   436,    28,   436,    -1,   436,    10,   436,
      -1,   436,    11,   436,    -1,   436,    21,   436,    -1,   436,
       9,   436,    -1,   436,    22,   436,    -1,   436,     8,   436,
      -1,   436,    23,   436,    -1,   436,     7,   436,    -1,   436,
      24,   436,    -1,    30,   436,    -1,   162,   296,   436,   297,
      -1,   434,    -1,   431,    -1,   430,    -1,    73,    -1,   432,
     435,    -1,   432,   505,    -1,   436,    31,   138,    -1,   436,
      31,   161,    -1,   436,    31,   155,   296,   436,   297,    -1,
     436,    31,   168,    -1,   436,    31,   169,    -1,    25,   171,
      25,    -1,    26,   171,    26,    -1,   167,   296,   436,   302,
     436,   297,    -1,   163,   296,   436,   302,   436,   297,    -1,
     164,   296,   436,   302,   436,   297,    -1,    18,   296,   441,
     297,    -1,   135,    -1,   133,    -1,   134,    -1,    45,    -1,
     129,    -1,   130,    -1,   436,    -1,    -1,   438,    -1,   438,
     302,   439,    -1,   436,    -1,   436,   302,   440,    -1,    -1,
     440,    -1,   147,   436,   148,   417,    -1,   147,   436,   148,
     417,   149,   417,    -1,    18,    21,   436,   298,    -1,   132,
     436,   298,    -1,   432,   505,    21,   436,   298,    -1,   432,
     435,    21,   436,   298,    -1,    73,    21,   436,   298,    -1,
     131,   296,   432,   505,   297,   298,    -1,   437,   439,   298,
      -1,   433,   298,    -1,   150,   436,   300,   425,   301,    -1,
     432,   505,    31,   171,   298,    -1,   432,   505,    31,   167,
     296,   436,   297,   298,    -1,   432,   505,    31,   163,   296,
     436,   297,   298,    -1,   432,   505,    31,   164,   296,   436,
     297,   298,    -1,   343,    99,    -1,   443,   344,   300,    -1,
     444,   345,   298,    -1,   444,   174,   176,   309,   298,    -1,
     444,   174,   312,   176,   309,   298,    -1,   444,   175,   312,
     176,   309,   298,    -1,   444,   203,   206,    71,   338,   298,
      -1,   444,   203,   205,   298,    -1,   444,   204,   205,   298,
      -1,   444,   177,    62,   309,   298,    -1,   444,   182,   177,
      62,   309,   298,    -1,   444,   178,   179,    62,   309,   298,
      -1,   444,   180,    62,   309,   298,    -1,   444,   181,   298,
      -1,   444,   181,   309,   298,    -1,   444,   184,   185,   186,
     298,    -1,   444,   214,   215,   186,   298,    -1,   444,   214,
     215,   216,   298,    -1,   444,   214,   215,   217,   298,    -1,
     444,   224,    75,   298,    -1,   444,   224,   225,   298,    -1,
     444,   188,   189,   311,   298,    -1,   444,   226,   189,   311,
     298,    -1,   444,   223,   189,   311,   298,    -1,   444,   212,
     213,   311,   298,    -1,   444,   187,   197,   309,   298,    -1,
     444,   187,   196,   309,   298,    -1,   444,   157,   102,   312,
     298,    -1,   444,   190,   191,    62,   309,   298,    -1,   444,
      45,   192,    62,   309,   298,    -1,   444,    45,   193,    62,
     309,   302,   309,   298,    -1,   444,   125,   194,    45,   311,
     298,    -1,   444,   126,   117,   218,   311,   298,    -1,   444,
     126,   207,   311,   298,    -1,   444,   210,   311,   298,    -1,
     444,   208,   209,   311,   298,    -1,   444,    83,    19,   298,
      -1,   444,   117,   211,   309,   298,    -1,   444,    85,   311,
     298,    -1,   444,   219,   220,   311,   298,    -1,   444,   223,
      67,   347,   298,    -1,   343,   227,    -1,   445,   344,   300,
      -1,   446,   447,   298,    -1,   345,    -1,   233,   311,    -1,
     268,   311,    -1,   268,   311,   211,   309,    -1,   235,   309,
      -1,   449,    -1,   228,   338,    -1,   448,   300,   450,   301,
      -1,    -1,   450,   451,   298,    -1,   234,   236,   309,    -1,
     234,   311,    -1,   257,   300,   461,   301,    -1,   266,   452,
      -1,    76,   472,    -1,   456,    -1,   453,   300,   454,   301,
      -1,   453,    -1,   313,    -1,    -1,   454,   455,   298,    -1,
     258,   311,    -1,   116,   311,    -1,   236,   309,    -1,   459,
     300,   457,   301,    -1,   459,    -1,    -1,   457,   458,   298,
      -1,    -1,   238,   309,    -1,   237,   309,    -1,   239,   191,
     309,    -1,   193,   309,    -1,   241,   309,    -1,   241,    93,
     309,    -1,   252,    80,    -1,   252,   251,    -1,   252,   254,
      -1,   364,    -1,   259,   261,   338,    -1,   236,   309,    -1,
     238,   309,    -1,   256,   309,    -1,   237,   309,    -1,   193,
     309,    -1,   241,   309,    -1,   241,    93,   309,    -1,   242,
     243,    -1,   242,   244,    -1,   242,   245,    -1,   242,   246,
      -1,   242,   247,    -1,   242,   248,    -1,   242,   249,    -1,
     242,   250,    -1,   239,   191,   309,    -1,   240,   309,    -1,
     253,   245,   311,    -1,   234,   311,    -1,   260,   261,   311,
      -1,   268,   269,   309,    -1,   122,   300,   465,   301,    -1,
     252,    80,    -1,   252,   251,    -1,   252,   254,    -1,   262,
     263,   264,    -1,   262,   263,   265,    -1,   262,   263,   309,
      -1,   364,    -1,    -1,   461,   462,    -1,   463,    -1,   464,
      -1,   313,   298,    -1,   313,   258,   298,    -1,    -1,   465,
     466,    -1,   467,    -1,   468,    -1,    17,   298,    -1,    17,
     255,   298,    -1,    -1,    -1,   470,   460,   298,    -1,    -1,
     300,   470,   301,    -1,   469,   353,   471,    -1,    19,    -1,
      -1,   114,   227,   373,     3,    -1,   114,   227,   122,   373,
     473,     3,    -1,   114,   227,    76,   373,   473,     3,    -1,
     114,   227,   270,   373,   473,     3,    -1,   114,   227,   270,
      39,   373,   473,     3,    -1,   114,   227,   271,   373,   473,
       3,    -1,   114,   227,   271,    39,   373,   473,     3,    -1,
     114,   227,   267,   373,   473,     3,    -1,   343,   272,    -1,
     482,   344,   300,    -1,   483,   345,   298,    -1,   483,   273,
      67,    18,   298,    -1,   483,   274,   275,   298,    -1,   483,
     274,   276,   298,    -1,   343,   277,    -1,   484,   344,   300,
      -1,   485,   345,   298,    -1,   485,   278,   309,   298,    -1,
     485,   103,   309,   298,    -1,   485,   279,   309,   298,    -1,
     485,   280,    62,   309,   298,    -1,   485,   281,    62,   309,
     298,    -1,   485,   252,   486,   298,    -1,   485,   364,   298,
      -1,   485,   288,   290,   298,    -1,   485,   288,   175,   298,
      -1,   485,   288,   289,   298,    -1,   485,    76,   492,   298,
      -1,   286,    -1,   287,    -1,    80,    -1,   243,    -1,   282,
      -1,   283,    -1,   284,    -1,   285,    -1,    -1,   189,   309,
      -1,   274,   487,    -1,    -1,   489,   488,   298,    -1,    -1,
     300,   489,   301,    -1,    -1,   491,   353,   490,    -1,   343,
     293,    -1,   493,   344,   300,    -1,   494,   345,   298,    -1,
     494,   260,   261,   311,   298,    -1,   494,   498,   298,    -1,
     117,   313,    -1,   183,   312,    -1,   496,   269,   309,    -1,
     496,    -1,   497,   496,    -1,   495,   183,   312,    -1,   495,
     183,    19,    -1,   495,   295,   497,    -1,   495,   216,    -1,
     495,   134,    -1,   495,   294,    -1,   114,   293,   373,     3,
      -1,   298,    -1,   310,    -1,   318,    -1,   324,    -1,   327,
      -1,   337,    -1,   339,    -1,   342,    -1,   502,    -1,   348,
      -1,   401,    -1,   403,    -1,   413,    -1,   328,    -1,   329,
      -1,   330,    -1,   369,    -1,   370,    -1,   371,    -1,   372,
      -1,   374,    -1,   375,    -1,   376,    -1,   379,    -1,   380,
      -1,   381,    -1,   382,    -1,   383,    -1,   384,    -1,   385,
      -1,   386,    -1,   387,    -1,   390,    -1,   391,    -1,   392,
      -1,   393,    -1,   394,    -1,   395,    -1,   396,    -1,   397,
      -1,   398,    -1,   474,    -1,   475,    -1,   476,    -1,   477,
      -1,   478,    -1,   479,    -1,   480,    -1,   481,    -1,   499,
      -1,   503,   301,    -1,   504,   301,    -1,   355,   301,    -1,
     444,   301,    -1,   446,   301,    -1,   483,   301,    -1,   485,
     301,    -1,   494,   301,    -1,   332,   344,   300,    -1,   503,
     345,   298,    -1,   503,   333,   298,    -1,   503,   336,   298,
      -1,   334,   344,   300,    -1,   504,   345,   298,    -1,   504,
     335,   298,    -1,    68,    -1,    69,    -1,   113,    -1,     5,
      -1,   198,    -1,   195,    -1,   199,    -1,   197,    -1,   196,
      -1,   200,    -1,   201,    -1,   202,    -1,   221,    -1,   222,
      -1,   229,    -1,   230,    -1,   231,    -1,   232,    -1,   291,
      -1,   292,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   383,   383,   384,   387,   389,   396,   397,   398,   402,
     406,   415,   416,   417,   418,   419,   420,   426,   427,   434,
     441,   442,   446,   450,   457,   465,   466,   472,   481,   482,
     486,   491,   492,   496,   497,   501,   502,   506,   507,   508,
     509,   510,   511,   512,   513,   514,   520,   521,   530,   531,
     532,   533,   536,   537,   538,   539,   543,   550,   553,   556,
     560,   561,   569,   585,   586,   590,   597,   603,   616,   620,
     633,   646,   652,   653,   654,   665,   667,   669,   673,   674,
     675,   676,   683,   691,   695,   701,   707,   709,   713,   714,
     715,   716,   717,   718,   719,   720,   724,   725,   726,   727,
     731,   739,   740,   748,   756,   757,   758,   762,   763,   767,
     772,   773,   780,   789,   790,   791,   795,   804,   810,   811,
     812,   816,   817,   821,   822,   823,   824,   825,   826,   832,
     833,   834,   838,   839,   843,   844,   850,   851,   854,   856,
     860,   861,   865,   883,   884,   885,   886,   887,   888,   896,
     899,   902,   905,   909,   910,   913,   916,   919,   923,   929,
     935,   942,   947,   952,   957,   961,   965,   975,   983,   987,
     994,   995,   998,  1002,  1004,  1006,  1008,  1010,  1012,  1014,
    1017,  1023,  1024,  1025,  1029,  1030,  1036,  1038,  1040,  1042,
    1044,  1046,  1050,  1054,  1057,  1061,  1062,  1063,  1067,  1068,
    1069,  1077,  1077,  1087,  1091,  1092,  1093,  1094,  1095,  1096,
    1097,  1098,  1099,  1100,  1101,  1121,  1132,  1133,  1140,  1141,
    1148,  1157,  1161,  1165,  1189,  1190,  1194,  1207,  1207,  1223,
    1224,  1227,  1228,  1232,  1235,  1244,  1248,  1249,  1250,  1251,
    1255,  1261,  1267,  1273,  1279,  1285,  1292,  1297,  1305,  1306,
    1310,  1317,  1318,  1319,  1320,  1327,  1328,  1331,  1332,  1337,
    1349,  1350,  1354,  1355,  1359,  1360,  1361,  1362,  1363,  1367,
    1368,  1369,  1373,  1386,  1387,  1388,  1389,  1390,  1391,  1392,
    1393,  1394,  1395,  1396,  1406,  1410,  1433,  1467,  1469,  1470,
    1471,  1472,  1473,  1474,  1475,  1479,  1480,  1481,  1482,  1483,
    1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,
    1494,  1496,  1497,  1498,  1500,  1502,  1504,  1506,  1507,  1508,
    1509,  1510,  1520,  1521,  1522,  1523,  1524,  1529,  1552,  1553,
    1554,  1555,  1556,  1557,  1561,  1564,  1565,  1566,  1575,  1582,
    1591,  1592,  1596,  1602,  1612,  1621,  1627,  1632,  1639,  1644,
    1650,  1651,  1652,  1660,  1662,  1663,  1664,  1670,  1692,  1693,
    1694,  1695,  1696,  1702,  1703,  1704,  1705,  1706,  1707,  1708,
    1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,  1718,
    1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,  1727,  1728,
    1729,  1730,  1731,  1732,  1733,  1734,  1735,  1736,  1746,  1757,
    1758,  1762,  1763,  1764,  1765,  1766,  1767,  1770,  1781,  1784,
    1786,  1790,  1791,  1792,  1793,  1794,  1795,  1799,  1800,  1804,
    1812,  1814,  1818,  1819,  1820,  1824,  1825,  1828,  1830,  1833,
    1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,  1843,
    1846,  1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,  1876,
    1877,  1878,  1879,  1880,  1881,  1882,  1883,  1884,  1885,  1886,
    1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,  1895,  1896,
    1899,  1901,  1905,  1906,  1908,  1917,  1927,  1929,  1933,  1934,
    1936,  1945,  1956,  1976,  1978,  1981,  1983,  1987,  1991,  1992,
    1996,  1999,  2002,  2007,  2010,  2015,  2018,  2021,  2027,  2035,
    2036,  2037,  2042,  2043,  2049,  2056,  2057,  2058,  2059,  2060,
    2061,  2062,  2063,  2064,  2065,  2066,  2067,  2068,  2072,  2073,
    2074,  2079,  2080,  2081,  2082,  2083,  2086,  2087,  2088,  2091,
    2093,  2096,  2098,  2102,  2111,  2118,  2125,  2126,  2127,  2128,
    2131,  2140,  2147,  2154,  2155,  2159,  2163,  2169,  2172,  2173,
    2174,  2177,  2184,  2184,  2184,  2184,  2184,  2184,  2184,  2184,
    2184,  2184,  2184,  2184,  2184,  2185,  2185,  2185,  2185,  2185,
    2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,
    2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,
    2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,  2185,
    2185,  2185,  2186,  2186,  2186,  2186,  2186,  2186,  2186,  2186,
    2187,  2187,  2187,  2187,  2188,  2188,  2188,  2189,  2189,  2189,
    2190,  2190,  2191,  2192,  2193,  2194,  2195,  2196,  2197,  2198,
    2199,  2200,  2200,  2200,  2200,  2200,  2200
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "CLI_MARKER", "INVALID_TOKEN",
  "ELSECOL", "GEQ", "LEQ", "NEQ", "AND", "OR", "PO", "PC", "NUM", "ENUM",
  "RTRID", "IPA", "SYM", "TEXT", "PREFIX_DUMMY", "'='", "'<'", "'>'",
  "'~'", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "'.'", "DEFINE", "ON",
  "OFF", "YES", "NO", "LOG", "SYSLOG", "ALL", "DEBUG", "TRACE", "INFO",
  "REMOTE", "WARNING", "ERROR", "AUTH", "FATAL", "BUG", "STDERR", "SOFT",
  "TIMEFORMAT", "ISO", "SHORT", "LONG", "BASE", "NAME", "CONFIGURE",
  "DOWN", "KERNEL", "PERSIST", "SCAN", "TIME", "LEARN", "DEVICE", "ROUTES",
  "ASYNC", "TABLE", "KRT_PREFSRC", "KRT_REALM", "ROUTER", "ID", "PROTOCOL",
  "PREFERENCE", "DISABLED", "DIRECT", "INTERFACE", "IMPORT", "EXPORT",
  "FILTER", "NONE", "STATES", "FILTERS", "PASSWORD", "FROM", "PASSIVE",
  "TO", "EVENTS", "PACKETS", "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS",
  "COUNT", "FOR", "COMMANDS", "PREEXPORT", "GENERATE", "LISTEN", "BGP",
  "V6ONLY", "DUAL", "ADDRESS", "PORT", "PASSWORDS", "DESCRIPTION",
  "RELOAD", "IN", "OUT", "MRTDUMP", "MESSAGES", "RESTRICT", "MEMORY",
  "IGP_METRIC", "SHOW", "STATUS", "SUMMARY", "ROUTE", "SYMBOLS", "DUMP",
  "RESOURCES", "SOCKETS", "NEIGHBORS", "ATTRIBUTES", "ECHO", "DISABLE",
  "ENABLE", "RESTART", "FUNCTION", "PRINT", "PRINTN", "UNSET", "RETURN",
  "ACCEPT", "REJECT", "QUITBIRD", "INT", "BOOL", "IP", "PREFIX", "PAIR",
  "QUAD", "SET", "STRING", "BGPMASK", "BGPPATH", "CLIST", "IF", "THEN",
  "ELSE", "CASE", "TRUE", "FALSE", "GW", "NET", "MASK", "PROTO", "SOURCE",
  "SCOPE", "CAST", "DEST", "LEN", "DEFINED", "ADD", "DELETE", "CONTAINS",
  "RESET", "PREPEND", "FIRST", "LAST", "MATCH", "EMPTY", "WHERE", "EVAL",
  "LOCAL", "NEIGHBOR", "AS", "HOLD", "CONNECT", "RETRY", "KEEPALIVE",
  "MULTIHOP", "STARTUP", "VIA", "NEXT", "HOP", "SELF", "DEFAULT", "PATH",
  "METRIC", "START", "DELAY", "FORGET", "WAIT", "AFTER", "BGP_PATH",
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
  "set_item", "set_items", "fprefix_s", "fprefix", "fprefix_set",
  "switch_body", "bgp_path_expr", "bgp_path", "bgp_path_tail1",
  "bgp_path_tail2", "dpair", "constant", "rtadot", "function_call",
  "symbol", "static_attr", "term", "break_command", "print_one",
  "print_list", "var_listn", "var_list", "cmd", "bgp_proto_start",
  "bgp_proto", "ospf_proto_start", "ospf_proto", "ospf_proto_item",
  "ospf_area_start", "ospf_area", "ospf_area_opts", "ospf_area_item",
  "ospf_stubnet", "ospf_stubnet_start", "ospf_stubnet_opts",
  "ospf_stubnet_item", "ospf_vlink", "ospf_vlink_opts", "ospf_vlink_item",
  "ospf_vlink_start", "ospf_iface_item", "pref_list", "pref_item",
  "pref_el", "pref_hid", "ipa_list", "ipa_item", "ipa_el", "ipa_ne",
  "ospf_iface_start", "ospf_iface_opts", "ospf_iface_opt_list",
  "ospf_iface", "opttext", "cmd_SHOW_OSPF", "cmd_SHOW_OSPF_NEIGHBORS",
  "cmd_SHOW_OSPF_INTERFACE", "cmd_SHOW_OSPF_TOPOLOGY",
  "cmd_SHOW_OSPF_TOPOLOGY_ALL", "cmd_SHOW_OSPF_STATE",
  "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB", "pipe_proto_start",
  "pipe_proto", "rip_cfg_start", "rip_cfg", "rip_auth", "rip_mode",
  "rip_iface_item", "rip_iface_opts", "rip_iface_opt_list",
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
     275,    61,    60,    62,   126,    43,    45,    42,    47,    37,
      33,    46,   276,   277,   278,   279,   280,   281,   282,   283,
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
     534,   535,   536,   537,   538,   539,    40,    41,    59,    58,
     123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   306,   307,   307,   308,   308,   309,   309,   309,   310,
     310,   311,   311,   311,   311,   311,   311,   312,   312,   313,
     314,   314,   315,   315,   316,   317,   317,   318,   319,   319,
     320,   320,   320,   321,   321,   322,   322,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   324,   324,   325,   325,
     325,   325,   326,   326,   326,   326,   327,   328,   329,   330,
     331,   331,   332,   333,   333,   333,   333,   334,   335,   335,
     336,   337,   338,   338,   338,   339,   340,   340,   341,   341,
     341,   341,   342,   343,   344,   344,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   346,   346,   346,   346,
     347,   348,   348,   349,   350,   350,   350,   351,   351,   352,
     353,   353,   354,   355,   355,   355,   356,   357,   358,   358,
     358,   359,   359,   360,   360,   360,   360,   360,   360,   361,
     361,   361,   362,   362,   363,   363,   364,   364,   365,   365,
     366,   366,   367,   368,   368,   368,   368,   368,   368,   369,
     370,   371,   372,   373,   373,   374,   375,   376,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     378,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   388,   388,   389,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   399,   399,   400,   400,
     400,   402,   401,   403,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   405,   406,   406,   407,   407,
     408,   409,   409,   410,   411,   411,   412,   414,   413,   415,
     415,   416,   416,   417,   417,   418,   419,   419,   419,   419,
     420,   420,   420,   420,   420,   420,   420,   420,   421,   421,
     422,   423,   423,   423,   423,   424,   424,   425,   425,   425,
     426,   426,   427,   427,   428,   428,   428,   428,   428,   429,
     429,   429,   430,   431,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   432,   433,   434,   435,   435,   435,
     435,   435,   435,   435,   435,   436,   436,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   437,   437,
     437,   437,   437,   437,   438,   439,   439,   439,   440,   440,
     441,   441,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   443,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   445,   446,
     446,   447,   447,   447,   447,   447,   447,   448,   449,   450,
     450,   451,   451,   451,   451,   451,   451,   452,   452,   453,
     454,   454,   455,   455,   455,   456,   456,   457,   457,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     459,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     460,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     460,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     461,   461,   462,   462,   463,   464,   465,   465,   466,   466,
     467,   468,   469,   470,   470,   471,   471,   472,   473,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     483,   483,   483,   483,   484,   485,   485,   485,   485,   485,
     485,   485,   485,   485,   485,   485,   485,   485,   486,   486,
     486,   487,   487,   487,   487,   487,   488,   488,   488,   489,
     489,   490,   490,   491,   492,   493,   494,   494,   494,   494,
     495,   496,   496,   497,   497,   498,   498,   498,   498,   498,
     498,   499,   500,   500,   500,   500,   500,   500,   500,   500,
     500,   500,   500,   500,   500,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   501,   501,   501,   501,   501,   501,   501,   501,
     501,   501,   502,   502,   502,   502,   502,   502,   502,   502,
     503,   503,   503,   503,   504,   504,   504,   505,   505,   505,
     505,   505,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   505,   505,   505,   505
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
       5,     8,    12,     5,     5,     5,     1,     4,     1,     3,
       3,     1,     2,     2,     6,     1,     3,     0,     4,     3,
       1,     3,     3,     3,     2,     2,     2,     2,     0,     2,
       2,     0,     5,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     1,     1,     0,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       4,     1,     1,     1,     1,     2,     2,     3,     3,     6,
       3,     3,     3,     3,     6,     6,     6,     4,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     3,     1,     3,
       0,     1,     4,     6,     4,     3,     5,     5,     4,     6,
       3,     2,     5,     5,     8,     8,     8,     2,     3,     3,
       5,     6,     6,     6,     4,     4,     5,     6,     6,     5,
       3,     4,     5,     5,     5,     5,     4,     4,     5,     5,
       5,     5,     5,     5,     5,     6,     6,     8,     6,     6,
       5,     4,     5,     4,     5,     4,     5,     5,     2,     3,
       3,     1,     2,     2,     4,     2,     1,     2,     4,     0,
       3,     3,     2,     4,     2,     2,     1,     4,     1,     1,
       0,     3,     2,     2,     2,     4,     1,     0,     3,     0,
       2,     2,     3,     2,     2,     3,     2,     2,     2,     1,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     2,     3,     2,
       3,     3,     4,     2,     2,     2,     3,     3,     3,     1,
       0,     2,     1,     1,     2,     3,     0,     2,     1,     1,
       2,     3,     0,     0,     3,     0,     3,     3,     1,     0,
       4,     6,     6,     6,     7,     6,     7,     6,     2,     3,
       3,     5,     4,     4,     2,     3,     3,     4,     4,     4,
       5,     5,     4,     3,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     2,     2,     0,
       3,     0,     3,     0,     3,     2,     3,     3,     5,     3,
       2,     2,     3,     1,     2,     3,     3,     3,     2,     2,
       2,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   565,   566,   567,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   587,   588,
     589,   590,   591,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,     3,     1,     2,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   284,   552,   553,
     554,   555,   556,    84,    84,   557,   558,   559,     0,   561,
      84,    86,   562,   563,   564,    84,    86,    84,    86,    84,
      86,    84,    86,    84,    86,     5,   560,    86,    86,   195,
     197,   196,     0,    61,    60,     0,    59,     0,     0,     0,
       0,   194,   199,     0,     0,     0,   158,   154,   154,   154,
       0,     0,     0,     0,     0,     0,     0,   183,   182,   181,
     184,     0,     0,     0,     0,    30,    29,    32,     0,     0,
       0,    51,    50,    49,    48,     0,     0,    82,     0,   201,
      76,     0,     0,   227,   268,   273,   282,   279,   235,   286,
     276,     0,     0,   271,   284,   314,   274,   275,     0,     0,
       0,     0,   284,     0,   277,   278,   283,   313,   312,     0,
     311,   203,    85,     0,     0,    62,    67,   112,   357,   398,
     498,   504,   535,     0,     0,     0,     0,     0,    16,   116,
       0,     0,     0,     0,   604,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    16,
       0,     0,     0,     0,     0,     0,   605,     0,     0,     0,
      16,     0,    16,   606,   401,     0,     0,   406,     0,     0,
       0,   607,     0,     0,   533,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   608,     0,     0,   137,   141,     0,
       0,     0,   609,     0,     0,     0,     0,    16,     0,    16,
       0,   602,     0,     0,     0,     0,    26,   603,     0,     0,
     119,   118,     0,     0,     0,    57,     0,     0,   189,   130,
     129,     0,     0,   198,   200,   199,     0,   155,     0,   150,
     149,     0,   153,     0,   154,   154,   154,   154,   154,     0,
       0,   178,   179,   175,   173,   174,   176,   177,   185,     0,
     186,   187,   188,     0,     0,    31,    33,     0,     0,   101,
       6,     8,   284,   102,    52,     0,    56,    72,    73,    74,
       0,   216,     0,    47,     0,     0,   268,   286,   268,   284,
     268,   268,     0,   260,     0,   284,     0,     0,   271,   271,
       0,   309,   284,   284,   284,   284,     0,   239,   237,   284,
     236,   238,   246,   248,     0,   251,   255,     0,   620,   617,
     618,   287,   619,   288,   289,   290,   291,   292,   293,   294,
     622,   625,   624,   621,   623,   626,   627,   628,   629,   630,
     631,   632,   633,   634,   635,   636,   315,   316,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,     0,   610,   614,   113,    89,   100,    93,     0,    87,
      12,    14,    13,    15,    11,    88,   103,    98,   216,    99,
     284,    91,    97,    92,    95,    90,   114,   115,   358,     0,
       0,     0,     0,     0,     0,     0,    16,     0,    17,    18,
       0,     0,     0,     0,     0,     0,   370,     0,     0,     0,
       0,     0,    16,     0,     0,     0,     0,    16,     0,    16,
       0,    16,     0,    16,     0,     0,    16,   359,   399,   407,
     402,   405,   403,   400,   409,   499,     0,     0,     0,   500,
     505,   103,     0,   142,     0,   138,   520,   518,   519,     0,
       0,     0,     0,     0,     0,     0,     0,   506,   513,   143,
     536,     0,   540,    16,   537,   549,     0,   548,   550,     0,
     539,     0,    63,     0,    65,    16,   612,   613,   611,     0,
      25,     0,   616,   615,   124,   123,   125,   127,   128,   126,
       0,   121,   192,    58,   190,   191,   134,   135,     0,   132,
     193,     0,   151,   156,   157,   164,     0,     0,   171,   216,
     165,   168,   169,     0,   170,   159,     0,   163,   172,   489,
     489,   489,   154,   489,   154,   489,   490,   551,   180,     0,
       0,    28,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,    35,    27,     0,     0,    54,    55,    71,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,     0,
       0,     0,   202,   220,    80,    81,     0,     0,    75,    77,
      46,     0,   216,   264,   265,     0,   266,   267,   262,   250,
     338,   341,     0,   322,   323,   269,   270,   263,     0,     0,
       0,     0,   295,   284,   273,   286,     0,   284,     0,     0,
       0,   280,   252,   253,     0,     0,   281,   307,   305,   303,
     300,   301,   302,   304,   306,   308,   296,   297,   298,   299,
     317,     0,   318,   320,   321,    94,   107,   110,   117,   221,
     222,    96,   223,     0,     0,   393,   395,     0,    16,    16,
       0,     0,     0,     0,     0,     0,     0,     0,   371,     0,
       0,     0,     0,     0,     0,   364,     0,   365,     0,   391,
       0,     0,     0,     0,     0,     0,     0,   376,   377,     0,
       0,     0,     0,   502,   503,   531,   517,   508,     0,     0,
     512,   507,   509,     0,     0,   515,   516,   514,     0,     0,
       0,     0,     0,     0,    19,     0,   546,   545,     0,   543,
     547,    70,    64,    66,    68,    21,    20,    69,   120,     0,
     131,     0,   152,   161,   167,   162,   160,   166,   488,     0,
       0,     0,   489,     0,   489,     0,    10,     9,    34,     0,
       7,    53,   215,   214,   216,   284,    78,    79,   225,   218,
       0,   228,   261,   284,   327,   310,   284,   284,   284,     0,
       0,     0,     0,     0,   235,   249,     0,     0,   256,   284,
     108,     0,   103,     0,     0,   394,     0,     0,   390,   384,
     360,     0,     0,   366,     0,   369,     0,   372,   383,   382,
     378,     0,     0,   392,   381,   373,   374,   375,   396,   397,
     380,   379,   404,   482,    16,     0,     0,     0,   408,     0,
     416,   426,   501,   529,   534,   136,   138,   510,   511,     0,
       0,     0,     0,     0,   140,    22,    23,   538,   541,     0,
     544,   122,   133,   492,   491,   497,     0,   493,     0,   495,
      36,   217,     0,   331,     0,   332,   333,     0,   284,   329,
     330,   328,   284,   284,     0,   284,     0,     0,   284,   231,
     224,     0,   339,     0,     0,     0,   272,     0,     0,   295,
       0,     0,   247,     0,     0,   104,   105,   109,   111,   386,
       0,   388,   389,   361,   362,   368,   367,   385,   363,   103,
     415,     0,   412,   470,     0,   419,   414,   418,   410,   427,
     526,   139,   143,    24,     0,     0,     0,     0,   542,   494,
     496,   284,   284,   284,   284,     0,     0,     0,   226,   232,
       0,     0,   351,   334,   336,     0,   219,   325,   326,   324,
     243,   245,   244,     0,   240,     0,   319,   106,     0,   485,
     411,     0,   440,   420,   429,     0,     0,   532,     0,   148,
     143,   143,   143,   143,     0,     0,     0,     0,   345,   284,
     257,   284,   284,     0,   284,   350,     0,     0,   254,   387,
     483,   487,   413,     0,   471,   472,   473,     0,     0,     0,
       0,     0,     0,     0,   425,   439,     0,   527,   521,   522,
     523,   524,   525,   528,   530,   144,   145,   146,   147,   344,
     285,   348,     0,   284,   342,   233,     0,     0,     0,     0,
       0,     0,     0,   337,     0,     0,     0,     0,   474,    16,
       0,    16,   417,     0,   433,   431,   430,     0,     0,   434,
     436,   437,   438,   428,     0,     0,   284,   284,   352,     0,
     347,   346,   284,   284,   284,   353,   241,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   486,   469,     0,   475,   423,   424,
     422,   421,   432,   435,   349,   234,   343,   259,   284,     0,
       0,     0,     0,   476,   445,   459,   441,   444,   442,     0,
     457,     0,   446,   448,   449,   450,   451,   452,   453,   454,
     455,   463,   464,   465,    16,   443,    16,     0,     0,   484,
     258,     0,     0,     0,     0,     0,   456,   447,   458,   460,
     466,   467,   468,   461,   355,   356,   354,     0,     0,   462,
     477,   478,   479,   242,     0,   480,   481
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   444,    69,   445,   531,   776,   777,   764,
     964,   551,    70,   335,   138,   338,   611,   612,    71,   145,
     146,    72,    16,    17,    18,   105,    73,   282,    74,   288,
     283,    75,   350,    76,   352,   639,    77,    78,   183,   205,
     451,   437,    79,   696,   937,   831,   697,   698,    80,    81,
     446,   206,   293,   560,   561,   302,   568,   569,   266,   748,
     267,   268,   761,    19,    20,    21,    22,   313,    23,    24,
      25,   311,   586,    26,    27,    28,    29,    30,    31,    32,
      33,    34,   130,   329,    35,    36,    37,    38,    39,    40,
      41,    42,    43,   102,   306,    82,   351,    83,   629,   630,
     631,   810,   700,   701,   452,   642,   633,    84,   355,   914,
     915,  1064,   174,   382,   383,   384,   175,   386,   387,  1066,
     361,   176,   362,   370,   177,   178,   179,   917,   180,   416,
     650,   918,   984,   985,   651,   652,   919,    85,    86,    87,
      88,   245,   246,   247,   741,   869,   956,   957,  1037,  1083,
     870,  1004,  1046,   871,  1126,  1001,  1034,  1035,  1036,  1175,
    1190,  1191,  1192,   949,  1076,  1031,   950,   789,    44,    45,
      46,    47,    48,    49,    50,    51,    89,    90,    91,    92,
     519,  1053,  1008,   960,   874,   511,   512,    93,    94,   274,
     769,   770,   275,    52,    95,    53,    96,    97,    98,   417
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -941
static const yytype_int16 yypact[] =
{
     166,  1710,   212,   513,   223,    56,   187,   621,   223,   307,
     648,   711,   653,   223,   223,   223,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,   275,   277,   224,   533,
     299,   251,  -941,   330,   226,   102,   363,   800,  -941,  -941,
    -941,  -941,  -941,   377,   377,  -941,  -941,  -941,   531,  -941,
     377,  1646,  -941,  -941,  -941,   377,  1465,   377,  1340,   377,
     899,   377,  1349,   377,  1632,  -941,  -941,  1634,  1511,  -941,
    -941,  -941,    44,  -941,   401,   422,  -941,   223,   223,   428,
      46,  -941,   563,   122,   445,   449,  -941,   415,    63,   415,
     460,   464,   471,   474,   477,   482,   505,  -941,  -941,  -941,
     503,   535,   537,   540,   542,  -941,   496,  -941,    23,    44,
      84,  -941,  -941,  -941,  -941,    60,   269,  -941,   240,  -941,
    -941,   280,    46,  -941,    -1,  -941,  -941,  -941,   556,   293,
    -941,   420,   427,     1,   800,  -941,  -941,  -941,   331,   350,
     353,   365,   800,    24,  -941,  -941,  -941,  -941,  -941,  1242,
    -941,  1831,  -941,   385,   388,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,   411,    44,   673,   654,    84,    74,  -941,
     370,   370,   701,    46,  -941,   436,   438,   461,   211,   733,
      74,   553,   573,   -65,   668,   111,   452,   715,   599,   720,
       2,   606,   609,   352,   608,   605,   432,   593,   590,    74,
     591,   592,   585,   -31,    21,   620,  -941,   512,   520,   240,
      74,    84,    74,  -941,  -941,   515,   521,  -941,   523,   755,
     300,  -941,   529,   536,  -941,   810,    84,   538,    -7,    84,
      84,   773,   775,  -121,  -941,   550,   551,  -941,   552,   559,
     452,   589,  -941,   555,   252,   558,   799,    74,   816,    74,
     807,  -941,   584,   597,   600,   821,   870,  -941,   601,   602,
    -941,  -941,   522,   894,   900,  -941,   905,   907,  -941,  -941,
    -941,    78,   908,  -941,  -941,   634,   911,  -941,   913,  -941,
    -941,   840,  -941,   914,   415,   415,   415,    75,   123,   917,
     932,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,   934,
    -941,  -941,  -941,    33,   883,  -941,  -941,  1081,   640,  -941,
    -941,  -941,   800,  -941,    84,   623,  -941,  -941,  -941,  -941,
     642,  1247,    51,  -941,   650,   657,    -1,  -941,    -1,   800,
      -1,    -1,   936,  -941,   931,   543,   933,   930,     1,     1,
     937,   929,   800,   800,   800,   800,   109,  -941,  -941,   729,
    -941,  -941,   940,  -941,   -58,    -3,  -941,   -24,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   716,  -941,  -941,  -941,  -941,  -941,  -941,   240,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,   724,  -941,
     800,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,   906,
     912,   677,   680,    84,   941,   767,    74,   452,  -941,  -941,
      84,   811,   812,    84,   928,    84,  -941,   703,   943,   820,
      84,    84,    74,   945,   704,   938,   712,    74,   723,    74,
     290,    74,   673,    74,   725,   734,    74,  -941,  -941,  -941,
    -941,  -941,   803,  -941,  -941,  -941,  1000,   736,   740,  -941,
    -941,  -941,   741,  -941,   750,   939,  -941,  -941,  -941,   751,
     753,   759,    84,    84,   761,   763,   772,  -941,  -941,   214,
    -941,   -10,  -941,    74,  -941,  -941,   596,  -941,  -941,   848,
    -941,    84,  -941,    84,  -941,    74,  -941,  -941,  -941,    84,
    -941,   452,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
     395,  -941,  -941,  -941,  -941,  -941,  -941,  -941,   402,  -941,
    -941,  1070,  -941,  -941,  -941,  -941,  1056,  1057,  -941,   724,
    -941,  -941,  -941,   452,  -941,  -941,  1059,  -941,  -941,  1068,
    1068,  1068,   415,  1068,   415,  1068,  -941,  -941,  -941,   778,
     790,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,   407,  -941,  -941,  1084,  1071,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,    71,
     801,   797,  -941,  -941,  -941,  -941,   452,    84,  -941,  -941,
    -941,   351,  1247,  -941,  -941,  1109,  -941,  -941,  -941,  -941,
     242,  -941,   804,  -941,  -941,  -941,  -941,  -941,  1134,   391,
     433,   504,  -941,   800,   796,    70,   798,   800,   836,  1072,
     132,  -941,  -941,  -941,  1088,  1096,  -941,  1842,  1842,  1842,
    1872,  1872,  1842,  1842,  1842,  1842,   506,   506,   929,   929,
    -941,   818,  -941,  -941,  -941,  -941,  1113,  -941,   844,  -941,
    -941,  -941,  1831,    84,    84,  -941,  -941,   849,    74,    74,
     850,   851,   852,    84,    84,   853,    84,   854,  -941,    84,
     855,   856,   865,   866,    84,  -941,   240,  -941,   873,  -941,
     876,   877,   878,   879,   880,   881,   890,  -941,  -941,   891,
      84,   320,   898,  -941,  -941,   -26,  -941,  -941,   896,   901,
    -941,  -941,  -941,   903,   904,  -941,  -941,  -941,    84,   305,
     324,   897,    84,   452,  -941,   915,  -941,  -941,   452,   935,
     848,  -941,  -941,  -941,  -941,   -10,  -941,  -941,  -941,   522,
    -941,    78,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  1200,
    1211,  1218,  1068,  1219,  1068,  1220,  -941,  -941,  -941,  1081,
    -941,  -941,  -941,  -941,  1247,    93,  -941,  -941,  -941,  -941,
     442,  -941,  -941,   800,  -941,  -941,   800,   800,   800,  1159,
      72,   208,    86,   186,  -941,  -941,   922,   556,  -941,   800,
    -941,   607,  -941,   927,   924,  -941,   948,   950,  -941,  -941,
    -941,   951,   952,  -941,   953,  -941,   954,  -941,  -941,  -941,
    -941,   955,   956,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,    35,   963,   967,   452,  -941,   968,
    -941,   964,  -941,  -941,  -941,  -941,   939,  -941,  -941,   969,
    1208,  1208,  1208,  1208,  -941,  -941,  -941,  -941,  -941,    84,
     935,  -941,  -941,  -941,  -941,  -941,  1226,  -941,  1235,  -941,
    -941,  -941,    25,  -941,  1248,  -941,  -941,   980,   800,  -941,
    -941,  -941,   800,   800,   976,    42,  1242,   970,   617,  -941,
    -941,  1247,  -941,  1184,  1209,  1234,  -941,   942,   981,   977,
     983,    30,  -941,  1267,  1264,   452,  -941,  -941,  -941,  -941,
      84,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,    84,  -941,  -941,   240,  -941,  -941,   982,  -941,  -941,
    -107,  -941,   214,  -941,   985,   986,   995,   996,  -941,  -941,
    -941,   800,   543,   800,  -941,   691,  1778,   647,  -941,  -941,
    1275,   190,  -941,  1831,   997,   999,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  1270,  1271,  1002,  -941,  -941,  1011,   121,
    -941,    15,  -941,  -941,   649,    84,   154,  -941,  1014,  -941,
     214,   214,   214,   214,   764,  1001,   972,  1295,  -941,   389,
    -941,   800,   800,   430,   617,  -941,    84,  1282,  -941,  -941,
    -941,  -941,  -941,  -155,  -941,  -941,  -941,   129,    84,    84,
      84,  1123,   179,    81,  -941,  -941,  1017,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  1030,    93,  1167,  -941,    13,  1019,  1058,  1038,
    1039,  1040,  1041,  -941,  1043,  1042,  1522,  1044,  -941,    74,
      84,    74,  -941,  1053,  -941,  -941,  -941,    84,    84,  -941,
    -941,  -941,  -941,  -941,  1054,  1036,   389,    49,  -941,  1055,
    -941,  -941,   800,   800,   800,  -941,  -941,    84,  1065,    84,
      74,    84,    84,    84,  1150,    84,   276,   679,   178,  1112,
      84,  1105,  1104,  1099,  -941,  -941,  1073,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,    49,  1297,
    1322,  1351,  1067,  -941,  -941,  -941,  -941,  -941,  -941,    84,
    -941,    84,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,    74,  -941,    74,    77,    84,  -941,
    -941,  1106,  1107,  1111,    84,    17,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  1097,  -111,  -941,
    -941,  -941,  -941,  -941,  1114,  -941,  -941
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -941,  -941,  -941,   -96,  -941,  -205,  -212,  -268,  -571,  -941,
    -218,  -941,  -941,  -941,  -941,  -941,  -941,   571,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  1293,  -941,  -941,  -941,  -941,
    -941,  -941,  -233,  -941,  -941,  -941,  -941,  -941,  1395,   966,
    1202,   919,  -941,  -941,  -941,  -941,   583,  -503,  -941,  -941,
    -941,  -941,    26,  -941,   641,   -76,  -941,   643,  -920,   545,
    -501,  -941,  -511,  -941,  -941,  -941,  -941,  -108,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,   458,  1124,  -941,  -941,  -941,  -941,  -620,
     624,  -941,  1079,   857,  1122,  -941,   792,  -941,  -941,  -940,
    -941,   339,  -172,   625,  -661,  -941,  -166,   771,  -941,  -941,
    -941,  -941,   323,   362,  -941,  -941,  -760,  -941,    32,   534,
     -67,  -941,  -941,   423,   638,   483,  -720,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -112,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
    -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,
     687,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -941,  -891
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -341
static const yytype_int16 yytable[] =
{
     181,   381,   532,   471,   472,   462,   499,   385,   745,   825,
     319,   320,   786,   356,   749,   368,   340,   357,   762,  1097,
     341,   809,   672,   673,   488,   981,   358,   340,   377,   378,
     824,   341,   468,   469,  1188,   500,   492,   502,   340,   377,
     378,   158,   341,   585,   343,   916,   971,   340,  -230,   340,
     599,   341,   465,   341,   524,  -229,  -230,  -230,  -230,  -230,
     902,   993,   336,  -229,  -229,  -229,  -229,   902,   440,   441,
     442,   443,   542,   516,   544,   103,   354,   380,   290,   344,
     299,   312,  1005,   291,  1045,   300,   340,   903,   340,   802,
     341,   340,   341,   312,   903,   341,   494,   371,   340,   927,
     340,   439,   341,  1077,   341,   376,   104,   440,   441,   442,
     443,   902,   345,   930,   592,   904,   418,   419,   420,   421,
     422,   151,   904,  1095,   477,   307,  1062,   455,   468,   469,
     423,   424,   425,   426,   427,   428,   429,   430,   903,   314,
     431,   312,   466,  1078,  1194,   501,   340,   377,   378,   824,
     341,   634,   635,   636,   637,   916,  1125,  1137,   493,   566,
     514,  1090,   594,   520,   521,   339,   904,  1006,   525,   526,
       1,   905,   906,   907,   908,   909,   910,   911,   905,   906,
     907,   908,   909,   910,   911,   315,   363,  1195,   567,   912,
     106,   152,   913,   340,  1007,   979,   912,   341,  1170,   913,
     340,   377,   378,   824,   341,   695,   589,   590,   591,   593,
     595,  1022,    54,   803,  1017,   418,   419,   420,   421,   422,
     435,  1023,   905,   906,   907,   908,   909,   910,   911,   423,
     424,   425,   426,   427,   428,   429,   430,   600,   308,   431,
     912,    99,   100,   913,   670,  1079,   495,   671,   615,   418,
     419,   420,   421,   422,   347,   711,   348,   349,  1161,   916,
     936,   710,   101,   423,   424,   425,   426,   427,   428,   429,
     430,   951,  1088,   431,   873,   614,   832,   723,   675,   517,
     518,   676,   728,   668,   730,   758,   734,   470,   736,   763,
     340,   739,   645,   134,   341,   359,   135,   674,   342,  1065,
     476,   986,   360,   916,   369,   658,   659,   660,   661,   379,
     111,   759,   614,   139,  1098,   136,  1032,   147,  1189,   140,
     379,   972,   148,   337,   767,   150,   137,   994,   765,   342,
     316,   342,  1091,   317,   318,  1092,   916,   916,  -230,   775,
     773,  1180,  1181,  -230,   292,  -229,   301,   760,   149,   638,
    -229,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   997,  1080,   365,   707,   342,  1151,
     342,   775,    -8,   342,   712,   749,  1065,   715,   916,   717,
     342,   153,   342,   702,   721,   722,   535,  1081,   363,   880,
     363,   881,   363,   363,  -229,   182,   863,  1048,   418,   419,
     420,   421,   422,   459,   460,  1099,   662,   902,   882,   447,
     883,   663,   423,   424,   425,   426,   427,   428,   429,   430,
     103,  1030,   431,   832,   806,   295,   753,   754,   379,  1162,
    1082,   298,  1163,   312,   903,   536,  1049,  1050,  1051,  1052,
     418,   419,   420,   421,   422,   771,   999,   772,   309,   448,
     449,  1009,   310,   774,   423,   424,   425,   426,   427,   428,
     429,   430,   904,   321,   431,   109,   110,   322,   537,   468,
     469,   131,   132,   133,   323,   342,   731,   324,   790,   791,
     325,   793,   342,   795,   792,   326,   794,   619,   620,   621,
     622,   623,   624,   852,   625,   626,   627,   628,   381,  1055,
    1056,  1057,  1058,   836,   837,   929,   732,   733,   327,   385,
     663,   418,   419,   420,   421,   422,    55,   328,   905,   906,
     907,   908,   909,   910,   911,   423,   424,   425,   426,   427,
     428,   429,   430,   429,   430,   431,   912,   431,   330,   913,
     331,   807,   450,   332,   813,    56,   538,   539,   480,   481,
      57,   886,   334,    58,   864,   154,   888,   155,   156,   157,
     158,   159,   160,   333,    59,   296,   297,   346,   161,   162,
     141,   163,   342,   164,   380,   507,   508,   865,   353,   866,
      60,   303,   304,    61,   364,    62,   867,   554,   142,   365,
     185,   366,    63,  1069,  1070,   186,   819,  1071,   367,   955,
     821,  1072,   305,   555,   556,   143,   187,   833,   834,   557,
     558,    64,   559,   468,   469,   766,   165,   841,   842,   775,
     844,   868,    65,   846,   468,   469,   935,   372,   851,   154,
     188,   155,   156,   157,   158,   159,   160,   484,   485,    99,
     100,    66,   161,   162,   862,   163,   373,   164,   808,   374,
     144,   381,   303,   304,   418,   419,   420,   421,   422,   952,
     101,   375,   879,   965,   966,   967,   885,   127,   423,   424,
     425,   426,   427,   428,   429,   430,   616,   617,   431,   643,
     896,   644,   898,   646,   647,   432,    67,   128,   433,  1063,
     165,   436,   129,   816,   166,   167,   778,   779,   418,   419,
     420,   421,   422,   780,   781,   168,   169,   170,   798,   799,
     171,   434,   423,   424,   425,   426,   427,   428,   429,   430,
     454,  1002,   431,   775,   928,   438,   931,   380,   107,   108,
     655,   656,   255,  1033,   456,   817,   457,   112,   113,   920,
     921,   154,   699,   664,   156,   157,   158,   665,   160,   923,
     924,   925,   461,   257,   161,   162,   666,   163,   189,   164,
     114,   458,   934,   115,   463,   116,   117,   464,   166,   167,
     467,   418,   419,   420,   421,   422,   120,   473,   474,   168,
     169,   170,   475,   478,   171,   423,   424,   425,   426,   427,
     428,   429,   430,   968,   479,   431,   483,   482,   486,   487,
     121,   122,   165,   190,   489,   491,   818,   490,   191,   496,
     497,    68,   154,   503,   155,   156,   157,   158,   159,   160,
     498,   504,   506,   505,   192,   161,   162,   509,   163,   513,
     164,   123,   124,   125,   126,   522,   510,   523,   515,   172,
    -340,   975,  1038,   574,   998,   976,   977,   173,   527,   528,
     533,   983,   529,   534,   690,  1000,   540,   468,   469,   530,
     619,   620,   621,   622,   623,   624,   541,   625,   626,   627,
     628,   691,   545,   165,  1128,   118,  1130,   692,   543,   575,
     166,   167,   546,   549,   693,   694,  1039,  1040,  1041,   550,
    1042,   168,   169,   170,   381,   547,   171,   562,   548,   552,
     553,  1043,   601,   563,  1014,  1145,  1016,   576,   564,  1047,
     565,   570,   577,   172,   572,  -335,   573,   588,   578,   579,
     596,   173,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1074,   580,   581,   582,   583,   597,   584,   598,   613,   194,
     618,   119,  1084,  1085,  1086,   649,  1089,  1020,   640,   648,
    1044,   166,   167,   641,  1067,  1068,   654,   983,   653,  1178,
     431,  1179,   168,   169,   170,   657,   195,   171,   703,   196,
     380,   669,   197,   198,   704,   705,   200,   201,   706,   418,
     419,   420,   421,   422,  1129,   709,   708,   713,   714,  1018,
     716,  1132,  1133,   423,   424,   425,   426,   427,   428,   429,
     430,   718,   725,   431,   202,   719,   720,   724,   203,   726,
     727,  1142,   450,  1144,   740,  1146,  1147,  1148,   742,  1150,
    1152,   729,   255,   737,  1165,   667,   418,   419,   420,   421,
     422,   768,   738,   173,   743,  1139,  1140,  1141,   744,   746,
     423,   424,   425,   426,   427,   428,   429,   430,   747,   750,
     431,   751,   237,  1176,   244,  1177,   252,   752,   265,   755,
     273,   756,  1059,   284,   289,   418,   419,   420,   421,   422,
     757,  1182,  1183,   782,   783,   784,   796,   787,  1187,   423,
     424,   425,   426,   427,   428,   429,   430,   788,   797,   431,
     801,   418,   419,   420,   421,   422,   172,   805,    -6,   804,
     820,   814,   826,   823,   173,   423,   424,   425,   426,   427,
     428,   429,   430,   827,   829,   431,   418,   419,   420,   421,
     422,   602,   603,   604,   605,   606,   607,   608,   609,   610,
     423,   424,   425,   426,   427,   428,   429,   430,   822,   830,
     431,   418,   419,   420,   421,   422,   832,   835,   838,   839,
     840,   843,   845,   847,   848,   423,   424,   425,   426,   427,
     428,   429,   430,   849,   850,   431,   418,   419,   420,   421,
     422,   853,   249,   250,   854,   855,   856,   857,   858,   859,
     423,   424,   425,   426,   427,   428,   429,   430,   860,   861,
     431,   418,   419,   420,   421,   422,   872,   875,   884,   876,
     251,   877,   878,   893,   889,   423,   424,   425,   426,   427,
     428,   429,   430,   887,   894,   431,   418,   419,   420,   421,
     422,   895,   897,   899,   933,   939,   940,   963,   954,   969,
     423,   424,   425,   426,   427,   428,   429,   430,   970,   990,
     431,   418,   419,   420,   421,   422,   941,   388,   942,   943,
     944,   945,   946,   947,   948,   423,   424,   425,   426,   427,
     428,   429,   430,   953,   959,   431,   958,   962,   982,   973,
    1061,   418,   419,   420,   421,   422,   974,   978,   991,    -7,
     992,   995,  1003,  1010,  1011,   423,   424,   425,   426,   427,
     428,   429,   430,  1012,  1013,   431,  1021,  1025,  1060,  1024,
     388,  1026,  1027,  1028,   418,   419,   420,   421,   422,  1029,
     389,   390,  1054,  1075,  1087,  1093,  1096,  1100,   423,   424,
     425,   426,   427,   428,   429,   430,   391,  1094,   431,   418,
     419,   420,   421,   422,  1102,  1103,  1104,  1135,  1107,  1105,
    1106,  1149,  1127,   423,   424,   425,   426,   427,   428,   429,
     430,  1131,  1134,   431,  1138,   392,  1101,  1164,   418,   419,
     420,   421,   422,   389,   390,  1143,  1166,  1167,  1168,  1174,
     900,  1169,   423,   424,   425,   426,   427,   428,   429,   430,
     194,   800,   431,   619,   620,   621,   622,   623,   624,   194,
     625,   626,   627,   628,  1193,   393,   394,   294,   395,   396,
     397,   398,   399,   453,  1184,  1185,   812,   195,   392,  1186,
     196,   735,  1196,   197,   198,   938,   195,   200,   201,   196,
     891,   961,   197,   198,   892,   254,   200,   201,   901,   571,
     632,   815,   255,   587,   811,  1136,   785,   400,   401,   402,
     403,   404,   405,   406,   407,   202,   828,  1073,   932,   203,
     980,   922,   256,   257,   202,  1015,   926,   890,   203,     0,
       0,     0,     0,   408,   409,     0,     0,     0,     0,   184,
       0,   410,   411,   412,   413,   193,     0,     0,     0,     0,
     207,   987,   238,     0,   248,     0,   253,     0,   269,     0,
     400,   401,   402,   403,   404,   405,   406,   407,     0,     0,
       0,     0,     0,     0,     0,   194,   988,     0,     0,     0,
     208,     0,     0,     0,     0,     0,   408,   409,     0,     0,
       0,     0,     0,     0,   410,   411,   412,   413,     0,     0,
       0,   989,   195,   414,   415,   196,     0,     0,   197,   198,
       0,     0,   200,   201,     0,     0,     0,     0,   209,     0,
     210,   194,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   996,     0,     0,     0,     0,     0,     0,   239,     0,
     202,     0,   285,   240,   203,   241,     0,     0,   195,     0,
       0,   196,   211,     0,   197,   198,   414,   415,   200,   201,
     212,   213,     0,     0,  1171,     0,     0,     0,     0,     0,
       0,   258,   286,     0,     0,   255,     0,     0,   242,     0,
       0,     0,     0,     0,     0,     0,   202,     0,     0,  1172,
     203,     0,   214,     0,     0,     0,   257,   259,   260,   261,
     262,     0,     0,     0,     0,     0,     0,   263,     0,   215,
     216,   243,   217,   218,  1108,   219,   220,   221,  1173,   222,
     264,     0,   223,   224,     0,   225,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,   227,
       0,     0,   194,   228,   194,   229,     0,   230,     0,   231,
       0,     0,     0,     0,   232,     0,   194,     0,   233,   234,
       0,   235,     0,   276,   277,   278,     0,   279,   280,   195,
       0,   195,   196,     0,   196,   197,   198,   197,   198,   200,
     201,   200,   201,   195,     0,  1109,   196,     0,     0,   197,
     198,     0,   199,   200,   201,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   202,     0,   202,
       0,   203,     0,   203,     0,     0,     0,     0,     0,   270,
       4,   202,     0,     0,     0,   203,  1110,     0,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,     0,   236,     5,     6,     0,
       0,     0,     0,     0,  1118,  1119,     0,     0,  1120,     0,
       0,     0,  1121,     0,  1122,   418,   419,   420,   421,   422,
    1123,     0,     0,     0,     0,     0,     0,     0,     0,   423,
     424,   425,   426,   427,   428,   429,   430,     0,     0,   431,
       0,     0,   287,     0,     0,     0,     7,     0,     0,     8,
       0,     9,     0,  1124,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,    13,    14,    15,   418,   419,
     420,   421,   422,     0,     0,     0,     0,     0,     0,  -341,
    -341,  -341,   423,   424,   425,   426,   427,   428,   429,   430,
       0,     0,   431,  -341,  -341,  -341,  -341,   427,   428,   429,
     430,     0,     0,   431,     0,     0,     0,     0,     0,   418,
     419,   420,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   271,   423,   424,   425,   426,   427,   428,   429,
     430,     0,     0,   431,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1019,     0,     0,     0,
       0,     0,     0,   272,     0,   281,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   204
};

static const yytype_int16 yycheck[] =
{
      67,   173,   270,   215,   216,   210,   239,   173,   511,   670,
     118,   119,   583,    14,   515,    14,    14,    18,    28,     6,
      18,   641,    25,    26,   229,   916,    27,    14,    15,    16,
      17,    18,    17,    18,    17,   240,    67,   242,    14,    15,
      16,    17,    18,   311,   140,   805,    21,    14,     6,    14,
      17,    18,   117,    18,   175,     6,    14,    15,    16,    17,
      18,    31,    39,    14,    15,    16,    17,    18,    33,    34,
      35,    36,   277,    80,   279,    19,   152,   173,    34,    19,
      34,    18,   189,    39,  1004,    39,    14,    45,    14,    18,
      18,    14,    18,    18,    45,    18,    75,   164,    14,    27,
      14,   197,    18,   258,    18,   172,    50,    33,    34,    35,
      36,    18,    52,    27,    39,    73,     7,     8,     9,    10,
      11,    19,    73,  1063,   220,     3,  1017,   203,    17,    18,
      21,    22,    23,    24,    25,    26,    27,    28,    45,    76,
      31,    18,   207,   298,   255,   241,    14,    15,    16,    17,
      18,   100,   101,   102,   103,   915,  1076,  1097,   189,    81,
     256,    80,    39,   259,   260,   139,    73,   274,   289,   290,
       4,   129,   130,   131,   132,   133,   134,   135,   129,   130,
     131,   132,   133,   134,   135,   122,   154,   298,   110,   147,
       3,    89,   150,    14,   301,   915,   147,    18,  1138,   150,
      14,    15,    16,    17,    18,   438,   314,   315,   316,   317,
     318,    21,     0,   142,   974,     7,     8,     9,    10,    11,
     194,    31,   129,   130,   131,   132,   133,   134,   135,    21,
      22,    23,    24,    25,    26,    27,    28,   333,   116,    31,
     147,    18,    19,   150,   302,   116,   225,   305,   344,     7,
       8,     9,    10,    11,    14,   467,    16,    17,    80,  1019,
     831,   466,    39,    21,    22,    23,    24,    25,    26,    27,
      28,   236,    93,    31,   300,   342,   302,   482,   302,   286,
     287,   305,   487,   379,   489,    71,   491,   176,   493,   299,
      14,   496,   359,    18,    18,   296,    19,   300,   296,  1019,
     298,   921,   303,  1063,   303,   372,   373,   374,   375,   296,
       3,    97,   379,    89,   301,    38,   301,    18,   301,    95,
     296,   296,    71,   300,   536,    99,    49,   297,   533,   296,
     267,   296,   251,   270,   271,   254,  1096,  1097,   296,   551,
     545,   264,   265,   301,   300,   296,   300,   133,    18,   298,
     301,   418,   419,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   935,   236,   296,   463,   296,    93,
     296,   583,   302,   296,   470,   876,  1096,   473,  1138,   475,
     296,    18,   296,   450,   480,   481,   134,   258,   356,    84,
     358,    86,   360,   361,   301,    18,    76,   243,     7,     8,
       9,    10,    11,   192,   193,  1066,   297,    18,    84,    39,
      86,   302,    21,    22,    23,    24,    25,    26,    27,    28,
      19,   300,    31,   302,   636,     3,   522,   523,   296,   251,
     301,     3,   254,    18,    45,   183,   282,   283,   284,   285,
       7,     8,     9,    10,    11,   541,   949,   543,     3,    79,
      80,   962,     3,   549,    21,    22,    23,    24,    25,    26,
      27,    28,    73,     3,    31,     7,     8,     3,   216,    17,
      18,    13,    14,    15,     3,   296,   186,     3,   590,   591,
       3,   593,   296,   595,   592,     3,   594,   136,   137,   138,
     139,   140,   141,   726,   143,   144,   145,   146,   670,  1010,
    1011,  1012,  1013,   708,   709,   297,   216,   217,     3,   675,
     302,     7,     8,     9,    10,    11,     3,    14,   129,   130,
     131,   132,   133,   134,   135,    21,    22,    23,    24,    25,
      26,    27,    28,    27,    28,    31,   147,    31,     3,   150,
       3,   637,   172,     3,   302,    32,   294,   295,   196,   197,
      37,   763,    56,    40,   234,    12,   768,    14,    15,    16,
      17,    18,    19,    21,    51,   107,   108,   298,    25,    26,
      37,    28,   296,    30,   670,   275,   276,   257,   298,   259,
      67,    18,    19,    70,    28,    72,   266,    65,    55,   296,
      59,   171,    79,   163,   164,    64,   663,   167,   171,   867,
     667,   171,    39,    81,    82,    72,    75,   703,   704,    87,
      88,    98,    90,    17,    18,    19,    73,   713,   714,   831,
     716,   301,   109,   719,    17,    18,    19,   296,   724,    12,
      99,    14,    15,    16,    17,    18,    19,   205,   206,    18,
      19,   128,    25,    26,   740,    28,   296,    30,   297,   296,
     117,   823,    18,    19,     7,     8,     9,    10,    11,   864,
      39,   296,   758,   881,   882,   883,   762,    14,    21,    22,
      23,    24,    25,    26,    27,    28,    53,    54,    31,   356,
     792,   358,   794,   360,   361,   300,   173,    34,   300,   300,
      73,    18,    39,   302,   151,   152,   301,   302,     7,     8,
       9,    10,    11,   301,   302,   162,   163,   164,   301,   302,
     167,   300,    21,    22,    23,    24,    25,    26,    27,    28,
      19,   954,    31,   935,   820,    71,   822,   823,   107,   108,
     368,   369,    83,  1001,   298,   302,   298,    89,    90,   297,
     298,    12,    18,    14,    15,    16,    17,    18,    19,   816,
     817,   818,    19,   104,    25,    26,    27,    28,   227,    30,
     112,   300,   829,   115,   211,   117,   118,   194,   151,   152,
     102,     7,     8,     9,    10,    11,    65,    62,   179,   162,
     163,   164,    62,   177,   167,    21,    22,    23,    24,    25,
      26,    27,    28,   889,   185,    31,   191,   189,   205,   209,
      89,    90,    73,   272,   213,   220,   302,   215,   277,   189,
     298,   298,    12,   298,    14,    15,    16,    17,    18,    19,
     300,   300,    67,   300,   293,    25,    26,   298,    28,    19,
      30,   120,   121,   122,   123,    62,   300,    62,   300,   296,
     297,   908,   193,     3,   940,   912,   913,   304,   298,   298,
     261,   918,   300,   298,   138,   951,   298,    17,    18,   300,
     136,   137,   138,   139,   140,   141,    67,   143,   144,   145,
     146,   155,    65,    73,  1079,   227,  1081,   161,    62,    39,
     151,   152,   298,    62,   168,   169,   237,   238,   239,    19,
     241,   162,   163,   164,  1066,   298,   167,     3,   298,   298,
     298,   252,    19,     3,   971,  1110,   973,    67,     3,  1005,
       3,     3,    72,   296,     3,   298,     3,     3,    78,    79,
       3,   304,   243,   244,   245,   246,   247,   248,   249,   250,
    1026,    91,    92,    93,    94,     3,    96,     3,   298,    40,
     298,   293,  1038,  1039,  1040,    14,  1042,   300,   298,    13,
     301,   151,   152,   296,  1021,  1022,    26,  1024,    25,  1164,
      31,  1166,   162,   163,   164,    28,    67,   167,    62,    70,
    1066,    31,    73,    74,    62,   298,    77,    78,   298,     7,
       8,     9,    10,    11,  1080,   218,    45,   176,   176,   298,
      62,  1087,  1088,    21,    22,    23,    24,    25,    26,    27,
      28,   298,   298,    31,   105,    62,   186,    62,   109,    71,
     298,  1107,   172,  1109,   211,  1111,  1112,  1113,    18,  1115,
    1116,   298,    83,   298,  1120,   296,     7,     8,     9,    10,
      11,   183,   298,   304,   298,  1102,  1103,  1104,   298,   298,
      21,    22,    23,    24,    25,    26,    27,    28,   298,   298,
      31,   298,    86,  1149,    88,  1151,    90,   298,    92,   298,
      94,   298,   298,    97,    98,     7,     8,     9,    10,    11,
     298,  1167,  1168,     3,    18,    18,   298,    18,  1174,    21,
      22,    23,    24,    25,    26,    27,    28,    19,   298,    31,
      19,     7,     8,     9,    10,    11,   296,   300,   302,   298,
     302,   297,    14,    31,   304,    21,    22,    23,    24,    25,
      26,    27,    28,    17,   296,    31,     7,     8,     9,    10,
      11,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      21,    22,    23,    24,    25,    26,    27,    28,   302,    26,
      31,     7,     8,     9,    10,    11,   302,   298,   298,   298,
     298,   298,   298,   298,   298,    21,    22,    23,    24,    25,
      26,    27,    28,   298,   298,    31,     7,     8,     9,    10,
      11,   298,   273,   274,   298,   298,   298,   298,   298,   298,
      21,    22,    23,    24,    25,    26,    27,    28,   298,   298,
      31,     7,     8,     9,    10,    11,   298,   301,   301,   298,
     301,   298,   298,     3,   269,    21,    22,    23,    24,    25,
      26,    27,    28,   298,     3,    31,     7,     8,     9,    10,
      11,     3,     3,     3,   302,   298,   302,    19,   261,     3,
      21,    22,    23,    24,    25,    26,    27,    28,     3,   297,
      31,     7,     8,     9,    10,    11,   298,     5,   298,   298,
     298,   298,   298,   298,   298,    21,    22,    23,    24,    25,
      26,    27,    28,   300,   300,    31,   298,   298,   298,    21,
     298,     7,     8,     9,    10,    11,   296,   301,   297,   302,
     297,    14,   300,   298,   298,    21,    22,    23,    24,    25,
      26,    27,    28,   298,   298,    31,    21,   298,   297,   302,
       5,    31,    31,   301,     7,     8,     9,    10,    11,   298,
      68,    69,   298,    31,   191,   298,   149,   298,    21,    22,
      23,    24,    25,    26,    27,    28,    84,   297,    31,     7,
       8,     9,    10,    11,   296,   296,   296,   301,   296,   298,
     297,   191,   298,    21,    22,    23,    24,    25,    26,    27,
      28,   298,   298,    31,   299,   113,   298,   245,     7,     8,
       9,    10,    11,    68,    69,   300,   261,   263,   269,   302,
     799,   298,    21,    22,    23,    24,    25,    26,    27,    28,
      40,   297,    31,   136,   137,   138,   139,   140,   141,    40,
     143,   144,   145,   146,   297,   153,   154,   104,   156,   157,
     158,   159,   160,   201,   298,   298,   297,    67,   113,   298,
      70,   492,   298,    73,    74,   832,    67,    77,    78,    70,
     779,   876,    73,    74,   781,    76,    77,    78,   804,   305,
     351,   297,    83,   311,   642,  1096,   579,   195,   196,   197,
     198,   199,   200,   201,   202,   105,   675,  1024,   823,   109,
     916,   813,   103,   104,   105,   972,   297,   770,   109,    -1,
      -1,    -1,    -1,   221,   222,    -1,    -1,    -1,    -1,    74,
      -1,   229,   230,   231,   232,    80,    -1,    -1,    -1,    -1,
      85,   297,    87,    -1,    89,    -1,    91,    -1,    93,    -1,
     195,   196,   197,   198,   199,   200,   201,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,   297,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,   221,   222,    -1,    -1,
      -1,    -1,    -1,    -1,   229,   230,   231,   232,    -1,    -1,
      -1,   297,    67,   291,   292,    70,    -1,    -1,    73,    74,
      -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    83,    -1,
      85,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   297,    -1,    -1,    -1,    -1,    -1,    -1,   228,    -1,
     105,    -1,    61,   233,   109,   235,    -1,    -1,    67,    -1,
      -1,    70,   117,    -1,    73,    74,   291,   292,    77,    78,
     125,   126,    -1,    -1,   297,    -1,    -1,    -1,    -1,    -1,
      -1,   252,    91,    -1,    -1,    83,    -1,    -1,   268,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,   297,
     109,    -1,   157,    -1,    -1,    -1,   104,   278,   279,   280,
     281,    -1,    -1,    -1,    -1,    -1,    -1,   288,    -1,   174,
     175,   301,   177,   178,   122,   180,   181,   182,   297,   184,
     301,    -1,   187,   188,    -1,   190,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,   204,
      -1,    -1,    40,   208,    40,   210,    -1,   212,    -1,   214,
      -1,    -1,    -1,    -1,   219,    -1,    40,    -1,   223,   224,
      -1,   226,    -1,    59,    60,    61,    -1,    63,    64,    67,
      -1,    67,    70,    -1,    70,    73,    74,    73,    74,    77,
      78,    77,    78,    67,    -1,   193,    70,    -1,    -1,    73,
      74,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,   105,
      -1,   109,    -1,   109,    -1,    -1,    -1,    -1,    -1,   117,
      40,   105,    -1,    -1,    -1,   109,   234,    -1,   236,   237,
     238,   239,   240,   241,   242,    -1,   301,    57,    58,    -1,
      -1,    -1,    -1,    -1,   252,   253,    -1,    -1,   256,    -1,
      -1,    -1,   260,    -1,   262,     7,     8,     9,    10,    11,
     268,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    -1,    31,
      -1,    -1,   301,    -1,    -1,    -1,   106,    -1,    -1,   109,
      -1,   111,    -1,   301,   114,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,     7,     8,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,     7,
       8,     9,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    -1,    31,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   260,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,    -1,    -1,   301,    -1,   301,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   301
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   307,   308,    40,    57,    58,   106,   109,   111,
     114,   119,   124,   125,   126,   127,   328,   329,   330,   369,
     370,   371,   372,   374,   375,   376,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   474,   475,   476,   477,   478,   479,
     480,   481,   499,   501,     0,     3,    32,    37,    40,    51,
      67,    70,    72,    79,    98,   109,   128,   173,   298,   310,
     318,   324,   327,   332,   334,   337,   339,   342,   343,   348,
     354,   355,   401,   403,   413,   443,   444,   445,   446,   482,
     483,   484,   485,   493,   494,   500,   502,   503,   504,    18,
      19,    39,   399,    19,    50,   331,     3,   107,   108,   399,
     399,     3,    89,    90,   112,   115,   117,   118,   227,   293,
      65,    89,    90,   120,   121,   122,   123,    14,    34,    39,
     388,   399,   399,   399,    18,    19,    38,    49,   320,    89,
      95,    37,    55,    72,   117,   325,   326,    18,    71,    18,
      99,    19,    89,    18,    12,    14,    15,    16,    17,    18,
      19,    25,    26,    28,    30,    73,   151,   152,   162,   163,
     164,   167,   296,   304,   418,   422,   427,   430,   431,   432,
     434,   436,    18,   344,   344,    59,    64,    75,    99,   227,
     272,   277,   293,   344,    40,    67,    70,    73,    74,    76,
      77,    78,   105,   109,   301,   345,   357,   344,    45,    83,
      85,   117,   125,   126,   157,   174,   175,   177,   178,   180,
     181,   182,   184,   187,   188,   190,   203,   204,   208,   210,
     212,   214,   219,   223,   224,   226,   301,   345,   344,   228,
     233,   235,   268,   301,   345,   447,   448,   449,   344,   273,
     274,   301,   345,   344,    76,    83,   103,   104,   252,   278,
     279,   280,   281,   288,   301,   345,   364,   366,   367,   344,
     117,   260,   301,   345,   495,   498,    59,    60,    61,    63,
      64,   301,   333,   336,   345,    61,    91,   301,   335,   345,
      34,    39,   300,   358,   331,     3,   399,   399,     3,    34,
      39,   300,   361,    18,    19,    39,   400,     3,   116,     3,
       3,   377,    18,   373,    76,   122,   267,   270,   271,   373,
     373,     3,     3,     3,     3,     3,     3,     3,    14,   389,
       3,     3,     3,    21,    56,   319,    39,   300,   321,   358,
      14,    18,   296,   309,    19,    52,   298,    14,    16,    17,
     338,   402,   340,   298,   361,   414,    14,    18,    27,   296,
     303,   426,   428,   434,    28,   296,   171,   171,    14,   303,
     429,   436,   296,   296,   296,   296,   436,    15,    16,   296,
     309,   418,   419,   420,   421,   422,   423,   424,     5,    68,
      69,    84,   113,   153,   154,   156,   157,   158,   159,   160,
     195,   196,   197,   198,   199,   200,   201,   202,   221,   222,
     229,   230,   231,   232,   291,   292,   435,   505,     7,     8,
       9,    10,    11,    21,    22,    23,    24,    25,    26,    27,
      28,    31,   300,   300,   300,   358,    18,   347,    71,   309,
      33,    34,    35,    36,   309,   311,   356,    39,    79,    80,
     172,   346,   410,   346,    19,   361,   298,   298,   300,   192,
     193,    19,   311,   211,   194,   117,   207,   102,    17,    18,
     176,   312,   312,    62,   179,    62,   298,   309,   177,   185,
     196,   197,   189,   191,   205,   206,   205,   209,   311,   213,
     215,   220,    67,   189,    75,   225,   189,   298,   300,   338,
     311,   309,   311,   298,   300,   300,    67,   275,   276,   298,
     300,   491,   492,    19,   309,   300,    80,   286,   287,   486,
     309,   309,    62,    62,   175,   289,   290,   298,   298,   300,
     300,   312,   313,   261,   298,   134,   183,   216,   294,   295,
     298,    67,   311,    62,   311,    65,   298,   298,   298,    62,
      19,   317,   298,   298,    65,    81,    82,    87,    88,    90,
     359,   360,     3,     3,     3,     3,    81,   110,   362,   363,
       3,   400,     3,     3,     3,    39,    67,    72,    78,    79,
      91,    92,    93,    94,    96,   313,   378,   410,     3,   373,
     373,   373,    39,   373,    39,   373,     3,     3,     3,    17,
     309,    19,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   322,   323,   298,   436,   309,    53,    54,   298,   136,
     137,   138,   139,   140,   141,   143,   144,   145,   146,   404,
     405,   406,   408,   412,   100,   101,   102,   103,   298,   341,
     298,   296,   411,   428,   428,   436,   428,   428,    13,    14,
     436,   440,   441,    25,    26,   429,   429,    28,   436,   436,
     436,   436,   297,   302,    14,    18,    27,   296,   309,    31,
     302,   305,    25,    26,   300,   302,   305,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
     138,   155,   161,   168,   169,   338,   349,   352,   353,    18,
     408,   409,   436,    62,    62,   298,   298,   309,    45,   218,
     311,   312,   309,   176,   176,   309,    62,   309,   298,    62,
     186,   309,   309,   311,    62,   298,    71,   298,   311,   298,
     311,   186,   216,   217,   311,   347,   311,   298,   298,   311,
     211,   450,    18,   298,   298,   353,   298,   298,   365,   366,
     298,   298,   298,   309,   309,   298,   298,   298,    71,    97,
     133,   368,    28,   299,   315,   311,    19,   312,   183,   496,
     497,   309,   309,   311,   309,   312,   313,   314,   301,   302,
     301,   302,     3,    18,    18,   409,   314,    18,    19,   473,
     473,   473,   373,   473,   373,   473,   298,   298,   301,   302,
     297,    19,    18,   142,   298,   300,   312,   309,   297,   405,
     407,   412,   297,   302,   297,   297,   302,   302,   302,   436,
     302,   436,   302,    31,    17,   420,    14,    17,   423,   296,
      26,   351,   302,   309,   309,   298,   311,   311,   298,   298,
     298,   309,   309,   298,   309,   298,   309,   298,   298,   298,
     298,   309,   338,   298,   298,   298,   298,   298,   298,   298,
     298,   298,   309,    76,   234,   257,   259,   266,   301,   451,
     456,   459,   298,   300,   490,   301,   298,   298,   298,   309,
      84,    86,    84,    86,   301,   309,   312,   298,   312,   269,
     496,   360,   363,     3,     3,     3,   473,     3,   473,     3,
     323,   406,    18,    45,    73,   129,   130,   131,   132,   133,
     134,   135,   147,   150,   415,   416,   432,   433,   437,   442,
     297,   298,   440,   436,   436,   436,   297,    27,   309,   297,
      27,   309,   419,   302,   436,    19,   314,   350,   352,   298,
     302,   298,   298,   298,   298,   298,   298,   298,   298,   469,
     472,   236,   311,   300,   261,   313,   452,   453,   298,   300,
     489,   365,   298,    19,   316,   316,   316,   316,   309,     3,
       3,    21,   296,    21,   296,   436,   436,   436,   301,   442,
     435,   505,   298,   436,   438,   439,   405,   297,   297,   297,
     297,   297,   297,    31,   297,    14,   297,   314,   309,   353,
     309,   461,   338,   300,   457,   189,   274,   301,   488,   368,
     298,   298,   298,   298,   436,   441,   436,   432,   298,   148,
     300,    21,    21,    31,   302,   298,    31,    31,   301,   298,
     300,   471,   301,   313,   462,   463,   464,   454,   193,   237,
     238,   239,   241,   252,   301,   364,   458,   309,   243,   282,
     283,   284,   285,   487,   298,   368,   368,   368,   368,   298,
     297,   298,   505,   300,   417,   442,   425,   436,   436,   163,
     164,   167,   171,   439,   309,    31,   470,   258,   298,   116,
     236,   258,   301,   455,   309,   309,   309,   191,    93,   309,
      80,   251,   254,   298,   297,   415,   149,     6,   301,   420,
     298,   298,   296,   296,   296,   298,   297,   296,   122,   193,
     234,   236,   237,   238,   239,   240,   241,   242,   252,   253,
     256,   260,   262,   268,   301,   364,   460,   298,   311,   309,
     311,   298,   309,   309,   298,   301,   417,   415,   299,   436,
     436,   436,   309,   300,   309,   311,   309,   309,   309,   191,
     309,    93,   309,   243,   244,   245,   246,   247,   248,   249,
     250,    80,   251,   254,   245,   309,   261,   263,   269,   298,
     415,   297,   297,   297,   302,   465,   309,   309,   311,   311,
     264,   265,   309,   309,   298,   298,   298,   309,    17,   301,
     466,   467,   468,   297,   255,   298,   298
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
#line 383 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 384 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 397 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 398 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 402 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 406 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 415 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 416 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 417 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 418 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 419 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 420 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 427 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 434 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 442 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 446 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 450 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 457 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 465 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 466 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 472 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 481 "cf-parse.y"
    { (yyval.t) = (yyvsp[(2) - (2)].t); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 482 "cf-parse.y"
    { (yyval.t) = bird_name; ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 486 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 491 "cf-parse.y"
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[(2) - (2)].t); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 492 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 496 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 497 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 501 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 502 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 506 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 507 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 508 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 509 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 510 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 511 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 512 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 513 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 514 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 520 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 521 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 530 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 531 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 532 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 533 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 536 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 537 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 538 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 539 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 551 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 554 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 557 "cf-parse.y"
    { cmd_shutdown(); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 560 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 569 "cf-parse.y"
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
#line 585 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 586 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 590 "cf-parse.y"
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
#line 597 "cf-parse.y"
    { THIS_KRT->devroutes = (yyvsp[(3) - (3)].i); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 603 "cf-parse.y"
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
#line 616 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 620 "cf-parse.y"
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
#line 633 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 646 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 652 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 654 "cf-parse.y"
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
#line 673 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 674 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 675 "cf-parse.y"
    { new_config->listen_bgp_flags = 0; ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 676 "cf-parse.y"
    { new_config->listen_bgp_flags = 1; ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 683 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 695 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 701 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 709 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 713 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 714 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 715 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 716 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 717 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 718 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 719 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 720 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 724 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 726 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 727 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 731 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 739 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 740 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 748 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 756 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 757 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 758 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 762 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 763 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 780 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 795 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 810 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 811 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 812 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 817 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 821 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 822 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 823 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 824 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 825 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 826 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 832 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 833 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 834 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 839 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 843 "cf-parse.y"
    { (yyval.i) = MD_STATES; ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 844 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 865 "cf-parse.y"
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
#line 883 "cf-parse.y"
    { ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 884 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 885 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 886 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 887 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 888 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 897 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 900 "cf-parse.y"
    { cmd_show_memory(); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 903 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 906 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 910 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 914 "cf-parse.y"
    { if_show(); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 917 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 920 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 923 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 929 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 935 "cf-parse.y"
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
#line 942 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 947 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 952 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 957 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 961 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 965 "cf-parse.y"
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
#line 975 "cf-parse.y"
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
#line 983 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 987 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 994 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 995 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 999 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1003 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1005 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1007 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1009 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1011 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1013 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1015 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1017 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1023 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1024 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1029 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1030 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1037 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1039 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1041 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1043 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1045 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1047 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1051 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1055 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1058 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1061 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1062 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1063 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1067 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1068 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1069 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1077 "cf-parse.y"
    { (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1078 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s)->def = (yyvsp[(4) - (4)].f);
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1087 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1091 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1092 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1093 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1094 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1095 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1096 "cf-parse.y"
    { (yyval.i) = T_QUAD; ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1097 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1098 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1099 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1100 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1101 "cf-parse.y"
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
#line 1121 "cf-parse.y"
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
#line 1132 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1133 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1140 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1141 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1148 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1157 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1165 "cf-parse.y"
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
#line 1189 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1190 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1194 "cf-parse.y"
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
#line 1207 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1210 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1223 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1224 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1227 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1228 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1232 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1235 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1244 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1248 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1249 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1250 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1251 "cf-parse.y"
    { (yyval.v).type = (yyvsp[(1) - (1)].i) >> 16; (yyval.v).val.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1255 "cf-parse.y"
    {
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
   ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1261 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (8)].i), (yyvsp[(4) - (8)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (8)].i), (yyvsp[(7) - (8)].i));
   ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1267 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (12)].i), (yyvsp[(4) - (12)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(9) - (12)].i), (yyvsp[(11) - (12)].i));
   ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1273 "cf-parse.y"
    { 	/* This is probably useless :-) */
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = 0;
	(yyval.e)->to.val.i = 0xffffffff;
   ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1279 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (5)].i), 0); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (5)].i), 0xffff);
   ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1285 "cf-parse.y"
    { 
	(yyval.e) = f_generate_rev_wcard(0, 0xffff, (yyvsp[(4) - (5)].i));
   ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1292 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (1)].v); 
	(yyval.e)->to = (yyvsp[(1) - (1)].v);
   ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1297 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (4)].v); 
	(yyval.e)->to = (yyvsp[(4) - (4)].v); 
   ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1305 "cf-parse.y"
    { (yyval.e) = (yyvsp[(1) - (1)].e); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1306 "cf-parse.y"
    { (yyval.e) = (yyvsp[(3) - (3)].e); (yyval.e)->left = (yyvsp[(1) - (3)].e); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1310 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1317 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1318 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1319 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1320 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1327 "cf-parse.y"
    { (yyval.trie) = f_new_trie(cfg_mem); trie_add_fprefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1328 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_fprefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1331 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1332 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[(2) - (4)].e);
     (yyval.e)->data = (yyvsp[(4) - (4)].x);
     (yyval.e)->left = (yyvsp[(1) - (4)].e);
   ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1337 "cf-parse.y"
    {
     (yyval.e) = f_new_tree(); 
     (yyval.e)->from.type = T_VOID; 
     (yyval.e)->to.type = T_VOID;
     (yyval.e)->data = (yyvsp[(3) - (3)].x);
     (yyval.e)->left = (yyvsp[(1) - (3)].e);
   ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1349 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1350 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1354 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1355 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1359 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1360 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1361 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1362 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1363 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1367 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1368 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1369 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1373 "cf-parse.y"
    {
        if (((yyvsp[(2) - (5)].x)->code == 'c') && ((yyvsp[(4) - (5)].x)->code == 'c'))
          { 
            if (((yyvsp[(2) - (5)].x)->aux != T_INT) || ((yyvsp[(4) - (5)].x)->aux != T_INT))
              cf_error( "Can't operate with value of non-integer type in pair constructor" );
            (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PAIR;  (yyval.x)->a2.i = make_pair((yyvsp[(2) - (5)].x)->a2.i, (yyvsp[(4) - (5)].x)->a2.i);
          }
	else
	  { (yyval.x) = f_new_inst(); (yyval.x)->code = P('m', 'p'); (yyval.x)->a1.p = (yyvsp[(2) - (5)].x); (yyval.x)->a2.p = (yyvsp[(4) - (5)].x); }
    ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1386 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1387 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1388 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1389 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1390 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1391 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1392 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1393 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1394 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1395 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1396 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1410 "cf-parse.y"
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

  case 286:

/* Line 1455 of yacc.c  */
#line 1433 "cf-parse.y"
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

  case 287:

/* Line 1455 of yacc.c  */
#line 1467 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1469 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1470 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1471 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1472 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1473 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1474 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1475 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1479 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1480 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1481 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1482 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1483 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1484 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1485 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1486 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1487 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1488 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1489 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1490 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1491 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1492 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1493 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1494 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1496 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1497 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1498 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1500 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1502 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1504 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1506 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1507 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1508 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1509 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1510 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1520 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1521 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1522 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1523 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1524 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1529 "cf-parse.y"
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

  case 328:

/* Line 1455 of yacc.c  */
#line 1552 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1553 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1554 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1555 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1556 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1557 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1561 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1564 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1565 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1566 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1575 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1582 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1591 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1592 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1596 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1602 "cf-parse.y"
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

  case 344:

/* Line 1455 of yacc.c  */
#line 1612 "cf-parse.y"
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

  case 345:

/* Line 1455 of yacc.c  */
#line 1621 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1627 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1632 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1639 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1644 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1650 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1651 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1652 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1661 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1662 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1663 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1664 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1670 "cf-parse.y"
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

  case 360:

/* Line 1455 of yacc.c  */
#line 1694 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1695 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(3) - (6)].a); BGP_CFG->local_as = (yyvsp[(5) - (6)].i); ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1696 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1702 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1703 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1704 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1705 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1706 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1707 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1708 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1709 "cf-parse.y"
    { BGP_CFG->multihop = 64; ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1710 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (4)].i); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1711 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1712 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1713 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1714 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1715 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_DIRECT; ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1716 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_RECURSIVE; ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1717 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1718 "cf-parse.y"
    { BGP_CFG->med_metric = (yyvsp[(4) - (5)].i); ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1719 "cf-parse.y"
    { BGP_CFG->igp_metric = (yyvsp[(4) - (5)].i); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1720 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1721 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1722 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1723 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1724 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1725 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1726 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1727 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1728 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1729 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1730 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1731 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1732 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1733 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1734 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1735 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1736 "cf-parse.y"
    { BGP_CFG->igp_table = (yyvsp[(4) - (5)].r); ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1746 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1763 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1764 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (2)].i) ? DEFAULT_ECMP_LIMIT : 0; ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1765 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (4)].i) ? (yyvsp[(4) - (4)].i) : 0; if ((yyvsp[(4) - (4)].i) < 0) cf_error("ECMP limit cannot be negative"); ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1766 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i); if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1770 "cf-parse.y"
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

  case 408:

/* Line 1455 of yacc.c  */
#line 1781 "cf-parse.y"
    { ospf_area_finish(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1790 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1791 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1804 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1818 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1819 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1820 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1824 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1834 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1835 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1836 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1837 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1838 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1839 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1840 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1841 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1842 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1847 "cf-parse.y"
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

  case 441:

/* Line 1455 of yacc.c  */
#line 1868 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1869 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1870 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1871 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1872 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1873 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1874 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1875 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1876 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1877 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1878 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1879 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1880 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1881 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1882 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1883 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1884 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1885 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1886 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1887 "cf-parse.y"
    { OSPF_PATT->check_link = (yyvsp[(3) - (3)].i); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1888 "cf-parse.y"
    { OSPF_PATT->ecmp_weight = (yyvsp[(3) - (3)].i) - 1; if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("ECMP weight must be in range 1-256"); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1890 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1891 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1892 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1893 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1894 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1895 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) || ((yyvsp[(3) - (3)].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1909 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1937 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1946 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1956 "cf-parse.y"
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

  case 487:

/* Line 1455 of yacc.c  */
#line 1987 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1992 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1997 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2000 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2003 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2008 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2011 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2016 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2019 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2022 "cf-parse.y"
    { ospf_sh_lsadb(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf)); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2027 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2037 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2042 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2043 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2049 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2058 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2059 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2060 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2061 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2062 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2063 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2065 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2066 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2067 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2072 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2073 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2074 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2079 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2080 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2081 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2082 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2083 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2087 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2088 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2102 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2118 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2127 "cf-parse.y"
    { STATIC_CFG->check_link = (yyvsp[(4) - (5)].i); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2131 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2140 "cf-parse.y"
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[(2) - (2)].a);
     this_srt_nh->if_name = (void *) this_srt; /* really */
   ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2147 "cf-parse.y"
    {
     this_srt_nh->masklen = (yyvsp[(3) - (3)].i) - 1; /* really */
     if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("Weight must be in range 1-256"); 
   ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2154 "cf-parse.y"
    { this_srt->mp_next = this_srt_nh; ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2155 "cf-parse.y"
    { last_srt_nh->mp_next = this_srt_nh; ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2159 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2163 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2169 "cf-parse.y"
    {
      this_srt->dest = RTD_MULTIPATH;
   ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2172 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2173 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2174 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2178 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2186 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2186 "cf-parse.y"
    { ospf_proto_finish(); ;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2186 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2189 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); ;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2189 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); ;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2190 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); ;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2190 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2191 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2192 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2193 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2194 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2195 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2196 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2197 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2199 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); ;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); ;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); ;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2200 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;



/* Line 1455 of yacc.c  */
#line 7263 "cf-parse.tab.c"
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
#line 2202 "cf-parse.y"

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



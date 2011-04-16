
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
     ROUTER = 312,
     ID = 313,
     PROTOCOL = 314,
     PREFERENCE = 315,
     DISABLED = 316,
     DIRECT = 317,
     INTERFACE = 318,
     IMPORT = 319,
     EXPORT = 320,
     FILTER = 321,
     NONE = 322,
     STATES = 323,
     FILTERS = 324,
     PASSWORD = 325,
     FROM = 326,
     PASSIVE = 327,
     TO = 328,
     EVENTS = 329,
     PACKETS = 330,
     PROTOCOLS = 331,
     INTERFACES = 332,
     PRIMARY = 333,
     STATS = 334,
     COUNT = 335,
     FOR = 336,
     COMMANDS = 337,
     PREEXPORT = 338,
     GENERATE = 339,
     LISTEN = 340,
     BGP = 341,
     V6ONLY = 342,
     DUAL = 343,
     ADDRESS = 344,
     PORT = 345,
     PASSWORDS = 346,
     DESCRIPTION = 347,
     RELOAD = 348,
     IN = 349,
     OUT = 350,
     MRTDUMP = 351,
     MESSAGES = 352,
     RESTRICT = 353,
     MEMORY = 354,
     IGP_METRIC = 355,
     SHOW = 356,
     STATUS = 357,
     SUMMARY = 358,
     ROUTE = 359,
     SYMBOLS = 360,
     DUMP = 361,
     RESOURCES = 362,
     SOCKETS = 363,
     NEIGHBORS = 364,
     ATTRIBUTES = 365,
     ECHO = 366,
     DISABLE = 367,
     ENABLE = 368,
     RESTART = 369,
     FUNCTION = 370,
     PRINT = 371,
     PRINTN = 372,
     UNSET = 373,
     RETURN = 374,
     ACCEPT = 375,
     REJECT = 376,
     QUITBIRD = 377,
     INT = 378,
     BOOL = 379,
     IP = 380,
     PREFIX = 381,
     PAIR = 382,
     QUAD = 383,
     SET = 384,
     STRING = 385,
     BGPMASK = 386,
     BGPPATH = 387,
     CLIST = 388,
     IF = 389,
     THEN = 390,
     ELSE = 391,
     CASE = 392,
     TRUE = 393,
     FALSE = 394,
     GW = 395,
     NET = 396,
     MASK = 397,
     PROTO = 398,
     SOURCE = 399,
     SCOPE = 400,
     CAST = 401,
     DEST = 402,
     LEN = 403,
     DEFINED = 404,
     ADD = 405,
     DELETE = 406,
     CONTAINS = 407,
     RESET = 408,
     PREPEND = 409,
     FIRST = 410,
     LAST = 411,
     MATCH = 412,
     EMPTY = 413,
     WHERE = 414,
     EVAL = 415,
     LOCAL = 416,
     NEIGHBOR = 417,
     AS = 418,
     HOLD = 419,
     CONNECT = 420,
     RETRY = 421,
     KEEPALIVE = 422,
     MULTIHOP = 423,
     STARTUP = 424,
     VIA = 425,
     NEXT = 426,
     HOP = 427,
     SELF = 428,
     DEFAULT = 429,
     PATH = 430,
     METRIC = 431,
     START = 432,
     DELAY = 433,
     FORGET = 434,
     WAIT = 435,
     AFTER = 436,
     BGP_PATH = 437,
     BGP_LOCAL_PREF = 438,
     BGP_MED = 439,
     BGP_ORIGIN = 440,
     BGP_NEXT_HOP = 441,
     BGP_ATOMIC_AGGR = 442,
     BGP_AGGREGATOR = 443,
     BGP_COMMUNITY = 444,
     RR = 445,
     RS = 446,
     CLIENT = 447,
     CLUSTER = 448,
     AS4 = 449,
     ADVERTISE = 450,
     IPV4 = 451,
     CAPABILITIES = 452,
     LIMIT = 453,
     PREFER = 454,
     OLDER = 455,
     MISSING = 456,
     LLADDR = 457,
     DROP = 458,
     IGNORE = 459,
     REFRESH = 460,
     INTERPRET = 461,
     COMMUNITIES = 462,
     BGP_ORIGINATOR_ID = 463,
     BGP_CLUSTER_LIST = 464,
     IGP = 465,
     GATEWAY = 466,
     RECURSIVE = 467,
     OSPF = 468,
     AREA = 469,
     OSPF_METRIC1 = 470,
     OSPF_METRIC2 = 471,
     OSPF_TAG = 472,
     OSPF_ROUTER_ID = 473,
     RFC1583COMPAT = 474,
     STUB = 475,
     TICK = 476,
     COST = 477,
     RETRANSMIT = 478,
     HELLO = 479,
     TRANSMIT = 480,
     PRIORITY = 481,
     DEAD = 482,
     TYPE = 483,
     BROADCAST = 484,
     BCAST = 485,
     NONBROADCAST = 486,
     NBMA = 487,
     POINTOPOINT = 488,
     PTP = 489,
     POINTOMULTIPOINT = 490,
     PTMP = 491,
     SIMPLE = 492,
     AUTHENTICATION = 493,
     STRICT = 494,
     CRYPTOGRAPHIC = 495,
     ELIGIBLE = 496,
     POLL = 497,
     NETWORKS = 498,
     HIDDEN = 499,
     VIRTUAL = 500,
     CHECK = 501,
     LINK = 502,
     RX = 503,
     BUFFER = 504,
     LARGE = 505,
     NORMAL = 506,
     STUBNET = 507,
     LSADB = 508,
     ECMP = 509,
     WEIGHT = 510,
     TOPOLOGY = 511,
     STATE = 512,
     PIPE = 513,
     PEER = 514,
     MODE = 515,
     OPAQUE = 516,
     TRANSPARENT = 517,
     RIP = 518,
     INFINITY = 519,
     PERIOD = 520,
     GARBAGE = 521,
     TIMEOUT = 522,
     MULTICAST = 523,
     QUIET = 524,
     NOLISTEN = 525,
     VERSION1 = 526,
     PLAINTEXT = 527,
     MD5 = 528,
     HONOR = 529,
     NEVER = 530,
     ALWAYS = 531,
     RIP_METRIC = 532,
     RIP_TAG = 533,
     STATIC = 534,
     PROHIBIT = 535,
     MULTIPATH = 536
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
#line 644 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 656 "cf-parse.tab.c"

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
#define YYLAST   1943

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  303
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  200
/* YYNRULES -- Number of rules.  */
#define YYNRULES  633
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1191

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   536

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,     2,    29,     2,     2,
     293,   294,    27,    25,   299,    26,    31,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   296,   295,
      22,    21,    23,   300,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   301,     2,   302,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   297,     2,   298,    24,     2,     2,     2,
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
     286,   287,   288,   289,   290,   291,   292
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
    1262,  1268,  1274,  1280,  1286,  1293,  1300,  1309,  1316,  1323,
    1329,  1334,  1340,  1345,  1351,  1356,  1362,  1368,  1371,  1375,
    1379,  1381,  1384,  1387,  1392,  1395,  1397,  1400,  1405,  1406,
    1410,  1414,  1417,  1422,  1425,  1428,  1430,  1435,  1437,  1439,
    1440,  1444,  1447,  1450,  1453,  1458,  1460,  1461,  1465,  1466,
    1469,  1472,  1476,  1479,  1482,  1486,  1489,  1492,  1495,  1497,
    1501,  1504,  1507,  1510,  1513,  1516,  1519,  1523,  1526,  1529,
    1532,  1535,  1538,  1541,  1544,  1547,  1551,  1554,  1558,  1561,
    1565,  1569,  1574,  1577,  1580,  1583,  1587,  1591,  1595,  1597,
    1598,  1601,  1603,  1605,  1608,  1612,  1613,  1616,  1618,  1620,
    1623,  1627,  1628,  1629,  1633,  1634,  1638,  1642,  1644,  1645,
    1650,  1657,  1664,  1671,  1679,  1686,  1694,  1701,  1704,  1708,
    1712,  1718,  1723,  1728,  1731,  1735,  1739,  1744,  1749,  1754,
    1760,  1766,  1771,  1775,  1780,  1785,  1790,  1795,  1797,  1799,
    1801,  1803,  1805,  1807,  1809,  1811,  1812,  1815,  1818,  1819,
    1823,  1824,  1828,  1829,  1833,  1836,  1840,  1844,  1850,  1854,
    1857,  1860,  1864,  1866,  1869,  1873,  1877,  1881,  1884,  1887,
    1890,  1895,  1897,  1899,  1901,  1903,  1905,  1907,  1909,  1911,
    1913,  1915,  1917,  1919,  1921,  1923,  1925,  1927,  1929,  1931,
    1933,  1935,  1937,  1939,  1941,  1943,  1945,  1947,  1949,  1951,
    1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,  1969,  1971,
    1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,  1989,  1991,
    1993,  1995,  1998,  2001,  2004,  2007,  2010,  2013,  2016,  2019,
    2023,  2027,  2031,  2035,  2039,  2043,  2047,  2049,  2051,  2053,
    2055,  2057,  2059,  2061,  2063,  2065,  2067,  2069,  2071,  2073,
    2075,  2077,  2079,  2081
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     304,     0,    -1,   305,     3,    -1,     4,   498,    -1,    -1,
     305,   497,    -1,    14,    -1,   293,   433,   294,    -1,    18,
      -1,    32,    18,    21,   306,   295,    -1,    32,    18,    21,
      17,   295,    -1,   306,    -1,    33,    -1,    35,    -1,    34,
      -1,    36,    -1,    -1,    17,    -1,    18,    -1,   309,   312,
      -1,   310,    -1,   309,    -1,    28,   306,    -1,   296,   309,
      -1,    19,    -1,    19,    -1,    -1,    37,   317,   318,   295,
      -1,    56,    19,    -1,    -1,    19,    -1,    38,   316,    -1,
      49,    -1,    39,    -1,   297,   319,   298,    -1,   320,    -1,
     319,   299,   320,    -1,    40,    -1,    41,    -1,    42,    -1,
      43,    -1,    44,    -1,    45,    -1,    46,    -1,    47,    -1,
      48,    -1,   107,    87,   358,   295,    -1,   107,    19,   295,
      -1,   115,    -1,    70,    -1,    55,    -1,    37,    -1,   322,
      19,    -1,   322,    19,   306,    19,    -1,   322,    52,    53,
      -1,   322,    52,    54,    -1,    51,   323,   295,    -1,    57,
     328,     3,    -1,    57,    50,   328,     3,    -1,    58,     3,
      -1,    -1,    19,    -1,   340,    59,    -1,    60,   308,    -1,
      61,    62,   306,    -1,    63,   308,    -1,    64,    65,   308,
      -1,   340,    64,    -1,    61,    62,   306,    -1,    89,   314,
     311,    -1,    59,    67,   306,    -1,    68,    69,   335,   295,
      -1,    14,    -1,    16,    -1,    17,    -1,    96,    97,   337,
     295,    -1,    -1,   337,   338,    -1,   100,   309,    -1,   101,
     306,    -1,    98,    -1,    99,    -1,    67,    18,    -1,    70,
      -1,    -1,    18,    -1,    -1,    71,   306,    -1,    72,   308,
      -1,    40,   355,    -1,   107,   358,    -1,    75,   343,    -1,
      76,   343,    -1,    67,   344,    -1,    68,    69,   335,    -1,
     103,    19,    -1,    77,   406,    -1,   407,    -1,    39,    -1,
      78,    -1,    18,    -1,    40,    87,   355,    -1,    40,    93,
     306,    -1,    -1,    19,    -1,   311,    -1,    19,   311,    -1,
      -1,    26,    -1,   346,   348,   347,    -1,   349,    -1,   350,
     299,   349,    -1,   340,    73,    -1,   351,   341,   297,    -1,
     352,   342,   295,    -1,   352,   354,   295,    -1,    -1,    74,
     353,   350,    -1,    39,    -1,    34,    -1,   297,   356,   298,
      -1,   357,    -1,   356,   299,   357,    -1,    79,    -1,    65,
      -1,    80,    -1,    88,    -1,    85,    -1,    86,    -1,    39,
      -1,    34,    -1,   297,   359,   298,    -1,   360,    -1,   359,
     299,   360,    -1,    79,    -1,   108,    -1,   102,   297,   362,
     298,    -1,   363,    -1,    -1,   363,   295,   362,    -1,   364,
     297,   365,   298,    -1,   364,    -1,    81,    19,    -1,    -1,
      95,    82,   313,   295,   365,    -1,    95,    84,   313,   295,
     365,    -1,   131,    82,   313,   295,   365,    -1,   131,    84,
     313,   295,   365,    -1,    69,   306,   295,   365,    -1,   112,
     113,     3,    -1,   112,   110,     3,    -1,   112,    87,   397,
       3,    -1,   112,    87,    39,   397,     3,    -1,    18,    -1,
      -1,   112,    88,     3,    -1,   112,    88,   114,     3,    -1,
     112,   115,   374,     3,    -1,    -1,   374,   310,    -1,   374,
      92,   311,    -1,   374,    67,    18,    -1,   374,    77,   406,
      -1,   374,   407,    -1,   374,    39,    -1,   374,    89,    -1,
     374,   375,    18,    -1,   374,    70,    18,    -1,   374,    90,
      -1,   374,    91,    -1,    94,    -1,    76,    -1,   112,   116,
     370,     3,    -1,   117,   118,     3,    -1,   117,   119,     3,
      -1,   117,    88,     3,    -1,   117,   120,     3,    -1,   117,
     121,     3,    -1,   117,    65,     3,    -1,   117,    87,     3,
      -1,   122,   385,   386,     3,    -1,    39,    -1,    34,    -1,
      14,    -1,    -1,    14,    -1,   123,   396,     3,    -1,   124,
     396,     3,    -1,   125,   396,     3,    -1,   104,   396,     3,
      -1,   104,   105,   396,     3,    -1,   104,   106,   396,     3,
      -1,    40,   396,   355,     3,    -1,   107,   396,   358,     3,
      -1,   109,     3,    -1,    18,    -1,    39,    -1,    19,    -1,
      18,    -1,    -1,    19,    -1,    -1,    77,    18,   399,   405,
      -1,   171,   433,    -1,   134,    -1,   135,    -1,   136,    -1,
     137,    -1,   138,    -1,   139,    -1,   141,    -1,   142,    -1,
     143,    -1,   144,    -1,   401,   140,    -1,   401,    18,    -1,
      -1,   402,   295,   403,    -1,   402,    -1,   404,   295,   402,
      -1,   409,    -1,    18,    -1,   405,    -1,   170,   433,    -1,
     293,   404,   294,    -1,   293,   294,    -1,   403,   297,   412,
     298,    -1,    -1,   126,    18,   411,   408,   409,    -1,    -1,
     413,    -1,   439,    -1,   413,   439,    -1,   439,    -1,   297,
     412,   298,    -1,    17,    -1,   306,    -1,    16,    -1,   415,
      -1,    15,    -1,   293,   306,   299,   306,   294,    -1,   293,
     306,   299,   306,    31,    31,   306,   294,    -1,   293,   306,
     299,   306,   294,    31,    31,   293,   306,   299,   306,   294,
      -1,   293,    27,   299,    27,   294,    -1,   293,   306,   299,
      27,   294,    -1,   293,    27,   299,   306,   294,    -1,   416,
      -1,   416,    31,    31,   416,    -1,   417,    -1,   418,   299,
     417,    -1,    17,    28,    14,    -1,   419,    -1,   419,    25,
      -1,   419,    26,    -1,   419,   297,    14,   299,    14,   298,
      -1,   420,    -1,   421,   299,   420,    -1,    -1,   422,   417,
     296,   412,    -1,   422,     6,   412,    -1,   431,    -1,   293,
     433,   294,    -1,    12,   425,    13,    -1,    28,   426,    28,
      -1,    14,   425,    -1,    27,   425,    -1,   300,   425,    -1,
     423,   425,    -1,    -1,    14,   426,    -1,   300,   426,    -1,
      -1,   293,   433,   299,   433,   294,    -1,    14,    -1,   149,
      -1,   150,    -1,    19,    -1,   415,    -1,   419,    -1,    16,
      -1,   301,   418,   302,    -1,   301,   421,   302,    -1,    15,
      -1,   424,    -1,    -1,    18,   293,   438,   294,    -1,    18,
      -1,    82,    -1,   151,    -1,   152,    -1,   154,    -1,   155,
      -1,   156,    -1,   157,    -1,   158,    -1,   293,   433,   294,
      -1,   433,    25,   433,    -1,   433,    26,   433,    -1,   433,
      27,   433,    -1,   433,    28,   433,    -1,   433,    10,   433,
      -1,   433,    11,   433,    -1,   433,    21,   433,    -1,   433,
       9,   433,    -1,   433,    22,   433,    -1,   433,     8,   433,
      -1,   433,    23,   433,    -1,   433,     7,   433,    -1,   433,
      24,   433,    -1,    30,   433,    -1,   160,   293,   433,   294,
      -1,   431,    -1,   428,    -1,   427,    -1,    71,    -1,   429,
     432,    -1,   429,   502,    -1,   433,    31,   136,    -1,   433,
      31,   159,    -1,   433,    31,   153,   293,   433,   294,    -1,
     433,    31,   166,    -1,   433,    31,   167,    -1,    25,   169,
      25,    -1,    26,   169,    26,    -1,   165,   293,   433,   299,
     433,   294,    -1,   161,   293,   433,   299,   433,   294,    -1,
     162,   293,   433,   299,   433,   294,    -1,    18,   293,   438,
     294,    -1,   133,    -1,   131,    -1,   132,    -1,    45,    -1,
     127,    -1,   128,    -1,   433,    -1,    -1,   435,    -1,   435,
     299,   436,    -1,   433,    -1,   433,   299,   437,    -1,    -1,
     437,    -1,   145,   433,   146,   414,    -1,   145,   433,   146,
     414,   147,   414,    -1,    18,    21,   433,   295,    -1,   130,
     433,   295,    -1,   429,   502,    21,   433,   295,    -1,   429,
     432,    21,   433,   295,    -1,    71,    21,   433,   295,    -1,
     129,   293,   429,   502,   294,   295,    -1,   434,   436,   295,
      -1,   430,   295,    -1,   148,   433,   297,   422,   298,    -1,
     429,   502,    31,   169,   295,    -1,   429,   502,    31,   165,
     293,   433,   294,   295,    -1,   429,   502,    31,   161,   293,
     433,   294,   295,    -1,   429,   502,    31,   162,   293,   433,
     294,   295,    -1,   340,    97,    -1,   440,   341,   297,    -1,
     441,   342,   295,    -1,   441,   172,   174,   306,   295,    -1,
     441,   172,   309,   174,   306,   295,    -1,   441,   173,   309,
     174,   306,   295,    -1,   441,   201,   204,    69,   335,   295,
      -1,   441,   201,   203,   295,    -1,   441,   202,   203,   295,
      -1,   441,   175,    62,   306,   295,    -1,   441,   180,   175,
      62,   306,   295,    -1,   441,   176,   177,    62,   306,   295,
      -1,   441,   178,    62,   306,   295,    -1,   441,   179,   295,
      -1,   441,   179,   306,   295,    -1,   441,   182,   183,   184,
     295,    -1,   441,   212,   213,   184,   295,    -1,   441,   212,
     213,   214,   295,    -1,   441,   212,   213,   215,   295,    -1,
     441,   222,    73,   295,    -1,   441,   222,   223,   295,    -1,
     441,   186,   187,   308,   295,    -1,   441,   221,   187,   308,
     295,    -1,   441,   210,   211,   308,   295,    -1,   441,   185,
     195,   306,   295,    -1,   441,   185,   194,   306,   295,    -1,
     441,   155,   100,   309,   295,    -1,   441,   188,   189,    62,
     306,   295,    -1,   441,    45,   190,    62,   306,   295,    -1,
     441,    45,   191,    62,   306,   299,   306,   295,    -1,   441,
     123,   192,    45,   308,   295,    -1,   441,   124,   115,   216,
     308,   295,    -1,   441,   124,   205,   308,   295,    -1,   441,
     208,   308,   295,    -1,   441,   206,   207,   308,   295,    -1,
     441,    81,    19,   295,    -1,   441,   115,   209,   306,   295,
      -1,   441,    83,   308,   295,    -1,   441,   217,   218,   308,
     295,    -1,   441,   221,    67,   344,   295,    -1,   340,   224,
      -1,   442,   341,   297,    -1,   443,   444,   295,    -1,   342,
      -1,   230,   308,    -1,   265,   308,    -1,   265,   308,   209,
     306,    -1,   232,   306,    -1,   446,    -1,   225,   335,    -1,
     445,   297,   447,   298,    -1,    -1,   447,   448,   295,    -1,
     231,   233,   306,    -1,   231,   308,    -1,   254,   297,   458,
     298,    -1,   263,   449,    -1,    74,   469,    -1,   453,    -1,
     450,   297,   451,   298,    -1,   450,    -1,   310,    -1,    -1,
     451,   452,   295,    -1,   255,   308,    -1,   114,   308,    -1,
     233,   306,    -1,   456,   297,   454,   298,    -1,   456,    -1,
      -1,   454,   455,   295,    -1,    -1,   235,   306,    -1,   234,
     306,    -1,   236,   189,   306,    -1,   191,   306,    -1,   238,
     306,    -1,   238,    91,   306,    -1,   249,    78,    -1,   249,
     248,    -1,   249,   251,    -1,   361,    -1,   256,   258,   335,
      -1,   233,   306,    -1,   235,   306,    -1,   253,   306,    -1,
     234,   306,    -1,   191,   306,    -1,   238,   306,    -1,   238,
      91,   306,    -1,   239,   240,    -1,   239,   241,    -1,   239,
     242,    -1,   239,   243,    -1,   239,   244,    -1,   239,   245,
      -1,   239,   246,    -1,   239,   247,    -1,   236,   189,   306,
      -1,   237,   306,    -1,   250,   242,   308,    -1,   231,   308,
      -1,   257,   258,   308,    -1,   265,   266,   306,    -1,   120,
     297,   462,   298,    -1,   249,    78,    -1,   249,   248,    -1,
     249,   251,    -1,   259,   260,   261,    -1,   259,   260,   262,
      -1,   259,   260,   306,    -1,   361,    -1,    -1,   458,   459,
      -1,   460,    -1,   461,    -1,   310,   295,    -1,   310,   255,
     295,    -1,    -1,   462,   463,    -1,   464,    -1,   465,    -1,
      17,   295,    -1,    17,   252,   295,    -1,    -1,    -1,   467,
     457,   295,    -1,    -1,   297,   467,   298,    -1,   466,   350,
     468,    -1,    19,    -1,    -1,   112,   224,   370,     3,    -1,
     112,   224,   120,   370,   470,     3,    -1,   112,   224,    74,
     370,   470,     3,    -1,   112,   224,   267,   370,   470,     3,
      -1,   112,   224,   267,    39,   370,   470,     3,    -1,   112,
     224,   268,   370,   470,     3,    -1,   112,   224,   268,    39,
     370,   470,     3,    -1,   112,   224,   264,   370,   470,     3,
      -1,   340,   269,    -1,   479,   341,   297,    -1,   480,   342,
     295,    -1,   480,   270,    67,    18,   295,    -1,   480,   271,
     272,   295,    -1,   480,   271,   273,   295,    -1,   340,   274,
      -1,   481,   341,   297,    -1,   482,   342,   295,    -1,   482,
     275,   306,   295,    -1,   482,   101,   306,   295,    -1,   482,
     276,   306,   295,    -1,   482,   277,    62,   306,   295,    -1,
     482,   278,    62,   306,   295,    -1,   482,   249,   483,   295,
      -1,   482,   361,   295,    -1,   482,   285,   287,   295,    -1,
     482,   285,   173,   295,    -1,   482,   285,   286,   295,    -1,
     482,    74,   489,   295,    -1,   283,    -1,   284,    -1,    78,
      -1,   240,    -1,   279,    -1,   280,    -1,   281,    -1,   282,
      -1,    -1,   187,   306,    -1,   271,   484,    -1,    -1,   486,
     485,   295,    -1,    -1,   297,   486,   298,    -1,    -1,   488,
     350,   487,    -1,   340,   290,    -1,   490,   341,   297,    -1,
     491,   342,   295,    -1,   491,   257,   258,   308,   295,    -1,
     491,   495,   295,    -1,   115,   310,    -1,   181,   309,    -1,
     493,   266,   306,    -1,   493,    -1,   494,   493,    -1,   492,
     181,   309,    -1,   492,   181,    19,    -1,   492,   292,   494,
      -1,   492,   214,    -1,   492,   132,    -1,   492,   291,    -1,
     112,   290,   370,     3,    -1,   295,    -1,   307,    -1,   315,
      -1,   321,    -1,   324,    -1,   334,    -1,   336,    -1,   339,
      -1,   499,    -1,   345,    -1,   398,    -1,   400,    -1,   410,
      -1,   325,    -1,   326,    -1,   327,    -1,   366,    -1,   367,
      -1,   368,    -1,   369,    -1,   371,    -1,   372,    -1,   373,
      -1,   376,    -1,   377,    -1,   378,    -1,   379,    -1,   380,
      -1,   381,    -1,   382,    -1,   383,    -1,   384,    -1,   387,
      -1,   388,    -1,   389,    -1,   390,    -1,   391,    -1,   392,
      -1,   393,    -1,   394,    -1,   395,    -1,   471,    -1,   472,
      -1,   473,    -1,   474,    -1,   475,    -1,   476,    -1,   477,
      -1,   478,    -1,   496,    -1,   500,   298,    -1,   501,   298,
      -1,   352,   298,    -1,   441,   298,    -1,   443,   298,    -1,
     480,   298,    -1,   482,   298,    -1,   491,   298,    -1,   329,
     341,   297,    -1,   500,   342,   295,    -1,   500,   330,   295,
      -1,   500,   333,   295,    -1,   331,   341,   297,    -1,   501,
     342,   295,    -1,   501,   332,   295,    -1,   111,    -1,     5,
      -1,   196,    -1,   193,    -1,   197,    -1,   195,    -1,   194,
      -1,   198,    -1,   199,    -1,   200,    -1,   219,    -1,   220,
      -1,   226,    -1,   227,    -1,   228,    -1,   229,    -1,   288,
      -1,   289,    -1
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
     633,   645,   651,   652,   653,   664,   666,   668,   672,   673,
     674,   675,   682,   690,   694,   700,   706,   708,   712,   713,
     714,   715,   716,   717,   718,   719,   723,   724,   725,   726,
     730,   738,   739,   747,   755,   756,   757,   761,   762,   766,
     771,   772,   779,   788,   789,   790,   794,   803,   809,   810,
     811,   815,   816,   820,   821,   822,   823,   824,   825,   831,
     832,   833,   837,   838,   842,   843,   849,   850,   853,   855,
     859,   860,   864,   882,   883,   884,   885,   886,   887,   895,
     898,   901,   904,   908,   909,   912,   915,   918,   922,   928,
     934,   941,   946,   951,   956,   960,   964,   974,   982,   986,
     993,   994,   997,  1001,  1003,  1005,  1007,  1009,  1011,  1013,
    1016,  1022,  1023,  1024,  1028,  1029,  1035,  1037,  1039,  1041,
    1043,  1045,  1049,  1053,  1056,  1060,  1061,  1062,  1066,  1067,
    1068,  1076,  1076,  1086,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1120,  1131,  1132,  1139,  1140,
    1147,  1156,  1160,  1164,  1188,  1189,  1193,  1206,  1206,  1222,
    1223,  1226,  1227,  1231,  1234,  1243,  1247,  1248,  1249,  1250,
    1254,  1260,  1266,  1272,  1278,  1284,  1291,  1296,  1304,  1305,
    1309,  1316,  1317,  1318,  1319,  1326,  1327,  1330,  1331,  1336,
    1348,  1349,  1353,  1354,  1358,  1359,  1360,  1361,  1362,  1366,
    1367,  1368,  1372,  1385,  1386,  1387,  1388,  1389,  1390,  1391,
    1392,  1393,  1394,  1395,  1405,  1409,  1432,  1466,  1468,  1469,
    1470,  1471,  1472,  1473,  1474,  1478,  1479,  1480,  1481,  1482,
    1483,  1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,
    1493,  1495,  1496,  1497,  1499,  1501,  1503,  1505,  1506,  1507,
    1508,  1509,  1519,  1520,  1521,  1522,  1523,  1528,  1551,  1552,
    1553,  1554,  1555,  1556,  1560,  1563,  1564,  1565,  1574,  1581,
    1590,  1591,  1595,  1601,  1611,  1620,  1626,  1631,  1638,  1643,
    1649,  1650,  1651,  1659,  1661,  1662,  1663,  1669,  1691,  1692,
    1693,  1694,  1695,  1701,  1702,  1703,  1704,  1705,  1706,  1707,
    1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,
    1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,  1727,
    1728,  1729,  1730,  1731,  1732,  1733,  1734,  1744,  1755,  1756,
    1760,  1761,  1762,  1763,  1764,  1765,  1768,  1779,  1782,  1784,
    1788,  1789,  1790,  1791,  1792,  1793,  1797,  1798,  1802,  1810,
    1812,  1816,  1817,  1818,  1822,  1823,  1826,  1828,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1844,
    1866,  1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,
    1876,  1877,  1878,  1879,  1880,  1881,  1882,  1883,  1884,  1885,
    1886,  1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,  1897,
    1899,  1903,  1904,  1906,  1915,  1925,  1927,  1931,  1932,  1934,
    1943,  1954,  1974,  1976,  1979,  1981,  1985,  1989,  1990,  1994,
    1997,  2000,  2005,  2008,  2013,  2016,  2019,  2025,  2033,  2034,
    2035,  2040,  2041,  2047,  2054,  2055,  2056,  2057,  2058,  2059,
    2060,  2061,  2062,  2063,  2064,  2065,  2066,  2070,  2071,  2072,
    2077,  2078,  2079,  2080,  2081,  2084,  2085,  2086,  2089,  2091,
    2094,  2096,  2100,  2109,  2116,  2123,  2124,  2125,  2126,  2129,
    2138,  2145,  2152,  2153,  2157,  2161,  2167,  2170,  2171,  2172,
    2175,  2182,  2182,  2182,  2182,  2182,  2182,  2182,  2182,  2182,
    2182,  2182,  2182,  2182,  2183,  2183,  2183,  2183,  2183,  2183,
    2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,
    2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,
    2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,  2183,
    2183,  2184,  2184,  2184,  2184,  2184,  2184,  2184,  2184,  2185,
    2185,  2185,  2185,  2186,  2186,  2186,  2187,  2188,  2188,  2189,
    2190,  2191,  2192,  2193,  2194,  2195,  2196,  2197,  2198,  2198,
    2198,  2198,  2198,  2198
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
  "ASYNC", "TABLE", "ROUTER", "ID", "PROTOCOL", "PREFERENCE", "DISABLED",
  "DIRECT", "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE", "STATES",
  "FILTERS", "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS",
  "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR",
  "COMMANDS", "PREEXPORT", "GENERATE", "LISTEN", "BGP", "V6ONLY", "DUAL",
  "ADDRESS", "PORT", "PASSWORDS", "DESCRIPTION", "RELOAD", "IN", "OUT",
  "MRTDUMP", "MESSAGES", "RESTRICT", "MEMORY", "IGP_METRIC", "SHOW",
  "STATUS", "SUMMARY", "ROUTE", "SYMBOLS", "DUMP", "RESOURCES", "SOCKETS",
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
  "IGP", "GATEWAY", "RECURSIVE", "OSPF", "AREA", "OSPF_METRIC1",
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
     534,   535,   536,    40,    41,    59,    58,   123,   125,    44,
      63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   303,   304,   304,   305,   305,   306,   306,   306,   307,
     307,   308,   308,   308,   308,   308,   308,   309,   309,   310,
     311,   311,   312,   312,   313,   314,   314,   315,   316,   316,
     317,   317,   317,   318,   318,   319,   319,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   321,   321,   322,   322,
     322,   322,   323,   323,   323,   323,   324,   325,   326,   327,
     328,   328,   329,   330,   330,   330,   330,   331,   332,   332,
     333,   334,   335,   335,   335,   336,   337,   337,   338,   338,
     338,   338,   339,   340,   341,   341,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   343,   343,   343,   343,
     344,   345,   345,   346,   347,   347,   347,   348,   348,   349,
     350,   350,   351,   352,   352,   352,   353,   354,   355,   355,
     355,   356,   356,   357,   357,   357,   357,   357,   357,   358,
     358,   358,   359,   359,   360,   360,   361,   361,   362,   362,
     363,   363,   364,   365,   365,   365,   365,   365,   365,   366,
     367,   368,   369,   370,   370,   371,   372,   373,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     375,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   385,   385,   386,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   396,   396,   397,   397,
     397,   399,   398,   400,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   402,   403,   403,   404,   404,
     405,   406,   406,   407,   408,   408,   409,   411,   410,   412,
     412,   413,   413,   414,   414,   415,   416,   416,   416,   416,
     417,   417,   417,   417,   417,   417,   417,   417,   418,   418,
     419,   420,   420,   420,   420,   421,   421,   422,   422,   422,
     423,   423,   424,   424,   425,   425,   425,   425,   425,   426,
     426,   426,   427,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   429,   430,   431,   432,   432,   432,
     432,   432,   432,   432,   432,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   434,   434,
     434,   434,   434,   434,   435,   436,   436,   436,   437,   437,
     438,   438,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   440,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   442,   443,   443,
     444,   444,   444,   444,   444,   444,   445,   446,   447,   447,
     448,   448,   448,   448,   448,   448,   449,   449,   450,   451,
     451,   452,   452,   452,   453,   453,   454,   454,   455,   455,
     455,   455,   455,   455,   455,   455,   455,   455,   455,   456,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   457,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   457,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   458,
     458,   459,   459,   460,   461,   462,   462,   463,   463,   464,
     465,   466,   467,   467,   468,   468,   469,   470,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   480,
     480,   480,   480,   481,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   483,   483,   483,
     484,   484,   484,   484,   484,   485,   485,   485,   486,   486,
     487,   487,   488,   489,   490,   491,   491,   491,   491,   492,
     493,   493,   494,   494,   495,   495,   495,   495,   495,   495,
     496,   497,   497,   497,   497,   497,   497,   497,   497,   497,
     497,   497,   497,   497,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   499,   499,   499,   499,   499,   499,   499,   499,   500,
     500,   500,   500,   501,   501,   501,   502,   502,   502,   502,
     502,   502,   502,   502,   502,   502,   502,   502,   502,   502,
     502,   502,   502,   502
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
       5,     5,     5,     5,     6,     6,     8,     6,     6,     5,
       4,     5,     4,     5,     4,     5,     5,     2,     3,     3,
       1,     2,     2,     4,     2,     1,     2,     4,     0,     3,
       3,     2,     4,     2,     2,     1,     4,     1,     1,     0,
       3,     2,     2,     2,     4,     1,     0,     3,     0,     2,
       2,     3,     2,     2,     3,     2,     2,     2,     1,     3,
       2,     2,     2,     2,     2,     2,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     2,     3,     2,     3,
       3,     4,     2,     2,     2,     3,     3,     3,     1,     0,
       2,     1,     1,     2,     3,     0,     2,     1,     1,     2,
       3,     0,     0,     3,     0,     3,     3,     1,     0,     4,
       6,     6,     6,     7,     6,     7,     6,     2,     3,     3,
       5,     4,     4,     2,     3,     3,     4,     4,     4,     5,
       5,     4,     3,     4,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     2,     2,     0,     3,
       0,     3,     0,     3,     2,     3,     3,     5,     3,     2,
       2,     3,     1,     2,     3,     3,     3,     2,     2,     2,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   564,   565,   566,   567,
     568,   569,   570,   571,   572,   573,   574,   575,   576,   577,
     578,   579,   580,   581,   582,   583,   584,   585,   586,   587,
     588,   589,   590,   591,   592,   593,   594,   595,   596,   597,
     598,   599,   600,     3,     1,     2,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   284,   551,   552,
     553,   554,   555,    84,    84,   556,   557,   558,     0,   560,
      84,    86,   561,   562,   563,    84,    86,    84,    86,    84,
      86,    84,    86,    84,    86,     5,   559,    86,    86,   195,
     197,   196,     0,    61,    60,     0,    59,     0,     0,     0,
       0,   194,   199,     0,     0,     0,   158,   154,   154,   154,
       0,     0,     0,     0,     0,     0,     0,   183,   182,   181,
     184,     0,     0,     0,     0,    30,    29,    32,     0,     0,
       0,    51,    50,    49,    48,     0,     0,    82,     0,   201,
      76,     0,     0,   227,   268,   273,   282,   279,   235,   286,
     276,     0,     0,   271,   284,   314,   274,   275,     0,     0,
       0,     0,   284,     0,   277,   278,   283,   313,   312,     0,
     311,   203,    85,     0,     0,    62,    67,   112,   357,   397,
     497,   503,   534,     0,     0,     0,     0,     0,    16,   116,
       0,     0,     0,     0,   603,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    16,
       0,     0,     0,     0,     0,   604,     0,     0,     0,    16,
       0,    16,   605,   400,     0,     0,   405,     0,     0,     0,
     606,     0,     0,   532,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   607,     0,     0,   137,   141,     0,     0,
       0,   608,     0,     0,     0,     0,    16,     0,    16,     0,
     601,     0,     0,     0,     0,    26,   602,     0,     0,   119,
     118,     0,     0,     0,    57,     0,     0,   189,   130,   129,
       0,     0,   198,   200,   199,     0,   155,     0,   150,   149,
       0,   153,     0,   154,   154,   154,   154,   154,     0,     0,
     178,   179,   175,   173,   174,   176,   177,   185,     0,   186,
     187,   188,     0,     0,    31,    33,     0,     0,   101,     6,
       8,   284,   102,    52,     0,    56,    72,    73,    74,     0,
     216,     0,    47,     0,     0,   268,   286,   268,   284,   268,
     268,     0,   260,     0,   284,     0,     0,   271,   271,     0,
     309,   284,   284,   284,   284,     0,   239,   237,   284,   236,
     238,   246,   248,     0,   251,   255,     0,   617,   287,   616,
     288,   289,   290,   291,   292,   293,   294,   619,   622,   621,
     618,   620,   623,   624,   625,   626,   627,   628,   629,   630,
     631,   632,   633,   315,   316,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,     0,   609,
     613,   113,    89,   100,    93,     0,    87,    12,    14,    13,
      15,    11,    88,   103,    98,   216,    99,   284,    91,    97,
      92,    95,    90,   114,   115,   358,     0,     0,     0,     0,
       0,     0,     0,    16,     0,    17,    18,     0,     0,     0,
       0,     0,     0,   370,     0,     0,     0,     0,     0,    16,
       0,     0,     0,     0,    16,     0,    16,     0,    16,     0,
      16,     0,     0,   359,   398,   406,   401,   404,   402,   399,
     408,   498,     0,     0,     0,   499,   504,   103,     0,   142,
       0,   138,   519,   517,   518,     0,     0,     0,     0,     0,
       0,     0,     0,   505,   512,   143,   535,     0,   539,    16,
     536,   548,     0,   547,   549,     0,   538,     0,    63,     0,
      65,    16,   611,   612,   610,     0,    25,     0,   615,   614,
     124,   123,   125,   127,   128,   126,     0,   121,   192,    58,
     190,   191,   134,   135,     0,   132,   193,     0,   151,   156,
     157,   164,     0,     0,   171,   216,   165,   168,   169,     0,
     170,   159,     0,   163,   172,   488,   488,   488,   154,   488,
     154,   488,   489,   550,   180,     0,     0,    28,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,    35,    27,
       0,     0,    54,    55,    71,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,     0,     0,     0,   202,   220,
      80,    81,     0,     0,    75,    77,    46,     0,   216,   264,
     265,     0,   266,   267,   262,   250,   338,   341,     0,   322,
     323,   269,   270,   263,     0,     0,     0,     0,   295,   284,
     273,   286,     0,   284,     0,     0,     0,   280,   252,   253,
       0,     0,   281,   307,   305,   303,   300,   301,   302,   304,
     306,   308,   296,   297,   298,   299,   317,     0,   318,   320,
     321,    94,   107,   110,   117,   221,   222,    96,   223,     0,
       0,   392,   394,     0,    16,    16,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,     0,     0,     0,     0,
       0,   364,     0,   365,     0,   390,     0,     0,     0,     0,
       0,     0,     0,   376,   377,     0,     0,     0,   501,   502,
     530,   516,   507,     0,     0,   511,   506,   508,     0,     0,
     514,   515,   513,     0,     0,     0,     0,     0,     0,    19,
       0,   545,   544,     0,   542,   546,    70,    64,    66,    68,
      21,    20,    69,   120,     0,   131,     0,   152,   161,   167,
     162,   160,   166,   487,     0,     0,     0,   488,     0,   488,
       0,    10,     9,    34,     0,     7,    53,   215,   214,   216,
     284,    78,    79,   225,   218,     0,   228,   261,   284,   327,
     310,   284,   284,   284,     0,     0,     0,     0,     0,   235,
     249,     0,     0,   256,   284,   108,     0,   103,     0,     0,
     393,     0,     0,   389,   383,   360,     0,     0,   366,     0,
     369,     0,   372,   382,   381,   378,     0,     0,   391,   380,
     373,   374,   375,   395,   396,   379,   403,   481,    16,     0,
       0,     0,   407,     0,   415,   425,   500,   528,   533,   136,
     138,   509,   510,     0,     0,     0,     0,     0,   140,    22,
      23,   537,   540,     0,   543,   122,   133,   491,   490,   496,
       0,   492,     0,   494,    36,   217,     0,   331,     0,   332,
     333,     0,   284,   329,   330,   328,   284,   284,     0,   284,
       0,     0,   284,   231,   224,     0,   339,     0,     0,     0,
     272,     0,     0,   295,     0,     0,   247,     0,     0,   104,
     105,   109,   111,   385,     0,   387,   388,   361,   362,   368,
     367,   384,   363,   103,   414,     0,   411,   469,     0,   418,
     413,   417,   409,   426,   525,   139,   143,    24,     0,     0,
       0,     0,   541,   493,   495,   284,   284,   284,   284,     0,
       0,     0,   226,   232,     0,     0,   351,   334,   336,     0,
     219,   325,   326,   324,   243,   245,   244,     0,   240,     0,
     319,   106,     0,   484,   410,     0,   439,   419,   428,     0,
       0,   531,     0,   148,   143,   143,   143,   143,     0,     0,
       0,     0,   345,   284,   257,   284,   284,     0,   284,   350,
       0,     0,   254,   386,   482,   486,   412,     0,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,   424,   438,
       0,   526,   520,   521,   522,   523,   524,   527,   529,   144,
     145,   146,   147,   344,   285,   348,     0,   284,   342,   233,
       0,     0,     0,     0,     0,     0,     0,   337,     0,     0,
       0,     0,   473,    16,     0,    16,   416,     0,   432,   430,
     429,     0,     0,   433,   435,   436,   437,   427,     0,     0,
     284,   284,   352,     0,   347,   346,   284,   284,   284,   353,
     241,     0,     0,     0,    16,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   485,   468,
       0,   474,   422,   423,   421,   420,   431,   434,   349,   234,
     343,   259,   284,     0,     0,     0,     0,   475,   444,   458,
     440,   443,   441,     0,   456,     0,   445,   447,   448,   449,
     450,   451,   452,   453,   454,   462,   463,   464,    16,   442,
      16,     0,     0,   483,   258,     0,     0,     0,     0,     0,
     455,   446,   457,   459,   465,   466,   467,   460,   355,   356,
     354,     0,     0,   461,   476,   477,   478,   242,     0,   479,
     480
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   441,    69,   442,   527,   771,   772,   759,
     958,   547,    70,   334,   138,   337,   607,   608,    71,   145,
     146,    72,    16,    17,    18,   105,    73,   281,    74,   287,
     282,    75,   349,    76,   351,   635,    77,    78,   183,   205,
     448,   434,    79,   692,   931,   826,   693,   694,    80,    81,
     443,   206,   292,   556,   557,   301,   564,   565,   265,   743,
     266,   267,   756,    19,    20,    21,    22,   312,    23,    24,
      25,   310,   582,    26,    27,    28,    29,    30,    31,    32,
      33,    34,   130,   328,    35,    36,    37,    38,    39,    40,
      41,    42,    43,   102,   305,    82,   350,    83,   625,   626,
     627,   805,   696,   697,   449,   638,   629,    84,   354,   908,
     909,  1058,   174,   381,   382,   383,   175,   385,   386,  1060,
     360,   176,   361,   369,   177,   178,   179,   911,   180,   413,
     646,   912,   978,   979,   647,   648,   913,    85,    86,    87,
      88,   244,   245,   246,   736,   863,   950,   951,  1031,  1077,
     864,   998,  1040,   865,  1120,   995,  1028,  1029,  1030,  1169,
    1184,  1185,  1186,   943,  1070,  1025,   944,   784,    44,    45,
      46,    47,    48,    49,    50,    51,    89,    90,    91,    92,
     515,  1047,  1002,   954,   868,   507,   508,    93,    94,   273,
     764,   765,   274,    52,    95,    53,    96,    97,    98,   414
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -876
static const yytype_int16 yypact[] =
{
      91,  1733,   136,   495,   474,    66,   148,   324,   474,   178,
     496,   617,   405,   474,   474,   474,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,   175,   291,   105,   460,
     181,   211,  -876,   266,   202,   114,   294,   839,  -876,  -876,
    -876,  -876,  -876,   298,   298,  -876,  -876,  -876,  1441,  -876,
     298,  1632,  -876,  -876,  -876,   298,  1452,   298,  1494,   298,
     895,   298,  1476,   298,  1541,  -876,  -876,  1391,  1609,  -876,
    -876,  -876,    37,  -876,   309,   364,  -876,   474,   474,   399,
      44,  -876,   499,    76,   401,   422,  -876,   413,   170,   413,
     433,   472,   476,   509,   511,   513,   541,  -876,  -876,  -876,
     449,   548,   550,   568,   456,  -876,   537,  -876,    30,    37,
       5,  -876,  -876,  -876,  -876,   103,   300,  -876,   431,  -876,
    -876,   327,    44,  -876,     0,  -876,  -876,  -876,   579,   332,
    -876,   464,   471,     1,   839,  -876,  -876,  -876,   345,   369,
     377,   386,   839,    72,  -876,  -876,  -876,  -876,  -876,  1284,
    -876,  1852,  -876,   396,   406,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,   409,    37,   665,   633,     5,    32,  -876,
     292,   292,   692,    44,  -876,   419,   420,   426,    48,   697,
      32,   508,   532,   -68,   625,    63,   228,   667,   549,   668,
       2,   556,   557,   129,   545,   552,    67,   536,   540,    32,
     543,   530,   534,    26,   -34,  -876,   470,   458,   431,    32,
       5,    32,  -876,  -876,   473,   467,  -876,   478,   699,   296,
    -876,   481,   480,  -876,   754,     5,   482,   -29,     5,     5,
     705,   726,  -122,  -876,   494,   500,  -876,   501,   502,   228,
     542,  -876,   506,   442,   515,   741,    32,   731,    32,   749,
    -876,   529,   551,   553,   763,   810,  -876,   555,   571,  -876,
    -876,   748,   827,   838,  -876,   849,   865,  -876,  -876,  -876,
      15,   868,  -876,  -876,   567,   878,  -876,   879,  -876,  -876,
    1719,  -876,   881,   413,   413,   413,   276,   278,   882,   883,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,   884,  -876,
    -876,  -876,    90,   870,  -876,  -876,  1010,   595,  -876,  -876,
    -876,   839,  -876,     5,   546,  -876,  -876,  -876,  -876,   596,
    1450,   287,  -876,   597,   602,     0,  -876,     0,   839,     0,
       0,   886,  -876,   888,   682,   871,   885,     1,     1,   869,
     876,   839,   839,   839,   839,   104,  -876,  -876,   744,  -876,
    -876,   877,  -876,   197,    33,  -876,   237,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   261,  -876,
    -876,  -876,  -876,  -876,  -876,   431,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,   493,  -876,   839,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,   851,   855,   619,   623,
       5,   874,   704,    32,   228,  -876,  -876,     5,   747,   751,
       5,   860,     5,  -876,   628,   872,   745,     5,     5,    32,
     880,   637,   861,   638,    32,   648,    32,   319,    32,   665,
      32,   649,   650,  -876,  -876,  -876,  -876,  -876,   737,  -876,
    -876,  -876,   932,   658,   673,  -876,  -876,  -876,   674,  -876,
     677,   892,  -876,  -876,  -876,   679,   684,   685,     5,     5,
     686,   687,   689,  -876,  -876,   161,  -876,    12,  -876,    32,
    -876,  -876,   372,  -876,  -876,   778,  -876,     5,  -876,     5,
    -876,    32,  -876,  -876,  -876,     5,  -876,   228,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,   346,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,   370,  -876,  -876,   974,  -876,  -876,
    -876,  -876,   969,   985,  -876,   493,  -876,  -876,  -876,   228,
    -876,  -876,   988,  -876,  -876,   989,   989,   989,   413,   989,
     413,   989,  -876,  -876,  -876,   717,   720,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,   378,  -876,  -876,
    1152,   997,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,    56,   722,   723,  -876,  -876,
    -876,  -876,   228,     5,  -876,  -876,  -876,   347,  1450,  -876,
    -876,  1177,  -876,  -876,  -876,  -876,   241,  -876,   725,  -876,
    -876,  -876,  -876,  -876,  1202,   385,   443,   498,  -876,   839,
     729,   -58,   734,   839,   736,  1015,   125,  -876,  -876,  -876,
    1027,  1031,  -876,  1887,  1887,  1887,  1912,  1912,  1887,  1887,
    1887,  1887,   620,   620,   876,   876,  -876,   767,  -876,  -876,
    -876,  -876,  1036,  -876,   764,  -876,  -876,  -876,  1852,     5,
       5,  -876,  -876,   769,    32,    32,   773,   774,   775,     5,
       5,   776,     5,   779,  -876,     5,   780,   781,   787,   788,
       5,  -876,   431,  -876,   789,  -876,   790,   791,   792,   793,
     794,   804,   805,  -876,  -876,     5,   310,   812,  -876,  -876,
      36,  -876,  -876,   811,   813,  -876,  -876,  -876,   815,   816,
    -876,  -876,  -876,     5,   263,   295,   814,     5,   228,  -876,
     818,  -876,  -876,   228,   807,   778,  -876,  -876,  -876,  -876,
      12,  -876,  -876,  -876,   748,  -876,    15,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  1111,  1112,  1121,   989,  1122,   989,
    1124,  -876,  -876,  -876,  1010,  -876,  -876,  -876,  -876,  1450,
     328,  -876,  -876,  -876,  -876,   323,  -876,  -876,   839,  -876,
    -876,   839,   839,   839,  1227,    10,   201,    82,   131,  -876,
    -876,   829,   579,  -876,   839,  -876,   635,  -876,   834,   831,
    -876,   836,   844,  -876,  -876,  -876,   846,   847,  -876,   848,
    -876,   850,  -876,  -876,  -876,  -876,   862,   873,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,    85,   859,
     875,   228,  -876,   887,  -876,   893,  -876,  -876,  -876,  -876,
     892,  -876,  -876,   897,  1125,  1125,  1125,  1125,  -876,  -876,
    -876,  -876,  -876,     5,   807,  -876,  -876,  -876,  -876,  -876,
    1143,  -876,  1161,  -876,  -876,  -876,    -4,  -876,  1146,  -876,
    -876,   896,   839,  -876,  -876,  -876,   839,   839,   898,    39,
    1284,   899,   766,  -876,  -876,  1450,  -876,  1252,  1277,  1302,
    -876,   901,   903,   907,   913,    -6,  -876,  1155,  1327,   228,
    -876,  -876,  -876,  -876,     5,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,     5,  -876,  -876,   431,  -876,
    -876,   917,  -876,  -876,   -37,  -876,   161,  -876,   920,   921,
     922,   923,  -876,  -876,  -876,   839,   682,   839,  -876,   795,
    1756,   664,  -876,  -876,  1149,    71,  -876,  1852,   933,   924,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  1140,  1141,   941,
    -876,  -876,   925,   143,  -876,    24,  -876,  -876,   429,     5,
     469,  -876,   926,  -876,   161,   161,   161,   161,   852,   928,
    1070,   361,  -876,   894,  -876,   839,   839,   436,   766,  -876,
       5,  1150,  -876,  -876,  -876,  -876,  -876,   -75,  -876,  -876,
    -876,   312,     5,     5,     5,  1042,   140,   -30,  -876,  -876,
     945,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,   947,   328,  1096,  -876,
      16,  1095,  1127,   949,   951,   952,   961,  -876,   953,   964,
    1566,   970,  -876,    32,     5,    32,  -876,   971,  -876,  -876,
    -876,     5,     5,  -876,  -876,  -876,  -876,  -876,   972,   948,
     894,    46,  -876,   968,  -876,  -876,   839,   839,   839,  -876,
    -876,     5,   973,     5,    32,     5,     5,     5,  1079,     5,
     142,   750,    -8,  1029,     5,  1011,  1012,  1016,  -876,  -876,
     986,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,    46,  1352,  1377,  1402,   991,  -876,  -876,  -876,
    -876,  -876,  -876,     5,  -876,     5,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,    32,  -876,
      32,   139,     5,  -876,  -876,   996,   998,   999,     5,     4,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  1001,  -100,  -876,  -876,  -876,  -876,  -876,  1002,  -876,
    -876
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -876,  -876,  -876,   -96,  -876,  -203,  -211,  -267,  -567,  -876,
    -217,  -876,  -876,  -876,  -876,  -876,  -876,   512,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  1188,  -876,  -876,  -876,  -876,
    -876,  -876,  -235,  -876,  -876,  -876,  -876,  -876,   867,   946,
    1106,   825,  -876,  -876,  -876,  -876,   488,  -497,  -876,  -876,
    -876,  -876,   -33,  -876,   522,   -18,  -876,   544,  -812,   446,
    -498,  -876,  -428,  -876,  -876,  -876,  -876,  -110,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,   535,  1013,  -876,  -876,  -876,  -876,  -602,
     519,  -876,   981,   746,  1009,  -876,   694,  -876,  -876,  -875,
    -876,   232,  -172,   521,  -655,  -876,  -167,   669,  -876,  -876,
    -876,  -876,   452,   313,  -876,  -876,  -771,  -876,   -82,   432,
     -67,  -876,  -876,   325,   533,   379,  -811,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -372,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
    -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,
     581,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -876,  -828
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -341
static const yytype_int16 yytable[] =
{
     181,   380,   528,   495,   468,   469,   384,   459,   318,   319,
     740,   820,   781,   744,   355,   367,   339,   965,   356,   339,
     340,  1182,  1091,   340,   339,   987,   485,   357,   340,   910,
     339,   376,   377,   819,   340,   804,   496,   921,   498,   491,
     757,   465,   466,   581,   342,  -230,   339,   462,  1084,   512,
     340,   520,  -229,  -230,  -230,  -230,  -230,   896,   668,   669,
    -229,  -229,  -229,  -229,   896,   437,   438,   439,   440,   335,
    1155,   289,   362,   538,   797,   540,   290,   379,   298,   306,
     465,   466,   975,   299,   897,   103,   339,   376,   377,   158,
     340,   897,  1016,   489,   562,     1,   339,   370,   973,   339,
     340,   436,  1017,   340,   339,   375,   338,   595,   340,   924,
     898,   415,   416,   417,   418,   419,   104,   898,   437,   438,
     439,   440,   343,   563,   474,   420,   421,   422,   423,   424,
     425,   426,   427,   151,   353,   428,    54,   463,   910,   339,
     376,   377,   819,   340,   497,   339,   376,   377,   819,   340,
     999,   106,  1188,   339,   339,   344,   339,   340,   340,   510,
     340,   432,   516,   517,   521,   522,   899,   900,   901,   902,
     903,   904,   905,   899,   900,   901,   902,   903,   904,   905,
    1071,   111,  1089,  1056,   906,   452,  1039,   907,   311,   492,
     307,   906,   139,   134,   907,  1189,   798,  1011,   140,   147,
     691,   152,  1059,   585,   586,   587,   589,   591,   415,   416,
     417,   418,   419,   490,   785,   786,  1131,   788,  1085,   790,
    1072,  1086,   420,   421,   422,   423,   424,   425,   426,   427,
     753,  1082,   428,  1145,  1000,   364,   596,   467,   456,   457,
    1156,    -8,   910,  1157,   313,   465,   466,   611,   415,   416,
     417,   418,   419,   707,   513,   514,   754,  1164,  1119,   930,
     706,  1001,   420,   421,   422,   423,   424,   425,   426,   427,
     481,   482,   428,   362,   610,   362,   719,   362,   362,  1059,
     148,   724,   664,   726,   149,   730,   910,   732,   988,   966,
     314,   641,   755,   358,   311,   341,   311,   473,   341,   150,
     359,   368,  1183,   341,   654,   655,   656,   657,   758,   378,
     135,   610,   153,   980,  1092,   588,   182,   590,   945,   910,
     910,   762,  1026,   477,   478,   341,   760,   336,   103,   136,
     670,   444,  -230,   867,   291,   827,   770,  -230,   768,  -229,
     137,   300,    99,   100,  -229,   874,   896,   875,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   910,   991,   101,   703,   378,   387,   294,   770,   445,
     446,   708,   744,   897,   711,   341,   713,   876,   341,   877,
     698,   717,   718,   341,   857,   630,   631,   632,   633,   465,
     466,   761,   415,   416,   417,   418,   419,   686,   658,   898,
    1174,  1175,   297,   659,   308,  1093,   420,   421,   422,   423,
     424,   425,   426,   427,   687,   890,   428,   892,   378,   127,
     688,   801,   748,   749,   341,   309,  1073,   689,   690,   107,
     108,   311,   341,   341,   315,   341,   320,   316,   317,   128,
    1024,   766,   827,   767,   129,   346,   993,   347,   348,   769,
     415,   416,   417,   418,   419,   899,   900,   901,   902,   903,
     904,   905,   447,   327,   420,   421,   422,   423,   424,   425,
     426,   427,   389,   906,   428,   321,   907,   332,   787,   322,
     789,   615,   616,   617,   618,   619,   620,   847,   621,   622,
     623,   624,    99,   100,   380,   923,   666,   141,    55,   667,
     659,   831,   832,   727,   384,   415,   416,   417,   418,   419,
     254,   695,   323,   101,   324,   142,   325,   302,   303,   420,
     421,   422,   423,   424,   425,   426,   427,    56,  1003,   428,
     143,   256,    57,   728,   729,    58,   671,   802,   304,   672,
     808,   858,   109,   110,   326,  1074,    59,   880,   131,   132,
     133,   329,   882,   330,   397,   398,   399,   400,   401,   402,
     403,   404,    60,    61,   859,    62,   860,  1075,   503,   504,
     379,   331,    63,   861,   531,   144,  1049,  1050,  1051,  1052,
     405,   406,   634,   112,   113,   302,   303,   407,   408,   409,
     410,    64,   814,   333,   949,   345,   816,  1063,  1064,   612,
     613,  1065,    65,   828,   829,  1066,   114,   363,   862,   115,
    1076,   116,   117,   836,   837,   770,   839,   914,   915,   841,
    1032,    66,   352,   532,   846,   364,  -229,   615,   616,   617,
     618,   619,   620,   365,   621,   622,   623,   624,   371,   856,
     366,   803,   295,   296,   773,   774,   380,   426,   427,   411,
     412,   428,   465,   466,   929,   946,   533,   873,   959,   960,
     961,   879,   372,  1033,  1034,  1035,    67,  1036,   775,   776,
     373,   415,   416,   417,   418,   419,   793,   794,  1037,   374,
     651,   652,   120,   433,   811,   420,   421,   422,   423,   424,
     425,   426,   427,   429,   154,   428,   155,   156,   157,   158,
     159,   160,   435,   430,   121,   122,   431,   161,   162,  1042,
     163,   451,   164,   996,   453,   454,   458,   460,   770,   922,
     118,   925,   379,   455,   461,   464,   471,  1038,  1027,   470,
     472,   475,   479,   534,   535,   123,   124,   125,   126,   483,
     476,   480,   812,   487,   917,   918,   919,   484,  1043,  1044,
    1045,  1046,   488,   165,   486,   494,   154,   928,   660,   156,
     157,   158,   661,   160,   500,   493,   502,   518,   499,   161,
     162,   662,   163,   509,   164,   501,   505,   506,   154,   511,
     155,   156,   157,   158,   159,   160,   119,   962,   519,   523,
      68,   161,   162,   539,   163,   524,   164,   813,   525,   526,
     529,   530,   415,   416,   417,   418,   419,   639,   537,   640,
     536,   642,   643,   550,   541,   165,   420,   421,   422,   423,
     424,   425,   426,   427,   542,   545,   428,   551,   552,   546,
     558,   166,   167,   553,   554,   969,   555,   165,   992,   970,
     971,   559,   168,   169,   170,   977,   543,   171,   544,   994,
     548,   154,   560,   155,   156,   157,   158,   159,   160,   415,
     416,   417,   418,   419,   161,   162,   549,   163,   561,   164,
    1122,   566,  1124,   420,   421,   422,   423,   424,   425,   426,
     427,   568,   569,   428,   584,   592,   593,   594,   380,   597,
     609,   614,   636,   166,   167,   637,   649,   653,  1008,   644,
    1010,  1139,   645,  1041,   168,   169,   170,   428,   665,   171,
     165,   650,   896,   699,   701,   166,   167,   700,   702,   704,
     705,   709,   712,   714,  1068,   710,   168,   169,   170,   716,
     722,   171,   721,   723,   715,   194,  1078,  1079,  1080,   897,
    1083,   184,   720,   725,   733,   734,   735,   193,  1061,  1062,
     737,   977,   207,   738,   237,  1172,   247,  1173,   252,   763,
     268,  1014,   195,   196,   379,   898,   197,   198,   739,   741,
     200,   201,   742,   254,   745,   172,  -340,   777,  1123,   746,
     747,   750,   751,   173,   752,  1126,  1127,   778,   166,   167,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,   202,   168,
     169,   170,   203,   779,   171,  1136,   782,  1138,   783,  1140,
    1141,  1142,   791,  1144,  1146,   792,   796,   799,  1159,   809,
     800,   899,   900,   901,   902,   903,   904,   905,    -6,  1133,
    1134,  1135,   236,   815,   243,   817,   251,   663,   264,   906,
     272,   821,   907,   283,   288,   173,   818,  1170,   822,  1171,
     598,   599,   600,   601,   602,   603,   604,   605,   606,   172,
     824,  -335,   825,   827,   830,  1176,  1177,   173,   833,   834,
     835,   838,  1181,   883,   840,   842,   843,   415,   416,   417,
     418,   419,   844,   845,   848,   849,   850,   851,   852,   853,
    1012,   420,   421,   422,   423,   424,   425,   426,   427,   854,
     855,   428,   415,   416,   417,   418,   419,   866,   870,   869,
     871,   872,   878,   881,   887,   888,   420,   421,   422,   423,
     424,   425,   426,   427,   889,   891,   428,   893,   927,   933,
     934,   935,   172,   948,   415,   416,   417,   418,   419,   936,
     173,   937,   938,   939,   957,   940,   963,  1053,   420,   421,
     422,   423,   424,   425,   426,   427,   947,   941,   428,   415,
     416,   417,   418,   419,   964,   248,   249,   967,   942,   989,
    1015,  1020,  1021,   420,   421,   422,   423,   424,   425,   426,
     427,  1069,   952,   428,   415,   416,   417,   418,   419,   968,
     953,  1057,   956,   250,   976,   984,   972,   985,   420,   421,
     422,   423,   424,   425,   426,   427,    -7,   986,   428,   415,
     416,   417,   418,   419,   997,  1004,  1005,  1006,  1007,  1019,
    1023,  1048,  1054,   420,   421,   422,   423,   424,   425,   426,
     427,  1081,  1018,   428,   415,   416,   417,   418,   419,  1022,
    1087,  1088,  1096,  1090,  1097,  1098,  1129,  1100,   420,   421,
     422,   423,   424,   425,   426,   427,  1099,  1101,   428,   415,
     416,   417,   418,   419,  1132,  1121,  1125,  1128,  1143,  1160,
    1137,  1158,  1161,   420,   421,   422,   423,   424,   425,   426,
     427,  1163,  1162,   428,   415,   416,   417,   418,   419,   387,
    1168,  1178,   293,  1179,  1180,  1187,   885,  1190,   420,   421,
     422,   423,   424,   425,   426,   427,   894,   450,   428,   415,
     416,   417,   418,   419,   731,   932,   955,   567,   895,   583,
     886,   780,  1130,   420,   421,   422,   423,   424,   425,   426,
     427,   628,   806,   428,   415,   416,   417,   418,   419,   926,
     823,   916,   974,  1067,     0,  1009,   884,     0,   420,   421,
     422,   423,   424,   425,   426,   427,     0,     0,   428,   415,
     416,   417,   418,   419,     0,  1055,   388,     0,     0,     0,
       0,     0,     0,   420,   421,   422,   423,   424,   425,   426,
     427,     0,     0,   428,   415,   416,   417,   418,   419,     0,
    1094,     0,     0,     0,     0,   389,     0,     0,   420,   421,
     422,   423,   424,   425,   426,   427,     0,     0,   428,   415,
     416,   417,   418,   419,     0,     0,     0,     0,     0,     0,
       0,     0,  1095,   420,   421,   422,   423,   424,   425,   426,
     427,   194,     0,   428,     0,   390,   391,     0,   392,   393,
     394,   395,   396,     0,     0,     0,   795,     0,     0,     0,
     275,   276,   277,     0,   278,   279,     0,     0,   195,   196,
       0,     0,   197,   198,     0,     0,   200,   201,     0,     0,
       0,   807,     0,     0,     0,     0,     0,   397,   398,   399,
     400,   401,   402,   403,   404,     0,     0,     0,     0,     0,
       0,     0,   194,     0,   202,     0,   810,   208,   203,     0,
     185,     0,     0,   405,   406,   186,     0,     0,     0,     0,
     407,   408,   409,   410,   187,     0,   194,     0,     0,   195,
     196,   920,     0,   197,   198,     0,     0,   200,   201,     0,
       0,     0,     0,   209,   194,   210,     0,     0,   188,     0,
       0,     0,     0,   195,   196,     0,   981,   197,   198,     0,
     253,   200,   201,     0,     0,   202,     0,   254,     0,   203,
       0,   195,   196,     0,     0,   197,   198,   211,     0,   200,
     201,   982,   411,   412,     0,   212,   213,   255,   256,   202,
       0,   194,     0,   203,   615,   616,   617,   618,   619,   620,
       0,   621,   622,   623,   624,     0,   983,   202,     0,     0,
       0,   203,     0,     0,     0,     0,     0,   214,   195,   196,
       0,     0,   197,   198,     0,     0,   200,   201,     0,     0,
       0,   990,     0,     0,   215,   216,     0,   217,   218,     0,
     219,   220,   221,     0,   222,     0,     0,   223,   224,     0,
     225,     0,     0,     0,   202,     0,  1165,   254,   203,   194,
       0,     0,     0,   226,   227,     0,   269,     0,   228,     0,
     229,     0,   230,     0,   231,   189,     0,     0,   256,   232,
     284,  1166,   194,   233,   234,     0,   195,   196,     0,     0,
     197,   198,     0,     0,   200,   201,  1102,     0,     0,   280,
       0,     0,     0,     0,     0,     0,  1167,     0,   285,   195,
     196,     0,     0,   197,   198,     0,   199,   200,   201,     0,
     190,     0,   202,     0,     0,   191,   203,     0,     0,   238,
       0,     0,   570,     0,   239,   257,   240,     0,     0,     0,
       0,   192,     0,     0,     0,   202,   465,   466,     0,   203,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     235,   258,   259,   260,   261,     0,     0,  1103,   571,   241,
       0,   262,     0,   415,   416,   417,   418,   419,     0,     0,
       0,     0,     0,     4,   263,     0,     0,   420,   421,   422,
     423,   424,   425,   426,   427,     0,   572,   428,     0,   573,
       5,     6,   242,     0,     0,   574,   575,  1104,   270,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,     0,     0,   576,   577,
     578,   579,     0,   580,     0,  1112,  1113,     0,     0,  1114,
       0,     0,     0,  1115,     0,  1116,     0,     0,     0,     0,
       0,  1117,     0,     0,     0,     0,     0,     7,     0,   271,
       8,     0,     9,     0,     0,    10,     0,     0,     0,     0,
      11,     0,     0,     0,     0,    12,    13,    14,    15,   415,
     416,   417,   418,   419,  1118,     0,     0,     0,     0,     0,
       0,     0,     0,   420,   421,   422,   423,   424,   425,   426,
     427,     0,     0,   428,     0,     0,     0,     0,     0,   447,
       0,     0,     0,     0,  -341,  -341,  -341,     0,     0,     0,
       0,     0,  1013,     0,     0,     0,     0,   286,  -341,  -341,
    -341,  -341,   424,   425,   426,   427,     0,     0,   428,   415,
     416,   417,     0,     0,     0,     0,     0,     0,     0,     0,
     204,     0,     0,   420,   421,   422,   423,   424,   425,   426,
     427,     0,     0,   428
};

static const yytype_int16 yycheck[] =
{
      67,   173,   269,   238,   215,   216,   173,   210,   118,   119,
     507,   666,   579,   511,    14,    14,    14,    21,    18,    14,
      18,    17,     6,    18,    14,    31,   229,    27,    18,   800,
      14,    15,    16,    17,    18,   637,   239,    27,   241,    73,
      28,    17,    18,   310,   140,     6,    14,   115,    78,    78,
      18,   173,     6,    14,    15,    16,    17,    18,    25,    26,
      14,    15,    16,    17,    18,    33,    34,    35,    36,    39,
      78,    34,   154,   276,    18,   278,    39,   173,    34,     3,
      17,    18,   910,    39,    45,    19,    14,    15,    16,    17,
      18,    45,    21,    67,    79,     4,    14,   164,   909,    14,
      18,   197,    31,    18,    14,   172,   139,    17,    18,    27,
      71,     7,     8,     9,    10,    11,    50,    71,    33,    34,
      35,    36,    19,   108,   220,    21,    22,    23,    24,    25,
      26,    27,    28,    19,   152,    31,     0,   205,   909,    14,
      15,    16,    17,    18,   240,    14,    15,    16,    17,    18,
     187,     3,   252,    14,    14,    52,    14,    18,    18,   255,
      18,   194,   258,   259,   286,   287,   127,   128,   129,   130,
     131,   132,   133,   127,   128,   129,   130,   131,   132,   133,
     255,     3,  1057,  1011,   145,   203,   998,   148,    18,   223,
     114,   145,    87,    18,   148,   295,   140,   968,    93,    18,
     435,    87,  1013,   313,   314,   315,   316,   317,     7,     8,
       9,    10,    11,   187,   586,   587,  1091,   589,   248,   591,
     295,   251,    21,    22,    23,    24,    25,    26,    27,    28,
      69,    91,    31,    91,   271,   293,   332,   174,   190,   191,
     248,   299,  1013,   251,    74,    17,    18,   343,     7,     8,
       9,    10,    11,   464,   283,   284,    95,  1132,  1070,   826,
     463,   298,    21,    22,    23,    24,    25,    26,    27,    28,
     203,   204,    31,   355,   341,   357,   479,   359,   360,  1090,
      69,   484,   378,   486,    18,   488,  1057,   490,   294,   293,
     120,   358,   131,   293,    18,   293,    18,   295,   293,    97,
     300,   300,   298,   293,   371,   372,   373,   374,   296,   293,
      19,   378,    18,   915,   298,    39,    18,    39,   233,  1090,
    1091,   532,   298,   194,   195,   293,   529,   297,    19,    38,
     297,    39,   293,   297,   297,   299,   547,   298,   541,   293,
      49,   297,    18,    19,   298,    82,    18,    84,   415,   416,
     417,   418,   419,   420,   421,   422,   423,   424,   425,   426,
     427,  1132,   929,    39,   460,   293,     5,     3,   579,    77,
      78,   467,   870,    45,   470,   293,   472,    82,   293,    84,
     447,   477,   478,   293,    74,    98,    99,   100,   101,    17,
      18,    19,     7,     8,     9,    10,    11,   136,   294,    71,
     261,   262,     3,   299,     3,  1060,    21,    22,    23,    24,
      25,    26,    27,    28,   153,   787,    31,   789,   293,    14,
     159,   632,   518,   519,   293,     3,   114,   166,   167,   105,
     106,    18,   293,   293,   264,   293,     3,   267,   268,    34,
     297,   537,   299,   539,    39,    14,   943,    16,    17,   545,
       7,     8,     9,    10,    11,   127,   128,   129,   130,   131,
     132,   133,   170,    14,    21,    22,    23,    24,    25,    26,
      27,    28,   111,   145,    31,     3,   148,    21,   588,     3,
     590,   134,   135,   136,   137,   138,   139,   722,   141,   142,
     143,   144,    18,    19,   666,   294,   299,    37,     3,   302,
     299,   704,   705,   184,   671,     7,     8,     9,    10,    11,
      81,    18,     3,    39,     3,    55,     3,    18,    19,    21,
      22,    23,    24,    25,    26,    27,    28,    32,   956,    31,
      70,   102,    37,   214,   215,    40,   299,   633,    39,   302,
     299,   231,     7,     8,     3,   233,    51,   758,    13,    14,
      15,     3,   763,     3,   193,   194,   195,   196,   197,   198,
     199,   200,    67,    68,   254,    70,   256,   255,   272,   273,
     666,     3,    77,   263,   132,   115,  1004,  1005,  1006,  1007,
     219,   220,   295,    87,    88,    18,    19,   226,   227,   228,
     229,    96,   659,    56,   861,   295,   663,   161,   162,    53,
      54,   165,   107,   699,   700,   169,   110,    28,   298,   113,
     298,   115,   116,   709,   710,   826,   712,   294,   295,   715,
     191,   126,   295,   181,   720,   293,   298,   134,   135,   136,
     137,   138,   139,   169,   141,   142,   143,   144,   293,   735,
     169,   294,   107,   108,   298,   299,   818,    27,    28,   288,
     289,    31,    17,    18,    19,   858,   214,   753,   875,   876,
     877,   757,   293,   234,   235,   236,   171,   238,   298,   299,
     293,     7,     8,     9,    10,    11,   298,   299,   249,   293,
     367,   368,    65,    18,   299,    21,    22,    23,    24,    25,
      26,    27,    28,   297,    12,    31,    14,    15,    16,    17,
      18,    19,    69,   297,    87,    88,   297,    25,    26,   240,
      28,    19,    30,   948,   295,   295,    19,   209,   929,   815,
     224,   817,   818,   297,   192,   100,   177,   298,   995,    62,
      62,   175,   187,   291,   292,   118,   119,   120,   121,   203,
     183,   189,   299,   213,   811,   812,   813,   207,   279,   280,
     281,   282,   218,    71,   211,   297,    12,   824,    14,    15,
      16,    17,    18,    19,   297,   295,    67,    62,   295,    25,
      26,    27,    28,    19,    30,   297,   295,   297,    12,   297,
      14,    15,    16,    17,    18,    19,   290,   883,    62,   295,
     295,    25,    26,    62,    28,   295,    30,   299,   297,   297,
     258,   295,     7,     8,     9,    10,    11,   355,    67,   357,
     295,   359,   360,    65,    65,    71,    21,    22,    23,    24,
      25,    26,    27,    28,   295,    62,    31,    79,    80,    19,
       3,   149,   150,    85,    86,   902,    88,    71,   934,   906,
     907,     3,   160,   161,   162,   912,   295,   165,   295,   945,
     295,    12,     3,    14,    15,    16,    17,    18,    19,     7,
       8,     9,    10,    11,    25,    26,   295,    28,     3,    30,
    1073,     3,  1075,    21,    22,    23,    24,    25,    26,    27,
      28,     3,     3,    31,     3,     3,     3,     3,  1060,    19,
     295,   295,   295,   149,   150,   293,    25,    28,   965,    13,
     967,  1104,    14,   999,   160,   161,   162,    31,    31,   165,
      71,    26,    18,    62,   295,   149,   150,    62,   295,    45,
     216,   174,    62,   295,  1020,   174,   160,   161,   162,   184,
      69,   165,   295,   295,    62,    40,  1032,  1033,  1034,    45,
    1036,    74,    62,   295,   295,   295,   209,    80,  1015,  1016,
      18,  1018,    85,   295,    87,  1158,    89,  1160,    91,   181,
      93,   297,    67,    68,  1060,    71,    71,    72,   295,   295,
      75,    76,   295,    81,   295,   293,   294,     3,  1074,   295,
     295,   295,   295,   301,   295,  1081,  1082,    18,   149,   150,
     240,   241,   242,   243,   244,   245,   246,   247,   103,   160,
     161,   162,   107,    18,   165,  1101,    18,  1103,    19,  1105,
    1106,  1107,   295,  1109,  1110,   295,    19,   295,  1114,   294,
     297,   127,   128,   129,   130,   131,   132,   133,   299,  1096,
    1097,  1098,    86,   299,    88,   299,    90,   293,    92,   145,
      94,    14,   148,    97,    98,   301,    31,  1143,    17,  1145,
      40,    41,    42,    43,    44,    45,    46,    47,    48,   293,
     293,   295,    26,   299,   295,  1161,  1162,   301,   295,   295,
     295,   295,  1168,   266,   295,   295,   295,     7,     8,     9,
      10,    11,   295,   295,   295,   295,   295,   295,   295,   295,
     295,    21,    22,    23,    24,    25,    26,    27,    28,   295,
     295,    31,     7,     8,     9,    10,    11,   295,   295,   298,
     295,   295,   298,   295,     3,     3,    21,    22,    23,    24,
      25,    26,    27,    28,     3,     3,    31,     3,   299,   295,
     299,   295,   293,   258,     7,     8,     9,    10,    11,   295,
     301,   295,   295,   295,    19,   295,     3,   295,    21,    22,
      23,    24,    25,    26,    27,    28,   297,   295,    31,     7,
       8,     9,    10,    11,     3,   270,   271,    21,   295,    14,
      21,    31,    31,    21,    22,    23,    24,    25,    26,    27,
      28,    31,   295,    31,     7,     8,     9,    10,    11,   293,
     297,   297,   295,   298,   295,   294,   298,   294,    21,    22,
      23,    24,    25,    26,    27,    28,   299,   294,    31,     7,
       8,     9,    10,    11,   297,   295,   295,   295,   295,   295,
     295,   295,   294,    21,    22,    23,    24,    25,    26,    27,
      28,   189,   299,    31,     7,     8,     9,    10,    11,   298,
     295,   294,   293,   147,   293,   293,   298,   294,    21,    22,
      23,    24,    25,    26,    27,    28,   295,   293,    31,     7,
       8,     9,    10,    11,   296,   295,   295,   295,   189,   258,
     297,   242,   260,    21,    22,    23,    24,    25,    26,    27,
      28,   295,   266,    31,     7,     8,     9,    10,    11,     5,
     299,   295,   104,   295,   295,   294,   774,   295,    21,    22,
      23,    24,    25,    26,    27,    28,   794,   201,    31,     7,
       8,     9,    10,    11,   489,   827,   870,   304,   799,   310,
     776,   575,  1090,    21,    22,    23,    24,    25,    26,    27,
      28,   350,   638,    31,     7,     8,     9,    10,    11,   818,
     671,   808,   910,  1018,    -1,   966,   765,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    31,     7,
       8,     9,    10,    11,    -1,   295,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    31,     7,     8,     9,    10,    11,    -1,
     295,    -1,    -1,    -1,    -1,   111,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    31,     7,
       8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   295,    21,    22,    23,    24,    25,    26,    27,
      28,    40,    -1,    31,    -1,   151,   152,    -1,   154,   155,
     156,   157,   158,    -1,    -1,    -1,   294,    -1,    -1,    -1,
      59,    60,    61,    -1,    63,    64,    -1,    -1,    67,    68,
      -1,    -1,    71,    72,    -1,    -1,    75,    76,    -1,    -1,
      -1,   294,    -1,    -1,    -1,    -1,    -1,   193,   194,   195,
     196,   197,   198,   199,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,   103,    -1,   294,    45,   107,    -1,
      59,    -1,    -1,   219,   220,    64,    -1,    -1,    -1,    -1,
     226,   227,   228,   229,    73,    -1,    40,    -1,    -1,    67,
      68,   294,    -1,    71,    72,    -1,    -1,    75,    76,    -1,
      -1,    -1,    -1,    81,    40,    83,    -1,    -1,    97,    -1,
      -1,    -1,    -1,    67,    68,    -1,   294,    71,    72,    -1,
      74,    75,    76,    -1,    -1,   103,    -1,    81,    -1,   107,
      -1,    67,    68,    -1,    -1,    71,    72,   115,    -1,    75,
      76,   294,   288,   289,    -1,   123,   124,   101,   102,   103,
      -1,    40,    -1,   107,   134,   135,   136,   137,   138,   139,
      -1,   141,   142,   143,   144,    -1,   294,   103,    -1,    -1,
      -1,   107,    -1,    -1,    -1,    -1,    -1,   155,    67,    68,
      -1,    -1,    71,    72,    -1,    -1,    75,    76,    -1,    -1,
      -1,   294,    -1,    -1,   172,   173,    -1,   175,   176,    -1,
     178,   179,   180,    -1,   182,    -1,    -1,   185,   186,    -1,
     188,    -1,    -1,    -1,   103,    -1,   294,    81,   107,    40,
      -1,    -1,    -1,   201,   202,    -1,   115,    -1,   206,    -1,
     208,    -1,   210,    -1,   212,   224,    -1,    -1,   102,   217,
      61,   294,    40,   221,   222,    -1,    67,    68,    -1,    -1,
      71,    72,    -1,    -1,    75,    76,   120,    -1,    -1,   298,
      -1,    -1,    -1,    -1,    -1,    -1,   294,    -1,    89,    67,
      68,    -1,    -1,    71,    72,    -1,    74,    75,    76,    -1,
     269,    -1,   103,    -1,    -1,   274,   107,    -1,    -1,   225,
      -1,    -1,     3,    -1,   230,   249,   232,    -1,    -1,    -1,
      -1,   290,    -1,    -1,    -1,   103,    17,    18,    -1,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     298,   275,   276,   277,   278,    -1,    -1,   191,    39,   265,
      -1,   285,    -1,     7,     8,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    40,   298,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    67,    31,    -1,    70,
      57,    58,   298,    -1,    -1,    76,    77,   231,   257,   233,
     234,   235,   236,   237,   238,   239,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,   249,   250,    -1,    -1,   253,
      -1,    -1,    -1,   257,    -1,   259,    -1,    -1,    -1,    -1,
      -1,   265,    -1,    -1,    -1,    -1,    -1,   104,    -1,   298,
     107,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,    -1,    -1,    -1,   122,   123,   124,   125,     7,
       8,     9,    10,    11,   298,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,   170,
      -1,    -1,    -1,    -1,     7,     8,     9,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   298,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    31,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     298,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   304,   305,    40,    57,    58,   104,   107,   109,
     112,   117,   122,   123,   124,   125,   325,   326,   327,   366,
     367,   368,   369,   371,   372,   373,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   471,   472,   473,   474,   475,   476,
     477,   478,   496,   498,     0,     3,    32,    37,    40,    51,
      67,    68,    70,    77,    96,   107,   126,   171,   295,   307,
     315,   321,   324,   329,   331,   334,   336,   339,   340,   345,
     351,   352,   398,   400,   410,   440,   441,   442,   443,   479,
     480,   481,   482,   490,   491,   497,   499,   500,   501,    18,
      19,    39,   396,    19,    50,   328,     3,   105,   106,   396,
     396,     3,    87,    88,   110,   113,   115,   116,   224,   290,
      65,    87,    88,   118,   119,   120,   121,    14,    34,    39,
     385,   396,   396,   396,    18,    19,    38,    49,   317,    87,
      93,    37,    55,    70,   115,   322,   323,    18,    69,    18,
      97,    19,    87,    18,    12,    14,    15,    16,    17,    18,
      19,    25,    26,    28,    30,    71,   149,   150,   160,   161,
     162,   165,   293,   301,   415,   419,   424,   427,   428,   429,
     431,   433,    18,   341,   341,    59,    64,    73,    97,   224,
     269,   274,   290,   341,    40,    67,    68,    71,    72,    74,
      75,    76,   103,   107,   298,   342,   354,   341,    45,    81,
      83,   115,   123,   124,   155,   172,   173,   175,   176,   178,
     179,   180,   182,   185,   186,   188,   201,   202,   206,   208,
     210,   212,   217,   221,   222,   298,   342,   341,   225,   230,
     232,   265,   298,   342,   444,   445,   446,   341,   270,   271,
     298,   342,   341,    74,    81,   101,   102,   249,   275,   276,
     277,   278,   285,   298,   342,   361,   363,   364,   341,   115,
     257,   298,   342,   492,   495,    59,    60,    61,    63,    64,
     298,   330,   333,   342,    61,    89,   298,   332,   342,    34,
      39,   297,   355,   328,     3,   396,   396,     3,    34,    39,
     297,   358,    18,    19,    39,   397,     3,   114,     3,     3,
     374,    18,   370,    74,   120,   264,   267,   268,   370,   370,
       3,     3,     3,     3,     3,     3,     3,    14,   386,     3,
       3,     3,    21,    56,   316,    39,   297,   318,   355,    14,
      18,   293,   306,    19,    52,   295,    14,    16,    17,   335,
     399,   337,   295,   358,   411,    14,    18,    27,   293,   300,
     423,   425,   431,    28,   293,   169,   169,    14,   300,   426,
     433,   293,   293,   293,   293,   433,    15,    16,   293,   306,
     415,   416,   417,   418,   419,   420,   421,     5,    82,   111,
     151,   152,   154,   155,   156,   157,   158,   193,   194,   195,
     196,   197,   198,   199,   200,   219,   220,   226,   227,   228,
     229,   288,   289,   432,   502,     7,     8,     9,    10,    11,
      21,    22,    23,    24,    25,    26,    27,    28,    31,   297,
     297,   297,   355,    18,   344,    69,   306,    33,    34,    35,
      36,   306,   308,   353,    39,    77,    78,   170,   343,   407,
     343,    19,   358,   295,   295,   297,   190,   191,    19,   308,
     209,   192,   115,   205,   100,    17,    18,   174,   309,   309,
      62,   177,    62,   295,   306,   175,   183,   194,   195,   187,
     189,   203,   204,   203,   207,   308,   211,   213,   218,    67,
     187,    73,   223,   295,   297,   335,   308,   306,   308,   295,
     297,   297,    67,   272,   273,   295,   297,   488,   489,    19,
     306,   297,    78,   283,   284,   483,   306,   306,    62,    62,
     173,   286,   287,   295,   295,   297,   297,   309,   310,   258,
     295,   132,   181,   214,   291,   292,   295,    67,   308,    62,
     308,    65,   295,   295,   295,    62,    19,   314,   295,   295,
      65,    79,    80,    85,    86,    88,   356,   357,     3,     3,
       3,     3,    79,   108,   359,   360,     3,   397,     3,     3,
       3,    39,    67,    70,    76,    77,    89,    90,    91,    92,
      94,   310,   375,   407,     3,   370,   370,   370,    39,   370,
      39,   370,     3,     3,     3,    17,   306,    19,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   319,   320,   295,
     433,   306,    53,    54,   295,   134,   135,   136,   137,   138,
     139,   141,   142,   143,   144,   401,   402,   403,   405,   409,
      98,    99,   100,   101,   295,   338,   295,   293,   408,   425,
     425,   433,   425,   425,    13,    14,   433,   437,   438,    25,
      26,   426,   426,    28,   433,   433,   433,   433,   294,   299,
      14,    18,    27,   293,   306,    31,   299,   302,    25,    26,
     297,   299,   302,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   136,   153,   159,   166,
     167,   335,   346,   349,   350,    18,   405,   406,   433,    62,
      62,   295,   295,   306,    45,   216,   308,   309,   306,   174,
     174,   306,    62,   306,   295,    62,   184,   306,   306,   308,
      62,   295,    69,   295,   308,   295,   308,   184,   214,   215,
     308,   344,   308,   295,   295,   209,   447,    18,   295,   295,
     350,   295,   295,   362,   363,   295,   295,   295,   306,   306,
     295,   295,   295,    69,    95,   131,   365,    28,   296,   312,
     308,    19,   309,   181,   493,   494,   306,   306,   308,   306,
     309,   310,   311,   298,   299,   298,   299,     3,    18,    18,
     406,   311,    18,    19,   470,   470,   470,   370,   470,   370,
     470,   295,   295,   298,   299,   294,    19,    18,   140,   295,
     297,   309,   306,   294,   402,   404,   409,   294,   299,   294,
     294,   299,   299,   299,   433,   299,   433,   299,    31,    17,
     417,    14,    17,   420,   293,    26,   348,   299,   306,   306,
     295,   308,   308,   295,   295,   295,   306,   306,   295,   306,
     295,   306,   295,   295,   295,   295,   306,   335,   295,   295,
     295,   295,   295,   295,   295,   295,   306,    74,   231,   254,
     256,   263,   298,   448,   453,   456,   295,   297,   487,   298,
     295,   295,   295,   306,    82,    84,    82,    84,   298,   306,
     309,   295,   309,   266,   493,   357,   360,     3,     3,     3,
     470,     3,   470,     3,   320,   403,    18,    45,    71,   127,
     128,   129,   130,   131,   132,   133,   145,   148,   412,   413,
     429,   430,   434,   439,   294,   295,   437,   433,   433,   433,
     294,    27,   306,   294,    27,   306,   416,   299,   433,    19,
     311,   347,   349,   295,   299,   295,   295,   295,   295,   295,
     295,   295,   295,   466,   469,   233,   308,   297,   258,   310,
     449,   450,   295,   297,   486,   362,   295,    19,   313,   313,
     313,   313,   306,     3,     3,    21,   293,    21,   293,   433,
     433,   433,   298,   439,   432,   502,   295,   433,   435,   436,
     402,   294,   294,   294,   294,   294,   294,    31,   294,    14,
     294,   311,   306,   350,   306,   458,   335,   297,   454,   187,
     271,   298,   485,   365,   295,   295,   295,   295,   433,   438,
     433,   429,   295,   146,   297,    21,    21,    31,   299,   295,
      31,    31,   298,   295,   297,   468,   298,   310,   459,   460,
     461,   451,   191,   234,   235,   236,   238,   249,   298,   361,
     455,   306,   240,   279,   280,   281,   282,   484,   295,   365,
     365,   365,   365,   295,   294,   295,   502,   297,   414,   439,
     422,   433,   433,   161,   162,   165,   169,   436,   306,    31,
     467,   255,   295,   114,   233,   255,   298,   452,   306,   306,
     306,   189,    91,   306,    78,   248,   251,   295,   294,   412,
     147,     6,   298,   417,   295,   295,   293,   293,   293,   295,
     294,   293,   120,   191,   231,   233,   234,   235,   236,   237,
     238,   239,   249,   250,   253,   257,   259,   265,   298,   361,
     457,   295,   308,   306,   308,   295,   306,   306,   295,   298,
     414,   412,   296,   433,   433,   433,   306,   297,   306,   308,
     306,   306,   306,   189,   306,    91,   306,   240,   241,   242,
     243,   244,   245,   246,   247,    78,   248,   251,   242,   306,
     258,   260,   266,   295,   412,   294,   294,   294,   299,   462,
     306,   306,   308,   308,   261,   262,   306,   306,   295,   295,
     295,   306,    17,   298,   463,   464,   465,   294,   252,   295,
     295
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
#line 645 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 651 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 653 "cf-parse.y"
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
#line 672 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 673 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 674 "cf-parse.y"
    { new_config->listen_bgp_flags = 0; ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 675 "cf-parse.y"
    { new_config->listen_bgp_flags = 1; ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 682 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 694 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 700 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 708 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 712 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 713 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 714 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 715 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 716 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 717 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 718 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 719 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 723 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 725 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 726 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 730 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 738 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 739 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 747 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 755 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 756 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 757 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 761 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 762 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 779 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 794 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 809 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 810 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 811 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 816 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 820 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 821 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 822 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 823 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 824 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 825 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 831 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 832 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 833 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 838 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 842 "cf-parse.y"
    { (yyval.i) = MD_STATES; ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 843 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 864 "cf-parse.y"
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
#line 882 "cf-parse.y"
    { ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 883 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 884 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 885 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 886 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 887 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 896 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 899 "cf-parse.y"
    { cmd_show_memory(); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 902 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 905 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 909 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 913 "cf-parse.y"
    { if_show(); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 916 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 919 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 922 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 928 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 934 "cf-parse.y"
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
#line 941 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 946 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 951 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 956 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 960 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 964 "cf-parse.y"
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
#line 974 "cf-parse.y"
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
#line 982 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 986 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 993 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 994 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 998 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1002 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1004 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1006 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1008 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1010 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1012 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1014 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1016 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1022 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1023 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1028 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1029 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1036 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1038 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1040 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1042 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1044 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1046 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1050 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1054 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1057 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1060 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1061 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1062 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1066 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1067 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1068 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1076 "cf-parse.y"
    { (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1077 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s)->def = (yyvsp[(4) - (4)].f);
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1086 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1090 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1091 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1092 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1093 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1094 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1095 "cf-parse.y"
    { (yyval.i) = T_QUAD; ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1096 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1097 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1098 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1099 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1100 "cf-parse.y"
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
#line 1120 "cf-parse.y"
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
#line 1131 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1132 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1139 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1140 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1147 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1156 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1164 "cf-parse.y"
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
#line 1188 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1189 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1193 "cf-parse.y"
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
#line 1206 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1209 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1222 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1223 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1226 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1227 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1231 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1234 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1243 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1247 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1248 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1249 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1250 "cf-parse.y"
    { (yyval.v).type = (yyvsp[(1) - (1)].i) >> 16; (yyval.v).val.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1254 "cf-parse.y"
    {
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
   ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1260 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (8)].i), (yyvsp[(4) - (8)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (8)].i), (yyvsp[(7) - (8)].i));
   ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1266 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (12)].i), (yyvsp[(4) - (12)].i)); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(9) - (12)].i), (yyvsp[(11) - (12)].i));
   ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1272 "cf-parse.y"
    { 	/* This is probably useless :-) */
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = 0;
	(yyval.e)->to.val.i = 0xffffffff;
   ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1278 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (5)].i), 0); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (5)].i), 0xffff);
   ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1284 "cf-parse.y"
    { 
	(yyval.e) = f_generate_rev_wcard(0, 0xffff, (yyvsp[(4) - (5)].i));
   ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1291 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (1)].v); 
	(yyval.e)->to = (yyvsp[(1) - (1)].v);
   ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1296 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (4)].v); 
	(yyval.e)->to = (yyvsp[(4) - (4)].v); 
   ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1304 "cf-parse.y"
    { (yyval.e) = (yyvsp[(1) - (1)].e); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1305 "cf-parse.y"
    { (yyval.e) = (yyvsp[(3) - (3)].e); (yyval.e)->left = (yyvsp[(1) - (3)].e); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1309 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1316 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1317 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1318 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1319 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1326 "cf-parse.y"
    { (yyval.trie) = f_new_trie(cfg_mem); trie_add_fprefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1327 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_fprefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1330 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1331 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[(2) - (4)].e);
     (yyval.e)->data = (yyvsp[(4) - (4)].x);
     (yyval.e)->left = (yyvsp[(1) - (4)].e);
   ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1336 "cf-parse.y"
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
#line 1348 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1349 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1353 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1354 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1358 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1359 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1360 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1361 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1362 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1366 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1367 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1368 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1372 "cf-parse.y"
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
#line 1385 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1386 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1387 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1388 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1389 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1390 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1391 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1392 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1393 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1394 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1395 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1409 "cf-parse.y"
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
#line 1432 "cf-parse.y"
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
#line 1466 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1468 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1469 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1470 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1471 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1472 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1473 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1474 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1478 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1479 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1480 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1481 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1482 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1483 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1484 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1485 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1486 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1487 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1488 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1489 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1490 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1491 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1492 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1493 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1495 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1496 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1497 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1499 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1501 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1503 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1505 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1506 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1507 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1508 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1509 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1519 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1520 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1521 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1522 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1523 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1528 "cf-parse.y"
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
#line 1551 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1552 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1553 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1554 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1555 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1556 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1560 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1563 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1564 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1565 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1574 "cf-parse.y"
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
#line 1581 "cf-parse.y"
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
#line 1590 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1591 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1595 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1601 "cf-parse.y"
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
#line 1611 "cf-parse.y"
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
#line 1620 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1626 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1631 "cf-parse.y"
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
#line 1638 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1643 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1649 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1650 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1651 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1660 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1661 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1662 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1663 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1669 "cf-parse.y"
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
#line 1693 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1694 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(3) - (6)].a); BGP_CFG->local_as = (yyvsp[(5) - (6)].i); ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1695 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1701 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1702 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1703 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1704 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1705 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1706 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1707 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1708 "cf-parse.y"
    { BGP_CFG->multihop = 64; ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1709 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (4)].i); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1710 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1711 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1712 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1713 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1714 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_DIRECT; ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1715 "cf-parse.y"
    { BGP_CFG->gw_mode = GW_RECURSIVE; ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1716 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1717 "cf-parse.y"
    { BGP_CFG->igp_metric = (yyvsp[(4) - (5)].i); ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1718 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1719 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1720 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1721 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1722 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1723 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1724 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1725 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1726 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1727 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1728 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1729 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1730 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1731 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1732 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1733 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1734 "cf-parse.y"
    { BGP_CFG->igp_table = (yyvsp[(4) - (5)].r); ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1744 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1761 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1762 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (2)].i) ? DEFAULT_ECMP_LIMIT : 0; ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1763 "cf-parse.y"
    { OSPF_CFG->ecmp = (yyvsp[(2) - (4)].i) ? (yyvsp[(4) - (4)].i) : 0; if ((yyvsp[(4) - (4)].i) < 0) cf_error("ECMP limit cannot be negative"); ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1764 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i); if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1768 "cf-parse.y"
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

  case 407:

/* Line 1455 of yacc.c  */
#line 1779 "cf-parse.y"
    { ospf_area_finish(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1788 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1789 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1802 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1816 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1817 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1818 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1822 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1832 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1833 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1834 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1835 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1836 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1837 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1838 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1839 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1840 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1845 "cf-parse.y"
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

  case 440:

/* Line 1455 of yacc.c  */
#line 1866 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1867 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1868 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1869 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1870 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1871 "cf-parse.y"
    { OSPF_PATT->deadint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1872 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1873 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1874 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1875 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1876 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1877 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1878 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1879 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1880 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTMP ; ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1881 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1882 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1883 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1884 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1885 "cf-parse.y"
    { OSPF_PATT->check_link = (yyvsp[(3) - (3)].i); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1886 "cf-parse.y"
    { OSPF_PATT->ecmp_weight = (yyvsp[(3) - (3)].i) - 1; if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("ECMP weight must be in range 1-256"); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1888 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1889 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1890 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1891 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1892 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1893 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) || ((yyvsp[(3) - (3)].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1907 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1916 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1935 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1944 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1954 "cf-parse.y"
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

  case 486:

/* Line 1455 of yacc.c  */
#line 1985 "cf-parse.y"
    { ospf_iface_finish(); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1990 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1995 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1998 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2001 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2006 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2009 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2014 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2017 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2020 "cf-parse.y"
    { ospf_sh_lsadb(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf)); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2025 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2035 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2040 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2041 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2047 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2056 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2057 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2058 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2059 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2060 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2061 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2063 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2064 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2065 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2070 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2071 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2072 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2077 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2078 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2079 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2080 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2081 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2085 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2086 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2100 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2116 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2125 "cf-parse.y"
    { STATIC_CFG->check_link = (yyvsp[(4) - (5)].i); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2129 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2138 "cf-parse.y"
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[(2) - (2)].a);
     this_srt_nh->if_name = (void *) this_srt; /* really */
   ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2145 "cf-parse.y"
    {
     this_srt_nh->masklen = (yyvsp[(3) - (3)].i) - 1; /* really */
     if (((yyvsp[(3) - (3)].i)<1) || ((yyvsp[(3) - (3)].i)>256)) cf_error("Weight must be in range 1-256"); 
   ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2152 "cf-parse.y"
    { this_srt->mp_next = this_srt_nh; ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2153 "cf-parse.y"
    { last_srt_nh->mp_next = this_srt_nh; ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2157 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2161 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2167 "cf-parse.y"
    {
      this_srt->dest = RTD_MULTIPATH;
   ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2170 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2171 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2172 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2176 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2184 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2184 "cf-parse.y"
    { ospf_proto_finish(); ;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2184 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2188 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); ;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2188 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2189 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2190 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2191 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2192 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2193 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2194 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2195 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2196 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2197 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); ;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); ;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); ;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2198 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;



/* Line 1455 of yacc.c  */
#line 7238 "cf-parse.tab.c"
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
#line 2200 "cf-parse.y"

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



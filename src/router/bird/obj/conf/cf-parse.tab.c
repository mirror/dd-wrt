
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

static void
finish_iface_config(struct ospf_iface_patt *ip)
{
  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");
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

static struct static_route *this_srt;



/* Line 189 of yacc.c  */
#line 225 "cf-parse.tab.c"

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
     GEQ = 261,
     LEQ = 262,
     NEQ = 263,
     AND = 264,
     OR = 265,
     PO = 266,
     PC = 267,
     NUM = 268,
     ENUM = 269,
     RTRID = 270,
     IPA = 271,
     SYM = 272,
     TEXT = 273,
     PREFIX_DUMMY = 274,
     DEFINE = 275,
     ON = 276,
     OFF = 277,
     YES = 278,
     NO = 279,
     LOG = 280,
     SYSLOG = 281,
     ALL = 282,
     DEBUG = 283,
     TRACE = 284,
     INFO = 285,
     REMOTE = 286,
     WARNING = 287,
     ERROR = 288,
     AUTH = 289,
     FATAL = 290,
     BUG = 291,
     STDERR = 292,
     SOFT = 293,
     CONFIGURE = 294,
     DOWN = 295,
     KERNEL = 296,
     PERSIST = 297,
     SCAN = 298,
     TIME = 299,
     LEARN = 300,
     DEVICE = 301,
     ASYNC = 302,
     TABLE = 303,
     ROUTER = 304,
     ID = 305,
     PROTOCOL = 306,
     PREFERENCE = 307,
     DISABLED = 308,
     DIRECT = 309,
     INTERFACE = 310,
     IMPORT = 311,
     EXPORT = 312,
     FILTER = 313,
     NONE = 314,
     STATES = 315,
     ROUTES = 316,
     FILTERS = 317,
     PASSWORD = 318,
     FROM = 319,
     PASSIVE = 320,
     TO = 321,
     EVENTS = 322,
     PACKETS = 323,
     PROTOCOLS = 324,
     INTERFACES = 325,
     PRIMARY = 326,
     STATS = 327,
     COUNT = 328,
     FOR = 329,
     COMMANDS = 330,
     PREEXPORT = 331,
     GENERATE = 332,
     LISTEN = 333,
     BGP = 334,
     V6ONLY = 335,
     ADDRESS = 336,
     PORT = 337,
     PASSWORDS = 338,
     SHOW = 339,
     STATUS = 340,
     SUMMARY = 341,
     ROUTE = 342,
     SYMBOLS = 343,
     DUMP = 344,
     RESOURCES = 345,
     SOCKETS = 346,
     NEIGHBORS = 347,
     ATTRIBUTES = 348,
     ECHO = 349,
     DISABLE = 350,
     ENABLE = 351,
     RESTART = 352,
     FUNCTION = 353,
     PRINT = 354,
     PRINTN = 355,
     UNSET = 356,
     RETURN = 357,
     ACCEPT = 358,
     REJECT = 359,
     QUITBIRD = 360,
     INT = 361,
     BOOL = 362,
     IP = 363,
     PREFIX = 364,
     PAIR = 365,
     SET = 366,
     STRING = 367,
     BGPMASK = 368,
     BGPPATH = 369,
     CLIST = 370,
     IF = 371,
     THEN = 372,
     ELSE = 373,
     CASE = 374,
     TRUE = 375,
     FALSE = 376,
     GW = 377,
     NET = 378,
     MASK = 379,
     PROTO = 380,
     SOURCE = 381,
     SCOPE = 382,
     CAST = 383,
     DEST = 384,
     LEN = 385,
     DEFINED = 386,
     ADD = 387,
     DELETE = 388,
     CONTAINS = 389,
     RESET = 390,
     PREPEND = 391,
     MATCH = 392,
     EMPTY = 393,
     WHERE = 394,
     EVAL = 395,
     LOCAL = 396,
     NEIGHBOR = 397,
     AS = 398,
     HOLD = 399,
     CONNECT = 400,
     RETRY = 401,
     KEEPALIVE = 402,
     MULTIHOP = 403,
     STARTUP = 404,
     VIA = 405,
     NEXT = 406,
     HOP = 407,
     SELF = 408,
     DEFAULT = 409,
     PATH = 410,
     METRIC = 411,
     START = 412,
     DELAY = 413,
     FORGET = 414,
     WAIT = 415,
     AFTER = 416,
     BGP_PATH = 417,
     BGP_LOCAL_PREF = 418,
     BGP_MED = 419,
     BGP_ORIGIN = 420,
     BGP_NEXT_HOP = 421,
     BGP_ATOMIC_AGGR = 422,
     BGP_AGGREGATOR = 423,
     BGP_COMMUNITY = 424,
     RR = 425,
     RS = 426,
     CLIENT = 427,
     CLUSTER = 428,
     AS4 = 429,
     ADVERTISE = 430,
     IPV4 = 431,
     CAPABILITIES = 432,
     LIMIT = 433,
     OSPF = 434,
     AREA = 435,
     OSPF_METRIC1 = 436,
     OSPF_METRIC2 = 437,
     OSPF_TAG = 438,
     BROADCAST = 439,
     RFC1583COMPAT = 440,
     STUB = 441,
     TICK = 442,
     COST = 443,
     RETRANSMIT = 444,
     HELLO = 445,
     TRANSMIT = 446,
     PRIORITY = 447,
     DEAD = 448,
     NONBROADCAST = 449,
     POINTOPOINT = 450,
     TYPE = 451,
     SIMPLE = 452,
     AUTHENTICATION = 453,
     STRICT = 454,
     CRYPTOGRAPHIC = 455,
     ELIGIBLE = 456,
     POLL = 457,
     NETWORKS = 458,
     HIDDEN = 459,
     VIRTUAL = 460,
     LINK = 461,
     RX = 462,
     BUFFER = 463,
     LARGE = 464,
     NORMAL = 465,
     STUBNET = 466,
     TOPOLOGY = 467,
     STATE = 468,
     PIPE = 469,
     PEER = 470,
     MODE = 471,
     OPAQUE = 472,
     TRANSPARENT = 473,
     RIP = 474,
     INFINITY = 475,
     PERIOD = 476,
     GARBAGE = 477,
     TIMEOUT = 478,
     MULTICAST = 479,
     QUIET = 480,
     NOLISTEN = 481,
     VERSION1 = 482,
     PLAINTEXT = 483,
     MD5 = 484,
     HONOR = 485,
     NEVER = 486,
     ALWAYS = 487,
     RIP_METRIC = 488,
     RIP_TAG = 489,
     STATIC = 490,
     DROP = 491,
     PROHIBIT = 492
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 146 "cf-parse.y"

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



/* Line 214 of yacc.c  */
#line 520 "cf-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 532 "cf-parse.tab.c"

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
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1509

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  259
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  180
/* YYNRULES -- Number of rules.  */
#define YYNRULES  542
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1009

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   492

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,     2,    29,     2,     2,
     249,   250,    27,    25,   255,    26,    24,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   252,   251,
      21,    20,    22,   256,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   257,     2,   258,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   253,     2,   254,    23,     2,     2,     2,
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
      15,    16,    17,    18,    19,    31,    32,    33,    34,    35,
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
     246,   247,   248
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    13,    15,    19,    21,
      27,    33,    35,    37,    39,    41,    43,    44,    46,    48,
      51,    53,    55,    58,    61,    63,    65,    66,    71,    73,
      75,    77,    79,    83,    85,    89,    91,    93,    95,    97,
      99,   101,   103,   105,   107,   111,   116,   119,   120,   122,
     125,   128,   132,   135,   138,   142,   146,   150,   155,   157,
     159,   161,   166,   167,   170,   173,   176,   178,   181,   183,
     184,   186,   187,   190,   193,   196,   199,   202,   205,   208,
     210,   212,   214,   216,   220,   224,   225,   227,   229,   232,
     233,   235,   239,   241,   245,   248,   252,   256,   260,   261,
     265,   267,   269,   273,   275,   279,   281,   283,   285,   287,
     289,   291,   296,   298,   299,   303,   308,   310,   313,   314,
     320,   326,   332,   338,   343,   347,   352,   358,   360,   361,
     365,   370,   375,   376,   379,   383,   387,   391,   394,   397,
     400,   404,   408,   411,   414,   416,   418,   423,   427,   431,
     435,   439,   443,   447,   451,   456,   458,   460,   462,   463,
     465,   469,   473,   477,   482,   484,   486,   488,   489,   494,
     497,   499,   501,   503,   505,   507,   509,   511,   513,   515,
     518,   521,   522,   526,   528,   532,   534,   536,   538,   541,
     545,   548,   553,   554,   560,   561,   564,   566,   570,   576,
     578,   580,   582,   584,   586,   588,   593,   595,   599,   603,
     605,   608,   611,   618,   620,   624,   625,   630,   634,   636,
     640,   644,   648,   651,   654,   657,   660,   661,   664,   667,
     668,   674,   676,   678,   680,   682,   684,   686,   690,   694,
     696,   698,   699,   704,   706,   708,   710,   712,   714,   716,
     718,   720,   722,   726,   730,   734,   738,   742,   746,   750,
     754,   758,   762,   766,   770,   774,   778,   781,   786,   788,
     790,   792,   794,   797,   800,   804,   808,   815,   819,   823,
     830,   837,   844,   849,   851,   853,   855,   857,   859,   861,
     863,   864,   866,   870,   872,   876,   877,   879,   884,   891,
     896,   900,   906,   912,   917,   924,   928,   931,   937,   943,
     952,   961,   970,   973,   977,   981,   987,   994,  1001,  1006,
    1011,  1017,  1024,  1031,  1037,  1044,  1050,  1056,  1062,  1068,
    1074,  1081,  1088,  1097,  1104,  1110,  1115,  1121,  1126,  1132,
    1135,  1139,  1143,  1145,  1148,  1151,  1154,  1158,  1161,  1162,
    1166,  1170,  1173,  1178,  1181,  1184,  1186,  1191,  1193,  1195,
    1196,  1200,  1203,  1206,  1209,  1214,  1216,  1217,  1221,  1222,
    1225,  1228,  1232,  1235,  1238,  1242,  1245,  1248,  1251,  1253,
    1257,  1260,  1263,  1266,  1269,  1273,  1276,  1279,  1282,  1286,
    1289,  1292,  1295,  1299,  1302,  1307,  1310,  1313,  1316,  1320,
    1324,  1328,  1330,  1331,  1334,  1336,  1338,  1341,  1345,  1346,
    1349,  1351,  1353,  1356,  1360,  1361,  1362,  1366,  1367,  1371,
    1375,  1377,  1378,  1383,  1390,  1397,  1404,  1411,  1414,  1418,
    1422,  1428,  1433,  1438,  1441,  1445,  1449,  1454,  1459,  1464,
    1470,  1476,  1481,  1485,  1490,  1495,  1500,  1505,  1507,  1509,
    1511,  1513,  1515,  1517,  1519,  1521,  1522,  1525,  1528,  1529,
    1533,  1534,  1538,  1539,  1543,  1546,  1550,  1554,  1558,  1561,
    1565,  1569,  1572,  1575,  1578,  1583,  1585,  1587,  1589,  1591,
    1593,  1595,  1597,  1599,  1601,  1603,  1605,  1607,  1609,  1611,
    1613,  1615,  1617,  1619,  1621,  1623,  1625,  1627,  1629,  1631,
    1633,  1635,  1637,  1639,  1641,  1643,  1645,  1647,  1649,  1651,
    1653,  1655,  1657,  1659,  1661,  1664,  1667,  1670,  1673,  1676,
    1679,  1682,  1685,  1689,  1693,  1697,  1701,  1705,  1709,  1713,
    1715,  1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,
    1735,  1737,  1739
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     260,     0,    -1,   261,     3,    -1,     4,   434,    -1,    -1,
     261,   433,    -1,    13,    -1,   249,   374,   250,    -1,    17,
      -1,    31,    17,    20,   262,   251,    -1,    31,    17,    20,
      16,   251,    -1,   262,    -1,    32,    -1,    34,    -1,    33,
      -1,    35,    -1,    -1,    16,    -1,    17,    -1,   265,   268,
      -1,   266,    -1,   265,    -1,    28,   262,    -1,   252,   265,
      -1,    18,    -1,    18,    -1,    -1,    36,   272,   273,   251,
      -1,    18,    -1,    37,    -1,    48,    -1,    38,    -1,   253,
     274,   254,    -1,   275,    -1,   274,   255,   275,    -1,    39,
      -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,
      -1,    45,    -1,    46,    -1,    47,    -1,    50,   279,     3,
      -1,    50,    49,   279,     3,    -1,    51,     3,    -1,    -1,
      18,    -1,   291,    52,    -1,    53,   264,    -1,    54,    55,
     262,    -1,    56,   264,    -1,   291,    57,    -1,    54,    55,
     262,    -1,    82,   270,   267,    -1,    52,    59,   262,    -1,
      60,    61,   286,   251,    -1,    13,    -1,    15,    -1,    16,
      -1,    89,    90,   288,   251,    -1,    -1,   288,   289,    -1,
      92,   265,    -1,    93,   262,    -1,    91,    -1,    59,    17,
      -1,    62,    -1,    -1,    17,    -1,    -1,    63,   262,    -1,
      64,   264,    -1,    39,   306,    -1,    67,   294,    -1,    68,
     294,    -1,    59,   295,    -1,    69,   347,    -1,   348,    -1,
      38,    -1,    70,    -1,    17,    -1,    39,    80,   306,    -1,
      39,    86,   262,    -1,    -1,    18,    -1,   266,    -1,    18,
     266,    -1,    -1,    26,    -1,   297,   299,   298,    -1,   300,
      -1,   301,   255,   300,    -1,   291,    65,    -1,   302,   292,
     253,    -1,   303,   293,   251,    -1,   303,   305,   251,    -1,
      -1,    66,   304,   301,    -1,    38,    -1,    33,    -1,   253,
     307,   254,    -1,   308,    -1,   307,   255,   308,    -1,    71,
      -1,    72,    -1,    73,    -1,    81,    -1,    78,    -1,    79,
      -1,    94,   253,   310,   254,    -1,   311,    -1,    -1,   311,
     251,   310,    -1,   312,   253,   313,   254,    -1,   312,    -1,
      74,    18,    -1,    -1,    88,    75,   269,   251,   313,    -1,
      88,    77,   269,   251,   313,    -1,   114,    75,   269,   251,
     313,    -1,   114,    77,   269,   251,   313,    -1,    61,   262,
     251,   313,    -1,    95,    96,     3,    -1,    95,    80,   317,
       3,    -1,    95,    80,    38,   317,     3,    -1,    17,    -1,
      -1,    95,    81,     3,    -1,    95,    81,    97,     3,    -1,
      95,    98,   321,     3,    -1,    -1,   321,   266,    -1,   321,
      85,   267,    -1,   321,    59,    17,    -1,   321,    69,   347,
      -1,   321,   348,    -1,   321,    38,    -1,   321,    82,    -1,
     321,   322,    17,    -1,   321,    62,    17,    -1,   321,    83,
      -1,   321,    84,    -1,    87,    -1,    68,    -1,    95,    99,
     317,     3,    -1,   100,   101,     3,    -1,   100,   102,     3,
      -1,   100,    81,     3,    -1,   100,   103,     3,    -1,   100,
     104,     3,    -1,   100,    72,     3,    -1,   100,    80,     3,
      -1,   105,   332,   333,     3,    -1,    38,    -1,    33,    -1,
      13,    -1,    -1,    13,    -1,   106,   338,     3,    -1,   107,
     338,     3,    -1,   108,   338,     3,    -1,    39,   338,   306,
       3,    -1,    17,    -1,    38,    -1,    18,    -1,    -1,    69,
      17,   340,   346,    -1,   151,   374,    -1,   117,    -1,   118,
      -1,   119,    -1,   120,    -1,   121,    -1,   123,    -1,   124,
      -1,   125,    -1,   126,    -1,   342,   122,    -1,   342,    17,
      -1,    -1,   343,   251,   344,    -1,   343,    -1,   345,   251,
     343,    -1,   350,    -1,    17,    -1,   346,    -1,   150,   374,
      -1,   249,   345,   250,    -1,   249,   250,    -1,   344,   253,
     353,   254,    -1,    -1,   109,    17,   352,   349,   350,    -1,
      -1,   380,   353,    -1,   380,    -1,   253,   353,   254,    -1,
     249,    13,   255,    13,   250,    -1,    16,    -1,    13,    -1,
     355,    -1,   356,    -1,    14,    -1,   357,    -1,   357,    24,
      24,   357,    -1,   358,    -1,   359,   255,   358,    -1,    16,
      28,    13,    -1,   360,    -1,   360,    25,    -1,   360,    26,
      -1,   360,   253,    13,   255,    13,   254,    -1,   361,    -1,
     362,   255,   361,    -1,    -1,   358,   252,   353,   363,    -1,
     129,   252,   353,    -1,   372,    -1,   249,   374,   250,    -1,
      11,   366,    12,    -1,    28,   367,    28,    -1,    13,   366,
      -1,    27,   366,    -1,   256,   366,    -1,   364,   366,    -1,
      -1,    13,   367,    -1,   256,   367,    -1,    -1,   249,   374,
     255,   374,   250,    -1,    13,    -1,   131,    -1,   132,    -1,
      18,    -1,   356,    -1,   360,    -1,   257,   359,   258,    -1,
     257,   362,   258,    -1,    14,    -1,   365,    -1,    -1,    17,
     249,   379,   250,    -1,    17,    -1,    75,    -1,   133,    -1,
     134,    -1,   136,    -1,   137,    -1,   138,    -1,   139,    -1,
     140,    -1,   249,   374,   250,    -1,   374,    25,   374,    -1,
     374,    26,   374,    -1,   374,    27,   374,    -1,   374,    28,
     374,    -1,   374,     9,   374,    -1,   374,    10,   374,    -1,
     374,    20,   374,    -1,   374,     8,   374,    -1,   374,    21,
     374,    -1,   374,     7,   374,    -1,   374,    22,   374,    -1,
     374,     6,   374,    -1,   374,    23,   374,    -1,    30,   374,
      -1,   142,   249,   374,   250,    -1,   372,    -1,   369,    -1,
     368,    -1,    63,    -1,   370,   373,    -1,   370,   438,    -1,
     374,    24,   119,    -1,   374,    24,   141,    -1,   374,    24,
     135,   249,   374,   250,    -1,    25,   149,    25,    -1,    26,
     149,    26,    -1,   147,   249,   374,   255,   374,   250,    -1,
     143,   249,   374,   255,   374,   250,    -1,   144,   249,   374,
     255,   374,   250,    -1,    17,   249,   379,   250,    -1,   116,
      -1,   114,    -1,   115,    -1,    44,    -1,   110,    -1,   111,
      -1,   374,    -1,    -1,   376,    -1,   376,   255,   377,    -1,
     374,    -1,   374,   255,   378,    -1,    -1,   378,    -1,   127,
     374,   128,   354,    -1,   127,   374,   128,   354,   129,   354,
      -1,    17,    20,   374,   251,    -1,   113,   374,   251,    -1,
     370,   438,    20,   374,   251,    -1,   370,   373,    20,   374,
     251,    -1,    63,    20,   374,   251,    -1,   112,   249,   370,
     438,   250,   251,    -1,   375,   377,   251,    -1,   371,   251,
      -1,   130,   374,   253,   363,   254,    -1,   370,   438,    24,
     149,   251,    -1,   370,   438,    24,   147,   249,   374,   250,
     251,    -1,   370,   438,    24,   143,   249,   374,   250,   251,
      -1,   370,   438,    24,   144,   249,   374,   250,   251,    -1,
     291,    90,    -1,   381,   292,   253,    -1,   382,   293,   251,
      -1,   382,   152,   154,   262,   251,    -1,   382,   153,   265,
     154,   262,   251,    -1,   382,   181,   184,    61,   262,   251,
      -1,   382,   181,   183,   251,    -1,   382,   182,   183,   251,
      -1,   382,   155,    55,   262,   251,    -1,   382,   160,   155,
      55,   262,   251,    -1,   382,   156,   157,    55,   262,   251,
      -1,   382,   158,    55,   262,   251,    -1,   382,   159,   262,
     161,   265,   251,    -1,   382,   162,   163,   164,   251,    -1,
     382,   166,   167,   264,   251,    -1,   382,   165,   175,   262,
     251,    -1,   382,   165,   174,   262,   251,    -1,   382,   137,
      92,   265,   251,    -1,   382,   168,   169,    55,   262,   251,
      -1,   382,    44,   170,    55,   262,   251,    -1,   382,    44,
     171,    55,   262,   255,   262,   251,    -1,   382,   106,   172,
      44,   264,   251,    -1,   382,   107,   185,   264,   251,    -1,
     382,   188,   264,   251,    -1,   382,   186,   187,   264,   251,
      -1,   382,    74,    18,   251,    -1,   382,    98,   189,   262,
     251,    -1,   291,   190,    -1,   383,   292,   253,    -1,   384,
     385,   251,    -1,   293,    -1,   196,   264,    -1,   198,   262,
      -1,   387,   254,    -1,   191,   286,   253,    -1,   386,   388,
      -1,    -1,   388,   389,   251,    -1,   197,   199,   262,    -1,
     197,   264,    -1,   214,   253,   399,   254,    -1,   222,   390,
      -1,    66,   410,    -1,   394,    -1,   391,   253,   392,   254,
      -1,   391,    -1,   266,    -1,    -1,   392,   393,   251,    -1,
     215,   264,    -1,    97,   264,    -1,   199,   262,    -1,   397,
     253,   395,   254,    -1,   397,    -1,    -1,   395,   396,   251,
      -1,    -1,   201,   262,    -1,   200,   262,    -1,   202,   169,
     262,    -1,   171,   262,    -1,   204,   262,    -1,   204,    84,
     262,    -1,   209,    70,    -1,   209,   208,    -1,   209,   211,
      -1,   309,    -1,   216,   217,   286,    -1,   199,   262,    -1,
     201,   262,    -1,   213,   262,    -1,   200,   262,    -1,   202,
     169,   262,    -1,   203,   262,    -1,   171,   262,    -1,   204,
     262,    -1,   204,    84,   262,    -1,   207,   195,    -1,   207,
     205,    -1,   207,   206,    -1,   210,   205,   264,    -1,   197,
     264,    -1,   103,   253,   403,   254,    -1,   209,    70,    -1,
     209,   208,    -1,   209,   211,    -1,   218,   219,   220,    -1,
     218,   219,   221,    -1,   218,   219,   262,    -1,   309,    -1,
      -1,   399,   400,    -1,   401,    -1,   402,    -1,   266,   251,
      -1,   266,   215,   251,    -1,    -1,   403,   404,    -1,   405,
      -1,   406,    -1,    16,   251,    -1,    16,   212,   251,    -1,
      -1,    -1,   408,   398,   251,    -1,    -1,   253,   408,   254,
      -1,   407,   301,   409,    -1,    18,    -1,    -1,    95,   190,
     317,     3,    -1,    95,   190,   103,   317,   411,     3,    -1,
      95,   190,    66,   317,   411,     3,    -1,    95,   190,   223,
     317,   411,     3,    -1,    95,   190,   224,   317,   411,     3,
      -1,   291,   225,    -1,   417,   292,   253,    -1,   418,   293,
     251,    -1,   418,   226,    59,    17,   251,    -1,   418,   227,
     228,   251,    -1,   418,   227,   229,   251,    -1,   291,   230,
      -1,   419,   292,   253,    -1,   420,   293,   251,    -1,   420,
     231,   262,   251,    -1,   420,    93,   262,   251,    -1,   420,
     232,   262,   251,    -1,   420,   233,    55,   262,   251,    -1,
     420,   234,    55,   262,   251,    -1,   420,   209,   421,   251,
      -1,   420,   309,   251,    -1,   420,   241,   243,   251,    -1,
     420,   241,   153,   251,    -1,   420,   241,   242,   251,    -1,
     420,    66,   427,   251,    -1,   239,    -1,   240,    -1,    70,
      -1,   195,    -1,   235,    -1,   236,    -1,   237,    -1,   238,
      -1,    -1,   167,   262,    -1,   227,   422,    -1,    -1,   424,
     423,   251,    -1,    -1,   253,   424,   254,    -1,    -1,   426,
     301,   425,    -1,   291,   246,    -1,   428,   292,   253,    -1,
     429,   293,   251,    -1,   429,   431,   251,    -1,    98,   266,
      -1,   430,   161,   265,    -1,   430,   161,    18,    -1,   430,
     247,    -1,   430,   115,    -1,   430,   248,    -1,    95,   246,
     317,     3,    -1,   251,    -1,   263,    -1,   271,    -1,   285,
      -1,   287,    -1,   290,    -1,   435,    -1,   296,    -1,   339,
      -1,   341,    -1,   351,    -1,   276,    -1,   277,    -1,   278,
      -1,   314,    -1,   315,    -1,   316,    -1,   318,    -1,   319,
      -1,   320,    -1,   323,    -1,   324,    -1,   325,    -1,   326,
      -1,   327,    -1,   328,    -1,   329,    -1,   330,    -1,   331,
      -1,   334,    -1,   335,    -1,   336,    -1,   337,    -1,   412,
      -1,   413,    -1,   414,    -1,   415,    -1,   416,    -1,   432,
      -1,   436,   254,    -1,   437,   254,    -1,   303,   254,    -1,
     382,   254,    -1,   384,   254,    -1,   418,   254,    -1,   420,
     254,    -1,   429,   254,    -1,   280,   292,   253,    -1,   436,
     293,   251,    -1,   436,   281,   251,    -1,   436,   284,   251,
      -1,   282,   292,   253,    -1,   437,   293,   251,    -1,   437,
     283,   251,    -1,     5,    -1,   173,    -1,   174,    -1,   175,
      -1,   176,    -1,   177,    -1,   178,    -1,   179,    -1,   180,
      -1,   192,    -1,   193,    -1,   194,    -1,   244,    -1,   245,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   293,   293,   294,   297,   299,   306,   307,   308,   312,
     316,   325,   326,   327,   328,   329,   330,   336,   337,   344,
     351,   352,   356,   360,   367,   375,   376,   382,   391,   396,
     397,   401,   402,   406,   407,   411,   412,   413,   414,   415,
     416,   417,   418,   419,   426,   429,   432,   436,   437,   445,
     461,   462,   466,   478,   491,   495,   508,   520,   526,   527,
     528,   539,   541,   543,   547,   548,   549,   556,   564,   568,
     574,   580,   582,   586,   587,   588,   589,   590,   594,   595,
     596,   597,   601,   609,   610,   616,   624,   625,   626,   630,
     631,   635,   640,   641,   648,   657,   658,   659,   663,   672,
     678,   679,   680,   684,   685,   689,   690,   691,   692,   693,
     694,   700,   701,   704,   706,   710,   711,   715,   733,   734,
     735,   736,   737,   738,   746,   749,   752,   756,   757,   760,
     763,   766,   770,   776,   782,   789,   794,   799,   804,   808,
     812,   822,   830,   834,   841,   842,   845,   849,   851,   853,
     855,   857,   859,   861,   864,   870,   871,   872,   876,   877,
     883,   885,   887,   891,   896,   897,   898,   904,   904,   913,
     917,   918,   919,   920,   921,   922,   923,   924,   925,   926,
     945,   956,   957,   964,   965,   972,   981,   985,   989,  1013,
    1014,  1018,  1024,  1024,  1037,  1038,  1049,  1052,  1061,  1068,
    1072,  1073,  1074,  1075,  1079,  1084,  1092,  1093,  1097,  1104,
    1105,  1106,  1107,  1114,  1115,  1118,  1119,  1124,  1135,  1136,
    1140,  1141,  1145,  1146,  1147,  1148,  1149,  1153,  1154,  1155,
    1159,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1191,  1195,  1218,  1251,  1253,  1254,  1255,  1256,  1257,
    1258,  1259,  1263,  1264,  1265,  1266,  1267,  1268,  1269,  1270,
    1271,  1272,  1273,  1274,  1275,  1276,  1277,  1278,  1280,  1281,
    1282,  1284,  1286,  1288,  1290,  1291,  1292,  1302,  1303,  1304,
    1305,  1306,  1311,  1334,  1335,  1336,  1337,  1338,  1339,  1343,
    1346,  1347,  1348,  1357,  1364,  1373,  1374,  1378,  1384,  1394,
    1403,  1409,  1414,  1421,  1426,  1432,  1433,  1434,  1442,  1444,
    1445,  1446,  1452,  1471,  1472,  1473,  1474,  1480,  1481,  1482,
    1483,  1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,
    1493,  1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1510,
    1520,  1521,  1525,  1526,  1527,  1528,  1531,  1543,  1546,  1548,
    1552,  1553,  1554,  1555,  1556,  1557,  1561,  1562,  1566,  1574,
    1576,  1580,  1581,  1582,  1586,  1587,  1590,  1592,  1595,  1596,
    1597,  1598,  1599,  1600,  1601,  1602,  1603,  1604,  1605,  1608,
    1630,  1631,  1632,  1633,  1634,  1635,  1636,  1637,  1638,  1639,
    1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,
    1650,  1651,  1654,  1656,  1660,  1661,  1663,  1672,  1682,  1684,
    1688,  1689,  1691,  1700,  1711,  1733,  1735,  1738,  1740,  1744,
    1748,  1749,  1753,  1756,  1759,  1762,  1765,  1771,  1779,  1780,
    1781,  1786,  1787,  1793,  1800,  1801,  1802,  1803,  1804,  1805,
    1806,  1807,  1808,  1809,  1810,  1811,  1812,  1816,  1817,  1818,
    1823,  1824,  1825,  1826,  1827,  1830,  1831,  1832,  1835,  1837,
    1840,  1842,  1846,  1855,  1862,  1869,  1870,  1871,  1874,  1883,
    1887,  1893,  1894,  1895,  1898,  1905,  1905,  1905,  1905,  1905,
    1905,  1905,  1905,  1905,  1905,  1905,  1906,  1906,  1906,  1906,
    1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,
    1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,  1906,
    1906,  1906,  1906,  1906,  1907,  1907,  1907,  1907,  1907,  1907,
    1907,  1907,  1908,  1908,  1908,  1908,  1909,  1909,  1909,  1910,
    1910,  1911,  1912,  1913,  1914,  1915,  1916,  1917,  1918,  1918,
    1918,  1918,  1918
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "CLI_MARKER", "INVALID_TOKEN",
  "GEQ", "LEQ", "NEQ", "AND", "OR", "PO", "PC", "NUM", "ENUM", "RTRID",
  "IPA", "SYM", "TEXT", "PREFIX_DUMMY", "'='", "'<'", "'>'", "'~'", "'.'",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "DEFINE", "ON", "OFF", "YES",
  "NO", "LOG", "SYSLOG", "ALL", "DEBUG", "TRACE", "INFO", "REMOTE",
  "WARNING", "ERROR", "AUTH", "FATAL", "BUG", "STDERR", "SOFT",
  "CONFIGURE", "DOWN", "KERNEL", "PERSIST", "SCAN", "TIME", "LEARN",
  "DEVICE", "ASYNC", "TABLE", "ROUTER", "ID", "PROTOCOL", "PREFERENCE",
  "DISABLED", "DIRECT", "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE",
  "STATES", "ROUTES", "FILTERS", "PASSWORD", "FROM", "PASSIVE", "TO",
  "EVENTS", "PACKETS", "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS",
  "COUNT", "FOR", "COMMANDS", "PREEXPORT", "GENERATE", "LISTEN", "BGP",
  "V6ONLY", "ADDRESS", "PORT", "PASSWORDS", "SHOW", "STATUS", "SUMMARY",
  "ROUTE", "SYMBOLS", "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS",
  "ATTRIBUTES", "ECHO", "DISABLE", "ENABLE", "RESTART", "FUNCTION",
  "PRINT", "PRINTN", "UNSET", "RETURN", "ACCEPT", "REJECT", "QUITBIRD",
  "INT", "BOOL", "IP", "PREFIX", "PAIR", "SET", "STRING", "BGPMASK",
  "BGPPATH", "CLIST", "IF", "THEN", "ELSE", "CASE", "TRUE", "FALSE", "GW",
  "NET", "MASK", "PROTO", "SOURCE", "SCOPE", "CAST", "DEST", "LEN",
  "DEFINED", "ADD", "DELETE", "CONTAINS", "RESET", "PREPEND", "MATCH",
  "EMPTY", "WHERE", "EVAL", "LOCAL", "NEIGHBOR", "AS", "HOLD", "CONNECT",
  "RETRY", "KEEPALIVE", "MULTIHOP", "STARTUP", "VIA", "NEXT", "HOP",
  "SELF", "DEFAULT", "PATH", "METRIC", "START", "DELAY", "FORGET", "WAIT",
  "AFTER", "BGP_PATH", "BGP_LOCAL_PREF", "BGP_MED", "BGP_ORIGIN",
  "BGP_NEXT_HOP", "BGP_ATOMIC_AGGR", "BGP_AGGREGATOR", "BGP_COMMUNITY",
  "RR", "RS", "CLIENT", "CLUSTER", "AS4", "ADVERTISE", "IPV4",
  "CAPABILITIES", "LIMIT", "OSPF", "AREA", "OSPF_METRIC1", "OSPF_METRIC2",
  "OSPF_TAG", "BROADCAST", "RFC1583COMPAT", "STUB", "TICK", "COST",
  "RETRANSMIT", "HELLO", "TRANSMIT", "PRIORITY", "DEAD", "NONBROADCAST",
  "POINTOPOINT", "TYPE", "SIMPLE", "AUTHENTICATION", "STRICT",
  "CRYPTOGRAPHIC", "ELIGIBLE", "POLL", "NETWORKS", "HIDDEN", "VIRTUAL",
  "LINK", "RX", "BUFFER", "LARGE", "NORMAL", "STUBNET", "TOPOLOGY",
  "STATE", "PIPE", "PEER", "MODE", "OPAQUE", "TRANSPARENT", "RIP",
  "INFINITY", "PERIOD", "GARBAGE", "TIMEOUT", "MULTICAST", "QUIET",
  "NOLISTEN", "VERSION1", "PLAINTEXT", "MD5", "HONOR", "NEVER", "ALWAYS",
  "RIP_METRIC", "RIP_TAG", "STATIC", "DROP", "PROHIBIT", "'('", "')'",
  "';'", "':'", "'{'", "'}'", "','", "'?'", "'['", "']'", "$accept",
  "config", "conf_entries", "expr", "definition", "bool", "ipa", "prefix",
  "prefix_or_ipa", "pxlen", "datetime", "text_or_none", "log_config",
  "log_file", "log_mask", "log_mask_list", "log_cat", "cmd_CONFIGURE",
  "cmd_CONFIGURE_SOFT", "cmd_DOWN", "cfg_name", "kern_proto_start",
  "kern_item", "kif_proto_start", "kif_item", "nl_item", "rtrid", "idval",
  "listen", "listen_opts", "listen_opt", "newtab", "proto_start",
  "proto_name", "proto_item", "imexport", "rtable", "debug_default",
  "iface_patt_node_init", "iface_patt_node_body", "iface_negate",
  "iface_patt_node", "iface_patt_list", "dev_proto_start", "dev_proto",
  "dev_iface_init", "dev_iface_patt", "debug_mask", "debug_list",
  "debug_flag", "password_list", "password_items", "password_item",
  "password_item_begin", "password_item_params", "cmd_SHOW_STATUS",
  "cmd_SHOW_PROTOCOLS", "cmd_SHOW_PROTOCOLS_ALL", "optsym",
  "cmd_SHOW_INTERFACES", "cmd_SHOW_INTERFACES_SUMMARY", "cmd_SHOW_ROUTE",
  "r_args", "export_or_preexport", "cmd_SHOW_SYMBOLS",
  "cmd_DUMP_RESOURCES", "cmd_DUMP_SOCKETS", "cmd_DUMP_INTERFACES",
  "cmd_DUMP_NEIGHBORS", "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES",
  "cmd_DUMP_PROTOCOLS", "cmd_ECHO", "echo_mask", "echo_size",
  "cmd_DISABLE", "cmd_ENABLE", "cmd_RESTART", "cmd_DEBUG", "proto_patt",
  "filter_def", "$@1", "filter_eval", "type", "one_decl", "decls",
  "declsn", "filter_body", "filter", "where_filter", "function_params",
  "function_body", "function_def", "$@2", "cmds", "block", "cpair", "fipa",
  "set_atom", "set_item", "set_items", "fprefix_s", "fprefix",
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
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_STATE", "pipe_proto_start",
  "pipe_proto", "rip_cfg_start", "rip_cfg", "rip_auth", "rip_mode",
  "rip_iface_item", "rip_iface_opts", "rip_iface_opt_list",
  "rip_iface_init", "rip_iface", "static_proto_start", "static_proto",
  "stat_route0", "stat_route", "cmd_SHOW_STATIC", "conf", "cli_cmd",
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
      61,    60,    62,   126,    46,    43,    45,    42,    47,    37,
      33,   275,   276,   277,   278,   279,   280,   281,   282,   283,
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
     484,   485,   486,   487,   488,   489,   490,   491,   492,    40,
      41,    59,    58,   123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   259,   260,   260,   261,   261,   262,   262,   262,   263,
     263,   264,   264,   264,   264,   264,   264,   265,   265,   266,
     267,   267,   268,   268,   269,   270,   270,   271,   272,   272,
     272,   273,   273,   274,   274,   275,   275,   275,   275,   275,
     275,   275,   275,   275,   276,   277,   278,   279,   279,   280,
     281,   281,   281,   282,   283,   283,   284,   285,   286,   286,
     286,   287,   288,   288,   289,   289,   289,   290,   291,   292,
     292,   293,   293,   293,   293,   293,   293,   293,   294,   294,
     294,   294,   295,   296,   296,   297,   298,   298,   298,   299,
     299,   300,   301,   301,   302,   303,   303,   303,   304,   305,
     306,   306,   306,   307,   307,   308,   308,   308,   308,   308,
     308,   309,   309,   310,   310,   311,   311,   312,   313,   313,
     313,   313,   313,   313,   314,   315,   316,   317,   317,   318,
     319,   320,   321,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   322,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   332,   332,   333,   333,
     334,   335,   336,   337,   338,   338,   338,   340,   339,   341,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     343,   344,   344,   345,   345,   346,   347,   347,   348,   349,
     349,   350,   352,   351,   353,   353,   354,   354,   355,   356,
     357,   357,   357,   357,   358,   358,   359,   359,   360,   361,
     361,   361,   361,   362,   362,   363,   363,   363,   364,   364,
     365,   365,   366,   366,   366,   366,   366,   367,   367,   367,
     368,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   370,   371,   372,   373,   373,   373,   373,   373,   373,
     373,   373,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   375,   375,   375,   375,   375,   375,   376,
     377,   377,   377,   378,   378,   379,   379,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   381,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   383,
     384,   384,   385,   385,   385,   385,   386,   387,   388,   388,
     389,   389,   389,   389,   389,   389,   390,   390,   391,   392,
     392,   393,   393,   393,   394,   394,   395,   395,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   397,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   399,   399,   400,   400,   401,   402,   403,   403,
     404,   404,   405,   406,   407,   408,   408,   409,   409,   410,
     411,   411,   412,   413,   414,   415,   416,   417,   418,   418,
     418,   418,   418,   419,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   421,   421,   421,
     422,   422,   422,   422,   422,   423,   423,   423,   424,   424,
     425,   425,   426,   427,   428,   429,   429,   429,   430,   431,
     431,   431,   431,   431,   432,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   435,   435,   435,   435,   435,   435,
     435,   435,   436,   436,   436,   436,   437,   437,   437,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     3,     1,     5,
       5,     1,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     2,     2,     1,     1,     0,     4,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     4,     2,     0,     1,     2,
       2,     3,     2,     2,     3,     3,     3,     4,     1,     1,
       1,     4,     0,     2,     2,     2,     1,     2,     1,     0,
       1,     0,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     3,     3,     0,     1,     1,     2,     0,
       1,     3,     1,     3,     2,     3,     3,     3,     0,     3,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     4,     1,     0,     3,     4,     1,     2,     0,     5,
       5,     5,     5,     4,     3,     4,     5,     1,     0,     3,
       4,     4,     0,     2,     3,     3,     3,     2,     2,     2,
       3,     3,     2,     2,     1,     1,     4,     3,     3,     3,
       3,     3,     3,     3,     4,     1,     1,     1,     0,     1,
       3,     3,     3,     4,     1,     1,     1,     0,     4,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     0,     3,     1,     3,     1,     1,     1,     2,     3,
       2,     4,     0,     5,     0,     2,     1,     3,     5,     1,
       1,     1,     1,     1,     1,     4,     1,     3,     3,     1,
       2,     2,     6,     1,     3,     0,     4,     3,     1,     3,
       3,     3,     2,     2,     2,     2,     0,     2,     2,     0,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     1,
       1,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     4,     1,     1,
       1,     1,     2,     2,     3,     3,     6,     3,     3,     6,
       6,     6,     4,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     3,     1,     3,     0,     1,     4,     6,     4,
       3,     5,     5,     4,     6,     3,     2,     5,     5,     8,
       8,     8,     2,     3,     3,     5,     6,     6,     4,     4,
       5,     6,     6,     5,     6,     5,     5,     5,     5,     5,
       6,     6,     8,     6,     5,     4,     5,     4,     5,     2,
       3,     3,     1,     2,     2,     2,     3,     2,     0,     3,
       3,     2,     4,     2,     2,     1,     4,     1,     1,     0,
       3,     2,     2,     2,     4,     1,     0,     3,     0,     2,
       2,     3,     2,     2,     3,     2,     2,     2,     1,     3,
       2,     2,     2,     2,     3,     2,     2,     2,     3,     2,
       2,     2,     3,     2,     4,     2,     2,     2,     3,     3,
       3,     1,     0,     2,     1,     1,     2,     3,     0,     2,
       1,     1,     2,     3,     0,     0,     3,     0,     3,     3,
       1,     0,     4,     6,     6,     6,     6,     2,     3,     3,
       5,     4,     4,     2,     3,     3,     4,     4,     4,     5,
       5,     4,     3,     4,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     2,     2,     0,     3,
       0,     3,     0,     3,     2,     3,     3,     3,     2,     3,
       3,     2,     2,     2,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   509,   510,   511,   512,
     513,     3,     1,     2,     0,     0,     0,     0,     0,    68,
       0,     0,     0,   241,   475,   476,   477,    69,    69,   478,
     479,   480,     0,   482,    69,    71,   483,   484,   485,    69,
      71,    69,    71,    69,    71,    69,    71,    69,    71,     5,
     481,    71,    71,   164,   166,   165,     0,    48,    47,     0,
      46,   128,     0,     0,   132,   128,   128,   128,     0,     0,
       0,     0,     0,     0,     0,   157,   156,   155,   158,     0,
       0,     0,     0,    28,    29,    30,     0,     0,     0,    67,
       0,   167,    62,   192,   226,   231,   239,   199,   243,   234,
       0,     0,   229,   241,   271,   232,   233,     0,     0,     0,
       0,   241,     0,   235,   236,   240,   270,   269,     0,   268,
     169,    70,     0,     0,    49,    53,    94,   312,   339,   427,
     433,   464,     0,     0,     0,     0,    16,    98,     0,     0,
     516,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    16,   517,     0,     0,     0,    16,
       0,   518,   342,     0,   348,     0,     0,     0,     0,   519,
       0,     0,   462,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   520,     0,     0,   112,   116,     0,     0,   521,
       0,     0,     0,     0,    16,     0,    16,   514,     0,     0,
       0,     0,    26,   515,     0,     0,   101,   100,     0,     0,
       0,    44,   127,   128,     0,   129,     0,   124,     0,     0,
     128,   128,   128,   128,     0,     0,   152,   153,   149,   147,
     148,   150,   151,   159,     0,   160,   161,   162,     0,    31,
       0,     0,    83,     6,     8,   241,    84,    58,    59,    60,
       0,   181,     0,     0,   226,   243,   226,   241,   226,   226,
       0,   218,     0,   241,     0,     0,   229,   229,     0,   266,
     241,   241,   241,   241,     0,   200,   203,     0,   201,   202,
     204,   206,     0,   209,   213,     0,   529,   244,   245,   246,
     247,   248,   249,   250,   251,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   272,   273,
     241,   241,   241,   241,   241,   241,   241,   241,   241,     0,
     241,   241,   241,   241,   522,   526,    95,    74,    82,    77,
      72,    12,    14,    13,    15,    11,    73,    85,    80,   181,
      81,   241,    75,    79,    76,    96,    97,   313,     0,     0,
       0,     0,     0,    16,     0,     0,    17,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16,     0,     0,
       0,     0,    16,     0,   314,   340,     0,   343,   344,   341,
     347,   345,   428,     0,     0,     0,   429,   434,    85,     0,
     117,     0,   113,   449,   447,   448,     0,     0,     0,     0,
       0,     0,     0,     0,   435,   442,   118,   465,     0,   468,
     466,   472,     0,   471,   473,   467,     0,    50,     0,    52,
     524,   525,   523,     0,    25,     0,   528,   527,   105,   106,
     107,   109,   110,   108,     0,   103,   163,    45,     0,   125,
     130,   131,   138,     0,     0,   145,   181,   139,   142,   143,
       0,   144,   133,     0,   137,   146,   421,   421,   421,   421,
     422,   474,   154,     0,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     0,    33,    27,     0,    57,   170,
     171,   172,   173,   174,   175,   176,   177,   178,     0,     0,
       0,   168,   185,    66,     0,     0,    61,    63,     0,   181,
     222,   223,     0,   224,   225,   220,   208,   293,   296,     0,
     277,   278,   227,   228,   221,     0,     0,     0,     0,   252,
     241,     0,     0,     0,   237,   210,   211,     0,     0,   238,
     264,   262,   260,   257,   258,   259,   261,   263,   265,   274,
       0,   275,   253,   254,   255,   256,    89,    92,    99,   186,
     187,    78,   188,     0,     0,   337,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   318,     0,   319,     0,   335,   346,   414,    16,
       0,     0,     0,     0,   355,   365,     0,   431,   432,   460,
     446,   437,     0,     0,   441,   436,   438,     0,     0,   444,
     445,   443,     0,     0,     0,     0,     0,     0,    19,   470,
     469,    56,    51,    54,    21,    20,    55,   102,     0,   126,
     135,   141,   136,   134,   140,   420,     0,     0,     0,     0,
      10,     9,    32,     0,     7,   180,   179,   181,   241,    64,
      65,   190,   183,     0,   193,   219,   241,   282,   267,   241,
     241,   241,     0,     0,     0,   199,   207,     0,     0,   214,
     241,    90,     0,    85,     0,     0,   338,     0,   334,   329,
     315,     0,   320,     0,   323,     0,     0,   325,   328,   327,
     326,     0,     0,   336,    85,   354,     0,   351,   402,     0,
     358,   353,   357,   349,   366,   430,   458,   463,   111,   113,
     439,   440,     0,     0,     0,     0,     0,   115,    22,    23,
     104,   424,   423,   425,   426,    34,   182,     0,   286,     0,
     287,   288,     0,   241,   284,   285,   283,   241,   241,     0,
       0,     0,   241,   241,   189,     0,   294,     0,     0,     0,
     230,     0,   205,     0,     0,    86,    87,    91,    93,   331,
       0,   333,   316,   322,   324,   321,   330,   317,   417,   350,
       0,   379,   359,   368,   455,   114,   118,    24,     0,     0,
       0,     0,   241,   241,   241,   241,     0,     0,     0,   191,
       0,     0,   306,   289,   291,     0,   195,   184,   280,   281,
     279,   198,     0,   276,    88,     0,   415,   419,   352,     0,
     403,   404,   405,     0,     0,     0,     0,     0,     0,     0,
     364,   378,     0,     0,     0,   461,     0,   123,   118,   118,
     118,   118,     0,     0,     0,     0,   300,   241,   215,   241,
     241,     0,   241,   305,   212,   332,     0,     0,   406,    16,
       0,    16,   356,     0,   372,   370,   369,     0,     0,   373,
     375,   376,   377,   367,   456,   450,   451,   452,   453,   454,
     457,   459,   119,   120,   121,   122,   299,   242,   303,     0,
     241,   297,   196,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,     0,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   418,   401,     0,
     407,   362,   363,   361,   360,   371,   374,     0,     0,   241,
     241,   241,   307,   302,   301,   241,   241,   241,   308,   408,
     386,   393,   380,   383,   381,     0,   385,     0,   387,   389,
     390,   391,   395,   396,   397,    16,   382,     0,   416,   304,
     197,   298,   217,   215,     0,     0,     0,     0,   384,   388,
     392,   398,   399,   400,   216,     0,     0,     0,     0,   394,
     409,   410,   411,   310,   311,   309,     0,   412,   413
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   375,    55,   376,   448,   655,   656,   648,
     808,   465,    56,   116,   281,   514,   515,    13,    14,    15,
      89,    57,   238,    58,   244,   239,    59,   290,    60,   292,
     537,    61,    62,   152,   171,   382,   369,    63,   586,   787,
     702,   587,   588,    64,    65,   377,   172,   249,   474,   475,
     224,   632,   225,   226,   645,    16,    17,    18,   254,    19,
      20,    21,   258,   493,    22,    23,    24,    25,    26,    27,
      28,    29,    30,   108,   274,    31,    32,    33,    34,    86,
      66,   291,    67,   528,   529,   530,   683,   590,   591,   383,
     539,   532,    68,   293,   769,   911,   318,   143,   320,   914,
     322,   144,   324,   325,   915,   299,   145,   300,   308,   146,
     147,   148,   771,   149,   348,   547,   772,   824,   825,   548,
     549,   773,    69,    70,    71,    72,   203,   204,   205,   420,
     623,   731,   732,   843,   883,   624,   803,   852,   625,   939,
     800,   840,   841,   842,   987,  1000,  1001,  1002,   724,   876,
     837,   725,   666,    35,    36,    37,    38,    39,    73,    74,
      75,    76,   436,   900,   856,   804,   737,   428,   429,    77,
      78,   231,   232,    40,    79,    41,    80,    81,    82,   349
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -848
static const yytype_int16 yypact[] =
{
      50,  1080,    46,   166,   375,    37,    90,   439,   568,   191,
     375,   375,   375,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,    64,   308,   251,    93,   107,  -848,
     181,   148,   263,   427,  -848,  -848,  -848,   279,   279,  -848,
    -848,  -848,  1002,  -848,   279,   762,  -848,  -848,  -848,   279,
    1109,   279,   552,   279,  1031,   279,  1145,   279,  1019,  -848,
    -848,  1012,   454,  -848,  -848,  -848,   -10,  -848,   294,   318,
    -848,   254,    14,   327,  -848,   331,   119,   331,   335,   341,
     350,   360,   365,   377,   382,  -848,  -848,  -848,   358,   401,
     404,   413,   422,  -848,  -848,  -848,    -8,   -10,    34,  -848,
     433,  -848,  -848,  -848,    23,  -848,  -848,   442,   226,  -848,
     334,   342,    -2,   427,  -848,  -848,  -848,   246,   248,   265,
     281,   427,    76,  -848,  -848,  -848,  -848,  -848,   410,  -848,
    1404,  -848,   289,   292,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,   304,   -10,   551,    34,   154,  -848,     0,     0,
    -848,   322,   357,   348,  -149,   592,   423,   451,   429,   536,
     478,   485,   584,   495,   610,    34,   519,   512,   349,   515,
     514,   368,   503,   501,   154,  -848,   456,   437,   433,   154,
      34,  -848,  -848,   458,  -848,   457,   459,   654,   338,  -848,
     465,   468,  -848,   700,    34,   469,    79,    34,    34,   668,
     669,   -97,  -848,   474,   475,  -848,   479,   481,   485,  -848,
     484,   317,   486,   692,   154,   697,   154,  -848,   502,   505,
     506,   699,   740,  -848,   516,   518,  -848,  -848,   482,   774,
     775,  -848,  -848,   331,   776,  -848,   777,  -848,  1283,   778,
     331,   331,   331,   331,   779,   781,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,   782,  -848,  -848,  -848,    78,  -848,
    1376,   544,  -848,  -848,  -848,   427,  -848,  -848,  -848,  -848,
     557,   679,   125,   563,    23,  -848,    23,   427,    23,    23,
     753,  -848,   802,   356,   795,   796,    -2,    -2,   799,  -848,
     427,   427,   427,   427,    97,  -848,  -848,   822,  -848,  -848,
     812,  -848,   -25,    -1,  -848,    99,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   214,
     427,   427,   427,   427,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  1209,
    -848,   427,  -848,  -848,  -848,  -848,  -848,  -848,   787,   788,
     595,    34,   803,   154,   485,    34,  -848,  -848,   695,    34,
     811,    34,   706,   813,   705,    34,    34,   154,   815,   622,
     823,   623,   154,   634,  -848,  -848,   619,  -848,  -848,  -848,
     161,  -848,  -848,   869,   637,   638,  -848,  -848,  -848,   639,
    -848,   645,   824,  -848,  -848,  -848,   646,   648,   649,    34,
      34,   650,   670,   701,  -848,  -848,   252,  -848,   -16,  -848,
    -848,  -848,   372,  -848,  -848,  -848,    34,  -848,    34,  -848,
    -848,  -848,  -848,    34,  -848,   485,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,   321,  -848,  -848,  -848,   916,  -848,
    -848,  -848,  -848,   903,   934,  -848,  1209,  -848,  -848,  -848,
     485,  -848,  -848,   936,  -848,  -848,   965,   965,   965,   965,
    -848,  -848,  -848,   703,   733,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,   363,  -848,  -848,   831,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,    22,   734,
     765,  -848,  -848,  -848,   485,    34,  -848,  -848,   621,   679,
    -848,  -848,   855,  -848,  -848,  -848,  -848,    51,  -848,   736,
    -848,  -848,  -848,  -848,  -848,   885,   106,   132,   156,  -848,
     427,   790,  1024,   159,  -848,  -848,  -848,  1036,  1034,  -848,
    1428,  1428,  1428,  1428,  1428,  1428,  1428,  1428,  1428,  -848,
     804,  -848,   594,   594,  -848,  -848,  1026,  -848,   800,  -848,
    -848,  -848,  1404,    34,    34,  -848,   805,   154,   806,   809,
     818,    34,   821,    34,   826,   485,    34,   833,   834,   837,
     838,    34,  -848,    34,  -848,   840,  -848,  -848,  -848,    32,
     810,   845,   485,   846,  -848,   820,   849,  -848,  -848,  -118,
    -848,  -848,   842,   850,  -848,  -848,  -848,   851,   852,  -848,
    -848,  -848,    34,   146,   351,   853,    34,   485,  -848,  -848,
    -848,  -848,  -848,  -848,   -16,  -848,  -848,  -848,   482,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  1101,  1103,  1112,  1113,
    -848,  -848,  -848,  1376,  -848,  -848,  -848,   679,   590,  -848,
    -848,  -848,  -848,   380,  -848,  -848,   427,  -848,  -848,   427,
     427,   427,   908,  1105,   159,  -848,  -848,   865,   442,  -848,
     427,  -848,   419,  -848,   870,   867,  -848,   872,  -848,  -848,
    -848,   873,  -848,   875,  -848,   876,   878,  -848,  -848,  -848,
    -848,   881,   882,  -848,  -848,  -848,    34,  -848,  -848,   433,
    -848,  -848,   883,  -848,  -848,  -848,  -848,  -848,  -848,   824,
    -848,  -848,   886,  1116,  1116,  1116,  1116,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,    -7,  -848,  1118,
    -848,  -848,   890,   427,  -848,  -848,  -848,   427,   427,   893,
     410,   898,   384,  1030,  -848,   679,  -848,   917,   940,   949,
    -848,   900,  -848,  1138,   972,   485,  -848,  -848,  -848,  -848,
      34,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   218,  -848,
       2,  -848,  -848,   220,   169,  -848,   252,  -848,   901,   904,
     905,   910,   427,   356,   427,  -848,   241,  1381,   186,  -848,
    1134,   271,  -848,  1404,   907,   912,  -848,  -848,  -848,  -848,
    -848,  -848,   911,  -848,  -848,   913,  -848,  -848,  -848,  -188,
    -848,  -848,  -848,   179,    34,    34,    34,   997,    24,    26,
    -848,  -848,   918,    34,   274,  -848,   919,  -848,   252,   252,
     252,   252,   572,   921,   636,   288,  -848,   998,    71,   427,
     427,   307,   384,  -848,  -848,  -848,  1107,   923,  -848,   154,
      34,   154,  -848,   927,  -848,  -848,  -848,    34,    34,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   929,
     590,  1053,  -848,   937,   939,   941,   671,   766,   944,   948,
     954,   963,  -848,   964,    34,   154,    34,    34,    34,  1049,
      34,   157,   129,   121,  1015,    34,  1005,  -848,  -848,   970,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,   974,   969,   998,
     590,  1030,  -848,  -848,  -848,   427,   427,   427,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,    34,  -848,    34,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,   154,  -848,   130,  -848,  -848,
    -848,  -848,  -848,    71,   981,  1004,  1013,    16,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,   977,   978,   979,  -169,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,   982,  -848,  -848
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -848,  -848,  -848,  -117,  -848,  -192,  -175,  -223,   744,  -848,
    -286,  -848,  -848,  -848,  -848,  -848,   562,  -848,  -848,  -848,
    1148,  -848,  -848,  -848,  -848,  -848,  -848,  -195,  -848,  -848,
    -848,  -848,  -848,   691,  1124,  1068,  -848,  -848,  -848,  -848,
    -848,   537,  -413,  -848,  -848,  -848,  -848,   -65,  -848,   583,
    -777,   504,  -423,  -848,  -234,  -848,  -848,  -848,   571,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   494,
    -848,  -848,  -848,  -848,  -522,   565,  -848,   953,   759,   989,
    -848,   710,  -848,  -848,  -711,   301,  -848,  -138,   558,  -132,
    -848,  -134,   685,  -848,   272,  -848,  -848,  -265,   344,  -848,
    -848,  -664,  -848,    66,   489,   -53,  -848,  -848,   388,   570,
     463,  -847,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,    42,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -717
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -296
static const yytype_int16 yytable[] =
{
     150,   286,   413,   416,   319,   449,   398,   417,   323,   633,
     321,   306,   646,   812,   770,   629,   682,   255,   396,   397,
     912,   388,   389,   246,   565,   566,   851,   877,   247,   540,
     279,   541,   998,   543,   544,   492,   294,   283,   378,   675,
     295,   284,   457,  1006,   459,   283,    42,   283,   370,   284,
     296,   284,   282,   821,     1,    87,   441,   350,   351,   352,
     353,   354,   826,   878,   371,   372,   373,   374,   402,   379,
     380,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     309,   112,  1007,   418,   315,   316,    88,   695,   314,   315,
     316,   283,   127,    90,   503,   284,   890,   431,   367,   938,
     437,   438,   912,   350,   351,   352,   353,   354,   888,   770,
     119,   256,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   736,   252,   703,   350,   351,
     352,   353,   354,   283,   676,   442,   443,   284,   909,   433,
     381,   865,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   504,   350,   351,   352,   353,   354,   283,   120,    43,
     283,   284,   315,   316,   284,   695,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   260,   371,   372,   373,   374,
     301,   972,   350,   351,   352,   353,   354,    44,   121,   948,
     913,   598,    45,   770,   105,    46,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   610,   533,   534,   535,   599,
     615,   743,   261,   744,   106,    47,    48,   618,    49,   107,
     563,   726,   517,   564,   891,    50,   647,   892,   122,   982,
     983,   967,   813,   248,   542,   280,   770,   350,   351,   352,
     353,   354,   567,   827,   307,    51,   838,   555,   556,   557,
     558,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     999,   252,   297,   285,   596,    52,   879,   650,   600,   298,
     123,   285,   602,   285,   604,   770,   770,   770,   608,   609,
     654,   870,   253,   326,   213,   871,   151,   570,   571,   572,
     573,   574,   575,   576,   577,   578,   686,   582,   583,   584,
     585,   798,    87,   642,   215,   654,   633,    53,   434,   435,
     317,   251,   637,   638,   969,   317,   113,   285,   592,   973,
     257,   117,   974,   579,   970,   971,   853,   118,   266,   651,
     643,   652,   262,   263,   267,   114,   653,   559,   252,   580,
     991,   992,   560,   268,   568,   581,   115,   569,   619,   679,
     301,   689,   301,   269,   301,   301,   644,   124,   270,   125,
     126,   273,   127,   128,   129,   620,   536,   621,   880,   285,
     271,   130,   131,   622,   132,   272,   133,   690,   396,   397,
     649,   844,    83,    84,   881,   124,   854,   125,   126,   730,
     127,   128,   129,   285,   275,   707,   285,   276,   317,   130,
     131,   691,   132,    85,   133,   326,   277,    54,   680,   134,
     845,   846,   847,   855,   848,   319,   745,   727,   746,   849,
     715,   696,   451,   882,   323,   396,   397,   785,   124,   868,
     125,   126,   278,   127,   128,   129,   287,   134,   288,   289,
     918,   919,   130,   131,   920,   132,   921,   133,   809,   810,
     811,   335,   336,   337,   338,   339,   340,   341,   342,   895,
     302,   836,   749,   703,   850,   303,   704,   705,   452,   786,
     343,   344,   345,   304,   711,   327,   713,   135,   136,   716,
     134,   305,   866,   163,   721,   310,   722,   311,   137,   138,
     139,   396,   397,   140,   109,   110,   111,   692,   241,   896,
     897,   898,   899,   164,   312,   135,   136,   165,   166,    91,
      92,   168,   169,   405,   406,   742,   137,   138,   139,   748,
     313,   140,   346,   347,   801,    93,   242,    94,    95,   667,
     668,   669,   364,   328,   329,   365,   330,   331,   332,   333,
     334,   409,   410,   468,   469,   470,   319,   366,   135,   136,
     471,   472,   834,   473,   453,   454,   424,   425,   368,   137,
     138,   139,   857,   385,   140,   657,   658,   839,   350,   351,
     352,   353,   354,   335,   336,   337,   338,   339,   340,   341,
     342,   163,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   387,   343,   344,   345,   141,  -295,   757,   386,   799,
     390,   164,   391,   142,   393,   165,   166,   672,   673,   168,
     169,   362,   363,   392,   902,   903,   904,   905,   394,    96,
     774,   775,   395,   141,   758,  -290,   777,   778,   779,   399,
      98,   142,   350,   351,   352,   353,   354,   784,    99,   100,
     552,   553,   400,   759,   346,   347,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   401,   259,   264,   265,   101,
     102,   103,   104,   835,   403,   404,   141,   350,   351,   352,
     353,   354,   407,   408,   142,    97,   411,   941,   412,   943,
     415,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     760,   761,   762,   763,   764,   765,   766,   414,   243,   419,
     816,   421,   422,   423,   817,   818,   426,   767,   430,   823,
     768,   427,   432,   439,   440,   444,   445,   884,   885,   886,
     319,   889,   446,   961,   447,   450,   894,   455,   519,   520,
     521,   522,   523,   198,   524,   525,   526,   527,   199,   153,
     200,   456,   458,   460,   463,   162,   461,   462,   464,   862,
     173,   864,   197,   942,   206,   545,   211,   466,   227,   467,
     945,   946,   350,   351,   352,   353,   354,   476,   477,   479,
     480,   495,   500,   990,   501,   502,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   516,   519,   520,   521,   522,
     523,   163,   524,   525,   526,   527,   201,   960,   518,   962,
     963,   964,   538,   966,   968,   546,   916,   917,   976,   823,
     550,   164,   551,   906,   478,   165,   166,   554,   167,   168,
     169,   496,   497,   498,   499,   561,   562,   350,   351,   352,
     353,   354,   593,   594,  -194,   319,   595,   597,   988,   601,
     989,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     993,   350,   351,   352,   353,   354,   603,   605,   606,   607,
     611,   681,   617,   612,   614,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   613,   616,   626,   908,   627,   628,
     630,   350,   351,   352,   353,   354,   631,   634,   213,   635,
     636,   639,   984,   985,   986,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   350,   351,   352,   353,   354,   659,
     660,   640,   953,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   350,   351,   352,   353,
     354,   661,   641,   664,   670,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   350,   351,
     352,   353,   354,   665,   671,   677,   687,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     350,   351,   352,   353,   354,   757,   170,   954,   678,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   758,  -194,  -194,   693,  -194,   757,   694,   697,
     698,   163,   701,   700,   154,   703,   706,   708,   163,   155,
     709,   759,   729,   728,   233,   234,   235,   156,   236,   710,
     163,   164,   712,   734,   758,   165,   166,   714,   164,   168,
     169,   674,   165,   166,   717,   718,   168,   169,   719,   720,
     164,   723,   157,   759,   165,   166,   738,   733,   168,   169,
     735,   739,   740,   741,   751,   685,   752,   747,   760,   761,
     762,   763,   764,   765,   766,   753,   754,   228,   781,     4,
     783,   789,   790,   791,   792,   767,   793,   794,   768,   795,
       5,     6,   796,   797,   807,   688,   802,   806,   814,   815,
     760,   761,   762,   763,   764,   765,   766,   819,   163,   822,
     831,   832,   858,   174,   869,   859,   860,   767,   780,  -194,
     768,   861,   872,   873,   875,   874,   887,   828,   164,   893,
     901,   907,   165,   166,   940,     7,   168,   169,   944,   947,
       8,   213,   949,   175,   163,     9,    10,    11,    12,   950,
     829,   951,   158,   955,   196,   952,   202,   956,   210,   830,
     223,   215,   230,   957,   164,   240,   245,   176,   165,   166,
     923,   212,   168,   169,   958,   177,   178,   959,   965,   213,
     975,   978,   833,   980,   977,   979,   589,   159,  1003,  1004,
    1005,   995,   160,  1008,   663,   755,   250,   384,   214,   215,
     788,   750,   756,   805,   531,   662,   179,   494,   161,   684,
     981,   910,   782,   699,   996,   994,   776,   207,   208,   820,
     922,   180,   181,   997,   182,   183,   237,   184,   185,   186,
       0,   187,     0,   229,   188,   189,   863,   190,   924,  -194,
       0,     0,     0,     0,  -194,   209,   481,     0,     0,     0,
     191,   192,     0,     0,     0,   193,     0,   194,     0,   396,
     397,     0,     0,     0,   925,     0,   926,   927,   928,   929,
     930,   931,     0,     0,   932,     0,   933,   934,     0,     0,
     935,   482,     0,     0,     0,   936,   519,   520,   521,   522,
     523,     0,   524,   525,   526,   527,     0,     0,     0,     0,
       0,     0,   483,     0,     0,   484,     0,     0,     0,     0,
       0,   485,   486,     0,   216,     0,     0,     0,     0,     0,
       0,   937,     0,   195,     0,   487,   488,   489,   490,     0,
     491,     0,     0,     0,     0,     0,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,   221,   350,   351,   352,
     353,   354,     0,     0,     0,     0,     0,     0,     0,   222,
       0,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     350,   351,   352,   353,   354,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   381,  -296,  -296,  -296,  -296,  -296,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -296,  -296,
    -296,  -296,  -296,   360,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   867
};

static const yytype_int16 yycheck[] =
{
      53,   118,   194,   198,   142,   228,   181,   199,   142,   432,
     142,    13,    28,    20,   678,   428,   538,     3,    16,    17,
     867,   170,   171,    33,    25,    26,   803,   215,    38,   294,
      38,   296,    16,   298,   299,   258,    13,    13,    38,    17,
      17,    17,   234,   212,   236,    13,     0,    13,   165,    17,
      27,    17,   117,   770,     4,    18,   153,     6,     7,     8,
       9,    10,   773,   251,    32,    33,    34,    35,   185,    69,
      70,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     133,    17,   251,   200,    13,    14,    49,    16,   141,    13,
      14,    13,    16,     3,    16,    17,    70,   214,   163,   876,
     217,   218,   949,     6,     7,     8,     9,    10,    84,   773,
      17,    97,     6,     7,     8,     9,    10,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   253,    17,   255,     6,     7,
       8,     9,    10,    13,   122,   242,   243,    17,   865,    70,
     150,   815,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   278,     6,     7,     8,     9,    10,    13,    61,     3,
      13,    17,    13,    14,    17,    16,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    66,    32,    33,    34,    35,
     124,    70,     6,     7,     8,     9,    10,    31,    17,   910,
     129,   393,    36,   867,    13,    39,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   407,    91,    92,    93,   394,
     412,    75,   103,    77,    33,    59,    60,    66,    62,    38,
     255,   199,   285,   258,   208,    69,   252,   211,    90,   950,
     951,    84,   249,   253,   297,   253,   910,     6,     7,     8,
       9,    10,   253,   775,   256,    89,   254,   310,   311,   312,
     313,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     254,    17,   249,   249,   391,   109,    97,   452,   395,   256,
      17,   249,   399,   249,   401,   949,   950,   951,   405,   406,
     465,    20,    38,     5,    74,    24,    17,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   255,   360,   361,   362,
     363,   724,    18,    61,    94,   490,   739,   151,   239,   240,
     249,     3,   439,   440,   195,   249,    18,   249,   381,   208,
       3,    80,   211,   119,   205,   206,   167,    86,     3,   456,
      88,   458,   223,   224,     3,    37,   463,   250,    17,   135,
     220,   221,   255,     3,   255,   141,    48,   258,   197,   534,
     294,   255,   296,     3,   298,   299,   114,    11,     3,    13,
      14,    13,    16,    17,    18,   214,   251,   216,   199,   249,
       3,    25,    26,   222,    28,     3,    30,   255,    16,    17,
      18,   171,    17,    18,   215,    11,   227,    13,    14,   622,
      16,    17,    18,   249,     3,   597,   249,     3,   249,    25,
      26,   255,    28,    38,    30,     5,     3,   251,   535,    63,
     200,   201,   202,   254,   204,   563,    75,   619,    77,   209,
     605,   563,   115,   254,   568,    16,    17,    18,    11,   253,
      13,    14,    20,    16,    17,    18,    13,    63,    15,    16,
     143,   144,    25,    26,   147,    28,   149,    30,   744,   745,
     746,   173,   174,   175,   176,   177,   178,   179,   180,   195,
      28,   253,   647,   255,   254,   249,   593,   594,   161,   702,
     192,   193,   194,   149,   601,    75,   603,   131,   132,   606,
      63,   149,   251,    39,   611,   249,   613,   249,   142,   143,
     144,    16,    17,   147,    10,    11,    12,   560,    54,   235,
     236,   237,   238,    59,   249,   131,   132,    63,    64,    80,
      81,    67,    68,   174,   175,   642,   142,   143,   144,   646,
     249,   147,   244,   245,   729,    96,    82,    98,    99,   497,
     498,   499,   253,   133,   134,   253,   136,   137,   138,   139,
     140,   183,   184,    71,    72,    73,   694,   253,   131,   132,
      78,    79,   785,    81,   247,   248,   228,   229,    17,   142,
     143,   144,   806,   251,   147,   254,   255,   800,     6,     7,
       8,     9,    10,   173,   174,   175,   176,   177,   178,   179,
     180,    39,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   253,   192,   193,   194,   249,   250,    17,   251,   726,
      18,    59,   189,   257,   185,    63,    64,   254,   255,    67,
      68,    27,    28,   172,   858,   859,   860,   861,    92,   190,
     250,   251,   154,   249,    44,   251,   689,   690,   691,    55,
      72,   257,     6,     7,     8,     9,    10,   700,    80,    81,
     306,   307,   157,    63,   244,   245,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    55,    95,    96,    97,   101,
     102,   103,   104,   790,   155,   163,   249,     6,     7,     8,
       9,    10,   167,   169,   257,   246,   183,   879,   187,   881,
     253,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     110,   111,   112,   113,   114,   115,   116,   251,   254,   251,
     763,   254,   253,    59,   767,   768,   251,   127,    18,   772,
     130,   253,   253,    55,    55,   251,   251,   844,   845,   846,
     868,   848,   253,   925,   253,   251,   853,   251,   117,   118,
     119,   120,   121,   191,   123,   124,   125,   126,   196,    58,
     198,    59,    55,   251,    55,    64,   251,   251,    18,   812,
      69,   814,    71,   880,    73,    12,    75,   251,    77,   251,
     887,   888,     6,     7,     8,     9,    10,     3,     3,     3,
       3,     3,     3,   975,     3,     3,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   251,   117,   118,   119,   120,
     121,    39,   123,   124,   125,   126,   254,   924,   251,   926,
     927,   928,   249,   930,   931,    13,   869,   870,   935,   872,
      25,    59,    26,   251,   253,    63,    64,    28,    66,    67,
      68,   260,   261,   262,   263,    13,    24,     6,     7,     8,
       9,    10,    55,    55,   254,   983,   251,    44,   965,   154,
     967,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     977,     6,     7,     8,     9,    10,    55,   161,    55,   164,
      55,   250,   253,   251,   251,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    61,   251,    17,   251,   251,   251,
     251,     6,     7,     8,     9,    10,   251,   251,    74,   251,
     251,   251,   955,   956,   957,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     6,     7,     8,     9,    10,     3,
      17,   251,   251,     6,     7,     8,     9,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     6,     7,     8,     9,
      10,    17,   251,    17,   251,     6,     7,     8,     9,    10,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     6,     7,
       8,     9,    10,    18,   251,   251,   250,     6,     7,     8,
       9,    10,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       6,     7,     8,     9,    10,    17,   254,   251,   253,     6,
       7,     8,     9,    10,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    44,    13,    14,   255,    16,    17,    24,    13,
      16,    39,    26,   249,    52,   255,   251,   251,    39,    57,
     251,    63,   217,   253,    52,    53,    54,    65,    56,   251,
      39,    59,   251,   253,    44,    63,    64,   251,    59,    67,
      68,   250,    63,    64,   251,   251,    67,    68,   251,   251,
      59,   251,    90,    63,    63,    64,   254,   251,    67,    68,
     251,   251,   251,   251,     3,   250,     3,   254,   110,   111,
     112,   113,   114,   115,   116,     3,     3,    98,    13,    39,
     255,   251,   255,   251,   251,   127,   251,   251,   130,   251,
      50,    51,   251,   251,    18,   250,   253,   251,    20,   249,
     110,   111,   112,   113,   114,   115,   116,   254,    39,   251,
     250,    13,   251,    44,    20,   251,   251,   127,   250,   129,
     130,   251,   255,   251,   251,   254,   169,   250,    59,   251,
     251,   250,    63,    64,   251,    95,    67,    68,   251,   250,
     100,    74,   129,    74,    39,   105,   106,   107,   108,   252,
     250,   252,   190,   249,    70,   254,    72,   249,    74,   250,
      76,    94,    78,   249,    59,    81,    82,    98,    63,    64,
     103,    66,    67,    68,   251,   106,   107,   253,   169,    74,
     205,   251,   250,   254,   219,   251,    17,   225,   251,   251,
     251,   250,   230,   251,   490,   673,    88,   169,    93,    94,
     703,   658,   677,   739,   291,   486,   137,   258,   246,   539,
     949,   253,   694,   568,   250,   983,   686,   226,   227,   770,
     872,   152,   153,   250,   155,   156,   254,   158,   159,   160,
      -1,   162,    -1,   254,   165,   166,   813,   168,   171,   249,
      -1,    -1,    -1,    -1,   254,   254,     3,    -1,    -1,    -1,
     181,   182,    -1,    -1,    -1,   186,    -1,   188,    -1,    16,
      17,    -1,    -1,    -1,   197,    -1,   199,   200,   201,   202,
     203,   204,    -1,    -1,   207,    -1,   209,   210,    -1,    -1,
     213,    38,    -1,    -1,    -1,   218,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    68,    69,    -1,   209,    -1,    -1,    -1,    -1,    -1,
      -1,   254,    -1,   254,    -1,    82,    83,    84,    85,    -1,
      87,    -1,    -1,    -1,    -1,    -1,   231,   232,   233,   234,
      -1,    -1,    -1,    -1,    -1,    -1,   241,     6,     7,     8,
       9,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   254,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       6,     7,     8,     9,    10,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    20,    21,    22,    23,    24,    25,
      26,    27,    28,   150,     6,     7,     8,     9,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   260,   261,    39,    50,    51,    95,   100,   105,
     106,   107,   108,   276,   277,   278,   314,   315,   316,   318,
     319,   320,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   334,   335,   336,   337,   412,   413,   414,   415,   416,
     432,   434,     0,     3,    31,    36,    39,    59,    60,    62,
      69,    89,   109,   151,   251,   263,   271,   280,   282,   285,
     287,   290,   291,   296,   302,   303,   339,   341,   351,   381,
     382,   383,   384,   417,   418,   419,   420,   428,   429,   433,
     435,   436,   437,    17,    18,    38,   338,    18,    49,   279,
       3,    80,    81,    96,    98,    99,   190,   246,    72,    80,
      81,   101,   102,   103,   104,    13,    33,    38,   332,   338,
     338,   338,    17,    18,    37,    48,   272,    80,    86,    17,
      61,    17,    90,    17,    11,    13,    14,    16,    17,    18,
      25,    26,    28,    30,    63,   131,   132,   142,   143,   144,
     147,   249,   257,   356,   360,   365,   368,   369,   370,   372,
     374,    17,   292,   292,    52,    57,    65,    90,   190,   225,
     230,   246,   292,    39,    59,    63,    64,    66,    67,    68,
     254,   293,   305,   292,    44,    74,    98,   106,   107,   137,
     152,   153,   155,   156,   158,   159,   160,   162,   165,   166,
     168,   181,   182,   186,   188,   254,   293,   292,   191,   196,
     198,   254,   293,   385,   386,   387,   292,   226,   227,   254,
     293,   292,    66,    74,    93,    94,   209,   231,   232,   233,
     234,   241,   254,   293,   309,   311,   312,   292,    98,   254,
     293,   430,   431,    52,    53,    54,    56,   254,   281,   284,
     293,    54,    82,   254,   283,   293,    33,    38,   253,   306,
     279,     3,    17,    38,   317,     3,    97,     3,   321,   317,
      66,   103,   223,   224,   317,   317,     3,     3,     3,     3,
       3,     3,     3,    13,   333,     3,     3,     3,    20,    38,
     253,   273,   306,    13,    17,   249,   262,    13,    15,    16,
     286,   340,   288,   352,    13,    17,    27,   249,   256,   364,
     366,   372,    28,   249,   149,   149,    13,   256,   367,   374,
     249,   249,   249,   249,   374,    13,    14,   249,   355,   356,
     357,   358,   359,   360,   361,   362,     5,    75,   133,   134,
     136,   137,   138,   139,   140,   173,   174,   175,   176,   177,
     178,   179,   180,   192,   193,   194,   244,   245,   373,   438,
       6,     7,     8,     9,    10,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   253,   253,   253,   306,    17,   295,
     262,    32,    33,    34,    35,   262,   264,   304,    38,    69,
      70,   150,   294,   348,   294,   251,   251,   253,   170,   171,
      18,   189,   172,   185,    92,   154,    16,    17,   265,    55,
     157,    55,   262,   155,   163,   174,   175,   167,   169,   183,
     184,   183,   187,   264,   251,   253,   286,   264,   262,   251,
     388,   254,   253,    59,   228,   229,   251,   253,   426,   427,
      18,   262,   253,    70,   239,   240,   421,   262,   262,    55,
      55,   153,   242,   243,   251,   251,   253,   253,   265,   266,
     251,   115,   161,   247,   248,   251,    59,   264,    55,   264,
     251,   251,   251,    55,    18,   270,   251,   251,    71,    72,
      73,    78,    79,    81,   307,   308,     3,     3,   317,     3,
       3,     3,    38,    59,    62,    68,    69,    82,    83,    84,
      85,    87,   266,   322,   348,     3,   317,   317,   317,   317,
       3,     3,     3,    16,   262,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   274,   275,   251,   374,   251,   117,
     118,   119,   120,   121,   123,   124,   125,   126,   342,   343,
     344,   346,   350,    91,    92,    93,   251,   289,   249,   349,
     366,   366,   374,   366,   366,    12,    13,   374,   378,   379,
      25,    26,   367,   367,    28,   374,   374,   374,   374,   250,
     255,    13,    24,   255,   258,    25,    26,   253,   255,   258,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   119,
     135,   141,   374,   374,   374,   374,   297,   300,   301,    17,
     346,   347,   374,    55,    55,   251,   262,    44,   264,   265,
     262,   154,   262,    55,   262,   161,    55,   164,   262,   262,
     264,    55,   251,    61,   251,   264,   251,   253,    66,   197,
     214,   216,   222,   389,   394,   397,    17,   251,   251,   301,
     251,   251,   310,   311,   251,   251,   251,   262,   262,   251,
     251,   251,    61,    88,   114,   313,    28,   252,   268,    18,
     265,   262,   262,   262,   265,   266,   267,   254,   255,     3,
      17,    17,   347,   267,    17,    18,   411,   411,   411,   411,
     251,   251,   254,   255,   250,    17,   122,   251,   253,   265,
     262,   250,   343,   345,   350,   250,   255,   250,   250,   255,
     255,   255,   374,   255,    24,    16,   358,    13,    16,   361,
     249,    26,   299,   255,   262,   262,   251,   264,   251,   251,
     251,   262,   251,   262,   251,   265,   262,   251,   251,   251,
     251,   262,   262,   251,   407,   410,   199,   264,   253,   217,
     266,   390,   391,   251,   253,   251,   253,   425,   254,   251,
     251,   251,   262,    75,    77,    75,    77,   254,   262,   265,
     308,     3,     3,     3,     3,   275,   344,    17,    44,    63,
     110,   111,   112,   113,   114,   115,   116,   127,   130,   353,
     370,   371,   375,   380,   250,   251,   378,   374,   374,   374,
     250,    13,   357,   255,   374,    18,   266,   298,   300,   251,
     255,   251,   251,   251,   251,   251,   251,   251,   301,   262,
     399,   286,   253,   395,   424,   310,   251,    18,   269,   269,
     269,   269,    20,   249,    20,   249,   374,   374,   374,   254,
     373,   438,   251,   374,   376,   377,   353,   343,   250,   250,
     250,   250,    13,   250,   266,   262,   253,   409,   254,   266,
     400,   401,   402,   392,   171,   200,   201,   202,   204,   209,
     254,   309,   396,   167,   227,   254,   423,   313,   251,   251,
     251,   251,   374,   379,   374,   370,   251,   128,   253,    20,
      20,    24,   255,   251,   254,   251,   408,   215,   251,    97,
     199,   215,   254,   393,   262,   262,   262,   169,    84,   262,
      70,   208,   211,   251,   262,   195,   235,   236,   237,   238,
     422,   251,   313,   313,   313,   313,   251,   250,   251,   438,
     253,   354,   380,   129,   358,   363,   374,   374,   143,   144,
     147,   149,   377,   103,   171,   197,   199,   200,   201,   202,
     203,   204,   207,   209,   210,   213,   218,   254,   309,   398,
     251,   264,   262,   264,   251,   262,   262,   250,   353,   129,
     252,   252,   254,   251,   251,   249,   249,   249,   251,   253,
     262,   264,   262,   262,   262,   169,   262,    84,   262,   195,
     205,   206,    70,   208,   211,   205,   262,   219,   251,   251,
     254,   354,   353,   353,   374,   374,   374,   403,   262,   262,
     264,   220,   221,   262,   363,   250,   250,   250,    16,   254,
     404,   405,   406,   251,   251,   251,   212,   251,   251
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
#line 293 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 294 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 307 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 308 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 312 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 316 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 325 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 326 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 327 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 328 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 329 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 330 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 337 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 344 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 352 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 356 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 360 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 367 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 375 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 376 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 382 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 391 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 396 "cf-parse.y"
    { (yyval.g) = NULL; ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 397 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 401 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 402 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 406 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 407 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 411 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 412 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 413 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 414 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 415 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 416 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 417 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 418 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 419 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 427 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 430 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 433 "cf-parse.y"
    { cli_msg(7, "Shutdown requested"); order_shutdown(); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 436 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 445 "cf-parse.y"
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

  case 50:

/* Line 1455 of yacc.c  */
#line 461 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 462 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 466 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[(2) - (2)].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[(2) - (2)].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 478 "cf-parse.y"
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

  case 54:

/* Line 1455 of yacc.c  */
#line 491 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 495 "cf-parse.y"
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[(2) - (3)].t);
     kpi->prefix = (yyvsp[(3) - (3)].px).addr;
     kpi->pxlen = (yyvsp[(3) - (3)].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 508 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 520 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 526 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 528 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[(1) - (1)].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 547 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 548 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 549 "cf-parse.y"
    { new_config->listen_bgp_flags |= SKF_V6ONLY; ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 556 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 568 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 574 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 582 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 255) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 586 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 587 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 588 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 589 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 590 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 594 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 596 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 597 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 601 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 609 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 610 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 616 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 624 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 625 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 626 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 630 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 631 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 648 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 663 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 678 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 679 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 680 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 685 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 689 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 690 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 691 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 692 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 693 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 694 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 715 "cf-parse.y"
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

  case 118:

/* Line 1455 of yacc.c  */
#line 733 "cf-parse.y"
    { ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 734 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 735 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 736 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 737 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 738 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 747 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 750 "cf-parse.y"
    { proto_show((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 753 "cf-parse.y"
    { proto_show((yyvsp[(4) - (5)].s), 1); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 757 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 761 "cf-parse.y"
    { if_show(); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 764 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 767 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 770 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 776 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 782 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 789 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 794 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 799 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 804 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 808 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 812 "cf-parse.y"
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

  case 141:

/* Line 1455 of yacc.c  */
#line 822 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 830 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 834 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 841 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 842 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 846 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 850 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 852 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 854 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 856 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 858 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 860 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 862 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 864 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 870 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 871 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 876 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 877 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 884 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), 0); ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 886 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), 1); ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 888 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), 2); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 892 "cf-parse.y"
    { proto_debug((yyvsp[(2) - (4)].t), (yyvsp[(3) - (4)].i)); ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 896 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].s)->name; ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 897 "cf-parse.y"
    { (yyval.t) = "*"; ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 904 "cf-parse.y"
    { cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 904 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s) = cf_define_symbol((yyvsp[(2) - (4)].s), SYM_FILTER, (yyvsp[(4) - (4)].f));
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 913 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 917 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 918 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 919 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 920 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 921 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 922 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 923 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 924 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 925 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 926 "cf-parse.y"
    { 
	switch ((yyvsp[(1) - (2)].i)) {
	  case T_INT:
	  case T_IP:
	  case T_PAIR:
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

  case 180:

/* Line 1455 of yacc.c  */
#line 945 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 956 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 957 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 964 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 965 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 972 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 981 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 989 "cf-parse.y"
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

  case 189:

/* Line 1455 of yacc.c  */
#line 1013 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1014 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1018 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1024 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1027 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1037 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1038 "cf-parse.y"
    {
     if ((yyvsp[(1) - (2)].x)) {
       if ((yyvsp[(1) - (2)].x)->next)
	 bug("Command has next already set");
       (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x);
       (yyval.x) = (yyvsp[(1) - (2)].x);
     } else (yyval.x) = (yyvsp[(2) - (2)].x);
   ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1049 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1052 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1061 "cf-parse.y"
    { (yyval.i) = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1068 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1072 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1073 "cf-parse.y"
    { (yyval.v).type = T_PAIR; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1074 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1075 "cf-parse.y"
    {  (yyval.v).type = (yyvsp[(1) - (1)].i) >> 16; (yyval.v).val.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1079 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (1)].v); 
	(yyval.e)->to = (yyvsp[(1) - (1)].v);
   ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1084 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (4)].v); 
	(yyval.e)->to = (yyvsp[(4) - (4)].v); 
   ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1092 "cf-parse.y"
    { (yyval.e) = (yyvsp[(1) - (1)].e); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1093 "cf-parse.y"
    { (yyval.e) = (yyvsp[(3) - (3)].e); (yyval.e)->left = (yyvsp[(1) - (3)].e); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1097 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1104 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1105 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1106 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1107 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1114 "cf-parse.y"
    { (yyval.trie) = f_new_trie(); trie_add_prefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1115 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_prefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1118 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1119 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[(1) - (4)].e);
     (yyval.e)->data = (yyvsp[(3) - (4)].x);
     (yyval.e)->left = (yyvsp[(4) - (4)].e);
   ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1124 "cf-parse.y"
    {
     (yyval.e) = f_new_tree(); 
     (yyval.e)->from.type = T_VOID; 
     (yyval.e)->to.type = T_VOID;
     (yyval.e)->data = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1135 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1136 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1140 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1141 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1145 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1146 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1147 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1148 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1149 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1153 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1154 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1155 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1159 "cf-parse.y"
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

  case 231:

/* Line 1455 of yacc.c  */
#line 1172 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1173 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1174 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1175 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1176 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1177 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1178 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1179 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1180 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1181 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1195 "cf-parse.y"
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

  case 243:

/* Line 1455 of yacc.c  */
#line 1218 "cf-parse.y"
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

  case 244:

/* Line 1455 of yacc.c  */
#line 1251 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1253 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1254 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1255 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1256 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1257 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1258 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1259 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1263 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1264 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1265 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1266 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1267 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1268 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1269 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1270 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1271 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1272 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1273 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1274 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1275 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1276 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1277 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1278 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1280 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1281 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1282 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1284 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1286 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1288 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1290 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1291 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1292 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1302 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1303 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1304 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1305 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1306 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1311 "cf-parse.y"
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

  case 283:

/* Line 1455 of yacc.c  */
#line 1334 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1335 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1336 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1337 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1338 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1339 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1343 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1346 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1347 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1348 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1357 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1364 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1373 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1374 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1378 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1384 "cf-parse.y"
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

  case 299:

/* Line 1455 of yacc.c  */
#line 1394 "cf-parse.y"
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

  case 300:

/* Line 1455 of yacc.c  */
#line 1403 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1409 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1414 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1421 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1426 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1432 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1433 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1434 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1443 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1444 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1445 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1446 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1452 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_bgp, sizeof(struct bgp_config));
     this_proto->preference = DEF_PREF_BGP;
     BGP_CFG->hold_time = 240;
     BGP_CFG->connect_retry_time = 120;
     BGP_CFG->initial_hold_time = 240;
     BGP_CFG->default_med = 0;
     BGP_CFG->compare_path_lengths = 1;
     BGP_CFG->start_delay_time = 5;
     BGP_CFG->error_amnesia_time = 300;
     BGP_CFG->error_delay_time_min = 60;
     BGP_CFG->error_delay_time_max = 300;
     BGP_CFG->enable_as4 = bgp_as4_support;
     BGP_CFG->capabilities = 2;
     BGP_CFG->advertise_ipv4 = 1;
 ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1473 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1474 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1480 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i); ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1481 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1482 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1483 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1484 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1485 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1486 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1487 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (6)].i); BGP_CFG->multihop_via = (yyvsp[(5) - (6)].a); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1488 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1489 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1490 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1491 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1492 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1493 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1494 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1495 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1496 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1497 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1498 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1499 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1500 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1501 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1510 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1526 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1527 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i) ; if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1531 "cf-parse.y"
    {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = (yyvsp[(2) - (3)].i32);
  this_area->stub = 0;
  init_list(&this_area->patt_list);
  init_list(&this_area->vlink_list);
  init_list(&this_area->net_list);
  init_list(&this_area->stubnet_list);
 ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1552 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1553 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1566 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1580 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1581 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1582 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1586 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1596 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1597 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1598 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1599 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1600 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1601 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1602 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1603 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1604 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1609 "cf-parse.y"
    {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->vid = (yyvsp[(3) - (3)].i32);
  OSPF_PATT->cost = COST_D;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->waitint = WAIT_DMH*HELLOINT_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->dead = 0;
  OSPF_PATT->type = OSPF_IT_VLINK;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1630 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1631 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1632 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1633 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1634 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1635 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1636 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1637 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1638 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1639 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1640 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1641 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1642 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1643 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1645 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1646 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1647 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1648 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1649 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1650 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) cf_error("Buffer size is too small") ; ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1664 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1673 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1692 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1701 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1711 "cf-parse.y"
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
  OSPF_PATT->dead = 0;
  OSPF_PATT->type = OSPF_IT_UNDEF;
  OSPF_PATT->strictnbma = 0;
  OSPF_PATT->stub = 0;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1744 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1749 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1754 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1757 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1760 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1763 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1766 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1771 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_OPAQUE;
  ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1781 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1786 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1787 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1793 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1802 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1803 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1804 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1805 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1806 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1807 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1809 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1810 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1811 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1816 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1817 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1818 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1823 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1824 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1825 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1826 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1827 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1831 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1832 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1846 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1862 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1874 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&((struct static_config *) this_proto)->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1883 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1887 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&((struct static_config *) this_proto)->iface_routes, &this_srt->n);
   ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1893 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1894 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1895 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1899 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1907 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1907 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1910 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1911 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1912 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1913 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1914 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1915 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1916 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1917 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1918 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;



/* Line 1455 of yacc.c  */
#line 6259 "cf-parse.tab.c"
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
#line 1920 "cf-parse.y"

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



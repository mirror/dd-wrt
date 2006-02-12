/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse cf_parse
#define yylex   cf_lex
#define yyerror cf_error
#define yylval  cf_lval
#define yychar  cf_char
#define yydebug cf_debug
#define yynerrs cf_nerrs


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
     NUM = 266,
     ENUM = 267,
     RTRID = 268,
     IPA = 269,
     SYM = 270,
     TEXT = 271,
     PREFIX_DUMMY = 272,
     DEFINE = 273,
     ON = 274,
     OFF = 275,
     YES = 276,
     NO = 277,
     LOG = 278,
     SYSLOG = 279,
     ALL = 280,
     DEBUG = 281,
     TRACE = 282,
     INFO = 283,
     REMOTE = 284,
     WARNING = 285,
     ERROR = 286,
     AUTH = 287,
     FATAL = 288,
     BUG = 289,
     STDERR = 290,
     CONFIGURE = 291,
     DOWN = 292,
     KERNEL = 293,
     PERSIST = 294,
     SCAN = 295,
     TIME = 296,
     LEARN = 297,
     DEVICE = 298,
     ASYNC = 299,
     TABLE = 300,
     ROUTER = 301,
     ID = 302,
     PROTOCOL = 303,
     PREFERENCE = 304,
     DISABLED = 305,
     DIRECT = 306,
     INTERFACE = 307,
     IMPORT = 308,
     EXPORT = 309,
     FILTER = 310,
     NONE = 311,
     STATES = 312,
     ROUTES = 313,
     FILTERS = 314,
     PASSWORD = 315,
     FROM = 316,
     PASSIVE = 317,
     TO = 318,
     EVENTS = 319,
     PACKETS = 320,
     PROTOCOLS = 321,
     INTERFACES = 322,
     PRIMARY = 323,
     STATS = 324,
     COUNT = 325,
     FOR = 326,
     COMMANDS = 327,
     PREIMPORT = 328,
     GENERATE = 329,
     SHOW = 330,
     STATUS = 331,
     SUMMARY = 332,
     ROUTE = 333,
     SYMBOLS = 334,
     DUMP = 335,
     RESOURCES = 336,
     SOCKETS = 337,
     NEIGHBORS = 338,
     ATTRIBUTES = 339,
     ECHO = 340,
     DISABLE = 341,
     ENABLE = 342,
     RESTART = 343,
     FUNCTION = 344,
     PRINT = 345,
     PRINTN = 346,
     UNSET = 347,
     RETURN = 348,
     ACCEPT = 349,
     REJECT = 350,
     QUITBIRD = 351,
     INT = 352,
     BOOL = 353,
     IP = 354,
     PREFIX = 355,
     PAIR = 356,
     SET = 357,
     STRING = 358,
     BGPMASK = 359,
     BGPPATH = 360,
     CLIST = 361,
     IF = 362,
     THEN = 363,
     ELSE = 364,
     CASE = 365,
     TRUE = 366,
     FALSE = 367,
     GW = 368,
     NET = 369,
     MASK = 370,
     SOURCE = 371,
     SCOPE = 372,
     CAST = 373,
     DEST = 374,
     LEN = 375,
     DEFINED = 376,
     ADD = 377,
     DELETE = 378,
     CONTAINS = 379,
     RESET = 380,
     PREPEND = 381,
     MATCH = 382,
     EMPTY = 383,
     WHERE = 384,
     EVAL = 385,
     BGP = 386,
     LOCAL = 387,
     NEIGHBOR = 388,
     AS = 389,
     HOLD = 390,
     CONNECT = 391,
     RETRY = 392,
     KEEPALIVE = 393,
     MULTIHOP = 394,
     STARTUP = 395,
     VIA = 396,
     NEXT = 397,
     HOP = 398,
     SELF = 399,
     DEFAULT = 400,
     PATH = 401,
     METRIC = 402,
     START = 403,
     DELAY = 404,
     FORGET = 405,
     WAIT = 406,
     AFTER = 407,
     BGP_PATH = 408,
     BGP_LOCAL_PREF = 409,
     BGP_MED = 410,
     BGP_ORIGIN = 411,
     BGP_NEXT_HOP = 412,
     BGP_ATOMIC_AGGR = 413,
     BGP_AGGREGATOR = 414,
     BGP_COMMUNITY = 415,
     ADDRESS = 416,
     OSPF = 417,
     AREA = 418,
     OSPF_METRIC1 = 419,
     OSPF_METRIC2 = 420,
     OSPF_TAG = 421,
     RFC1583COMPAT = 422,
     STUB = 423,
     TICK = 424,
     COST = 425,
     RETRANSMIT = 426,
     HELLO = 427,
     TRANSMIT = 428,
     PRIORITY = 429,
     DEAD = 430,
     NONBROADCAST = 431,
     POINTOPOINT = 432,
     TYPE = 433,
     SIMPLE = 434,
     AUTHENTICATION = 435,
     STRICT = 436,
     CRYPTOGRAPHIC = 437,
     ELIGIBLE = 438,
     POLL = 439,
     NETWORKS = 440,
     HIDDEN = 441,
     VIRTUAL = 442,
     LINK = 443,
     PIPE = 444,
     PEER = 445,
     RIP = 446,
     INFINITY = 447,
     PORT = 448,
     PERIOD = 449,
     GARBAGE = 450,
     TIMEOUT = 451,
     PASSWORDS = 452,
     MODE = 453,
     BROADCAST = 454,
     MULTICAST = 455,
     QUIET = 456,
     NOLISTEN = 457,
     VERSION1 = 458,
     PLAINTEXT = 459,
     MD5 = 460,
     HONOR = 461,
     NEVER = 462,
     ALWAYS = 463,
     RIP_METRIC = 464,
     RIP_TAG = 465,
     STATIC = 466,
     DROP = 467,
     PROHIBIT = 468
   };
#endif
#define END 258
#define CLI_MARKER 259
#define INVALID_TOKEN 260
#define GEQ 261
#define LEQ 262
#define NEQ 263
#define AND 264
#define OR 265
#define NUM 266
#define ENUM 267
#define RTRID 268
#define IPA 269
#define SYM 270
#define TEXT 271
#define PREFIX_DUMMY 272
#define DEFINE 273
#define ON 274
#define OFF 275
#define YES 276
#define NO 277
#define LOG 278
#define SYSLOG 279
#define ALL 280
#define DEBUG 281
#define TRACE 282
#define INFO 283
#define REMOTE 284
#define WARNING 285
#define ERROR 286
#define AUTH 287
#define FATAL 288
#define BUG 289
#define STDERR 290
#define CONFIGURE 291
#define DOWN 292
#define KERNEL 293
#define PERSIST 294
#define SCAN 295
#define TIME 296
#define LEARN 297
#define DEVICE 298
#define ASYNC 299
#define TABLE 300
#define ROUTER 301
#define ID 302
#define PROTOCOL 303
#define PREFERENCE 304
#define DISABLED 305
#define DIRECT 306
#define INTERFACE 307
#define IMPORT 308
#define EXPORT 309
#define FILTER 310
#define NONE 311
#define STATES 312
#define ROUTES 313
#define FILTERS 314
#define PASSWORD 315
#define FROM 316
#define PASSIVE 317
#define TO 318
#define EVENTS 319
#define PACKETS 320
#define PROTOCOLS 321
#define INTERFACES 322
#define PRIMARY 323
#define STATS 324
#define COUNT 325
#define FOR 326
#define COMMANDS 327
#define PREIMPORT 328
#define GENERATE 329
#define SHOW 330
#define STATUS 331
#define SUMMARY 332
#define ROUTE 333
#define SYMBOLS 334
#define DUMP 335
#define RESOURCES 336
#define SOCKETS 337
#define NEIGHBORS 338
#define ATTRIBUTES 339
#define ECHO 340
#define DISABLE 341
#define ENABLE 342
#define RESTART 343
#define FUNCTION 344
#define PRINT 345
#define PRINTN 346
#define UNSET 347
#define RETURN 348
#define ACCEPT 349
#define REJECT 350
#define QUITBIRD 351
#define INT 352
#define BOOL 353
#define IP 354
#define PREFIX 355
#define PAIR 356
#define SET 357
#define STRING 358
#define BGPMASK 359
#define BGPPATH 360
#define CLIST 361
#define IF 362
#define THEN 363
#define ELSE 364
#define CASE 365
#define TRUE 366
#define FALSE 367
#define GW 368
#define NET 369
#define MASK 370
#define SOURCE 371
#define SCOPE 372
#define CAST 373
#define DEST 374
#define LEN 375
#define DEFINED 376
#define ADD 377
#define DELETE 378
#define CONTAINS 379
#define RESET 380
#define PREPEND 381
#define MATCH 382
#define EMPTY 383
#define WHERE 384
#define EVAL 385
#define BGP 386
#define LOCAL 387
#define NEIGHBOR 388
#define AS 389
#define HOLD 390
#define CONNECT 391
#define RETRY 392
#define KEEPALIVE 393
#define MULTIHOP 394
#define STARTUP 395
#define VIA 396
#define NEXT 397
#define HOP 398
#define SELF 399
#define DEFAULT 400
#define PATH 401
#define METRIC 402
#define START 403
#define DELAY 404
#define FORGET 405
#define WAIT 406
#define AFTER 407
#define BGP_PATH 408
#define BGP_LOCAL_PREF 409
#define BGP_MED 410
#define BGP_ORIGIN 411
#define BGP_NEXT_HOP 412
#define BGP_ATOMIC_AGGR 413
#define BGP_AGGREGATOR 414
#define BGP_COMMUNITY 415
#define ADDRESS 416
#define OSPF 417
#define AREA 418
#define OSPF_METRIC1 419
#define OSPF_METRIC2 420
#define OSPF_TAG 421
#define RFC1583COMPAT 422
#define STUB 423
#define TICK 424
#define COST 425
#define RETRANSMIT 426
#define HELLO 427
#define TRANSMIT 428
#define PRIORITY 429
#define DEAD 430
#define NONBROADCAST 431
#define POINTOPOINT 432
#define TYPE 433
#define SIMPLE 434
#define AUTHENTICATION 435
#define STRICT 436
#define CRYPTOGRAPHIC 437
#define ELIGIBLE 438
#define POLL 439
#define NETWORKS 440
#define HIDDEN 441
#define VIRTUAL 442
#define LINK 443
#define PIPE 444
#define PEER 445
#define RIP 446
#define INFINITY 447
#define PORT 448
#define PERIOD 449
#define GARBAGE 450
#define TIMEOUT 451
#define PASSWORDS 452
#define MODE 453
#define BROADCAST 454
#define MULTICAST 455
#define QUIET 456
#define NOLISTEN 457
#define VERSION1 458
#define PLAINTEXT 459
#define MD5 460
#define HONOR 461
#define NEVER 462
#define ALWAYS 463
#define RIP_METRIC 464
#define RIP_TAG 465
#define STATIC 466
#define DROP 467
#define PROHIBIT 468




/* Copy the first part of user declarations.  */
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
static list *this_p_list;
static struct password_item *this_p_item;

/* Headers from ../../filter/config.Y */

/* Defines from ../../filter/config.Y */

#define P(a,b) ((a<<8) | b)

/* Headers from ../../proto/bgp/config.Y */

#include "proto/bgp/bgp.h"

/* Defines from ../../proto/bgp/config.Y */

#define BGP_CFG ((struct bgp_config *) this_proto)

/* Headers from ../../proto/ospf/config.Y */

#include "proto/ospf/ospf.h"

/* Defines from ../../proto/ospf/config.Y */

#define OSPF_CFG ((struct ospf_config *) this_proto)
static struct ospf_area_config *this_area;
static struct iface_patt *this_ipatt;
#define OSPF_PATT ((struct ospf_iface_patt *) this_ipatt)
static struct nbma_node *this_nbma;
static struct area_net_config *this_pref;

/* Headers from ../../proto/pipe/config.Y */

#include "proto/pipe/pipe.h"

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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 101 "cf-parse.y"
typedef union YYSTYPE {
  int i;
  u32 i32;
  ip_addr a;
  struct symbol *s;
  char *t;
  struct rtable_config *r;
  struct f_inst *x;
  struct filter *f;
  struct f_tree *e;
  struct f_val v;
  struct f_path_mask *h;
  struct password_item *p;
  struct rt_show_data *ra;
  void *g;
  bird_clock_t time;
  struct prefix px;
} YYSTYPE;
/* Line 190 of yacc.c.  */
#line 627 "cf-parse.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 639 "cf-parse.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  39
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1424

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  235
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  165
/* YYNRULES -- Number of rules. */
#define YYNRULES  492
/* YYNRULES -- Number of states. */
#define YYNSTATES  903

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   468

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    28,     2,     2,     2,    27,     2,     2,
     225,   226,    25,    23,   231,    24,    22,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   228,   227,
      19,    18,    20,   232,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   233,     2,   234,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   229,     2,   230,    21,     2,     2,     2,
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
      15,    16,    17,    29,    30,    31,    32,    33,    34,    35,
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
     216,   217,   218,   219,   220,   221,   222,   223,   224
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    13,    15,    19,    21,
      27,    33,    35,    37,    39,    41,    43,    44,    46,    48,
      51,    53,    55,    58,    61,    63,    68,    70,    72,    74,
      76,    80,    82,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   104,   108,   111,   112,   114,   117,   120,   124,
     127,   130,   134,   138,   143,   145,   147,   149,   152,   154,
     155,   157,   158,   161,   164,   167,   170,   173,   176,   179,
     181,   183,   185,   187,   191,   195,   197,   199,   202,   205,
     209,   213,   217,   218,   221,   224,   228,   230,   232,   236,
     238,   242,   244,   246,   248,   250,   252,   254,   255,   259,
     264,   266,   269,   270,   276,   282,   288,   294,   299,   304,
     306,   308,   311,   315,   320,   326,   328,   329,   333,   338,
     343,   344,   347,   351,   355,   359,   362,   365,   368,   372,
     375,   378,   380,   382,   387,   391,   395,   399,   403,   407,
     411,   415,   420,   422,   424,   426,   427,   429,   433,   437,
     441,   446,   448,   450,   452,   453,   458,   461,   463,   465,
     467,   469,   471,   473,   475,   477,   479,   482,   485,   486,
     490,   492,   496,   498,   500,   502,   505,   509,   512,   517,
     518,   524,   525,   528,   530,   534,   540,   544,   546,   549,
     552,   559,   561,   563,   565,   567,   569,   571,   573,   578,
     580,   584,   585,   590,   594,   596,   598,   600,   603,   605,
     607,   609,   611,   613,   615,   617,   621,   623,   627,   628,
     633,   635,   637,   639,   641,   643,   645,   647,   651,   655,
     659,   663,   667,   671,   675,   679,   683,   687,   691,   695,
     699,   703,   706,   711,   713,   715,   717,   720,   723,   727,
     731,   738,   742,   746,   753,   760,   767,   772,   774,   776,
     778,   780,   782,   784,   786,   787,   789,   793,   795,   799,
     800,   802,   807,   814,   819,   823,   829,   835,   840,   847,
     851,   854,   860,   866,   875,   884,   893,   896,   900,   904,
     910,   917,   923,   930,   937,   943,   950,   956,   962,   968,
     974,   980,   987,   994,  1003,  1010,  1013,  1017,  1021,  1023,
    1026,  1029,  1032,  1036,  1039,  1040,  1044,  1048,  1051,  1056,
    1059,  1061,  1066,  1068,  1069,  1073,  1074,  1077,  1080,  1084,
    1087,  1090,  1094,  1097,  1100,  1103,  1105,  1109,  1112,  1115,
    1118,  1121,  1125,  1128,  1131,  1134,  1138,  1141,  1144,  1147,
    1151,  1154,  1159,  1162,  1165,  1168,  1170,  1171,  1174,  1176,
    1178,  1181,  1185,  1186,  1189,  1191,  1193,  1196,  1200,  1201,
    1202,  1206,  1207,  1211,  1215,  1217,  1221,  1223,  1224,  1229,
    1236,  1243,  1246,  1250,  1254,  1260,  1263,  1267,  1271,  1276,
    1281,  1286,  1292,  1298,  1303,  1307,  1312,  1317,  1322,  1326,
    1328,  1330,  1332,  1334,  1336,  1338,  1340,  1342,  1343,  1346,
    1349,  1351,  1355,  1356,  1359,  1360,  1364,  1367,  1371,  1374,
    1378,  1382,  1386,  1389,  1393,  1397,  1400,  1403,  1406,  1411,
    1413,  1415,  1417,  1419,  1421,  1423,  1425,  1427,  1429,  1431,
    1433,  1435,  1437,  1439,  1441,  1443,  1445,  1447,  1449,  1451,
    1453,  1455,  1457,  1459,  1461,  1463,  1465,  1467,  1469,  1471,
    1473,  1475,  1477,  1479,  1481,  1484,  1487,  1490,  1493,  1496,
    1499,  1502,  1505,  1509,  1513,  1517,  1521,  1525,  1529,  1533,
    1535,  1537,  1539,  1541,  1543,  1545,  1547,  1549,  1551,  1553,
    1555,  1557,  1559
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     236,     0,    -1,   237,     3,    -1,     4,   395,    -1,    -1,
     237,   394,    -1,    11,    -1,   225,   339,   226,    -1,    15,
      -1,    29,    15,    18,   238,   227,    -1,    29,    15,    18,
      14,   227,    -1,   238,    -1,    30,    -1,    32,    -1,    31,
      -1,    33,    -1,    -1,    14,    -1,    15,    -1,   241,   244,
      -1,   242,    -1,   241,    -1,    26,   238,    -1,   228,   241,
      -1,    16,    -1,    34,   247,   248,   227,    -1,    16,    -1,
      35,    -1,    46,    -1,    36,    -1,   229,   249,   230,    -1,
     250,    -1,   249,   231,   250,    -1,    37,    -1,    38,    -1,
      39,    -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,
      44,    -1,    45,    -1,    47,   253,     3,    -1,    48,     3,
      -1,    -1,    16,    -1,   262,    49,    -1,    50,   240,    -1,
      51,    52,   238,    -1,    53,   240,    -1,   262,    54,    -1,
      51,    52,   238,    -1,    49,    56,   238,    -1,    57,    58,
     260,   227,    -1,    11,    -1,    13,    -1,    14,    -1,    56,
      15,    -1,    59,    -1,    -1,    15,    -1,    -1,    60,   238,
      -1,    61,   240,    -1,    37,   274,    -1,    64,   265,    -1,
      65,   265,    -1,    56,   266,    -1,    66,   317,    -1,   318,
      -1,    36,    -1,    67,    -1,    15,    -1,    37,    77,   274,
      -1,    37,    83,   238,    -1,    16,    -1,   242,    -1,    16,
     242,    -1,   262,    62,    -1,   269,   263,   229,    -1,   270,
     264,   227,    -1,   270,   273,   227,    -1,    -1,   271,   268,
      -1,    63,   272,    -1,   273,   231,   272,    -1,    36,    -1,
      31,    -1,   229,   275,   230,    -1,   276,    -1,   275,   231,
     276,    -1,    68,    -1,    69,    -1,    70,    -1,    78,    -1,
      75,    -1,    76,    -1,    -1,   278,   227,   277,    -1,   279,
     229,   280,   230,    -1,   279,    -1,    71,    16,    -1,    -1,
      85,    72,   245,   227,   280,    -1,    85,    74,   245,   227,
     280,    -1,   105,    72,   245,   227,   280,    -1,   105,    74,
     245,   227,   280,    -1,    58,   238,   227,   280,    -1,   282,
     229,   277,   230,    -1,   283,    -1,   208,    -1,    71,    16,
      -1,    86,    87,     3,    -1,    86,    77,   287,     3,    -1,
      86,    77,    36,   287,     3,    -1,    15,    -1,    -1,    86,
      78,     3,    -1,    86,    78,    88,     3,    -1,    86,    89,
     291,     3,    -1,    -1,   291,   242,    -1,   291,    82,   243,
      -1,   291,    56,    15,    -1,   291,    66,   317,    -1,   291,
     318,    -1,   291,    36,    -1,   291,    79,    -1,   291,   292,
      15,    -1,   291,    80,    -1,   291,    81,    -1,    84,    -1,
      64,    -1,    86,    90,   287,     3,    -1,    91,    92,     3,
      -1,    91,    93,     3,    -1,    91,    78,     3,    -1,    91,
      94,     3,    -1,    91,    95,     3,    -1,    91,    69,     3,
      -1,    91,    77,     3,    -1,    96,   302,   303,     3,    -1,
      36,    -1,    31,    -1,    11,    -1,    -1,    11,    -1,    97,
     308,     3,    -1,    98,   308,     3,    -1,    99,   308,     3,
      -1,    37,   308,   274,     3,    -1,    15,    -1,    36,    -1,
      16,    -1,    -1,    66,    15,   310,   316,    -1,   141,   339,
      -1,   108,    -1,   109,    -1,   110,    -1,   111,    -1,   112,
      -1,   114,    -1,   115,    -1,   116,    -1,   117,    -1,   312,
     113,    -1,   312,    15,    -1,    -1,   313,   227,   314,    -1,
     313,    -1,   315,   227,   313,    -1,   320,    -1,    15,    -1,
     316,    -1,   140,   339,    -1,   225,   315,   226,    -1,   225,
     226,    -1,   314,   229,   323,   230,    -1,    -1,   100,    15,
     322,   319,   320,    -1,    -1,   345,   323,    -1,   345,    -1,
     229,   323,   230,    -1,   225,    11,   231,    11,   226,    -1,
      14,    26,    11,    -1,   326,    -1,   326,    23,    -1,   326,
      24,    -1,   326,   229,    11,   231,    11,   230,    -1,    14,
      -1,    11,    -1,   325,    -1,   328,    -1,   327,    -1,    12,
      -1,   329,    -1,   329,    22,    22,   329,    -1,   330,    -1,
     331,   231,   330,    -1,    -1,   330,   228,   323,   332,    -1,
     120,   228,   323,    -1,    11,    -1,   232,    -1,   333,    -1,
     333,   334,    -1,    11,    -1,   122,    -1,   123,    -1,    16,
      -1,   325,    -1,   328,    -1,   326,    -1,   233,   331,   234,
      -1,    12,    -1,    26,   334,    26,    -1,    -1,    15,   225,
     344,   226,    -1,    72,    -1,   124,    -1,   125,    -1,   127,
      -1,   128,    -1,   129,    -1,   130,    -1,   225,   339,   226,
      -1,   339,    23,   339,    -1,   339,    24,   339,    -1,   339,
      25,   339,    -1,   339,    26,   339,    -1,   339,     9,   339,
      -1,   339,    10,   339,    -1,   339,    18,   339,    -1,   339,
       8,   339,    -1,   339,    19,   339,    -1,   339,     7,   339,
      -1,   339,    20,   339,    -1,   339,     6,   339,    -1,   339,
      21,   339,    -1,    28,   339,    -1,   132,   225,   339,   226,
      -1,   335,    -1,    15,    -1,    60,    -1,   336,   338,    -1,
     336,   399,    -1,   339,    22,   110,    -1,   339,    22,   131,
      -1,   339,    22,   126,   225,   339,   226,    -1,    23,   139,
      23,    -1,    24,   139,    24,    -1,   137,   225,   339,   231,
     339,   226,    -1,   133,   225,   339,   231,   339,   226,    -1,
     134,   225,   339,   231,   339,   226,    -1,    15,   225,   344,
     226,    -1,   107,    -1,   105,    -1,   106,    -1,    42,    -1,
     101,    -1,   102,    -1,   339,    -1,    -1,   341,    -1,   341,
     231,   342,    -1,   339,    -1,   339,   231,   343,    -1,    -1,
     343,    -1,   118,   339,   119,   324,    -1,   118,   339,   119,
     324,   120,   324,    -1,    15,    18,   339,   227,    -1,   104,
     339,   227,    -1,   336,   399,    18,   339,   227,    -1,   336,
     338,    18,   339,   227,    -1,    60,    18,   339,   227,    -1,
     103,   225,   336,   399,   226,   227,    -1,   340,   342,   227,
      -1,   337,   227,    -1,   121,   339,   229,   332,   230,    -1,
     336,   399,    22,   139,   227,    -1,   336,   399,    22,   137,
     225,   339,   226,   227,    -1,   336,   399,    22,   133,   225,
     339,   226,   227,    -1,   336,   399,    22,   134,   225,   339,
     226,   227,    -1,   262,   142,    -1,   346,   263,   229,    -1,
     347,   264,   227,    -1,   347,   143,   145,   238,   227,    -1,
     347,   144,   241,   145,   238,   227,    -1,   347,   146,    52,
     238,   227,    -1,   347,   151,   146,    52,   238,   227,    -1,
     347,   147,   148,    52,   238,   227,    -1,   347,   149,    52,
     238,   227,    -1,   347,   150,   238,   152,   241,   227,    -1,
     347,   153,   154,   155,   227,    -1,   347,   157,   158,   240,
     227,    -1,   347,   156,   166,   238,   227,    -1,   347,   156,
     165,   238,   227,    -1,   347,   127,   172,   241,   227,    -1,
     347,   159,   160,    52,   238,   227,    -1,   347,    42,   161,
      52,   238,   227,    -1,   347,    42,   162,    52,   238,   231,
     238,   227,    -1,   347,    97,   163,    42,   240,   227,    -1,
     262,   173,    -1,   348,   263,   229,    -1,   349,   350,   227,
      -1,   264,    -1,   178,   240,    -1,   180,   238,    -1,   352,
     230,    -1,   174,   260,   229,    -1,   351,   353,    -1,    -1,
     353,   354,   227,    -1,   179,   181,   238,    -1,   179,   240,
      -1,   196,   229,   360,   230,    -1,    63,   372,    -1,   355,
      -1,   358,   229,   356,   230,    -1,   358,    -1,    -1,   356,
     357,   227,    -1,    -1,   183,   238,    -1,   182,   238,    -1,
     184,   160,   238,    -1,   162,   238,    -1,   186,   238,    -1,
     186,    81,   238,    -1,   191,    67,    -1,   191,   190,    -1,
     191,   193,    -1,   281,    -1,   198,   199,   260,    -1,   181,
     238,    -1,   183,   238,    -1,   195,   238,    -1,   182,   238,
      -1,   184,   160,   238,    -1,   185,   238,    -1,   162,   238,
      -1,   186,   238,    -1,   186,    81,   238,    -1,   189,   210,
      -1,   189,   187,    -1,   189,   188,    -1,   192,   187,   240,
      -1,   179,   240,    -1,    94,   229,   364,   230,    -1,   191,
      67,    -1,   191,   190,    -1,   191,   193,    -1,   281,    -1,
      -1,   360,   361,    -1,   362,    -1,   363,    -1,   242,   227,
      -1,   242,   197,   227,    -1,    -1,   364,   365,    -1,   366,
      -1,   367,    -1,    14,   227,    -1,    14,   194,   227,    -1,
      -1,    -1,   369,   359,   227,    -1,    -1,   229,   369,   230,
      -1,   368,   268,   370,    -1,   371,    -1,   372,   231,   371,
      -1,    16,    -1,    -1,    86,   173,   287,     3,    -1,    86,
     173,    94,   287,   373,     3,    -1,    86,   173,    63,   287,
     373,     3,    -1,   262,   200,    -1,   377,   263,   229,    -1,
     378,   264,   227,    -1,   378,   201,    56,    15,   227,    -1,
     262,   202,    -1,   379,   263,   229,    -1,   380,   264,   227,
      -1,   380,   203,   238,   227,    -1,   380,   204,   238,   227,
      -1,   380,   205,   238,   227,    -1,   380,   206,    52,   238,
     227,    -1,   380,   207,    52,   238,   227,    -1,   380,   191,
     381,   227,    -1,   380,   281,   227,    -1,   380,   217,   219,
     227,    -1,   380,   217,   144,   227,    -1,   380,   217,   218,
     227,    -1,   380,   388,   227,    -1,   215,    -1,   216,    -1,
      67,    -1,   210,    -1,   211,    -1,   212,    -1,   213,    -1,
     214,    -1,    -1,   158,   238,    -1,   209,   382,    -1,   229,
      -1,   384,   383,   227,    -1,    -1,   384,   230,    -1,    -1,
     386,   268,   385,    -1,    63,   387,    -1,   388,   231,   387,
      -1,   262,   222,    -1,   389,   263,   229,    -1,   390,   264,
     227,    -1,   390,   392,   227,    -1,    89,   242,    -1,   391,
     152,   241,    -1,   391,   152,    16,    -1,   391,   223,    -1,
     391,   106,    -1,   391,   224,    -1,    86,   222,   287,     3,
      -1,   227,    -1,   239,    -1,   246,    -1,   259,    -1,   261,
      -1,   396,    -1,   267,    -1,   309,    -1,   311,    -1,   321,
      -1,   251,    -1,   252,    -1,   284,    -1,   285,    -1,   286,
      -1,   288,    -1,   289,    -1,   290,    -1,   293,    -1,   294,
      -1,   295,    -1,   296,    -1,   297,    -1,   298,    -1,   299,
      -1,   300,    -1,   301,    -1,   304,    -1,   305,    -1,   306,
      -1,   307,    -1,   374,    -1,   375,    -1,   376,    -1,   393,
      -1,   397,   230,    -1,   398,   230,    -1,   270,   230,    -1,
     347,   230,    -1,   349,   230,    -1,   378,   230,    -1,   380,
     230,    -1,   390,   230,    -1,   254,   263,   229,    -1,   397,
     264,   227,    -1,   397,   255,   227,    -1,   397,   258,   227,
      -1,   256,   263,   229,    -1,   398,   264,   227,    -1,   398,
     257,   227,    -1,     5,    -1,   164,    -1,   165,    -1,   166,
      -1,   167,    -1,   168,    -1,   169,    -1,   170,    -1,   171,
      -1,   175,    -1,   176,    -1,   177,    -1,   220,    -1,   221,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   242,   242,   243,   246,   248,   255,   256,   257,   261,
     265,   274,   275,   276,   277,   278,   279,   285,   286,   293,
     300,   301,   305,   309,   316,   326,   335,   340,   341,   345,
     346,   350,   351,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   368,   371,   375,   376,   384,   400,   401,   405,
     417,   429,   439,   456,   462,   463,   464,   476,   484,   488,
     494,   500,   502,   506,   507,   508,   509,   510,   514,   515,
     516,   517,   521,   529,   530,   536,   537,   538,   544,   553,
     554,   555,   559,   568,   572,   573,   579,   580,   581,   585,
     586,   590,   591,   592,   593,   594,   595,   600,   602,   606,
     607,   611,   625,   626,   627,   628,   629,   630,   634,   637,
     641,   649,   667,   670,   673,   677,   678,   681,   684,   687,
     691,   697,   703,   710,   715,   720,   725,   729,   733,   743,
     747,   754,   755,   758,   762,   764,   766,   768,   770,   772,
     774,   777,   783,   784,   785,   789,   790,   796,   798,   800,
     804,   809,   810,   811,   817,   817,   826,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   850,   865,   866,
     873,   874,   881,   890,   894,   898,   922,   923,   927,   933,
     933,   947,   948,   959,   962,   971,   978,   985,   986,   987,
     988,   992,   996,   997,   998,   999,  1000,  1004,  1014,  1023,
    1024,  1027,  1028,  1033,  1044,  1045,  1049,  1050,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1072,  1076,
    1099,  1101,  1102,  1103,  1104,  1105,  1106,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1127,  1128,  1155,  1157,  1159,  1161,  1162,
    1163,  1173,  1174,  1175,  1176,  1177,  1182,  1205,  1206,  1207,
    1208,  1209,  1210,  1214,  1217,  1218,  1219,  1228,  1235,  1244,
    1245,  1249,  1255,  1265,  1274,  1280,  1285,  1292,  1297,  1303,
    1304,  1305,  1313,  1315,  1316,  1317,  1323,  1339,  1340,  1341,
    1345,  1350,  1351,  1352,  1353,  1354,  1355,  1356,  1357,  1358,
    1359,  1360,  1361,  1362,  1363,  1372,  1382,  1383,  1387,  1388,
    1389,  1390,  1393,  1404,  1407,  1409,  1413,  1414,  1415,  1416,
    1417,  1421,  1422,  1425,  1427,  1430,  1431,  1432,  1433,  1434,
    1435,  1436,  1437,  1438,  1439,  1440,  1443,  1463,  1464,  1465,
    1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,
    1476,  1477,  1478,  1479,  1480,  1481,  1484,  1486,  1490,  1491,
    1493,  1502,  1512,  1514,  1518,  1519,  1521,  1530,  1541,  1561,
    1563,  1566,  1568,  1572,  1576,  1577,  1581,  1582,  1586,  1589,
    1592,  1598,  1605,  1606,  1607,  1617,  1624,  1625,  1626,  1627,
    1628,  1629,  1630,  1631,  1632,  1633,  1634,  1635,  1636,  1640,
    1641,  1642,  1646,  1647,  1648,  1649,  1650,  1653,  1654,  1655,
    1659,  1660,  1663,  1663,  1666,  1675,  1679,  1680,  1687,  1694,
    1695,  1696,  1699,  1708,  1712,  1718,  1719,  1720,  1723,  1730,
    1730,  1730,  1730,  1730,  1730,  1730,  1730,  1730,  1730,  1731,
    1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,
    1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,  1731,
    1731,  1731,  1731,  1731,  1732,  1732,  1732,  1732,  1732,  1732,
    1732,  1732,  1733,  1733,  1733,  1733,  1734,  1734,  1734,  1735,
    1735,  1736,  1737,  1738,  1739,  1740,  1741,  1742,  1743,  1743,
    1743,  1743,  1743
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "CLI_MARKER", "INVALID_TOKEN",
  "GEQ", "LEQ", "NEQ", "AND", "OR", "NUM", "ENUM", "RTRID", "IPA", "SYM",
  "TEXT", "PREFIX_DUMMY", "'='", "'<'", "'>'", "'~'", "'.'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "DEFINE", "ON", "OFF", "YES", "NO", "LOG",
  "SYSLOG", "ALL", "DEBUG", "TRACE", "INFO", "REMOTE", "WARNING", "ERROR",
  "AUTH", "FATAL", "BUG", "STDERR", "CONFIGURE", "DOWN", "KERNEL",
  "PERSIST", "SCAN", "TIME", "LEARN", "DEVICE", "ASYNC", "TABLE", "ROUTER",
  "ID", "PROTOCOL", "PREFERENCE", "DISABLED", "DIRECT", "INTERFACE",
  "IMPORT", "EXPORT", "FILTER", "NONE", "STATES", "ROUTES", "FILTERS",
  "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS", "PROTOCOLS",
  "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR", "COMMANDS",
  "PREIMPORT", "GENERATE", "SHOW", "STATUS", "SUMMARY", "ROUTE", "SYMBOLS",
  "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS", "ATTRIBUTES", "ECHO",
  "DISABLE", "ENABLE", "RESTART", "FUNCTION", "PRINT", "PRINTN", "UNSET",
  "RETURN", "ACCEPT", "REJECT", "QUITBIRD", "INT", "BOOL", "IP", "PREFIX",
  "PAIR", "SET", "STRING", "BGPMASK", "BGPPATH", "CLIST", "IF", "THEN",
  "ELSE", "CASE", "TRUE", "FALSE", "GW", "NET", "MASK", "SOURCE", "SCOPE",
  "CAST", "DEST", "LEN", "DEFINED", "ADD", "DELETE", "CONTAINS", "RESET",
  "PREPEND", "MATCH", "EMPTY", "WHERE", "EVAL", "BGP", "LOCAL", "NEIGHBOR",
  "AS", "HOLD", "CONNECT", "RETRY", "KEEPALIVE", "MULTIHOP", "STARTUP",
  "VIA", "NEXT", "HOP", "SELF", "DEFAULT", "PATH", "METRIC", "START",
  "DELAY", "FORGET", "WAIT", "AFTER", "BGP_PATH", "BGP_LOCAL_PREF",
  "BGP_MED", "BGP_ORIGIN", "BGP_NEXT_HOP", "BGP_ATOMIC_AGGR",
  "BGP_AGGREGATOR", "BGP_COMMUNITY", "ADDRESS", "OSPF", "AREA",
  "OSPF_METRIC1", "OSPF_METRIC2", "OSPF_TAG", "RFC1583COMPAT", "STUB",
  "TICK", "COST", "RETRANSMIT", "HELLO", "TRANSMIT", "PRIORITY", "DEAD",
  "NONBROADCAST", "POINTOPOINT", "TYPE", "SIMPLE", "AUTHENTICATION",
  "STRICT", "CRYPTOGRAPHIC", "ELIGIBLE", "POLL", "NETWORKS", "HIDDEN",
  "VIRTUAL", "LINK", "PIPE", "PEER", "RIP", "INFINITY", "PORT", "PERIOD",
  "GARBAGE", "TIMEOUT", "PASSWORDS", "MODE", "BROADCAST", "MULTICAST",
  "QUIET", "NOLISTEN", "VERSION1", "PLAINTEXT", "MD5", "HONOR", "NEVER",
  "ALWAYS", "RIP_METRIC", "RIP_TAG", "STATIC", "DROP", "PROHIBIT", "'('",
  "')'", "';'", "':'", "'{'", "'}'", "','", "'?'", "'['", "']'", "$accept",
  "config", "conf_entries", "expr", "definition", "bool", "ipa", "prefix",
  "prefix_or_ipa", "pxlen", "datetime", "log_config", "log_file",
  "log_mask", "log_mask_list", "log_cat", "cmd_CONFIGURE", "cmd_DOWN",
  "cfg_name", "kern_proto_start", "kern_item", "kif_proto_start",
  "kif_item", "nl_item", "rtrid", "idval", "newtab", "proto_start",
  "proto_name", "proto_item", "imexport", "rtable", "debug_default",
  "iface_patt", "dev_proto_start", "dev_proto", "dev_iface_entry_init",
  "dev_iface_entry", "dev_iface_list", "debug_mask", "debug_list",
  "debug_flag", "password_items", "password_item", "password_item_begin",
  "password_item_params", "password_list", "password_begin_list",
  "password_begin", "cmd_SHOW_STATUS", "cmd_SHOW_PROTOCOLS",
  "cmd_SHOW_PROTOCOLS_ALL", "optsym", "cmd_SHOW_INTERFACES",
  "cmd_SHOW_INTERFACES_SUMMARY", "cmd_SHOW_ROUTE", "r_args",
  "import_or_proto", "cmd_SHOW_SYMBOLS", "cmd_DUMP_RESOURCES",
  "cmd_DUMP_SOCKETS", "cmd_DUMP_INTERFACES", "cmd_DUMP_NEIGHBORS",
  "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES", "cmd_DUMP_PROTOCOLS",
  "cmd_ECHO", "echo_mask", "echo_size", "cmd_DISABLE", "cmd_ENABLE",
  "cmd_RESTART", "cmd_DEBUG", "proto_patt", "filter_def", "@1",
  "filter_eval", "type", "one_decl", "decls", "declsn", "filter_body",
  "filter", "where_filter", "function_params", "function_body",
  "function_def", "@2", "cmds", "block", "pair", "fprefix_s", "fprefix",
  "fipa", "set_atom", "set_item", "set_items", "switch_body", "bgp_one",
  "bgp_path", "constant", "rtadot", "function_call", "static_attr", "term",
  "break_command", "print_one", "print_list", "var_listn", "var_list",
  "cmd", "bgp_proto_start", "bgp_proto", "ospf_proto_start", "ospf_proto",
  "ospf_proto_item", "ospf_area_start", "ospf_area", "ospf_area_opts",
  "ospf_area_item", "ospf_vlink", "ospf_vlink_opts", "ospf_vlink_item",
  "ospf_vlink_start", "ospf_iface_item", "pref_list", "pref_item",
  "pref_el", "pref_hid", "ipa_list", "ipa_item", "ipa_el", "ipa_ne",
  "ospf_iface_start", "ospf_iface_opts", "ospf_iface_opt_list",
  "ospf_iface", "ospf_iface_list", "opttext", "cmd_SHOW_OSPF",
  "cmd_SHOW_OSPF_NEIGHBORS", "cmd_SHOW_OSPF_INTERFACE", "pipe_proto_start",
  "pipe_proto", "rip_cfg_start", "rip_cfg", "rip_auth", "rip_mode",
  "rip_iface_item", "rip_iface_opts", "rip_iface_opt_list",
  "rip_iface_init", "rip_iface", "rip_iface_list", "static_proto_start",
  "static_proto", "stat_route0", "stat_route", "cmd_SHOW_STATIC", "conf",
  "cli_cmd", "proto", "kern_proto", "kif_proto", "dynamic_attr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,    61,    60,
      62,   126,    46,    43,    45,    42,    47,    37,    33,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
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
     464,   465,   466,   467,   468,    40,    41,    59,    58,   123,
     125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short int yyr1[] =
{
       0,   235,   236,   236,   237,   237,   238,   238,   238,   239,
     239,   240,   240,   240,   240,   240,   240,   241,   241,   242,
     243,   243,   244,   244,   245,   246,   247,   247,   247,   248,
     248,   249,   249,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   251,   252,   253,   253,   254,   255,   255,   255,
     256,   257,   258,   259,   260,   260,   260,   261,   262,   263,
     263,   264,   264,   264,   264,   264,   264,   264,   265,   265,
     265,   265,   266,   267,   267,   268,   268,   268,   269,   270,
     270,   270,   271,   272,   273,   273,   274,   274,   274,   275,
     275,   276,   276,   276,   276,   276,   276,   277,   277,   278,
     278,   279,   280,   280,   280,   280,   280,   280,   281,   281,
     282,   283,   284,   285,   286,   287,   287,   288,   289,   290,
     291,   291,   291,   291,   291,   291,   291,   291,   291,   291,
     291,   292,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   302,   302,   303,   303,   304,   305,   306,
     307,   308,   308,   308,   310,   309,   311,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   313,   314,   314,
     315,   315,   316,   317,   317,   318,   319,   319,   320,   322,
     321,   323,   323,   324,   324,   325,   326,   327,   327,   327,
     327,   328,   329,   329,   329,   329,   329,   330,   330,   331,
     331,   332,   332,   332,   333,   333,   334,   334,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   336,   337,
     338,   338,   338,   338,   338,   338,   338,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   340,   340,   340,
     340,   340,   340,   341,   342,   342,   342,   343,   343,   344,
     344,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   346,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   348,   349,   349,   350,   350,
     350,   350,   351,   352,   353,   353,   354,   354,   354,   354,
     354,   355,   355,   356,   356,   357,   357,   357,   357,   357,
     357,   357,   357,   357,   357,   357,   358,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   360,   360,   361,   361,
     362,   363,   364,   364,   365,   365,   366,   367,   368,   369,
     369,   370,   370,   371,   372,   372,   373,   373,   374,   375,
     376,   377,   378,   378,   378,   379,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   381,
     381,   381,   382,   382,   382,   382,   382,   383,   383,   383,
     384,   384,   385,   385,   386,   387,   388,   388,   389,   390,
     390,   390,   391,   392,   392,   392,   392,   392,   393,   394,
     394,   394,   394,   394,   394,   394,   394,   394,   394,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   396,   396,   396,   396,   396,   396,
     396,   396,   397,   397,   397,   397,   398,   398,   398,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     3,     1,     5,
       5,     1,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     2,     2,     1,     4,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     0,     1,     2,     2,     3,     2,
       2,     3,     3,     4,     1,     1,     1,     2,     1,     0,
       1,     0,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     3,     3,     1,     1,     2,     2,     3,
       3,     3,     0,     2,     2,     3,     1,     1,     3,     1,
       3,     1,     1,     1,     1,     1,     1,     0,     3,     4,
       1,     2,     0,     5,     5,     5,     5,     4,     4,     1,
       1,     2,     3,     4,     5,     1,     0,     3,     4,     4,
       0,     2,     3,     3,     3,     2,     2,     2,     3,     2,
       2,     1,     1,     4,     3,     3,     3,     3,     3,     3,
       3,     4,     1,     1,     1,     0,     1,     3,     3,     3,
       4,     1,     1,     1,     0,     4,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     0,     3,
       1,     3,     1,     1,     1,     2,     3,     2,     4,     0,
       5,     0,     2,     1,     3,     5,     3,     1,     2,     2,
       6,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       3,     0,     4,     3,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     4,     1,     1,     1,     2,     2,     3,     3,
       6,     3,     3,     6,     6,     6,     4,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     3,     1,     3,     0,
       1,     4,     6,     4,     3,     5,     5,     4,     6,     3,
       2,     5,     5,     8,     8,     8,     2,     3,     3,     5,
       6,     5,     6,     6,     5,     6,     5,     5,     5,     5,
       5,     6,     6,     8,     6,     2,     3,     3,     1,     2,
       2,     2,     3,     2,     0,     3,     3,     2,     4,     2,
       1,     4,     1,     0,     3,     0,     2,     2,     3,     2,
       2,     3,     2,     2,     2,     1,     3,     2,     2,     2,
       2,     3,     2,     2,     2,     3,     2,     2,     2,     3,
       2,     4,     2,     2,     2,     1,     0,     2,     1,     1,
       2,     3,     0,     2,     1,     1,     2,     3,     0,     0,
       3,     0,     3,     3,     1,     3,     1,     0,     4,     6,
       6,     2,     3,     3,     5,     2,     3,     3,     4,     4,
       4,     5,     5,     4,     3,     4,     4,     4,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     2,     2,
       1,     3,     0,     2,     0,     3,     2,     3,     2,     3,
       3,     3,     2,     3,     3,     2,     2,     2,     4,     1,
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
static const unsigned short int yydefact[] =
{
       4,     0,     0,     0,     0,    44,     0,     0,     0,     0,
       0,     0,     0,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,     3,     1,
       2,     0,     0,     0,     0,     0,    58,     0,     0,   218,
     429,   430,   431,    59,    59,   432,   433,     0,   435,    59,
      61,   436,   437,   438,    59,    61,    59,    61,    59,    61,
      59,    61,    59,    61,     5,   434,    61,    61,   151,   153,
     152,     0,    45,     0,    43,   116,     0,     0,   120,   116,
     116,   116,     0,     0,     0,     0,     0,     0,     0,   144,
     143,   142,   145,     0,     0,     0,     0,    26,    27,    28,
       0,     0,     0,    57,     0,   154,   179,   208,   216,   191,
     244,   211,     0,     0,     0,   218,   245,   209,   210,     0,
       0,     0,     0,   218,     0,   212,   214,   213,   243,     0,
     156,    60,     0,     0,    46,    50,    78,   286,   305,   381,
     385,   418,     0,     0,     0,     0,    16,    82,     0,     0,
     466,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   467,     0,
       0,     0,    16,     0,   468,   308,     0,   314,     0,     0,
       0,   469,     0,     0,   414,     0,     0,     0,     0,     0,
       0,     0,   110,     0,   470,     0,     0,     0,   109,     0,
       0,     0,   471,     0,     0,     0,     0,    16,     0,    16,
     464,     0,     0,     0,     0,   465,     0,     0,    87,    86,
       0,     0,    42,   115,   116,     0,   117,     0,   112,     0,
       0,   116,   116,     0,     0,   139,   140,   136,   134,   135,
     137,   138,   146,     0,   147,   148,   149,     0,    29,     0,
       0,    73,     6,     8,   218,    74,    54,    55,    56,     0,
     168,     0,     0,   218,     0,     0,   204,   205,   206,     0,
     241,   218,   218,   218,   218,   208,     0,   192,   196,     0,
     193,   187,   195,   194,   197,   199,     0,   479,   220,   221,
     222,   223,   224,   225,   226,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   246,   247,
     218,   218,   218,   218,   218,   218,   218,   218,   218,     0,
     218,   218,   218,   218,   472,   476,    79,    64,    72,    67,
      62,    12,    14,    13,    15,    11,    63,     0,    84,    70,
     168,    71,   218,    65,    69,    66,    80,    81,    82,   287,
       0,     0,     0,     0,     0,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,   288,   306,
       0,   309,   310,   307,   313,   311,   382,     0,   383,   386,
       0,   416,   111,   401,   399,   400,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,   394,    97,   398,   414,
     419,     0,   422,   420,   426,     0,   425,   427,   421,     0,
      47,     0,    49,   474,   475,   473,     0,   478,   477,    91,
      92,    93,    95,    96,    94,     0,    89,   150,     0,   113,
     118,   119,   126,     0,   132,   168,   127,   129,   130,     0,
     131,   121,     0,   125,   133,   377,   377,   378,   428,   141,
       0,     0,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     0,    31,    25,     0,    53,   157,   158,   159,   160,
     161,   162,   163,   164,   165,     0,     0,     0,   155,   172,
       0,   168,   186,   267,   270,     0,   251,   252,   207,   217,
       0,     0,     0,     0,     0,   227,     0,   188,   189,     0,
       0,     0,   215,   239,   237,   235,   232,   233,   234,   236,
     238,   240,   248,     0,   249,   228,   229,   230,   231,    75,
      76,    83,   173,   174,    68,   175,    85,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   312,   368,    16,     0,     0,     0,   320,
     322,     0,   412,   393,   388,   389,   390,     0,     0,   396,
     397,   395,     0,     0,     0,   100,   417,     0,     0,    19,
     424,   423,    52,    48,    51,    88,     0,   114,   123,   124,
      21,    20,   122,   128,   376,     0,     0,    10,     9,    30,
       0,     7,   167,   166,   168,   218,   177,   170,     0,   180,
     218,   256,   242,   218,   218,   218,     0,     0,     0,   200,
     218,    77,     0,     0,     0,   300,   289,     0,   291,     0,
     294,     0,     0,   296,   299,   298,   297,     0,     0,   374,
     319,     0,   317,   356,     0,   315,   323,   384,   410,   407,
     415,   391,   392,   101,   108,    97,   102,    22,    23,    90,
     380,   379,    32,   169,     0,   260,     0,   261,   262,     0,
     218,   258,   259,   257,   218,   218,     0,     0,     0,   218,
     218,   176,     0,   268,     0,     0,     0,   185,     0,   198,
       0,   302,     0,   304,   290,   293,   295,   292,   301,   371,
     368,   316,     0,   336,   325,     0,     0,   413,     0,    98,
       0,     0,     0,     0,   218,   218,   218,   218,     0,     0,
       0,   178,     0,     0,   280,   263,   265,     0,   182,   171,
     254,   255,   253,     0,   250,     0,   369,   373,   375,   318,
       0,   357,   358,   359,     0,     0,     0,     0,     0,     0,
     321,   335,     0,   408,   402,   403,   404,   405,   406,   409,
     411,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   274,   218,   201,   218,   218,     0,   218,   279,   190,
     303,     0,     0,   360,   329,   327,   326,     0,     0,   330,
     332,   333,   334,   324,   102,    24,     0,     0,     0,     0,
     273,   219,   277,     0,   218,   271,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   266,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     372,   355,     0,   361,   328,   331,   107,   102,   102,   102,
     102,     0,     0,   218,   218,   218,   281,   276,   275,   218,
     218,   218,   282,   362,   343,   350,   337,   340,   338,     0,
     342,     0,   344,   347,   348,   346,   352,   353,   354,    16,
     339,   370,   103,   104,   105,   106,   278,   184,   272,   203,
     201,     0,     0,     0,     0,   341,   345,   349,   202,     0,
       0,     0,     0,   351,   363,   364,   365,   284,   285,   283,
       0,   366,   367
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     2,     3,   345,    51,   346,   411,   530,   592,   579,
     796,    52,   110,   260,   471,   472,    13,    14,    83,    53,
     221,    54,   226,   222,    55,   269,    56,    57,   142,   161,
     353,   339,    58,   531,    59,    60,   347,   348,   162,   231,
     435,   436,   573,   574,   575,   713,   206,   207,   208,    15,
      16,    17,   235,    18,    19,    20,   239,   452,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   102,   253,    30,
      31,    32,    33,    81,    61,   270,    62,   485,   486,   487,
     608,   533,   534,   354,   491,   489,    63,   271,   676,   805,
     135,   136,   292,   137,   294,   808,   296,   809,   278,   279,
     138,   139,   678,   318,   493,   679,   726,   727,   494,   495,
     680,    64,    65,    66,    67,   186,   187,   188,   384,   558,
     559,   704,   752,   560,   832,   702,   741,   742,   743,   884,
     894,   895,   896,   638,   781,   737,   639,   640,   595,    34,
      35,    36,    68,    69,    70,    71,   396,   759,   708,   649,
     650,   390,   391,   209,    72,    73,   214,   215,    37,    74,
      38,    75,    76,    77,   319
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -661
static const short int yypact[] =
{
      41,  1254,    83,   154,   192,    89,   223,   551,  1131,   257,
     192,   192,   192,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,   104,   170,   178,   222,   193,  -661,   224,   241,   385,
    -661,  -661,  -661,   265,   265,  -661,  -661,   734,  -661,   265,
     115,  -661,  -661,  -661,   265,  1130,   265,  1070,   265,   302,
     265,   998,   265,  1015,  -661,  -661,   497,   413,  -661,  -661,
    -661,    72,  -661,   259,  -661,   285,    40,   284,  -661,   282,
     130,   282,   301,   306,   348,   358,   361,   371,   373,  -661,
    -661,  -661,    45,   381,   395,   403,   335,  -661,  -661,  -661,
     -17,    72,    13,  -661,   313,  -661,  -661,  -661,  -661,   378,
     194,  -661,   276,   278,    -2,   385,  -661,  -661,  -661,   197,
     200,   205,   210,   468,    22,  -661,  -661,  -661,  -661,   993,
    1349,  -661,   211,   232,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,   233,    72,   450,    13,    27,  -661,   151,   151,
    -661,   240,   -57,   246,    42,   318,   304,   340,   298,   434,
     345,   445,    13,   363,   356,   190,   365,   354,  -661,   294,
     297,   313,    27,    13,  -661,  -661,   311,  -661,   314,   312,
     493,  -661,   333,   338,  -661,   508,   -26,    13,    13,    13,
     488,   516,  -661,   -37,  -661,   342,   343,   349,  -661,    38,
     353,   298,  -661,   352,    62,   359,   521,    27,   536,    27,
    -661,   366,   368,   369,   537,  -661,   370,   376,  -661,  -661,
     467,   595,  -661,  -661,   282,   604,  -661,   605,  -661,  1282,
     608,   282,   282,   610,   611,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,   612,  -661,  -661,  -661,    15,  -661,   883,
     389,  -661,  -661,  -661,   385,  -661,  -661,  -661,  -661,   390,
    1028,   394,   609,   326,   599,   600,  -661,  -661,    -2,   597,
    -661,   385,   385,   385,   385,   396,    44,  -661,  -661,   621,
    -661,    -7,  -661,  -661,   614,  -661,    65,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   247,
     385,   385,   385,   385,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,   428,  -661,  -661,
    1067,  -661,   385,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
     594,   603,   615,   298,    13,  -661,  -661,   511,    13,   622,
      13,   509,   623,   507,    13,    13,    27,   626,  -661,  -661,
     435,  -661,  -661,  -661,   181,  -661,  -661,   648,  -661,  -661,
     428,  -661,  -661,  -661,  -661,  -661,   439,   446,   452,   463,
      13,    13,   464,   465,   469,  -661,  -661,   624,  -661,  -661,
    -661,    -5,  -661,  -661,  -661,   484,  -661,  -661,  -661,    13,
    -661,    13,  -661,  -661,  -661,  -661,    13,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,   116,  -661,  -661,   691,  -661,
    -661,  -661,  -661,   682,  -661,  1067,  -661,  -661,  -661,   298,
    -661,  -661,   683,  -661,  -661,   684,   684,  -661,  -661,  -661,
     472,   475,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,   157,  -661,  -661,   806,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,    56,   478,   477,  -661,  -661,
     929,  1028,  -661,    71,  -661,   482,  -661,  -661,  -661,  -661,
     827,    92,   117,   141,   701,  -661,   396,  -661,  -661,   704,
     694,    22,  -661,  1370,  1370,  1370,  1370,  1370,  1370,  1370,
    1370,  1370,  -661,   492,  -661,   412,   412,  -661,  -661,   298,
    -661,  -661,  -661,  -661,  -661,  1349,  -661,    13,    13,    27,
     502,   503,    13,   513,    13,   519,   298,    13,   520,   540,
     541,   542,    13,  -661,  -661,    16,   543,   571,   557,  -661,
     556,   559,   558,  -661,  -661,  -661,  -661,   562,   563,  -661,
    -661,  -661,   755,   567,   572,   569,  -661,    13,   298,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,   467,  -661,  -661,  -661,
      -5,  -661,  -661,  -661,  -661,   814,   815,  -661,  -661,  -661,
     883,  -661,  -661,  -661,  1028,   350,  -661,  -661,   220,  -661,
     385,  -661,  -661,   385,   385,   385,   593,   589,    22,  -661,
     385,  -661,   596,   590,   613,  -661,  -661,   616,  -661,   617,
    -661,   632,   633,  -661,  -661,  -661,  -661,   634,   428,  -661,
     591,    13,  -661,  -661,   313,  -661,  -661,  -661,  -661,    36,
    -661,  -661,  -661,  -661,  -661,   624,   209,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,     0,  -661,   820,  -661,  -661,   637,
     385,  -661,  -661,  -661,   385,   385,   635,   993,   636,   379,
     982,  -661,  1028,  -661,   848,   872,   893,  -661,   828,  -661,
     923,  -661,    13,  -661,  -661,  -661,  -661,  -661,  -661,   646,
    -661,  -661,    -1,  -661,   401,    13,   362,  -661,   650,  -661,
      13,    84,   147,   653,   385,   326,   385,  -661,   662,  1305,
     310,  -661,   823,   289,  -661,  1349,   654,   657,  -661,  -661,
    -661,  -661,  -661,   656,  -661,   660,  -661,  -661,  -661,  -661,
    -123,  -661,  -661,  -661,    13,    13,    13,   728,   118,     8,
    -661,  -661,   677,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,   678,   826,   826,   826,   826,  -661,   713,   638,   735,
     483,  -661,  1007,    11,   385,   385,   295,   379,  -661,  -661,
    -661,  1080,   679,  -661,  -661,  -661,  -661,    13,    13,  -661,
    -661,  -661,  -661,  -661,   209,  -661,   681,   708,   710,   711,
    -661,  -661,  -661,   724,   350,   789,  -661,   729,   730,   680,
     756,   785,   714,   736,   752,   732,  -661,   731,    13,    27,
      13,    13,    13,   818,    13,   119,   -15,    53,   792,    13,
    -661,  -661,   753,  -661,  -661,  -661,  -661,   209,   209,   209,
     209,   754,   765,  1007,   350,   982,  -661,  -661,  -661,   385,
     385,   385,  -661,  -661,  -661,  -661,  -661,  -661,  -661,    13,
    -661,    13,  -661,  -661,  -661,  -661,  -661,  -661,  -661,    27,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
      11,   945,   966,   995,     1,  -661,  -661,  -661,  -661,   772,
     773,   779,  -121,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
     780,  -661,  -661
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -661,  -661,  -661,  -111,  -661,  -180,  -157,  -207,  -661,  -661,
    -259,  -661,  -661,  -661,  -661,   382,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -174,  -661,  -661,   461,   347,
     849,  -661,  -661,  -378,  -661,  -661,  -661,   651,  -661,   -71,
    -661,   424,   372,  -661,  -661,  -469,  -649,  -661,  -661,  -661,
    -661,  -661,    64,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,   544,  -661,  -661,  -661,  -661,  -455,   407,
    -661,   758,   578,   786,  -661,   535,  -661,  -661,  -660,   186,
    -131,  -129,  -661,  -128,   415,  -126,  -661,   150,  -661,   769,
    -661,  -595,  -661,   357,   -49,  -661,  -661,   254,   426,   336,
    -651,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,   355,  -661,   592,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,   641,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -568
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -270
static const short int yytable[] =
{
     140,   265,   381,   290,   412,   291,   293,   380,   295,   276,
     677,   367,   562,   365,   366,   892,   507,   508,   714,   258,
     728,   577,   287,   288,   262,   119,   262,   262,   263,   460,
     263,   263,   451,   287,   288,   607,   119,   420,   262,   422,
     261,   393,   263,   236,   340,     1,   341,   342,   343,   344,
     320,   321,   322,   323,   324,   751,   252,   341,   342,   343,
     344,   371,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   602,   382,   900,   782,   790,   280,   320,   321,   322,
     323,   324,   337,    39,   286,   677,   397,   398,   399,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   320,   321,
     322,   323,   324,   228,   783,    82,   901,   402,   229,   723,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   106,
     866,   806,   770,   320,   321,   322,   323,   324,   237,   262,
     262,   807,   831,   263,   263,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   842,   233,   461,   320,   321,   322,
     323,   324,   153,   240,   243,   244,   762,    40,   763,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   414,   603,
     357,   154,   863,   864,   358,   155,   156,   677,   157,   158,
     159,   403,   404,    41,   879,   880,   107,   349,    42,   394,
     395,    43,   806,   241,   705,   865,   551,   641,   791,   788,
     861,   792,   803,   360,   361,   108,   540,    78,    79,   677,
      44,    45,   259,    46,   415,   474,   109,   350,   351,   764,
      47,   765,   509,   578,   242,   715,    84,   729,    80,   739,
     277,   893,   500,   501,   502,   503,   289,   113,   264,   115,
     264,   264,   591,   867,   554,   706,   868,   289,   677,   677,
     677,   114,   264,   541,    48,   111,   116,   543,   581,   545,
     699,   112,   232,   549,   550,   408,   707,   710,    99,   409,
     505,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     141,   525,   526,   527,   528,   416,   417,   238,   100,   567,
     568,   352,   590,   101,   711,    49,   511,   233,   438,   512,
     233,   230,   610,   535,   245,   455,   456,   775,   582,   246,
     583,   776,   365,   366,   712,   584,   320,   321,   322,   323,
     324,   234,   621,   613,   266,   836,   267,   268,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   117,   118,   153,
     119,   120,   121,   264,   264,   160,   585,   586,   614,   122,
     123,   247,   124,   257,   125,   374,   375,   522,   154,   624,
     555,   248,   155,   156,   249,   664,   158,   159,   872,   873,
     874,   875,   615,   523,   250,   642,   251,   556,   524,   557,
     290,    50,   291,   293,   254,   619,   126,   599,   600,   631,
     117,   118,   665,   119,   120,   121,   117,   118,   255,   119,
     120,   121,   122,   123,   272,   124,   256,   125,   122,   123,
     666,   124,   179,   125,   185,   274,   192,   275,   205,   273,
     213,   658,   281,   223,   227,   282,   622,   623,   812,   813,
     283,   627,   814,   629,   815,   284,   632,   332,   333,   126,
     334,   637,   365,   366,   529,   126,   681,   682,   127,   128,
     153,   667,   668,   669,   670,   671,   672,   673,   129,   130,
     131,   335,   336,   132,   224,   338,   657,   356,   674,   154,
     703,   675,   195,   155,   156,   359,   363,   158,   159,   285,
     118,   362,   119,   120,   121,   364,   368,   290,   297,   291,
     293,   122,   123,   369,   124,   740,   125,   370,   365,   366,
     580,   127,   128,   190,   797,   798,   799,   127,   128,   372,
     373,   129,   130,   131,   377,   143,   132,   129,   130,   131,
     152,   378,   132,   376,   392,   163,   379,   180,   126,   189,
     701,   193,   191,   210,   153,   429,   430,   431,   383,   773,
     400,   386,   432,   433,   385,   434,   216,   217,   218,   387,
     219,   133,  -269,   154,   103,   104,   105,   155,   156,   134,
     388,   158,   159,   744,   684,   685,   686,   389,   401,   405,
     406,   690,   754,   755,   756,   757,   758,   419,   407,   413,
    -181,   735,   410,   745,   746,   747,   418,   748,   421,   426,
     127,   128,   749,   423,   753,   424,   425,   427,   437,   761,
     129,   130,   131,   428,   133,   132,  -264,   439,   440,   202,
     133,   454,   134,   457,   458,   459,   473,   475,   134,   490,
     492,   718,   496,   499,   497,   719,   720,   504,    85,    86,
     725,   750,   506,   784,   785,   786,   510,   789,    87,   855,
      88,    89,   290,   225,   291,   293,   537,   305,   306,   307,
     308,   309,   310,   311,   312,   538,   542,   539,   313,   314,
     315,   546,   548,   561,   553,   767,   563,   769,   320,   321,
     322,   323,   324,   564,   544,   547,   834,   835,   552,   565,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   887,
     566,   569,   570,   133,   587,   572,   571,   588,   593,   597,
     594,   134,   598,   316,   317,   604,   605,   854,   611,   856,
     857,   858,   616,   860,   862,   617,   618,   620,   870,   320,
     321,   322,   323,   324,    90,   810,   811,   220,   725,   625,
     626,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     628,   320,   321,   322,   323,   324,   630,   633,   885,   290,
     886,   291,   293,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   320,   321,   322,   323,   324,   634,   635,   636,
     644,   653,   643,    91,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   144,   645,   646,   647,   648,   145,   651,
     652,   320,   321,   322,   323,   324,   146,   654,   656,   655,
     881,   882,   883,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   320,   321,   322,   323,   324,   660,   661,   687,
     688,   692,   700,   691,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   320,   321,   322,   323,   324,   716,   733,
     693,   774,   795,   694,   695,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   320,   321,   322,   323,   324,   696,
     697,   698,   717,   724,   801,   721,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   736,   147,   760,   320,   321,
     322,   323,   324,   766,   778,   777,   779,   780,   787,   771,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   320,
     321,   322,   323,   324,   793,   794,   833,   148,   837,   843,
     846,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   320,
     321,   322,   323,   324,   149,   838,   150,   839,   840,   849,
     800,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     841,   320,   321,   322,   323,   324,   151,   844,   845,   852,
     853,   850,   802,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   320,   321,   322,   323,   324,   851,   859,   869,
     871,   876,   662,   847,   325,   326,   327,   328,   329,   330,
     331,   332,   333,  -181,  -181,   877,  -181,   664,   297,   897,
     898,   320,   321,   322,   323,   324,   899,   902,   355,   536,
     659,   663,   848,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   664,   589,   665,   453,   609,   709,   488,   878,
     888,   816,   601,   689,   722,   153,   683,   476,   477,   478,
     479,   480,   666,   481,   482,   483,   484,   498,   596,   665,
     576,   768,   153,   612,   154,   738,     0,     0,   155,   156,
       0,   194,   158,   159,     0,   298,     0,   666,     0,   195,
       0,   154,     0,     0,   730,   155,   156,     0,     0,   158,
     159,     0,   532,   667,   668,   669,   670,   671,   672,   673,
       0,     0,     0,     0,     0,     0,     0,     0,   731,     0,
     674,     0,  -181,   675,   211,     0,     0,   153,   667,   668,
     669,   670,   671,   672,   673,     0,     0,   299,   300,   732,
     301,   302,   303,   304,     0,   674,   154,     0,   675,     0,
     155,   156,     0,     0,   158,   159,   476,   477,   478,   479,
     480,     0,   481,   482,   483,   484,     0,     0,     0,   734,
       0,   195,     0,     0,     0,   606,     0,   305,   306,   307,
     308,   309,   310,   311,   312,     0,     0,   153,   313,   314,
     315,   889,   164,     0,   817,   476,   477,   478,   479,   480,
       0,   481,   482,   483,   484,     0,   154,     0,     0,   196,
     155,   156,   890,     0,   158,   159,     0,     0,     0,     0,
      92,   197,   198,   199,   200,   201,   202,  -181,    93,    94,
       0,     0,  -181,   316,   317,   203,     0,     0,     0,     0,
       0,   891,     0,    95,    96,    97,    98,   165,   204,     0,
       0,     0,     0,     0,     0,     0,   804,     0,     0,     0,
       0,     0,   818,     0,   181,   212,     0,     0,   182,     0,
     183,     0,     0,     0,     0,     0,     0,   166,     0,   819,
       0,   820,   821,   822,   823,   824,   825,     0,     0,   826,
       0,   827,   828,   167,   168,   829,   169,   170,     0,   171,
     172,   173,     0,   174,     0,   441,   175,   176,   202,   177,
       0,     4,     0,     0,     0,     0,   365,   366,     0,     0,
     184,     5,     6,     0,     0,     0,     0,     0,     0,     0,
     830,   320,   321,   322,   323,   324,     0,     0,   442,     0,
       0,     0,     0,   325,   326,   327,   328,   329,   330,   331,
     332,   333,     0,     0,     0,     0,     0,     0,   443,     0,
       7,     0,     0,     0,     0,     8,   444,     0,   445,     0,
       9,    10,    11,    12,     0,   320,   321,   322,   323,   324,
     178,   446,   447,   448,   449,     0,   450,   325,   326,   327,
     328,   329,   330,   331,   332,   333,  -270,  -270,  -270,  -270,
    -270,     0,     0,     0,     0,     0,     0,     0,  -270,  -270,
    -270,  -270,  -270,   330,   331,   332,   333,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   352,     0,   772
};

static const short int yycheck[] =
{
      49,   112,   182,   134,   211,   134,   134,   181,   134,    11,
     605,   168,   390,    14,    15,    14,    23,    24,    18,    36,
     680,    26,    11,    12,    11,    14,    11,    11,    15,    14,
      15,    15,   239,    11,    12,   490,    14,   217,    11,   219,
     111,    67,    15,     3,   155,     4,    30,    31,    32,    33,
       6,     7,     8,     9,    10,   704,    11,    30,    31,    32,
      33,   172,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    15,   183,   194,   197,    67,   125,     6,     7,     8,
       9,    10,   153,     0,   133,   680,   197,   198,   199,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     6,     7,
       8,     9,    10,    31,   227,    16,   227,   144,    36,   677,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    15,
      67,   772,   717,     6,     7,     8,     9,    10,    88,    11,
      11,   120,   781,    15,    15,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   804,    15,   257,     6,     7,     8,
       9,    10,    37,    89,    90,    91,    72,     3,    74,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   106,   113,
     227,    56,   187,   188,   231,    60,    61,   772,    63,    64,
      65,   218,   219,    29,   844,   845,    16,    36,    34,   215,
     216,    37,   843,    63,   158,   210,   376,   181,   190,    81,
      81,   193,   770,   161,   162,    35,   363,    15,    16,   804,
      56,    57,   229,    59,   152,   264,    46,    66,    67,    72,
      66,    74,   229,   228,    94,   225,     3,   682,    36,   230,
     232,   230,   281,   282,   283,   284,   225,    15,   225,    15,
     225,   225,   449,   190,    63,   209,   193,   225,   843,   844,
     845,    58,   225,   364,   100,    77,    15,   368,   415,   370,
     638,    83,     3,   374,   375,   227,   230,    58,    11,   231,
     226,   320,   321,   322,   323,   324,   325,   326,   327,   328,
      15,   330,   331,   332,   333,   223,   224,     3,    31,   400,
     401,   140,   449,    36,    85,   141,   231,    15,   234,   234,
      15,   229,   231,   352,     3,   241,   242,    18,   419,     3,
     421,    22,    14,    15,   105,   426,     6,     7,     8,     9,
      10,    36,   529,   231,    11,   794,    13,    14,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    11,    12,    37,
      14,    15,    16,   225,   225,   230,   230,   231,   231,    23,
      24,     3,    26,    18,    28,   165,   166,   110,    56,   539,
     179,     3,    60,    61,     3,    15,    64,    65,   837,   838,
     839,   840,   231,   126,     3,   555,     3,   196,   131,   198,
     511,   227,   511,   511,     3,   511,    60,   230,   231,   546,
      11,    12,    42,    14,    15,    16,    11,    12,     3,    14,
      15,    16,    23,    24,    26,    26,     3,    28,    23,    24,
      60,    26,    65,    28,    67,   139,    69,   139,    71,   225,
      73,   578,   225,    76,    77,   225,   537,   538,   133,   134,
     225,   542,   137,   544,   139,   225,   547,    25,    26,    60,
     229,   552,    14,    15,    16,    60,   226,   227,   122,   123,
      37,   101,   102,   103,   104,   105,   106,   107,   132,   133,
     134,   229,   229,   137,    51,    15,   577,   227,   118,    56,
     644,   121,    71,    60,    61,   229,   172,    64,    65,    11,
      12,   163,    14,    15,    16,   145,    52,   618,     5,   618,
     618,    23,    24,   148,    26,   702,    28,    52,    14,    15,
      16,   122,   123,   201,   763,   764,   765,   122,   123,   146,
     154,   132,   133,   134,   160,    54,   137,   132,   133,   134,
      59,   227,   137,   158,    16,    64,   229,    66,    60,    68,
     641,    70,   230,    72,    37,    68,    69,    70,   227,   229,
      52,   229,    75,    76,   230,    78,    49,    50,    51,    56,
      53,   225,   226,    56,    10,    11,    12,    60,    61,   233,
     227,    64,    65,   162,   613,   614,   615,   229,    52,   227,
     227,   620,   210,   211,   212,   213,   214,    56,   229,   227,
     230,   692,   229,   182,   183,   184,   227,   186,    52,    52,
     122,   123,   191,   227,   705,   227,   227,   227,     3,   710,
     132,   133,   134,   227,   225,   137,   227,     3,     3,   208,
     225,     3,   233,     3,     3,     3,   227,   227,   233,   225,
      11,   670,    23,    26,    24,   674,   675,   231,    77,    78,
     679,   230,    11,   744,   745,   746,    22,   748,    87,   819,
      89,    90,   773,   230,   773,   773,    52,   164,   165,   166,
     167,   168,   169,   170,   171,    52,   145,    42,   175,   176,
     177,   152,   155,    15,   229,   714,   227,   716,     6,     7,
       8,     9,    10,   227,    52,    52,   787,   788,    52,   227,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   869,
     227,   227,   227,   225,     3,    71,   227,    15,    15,   227,
      16,   233,   227,   220,   221,   227,   229,   818,   226,   820,
     821,   822,    11,   824,   825,    11,    22,   225,   829,     6,
       7,     8,     9,    10,   173,   774,   775,   230,   777,   227,
     227,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     227,     6,     7,     8,     9,    10,   227,   227,   859,   880,
     861,   880,   880,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     6,     7,     8,     9,    10,   227,   227,   227,
     199,    16,   229,   222,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    49,   227,   229,   227,   229,    54,   227,
     227,     6,     7,     8,     9,    10,    62,   230,   229,   227,
     849,   850,   851,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     6,     7,     8,     9,    10,     3,     3,   226,
     231,   231,   231,   227,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     6,     7,     8,     9,    10,    18,    11,
     227,    18,    16,   227,   227,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     6,     7,     8,     9,    10,   227,
     227,   227,   225,   227,   226,   230,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   229,   142,   227,     6,     7,
       8,     9,    10,   230,   227,   231,   230,   227,   160,   227,
      18,    19,    20,    21,    22,    23,    24,    25,    26,     6,
       7,     8,     9,    10,   227,   227,   227,   173,   227,   120,
     230,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     6,
       7,     8,     9,    10,   200,   227,   202,   227,   227,   225,
     227,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     226,     6,     7,     8,     9,    10,   222,   228,   228,   227,
     229,   225,   227,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     6,     7,     8,     9,    10,   225,   160,   187,
     227,   227,   600,   227,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    11,    12,   230,    14,    15,     5,   227,
     227,     6,     7,     8,     9,    10,   227,   227,   159,   358,
     586,   604,   227,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    15,   445,    42,   239,   491,   655,   270,   843,
     880,   777,   226,   618,   677,    37,   610,   108,   109,   110,
     111,   112,    60,   114,   115,   116,   117,   278,   456,    42,
     409,   715,    37,   226,    56,   700,    -1,    -1,    60,    61,
      -1,    63,    64,    65,    -1,    72,    -1,    60,    -1,    71,
      -1,    56,    -1,    -1,   226,    60,    61,    -1,    -1,    64,
      65,    -1,    15,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   226,    -1,
     118,    -1,   120,   121,    89,    -1,    -1,    37,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,   124,   125,   226,
     127,   128,   129,   130,    -1,   118,    56,    -1,   121,    -1,
      60,    61,    -1,    -1,    64,    65,   108,   109,   110,   111,
     112,    -1,   114,   115,   116,   117,    -1,    -1,    -1,   226,
      -1,    71,    -1,    -1,    -1,   226,    -1,   164,   165,   166,
     167,   168,   169,   170,   171,    -1,    -1,    37,   175,   176,
     177,   226,    42,    -1,    94,   108,   109,   110,   111,   112,
      -1,   114,   115,   116,   117,    -1,    56,    -1,    -1,   191,
      60,    61,   226,    -1,    64,    65,    -1,    -1,    -1,    -1,
      69,   203,   204,   205,   206,   207,   208,   225,    77,    78,
      -1,    -1,   230,   220,   221,   217,    -1,    -1,    -1,    -1,
      -1,   226,    -1,    92,    93,    94,    95,    97,   230,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,
      -1,    -1,   162,    -1,   174,   230,    -1,    -1,   178,    -1,
     180,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,   179,
      -1,   181,   182,   183,   184,   185,   186,    -1,    -1,   189,
      -1,   191,   192,   143,   144,   195,   146,   147,    -1,   149,
     150,   151,    -1,   153,    -1,     3,   156,   157,   208,   159,
      -1,    37,    -1,    -1,    -1,    -1,    14,    15,    -1,    -1,
     230,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     230,     6,     7,     8,     9,    10,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      86,    -1,    -1,    -1,    -1,    91,    64,    -1,    66,    -1,
      96,    97,    98,    99,    -1,     6,     7,     8,     9,    10,
     230,    79,    80,    81,    82,    -1,    84,    18,    19,    20,
      21,    22,    23,    24,    25,    26,     6,     7,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,    -1,   119
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short int yystos[] =
{
       0,     4,   236,   237,    37,    47,    48,    86,    91,    96,
      97,    98,    99,   251,   252,   284,   285,   286,   288,   289,
     290,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     304,   305,   306,   307,   374,   375,   376,   393,   395,     0,
       3,    29,    34,    37,    56,    57,    59,    66,   100,   141,
     227,   239,   246,   254,   256,   259,   261,   262,   267,   269,
     270,   309,   311,   321,   346,   347,   348,   349,   377,   378,
     379,   380,   389,   390,   394,   396,   397,   398,    15,    16,
      36,   308,    16,   253,     3,    77,    78,    87,    89,    90,
     173,   222,    69,    77,    78,    92,    93,    94,    95,    11,
      31,    36,   302,   308,   308,   308,    15,    16,    35,    46,
     247,    77,    83,    15,    58,    15,    15,    11,    12,    14,
      15,    16,    23,    24,    26,    28,    60,   122,   123,   132,
     133,   134,   137,   225,   233,   325,   326,   328,   335,   336,
     339,    15,   263,   263,    49,    54,    62,   142,   173,   200,
     202,   222,   263,    37,    56,    60,    61,    63,    64,    65,
     230,   264,   273,   263,    42,    97,   127,   143,   144,   146,
     147,   149,   150,   151,   153,   156,   157,   159,   230,   264,
     263,   174,   178,   180,   230,   264,   350,   351,   352,   263,
     201,   230,   264,   263,    63,    71,   191,   203,   204,   205,
     206,   207,   208,   217,   230,   264,   281,   282,   283,   388,
     263,    89,   230,   264,   391,   392,    49,    50,    51,    53,
     230,   255,   258,   264,    51,   230,   257,   264,    31,    36,
     229,   274,     3,    15,    36,   287,     3,    88,     3,   291,
     287,    63,    94,   287,   287,     3,     3,     3,     3,     3,
       3,     3,    11,   303,     3,     3,     3,    18,    36,   229,
     248,   274,    11,    15,   225,   238,    11,    13,    14,   260,
     310,   322,    26,   225,   139,   139,    11,   232,   333,   334,
     339,   225,   225,   225,   225,    11,   339,    11,    12,   225,
     325,   326,   327,   328,   329,   330,   331,     5,    72,   124,
     125,   127,   128,   129,   130,   164,   165,   166,   167,   168,
     169,   170,   171,   175,   176,   177,   220,   221,   338,   399,
       6,     7,     8,     9,    10,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   229,   229,   229,   274,    15,   266,
     238,    30,    31,    32,    33,   238,   240,   271,   272,    36,
      66,    67,   140,   265,   318,   265,   227,   227,   231,   229,
     161,   162,   163,   172,   145,    14,    15,   241,    52,   148,
      52,   238,   146,   154,   165,   166,   158,   160,   227,   229,
     260,   240,   238,   227,   353,   230,   229,    56,   227,   229,
     386,   387,    16,    67,   215,   216,   381,   238,   238,   238,
      52,    52,   144,   218,   219,   227,   227,   229,   227,   231,
     229,   241,   242,   227,   106,   152,   223,   224,   227,    56,
     240,    52,   240,   227,   227,   227,    52,   227,   227,    68,
      69,    70,    75,    76,    78,   275,   276,     3,   287,     3,
       3,     3,    36,    56,    64,    66,    79,    80,    81,    82,
      84,   242,   292,   318,     3,   287,   287,     3,     3,     3,
      14,   238,    37,    38,    39,    40,    41,    42,    43,    44,
      45,   249,   250,   227,   339,   227,   108,   109,   110,   111,
     112,   114,   115,   116,   117,   312,   313,   314,   316,   320,
     225,   319,    11,   339,   343,   344,    23,    24,   334,    26,
     339,   339,   339,   339,   231,   226,    11,    23,    24,   229,
      22,   231,   234,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   110,   126,   131,   339,   339,   339,   339,    16,
     242,   268,    15,   316,   317,   339,   272,    52,    52,    42,
     241,   238,   145,   238,    52,   238,   152,    52,   155,   238,
     238,   240,    52,   229,    63,   179,   196,   198,   354,   355,
     358,    15,   268,   227,   227,   227,   227,   238,   238,   227,
     227,   227,    71,   277,   278,   279,   387,    26,   228,   244,
      16,   241,   238,   238,   238,   230,   231,     3,    15,   317,
     241,   242,   243,    15,    16,   373,   373,   227,   227,   230,
     231,   226,    15,   113,   227,   229,   226,   313,   315,   320,
     231,   226,   226,   231,   231,   231,    11,    11,    22,   330,
     225,   242,   238,   238,   240,   227,   227,   238,   227,   238,
     227,   241,   238,   227,   227,   227,   227,   238,   368,   371,
     372,   181,   240,   229,   199,   227,   229,   227,   229,   384,
     385,   227,   227,    16,   230,   227,   229,   238,   241,   276,
       3,     3,   250,   314,    15,    42,    60,   101,   102,   103,
     104,   105,   106,   107,   118,   121,   323,   336,   337,   340,
     345,   226,   227,   343,   339,   339,   339,   226,   231,   329,
     339,   227,   231,   227,   227,   227,   227,   227,   227,   268,
     231,   238,   360,   260,   356,   158,   209,   230,   383,   277,
      58,    85,   105,   280,    18,   225,    18,   225,   339,   339,
     339,   230,   338,   399,   227,   339,   341,   342,   323,   313,
     226,   226,   226,    11,   226,   238,   229,   370,   371,   230,
     242,   361,   362,   363,   162,   182,   183,   184,   186,   191,
     230,   281,   357,   238,   210,   211,   212,   213,   214,   382,
     227,   238,    72,    74,    72,    74,   230,   339,   344,   339,
     336,   227,   119,   229,    18,    18,    22,   231,   227,   230,
     227,   369,   197,   227,   238,   238,   238,   160,    81,   238,
      67,   190,   193,   227,   227,    16,   245,   245,   245,   245,
     227,   226,   227,   399,   229,   324,   345,   120,   330,   332,
     339,   339,   133,   134,   137,   139,   342,    94,   162,   179,
     181,   182,   183,   184,   185,   186,   189,   191,   192,   195,
     230,   281,   359,   227,   238,   238,   280,   227,   227,   227,
     227,   226,   323,   120,   228,   228,   230,   227,   227,   225,
     225,   225,   227,   229,   238,   240,   238,   238,   238,   160,
     238,    81,   238,   187,   188,   210,    67,   190,   193,   187,
     238,   227,   280,   280,   280,   280,   227,   230,   324,   323,
     323,   339,   339,   339,   364,   238,   238,   240,   332,   226,
     226,   226,    14,   230,   365,   366,   367,   227,   227,   227,
     194,   227,   227
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  yyvsp[0] = yylval;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
#line 242 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:
#line 243 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:
#line 256 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[-1].x)); ;}
    break;

  case 8:
#line 257 "cf-parse.y"
    { if ((yyvsp[0].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[0].s)->aux; ;}
    break;

  case 9:
#line 261 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[-3].s), SYM_NUMBER, NULL);
     (yyvsp[-3].s)->aux = (yyvsp[-1].i);
   ;}
    break;

  case 10:
#line 265 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[-3].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[-3].s)->def = (yyvsp[-1].a);
   ;}
    break;

  case 11:
#line 274 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[0].i); ;}
    break;

  case 12:
#line 275 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:
#line 276 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:
#line 277 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:
#line 278 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:
#line 279 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:
#line 286 "cf-parse.y"
    {
     if ((yyvsp[0].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[0].s)->def;
   ;}
    break;

  case 19:
#line 293 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[-1].a), (yyvsp[0].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[-1].a); (yyval.px).len = (yyvsp[0].i);
   ;}
    break;

  case 21:
#line 301 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[0].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:
#line 305 "cf-parse.y"
    {
     if ((yyvsp[0].i) < 0 || (yyvsp[0].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[0].i));
     (yyval.i) = (yyvsp[0].i);
   ;}
    break;

  case 23:
#line 309 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[0].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[0].a));
   ;}
    break;

  case 24:
#line 316 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[0].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:
#line 326 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[-2].g);
    c->mask = (yyvsp[-1].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 26:
#line 335 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[0].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[0].t));
     (yyval.g) = f;
   ;}
    break;

  case 27:
#line 340 "cf-parse.y"
    { (yyval.g) = NULL; ;}
    break;

  case 28:
#line 341 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 29:
#line 345 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 30:
#line 346 "cf-parse.y"
    { (yyval.i) = (yyvsp[-1].i); ;}
    break;

  case 31:
#line 350 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[0].i); ;}
    break;

  case 32:
#line 351 "cf-parse.y"
    { (yyval.i) = (yyvsp[-2].i) | (1 << (yyvsp[0].i)); ;}
    break;

  case 33:
#line 355 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 34:
#line 356 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 35:
#line 357 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 36:
#line 358 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 37:
#line 359 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 38:
#line 360 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 39:
#line 361 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 40:
#line 362 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 41:
#line 363 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 42:
#line 369 "cf-parse.y"
    { cmd_reconfig((yyvsp[-1].t)); ;}
    break;

  case 43:
#line 372 "cf-parse.y"
    { cli_msg(7, "Shutdown requested"); order_shutdown(); ;}
    break;

  case 44:
#line 375 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 46:
#line 384 "cf-parse.y"
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

  case 47:
#line 400 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[0].i); ;}
    break;

  case 48:
#line 401 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[0].i);
   ;}
    break;

  case 49:
#line 405 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[0].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[0].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   ;}
    break;

  case 50:
#line 417 "cf-parse.y"
    {
     if (cf_kif)
       cf_error("Kernel device protocol already defined");
     cf_kif = this_proto = proto_config_new(&proto_unix_iface, sizeof(struct kif_config));
     this_proto->preference = DEF_PREF_DIRECT;
     THIS_KIF->scan_time = 60;
     krt_if_construct(THIS_KIF);
   ;}
    break;

  case 51:
#line 429 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[0].i);
   ;}
    break;

  case 52:
#line 439 "cf-parse.y"
    {
#ifndef IPV6
	if ((yyvsp[0].i) <= 0 || (yyvsp[0].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
#else
	if ((yyvsp[0].i) != 254)
	  cf_error("Linux implementation of IPv6 doesn't support multiple routing tables");
#endif
	THIS_KRT->scan.table_id = (yyvsp[0].i);
   ;}
    break;

  case 53:
#line 456 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[-1].i32);
   ;}
    break;

  case 54:
#line 462 "cf-parse.y"
    { (yyval.i32) = (yyvsp[0].i); ;}
    break;

  case 56:
#line 464 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[0].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   ;}
    break;

  case 57:
#line 476 "cf-parse.y"
    {
   rt_new_table((yyvsp[0].s));
   ;}
    break;

  case 59:
#line 488 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 60:
#line 494 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[0].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[0].s)->name;
   ;}
    break;

  case 62:
#line 502 "cf-parse.y"
    {
     if ((yyvsp[0].i) < 0 || (yyvsp[0].i) > 255) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[0].i);
   ;}
    break;

  case 63:
#line 506 "cf-parse.y"
    { this_proto->disabled = (yyvsp[0].i); ;}
    break;

  case 64:
#line 507 "cf-parse.y"
    { this_proto->debug = (yyvsp[0].i); ;}
    break;

  case 65:
#line 508 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[0].f); ;}
    break;

  case 66:
#line 509 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[0].f); ;}
    break;

  case 67:
#line 510 "cf-parse.y"
    { this_proto->table = (yyvsp[0].r); ;}
    break;

  case 68:
#line 514 "cf-parse.y"
    { (yyval.f) = (yyvsp[0].f); ;}
    break;

  case 70:
#line 516 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 71:
#line 517 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 72:
#line 521 "cf-parse.y"
    {
     if ((yyvsp[0].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[0].s)->def;
   ;}
    break;

  case 73:
#line 529 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[0].i); ;}
    break;

  case 74:
#line 530 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[0].i); ;}
    break;

  case 75:
#line 536 "cf-parse.y"
    { this_ipatt->pattern = (yyvsp[0].t); this_ipatt->prefix = IPA_NONE; this_ipatt->pxlen = 0; ;}
    break;

  case 76:
#line 537 "cf-parse.y"
    { this_ipatt->pattern = NULL; this_ipatt->prefix = (yyvsp[0].px).addr; this_ipatt->pxlen = (yyvsp[0].px).len; ;}
    break;

  case 77:
#line 538 "cf-parse.y"
    { this_ipatt->pattern = (yyvsp[-1].t); this_ipatt->prefix = (yyvsp[0].px).addr; this_ipatt->pxlen = (yyvsp[0].px).len; ;}
    break;

  case 78:
#line 544 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 82:
#line 559 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     struct iface_patt *k = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, &k->n);
     this_ipatt = k;
   ;}
    break;

  case 86:
#line 579 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 87:
#line 580 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 88:
#line 581 "cf-parse.y"
    { (yyval.i) = (yyvsp[-1].i); ;}
    break;

  case 90:
#line 586 "cf-parse.y"
    { (yyval.i) = (yyvsp[-2].i) | (yyvsp[0].i); ;}
    break;

  case 91:
#line 590 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 92:
#line 591 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 93:
#line 592 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 94:
#line 593 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 95:
#line 594 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 96:
#line 595 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 101:
#line 611 "cf-parse.y"
    {
     static int id = 1;
     this_p_item = cfg_alloc(sizeof (struct password_item));
     this_p_item->password = (yyvsp[0].t);
     this_p_item->genfrom = 0;
     this_p_item->gento = TIME_INFINITY;
     this_p_item->accfrom = 0;
     this_p_item->accto = TIME_INFINITY;
     this_p_item->id = id++;
     add_tail(this_p_list, &this_p_item->n);
   ;}
    break;

  case 102:
#line 625 "cf-parse.y"
    { ;}
    break;

  case 103:
#line 626 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[-2].time); ;}
    break;

  case 104:
#line 627 "cf-parse.y"
    { this_p_item->gento = (yyvsp[-2].time); ;}
    break;

  case 105:
#line 628 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[-2].time); ;}
    break;

  case 106:
#line 629 "cf-parse.y"
    { this_p_item->accto = (yyvsp[-2].time); ;}
    break;

  case 107:
#line 630 "cf-parse.y"
    { this_p_item->id = (yyvsp[-2].i); if ((yyvsp[-2].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 108:
#line 634 "cf-parse.y"
    {
     (yyval.p) = (yyvsp[-3].p);
   ;}
    break;

  case 110:
#line 641 "cf-parse.y"
    {
     this_p_list = cfg_alloc(sizeof(list));
     init_list(this_p_list);
     (yyval.p) = (void *) this_p_list;
  ;}
    break;

  case 111:
#line 649 "cf-parse.y"
    {
     this_p_list = cfg_alloc(sizeof(list));
     init_list(this_p_list);
     this_p_item = cfg_alloc(sizeof (struct password_item));
     this_p_item->password = (yyvsp[0].t);
     this_p_item->genfrom = 0;
     this_p_item->gento = TIME_INFINITY;
     this_p_item->accfrom = 0;
     this_p_item->accto = TIME_INFINITY;
     this_p_item->id = 1;
     add_tail(this_p_list, &this_p_item->n);
     (yyval.p) = (void *) this_p_list;
  ;}
    break;

  case 112:
#line 668 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 113:
#line 671 "cf-parse.y"
    { proto_show((yyvsp[-1].s), 0); ;}
    break;

  case 114:
#line 674 "cf-parse.y"
    { proto_show((yyvsp[-1].s), 1); ;}
    break;

  case 116:
#line 678 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 117:
#line 682 "cf-parse.y"
    { if_show(); ;}
    break;

  case 118:
#line 685 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 119:
#line 688 "cf-parse.y"
    { rt_show((yyvsp[-1].ra)); ;}
    break;

  case 120:
#line 691 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 121:
#line 697 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[0].px).addr;
     (yyval.ra)->pxlen = (yyvsp[0].px).len;
   ;}
    break;

  case 122:
#line 703 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[0].px).addr;
     (yyval.ra)->pxlen = (yyvsp[0].px).len;
     (yyval.ra)->show_for = 1;
   ;}
    break;

  case 123:
#line 710 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyvsp[0].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[0].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[0].s)->def)->table;
   ;}
    break;

  case 124:
#line 715 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[0].f);
   ;}
    break;

  case 125:
#line 720 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[0].f);
   ;}
    break;

  case 126:
#line 725 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 127:
#line 729 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 128:
#line 733 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[0].s)->def;
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->import_mode) cf_error("Protocol specified twice");
     if ((yyvsp[0].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[0].s)->name);
     (yyval.ra)->import_mode = (yyvsp[-1].i);
     (yyval.ra)->primary_only = 1;
     (yyval.ra)->import_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 129:
#line 743 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 130:
#line 747 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 131:
#line 754 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 132:
#line 755 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 133:
#line 759 "cf-parse.y"
    { cmd_show_symbols((yyvsp[-1].s)); ;}
    break;

  case 134:
#line 763 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 135:
#line 765 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 136:
#line 767 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 137:
#line 769 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 138:
#line 771 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 139:
#line 773 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 140:
#line 775 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 141:
#line 777 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[-2].i), (yyvsp[-1].i));
  cli_msg(0, "");
;}
    break;

  case 142:
#line 783 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 143:
#line 784 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 145:
#line 789 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 146:
#line 790 "cf-parse.y"
    {
     if ((yyvsp[0].i) < 256 || (yyvsp[0].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[0].i);
   ;}
    break;

  case 147:
#line 797 "cf-parse.y"
    { proto_xxable((yyvsp[-1].t), 0); ;}
    break;

  case 148:
#line 799 "cf-parse.y"
    { proto_xxable((yyvsp[-1].t), 1); ;}
    break;

  case 149:
#line 801 "cf-parse.y"
    { proto_xxable((yyvsp[-1].t), 2); ;}
    break;

  case 150:
#line 805 "cf-parse.y"
    { proto_debug((yyvsp[-2].t), (yyvsp[-1].i)); ;}
    break;

  case 151:
#line 809 "cf-parse.y"
    { (yyval.t) = (yyvsp[0].s)->name; ;}
    break;

  case 152:
#line 810 "cf-parse.y"
    { (yyval.t) = "*"; ;}
    break;

  case 154:
#line 817 "cf-parse.y"
    { cf_push_scope( (yyvsp[0].s) ); ;}
    break;

  case 155:
#line 817 "cf-parse.y"
    {
     (yyvsp[-2].s) = cf_define_symbol((yyvsp[-2].s), SYM_FILTER, (yyvsp[0].f));
     (yyvsp[0].f)->name = (yyvsp[-2].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[-2].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 156:
#line 826 "cf-parse.y"
    { f_eval_int((yyvsp[0].x)); ;}
    break;

  case 157:
#line 830 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 158:
#line 831 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 159:
#line 832 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 160:
#line 833 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 161:
#line 834 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 162:
#line 835 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 163:
#line 836 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 164:
#line 837 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 165:
#line 838 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 166:
#line 839 "cf-parse.y"
    { 
	switch ((yyvsp[-1].i)) {
	  default:
		cf_error( "You can't create sets of this type." );
	  case T_INT: case T_IP: case T_PREFIX: case T_PAIR: ;
	}
	(yyval.i) = (yyvsp[-1].i) | T_SET;
	;}
    break;

  case 167:
#line 850 "cf-parse.y"
    {
     (yyvsp[0].s) = cf_define_symbol((yyvsp[0].s), SYM_VARIABLE | (yyvsp[-1].i), NULL);
     DBG( "New variable %s type %x\n", (yyvsp[0].s)->name, (yyvsp[-1].i) );
     (yyvsp[0].s)->aux = 0;
     {
       struct f_val * val; 
       val = cfg_alloc(sizeof(struct f_val)); 
       val->type = (yyvsp[-1].i); 
       (yyvsp[0].s)->aux2 = val;
     }
     (yyval.s)=(yyvsp[0].s);
   ;}
    break;

  case 168:
#line 865 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 169:
#line 866 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[-2].s);
     (yyval.s)->aux = (int) (yyvsp[0].s);
   ;}
    break;

  case 170:
#line 873 "cf-parse.y"
    { (yyval.s) = (yyvsp[0].s); ;}
    break;

  case 171:
#line 874 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[-2].s);
     (yyval.s)->aux = (int) (yyvsp[0].s);
   ;}
    break;

  case 172:
#line 881 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[0].x);
     (yyval.f) = f;
   ;}
    break;

  case 173:
#line 890 "cf-parse.y"
    {
     if ((yyvsp[0].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[0].s)->def;
   ;}
    break;

  case 175:
#line 898 "cf-parse.y"
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
     i->a1.p = (yyvsp[0].x);
     i->a2.p = acc;
     i->next = rej;
     f->name = NULL;
     f->root = i;
     (yyval.f) = f;
  ;}
    break;

  case 176:
#line 922 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[-1].s); ;}
    break;

  case 177:
#line 923 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 178:
#line 927 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[-1].x);
   ;}
    break;

  case 179:
#line 933 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[0].s)->name );
     (yyvsp[0].s) = cf_define_symbol((yyvsp[0].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[0].s));
   ;}
    break;

  case 180:
#line 936 "cf-parse.y"
    {
     (yyvsp[-3].s)->def = (yyvsp[0].x);
     (yyvsp[-3].s)->aux = (int) (yyvsp[-1].s);
     (yyvsp[-3].s)->aux2 = (yyvsp[0].x);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[-3].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 181:
#line 947 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 182:
#line 948 "cf-parse.y"
    {
     if ((yyvsp[-1].x)) {
       if ((yyvsp[-1].x)->next)
	 bug("Command has next already set");
       (yyvsp[-1].x)->next = (yyvsp[0].x);
       (yyval.x) = (yyvsp[-1].x);
     } else (yyval.x) = (yyvsp[0].x);
   ;}
    break;

  case 183:
#line 959 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[0].x);
   ;}
    break;

  case 184:
#line 962 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[-1].x);
   ;}
    break;

  case 185:
#line 971 "cf-parse.y"
    { (yyval.i) = (yyvsp[-3].i) << 16 | (yyvsp[-1].i); ;}
    break;

  case 186:
#line 978 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[-2].a), (yyvsp[0].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[-2].a), (yyvsp[0].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[-2].a); (yyval.v).val.px.len = (yyvsp[0].i);
   ;}
    break;

  case 187:
#line 985 "cf-parse.y"
    { (yyval.v) = (yyvsp[0].v); ;}
    break;

  case 188:
#line 986 "cf-parse.y"
    { (yyval.v) = (yyvsp[-1].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 189:
#line 987 "cf-parse.y"
    { (yyval.v) = (yyvsp[-1].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 190:
#line 988 "cf-parse.y"
    { (yyval.v) = (yyvsp[-5].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[-3].i) << 16) | ((yyvsp[-1].i) << 8); ;}
    break;

  case 191:
#line 992 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[0].a); ;}
    break;

  case 192:
#line 996 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[0].i); ;}
    break;

  case 193:
#line 997 "cf-parse.y"
    { (yyval.v).type = T_PAIR; (yyval.v).val.i = (yyvsp[0].i); ;}
    break;

  case 194:
#line 998 "cf-parse.y"
    { (yyval.v) = (yyvsp[0].v); ;}
    break;

  case 195:
#line 999 "cf-parse.y"
    { (yyval.v) = (yyvsp[0].v); ;}
    break;

  case 196:
#line 1000 "cf-parse.y"
    {  (yyval.v).type = (yyvsp[0].i) >> 16; (yyval.v).val.i = (yyvsp[0].i) & 0xffff; ;}
    break;

  case 197:
#line 1004 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[0].v); 
	if ((yyvsp[0].v).type != T_PREFIX)
		(yyval.e)->to = (yyvsp[0].v);
	else {
		(yyval.e)->to = (yyvsp[0].v);
		(yyval.e)->to.val.px.ip = ipa_or( (yyval.e)->to.val.px.ip, ipa_not( ipa_mkmask( (yyval.e)->to.val.px.len ) ));
	}
   ;}
    break;

  case 198:
#line 1014 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[-3].v); 
	(yyval.e)->to = (yyvsp[0].v); 
	if (((yyvsp[-3].v).type == T_PREFIX) || ((yyvsp[0].v).type == T_PREFIX)) cf_error( "You can't use prefixes for range." ); 
   ;}
    break;

  case 199:
#line 1023 "cf-parse.y"
    { (yyval.e) = (yyvsp[0].e); ;}
    break;

  case 200:
#line 1024 "cf-parse.y"
    { (yyval.e) = (yyvsp[0].e); (yyval.e)->left = (yyvsp[-2].e); ;}
    break;

  case 201:
#line 1027 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 202:
#line 1028 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[-3].e);
     (yyval.e)->data = (yyvsp[-1].x);
     (yyval.e)->left = (yyvsp[0].e);
   ;}
    break;

  case 203:
#line 1033 "cf-parse.y"
    {
     (yyval.e) = f_new_tree(); 
     (yyval.e)->from.type = T_VOID; 
     (yyval.e)->to.type = T_VOID;
     (yyval.e)->data = (yyvsp[0].x);
   ;}
    break;

  case 204:
#line 1044 "cf-parse.y"
    { (yyval.i) = (yyvsp[0].i); ;}
    break;

  case 205:
#line 1045 "cf-parse.y"
    { (yyval.i) = PM_ANY; ;}
    break;

  case 206:
#line 1049 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = NULL; (yyval.h)->val  = (yyvsp[0].i); ;}
    break;

  case 207:
#line 1050 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h);   (yyval.h)->val  = (yyvsp[-1].i); ;}
    break;

  case 208:
#line 1054 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[0].i); ;}
    break;

  case 209:
#line 1055 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 210:
#line 1056 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 211:
#line 1057 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[0].t); ;}
    break;

  case 212:
#line 1058 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PAIR;  (yyval.x)->a2.i = (yyvsp[0].i); ;}
    break;

  case 213:
#line 1059 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[0].v); ;}
    break;

  case 214:
#line 1060 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[0].v); ;}
    break;

  case 215:
#line 1061 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[-1].e)); DBG( "ook\n" ); ;}
    break;

  case 216:
#line 1062 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[0].i) >> 16; (yyval.x)->a2.i = (yyvsp[0].i) & 0xffff; ;}
    break;

  case 217:
#line 1063 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[-1].h); (yyval.x)->a1.p = val; ;}
    break;

  case 219:
#line 1076 "cf-parse.y"
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[-1].x);
     if ((yyvsp[-3].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[-3].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[-3].s)->aux2;
     sym = (void *) (yyvsp[-3].s)->aux;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[-3].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = (void *) sym->aux;
       inst = inst->next;
     }
   ;}
    break;

  case 220:
#line 1099 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 221:
#line 1101 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 222:
#line 1102 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 223:
#line 1103 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 224:
#line 1104 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 225:
#line 1105 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 226:
#line 1106 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 227:
#line 1110 "cf-parse.y"
    { (yyval.x) = (yyvsp[-1].x); ;}
    break;

  case 228:
#line 1111 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 229:
#line 1112 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 230:
#line 1113 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 231:
#line 1114 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 232:
#line 1115 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 233:
#line 1116 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 234:
#line 1117 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 235:
#line 1118 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 236:
#line 1119 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 237:
#line 1120 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 238:
#line 1121 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = (yyvsp[-2].x); ;}
    break;

  case 239:
#line 1122 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = (yyvsp[-2].x); ;}
    break;

  case 240:
#line 1123 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); ;}
    break;

  case 241:
#line 1124 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[0].x); ;}
    break;

  case 242:
#line 1125 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[-1].x); ;}
    break;

  case 243:
#line 1127 "cf-parse.y"
    { (yyval.x) = (yyvsp[0].x); ;}
    break;

  case 244:
#line 1128 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     switch ((yyvsp[0].s)->class) {
       case SYM_NUMBER:
	(yyval.x) = f_new_inst();
	(yyval.x)->code = 'c'; 
	(yyval.x)->aux = T_INT; 
	(yyval.x)->a2.i = (yyvsp[0].s)->aux;
	break;
       case SYM_IPA:
	{ NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; val->type = T_IP; val->val.px.ip = * (ip_addr *) ((yyvsp[0].s)->def); }
	break;
       case SYM_VARIABLE | T_INT:
       case SYM_VARIABLE | T_PAIR:
       case SYM_VARIABLE | T_PREFIX:
       case SYM_VARIABLE | T_IP:
       case SYM_VARIABLE | T_PATH_MASK:
       case SYM_VARIABLE | T_PATH:
       case SYM_VARIABLE | T_CLIST:
	 (yyval.x)->code = 'C';
	 (yyval.x)->a1.p = (yyvsp[0].s)->aux2;
	 break;
       default:
	 cf_error("%s: variable expected.", (yyvsp[0].s)->name );
     }
   ;}
    break;

  case 245:
#line 1155 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 246:
#line 1157 "cf-parse.y"
    { (yyval.x) = (yyvsp[0].x); (yyval.x)->code = 'a'; ;}
    break;

  case 247:
#line 1159 "cf-parse.y"
    { (yyval.x) = (yyvsp[0].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 248:
#line 1161 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 249:
#line 1162 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[-2].x); ;}
    break;

  case 250:
#line 1163 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[-5].x); (yyval.x)->a2.p = (yyvsp[-1].x); ;}
    break;

  case 251:
#line 1173 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 252:
#line 1174 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 253:
#line 1175 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); ;}
    break;

  case 254:
#line 1176 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 255:
#line 1177 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 256:
#line 1182 "cf-parse.y"
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[-1].x);
     if ((yyvsp[-3].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[-3].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[-3].s)->aux2;
     sym = (void *) (yyvsp[-3].s)->aux;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[-3].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = (void *) sym->aux;
       inst = inst->next;
     }
   ;}
    break;

  case 257:
#line 1205 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 258:
#line 1206 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 259:
#line 1207 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 260:
#line 1208 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 261:
#line 1209 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 262:
#line 1210 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 263:
#line 1214 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 264:
#line 1217 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 265:
#line 1218 "cf-parse.y"
    { (yyval.x) = (yyvsp[0].x); ;}
    break;

  case 266:
#line 1219 "cf-parse.y"
    {
     if ((yyvsp[-2].x)) {
       (yyvsp[-2].x)->next = (yyvsp[0].x);
       (yyval.x) = (yyvsp[-2].x);
     } else (yyval.x) = (yyvsp[0].x);
   ;}
    break;

  case 267:
#line 1228 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[0].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 268:
#line 1235 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[-2].x);
     (yyval.x)->next = (yyvsp[0].x);
   ;}
    break;

  case 269:
#line 1244 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 270:
#line 1245 "cf-parse.y"
    { (yyval.x) = (yyvsp[0].x); ;}
    break;

  case 271:
#line 1249 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[-2].x);
     (yyval.x)->a2.p = (yyvsp[0].x);
   ;}
    break;

  case 272:
#line 1255 "cf-parse.y"
    {
     struct f_inst *i = f_new_inst();
     i->code = '?';
     i->a1.p = (yyvsp[-4].x);
     i->a2.p = (yyvsp[-2].x);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = i;
     (yyval.x)->a2.p = (yyvsp[0].x);
   ;}
    break;

  case 273:
#line 1265 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll set value\n" );
     if (((yyvsp[-3].s)->class & ~T_MASK) != SYM_VARIABLE)
       cf_error( "You may set only variables." );
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = (yyvsp[-3].s);
     (yyval.x)->a2.p = (yyvsp[-1].x);
   ;}
    break;

  case 274:
#line 1274 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[-1].x);
   ;}
    break;

  case 275:
#line 1280 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[-3].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   ;}
    break;

  case 276:
#line 1285 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[-3].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   ;}
    break;

  case 277:
#line 1292 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   ;}
    break;

  case 278:
#line 1297 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[-2].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 279:
#line 1303 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[-1].x); (yyval.x)->a2.i = (yyvsp[-2].i); ;}
    break;

  case 280:
#line 1304 "cf-parse.y"
    { (yyval.x) = (yyvsp[-1].x); ;}
    break;

  case 281:
#line 1305 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[-3].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[-1].e) );
   ;}
    break;

  case 282:
#line 1314 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[-3].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 283:
#line 1315 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[-6].x), (yyvsp[-2].x) ); ;}
    break;

  case 284:
#line 1316 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[-6].x), (yyvsp[-2].x) ); ;}
    break;

  case 285:
#line 1317 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[-6].x), (yyvsp[-2].x) ); ;}
    break;

  case 286:
#line 1323 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_bgp, sizeof(struct bgp_config));
     this_proto->preference = DEF_PREF_BGP;
     BGP_CFG->hold_time = 240;
     BGP_CFG->connect_retry_time = 120;
     BGP_CFG->initial_hold_time = 240;
     BGP_CFG->default_med = ~0;		/* RFC 1771 doesn't specify this, draft-09 says ~0 */
     BGP_CFG->compare_path_lengths = 1;
     BGP_CFG->start_delay_time = 5;
     BGP_CFG->error_amnesia_time = 300;
     BGP_CFG->error_delay_time_min = 60;
     BGP_CFG->error_delay_time_max = 300;
 ;}
    break;

  case 289:
#line 1341 "cf-parse.y"
    {
     if ((yyvsp[-1].i) < 0 || (yyvsp[-1].i) > 65535) cf_error("AS number out of range");
     BGP_CFG->local_as = (yyvsp[-1].i);
   ;}
    break;

  case 290:
#line 1345 "cf-parse.y"
    {
     if ((yyvsp[-1].i) < 0 || (yyvsp[-1].i) > 65535) cf_error("AS number out of range");
     BGP_CFG->remote_ip = (yyvsp[-3].a);
     BGP_CFG->remote_as = (yyvsp[-1].i);
   ;}
    break;

  case 291:
#line 1350 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[-1].i); ;}
    break;

  case 292:
#line 1351 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[-1].i); ;}
    break;

  case 293:
#line 1352 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[-1].i); ;}
    break;

  case 294:
#line 1353 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[-1].i); ;}
    break;

  case 295:
#line 1354 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[-3].i); BGP_CFG->multihop_via = (yyvsp[-1].a); ;}
    break;

  case 296:
#line 1355 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 297:
#line 1356 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[-1].i); ;}
    break;

  case 298:
#line 1357 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[-1].i); ;}
    break;

  case 299:
#line 1358 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[-1].i); ;}
    break;

  case 300:
#line 1359 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[-1].a); ;}
    break;

  case 301:
#line 1360 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[-1].i); ;}
    break;

  case 302:
#line 1361 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[-1].i); ;}
    break;

  case 303:
#line 1362 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[-3].i); BGP_CFG->error_delay_time_max = (yyvsp[-1].i); ;}
    break;

  case 304:
#line 1363 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[-1].i); ;}
    break;

  case 305:
#line 1372 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 309:
#line 1388 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[0].i); ;}
    break;

  case 310:
#line 1389 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[0].i) ; if((yyvsp[0].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 312:
#line 1393 "cf-parse.y"
    {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = (yyvsp[-1].i32);
  this_area->stub = 0;
  init_list(&this_area->patt_list);
  init_list(&this_area->vlink_list);
  init_list(&this_area->net_list);
 ;}
    break;

  case 316:
#line 1413 "cf-parse.y"
    { this_area->stub = (yyvsp[0].i) ; if((yyvsp[0].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 317:
#line 1414 "cf-parse.y"
    {if((yyvsp[0].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 326:
#line 1431 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 327:
#line 1432 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 328:
#line 1433 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 329:
#line 1434 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[0].i) ; ;}
    break;

  case 330:
#line 1435 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 331:
#line 1436 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 332:
#line 1437 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 333:
#line 1438 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 334:
#line 1439 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 335:
#line 1440 "cf-parse.y"
    {OSPF_PATT->passwords = (list *) (yyvsp[0].p); ;}
    break;

  case 336:
#line 1444 "cf-parse.y"
    {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->vlink_list, NODE this_ipatt);
  OSPF_PATT->vid = (yyvsp[0].i32);
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
 ;}
    break;

  case 337:
#line 1463 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 338:
#line 1464 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 339:
#line 1465 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 340:
#line 1466 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 341:
#line 1467 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 342:
#line 1468 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[0].i) ; if (((yyvsp[0].i)<0) || ((yyvsp[0].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 343:
#line 1469 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[0].i) ; ;}
    break;

  case 344:
#line 1470 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 345:
#line 1471 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 346:
#line 1472 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 347:
#line 1473 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 348:
#line 1474 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 349:
#line 1475 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[0].i) ; ;}
    break;

  case 350:
#line 1476 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[0].i) ; ;}
    break;

  case 352:
#line 1478 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 353:
#line 1479 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 354:
#line 1480 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 355:
#line 1481 "cf-parse.y"
    {OSPF_PATT->passwords = (list *) (yyvsp[0].p); ;}
    break;

  case 360:
#line 1494 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[-1].px).addr;
   this_pref->px.len = (yyvsp[-1].px).len;
 ;}
    break;

  case 361:
#line 1503 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[-2].px).addr;
   this_pref->px.len = (yyvsp[-2].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 366:
#line 1522 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[-1].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 367:
#line 1531 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[-2].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 368:
#line 1541 "cf-parse.y"
    {
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->patt_list, NODE this_ipatt);
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
 ;}
    break;

  case 377:
#line 1582 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 378:
#line 1587 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[-1].s), &proto_ospf)); ;}
    break;

  case 379:
#line 1590 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[-2].s), &proto_ospf), (yyvsp[-1].t)); ;}
    break;

  case 380:
#line 1593 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[-2].s), &proto_ospf), (yyvsp[-1].t)); ;}
    break;

  case 381:
#line 1598 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
  ;}
    break;

  case 384:
#line 1607 "cf-parse.y"
    {
     if ((yyvsp[-1].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     ((struct pipe_config *) this_proto)->peer = (yyvsp[-1].s)->def;
   ;}
    break;

  case 385:
#line 1617 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 388:
#line 1626 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[-1].i); ;}
    break;

  case 389:
#line 1627 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[-1].i); ;}
    break;

  case 390:
#line 1628 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[-1].i); ;}
    break;

  case 391:
#line 1629 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[-1].i); ;}
    break;

  case 392:
#line 1630 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[-1].i); ;}
    break;

  case 393:
#line 1631 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[-1].i); ;}
    break;

  case 394:
#line 1632 "cf-parse.y"
    {RIP_CFG->passwords = (list *)(yyvsp[-1].p); ;}
    break;

  case 395:
#line 1633 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 396:
#line 1634 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 397:
#line 1635 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 399:
#line 1640 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 400:
#line 1641 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 401:
#line 1642 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 402:
#line 1646 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 403:
#line 1647 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 404:
#line 1648 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 405:
#line 1649 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 406:
#line 1650 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 408:
#line 1654 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[0].i); ;}
    break;

  case 409:
#line 1655 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[0].i); ;}
    break;

  case 414:
#line 1666 "cf-parse.y"
    {
     struct rip_patt *k = cfg_allocz(sizeof(struct rip_patt));
     k->metric = 1;
     add_tail(&RIP_CFG->iface_list, &k->i.n);
     this_ipatt = &k->i;
   ;}
    break;

  case 418:
#line 1687 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 422:
#line 1699 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&((struct static_config *) this_proto)->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[0].px).addr;
     this_srt->masklen = (yyvsp[0].px).len;
  ;}
    break;

  case 423:
#line 1708 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[0].a);
   ;}
    break;

  case 424:
#line 1712 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[0].t);
      rem_node(&this_srt->n);
      add_tail(&((struct static_config *) this_proto)->iface_routes, &this_srt->n);
   ;}
    break;

  case 425:
#line 1718 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 426:
#line 1719 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 427:
#line 1720 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 428:
#line 1724 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[-1].s), &proto_static)); ;}
    break;

  case 467:
#line 1732 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 479:
#line 1735 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 480:
#line 1736 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 481:
#line 1737 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 482:
#line 1738 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 483:
#line 1739 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 484:
#line 1740 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 485:
#line 1741 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 486:
#line 1742 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 487:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 488:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 489:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 490:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 491:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 492:
#line 1743 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 4948 "cf-parse.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1745 "cf-parse.y"

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



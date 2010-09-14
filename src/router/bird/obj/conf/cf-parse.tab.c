/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

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
     TIMEFORMAT = 294,
     ISO = 295,
     SHORT = 296,
     LONG = 297,
     BASE = 298,
     NAME = 299,
     CONFIGURE = 300,
     DOWN = 301,
     KERNEL = 302,
     PERSIST = 303,
     SCAN = 304,
     TIME = 305,
     LEARN = 306,
     DEVICE = 307,
     ROUTES = 308,
     ASYNC = 309,
     TABLE = 310,
     ROUTER = 311,
     ID = 312,
     PROTOCOL = 313,
     PREFERENCE = 314,
     DISABLED = 315,
     DIRECT = 316,
     INTERFACE = 317,
     IMPORT = 318,
     EXPORT = 319,
     FILTER = 320,
     NONE = 321,
     STATES = 322,
     FILTERS = 323,
     PASSWORD = 324,
     FROM = 325,
     PASSIVE = 326,
     TO = 327,
     EVENTS = 328,
     PACKETS = 329,
     PROTOCOLS = 330,
     INTERFACES = 331,
     PRIMARY = 332,
     STATS = 333,
     COUNT = 334,
     FOR = 335,
     COMMANDS = 336,
     PREEXPORT = 337,
     GENERATE = 338,
     LISTEN = 339,
     BGP = 340,
     V6ONLY = 341,
     ADDRESS = 342,
     PORT = 343,
     PASSWORDS = 344,
     DESCRIPTION = 345,
     RELOAD = 346,
     IN = 347,
     OUT = 348,
     MRTDUMP = 349,
     MESSAGES = 350,
     RESTRICT = 351,
     MEMORY = 352,
     SHOW = 353,
     STATUS = 354,
     SUMMARY = 355,
     ROUTE = 356,
     SYMBOLS = 357,
     DUMP = 358,
     RESOURCES = 359,
     SOCKETS = 360,
     NEIGHBORS = 361,
     ATTRIBUTES = 362,
     ECHO = 363,
     DISABLE = 364,
     ENABLE = 365,
     RESTART = 366,
     FUNCTION = 367,
     PRINT = 368,
     PRINTN = 369,
     UNSET = 370,
     RETURN = 371,
     ACCEPT = 372,
     REJECT = 373,
     QUITBIRD = 374,
     INT = 375,
     BOOL = 376,
     IP = 377,
     PREFIX = 378,
     PAIR = 379,
     QUAD = 380,
     SET = 381,
     STRING = 382,
     BGPMASK = 383,
     BGPPATH = 384,
     CLIST = 385,
     IF = 386,
     THEN = 387,
     ELSE = 388,
     CASE = 389,
     TRUE = 390,
     FALSE = 391,
     GW = 392,
     NET = 393,
     MASK = 394,
     PROTO = 395,
     SOURCE = 396,
     SCOPE = 397,
     CAST = 398,
     DEST = 399,
     LEN = 400,
     DEFINED = 401,
     ADD = 402,
     DELETE = 403,
     CONTAINS = 404,
     RESET = 405,
     PREPEND = 406,
     FIRST = 407,
     LAST = 408,
     MATCH = 409,
     EMPTY = 410,
     WHERE = 411,
     EVAL = 412,
     LOCAL = 413,
     NEIGHBOR = 414,
     AS = 415,
     HOLD = 416,
     CONNECT = 417,
     RETRY = 418,
     KEEPALIVE = 419,
     MULTIHOP = 420,
     STARTUP = 421,
     VIA = 422,
     NEXT = 423,
     HOP = 424,
     SELF = 425,
     DEFAULT = 426,
     PATH = 427,
     METRIC = 428,
     START = 429,
     DELAY = 430,
     FORGET = 431,
     WAIT = 432,
     AFTER = 433,
     BGP_PATH = 434,
     BGP_LOCAL_PREF = 435,
     BGP_MED = 436,
     BGP_ORIGIN = 437,
     BGP_NEXT_HOP = 438,
     BGP_ATOMIC_AGGR = 439,
     BGP_AGGREGATOR = 440,
     BGP_COMMUNITY = 441,
     RR = 442,
     RS = 443,
     CLIENT = 444,
     CLUSTER = 445,
     AS4 = 446,
     ADVERTISE = 447,
     IPV4 = 448,
     CAPABILITIES = 449,
     LIMIT = 450,
     PREFER = 451,
     OLDER = 452,
     MISSING = 453,
     LLADDR = 454,
     DROP = 455,
     IGNORE = 456,
     REFRESH = 457,
     INTERPRET = 458,
     COMMUNITIES = 459,
     BGP_ORIGINATOR_ID = 460,
     BGP_CLUSTER_LIST = 461,
     OSPF = 462,
     AREA = 463,
     OSPF_METRIC1 = 464,
     OSPF_METRIC2 = 465,
     OSPF_TAG = 466,
     OSPF_ROUTER_ID = 467,
     BROADCAST = 468,
     RFC1583COMPAT = 469,
     STUB = 470,
     TICK = 471,
     COST = 472,
     RETRANSMIT = 473,
     HELLO = 474,
     TRANSMIT = 475,
     PRIORITY = 476,
     DEAD = 477,
     NONBROADCAST = 478,
     POINTOPOINT = 479,
     TYPE = 480,
     SIMPLE = 481,
     AUTHENTICATION = 482,
     STRICT = 483,
     CRYPTOGRAPHIC = 484,
     ELIGIBLE = 485,
     POLL = 486,
     NETWORKS = 487,
     HIDDEN = 488,
     VIRTUAL = 489,
     LINK = 490,
     RX = 491,
     BUFFER = 492,
     LARGE = 493,
     NORMAL = 494,
     STUBNET = 495,
     LSADB = 496,
     TOPOLOGY = 497,
     STATE = 498,
     PIPE = 499,
     PEER = 500,
     MODE = 501,
     OPAQUE = 502,
     TRANSPARENT = 503,
     RIP = 504,
     INFINITY = 505,
     PERIOD = 506,
     GARBAGE = 507,
     TIMEOUT = 508,
     MULTICAST = 509,
     QUIET = 510,
     NOLISTEN = 511,
     VERSION1 = 512,
     PLAINTEXT = 513,
     MD5 = 514,
     HONOR = 515,
     NEVER = 516,
     ALWAYS = 517,
     RIP_METRIC = 518,
     RIP_TAG = 519,
     STATIC = 520,
     PROHIBIT = 521
   };
#endif
/* Tokens.  */
#define END 258
#define CLI_MARKER 259
#define INVALID_TOKEN 260
#define GEQ 261
#define LEQ 262
#define NEQ 263
#define AND 264
#define OR 265
#define PO 266
#define PC 267
#define NUM 268
#define ENUM 269
#define RTRID 270
#define IPA 271
#define SYM 272
#define TEXT 273
#define PREFIX_DUMMY 274
#define DEFINE 275
#define ON 276
#define OFF 277
#define YES 278
#define NO 279
#define LOG 280
#define SYSLOG 281
#define ALL 282
#define DEBUG 283
#define TRACE 284
#define INFO 285
#define REMOTE 286
#define WARNING 287
#define ERROR 288
#define AUTH 289
#define FATAL 290
#define BUG 291
#define STDERR 292
#define SOFT 293
#define TIMEFORMAT 294
#define ISO 295
#define SHORT 296
#define LONG 297
#define BASE 298
#define NAME 299
#define CONFIGURE 300
#define DOWN 301
#define KERNEL 302
#define PERSIST 303
#define SCAN 304
#define TIME 305
#define LEARN 306
#define DEVICE 307
#define ROUTES 308
#define ASYNC 309
#define TABLE 310
#define ROUTER 311
#define ID 312
#define PROTOCOL 313
#define PREFERENCE 314
#define DISABLED 315
#define DIRECT 316
#define INTERFACE 317
#define IMPORT 318
#define EXPORT 319
#define FILTER 320
#define NONE 321
#define STATES 322
#define FILTERS 323
#define PASSWORD 324
#define FROM 325
#define PASSIVE 326
#define TO 327
#define EVENTS 328
#define PACKETS 329
#define PROTOCOLS 330
#define INTERFACES 331
#define PRIMARY 332
#define STATS 333
#define COUNT 334
#define FOR 335
#define COMMANDS 336
#define PREEXPORT 337
#define GENERATE 338
#define LISTEN 339
#define BGP 340
#define V6ONLY 341
#define ADDRESS 342
#define PORT 343
#define PASSWORDS 344
#define DESCRIPTION 345
#define RELOAD 346
#define IN 347
#define OUT 348
#define MRTDUMP 349
#define MESSAGES 350
#define RESTRICT 351
#define MEMORY 352
#define SHOW 353
#define STATUS 354
#define SUMMARY 355
#define ROUTE 356
#define SYMBOLS 357
#define DUMP 358
#define RESOURCES 359
#define SOCKETS 360
#define NEIGHBORS 361
#define ATTRIBUTES 362
#define ECHO 363
#define DISABLE 364
#define ENABLE 365
#define RESTART 366
#define FUNCTION 367
#define PRINT 368
#define PRINTN 369
#define UNSET 370
#define RETURN 371
#define ACCEPT 372
#define REJECT 373
#define QUITBIRD 374
#define INT 375
#define BOOL 376
#define IP 377
#define PREFIX 378
#define PAIR 379
#define QUAD 380
#define SET 381
#define STRING 382
#define BGPMASK 383
#define BGPPATH 384
#define CLIST 385
#define IF 386
#define THEN 387
#define ELSE 388
#define CASE 389
#define TRUE 390
#define FALSE 391
#define GW 392
#define NET 393
#define MASK 394
#define PROTO 395
#define SOURCE 396
#define SCOPE 397
#define CAST 398
#define DEST 399
#define LEN 400
#define DEFINED 401
#define ADD 402
#define DELETE 403
#define CONTAINS 404
#define RESET 405
#define PREPEND 406
#define FIRST 407
#define LAST 408
#define MATCH 409
#define EMPTY 410
#define WHERE 411
#define EVAL 412
#define LOCAL 413
#define NEIGHBOR 414
#define AS 415
#define HOLD 416
#define CONNECT 417
#define RETRY 418
#define KEEPALIVE 419
#define MULTIHOP 420
#define STARTUP 421
#define VIA 422
#define NEXT 423
#define HOP 424
#define SELF 425
#define DEFAULT 426
#define PATH 427
#define METRIC 428
#define START 429
#define DELAY 430
#define FORGET 431
#define WAIT 432
#define AFTER 433
#define BGP_PATH 434
#define BGP_LOCAL_PREF 435
#define BGP_MED 436
#define BGP_ORIGIN 437
#define BGP_NEXT_HOP 438
#define BGP_ATOMIC_AGGR 439
#define BGP_AGGREGATOR 440
#define BGP_COMMUNITY 441
#define RR 442
#define RS 443
#define CLIENT 444
#define CLUSTER 445
#define AS4 446
#define ADVERTISE 447
#define IPV4 448
#define CAPABILITIES 449
#define LIMIT 450
#define PREFER 451
#define OLDER 452
#define MISSING 453
#define LLADDR 454
#define DROP 455
#define IGNORE 456
#define REFRESH 457
#define INTERPRET 458
#define COMMUNITIES 459
#define BGP_ORIGINATOR_ID 460
#define BGP_CLUSTER_LIST 461
#define OSPF 462
#define AREA 463
#define OSPF_METRIC1 464
#define OSPF_METRIC2 465
#define OSPF_TAG 466
#define OSPF_ROUTER_ID 467
#define BROADCAST 468
#define RFC1583COMPAT 469
#define STUB 470
#define TICK 471
#define COST 472
#define RETRANSMIT 473
#define HELLO 474
#define TRANSMIT 475
#define PRIORITY 476
#define DEAD 477
#define NONBROADCAST 478
#define POINTOPOINT 479
#define TYPE 480
#define SIMPLE 481
#define AUTHENTICATION 482
#define STRICT 483
#define CRYPTOGRAPHIC 484
#define ELIGIBLE 485
#define POLL 486
#define NETWORKS 487
#define HIDDEN 488
#define VIRTUAL 489
#define LINK 490
#define RX 491
#define BUFFER 492
#define LARGE 493
#define NORMAL 494
#define STUBNET 495
#define LSADB 496
#define TOPOLOGY 497
#define STATE 498
#define PIPE 499
#define PEER 500
#define MODE 501
#define OPAQUE 502
#define TRANSPARENT 503
#define RIP 504
#define INFINITY 505
#define PERIOD 506
#define GARBAGE 507
#define TIMEOUT 508
#define MULTICAST 509
#define QUIET 510
#define NOLISTEN 511
#define VERSION1 512
#define PLAINTEXT 513
#define MD5 514
#define HONOR 515
#define NEVER 516
#define ALWAYS 517
#define RIP_METRIC 518
#define RIP_TAG 519
#define STATIC 520
#define PROHIBIT 521




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

#ifdef OSPFv2
static void
finish_iface_config(struct ospf_iface_patt *ip)
{
  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");
}
#endif

#ifdef OSPFv3
static void
finish_iface_config(struct ospf_iface_patt *ip)
{
  if ((ip->autype != OSPF_AUTH_NONE) || (get_passwords() != NULL))
    cf_error("Authentication not supported in OSPFv3");
}
#endif

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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 157 "cf-parse.y"
{
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
/* Line 187 of yacc.c.  */
#line 812 "cf-parse.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 825 "cf-parse.tab.c"

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
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
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
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1762

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  288
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  199
/* YYNRULES -- Number of rules.  */
#define YYNRULES  607
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1130

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   521

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,     2,    29,     2,     2,
     278,   279,    27,    25,   284,    26,    24,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   281,   280,
      21,    20,    22,   285,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   286,     2,   287,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   282,     2,   283,    23,     2,     2,     2,
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
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277
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
     222,   224,   227,   229,   230,   232,   233,   236,   239,   242,
     245,   248,   251,   254,   258,   261,   264,   266,   268,   270,
     272,   276,   280,   281,   283,   285,   288,   289,   291,   295,
     297,   301,   304,   308,   312,   316,   317,   321,   323,   325,
     329,   331,   335,   337,   339,   341,   343,   345,   347,   349,
     351,   355,   357,   361,   363,   365,   370,   372,   373,   377,
     382,   384,   387,   388,   394,   400,   406,   412,   417,   421,
     425,   430,   436,   438,   439,   443,   448,   453,   454,   457,
     461,   465,   469,   472,   475,   478,   482,   486,   489,   492,
     494,   496,   501,   505,   509,   513,   517,   521,   525,   529,
     534,   536,   538,   540,   541,   543,   547,   551,   555,   559,
     564,   569,   574,   579,   582,   584,   586,   588,   590,   591,
     593,   594,   599,   602,   604,   606,   608,   610,   612,   614,
     616,   618,   620,   622,   625,   628,   629,   633,   635,   639,
     641,   643,   645,   648,   652,   655,   660,   661,   667,   668,
     670,   672,   675,   677,   681,   687,   689,   691,   693,   695,
     697,   699,   705,   707,   712,   714,   718,   722,   724,   727,
     730,   737,   739,   743,   744,   749,   754,   756,   760,   764,
     768,   771,   774,   777,   780,   781,   784,   787,   788,   794,
     796,   798,   800,   802,   804,   806,   808,   812,   816,   818,
     820,   821,   826,   828,   830,   832,   834,   836,   838,   840,
     842,   844,   848,   852,   856,   860,   864,   868,   872,   876,
     880,   884,   888,   892,   896,   900,   903,   908,   910,   912,
     914,   916,   919,   922,   926,   930,   937,   941,   945,   949,
     953,   960,   967,   974,   979,   981,   983,   985,   987,   989,
     991,   993,   994,   996,  1000,  1002,  1006,  1007,  1009,  1014,
    1021,  1026,  1030,  1036,  1042,  1047,  1054,  1058,  1061,  1067,
    1073,  1082,  1091,  1100,  1103,  1107,  1111,  1117,  1124,  1131,
    1136,  1141,  1147,  1154,  1161,  1167,  1174,  1180,  1186,  1192,
    1198,  1204,  1210,  1216,  1222,  1228,  1235,  1242,  1251,  1258,
    1265,  1271,  1276,  1282,  1287,  1293,  1298,  1304,  1307,  1311,
    1315,  1317,  1320,  1323,  1326,  1330,  1333,  1334,  1338,  1342,
    1345,  1350,  1353,  1356,  1358,  1363,  1365,  1367,  1368,  1372,
    1375,  1378,  1381,  1386,  1388,  1389,  1393,  1394,  1397,  1400,
    1404,  1407,  1410,  1414,  1417,  1420,  1423,  1425,  1429,  1432,
    1435,  1438,  1441,  1445,  1448,  1451,  1454,  1458,  1461,  1464,
    1467,  1471,  1474,  1479,  1482,  1485,  1488,  1492,  1496,  1500,
    1502,  1503,  1506,  1508,  1510,  1513,  1517,  1518,  1521,  1523,
    1525,  1528,  1532,  1533,  1534,  1538,  1539,  1543,  1547,  1549,
    1550,  1555,  1562,  1569,  1576,  1584,  1591,  1599,  1606,  1609,
    1613,  1617,  1623,  1628,  1633,  1636,  1640,  1644,  1649,  1654,
    1659,  1665,  1671,  1676,  1680,  1685,  1690,  1695,  1700,  1702,
    1704,  1706,  1708,  1710,  1712,  1714,  1716,  1717,  1720,  1723,
    1724,  1728,  1729,  1733,  1734,  1738,  1741,  1745,  1749,  1753,
    1756,  1760,  1764,  1767,  1770,  1773,  1778,  1780,  1782,  1784,
    1786,  1788,  1790,  1792,  1794,  1796,  1798,  1800,  1802,  1804,
    1806,  1808,  1810,  1812,  1814,  1816,  1818,  1820,  1822,  1824,
    1826,  1828,  1830,  1832,  1834,  1836,  1838,  1840,  1842,  1844,
    1846,  1848,  1850,  1852,  1854,  1856,  1858,  1860,  1862,  1864,
    1866,  1868,  1870,  1872,  1874,  1876,  1878,  1881,  1884,  1887,
    1890,  1893,  1896,  1899,  1902,  1906,  1910,  1914,  1918,  1922,
    1926,  1930,  1932,  1934,  1936,  1938,  1940,  1942,  1944,  1946,
    1948,  1950,  1952,  1954,  1956,  1958,  1960,  1962
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     289,     0,    -1,   290,     3,    -1,     4,   482,    -1,    -1,
     290,   481,    -1,    13,    -1,   278,   419,   279,    -1,    17,
      -1,    31,    17,    20,   291,   280,    -1,    31,    17,    20,
      16,   280,    -1,   291,    -1,    32,    -1,    34,    -1,    33,
      -1,    35,    -1,    -1,    16,    -1,    17,    -1,   294,   297,
      -1,   295,    -1,   294,    -1,    28,   291,    -1,   281,   294,
      -1,    18,    -1,    18,    -1,    -1,    36,   302,   303,   280,
      -1,    55,    18,    -1,    -1,    18,    -1,    37,   301,    -1,
      48,    -1,    38,    -1,   282,   304,   283,    -1,   305,    -1,
     304,   284,   305,    -1,    39,    -1,    40,    -1,    41,    -1,
      42,    -1,    43,    -1,    44,    -1,    45,    -1,    46,    -1,
      47,    -1,   105,    86,   343,   280,    -1,   105,    18,   280,
      -1,   112,    -1,    69,    -1,    54,    -1,    36,    -1,   307,
      18,    -1,   307,    18,   291,    18,    -1,   307,    51,    52,
      -1,   307,    51,    53,    -1,    50,   308,   280,    -1,    56,
     313,     3,    -1,    56,    49,   313,     3,    -1,    57,     3,
      -1,    -1,    18,    -1,   325,    58,    -1,    59,   293,    -1,
      60,    61,   291,    -1,    62,   293,    -1,    63,    64,   293,
      -1,   325,    63,    -1,    60,    61,   291,    -1,    88,   299,
     296,    -1,    58,    66,   291,    -1,    67,    68,   320,   280,
      -1,    13,    -1,    15,    -1,    16,    -1,    95,    96,   322,
     280,    -1,    -1,   322,   323,    -1,    98,   294,    -1,    99,
     291,    -1,    97,    -1,    66,    17,    -1,    69,    -1,    -1,
      17,    -1,    -1,    70,   291,    -1,    71,   293,    -1,    39,
     340,    -1,   105,   343,    -1,    74,   328,    -1,    75,   328,
      -1,    66,   329,    -1,    67,    68,   320,    -1,   101,    18,
      -1,    76,   391,    -1,   392,    -1,    38,    -1,    77,    -1,
      17,    -1,    39,    86,   340,    -1,    39,    92,   291,    -1,
      -1,    18,    -1,   296,    -1,    18,   296,    -1,    -1,    26,
      -1,   331,   333,   332,    -1,   334,    -1,   335,   284,   334,
      -1,   325,    72,    -1,   336,   326,   282,    -1,   337,   327,
     280,    -1,   337,   339,   280,    -1,    -1,    73,   338,   335,
      -1,    38,    -1,    33,    -1,   282,   341,   283,    -1,   342,
      -1,   341,   284,   342,    -1,    78,    -1,    64,    -1,    79,
      -1,    87,    -1,    84,    -1,    85,    -1,    38,    -1,    33,
      -1,   282,   344,   283,    -1,   345,    -1,   344,   284,   345,
      -1,    78,    -1,   106,    -1,   100,   282,   347,   283,    -1,
     348,    -1,    -1,   348,   280,   347,    -1,   349,   282,   350,
     283,    -1,   349,    -1,    80,    18,    -1,    -1,    94,    81,
     298,   280,   350,    -1,    94,    83,   298,   280,   350,    -1,
     128,    81,   298,   280,   350,    -1,   128,    83,   298,   280,
     350,    -1,    68,   291,   280,   350,    -1,   109,   110,     3,
      -1,   109,   108,     3,    -1,   109,    86,   382,     3,    -1,
     109,    86,    38,   382,     3,    -1,    17,    -1,    -1,   109,
      87,     3,    -1,   109,    87,   111,     3,    -1,   109,   112,
     359,     3,    -1,    -1,   359,   295,    -1,   359,    91,   296,
      -1,   359,    66,    17,    -1,   359,    76,   391,    -1,   359,
     392,    -1,   359,    38,    -1,   359,    88,    -1,   359,   360,
      17,    -1,   359,    69,    17,    -1,   359,    89,    -1,   359,
      90,    -1,    93,    -1,    75,    -1,   109,   113,   355,     3,
      -1,   114,   115,     3,    -1,   114,   116,     3,    -1,   114,
      87,     3,    -1,   114,   117,     3,    -1,   114,   118,     3,
      -1,   114,    64,     3,    -1,   114,    86,     3,    -1,   119,
     370,   371,     3,    -1,    38,    -1,    33,    -1,    13,    -1,
      -1,    13,    -1,   120,   381,     3,    -1,   121,   381,     3,
      -1,   122,   381,     3,    -1,   102,   381,     3,    -1,   102,
     103,   381,     3,    -1,   102,   104,   381,     3,    -1,    39,
     381,   340,     3,    -1,   105,   381,   343,     3,    -1,   107,
       3,    -1,    17,    -1,    38,    -1,    18,    -1,    17,    -1,
      -1,    18,    -1,    -1,    76,    17,   384,   390,    -1,   168,
     419,    -1,   131,    -1,   132,    -1,   133,    -1,   134,    -1,
     135,    -1,   136,    -1,   138,    -1,   139,    -1,   140,    -1,
     141,    -1,   386,   137,    -1,   386,    17,    -1,    -1,   387,
     280,   388,    -1,   387,    -1,   389,   280,   387,    -1,   394,
      -1,    17,    -1,   390,    -1,   167,   419,    -1,   278,   389,
     279,    -1,   278,   279,    -1,   388,   282,   397,   283,    -1,
      -1,   123,    17,   396,   393,   394,    -1,    -1,   398,    -1,
     425,    -1,   398,   425,    -1,   425,    -1,   282,   397,   283,
      -1,   278,    13,   284,    13,   279,    -1,    16,    -1,    13,
      -1,    15,    -1,   400,    -1,   401,    -1,    14,    -1,   278,
      13,   284,    27,   279,    -1,   402,    -1,   402,    24,    24,
     402,    -1,   403,    -1,   404,   284,   403,    -1,    16,    28,
      13,    -1,   405,    -1,   405,    25,    -1,   405,    26,    -1,
     405,   282,    13,   284,    13,   283,    -1,   406,    -1,   407,
     284,   406,    -1,    -1,   408,   403,   281,   397,    -1,   408,
     144,   281,   397,    -1,   417,    -1,   278,   419,   279,    -1,
      11,   411,    12,    -1,    28,   412,    28,    -1,    13,   411,
      -1,    27,   411,    -1,   285,   411,    -1,   409,   411,    -1,
      -1,    13,   412,    -1,   285,   412,    -1,    -1,   278,   419,
     284,   419,   279,    -1,    13,    -1,   146,    -1,   147,    -1,
      18,    -1,   401,    -1,   405,    -1,    15,    -1,   286,   404,
     287,    -1,   286,   407,   287,    -1,    14,    -1,   410,    -1,
      -1,    17,   278,   424,   279,    -1,    17,    -1,    81,    -1,
     148,    -1,   149,    -1,   151,    -1,   152,    -1,   153,    -1,
     154,    -1,   155,    -1,   278,   419,   279,    -1,   419,    25,
     419,    -1,   419,    26,   419,    -1,   419,    27,   419,    -1,
     419,    28,   419,    -1,   419,     9,   419,    -1,   419,    10,
     419,    -1,   419,    20,   419,    -1,   419,     8,   419,    -1,
     419,    21,   419,    -1,   419,     7,   419,    -1,   419,    22,
     419,    -1,   419,     6,   419,    -1,   419,    23,   419,    -1,
      30,   419,    -1,   157,   278,   419,   279,    -1,   417,    -1,
     414,    -1,   413,    -1,    70,    -1,   415,   418,    -1,   415,
     486,    -1,   419,    24,   133,    -1,   419,    24,   156,    -1,
     419,    24,   150,   278,   419,   279,    -1,   419,    24,   163,
      -1,   419,    24,   164,    -1,    25,   166,    25,    -1,    26,
     166,    26,    -1,   162,   278,   419,   284,   419,   279,    -1,
     158,   278,   419,   284,   419,   279,    -1,   159,   278,   419,
     284,   419,   279,    -1,    17,   278,   424,   279,    -1,   130,
      -1,   128,    -1,   129,    -1,    44,    -1,   124,    -1,   125,
      -1,   419,    -1,    -1,   421,    -1,   421,   284,   422,    -1,
     419,    -1,   419,   284,   423,    -1,    -1,   423,    -1,   142,
     419,   143,   399,    -1,   142,   419,   143,   399,   144,   399,
      -1,    17,    20,   419,   280,    -1,   127,   419,   280,    -1,
     415,   486,    20,   419,   280,    -1,   415,   418,    20,   419,
     280,    -1,    70,    20,   419,   280,    -1,   126,   278,   415,
     486,   279,   280,    -1,   420,   422,   280,    -1,   416,   280,
      -1,   145,   419,   282,   408,   283,    -1,   415,   486,    24,
     166,   280,    -1,   415,   486,    24,   162,   278,   419,   279,
     280,    -1,   415,   486,    24,   158,   278,   419,   279,   280,
      -1,   415,   486,    24,   159,   278,   419,   279,   280,    -1,
     325,    96,    -1,   426,   326,   282,    -1,   427,   327,   280,
      -1,   427,   169,   171,   291,   280,    -1,   427,   170,   294,
     171,   291,   280,    -1,   427,   198,   201,    68,   320,   280,
      -1,   427,   198,   200,   280,    -1,   427,   199,   200,   280,
      -1,   427,   172,    61,   291,   280,    -1,   427,   177,   172,
      61,   291,   280,    -1,   427,   173,   174,    61,   291,   280,
      -1,   427,   175,    61,   291,   280,    -1,   427,   176,   291,
     178,   294,   280,    -1,   427,   179,   180,   181,   280,    -1,
     427,   209,   210,   181,   280,    -1,   427,   209,   210,   211,
     280,    -1,   427,   209,   210,   212,   280,    -1,   427,   183,
     184,   293,   280,    -1,   427,   207,   208,   293,   280,    -1,
     427,   182,   192,   291,   280,    -1,   427,   182,   191,   291,
     280,    -1,   427,   152,    98,   294,   280,    -1,   427,   185,
     186,    61,   291,   280,    -1,   427,    44,   187,    61,   291,
     280,    -1,   427,    44,   188,    61,   291,   284,   291,   280,
      -1,   427,   120,   189,    44,   293,   280,    -1,   427,   121,
     112,   213,   293,   280,    -1,   427,   121,   202,   293,   280,
      -1,   427,   205,   293,   280,    -1,   427,   203,   204,   293,
     280,    -1,   427,    80,    18,   280,    -1,   427,   112,   206,
     291,   280,    -1,   427,    82,   293,   280,    -1,   427,   214,
     215,   293,   280,    -1,   325,   218,    -1,   428,   326,   282,
      -1,   429,   430,   280,    -1,   327,    -1,   225,   293,    -1,
     227,   291,    -1,   432,   283,    -1,   219,   320,   282,    -1,
     431,   433,    -1,    -1,   433,   434,   280,    -1,   226,   228,
     291,    -1,   226,   293,    -1,   243,   282,   444,   283,    -1,
     251,   435,    -1,    73,   455,    -1,   439,    -1,   436,   282,
     437,   283,    -1,   436,    -1,   295,    -1,    -1,   437,   438,
     280,    -1,   244,   293,    -1,   111,   293,    -1,   228,   291,
      -1,   442,   282,   440,   283,    -1,   442,    -1,    -1,   440,
     441,   280,    -1,    -1,   230,   291,    -1,   229,   291,    -1,
     231,   186,   291,    -1,   188,   291,    -1,   233,   291,    -1,
     233,    90,   291,    -1,   238,    77,    -1,   238,   237,    -1,
     238,   240,    -1,   346,    -1,   245,   246,   320,    -1,   228,
     291,    -1,   230,   291,    -1,   242,   291,    -1,   229,   291,
      -1,   231,   186,   291,    -1,   232,   291,    -1,   188,   291,
      -1,   233,   291,    -1,   233,    90,   291,    -1,   236,   224,
      -1,   236,   234,    -1,   236,   235,    -1,   239,   234,   293,
      -1,   226,   293,    -1,   117,   282,   448,   283,    -1,   238,
      77,    -1,   238,   237,    -1,   238,   240,    -1,   247,   248,
     249,    -1,   247,   248,   250,    -1,   247,   248,   291,    -1,
     346,    -1,    -1,   444,   445,    -1,   446,    -1,   447,    -1,
     295,   280,    -1,   295,   244,   280,    -1,    -1,   448,   449,
      -1,   450,    -1,   451,    -1,    16,   280,    -1,    16,   241,
     280,    -1,    -1,    -1,   453,   443,   280,    -1,    -1,   282,
     453,   283,    -1,   452,   335,   454,    -1,    18,    -1,    -1,
     109,   218,   355,     3,    -1,   109,   218,   117,   355,   456,
       3,    -1,   109,   218,    73,   355,   456,     3,    -1,   109,
     218,   253,   355,   456,     3,    -1,   109,   218,   253,    38,
     355,   456,     3,    -1,   109,   218,   254,   355,   456,     3,
      -1,   109,   218,   254,    38,   355,   456,     3,    -1,   109,
     218,   252,   355,   456,     3,    -1,   325,   255,    -1,   465,
     326,   282,    -1,   466,   327,   280,    -1,   466,   256,    66,
      17,   280,    -1,   466,   257,   258,   280,    -1,   466,   257,
     259,   280,    -1,   325,   260,    -1,   467,   326,   282,    -1,
     468,   327,   280,    -1,   468,   261,   291,   280,    -1,   468,
      99,   291,   280,    -1,   468,   262,   291,   280,    -1,   468,
     263,    61,   291,   280,    -1,   468,   264,    61,   291,   280,
      -1,   468,   238,   469,   280,    -1,   468,   346,   280,    -1,
     468,   271,   273,   280,    -1,   468,   271,   170,   280,    -1,
     468,   271,   272,   280,    -1,   468,    73,   475,   280,    -1,
     269,    -1,   270,    -1,    77,    -1,   224,    -1,   265,    -1,
     266,    -1,   267,    -1,   268,    -1,    -1,   184,   291,    -1,
     257,   470,    -1,    -1,   472,   471,   280,    -1,    -1,   282,
     472,   283,    -1,    -1,   474,   335,   473,    -1,   325,   276,
      -1,   476,   326,   282,    -1,   477,   327,   280,    -1,   477,
     479,   280,    -1,   112,   295,    -1,   478,   178,   294,    -1,
     478,   178,    18,    -1,   478,   211,    -1,   478,   129,    -1,
     478,   277,    -1,   109,   276,   355,     3,    -1,   280,    -1,
     292,    -1,   300,    -1,   306,    -1,   309,    -1,   319,    -1,
     321,    -1,   324,    -1,   483,    -1,   330,    -1,   383,    -1,
     385,    -1,   395,    -1,   310,    -1,   311,    -1,   312,    -1,
     351,    -1,   352,    -1,   353,    -1,   354,    -1,   356,    -1,
     357,    -1,   358,    -1,   361,    -1,   362,    -1,   363,    -1,
     364,    -1,   365,    -1,   366,    -1,   367,    -1,   368,    -1,
     369,    -1,   372,    -1,   373,    -1,   374,    -1,   375,    -1,
     376,    -1,   377,    -1,   378,    -1,   379,    -1,   380,    -1,
     457,    -1,   458,    -1,   459,    -1,   460,    -1,   461,    -1,
     462,    -1,   463,    -1,   464,    -1,   480,    -1,   484,   283,
      -1,   485,   283,    -1,   337,   283,    -1,   427,   283,    -1,
     429,   283,    -1,   466,   283,    -1,   468,   283,    -1,   477,
     283,    -1,   314,   326,   282,    -1,   484,   327,   280,    -1,
     484,   315,   280,    -1,   484,   318,   280,    -1,   316,   326,
     282,    -1,   485,   327,   280,    -1,   485,   317,   280,    -1,
       5,    -1,   193,    -1,   190,    -1,   194,    -1,   192,    -1,
     191,    -1,   195,    -1,   196,    -1,   197,    -1,   216,    -1,
     217,    -1,   220,    -1,   221,    -1,   222,    -1,   223,    -1,
     274,    -1,   275,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   312,   312,   313,   316,   318,   325,   326,   327,   331,
     335,   344,   345,   346,   347,   348,   349,   355,   356,   363,
     370,   371,   375,   379,   386,   394,   395,   401,   410,   411,
     415,   420,   421,   425,   426,   430,   431,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   449,   450,   459,   460,
     461,   462,   465,   466,   467,   468,   472,   479,   482,   485,
     489,   490,   498,   514,   515,   519,   526,   532,   545,   549,
     562,   574,   580,   581,   582,   593,   595,   597,   601,   602,
     603,   610,   618,   622,   628,   634,   636,   640,   641,   642,
     643,   644,   645,   646,   647,   651,   652,   653,   654,   658,
     666,   667,   675,   683,   684,   685,   689,   690,   694,   699,
     700,   707,   716,   717,   718,   722,   731,   737,   738,   739,
     743,   744,   748,   749,   750,   751,   752,   753,   759,   760,
     761,   765,   766,   770,   771,   777,   778,   781,   783,   787,
     788,   792,   810,   811,   812,   813,   814,   815,   823,   826,
     829,   832,   836,   837,   840,   843,   846,   850,   856,   862,
     869,   874,   879,   884,   888,   892,   902,   910,   914,   921,
     922,   925,   929,   931,   933,   935,   937,   939,   941,   944,
     950,   951,   952,   956,   957,   963,   965,   967,   969,   971,
     973,   977,   981,   984,   988,   989,   990,   994,   995,   996,
    1003,  1003,  1012,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1046,  1057,  1058,  1065,  1066,  1073,
    1082,  1086,  1090,  1114,  1115,  1119,  1132,  1132,  1148,  1149,
    1152,  1153,  1157,  1160,  1169,  1176,  1180,  1181,  1182,  1183,
    1184,  1188,  1194,  1199,  1207,  1208,  1212,  1219,  1220,  1221,
    1222,  1229,  1230,  1233,  1234,  1239,  1251,  1252,  1256,  1257,
    1261,  1262,  1263,  1264,  1265,  1269,  1270,  1271,  1275,  1288,
    1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,  1297,  1298,
    1308,  1312,  1335,  1369,  1371,  1372,  1373,  1374,  1375,  1376,
    1377,  1381,  1382,  1383,  1384,  1385,  1386,  1387,  1388,  1389,
    1390,  1391,  1392,  1393,  1394,  1395,  1396,  1398,  1399,  1400,
    1402,  1404,  1406,  1408,  1409,  1410,  1411,  1412,  1422,  1423,
    1424,  1425,  1426,  1431,  1454,  1455,  1456,  1457,  1458,  1459,
    1463,  1466,  1467,  1468,  1477,  1484,  1493,  1494,  1498,  1504,
    1514,  1523,  1529,  1534,  1541,  1546,  1552,  1553,  1554,  1562,
    1564,  1565,  1566,  1572,  1593,  1594,  1595,  1596,  1602,  1603,
    1604,  1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,  1613,
    1614,  1615,  1616,  1617,  1618,  1619,  1620,  1621,  1622,  1623,
    1624,  1625,  1626,  1627,  1628,  1629,  1630,  1640,  1650,  1651,
    1655,  1656,  1657,  1658,  1661,  1673,  1676,  1678,  1682,  1683,
    1684,  1685,  1686,  1687,  1691,  1692,  1696,  1704,  1706,  1710,
    1711,  1712,  1716,  1717,  1720,  1722,  1725,  1726,  1727,  1728,
    1729,  1730,  1731,  1732,  1733,  1734,  1735,  1738,  1759,  1760,
    1761,  1762,  1763,  1764,  1765,  1766,  1767,  1768,  1769,  1770,
    1771,  1772,  1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,
    1783,  1785,  1789,  1790,  1792,  1801,  1811,  1813,  1817,  1818,
    1820,  1829,  1840,  1862,  1864,  1867,  1869,  1873,  1877,  1878,
    1882,  1885,  1888,  1893,  1896,  1901,  1904,  1907,  1913,  1921,
    1922,  1923,  1928,  1929,  1935,  1942,  1943,  1944,  1945,  1946,
    1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1958,  1959,
    1960,  1965,  1966,  1967,  1968,  1969,  1972,  1973,  1974,  1977,
    1979,  1982,  1984,  1988,  1997,  2004,  2011,  2012,  2013,  2016,
    2025,  2029,  2035,  2036,  2037,  2040,  2047,  2047,  2047,  2047,
    2047,  2047,  2047,  2047,  2047,  2047,  2047,  2047,  2047,  2048,
    2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,
    2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,
    2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,
    2048,  2048,  2048,  2048,  2048,  2048,  2049,  2049,  2049,  2049,
    2049,  2049,  2049,  2049,  2050,  2050,  2050,  2050,  2051,  2051,
    2051,  2052,  2052,  2053,  2054,  2055,  2056,  2057,  2058,  2059,
    2060,  2061,  2062,  2062,  2062,  2062,  2062,  2062
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
  "TIMEFORMAT", "ISO", "SHORT", "LONG", "BASE", "NAME", "CONFIGURE",
  "DOWN", "KERNEL", "PERSIST", "SCAN", "TIME", "LEARN", "DEVICE", "ROUTES",
  "ASYNC", "TABLE", "ROUTER", "ID", "PROTOCOL", "PREFERENCE", "DISABLED",
  "DIRECT", "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE", "STATES",
  "FILTERS", "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS",
  "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR",
  "COMMANDS", "PREEXPORT", "GENERATE", "LISTEN", "BGP", "V6ONLY",
  "ADDRESS", "PORT", "PASSWORDS", "DESCRIPTION", "RELOAD", "IN", "OUT",
  "MRTDUMP", "MESSAGES", "RESTRICT", "MEMORY", "SHOW", "STATUS", "SUMMARY",
  "ROUTE", "SYMBOLS", "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS",
  "ATTRIBUTES", "ECHO", "DISABLE", "ENABLE", "RESTART", "FUNCTION",
  "PRINT", "PRINTN", "UNSET", "RETURN", "ACCEPT", "REJECT", "QUITBIRD",
  "INT", "BOOL", "IP", "PREFIX", "PAIR", "QUAD", "SET", "STRING",
  "BGPMASK", "BGPPATH", "CLIST", "IF", "THEN", "ELSE", "CASE", "TRUE",
  "FALSE", "GW", "NET", "MASK", "PROTO", "SOURCE", "SCOPE", "CAST", "DEST",
  "LEN", "DEFINED", "ADD", "DELETE", "CONTAINS", "RESET", "PREPEND",
  "FIRST", "LAST", "MATCH", "EMPTY", "WHERE", "EVAL", "LOCAL", "NEIGHBOR",
  "AS", "HOLD", "CONNECT", "RETRY", "KEEPALIVE", "MULTIHOP", "STARTUP",
  "VIA", "NEXT", "HOP", "SELF", "DEFAULT", "PATH", "METRIC", "START",
  "DELAY", "FORGET", "WAIT", "AFTER", "BGP_PATH", "BGP_LOCAL_PREF",
  "BGP_MED", "BGP_ORIGIN", "BGP_NEXT_HOP", "BGP_ATOMIC_AGGR",
  "BGP_AGGREGATOR", "BGP_COMMUNITY", "RR", "RS", "CLIENT", "CLUSTER",
  "AS4", "ADVERTISE", "IPV4", "CAPABILITIES", "LIMIT", "PREFER", "OLDER",
  "MISSING", "LLADDR", "DROP", "IGNORE", "REFRESH", "INTERPRET",
  "COMMUNITIES", "BGP_ORIGINATOR_ID", "BGP_CLUSTER_LIST", "OSPF", "AREA",
  "OSPF_METRIC1", "OSPF_METRIC2", "OSPF_TAG", "OSPF_ROUTER_ID",
  "BROADCAST", "RFC1583COMPAT", "STUB", "TICK", "COST", "RETRANSMIT",
  "HELLO", "TRANSMIT", "PRIORITY", "DEAD", "NONBROADCAST", "POINTOPOINT",
  "TYPE", "SIMPLE", "AUTHENTICATION", "STRICT", "CRYPTOGRAPHIC",
  "ELIGIBLE", "POLL", "NETWORKS", "HIDDEN", "VIRTUAL", "LINK", "RX",
  "BUFFER", "LARGE", "NORMAL", "STUBNET", "LSADB", "TOPOLOGY", "STATE",
  "PIPE", "PEER", "MODE", "OPAQUE", "TRANSPARENT", "RIP", "INFINITY",
  "PERIOD", "GARBAGE", "TIMEOUT", "MULTICAST", "QUIET", "NOLISTEN",
  "VERSION1", "PLAINTEXT", "MD5", "HONOR", "NEVER", "ALWAYS", "RIP_METRIC",
  "RIP_TAG", "STATIC", "PROHIBIT", "'('", "')'", "';'", "':'", "'{'",
  "'}'", "','", "'?'", "'['", "']'", "$accept", "config", "conf_entries",
  "expr", "definition", "bool", "ipa", "prefix", "prefix_or_ipa", "pxlen",
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
  "cmd_RESTRICT", "proto_patt", "proto_patt2", "filter_def", "@1",
  "filter_eval", "type", "one_decl", "decls", "declsn", "filter_body",
  "filter", "where_filter", "function_params", "function_body",
  "function_def", "@2", "cmds", "cmds_int", "block", "cpair", "fipa",
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
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_TOPOLOGY_ALL",
  "cmd_SHOW_OSPF_STATE", "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB",
  "pipe_proto_start", "pipe_proto", "rip_cfg_start", "rip_cfg", "rip_auth",
  "rip_mode", "rip_iface_item", "rip_iface_opts", "rip_iface_opt_list",
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
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,    40,    41,
      59,    58,   123,   125,    44,    63,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   288,   289,   289,   290,   290,   291,   291,   291,   292,
     292,   293,   293,   293,   293,   293,   293,   294,   294,   295,
     296,   296,   297,   297,   298,   299,   299,   300,   301,   301,
     302,   302,   302,   303,   303,   304,   304,   305,   305,   305,
     305,   305,   305,   305,   305,   305,   306,   306,   307,   307,
     307,   307,   308,   308,   308,   308,   309,   310,   311,   312,
     313,   313,   314,   315,   315,   315,   315,   316,   317,   317,
     318,   319,   320,   320,   320,   321,   322,   322,   323,   323,
     323,   324,   325,   326,   326,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   328,   328,   328,   328,   329,
     330,   330,   331,   332,   332,   332,   333,   333,   334,   335,
     335,   336,   337,   337,   337,   338,   339,   340,   340,   340,
     341,   341,   342,   342,   342,   342,   342,   342,   343,   343,
     343,   344,   344,   345,   345,   346,   346,   347,   347,   348,
     348,   349,   350,   350,   350,   350,   350,   350,   351,   352,
     353,   354,   355,   355,   356,   357,   358,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   360,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   370,   370,   371,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   381,   381,   382,   382,   382,
     384,   383,   385,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   387,   388,   388,   389,   389,   390,
     391,   391,   392,   393,   393,   394,   396,   395,   397,   397,
     398,   398,   399,   399,   400,   401,   402,   402,   402,   402,
     402,   403,   403,   403,   404,   404,   405,   406,   406,   406,
     406,   407,   407,   408,   408,   408,   409,   409,   410,   410,
     411,   411,   411,   411,   411,   412,   412,   412,   413,   414,
     414,   414,   414,   414,   414,   414,   414,   414,   414,   414,
     415,   416,   417,   418,   418,   418,   418,   418,   418,   418,
     418,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   420,   420,   420,   420,   420,   420,
     421,   422,   422,   422,   423,   423,   424,   424,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   426,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   428,   429,   429,
     430,   430,   430,   430,   431,   432,   433,   433,   434,   434,
     434,   434,   434,   434,   435,   435,   436,   437,   437,   438,
     438,   438,   439,   439,   440,   440,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   442,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     444,   444,   445,   445,   446,   447,   448,   448,   449,   449,
     450,   451,   452,   453,   453,   454,   454,   455,   456,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     466,   466,   466,   466,   467,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   468,   468,   468,   468,   469,   469,
     469,   470,   470,   470,   470,   470,   471,   471,   471,   472,
     472,   473,   473,   474,   475,   476,   477,   477,   477,   478,
     479,   479,   479,   479,   479,   480,   481,   481,   481,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   482,
     482,   482,   482,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   483,   483,   483,   483,
     483,   483,   483,   483,   484,   484,   484,   484,   485,   485,
     485,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486
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
       1,     2,     1,     0,     1,     0,     2,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     1,     1,     1,     1,
       3,     3,     0,     1,     1,     2,     0,     1,     3,     1,
       3,     2,     3,     3,     3,     0,     3,     1,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     4,     1,     0,     3,     4,
       1,     2,     0,     5,     5,     5,     5,     4,     3,     3,
       4,     5,     1,     0,     3,     4,     4,     0,     2,     3,
       3,     3,     2,     2,     2,     3,     3,     2,     2,     1,
       1,     4,     3,     3,     3,     3,     3,     3,     3,     4,
       1,     1,     1,     0,     1,     3,     3,     3,     3,     4,
       4,     4,     4,     2,     1,     1,     1,     1,     0,     1,
       0,     4,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     3,     1,     3,     1,
       1,     1,     2,     3,     2,     4,     0,     5,     0,     1,
       1,     2,     1,     3,     5,     1,     1,     1,     1,     1,
       1,     5,     1,     4,     1,     3,     3,     1,     2,     2,
       6,     1,     3,     0,     4,     4,     1,     3,     3,     3,
       2,     2,     2,     2,     0,     2,     2,     0,     5,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       0,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     4,     1,     1,     1,
       1,     2,     2,     3,     3,     6,     3,     3,     3,     3,
       6,     6,     6,     4,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     3,     1,     3,     0,     1,     4,     6,
       4,     3,     5,     5,     4,     6,     3,     2,     5,     5,
       8,     8,     8,     2,     3,     3,     5,     6,     6,     4,
       4,     5,     6,     6,     5,     6,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     6,     6,     8,     6,     6,
       5,     4,     5,     4,     5,     4,     5,     2,     3,     3,
       1,     2,     2,     2,     3,     2,     0,     3,     3,     2,
       4,     2,     2,     1,     4,     1,     1,     0,     3,     2,
       2,     2,     4,     1,     0,     3,     0,     2,     2,     3,
       2,     2,     3,     2,     2,     2,     1,     3,     2,     2,
       2,     2,     3,     2,     2,     2,     3,     2,     2,     2,
       3,     2,     4,     2,     2,     2,     3,     3,     3,     1,
       0,     2,     1,     1,     2,     3,     0,     2,     1,     1,
       2,     3,     0,     0,     3,     0,     3,     3,     1,     0,
       4,     6,     6,     6,     7,     6,     7,     6,     2,     3,
       3,     5,     4,     4,     2,     3,     3,     4,     4,     4,
       5,     5,     4,     3,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     2,     2,     0,
       3,     0,     3,     0,     3,     2,     3,     3,     3,     2,
       3,     3,     2,     2,     2,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   539,   540,   541,   542,
     543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,   564,   565,   566,   567,   568,   569,   570,   571,   572,
     573,   574,   575,     3,     1,     2,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   280,   526,   527,
     528,   529,   530,    83,    83,   531,   532,   533,     0,   535,
      83,    85,   536,   537,   538,    83,    85,    83,    85,    83,
      85,    83,    85,    83,    85,     5,   534,    85,    85,   194,
     196,   195,     0,    61,    60,     0,    59,     0,     0,     0,
       0,   193,   198,     0,     0,     0,   157,   153,   153,   153,
       0,     0,     0,     0,     0,     0,     0,   182,   181,   180,
     183,     0,     0,     0,     0,    30,    29,    32,     0,     0,
       0,    51,    50,    49,    48,     0,     0,    81,     0,   200,
      76,     0,     0,   226,   264,   269,   278,   275,   235,   282,
     272,     0,     0,   267,   280,   310,   270,   271,     0,     0,
       0,     0,   280,     0,   273,   274,   279,   309,   308,     0,
     307,   202,    84,     0,     0,    62,    67,   111,   353,   387,
     478,   484,   515,     0,     0,     0,     0,     0,    16,   115,
       0,     0,     0,     0,   578,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    16,
       0,     0,     0,   579,     0,     0,     0,    16,     0,   580,
     390,     0,   396,     0,     0,     0,     0,   581,     0,     0,
     513,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     582,     0,     0,   136,   140,     0,     0,   583,     0,     0,
       0,     0,    16,     0,    16,     0,   576,     0,     0,     0,
       0,    26,   577,     0,     0,   118,   117,     0,     0,     0,
      57,     0,     0,   188,   129,   128,     0,     0,   197,   199,
     198,     0,   154,     0,   149,   148,     0,   152,     0,   153,
     153,   153,   153,   153,     0,     0,   177,   178,   174,   172,
     173,   175,   176,   184,     0,   185,   186,   187,     0,     0,
      31,    33,     0,     0,   100,     6,     8,   280,   101,    52,
       0,    56,    72,    73,    74,     0,   215,     0,    47,     0,
       0,   264,   282,   264,   280,   264,   264,     0,   256,     0,
     280,     0,     0,   267,   267,     0,   305,   280,   280,   280,
     280,     0,   236,   240,   237,     0,   238,   239,   242,   244,
       0,   247,   251,     0,   591,   283,   284,   285,   286,   287,
     288,   289,   290,   593,   596,   595,   592,   594,   597,   598,
     599,   600,   601,   602,   603,   604,   605,   606,   607,   311,
     312,   280,   280,   280,   280,   280,   280,   280,   280,   280,
       0,   280,   280,   280,   280,   584,   588,   112,    88,    99,
      92,     0,    86,    12,    14,    13,    15,    11,    87,   102,
      97,   215,    98,   280,    90,    96,    91,    94,    89,   113,
     114,   354,     0,     0,     0,     0,     0,     0,     0,    16,
       0,     0,    17,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    16,     0,     0,     0,     0,    16,     0,
      16,     0,    16,   355,   388,     0,   391,   392,   389,   395,
     393,   479,     0,     0,     0,   480,   485,   102,     0,   141,
       0,   137,   500,   498,   499,     0,     0,     0,     0,     0,
       0,     0,     0,   486,   493,   142,   516,     0,   519,   517,
     523,     0,   522,   524,   518,     0,    63,     0,    65,    16,
     586,   587,   585,     0,    25,     0,   590,   589,   123,   122,
     124,   126,   127,   125,     0,   120,   191,    58,   189,   190,
     133,   134,     0,   131,   192,     0,   150,   155,   156,   163,
       0,     0,   170,   215,   164,   167,   168,     0,   169,   158,
       0,   162,   171,   469,   469,   469,   153,   469,   153,   469,
     470,   525,   179,     0,     0,    28,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,    35,    27,     0,     0,
      54,    55,    71,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,     0,     0,     0,   201,   219,    80,     0,
       0,    75,    77,    46,     0,   215,   260,   261,     0,   262,
     263,   258,   246,   334,   337,     0,   318,   319,   265,   266,
     259,     0,     0,     0,     0,   291,   280,     0,     0,     0,
     276,   248,   249,     0,     0,   277,   303,   301,   299,   296,
     297,   298,   300,   302,   304,   313,     0,   314,   316,   317,
     292,   293,   294,   295,    93,   106,   109,   116,   220,   221,
      95,   222,     0,     0,   383,   385,     0,    16,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,     0,   360,     0,   381,     0,     0,
       0,     0,     0,   394,   462,    16,     0,     0,     0,     0,
     403,   413,     0,   482,   483,   511,   497,   488,     0,     0,
     492,   487,   489,     0,     0,   495,   496,   494,     0,     0,
       0,     0,     0,     0,    19,   521,   520,    70,    64,    66,
      68,    21,    20,    69,   119,     0,   130,     0,   151,   160,
     166,   161,   159,   165,   468,     0,     0,     0,   469,     0,
     469,     0,    10,     9,    34,     0,     7,    53,   214,   213,
     215,   280,    78,    79,   224,   217,     0,   227,   257,   280,
     323,   306,   280,   280,   280,     0,     0,     0,   235,   245,
       0,     0,   252,   280,   107,     0,   102,     0,     0,   384,
       0,     0,   380,   374,   356,     0,   361,     0,   364,     0,
       0,   366,   373,   372,   370,     0,     0,   382,   371,   367,
     368,   369,   386,   102,   402,     0,   399,   450,     0,   406,
     401,   405,   397,   414,   481,   509,   514,   135,   137,   490,
     491,     0,     0,     0,     0,     0,   139,    22,    23,   121,
     132,   472,   471,   477,     0,   473,     0,   475,    36,   216,
       0,   327,     0,   328,   329,     0,   280,   325,   326,   324,
     280,   280,     0,   280,     0,     0,   280,   230,   223,     0,
     335,     0,     0,     0,   268,     0,     0,     0,   243,     0,
       0,   103,   104,   108,   110,   376,     0,   378,   379,   357,
     363,   365,   362,   375,   358,   465,   398,     0,   427,   407,
     416,   506,   138,   142,    24,     0,     0,     0,     0,   474,
     476,   280,   280,   280,   280,     0,     0,     0,   225,   231,
       0,     0,   347,   330,   332,     0,   218,   321,   322,   320,
     234,   241,     0,     0,   315,   105,     0,   463,   467,   400,
       0,   451,   452,   453,     0,     0,     0,     0,     0,     0,
       0,   412,   426,     0,     0,     0,   512,     0,   147,   142,
     142,   142,   142,     0,     0,     0,     0,   341,   280,   253,
     280,   280,     0,   280,   346,     0,   250,   377,     0,     0,
     454,    16,     0,    16,   404,     0,   420,   418,   417,     0,
       0,   421,   423,   424,   425,   415,   507,   501,   502,   503,
     504,   505,   508,   510,   143,   144,   145,   146,   340,   281,
     344,     0,   280,   338,   232,     0,     0,     0,     0,     0,
       0,     0,   333,     0,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   466,   449,     0,
     455,   410,   411,   409,   408,   419,   422,     0,     0,   280,
       0,   348,     0,   343,   342,   280,   280,   280,   349,   456,
     434,   441,   428,   431,   429,     0,   433,     0,   435,   437,
     438,   439,   443,   444,   445,    16,   430,     0,   464,   345,
     233,   339,   280,   280,     0,     0,     0,     0,   432,   436,
     440,   446,   447,   448,   255,   254,     0,     0,     0,     0,
     442,   457,   458,   459,   351,   352,   350,     0,   460,   461
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   437,    69,   438,   517,   752,   753,   744,
     925,   535,    70,   330,   138,   333,   595,   596,    71,   145,
     146,    72,    16,    17,    18,   105,    73,   277,    74,   283,
     278,    75,   345,    76,   347,   622,    77,    78,   183,   205,
     444,   430,    79,   675,   903,   805,   676,   677,    80,    81,
     439,   206,   288,   544,   545,   297,   552,   553,   262,   728,
     263,   264,   741,    19,    20,    21,    22,   308,    23,    24,
      25,   306,   570,    26,    27,    28,    29,    30,    31,    32,
      33,    34,   130,   324,    35,    36,    37,    38,    39,    40,
      41,    42,    43,   102,   301,    82,   346,    83,   613,   614,
     615,   786,   679,   680,   445,   625,   617,    84,   350,   882,
     883,  1033,   376,   174,   378,   379,   380,   175,   382,   383,
    1035,   356,   176,   357,   365,   177,   178,   179,   885,   180,
     409,   633,   886,   944,   945,   634,   635,   887,    85,    86,
      87,    88,   241,   242,   243,   489,   719,   840,   841,   964,
    1005,   720,   920,   973,   721,  1059,   917,   961,   962,   963,
    1107,  1121,  1122,  1123,   833,   998,   958,   834,   765,    44,
      45,    46,    47,    48,    49,    50,    51,    89,    90,    91,
      92,   505,  1022,   977,   921,   846,   497,   498,    93,    94,
     269,   270,    52,    95,    53,    96,    97,    98,   410
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -864
static const yytype_int16 yypact[] =
{
      72,  1546,    29,   193,   395,   177,    97,   397,   395,   194,
     542,   416,   358,   395,   395,   395,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,   199,   401,   126,   417,
     282,   196,  -864,   288,   175,   169,   304,   500,  -864,  -864,
    -864,  -864,  -864,   313,   313,  -864,  -864,  -864,   200,  -864,
     313,   538,  -864,  -864,  -864,   313,  1349,   313,  1142,   313,
    1144,   313,  1337,   313,  1401,  -864,  -864,  1389,   865,  -864,
    -864,  -864,   135,  -864,   371,   362,  -864,   395,   395,   402,
     152,  -864,   447,    77,   422,   428,  -864,   435,   149,   435,
     464,   472,   505,   509,   524,   545,   549,  -864,  -864,  -864,
     471,   551,   553,   561,   547,  -864,   514,  -864,    -8,   135,
      51,  -864,  -864,  -864,  -864,   176,   316,  -864,   446,  -864,
    -864,   327,   152,  -864,     1,  -864,  -864,  -864,   582,   342,
    -864,   474,   476,    -1,   500,  -864,  -864,  -864,   366,   373,
     378,   411,   500,    31,  -864,  -864,  -864,  -864,  -864,  1111,
    -864,  1667,  -864,   412,   421,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,   423,   135,   687,   638,    51,    58,  -864,
     218,   218,   689,   152,  -864,   429,   430,   426,   124,   702,
      58,   515,   533,   -11,   625,   562,   393,   663,   560,   674,
      51,   567,   557,   245,   568,   555,   323,   554,   552,    58,
     543,   571,   556,  -864,   473,   482,   446,    58,    51,  -864,
    -864,   478,  -864,   480,   488,   706,   283,  -864,   502,   501,
    -864,   766,    51,   503,   -29,    51,    51,   736,   738,   -66,
    -864,   523,   525,  -864,   528,   534,   393,  -864,   527,     8,
     535,   751,    58,   759,    58,   758,  -864,   563,   569,   575,
     762,   806,  -864,   577,   587,  -864,  -864,   481,   822,   830,
    -864,   831,   833,  -864,  -864,  -864,   183,   838,  -864,  -864,
     544,   844,  -864,   866,  -864,  -864,   763,  -864,   883,   435,
     435,   435,   213,   240,   884,   885,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,   886,  -864,  -864,  -864,    61,   850,
    -864,  -864,   629,   611,  -864,  -864,  -864,   500,  -864,    51,
     548,  -864,  -864,  -864,  -864,   612,  1346,   -15,  -864,   613,
     616,     1,  -864,     1,   500,     1,     1,   894,  -864,   897,
     220,   888,   889,    -1,    -1,   890,  -864,   500,   500,   500,
     500,    13,  -864,  -864,  -864,   901,  -864,  -864,   893,  -864,
     182,    86,  -864,   190,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,   500,   500,   500,   500,   500,   500,   500,   500,   500,
     605,   500,   500,   500,   500,  -864,  -864,  -864,  -864,  -864,
    -864,   446,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  1178,  -864,   500,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,   858,   859,   641,   642,    51,   898,   715,    58,
     393,    51,  -864,  -864,   756,    51,   868,    51,   755,   873,
     760,    51,    51,    58,   877,   664,   879,   665,    58,   669,
      58,    95,    58,  -864,  -864,   661,  -864,  -864,  -864,   147,
    -864,  -864,   933,   671,   672,  -864,  -864,  -864,   675,  -864,
     676,   874,  -864,  -864,  -864,   677,   684,   685,    51,    51,
     688,   692,   703,  -864,  -864,   247,  -864,   106,  -864,  -864,
    -864,    78,  -864,  -864,  -864,    51,  -864,    51,  -864,    58,
    -864,  -864,  -864,    51,  -864,   393,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,   347,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,   349,  -864,  -864,   964,  -864,  -864,  -864,  -864,
     952,   965,  -864,  1178,  -864,  -864,  -864,   393,  -864,  -864,
     967,  -864,  -864,   969,   969,   969,   435,   969,   435,   969,
    -864,  -864,  -864,   705,   708,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,   351,  -864,  -864,   852,   971,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,    56,   710,   709,  -864,  -864,  -864,   393,
      51,  -864,  -864,  -864,   483,  1346,  -864,  -864,   875,  -864,
    -864,  -864,  -864,   100,  -864,   713,  -864,  -864,  -864,  -864,
    -864,   953,   123,   132,   155,  -864,   500,   711,   970,    36,
    -864,  -864,  -864,   980,   987,  -864,  1589,  1589,  1589,  1690,
    1690,  1589,  1589,  1589,  1589,  -864,   718,  -864,  -864,  -864,
     621,   621,  -864,  -864,  -864,   978,  -864,   721,  -864,  -864,
    -864,  1667,    51,    51,  -864,  -864,   726,    58,    58,   727,
     745,   746,    51,   747,    51,   748,   393,    51,   749,   750,
     752,   753,    51,  -864,   446,  -864,   764,  -864,   765,   773,
     774,   775,   776,  -864,  -864,    53,   761,   785,   393,   805,
    -864,   804,   807,  -864,  -864,    90,  -864,  -864,   829,   808,
    -864,  -864,  -864,   834,   835,  -864,  -864,  -864,    51,   259,
     300,   840,    51,   393,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,   106,  -864,  -864,  -864,   481,  -864,   183,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  1043,  1110,  1114,   969,  1121,
     969,  1122,  -864,  -864,  -864,   629,  -864,  -864,  -864,  -864,
    1346,  1109,  -864,  -864,  -864,  -864,   381,  -864,  -864,   500,
    -864,  -864,   500,   500,   500,   991,    76,    47,  -864,  -864,
     843,   582,  -864,   500,  -864,   522,  -864,   848,   845,  -864,
     867,   869,  -864,  -864,  -864,   870,  -864,   871,  -864,   872,
     900,  -864,  -864,  -864,  -864,   902,   904,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,    51,  -864,  -864,   446,  -864,
    -864,   864,  -864,  -864,  -864,  -864,  -864,  -864,   874,  -864,
    -864,   905,  1146,  1146,  1146,  1146,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  1127,  -864,  1167,  -864,  -864,  -864,
       5,  -864,  1166,  -864,  -864,   909,   500,  -864,  -864,  -864,
     500,   500,   906,  1152,  1111,   908,   479,  -864,  -864,  1346,
    -864,  1014,  1042,  1051,  -864,   911,   912,  1180,  -864,  1181,
    1074,   393,  -864,  -864,  -864,  -864,    51,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,   207,  -864,    10,  -864,  -864,
     457,    25,  -864,   247,  -864,   919,   920,   925,   926,  -864,
    -864,   500,   220,   500,  -864,   565,  1619,   420,  -864,  -864,
    1187,   338,  -864,  1667,   936,   941,  -864,  -864,  -864,  -864,
    -864,  -864,   939,   942,  -864,  -864,   944,  -864,  -864,  -864,
     -72,  -864,  -864,  -864,   180,    51,    51,    51,  1045,   102,
     -12,  -864,  -864,   960,    51,   254,  -864,   961,  -864,   247,
     247,   247,   247,   657,   963,   691,   981,  -864,  1254,  -864,
     500,   500,   436,   479,  -864,  1231,  -864,  -864,  1341,   966,
    -864,    58,    51,    58,  -864,   968,  -864,  -864,  -864,    51,
      51,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,   973,  1109,  1106,  -864,    40,   722,   767,   975,   983,
     989,   988,  -864,   976,    51,    58,    51,    51,    51,  1086,
      51,   154,   312,     2,  1035,    51,  1025,  -864,  -864,   994,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,   995,  1001,  1254,
    1004,  -864,  1005,  -864,  -864,   500,   500,   500,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,    51,  -864,    51,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,    58,  -864,    85,  -864,  -864,
    -864,  -864,  1213,  1213,  1083,  1112,  1135,    27,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  1007,  1008,  1009,   -27,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  1010,  -864,  -864
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -864,  -864,  -864,  -139,  -864,  -205,  -208,  -264,  -558,  -864,
    -271,  -864,  -864,  -864,  -864,  -864,  -864,   516,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  1188,  -864,  -864,  -864,  -864,
    -864,  -864,  -232,  -864,  -864,  -864,  -864,  -864,   757,   714,
    1094,  -864,  -864,  -864,  -864,  -864,   493,  -480,  -864,  -864,
    -864,  -864,   -80,  -864,   570,    18,  -864,   558,  -863,   452,
    -488,  -864,  -559,  -864,  -864,  -864,  -864,  -108,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,   443,  1020,  -864,  -864,  -864,  -864,  -609,
     546,  -864,   977,   772,  1016,  -864,   704,  -864,  -864,  -783,
    -864,   267,  -864,  -170,   550,  -642,  -864,  -167,   690,  -864,
    -864,  -864,  -864,   225,   328,  -864,  -864,  -765,  -864,    44,
     461,   -67,  -864,  -864,   353,   559,   418,  -852,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -364,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,
    -864,  -864,  -864,  -864,  -864,  -864,  -864,  -864,  -812
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -337
static const yytype_int16 yytable[] =
{
     181,   338,   518,   377,   485,   455,   381,   799,   464,   762,
     314,   315,   363,   729,   351,   785,   884,   725,   352,   411,
     412,   413,   414,   415,   479,   931,   462,   463,   353,    54,
     331,   939,   486,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   569,  1119,   372,   373,   374,   158,   502,   372,
     373,   374,   798,   372,   373,   374,   798,   972,   432,   334,
     372,   373,   374,   798,   335,  1012,   335,   526,   336,   528,
     336,   335,   941,   778,   335,   336,     1,   583,   336,  1092,
     302,   468,   618,   619,   620,   433,   434,   435,   436,   895,
     433,   434,   435,   436,   462,   463,   745,   366,   335,   487,
     106,   458,   336,   896,   510,   371,   411,   412,   413,   414,
     415,   651,   652,   500,   428,   335,   506,   507,   884,   336,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   411,
     412,   413,   414,   415,   742,  1058,  1034,   520,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   411,   412,   413,   414,   415,   307,   335,   285,   986,
     349,   336,   999,   286,  1031,   416,   417,   418,   419,   420,
     421,   422,   423,   424,  1070,   294,   521,   151,   303,   584,
     295,   459,  1010,   779,   339,   103,    55,   111,   358,   674,
     599,   573,   574,   575,   577,   579,   511,   512,  1000,   974,
     766,   767,   139,   769,  1127,   771,   134,  1034,   140,   522,
     714,   448,   309,   884,    56,  1013,   104,   340,  1014,    57,
     307,   154,    58,   155,   156,   157,   158,   159,   160,  1093,
     503,   504,  1094,    59,  1087,   161,   162,   902,   163,  1068,
     164,   576,   690,  1128,   689,   152,   440,   307,   185,    60,
      61,   550,    62,   186,   148,   621,   310,   884,   701,    63,
     598,   150,   187,   706,   332,   708,   709,   712,   578,   354,
     946,   835,   975,   932,   364,   523,   355,   628,    64,   551,
     165,  1001,   645,   959,   441,   442,   188,   646,    65,   147,
     641,   642,   643,   644,   884,   149,   710,   711,   976,   375,
    1120,   452,   453,   746,   375,   738,    66,   686,   375,  1114,
    1115,   153,   691,  1071,   749,   897,   693,   751,   695,   337,
     182,   337,   699,   700,  1111,  1112,   337,   884,   884,   337,
     852,   739,   853,   955,   656,   657,   658,   659,   660,   661,
     662,   663,   664,   915,   670,   671,   672,   673,   991,   751,
     729,    67,   992,   337,   978,   290,   166,   167,   653,   733,
     734,   127,   845,   715,   806,   740,   681,   168,   169,   170,
     337,   854,   171,   855,   789,   443,   747,   743,   748,   103,
     716,   128,   717,  1072,   750,   358,   129,   358,   718,   358,
     358,   311,   312,   313,   864,   293,   866,   792,  1002,   462,
     463,   782,    99,   100,    99,   100,   793,   287,   189,   135,
    1024,  1025,  1026,  1027,  1003,   304,   411,   412,   413,   414,
     415,   305,   337,   101,   296,   101,   471,   472,   136,   794,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   137,
     109,   110,   307,   141,   839,   190,   131,   132,   133,   342,
     191,   343,   344,  1004,   298,   299,   649,   316,   768,   650,
     770,   142,   826,    68,   654,   317,   192,   655,  1017,   377,
     120,   783,   810,   811,   323,   300,   143,   381,   819,   957,
     154,   806,   155,   156,   157,   158,   159,   160,   172,  -336,
     107,   108,   121,   122,   161,   162,   173,   163,   318,   164,
     836,   154,   319,   155,   156,   157,   158,   159,   160,  1018,
    1019,  1020,  1021,   475,   476,   161,   162,   320,   163,   144,
     164,   123,   124,   125,   126,   858,  1089,   251,   462,   463,
     901,   493,   494,   807,   808,   538,  1090,  1091,   321,   165,
     291,   292,   322,   815,   325,   817,   326,   253,   820,   539,
     540,   298,   299,   825,   327,   541,   542,   328,   543,   329,
     165,   411,   412,   413,   414,   415,   626,   194,   627,   795,
     629,   630,   926,   927,   928,   416,   417,   418,   419,   420,
     421,   422,   423,   424,  1038,  1039,   341,   751,  1040,   851,
     600,   601,  1041,   857,   195,   196,   918,   348,   197,   198,
     359,   199,   200,   201,   603,   604,   605,   606,   607,   608,
     360,   609,   610,   611,   612,   166,   167,   377,   112,   113,
     754,   755,   756,   757,   774,   775,   168,   169,   170,   202,
     361,   171,   362,   203,   367,   965,   166,   167,   423,   424,
     114,   368,   115,   960,   116,   117,   369,   168,   169,   170,
     888,   889,   171,   411,   412,   413,   414,   415,   586,   587,
     588,   589,   590,   591,   592,   593,   594,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   966,   967,   968,   370,
     969,   638,   639,   751,   425,   970,   916,   411,   412,   413,
     414,   415,   989,   426,   429,   427,   431,   447,   451,   449,
     450,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     454,   456,   457,   460,   465,   891,   892,   893,   411,   412,
     413,   414,   415,   461,   466,   467,   900,   470,   665,   469,
     971,   474,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   480,   473,   483,   477,   666,   478,   172,   488,  -331,
     118,   667,   784,   490,   484,   173,   558,   956,   668,   669,
     491,   482,   492,   411,   412,   413,   414,   415,   172,   462,
     463,   481,   495,   496,   499,   501,   173,   416,   417,   418,
     419,   420,   421,   422,   423,   424,  1061,   508,  1063,   509,
     234,   559,   240,   513,   248,   514,   261,   519,   268,   935,
     515,   279,   284,   936,   937,   524,   516,   525,   119,   943,
     527,   204,   529,   533,   534,   546,  1006,  1007,  1008,   560,
    1011,   184,   561,   547,   548,  1016,   549,   193,   562,   563,
    1081,   554,   207,   530,   235,   987,   244,   556,   249,   531,
     265,   564,   565,   566,   567,   532,   568,   536,   411,   412,
     413,   414,   415,  1062,   983,   377,   985,   537,   585,   557,
    1065,  1066,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   411,   412,   413,   414,   415,   572,   580,   581,   582,
    1110,   597,   602,   623,   624,   416,   417,   418,   419,   420,
     421,   422,   423,   424,   194,  1080,   631,  1082,  1083,  1084,
     632,  1086,  1088,   636,   647,   637,  1096,   648,   640,   682,
     683,   684,   685,  1036,  1037,   280,   943,   692,   688,   694,
     443,   195,   196,   696,   697,   197,   198,  1028,   702,   200,
     201,   698,   687,   713,   703,   705,  1108,   704,  1109,   707,
     722,   723,   724,   281,   251,   726,   727,   730,  1113,   411,
     412,   413,   414,   415,   731,   732,   202,   758,   735,   759,
     203,  1030,   736,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   760,   737,   763,   772,   384,   764,   773,   777,
     780,   781,   790,   800,   797,   796,   803,   411,   412,   413,
     414,   415,  1073,   801,   804,   806,   809,   812,  1104,  1105,
    1106,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     411,   412,   413,   414,   415,   813,   814,   816,   818,   821,
     822,   838,   823,   824,   416,   417,   418,   419,   420,   421,
     422,   423,   424,   837,   827,   828,   861,  1074,   411,   412,
     413,   414,   415,   829,   830,   831,   832,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     411,   412,   413,   414,   415,   842,   843,   844,   848,   411,
     412,   413,   414,   415,   416,   417,   418,   419,   420,   421,
     422,   423,   424,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   847,   862,   849,   850,   384,   863,   411,   412,
     413,   414,   415,   856,   865,   867,   870,   899,   905,   906,
     929,   776,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   411,   412,   413,   414,   415,   919,   907,   282,   908,
     909,   910,   911,   871,   788,   416,   417,   418,   419,   420,
     421,   422,   423,   424,   924,  -229,  -229,  -229,  -229,   870,
     930,   393,   394,   395,   396,   397,   398,   399,   400,   872,
     912,   194,   913,   194,   914,   923,   933,   934,   942,   938,
     950,   951,   385,   952,   953,   678,   871,   401,   402,   979,
     980,   403,   404,   405,   406,   981,   982,   990,   195,   196,
     195,   196,   197,   198,   197,   198,   200,   201,   200,   201,
     993,   994,   872,   995,   997,   996,  -228,  -228,  -228,  -228,
     870,  1009,   791,   873,   874,   875,   876,   877,   878,   879,
    1015,  1023,  1029,   202,   895,   202,  1060,   203,  1064,   203,
    1069,   880,  1067,  1075,   881,   407,   408,   871,  1079,   386,
     387,  1076,   388,   389,   390,   391,   392,  1077,  1078,  1095,
     894,   870,  1085,  1097,  1098,  1099,   873,   874,   875,   876,
     877,   878,   879,   872,  1100,  1102,  1103,  1124,  1125,  1126,
    1129,   868,   289,   947,   880,   446,  -229,   881,   871,   904,
     922,   393,   394,   395,   396,   397,   398,   399,   400,   603,
     604,   605,   606,   607,   608,   860,   609,   610,   611,   612,
     555,   948,   571,   616,   872,   859,   869,   401,   402,   787,
     949,   403,   404,   405,   406,   761,  1101,   873,   874,   875,
     876,   877,   878,   879,   802,   940,  1042,   898,   890,     0,
     984,     0,     0,   954,     0,   880,     0,  -228,   881,     0,
       0,   236,  1116,     0,     0,     0,     0,   237,     0,   238,
       0,     0,     0,     0,     0,     0,   194,     0,   873,   874,
     875,   876,   877,   878,   879,   407,   408,     0,   194,     0,
       0,  1117,  -228,   208,     0,     0,   880,     0,     0,   881,
     245,   246,     0,   195,   196,     0,     0,   197,   198,     0,
     250,   200,   201,     0,  1118,   195,   196,   251,     0,   197,
     198,   251,     0,   200,   201,   239,     0,   247,   194,   209,
    -229,   210,     0,     0,     0,  -229,   252,   253,   202,     0,
     194,   253,   203,     0,     0,     0,     0,   271,   272,   273,
     202,   274,   275,     0,   203,   195,   196,     0,  1043,   197,
     198,   211,     0,   200,   201,     0,     0,   195,   196,   212,
     213,   197,   198,     0,     0,   200,   201,   603,   604,   605,
     606,   607,   608,     0,   609,   610,   611,   612,     0,     0,
     202,  -228,     0,     0,   203,     0,  -228,     0,     0,     0,
       0,   214,   202,     0,     0,     0,   203,     0,     0,     0,
       0,     0,     0,   266,     0,     0,     0,     0,   215,   216,
       0,   217,   218,     0,   219,   220,   221,     0,   222,  1044,
       0,   223,   224,     0,   225,     0,  1032,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   226,   227,     0,
       0,     0,   228,     0,   229,     0,   230,     0,   231,     0,
       0,     0,     0,   232,     0,     0,     0,  1045,     0,  1046,
    1047,  1048,  1049,  1050,  1051,   254,     0,  1052,     0,  1053,
    1054,     0,     0,  1055,     0,     4,     0,     0,  1056,     0,
       0,     0,     0,     0,     0,  -337,  -337,  -337,   255,   256,
     257,   258,     5,     6,     0,     0,     0,     0,   259,  -337,
    -337,  -337,  -337,  -337,   421,   422,   423,   424,     0,     0,
     260,     0,     0,     0,  1057,   411,   412,   413,   414,   415,
       0,     0,   233,     0,     0,     0,     0,     0,     0,   416,
     417,   418,   419,   420,   421,   422,   423,   424,     7,     0,
       0,     8,     0,     9,     0,    10,     0,     0,     0,     0,
      11,     0,     0,     0,     0,    12,    13,    14,    15,     0,
       0,     0,   276,   411,   412,   413,   414,   415,     0,     0,
       0,     0,     0,     0,   267,     0,     0,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   411,   412,   413,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     416,   417,   418,   419,   420,   421,   422,   423,   424,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   988
};

static const yytype_int16 yycheck[] =
{
      67,   140,   266,   173,   236,   210,   173,   649,   216,   567,
     118,   119,    13,   501,    13,   624,   781,   497,    17,     6,
       7,     8,     9,    10,   229,    20,    16,    17,    27,     0,
      38,   883,   237,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   306,    16,    13,    14,    15,    16,    77,    13,
      14,    15,    16,    13,    14,    15,    16,   920,   197,   139,
      13,    14,    15,    16,    13,    77,    13,   272,    17,   274,
      17,    13,   884,    17,    13,    17,     4,    16,    17,    77,
       3,   220,    97,    98,    99,    32,    33,    34,    35,    13,
      32,    33,    34,    35,    16,    17,    18,   164,    13,   238,
       3,   112,    17,    27,   170,   172,     6,     7,     8,     9,
      10,    25,    26,   252,   194,    13,   255,   256,   883,    17,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     6,
       7,     8,     9,    10,    28,   998,   988,   129,     6,     7,
       8,     9,    10,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     6,     7,     8,     9,    10,    17,    13,    33,   934,
     152,    17,   244,    38,   986,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   144,    33,   178,    18,   111,   328,
      38,   202,    90,   137,    18,    18,     3,     3,   154,   431,
     339,   309,   310,   311,   312,   313,   272,   273,   280,   184,
     574,   575,    86,   577,   241,   579,    17,  1069,    92,   211,
      73,   203,    73,   988,    31,   237,    49,    51,   240,    36,
      17,    11,    39,    13,    14,    15,    16,    17,    18,   237,
     269,   270,   240,    50,    90,    25,    26,   805,    28,  1032,
      30,    38,   460,   280,   459,    86,    38,    17,    58,    66,
      67,    78,    69,    63,    68,   280,   117,  1032,   473,    76,
     337,    96,    72,   478,   282,   480,   181,   482,    38,   278,
     889,   228,   257,   278,   285,   277,   285,   354,    95,   106,
      70,   111,   279,   283,    76,    77,    96,   284,   105,    17,
     367,   368,   369,   370,  1069,    17,   211,   212,   283,   278,
     283,   187,   188,   521,   278,    68,   123,   456,   278,  1102,
    1103,    17,   461,   283,   529,   278,   465,   535,   467,   278,
      17,   278,   471,   472,   249,   250,   278,  1102,  1103,   278,
      81,    94,    83,   901,   411,   412,   413,   414,   415,   416,
     417,   418,   419,   833,   421,   422,   423,   424,    20,   567,
     848,   168,    24,   278,   923,     3,   146,   147,   282,   508,
     509,    13,   282,   226,   284,   128,   443,   157,   158,   159,
     278,    81,   162,    83,   284,   167,   525,   281,   527,    18,
     243,    33,   245,  1035,   533,   351,    38,   353,   251,   355,
     356,   252,   253,   254,   768,     3,   770,   284,   228,    16,
      17,   619,    17,    18,    17,    18,   284,   282,   218,    18,
     979,   980,   981,   982,   244,     3,     6,     7,     8,     9,
      10,     3,   278,    38,   282,    38,   191,   192,    37,   284,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    48,
       7,     8,    17,    36,   718,   255,    13,    14,    15,    13,
     260,    15,    16,   283,    17,    18,   284,     3,   576,   287,
     578,    54,   704,   280,   284,     3,   276,   287,   224,   649,
      64,   620,   687,   688,    13,    38,    69,   654,   696,   282,
      11,   284,    13,    14,    15,    16,    17,    18,   278,   279,
     103,   104,    86,    87,    25,    26,   286,    28,     3,    30,
     715,    11,     3,    13,    14,    15,    16,    17,    18,   265,
     266,   267,   268,   200,   201,    25,    26,     3,    28,   112,
      30,   115,   116,   117,   118,   743,   224,    80,    16,    17,
      18,   258,   259,   682,   683,    64,   234,   235,     3,    70,
     107,   108,     3,   692,     3,   694,     3,   100,   697,    78,
      79,    17,    18,   702,     3,    84,    85,    20,    87,    55,
      70,     6,     7,     8,     9,    10,   351,    39,   353,   646,
     355,   356,   853,   854,   855,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   158,   159,   280,   805,   162,   738,
      52,    53,   166,   742,    66,    67,   838,   280,    70,    71,
      28,    73,    74,    75,   131,   132,   133,   134,   135,   136,
     278,   138,   139,   140,   141,   146,   147,   797,    86,    87,
     283,   284,   283,   284,   283,   284,   157,   158,   159,   101,
     166,   162,   166,   105,   278,   188,   146,   147,    27,    28,
     108,   278,   110,   917,   112,   113,   278,   157,   158,   159,
     279,   280,   162,     6,     7,     8,     9,    10,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   229,   230,   231,   278,
     233,   363,   364,   901,   282,   238,   835,     6,     7,     8,
       9,    10,   282,   282,    17,   282,    68,    18,   282,   280,
     280,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      18,   206,   189,    98,    61,   792,   793,   794,     6,     7,
       8,     9,    10,   171,   174,    61,   803,   180,   133,   172,
     283,   186,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   208,   184,   280,   200,   150,   204,   278,   280,   280,
     218,   156,   279,   283,   282,   286,     3,   906,   163,   164,
     282,   215,    66,     6,     7,     8,     9,    10,   278,    16,
      17,   210,   280,   282,    18,   282,   286,    20,    21,    22,
      23,    24,    25,    26,    27,    28,  1001,    61,  1003,    61,
      86,    38,    88,   280,    90,   280,    92,   280,    94,   876,
     282,    97,    98,   880,   881,   280,   282,    66,   276,   886,
      61,   283,    64,    61,    18,     3,   965,   966,   967,    66,
     969,    74,    69,     3,     3,   974,     3,    80,    75,    76,
    1045,     3,    85,   280,    87,   280,    89,     3,    91,   280,
      93,    88,    89,    90,    91,   280,    93,   280,     6,     7,
       8,     9,    10,  1002,   931,  1035,   933,   280,    18,     3,
    1009,  1010,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     6,     7,     8,     9,    10,     3,     3,     3,     3,
    1095,   280,   280,   280,   278,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    39,  1044,    12,  1046,  1047,  1048,
      13,  1050,  1051,    25,    13,    26,  1055,    24,    28,    61,
      61,   280,   280,   990,   991,    60,   993,   171,   213,    61,
     167,    66,    67,   178,    61,    70,    71,   280,    61,    74,
      75,   181,    44,   282,   280,   280,  1085,    68,  1087,   280,
      17,   280,   280,    88,    80,   280,   280,   280,  1097,     6,
       7,     8,     9,    10,   280,   280,   101,     3,   280,    17,
     105,   280,   280,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    17,   280,    17,   280,     5,    18,   280,    18,
     280,   282,   279,    13,    24,   284,   278,     6,     7,     8,
       9,    10,   280,    16,    26,   284,   280,   280,  1075,  1076,
    1077,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       6,     7,     8,     9,    10,   280,   280,   280,   280,   280,
     280,   246,   280,   280,    20,    21,    22,    23,    24,    25,
      26,    27,    28,   282,   280,   280,     3,   280,     6,     7,
       8,     9,    10,   280,   280,   280,   280,     6,     7,     8,
       9,    10,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       6,     7,     8,     9,    10,   280,   282,   280,   280,     6,
       7,     8,     9,    10,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   283,     3,   280,   280,     5,     3,     6,     7,
       8,     9,    10,   283,     3,     3,    17,   284,   280,   284,
       3,   279,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     6,     7,     8,     9,    10,   282,   280,   283,   280,
     280,   280,   280,    44,   279,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    18,    13,    14,    15,    16,    17,
       3,   190,   191,   192,   193,   194,   195,   196,   197,    70,
     280,    39,   280,    39,   280,   280,    20,   278,   280,   283,
     279,   279,    81,    13,    13,    17,    44,   216,   217,   280,
     280,   220,   221,   222,   223,   280,   280,    20,    66,    67,
      66,    67,    70,    71,    70,    71,    74,    75,    74,    75,
     284,   280,    70,   284,   280,   283,    13,    14,    15,    16,
      17,   186,   279,   124,   125,   126,   127,   128,   129,   130,
     280,   280,   279,   101,    13,   101,   280,   105,   280,   105,
     144,   142,   279,   278,   145,   274,   275,    44,   282,   148,
     149,   278,   151,   152,   153,   154,   155,   278,   280,   234,
     279,    17,   186,   248,   280,   280,   124,   125,   126,   127,
     128,   129,   130,    70,   283,   281,   281,   280,   280,   280,
     280,   775,   104,   279,   142,   201,   144,   145,    44,   806,
     848,   190,   191,   192,   193,   194,   195,   196,   197,   131,
     132,   133,   134,   135,   136,   757,   138,   139,   140,   141,
     300,   279,   306,   346,    70,   755,   780,   216,   217,   625,
     279,   220,   221,   222,   223,   563,  1069,   124,   125,   126,
     127,   128,   129,   130,   654,   884,   993,   797,   789,    -1,
     932,    -1,    -1,   279,    -1,   142,    -1,   144,   145,    -1,
      -1,   219,   279,    -1,    -1,    -1,    -1,   225,    -1,   227,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,   124,   125,
     126,   127,   128,   129,   130,   274,   275,    -1,    39,    -1,
      -1,   279,   283,    44,    -1,    -1,   142,    -1,    -1,   145,
     256,   257,    -1,    66,    67,    -1,    -1,    70,    71,    -1,
      73,    74,    75,    -1,   279,    66,    67,    80,    -1,    70,
      71,    80,    -1,    74,    75,   283,    -1,   283,    39,    80,
     278,    82,    -1,    -1,    -1,   283,    99,   100,   101,    -1,
      39,   100,   105,    -1,    -1,    -1,    -1,    58,    59,    60,
     101,    62,    63,    -1,   105,    66,    67,    -1,   117,    70,
      71,   112,    -1,    74,    75,    -1,    -1,    66,    67,   120,
     121,    70,    71,    -1,    -1,    74,    75,   131,   132,   133,
     134,   135,   136,    -1,   138,   139,   140,   141,    -1,    -1,
     101,   278,    -1,    -1,   105,    -1,   283,    -1,    -1,    -1,
      -1,   152,   101,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,   169,   170,
      -1,   172,   173,    -1,   175,   176,   177,    -1,   179,   188,
      -1,   182,   183,    -1,   185,    -1,   282,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,   203,    -1,   205,    -1,   207,    -1,   209,    -1,
      -1,    -1,    -1,   214,    -1,    -1,    -1,   226,    -1,   228,
     229,   230,   231,   232,   233,   238,    -1,   236,    -1,   238,
     239,    -1,    -1,   242,    -1,    39,    -1,    -1,   247,    -1,
      -1,    -1,    -1,    -1,    -1,     6,     7,     8,   261,   262,
     263,   264,    56,    57,    -1,    -1,    -1,    -1,   271,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    -1,
     283,    -1,    -1,    -1,   283,     6,     7,     8,     9,    10,
      -1,    -1,   283,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,   102,    -1,
      -1,   105,    -1,   107,    -1,   109,    -1,    -1,    -1,    -1,
     114,    -1,    -1,    -1,    -1,   119,   120,   121,   122,    -1,
      -1,    -1,   283,     6,     7,     8,     9,    10,    -1,    -1,
      -1,    -1,    -1,    -1,   283,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   143
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   289,   290,    39,    56,    57,   102,   105,   107,
     109,   114,   119,   120,   121,   122,   310,   311,   312,   351,
     352,   353,   354,   356,   357,   358,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   457,   458,   459,   460,   461,   462,
     463,   464,   480,   482,     0,     3,    31,    36,    39,    50,
      66,    67,    69,    76,    95,   105,   123,   168,   280,   292,
     300,   306,   309,   314,   316,   319,   321,   324,   325,   330,
     336,   337,   383,   385,   395,   426,   427,   428,   429,   465,
     466,   467,   468,   476,   477,   481,   483,   484,   485,    17,
      18,    38,   381,    18,    49,   313,     3,   103,   104,   381,
     381,     3,    86,    87,   108,   110,   112,   113,   218,   276,
      64,    86,    87,   115,   116,   117,   118,    13,    33,    38,
     370,   381,   381,   381,    17,    18,    37,    48,   302,    86,
      92,    36,    54,    69,   112,   307,   308,    17,    68,    17,
      96,    18,    86,    17,    11,    13,    14,    15,    16,    17,
      18,    25,    26,    28,    30,    70,   146,   147,   157,   158,
     159,   162,   278,   286,   401,   405,   410,   413,   414,   415,
     417,   419,    17,   326,   326,    58,    63,    72,    96,   218,
     255,   260,   276,   326,    39,    66,    67,    70,    71,    73,
      74,    75,   101,   105,   283,   327,   339,   326,    44,    80,
      82,   112,   120,   121,   152,   169,   170,   172,   173,   175,
     176,   177,   179,   182,   183,   185,   198,   199,   203,   205,
     207,   209,   214,   283,   327,   326,   219,   225,   227,   283,
     327,   430,   431,   432,   326,   256,   257,   283,   327,   326,
      73,    80,    99,   100,   238,   261,   262,   263,   264,   271,
     283,   327,   346,   348,   349,   326,   112,   283,   327,   478,
     479,    58,    59,    60,    62,    63,   283,   315,   318,   327,
      60,    88,   283,   317,   327,    33,    38,   282,   340,   313,
       3,   381,   381,     3,    33,    38,   282,   343,    17,    18,
      38,   382,     3,   111,     3,     3,   359,    17,   355,    73,
     117,   252,   253,   254,   355,   355,     3,     3,     3,     3,
       3,     3,     3,    13,   371,     3,     3,     3,    20,    55,
     301,    38,   282,   303,   340,    13,    17,   278,   291,    18,
      51,   280,    13,    15,    16,   320,   384,   322,   280,   343,
     396,    13,    17,    27,   278,   285,   409,   411,   417,    28,
     278,   166,   166,    13,   285,   412,   419,   278,   278,   278,
     278,   419,    13,    14,    15,   278,   400,   401,   402,   403,
     404,   405,   406,   407,     5,    81,   148,   149,   151,   152,
     153,   154,   155,   190,   191,   192,   193,   194,   195,   196,
     197,   216,   217,   220,   221,   222,   223,   274,   275,   418,
     486,     6,     7,     8,     9,    10,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   282,   282,   282,   340,    17,
     329,    68,   291,    32,    33,    34,    35,   291,   293,   338,
      38,    76,    77,   167,   328,   392,   328,    18,   343,   280,
     280,   282,   187,   188,    18,   293,   206,   189,   112,   202,
      98,   171,    16,    17,   294,    61,   174,    61,   291,   172,
     180,   191,   192,   184,   186,   200,   201,   200,   204,   293,
     208,   210,   215,   280,   282,   320,   293,   291,   280,   433,
     283,   282,    66,   258,   259,   280,   282,   474,   475,    18,
     291,   282,    77,   269,   270,   469,   291,   291,    61,    61,
     170,   272,   273,   280,   280,   282,   282,   294,   295,   280,
     129,   178,   211,   277,   280,    66,   293,    61,   293,    64,
     280,   280,   280,    61,    18,   299,   280,   280,    64,    78,
      79,    84,    85,    87,   341,   342,     3,     3,     3,     3,
      78,   106,   344,   345,     3,   382,     3,     3,     3,    38,
      66,    69,    75,    76,    88,    89,    90,    91,    93,   295,
     360,   392,     3,   355,   355,   355,    38,   355,    38,   355,
       3,     3,     3,    16,   291,    18,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   304,   305,   280,   419,   291,
      52,    53,   280,   131,   132,   133,   134,   135,   136,   138,
     139,   140,   141,   386,   387,   388,   390,   394,    97,    98,
      99,   280,   323,   280,   278,   393,   411,   411,   419,   411,
     411,    12,    13,   419,   423,   424,    25,    26,   412,   412,
      28,   419,   419,   419,   419,   279,   284,    13,    24,   284,
     287,    25,    26,   282,   284,   287,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   133,   150,   156,   163,   164,
     419,   419,   419,   419,   320,   331,   334,   335,    17,   390,
     391,   419,    61,    61,   280,   280,   291,    44,   213,   293,
     294,   291,   171,   291,    61,   291,   178,    61,   181,   291,
     291,   293,    61,   280,    68,   280,   293,   280,   293,   181,
     211,   212,   293,   282,    73,   226,   243,   245,   251,   434,
     439,   442,    17,   280,   280,   335,   280,   280,   347,   348,
     280,   280,   280,   291,   291,   280,   280,   280,    68,    94,
     128,   350,    28,   281,   297,    18,   294,   291,   291,   293,
     291,   294,   295,   296,   283,   284,   283,   284,     3,    17,
      17,   391,   296,    17,    18,   456,   456,   456,   355,   456,
     355,   456,   280,   280,   283,   284,   279,    18,    17,   137,
     280,   282,   294,   291,   279,   387,   389,   394,   279,   284,
     279,   279,   284,   284,   284,   419,   284,    24,    16,   403,
      13,    16,   406,   278,    26,   333,   284,   291,   291,   280,
     293,   293,   280,   280,   280,   291,   280,   291,   280,   294,
     291,   280,   280,   280,   280,   291,   320,   280,   280,   280,
     280,   280,   280,   452,   455,   228,   293,   282,   246,   295,
     435,   436,   280,   282,   280,   282,   473,   283,   280,   280,
     280,   291,    81,    83,    81,    83,   283,   291,   294,   342,
     345,     3,     3,     3,   456,     3,   456,     3,   305,   388,
      17,    44,    70,   124,   125,   126,   127,   128,   129,   130,
     142,   145,   397,   398,   415,   416,   420,   425,   279,   280,
     423,   419,   419,   419,   279,    13,    27,   278,   402,   284,
     419,    18,   296,   332,   334,   280,   284,   280,   280,   280,
     280,   280,   280,   280,   280,   335,   291,   444,   320,   282,
     440,   472,   347,   280,    18,   298,   298,   298,   298,     3,
       3,    20,   278,    20,   278,   419,   419,   419,   283,   425,
     418,   486,   280,   419,   421,   422,   387,   279,   279,   279,
     279,   279,    13,    13,   279,   296,   291,   282,   454,   283,
     295,   445,   446,   447,   437,   188,   229,   230,   231,   233,
     238,   283,   346,   441,   184,   257,   283,   471,   350,   280,
     280,   280,   280,   419,   424,   419,   415,   280,   143,   282,
      20,    20,    24,   284,   280,   284,   283,   280,   453,   244,
     280,   111,   228,   244,   283,   438,   291,   291,   291,   186,
      90,   291,    77,   237,   240,   280,   291,   224,   265,   266,
     267,   268,   470,   280,   350,   350,   350,   350,   280,   279,
     280,   486,   282,   399,   425,   408,   419,   419,   158,   159,
     162,   166,   422,   117,   188,   226,   228,   229,   230,   231,
     232,   233,   236,   238,   239,   242,   247,   283,   346,   443,
     280,   293,   291,   293,   280,   291,   291,   279,   397,   144,
     144,   283,   403,   280,   280,   278,   278,   278,   280,   282,
     291,   293,   291,   291,   291,   186,   291,    90,   291,   224,
     234,   235,    77,   237,   240,   234,   291,   248,   280,   280,
     283,   399,   281,   281,   419,   419,   419,   448,   291,   291,
     293,   249,   250,   291,   397,   397,   279,   279,   279,    16,
     283,   449,   450,   451,   280,   280,   280,   241,   280,   280
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
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
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
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
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
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
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
#line 312 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:
#line 313 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:
#line 326 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 8:
#line 327 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; ;}
    break;

  case 9:
#line 331 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   ;}
    break;

  case 10:
#line 335 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   ;}
    break;

  case 11:
#line 344 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); ;}
    break;

  case 12:
#line 345 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:
#line 346 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:
#line 347 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:
#line 348 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:
#line 349 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:
#line 356 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 19:
#line 363 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 21:
#line 371 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:
#line 375 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 23:
#line 379 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   ;}
    break;

  case 24:
#line 386 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:
#line 394 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); ;}
    break;

  case 26:
#line 395 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 27:
#line 401 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 28:
#line 410 "cf-parse.y"
    { (yyval.t) = (yyvsp[(2) - (2)].t); ;}
    break;

  case 29:
#line 411 "cf-parse.y"
    { (yyval.t) = bird_name; ;}
    break;

  case 30:
#line 415 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   ;}
    break;

  case 31:
#line 420 "cf-parse.y"
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[(2) - (2)].t); ;}
    break;

  case 32:
#line 421 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 33:
#line 425 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 34:
#line 426 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 35:
#line 430 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); ;}
    break;

  case 36:
#line 431 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); ;}
    break;

  case 37:
#line 435 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 38:
#line 436 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 39:
#line 437 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 40:
#line 438 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 41:
#line 439 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 42:
#line 440 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 43:
#line 441 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 44:
#line 442 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 45:
#line 443 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 46:
#line 449 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); ;}
    break;

  case 47:
#line 450 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   ;}
    break;

  case 48:
#line 459 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; ;}
    break;

  case 49:
#line 460 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; ;}
    break;

  case 50:
#line 461 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; ;}
    break;

  case 51:
#line 462 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; ;}
    break;

  case 52:
#line 465 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; ;}
    break;

  case 53:
#line 466 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; ;}
    break;

  case 54:
#line 467 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; ;}
    break;

  case 55:
#line 468 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; ;}
    break;

  case 57:
#line 480 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); ;}
    break;

  case 58:
#line 483 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); ;}
    break;

  case 59:
#line 486 "cf-parse.y"
    { cmd_shutdown(); ;}
    break;

  case 60:
#line 489 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 62:
#line 498 "cf-parse.y"
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
#line 514 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); ;}
    break;

  case 64:
#line 515 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 65:
#line 519 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[(2) - (2)].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[(2) - (2)].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   ;}
    break;

  case 66:
#line 526 "cf-parse.y"
    { THIS_KRT->devroutes = (yyvsp[(3) - (3)].i); ;}
    break;

  case 67:
#line 532 "cf-parse.y"
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
#line 545 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 69:
#line 549 "cf-parse.y"
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[(2) - (3)].t);
     kpi->prefix = (yyvsp[(3) - (3)].px).addr;
     kpi->pxlen = (yyvsp[(3) - (3)].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   ;}
    break;

  case 70:
#line 562 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 71:
#line 574 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 72:
#line 580 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 74:
#line 582 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[(1) - (1)].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   ;}
    break;

  case 78:
#line 601 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 79:
#line 602 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 80:
#line 603 "cf-parse.y"
    { new_config->listen_bgp_flags |= SKF_V6ONLY; ;}
    break;

  case 81:
#line 610 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 83:
#line 622 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 84:
#line 628 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 86:
#line 636 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 87:
#line 640 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 88:
#line 641 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 89:
#line 642 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); ;}
    break;

  case 90:
#line 643 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 91:
#line 644 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 92:
#line 645 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 93:
#line 646 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); ;}
    break;

  case 94:
#line 647 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); ;}
    break;

  case 95:
#line 651 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 97:
#line 653 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 98:
#line 654 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 99:
#line 658 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 100:
#line 666 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 101:
#line 667 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 102:
#line 675 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 103:
#line 683 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 104:
#line 684 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 105:
#line 685 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 106:
#line 689 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 107:
#line 690 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 111:
#line 707 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 115:
#line 722 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 117:
#line 737 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 118:
#line 738 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 119:
#line 739 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 121:
#line 744 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 122:
#line 748 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 123:
#line 749 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 124:
#line 750 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 125:
#line 751 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 126:
#line 752 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 127:
#line 753 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 128:
#line 759 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 129:
#line 760 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 130:
#line 761 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 132:
#line 766 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 133:
#line 770 "cf-parse.y"
    { (yyval.i) = MD_STATES; ;}
    break;

  case 134:
#line 771 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; ;}
    break;

  case 141:
#line 792 "cf-parse.y"
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

  case 142:
#line 810 "cf-parse.y"
    { ;}
    break;

  case 143:
#line 811 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 144:
#line 812 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 145:
#line 813 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 146:
#line 814 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 147:
#line 815 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 148:
#line 824 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 149:
#line 827 "cf-parse.y"
    { cmd_show_memory(); ;}
    break;

  case 150:
#line 830 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_show, 0, 0); ;}
    break;

  case 151:
#line 833 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(4) - (5)].ps), proto_cmd_show, 0, 1); ;}
    break;

  case 153:
#line 837 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 154:
#line 841 "cf-parse.y"
    { if_show(); ;}
    break;

  case 155:
#line 844 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 156:
#line 847 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 157:
#line 850 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 158:
#line 856 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 159:
#line 862 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   ;}
    break;

  case 160:
#line 869 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 161:
#line 874 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 162:
#line 879 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 163:
#line 884 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 164:
#line 888 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 165:
#line 892 "cf-parse.y"
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

  case 166:
#line 902 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 167:
#line 910 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 168:
#line 914 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 169:
#line 921 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 170:
#line 922 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 171:
#line 926 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 172:
#line 930 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 173:
#line 932 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 174:
#line 934 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 175:
#line 936 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 176:
#line 938 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 177:
#line 940 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 178:
#line 942 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 179:
#line 944 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 180:
#line 950 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 181:
#line 951 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 183:
#line 956 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 184:
#line 957 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 185:
#line 964 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_disable, 1, 0); ;}
    break;

  case 186:
#line 966 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_enable, 1, 0); ;}
    break;

  case 187:
#line 968 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_restart, 1, 0); ;}
    break;

  case 188:
#line 970 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (3)].ps), proto_cmd_reload, 1, CMD_RELOAD); ;}
    break;

  case 189:
#line 972 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); ;}
    break;

  case 190:
#line 974 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(3) - (4)].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); ;}
    break;

  case 191:
#line 978 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_debug, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 192:
#line 982 "cf-parse.y"
    { proto_apply_cmd((yyvsp[(2) - (4)].ps), proto_cmd_mrtdump, 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 193:
#line 985 "cf-parse.y"
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); ;}
    break;

  case 194:
#line 988 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 195:
#line 989 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 196:
#line 990 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 197:
#line 994 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].s); (yyval.ps).patt = 0; ;}
    break;

  case 198:
#line 995 "cf-parse.y"
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; ;}
    break;

  case 199:
#line 996 "cf-parse.y"
    { (yyval.ps).ptr = (yyvsp[(1) - (1)].t); (yyval.ps).patt = 1; ;}
    break;

  case 200:
#line 1003 "cf-parse.y"
    { cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 201:
#line 1003 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s) = cf_define_symbol((yyvsp[(2) - (4)].s), SYM_FILTER, (yyvsp[(4) - (4)].f));
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 202:
#line 1012 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 203:
#line 1016 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 204:
#line 1017 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 205:
#line 1018 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 206:
#line 1019 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 207:
#line 1020 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 208:
#line 1021 "cf-parse.y"
    { (yyval.i) = T_QUAD; ;}
    break;

  case 209:
#line 1022 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 210:
#line 1023 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 211:
#line 1024 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 212:
#line 1025 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 213:
#line 1026 "cf-parse.y"
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

  case 214:
#line 1046 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   ;}
    break;

  case 215:
#line 1057 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 216:
#line 1058 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 217:
#line 1065 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 218:
#line 1066 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 219:
#line 1073 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 220:
#line 1082 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 222:
#line 1090 "cf-parse.y"
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

  case 223:
#line 1114 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 224:
#line 1115 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 225:
#line 1119 "cf-parse.y"
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

  case 226:
#line 1132 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 227:
#line 1135 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 228:
#line 1148 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 229:
#line 1149 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; ;}
    break;

  case 230:
#line 1152 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); ;}
    break;

  case 231:
#line 1153 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); ;}
    break;

  case 232:
#line 1157 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 233:
#line 1160 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 234:
#line 1169 "cf-parse.y"
    { (yyval.i) = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); ;}
    break;

  case 235:
#line 1176 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 236:
#line 1180 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 237:
#line 1181 "cf-parse.y"
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 238:
#line 1182 "cf-parse.y"
    { (yyval.v).type = T_PAIR; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 239:
#line 1183 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 240:
#line 1184 "cf-parse.y"
    {  (yyval.v).type = (yyvsp[(1) - (1)].i) >> 16; (yyval.v).val.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 241:
#line 1188 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from.type = (yyval.e)->to.type = T_PAIR;
	(yyval.e)->from.val.i = make_pair((yyvsp[(2) - (5)].i), 0); 
	(yyval.e)->to.val.i = make_pair((yyvsp[(2) - (5)].i), 0xffff);
   ;}
    break;

  case 242:
#line 1194 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (1)].v); 
	(yyval.e)->to = (yyvsp[(1) - (1)].v);
   ;}
    break;

  case 243:
#line 1199 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (4)].v); 
	(yyval.e)->to = (yyvsp[(4) - (4)].v); 
   ;}
    break;

  case 244:
#line 1207 "cf-parse.y"
    { (yyval.e) = (yyvsp[(1) - (1)].e); ;}
    break;

  case 245:
#line 1208 "cf-parse.y"
    { (yyval.e) = (yyvsp[(3) - (3)].e); (yyval.e)->left = (yyvsp[(1) - (3)].e); ;}
    break;

  case 246:
#line 1212 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 247:
#line 1219 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 248:
#line 1220 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 249:
#line 1221 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 250:
#line 1222 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 251:
#line 1229 "cf-parse.y"
    { (yyval.trie) = f_new_trie(); trie_add_prefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 252:
#line 1230 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_prefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 253:
#line 1233 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 254:
#line 1234 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[(2) - (4)].e);
     (yyval.e)->data = (yyvsp[(4) - (4)].x);
     (yyval.e)->left = (yyvsp[(1) - (4)].e);
   ;}
    break;

  case 255:
#line 1239 "cf-parse.y"
    {
     (yyval.e) = f_new_tree(); 
     (yyval.e)->from.type = T_VOID; 
     (yyval.e)->to.type = T_VOID;
     (yyval.e)->data = (yyvsp[(4) - (4)].x);
     (yyval.e)->left = (yyvsp[(1) - (4)].e);
   ;}
    break;

  case 256:
#line 1251 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 257:
#line 1252 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 258:
#line 1256 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 259:
#line 1257 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 260:
#line 1261 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 261:
#line 1262 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 262:
#line 1263 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 263:
#line 1264 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 264:
#line 1265 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 265:
#line 1269 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 266:
#line 1270 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 267:
#line 1271 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 268:
#line 1275 "cf-parse.y"
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

  case 269:
#line 1288 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 270:
#line 1289 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 271:
#line 1290 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 272:
#line 1291 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 273:
#line 1292 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 274:
#line 1293 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 275:
#line 1294 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i32); ;}
    break;

  case 276:
#line 1295 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 277:
#line 1296 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 278:
#line 1297 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 279:
#line 1298 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 281:
#line 1312 "cf-parse.y"
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

  case 282:
#line 1335 "cf-parse.y"
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

  case 283:
#line 1369 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 284:
#line 1371 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 285:
#line 1372 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 286:
#line 1373 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 287:
#line 1374 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 288:
#line 1375 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 289:
#line 1376 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 290:
#line 1377 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 291:
#line 1381 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 292:
#line 1382 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 293:
#line 1383 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 294:
#line 1384 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 295:
#line 1385 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 296:
#line 1386 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 297:
#line 1387 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 298:
#line 1388 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 299:
#line 1389 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 300:
#line 1390 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 301:
#line 1391 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 302:
#line 1392 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 303:
#line 1393 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 304:
#line 1394 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 305:
#line 1395 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 306:
#line 1396 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 307:
#line 1398 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 308:
#line 1399 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 309:
#line 1400 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 310:
#line 1402 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 311:
#line 1404 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 312:
#line 1406 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 313:
#line 1408 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 314:
#line 1409 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 315:
#line 1410 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 316:
#line 1411 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 317:
#line 1412 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 318:
#line 1422 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 319:
#line 1423 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 320:
#line 1424 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 321:
#line 1425 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 322:
#line 1426 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 323:
#line 1431 "cf-parse.y"
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

  case 324:
#line 1454 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 325:
#line 1455 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 326:
#line 1456 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 327:
#line 1457 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 328:
#line 1458 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 329:
#line 1459 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 330:
#line 1463 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 331:
#line 1466 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 332:
#line 1467 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 333:
#line 1468 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 334:
#line 1477 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 335:
#line 1484 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 336:
#line 1493 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 337:
#line 1494 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 338:
#line 1498 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 339:
#line 1504 "cf-parse.y"
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

  case 340:
#line 1514 "cf-parse.y"
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

  case 341:
#line 1523 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 342:
#line 1529 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 343:
#line 1534 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 344:
#line 1541 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 345:
#line 1546 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 346:
#line 1552 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 347:
#line 1553 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 348:
#line 1554 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 349:
#line 1563 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 350:
#line 1564 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 351:
#line 1565 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 352:
#line 1566 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 353:
#line 1572 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_bgp, sizeof(struct bgp_config));
     this_proto->preference = DEF_PREF_BGP;
     BGP_CFG->hold_time = 240;
     BGP_CFG->connect_retry_time = 120;
     BGP_CFG->initial_hold_time = 240;
     BGP_CFG->compare_path_lengths = 1;
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

  case 356:
#line 1595 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 357:
#line 1596 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 358:
#line 1602 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i32); ;}
    break;

  case 359:
#line 1603 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 360:
#line 1604 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 361:
#line 1605 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 362:
#line 1606 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 363:
#line 1607 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 364:
#line 1608 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 365:
#line 1609 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (6)].i); BGP_CFG->multihop_via = (yyvsp[(5) - (6)].a); ;}
    break;

  case 366:
#line 1610 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 367:
#line 1611 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; ;}
    break;

  case 368:
#line 1612 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; ;}
    break;

  case 369:
#line 1613 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; ;}
    break;

  case 370:
#line 1614 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 371:
#line 1615 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); ;}
    break;

  case 372:
#line 1616 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 373:
#line 1617 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 374:
#line 1618 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 375:
#line 1619 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 376:
#line 1620 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 377:
#line 1621 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 378:
#line 1622 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 379:
#line 1623 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); ;}
    break;

  case 380:
#line 1624 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 381:
#line 1625 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 382:
#line 1626 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 383:
#line 1627 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 384:
#line 1628 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 385:
#line 1629 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); ;}
    break;

  case 386:
#line 1630 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); ;}
    break;

  case 387:
#line 1640 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 391:
#line 1656 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 392:
#line 1657 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i) ; if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 394:
#line 1661 "cf-parse.y"
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

  case 398:
#line 1682 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 399:
#line 1683 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 406:
#line 1696 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 409:
#line 1710 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 410:
#line 1711 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 411:
#line 1712 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 412:
#line 1716 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 417:
#line 1726 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 418:
#line 1727 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 419:
#line 1728 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 420:
#line 1729 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 421:
#line 1730 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 422:
#line 1731 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 423:
#line 1732 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 424:
#line 1733 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 425:
#line 1734 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 427:
#line 1739 "cf-parse.y"
    {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->vid = (yyvsp[(3) - (3)].i32);
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

  case 428:
#line 1759 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 429:
#line 1760 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 430:
#line 1761 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 431:
#line 1762 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 432:
#line 1763 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 433:
#line 1764 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 434:
#line 1765 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 435:
#line 1766 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 436:
#line 1767 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 437:
#line 1768 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 438:
#line 1769 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 439:
#line 1770 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 440:
#line 1771 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 441:
#line 1772 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 443:
#line 1774 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 444:
#line 1775 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 445:
#line 1776 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 446:
#line 1777 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 447:
#line 1778 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 448:
#line 1779 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) cf_error("Buffer size is too small") ; ;}
    break;

  case 454:
#line 1793 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 455:
#line 1802 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 460:
#line 1821 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 461:
#line 1830 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 462:
#line 1840 "cf-parse.y"
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

  case 467:
#line 1873 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 469:
#line 1878 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 470:
#line 1883 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 471:
#line 1886 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 472:
#line 1889 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 473:
#line 1894 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0, 1); ;}
    break;

  case 474:
#line 1897 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 0, 0); ;}
    break;

  case 475:
#line 1902 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1, 1); ;}
    break;

  case 476:
#line 1905 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(5) - (7)].s), &proto_ospf), 1, 0); ;}
    break;

  case 477:
#line 1908 "cf-parse.y"
    { ospf_sh_lsadb(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf)); ;}
    break;

  case 478:
#line 1913 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  ;}
    break;

  case 481:
#line 1923 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 482:
#line 1928 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 483:
#line 1929 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 484:
#line 1935 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 487:
#line 1944 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 488:
#line 1945 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 489:
#line 1946 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 490:
#line 1947 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 491:
#line 1948 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 492:
#line 1949 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 494:
#line 1951 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 495:
#line 1952 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 496:
#line 1953 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 498:
#line 1958 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 499:
#line 1959 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 500:
#line 1960 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 501:
#line 1965 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 502:
#line 1966 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 503:
#line 1967 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 504:
#line 1968 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 505:
#line 1969 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 507:
#line 1973 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 508:
#line 1974 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 513:
#line 1988 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 515:
#line 2004 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 519:
#line 2016 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&((struct static_config *) this_proto)->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 520:
#line 2025 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 521:
#line 2029 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&((struct static_config *) this_proto)->iface_routes, &this_srt->n);
   ;}
    break;

  case 522:
#line 2035 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 523:
#line 2036 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 524:
#line 2037 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 525:
#line 2041 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 579:
#line 2049 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 582:
#line 2049 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 591:
#line 2052 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 592:
#line 2053 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 593:
#line 2054 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 594:
#line 2055 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 595:
#line 2056 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 596:
#line 2057 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 597:
#line 2058 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 598:
#line 2059 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 599:
#line 2060 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 600:
#line 2061 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); ;}
    break;

  case 601:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); ;}
    break;

  case 602:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 603:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 604:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 605:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); ;}
    break;

  case 606:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 607:
#line 2062 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 6177 "cf-parse.tab.c"
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
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
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

  if (yyn == YYFINAL)
    YYACCEPT;

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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
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


#line 2064 "cf-parse.y"

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



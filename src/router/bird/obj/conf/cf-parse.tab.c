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
     CONFIGURE = 299,
     DOWN = 300,
     KERNEL = 301,
     PERSIST = 302,
     SCAN = 303,
     TIME = 304,
     LEARN = 305,
     DEVICE = 306,
     ASYNC = 307,
     TABLE = 308,
     ROUTER = 309,
     ID = 310,
     PROTOCOL = 311,
     PREFERENCE = 312,
     DISABLED = 313,
     DIRECT = 314,
     INTERFACE = 315,
     IMPORT = 316,
     EXPORT = 317,
     FILTER = 318,
     NONE = 319,
     STATES = 320,
     ROUTES = 321,
     FILTERS = 322,
     PASSWORD = 323,
     FROM = 324,
     PASSIVE = 325,
     TO = 326,
     EVENTS = 327,
     PACKETS = 328,
     PROTOCOLS = 329,
     INTERFACES = 330,
     PRIMARY = 331,
     STATS = 332,
     COUNT = 333,
     FOR = 334,
     COMMANDS = 335,
     PREEXPORT = 336,
     GENERATE = 337,
     LISTEN = 338,
     BGP = 339,
     V6ONLY = 340,
     ADDRESS = 341,
     PORT = 342,
     PASSWORDS = 343,
     DESCRIPTION = 344,
     RELOAD = 345,
     IN = 346,
     OUT = 347,
     MRTDUMP = 348,
     MESSAGES = 349,
     SHOW = 350,
     STATUS = 351,
     SUMMARY = 352,
     ROUTE = 353,
     SYMBOLS = 354,
     DUMP = 355,
     RESOURCES = 356,
     SOCKETS = 357,
     NEIGHBORS = 358,
     ATTRIBUTES = 359,
     ECHO = 360,
     DISABLE = 361,
     ENABLE = 362,
     RESTART = 363,
     FUNCTION = 364,
     PRINT = 365,
     PRINTN = 366,
     UNSET = 367,
     RETURN = 368,
     ACCEPT = 369,
     REJECT = 370,
     QUITBIRD = 371,
     INT = 372,
     BOOL = 373,
     IP = 374,
     PREFIX = 375,
     PAIR = 376,
     SET = 377,
     STRING = 378,
     BGPMASK = 379,
     BGPPATH = 380,
     CLIST = 381,
     IF = 382,
     THEN = 383,
     ELSE = 384,
     CASE = 385,
     TRUE = 386,
     FALSE = 387,
     GW = 388,
     NET = 389,
     MASK = 390,
     PROTO = 391,
     SOURCE = 392,
     SCOPE = 393,
     CAST = 394,
     DEST = 395,
     LEN = 396,
     DEFINED = 397,
     ADD = 398,
     DELETE = 399,
     CONTAINS = 400,
     RESET = 401,
     PREPEND = 402,
     FIRST = 403,
     LAST = 404,
     MATCH = 405,
     EMPTY = 406,
     WHERE = 407,
     EVAL = 408,
     LOCAL = 409,
     NEIGHBOR = 410,
     AS = 411,
     HOLD = 412,
     CONNECT = 413,
     RETRY = 414,
     KEEPALIVE = 415,
     MULTIHOP = 416,
     STARTUP = 417,
     VIA = 418,
     NEXT = 419,
     HOP = 420,
     SELF = 421,
     DEFAULT = 422,
     PATH = 423,
     METRIC = 424,
     START = 425,
     DELAY = 426,
     FORGET = 427,
     WAIT = 428,
     AFTER = 429,
     BGP_PATH = 430,
     BGP_LOCAL_PREF = 431,
     BGP_MED = 432,
     BGP_ORIGIN = 433,
     BGP_NEXT_HOP = 434,
     BGP_ATOMIC_AGGR = 435,
     BGP_AGGREGATOR = 436,
     BGP_COMMUNITY = 437,
     RR = 438,
     RS = 439,
     CLIENT = 440,
     CLUSTER = 441,
     AS4 = 442,
     ADVERTISE = 443,
     IPV4 = 444,
     CAPABILITIES = 445,
     LIMIT = 446,
     PREFER = 447,
     OLDER = 448,
     MISSING = 449,
     LLADDR = 450,
     DROP = 451,
     IGNORE = 452,
     REFRESH = 453,
     INTERPRET = 454,
     COMMUNITIES = 455,
     OSPF = 456,
     AREA = 457,
     OSPF_METRIC1 = 458,
     OSPF_METRIC2 = 459,
     OSPF_TAG = 460,
     OSPF_ROUTER_ID = 461,
     BROADCAST = 462,
     RFC1583COMPAT = 463,
     STUB = 464,
     TICK = 465,
     COST = 466,
     RETRANSMIT = 467,
     HELLO = 468,
     TRANSMIT = 469,
     PRIORITY = 470,
     DEAD = 471,
     NONBROADCAST = 472,
     POINTOPOINT = 473,
     TYPE = 474,
     SIMPLE = 475,
     AUTHENTICATION = 476,
     STRICT = 477,
     CRYPTOGRAPHIC = 478,
     ELIGIBLE = 479,
     POLL = 480,
     NETWORKS = 481,
     HIDDEN = 482,
     VIRTUAL = 483,
     LINK = 484,
     RX = 485,
     BUFFER = 486,
     LARGE = 487,
     NORMAL = 488,
     STUBNET = 489,
     LSADB = 490,
     TOPOLOGY = 491,
     STATE = 492,
     PIPE = 493,
     PEER = 494,
     MODE = 495,
     OPAQUE = 496,
     TRANSPARENT = 497,
     RIP = 498,
     INFINITY = 499,
     PERIOD = 500,
     GARBAGE = 501,
     TIMEOUT = 502,
     MULTICAST = 503,
     QUIET = 504,
     NOLISTEN = 505,
     VERSION1 = 506,
     PLAINTEXT = 507,
     MD5 = 508,
     HONOR = 509,
     NEVER = 510,
     ALWAYS = 511,
     RIP_METRIC = 512,
     RIP_TAG = 513,
     STATIC = 514,
     PROHIBIT = 515
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
#define CONFIGURE 299
#define DOWN 300
#define KERNEL 301
#define PERSIST 302
#define SCAN 303
#define TIME 304
#define LEARN 305
#define DEVICE 306
#define ASYNC 307
#define TABLE 308
#define ROUTER 309
#define ID 310
#define PROTOCOL 311
#define PREFERENCE 312
#define DISABLED 313
#define DIRECT 314
#define INTERFACE 315
#define IMPORT 316
#define EXPORT 317
#define FILTER 318
#define NONE 319
#define STATES 320
#define ROUTES 321
#define FILTERS 322
#define PASSWORD 323
#define FROM 324
#define PASSIVE 325
#define TO 326
#define EVENTS 327
#define PACKETS 328
#define PROTOCOLS 329
#define INTERFACES 330
#define PRIMARY 331
#define STATS 332
#define COUNT 333
#define FOR 334
#define COMMANDS 335
#define PREEXPORT 336
#define GENERATE 337
#define LISTEN 338
#define BGP 339
#define V6ONLY 340
#define ADDRESS 341
#define PORT 342
#define PASSWORDS 343
#define DESCRIPTION 344
#define RELOAD 345
#define IN 346
#define OUT 347
#define MRTDUMP 348
#define MESSAGES 349
#define SHOW 350
#define STATUS 351
#define SUMMARY 352
#define ROUTE 353
#define SYMBOLS 354
#define DUMP 355
#define RESOURCES 356
#define SOCKETS 357
#define NEIGHBORS 358
#define ATTRIBUTES 359
#define ECHO 360
#define DISABLE 361
#define ENABLE 362
#define RESTART 363
#define FUNCTION 364
#define PRINT 365
#define PRINTN 366
#define UNSET 367
#define RETURN 368
#define ACCEPT 369
#define REJECT 370
#define QUITBIRD 371
#define INT 372
#define BOOL 373
#define IP 374
#define PREFIX 375
#define PAIR 376
#define SET 377
#define STRING 378
#define BGPMASK 379
#define BGPPATH 380
#define CLIST 381
#define IF 382
#define THEN 383
#define ELSE 384
#define CASE 385
#define TRUE 386
#define FALSE 387
#define GW 388
#define NET 389
#define MASK 390
#define PROTO 391
#define SOURCE 392
#define SCOPE 393
#define CAST 394
#define DEST 395
#define LEN 396
#define DEFINED 397
#define ADD 398
#define DELETE 399
#define CONTAINS 400
#define RESET 401
#define PREPEND 402
#define FIRST 403
#define LAST 404
#define MATCH 405
#define EMPTY 406
#define WHERE 407
#define EVAL 408
#define LOCAL 409
#define NEIGHBOR 410
#define AS 411
#define HOLD 412
#define CONNECT 413
#define RETRY 414
#define KEEPALIVE 415
#define MULTIHOP 416
#define STARTUP 417
#define VIA 418
#define NEXT 419
#define HOP 420
#define SELF 421
#define DEFAULT 422
#define PATH 423
#define METRIC 424
#define START 425
#define DELAY 426
#define FORGET 427
#define WAIT 428
#define AFTER 429
#define BGP_PATH 430
#define BGP_LOCAL_PREF 431
#define BGP_MED 432
#define BGP_ORIGIN 433
#define BGP_NEXT_HOP 434
#define BGP_ATOMIC_AGGR 435
#define BGP_AGGREGATOR 436
#define BGP_COMMUNITY 437
#define RR 438
#define RS 439
#define CLIENT 440
#define CLUSTER 441
#define AS4 442
#define ADVERTISE 443
#define IPV4 444
#define CAPABILITIES 445
#define LIMIT 446
#define PREFER 447
#define OLDER 448
#define MISSING 449
#define LLADDR 450
#define DROP 451
#define IGNORE 452
#define REFRESH 453
#define INTERPRET 454
#define COMMUNITIES 455
#define OSPF 456
#define AREA 457
#define OSPF_METRIC1 458
#define OSPF_METRIC2 459
#define OSPF_TAG 460
#define OSPF_ROUTER_ID 461
#define BROADCAST 462
#define RFC1583COMPAT 463
#define STUB 464
#define TICK 465
#define COST 466
#define RETRANSMIT 467
#define HELLO 468
#define TRANSMIT 469
#define PRIORITY 470
#define DEAD 471
#define NONBROADCAST 472
#define POINTOPOINT 473
#define TYPE 474
#define SIMPLE 475
#define AUTHENTICATION 476
#define STRICT 477
#define CRYPTOGRAPHIC 478
#define ELIGIBLE 479
#define POLL 480
#define NETWORKS 481
#define HIDDEN 482
#define VIRTUAL 483
#define LINK 484
#define RX 485
#define BUFFER 486
#define LARGE 487
#define NORMAL 488
#define STUBNET 489
#define LSADB 490
#define TOPOLOGY 491
#define STATE 492
#define PIPE 493
#define PEER 494
#define MODE 495
#define OPAQUE 496
#define TRANSPARENT 497
#define RIP 498
#define INFINITY 499
#define PERIOD 500
#define GARBAGE 501
#define TIMEOUT 502
#define MULTICAST 503
#define QUIET 504
#define NOLISTEN 505
#define VERSION1 506
#define PLAINTEXT 507
#define MD5 508
#define HONOR 509
#define NEVER 510
#define ALWAYS 511
#define RIP_METRIC 512
#define RIP_TAG 513
#define STATIC 514
#define PROHIBIT 515




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
  struct timeformat *tf;
}
/* Line 187 of yacc.c.  */
#line 799 "cf-parse.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 812 "cf-parse.tab.c"

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
#define YYFINAL  49
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1629

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  282
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  193
/* YYNRULES -- Number of rules.  */
#define YYNRULES  587
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1097

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   515

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,     2,    29,     2,     2,
     272,   273,    27,    25,   278,    26,    24,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   275,   274,
      21,    20,    22,   279,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   280,     2,   281,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   276,     2,   277,    23,     2,     2,     2,
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
     266,   267,   268,   269,   270,   271
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
      99,   101,   103,   105,   107,   112,   116,   118,   120,   122,
     124,   127,   132,   136,   140,   144,   148,   153,   156,   157,
     159,   162,   165,   169,   172,   175,   179,   183,   187,   192,
     194,   196,   198,   203,   204,   207,   210,   213,   215,   218,
     220,   221,   223,   224,   227,   230,   233,   236,   239,   242,
     245,   249,   252,   255,   257,   259,   261,   263,   267,   271,
     272,   274,   276,   279,   280,   282,   286,   288,   292,   295,
     299,   303,   307,   308,   312,   314,   316,   320,   322,   326,
     328,   330,   332,   334,   336,   338,   340,   342,   346,   348,
     352,   354,   356,   361,   363,   364,   368,   373,   375,   378,
     379,   385,   391,   397,   403,   408,   412,   417,   423,   425,
     426,   430,   435,   440,   441,   444,   448,   452,   456,   459,
     462,   465,   469,   473,   476,   479,   481,   483,   488,   492,
     496,   500,   504,   508,   512,   516,   521,   523,   525,   527,
     528,   530,   534,   538,   542,   546,   551,   556,   561,   566,
     568,   570,   572,   573,   578,   581,   583,   585,   587,   589,
     591,   593,   595,   597,   599,   602,   605,   606,   610,   612,
     616,   618,   620,   622,   625,   629,   632,   637,   638,   644,
     645,   647,   649,   652,   654,   658,   664,   666,   668,   670,
     672,   674,   676,   681,   683,   687,   691,   693,   696,   699,
     706,   708,   712,   713,   718,   722,   724,   728,   732,   736,
     739,   742,   745,   748,   749,   752,   755,   756,   762,   764,
     766,   768,   770,   772,   774,   778,   782,   784,   786,   787,
     792,   794,   796,   798,   800,   802,   804,   806,   808,   810,
     814,   818,   822,   826,   830,   834,   838,   842,   846,   850,
     854,   858,   862,   866,   869,   874,   876,   878,   880,   882,
     885,   888,   892,   896,   903,   907,   911,   915,   919,   926,
     933,   940,   945,   947,   949,   951,   953,   955,   957,   959,
     960,   962,   966,   968,   972,   973,   975,   980,   987,   992,
     996,  1002,  1008,  1013,  1020,  1024,  1027,  1033,  1039,  1048,
    1057,  1066,  1069,  1073,  1077,  1083,  1090,  1097,  1102,  1107,
    1113,  1120,  1127,  1133,  1140,  1146,  1152,  1158,  1164,  1170,
    1176,  1182,  1188,  1194,  1201,  1208,  1217,  1224,  1231,  1237,
    1242,  1248,  1253,  1259,  1264,  1270,  1273,  1277,  1281,  1283,
    1286,  1289,  1292,  1296,  1299,  1300,  1304,  1308,  1311,  1316,
    1319,  1322,  1324,  1329,  1331,  1333,  1334,  1338,  1341,  1344,
    1347,  1352,  1354,  1355,  1359,  1360,  1363,  1366,  1370,  1373,
    1376,  1380,  1383,  1386,  1389,  1391,  1395,  1398,  1401,  1404,
    1407,  1411,  1414,  1417,  1420,  1424,  1427,  1430,  1433,  1437,
    1440,  1445,  1448,  1451,  1454,  1458,  1462,  1466,  1468,  1469,
    1472,  1474,  1476,  1479,  1483,  1484,  1487,  1489,  1491,  1494,
    1498,  1499,  1500,  1504,  1505,  1509,  1513,  1515,  1516,  1521,
    1528,  1535,  1542,  1549,  1556,  1559,  1563,  1567,  1573,  1578,
    1583,  1586,  1590,  1594,  1599,  1604,  1609,  1615,  1621,  1626,
    1630,  1635,  1640,  1645,  1650,  1652,  1654,  1656,  1658,  1660,
    1662,  1664,  1666,  1667,  1670,  1673,  1674,  1678,  1679,  1683,
    1684,  1688,  1691,  1695,  1699,  1703,  1706,  1710,  1714,  1717,
    1720,  1723,  1728,  1730,  1732,  1734,  1736,  1738,  1740,  1742,
    1744,  1746,  1748,  1750,  1752,  1754,  1756,  1758,  1760,  1762,
    1764,  1766,  1768,  1770,  1772,  1774,  1776,  1778,  1780,  1782,
    1784,  1786,  1788,  1790,  1792,  1794,  1796,  1798,  1800,  1802,
    1804,  1806,  1808,  1810,  1812,  1814,  1816,  1818,  1820,  1823,
    1826,  1829,  1832,  1835,  1838,  1841,  1844,  1848,  1852,  1856,
    1860,  1864,  1868,  1872,  1874,  1876,  1878,  1880,  1882,  1884,
    1886,  1888,  1890,  1892,  1894,  1896,  1898,  1900
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     283,     0,    -1,   284,     3,    -1,     4,   470,    -1,    -1,
     284,   469,    -1,    13,    -1,   272,   409,   273,    -1,    17,
      -1,    31,    17,    20,   285,   274,    -1,    31,    17,    20,
      16,   274,    -1,   285,    -1,    32,    -1,    34,    -1,    33,
      -1,    35,    -1,    -1,    16,    -1,    17,    -1,   288,   291,
      -1,   289,    -1,   288,    -1,    28,   285,    -1,   275,   288,
      -1,    18,    -1,    18,    -1,    -1,    36,   295,   296,   274,
      -1,    18,    -1,    37,    -1,    48,    -1,    38,    -1,   276,
     297,   277,    -1,   298,    -1,   297,   278,   298,    -1,    39,
      -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,
      -1,    45,    -1,    46,    -1,    47,    -1,   104,    85,   336,
     274,    -1,   104,    18,   274,    -1,   109,    -1,    67,    -1,
      54,    -1,    36,    -1,   300,    18,    -1,   300,    18,   285,
      18,    -1,   300,    51,    52,    -1,   300,    51,    53,    -1,
      50,   301,   274,    -1,    55,   306,     3,    -1,    55,    49,
     306,     3,    -1,    56,     3,    -1,    -1,    18,    -1,   318,
      57,    -1,    58,   287,    -1,    59,    60,   285,    -1,    61,
     287,    -1,   318,    62,    -1,    59,    60,   285,    -1,    87,
     293,   290,    -1,    57,    64,   285,    -1,    65,    66,   313,
     274,    -1,    13,    -1,    15,    -1,    16,    -1,    94,    95,
     315,   274,    -1,    -1,   315,   316,    -1,    97,   288,    -1,
      98,   285,    -1,    96,    -1,    64,    17,    -1,    67,    -1,
      -1,    17,    -1,    -1,    68,   285,    -1,    69,   287,    -1,
      39,   333,    -1,   104,   336,    -1,    72,   321,    -1,    73,
     321,    -1,    64,   322,    -1,    65,    66,   313,    -1,   100,
      18,    -1,    74,   381,    -1,   382,    -1,    38,    -1,    75,
      -1,    17,    -1,    39,    85,   333,    -1,    39,    91,   285,
      -1,    -1,    18,    -1,   289,    -1,    18,   289,    -1,    -1,
      26,    -1,   324,   326,   325,    -1,   327,    -1,   328,   278,
     327,    -1,   318,    70,    -1,   329,   319,   276,    -1,   330,
     320,   274,    -1,   330,   332,   274,    -1,    -1,    71,   331,
     328,    -1,    38,    -1,    33,    -1,   276,   334,   277,    -1,
     335,    -1,   334,   278,   335,    -1,    76,    -1,    77,    -1,
      78,    -1,    86,    -1,    83,    -1,    84,    -1,    38,    -1,
      33,    -1,   276,   337,   277,    -1,   338,    -1,   337,   278,
     338,    -1,    76,    -1,   105,    -1,    99,   276,   340,   277,
      -1,   341,    -1,    -1,   341,   274,   340,    -1,   342,   276,
     343,   277,    -1,   342,    -1,    79,    18,    -1,    -1,    93,
      80,   292,   274,   343,    -1,    93,    82,   292,   274,   343,
      -1,   125,    80,   292,   274,   343,    -1,   125,    82,   292,
     274,   343,    -1,    66,   285,   274,   343,    -1,   106,   107,
       3,    -1,   106,    85,   347,     3,    -1,   106,    85,    38,
     347,     3,    -1,    17,    -1,    -1,   106,    86,     3,    -1,
     106,    86,   108,     3,    -1,   106,   109,   351,     3,    -1,
      -1,   351,   289,    -1,   351,    90,   290,    -1,   351,    64,
      17,    -1,   351,    74,   381,    -1,   351,   382,    -1,   351,
      38,    -1,   351,    87,    -1,   351,   352,    17,    -1,   351,
      67,    17,    -1,   351,    88,    -1,   351,    89,    -1,    92,
      -1,    73,    -1,   106,   110,   347,     3,    -1,   111,   112,
       3,    -1,   111,   113,     3,    -1,   111,    86,     3,    -1,
     111,   114,     3,    -1,   111,   115,     3,    -1,   111,    77,
       3,    -1,   111,    85,     3,    -1,   116,   362,   363,     3,
      -1,    38,    -1,    33,    -1,    13,    -1,    -1,    13,    -1,
     117,   372,     3,    -1,   118,   372,     3,    -1,   119,   372,
       3,    -1,   101,   372,     3,    -1,   101,   102,   372,     3,
      -1,   101,   103,   372,     3,    -1,    39,   372,   333,     3,
      -1,   104,   372,   336,     3,    -1,    17,    -1,    38,    -1,
      18,    -1,    -1,    74,    17,   374,   380,    -1,   164,   409,
      -1,   128,    -1,   129,    -1,   130,    -1,   131,    -1,   132,
      -1,   134,    -1,   135,    -1,   136,    -1,   137,    -1,   376,
     133,    -1,   376,    17,    -1,    -1,   377,   274,   378,    -1,
     377,    -1,   379,   274,   377,    -1,   384,    -1,    17,    -1,
     380,    -1,   163,   409,    -1,   272,   379,   273,    -1,   272,
     273,    -1,   378,   276,   387,   277,    -1,    -1,   120,    17,
     386,   383,   384,    -1,    -1,   388,    -1,   415,    -1,   388,
     415,    -1,   415,    -1,   276,   387,   277,    -1,   272,    13,
     278,    13,   273,    -1,    16,    -1,    13,    -1,   390,    -1,
     391,    -1,    14,    -1,   392,    -1,   392,    24,    24,   392,
      -1,   393,    -1,   394,   278,   393,    -1,    16,    28,    13,
      -1,   395,    -1,   395,    25,    -1,   395,    26,    -1,   395,
     276,    13,   278,    13,   277,    -1,   396,    -1,   397,   278,
     396,    -1,    -1,   393,   275,   387,   398,    -1,   140,   275,
     387,    -1,   407,    -1,   272,   409,   273,    -1,    11,   401,
      12,    -1,    28,   402,    28,    -1,    13,   401,    -1,    27,
     401,    -1,   279,   401,    -1,   399,   401,    -1,    -1,    13,
     402,    -1,   279,   402,    -1,    -1,   272,   409,   278,   409,
     273,    -1,    13,    -1,   142,    -1,   143,    -1,    18,    -1,
     391,    -1,   395,    -1,   280,   394,   281,    -1,   280,   397,
     281,    -1,    14,    -1,   400,    -1,    -1,    17,   272,   414,
     273,    -1,    17,    -1,    80,    -1,   144,    -1,   145,    -1,
     147,    -1,   148,    -1,   149,    -1,   150,    -1,   151,    -1,
     272,   409,   273,    -1,   409,    25,   409,    -1,   409,    26,
     409,    -1,   409,    27,   409,    -1,   409,    28,   409,    -1,
     409,     9,   409,    -1,   409,    10,   409,    -1,   409,    20,
     409,    -1,   409,     8,   409,    -1,   409,    21,   409,    -1,
     409,     7,   409,    -1,   409,    22,   409,    -1,   409,     6,
     409,    -1,   409,    23,   409,    -1,    30,   409,    -1,   153,
     272,   409,   273,    -1,   407,    -1,   404,    -1,   403,    -1,
      68,    -1,   405,   408,    -1,   405,   474,    -1,   409,    24,
     130,    -1,   409,    24,   152,    -1,   409,    24,   146,   272,
     409,   273,    -1,   409,    24,   159,    -1,   409,    24,   160,
      -1,    25,   162,    25,    -1,    26,   162,    26,    -1,   158,
     272,   409,   278,   409,   273,    -1,   154,   272,   409,   278,
     409,   273,    -1,   155,   272,   409,   278,   409,   273,    -1,
      17,   272,   414,   273,    -1,   127,    -1,   125,    -1,   126,
      -1,    44,    -1,   121,    -1,   122,    -1,   409,    -1,    -1,
     411,    -1,   411,   278,   412,    -1,   409,    -1,   409,   278,
     413,    -1,    -1,   413,    -1,   138,   409,   139,   389,    -1,
     138,   409,   139,   389,   140,   389,    -1,    17,    20,   409,
     274,    -1,   124,   409,   274,    -1,   405,   474,    20,   409,
     274,    -1,   405,   408,    20,   409,   274,    -1,    68,    20,
     409,   274,    -1,   123,   272,   405,   474,   273,   274,    -1,
     410,   412,   274,    -1,   406,   274,    -1,   141,   409,   276,
     398,   277,    -1,   405,   474,    24,   162,   274,    -1,   405,
     474,    24,   158,   272,   409,   273,   274,    -1,   405,   474,
      24,   154,   272,   409,   273,   274,    -1,   405,   474,    24,
     155,   272,   409,   273,   274,    -1,   318,    95,    -1,   416,
     319,   276,    -1,   417,   320,   274,    -1,   417,   165,   167,
     285,   274,    -1,   417,   166,   288,   167,   285,   274,    -1,
     417,   194,   197,    66,   285,   274,    -1,   417,   194,   196,
     274,    -1,   417,   195,   196,   274,    -1,   417,   168,    60,
     285,   274,    -1,   417,   173,   168,    60,   285,   274,    -1,
     417,   169,   170,    60,   285,   274,    -1,   417,   171,    60,
     285,   274,    -1,   417,   172,   285,   174,   288,   274,    -1,
     417,   175,   176,   177,   274,    -1,   417,   205,   206,   177,
     274,    -1,   417,   205,   206,   207,   274,    -1,   417,   205,
     206,   208,   274,    -1,   417,   179,   180,   287,   274,    -1,
     417,   203,   204,   287,   274,    -1,   417,   178,   188,   285,
     274,    -1,   417,   178,   187,   285,   274,    -1,   417,   148,
      97,   288,   274,    -1,   417,   181,   182,    60,   285,   274,
      -1,   417,    44,   183,    60,   285,   274,    -1,   417,    44,
     184,    60,   285,   278,   285,   274,    -1,   417,   117,   185,
      44,   287,   274,    -1,   417,   118,   109,   209,   287,   274,
      -1,   417,   118,   198,   287,   274,    -1,   417,   201,   287,
     274,    -1,   417,   199,   200,   287,   274,    -1,   417,    79,
      18,   274,    -1,   417,   109,   202,   285,   274,    -1,   417,
      81,   287,   274,    -1,   417,   210,   211,   287,   274,    -1,
     318,   212,    -1,   418,   319,   276,    -1,   419,   420,   274,
      -1,   320,    -1,   219,   287,    -1,   221,   285,    -1,   422,
     277,    -1,   213,   313,   276,    -1,   421,   423,    -1,    -1,
     423,   424,   274,    -1,   220,   222,   285,    -1,   220,   287,
      -1,   237,   276,   434,   277,    -1,   245,   425,    -1,    71,
     445,    -1,   429,    -1,   426,   276,   427,   277,    -1,   426,
      -1,   289,    -1,    -1,   427,   428,   274,    -1,   238,   287,
      -1,   108,   287,    -1,   222,   285,    -1,   432,   276,   430,
     277,    -1,   432,    -1,    -1,   430,   431,   274,    -1,    -1,
     224,   285,    -1,   223,   285,    -1,   225,   182,   285,    -1,
     184,   285,    -1,   227,   285,    -1,   227,    89,   285,    -1,
     232,    75,    -1,   232,   231,    -1,   232,   234,    -1,   339,
      -1,   239,   240,   313,    -1,   222,   285,    -1,   224,   285,
      -1,   236,   285,    -1,   223,   285,    -1,   225,   182,   285,
      -1,   226,   285,    -1,   184,   285,    -1,   227,   285,    -1,
     227,    89,   285,    -1,   230,   218,    -1,   230,   228,    -1,
     230,   229,    -1,   233,   228,   287,    -1,   220,   287,    -1,
     114,   276,   438,   277,    -1,   232,    75,    -1,   232,   231,
      -1,   232,   234,    -1,   241,   242,   243,    -1,   241,   242,
     244,    -1,   241,   242,   285,    -1,   339,    -1,    -1,   434,
     435,    -1,   436,    -1,   437,    -1,   289,   274,    -1,   289,
     238,   274,    -1,    -1,   438,   439,    -1,   440,    -1,   441,
      -1,    16,   274,    -1,    16,   235,   274,    -1,    -1,    -1,
     443,   433,   274,    -1,    -1,   276,   443,   277,    -1,   442,
     328,   444,    -1,    18,    -1,    -1,   106,   212,   347,     3,
      -1,   106,   212,   114,   347,   446,     3,    -1,   106,   212,
      71,   347,   446,     3,    -1,   106,   212,   247,   347,   446,
       3,    -1,   106,   212,   248,   347,   446,     3,    -1,   106,
     212,   246,   347,   446,     3,    -1,   318,   249,    -1,   453,
     319,   276,    -1,   454,   320,   274,    -1,   454,   250,    64,
      17,   274,    -1,   454,   251,   252,   274,    -1,   454,   251,
     253,   274,    -1,   318,   254,    -1,   455,   319,   276,    -1,
     456,   320,   274,    -1,   456,   255,   285,   274,    -1,   456,
      98,   285,   274,    -1,   456,   256,   285,   274,    -1,   456,
     257,    60,   285,   274,    -1,   456,   258,    60,   285,   274,
      -1,   456,   232,   457,   274,    -1,   456,   339,   274,    -1,
     456,   265,   267,   274,    -1,   456,   265,   166,   274,    -1,
     456,   265,   266,   274,    -1,   456,    71,   463,   274,    -1,
     263,    -1,   264,    -1,    75,    -1,   218,    -1,   259,    -1,
     260,    -1,   261,    -1,   262,    -1,    -1,   180,   285,    -1,
     251,   458,    -1,    -1,   460,   459,   274,    -1,    -1,   276,
     460,   277,    -1,    -1,   462,   328,   461,    -1,   318,   270,
      -1,   464,   319,   276,    -1,   465,   320,   274,    -1,   465,
     467,   274,    -1,   109,   289,    -1,   466,   174,   288,    -1,
     466,   174,    18,    -1,   466,   207,    -1,   466,   126,    -1,
     466,   271,    -1,   106,   270,   347,     3,    -1,   274,    -1,
     286,    -1,   294,    -1,   299,    -1,   302,    -1,   312,    -1,
     314,    -1,   317,    -1,   471,    -1,   323,    -1,   373,    -1,
     375,    -1,   385,    -1,   303,    -1,   304,    -1,   305,    -1,
     344,    -1,   345,    -1,   346,    -1,   348,    -1,   349,    -1,
     350,    -1,   353,    -1,   354,    -1,   355,    -1,   356,    -1,
     357,    -1,   358,    -1,   359,    -1,   360,    -1,   361,    -1,
     364,    -1,   365,    -1,   366,    -1,   367,    -1,   368,    -1,
     369,    -1,   370,    -1,   371,    -1,   447,    -1,   448,    -1,
     449,    -1,   450,    -1,   451,    -1,   452,    -1,   468,    -1,
     472,   277,    -1,   473,   277,    -1,   330,   277,    -1,   417,
     277,    -1,   419,   277,    -1,   454,   277,    -1,   456,   277,
      -1,   465,   277,    -1,   307,   319,   276,    -1,   472,   320,
     274,    -1,   472,   308,   274,    -1,   472,   311,   274,    -1,
     309,   319,   276,    -1,   473,   320,   274,    -1,   473,   310,
     274,    -1,     5,    -1,   186,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     214,    -1,   215,    -1,   216,    -1,   217,    -1,   268,    -1,
     269,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   310,   310,   311,   314,   316,   323,   324,   325,   329,
     333,   342,   343,   344,   345,   346,   347,   353,   354,   361,
     368,   369,   373,   377,   384,   392,   393,   399,   408,   413,
     414,   418,   419,   423,   424,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   442,   443,   452,   453,   454,   455,
     458,   459,   460,   461,   465,   472,   475,   478,   482,   483,
     491,   507,   508,   512,   524,   537,   541,   554,   566,   572,
     573,   574,   585,   587,   589,   593,   594,   595,   602,   610,
     614,   620,   626,   628,   632,   633,   634,   635,   636,   637,
     638,   639,   643,   644,   645,   646,   650,   658,   659,   667,
     675,   676,   677,   681,   682,   686,   691,   692,   699,   708,
     709,   710,   714,   723,   729,   730,   731,   735,   736,   740,
     741,   742,   743,   744,   745,   751,   752,   753,   757,   758,
     762,   763,   769,   770,   773,   775,   779,   780,   784,   802,
     803,   804,   805,   806,   807,   815,   818,   821,   825,   826,
     829,   832,   835,   839,   845,   851,   858,   863,   868,   873,
     877,   881,   891,   899,   903,   910,   911,   914,   918,   920,
     922,   924,   926,   928,   930,   933,   939,   940,   941,   945,
     946,   952,   954,   956,   958,   960,   962,   966,   971,   976,
     977,   978,   984,   984,   993,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1025,  1036,  1037,  1044,  1045,
    1052,  1061,  1065,  1069,  1093,  1094,  1098,  1104,  1104,  1120,
    1121,  1124,  1125,  1129,  1132,  1141,  1148,  1152,  1153,  1154,
    1155,  1159,  1164,  1172,  1173,  1177,  1184,  1185,  1186,  1187,
    1194,  1195,  1198,  1199,  1204,  1215,  1216,  1220,  1221,  1225,
    1226,  1227,  1228,  1229,  1233,  1234,  1235,  1239,  1252,  1253,
    1254,  1255,  1256,  1257,  1258,  1259,  1260,  1261,  1271,  1275,
    1298,  1331,  1333,  1334,  1335,  1336,  1337,  1338,  1339,  1343,
    1344,  1345,  1346,  1347,  1348,  1349,  1350,  1351,  1352,  1353,
    1354,  1355,  1356,  1357,  1358,  1360,  1361,  1362,  1364,  1366,
    1368,  1370,  1371,  1372,  1373,  1374,  1384,  1385,  1386,  1387,
    1388,  1393,  1416,  1417,  1418,  1419,  1420,  1421,  1425,  1428,
    1429,  1430,  1439,  1446,  1455,  1456,  1460,  1466,  1476,  1485,
    1491,  1496,  1503,  1508,  1514,  1515,  1516,  1524,  1526,  1527,
    1528,  1534,  1555,  1556,  1557,  1558,  1564,  1565,  1566,  1567,
    1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,  1576,  1577,
    1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,  1586,  1587,
    1588,  1589,  1590,  1591,  1592,  1601,  1611,  1612,  1616,  1617,
    1618,  1619,  1622,  1634,  1637,  1639,  1643,  1644,  1645,  1646,
    1647,  1648,  1652,  1653,  1657,  1665,  1667,  1671,  1672,  1673,
    1677,  1678,  1681,  1683,  1686,  1687,  1688,  1689,  1690,  1691,
    1692,  1693,  1694,  1695,  1696,  1699,  1721,  1722,  1723,  1724,
    1725,  1726,  1727,  1728,  1729,  1730,  1731,  1732,  1733,  1734,
    1735,  1736,  1737,  1738,  1739,  1740,  1741,  1742,  1745,  1747,
    1751,  1752,  1754,  1763,  1773,  1775,  1779,  1780,  1782,  1791,
    1802,  1824,  1826,  1829,  1831,  1835,  1839,  1840,  1844,  1847,
    1850,  1853,  1856,  1859,  1865,  1873,  1874,  1875,  1880,  1881,
    1887,  1894,  1895,  1896,  1897,  1898,  1899,  1900,  1901,  1902,
    1903,  1904,  1905,  1906,  1910,  1911,  1912,  1917,  1918,  1919,
    1920,  1921,  1924,  1925,  1926,  1929,  1931,  1934,  1936,  1940,
    1949,  1956,  1963,  1964,  1965,  1968,  1977,  1981,  1987,  1988,
    1989,  1992,  1999,  1999,  1999,  1999,  1999,  1999,  1999,  1999,
    1999,  1999,  1999,  1999,  1999,  2000,  2000,  2000,  2000,  2000,
    2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,
    2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,
    2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2001,  2001,
    2001,  2001,  2001,  2001,  2001,  2001,  2002,  2002,  2002,  2002,
    2003,  2003,  2003,  2004,  2004,  2005,  2006,  2007,  2008,  2009,
    2010,  2011,  2012,  2012,  2012,  2012,  2012,  2012
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
  "TIMEFORMAT", "ISO", "SHORT", "LONG", "BASE", "CONFIGURE", "DOWN",
  "KERNEL", "PERSIST", "SCAN", "TIME", "LEARN", "DEVICE", "ASYNC", "TABLE",
  "ROUTER", "ID", "PROTOCOL", "PREFERENCE", "DISABLED", "DIRECT",
  "INTERFACE", "IMPORT", "EXPORT", "FILTER", "NONE", "STATES", "ROUTES",
  "FILTERS", "PASSWORD", "FROM", "PASSIVE", "TO", "EVENTS", "PACKETS",
  "PROTOCOLS", "INTERFACES", "PRIMARY", "STATS", "COUNT", "FOR",
  "COMMANDS", "PREEXPORT", "GENERATE", "LISTEN", "BGP", "V6ONLY",
  "ADDRESS", "PORT", "PASSWORDS", "DESCRIPTION", "RELOAD", "IN", "OUT",
  "MRTDUMP", "MESSAGES", "SHOW", "STATUS", "SUMMARY", "ROUTE", "SYMBOLS",
  "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS", "ATTRIBUTES", "ECHO",
  "DISABLE", "ENABLE", "RESTART", "FUNCTION", "PRINT", "PRINTN", "UNSET",
  "RETURN", "ACCEPT", "REJECT", "QUITBIRD", "INT", "BOOL", "IP", "PREFIX",
  "PAIR", "SET", "STRING", "BGPMASK", "BGPPATH", "CLIST", "IF", "THEN",
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
  "INTERPRET", "COMMUNITIES", "OSPF", "AREA", "OSPF_METRIC1",
  "OSPF_METRIC2", "OSPF_TAG", "OSPF_ROUTER_ID", "BROADCAST",
  "RFC1583COMPAT", "STUB", "TICK", "COST", "RETRANSMIT", "HELLO",
  "TRANSMIT", "PRIORITY", "DEAD", "NONBROADCAST", "POINTOPOINT", "TYPE",
  "SIMPLE", "AUTHENTICATION", "STRICT", "CRYPTOGRAPHIC", "ELIGIBLE",
  "POLL", "NETWORKS", "HIDDEN", "VIRTUAL", "LINK", "RX", "BUFFER", "LARGE",
  "NORMAL", "STUBNET", "LSADB", "TOPOLOGY", "STATE", "PIPE", "PEER",
  "MODE", "OPAQUE", "TRANSPARENT", "RIP", "INFINITY", "PERIOD", "GARBAGE",
  "TIMEOUT", "MULTICAST", "QUIET", "NOLISTEN", "VERSION1", "PLAINTEXT",
  "MD5", "HONOR", "NEVER", "ALWAYS", "RIP_METRIC", "RIP_TAG", "STATIC",
  "PROHIBIT", "'('", "')'", "';'", "':'", "'{'", "'}'", "','", "'?'",
  "'['", "']'", "$accept", "config", "conf_entries", "expr", "definition",
  "bool", "ipa", "prefix", "prefix_or_ipa", "pxlen", "datetime",
  "text_or_none", "log_config", "log_file", "log_mask", "log_mask_list",
  "log_cat", "mrtdump_base", "timeformat_which", "timeformat_spec",
  "timeformat_base", "cmd_CONFIGURE", "cmd_CONFIGURE_SOFT", "cmd_DOWN",
  "cfg_name", "kern_proto_start", "kern_item", "kif_proto_start",
  "kif_item", "nl_item", "rtrid", "idval", "listen", "listen_opts",
  "listen_opt", "newtab", "proto_start", "proto_name", "proto_item",
  "imexport", "rtable", "debug_default", "iface_patt_node_init",
  "iface_patt_node_body", "iface_negate", "iface_patt_node",
  "iface_patt_list", "dev_proto_start", "dev_proto", "dev_iface_init",
  "dev_iface_patt", "debug_mask", "debug_list", "debug_flag",
  "mrtdump_mask", "mrtdump_list", "mrtdump_flag", "password_list",
  "password_items", "password_item", "password_item_begin",
  "password_item_params", "cmd_SHOW_STATUS", "cmd_SHOW_PROTOCOLS",
  "cmd_SHOW_PROTOCOLS_ALL", "optsym", "cmd_SHOW_INTERFACES",
  "cmd_SHOW_INTERFACES_SUMMARY", "cmd_SHOW_ROUTE", "r_args",
  "export_or_preexport", "cmd_SHOW_SYMBOLS", "cmd_DUMP_RESOURCES",
  "cmd_DUMP_SOCKETS", "cmd_DUMP_INTERFACES", "cmd_DUMP_NEIGHBORS",
  "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES", "cmd_DUMP_PROTOCOLS",
  "cmd_ECHO", "echo_mask", "echo_size", "cmd_DISABLE", "cmd_ENABLE",
  "cmd_RESTART", "cmd_RELOAD", "cmd_RELOAD_IN", "cmd_RELOAD_OUT",
  "cmd_DEBUG", "cmd_MRTDUMP", "proto_patt", "filter_def", "@1",
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
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_STATE", "cmd_SHOW_OSPF_LSADB",
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
     514,   515,    40,    41,    59,    58,   123,   125,    44,    63,
      91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   282,   283,   283,   284,   284,   285,   285,   285,   286,
     286,   287,   287,   287,   287,   287,   287,   288,   288,   289,
     290,   290,   291,   291,   292,   293,   293,   294,   295,   295,
     295,   296,   296,   297,   297,   298,   298,   298,   298,   298,
     298,   298,   298,   298,   299,   299,   300,   300,   300,   300,
     301,   301,   301,   301,   302,   303,   304,   305,   306,   306,
     307,   308,   308,   308,   309,   310,   310,   311,   312,   313,
     313,   313,   314,   315,   315,   316,   316,   316,   317,   318,
     319,   319,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   321,   321,   321,   321,   322,   323,   323,   324,
     325,   325,   325,   326,   326,   327,   328,   328,   329,   330,
     330,   330,   331,   332,   333,   333,   333,   334,   334,   335,
     335,   335,   335,   335,   335,   336,   336,   336,   337,   337,
     338,   338,   339,   339,   340,   340,   341,   341,   342,   343,
     343,   343,   343,   343,   343,   344,   345,   346,   347,   347,
     348,   349,   350,   351,   351,   351,   351,   351,   351,   351,
     351,   351,   351,   351,   351,   352,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   362,   362,   363,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     372,   372,   374,   373,   375,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   377,   378,   378,   379,   379,
     380,   381,   381,   382,   383,   383,   384,   386,   385,   387,
     387,   388,   388,   389,   389,   390,   391,   392,   392,   392,
     392,   393,   393,   394,   394,   395,   396,   396,   396,   396,
     397,   397,   398,   398,   398,   399,   399,   400,   400,   401,
     401,   401,   401,   401,   402,   402,   402,   403,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   405,   406,
     407,   408,   408,   408,   408,   408,   408,   408,   408,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   410,   410,   410,   410,   410,   410,   411,   412,
     412,   412,   413,   413,   414,   414,   415,   415,   415,   415,
     415,   415,   415,   415,   415,   415,   415,   415,   415,   415,
     415,   416,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   418,   419,   419,   420,   420,
     420,   420,   421,   422,   423,   423,   424,   424,   424,   424,
     424,   424,   425,   425,   426,   427,   427,   428,   428,   428,
     429,   429,   430,   430,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   432,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   434,   434,
     435,   435,   436,   437,   438,   438,   439,   439,   440,   441,
     442,   443,   443,   444,   444,   445,   446,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   454,   454,   454,   454,
     455,   456,   456,   456,   456,   456,   456,   456,   456,   456,
     456,   456,   456,   456,   457,   457,   457,   458,   458,   458,
     458,   458,   459,   459,   459,   460,   460,   461,   461,   462,
     463,   464,   465,   465,   465,   466,   467,   467,   467,   467,
     467,   468,   469,   469,   469,   469,   469,   469,   469,   469,
     469,   469,   469,   469,   469,   470,   470,   470,   470,   470,
     470,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   470,   470,   470,   470,   470,   470,   470,   471,   471,
     471,   471,   471,   471,   471,   471,   472,   472,   472,   472,
     473,   473,   473,   474,   474,   474,   474,   474,   474,   474,
     474,   474,   474,   474,   474,   474,   474,   474
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     3,     1,     5,
       5,     1,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     2,     2,     1,     1,     0,     4,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     3,     1,     1,     1,     1,
       2,     4,     3,     3,     3,     3,     4,     2,     0,     1,
       2,     2,     3,     2,     2,     3,     3,     3,     4,     1,
       1,     1,     4,     0,     2,     2,     2,     1,     2,     1,
       0,     1,     0,     2,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     1,     1,     1,     1,     3,     3,     0,
       1,     1,     2,     0,     1,     3,     1,     3,     2,     3,
       3,     3,     0,     3,     1,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     4,     1,     0,     3,     4,     1,     2,     0,
       5,     5,     5,     5,     4,     3,     4,     5,     1,     0,
       3,     4,     4,     0,     2,     3,     3,     3,     2,     2,
       2,     3,     3,     2,     2,     1,     1,     4,     3,     3,
       3,     3,     3,     3,     3,     4,     1,     1,     1,     0,
       1,     3,     3,     3,     3,     4,     4,     4,     4,     1,
       1,     1,     0,     4,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     0,     3,     1,     3,
       1,     1,     1,     2,     3,     2,     4,     0,     5,     0,
       1,     1,     2,     1,     3,     5,     1,     1,     1,     1,
       1,     1,     4,     1,     3,     3,     1,     2,     2,     6,
       1,     3,     0,     4,     3,     1,     3,     3,     3,     2,
       2,     2,     2,     0,     2,     2,     0,     5,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     4,     1,     1,     1,     1,     2,
       2,     3,     3,     6,     3,     3,     3,     3,     6,     6,
       6,     4,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     3,     1,     3,     0,     1,     4,     6,     4,     3,
       5,     5,     4,     6,     3,     2,     5,     5,     8,     8,
       8,     2,     3,     3,     5,     6,     6,     4,     4,     5,
       6,     6,     5,     6,     5,     5,     5,     5,     5,     5,
       5,     5,     5,     6,     6,     8,     6,     6,     5,     4,
       5,     4,     5,     4,     5,     2,     3,     3,     1,     2,
       2,     2,     3,     2,     0,     3,     3,     2,     4,     2,
       2,     1,     4,     1,     1,     0,     3,     2,     2,     2,
       4,     1,     0,     3,     0,     2,     2,     3,     2,     2,
       3,     2,     2,     2,     1,     3,     2,     2,     2,     2,
       3,     2,     2,     2,     3,     2,     2,     2,     3,     2,
       4,     2,     2,     2,     3,     3,     3,     1,     0,     2,
       1,     1,     2,     3,     0,     2,     1,     1,     2,     3,
       0,     0,     3,     0,     3,     3,     1,     0,     4,     6,
       6,     6,     6,     6,     2,     3,     3,     5,     4,     4,
       2,     3,     3,     4,     4,     4,     5,     5,     4,     3,
       4,     4,     4,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     2,     2,     0,     3,     0,     3,     0,
       3,     2,     3,     3,     3,     2,     3,     3,     2,     2,
       2,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    58,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   525,   526,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   537,   538,   539,
     540,   541,   542,   543,   544,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,     3,     1,
       2,     0,     0,     0,     0,     0,     0,    79,     0,     0,
       0,     0,   268,   512,   513,   514,   515,   516,    80,    80,
     517,   518,   519,     0,   521,    80,    82,   522,   523,   524,
      80,    82,    80,    82,    80,    82,    80,    82,    80,    82,
       5,   520,    82,    82,   189,   191,   190,     0,    59,    58,
       0,    57,     0,     0,     0,     0,   149,     0,     0,   153,
     149,   149,   149,     0,     0,     0,     0,     0,     0,     0,
     178,   177,   176,   179,     0,     0,     0,     0,    28,    29,
      30,     0,     0,     0,    49,    48,    47,    46,     0,     0,
      78,     0,   192,    73,     0,     0,   217,   253,   258,   266,
     226,   270,   261,     0,     0,   256,   268,   298,   259,   260,
       0,     0,     0,     0,   268,     0,   262,   263,   267,   297,
     296,     0,   295,   194,    81,     0,     0,    60,    64,   108,
     341,   375,   464,   470,   501,     0,     0,     0,     0,     0,
      16,   112,     0,     0,     0,     0,   560,     0,     0,     0,
       0,     0,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    16,     0,     0,     0,   561,     0,     0,     0,    16,
       0,   562,   378,     0,   384,     0,     0,     0,     0,   563,
       0,     0,   499,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   564,     0,     0,   133,   137,     0,     0,   565,
       0,     0,     0,     0,    16,     0,    16,   558,     0,     0,
       0,     0,    26,   559,     0,     0,   115,   114,     0,     0,
       0,    55,     0,     0,   184,   126,   125,     0,     0,   148,
     149,     0,   150,     0,   145,     0,     0,   149,   149,   149,
     149,   149,     0,     0,   173,   174,   170,   168,   169,   171,
     172,   180,     0,   181,   182,   183,     0,    31,     0,     0,
      97,     6,     8,   268,    98,    50,     0,    54,    69,    70,
      71,     0,   206,     0,    45,     0,     0,   253,   270,   253,
     268,   253,   253,     0,   245,     0,   268,     0,     0,   256,
     256,     0,   293,   268,   268,   268,   268,     0,   227,   230,
       0,   228,   229,   231,   233,     0,   236,   240,     0,   573,
     271,   272,   273,   274,   275,   276,   277,   278,   574,   575,
     576,   577,   578,   579,   580,   581,   582,   583,   584,   585,
     586,   587,   299,   300,   268,   268,   268,   268,   268,   268,
     268,   268,   268,     0,   268,   268,   268,   268,   566,   570,
     109,    85,    96,    89,     0,    83,    12,    14,    13,    15,
      11,    84,    99,    94,   206,    95,   268,    87,    93,    88,
      91,    86,   110,   111,   342,     0,     0,     0,     0,     0,
       0,     0,    16,     0,     0,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,     0,
       0,    16,     0,    16,     0,    16,   343,   376,     0,   379,
     380,   377,   383,   381,   465,     0,     0,     0,   466,   471,
      99,     0,   138,     0,   134,   486,   484,   485,     0,     0,
       0,     0,     0,     0,     0,     0,   472,   479,   139,   502,
       0,   505,   503,   509,     0,   508,   510,   504,     0,    61,
       0,    63,   568,   569,   567,     0,    25,     0,   572,   571,
     119,   120,   121,   123,   124,   122,     0,   117,   187,    56,
     185,   186,   130,   131,     0,   128,   188,     0,   146,   151,
     152,   159,     0,     0,   166,   206,   160,   163,   164,     0,
     165,   154,     0,   158,   167,   457,   457,   457,   457,   457,
     458,   511,   175,     0,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     0,    33,    27,     0,     0,    52,
      53,    68,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,     0,     0,   193,   210,    77,     0,     0,    72,
      74,    44,     0,   206,   249,   250,     0,   251,   252,   247,
     235,   322,   325,     0,   306,   307,   254,   255,   248,     0,
       0,     0,     0,   279,   268,     0,     0,     0,   264,   237,
     238,     0,     0,   265,   291,   289,   287,   284,   285,   286,
     288,   290,   292,   301,     0,   302,   304,   305,   280,   281,
     282,   283,    90,   103,   106,   113,   211,   212,    92,   213,
       0,     0,   371,   373,     0,    16,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   347,     0,   348,     0,   369,     0,     0,     0,     0,
       0,   382,   450,    16,     0,     0,     0,     0,   391,   401,
       0,   468,   469,   497,   483,   474,     0,     0,   478,   473,
     475,     0,     0,   481,   482,   480,     0,     0,     0,     0,
       0,     0,    19,   507,   506,    67,    62,    65,    21,    20,
      66,   116,     0,   127,     0,   147,   156,   162,   157,   155,
     161,   456,     0,     0,     0,     0,     0,    10,     9,    32,
       0,     7,    51,   205,   204,   206,   268,    75,    76,   215,
     208,     0,   218,   246,   268,   311,   294,   268,   268,   268,
       0,     0,     0,   226,   234,     0,     0,   241,   268,   104,
       0,    99,     0,     0,   372,     0,     0,   368,   362,   344,
       0,   349,     0,   352,     0,     0,   354,   361,   360,   358,
       0,     0,   370,   359,   355,   356,   357,   374,    99,   390,
       0,   387,   438,     0,   394,   389,   393,   385,   402,   467,
     495,   500,   132,   134,   476,   477,     0,     0,     0,     0,
       0,   136,    22,    23,   118,   129,   460,   459,   463,   461,
     462,    34,   207,     0,   315,     0,   316,   317,     0,   268,
     313,   314,   312,   268,   268,     0,   268,     0,     0,   268,
     221,   214,     0,   323,     0,     0,     0,   257,     0,   232,
       0,     0,   100,   101,   105,   107,   364,     0,   366,   367,
     345,   351,   353,   350,   363,   346,   453,   386,     0,   415,
     395,   404,   492,   135,   139,    24,     0,     0,     0,     0,
     268,   268,   268,   268,     0,     0,     0,   216,   222,     0,
       0,   335,   318,   320,     0,   209,   309,   310,   308,   225,
       0,   303,   102,     0,   451,   455,   388,     0,   439,   440,
     441,     0,     0,     0,     0,     0,     0,     0,   400,   414,
       0,     0,     0,   498,     0,   144,   139,   139,   139,   139,
       0,     0,     0,     0,   329,   268,   242,   268,   268,     0,
     268,   334,   239,   365,     0,     0,   442,    16,     0,    16,
     392,     0,   408,   406,   405,     0,     0,   409,   411,   412,
     413,   403,   493,   487,   488,   489,   490,   491,   494,   496,
     140,   141,   142,   143,   328,   269,   332,     0,   268,   326,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     321,     0,     0,    16,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   437,     0,   443,   398,
     399,   397,   396,   407,   410,     0,     0,   268,   268,   268,
     336,   331,   330,   268,   268,   268,   337,   444,   422,   429,
     416,   419,   417,     0,   421,     0,   423,   425,   426,   427,
     431,   432,   433,    16,   418,     0,   452,   333,   224,   327,
     244,   242,     0,     0,     0,     0,   420,   424,   428,   434,
     435,   436,   243,     0,     0,     0,     0,   430,   445,   446,
     447,   339,   340,   338,     0,   448,   449
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   420,    64,   421,   500,   729,   730,   722,
     896,   517,    65,   131,   319,   574,   575,    66,   138,   139,
      67,    15,    16,    17,   100,    68,   268,    69,   274,   269,
      70,   331,    71,   333,   600,    72,    73,   175,   197,   427,
     413,    74,   653,   874,   780,   654,   655,    75,    76,   422,
     198,   279,   526,   527,   288,   534,   535,   254,   706,   255,
     256,   719,    18,    19,    20,   291,    21,    22,    23,   295,
     552,    24,    25,    26,    27,    28,    29,    30,    31,    32,
     123,   312,    33,    34,    35,    36,    37,    38,    39,    40,
      97,    77,   332,    78,   591,   592,   593,   761,   657,   658,
     428,   603,   595,    79,   336,   855,   856,   999,   361,   166,
     363,  1002,   365,   167,   367,   368,  1003,   342,   168,   343,
     351,   169,   170,   171,   858,   172,   392,   611,   859,   913,
     914,   612,   613,   860,    80,    81,    82,    83,   233,   234,
     235,   472,   697,   815,   816,   931,   971,   698,   891,   940,
     699,  1027,   888,   928,   929,   930,  1075,  1088,  1089,  1090,
     808,   964,   925,   809,   742,    41,    42,    43,    44,    45,
      46,    84,    85,    86,    87,   488,   988,   944,   892,   821,
     480,   481,    88,    89,   261,   262,    47,    90,    48,    91,
      92,    93,   393
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -870
static const yytype_int16 yypact[] =
{
      39,  1446,    64,   156,   291,   170,    96,   228,   291,   353,
     764,   386,   291,   291,   291,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,   107,   405,   220,   349,   173,   128,  -870,   226,   153,
       3,   283,   496,  -870,  -870,  -870,  -870,  -870,   341,   341,
    -870,  -870,  -870,  1172,  -870,   341,  1332,  -870,  -870,  -870,
     341,  1184,   341,  1064,   341,  1058,   341,  1207,   341,  1227,
    -870,  -870,   543,  1308,  -870,  -870,  -870,   139,  -870,   356,
     360,  -870,   291,   291,   374,   142,   201,    97,   381,  -870,
     383,   308,   383,   411,   417,   437,   442,   449,   456,   458,
    -870,  -870,  -870,   420,   462,   466,   468,   453,  -870,  -870,
    -870,   -21,   139,    45,  -870,  -870,  -870,  -870,   138,   214,
    -870,   590,  -870,  -870,   217,   142,  -870,     2,  -870,  -870,
     469,   231,  -870,   338,   343,    -2,   496,  -870,  -870,  -870,
     236,   244,   246,   248,   496,    47,  -870,  -870,  -870,  -870,
    -870,   964,  -870,  1566,  -870,   251,   263,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,   287,   139,   572,   544,    45,
      52,  -870,   312,   312,   602,   142,  -870,   348,   354,   376,
     213,   614,    52,   439,   471,   -13,   563,   512,   460,   629,
     527,   641,    45,   547,   564,   347,   562,   559,   361,   548,
     546,    52,   539,   541,   538,  -870,   487,   486,   590,    52,
      45,  -870,  -870,   498,  -870,   493,   497,   710,   322,  -870,
     501,   510,  -870,   770,    45,   513,   -47,    45,    45,   730,
     734,   -93,  -870,   522,   524,  -870,   523,   529,   460,  -870,
     532,    50,   533,   739,    52,   748,    52,  -870,   542,   545,
     549,   758,   799,  -870,   560,   565,  -870,  -870,   616,   818,
     830,  -870,   832,   834,  -870,  -870,  -870,   146,   839,  -870,
     383,   844,  -870,   845,  -870,  1403,   854,   383,   383,   383,
     383,   383,   860,   861,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,   872,  -870,  -870,  -870,    54,  -870,  1413,   607,
    -870,  -870,  -870,   496,  -870,    45,   584,  -870,  -870,  -870,
    -870,   611,  1090,   -15,  -870,   618,   622,     2,  -870,     2,
     496,     2,     2,   876,  -870,   893,   418,   882,   883,    -2,
      -2,   887,  -870,   496,   496,   496,   496,    26,  -870,  -870,
     905,  -870,  -870,   896,  -870,  -236,    -1,  -870,    32,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,   496,   496,   496,   496,   496,   496,
     496,   496,   496,   407,   496,   496,   496,   496,  -870,  -870,
    -870,  -870,  -870,  -870,   590,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,   623,  -870,   496,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,   857,   880,   667,   669,    45,
     923,   759,    52,   460,    45,  -870,  -870,   803,    45,   917,
      45,   805,   918,   822,    45,    45,    52,   920,   726,   935,
     728,    52,   752,    52,   334,    52,  -870,  -870,   753,  -870,
    -870,  -870,   278,  -870,  -870,  1011,   762,   763,  -870,  -870,
    -870,   765,  -870,   766,   959,  -870,  -870,  -870,   767,   768,
     769,    45,    45,   781,   782,   783,  -870,  -870,   115,  -870,
      28,  -870,  -870,  -870,   552,  -870,  -870,  -870,    45,  -870,
      45,  -870,  -870,  -870,  -870,    45,  -870,   460,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,   336,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,   397,  -870,  -870,  1055,  -870,  -870,
    -870,  -870,  1042,  1043,  -870,   623,  -870,  -870,  -870,   460,
    -870,  -870,  1046,  -870,  -870,  1051,  1051,  1051,  1051,  1051,
    -870,  -870,  -870,   797,   807,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,   399,  -870,  -870,   711,  1061,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,    27,   808,   809,  -870,  -870,  -870,   460,    45,  -870,
    -870,  -870,   970,  1090,  -870,  -870,   757,  -870,  -870,  -870,
    -870,    83,  -870,   800,  -870,  -870,  -870,  -870,  -870,   804,
     111,   120,   143,  -870,   496,   811,  1062,   141,  -870,  -870,
    -870,  1080,  1078,  -870,  1423,  1423,  1423,  1575,  1575,  1423,
    1423,  1423,  1423,  -870,   823,  -870,  -870,  -870,   668,   668,
    -870,  -870,  -870,  1070,  -870,   847,  -870,  -870,  -870,  1566,
      45,    45,  -870,  -870,   836,    52,    52,   850,   864,   865,
      45,   866,    45,   867,   460,    45,   869,   870,   871,   873,
      45,  -870,    45,  -870,   874,  -870,   875,   885,   886,   889,
     891,  -870,  -870,    42,   841,   881,   460,   892,  -870,   858,
     902,  -870,  -870,   -45,  -870,  -870,   884,   908,  -870,  -870,
    -870,   909,   910,  -870,  -870,  -870,    45,   300,   367,   890,
      45,   460,  -870,  -870,  -870,  -870,  -870,  -870,    28,  -870,
    -870,  -870,   616,  -870,   146,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  1132,  1143,  1182,  1187,  1188,  -870,  -870,  -870,
    1413,  -870,  -870,  -870,  -870,  1090,   358,  -870,  -870,  -870,
    -870,   430,  -870,  -870,   496,  -870,  -870,   496,   496,   496,
     846,  1174,   141,  -870,  -870,   914,   469,  -870,   496,  -870,
     569,  -870,   919,   916,  -870,   930,   931,  -870,  -870,  -870,
     932,  -870,   933,  -870,   934,   936,  -870,  -870,  -870,  -870,
     939,   942,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
      45,  -870,  -870,   590,  -870,  -870,   927,  -870,  -870,  -870,
    -870,  -870,  -870,   959,  -870,  -870,   943,  1191,  1191,  1191,
    1191,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,   -10,  -870,  1210,  -870,  -870,   963,   496,
    -870,  -870,  -870,   496,   496,   954,  1048,   964,   962,   476,
    -870,  -870,  1090,  -870,   904,   929,   938,  -870,   966,  -870,
    1224,   965,   460,  -870,  -870,  -870,  -870,    45,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,   149,  -870,    21,  -870,
    -870,   843,   177,  -870,   115,  -870,   967,   971,   973,   976,
     496,   418,   496,  -870,   262,  1490,   345,  -870,  -870,  1220,
       6,  -870,  1566,   977,   980,  -870,  -870,  -870,  -870,  -870,
     974,  -870,  -870,   984,  -870,  -870,  -870,   -27,  -870,  -870,
    -870,   124,    45,    45,    45,  1077,   145,   -48,  -870,  -870,
     986,    45,   365,  -870,   990,  -870,   115,   115,   115,   115,
     384,   989,   571,    11,  -870,   778,   109,   496,   496,   735,
     476,  -870,  -870,  -870,  1303,   994,  -870,    52,    45,    52,
    -870,   995,  -870,  -870,  -870,    45,    45,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  1000,   358,  1104,
    -870,   999,  1006,  1005,   660,   702,  1015,  1017,  1018,  1020,
    -870,  1021,    45,    52,    45,    45,    45,  1121,    45,   165,
      24,     4,  1076,    45,  1068,  -870,  -870,  1038,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  1039,  1037,   778,   358,  1074,
    -870,  -870,  -870,   496,   496,   496,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,    45,  -870,    45,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,    52,  -870,    84,  -870,  -870,  -870,  -870,
    -870,   109,   988,   997,  1025,    -4,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  1041,  1044,  1045,  -196,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  1047,  -870,  -870
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -870,  -870,  -870,  -132,  -870,  -198,  -202,  -255,   773,  -870,
    -184,  -870,  -870,  -870,  -870,  -870,   566,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  1218,  -870,  -870,  -870,  -870,  -870,
    -870,  -223,  -870,  -870,  -870,  -870,  -870,   573,   991,  1130,
    -870,  -870,  -870,  -870,  -870,   553,  -467,  -870,  -870,  -870,
    -870,  -114,  -870,   592,    34,  -870,   594,  -869,   503,  -475,
    -870,  -416,  -870,  -870,  -870,   561,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
     443,  -870,  -870,  -870,  -870,  -561,   574,  -870,   998,   788,
    1049,  -870,   736,  -870,  -870,  -802,  -870,   301,  -870,  -163,
     568,  -157,  -870,  -158,   705,  -870,   271,  -870,  -870,   210,
     364,  -870,  -870,  -742,  -870,  -127,   488,   -62,  -870,  -870,
     388,   579,   457,  -649,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,   106,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -744
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -325
static const yytype_int16 yytable[] =
{
     173,   324,   362,   501,   438,   468,   447,   366,   364,   707,
     900,   349,  1086,   703,   857,   337,   369,   317,   320,   338,
     344,   144,   939,   462,   629,   630,   958,   978,   485,   339,
     959,   469,   394,   395,   396,   397,   398,   445,   446,  1094,
     551,   760,   627,     1,   753,   628,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   321,   720,   415,   321,   322,
     358,   359,   322,   150,    49,   321,   509,   321,   511,   322,
     563,   322,   411,   493,   416,   417,   418,   419,  1095,  1060,
     451,   596,   597,   598,   416,   417,   418,   419,   145,   394,
     395,   396,   397,   398,   352,  1026,   441,   321,   470,   101,
     292,   322,   357,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   483,   910,   857,   489,   490,   394,   395,   396,
     397,   398,   358,   359,   127,   773,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   394,
     395,   396,   397,   398,   358,   359,   325,   773,   321,    50,
     754,   953,   322,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   276,   494,   495,   285,   503,   277,   321,   335,
     286,   716,   322,   979,   564,   442,   980,    51,    98,   326,
     140,   652,    52,   578,   141,    53,  1036,   378,   379,   380,
     381,   382,   383,   384,   385,   293,    54,   908,   717,   997,
     344,   965,   344,   857,   344,   344,   486,   487,   289,    99,
      55,    56,   532,    57,   504,   386,   387,   388,   389,   431,
      58,   820,   967,   781,   976,  1061,  1070,  1071,  1062,   290,
     718,   668,  1057,   142,   667,    94,    95,   966,   143,  1001,
      59,   533,  1058,  1059,  1055,   318,   857,   505,   679,   599,
      60,   577,   901,   684,   810,   686,    96,   690,   394,   395,
     396,   397,   398,  1087,   340,   631,    61,   350,   606,   390,
     391,   341,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   619,   620,   621,   622,   857,   857,   857,   926,   623,
     146,   915,   724,   721,   624,   132,  1000,   664,    94,    95,
     632,   133,   669,   633,   323,   728,   671,   323,   673,   360,
      62,   506,   677,   678,   323,   289,   323,  1079,  1080,    96,
     102,   103,   634,   635,   636,   637,   638,   639,   640,   641,
     642,   886,   648,   649,   650,   651,   968,   728,   707,   692,
     423,   394,   395,   396,   397,   398,   323,   941,   174,   711,
     712,   764,   969,   281,   659,   399,   400,   401,   402,   403,
     404,   405,   406,   407,    98,   843,   725,   284,   726,   297,
     827,   360,   828,   727,   294,   134,   424,   425,  1000,   767,
     394,   395,   396,   397,   398,   757,   435,   436,   768,   120,
     289,   970,   844,   135,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   360,   304,   278,   136,   323,   287,   121,
     305,   769,   298,   128,   122,   924,   845,   781,   942,   147,
      63,   148,   149,   311,   150,   151,   152,   323,   106,   107,
     306,   814,   129,   153,   154,   307,   155,   829,   156,   830,
     104,   105,   308,   130,   943,   124,   125,   126,   137,   309,
     108,   310,   109,   110,   362,   313,   758,   785,   786,   314,
     774,   315,   794,   316,   366,   426,   445,   446,   945,   846,
     847,   848,   849,   850,   851,   852,   157,   147,   327,   148,
     149,   334,   150,   151,   152,   811,   853,   345,   693,   854,
     347,   153,   154,   346,   155,   348,   156,   147,   353,   148,
     149,   687,   150,   151,   152,   694,   354,   695,   355,   833,
     356,   153,   154,   696,   155,   873,   156,   408,   782,   783,
     990,   991,   992,   993,   454,   455,   954,   643,   790,   409,
     792,   688,   689,   795,   157,   282,   283,   604,   800,   605,
     801,   607,   608,   644,   299,   300,   301,   458,   459,   645,
     158,   159,   770,   410,   157,   111,   646,   647,   445,   446,
     723,   160,   161,   162,   476,   477,   163,   394,   395,   396,
     397,   398,   186,   983,   826,   445,   446,   872,   832,   412,
     889,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     263,   264,   265,   328,   266,   329,   330,   187,   188,   362,
     414,   189,   190,   731,   732,   192,   193,   922,   158,   159,
     430,   956,   432,   112,   984,   985,   986,   987,   433,   160,
     161,   162,   437,   927,   163,  -219,   579,   580,   158,   159,
     656,   439,   176,   194,   897,   898,   899,   195,   185,   160,
     161,   162,   434,   199,   163,   227,   440,   236,   994,   241,
     443,   257,   743,   744,   745,   746,   394,   395,   396,   397,
     398,   296,   302,   303,   733,   734,   749,   750,   887,   444,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   448,
     164,  -324,   520,   521,   522,   406,   407,   449,   165,   523,
     524,   450,   525,   861,   862,   864,   865,   866,   394,   395,
     396,   397,   398,   616,   617,   452,   871,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     453,   457,   456,   463,   460,   923,   461,   464,   164,   465,
    -319,   582,   583,   584,   585,   586,   165,   587,   588,   589,
     590,   466,   467,   394,   395,   396,   397,   398,   164,  1029,
     473,  1031,   471,   474,   475,   478,   165,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   479,   904,   482,   484,
     491,   905,   906,   362,   492,   843,   496,   912,   497,   498,
     972,   973,   974,   508,   977,   499,   502,   507,   510,   982,
     394,   395,   396,   397,   398,  1049,   512,   516,   515,   513,
     267,   528,   844,   514,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   529,   518,   530,  1030,   531,   950,   519,
     952,   113,   536,  1033,  1034,   996,   845,   538,   539,   114,
     115,   537,   394,   395,   396,   397,   398,   554,   555,   556,
     557,   558,   559,   560,   561,  1078,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   562,   116,   117,   118,   119,
    1048,   576,  1050,  1051,  1052,   581,  1054,  1056,   609,  1006,
    1007,  1064,   601,  1008,   602,  1004,  1005,  1009,   912,   846,
     847,   848,   849,   850,   851,   852,   610,   614,   362,   615,
     394,   395,   396,   397,   398,   618,   853,   660,   625,   854,
     626,  1076,   243,  1077,   399,   400,   401,   402,   403,   404,
     405,   406,   407,  1081,  1041,   394,   395,   396,   397,   398,
     661,   662,   245,   663,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   665,   666,   369,
     670,   394,   395,   396,   397,   398,  1042,   672,   675,   674,
     680,  1072,  1073,  1074,   751,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   394,   395,   396,   397,   398,   676,
     681,   682,   683,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   685,   932,   700,   691,
     763,   394,   395,   396,   397,   398,   701,   702,   243,   704,
     705,   708,   709,   710,   370,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   998,   713,   714,   715,   735,   736,
     737,  -220,  -220,   740,  -220,   843,   933,   934,   935,   741,
     936,   747,   226,   765,   232,   937,   240,   766,   253,   752,
     260,   748,   755,   270,   275,   756,   772,  -219,  -219,   771,
    -219,   843,   844,   775,   776,   778,   779,   186,   582,   583,
     584,   585,   586,   186,   587,   588,   589,   590,   371,   372,
     784,   373,   374,   375,   376,   377,   845,   812,   844,   867,
     938,   813,   187,   188,   787,   781,   189,   190,   187,   188,
     192,   193,   189,   190,   818,   836,   192,   193,   788,   789,
     791,   793,   845,   796,   797,   798,   837,   799,   802,   803,
     378,   379,   380,   381,   382,   383,   384,   385,   194,   804,
     805,   822,   195,   806,   194,   807,   817,   831,   195,   846,
     847,   848,   849,   850,   851,   852,   819,   916,   386,   387,
     388,   389,   823,   824,   825,   838,   853,   868,  -220,   854,
     839,   840,   870,   876,   877,   846,   847,   848,   849,   850,
     851,   852,   917,   890,   878,   879,   880,   881,   882,   895,
     883,   918,   853,   884,  -219,   854,   885,   894,   582,   583,
     584,   585,   586,   186,   587,   588,   589,   590,   200,   177,
     902,   907,   390,   391,   178,   903,   911,   920,   921,   919,
     957,   946,   179,   759,  1037,   947,   186,   948,   187,   188,
     949,   962,   189,   190,   961,   960,   192,   193,   963,   975,
     981,  1083,   995,   201,   989,   202,   186,   180,  1028,  1032,
    1084,   187,   188,  1035,  1038,   189,   190,   228,   242,   192,
     193,  1039,  1040,   229,   194,   230,   243,  1043,   195,  1044,
    1045,   187,   188,   203,  1046,   189,   190,  1047,  1085,   192,
     193,   204,   205,  1053,  1063,   244,   245,   194,   237,   238,
    1065,   195,  1066,  1067,  1068,  1091,   841,   280,  1092,  1093,
    -220,  1096,   739,   429,   834,  -220,   893,   194,   835,   842,
     594,   195,   206,   738,   875,   239,   258,   777,  1069,   762,
     869,   231,  1082,   863,   553,   909,  -219,   186,  1010,   207,
     208,  -219,   209,   210,     0,   211,   212,   213,   951,   214,
       0,     0,   215,   216,     0,   217,     0,   271,     0,     0,
       0,   186,   187,   188,     0,     0,   189,   190,   218,   219,
     192,   193,   243,   220,   181,   221,     0,   222,     0,   223,
       0,     0,     0,     0,   224,   272,   187,   188,     0,     0,
     189,   190,   245,   191,   192,   193,   540,     0,   194,     0,
       0,     0,   195,     0,     0,     0,     0,  1011,     0,   445,
     446,   182,     0,     0,     0,     0,   183,     0,     0,  -325,
    -325,  -325,   194,     0,     0,     0,   195,     0,     0,   246,
       0,   541,   184,  -325,  -325,  -325,  -325,  -325,   404,   405,
     406,   407,   565,   566,   567,   568,   569,   570,   571,   572,
     573,   225,   247,   248,   249,   250,     0,   542,     0,     0,
     543,     0,   251,     0,     0,     0,   544,   545,     0,     0,
       0,     0,     0,     0,   252,     4,     0,  1012,     0,     0,
     546,   547,   548,   549,     0,   550,   394,   395,   396,   397,
     398,     5,     6,     0,   259,     0,     0,     0,     0,     0,
     399,   400,   401,   402,   403,   404,   405,   406,   407,     0,
       0,     0,     0,  1013,     0,  1014,  1015,  1016,  1017,  1018,
    1019,     0,     0,  1020,     0,  1021,  1022,     0,     0,  1023,
       0,     0,     0,     0,  1024,     0,     0,     7,     0,     0,
       8,     0,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,    11,    12,    13,    14,   426,     0,     0,     0,
       0,     0,   394,   395,   396,   397,   398,     0,     0,     0,
    1025,   394,   395,   396,     0,   273,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   399,   400,   401,   402,   403,
     404,   405,   406,   407,     0,     0,     0,     0,     0,   196,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   955
};

static const yytype_int16 yycheck[] =
{
      62,   133,   165,   258,   202,   228,   208,   165,   165,   484,
      20,    13,    16,   480,   756,    13,     5,    38,   132,    17,
     147,    18,   891,   221,    25,    26,    20,    75,    75,    27,
      24,   229,     6,     7,     8,     9,    10,    16,    17,   235,
     295,   602,   278,     4,    17,   281,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    13,    28,   189,    13,    17,
      13,    14,    17,    16,     0,    13,   264,    13,   266,    17,
      16,    17,   186,   166,    32,    33,    34,    35,   274,    75,
     212,    96,    97,    98,    32,    33,    34,    35,    85,     6,
       7,     8,     9,    10,   156,   964,   109,    13,   230,     3,
       3,    17,   164,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   244,   857,   856,   247,   248,     6,     7,     8,
       9,    10,    13,    14,    17,    16,     6,     7,     8,     9,
      10,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     6,
       7,     8,     9,    10,    13,    14,    18,    16,    13,     3,
     133,   903,    17,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    33,   266,   267,    33,   126,    38,    13,   145,
      38,    66,    17,   231,   316,   198,   234,    31,    18,    51,
      17,   414,    36,   325,    66,    39,   998,   186,   187,   188,
     189,   190,   191,   192,   193,   108,    50,   856,    93,   953,
     337,   238,   339,   955,   341,   342,   263,   264,    17,    49,
      64,    65,    76,    67,   174,   214,   215,   216,   217,   195,
      74,   276,   108,   278,    89,   231,  1038,  1039,   234,    38,
     125,   443,   218,    17,   442,    17,    18,   274,    95,   140,
      94,   105,   228,   229,    89,   276,   998,   207,   456,   274,
     104,   323,   272,   461,   222,   463,    38,   465,     6,     7,
       8,     9,    10,   277,   272,   276,   120,   279,   340,   268,
     269,   279,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   353,   354,   355,   356,  1037,  1038,  1039,   277,   273,
      17,   862,   504,   275,   278,    85,   955,   439,    17,    18,
     278,    91,   444,   281,   272,   517,   448,   272,   450,   272,
     164,   271,   454,   455,   272,    17,   272,   243,   244,    38,
     102,   103,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   808,   404,   405,   406,   407,   222,   549,   823,    71,
      38,     6,     7,     8,     9,    10,   272,   180,    17,   491,
     492,   278,   238,     3,   426,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    18,    17,   508,     3,   510,    71,
      80,   272,    82,   515,     3,    36,    74,    75,  1037,   278,
       6,     7,     8,     9,    10,   597,   183,   184,   278,    13,
      17,   277,    44,    54,    20,    21,    22,    23,    24,    25,
      26,    27,    28,   272,     3,   276,    67,   272,   276,    33,
       3,   278,   114,    18,    38,   276,    68,   278,   251,    11,
     274,    13,    14,    13,    16,    17,    18,   272,    85,    86,
       3,   696,    37,    25,    26,     3,    28,    80,    30,    82,
       7,     8,     3,    48,   277,    12,    13,    14,   109,     3,
     107,     3,   109,   110,   627,     3,   598,   665,   666,     3,
     627,     3,   674,    20,   632,   163,    16,    17,   894,   121,
     122,   123,   124,   125,   126,   127,    68,    11,   274,    13,
      14,   274,    16,    17,    18,   693,   138,    28,   220,   141,
     162,    25,    26,   272,    28,   162,    30,    11,   272,    13,
      14,   177,    16,    17,    18,   237,   272,   239,   272,   721,
     272,    25,    26,   245,    28,   780,    30,   276,   660,   661,
     946,   947,   948,   949,   187,   188,   274,   130,   670,   276,
     672,   207,   208,   675,    68,   102,   103,   337,   680,   339,
     682,   341,   342,   146,   246,   247,   248,   196,   197,   152,
     142,   143,   624,   276,    68,   212,   159,   160,    16,    17,
      18,   153,   154,   155,   252,   253,   158,     6,     7,     8,
       9,    10,    39,   218,   716,    16,    17,    18,   720,    17,
     813,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      57,    58,    59,    13,    61,    15,    16,    64,    65,   772,
      66,    68,    69,   277,   278,    72,    73,   872,   142,   143,
      18,   276,   274,   270,   259,   260,   261,   262,   274,   153,
     154,   155,    18,   888,   158,   277,    52,    53,   142,   143,
      17,   202,    69,   100,   828,   829,   830,   104,    75,   153,
     154,   155,   276,    80,   158,    82,   185,    84,   274,    86,
      97,    88,   556,   557,   558,   559,     6,     7,     8,     9,
      10,   110,   111,   112,   277,   278,   277,   278,   810,   167,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    60,
     272,   273,    76,    77,    78,    27,    28,   170,   280,    83,
      84,    60,    86,   273,   274,   767,   768,   769,     6,     7,
       8,     9,    10,   349,   350,   168,   778,     6,     7,     8,
       9,    10,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     176,   182,   180,   204,   196,   877,   200,   206,   272,   211,
     274,   128,   129,   130,   131,   132,   280,   134,   135,   136,
     137,   274,   276,     6,     7,     8,     9,    10,   272,   967,
     277,   969,   274,   276,    64,   274,   280,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   276,   849,    18,   276,
      60,   853,   854,   956,    60,    17,   274,   859,   274,   276,
     932,   933,   934,    64,   936,   276,   274,   274,    60,   941,
       6,     7,     8,     9,    10,  1013,   274,    18,    60,   274,
     277,     3,    44,   274,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     3,   274,     3,   968,     3,   900,   274,
     902,    77,     3,   975,   976,   274,    68,     3,     3,    85,
      86,   290,     6,     7,     8,     9,    10,     3,   297,   298,
     299,   300,   301,     3,     3,  1063,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     3,   112,   113,   114,   115,
    1012,   274,  1014,  1015,  1016,   274,  1018,  1019,    12,   154,
     155,  1023,   274,   158,   272,   957,   958,   162,   960,   121,
     122,   123,   124,   125,   126,   127,    13,    25,  1071,    26,
       6,     7,     8,     9,    10,    28,   138,    60,    13,   141,
      24,  1053,    79,  1055,    20,    21,    22,    23,    24,    25,
      26,    27,    28,  1065,   274,     6,     7,     8,     9,    10,
      60,   274,    99,   274,     6,     7,     8,     9,    10,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    44,   209,     5,
     167,     6,     7,     8,     9,    10,   274,    60,    60,   174,
      60,  1043,  1044,  1045,   273,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     6,     7,     8,     9,    10,   177,
     274,    66,   274,     6,     7,     8,     9,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   274,   184,    17,   276,
     273,     6,     7,     8,     9,    10,   274,   274,    79,   274,
     274,   274,   274,   274,    80,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   276,   274,   274,   274,     3,    17,
      17,    13,    14,    17,    16,    17,   223,   224,   225,    18,
     227,   274,    81,   273,    83,   232,    85,   273,    87,    18,
      89,   274,   274,    92,    93,   276,    24,    13,    14,   278,
      16,    17,    44,    13,    16,   272,    26,    39,   128,   129,
     130,   131,   132,    39,   134,   135,   136,   137,   144,   145,
     274,   147,   148,   149,   150,   151,    68,   276,    44,   273,
     277,   240,    64,    65,   274,   278,    68,    69,    64,    65,
      72,    73,    68,    69,   276,     3,    72,    73,   274,   274,
     274,   274,    68,   274,   274,   274,     3,   274,   274,   274,
     186,   187,   188,   189,   190,   191,   192,   193,   100,   274,
     274,   277,   104,   274,   100,   274,   274,   277,   104,   121,
     122,   123,   124,   125,   126,   127,   274,   273,   214,   215,
     216,   217,   274,   274,   274,     3,   138,    13,   140,   141,
       3,     3,   278,   274,   278,   121,   122,   123,   124,   125,
     126,   127,   273,   276,   274,   274,   274,   274,   274,    18,
     274,   273,   138,   274,   140,   141,   274,   274,   128,   129,
     130,   131,   132,    39,   134,   135,   136,   137,    44,    57,
      20,   277,   268,   269,    62,   272,   274,    13,   273,   273,
      20,   274,    70,   273,   140,   274,    39,   274,    64,    65,
     274,   277,    68,    69,   274,   278,    72,    73,   274,   182,
     274,   273,   273,    79,   274,    81,    39,    95,   274,   274,
     273,    64,    65,   273,   275,    68,    69,   213,    71,    72,
      73,   275,   277,   219,   100,   221,    79,   272,   104,   272,
     272,    64,    65,   109,   274,    68,    69,   276,   273,    72,
      73,   117,   118,   182,   228,    98,    99,   100,   250,   251,
     242,   104,   274,   274,   277,   274,   750,    99,   274,   274,
     272,   274,   549,   193,   732,   277,   823,   100,   734,   755,
     332,   104,   148,   545,   781,   277,   109,   632,  1037,   603,
     772,   277,  1071,   764,   295,   857,   272,    39,   960,   165,
     166,   277,   168,   169,    -1,   171,   172,   173,   901,   175,
      -1,    -1,   178,   179,    -1,   181,    -1,    59,    -1,    -1,
      -1,    39,    64,    65,    -1,    -1,    68,    69,   194,   195,
      72,    73,    79,   199,   212,   201,    -1,   203,    -1,   205,
      -1,    -1,    -1,    -1,   210,    87,    64,    65,    -1,    -1,
      68,    69,    99,    71,    72,    73,     3,    -1,   100,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,   114,    -1,    16,
      17,   249,    -1,    -1,    -1,    -1,   254,    -1,    -1,     6,
       7,     8,   100,    -1,    -1,    -1,   104,    -1,    -1,   232,
      -1,    38,   270,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   277,   255,   256,   257,   258,    -1,    64,    -1,    -1,
      67,    -1,   265,    -1,    -1,    -1,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,   277,    39,    -1,   184,    -1,    -1,
      87,    88,    89,    90,    -1,    92,     6,     7,     8,     9,
      10,    55,    56,    -1,   277,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      -1,    -1,    -1,   220,    -1,   222,   223,   224,   225,   226,
     227,    -1,    -1,   230,    -1,   232,   233,    -1,    -1,   236,
      -1,    -1,    -1,    -1,   241,    -1,    -1,   101,    -1,    -1,
     104,    -1,   106,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   163,    -1,    -1,    -1,
      -1,    -1,     6,     7,     8,     9,    10,    -1,    -1,    -1,
     277,     6,     7,     8,    -1,   277,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    -1,    -1,    -1,    -1,   277,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   283,   284,    39,    55,    56,   101,   104,   106,
     111,   116,   117,   118,   119,   303,   304,   305,   344,   345,
     346,   348,   349,   350,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   364,   365,   366,   367,   368,   369,   370,
     371,   447,   448,   449,   450,   451,   452,   468,   470,     0,
       3,    31,    36,    39,    50,    64,    65,    67,    74,    94,
     104,   120,   164,   274,   286,   294,   299,   302,   307,   309,
     312,   314,   317,   318,   323,   329,   330,   373,   375,   385,
     416,   417,   418,   419,   453,   454,   455,   456,   464,   465,
     469,   471,   472,   473,    17,    18,    38,   372,    18,    49,
     306,     3,   102,   103,   372,   372,    85,    86,   107,   109,
     110,   212,   270,    77,    85,    86,   112,   113,   114,   115,
      13,    33,    38,   362,   372,   372,   372,    17,    18,    37,
      48,   295,    85,    91,    36,    54,    67,   109,   300,   301,
      17,    66,    17,    95,    18,    85,    17,    11,    13,    14,
      16,    17,    18,    25,    26,    28,    30,    68,   142,   143,
     153,   154,   155,   158,   272,   280,   391,   395,   400,   403,
     404,   405,   407,   409,    17,   319,   319,    57,    62,    70,
      95,   212,   249,   254,   270,   319,    39,    64,    65,    68,
      69,    71,    72,    73,   100,   104,   277,   320,   332,   319,
      44,    79,    81,   109,   117,   118,   148,   165,   166,   168,
     169,   171,   172,   173,   175,   178,   179,   181,   194,   195,
     199,   201,   203,   205,   210,   277,   320,   319,   213,   219,
     221,   277,   320,   420,   421,   422,   319,   250,   251,   277,
     320,   319,    71,    79,    98,    99,   232,   255,   256,   257,
     258,   265,   277,   320,   339,   341,   342,   319,   109,   277,
     320,   466,   467,    57,    58,    59,    61,   277,   308,   311,
     320,    59,    87,   277,   310,   320,    33,    38,   276,   333,
     306,     3,   372,   372,     3,    33,    38,   276,   336,    17,
      38,   347,     3,   108,     3,   351,   347,    71,   114,   246,
     247,   248,   347,   347,     3,     3,     3,     3,     3,     3,
       3,    13,   363,     3,     3,     3,    20,    38,   276,   296,
     333,    13,    17,   272,   285,    18,    51,   274,    13,    15,
      16,   313,   374,   315,   274,   336,   386,    13,    17,    27,
     272,   279,   399,   401,   407,    28,   272,   162,   162,    13,
     279,   402,   409,   272,   272,   272,   272,   409,    13,    14,
     272,   390,   391,   392,   393,   394,   395,   396,   397,     5,
      80,   144,   145,   147,   148,   149,   150,   151,   186,   187,
     188,   189,   190,   191,   192,   193,   214,   215,   216,   217,
     268,   269,   408,   474,     6,     7,     8,     9,    10,    20,
      21,    22,    23,    24,    25,    26,    27,    28,   276,   276,
     276,   333,    17,   322,    66,   285,    32,    33,    34,    35,
     285,   287,   331,    38,    74,    75,   163,   321,   382,   321,
      18,   336,   274,   274,   276,   183,   184,    18,   287,   202,
     185,   109,   198,    97,   167,    16,    17,   288,    60,   170,
      60,   285,   168,   176,   187,   188,   180,   182,   196,   197,
     196,   200,   287,   204,   206,   211,   274,   276,   313,   287,
     285,   274,   423,   277,   276,    64,   252,   253,   274,   276,
     462,   463,    18,   285,   276,    75,   263,   264,   457,   285,
     285,    60,    60,   166,   266,   267,   274,   274,   276,   276,
     288,   289,   274,   126,   174,   207,   271,   274,    64,   287,
      60,   287,   274,   274,   274,    60,    18,   293,   274,   274,
      76,    77,    78,    83,    84,    86,   334,   335,     3,     3,
       3,     3,    76,   105,   337,   338,     3,   347,     3,     3,
       3,    38,    64,    67,    73,    74,    87,    88,    89,    90,
      92,   289,   352,   382,     3,   347,   347,   347,   347,   347,
       3,     3,     3,    16,   285,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   297,   298,   274,   409,   285,    52,
      53,   274,   128,   129,   130,   131,   132,   134,   135,   136,
     137,   376,   377,   378,   380,   384,    96,    97,    98,   274,
     316,   274,   272,   383,   401,   401,   409,   401,   401,    12,
      13,   409,   413,   414,    25,    26,   402,   402,    28,   409,
     409,   409,   409,   273,   278,    13,    24,   278,   281,    25,
      26,   276,   278,   281,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   130,   146,   152,   159,   160,   409,   409,
     409,   409,   313,   324,   327,   328,    17,   380,   381,   409,
      60,    60,   274,   274,   285,    44,   209,   287,   288,   285,
     167,   285,    60,   285,   174,    60,   177,   285,   285,   287,
      60,   274,    66,   274,   287,   274,   287,   177,   207,   208,
     287,   276,    71,   220,   237,   239,   245,   424,   429,   432,
      17,   274,   274,   328,   274,   274,   340,   341,   274,   274,
     274,   285,   285,   274,   274,   274,    66,    93,   125,   343,
      28,   275,   291,    18,   288,   285,   285,   285,   288,   289,
     290,   277,   278,   277,   278,     3,    17,    17,   381,   290,
      17,    18,   446,   446,   446,   446,   446,   274,   274,   277,
     278,   273,    18,    17,   133,   274,   276,   288,   285,   273,
     377,   379,   384,   273,   278,   273,   273,   278,   278,   278,
     409,   278,    24,    16,   393,    13,    16,   396,   272,    26,
     326,   278,   285,   285,   274,   287,   287,   274,   274,   274,
     285,   274,   285,   274,   288,   285,   274,   274,   274,   274,
     285,   285,   274,   274,   274,   274,   274,   274,   442,   445,
     222,   287,   276,   240,   289,   425,   426,   274,   276,   274,
     276,   461,   277,   274,   274,   274,   285,    80,    82,    80,
      82,   277,   285,   288,   335,   338,     3,     3,     3,     3,
       3,   298,   378,    17,    44,    68,   121,   122,   123,   124,
     125,   126,   127,   138,   141,   387,   388,   405,   406,   410,
     415,   273,   274,   413,   409,   409,   409,   273,    13,   392,
     278,   409,    18,   289,   325,   327,   274,   278,   274,   274,
     274,   274,   274,   274,   274,   274,   328,   285,   434,   313,
     276,   430,   460,   340,   274,    18,   292,   292,   292,   292,
      20,   272,    20,   272,   409,   409,   409,   277,   415,   408,
     474,   274,   409,   411,   412,   377,   273,   273,   273,   273,
      13,   273,   289,   285,   276,   444,   277,   289,   435,   436,
     437,   427,   184,   223,   224,   225,   227,   232,   277,   339,
     431,   180,   251,   277,   459,   343,   274,   274,   274,   274,
     409,   414,   409,   405,   274,   139,   276,    20,    20,    24,
     278,   274,   277,   274,   443,   238,   274,   108,   222,   238,
     277,   428,   285,   285,   285,   182,    89,   285,    75,   231,
     234,   274,   285,   218,   259,   260,   261,   262,   458,   274,
     343,   343,   343,   343,   274,   273,   274,   474,   276,   389,
     415,   140,   393,   398,   409,   409,   154,   155,   158,   162,
     412,   114,   184,   220,   222,   223,   224,   225,   226,   227,
     230,   232,   233,   236,   241,   277,   339,   433,   274,   287,
     285,   287,   274,   285,   285,   273,   387,   140,   275,   275,
     277,   274,   274,   272,   272,   272,   274,   276,   285,   287,
     285,   285,   285,   182,   285,    89,   285,   218,   228,   229,
      75,   231,   234,   228,   285,   242,   274,   274,   277,   389,
     387,   387,   409,   409,   409,   438,   285,   285,   287,   243,
     244,   285,   398,   273,   273,   273,    16,   277,   439,   440,
     441,   274,   274,   274,   235,   274,   274
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
#line 310 "cf-parse.y"
    { return 0; ;}
    break;

  case 3:
#line 311 "cf-parse.y"
    { return 0; ;}
    break;

  case 7:
#line 324 "cf-parse.y"
    { (yyval.i) = f_eval_int((yyvsp[(2) - (3)].x)); ;}
    break;

  case 8:
#line 325 "cf-parse.y"
    { if ((yyvsp[(1) - (1)].s)->class != SYM_NUMBER) cf_error("Number expected"); else (yyval.i) = (yyvsp[(1) - (1)].s)->aux; ;}
    break;

  case 9:
#line 329 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_NUMBER, NULL);
     (yyvsp[(2) - (5)].s)->aux = (yyvsp[(4) - (5)].i);
   ;}
    break;

  case 10:
#line 333 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(2) - (5)].s), SYM_IPA, cfg_alloc(sizeof(ip_addr)));
     *(ip_addr *)(yyvsp[(2) - (5)].s)->def = (yyvsp[(4) - (5)].a);
   ;}
    break;

  case 11:
#line 342 "cf-parse.y"
    {(yyval.i) = !!(yyvsp[(1) - (1)].i); ;}
    break;

  case 12:
#line 343 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 13:
#line 344 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 14:
#line 345 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 15:
#line 346 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 16:
#line 347 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 18:
#line 354 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_IPA) cf_error("IP address expected");
     (yyval.a) = *(ip_addr *)(yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 19:
#line 361 "cf-parse.y"
    {
     if (!ip_is_prefix((yyvsp[(1) - (2)].a), (yyvsp[(2) - (2)].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[(1) - (2)].a); (yyval.px).len = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 21:
#line 369 "cf-parse.y"
    { (yyval.px).addr = (yyvsp[(1) - (1)].a); (yyval.px).len = BITS_PER_IP_ADDRESS; ;}
    break;

  case 22:
#line 373 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[(2) - (2)].i));
     (yyval.i) = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 23:
#line 377 "cf-parse.y"
    {
     (yyval.i) = ipa_mklen((yyvsp[(2) - (2)].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[(2) - (2)].a));
   ;}
    break;

  case 24:
#line 384 "cf-parse.y"
    {
     (yyval.time) = tm_parse_datetime((yyvsp[(1) - (1)].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   ;}
    break;

  case 25:
#line 392 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].t); ;}
    break;

  case 26:
#line 393 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 27:
#line 399 "cf-parse.y"
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[(2) - (4)].g);
    c->mask = (yyvsp[(3) - (4)].i);
    add_tail(&new_config->logfiles, &c->n);
  ;}
    break;

  case 28:
#line 408 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(1) - (1)].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[(1) - (1)].t));
     (yyval.g) = f;
   ;}
    break;

  case 29:
#line 413 "cf-parse.y"
    { (yyval.g) = NULL; ;}
    break;

  case 30:
#line 414 "cf-parse.y"
    { (yyval.g) = stderr; ;}
    break;

  case 31:
#line 418 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 32:
#line 419 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 33:
#line 423 "cf-parse.y"
    { (yyval.i) = 1 << (yyvsp[(1) - (1)].i); ;}
    break;

  case 34:
#line 424 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (1 << (yyvsp[(3) - (3)].i)); ;}
    break;

  case 35:
#line 428 "cf-parse.y"
    { (yyval.i) = L_DEBUG[0]; ;}
    break;

  case 36:
#line 429 "cf-parse.y"
    { (yyval.i) = L_TRACE[0]; ;}
    break;

  case 37:
#line 430 "cf-parse.y"
    { (yyval.i) = L_INFO[0]; ;}
    break;

  case 38:
#line 431 "cf-parse.y"
    { (yyval.i) = L_REMOTE[0]; ;}
    break;

  case 39:
#line 432 "cf-parse.y"
    { (yyval.i) = L_WARN[0]; ;}
    break;

  case 40:
#line 433 "cf-parse.y"
    { (yyval.i) = L_ERR[0]; ;}
    break;

  case 41:
#line 434 "cf-parse.y"
    { (yyval.i) = L_AUTH[0]; ;}
    break;

  case 42:
#line 435 "cf-parse.y"
    { (yyval.i) = L_FATAL[0]; ;}
    break;

  case 43:
#line 436 "cf-parse.y"
    { (yyval.i) = L_BUG[0]; ;}
    break;

  case 44:
#line 442 "cf-parse.y"
    { new_config->proto_default_mrtdump = (yyvsp[(3) - (4)].i); ;}
    break;

  case 45:
#line 443 "cf-parse.y"
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[(2) - (3)].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[(2) - (3)].t));
     new_config->mrtdump_file = fileno(f);
   ;}
    break;

  case 46:
#line 452 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_route; ;}
    break;

  case 47:
#line 453 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_proto; ;}
    break;

  case 48:
#line 454 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_base; ;}
    break;

  case 49:
#line 455 "cf-parse.y"
    { (yyval.tf) = &new_config->tf_log; ;}
    break;

  case 50:
#line 458 "cf-parse.y"
    { *(yyvsp[(1) - (2)].tf) = (struct timeformat){(yyvsp[(2) - (2)].t), NULL, 0}; ;}
    break;

  case 51:
#line 459 "cf-parse.y"
    { *(yyvsp[(1) - (4)].tf) = (struct timeformat){(yyvsp[(2) - (4)].t), (yyvsp[(4) - (4)].t), (yyvsp[(3) - (4)].i)}; ;}
    break;

  case 52:
#line 460 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%T", "%F", 20*3600}; ;}
    break;

  case 53:
#line 461 "cf-parse.y"
    { *(yyvsp[(1) - (3)].tf) = (struct timeformat){"%F %T", NULL, 0}; ;}
    break;

  case 55:
#line 473 "cf-parse.y"
    { cmd_reconfig((yyvsp[(2) - (3)].t), RECONFIG_HARD); ;}
    break;

  case 56:
#line 476 "cf-parse.y"
    { cmd_reconfig((yyvsp[(3) - (4)].t), RECONFIG_SOFT); ;}
    break;

  case 57:
#line 479 "cf-parse.y"
    { cli_msg(7, "Shutdown requested"); order_shutdown(); ;}
    break;

  case 58:
#line 482 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 60:
#line 491 "cf-parse.y"
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

  case 61:
#line 507 "cf-parse.y"
    { THIS_KRT->persist = (yyvsp[(2) - (2)].i); ;}
    break;

  case 62:
#line 508 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 63:
#line 512 "cf-parse.y"
    {
      THIS_KRT->learn = (yyvsp[(2) - (2)].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[(2) - (2)].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   ;}
    break;

  case 64:
#line 524 "cf-parse.y"
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

  case 65:
#line 537 "cf-parse.y"
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 66:
#line 541 "cf-parse.y"
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[(2) - (3)].t);
     kpi->prefix = (yyvsp[(3) - (3)].px).addr;
     kpi->pxlen = (yyvsp[(3) - (3)].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   ;}
    break;

  case 67:
#line 554 "cf-parse.y"
    {
	if ((yyvsp[(3) - (3)].i) <= 0 || (yyvsp[(3) - (3)].i) >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->scan.table_id = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 68:
#line 566 "cf-parse.y"
    {
   new_config->router_id = (yyvsp[(3) - (4)].i32);
   ;}
    break;

  case 69:
#line 572 "cf-parse.y"
    { (yyval.i32) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 71:
#line 574 "cf-parse.y"
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[(1) - (1)].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   ;}
    break;

  case 75:
#line 593 "cf-parse.y"
    { new_config->listen_bgp_addr = (yyvsp[(2) - (2)].a); ;}
    break;

  case 76:
#line 594 "cf-parse.y"
    { new_config->listen_bgp_port = (yyvsp[(2) - (2)].i); ;}
    break;

  case 77:
#line 595 "cf-parse.y"
    { new_config->listen_bgp_flags |= SKF_V6ONLY; ;}
    break;

  case 78:
#line 602 "cf-parse.y"
    {
   rt_new_table((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 80:
#line 614 "cf-parse.y"
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     ;}
    break;

  case 81:
#line 620 "cf-parse.y"
    {
     cf_define_symbol((yyvsp[(1) - (1)].s), SYM_PROTO, this_proto);
     this_proto->name = (yyvsp[(1) - (1)].s)->name;
   ;}
    break;

  case 83:
#line 628 "cf-parse.y"
    {
     if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[(2) - (2)].i);
   ;}
    break;

  case 84:
#line 632 "cf-parse.y"
    { this_proto->disabled = (yyvsp[(2) - (2)].i); ;}
    break;

  case 85:
#line 633 "cf-parse.y"
    { this_proto->debug = (yyvsp[(2) - (2)].i); ;}
    break;

  case 86:
#line 634 "cf-parse.y"
    { this_proto->mrtdump = (yyvsp[(2) - (2)].i); ;}
    break;

  case 87:
#line 635 "cf-parse.y"
    { this_proto->in_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 88:
#line 636 "cf-parse.y"
    { this_proto->out_filter = (yyvsp[(2) - (2)].f); ;}
    break;

  case 89:
#line 637 "cf-parse.y"
    { this_proto->table = (yyvsp[(2) - (2)].r); ;}
    break;

  case 90:
#line 638 "cf-parse.y"
    { this_proto->router_id = (yyvsp[(3) - (3)].i32); ;}
    break;

  case 91:
#line 639 "cf-parse.y"
    { this_proto->dsc = (yyvsp[(2) - (2)].t); ;}
    break;

  case 92:
#line 643 "cf-parse.y"
    { (yyval.f) = (yyvsp[(2) - (2)].f); ;}
    break;

  case 94:
#line 645 "cf-parse.y"
    { (yyval.f) = FILTER_ACCEPT; ;}
    break;

  case 95:
#line 646 "cf-parse.y"
    { (yyval.f) = FILTER_REJECT; ;}
    break;

  case 96:
#line 650 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 97:
#line 658 "cf-parse.y"
    { new_config->proto_default_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 98:
#line 659 "cf-parse.y"
    { new_config->cli_debug = (yyvsp[(3) - (3)].i); ;}
    break;

  case 99:
#line 667 "cf-parse.y"
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   ;}
    break;

  case 100:
#line 675 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (1)].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; ;}
    break;

  case 101:
#line 676 "cf-parse.y"
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[(1) - (1)].px).addr; this_ipn->pxlen = (yyvsp[(1) - (1)].px).len; ;}
    break;

  case 102:
#line 677 "cf-parse.y"
    { this_ipn->pattern = (yyvsp[(1) - (2)].t); this_ipn->prefix = (yyvsp[(2) - (2)].px).addr; this_ipn->pxlen = (yyvsp[(2) - (2)].px).len; ;}
    break;

  case 103:
#line 681 "cf-parse.y"
    { this_ipn->positive = 1; ;}
    break;

  case 104:
#line 682 "cf-parse.y"
    { this_ipn->positive = 0; ;}
    break;

  case 108:
#line 699 "cf-parse.y"
    {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   ;}
    break;

  case 112:
#line 714 "cf-parse.y"
    {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   ;}
    break;

  case 114:
#line 729 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 115:
#line 730 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 116:
#line 731 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 118:
#line 736 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 119:
#line 740 "cf-parse.y"
    { (yyval.i) = D_STATES; ;}
    break;

  case 120:
#line 741 "cf-parse.y"
    { (yyval.i) = D_ROUTES; ;}
    break;

  case 121:
#line 742 "cf-parse.y"
    { (yyval.i) = D_FILTERS; ;}
    break;

  case 122:
#line 743 "cf-parse.y"
    { (yyval.i) = D_IFACES; ;}
    break;

  case 123:
#line 744 "cf-parse.y"
    { (yyval.i) = D_EVENTS; ;}
    break;

  case 124:
#line 745 "cf-parse.y"
    { (yyval.i) = D_PACKETS; ;}
    break;

  case 125:
#line 751 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 126:
#line 752 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 127:
#line 753 "cf-parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 129:
#line 758 "cf-parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i); ;}
    break;

  case 130:
#line 762 "cf-parse.y"
    { (yyval.i) = MD_STATES; ;}
    break;

  case 131:
#line 763 "cf-parse.y"
    { (yyval.i) = MD_MESSAGES; ;}
    break;

  case 138:
#line 784 "cf-parse.y"
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

  case 139:
#line 802 "cf-parse.y"
    { ;}
    break;

  case 140:
#line 803 "cf-parse.y"
    { this_p_item->genfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 141:
#line 804 "cf-parse.y"
    { this_p_item->gento = (yyvsp[(3) - (5)].time); ;}
    break;

  case 142:
#line 805 "cf-parse.y"
    { this_p_item->accfrom = (yyvsp[(3) - (5)].time); ;}
    break;

  case 143:
#line 806 "cf-parse.y"
    { this_p_item->accto = (yyvsp[(3) - (5)].time); ;}
    break;

  case 144:
#line 807 "cf-parse.y"
    { this_p_item->id = (yyvsp[(2) - (4)].i); if ((yyvsp[(2) - (4)].i) <= 0) cf_error("Password ID has to be greated than zero."); ;}
    break;

  case 145:
#line 816 "cf-parse.y"
    { cmd_show_status(); ;}
    break;

  case 146:
#line 819 "cf-parse.y"
    { proto_show((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 147:
#line 822 "cf-parse.y"
    { proto_show((yyvsp[(4) - (5)].s), 1); ;}
    break;

  case 149:
#line 826 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 150:
#line 830 "cf-parse.y"
    { if_show(); ;}
    break;

  case 151:
#line 833 "cf-parse.y"
    { if_show_summary(); ;}
    break;

  case 152:
#line 836 "cf-parse.y"
    { rt_show((yyvsp[(3) - (4)].ra)); ;}
    break;

  case 153:
#line 839 "cf-parse.y"
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
     (yyval.ra)->table = config->master_rtc->table;
   ;}
    break;

  case 154:
#line 845 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(2) - (2)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(2) - (2)].px).len;
   ;}
    break;

  case 155:
#line 851 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[(3) - (3)].px).addr;
     (yyval.ra)->pxlen = (yyvsp[(3) - (3)].px).len;
     (yyval.ra)->show_for = 1;
   ;}
    break;

  case 156:
#line 858 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyvsp[(3) - (3)].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[(3) - (3)].s)->def)->table;
   ;}
    break;

  case 157:
#line 863 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(3) - (3)].f);
   ;}
    break;

  case 158:
#line 868 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[(2) - (2)].f);
   ;}
    break;

  case 159:
#line 873 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->verbose = 1;
   ;}
    break;

  case 160:
#line 877 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->primary_only = 1;
   ;}
    break;

  case 161:
#line 881 "cf-parse.y"
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

  case 162:
#line 891 "cf-parse.y"
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[(3) - (3)].s)->def;
     (yyval.ra) = (yyvsp[(1) - (3)].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[(3) - (3)].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[(3) - (3)].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   ;}
    break;

  case 163:
#line 899 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 1;
   ;}
    break;

  case 164:
#line 903 "cf-parse.y"
    {
     (yyval.ra) = (yyvsp[(1) - (2)].ra);
     (yyval.ra)->stats = 2;
   ;}
    break;

  case 165:
#line 910 "cf-parse.y"
    { (yyval.i) = 1; ;}
    break;

  case 166:
#line 911 "cf-parse.y"
    { (yyval.i) = 2; ;}
    break;

  case 167:
#line 915 "cf-parse.y"
    { cmd_show_symbols((yyvsp[(3) - (4)].s)); ;}
    break;

  case 168:
#line 919 "cf-parse.y"
    { rdump(&root_pool); cli_msg(0, ""); ;}
    break;

  case 169:
#line 921 "cf-parse.y"
    { sk_dump_all(); cli_msg(0, ""); ;}
    break;

  case 170:
#line 923 "cf-parse.y"
    { if_dump_all(); cli_msg(0, ""); ;}
    break;

  case 171:
#line 925 "cf-parse.y"
    { neigh_dump_all(); cli_msg(0, ""); ;}
    break;

  case 172:
#line 927 "cf-parse.y"
    { rta_dump_all(); cli_msg(0, ""); ;}
    break;

  case 173:
#line 929 "cf-parse.y"
    { rt_dump_all(); cli_msg(0, ""); ;}
    break;

  case 174:
#line 931 "cf-parse.y"
    { protos_dump_all(); cli_msg(0, ""); ;}
    break;

  case 175:
#line 933 "cf-parse.y"
    {
  cli_set_log_echo(this_cli, (yyvsp[(2) - (4)].i), (yyvsp[(3) - (4)].i));
  cli_msg(0, "");
;}
    break;

  case 176:
#line 939 "cf-parse.y"
    { (yyval.i) = ~0; ;}
    break;

  case 177:
#line 940 "cf-parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 179:
#line 945 "cf-parse.y"
    { (yyval.i) = 4096; ;}
    break;

  case 180:
#line 946 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].i) < 256 || (yyvsp[(1) - (1)].i) > 65536) cf_error("Invalid log buffer size");
     (yyval.i) = (yyvsp[(1) - (1)].i);
   ;}
    break;

  case 181:
#line 953 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), XX_DISABLE); ;}
    break;

  case 182:
#line 955 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), XX_ENABLE); ;}
    break;

  case 183:
#line 957 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), XX_RESTART); ;}
    break;

  case 184:
#line 959 "cf-parse.y"
    { proto_xxable((yyvsp[(2) - (3)].t), XX_RELOAD); ;}
    break;

  case 185:
#line 961 "cf-parse.y"
    { proto_xxable((yyvsp[(3) - (4)].t), XX_RELOAD_IN); ;}
    break;

  case 186:
#line 963 "cf-parse.y"
    { proto_xxable((yyvsp[(3) - (4)].t), XX_RELOAD_OUT); ;}
    break;

  case 187:
#line 967 "cf-parse.y"
    { proto_debug((yyvsp[(2) - (4)].t), 0, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 188:
#line 972 "cf-parse.y"
    { proto_debug((yyvsp[(2) - (4)].t), 1, (yyvsp[(3) - (4)].i)); ;}
    break;

  case 189:
#line 976 "cf-parse.y"
    { (yyval.t) = (yyvsp[(1) - (1)].s)->name; ;}
    break;

  case 190:
#line 977 "cf-parse.y"
    { (yyval.t) = "*"; ;}
    break;

  case 192:
#line 984 "cf-parse.y"
    { cf_push_scope( (yyvsp[(2) - (2)].s) ); ;}
    break;

  case 193:
#line 984 "cf-parse.y"
    {
     (yyvsp[(2) - (4)].s) = cf_define_symbol((yyvsp[(2) - (4)].s), SYM_FILTER, (yyvsp[(4) - (4)].f));
     (yyvsp[(4) - (4)].f)->name = (yyvsp[(2) - (4)].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[(2) - (4)].s)->name );
     cf_pop_scope();
   ;}
    break;

  case 194:
#line 993 "cf-parse.y"
    { f_eval_int((yyvsp[(2) - (2)].x)); ;}
    break;

  case 195:
#line 997 "cf-parse.y"
    { (yyval.i) = T_INT; ;}
    break;

  case 196:
#line 998 "cf-parse.y"
    { (yyval.i) = T_BOOL; ;}
    break;

  case 197:
#line 999 "cf-parse.y"
    { (yyval.i) = T_IP; ;}
    break;

  case 198:
#line 1000 "cf-parse.y"
    { (yyval.i) = T_PREFIX; ;}
    break;

  case 199:
#line 1001 "cf-parse.y"
    { (yyval.i) = T_PAIR; ;}
    break;

  case 200:
#line 1002 "cf-parse.y"
    { (yyval.i) = T_STRING; ;}
    break;

  case 201:
#line 1003 "cf-parse.y"
    { (yyval.i) = T_PATH_MASK; ;}
    break;

  case 202:
#line 1004 "cf-parse.y"
    { (yyval.i) = T_PATH; ;}
    break;

  case 203:
#line 1005 "cf-parse.y"
    { (yyval.i) = T_CLIST; ;}
    break;

  case 204:
#line 1006 "cf-parse.y"
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

  case 205:
#line 1025 "cf-parse.y"
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val)); 
     val->type = (yyvsp[(1) - (2)].i); 
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_VARIABLE | (yyvsp[(1) - (2)].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[(2) - (2)].s)->name, (yyvsp[(1) - (2)].i) );
     (yyvsp[(2) - (2)].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[(2) - (2)].s);
   ;}
    break;

  case 206:
#line 1036 "cf-parse.y"
    { (yyval.s) = NULL; ;}
    break;

  case 207:
#line 1037 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 208:
#line 1044 "cf-parse.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 209:
#line 1045 "cf-parse.y"
    {
     (yyval.s) = (yyvsp[(1) - (3)].s);
     (yyval.s)->aux2 = (yyvsp[(3) - (3)].s);
   ;}
    break;

  case 210:
#line 1052 "cf-parse.y"
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[(1) - (1)].x);
     (yyval.f) = f;
   ;}
    break;

  case 211:
#line 1061 "cf-parse.y"
    {
     if ((yyvsp[(1) - (1)].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[(1) - (1)].s)->def;
   ;}
    break;

  case 213:
#line 1069 "cf-parse.y"
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

  case 214:
#line 1093 "cf-parse.y"
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[(2) - (3)].s); ;}
    break;

  case 215:
#line 1094 "cf-parse.y"
    { (yyval.s)=NULL; ;}
    break;

  case 216:
#line 1098 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 217:
#line 1104 "cf-parse.y"
    { DBG( "Beginning of function %s\n", (yyvsp[(2) - (2)].s)->name );
     (yyvsp[(2) - (2)].s) = cf_define_symbol((yyvsp[(2) - (2)].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[(2) - (2)].s));
   ;}
    break;

  case 218:
#line 1107 "cf-parse.y"
    {
     (yyvsp[(2) - (5)].s)->def = (yyvsp[(5) - (5)].x);
     (yyvsp[(2) - (5)].s)->aux2 = (yyvsp[(4) - (5)].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[(2) - (5)].s)->name); 
     cf_pop_scope();
   ;}
    break;

  case 219:
#line 1120 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 220:
#line 1121 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x)->next; (yyvsp[(1) - (1)].x)->next = NULL; ;}
    break;

  case 221:
#line 1124 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); (yyvsp[(1) - (1)].x)->next = (yyvsp[(1) - (1)].x); ;}
    break;

  case 222:
#line 1125 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyvsp[(2) - (2)].x)->next = (yyvsp[(1) - (2)].x)->next ; (yyvsp[(1) - (2)].x)->next = (yyvsp[(2) - (2)].x); ;}
    break;

  case 223:
#line 1129 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(1) - (1)].x);
   ;}
    break;

  case 224:
#line 1132 "cf-parse.y"
    {
     (yyval.x)=(yyvsp[(2) - (3)].x);
   ;}
    break;

  case 225:
#line 1141 "cf-parse.y"
    { (yyval.i) = make_pair((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); ;}
    break;

  case 226:
#line 1148 "cf-parse.y"
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[(1) - (1)].a); ;}
    break;

  case 227:
#line 1152 "cf-parse.y"
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 228:
#line 1153 "cf-parse.y"
    { (yyval.v).type = T_PAIR; (yyval.v).val.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 229:
#line 1154 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 230:
#line 1155 "cf-parse.y"
    {  (yyval.v).type = (yyvsp[(1) - (1)].i) >> 16; (yyval.v).val.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 231:
#line 1159 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (1)].v); 
	(yyval.e)->to = (yyvsp[(1) - (1)].v);
   ;}
    break;

  case 232:
#line 1164 "cf-parse.y"
    { 
	(yyval.e) = f_new_tree(); 
	(yyval.e)->from = (yyvsp[(1) - (4)].v); 
	(yyval.e)->to = (yyvsp[(4) - (4)].v); 
   ;}
    break;

  case 233:
#line 1172 "cf-parse.y"
    { (yyval.e) = (yyvsp[(1) - (1)].e); ;}
    break;

  case 234:
#line 1173 "cf-parse.y"
    { (yyval.e) = (yyvsp[(3) - (3)].e); (yyval.e)->left = (yyvsp[(1) - (3)].e); ;}
    break;

  case 235:
#line 1177 "cf-parse.y"
    {
     if (((yyvsp[(3) - (3)].i) < 0) || ((yyvsp[(3) - (3)].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[(1) - (3)].a), (yyvsp[(3) - (3)].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[(1) - (3)].a); (yyval.v).val.px.len = (yyvsp[(3) - (3)].i);
   ;}
    break;

  case 236:
#line 1184 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (1)].v); ;}
    break;

  case 237:
#line 1185 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_PLUS; ;}
    break;

  case 238:
#line 1186 "cf-parse.y"
    { (yyval.v) = (yyvsp[(1) - (2)].v); (yyval.v).val.px.len |= LEN_MINUS; ;}
    break;

  case 239:
#line 1187 "cf-parse.y"
    { 
     if (! ((0 <= (yyvsp[(3) - (6)].i)) && ((yyvsp[(3) - (6)].i) <= (yyvsp[(5) - (6)].i)) && ((yyvsp[(5) - (6)].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[(3) - (6)].i), (yyvsp[(5) - (6)].i));
     (yyval.v) = (yyvsp[(1) - (6)].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[(3) - (6)].i) << 16) | ((yyvsp[(5) - (6)].i) << 8);
   ;}
    break;

  case 240:
#line 1194 "cf-parse.y"
    { (yyval.trie) = f_new_trie(); trie_add_prefix((yyval.trie), &((yyvsp[(1) - (1)].v).val.px)); ;}
    break;

  case 241:
#line 1195 "cf-parse.y"
    { (yyval.trie) = (yyvsp[(1) - (3)].trie); trie_add_prefix((yyval.trie), &((yyvsp[(3) - (3)].v).val.px)); ;}
    break;

  case 242:
#line 1198 "cf-parse.y"
    { (yyval.e) = NULL; ;}
    break;

  case 243:
#line 1199 "cf-parse.y"
    {
     (yyval.e) = (yyvsp[(1) - (4)].e);
     (yyval.e)->data = (yyvsp[(3) - (4)].x);
     (yyval.e)->left = (yyvsp[(4) - (4)].e);
   ;}
    break;

  case 244:
#line 1204 "cf-parse.y"
    {
     (yyval.e) = f_new_tree(); 
     (yyval.e)->from.type = T_VOID; 
     (yyval.e)->to.type = T_VOID;
     (yyval.e)->data = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 245:
#line 1215 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 246:
#line 1216 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 247:
#line 1220 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 248:
#line 1221 "cf-parse.y"
    { (yyval.h) = (yyvsp[(2) - (3)].h); ;}
    break;

  case 249:
#line 1225 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 250:
#line 1226 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 251:
#line 1227 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_QUESTION; (yyval.h)->val  = 0; ;}
    break;

  case 252:
#line 1228 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[(1) - (2)].x); ;}
    break;

  case 253:
#line 1229 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 254:
#line 1233 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASN;      (yyval.h)->val = (yyvsp[(1) - (2)].i); ;}
    break;

  case 255:
#line 1234 "cf-parse.y"
    { (yyval.h) = cfg_alloc(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[(2) - (2)].h); (yyval.h)->kind = PM_ASTERISK; (yyval.h)->val  = 0; ;}
    break;

  case 256:
#line 1235 "cf-parse.y"
    { (yyval.h) = NULL; ;}
    break;

  case 257:
#line 1239 "cf-parse.y"
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

  case 258:
#line 1252 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[(1) - (1)].i); ;}
    break;

  case 259:
#line 1253 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  ;}
    break;

  case 260:
#line 1254 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  ;}
    break;

  case 261:
#line 1255 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[(1) - (1)].t); ;}
    break;

  case 262:
#line 1256 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 263:
#line 1257 "cf-parse.y"
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[(1) - (1)].v); ;}
    break;

  case 264:
#line 1258 "cf-parse.y"
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[(2) - (3)].e)); DBG( "ook\n" ); ;}
    break;

  case 265:
#line 1259 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[(2) - (3)].trie); ;}
    break;

  case 266:
#line 1260 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[(1) - (1)].i) >> 16; (yyval.x)->a2.i = (yyvsp[(1) - (1)].i) & 0xffff; ;}
    break;

  case 267:
#line 1261 "cf-parse.y"
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[(1) - (1)].h); (yyval.x)->a1.p = val; ;}
    break;

  case 269:
#line 1275 "cf-parse.y"
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

  case 270:
#line 1298 "cf-parse.y"
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

  case 271:
#line 1331 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, from);   (yyval.x)->a1.i = 1; ;}
    break;

  case 272:
#line 1333 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = OFFSETOF(struct rta, gw);     (yyval.x)->a1.i = 1; ;}
    break;

  case 273:
#line 1334 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ ;}
    break;

  case 274:
#line 1335 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = 0x12345678; /* T_STRING is also special-cased. */ ;}
    break;

  case 275:
#line 1336 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = OFFSETOF(struct rta, source); ;}
    break;

  case 276:
#line 1337 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = OFFSETOF(struct rta, scope);  (yyval.x)->a1.i = 1; ;}
    break;

  case 277:
#line 1338 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = OFFSETOF(struct rta, cast); ;}
    break;

  case 278:
#line 1339 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = OFFSETOF(struct rta, dest); ;}
    break;

  case 279:
#line 1343 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (3)].x); ;}
    break;

  case 280:
#line 1344 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 281:
#line 1345 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 282:
#line 1346 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 283:
#line 1347 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 284:
#line 1348 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 285:
#line 1349 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 286:
#line 1350 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 287:
#line 1351 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 288:
#line 1352 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 289:
#line 1353 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 290:
#line 1354 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 291:
#line 1355 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[(3) - (3)].x); (yyval.x)->a2.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 292:
#line 1356 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->a2.p = (yyvsp[(3) - (3)].x); ;}
    break;

  case 293:
#line 1357 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[(2) - (2)].x); ;}
    break;

  case 294:
#line 1358 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[(3) - (4)].x); ;}
    break;

  case 295:
#line 1360 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 296:
#line 1361 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 297:
#line 1362 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 298:
#line 1364 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; ;}
    break;

  case 299:
#line 1366 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = 'a'; ;}
    break;

  case 300:
#line 1368 "cf-parse.y"
    { (yyval.x) = (yyvsp[(2) - (2)].x); (yyval.x)->code = P('e','a'); ;}
    break;

  case 301:
#line 1370 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); (yyval.x)->aux = T_IP; ;}
    break;

  case 302:
#line 1371 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 303:
#line 1372 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[(1) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 304:
#line 1373 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 305:
#line 1374 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[(1) - (3)].x); ;}
    break;

  case 306:
#line 1384 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; ;}
    break;

  case 307:
#line 1385 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; ;}
    break;

  case 308:
#line 1386 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); ;}
    break;

  case 309:
#line 1387 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'a'; ;}
    break;

  case 310:
#line 1388 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[(3) - (6)].x); (yyval.x)->a2.p = (yyvsp[(5) - (6)].x); (yyval.x)->aux = 'd'; ;}
    break;

  case 311:
#line 1393 "cf-parse.y"
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

  case 312:
#line 1416 "cf-parse.y"
    { (yyval.i) = F_QUITBIRD; ;}
    break;

  case 313:
#line 1417 "cf-parse.y"
    { (yyval.i) = F_ACCEPT; ;}
    break;

  case 314:
#line 1418 "cf-parse.y"
    { (yyval.i) = F_REJECT; ;}
    break;

  case 315:
#line 1419 "cf-parse.y"
    { (yyval.i) = F_ERROR; ;}
    break;

  case 316:
#line 1420 "cf-parse.y"
    { (yyval.i) = F_NOP; ;}
    break;

  case 317:
#line 1421 "cf-parse.y"
    { (yyval.i) = F_NONL; ;}
    break;

  case 318:
#line 1425 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[(1) - (1)].x); (yyval.x)->a2.p = NULL; ;}
    break;

  case 319:
#line 1428 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 320:
#line 1429 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 321:
#line 1430 "cf-parse.y"
    {
     if ((yyvsp[(1) - (3)].x)) {
       (yyvsp[(1) - (3)].x)->next = (yyvsp[(3) - (3)].x);
       (yyval.x) = (yyvsp[(1) - (3)].x);
     } else (yyval.x) = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 322:
#line 1439 "cf-parse.y"
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (1)].x);
     (yyval.x)->next = NULL;
   ;}
    break;

  case 323:
#line 1446 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[(1) - (3)].x);
     (yyval.x)->next = (yyvsp[(3) - (3)].x);
   ;}
    break;

  case 324:
#line 1455 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 325:
#line 1456 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (1)].x); ;}
    break;

  case 326:
#line 1460 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[(2) - (4)].x);
     (yyval.x)->a2.p = (yyvsp[(4) - (4)].x);
   ;}
    break;

  case 327:
#line 1466 "cf-parse.y"
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

  case 328:
#line 1476 "cf-parse.y"
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

  case 329:
#line 1485 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[(2) - (3)].x);
   ;}
    break;

  case 330:
#line 1491 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 331:
#line 1496 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(2) - (5)].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[(4) - (5)].x);
   ;}
    break;

  case 332:
#line 1503 "cf-parse.y"
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[(3) - (4)].x);
   ;}
    break;

  case 333:
#line 1508 "cf-parse.y"
    {
     (yyval.x) = (yyvsp[(4) - (6)].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   ;}
    break;

  case 334:
#line 1514 "cf-parse.y"
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[(2) - (3)].x); (yyval.x)->a2.i = (yyvsp[(1) - (3)].i); ;}
    break;

  case 335:
#line 1515 "cf-parse.y"
    { (yyval.x) = (yyvsp[(1) - (2)].x); ;}
    break;

  case 336:
#line 1516 "cf-parse.y"
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[(2) - (5)].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[(4) - (5)].e) );
   ;}
    break;

  case 337:
#line 1525 "cf-parse.y"
    { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; (yyval.x) = (yyvsp[(2) - (5)].x); (yyval.x)->code = P('e','S'); (yyval.x)->a1.p = i; ;}
    break;

  case 338:
#line 1526 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 339:
#line 1527 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 340:
#line 1528 "cf-parse.y"
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[(2) - (8)].x), (yyvsp[(6) - (8)].x) ); ;}
    break;

  case 341:
#line 1534 "cf-parse.y"
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

  case 344:
#line 1557 "cf-parse.y"
    { BGP_CFG->local_as = (yyvsp[(4) - (5)].i); ;}
    break;

  case 345:
#line 1558 "cf-parse.y"
    {
     if (ipa_nonzero(BGP_CFG->remote_ip)) cf_error("Only one neighbor per BGP instance is allowed");

     BGP_CFG->remote_ip = (yyvsp[(3) - (6)].a);
     BGP_CFG->remote_as = (yyvsp[(5) - (6)].i);
   ;}
    break;

  case 346:
#line 1564 "cf-parse.y"
    { BGP_CFG->rr_cluster_id = (yyvsp[(5) - (6)].i); ;}
    break;

  case 347:
#line 1565 "cf-parse.y"
    { BGP_CFG->rr_client = 1; ;}
    break;

  case 348:
#line 1566 "cf-parse.y"
    { BGP_CFG->rs_client = 1; ;}
    break;

  case 349:
#line 1567 "cf-parse.y"
    { BGP_CFG->hold_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 350:
#line 1568 "cf-parse.y"
    { BGP_CFG->initial_hold_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 351:
#line 1569 "cf-parse.y"
    { BGP_CFG->connect_retry_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 352:
#line 1570 "cf-parse.y"
    { BGP_CFG->keepalive_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 353:
#line 1571 "cf-parse.y"
    { BGP_CFG->multihop = (yyvsp[(3) - (6)].i); BGP_CFG->multihop_via = (yyvsp[(5) - (6)].a); ;}
    break;

  case 354:
#line 1572 "cf-parse.y"
    { BGP_CFG->next_hop_self = 1; ;}
    break;

  case 355:
#line 1573 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_SELF; ;}
    break;

  case 356:
#line 1574 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_DROP; ;}
    break;

  case 357:
#line 1575 "cf-parse.y"
    { BGP_CFG->missing_lladdr = MLL_IGNORE; ;}
    break;

  case 358:
#line 1576 "cf-parse.y"
    { BGP_CFG->compare_path_lengths = (yyvsp[(4) - (5)].i); ;}
    break;

  case 359:
#line 1577 "cf-parse.y"
    { BGP_CFG->prefer_older = (yyvsp[(4) - (5)].i); ;}
    break;

  case 360:
#line 1578 "cf-parse.y"
    { BGP_CFG->default_med = (yyvsp[(4) - (5)].i); ;}
    break;

  case 361:
#line 1579 "cf-parse.y"
    { BGP_CFG->default_local_pref = (yyvsp[(4) - (5)].i); ;}
    break;

  case 362:
#line 1580 "cf-parse.y"
    { BGP_CFG->source_addr = (yyvsp[(4) - (5)].a); ;}
    break;

  case 363:
#line 1581 "cf-parse.y"
    { BGP_CFG->start_delay_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 364:
#line 1582 "cf-parse.y"
    { BGP_CFG->error_amnesia_time = (yyvsp[(5) - (6)].i); ;}
    break;

  case 365:
#line 1583 "cf-parse.y"
    { BGP_CFG->error_delay_time_min = (yyvsp[(5) - (8)].i); BGP_CFG->error_delay_time_max = (yyvsp[(7) - (8)].i); ;}
    break;

  case 366:
#line 1584 "cf-parse.y"
    { BGP_CFG->disable_after_error = (yyvsp[(5) - (6)].i); ;}
    break;

  case 367:
#line 1585 "cf-parse.y"
    { BGP_CFG->enable_refresh = (yyvsp[(5) - (6)].i); ;}
    break;

  case 368:
#line 1586 "cf-parse.y"
    { BGP_CFG->enable_as4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 369:
#line 1587 "cf-parse.y"
    { BGP_CFG->capabilities = (yyvsp[(3) - (4)].i); ;}
    break;

  case 370:
#line 1588 "cf-parse.y"
    { BGP_CFG->advertise_ipv4 = (yyvsp[(4) - (5)].i); ;}
    break;

  case 371:
#line 1589 "cf-parse.y"
    { BGP_CFG->password = (yyvsp[(3) - (4)].t); ;}
    break;

  case 372:
#line 1590 "cf-parse.y"
    { BGP_CFG->route_limit = (yyvsp[(4) - (5)].i); ;}
    break;

  case 373:
#line 1591 "cf-parse.y"
    { BGP_CFG->passive = (yyvsp[(3) - (4)].i); ;}
    break;

  case 374:
#line 1592 "cf-parse.y"
    { BGP_CFG->interpret_communities = (yyvsp[(4) - (5)].i); ;}
    break;

  case 375:
#line 1601 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  ;}
    break;

  case 379:
#line 1617 "cf-parse.y"
    { OSPF_CFG->rfc1583 = (yyvsp[(2) - (2)].i); ;}
    break;

  case 380:
#line 1618 "cf-parse.y"
    { OSPF_CFG->tick = (yyvsp[(2) - (2)].i) ; if((yyvsp[(2) - (2)].i)<=0) cf_error("Tick must be greater than zero"); ;}
    break;

  case 382:
#line 1622 "cf-parse.y"
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

  case 386:
#line 1643 "cf-parse.y"
    { this_area->stub = (yyvsp[(3) - (3)].i) ; if((yyvsp[(3) - (3)].i)<=0) cf_error("Stub cost must be greater than zero"); ;}
    break;

  case 387:
#line 1644 "cf-parse.y"
    {if((yyvsp[(2) - (2)].i)) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;};}
    break;

  case 394:
#line 1657 "cf-parse.y"
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[(1) - (1)].px);
     this_stubnet->cost = COST_D;
   ;}
    break;

  case 397:
#line 1671 "cf-parse.y"
    { this_stubnet->hidden = (yyvsp[(2) - (2)].i); ;}
    break;

  case 398:
#line 1672 "cf-parse.y"
    { this_stubnet->summary = (yyvsp[(2) - (2)].i); ;}
    break;

  case 399:
#line 1673 "cf-parse.y"
    { this_stubnet->cost = (yyvsp[(2) - (2)].i); ;}
    break;

  case 400:
#line 1677 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 405:
#line 1687 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 406:
#line 1688 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 407:
#line 1689 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 408:
#line 1690 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 409:
#line 1691 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 410:
#line 1692 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 411:
#line 1693 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 412:
#line 1694 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 413:
#line 1695 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 415:
#line 1700 "cf-parse.y"
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

  case 416:
#line 1721 "cf-parse.y"
    { OSPF_PATT->cost = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Cost must be in range 1-65535"); ;}
    break;

  case 417:
#line 1722 "cf-parse.y"
    { OSPF_PATT->helloint = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<=0) || ((yyvsp[(2) - (2)].i)>65535)) cf_error("Hello interval must be in range 1-65535"); ;}
    break;

  case 418:
#line 1723 "cf-parse.y"
    { OSPF_PATT->pollint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Poll int must be greater than zero"); ;}
    break;

  case 419:
#line 1724 "cf-parse.y"
    { OSPF_PATT->rxmtint = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=0) cf_error("Retransmit int must be greater than zero"); ;}
    break;

  case 420:
#line 1725 "cf-parse.y"
    { OSPF_PATT->inftransdelay = (yyvsp[(3) - (3)].i) ; if (((yyvsp[(3) - (3)].i)<=0) || ((yyvsp[(3) - (3)].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); ;}
    break;

  case 421:
#line 1726 "cf-parse.y"
    { OSPF_PATT->priority = (yyvsp[(2) - (2)].i) ; if (((yyvsp[(2) - (2)].i)<0) || ((yyvsp[(2) - (2)].i)>255)) cf_error("Priority must be in range 0-255"); ;}
    break;

  case 422:
#line 1727 "cf-parse.y"
    { OSPF_PATT->waitint = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 423:
#line 1728 "cf-parse.y"
    { OSPF_PATT->dead = (yyvsp[(2) - (2)].i) ; if ((yyvsp[(2) - (2)].i)<=1) cf_error("Dead interval must be greater than one"); ;}
    break;

  case 424:
#line 1729 "cf-parse.y"
    { OSPF_PATT->deadc = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i)<=1) cf_error("Dead count must be greater than one"); ;}
    break;

  case 425:
#line 1730 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_BCAST ; ;}
    break;

  case 426:
#line 1731 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_NBMA ; ;}
    break;

  case 427:
#line 1732 "cf-parse.y"
    { OSPF_PATT->type = OSPF_IT_PTP ; ;}
    break;

  case 428:
#line 1733 "cf-parse.y"
    { OSPF_PATT->strictnbma = (yyvsp[(3) - (3)].i) ; ;}
    break;

  case 429:
#line 1734 "cf-parse.y"
    { OSPF_PATT->stub = (yyvsp[(2) - (2)].i) ; ;}
    break;

  case 431:
#line 1736 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_NONE ; ;}
    break;

  case 432:
#line 1737 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; ;}
    break;

  case 433:
#line 1738 "cf-parse.y"
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; ;}
    break;

  case 434:
#line 1739 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; ;}
    break;

  case 435:
#line 1740 "cf-parse.y"
    { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; ;}
    break;

  case 436:
#line 1741 "cf-parse.y"
    { OSPF_PATT->rxbuf = (yyvsp[(3) - (3)].i) ; if ((yyvsp[(3) - (3)].i) < OSPF_RXBUF_MINSIZE) cf_error("Buffer size is too small") ; ;}
    break;

  case 442:
#line 1755 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (2)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (2)].px).len;
 ;}
    break;

  case 443:
#line 1764 "cf-parse.y"
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = (yyvsp[(1) - (3)].px).addr;
   this_pref->px.len = (yyvsp[(1) - (3)].px).len;
   this_pref->hidden = 1;
 ;}
    break;

  case 448:
#line 1783 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (2)].a);
   this_nbma->eligible=0;
 ;}
    break;

  case 449:
#line 1792 "cf-parse.y"
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[(1) - (3)].a);
   this_nbma->eligible=1;
 ;}
    break;

  case 450:
#line 1802 "cf-parse.y"
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

  case 455:
#line 1835 "cf-parse.y"
    { finish_iface_config(OSPF_PATT); ;}
    break;

  case 457:
#line 1840 "cf-parse.y"
    { (yyval.t) = NULL; ;}
    break;

  case 458:
#line 1845 "cf-parse.y"
    { ospf_sh(proto_get_named((yyvsp[(3) - (4)].s), &proto_ospf)); ;}
    break;

  case 459:
#line 1848 "cf-parse.y"
    { ospf_sh_neigh(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 460:
#line 1851 "cf-parse.y"
    { ospf_sh_iface(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), (yyvsp[(5) - (6)].t)); ;}
    break;

  case 461:
#line 1854 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 0); ;}
    break;

  case 462:
#line 1857 "cf-parse.y"
    { ospf_sh_state(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf), 1); ;}
    break;

  case 463:
#line 1860 "cf-parse.y"
    { ospf_sh_lsadb(proto_get_named((yyvsp[(4) - (6)].s), &proto_ospf)); ;}
    break;

  case 464:
#line 1865 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  ;}
    break;

  case 467:
#line 1875 "cf-parse.y"
    {
     if ((yyvsp[(4) - (5)].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[(4) - (5)].s)->def;
   ;}
    break;

  case 468:
#line 1880 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_OPAQUE; ;}
    break;

  case 469:
#line 1881 "cf-parse.y"
    { PIPE_CFG->mode = PIPE_TRANSPARENT; ;}
    break;

  case 470:
#line 1887 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
     rip_init_config(RIP_CFG);
   ;}
    break;

  case 473:
#line 1896 "cf-parse.y"
    { RIP_CFG->infinity = (yyvsp[(3) - (4)].i); ;}
    break;

  case 474:
#line 1897 "cf-parse.y"
    { RIP_CFG->port = (yyvsp[(3) - (4)].i); ;}
    break;

  case 475:
#line 1898 "cf-parse.y"
    { RIP_CFG->period = (yyvsp[(3) - (4)].i); ;}
    break;

  case 476:
#line 1899 "cf-parse.y"
    { RIP_CFG->garbage_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 477:
#line 1900 "cf-parse.y"
    { RIP_CFG->timeout_time = (yyvsp[(4) - (5)].i); ;}
    break;

  case 478:
#line 1901 "cf-parse.y"
    {RIP_CFG->authtype = (yyvsp[(3) - (4)].i); ;}
    break;

  case 480:
#line 1903 "cf-parse.y"
    { RIP_CFG->honor = HO_ALWAYS; ;}
    break;

  case 481:
#line 1904 "cf-parse.y"
    { RIP_CFG->honor = HO_NEIGHBOR; ;}
    break;

  case 482:
#line 1905 "cf-parse.y"
    { RIP_CFG->honor = HO_NEVER; ;}
    break;

  case 484:
#line 1910 "cf-parse.y"
    { (yyval.i)=AT_PLAINTEXT; ;}
    break;

  case 485:
#line 1911 "cf-parse.y"
    { (yyval.i)=AT_MD5; ;}
    break;

  case 486:
#line 1912 "cf-parse.y"
    { (yyval.i)=AT_NONE; ;}
    break;

  case 487:
#line 1917 "cf-parse.y"
    { (yyval.i)=IM_BROADCAST; ;}
    break;

  case 488:
#line 1918 "cf-parse.y"
    { (yyval.i)=0; ;}
    break;

  case 489:
#line 1919 "cf-parse.y"
    { (yyval.i)=IM_QUIET; ;}
    break;

  case 490:
#line 1920 "cf-parse.y"
    { (yyval.i)=IM_NOLISTEN; ;}
    break;

  case 491:
#line 1921 "cf-parse.y"
    { (yyval.i)=IM_VERSION1 | IM_BROADCAST; ;}
    break;

  case 493:
#line 1925 "cf-parse.y"
    { RIP_IPATT->metric = (yyvsp[(2) - (2)].i); ;}
    break;

  case 494:
#line 1926 "cf-parse.y"
    { RIP_IPATT->mode |= (yyvsp[(2) - (2)].i); ;}
    break;

  case 499:
#line 1940 "cf-parse.y"
    {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
   ;}
    break;

  case 501:
#line 1956 "cf-parse.y"
    {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  ;}
    break;

  case 505:
#line 1968 "cf-parse.y"
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&((struct static_config *) this_proto)->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[(2) - (2)].px).addr;
     this_srt->masklen = (yyvsp[(2) - (2)].px).len;
  ;}
    break;

  case 506:
#line 1977 "cf-parse.y"
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[(3) - (3)].a);
   ;}
    break;

  case 507:
#line 1981 "cf-parse.y"
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[(3) - (3)].t);
      rem_node(&this_srt->n);
      add_tail(&((struct static_config *) this_proto)->iface_routes, &this_srt->n);
   ;}
    break;

  case 508:
#line 1987 "cf-parse.y"
    { this_srt->dest = RTD_BLACKHOLE; ;}
    break;

  case 509:
#line 1988 "cf-parse.y"
    { this_srt->dest = RTD_UNREACHABLE; ;}
    break;

  case 510:
#line 1989 "cf-parse.y"
    { this_srt->dest = RTD_PROHIBIT; ;}
    break;

  case 511:
#line 1993 "cf-parse.y"
    { static_show(proto_get_named((yyvsp[(3) - (4)].s), &proto_static)); ;}
    break;

  case 561:
#line 2001 "cf-parse.y"
    { bgp_check(BGP_CFG); ;}
    break;

  case 564:
#line 2001 "cf-parse.y"
    { RIP_CFG->passwords = get_passwords(); ;}
    break;

  case 573:
#line 2004 "cf-parse.y"
    { (yyval.x) = NULL; ;}
    break;

  case 574:
#line 2005 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); ;}
    break;

  case 575:
#line 2006 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); ;}
    break;

  case 576:
#line 2007 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); ;}
    break;

  case 577:
#line 2008 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); ;}
    break;

  case 578:
#line 2009 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); ;}
    break;

  case 579:
#line 2010 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); ;}
    break;

  case 580:
#line 2011 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); ;}
    break;

  case 581:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); ;}
    break;

  case 582:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); ;}
    break;

  case 583:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); ;}
    break;

  case 584:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); ;}
    break;

  case 585:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_ROUTER_ID); ;}
    break;

  case 586:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); ;}
    break;

  case 587:
#line 2012 "cf-parse.y"
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 6009 "cf-parse.tab.c"
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


#line 2014 "cf-parse.y"

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



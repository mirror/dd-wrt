
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 360 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



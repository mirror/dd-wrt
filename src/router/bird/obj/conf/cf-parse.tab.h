/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
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

/* Line 2068 of yacc.c  */
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



/* Line 2068 of yacc.c  */
#line 396 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



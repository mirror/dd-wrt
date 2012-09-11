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

/* Line 2068 of yacc.c  */
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



/* Line 2068 of yacc.c  */
#line 387 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



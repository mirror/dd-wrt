
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 361 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



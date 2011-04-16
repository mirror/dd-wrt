
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
#line 357 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



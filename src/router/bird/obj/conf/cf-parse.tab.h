
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 311 "cf-parse.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cf_lval;



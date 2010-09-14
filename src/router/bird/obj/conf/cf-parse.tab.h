/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1489 of yacc.c.  */
#line 590 "cf-parse.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE cf_lval;


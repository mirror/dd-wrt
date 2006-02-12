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
/* Line 1318 of yacc.c.  */
#line 482 "cf-parse.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE cf_lval;




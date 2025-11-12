/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_NFT_SRC_PARSER_BISON_H_INCLUDED
# define YY_NFT_SRC_PARSER_BISON_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int nft_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    TOKEN_EOF = 0,                 /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    JUNK = 258,                    /* "junk"  */
    CRLF = 259,                    /* "CRLF line terminators"  */
    NEWLINE = 260,                 /* "newline"  */
    COLON = 261,                   /* "colon"  */
    SEMICOLON = 262,               /* "semicolon"  */
    COMMA = 263,                   /* "comma"  */
    DOT = 264,                     /* "."  */
    EQ = 265,                      /* "=="  */
    NEQ = 266,                     /* "!="  */
    LT = 267,                      /* "<"  */
    GT = 268,                      /* ">"  */
    GTE = 269,                     /* ">="  */
    LTE = 270,                     /* "<="  */
    LSHIFT = 271,                  /* "<<"  */
    RSHIFT = 272,                  /* ">>"  */
    AMPERSAND = 273,               /* "&"  */
    CARET = 274,                   /* "^"  */
    NOT = 275,                     /* "!"  */
    SLASH = 276,                   /* "/"  */
    ASTERISK = 277,                /* "*"  */
    DASH = 278,                    /* "-"  */
    AT = 279,                      /* "@"  */
    VMAP = 280,                    /* "vmap"  */
    PLUS = 281,                    /* "+"  */
    INCLUDE = 282,                 /* "include"  */
    DEFINE = 283,                  /* "define"  */
    REDEFINE = 284,                /* "redefine"  */
    UNDEFINE = 285,                /* "undefine"  */
    FIB = 286,                     /* "fib"  */
    CHECK = 287,                   /* "check"  */
    SOCKET = 288,                  /* "socket"  */
    TRANSPARENT = 289,             /* "transparent"  */
    WILDCARD = 290,                /* "wildcard"  */
    CGROUPV2 = 291,                /* "cgroupv2"  */
    TPROXY = 292,                  /* "tproxy"  */
    OSF = 293,                     /* "osf"  */
    SYNPROXY = 294,                /* "synproxy"  */
    MSS = 295,                     /* "mss"  */
    WSCALE = 296,                  /* "wscale"  */
    TYPEOF = 297,                  /* "typeof"  */
    HOOK = 298,                    /* "hook"  */
    HOOKS = 299,                   /* "hooks"  */
    DEVICE = 300,                  /* "device"  */
    DEVICES = 301,                 /* "devices"  */
    TABLE = 302,                   /* "table"  */
    TABLES = 303,                  /* "tables"  */
    CHAIN = 304,                   /* "chain"  */
    CHAINS = 305,                  /* "chains"  */
    RULE = 306,                    /* "rule"  */
    RULES = 307,                   /* "rules"  */
    SETS = 308,                    /* "sets"  */
    SET = 309,                     /* "set"  */
    ELEMENT = 310,                 /* "element"  */
    MAP = 311,                     /* "map"  */
    MAPS = 312,                    /* "maps"  */
    FLOWTABLE = 313,               /* "flowtable"  */
    HANDLE = 314,                  /* "handle"  */
    RULESET = 315,                 /* "ruleset"  */
    TRACE = 316,                   /* "trace"  */
    INET = 317,                    /* "inet"  */
    NETDEV = 318,                  /* "netdev"  */
    ADD = 319,                     /* "add"  */
    UPDATE = 320,                  /* "update"  */
    REPLACE = 321,                 /* "replace"  */
    CREATE = 322,                  /* "create"  */
    INSERT = 323,                  /* "insert"  */
    DELETE = 324,                  /* "delete"  */
    GET = 325,                     /* "get"  */
    LIST = 326,                    /* "list"  */
    RESET = 327,                   /* "reset"  */
    FLUSH = 328,                   /* "flush"  */
    RENAME = 329,                  /* "rename"  */
    DESCRIBE = 330,                /* "describe"  */
    IMPORT = 331,                  /* "import"  */
    EXPORT = 332,                  /* "export"  */
    DESTROY = 333,                 /* "destroy"  */
    MONITOR = 334,                 /* "monitor"  */
    ALL = 335,                     /* "all"  */
    ACCEPT = 336,                  /* "accept"  */
    DROP = 337,                    /* "drop"  */
    CONTINUE = 338,                /* "continue"  */
    JUMP = 339,                    /* "jump"  */
    GOTO = 340,                    /* "goto"  */
    RETURN = 341,                  /* "return"  */
    TO = 342,                      /* "to"  */
    CONSTANT = 343,                /* "constant"  */
    INTERVAL = 344,                /* "interval"  */
    DYNAMIC = 345,                 /* "dynamic"  */
    AUTOMERGE = 346,               /* "auto-merge"  */
    TIMEOUT = 347,                 /* "timeout"  */
    GC_INTERVAL = 348,             /* "gc-interval"  */
    ELEMENTS = 349,                /* "elements"  */
    EXPIRES = 350,                 /* "expires"  */
    POLICY = 351,                  /* "policy"  */
    MEMORY = 352,                  /* "memory"  */
    PERFORMANCE = 353,             /* "performance"  */
    SIZE = 354,                    /* "size"  */
    FLOW = 355,                    /* "flow"  */
    OFFLOAD = 356,                 /* "offload"  */
    METER = 357,                   /* "meter"  */
    METERS = 358,                  /* "meters"  */
    FLOWTABLES = 359,              /* "flowtables"  */
    NUM = 360,                     /* "number"  */
    STRING = 361,                  /* "string"  */
    QUOTED_STRING = 362,           /* "quoted string"  */
    ASTERISK_STRING = 363,         /* "string with a trailing asterisk"  */
    LL_HDR = 364,                  /* "ll"  */
    NETWORK_HDR = 365,             /* "nh"  */
    TRANSPORT_HDR = 366,           /* "th"  */
    BRIDGE = 367,                  /* "bridge"  */
    ETHER = 368,                   /* "ether"  */
    SADDR = 369,                   /* "saddr"  */
    DADDR = 370,                   /* "daddr"  */
    TYPE = 371,                    /* "type"  */
    VLAN = 372,                    /* "vlan"  */
    ID = 373,                      /* "id"  */
    CFI = 374,                     /* "cfi"  */
    DEI = 375,                     /* "dei"  */
    PCP = 376,                     /* "pcp"  */
    ARP = 377,                     /* "arp"  */
    HTYPE = 378,                   /* "htype"  */
    PTYPE = 379,                   /* "ptype"  */
    HLEN = 380,                    /* "hlen"  */
    PLEN = 381,                    /* "plen"  */
    OPERATION = 382,               /* "operation"  */
    IP = 383,                      /* "ip"  */
    HDRVERSION = 384,              /* "version"  */
    HDRLENGTH = 385,               /* "hdrlength"  */
    DSCP = 386,                    /* "dscp"  */
    ECN = 387,                     /* "ecn"  */
    LENGTH = 388,                  /* "length"  */
    FRAG_OFF = 389,                /* "frag-off"  */
    TTL = 390,                     /* "ttl"  */
    PROTOCOL = 391,                /* "protocol"  */
    CHECKSUM = 392,                /* "checksum"  */
    PTR = 393,                     /* "ptr"  */
    VALUE = 394,                   /* "value"  */
    LSRR = 395,                    /* "lsrr"  */
    RR = 396,                      /* "rr"  */
    SSRR = 397,                    /* "ssrr"  */
    RA = 398,                      /* "ra"  */
    ICMP = 399,                    /* "icmp"  */
    CODE = 400,                    /* "code"  */
    SEQUENCE = 401,                /* "seq"  */
    GATEWAY = 402,                 /* "gateway"  */
    MTU = 403,                     /* "mtu"  */
    IGMP = 404,                    /* "igmp"  */
    MRT = 405,                     /* "mrt"  */
    OPTIONS = 406,                 /* "options"  */
    IP6 = 407,                     /* "ip6"  */
    PRIORITY = 408,                /* "priority"  */
    FLOWLABEL = 409,               /* "flowlabel"  */
    NEXTHDR = 410,                 /* "nexthdr"  */
    HOPLIMIT = 411,                /* "hoplimit"  */
    ICMP6 = 412,                   /* "icmpv6"  */
    PPTR = 413,                    /* "param-problem"  */
    MAXDELAY = 414,                /* "max-delay"  */
    TADDR = 415,                   /* "taddr"  */
    AH = 416,                      /* "ah"  */
    RESERVED = 417,                /* "reserved"  */
    SPI = 418,                     /* "spi"  */
    ESP = 419,                     /* "esp"  */
    COMP = 420,                    /* "comp"  */
    FLAGS = 421,                   /* "flags"  */
    CPI = 422,                     /* "cpi"  */
    PORT = 423,                    /* "port"  */
    UDP = 424,                     /* "udp"  */
    SPORT = 425,                   /* "sport"  */
    DPORT = 426,                   /* "dport"  */
    UDPLITE = 427,                 /* "udplite"  */
    CSUMCOV = 428,                 /* "csumcov"  */
    TCP = 429,                     /* "tcp"  */
    ACKSEQ = 430,                  /* "ackseq"  */
    DOFF = 431,                    /* "doff"  */
    WINDOW = 432,                  /* "window"  */
    URGPTR = 433,                  /* "urgptr"  */
    OPTION = 434,                  /* "option"  */
    ECHO = 435,                    /* "echo"  */
    EOL = 436,                     /* "eol"  */
    MPTCP = 437,                   /* "mptcp"  */
    NOP = 438,                     /* "nop"  */
    SACK = 439,                    /* "sack"  */
    SACK0 = 440,                   /* "sack0"  */
    SACK1 = 441,                   /* "sack1"  */
    SACK2 = 442,                   /* "sack2"  */
    SACK3 = 443,                   /* "sack3"  */
    SACK_PERM = 444,               /* "sack-permitted"  */
    FASTOPEN = 445,                /* "fastopen"  */
    MD5SIG = 446,                  /* "md5sig"  */
    TIMESTAMP = 447,               /* "timestamp"  */
    COUNT = 448,                   /* "count"  */
    LEFT = 449,                    /* "left"  */
    RIGHT = 450,                   /* "right"  */
    TSVAL = 451,                   /* "tsval"  */
    TSECR = 452,                   /* "tsecr"  */
    SUBTYPE = 453,                 /* "subtype"  */
    DCCP = 454,                    /* "dccp"  */
    VXLAN = 455,                   /* "vxlan"  */
    VNI = 456,                     /* "vni"  */
    GRE = 457,                     /* "gre"  */
    GRETAP = 458,                  /* "gretap"  */
    GENEVE = 459,                  /* "geneve"  */
    SCTP = 460,                    /* "sctp"  */
    CHUNK = 461,                   /* "chunk"  */
    DATA = 462,                    /* "data"  */
    INIT = 463,                    /* "init"  */
    INIT_ACK = 464,                /* "init-ack"  */
    HEARTBEAT = 465,               /* "heartbeat"  */
    HEARTBEAT_ACK = 466,           /* "heartbeat-ack"  */
    ABORT = 467,                   /* "abort"  */
    SHUTDOWN = 468,                /* "shutdown"  */
    SHUTDOWN_ACK = 469,            /* "shutdown-ack"  */
    ERROR = 470,                   /* "error"  */
    COOKIE_ECHO = 471,             /* "cookie-echo"  */
    COOKIE_ACK = 472,              /* "cookie-ack"  */
    ECNE = 473,                    /* "ecne"  */
    CWR = 474,                     /* "cwr"  */
    SHUTDOWN_COMPLETE = 475,       /* "shutdown-complete"  */
    ASCONF_ACK = 476,              /* "asconf-ack"  */
    FORWARD_TSN = 477,             /* "forward-tsn"  */
    ASCONF = 478,                  /* "asconf"  */
    TSN = 479,                     /* "tsn"  */
    STREAM = 480,                  /* "stream"  */
    SSN = 481,                     /* "ssn"  */
    PPID = 482,                    /* "ppid"  */
    INIT_TAG = 483,                /* "init-tag"  */
    A_RWND = 484,                  /* "a-rwnd"  */
    NUM_OSTREAMS = 485,            /* "num-outbound-streams"  */
    NUM_ISTREAMS = 486,            /* "num-inbound-streams"  */
    INIT_TSN = 487,                /* "initial-tsn"  */
    CUM_TSN_ACK = 488,             /* "cum-tsn-ack"  */
    NUM_GACK_BLOCKS = 489,         /* "num-gap-ack-blocks"  */
    NUM_DUP_TSNS = 490,            /* "num-dup-tsns"  */
    LOWEST_TSN = 491,              /* "lowest-tsn"  */
    SEQNO = 492,                   /* "seqno"  */
    NEW_CUM_TSN = 493,             /* "new-cum-tsn"  */
    VTAG = 494,                    /* "vtag"  */
    RT = 495,                      /* "rt"  */
    RT0 = 496,                     /* "rt0"  */
    RT2 = 497,                     /* "rt2"  */
    RT4 = 498,                     /* "srh"  */
    SEG_LEFT = 499,                /* "seg-left"  */
    ADDR = 500,                    /* "addr"  */
    LAST_ENT = 501,                /* "last-entry"  */
    TAG = 502,                     /* "tag"  */
    SID = 503,                     /* "sid"  */
    HBH = 504,                     /* "hbh"  */
    FRAG = 505,                    /* "frag"  */
    RESERVED2 = 506,               /* "reserved2"  */
    MORE_FRAGMENTS = 507,          /* "more-fragments"  */
    DST = 508,                     /* "dst"  */
    MH = 509,                      /* "mh"  */
    META = 510,                    /* "meta"  */
    MARK = 511,                    /* "mark"  */
    IIF = 512,                     /* "iif"  */
    IIFNAME = 513,                 /* "iifname"  */
    IIFTYPE = 514,                 /* "iiftype"  */
    OIF = 515,                     /* "oif"  */
    OIFNAME = 516,                 /* "oifname"  */
    OIFTYPE = 517,                 /* "oiftype"  */
    SKUID = 518,                   /* "skuid"  */
    SKGID = 519,                   /* "skgid"  */
    NFTRACE = 520,                 /* "nftrace"  */
    RTCLASSID = 521,               /* "rtclassid"  */
    IBRIPORT = 522,                /* "ibriport"  */
    OBRIPORT = 523,                /* "obriport"  */
    IBRIDGENAME = 524,             /* "ibrname"  */
    OBRIDGENAME = 525,             /* "obrname"  */
    PKTTYPE = 526,                 /* "pkttype"  */
    CPU = 527,                     /* "cpu"  */
    IIFGROUP = 528,                /* "iifgroup"  */
    OIFGROUP = 529,                /* "oifgroup"  */
    CGROUP = 530,                  /* "cgroup"  */
    TIME = 531,                    /* "time"  */
    CLASSID = 532,                 /* "classid"  */
    NEXTHOP = 533,                 /* "nexthop"  */
    CT = 534,                      /* "ct"  */
    L3PROTOCOL = 535,              /* "l3proto"  */
    PROTO_SRC = 536,               /* "proto-src"  */
    PROTO_DST = 537,               /* "proto-dst"  */
    ZONE = 538,                    /* "zone"  */
    DIRECTION = 539,               /* "direction"  */
    EVENT = 540,                   /* "event"  */
    EXPECTATION = 541,             /* "expectation"  */
    EXPIRATION = 542,              /* "expiration"  */
    HELPER = 543,                  /* "helper"  */
    LABEL = 544,                   /* "label"  */
    STATE = 545,                   /* "state"  */
    STATUS = 546,                  /* "status"  */
    ORIGINAL = 547,                /* "original"  */
    REPLY = 548,                   /* "reply"  */
    COUNTER = 549,                 /* "counter"  */
    NAME = 550,                    /* "name"  */
    PACKETS = 551,                 /* "packets"  */
    BYTES = 552,                   /* "bytes"  */
    AVGPKT = 553,                  /* "avgpkt"  */
    LAST = 554,                    /* "last"  */
    NEVER = 555,                   /* "never"  */
    COUNTERS = 556,                /* "counters"  */
    QUOTAS = 557,                  /* "quotas"  */
    LIMITS = 558,                  /* "limits"  */
    SYNPROXYS = 559,               /* "synproxys"  */
    HELPERS = 560,                 /* "helpers"  */
    LOG = 561,                     /* "log"  */
    PREFIX = 562,                  /* "prefix"  */
    GROUP = 563,                   /* "group"  */
    SNAPLEN = 564,                 /* "snaplen"  */
    QUEUE_THRESHOLD = 565,         /* "queue-threshold"  */
    LEVEL = 566,                   /* "level"  */
    LIMIT = 567,                   /* "limit"  */
    RATE = 568,                    /* "rate"  */
    BURST = 569,                   /* "burst"  */
    OVER = 570,                    /* "over"  */
    UNTIL = 571,                   /* "until"  */
    QUOTA = 572,                   /* "quota"  */
    USED = 573,                    /* "used"  */
    SECMARK = 574,                 /* "secmark"  */
    SECMARKS = 575,                /* "secmarks"  */
    SECOND = 576,                  /* "second"  */
    MINUTE = 577,                  /* "minute"  */
    HOUR = 578,                    /* "hour"  */
    DAY = 579,                     /* "day"  */
    WEEK = 580,                    /* "week"  */
    _REJECT = 581,                 /* "reject"  */
    WITH = 582,                    /* "with"  */
    ICMPX = 583,                   /* "icmpx"  */
    SNAT = 584,                    /* "snat"  */
    DNAT = 585,                    /* "dnat"  */
    MASQUERADE = 586,              /* "masquerade"  */
    REDIRECT = 587,                /* "redirect"  */
    RANDOM = 588,                  /* "random"  */
    FULLY_RANDOM = 589,            /* "fully-random"  */
    PERSISTENT = 590,              /* "persistent"  */
    QUEUE = 591,                   /* "queue"  */
    QUEUENUM = 592,                /* "num"  */
    BYPASS = 593,                  /* "bypass"  */
    FANOUT = 594,                  /* "fanout"  */
    DUP = 595,                     /* "dup"  */
    FWD = 596,                     /* "fwd"  */
    NUMGEN = 597,                  /* "numgen"  */
    INC = 598,                     /* "inc"  */
    MOD = 599,                     /* "mod"  */
    OFFSET = 600,                  /* "offset"  */
    JHASH = 601,                   /* "jhash"  */
    SYMHASH = 602,                 /* "symhash"  */
    SEED = 603,                    /* "seed"  */
    POSITION = 604,                /* "position"  */
    INDEX = 605,                   /* "index"  */
    COMMENT = 606,                 /* "comment"  */
    XML = 607,                     /* "xml"  */
    JSON = 608,                    /* "json"  */
    VM = 609,                      /* "vm"  */
    NOTRACK = 610,                 /* "notrack"  */
    EXISTS = 611,                  /* "exists"  */
    MISSING = 612,                 /* "missing"  */
    EXTHDR = 613,                  /* "exthdr"  */
    IPSEC = 614,                   /* "ipsec"  */
    REQID = 615,                   /* "reqid"  */
    SPNUM = 616,                   /* "spnum"  */
    IN = 617,                      /* "in"  */
    OUT = 618,                     /* "out"  */
    XT = 619                       /* "xt"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define TOKEN_EOF 0
#define YYerror 256
#define YYUNDEF 257
#define JUNK 258
#define CRLF 259
#define NEWLINE 260
#define COLON 261
#define SEMICOLON 262
#define COMMA 263
#define DOT 264
#define EQ 265
#define NEQ 266
#define LT 267
#define GT 268
#define GTE 269
#define LTE 270
#define LSHIFT 271
#define RSHIFT 272
#define AMPERSAND 273
#define CARET 274
#define NOT 275
#define SLASH 276
#define ASTERISK 277
#define DASH 278
#define AT 279
#define VMAP 280
#define PLUS 281
#define INCLUDE 282
#define DEFINE 283
#define REDEFINE 284
#define UNDEFINE 285
#define FIB 286
#define CHECK 287
#define SOCKET 288
#define TRANSPARENT 289
#define WILDCARD 290
#define CGROUPV2 291
#define TPROXY 292
#define OSF 293
#define SYNPROXY 294
#define MSS 295
#define WSCALE 296
#define TYPEOF 297
#define HOOK 298
#define HOOKS 299
#define DEVICE 300
#define DEVICES 301
#define TABLE 302
#define TABLES 303
#define CHAIN 304
#define CHAINS 305
#define RULE 306
#define RULES 307
#define SETS 308
#define SET 309
#define ELEMENT 310
#define MAP 311
#define MAPS 312
#define FLOWTABLE 313
#define HANDLE 314
#define RULESET 315
#define TRACE 316
#define INET 317
#define NETDEV 318
#define ADD 319
#define UPDATE 320
#define REPLACE 321
#define CREATE 322
#define INSERT 323
#define DELETE 324
#define GET 325
#define LIST 326
#define RESET 327
#define FLUSH 328
#define RENAME 329
#define DESCRIBE 330
#define IMPORT 331
#define EXPORT 332
#define DESTROY 333
#define MONITOR 334
#define ALL 335
#define ACCEPT 336
#define DROP 337
#define CONTINUE 338
#define JUMP 339
#define GOTO 340
#define RETURN 341
#define TO 342
#define CONSTANT 343
#define INTERVAL 344
#define DYNAMIC 345
#define AUTOMERGE 346
#define TIMEOUT 347
#define GC_INTERVAL 348
#define ELEMENTS 349
#define EXPIRES 350
#define POLICY 351
#define MEMORY 352
#define PERFORMANCE 353
#define SIZE 354
#define FLOW 355
#define OFFLOAD 356
#define METER 357
#define METERS 358
#define FLOWTABLES 359
#define NUM 360
#define STRING 361
#define QUOTED_STRING 362
#define ASTERISK_STRING 363
#define LL_HDR 364
#define NETWORK_HDR 365
#define TRANSPORT_HDR 366
#define BRIDGE 367
#define ETHER 368
#define SADDR 369
#define DADDR 370
#define TYPE 371
#define VLAN 372
#define ID 373
#define CFI 374
#define DEI 375
#define PCP 376
#define ARP 377
#define HTYPE 378
#define PTYPE 379
#define HLEN 380
#define PLEN 381
#define OPERATION 382
#define IP 383
#define HDRVERSION 384
#define HDRLENGTH 385
#define DSCP 386
#define ECN 387
#define LENGTH 388
#define FRAG_OFF 389
#define TTL 390
#define PROTOCOL 391
#define CHECKSUM 392
#define PTR 393
#define VALUE 394
#define LSRR 395
#define RR 396
#define SSRR 397
#define RA 398
#define ICMP 399
#define CODE 400
#define SEQUENCE 401
#define GATEWAY 402
#define MTU 403
#define IGMP 404
#define MRT 405
#define OPTIONS 406
#define IP6 407
#define PRIORITY 408
#define FLOWLABEL 409
#define NEXTHDR 410
#define HOPLIMIT 411
#define ICMP6 412
#define PPTR 413
#define MAXDELAY 414
#define TADDR 415
#define AH 416
#define RESERVED 417
#define SPI 418
#define ESP 419
#define COMP 420
#define FLAGS 421
#define CPI 422
#define PORT 423
#define UDP 424
#define SPORT 425
#define DPORT 426
#define UDPLITE 427
#define CSUMCOV 428
#define TCP 429
#define ACKSEQ 430
#define DOFF 431
#define WINDOW 432
#define URGPTR 433
#define OPTION 434
#define ECHO 435
#define EOL 436
#define MPTCP 437
#define NOP 438
#define SACK 439
#define SACK0 440
#define SACK1 441
#define SACK2 442
#define SACK3 443
#define SACK_PERM 444
#define FASTOPEN 445
#define MD5SIG 446
#define TIMESTAMP 447
#define COUNT 448
#define LEFT 449
#define RIGHT 450
#define TSVAL 451
#define TSECR 452
#define SUBTYPE 453
#define DCCP 454
#define VXLAN 455
#define VNI 456
#define GRE 457
#define GRETAP 458
#define GENEVE 459
#define SCTP 460
#define CHUNK 461
#define DATA 462
#define INIT 463
#define INIT_ACK 464
#define HEARTBEAT 465
#define HEARTBEAT_ACK 466
#define ABORT 467
#define SHUTDOWN 468
#define SHUTDOWN_ACK 469
#define ERROR 470
#define COOKIE_ECHO 471
#define COOKIE_ACK 472
#define ECNE 473
#define CWR 474
#define SHUTDOWN_COMPLETE 475
#define ASCONF_ACK 476
#define FORWARD_TSN 477
#define ASCONF 478
#define TSN 479
#define STREAM 480
#define SSN 481
#define PPID 482
#define INIT_TAG 483
#define A_RWND 484
#define NUM_OSTREAMS 485
#define NUM_ISTREAMS 486
#define INIT_TSN 487
#define CUM_TSN_ACK 488
#define NUM_GACK_BLOCKS 489
#define NUM_DUP_TSNS 490
#define LOWEST_TSN 491
#define SEQNO 492
#define NEW_CUM_TSN 493
#define VTAG 494
#define RT 495
#define RT0 496
#define RT2 497
#define RT4 498
#define SEG_LEFT 499
#define ADDR 500
#define LAST_ENT 501
#define TAG 502
#define SID 503
#define HBH 504
#define FRAG 505
#define RESERVED2 506
#define MORE_FRAGMENTS 507
#define DST 508
#define MH 509
#define META 510
#define MARK 511
#define IIF 512
#define IIFNAME 513
#define IIFTYPE 514
#define OIF 515
#define OIFNAME 516
#define OIFTYPE 517
#define SKUID 518
#define SKGID 519
#define NFTRACE 520
#define RTCLASSID 521
#define IBRIPORT 522
#define OBRIPORT 523
#define IBRIDGENAME 524
#define OBRIDGENAME 525
#define PKTTYPE 526
#define CPU 527
#define IIFGROUP 528
#define OIFGROUP 529
#define CGROUP 530
#define TIME 531
#define CLASSID 532
#define NEXTHOP 533
#define CT 534
#define L3PROTOCOL 535
#define PROTO_SRC 536
#define PROTO_DST 537
#define ZONE 538
#define DIRECTION 539
#define EVENT 540
#define EXPECTATION 541
#define EXPIRATION 542
#define HELPER 543
#define LABEL 544
#define STATE 545
#define STATUS 546
#define ORIGINAL 547
#define REPLY 548
#define COUNTER 549
#define NAME 550
#define PACKETS 551
#define BYTES 552
#define AVGPKT 553
#define LAST 554
#define NEVER 555
#define COUNTERS 556
#define QUOTAS 557
#define LIMITS 558
#define SYNPROXYS 559
#define HELPERS 560
#define LOG 561
#define PREFIX 562
#define GROUP 563
#define SNAPLEN 564
#define QUEUE_THRESHOLD 565
#define LEVEL 566
#define LIMIT 567
#define RATE 568
#define BURST 569
#define OVER 570
#define UNTIL 571
#define QUOTA 572
#define USED 573
#define SECMARK 574
#define SECMARKS 575
#define SECOND 576
#define MINUTE 577
#define HOUR 578
#define DAY 579
#define WEEK 580
#define _REJECT 581
#define WITH 582
#define ICMPX 583
#define SNAT 584
#define DNAT 585
#define MASQUERADE 586
#define REDIRECT 587
#define RANDOM 588
#define FULLY_RANDOM 589
#define PERSISTENT 590
#define QUEUE 591
#define QUEUENUM 592
#define BYPASS 593
#define FANOUT 594
#define DUP 595
#define FWD 596
#define NUMGEN 597
#define INC 598
#define MOD 599
#define OFFSET 600
#define JHASH 601
#define SYMHASH 602
#define SEED 603
#define POSITION 604
#define INDEX 605
#define COMMENT 606
#define XML 607
#define JSON 608
#define VM 609
#define NOTRACK 610
#define EXISTS 611
#define MISSING 612
#define EXTHDR 613
#define IPSEC 614
#define REQID 615
#define SPNUM 616
#define IN 617
#define OUT 618
#define XT 619

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 222 "src/parser_bison.y"

	uint64_t		val;
	uint32_t		val32;
	uint8_t			val8;
	const char *		string;

	struct list_head	*list;
	struct cmd		*cmd;
	struct handle		handle;
	struct table		*table;
	struct chain		*chain;
	struct rule		*rule;
	struct stmt		*stmt;
	struct expr		*expr;
	struct set		*set;
	struct obj		*obj;
	struct flowtable	*flowtable;
	struct ct		*ct;
	const struct datatype	*datatype;
	struct handle_spec	handle_spec;
	struct position_spec	position_spec;
	struct prio_spec	prio_spec;
	struct limit_rate	limit_rate;
	struct tcp_kind_field {
		uint16_t kind; /* must allow > 255 for SACK1, 2.. hack */
		uint8_t field;
	} tcp_kind_field;
	struct timeout_state	*timeout_state;

#line 825 "src/parser_bison.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int nft_parse (struct nft_ctx *nft, void *scanner, struct parser_state *state);


#endif /* !YY_NFT_SRC_PARSER_BISON_H_INCLUDED  */

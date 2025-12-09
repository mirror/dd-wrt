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
    PATH = 317,                    /* "path"  */
    INET = 318,                    /* "inet"  */
    NETDEV = 319,                  /* "netdev"  */
    ADD = 320,                     /* "add"  */
    UPDATE = 321,                  /* "update"  */
    REPLACE = 322,                 /* "replace"  */
    CREATE = 323,                  /* "create"  */
    INSERT = 324,                  /* "insert"  */
    DELETE = 325,                  /* "delete"  */
    GET = 326,                     /* "get"  */
    LIST = 327,                    /* "list"  */
    RESET = 328,                   /* "reset"  */
    FLUSH = 329,                   /* "flush"  */
    RENAME = 330,                  /* "rename"  */
    DESCRIBE = 331,                /* "describe"  */
    IMPORT = 332,                  /* "import"  */
    EXPORT = 333,                  /* "export"  */
    DESTROY = 334,                 /* "destroy"  */
    MONITOR = 335,                 /* "monitor"  */
    ALL = 336,                     /* "all"  */
    ACCEPT = 337,                  /* "accept"  */
    DROP = 338,                    /* "drop"  */
    CONTINUE = 339,                /* "continue"  */
    JUMP = 340,                    /* "jump"  */
    GOTO = 341,                    /* "goto"  */
    RETURN = 342,                  /* "return"  */
    TO = 343,                      /* "to"  */
    CONSTANT = 344,                /* "constant"  */
    INTERVAL = 345,                /* "interval"  */
    DYNAMIC = 346,                 /* "dynamic"  */
    AUTOMERGE = 347,               /* "auto-merge"  */
    TIMEOUT = 348,                 /* "timeout"  */
    GC_INTERVAL = 349,             /* "gc-interval"  */
    ELEMENTS = 350,                /* "elements"  */
    EXPIRES = 351,                 /* "expires"  */
    POLICY = 352,                  /* "policy"  */
    MEMORY = 353,                  /* "memory"  */
    PERFORMANCE = 354,             /* "performance"  */
    SIZE = 355,                    /* "size"  */
    FLOW = 356,                    /* "flow"  */
    OFFLOAD = 357,                 /* "offload"  */
    METER = 358,                   /* "meter"  */
    METERS = 359,                  /* "meters"  */
    FLOWTABLES = 360,              /* "flowtables"  */
    NUM = 361,                     /* "number"  */
    STRING = 362,                  /* "string"  */
    QUOTED_STRING = 363,           /* "quoted string"  */
    ASTERISK_STRING = 364,         /* "string with a trailing asterisk"  */
    LL_HDR = 365,                  /* "ll"  */
    NETWORK_HDR = 366,             /* "nh"  */
    TRANSPORT_HDR = 367,           /* "th"  */
    BRIDGE = 368,                  /* "bridge"  */
    ETHER = 369,                   /* "ether"  */
    SADDR = 370,                   /* "saddr"  */
    DADDR = 371,                   /* "daddr"  */
    TYPE = 372,                    /* "type"  */
    VLAN = 373,                    /* "vlan"  */
    ID = 374,                      /* "id"  */
    CFI = 375,                     /* "cfi"  */
    DEI = 376,                     /* "dei"  */
    PCP = 377,                     /* "pcp"  */
    ARP = 378,                     /* "arp"  */
    HTYPE = 379,                   /* "htype"  */
    PTYPE = 380,                   /* "ptype"  */
    HLEN = 381,                    /* "hlen"  */
    PLEN = 382,                    /* "plen"  */
    OPERATION = 383,               /* "operation"  */
    IP = 384,                      /* "ip"  */
    HDRVERSION = 385,              /* "version"  */
    HDRLENGTH = 386,               /* "hdrlength"  */
    DSCP = 387,                    /* "dscp"  */
    ECN = 388,                     /* "ecn"  */
    LENGTH = 389,                  /* "length"  */
    FRAG_OFF = 390,                /* "frag-off"  */
    TTL = 391,                     /* "ttl"  */
    TOS = 392,                     /* "tos"  */
    PROTOCOL = 393,                /* "protocol"  */
    CHECKSUM = 394,                /* "checksum"  */
    PTR = 395,                     /* "ptr"  */
    VALUE = 396,                   /* "value"  */
    LSRR = 397,                    /* "lsrr"  */
    RR = 398,                      /* "rr"  */
    SSRR = 399,                    /* "ssrr"  */
    RA = 400,                      /* "ra"  */
    ICMP = 401,                    /* "icmp"  */
    CODE = 402,                    /* "code"  */
    SEQUENCE = 403,                /* "seq"  */
    GATEWAY = 404,                 /* "gateway"  */
    MTU = 405,                     /* "mtu"  */
    IGMP = 406,                    /* "igmp"  */
    MRT = 407,                     /* "mrt"  */
    OPTIONS = 408,                 /* "options"  */
    IP6 = 409,                     /* "ip6"  */
    PRIORITY = 410,                /* "priority"  */
    FLOWLABEL = 411,               /* "flowlabel"  */
    NEXTHDR = 412,                 /* "nexthdr"  */
    HOPLIMIT = 413,                /* "hoplimit"  */
    ICMP6 = 414,                   /* "icmpv6"  */
    PPTR = 415,                    /* "param-problem"  */
    MAXDELAY = 416,                /* "max-delay"  */
    TADDR = 417,                   /* "taddr"  */
    AH = 418,                      /* "ah"  */
    RESERVED = 419,                /* "reserved"  */
    SPI = 420,                     /* "spi"  */
    ESP = 421,                     /* "esp"  */
    COMP = 422,                    /* "comp"  */
    FLAGS = 423,                   /* "flags"  */
    CPI = 424,                     /* "cpi"  */
    PORT = 425,                    /* "port"  */
    UDP = 426,                     /* "udp"  */
    SPORT = 427,                   /* "sport"  */
    DPORT = 428,                   /* "dport"  */
    UDPLITE = 429,                 /* "udplite"  */
    CSUMCOV = 430,                 /* "csumcov"  */
    TCP = 431,                     /* "tcp"  */
    ACKSEQ = 432,                  /* "ackseq"  */
    DOFF = 433,                    /* "doff"  */
    WINDOW = 434,                  /* "window"  */
    URGPTR = 435,                  /* "urgptr"  */
    OPTION = 436,                  /* "option"  */
    ECHO = 437,                    /* "echo"  */
    EOL = 438,                     /* "eol"  */
    MPTCP = 439,                   /* "mptcp"  */
    NOP = 440,                     /* "nop"  */
    SACK = 441,                    /* "sack"  */
    SACK0 = 442,                   /* "sack0"  */
    SACK1 = 443,                   /* "sack1"  */
    SACK2 = 444,                   /* "sack2"  */
    SACK3 = 445,                   /* "sack3"  */
    SACK_PERM = 446,               /* "sack-permitted"  */
    FASTOPEN = 447,                /* "fastopen"  */
    MD5SIG = 448,                  /* "md5sig"  */
    TIMESTAMP = 449,               /* "timestamp"  */
    COUNT = 450,                   /* "count"  */
    LEFT = 451,                    /* "left"  */
    RIGHT = 452,                   /* "right"  */
    TSVAL = 453,                   /* "tsval"  */
    TSECR = 454,                   /* "tsecr"  */
    SUBTYPE = 455,                 /* "subtype"  */
    DCCP = 456,                    /* "dccp"  */
    VXLAN = 457,                   /* "vxlan"  */
    VNI = 458,                     /* "vni"  */
    GRE = 459,                     /* "gre"  */
    GRETAP = 460,                  /* "gretap"  */
    GENEVE = 461,                  /* "geneve"  */
    SCTP = 462,                    /* "sctp"  */
    CHUNK = 463,                   /* "chunk"  */
    DATA = 464,                    /* "data"  */
    INIT = 465,                    /* "init"  */
    INIT_ACK = 466,                /* "init-ack"  */
    HEARTBEAT = 467,               /* "heartbeat"  */
    HEARTBEAT_ACK = 468,           /* "heartbeat-ack"  */
    ABORT = 469,                   /* "abort"  */
    SHUTDOWN = 470,                /* "shutdown"  */
    SHUTDOWN_ACK = 471,            /* "shutdown-ack"  */
    ERROR = 472,                   /* "error"  */
    COOKIE_ECHO = 473,             /* "cookie-echo"  */
    COOKIE_ACK = 474,              /* "cookie-ack"  */
    ECNE = 475,                    /* "ecne"  */
    CWR = 476,                     /* "cwr"  */
    SHUTDOWN_COMPLETE = 477,       /* "shutdown-complete"  */
    ASCONF_ACK = 478,              /* "asconf-ack"  */
    FORWARD_TSN = 479,             /* "forward-tsn"  */
    ASCONF = 480,                  /* "asconf"  */
    TSN = 481,                     /* "tsn"  */
    STREAM = 482,                  /* "stream"  */
    SSN = 483,                     /* "ssn"  */
    PPID = 484,                    /* "ppid"  */
    INIT_TAG = 485,                /* "init-tag"  */
    A_RWND = 486,                  /* "a-rwnd"  */
    NUM_OSTREAMS = 487,            /* "num-outbound-streams"  */
    NUM_ISTREAMS = 488,            /* "num-inbound-streams"  */
    INIT_TSN = 489,                /* "initial-tsn"  */
    CUM_TSN_ACK = 490,             /* "cum-tsn-ack"  */
    NUM_GACK_BLOCKS = 491,         /* "num-gap-ack-blocks"  */
    NUM_DUP_TSNS = 492,            /* "num-dup-tsns"  */
    LOWEST_TSN = 493,              /* "lowest-tsn"  */
    SEQNO = 494,                   /* "seqno"  */
    NEW_CUM_TSN = 495,             /* "new-cum-tsn"  */
    VTAG = 496,                    /* "vtag"  */
    RT = 497,                      /* "rt"  */
    RT0 = 498,                     /* "rt0"  */
    RT2 = 499,                     /* "rt2"  */
    RT4 = 500,                     /* "srh"  */
    SEG_LEFT = 501,                /* "seg-left"  */
    ADDR = 502,                    /* "addr"  */
    LAST_ENT = 503,                /* "last-entry"  */
    TAG = 504,                     /* "tag"  */
    SID = 505,                     /* "sid"  */
    HBH = 506,                     /* "hbh"  */
    FRAG = 507,                    /* "frag"  */
    RESERVED2 = 508,               /* "reserved2"  */
    MORE_FRAGMENTS = 509,          /* "more-fragments"  */
    DST = 510,                     /* "dst"  */
    MH = 511,                      /* "mh"  */
    META = 512,                    /* "meta"  */
    MARK = 513,                    /* "mark"  */
    IIF = 514,                     /* "iif"  */
    IIFNAME = 515,                 /* "iifname"  */
    IIFTYPE = 516,                 /* "iiftype"  */
    OIF = 517,                     /* "oif"  */
    OIFNAME = 518,                 /* "oifname"  */
    OIFTYPE = 519,                 /* "oiftype"  */
    SKUID = 520,                   /* "skuid"  */
    SKGID = 521,                   /* "skgid"  */
    NFTRACE = 522,                 /* "nftrace"  */
    RTCLASSID = 523,               /* "rtclassid"  */
    IBRIPORT = 524,                /* "ibriport"  */
    OBRIPORT = 525,                /* "obriport"  */
    IBRIDGENAME = 526,             /* "ibrname"  */
    OBRIDGENAME = 527,             /* "obrname"  */
    PKTTYPE = 528,                 /* "pkttype"  */
    CPU = 529,                     /* "cpu"  */
    IIFGROUP = 530,                /* "iifgroup"  */
    OIFGROUP = 531,                /* "oifgroup"  */
    CGROUP = 532,                  /* "cgroup"  */
    TIME = 533,                    /* "time"  */
    CLASSID = 534,                 /* "classid"  */
    NEXTHOP = 535,                 /* "nexthop"  */
    CT = 536,                      /* "ct"  */
    L3PROTOCOL = 537,              /* "l3proto"  */
    PROTO_SRC = 538,               /* "proto-src"  */
    PROTO_DST = 539,               /* "proto-dst"  */
    ZONE = 540,                    /* "zone"  */
    DIRECTION = 541,               /* "direction"  */
    EVENT = 542,                   /* "event"  */
    EXPECTATION = 543,             /* "expectation"  */
    EXPIRATION = 544,              /* "expiration"  */
    HELPER = 545,                  /* "helper"  */
    LABEL = 546,                   /* "label"  */
    STATE = 547,                   /* "state"  */
    STATUS = 548,                  /* "status"  */
    ORIGINAL = 549,                /* "original"  */
    REPLY = 550,                   /* "reply"  */
    COUNTER = 551,                 /* "counter"  */
    NAME = 552,                    /* "name"  */
    PACKETS = 553,                 /* "packets"  */
    BYTES = 554,                   /* "bytes"  */
    AVGPKT = 555,                  /* "avgpkt"  */
    LAST = 556,                    /* "last"  */
    NEVER = 557,                   /* "never"  */
    TUNNEL = 558,                  /* "tunnel"  */
    ERSPAN = 559,                  /* "erspan"  */
    EGRESS = 560,                  /* "egress"  */
    INGRESS = 561,                 /* "ingress"  */
    GBP = 562,                     /* "gbp"  */
    CLASS = 563,                   /* "class"  */
    OPTTYPE = 564,                 /* "opt-type"  */
    COUNTERS = 565,                /* "counters"  */
    QUOTAS = 566,                  /* "quotas"  */
    LIMITS = 567,                  /* "limits"  */
    TUNNELS = 568,                 /* "tunnels"  */
    SYNPROXYS = 569,               /* "synproxys"  */
    HELPERS = 570,                 /* "helpers"  */
    LOG = 571,                     /* "log"  */
    PREFIX = 572,                  /* "prefix"  */
    GROUP = 573,                   /* "group"  */
    SNAPLEN = 574,                 /* "snaplen"  */
    QUEUE_THRESHOLD = 575,         /* "queue-threshold"  */
    LEVEL = 576,                   /* "level"  */
    LIMIT = 577,                   /* "limit"  */
    RATE = 578,                    /* "rate"  */
    BURST = 579,                   /* "burst"  */
    OVER = 580,                    /* "over"  */
    UNTIL = 581,                   /* "until"  */
    QUOTA = 582,                   /* "quota"  */
    USED = 583,                    /* "used"  */
    SECMARK = 584,                 /* "secmark"  */
    SECMARKS = 585,                /* "secmarks"  */
    SECOND = 586,                  /* "second"  */
    MINUTE = 587,                  /* "minute"  */
    HOUR = 588,                    /* "hour"  */
    DAY = 589,                     /* "day"  */
    WEEK = 590,                    /* "week"  */
    _REJECT = 591,                 /* "reject"  */
    WITH = 592,                    /* "with"  */
    ICMPX = 593,                   /* "icmpx"  */
    SNAT = 594,                    /* "snat"  */
    DNAT = 595,                    /* "dnat"  */
    MASQUERADE = 596,              /* "masquerade"  */
    REDIRECT = 597,                /* "redirect"  */
    RANDOM = 598,                  /* "random"  */
    FULLY_RANDOM = 599,            /* "fully-random"  */
    PERSISTENT = 600,              /* "persistent"  */
    QUEUE = 601,                   /* "queue"  */
    QUEUENUM = 602,                /* "num"  */
    BYPASS = 603,                  /* "bypass"  */
    FANOUT = 604,                  /* "fanout"  */
    DUP = 605,                     /* "dup"  */
    FWD = 606,                     /* "fwd"  */
    NUMGEN = 607,                  /* "numgen"  */
    INC = 608,                     /* "inc"  */
    MOD = 609,                     /* "mod"  */
    OFFSET = 610,                  /* "offset"  */
    JHASH = 611,                   /* "jhash"  */
    SYMHASH = 612,                 /* "symhash"  */
    SEED = 613,                    /* "seed"  */
    POSITION = 614,                /* "position"  */
    INDEX = 615,                   /* "index"  */
    COMMENT = 616,                 /* "comment"  */
    XML = 617,                     /* "xml"  */
    JSON = 618,                    /* "json"  */
    VM = 619,                      /* "vm"  */
    NOTRACK = 620,                 /* "notrack"  */
    EXISTS = 621,                  /* "exists"  */
    MISSING = 622,                 /* "missing"  */
    EXTHDR = 623,                  /* "exthdr"  */
    IPSEC = 624,                   /* "ipsec"  */
    REQID = 625,                   /* "reqid"  */
    SPNUM = 626,                   /* "spnum"  */
    IN = 627,                      /* "in"  */
    OUT = 628,                     /* "out"  */
    XT = 629                       /* "xt"  */
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
#define PATH 317
#define INET 318
#define NETDEV 319
#define ADD 320
#define UPDATE 321
#define REPLACE 322
#define CREATE 323
#define INSERT 324
#define DELETE 325
#define GET 326
#define LIST 327
#define RESET 328
#define FLUSH 329
#define RENAME 330
#define DESCRIBE 331
#define IMPORT 332
#define EXPORT 333
#define DESTROY 334
#define MONITOR 335
#define ALL 336
#define ACCEPT 337
#define DROP 338
#define CONTINUE 339
#define JUMP 340
#define GOTO 341
#define RETURN 342
#define TO 343
#define CONSTANT 344
#define INTERVAL 345
#define DYNAMIC 346
#define AUTOMERGE 347
#define TIMEOUT 348
#define GC_INTERVAL 349
#define ELEMENTS 350
#define EXPIRES 351
#define POLICY 352
#define MEMORY 353
#define PERFORMANCE 354
#define SIZE 355
#define FLOW 356
#define OFFLOAD 357
#define METER 358
#define METERS 359
#define FLOWTABLES 360
#define NUM 361
#define STRING 362
#define QUOTED_STRING 363
#define ASTERISK_STRING 364
#define LL_HDR 365
#define NETWORK_HDR 366
#define TRANSPORT_HDR 367
#define BRIDGE 368
#define ETHER 369
#define SADDR 370
#define DADDR 371
#define TYPE 372
#define VLAN 373
#define ID 374
#define CFI 375
#define DEI 376
#define PCP 377
#define ARP 378
#define HTYPE 379
#define PTYPE 380
#define HLEN 381
#define PLEN 382
#define OPERATION 383
#define IP 384
#define HDRVERSION 385
#define HDRLENGTH 386
#define DSCP 387
#define ECN 388
#define LENGTH 389
#define FRAG_OFF 390
#define TTL 391
#define TOS 392
#define PROTOCOL 393
#define CHECKSUM 394
#define PTR 395
#define VALUE 396
#define LSRR 397
#define RR 398
#define SSRR 399
#define RA 400
#define ICMP 401
#define CODE 402
#define SEQUENCE 403
#define GATEWAY 404
#define MTU 405
#define IGMP 406
#define MRT 407
#define OPTIONS 408
#define IP6 409
#define PRIORITY 410
#define FLOWLABEL 411
#define NEXTHDR 412
#define HOPLIMIT 413
#define ICMP6 414
#define PPTR 415
#define MAXDELAY 416
#define TADDR 417
#define AH 418
#define RESERVED 419
#define SPI 420
#define ESP 421
#define COMP 422
#define FLAGS 423
#define CPI 424
#define PORT 425
#define UDP 426
#define SPORT 427
#define DPORT 428
#define UDPLITE 429
#define CSUMCOV 430
#define TCP 431
#define ACKSEQ 432
#define DOFF 433
#define WINDOW 434
#define URGPTR 435
#define OPTION 436
#define ECHO 437
#define EOL 438
#define MPTCP 439
#define NOP 440
#define SACK 441
#define SACK0 442
#define SACK1 443
#define SACK2 444
#define SACK3 445
#define SACK_PERM 446
#define FASTOPEN 447
#define MD5SIG 448
#define TIMESTAMP 449
#define COUNT 450
#define LEFT 451
#define RIGHT 452
#define TSVAL 453
#define TSECR 454
#define SUBTYPE 455
#define DCCP 456
#define VXLAN 457
#define VNI 458
#define GRE 459
#define GRETAP 460
#define GENEVE 461
#define SCTP 462
#define CHUNK 463
#define DATA 464
#define INIT 465
#define INIT_ACK 466
#define HEARTBEAT 467
#define HEARTBEAT_ACK 468
#define ABORT 469
#define SHUTDOWN 470
#define SHUTDOWN_ACK 471
#define ERROR 472
#define COOKIE_ECHO 473
#define COOKIE_ACK 474
#define ECNE 475
#define CWR 476
#define SHUTDOWN_COMPLETE 477
#define ASCONF_ACK 478
#define FORWARD_TSN 479
#define ASCONF 480
#define TSN 481
#define STREAM 482
#define SSN 483
#define PPID 484
#define INIT_TAG 485
#define A_RWND 486
#define NUM_OSTREAMS 487
#define NUM_ISTREAMS 488
#define INIT_TSN 489
#define CUM_TSN_ACK 490
#define NUM_GACK_BLOCKS 491
#define NUM_DUP_TSNS 492
#define LOWEST_TSN 493
#define SEQNO 494
#define NEW_CUM_TSN 495
#define VTAG 496
#define RT 497
#define RT0 498
#define RT2 499
#define RT4 500
#define SEG_LEFT 501
#define ADDR 502
#define LAST_ENT 503
#define TAG 504
#define SID 505
#define HBH 506
#define FRAG 507
#define RESERVED2 508
#define MORE_FRAGMENTS 509
#define DST 510
#define MH 511
#define META 512
#define MARK 513
#define IIF 514
#define IIFNAME 515
#define IIFTYPE 516
#define OIF 517
#define OIFNAME 518
#define OIFTYPE 519
#define SKUID 520
#define SKGID 521
#define NFTRACE 522
#define RTCLASSID 523
#define IBRIPORT 524
#define OBRIPORT 525
#define IBRIDGENAME 526
#define OBRIDGENAME 527
#define PKTTYPE 528
#define CPU 529
#define IIFGROUP 530
#define OIFGROUP 531
#define CGROUP 532
#define TIME 533
#define CLASSID 534
#define NEXTHOP 535
#define CT 536
#define L3PROTOCOL 537
#define PROTO_SRC 538
#define PROTO_DST 539
#define ZONE 540
#define DIRECTION 541
#define EVENT 542
#define EXPECTATION 543
#define EXPIRATION 544
#define HELPER 545
#define LABEL 546
#define STATE 547
#define STATUS 548
#define ORIGINAL 549
#define REPLY 550
#define COUNTER 551
#define NAME 552
#define PACKETS 553
#define BYTES 554
#define AVGPKT 555
#define LAST 556
#define NEVER 557
#define TUNNEL 558
#define ERSPAN 559
#define EGRESS 560
#define INGRESS 561
#define GBP 562
#define CLASS 563
#define OPTTYPE 564
#define COUNTERS 565
#define QUOTAS 566
#define LIMITS 567
#define TUNNELS 568
#define SYNPROXYS 569
#define HELPERS 570
#define LOG 571
#define PREFIX 572
#define GROUP 573
#define SNAPLEN 574
#define QUEUE_THRESHOLD 575
#define LEVEL 576
#define LIMIT 577
#define RATE 578
#define BURST 579
#define OVER 580
#define UNTIL 581
#define QUOTA 582
#define USED 583
#define SECMARK 584
#define SECMARKS 585
#define SECOND 586
#define MINUTE 587
#define HOUR 588
#define DAY 589
#define WEEK 590
#define _REJECT 591
#define WITH 592
#define ICMPX 593
#define SNAT 594
#define DNAT 595
#define MASQUERADE 596
#define REDIRECT 597
#define RANDOM 598
#define FULLY_RANDOM 599
#define PERSISTENT 600
#define QUEUE 601
#define QUEUENUM 602
#define BYPASS 603
#define FANOUT 604
#define DUP 605
#define FWD 606
#define NUMGEN 607
#define INC 608
#define MOD 609
#define OFFSET 610
#define JHASH 611
#define SYMHASH 612
#define SEED 613
#define POSITION 614
#define INDEX 615
#define COMMENT 616
#define XML 617
#define JSON 618
#define VM 619
#define NOTRACK 620
#define EXISTS 621
#define MISSING 622
#define EXTHDR 623
#define IPSEC 624
#define REQID 625
#define SPNUM 626
#define IN 627
#define OUT 628
#define XT 629

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 235 "src/parser_bison.y"

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

#line 845 "src/parser_bison.h"

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

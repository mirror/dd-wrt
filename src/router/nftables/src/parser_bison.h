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
    NEW = 334,                     /* "new"  */
    DESTROY = 335,                 /* "destroy"  */
    MONITOR = 336,                 /* "monitor"  */
    ALL = 337,                     /* "all"  */
    ACCEPT = 338,                  /* "accept"  */
    DROP = 339,                    /* "drop"  */
    CONTINUE = 340,                /* "continue"  */
    JUMP = 341,                    /* "jump"  */
    GOTO = 342,                    /* "goto"  */
    RETURN = 343,                  /* "return"  */
    TO = 344,                      /* "to"  */
    CONSTANT = 345,                /* "constant"  */
    INTERVAL = 346,                /* "interval"  */
    DYNAMIC = 347,                 /* "dynamic"  */
    AUTOMERGE = 348,               /* "auto-merge"  */
    TIMEOUT = 349,                 /* "timeout"  */
    GC_INTERVAL = 350,             /* "gc-interval"  */
    ELEMENTS = 351,                /* "elements"  */
    EXPIRES = 352,                 /* "expires"  */
    POLICY = 353,                  /* "policy"  */
    MEMORY = 354,                  /* "memory"  */
    PERFORMANCE = 355,             /* "performance"  */
    SIZE = 356,                    /* "size"  */
    FLOW = 357,                    /* "flow"  */
    OFFLOAD = 358,                 /* "offload"  */
    METER = 359,                   /* "meter"  */
    METERS = 360,                  /* "meters"  */
    FLOWTABLES = 361,              /* "flowtables"  */
    NUM = 362,                     /* "number"  */
    STRING = 363,                  /* "string"  */
    QUOTED_STRING = 364,           /* "quoted string"  */
    ASTERISK_STRING = 365,         /* "string with a trailing asterisk"  */
    LL_HDR = 366,                  /* "ll"  */
    NETWORK_HDR = 367,             /* "nh"  */
    TRANSPORT_HDR = 368,           /* "th"  */
    BRIDGE = 369,                  /* "bridge"  */
    ETHER = 370,                   /* "ether"  */
    SADDR = 371,                   /* "saddr"  */
    DADDR = 372,                   /* "daddr"  */
    TYPE = 373,                    /* "type"  */
    VLAN = 374,                    /* "vlan"  */
    ID = 375,                      /* "id"  */
    CFI = 376,                     /* "cfi"  */
    DEI = 377,                     /* "dei"  */
    PCP = 378,                     /* "pcp"  */
    ARP = 379,                     /* "arp"  */
    HTYPE = 380,                   /* "htype"  */
    PTYPE = 381,                   /* "ptype"  */
    HLEN = 382,                    /* "hlen"  */
    PLEN = 383,                    /* "plen"  */
    OPERATION = 384,               /* "operation"  */
    IP = 385,                      /* "ip"  */
    HDRVERSION = 386,              /* "version"  */
    HDRLENGTH = 387,               /* "hdrlength"  */
    DSCP = 388,                    /* "dscp"  */
    ECN = 389,                     /* "ecn"  */
    LENGTH = 390,                  /* "length"  */
    FRAG_OFF = 391,                /* "frag-off"  */
    TTL = 392,                     /* "ttl"  */
    TOS = 393,                     /* "tos"  */
    PROTOCOL = 394,                /* "protocol"  */
    CHECKSUM = 395,                /* "checksum"  */
    PTR = 396,                     /* "ptr"  */
    VALUE = 397,                   /* "value"  */
    LSRR = 398,                    /* "lsrr"  */
    RR = 399,                      /* "rr"  */
    SSRR = 400,                    /* "ssrr"  */
    RA = 401,                      /* "ra"  */
    ICMP = 402,                    /* "icmp"  */
    CODE = 403,                    /* "code"  */
    SEQUENCE = 404,                /* "seq"  */
    GATEWAY = 405,                 /* "gateway"  */
    MTU = 406,                     /* "mtu"  */
    IGMP = 407,                    /* "igmp"  */
    MRT = 408,                     /* "mrt"  */
    OPTIONS = 409,                 /* "options"  */
    IP6 = 410,                     /* "ip6"  */
    PRIORITY = 411,                /* "priority"  */
    FLOWLABEL = 412,               /* "flowlabel"  */
    NEXTHDR = 413,                 /* "nexthdr"  */
    HOPLIMIT = 414,                /* "hoplimit"  */
    ICMP6 = 415,                   /* "icmpv6"  */
    PPTR = 416,                    /* "param-problem"  */
    MAXDELAY = 417,                /* "max-delay"  */
    TADDR = 418,                   /* "taddr"  */
    AH = 419,                      /* "ah"  */
    RESERVED = 420,                /* "reserved"  */
    SPI = 421,                     /* "spi"  */
    ESP = 422,                     /* "esp"  */
    COMP = 423,                    /* "comp"  */
    FLAGS = 424,                   /* "flags"  */
    CPI = 425,                     /* "cpi"  */
    PORT = 426,                    /* "port"  */
    UDP = 427,                     /* "udp"  */
    SPORT = 428,                   /* "sport"  */
    DPORT = 429,                   /* "dport"  */
    UDPLITE = 430,                 /* "udplite"  */
    CSUMCOV = 431,                 /* "csumcov"  */
    TCP = 432,                     /* "tcp"  */
    ACKSEQ = 433,                  /* "ackseq"  */
    DOFF = 434,                    /* "doff"  */
    WINDOW = 435,                  /* "window"  */
    URGPTR = 436,                  /* "urgptr"  */
    OPTION = 437,                  /* "option"  */
    ECHO = 438,                    /* "echo"  */
    EOL = 439,                     /* "eol"  */
    MPTCP = 440,                   /* "mptcp"  */
    NOP = 441,                     /* "nop"  */
    SACK = 442,                    /* "sack"  */
    SACK0 = 443,                   /* "sack0"  */
    SACK1 = 444,                   /* "sack1"  */
    SACK2 = 445,                   /* "sack2"  */
    SACK3 = 446,                   /* "sack3"  */
    SACK_PERM = 447,               /* "sack-permitted"  */
    FASTOPEN = 448,                /* "fastopen"  */
    MD5SIG = 449,                  /* "md5sig"  */
    TIMESTAMP = 450,               /* "timestamp"  */
    COUNT = 451,                   /* "count"  */
    LEFT = 452,                    /* "left"  */
    RIGHT = 453,                   /* "right"  */
    TSVAL = 454,                   /* "tsval"  */
    TSECR = 455,                   /* "tsecr"  */
    SUBTYPE = 456,                 /* "subtype"  */
    DCCP = 457,                    /* "dccp"  */
    VXLAN = 458,                   /* "vxlan"  */
    VNI = 459,                     /* "vni"  */
    GRE = 460,                     /* "gre"  */
    GRETAP = 461,                  /* "gretap"  */
    GENEVE = 462,                  /* "geneve"  */
    SCTP = 463,                    /* "sctp"  */
    CHUNK = 464,                   /* "chunk"  */
    DATA = 465,                    /* "data"  */
    INIT = 466,                    /* "init"  */
    INIT_ACK = 467,                /* "init-ack"  */
    HEARTBEAT = 468,               /* "heartbeat"  */
    HEARTBEAT_ACK = 469,           /* "heartbeat-ack"  */
    ABORT = 470,                   /* "abort"  */
    SHUTDOWN = 471,                /* "shutdown"  */
    SHUTDOWN_ACK = 472,            /* "shutdown-ack"  */
    ERROR = 473,                   /* "error"  */
    COOKIE_ECHO = 474,             /* "cookie-echo"  */
    COOKIE_ACK = 475,              /* "cookie-ack"  */
    ECNE = 476,                    /* "ecne"  */
    CWR = 477,                     /* "cwr"  */
    SHUTDOWN_COMPLETE = 478,       /* "shutdown-complete"  */
    ASCONF_ACK = 479,              /* "asconf-ack"  */
    FORWARD_TSN = 480,             /* "forward-tsn"  */
    ASCONF = 481,                  /* "asconf"  */
    TSN = 482,                     /* "tsn"  */
    STREAM = 483,                  /* "stream"  */
    SSN = 484,                     /* "ssn"  */
    PPID = 485,                    /* "ppid"  */
    INIT_TAG = 486,                /* "init-tag"  */
    A_RWND = 487,                  /* "a-rwnd"  */
    NUM_OSTREAMS = 488,            /* "num-outbound-streams"  */
    NUM_ISTREAMS = 489,            /* "num-inbound-streams"  */
    INIT_TSN = 490,                /* "initial-tsn"  */
    CUM_TSN_ACK = 491,             /* "cum-tsn-ack"  */
    NUM_GACK_BLOCKS = 492,         /* "num-gap-ack-blocks"  */
    NUM_DUP_TSNS = 493,            /* "num-dup-tsns"  */
    LOWEST_TSN = 494,              /* "lowest-tsn"  */
    SEQNO = 495,                   /* "seqno"  */
    NEW_CUM_TSN = 496,             /* "new-cum-tsn"  */
    VTAG = 497,                    /* "vtag"  */
    RT = 498,                      /* "rt"  */
    RT0 = 499,                     /* "rt0"  */
    RT2 = 500,                     /* "rt2"  */
    RT4 = 501,                     /* "srh"  */
    SEG_LEFT = 502,                /* "seg-left"  */
    ADDR = 503,                    /* "addr"  */
    LAST_ENT = 504,                /* "last-entry"  */
    TAG = 505,                     /* "tag"  */
    SID = 506,                     /* "sid"  */
    HBH = 507,                     /* "hbh"  */
    FRAG = 508,                    /* "frag"  */
    RESERVED2 = 509,               /* "reserved2"  */
    MORE_FRAGMENTS = 510,          /* "more-fragments"  */
    DST = 511,                     /* "dst"  */
    MH = 512,                      /* "mh"  */
    META = 513,                    /* "meta"  */
    MARK = 514,                    /* "mark"  */
    IIF = 515,                     /* "iif"  */
    IIFNAME = 516,                 /* "iifname"  */
    IIFTYPE = 517,                 /* "iiftype"  */
    OIF = 518,                     /* "oif"  */
    OIFNAME = 519,                 /* "oifname"  */
    OIFTYPE = 520,                 /* "oiftype"  */
    SKUID = 521,                   /* "skuid"  */
    SKGID = 522,                   /* "skgid"  */
    NFTRACE = 523,                 /* "nftrace"  */
    RTCLASSID = 524,               /* "rtclassid"  */
    IBRIDGENAME = 525,             /* "ibrname"  */
    OBRIDGENAME = 526,             /* "obrname"  */
    PKTTYPE = 527,                 /* "pkttype"  */
    CPU = 528,                     /* "cpu"  */
    IIFGROUP = 529,                /* "iifgroup"  */
    OIFGROUP = 530,                /* "oifgroup"  */
    CGROUP = 531,                  /* "cgroup"  */
    TIME = 532,                    /* "time"  */
    NFPROTO = 533,                 /* "nfproto"  */
    L4PROTO = 534,                 /* "l4proto"  */
    IIFKIND = 535,                 /* "iifkind"  */
    OIFKIND = 536,                 /* "oifkind"  */
    IBRPVID = 537,                 /* "ibrpvid"  */
    IBRVPROTO = 538,               /* "ibrvproto"  */
    SDIF = 539,                    /* "sdif"  */
    SDIFNAME = 540,                /* "sdifname"  */
    BROUTE = 541,                  /* "broute"  */
    BRIFHWADDR = 542,              /* "ibrhwaddr"  */
    CLASSID = 543,                 /* "classid"  */
    NEXTHOP = 544,                 /* "nexthop"  */
    CT = 545,                      /* "ct"  */
    L3PROTOCOL = 546,              /* "l3proto"  */
    PROTO_SRC = 547,               /* "proto-src"  */
    PROTO_DST = 548,               /* "proto-dst"  */
    ZONE = 549,                    /* "zone"  */
    DIRECTION = 550,               /* "direction"  */
    EVENT = 551,                   /* "event"  */
    EXPECTATION = 552,             /* "expectation"  */
    EXPIRATION = 553,              /* "expiration"  */
    HELPER = 554,                  /* "helper"  */
    LABEL = 555,                   /* "label"  */
    STATE = 556,                   /* "state"  */
    STATUS = 557,                  /* "status"  */
    ORIGINAL = 558,                /* "original"  */
    REPLY = 559,                   /* "reply"  */
    COUNTER = 560,                 /* "counter"  */
    NAME = 561,                    /* "name"  */
    PACKETS = 562,                 /* "packets"  */
    BYTES = 563,                   /* "bytes"  */
    KBYTES = 564,                  /* "kbytes"  */
    MBYTES = 565,                  /* "mbytes"  */
    AVGPKT = 566,                  /* "avgpkt"  */
    LAST = 567,                    /* "last"  */
    NEVER = 568,                   /* "never"  */
    TUNNEL = 569,                  /* "tunnel"  */
    ERSPAN = 570,                  /* "erspan"  */
    EGRESS = 571,                  /* "egress"  */
    INGRESS = 572,                 /* "ingress"  */
    GBP = 573,                     /* "gbp"  */
    CLASS = 574,                   /* "class"  */
    OPTTYPE = 575,                 /* "opt-type"  */
    COUNTERS = 576,                /* "counters"  */
    QUOTAS = 577,                  /* "quotas"  */
    LIMITS = 578,                  /* "limits"  */
    TUNNELS = 579,                 /* "tunnels"  */
    SYNPROXYS = 580,               /* "synproxys"  */
    COUNTS = 581,                  /* "counts"  */
    HELPERS = 582,                 /* "helpers"  */
    LOG = 583,                     /* "log"  */
    PREFIX = 584,                  /* "prefix"  */
    GROUP = 585,                   /* "group"  */
    SNAPLEN = 586,                 /* "snaplen"  */
    QUEUE_THRESHOLD = 587,         /* "queue-threshold"  */
    LEVEL = 588,                   /* "level"  */
    EMERG = 589,                   /* "emerg"  */
    ALERT = 590,                   /* "alert"  */
    CRIT = 591,                    /* "crit"  */
    ERR = 592,                     /* "err"  */
    WARN = 593,                    /* "warn"  */
    NOTICE = 594,                  /* "notice"  */
    INFO = 595,                    /* "info"  */
    DEBUG_TOKEN = 596,             /* "debug"  */
    AUDIT = 597,                   /* "audit"  */
    LIMIT = 598,                   /* "limit"  */
    RATE = 599,                    /* "rate"  */
    BURST = 600,                   /* "burst"  */
    OVER = 601,                    /* "over"  */
    UNTIL = 602,                   /* "until"  */
    QUOTA = 603,                   /* "quota"  */
    USED = 604,                    /* "used"  */
    SECMARK = 605,                 /* "secmark"  */
    SECMARKS = 606,                /* "secmarks"  */
    SECOND = 607,                  /* "second"  */
    MINUTE = 608,                  /* "minute"  */
    HOUR = 609,                    /* "hour"  */
    DAY = 610,                     /* "day"  */
    WEEK = 611,                    /* "week"  */
    _REJECT = 612,                 /* "reject"  */
    WITH = 613,                    /* "with"  */
    ICMPX = 614,                   /* "icmpx"  */
    SNAT = 615,                    /* "snat"  */
    DNAT = 616,                    /* "dnat"  */
    MASQUERADE = 617,              /* "masquerade"  */
    REDIRECT = 618,                /* "redirect"  */
    RANDOM = 619,                  /* "random"  */
    FULLY_RANDOM = 620,            /* "fully-random"  */
    PERSISTENT = 621,              /* "persistent"  */
    QUEUE = 622,                   /* "queue"  */
    QUEUENUM = 623,                /* "num"  */
    BYPASS = 624,                  /* "bypass"  */
    FANOUT = 625,                  /* "fanout"  */
    DUP = 626,                     /* "dup"  */
    FWD = 627,                     /* "fwd"  */
    NUMGEN = 628,                  /* "numgen"  */
    INC = 629,                     /* "inc"  */
    MOD = 630,                     /* "mod"  */
    OFFSET = 631,                  /* "offset"  */
    JHASH = 632,                   /* "jhash"  */
    SYMHASH = 633,                 /* "symhash"  */
    SEED = 634,                    /* "seed"  */
    POSITION = 635,                /* "position"  */
    INDEX = 636,                   /* "index"  */
    COMMENT = 637,                 /* "comment"  */
    XML = 638,                     /* "xml"  */
    JSON = 639,                    /* "json"  */
    VM = 640,                      /* "vm"  */
    NOTRACK = 641,                 /* "notrack"  */
    EXISTS = 642,                  /* "exists"  */
    MISSING = 643,                 /* "missing"  */
    EXTHDR = 644,                  /* "exthdr"  */
    IPSEC = 645,                   /* "ipsec"  */
    REQID = 646,                   /* "reqid"  */
    SPNUM = 647,                   /* "spnum"  */
    IN = 648,                      /* "in"  */
    OUT = 649,                     /* "out"  */
    XT = 650,                      /* "xt"  */
    FILTER = 651,                  /* "filter"  */
    NAT = 652,                     /* "nat"  */
    ROUTE = 653,                   /* "route"  */
    LOOSE = 654,                   /* "loose"  */
    SKIP = 655                     /* "skip"  */
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
#define NEW 334
#define DESTROY 335
#define MONITOR 336
#define ALL 337
#define ACCEPT 338
#define DROP 339
#define CONTINUE 340
#define JUMP 341
#define GOTO 342
#define RETURN 343
#define TO 344
#define CONSTANT 345
#define INTERVAL 346
#define DYNAMIC 347
#define AUTOMERGE 348
#define TIMEOUT 349
#define GC_INTERVAL 350
#define ELEMENTS 351
#define EXPIRES 352
#define POLICY 353
#define MEMORY 354
#define PERFORMANCE 355
#define SIZE 356
#define FLOW 357
#define OFFLOAD 358
#define METER 359
#define METERS 360
#define FLOWTABLES 361
#define NUM 362
#define STRING 363
#define QUOTED_STRING 364
#define ASTERISK_STRING 365
#define LL_HDR 366
#define NETWORK_HDR 367
#define TRANSPORT_HDR 368
#define BRIDGE 369
#define ETHER 370
#define SADDR 371
#define DADDR 372
#define TYPE 373
#define VLAN 374
#define ID 375
#define CFI 376
#define DEI 377
#define PCP 378
#define ARP 379
#define HTYPE 380
#define PTYPE 381
#define HLEN 382
#define PLEN 383
#define OPERATION 384
#define IP 385
#define HDRVERSION 386
#define HDRLENGTH 387
#define DSCP 388
#define ECN 389
#define LENGTH 390
#define FRAG_OFF 391
#define TTL 392
#define TOS 393
#define PROTOCOL 394
#define CHECKSUM 395
#define PTR 396
#define VALUE 397
#define LSRR 398
#define RR 399
#define SSRR 400
#define RA 401
#define ICMP 402
#define CODE 403
#define SEQUENCE 404
#define GATEWAY 405
#define MTU 406
#define IGMP 407
#define MRT 408
#define OPTIONS 409
#define IP6 410
#define PRIORITY 411
#define FLOWLABEL 412
#define NEXTHDR 413
#define HOPLIMIT 414
#define ICMP6 415
#define PPTR 416
#define MAXDELAY 417
#define TADDR 418
#define AH 419
#define RESERVED 420
#define SPI 421
#define ESP 422
#define COMP 423
#define FLAGS 424
#define CPI 425
#define PORT 426
#define UDP 427
#define SPORT 428
#define DPORT 429
#define UDPLITE 430
#define CSUMCOV 431
#define TCP 432
#define ACKSEQ 433
#define DOFF 434
#define WINDOW 435
#define URGPTR 436
#define OPTION 437
#define ECHO 438
#define EOL 439
#define MPTCP 440
#define NOP 441
#define SACK 442
#define SACK0 443
#define SACK1 444
#define SACK2 445
#define SACK3 446
#define SACK_PERM 447
#define FASTOPEN 448
#define MD5SIG 449
#define TIMESTAMP 450
#define COUNT 451
#define LEFT 452
#define RIGHT 453
#define TSVAL 454
#define TSECR 455
#define SUBTYPE 456
#define DCCP 457
#define VXLAN 458
#define VNI 459
#define GRE 460
#define GRETAP 461
#define GENEVE 462
#define SCTP 463
#define CHUNK 464
#define DATA 465
#define INIT 466
#define INIT_ACK 467
#define HEARTBEAT 468
#define HEARTBEAT_ACK 469
#define ABORT 470
#define SHUTDOWN 471
#define SHUTDOWN_ACK 472
#define ERROR 473
#define COOKIE_ECHO 474
#define COOKIE_ACK 475
#define ECNE 476
#define CWR 477
#define SHUTDOWN_COMPLETE 478
#define ASCONF_ACK 479
#define FORWARD_TSN 480
#define ASCONF 481
#define TSN 482
#define STREAM 483
#define SSN 484
#define PPID 485
#define INIT_TAG 486
#define A_RWND 487
#define NUM_OSTREAMS 488
#define NUM_ISTREAMS 489
#define INIT_TSN 490
#define CUM_TSN_ACK 491
#define NUM_GACK_BLOCKS 492
#define NUM_DUP_TSNS 493
#define LOWEST_TSN 494
#define SEQNO 495
#define NEW_CUM_TSN 496
#define VTAG 497
#define RT 498
#define RT0 499
#define RT2 500
#define RT4 501
#define SEG_LEFT 502
#define ADDR 503
#define LAST_ENT 504
#define TAG 505
#define SID 506
#define HBH 507
#define FRAG 508
#define RESERVED2 509
#define MORE_FRAGMENTS 510
#define DST 511
#define MH 512
#define META 513
#define MARK 514
#define IIF 515
#define IIFNAME 516
#define IIFTYPE 517
#define OIF 518
#define OIFNAME 519
#define OIFTYPE 520
#define SKUID 521
#define SKGID 522
#define NFTRACE 523
#define RTCLASSID 524
#define IBRIDGENAME 525
#define OBRIDGENAME 526
#define PKTTYPE 527
#define CPU 528
#define IIFGROUP 529
#define OIFGROUP 530
#define CGROUP 531
#define TIME 532
#define NFPROTO 533
#define L4PROTO 534
#define IIFKIND 535
#define OIFKIND 536
#define IBRPVID 537
#define IBRVPROTO 538
#define SDIF 539
#define SDIFNAME 540
#define BROUTE 541
#define BRIFHWADDR 542
#define CLASSID 543
#define NEXTHOP 544
#define CT 545
#define L3PROTOCOL 546
#define PROTO_SRC 547
#define PROTO_DST 548
#define ZONE 549
#define DIRECTION 550
#define EVENT 551
#define EXPECTATION 552
#define EXPIRATION 553
#define HELPER 554
#define LABEL 555
#define STATE 556
#define STATUS 557
#define ORIGINAL 558
#define REPLY 559
#define COUNTER 560
#define NAME 561
#define PACKETS 562
#define BYTES 563
#define KBYTES 564
#define MBYTES 565
#define AVGPKT 566
#define LAST 567
#define NEVER 568
#define TUNNEL 569
#define ERSPAN 570
#define EGRESS 571
#define INGRESS 572
#define GBP 573
#define CLASS 574
#define OPTTYPE 575
#define COUNTERS 576
#define QUOTAS 577
#define LIMITS 578
#define TUNNELS 579
#define SYNPROXYS 580
#define COUNTS 581
#define HELPERS 582
#define LOG 583
#define PREFIX 584
#define GROUP 585
#define SNAPLEN 586
#define QUEUE_THRESHOLD 587
#define LEVEL 588
#define EMERG 589
#define ALERT 590
#define CRIT 591
#define ERR 592
#define WARN 593
#define NOTICE 594
#define INFO 595
#define DEBUG_TOKEN 596
#define AUDIT 597
#define LIMIT 598
#define RATE 599
#define BURST 600
#define OVER 601
#define UNTIL 602
#define QUOTA 603
#define USED 604
#define SECMARK 605
#define SECMARKS 606
#define SECOND 607
#define MINUTE 608
#define HOUR 609
#define DAY 610
#define WEEK 611
#define _REJECT 612
#define WITH 613
#define ICMPX 614
#define SNAT 615
#define DNAT 616
#define MASQUERADE 617
#define REDIRECT 618
#define RANDOM 619
#define FULLY_RANDOM 620
#define PERSISTENT 621
#define QUEUE 622
#define QUEUENUM 623
#define BYPASS 624
#define FANOUT 625
#define DUP 626
#define FWD 627
#define NUMGEN 628
#define INC 629
#define MOD 630
#define OFFSET 631
#define JHASH 632
#define SYMHASH 633
#define SEED 634
#define POSITION 635
#define INDEX 636
#define COMMENT 637
#define XML 638
#define JSON 639
#define VM 640
#define NOTRACK 641
#define EXISTS 642
#define MISSING 643
#define EXTHDR 644
#define IPSEC 645
#define REQID 646
#define SPNUM 647
#define IN 648
#define OUT 649
#define XT 650
#define FILTER 651
#define NAT 652
#define ROUTE 653
#define LOOSE 654
#define SKIP 655

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 234 "src/parser_bison.y"

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

#line 897 "src/parser_bison.h"

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

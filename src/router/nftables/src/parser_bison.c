/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         nft_parse
#define yylex           nft_lex
#define yyerror         nft_error
#define yydebug         nft_debug
#define yynerrs         nft_nerrs

/* First part of user prologue.  */
#line 11 "src/parser_bison.y"

#include <nft.h>

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <syslog.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/nf_log.h>
#include <linux/netfilter/nfnetlink_osf.h>
#include <linux/netfilter/nf_synproxy.h>
#include <linux/xfrm.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <libnftnl/common.h>
#include <libnftnl/set.h>
#include <libnftnl/udata.h>

#include <rule.h>
#include <cmd.h>
#include <statement.h>
#include <expression.h>
#include <headers.h>
#include <utils.h>
#include <parser.h>
#include <erec.h>
#include <sctp_chunk.h>

#include "parser_bison.h"

void parser_init(struct nft_ctx *nft, struct parser_state *state,
		 struct list_head *msgs, struct list_head *cmds,
		 struct scope *top_scope)
{
	memset(state, 0, sizeof(*state));
	state->msgs = msgs;
	state->cmds = cmds;
	state->scopes[0] = scope_init(top_scope, NULL);
	init_list_head(&state->indesc_list);
}

static void yyerror(struct location *loc, struct nft_ctx *nft, void *scanner,
		    struct parser_state *state, const char *s)
{
	erec_queue(error(loc, "%s", s), state->msgs);
}

static struct scope *current_scope(const struct parser_state *state)
{
	return state->scopes[state->scope];
}

static int open_scope(struct parser_state *state, struct scope *scope)
{
	if (state->scope >= array_size(state->scopes) - 1) {
		state->scope_err = true;
		return -1;
	}

	scope_init(scope, current_scope(state));
	state->scopes[++state->scope] = scope;

	return 0;
}

static void close_scope(struct parser_state *state)
{
	if (state->scope_err || state->scope == 0) {
		state->scope_err = false;
		return;
	}

	state->scope--;
}

static void location_init(void *scanner, struct parser_state *state,
			  struct location *loc)
{
	memset(loc, 0, sizeof(*loc));
	loc->indesc = state->indesc;
}

static void location_update(struct location *loc, struct location *rhs, int n)
{
	if (n) {
		loc->indesc       = rhs[n].indesc;
		loc->line_offset  = rhs[1].line_offset;
		loc->first_line   = rhs[1].first_line;
		loc->first_column = rhs[1].first_column;
		loc->last_column  = rhs[n].last_column;
	} else {
		loc->indesc       = rhs[0].indesc;
		loc->line_offset  = rhs[0].line_offset;
		loc->first_line   = rhs[0].first_line;
		loc->first_column = loc->last_column = rhs[0].last_column;
	}
}

static struct expr *handle_concat_expr(const struct location *loc,
					 struct expr *expr,
					 struct expr *expr_l, struct expr *expr_r,
					 struct location loc_rhs[3])
{
	if (expr->etype != EXPR_CONCAT) {
		expr = concat_expr_alloc(loc);
		compound_expr_add(expr, expr_l);
	} else {
		location_update(&expr_r->location, loc_rhs, 2);

		expr = expr_l;
		expr->location = *loc;
	}

	compound_expr_add(expr, expr_r);
	return expr;
}

static bool already_set(const void *attr, const struct location *loc,
			struct parser_state *state)
{
	if (!attr)
		return false;

	erec_queue(error(loc, "You can only specify this once. This statement is duplicated."),
		   state->msgs);
	return true;
}

static struct expr *ifname_expr_alloc(const struct location *location,
				      struct list_head *queue,
				      const char *name)
{
	size_t length = strlen(name);
	struct expr *expr;

	if (length == 0) {
		free_const(name);
		erec_queue(error(location, "empty interface name"), queue);
		return NULL;
	}

	if (length >= IFNAMSIZ) {
		free_const(name);
		erec_queue(error(location, "interface name too long"), queue);
		return NULL;
	}

	expr = constant_expr_alloc(location, &ifname_type, BYTEORDER_HOST_ENDIAN,
				   length * BITS_PER_BYTE, name);

	free_const(name);

	return expr;
}

static void timeout_state_free(struct timeout_state *s)
{
	free_const(s->timeout_str);
	free(s);
}

static void timeout_states_free(struct list_head *list)
{
	struct timeout_state *ts, *next;

	list_for_each_entry_safe(ts, next, list, head) {
		list_del(&ts->head);
		timeout_state_free(ts);
	}

	free(list);
}

#define YYLLOC_DEFAULT(Current, Rhs, N)	location_update(&Current, Rhs, N)

#define symbol_value(loc, str) \
	symbol_expr_alloc(loc, SYMBOL_VALUE, current_scope(state), str)

/* Declare those here to avoid compiler warnings */
void nft_set_debug(int, void *);
int nft_lex(void *, void *, void *);

#line 267 "src/parser_bison.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 1078 "src/parser_bison.c"

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
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_JUNK = 3,                       /* "junk"  */
  YYSYMBOL_CRLF = 4,                       /* "CRLF line terminators"  */
  YYSYMBOL_NEWLINE = 5,                    /* "newline"  */
  YYSYMBOL_COLON = 6,                      /* "colon"  */
  YYSYMBOL_SEMICOLON = 7,                  /* "semicolon"  */
  YYSYMBOL_COMMA = 8,                      /* "comma"  */
  YYSYMBOL_DOT = 9,                        /* "."  */
  YYSYMBOL_EQ = 10,                        /* "=="  */
  YYSYMBOL_NEQ = 11,                       /* "!="  */
  YYSYMBOL_LT = 12,                        /* "<"  */
  YYSYMBOL_GT = 13,                        /* ">"  */
  YYSYMBOL_GTE = 14,                       /* ">="  */
  YYSYMBOL_LTE = 15,                       /* "<="  */
  YYSYMBOL_LSHIFT = 16,                    /* "<<"  */
  YYSYMBOL_RSHIFT = 17,                    /* ">>"  */
  YYSYMBOL_AMPERSAND = 18,                 /* "&"  */
  YYSYMBOL_CARET = 19,                     /* "^"  */
  YYSYMBOL_NOT = 20,                       /* "!"  */
  YYSYMBOL_SLASH = 21,                     /* "/"  */
  YYSYMBOL_ASTERISK = 22,                  /* "*"  */
  YYSYMBOL_DASH = 23,                      /* "-"  */
  YYSYMBOL_AT = 24,                        /* "@"  */
  YYSYMBOL_VMAP = 25,                      /* "vmap"  */
  YYSYMBOL_PLUS = 26,                      /* "+"  */
  YYSYMBOL_INCLUDE = 27,                   /* "include"  */
  YYSYMBOL_DEFINE = 28,                    /* "define"  */
  YYSYMBOL_REDEFINE = 29,                  /* "redefine"  */
  YYSYMBOL_UNDEFINE = 30,                  /* "undefine"  */
  YYSYMBOL_FIB = 31,                       /* "fib"  */
  YYSYMBOL_CHECK = 32,                     /* "check"  */
  YYSYMBOL_SOCKET = 33,                    /* "socket"  */
  YYSYMBOL_TRANSPARENT = 34,               /* "transparent"  */
  YYSYMBOL_WILDCARD = 35,                  /* "wildcard"  */
  YYSYMBOL_CGROUPV2 = 36,                  /* "cgroupv2"  */
  YYSYMBOL_TPROXY = 37,                    /* "tproxy"  */
  YYSYMBOL_OSF = 38,                       /* "osf"  */
  YYSYMBOL_SYNPROXY = 39,                  /* "synproxy"  */
  YYSYMBOL_MSS = 40,                       /* "mss"  */
  YYSYMBOL_WSCALE = 41,                    /* "wscale"  */
  YYSYMBOL_TYPEOF = 42,                    /* "typeof"  */
  YYSYMBOL_HOOK = 43,                      /* "hook"  */
  YYSYMBOL_HOOKS = 44,                     /* "hooks"  */
  YYSYMBOL_DEVICE = 45,                    /* "device"  */
  YYSYMBOL_DEVICES = 46,                   /* "devices"  */
  YYSYMBOL_TABLE = 47,                     /* "table"  */
  YYSYMBOL_TABLES = 48,                    /* "tables"  */
  YYSYMBOL_CHAIN = 49,                     /* "chain"  */
  YYSYMBOL_CHAINS = 50,                    /* "chains"  */
  YYSYMBOL_RULE = 51,                      /* "rule"  */
  YYSYMBOL_RULES = 52,                     /* "rules"  */
  YYSYMBOL_SETS = 53,                      /* "sets"  */
  YYSYMBOL_SET = 54,                       /* "set"  */
  YYSYMBOL_ELEMENT = 55,                   /* "element"  */
  YYSYMBOL_MAP = 56,                       /* "map"  */
  YYSYMBOL_MAPS = 57,                      /* "maps"  */
  YYSYMBOL_FLOWTABLE = 58,                 /* "flowtable"  */
  YYSYMBOL_HANDLE = 59,                    /* "handle"  */
  YYSYMBOL_RULESET = 60,                   /* "ruleset"  */
  YYSYMBOL_TRACE = 61,                     /* "trace"  */
  YYSYMBOL_INET = 62,                      /* "inet"  */
  YYSYMBOL_NETDEV = 63,                    /* "netdev"  */
  YYSYMBOL_ADD = 64,                       /* "add"  */
  YYSYMBOL_UPDATE = 65,                    /* "update"  */
  YYSYMBOL_REPLACE = 66,                   /* "replace"  */
  YYSYMBOL_CREATE = 67,                    /* "create"  */
  YYSYMBOL_INSERT = 68,                    /* "insert"  */
  YYSYMBOL_DELETE = 69,                    /* "delete"  */
  YYSYMBOL_GET = 70,                       /* "get"  */
  YYSYMBOL_LIST = 71,                      /* "list"  */
  YYSYMBOL_RESET = 72,                     /* "reset"  */
  YYSYMBOL_FLUSH = 73,                     /* "flush"  */
  YYSYMBOL_RENAME = 74,                    /* "rename"  */
  YYSYMBOL_DESCRIBE = 75,                  /* "describe"  */
  YYSYMBOL_IMPORT = 76,                    /* "import"  */
  YYSYMBOL_EXPORT = 77,                    /* "export"  */
  YYSYMBOL_DESTROY = 78,                   /* "destroy"  */
  YYSYMBOL_MONITOR = 79,                   /* "monitor"  */
  YYSYMBOL_ALL = 80,                       /* "all"  */
  YYSYMBOL_ACCEPT = 81,                    /* "accept"  */
  YYSYMBOL_DROP = 82,                      /* "drop"  */
  YYSYMBOL_CONTINUE = 83,                  /* "continue"  */
  YYSYMBOL_JUMP = 84,                      /* "jump"  */
  YYSYMBOL_GOTO = 85,                      /* "goto"  */
  YYSYMBOL_RETURN = 86,                    /* "return"  */
  YYSYMBOL_TO = 87,                        /* "to"  */
  YYSYMBOL_CONSTANT = 88,                  /* "constant"  */
  YYSYMBOL_INTERVAL = 89,                  /* "interval"  */
  YYSYMBOL_DYNAMIC = 90,                   /* "dynamic"  */
  YYSYMBOL_AUTOMERGE = 91,                 /* "auto-merge"  */
  YYSYMBOL_TIMEOUT = 92,                   /* "timeout"  */
  YYSYMBOL_GC_INTERVAL = 93,               /* "gc-interval"  */
  YYSYMBOL_ELEMENTS = 94,                  /* "elements"  */
  YYSYMBOL_EXPIRES = 95,                   /* "expires"  */
  YYSYMBOL_POLICY = 96,                    /* "policy"  */
  YYSYMBOL_MEMORY = 97,                    /* "memory"  */
  YYSYMBOL_PERFORMANCE = 98,               /* "performance"  */
  YYSYMBOL_SIZE = 99,                      /* "size"  */
  YYSYMBOL_FLOW = 100,                     /* "flow"  */
  YYSYMBOL_OFFLOAD = 101,                  /* "offload"  */
  YYSYMBOL_METER = 102,                    /* "meter"  */
  YYSYMBOL_METERS = 103,                   /* "meters"  */
  YYSYMBOL_FLOWTABLES = 104,               /* "flowtables"  */
  YYSYMBOL_NUM = 105,                      /* "number"  */
  YYSYMBOL_STRING = 106,                   /* "string"  */
  YYSYMBOL_QUOTED_STRING = 107,            /* "quoted string"  */
  YYSYMBOL_ASTERISK_STRING = 108,          /* "string with a trailing asterisk"  */
  YYSYMBOL_LL_HDR = 109,                   /* "ll"  */
  YYSYMBOL_NETWORK_HDR = 110,              /* "nh"  */
  YYSYMBOL_TRANSPORT_HDR = 111,            /* "th"  */
  YYSYMBOL_BRIDGE = 112,                   /* "bridge"  */
  YYSYMBOL_ETHER = 113,                    /* "ether"  */
  YYSYMBOL_SADDR = 114,                    /* "saddr"  */
  YYSYMBOL_DADDR = 115,                    /* "daddr"  */
  YYSYMBOL_TYPE = 116,                     /* "type"  */
  YYSYMBOL_VLAN = 117,                     /* "vlan"  */
  YYSYMBOL_ID = 118,                       /* "id"  */
  YYSYMBOL_CFI = 119,                      /* "cfi"  */
  YYSYMBOL_DEI = 120,                      /* "dei"  */
  YYSYMBOL_PCP = 121,                      /* "pcp"  */
  YYSYMBOL_ARP = 122,                      /* "arp"  */
  YYSYMBOL_HTYPE = 123,                    /* "htype"  */
  YYSYMBOL_PTYPE = 124,                    /* "ptype"  */
  YYSYMBOL_HLEN = 125,                     /* "hlen"  */
  YYSYMBOL_PLEN = 126,                     /* "plen"  */
  YYSYMBOL_OPERATION = 127,                /* "operation"  */
  YYSYMBOL_IP = 128,                       /* "ip"  */
  YYSYMBOL_HDRVERSION = 129,               /* "version"  */
  YYSYMBOL_HDRLENGTH = 130,                /* "hdrlength"  */
  YYSYMBOL_DSCP = 131,                     /* "dscp"  */
  YYSYMBOL_ECN = 132,                      /* "ecn"  */
  YYSYMBOL_LENGTH = 133,                   /* "length"  */
  YYSYMBOL_FRAG_OFF = 134,                 /* "frag-off"  */
  YYSYMBOL_TTL = 135,                      /* "ttl"  */
  YYSYMBOL_PROTOCOL = 136,                 /* "protocol"  */
  YYSYMBOL_CHECKSUM = 137,                 /* "checksum"  */
  YYSYMBOL_PTR = 138,                      /* "ptr"  */
  YYSYMBOL_VALUE = 139,                    /* "value"  */
  YYSYMBOL_LSRR = 140,                     /* "lsrr"  */
  YYSYMBOL_RR = 141,                       /* "rr"  */
  YYSYMBOL_SSRR = 142,                     /* "ssrr"  */
  YYSYMBOL_RA = 143,                       /* "ra"  */
  YYSYMBOL_ICMP = 144,                     /* "icmp"  */
  YYSYMBOL_CODE = 145,                     /* "code"  */
  YYSYMBOL_SEQUENCE = 146,                 /* "seq"  */
  YYSYMBOL_GATEWAY = 147,                  /* "gateway"  */
  YYSYMBOL_MTU = 148,                      /* "mtu"  */
  YYSYMBOL_IGMP = 149,                     /* "igmp"  */
  YYSYMBOL_MRT = 150,                      /* "mrt"  */
  YYSYMBOL_OPTIONS = 151,                  /* "options"  */
  YYSYMBOL_IP6 = 152,                      /* "ip6"  */
  YYSYMBOL_PRIORITY = 153,                 /* "priority"  */
  YYSYMBOL_FLOWLABEL = 154,                /* "flowlabel"  */
  YYSYMBOL_NEXTHDR = 155,                  /* "nexthdr"  */
  YYSYMBOL_HOPLIMIT = 156,                 /* "hoplimit"  */
  YYSYMBOL_ICMP6 = 157,                    /* "icmpv6"  */
  YYSYMBOL_PPTR = 158,                     /* "param-problem"  */
  YYSYMBOL_MAXDELAY = 159,                 /* "max-delay"  */
  YYSYMBOL_TADDR = 160,                    /* "taddr"  */
  YYSYMBOL_AH = 161,                       /* "ah"  */
  YYSYMBOL_RESERVED = 162,                 /* "reserved"  */
  YYSYMBOL_SPI = 163,                      /* "spi"  */
  YYSYMBOL_ESP = 164,                      /* "esp"  */
  YYSYMBOL_COMP = 165,                     /* "comp"  */
  YYSYMBOL_FLAGS = 166,                    /* "flags"  */
  YYSYMBOL_CPI = 167,                      /* "cpi"  */
  YYSYMBOL_PORT = 168,                     /* "port"  */
  YYSYMBOL_UDP = 169,                      /* "udp"  */
  YYSYMBOL_SPORT = 170,                    /* "sport"  */
  YYSYMBOL_DPORT = 171,                    /* "dport"  */
  YYSYMBOL_UDPLITE = 172,                  /* "udplite"  */
  YYSYMBOL_CSUMCOV = 173,                  /* "csumcov"  */
  YYSYMBOL_TCP = 174,                      /* "tcp"  */
  YYSYMBOL_ACKSEQ = 175,                   /* "ackseq"  */
  YYSYMBOL_DOFF = 176,                     /* "doff"  */
  YYSYMBOL_WINDOW = 177,                   /* "window"  */
  YYSYMBOL_URGPTR = 178,                   /* "urgptr"  */
  YYSYMBOL_OPTION = 179,                   /* "option"  */
  YYSYMBOL_ECHO = 180,                     /* "echo"  */
  YYSYMBOL_EOL = 181,                      /* "eol"  */
  YYSYMBOL_MPTCP = 182,                    /* "mptcp"  */
  YYSYMBOL_NOP = 183,                      /* "nop"  */
  YYSYMBOL_SACK = 184,                     /* "sack"  */
  YYSYMBOL_SACK0 = 185,                    /* "sack0"  */
  YYSYMBOL_SACK1 = 186,                    /* "sack1"  */
  YYSYMBOL_SACK2 = 187,                    /* "sack2"  */
  YYSYMBOL_SACK3 = 188,                    /* "sack3"  */
  YYSYMBOL_SACK_PERM = 189,                /* "sack-permitted"  */
  YYSYMBOL_FASTOPEN = 190,                 /* "fastopen"  */
  YYSYMBOL_MD5SIG = 191,                   /* "md5sig"  */
  YYSYMBOL_TIMESTAMP = 192,                /* "timestamp"  */
  YYSYMBOL_COUNT = 193,                    /* "count"  */
  YYSYMBOL_LEFT = 194,                     /* "left"  */
  YYSYMBOL_RIGHT = 195,                    /* "right"  */
  YYSYMBOL_TSVAL = 196,                    /* "tsval"  */
  YYSYMBOL_TSECR = 197,                    /* "tsecr"  */
  YYSYMBOL_SUBTYPE = 198,                  /* "subtype"  */
  YYSYMBOL_DCCP = 199,                     /* "dccp"  */
  YYSYMBOL_VXLAN = 200,                    /* "vxlan"  */
  YYSYMBOL_VNI = 201,                      /* "vni"  */
  YYSYMBOL_GRE = 202,                      /* "gre"  */
  YYSYMBOL_GRETAP = 203,                   /* "gretap"  */
  YYSYMBOL_GENEVE = 204,                   /* "geneve"  */
  YYSYMBOL_SCTP = 205,                     /* "sctp"  */
  YYSYMBOL_CHUNK = 206,                    /* "chunk"  */
  YYSYMBOL_DATA = 207,                     /* "data"  */
  YYSYMBOL_INIT = 208,                     /* "init"  */
  YYSYMBOL_INIT_ACK = 209,                 /* "init-ack"  */
  YYSYMBOL_HEARTBEAT = 210,                /* "heartbeat"  */
  YYSYMBOL_HEARTBEAT_ACK = 211,            /* "heartbeat-ack"  */
  YYSYMBOL_ABORT = 212,                    /* "abort"  */
  YYSYMBOL_SHUTDOWN = 213,                 /* "shutdown"  */
  YYSYMBOL_SHUTDOWN_ACK = 214,             /* "shutdown-ack"  */
  YYSYMBOL_ERROR = 215,                    /* "error"  */
  YYSYMBOL_COOKIE_ECHO = 216,              /* "cookie-echo"  */
  YYSYMBOL_COOKIE_ACK = 217,               /* "cookie-ack"  */
  YYSYMBOL_ECNE = 218,                     /* "ecne"  */
  YYSYMBOL_CWR = 219,                      /* "cwr"  */
  YYSYMBOL_SHUTDOWN_COMPLETE = 220,        /* "shutdown-complete"  */
  YYSYMBOL_ASCONF_ACK = 221,               /* "asconf-ack"  */
  YYSYMBOL_FORWARD_TSN = 222,              /* "forward-tsn"  */
  YYSYMBOL_ASCONF = 223,                   /* "asconf"  */
  YYSYMBOL_TSN = 224,                      /* "tsn"  */
  YYSYMBOL_STREAM = 225,                   /* "stream"  */
  YYSYMBOL_SSN = 226,                      /* "ssn"  */
  YYSYMBOL_PPID = 227,                     /* "ppid"  */
  YYSYMBOL_INIT_TAG = 228,                 /* "init-tag"  */
  YYSYMBOL_A_RWND = 229,                   /* "a-rwnd"  */
  YYSYMBOL_NUM_OSTREAMS = 230,             /* "num-outbound-streams"  */
  YYSYMBOL_NUM_ISTREAMS = 231,             /* "num-inbound-streams"  */
  YYSYMBOL_INIT_TSN = 232,                 /* "initial-tsn"  */
  YYSYMBOL_CUM_TSN_ACK = 233,              /* "cum-tsn-ack"  */
  YYSYMBOL_NUM_GACK_BLOCKS = 234,          /* "num-gap-ack-blocks"  */
  YYSYMBOL_NUM_DUP_TSNS = 235,             /* "num-dup-tsns"  */
  YYSYMBOL_LOWEST_TSN = 236,               /* "lowest-tsn"  */
  YYSYMBOL_SEQNO = 237,                    /* "seqno"  */
  YYSYMBOL_NEW_CUM_TSN = 238,              /* "new-cum-tsn"  */
  YYSYMBOL_VTAG = 239,                     /* "vtag"  */
  YYSYMBOL_RT = 240,                       /* "rt"  */
  YYSYMBOL_RT0 = 241,                      /* "rt0"  */
  YYSYMBOL_RT2 = 242,                      /* "rt2"  */
  YYSYMBOL_RT4 = 243,                      /* "srh"  */
  YYSYMBOL_SEG_LEFT = 244,                 /* "seg-left"  */
  YYSYMBOL_ADDR = 245,                     /* "addr"  */
  YYSYMBOL_LAST_ENT = 246,                 /* "last-entry"  */
  YYSYMBOL_TAG = 247,                      /* "tag"  */
  YYSYMBOL_SID = 248,                      /* "sid"  */
  YYSYMBOL_HBH = 249,                      /* "hbh"  */
  YYSYMBOL_FRAG = 250,                     /* "frag"  */
  YYSYMBOL_RESERVED2 = 251,                /* "reserved2"  */
  YYSYMBOL_MORE_FRAGMENTS = 252,           /* "more-fragments"  */
  YYSYMBOL_DST = 253,                      /* "dst"  */
  YYSYMBOL_MH = 254,                       /* "mh"  */
  YYSYMBOL_META = 255,                     /* "meta"  */
  YYSYMBOL_MARK = 256,                     /* "mark"  */
  YYSYMBOL_IIF = 257,                      /* "iif"  */
  YYSYMBOL_IIFNAME = 258,                  /* "iifname"  */
  YYSYMBOL_IIFTYPE = 259,                  /* "iiftype"  */
  YYSYMBOL_OIF = 260,                      /* "oif"  */
  YYSYMBOL_OIFNAME = 261,                  /* "oifname"  */
  YYSYMBOL_OIFTYPE = 262,                  /* "oiftype"  */
  YYSYMBOL_SKUID = 263,                    /* "skuid"  */
  YYSYMBOL_SKGID = 264,                    /* "skgid"  */
  YYSYMBOL_NFTRACE = 265,                  /* "nftrace"  */
  YYSYMBOL_RTCLASSID = 266,                /* "rtclassid"  */
  YYSYMBOL_IBRIPORT = 267,                 /* "ibriport"  */
  YYSYMBOL_OBRIPORT = 268,                 /* "obriport"  */
  YYSYMBOL_IBRIDGENAME = 269,              /* "ibrname"  */
  YYSYMBOL_OBRIDGENAME = 270,              /* "obrname"  */
  YYSYMBOL_PKTTYPE = 271,                  /* "pkttype"  */
  YYSYMBOL_CPU = 272,                      /* "cpu"  */
  YYSYMBOL_IIFGROUP = 273,                 /* "iifgroup"  */
  YYSYMBOL_OIFGROUP = 274,                 /* "oifgroup"  */
  YYSYMBOL_CGROUP = 275,                   /* "cgroup"  */
  YYSYMBOL_TIME = 276,                     /* "time"  */
  YYSYMBOL_CLASSID = 277,                  /* "classid"  */
  YYSYMBOL_NEXTHOP = 278,                  /* "nexthop"  */
  YYSYMBOL_CT = 279,                       /* "ct"  */
  YYSYMBOL_L3PROTOCOL = 280,               /* "l3proto"  */
  YYSYMBOL_PROTO_SRC = 281,                /* "proto-src"  */
  YYSYMBOL_PROTO_DST = 282,                /* "proto-dst"  */
  YYSYMBOL_ZONE = 283,                     /* "zone"  */
  YYSYMBOL_DIRECTION = 284,                /* "direction"  */
  YYSYMBOL_EVENT = 285,                    /* "event"  */
  YYSYMBOL_EXPECTATION = 286,              /* "expectation"  */
  YYSYMBOL_EXPIRATION = 287,               /* "expiration"  */
  YYSYMBOL_HELPER = 288,                   /* "helper"  */
  YYSYMBOL_LABEL = 289,                    /* "label"  */
  YYSYMBOL_STATE = 290,                    /* "state"  */
  YYSYMBOL_STATUS = 291,                   /* "status"  */
  YYSYMBOL_ORIGINAL = 292,                 /* "original"  */
  YYSYMBOL_REPLY = 293,                    /* "reply"  */
  YYSYMBOL_COUNTER = 294,                  /* "counter"  */
  YYSYMBOL_NAME = 295,                     /* "name"  */
  YYSYMBOL_PACKETS = 296,                  /* "packets"  */
  YYSYMBOL_BYTES = 297,                    /* "bytes"  */
  YYSYMBOL_AVGPKT = 298,                   /* "avgpkt"  */
  YYSYMBOL_LAST = 299,                     /* "last"  */
  YYSYMBOL_NEVER = 300,                    /* "never"  */
  YYSYMBOL_COUNTERS = 301,                 /* "counters"  */
  YYSYMBOL_QUOTAS = 302,                   /* "quotas"  */
  YYSYMBOL_LIMITS = 303,                   /* "limits"  */
  YYSYMBOL_SYNPROXYS = 304,                /* "synproxys"  */
  YYSYMBOL_HELPERS = 305,                  /* "helpers"  */
  YYSYMBOL_LOG = 306,                      /* "log"  */
  YYSYMBOL_PREFIX = 307,                   /* "prefix"  */
  YYSYMBOL_GROUP = 308,                    /* "group"  */
  YYSYMBOL_SNAPLEN = 309,                  /* "snaplen"  */
  YYSYMBOL_QUEUE_THRESHOLD = 310,          /* "queue-threshold"  */
  YYSYMBOL_LEVEL = 311,                    /* "level"  */
  YYSYMBOL_LIMIT = 312,                    /* "limit"  */
  YYSYMBOL_RATE = 313,                     /* "rate"  */
  YYSYMBOL_BURST = 314,                    /* "burst"  */
  YYSYMBOL_OVER = 315,                     /* "over"  */
  YYSYMBOL_UNTIL = 316,                    /* "until"  */
  YYSYMBOL_QUOTA = 317,                    /* "quota"  */
  YYSYMBOL_USED = 318,                     /* "used"  */
  YYSYMBOL_SECMARK = 319,                  /* "secmark"  */
  YYSYMBOL_SECMARKS = 320,                 /* "secmarks"  */
  YYSYMBOL_SECOND = 321,                   /* "second"  */
  YYSYMBOL_MINUTE = 322,                   /* "minute"  */
  YYSYMBOL_HOUR = 323,                     /* "hour"  */
  YYSYMBOL_DAY = 324,                      /* "day"  */
  YYSYMBOL_WEEK = 325,                     /* "week"  */
  YYSYMBOL__REJECT = 326,                  /* "reject"  */
  YYSYMBOL_WITH = 327,                     /* "with"  */
  YYSYMBOL_ICMPX = 328,                    /* "icmpx"  */
  YYSYMBOL_SNAT = 329,                     /* "snat"  */
  YYSYMBOL_DNAT = 330,                     /* "dnat"  */
  YYSYMBOL_MASQUERADE = 331,               /* "masquerade"  */
  YYSYMBOL_REDIRECT = 332,                 /* "redirect"  */
  YYSYMBOL_RANDOM = 333,                   /* "random"  */
  YYSYMBOL_FULLY_RANDOM = 334,             /* "fully-random"  */
  YYSYMBOL_PERSISTENT = 335,               /* "persistent"  */
  YYSYMBOL_QUEUE = 336,                    /* "queue"  */
  YYSYMBOL_QUEUENUM = 337,                 /* "num"  */
  YYSYMBOL_BYPASS = 338,                   /* "bypass"  */
  YYSYMBOL_FANOUT = 339,                   /* "fanout"  */
  YYSYMBOL_DUP = 340,                      /* "dup"  */
  YYSYMBOL_FWD = 341,                      /* "fwd"  */
  YYSYMBOL_NUMGEN = 342,                   /* "numgen"  */
  YYSYMBOL_INC = 343,                      /* "inc"  */
  YYSYMBOL_MOD = 344,                      /* "mod"  */
  YYSYMBOL_OFFSET = 345,                   /* "offset"  */
  YYSYMBOL_JHASH = 346,                    /* "jhash"  */
  YYSYMBOL_SYMHASH = 347,                  /* "symhash"  */
  YYSYMBOL_SEED = 348,                     /* "seed"  */
  YYSYMBOL_POSITION = 349,                 /* "position"  */
  YYSYMBOL_INDEX = 350,                    /* "index"  */
  YYSYMBOL_COMMENT = 351,                  /* "comment"  */
  YYSYMBOL_XML = 352,                      /* "xml"  */
  YYSYMBOL_JSON = 353,                     /* "json"  */
  YYSYMBOL_VM = 354,                       /* "vm"  */
  YYSYMBOL_NOTRACK = 355,                  /* "notrack"  */
  YYSYMBOL_EXISTS = 356,                   /* "exists"  */
  YYSYMBOL_MISSING = 357,                  /* "missing"  */
  YYSYMBOL_EXTHDR = 358,                   /* "exthdr"  */
  YYSYMBOL_IPSEC = 359,                    /* "ipsec"  */
  YYSYMBOL_REQID = 360,                    /* "reqid"  */
  YYSYMBOL_SPNUM = 361,                    /* "spnum"  */
  YYSYMBOL_IN = 362,                       /* "in"  */
  YYSYMBOL_OUT = 363,                      /* "out"  */
  YYSYMBOL_XT = 364,                       /* "xt"  */
  YYSYMBOL_365_ = 365,                     /* '='  */
  YYSYMBOL_366_ = 366,                     /* '{'  */
  YYSYMBOL_367_ = 367,                     /* '}'  */
  YYSYMBOL_368_ = 368,                     /* '('  */
  YYSYMBOL_369_ = 369,                     /* ')'  */
  YYSYMBOL_370_ = 370,                     /* '|'  */
  YYSYMBOL_371_ = 371,                     /* '$'  */
  YYSYMBOL_372_ = 372,                     /* '['  */
  YYSYMBOL_373_ = 373,                     /* ']'  */
  YYSYMBOL_YYACCEPT = 374,                 /* $accept  */
  YYSYMBOL_input = 375,                    /* input  */
  YYSYMBOL_stmt_separator = 376,           /* stmt_separator  */
  YYSYMBOL_opt_newline = 377,              /* opt_newline  */
  YYSYMBOL_close_scope_ah = 378,           /* close_scope_ah  */
  YYSYMBOL_close_scope_arp = 379,          /* close_scope_arp  */
  YYSYMBOL_close_scope_at = 380,           /* close_scope_at  */
  YYSYMBOL_close_scope_comp = 381,         /* close_scope_comp  */
  YYSYMBOL_close_scope_ct = 382,           /* close_scope_ct  */
  YYSYMBOL_close_scope_counter = 383,      /* close_scope_counter  */
  YYSYMBOL_close_scope_last = 384,         /* close_scope_last  */
  YYSYMBOL_close_scope_dccp = 385,         /* close_scope_dccp  */
  YYSYMBOL_close_scope_destroy = 386,      /* close_scope_destroy  */
  YYSYMBOL_close_scope_dst = 387,          /* close_scope_dst  */
  YYSYMBOL_close_scope_dup = 388,          /* close_scope_dup  */
  YYSYMBOL_close_scope_esp = 389,          /* close_scope_esp  */
  YYSYMBOL_close_scope_eth = 390,          /* close_scope_eth  */
  YYSYMBOL_close_scope_export = 391,       /* close_scope_export  */
  YYSYMBOL_close_scope_fib = 392,          /* close_scope_fib  */
  YYSYMBOL_close_scope_frag = 393,         /* close_scope_frag  */
  YYSYMBOL_close_scope_fwd = 394,          /* close_scope_fwd  */
  YYSYMBOL_close_scope_gre = 395,          /* close_scope_gre  */
  YYSYMBOL_close_scope_hash = 396,         /* close_scope_hash  */
  YYSYMBOL_close_scope_hbh = 397,          /* close_scope_hbh  */
  YYSYMBOL_close_scope_ip = 398,           /* close_scope_ip  */
  YYSYMBOL_close_scope_ip6 = 399,          /* close_scope_ip6  */
  YYSYMBOL_close_scope_vlan = 400,         /* close_scope_vlan  */
  YYSYMBOL_close_scope_icmp = 401,         /* close_scope_icmp  */
  YYSYMBOL_close_scope_igmp = 402,         /* close_scope_igmp  */
  YYSYMBOL_close_scope_import = 403,       /* close_scope_import  */
  YYSYMBOL_close_scope_ipsec = 404,        /* close_scope_ipsec  */
  YYSYMBOL_close_scope_list = 405,         /* close_scope_list  */
  YYSYMBOL_close_scope_limit = 406,        /* close_scope_limit  */
  YYSYMBOL_close_scope_meta = 407,         /* close_scope_meta  */
  YYSYMBOL_close_scope_mh = 408,           /* close_scope_mh  */
  YYSYMBOL_close_scope_monitor = 409,      /* close_scope_monitor  */
  YYSYMBOL_close_scope_nat = 410,          /* close_scope_nat  */
  YYSYMBOL_close_scope_numgen = 411,       /* close_scope_numgen  */
  YYSYMBOL_close_scope_osf = 412,          /* close_scope_osf  */
  YYSYMBOL_close_scope_policy = 413,       /* close_scope_policy  */
  YYSYMBOL_close_scope_quota = 414,        /* close_scope_quota  */
  YYSYMBOL_close_scope_queue = 415,        /* close_scope_queue  */
  YYSYMBOL_close_scope_reject = 416,       /* close_scope_reject  */
  YYSYMBOL_close_scope_reset = 417,        /* close_scope_reset  */
  YYSYMBOL_close_scope_rt = 418,           /* close_scope_rt  */
  YYSYMBOL_close_scope_sctp = 419,         /* close_scope_sctp  */
  YYSYMBOL_close_scope_sctp_chunk = 420,   /* close_scope_sctp_chunk  */
  YYSYMBOL_close_scope_secmark = 421,      /* close_scope_secmark  */
  YYSYMBOL_close_scope_socket = 422,       /* close_scope_socket  */
  YYSYMBOL_close_scope_tcp = 423,          /* close_scope_tcp  */
  YYSYMBOL_close_scope_tproxy = 424,       /* close_scope_tproxy  */
  YYSYMBOL_close_scope_type = 425,         /* close_scope_type  */
  YYSYMBOL_close_scope_th = 426,           /* close_scope_th  */
  YYSYMBOL_close_scope_udp = 427,          /* close_scope_udp  */
  YYSYMBOL_close_scope_udplite = 428,      /* close_scope_udplite  */
  YYSYMBOL_close_scope_log = 429,          /* close_scope_log  */
  YYSYMBOL_close_scope_synproxy = 430,     /* close_scope_synproxy  */
  YYSYMBOL_close_scope_xt = 431,           /* close_scope_xt  */
  YYSYMBOL_common_block = 432,             /* common_block  */
  YYSYMBOL_line = 433,                     /* line  */
  YYSYMBOL_base_cmd = 434,                 /* base_cmd  */
  YYSYMBOL_add_cmd = 435,                  /* add_cmd  */
  YYSYMBOL_replace_cmd = 436,              /* replace_cmd  */
  YYSYMBOL_create_cmd = 437,               /* create_cmd  */
  YYSYMBOL_insert_cmd = 438,               /* insert_cmd  */
  YYSYMBOL_table_or_id_spec = 439,         /* table_or_id_spec  */
  YYSYMBOL_chain_or_id_spec = 440,         /* chain_or_id_spec  */
  YYSYMBOL_set_or_id_spec = 441,           /* set_or_id_spec  */
  YYSYMBOL_obj_or_id_spec = 442,           /* obj_or_id_spec  */
  YYSYMBOL_delete_cmd = 443,               /* delete_cmd  */
  YYSYMBOL_destroy_cmd = 444,              /* destroy_cmd  */
  YYSYMBOL_get_cmd = 445,                  /* get_cmd  */
  YYSYMBOL_list_cmd_spec_table = 446,      /* list_cmd_spec_table  */
  YYSYMBOL_list_cmd_spec_any = 447,        /* list_cmd_spec_any  */
  YYSYMBOL_list_cmd = 448,                 /* list_cmd  */
  YYSYMBOL_basehook_device_name = 449,     /* basehook_device_name  */
  YYSYMBOL_basehook_spec = 450,            /* basehook_spec  */
  YYSYMBOL_reset_cmd = 451,                /* reset_cmd  */
  YYSYMBOL_flush_cmd = 452,                /* flush_cmd  */
  YYSYMBOL_rename_cmd = 453,               /* rename_cmd  */
  YYSYMBOL_import_cmd = 454,               /* import_cmd  */
  YYSYMBOL_export_cmd = 455,               /* export_cmd  */
  YYSYMBOL_monitor_cmd = 456,              /* monitor_cmd  */
  YYSYMBOL_monitor_event = 457,            /* monitor_event  */
  YYSYMBOL_monitor_object = 458,           /* monitor_object  */
  YYSYMBOL_monitor_format = 459,           /* monitor_format  */
  YYSYMBOL_markup_format = 460,            /* markup_format  */
  YYSYMBOL_describe_cmd = 461,             /* describe_cmd  */
  YYSYMBOL_table_block_alloc = 462,        /* table_block_alloc  */
  YYSYMBOL_table_options = 463,            /* table_options  */
  YYSYMBOL_table_flags = 464,              /* table_flags  */
  YYSYMBOL_table_flag = 465,               /* table_flag  */
  YYSYMBOL_table_block = 466,              /* table_block  */
  YYSYMBOL_chain_block_alloc = 467,        /* chain_block_alloc  */
  YYSYMBOL_chain_block = 468,              /* chain_block  */
  YYSYMBOL_subchain_block = 469,           /* subchain_block  */
  YYSYMBOL_typeof_verdict_expr = 470,      /* typeof_verdict_expr  */
  YYSYMBOL_typeof_data_expr = 471,         /* typeof_data_expr  */
  YYSYMBOL_primary_typeof_expr = 472,      /* primary_typeof_expr  */
  YYSYMBOL_typeof_expr = 473,              /* typeof_expr  */
  YYSYMBOL_set_block_alloc = 474,          /* set_block_alloc  */
  YYSYMBOL_typeof_key_expr = 475,          /* typeof_key_expr  */
  YYSYMBOL_set_block = 476,                /* set_block  */
  YYSYMBOL_set_block_expr = 477,           /* set_block_expr  */
  YYSYMBOL_set_flag_list = 478,            /* set_flag_list  */
  YYSYMBOL_set_flag = 479,                 /* set_flag  */
  YYSYMBOL_map_block_alloc = 480,          /* map_block_alloc  */
  YYSYMBOL_ct_obj_type_map = 481,          /* ct_obj_type_map  */
  YYSYMBOL_map_block_obj_type = 482,       /* map_block_obj_type  */
  YYSYMBOL_map_block_obj_typeof = 483,     /* map_block_obj_typeof  */
  YYSYMBOL_map_block_data_interval = 484,  /* map_block_data_interval  */
  YYSYMBOL_map_block = 485,                /* map_block  */
  YYSYMBOL_set_mechanism = 486,            /* set_mechanism  */
  YYSYMBOL_set_policy_spec = 487,          /* set_policy_spec  */
  YYSYMBOL_flowtable_block_alloc = 488,    /* flowtable_block_alloc  */
  YYSYMBOL_flowtable_block = 489,          /* flowtable_block  */
  YYSYMBOL_flowtable_expr = 490,           /* flowtable_expr  */
  YYSYMBOL_flowtable_list_expr = 491,      /* flowtable_list_expr  */
  YYSYMBOL_flowtable_expr_member = 492,    /* flowtable_expr_member  */
  YYSYMBOL_data_type_atom_expr = 493,      /* data_type_atom_expr  */
  YYSYMBOL_data_type_expr = 494,           /* data_type_expr  */
  YYSYMBOL_obj_block_alloc = 495,          /* obj_block_alloc  */
  YYSYMBOL_counter_block = 496,            /* counter_block  */
  YYSYMBOL_quota_block = 497,              /* quota_block  */
  YYSYMBOL_ct_helper_block = 498,          /* ct_helper_block  */
  YYSYMBOL_ct_timeout_block = 499,         /* ct_timeout_block  */
  YYSYMBOL_ct_expect_block = 500,          /* ct_expect_block  */
  YYSYMBOL_limit_block = 501,              /* limit_block  */
  YYSYMBOL_secmark_block = 502,            /* secmark_block  */
  YYSYMBOL_synproxy_block = 503,           /* synproxy_block  */
  YYSYMBOL_type_identifier = 504,          /* type_identifier  */
  YYSYMBOL_hook_spec = 505,                /* hook_spec  */
  YYSYMBOL_prio_spec = 506,                /* prio_spec  */
  YYSYMBOL_extended_prio_name = 507,       /* extended_prio_name  */
  YYSYMBOL_extended_prio_spec = 508,       /* extended_prio_spec  */
  YYSYMBOL_int_num = 509,                  /* int_num  */
  YYSYMBOL_dev_spec = 510,                 /* dev_spec  */
  YYSYMBOL_flags_spec = 511,               /* flags_spec  */
  YYSYMBOL_policy_spec = 512,              /* policy_spec  */
  YYSYMBOL_policy_expr = 513,              /* policy_expr  */
  YYSYMBOL_chain_policy = 514,             /* chain_policy  */
  YYSYMBOL_identifier = 515,               /* identifier  */
  YYSYMBOL_string = 516,                   /* string  */
  YYSYMBOL_time_spec = 517,                /* time_spec  */
  YYSYMBOL_time_spec_or_num_s = 518,       /* time_spec_or_num_s  */
  YYSYMBOL_family_spec = 519,              /* family_spec  */
  YYSYMBOL_family_spec_explicit = 520,     /* family_spec_explicit  */
  YYSYMBOL_table_spec = 521,               /* table_spec  */
  YYSYMBOL_tableid_spec = 522,             /* tableid_spec  */
  YYSYMBOL_chain_spec = 523,               /* chain_spec  */
  YYSYMBOL_chainid_spec = 524,             /* chainid_spec  */
  YYSYMBOL_chain_identifier = 525,         /* chain_identifier  */
  YYSYMBOL_set_spec = 526,                 /* set_spec  */
  YYSYMBOL_setid_spec = 527,               /* setid_spec  */
  YYSYMBOL_set_identifier = 528,           /* set_identifier  */
  YYSYMBOL_flowtable_spec = 529,           /* flowtable_spec  */
  YYSYMBOL_flowtableid_spec = 530,         /* flowtableid_spec  */
  YYSYMBOL_flowtable_identifier = 531,     /* flowtable_identifier  */
  YYSYMBOL_obj_spec = 532,                 /* obj_spec  */
  YYSYMBOL_objid_spec = 533,               /* objid_spec  */
  YYSYMBOL_obj_identifier = 534,           /* obj_identifier  */
  YYSYMBOL_handle_spec = 535,              /* handle_spec  */
  YYSYMBOL_position_spec = 536,            /* position_spec  */
  YYSYMBOL_index_spec = 537,               /* index_spec  */
  YYSYMBOL_rule_position = 538,            /* rule_position  */
  YYSYMBOL_ruleid_spec = 539,              /* ruleid_spec  */
  YYSYMBOL_comment_spec = 540,             /* comment_spec  */
  YYSYMBOL_ruleset_spec = 541,             /* ruleset_spec  */
  YYSYMBOL_rule = 542,                     /* rule  */
  YYSYMBOL_rule_alloc = 543,               /* rule_alloc  */
  YYSYMBOL_stmt_list = 544,                /* stmt_list  */
  YYSYMBOL_stateful_stmt_list = 545,       /* stateful_stmt_list  */
  YYSYMBOL_objref_stmt_counter = 546,      /* objref_stmt_counter  */
  YYSYMBOL_objref_stmt_limit = 547,        /* objref_stmt_limit  */
  YYSYMBOL_objref_stmt_quota = 548,        /* objref_stmt_quota  */
  YYSYMBOL_objref_stmt_synproxy = 549,     /* objref_stmt_synproxy  */
  YYSYMBOL_objref_stmt_ct = 550,           /* objref_stmt_ct  */
  YYSYMBOL_objref_stmt = 551,              /* objref_stmt  */
  YYSYMBOL_stateful_stmt = 552,            /* stateful_stmt  */
  YYSYMBOL_stmt = 553,                     /* stmt  */
  YYSYMBOL_xt_stmt = 554,                  /* xt_stmt  */
  YYSYMBOL_chain_stmt_type = 555,          /* chain_stmt_type  */
  YYSYMBOL_chain_stmt = 556,               /* chain_stmt  */
  YYSYMBOL_verdict_stmt = 557,             /* verdict_stmt  */
  YYSYMBOL_verdict_map_stmt = 558,         /* verdict_map_stmt  */
  YYSYMBOL_verdict_map_expr = 559,         /* verdict_map_expr  */
  YYSYMBOL_verdict_map_list_expr = 560,    /* verdict_map_list_expr  */
  YYSYMBOL_verdict_map_list_member_expr = 561, /* verdict_map_list_member_expr  */
  YYSYMBOL_ct_limit_stmt_alloc = 562,      /* ct_limit_stmt_alloc  */
  YYSYMBOL_connlimit_stmt = 563,           /* connlimit_stmt  */
  YYSYMBOL_ct_limit_args = 564,            /* ct_limit_args  */
  YYSYMBOL_counter_stmt = 565,             /* counter_stmt  */
  YYSYMBOL_counter_stmt_alloc = 566,       /* counter_stmt_alloc  */
  YYSYMBOL_counter_args = 567,             /* counter_args  */
  YYSYMBOL_counter_arg = 568,              /* counter_arg  */
  YYSYMBOL_last_stmt_alloc = 569,          /* last_stmt_alloc  */
  YYSYMBOL_last_stmt = 570,                /* last_stmt  */
  YYSYMBOL_last_args = 571,                /* last_args  */
  YYSYMBOL_log_stmt = 572,                 /* log_stmt  */
  YYSYMBOL_log_stmt_alloc = 573,           /* log_stmt_alloc  */
  YYSYMBOL_log_args = 574,                 /* log_args  */
  YYSYMBOL_log_arg = 575,                  /* log_arg  */
  YYSYMBOL_level_type = 576,               /* level_type  */
  YYSYMBOL_log_flags = 577,                /* log_flags  */
  YYSYMBOL_log_flags_tcp = 578,            /* log_flags_tcp  */
  YYSYMBOL_log_flag_tcp = 579,             /* log_flag_tcp  */
  YYSYMBOL_limit_stmt_alloc = 580,         /* limit_stmt_alloc  */
  YYSYMBOL_limit_stmt = 581,               /* limit_stmt  */
  YYSYMBOL_limit_args = 582,               /* limit_args  */
  YYSYMBOL_quota_mode = 583,               /* quota_mode  */
  YYSYMBOL_quota_unit = 584,               /* quota_unit  */
  YYSYMBOL_quota_used = 585,               /* quota_used  */
  YYSYMBOL_quota_stmt_alloc = 586,         /* quota_stmt_alloc  */
  YYSYMBOL_quota_stmt = 587,               /* quota_stmt  */
  YYSYMBOL_quota_args = 588,               /* quota_args  */
  YYSYMBOL_limit_mode = 589,               /* limit_mode  */
  YYSYMBOL_limit_burst_pkts = 590,         /* limit_burst_pkts  */
  YYSYMBOL_limit_rate_pkts = 591,          /* limit_rate_pkts  */
  YYSYMBOL_limit_burst_bytes = 592,        /* limit_burst_bytes  */
  YYSYMBOL_limit_rate_bytes = 593,         /* limit_rate_bytes  */
  YYSYMBOL_limit_bytes = 594,              /* limit_bytes  */
  YYSYMBOL_time_unit = 595,                /* time_unit  */
  YYSYMBOL_reject_stmt = 596,              /* reject_stmt  */
  YYSYMBOL_reject_stmt_alloc = 597,        /* reject_stmt_alloc  */
  YYSYMBOL_reject_with_expr = 598,         /* reject_with_expr  */
  YYSYMBOL_reject_opts = 599,              /* reject_opts  */
  YYSYMBOL_nat_stmt = 600,                 /* nat_stmt  */
  YYSYMBOL_nat_stmt_alloc = 601,           /* nat_stmt_alloc  */
  YYSYMBOL_tproxy_stmt = 602,              /* tproxy_stmt  */
  YYSYMBOL_synproxy_stmt = 603,            /* synproxy_stmt  */
  YYSYMBOL_synproxy_stmt_alloc = 604,      /* synproxy_stmt_alloc  */
  YYSYMBOL_synproxy_args = 605,            /* synproxy_args  */
  YYSYMBOL_synproxy_arg = 606,             /* synproxy_arg  */
  YYSYMBOL_synproxy_config = 607,          /* synproxy_config  */
  YYSYMBOL_synproxy_obj = 608,             /* synproxy_obj  */
  YYSYMBOL_synproxy_ts = 609,              /* synproxy_ts  */
  YYSYMBOL_synproxy_sack = 610,            /* synproxy_sack  */
  YYSYMBOL_primary_stmt_expr = 611,        /* primary_stmt_expr  */
  YYSYMBOL_shift_stmt_expr = 612,          /* shift_stmt_expr  */
  YYSYMBOL_and_stmt_expr = 613,            /* and_stmt_expr  */
  YYSYMBOL_exclusive_or_stmt_expr = 614,   /* exclusive_or_stmt_expr  */
  YYSYMBOL_inclusive_or_stmt_expr = 615,   /* inclusive_or_stmt_expr  */
  YYSYMBOL_basic_stmt_expr = 616,          /* basic_stmt_expr  */
  YYSYMBOL_concat_stmt_expr = 617,         /* concat_stmt_expr  */
  YYSYMBOL_map_stmt_expr_set = 618,        /* map_stmt_expr_set  */
  YYSYMBOL_map_stmt_expr = 619,            /* map_stmt_expr  */
  YYSYMBOL_prefix_stmt_expr = 620,         /* prefix_stmt_expr  */
  YYSYMBOL_range_stmt_expr = 621,          /* range_stmt_expr  */
  YYSYMBOL_multiton_stmt_expr = 622,       /* multiton_stmt_expr  */
  YYSYMBOL_stmt_expr = 623,                /* stmt_expr  */
  YYSYMBOL_nat_stmt_args = 624,            /* nat_stmt_args  */
  YYSYMBOL_masq_stmt = 625,                /* masq_stmt  */
  YYSYMBOL_masq_stmt_alloc = 626,          /* masq_stmt_alloc  */
  YYSYMBOL_masq_stmt_args = 627,           /* masq_stmt_args  */
  YYSYMBOL_redir_stmt = 628,               /* redir_stmt  */
  YYSYMBOL_redir_stmt_alloc = 629,         /* redir_stmt_alloc  */
  YYSYMBOL_redir_stmt_arg = 630,           /* redir_stmt_arg  */
  YYSYMBOL_dup_stmt = 631,                 /* dup_stmt  */
  YYSYMBOL_fwd_stmt = 632,                 /* fwd_stmt  */
  YYSYMBOL_nf_nat_flags = 633,             /* nf_nat_flags  */
  YYSYMBOL_nf_nat_flag = 634,              /* nf_nat_flag  */
  YYSYMBOL_queue_stmt = 635,               /* queue_stmt  */
  YYSYMBOL_queue_stmt_compat = 636,        /* queue_stmt_compat  */
  YYSYMBOL_queue_stmt_alloc = 637,         /* queue_stmt_alloc  */
  YYSYMBOL_queue_stmt_args = 638,          /* queue_stmt_args  */
  YYSYMBOL_queue_stmt_arg = 639,           /* queue_stmt_arg  */
  YYSYMBOL_queue_expr = 640,               /* queue_expr  */
  YYSYMBOL_queue_stmt_expr_simple = 641,   /* queue_stmt_expr_simple  */
  YYSYMBOL_queue_stmt_expr = 642,          /* queue_stmt_expr  */
  YYSYMBOL_queue_stmt_flags = 643,         /* queue_stmt_flags  */
  YYSYMBOL_queue_stmt_flag = 644,          /* queue_stmt_flag  */
  YYSYMBOL_set_elem_expr_stmt = 645,       /* set_elem_expr_stmt  */
  YYSYMBOL_set_elem_expr_stmt_alloc = 646, /* set_elem_expr_stmt_alloc  */
  YYSYMBOL_set_stmt = 647,                 /* set_stmt  */
  YYSYMBOL_set_stmt_op = 648,              /* set_stmt_op  */
  YYSYMBOL_map_stmt = 649,                 /* map_stmt  */
  YYSYMBOL_meter_stmt = 650,               /* meter_stmt  */
  YYSYMBOL_match_stmt = 651,               /* match_stmt  */
  YYSYMBOL_variable_expr = 652,            /* variable_expr  */
  YYSYMBOL_symbol_expr = 653,              /* symbol_expr  */
  YYSYMBOL_set_ref_expr = 654,             /* set_ref_expr  */
  YYSYMBOL_set_ref_symbol_expr = 655,      /* set_ref_symbol_expr  */
  YYSYMBOL_integer_expr = 656,             /* integer_expr  */
  YYSYMBOL_selector_expr = 657,            /* selector_expr  */
  YYSYMBOL_primary_expr = 658,             /* primary_expr  */
  YYSYMBOL_fib_expr = 659,                 /* fib_expr  */
  YYSYMBOL_fib_result = 660,               /* fib_result  */
  YYSYMBOL_fib_flag = 661,                 /* fib_flag  */
  YYSYMBOL_fib_tuple = 662,                /* fib_tuple  */
  YYSYMBOL_osf_expr = 663,                 /* osf_expr  */
  YYSYMBOL_osf_ttl = 664,                  /* osf_ttl  */
  YYSYMBOL_shift_expr = 665,               /* shift_expr  */
  YYSYMBOL_and_expr = 666,                 /* and_expr  */
  YYSYMBOL_exclusive_or_expr = 667,        /* exclusive_or_expr  */
  YYSYMBOL_inclusive_or_expr = 668,        /* inclusive_or_expr  */
  YYSYMBOL_basic_expr = 669,               /* basic_expr  */
  YYSYMBOL_concat_expr = 670,              /* concat_expr  */
  YYSYMBOL_prefix_rhs_expr = 671,          /* prefix_rhs_expr  */
  YYSYMBOL_range_rhs_expr = 672,           /* range_rhs_expr  */
  YYSYMBOL_multiton_rhs_expr = 673,        /* multiton_rhs_expr  */
  YYSYMBOL_map_expr = 674,                 /* map_expr  */
  YYSYMBOL_expr = 675,                     /* expr  */
  YYSYMBOL_set_expr = 676,                 /* set_expr  */
  YYSYMBOL_set_list_expr = 677,            /* set_list_expr  */
  YYSYMBOL_set_list_member_expr = 678,     /* set_list_member_expr  */
  YYSYMBOL_meter_key_expr = 679,           /* meter_key_expr  */
  YYSYMBOL_meter_key_expr_alloc = 680,     /* meter_key_expr_alloc  */
  YYSYMBOL_set_elem_expr = 681,            /* set_elem_expr  */
  YYSYMBOL_set_elem_key_expr = 682,        /* set_elem_key_expr  */
  YYSYMBOL_set_elem_expr_alloc = 683,      /* set_elem_expr_alloc  */
  YYSYMBOL_set_elem_options = 684,         /* set_elem_options  */
  YYSYMBOL_set_elem_time_spec = 685,       /* set_elem_time_spec  */
  YYSYMBOL_set_elem_option = 686,          /* set_elem_option  */
  YYSYMBOL_set_elem_expr_options = 687,    /* set_elem_expr_options  */
  YYSYMBOL_set_elem_stmt_list = 688,       /* set_elem_stmt_list  */
  YYSYMBOL_set_elem_stmt = 689,            /* set_elem_stmt  */
  YYSYMBOL_set_elem_expr_option = 690,     /* set_elem_expr_option  */
  YYSYMBOL_set_lhs_expr = 691,             /* set_lhs_expr  */
  YYSYMBOL_set_rhs_expr = 692,             /* set_rhs_expr  */
  YYSYMBOL_initializer_expr = 693,         /* initializer_expr  */
  YYSYMBOL_counter_config = 694,           /* counter_config  */
  YYSYMBOL_counter_obj = 695,              /* counter_obj  */
  YYSYMBOL_quota_config = 696,             /* quota_config  */
  YYSYMBOL_quota_obj = 697,                /* quota_obj  */
  YYSYMBOL_secmark_config = 698,           /* secmark_config  */
  YYSYMBOL_secmark_obj = 699,              /* secmark_obj  */
  YYSYMBOL_ct_obj_type = 700,              /* ct_obj_type  */
  YYSYMBOL_ct_cmd_type = 701,              /* ct_cmd_type  */
  YYSYMBOL_ct_l4protoname = 702,           /* ct_l4protoname  */
  YYSYMBOL_ct_helper_config = 703,         /* ct_helper_config  */
  YYSYMBOL_timeout_states = 704,           /* timeout_states  */
  YYSYMBOL_timeout_state = 705,            /* timeout_state  */
  YYSYMBOL_ct_timeout_config = 706,        /* ct_timeout_config  */
  YYSYMBOL_ct_expect_config = 707,         /* ct_expect_config  */
  YYSYMBOL_ct_obj_alloc = 708,             /* ct_obj_alloc  */
  YYSYMBOL_limit_config = 709,             /* limit_config  */
  YYSYMBOL_limit_obj = 710,                /* limit_obj  */
  YYSYMBOL_relational_expr = 711,          /* relational_expr  */
  YYSYMBOL_list_rhs_expr = 712,            /* list_rhs_expr  */
  YYSYMBOL_rhs_expr = 713,                 /* rhs_expr  */
  YYSYMBOL_shift_rhs_expr = 714,           /* shift_rhs_expr  */
  YYSYMBOL_and_rhs_expr = 715,             /* and_rhs_expr  */
  YYSYMBOL_exclusive_or_rhs_expr = 716,    /* exclusive_or_rhs_expr  */
  YYSYMBOL_inclusive_or_rhs_expr = 717,    /* inclusive_or_rhs_expr  */
  YYSYMBOL_basic_rhs_expr = 718,           /* basic_rhs_expr  */
  YYSYMBOL_concat_rhs_expr = 719,          /* concat_rhs_expr  */
  YYSYMBOL_boolean_keys = 720,             /* boolean_keys  */
  YYSYMBOL_boolean_expr = 721,             /* boolean_expr  */
  YYSYMBOL_keyword_expr = 722,             /* keyword_expr  */
  YYSYMBOL_primary_rhs_expr = 723,         /* primary_rhs_expr  */
  YYSYMBOL_relational_op = 724,            /* relational_op  */
  YYSYMBOL_verdict_expr = 725,             /* verdict_expr  */
  YYSYMBOL_chain_expr = 726,               /* chain_expr  */
  YYSYMBOL_meta_expr = 727,                /* meta_expr  */
  YYSYMBOL_meta_key = 728,                 /* meta_key  */
  YYSYMBOL_meta_key_qualified = 729,       /* meta_key_qualified  */
  YYSYMBOL_meta_key_unqualified = 730,     /* meta_key_unqualified  */
  YYSYMBOL_meta_stmt = 731,                /* meta_stmt  */
  YYSYMBOL_socket_expr = 732,              /* socket_expr  */
  YYSYMBOL_socket_key = 733,               /* socket_key  */
  YYSYMBOL_offset_opt = 734,               /* offset_opt  */
  YYSYMBOL_numgen_type = 735,              /* numgen_type  */
  YYSYMBOL_numgen_expr = 736,              /* numgen_expr  */
  YYSYMBOL_xfrm_spnum = 737,               /* xfrm_spnum  */
  YYSYMBOL_xfrm_dir = 738,                 /* xfrm_dir  */
  YYSYMBOL_xfrm_state_key = 739,           /* xfrm_state_key  */
  YYSYMBOL_xfrm_state_proto_key = 740,     /* xfrm_state_proto_key  */
  YYSYMBOL_xfrm_expr = 741,                /* xfrm_expr  */
  YYSYMBOL_hash_expr = 742,                /* hash_expr  */
  YYSYMBOL_nf_key_proto = 743,             /* nf_key_proto  */
  YYSYMBOL_rt_expr = 744,                  /* rt_expr  */
  YYSYMBOL_rt_key = 745,                   /* rt_key  */
  YYSYMBOL_ct_expr = 746,                  /* ct_expr  */
  YYSYMBOL_ct_dir = 747,                   /* ct_dir  */
  YYSYMBOL_ct_key = 748,                   /* ct_key  */
  YYSYMBOL_ct_key_dir = 749,               /* ct_key_dir  */
  YYSYMBOL_ct_key_proto_field = 750,       /* ct_key_proto_field  */
  YYSYMBOL_ct_key_dir_optional = 751,      /* ct_key_dir_optional  */
  YYSYMBOL_symbol_stmt_expr = 752,         /* symbol_stmt_expr  */
  YYSYMBOL_list_stmt_expr = 753,           /* list_stmt_expr  */
  YYSYMBOL_ct_stmt = 754,                  /* ct_stmt  */
  YYSYMBOL_payload_stmt = 755,             /* payload_stmt  */
  YYSYMBOL_payload_expr = 756,             /* payload_expr  */
  YYSYMBOL_payload_raw_len = 757,          /* payload_raw_len  */
  YYSYMBOL_payload_raw_expr = 758,         /* payload_raw_expr  */
  YYSYMBOL_payload_base_spec = 759,        /* payload_base_spec  */
  YYSYMBOL_eth_hdr_expr = 760,             /* eth_hdr_expr  */
  YYSYMBOL_eth_hdr_field = 761,            /* eth_hdr_field  */
  YYSYMBOL_vlan_hdr_expr = 762,            /* vlan_hdr_expr  */
  YYSYMBOL_vlan_hdr_field = 763,           /* vlan_hdr_field  */
  YYSYMBOL_arp_hdr_expr = 764,             /* arp_hdr_expr  */
  YYSYMBOL_arp_hdr_field = 765,            /* arp_hdr_field  */
  YYSYMBOL_ip_hdr_expr = 766,              /* ip_hdr_expr  */
  YYSYMBOL_ip_hdr_field = 767,             /* ip_hdr_field  */
  YYSYMBOL_ip_option_type = 768,           /* ip_option_type  */
  YYSYMBOL_ip_option_field = 769,          /* ip_option_field  */
  YYSYMBOL_icmp_hdr_expr = 770,            /* icmp_hdr_expr  */
  YYSYMBOL_icmp_hdr_field = 771,           /* icmp_hdr_field  */
  YYSYMBOL_igmp_hdr_expr = 772,            /* igmp_hdr_expr  */
  YYSYMBOL_igmp_hdr_field = 773,           /* igmp_hdr_field  */
  YYSYMBOL_ip6_hdr_expr = 774,             /* ip6_hdr_expr  */
  YYSYMBOL_ip6_hdr_field = 775,            /* ip6_hdr_field  */
  YYSYMBOL_icmp6_hdr_expr = 776,           /* icmp6_hdr_expr  */
  YYSYMBOL_icmp6_hdr_field = 777,          /* icmp6_hdr_field  */
  YYSYMBOL_auth_hdr_expr = 778,            /* auth_hdr_expr  */
  YYSYMBOL_auth_hdr_field = 779,           /* auth_hdr_field  */
  YYSYMBOL_esp_hdr_expr = 780,             /* esp_hdr_expr  */
  YYSYMBOL_esp_hdr_field = 781,            /* esp_hdr_field  */
  YYSYMBOL_comp_hdr_expr = 782,            /* comp_hdr_expr  */
  YYSYMBOL_comp_hdr_field = 783,           /* comp_hdr_field  */
  YYSYMBOL_udp_hdr_expr = 784,             /* udp_hdr_expr  */
  YYSYMBOL_udp_hdr_field = 785,            /* udp_hdr_field  */
  YYSYMBOL_udplite_hdr_expr = 786,         /* udplite_hdr_expr  */
  YYSYMBOL_udplite_hdr_field = 787,        /* udplite_hdr_field  */
  YYSYMBOL_tcp_hdr_expr = 788,             /* tcp_hdr_expr  */
  YYSYMBOL_inner_inet_expr = 789,          /* inner_inet_expr  */
  YYSYMBOL_inner_eth_expr = 790,           /* inner_eth_expr  */
  YYSYMBOL_inner_expr = 791,               /* inner_expr  */
  YYSYMBOL_vxlan_hdr_expr = 792,           /* vxlan_hdr_expr  */
  YYSYMBOL_vxlan_hdr_field = 793,          /* vxlan_hdr_field  */
  YYSYMBOL_geneve_hdr_expr = 794,          /* geneve_hdr_expr  */
  YYSYMBOL_geneve_hdr_field = 795,         /* geneve_hdr_field  */
  YYSYMBOL_gre_hdr_expr = 796,             /* gre_hdr_expr  */
  YYSYMBOL_gre_hdr_field = 797,            /* gre_hdr_field  */
  YYSYMBOL_gretap_hdr_expr = 798,          /* gretap_hdr_expr  */
  YYSYMBOL_optstrip_stmt = 799,            /* optstrip_stmt  */
  YYSYMBOL_tcp_hdr_field = 800,            /* tcp_hdr_field  */
  YYSYMBOL_tcp_hdr_option_kind_and_field = 801, /* tcp_hdr_option_kind_and_field  */
  YYSYMBOL_tcp_hdr_option_sack = 802,      /* tcp_hdr_option_sack  */
  YYSYMBOL_tcp_hdr_option_type = 803,      /* tcp_hdr_option_type  */
  YYSYMBOL_tcpopt_field_sack = 804,        /* tcpopt_field_sack  */
  YYSYMBOL_tcpopt_field_window = 805,      /* tcpopt_field_window  */
  YYSYMBOL_tcpopt_field_tsopt = 806,       /* tcpopt_field_tsopt  */
  YYSYMBOL_tcpopt_field_maxseg = 807,      /* tcpopt_field_maxseg  */
  YYSYMBOL_tcpopt_field_mptcp = 808,       /* tcpopt_field_mptcp  */
  YYSYMBOL_dccp_hdr_expr = 809,            /* dccp_hdr_expr  */
  YYSYMBOL_dccp_hdr_field = 810,           /* dccp_hdr_field  */
  YYSYMBOL_sctp_chunk_type = 811,          /* sctp_chunk_type  */
  YYSYMBOL_sctp_chunk_common_field = 812,  /* sctp_chunk_common_field  */
  YYSYMBOL_sctp_chunk_data_field = 813,    /* sctp_chunk_data_field  */
  YYSYMBOL_sctp_chunk_init_field = 814,    /* sctp_chunk_init_field  */
  YYSYMBOL_sctp_chunk_sack_field = 815,    /* sctp_chunk_sack_field  */
  YYSYMBOL_sctp_chunk_alloc = 816,         /* sctp_chunk_alloc  */
  YYSYMBOL_sctp_hdr_expr = 817,            /* sctp_hdr_expr  */
  YYSYMBOL_sctp_hdr_field = 818,           /* sctp_hdr_field  */
  YYSYMBOL_th_hdr_expr = 819,              /* th_hdr_expr  */
  YYSYMBOL_th_hdr_field = 820,             /* th_hdr_field  */
  YYSYMBOL_exthdr_expr = 821,              /* exthdr_expr  */
  YYSYMBOL_hbh_hdr_expr = 822,             /* hbh_hdr_expr  */
  YYSYMBOL_hbh_hdr_field = 823,            /* hbh_hdr_field  */
  YYSYMBOL_rt_hdr_expr = 824,              /* rt_hdr_expr  */
  YYSYMBOL_rt_hdr_field = 825,             /* rt_hdr_field  */
  YYSYMBOL_rt0_hdr_expr = 826,             /* rt0_hdr_expr  */
  YYSYMBOL_rt0_hdr_field = 827,            /* rt0_hdr_field  */
  YYSYMBOL_rt2_hdr_expr = 828,             /* rt2_hdr_expr  */
  YYSYMBOL_rt2_hdr_field = 829,            /* rt2_hdr_field  */
  YYSYMBOL_rt4_hdr_expr = 830,             /* rt4_hdr_expr  */
  YYSYMBOL_rt4_hdr_field = 831,            /* rt4_hdr_field  */
  YYSYMBOL_frag_hdr_expr = 832,            /* frag_hdr_expr  */
  YYSYMBOL_frag_hdr_field = 833,           /* frag_hdr_field  */
  YYSYMBOL_dst_hdr_expr = 834,             /* dst_hdr_expr  */
  YYSYMBOL_dst_hdr_field = 835,            /* dst_hdr_field  */
  YYSYMBOL_mh_hdr_expr = 836,              /* mh_hdr_expr  */
  YYSYMBOL_mh_hdr_field = 837,             /* mh_hdr_field  */
  YYSYMBOL_exthdr_exists_expr = 838,       /* exthdr_exists_expr  */
  YYSYMBOL_exthdr_key = 839                /* exthdr_key  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   9534

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  374
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  466
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1384
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2341

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   619


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,   371,     2,     2,     2,
     368,   369,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   365,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   372,     2,   373,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   366,   370,   367,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,  1015,  1015,  1016,  1025,  1026,  1029,  1030,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1085,
    1086,  1087,  1089,  1097,  1112,  1119,  1131,  1139,  1140,  1141,
    1142,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1180,  1184,  1191,
    1195,  1203,  1207,  1211,  1218,  1225,  1235,  1242,  1251,  1255,
    1259,  1263,  1267,  1271,  1275,  1279,  1283,  1287,  1291,  1295,
    1299,  1305,  1311,  1315,  1322,  1326,  1334,  1341,  1348,  1358,
    1365,  1374,  1378,  1382,  1386,  1390,  1394,  1398,  1402,  1408,
    1414,  1415,  1418,  1419,  1422,  1423,  1426,  1427,  1430,  1434,
    1438,  1446,  1450,  1454,  1458,  1462,  1466,  1470,  1477,  1481,
    1485,  1491,  1495,  1499,  1505,  1509,  1513,  1517,  1521,  1525,
    1529,  1533,  1537,  1544,  1548,  1552,  1558,  1562,  1566,  1573,
    1579,  1580,  1582,  1583,  1586,  1590,  1594,  1598,  1602,  1606,
    1610,  1614,  1618,  1622,  1626,  1630,  1634,  1638,  1642,  1646,
    1650,  1654,  1658,  1662,  1666,  1670,  1674,  1678,  1682,  1686,
    1690,  1694,  1700,  1706,  1710,  1720,  1724,  1728,  1732,  1736,
    1740,  1744,  1748,  1753,  1757,  1761,  1765,  1771,  1775,  1779,
    1783,  1787,  1791,  1795,  1801,  1808,  1814,  1822,  1828,  1836,
    1845,  1846,  1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,
    1859,  1860,  1863,  1864,  1865,  1868,  1877,  1887,  1891,  1901,
    1902,  1907,  1921,  1922,  1923,  1924,  1925,  1936,  1946,  1957,
    1967,  1978,  1989,  1998,  2007,  2016,  2027,  2038,  2052,  2062,
    2063,  2064,  2065,  2066,  2067,  2068,  2073,  2082,  2092,  2093,
    2094,  2101,  2113,  2124,  2129,  2133,  2137,  2154,  2167,  2171,
    2184,  2189,  2190,  2193,  2194,  2195,  2196,  2206,  2211,  2216,
    2221,  2227,  2236,  2241,  2242,  2253,  2254,  2257,  2261,  2264,
    2265,  2266,  2267,  2271,  2276,  2277,  2280,  2281,  2282,  2283,
    2284,  2287,  2288,  2291,  2292,  2295,  2296,  2297,  2298,  2303,
    2308,  2325,  2348,  2362,  2371,  2376,  2382,  2387,  2396,  2399,
    2403,  2409,  2410,  2414,  2419,  2420,  2421,  2422,  2437,  2441,
    2445,  2451,  2456,  2463,  2468,  2473,  2476,  2485,  2494,  2501,
    2514,  2521,  2522,  2534,  2539,  2540,  2541,  2542,  2546,  2556,
    2557,  2558,  2559,  2563,  2573,  2574,  2575,  2576,  2580,  2591,
    2596,  2597,  2598,  2602,  2612,  2613,  2614,  2615,  2619,  2629,
    2630,  2631,  2632,  2636,  2646,  2647,  2648,  2649,  2653,  2663,
    2664,  2665,  2666,  2670,  2680,  2681,  2682,  2683,  2684,  2687,
    2723,  2730,  2734,  2737,  2747,  2754,  2765,  2778,  2793,  2794,
    2797,  2808,  2814,  2818,  2821,  2827,  2841,  2846,  2855,  2856,
    2859,  2860,  2863,  2864,  2865,  2868,  2884,  2885,  2888,  2889,
    2892,  2893,  2894,  2895,  2896,  2897,  2900,  2909,  2918,  2926,
    2934,  2942,  2950,  2958,  2966,  2974,  2982,  2990,  2998,  3006,
    3014,  3022,  3030,  3038,  3042,  3047,  3055,  3062,  3069,  3083,
    3087,  3094,  3098,  3104,  3116,  3122,  3129,  3135,  3142,  3150,
    3158,  3166,  3174,  3181,  3189,  3190,  3191,  3192,  3193,  3196,
    3197,  3198,  3199,  3200,  3203,  3204,  3205,  3206,  3207,  3208,
    3209,  3210,  3211,  3212,  3213,  3214,  3215,  3216,  3217,  3218,
    3219,  3220,  3221,  3222,  3223,  3224,  3225,  3228,  3239,  3240,
    3243,  3252,  3256,  3262,  3268,  3273,  3276,  3281,  3286,  3289,
    3295,  3301,  3304,  3310,  3319,  3320,  3322,  3328,  3332,  3335,
    3340,  3347,  3353,  3354,  3357,  3358,  3369,  3370,  3373,  3379,
    3383,  3386,  3403,  3408,  3413,  3418,  3423,  3429,  3459,  3463,
    3467,  3471,  3475,  3481,  3485,  3488,  3492,  3498,  3504,  3507,
    3525,  3540,  3541,  3542,  3545,  3546,  3549,  3550,  3565,  3571,
    3574,  3595,  3596,  3597,  3600,  3601,  3604,  3611,  3612,  3615,
    3629,  3636,  3637,  3652,  3653,  3654,  3655,  3656,  3659,  3662,
    3668,  3674,  3678,  3682,  3689,  3696,  3703,  3710,  3716,  3722,
    3728,  3731,  3732,  3735,  3741,  3747,  3753,  3760,  3767,  3775,
    3776,  3779,  3785,  3789,  3792,  3797,  3802,  3806,  3812,  3828,
    3847,  3853,  3854,  3860,  3861,  3867,  3868,  3869,  3870,  3871,
    3872,  3873,  3874,  3875,  3876,  3877,  3878,  3879,  3880,  3883,
    3884,  3888,  3894,  3895,  3901,  3902,  3908,  3909,  3915,  3918,
    3919,  3930,  3931,  3934,  3938,  3941,  3947,  3953,  3954,  3957,
    3958,  3959,  3962,  3966,  3970,  3975,  3980,  3985,  3991,  3995,
    3999,  4003,  4009,  4014,  4018,  4026,  4035,  4036,  4039,  4042,
    4046,  4051,  4057,  4058,  4061,  4064,  4068,  4072,  4076,  4081,
    4088,  4093,  4101,  4106,  4115,  4116,  4122,  4123,  4124,  4127,
    4128,  4132,  4136,  4142,  4143,  4146,  4152,  4156,  4159,  4164,
    4170,  4171,  4174,  4175,  4176,  4182,  4183,  4184,  4185,  4188,
    4189,  4195,  4196,  4199,  4200,  4203,  4209,  4216,  4223,  4234,
    4235,  4236,  4239,  4247,  4259,  4268,  4279,  4285,  4311,  4312,
    4321,  4322,  4325,  4334,  4345,  4346,  4347,  4348,  4349,  4350,
    4351,  4352,  4353,  4354,  4355,  4356,  4359,  4360,  4361,  4362,
    4365,  4395,  4396,  4397,  4398,  4401,  4402,  4403,  4404,  4405,
    4408,  4412,  4415,  4419,  4426,  4429,  4445,  4446,  4450,  4456,
    4457,  4463,  4464,  4470,  4471,  4477,  4480,  4481,  4492,  4498,
    4511,  4512,  4515,  4521,  4522,  4523,  4526,  4533,  4538,  4543,
    4546,  4550,  4554,  4560,  4561,  4568,  4574,  4575,  4576,  4584,
    4585,  4588,  4594,  4600,  4604,  4607,  4628,  4632,  4636,  4646,
    4650,  4653,  4659,  4666,  4667,  4668,  4669,  4670,  4673,  4677,
    4681,  4691,  4694,  4695,  4698,  4699,  4700,  4701,  4712,  4723,
    4729,  4750,  4756,  4773,  4779,  4780,  4781,  4784,  4785,  4786,
    4789,  4790,  4793,  4816,  4822,  4828,  4835,  4848,  4856,  4864,
    4870,  4874,  4878,  4882,  4886,  4893,  4898,  4909,  4923,  4929,
    4933,  4937,  4944,  4952,  4959,  4967,  4971,  4977,  4983,  4991,
    4992,  4993,  4996,  4997,  5001,  5007,  5008,  5014,  5015,  5021,
    5022,  5028,  5031,  5032,  5033,  5042,  5053,  5054,  5057,  5065,
    5066,  5067,  5068,  5069,  5070,  5071,  5072,  5073,  5074,  5075,
    5076,  5077,  5078,  5081,  5082,  5083,  5084,  5085,  5092,  5099,
    5106,  5113,  5120,  5127,  5134,  5141,  5148,  5155,  5162,  5169,
    5176,  5179,  5180,  5181,  5182,  5183,  5184,  5185,  5188,  5192,
    5196,  5200,  5204,  5208,  5214,  5215,  5225,  5229,  5233,  5249,
    5250,  5253,  5254,  5255,  5256,  5257,  5260,  5261,  5262,  5263,
    5264,  5265,  5266,  5267,  5268,  5269,  5270,  5271,  5272,  5273,
    5274,  5275,  5276,  5277,  5278,  5279,  5280,  5281,  5282,  5283,
    5286,  5306,  5310,  5325,  5329,  5333,  5339,  5343,  5349,  5350,
    5351,  5354,  5355,  5358,  5359,  5362,  5368,  5369,  5372,  5373,
    5376,  5377,  5380,  5381,  5384,  5392,  5419,  5424,  5429,  5435,
    5436,  5439,  5443,  5463,  5464,  5465,  5466,  5469,  5473,  5477,
    5483,  5484,  5487,  5488,  5489,  5490,  5491,  5492,  5493,  5494,
    5495,  5496,  5497,  5498,  5499,  5500,  5501,  5502,  5503,  5506,
    5507,  5508,  5509,  5510,  5511,  5512,  5515,  5516,  5517,  5518,
    5521,  5522,  5523,  5524,  5527,  5528,  5531,  5537,  5545,  5558,
    5564,  5573,  5574,  5575,  5576,  5577,  5578,  5579,  5580,  5581,
    5582,  5583,  5584,  5585,  5586,  5587,  5588,  5589,  5590,  5591,
    5592,  5593,  5594,  5597,  5615,  5624,  5625,  5626,  5627,  5640,
    5646,  5647,  5648,  5651,  5657,  5658,  5659,  5660,  5661,  5664,
    5670,  5671,  5672,  5673,  5674,  5675,  5676,  5677,  5678,  5681,
    5685,  5696,  5703,  5704,  5705,  5706,  5707,  5708,  5709,  5710,
    5711,  5712,  5713,  5714,  5717,  5718,  5719,  5720,  5723,  5724,
    5725,  5726,  5727,  5730,  5736,  5737,  5738,  5739,  5740,  5741,
    5742,  5745,  5751,  5752,  5753,  5754,  5757,  5763,  5764,  5765,
    5766,  5767,  5768,  5769,  5770,  5771,  5773,  5779,  5780,  5781,
    5782,  5783,  5784,  5785,  5786,  5787,  5788,  5791,  5797,  5798,
    5799,  5800,  5801,  5804,  5810,  5811,  5814,  5820,  5821,  5822,
    5825,  5831,  5832,  5833,  5834,  5837,  5843,  5844,  5845,  5846,
    5849,  5853,  5858,  5866,  5873,  5874,  5875,  5876,  5877,  5878,
    5879,  5880,  5881,  5882,  5883,  5884,  5885,  5886,  5889,  5890,
    5891,  5894,  5895,  5898,  5906,  5914,  5915,  5918,  5926,  5934,
    5935,  5938,  5942,  5949,  5950,  5951,  5954,  5961,  5968,  5969,
    5970,  5971,  5972,  5973,  5974,  5975,  5976,  5977,  5980,  5985,
    5990,  5995,  6000,  6005,  6012,  6013,  6014,  6015,  6016,  6019,
    6020,  6021,  6022,  6023,  6024,  6025,  6026,  6027,  6028,  6029,
    6030,  6039,  6040,  6043,  6046,  6047,  6050,  6053,  6056,  6060,
    6071,  6072,  6073,  6076,  6077,  6078,  6079,  6080,  6081,  6082,
    6083,  6084,  6085,  6086,  6087,  6088,  6089,  6090,  6091,  6092,
    6093,  6096,  6097,  6098,  6101,  6102,  6103,  6104,  6107,  6108,
    6109,  6110,  6111,  6114,  6115,  6116,  6117,  6120,  6125,  6129,
    6133,  6137,  6141,  6145,  6150,  6155,  6160,  6165,  6170,  6177,
    6181,  6187,  6188,  6189,  6190,  6193,  6201,  6202,  6205,  6206,
    6207,  6208,  6209,  6210,  6211,  6212,  6215,  6221,  6222,  6225,
    6231,  6232,  6233,  6234,  6237,  6243,  6249,  6255,  6258,  6264,
    6265,  6266,  6267,  6273,  6279,  6280,  6281,  6282,  6283,  6284,
    6287,  6293,  6294,  6297,  6303,  6304,  6305,  6306,  6307,  6310,
    6324,  6325,  6326,  6327,  6328
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "\"junk\"",
  "\"CRLF line terminators\"", "\"newline\"", "\"colon\"", "\"semicolon\"",
  "\"comma\"", "\".\"", "\"==\"", "\"!=\"", "\"<\"", "\">\"", "\">=\"",
  "\"<=\"", "\"<<\"", "\">>\"", "\"&\"", "\"^\"", "\"!\"", "\"/\"",
  "\"*\"", "\"-\"", "\"@\"", "\"vmap\"", "\"+\"", "\"include\"",
  "\"define\"", "\"redefine\"", "\"undefine\"", "\"fib\"", "\"check\"",
  "\"socket\"", "\"transparent\"", "\"wildcard\"", "\"cgroupv2\"",
  "\"tproxy\"", "\"osf\"", "\"synproxy\"", "\"mss\"", "\"wscale\"",
  "\"typeof\"", "\"hook\"", "\"hooks\"", "\"device\"", "\"devices\"",
  "\"table\"", "\"tables\"", "\"chain\"", "\"chains\"", "\"rule\"",
  "\"rules\"", "\"sets\"", "\"set\"", "\"element\"", "\"map\"", "\"maps\"",
  "\"flowtable\"", "\"handle\"", "\"ruleset\"", "\"trace\"", "\"inet\"",
  "\"netdev\"", "\"add\"", "\"update\"", "\"replace\"", "\"create\"",
  "\"insert\"", "\"delete\"", "\"get\"", "\"list\"", "\"reset\"",
  "\"flush\"", "\"rename\"", "\"describe\"", "\"import\"", "\"export\"",
  "\"destroy\"", "\"monitor\"", "\"all\"", "\"accept\"", "\"drop\"",
  "\"continue\"", "\"jump\"", "\"goto\"", "\"return\"", "\"to\"",
  "\"constant\"", "\"interval\"", "\"dynamic\"", "\"auto-merge\"",
  "\"timeout\"", "\"gc-interval\"", "\"elements\"", "\"expires\"",
  "\"policy\"", "\"memory\"", "\"performance\"", "\"size\"", "\"flow\"",
  "\"offload\"", "\"meter\"", "\"meters\"", "\"flowtables\"", "\"number\"",
  "\"string\"", "\"quoted string\"", "\"string with a trailing asterisk\"",
  "\"ll\"", "\"nh\"", "\"th\"", "\"bridge\"", "\"ether\"", "\"saddr\"",
  "\"daddr\"", "\"type\"", "\"vlan\"", "\"id\"", "\"cfi\"", "\"dei\"",
  "\"pcp\"", "\"arp\"", "\"htype\"", "\"ptype\"", "\"hlen\"", "\"plen\"",
  "\"operation\"", "\"ip\"", "\"version\"", "\"hdrlength\"", "\"dscp\"",
  "\"ecn\"", "\"length\"", "\"frag-off\"", "\"ttl\"", "\"protocol\"",
  "\"checksum\"", "\"ptr\"", "\"value\"", "\"lsrr\"", "\"rr\"", "\"ssrr\"",
  "\"ra\"", "\"icmp\"", "\"code\"", "\"seq\"", "\"gateway\"", "\"mtu\"",
  "\"igmp\"", "\"mrt\"", "\"options\"", "\"ip6\"", "\"priority\"",
  "\"flowlabel\"", "\"nexthdr\"", "\"hoplimit\"", "\"icmpv6\"",
  "\"param-problem\"", "\"max-delay\"", "\"taddr\"", "\"ah\"",
  "\"reserved\"", "\"spi\"", "\"esp\"", "\"comp\"", "\"flags\"", "\"cpi\"",
  "\"port\"", "\"udp\"", "\"sport\"", "\"dport\"", "\"udplite\"",
  "\"csumcov\"", "\"tcp\"", "\"ackseq\"", "\"doff\"", "\"window\"",
  "\"urgptr\"", "\"option\"", "\"echo\"", "\"eol\"", "\"mptcp\"",
  "\"nop\"", "\"sack\"", "\"sack0\"", "\"sack1\"", "\"sack2\"",
  "\"sack3\"", "\"sack-permitted\"", "\"fastopen\"", "\"md5sig\"",
  "\"timestamp\"", "\"count\"", "\"left\"", "\"right\"", "\"tsval\"",
  "\"tsecr\"", "\"subtype\"", "\"dccp\"", "\"vxlan\"", "\"vni\"",
  "\"gre\"", "\"gretap\"", "\"geneve\"", "\"sctp\"", "\"chunk\"",
  "\"data\"", "\"init\"", "\"init-ack\"", "\"heartbeat\"",
  "\"heartbeat-ack\"", "\"abort\"", "\"shutdown\"", "\"shutdown-ack\"",
  "\"error\"", "\"cookie-echo\"", "\"cookie-ack\"", "\"ecne\"", "\"cwr\"",
  "\"shutdown-complete\"", "\"asconf-ack\"", "\"forward-tsn\"",
  "\"asconf\"", "\"tsn\"", "\"stream\"", "\"ssn\"", "\"ppid\"",
  "\"init-tag\"", "\"a-rwnd\"", "\"num-outbound-streams\"",
  "\"num-inbound-streams\"", "\"initial-tsn\"", "\"cum-tsn-ack\"",
  "\"num-gap-ack-blocks\"", "\"num-dup-tsns\"", "\"lowest-tsn\"",
  "\"seqno\"", "\"new-cum-tsn\"", "\"vtag\"", "\"rt\"", "\"rt0\"",
  "\"rt2\"", "\"srh\"", "\"seg-left\"", "\"addr\"", "\"last-entry\"",
  "\"tag\"", "\"sid\"", "\"hbh\"", "\"frag\"", "\"reserved2\"",
  "\"more-fragments\"", "\"dst\"", "\"mh\"", "\"meta\"", "\"mark\"",
  "\"iif\"", "\"iifname\"", "\"iiftype\"", "\"oif\"", "\"oifname\"",
  "\"oiftype\"", "\"skuid\"", "\"skgid\"", "\"nftrace\"", "\"rtclassid\"",
  "\"ibriport\"", "\"obriport\"", "\"ibrname\"", "\"obrname\"",
  "\"pkttype\"", "\"cpu\"", "\"iifgroup\"", "\"oifgroup\"", "\"cgroup\"",
  "\"time\"", "\"classid\"", "\"nexthop\"", "\"ct\"", "\"l3proto\"",
  "\"proto-src\"", "\"proto-dst\"", "\"zone\"", "\"direction\"",
  "\"event\"", "\"expectation\"", "\"expiration\"", "\"helper\"",
  "\"label\"", "\"state\"", "\"status\"", "\"original\"", "\"reply\"",
  "\"counter\"", "\"name\"", "\"packets\"", "\"bytes\"", "\"avgpkt\"",
  "\"last\"", "\"never\"", "\"counters\"", "\"quotas\"", "\"limits\"",
  "\"synproxys\"", "\"helpers\"", "\"log\"", "\"prefix\"", "\"group\"",
  "\"snaplen\"", "\"queue-threshold\"", "\"level\"", "\"limit\"",
  "\"rate\"", "\"burst\"", "\"over\"", "\"until\"", "\"quota\"",
  "\"used\"", "\"secmark\"", "\"secmarks\"", "\"second\"", "\"minute\"",
  "\"hour\"", "\"day\"", "\"week\"", "\"reject\"", "\"with\"", "\"icmpx\"",
  "\"snat\"", "\"dnat\"", "\"masquerade\"", "\"redirect\"", "\"random\"",
  "\"fully-random\"", "\"persistent\"", "\"queue\"", "\"num\"",
  "\"bypass\"", "\"fanout\"", "\"dup\"", "\"fwd\"", "\"numgen\"",
  "\"inc\"", "\"mod\"", "\"offset\"", "\"jhash\"", "\"symhash\"",
  "\"seed\"", "\"position\"", "\"index\"", "\"comment\"", "\"xml\"",
  "\"json\"", "\"vm\"", "\"notrack\"", "\"exists\"", "\"missing\"",
  "\"exthdr\"", "\"ipsec\"", "\"reqid\"", "\"spnum\"", "\"in\"", "\"out\"",
  "\"xt\"", "'='", "'{'", "'}'", "'('", "')'", "'|'", "'$'", "'['", "']'",
  "$accept", "input", "stmt_separator", "opt_newline", "close_scope_ah",
  "close_scope_arp", "close_scope_at", "close_scope_comp",
  "close_scope_ct", "close_scope_counter", "close_scope_last",
  "close_scope_dccp", "close_scope_destroy", "close_scope_dst",
  "close_scope_dup", "close_scope_esp", "close_scope_eth",
  "close_scope_export", "close_scope_fib", "close_scope_frag",
  "close_scope_fwd", "close_scope_gre", "close_scope_hash",
  "close_scope_hbh", "close_scope_ip", "close_scope_ip6",
  "close_scope_vlan", "close_scope_icmp", "close_scope_igmp",
  "close_scope_import", "close_scope_ipsec", "close_scope_list",
  "close_scope_limit", "close_scope_meta", "close_scope_mh",
  "close_scope_monitor", "close_scope_nat", "close_scope_numgen",
  "close_scope_osf", "close_scope_policy", "close_scope_quota",
  "close_scope_queue", "close_scope_reject", "close_scope_reset",
  "close_scope_rt", "close_scope_sctp", "close_scope_sctp_chunk",
  "close_scope_secmark", "close_scope_socket", "close_scope_tcp",
  "close_scope_tproxy", "close_scope_type", "close_scope_th",
  "close_scope_udp", "close_scope_udplite", "close_scope_log",
  "close_scope_synproxy", "close_scope_xt", "common_block", "line",
  "base_cmd", "add_cmd", "replace_cmd", "create_cmd", "insert_cmd",
  "table_or_id_spec", "chain_or_id_spec", "set_or_id_spec",
  "obj_or_id_spec", "delete_cmd", "destroy_cmd", "get_cmd",
  "list_cmd_spec_table", "list_cmd_spec_any", "list_cmd",
  "basehook_device_name", "basehook_spec", "reset_cmd", "flush_cmd",
  "rename_cmd", "import_cmd", "export_cmd", "monitor_cmd", "monitor_event",
  "monitor_object", "monitor_format", "markup_format", "describe_cmd",
  "table_block_alloc", "table_options", "table_flags", "table_flag",
  "table_block", "chain_block_alloc", "chain_block", "subchain_block",
  "typeof_verdict_expr", "typeof_data_expr", "primary_typeof_expr",
  "typeof_expr", "set_block_alloc", "typeof_key_expr", "set_block",
  "set_block_expr", "set_flag_list", "set_flag", "map_block_alloc",
  "ct_obj_type_map", "map_block_obj_type", "map_block_obj_typeof",
  "map_block_data_interval", "map_block", "set_mechanism",
  "set_policy_spec", "flowtable_block_alloc", "flowtable_block",
  "flowtable_expr", "flowtable_list_expr", "flowtable_expr_member",
  "data_type_atom_expr", "data_type_expr", "obj_block_alloc",
  "counter_block", "quota_block", "ct_helper_block", "ct_timeout_block",
  "ct_expect_block", "limit_block", "secmark_block", "synproxy_block",
  "type_identifier", "hook_spec", "prio_spec", "extended_prio_name",
  "extended_prio_spec", "int_num", "dev_spec", "flags_spec", "policy_spec",
  "policy_expr", "chain_policy", "identifier", "string", "time_spec",
  "time_spec_or_num_s", "family_spec", "family_spec_explicit",
  "table_spec", "tableid_spec", "chain_spec", "chainid_spec",
  "chain_identifier", "set_spec", "setid_spec", "set_identifier",
  "flowtable_spec", "flowtableid_spec", "flowtable_identifier", "obj_spec",
  "objid_spec", "obj_identifier", "handle_spec", "position_spec",
  "index_spec", "rule_position", "ruleid_spec", "comment_spec",
  "ruleset_spec", "rule", "rule_alloc", "stmt_list", "stateful_stmt_list",
  "objref_stmt_counter", "objref_stmt_limit", "objref_stmt_quota",
  "objref_stmt_synproxy", "objref_stmt_ct", "objref_stmt", "stateful_stmt",
  "stmt", "xt_stmt", "chain_stmt_type", "chain_stmt", "verdict_stmt",
  "verdict_map_stmt", "verdict_map_expr", "verdict_map_list_expr",
  "verdict_map_list_member_expr", "ct_limit_stmt_alloc", "connlimit_stmt",
  "ct_limit_args", "counter_stmt", "counter_stmt_alloc", "counter_args",
  "counter_arg", "last_stmt_alloc", "last_stmt", "last_args", "log_stmt",
  "log_stmt_alloc", "log_args", "log_arg", "level_type", "log_flags",
  "log_flags_tcp", "log_flag_tcp", "limit_stmt_alloc", "limit_stmt",
  "limit_args", "quota_mode", "quota_unit", "quota_used",
  "quota_stmt_alloc", "quota_stmt", "quota_args", "limit_mode",
  "limit_burst_pkts", "limit_rate_pkts", "limit_burst_bytes",
  "limit_rate_bytes", "limit_bytes", "time_unit", "reject_stmt",
  "reject_stmt_alloc", "reject_with_expr", "reject_opts", "nat_stmt",
  "nat_stmt_alloc", "tproxy_stmt", "synproxy_stmt", "synproxy_stmt_alloc",
  "synproxy_args", "synproxy_arg", "synproxy_config", "synproxy_obj",
  "synproxy_ts", "synproxy_sack", "primary_stmt_expr", "shift_stmt_expr",
  "and_stmt_expr", "exclusive_or_stmt_expr", "inclusive_or_stmt_expr",
  "basic_stmt_expr", "concat_stmt_expr", "map_stmt_expr_set",
  "map_stmt_expr", "prefix_stmt_expr", "range_stmt_expr",
  "multiton_stmt_expr", "stmt_expr", "nat_stmt_args", "masq_stmt",
  "masq_stmt_alloc", "masq_stmt_args", "redir_stmt", "redir_stmt_alloc",
  "redir_stmt_arg", "dup_stmt", "fwd_stmt", "nf_nat_flags", "nf_nat_flag",
  "queue_stmt", "queue_stmt_compat", "queue_stmt_alloc", "queue_stmt_args",
  "queue_stmt_arg", "queue_expr", "queue_stmt_expr_simple",
  "queue_stmt_expr", "queue_stmt_flags", "queue_stmt_flag",
  "set_elem_expr_stmt", "set_elem_expr_stmt_alloc", "set_stmt",
  "set_stmt_op", "map_stmt", "meter_stmt", "match_stmt", "variable_expr",
  "symbol_expr", "set_ref_expr", "set_ref_symbol_expr", "integer_expr",
  "selector_expr", "primary_expr", "fib_expr", "fib_result", "fib_flag",
  "fib_tuple", "osf_expr", "osf_ttl", "shift_expr", "and_expr",
  "exclusive_or_expr", "inclusive_or_expr", "basic_expr", "concat_expr",
  "prefix_rhs_expr", "range_rhs_expr", "multiton_rhs_expr", "map_expr",
  "expr", "set_expr", "set_list_expr", "set_list_member_expr",
  "meter_key_expr", "meter_key_expr_alloc", "set_elem_expr",
  "set_elem_key_expr", "set_elem_expr_alloc", "set_elem_options",
  "set_elem_time_spec", "set_elem_option", "set_elem_expr_options",
  "set_elem_stmt_list", "set_elem_stmt", "set_elem_expr_option",
  "set_lhs_expr", "set_rhs_expr", "initializer_expr", "counter_config",
  "counter_obj", "quota_config", "quota_obj", "secmark_config",
  "secmark_obj", "ct_obj_type", "ct_cmd_type", "ct_l4protoname",
  "ct_helper_config", "timeout_states", "timeout_state",
  "ct_timeout_config", "ct_expect_config", "ct_obj_alloc", "limit_config",
  "limit_obj", "relational_expr", "list_rhs_expr", "rhs_expr",
  "shift_rhs_expr", "and_rhs_expr", "exclusive_or_rhs_expr",
  "inclusive_or_rhs_expr", "basic_rhs_expr", "concat_rhs_expr",
  "boolean_keys", "boolean_expr", "keyword_expr", "primary_rhs_expr",
  "relational_op", "verdict_expr", "chain_expr", "meta_expr", "meta_key",
  "meta_key_qualified", "meta_key_unqualified", "meta_stmt", "socket_expr",
  "socket_key", "offset_opt", "numgen_type", "numgen_expr", "xfrm_spnum",
  "xfrm_dir", "xfrm_state_key", "xfrm_state_proto_key", "xfrm_expr",
  "hash_expr", "nf_key_proto", "rt_expr", "rt_key", "ct_expr", "ct_dir",
  "ct_key", "ct_key_dir", "ct_key_proto_field", "ct_key_dir_optional",
  "symbol_stmt_expr", "list_stmt_expr", "ct_stmt", "payload_stmt",
  "payload_expr", "payload_raw_len", "payload_raw_expr",
  "payload_base_spec", "eth_hdr_expr", "eth_hdr_field", "vlan_hdr_expr",
  "vlan_hdr_field", "arp_hdr_expr", "arp_hdr_field", "ip_hdr_expr",
  "ip_hdr_field", "ip_option_type", "ip_option_field", "icmp_hdr_expr",
  "icmp_hdr_field", "igmp_hdr_expr", "igmp_hdr_field", "ip6_hdr_expr",
  "ip6_hdr_field", "icmp6_hdr_expr", "icmp6_hdr_field", "auth_hdr_expr",
  "auth_hdr_field", "esp_hdr_expr", "esp_hdr_field", "comp_hdr_expr",
  "comp_hdr_field", "udp_hdr_expr", "udp_hdr_field", "udplite_hdr_expr",
  "udplite_hdr_field", "tcp_hdr_expr", "inner_inet_expr", "inner_eth_expr",
  "inner_expr", "vxlan_hdr_expr", "vxlan_hdr_field", "geneve_hdr_expr",
  "geneve_hdr_field", "gre_hdr_expr", "gre_hdr_field", "gretap_hdr_expr",
  "optstrip_stmt", "tcp_hdr_field", "tcp_hdr_option_kind_and_field",
  "tcp_hdr_option_sack", "tcp_hdr_option_type", "tcpopt_field_sack",
  "tcpopt_field_window", "tcpopt_field_tsopt", "tcpopt_field_maxseg",
  "tcpopt_field_mptcp", "dccp_hdr_expr", "dccp_hdr_field",
  "sctp_chunk_type", "sctp_chunk_common_field", "sctp_chunk_data_field",
  "sctp_chunk_init_field", "sctp_chunk_sack_field", "sctp_chunk_alloc",
  "sctp_hdr_expr", "sctp_hdr_field", "th_hdr_expr", "th_hdr_field",
  "exthdr_expr", "hbh_hdr_expr", "hbh_hdr_field", "rt_hdr_expr",
  "rt_hdr_field", "rt0_hdr_expr", "rt0_hdr_field", "rt2_hdr_expr",
  "rt2_hdr_field", "rt4_hdr_expr", "rt4_hdr_field", "frag_hdr_expr",
  "frag_hdr_field", "dst_hdr_expr", "dst_hdr_field", "mh_hdr_expr",
  "mh_hdr_field", "exthdr_exists_expr", "exthdr_key", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-1846)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1066)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1846,  8881, -1846,   914, -1846, -1846,   171,   151,   151,   151,
    1366,  1366,  1366,  1366,  1366,  1366,  1366,  1366, -1846, -1846,
    3934,   231,  3482,   243,  3147,   265,  5611,   735,  1324,   304,
    8036,   140,   211,  3194,   230, -1846, -1846, -1846, -1846,   632,
    1366,  1366,  1366,  1366, -1846, -1846, -1846,  1072, -1846,   151,
   -1846,   151,   196,  7314, -1846,   914, -1846, -1846,   -10,    -4,
     914,   151, -1846,    11,    49,  7314,   151, -1846,   282, -1846,
     151, -1846, -1846,  1366, -1846,  1366,  1366,  1366,  1366,  1366,
    1366,  1366,   700,  1366,  1366,  1366,  1366, -1846,  1366, -1846,
    1366,  1366,  1366,  1366,  1366,  1366,  1366,  1366,   744,  1366,
    1366,  1366,  1366, -1846,  1366, -1846,  1366,  1366,  1366,  1366,
    1366,  1366,  1160,  1366,  1366,  1160,  1366,  1366,   891,  1366,
    1366,  1160,   447,  1366,  1160,  1160,  1160,  1160,  1366,  1366,
    1366,  1160, -1846,  1366,  1419,  1366,  1366,  1366,  1366,  1160,
    1160,  1366, -1846,  1366,  1366,  1366,  1366,  1366,   344,  1366,
   -1846,  1366, -1846,   695,   902,   582,   310, -1846, -1846, -1846,
   -1846,   814,  1252,  1672,  1817,  2501,  1745,   307,  1578,  1845,
    1461,   142,   855,  1148,  1020,  2232,   915,  4793,   798, -1846,
    5138,  1199,  2027,   290,   298,   646,   162,  1099,   191,   927,
    3354, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  4572, -1846, -1846,   497,  7848,   205,  1255,   628,
    8036,   151, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1330, -1846, -1846,   280, -1846, -1846,  1330, -1846,
   -1846,  1366,  1366,  1366,  1366,  1366,  1366,  1366,  1366,   744,
    1366,  1366,  1366,  1366, -1846, -1846, -1846,  1560, -1846, -1846,
   -1846,  1366,  1366,  1366,   330, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,   465,   536,   554, -1846, -1846, -1846,   801,   405,
     657, -1846, -1846, -1846,   539, -1846, -1846, -1846,   110,   110,
   -1846,   236,   151,  4076,  3656,   450, -1846, -1846,    47,   463,
   -1846, -1846, -1846, -1846, -1846,   157,   683,   849, -1846,   691,
     812, -1846,   473,  7314, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,   134, -1846, -1846,
     709,   523, -1846, -1846,  1014,   790, -1846,   816, -1846, -1846,
     548, -1846,  5433, -1846, -1846,   753, -1846,   165, -1846,   248,
   -1846, -1846, -1846, -1846,  1183, -1846,   143, -1846, -1846, -1846,
   -1846,  1167,   865,   868,   542, -1846,   938, -1846,  6726, -1846,
   -1846, -1846,   871, -1846, -1846, -1846,   879, -1846, -1846,  7083,
    7083, -1846, -1846,   139,   578,   590, -1846, -1846,   596, -1846,
   -1846, -1846,   606, -1846,   634,   969,  7314, -1846,    11,    49,
   -1846,   282, -1846, -1846,  1366,  1366,  1366,   746, -1846, -1846,
   -1846,  7314, -1846,   228, -1846, -1846, -1846,   257, -1846, -1846,
   -1846,   270,    49, -1846, -1846, -1846,   273, -1846, -1846,   282,
   -1846,   358,   684, -1846, -1846, -1846, -1846,  1366, -1846, -1846,
   -1846, -1846,   282, -1846, -1846, -1846,  1011, -1846, -1846, -1846,
   -1846,  1366, -1846, -1846,   192, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1366,  1366, -1846, -1846, -1846,  1036,  1048, -1846,
    1366,  1054, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846,  1366, -1846,   151, -1846, -1846, -1846,   282,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846,  1366, -1846,   151, -1846, -1846, -1846, -1846,  1114, -1846,
   -1846, -1846, -1846, -1846,  1058,   360, -1846, -1846,   833, -1846,
   -1846,  1043,   103, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,   268,   309, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,  1485, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846,  4110, -1846, -1846, -1846, -1846,
    1056, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  5686, -1846,
    5447, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  4811, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846,   612, -1846, -1846,   793, -1846, -1846, -1846, -1846, -1846,
   -1846,   802, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1798, -1846, -1846, -1846, -1846,   862,
     331,   867,  1094, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,   858,   852, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,   282, -1846,   684,
   -1846,  1366, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846,  1330, -1846, -1846, -1846, -1846,
     391,   -64,   434,   218, -1846, -1846, -1846,  5704,  1143,  7581,
    8036,  1055, -1846, -1846, -1846, -1846,  1212,  1214,    86,  1198,
    1204,  1206, -1846,  1208,  1798,  1235,  7581,  7581, -1846,  7581,
    8036,   877,  7581,  7581,  1187,  1606, -1846,  6827,   148, -1846,
    1606, -1846, -1846, -1846,   936, -1846,  1205, -1846, -1846, -1846,
    1222,  1228,   709, -1846,   207, -1846, -1846, -1846,   846,  1606,
    1233,  1244,  1251,  1606,  1014, -1846, -1846, -1846, -1846,  1256,
   -1846, -1846, -1846,  1258, -1846, -1846, -1846,   212, -1846, -1846,
    7581, -1846, -1846,  5975,  1230,  1252,  1672,  1817,  2501, -1846,
    1578,   636, -1846, -1846, -1846, -1846,  1253, -1846, -1846, -1846,
   -1846,  7581, -1846,  1226,  1326,  1353,  1007,   955,   588, -1846,
   -1846, -1846, -1846,  1375,  1386,  1377, -1846, -1846, -1846, -1846,
   -1846,  1383, -1846, -1846, -1846, -1846,   574, -1846, -1846,  1396,
    1398, -1846, -1846, -1846,  1278,  1308, -1846, -1846,   753, -1846,
   -1846,  1409, -1846, -1846, -1846, -1846,  1417, -1846, -1846,  6246,
   -1846,  1417, -1846, -1846, -1846,    96, -1846, -1846,  1183, -1846,
    1445, -1846,   151, -1846,  1098, -1846,  8974,  8974,  8974,  8974,
    8974,  8036,   138,  8241, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  8974,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,   408, -1846,  1275,
    1449,  1458,  1109,  1026,  1474, -1846, -1846, -1846,  8241,  7581,
    7581,  1388,   149,   914,  1498, -1846,  1057,   914,  1406, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1476,  1162,
    1169,  1170, -1846,  1173,  1174, -1846, -1846, -1846, -1846,  1237,
    1229,   816,  1606, -1846, -1846,  1439,  1441,  1453,  1193,  1457,
   -1846,  1462,  1197, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
    1460, -1846, -1846, -1846, -1846, -1846,  1366, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1464,   902,
   -1846, -1846, -1846, -1846, -1846,  1467, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1102, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1477,
   -1846,  1380, -1846, -1846,  1382, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1117, -1846,  1216,  1442, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846,   942,  1504,  1168,  1168, -1846,
   -1846, -1846,  1351, -1846, -1846, -1846, -1846,  1349,  1352, -1846,
    1356,  1359,  1364,   739, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846,  1491, -1846, -1846,  1497, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1184,
   -1846,  1239, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1501,
    1509,  1273, -1846, -1846, -1846, -1846, -1846,  1517,   619, -1846,
   -1846, -1846,  1265, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
    1268,  1269,  1272,  1534, -1846, -1846,   790, -1846, -1846, -1846,
    1536, -1846, -1846, -1846, -1846,  7581,  2501,  1578,  1639,  6517,
   -1846,   143,   100,  1633,  4564,  1606,  1606,  1541,  8036,  7581,
    7581,  7581,  7581,  1595,  7581, -1846, -1846, -1846,  1628, -1846,
   -1846,   258,   809,   331, -1846,   348,   861,   217,  1607, -1846,
    7581, -1846, -1846,   812,  1413,  1470,   146, -1846,  1104,  1474,
     812, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846,  1512,   373, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,   206,  1341,  1354,  1646,   183,  1163,
    1200, -1846,  1236, -1846, -1846, -1846,  7581,  1668,  7581, -1846,
   -1846, -1846,   685,   774, -1846,  7581, -1846, -1846,  1310, -1846,
   -1846,  7581,  7581,  7581,  7581,  7581,  1575,  7581,  7581,   167,
    7581,  1417,  7581,  1599,  1681,  1609,  3527,  3527, -1846, -1846,
   -1846,  7581,  1386,  7581,  1386, -1846,  1674,  1676, -1846,   877,
   -1846,  8036, -1846, -1846,  1275,  1449,  1458, -1846,   812, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,  1348,  8974,  8974,  8974,
    8974,  8974,  8974,  8974,  8974,  9163,  8974,  8974,   603, -1846,
    1113, -1846, -1846, -1846, -1846, -1846,  1619, -1846,   712,   573,
   -1846,  3103,  3758,  1266,  2660,   501, -1846, -1846, -1846, -1846,
   -1846, -1846,  1374,  1379,  1384, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
    1733, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  4564, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1373,  1389, -1846, -1846, -1846, -1846, -1846, -1846,
    1273,   287,  1642, -1846, -1846, -1846, -1846, -1846,  1307, -1846,
   -1846, -1846, -1846, -1846,  1455,  2903, -1846,  1256,   830, -1846,
     483,   183, -1846,   953, -1846, -1846,  7581,  7581,  1753, -1846,
   -1846,  1657,  1657, -1846,   100, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1399,  1633,  7314,   100, -1846, -1846,
   -1846, -1846,  7581, -1846, -1846, -1846, -1846,    96, -1846,  8036,
      96,  7581,  1735, -1846,  8893, -1846,  1588, -1846,  1486, -1846,
   -1846, -1846, -1846, -1846, -1846,  1470, -1846,  1677,  1657, -1846,
     656, -1846,  6827, -1846,  5157, -1846, -1846, -1846, -1846,  1793,
   -1846,  1381,  1781, -1846,  1698, -1846,  1700, -1846,  1381, -1846,
   -1846,  1500, -1846,  1335, -1846, -1846,  1335, -1846,  1743,  1335,
   -1846, -1846,  7581, -1846, -1846, -1846, -1846, -1846,  1226,  1326,
    1353, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1810,  7581,
    1651,  7581, -1846, -1846, -1846, -1846,  1386, -1846,  1386,  1417,
   -1846, -1846,  1186,  6973,   153, -1846, -1846, -1846,  1498,  1816,
   -1846, -1846,  1275,  1449,  1458, -1846,   333,  1498, -1846, -1846,
    1104,  8974,  9163, -1846,  1723,  1788, -1846, -1846, -1846, -1846,
   -1846,   151,   151,   151,   151,   151,  1724,   747,   151,   151,
     151,   151, -1846, -1846, -1846,   914, -1846,  1466,   204, -1846,
    1732, -1846, -1846, -1846,   914,   914,   914,   914,   914,  8575,
   -1846,  1657,  1657,  1469,  1358,  1734,  1024,  1569, -1846, -1846,
   -1846,   914,   914,   914,   649, -1846,  8575,  1657,  1657,  1473,
    1024,  1569, -1846, -1846, -1846,   914,   914,   649,  1737,  1479,
    1740, -1846, -1846, -1846, -1846, -1846,  3534,  4114,  2443,  2864,
    2047, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  4463,  2346,
   -1846, -1846,  1741, -1846, -1846, -1846,  1843, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,  1748, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  2555, -1846,   997,  1140,  1159,  1749,
   -1846, -1846, -1846, -1846, -1846,  1341,  1354, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1500, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846,  7581, -1846, -1846, -1846, -1846,
   -1846, -1846,  8036,  1502,   100, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,  7581,   110,   110,
     812,  1474, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1470, -1846, -1846, -1846,   914, -1846,   373, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,  1579,   288, -1846, -1846,
    1766, -1846, -1846, -1846, -1846, -1846, -1846,  7581, -1846,  1787,
   -1846,  1417,  1417,  8036, -1846,  1238,  1870,   812, -1846,  1498,
    1498,  1685,  1775, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1875, -1846,   151,   151,   151, -1846,
   -1846, -1846, -1846, -1846,   529, -1846, -1846, -1846, -1846, -1846,
    1779, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1877, -1846,
     914,   914,   282, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,  1878, -1846, -1846, -1846, -1846, -1846,
    1279, -1846, -1846, -1846, -1846, -1846, -1846,   860,   914,   914,
     282,   961,  1279, -1846, -1846, -1846,  1742,   529,   914, -1846,
   -1846, -1846, -1846, -1846, -1846,  2163,  1643,  2598, -1846, -1846,
   -1846, -1846,  1783, -1846,  1273, -1846, -1846, -1846,  1529,   730,
    1366, -1846, -1846, -1846, -1846, -1846,  1657,  1791,   730,  1795,
    1366, -1846, -1846, -1846, -1846, -1846,  1796,  1366, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  7314, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846,   183, -1846, -1846, -1846, -1846, -1846,  7581,
    1535,  8036, -1846,  1825,  6973, -1846, -1846,  1715,   914,  1548,
    1549,  1550,  1551,  1554,  1724, -1846, -1846, -1846,  1556,  1558,
    1567,  1570,   152,   914, -1846, -1846,  1892,  8575, -1846, -1846,
   -1846, -1846,  1024, -1846,  1569, -1846,  8387, -1846, -1846, -1846,
     834, -1846,   190,   914,   914, -1846, -1846, -1846, -1846, -1846,
    1929, -1846,  1573, -1846, -1846,   914,   914, -1846,   914,   914,
     914,   914,   914, -1846,  1819,   914, -1846,  1585, -1846, -1846,
   -1846, -1846, -1846,  1589,   812, -1846, -1846,  1685, -1846, -1846,
   -1846, -1846, -1846, -1846,  1580,  1593,  1596, -1846, -1846, -1846,
   -1846, -1846, -1846,   158, -1846, -1846, -1846,  1858, -1846, -1846,
   -1846, -1846,  8575, -1846,  4361, -1846, -1846, -1846, -1846, -1846,
   -1846,   914,  1957, -1846,   914,  1958, -1846,   914,  1024,  1867,
   -1846, -1846, -1846,  1061, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1741, -1846,  1869, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846,   730, -1846, -1846, -1846, -1846,  1715,
    1219,  4812,  2560,  3359,  2690, -1846, -1846, -1846,  2941,  1302,
    2109,  1407,   155, -1846,  1424, -1846,  1877, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846,  8575, -1846, -1846,  1220, -1846,
    1868,  1872, -1846,  1972,   170, -1846,   914, -1846,   914,   914,
     914,   914,   914,  2762,  2804,  3058,   914,   914,   914,   914,
   -1846, -1846,   202,  1616,  1742, -1846,  1958, -1846, -1846, -1846,
   -1846,  1394,  1869,   914, -1846, -1846, -1846, -1846, -1846, -1846,
     914,   914,   914, -1846, -1846, -1846, -1846, -1846, -1846,   529,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     1,     0,     4,     5,     0,     0,     0,     0,
     438,   438,   438,   438,   438,   438,   438,   438,   442,   445,
     438,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   230,   444,     9,    28,    29,     0,
     438,   438,   438,   438,    68,    67,     3,     0,    71,     0,
     439,     0,   463,     0,    66,     0,   430,   431,     0,     0,
       0,     0,   630,    87,    89,     0,     0,   290,     0,   313,
       0,   343,    72,   438,    73,   438,   438,   438,   438,   438,
     438,   438,     0,   438,   438,   438,   438,    74,   438,    75,
     438,   438,   438,   438,   438,   438,   438,   438,     0,   438,
     438,   438,   438,    76,   438,    77,   438,   469,   438,   469,
     438,   469,   469,   438,   438,   469,   438,   469,     0,   438,
     469,   469,     0,   438,   469,   469,   469,   469,   438,   438,
     438,   469,    35,   438,   469,   438,   438,   438,   438,   469,
     469,   438,    47,   438,   438,   438,   438,   469,     0,   438,
      80,   438,    81,     0,     0,     0,   784,   753,   432,   433,
     434,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    25,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   987,     0,   989,   988,     0,     0,     0,     0,    34,
       0,     0,    85,   749,   748,   766,   767,   768,   245,   763,
     764,   757,   957,   758,   761,   765,   762,   759,   760,   754,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,    53,  1089,  1090,  1091,  1092,  1086,
    1087,  1088,   755,  1338,  1339,  1340,  1341,  1342,  1343,  1344,
    1345,   756,     0,   242,   243,     0,    33,   226,     0,    21,
     228,   438,   438,   438,   438,   438,   438,   438,   438,     0,
     438,   438,   438,   438,    16,   231,    39,   232,   443,   440,
     441,   438,   438,   438,    13,   878,   851,   853,    70,    69,
     446,   448,     0,     0,     0,   465,   464,   466,     0,   621,
       0,   739,   740,   741,     0,   948,   949,   950,   518,   519,
     953,     0,     0,     0,     0,   536,   541,   548,     0,   578,
     599,   611,   612,   688,   694,   715,     0,     0,   993,     0,
       7,    92,   471,   473,   484,   485,   486,   487,   488,   516,
     498,   474,    61,   268,   513,   494,   522,     0,    12,    13,
     534,   542,    14,    59,   546,   583,    36,   573,    44,    46,
     602,    40,     0,    54,    60,   619,    40,   687,    40,   693,
      18,    24,   504,    45,   713,   510,     0,   511,   496,   495,
     786,   789,   791,   793,   795,   796,   803,   805,     0,   804,
     746,   521,   957,   499,   505,   497,   754,   514,    62,     0,
       0,    65,   457,     0,     0,     0,    91,   451,     0,    95,
     306,   305,     0,   454,     0,     0,     0,   630,   112,   114,
     290,     0,   313,   343,   438,   438,   438,    13,   878,   851,
     853,     0,    60,     0,   136,   137,   138,     0,   130,   131,
     139,     0,   132,   133,   141,   142,     0,   134,   135,     0,
     143,     0,   145,   146,   855,   856,   854,   438,    13,    36,
      44,    51,     0,    60,   201,   470,   203,   174,   175,   176,
     177,   438,   172,   178,   470,   171,   173,   179,   198,   197,
     196,   190,   438,   469,   194,   193,   195,   855,   856,   857,
     438,     0,    13,   180,   182,   184,   188,    36,    44,    51,
     186,    78,   213,   438,   210,   171,   211,   209,   215,     0,
     216,    13,   205,   207,    44,    79,   217,   218,   219,   220,
     223,   438,   222,     0,  1098,  1095,  1096,    56,     0,   775,
     776,   777,   778,   779,   781,     0,   998,  1000,     0,   999,
      52,     0,     0,  1336,  1337,    56,  1100,  1101,    55,    20,
      55,  1104,  1105,  1106,  1107,    30,     0,     0,  1110,  1111,
    1112,  1113,  1114,     9,  1132,  1133,  1127,  1122,  1123,  1124,
    1125,  1126,  1128,  1129,  1130,  1131,     0,    28,    55,  1147,
    1146,  1145,  1148,  1149,  1150,    31,    55,  1153,  1154,  1155,
      32,  1164,  1165,  1157,  1158,  1159,  1161,  1160,  1162,  1163,
      29,  1176,    55,  1172,  1169,  1168,  1173,  1171,  1170,  1174,
    1175,    31,  1179,  1182,  1178,  1180,  1181,     8,  1185,  1184,
      19,  1187,  1188,  1189,    11,  1193,  1194,  1191,  1192,    57,
    1199,  1196,  1197,  1198,    58,  1246,  1240,  1243,  1244,  1238,
    1239,  1241,  1242,  1245,  1247,     0,  1200,    55,  1280,  1281,
       0,    15,  1226,  1225,  1218,  1219,  1220,  1204,  1205,  1206,
    1207,  1208,  1209,  1210,  1211,  1212,  1213,    53,  1222,  1221,
    1224,  1223,  1215,  1216,  1217,  1233,  1235,  1234,     0,    25,
       0,  1230,  1229,  1228,  1227,  1334,  1331,  1332,     0,  1333,
      49,    55,    28,  1351,  1025,    29,  1350,  1353,  1023,  1024,
      34,     0,    48,    48,     0,    48,  1357,    48,  1360,  1359,
    1361,     0,    48,  1348,  1347,    27,  1369,  1366,  1364,  1365,
    1367,  1368,    23,  1372,  1371,    17,    55,  1375,  1378,  1374,
    1377,    38,    37,   961,   962,   963,    51,   964,    34,    37,
     959,   960,  1040,  1041,  1047,  1033,  1034,  1032,  1042,  1043,
    1063,  1036,  1045,  1038,  1039,  1044,  1035,  1037,  1030,  1031,
    1061,  1060,  1062,    51,     0,    12,  1048,  1004,  1003,     0,
     803,     0,     0,    48,    27,    23,    17,    38,  1379,  1008,
    1009,   986,  1007,     0,   747,  1085,   225,   244,    82,   227,
      83,    60,   154,   155,   132,   156,   157,     0,   158,   160,
     161,   438,    13,    36,    44,    51,    86,    84,   233,   234,
     236,   235,   238,   239,   237,   240,   875,   875,   875,    97,
       0,     0,   573,     0,   460,   461,   462,     0,     0,     0,
       0,     0,   955,   954,   951,   952,     0,     0,     0,    37,
      37,     0,   530,     0,     0,    12,     0,     0,   567,     0,
       0,     0,     0,     0,     0,     0,     6,     0,     0,   807,
       0,   472,   475,   515,     0,   532,     0,   531,   492,   489,
       0,     0,   535,   537,     0,   543,   493,   500,     0,     0,
       0,     0,     0,     0,   547,   549,   581,   582,   568,     0,
     490,   571,   572,     0,   579,   491,   501,     0,   598,   502,
       0,    47,    16,     0,     0,    20,    30,     9,    28,   916,
      29,     0,   921,   919,   920,    14,     0,    40,    40,   906,
     907,     0,   649,   652,   654,   656,   658,   659,   664,   669,
     667,   668,   670,   672,   610,   635,   636,   646,   647,   908,
     637,   644,   638,   645,   641,   642,     0,   639,   640,     0,
     671,   643,   503,   512,     0,     0,   627,   626,   620,   622,
     506,     0,   706,   707,   708,   686,   691,   704,   507,     0,
     692,   697,   508,   509,   709,     0,   731,   732,   714,   716,
     719,   729,     0,   751,     0,   750,     0,     0,     0,     0,
       0,     0,     0,     0,   941,   942,   943,   944,   945,   946,
     947,    20,    30,     9,    28,    31,   933,    29,    31,     8,
      19,    11,    57,    58,    53,    15,    25,    49,    40,     0,
     923,   891,   924,   800,   801,   903,   890,   880,   879,   895,
     897,   899,   901,   902,   889,   925,   926,   892,     0,     0,
       0,     0,     7,     0,   845,   844,   902,     0,     0,   399,
      60,   252,   269,   293,   325,   344,   467,   111,     0,     0,
       0,     0,   118,     0,     0,   875,   875,   875,   120,     0,
       0,   573,     0,   129,   153,     0,     0,     0,     0,     0,
     144,     0,     0,   875,   148,   151,   149,   152,   169,   189,
       0,   204,   170,   192,   191,    12,   438,   181,   185,   183,
     187,   212,   214,   206,   208,   221,   224,  1097,     0,     0,
     774,    55,   771,   772,    22,     0,   996,   785,    42,    42,
    1335,  1102,  1099,  1108,  1103,    20,    28,    20,    28,  1109,
    1134,  1135,  1136,  1137,    28,  1119,  1144,  1143,  1152,  1151,
    1156,  1167,  1166,  1177,  1183,  1186,  1190,  1195,    10,  1264,
    1270,  1268,  1259,  1260,  1263,  1265,  1254,  1255,  1256,  1257,
    1258,  1266,  1261,  1262,  1267,  1202,  1269,  1201,  1282,    15,
    1278,  1214,  1232,  1231,  1236,  1286,  1283,  1284,  1285,  1287,
    1288,  1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,  1297,
    1298,  1299,  1300,  1317,    50,  1329,  1352,  1019,  1020,  1026,
      48,  1021,  1349,     0,  1354,  1356,     0,  1358,  1346,  1363,
    1370,  1376,  1373,   958,   965,   956,  1046,  1049,  1050,     0,
    1052,     0,  1051,  1053,  1054,    12,    12,  1055,  1027,     0,
       0,  1001,  1381,  1380,  1382,  1383,  1384,     0,     0,   769,
     168,   159,     0,   875,   163,   166,   164,   167,   229,   241,
       0,     0,     0,     0,   364,    13,   583,   389,    36,   369,
       0,    44,   394,   852,    51,     0,    28,    29,   613,     0,
      60,     0,   733,   735,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1055,     0,    13,    36,    44,     0,   728,
      45,   723,   722,     0,   727,   725,   726,     0,   700,   702,
       0,   517,   820,     7,     7,   822,   816,   819,   902,   841,
       7,   806,   468,   278,   533,   539,   540,   538,   435,   544,
     545,   562,    20,     0,     0,   560,   556,   551,   552,   553,
     554,   557,   555,   550,     0,   584,   587,     0,     0,     0,
       0,    53,     0,   678,   917,   918,     0,   673,     0,   909,
     912,   913,   910,   911,   922,     0,   915,   914,     0,   635,
     644,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   680,     0,     0,     0,     0,     0,     0,   624,   625,
     623,     0,     0,     0,   695,   718,   723,   722,   717,     0,
      10,     0,   787,   788,   790,   792,   794,   797,     7,   523,
     525,   802,   910,   932,   911,   934,   931,   930,   936,   928,
     929,   927,   937,   935,   938,   939,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   886,   885,
     902,   991,  1070,   847,   846,    63,     0,    64,     0,     0,
     109,     0,     0,     0,     0,     0,    60,   252,   269,   293,
     325,   344,     0,     0,     0,    13,    36,    44,    51,   458,
     447,   449,   269,   452,   455,   344,    12,   202,   199,    12,
       0,   780,   773,   770,    52,   782,   783,  1115,  1117,  1116,
    1118,    55,  1139,  1141,  1140,  1142,  1121,    28,     0,  1276,
    1248,  1273,  1250,  1277,  1253,  1274,  1275,  1251,  1271,  1272,
    1249,  1252,  1279,  1314,  1313,  1315,  1316,  1322,  1304,  1305,
    1306,  1307,  1319,  1308,  1309,  1310,  1311,  1312,  1320,  1321,
    1323,  1324,  1325,  1326,  1327,  1328,    55,  1303,  1302,  1318,
      49,  1022,     0,     0,    28,    28,    29,    29,  1028,  1029,
    1001,  1001,     0,    26,  1006,  1010,  1011,    34,     0,   344,
      12,   379,   384,   374,     0,     0,    98,     0,     0,   105,
       0,     0,   100,     0,   107,   615,     0,     0,   614,   481,
     736,     0,     0,   828,   734,   823,  1264,  1268,  1263,  1267,
    1269,    53,    10,    10,     0,   815,     0,   813,    37,    37,
      12,    12,     0,    12,   478,   479,   480,     0,   710,     0,
       0,     0,     0,   810,     0,   811,     0,   536,     0,   578,
      12,    13,    14,    36,    44,   821,   831,     0,     0,   840,
     817,   829,   809,   808,     0,   561,    28,   565,   566,    53,
     564,     0,   589,   591,     0,   569,     0,   570,     0,   575,
     574,   576,   600,     0,   604,   601,     0,   606,     0,     0,
     608,   679,     0,   683,   685,   648,   650,   651,   653,   655,
     657,   665,   666,   660,   663,   662,   661,   675,   674,     0,
       0,     0,  1064,  1065,  1066,  1067,   689,   705,   696,   698,
     730,   752,     0,     0,     0,   526,   940,   888,   882,     0,
     893,   894,   896,   898,   900,   887,   798,   881,   799,   904,
     905,     0,     0,   798,     0,     0,    60,   401,   400,   403,
     402,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    88,   254,   253,     0,   248,     0,     0,    55,
       0,    90,   271,   270,     0,     0,     0,     0,     0,     0,
     302,     0,     0,     0,     0,     0,     0,     0,    93,   295,
     294,     0,     0,     0,     0,   476,     0,     0,     0,     0,
       0,     0,    94,   327,   326,     0,     0,     0,     0,     0,
       0,    13,    96,   346,   345,   128,     0,     0,     0,     0,
       0,   379,   384,   374,   121,   126,   122,   127,     0,     0,
     150,   200,     0,   997,  1138,  1120,     0,  1301,  1330,  1355,
    1362,  1056,  1057,  1058,  1059,    41,     0,    26,  1002,  1018,
    1014,  1013,  1012,    34,     0,   165,     0,     0,     0,     0,
      13,   366,   365,   368,   367,   584,   587,    36,   391,   390,
     393,   392,    44,   371,   370,   373,   372,   576,    51,   396,
     395,   398,   397,   616,   618,     0,   826,   827,   824,  1237,
     995,   994,     0,     0,   814,   992,   990,   482,   483,    12,
    1068,   724,   720,   721,    45,    45,   701,     0,     0,     0,
       7,   842,   843,   835,   833,   837,   834,   836,   832,   825,
     838,   839,   818,   830,   520,   279,     0,   559,     0,   558,
     593,   594,   595,   596,   597,   586,     0,     0,   588,   590,
       0,   580,    55,    55,    47,    55,   676,     0,   682,     0,
     684,   690,   699,     0,   737,     0,     0,     7,   524,   884,
     883,   631,     0,   110,   459,   363,   450,   268,   453,   290,
     313,   456,   343,   251,   247,   249,     0,     0,     0,   363,
     363,   363,   363,   255,     0,   428,   429,    43,   427,   426,
       0,   424,   272,   274,   273,   277,   275,   288,   291,   287,
       0,     0,     0,   342,   341,    43,   340,   404,   406,   407,
     405,   360,   408,   361,    55,   359,   309,   310,   312,   311,
       0,   308,   296,   303,   304,   300,   477,     0,     0,     0,
       0,     0,     0,   338,   337,   335,     0,     0,     0,   349,
     113,   115,   116,   117,   119,     0,     0,     0,   140,   147,
    1093,    10,     0,  1005,  1001,  1017,  1015,   162,     0,     0,
       0,    12,   381,   380,   383,   382,     0,     0,     0,     0,
       0,    12,   386,   385,   388,   387,     0,     0,    12,   376,
     375,   378,   377,   848,    99,   876,   877,   106,   101,   850,
     108,   617,     0,   744,  1069,   711,   712,   703,   812,   280,
     563,   585,   592,     0,    31,    31,   609,   607,   677,     0,
       0,     0,   738,     0,   528,   527,   632,   633,     0,     0,
       0,     0,     0,     0,     0,   363,   363,   363,     0,     0,
       0,     0,     0,     0,   352,   425,     0,     0,   298,   299,
     301,   339,     0,   292,     0,   297,     0,   328,   329,   336,
     324,   334,     0,     0,     0,   350,    12,    12,    12,  1094,
       0,    26,     0,    57,    53,     0,     0,   103,     0,     0,
       0,     0,     0,   104,     0,     0,   102,     0,   577,   603,
     605,   681,   742,     0,     7,   634,   628,   631,   399,   269,
     293,   325,   344,   250,     0,     0,     0,   364,   389,   369,
     394,   357,   356,     0,   353,   358,   276,     0,   289,   362,
     307,    60,     0,   286,     0,    13,    36,    44,    51,   285,
     284,     0,     0,   321,     0,   281,   323,     0,     0,     0,
     418,   412,   411,   415,   410,   413,   414,   347,   348,   124,
     125,   123,     0,  1016,     0,   861,   860,   867,   869,   872,
     873,   870,   871,   874,     0,   863,   745,   743,   529,   633,
       0,     0,     0,     0,     0,   379,   384,   374,     0,     0,
       0,     0,     7,   351,   423,   320,   283,   314,   315,    12,
     316,   318,   317,   319,   331,     0,   333,    55,     0,   419,
       0,     0,  1203,     0,     0,   864,     0,   629,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     355,   354,     0,     0,     0,   322,   282,   332,    55,   417,
     416,     0,     0,     0,    55,    60,   256,   257,   258,   259,
       0,     0,     0,    13,    36,    44,    51,   420,   421,     0,
     409,   330,   436,   437,   866,   865,    43,   862,   267,    12,
      12,    12,   260,   265,   261,   266,   422,   868,   263,   264,
     262
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1846, -1846,    -1, -1253,   954,    64, -1334,   956,  -360,  -368,
    -866,  -790,  1062,  1190, -1846,   958,  -509, -1846, -1846,  1201,
   -1846,  -128, -1654,  1203,    -6,   -27,  1423,  -595, -1846, -1846,
    -683, -1846,  -430,  -707,  1195, -1846,  -292, -1846,   870, -1808,
    -436, -1186, -1846,  -871,  -551,  -936, -1846,  -503,   506,  -660,
   -1846,  -492,  1436,  -991,   982, -1846,  -443, -1846,    12, -1846,
   -1846,  1996, -1846, -1846, -1846,  1736,  1744,   174,  1637, -1846,
   -1846, -1846,  1886,  2353, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846,    21, -1846,  1590, -1846,
   -1846,   -73,   565,  -353, -1394, -1846, -1846, -1846, -1845, -1637,
    -422, -1846, -1401,  -378,   252,   -89,  -416, -1846,   -94, -1846,
   -1846, -1395, -1397, -1846,  -429, -1405, -1837, -1846,  -212,   -80,
   -1632,  -284,  -136,  -134, -1690, -1683, -1679,  -135,  -127,  -122,
   -1846, -1846,  -237, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846,   133,  -754, -1434, -1846,   314,   -92,  3210, -1846,   238,
   -1846, -1846,  1059, -1846,   324,   699,  1771, -1846,   768, -1846,
    -918,  1625, -1846, -1846,   251,   308,   756,   126,   -41, -1846,
   -1846, -1399, -1846, -1846, -1846, -1846, -1846, -1846, -1327,  -350,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846,   128, -1846, -1228,
   -1846, -1209, -1846, -1846,  1171, -1846, -1208, -1846, -1846, -1846,
   -1846,  1164, -1846, -1846, -1846,   159, -1846, -1200, -1846,  1689,
   -1474,   214, -1846, -1187, -1846,   797,   239,   508,   240,   511,
     433,   442, -1846, -1846,  -904, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846,  1123,  -339,  1655,   -65,  -133,  -716,   720,   721,
     719, -1846,  -829, -1846, -1846, -1846, -1846, -1846, -1846,  1515,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  -359,
     705, -1846, -1846, -1846, -1846,  1115,   494,  -900,   496,  1246,
     710, -1328, -1846, -1846,  1800, -1846, -1846, -1846,   -63,  1636,
    -853,  -334,  2112, -1528,  2091,    -8, -1846, -1846,   994,    81,
   -1846, -1846, -1846, -1846, -1846,  -162,  -210, -1846, -1846,   677,
    -806,  1911,   -61, -1846,   800,   256, -1846, -1560, -1846, -1846,
     527, -1846, -1411, -1846,   495, -1447,   512, -1846, -1846,  1721,
    -614,  1699,  -606,  1707,  -609,  1708,    95, -1846, -1675, -1846,
   -1846,  -141, -1846, -1846,  -608,  -594,  1714, -1846,  -387,  -318,
    -881,  -876,  -875, -1846,  -347,  -839, -1846,  -327,     5,  -809,
   -1846, -1516,  -325,   198,  1833, -1846,    50, -1846,   226, -1846,
   -1391, -1846,   253, -1846, -1846, -1846, -1846, -1846,   254,  -262,
     332,  1446,   557,  1835,  1837, -1846, -1846,  -494,    90, -1846,
   -1846, -1846,   749,   -44, -1846, -1846,   -58, -1846,   -49, -1846,
      -9, -1846,   -75, -1846, -1846, -1846,   -48, -1846,   -30, -1846,
     -25, -1846,     9, -1846,    10, -1846,    16, -1846,    25, -1846,
      29, -1846,    30, -1846,    34,  1475, -1846,   -81, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,  1516, -1072,
   -1846, -1846, -1846, -1846, -1846,    38, -1846, -1846, -1846, -1846,
     976, -1846, -1846,    43, -1846,    44, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846, -1846,
   -1846, -1846, -1846, -1846, -1846, -1846
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,  1793,   877,  1163,  1371,  1508,  1165,  1248,   839,
     896,  1190,   826,  1230,   992,  1164,  1369,   810,  1493,  1229,
     993,   698,  1829,  1228,  1422,  1424,  1370,  1157,  1159,   808,
     801,   521,   910,  1233,  1232,   827,   919,  2033,  1495,  2115,
     915,   994,   916,   535,  1221,  1215,  1550,  1107,  1136,   805,
     972,  1141,  1127,  1166,  1167,   897,   973,   883,  1794,    46,
      47,    48,    74,    87,    89,   456,   460,   465,   452,   103,
     294,   105,   492,   493,   132,  1111,   484,   142,   150,   152,
     276,   279,   296,   297,   835,  1268,   277,   222,   424,  1745,
    1954,  1955,  1461,   425,  1462,  1644,  2200,  2201,  1977,  1978,
     428,  1771,  1463,   429,  2000,  2001,   432,  2259,  2203,  2204,
    2208,  1464,  1772,  1985,   434,  1465,  2113,  2183,  2184,  1993,
    1994,  2099,  1575,  1580,  1838,  1836,  1837,  1578,  1583,  1459,
    1995,  1754,  2133,  2213,  2214,  2215,  2294,  1755,  1756,  1967,
    1968,  1944,   223,  1340,  2324,    49,    50,    61,   459,    52,
     463,  1947,   467,   468,  1949,    71,   473,  1952,   454,   455,
    1945,   315,   316,   317,    53,   436,  1593,   496,  1758,   352,
     353,  1774,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,  1419,  1704,  1705,   367,   368,
     887,   369,   370,   892,   893,   371,   372,   895,   373,   374,
     904,   905,  1352,  1346,  1649,  1650,   375,   376,   908,  1280,
    1661,  1921,   377,   378,   914,   909,  1655,  1355,  1657,  1356,
    1357,  1915,   379,   380,  1664,   918,   381,   382,   383,   384,
     385,   978,   979,  1730,   423,  2097,  2166,   942,   943,   944,
     945,   946,   947,   948,  1684,   949,   950,   951,   952,   953,
     954,   386,   387,   985,   388,   389,   990,   390,   391,   986,
     987,   392,   393,   394,   998,   999,  1308,  1309,  1310,  1000,
    1001,  1291,  1292,   395,   396,   397,   398,   399,   224,   955,
    1004,  1041,   956,   227,   400,   957,  1134,   554,   555,   958,
     562,   401,   402,   403,   404,   405,   406,  1043,  1044,  1045,
     407,   408,   409,   878,   879,  1606,  1607,  1324,  1325,  1326,
    1594,  1900,  1595,  1640,  1635,  1636,  1641,  1327,  1890,  1063,
    1844,   840,  1856,   842,  1862,   843,   477,   511,  2145,  2062,
    2274,  2275,  2045,  2055,  1270,  1851,   841,   410,  1064,  1065,
    1049,  1050,  1051,  1052,  1328,  1054,   959,   960,   961,  1057,
    1058,   411,   854,   962,   759,   760,   232,   413,   963,   560,
    1563,   789,   964,  1258,   802,  1567,  1833,   235,   965,   721,
     967,   722,   968,   784,   785,  1245,  1246,   786,   969,   970,
     414,   415,   971,  2031,   240,   548,   241,   569,   242,   575,
     243,   583,   244,   597,  1154,  1507,   245,   605,   246,   610,
     247,   620,   248,   631,   249,   637,   250,   640,   251,   644,
     252,   649,   253,   654,   254,   688,   689,   690,   255,   691,
     256,   704,   257,   699,   258,   417,   666,  1185,  1600,  1187,
    1520,  1512,  1517,  1510,  1514,   259,   671,  1213,  1549,  1532,
    1538,  1527,  1214,   260,   710,   261,   565,   262,   263,   735,
     264,   723,   265,   725,   266,   727,   267,   732,   268,   742,
     269,   745,   270,   751,   271,   798
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      44,   889,    54,   882,   855,   430,   790,   431,   888,  1094,
     884,   300,   351,    45,  1084,   485,  1120,   485,  1081,   485,
     494,  1047,   229,   494,   426,   485,  1083,  1191,   485,   494,
     991,   299,   494,   494,   494,   494,  1162,  1219,  1329,   494,
    1109,  1429,   494,  1102,  1106,   229,   309,   494,   494,  1105,
    1364,   700,  1235,   280,   418,   485,   848,   229,   803,   421,
    1142,  1053,  1005,  1082,  1314,  1787,  1800,  1785,  1798,  1374,
    1623,  1625,  1066,  1066,  1797,  1799,  1701,  1642,  1143,  1088,
    1809,  1055,  1119,  1702,  1070,   874,  1080,  1118,  1808,  1283,
    1048,  1100,  1055,  1055,   980,  1405,   988,  1630,  1124,   703,
     298,  1434,   677,   412,  1108,   677,  1156,  1857,  1892,  1098,
    1104,   230,  1378,  2027,  1158,   412,  1631,  1632,  2025,   674,
     966,  1321,   674,  2026,  1618,  1633,  1332,  1414,   675,   678,
    1161,   675,   678,  1415,   230,  1416,  1775,  1775,  1634,  2007,
      58,    59,    60,  1936,  1117,  1347,   230,   679,  2011,  1351,
     679,  1122,   680,  1235,   876,   680,  1330,  1866,  1867,  1420,
     876,  1937,  1002,  1123,  1834,  1703,  2252,  1002,   676,  1825,
    1827,   676,  1222,  2035,  1224,  1188,  1225,  2121,  2302,  1068,
    2134,  1227,   310,  1868,   311,  1297,   681,   682,  1898,   681,
     682,  1002,  1591,   683,   422,  1592,   683,  1412,  1413,   427,
     272,   157,   684,   433,  1901,   684,   685,   686,   229,   685,
     686,   687,   229,  2209,   687,   692,    56,   510,   692,  1216,
     693,   694,  1601,   693,   694,  1409,  1275,  1651,   231,  1271,
    1272,  1979,  1138,   486,  1284,   488,  1281,   490,  1637,   885,
     761,  1638,  1252,   501,   870,  1432,   505,  1278,  1979,  1276,
      64,   231,   981,  1234,  1231,   312,   233,    56,  2181,  2182,
     527,  2181,  2182,   231,    65,   853,   853,  -748,  1870,  1871,
     470,   278,  2188,   540,  -748,  -748,  -748,  -748,    55,   233,
    1236,  -720,    73,   234,   236,  1965,  1966,  1095,   638,  1659,
    1247,   233,   733,   806,    88,  2210,  2211,   230,  -439,   809,
     856,   230,  1277,  1935,  1619,   639,   234,   236,   158,   159,
     160,   435,  1652,  1338,  -748,   439,  1096,   734,   234,   236,
     104,   743,  1267,   871,   158,   159,   160,  1980,  1981,  1097,
     462,   435,  1099,  1003,    56,   989,   295,   857,  1283,   451,
    1011,  -753,   867,  2008,  2009,   229,   744,  1046,   489,  -753,
    -753,  -753,  -753,   151,   804,   419,  1359,  -761,  1046,  1046,
     868,   420,   237,    56,  -761,  -761,  -761,  -761,  1260,  1360,
    1303,   435,   526,  2150,  1431,  1775,    56,  -246,   430,    56,
     431,  1145,   537,   761,   821,   237,  1361,  1013,  1266,   543,
    1262,   541,  1130,  1265,  2082,  1077,  1146,   237,  1139,  1522,
    1787,   464,  1785,   412,  -761,   457,   430,  1630,   431,    57,
    1093,   485,  1630,  1056,   231,  -268,  1437,  1101,   231,   430,
    2188,   431,  1147,   606,  1056,  1056,  1631,  1632,   229,  1438,
    1423,  1631,  1632,  1425,   230,  1633,  1816,  1148,  1590,  1261,
    1633,   522,   233,   229,   607,   561,   233,  2006,  1634,   886,
      57,   880,  1298,  1634,  1264,  1898,  1667,   608,  1670,   816,
    2006,   852,   852,  1868,    56,   858,   430,   221,   431,   234,
     236,  1775,  1775,   234,   236,  1475,  1131,  1472,  1473,  1474,
    1660,   221,  2336,  1478,     3,  1477,   412,  2223,     4,  2202,
       5,  -439,   273,   274,   275,  1486,  1476,   880,   982,   983,
     984,   412,     3,  1653,  1418,  1069,     4,  1339,     5,   221,
       6,     7,     8,     9,   221,  1331,  1454,   230,  2337,  1647,
    1938,   814,   435,   221,  1648,  2253,   221,    57,     6,     7,
       8,     9,   230,   350,  1936,   724,  1685,  2303,   221,   507,
    1362,  1602,  1603,   726,  1788,   313,   314,  1789,   237,   792,
    1055,   231,   237,  2212,  1620,  2256,    57,  2285,  1682,  2276,
    1712,   221,  2283,   273,   274,   275,  1713,  2284,  1714,    57,
     844,  1193,    57,   221,     3,   221,  2268,  2164,     4,   233,
       5,   982,   983,   984,  1282,  1653,   422,   238,  -573,  1979,
     310,  1155,   815,  1160,   311,  1391,   457,  1388,  2205,   427,
       6,     7,     8,     9,   433,  2090,   234,   236,  2006,  2158,
     238,  1437,  2148,  1068,  1818,   609,   556,   557,   558,  1194,
    1132,  1133,   238,   677,  1721,   677,  -849,  1460,  -748,  1114,
    1710,  1711,  1562,   807,   231,  1826,  1497,  2078,  1499,  1492,
    1293,   845,   674,  2141,  1389,  1376,  1377,  1149,   350,   231,
     678,   675,   678,   221,     4,  1570,     5,    57,   311,   846,
    1313,  1392,   233,  1393,  1979,  1676,  1677,  1790,   679,  1551,
     679,  1448,  1683,   680,  1630,   680,  1126,   233,  1005,  1055,
    1055,  1055,  1055,  1055,  2094,   237,  1055,  1273,  1218,   234,
     236,   676,  1436,  1631,  1632,  1421,  -849,  2139,  2075,  2076,
     849,  1668,  1633,  -753,   234,   236,  1217,   681,   682,   681,
     682,  1450,  1055,   851,   683,  1634,   683,     4,  -761,     5,
    1885,   321,   322,   684,   301,   684,   323,   685,   686,   685,
     686,  1055,   687,   508,   687,   476,   692,  2296,   692,  1466,
    1449,   693,   694,   693,   694,   866,  1435,   712,  1637,   911,
     912,  1638,   509,  1724,   430,  1488,   431,  1274,   869,  1922,
     714,  1121,  1923,  2163,   712,  1925,  1895,  2244,   237,  2242,
     872,   715, -1019,   238, -1019,  2241,  2243,   238,    62,   239,
     443,  1584,  1565,   237,   714,  1891,   133,   134,   715,   135,
     136,   137,   444,   974,   975,  1791,   472,   875,   911,   912,
    1279,   544,   416,  1329,   545,   546,   547,  1311,   304,   305,
     306,   307,   728,  1314,   416,   500,  1323,   876,  -767,  1394,
    1959,  1960,  1961,  1962,   880,  -767,  -767,  -767,  -767,  1283,
     787,     3,  -721,  1645,   880,     4,   474,     5,   559,  1956,
     788,   894,   229,   437,  1787,  1582,  1785,  1589,  1579,  1417,
    1852,   447,   448,   449,   450,  1546,  1269,     6,     7,     8,
       9, -1020,   229, -1020,  1329,  -767,  2126,  2323,  1792,  2117,
    -762,  1616,  1547,  2191,   483,   917,  1615,  -762,  -762,  -762,
    -762,  1395,  1056,  1008,  1830,  1558,  1559,  1009,   847,   718,
     719,   512,   729,   730,   731,  2112,   517,   518,   519,  2143,
     221,  1875,  1876,  1373,  2144,  1548,   531,  1576,  1433,   534,
     238,  2238,  1010,   718,   719,  1775,  1775,  -762,   302,     4,
     303,     5,  1372,  2206,   880,  1059,  1341,   695,  1626,   712,
   -1019,   230,  1406,  1060,   696,  1626,   873,  1614,   502,   503,
    1726,  1869,   976,  1627,  1071,   977,  1380,  1011,   336,  1003,
    1627,   230,  1046,   715,     3,   336,  1072,  1005,     4,  1342,
       5,  1628,  1073,  1012,   697,   239,  1629,  2130,  1628,   239,
    2122,   720,  1074,  1629,  1343,  1807,  1386,   712,  1387,  1566,
       6,     7,     8,     9,   563,   564,   445,   819,   446,  1909,
     799,   800, -1019,   238,  1013,   720,  1568,  1046,     3,  2290,
    1075,   715,     4,   229,     5,   890,   891,   880,   238,  1814,
     641,  1056,  1056,  1056,  1056,  1056,   549,   550,  1056, -1020,
    1344,   642,   643,  1795,     6,     7,     8,     9,   312,   138,
     475,   667,   476,  1957,  1444,  1958,   139,   140,  2105,  2106,
    2107,  1806,  -849,   746,  1056,  1699,  1805,  1445,   231,  1446,
    -343,  1708,   141,  2086,  1817,  1005,  1110,   747,  1717,   158,
     159,   160,  1455,  1056,   748,  1444,  1457,  1129,   231,   836,
     837,   838,   308,    67,    68,    69,   233,     4,  1456,     5,
    1446, -1020,   749,  -858,  2270,   668,   669,  2271,  1605,   750,
    1707,  1709,   230,  2038,   670,  -859,   233,  1715,  1709,  1718,
    1720,  1116,   416,   234,   236,   906,   907,  1804,   881,  1345,
    1055,  1055,  1055,  1055,  1055,  1055,  1055,  1055,  1055,  1055,
    1055,  1444,  1128,  1315,  1316,  1456,  1810,  1446,  2195,  1811,
    1987,   911,   912,  2039,  1722,  1410,  1446,   440,   441,   442,
    1498,     3,  1500,  1276,  1135,     4,  2196,     5,  1506,  1137,
    2036,  2197,  2225,  2198,   469,  1988,  1989,   650,   551,   552,
       3,  1189,   553,   482,     4,  1223,     5,     6,     7,     8,
       9,  1523,   497,   498,  1226,  1524,  1525,  1526,   504,  -767,
     898,   880,   237,  1006,  1007,   416,     6,     7,     8,     9,
     651,   652,  1933,   653,   528,   529,   530,  1847,  1897,  1251,
     416,  1293,   237,  1896,   538,   539,  1249,   491,   542,   231,
    1835,  1250,  1085,  1086,  1087,   996,   997,   736,  1501,  1257,
       3,  1259,    18,    19,     4,     4,     5,     5,  1003,  2122,
    1289,  -762,  2046,   737,  1294,  1502,  1295,   233,  1296,  2047,
    1503,  1504,  1381,  1382,  2091,  1103,     6,     7,     8,     9,
    1877,  1878,  1299,  1880,   738,  1329,  1873,  1970,  1300,  1068,
    1301,   739,  1302,  1894,   234,   236,  -438,     3,   157,  1662,
    1893,     4,    35,     5,  1320,  2056,  2048,  2040,  1115,  1663,
    1990,   645,    36,  1943,     4,   646,     5,  2124,    37,  1304,
     229,  1439,  1440,     6,     7,     8,     9,  1055,  1554,  1555,
    1991,  1992,  1333,     3,   880,   157,  1662,     4,  1759,     5,
    1334,  2049,    38,  1515,  1516,  1055,  1666,  1368,   647,   648,
    1858,   899,   900,   901,   902,   903,  1003,  1335,  1686,     6,
       7,     8,     9,  1336,  1939,  1940,   705,  1931,  1348,  1932,
    1375,   157,  1662,   237,  1383,   817,   818,  1505,   880,  1349,
     740,   741,  1669,  1556,  1557,  2070,  1350,  1760,  1761,  1762,
    1763,  1354,  1764,  1358,  2041,  1765,   566,   567,   568,   706,
     707,   143,  1384,   144,  1709,  1709,  1055,  1385,   145,   230,
     146,  1390,  1766,  1398,   147, -1064,  1380,  1380,  1380,  1380,
    1380, -1065,  1380,  1380,  1055,  1055,  1533,  1534,  1535,  1536,
    1537,  1693,  1693,   229,  1396,   708,  1397,   238,     3,  1313,
    1518,  1519,     4,  1399,     5,  1401,  2068,  2067,   876,  1624,
    2050,  1831,  1832,  2019,   148,  1402,   149,   238,    18,    19,
    2084,  2085,  1767,  2087,     6,     7,     8,     9,   709,  2057,
     157,  1662,  1056,  1056,  1056,  1056,  1056,  1056,  1056,  1056,
    1056,  1056,  1056,  1409,   229,  1983,  1984,  1725,  1727,  -438,
    1743,  1752,  1769,  1783,  1411,  1626,   491,  1441,   523,  2292,
    2293,  1728,  2064,  1744,  1753,  1770,  1784,  1442,    35,  1443,
    1627,    18,    19,  1447,  2226,   336,  1694,  1695,    36,  2159,
    2160,   880,   230,  1453,    37,   793,   231,  1283,  1628,  2322,
    1338,  1815,  2123,  1629,   794,   795,  1437,  2051,   796,   797,
     880,  1458,   412,   158,   159,   160,  1068,  1626,    38,  2074,
     995,   996,   997,  2103,   233,  -438,  2058,  2101,  1467,  1823,
    1824,    35,  1627,  1273,  2102,  1468,  1469,   336,  2317,  1470,
    1471,    36,  1276,   230,  1479,  1626,  1480,    37,  1821,  1822,
    1628,   234,   236,  1934,  1882,  1629,  1311,  1406,  1481,  1482,
    1627,  1113,  1483,  1485,   855,   336,  1487,  1484,   238,  1490,
     880,    38,  1494,  1511,  1841,  1521,  1509,  1848,  1628,  1853,
    1513,  1323,  1859,  1629,  1540,  1541,  2278,  1842,  1542,  1263,
    1849,   632,  1854,  1543,  2100,  1860,  1552,  1544,   229,   239,
    1125,  1545,  1553,  1906,  2120,  2092,  1560,   633,   828,   231,
     829,   229,   830,   831,  1561,  1276,   634,   880,  1562,   239,
     832,   833,  1564,   635,   636,  1150,  1151,  1152,  1153,  1056,
     237,  1569,  2129,  1768,  1571,  1572,   229,   233,  1573,  1574,
    1907,  1581,  1011,  1905,     3,  1586,  1604,  1056,     4,  1612,
       5,  1617,  1621,   880,   834,  1654,   412,  1996,  1997,  1998,
     231,  1999,  1605,  1646,   234,   236,   225,  1658,  1656,  2287,
       6,     7,     8,     9,  1672,  2108,  2109,  2110,  2111,  1675,
    1681,  2147,   273,   274,   275,  1969,  1689,   230,   233,   225,
    1690,  2153,   611,   612,   412,  2263,  1691,  -720,  2156,  -721,
     230,   225,  1910,  1911,  1912,  1913,  1914,   613,  1056,   614,
     615,   616,   158,   159,   160,   234,   236,  1706,  -438,   982,
     983,   984,  2157,  1293,  1723,   230,  1056,  1056,  1528,  1529,
    1530,  1531,   617,   618,   619,  2046,   478,   479,   480,   481,
    1801,  1812,  2047,   237,  1963,  1802,  1819,  1828,  2255,  1626,
    1803,   229,  1839,  1972,  1973,  1974,  1975,  1976,   880,  1865,
     239,  2262,  1820,  1338,  1627,  1872,  2261,  1055,   229,   336,
    2002,  2003,  2004,  2005,  2289,  2297,  2219,  2220,  2221,  2048,
    1887,   862,  1628,  1899,  2013,  2014,  2015,  1629,   570,   229,
     571,   572,   573,   574,   237,  1743,  1752,  1769,  1783,   868,
     229,  1908,  -592,  1916,   231,  1917,  2321,  1752,  1744,  1753,
    1770,  1784,  2327,  2335,  2049,  1924,  1927,   231,  1920,  1929,
    1753,  2174,  2175,  2176,  1444,   853,   853,  2260,  1941,  1942,
    1953,  1964,   233,  1971,  1982,  2042,  2052,  2059,  2010,  1986,
     230,  2018,   231,  2016,  2017,   233,  2030,   412,  2043,  2053,
    2060,  2032,   225,  2034,  2063,   238,   225,   230,   412,   234,
     236,   598,  2328,   599,   229,  1946,  1948,  1948,  1951,  2073,
     233,  2083,  1315,  1316,  2089,  2081,  2093,  2096,   230,  2334,
    2098,  1293,   600,  2104,  2333,  2116,  2117,  2122,  2140,   230,
     601,   602,   603,   604,  2142,  2132,  2149,   234,   236,  2295,
    2151,  2114,  2162,  2154,  2165,  2079,   325,   326,   327,  1888,
    1889,   330,  1237,  1238,  2168,  2169,  2170,  2171,   811,   430,
    2172,   431,  2177,  2050,  2178,   229,  1239,   822,   823,   824,
     825,   576,   577,  2179,  1240,  2187,  2180,  2222,   237,  2224,
     578,   579,   580,   581,   582,  2332,  2245,   430,  2146,   431,
    1241,   237,  2236,   230,  2114,  2234,  2237,   231,  2152,  2246,
     621,   622,  2247,   623,  2254,  2155,  2265,  -287,   238,  2338,
    2339,  2340,  2269,  2299,   231,  2273,   237,  2300,  2301,  2118,
    2119,  2319,   624,  1426,  1365,   233,  1255,  1428,  1427,   225,
     625,   626,  1256,   627,   880,   231,  1254,  1253,  1144,  2125,
    1813,  1140,   233,   628,   629,   630,   231,  2127,  2128,  1496,
    2137,  2131,   234,   236,   230,  1430,    72,  2135,   812,   238,
     524,   852,   852,   233,  2042,  2052,  2059,   813,  1079,   234,
     236,  2173,  1796,  2012,   233,  2190,  2207,  2043,  2053,  2060,
    2291,  2248,  2189,  2249,  1040,  2250,  2240,   239,     3,  2185,
     234,   236,     4,  2251,     5,  1040,  1040,  2320,  1950,   820,
    1076,   234,   236,  1337,   229,  2095,   913,  2080,  1353,  2216,
     231,  2069,   225,  1577,     6,     7,     8,     9,  1242,  1243,
    1244,   770,  1639,   229,  2065,  1845,  2066,   225,  1846,  1918,
    1788,   237,  1078,  1789,   780,   781,   782,  2167,   233,  1056,
    1919,  1400,  2239,  1678,  1680,  1679,  2277,  1697,   237,   229,
       3,  1881,  2186,  1408,     4,  1884,     5,  1317,   229,  1700,
     850,   228,   412,  1491,  1719,   234,   236,   791,  2072,   237,
    1643,   231,  2217,  2218,  1874,  1902,     6,     7,     8,     9,
     237,  1067,   226,   711,  2227,  2228,  1089,  2229,  2230,  2231,
    2232,  2233,  1903,   230,  2235,   712,  1091,   713,  1092,   233,
     239,  2325,  1090,   238,     3,   226,   860,  1220,     4,   864,
       5,   865,   230,  1192,  1539,   714,   238,   226,  2272,   715,
       0,  1186,   716,     0,   229,     0,   234,   236,     0,  2185,
       6,     7,     8,     9,     0,     0,     0,     0,   230,     0,
    2264,   238,     0,  2266,   237,     0,  2267,   230,     0,     0,
       0,   416,     0,  1790,  -573,  1729,     0,  1746,  1757,  1773,
    1786,     0,     0,     0,     0,     0,     0,     0,     0,  2318,
       0,     0,     0,   229,     0,     0,     0,     0,     0,  1727,
    1752,  1769,  1783,     0,     0,     0,     0,  1841,  1848,  1853,
    1859,     0,  1728,  1753,  1770,  1784,  2114,   229,     0,  2038,
    1842,  1849,  1854,  1860,     0,   237,     0,  2298,     0,     0,
     231,   717,     0,   230,     0,  2304,     0,  2305,  2306,  2307,
    2308,  2309,  2042,  2052,  2059,  2313,  2314,  2315,  2316,   231,
       0,   412,     0,     0,     0,  2043,  2053,  2060,   233,  2039,
       0,     0,  2326,     0,   718,   719,     0,     0,     0,  2329,
    2330,  2331,     0,     0,     0,   231,   238,   233,     0,     0,
       0,     0,   230,     0,   231,   234,   236,     0,   226,     0,
       0,  1843,   226,   238,  1850,     0,  1855,     0,     0,  1861,
       0,  1791,     0,   233,   234,   236,   230,     3,     0,     0,
       0,     4,   233,     5,   238,   416,     0,     0,     0,     0,
       0,     0,  1288,     0,  1290,   238,     0,     0,   239,   655,
     234,   236,     0,     6,     7,     8,     9,     0,   656,   234,
     236,  1305,  1306,     0,  1307,     0,   720,  1318,  1319,  1788,
     231,     0,  1789,   416,   657,     0,  1639,     0,   658,     0,
       0,     0,   659,   660,   237,     0,     0,   661,   662,   663,
     664,   665,     0,     0,  2024,     0,     0,     0,   233,     0,
       0,     0,     0,   237,   911,   912,     0,     0,     0,   238,
       0,     0,     0,     0,     0,  1363,     0,     0,  1367,   231,
       0,     0,     0,  2040,     3,   234,   236,     0,     4,   237,
       5,     0,     0,     0,     0,     0,     0,     0,   237,     0,
     880,     0,     0,   231,     0,   226,     0,   233,   499,     0,
       6,     7,     8,     9,   506,     0,  2288,   513,   514,   515,
     516,     0,     0,     0,   520,  1759,   225,     0,     0,     0,
     238,   233,   532,   533,   234,   236,     0,     0,     0,     0,
       0,     0,     0,     0,  1404,     0,   225,     0,   239,     0,
       0,     0,  1790,  1040,   880,     0,     0,     0,   234,   236,
    1042,     0,     0,     0,   237,   239,     0,     0,     0,     0,
    2136,  1042,  1042,     0,  1760,  1761,  1762,  1763,     0,  1764,
       0,     0,  1765,     0,     0,     0,   416,     0,   226,     0,
       0,     0,  1746,  1757,  1773,  1786,     3,   416,     0,  1766,
       4,     3,     5,   226,  1757,     4,     0,     5,     0,     0,
       0,     0,     0,   237,  1451,  1452,     0,  1379,     0,     0,
       0,     0,     6,     7,     8,     9,     0,     6,     7,     8,
       9,     0,  2044,  2054,  2061,     0,     0,   237,  1788,     3,
       0,  1789,  1759,     4,     0,     5,     0,     0,     0,  1767,
       0,     0,     0,     0,     0,   584,   585,     0,     0,   586,
       0,   239,     0,     0,     0,     6,     7,     8,     9,   238,
     587,   588,   589,   590,   591,   592,   593,   594,   595,     0,
    1791,     0,  1040,  1040,  1040,  1040,  1040,   225,   238,  1040,
       0,  1760,  1761,  1762,  1763,     0,  1764,     0,     0,  1765,
       0,     3,     0,     0,     0,     4,     0,     5,     0,     0,
       0,     0,     0,     0,   238,  1040,  1766,     0,     0,     0,
     596,     0,   239,   238,     0,     0,     0,     6,     7,     8,
       9,     3,     0,     0,  1040,     4,     0,     5,     0,     0,
       0,     0,  1776,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  2029,  2056,     0,     0,     6,     7,     8,
       9,  1790,  1626,     0,     0,     0,  1767,     0,     0,     0,
       0,     0,     0,  1788,     0,     0,  1789,  1627,     0,     0,
       0,     0,   336,     0,     0,     0,     0,     0,     0,   238,
       0,     0,  1777,  1778,  1779,  1628,  1764,     0,     0,  1765,
    1629,     0,     0,     3,     0,     0,     0,     4,     0,     5,
       0,     0,     0,     0,     0,     0,  1780,     0,     0,     0,
       0,  2044,  2054,  2061,     0,     0,     0,     0,     0,     6,
       7,     8,     9,     0,   880,     0,     0,     0,   238,     0,
    1585,     0,     0,     0,  1588,     3,     0,     0,     0,     4,
    2022,     5,     0,     0,  1608,  1609,  1610,  1611,     0,  1613,
       0,   416,   238,     0,     0,     0,  1781,     0,     0,     0,
       0,     6,     7,     8,     9,  1622,     0,     0,     0,  1626,
     239,     0,     0,     0,     0,     0,     0,     0,     0,  1791,
       0,     0,     0,     0,  1627,     0,  1790,     0,  2038,   336,
       0,     0,     0,     0,     0,     3,   239,     0,     0,     4,
       0,     5,  1628,     0,     0,   239,     0,  1629,  2057,     0,
       0,  1671,     0,  1673,     0,     0,     0,     0,     0,     0,
    1674,     6,     7,     8,     9,     0,  2046,     0,  2039,     0,
       0,     0,     0,  2047,     3,  1687,  1776,  1688,     4,     0,
       5,   880,     0,     0,     0,     0,  1696,     0,  1698,     0,
       0,     0,  2037,     0,     0,     0,     0,  2280,     0,     0,
       6,     7,     8,     9,   225,     0,     0,     0,     0,  1626,
    2048,   239,     3,     0,     0,     0,     4,     0,     5,   880,
       0,     0,     0,     0,  1627,     0,  1777,  1778,  1779,   336,
    1764,     0,   226,  1765,     0,  2138,     0,     0,     6,     7,
       8,     9,  1628,     0,     0,  2049,     0,  1629,     0,     0,
    1780,     0,  1312,     0,  1791,     0,     0,     0,     0,  1042,
     416,     0,     0,     0,     0,     0,  1729,  1757,  1773,  1786,
       0,     0,     0,     0,  1843,  1850,  1855,  1861,     0,     0,
       0,   880,     0,     0,   239,     0,     0,  1379,  1379,  1379,
    1379,  1379,     0,  1379,  1379,     0,     0,  1782,     0,     0,
    1781,     0,  1692,  1692,     0,     0,     0,     0,     0,  2044,
    2054,  2061,  2040,     0,     0,     0,     0,   225,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  2282,     0,     3,
       0,     0,     0,     4,     0,     5,     0,     0,     0,     0,
       0,     0,     0,  1040,  1040,  1040,  1040,  1040,  1040,  1040,
    1040,  1040,  1040,  1040,  2050,     6,     7,     8,     9,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   225,     0,
       0,  1863,  1864,     0,     3,     0,     0,  1407,     4,     0,
       5,     0,     0,   880,     0,     0,     0,     0,  1042,  1042,
    1042,  1042,  1042,   226,     0,  1042,     0,  1879,     0,  2310,
       6,     7,     8,     9,     0,     0,  1886,     0,     0,     0,
       0,     0,  1731,  1626,     0,     0,     0,     0,     0,     0,
       0,  1042,  1732,     0,     0,   880,     0,  1733,  1627,  1734,
       0,  1735,     0,   336,     0,     0,     0,     0,     0,     0,
    1042,  2311,     0,     0,  2056,     0,  1628,     0,     0,     0,
       0,  1629,     0,     0,     0,     0,    90,  1926,     0,     0,
       0,     0,     0,     0,    91,     0,    92,     0,    93,  1273,
       0,    94,    95,    96,  1928,    97,  1930,     0,     0,     0,
       0,    51,     0,     0,     0,   880,     0,     0,     0,     0,
       0,    63,    51,    51,    66,    66,    66,    70,     0,     0,
      51,  2023,     0,   281,     0,     0,     0,  1273,     0,     0,
       0,   282,   225,   283,     0,   284,     0,     0,   285,   286,
     287,     0,   288,     0,   880,   225,     0,     0,     0,     0,
    1040,     0,     0,     0,     0,     0,     0,     0,     0,  1736,
    1840,     0,     0,     0,     0,     0,     0,     0,  1040,     0,
     225,     0,     0,    51,     0,     0,   438,    51,    66,    66,
      66,    70,   880,     0,     0,     0,     0,     0,    51,     0,
     453,   458,   461,    51,   466,    66,   466,   471,  2286,   453,
     453,   453,   453,     0,    66,     0,     0,     0,   487,     0,
      51,     0,   495,    66,    66,   495,    70,     0,     0,    66,
       0,   495,     0,     0,   495,   495,   495,   495,  2057,  1040,
       0,   495,     0,    51,   525,    66,    66,    66,     0,   495,
     495,     0,     0,   536,    51,    66,    66,  1040,  1040,    66,
       3,    51,     0,     0,     4,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    2071,     0,  1737,     0,     0,     0,     6,     7,     8,     9,
       0,     0,     0,     0,     0,     0,     0,  1738,     0,     0,
       0,  1776,  2077,     0,     0,     0,     0,     0,     0,   880,
     226,     0,     0,     0,     0,  1739,     0,     0,     0,     0,
    1740,     0,  1741,     0,     0,  2312,    98,     0,     0,     0,
       0,     0,     0,   225,     0,     0,     0,     0,     0,     0,
       0,    99,  2088,     0,   225,     0,     0,     0,     0,     0,
       0,  1777,  1778,  1779,   880,  1764,     0,     0,  1765,   100,
     752,     0,     0,     0,   101,     0,   102,     0,     0,     0,
    1742,  1665,  1665,   289,  1665,  1780,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   753,   290,     0,
     754,   453,   458,   461,    51,   466,    66,    66,   471,     0,
     453,   453,   453,   453,     0,     0,   291,   755,   225,     0,
       0,   292,     0,   293,     0,     0,     0,     0,     0,     0,
       0,    75,     0,   226,     0,  1781,     0,     0,     0,    76,
       0,    77,     0,     0,     0,     3,    78,    79,    80,     4,
      81,     5,     0,     0,     0,     0,     0,     0,     0,  1042,
    1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,
       0,     6,     7,     8,     9,     0,     0,     0,     0,   225,
       0,     0,     0,  1731,   226,     0,     0,     0,     0,     0,
       0,     0,     0,  1732,     0,     0,     0,     0,  1733,     0,
    1734,     0,  1735,     0,     0,     0,     0,     0,     0,   921,
       0,     0,     0,     0,  2161,   922,     0,     0,     0,     0,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,     0,     0,   158,   159,   160,     0,     0,  1626,     0,
    1021,     0,     0,     0,  1022,     0,     0,     0,     0,  1023,
       0,     0,     0,  1627,     0,  1024,     0,     0,   336,   929,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1628,     0,   756,     0,     0,  1629,   213,   214,  1027,
       0,     0,     0,     0,     0,     0,     0,   757,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1736,  1112,     0,     0,     0,     0,     0,     0,   225,     0,
     880,     0,    66,   758,     0,     0,     0,     0,   226,     0,
       0,     0,     0,     0,     0,     0,  2281,   225,     0,  1883,
    1040,  1312,  1407,    51,     0,     0,  1042,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   861,     0,
       0,    66,     0,     0,  1042,     0,   226,     0,     0,     3,
       0,    82,     0,     4,     0,     5,     0,     0,     0,     0,
     762,   763,     0,     0,   764,  1665,    83,     0,  1665,     0,
       0,  1665,   153,     0,     0,     6,     7,     8,     9,   154,
       0,   155,   765,     0,    84,   318,   156,   319,     0,    85,
       0,    86,     0,     0,  1747,     0,     0,     0,     0,     0,
       0,     0,   320,  1737,     0,  1042,   932,     0,     0,   933,
     934,     0,   321,   322,     0,     0,   935,   323,  1738,     0,
     324,     0,     0,  1042,  1042,     0,     0,     0,     0,   325,
     326,   327,   328,   329,   330,     0,  1739,     0,     0,   862,
       0,  1740,     0,  1741,  1748,     0,   937,   938,   331,     0,
     332,     0,     0,   157,   158,   159,   160,     0,     0,   161,
       0,   162,     0,     0,  1749,   163,     0,   225,     0,     0,
     164,     0,     0,     0,     0,   880,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   221,     0,
       0,  2020,   166,     0,     0,     0,     0,   167,     0,   226,
     168,     0,   766,     0,     0,   169,     0,     0,     0,   170,
     226,     0,   171,   172,  1750,     0,     0,   173,     0,     0,
     174,     0,   175,     0,     0,     0,   767,   768,   769,   770,
     771,   772,   863,   773,   774,   775,   776,   777,   778,   779,
       0,     0,   780,   781,   782,     0,     0,   176,   177,     0,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,   783,     0,     0,     0,     0,
       0,    11,     0,    12,   226,    13,     0,     0,    14,    15,
      16,     0,    17,     0,     0,     0,    18,    19,   182,   183,
     184,   185,     0,     0,     0,     0,     0,   186,   187,     0,
       0,   188,   189,   333,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,     0,     0,   334,     0,     0,
       0,     0,     0,     0,     0,   226,    35,     0,     0,     0,
       0,     0,   335,     0,     0,     0,    36,   336,     0,     0,
       0,     0,    37,     0,   337,     0,     0,     0,     0,     0,
     338,     0,     0,     0,     0,   339,     0,     0,     0,     0,
       0,   213,   214,     0,   340,     0,    38,   341,   342,   343,
     344,     0,     0,     0,   345,     0,     0,     0,   346,   347,
     215,     0,     0,     0,   216,   217,     0,     0,     0,   880,
       0,     0,     0,   348,     0,     3,   218,   219,     0,     4,
       0,     5,   349,     0,   350,  1751,   220,     0,     0,   221,
       0,     0,     0,     0,  1168,     0,     0,     0,   153,     0,
       0,     6,     7,     8,     9,   154,     0,   155,     0,     0,
    1169,   318,   156,   319,     0,     0,     0,     0,     0,     0,
    1747,     0,     0,     0,     0,     0,     0,     0,   320,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,   322,
       0,     0,   859,   323,   226,     0,   324,     0,     0,     0,
       0,     0,     0,     0,     0,   325,   326,   327,   328,   329,
     330,     0,     0,   226,     0,     0,  1042,     0,     0,   753,
    1748,     0,   754,    39,   331,  1170,   332,     0,     0,   157,
     158,   159,   160,     0,     0,   161,     0,   162,    40,   755,
    1749,   163,     0,     0,     0,     0,   164,     0,     0,     0,
       0,     0,   165,     0,     0,     0,    41,     0,     0,     0,
       0,    42,     0,    43,     0,     0,     0,     0,   166,     0,
       0,     0,     0,   167,     0,     0,   168,     0,     0,     0,
       0,   169,     0,     0,     0,   170,     0,     0,   171,   172,
    1750,     0,     0,   173,     0,     0,   174,  1171,   175,     0,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   176,   177,     0,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,  1489,     0,     0,     0,
       0,     0,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   226,   182,   183,   184,   185,     0,     0,
       0,     0,     0,   186,   187,     0,     0,   188,   189,   333,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,     0,     0,   334,     0,   756,     0,     0,     0,   213,
     214,     0,     0,     0,     0,     0,     0,     0,   335,   757,
       0,     0,     0,   336,     0,     0,     0,     0,     0,     0,
     337,     0,     0,     0,     0,     0,   338,     0,     0,     0,
       0,   339,     0,     0,     0,   758,     0,   213,   214,     0,
     340,     0,     0,   341,   342,   343,   344,     0,     0,     0,
     345,     0,     0,  2257,   346,   347,   215,     0,     0,     0,
     216,   217,     0,     0,     3,   880,     0,     0,     4,   348,
       5,     0,   218,   219,     0,   762,   763,     0,   349,   764,
     350,  2021,   220,     0,     0,   221,     0,   153,     0,     0,
       6,     7,     8,     9,   154,     0,   155,   765,     0,     0,
     318,   156,   319,     0,     0,     0,     0,     0,     0,  1747,
       0,     0,     0,     0,     0,     0,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   321,   322,     0,
       0,     0,   323,     0,     0,   324,     0,     0,     0,     0,
       0,     0,     0,     0,   325,   326,   327,   328,   329,   330,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1748,
       0,     0,     0,   331,     0,   332,     0,     0,   157,   158,
     159,   160,     0,     0,   161,     0,   162,     0,     0,  1749,
     163,     0,     0,     0,     0,   164,     0,     0,     0,     0,
       0,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1596,     0,     0,   166,     0,     0,
       0,     0,   167,     0,     0,   168,     0,   766,     0,     0,
     169,     0,     0,     0,   170,     0,     0,   171,   172,  1750,
       0,     0,   173,     0,     0,   174,     0,   175,     0,     0,
       0,   767,   768,   769,   770,   771,   772,  2258,   773,   774,
     775,   776,   777,   778,   779,     0,     0,   780,   781,   782,
       0,     0,   176,   177,     0,   178,   179,   180,   181,  1170,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     783,     0,     0,     0,     0,     0,   762,   763,     0,     0,
     764,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,   183,   184,   185,     0,   765,     0,
       0,     0,   186,   187,     0,     0,   188,   189,   333,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
       0,  1597,   334,     0,  1172,  1173,  1598,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1599,   335,     0,     0,
       0,     0,   336,     0,     0,     0,     0,     0,     0,   337,
       0,     0,     0,     0,     0,   338,     0,     0,     0,     0,
     339,     0,     0,     0,     0,     0,   213,   214,     0,   340,
       0,     0,   341,   342,   343,   344,     0,     0,     0,   345,
       0,     0,     0,   346,   347,   215,     0,     0,     0,   216,
     217,     0,     0,     3,   880,     0,     0,     4,   348,     5,
       0,   218,   219,     0,     0,     0,     0,   349,   766,   350,
    2028,   220,     0,     0,   221,     0,   153,     0,     0,     6,
       7,     8,     9,   154,     0,   155,     0,     0,     0,   318,
     156,   319,   767,   768,   769,   770,   771,   772,  1747,   773,
     774,   775,   776,   777,   778,   779,   320,     0,   780,   781,
     782,     0,     0,     0,     0,     0,   321,   322,     0,     0,
       0,   323,     0,     0,   324,     0,     0,     0,     0,     0,
       0,   783,     0,   325,   326,   327,   328,   329,   330,     0,
       0,     0,     0,     0,   161,     0,   162,     0,  1748,     0,
     163,     0,   331,     0,   332,   164,     0,   157,   158,   159,
     160,   165,     0,   161,     0,   162,     0,     0,  1749,   163,
       0,     0,     0,     0,   164,     0,     0,   166,     0,     0,
     165,     0,   167,     0,     0,   168,     0,     0,     0,     0,
     169,     0,     0,     0,   170,     0,   166,   171,   172,   672,
       0,   167,   173,     0,   168,   174,     0,   175,     0,   169,
       0,     0,     0,   170,     0,     0,   171,   172,  1750,     0,
       0,   173,     0,     0,   174,     0,   175,     0,     0,     0,
       0,     0,   176,     0,   673,  1195,     0,     0,   181,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   176,   177,     0,   178,   179,   180,   181,  1196,  1197,
    1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,  1207,
    1208,  1209,  1210,  1211,  1212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   182,   183,   184,   185,     0,     0,     0,     0,
       0,   186,   187,     0,     0,   188,   189,   333,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,     0,
       0,   334,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,     0,     0,     0,
       0,   336,     0,     0,     0,     0,     0,     0,   337,     0,
       0,     0,     0,     0,   338,     0,     0,     0,     0,   339,
       0,     0,     0,     0,     0,   213,   214,     0,   340,     0,
       0,   341,   342,   343,   344,     0,     0,     0,   345,     0,
       0,     0,   346,   347,   215,     0,     0,     0,   216,   217,
       0,     0,     4,   880,     5,     0,     0,   348,     0,     0,
     218,   219,     0,     0,     0,     0,   349,     0,   350,  2279,
     220,   153,     0,   221,     0,     0,     0,     0,   154,     0,
     155,     0,     0,     0,   318,   156,   319,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   321,   322,     0,     0,     0,   323,     0,     0,   324,
       0,     0,     0,     0,     0,     0,     0,     0,   325,   326,
     327,   328,   329,   330,     0,     0,     0,     0,     0,   161,
       0,   162,     0,     0,   701,   163,     0,   331,     0,   332,
     164,     0,   157,   158,   159,   160,   165,     0,   161,     0,
     162,     0,     0,     0,   163,     0,     0,     0,     0,   164,
       0,     0,   166,     0,     0,   165,     0,   167,     0,     0,
     168,     0,     0,     0,     0,   169,     0,     0,     0,   170,
       0,   166,   171,   172,     0,     0,   167,   173,     0,   168,
     174,     0,   175,     0,   169,     0,     0,     0,   170,     0,
       0,   171,   172,     0,     0,     0,   173,     0,     0,   174,
       0,   175,     0,     0,     0,     0,     0,   176,     0,   702,
       0,     0,     0,   181,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   176,   177,     0,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   182,   183,   184,
     185,     0,     0,     0,     0,     0,   186,   187,     0,     0,
     188,   189,   333,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,     0,     0,   334,     0,     0,   920,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,     0,     0,   336,   153,     0,     0,
       0,     0,     0,   337,   154,     0,   155,     0,     0,   338,
       0,   156,     0,     0,   339,     0,     0,     0,     0,     0,
     213,   214,     0,   340,     0,     0,   341,   342,   343,   344,
       0,     0,     0,   345,     0,     0,     0,   346,   347,   215,
       0,     0,     0,   216,   217,   921,     0,     0,     0,     0,
       0,   922,   348,     0,     0,   218,   219,     0,     0,     0,
     923,   349,   924,   350,  1904,   220,     0,     0,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   157,   158,
     159,   160,     0,     0,   161,     0,   925,     0,     0,     0,
     926,     0,     0,     0,     0,   927,     0,     0,   161,     0,
     162,   928,     0,     0,   163,   929,     0,     0,     0,   164,
       0,     0,     0,     0,     0,   165,     0,   166,     0,     0,
       0,     0,   167,     0,     0,   930,     0,     0,     0,     0,
     169,   166,     0,     0,   170,     0,   167,   171,   172,   168,
       0,     0,   173,     0,   169,   174,     0,   175,   170,     0,
       0,   171,   172,     0,     0,     0,   173,     0,     0,   174,
       0,   175,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   176,   177,     0,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,     0,   176,     0,     0,     0,
     106,     0,   181,     0,     0,   107,     0,     0,   108,   109,
     110,   111,     0,     0,   112,   113,     0,   114,   115,   116,
       0,   117,     0,   931,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
    1285,   118,   212,   119,   120,   121,     0,     0,     0,     0,
       0,     0,   932,     0,     0,   933,   934,     0,   153,     0,
       0,     0,   935,     0,     0,   154,     0,   155,     0,     0,
     936,     0,   156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   214,     0,     0,
       0,     0,   937,   938,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,   921,     0,     0,   216,
     217,     0,   922,     0,     0,     0,     0,     0,     0,   939,
     940,     0,   758,     0,     0,     0,     0,   161,     0,     0,
       0,   941,     0,     0,   221,     0,     0,     0,     0,   157,
     158,   159,   160,     0,   165,   161,     0,   925,     0,     0,
       0,   926,     0,     0,     0,     0,   927,     0,     0,     0,
     166,     0,  1286,     0,     0,   167,   929,     0,   168,     0,
       0,     0,     0,   169,     0,     0,     0,   170,   166,     0,
     171,   172,     0,   167,     0,   173,  1287,     0,   174,     0,
     175,   169,     0,     0,     0,   170,     0,     0,   171,   172,
       0,     0,     0,   173,     0,     0,   174,     0,   175,     0,
       0,     0,     0,     0,     0,   176,     0,     0,     0,     0,
     122,   181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   176,   177,   123,   178,   179,   180,   181,
       0,     0,   124,   125,   126,   127,     0,     0,     0,     0,
       0,     0,     0,   128,     0,     0,     0,     0,   129,     0,
     130,   131,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   931,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,  1366,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   932,     0,     0,   933,   934,     0,   153,
       0,     0,     0,   935,     0,     0,   154,     0,   155,     0,
       0,     0,     0,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   214,     0,
       0,     0,     0,   937,   938,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,   921,     0,     0,
     216,   217,     0,   922,     0,     0,     0,     0,     0,     0,
     939,   940,     0,   758,     0,     0,     0,     0,     0,     0,
       0,     0,   941,     0,     0,   221,     0,     0,     0,     0,
     157,   158,   159,   160,     0,     0,   161,     0,   925,     0,
       0,     0,   926,     0,     0,     0,     0,   927,     0,     0,
       0,     0,     0,  1286,     0,     0,     0,   929,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
       0,     0,     0,     0,   167,     0,     0,  1287,     0,     0,
       0,     0,   169,     0,     0,     0,   170,     0,     0,   171,
     172,     0,     0,     0,   173,     0,     0,   174,     0,   175,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   176,   177,     0,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   931,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,  1403,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   932,     0,     0,   933,   934,     0,
     153,     0,     0,     0,   935,     0,     0,   154,     0,   155,
       0,     0,     0,     0,   156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
       0,     0,     0,     0,   937,   938,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,   921,     0,
       0,   216,   217,     0,   922,     0,     0,     0,     0,     0,
       0,   939,   940,     0,   758,     0,     0,     0,     0,     0,
       0,     0,     0,   941,     0,     0,   221,     0,     0,     0,
       0,   157,   158,   159,   160,     0,     0,   161,     0,   925,
       0,     0,     0,   926,     0,     0,     0,     0,   927,     0,
       0,     0,     0,     0,  1286,     0,     0,     0,   929,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     166,     0,     0,     0,     0,   167,     0,     0,  1287,     0,
       0,     0,     0,   169,     0,     0,     0,   170,     0,     0,
     171,   172,     0,     0,     0,   173,     0,     0,   174,     0,
     175,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   176,   177,     0,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   931,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,  1587,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   932,     0,     0,   933,   934,
       0,   153,     0,     0,     0,   935,     0,     0,   154,     0,
     155,     0,     0,     0,     0,   156,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
     214,     0,     0,     0,     0,   937,   938,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,   921,
       0,     0,   216,   217,     0,   922,     0,     0,     0,     0,
       0,     0,   939,   940,     0,   758,     0,     0,     0,     0,
       0,     0,     0,     0,   941,     0,     0,   221,     0,     0,
       0,     0,   157,   158,   159,   160,     0,     0,   161,     0,
     925,     0,     0,     0,   926,     0,     0,     0,     0,   927,
       0,     0,     0,     0,     0,  1286,     0,     0,     0,   929,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   166,     0,     0,     0,     0,   167,     0,     0,  1287,
       0,     0,     0,     0,   169,     0,     0,     0,   170,     0,
       0,   171,   172,     0,     0,     0,   173,     0,     0,   174,
       0,   175,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   176,   177,     0,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1014,  1015,  1016,  1017,
    1018,  1019,     0,     0,     0,     0,  1020,     0,     0,     0,
    1002,     0,     0,     0,     0,     0,     0,   931,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,     0,     0,   212,     0,   921,     0,
       0,     0,     0,     0,   922,     0,   932,     0,     0,   933,
     934,     0,     0,     0,     0,     0,   935,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,   158,   159,   160,     0,     0,     0,     0,  1021,
     213,   214,     0,  1022,     0,     0,   937,   938,  1023,  1322,
       0,     0,     0,     0,  1024,     0,     0,     0,   929,   215,
       0,     0,     0,   216,   217,     0,     0,     0,     0,     0,
    1025,     0,     0,   939,   940,  1026,   758,     0,  1027,     0,
       0,     0,     0,  1028,     0,   941,     0,  1029,   221,     0,
    1030,  1031,     0,     0,     0,  1032,     0,     0,  1033,   921,
    1034,     0,     0,     0,     0,   922,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1035,     0,     0,  1036,     0,
       0,  1037,   157,   158,   159,   160,     0,     0,     0,     0,
    1021,     0,     0,     0,  1022,     0,     0,     0,     0,  1023,
       0,     0,     0,     0,     0,  1024,     0,     0,     0,   929,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1025,     0,     0,     0,     0,  1026,     0,     0,  1027,
       0,     0,     0,     0,  1028,     0,     0,     0,  1029,     0,
       0,  1030,  1031,     0,     0,  1322,  1032,     0,     0,  1033,
       0,  1034,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   932,     0,     0,   933,   934,
       0,     0,     0,     0,     0,   935,  1035,     0,     0,  1036,
       0,     0,  1037,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   921,     0,     0,     0,     0,
       0,   922,     0,     0,     0,   937,   938,     0,  1038,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   157,   158,
     159,   160,   939,   940,     0,     0,  1021,     0,     0,     0,
    1022,     0,   350,     0,  1039,  1023,     0,   221,     0,     0,
       0,  1024,     0,     0,     0,   929,  1061,  1002,     0,     0,
       0,     0,     0,     0,     0,     0,   932,  1025,     0,   933,
     934,     0,  1026,     0,     0,  1027,   935,     0,     0,     0,
    1028,     0,     0,     0,  1029,     0,     0,  1030,  1031,     0,
       0,     0,  1032,     0,     0,  1033,     0,  1034,     0,     0,
       0,     0,     0,     0,     0,   921,   937,   938,     0,  1038,
       0,   922,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1035,     0,     0,  1036,     0,     0,  1037,     0,
       0,     0,     0,   939,   940,     0,     0,     0,   157,   158,
     159,   160,     0,   350,     0,  1039,  1021,     0,   221,     0,
    1022,     0,     0,     0,     0,  1023,     0,     0,     0,     0,
       0,  1024,     0,     0,     0,   929,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,     0,     0,
       0,     0,  1026,     0,     0,  1027,     0,     0,     0,     0,
    1028,     0,     0,     0,  1029,     0,     0,  1030,  1031,     0,
       0,     0,  1032,     0,     0,  1033,     0,  1034,     0,     0,
       0,     0,   932,     0,     0,   933,   934,     0,     0,     0,
       0,     0,   935,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1035,     0,     0,  1036,     0,     0,  1037,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   937,   938,     0,  1038,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   939,
     940,     0,     0,     0,     0,     0,     0,     0,   153,     0,
       0,  1039,     0,     0,   221,   154,     0,   155,     0,     0,
       0,   318,   156,   319,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   320,     0,
       0,     0,   932,     0,     0,   933,   934,     0,   321,   322,
       0,     0,   935,   323,     0,     0,   324,     0,     0,     0,
       0,     0,     0,     0,     0,   325,   326,   327,   328,   329,
     330,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   937,   938,   331,  1038,   332,     0,     0,   157,
     158,   159,   160,     0,     0,   161,     0,   162,     0,     0,
       0,   163,     0,     0,     0,     0,   164,     0,     0,   939,
     940,     0,   165,     0,     0,     0,     0,     0,     0,  1062,
       0,  1039,     0,     0,   221,     0,     0,     0,   166,     0,
       0,     0,     0,   167,     0,     0,   168,     0,     0,     0,
       0,   169,     0,     0,     0,   170,     0,     0,   171,   172,
       0,     0,     0,   173,     0,     0,   174,     0,   175,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   176,   177,     0,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   182,   183,   184,   185,     0,     0,
       0,     0,     0,   186,   187,     0,     0,   188,   189,   333,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,     0,     0,   334,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   153,     0,     0,   335,     0,
       0,     0,   154,   336,   155,     0,     0,     0,     0,   156,
     337,     0,     0,     0,     0,     0,   338,     0,     0,     0,
       0,   339,     0,     0,     0,     0,     0,   213,   214,     0,
     340,     0,     0,   341,   342,   343,   344,     0,     0,     0,
     345,     0,     0,   921,   346,   347,   215,     0,     0,   922,
     216,   217,     0,     0,     0,     0,     0,     0,     0,   348,
       0,     0,   218,   219,     0,     0,     0,     0,   349,     0,
     350,     0,   220,     0,     0,   221,   157,   158,   159,   160,
       0,     0,   161,     0,   925,     0,     0,     0,   926,     0,
       0,     0,     0,   927,     0,     0,     0,     0,     0,  1286,
       0,     0,     0,   929,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   166,     0,     0,     0,     0,
     167,     0,     0,  1287,     0,     0,     0,     0,   169,     0,
       0,     0,   170,     0,     0,   171,   172,     0,     0,     0,
     173,     0,     0,   174,     0,   175,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     176,   177,     0,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   931,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     932,     0,   153,   933,   934,     0,     0,     0,     0,   154,
     935,   155,     0,     0,     0,     0,   156,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,   214,     0,     0,     0,     0,
     937,   938,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,   216,   217,     0,
       0,     0,     0,     0,     0,     0,     0,   939,   940,     0,
     758,     0,     0,     0,     0,     0,     0,     0,     0,   941,
       0,     0,   221,   157,   158,   159,   160,     0,     0,   161,
       0,   162,     0,     0,     0,   163,     0,     0,     0,     0,
     164,     0,     0,     0,     0,     0,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   166,     0,     0,     0,     0,   167,     0,     0,
     168,     0,     0,     0,     0,   169,     0,     0,     0,   170,
       0,     0,   171,   172,     0,     0,     0,   173,     0,     0,
     174,     0,   175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   176,   177,     0,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     153,     0,     0,     0,     0,     0,     0,   154,     0,   155,
       0,     0,     0,     0,   156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,   183,
     184,   185,     0,     0,     0,     0,     0,   186,   187,     0,
       0,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,   158,   159,   160,     0,     0,   161,     0,   162,
       0,     0,     0,   163,     0,     0,     0,     0,   164,     0,
       0,     0,     0,     0,   165,     0,     0,     0,     0,     0,
       0,   213,   214,     0,     0,     0,     0,     0,     0,     0,
     166,     0,     0,     0,     0,   167,     0,     0,   168,     0,
     215,     0,     0,   169,   216,   217,     0,   170,     0,     0,
     171,   172,     0,     0,     0,   173,   218,   219,   174,     0,
     175,     0,     0,     0,   350,     0,   220,     0,     0,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   176,   177,     0,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1002,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   183,   184,   185,
       0,     0,     0,     0,     0,   186,   187,     0,     0,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   921,     0,   212,     0,     0,     0,   922,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   157,   158,   159,   160,
       0,     0,     0,     0,  1021,     0,     0,     0,  1022,   213,
     214,     0,     0,  1023,     0,     0,     0,     0,     0,  1024,
       0,     0,     0,   929,     0,     0,     0,     0,   215,     0,
       0,     0,   216,   217,     0,  1025,     0,     0,     0,     0,
    1026,     0,     0,  1027,   218,   219,     0,     0,  1028,     0,
       0,     0,  1029,     0,   220,  1030,  1031,   221,     0,     0,
    1032,   153,     0,  1033,     0,  1034,     0,     0,   154,     0,
     155,     0,     0,     0,     0,   156,  2191,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1035,     0,     0,  1036,     0,     0,  1037,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  2192,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  2193,     0,     0,     0,     0,   161,     0,
     162,     0,     0,     0,   163,     0,     0,     0,     0,   164,
       0,     0,     0,     0,     0,   165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     932,   166,     0,   933,   934,     0,   167,     0,     0,   168,
     935,     0,     0,     0,   169,     0,     0,     0,   170,     0,
       0,   171,   172,     0,     0,     0,   173,     0,     0,   174,
       0,   175,     0,     0,     0,     0,     0,     0,     0,     0,
     937,   938,     0,  1038,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   176,   177,     0,   178,
     179,   180,   181,     0,     0,     0,     0,   939,   940,   153,
       0,     0,     0,     0,     0,     0,   154,   350,   155,  1039,
       0,     0,   221,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   182,   183,   184,
     185,     0,     0,     0,     0,     0,   186,   187,     0,     0,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,     0,     0,  2194,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  2195,     0,     0,     0,     0,   161,     0,   162,     0,
       0,     0,   163,     0,     0,     0,     0,   164,     0,  2196,
       0,     0,     0,   165,  2197,     0,  2198,     0,     0,     0,
     213,   214,     0,     0,     0,     0,     0,     0,     0,   166,
       0,     0,     0,  2199,   167,     0,     0,   168,     0,   215,
       0,     0,   169,   216,   217,     0,   170,     0,     0,   171,
     172,     0,     0,     0,   173,   218,   219,   174,     0,   175,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   176,   177,     0,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,   183,   184,   185,     0,
       0,     0,     0,     0,   186,   187,     0,     0,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     3,     0,     0,     0,     4,     0,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
       0,     0,     0,     0,     0,     0,     0,     0,     6,     7,
       8,     9,     0,     0,     0,     0,     0,   215,     0,     0,
      10,   216,   217,     0,     0,     0,     0,     0,    11,     0,
      12,     0,    13,   218,   219,    14,    15,    16,     0,    17,
       0,     0,     0,    18,    19,    20,     0,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,     0,     0,     0,     0,   921,     0,     0,     0,     0,
       0,   922,     0,     0,   325,   326,   327,  1888,  1889,   330,
       0,     0,     0,     0,     0,     0,     0,  -438,     0,     0,
       0,     0,     0,    35,     0,     0,     0,     0,   157,   158,
     159,   160,     0,    36,     0,     0,  1021,     0,     0,    37,
    1022,     0,     0,     0,     0,  1023,     0,     0,     0,     0,
       0,  1024,     0,     0,     0,   929,     0,     0,     0,     0,
       0,     0,     0,    38,     0,     0,     0,  1025,     0,     0,
       0,     0,  1026,     0,     0,  1027,   921,     0,     0,     0,
    1028,     0,   922,     0,  1029,     0,     0,  1030,  1031,     0,
       0,     0,  1032,     0,     0,  1033,     0,  1034,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   157,
     158,   159,   160,     0,     0,     0,     0,  1021,     0,     0,
       0,  1022,  1035,     0,     0,  1036,  1023,     0,  1037,     0,
       0,     0,  1024,     0,     0,     0,   929,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1025,     0,
       0,     0,     0,  1026,     0,     0,  1027,     0,     0,     0,
       0,  1028,     0,     0,     0,  1029,     0,     0,  1030,  1031,
       0,     0,     0,  1032,     0,     0,  1033,     0,  1034,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1035,     0,    40,  1036,     0,     0,  1037,
    -438,     0,   932,     0,     0,   933,   934,     0,     0,     0,
       0,     0,   935,    41,     0,     0,     0,     0,    42,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   937,   938,     0,  1038,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   921,     0,     0,     0,     0,
       0,   922,     0,     0,     0,     0,     0,     0,     0,   939,
     940,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1039,     0,   932,   221,     0,   933,   934,  1716,   158,
     159,   160,     0,   935,     0,     0,  1021,     0,     0,     0,
    1022,     0,     0,     0,     0,  1023,     0,     0,     0,     0,
       0,  1024,     0,     0,     0,   929,     0,     0,     0,     0,
       0,     0,     0,   937,   938,     0,  1038,  1025,     0,     0,
       0,     0,  1026,     0,     0,  1027,     0,     0,     0,     0,
    1028,     0,     0,     0,  1029,     0,     0,  1030,  1031,     0,
     939,   940,  1032,     0,     0,  1033,     0,  1034,     0,     0,
       0,     0,  1039,     0,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1035,     0,     0,  1036,     0,     0,  1037,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   932,     0,     0,   933,   934,     0,     0,     0,
       0,     0,   935,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   937,   938,     0,  1038,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   939,
     940,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1039,     0,     0,   221
};

static const yytype_int16 yycheck[] =
{
       1,   369,     3,   353,   329,    68,   216,    68,   368,   452,
     363,    38,    53,     1,   443,   107,   519,   109,   440,   111,
     112,   408,    30,   115,    65,   117,   442,   687,   120,   121,
     389,    37,   124,   125,   126,   127,   631,   720,   877,   131,
     483,  1032,   134,   472,   480,    53,    47,   139,   140,   479,
     921,   179,   759,    32,    55,   147,   318,    65,   220,    60,
     569,   408,   396,   441,   870,  1464,  1471,  1464,  1469,   935,
    1323,  1324,   419,   420,  1468,  1470,  1410,  1330,   570,   447,
    1485,   408,   518,  1411,   423,   347,   439,   517,  1482,   843,
     408,   469,   419,   420,   386,   995,   388,  1325,   534,   180,
      36,  1037,   177,    53,   482,   180,   598,  1581,  1624,   462,
     478,    30,   941,  1803,   606,    65,  1325,  1325,  1801,   177,
     382,   875,   180,  1802,  1310,  1325,   880,  1008,   177,   177,
     622,   180,   180,  1009,    53,  1010,  1463,  1464,  1325,  1776,
       7,     8,     9,  1703,   512,   899,    65,   177,  1780,   903,
     180,   529,   177,   860,     5,   180,     8,  1591,  1592,  1012,
       5,     8,    24,   531,  1569,  1418,     8,    24,   177,  1560,
    1561,   180,   723,  1827,   725,   667,   727,  1985,     8,    40,
    2017,   732,    49,  1594,    51,    99,   177,   177,  1635,   180,
     180,    24,    92,   177,    61,    95,   180,  1006,  1007,    66,
      60,   105,   177,    70,  1638,   180,   177,   177,   216,   180,
     180,   177,   220,    23,   180,   177,   106,   122,   180,   711,
     177,   177,  1294,   180,   180,     8,   840,    21,    30,   837,
     838,  1759,   129,   107,   843,   109,   842,   111,    92,   105,
     190,    95,   793,   117,    87,  1035,   120,   841,  1776,   313,
      12,    53,    87,   756,   746,    59,    30,   106,   106,   107,
     134,   106,   107,    65,    13,   328,   329,     9,  1602,  1603,
      96,    60,  2117,   147,    16,    17,    18,    19,   107,    53,
     783,    23,    51,    30,    30,    81,    82,    59,   146,   106,
     784,    65,   130,   272,    51,   105,   106,   216,   106,   278,
      64,   220,   366,  1702,    87,   163,    53,    53,   106,   107,
     108,    73,   106,   106,    56,    77,    59,   155,    65,    65,
      55,   130,   825,   166,   106,   107,   108,  1761,  1762,    59,
      92,    93,    59,   396,   106,    87,   106,   101,  1092,    88,
       9,     8,   295,  1777,  1778,   353,   155,   408,   110,    16,
      17,    18,    19,    49,   221,   365,   144,     9,   419,   420,
     313,   365,    30,   106,    16,    17,    18,    19,   811,   157,
     864,   133,   134,  2048,  1034,  1702,   106,   366,   441,   106,
     441,   113,   144,   333,   289,    53,   174,    56,   824,   151,
     819,    47,    32,   823,   106,   436,   128,    65,   295,  1189,
    1799,    93,  1799,   353,    56,    91,   469,  1635,   469,   299,
     451,   503,  1640,   408,   216,   366,     8,    59,   220,   482,
    2265,   482,   113,   116,   419,   420,  1635,  1635,   436,    21,
    1025,  1640,  1640,  1028,   353,  1635,  1508,   128,  1291,   817,
    1640,   133,   216,   451,   137,   135,   220,  1774,  1635,   315,
     299,   351,   366,  1640,   822,  1902,  1360,   150,  1362,   285,
    1787,   328,   329,  1874,   106,   332,   529,   371,   529,   216,
     216,  1798,  1799,   220,   220,  1089,   116,  1085,  1086,  1087,
     297,   371,  2319,  1092,     1,  1091,   436,  2141,     5,  2126,
       7,   299,   352,   353,   354,  1103,  1090,   351,   333,   334,
     335,   451,     1,   297,   366,   366,     5,   300,     7,   371,
      27,    28,    29,    30,   371,   367,   367,   436,  2326,   146,
     367,   283,   284,   371,   151,   367,   371,   299,    27,    28,
      29,    30,   451,   366,  2094,   245,  1389,   367,   371,    92,
     328,  1295,  1296,   245,    43,   349,   350,    46,   216,   344,
     877,   353,   220,   363,   337,  2192,   299,  2247,  1387,  2234,
    1441,   371,  2245,   352,   353,   354,  1442,  2246,  1443,   299,
     105,   699,   299,   371,     1,   371,  2208,  2093,     5,   353,
       7,   333,   334,   335,   366,   297,   453,    30,   105,  2117,
     457,   597,   284,   620,   461,   954,   282,     9,  2126,   466,
      27,    28,    29,    30,   471,  1933,   353,   353,  1935,  2083,
      53,     8,  2046,    40,  1550,   308,    34,    35,    36,   700,
     260,   261,    65,   698,    21,   700,   296,  1070,   370,   503,
    1439,  1440,   345,   353,   436,   348,  1145,  1890,  1147,  1131,
     850,   105,   700,  2034,    56,   937,   938,   583,   366,   451,
     698,   700,   700,   371,     5,  1263,     7,   299,   525,   105,
     870,    87,   436,    89,  2192,  1381,  1382,   166,   698,  1220,
     700,  1058,  1388,   698,  1902,   700,   543,   451,  1012,  1006,
    1007,  1008,  1009,  1010,  1937,   353,  1013,   296,   715,   436,
     436,   700,  1039,  1902,  1902,  1013,   366,  2031,  1884,  1885,
     295,  1361,  1902,   370,   451,   451,   712,   698,   698,   700,
     700,  1058,  1039,   174,   698,  1902,   700,     5,   370,     7,
    1620,    64,    65,   698,    92,   700,    69,   698,   698,   700,
     700,  1058,   698,   286,   700,   288,   698,  2265,   700,  1078,
    1058,   698,   698,   700,   700,   295,  1038,   128,    92,   315,
     316,    95,   305,    41,   817,  1115,   817,   366,   295,  1663,
     148,   523,  1666,  2091,   128,  1669,  1632,  2172,   436,  2170,
      87,   152,    87,   216,    89,  2169,  2171,   220,    10,    30,
      81,  1284,   163,   451,   148,  1624,    51,    52,   152,    54,
      55,    56,    92,    40,    41,   294,    97,   106,   315,   316,
     366,   106,    53,  1642,   109,   110,   111,   870,    40,    41,
      42,    43,   166,  1619,    65,   116,   877,     5,     9,   245,
    1738,  1739,  1740,  1741,   351,    16,    17,    18,    19,  1583,
     333,     1,    23,  1342,   351,     5,    92,     7,   256,    92,
     343,   318,   850,    75,  2243,  1281,  2243,  1290,  1278,  1011,
     367,    83,    84,    85,    86,   116,   835,    27,    28,    29,
      30,    87,   870,    89,  1703,    56,     6,  2301,   367,     9,
       9,  1307,   133,    39,   106,   327,  1306,    16,    17,    18,
      19,   307,   877,    18,  1567,  1245,  1246,    19,    87,   277,
     278,   123,   246,   247,   248,   366,   128,   129,   130,   169,
     371,  1608,  1609,   930,   174,   166,   138,  1275,  1036,   141,
     353,  2164,   370,   277,   278,  2242,  2243,    56,   286,     5,
     288,     7,   928,    89,   351,    54,    80,   129,   279,   128,
     245,   850,   995,    54,   136,   279,    87,  1305,    47,    48,
     367,  1601,   189,   294,   366,   192,   941,     9,   299,  1012,
     294,   870,  1013,   152,     1,   299,   366,  1291,     5,   113,
       7,   312,   366,    25,   166,   216,   317,     6,   312,   220,
       9,   359,   366,   317,   128,  1478,    21,   128,    23,   360,
      27,    28,    29,    30,   170,   171,   286,   288,   288,  1649,
     362,   363,   307,   436,    56,   359,  1258,  1058,     1,  2252,
     366,   152,     5,  1011,     7,   296,   297,   351,   451,  1501,
     155,  1006,  1007,  1008,  1009,  1010,   114,   115,  1013,   245,
     174,   166,   167,  1466,    27,    28,    29,    30,    59,   294,
     286,   116,   288,   286,     8,   288,   301,   302,  1956,  1957,
    1958,  1477,   296,   116,  1039,  1404,  1476,    21,   850,    23,
     366,  1438,   317,  1924,  1546,  1389,    45,   130,  1445,   106,
     107,   108,  1063,  1058,   137,     8,  1067,     9,   870,   301,
     302,   303,     0,    14,    15,    16,   850,     5,    21,     7,
      23,   307,   155,    47,    23,   170,   171,    26,  1298,   162,
    1437,  1438,  1011,    96,   179,    47,   870,  1444,  1445,  1446,
    1447,    47,   353,   850,   850,   315,   316,  1475,   352,   263,
    1437,  1438,  1439,  1440,  1441,  1442,  1443,  1444,  1445,  1446,
    1447,     8,     8,   870,   870,    21,  1486,    23,   294,  1489,
     106,   315,   316,   136,    21,  1002,    23,    78,    79,    80,
    1146,     1,  1148,   313,   311,     5,   312,     7,  1154,   106,
    1833,   317,  2143,   319,    95,   131,   132,   137,   256,   257,
       1,   105,   260,   104,     5,   372,     7,    27,    28,    29,
      30,   229,   113,   114,   372,   233,   234,   235,   119,   370,
     166,   351,   850,    16,    17,   436,    27,    28,    29,    30,
     170,   171,     6,   173,   135,   136,   137,   367,  1634,   105,
     451,  1411,   870,  1633,   145,   146,   344,    47,   149,  1011,
    1570,   344,   444,   445,   446,   338,   339,   118,   116,   361,
       1,   369,    62,    63,     5,     5,     7,     7,  1291,     9,
      87,   370,    92,   134,   179,   133,    24,  1011,    24,    99,
     138,   139,    16,    17,     6,   477,    27,    28,    29,    30,
    1610,  1611,    54,  1613,   155,  2094,  1606,  1749,    54,    40,
      54,   162,    54,  1631,  1011,  1011,   106,     1,   105,   106,
    1630,     5,   112,     7,    87,   116,   136,   280,   510,   116,
     256,   133,   122,  1726,     5,   137,     7,     8,   128,    54,
    1298,    16,    17,    27,    28,    29,    30,  1624,   114,   115,
     276,   277,   366,     1,   351,   105,   106,     5,    42,     7,
     105,   171,   152,   196,   197,  1642,   116,    87,   170,   171,
     367,   307,   308,   309,   310,   311,  1389,   105,  1389,    27,
      28,    29,    30,   105,  1721,  1722,   137,  1696,   105,  1698,
      87,   105,   106,  1011,    18,   286,   287,   245,   351,   105,
     251,   252,   116,   114,   115,  1858,   105,    91,    92,    93,
      94,   105,    96,   105,   367,    99,   114,   115,   116,   170,
     171,    47,    19,    49,  1721,  1722,  1703,   370,    54,  1298,
      56,     6,   116,   105,    60,     8,  1381,  1382,  1383,  1384,
    1385,     8,  1387,  1388,  1721,  1722,   228,   229,   230,   231,
     232,  1396,  1397,  1411,     8,   206,     8,   850,     1,  1619,
     194,   195,     5,   105,     7,     6,  1852,  1847,     5,     6,
     280,   114,   115,  1791,   100,     8,   102,   870,    62,    63,
    1922,  1923,   166,  1925,    27,    28,    29,    30,   239,   280,
     105,   106,  1437,  1438,  1439,  1440,  1441,  1442,  1443,  1444,
    1445,  1446,  1447,     8,  1462,    97,    98,  1458,  1459,   299,
    1461,  1462,  1463,  1464,   366,   279,    47,    18,    49,    45,
      46,  1459,  1840,  1461,  1462,  1463,  1464,    19,   112,   370,
     294,    62,    63,     9,  2144,   299,  1396,  1397,   122,  2084,
    2085,   351,  1411,   105,   128,   240,  1298,  2251,   312,   105,
     106,  1507,  1994,   317,   249,   250,     8,   367,   253,   254,
     351,   105,  1462,   106,   107,   108,    40,   279,   152,  1879,
     337,   338,   339,  1952,  1298,   106,   367,  1949,   366,  1556,
    1557,   112,   294,   296,  1950,   366,   366,   299,  2292,   366,
     366,   122,   313,  1462,   105,   279,   105,   128,  1554,  1555,
     312,  1298,  1298,   367,  1617,   317,  1619,  1620,   105,   366,
     294,   502,   105,   366,  1889,   299,   106,   105,  1011,   105,
     351,   152,   105,   193,  1575,   133,    99,  1578,   312,  1580,
     198,  1642,  1583,   317,   233,   236,   367,  1575,   236,   821,
    1578,   130,  1580,   237,  1947,  1583,   105,   238,  1606,   850,
     541,   237,   105,  1644,  1982,   367,   105,   146,    48,  1411,
      50,  1619,    52,    53,   105,   313,   155,   351,   345,   870,
      60,    61,   105,   162,   163,   140,   141,   142,   143,  1624,
    1298,   366,  2010,   367,   366,   366,  1644,  1411,   366,   105,
    1646,   105,     9,  1644,     1,     6,   105,  1642,     5,    54,
       7,    23,    45,   351,    94,   314,  1606,    88,    89,    90,
    1462,    92,  1872,   151,  1411,  1411,    30,    21,   314,   367,
      27,    28,    29,    30,     6,  1959,  1960,  1961,  1962,   369,
     105,  2041,   352,   353,   354,  1748,    87,  1606,  1462,    53,
       9,  2051,   114,   115,  1644,  2198,    87,    23,  2058,    23,
    1619,    65,   321,   322,   323,   324,   325,   129,  1703,   131,
     132,   133,   106,   107,   108,  1462,  1462,   369,   299,   333,
     334,   335,  2072,  1933,   105,  1644,  1721,  1722,   224,   225,
     226,   227,   154,   155,   156,    92,    99,   100,   101,   102,
     366,     8,    99,  1411,  1745,   366,   373,   105,  2191,   279,
     366,  1759,   297,  1754,  1755,  1756,  1757,  1758,   351,     6,
    1011,  2197,   373,   106,   294,   366,  2196,  2094,  1776,   299,
    1771,  1772,  1773,  1774,   367,  2267,  2136,  2137,  2138,   136,
      45,   193,   312,   106,  1785,  1786,  1787,   317,   116,  1797,
     118,   119,   120,   121,  1462,  1796,  1797,  1798,  1799,   313,
    1808,     8,    21,   105,  1606,   105,  2298,  1808,  1796,  1797,
    1798,  1799,  2304,  2316,   171,    72,     6,  1619,   318,   168,
    1808,  2105,  2106,  2107,     8,  1888,  1889,  2195,   105,    41,
     106,   365,  1606,   101,   365,  1836,  1837,  1838,   365,   105,
    1759,   101,  1644,   106,   365,  1619,   105,  1797,  1836,  1837,
    1838,     8,   216,   105,   105,  1298,   220,  1776,  1808,  1606,
    1606,   116,  2305,   118,  1872,  1732,  1733,  1734,  1735,   367,
    1644,   105,  1619,  1619,    87,   296,     6,   192,  1797,  2315,
     105,  2091,   137,     8,  2314,   106,     9,     9,   105,  1808,
     145,   146,   147,   148,   365,   153,   105,  1644,  1644,  2259,
     105,  1964,   367,   107,   189,  1906,    81,    82,    83,    84,
      85,    86,   114,   115,   366,   366,   366,   366,   281,  1982,
     366,  1982,   366,   280,   366,  1933,   128,   290,   291,   292,
     293,   114,   115,   366,   136,    43,   366,     8,  1606,   366,
     123,   124,   125,   126,   127,  2313,   366,  2010,  2040,  2010,
     152,  1619,   367,  1872,  2017,   136,   367,  1759,  2050,   366,
     115,   116,   366,   118,   106,  2057,     9,     9,  1411,  2329,
    2330,  2331,   105,   105,  1776,   106,  1644,   105,     6,  1980,
    1981,   365,   137,  1029,   922,  1759,   796,  1031,  1030,   353,
     145,   146,   797,   148,   351,  1797,   795,   794,   575,  2000,
    1494,   565,  1776,   158,   159,   160,  1808,  2008,  2009,  1139,
     367,  2012,  1759,  1759,  1933,  1033,    20,  2018,   282,  1462,
     134,  1888,  1889,  1797,  2025,  2026,  2027,   283,   438,  1776,
    1776,  2104,  1467,  1781,  1808,  2124,  2130,  2025,  2026,  2027,
    2252,  2177,  2122,  2178,   408,  2179,  2168,  1298,     1,  2112,
    1797,  1797,     5,  2180,     7,   419,   420,  2294,  1734,   288,
     435,  1808,  1808,   892,  2072,  1937,   377,  1908,   904,  2132,
    1872,  1857,   436,  1276,    27,    28,    29,    30,   280,   281,
     282,   283,  1326,  2091,  1845,  1577,  1846,   451,  1577,  1656,
      43,  1759,   437,    46,   296,   297,   298,  2098,  1872,  2094,
    1658,   978,  2167,  1383,  1385,  1384,  2239,  1402,  1776,  2117,
       1,  1617,  2113,   998,     5,  1619,     7,   871,  2126,  1409,
     320,    30,  2072,  1129,  1447,  1872,  1872,   216,  1872,  1797,
    1330,  1933,  2133,  2134,  1607,  1640,    27,    28,    29,    30,
    1808,   420,    30,   116,  2145,  2146,   447,  2148,  2149,  2150,
    2151,  2152,  1640,  2072,  2155,   128,   449,   130,   450,  1933,
    1411,  2302,   448,  1606,     1,    53,   333,   721,     5,   334,
       7,   334,  2091,   698,  1198,   148,  1619,    65,  2222,   152,
      -1,   665,   155,    -1,  2192,    -1,  1933,  1933,    -1,  2252,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,  2117,    -1,
    2201,  1644,    -1,  2204,  1872,    -1,  2207,  2126,    -1,    -1,
      -1,  1462,    -1,   166,   105,  1459,    -1,  1461,  1462,  1463,
    1464,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2292,
      -1,    -1,    -1,  2241,    -1,    -1,    -1,    -1,    -1,  2240,
    2241,  2242,  2243,    -1,    -1,    -1,    -1,  2248,  2249,  2250,
    2251,    -1,  2240,  2241,  2242,  2243,  2319,  2265,    -1,    96,
    2248,  2249,  2250,  2251,    -1,  1933,    -1,  2268,    -1,    -1,
    2072,   244,    -1,  2192,    -1,  2276,    -1,  2278,  2279,  2280,
    2281,  2282,  2283,  2284,  2285,  2286,  2287,  2288,  2289,  2091,
      -1,  2241,    -1,    -1,    -1,  2283,  2284,  2285,  2072,   136,
      -1,    -1,  2303,    -1,   277,   278,    -1,    -1,    -1,  2310,
    2311,  2312,    -1,    -1,    -1,  2117,  1759,  2091,    -1,    -1,
      -1,    -1,  2241,    -1,  2126,  2072,  2072,    -1,   216,    -1,
      -1,  1575,   220,  1776,  1578,    -1,  1580,    -1,    -1,  1583,
      -1,   294,    -1,  2117,  2091,  2091,  2265,     1,    -1,    -1,
      -1,     5,  2126,     7,  1797,  1606,    -1,    -1,    -1,    -1,
      -1,    -1,   847,    -1,   849,  1808,    -1,    -1,  1619,   137,
    2117,  2117,    -1,    27,    28,    29,    30,    -1,   146,  2126,
    2126,   866,   867,    -1,   869,    -1,   359,   872,   873,    43,
    2192,    -1,    46,  1644,   162,    -1,  1640,    -1,   166,    -1,
      -1,    -1,   170,   171,  2072,    -1,    -1,   175,   176,   177,
     178,   179,    -1,    -1,   367,    -1,    -1,    -1,  2192,    -1,
      -1,    -1,    -1,  2091,   315,   316,    -1,    -1,    -1,  1872,
      -1,    -1,    -1,    -1,    -1,   920,    -1,    -1,   923,  2241,
      -1,    -1,    -1,   280,     1,  2192,  2192,    -1,     5,  2117,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2126,    -1,
     351,    -1,    -1,  2265,    -1,   353,    -1,  2241,   115,    -1,
      27,    28,    29,    30,   121,    -1,   367,   124,   125,   126,
     127,    -1,    -1,    -1,   131,    42,   850,    -1,    -1,    -1,
    1933,  2265,   139,   140,  2241,  2241,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   989,    -1,   870,    -1,  1759,    -1,
      -1,    -1,   166,   877,   351,    -1,    -1,    -1,  2265,  2265,
     408,    -1,    -1,    -1,  2192,  1776,    -1,    -1,    -1,    -1,
     367,   419,   420,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    -1,    99,    -1,    -1,    -1,  1797,    -1,   436,    -1,
      -1,    -1,  1796,  1797,  1798,  1799,     1,  1808,    -1,   116,
       5,     1,     7,   451,  1808,     5,    -1,     7,    -1,    -1,
      -1,    -1,    -1,  2241,  1059,  1060,    -1,   941,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,    -1,    27,    28,    29,
      30,    -1,  1836,  1837,  1838,    -1,    -1,  2265,    43,     1,
      -1,    46,    42,     5,    -1,     7,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   114,   115,    -1,    -1,   118,
      -1,  1872,    -1,    -1,    -1,    27,    28,    29,    30,  2072,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    -1,
     294,    -1,  1006,  1007,  1008,  1009,  1010,  1011,  2091,  1013,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    -1,    99,
      -1,     1,    -1,    -1,    -1,     5,    -1,     7,    -1,    -1,
      -1,    -1,    -1,    -1,  2117,  1039,   116,    -1,    -1,    -1,
     179,    -1,  1933,  2126,    -1,    -1,    -1,    27,    28,    29,
      30,     1,    -1,    -1,  1058,     5,    -1,     7,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   367,   116,    -1,    -1,    27,    28,    29,
      30,   166,   279,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    46,   294,    -1,    -1,
      -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,  2192,
      -1,    -1,    92,    93,    94,   312,    96,    -1,    -1,    99,
     317,    -1,    -1,     1,    -1,    -1,    -1,     5,    -1,     7,
      -1,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,
      -1,  2025,  2026,  2027,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    30,    -1,   351,    -1,    -1,    -1,  2241,    -1,
    1285,    -1,    -1,    -1,  1289,     1,    -1,    -1,    -1,     5,
     367,     7,    -1,    -1,  1299,  1300,  1301,  1302,    -1,  1304,
      -1,  2072,  2265,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    27,    28,    29,    30,  1320,    -1,    -1,    -1,   279,
    2091,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   294,
      -1,    -1,    -1,    -1,   294,    -1,   166,    -1,    96,   299,
      -1,    -1,    -1,    -1,    -1,     1,  2117,    -1,    -1,     5,
      -1,     7,   312,    -1,    -1,  2126,    -1,   317,   280,    -1,
      -1,  1366,    -1,  1368,    -1,    -1,    -1,    -1,    -1,    -1,
    1375,    27,    28,    29,    30,    -1,    92,    -1,   136,    -1,
      -1,    -1,    -1,    99,     1,  1390,    42,  1392,     5,    -1,
       7,   351,    -1,    -1,    -1,    -1,  1401,    -1,  1403,    -1,
      -1,    -1,   367,    -1,    -1,    -1,    -1,   367,    -1,    -1,
      27,    28,    29,    30,  1298,    -1,    -1,    -1,    -1,   279,
     136,  2192,     1,    -1,    -1,    -1,     5,    -1,     7,   351,
      -1,    -1,    -1,    -1,   294,    -1,    92,    93,    94,   299,
      96,    -1,   850,    99,    -1,   367,    -1,    -1,    27,    28,
      29,    30,   312,    -1,    -1,   171,    -1,   317,    -1,    -1,
     116,    -1,   870,    -1,   294,    -1,    -1,    -1,    -1,   877,
    2241,    -1,    -1,    -1,    -1,    -1,  2240,  2241,  2242,  2243,
      -1,    -1,    -1,    -1,  2248,  2249,  2250,  2251,    -1,    -1,
      -1,   351,    -1,    -1,  2265,    -1,    -1,  1381,  1382,  1383,
    1384,  1385,    -1,  1387,  1388,    -1,    -1,   367,    -1,    -1,
     166,    -1,  1396,  1397,    -1,    -1,    -1,    -1,    -1,  2283,
    2284,  2285,   280,    -1,    -1,    -1,    -1,  1411,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   367,    -1,     1,
      -1,    -1,    -1,     5,    -1,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1437,  1438,  1439,  1440,  1441,  1442,  1443,
    1444,  1445,  1446,  1447,   280,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1462,    -1,
      -1,  1586,  1587,    -1,     1,    -1,    -1,   995,     5,    -1,
       7,    -1,    -1,   351,    -1,    -1,    -1,    -1,  1006,  1007,
    1008,  1009,  1010,  1011,    -1,  1013,    -1,  1612,    -1,   367,
      27,    28,    29,    30,    -1,    -1,  1621,    -1,    -1,    -1,
      -1,    -1,    39,   279,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1039,    49,    -1,    -1,   351,    -1,    54,   294,    56,
      -1,    58,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,
    1058,   367,    -1,    -1,   116,    -1,   312,    -1,    -1,    -1,
      -1,   317,    -1,    -1,    -1,    -1,    39,  1672,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    51,   296,
      -1,    54,    55,    56,  1689,    58,  1691,    -1,    -1,    -1,
      -1,     1,    -1,    -1,    -1,   351,    -1,    -1,    -1,    -1,
      -1,    11,    12,    13,    14,    15,    16,    17,    -1,    -1,
      20,   367,    -1,    39,    -1,    -1,    -1,   296,    -1,    -1,
      -1,    47,  1606,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    -1,    58,    -1,   351,  1619,    -1,    -1,    -1,    -1,
    1624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1642,    -1,
    1644,    -1,    -1,    73,    -1,    -1,    76,    77,    78,    79,
      80,    81,   351,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,   367,    99,
     100,   101,   102,    -1,   104,    -1,    -1,    -1,   108,    -1,
     110,    -1,   112,   113,   114,   115,   116,    -1,    -1,   119,
      -1,   121,    -1,    -1,   124,   125,   126,   127,   280,  1703,
      -1,   131,    -1,   133,   134,   135,   136,   137,    -1,   139,
     140,    -1,    -1,   143,   144,   145,   146,  1721,  1722,   149,
       1,   151,    -1,    -1,     5,    -1,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1865,    -1,   279,    -1,    -1,    -1,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   294,    -1,    -1,
      -1,    42,  1887,    -1,    -1,    -1,    -1,    -1,    -1,   351,
    1298,    -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,    -1,
     317,    -1,   319,    -1,    -1,   367,   279,    -1,    -1,    -1,
      -1,    -1,    -1,  1797,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   294,  1927,    -1,  1808,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,   351,    96,    -1,    -1,    99,   312,
     106,    -1,    -1,    -1,   317,    -1,   319,    -1,    -1,    -1,
     367,  1359,  1360,   279,  1362,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   294,    -1,
     136,   281,   282,   283,   284,   285,   286,   287,   288,    -1,
     290,   291,   292,   293,    -1,    -1,   312,   153,  1872,    -1,
      -1,   317,    -1,   319,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    -1,  1411,    -1,   166,    -1,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,     1,    54,    55,    56,     5,
      58,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1437,
    1438,  1439,  1440,  1441,  1442,  1443,  1444,  1445,  1446,  1447,
      -1,    27,    28,    29,    30,    -1,    -1,    -1,    -1,  1933,
      -1,    -1,    -1,    39,  1462,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    -1,    -1,  2089,    78,    -1,    -1,    -1,    -1,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,    -1,    -1,   106,   107,   108,    -1,    -1,   279,    -1,
     113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,
      -1,    -1,    -1,   294,    -1,   128,    -1,    -1,   299,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   312,    -1,   319,    -1,    -1,   317,   323,   324,   152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   333,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     166,   491,    -1,    -1,    -1,    -1,    -1,    -1,  2072,    -1,
     351,    -1,   502,   359,    -1,    -1,    -1,    -1,  1606,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   367,  2091,    -1,  1617,
    2094,  1619,  1620,   523,    -1,    -1,  1624,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,   541,    -1,    -1,  1642,    -1,  1644,    -1,    -1,     1,
      -1,   279,    -1,     5,    -1,     7,    -1,    -1,    -1,    -1,
     114,   115,    -1,    -1,   118,  1663,   294,    -1,  1666,    -1,
      -1,  1669,    24,    -1,    -1,    27,    28,    29,    30,    31,
      -1,    33,   136,    -1,   312,    37,    38,    39,    -1,   317,
      -1,   319,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,   279,    -1,  1703,   289,    -1,    -1,   292,
     293,    -1,    64,    65,    -1,    -1,   299,    69,   294,    -1,
      72,    -1,    -1,  1721,  1722,    -1,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    86,    -1,   312,    -1,    -1,   193,
      -1,   317,    -1,   319,    96,    -1,   329,   330,   100,    -1,
     102,    -1,    -1,   105,   106,   107,   108,    -1,    -1,   111,
      -1,   113,    -1,    -1,   116,   117,    -1,  2241,    -1,    -1,
     122,    -1,    -1,    -1,    -1,   351,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   371,    -1,
      -1,   367,   144,    -1,    -1,    -1,    -1,   149,    -1,  1797,
     152,    -1,   256,    -1,    -1,   157,    -1,    -1,    -1,   161,
    1808,    -1,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,   174,    -1,    -1,    -1,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
      -1,    -1,   296,   297,   298,    -1,    -1,   199,   200,    -1,
     202,   203,   204,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,   319,    -1,    -1,    -1,    -1,
      -1,    47,    -1,    49,  1872,    51,    -1,    -1,    54,    55,
      56,    -1,    58,    -1,    -1,    -1,    62,    63,   240,   241,
     242,   243,    -1,    -1,    -1,    -1,    -1,   249,   250,    -1,
      -1,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,    -1,    -1,   279,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1933,   112,    -1,    -1,    -1,
      -1,    -1,   294,    -1,    -1,    -1,   122,   299,    -1,    -1,
      -1,    -1,   128,    -1,   306,    -1,    -1,    -1,    -1,    -1,
     312,    -1,    -1,    -1,    -1,   317,    -1,    -1,    -1,    -1,
      -1,   323,   324,    -1,   326,    -1,   152,   329,   330,   331,
     332,    -1,    -1,    -1,   336,    -1,    -1,    -1,   340,   341,
     342,    -1,    -1,    -1,   346,   347,    -1,    -1,    -1,   351,
      -1,    -1,    -1,   355,    -1,     1,   358,   359,    -1,     5,
      -1,     7,   364,    -1,   366,   367,   368,    -1,    -1,   371,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    24,    -1,
      -1,    27,    28,    29,    30,    31,    -1,    33,    -1,    -1,
      40,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    65,
      -1,    -1,   106,    69,  2072,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    -1,    -1,  2091,    -1,    -1,  2094,    -1,    -1,   133,
      96,    -1,   136,   279,   100,   105,   102,    -1,    -1,   105,
     106,   107,   108,    -1,    -1,   111,    -1,   113,   294,   153,
     116,   117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,
      -1,    -1,   128,    -1,    -1,    -1,   312,    -1,    -1,    -1,
      -1,   317,    -1,   319,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,   177,   174,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,   204,   205,
      -1,    -1,    -1,    -1,    -1,    -1,  1116,    -1,    -1,    -1,
      -1,    -1,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,  2241,   240,   241,   242,   243,    -1,    -1,
      -1,    -1,    -1,   249,   250,    -1,    -1,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,    -1,    -1,   279,    -1,   319,    -1,    -1,    -1,   323,
     324,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   294,   333,
      -1,    -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,
     306,    -1,    -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,
      -1,   317,    -1,    -1,    -1,   359,    -1,   323,   324,    -1,
     326,    -1,    -1,   329,   330,   331,   332,    -1,    -1,    -1,
     336,    -1,    -1,    92,   340,   341,   342,    -1,    -1,    -1,
     346,   347,    -1,    -1,     1,   351,    -1,    -1,     5,   355,
       7,    -1,   358,   359,    -1,   114,   115,    -1,   364,   118,
     366,   367,   368,    -1,    -1,   371,    -1,    24,    -1,    -1,
      27,    28,    29,    30,    31,    -1,    33,   136,    -1,    -1,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,
      -1,    -1,    69,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
      -1,    -1,    -1,   100,    -1,   102,    -1,    -1,   105,   106,
     107,   108,    -1,    -1,   111,    -1,   113,    -1,    -1,   116,
     117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,   256,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,
      -1,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,    -1,    -1,   296,   297,   298,
      -1,    -1,   199,   200,    -1,   202,   203,   204,   205,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     319,    -1,    -1,    -1,    -1,    -1,   114,   115,    -1,    -1,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   240,   241,   242,   243,    -1,   136,    -1,
      -1,    -1,   249,   250,    -1,    -1,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
      -1,   177,   279,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   294,    -1,    -1,
      -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,   306,
      -1,    -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,    -1,
     317,    -1,    -1,    -1,    -1,    -1,   323,   324,    -1,   326,
      -1,    -1,   329,   330,   331,   332,    -1,    -1,    -1,   336,
      -1,    -1,    -1,   340,   341,   342,    -1,    -1,    -1,   346,
     347,    -1,    -1,     1,   351,    -1,    -1,     5,   355,     7,
      -1,   358,   359,    -1,    -1,    -1,    -1,   364,   256,   366,
     367,   368,    -1,    -1,   371,    -1,    24,    -1,    -1,    27,
      28,    29,    30,    31,    -1,    33,    -1,    -1,    -1,    37,
      38,    39,   280,   281,   282,   283,   284,   285,    46,   287,
     288,   289,   290,   291,   292,   293,    54,    -1,   296,   297,
     298,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    -1,
      -1,    69,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,   319,    -1,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    -1,    -1,   111,    -1,   113,    -1,    96,    -1,
     117,    -1,   100,    -1,   102,   122,    -1,   105,   106,   107,
     108,   128,    -1,   111,    -1,   113,    -1,    -1,   116,   117,
      -1,    -1,    -1,    -1,   122,    -1,    -1,   144,    -1,    -1,
     128,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,   144,   164,   165,   166,
      -1,   149,   169,    -1,   152,   172,    -1,   174,    -1,   157,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,   166,    -1,
      -1,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   199,    -1,   201,   184,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   199,   200,    -1,   202,   203,   204,   205,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   240,   241,   242,   243,    -1,    -1,    -1,    -1,
      -1,   249,   250,    -1,    -1,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,    -1,
      -1,   279,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   294,    -1,    -1,    -1,
      -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,   306,    -1,
      -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,    -1,   317,
      -1,    -1,    -1,    -1,    -1,   323,   324,    -1,   326,    -1,
      -1,   329,   330,   331,   332,    -1,    -1,    -1,   336,    -1,
      -1,    -1,   340,   341,   342,    -1,    -1,    -1,   346,   347,
      -1,    -1,     5,   351,     7,    -1,    -1,   355,    -1,    -1,
     358,   359,    -1,    -1,    -1,    -1,   364,    -1,   366,   367,
     368,    24,    -1,   371,    -1,    -1,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    65,    -1,    -1,    -1,    69,    -1,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    -1,    -1,   111,
      -1,   113,    -1,    -1,   116,   117,    -1,   100,    -1,   102,
     122,    -1,   105,   106,   107,   108,   128,    -1,   111,    -1,
     113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,
      -1,    -1,   144,    -1,    -1,   128,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,   161,
      -1,   144,   164,   165,    -1,    -1,   149,   169,    -1,   152,
     172,    -1,   174,    -1,   157,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   199,    -1,   201,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,
     203,   204,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   240,   241,   242,
     243,    -1,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,    -1,    -1,   279,    -1,    -1,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   294,    -1,    -1,    -1,    -1,   299,    24,    -1,    -1,
      -1,    -1,    -1,   306,    31,    -1,    33,    -1,    -1,   312,
      -1,    38,    -1,    -1,   317,    -1,    -1,    -1,    -1,    -1,
     323,   324,    -1,   326,    -1,    -1,   329,   330,   331,   332,
      -1,    -1,    -1,   336,    -1,    -1,    -1,   340,   341,   342,
      -1,    -1,    -1,   346,   347,    72,    -1,    -1,    -1,    -1,
      -1,    78,   355,    -1,    -1,   358,   359,    -1,    -1,    -1,
      87,   364,    89,   366,   367,   368,    -1,    -1,   371,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,    -1,   111,    -1,   113,    -1,    -1,    -1,
     117,    -1,    -1,    -1,    -1,   122,    -1,    -1,   111,    -1,
     113,   128,    -1,    -1,   117,   132,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,   128,    -1,   144,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   144,    -1,    -1,   161,    -1,   149,   164,   165,   152,
      -1,    -1,   169,    -1,   157,   172,    -1,   174,   161,    -1,
      -1,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,   204,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      39,    -1,   205,    -1,    -1,    44,    -1,    -1,    47,    48,
      49,    50,    -1,    -1,    53,    54,    -1,    56,    57,    58,
      -1,    60,    -1,   240,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
       6,   100,   279,   102,   103,   104,    -1,    -1,    -1,    -1,
      -1,    -1,   289,    -1,    -1,   292,   293,    -1,    24,    -1,
      -1,    -1,   299,    -1,    -1,    31,    -1,    33,    -1,    -1,
     307,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   323,   324,    -1,    -1,
      -1,    -1,   329,   330,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   342,    72,    -1,    -1,   346,
     347,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,   356,
     357,    -1,   359,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,   368,    -1,    -1,   371,    -1,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   128,   111,    -1,   113,    -1,    -1,
      -1,   117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,
     144,    -1,   128,    -1,    -1,   149,   132,    -1,   152,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,   161,   144,    -1,
     164,   165,    -1,   149,    -1,   169,   152,    -1,   172,    -1,
     174,   157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     279,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,   200,   294,   202,   203,   204,   205,
      -1,    -1,   301,   302,   303,   304,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   312,    -1,    -1,    -1,    -1,   317,    -1,
     319,   320,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   240,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,     6,    -1,   279,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   289,    -1,    -1,   292,   293,    -1,    24,
      -1,    -1,    -1,   299,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   323,   324,    -1,
      -1,    -1,    -1,   329,   330,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   342,    72,    -1,    -1,
     346,   347,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
     356,   357,    -1,   359,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   368,    -1,    -1,   371,    -1,    -1,    -1,    -1,
     105,   106,   107,   108,    -1,    -1,   111,    -1,   113,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,   128,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,   204,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   240,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,     6,    -1,   279,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   289,    -1,    -1,   292,   293,    -1,
      24,    -1,    -1,    -1,   299,    -1,    -1,    31,    -1,    33,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   323,   324,
      -1,    -1,    -1,    -1,   329,   330,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   342,    72,    -1,
      -1,   346,   347,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,   356,   357,    -1,   359,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   368,    -1,    -1,   371,    -1,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,    -1,   111,    -1,   113,
      -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
     204,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   240,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,     6,    -1,   279,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   289,    -1,    -1,   292,   293,
      -1,    24,    -1,    -1,    -1,   299,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   323,
     324,    -1,    -1,    -1,    -1,   329,   330,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   342,    72,
      -1,    -1,   346,   347,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,   356,   357,    -1,   359,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   368,    -1,    -1,   371,    -1,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,    -1,   111,    -1,
     113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,
     203,   204,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,   240,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,    -1,    -1,   279,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    78,    -1,   289,    -1,    -1,   292,
     293,    -1,    -1,    -1,    -1,    -1,   299,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,    -1,    -1,    -1,   113,
     323,   324,    -1,   117,    -1,    -1,   329,   330,   122,    22,
      -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,   132,   342,
      -1,    -1,    -1,   346,   347,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,   356,   357,   149,   359,    -1,   152,    -1,
      -1,    -1,    -1,   157,    -1,   368,    -1,   161,   371,    -1,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,    72,
     174,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,   202,    -1,
      -1,   205,   105,   106,   107,   108,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,    -1,    22,   169,    -1,    -1,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   289,    -1,    -1,   292,   293,
      -1,    -1,    -1,    -1,    -1,   299,   199,    -1,    -1,   202,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,   329,   330,    -1,   332,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,
     107,   108,   356,   357,    -1,    -1,   113,    -1,    -1,    -1,
     117,    -1,   366,    -1,   368,   122,    -1,   371,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   289,   144,    -1,   292,
     293,    -1,   149,    -1,    -1,   152,   299,    -1,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,   329,   330,    -1,   332,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,    -1,    -1,   202,    -1,    -1,   205,    -1,
      -1,    -1,    -1,   356,   357,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   366,    -1,   368,   113,    -1,   371,    -1,
     117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,
      -1,    -1,   289,    -1,    -1,   292,   293,    -1,    -1,    -1,
      -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,    -1,    -1,   202,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   329,   330,    -1,   332,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   356,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,   368,    -1,    -1,   371,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,   289,    -1,    -1,   292,   293,    -1,    64,    65,
      -1,    -1,   299,    69,    -1,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   329,   330,   100,   332,   102,    -1,    -1,   105,
     106,   107,   108,    -1,    -1,   111,    -1,   113,    -1,    -1,
      -1,   117,    -1,    -1,    -1,    -1,   122,    -1,    -1,   356,
     357,    -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,   366,
      -1,   368,    -1,    -1,   371,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,   204,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   240,   241,   242,   243,    -1,    -1,
      -1,    -1,    -1,   249,   250,    -1,    -1,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,    -1,    -1,   279,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,   294,    -1,
      -1,    -1,    31,   299,    33,    -1,    -1,    -1,    -1,    38,
     306,    -1,    -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,
      -1,   317,    -1,    -1,    -1,    -1,    -1,   323,   324,    -1,
     326,    -1,    -1,   329,   330,   331,   332,    -1,    -1,    -1,
     336,    -1,    -1,    72,   340,   341,   342,    -1,    -1,    78,
     346,   347,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   355,
      -1,    -1,   358,   359,    -1,    -1,    -1,    -1,   364,    -1,
     366,    -1,   368,    -1,    -1,   371,   105,   106,   107,   108,
      -1,    -1,   111,    -1,   113,    -1,    -1,    -1,   117,    -1,
      -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,   128,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,    -1,    -1,
     169,    -1,    -1,   172,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,   200,    -1,   202,   203,   204,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   240,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,    -1,    -1,
     279,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     289,    -1,    24,   292,   293,    -1,    -1,    -1,    -1,    31,
     299,    33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   323,   324,    -1,    -1,    -1,    -1,
     329,   330,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   342,    -1,    -1,    -1,   346,   347,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   356,   357,    -1,
     359,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   368,
      -1,    -1,   371,   105,   106,   107,   108,    -1,    -1,   111,
      -1,   113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,
     122,    -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,   204,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    33,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   240,   241,
     242,   243,    -1,    -1,    -1,    -1,    -1,   249,   250,    -1,
      -1,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,    -1,    -1,   279,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,    -1,   111,    -1,   113,
      -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,    -1,    -1,
      -1,   323,   324,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
     342,    -1,    -1,   157,   346,   347,    -1,   161,    -1,    -1,
     164,   165,    -1,    -1,    -1,   169,   358,   359,   172,    -1,
     174,    -1,    -1,    -1,   366,    -1,   368,    -1,    -1,   371,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
     204,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   240,   241,   242,   243,
      -1,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,    72,    -1,   279,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,   117,   323,
     324,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,   128,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,   342,    -1,
      -1,    -1,   346,   347,    -1,   144,    -1,    -1,    -1,    -1,
     149,    -1,    -1,   152,   358,   359,    -1,    -1,   157,    -1,
      -1,    -1,   161,    -1,   368,   164,   165,   371,    -1,    -1,
     169,    24,    -1,   172,    -1,   174,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,   111,    -1,
     113,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     289,   144,    -1,   292,   293,    -1,   149,    -1,    -1,   152,
     299,    -1,    -1,    -1,   157,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     329,   330,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,
     203,   204,   205,    -1,    -1,    -1,    -1,   356,   357,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   366,    33,   368,
      -1,    -1,   371,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   240,   241,   242,
     243,    -1,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,    -1,    -1,   279,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   294,    -1,    -1,    -1,    -1,   111,    -1,   113,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,   122,    -1,   312,
      -1,    -1,    -1,   128,   317,    -1,   319,    -1,    -1,    -1,
     323,   324,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,   336,   149,    -1,    -1,   152,    -1,   342,
      -1,    -1,   157,   346,   347,    -1,   161,    -1,    -1,   164,
     165,    -1,    -1,    -1,   169,   358,   359,   172,    -1,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,   204,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   240,   241,   242,   243,    -1,
      -1,    -1,    -1,    -1,   249,   250,    -1,    -1,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    -1,    -1,   279,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     0,     1,    -1,    -1,    -1,     5,    -1,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   323,   324,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    -1,   342,    -1,    -1,
      39,   346,   347,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      49,    -1,    51,   358,   359,    54,    55,    56,    -1,    58,
      -1,    -1,    -1,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   122,    -1,    -1,   113,    -1,    -1,   128,
     117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,    72,    -1,    -1,    -1,
     157,    -1,    78,    -1,   161,    -1,    -1,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,    -1,    -1,    -1,   113,    -1,    -1,
      -1,   117,   199,    -1,    -1,   202,   122,    -1,   205,    -1,
      -1,    -1,   128,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     279,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,    -1,   294,   202,    -1,    -1,   205,
     299,    -1,   289,    -1,    -1,   292,   293,    -1,    -1,    -1,
      -1,    -1,   299,   312,    -1,    -1,    -1,    -1,   317,    -1,
     319,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   329,   330,    -1,   332,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   356,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   368,    -1,   289,   371,    -1,   292,   293,   105,   106,
     107,   108,    -1,   299,    -1,    -1,   113,    -1,    -1,    -1,
     117,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   329,   330,    -1,   332,   144,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     356,   357,   169,    -1,    -1,   172,    -1,   174,    -1,    -1,
      -1,    -1,   368,    -1,    -1,   371,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,    -1,    -1,   202,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   289,    -1,    -1,   292,   293,    -1,    -1,    -1,
      -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   329,   330,    -1,   332,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   356,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   368,    -1,    -1,   371
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   375,     0,     1,     5,     7,    27,    28,    29,    30,
      39,    47,    49,    51,    54,    55,    56,    58,    62,    63,
      64,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,   112,   122,   128,   152,   279,
     294,   312,   317,   319,   376,   432,   433,   434,   435,   519,
     520,   521,   523,   538,   376,   107,   106,   299,   515,   515,
     515,   521,   532,   521,   523,   538,   521,   526,   526,   526,
     521,   529,   435,    51,   436,    39,    47,    49,    54,    55,
      56,    58,   279,   294,   312,   317,   319,   437,    51,   438,
      39,    47,    49,    51,    54,    55,    56,    58,   279,   294,
     312,   317,   319,   443,    55,   445,    39,    44,    47,    48,
      49,    50,    53,    54,    56,    57,    58,    60,   100,   102,
     103,   104,   279,   294,   301,   302,   303,   304,   312,   317,
     319,   320,   448,    51,    52,    54,    55,    56,   294,   301,
     302,   317,   451,    47,    49,    54,    56,    60,   100,   102,
     452,    49,   453,    24,    31,    33,    38,   105,   106,   107,
     108,   111,   113,   117,   122,   128,   144,   149,   152,   157,
     161,   164,   165,   169,   172,   174,   199,   200,   202,   203,
     204,   205,   240,   241,   242,   243,   249,   250,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   279,   323,   324,   342,   346,   347,   358,   359,
     368,   371,   461,   516,   652,   653,   656,   657,   658,   659,
     663,   727,   730,   732,   736,   741,   742,   744,   746,   756,
     758,   760,   762,   764,   766,   770,   772,   774,   776,   778,
     780,   782,   784,   786,   788,   792,   794,   796,   798,   809,
     817,   819,   821,   822,   824,   826,   828,   830,   832,   834,
     836,   838,    60,   352,   353,   354,   454,   460,    60,   455,
     460,    39,    47,    49,    51,    54,    55,    56,    58,   279,
     294,   312,   317,   319,   444,   106,   456,   457,   379,   398,
     399,    92,   286,   288,   532,   532,   532,   532,     0,   376,
     515,   515,    59,   349,   350,   535,   536,   537,    37,    39,
      54,    64,    65,    69,    72,    81,    82,    83,    84,    85,
      86,   100,   102,   255,   279,   294,   299,   306,   312,   317,
     326,   329,   330,   331,   332,   336,   340,   341,   355,   364,
     366,   542,   543,   544,   546,   547,   548,   549,   550,   551,
     552,   553,   554,   555,   556,   557,   558,   562,   563,   565,
     566,   569,   570,   572,   573,   580,   581,   586,   587,   596,
     597,   600,   601,   602,   603,   604,   625,   626,   628,   629,
     631,   632,   635,   636,   637,   647,   648,   649,   650,   651,
     658,   665,   666,   667,   668,   669,   670,   674,   675,   676,
     711,   725,   730,   731,   754,   755,   756,   799,   376,   365,
     365,   376,   515,   608,   462,   467,   542,   515,   474,   477,
     652,   676,   480,   515,   488,   523,   539,   532,   521,   523,
     526,   526,   526,   529,    92,   286,   288,   532,   532,   532,
     532,   538,   442,   521,   532,   533,   439,   519,   521,   522,
     440,   521,   523,   524,   539,   441,   521,   526,   527,   526,
     441,   521,   529,   530,    92,   286,   288,   700,   442,   442,
     442,   442,   526,   532,   450,   520,   541,   521,   541,   523,
     541,    47,   446,   447,   520,   521,   541,   526,   526,   447,
     529,   541,    47,    48,   526,   541,   447,    92,   286,   305,
     700,   701,   532,   447,   447,   447,   447,   532,   532,   532,
     447,   405,   539,    49,   446,   521,   523,   541,   526,   526,
     526,   532,   447,   447,   532,   417,   521,   523,   526,   526,
     541,    47,   526,   523,   106,   109,   110,   111,   759,   114,
     115,   256,   257,   260,   661,   662,    34,    35,    36,   256,
     733,   135,   664,   170,   171,   820,   114,   115,   116,   761,
     116,   118,   119,   120,   121,   763,   114,   115,   123,   124,
     125,   126,   127,   765,   114,   115,   118,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   179,   767,   116,   118,
     137,   145,   146,   147,   148,   771,   116,   137,   150,   308,
     773,   114,   115,   129,   131,   132,   133,   154,   155,   156,
     775,   115,   116,   118,   137,   145,   146,   148,   158,   159,
     160,   777,   130,   146,   155,   162,   163,   779,   146,   163,
     781,   155,   166,   167,   783,   133,   137,   170,   171,   785,
     137,   170,   171,   173,   787,   137,   146,   162,   166,   170,
     171,   175,   176,   177,   178,   179,   800,   116,   170,   171,
     179,   810,   166,   201,   760,   762,   764,   766,   770,   772,
     774,   776,   778,   780,   782,   784,   786,   788,   789,   790,
     791,   793,   809,   817,   819,   129,   136,   166,   395,   797,
     395,   116,   201,   791,   795,   137,   170,   171,   206,   239,
     818,   116,   128,   130,   148,   152,   155,   244,   277,   278,
     359,   743,   745,   825,   245,   827,   245,   829,   166,   246,
     247,   248,   831,   130,   155,   823,   118,   134,   155,   162,
     251,   252,   833,   130,   155,   835,   116,   130,   137,   155,
     162,   837,   106,   133,   136,   153,   319,   333,   359,   728,
     729,   730,   114,   115,   118,   136,   256,   280,   281,   282,
     283,   284,   285,   287,   288,   289,   290,   291,   292,   293,
     296,   297,   298,   319,   747,   748,   751,   333,   343,   735,
     670,   675,   344,   240,   249,   250,   253,   254,   839,   362,
     363,   404,   738,   669,   515,   423,   460,   353,   403,   460,
     391,   442,   439,   440,   523,   539,   441,   526,   526,   529,
     530,   700,   442,   442,   442,   442,   386,   409,    48,    50,
      52,    53,    60,    61,    94,   458,   532,   532,   532,   383,
     695,   710,   697,   699,   105,   105,   105,    87,   743,   295,
     648,   174,   515,   652,   726,   726,    64,   101,   515,   106,
     728,    92,   193,   286,   747,   748,   295,   295,   313,   295,
      87,   166,    87,    87,   743,   106,     5,   377,   677,   678,
     351,   540,   553,   431,   467,   105,   315,   564,   382,   383,
     296,   297,   567,   568,   318,   571,   384,   429,   166,   307,
     308,   309,   310,   311,   574,   575,   315,   316,   582,   589,
     406,   315,   316,   583,   588,   414,   416,   327,   599,   410,
       6,    72,    78,    87,    89,   113,   117,   122,   128,   132,
     152,   240,   289,   292,   293,   299,   307,   329,   330,   356,
     357,   368,   611,   612,   613,   614,   615,   616,   617,   619,
     620,   621,   622,   623,   624,   653,   656,   659,   663,   720,
     721,   722,   727,   732,   736,   742,   743,   744,   746,   752,
     753,   756,   424,   430,    40,    41,   189,   192,   605,   606,
     410,    87,   333,   334,   335,   627,   633,   634,   410,    87,
     630,   633,   388,   394,   415,   337,   338,   339,   638,   639,
     643,   644,    24,   652,   654,   655,    16,    17,    18,    19,
     370,     9,    25,    56,    10,    11,    12,    13,    14,    15,
      20,   113,   117,   122,   128,   144,   149,   152,   157,   161,
     164,   165,   169,   172,   174,   199,   202,   205,   332,   368,
     653,   655,   656,   671,   672,   673,   676,   712,   713,   714,
     715,   716,   717,   718,   719,   721,   722,   723,   724,    54,
      54,    23,   366,   693,   712,   713,   718,   693,    40,   366,
     607,   366,   366,   366,   366,   366,   535,   542,   608,   462,
     467,   474,   477,   480,   488,   532,   532,   532,   383,   695,
     710,   697,   699,   542,   430,    59,    59,    59,   467,    59,
     477,    59,   488,   532,   383,   406,   414,   421,   477,   430,
      45,   449,   521,   526,   541,   532,    47,   383,   406,   414,
     421,   523,   477,   383,   414,   526,   515,   426,     8,     9,
      32,   116,   260,   261,   660,   311,   422,   106,   129,   295,
     426,   425,   390,   425,   400,   113,   128,   113,   128,   379,
     140,   141,   142,   143,   768,   398,   425,   401,   425,   402,
     399,   425,   401,   378,   389,   381,   427,   428,    24,    40,
     105,   177,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   801,   802,   803,   425,   105,
     385,   423,   789,   395,   791,   184,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   811,   816,   419,   425,   398,   399,   404,
     745,   418,   418,   372,   418,   418,   372,   418,   397,   393,
     387,   425,   408,   407,   421,   407,   421,   114,   115,   128,
     136,   152,   280,   281,   282,   749,   750,   751,   382,   344,
     344,   105,   418,   397,   393,   387,   408,   361,   737,   369,
     430,   477,   488,   532,   383,   406,   414,   421,   459,   460,
     708,   708,   708,   296,   366,   694,   313,   366,   709,   366,
     583,   696,   366,   516,   698,     6,   128,   152,   623,    87,
     623,   645,   646,   670,   179,    24,    24,    99,   366,    54,
      54,    54,    54,   751,    54,   623,   623,   623,   640,   641,
     642,   652,   656,   670,   674,   736,   742,   643,   623,   623,
      87,   516,    22,   676,   681,   682,   683,   691,   718,   719,
       8,   367,   516,   366,   105,   105,   105,   568,   106,   300,
     517,    80,   113,   128,   174,   263,   577,   516,   105,   105,
     105,   516,   576,   575,   105,   591,   593,   594,   105,   144,
     157,   174,   328,   623,   417,   386,     6,   623,    87,   390,
     400,   379,   398,   399,   384,    87,   410,   410,   616,   653,
     722,    16,    17,    18,    19,   370,    21,    23,     9,    56,
       6,   633,    87,    89,   245,   307,     8,     8,   105,   105,
     606,     6,     8,     6,   623,   641,   652,   656,   639,     8,
     515,   366,   723,   723,   714,   715,   716,   669,   366,   559,
     654,   713,   398,   401,   399,   401,   378,   389,   381,   427,
     428,   423,   385,   395,   419,   410,   718,     8,    21,    16,
      17,    18,    19,   370,     8,    21,    23,     9,   712,   713,
     718,   623,   623,   105,   367,   376,    21,   376,   105,   503,
     430,   466,   468,   476,   485,   489,   607,   366,   366,   366,
     366,   366,   708,   708,   708,   694,   709,   696,   698,   105,
     105,   105,   366,   105,   105,   366,   708,   106,   382,   521,
     105,   662,   425,   392,   105,   412,   412,   390,   398,   390,
     398,   116,   133,   138,   139,   245,   398,   769,   380,    99,
     807,   193,   805,   198,   808,   196,   197,   806,   194,   195,
     804,   133,   385,   229,   233,   234,   235,   815,   224,   225,
     226,   227,   813,   228,   229,   230,   231,   232,   814,   814,
     233,   236,   236,   237,   238,   237,   116,   133,   166,   812,
     420,   418,   105,   105,   114,   115,   114,   115,   382,   382,
     105,   105,   345,   734,   105,   163,   360,   739,   743,   366,
     708,   366,   366,   366,   105,   496,   383,   589,   501,   406,
     497,   105,   414,   502,   421,   623,     6,     6,   623,   430,
     654,    92,    95,   540,   684,   686,    40,   177,   182,   192,
     802,   803,   516,   516,   105,   670,   679,   680,   623,   623,
     623,   623,    54,   623,   383,   406,   414,    23,   415,    87,
     337,    45,   623,   377,     6,   377,   279,   294,   312,   317,
     563,   565,   570,   581,   587,   688,   689,    92,    95,   540,
     687,   690,   377,   678,   469,   390,   151,   146,   151,   578,
     579,    21,   106,   297,   314,   590,   314,   592,    21,   106,
     297,   584,   106,   116,   598,   656,   116,   598,   423,   116,
     598,   623,     6,   623,   623,   369,   611,   611,   612,   613,
     614,   105,   616,   611,   618,   654,   676,   623,   623,    87,
       9,    87,   653,   722,   752,   752,   623,   634,   623,   633,
     644,   380,   645,   377,   560,   561,   369,   718,   712,   718,
     723,   723,   714,   715,   716,   718,   105,   712,   718,   673,
     718,    21,    21,   105,    41,   376,   367,   376,   432,   540,
     607,    39,    49,    54,    56,    58,   166,   279,   294,   312,
     317,   319,   367,   376,   432,   463,   540,    46,    96,   116,
     166,   367,   376,   432,   505,   511,   512,   540,   542,    42,
      91,    92,    93,    94,    96,    99,   116,   166,   367,   376,
     432,   475,   486,   540,   545,   552,    42,    92,    93,    94,
     116,   166,   367,   376,   432,   486,   540,   545,    43,    46,
     166,   294,   367,   376,   432,   430,   466,   468,   476,   485,
     489,   366,   366,   366,   383,   406,   414,   421,   468,   489,
     382,   382,     8,   422,   425,   398,   803,   425,   419,   373,
     373,   398,   398,   399,   399,   734,   348,   734,   105,   396,
     404,   114,   115,   740,   489,   382,   499,   500,   498,   297,
     367,   376,   432,   540,   694,   591,   593,   367,   376,   432,
     540,   709,   367,   376,   432,   540,   696,   584,   367,   376,
     432,   540,   698,   623,   623,     6,   517,   517,   686,   423,
     380,   380,   366,   553,   684,   407,   407,   382,   382,   623,
     382,   640,   652,   656,   642,   641,   623,    45,    84,    85,
     692,   719,   725,   382,   383,   384,   406,   414,   689,   106,
     685,   517,   688,   690,   367,   376,   542,   398,     8,   423,
     321,   322,   323,   324,   325,   595,   105,   105,   594,   595,
     318,   585,   598,   598,    72,   598,   623,     6,   623,   168,
     623,   633,   633,     6,   367,   545,   681,     8,   367,   712,
     712,   105,    41,   430,   515,   534,   515,   525,   515,   528,
     528,   515,   531,   106,   464,   465,    92,   286,   288,   534,
     534,   534,   534,   376,   365,    81,    82,   513,   514,   652,
     425,   101,   376,   376,   376,   376,   376,   472,   473,   657,
     517,   517,   365,    97,    98,   487,   105,   106,   131,   132,
     256,   276,   277,   493,   494,   504,    88,    89,    90,    92,
     478,   479,   376,   376,   376,   376,   552,   473,   517,   517,
     365,   494,   478,   376,   376,   376,   106,   365,   101,   383,
     367,   367,   367,   367,   367,   499,   500,   498,   367,   367,
     105,   757,     8,   411,   105,   396,   404,   367,    96,   136,
     280,   367,   376,   432,   540,   706,    92,    99,   136,   171,
     280,   367,   376,   432,   540,   707,   116,   280,   367,   376,
     432,   540,   703,   105,   383,   590,   592,   406,   414,   585,
     421,   623,   679,   367,   382,   415,   415,   623,   377,   376,
     579,   296,   106,   105,   425,   425,   417,   425,   623,    87,
     645,     6,   367,     6,   377,   561,   192,   609,   105,   495,
     467,   474,   480,   488,     8,   534,   534,   534,   495,   495,
     495,   495,   366,   490,   652,   413,   106,     9,   376,   376,
     477,   413,     9,   425,     8,   376,     6,   376,   376,   477,
       6,   376,   153,   506,   490,   376,   367,   367,   367,   380,
     105,   734,   365,   169,   174,   702,   520,   382,   517,   105,
     702,   105,   520,   382,   107,   520,   382,   553,   584,   401,
     401,   623,   367,   645,   725,   189,   610,   376,   366,   366,
     366,   366,   366,   465,   495,   495,   495,   366,   366,   366,
     366,   106,   107,   491,   492,   652,   376,    43,   472,   493,
     479,    39,    89,   106,   279,   294,   312,   317,   319,   336,
     470,   471,   473,   482,   483,   657,    89,   482,   484,    23,
     105,   106,   363,   507,   508,   509,   652,   376,   376,   382,
     382,   382,     8,   396,   366,   427,   423,   376,   376,   376,
     376,   376,   376,   376,   136,   376,   367,   367,   377,   609,
     503,   468,   476,   485,   489,   366,   366,   366,   496,   501,
     497,   502,     8,   367,   106,   430,   473,    92,   286,   481,
     383,   406,   414,   421,   376,     9,   376,   376,   494,   105,
      23,    26,   757,   106,   704,   705,   702,   610,   367,   367,
     367,   367,   367,   499,   500,   498,   367,   367,   367,   367,
     377,   492,    45,    46,   510,   382,   657,   425,   376,   105,
     105,     6,     8,   367,   376,   376,   376,   376,   376,   376,
     367,   367,   367,   376,   376,   376,   376,   516,   652,   365,
     506,   425,   105,   517,   518,   705,   376,   425,   430,   376,
     376,   376,   383,   406,   414,   421,   490,   413,   382,   382,
     382
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   374,   375,   375,   376,   376,   377,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   432,   432,   432,   432,   433,   433,   433,
     433,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   435,   435,   435,
     435,   435,   435,   435,   435,   435,   435,   435,   435,   435,
     435,   435,   435,   435,   435,   435,   435,   435,   435,   435,
     435,   436,   437,   437,   437,   437,   437,   437,   437,   437,
     437,   437,   437,   437,   437,   437,   437,   437,   437,   438,
     439,   439,   440,   440,   441,   441,   442,   442,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   445,
     446,   446,   447,   447,   448,   448,   448,   448,   448,   448,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   448,   449,   450,   450,   451,   451,   451,   451,   451,
     451,   451,   451,   451,   451,   451,   451,   452,   452,   452,
     452,   452,   452,   452,   453,   454,   454,   455,   455,   456,
     457,   457,   458,   458,   458,   458,   458,   458,   458,   458,
     459,   459,   460,   460,   460,   461,   462,   463,   463,   464,
     464,   465,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   466,   466,   466,   466,   466,   466,   467,   468,
     468,   468,   468,   468,   468,   468,   468,   468,   469,   469,
     469,   470,   470,   471,   471,   471,   471,   472,   473,   473,
     474,   475,   475,   476,   476,   476,   476,   476,   476,   476,
     476,   476,   476,   476,   476,   477,   477,   478,   478,   479,
     479,   479,   479,   480,   481,   481,   482,   482,   482,   482,
     482,   483,   483,   484,   484,   485,   485,   485,   485,   485,
     485,   485,   485,   485,   485,   485,   485,   485,   485,   486,
     486,   487,   487,   488,   489,   489,   489,   489,   489,   489,
     489,   490,   490,   491,   491,   491,   492,   492,   492,   493,
     493,   494,   494,   495,   496,   496,   496,   496,   496,   497,
     497,   497,   497,   497,   498,   498,   498,   498,   498,   499,
     499,   499,   499,   499,   500,   500,   500,   500,   500,   501,
     501,   501,   501,   501,   502,   502,   502,   502,   502,   503,
     503,   503,   503,   503,   504,   504,   504,   504,   504,   505,
     506,   507,   507,   508,   508,   508,   508,   508,   509,   509,
     510,   510,   510,   510,   511,   512,   513,   513,   514,   514,
     515,   515,   516,   516,   516,   517,   518,   518,   519,   519,
     520,   520,   520,   520,   520,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   538,   538,   538,   539,   540,   541,
     541,   542,   542,   543,   544,   544,   545,   545,   546,   547,
     548,   549,   550,   550,   551,   551,   551,   551,   551,   552,
     552,   552,   552,   552,   553,   553,   553,   553,   553,   553,
     553,   553,   553,   553,   553,   553,   553,   553,   553,   553,
     553,   553,   553,   553,   553,   553,   553,   554,   555,   555,
     556,   557,   557,   558,   559,   559,   560,   560,   560,   561,
     562,   563,   564,   564,   565,   565,   566,   567,   567,   568,
     568,   569,   570,   570,   571,   571,   572,   572,   573,   574,
     574,   575,   575,   575,   575,   575,   575,   576,   577,   577,
     577,   577,   577,   578,   578,   579,   579,   580,   581,   582,
     582,   583,   583,   583,   584,   584,   585,   585,   586,   587,
     588,   589,   589,   589,   590,   590,   591,   592,   592,   593,
     593,   594,   594,   595,   595,   595,   595,   595,   596,   597,
     598,   598,   599,   599,   599,   599,   599,   599,   599,   599,
     600,   601,   601,   602,   602,   602,   602,   602,   602,   603,
     603,   604,   605,   605,   606,   606,   606,   606,   607,   607,
     608,   609,   609,   610,   610,   611,   611,   611,   611,   611,
     611,   611,   611,   611,   611,   611,   611,   611,   611,   612,
     612,   612,   613,   613,   614,   614,   615,   615,   616,   617,
     617,   618,   618,   619,   619,   620,   621,   622,   622,   623,
     623,   623,   624,   624,   624,   624,   624,   624,   624,   624,
     624,   624,   624,   624,   624,   624,   625,   625,   626,   627,
     627,   627,   628,   628,   629,   630,   630,   630,   630,   630,
     631,   631,   632,   632,   633,   633,   634,   634,   634,   635,
     635,   635,   635,   636,   636,   637,   638,   638,   639,   639,
     640,   640,   641,   641,   641,   642,   642,   642,   642,   643,
     643,   644,   644,   645,   645,   646,   647,   647,   647,   648,
     648,   648,   649,   649,   650,   650,   651,   652,   653,   653,
     654,   654,   655,   656,   657,   657,   657,   657,   657,   657,
     657,   657,   657,   657,   657,   657,   658,   658,   658,   658,
     659,   660,   660,   660,   660,   661,   661,   661,   661,   661,
     662,   662,   663,   663,   664,   664,   665,   665,   665,   666,
     666,   667,   667,   668,   668,   669,   670,   670,   671,   672,
     673,   673,   674,   675,   675,   675,   676,   677,   677,   677,
     678,   678,   678,   679,   679,   680,   681,   681,   681,   682,
     682,   683,   683,   684,   684,   685,   686,   686,   686,   687,
     687,   688,   688,   689,   689,   689,   689,   689,   690,   690,
     690,   691,   692,   692,   693,   693,   693,   693,   694,   695,
     696,   697,   698,   699,   700,   700,   700,   701,   701,   701,
     702,   702,   703,   703,   704,   704,   705,   706,   706,   706,
     707,   707,   707,   707,   707,   708,   709,   709,   710,   711,
     711,   711,   711,   711,   711,   711,   711,   712,   712,   713,
     713,   713,   714,   714,   714,   715,   715,   716,   716,   717,
     717,   718,   719,   719,   719,   719,   720,   720,   721,   722,
     722,   722,   722,   722,   722,   722,   722,   722,   722,   722,
     722,   722,   722,   723,   723,   723,   723,   723,   723,   723,
     723,   723,   723,   723,   723,   723,   723,   723,   723,   723,
     723,   724,   724,   724,   724,   724,   724,   724,   725,   725,
     725,   725,   725,   725,   726,   726,   727,   727,   727,   728,
     728,   729,   729,   729,   729,   729,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     731,   731,   731,   731,   731,   731,   732,   732,   733,   733,
     733,   734,   734,   735,   735,   736,   737,   737,   738,   738,
     739,   739,   740,   740,   741,   741,   742,   742,   742,   743,
     743,   744,   744,   745,   745,   745,   745,   746,   746,   746,
     747,   747,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   749,
     749,   749,   749,   749,   749,   749,   750,   750,   750,   750,
     751,   751,   751,   751,   752,   752,   753,   753,   754,   754,
     755,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   757,   758,   759,   759,   759,   759,   760,
     761,   761,   761,   762,   763,   763,   763,   763,   763,   764,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   766,
     766,   766,   767,   767,   767,   767,   767,   767,   767,   767,
     767,   767,   767,   767,   768,   768,   768,   768,   769,   769,
     769,   769,   769,   770,   771,   771,   771,   771,   771,   771,
     771,   772,   773,   773,   773,   773,   774,   775,   775,   775,
     775,   775,   775,   775,   775,   775,   776,   777,   777,   777,
     777,   777,   777,   777,   777,   777,   777,   778,   779,   779,
     779,   779,   779,   780,   781,   781,   782,   783,   783,   783,
     784,   785,   785,   785,   785,   786,   787,   787,   787,   787,
     788,   788,   788,   788,   789,   789,   789,   789,   789,   789,
     789,   789,   789,   789,   789,   789,   789,   789,   790,   790,
     790,   791,   791,   792,   792,   793,   793,   794,   794,   795,
     795,   796,   796,   797,   797,   797,   798,   799,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   801,   801,
     801,   801,   801,   801,   802,   802,   802,   802,   802,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   804,   804,   805,   806,   806,   807,   808,   809,   809,
     810,   810,   810,   811,   811,   811,   811,   811,   811,   811,
     811,   811,   811,   811,   811,   811,   811,   811,   811,   811,
     811,   812,   812,   812,   813,   813,   813,   813,   814,   814,
     814,   814,   814,   815,   815,   815,   815,   816,   816,   816,
     816,   816,   816,   816,   816,   816,   816,   816,   816,   817,
     817,   818,   818,   818,   818,   819,   820,   820,   821,   821,
     821,   821,   821,   821,   821,   821,   822,   823,   823,   824,
     825,   825,   825,   825,   826,   827,   828,   829,   830,   831,
     831,   831,   831,   832,   833,   833,   833,   833,   833,   833,
     834,   835,   835,   836,   837,   837,   837,   837,   837,   838,
     839,   839,   839,   839,   839
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     5,     5,     3,     2,     1,     1,     2,
       2,     1,     2,     2,     2,     2,     2,     2,     3,     3,
       2,     2,     3,     3,     3,     2,     3,     2,     6,     2,
       6,     3,     2,     6,     6,     3,     6,     3,     5,     7,
       5,     7,     8,     8,     8,     5,     7,     5,     7,     5,
       7,     3,     2,     6,     2,     6,     6,     6,     3,     6,
       3,     5,     5,     8,     8,     8,     5,     5,     5,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       6,     2,     2,     2,     3,     2,     2,     6,     3,     3,
       5,     3,     3,     3,     2,     2,     2,     2,     2,     3,
       2,     2,     6,     3,     3,     5,     3,     3,     3,     3,
       2,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     3,     2,     2,     2,     2,     2,     2,     4,
       5,     2,     2,     1,     2,     2,     3,     2,     3,     2,
       2,     2,     3,     2,     3,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     3,     2,     1,     2,     1,     3,
       0,     1,     0,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     2,     1,     0,     2,     1,     1,
       3,     1,     0,     2,     2,     3,     8,     8,     8,     8,
       9,     9,    10,    10,    10,     9,     9,     9,     0,     0,
       2,     2,     3,     3,     3,     3,     5,     3,     0,     2,
       3,     1,     3,     2,     1,     1,     1,     1,     1,     3,
       0,     2,     3,     0,     2,     2,     3,     4,     4,     4,
       3,     4,     2,     3,     3,     1,     1,     3,     1,     1,
       1,     1,     1,     0,     1,     1,     2,     2,     2,     2,
       2,     1,     3,     1,     0,     0,     2,     2,     4,     4,
       8,     6,     7,     6,     4,     3,     4,     3,     3,     3,
       2,     1,     1,     0,     0,     2,     2,     5,     5,     3,
       4,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     3,     0,     0,     2,     2,     2,     2,     0,
       2,     2,     2,     2,     0,     2,     2,     2,     2,     0,
       2,     2,     2,     2,     0,     2,     2,     2,     2,     0,
       2,     2,     2,     2,     0,     2,     2,     2,     2,     0,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     7,
       2,     1,     1,     1,     1,     1,     3,     3,     1,     2,
       2,     2,     3,     0,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       2,     2,     1,     2,     1,     1,     2,     3,     2,     3,
       1,     2,     3,     1,     2,     3,     1,     2,     3,     1,
       2,     2,     2,     1,     2,     2,     2,     2,     2,     0,
       1,     1,     2,     1,     1,     2,     1,     2,     4,     4,
       4,     4,     5,     5,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     1,     1,     2,     2,     2,     2,
       1,     1,     2,     1,     1,     2,     1,     3,     1,     1,
       5,     1,     1,     3,     3,     1,     1,     3,     3,     5,
       2,     2,     1,     2,     1,     2,     1,     1,     2,     2,
       2,     1,     1,     2,     2,     2,     1,     2,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     1,     3,     3,
       1,     2,     1,     3,     1,     1,     1,     2,     2,     3,
       3,     1,     1,     0,     1,     1,     0,     3,     1,     2,
       4,     1,     1,     0,     0,     3,     3,     0,     2,     2,
       3,     2,     2,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     0,     6,     3,     6,     3,     5,     3,     5,
       2,     1,     1,     3,     4,     4,     5,     6,     5,     1,
       2,     1,     1,     2,     2,     2,     1,     1,     6,     8,
       0,     0,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     3,     1,     3,     1,     3,     1,     3,     1,     1,
       3,     1,     1,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     3,     3,     4,     5,     2,     3,
       2,     6,     4,     3,     4,     3,     2,     1,     1,     3,
       4,     1,     2,     1,     1,     2,     3,     1,     3,     4,
       3,     5,     3,     6,     1,     3,     1,     1,     1,     2,
       4,     6,     6,     1,     2,     1,     1,     2,     2,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     2,     1,     4,     5,     6,     1,
       1,     1,     7,     8,     6,     8,     1,     2,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       4,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       3,     1,     4,     4,     0,     2,     1,     3,     3,     1,
       3,     1,     3,     1,     3,     1,     1,     3,     3,     3,
       1,     1,     3,     1,     1,     1,     3,     1,     3,     3,
       3,     3,     5,     1,     2,     1,     1,     2,     3,     1,
       1,     2,     1,     1,     2,     1,     2,     2,     1,     1,
       2,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     2,     2,     4,     0,
       4,     0,     1,     0,     1,     1,     1,     1,     1,     1,
       2,     2,     6,     3,     1,     3,     3,     3,     7,     3,
       3,     3,     3,     3,     3,     0,     4,     4,     0,     2,
       2,     4,     4,     5,     5,     3,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     1,     1,     3,     3,     1,     1,     1,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     2,     1,
       1,     1,     2,     1,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     1,     1,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       5,     3,     5,     1,     5,     5,     3,     5,     1,     1,
       1,     0,     2,     1,     1,     6,     2,     0,     1,     1,
       1,     1,     1,     1,     5,     6,     8,     6,     5,     2,
       2,     3,     4,     1,     1,     1,     2,     3,     4,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     5,     6,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     7,     1,     1,     2,     1,     3,
       1,     1,     2,     3,     1,     1,     1,     1,     2,     3,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       5,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     3,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     1,     3,     1,     1,     1,     1,
       2,     3,     3,     9,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     1,     1,     2,     2,     1,
       1,     3,     3,     1,     1,     1,     3,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     4,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       5,     1,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     3,
       1,     1,     2,     1,     3,     4,     3,     1,     3,     1,
       1,     1,     4,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     3,     1,     1,     2,     1,     1,     2,
       2,     2,     2,     2,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, nft, scanner, state, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, nft, scanner, state); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct nft_ctx *nft, void *scanner, struct parser_state *state)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (nft);
  YY_USE (scanner);
  YY_USE (state);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct nft_ctx *nft, void *scanner, struct parser_state *state)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, nft, scanner, state);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, struct nft_ctx *nft, void *scanner, struct parser_state *state)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), nft, scanner, state);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, nft, scanner, state); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct nft_ctx *nft, void *scanner, struct parser_state *state)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (nft);
  YY_USE (scanner);
  YY_USE (state);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_STRING: /* "string"  */
#line 380 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6349 "src/parser_bison.c"
        break;

    case YYSYMBOL_QUOTED_STRING: /* "quoted string"  */
#line 380 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6355 "src/parser_bison.c"
        break;

    case YYSYMBOL_ASTERISK_STRING: /* "string with a trailing asterisk"  */
#line 380 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6361 "src/parser_bison.c"
        break;

    case YYSYMBOL_line: /* line  */
#line 704 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6367 "src/parser_bison.c"
        break;

    case YYSYMBOL_base_cmd: /* base_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6373 "src/parser_bison.c"
        break;

    case YYSYMBOL_add_cmd: /* add_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6379 "src/parser_bison.c"
        break;

    case YYSYMBOL_replace_cmd: /* replace_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6385 "src/parser_bison.c"
        break;

    case YYSYMBOL_create_cmd: /* create_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6391 "src/parser_bison.c"
        break;

    case YYSYMBOL_insert_cmd: /* insert_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6397 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_or_id_spec: /* table_or_id_spec  */
#line 710 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6403 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_or_id_spec: /* chain_or_id_spec  */
#line 712 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6409 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_or_id_spec: /* set_or_id_spec  */
#line 717 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6415 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_or_id_spec: /* obj_or_id_spec  */
#line 719 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6421 "src/parser_bison.c"
        break;

    case YYSYMBOL_delete_cmd: /* delete_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6427 "src/parser_bison.c"
        break;

    case YYSYMBOL_destroy_cmd: /* destroy_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6433 "src/parser_bison.c"
        break;

    case YYSYMBOL_get_cmd: /* get_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6439 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_table: /* list_cmd_spec_table  */
#line 728 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6445 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_any: /* list_cmd_spec_any  */
#line 728 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6451 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd: /* list_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6457 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_device_name: /* basehook_device_name  */
#line 736 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6463 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_spec: /* basehook_spec  */
#line 725 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6469 "src/parser_bison.c"
        break;

    case YYSYMBOL_reset_cmd: /* reset_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6475 "src/parser_bison.c"
        break;

    case YYSYMBOL_flush_cmd: /* flush_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6481 "src/parser_bison.c"
        break;

    case YYSYMBOL_rename_cmd: /* rename_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6487 "src/parser_bison.c"
        break;

    case YYSYMBOL_import_cmd: /* import_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6493 "src/parser_bison.c"
        break;

    case YYSYMBOL_export_cmd: /* export_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6499 "src/parser_bison.c"
        break;

    case YYSYMBOL_monitor_cmd: /* monitor_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6505 "src/parser_bison.c"
        break;

    case YYSYMBOL_monitor_event: /* monitor_event  */
#line 964 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6511 "src/parser_bison.c"
        break;

    case YYSYMBOL_describe_cmd: /* describe_cmd  */
#line 707 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6517 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_block_alloc: /* table_block_alloc  */
#line 742 "src/parser_bison.y"
            { close_scope(state); table_free(((*yyvaluep).table)); }
#line 6523 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_block_alloc: /* chain_block_alloc  */
#line 744 "src/parser_bison.y"
            { close_scope(state); chain_free(((*yyvaluep).chain)); }
#line 6529 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_verdict_expr: /* typeof_verdict_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6535 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_data_expr: /* typeof_data_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6541 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_typeof_expr: /* primary_typeof_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6547 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_expr: /* typeof_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6553 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_alloc: /* set_block_alloc  */
#line 755 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6559 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_key_expr: /* typeof_key_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6565 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_expr: /* set_block_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6571 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_block_alloc: /* map_block_alloc  */
#line 758 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6577 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_block_alloc: /* flowtable_block_alloc  */
#line 762 "src/parser_bison.y"
            { flowtable_free(((*yyvaluep).flowtable)); }
#line 6583 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr: /* flowtable_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6589 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_list_expr: /* flowtable_list_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6595 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr_member: /* flowtable_expr_member  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6601 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_atom_expr: /* data_type_atom_expr  */
#line 701 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6607 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_expr: /* data_type_expr  */
#line 701 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6613 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_block_alloc: /* obj_block_alloc  */
#line 765 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 6619 "src/parser_bison.c"
        break;

    case YYSYMBOL_type_identifier: /* type_identifier  */
#line 696 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6625 "src/parser_bison.c"
        break;

    case YYSYMBOL_prio_spec: /* prio_spec  */
#line 733 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6631 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_name: /* extended_prio_name  */
#line 736 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6637 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_spec: /* extended_prio_spec  */
#line 733 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6643 "src/parser_bison.c"
        break;

    case YYSYMBOL_dev_spec: /* dev_spec  */
#line 739 "src/parser_bison.y"
            { free(((*yyvaluep).expr)); }
#line 6649 "src/parser_bison.c"
        break;

    case YYSYMBOL_policy_expr: /* policy_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6655 "src/parser_bison.c"
        break;

    case YYSYMBOL_identifier: /* identifier  */
#line 696 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6661 "src/parser_bison.c"
        break;

    case YYSYMBOL_string: /* string  */
#line 696 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6667 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_spec: /* table_spec  */
#line 710 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6673 "src/parser_bison.c"
        break;

    case YYSYMBOL_tableid_spec: /* tableid_spec  */
#line 710 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6679 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_spec: /* chain_spec  */
#line 712 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6685 "src/parser_bison.c"
        break;

    case YYSYMBOL_chainid_spec: /* chainid_spec  */
#line 712 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6691 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_identifier: /* chain_identifier  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6697 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_spec: /* set_spec  */
#line 717 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6703 "src/parser_bison.c"
        break;

    case YYSYMBOL_setid_spec: /* setid_spec  */
#line 717 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6709 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_identifier: /* set_identifier  */
#line 722 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6715 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_spec: /* flowtable_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6721 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtableid_spec: /* flowtableid_spec  */
#line 722 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6727 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_identifier: /* flowtable_identifier  */
#line 722 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6733 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_spec: /* obj_spec  */
#line 719 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6739 "src/parser_bison.c"
        break;

    case YYSYMBOL_objid_spec: /* objid_spec  */
#line 719 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6745 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_identifier: /* obj_identifier  */
#line 722 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6751 "src/parser_bison.c"
        break;

    case YYSYMBOL_handle_spec: /* handle_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6757 "src/parser_bison.c"
        break;

    case YYSYMBOL_position_spec: /* position_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6763 "src/parser_bison.c"
        break;

    case YYSYMBOL_index_spec: /* index_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6769 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule_position: /* rule_position  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6775 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleid_spec: /* ruleid_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6781 "src/parser_bison.c"
        break;

    case YYSYMBOL_comment_spec: /* comment_spec  */
#line 696 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6787 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleset_spec: /* ruleset_spec  */
#line 715 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6793 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule: /* rule  */
#line 746 "src/parser_bison.y"
            { rule_free(((*yyvaluep).rule)); }
#line 6799 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_list: /* stmt_list  */
#line 768 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 6805 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt_list: /* stateful_stmt_list  */
#line 768 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 6811 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_counter: /* objref_stmt_counter  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6817 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_limit: /* objref_stmt_limit  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6823 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_quota: /* objref_stmt_quota  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6829 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_synproxy: /* objref_stmt_synproxy  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6835 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_ct: /* objref_stmt_ct  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6841 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt: /* objref_stmt  */
#line 776 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6847 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt: /* stateful_stmt  */
#line 772 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6853 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 770 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6859 "src/parser_bison.c"
        break;

    case YYSYMBOL_xt_stmt: /* xt_stmt  */
#line 988 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6865 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_stmt: /* chain_stmt  */
#line 800 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6871 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_stmt: /* verdict_stmt  */
#line 770 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6877 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_stmt: /* verdict_map_stmt  */
#line 858 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6883 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_expr: /* verdict_map_expr  */
#line 861 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6889 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_expr: /* verdict_map_list_expr  */
#line 861 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6895 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_member_expr: /* verdict_map_list_member_expr  */
#line 861 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6901 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_limit_stmt_alloc: /* ct_limit_stmt_alloc  */
#line 774 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6907 "src/parser_bison.c"
        break;

    case YYSYMBOL_connlimit_stmt: /* connlimit_stmt  */
#line 788 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6913 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt: /* counter_stmt  */
#line 772 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6919 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt_alloc: /* counter_stmt_alloc  */
#line 772 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6925 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt_alloc: /* last_stmt_alloc  */
#line 774 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6931 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt: /* last_stmt  */
#line 772 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6937 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt: /* log_stmt  */
#line 785 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6943 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt_alloc: /* log_stmt_alloc  */
#line 785 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6949 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt_alloc: /* limit_stmt_alloc  */
#line 774 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6955 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt: /* limit_stmt  */
#line 788 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6961 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_unit: /* quota_unit  */
#line 736 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6967 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt_alloc: /* quota_stmt_alloc  */
#line 774 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6973 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt: /* quota_stmt  */
#line 788 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6979 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt: /* reject_stmt  */
#line 791 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6985 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt_alloc: /* reject_stmt_alloc  */
#line 791 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 6991 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_with_expr: /* reject_with_expr  */
#line 806 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6997 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt: /* nat_stmt  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7003 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt_alloc: /* nat_stmt_alloc  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7009 "src/parser_bison.c"
        break;

    case YYSYMBOL_tproxy_stmt: /* tproxy_stmt  */
#line 796 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7015 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt: /* synproxy_stmt  */
#line 798 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7021 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt_alloc: /* synproxy_stmt_alloc  */
#line 798 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7027 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_obj: /* synproxy_obj  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7033 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_stmt_expr: /* primary_stmt_expr  */
#line 845 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7039 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_stmt_expr: /* shift_stmt_expr  */
#line 847 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7045 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_stmt_expr: /* and_stmt_expr  */
#line 849 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7051 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_stmt_expr: /* exclusive_or_stmt_expr  */
#line 849 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7057 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_stmt_expr: /* inclusive_or_stmt_expr  */
#line 849 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7063 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_stmt_expr: /* basic_stmt_expr  */
#line 845 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7069 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_stmt_expr: /* concat_stmt_expr  */
#line 837 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7075 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr_set: /* map_stmt_expr_set  */
#line 837 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7081 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr: /* map_stmt_expr  */
#line 837 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7087 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_stmt_expr: /* prefix_stmt_expr  */
#line 842 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7093 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_stmt_expr: /* range_stmt_expr  */
#line 842 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7099 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_stmt_expr: /* multiton_stmt_expr  */
#line 840 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7105 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_expr: /* stmt_expr  */
#line 837 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7111 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt: /* masq_stmt  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7117 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt_alloc: /* masq_stmt_alloc  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7123 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt: /* redir_stmt  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7129 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt_alloc: /* redir_stmt_alloc  */
#line 793 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7135 "src/parser_bison.c"
        break;

    case YYSYMBOL_dup_stmt: /* dup_stmt  */
#line 809 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7141 "src/parser_bison.c"
        break;

    case YYSYMBOL_fwd_stmt: /* fwd_stmt  */
#line 811 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7147 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt: /* queue_stmt  */
#line 804 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7153 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_compat: /* queue_stmt_compat  */
#line 804 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7159 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_alloc: /* queue_stmt_alloc  */
#line 804 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7165 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_expr: /* queue_expr  */
#line 806 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7171 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr_simple: /* queue_stmt_expr_simple  */
#line 806 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7177 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr: /* queue_stmt_expr  */
#line 806 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7183 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt: /* set_elem_expr_stmt  */
#line 868 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7189 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt_alloc: /* set_elem_expr_stmt_alloc  */
#line 868 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7195 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_stmt: /* set_stmt  */
#line 813 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7201 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt: /* map_stmt  */
#line 816 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7207 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_stmt: /* meter_stmt  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7213 "src/parser_bison.c"
        break;

    case YYSYMBOL_match_stmt: /* match_stmt  */
#line 770 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7219 "src/parser_bison.c"
        break;

    case YYSYMBOL_variable_expr: /* variable_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7225 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_expr: /* symbol_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7231 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_expr: /* set_ref_expr  */
#line 829 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7237 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_symbol_expr: /* set_ref_symbol_expr  */
#line 829 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7243 "src/parser_bison.c"
        break;

    case YYSYMBOL_integer_expr: /* integer_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7249 "src/parser_bison.c"
        break;

    case YYSYMBOL_selector_expr: /* selector_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7255 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_expr: /* primary_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7261 "src/parser_bison.c"
        break;

    case YYSYMBOL_fib_expr: /* fib_expr  */
#line 955 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7267 "src/parser_bison.c"
        break;

    case YYSYMBOL_osf_expr: /* osf_expr  */
#line 960 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7273 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_expr: /* shift_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7279 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_expr: /* and_expr  */
#line 823 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7285 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_expr: /* exclusive_or_expr  */
#line 825 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7291 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_expr: /* inclusive_or_expr  */
#line 825 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7297 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_expr: /* basic_expr  */
#line 827 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7303 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_expr: /* concat_expr  */
#line 852 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7309 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_rhs_expr: /* prefix_rhs_expr  */
#line 834 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7315 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_rhs_expr: /* range_rhs_expr  */
#line 834 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7321 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_rhs_expr: /* multiton_rhs_expr  */
#line 832 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7327 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_expr: /* map_expr  */
#line 855 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7333 "src/parser_bison.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7339 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_expr: /* set_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7345 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_expr: /* set_list_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7351 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_member_expr: /* set_list_member_expr  */
#line 864 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7357 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr: /* meter_key_expr  */
#line 871 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7363 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr_alloc: /* meter_key_expr_alloc  */
#line 871 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7369 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr: /* set_elem_expr  */
#line 866 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7375 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_key_expr: /* set_elem_key_expr  */
#line 1011 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7381 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_alloc: /* set_elem_expr_alloc  */
#line 866 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7387 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt_list: /* set_elem_stmt_list  */
#line 768 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7393 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt: /* set_elem_stmt  */
#line 770 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7399 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_lhs_expr: /* set_lhs_expr  */
#line 866 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7405 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_rhs_expr: /* set_rhs_expr  */
#line 866 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7411 "src/parser_bison.c"
        break;

    case YYSYMBOL_initializer_expr: /* initializer_expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7417 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_obj: /* counter_obj  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7423 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_obj: /* quota_obj  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7429 "src/parser_bison.c"
        break;

    case YYSYMBOL_secmark_obj: /* secmark_obj  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7435 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_states: /* timeout_states  */
#line 1004 "src/parser_bison.y"
            { timeout_states_free(((*yyvaluep).list)); }
#line 7441 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_state: /* timeout_state  */
#line 1001 "src/parser_bison.y"
            { timeout_state_free(((*yyvaluep).timeout_state)); }
#line 7447 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_obj_alloc: /* ct_obj_alloc  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7453 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_obj: /* limit_obj  */
#line 884 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7459 "src/parser_bison.c"
        break;

    case YYSYMBOL_relational_expr: /* relational_expr  */
#line 887 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7465 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_rhs_expr: /* list_rhs_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7471 "src/parser_bison.c"
        break;

    case YYSYMBOL_rhs_expr: /* rhs_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7477 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_rhs_expr: /* shift_rhs_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7483 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_rhs_expr: /* and_rhs_expr  */
#line 881 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7489 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_rhs_expr: /* exclusive_or_rhs_expr  */
#line 881 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7495 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_rhs_expr: /* inclusive_or_rhs_expr  */
#line 881 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7501 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_rhs_expr: /* basic_rhs_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7507 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_rhs_expr: /* concat_rhs_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7513 "src/parser_bison.c"
        break;

    case YYSYMBOL_boolean_expr: /* boolean_expr  */
#line 991 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7519 "src/parser_bison.c"
        break;

    case YYSYMBOL_keyword_expr: /* keyword_expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7525 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_rhs_expr: /* primary_rhs_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7531 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_expr: /* verdict_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7537 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_expr: /* chain_expr  */
#line 821 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7543 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_expr: /* meta_expr  */
#line 937 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7549 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_stmt: /* meta_stmt  */
#line 783 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7555 "src/parser_bison.c"
        break;

    case YYSYMBOL_socket_expr: /* socket_expr  */
#line 941 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7561 "src/parser_bison.c"
        break;

    case YYSYMBOL_numgen_expr: /* numgen_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7567 "src/parser_bison.c"
        break;

    case YYSYMBOL_xfrm_expr: /* xfrm_expr  */
#line 1008 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7573 "src/parser_bison.c"
        break;

    case YYSYMBOL_hash_expr: /* hash_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7579 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_expr: /* rt_expr  */
#line 947 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7585 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_expr: /* ct_expr  */
#line 951 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7591 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_stmt_expr: /* symbol_stmt_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7597 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_stmt_expr: /* list_stmt_expr  */
#line 847 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7603 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_stmt: /* ct_stmt  */
#line 781 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7609 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_stmt: /* payload_stmt  */
#line 779 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7615 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_expr: /* payload_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7621 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_raw_expr: /* payload_raw_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7627 "src/parser_bison.c"
        break;

    case YYSYMBOL_eth_hdr_expr: /* eth_hdr_expr  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7633 "src/parser_bison.c"
        break;

    case YYSYMBOL_vlan_hdr_expr: /* vlan_hdr_expr  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7639 "src/parser_bison.c"
        break;

    case YYSYMBOL_arp_hdr_expr: /* arp_hdr_expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7645 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip_hdr_expr: /* ip_hdr_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7651 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp_hdr_expr: /* icmp_hdr_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7657 "src/parser_bison.c"
        break;

    case YYSYMBOL_igmp_hdr_expr: /* igmp_hdr_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7663 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip6_hdr_expr: /* ip6_hdr_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7669 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp6_hdr_expr: /* icmp6_hdr_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7675 "src/parser_bison.c"
        break;

    case YYSYMBOL_auth_hdr_expr: /* auth_hdr_expr  */
#line 909 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7681 "src/parser_bison.c"
        break;

    case YYSYMBOL_esp_hdr_expr: /* esp_hdr_expr  */
#line 909 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7687 "src/parser_bison.c"
        break;

    case YYSYMBOL_comp_hdr_expr: /* comp_hdr_expr  */
#line 909 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7693 "src/parser_bison.c"
        break;

    case YYSYMBOL_udp_hdr_expr: /* udp_hdr_expr  */
#line 912 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7699 "src/parser_bison.c"
        break;

    case YYSYMBOL_udplite_hdr_expr: /* udplite_hdr_expr  */
#line 912 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7705 "src/parser_bison.c"
        break;

    case YYSYMBOL_tcp_hdr_expr: /* tcp_hdr_expr  */
#line 970 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7711 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_inet_expr: /* inner_inet_expr  */
#line 978 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7717 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_eth_expr: /* inner_eth_expr  */
#line 978 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7723 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_expr: /* inner_expr  */
#line 978 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7729 "src/parser_bison.c"
        break;

    case YYSYMBOL_vxlan_hdr_expr: /* vxlan_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7735 "src/parser_bison.c"
        break;

    case YYSYMBOL_geneve_hdr_expr: /* geneve_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7741 "src/parser_bison.c"
        break;

    case YYSYMBOL_gre_hdr_expr: /* gre_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7747 "src/parser_bison.c"
        break;

    case YYSYMBOL_gretap_hdr_expr: /* gretap_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7753 "src/parser_bison.c"
        break;

    case YYSYMBOL_optstrip_stmt: /* optstrip_stmt  */
#line 985 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7759 "src/parser_bison.c"
        break;

    case YYSYMBOL_dccp_hdr_expr: /* dccp_hdr_expr  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7765 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_chunk_alloc: /* sctp_chunk_alloc  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7771 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_hdr_expr: /* sctp_hdr_expr  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7777 "src/parser_bison.c"
        break;

    case YYSYMBOL_th_hdr_expr: /* th_hdr_expr  */
#line 921 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7783 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_expr: /* exthdr_expr  */
#line 925 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7789 "src/parser_bison.c"
        break;

    case YYSYMBOL_hbh_hdr_expr: /* hbh_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7795 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_hdr_expr: /* rt_hdr_expr  */
#line 930 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7801 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt0_hdr_expr: /* rt0_hdr_expr  */
#line 930 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7807 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt2_hdr_expr: /* rt2_hdr_expr  */
#line 930 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7813 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt4_hdr_expr: /* rt4_hdr_expr  */
#line 930 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7819 "src/parser_bison.c"
        break;

    case YYSYMBOL_frag_hdr_expr: /* frag_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7825 "src/parser_bison.c"
        break;

    case YYSYMBOL_dst_hdr_expr: /* dst_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7831 "src/parser_bison.c"
        break;

    case YYSYMBOL_mh_hdr_expr: /* mh_hdr_expr  */
#line 933 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7837 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_exists_expr: /* exthdr_exists_expr  */
#line 995 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7843 "src/parser_bison.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct nft_ctx *nft, void *scanner, struct parser_state *state)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */


/* User initialization code.  */
#line 214 "src/parser_bison.y"
{
	location_init(scanner, state, &yylloc);
	if (nft->debug_mask & NFT_DEBUG_SCANNER)
		nft_set_debug(1, scanner);
	if (nft->debug_mask & NFT_DEBUG_PARSER)
		yydebug = 1;
}

#line 7949 "src/parser_bison.c"

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= TOKEN_EOF)
    {
      yychar = TOKEN_EOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* input: input line  */
#line 1017 "src/parser_bison.y"
                        {
				if ((yyvsp[0].cmd) != NULL) {
					(yyvsp[0].cmd)->location = (yylsp[0]);
					list_add_tail(&(yyvsp[0].cmd)->list, state->cmds);
				}
			}
#line 8167 "src/parser_bison.c"
    break;

  case 8: /* close_scope_ah: %empty  */
#line 1033 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_AH); }
#line 8173 "src/parser_bison.c"
    break;

  case 9: /* close_scope_arp: %empty  */
#line 1034 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ARP); }
#line 8179 "src/parser_bison.c"
    break;

  case 10: /* close_scope_at: %empty  */
#line 1035 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_AT); }
#line 8185 "src/parser_bison.c"
    break;

  case 11: /* close_scope_comp: %empty  */
#line 1036 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_COMP); }
#line 8191 "src/parser_bison.c"
    break;

  case 12: /* close_scope_ct: %empty  */
#line 1037 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CT); }
#line 8197 "src/parser_bison.c"
    break;

  case 13: /* close_scope_counter: %empty  */
#line 1038 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_COUNTER); }
#line 8203 "src/parser_bison.c"
    break;

  case 14: /* close_scope_last: %empty  */
#line 1039 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LAST); }
#line 8209 "src/parser_bison.c"
    break;

  case 15: /* close_scope_dccp: %empty  */
#line 1040 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DCCP); }
#line 8215 "src/parser_bison.c"
    break;

  case 16: /* close_scope_destroy: %empty  */
#line 1041 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_DESTROY); }
#line 8221 "src/parser_bison.c"
    break;

  case 17: /* close_scope_dst: %empty  */
#line 1042 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DST); }
#line 8227 "src/parser_bison.c"
    break;

  case 18: /* close_scope_dup: %empty  */
#line 1043 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_DUP); }
#line 8233 "src/parser_bison.c"
    break;

  case 19: /* close_scope_esp: %empty  */
#line 1044 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_ESP); }
#line 8239 "src/parser_bison.c"
    break;

  case 20: /* close_scope_eth: %empty  */
#line 1045 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ETH); }
#line 8245 "src/parser_bison.c"
    break;

  case 21: /* close_scope_export: %empty  */
#line 1046 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_EXPORT); }
#line 8251 "src/parser_bison.c"
    break;

  case 22: /* close_scope_fib: %empty  */
#line 1047 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FIB); }
#line 8257 "src/parser_bison.c"
    break;

  case 23: /* close_scope_frag: %empty  */
#line 1048 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FRAG); }
#line 8263 "src/parser_bison.c"
    break;

  case 24: /* close_scope_fwd: %empty  */
#line 1049 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_FWD); }
#line 8269 "src/parser_bison.c"
    break;

  case 25: /* close_scope_gre: %empty  */
#line 1050 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_GRE); }
#line 8275 "src/parser_bison.c"
    break;

  case 26: /* close_scope_hash: %empty  */
#line 1051 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HASH); }
#line 8281 "src/parser_bison.c"
    break;

  case 27: /* close_scope_hbh: %empty  */
#line 1052 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HBH); }
#line 8287 "src/parser_bison.c"
    break;

  case 28: /* close_scope_ip: %empty  */
#line 1053 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP); }
#line 8293 "src/parser_bison.c"
    break;

  case 29: /* close_scope_ip6: %empty  */
#line 1054 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP6); }
#line 8299 "src/parser_bison.c"
    break;

  case 30: /* close_scope_vlan: %empty  */
#line 1055 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_VLAN); }
#line 8305 "src/parser_bison.c"
    break;

  case 31: /* close_scope_icmp: %empty  */
#line 1056 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ICMP); }
#line 8311 "src/parser_bison.c"
    break;

  case 32: /* close_scope_igmp: %empty  */
#line 1057 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IGMP); }
#line 8317 "src/parser_bison.c"
    break;

  case 33: /* close_scope_import: %empty  */
#line 1058 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_IMPORT); }
#line 8323 "src/parser_bison.c"
    break;

  case 34: /* close_scope_ipsec: %empty  */
#line 1059 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_IPSEC); }
#line 8329 "src/parser_bison.c"
    break;

  case 35: /* close_scope_list: %empty  */
#line 1060 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_LIST); }
#line 8335 "src/parser_bison.c"
    break;

  case 36: /* close_scope_limit: %empty  */
#line 1061 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LIMIT); }
#line 8341 "src/parser_bison.c"
    break;

  case 37: /* close_scope_meta: %empty  */
#line 1062 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_META); }
#line 8347 "src/parser_bison.c"
    break;

  case 38: /* close_scope_mh: %empty  */
#line 1063 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_MH); }
#line 8353 "src/parser_bison.c"
    break;

  case 39: /* close_scope_monitor: %empty  */
#line 1064 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_MONITOR); }
#line 8359 "src/parser_bison.c"
    break;

  case 40: /* close_scope_nat: %empty  */
#line 1065 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_NAT); }
#line 8365 "src/parser_bison.c"
    break;

  case 41: /* close_scope_numgen: %empty  */
#line 1066 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_NUMGEN); }
#line 8371 "src/parser_bison.c"
    break;

  case 42: /* close_scope_osf: %empty  */
#line 1067 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_OSF); }
#line 8377 "src/parser_bison.c"
    break;

  case 43: /* close_scope_policy: %empty  */
#line 1068 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_POLICY); }
#line 8383 "src/parser_bison.c"
    break;

  case 44: /* close_scope_quota: %empty  */
#line 1069 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_QUOTA); }
#line 8389 "src/parser_bison.c"
    break;

  case 45: /* close_scope_queue: %empty  */
#line 1070 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_QUEUE); }
#line 8395 "src/parser_bison.c"
    break;

  case 46: /* close_scope_reject: %empty  */
#line 1071 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_REJECT); }
#line 8401 "src/parser_bison.c"
    break;

  case 47: /* close_scope_reset: %empty  */
#line 1072 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_RESET); }
#line 8407 "src/parser_bison.c"
    break;

  case 48: /* close_scope_rt: %empty  */
#line 1073 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_RT); }
#line 8413 "src/parser_bison.c"
    break;

  case 49: /* close_scope_sctp: %empty  */
#line 1074 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SCTP); }
#line 8419 "src/parser_bison.c"
    break;

  case 50: /* close_scope_sctp_chunk: %empty  */
#line 1075 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SCTP_CHUNK); }
#line 8425 "src/parser_bison.c"
    break;

  case 51: /* close_scope_secmark: %empty  */
#line 1076 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SECMARK); }
#line 8431 "src/parser_bison.c"
    break;

  case 52: /* close_scope_socket: %empty  */
#line 1077 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SOCKET); }
#line 8437 "src/parser_bison.c"
    break;

  case 53: /* close_scope_tcp: %empty  */
#line 1078 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TCP); }
#line 8443 "src/parser_bison.c"
    break;

  case 54: /* close_scope_tproxy: %empty  */
#line 1079 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_TPROXY); }
#line 8449 "src/parser_bison.c"
    break;

  case 55: /* close_scope_type: %empty  */
#line 1080 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TYPE); }
#line 8455 "src/parser_bison.c"
    break;

  case 56: /* close_scope_th: %empty  */
#line 1081 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_TH); }
#line 8461 "src/parser_bison.c"
    break;

  case 57: /* close_scope_udp: %empty  */
#line 1082 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDP); }
#line 8467 "src/parser_bison.c"
    break;

  case 58: /* close_scope_udplite: %empty  */
#line 1083 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDPLITE); }
#line 8473 "src/parser_bison.c"
    break;

  case 59: /* close_scope_log: %empty  */
#line 1085 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_LOG); }
#line 8479 "src/parser_bison.c"
    break;

  case 60: /* close_scope_synproxy: %empty  */
#line 1086 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_SYNPROXY); }
#line 8485 "src/parser_bison.c"
    break;

  case 61: /* close_scope_xt: %empty  */
#line 1087 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_XT); }
#line 8491 "src/parser_bison.c"
    break;

  case 62: /* common_block: "include" "quoted string" stmt_separator  */
#line 1090 "src/parser_bison.y"
                        {
				if (scanner_include_file(nft, scanner, (yyvsp[-1].string), &(yyloc)) < 0) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				free_const((yyvsp[-1].string));
			}
#line 8503 "src/parser_bison.c"
    break;

  case 63: /* common_block: "define" identifier '=' initializer_expr stmt_separator  */
#line 1098 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);

				if (symbol_lookup(scope, (yyvsp[-3].string)) != NULL) {
					erec_queue(error(&(yylsp[-3]), "redefinition of symbol '%s'", (yyvsp[-3].string)),
						   state->msgs);
					expr_free((yyvsp[-1].expr));
					free_const((yyvsp[-3].string));
					YYERROR;
				}

				symbol_bind(scope, (yyvsp[-3].string), (yyvsp[-1].expr));
				free_const((yyvsp[-3].string));
			}
#line 8522 "src/parser_bison.c"
    break;

  case 64: /* common_block: "redefine" identifier '=' initializer_expr stmt_separator  */
#line 1113 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);

				symbol_bind(scope, (yyvsp[-3].string), (yyvsp[-1].expr));
				free_const((yyvsp[-3].string));
			}
#line 8533 "src/parser_bison.c"
    break;

  case 65: /* common_block: "undefine" identifier stmt_separator  */
#line 1120 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);

				if (symbol_unbind(scope, (yyvsp[-1].string)) < 0) {
					erec_queue(error(&(yylsp[-1]), "undefined symbol '%s'", (yyvsp[-1].string)),
						   state->msgs);
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				free_const((yyvsp[-1].string));
			}
#line 8549 "src/parser_bison.c"
    break;

  case 66: /* common_block: error stmt_separator  */
#line 1132 "src/parser_bison.y"
                        {
				if (++state->nerrs == nft->parser_max_errors)
					YYABORT;
				yyerrok;
			}
#line 8559 "src/parser_bison.c"
    break;

  case 67: /* line: common_block  */
#line 1139 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8565 "src/parser_bison.c"
    break;

  case 68: /* line: stmt_separator  */
#line 1140 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8571 "src/parser_bison.c"
    break;

  case 69: /* line: base_cmd stmt_separator  */
#line 1141 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8577 "src/parser_bison.c"
    break;

  case 70: /* line: base_cmd "end of file"  */
#line 1143 "src/parser_bison.y"
                        {
				/*
				 * Very hackish workaround for bison >= 2.4: previous versions
				 * terminated parsing after EOF, 2.4+ tries to get further input
				 * in 'input' and calls the scanner again, causing a crash when
				 * the final input buffer has been popped. Terminate manually to
				 * avoid this. The correct fix should be to adjust the grammar
				 * to accept EOF in input, but for unknown reasons it does not
				 * work.
				 */
				if ((yyvsp[-1].cmd) != NULL) {
					(yyvsp[-1].cmd)->location = (yylsp[-1]);
					list_add_tail(&(yyvsp[-1].cmd)->list, state->cmds);
				}
				(yyval.cmd) = NULL;
				YYACCEPT;
			}
#line 8599 "src/parser_bison.c"
    break;

  case 71: /* base_cmd: add_cmd  */
#line 1162 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8605 "src/parser_bison.c"
    break;

  case 72: /* base_cmd: "add" add_cmd  */
#line 1163 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8611 "src/parser_bison.c"
    break;

  case 73: /* base_cmd: "replace" replace_cmd  */
#line 1164 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8617 "src/parser_bison.c"
    break;

  case 74: /* base_cmd: "create" create_cmd  */
#line 1165 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8623 "src/parser_bison.c"
    break;

  case 75: /* base_cmd: "insert" insert_cmd  */
#line 1166 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8629 "src/parser_bison.c"
    break;

  case 76: /* base_cmd: "delete" delete_cmd  */
#line 1167 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8635 "src/parser_bison.c"
    break;

  case 77: /* base_cmd: "get" get_cmd  */
#line 1168 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8641 "src/parser_bison.c"
    break;

  case 78: /* base_cmd: "list" list_cmd close_scope_list  */
#line 1169 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8647 "src/parser_bison.c"
    break;

  case 79: /* base_cmd: "reset" reset_cmd close_scope_reset  */
#line 1170 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8653 "src/parser_bison.c"
    break;

  case 80: /* base_cmd: "flush" flush_cmd  */
#line 1171 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8659 "src/parser_bison.c"
    break;

  case 81: /* base_cmd: "rename" rename_cmd  */
#line 1172 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8665 "src/parser_bison.c"
    break;

  case 82: /* base_cmd: "import" import_cmd close_scope_import  */
#line 1173 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8671 "src/parser_bison.c"
    break;

  case 83: /* base_cmd: "export" export_cmd close_scope_export  */
#line 1174 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8677 "src/parser_bison.c"
    break;

  case 84: /* base_cmd: "monitor" monitor_cmd close_scope_monitor  */
#line 1175 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8683 "src/parser_bison.c"
    break;

  case 85: /* base_cmd: "describe" describe_cmd  */
#line 1176 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8689 "src/parser_bison.c"
    break;

  case 86: /* base_cmd: "destroy" destroy_cmd close_scope_destroy  */
#line 1177 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8695 "src/parser_bison.c"
    break;

  case 87: /* add_cmd: "table" table_spec  */
#line 1181 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8703 "src/parser_bison.c"
    break;

  case 88: /* add_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1186 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 8713 "src/parser_bison.c"
    break;

  case 89: /* add_cmd: "chain" chain_spec  */
#line 1192 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8721 "src/parser_bison.c"
    break;

  case 90: /* add_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1197 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 8732 "src/parser_bison.c"
    break;

  case 91: /* add_cmd: "rule" rule_position rule  */
#line 1204 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 8740 "src/parser_bison.c"
    break;

  case 92: /* add_cmd: rule_position rule  */
#line 1208 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 8748 "src/parser_bison.c"
    break;

  case 93: /* add_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1213 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8758 "src/parser_bison.c"
    break;

  case 94: /* add_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1220 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8768 "src/parser_bison.c"
    break;

  case 95: /* add_cmd: "element" set_spec set_block_expr  */
#line 1226 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_ADD, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 8782 "src/parser_bison.c"
    break;

  case 96: /* add_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1237 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 8792 "src/parser_bison.c"
    break;

  case 97: /* add_cmd: "counter" obj_spec close_scope_counter  */
#line 1243 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 8805 "src/parser_bison.c"
    break;

  case 98: /* add_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1252 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 8813 "src/parser_bison.c"
    break;

  case 99: /* add_cmd: "counter" obj_spec counter_obj '{' counter_block '}' close_scope_counter  */
#line 1256 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8821 "src/parser_bison.c"
    break;

  case 100: /* add_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1260 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 8829 "src/parser_bison.c"
    break;

  case 101: /* add_cmd: "quota" obj_spec quota_obj '{' quota_block '}' close_scope_quota  */
#line 1264 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8837 "src/parser_bison.c"
    break;

  case 102: /* add_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1268 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8845 "src/parser_bison.c"
    break;

  case 103: /* add_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1272 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8853 "src/parser_bison.c"
    break;

  case 104: /* add_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1276 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8861 "src/parser_bison.c"
    break;

  case 105: /* add_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1280 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 8869 "src/parser_bison.c"
    break;

  case 106: /* add_cmd: "limit" obj_spec limit_obj '{' limit_block '}' close_scope_limit  */
#line 1284 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8877 "src/parser_bison.c"
    break;

  case 107: /* add_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1288 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 8885 "src/parser_bison.c"
    break;

  case 108: /* add_cmd: "secmark" obj_spec secmark_obj '{' secmark_block '}' close_scope_secmark  */
#line 1292 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8893 "src/parser_bison.c"
    break;

  case 109: /* add_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1296 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 8901 "src/parser_bison.c"
    break;

  case 110: /* add_cmd: "synproxy" obj_spec synproxy_obj '{' synproxy_block '}' close_scope_synproxy  */
#line 1300 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 8909 "src/parser_bison.c"
    break;

  case 111: /* replace_cmd: "rule" ruleid_spec rule  */
#line 1306 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_REPLACE, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 8917 "src/parser_bison.c"
    break;

  case 112: /* create_cmd: "table" table_spec  */
#line 1312 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8925 "src/parser_bison.c"
    break;

  case 113: /* create_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1317 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 8935 "src/parser_bison.c"
    break;

  case 114: /* create_cmd: "chain" chain_spec  */
#line 1323 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8943 "src/parser_bison.c"
    break;

  case 115: /* create_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1328 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 8954 "src/parser_bison.c"
    break;

  case 116: /* create_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1336 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8964 "src/parser_bison.c"
    break;

  case 117: /* create_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1343 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8974 "src/parser_bison.c"
    break;

  case 118: /* create_cmd: "element" set_spec set_block_expr  */
#line 1349 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_CREATE, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 8988 "src/parser_bison.c"
    break;

  case 119: /* create_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1360 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 8998 "src/parser_bison.c"
    break;

  case 120: /* create_cmd: "counter" obj_spec close_scope_counter  */
#line 1366 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 9011 "src/parser_bison.c"
    break;

  case 121: /* create_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1375 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9019 "src/parser_bison.c"
    break;

  case 122: /* create_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1379 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9027 "src/parser_bison.c"
    break;

  case 123: /* create_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1383 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9035 "src/parser_bison.c"
    break;

  case 124: /* create_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1387 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9043 "src/parser_bison.c"
    break;

  case 125: /* create_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1391 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9051 "src/parser_bison.c"
    break;

  case 126: /* create_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1395 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9059 "src/parser_bison.c"
    break;

  case 127: /* create_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1399 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9067 "src/parser_bison.c"
    break;

  case 128: /* create_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1403 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9075 "src/parser_bison.c"
    break;

  case 129: /* insert_cmd: "rule" rule_position rule  */
#line 1409 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_INSERT, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9083 "src/parser_bison.c"
    break;

  case 138: /* delete_cmd: "table" table_or_id_spec  */
#line 1431 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9091 "src/parser_bison.c"
    break;

  case 139: /* delete_cmd: "chain" chain_or_id_spec  */
#line 1435 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9099 "src/parser_bison.c"
    break;

  case 140: /* delete_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1440 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9110 "src/parser_bison.c"
    break;

  case 141: /* delete_cmd: "rule" ruleid_spec  */
#line 1447 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9118 "src/parser_bison.c"
    break;

  case 142: /* delete_cmd: "set" set_or_id_spec  */
#line 1451 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9126 "src/parser_bison.c"
    break;

  case 143: /* delete_cmd: "map" set_or_id_spec  */
#line 1455 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9134 "src/parser_bison.c"
    break;

  case 144: /* delete_cmd: "element" set_spec set_block_expr  */
#line 1459 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9142 "src/parser_bison.c"
    break;

  case 145: /* delete_cmd: "flowtable" flowtable_spec  */
#line 1463 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9150 "src/parser_bison.c"
    break;

  case 146: /* delete_cmd: "flowtable" flowtableid_spec  */
#line 1467 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9158 "src/parser_bison.c"
    break;

  case 147: /* delete_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1472 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9168 "src/parser_bison.c"
    break;

  case 148: /* delete_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1478 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9176 "src/parser_bison.c"
    break;

  case 149: /* delete_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1482 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9184 "src/parser_bison.c"
    break;

  case 150: /* delete_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1486 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DELETE, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9194 "src/parser_bison.c"
    break;

  case 151: /* delete_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1492 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9202 "src/parser_bison.c"
    break;

  case 152: /* delete_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1496 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9210 "src/parser_bison.c"
    break;

  case 153: /* delete_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1500 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9218 "src/parser_bison.c"
    break;

  case 154: /* destroy_cmd: "table" table_or_id_spec  */
#line 1506 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9226 "src/parser_bison.c"
    break;

  case 155: /* destroy_cmd: "chain" chain_or_id_spec  */
#line 1510 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9234 "src/parser_bison.c"
    break;

  case 156: /* destroy_cmd: "rule" ruleid_spec  */
#line 1514 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9242 "src/parser_bison.c"
    break;

  case 157: /* destroy_cmd: "set" set_or_id_spec  */
#line 1518 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9250 "src/parser_bison.c"
    break;

  case 158: /* destroy_cmd: "map" set_spec  */
#line 1522 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9258 "src/parser_bison.c"
    break;

  case 159: /* destroy_cmd: "element" set_spec set_block_expr  */
#line 1526 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9266 "src/parser_bison.c"
    break;

  case 160: /* destroy_cmd: "flowtable" flowtable_spec  */
#line 1530 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9274 "src/parser_bison.c"
    break;

  case 161: /* destroy_cmd: "flowtable" flowtableid_spec  */
#line 1534 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9282 "src/parser_bison.c"
    break;

  case 162: /* destroy_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1539 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9292 "src/parser_bison.c"
    break;

  case 163: /* destroy_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1545 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9300 "src/parser_bison.c"
    break;

  case 164: /* destroy_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1549 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9308 "src/parser_bison.c"
    break;

  case 165: /* destroy_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1553 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DESTROY, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9318 "src/parser_bison.c"
    break;

  case 166: /* destroy_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1559 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9326 "src/parser_bison.c"
    break;

  case 167: /* destroy_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1563 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9334 "src/parser_bison.c"
    break;

  case 168: /* destroy_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1567 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9342 "src/parser_bison.c"
    break;

  case 169: /* get_cmd: "element" set_spec set_block_expr  */
#line 1574 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_GET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9350 "src/parser_bison.c"
    break;

  case 170: /* list_cmd_spec_table: "table" table_spec  */
#line 1579 "src/parser_bison.y"
                                                        { (yyval.handle) = (yyvsp[0].handle); }
#line 9356 "src/parser_bison.c"
    break;

  case 174: /* list_cmd: "table" table_spec  */
#line 1587 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9364 "src/parser_bison.c"
    break;

  case 175: /* list_cmd: "tables" ruleset_spec  */
#line 1591 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9372 "src/parser_bison.c"
    break;

  case 176: /* list_cmd: "chain" chain_spec  */
#line 1595 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9380 "src/parser_bison.c"
    break;

  case 177: /* list_cmd: "chains" ruleset_spec  */
#line 1599 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAINS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9388 "src/parser_bison.c"
    break;

  case 178: /* list_cmd: "sets" list_cmd_spec_any  */
#line 1603 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SETS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9396 "src/parser_bison.c"
    break;

  case 179: /* list_cmd: "set" set_spec  */
#line 1607 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9404 "src/parser_bison.c"
    break;

  case 180: /* list_cmd: "counters" list_cmd_spec_any  */
#line 1611 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9412 "src/parser_bison.c"
    break;

  case 181: /* list_cmd: "counter" obj_spec close_scope_counter  */
#line 1615 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9420 "src/parser_bison.c"
    break;

  case 182: /* list_cmd: "quotas" list_cmd_spec_any  */
#line 1619 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9428 "src/parser_bison.c"
    break;

  case 183: /* list_cmd: "quota" obj_spec close_scope_quota  */
#line 1623 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9436 "src/parser_bison.c"
    break;

  case 184: /* list_cmd: "limits" list_cmd_spec_any  */
#line 1627 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMITS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9444 "src/parser_bison.c"
    break;

  case 185: /* list_cmd: "limit" obj_spec close_scope_limit  */
#line 1631 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9452 "src/parser_bison.c"
    break;

  case 186: /* list_cmd: "secmarks" list_cmd_spec_any  */
#line 1635 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9460 "src/parser_bison.c"
    break;

  case 187: /* list_cmd: "secmark" obj_spec close_scope_secmark  */
#line 1639 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9468 "src/parser_bison.c"
    break;

  case 188: /* list_cmd: "synproxys" list_cmd_spec_any  */
#line 1643 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXYS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9476 "src/parser_bison.c"
    break;

  case 189: /* list_cmd: "synproxy" obj_spec close_scope_synproxy  */
#line 1647 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9484 "src/parser_bison.c"
    break;

  case 190: /* list_cmd: "ruleset" ruleset_spec  */
#line 1651 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9492 "src/parser_bison.c"
    break;

  case 191: /* list_cmd: "flow" "tables" ruleset_spec  */
#line 1655 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9500 "src/parser_bison.c"
    break;

  case 192: /* list_cmd: "flow" "table" set_spec  */
#line 1659 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9508 "src/parser_bison.c"
    break;

  case 193: /* list_cmd: "meters" ruleset_spec  */
#line 1663 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9516 "src/parser_bison.c"
    break;

  case 194: /* list_cmd: "meter" set_spec  */
#line 1667 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9524 "src/parser_bison.c"
    break;

  case 195: /* list_cmd: "flowtables" list_cmd_spec_any  */
#line 1671 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9532 "src/parser_bison.c"
    break;

  case 196: /* list_cmd: "flowtable" flowtable_spec  */
#line 1675 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9540 "src/parser_bison.c"
    break;

  case 197: /* list_cmd: "maps" list_cmd_spec_any  */
#line 1679 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAPS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9548 "src/parser_bison.c"
    break;

  case 198: /* list_cmd: "map" set_spec  */
#line 1683 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9556 "src/parser_bison.c"
    break;

  case 199: /* list_cmd: "ct" ct_obj_type obj_spec close_scope_ct  */
#line 1687 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_LIST, (yyvsp[-2].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9564 "src/parser_bison.c"
    break;

  case 200: /* list_cmd: "ct" ct_cmd_type "table" table_spec close_scope_ct  */
#line 1691 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, (yyvsp[-3].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9572 "src/parser_bison.c"
    break;

  case 201: /* list_cmd: "hooks" basehook_spec  */
#line 1695 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_HOOKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9580 "src/parser_bison.c"
    break;

  case 202: /* basehook_device_name: "device" "string"  */
#line 1701 "src/parser_bison.y"
                        {
				(yyval.string) = (yyvsp[0].string);
			}
#line 9588 "src/parser_bison.c"
    break;

  case 203: /* basehook_spec: ruleset_spec  */
#line 1707 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 9596 "src/parser_bison.c"
    break;

  case 204: /* basehook_spec: ruleset_spec basehook_device_name  */
#line 1711 "src/parser_bison.y"
                        {
				if ((yyvsp[0].string)) {
					(yyvsp[-1].handle).obj.name = (yyvsp[0].string);
					(yyvsp[-1].handle).obj.location = (yylsp[0]);
				}
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 9608 "src/parser_bison.c"
    break;

  case 205: /* reset_cmd: "counters" list_cmd_spec_any  */
#line 1721 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9616 "src/parser_bison.c"
    break;

  case 206: /* reset_cmd: "counter" obj_spec close_scope_counter  */
#line 1725 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTER, &(yyvsp[-1].handle),&(yyloc), NULL);
			}
#line 9624 "src/parser_bison.c"
    break;

  case 207: /* reset_cmd: "quotas" list_cmd_spec_any  */
#line 1729 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9632 "src/parser_bison.c"
    break;

  case 208: /* reset_cmd: "quota" obj_spec close_scope_quota  */
#line 1733 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9640 "src/parser_bison.c"
    break;

  case 209: /* reset_cmd: "rules" ruleset_spec  */
#line 1737 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9648 "src/parser_bison.c"
    break;

  case 210: /* reset_cmd: "rules" list_cmd_spec_table  */
#line 1741 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9656 "src/parser_bison.c"
    break;

  case 211: /* reset_cmd: "rules" chain_spec  */
#line 1745 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9664 "src/parser_bison.c"
    break;

  case 212: /* reset_cmd: "rules" "chain" chain_spec  */
#line 1749 "src/parser_bison.y"
                        {
				/* alias of previous rule. */
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9673 "src/parser_bison.c"
    break;

  case 213: /* reset_cmd: "rule" ruleid_spec  */
#line 1754 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9681 "src/parser_bison.c"
    break;

  case 214: /* reset_cmd: "element" set_spec set_block_expr  */
#line 1758 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9689 "src/parser_bison.c"
    break;

  case 215: /* reset_cmd: "set" set_spec  */
#line 1762 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9697 "src/parser_bison.c"
    break;

  case 216: /* reset_cmd: "map" set_spec  */
#line 1766 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9705 "src/parser_bison.c"
    break;

  case 217: /* flush_cmd: "table" table_spec  */
#line 1772 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9713 "src/parser_bison.c"
    break;

  case 218: /* flush_cmd: "chain" chain_spec  */
#line 1776 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9721 "src/parser_bison.c"
    break;

  case 219: /* flush_cmd: "set" set_spec  */
#line 1780 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9729 "src/parser_bison.c"
    break;

  case 220: /* flush_cmd: "map" set_spec  */
#line 1784 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9737 "src/parser_bison.c"
    break;

  case 221: /* flush_cmd: "flow" "table" set_spec  */
#line 1788 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9745 "src/parser_bison.c"
    break;

  case 222: /* flush_cmd: "meter" set_spec  */
#line 1792 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9753 "src/parser_bison.c"
    break;

  case 223: /* flush_cmd: "ruleset" ruleset_spec  */
#line 1796 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9761 "src/parser_bison.c"
    break;

  case 224: /* rename_cmd: "chain" chain_spec identifier  */
#line 1802 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RENAME, CMD_OBJ_CHAIN, &(yyvsp[-1].handle), &(yyloc), NULL);
				(yyval.cmd)->arg = (yyvsp[0].string);
			}
#line 9770 "src/parser_bison.c"
    break;

  case 225: /* import_cmd: "ruleset" markup_format  */
#line 1809 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 9780 "src/parser_bison.c"
    break;

  case 226: /* import_cmd: markup_format  */
#line 1815 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 9790 "src/parser_bison.c"
    break;

  case 227: /* export_cmd: "ruleset" markup_format  */
#line 1823 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 9800 "src/parser_bison.c"
    break;

  case 228: /* export_cmd: markup_format  */
#line 1829 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 9810 "src/parser_bison.c"
    break;

  case 229: /* monitor_cmd: monitor_event monitor_object monitor_format  */
#line 1837 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct monitor *m = monitor_alloc((yyvsp[0].val), (yyvsp[-1].val), (yyvsp[-2].string));
				m->location = (yylsp[-2]);
				(yyval.cmd) = cmd_alloc(CMD_MONITOR, CMD_OBJ_MONITOR, &h, &(yyloc), m);
			}
#line 9821 "src/parser_bison.c"
    break;

  case 230: /* monitor_event: %empty  */
#line 1845 "src/parser_bison.y"
                                                { (yyval.string) = NULL; }
#line 9827 "src/parser_bison.c"
    break;

  case 231: /* monitor_event: "string"  */
#line 1846 "src/parser_bison.y"
                                                { (yyval.string) = (yyvsp[0].string); }
#line 9833 "src/parser_bison.c"
    break;

  case 232: /* monitor_object: %empty  */
#line 1849 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ANY; }
#line 9839 "src/parser_bison.c"
    break;

  case 233: /* monitor_object: "tables"  */
#line 1850 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TABLES; }
#line 9845 "src/parser_bison.c"
    break;

  case 234: /* monitor_object: "chains"  */
#line 1851 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_CHAINS; }
#line 9851 "src/parser_bison.c"
    break;

  case 235: /* monitor_object: "sets"  */
#line 1852 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_SETS; }
#line 9857 "src/parser_bison.c"
    break;

  case 236: /* monitor_object: "rules"  */
#line 1853 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULES; }
#line 9863 "src/parser_bison.c"
    break;

  case 237: /* monitor_object: "elements"  */
#line 1854 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ELEMS; }
#line 9869 "src/parser_bison.c"
    break;

  case 238: /* monitor_object: "ruleset"  */
#line 1855 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULESET; }
#line 9875 "src/parser_bison.c"
    break;

  case 239: /* monitor_object: "trace"  */
#line 1856 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TRACE; }
#line 9881 "src/parser_bison.c"
    break;

  case 240: /* monitor_format: %empty  */
#line 1859 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_DEFAULT; }
#line 9887 "src/parser_bison.c"
    break;

  case 242: /* markup_format: "xml"  */
#line 1863 "src/parser_bison.y"
                                                { (yyval.val) = __NFT_OUTPUT_NOTSUPP; }
#line 9893 "src/parser_bison.c"
    break;

  case 243: /* markup_format: "json"  */
#line 1864 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 9899 "src/parser_bison.c"
    break;

  case 244: /* markup_format: "vm" "json"  */
#line 1865 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 9905 "src/parser_bison.c"
    break;

  case 245: /* describe_cmd: primary_expr  */
#line 1869 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				(yyval.cmd) = cmd_alloc(CMD_DESCRIBE, CMD_OBJ_EXPR, &h, &(yyloc), NULL);
				(yyval.cmd)->expr = (yyvsp[0].expr);
			}
#line 9915 "src/parser_bison.c"
    break;

  case 246: /* table_block_alloc: %empty  */
#line 1877 "src/parser_bison.y"
                        {
				(yyval.table) = table_alloc();
				if (open_scope(state, &(yyval.table)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 9928 "src/parser_bison.c"
    break;

  case 247: /* table_options: "flags" table_flags  */
#line 1888 "src/parser_bison.y"
                        {
				(yyvsp[-2].table)->flags |= (yyvsp[0].val);
			}
#line 9936 "src/parser_bison.c"
    break;

  case 248: /* table_options: comment_spec  */
#line 1892 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].table)->comment, &(yyloc), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].table)->comment = (yyvsp[0].string);
			}
#line 9948 "src/parser_bison.c"
    break;

  case 250: /* table_flags: table_flags "comma" table_flag  */
#line 1903 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 9956 "src/parser_bison.c"
    break;

  case 251: /* table_flag: "string"  */
#line 1908 "src/parser_bison.y"
                        {
				(yyval.val) = parse_table_flag((yyvsp[0].string));
				if ((yyval.val) == 0) {
					erec_queue(error(&(yylsp[0]), "unknown table option %s", (yyvsp[0].string)),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}

				free_const((yyvsp[0].string));
			}
#line 9972 "src/parser_bison.c"
    break;

  case 252: /* table_block: %empty  */
#line 1921 "src/parser_bison.y"
                                                { (yyval.table) = (yyvsp[(-1) - (0)].table); }
#line 9978 "src/parser_bison.c"
    break;

  case 256: /* table_block: table_block "chain" chain_identifier chain_block_alloc '{' chain_block '}' stmt_separator  */
#line 1928 "src/parser_bison.y"
                        {
				(yyvsp[-4].chain)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].chain)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				close_scope(state);
				list_add_tail(&(yyvsp[-4].chain)->list, &(yyvsp[-7].table)->chains);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 9991 "src/parser_bison.c"
    break;

  case 257: /* table_block: table_block "set" set_identifier set_block_alloc '{' set_block '}' stmt_separator  */
#line 1939 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10003 "src/parser_bison.c"
    break;

  case 258: /* table_block: table_block "map" set_identifier map_block_alloc '{' map_block '}' stmt_separator  */
#line 1949 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10015 "src/parser_bison.c"
    break;

  case 259: /* table_block: table_block "flowtable" flowtable_identifier flowtable_block_alloc '{' flowtable_block '}' stmt_separator  */
#line 1960 "src/parser_bison.y"
                        {
				(yyvsp[-4].flowtable)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].flowtable)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].flowtable)->list, &(yyvsp[-7].table)->flowtables);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10027 "src/parser_bison.c"
    break;

  case 260: /* table_block: table_block "counter" obj_identifier obj_block_alloc '{' counter_block '}' stmt_separator close_scope_counter  */
#line 1970 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_COUNTER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10040 "src/parser_bison.c"
    break;

  case 261: /* table_block: table_block "quota" obj_identifier obj_block_alloc '{' quota_block '}' stmt_separator close_scope_quota  */
#line 1981 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_QUOTA;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10053 "src/parser_bison.c"
    break;

  case 262: /* table_block: table_block "ct" "helper" obj_identifier obj_block_alloc '{' ct_helper_block '}' stmt_separator close_scope_ct  */
#line 1990 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_HELPER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10066 "src/parser_bison.c"
    break;

  case 263: /* table_block: table_block "ct" "timeout" obj_identifier obj_block_alloc '{' ct_timeout_block '}' stmt_separator close_scope_ct  */
#line 1999 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_TIMEOUT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10079 "src/parser_bison.c"
    break;

  case 264: /* table_block: table_block "ct" "expectation" obj_identifier obj_block_alloc '{' ct_expect_block '}' stmt_separator close_scope_ct  */
#line 2008 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_EXPECT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10092 "src/parser_bison.c"
    break;

  case 265: /* table_block: table_block "limit" obj_identifier obj_block_alloc '{' limit_block '}' stmt_separator close_scope_limit  */
#line 2019 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_LIMIT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10105 "src/parser_bison.c"
    break;

  case 266: /* table_block: table_block "secmark" obj_identifier obj_block_alloc '{' secmark_block '}' stmt_separator close_scope_secmark  */
#line 2030 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SECMARK;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10118 "src/parser_bison.c"
    break;

  case 267: /* table_block: table_block "synproxy" obj_identifier obj_block_alloc '{' synproxy_block '}' stmt_separator close_scope_synproxy  */
#line 2041 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SYNPROXY;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10131 "src/parser_bison.c"
    break;

  case 268: /* chain_block_alloc: %empty  */
#line 2052 "src/parser_bison.y"
                        {
				(yyval.chain) = chain_alloc();
				if (open_scope(state, &(yyval.chain)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 10144 "src/parser_bison.c"
    break;

  case 269: /* chain_block: %empty  */
#line 2062 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10150 "src/parser_bison.c"
    break;

  case 275: /* chain_block: chain_block rule stmt_separator  */
#line 2069 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10159 "src/parser_bison.c"
    break;

  case 276: /* chain_block: chain_block "devices" '=' flowtable_expr stmt_separator  */
#line 2074 "src/parser_bison.y"
                        {
				if ((yyval.chain)->dev_expr) {
					list_splice_init(&expr_list((yyvsp[-1].expr))->expressions, &expr_list((yyval.chain)->dev_expr)->expressions);
					expr_free((yyvsp[-1].expr));
					break;
				}
				(yyval.chain)->dev_expr = (yyvsp[-1].expr);
			}
#line 10172 "src/parser_bison.c"
    break;

  case 277: /* chain_block: chain_block comment_spec stmt_separator  */
#line 2083 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].chain)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].chain)->comment = (yyvsp[-1].string);
			}
#line 10184 "src/parser_bison.c"
    break;

  case 278: /* subchain_block: %empty  */
#line 2092 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10190 "src/parser_bison.c"
    break;

  case 280: /* subchain_block: subchain_block rule stmt_separator  */
#line 2095 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10199 "src/parser_bison.c"
    break;

  case 281: /* typeof_verdict_expr: selector_expr  */
#line 2102 "src/parser_bison.y"
                        {
				struct expr *e = (yyvsp[0].expr);

				if (expr_ops(e)->build_udata == NULL) {
					erec_queue(error(&(yylsp[0]), "map data type '%s' lacks typeof serialization", expr_ops(e)->name),
						   state->msgs);
					expr_free(e);
					YYERROR;
				}
				(yyval.expr) = e;
			}
#line 10215 "src/parser_bison.c"
    break;

  case 282: /* typeof_verdict_expr: typeof_expr "." selector_expr  */
#line 2114 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10228 "src/parser_bison.c"
    break;

  case 283: /* typeof_data_expr: "interval" typeof_expr  */
#line 2125 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->flags |= EXPR_F_INTERVAL;
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10237 "src/parser_bison.c"
    break;

  case 284: /* typeof_data_expr: typeof_verdict_expr  */
#line 2130 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10245 "src/parser_bison.c"
    break;

  case 285: /* typeof_data_expr: "queue"  */
#line 2134 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &queue_type, BYTEORDER_HOST_ENDIAN, 16, NULL);
			}
#line 10253 "src/parser_bison.c"
    break;

  case 286: /* typeof_data_expr: "string"  */
#line 2138 "src/parser_bison.y"
                        {
				struct expr *verdict;

				if (strcmp("verdict", (yyvsp[0].string)) != 0) {
					erec_queue(error(&(yylsp[0]), "map data type '%s' lacks typeof serialization", (yyvsp[0].string)),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				verdict = verdict_expr_alloc(&(yylsp[0]), NF_ACCEPT, NULL);
				verdict->flags &= ~EXPR_F_CONSTANT;
				(yyval.expr) = verdict;
				free_const((yyvsp[0].string));
			}
#line 10272 "src/parser_bison.c"
    break;

  case 287: /* primary_typeof_expr: selector_expr  */
#line 2155 "src/parser_bison.y"
                        {
				if (expr_ops((yyvsp[0].expr))->build_udata == NULL) {
					erec_queue(error(&(yylsp[0]), "primary expression type '%s' lacks typeof serialization", expr_ops((yyvsp[0].expr))->name),
						   state->msgs);
					expr_free((yyvsp[0].expr));
					YYERROR;
				}

				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10287 "src/parser_bison.c"
    break;

  case 288: /* typeof_expr: primary_typeof_expr  */
#line 2168 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10295 "src/parser_bison.c"
    break;

  case 289: /* typeof_expr: typeof_expr "." primary_typeof_expr  */
#line 2172 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10308 "src/parser_bison.c"
    break;

  case 290: /* set_block_alloc: %empty  */
#line 2184 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10316 "src/parser_bison.c"
    break;

  case 291: /* typeof_key_expr: "typeof" typeof_expr  */
#line 2189 "src/parser_bison.y"
                                                    { (yyval.expr) = (yyvsp[0].expr); }
#line 10322 "src/parser_bison.c"
    break;

  case 292: /* typeof_key_expr: "type" data_type_expr close_scope_type  */
#line 2190 "src/parser_bison.y"
                                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 10328 "src/parser_bison.c"
    break;

  case 293: /* set_block: %empty  */
#line 2193 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 10334 "src/parser_bison.c"
    break;

  case 296: /* set_block: set_block typeof_key_expr stmt_separator  */
#line 2197 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->key, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-2].set)->key = (yyvsp[-1].expr);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10348 "src/parser_bison.c"
    break;

  case 297: /* set_block: set_block "flags" set_flag_list stmt_separator  */
#line 2207 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10357 "src/parser_bison.c"
    break;

  case 298: /* set_block: set_block "timeout" time_spec stmt_separator  */
#line 2212 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10366 "src/parser_bison.c"
    break;

  case 299: /* set_block: set_block "gc-interval" time_spec stmt_separator  */
#line 2217 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10375 "src/parser_bison.c"
    break;

  case 300: /* set_block: set_block stateful_stmt_list stmt_separator  */
#line 2222 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 10385 "src/parser_bison.c"
    break;

  case 301: /* set_block: set_block "elements" '=' set_block_expr  */
#line 2228 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-3].set)->init, &(yylsp[-2]), state)) {
					expr_free((yyvsp[0].expr));
					YYERROR;
				}
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10398 "src/parser_bison.c"
    break;

  case 302: /* set_block: set_block "auto-merge"  */
#line 2237 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->automerge = true;
				(yyval.set) = (yyvsp[-1].set);
			}
#line 10407 "src/parser_bison.c"
    break;

  case 304: /* set_block: set_block comment_spec stmt_separator  */
#line 2243 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10420 "src/parser_bison.c"
    break;

  case 307: /* set_flag_list: set_flag_list "comma" set_flag  */
#line 2258 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 10428 "src/parser_bison.c"
    break;

  case 309: /* set_flag: "constant"  */
#line 2264 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_CONSTANT; }
#line 10434 "src/parser_bison.c"
    break;

  case 310: /* set_flag: "interval"  */
#line 2265 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_INTERVAL; }
#line 10440 "src/parser_bison.c"
    break;

  case 311: /* set_flag: "timeout"  */
#line 2266 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_TIMEOUT; }
#line 10446 "src/parser_bison.c"
    break;

  case 312: /* set_flag: "dynamic"  */
#line 2267 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_EVAL; }
#line 10452 "src/parser_bison.c"
    break;

  case 313: /* map_block_alloc: %empty  */
#line 2271 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10460 "src/parser_bison.c"
    break;

  case 314: /* ct_obj_type_map: "timeout"  */
#line 2276 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 10466 "src/parser_bison.c"
    break;

  case 315: /* ct_obj_type_map: "expectation"  */
#line 2277 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 10472 "src/parser_bison.c"
    break;

  case 316: /* map_block_obj_type: "counter" close_scope_counter  */
#line 2280 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_COUNTER; }
#line 10478 "src/parser_bison.c"
    break;

  case 317: /* map_block_obj_type: "quota" close_scope_quota  */
#line 2281 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_QUOTA; }
#line 10484 "src/parser_bison.c"
    break;

  case 318: /* map_block_obj_type: "limit" close_scope_limit  */
#line 2282 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_LIMIT; }
#line 10490 "src/parser_bison.c"
    break;

  case 319: /* map_block_obj_type: "secmark" close_scope_secmark  */
#line 2283 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_SECMARK; }
#line 10496 "src/parser_bison.c"
    break;

  case 320: /* map_block_obj_type: "synproxy" close_scope_synproxy  */
#line 2284 "src/parser_bison.y"
                                                              { (yyval.val) = NFT_OBJECT_SYNPROXY; }
#line 10502 "src/parser_bison.c"
    break;

  case 322: /* map_block_obj_typeof: "ct" ct_obj_type_map close_scope_ct  */
#line 2288 "src/parser_bison.y"
                                                                        { (yyval.val) = (yyvsp[-1].val); }
#line 10508 "src/parser_bison.c"
    break;

  case 323: /* map_block_data_interval: "interval"  */
#line 2291 "src/parser_bison.y"
                                         { (yyval.val) = EXPR_F_INTERVAL; }
#line 10514 "src/parser_bison.c"
    break;

  case 324: /* map_block_data_interval: %empty  */
#line 2292 "src/parser_bison.y"
                                { (yyval.val) = 0; }
#line 10520 "src/parser_bison.c"
    break;

  case 325: /* map_block: %empty  */
#line 2295 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 10526 "src/parser_bison.c"
    break;

  case 328: /* map_block: map_block "timeout" time_spec stmt_separator  */
#line 2299 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10535 "src/parser_bison.c"
    break;

  case 329: /* map_block: map_block "gc-interval" time_spec stmt_separator  */
#line 2304 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10544 "src/parser_bison.c"
    break;

  case 330: /* map_block: map_block "type" data_type_expr "colon" map_block_data_interval data_type_expr stmt_separator close_scope_type  */
#line 2311 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-7].set)->key, &(yylsp[-6]), state)) {
					expr_free((yyvsp[-5].expr));
					expr_free((yyvsp[-2].expr));
					YYERROR;
				}

				(yyvsp[-7].set)->key = (yyvsp[-5].expr);
				(yyvsp[-7].set)->data = (yyvsp[-2].expr);
				(yyvsp[-7].set)->data->flags |= (yyvsp[-3].val);

				(yyvsp[-7].set)->flags |= NFT_SET_MAP;
				(yyval.set) = (yyvsp[-7].set);
			}
#line 10563 "src/parser_bison.c"
    break;

  case 331: /* map_block: map_block "typeof" typeof_expr "colon" typeof_data_expr stmt_separator  */
#line 2328 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-5].set)->key, &(yylsp[-4]), state)) {
					expr_free((yyvsp[-3].expr));
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-5].set)->key = (yyvsp[-3].expr);

				if ((yyvsp[-1].expr)->etype == EXPR_CT && (yyvsp[-1].expr)->ct.key == NFT_CT_HELPER) {
					(yyvsp[-5].set)->objtype = NFT_OBJECT_CT_HELPER;
					(yyvsp[-5].set)->flags  |= NFT_SET_OBJECT;
					expr_free((yyvsp[-1].expr));
				} else {
					(yyvsp[-5].set)->data = (yyvsp[-1].expr);
					(yyvsp[-5].set)->flags |= NFT_SET_MAP;
				}

				(yyval.set) = (yyvsp[-5].set);
			}
#line 10588 "src/parser_bison.c"
    break;

  case 332: /* map_block: map_block "type" data_type_expr "colon" map_block_obj_type stmt_separator close_scope_type  */
#line 2351 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-6].set)->key, &(yylsp[-5]), state)) {
					expr_free((yyvsp[-4].expr));
					YYERROR;
				}

				(yyvsp[-6].set)->key = (yyvsp[-4].expr);
				(yyvsp[-6].set)->objtype = (yyvsp[-2].val);
				(yyvsp[-6].set)->flags  |= NFT_SET_OBJECT;
				(yyval.set) = (yyvsp[-6].set);
			}
#line 10604 "src/parser_bison.c"
    break;

  case 333: /* map_block: map_block "typeof" typeof_expr "colon" map_block_obj_typeof stmt_separator  */
#line 2365 "src/parser_bison.y"
                        {
				(yyvsp[-5].set)->key = (yyvsp[-3].expr);
				(yyvsp[-5].set)->objtype = (yyvsp[-1].val);
				(yyvsp[-5].set)->flags  |= NFT_SET_OBJECT;
				(yyval.set) = (yyvsp[-5].set);
			}
#line 10615 "src/parser_bison.c"
    break;

  case 334: /* map_block: map_block "flags" set_flag_list stmt_separator  */
#line 2372 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags |= (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10624 "src/parser_bison.c"
    break;

  case 335: /* map_block: map_block stateful_stmt_list stmt_separator  */
#line 2377 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 10634 "src/parser_bison.c"
    break;

  case 336: /* map_block: map_block "elements" '=' set_block_expr  */
#line 2383 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10643 "src/parser_bison.c"
    break;

  case 337: /* map_block: map_block comment_spec stmt_separator  */
#line 2388 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10656 "src/parser_bison.c"
    break;

  case 339: /* set_mechanism: "policy" set_policy_spec close_scope_policy  */
#line 2400 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->policy = (yyvsp[-1].val);
			}
#line 10664 "src/parser_bison.c"
    break;

  case 340: /* set_mechanism: "size" "number"  */
#line 2404 "src/parser_bison.y"
                        {
				(yyvsp[-2].set)->desc.size = (yyvsp[0].val);
			}
#line 10672 "src/parser_bison.c"
    break;

  case 341: /* set_policy_spec: "performance"  */
#line 2409 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_PERFORMANCE; }
#line 10678 "src/parser_bison.c"
    break;

  case 342: /* set_policy_spec: "memory"  */
#line 2410 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_MEMORY; }
#line 10684 "src/parser_bison.c"
    break;

  case 343: /* flowtable_block_alloc: %empty  */
#line 2414 "src/parser_bison.y"
                        {
				(yyval.flowtable) = flowtable_alloc(&internal_location);
			}
#line 10692 "src/parser_bison.c"
    break;

  case 344: /* flowtable_block: %empty  */
#line 2419 "src/parser_bison.y"
                                                { (yyval.flowtable) = (yyvsp[(-1) - (0)].flowtable); }
#line 10698 "src/parser_bison.c"
    break;

  case 347: /* flowtable_block: flowtable_block "hook" "string" prio_spec stmt_separator  */
#line 2423 "src/parser_bison.y"
                        {
				(yyval.flowtable)->hook.loc = (yylsp[-2]);
				(yyval.flowtable)->hook.name = chain_hookname_lookup((yyvsp[-2].string));
				if ((yyval.flowtable)->hook.name == NULL) {
					erec_queue(error(&(yylsp[-2]), "unknown chain hook"),
						   state->msgs);
					free_const((yyvsp[-2].string));
					expr_free((yyvsp[-1].prio_spec).expr);
					YYERROR;
				}
				free_const((yyvsp[-2].string));

				(yyval.flowtable)->priority = (yyvsp[-1].prio_spec);
			}
#line 10717 "src/parser_bison.c"
    break;

  case 348: /* flowtable_block: flowtable_block "devices" '=' flowtable_expr stmt_separator  */
#line 2438 "src/parser_bison.y"
                        {
				(yyval.flowtable)->dev_expr = (yyvsp[-1].expr);
			}
#line 10725 "src/parser_bison.c"
    break;

  case 349: /* flowtable_block: flowtable_block "counter" close_scope_counter  */
#line 2442 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= NFT_FLOWTABLE_COUNTER;
			}
#line 10733 "src/parser_bison.c"
    break;

  case 350: /* flowtable_block: flowtable_block "flags" "offload" stmt_separator  */
#line 2446 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= FLOWTABLE_F_HW_OFFLOAD;
			}
#line 10741 "src/parser_bison.c"
    break;

  case 351: /* flowtable_expr: '{' flowtable_list_expr '}'  */
#line 2452 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 10750 "src/parser_bison.c"
    break;

  case 352: /* flowtable_expr: variable_expr  */
#line 2457 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10759 "src/parser_bison.c"
    break;

  case 353: /* flowtable_list_expr: flowtable_expr_member  */
#line 2464 "src/parser_bison.y"
                        {
				(yyval.expr) = compound_expr_alloc(&(yyloc), EXPR_LIST);
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 10768 "src/parser_bison.c"
    break;

  case 354: /* flowtable_list_expr: flowtable_list_expr "comma" flowtable_expr_member  */
#line 2469 "src/parser_bison.y"
                        {
				compound_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 10777 "src/parser_bison.c"
    break;

  case 356: /* flowtable_expr_member: "quoted string"  */
#line 2477 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = expr;
			}
#line 10790 "src/parser_bison.c"
    break;

  case 357: /* flowtable_expr_member: "string"  */
#line 2486 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = expr;
			}
#line 10803 "src/parser_bison.c"
    break;

  case 358: /* flowtable_expr_member: variable_expr  */
#line 2495 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10812 "src/parser_bison.c"
    break;

  case 359: /* data_type_atom_expr: type_identifier  */
#line 2502 "src/parser_bison.y"
                        {
				const struct datatype *dtype = datatype_lookup_byname((yyvsp[0].string));
				if (dtype == NULL) {
					erec_queue(error(&(yylsp[0]), "unknown datatype %s", (yyvsp[0].string)),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyval.expr) = constant_expr_alloc(&(yylsp[0]), dtype, dtype->byteorder,
							 dtype->size, NULL);
				free_const((yyvsp[0].string));
			}
#line 10829 "src/parser_bison.c"
    break;

  case 360: /* data_type_atom_expr: "time"  */
#line 2515 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yylsp[0]), &time_type, time_type.byteorder,
							 time_type.size, NULL);
			}
#line 10838 "src/parser_bison.c"
    break;

  case 362: /* data_type_expr: data_type_expr "." data_type_atom_expr  */
#line 2523 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10851 "src/parser_bison.c"
    break;

  case 363: /* obj_block_alloc: %empty  */
#line 2534 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&internal_location);
			}
#line 10859 "src/parser_bison.c"
    break;

  case 364: /* counter_block: %empty  */
#line 2539 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 10865 "src/parser_bison.c"
    break;

  case 367: /* counter_block: counter_block counter_config  */
#line 2543 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 10873 "src/parser_bison.c"
    break;

  case 368: /* counter_block: counter_block comment_spec  */
#line 2547 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 10885 "src/parser_bison.c"
    break;

  case 369: /* quota_block: %empty  */
#line 2556 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 10891 "src/parser_bison.c"
    break;

  case 372: /* quota_block: quota_block quota_config  */
#line 2560 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 10899 "src/parser_bison.c"
    break;

  case 373: /* quota_block: quota_block comment_spec  */
#line 2564 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 10911 "src/parser_bison.c"
    break;

  case 374: /* ct_helper_block: %empty  */
#line 2573 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 10917 "src/parser_bison.c"
    break;

  case 377: /* ct_helper_block: ct_helper_block ct_helper_config  */
#line 2577 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 10925 "src/parser_bison.c"
    break;

  case 378: /* ct_helper_block: ct_helper_block comment_spec  */
#line 2581 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 10937 "src/parser_bison.c"
    break;

  case 379: /* ct_timeout_block: %empty  */
#line 2591 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);
				init_list_head(&(yyval.obj)->ct_timeout.timeout_list);
				(yyval.obj)->type = NFT_OBJECT_CT_TIMEOUT;
			}
#line 10947 "src/parser_bison.c"
    break;

  case 382: /* ct_timeout_block: ct_timeout_block ct_timeout_config  */
#line 2599 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 10955 "src/parser_bison.c"
    break;

  case 383: /* ct_timeout_block: ct_timeout_block comment_spec  */
#line 2603 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 10967 "src/parser_bison.c"
    break;

  case 384: /* ct_expect_block: %empty  */
#line 2612 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 10973 "src/parser_bison.c"
    break;

  case 387: /* ct_expect_block: ct_expect_block ct_expect_config  */
#line 2616 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 10981 "src/parser_bison.c"
    break;

  case 388: /* ct_expect_block: ct_expect_block comment_spec  */
#line 2620 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 10993 "src/parser_bison.c"
    break;

  case 389: /* limit_block: %empty  */
#line 2629 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 10999 "src/parser_bison.c"
    break;

  case 392: /* limit_block: limit_block limit_config  */
#line 2633 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11007 "src/parser_bison.c"
    break;

  case 393: /* limit_block: limit_block comment_spec  */
#line 2637 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11019 "src/parser_bison.c"
    break;

  case 394: /* secmark_block: %empty  */
#line 2646 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11025 "src/parser_bison.c"
    break;

  case 397: /* secmark_block: secmark_block secmark_config  */
#line 2650 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11033 "src/parser_bison.c"
    break;

  case 398: /* secmark_block: secmark_block comment_spec  */
#line 2654 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11045 "src/parser_bison.c"
    break;

  case 399: /* synproxy_block: %empty  */
#line 2663 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11051 "src/parser_bison.c"
    break;

  case 402: /* synproxy_block: synproxy_block synproxy_config  */
#line 2667 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11059 "src/parser_bison.c"
    break;

  case 403: /* synproxy_block: synproxy_block comment_spec  */
#line 2671 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11071 "src/parser_bison.c"
    break;

  case 404: /* type_identifier: "string"  */
#line 2680 "src/parser_bison.y"
                                        { (yyval.string) = (yyvsp[0].string); }
#line 11077 "src/parser_bison.c"
    break;

  case 405: /* type_identifier: "mark"  */
#line 2681 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("mark"); }
#line 11083 "src/parser_bison.c"
    break;

  case 406: /* type_identifier: "dscp"  */
#line 2682 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("dscp"); }
#line 11089 "src/parser_bison.c"
    break;

  case 407: /* type_identifier: "ecn"  */
#line 2683 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("ecn"); }
#line 11095 "src/parser_bison.c"
    break;

  case 408: /* type_identifier: "classid"  */
#line 2684 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("classid"); }
#line 11101 "src/parser_bison.c"
    break;

  case 409: /* hook_spec: "type" close_scope_type "string" "hook" "string" dev_spec prio_spec  */
#line 2688 "src/parser_bison.y"
                        {
				const char *chain_type = chain_type_name_lookup((yyvsp[-4].string));

				if (chain_type == NULL) {
					erec_queue(error(&(yylsp[-4]), "unknown chain type"),
						   state->msgs);
					free_const((yyvsp[-4].string));
					free_const((yyvsp[-2].string));
					expr_free((yyvsp[-1].expr));
					expr_free((yyvsp[0].prio_spec).expr);
					YYERROR;
				}
				(yyvsp[-7].chain)->type.loc = (yylsp[-4]);
				(yyvsp[-7].chain)->type.str = xstrdup(chain_type);
				free_const((yyvsp[-4].string));

				(yyvsp[-7].chain)->loc = (yyloc);
				(yyvsp[-7].chain)->hook.loc = (yylsp[-2]);
				(yyvsp[-7].chain)->hook.name = chain_hookname_lookup((yyvsp[-2].string));
				if ((yyvsp[-7].chain)->hook.name == NULL) {
					erec_queue(error(&(yylsp[-2]), "unknown chain hook"),
						   state->msgs);
					free_const((yyvsp[-2].string));
					expr_free((yyvsp[-1].expr));
					expr_free((yyvsp[0].prio_spec).expr);
					YYERROR;
				}
				free_const((yyvsp[-2].string));

				(yyvsp[-7].chain)->dev_expr	= (yyvsp[-1].expr);
				(yyvsp[-7].chain)->priority	= (yyvsp[0].prio_spec);
				(yyvsp[-7].chain)->flags	|= CHAIN_F_BASECHAIN;
			}
#line 11139 "src/parser_bison.c"
    break;

  case 410: /* prio_spec: "priority" extended_prio_spec  */
#line 2724 "src/parser_bison.y"
                        {
				(yyval.prio_spec) = (yyvsp[0].prio_spec);
				(yyval.prio_spec).loc = (yyloc);
			}
#line 11148 "src/parser_bison.c"
    break;

  case 411: /* extended_prio_name: "out"  */
#line 2731 "src/parser_bison.y"
                        {
				(yyval.string) = strdup("out");
			}
#line 11156 "src/parser_bison.c"
    break;

  case 413: /* extended_prio_spec: int_num  */
#line 2738 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &integer_type,
								BYTEORDER_HOST_ENDIAN,
								sizeof(int) *
								BITS_PER_BYTE, &(yyvsp[0].val32));
				(yyval.prio_spec) = spec;
			}
#line 11170 "src/parser_bison.c"
    break;

  case 414: /* extended_prio_spec: variable_expr  */
#line 2748 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = (yyvsp[0].expr);
				(yyval.prio_spec) = spec;
			}
#line 11181 "src/parser_bison.c"
    break;

  case 415: /* extended_prio_spec: extended_prio_name  */
#line 2755 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &string_type,
								BYTEORDER_HOST_ENDIAN,
								strlen((yyvsp[0].string)) * BITS_PER_BYTE,
								(yyvsp[0].string));
				free_const((yyvsp[0].string));
				(yyval.prio_spec) = spec;
			}
#line 11196 "src/parser_bison.c"
    break;

  case 416: /* extended_prio_spec: extended_prio_name "+" "number"  */
#line 2766 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				char str[NFT_NAME_MAXLEN];
				snprintf(str, sizeof(str), "%s + %" PRIu64, (yyvsp[-2].string), (yyvsp[0].val));
				spec.expr = constant_expr_alloc(&(yyloc), &string_type,
								BYTEORDER_HOST_ENDIAN,
								strlen(str) * BITS_PER_BYTE,
								str);
				free_const((yyvsp[-2].string));
				(yyval.prio_spec) = spec;
			}
#line 11213 "src/parser_bison.c"
    break;

  case 417: /* extended_prio_spec: extended_prio_name "-" "number"  */
#line 2779 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};
				char str[NFT_NAME_MAXLEN];

				snprintf(str, sizeof(str), "%s - %" PRIu64, (yyvsp[-2].string), (yyvsp[0].val));
				spec.expr = constant_expr_alloc(&(yyloc), &string_type,
								BYTEORDER_HOST_ENDIAN,
								strlen(str) * BITS_PER_BYTE,
								str);
				free_const((yyvsp[-2].string));
				(yyval.prio_spec) = spec;
			}
#line 11230 "src/parser_bison.c"
    break;

  case 418: /* int_num: "number"  */
#line 2793 "src/parser_bison.y"
                                                        { (yyval.val32) = (yyvsp[0].val); }
#line 11236 "src/parser_bison.c"
    break;

  case 419: /* int_num: "-" "number"  */
#line 2794 "src/parser_bison.y"
                                                        { (yyval.val32) = -(yyvsp[0].val); }
#line 11242 "src/parser_bison.c"
    break;

  case 420: /* dev_spec: "device" string  */
#line 2798 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = compound_expr_alloc(&(yyloc), EXPR_LIST);
				compound_expr_add((yyval.expr), expr);

			}
#line 11257 "src/parser_bison.c"
    break;

  case 421: /* dev_spec: "device" variable_expr  */
#line 2809 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = compound_expr_alloc(&(yyloc), EXPR_LIST);
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11267 "src/parser_bison.c"
    break;

  case 422: /* dev_spec: "devices" '=' flowtable_expr  */
#line 2815 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11275 "src/parser_bison.c"
    break;

  case 423: /* dev_spec: %empty  */
#line 2818 "src/parser_bison.y"
                                                        { (yyval.expr) = NULL; }
#line 11281 "src/parser_bison.c"
    break;

  case 424: /* flags_spec: "flags" "offload"  */
#line 2822 "src/parser_bison.y"
                        {
				(yyvsp[-2].chain)->flags |= CHAIN_F_HW_OFFLOAD;
			}
#line 11289 "src/parser_bison.c"
    break;

  case 425: /* policy_spec: "policy" policy_expr close_scope_policy  */
#line 2828 "src/parser_bison.y"
                        {
				if ((yyvsp[-3].chain)->policy) {
					erec_queue(error(&(yyloc), "you cannot set chain policy twice"),
						   state->msgs);
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-3].chain)->policy		= (yyvsp[-1].expr);
				(yyvsp[-3].chain)->policy->location	= (yyloc);
				(yyvsp[-3].chain)->flags		|= CHAIN_F_BASECHAIN;
			}
#line 11305 "src/parser_bison.c"
    break;

  case 426: /* policy_expr: variable_expr  */
#line 2842 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &policy_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11314 "src/parser_bison.c"
    break;

  case 427: /* policy_expr: chain_policy  */
#line 2847 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(int) *
							 BITS_PER_BYTE, &(yyvsp[0].val32));
			}
#line 11325 "src/parser_bison.c"
    break;

  case 428: /* chain_policy: "accept"  */
#line 2855 "src/parser_bison.y"
                                                { (yyval.val32) = NF_ACCEPT; }
#line 11331 "src/parser_bison.c"
    break;

  case 429: /* chain_policy: "drop"  */
#line 2856 "src/parser_bison.y"
                                                { (yyval.val32) = NF_DROP;   }
#line 11337 "src/parser_bison.c"
    break;

  case 431: /* identifier: "last"  */
#line 2860 "src/parser_bison.y"
                                                { (yyval.string) = xstrdup("last"); }
#line 11343 "src/parser_bison.c"
    break;

  case 435: /* time_spec: "string"  */
#line 2869 "src/parser_bison.y"
                        {
				struct error_record *erec;
				uint64_t res;

				erec = time_parse(&(yylsp[0]), (yyvsp[0].string), &res);
				free_const((yyvsp[0].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				(yyval.val) = res;
			}
#line 11360 "src/parser_bison.c"
    break;

  case 437: /* time_spec_or_num_s: time_spec  */
#line 2885 "src/parser_bison.y"
                                          { (yyval.val) = (yyvsp[0].val) / 1000u; }
#line 11366 "src/parser_bison.c"
    break;

  case 438: /* family_spec: %empty  */
#line 2888 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11372 "src/parser_bison.c"
    break;

  case 440: /* family_spec_explicit: "ip" close_scope_ip  */
#line 2892 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11378 "src/parser_bison.c"
    break;

  case 441: /* family_spec_explicit: "ip6" close_scope_ip6  */
#line 2893 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 11384 "src/parser_bison.c"
    break;

  case 442: /* family_spec_explicit: "inet"  */
#line 2894 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_INET; }
#line 11390 "src/parser_bison.c"
    break;

  case 443: /* family_spec_explicit: "arp" close_scope_arp  */
#line 2895 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_ARP; }
#line 11396 "src/parser_bison.c"
    break;

  case 444: /* family_spec_explicit: "bridge"  */
#line 2896 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_BRIDGE; }
#line 11402 "src/parser_bison.c"
    break;

  case 445: /* family_spec_explicit: "netdev"  */
#line 2897 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_NETDEV; }
#line 11408 "src/parser_bison.c"
    break;

  case 446: /* table_spec: family_spec identifier  */
#line 2901 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[-1].val);
				(yyval.handle).table.location = (yylsp[0]);
				(yyval.handle).table.name	= (yyvsp[0].string);
			}
#line 11419 "src/parser_bison.c"
    break;

  case 447: /* tableid_spec: family_spec "handle" "number"  */
#line 2910 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family 		= (yyvsp[-2].val);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
				(yyval.handle).handle.location	= (yylsp[0]);
			}
#line 11430 "src/parser_bison.c"
    break;

  case 448: /* chain_spec: table_spec identifier  */
#line 2919 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).chain.name	= (yyvsp[0].string);
				(yyval.handle).chain.location = (yylsp[0]);
			}
#line 11440 "src/parser_bison.c"
    break;

  case 449: /* chainid_spec: table_spec "handle" "number"  */
#line 2927 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 11450 "src/parser_bison.c"
    break;

  case 450: /* chain_identifier: identifier  */
#line 2935 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).chain.name		= (yyvsp[0].string);
				(yyval.handle).chain.location	= (yylsp[0]);
			}
#line 11460 "src/parser_bison.c"
    break;

  case 451: /* set_spec: table_spec identifier  */
#line 2943 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 11470 "src/parser_bison.c"
    break;

  case 452: /* setid_spec: table_spec "handle" "number"  */
#line 2951 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 11480 "src/parser_bison.c"
    break;

  case 453: /* set_identifier: identifier  */
#line 2959 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 11490 "src/parser_bison.c"
    break;

  case 454: /* flowtable_spec: table_spec identifier  */
#line 2967 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-1].handle);
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 11500 "src/parser_bison.c"
    break;

  case 455: /* flowtableid_spec: table_spec "handle" "number"  */
#line 2975 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11510 "src/parser_bison.c"
    break;

  case 456: /* flowtable_identifier: identifier  */
#line 2983 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 11520 "src/parser_bison.c"
    break;

  case 457: /* obj_spec: table_spec identifier  */
#line 2991 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).obj.name	= (yyvsp[0].string);
				(yyval.handle).obj.location	= (yylsp[0]);
			}
#line 11530 "src/parser_bison.c"
    break;

  case 458: /* objid_spec: table_spec "handle" "number"  */
#line 2999 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11540 "src/parser_bison.c"
    break;

  case 459: /* obj_identifier: identifier  */
#line 3007 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).obj.name		= (yyvsp[0].string);
				(yyval.handle).obj.location		= (yylsp[0]);
			}
#line 11550 "src/parser_bison.c"
    break;

  case 460: /* handle_spec: "handle" "number"  */
#line 3015 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11560 "src/parser_bison.c"
    break;

  case 461: /* position_spec: "position" "number"  */
#line 3023 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).position.location	= (yyloc);
				(yyval.handle).position.id		= (yyvsp[0].val);
			}
#line 11570 "src/parser_bison.c"
    break;

  case 462: /* index_spec: "index" "number"  */
#line 3031 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).index.location	= (yyloc);
				(yyval.handle).index.id		= (yyvsp[0].val) + 1;
			}
#line 11580 "src/parser_bison.c"
    break;

  case 463: /* rule_position: chain_spec  */
#line 3039 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 11588 "src/parser_bison.c"
    break;

  case 464: /* rule_position: chain_spec position_spec  */
#line 3043 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11597 "src/parser_bison.c"
    break;

  case 465: /* rule_position: chain_spec handle_spec  */
#line 3048 "src/parser_bison.y"
                        {
				(yyvsp[0].handle).position.location = (yyvsp[0].handle).handle.location;
				(yyvsp[0].handle).position.id = (yyvsp[0].handle).handle.id;
				(yyvsp[0].handle).handle.id = 0;
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11609 "src/parser_bison.c"
    break;

  case 466: /* rule_position: chain_spec index_spec  */
#line 3056 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11618 "src/parser_bison.c"
    break;

  case 467: /* ruleid_spec: chain_spec handle_spec  */
#line 3063 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11627 "src/parser_bison.c"
    break;

  case 468: /* comment_spec: "comment" string  */
#line 3070 "src/parser_bison.y"
                        {
				if (strlen((yyvsp[0].string)) > NFTNL_UDATA_COMMENT_MAXLEN) {
					erec_queue(error(&(yylsp[0]), "comment too long, %d characters maximum allowed",
							 NFTNL_UDATA_COMMENT_MAXLEN),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyval.string) = (yyvsp[0].string);
			}
#line 11642 "src/parser_bison.c"
    break;

  case 469: /* ruleset_spec: %empty  */
#line 3083 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= NFPROTO_UNSPEC;
			}
#line 11651 "src/parser_bison.c"
    break;

  case 470: /* ruleset_spec: family_spec_explicit  */
#line 3088 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[0].val);
			}
#line 11660 "src/parser_bison.c"
    break;

  case 471: /* rule: rule_alloc  */
#line 3095 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = NULL;
			}
#line 11668 "src/parser_bison.c"
    break;

  case 472: /* rule: rule_alloc comment_spec  */
#line 3099 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = (yyvsp[0].string);
			}
#line 11676 "src/parser_bison.c"
    break;

  case 473: /* rule_alloc: stmt_list  */
#line 3105 "src/parser_bison.y"
                        {
				struct stmt *i;

				(yyval.rule) = rule_alloc(&(yyloc), NULL);
				list_for_each_entry(i, (yyvsp[0].list), list)
					(yyval.rule)->num_stmts++;
				list_splice_tail((yyvsp[0].list), &(yyval.rule)->stmts);
				free((yyvsp[0].list));
			}
#line 11690 "src/parser_bison.c"
    break;

  case 474: /* stmt_list: stmt  */
#line 3117 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 11700 "src/parser_bison.c"
    break;

  case 475: /* stmt_list: stmt_list stmt  */
#line 3123 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 11709 "src/parser_bison.c"
    break;

  case 476: /* stateful_stmt_list: stateful_stmt  */
#line 3130 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 11719 "src/parser_bison.c"
    break;

  case 477: /* stateful_stmt_list: stateful_stmt_list stateful_stmt  */
#line 3136 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 11728 "src/parser_bison.c"
    break;

  case 478: /* objref_stmt_counter: "counter" "name" stmt_expr close_scope_counter  */
#line 3143 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_COUNTER;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 11738 "src/parser_bison.c"
    break;

  case 479: /* objref_stmt_limit: "limit" "name" stmt_expr close_scope_limit  */
#line 3151 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_LIMIT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 11748 "src/parser_bison.c"
    break;

  case 480: /* objref_stmt_quota: "quota" "name" stmt_expr close_scope_quota  */
#line 3159 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_QUOTA;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 11758 "src/parser_bison.c"
    break;

  case 481: /* objref_stmt_synproxy: "synproxy" "name" stmt_expr close_scope_synproxy  */
#line 3167 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_SYNPROXY;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 11768 "src/parser_bison.c"
    break;

  case 482: /* objref_stmt_ct: "ct" "timeout" "set" stmt_expr close_scope_ct  */
#line 3175 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_TIMEOUT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);

			}
#line 11779 "src/parser_bison.c"
    break;

  case 483: /* objref_stmt_ct: "ct" "expectation" "set" stmt_expr close_scope_ct  */
#line 3182 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_EXPECT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 11789 "src/parser_bison.c"
    break;

  case 517: /* xt_stmt: "xt" "string" string  */
#line 3229 "src/parser_bison.y"
                        {
				(yyval.stmt) = NULL;
				free_const((yyvsp[-1].string));
				free_const((yyvsp[0].string));
				erec_queue(error(&(yyloc), "unsupported xtables compat expression, use iptables-nft with this ruleset"),
					   state->msgs);
				YYERROR;
			}
#line 11802 "src/parser_bison.c"
    break;

  case 518: /* chain_stmt_type: "jump"  */
#line 3239 "src/parser_bison.y"
                                        { (yyval.val) = NFT_JUMP; }
#line 11808 "src/parser_bison.c"
    break;

  case 519: /* chain_stmt_type: "goto"  */
#line 3240 "src/parser_bison.y"
                                        { (yyval.val) = NFT_GOTO; }
#line 11814 "src/parser_bison.c"
    break;

  case 520: /* chain_stmt: chain_stmt_type chain_block_alloc '{' subchain_block '}'  */
#line 3244 "src/parser_bison.y"
                        {
				(yyvsp[-3].chain)->location = (yylsp[-3]);
				close_scope(state);
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				(yyval.stmt) = chain_stmt_alloc(&(yyloc), (yyvsp[-1].chain), (yyvsp[-4].val));
			}
#line 11825 "src/parser_bison.c"
    break;

  case 521: /* verdict_stmt: verdict_expr  */
#line 3253 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 11833 "src/parser_bison.c"
    break;

  case 522: /* verdict_stmt: verdict_map_stmt  */
#line 3257 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 11841 "src/parser_bison.c"
    break;

  case 523: /* verdict_map_stmt: concat_expr "vmap" verdict_map_expr  */
#line 3263 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 11849 "src/parser_bison.c"
    break;

  case 524: /* verdict_map_expr: '{' verdict_map_list_expr '}'  */
#line 3269 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 11858 "src/parser_bison.c"
    break;

  case 526: /* verdict_map_list_expr: verdict_map_list_member_expr  */
#line 3277 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11867 "src/parser_bison.c"
    break;

  case 527: /* verdict_map_list_expr: verdict_map_list_expr "comma" verdict_map_list_member_expr  */
#line 3282 "src/parser_bison.y"
                        {
				compound_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 11876 "src/parser_bison.c"
    break;

  case 529: /* verdict_map_list_member_expr: opt_newline set_elem_expr "colon" verdict_expr opt_newline  */
#line 3290 "src/parser_bison.y"
                        {
				(yyval.expr) = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr), (yyvsp[-1].expr));
			}
#line 11884 "src/parser_bison.c"
    break;

  case 530: /* ct_limit_stmt_alloc: "ct" "count"  */
#line 3296 "src/parser_bison.y"
                        {
				(yyval.stmt) = connlimit_stmt_alloc(&(yyloc));
			}
#line 11892 "src/parser_bison.c"
    break;

  case 532: /* ct_limit_args: "number"  */
#line 3305 "src/parser_bison.y"
                        {
				assert((yyvsp[-1].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-1].stmt)->connlimit.count	= (yyvsp[0].val);
			}
#line 11902 "src/parser_bison.c"
    break;

  case 533: /* ct_limit_args: "over" "number"  */
#line 3311 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-2].stmt)->connlimit.count = (yyvsp[0].val);
				(yyvsp[-2].stmt)->connlimit.flags = NFT_CONNLIMIT_F_INV;
			}
#line 11913 "src/parser_bison.c"
    break;

  case 536: /* counter_stmt_alloc: "counter"  */
#line 3323 "src/parser_bison.y"
                        {
				(yyval.stmt) = counter_stmt_alloc(&(yyloc));
			}
#line 11921 "src/parser_bison.c"
    break;

  case 537: /* counter_args: counter_arg  */
#line 3329 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 11929 "src/parser_bison.c"
    break;

  case 539: /* counter_arg: "packets" "number"  */
#line 3336 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.packets = (yyvsp[0].val);
			}
#line 11938 "src/parser_bison.c"
    break;

  case 540: /* counter_arg: "bytes" "number"  */
#line 3341 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.bytes	 = (yyvsp[0].val);
			}
#line 11947 "src/parser_bison.c"
    break;

  case 541: /* last_stmt_alloc: "last"  */
#line 3348 "src/parser_bison.y"
                        {
				(yyval.stmt) = last_stmt_alloc(&(yyloc));
			}
#line 11955 "src/parser_bison.c"
    break;

  case 545: /* last_args: "used" time_spec  */
#line 3359 "src/parser_bison.y"
                        {
				struct last_stmt *last;

				assert((yyvsp[-2].stmt)->type == STMT_LAST);
				last = &(yyvsp[-2].stmt)->last;
				last->used = (yyvsp[0].val);
				last->set = true;
			}
#line 11968 "src/parser_bison.c"
    break;

  case 548: /* log_stmt_alloc: "log"  */
#line 3374 "src/parser_bison.y"
                        {
				(yyval.stmt) = log_stmt_alloc(&(yyloc));
			}
#line 11976 "src/parser_bison.c"
    break;

  case 549: /* log_args: log_arg  */
#line 3380 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 11984 "src/parser_bison.c"
    break;

  case 551: /* log_arg: "prefix" string  */
#line 3387 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);
				struct error_record *erec;
				const char *prefix;

				prefix = str_preprocess(state, &(yylsp[0]), scope, (yyvsp[0].string), &erec);
				if (!prefix) {
					erec_queue(erec, state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}

				free_const((yyvsp[0].string));
				(yyvsp[-2].stmt)->log.prefix = prefix;
				(yyvsp[-2].stmt)->log.flags |= STMT_LOG_PREFIX;
			}
#line 12005 "src/parser_bison.c"
    break;

  case 552: /* log_arg: "group" "number"  */
#line 3404 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.group	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_GROUP;
			}
#line 12014 "src/parser_bison.c"
    break;

  case 553: /* log_arg: "snaplen" "number"  */
#line 3409 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.snaplen	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_SNAPLEN;
			}
#line 12023 "src/parser_bison.c"
    break;

  case 554: /* log_arg: "queue-threshold" "number"  */
#line 3414 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.qthreshold = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_QTHRESHOLD;
			}
#line 12032 "src/parser_bison.c"
    break;

  case 555: /* log_arg: "level" level_type  */
#line 3419 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.level	= (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_LEVEL;
			}
#line 12041 "src/parser_bison.c"
    break;

  case 556: /* log_arg: "flags" log_flags  */
#line 3424 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.logflags	|= (yyvsp[0].val);
			}
#line 12049 "src/parser_bison.c"
    break;

  case 557: /* level_type: string  */
#line 3430 "src/parser_bison.y"
                        {
				if (!strcmp("emerg", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_EMERG;
				else if (!strcmp("alert", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_ALERT;
				else if (!strcmp("crit", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_CRIT;
				else if (!strcmp("err", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_ERR;
				else if (!strcmp("warn", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_WARNING;
				else if (!strcmp("notice", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_NOTICE;
				else if (!strcmp("info", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_INFO;
				else if (!strcmp("debug", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_DEBUG;
				else if (!strcmp("audit", (yyvsp[0].string)))
					(yyval.val) = NFT_LOGLEVEL_AUDIT;
				else {
					erec_queue(error(&(yylsp[0]), "invalid log level"),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				free_const((yyvsp[0].string));
			}
#line 12081 "src/parser_bison.c"
    break;

  case 558: /* log_flags: "tcp" log_flags_tcp close_scope_tcp  */
#line 3460 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-1].val);
			}
#line 12089 "src/parser_bison.c"
    break;

  case 559: /* log_flags: "ip" "options" close_scope_ip  */
#line 3464 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_IPOPT;
			}
#line 12097 "src/parser_bison.c"
    break;

  case 560: /* log_flags: "skuid"  */
#line 3468 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_UID;
			}
#line 12105 "src/parser_bison.c"
    break;

  case 561: /* log_flags: "ether" close_scope_eth  */
#line 3472 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MACDECODE;
			}
#line 12113 "src/parser_bison.c"
    break;

  case 562: /* log_flags: "all"  */
#line 3476 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MASK;
			}
#line 12121 "src/parser_bison.c"
    break;

  case 563: /* log_flags_tcp: log_flags_tcp "comma" log_flag_tcp  */
#line 3482 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 12129 "src/parser_bison.c"
    break;

  case 565: /* log_flag_tcp: "seq"  */
#line 3489 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPSEQ;
			}
#line 12137 "src/parser_bison.c"
    break;

  case 566: /* log_flag_tcp: "options"  */
#line 3493 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPOPT;
			}
#line 12145 "src/parser_bison.c"
    break;

  case 567: /* limit_stmt_alloc: "limit" "rate"  */
#line 3499 "src/parser_bison.y"
                        {
				(yyval.stmt) = limit_stmt_alloc(&(yyloc));
			}
#line 12153 "src/parser_bison.c"
    break;

  case 569: /* limit_args: limit_mode limit_rate_pkts limit_burst_pkts  */
#line 3508 "src/parser_bison.y"
                        {
				struct limit_stmt *limit;

				assert((yyvsp[-3].stmt)->type == STMT_LIMIT);

				if ((yyvsp[0].val) == 0) {
					erec_queue(error(&(yylsp[0]), "packet limit burst must be > 0"),
						   state->msgs);
					YYERROR;
				}
				limit = &(yyvsp[-3].stmt)->limit;
				limit->rate = (yyvsp[-1].limit_rate).rate;
				limit->unit = (yyvsp[-1].limit_rate).unit;
				limit->burst = (yyvsp[0].val);
				limit->type = NFT_LIMIT_PKTS;
				limit->flags = (yyvsp[-2].val);
			}
#line 12175 "src/parser_bison.c"
    break;

  case 570: /* limit_args: limit_mode limit_rate_bytes limit_burst_bytes  */
#line 3526 "src/parser_bison.y"
                        {
				struct limit_stmt *limit;

				assert((yyvsp[-3].stmt)->type == STMT_LIMIT);

				limit = &(yyvsp[-3].stmt)->limit;
				limit->rate = (yyvsp[-1].limit_rate).rate;
				limit->unit = (yyvsp[-1].limit_rate).unit;
				limit->burst = (yyvsp[0].val);
				limit->type = NFT_LIMIT_PKT_BYTES;
				limit->flags = (yyvsp[-2].val);
			}
#line 12192 "src/parser_bison.c"
    break;

  case 571: /* quota_mode: "over"  */
#line 3540 "src/parser_bison.y"
                                                { (yyval.val) = NFT_QUOTA_F_INV; }
#line 12198 "src/parser_bison.c"
    break;

  case 572: /* quota_mode: "until"  */
#line 3541 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12204 "src/parser_bison.c"
    break;

  case 573: /* quota_mode: %empty  */
#line 3542 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12210 "src/parser_bison.c"
    break;

  case 574: /* quota_unit: "bytes"  */
#line 3545 "src/parser_bison.y"
                                                { (yyval.string) = xstrdup("bytes"); }
#line 12216 "src/parser_bison.c"
    break;

  case 575: /* quota_unit: "string"  */
#line 3546 "src/parser_bison.y"
                                                { (yyval.string) = (yyvsp[0].string); }
#line 12222 "src/parser_bison.c"
    break;

  case 576: /* quota_used: %empty  */
#line 3549 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12228 "src/parser_bison.c"
    break;

  case 577: /* quota_used: "used" "number" quota_unit  */
#line 3551 "src/parser_bison.y"
                        {
				struct error_record *erec;
				uint64_t rate;

				erec = data_unit_parse(&(yyloc), (yyvsp[0].string), &rate);
				free_const((yyvsp[0].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				(yyval.val) = (yyvsp[-1].val) * rate;
			}
#line 12245 "src/parser_bison.c"
    break;

  case 578: /* quota_stmt_alloc: "quota"  */
#line 3566 "src/parser_bison.y"
                        {
				(yyval.stmt) = quota_stmt_alloc(&(yyloc));
			}
#line 12253 "src/parser_bison.c"
    break;

  case 580: /* quota_args: quota_mode "number" quota_unit quota_used  */
#line 3575 "src/parser_bison.y"
                        {
				struct error_record *erec;
				struct quota_stmt *quota;
				uint64_t rate;

				assert((yyvsp[-4].stmt)->type == STMT_QUOTA);

				erec = data_unit_parse(&(yyloc), (yyvsp[-1].string), &rate);
				free_const((yyvsp[-1].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				quota = &(yyvsp[-4].stmt)->quota;
				quota->bytes = (yyvsp[-2].val) * rate;
				quota->used = (yyvsp[0].val);
				quota->flags = (yyvsp[-3].val);
			}
#line 12276 "src/parser_bison.c"
    break;

  case 581: /* limit_mode: "over"  */
#line 3595 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_LIMIT_F_INV; }
#line 12282 "src/parser_bison.c"
    break;

  case 582: /* limit_mode: "until"  */
#line 3596 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12288 "src/parser_bison.c"
    break;

  case 583: /* limit_mode: %empty  */
#line 3597 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12294 "src/parser_bison.c"
    break;

  case 584: /* limit_burst_pkts: %empty  */
#line 3600 "src/parser_bison.y"
                                                                { (yyval.val) = 5; }
#line 12300 "src/parser_bison.c"
    break;

  case 585: /* limit_burst_pkts: "burst" "number" "packets"  */
#line 3601 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[-1].val); }
#line 12306 "src/parser_bison.c"
    break;

  case 586: /* limit_rate_pkts: "number" "/" time_unit  */
#line 3605 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12315 "src/parser_bison.c"
    break;

  case 587: /* limit_burst_bytes: %empty  */
#line 3611 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12321 "src/parser_bison.c"
    break;

  case 588: /* limit_burst_bytes: "burst" limit_bytes  */
#line 3612 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[0].val); }
#line 12327 "src/parser_bison.c"
    break;

  case 589: /* limit_rate_bytes: "number" "string"  */
#line 3616 "src/parser_bison.y"
                        {
				struct error_record *erec;
				uint64_t rate, unit;

				erec = rate_parse(&(yyloc), (yyvsp[0].string), &rate, &unit);
				free_const((yyvsp[0].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				(yyval.limit_rate).rate = rate * (yyvsp[-1].val);
				(yyval.limit_rate).unit = unit;
			}
#line 12345 "src/parser_bison.c"
    break;

  case 590: /* limit_rate_bytes: limit_bytes "/" time_unit  */
#line 3630 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12354 "src/parser_bison.c"
    break;

  case 591: /* limit_bytes: "number" "bytes"  */
#line 3636 "src/parser_bison.y"
                                                        { (yyval.val) = (yyvsp[-1].val); }
#line 12360 "src/parser_bison.c"
    break;

  case 592: /* limit_bytes: "number" "string"  */
#line 3638 "src/parser_bison.y"
                        {
				struct error_record *erec;
				uint64_t rate;

				erec = data_unit_parse(&(yyloc), (yyvsp[0].string), &rate);
				free_const((yyvsp[0].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				(yyval.val) = (yyvsp[-1].val) * rate;
			}
#line 12377 "src/parser_bison.c"
    break;

  case 593: /* time_unit: "second"  */
#line 3652 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL; }
#line 12383 "src/parser_bison.c"
    break;

  case 594: /* time_unit: "minute"  */
#line 3653 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60; }
#line 12389 "src/parser_bison.c"
    break;

  case 595: /* time_unit: "hour"  */
#line 3654 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60; }
#line 12395 "src/parser_bison.c"
    break;

  case 596: /* time_unit: "day"  */
#line 3655 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24; }
#line 12401 "src/parser_bison.c"
    break;

  case 597: /* time_unit: "week"  */
#line 3656 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24 * 7; }
#line 12407 "src/parser_bison.c"
    break;

  case 599: /* reject_stmt_alloc: "reject"  */
#line 3663 "src/parser_bison.y"
                        {
				(yyval.stmt) = reject_stmt_alloc(&(yyloc));
			}
#line 12415 "src/parser_bison.c"
    break;

  case 600: /* reject_with_expr: "string"  */
#line 3669 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state), (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 12425 "src/parser_bison.c"
    break;

  case 601: /* reject_with_expr: integer_expr  */
#line 3674 "src/parser_bison.y"
                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12431 "src/parser_bison.c"
    break;

  case 602: /* reject_opts: %empty  */
#line 3678 "src/parser_bison.y"
                        {
				(yyvsp[0].stmt)->reject.type = -1;
				(yyvsp[0].stmt)->reject.icmp_code = -1;
			}
#line 12440 "src/parser_bison.c"
    break;

  case 603: /* reject_opts: "with" "icmp" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3683 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 12451 "src/parser_bison.c"
    break;

  case 604: /* reject_opts: "with" "icmp" reject_with_expr  */
#line 3690 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 12462 "src/parser_bison.c"
    break;

  case 605: /* reject_opts: "with" "icmpv6" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3697 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 12473 "src/parser_bison.c"
    break;

  case 606: /* reject_opts: "with" "icmpv6" reject_with_expr  */
#line 3704 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 12484 "src/parser_bison.c"
    break;

  case 607: /* reject_opts: "with" "icmpx" "type" reject_with_expr close_scope_type  */
#line 3711 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-5].stmt)->reject.expr = (yyvsp[-1].expr);
				datatype_set((yyvsp[-5].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 12494 "src/parser_bison.c"
    break;

  case 608: /* reject_opts: "with" "icmpx" reject_with_expr  */
#line 3717 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 12504 "src/parser_bison.c"
    break;

  case 609: /* reject_opts: "with" "tcp" close_scope_tcp "reset" close_scope_reset  */
#line 3723 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_TCP_RST;
			}
#line 12512 "src/parser_bison.c"
    break;

  case 611: /* nat_stmt_alloc: "snat"  */
#line 3731 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_SNAT); }
#line 12518 "src/parser_bison.c"
    break;

  case 612: /* nat_stmt_alloc: "dnat"  */
#line 3732 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_DNAT); }
#line 12524 "src/parser_bison.c"
    break;

  case 613: /* tproxy_stmt: "tproxy" "to" stmt_expr  */
#line 3736 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 12534 "src/parser_bison.c"
    break;

  case 614: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr  */
#line 3742 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-2].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 12544 "src/parser_bison.c"
    break;

  case 615: /* tproxy_stmt: "tproxy" "to" "colon" stmt_expr  */
#line 3748 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12554 "src/parser_bison.c"
    break;

  case 616: /* tproxy_stmt: "tproxy" "to" stmt_expr "colon" stmt_expr  */
#line 3754 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12565 "src/parser_bison.c"
    break;

  case 617: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 3761 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-4].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12576 "src/parser_bison.c"
    break;

  case 618: /* tproxy_stmt: "tproxy" nf_key_proto "to" "colon" stmt_expr  */
#line 3768 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-3].val);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12586 "src/parser_bison.c"
    break;

  case 621: /* synproxy_stmt_alloc: "synproxy"  */
#line 3780 "src/parser_bison.y"
                        {
				(yyval.stmt) = synproxy_stmt_alloc(&(yyloc));
			}
#line 12594 "src/parser_bison.c"
    break;

  case 622: /* synproxy_args: synproxy_arg  */
#line 3786 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12602 "src/parser_bison.c"
    break;

  case 624: /* synproxy_arg: "mss" "number"  */
#line 3793 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->synproxy.mss = (yyvsp[0].val);
				(yyvsp[-2].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_MSS;
			}
#line 12611 "src/parser_bison.c"
    break;

  case 625: /* synproxy_arg: "wscale" "number"  */
#line 3798 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->synproxy.wscale = (yyvsp[0].val);
				(yyvsp[-2].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_WSCALE;
			}
#line 12620 "src/parser_bison.c"
    break;

  case 626: /* synproxy_arg: "timestamp"  */
#line 3803 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 12628 "src/parser_bison.c"
    break;

  case 627: /* synproxy_arg: "sack-permitted"  */
#line 3807 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 12636 "src/parser_bison.c"
    break;

  case 628: /* synproxy_config: "mss" "number" "wscale" "number" synproxy_ts synproxy_sack  */
#line 3813 "src/parser_bison.y"
                        {
				struct synproxy *synproxy;
				uint32_t flags = 0;

				synproxy = &(yyvsp[-6].obj)->synproxy;
				synproxy->mss = (yyvsp[-4].val);
				flags |= NF_SYNPROXY_OPT_MSS;
				synproxy->wscale = (yyvsp[-2].val);
				flags |= NF_SYNPROXY_OPT_WSCALE;
				if ((yyvsp[-1].val))
					flags |= (yyvsp[-1].val);
				if ((yyvsp[0].val))
					flags |= (yyvsp[0].val);
				synproxy->flags = flags;
			}
#line 12656 "src/parser_bison.c"
    break;

  case 629: /* synproxy_config: "mss" "number" stmt_separator "wscale" "number" stmt_separator synproxy_ts synproxy_sack  */
#line 3829 "src/parser_bison.y"
                        {
				struct synproxy *synproxy;
				uint32_t flags = 0;

				synproxy = &(yyvsp[-8].obj)->synproxy;
				synproxy->mss = (yyvsp[-6].val);
				flags |= NF_SYNPROXY_OPT_MSS;
				synproxy->wscale = (yyvsp[-3].val);
				flags |= NF_SYNPROXY_OPT_WSCALE;
				if ((yyvsp[-1].val))
					flags |= (yyvsp[-1].val);
				if ((yyvsp[0].val))
					flags |= (yyvsp[0].val);
				synproxy->flags = flags;
			}
#line 12676 "src/parser_bison.c"
    break;

  case 630: /* synproxy_obj: %empty  */
#line 3847 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SYNPROXY;
			}
#line 12685 "src/parser_bison.c"
    break;

  case 631: /* synproxy_ts: %empty  */
#line 3853 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12691 "src/parser_bison.c"
    break;

  case 632: /* synproxy_ts: "timestamp"  */
#line 3855 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 12699 "src/parser_bison.c"
    break;

  case 633: /* synproxy_sack: %empty  */
#line 3860 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12705 "src/parser_bison.c"
    break;

  case 634: /* synproxy_sack: "sack-permitted"  */
#line 3862 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 12713 "src/parser_bison.c"
    break;

  case 635: /* primary_stmt_expr: symbol_expr  */
#line 3867 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12719 "src/parser_bison.c"
    break;

  case 636: /* primary_stmt_expr: integer_expr  */
#line 3868 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12725 "src/parser_bison.c"
    break;

  case 637: /* primary_stmt_expr: boolean_expr  */
#line 3869 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12731 "src/parser_bison.c"
    break;

  case 638: /* primary_stmt_expr: meta_expr  */
#line 3870 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12737 "src/parser_bison.c"
    break;

  case 639: /* primary_stmt_expr: rt_expr  */
#line 3871 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12743 "src/parser_bison.c"
    break;

  case 640: /* primary_stmt_expr: ct_expr  */
#line 3872 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12749 "src/parser_bison.c"
    break;

  case 641: /* primary_stmt_expr: numgen_expr  */
#line 3873 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12755 "src/parser_bison.c"
    break;

  case 642: /* primary_stmt_expr: hash_expr  */
#line 3874 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12761 "src/parser_bison.c"
    break;

  case 643: /* primary_stmt_expr: payload_expr  */
#line 3875 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12767 "src/parser_bison.c"
    break;

  case 644: /* primary_stmt_expr: keyword_expr  */
#line 3876 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12773 "src/parser_bison.c"
    break;

  case 645: /* primary_stmt_expr: socket_expr  */
#line 3877 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12779 "src/parser_bison.c"
    break;

  case 646: /* primary_stmt_expr: fib_expr  */
#line 3878 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12785 "src/parser_bison.c"
    break;

  case 647: /* primary_stmt_expr: osf_expr  */
#line 3879 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12791 "src/parser_bison.c"
    break;

  case 648: /* primary_stmt_expr: '(' basic_stmt_expr ')'  */
#line 3880 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 12797 "src/parser_bison.c"
    break;

  case 650: /* shift_stmt_expr: shift_stmt_expr "<<" primary_stmt_expr  */
#line 3885 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12805 "src/parser_bison.c"
    break;

  case 651: /* shift_stmt_expr: shift_stmt_expr ">>" primary_stmt_expr  */
#line 3889 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12813 "src/parser_bison.c"
    break;

  case 653: /* and_stmt_expr: and_stmt_expr "&" shift_stmt_expr  */
#line 3896 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12821 "src/parser_bison.c"
    break;

  case 655: /* exclusive_or_stmt_expr: exclusive_or_stmt_expr "^" and_stmt_expr  */
#line 3903 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12829 "src/parser_bison.c"
    break;

  case 657: /* inclusive_or_stmt_expr: inclusive_or_stmt_expr '|' exclusive_or_stmt_expr  */
#line 3910 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12837 "src/parser_bison.c"
    break;

  case 660: /* concat_stmt_expr: concat_stmt_expr "." primary_stmt_expr  */
#line 3920 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 12850 "src/parser_bison.c"
    break;

  case 663: /* map_stmt_expr: concat_stmt_expr "map" map_stmt_expr_set  */
#line 3935 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12858 "src/parser_bison.c"
    break;

  case 664: /* map_stmt_expr: concat_stmt_expr  */
#line 3938 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 12864 "src/parser_bison.c"
    break;

  case 665: /* prefix_stmt_expr: basic_stmt_expr "/" "number"  */
#line 3942 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 12872 "src/parser_bison.c"
    break;

  case 666: /* range_stmt_expr: basic_stmt_expr "-" basic_stmt_expr  */
#line 3948 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12880 "src/parser_bison.c"
    break;

  case 672: /* nat_stmt_args: stmt_expr  */
#line 3963 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 12888 "src/parser_bison.c"
    break;

  case 673: /* nat_stmt_args: "to" stmt_expr  */
#line 3967 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 12896 "src/parser_bison.c"
    break;

  case 674: /* nat_stmt_args: nf_key_proto "to" stmt_expr  */
#line 3971 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.family = (yyvsp[-2].val);
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 12905 "src/parser_bison.c"
    break;

  case 675: /* nat_stmt_args: stmt_expr "colon" stmt_expr  */
#line 3976 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 12914 "src/parser_bison.c"
    break;

  case 676: /* nat_stmt_args: "to" stmt_expr "colon" stmt_expr  */
#line 3981 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 12923 "src/parser_bison.c"
    break;

  case 677: /* nat_stmt_args: nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 3986 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->nat.family = (yyvsp[-4].val);
				(yyvsp[-5].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-5].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 12933 "src/parser_bison.c"
    break;

  case 678: /* nat_stmt_args: "colon" stmt_expr  */
#line 3992 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 12941 "src/parser_bison.c"
    break;

  case 679: /* nat_stmt_args: "to" "colon" stmt_expr  */
#line 3996 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 12949 "src/parser_bison.c"
    break;

  case 680: /* nat_stmt_args: nat_stmt_args nf_nat_flags  */
#line 4000 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 12957 "src/parser_bison.c"
    break;

  case 681: /* nat_stmt_args: nf_key_proto "addr" "." "port" "to" stmt_expr  */
#line 4004 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->nat.family = (yyvsp[-5].val);
				(yyvsp[-6].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-6].stmt)->nat.type_flags = STMT_NAT_F_CONCAT;
			}
#line 12967 "src/parser_bison.c"
    break;

  case 682: /* nat_stmt_args: nf_key_proto "interval" "to" stmt_expr  */
#line 4010 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 12976 "src/parser_bison.c"
    break;

  case 683: /* nat_stmt_args: "interval" "to" stmt_expr  */
#line 4015 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 12984 "src/parser_bison.c"
    break;

  case 684: /* nat_stmt_args: nf_key_proto "prefix" "to" stmt_expr  */
#line 4019 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-4].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-4].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 12996 "src/parser_bison.c"
    break;

  case 685: /* nat_stmt_args: "prefix" "to" stmt_expr  */
#line 4027 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-3].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-3].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 13007 "src/parser_bison.c"
    break;

  case 688: /* masq_stmt_alloc: "masquerade"  */
#line 4039 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_MASQ); }
#line 13013 "src/parser_bison.c"
    break;

  case 689: /* masq_stmt_args: "to" "colon" stmt_expr  */
#line 4043 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13021 "src/parser_bison.c"
    break;

  case 690: /* masq_stmt_args: "to" "colon" stmt_expr nf_nat_flags  */
#line 4047 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13030 "src/parser_bison.c"
    break;

  case 691: /* masq_stmt_args: nf_nat_flags  */
#line 4052 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13038 "src/parser_bison.c"
    break;

  case 694: /* redir_stmt_alloc: "redirect"  */
#line 4061 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_REDIR); }
#line 13044 "src/parser_bison.c"
    break;

  case 695: /* redir_stmt_arg: "to" stmt_expr  */
#line 4065 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13052 "src/parser_bison.c"
    break;

  case 696: /* redir_stmt_arg: "to" "colon" stmt_expr  */
#line 4069 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13060 "src/parser_bison.c"
    break;

  case 697: /* redir_stmt_arg: nf_nat_flags  */
#line 4073 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13068 "src/parser_bison.c"
    break;

  case 698: /* redir_stmt_arg: "to" stmt_expr nf_nat_flags  */
#line 4077 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-3].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13077 "src/parser_bison.c"
    break;

  case 699: /* redir_stmt_arg: "to" "colon" stmt_expr nf_nat_flags  */
#line 4082 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13086 "src/parser_bison.c"
    break;

  case 700: /* dup_stmt: "dup" "to" stmt_expr  */
#line 4089 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[0].expr);
			}
#line 13095 "src/parser_bison.c"
    break;

  case 701: /* dup_stmt: "dup" "to" stmt_expr "device" stmt_expr  */
#line 4094 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[-2].expr);
				(yyval.stmt)->dup.dev = (yyvsp[0].expr);
			}
#line 13105 "src/parser_bison.c"
    break;

  case 702: /* fwd_stmt: "fwd" "to" stmt_expr  */
#line 4102 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13114 "src/parser_bison.c"
    break;

  case 703: /* fwd_stmt: "fwd" nf_key_proto "to" stmt_expr "device" stmt_expr  */
#line 4107 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.family = (yyvsp[-4].val);
				(yyval.stmt)->fwd.addr = (yyvsp[-2].expr);
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13125 "src/parser_bison.c"
    break;

  case 705: /* nf_nat_flags: nf_nat_flags "comma" nf_nat_flag  */
#line 4117 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13133 "src/parser_bison.c"
    break;

  case 706: /* nf_nat_flag: "random"  */
#line 4122 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM; }
#line 13139 "src/parser_bison.c"
    break;

  case 707: /* nf_nat_flag: "fully-random"  */
#line 4123 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM_FULLY; }
#line 13145 "src/parser_bison.c"
    break;

  case 708: /* nf_nat_flag: "persistent"  */
#line 4124 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PERSISTENT; }
#line 13151 "src/parser_bison.c"
    break;

  case 710: /* queue_stmt: "queue" "to" queue_stmt_expr close_scope_queue  */
#line 4129 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), 0);
			}
#line 13159 "src/parser_bison.c"
    break;

  case 711: /* queue_stmt: "queue" "flags" queue_stmt_flags "to" queue_stmt_expr close_scope_queue  */
#line 4133 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13167 "src/parser_bison.c"
    break;

  case 712: /* queue_stmt: "queue" "flags" queue_stmt_flags "num" queue_stmt_expr_simple close_scope_queue  */
#line 4137 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13175 "src/parser_bison.c"
    break;

  case 715: /* queue_stmt_alloc: "queue"  */
#line 4147 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), NULL, 0);
			}
#line 13183 "src/parser_bison.c"
    break;

  case 716: /* queue_stmt_args: queue_stmt_arg  */
#line 4153 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 13191 "src/parser_bison.c"
    break;

  case 718: /* queue_stmt_arg: "num" queue_stmt_expr_simple  */
#line 4160 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->queue.queue = (yyvsp[0].expr);
				(yyvsp[-2].stmt)->queue.queue->location = (yyloc);
			}
#line 13200 "src/parser_bison.c"
    break;

  case 719: /* queue_stmt_arg: queue_stmt_flags  */
#line 4165 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->queue.flags |= (yyvsp[0].val);
			}
#line 13208 "src/parser_bison.c"
    break;

  case 724: /* queue_stmt_expr_simple: queue_expr "-" queue_expr  */
#line 4177 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13216 "src/parser_bison.c"
    break;

  case 730: /* queue_stmt_flags: queue_stmt_flags "comma" queue_stmt_flag  */
#line 4190 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13224 "src/parser_bison.c"
    break;

  case 731: /* queue_stmt_flag: "bypass"  */
#line 4195 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_BYPASS; }
#line 13230 "src/parser_bison.c"
    break;

  case 732: /* queue_stmt_flag: "fanout"  */
#line 4196 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_CPU_FANOUT; }
#line 13236 "src/parser_bison.c"
    break;

  case 735: /* set_elem_expr_stmt_alloc: concat_expr  */
#line 4204 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 13244 "src/parser_bison.c"
    break;

  case 736: /* set_stmt: "set" set_stmt_op set_elem_expr_stmt set_ref_expr  */
#line 4210 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-2].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[0].expr);
			}
#line 13255 "src/parser_bison.c"
    break;

  case 737: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt '}'  */
#line 4217 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-4].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[-3].expr);
			}
#line 13266 "src/parser_bison.c"
    break;

  case 738: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list '}'  */
#line 4224 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-5].val);
				(yyval.stmt)->set.key = (yyvsp[-2].expr);
				(yyval.stmt)->set.set = (yyvsp[-4].expr);
				list_splice_tail((yyvsp[-1].list), &(yyval.stmt)->set.stmt_list);
				free((yyvsp[-1].list));
			}
#line 13279 "src/parser_bison.c"
    break;

  case 739: /* set_stmt_op: "add"  */
#line 4234 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_ADD; }
#line 13285 "src/parser_bison.c"
    break;

  case 740: /* set_stmt_op: "update"  */
#line 4235 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_UPDATE; }
#line 13291 "src/parser_bison.c"
    break;

  case 741: /* set_stmt_op: "delete"  */
#line 4236 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_DELETE; }
#line 13297 "src/parser_bison.c"
    break;

  case 742: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt "colon" set_elem_expr_stmt '}'  */
#line 4240 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-6].val);
				(yyval.stmt)->map.key = (yyvsp[-3].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-5].expr);
			}
#line 13309 "src/parser_bison.c"
    break;

  case 743: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list "colon" set_elem_expr_stmt '}'  */
#line 4248 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-7].val);
				(yyval.stmt)->map.key = (yyvsp[-4].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-6].expr);
				list_splice_tail((yyvsp[-3].list), &(yyval.stmt)->map.stmt_list);
				free((yyvsp[-3].list));
			}
#line 13323 "src/parser_bison.c"
    break;

  case 744: /* meter_stmt: "meter" identifier '{' meter_key_expr stmt '}'  */
#line 4260 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-4].string);
				(yyval.stmt)->meter.size = 0;
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13336 "src/parser_bison.c"
    break;

  case 745: /* meter_stmt: "meter" identifier "size" "number" '{' meter_key_expr stmt '}'  */
#line 4269 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-6].string);
				(yyval.stmt)->meter.size = (yyvsp[-4].val);
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13349 "src/parser_bison.c"
    break;

  case 746: /* match_stmt: relational_expr  */
#line 4280 "src/parser_bison.y"
                        {
				(yyval.stmt) = expr_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 13357 "src/parser_bison.c"
    break;

  case 747: /* variable_expr: '$' identifier  */
#line 4286 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);
				struct symbol *sym;

				sym = symbol_get(scope, (yyvsp[0].string));
				if (!sym) {
					sym = symbol_lookup_fuzzy(scope, (yyvsp[0].string));
					if (sym) {
						erec_queue(error(&(yylsp[0]), "unknown identifier '%s'; "
								      "did you mean identifier '%s’?",
								      (yyvsp[0].string), sym->identifier),
							   state->msgs);
					} else {
						erec_queue(error(&(yylsp[0]), "unknown identifier '%s'", (yyvsp[0].string)),
							   state->msgs);
					}
					free_const((yyvsp[0].string));
					YYERROR;
				}

				(yyval.expr) = variable_expr_alloc(&(yyloc), scope, sym);
				free_const((yyvsp[0].string));
			}
#line 13385 "src/parser_bison.c"
    break;

  case 749: /* symbol_expr: string  */
#line 4313 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 13396 "src/parser_bison.c"
    break;

  case 752: /* set_ref_symbol_expr: "@" identifier close_scope_at  */
#line 4326 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_SET,
						       current_scope(state),
						       (yyvsp[-1].string));
				free_const((yyvsp[-1].string));
			}
#line 13407 "src/parser_bison.c"
    break;

  case 753: /* integer_expr: "number"  */
#line 4335 "src/parser_bison.y"
                        {
				char str[64];

				snprintf(str, sizeof(str), "%" PRIu64, (yyvsp[0].val));
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       str);
			}
#line 13420 "src/parser_bison.c"
    break;

  case 754: /* selector_expr: payload_expr  */
#line 4345 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13426 "src/parser_bison.c"
    break;

  case 755: /* selector_expr: exthdr_expr  */
#line 4346 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13432 "src/parser_bison.c"
    break;

  case 756: /* selector_expr: exthdr_exists_expr  */
#line 4347 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13438 "src/parser_bison.c"
    break;

  case 757: /* selector_expr: meta_expr  */
#line 4348 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13444 "src/parser_bison.c"
    break;

  case 758: /* selector_expr: socket_expr  */
#line 4349 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13450 "src/parser_bison.c"
    break;

  case 759: /* selector_expr: rt_expr  */
#line 4350 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13456 "src/parser_bison.c"
    break;

  case 760: /* selector_expr: ct_expr  */
#line 4351 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13462 "src/parser_bison.c"
    break;

  case 761: /* selector_expr: numgen_expr  */
#line 4352 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13468 "src/parser_bison.c"
    break;

  case 762: /* selector_expr: hash_expr  */
#line 4353 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13474 "src/parser_bison.c"
    break;

  case 763: /* selector_expr: fib_expr  */
#line 4354 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13480 "src/parser_bison.c"
    break;

  case 764: /* selector_expr: osf_expr  */
#line 4355 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13486 "src/parser_bison.c"
    break;

  case 765: /* selector_expr: xfrm_expr  */
#line 4356 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13492 "src/parser_bison.c"
    break;

  case 766: /* primary_expr: symbol_expr  */
#line 4359 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13498 "src/parser_bison.c"
    break;

  case 767: /* primary_expr: integer_expr  */
#line 4360 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13504 "src/parser_bison.c"
    break;

  case 768: /* primary_expr: selector_expr  */
#line 4361 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13510 "src/parser_bison.c"
    break;

  case 769: /* primary_expr: '(' basic_expr ')'  */
#line 4362 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 13516 "src/parser_bison.c"
    break;

  case 770: /* fib_expr: "fib" fib_tuple fib_result close_scope_fib  */
#line 4366 "src/parser_bison.y"
                        {
				uint32_t flags = (yyvsp[-2].val), result = (yyvsp[-1].val);

				if (result == __NFT_FIB_RESULT_MAX) {
					result = NFT_FIB_RESULT_OIF;
					flags |= NFTA_FIB_F_PRESENT;
				}

				if ((flags & (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) == 0) {
					erec_queue(error(&(yylsp[-2]), "fib: need either saddr or daddr"), state->msgs);
					YYERROR;
				}

				if ((flags & (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) ==
					     (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) {
					erec_queue(error(&(yylsp[-2]), "fib: saddr and daddr are mutually exclusive"), state->msgs);
					YYERROR;
				}

				if ((flags & (NFTA_FIB_F_IIF|NFTA_FIB_F_OIF)) ==
					     (NFTA_FIB_F_IIF|NFTA_FIB_F_OIF)) {
					erec_queue(error(&(yylsp[-2]), "fib: iif and oif are mutually exclusive"), state->msgs);
					YYERROR;
				}

				(yyval.expr) = fib_expr_alloc(&(yyloc), flags, result);
			}
#line 13548 "src/parser_bison.c"
    break;

  case 771: /* fib_result: "oif"  */
#line 4395 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIF; }
#line 13554 "src/parser_bison.c"
    break;

  case 772: /* fib_result: "oifname"  */
#line 4396 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIFNAME; }
#line 13560 "src/parser_bison.c"
    break;

  case 773: /* fib_result: "type" close_scope_type  */
#line 4397 "src/parser_bison.y"
                                                                { (yyval.val) =NFT_FIB_RESULT_ADDRTYPE; }
#line 13566 "src/parser_bison.c"
    break;

  case 774: /* fib_result: "check"  */
#line 4398 "src/parser_bison.y"
                                        { (yyval.val) = __NFT_FIB_RESULT_MAX; }
#line 13572 "src/parser_bison.c"
    break;

  case 775: /* fib_flag: "saddr"  */
#line 4401 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_SADDR; }
#line 13578 "src/parser_bison.c"
    break;

  case 776: /* fib_flag: "daddr"  */
#line 4402 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_DADDR; }
#line 13584 "src/parser_bison.c"
    break;

  case 777: /* fib_flag: "mark"  */
#line 4403 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_MARK; }
#line 13590 "src/parser_bison.c"
    break;

  case 778: /* fib_flag: "iif"  */
#line 4404 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_IIF; }
#line 13596 "src/parser_bison.c"
    break;

  case 779: /* fib_flag: "oif"  */
#line 4405 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_OIF; }
#line 13602 "src/parser_bison.c"
    break;

  case 780: /* fib_tuple: fib_flag "." fib_tuple  */
#line 4409 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13610 "src/parser_bison.c"
    break;

  case 782: /* osf_expr: "osf" osf_ttl "version" close_scope_osf  */
#line 4416 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), NFT_OSF_F_VERSION);
			}
#line 13618 "src/parser_bison.c"
    break;

  case 783: /* osf_expr: "osf" osf_ttl "name" close_scope_osf  */
#line 4420 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), 0);
			}
#line 13626 "src/parser_bison.c"
    break;

  case 784: /* osf_ttl: %empty  */
#line 4426 "src/parser_bison.y"
                        {
				(yyval.val) = NF_OSF_TTL_TRUE;
			}
#line 13634 "src/parser_bison.c"
    break;

  case 785: /* osf_ttl: "ttl" "string"  */
#line 4430 "src/parser_bison.y"
                        {
				if (!strcmp((yyvsp[0].string), "loose"))
					(yyval.val) = NF_OSF_TTL_LESS;
				else if (!strcmp((yyvsp[0].string), "skip"))
					(yyval.val) = NF_OSF_TTL_NOCHECK;
				else {
					erec_queue(error(&(yylsp[0]), "invalid ttl option"),
						   state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				free_const((yyvsp[0].string));
			}
#line 13652 "src/parser_bison.c"
    break;

  case 787: /* shift_expr: shift_expr "<<" primary_rhs_expr  */
#line 4447 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13660 "src/parser_bison.c"
    break;

  case 788: /* shift_expr: shift_expr ">>" primary_rhs_expr  */
#line 4451 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13668 "src/parser_bison.c"
    break;

  case 790: /* and_expr: and_expr "&" shift_rhs_expr  */
#line 4458 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13676 "src/parser_bison.c"
    break;

  case 792: /* exclusive_or_expr: exclusive_or_expr "^" and_rhs_expr  */
#line 4465 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13684 "src/parser_bison.c"
    break;

  case 794: /* inclusive_or_expr: inclusive_or_expr '|' exclusive_or_rhs_expr  */
#line 4472 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13692 "src/parser_bison.c"
    break;

  case 797: /* concat_expr: concat_expr "." basic_expr  */
#line 4482 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 13705 "src/parser_bison.c"
    break;

  case 798: /* prefix_rhs_expr: basic_rhs_expr "/" "number"  */
#line 4493 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 13713 "src/parser_bison.c"
    break;

  case 799: /* range_rhs_expr: basic_rhs_expr "-" basic_rhs_expr  */
#line 4499 "src/parser_bison.y"
                        {
				if (is_symbol_value_expr((yyvsp[-2].expr)) &&
				    is_symbol_value_expr((yyvsp[0].expr))) {
					(yyval.expr) = symbol_range_expr_alloc(&(yyloc), (yyvsp[-2].expr)->symtype, (yyvsp[-2].expr)->scope, (yyvsp[-2].expr)->identifier, (yyvsp[0].expr)->identifier);
					expr_free((yyvsp[-2].expr));
					expr_free((yyvsp[0].expr));
				} else {
					(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
				}
			}
#line 13728 "src/parser_bison.c"
    break;

  case 802: /* map_expr: concat_expr "map" rhs_expr  */
#line 4516 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13736 "src/parser_bison.c"
    break;

  case 806: /* set_expr: '{' set_list_expr '}'  */
#line 4527 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 13745 "src/parser_bison.c"
    break;

  case 807: /* set_list_expr: set_list_member_expr  */
#line 4534 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 13754 "src/parser_bison.c"
    break;

  case 808: /* set_list_expr: set_list_expr "comma" set_list_member_expr  */
#line 4539 "src/parser_bison.y"
                        {
				compound_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 13763 "src/parser_bison.c"
    break;

  case 810: /* set_list_member_expr: opt_newline set_expr opt_newline  */
#line 4547 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 13771 "src/parser_bison.c"
    break;

  case 811: /* set_list_member_expr: opt_newline set_elem_expr opt_newline  */
#line 4551 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 13779 "src/parser_bison.c"
    break;

  case 812: /* set_list_member_expr: opt_newline set_elem_expr "colon" set_rhs_expr opt_newline  */
#line 4555 "src/parser_bison.y"
                        {
				(yyval.expr) = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr), (yyvsp[-1].expr));
			}
#line 13787 "src/parser_bison.c"
    break;

  case 814: /* meter_key_expr: meter_key_expr_alloc set_elem_options  */
#line 4562 "src/parser_bison.y"
                        {
				(yyval.expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 13796 "src/parser_bison.c"
    break;

  case 815: /* meter_key_expr_alloc: concat_expr  */
#line 4569 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 13804 "src/parser_bison.c"
    break;

  case 818: /* set_elem_expr: set_elem_expr_alloc set_elem_expr_options set_elem_stmt_list  */
#line 4577 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 13814 "src/parser_bison.c"
    break;

  case 819: /* set_elem_key_expr: set_lhs_expr  */
#line 4584 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 13820 "src/parser_bison.c"
    break;

  case 820: /* set_elem_key_expr: "*"  */
#line 4585 "src/parser_bison.y"
                                                        { (yyval.expr) = set_elem_catchall_expr_alloc(&(yylsp[0])); }
#line 13826 "src/parser_bison.c"
    break;

  case 821: /* set_elem_expr_alloc: set_elem_key_expr set_elem_stmt_list  */
#line 4589 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[-1]), (yyvsp[-1].expr));
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 13836 "src/parser_bison.c"
    break;

  case 822: /* set_elem_expr_alloc: set_elem_key_expr  */
#line 4595 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 13844 "src/parser_bison.c"
    break;

  case 823: /* set_elem_options: set_elem_option  */
#line 4601 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 13852 "src/parser_bison.c"
    break;

  case 825: /* set_elem_time_spec: "string"  */
#line 4608 "src/parser_bison.y"
                        {
				struct error_record *erec;
				uint64_t res;

				if (!strcmp("never", (yyvsp[0].string))) {
					free_const((yyvsp[0].string));
					(yyval.val) = NFT_NEVER_TIMEOUT;
					break;
				}

				erec = time_parse(&(yylsp[0]), (yyvsp[0].string), &res);
				free_const((yyvsp[0].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}
				(yyval.val) = res;
			}
#line 13875 "src/parser_bison.c"
    break;

  case 826: /* set_elem_option: "timeout" time_spec  */
#line 4629 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 13883 "src/parser_bison.c"
    break;

  case 827: /* set_elem_option: "expires" time_spec  */
#line 4633 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 13891 "src/parser_bison.c"
    break;

  case 828: /* set_elem_option: comment_spec  */
#line 4637 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 13903 "src/parser_bison.c"
    break;

  case 829: /* set_elem_expr_options: set_elem_expr_option  */
#line 4647 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 13911 "src/parser_bison.c"
    break;

  case 831: /* set_elem_stmt_list: set_elem_stmt  */
#line 4654 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 13921 "src/parser_bison.c"
    break;

  case 832: /* set_elem_stmt_list: set_elem_stmt_list set_elem_stmt  */
#line 4660 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 13930 "src/parser_bison.c"
    break;

  case 838: /* set_elem_expr_option: "timeout" set_elem_time_spec  */
#line 4674 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 13938 "src/parser_bison.c"
    break;

  case 839: /* set_elem_expr_option: "expires" time_spec  */
#line 4678 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 13946 "src/parser_bison.c"
    break;

  case 840: /* set_elem_expr_option: comment_spec  */
#line 4682 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 13958 "src/parser_bison.c"
    break;

  case 846: /* initializer_expr: '{' '}'  */
#line 4700 "src/parser_bison.y"
                                                { (yyval.expr) = compound_expr_alloc(&(yyloc), EXPR_SET); }
#line 13964 "src/parser_bison.c"
    break;

  case 847: /* initializer_expr: "-" "number"  */
#line 4702 "src/parser_bison.y"
                        {
				int32_t num = -(yyvsp[0].val);

				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(num) * BITS_PER_BYTE,
							 &num);
			}
#line 13977 "src/parser_bison.c"
    break;

  case 848: /* counter_config: "packets" "number" "bytes" "number"  */
#line 4713 "src/parser_bison.y"
                        {
				struct counter *counter;

				counter = &(yyvsp[-4].obj)->counter;
				counter->packets = (yyvsp[-2].val);
				counter->bytes = (yyvsp[0].val);
			}
#line 13989 "src/parser_bison.c"
    break;

  case 849: /* counter_obj: %empty  */
#line 4723 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_COUNTER;
			}
#line 13998 "src/parser_bison.c"
    break;

  case 850: /* quota_config: quota_mode "number" quota_unit quota_used  */
#line 4730 "src/parser_bison.y"
                        {
				struct error_record *erec;
				struct quota *quota;
				uint64_t rate;

				erec = data_unit_parse(&(yyloc), (yyvsp[-1].string), &rate);
				free_const((yyvsp[-1].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}

				quota = &(yyvsp[-4].obj)->quota;
				quota->bytes	= (yyvsp[-2].val) * rate;
				quota->used	= (yyvsp[0].val);
				quota->flags	= (yyvsp[-3].val);
			}
#line 14020 "src/parser_bison.c"
    break;

  case 851: /* quota_obj: %empty  */
#line 4750 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_QUOTA;
			}
#line 14029 "src/parser_bison.c"
    break;

  case 852: /* secmark_config: string  */
#line 4757 "src/parser_bison.y"
                        {
				int ret;
				struct secmark *secmark;

				secmark = &(yyvsp[-1].obj)->secmark;
				ret = snprintf(secmark->ctx, sizeof(secmark->ctx), "%s", (yyvsp[0].string));
				if (ret <= 0 || ret >= (int)sizeof(secmark->ctx)) {
					erec_queue(error(&(yylsp[0]), "invalid context '%s', max length is %u\n", (yyvsp[0].string), (int)sizeof(secmark->ctx)), state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				free_const((yyvsp[0].string));
			}
#line 14047 "src/parser_bison.c"
    break;

  case 853: /* secmark_obj: %empty  */
#line 4773 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SECMARK;
			}
#line 14056 "src/parser_bison.c"
    break;

  case 854: /* ct_obj_type: "helper"  */
#line 4779 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_HELPER; }
#line 14062 "src/parser_bison.c"
    break;

  case 855: /* ct_obj_type: "timeout"  */
#line 4780 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 14068 "src/parser_bison.c"
    break;

  case 856: /* ct_obj_type: "expectation"  */
#line 4781 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 14074 "src/parser_bison.c"
    break;

  case 857: /* ct_cmd_type: "helpers"  */
#line 4784 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_HELPERS; }
#line 14080 "src/parser_bison.c"
    break;

  case 858: /* ct_cmd_type: "timeout"  */
#line 4785 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_TIMEOUTS; }
#line 14086 "src/parser_bison.c"
    break;

  case 859: /* ct_cmd_type: "expectation"  */
#line 4786 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_EXPECTATIONS; }
#line 14092 "src/parser_bison.c"
    break;

  case 860: /* ct_l4protoname: "tcp" close_scope_tcp  */
#line 4789 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_TCP; }
#line 14098 "src/parser_bison.c"
    break;

  case 861: /* ct_l4protoname: "udp" close_scope_udp  */
#line 4790 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_UDP; }
#line 14104 "src/parser_bison.c"
    break;

  case 862: /* ct_helper_config: "type" "quoted string" "protocol" ct_l4protoname stmt_separator close_scope_type  */
#line 4794 "src/parser_bison.y"
                        {
				struct ct_helper *ct;
				int ret;

				ct = &(yyvsp[-6].obj)->ct_helper;

				if (ct->l4proto) {
					erec_queue(error(&(yylsp[-4]), "You can only specify this once. This statement is already set for %s.", ct->name), state->msgs);
					free_const((yyvsp[-4].string));
					YYERROR;
				}

				ret = snprintf(ct->name, sizeof(ct->name), "%s", (yyvsp[-4].string));
				if (ret <= 0 || ret >= (int)sizeof(ct->name)) {
					erec_queue(error(&(yylsp[-4]), "invalid name '%s', max length is %u\n", (yyvsp[-4].string), (int)sizeof(ct->name)), state->msgs);
					free_const((yyvsp[-4].string));
					YYERROR;
				}
				free_const((yyvsp[-4].string));

				ct->l4proto = (yyvsp[-2].val);
			}
#line 14131 "src/parser_bison.c"
    break;

  case 863: /* ct_helper_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4817 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_helper.l3proto = (yyvsp[-1].val);
			}
#line 14139 "src/parser_bison.c"
    break;

  case 864: /* timeout_states: timeout_state  */
#line 4823 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyval.list));
			}
#line 14149 "src/parser_bison.c"
    break;

  case 865: /* timeout_states: timeout_states "comma" timeout_state  */
#line 4829 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyvsp[-2].list));
				(yyval.list) = (yyvsp[-2].list);
			}
#line 14158 "src/parser_bison.c"
    break;

  case 866: /* timeout_state: "string" "colon" time_spec_or_num_s  */
#line 4836 "src/parser_bison.y"
                        {
				struct timeout_state *ts;

				ts = xzalloc(sizeof(*ts));
				ts->timeout_str = (yyvsp[-2].string);
				ts->timeout_value = (yyvsp[0].val);
				ts->location = (yylsp[-2]);
				init_list_head(&ts->head);
				(yyval.timeout_state) = ts;
			}
#line 14173 "src/parser_bison.c"
    break;

  case 867: /* ct_timeout_config: "protocol" ct_l4protoname stmt_separator  */
#line 4849 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;
				int l4proto = (yyvsp[-1].val);

				ct = &(yyvsp[-3].obj)->ct_timeout;
				ct->l4proto = l4proto;
			}
#line 14185 "src/parser_bison.c"
    break;

  case 868: /* ct_timeout_config: "policy" '=' '{' timeout_states '}' stmt_separator close_scope_policy  */
#line 4857 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;

				ct = &(yyvsp[-7].obj)->ct_timeout;
				list_splice_tail((yyvsp[-3].list), &ct->timeout_list);
				free((yyvsp[-3].list));
			}
#line 14197 "src/parser_bison.c"
    break;

  case 869: /* ct_timeout_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4865 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_timeout.l3proto = (yyvsp[-1].val);
			}
#line 14205 "src/parser_bison.c"
    break;

  case 870: /* ct_expect_config: "protocol" ct_l4protoname stmt_separator  */
#line 4871 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l4proto = (yyvsp[-1].val);
			}
#line 14213 "src/parser_bison.c"
    break;

  case 871: /* ct_expect_config: "dport" "number" stmt_separator  */
#line 4875 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.dport = (yyvsp[-1].val);
			}
#line 14221 "src/parser_bison.c"
    break;

  case 872: /* ct_expect_config: "timeout" time_spec stmt_separator  */
#line 4879 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.timeout = (yyvsp[-1].val);
			}
#line 14229 "src/parser_bison.c"
    break;

  case 873: /* ct_expect_config: "size" "number" stmt_separator  */
#line 4883 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.size = (yyvsp[-1].val);
			}
#line 14237 "src/parser_bison.c"
    break;

  case 874: /* ct_expect_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4887 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l3proto = (yyvsp[-1].val);
			}
#line 14245 "src/parser_bison.c"
    break;

  case 875: /* ct_obj_alloc: %empty  */
#line 4893 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
			}
#line 14253 "src/parser_bison.c"
    break;

  case 876: /* limit_config: "rate" limit_mode limit_rate_pkts limit_burst_pkts  */
#line 4899 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-4].obj)->limit;
				limit->rate	= (yyvsp[-1].limit_rate).rate;
				limit->unit	= (yyvsp[-1].limit_rate).unit;
				limit->burst	= (yyvsp[0].val);
				limit->type	= NFT_LIMIT_PKTS;
				limit->flags	= (yyvsp[-2].val);
			}
#line 14268 "src/parser_bison.c"
    break;

  case 877: /* limit_config: "rate" limit_mode limit_rate_bytes limit_burst_bytes  */
#line 4910 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-4].obj)->limit;
				limit->rate	= (yyvsp[-1].limit_rate).rate;
				limit->unit	= (yyvsp[-1].limit_rate).unit;
				limit->burst	= (yyvsp[0].val);
				limit->type	= NFT_LIMIT_PKT_BYTES;
				limit->flags	= (yyvsp[-2].val);
			}
#line 14283 "src/parser_bison.c"
    break;

  case 878: /* limit_obj: %empty  */
#line 4923 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_LIMIT;
			}
#line 14292 "src/parser_bison.c"
    break;

  case 879: /* relational_expr: expr rhs_expr  */
#line 4930 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 14300 "src/parser_bison.c"
    break;

  case 880: /* relational_expr: expr list_rhs_expr  */
#line 4934 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 14308 "src/parser_bison.c"
    break;

  case 881: /* relational_expr: expr basic_rhs_expr "/" list_rhs_expr  */
#line 4938 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, (yyvsp[-2].expr));
			}
#line 14319 "src/parser_bison.c"
    break;

  case 882: /* relational_expr: expr list_rhs_expr "/" list_rhs_expr  */
#line 4945 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, value);
			}
#line 14331 "src/parser_bison.c"
    break;

  case 883: /* relational_expr: expr relational_op basic_rhs_expr "/" list_rhs_expr  */
#line 4953 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, (yyvsp[-2].expr));
			}
#line 14342 "src/parser_bison.c"
    break;

  case 884: /* relational_expr: expr relational_op list_rhs_expr "/" list_rhs_expr  */
#line 4960 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, value);
			}
#line 14354 "src/parser_bison.c"
    break;

  case 885: /* relational_expr: expr relational_op rhs_expr  */
#line 4968 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14362 "src/parser_bison.c"
    break;

  case 886: /* relational_expr: expr relational_op list_rhs_expr  */
#line 4972 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14370 "src/parser_bison.c"
    break;

  case 887: /* list_rhs_expr: basic_rhs_expr "comma" basic_rhs_expr  */
#line 4978 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				compound_expr_add((yyval.expr), (yyvsp[-2].expr));
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 14380 "src/parser_bison.c"
    break;

  case 888: /* list_rhs_expr: list_rhs_expr "comma" basic_rhs_expr  */
#line 4984 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				compound_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 14390 "src/parser_bison.c"
    break;

  case 889: /* rhs_expr: concat_rhs_expr  */
#line 4991 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14396 "src/parser_bison.c"
    break;

  case 890: /* rhs_expr: set_expr  */
#line 4992 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14402 "src/parser_bison.c"
    break;

  case 891: /* rhs_expr: set_ref_symbol_expr  */
#line 4993 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14408 "src/parser_bison.c"
    break;

  case 893: /* shift_rhs_expr: shift_rhs_expr "<<" primary_rhs_expr  */
#line 4998 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14416 "src/parser_bison.c"
    break;

  case 894: /* shift_rhs_expr: shift_rhs_expr ">>" primary_rhs_expr  */
#line 5002 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14424 "src/parser_bison.c"
    break;

  case 896: /* and_rhs_expr: and_rhs_expr "&" shift_rhs_expr  */
#line 5009 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14432 "src/parser_bison.c"
    break;

  case 898: /* exclusive_or_rhs_expr: exclusive_or_rhs_expr "^" and_rhs_expr  */
#line 5016 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14440 "src/parser_bison.c"
    break;

  case 900: /* inclusive_or_rhs_expr: inclusive_or_rhs_expr '|' exclusive_or_rhs_expr  */
#line 5023 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14448 "src/parser_bison.c"
    break;

  case 904: /* concat_rhs_expr: concat_rhs_expr "." multiton_rhs_expr  */
#line 5034 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 14461 "src/parser_bison.c"
    break;

  case 905: /* concat_rhs_expr: concat_rhs_expr "." basic_rhs_expr  */
#line 5043 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 14474 "src/parser_bison.c"
    break;

  case 906: /* boolean_keys: "exists"  */
#line 5053 "src/parser_bison.y"
                                                { (yyval.val8) = true; }
#line 14480 "src/parser_bison.c"
    break;

  case 907: /* boolean_keys: "missing"  */
#line 5054 "src/parser_bison.y"
                                                { (yyval.val8) = false; }
#line 14486 "src/parser_bison.c"
    break;

  case 908: /* boolean_expr: boolean_keys  */
#line 5058 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &boolean_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof((yyvsp[0].val8)) * BITS_PER_BYTE, &(yyvsp[0].val8));
			}
#line 14496 "src/parser_bison.c"
    break;

  case 909: /* keyword_expr: "ether" close_scope_eth  */
#line 5065 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ether"); }
#line 14502 "src/parser_bison.c"
    break;

  case 910: /* keyword_expr: "ip" close_scope_ip  */
#line 5066 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip"); }
#line 14508 "src/parser_bison.c"
    break;

  case 911: /* keyword_expr: "ip6" close_scope_ip6  */
#line 5067 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip6"); }
#line 14514 "src/parser_bison.c"
    break;

  case 912: /* keyword_expr: "vlan" close_scope_vlan  */
#line 5068 "src/parser_bison.y"
                                                         { (yyval.expr) = symbol_value(&(yyloc), "vlan"); }
#line 14520 "src/parser_bison.c"
    break;

  case 913: /* keyword_expr: "arp" close_scope_arp  */
#line 5069 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "arp"); }
#line 14526 "src/parser_bison.c"
    break;

  case 914: /* keyword_expr: "dnat" close_scope_nat  */
#line 5070 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "dnat"); }
#line 14532 "src/parser_bison.c"
    break;

  case 915: /* keyword_expr: "snat" close_scope_nat  */
#line 5071 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "snat"); }
#line 14538 "src/parser_bison.c"
    break;

  case 916: /* keyword_expr: "ecn"  */
#line 5072 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ecn"); }
#line 14544 "src/parser_bison.c"
    break;

  case 917: /* keyword_expr: "reset" close_scope_reset  */
#line 5073 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "reset"); }
#line 14550 "src/parser_bison.c"
    break;

  case 918: /* keyword_expr: "destroy" close_scope_destroy  */
#line 5074 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "destroy"); }
#line 14556 "src/parser_bison.c"
    break;

  case 919: /* keyword_expr: "original"  */
#line 5075 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "original"); }
#line 14562 "src/parser_bison.c"
    break;

  case 920: /* keyword_expr: "reply"  */
#line 5076 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "reply"); }
#line 14568 "src/parser_bison.c"
    break;

  case 921: /* keyword_expr: "label"  */
#line 5077 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "label"); }
#line 14574 "src/parser_bison.c"
    break;

  case 922: /* keyword_expr: "last" close_scope_last  */
#line 5078 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "last"); }
#line 14580 "src/parser_bison.c"
    break;

  case 923: /* primary_rhs_expr: symbol_expr  */
#line 5081 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14586 "src/parser_bison.c"
    break;

  case 924: /* primary_rhs_expr: integer_expr  */
#line 5082 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14592 "src/parser_bison.c"
    break;

  case 925: /* primary_rhs_expr: boolean_expr  */
#line 5083 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14598 "src/parser_bison.c"
    break;

  case 926: /* primary_rhs_expr: keyword_expr  */
#line 5084 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14604 "src/parser_bison.c"
    break;

  case 927: /* primary_rhs_expr: "tcp" close_scope_tcp  */
#line 5086 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_TCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14615 "src/parser_bison.c"
    break;

  case 928: /* primary_rhs_expr: "udp" close_scope_udp  */
#line 5093 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14626 "src/parser_bison.c"
    break;

  case 929: /* primary_rhs_expr: "udplite" close_scope_udplite  */
#line 5100 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDPLITE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14637 "src/parser_bison.c"
    break;

  case 930: /* primary_rhs_expr: "esp" close_scope_esp  */
#line 5107 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ESP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14648 "src/parser_bison.c"
    break;

  case 931: /* primary_rhs_expr: "ah" close_scope_ah  */
#line 5114 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_AH;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14659 "src/parser_bison.c"
    break;

  case 932: /* primary_rhs_expr: "icmp" close_scope_icmp  */
#line 5121 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14670 "src/parser_bison.c"
    break;

  case 933: /* primary_rhs_expr: "igmp"  */
#line 5128 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_IGMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14681 "src/parser_bison.c"
    break;

  case 934: /* primary_rhs_expr: "icmpv6" close_scope_icmp  */
#line 5135 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMPV6;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14692 "src/parser_bison.c"
    break;

  case 935: /* primary_rhs_expr: "gre" close_scope_gre  */
#line 5142 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_GRE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14703 "src/parser_bison.c"
    break;

  case 936: /* primary_rhs_expr: "comp" close_scope_comp  */
#line 5149 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_COMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14714 "src/parser_bison.c"
    break;

  case 937: /* primary_rhs_expr: "dccp" close_scope_dccp  */
#line 5156 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_DCCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14725 "src/parser_bison.c"
    break;

  case 938: /* primary_rhs_expr: "sctp" close_scope_sctp  */
#line 5163 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_SCTP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14736 "src/parser_bison.c"
    break;

  case 939: /* primary_rhs_expr: "redirect" close_scope_nat  */
#line 5170 "src/parser_bison.y"
                        {
				uint8_t data = ICMP_REDIRECT;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &icmp_type_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 14747 "src/parser_bison.c"
    break;

  case 940: /* primary_rhs_expr: '(' basic_rhs_expr ')'  */
#line 5176 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 14753 "src/parser_bison.c"
    break;

  case 941: /* relational_op: "=="  */
#line 5179 "src/parser_bison.y"
                                                { (yyval.val) = OP_EQ; }
#line 14759 "src/parser_bison.c"
    break;

  case 942: /* relational_op: "!="  */
#line 5180 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEQ; }
#line 14765 "src/parser_bison.c"
    break;

  case 943: /* relational_op: "<"  */
#line 5181 "src/parser_bison.y"
                                                { (yyval.val) = OP_LT; }
#line 14771 "src/parser_bison.c"
    break;

  case 944: /* relational_op: ">"  */
#line 5182 "src/parser_bison.y"
                                                { (yyval.val) = OP_GT; }
#line 14777 "src/parser_bison.c"
    break;

  case 945: /* relational_op: ">="  */
#line 5183 "src/parser_bison.y"
                                                { (yyval.val) = OP_GTE; }
#line 14783 "src/parser_bison.c"
    break;

  case 946: /* relational_op: "<="  */
#line 5184 "src/parser_bison.y"
                                                { (yyval.val) = OP_LTE; }
#line 14789 "src/parser_bison.c"
    break;

  case 947: /* relational_op: "!"  */
#line 5185 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEG; }
#line 14795 "src/parser_bison.c"
    break;

  case 948: /* verdict_expr: "accept"  */
#line 5189 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_ACCEPT, NULL);
			}
#line 14803 "src/parser_bison.c"
    break;

  case 949: /* verdict_expr: "drop"  */
#line 5193 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_DROP, NULL);
			}
#line 14811 "src/parser_bison.c"
    break;

  case 950: /* verdict_expr: "continue"  */
#line 5197 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_CONTINUE, NULL);
			}
#line 14819 "src/parser_bison.c"
    break;

  case 951: /* verdict_expr: "jump" chain_expr  */
#line 5201 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_JUMP, (yyvsp[0].expr));
			}
#line 14827 "src/parser_bison.c"
    break;

  case 952: /* verdict_expr: "goto" chain_expr  */
#line 5205 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_GOTO, (yyvsp[0].expr));
			}
#line 14835 "src/parser_bison.c"
    break;

  case 953: /* verdict_expr: "return"  */
#line 5209 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_RETURN, NULL);
			}
#line 14843 "src/parser_bison.c"
    break;

  case 955: /* chain_expr: identifier  */
#line 5216 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &string_type,
							 BYTEORDER_HOST_ENDIAN,
							 strlen((yyvsp[0].string)) * BITS_PER_BYTE,
							 (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 14855 "src/parser_bison.c"
    break;

  case 956: /* meta_expr: "meta" meta_key close_scope_meta  */
#line 5226 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 14863 "src/parser_bison.c"
    break;

  case 957: /* meta_expr: meta_key_unqualified  */
#line 5230 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[0].val));
			}
#line 14871 "src/parser_bison.c"
    break;

  case 958: /* meta_expr: "meta" "string" close_scope_meta  */
#line 5234 "src/parser_bison.y"
                        {
				struct error_record *erec;
				unsigned int key;

				erec = meta_key_parse(&(yyloc), (yyvsp[-1].string), &key);
				free_const((yyvsp[-1].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					YYERROR;
				}

				(yyval.expr) = meta_expr_alloc(&(yyloc), key);
			}
#line 14889 "src/parser_bison.c"
    break;

  case 961: /* meta_key_qualified: "length"  */
#line 5253 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_LEN; }
#line 14895 "src/parser_bison.c"
    break;

  case 962: /* meta_key_qualified: "protocol"  */
#line 5254 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PROTOCOL; }
#line 14901 "src/parser_bison.c"
    break;

  case 963: /* meta_key_qualified: "priority"  */
#line 5255 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRIORITY; }
#line 14907 "src/parser_bison.c"
    break;

  case 964: /* meta_key_qualified: "random"  */
#line 5256 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRANDOM; }
#line 14913 "src/parser_bison.c"
    break;

  case 965: /* meta_key_qualified: "secmark" close_scope_secmark  */
#line 5257 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_META_SECMARK; }
#line 14919 "src/parser_bison.c"
    break;

  case 966: /* meta_key_unqualified: "mark"  */
#line 5260 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_MARK; }
#line 14925 "src/parser_bison.c"
    break;

  case 967: /* meta_key_unqualified: "iif"  */
#line 5261 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIF; }
#line 14931 "src/parser_bison.c"
    break;

  case 968: /* meta_key_unqualified: "iifname"  */
#line 5262 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFNAME; }
#line 14937 "src/parser_bison.c"
    break;

  case 969: /* meta_key_unqualified: "iiftype"  */
#line 5263 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFTYPE; }
#line 14943 "src/parser_bison.c"
    break;

  case 970: /* meta_key_unqualified: "oif"  */
#line 5264 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIF; }
#line 14949 "src/parser_bison.c"
    break;

  case 971: /* meta_key_unqualified: "oifname"  */
#line 5265 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFNAME; }
#line 14955 "src/parser_bison.c"
    break;

  case 972: /* meta_key_unqualified: "oiftype"  */
#line 5266 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFTYPE; }
#line 14961 "src/parser_bison.c"
    break;

  case 973: /* meta_key_unqualified: "skuid"  */
#line 5267 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKUID; }
#line 14967 "src/parser_bison.c"
    break;

  case 974: /* meta_key_unqualified: "skgid"  */
#line 5268 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKGID; }
#line 14973 "src/parser_bison.c"
    break;

  case 975: /* meta_key_unqualified: "nftrace"  */
#line 5269 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_NFTRACE; }
#line 14979 "src/parser_bison.c"
    break;

  case 976: /* meta_key_unqualified: "rtclassid"  */
#line 5270 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_RTCLASSID; }
#line 14985 "src/parser_bison.c"
    break;

  case 977: /* meta_key_unqualified: "ibriport"  */
#line 5271 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFNAME; }
#line 14991 "src/parser_bison.c"
    break;

  case 978: /* meta_key_unqualified: "obriport"  */
#line 5272 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_OIFNAME; }
#line 14997 "src/parser_bison.c"
    break;

  case 979: /* meta_key_unqualified: "ibrname"  */
#line 5273 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFNAME; }
#line 15003 "src/parser_bison.c"
    break;

  case 980: /* meta_key_unqualified: "obrname"  */
#line 5274 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_OIFNAME; }
#line 15009 "src/parser_bison.c"
    break;

  case 981: /* meta_key_unqualified: "pkttype"  */
#line 5275 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PKTTYPE; }
#line 15015 "src/parser_bison.c"
    break;

  case 982: /* meta_key_unqualified: "cpu"  */
#line 5276 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CPU; }
#line 15021 "src/parser_bison.c"
    break;

  case 983: /* meta_key_unqualified: "iifgroup"  */
#line 5277 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFGROUP; }
#line 15027 "src/parser_bison.c"
    break;

  case 984: /* meta_key_unqualified: "oifgroup"  */
#line 5278 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFGROUP; }
#line 15033 "src/parser_bison.c"
    break;

  case 985: /* meta_key_unqualified: "cgroup"  */
#line 5279 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CGROUP; }
#line 15039 "src/parser_bison.c"
    break;

  case 986: /* meta_key_unqualified: "ipsec" close_scope_ipsec  */
#line 5280 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_META_SECPATH; }
#line 15045 "src/parser_bison.c"
    break;

  case 987: /* meta_key_unqualified: "time"  */
#line 5281 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_NS; }
#line 15051 "src/parser_bison.c"
    break;

  case 988: /* meta_key_unqualified: "day"  */
#line 5282 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_DAY; }
#line 15057 "src/parser_bison.c"
    break;

  case 989: /* meta_key_unqualified: "hour"  */
#line 5283 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_HOUR; }
#line 15063 "src/parser_bison.c"
    break;

  case 990: /* meta_stmt: "meta" meta_key "set" stmt_expr close_scope_meta  */
#line 5287 "src/parser_bison.y"
                        {
				switch ((yyvsp[-3].val)) {
				case NFT_META_SECMARK:
					switch ((yyvsp[-1].expr)->etype) {
					case EXPR_CT:
						(yyval.stmt) = meta_stmt_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-1].expr));
						break;
					default:
						(yyval.stmt) = objref_stmt_alloc(&(yyloc));
						(yyval.stmt)->objref.type = NFT_OBJECT_SECMARK;
						(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
						break;
					}
					break;
				default:
					(yyval.stmt) = meta_stmt_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-1].expr));
					break;
				}
			}
#line 15087 "src/parser_bison.c"
    break;

  case 991: /* meta_stmt: meta_key_unqualified "set" stmt_expr  */
#line 5307 "src/parser_bison.y"
                        {
				(yyval.stmt) = meta_stmt_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[0].expr));
			}
#line 15095 "src/parser_bison.c"
    break;

  case 992: /* meta_stmt: "meta" "string" "set" stmt_expr close_scope_meta  */
#line 5311 "src/parser_bison.y"
                        {
				struct error_record *erec;
				unsigned int key;

				erec = meta_key_parse(&(yyloc), (yyvsp[-3].string), &key);
				free_const((yyvsp[-3].string));
				if (erec != NULL) {
					erec_queue(erec, state->msgs);
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyval.stmt) = meta_stmt_alloc(&(yyloc), key, (yyvsp[-1].expr));
			}
#line 15114 "src/parser_bison.c"
    break;

  case 993: /* meta_stmt: "notrack"  */
#line 5326 "src/parser_bison.y"
                        {
				(yyval.stmt) = notrack_stmt_alloc(&(yyloc));
			}
#line 15122 "src/parser_bison.c"
    break;

  case 994: /* meta_stmt: "flow" "offload" "@" string close_scope_at  */
#line 5330 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 15130 "src/parser_bison.c"
    break;

  case 995: /* meta_stmt: "flow" "add" "@" string close_scope_at  */
#line 5334 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 15138 "src/parser_bison.c"
    break;

  case 996: /* socket_expr: "socket" socket_key close_scope_socket  */
#line 5340 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), (yyvsp[-1].val), 0);
			}
#line 15146 "src/parser_bison.c"
    break;

  case 997: /* socket_expr: "socket" "cgroupv2" "level" "number" close_scope_socket  */
#line 5344 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), NFT_SOCKET_CGROUPV2, (yyvsp[-1].val));
			}
#line 15154 "src/parser_bison.c"
    break;

  case 998: /* socket_key: "transparent"  */
#line 5349 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_TRANSPARENT; }
#line 15160 "src/parser_bison.c"
    break;

  case 999: /* socket_key: "mark"  */
#line 5350 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_MARK; }
#line 15166 "src/parser_bison.c"
    break;

  case 1000: /* socket_key: "wildcard"  */
#line 5351 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_WILDCARD; }
#line 15172 "src/parser_bison.c"
    break;

  case 1001: /* offset_opt: %empty  */
#line 5354 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 15178 "src/parser_bison.c"
    break;

  case 1002: /* offset_opt: "offset" "number"  */
#line 5355 "src/parser_bison.y"
                                                { (yyval.val) = (yyvsp[0].val); }
#line 15184 "src/parser_bison.c"
    break;

  case 1003: /* numgen_type: "inc"  */
#line 5358 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_INCREMENTAL; }
#line 15190 "src/parser_bison.c"
    break;

  case 1004: /* numgen_type: "random"  */
#line 5359 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_RANDOM; }
#line 15196 "src/parser_bison.c"
    break;

  case 1005: /* numgen_expr: "numgen" numgen_type "mod" "number" offset_opt close_scope_numgen  */
#line 5363 "src/parser_bison.y"
                        {
				(yyval.expr) = numgen_expr_alloc(&(yyloc), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 15204 "src/parser_bison.c"
    break;

  case 1006: /* xfrm_spnum: "spnum" "number"  */
#line 5368 "src/parser_bison.y"
                                            { (yyval.val) = (yyvsp[0].val); }
#line 15210 "src/parser_bison.c"
    break;

  case 1007: /* xfrm_spnum: %empty  */
#line 5369 "src/parser_bison.y"
                                            { (yyval.val) = 0; }
#line 15216 "src/parser_bison.c"
    break;

  case 1008: /* xfrm_dir: "in"  */
#line 5372 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_IN; }
#line 15222 "src/parser_bison.c"
    break;

  case 1009: /* xfrm_dir: "out"  */
#line 5373 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_OUT; }
#line 15228 "src/parser_bison.c"
    break;

  case 1010: /* xfrm_state_key: "spi"  */
#line 5376 "src/parser_bison.y"
                                    { (yyval.val) = NFT_XFRM_KEY_SPI; }
#line 15234 "src/parser_bison.c"
    break;

  case 1011: /* xfrm_state_key: "reqid"  */
#line 5377 "src/parser_bison.y"
                                      { (yyval.val) = NFT_XFRM_KEY_REQID; }
#line 15240 "src/parser_bison.c"
    break;

  case 1012: /* xfrm_state_proto_key: "daddr"  */
#line 5380 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_DADDR_IP4; }
#line 15246 "src/parser_bison.c"
    break;

  case 1013: /* xfrm_state_proto_key: "saddr"  */
#line 5381 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_SADDR_IP4; }
#line 15252 "src/parser_bison.c"
    break;

  case 1014: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum xfrm_state_key close_scope_ipsec  */
#line 5385 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].val) > 255) {
					erec_queue(error(&(yylsp[-2]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.expr) = xfrm_expr_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 15264 "src/parser_bison.c"
    break;

  case 1015: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum nf_key_proto xfrm_state_proto_key close_scope_ipsec  */
#line 5393 "src/parser_bison.y"
                        {
				enum nft_xfrm_keys xfrmk = (yyvsp[-1].val);

				switch ((yyvsp[-2].val)) {
				case NFPROTO_IPV4:
					break;
				case NFPROTO_IPV6:
					if ((yyvsp[-1].val) == NFT_XFRM_KEY_SADDR_IP4)
						xfrmk = NFT_XFRM_KEY_SADDR_IP6;
					else if ((yyvsp[-1].val) == NFT_XFRM_KEY_DADDR_IP4)
						xfrmk = NFT_XFRM_KEY_DADDR_IP6;
					break;
				default:
					YYERROR;
					break;
				}

				if ((yyvsp[-3].val) > 255) {
					erec_queue(error(&(yylsp[-3]), "value too large"), state->msgs);
					YYERROR;
				}

				(yyval.expr) = xfrm_expr_alloc(&(yyloc), (yyvsp[-4].val), (yyvsp[-3].val), xfrmk);
			}
#line 15293 "src/parser_bison.c"
    break;

  case 1016: /* hash_expr: "jhash" expr "mod" "number" "seed" "number" offset_opt close_scope_hash  */
#line 5420 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-4].val), true, (yyvsp[-2].val), (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-6].expr);
			}
#line 15302 "src/parser_bison.c"
    break;

  case 1017: /* hash_expr: "jhash" expr "mod" "number" offset_opt close_scope_hash  */
#line 5425 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-4].expr);
			}
#line 15311 "src/parser_bison.c"
    break;

  case 1018: /* hash_expr: "symhash" "mod" "number" offset_opt close_scope_hash  */
#line 5430 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_SYM);
			}
#line 15319 "src/parser_bison.c"
    break;

  case 1019: /* nf_key_proto: "ip" close_scope_ip  */
#line 5435 "src/parser_bison.y"
                                                       { (yyval.val) = NFPROTO_IPV4; }
#line 15325 "src/parser_bison.c"
    break;

  case 1020: /* nf_key_proto: "ip6" close_scope_ip6  */
#line 5436 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 15331 "src/parser_bison.c"
    break;

  case 1021: /* rt_expr: "rt" rt_key close_scope_rt  */
#line 5440 "src/parser_bison.y"
                        {
				(yyval.expr) = rt_expr_alloc(&(yyloc), (yyvsp[-1].val), true);
			}
#line 15339 "src/parser_bison.c"
    break;

  case 1022: /* rt_expr: "rt" nf_key_proto rt_key close_scope_rt  */
#line 5444 "src/parser_bison.y"
                        {
				enum nft_rt_keys rtk = (yyvsp[-1].val);

				switch ((yyvsp[-2].val)) {
				case NFPROTO_IPV4:
					break;
				case NFPROTO_IPV6:
					if ((yyvsp[-1].val) == NFT_RT_NEXTHOP4)
						rtk = NFT_RT_NEXTHOP6;
					break;
				default:
					YYERROR;
					break;
				}

				(yyval.expr) = rt_expr_alloc(&(yyloc), rtk, false);
			}
#line 15361 "src/parser_bison.c"
    break;

  case 1023: /* rt_key: "classid"  */
#line 5463 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_CLASSID; }
#line 15367 "src/parser_bison.c"
    break;

  case 1024: /* rt_key: "nexthop"  */
#line 5464 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_NEXTHOP4; }
#line 15373 "src/parser_bison.c"
    break;

  case 1025: /* rt_key: "mtu"  */
#line 5465 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_TCPMSS; }
#line 15379 "src/parser_bison.c"
    break;

  case 1026: /* rt_key: "ipsec" close_scope_ipsec  */
#line 5466 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_RT_XFRM; }
#line 15385 "src/parser_bison.c"
    break;

  case 1027: /* ct_expr: "ct" ct_key close_scope_ct  */
#line 5470 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), -1);
			}
#line 15393 "src/parser_bison.c"
    break;

  case 1028: /* ct_expr: "ct" ct_dir ct_key_dir close_scope_ct  */
#line 5474 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 15401 "src/parser_bison.c"
    break;

  case 1029: /* ct_expr: "ct" ct_dir ct_key_proto_field close_scope_ct  */
#line 5478 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 15409 "src/parser_bison.c"
    break;

  case 1030: /* ct_dir: "original"  */
#line 5483 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_ORIGINAL; }
#line 15415 "src/parser_bison.c"
    break;

  case 1031: /* ct_dir: "reply"  */
#line 5484 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_REPLY; }
#line 15421 "src/parser_bison.c"
    break;

  case 1032: /* ct_key: "l3proto"  */
#line 5487 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 15427 "src/parser_bison.c"
    break;

  case 1033: /* ct_key: "protocol"  */
#line 5488 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 15433 "src/parser_bison.c"
    break;

  case 1034: /* ct_key: "mark"  */
#line 5489 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_MARK; }
#line 15439 "src/parser_bison.c"
    break;

  case 1035: /* ct_key: "state"  */
#line 5490 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATE; }
#line 15445 "src/parser_bison.c"
    break;

  case 1036: /* ct_key: "direction"  */
#line 5491 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DIRECTION; }
#line 15451 "src/parser_bison.c"
    break;

  case 1037: /* ct_key: "status"  */
#line 5492 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATUS; }
#line 15457 "src/parser_bison.c"
    break;

  case 1038: /* ct_key: "expiration"  */
#line 5493 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EXPIRATION; }
#line 15463 "src/parser_bison.c"
    break;

  case 1039: /* ct_key: "helper"  */
#line 5494 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_HELPER; }
#line 15469 "src/parser_bison.c"
    break;

  case 1040: /* ct_key: "saddr"  */
#line 5495 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 15475 "src/parser_bison.c"
    break;

  case 1041: /* ct_key: "daddr"  */
#line 5496 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 15481 "src/parser_bison.c"
    break;

  case 1042: /* ct_key: "proto-src"  */
#line 5497 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 15487 "src/parser_bison.c"
    break;

  case 1043: /* ct_key: "proto-dst"  */
#line 5498 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 15493 "src/parser_bison.c"
    break;

  case 1044: /* ct_key: "label"  */
#line 5499 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_LABELS; }
#line 15499 "src/parser_bison.c"
    break;

  case 1045: /* ct_key: "event"  */
#line 5500 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EVENTMASK; }
#line 15505 "src/parser_bison.c"
    break;

  case 1046: /* ct_key: "secmark" close_scope_secmark  */
#line 5501 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_CT_SECMARK; }
#line 15511 "src/parser_bison.c"
    break;

  case 1047: /* ct_key: "id"  */
#line 5502 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ID; }
#line 15517 "src/parser_bison.c"
    break;

  case 1049: /* ct_key_dir: "saddr"  */
#line 5506 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 15523 "src/parser_bison.c"
    break;

  case 1050: /* ct_key_dir: "daddr"  */
#line 5507 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 15529 "src/parser_bison.c"
    break;

  case 1051: /* ct_key_dir: "l3proto"  */
#line 5508 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 15535 "src/parser_bison.c"
    break;

  case 1052: /* ct_key_dir: "protocol"  */
#line 5509 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 15541 "src/parser_bison.c"
    break;

  case 1053: /* ct_key_dir: "proto-src"  */
#line 5510 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 15547 "src/parser_bison.c"
    break;

  case 1054: /* ct_key_dir: "proto-dst"  */
#line 5511 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 15553 "src/parser_bison.c"
    break;

  case 1056: /* ct_key_proto_field: "ip" "saddr" close_scope_ip  */
#line 5515 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_SRC_IP; }
#line 15559 "src/parser_bison.c"
    break;

  case 1057: /* ct_key_proto_field: "ip" "daddr" close_scope_ip  */
#line 5516 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_DST_IP; }
#line 15565 "src/parser_bison.c"
    break;

  case 1058: /* ct_key_proto_field: "ip6" "saddr" close_scope_ip6  */
#line 5517 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_SRC_IP6; }
#line 15571 "src/parser_bison.c"
    break;

  case 1059: /* ct_key_proto_field: "ip6" "daddr" close_scope_ip6  */
#line 5518 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_DST_IP6; }
#line 15577 "src/parser_bison.c"
    break;

  case 1060: /* ct_key_dir_optional: "bytes"  */
#line 5521 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_BYTES; }
#line 15583 "src/parser_bison.c"
    break;

  case 1061: /* ct_key_dir_optional: "packets"  */
#line 5522 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PKTS; }
#line 15589 "src/parser_bison.c"
    break;

  case 1062: /* ct_key_dir_optional: "avgpkt"  */
#line 5523 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_AVGPKT; }
#line 15595 "src/parser_bison.c"
    break;

  case 1063: /* ct_key_dir_optional: "zone"  */
#line 5524 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ZONE; }
#line 15601 "src/parser_bison.c"
    break;

  case 1066: /* list_stmt_expr: symbol_stmt_expr "comma" symbol_stmt_expr  */
#line 5532 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				compound_expr_add((yyval.expr), (yyvsp[-2].expr));
				compound_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 15611 "src/parser_bison.c"
    break;

  case 1067: /* list_stmt_expr: list_stmt_expr "comma" symbol_stmt_expr  */
#line 5538 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				compound_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 15621 "src/parser_bison.c"
    break;

  case 1068: /* ct_stmt: "ct" ct_key "set" stmt_expr close_scope_ct  */
#line 5546 "src/parser_bison.y"
                        {
				switch ((yyvsp[-3].val)) {
				case NFT_CT_HELPER:
					(yyval.stmt) = objref_stmt_alloc(&(yyloc));
					(yyval.stmt)->objref.type = NFT_OBJECT_CT_HELPER;
					(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
					break;
				default:
					(yyval.stmt) = ct_stmt_alloc(&(yyloc), (yyvsp[-3].val), -1, (yyvsp[-1].expr));
					break;
				}
			}
#line 15638 "src/parser_bison.c"
    break;

  case 1069: /* ct_stmt: "ct" ct_dir ct_key_dir_optional "set" stmt_expr close_scope_ct  */
#line 5559 "src/parser_bison.y"
                        {
				(yyval.stmt) = ct_stmt_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-4].val), (yyvsp[-1].expr));
			}
#line 15646 "src/parser_bison.c"
    break;

  case 1070: /* payload_stmt: payload_expr "set" stmt_expr  */
#line 5565 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].expr)->etype == EXPR_EXTHDR)
					(yyval.stmt) = exthdr_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
				else
					(yyval.stmt) = payload_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15657 "src/parser_bison.c"
    break;

  case 1093: /* payload_raw_len: "number"  */
#line 5598 "src/parser_bison.y"
                        {
				if ((yyvsp[0].val) > NFT_MAX_EXPR_LEN_BITS) {
					erec_queue(error(&(yylsp[0]), "raw payload length %u exceeds upper limit of %u",
							 (yyvsp[0].val), NFT_MAX_EXPR_LEN_BITS),
						 state->msgs);
					YYERROR;
				}

				if ((yyvsp[0].val) == 0) {
					erec_queue(error(&(yylsp[0]), "raw payload length cannot be 0"), state->msgs);
					YYERROR;
				}

				(yyval.val) = (yyvsp[0].val);
			}
#line 15677 "src/parser_bison.c"
    break;

  case 1094: /* payload_raw_expr: "@" payload_base_spec "comma" "number" "comma" payload_raw_len close_scope_at  */
#line 5616 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), NULL, 0);
				payload_init_raw((yyval.expr), (yyvsp[-5].val), (yyvsp[-3].val), (yyvsp[-1].val));
				(yyval.expr)->byteorder		= BYTEORDER_BIG_ENDIAN;
				(yyval.expr)->payload.is_raw	= true;
			}
#line 15688 "src/parser_bison.c"
    break;

  case 1095: /* payload_base_spec: "ll"  */
#line 5624 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_LL_HDR; }
#line 15694 "src/parser_bison.c"
    break;

  case 1096: /* payload_base_spec: "nh"  */
#line 5625 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_NETWORK_HDR; }
#line 15700 "src/parser_bison.c"
    break;

  case 1097: /* payload_base_spec: "th" close_scope_th  */
#line 5626 "src/parser_bison.y"
                                                                { (yyval.val) = PROTO_BASE_TRANSPORT_HDR; }
#line 15706 "src/parser_bison.c"
    break;

  case 1098: /* payload_base_spec: "string"  */
#line 5628 "src/parser_bison.y"
                        {
				if (!strcmp((yyvsp[0].string), "ih")) {
					(yyval.val) = PROTO_BASE_INNER_HDR;
				} else {
					erec_queue(error(&(yylsp[0]), "unknown raw payload base"), state->msgs);
					free_const((yyvsp[0].string));
					YYERROR;
				}
				free_const((yyvsp[0].string));
			}
#line 15721 "src/parser_bison.c"
    break;

  case 1099: /* eth_hdr_expr: "ether" eth_hdr_field close_scope_eth  */
#line 5641 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_eth, (yyvsp[-1].val));
			}
#line 15729 "src/parser_bison.c"
    break;

  case 1100: /* eth_hdr_field: "saddr"  */
#line 5646 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_SADDR; }
#line 15735 "src/parser_bison.c"
    break;

  case 1101: /* eth_hdr_field: "daddr"  */
#line 5647 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_DADDR; }
#line 15741 "src/parser_bison.c"
    break;

  case 1102: /* eth_hdr_field: "type" close_scope_type  */
#line 5648 "src/parser_bison.y"
                                                                        { (yyval.val) = ETHHDR_TYPE; }
#line 15747 "src/parser_bison.c"
    break;

  case 1103: /* vlan_hdr_expr: "vlan" vlan_hdr_field close_scope_vlan  */
#line 5652 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_vlan, (yyvsp[-1].val));
			}
#line 15755 "src/parser_bison.c"
    break;

  case 1104: /* vlan_hdr_field: "id"  */
#line 5657 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_VID; }
#line 15761 "src/parser_bison.c"
    break;

  case 1105: /* vlan_hdr_field: "cfi"  */
#line 5658 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_CFI; }
#line 15767 "src/parser_bison.c"
    break;

  case 1106: /* vlan_hdr_field: "dei"  */
#line 5659 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_DEI; }
#line 15773 "src/parser_bison.c"
    break;

  case 1107: /* vlan_hdr_field: "pcp"  */
#line 5660 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_PCP; }
#line 15779 "src/parser_bison.c"
    break;

  case 1108: /* vlan_hdr_field: "type" close_scope_type  */
#line 5661 "src/parser_bison.y"
                                                                        { (yyval.val) = VLANHDR_TYPE; }
#line 15785 "src/parser_bison.c"
    break;

  case 1109: /* arp_hdr_expr: "arp" arp_hdr_field close_scope_arp  */
#line 5665 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_arp, (yyvsp[-1].val));
			}
#line 15793 "src/parser_bison.c"
    break;

  case 1110: /* arp_hdr_field: "htype"  */
#line 5670 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HRD; }
#line 15799 "src/parser_bison.c"
    break;

  case 1111: /* arp_hdr_field: "ptype"  */
#line 5671 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PRO; }
#line 15805 "src/parser_bison.c"
    break;

  case 1112: /* arp_hdr_field: "hlen"  */
#line 5672 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HLN; }
#line 15811 "src/parser_bison.c"
    break;

  case 1113: /* arp_hdr_field: "plen"  */
#line 5673 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PLN; }
#line 15817 "src/parser_bison.c"
    break;

  case 1114: /* arp_hdr_field: "operation"  */
#line 5674 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_OP; }
#line 15823 "src/parser_bison.c"
    break;

  case 1115: /* arp_hdr_field: "saddr" "ether" close_scope_eth  */
#line 5675 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_ETHER; }
#line 15829 "src/parser_bison.c"
    break;

  case 1116: /* arp_hdr_field: "daddr" "ether" close_scope_eth  */
#line 5676 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_ETHER; }
#line 15835 "src/parser_bison.c"
    break;

  case 1117: /* arp_hdr_field: "saddr" "ip" close_scope_ip  */
#line 5677 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_IP; }
#line 15841 "src/parser_bison.c"
    break;

  case 1118: /* arp_hdr_field: "daddr" "ip" close_scope_ip  */
#line 5678 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_IP; }
#line 15847 "src/parser_bison.c"
    break;

  case 1119: /* ip_hdr_expr: "ip" ip_hdr_field close_scope_ip  */
#line 5682 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip, (yyvsp[-1].val));
			}
#line 15855 "src/parser_bison.c"
    break;

  case 1120: /* ip_hdr_expr: "ip" "option" ip_option_type ip_option_field close_scope_ip  */
#line 5686 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[-1].val));
				if (!(yyval.expr)) {
					erec_queue(error(&(yylsp[-4]), "unknown ip option type/field"), state->msgs);
					YYERROR;
				}

				if ((yyvsp[-1].val) == IPOPT_FIELD_TYPE)
					(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 15870 "src/parser_bison.c"
    break;

  case 1121: /* ip_hdr_expr: "ip" "option" ip_option_type close_scope_ip  */
#line 5697 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-1].val), IPOPT_FIELD_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 15879 "src/parser_bison.c"
    break;

  case 1122: /* ip_hdr_field: "version"  */
#line 5703 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_VERSION; }
#line 15885 "src/parser_bison.c"
    break;

  case 1123: /* ip_hdr_field: "hdrlength"  */
#line 5704 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_HDRLENGTH; }
#line 15891 "src/parser_bison.c"
    break;

  case 1124: /* ip_hdr_field: "dscp"  */
#line 5705 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DSCP; }
#line 15897 "src/parser_bison.c"
    break;

  case 1125: /* ip_hdr_field: "ecn"  */
#line 5706 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ECN; }
#line 15903 "src/parser_bison.c"
    break;

  case 1126: /* ip_hdr_field: "length"  */
#line 5707 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_LENGTH; }
#line 15909 "src/parser_bison.c"
    break;

  case 1127: /* ip_hdr_field: "id"  */
#line 5708 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ID; }
#line 15915 "src/parser_bison.c"
    break;

  case 1128: /* ip_hdr_field: "frag-off"  */
#line 5709 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_FRAG_OFF; }
#line 15921 "src/parser_bison.c"
    break;

  case 1129: /* ip_hdr_field: "ttl"  */
#line 5710 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_TTL; }
#line 15927 "src/parser_bison.c"
    break;

  case 1130: /* ip_hdr_field: "protocol"  */
#line 5711 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_PROTOCOL; }
#line 15933 "src/parser_bison.c"
    break;

  case 1131: /* ip_hdr_field: "checksum"  */
#line 5712 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_CHECKSUM; }
#line 15939 "src/parser_bison.c"
    break;

  case 1132: /* ip_hdr_field: "saddr"  */
#line 5713 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_SADDR; }
#line 15945 "src/parser_bison.c"
    break;

  case 1133: /* ip_hdr_field: "daddr"  */
#line 5714 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DADDR; }
#line 15951 "src/parser_bison.c"
    break;

  case 1134: /* ip_option_type: "lsrr"  */
#line 5717 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_LSRR; }
#line 15957 "src/parser_bison.c"
    break;

  case 1135: /* ip_option_type: "rr"  */
#line 5718 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RR; }
#line 15963 "src/parser_bison.c"
    break;

  case 1136: /* ip_option_type: "ssrr"  */
#line 5719 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_SSRR; }
#line 15969 "src/parser_bison.c"
    break;

  case 1137: /* ip_option_type: "ra"  */
#line 5720 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RA; }
#line 15975 "src/parser_bison.c"
    break;

  case 1138: /* ip_option_field: "type" close_scope_type  */
#line 5723 "src/parser_bison.y"
                                                                        { (yyval.val) = IPOPT_FIELD_TYPE; }
#line 15981 "src/parser_bison.c"
    break;

  case 1139: /* ip_option_field: "length"  */
#line 5724 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_LENGTH; }
#line 15987 "src/parser_bison.c"
    break;

  case 1140: /* ip_option_field: "value"  */
#line 5725 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_VALUE; }
#line 15993 "src/parser_bison.c"
    break;

  case 1141: /* ip_option_field: "ptr"  */
#line 5726 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_PTR; }
#line 15999 "src/parser_bison.c"
    break;

  case 1142: /* ip_option_field: "addr"  */
#line 5727 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_ADDR_0; }
#line 16005 "src/parser_bison.c"
    break;

  case 1143: /* icmp_hdr_expr: "icmp" icmp_hdr_field close_scope_icmp  */
#line 5731 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp, (yyvsp[-1].val));
			}
#line 16013 "src/parser_bison.c"
    break;

  case 1144: /* icmp_hdr_field: "type" close_scope_type  */
#line 5736 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMPHDR_TYPE; }
#line 16019 "src/parser_bison.c"
    break;

  case 1145: /* icmp_hdr_field: "code"  */
#line 5737 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CODE; }
#line 16025 "src/parser_bison.c"
    break;

  case 1146: /* icmp_hdr_field: "checksum"  */
#line 5738 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CHECKSUM; }
#line 16031 "src/parser_bison.c"
    break;

  case 1147: /* icmp_hdr_field: "id"  */
#line 5739 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_ID; }
#line 16037 "src/parser_bison.c"
    break;

  case 1148: /* icmp_hdr_field: "seq"  */
#line 5740 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_SEQ; }
#line 16043 "src/parser_bison.c"
    break;

  case 1149: /* icmp_hdr_field: "gateway"  */
#line 5741 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_GATEWAY; }
#line 16049 "src/parser_bison.c"
    break;

  case 1150: /* icmp_hdr_field: "mtu"  */
#line 5742 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_MTU; }
#line 16055 "src/parser_bison.c"
    break;

  case 1151: /* igmp_hdr_expr: "igmp" igmp_hdr_field close_scope_igmp  */
#line 5746 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_igmp, (yyvsp[-1].val));
			}
#line 16063 "src/parser_bison.c"
    break;

  case 1152: /* igmp_hdr_field: "type" close_scope_type  */
#line 5751 "src/parser_bison.y"
                                                                        { (yyval.val) = IGMPHDR_TYPE; }
#line 16069 "src/parser_bison.c"
    break;

  case 1153: /* igmp_hdr_field: "checksum"  */
#line 5752 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_CHECKSUM; }
#line 16075 "src/parser_bison.c"
    break;

  case 1154: /* igmp_hdr_field: "mrt"  */
#line 5753 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_MRT; }
#line 16081 "src/parser_bison.c"
    break;

  case 1155: /* igmp_hdr_field: "group"  */
#line 5754 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_GROUP; }
#line 16087 "src/parser_bison.c"
    break;

  case 1156: /* ip6_hdr_expr: "ip6" ip6_hdr_field close_scope_ip6  */
#line 5758 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip6, (yyvsp[-1].val));
			}
#line 16095 "src/parser_bison.c"
    break;

  case 1157: /* ip6_hdr_field: "version"  */
#line 5763 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_VERSION; }
#line 16101 "src/parser_bison.c"
    break;

  case 1158: /* ip6_hdr_field: "dscp"  */
#line 5764 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DSCP; }
#line 16107 "src/parser_bison.c"
    break;

  case 1159: /* ip6_hdr_field: "ecn"  */
#line 5765 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_ECN; }
#line 16113 "src/parser_bison.c"
    break;

  case 1160: /* ip6_hdr_field: "flowlabel"  */
#line 5766 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_FLOWLABEL; }
#line 16119 "src/parser_bison.c"
    break;

  case 1161: /* ip6_hdr_field: "length"  */
#line 5767 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_LENGTH; }
#line 16125 "src/parser_bison.c"
    break;

  case 1162: /* ip6_hdr_field: "nexthdr"  */
#line 5768 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_NEXTHDR; }
#line 16131 "src/parser_bison.c"
    break;

  case 1163: /* ip6_hdr_field: "hoplimit"  */
#line 5769 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_HOPLIMIT; }
#line 16137 "src/parser_bison.c"
    break;

  case 1164: /* ip6_hdr_field: "saddr"  */
#line 5770 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_SADDR; }
#line 16143 "src/parser_bison.c"
    break;

  case 1165: /* ip6_hdr_field: "daddr"  */
#line 5771 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DADDR; }
#line 16149 "src/parser_bison.c"
    break;

  case 1166: /* icmp6_hdr_expr: "icmpv6" icmp6_hdr_field close_scope_icmp  */
#line 5774 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp6, (yyvsp[-1].val));
			}
#line 16157 "src/parser_bison.c"
    break;

  case 1167: /* icmp6_hdr_field: "type" close_scope_type  */
#line 5779 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMP6HDR_TYPE; }
#line 16163 "src/parser_bison.c"
    break;

  case 1168: /* icmp6_hdr_field: "code"  */
#line 5780 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CODE; }
#line 16169 "src/parser_bison.c"
    break;

  case 1169: /* icmp6_hdr_field: "checksum"  */
#line 5781 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CHECKSUM; }
#line 16175 "src/parser_bison.c"
    break;

  case 1170: /* icmp6_hdr_field: "param-problem"  */
#line 5782 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_PPTR; }
#line 16181 "src/parser_bison.c"
    break;

  case 1171: /* icmp6_hdr_field: "mtu"  */
#line 5783 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MTU; }
#line 16187 "src/parser_bison.c"
    break;

  case 1172: /* icmp6_hdr_field: "id"  */
#line 5784 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_ID; }
#line 16193 "src/parser_bison.c"
    break;

  case 1173: /* icmp6_hdr_field: "seq"  */
#line 5785 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_SEQ; }
#line 16199 "src/parser_bison.c"
    break;

  case 1174: /* icmp6_hdr_field: "max-delay"  */
#line 5786 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MAXDELAY; }
#line 16205 "src/parser_bison.c"
    break;

  case 1175: /* icmp6_hdr_field: "taddr"  */
#line 5787 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_TADDR; }
#line 16211 "src/parser_bison.c"
    break;

  case 1176: /* icmp6_hdr_field: "daddr"  */
#line 5788 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_DADDR; }
#line 16217 "src/parser_bison.c"
    break;

  case 1177: /* auth_hdr_expr: "ah" auth_hdr_field close_scope_ah  */
#line 5792 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ah, (yyvsp[-1].val));
			}
#line 16225 "src/parser_bison.c"
    break;

  case 1178: /* auth_hdr_field: "nexthdr"  */
#line 5797 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_NEXTHDR; }
#line 16231 "src/parser_bison.c"
    break;

  case 1179: /* auth_hdr_field: "hdrlength"  */
#line 5798 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_HDRLENGTH; }
#line 16237 "src/parser_bison.c"
    break;

  case 1180: /* auth_hdr_field: "reserved"  */
#line 5799 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_RESERVED; }
#line 16243 "src/parser_bison.c"
    break;

  case 1181: /* auth_hdr_field: "spi"  */
#line 5800 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SPI; }
#line 16249 "src/parser_bison.c"
    break;

  case 1182: /* auth_hdr_field: "seq"  */
#line 5801 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SEQUENCE; }
#line 16255 "src/parser_bison.c"
    break;

  case 1183: /* esp_hdr_expr: "esp" esp_hdr_field close_scope_esp  */
#line 5805 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_esp, (yyvsp[-1].val));
			}
#line 16263 "src/parser_bison.c"
    break;

  case 1184: /* esp_hdr_field: "spi"  */
#line 5810 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SPI; }
#line 16269 "src/parser_bison.c"
    break;

  case 1185: /* esp_hdr_field: "seq"  */
#line 5811 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SEQUENCE; }
#line 16275 "src/parser_bison.c"
    break;

  case 1186: /* comp_hdr_expr: "comp" comp_hdr_field close_scope_comp  */
#line 5815 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_comp, (yyvsp[-1].val));
			}
#line 16283 "src/parser_bison.c"
    break;

  case 1187: /* comp_hdr_field: "nexthdr"  */
#line 5820 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_NEXTHDR; }
#line 16289 "src/parser_bison.c"
    break;

  case 1188: /* comp_hdr_field: "flags"  */
#line 5821 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_FLAGS; }
#line 16295 "src/parser_bison.c"
    break;

  case 1189: /* comp_hdr_field: "cpi"  */
#line 5822 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_CPI; }
#line 16301 "src/parser_bison.c"
    break;

  case 1190: /* udp_hdr_expr: "udp" udp_hdr_field close_scope_udp  */
#line 5826 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udp, (yyvsp[-1].val));
			}
#line 16309 "src/parser_bison.c"
    break;

  case 1191: /* udp_hdr_field: "sport"  */
#line 5831 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 16315 "src/parser_bison.c"
    break;

  case 1192: /* udp_hdr_field: "dport"  */
#line 5832 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 16321 "src/parser_bison.c"
    break;

  case 1193: /* udp_hdr_field: "length"  */
#line 5833 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 16327 "src/parser_bison.c"
    break;

  case 1194: /* udp_hdr_field: "checksum"  */
#line 5834 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 16333 "src/parser_bison.c"
    break;

  case 1195: /* udplite_hdr_expr: "udplite" udplite_hdr_field close_scope_udplite  */
#line 5838 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udplite, (yyvsp[-1].val));
			}
#line 16341 "src/parser_bison.c"
    break;

  case 1196: /* udplite_hdr_field: "sport"  */
#line 5843 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 16347 "src/parser_bison.c"
    break;

  case 1197: /* udplite_hdr_field: "dport"  */
#line 5844 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 16353 "src/parser_bison.c"
    break;

  case 1198: /* udplite_hdr_field: "csumcov"  */
#line 5845 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 16359 "src/parser_bison.c"
    break;

  case 1199: /* udplite_hdr_field: "checksum"  */
#line 5846 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 16365 "src/parser_bison.c"
    break;

  case 1200: /* tcp_hdr_expr: "tcp" tcp_hdr_field  */
#line 5850 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_tcp, (yyvsp[0].val));
			}
#line 16373 "src/parser_bison.c"
    break;

  case 1201: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_type  */
#line 5854 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].val), TCPOPT_COMMON_KIND);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 16382 "src/parser_bison.c"
    break;

  case 1202: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_kind_and_field  */
#line 5859 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].tcp_kind_field).kind, (yyvsp[0].tcp_kind_field).field);
				if ((yyval.expr) == NULL) {
					erec_queue(error(&(yylsp[-2]), "Could not find a tcp option template"), state->msgs);
					YYERROR;
				}
			}
#line 16394 "src/parser_bison.c"
    break;

  case 1203: /* tcp_hdr_expr: "tcp" "option" "@" close_scope_at tcp_hdr_option_type "comma" "number" "comma" payload_raw_len  */
#line 5867 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[-4].val), 0);
				tcpopt_init_raw((yyval.expr), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[0].val), 0);
			}
#line 16403 "src/parser_bison.c"
    break;

  case 1223: /* vxlan_hdr_expr: "vxlan" vxlan_hdr_field  */
#line 5899 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_vxlan, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_vxlan;
				(yyval.expr) = expr;
			}
#line 16415 "src/parser_bison.c"
    break;

  case 1224: /* vxlan_hdr_expr: "vxlan" inner_expr  */
#line 5907 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_vxlan;
			}
#line 16425 "src/parser_bison.c"
    break;

  case 1225: /* vxlan_hdr_field: "vni"  */
#line 5914 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_VNI; }
#line 16431 "src/parser_bison.c"
    break;

  case 1226: /* vxlan_hdr_field: "flags"  */
#line 5915 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_FLAGS; }
#line 16437 "src/parser_bison.c"
    break;

  case 1227: /* geneve_hdr_expr: "geneve" geneve_hdr_field  */
#line 5919 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_geneve, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_geneve;
				(yyval.expr) = expr;
			}
#line 16449 "src/parser_bison.c"
    break;

  case 1228: /* geneve_hdr_expr: "geneve" inner_expr  */
#line 5927 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_geneve;
			}
#line 16459 "src/parser_bison.c"
    break;

  case 1229: /* geneve_hdr_field: "vni"  */
#line 5934 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_VNI; }
#line 16465 "src/parser_bison.c"
    break;

  case 1230: /* geneve_hdr_field: "type"  */
#line 5935 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_TYPE; }
#line 16471 "src/parser_bison.c"
    break;

  case 1231: /* gre_hdr_expr: "gre" gre_hdr_field close_scope_gre  */
#line 5939 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_gre, (yyvsp[-1].val));
			}
#line 16479 "src/parser_bison.c"
    break;

  case 1232: /* gre_hdr_expr: "gre" close_scope_gre inner_inet_expr  */
#line 5943 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gre;
			}
#line 16488 "src/parser_bison.c"
    break;

  case 1233: /* gre_hdr_field: "version"  */
#line 5949 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_VERSION;	}
#line 16494 "src/parser_bison.c"
    break;

  case 1234: /* gre_hdr_field: "flags"  */
#line 5950 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_FLAGS; }
#line 16500 "src/parser_bison.c"
    break;

  case 1235: /* gre_hdr_field: "protocol"  */
#line 5951 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_PROTOCOL; }
#line 16506 "src/parser_bison.c"
    break;

  case 1236: /* gretap_hdr_expr: "gretap" close_scope_gre inner_expr  */
#line 5955 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gretap;
			}
#line 16515 "src/parser_bison.c"
    break;

  case 1237: /* optstrip_stmt: "reset" "tcp" "option" tcp_hdr_option_type close_scope_tcp  */
#line 5962 "src/parser_bison.y"
                        {
				(yyval.stmt) = optstrip_stmt_alloc(&(yyloc), tcpopt_expr_alloc(&(yyloc),
										(yyvsp[-1].val), TCPOPT_COMMON_KIND));
			}
#line 16524 "src/parser_bison.c"
    break;

  case 1238: /* tcp_hdr_field: "sport"  */
#line 5968 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SPORT; }
#line 16530 "src/parser_bison.c"
    break;

  case 1239: /* tcp_hdr_field: "dport"  */
#line 5969 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DPORT; }
#line 16536 "src/parser_bison.c"
    break;

  case 1240: /* tcp_hdr_field: "seq"  */
#line 5970 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SEQ; }
#line 16542 "src/parser_bison.c"
    break;

  case 1241: /* tcp_hdr_field: "ackseq"  */
#line 5971 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_ACKSEQ; }
#line 16548 "src/parser_bison.c"
    break;

  case 1242: /* tcp_hdr_field: "doff"  */
#line 5972 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DOFF; }
#line 16554 "src/parser_bison.c"
    break;

  case 1243: /* tcp_hdr_field: "reserved"  */
#line 5973 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_RESERVED; }
#line 16560 "src/parser_bison.c"
    break;

  case 1244: /* tcp_hdr_field: "flags"  */
#line 5974 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_FLAGS; }
#line 16566 "src/parser_bison.c"
    break;

  case 1245: /* tcp_hdr_field: "window"  */
#line 5975 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_WINDOW; }
#line 16572 "src/parser_bison.c"
    break;

  case 1246: /* tcp_hdr_field: "checksum"  */
#line 5976 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_CHECKSUM; }
#line 16578 "src/parser_bison.c"
    break;

  case 1247: /* tcp_hdr_field: "urgptr"  */
#line 5977 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_URGPTR; }
#line 16584 "src/parser_bison.c"
    break;

  case 1248: /* tcp_hdr_option_kind_and_field: "mss" tcpopt_field_maxseg  */
#line 5981 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MAXSEG, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16593 "src/parser_bison.c"
    break;

  case 1249: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_sack tcpopt_field_sack  */
#line 5986 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16602 "src/parser_bison.c"
    break;

  case 1250: /* tcp_hdr_option_kind_and_field: "window" tcpopt_field_window  */
#line 5991 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_WINDOW, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16611 "src/parser_bison.c"
    break;

  case 1251: /* tcp_hdr_option_kind_and_field: "timestamp" tcpopt_field_tsopt  */
#line 5996 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_TIMESTAMP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16620 "src/parser_bison.c"
    break;

  case 1252: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_type "length"  */
#line 6001 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = TCPOPT_COMMON_LENGTH };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16629 "src/parser_bison.c"
    break;

  case 1253: /* tcp_hdr_option_kind_and_field: "mptcp" tcpopt_field_mptcp  */
#line 6006 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MPTCP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 16638 "src/parser_bison.c"
    break;

  case 1254: /* tcp_hdr_option_sack: "sack"  */
#line 6012 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 16644 "src/parser_bison.c"
    break;

  case 1255: /* tcp_hdr_option_sack: "sack0"  */
#line 6013 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 16650 "src/parser_bison.c"
    break;

  case 1256: /* tcp_hdr_option_sack: "sack1"  */
#line 6014 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK1; }
#line 16656 "src/parser_bison.c"
    break;

  case 1257: /* tcp_hdr_option_sack: "sack2"  */
#line 6015 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK2; }
#line 16662 "src/parser_bison.c"
    break;

  case 1258: /* tcp_hdr_option_sack: "sack3"  */
#line 6016 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK3; }
#line 16668 "src/parser_bison.c"
    break;

  case 1259: /* tcp_hdr_option_type: "echo"  */
#line 6019 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_ECHO; }
#line 16674 "src/parser_bison.c"
    break;

  case 1260: /* tcp_hdr_option_type: "eol"  */
#line 6020 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_EOL; }
#line 16680 "src/parser_bison.c"
    break;

  case 1261: /* tcp_hdr_option_type: "fastopen"  */
#line 6021 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_FASTOPEN; }
#line 16686 "src/parser_bison.c"
    break;

  case 1262: /* tcp_hdr_option_type: "md5sig"  */
#line 6022 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MD5SIG; }
#line 16692 "src/parser_bison.c"
    break;

  case 1263: /* tcp_hdr_option_type: "mptcp"  */
#line 6023 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MPTCP; }
#line 16698 "src/parser_bison.c"
    break;

  case 1264: /* tcp_hdr_option_type: "mss"  */
#line 6024 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MAXSEG; }
#line 16704 "src/parser_bison.c"
    break;

  case 1265: /* tcp_hdr_option_type: "nop"  */
#line 6025 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_NOP; }
#line 16710 "src/parser_bison.c"
    break;

  case 1266: /* tcp_hdr_option_type: "sack-permitted"  */
#line 6026 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_SACK_PERMITTED; }
#line 16716 "src/parser_bison.c"
    break;

  case 1267: /* tcp_hdr_option_type: "timestamp"  */
#line 6027 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_TIMESTAMP; }
#line 16722 "src/parser_bison.c"
    break;

  case 1268: /* tcp_hdr_option_type: "window"  */
#line 6028 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_WINDOW; }
#line 16728 "src/parser_bison.c"
    break;

  case 1269: /* tcp_hdr_option_type: tcp_hdr_option_sack  */
#line 6029 "src/parser_bison.y"
                                                        { (yyval.val) = (yyvsp[0].val); }
#line 16734 "src/parser_bison.c"
    break;

  case 1270: /* tcp_hdr_option_type: "number"  */
#line 6030 "src/parser_bison.y"
                                                        {
				if ((yyvsp[0].val) > 255) {
					erec_queue(error(&(yylsp[0]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.val) = (yyvsp[0].val);
			}
#line 16746 "src/parser_bison.c"
    break;

  case 1271: /* tcpopt_field_sack: "left"  */
#line 6039 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_LEFT; }
#line 16752 "src/parser_bison.c"
    break;

  case 1272: /* tcpopt_field_sack: "right"  */
#line 6040 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_RIGHT; }
#line 16758 "src/parser_bison.c"
    break;

  case 1273: /* tcpopt_field_window: "count"  */
#line 6043 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_WINDOW_COUNT; }
#line 16764 "src/parser_bison.c"
    break;

  case 1274: /* tcpopt_field_tsopt: "tsval"  */
#line 6046 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSVAL; }
#line 16770 "src/parser_bison.c"
    break;

  case 1275: /* tcpopt_field_tsopt: "tsecr"  */
#line 6047 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSECR; }
#line 16776 "src/parser_bison.c"
    break;

  case 1276: /* tcpopt_field_maxseg: "size"  */
#line 6050 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MAXSEG_SIZE; }
#line 16782 "src/parser_bison.c"
    break;

  case 1277: /* tcpopt_field_mptcp: "subtype"  */
#line 6053 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MPTCP_SUBTYPE; }
#line 16788 "src/parser_bison.c"
    break;

  case 1278: /* dccp_hdr_expr: "dccp" dccp_hdr_field close_scope_dccp  */
#line 6057 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_dccp, (yyvsp[-1].val));
			}
#line 16796 "src/parser_bison.c"
    break;

  case 1279: /* dccp_hdr_expr: "dccp" "option" "number" close_scope_dccp  */
#line 6061 "src/parser_bison.y"
                        {
				if ((yyvsp[-1].val) > DCCPOPT_TYPE_MAX) {
					erec_queue(error(&(yylsp[-3]), "value too large"),
						   state->msgs);
					YYERROR;
				}
				(yyval.expr) = dccpopt_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 16809 "src/parser_bison.c"
    break;

  case 1280: /* dccp_hdr_field: "sport"  */
#line 6071 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_SPORT; }
#line 16815 "src/parser_bison.c"
    break;

  case 1281: /* dccp_hdr_field: "dport"  */
#line 6072 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_DPORT; }
#line 16821 "src/parser_bison.c"
    break;

  case 1282: /* dccp_hdr_field: "type" close_scope_type  */
#line 6073 "src/parser_bison.y"
                                                                        { (yyval.val) = DCCPHDR_TYPE; }
#line 16827 "src/parser_bison.c"
    break;

  case 1283: /* sctp_chunk_type: "data"  */
#line 6076 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_DATA; }
#line 16833 "src/parser_bison.c"
    break;

  case 1284: /* sctp_chunk_type: "init"  */
#line 6077 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT; }
#line 16839 "src/parser_bison.c"
    break;

  case 1285: /* sctp_chunk_type: "init-ack"  */
#line 6078 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT_ACK; }
#line 16845 "src/parser_bison.c"
    break;

  case 1286: /* sctp_chunk_type: "sack"  */
#line 6079 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SACK; }
#line 16851 "src/parser_bison.c"
    break;

  case 1287: /* sctp_chunk_type: "heartbeat"  */
#line 6080 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT; }
#line 16857 "src/parser_bison.c"
    break;

  case 1288: /* sctp_chunk_type: "heartbeat-ack"  */
#line 6081 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT_ACK; }
#line 16863 "src/parser_bison.c"
    break;

  case 1289: /* sctp_chunk_type: "abort"  */
#line 6082 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ABORT; }
#line 16869 "src/parser_bison.c"
    break;

  case 1290: /* sctp_chunk_type: "shutdown"  */
#line 6083 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN; }
#line 16875 "src/parser_bison.c"
    break;

  case 1291: /* sctp_chunk_type: "shutdown-ack"  */
#line 6084 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_ACK; }
#line 16881 "src/parser_bison.c"
    break;

  case 1292: /* sctp_chunk_type: "error"  */
#line 6085 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ERROR; }
#line 16887 "src/parser_bison.c"
    break;

  case 1293: /* sctp_chunk_type: "cookie-echo"  */
#line 6086 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ECHO; }
#line 16893 "src/parser_bison.c"
    break;

  case 1294: /* sctp_chunk_type: "cookie-ack"  */
#line 6087 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ACK; }
#line 16899 "src/parser_bison.c"
    break;

  case 1295: /* sctp_chunk_type: "ecne"  */
#line 6088 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ECNE; }
#line 16905 "src/parser_bison.c"
    break;

  case 1296: /* sctp_chunk_type: "cwr"  */
#line 6089 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_CWR; }
#line 16911 "src/parser_bison.c"
    break;

  case 1297: /* sctp_chunk_type: "shutdown-complete"  */
#line 6090 "src/parser_bison.y"
                                                  { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_COMPLETE; }
#line 16917 "src/parser_bison.c"
    break;

  case 1298: /* sctp_chunk_type: "asconf-ack"  */
#line 6091 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF_ACK; }
#line 16923 "src/parser_bison.c"
    break;

  case 1299: /* sctp_chunk_type: "forward-tsn"  */
#line 6092 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_FORWARD_TSN; }
#line 16929 "src/parser_bison.c"
    break;

  case 1300: /* sctp_chunk_type: "asconf"  */
#line 6093 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF; }
#line 16935 "src/parser_bison.c"
    break;

  case 1301: /* sctp_chunk_common_field: "type" close_scope_type  */
#line 6096 "src/parser_bison.y"
                                                                { (yyval.val) = SCTP_CHUNK_COMMON_TYPE; }
#line 16941 "src/parser_bison.c"
    break;

  case 1302: /* sctp_chunk_common_field: "flags"  */
#line 6097 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_FLAGS; }
#line 16947 "src/parser_bison.c"
    break;

  case 1303: /* sctp_chunk_common_field: "length"  */
#line 6098 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_LENGTH; }
#line 16953 "src/parser_bison.c"
    break;

  case 1304: /* sctp_chunk_data_field: "tsn"  */
#line 6101 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_TSN; }
#line 16959 "src/parser_bison.c"
    break;

  case 1305: /* sctp_chunk_data_field: "stream"  */
#line 6102 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_STREAM; }
#line 16965 "src/parser_bison.c"
    break;

  case 1306: /* sctp_chunk_data_field: "ssn"  */
#line 6103 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_SSN; }
#line 16971 "src/parser_bison.c"
    break;

  case 1307: /* sctp_chunk_data_field: "ppid"  */
#line 6104 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_PPID; }
#line 16977 "src/parser_bison.c"
    break;

  case 1308: /* sctp_chunk_init_field: "init-tag"  */
#line 6107 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TAG; }
#line 16983 "src/parser_bison.c"
    break;

  case 1309: /* sctp_chunk_init_field: "a-rwnd"  */
#line 6108 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_RWND; }
#line 16989 "src/parser_bison.c"
    break;

  case 1310: /* sctp_chunk_init_field: "num-outbound-streams"  */
#line 6109 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_OSTREAMS; }
#line 16995 "src/parser_bison.c"
    break;

  case 1311: /* sctp_chunk_init_field: "num-inbound-streams"  */
#line 6110 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_ISTREAMS; }
#line 17001 "src/parser_bison.c"
    break;

  case 1312: /* sctp_chunk_init_field: "initial-tsn"  */
#line 6111 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TSN; }
#line 17007 "src/parser_bison.c"
    break;

  case 1313: /* sctp_chunk_sack_field: "cum-tsn-ack"  */
#line 6114 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_CTSN_ACK; }
#line 17013 "src/parser_bison.c"
    break;

  case 1314: /* sctp_chunk_sack_field: "a-rwnd"  */
#line 6115 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_RWND; }
#line 17019 "src/parser_bison.c"
    break;

  case 1315: /* sctp_chunk_sack_field: "num-gap-ack-blocks"  */
#line 6116 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_GACK_BLOCKS; }
#line 17025 "src/parser_bison.c"
    break;

  case 1316: /* sctp_chunk_sack_field: "num-dup-tsns"  */
#line 6117 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_DUP_TSNS; }
#line 17031 "src/parser_bison.c"
    break;

  case 1317: /* sctp_chunk_alloc: sctp_chunk_type  */
#line 6121 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[0].val), SCTP_CHUNK_COMMON_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17040 "src/parser_bison.c"
    break;

  case 1318: /* sctp_chunk_alloc: sctp_chunk_type sctp_chunk_common_field  */
#line 6126 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[0].val));
			}
#line 17048 "src/parser_bison.c"
    break;

  case 1319: /* sctp_chunk_alloc: "data" sctp_chunk_data_field  */
#line 6130 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_DATA, (yyvsp[0].val));
			}
#line 17056 "src/parser_bison.c"
    break;

  case 1320: /* sctp_chunk_alloc: "init" sctp_chunk_init_field  */
#line 6134 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT, (yyvsp[0].val));
			}
#line 17064 "src/parser_bison.c"
    break;

  case 1321: /* sctp_chunk_alloc: "init-ack" sctp_chunk_init_field  */
#line 6138 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT_ACK, (yyvsp[0].val));
			}
#line 17072 "src/parser_bison.c"
    break;

  case 1322: /* sctp_chunk_alloc: "sack" sctp_chunk_sack_field  */
#line 6142 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SACK, (yyvsp[0].val));
			}
#line 17080 "src/parser_bison.c"
    break;

  case 1323: /* sctp_chunk_alloc: "shutdown" "cum-tsn-ack"  */
#line 6146 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SHUTDOWN,
							   SCTP_CHUNK_SHUTDOWN_CTSN_ACK);
			}
#line 17089 "src/parser_bison.c"
    break;

  case 1324: /* sctp_chunk_alloc: "ecne" "lowest-tsn"  */
#line 6151 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ECNE,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 17098 "src/parser_bison.c"
    break;

  case 1325: /* sctp_chunk_alloc: "cwr" "lowest-tsn"  */
#line 6156 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_CWR,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 17107 "src/parser_bison.c"
    break;

  case 1326: /* sctp_chunk_alloc: "asconf-ack" "seqno"  */
#line 6161 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF_ACK,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 17116 "src/parser_bison.c"
    break;

  case 1327: /* sctp_chunk_alloc: "forward-tsn" "new-cum-tsn"  */
#line 6166 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_FORWARD_TSN,
							   SCTP_CHUNK_FORWARD_TSN_NCTSN);
			}
#line 17125 "src/parser_bison.c"
    break;

  case 1328: /* sctp_chunk_alloc: "asconf" "seqno"  */
#line 6171 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 17134 "src/parser_bison.c"
    break;

  case 1329: /* sctp_hdr_expr: "sctp" sctp_hdr_field close_scope_sctp  */
#line 6178 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_sctp, (yyvsp[-1].val));
			}
#line 17142 "src/parser_bison.c"
    break;

  case 1330: /* sctp_hdr_expr: "sctp" "chunk" sctp_chunk_alloc close_scope_sctp_chunk close_scope_sctp  */
#line 6182 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 17150 "src/parser_bison.c"
    break;

  case 1331: /* sctp_hdr_field: "sport"  */
#line 6187 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_SPORT; }
#line 17156 "src/parser_bison.c"
    break;

  case 1332: /* sctp_hdr_field: "dport"  */
#line 6188 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_DPORT; }
#line 17162 "src/parser_bison.c"
    break;

  case 1333: /* sctp_hdr_field: "vtag"  */
#line 6189 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_VTAG; }
#line 17168 "src/parser_bison.c"
    break;

  case 1334: /* sctp_hdr_field: "checksum"  */
#line 6190 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_CHECKSUM; }
#line 17174 "src/parser_bison.c"
    break;

  case 1335: /* th_hdr_expr: "th" th_hdr_field close_scope_th  */
#line 6194 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_th, (yyvsp[-1].val));
				if ((yyval.expr))
					(yyval.expr)->payload.is_raw = true;
			}
#line 17184 "src/parser_bison.c"
    break;

  case 1336: /* th_hdr_field: "sport"  */
#line 6201 "src/parser_bison.y"
                                                { (yyval.val) = THDR_SPORT; }
#line 17190 "src/parser_bison.c"
    break;

  case 1337: /* th_hdr_field: "dport"  */
#line 6202 "src/parser_bison.y"
                                                { (yyval.val) = THDR_DPORT; }
#line 17196 "src/parser_bison.c"
    break;

  case 1346: /* hbh_hdr_expr: "hbh" hbh_hdr_field close_scope_hbh  */
#line 6216 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_hbh, (yyvsp[-1].val));
			}
#line 17204 "src/parser_bison.c"
    break;

  case 1347: /* hbh_hdr_field: "nexthdr"  */
#line 6221 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_NEXTHDR; }
#line 17210 "src/parser_bison.c"
    break;

  case 1348: /* hbh_hdr_field: "hdrlength"  */
#line 6222 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_HDRLENGTH; }
#line 17216 "src/parser_bison.c"
    break;

  case 1349: /* rt_hdr_expr: "rt" rt_hdr_field close_scope_rt  */
#line 6226 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt, (yyvsp[-1].val));
			}
#line 17224 "src/parser_bison.c"
    break;

  case 1350: /* rt_hdr_field: "nexthdr"  */
#line 6231 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_NEXTHDR; }
#line 17230 "src/parser_bison.c"
    break;

  case 1351: /* rt_hdr_field: "hdrlength"  */
#line 6232 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_HDRLENGTH; }
#line 17236 "src/parser_bison.c"
    break;

  case 1352: /* rt_hdr_field: "type" close_scope_type  */
#line 6233 "src/parser_bison.y"
                                                                        { (yyval.val) = RTHDR_TYPE; }
#line 17242 "src/parser_bison.c"
    break;

  case 1353: /* rt_hdr_field: "seg-left"  */
#line 6234 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_SEG_LEFT; }
#line 17248 "src/parser_bison.c"
    break;

  case 1354: /* rt0_hdr_expr: "rt0" rt0_hdr_field close_scope_rt  */
#line 6238 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt0, (yyvsp[-1].val));
			}
#line 17256 "src/parser_bison.c"
    break;

  case 1355: /* rt0_hdr_field: "addr" '[' "number" ']'  */
#line 6244 "src/parser_bison.y"
                        {
				(yyval.val) = RT0HDR_ADDR_1 + (yyvsp[-1].val) - 1;
			}
#line 17264 "src/parser_bison.c"
    break;

  case 1356: /* rt2_hdr_expr: "rt2" rt2_hdr_field close_scope_rt  */
#line 6250 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt2, (yyvsp[-1].val));
			}
#line 17272 "src/parser_bison.c"
    break;

  case 1357: /* rt2_hdr_field: "addr"  */
#line 6255 "src/parser_bison.y"
                                                { (yyval.val) = RT2HDR_ADDR; }
#line 17278 "src/parser_bison.c"
    break;

  case 1358: /* rt4_hdr_expr: "srh" rt4_hdr_field close_scope_rt  */
#line 6259 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt4, (yyvsp[-1].val));
			}
#line 17286 "src/parser_bison.c"
    break;

  case 1359: /* rt4_hdr_field: "last-entry"  */
#line 6264 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_LASTENT; }
#line 17292 "src/parser_bison.c"
    break;

  case 1360: /* rt4_hdr_field: "flags"  */
#line 6265 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_FLAGS; }
#line 17298 "src/parser_bison.c"
    break;

  case 1361: /* rt4_hdr_field: "tag"  */
#line 6266 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_TAG; }
#line 17304 "src/parser_bison.c"
    break;

  case 1362: /* rt4_hdr_field: "sid" '[' "number" ']'  */
#line 6268 "src/parser_bison.y"
                        {
				(yyval.val) = RT4HDR_SID_1 + (yyvsp[-1].val) - 1;
			}
#line 17312 "src/parser_bison.c"
    break;

  case 1363: /* frag_hdr_expr: "frag" frag_hdr_field close_scope_frag  */
#line 6274 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_frag, (yyvsp[-1].val));
			}
#line 17320 "src/parser_bison.c"
    break;

  case 1364: /* frag_hdr_field: "nexthdr"  */
#line 6279 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_NEXTHDR; }
#line 17326 "src/parser_bison.c"
    break;

  case 1365: /* frag_hdr_field: "reserved"  */
#line 6280 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED; }
#line 17332 "src/parser_bison.c"
    break;

  case 1366: /* frag_hdr_field: "frag-off"  */
#line 6281 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_FRAG_OFF; }
#line 17338 "src/parser_bison.c"
    break;

  case 1367: /* frag_hdr_field: "reserved2"  */
#line 6282 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED2; }
#line 17344 "src/parser_bison.c"
    break;

  case 1368: /* frag_hdr_field: "more-fragments"  */
#line 6283 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_MFRAGS; }
#line 17350 "src/parser_bison.c"
    break;

  case 1369: /* frag_hdr_field: "id"  */
#line 6284 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_ID; }
#line 17356 "src/parser_bison.c"
    break;

  case 1370: /* dst_hdr_expr: "dst" dst_hdr_field close_scope_dst  */
#line 6288 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_dst, (yyvsp[-1].val));
			}
#line 17364 "src/parser_bison.c"
    break;

  case 1371: /* dst_hdr_field: "nexthdr"  */
#line 6293 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_NEXTHDR; }
#line 17370 "src/parser_bison.c"
    break;

  case 1372: /* dst_hdr_field: "hdrlength"  */
#line 6294 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_HDRLENGTH; }
#line 17376 "src/parser_bison.c"
    break;

  case 1373: /* mh_hdr_expr: "mh" mh_hdr_field close_scope_mh  */
#line 6298 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_mh, (yyvsp[-1].val));
			}
#line 17384 "src/parser_bison.c"
    break;

  case 1374: /* mh_hdr_field: "nexthdr"  */
#line 6303 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_NEXTHDR; }
#line 17390 "src/parser_bison.c"
    break;

  case 1375: /* mh_hdr_field: "hdrlength"  */
#line 6304 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_HDRLENGTH; }
#line 17396 "src/parser_bison.c"
    break;

  case 1376: /* mh_hdr_field: "type" close_scope_type  */
#line 6305 "src/parser_bison.y"
                                                                        { (yyval.val) = MHHDR_TYPE; }
#line 17402 "src/parser_bison.c"
    break;

  case 1377: /* mh_hdr_field: "reserved"  */
#line 6306 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_RESERVED; }
#line 17408 "src/parser_bison.c"
    break;

  case 1378: /* mh_hdr_field: "checksum"  */
#line 6307 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_CHECKSUM; }
#line 17414 "src/parser_bison.c"
    break;

  case 1379: /* exthdr_exists_expr: "exthdr" exthdr_key  */
#line 6311 "src/parser_bison.y"
                        {
				const struct exthdr_desc *desc;

				desc = exthdr_find_proto((yyvsp[0].val));

				/* Assume that NEXTHDR template is always
				 * the first one in list of templates.
				 */
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), desc, 1);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17430 "src/parser_bison.c"
    break;

  case 1380: /* exthdr_key: "hbh" close_scope_hbh  */
#line 6324 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_HOPOPTS; }
#line 17436 "src/parser_bison.c"
    break;

  case 1381: /* exthdr_key: "rt" close_scope_rt  */
#line 6325 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_ROUTING; }
#line 17442 "src/parser_bison.c"
    break;

  case 1382: /* exthdr_key: "frag" close_scope_frag  */
#line 6326 "src/parser_bison.y"
                                                                { (yyval.val) = IPPROTO_FRAGMENT; }
#line 17448 "src/parser_bison.c"
    break;

  case 1383: /* exthdr_key: "dst" close_scope_dst  */
#line 6327 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_DSTOPTS; }
#line 17454 "src/parser_bison.c"
    break;

  case 1384: /* exthdr_key: "mh" close_scope_mh  */
#line 6328 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_MH; }
#line 17460 "src/parser_bison.c"
    break;


#line 17464 "src/parser_bison.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, nft, scanner, state, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= TOKEN_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == TOKEN_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, nft, scanner, state);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, nft, scanner, state);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, nft, scanner, state, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, nft, scanner, state);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, nft, scanner, state);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 6331 "src/parser_bison.y"


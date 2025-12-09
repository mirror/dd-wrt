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
		concat_expr_add(expr, expr_l);
	} else {
		location_update(&expr_r->location, loc_rhs, 2);

		expr = expr_l;
		expr->location = *loc;
	}

	concat_expr_add(expr, expr_r);
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

static bool tunnel_set_type(const struct location *loc,
			    struct obj *obj, enum tunnel_type type, const char *name,
			    struct parser_state *state)
{
	if (obj->tunnel.type) {
		erec_queue(error(loc, "Cannot create new %s section inside another tunnel", name), state->msgs);
		return false;
	}

	obj->tunnel.type = type;
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

#line 280 "src/parser_bison.c"

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

#line 1111 "src/parser_bison.c"

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
  YYSYMBOL_PATH = 62,                      /* "path"  */
  YYSYMBOL_INET = 63,                      /* "inet"  */
  YYSYMBOL_NETDEV = 64,                    /* "netdev"  */
  YYSYMBOL_ADD = 65,                       /* "add"  */
  YYSYMBOL_UPDATE = 66,                    /* "update"  */
  YYSYMBOL_REPLACE = 67,                   /* "replace"  */
  YYSYMBOL_CREATE = 68,                    /* "create"  */
  YYSYMBOL_INSERT = 69,                    /* "insert"  */
  YYSYMBOL_DELETE = 70,                    /* "delete"  */
  YYSYMBOL_GET = 71,                       /* "get"  */
  YYSYMBOL_LIST = 72,                      /* "list"  */
  YYSYMBOL_RESET = 73,                     /* "reset"  */
  YYSYMBOL_FLUSH = 74,                     /* "flush"  */
  YYSYMBOL_RENAME = 75,                    /* "rename"  */
  YYSYMBOL_DESCRIBE = 76,                  /* "describe"  */
  YYSYMBOL_IMPORT = 77,                    /* "import"  */
  YYSYMBOL_EXPORT = 78,                    /* "export"  */
  YYSYMBOL_DESTROY = 79,                   /* "destroy"  */
  YYSYMBOL_MONITOR = 80,                   /* "monitor"  */
  YYSYMBOL_ALL = 81,                       /* "all"  */
  YYSYMBOL_ACCEPT = 82,                    /* "accept"  */
  YYSYMBOL_DROP = 83,                      /* "drop"  */
  YYSYMBOL_CONTINUE = 84,                  /* "continue"  */
  YYSYMBOL_JUMP = 85,                      /* "jump"  */
  YYSYMBOL_GOTO = 86,                      /* "goto"  */
  YYSYMBOL_RETURN = 87,                    /* "return"  */
  YYSYMBOL_TO = 88,                        /* "to"  */
  YYSYMBOL_CONSTANT = 89,                  /* "constant"  */
  YYSYMBOL_INTERVAL = 90,                  /* "interval"  */
  YYSYMBOL_DYNAMIC = 91,                   /* "dynamic"  */
  YYSYMBOL_AUTOMERGE = 92,                 /* "auto-merge"  */
  YYSYMBOL_TIMEOUT = 93,                   /* "timeout"  */
  YYSYMBOL_GC_INTERVAL = 94,               /* "gc-interval"  */
  YYSYMBOL_ELEMENTS = 95,                  /* "elements"  */
  YYSYMBOL_EXPIRES = 96,                   /* "expires"  */
  YYSYMBOL_POLICY = 97,                    /* "policy"  */
  YYSYMBOL_MEMORY = 98,                    /* "memory"  */
  YYSYMBOL_PERFORMANCE = 99,               /* "performance"  */
  YYSYMBOL_SIZE = 100,                     /* "size"  */
  YYSYMBOL_FLOW = 101,                     /* "flow"  */
  YYSYMBOL_OFFLOAD = 102,                  /* "offload"  */
  YYSYMBOL_METER = 103,                    /* "meter"  */
  YYSYMBOL_METERS = 104,                   /* "meters"  */
  YYSYMBOL_FLOWTABLES = 105,               /* "flowtables"  */
  YYSYMBOL_NUM = 106,                      /* "number"  */
  YYSYMBOL_STRING = 107,                   /* "string"  */
  YYSYMBOL_QUOTED_STRING = 108,            /* "quoted string"  */
  YYSYMBOL_ASTERISK_STRING = 109,          /* "string with a trailing asterisk"  */
  YYSYMBOL_LL_HDR = 110,                   /* "ll"  */
  YYSYMBOL_NETWORK_HDR = 111,              /* "nh"  */
  YYSYMBOL_TRANSPORT_HDR = 112,            /* "th"  */
  YYSYMBOL_BRIDGE = 113,                   /* "bridge"  */
  YYSYMBOL_ETHER = 114,                    /* "ether"  */
  YYSYMBOL_SADDR = 115,                    /* "saddr"  */
  YYSYMBOL_DADDR = 116,                    /* "daddr"  */
  YYSYMBOL_TYPE = 117,                     /* "type"  */
  YYSYMBOL_VLAN = 118,                     /* "vlan"  */
  YYSYMBOL_ID = 119,                       /* "id"  */
  YYSYMBOL_CFI = 120,                      /* "cfi"  */
  YYSYMBOL_DEI = 121,                      /* "dei"  */
  YYSYMBOL_PCP = 122,                      /* "pcp"  */
  YYSYMBOL_ARP = 123,                      /* "arp"  */
  YYSYMBOL_HTYPE = 124,                    /* "htype"  */
  YYSYMBOL_PTYPE = 125,                    /* "ptype"  */
  YYSYMBOL_HLEN = 126,                     /* "hlen"  */
  YYSYMBOL_PLEN = 127,                     /* "plen"  */
  YYSYMBOL_OPERATION = 128,                /* "operation"  */
  YYSYMBOL_IP = 129,                       /* "ip"  */
  YYSYMBOL_HDRVERSION = 130,               /* "version"  */
  YYSYMBOL_HDRLENGTH = 131,                /* "hdrlength"  */
  YYSYMBOL_DSCP = 132,                     /* "dscp"  */
  YYSYMBOL_ECN = 133,                      /* "ecn"  */
  YYSYMBOL_LENGTH = 134,                   /* "length"  */
  YYSYMBOL_FRAG_OFF = 135,                 /* "frag-off"  */
  YYSYMBOL_TTL = 136,                      /* "ttl"  */
  YYSYMBOL_TOS = 137,                      /* "tos"  */
  YYSYMBOL_PROTOCOL = 138,                 /* "protocol"  */
  YYSYMBOL_CHECKSUM = 139,                 /* "checksum"  */
  YYSYMBOL_PTR = 140,                      /* "ptr"  */
  YYSYMBOL_VALUE = 141,                    /* "value"  */
  YYSYMBOL_LSRR = 142,                     /* "lsrr"  */
  YYSYMBOL_RR = 143,                       /* "rr"  */
  YYSYMBOL_SSRR = 144,                     /* "ssrr"  */
  YYSYMBOL_RA = 145,                       /* "ra"  */
  YYSYMBOL_ICMP = 146,                     /* "icmp"  */
  YYSYMBOL_CODE = 147,                     /* "code"  */
  YYSYMBOL_SEQUENCE = 148,                 /* "seq"  */
  YYSYMBOL_GATEWAY = 149,                  /* "gateway"  */
  YYSYMBOL_MTU = 150,                      /* "mtu"  */
  YYSYMBOL_IGMP = 151,                     /* "igmp"  */
  YYSYMBOL_MRT = 152,                      /* "mrt"  */
  YYSYMBOL_OPTIONS = 153,                  /* "options"  */
  YYSYMBOL_IP6 = 154,                      /* "ip6"  */
  YYSYMBOL_PRIORITY = 155,                 /* "priority"  */
  YYSYMBOL_FLOWLABEL = 156,                /* "flowlabel"  */
  YYSYMBOL_NEXTHDR = 157,                  /* "nexthdr"  */
  YYSYMBOL_HOPLIMIT = 158,                 /* "hoplimit"  */
  YYSYMBOL_ICMP6 = 159,                    /* "icmpv6"  */
  YYSYMBOL_PPTR = 160,                     /* "param-problem"  */
  YYSYMBOL_MAXDELAY = 161,                 /* "max-delay"  */
  YYSYMBOL_TADDR = 162,                    /* "taddr"  */
  YYSYMBOL_AH = 163,                       /* "ah"  */
  YYSYMBOL_RESERVED = 164,                 /* "reserved"  */
  YYSYMBOL_SPI = 165,                      /* "spi"  */
  YYSYMBOL_ESP = 166,                      /* "esp"  */
  YYSYMBOL_COMP = 167,                     /* "comp"  */
  YYSYMBOL_FLAGS = 168,                    /* "flags"  */
  YYSYMBOL_CPI = 169,                      /* "cpi"  */
  YYSYMBOL_PORT = 170,                     /* "port"  */
  YYSYMBOL_UDP = 171,                      /* "udp"  */
  YYSYMBOL_SPORT = 172,                    /* "sport"  */
  YYSYMBOL_DPORT = 173,                    /* "dport"  */
  YYSYMBOL_UDPLITE = 174,                  /* "udplite"  */
  YYSYMBOL_CSUMCOV = 175,                  /* "csumcov"  */
  YYSYMBOL_TCP = 176,                      /* "tcp"  */
  YYSYMBOL_ACKSEQ = 177,                   /* "ackseq"  */
  YYSYMBOL_DOFF = 178,                     /* "doff"  */
  YYSYMBOL_WINDOW = 179,                   /* "window"  */
  YYSYMBOL_URGPTR = 180,                   /* "urgptr"  */
  YYSYMBOL_OPTION = 181,                   /* "option"  */
  YYSYMBOL_ECHO = 182,                     /* "echo"  */
  YYSYMBOL_EOL = 183,                      /* "eol"  */
  YYSYMBOL_MPTCP = 184,                    /* "mptcp"  */
  YYSYMBOL_NOP = 185,                      /* "nop"  */
  YYSYMBOL_SACK = 186,                     /* "sack"  */
  YYSYMBOL_SACK0 = 187,                    /* "sack0"  */
  YYSYMBOL_SACK1 = 188,                    /* "sack1"  */
  YYSYMBOL_SACK2 = 189,                    /* "sack2"  */
  YYSYMBOL_SACK3 = 190,                    /* "sack3"  */
  YYSYMBOL_SACK_PERM = 191,                /* "sack-permitted"  */
  YYSYMBOL_FASTOPEN = 192,                 /* "fastopen"  */
  YYSYMBOL_MD5SIG = 193,                   /* "md5sig"  */
  YYSYMBOL_TIMESTAMP = 194,                /* "timestamp"  */
  YYSYMBOL_COUNT = 195,                    /* "count"  */
  YYSYMBOL_LEFT = 196,                     /* "left"  */
  YYSYMBOL_RIGHT = 197,                    /* "right"  */
  YYSYMBOL_TSVAL = 198,                    /* "tsval"  */
  YYSYMBOL_TSECR = 199,                    /* "tsecr"  */
  YYSYMBOL_SUBTYPE = 200,                  /* "subtype"  */
  YYSYMBOL_DCCP = 201,                     /* "dccp"  */
  YYSYMBOL_VXLAN = 202,                    /* "vxlan"  */
  YYSYMBOL_VNI = 203,                      /* "vni"  */
  YYSYMBOL_GRE = 204,                      /* "gre"  */
  YYSYMBOL_GRETAP = 205,                   /* "gretap"  */
  YYSYMBOL_GENEVE = 206,                   /* "geneve"  */
  YYSYMBOL_SCTP = 207,                     /* "sctp"  */
  YYSYMBOL_CHUNK = 208,                    /* "chunk"  */
  YYSYMBOL_DATA = 209,                     /* "data"  */
  YYSYMBOL_INIT = 210,                     /* "init"  */
  YYSYMBOL_INIT_ACK = 211,                 /* "init-ack"  */
  YYSYMBOL_HEARTBEAT = 212,                /* "heartbeat"  */
  YYSYMBOL_HEARTBEAT_ACK = 213,            /* "heartbeat-ack"  */
  YYSYMBOL_ABORT = 214,                    /* "abort"  */
  YYSYMBOL_SHUTDOWN = 215,                 /* "shutdown"  */
  YYSYMBOL_SHUTDOWN_ACK = 216,             /* "shutdown-ack"  */
  YYSYMBOL_ERROR = 217,                    /* "error"  */
  YYSYMBOL_COOKIE_ECHO = 218,              /* "cookie-echo"  */
  YYSYMBOL_COOKIE_ACK = 219,               /* "cookie-ack"  */
  YYSYMBOL_ECNE = 220,                     /* "ecne"  */
  YYSYMBOL_CWR = 221,                      /* "cwr"  */
  YYSYMBOL_SHUTDOWN_COMPLETE = 222,        /* "shutdown-complete"  */
  YYSYMBOL_ASCONF_ACK = 223,               /* "asconf-ack"  */
  YYSYMBOL_FORWARD_TSN = 224,              /* "forward-tsn"  */
  YYSYMBOL_ASCONF = 225,                   /* "asconf"  */
  YYSYMBOL_TSN = 226,                      /* "tsn"  */
  YYSYMBOL_STREAM = 227,                   /* "stream"  */
  YYSYMBOL_SSN = 228,                      /* "ssn"  */
  YYSYMBOL_PPID = 229,                     /* "ppid"  */
  YYSYMBOL_INIT_TAG = 230,                 /* "init-tag"  */
  YYSYMBOL_A_RWND = 231,                   /* "a-rwnd"  */
  YYSYMBOL_NUM_OSTREAMS = 232,             /* "num-outbound-streams"  */
  YYSYMBOL_NUM_ISTREAMS = 233,             /* "num-inbound-streams"  */
  YYSYMBOL_INIT_TSN = 234,                 /* "initial-tsn"  */
  YYSYMBOL_CUM_TSN_ACK = 235,              /* "cum-tsn-ack"  */
  YYSYMBOL_NUM_GACK_BLOCKS = 236,          /* "num-gap-ack-blocks"  */
  YYSYMBOL_NUM_DUP_TSNS = 237,             /* "num-dup-tsns"  */
  YYSYMBOL_LOWEST_TSN = 238,               /* "lowest-tsn"  */
  YYSYMBOL_SEQNO = 239,                    /* "seqno"  */
  YYSYMBOL_NEW_CUM_TSN = 240,              /* "new-cum-tsn"  */
  YYSYMBOL_VTAG = 241,                     /* "vtag"  */
  YYSYMBOL_RT = 242,                       /* "rt"  */
  YYSYMBOL_RT0 = 243,                      /* "rt0"  */
  YYSYMBOL_RT2 = 244,                      /* "rt2"  */
  YYSYMBOL_RT4 = 245,                      /* "srh"  */
  YYSYMBOL_SEG_LEFT = 246,                 /* "seg-left"  */
  YYSYMBOL_ADDR = 247,                     /* "addr"  */
  YYSYMBOL_LAST_ENT = 248,                 /* "last-entry"  */
  YYSYMBOL_TAG = 249,                      /* "tag"  */
  YYSYMBOL_SID = 250,                      /* "sid"  */
  YYSYMBOL_HBH = 251,                      /* "hbh"  */
  YYSYMBOL_FRAG = 252,                     /* "frag"  */
  YYSYMBOL_RESERVED2 = 253,                /* "reserved2"  */
  YYSYMBOL_MORE_FRAGMENTS = 254,           /* "more-fragments"  */
  YYSYMBOL_DST = 255,                      /* "dst"  */
  YYSYMBOL_MH = 256,                       /* "mh"  */
  YYSYMBOL_META = 257,                     /* "meta"  */
  YYSYMBOL_MARK = 258,                     /* "mark"  */
  YYSYMBOL_IIF = 259,                      /* "iif"  */
  YYSYMBOL_IIFNAME = 260,                  /* "iifname"  */
  YYSYMBOL_IIFTYPE = 261,                  /* "iiftype"  */
  YYSYMBOL_OIF = 262,                      /* "oif"  */
  YYSYMBOL_OIFNAME = 263,                  /* "oifname"  */
  YYSYMBOL_OIFTYPE = 264,                  /* "oiftype"  */
  YYSYMBOL_SKUID = 265,                    /* "skuid"  */
  YYSYMBOL_SKGID = 266,                    /* "skgid"  */
  YYSYMBOL_NFTRACE = 267,                  /* "nftrace"  */
  YYSYMBOL_RTCLASSID = 268,                /* "rtclassid"  */
  YYSYMBOL_IBRIPORT = 269,                 /* "ibriport"  */
  YYSYMBOL_OBRIPORT = 270,                 /* "obriport"  */
  YYSYMBOL_IBRIDGENAME = 271,              /* "ibrname"  */
  YYSYMBOL_OBRIDGENAME = 272,              /* "obrname"  */
  YYSYMBOL_PKTTYPE = 273,                  /* "pkttype"  */
  YYSYMBOL_CPU = 274,                      /* "cpu"  */
  YYSYMBOL_IIFGROUP = 275,                 /* "iifgroup"  */
  YYSYMBOL_OIFGROUP = 276,                 /* "oifgroup"  */
  YYSYMBOL_CGROUP = 277,                   /* "cgroup"  */
  YYSYMBOL_TIME = 278,                     /* "time"  */
  YYSYMBOL_CLASSID = 279,                  /* "classid"  */
  YYSYMBOL_NEXTHOP = 280,                  /* "nexthop"  */
  YYSYMBOL_CT = 281,                       /* "ct"  */
  YYSYMBOL_L3PROTOCOL = 282,               /* "l3proto"  */
  YYSYMBOL_PROTO_SRC = 283,                /* "proto-src"  */
  YYSYMBOL_PROTO_DST = 284,                /* "proto-dst"  */
  YYSYMBOL_ZONE = 285,                     /* "zone"  */
  YYSYMBOL_DIRECTION = 286,                /* "direction"  */
  YYSYMBOL_EVENT = 287,                    /* "event"  */
  YYSYMBOL_EXPECTATION = 288,              /* "expectation"  */
  YYSYMBOL_EXPIRATION = 289,               /* "expiration"  */
  YYSYMBOL_HELPER = 290,                   /* "helper"  */
  YYSYMBOL_LABEL = 291,                    /* "label"  */
  YYSYMBOL_STATE = 292,                    /* "state"  */
  YYSYMBOL_STATUS = 293,                   /* "status"  */
  YYSYMBOL_ORIGINAL = 294,                 /* "original"  */
  YYSYMBOL_REPLY = 295,                    /* "reply"  */
  YYSYMBOL_COUNTER = 296,                  /* "counter"  */
  YYSYMBOL_NAME = 297,                     /* "name"  */
  YYSYMBOL_PACKETS = 298,                  /* "packets"  */
  YYSYMBOL_BYTES = 299,                    /* "bytes"  */
  YYSYMBOL_AVGPKT = 300,                   /* "avgpkt"  */
  YYSYMBOL_LAST = 301,                     /* "last"  */
  YYSYMBOL_NEVER = 302,                    /* "never"  */
  YYSYMBOL_TUNNEL = 303,                   /* "tunnel"  */
  YYSYMBOL_ERSPAN = 304,                   /* "erspan"  */
  YYSYMBOL_EGRESS = 305,                   /* "egress"  */
  YYSYMBOL_INGRESS = 306,                  /* "ingress"  */
  YYSYMBOL_GBP = 307,                      /* "gbp"  */
  YYSYMBOL_CLASS = 308,                    /* "class"  */
  YYSYMBOL_OPTTYPE = 309,                  /* "opt-type"  */
  YYSYMBOL_COUNTERS = 310,                 /* "counters"  */
  YYSYMBOL_QUOTAS = 311,                   /* "quotas"  */
  YYSYMBOL_LIMITS = 312,                   /* "limits"  */
  YYSYMBOL_TUNNELS = 313,                  /* "tunnels"  */
  YYSYMBOL_SYNPROXYS = 314,                /* "synproxys"  */
  YYSYMBOL_HELPERS = 315,                  /* "helpers"  */
  YYSYMBOL_LOG = 316,                      /* "log"  */
  YYSYMBOL_PREFIX = 317,                   /* "prefix"  */
  YYSYMBOL_GROUP = 318,                    /* "group"  */
  YYSYMBOL_SNAPLEN = 319,                  /* "snaplen"  */
  YYSYMBOL_QUEUE_THRESHOLD = 320,          /* "queue-threshold"  */
  YYSYMBOL_LEVEL = 321,                    /* "level"  */
  YYSYMBOL_LIMIT = 322,                    /* "limit"  */
  YYSYMBOL_RATE = 323,                     /* "rate"  */
  YYSYMBOL_BURST = 324,                    /* "burst"  */
  YYSYMBOL_OVER = 325,                     /* "over"  */
  YYSYMBOL_UNTIL = 326,                    /* "until"  */
  YYSYMBOL_QUOTA = 327,                    /* "quota"  */
  YYSYMBOL_USED = 328,                     /* "used"  */
  YYSYMBOL_SECMARK = 329,                  /* "secmark"  */
  YYSYMBOL_SECMARKS = 330,                 /* "secmarks"  */
  YYSYMBOL_SECOND = 331,                   /* "second"  */
  YYSYMBOL_MINUTE = 332,                   /* "minute"  */
  YYSYMBOL_HOUR = 333,                     /* "hour"  */
  YYSYMBOL_DAY = 334,                      /* "day"  */
  YYSYMBOL_WEEK = 335,                     /* "week"  */
  YYSYMBOL__REJECT = 336,                  /* "reject"  */
  YYSYMBOL_WITH = 337,                     /* "with"  */
  YYSYMBOL_ICMPX = 338,                    /* "icmpx"  */
  YYSYMBOL_SNAT = 339,                     /* "snat"  */
  YYSYMBOL_DNAT = 340,                     /* "dnat"  */
  YYSYMBOL_MASQUERADE = 341,               /* "masquerade"  */
  YYSYMBOL_REDIRECT = 342,                 /* "redirect"  */
  YYSYMBOL_RANDOM = 343,                   /* "random"  */
  YYSYMBOL_FULLY_RANDOM = 344,             /* "fully-random"  */
  YYSYMBOL_PERSISTENT = 345,               /* "persistent"  */
  YYSYMBOL_QUEUE = 346,                    /* "queue"  */
  YYSYMBOL_QUEUENUM = 347,                 /* "num"  */
  YYSYMBOL_BYPASS = 348,                   /* "bypass"  */
  YYSYMBOL_FANOUT = 349,                   /* "fanout"  */
  YYSYMBOL_DUP = 350,                      /* "dup"  */
  YYSYMBOL_FWD = 351,                      /* "fwd"  */
  YYSYMBOL_NUMGEN = 352,                   /* "numgen"  */
  YYSYMBOL_INC = 353,                      /* "inc"  */
  YYSYMBOL_MOD = 354,                      /* "mod"  */
  YYSYMBOL_OFFSET = 355,                   /* "offset"  */
  YYSYMBOL_JHASH = 356,                    /* "jhash"  */
  YYSYMBOL_SYMHASH = 357,                  /* "symhash"  */
  YYSYMBOL_SEED = 358,                     /* "seed"  */
  YYSYMBOL_POSITION = 359,                 /* "position"  */
  YYSYMBOL_INDEX = 360,                    /* "index"  */
  YYSYMBOL_COMMENT = 361,                  /* "comment"  */
  YYSYMBOL_XML = 362,                      /* "xml"  */
  YYSYMBOL_JSON = 363,                     /* "json"  */
  YYSYMBOL_VM = 364,                       /* "vm"  */
  YYSYMBOL_NOTRACK = 365,                  /* "notrack"  */
  YYSYMBOL_EXISTS = 366,                   /* "exists"  */
  YYSYMBOL_MISSING = 367,                  /* "missing"  */
  YYSYMBOL_EXTHDR = 368,                   /* "exthdr"  */
  YYSYMBOL_IPSEC = 369,                    /* "ipsec"  */
  YYSYMBOL_REQID = 370,                    /* "reqid"  */
  YYSYMBOL_SPNUM = 371,                    /* "spnum"  */
  YYSYMBOL_IN = 372,                       /* "in"  */
  YYSYMBOL_OUT = 373,                      /* "out"  */
  YYSYMBOL_XT = 374,                       /* "xt"  */
  YYSYMBOL_375_ = 375,                     /* '='  */
  YYSYMBOL_376_ = 376,                     /* '{'  */
  YYSYMBOL_377_ = 377,                     /* '}'  */
  YYSYMBOL_378_ = 378,                     /* '('  */
  YYSYMBOL_379_ = 379,                     /* ')'  */
  YYSYMBOL_380_ = 380,                     /* '|'  */
  YYSYMBOL_381_ = 381,                     /* '$'  */
  YYSYMBOL_382_ = 382,                     /* '['  */
  YYSYMBOL_383_ = 383,                     /* ']'  */
  YYSYMBOL_YYACCEPT = 384,                 /* $accept  */
  YYSYMBOL_input = 385,                    /* input  */
  YYSYMBOL_stmt_separator = 386,           /* stmt_separator  */
  YYSYMBOL_opt_newline = 387,              /* opt_newline  */
  YYSYMBOL_close_scope_ah = 388,           /* close_scope_ah  */
  YYSYMBOL_close_scope_arp = 389,          /* close_scope_arp  */
  YYSYMBOL_close_scope_at = 390,           /* close_scope_at  */
  YYSYMBOL_close_scope_comp = 391,         /* close_scope_comp  */
  YYSYMBOL_close_scope_ct = 392,           /* close_scope_ct  */
  YYSYMBOL_close_scope_counter = 393,      /* close_scope_counter  */
  YYSYMBOL_close_scope_last = 394,         /* close_scope_last  */
  YYSYMBOL_close_scope_dccp = 395,         /* close_scope_dccp  */
  YYSYMBOL_close_scope_destroy = 396,      /* close_scope_destroy  */
  YYSYMBOL_close_scope_dst = 397,          /* close_scope_dst  */
  YYSYMBOL_close_scope_dup = 398,          /* close_scope_dup  */
  YYSYMBOL_close_scope_esp = 399,          /* close_scope_esp  */
  YYSYMBOL_close_scope_eth = 400,          /* close_scope_eth  */
  YYSYMBOL_close_scope_export = 401,       /* close_scope_export  */
  YYSYMBOL_close_scope_fib = 402,          /* close_scope_fib  */
  YYSYMBOL_close_scope_frag = 403,         /* close_scope_frag  */
  YYSYMBOL_close_scope_fwd = 404,          /* close_scope_fwd  */
  YYSYMBOL_close_scope_gre = 405,          /* close_scope_gre  */
  YYSYMBOL_close_scope_hash = 406,         /* close_scope_hash  */
  YYSYMBOL_close_scope_hbh = 407,          /* close_scope_hbh  */
  YYSYMBOL_close_scope_ip = 408,           /* close_scope_ip  */
  YYSYMBOL_close_scope_ip6 = 409,          /* close_scope_ip6  */
  YYSYMBOL_close_scope_vlan = 410,         /* close_scope_vlan  */
  YYSYMBOL_close_scope_icmp = 411,         /* close_scope_icmp  */
  YYSYMBOL_close_scope_igmp = 412,         /* close_scope_igmp  */
  YYSYMBOL_close_scope_import = 413,       /* close_scope_import  */
  YYSYMBOL_close_scope_ipsec = 414,        /* close_scope_ipsec  */
  YYSYMBOL_close_scope_list = 415,         /* close_scope_list  */
  YYSYMBOL_close_scope_limit = 416,        /* close_scope_limit  */
  YYSYMBOL_close_scope_meta = 417,         /* close_scope_meta  */
  YYSYMBOL_close_scope_mh = 418,           /* close_scope_mh  */
  YYSYMBOL_close_scope_monitor = 419,      /* close_scope_monitor  */
  YYSYMBOL_close_scope_nat = 420,          /* close_scope_nat  */
  YYSYMBOL_close_scope_numgen = 421,       /* close_scope_numgen  */
  YYSYMBOL_close_scope_osf = 422,          /* close_scope_osf  */
  YYSYMBOL_close_scope_policy = 423,       /* close_scope_policy  */
  YYSYMBOL_close_scope_quota = 424,        /* close_scope_quota  */
  YYSYMBOL_close_scope_queue = 425,        /* close_scope_queue  */
  YYSYMBOL_close_scope_reject = 426,       /* close_scope_reject  */
  YYSYMBOL_close_scope_reset = 427,        /* close_scope_reset  */
  YYSYMBOL_close_scope_rt = 428,           /* close_scope_rt  */
  YYSYMBOL_close_scope_sctp = 429,         /* close_scope_sctp  */
  YYSYMBOL_close_scope_sctp_chunk = 430,   /* close_scope_sctp_chunk  */
  YYSYMBOL_close_scope_secmark = 431,      /* close_scope_secmark  */
  YYSYMBOL_close_scope_socket = 432,       /* close_scope_socket  */
  YYSYMBOL_close_scope_tcp = 433,          /* close_scope_tcp  */
  YYSYMBOL_close_scope_tproxy = 434,       /* close_scope_tproxy  */
  YYSYMBOL_close_scope_type = 435,         /* close_scope_type  */
  YYSYMBOL_close_scope_th = 436,           /* close_scope_th  */
  YYSYMBOL_close_scope_udp = 437,          /* close_scope_udp  */
  YYSYMBOL_close_scope_udplite = 438,      /* close_scope_udplite  */
  YYSYMBOL_close_scope_log = 439,          /* close_scope_log  */
  YYSYMBOL_close_scope_synproxy = 440,     /* close_scope_synproxy  */
  YYSYMBOL_close_scope_tunnel = 441,       /* close_scope_tunnel  */
  YYSYMBOL_close_scope_xt = 442,           /* close_scope_xt  */
  YYSYMBOL_common_block = 443,             /* common_block  */
  YYSYMBOL_line = 444,                     /* line  */
  YYSYMBOL_base_cmd = 445,                 /* base_cmd  */
  YYSYMBOL_add_cmd = 446,                  /* add_cmd  */
  YYSYMBOL_replace_cmd = 447,              /* replace_cmd  */
  YYSYMBOL_create_cmd = 448,               /* create_cmd  */
  YYSYMBOL_insert_cmd = 449,               /* insert_cmd  */
  YYSYMBOL_table_or_id_spec = 450,         /* table_or_id_spec  */
  YYSYMBOL_chain_or_id_spec = 451,         /* chain_or_id_spec  */
  YYSYMBOL_set_or_id_spec = 452,           /* set_or_id_spec  */
  YYSYMBOL_obj_or_id_spec = 453,           /* obj_or_id_spec  */
  YYSYMBOL_delete_cmd = 454,               /* delete_cmd  */
  YYSYMBOL_destroy_cmd = 455,              /* destroy_cmd  */
  YYSYMBOL_get_cmd = 456,                  /* get_cmd  */
  YYSYMBOL_list_cmd_spec_table = 457,      /* list_cmd_spec_table  */
  YYSYMBOL_list_cmd_spec_any = 458,        /* list_cmd_spec_any  */
  YYSYMBOL_list_cmd = 459,                 /* list_cmd  */
  YYSYMBOL_basehook_device_name = 460,     /* basehook_device_name  */
  YYSYMBOL_basehook_spec = 461,            /* basehook_spec  */
  YYSYMBOL_reset_cmd = 462,                /* reset_cmd  */
  YYSYMBOL_flush_cmd = 463,                /* flush_cmd  */
  YYSYMBOL_rename_cmd = 464,               /* rename_cmd  */
  YYSYMBOL_import_cmd = 465,               /* import_cmd  */
  YYSYMBOL_export_cmd = 466,               /* export_cmd  */
  YYSYMBOL_monitor_cmd = 467,              /* monitor_cmd  */
  YYSYMBOL_monitor_event = 468,            /* monitor_event  */
  YYSYMBOL_monitor_object = 469,           /* monitor_object  */
  YYSYMBOL_monitor_format = 470,           /* monitor_format  */
  YYSYMBOL_markup_format = 471,            /* markup_format  */
  YYSYMBOL_describe_cmd = 472,             /* describe_cmd  */
  YYSYMBOL_table_block_alloc = 473,        /* table_block_alloc  */
  YYSYMBOL_table_options = 474,            /* table_options  */
  YYSYMBOL_table_flags = 475,              /* table_flags  */
  YYSYMBOL_table_flag = 476,               /* table_flag  */
  YYSYMBOL_table_block = 477,              /* table_block  */
  YYSYMBOL_chain_block_alloc = 478,        /* chain_block_alloc  */
  YYSYMBOL_chain_block = 479,              /* chain_block  */
  YYSYMBOL_subchain_block = 480,           /* subchain_block  */
  YYSYMBOL_typeof_verdict_expr = 481,      /* typeof_verdict_expr  */
  YYSYMBOL_typeof_data_expr = 482,         /* typeof_data_expr  */
  YYSYMBOL_primary_typeof_expr = 483,      /* primary_typeof_expr  */
  YYSYMBOL_typeof_expr = 484,              /* typeof_expr  */
  YYSYMBOL_set_block_alloc = 485,          /* set_block_alloc  */
  YYSYMBOL_typeof_key_expr = 486,          /* typeof_key_expr  */
  YYSYMBOL_set_block = 487,                /* set_block  */
  YYSYMBOL_set_block_expr = 488,           /* set_block_expr  */
  YYSYMBOL_set_flag_list = 489,            /* set_flag_list  */
  YYSYMBOL_set_flag = 490,                 /* set_flag  */
  YYSYMBOL_map_block_alloc = 491,          /* map_block_alloc  */
  YYSYMBOL_ct_obj_type_map = 492,          /* ct_obj_type_map  */
  YYSYMBOL_map_block_obj_type = 493,       /* map_block_obj_type  */
  YYSYMBOL_map_block_obj_typeof = 494,     /* map_block_obj_typeof  */
  YYSYMBOL_map_block_data_interval = 495,  /* map_block_data_interval  */
  YYSYMBOL_map_block = 496,                /* map_block  */
  YYSYMBOL_set_mechanism = 497,            /* set_mechanism  */
  YYSYMBOL_set_policy_spec = 498,          /* set_policy_spec  */
  YYSYMBOL_flowtable_block_alloc = 499,    /* flowtable_block_alloc  */
  YYSYMBOL_flowtable_block = 500,          /* flowtable_block  */
  YYSYMBOL_flowtable_expr = 501,           /* flowtable_expr  */
  YYSYMBOL_flowtable_list_expr = 502,      /* flowtable_list_expr  */
  YYSYMBOL_flowtable_expr_member = 503,    /* flowtable_expr_member  */
  YYSYMBOL_data_type_atom_expr = 504,      /* data_type_atom_expr  */
  YYSYMBOL_data_type_expr = 505,           /* data_type_expr  */
  YYSYMBOL_obj_block_alloc = 506,          /* obj_block_alloc  */
  YYSYMBOL_counter_block = 507,            /* counter_block  */
  YYSYMBOL_quota_block = 508,              /* quota_block  */
  YYSYMBOL_ct_helper_block = 509,          /* ct_helper_block  */
  YYSYMBOL_ct_timeout_block = 510,         /* ct_timeout_block  */
  YYSYMBOL_ct_expect_block = 511,          /* ct_expect_block  */
  YYSYMBOL_limit_block = 512,              /* limit_block  */
  YYSYMBOL_secmark_block = 513,            /* secmark_block  */
  YYSYMBOL_synproxy_block = 514,           /* synproxy_block  */
  YYSYMBOL_type_identifier = 515,          /* type_identifier  */
  YYSYMBOL_hook_spec = 516,                /* hook_spec  */
  YYSYMBOL_prio_spec = 517,                /* prio_spec  */
  YYSYMBOL_extended_prio_name = 518,       /* extended_prio_name  */
  YYSYMBOL_extended_prio_spec = 519,       /* extended_prio_spec  */
  YYSYMBOL_int_num = 520,                  /* int_num  */
  YYSYMBOL_dev_spec = 521,                 /* dev_spec  */
  YYSYMBOL_flags_spec = 522,               /* flags_spec  */
  YYSYMBOL_policy_spec = 523,              /* policy_spec  */
  YYSYMBOL_policy_expr = 524,              /* policy_expr  */
  YYSYMBOL_chain_policy = 525,             /* chain_policy  */
  YYSYMBOL_identifier = 526,               /* identifier  */
  YYSYMBOL_string = 527,                   /* string  */
  YYSYMBOL_time_spec = 528,                /* time_spec  */
  YYSYMBOL_time_spec_or_num_s = 529,       /* time_spec_or_num_s  */
  YYSYMBOL_family_spec = 530,              /* family_spec  */
  YYSYMBOL_family_spec_explicit = 531,     /* family_spec_explicit  */
  YYSYMBOL_table_spec = 532,               /* table_spec  */
  YYSYMBOL_tableid_spec = 533,             /* tableid_spec  */
  YYSYMBOL_chain_spec = 534,               /* chain_spec  */
  YYSYMBOL_chainid_spec = 535,             /* chainid_spec  */
  YYSYMBOL_chain_identifier = 536,         /* chain_identifier  */
  YYSYMBOL_set_spec = 537,                 /* set_spec  */
  YYSYMBOL_setid_spec = 538,               /* setid_spec  */
  YYSYMBOL_set_identifier = 539,           /* set_identifier  */
  YYSYMBOL_flowtable_spec = 540,           /* flowtable_spec  */
  YYSYMBOL_flowtableid_spec = 541,         /* flowtableid_spec  */
  YYSYMBOL_flowtable_identifier = 542,     /* flowtable_identifier  */
  YYSYMBOL_obj_spec = 543,                 /* obj_spec  */
  YYSYMBOL_objid_spec = 544,               /* objid_spec  */
  YYSYMBOL_obj_identifier = 545,           /* obj_identifier  */
  YYSYMBOL_handle_spec = 546,              /* handle_spec  */
  YYSYMBOL_position_spec = 547,            /* position_spec  */
  YYSYMBOL_index_spec = 548,               /* index_spec  */
  YYSYMBOL_rule_position = 549,            /* rule_position  */
  YYSYMBOL_ruleid_spec = 550,              /* ruleid_spec  */
  YYSYMBOL_comment_spec = 551,             /* comment_spec  */
  YYSYMBOL_ruleset_spec = 552,             /* ruleset_spec  */
  YYSYMBOL_rule = 553,                     /* rule  */
  YYSYMBOL_rule_alloc = 554,               /* rule_alloc  */
  YYSYMBOL_stmt_list = 555,                /* stmt_list  */
  YYSYMBOL_stateful_stmt_list = 556,       /* stateful_stmt_list  */
  YYSYMBOL_objref_stmt_counter = 557,      /* objref_stmt_counter  */
  YYSYMBOL_objref_stmt_limit = 558,        /* objref_stmt_limit  */
  YYSYMBOL_objref_stmt_quota = 559,        /* objref_stmt_quota  */
  YYSYMBOL_objref_stmt_synproxy = 560,     /* objref_stmt_synproxy  */
  YYSYMBOL_objref_stmt_tunnel = 561,       /* objref_stmt_tunnel  */
  YYSYMBOL_objref_stmt_ct = 562,           /* objref_stmt_ct  */
  YYSYMBOL_objref_stmt = 563,              /* objref_stmt  */
  YYSYMBOL_stateful_stmt = 564,            /* stateful_stmt  */
  YYSYMBOL_stmt = 565,                     /* stmt  */
  YYSYMBOL_xt_stmt = 566,                  /* xt_stmt  */
  YYSYMBOL_chain_stmt_type = 567,          /* chain_stmt_type  */
  YYSYMBOL_chain_stmt = 568,               /* chain_stmt  */
  YYSYMBOL_verdict_stmt = 569,             /* verdict_stmt  */
  YYSYMBOL_verdict_map_stmt = 570,         /* verdict_map_stmt  */
  YYSYMBOL_verdict_map_expr = 571,         /* verdict_map_expr  */
  YYSYMBOL_verdict_map_list_expr = 572,    /* verdict_map_list_expr  */
  YYSYMBOL_verdict_map_list_member_expr = 573, /* verdict_map_list_member_expr  */
  YYSYMBOL_ct_limit_stmt_alloc = 574,      /* ct_limit_stmt_alloc  */
  YYSYMBOL_connlimit_stmt = 575,           /* connlimit_stmt  */
  YYSYMBOL_ct_limit_args = 576,            /* ct_limit_args  */
  YYSYMBOL_counter_stmt = 577,             /* counter_stmt  */
  YYSYMBOL_counter_stmt_alloc = 578,       /* counter_stmt_alloc  */
  YYSYMBOL_counter_args = 579,             /* counter_args  */
  YYSYMBOL_counter_arg = 580,              /* counter_arg  */
  YYSYMBOL_last_stmt_alloc = 581,          /* last_stmt_alloc  */
  YYSYMBOL_last_stmt = 582,                /* last_stmt  */
  YYSYMBOL_last_args = 583,                /* last_args  */
  YYSYMBOL_log_stmt = 584,                 /* log_stmt  */
  YYSYMBOL_log_stmt_alloc = 585,           /* log_stmt_alloc  */
  YYSYMBOL_log_args = 586,                 /* log_args  */
  YYSYMBOL_log_arg = 587,                  /* log_arg  */
  YYSYMBOL_level_type = 588,               /* level_type  */
  YYSYMBOL_log_flags = 589,                /* log_flags  */
  YYSYMBOL_log_flags_tcp = 590,            /* log_flags_tcp  */
  YYSYMBOL_log_flag_tcp = 591,             /* log_flag_tcp  */
  YYSYMBOL_limit_stmt_alloc = 592,         /* limit_stmt_alloc  */
  YYSYMBOL_limit_stmt = 593,               /* limit_stmt  */
  YYSYMBOL_limit_args = 594,               /* limit_args  */
  YYSYMBOL_quota_mode = 595,               /* quota_mode  */
  YYSYMBOL_quota_unit = 596,               /* quota_unit  */
  YYSYMBOL_quota_used = 597,               /* quota_used  */
  YYSYMBOL_quota_stmt_alloc = 598,         /* quota_stmt_alloc  */
  YYSYMBOL_quota_stmt = 599,               /* quota_stmt  */
  YYSYMBOL_quota_args = 600,               /* quota_args  */
  YYSYMBOL_limit_mode = 601,               /* limit_mode  */
  YYSYMBOL_limit_burst_pkts = 602,         /* limit_burst_pkts  */
  YYSYMBOL_limit_rate_pkts = 603,          /* limit_rate_pkts  */
  YYSYMBOL_limit_burst_bytes = 604,        /* limit_burst_bytes  */
  YYSYMBOL_limit_rate_bytes = 605,         /* limit_rate_bytes  */
  YYSYMBOL_limit_bytes = 606,              /* limit_bytes  */
  YYSYMBOL_time_unit = 607,                /* time_unit  */
  YYSYMBOL_reject_stmt = 608,              /* reject_stmt  */
  YYSYMBOL_reject_stmt_alloc = 609,        /* reject_stmt_alloc  */
  YYSYMBOL_reject_with_expr = 610,         /* reject_with_expr  */
  YYSYMBOL_reject_opts = 611,              /* reject_opts  */
  YYSYMBOL_nat_stmt = 612,                 /* nat_stmt  */
  YYSYMBOL_nat_stmt_alloc = 613,           /* nat_stmt_alloc  */
  YYSYMBOL_tproxy_stmt = 614,              /* tproxy_stmt  */
  YYSYMBOL_synproxy_stmt = 615,            /* synproxy_stmt  */
  YYSYMBOL_synproxy_stmt_alloc = 616,      /* synproxy_stmt_alloc  */
  YYSYMBOL_synproxy_args = 617,            /* synproxy_args  */
  YYSYMBOL_synproxy_arg = 618,             /* synproxy_arg  */
  YYSYMBOL_synproxy_config = 619,          /* synproxy_config  */
  YYSYMBOL_synproxy_obj = 620,             /* synproxy_obj  */
  YYSYMBOL_synproxy_ts = 621,              /* synproxy_ts  */
  YYSYMBOL_synproxy_sack = 622,            /* synproxy_sack  */
  YYSYMBOL_primary_stmt_expr = 623,        /* primary_stmt_expr  */
  YYSYMBOL_shift_stmt_expr = 624,          /* shift_stmt_expr  */
  YYSYMBOL_and_stmt_expr = 625,            /* and_stmt_expr  */
  YYSYMBOL_exclusive_or_stmt_expr = 626,   /* exclusive_or_stmt_expr  */
  YYSYMBOL_inclusive_or_stmt_expr = 627,   /* inclusive_or_stmt_expr  */
  YYSYMBOL_basic_stmt_expr = 628,          /* basic_stmt_expr  */
  YYSYMBOL_concat_stmt_expr = 629,         /* concat_stmt_expr  */
  YYSYMBOL_map_stmt_expr_set = 630,        /* map_stmt_expr_set  */
  YYSYMBOL_map_stmt_expr = 631,            /* map_stmt_expr  */
  YYSYMBOL_prefix_stmt_expr = 632,         /* prefix_stmt_expr  */
  YYSYMBOL_range_stmt_expr = 633,          /* range_stmt_expr  */
  YYSYMBOL_multiton_stmt_expr = 634,       /* multiton_stmt_expr  */
  YYSYMBOL_stmt_expr = 635,                /* stmt_expr  */
  YYSYMBOL_nat_stmt_args = 636,            /* nat_stmt_args  */
  YYSYMBOL_masq_stmt = 637,                /* masq_stmt  */
  YYSYMBOL_masq_stmt_alloc = 638,          /* masq_stmt_alloc  */
  YYSYMBOL_masq_stmt_args = 639,           /* masq_stmt_args  */
  YYSYMBOL_redir_stmt = 640,               /* redir_stmt  */
  YYSYMBOL_redir_stmt_alloc = 641,         /* redir_stmt_alloc  */
  YYSYMBOL_redir_stmt_arg = 642,           /* redir_stmt_arg  */
  YYSYMBOL_dup_stmt = 643,                 /* dup_stmt  */
  YYSYMBOL_fwd_stmt = 644,                 /* fwd_stmt  */
  YYSYMBOL_nf_nat_flags = 645,             /* nf_nat_flags  */
  YYSYMBOL_nf_nat_flag = 646,              /* nf_nat_flag  */
  YYSYMBOL_queue_stmt = 647,               /* queue_stmt  */
  YYSYMBOL_queue_stmt_compat = 648,        /* queue_stmt_compat  */
  YYSYMBOL_queue_stmt_alloc = 649,         /* queue_stmt_alloc  */
  YYSYMBOL_queue_stmt_args = 650,          /* queue_stmt_args  */
  YYSYMBOL_queue_stmt_arg = 651,           /* queue_stmt_arg  */
  YYSYMBOL_queue_expr = 652,               /* queue_expr  */
  YYSYMBOL_queue_stmt_expr_simple = 653,   /* queue_stmt_expr_simple  */
  YYSYMBOL_queue_stmt_expr = 654,          /* queue_stmt_expr  */
  YYSYMBOL_queue_stmt_flags = 655,         /* queue_stmt_flags  */
  YYSYMBOL_queue_stmt_flag = 656,          /* queue_stmt_flag  */
  YYSYMBOL_set_elem_expr_stmt = 657,       /* set_elem_expr_stmt  */
  YYSYMBOL_set_elem_expr_stmt_alloc = 658, /* set_elem_expr_stmt_alloc  */
  YYSYMBOL_set_stmt = 659,                 /* set_stmt  */
  YYSYMBOL_set_stmt_op = 660,              /* set_stmt_op  */
  YYSYMBOL_map_stmt = 661,                 /* map_stmt  */
  YYSYMBOL_meter_stmt = 662,               /* meter_stmt  */
  YYSYMBOL_match_stmt = 663,               /* match_stmt  */
  YYSYMBOL_variable_expr = 664,            /* variable_expr  */
  YYSYMBOL_symbol_expr = 665,              /* symbol_expr  */
  YYSYMBOL_set_ref_expr = 666,             /* set_ref_expr  */
  YYSYMBOL_set_ref_symbol_expr = 667,      /* set_ref_symbol_expr  */
  YYSYMBOL_integer_expr = 668,             /* integer_expr  */
  YYSYMBOL_selector_expr = 669,            /* selector_expr  */
  YYSYMBOL_primary_expr = 670,             /* primary_expr  */
  YYSYMBOL_fib_expr = 671,                 /* fib_expr  */
  YYSYMBOL_fib_result = 672,               /* fib_result  */
  YYSYMBOL_fib_flag = 673,                 /* fib_flag  */
  YYSYMBOL_fib_tuple = 674,                /* fib_tuple  */
  YYSYMBOL_osf_expr = 675,                 /* osf_expr  */
  YYSYMBOL_osf_ttl = 676,                  /* osf_ttl  */
  YYSYMBOL_shift_expr = 677,               /* shift_expr  */
  YYSYMBOL_and_expr = 678,                 /* and_expr  */
  YYSYMBOL_exclusive_or_expr = 679,        /* exclusive_or_expr  */
  YYSYMBOL_inclusive_or_expr = 680,        /* inclusive_or_expr  */
  YYSYMBOL_basic_expr = 681,               /* basic_expr  */
  YYSYMBOL_concat_expr = 682,              /* concat_expr  */
  YYSYMBOL_prefix_rhs_expr = 683,          /* prefix_rhs_expr  */
  YYSYMBOL_range_rhs_expr = 684,           /* range_rhs_expr  */
  YYSYMBOL_multiton_rhs_expr = 685,        /* multiton_rhs_expr  */
  YYSYMBOL_map_expr = 686,                 /* map_expr  */
  YYSYMBOL_expr = 687,                     /* expr  */
  YYSYMBOL_set_expr = 688,                 /* set_expr  */
  YYSYMBOL_set_list_expr = 689,            /* set_list_expr  */
  YYSYMBOL_set_list_member_expr = 690,     /* set_list_member_expr  */
  YYSYMBOL_meter_key_expr = 691,           /* meter_key_expr  */
  YYSYMBOL_meter_key_expr_alloc = 692,     /* meter_key_expr_alloc  */
  YYSYMBOL_set_elem_expr = 693,            /* set_elem_expr  */
  YYSYMBOL_set_elem_key_expr = 694,        /* set_elem_key_expr  */
  YYSYMBOL_set_elem_expr_alloc = 695,      /* set_elem_expr_alloc  */
  YYSYMBOL_set_elem_options = 696,         /* set_elem_options  */
  YYSYMBOL_set_elem_time_spec = 697,       /* set_elem_time_spec  */
  YYSYMBOL_set_elem_option = 698,          /* set_elem_option  */
  YYSYMBOL_set_elem_expr_options = 699,    /* set_elem_expr_options  */
  YYSYMBOL_set_elem_stmt_list = 700,       /* set_elem_stmt_list  */
  YYSYMBOL_set_elem_stmt = 701,            /* set_elem_stmt  */
  YYSYMBOL_set_elem_expr_option = 702,     /* set_elem_expr_option  */
  YYSYMBOL_set_lhs_expr = 703,             /* set_lhs_expr  */
  YYSYMBOL_set_rhs_expr = 704,             /* set_rhs_expr  */
  YYSYMBOL_initializer_expr = 705,         /* initializer_expr  */
  YYSYMBOL_counter_config = 706,           /* counter_config  */
  YYSYMBOL_counter_obj = 707,              /* counter_obj  */
  YYSYMBOL_quota_config = 708,             /* quota_config  */
  YYSYMBOL_quota_obj = 709,                /* quota_obj  */
  YYSYMBOL_secmark_config = 710,           /* secmark_config  */
  YYSYMBOL_secmark_obj = 711,              /* secmark_obj  */
  YYSYMBOL_ct_obj_type = 712,              /* ct_obj_type  */
  YYSYMBOL_ct_cmd_type = 713,              /* ct_cmd_type  */
  YYSYMBOL_ct_l4protoname = 714,           /* ct_l4protoname  */
  YYSYMBOL_ct_helper_config = 715,         /* ct_helper_config  */
  YYSYMBOL_timeout_states = 716,           /* timeout_states  */
  YYSYMBOL_timeout_state = 717,            /* timeout_state  */
  YYSYMBOL_ct_timeout_config = 718,        /* ct_timeout_config  */
  YYSYMBOL_ct_expect_config = 719,         /* ct_expect_config  */
  YYSYMBOL_ct_obj_alloc = 720,             /* ct_obj_alloc  */
  YYSYMBOL_limit_config = 721,             /* limit_config  */
  YYSYMBOL_limit_obj = 722,                /* limit_obj  */
  YYSYMBOL_erspan_block = 723,             /* erspan_block  */
  YYSYMBOL_erspan_block_alloc = 724,       /* erspan_block_alloc  */
  YYSYMBOL_erspan_config = 725,            /* erspan_config  */
  YYSYMBOL_geneve_block = 726,             /* geneve_block  */
  YYSYMBOL_geneve_block_alloc = 727,       /* geneve_block_alloc  */
  YYSYMBOL_geneve_config = 728,            /* geneve_config  */
  YYSYMBOL_vxlan_block = 729,              /* vxlan_block  */
  YYSYMBOL_vxlan_block_alloc = 730,        /* vxlan_block_alloc  */
  YYSYMBOL_vxlan_config = 731,             /* vxlan_config  */
  YYSYMBOL_tunnel_config = 732,            /* tunnel_config  */
  YYSYMBOL_tunnel_block = 733,             /* tunnel_block  */
  YYSYMBOL_tunnel_obj = 734,               /* tunnel_obj  */
  YYSYMBOL_relational_expr = 735,          /* relational_expr  */
  YYSYMBOL_list_rhs_expr = 736,            /* list_rhs_expr  */
  YYSYMBOL_rhs_expr = 737,                 /* rhs_expr  */
  YYSYMBOL_shift_rhs_expr = 738,           /* shift_rhs_expr  */
  YYSYMBOL_and_rhs_expr = 739,             /* and_rhs_expr  */
  YYSYMBOL_exclusive_or_rhs_expr = 740,    /* exclusive_or_rhs_expr  */
  YYSYMBOL_inclusive_or_rhs_expr = 741,    /* inclusive_or_rhs_expr  */
  YYSYMBOL_basic_rhs_expr = 742,           /* basic_rhs_expr  */
  YYSYMBOL_concat_rhs_expr = 743,          /* concat_rhs_expr  */
  YYSYMBOL_boolean_keys = 744,             /* boolean_keys  */
  YYSYMBOL_boolean_expr = 745,             /* boolean_expr  */
  YYSYMBOL_keyword_expr = 746,             /* keyword_expr  */
  YYSYMBOL_primary_rhs_expr = 747,         /* primary_rhs_expr  */
  YYSYMBOL_relational_op = 748,            /* relational_op  */
  YYSYMBOL_verdict_expr = 749,             /* verdict_expr  */
  YYSYMBOL_chain_expr = 750,               /* chain_expr  */
  YYSYMBOL_meta_expr = 751,                /* meta_expr  */
  YYSYMBOL_meta_key = 752,                 /* meta_key  */
  YYSYMBOL_meta_key_qualified = 753,       /* meta_key_qualified  */
  YYSYMBOL_meta_key_unqualified = 754,     /* meta_key_unqualified  */
  YYSYMBOL_meta_stmt = 755,                /* meta_stmt  */
  YYSYMBOL_socket_expr = 756,              /* socket_expr  */
  YYSYMBOL_socket_key = 757,               /* socket_key  */
  YYSYMBOL_tunnel_key = 758,               /* tunnel_key  */
  YYSYMBOL_tunnel_expr = 759,              /* tunnel_expr  */
  YYSYMBOL_offset_opt = 760,               /* offset_opt  */
  YYSYMBOL_numgen_type = 761,              /* numgen_type  */
  YYSYMBOL_numgen_expr = 762,              /* numgen_expr  */
  YYSYMBOL_xfrm_spnum = 763,               /* xfrm_spnum  */
  YYSYMBOL_xfrm_dir = 764,                 /* xfrm_dir  */
  YYSYMBOL_xfrm_state_key = 765,           /* xfrm_state_key  */
  YYSYMBOL_xfrm_state_proto_key = 766,     /* xfrm_state_proto_key  */
  YYSYMBOL_xfrm_expr = 767,                /* xfrm_expr  */
  YYSYMBOL_hash_expr = 768,                /* hash_expr  */
  YYSYMBOL_nf_key_proto = 769,             /* nf_key_proto  */
  YYSYMBOL_rt_expr = 770,                  /* rt_expr  */
  YYSYMBOL_rt_key = 771,                   /* rt_key  */
  YYSYMBOL_ct_expr = 772,                  /* ct_expr  */
  YYSYMBOL_ct_dir = 773,                   /* ct_dir  */
  YYSYMBOL_ct_key = 774,                   /* ct_key  */
  YYSYMBOL_ct_key_dir = 775,               /* ct_key_dir  */
  YYSYMBOL_ct_key_proto_field = 776,       /* ct_key_proto_field  */
  YYSYMBOL_ct_key_dir_optional = 777,      /* ct_key_dir_optional  */
  YYSYMBOL_symbol_stmt_expr = 778,         /* symbol_stmt_expr  */
  YYSYMBOL_list_stmt_expr = 779,           /* list_stmt_expr  */
  YYSYMBOL_ct_stmt = 780,                  /* ct_stmt  */
  YYSYMBOL_payload_stmt = 781,             /* payload_stmt  */
  YYSYMBOL_payload_expr = 782,             /* payload_expr  */
  YYSYMBOL_payload_raw_len = 783,          /* payload_raw_len  */
  YYSYMBOL_payload_raw_expr = 784,         /* payload_raw_expr  */
  YYSYMBOL_payload_base_spec = 785,        /* payload_base_spec  */
  YYSYMBOL_eth_hdr_expr = 786,             /* eth_hdr_expr  */
  YYSYMBOL_eth_hdr_field = 787,            /* eth_hdr_field  */
  YYSYMBOL_vlan_hdr_expr = 788,            /* vlan_hdr_expr  */
  YYSYMBOL_vlan_hdr_field = 789,           /* vlan_hdr_field  */
  YYSYMBOL_arp_hdr_expr = 790,             /* arp_hdr_expr  */
  YYSYMBOL_arp_hdr_field = 791,            /* arp_hdr_field  */
  YYSYMBOL_ip_hdr_expr = 792,              /* ip_hdr_expr  */
  YYSYMBOL_ip_hdr_field = 793,             /* ip_hdr_field  */
  YYSYMBOL_ip_option_type = 794,           /* ip_option_type  */
  YYSYMBOL_ip_option_field = 795,          /* ip_option_field  */
  YYSYMBOL_icmp_hdr_expr = 796,            /* icmp_hdr_expr  */
  YYSYMBOL_icmp_hdr_field = 797,           /* icmp_hdr_field  */
  YYSYMBOL_igmp_hdr_expr = 798,            /* igmp_hdr_expr  */
  YYSYMBOL_igmp_hdr_field = 799,           /* igmp_hdr_field  */
  YYSYMBOL_ip6_hdr_expr = 800,             /* ip6_hdr_expr  */
  YYSYMBOL_ip6_hdr_field = 801,            /* ip6_hdr_field  */
  YYSYMBOL_icmp6_hdr_expr = 802,           /* icmp6_hdr_expr  */
  YYSYMBOL_icmp6_hdr_field = 803,          /* icmp6_hdr_field  */
  YYSYMBOL_auth_hdr_expr = 804,            /* auth_hdr_expr  */
  YYSYMBOL_auth_hdr_field = 805,           /* auth_hdr_field  */
  YYSYMBOL_esp_hdr_expr = 806,             /* esp_hdr_expr  */
  YYSYMBOL_esp_hdr_field = 807,            /* esp_hdr_field  */
  YYSYMBOL_comp_hdr_expr = 808,            /* comp_hdr_expr  */
  YYSYMBOL_comp_hdr_field = 809,           /* comp_hdr_field  */
  YYSYMBOL_udp_hdr_expr = 810,             /* udp_hdr_expr  */
  YYSYMBOL_udp_hdr_field = 811,            /* udp_hdr_field  */
  YYSYMBOL_udplite_hdr_expr = 812,         /* udplite_hdr_expr  */
  YYSYMBOL_udplite_hdr_field = 813,        /* udplite_hdr_field  */
  YYSYMBOL_tcp_hdr_expr = 814,             /* tcp_hdr_expr  */
  YYSYMBOL_inner_inet_expr = 815,          /* inner_inet_expr  */
  YYSYMBOL_inner_eth_expr = 816,           /* inner_eth_expr  */
  YYSYMBOL_inner_expr = 817,               /* inner_expr  */
  YYSYMBOL_vxlan_hdr_expr = 818,           /* vxlan_hdr_expr  */
  YYSYMBOL_vxlan_hdr_field = 819,          /* vxlan_hdr_field  */
  YYSYMBOL_geneve_hdr_expr = 820,          /* geneve_hdr_expr  */
  YYSYMBOL_geneve_hdr_field = 821,         /* geneve_hdr_field  */
  YYSYMBOL_gre_hdr_expr = 822,             /* gre_hdr_expr  */
  YYSYMBOL_gre_hdr_field = 823,            /* gre_hdr_field  */
  YYSYMBOL_gretap_hdr_expr = 824,          /* gretap_hdr_expr  */
  YYSYMBOL_optstrip_stmt = 825,            /* optstrip_stmt  */
  YYSYMBOL_tcp_hdr_field = 826,            /* tcp_hdr_field  */
  YYSYMBOL_tcp_hdr_option_kind_and_field = 827, /* tcp_hdr_option_kind_and_field  */
  YYSYMBOL_tcp_hdr_option_sack = 828,      /* tcp_hdr_option_sack  */
  YYSYMBOL_tcp_hdr_option_type = 829,      /* tcp_hdr_option_type  */
  YYSYMBOL_tcpopt_field_sack = 830,        /* tcpopt_field_sack  */
  YYSYMBOL_tcpopt_field_window = 831,      /* tcpopt_field_window  */
  YYSYMBOL_tcpopt_field_tsopt = 832,       /* tcpopt_field_tsopt  */
  YYSYMBOL_tcpopt_field_maxseg = 833,      /* tcpopt_field_maxseg  */
  YYSYMBOL_tcpopt_field_mptcp = 834,       /* tcpopt_field_mptcp  */
  YYSYMBOL_dccp_hdr_expr = 835,            /* dccp_hdr_expr  */
  YYSYMBOL_dccp_hdr_field = 836,           /* dccp_hdr_field  */
  YYSYMBOL_sctp_chunk_type = 837,          /* sctp_chunk_type  */
  YYSYMBOL_sctp_chunk_common_field = 838,  /* sctp_chunk_common_field  */
  YYSYMBOL_sctp_chunk_data_field = 839,    /* sctp_chunk_data_field  */
  YYSYMBOL_sctp_chunk_init_field = 840,    /* sctp_chunk_init_field  */
  YYSYMBOL_sctp_chunk_sack_field = 841,    /* sctp_chunk_sack_field  */
  YYSYMBOL_sctp_chunk_alloc = 842,         /* sctp_chunk_alloc  */
  YYSYMBOL_sctp_hdr_expr = 843,            /* sctp_hdr_expr  */
  YYSYMBOL_sctp_hdr_field = 844,           /* sctp_hdr_field  */
  YYSYMBOL_th_hdr_expr = 845,              /* th_hdr_expr  */
  YYSYMBOL_th_hdr_field = 846,             /* th_hdr_field  */
  YYSYMBOL_exthdr_expr = 847,              /* exthdr_expr  */
  YYSYMBOL_hbh_hdr_expr = 848,             /* hbh_hdr_expr  */
  YYSYMBOL_hbh_hdr_field = 849,            /* hbh_hdr_field  */
  YYSYMBOL_rt_hdr_expr = 850,              /* rt_hdr_expr  */
  YYSYMBOL_rt_hdr_field = 851,             /* rt_hdr_field  */
  YYSYMBOL_rt0_hdr_expr = 852,             /* rt0_hdr_expr  */
  YYSYMBOL_rt0_hdr_field = 853,            /* rt0_hdr_field  */
  YYSYMBOL_rt2_hdr_expr = 854,             /* rt2_hdr_expr  */
  YYSYMBOL_rt2_hdr_field = 855,            /* rt2_hdr_field  */
  YYSYMBOL_rt4_hdr_expr = 856,             /* rt4_hdr_expr  */
  YYSYMBOL_rt4_hdr_field = 857,            /* rt4_hdr_field  */
  YYSYMBOL_frag_hdr_expr = 858,            /* frag_hdr_expr  */
  YYSYMBOL_frag_hdr_field = 859,           /* frag_hdr_field  */
  YYSYMBOL_dst_hdr_expr = 860,             /* dst_hdr_expr  */
  YYSYMBOL_dst_hdr_field = 861,            /* dst_hdr_field  */
  YYSYMBOL_mh_hdr_expr = 862,              /* mh_hdr_expr  */
  YYSYMBOL_mh_hdr_field = 863,             /* mh_hdr_field  */
  YYSYMBOL_exthdr_exists_expr = 864,       /* exthdr_exists_expr  */
  YYSYMBOL_exthdr_key = 865                /* exthdr_key  */
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
#define YYLAST   9954

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  384
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  482
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1438
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2457

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   629


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
       2,     2,     2,     2,     2,     2,   381,     2,     2,     2,
     378,   379,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   375,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   382,     2,   383,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   376,   380,   377,     2,     2,     2,     2,
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
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,  1040,  1040,  1041,  1050,  1051,  1054,  1055,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1110,
    1111,  1112,  1113,  1115,  1123,  1138,  1145,  1157,  1165,  1166,
    1167,  1168,  1188,  1189,  1190,  1191,  1192,  1193,  1194,  1195,
    1196,  1197,  1198,  1199,  1200,  1201,  1202,  1203,  1206,  1210,
    1217,  1221,  1229,  1233,  1237,  1244,  1251,  1261,  1268,  1277,
    1281,  1285,  1289,  1293,  1297,  1301,  1305,  1309,  1313,  1317,
    1321,  1325,  1329,  1335,  1341,  1345,  1352,  1356,  1364,  1371,
    1378,  1388,  1395,  1404,  1408,  1412,  1416,  1420,  1424,  1428,
    1432,  1436,  1442,  1448,  1449,  1452,  1453,  1456,  1457,  1460,
    1461,  1464,  1468,  1472,  1480,  1484,  1488,  1492,  1496,  1500,
    1504,  1511,  1515,  1519,  1525,  1529,  1533,  1537,  1543,  1547,
    1551,  1555,  1559,  1563,  1567,  1571,  1575,  1582,  1586,  1590,
    1596,  1600,  1604,  1608,  1615,  1621,  1622,  1624,  1625,  1628,
    1632,  1636,  1640,  1644,  1648,  1652,  1656,  1660,  1664,  1668,
    1672,  1676,  1680,  1684,  1688,  1692,  1696,  1700,  1704,  1708,
    1712,  1716,  1720,  1724,  1728,  1732,  1736,  1740,  1744,  1750,
    1756,  1760,  1770,  1774,  1778,  1782,  1786,  1790,  1794,  1798,
    1803,  1807,  1811,  1815,  1821,  1825,  1829,  1833,  1837,  1841,
    1845,  1851,  1858,  1864,  1872,  1878,  1886,  1895,  1896,  1899,
    1900,  1901,  1902,  1903,  1904,  1905,  1906,  1909,  1910,  1913,
    1914,  1915,  1918,  1927,  1937,  1941,  1951,  1952,  1957,  1971,
    1972,  1973,  1974,  1975,  1986,  1996,  2007,  2017,  2028,  2039,
    2048,  2057,  2066,  2077,  2088,  2099,  2113,  2123,  2124,  2125,
    2126,  2127,  2128,  2129,  2134,  2143,  2153,  2154,  2155,  2162,
    2174,  2185,  2190,  2194,  2198,  2215,  2228,  2232,  2245,  2250,
    2251,  2254,  2255,  2256,  2257,  2267,  2272,  2277,  2282,  2288,
    2297,  2302,  2303,  2314,  2315,  2318,  2322,  2325,  2326,  2327,
    2328,  2332,  2337,  2338,  2341,  2342,  2343,  2344,  2345,  2348,
    2349,  2352,  2353,  2356,  2357,  2358,  2359,  2364,  2369,  2386,
    2409,  2423,  2432,  2437,  2443,  2448,  2457,  2460,  2464,  2470,
    2471,  2475,  2480,  2481,  2482,  2483,  2498,  2502,  2506,  2512,
    2517,  2524,  2529,  2534,  2537,  2546,  2553,  2566,  2573,  2574,
    2586,  2591,  2592,  2593,  2594,  2598,  2608,  2609,  2610,  2611,
    2615,  2625,  2626,  2627,  2628,  2632,  2643,  2648,  2649,  2650,
    2654,  2664,  2665,  2666,  2667,  2671,  2681,  2682,  2683,  2684,
    2688,  2698,  2699,  2700,  2701,  2705,  2715,  2716,  2717,  2718,
    2722,  2732,  2733,  2734,  2735,  2736,  2739,  2775,  2782,  2786,
    2789,  2799,  2806,  2817,  2830,  2845,  2846,  2849,  2860,  2866,
    2870,  2873,  2879,  2893,  2898,  2907,  2908,  2911,  2912,  2915,
    2916,  2917,  2920,  2936,  2937,  2940,  2941,  2944,  2945,  2946,
    2947,  2948,  2949,  2952,  2961,  2970,  2978,  2986,  2994,  3002,
    3010,  3018,  3026,  3034,  3042,  3050,  3058,  3066,  3074,  3082,
    3090,  3094,  3099,  3107,  3114,  3121,  3135,  3139,  3146,  3150,
    3156,  3168,  3174,  3181,  3187,  3194,  3202,  3210,  3218,  3226,
    3234,  3241,  3249,  3250,  3251,  3252,  3253,  3254,  3257,  3258,
    3259,  3260,  3261,  3264,  3265,  3266,  3267,  3268,  3269,  3270,
    3271,  3272,  3273,  3274,  3275,  3276,  3277,  3278,  3279,  3280,
    3281,  3282,  3283,  3284,  3285,  3286,  3289,  3300,  3301,  3304,
    3313,  3317,  3323,  3329,  3334,  3337,  3342,  3347,  3350,  3356,
    3362,  3365,  3371,  3380,  3381,  3383,  3389,  3393,  3396,  3401,
    3408,  3414,  3415,  3418,  3419,  3430,  3431,  3434,  3440,  3444,
    3447,  3464,  3469,  3474,  3479,  3484,  3490,  3520,  3524,  3528,
    3532,  3536,  3542,  3546,  3549,  3553,  3559,  3565,  3568,  3586,
    3601,  3602,  3603,  3606,  3607,  3610,  3611,  3626,  3632,  3635,
    3656,  3657,  3658,  3661,  3662,  3665,  3672,  3673,  3676,  3690,
    3697,  3698,  3713,  3714,  3715,  3716,  3717,  3720,  3723,  3729,
    3735,  3739,  3743,  3750,  3757,  3764,  3771,  3777,  3783,  3789,
    3792,  3793,  3796,  3802,  3808,  3814,  3821,  3828,  3836,  3837,
    3840,  3846,  3850,  3853,  3858,  3863,  3867,  3873,  3889,  3908,
    3914,  3915,  3921,  3922,  3928,  3929,  3930,  3931,  3932,  3933,
    3934,  3935,  3936,  3937,  3938,  3939,  3940,  3941,  3942,  3945,
    3946,  3950,  3956,  3957,  3963,  3964,  3970,  3971,  3977,  3980,
    3981,  3992,  3993,  3996,  4000,  4003,  4009,  4015,  4016,  4019,
    4020,  4021,  4024,  4028,  4032,  4037,  4042,  4047,  4053,  4057,
    4061,  4065,  4071,  4076,  4080,  4088,  4097,  4098,  4101,  4104,
    4108,  4113,  4119,  4120,  4123,  4126,  4130,  4134,  4138,  4143,
    4150,  4155,  4163,  4168,  4177,  4178,  4184,  4185,  4186,  4189,
    4190,  4194,  4198,  4204,  4205,  4208,  4214,  4218,  4221,  4226,
    4232,  4233,  4236,  4237,  4238,  4244,  4245,  4246,  4247,  4250,
    4251,  4257,  4258,  4261,  4262,  4265,  4271,  4278,  4285,  4296,
    4297,  4298,  4301,  4309,  4321,  4330,  4341,  4347,  4373,  4374,
    4383,  4384,  4387,  4396,  4407,  4408,  4409,  4410,  4411,  4412,
    4413,  4414,  4415,  4416,  4417,  4418,  4419,  4422,  4423,  4424,
    4425,  4428,  4458,  4459,  4460,  4461,  4464,  4465,  4466,  4467,
    4468,  4471,  4475,  4478,  4482,  4489,  4492,  4508,  4509,  4513,
    4519,  4520,  4526,  4527,  4533,  4534,  4540,  4543,  4544,  4555,
    4561,  4574,  4575,  4578,  4584,  4585,  4586,  4589,  4596,  4601,
    4606,  4609,  4613,  4617,  4623,  4624,  4631,  4637,  4638,  4639,
    4647,  4648,  4651,  4657,  4663,  4667,  4670,  4691,  4695,  4699,
    4709,  4713,  4716,  4722,  4729,  4730,  4731,  4732,  4733,  4736,
    4740,  4744,  4754,  4757,  4758,  4761,  4762,  4763,  4764,  4775,
    4786,  4792,  4813,  4819,  4836,  4842,  4843,  4844,  4847,  4848,
    4849,  4852,  4853,  4856,  4879,  4885,  4891,  4898,  4911,  4919,
    4927,  4933,  4937,  4941,  4945,  4949,  4956,  4961,  4972,  4986,
    4992,  4993,  4994,  4995,  5002,  5010,  5015,  5019,  5023,  5027,
    5033,  5034,  5035,  5036,  5043,  5052,  5073,  5074,  5075,  5076,
    5083,  5091,  5098,  5102,  5112,  5121,  5130,  5139,  5143,  5147,
    5151,  5155,  5159,  5163,  5169,  5170,  5171,  5172,  5176,  5187,
    5193,  5197,  5201,  5208,  5216,  5223,  5231,  5235,  5241,  5247,
    5255,  5256,  5257,  5260,  5261,  5265,  5271,  5272,  5278,  5279,
    5285,  5286,  5292,  5295,  5296,  5297,  5306,  5317,  5318,  5321,
    5329,  5330,  5331,  5332,  5333,  5334,  5335,  5336,  5337,  5338,
    5339,  5340,  5341,  5342,  5345,  5346,  5347,  5348,  5349,  5356,
    5363,  5370,  5377,  5384,  5391,  5398,  5405,  5412,  5419,  5426,
    5433,  5440,  5443,  5444,  5445,  5446,  5447,  5448,  5449,  5452,
    5456,  5460,  5464,  5468,  5472,  5478,  5479,  5489,  5493,  5497,
    5513,  5514,  5517,  5518,  5519,  5520,  5521,  5524,  5525,  5526,
    5527,  5528,  5529,  5530,  5531,  5532,  5533,  5534,  5535,  5536,
    5537,  5538,  5539,  5540,  5541,  5542,  5543,  5544,  5545,  5546,
    5547,  5550,  5570,  5574,  5589,  5593,  5597,  5603,  5607,  5613,
    5614,  5615,  5618,  5619,  5622,  5628,  5629,  5632,  5633,  5636,
    5642,  5643,  5646,  5647,  5650,  5651,  5654,  5655,  5658,  5666,
    5693,  5698,  5703,  5709,  5710,  5713,  5717,  5737,  5738,  5739,
    5740,  5743,  5747,  5751,  5757,  5758,  5761,  5762,  5763,  5764,
    5765,  5766,  5767,  5768,  5769,  5770,  5771,  5772,  5773,  5774,
    5775,  5776,  5777,  5780,  5781,  5782,  5783,  5784,  5785,  5786,
    5789,  5790,  5791,  5792,  5795,  5796,  5797,  5798,  5801,  5802,
    5805,  5811,  5819,  5832,  5838,  5847,  5848,  5849,  5850,  5851,
    5852,  5853,  5854,  5855,  5856,  5857,  5858,  5859,  5860,  5861,
    5862,  5863,  5864,  5865,  5866,  5867,  5868,  5871,  5889,  5898,
    5899,  5900,  5901,  5914,  5920,  5921,  5922,  5925,  5931,  5932,
    5933,  5934,  5935,  5938,  5944,  5945,  5946,  5947,  5948,  5949,
    5950,  5951,  5952,  5955,  5959,  5970,  5977,  5978,  5979,  5980,
    5981,  5982,  5983,  5984,  5985,  5986,  5987,  5988,  5991,  5992,
    5993,  5994,  5997,  5998,  5999,  6000,  6001,  6004,  6010,  6011,
    6012,  6013,  6014,  6015,  6016,  6019,  6025,  6026,  6027,  6028,
    6031,  6037,  6038,  6039,  6040,  6041,  6042,  6043,  6044,  6045,
    6047,  6053,  6054,  6055,  6056,  6057,  6058,  6059,  6060,  6061,
    6062,  6065,  6071,  6072,  6073,  6074,  6075,  6078,  6084,  6085,
    6088,  6094,  6095,  6096,  6099,  6105,  6106,  6107,  6108,  6111,
    6117,  6118,  6119,  6120,  6123,  6127,  6132,  6140,  6147,  6148,
    6149,  6150,  6151,  6152,  6153,  6154,  6155,  6156,  6157,  6158,
    6159,  6160,  6163,  6164,  6165,  6168,  6169,  6172,  6180,  6188,
    6189,  6192,  6200,  6208,  6209,  6212,  6216,  6223,  6224,  6225,
    6228,  6235,  6242,  6243,  6244,  6245,  6246,  6247,  6248,  6249,
    6250,  6251,  6254,  6259,  6264,  6269,  6274,  6279,  6286,  6287,
    6288,  6289,  6290,  6293,  6294,  6295,  6296,  6297,  6298,  6299,
    6300,  6301,  6302,  6303,  6304,  6313,  6314,  6317,  6320,  6321,
    6324,  6327,  6330,  6334,  6345,  6346,  6347,  6350,  6351,  6352,
    6353,  6354,  6355,  6356,  6357,  6358,  6359,  6360,  6361,  6362,
    6363,  6364,  6365,  6366,  6367,  6370,  6371,  6372,  6375,  6376,
    6377,  6378,  6381,  6382,  6383,  6384,  6385,  6388,  6389,  6390,
    6391,  6394,  6399,  6403,  6407,  6411,  6415,  6419,  6424,  6429,
    6434,  6439,  6444,  6451,  6455,  6461,  6462,  6463,  6464,  6467,
    6475,  6476,  6479,  6480,  6481,  6482,  6483,  6484,  6485,  6486,
    6489,  6495,  6496,  6499,  6505,  6506,  6507,  6508,  6511,  6517,
    6523,  6529,  6532,  6538,  6539,  6540,  6541,  6547,  6553,  6554,
    6555,  6556,  6557,  6558,  6561,  6567,  6568,  6571,  6577,  6578,
    6579,  6580,  6581,  6584,  6598,  6599,  6600,  6601,  6602
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
  "\"flowtable\"", "\"handle\"", "\"ruleset\"", "\"trace\"", "\"path\"",
  "\"inet\"", "\"netdev\"", "\"add\"", "\"update\"", "\"replace\"",
  "\"create\"", "\"insert\"", "\"delete\"", "\"get\"", "\"list\"",
  "\"reset\"", "\"flush\"", "\"rename\"", "\"describe\"", "\"import\"",
  "\"export\"", "\"destroy\"", "\"monitor\"", "\"all\"", "\"accept\"",
  "\"drop\"", "\"continue\"", "\"jump\"", "\"goto\"", "\"return\"",
  "\"to\"", "\"constant\"", "\"interval\"", "\"dynamic\"",
  "\"auto-merge\"", "\"timeout\"", "\"gc-interval\"", "\"elements\"",
  "\"expires\"", "\"policy\"", "\"memory\"", "\"performance\"", "\"size\"",
  "\"flow\"", "\"offload\"", "\"meter\"", "\"meters\"", "\"flowtables\"",
  "\"number\"", "\"string\"", "\"quoted string\"",
  "\"string with a trailing asterisk\"", "\"ll\"", "\"nh\"", "\"th\"",
  "\"bridge\"", "\"ether\"", "\"saddr\"", "\"daddr\"", "\"type\"",
  "\"vlan\"", "\"id\"", "\"cfi\"", "\"dei\"", "\"pcp\"", "\"arp\"",
  "\"htype\"", "\"ptype\"", "\"hlen\"", "\"plen\"", "\"operation\"",
  "\"ip\"", "\"version\"", "\"hdrlength\"", "\"dscp\"", "\"ecn\"",
  "\"length\"", "\"frag-off\"", "\"ttl\"", "\"tos\"", "\"protocol\"",
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
  "\"last\"", "\"never\"", "\"tunnel\"", "\"erspan\"", "\"egress\"",
  "\"ingress\"", "\"gbp\"", "\"class\"", "\"opt-type\"", "\"counters\"",
  "\"quotas\"", "\"limits\"", "\"tunnels\"", "\"synproxys\"",
  "\"helpers\"", "\"log\"", "\"prefix\"", "\"group\"", "\"snaplen\"",
  "\"queue-threshold\"", "\"level\"", "\"limit\"", "\"rate\"", "\"burst\"",
  "\"over\"", "\"until\"", "\"quota\"", "\"used\"", "\"secmark\"",
  "\"secmarks\"", "\"second\"", "\"minute\"", "\"hour\"", "\"day\"",
  "\"week\"", "\"reject\"", "\"with\"", "\"icmpx\"", "\"snat\"",
  "\"dnat\"", "\"masquerade\"", "\"redirect\"", "\"random\"",
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
  "close_scope_synproxy", "close_scope_tunnel", "close_scope_xt",
  "common_block", "line", "base_cmd", "add_cmd", "replace_cmd",
  "create_cmd", "insert_cmd", "table_or_id_spec", "chain_or_id_spec",
  "set_or_id_spec", "obj_or_id_spec", "delete_cmd", "destroy_cmd",
  "get_cmd", "list_cmd_spec_table", "list_cmd_spec_any", "list_cmd",
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
  "objref_stmt_synproxy", "objref_stmt_tunnel", "objref_stmt_ct",
  "objref_stmt", "stateful_stmt", "stmt", "xt_stmt", "chain_stmt_type",
  "chain_stmt", "verdict_stmt", "verdict_map_stmt", "verdict_map_expr",
  "verdict_map_list_expr", "verdict_map_list_member_expr",
  "ct_limit_stmt_alloc", "connlimit_stmt", "ct_limit_args", "counter_stmt",
  "counter_stmt_alloc", "counter_args", "counter_arg", "last_stmt_alloc",
  "last_stmt", "last_args", "log_stmt", "log_stmt_alloc", "log_args",
  "log_arg", "level_type", "log_flags", "log_flags_tcp", "log_flag_tcp",
  "limit_stmt_alloc", "limit_stmt", "limit_args", "quota_mode",
  "quota_unit", "quota_used", "quota_stmt_alloc", "quota_stmt",
  "quota_args", "limit_mode", "limit_burst_pkts", "limit_rate_pkts",
  "limit_burst_bytes", "limit_rate_bytes", "limit_bytes", "time_unit",
  "reject_stmt", "reject_stmt_alloc", "reject_with_expr", "reject_opts",
  "nat_stmt", "nat_stmt_alloc", "tproxy_stmt", "synproxy_stmt",
  "synproxy_stmt_alloc", "synproxy_args", "synproxy_arg",
  "synproxy_config", "synproxy_obj", "synproxy_ts", "synproxy_sack",
  "primary_stmt_expr", "shift_stmt_expr", "and_stmt_expr",
  "exclusive_or_stmt_expr", "inclusive_or_stmt_expr", "basic_stmt_expr",
  "concat_stmt_expr", "map_stmt_expr_set", "map_stmt_expr",
  "prefix_stmt_expr", "range_stmt_expr", "multiton_stmt_expr", "stmt_expr",
  "nat_stmt_args", "masq_stmt", "masq_stmt_alloc", "masq_stmt_args",
  "redir_stmt", "redir_stmt_alloc", "redir_stmt_arg", "dup_stmt",
  "fwd_stmt", "nf_nat_flags", "nf_nat_flag", "queue_stmt",
  "queue_stmt_compat", "queue_stmt_alloc", "queue_stmt_args",
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
  "limit_obj", "erspan_block", "erspan_block_alloc", "erspan_config",
  "geneve_block", "geneve_block_alloc", "geneve_config", "vxlan_block",
  "vxlan_block_alloc", "vxlan_config", "tunnel_config", "tunnel_block",
  "tunnel_obj", "relational_expr", "list_rhs_expr", "rhs_expr",
  "shift_rhs_expr", "and_rhs_expr", "exclusive_or_rhs_expr",
  "inclusive_or_rhs_expr", "basic_rhs_expr", "concat_rhs_expr",
  "boolean_keys", "boolean_expr", "keyword_expr", "primary_rhs_expr",
  "relational_op", "verdict_expr", "chain_expr", "meta_expr", "meta_key",
  "meta_key_qualified", "meta_key_unqualified", "meta_stmt", "socket_expr",
  "socket_key", "tunnel_key", "tunnel_expr", "offset_opt", "numgen_type",
  "numgen_expr", "xfrm_spnum", "xfrm_dir", "xfrm_state_key",
  "xfrm_state_proto_key", "xfrm_expr", "hash_expr", "nf_key_proto",
  "rt_expr", "rt_key", "ct_expr", "ct_dir", "ct_key", "ct_key_dir",
  "ct_key_proto_field", "ct_key_dir_optional", "symbol_stmt_expr",
  "list_stmt_expr", "ct_stmt", "payload_stmt", "payload_expr",
  "payload_raw_len", "payload_raw_expr", "payload_base_spec",
  "eth_hdr_expr", "eth_hdr_field", "vlan_hdr_expr", "vlan_hdr_field",
  "arp_hdr_expr", "arp_hdr_field", "ip_hdr_expr", "ip_hdr_field",
  "ip_option_type", "ip_option_field", "icmp_hdr_expr", "icmp_hdr_field",
  "igmp_hdr_expr", "igmp_hdr_field", "ip6_hdr_expr", "ip6_hdr_field",
  "icmp6_hdr_expr", "icmp6_hdr_field", "auth_hdr_expr", "auth_hdr_field",
  "esp_hdr_expr", "esp_hdr_field", "comp_hdr_expr", "comp_hdr_field",
  "udp_hdr_expr", "udp_hdr_field", "udplite_hdr_expr", "udplite_hdr_field",
  "tcp_hdr_expr", "inner_inet_expr", "inner_eth_expr", "inner_expr",
  "vxlan_hdr_expr", "vxlan_hdr_field", "geneve_hdr_expr",
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

#define YYPACT_NINF (-1920)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1120)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1920,  9363, -1920,  1033, -1920, -1920,   168,   174,   174,   174,
     978,   978,   978,   978,   978,   978,   978,   978, -1920, -1920,
    3494,   203,  2879,   307,  2547,   287,  5944,  1288,  1474,   316,
    8429,   148,   297,  3753,   296, -1920, -1920, -1920, -1920,   241,
     978,   978,   978,   978,   978, -1920, -1920, -1920,   763, -1920,
     174, -1920,   174,   164,  7693, -1920,  1033, -1920, -1920,   276,
     283,  1033,   174, -1920,   259,   318,  7693,   174, -1920,   273,
   -1920,   174, -1920, -1920,   978, -1920,   978,   978,   978,   978,
     978,   978,   978,   596,   978,   978,   978,   978,   978, -1920,
     978, -1920,   978,   978,   978,   978,   978,   978,   978,   978,
     629,   978,   978,   978,   978,   978, -1920,   978, -1920,   978,
     978,   978,   978,   978,   978,  1883,   978,   978,  1883,   978,
     978,   262,   978,   978,  1883,   158,   978,   978,  1883,  1883,
    1883,  1883,  1883,   978,   978,   978,  1883, -1920,   978,  1743,
     978,   978,   978,   978,  1883,  1883,   978, -1920,   978,   978,
     978,   978,   978,   502,   978, -1920,   978, -1920,  1476,   880,
     374,   585, -1920, -1920, -1920, -1920,   682,   708,  1499,  2169,
    2721,  1390,   345,  2013,  1872,  1565,   173,   612,  1137,  1291,
    2871,   634,  3969,   322, -1920,  4645,  1195,  2308,   513,   517,
    1000,   534,  1048,   618,  1255,  4618, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,  5658,   265, -1920,
   -1920,   116,  8245,   423,  1189,   632,  8429,   174, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1035,
   -1920, -1920,   429, -1920, -1920,  1035, -1920, -1920,   978,   978,
     978,   978,   978,   978,   978,   978,   629,   978,   978,   978,
     978,   978, -1920, -1920, -1920,  1667, -1920, -1920, -1920,   978,
     978,   978,     6, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,   734,   740,   762, -1920, -1920, -1920,   649,   679,  1006,
   -1920, -1920, -1920,   717, -1920, -1920, -1920,   113,   113, -1920,
     222,   174,  5552,  6240,   688, -1920,   245, -1920,   507,   719,
   -1920, -1920, -1920, -1920, -1920,   201,   854,   673, -1920,   802,
    1018, -1920,   666,  7693, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,   195, -1920,
   -1920,   761,   723, -1920, -1920,   778,   796, -1920,   808, -1920,
   -1920,   728, -1920,  5761, -1920, -1920,   875, -1920,   242, -1920,
     515, -1920, -1920, -1920, -1920,  1178, -1920,   182, -1920, -1920,
   -1920, -1920,  1176,  1062,  1068,   710, -1920,   887, -1920,  7074,
   -1920, -1920, -1920,  1050, -1920, -1920, -1920,  1052, -1920, -1920,
    7470,  7470, -1920, -1920,   146,   733,   736, -1920, -1920,   742,
   -1920, -1920, -1920,   767, -1920,   780,  1078,  7693, -1920,   259,
     318, -1920,   273, -1920, -1920,   978,   978,   978,   864, -1920,
   -1920, -1920, -1920,  7693, -1920,   243, -1920, -1920, -1920,   336,
   -1920, -1920, -1920,   347,   318, -1920, -1920, -1920,   382, -1920,
   -1920,   273, -1920,   540,   788, -1920, -1920, -1920, -1920,   978,
   -1920, -1920, -1920, -1920, -1920,   273, -1920, -1920, -1920,  1136,
   -1920, -1920, -1920, -1920,   978, -1920, -1920,   255, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920,   978,   978, -1920, -1920, -1920,
    1119,  1153, -1920,   978,  1162, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,   978, -1920,
     174, -1920, -1920, -1920,   273, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,   978, -1920,   174, -1920,
   -1920, -1920, -1920,  1203, -1920, -1920, -1920, -1920, -1920,  1175,
     344, -1920, -1920,   900, -1920, -1920,  1135,   210, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,   186,   538, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  1731, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    2839, -1920, -1920, -1920, -1920,  1145, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920,  6008, -1920,  4998, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920,  6419, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,   650, -1920, -1920,   873,
   -1920, -1920, -1920, -1920, -1920, -1920,   876, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  2533,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,   918,   560,   924,
    1174, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
     911,   910, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920,   273, -1920,   788, -1920,   978,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1035, -1920, -1920, -1920, -1920,    24,
     919,    29,   724,   157, -1920, -1920, -1920,  6037,  1215,  7969,
    8429,  1130, -1920, -1920, -1920, -1920,  1290,  1301,   138,  1287,
    1295,  1297, -1920,  1308,  2533,  1324,  7969,  7969,  7969, -1920,
    7969,  8429,   848,  7969,  7969,  1299,  1435, -1920,  7178,   180,
   -1920,  1435, -1920, -1920, -1920,  1007, -1920,  1270, -1920, -1920,
   -1920,  1279,  1302,   761, -1920,   331, -1920, -1920, -1920,   934,
    1435,  1307,  1320,  1331,  1435,   778, -1920, -1920, -1920, -1920,
    1333, -1920, -1920, -1920,  1341, -1920, -1920, -1920,   554, -1920,
   -1920,  7969, -1920, -1920,  6313,  1361,   708,  1499,  2169,  2721,
   -1920,  2013,   994, -1920, -1920, -1920, -1920,  1362, -1920, -1920,
   -1920, -1920,  7969, -1920,  1202,  1440,  1448,  1088,   570,   606,
   -1920, -1920, -1920, -1920,  1453,  1250,  1462, -1920, -1920, -1920,
   -1920, -1920,  1463, -1920, -1920, -1920, -1920, -1920,   154, -1920,
   -1920,  1466,  1469, -1920, -1920, -1920,  1375,  1378, -1920, -1920,
     875, -1920, -1920,  1473, -1920, -1920, -1920, -1920,  1479, -1920,
   -1920,  6589, -1920,  1479, -1920, -1920, -1920,    98, -1920, -1920,
    1178, -1920,  1487, -1920,   174, -1920,  1126, -1920,  9461,  9461,
    9461,  9461,  9461,  8429,   219,  8642, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  9461, -1920, -1920, -1920, -1920, -1920, -1920, -1920,   411,
   -1920,  1216,  1488,  1486,  1128,   338,  1522, -1920, -1920, -1920,
    8642,  7969,  7969,  1426,   170,  1033,  1528, -1920,   899,  1033,
    1444, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    1520,  1188,  1196,  1197, -1920,  1214,  1230, -1920, -1920, -1920,
   -1920,  1310,  1234,  1300,   808,  1435, -1920, -1920,  1508,  1518,
    1519,  1252,  1531, -1920,  1532,  1266, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1527, -1920, -1920, -1920, -1920, -1920,
     978, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920,  1537,   880, -1920, -1920, -1920, -1920, -1920,
    1542, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1073,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1546, -1920,  1456, -1920, -1920,  1452,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1037,
   -1920,  1064,  1523, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    1345,  1699,  1504,  1504, -1920, -1920, -1920,  1421, -1920, -1920,
   -1920, -1920,  1422,  1424, -1920,  1425,  1423,  1432,   819, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1566, -1920,
   -1920,  1567, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1124, -1920,  1183, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1569,  1570,  1325, -1920, -1920, -1920,
   -1920, -1920,  1575,   218, -1920, -1920, -1920,  1312, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,  1315,  1316,  1318,  1578,
   -1920, -1920, -1920,   796, -1920, -1920, -1920,  1580, -1920, -1920,
   -1920, -1920,  7969,  2721,  2013,  1689,  6865, -1920,   182,   198,
    1688,  3279,  1435,  1435,  1592,  8429,  7969,  7969,  7969,  7969,
    1646,  7969, -1920, -1920, -1920, -1920,  1678, -1920, -1920,   557,
     847,   560, -1920,   882,   955,   187,  1657, -1920,  7969, -1920,
   -1920,  1018,  1286,  1079,   199, -1920,  1104,  1522,  1018, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  1563,   558, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920,   196,  1394,  1399,  1703,   132,  1029,  1043, -1920,
    1150, -1920, -1920, -1920,  7969,  1719,  7969, -1920, -1920, -1920,
     375,   745, -1920,  7969, -1920, -1920,  1352, -1920, -1920,  7969,
    7969,  7969,  7969,  7969,  1626,  7969,  7969,   264,  7969,  1479,
    7969,  1653,  1736,  1659,  4323,  4323, -1920, -1920, -1920,  7969,
    1250,  7969,  1250, -1920,  1725,  1726, -1920,   848, -1920,  8429,
   -1920, -1920,  1216,  1488,  1486, -1920,  1018, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1380,  9461,  9461,  9461,  9461,  9461,
    9461,  9461,  9461,  9573,  9461,  9461,   707, -1920,  1056, -1920,
   -1920, -1920, -1920, -1920,  1654, -1920,   750,  1938, -1920,  3241,
    3945,  2877,  3278,   979, -1920, -1920, -1920, -1920, -1920, -1920,
    1385,  1389,  1392, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1758,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,  3279, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  1386,  1388, -1920, -1920, -1920, -1920, -1920, -1920,  1325,
     -27,  1668, -1920, -1920, -1920, -1920, -1920,  1221, -1920, -1920,
   -1920, -1920, -1920,  1481,   476, -1920,  2793,  1333,   961, -1920,
     920,   132, -1920,   204, -1920, -1920,  7969,  7969,  1767, -1920,
   -1920,  1674,  1674, -1920,   198, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1407,  1688,  7693,   198, -1920, -1920,
   -1920, -1920,  7969, -1920, -1920, -1920, -1920, -1920,    98, -1920,
    8429,    98,  7969,  1730, -1920,  9376, -1920,  1589, -1920,  1468,
   -1920, -1920, -1920, -1920, -1920, -1920,  1079, -1920,  1690,  1674,
   -1920,   830, -1920,  7178, -1920,  5377, -1920, -1920, -1920, -1920,
    1785, -1920,  1373,  1775, -1920,  1694, -1920,  1695, -1920,  1373,
   -1920, -1920,  1480, -1920,  1225, -1920, -1920,  1225, -1920,  1737,
    1225, -1920, -1920,  7969, -1920, -1920, -1920, -1920, -1920,  1202,
    1440,  1448, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1809,
    7969,  1651,  7969, -1920, -1920, -1920, -1920,  1250, -1920,  1250,
    1479, -1920, -1920,   586,  7324,   181, -1920, -1920, -1920,  1528,
    1815, -1920, -1920,  1216,  1488,  1486, -1920,   239,  1528, -1920,
   -1920,  1104,  9461,  9573, -1920,  1728,  1794, -1920, -1920, -1920,
   -1920, -1920,   174,   174,   174,   174,   174,  1734,   645,   174,
     174,   174,   174,   174, -1920, -1920, -1920,  1033, -1920,  1467,
     145, -1920,  1735, -1920, -1920, -1920,  1033,  1033,  1033,  1033,
    1033,  9047, -1920,  1674,  1674,  1482,  1248,  1732,  1238,  1511,
   -1920, -1920, -1920,  1033,  1033,  1033,   512, -1920,  9047,  1674,
    1674,  1483,  1238,  1511, -1920, -1920, -1920,  1033,  1033,   512,
    1739,  1484,  1747, -1920, -1920, -1920, -1920, -1920,  5569,  4304,
    3158,  5212,  1027, -1920, -1920, -1920, -1920,  3867, -1920, -1920,
   -1920,  4663,  1158, -1920, -1920,  1749, -1920, -1920, -1920,  1853,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1757,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1201, -1920,   508,
    1289,  1682,  1759, -1920, -1920, -1920, -1920, -1920,  1761,  1258,
    1765,  1773,  1275,  1774,  1777, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920,  1033,  1394,  1399, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920,  1480, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920,  7969, -1920, -1920, -1920, -1920, -1920, -1920,
    8429,  1507,   198, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920,  7969,   113,   113,  1018,  1522,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    1079, -1920, -1920, -1920,  1033, -1920,   558, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1587,   165, -1920, -1920,  1781, -1920,
   -1920, -1920, -1920, -1920, -1920,  7969, -1920,  1800, -1920,  1479,
    1479,  8429, -1920,  1083,  1889,  1018, -1920,  1528,  1528,  1705,
    1783, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920,  1892, -1920,   174,   174,   174, -1920, -1920, -1920,
   -1920, -1920, -1920,   521, -1920, -1920, -1920, -1920, -1920,  1798,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1898, -1920,  1033,
    1033,   273, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920,  1899, -1920, -1920, -1920, -1920, -1920,   871,
   -1920, -1920, -1920, -1920, -1920, -1920,   935,  1033,  1033,   273,
    1012,   871, -1920, -1920, -1920,  1754,   521,  1033, -1920, -1920,
   -1920, -1920, -1920, -1920,   821,  1640,  2442, -1920, -1920, -1920,
   -1920, -1920,  1805, -1920,  1325, -1920, -1920, -1920,  1539,   807,
     978, -1920, -1920, -1920, -1920, -1920,  1674,  1806,   807,  1810,
     978, -1920, -1920, -1920, -1920, -1920,  1811,   978, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,   228,   228, -1920, -1920,
     228,   228, -1920, -1920,  1545,  1555,  1557, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,  7693, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,   132, -1920, -1920,
   -1920, -1920, -1920,  7969,  1541,  8429, -1920,  2449,  7324, -1920,
   -1920,  1746,  1033,  1558,  1564,  1568,  1573,  1574,  1734, -1920,
   -1920, -1920,  1576,  1582,  1583,  1585,  1586,   228,  1033, -1920,
   -1920,  1908,  9047, -1920, -1920, -1920, -1920,  1238, -1920,  1511,
   -1920,  8788, -1920, -1920, -1920,   489, -1920,   237,  1033,  1033,
   -1920, -1920, -1920, -1920, -1920, -1920,  1933, -1920,  1594, -1920,
   -1920,  1033,  1033, -1920,  1033,  1033,  1033,  1033,  1033, -1920,
    1804,  1033, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    1579, -1920, -1920, -1920, -1920, -1920,  1599,  1018, -1920, -1920,
    1705, -1920, -1920, -1920, -1920, -1920, -1920,  1595,  1601,  1603,
   -1920, -1920, -1920, -1920, -1920,   184, -1920, -1920, -1920, -1920,
    1856, -1920, -1920, -1920, -1920,  9047, -1920,  4201, -1920, -1920,
   -1920, -1920, -1920, -1920,  1033,  1971, -1920,  1033,  1973, -1920,
    1033,  1238,  1877, -1920, -1920, -1920,  1055, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920,  1749, -1920,  1879, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,   807, -1920, -1920,
   -1920, -1920, -1920,  1540,  1471,  1236, -1920, -1920, -1920,  1746,
    2731,  5022,  3259,  5425,  1278, -1920, -1920, -1920,  2536,  5203,
    2784,   927,  2484,   162, -1920,  1372, -1920,  1898, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920,  9047, -1920, -1920,  1005,
   -1920,  1881,  1886, -1920,  1987,   191, -1920,  1033,  1891, -1920,
   -1920, -1920,  1033,  1896, -1920, -1920, -1920,  1033,  1903,  1904,
    1115,  1910, -1920, -1920, -1920,  1033, -1920,  1033,  1033,  1033,
    1033,  1033,  1395,  2107,  2519,  1033,  1033,  1033,  1033,  1033,
   -1920, -1920,   228,  1623,  1754, -1920,  1973, -1920, -1920, -1920,
   -1920,  1322,  1879,  1033, -1920, -1920, -1920,  1696, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
    1033,  1033,  1033, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
     521, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,  1912,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  1795, -1920, -1920, -1920,  1435, -1920
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     1,     0,     4,     5,     0,     0,     0,     0,
     445,   445,   445,   445,   445,   445,   445,   445,   449,   452,
     445,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   237,   451,     9,    28,    29,     0,
     445,   445,   445,   445,   445,    69,    68,     3,     0,    72,
       0,   446,     0,   470,     0,    67,     0,   437,   438,     0,
       0,     0,     0,   639,    88,    90,     0,     0,   298,     0,
     321,     0,   351,    73,   445,    74,   445,   445,   445,   445,
     445,   445,   445,     0,   445,   445,   445,   445,   445,    75,
     445,    76,   445,   445,   445,   445,   445,   445,   445,   445,
       0,   445,   445,   445,   445,   445,    77,   445,    78,   445,
     476,   445,   476,   445,   476,   476,   445,   445,   476,   445,
     476,     0,   445,   476,   476,     0,   445,   445,   476,   476,
     476,   476,   476,   445,   445,   445,   476,    35,   445,   476,
     445,   445,   445,   445,   476,   476,   445,    47,   445,   445,
     445,   445,   476,     0,   445,    81,   445,    82,     0,     0,
       0,   795,   763,   439,   440,   441,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1038,     0,     0,  1040,
    1039,     0,     0,     0,     0,    34,     0,     0,    86,   759,
     758,   777,   778,   779,   252,   774,   775,   767,  1008,   769,
     768,   772,   776,   773,   770,   771,   764,  1125,  1126,  1127,
    1128,  1129,  1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,
    1138,    53,  1143,  1144,  1145,  1146,  1140,  1141,  1142,   765,
    1392,  1393,  1394,  1395,  1396,  1397,  1398,  1399,   766,     0,
     249,   250,     0,    33,   233,     0,    21,   235,   445,   445,
     445,   445,   445,   445,   445,   445,     0,   445,   445,   445,
     445,   445,    16,   238,    39,   239,   450,   447,   448,   445,
     445,   445,    13,   929,   889,   862,   864,    71,    70,   453,
     455,     0,     0,     0,   472,   471,   473,     0,   630,     0,
     749,   750,   751,     0,   999,  1000,  1001,   527,   528,  1004,
       0,     0,     0,     0,   545,   550,     0,   557,     0,   587,
     608,   620,   621,   698,   704,   725,     0,     0,  1044,     0,
       7,    93,   478,   480,   492,   493,   494,   495,   497,   496,
     525,   507,   481,    62,   276,   522,   503,   531,     0,    12,
      13,   543,   551,    14,    59,   555,   592,    36,   582,    44,
      46,   611,    40,     0,    54,    60,   628,    40,   697,    40,
     703,    18,    24,   513,    45,   723,   519,     0,   520,   505,
     504,   797,   800,   802,   804,   806,   807,   814,   816,     0,
     815,   756,   530,  1008,   508,   514,   506,   764,   523,    63,
       0,     0,    66,   464,     0,     0,     0,    92,   458,     0,
      96,   314,   313,     0,   461,     0,     0,     0,   639,   114,
     116,   298,     0,   321,   351,   445,   445,   445,    13,   929,
     889,   862,   864,     0,    60,     0,   139,   140,   141,     0,
     133,   134,   142,     0,   135,   136,   144,   145,     0,   137,
     138,     0,   146,     0,   148,   149,   866,   867,   865,   445,
      13,    61,    36,    44,    51,     0,    60,   206,   477,   210,
     179,   180,   181,   182,   445,   177,   183,   477,   176,   178,
     184,   203,   202,   201,   195,   445,   476,   199,   198,   200,
     866,   867,   868,   445,     0,    13,    61,   185,   187,   189,
     207,   193,    36,    44,    51,   191,    79,   220,   445,   217,
     176,   218,   216,   222,     0,   223,    13,   212,   214,    44,
      80,   224,   225,   226,   227,   230,   445,   229,     0,  1152,
    1149,  1150,    56,     0,   786,   787,   788,   789,   790,   792,
       0,  1049,  1051,     0,  1050,    52,     0,     0,  1390,  1391,
      56,  1154,  1155,    55,    20,    55,  1158,  1159,  1160,  1161,
      30,     0,     0,  1164,  1165,  1166,  1167,  1168,     9,  1186,
    1187,  1181,  1176,  1177,  1178,  1179,  1180,  1182,  1183,  1184,
    1185,     0,    28,    55,  1201,  1200,  1199,  1202,  1203,  1204,
      31,    55,  1207,  1208,  1209,    32,  1218,  1219,  1211,  1212,
    1213,  1215,  1214,  1216,  1217,    29,  1230,    55,  1226,  1223,
    1222,  1227,  1225,  1224,  1228,  1229,    31,  1233,  1236,  1232,
    1234,  1235,     8,  1239,  1238,    19,  1241,  1242,  1243,    11,
    1247,  1248,  1245,  1246,    57,  1253,  1250,  1251,  1252,    58,
    1300,  1294,  1297,  1298,  1292,  1293,  1295,  1296,  1299,  1301,
       0,  1254,    55,  1334,  1335,     0,    15,  1280,  1279,  1272,
    1273,  1274,  1258,  1259,  1260,  1261,  1262,  1263,  1264,  1265,
    1266,  1267,    53,  1276,  1275,  1278,  1277,  1269,  1270,  1271,
    1287,  1289,  1288,     0,    25,     0,  1284,  1283,  1282,  1281,
    1388,  1385,  1386,     0,  1387,    49,    55,    28,  1405,  1079,
      29,  1404,  1407,  1077,  1078,    34,     0,    48,    48,     0,
      48,  1411,    48,  1414,  1413,  1415,     0,    48,  1402,  1401,
      27,  1423,  1420,  1418,  1419,  1421,  1422,    23,  1426,  1425,
      17,    55,  1429,  1432,  1428,  1431,    38,    37,  1012,  1013,
    1014,    51,  1015,    34,    37,  1010,  1011,  1094,  1095,  1101,
    1087,  1088,  1086,  1096,  1097,  1117,  1090,  1099,  1092,  1093,
    1098,  1089,  1091,  1084,  1085,  1115,  1114,  1116,    51,     0,
      12,  1102,  1052,  1053,  1054,  1058,  1057,     0,   814,     0,
       0,    48,    27,    23,    17,    38,  1433,  1062,  1063,  1037,
    1061,     0,   757,  1139,   232,   251,    83,   234,    84,    60,
     158,   159,   135,   160,   161,     0,   162,   164,   165,   445,
      13,    61,    36,    44,    51,    87,    85,   240,   241,   243,
     242,   245,   246,   244,   247,   886,   886,   886,    98,     0,
       0,     0,   582,     0,   467,   468,   469,     0,     0,     0,
       0,     0,  1006,  1005,  1002,  1003,     0,     0,     0,    37,
      37,     0,   539,     0,     0,    12,     0,     0,     0,   576,
       0,     0,     0,     0,     0,     0,     0,     6,     0,     0,
     818,     0,   479,   482,   524,     0,   541,     0,   540,   501,
     498,     0,     0,   544,   546,     0,   552,   502,   509,     0,
       0,     0,     0,     0,     0,   556,   558,   590,   591,   577,
       0,   499,   580,   581,     0,   588,   500,   510,     0,   607,
     511,     0,    47,    16,     0,     0,    20,    30,     9,    28,
     967,    29,     0,   972,   970,   971,    14,     0,    40,    40,
     957,   958,     0,   659,   662,   664,   666,   668,   669,   674,
     679,   677,   678,   680,   682,   619,   644,   645,   656,   657,
     959,   646,   654,   647,   655,   649,   651,   652,     0,   648,
     650,     0,   681,   653,   512,   521,     0,     0,   636,   635,
     629,   631,   515,     0,   716,   717,   718,   696,   701,   714,
     516,     0,   702,   707,   517,   518,   719,     0,   741,   742,
     724,   726,   729,   739,     0,   761,     0,   760,     0,     0,
       0,     0,     0,     0,     0,     0,   992,   993,   994,   995,
     996,   997,   998,    20,    30,     9,    28,    31,   984,    29,
      31,     8,    19,    11,    57,    58,    53,    15,    25,    49,
      40,     0,   974,   942,   975,   811,   812,   954,   941,   931,
     930,   946,   948,   950,   952,   953,   940,   976,   977,   943,
       0,     0,     0,     0,     7,     0,   856,   855,   953,     0,
       0,   406,    60,   259,   277,   301,   333,   352,   474,   113,
       0,     0,     0,     0,   120,     0,     0,   886,   886,   886,
     122,     0,     0,     0,   582,     0,   132,   156,     0,     0,
       0,     0,     0,   147,     0,     0,   886,   151,   157,   154,
     152,   155,   174,   194,     0,   211,   175,   197,   196,    12,
     445,   186,   208,   190,   188,   192,   219,   221,   213,   215,
     228,   231,  1151,     0,     0,   785,    55,   782,   783,    22,
       0,  1047,   796,    42,    42,  1389,  1156,  1153,  1162,  1157,
      20,    28,    20,    28,  1163,  1188,  1189,  1190,  1191,    28,
    1173,  1198,  1197,  1206,  1205,  1210,  1221,  1220,  1231,  1237,
    1240,  1244,  1249,    10,  1318,  1324,  1322,  1313,  1314,  1317,
    1319,  1308,  1309,  1310,  1311,  1312,  1320,  1315,  1316,  1321,
    1256,  1323,  1255,  1336,    15,  1332,  1268,  1286,  1285,  1290,
    1340,  1337,  1338,  1339,  1341,  1342,  1343,  1344,  1345,  1346,
    1347,  1348,  1349,  1350,  1351,  1352,  1353,  1354,  1371,    50,
    1383,  1406,  1073,  1074,  1080,    48,  1075,  1403,     0,  1408,
    1410,     0,  1412,  1400,  1417,  1424,  1430,  1427,  1009,  1016,
    1007,  1100,  1103,  1104,     0,  1106,     0,  1105,  1107,  1108,
      12,    12,  1109,  1081,     0,     0,  1055,  1435,  1434,  1436,
    1437,  1438,     0,     0,   780,   172,   163,     0,   886,   167,
     173,   170,   168,   171,   236,   248,     0,     0,     0,     0,
     371,    13,   924,   592,   396,    36,   376,     0,    44,   401,
     863,    51,     0,    28,    29,   622,     0,    60,     0,   743,
     745,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1109,     0,    13,    61,    36,    44,     0,   738,    45,   733,
     732,     0,   737,   735,   736,     0,   710,   712,     0,   526,
     831,     7,     7,   833,   827,   830,   953,   852,     7,   817,
     475,   286,   542,   548,   549,   547,   442,   553,   554,   571,
      20,     0,     0,   569,   565,   560,   561,   562,   563,   566,
     564,   559,     0,   593,   596,     0,     0,     0,     0,    53,
       0,   688,   968,   969,     0,   683,     0,   960,   963,   964,
     961,   962,   973,     0,   966,   965,     0,   644,   654,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   690,
       0,     0,     0,     0,     0,     0,   633,   634,   632,     0,
       0,     0,   705,   728,   733,   732,   727,     0,    10,     0,
     798,   799,   801,   803,   805,   808,     7,   532,   534,   813,
     961,   983,   962,   985,   982,   981,   987,   979,   980,   978,
     988,   986,   989,   990,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   937,   936,   953,  1042,
    1124,   858,   857,    64,     0,    65,     0,     0,   110,     0,
       0,     0,     0,     0,    60,   259,   277,   301,   333,   352,
       0,     0,     0,    13,   924,    36,    44,    51,   465,   454,
     456,   277,   459,   462,   352,    12,   209,   204,    12,     0,
     791,   784,   781,    52,   793,   794,  1169,  1171,  1170,  1172,
      55,  1193,  1195,  1194,  1196,  1175,    28,     0,  1330,  1302,
    1327,  1304,  1331,  1307,  1328,  1329,  1305,  1325,  1326,  1303,
    1306,  1333,  1368,  1367,  1369,  1370,  1376,  1358,  1359,  1360,
    1361,  1373,  1362,  1363,  1364,  1365,  1366,  1374,  1375,  1377,
    1378,  1379,  1380,  1381,  1382,    55,  1357,  1356,  1372,    49,
    1076,     0,     0,    28,    28,    29,    29,  1082,  1083,  1055,
    1055,     0,    26,  1060,  1064,  1065,    34,     0,   352,    12,
     386,   391,   381,     0,     0,    99,     0,     0,     0,   106,
       0,     0,   101,     0,   108,   624,     0,     0,   623,   488,
     746,     0,     0,   839,   744,   834,  1318,  1322,  1317,  1321,
    1323,    53,    10,    10,     0,   826,     0,   824,    37,    37,
      12,    12,     0,    12,   485,   489,   486,   487,     0,   720,
       0,     0,     0,     0,   821,     0,   822,     0,   545,     0,
     587,    12,    13,    14,    36,    44,   832,   842,     0,     0,
     851,   828,   840,   820,   819,     0,   570,    28,   574,   575,
      53,   573,     0,   598,   600,     0,   578,     0,   579,     0,
     584,   583,   585,   609,     0,   613,   610,     0,   615,     0,
       0,   617,   689,     0,   693,   695,   658,   660,   661,   663,
     665,   667,   675,   676,   670,   673,   672,   671,   685,   684,
       0,     0,     0,  1118,  1119,  1120,  1121,   699,   715,   706,
     708,   740,   762,     0,     0,     0,   535,   991,   939,   933,
       0,   944,   945,   947,   949,   951,   938,   809,   932,   810,
     955,   956,     0,     0,   809,     0,     0,    60,   408,   407,
     410,   409,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    89,   261,   260,     0,   255,     0,
       0,    55,     0,    91,   279,   278,     0,     0,     0,     0,
       0,     0,   310,     0,     0,     0,     0,     0,     0,     0,
      94,   303,   302,     0,     0,     0,     0,   483,     0,     0,
       0,     0,     0,     0,    95,   335,   334,     0,     0,     0,
       0,     0,     0,    13,    97,   354,   353,   130,     0,     0,
       0,     0,     0,   386,   391,   381,   123,     0,   128,   124,
     129,     0,     0,   153,   205,     0,  1048,  1192,  1174,     0,
    1355,  1384,  1409,  1416,  1110,  1111,  1112,  1113,    41,     0,
      26,  1056,  1072,  1068,  1067,  1066,    34,     0,   169,     0,
       0,     0,     0,    13,   373,   372,   375,   374,     0,     0,
       0,     0,     0,     0,     0,   910,   904,   894,    61,   926,
     925,   928,     0,   593,   596,    36,   398,   397,   400,   399,
      44,   378,   377,   380,   379,   585,    51,   403,   402,   405,
     404,   625,   627,     0,   837,   838,   835,  1291,  1046,  1045,
       0,     0,   825,  1043,  1041,   490,   491,    12,  1122,   734,
     730,   731,    45,    45,   711,     0,     0,     0,     7,   853,
     854,   846,   844,   848,   845,   847,   843,   836,   849,   850,
     829,   841,   529,   287,     0,   568,     0,   567,   602,   603,
     604,   605,   606,   595,     0,     0,   597,   599,     0,   589,
      55,    55,    47,    55,   686,     0,   692,     0,   694,   700,
     709,     0,   747,     0,     0,     7,   533,   935,   934,   640,
       0,   111,   466,   370,   457,   276,   460,   298,   321,   463,
     351,   258,   254,   256,     0,     0,     0,   370,   370,   370,
     370,   370,   262,     0,   435,   436,    43,   434,   433,     0,
     431,   280,   282,   281,   285,   283,   296,   299,   295,     0,
       0,     0,   350,   349,    43,   348,   411,   413,   414,   412,
     367,   415,   368,    55,   366,   317,   318,   320,   319,     0,
     316,   304,   311,   312,   308,   484,     0,     0,     0,     0,
       0,     0,   346,   345,   343,     0,     0,     0,   357,   115,
     117,   118,   119,   121,     0,     0,     0,    61,   143,   150,
    1147,    10,     0,  1059,  1055,  1071,  1069,   166,     0,     0,
       0,    12,   388,   387,   390,   389,     0,     0,     0,     0,
       0,    12,   393,   392,   395,   394,     0,     0,    12,   383,
     382,   385,   384,   859,   100,   912,     0,     0,   919,   920,
       0,     0,   917,   918,     0,     0,     0,   112,   927,   887,
     888,   107,   102,   861,   109,   626,     0,   754,  1123,   721,
     722,   713,   823,   288,   572,   594,   601,     0,    31,    31,
     618,   616,   687,     0,     0,     0,   748,     0,   537,   536,
     641,   642,     0,     0,     0,     0,     0,     0,     0,   370,
     370,   370,     0,     0,     0,     0,     0,     0,     0,   360,
     432,     0,     0,   306,   307,   309,   347,     0,   300,     0,
     305,     0,   336,   337,   344,   332,   342,     0,     0,     0,
     358,    12,    12,    12,   131,  1148,     0,    26,     0,    57,
      53,     0,     0,   104,     0,     0,     0,     0,     0,   105,
       0,     0,   103,    28,    28,    29,    29,   906,   900,   890,
       0,   586,   612,   614,   691,   752,     0,     7,   643,   637,
     640,   406,   277,   301,   333,   352,   257,     0,     0,     0,
     371,   924,   396,   376,   401,     0,   361,   364,   365,   284,
       0,   297,   369,   315,    60,     0,   294,     0,    13,    36,
      44,    51,   293,   292,     0,     0,   329,     0,   289,   331,
       0,     0,     0,   425,   419,   418,   422,   417,   420,   421,
     355,   356,   126,   127,   125,     0,  1070,     0,   872,   871,
     878,   880,   883,   884,   881,   882,   885,     0,   874,   913,
     914,   915,   916,     0,     0,     0,   755,   753,   538,   642,
       0,     0,     0,     0,     0,   386,   391,   381,     0,     0,
       0,     0,     0,     7,   359,   430,   328,   291,   322,   323,
      12,   324,   326,   325,   327,   339,     0,   341,    55,     0,
     426,     0,     0,  1257,     0,     0,   875,     0,     0,   922,
     908,   907,     0,     0,   923,   902,   901,     0,     0,     0,
       0,     0,   921,   892,   891,     0,   638,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     363,   362,     0,     0,     0,   330,   290,   340,    55,   424,
     423,     0,     0,     0,    55,   911,   909,     0,   903,   899,
     895,   898,   897,   896,   893,    60,   263,   264,   265,   266,
       0,     0,     0,    13,    61,    36,    44,    51,   427,   428,
       0,   416,   338,   443,   444,   877,   876,    43,   873,     0,
     274,    12,    12,    12,   267,   275,   272,   268,   273,   429,
     879,     0,   270,   271,   269,     0,   905
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1920, -1920,    -1, -1313,   970,    41, -1365,   971,  -188,  -362,
    -890,  -740,  1082,  1212, -1920,   983,  -534, -1920, -1920,  1217,
   -1920,  -152, -1682,  1219,   -18,   -37,  1446,  -629, -1920, -1920,
    -708, -1920,  -444,  -728,  1223, -1920,  -315, -1920,   877, -1870,
    -431, -1263, -1920,  -914,  -526,  -948, -1920,  -525,   516,  -686,
   -1920,  -578,  1460, -1029,   987, -1920,  -442,  -523, -1920,    23,
   -1920, -1920,  2026, -1920, -1920, -1920,  1760,  1768,   279,  1454,
   -1920, -1920, -1920,  1911,  2364, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920,    -2, -1920,  1598,
   -1920, -1920,  -105,   569,  -360, -1428, -1920, -1920, -1920, -1919,
   -1664,  -436, -1920, -1444,  -401,   252,  -120,  -445, -1920,  -125,
   -1920, -1920, -1427, -1443, -1920,  -448, -1436, -1890, -1920,  -262,
    -115, -1692,    95,  -174,  -173, -1716, -1701, -1698,  -171,  -172,
    -156, -1920, -1920,  -308, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920,   289,  -770, -1557, -1920,   288,   -23,  3618, -1920,
     167, -1920, -1920,  2601, -1920,   328,   325,  1792, -1920,   959,
   -1920,  -975,  1648, -1920, -1920,   312,   291,  1121,   303,   -43,
   -1920, -1920, -1440, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1451,  -359, -1920, -1920, -1920, -1920, -1920, -1920, -1920,   111,
   -1920, -1274, -1920, -1272, -1920, -1920,  1187, -1920, -1255, -1920,
   -1920, -1920, -1920,  1184, -1920, -1920, -1920,   155, -1920, -1238,
   -1920,  1733, -1526,   208, -1920, -1236, -1920,   816,   227,   523,
     230,   524,   435,   436, -1920, -1920,  -941, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1133,  -358,  1693,   -98,  -175,  -449,
     738,   739,   737, -1920,  -807, -1920, -1920, -1920, -1920, -1920,
   -1920,  1686, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920,  -379,   725, -1920, -1920, -1920, -1920,  1142,   509,  -943,
     506,  1271,   729, -1367, -1920, -1920,  1838, -1920, -1920, -1920,
     -59,  1660,  -862,  -378,  2204, -1661,  2142,     3, -1920, -1920,
    1019,     4, -1920, -1920, -1920, -1920, -1920,  -195,  -217, -1920,
   -1920,   703,  -830,  1957,   -56, -1920,   823,   263, -1920, -1575,
   -1920, -1920,   545, -1920, -1424, -1920,   519, -1459,   520, -1920,
   -1920,  1756,  -624,  1738,  -622,  1740,  -613,  1741,   149, -1920,
   -1839, -1920, -1920,  -210, -1920, -1920,  -608,  -562,  1742, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1462,
    1750, -1920,  -399,  -333,  -905,  -880,  -874, -1920,  -249,  -872,
   -1920,  1745,  -296,  -826, -1920, -1545,  -326,   107,  1857, -1920,
     -10, -1920,   254, -1920, -1920,   260, -1450, -1920,   319, -1920,
   -1920, -1920, -1920, -1920,   324,  -290,   442,  1472,   518,  1861,
    1869, -1920, -1920,  -513,    10, -1920, -1920, -1920,   791,   -77,
   -1920, -1920,   -44, -1920,   -11, -1920,    -5, -1920,   -58, -1920,
   -1920, -1920,   -40, -1920,   -36, -1920,   -29, -1920,   -25, -1920,
     -17, -1920,   -16, -1920,     5, -1920,    11, -1920,    12, -1920,
      16,  1506, -1920,   -86, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920,  1543, -1097, -1920, -1920, -1920, -1920,
   -1920,    28, -1920, -1920, -1920, -1920,   998, -1920, -1920,    33,
   -1920,    37, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920, -1920,
   -1920, -1920
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,  1825,   898,  1188,  1399,  1537,  1190,  1273,   858,
     917,  1215,   845,  1255,  1014,  1189,  1397,   828,  1522,  1254,
    1015,   713,  1862,  1253,  1450,  1452,  1398,  1182,  1184,   826,
     819,   536,   931,  1258,  1257,   846,   940,  2083,  1524,  2180,
     936,  1016,   937,   550,  1246,  1240,  1579,  1131,  1161,   823,
     994,  1166,  1152,  1191,  1192,   918,   995,  1128,   904,  1826,
      47,    48,    49,    75,    89,    91,   468,   472,   477,   464,
     106,   302,   108,   505,   506,   137,  1135,   497,   147,   155,
     157,   283,   286,   304,   305,   854,  1294,   284,   228,   435,
    1777,  2002,  2003,  1489,   436,  1490,  1675,  2273,  2274,  2026,
    2027,   439,  1803,  1491,   440,  2049,  2050,   443,  2340,  2276,
    2277,  2281,  1492,  1804,  2034,   445,  1493,  2178,  2255,  2256,
    2042,  2043,  2163,  1604,  1610,  1871,  1869,  1870,  1608,  1613,
    1487,  2044,  1786,  2198,  2286,  2287,  2288,  2394,  1787,  1788,
    2016,  2017,  1992,   229,  1368,  2435,    50,    51,    62,   471,
      53,   475,  1995,   479,   480,  1997,    72,   485,  2000,   466,
     467,  1993,   324,   325,   326,    54,   447,  1623,   509,  1790,
     362,   363,  1806,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,  1447,  1735,  1736,
     378,   379,   908,   380,   381,   913,   914,   382,   383,   916,
     384,   385,   925,   926,  1380,  1374,  1680,  1681,   386,   387,
     929,  1307,  1692,  1969,   388,   389,   935,   930,  1686,  1383,
    1688,  1384,  1385,  1963,   390,   391,  1695,   939,   392,   393,
     394,   395,   396,  1000,  1001,  1761,   434,  2161,  2239,   963,
     964,   965,   966,   967,   968,   969,  1715,   970,   971,   972,
     973,   974,   975,   397,   398,  1007,   399,   400,  1012,   401,
     402,  1008,  1009,   403,   404,   405,  1020,  1021,  1336,  1337,
    1338,  1022,  1023,  1318,  1319,   406,   407,   408,   409,   410,
     230,   976,  1026,  1063,   977,   233,   411,   978,  1159,   569,
     570,   979,   577,   412,   413,   414,   415,   416,   417,  1065,
    1066,  1067,   418,   419,   420,   899,   900,  1636,  1637,  1352,
    1353,  1354,  1624,  1948,  1625,  1671,  1666,  1667,  1672,  1355,
    1938,  1085,  1877,   859,  1904,   862,  1910,   863,   489,   524,
    2211,  2112,  2355,  2356,  2095,  2105,  1296,  1899,   861,  2315,
    2126,  2375,  2314,  2125,  2367,  2313,  2124,  2362,  1892,  1606,
     860,   421,  1086,  1087,  1071,  1072,  1073,  1074,  1356,  1076,
     980,   981,   982,  1079,  1080,   422,   874,   983,   774,   775,
     238,   424,   984,   575,   804,   985,  1592,   807,   986,  1283,
     820,  1596,  1866,   242,   987,   736,   989,   737,   990,   799,
     800,  1270,  1271,   801,   991,   992,   425,   426,   993,  2081,
     247,   563,   248,   584,   249,   590,   250,   598,   251,   612,
    1179,  1536,   252,   620,   253,   625,   254,   635,   255,   646,
     256,   652,   257,   655,   258,   659,   259,   664,   260,   669,
     261,   703,   704,   705,   262,   706,   263,   719,   264,   714,
     265,   428,   681,  1210,  1630,  1212,  1549,  1541,  1546,  1539,
    1543,   266,   686,  1238,  1578,  1561,  1567,  1556,  1239,   267,
     725,   268,   580,   269,   270,   750,   271,   738,   272,   740,
     273,   742,   274,   747,   275,   757,   276,   760,   277,   766,
     278,   816
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      45,   308,    55,  1142,   903,   808,  1106,  1168,  1105,  1145,
     441,   361,   875,   442,   905,  1103,  1216,  1187,   910,   307,
    1069,  1013,  1117,   437,    46,  1457,  1357,  1244,  1392,  1027,
     287,   821,   715,   235,   236,  1181,  1125,   868,  1654,  1656,
    1807,  1807,  1837,  1183,   423,  1673,  1260,   318,  1129,  1817,
    1167,  1104,  1819,  1830,  1133,   429,   423,   235,   236,  1186,
     432,  1342,  1130,  1832,  1914,  1915,  1402,   895,  1829,   235,
     236,  1831,  1733,  1732,  1433,  1649,  1092,   306,  1842,  1661,
    1123,  1662,  1002,  1841,  1010,  1905,  1070,   498,  1143,   498,
    1102,   498,   507,  1310,  1132,   507,  1110,   498,  1663,   718,
     498,   507,  1144,   988,  1213,   507,   507,   507,   507,   507,
    1940,  1462,  1949,   507,  1121,  1664,   507,  1665,  1149,  2076,
    2060,   507,   507,  1078,   692,  1442,  1349,   692,  1127,   498,
    2028,  1360,  2074,  1734,  1078,  1078,  2075,   237,   689,  1858,
    1860,   689,   693,  1147,  2056,   693,   694,  2028,  1241,   694,
    1375,  1443,  1260,   695,  1379,  1406,   695,   696,  1444,  1984,
     696,   237,  1867,  1141,  2186,   697,   698,   897,   697,   698,
    1075,   690,  1448,   237,   690,   897,  2199,   691,  2085,    65,
     691,  1088,  1088,  1256,  1148,   776,  1090,   699,  1358,  1985,
     699,   909,  2333,   700,   701,  1437,   700,   701,   702,  2402,
    1916,   702,  1440,  1441,   162,     3,  1024,  1946,   279,     4,
     707,     5,  1247,   707,  1249,   708,  1250,  1682,   708,   709,
      57,  1252,   709,   321,  1631,   235,   236,  2014,  2015,   235,
     236,     6,     7,     8,     9,  1301,  2029,  2030,  1324,  1690,
    1308,   446,  1420,  1024,  1421,   450,  1259,  -763,  1297,  1298,
    1311,   520,  2057,  2058,    74,  -763,  -763,  -763,  -763,  2216,
    2282,   474,   446,  2261,   163,   164,   165,  1918,  1919,   163,
     164,   165,  2146,  1261,   523,  1650,    56,   824,   873,   873,
     502,    57,  1807,   827,   239,  1277,  1272,   876,  1024,   891,
     240,  1621,  1668,  1983,  1622,  1669,    59,    60,    61,  1305,
    1170,   906,  1118,  1683,  -860,   446,   541,   802,   239,   515,
     516,   163,   164,   165,   240,  1171,   552,  1460,  1290,  1293,
     239,   653,  1299,   558,   877,    66,   240,   802,  1591,   237,
    1003,  1859,   776,   237,   309,   163,   164,   165,   654,   319,
    1163,   320,   107,  2283,  2284,  1310,  1472,   727,  1025,   241,
      57,   433,  1303,   423,   243,  2055,   438,   285,    90,  1473,
     444,  1474,  -446,  1068,   803,   156,   235,   236,  2055,   892,
    1459,  1330,   730,   241,  1068,  1068,  1155,   482,   243,  1807,
    1807,   469,  -860,  1594,   803,   241,   476,  1285,  1817,  1287,
     243,  1819,  1661,   441,  1662,  1119,   442,  1661,  1291,  1662,
    1300,  1422,   463,   303,  1099,  1304,  1120,   454,   571,   572,
     573,  1663,  1292,   499,    58,   501,  1663,   503,  1451,  1465,
    1116,  1453,   441,   514,   484,   442,   518,  2261,  1664,   537,
    1665,  1691,  1466,  1664,  1286,  1665,   441,   423,  1366,   442,
    1849,  1122,   542,    57,   513,   839,   521,  1698,   488,  1701,
     235,   236,   710,   423,    57,   555,  1620,   832,   446,   805,
     711,  1156,   621, -1073,  1684, -1073,   235,   236,  2357,   806,
     237,  1423,   244,   522,  1551,    58,   239,     3,  1289,   227,
     239,     4,   240,     5,   622,   441,   240,  1503,   442,    57,
     712,  1946,  1506,   498,   227,  1684,   244,   623,  1916,  1500,
    1501,  1502,  1507,     6,     7,     8,     9,  1164,   244,     3,
     280,   281,   282,     4,  1325,     5,   822,     4,  1515,     5,
     907,  2028,  1091,   322,   323,  2296,   227,  2275,  2264,   310,
    2278,   311,  2055,  1309,  1651,     6,     7,     8,     9,  2214,
    2449,   241,   887,   227,    58,   241,   243,  1482,   245,   556,
     243,  1505,  1632,  1633,   237,  1716,  -446,  1359,  1986,   901,
     901,  2334,  1218,   227,  1743,   901,  -758,  2450,  2403,  1033,
     237,   834,   245,  -758,  -758,  -758,  -758,   469,  1521,  2279,
    -730,  1906,   833,  1984,   245,  1004,  1005,  1006,  1595,  2349,
    1744,  1414,  1981,  1415,  1180,  1446,  1419,  1745,  1185,  1124,
     227,  2337,  1078,  1011,  2028,  2088,  1157,  1158,  1713,   227,
    2285,  2384,  2237,  -758,  2154,  1416,  1035,   239,   227,  -763,
     837,  2231, -1073,   240,  2382,  2142,   872,   872,  2383,  1219,
     878,  1851,   574,  1367,  2207,  -253,  1526,    58,  1528,  1174,
     360,  1741,  1742,  1404,  1405,   227,  2089,    57,    58,   360,
    1488,   430,  1172,  1320,   227,   692,  1027,   692,   431,   280,
     281,   282,  1417,   624,   244,   748,  1408,  1173,   244,  2139,
    2140,   689,  2158,   693,  1341,   693,  1661,   694,  1662,   694,
    1599,  1476,   241,    58,   695,  2396,   695,   243,   696,   455,
     696,   749, -1073,  1243,  -276,  1663,   697,   698,   697,   698,
    1387,   239,  1449,  1699,   690,  1146,  1678,   240,  1933,  1242,
     691,  1679,  1664,  1388,  1665,  1465,  2205,   239,   699,  1580,
     699,   576,   486,   240,   700,   701,   700,   701,  1752,   702,
    1389,   702,  1078,  1078,  1078,  1078,  1078,   867,  2004,  1078,
     245,   707,  1494,   707,   245,  1463,   708,  1477,   708,   758,
     709,   682,   709,  1970,   433,     4,  1971,     5,   319,  1973,
     739,   894,   320,   317,   741,  1078,   241,   438,     4,   656,
       5,   243,   444,  1943,  1299,   759,   441,   810,   727,   442,
     657,   658,   241,  1939,  1078,  2268,  1614,   243,  2236,  2329,
    2090,  1755,   825,  1657,  2007,  2008,  2009,  2010,  2011,  2322,
     729,  1357,   727,   730,   888,   244,   683,   684,  1658,  2324,
    1645,  2269,  1464,   345,  2321,   685,  2270,  2323,  2271,  1138,
    1342,   246,     3,   581,   582,   583,     4,   730,     5,   320,
     889,  1478,  1339, -1074,  1659, -1074,  1676,   901,  1445,  1660,
     864,    58,  1351,  1310,  2434,   427,   865,  1151,     6,     7,
       8,     9,  1295,  1873,   578,   579,  -778,   427,  1004,  1005,
    1006,  1609,  1357,  -778,  -778,  -778,  -778,  1657,   866,   901,
    -731,  1807,  1807,   235,   236,  1619,     4,  1612,     5,  2189,
    1817,   245,  1658,  1819,   456,  2091,   457,   345,  1863,   244,
    1646,  -772,  1390,   871,   235,   236,  1033,  2177,  -772,  -772,
    -772,  -772,   227,  -778,  1647,   244,  1461,  1472,  1659,   896,
    1923,  1924,  1034,  1660,  1401,   996,   997,   487,  2088,   488,
    1484,     3,  1474,  1668,  2318,     4,  1669,     5,     3,   733,
     734,  1400,     4,  2005,     5,  2006,  1575,  -758,  -772,  1605,
    1027,  2191,   893,  1035,  2182,  1917,   919,     6,     7,     8,
       9,  1517,  1847,  1576,     6,     7,     8,     9,  1434,  2089,
    1707,  1708,     3,  1982,  -773,   245,     4,  1714,     5,    63,
    1644,  -773,  -773,  -773,  -773,  1025,   869,   237,  2209,  1068,
       3,   245,  1840,  2210,     4,   886,     5,  1577,     6,     7,
       8,     9, -1074,  1597,  1957,   564,   565,  1850,   237,   312,
     313,   314,   315,   316,   817,   818,     6,     7,     8,     9,
       4,  -773,     5,   246,  2187,  1369,   890,   246,  2195,   735,
    2390,  2187,  1820,   897,  1068,  1821,  -582,   901,     3,  2169,
    2170,  2171,     4,  -582,     5,   448,   235,   236,     4,  1027,
       5,    18,    19,   458,   459,   460,   461,   462,  1370,   932,
     933,   915,  1827,  1730,     6,     7,     8,     9,  2150,   911,
     912,  1838, -1074,  1371,  1472,   938,   998,  1739,   496,   999,
    1820,   330,   331,  1821,  1748,  1839,   332,  1753,  2351,  1474,
    1030,  2352,  1587,  1588,  1483,   525,   526,  1031,  1485,  2155,
    1032,    35,   532,   533,   534,   920,   921,   922,   923,   924,
    1306,    36,   546,  2090,  1081,   549,  1082,    37,  1635,  1093,
    1372,  1657,  1094,  1408,  1408,  1408,  1408,  1408,  1095,  1408,
    1408,   927,   928,   727,   239,  1484,  1658,  1474,  1724,  1724,
     240,   345,    38,   932,   933,   162,  1693,   321,   566,   567,
     237,  1836,   568,  1096,   729,   239,  1694,  1822,   730,   162,
    1693,   240,  1659,  1527,   427,  1529,  1097,  1660,  2086,     3,
    1697,  1535,  -860,     4,  -351,     5,  -869,   751,   743,  1078,
    1078,  1078,  1078,  1078,  1078,  1078,  1078,  1078,  1078,  1078,
    2298,  1134,   901,   752,  1154,     6,     7,     8,     9,   241,
    1530,   901,  1028,  1029,   243,  1822,  1018,  1019,  2201,  1373,
    -870,  1820,     3,  2019,  1821,   753,     4,  1531,     5,  1140,
    1343,  1153,   754,  1532,  1533,  1344,  1738,  1740,  1409,  1410,
    1944,  1160,  1320,  1746,  1740,  1749,  1751,  -778,     6,     7,
       8,     9,  1467,  1468,  1945,  1544,  1545,     3,   427,  1583,
    1584,     4,  1162,     5,  1820,   932,   933,  1821,   744,   745,
     746,  1214,   932,   933,   427,  1248,   162,  1693,  1251,  1025,
    1547,  1548,  -772,     6,     7,     8,     9,  1700,   855,   856,
     857,   660,  1274,   733,   734,  1823,   661,  1921,  1275,     3,
    1276,   901,  1282,     4,  1303,     5,  1357,   239,   901,  1284,
       3,   897,  1655,   240,     4,  1302,     5,  1900,  1585,  1586,
    1942,   755,   756,  1316,  2388,     6,     7,     8,     9,   662,
     663,  1321,   244,  1438,  1322,  1991,     6,     7,     8,     9,
    1534,  1820,   901,  1823,  1821,  1323,  1822,  1843,   235,   236,
    1844,   162,  1693,   244,   720,  -773,  1864,  1865,  1895,   138,
     139,  1326,   140,   141,   142,  2036,  2032,  2033,  1979,  1327,
    1980,  1328,   241,  1987,  1988,  2368,  1824,   243,  1025,  1078,
    1657,  1717,  1329,   735,  1657,  2127,  2369,   721,   722,  1822,
    2037,  2038,   761,  2116,  2117,  1658,  1362,  1078,  1331,  1658,
     345,  2134,  2096,  1361,   345,  1363,   762,  1348,   245,  2097,
    2120,  2121,  2148,  2149,   763,  2151,     3,   280,   281,   282,
       4,  1659,     5,   723,  2073,  1659,  1660,  2257,  1364,   245,
    1660,  1868,   764,  1376,  1107,  1108,  1109,  2392,  2393,   765,
    2411,  2412,     6,     7,     8,     9,  1377,  2098,  2433,  1366,
     665,   811,   237,  1341,  1725,  1726,   724,  1378,  1078,  1382,
     812,   813,   235,   236,   814,   815,  1822,  1386,  1126,  1396,
    1403,  2131,  1925,  1926,  1823,  1928,  1078,  1078,  1411,  1418,
    2156,  2068,  2099,   666,   667,  2188,   668,  1412,  1413,  2132,
   -1118, -1119,     3,  1941,  1424,   244,     4,  1425,     5,  1429,
     423,  1426,  1139,   902,  1427,  1756,  1758,  1430,  1775,  1784,
    1801,  1815,  2088,   235,   236,  1437,  2039,  1823,     6,     7,
       8,     9,  1439,  1740,  1740,  1470,  1469,   613,  1471,   614,
    1759,  2114,  1776,  1785,  1802,  1816,  2040,  2041,  1848,  2232,
    2233,   148,  2370,   149,  2299,  1017,  1018,  1019,   150,   615,
     151,  1475,  1481,  2089,   152,  2079,  1465,   616,   617,   618,
     619,     3,   163,   164,   165,     4,   237,     5,  1856,  1857,
    1486,   245,  2167,  2166,  2204,   490,   491,   492,   493,   494,
    1090,  2165,  1310,  2257,  1495,  1854,  1855,     6,     7,     8,
       9,  2100,  1496,  1497,  1823,   153,  1552,   154,  2087,   239,
    1553,  1554,  1555,   559,   143,   240,   560,   561,   562,  1930,
    1498,  1339,  1434,  1004,  1005,  1006,  2371,   237,   144,   145,
    2045,  2046,  2047,  1874,  2048,  1889,  1499,  1896,  1299,  1901,
    1504,   875,  1907,  2372,  1508,   146,   585,  1351,   586,   587,
     588,   589,  2428,  1303,  1509,  1510,   423,  1875,  1511,  1890,
    2185,  1897,  1954,  1902,  1516,  2164,  1908,  1512,  1513,   235,
     236,     3,  1514,  1519,   241,     4,  1538,     5,  1523,   243,
     901,  1540,  1542,   235,   236,  2381,  1569,  1550,  2194,  1955,
    1570,   246,  1571,  1573,  1572,   423,  2101,     6,     7,     8,
       9,  1574,  1581,  1582,  1953,  1589,  1590,  2090,   235,   236,
    1591,  1593,   246,     3,  1603,  2456,  1611,     4,  1598,     5,
     231,  1600,  1601,   239,  1602,  1616,   647,  1033,  1634,   240,
    1642,  1648,  1652,  1635,  1958,  1959,  1960,  1961,  1962,     6,
       7,     8,     9,   648,   231,   847,  1677,   848,  1685,   849,
     850,  2018,   649,  1687,  1689,  1703,   231,   851,   852,   650,
     651,  1706,  1712,  2096,  1562,  1563,  1564,  1565,  1566,  2138,
    2097,  1720,   829,   237,   239,  1721,  2344,  1722,  -730,  -731,
     240,   840,   841,   842,   843,   844,   901,   237,   241,  1737,
    1754,  1833,   853,   243,  1320,  1834,  1845,   244,  1835,  1852,
    2397,  1853,  2420,  1913,  1861,  1935,  2012,  2230,  2098,  2363,
    1872,  1366,   237,  1920,   882,  2021,  2022,  2023,  2024,  2025,
     504,   889,   538,  1956,   235,   236,  -601,  1947,  1288,  2106,
    1964,  1965,  2051,  2052,  2053,  2054,    18,    19,  1968,   241,
    1972,   235,   236,  2099,   243,  1975,  2062,  2063,  2064,   423,
    2432,  1977,  2336,  1472,   246,  2342,  2438,  1775,  1784,  1801,
    1815,   423,   235,   236,  1989,  1990,  1889,  2020,  2035,  2343,
    1784,  2001,  2013,   245,   235,   236,  2065,  2358,  2364,  2067,
    -445,  1776,  1785,  1802,  1816,  2080,    35,  2031,  2059,  2066,
    1890,  2082,  1078,  2084,  1785,  2113,    36,  2115,  2092,  2102,
    2109,  2118,    37,  1175,  1176,  1177,  1178,   873,   873,  2119,
    2122,   244,   231,  2123,  2137,  2145,   231,  2147,  2153,  2162,
     239,  2128,  2093,  2103,  2110,  2157,   240,    38,   237,  2160,
    2168,  2445,  2448,  2213,   239,  2181,  2341,  2182,  2187,  2197,
     240,  2206,  2215,  2219,  2208,   237,  2217,  2359,  2235,  2220,
    2222,  2227,  2100,   235,   236,  1557,  1558,  1559,  1560,   239,
     504,  2228,   244,  2229,  2241,   240,   237,  2238,  1320,     3,
    2242,  2295,  2307,     4,  2243,     5,    18,    19,   237,  2244,
    2245,  2260,  2250,  2143,  2179,   241,  2316,   245,  2251,  2252,
     243,  2253,  2254,  2335,  2107,     6,     7,     8,     9,  1343,
    2297,  2325,   441,  2440,  1344,   442,  2317,  2326,  1090,  2327,
    2346,  2446,  -295,  2350,   235,   236,  2354,  2399,   636,   637,
    -445,   638,  2400,  2401,   241,  2447,    35,  2405,  2430,   243,
     441,   901,  2407,   442,  2455,  2439,    36,  2179,   245,  2409,
    2410,   639,    37,  2292,  2293,  2294,  2413,  2202,  2451,   640,
     641,  1454,   642,   231,  1456,  1393,  1280,   237,  2183,  2184,
    1279,  1278,   643,   644,   645,  1455,  1169,    38,  1281,  1846,
    1165,  1525,  1458,   901,  -445,   239,    73,  1101,  2190,   830,
     539,   240,  1994,  1996,  1996,  1999,  2192,  2193,   831,  2108,
    2196,  2444,   239,  2246,  1828,  2061,  2200,  2212,   240,  2263,
    2280,  2391,  2262,  2092,  2102,  2109,  2328,  2218,   244,  1062,
    2331,  2330,  2332,   239,  2221,  2320,  2431,   838,   237,   240,
    1062,  1062,   244,  1998,  1098,   239,  2159,  2093,  2103,  2110,
    1365,   240,  2172,  2173,  2174,  2175,  2176,   231,     3,  1381,
     241,  2144,     4,  2133,     5,   243,   246,   244,  2258,  1607,
    2129,   934,  1966,   231,  2130,  1967,   423,   241,   626,   627,
    1893,  1894,   243,  1428,     6,     7,     8,     9,  2289,   235,
     236,  1100,  2319,   628,  2376,   629,   630,   631,   241,  1709,
    1711,  1710,  2395,   243,   245,  1728,  1932,  1929,   235,   236,
     241,  2240,  1436,  1345,  1077,   243,  1731,   870,   245,   632,
     633,   634,   234,  1520,   239,  1077,  1077,  2259,  1750,   809,
     240,  1674,  1922,  2136,  -445,   235,   236,  1089,  2311,  2312,
    1950,  1951,  2436,   245,   235,   236,  1111,  2290,  2291,   880,
    2096,  1114,  1113,  1115,   884,  2309,  2310,  2097,  1245,  1112,
    2300,  2301,   885,  2302,  2303,  2304,  2305,  2306,  2353,  1217,
    2308,  1568,     0,  1211,     0,   872,   872,     0,     0,     0,
     246,     0,     0,   244,   232,   239,     0,     0,     0,   241,
       0,   240,     0,   237,   243,  2098,     0,     0,     0,     0,
     244,     0,     0,  2452,  2453,  2454,     0,     0,   232,     0,
       0,     0,   237,     0,  2247,  2248,  2249,     0,   235,   236,
     232,   244,     0,  2345,  2258,     0,  2347,     0,     0,  2348,
    2099,   427,     0,   244,   591,   592,     0,     0,     0,   237,
       0,     0,     0,   593,   594,   595,   596,   597,   237,   901,
     241,     0,     0,     0,     0,   243,     0,     0,     0,   245,
       0,   423,  2360,  2365,  2373,  1757,     0,     0,     0,  1758,
    1784,  1801,  1815,     0,   235,   236,   245,  1874,  1889,  1896,
    1901,  1907,     0,  2429,     0,     0,  2361,  2366,  2374,     0,
       0,     0,     0,  1759,  1785,  1802,  1816,   245,  2398,   235,
     236,  1875,  1890,  1897,  1902,  1908,  2404,     0,     0,   245,
       0,  2406,   244,     0,     0,     0,  2408,     0,     0,     0,
       0,  2179,   237,     0,  2414,     0,  2415,  2416,  2417,  2418,
    2419,  2092,  2102,  2109,  2423,  2424,  2425,  2426,  2427,  2100,
     239,     0,     0,     0,     0,     0,   240,     0,     0,     0,
       0,     0,  2437,     0,     0,  2093,  2103,  2110,     0,   239,
       0,     0,     0,     0,     0,   240,     0,     0,     0,  2441,
    2442,  2443,     0,   244,     0,   726,   232,   427,   237,     0,
     232,     0,     0,     0,     0,     0,   239,   727,   245,   728,
       0,   246,   240,     3,     0,   239,     0,     4,     0,     5,
       0,   240,     0,   237,     0,   241,     0,     0,   729,     0,
     243,     0,   730,     0,     0,   731,   427,     0,   901,     6,
       7,     8,     9,     0,   241,  1670,     0,     0,     0,   243,
       0,     0,   512,     0,  2421,     3,     0,     0,   519,     4,
       0,     5,   527,   528,   529,   530,   531,     0,     0,   245,
     535,   241,     0,     0,     0,     0,   243,     0,   547,   548,
     241,     6,     7,     8,     9,   243,     0,     0,     0,   239,
       3,     0,     0,     0,     4,   240,     5,     0,     0,     0,
     231,   334,   335,   336,  1936,  1937,   339,     3,     0,     0,
       0,     4,     0,     5,     0,     0,     6,     7,     8,     9,
       0,   231,     0,  1315,   732,  1317,     0,     0,  1062,  2106,
       0,     0,     0,     6,     7,     8,     9,   232,     0,     0,
       0,     0,  1332,  1333,  1334,   239,  1335,     0,   244,  1346,
    1347,   240,   246,     0,   241,     0,    92,   733,   734,   243,
       0,   163,   164,   165,    93,     0,    94,   244,    95,   246,
     239,    96,    97,    98,     0,    99,   240,     0,  1760,     0,
    1778,  1789,  1805,  1818,     0,    68,    69,    70,     0,     0,
     427,     0,  1407,  1064,   244,     0,     0,  1391,     0,     0,
    1395,     0,   427,   244,  1064,  1064,  2106,     0,     0,     0,
     241,     0,     0,  1077,     0,   243,     0,     0,  1262,  1263,
       0,   232,     0,     0,   245,     0,     0,     0,     0,     0,
       0,     0,  1264,     0,     0,   241,     0,   232,     0,     0,
     243,  1265,     0,   245,     0,     0,     0,   735,     0,     0,
     451,   452,   453,     0,     0,     0,     0,  1266,  1062,  1062,
    1062,  1062,  1062,   231,     0,  1062,     0,  1432,   481,     0,
     245,     0,     0,     0,     0,     0,     0,   244,   495,   245,
       0,   246,     0,     0,     0,     0,     0,   510,   511,     0,
       0,  1062,     0,   517,  2107,  1876,     0,  1891,     0,  1898,
       0,  1903,     3,     0,  1909,     0,     4,     0,     5,     0,
    1062,   543,   544,   545,     0,     0,     0,     0,     0,     0,
       0,   553,   554,     0,     0,   557,     0,     0,     6,     7,
       8,     9,     0,   244,     0,     0,     0,  1479,  1480,     0,
       0,  1090,   246,  1077,  1077,  1077,  1077,  1077,     0,     0,
    1077,     0,     0,   245,     0,     3,     0,     0,   244,     4,
       0,     5,  1670,     0,     3,     0,     0,     0,     4,     0,
       5,  2107,     0,   901,     0,     0,  1077,     0,     0,     0,
       0,     6,     7,     8,     9,  1267,  1268,  1269,   785,  2203,
       6,     7,     8,     9,     0,  1077,     0,     0,   100,     0,
       0,   795,   796,   797,  1299,     0,   599,   600,     0,   245,
     601,     0,     0,   101,     0,   901,     0,     0,     0,     0,
     102,   602,   603,   604,   605,   606,   607,   608,     0,   609,
     610,  2389,     0,  1193,   245,     0,     0,     0,     0,   103,
       0,     0,     0,     0,   104,     0,   105,     0,     3,  1194,
     901,     0,     4,     0,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   835,   836,  2422,   901,     0,     0,
       0,     0,   611,     0,     6,     7,     8,     9,     0,     0,
       0,     0,  1878,  2385,     0,     0,     0,     0,    76,  1791,
       0,     0,  1879,     0,     0,     0,    77,   427,    78,  1880,
    1881,     0,     0,    79,    80,    81,     0,    82,     0,     0,
       0,     0,     0,     0,     0,  1195,   246,  1882,     0,  1778,
    1789,  1805,  1818,     0,     0,     0,     0,     0,  1891,     0,
       0,     0,  1789,     0,     0,  1883,  1884,     0,     0,  1792,
    1793,  1794,  1795,   246,  1796,     0,     0,  1797,     0,     0,
       0,     0,   246,     0,     0,   231,     0,     0,     0,     0,
    2094,  2104,  2111,     0,  1798,  1885,     0,     0,  1615,  1886,
       0,     0,  1618,     0,     0,     0,     0,     0,     0,     0,
     670,     0,  1638,  1639,  1640,  1641,     0,  1643,  1196,   671,
       0,  1197,  1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,
    1206,  1207,  1208,  1209,  1653,   672,     0,     0,     0,   673,
       0,     0,     0,   674,   675,  1799,     0,     0,   676,   677,
     678,   679,   680,     0,     0,     0,   246,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1407,
    1407,  1407,  1407,  1407,   232,  1407,  1407,     0,     0,     0,
    1702,     0,  1704,     0,  1723,  1723,     0,     0,     0,  1705,
       0,     0,   901,     0,     0,  1340,     0,  1887,     0,   231,
       0,     0,  1064,     0,  1718,     0,  1719,  1303,  2377,     0,
       0,     0,   427,     0,     0,  1727,  1137,  1729,     0,     0,
       0,     0,     0,     0,     0,  1062,  1062,  1062,  1062,  1062,
    1062,  1062,  1062,  1062,  1062,  1062,     0,   246,     0,     0,
       0,     0,     0,     0,     0,   901,     0,     0,     0,     0,
     231,     0,     0,     0,   901,     0,     0,  1150,  1657,     3,
      83,  2387,     0,     4,     0,     5,     0,     0,     0,     0,
    1888,     0,     0,  1658,     0,    84,     0,     0,   345,     0,
       0,     0,    85,     0,     0,     6,     7,     8,     9,     0,
       0,     0,     0,     0,     0,  2094,  2104,  2111,     0,  1659,
    1791,    86,     0,     0,  1660,     0,    87,     0,    88,     0,
    1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,
    1077,  1435,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1064,  1064,  1064,  1064,  1064,   232,   901,  1064,
       0,     0,     3,     0,     0,     0,     4,     0,     5,     0,
    1792,  1793,  1794,  1795,  1800,  1796,     0,     0,  1797,     0,
       3,     0,     0,     0,     4,  1064,     5,     0,     6,     7,
       8,     9,     0,     0,     0,  1798,     0,     0,     0,     3,
    1762,     0,     0,     4,  1064,     5,     6,     7,     8,     9,
    1763,     0,     0,     0,     0,  1764,   231,  1765,     0,  1766,
       0,  1791,  1911,  1912,     0,     6,     7,     8,     9,     0,
     231,     0,     0,     0,     0,  1062,     0,     0,     0,  1626,
    1808,     0,     0,     0,     0,     0,  1799,     0,  1927,     0,
       0,     0,     0,  1062,     0,   231,     0,     0,  1934,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1792,  1793,  1794,  1795,     0,  1796,     0,     0,  1797,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1809,  1810,  1811,     0,  1796,  1798,     0,  1797,     0,
       0,     0,     0,     0,     0,  1195,     0,     0,     0,  1974,
       0,     0,     0,     0,  1062,  1812,     0,     0,     0,     0,
    1077,     0,     0,     0,     0,     0,  1976,     0,  1978,  1767,
       0,     0,  1062,  1062,     0,     0,     0,     0,  1077,     0,
       0,     0,     0,     0,     0,     0,     0,  1799,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1657,
       0,  1760,  1789,  1805,  1818,     0,  1813,     0,     0,  1876,
    1891,  1898,  1903,  1909,  1658,     0,     0,     0,  1627,   345,
       0,  1197,  1198,  1628,  1200,  1201,  1202,  1203,  1204,  1205,
    1206,  1207,  1208,  1629,     0,     0,     0,     0,     0,  1077,
    1659,     0,     0,     0,     0,  1660,     0,     0,     0,   231,
       0,     0,     0,     0,     0,     0,     0,  1077,  1077,     0,
       0,   231,     0,  2094,  2104,  2111,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   901,
       0,     0,  1768,     0,     0,     0,     0,     0,     0,   232,
       0,     0,     0,    10,     0,  2071,     0,  1769,     0,     0,
    1657,    11,     0,    12,  1770,    13,     0,     0,    14,    15,
      16,     0,    17,     0,     0,  1658,     0,    18,    19,  1657,
     345,     0,     0,  1771,     0,     0,     0,     0,  1772,     0,
    1773,     0,     0,     0,  1658,     0,     0,     0,     0,   345,
     231,  1659,     0,     0,     0,     0,  1660,     0,     0,     0,
       0,  1696,  1696,     0,  1696,     0,     0,     0,     0,  2135,
    1659,     0,   901,     0,     0,  1660,     0,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,  1774,    52,
     901,  2141,     0,    37,     0,     0,     0,     0,     0,    64,
      52,    52,    67,    67,    67,    71,  2379,     0,    52,   901,
       0,   231,     0,   232,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,  1814,     0,     0,     0,     0,
       0,  2152,     0,     0,     0,     0,     0,     0,     0,  1064,
    1064,  1064,  1064,  1064,  1064,  1064,  1064,  1064,  1064,  1064,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,   232,   449,    52,    67,    67,    67,
      71,     0,     0,     0,     0,     0,     0,     0,    52,     0,
     465,   470,   473,    52,   478,    67,   478,   483,     0,   465,
     465,   465,   465,   465,     0,    67,     0,     0,     0,   500,
       0,    52,     0,   508,    67,    67,   508,    71,     0,     0,
      67,     0,   508,     0,     0,     0,   508,   508,   508,   508,
     508,     0,     0,     0,   508,     0,    52,   540,    67,    67,
      67,     0,   508,   508,     0,     0,   551,    52,    67,    67,
       0,     0,    67,     0,    52,    39,  2223,  2224,     0,     0,
    2225,  2226,     0,     0,     0,     0,     0,     0,     0,     0,
      40,     0,   288,     0,     0,     0,   231,    41,     0,     0,
     289,     0,   290,     0,   291,     0,     0,   292,   293,   294,
       0,   295,     0,     0,     0,   231,    42,     0,  1062,     0,
       0,    43,     0,    44,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  2234,
     232,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1931,     0,  1340,  1435,     0,     0,     0,  1064,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,     0,     4,     0,     5,     0,     0,  1064,     0,   232,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     6,     7,     8,     9,  1696,     0,
       0,  1696,     0,  1077,  1696,     0,   465,   470,   473,    52,
     478,    67,    67,   483,     0,   465,   465,   465,   465,   465,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1064,     0,
       0,     0,     0,     0,     0,     0,     3,     0,     0,     0,
       4,     0,     5,     0,     0,     0,  1064,  1064,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   158,
       0,     0,     6,     7,     8,     9,   159,     0,   160,     0,
       0,   231,   327,   161,   328,     0,  1878,     0,     0,     0,
       0,  1779,     0,     0,     0,     0,  1879,     0,     0,   329,
       0,     0,     0,  1880,  1881,     0,     0,     0,     0,     0,
     330,   331,     0,     0,     0,   332,     0,     0,   333,     0,
       0,  1882,     0,     0,     0,     0,     0,   334,   335,   336,
     337,   338,   339,   232,   296,     0,     0,     0,     0,  1883,
    1884,     0,  1780,     0,     0,   232,   340,     0,   341,   297,
       0,   162,   163,   164,   165,     0,   298,   166,     0,   167,
       0,     0,  1781,   168,     0,     0,     0,     0,   169,  1885,
       0,     0,     0,  1886,   170,   299,     0,     0,     0,     0,
     300,   166,   301,   167,     0,     0,     0,   168,     0,     0,
       0,   171,   169,     0,     0,     0,   172,     0,   170,   173,
       0,     0,     0,     0,   174,     0,     0,     0,   175,     0,
       0,   176,   177,  1782,     0,   171,   178,     0,     0,   179,
     172,   180,  1136,   173,   232,     0,     0,     0,   174,     0,
       0,     0,   175,    67,     0,   176,   177,   687,     0,     0,
     178,     0,     0,   179,     0,   180,   181,   182,     0,   183,
     184,   185,   186,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     181,  1887,   688,     0,    67,     0,   186,     0,     0,     0,
       0,     0,     0,     0,     0,   232,     0,   187,   188,   189,
     190,     0,     0,     0,     0,     0,   191,   192,     0,     0,
     193,   194,   342,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,     0,     0,   343,     0,   901,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,     0,     0,  2077,     0,   345,     0,   346,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   347,     0,     0,     0,     0,     0,   348,     0,     0,
       0,     0,   349,     0,     0,     0,     0,     0,   219,   220,
       0,   350,     0,     0,   351,   352,   353,   354,     0,     0,
       0,   355,     0,     0,  2338,   356,   357,   221,     0,     0,
       0,   222,   223,     0,     0,     3,   901,     0,     0,     4,
     358,     5,     0,   224,   225,     0,   777,   778,     0,   359,
     779,   360,  1783,   226,     0,     0,   227,     0,   158,     0,
       0,     6,     7,     8,     9,   159,     0,   160,     0,   780,
     232,   327,   161,   328,     0,     0,     0,     0,     0,     0,
    1779,     0,     0,     0,     0,     0,     0,     0,   329,   232,
       0,     0,  1064,     0,     0,     0,     0,     0,     0,   330,
     331,     0,     0,     0,   332,     0,     0,   333,     0,     0,
       0,     0,     0,     0,     0,     0,   334,   335,   336,   337,
     338,   339,     0,     0,     0,     0,   942,     0,     0,     0,
       0,  1780,   943,     0,     0,   340,     0,   341,     0,     0,
     162,   163,   164,   165,     0,     0,   166,     0,   167,     0,
       0,  1781,   168,     0,     0,     0,     0,   169,     0,     0,
     163,   164,   165,   170,     0,     0,     0,  1043,     0,     0,
       0,  1044,     0,     0,     0,     0,  1045,     0,     0,     0,
     171,     0,  1046,     0,     0,   172,   950,     0,   173,   781,
       0,     0,     0,   174,     0,     0,     0,   175,     0,     0,
     176,   177,  1782,     0,     0,   178,     0,  1049,   179,     0,
     180,     0,     0,   782,   783,   784,   785,   786,   787,  2339,
     788,   789,   790,   791,   792,   793,   794,     0,     0,   795,
     796,   797,     0,     0,     0,   181,   182,     0,   183,   184,
     185,   186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   232,     0,     0,     0,     0,
     798,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   187,   188,   189,   190,
       0,     0,     0,     0,     0,   191,   192,     0,     0,   193,
     194,   342,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,     0,     0,   343,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     344,     0,     0,     0,     0,   345,     0,   346,     0,     0,
       0,     0,     0,     0,   953,     0,     0,   954,   955,     0,
     347,     0,     0,     0,   956,     0,   348,     0,     0,     0,
       0,   349,     0,     0,     0,     0,     0,   219,   220,     0,
     350,     0,     0,   351,   352,   353,   354,     0,     0,     0,
     355,     0,     0,     0,   356,   357,   221,     0,     0,     0,
     222,   223,   958,   959,     3,   901,     0,     0,     4,   358,
       5,     0,   224,   225,     0,     0,     0,     0,   359,     0,
     360,  2070,   226,     0,     0,   227,     0,   158,     0,     0,
       6,     7,     8,     9,   159,     0,   160,     0,     0,     0,
     327,   161,   328,     0,   227,     0,     0,     0,     0,  1779,
       0,     0,     0,     0,     0,     0,     0,   329,     0,     0,
       0,     0,     0,     0,     0,   767,     0,     0,   330,   331,
       0,     0,     0,   332,     0,     0,   333,     0,     0,     0,
       0,     0,     0,     0,     0,   334,   335,   336,   337,   338,
     339,     0,   768,     0,     0,     0,   769,   166,  1518,   167,
    1780,     0,   716,   168,   340,     0,   341,     0,   169,   162,
     163,   164,   165,   770,   170,   166,     0,   167,     0,     0,
    1781,   168,     0,     0,     0,     0,   169,     0,     0,     0,
       0,   171,   170,     0,     0,     0,   172,     0,     0,   173,
       0,     0,     0,     0,   174,     0,     0,     0,   175,   171,
       0,   176,   177,     0,   172,     0,   178,   173,     0,   179,
       0,   180,   174,     0,     0,     0,   175,     0,     0,   176,
     177,  1782,     0,     0,   178,     0,     0,   179,     0,   180,
       0,     0,     0,     0,     0,     0,   181,     0,   717,     0,
       0,     0,   186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,   182,     0,   183,   184,   185,
     186,     0,     0,     0,     0,     0,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,     0,     0,     0,
       0,     0,     0,     0,     0,   187,   188,   189,   190,     0,
       0,     0,     0,     0,   191,   192,     0,     0,   193,   194,
     342,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,     0,     0,   343,     0,     0,   771,     0,     0,
       0,   219,   220,     0,     0,     0,     0,     0,     0,   344,
       0,   772,     0,     0,   345,     0,   346,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   347,
       0,     0,     0,     0,     0,   348,     0,   773,     0,     0,
     349,     0,     0,     0,     0,     0,   219,   220,     0,   350,
       0,     0,   351,   352,   353,   354,     0,     0,     0,   355,
       0,     0,     0,   356,   357,   221,     0,     0,     0,   222,
     223,     0,     0,     3,   901,     0,     0,     4,   358,     5,
       0,   224,   225,     0,     0,     0,     0,   359,     0,   360,
    2078,   226,     0,     0,   227,     0,   158,     0,     0,     6,
       7,     8,     9,   159,     0,   160,     0,     0,     0,   327,
     161,   328,     0,     0,     0,     0,     0,     0,  1779,     0,
       0,     0,     0,     0,     0,     0,   329,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   330,   331,     0,
       0,     0,   332,     0,     0,   333,     0,     0,     0,     0,
       0,     0,     0,     0,   334,   335,   336,   337,   338,   339,
     166,     0,   167,     0,     0,     0,   168,     0,     0,  1780,
       0,   169,     0,   340,     0,   341,     0,   170,   162,   163,
     164,   165,     0,     0,   166,     0,   167,     0,     0,  1781,
     168,     0,     0,     0,   171,   169,     0,     0,     0,   172,
       0,   170,   173,     0,     0,     0,     0,   174,     0,     0,
       0,   175,     0,     0,   176,   177,     0,     0,   171,   178,
       0,     0,   179,   172,   180,     0,   173,     0,     0,     0,
       0,   174,     0,     0,     0,   175,     0,     0,   176,   177,
    1782,     0,     0,   178,     0,     0,   179,     0,   180,   181,
       0,     0,     0,     0,     3,   186,     0,     0,     4,     0,
       5,     0,     0,     3,     0,     0,     0,     4,     0,     5,
       0,     0,     0,   181,   182,     0,   183,   184,   185,   186,
       6,     7,     8,     9,     0,     0,     0,     0,     0,     6,
       7,     8,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1808,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   187,   188,   189,   190,     0,     0,
       0,     0,     0,   191,   192,     0,     0,   193,   194,   342,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,     0,     0,   343,     0,  1809,  1810,  1811,     0,  1796,
       0,     0,  1797,     0,     0,     0,     0,     0,   344,     0,
       0,     0,  1878,   345,     0,   346,     0,     0,     0,  1812,
       0,     0,  1879,     0,     0,     0,     0,     0,   347,  1880,
    1881,     0,     0,     0,   348,     0,     0,     0,     0,   349,
       0,     0,     0,     0,     0,   219,   220,  1882,   350,     0,
       0,   351,   352,   353,   354,     0,     0,     0,   355,     0,
       0,     0,   356,   357,   221,  1883,  1884,     0,   222,   223,
    1813,     0,     4,   901,     5,     0,     0,   358,     0,     0,
     224,   225,     0,     0,     0,     0,   359,     0,   360,  2378,
     226,   158,     0,   227,     0,  1885,     0,     0,   159,  1886,
     160,     0,     0,     0,   327,   161,   328,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     0,     0,     0,
       4,   329,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   330,   331,     0,     0,     0,   332,     0,     0,
     333,     0,     6,     7,     8,     9,     0,     0,     0,   334,
     335,   336,   337,   338,   339,     0,     0,  1808,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,     0,
     341,     0,     0,   162,   163,   164,   165,     0,     0,   166,
       0,   167,     0,  1657,     0,   168,     0,     0,     0,     0,
     169,     0,     0,     0,     0,     0,   170,  1887,  1658,     0,
       0,     0,     0,   345,     0,     0,     0,     0,  1809,  1810,
    1811,     0,  1796,   171,     0,  1797,     0,     0,   172,     0,
       0,   173,     0,     0,  1659,     0,   174,     0,     0,  1660,
     175,     0,  1812,   176,   177,     0,     0,     0,   178,     0,
       0,   179,     0,   180,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   901,     0,     0,     0,     0,     0,
       3,     0,     0,   901,     4,     0,     5,     0,   181,   182,
    2386,   183,   184,   185,   186,     0,     0,     0,     0,  2072,
       0,     0,     0,  1813,     0,     0,     6,     7,     8,     9,
       0,     0,     0,     0,     0,     0,     0,     0,  1762,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1763,   187,
     188,   189,   190,  1764,     0,  1765,     0,  1766,   191,   192,
       0,     0,   193,   194,   342,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,     0,     0,   343,   879,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   344,     0,     0,     0,     0,   345,     0,
     346,     0,     0,     0,     0,     0,   768,     0,     0,     0,
     769,     0,     0,   347,     0,     0,     0,     0,     0,   348,
       0,     0,     0,     0,   349,     0,  1657,   770,     0,     0,
     219,   220,     0,   350,     0,     0,   351,   352,   353,   354,
       0,  1658,     0,   355,     0,     0,   345,   356,   357,   221,
       0,     0,     0,   222,   223,     0,     0,  1767,     0,     0,
       0,     0,   358,     0,     0,   224,   225,  1659,     0,     0,
       0,   359,  1660,   360,  1952,   226,     0,     0,   227,     0,
       0,     0,     0,     0,     0,     0,     0,   941,     0,     0,
       0,     0,     0,   777,   778,     0,     0,   779,     0,     0,
       0,     0,     0,     0,     0,   158,   901,     0,     0,     0,
       0,     0,   159,     0,   160,     0,   780,     0,     0,   161,
       0,     0,  2380,     0,     0,     0,     0,     0,     0,     0,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,     0,     0,     0,   942,     0,     0,     0,     0,     0,
     943,     0,     0,     0,     0,     0,     0,     0,     0,   944,
    1768,   945,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1769,     0,   162,   163,   164,
     165,     0,  1770,   166,     0,   946,     0,     0,     0,   947,
       0,   771,     0,     0,   948,   219,   220,     0,     0,     0,
     949,  1771,     0,     0,   950,   772,  1772,     0,  1773,     0,
       0,     0,     0,     0,     0,     0,     0,   171,     0,     0,
       0,     0,   172,     0,     0,   951,   781,     0,     0,     0,
     174,   773,     0,     0,   175,     0,     0,   176,   177,     0,
     901,     0,   178,     0,     0,   179,     0,   180,     0,     0,
     782,   783,   784,   785,   786,   787,  2069,   788,   789,   790,
     791,   792,   793,   794,     0,     0,   795,   796,   797,     0,
       0,     0,   181,   182,     0,   183,   184,   185,   186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   109,     0,     0,     0,   798,   110,     0,
       0,   111,   112,   113,   114,     0,     0,   115,   116,     0,
     117,   118,   119,   952,   120,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
       0,     0,   217,  1312,     0,   121,     0,   122,   123,   124,
       0,     0,   953,     0,     0,   954,   955,     0,     0,     0,
       0,   158,   956,     0,   218,     0,     0,     0,   159,     0,
     160,     0,     0,     0,     0,   161,     0,     0,   957,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,   220,     0,     0,     0,     0,
     958,   959,     0,     0,     0,     0,     0,     0,     0,     0,
     942,     0,     0,   221,     0,     0,   943,   222,   223,     0,
     166,     0,     0,     0,     0,     0,     0,   960,   961,     0,
     773,     0,     0,     0,     0,     0,     0,   170,     0,   962,
       0,     0,   227,   162,   163,   164,   165,     0,     0,   166,
       0,   946,     0,     0,   171,   947,     0,     0,     0,   172,
     948,     0,   173,     0,     0,     0,  1313,   174,     0,     0,
     950,   175,     0,     0,   176,   177,     0,     0,     0,   178,
       0,     0,   179,   171,   180,     0,     0,     0,   172,     0,
       0,  1314,     0,     0,     0,     0,   174,     0,     0,     0,
     175,     0,     0,   176,   177,     0,     0,     0,   178,   181,
       0,   179,     0,   180,     0,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   181,   182,
     126,   183,   184,   185,   186,     0,     0,   127,     0,     0,
       0,     0,     0,     0,   128,   129,   130,   131,   132,     0,
       0,     0,     0,     0,     0,     0,   133,     0,     0,     0,
       0,   134,     0,   135,   136,     0,     0,     0,     0,   952,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,     0,     0,   217,  1394,
       0,     0,     0,     0,     0,     0,     0,     0,   953,     0,
       0,   954,   955,   881,     0,     0,     0,   158,   956,     0,
     218,     0,     0,     0,   159,     0,   160,     0,     0,     0,
       0,   161,     0,     0,     0,   777,   778,     0,     0,   779,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     219,   220,     0,     0,     0,     0,   958,   959,   780,     0,
       0,     0,     0,     0,     0,     0,   942,     0,     0,   221,
       0,     0,   943,   222,   223,     0,     0,     0,     0,     0,
       0,     0,     0,   960,   961,     0,   773,     0,     0,     0,
       0,     0,     0,     0,     0,   962,     0,     0,   227,   162,
     163,   164,   165,     0,     0,   166,     0,   946,     0,     0,
       0,   947,     0,     0,     0,   882,   948,     0,     0,     0,
       0,     0,  1313,     0,     0,     0,   950,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,   172,     0,     0,  1314,     0,     0,
       0,     0,   174,     0,     0,     0,   175,     0,     0,   176,
     177,     0,     0,     0,   178,     0,     0,   179,     0,   180,
       0,     0,     0,     0,     0,     0,     0,     0,   781,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,   182,     0,   183,   184,   185,
     186,     0,   782,   783,   784,   785,   786,   787,   883,   788,
     789,   790,   791,   792,   793,   794,     0,     0,   795,   796,
     797,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   952,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   798,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,     0,     0,   217,  1431,     0,     0,     0,     0,
       0,     0,     0,     0,   953,  1220,     0,   954,   955,     0,
       0,     0,     0,   158,   956,     0,   218,     0,     0,     0,
     159,     0,   160,     0,     0,     0,     0,   161,  1221,  1222,
    1223,  1224,  1225,  1226,  1227,  1228,  1229,  1230,  1231,  1232,
    1233,  1234,  1235,  1236,  1237,     0,   219,   220,     0,     0,
       0,     0,   958,   959,     0,     0,     0,     0,     0,     0,
       0,     0,   942,     0,     0,   221,     0,     0,   943,   222,
     223,     0,     0,     0,     0,     0,     0,     0,     0,   960,
     961,     0,   773,     0,     0,     0,     0,     0,     0,     0,
       0,   962,     0,     0,   227,   162,   163,   164,   165,     0,
       0,   166,     0,   946,     0,     0,     0,   947,     0,     0,
       0,     0,   948,     0,     0,     0,     0,     0,  1313,     0,
       0,     0,   950,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
     172,     0,     0,  1314,     0,     0,     0,     0,   174,     0,
       0,     0,   175,     0,     0,   176,   177,     0,     0,     0,
     178,     0,     0,   179,     0,   180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     181,   182,     0,   183,   184,   185,   186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   952,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,     0,     0,
     217,  1617,     0,     0,     0,     0,     0,     0,     0,     0,
     953,     0,     0,   954,   955,     0,     0,     0,     0,   158,
     956,     0,   218,     0,     0,     0,   159,     0,   160,     0,
       0,     0,     0,   161,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,   220,     0,     0,     0,     0,   958,   959,
       0,     0,     0,     0,     0,     0,     0,     0,   942,     0,
       0,   221,     0,     0,   943,   222,   223,     0,     0,     0,
       0,     0,     0,     0,     0,   960,   961,     0,   773,     0,
       0,     0,     0,     0,     0,     0,     0,   962,     0,     0,
     227,   162,   163,   164,   165,     0,     0,   166,     0,   946,
       0,     0,     0,   947,     0,     0,     0,     0,   948,     0,
       0,     0,     0,     0,  1313,     0,     0,     0,   950,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,   172,     0,     0,  1314,
       0,     0,     0,     0,   174,     0,     0,     0,   175,     0,
       0,   176,   177,     0,     0,     0,   178,     0,     0,   179,
       0,   180,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,   182,     0,   183,
     184,   185,   186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1036,  1037,  1038,  1039,  1040,  1041,
       0,     0,     0,     0,  1042,     0,     0,     0,  1024,     0,
       0,     0,     0,     0,     0,     0,     0,   952,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,     0,     0,   217,   942,     0,     0,
       0,     0,     0,   943,     0,     0,   953,     0,     0,   954,
     955,     0,     0,     0,     0,     0,   956,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     162,   163,   164,   165,     0,     0,     0,     0,  1043,     0,
       0,     0,  1044,     0,     0,     0,     0,  1045,   219,   220,
    1350,     0,     0,  1046,   958,   959,     0,   950,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   221,     0,     0,
    1047,   222,   223,     0,     0,  1048,     0,     0,  1049,     0,
       0,   960,   961,  1050,   773,     0,     0,  1051,     0,     0,
    1052,  1053,     0,   962,     0,  1054,   227,     0,  1055,     0,
    1056,   942,     0,     0,     0,     0,     0,   943,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1057,     0,     0,  1058,     0,
       0,  1059,     0,     0,   162,   163,   164,   165,     0,     0,
       0,     0,  1043,     0,     0,     0,  1044,     0,     0,     0,
       0,  1045,     0,     0,     0,     0,     0,  1046,     0,     0,
       0,   950,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1047,     0,     0,     0,     0,  1048,
       0,     0,  1049,     0,     0,     0,     0,  1050,     0,     0,
       0,  1051,     0,     0,  1052,  1053,  1350,     0,     0,  1054,
       0,     0,  1055,     0,  1056,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   953,     0,     0,   954,   955,
       0,     0,     0,     0,     0,   956,     0,     0,     0,  1057,
       0,     0,  1058,     0,     0,  1059,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   942,     0,     0,
       0,     0,     0,   943,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   958,   959,     0,  1060,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     162,   163,   164,   165,     0,     0,     0,     0,  1043,     0,
     960,   961,  1044,     0,     0,     0,     0,  1045,     0,     0,
     360,     0,  1061,  1046,     0,   227,     0,   950,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   953,
    1047,     0,   954,   955,     0,  1048,     0,     0,  1049,   956,
       0,     0,     0,  1050,     0,     0,     0,  1051,     0,     0,
    1052,  1053,     0,  1083,  1024,  1054,     0,     0,  1055,     0,
    1056,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   958,   959,     0,
    1060,     0,     0,     0,     0,  1057,     0,     0,  1058,     0,
       0,  1059,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   942,   960,   961,     0,     0,     0,   943,
       0,     0,     0,     0,   360,     0,  1061,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   162,   163,   164,   165,
       0,     0,     0,     0,  1043,     0,     0,     0,  1044,     0,
       0,     0,     0,  1045,     0,     0,     0,     0,     0,  1046,
       0,     0,     0,   950,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   953,  1047,     0,   954,   955,
       0,  1048,     0,     0,  1049,   956,     0,     0,     0,  1050,
       0,     0,     0,  1051,     0,     0,  1052,  1053,     0,     0,
       0,  1054,     0,     0,  1055,     0,  1056,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   958,   959,     0,  1060,     0,     0,     0,
       0,  1057,     0,     0,  1058,     0,     0,  1059,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     960,   961,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1061,     0,     0,   227,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   158,     0,     0,
       0,     0,     0,     0,   159,     0,   160,     0,     0,     0,
     327,   161,   328,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   329,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   330,   331,
       0,   953,     0,   332,   954,   955,   333,     0,     0,     0,
       0,   956,     0,     0,     0,   334,   335,   336,   337,   338,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   340,     0,   341,     0,     0,   162,
     163,   164,   165,     0,     0,   166,     0,   167,     0,   958,
     959,   168,  1060,     0,     0,     0,   169,     0,     0,     0,
       0,     0,   170,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   960,   961,     0,   171,
       0,     0,     0,     0,   172,     0,  1084,   173,  1061,     0,
       0,   227,   174,     0,     0,     0,   175,     0,     0,   176,
     177,     0,     0,     0,   178,     0,     0,   179,     0,   180,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,   182,     0,   183,   184,   185,
     186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   187,   188,   189,   190,     0,
       0,     0,     0,     0,   191,   192,     0,     0,   193,   194,
     342,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,     0,     0,   343,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,     0,     0,   158,   345,     0,   346,     0,     0,     0,
     159,     0,   160,     0,     0,     0,     0,   161,     0,   347,
       0,     0,     0,     0,     0,   348,     0,     0,     0,     0,
     349,     0,     0,     0,     0,     0,   219,   220,     0,   350,
       0,     0,   351,   352,   353,   354,     0,     0,     0,   355,
       0,     0,   942,   356,   357,   221,     0,     0,   943,   222,
     223,     0,     0,     0,     0,     0,     0,     0,   358,     0,
       0,   224,   225,     0,     0,     0,     0,   359,     0,   360,
       0,   226,     0,     0,   227,   162,   163,   164,   165,     0,
       0,   166,     0,   946,     0,     0,     0,   947,     0,     0,
       0,     0,   948,     0,     0,     0,     0,     0,  1313,     0,
       0,     0,   950,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
     172,     0,     0,  1314,     0,     0,     0,     0,   174,     0,
       0,     0,   175,     0,     0,   176,   177,     0,     0,     0,
     178,     0,     0,   179,     0,   180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     181,   182,     0,   183,   184,   185,   186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   952,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     953,     0,     0,   954,   955,     0,     0,     0,     0,   158,
     956,     0,   218,     0,     0,     0,   159,     0,   160,     0,
       0,     0,     0,   161,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,   220,     0,     0,     0,     0,   958,   959,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,   222,   223,     0,     0,     0,
       0,     0,     0,     0,     0,   960,   961,     0,   773,     0,
       0,     0,     0,     0,     0,     0,     0,   962,     0,     0,
     227,   162,   163,   164,   165,     0,     0,   166,     0,   167,
       0,     0,     0,   168,     0,     0,     0,     0,   169,     0,
       0,     0,     0,     0,   170,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,   172,     0,     0,   173,
       0,     0,     0,     0,   174,     0,     0,     0,   175,     0,
       0,   176,   177,     0,     0,     0,   178,     0,     0,   179,
       0,   180,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,   182,     0,   183,
     184,   185,   186,   158,     0,     0,     0,     0,     0,     0,
     159,     0,   160,     0,     0,     0,     0,   161,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   187,   188,   189,
     190,     0,     0,     0,     0,     0,   191,   192,     0,     0,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,     0,     0,   217,     0,     0,     0,
       0,     0,     0,     0,     0,   162,   163,   164,   165,     0,
       0,   166,     0,   167,     0,     0,     0,   168,   218,     0,
       0,     0,   169,     0,     0,     0,     0,     0,   170,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,     0,   219,   220,
     172,     0,     0,   173,     0,     0,     0,     0,   174,     0,
       0,     0,   175,     0,     0,   176,   177,   221,     0,     0,
     178,   222,   223,   179,     0,   180,     0,     0,     0,     0,
       0,     0,     0,   224,   225,     0,     0,     0,     0,     0,
       0,   360,     0,   226,     0,     0,   227,     0,     0,     0,
     181,   182,     0,   183,   184,   185,   186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1024,     0,     0,     0,
       0,   187,   188,   189,   190,     0,     0,     0,     0,     0,
     191,   192,     0,     0,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,     0,     0,
     217,     0,     0,     0,     0,   942,     0,     0,     0,     0,
       0,   943,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   162,   163,
     164,   165,     0,     0,     0,     0,  1043,     0,     0,     0,
    1044,     0,   219,   220,     0,  1045,     0,     0,     0,     0,
       0,  1046,     0,     0,     0,   950,     0,     0,     0,     0,
       0,   221,     0,     0,     0,   222,   223,     0,  1047,     0,
       0,     0,     0,  1048,     0,     0,  1049,   224,   225,     0,
       0,  1050,     0,     0,     0,  1051,     0,   226,  1052,  1053,
     227,     0,   158,  1054,     0,     0,  1055,     0,  1056,   159,
       0,   160,     0,     0,     0,     0,   161,  2264,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1057,     0,     0,  1058,     0,     0,  1059,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  2265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2266,     0,     0,     0,     0,
     166,     0,   167,     0,     0,     0,   168,     0,     0,     0,
       0,   169,     0,     0,     0,     0,     0,   170,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   953,   171,     0,   954,   955,     0,   172,
       0,     0,   173,   956,     0,     0,     0,   174,     0,     0,
       0,   175,     0,     0,   176,   177,     0,     0,     0,   178,
       0,     0,   179,     0,   180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   958,   959,     0,  1060,     0,     0,     0,     0,   181,
     182,     0,   183,   184,   185,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   960,   961,
       0,     0,     0,     0,     0,     0,     0,     0,   360,     0,
    1061,     0,     0,   227,     0,     0,     0,     0,     0,     0,
     187,   188,   189,   190,     0,     0,     0,     0,     0,   191,
     192,     0,     0,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,     0,     0,  2267,
       0,   158,     0,     0,     0,     0,     0,     0,   159,     0,
     160,     0,     0,     0,  2268,   161,     0,     0,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    2269,     0,     0,     0,     0,  2270,     0,  2271,     0,     0,
       0,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  2272,     0,     0,     0,     0,     0,
     221,     0,     0,     0,   222,   223,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   224,   225,     0,   166,
       0,   167,     0,     0,     0,   168,     0,     0,     0,     0,
     169,     0,     0,     0,     0,     0,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   171,     0,     0,     0,     0,   172,     0,
       0,   173,     0,     0,     0,     0,   174,     0,     0,     0,
     175,     0,     0,   176,   177,     0,     0,     0,   178,     0,
       0,   179,     0,   180,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   181,   182,
       0,   183,   184,   185,   186,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   187,
     188,   189,   190,     0,     0,     0,     0,     0,   191,   192,
       0,     0,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     3,     0,     0,     0,     4,     0,
       5,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     219,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       6,     7,     8,     9,     0,     0,     0,     0,     0,   221,
       0,     0,    10,   222,   223,     0,     0,     0,     0,     0,
      11,     0,    12,     0,    13,   224,   225,    14,    15,    16,
       0,    17,     0,     0,     0,     0,    18,    19,    20,     0,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,     0,     0,     0,     0,   942,
       0,     0,     0,     0,     0,   943,     0,     0,   334,   335,
     336,  1936,  1937,   339,     0,     0,     0,     0,     0,     0,
    -445,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,   162,   163,   164,   165,    36,     0,     0,     0,
    1043,     0,    37,     0,  1044,     0,     0,     0,     0,  1045,
       0,     0,     0,     0,     0,  1046,     0,     0,     0,   950,
       0,     0,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,  1047,     0,     0,     0,     0,  1048,     0,     0,
    1049,     0,     0,     0,   942,  1050,     0,     0,     0,  1051,
     943,     0,  1052,  1053,     0,     0,     0,  1054,     0,     0,
    1055,     0,  1056,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   162,   163,   164,
     165,     0,     0,     0,     0,  1043,     0,  1057,     0,  1044,
    1058,     0,     0,  1059,  1045,     0,     0,     0,     0,     0,
    1046,     0,     0,     0,   950,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1047,     0,     0,
       0,     0,  1048,     0,     0,  1049,     0,     0,     0,     0,
    1050,     0,     0,     0,  1051,     0,     0,  1052,  1053,     0,
       0,     0,  1054,     0,     0,  1055,     0,  1056,     0,     0,
       0,     0,     0,     0,    39,     0,   942,     0,     0,     0,
       0,     0,   943,     0,     0,     0,     0,     0,     0,    40,
       0,     0,  1057,     0,  -445,  1058,    41,   953,  1059,     0,
     954,   955,     0,     0,     0,     0,     0,   956,     0,  1747,
     163,   164,   165,     0,     0,    42,     0,  1043,     0,     0,
      43,  1044,    44,     0,     0,     0,  1045,     0,     0,     0,
       0,     0,  1046,     0,     0,     0,   950,     0,     0,     0,
       0,     0,     0,     0,     0,   958,   959,     0,  1060,  1047,
       0,     0,     0,     0,  1048,     0,     0,  1049,     0,     0,
       0,     0,  1050,     0,     0,     0,  1051,     0,     0,  1052,
    1053,     0,   960,   961,  1054,     0,     0,  1055,     0,  1056,
       0,     0,   953,     0,  1061,   954,   955,   227,     0,     0,
       0,     0,   956,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1057,     0,     0,  1058,     0,     0,
    1059,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     958,   959,     0,  1060,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   960,   961,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1061,
       0,     0,   227,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   953,     0,     0,   954,   955,     0,
       0,     0,     0,     0,   956,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   958,   959,     0,  1060,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   960,
     961,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1061,     0,     0,   227
};

static const yytype_int16 yycheck[] =
{
       1,    38,     3,   526,   363,   222,   454,   585,   453,   534,
      69,    54,   338,    69,   374,   451,   702,   646,   380,    37,
     419,   400,   464,    66,     1,  1054,   898,   735,   942,   407,
      32,   226,   184,    30,    30,   613,   484,   327,  1351,  1352,
    1491,  1492,  1504,   621,    54,  1358,   774,    48,   492,  1492,
     584,   452,  1492,  1497,   496,    56,    66,    54,    54,   637,
      61,   891,   493,  1499,  1621,  1622,   956,   357,  1496,    66,
      66,  1498,  1439,  1438,  1017,  1338,   434,    36,  1514,  1353,
     481,  1353,   397,  1511,   399,  1611,   419,   110,   532,   112,
     450,   114,   115,   863,   495,   118,   458,   120,  1353,   185,
     123,   124,   533,   393,   682,   128,   129,   130,   131,   132,
    1655,  1059,  1669,   136,   474,  1353,   139,  1353,   549,  1835,
    1812,   144,   145,   419,   182,  1030,   896,   185,   490,   152,
    1791,   901,  1833,  1446,   430,   431,  1834,    30,   182,  1589,
    1590,   185,   182,   544,  1808,   185,   182,  1808,   726,   185,
     920,  1031,   880,   182,   924,   962,   185,   182,  1032,  1734,
     185,    54,  1598,   525,  2034,   182,   182,     5,   185,   185,
     419,   182,  1034,    66,   185,     5,  2066,   182,  1860,    12,
     185,   430,   431,   761,   546,   195,    40,   182,     8,     8,
     185,   379,     8,   182,   182,     8,   185,   185,   182,     8,
    1624,   185,  1028,  1029,   106,     1,    24,  1666,    60,     5,
     182,     7,   738,   185,   740,   182,   742,    21,   185,   182,
     107,   747,   185,    59,  1321,   222,   222,    82,    83,   226,
     226,    27,    28,    29,    30,   859,  1793,  1794,   100,   107,
     862,    74,    88,    24,    90,    78,   771,     8,   856,   857,
     863,    93,  1809,  1810,    51,    16,    17,    18,    19,  2098,
      23,    94,    95,  2182,   107,   108,   109,  1632,  1633,   107,
     108,   109,   107,   798,   125,    88,   108,   279,   337,   338,
     113,   107,  1733,   285,    30,   811,   799,    65,    24,    88,
      30,    93,    93,  1733,    96,    96,     7,     8,     9,   861,
     114,   106,    59,   107,   298,   138,   139,    62,    54,    47,
      48,   107,   108,   109,    54,   129,   149,  1057,   841,   844,
      66,   148,   298,   156,   102,    13,    66,    62,   355,   222,
      88,   358,   342,   226,    93,   107,   108,   109,   165,    50,
     130,    52,    55,   106,   107,  1115,     8,   129,   407,    30,
     107,    62,   323,   363,    30,  1806,    67,    60,    51,    21,
      71,    23,   107,   419,   119,    49,   363,   363,  1819,   168,
    1056,   884,   154,    54,   430,   431,    32,    98,    54,  1830,
    1831,    93,   376,   165,   119,    66,    95,   829,  1831,   837,
      66,  1831,  1666,   452,  1666,    59,   452,  1671,   842,  1671,
     376,   247,    90,   107,   447,   376,    59,    82,    34,    35,
      36,  1666,   843,   110,   301,   112,  1671,   114,  1047,     8,
     463,  1050,   481,   120,    99,   481,   123,  2346,  1666,   138,
    1666,   299,    21,  1671,   835,  1671,   495,   447,   107,   495,
    1537,    59,   139,   107,   119,   296,   288,  1388,   290,  1390,
     447,   447,   130,   463,   107,   152,  1318,   290,   291,   343,
     138,   117,   117,    88,   299,    90,   463,   463,  2307,   353,
     363,   317,    30,   315,  1214,   301,   222,     1,   840,   381,
     226,     5,   222,     7,   139,   544,   226,  1111,   544,   107,
     168,  1950,  1114,   516,   381,   299,    54,   152,  1922,  1107,
    1108,  1109,  1115,    27,    28,    29,    30,   297,    66,     1,
     362,   363,   364,     5,   376,     7,   227,     5,  1126,     7,
     325,  2182,   376,   359,   360,  2207,   381,  2191,    39,   288,
    2191,   290,  1983,   376,   347,    27,    28,    29,    30,  2096,
    2430,   222,   297,   381,   301,   226,   222,   377,    30,    47,
     226,  1113,  1322,  1323,   447,  1417,   301,   377,   377,   361,
     361,   377,   714,   381,  1469,   361,     9,  2437,   377,     9,
     463,   292,    54,    16,    17,    18,    19,   289,  1156,    90,
      23,   377,   291,  2158,    66,   343,   344,   345,   370,  2281,
    1470,    21,     6,    23,   612,   376,   975,  1471,   635,    59,
     381,  2265,   898,    88,  2265,    97,   262,   263,  1415,   381,
     373,  2327,  2157,    56,  1981,     9,    56,   363,   381,   380,
     295,  2147,   247,   363,  2325,  1938,   337,   338,  2326,   715,
     341,  1579,   258,   302,  2084,   376,  1170,   301,  1172,   598,
     376,  1467,  1468,   958,   959,   381,   138,   107,   301,   376,
    1092,   375,   114,   870,   381,   713,  1034,   715,   375,   362,
     363,   364,    56,   318,   222,   131,   962,   129,   226,  1932,
    1933,   715,  1985,   713,   891,   715,  1950,   713,  1950,   715,
    1288,  1080,   363,   301,   713,  2346,   715,   363,   713,    93,
     715,   157,   317,   730,   376,  1950,   713,   713,   715,   715,
     146,   447,  1035,  1389,   715,   538,   148,   447,  1651,   727,
     715,   153,  1950,   159,  1950,     8,  2081,   463,   713,  1245,
     715,   136,    93,   463,   713,   713,   715,   715,    21,   713,
     176,   715,  1028,  1029,  1030,  1031,  1032,    88,    93,  1035,
     222,   713,  1100,   715,   226,  1060,   713,  1080,   715,   131,
     713,   117,   715,  1694,   465,     5,  1697,     7,   469,  1700,
     247,    88,   473,     0,   247,  1061,   447,   478,     5,   157,
       7,   447,   483,  1663,   298,   157,   835,   354,   129,   835,
     168,   169,   463,  1655,  1080,   296,  1311,   463,  2155,  2251,
     282,    41,   363,   281,  1769,  1770,  1771,  1772,  1773,  2243,
     150,  1673,   129,   154,   297,   363,   172,   173,   296,  2245,
    1333,   322,  1061,   301,  2242,   181,   327,  2244,   329,   516,
    1650,    30,     1,   115,   116,   117,     5,   154,     7,   540,
     323,  1080,   891,    88,   322,    90,  1370,   361,  1033,   327,
     106,   301,   898,  1613,  2401,    54,   106,   558,    27,    28,
      29,    30,   854,   377,   172,   173,     9,    66,   343,   344,
     345,  1305,  1734,    16,    17,    18,    19,   281,   106,   361,
      23,  2322,  2323,   870,   870,  1317,     5,  1308,     7,     8,
    2323,   363,   296,  2323,   288,   377,   290,   301,  1596,   447,
    1334,     9,   338,   176,   891,   891,     9,   376,    16,    17,
      18,    19,   381,    56,  1335,   463,  1058,     8,   322,   107,
    1638,  1639,    25,   327,   951,    40,    41,   288,    97,   290,
      21,     1,    23,    93,  2237,     5,    96,     7,     1,   279,
     280,   949,     5,   288,     7,   290,   117,   380,    56,  1301,
    1318,     6,    88,    56,     9,  1631,   168,    27,    28,    29,
      30,  1139,  1530,   134,    27,    28,    29,    30,  1017,   138,
    1409,  1410,     1,   377,     9,   447,     5,  1416,     7,    10,
    1332,    16,    17,    18,    19,  1034,   297,   870,   171,  1035,
       1,   463,  1507,   176,     5,   297,     7,   168,    27,    28,
      29,    30,   247,  1283,  1680,   115,   116,  1575,   891,    40,
      41,    42,    43,    44,   372,   373,    27,    28,    29,    30,
       5,    56,     7,   222,     9,    81,   297,   226,     6,   369,
    2333,     9,    43,     5,  1080,    46,   106,   361,     1,  2004,
    2005,  2006,     5,   106,     7,    76,  1033,  1033,     5,  1417,
       7,    63,    64,    84,    85,    86,    87,    88,   114,   325,
     326,   328,  1494,  1432,    27,    28,    29,    30,  1972,   298,
     299,  1505,   317,   129,     8,   337,   191,  1466,   109,   194,
      43,    65,    66,    46,  1473,  1506,    70,    21,    23,    23,
      18,    26,  1270,  1271,  1085,   126,   127,    19,  1089,     6,
     380,   113,   133,   134,   135,   317,   318,   319,   320,   321,
     376,   123,   143,   282,    54,   146,    54,   129,  1325,   376,
     176,   281,   376,  1409,  1410,  1411,  1412,  1413,   376,  1415,
    1416,   325,   326,   129,   870,    21,   296,    23,  1424,  1425,
     870,   301,   154,   325,   326,   106,   107,    59,   258,   259,
    1033,  1503,   262,   376,   150,   891,   117,   168,   154,   106,
     107,   891,   322,  1171,   363,  1173,   376,   327,  1866,     1,
     117,  1179,   298,     5,   376,     7,    47,   119,   168,  1465,
    1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,
    2209,    45,   361,   135,     9,    27,    28,    29,    30,   870,
     117,   361,    16,    17,   870,   168,   348,   349,   377,   265,
      47,    43,     1,  1781,    46,   157,     5,   134,     7,    47,
     891,     8,   164,   140,   141,   891,  1465,  1466,    16,    17,
    1664,   321,  1439,  1472,  1473,  1474,  1475,   380,    27,    28,
      29,    30,    16,    17,  1665,   198,   199,     1,   447,   115,
     116,     5,   107,     7,    43,   325,   326,    46,   248,   249,
     250,   106,   325,   326,   463,   382,   106,   107,   382,  1318,
     196,   197,   380,    27,    28,    29,    30,   117,   309,   310,
     311,   134,   354,   279,   280,   296,   139,  1636,   354,     1,
     106,   361,   371,     5,   323,     7,  2158,  1033,   361,   379,
       1,     5,     6,  1033,     5,   376,     7,   377,   115,   116,
    1662,   253,   254,    88,   377,    27,    28,    29,    30,   172,
     173,   181,   870,  1024,    24,  1757,    27,    28,    29,    30,
     247,    43,   361,   296,    46,    24,   168,  1515,  1325,  1325,
    1518,   106,   107,   891,   139,   380,   115,   116,   377,    51,
      52,    54,    54,    55,    56,   107,    98,    99,  1727,    54,
    1729,    54,  1033,  1752,  1753,   119,   377,  1033,  1417,  1655,
     281,  1417,    54,   369,   281,  1888,   130,   172,   173,   168,
     132,   133,   117,   115,   116,   296,   106,  1673,    54,   296,
     301,  1906,    93,   376,   301,   106,   131,    88,   870,   100,
     115,   116,  1970,  1971,   139,  1973,     1,   362,   363,   364,
       5,   322,     7,   208,   377,   322,   327,  2177,   106,   891,
     327,  1599,   157,   106,   455,   456,   457,    45,    46,   164,
     305,   306,    27,    28,    29,    30,   106,   138,   106,   107,
     139,   242,  1325,  1650,  1424,  1425,   241,   106,  1734,   106,
     251,   252,  1439,  1439,   255,   256,   168,   106,   489,    88,
      88,  1895,  1640,  1641,   296,  1643,  1752,  1753,    18,     6,
     377,  1823,   173,   172,   173,  2043,   175,    19,   380,  1900,
       8,     8,     1,  1661,     8,  1033,     5,     8,     7,     6,
    1490,   106,   523,   362,   106,  1486,  1487,     8,  1489,  1490,
    1491,  1492,    97,  1490,  1490,     8,   258,   296,    27,    28,
      29,    30,   376,  1752,  1753,    19,    18,   117,   380,   119,
    1487,  1873,  1489,  1490,  1491,  1492,   278,   279,  1536,  2148,
    2149,    47,   286,    49,  2210,   347,   348,   349,    54,   139,
      56,     9,   106,   138,    60,   377,     8,   147,   148,   149,
     150,     1,   107,   108,   109,     5,  1439,     7,  1585,  1586,
     106,  1033,  2000,  1998,  2077,   101,   102,   103,   104,   105,
      40,  1997,  2332,  2333,   376,  1583,  1584,    27,    28,    29,
      30,   282,   376,   376,   296,   101,   231,   103,   377,  1325,
     235,   236,   237,   107,   296,  1325,   110,   111,   112,  1648,
     376,  1650,  1651,   343,   344,   345,   360,  1490,   310,   311,
      89,    90,    91,  1604,    93,  1606,   376,  1608,   298,  1610,
     376,  1937,  1613,   377,   106,   327,   117,  1673,   119,   120,
     121,   122,  2392,   323,   106,   106,  1636,  1604,   376,  1606,
    2031,  1608,  1675,  1610,   107,  1995,  1613,   106,   106,  1636,
    1636,     1,   376,   106,  1325,     5,   100,     7,   106,  1325,
     361,   195,   200,  1650,  1650,   377,   235,   134,  2059,  1677,
     238,   870,   238,   240,   239,  1675,   377,    27,    28,    29,
      30,   239,   106,   106,  1675,   106,   106,   282,  1675,  1675,
     355,   106,   891,     1,   106,  2455,   106,     5,   376,     7,
      30,   376,   376,  1439,   376,     6,   131,     9,   106,  1439,
      54,    23,    45,  1920,   331,   332,   333,   334,   335,    27,
      28,    29,    30,   148,    54,    48,   153,    50,   324,    52,
      53,  1780,   157,   324,    21,     6,    66,    60,    61,   164,
     165,   379,   106,    93,   230,   231,   232,   233,   234,  1927,
     100,    88,   288,  1636,  1490,     9,  2271,    88,    23,    23,
    1490,   297,   298,   299,   300,   301,   361,  1650,  1439,   379,
     106,   376,    95,  1439,  1981,   376,     8,  1325,   376,   383,
    2348,   383,   377,     6,   106,    45,  1777,  2136,   138,   308,
     299,   107,  1675,   376,   195,  1786,  1787,  1788,  1789,  1790,
      47,   323,    49,     8,  1791,  1791,    21,   107,   839,   117,
     106,   106,  1803,  1804,  1805,  1806,    63,    64,   328,  1490,
      73,  1808,  1808,   173,  1490,     6,  1817,  1818,  1819,  1829,
    2398,   170,  2264,     8,  1033,  2269,  2404,  1828,  1829,  1830,
    1831,  1841,  1829,  1829,   106,    41,  1837,   102,   106,  2270,
    1841,   107,   375,  1325,  1841,  1841,   107,   307,   377,   102,
     107,  1828,  1829,  1830,  1831,   106,   113,   375,   375,   375,
    1837,     8,  2158,   106,  1841,   106,   123,   106,  1869,  1870,
    1871,   106,   129,   142,   143,   144,   145,  1936,  1937,   106,
     106,  1439,   222,   106,   377,   298,   226,   106,    88,   106,
    1636,  1892,  1869,  1870,  1871,     6,  1636,   154,  1791,   194,
       8,  2424,  2427,  2091,  1650,   107,  2268,     9,     9,   155,
    1650,   106,   106,  2101,   375,  1808,   106,   377,   377,   108,
    2108,   376,   282,  1920,  1920,   226,   227,   228,   229,  1675,
      47,   376,  1490,   376,   376,  1675,  1829,   191,  2155,     1,
     376,     8,   138,     5,   376,     7,    63,    64,  1841,   376,
     376,    43,   376,  1954,  2013,  1636,   377,  1439,   376,   376,
    1636,   376,   376,   107,   282,    27,    28,    29,    30,  1650,
     376,   376,  2031,  2415,  1650,  2031,   377,   376,    40,   376,
       9,  2425,     9,   106,  1981,  1981,   107,   106,   116,   117,
     107,   119,   106,     6,  1675,  2426,   113,   106,   375,  1675,
    2059,   361,   106,  2059,   209,   309,   123,  2066,  1490,   106,
     106,   139,   129,  2201,  2202,  2203,   106,   377,   106,   147,
     148,  1051,   150,   363,  1053,   943,   814,  1920,  2029,  2030,
     813,   812,   160,   161,   162,  1052,   590,   154,   815,  1523,
     580,  1164,  1055,   361,   301,  1791,    20,   449,  2049,   289,
     139,  1791,  1763,  1764,  1765,  1766,  2057,  2058,   290,   377,
    2061,  2423,  1808,  2168,  1495,  1813,  2067,  2090,  1808,  2189,
    2195,  2333,  2187,  2074,  2075,  2076,  2250,  2100,  1636,   419,
    2253,  2252,  2254,  1829,  2107,  2241,  2394,   295,  1981,  1829,
     430,   431,  1650,  1765,   446,  1841,  1985,  2074,  2075,  2076,
     913,  1841,  2007,  2008,  2009,  2010,  2011,   447,     1,   925,
    1791,  1956,     5,  1905,     7,  1791,  1325,  1675,  2177,  1303,
    1893,   388,  1687,   463,  1894,  1689,  2136,  1808,   115,   116,
    1607,  1607,  1808,  1000,    27,    28,    29,    30,  2197,  2136,
    2136,   448,  2240,   130,  2319,   132,   133,   134,  1829,  1411,
    1413,  1412,  2340,  1829,  1636,  1430,  1650,  1648,  2155,  2155,
    1841,  2162,  1020,   892,   419,  1841,  1437,   329,  1650,   156,
     157,   158,    30,  1154,  1920,   430,   431,  2178,  1475,   222,
    1920,  1358,  1637,  1920,   301,  2182,  2182,   431,  2225,  2226,
    1671,  1671,  2402,  1675,  2191,  2191,   458,  2198,  2199,   342,
      93,   461,   460,   462,   343,  2223,  2224,   100,   736,   459,
    2211,  2212,   343,  2214,  2215,  2216,  2217,  2218,  2295,   713,
    2221,  1223,    -1,   680,    -1,  1936,  1937,    -1,    -1,    -1,
    1439,    -1,    -1,  1791,    30,  1981,    -1,    -1,    -1,  1920,
      -1,  1981,    -1,  2136,  1920,   138,    -1,    -1,    -1,    -1,
    1808,    -1,    -1,  2441,  2442,  2443,    -1,    -1,    54,    -1,
      -1,    -1,  2155,    -1,  2169,  2170,  2171,    -1,  2265,  2265,
      66,  1829,    -1,  2274,  2333,    -1,  2277,    -1,    -1,  2280,
     173,  1490,    -1,  1841,   115,   116,    -1,    -1,    -1,  2182,
      -1,    -1,    -1,   124,   125,   126,   127,   128,  2191,   361,
    1981,    -1,    -1,    -1,    -1,  1981,    -1,    -1,    -1,  1791,
      -1,  2321,  2313,  2314,  2315,   377,    -1,    -1,    -1,  2320,
    2321,  2322,  2323,    -1,  2321,  2321,  1808,  2328,  2329,  2330,
    2331,  2332,    -1,  2392,    -1,    -1,  2313,  2314,  2315,    -1,
      -1,    -1,    -1,  2320,  2321,  2322,  2323,  1829,  2349,  2346,
    2346,  2328,  2329,  2330,  2331,  2332,  2357,    -1,    -1,  1841,
      -1,  2362,  1920,    -1,    -1,    -1,  2367,    -1,    -1,    -1,
      -1,  2430,  2265,    -1,  2375,    -1,  2377,  2378,  2379,  2380,
    2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,  2389,   282,
    2136,    -1,    -1,    -1,    -1,    -1,  2136,    -1,    -1,    -1,
      -1,    -1,  2403,    -1,    -1,  2382,  2383,  2384,    -1,  2155,
      -1,    -1,    -1,    -1,    -1,  2155,    -1,    -1,    -1,  2420,
    2421,  2422,    -1,  1981,    -1,   117,   222,  1636,  2321,    -1,
     226,    -1,    -1,    -1,    -1,    -1,  2182,   129,  1920,   131,
      -1,  1650,  2182,     1,    -1,  2191,    -1,     5,    -1,     7,
      -1,  2191,    -1,  2346,    -1,  2136,    -1,    -1,   150,    -1,
    2136,    -1,   154,    -1,    -1,   157,  1675,    -1,   361,    27,
      28,    29,    30,    -1,  2155,  1354,    -1,    -1,    -1,  2155,
      -1,    -1,   118,    -1,   377,     1,    -1,    -1,   124,     5,
      -1,     7,   128,   129,   130,   131,   132,    -1,    -1,  1981,
     136,  2182,    -1,    -1,    -1,    -1,  2182,    -1,   144,   145,
    2191,    27,    28,    29,    30,  2191,    -1,    -1,    -1,  2265,
       1,    -1,    -1,    -1,     5,  2265,     7,    -1,    -1,    -1,
     870,    82,    83,    84,    85,    86,    87,     1,    -1,    -1,
      -1,     5,    -1,     7,    -1,    -1,    27,    28,    29,    30,
      -1,   891,    -1,   867,   246,   869,    -1,    -1,   898,   117,
      -1,    -1,    -1,    27,    28,    29,    30,   363,    -1,    -1,
      -1,    -1,   886,   887,   888,  2321,   890,    -1,  2136,   893,
     894,  2321,  1791,    -1,  2265,    -1,    39,   279,   280,  2265,
      -1,   107,   108,   109,    47,    -1,    49,  2155,    51,  1808,
    2346,    54,    55,    56,    -1,    58,  2346,    -1,  1487,    -1,
    1489,  1490,  1491,  1492,    -1,    14,    15,    16,    -1,    -1,
    1829,    -1,   962,   419,  2182,    -1,    -1,   941,    -1,    -1,
     944,    -1,  1841,  2191,   430,   431,   117,    -1,    -1,    -1,
    2321,    -1,    -1,   898,    -1,  2321,    -1,    -1,   115,   116,
      -1,   447,    -1,    -1,  2136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,  2346,    -1,   463,    -1,    -1,
    2346,   138,    -1,  2155,    -1,    -1,    -1,   369,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,   154,  1028,  1029,
    1030,  1031,  1032,  1033,    -1,  1035,    -1,  1011,    97,    -1,
    2182,    -1,    -1,    -1,    -1,    -1,    -1,  2265,   107,  2191,
      -1,  1920,    -1,    -1,    -1,    -1,    -1,   116,   117,    -1,
      -1,  1061,    -1,   122,   282,  1604,    -1,  1606,    -1,  1608,
      -1,  1610,     1,    -1,  1613,    -1,     5,    -1,     7,    -1,
    1080,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,    -1,    -1,   154,    -1,    -1,    27,    28,
      29,    30,    -1,  2321,    -1,    -1,    -1,  1081,  1082,    -1,
      -1,    40,  1981,  1028,  1029,  1030,  1031,  1032,    -1,    -1,
    1035,    -1,    -1,  2265,    -1,     1,    -1,    -1,  2346,     5,
      -1,     7,  1671,    -1,     1,    -1,    -1,    -1,     5,    -1,
       7,   282,    -1,   361,    -1,    -1,  1061,    -1,    -1,    -1,
      -1,    27,    28,    29,    30,   282,   283,   284,   285,   377,
      27,    28,    29,    30,    -1,  1080,    -1,    -1,   281,    -1,
      -1,   298,   299,   300,   298,    -1,   115,   116,    -1,  2321,
     119,    -1,    -1,   296,    -1,   361,    -1,    -1,    -1,    -1,
     303,   130,   131,   132,   133,   134,   135,   136,    -1,   138,
     139,   377,    -1,    24,  2346,    -1,    -1,    -1,    -1,   322,
      -1,    -1,    -1,    -1,   327,    -1,   329,    -1,     1,    40,
     361,    -1,     5,    -1,     7,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   293,   294,   377,   361,    -1,    -1,
      -1,    -1,   181,    -1,    27,    28,    29,    30,    -1,    -1,
      -1,    -1,   119,   377,    -1,    -1,    -1,    -1,    39,    42,
      -1,    -1,   129,    -1,    -1,    -1,    47,  2136,    49,   136,
     137,    -1,    -1,    54,    55,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,  2155,   154,    -1,  1828,
    1829,  1830,  1831,    -1,    -1,    -1,    -1,    -1,  1837,    -1,
      -1,    -1,  1841,    -1,    -1,   172,   173,    -1,    -1,    92,
      93,    94,    95,  2182,    97,    -1,    -1,   100,    -1,    -1,
      -1,    -1,  2191,    -1,    -1,  1325,    -1,    -1,    -1,    -1,
    1869,  1870,  1871,    -1,   117,   202,    -1,    -1,  1312,   206,
      -1,    -1,  1316,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,  1326,  1327,  1328,  1329,    -1,  1331,   179,   148,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,  1348,   164,    -1,    -1,    -1,   168,
      -1,    -1,    -1,   172,   173,   168,    -1,    -1,   177,   178,
     179,   180,   181,    -1,    -1,    -1,  2265,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1409,
    1410,  1411,  1412,  1413,   870,  1415,  1416,    -1,    -1,    -1,
    1394,    -1,  1396,    -1,  1424,  1425,    -1,    -1,    -1,  1403,
      -1,    -1,   361,    -1,    -1,   891,    -1,   304,    -1,  1439,
      -1,    -1,   898,    -1,  1418,    -1,  1420,   323,   377,    -1,
      -1,    -1,  2321,    -1,    -1,  1429,   515,  1431,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1465,  1466,  1467,  1468,  1469,
    1470,  1471,  1472,  1473,  1474,  1475,    -1,  2346,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,
    1490,    -1,    -1,    -1,   361,    -1,    -1,   556,   281,     1,
     281,   377,    -1,     5,    -1,     7,    -1,    -1,    -1,    -1,
     377,    -1,    -1,   296,    -1,   296,    -1,    -1,   301,    -1,
      -1,    -1,   303,    -1,    -1,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    -1,    -1,  2074,  2075,  2076,    -1,   322,
      42,   322,    -1,    -1,   327,    -1,   327,    -1,   329,    -1,
    1465,  1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,
    1475,  1017,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1028,  1029,  1030,  1031,  1032,  1033,   361,  1035,
      -1,    -1,     1,    -1,    -1,    -1,     5,    -1,     7,    -1,
      92,    93,    94,    95,   377,    97,    -1,    -1,   100,    -1,
       1,    -1,    -1,    -1,     5,  1061,     7,    -1,    27,    28,
      29,    30,    -1,    -1,    -1,   117,    -1,    -1,    -1,     1,
      39,    -1,    -1,     5,  1080,     7,    27,    28,    29,    30,
      49,    -1,    -1,    -1,    -1,    54,  1636,    56,    -1,    58,
      -1,    42,  1616,  1617,    -1,    27,    28,    29,    30,    -1,
    1650,    -1,    -1,    -1,    -1,  1655,    -1,    -1,    -1,    40,
      42,    -1,    -1,    -1,    -1,    -1,   168,    -1,  1642,    -1,
      -1,    -1,    -1,  1673,    -1,  1675,    -1,    -1,  1652,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    -1,    97,   117,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,  1703,
      -1,    -1,    -1,    -1,  1734,   117,    -1,    -1,    -1,    -1,
    1655,    -1,    -1,    -1,    -1,    -1,  1720,    -1,  1722,   168,
      -1,    -1,  1752,  1753,    -1,    -1,    -1,    -1,  1673,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,
      -1,  2320,  2321,  2322,  2323,    -1,   168,    -1,    -1,  2328,
    2329,  2330,  2331,  2332,   296,    -1,    -1,    -1,   179,   301,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,  1734,
     322,    -1,    -1,    -1,    -1,   327,    -1,    -1,    -1,  1829,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1752,  1753,    -1,
      -1,  1841,    -1,  2382,  2383,  2384,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   361,
      -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,  1325,
      -1,    -1,    -1,    39,    -1,   377,    -1,   296,    -1,    -1,
     281,    47,    -1,    49,   303,    51,    -1,    -1,    54,    55,
      56,    -1,    58,    -1,    -1,   296,    -1,    63,    64,   281,
     301,    -1,    -1,   322,    -1,    -1,    -1,    -1,   327,    -1,
     329,    -1,    -1,    -1,   296,    -1,    -1,    -1,    -1,   301,
    1920,   322,    -1,    -1,    -1,    -1,   327,    -1,    -1,    -1,
      -1,  1387,  1388,    -1,  1390,    -1,    -1,    -1,    -1,  1913,
     322,    -1,   361,    -1,    -1,   327,    -1,   113,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   377,     1,
     361,  1935,    -1,   129,    -1,    -1,    -1,    -1,    -1,    11,
      12,    13,    14,    15,    16,    17,   377,    -1,    20,   361,
      -1,  1981,    -1,  1439,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,   377,    -1,    -1,    -1,    -1,
      -1,  1975,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1465,
    1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    -1,  1490,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,   104,   105,    -1,   107,    -1,    -1,    -1,   111,
      -1,   113,    -1,   115,   116,   117,   118,   119,    -1,    -1,
     122,    -1,   124,    -1,    -1,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,   136,    -1,   138,   139,   140,   141,
     142,    -1,   144,   145,    -1,    -1,   148,   149,   150,   151,
      -1,    -1,   154,    -1,   156,   281,  2116,  2117,    -1,    -1,
    2120,  2121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     296,    -1,    39,    -1,    -1,    -1,  2136,   303,    -1,    -1,
      47,    -1,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    58,    -1,    -1,    -1,  2155,   322,    -1,  2158,    -1,
      -1,   327,    -1,   329,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2153,
    1636,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1648,    -1,  1650,  1651,    -1,    -1,    -1,  1655,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,     5,    -1,     7,    -1,    -1,  1673,    -1,  1675,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    30,  1694,    -1,
      -1,  1697,    -1,  2158,  1700,    -1,   288,   289,   290,   291,
     292,   293,   294,   295,    -1,   297,   298,   299,   300,   301,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1734,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
       5,    -1,     7,    -1,    -1,    -1,  1752,  1753,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    27,    28,    29,    30,    31,    -1,    33,    -1,
      -1,  2321,    37,    38,    39,    -1,   119,    -1,    -1,    -1,
      -1,    46,    -1,    -1,    -1,    -1,   129,    -1,    -1,    54,
      -1,    -1,    -1,   136,   137,    -1,    -1,    -1,    -1,    -1,
      65,    66,    -1,    -1,    -1,    70,    -1,    -1,    73,    -1,
      -1,   154,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,  1829,   281,    -1,    -1,    -1,    -1,   172,
     173,    -1,    97,    -1,    -1,  1841,   101,    -1,   103,   296,
      -1,   106,   107,   108,   109,    -1,   303,   112,    -1,   114,
      -1,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   202,
      -1,    -1,    -1,   206,   129,   322,    -1,    -1,    -1,    -1,
     327,   112,   329,   114,    -1,    -1,    -1,   118,    -1,    -1,
      -1,   146,   123,    -1,    -1,    -1,   151,    -1,   129,   154,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,   168,    -1,   146,   171,    -1,    -1,   174,
     151,   176,   504,   154,  1920,    -1,    -1,    -1,   159,    -1,
      -1,    -1,   163,   515,    -1,   166,   167,   168,    -1,    -1,
     171,    -1,    -1,   174,    -1,   176,   201,   202,    -1,   204,
     205,   206,   207,    -1,    -1,    -1,   538,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     201,   304,   203,    -1,   556,    -1,   207,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1981,    -1,   242,   243,   244,
     245,    -1,    -1,    -1,    -1,    -1,   251,   252,    -1,    -1,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    -1,    -1,   281,    -1,   361,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   296,    -1,    -1,   377,    -1,   301,    -1,   303,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   316,    -1,    -1,    -1,    -1,    -1,   322,    -1,    -1,
      -1,    -1,   327,    -1,    -1,    -1,    -1,    -1,   333,   334,
      -1,   336,    -1,    -1,   339,   340,   341,   342,    -1,    -1,
      -1,   346,    -1,    -1,    93,   350,   351,   352,    -1,    -1,
      -1,   356,   357,    -1,    -1,     1,   361,    -1,    -1,     5,
     365,     7,    -1,   368,   369,    -1,   115,   116,    -1,   374,
     119,   376,   377,   378,    -1,    -1,   381,    -1,    24,    -1,
      -1,    27,    28,    29,    30,    31,    -1,    33,    -1,   138,
    2136,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,  2155,
      -1,    -1,  2158,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    -1,    -1,    70,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    97,    79,    -1,    -1,   101,    -1,   103,    -1,    -1,
     106,   107,   108,   109,    -1,    -1,   112,    -1,   114,    -1,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,    -1,    -1,
     107,   108,   109,   129,    -1,    -1,    -1,   114,    -1,    -1,
      -1,   118,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,
     146,    -1,   129,    -1,    -1,   151,   133,    -1,   154,   258,
      -1,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,   168,    -1,    -1,   171,    -1,   154,   174,    -1,
     176,    -1,    -1,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,    -1,    -1,   298,
     299,   300,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
     206,   207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2321,    -1,    -1,    -1,    -1,
     329,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   242,   243,   244,   245,
      -1,    -1,    -1,    -1,    -1,   251,   252,    -1,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,    -1,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     296,    -1,    -1,    -1,    -1,   301,    -1,   303,    -1,    -1,
      -1,    -1,    -1,    -1,   291,    -1,    -1,   294,   295,    -1,
     316,    -1,    -1,    -1,   301,    -1,   322,    -1,    -1,    -1,
      -1,   327,    -1,    -1,    -1,    -1,    -1,   333,   334,    -1,
     336,    -1,    -1,   339,   340,   341,   342,    -1,    -1,    -1,
     346,    -1,    -1,    -1,   350,   351,   352,    -1,    -1,    -1,
     356,   357,   339,   340,     1,   361,    -1,    -1,     5,   365,
       7,    -1,   368,   369,    -1,    -1,    -1,    -1,   374,    -1,
     376,   377,   378,    -1,    -1,   381,    -1,    24,    -1,    -1,
      27,    28,    29,    30,    31,    -1,    33,    -1,    -1,    -1,
      37,    38,    39,    -1,   381,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    65,    66,
      -1,    -1,    -1,    70,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    -1,   134,    -1,    -1,    -1,   138,   112,  1140,   114,
      97,    -1,   117,   118,   101,    -1,   103,    -1,   123,   106,
     107,   108,   109,   155,   129,   112,    -1,   114,    -1,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,   146,   129,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,   163,   146,
      -1,   166,   167,    -1,   151,    -1,   171,   154,    -1,   174,
      -1,   176,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,   168,    -1,    -1,   171,    -1,    -1,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,   203,    -1,
      -1,    -1,   207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,   206,
     207,    -1,    -1,    -1,    -1,    -1,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   242,   243,   244,   245,    -1,
      -1,    -1,    -1,    -1,   251,   252,    -1,    -1,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,    -1,    -1,   281,    -1,    -1,   329,    -1,    -1,
      -1,   333,   334,    -1,    -1,    -1,    -1,    -1,    -1,   296,
      -1,   343,    -1,    -1,   301,    -1,   303,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   316,
      -1,    -1,    -1,    -1,    -1,   322,    -1,   369,    -1,    -1,
     327,    -1,    -1,    -1,    -1,    -1,   333,   334,    -1,   336,
      -1,    -1,   339,   340,   341,   342,    -1,    -1,    -1,   346,
      -1,    -1,    -1,   350,   351,   352,    -1,    -1,    -1,   356,
     357,    -1,    -1,     1,   361,    -1,    -1,     5,   365,     7,
      -1,   368,   369,    -1,    -1,    -1,    -1,   374,    -1,   376,
     377,   378,    -1,    -1,   381,    -1,    24,    -1,    -1,    27,
      28,    29,    30,    31,    -1,    33,    -1,    -1,    -1,    37,
      38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,
      -1,    -1,    70,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
     112,    -1,   114,    -1,    -1,    -1,   118,    -1,    -1,    97,
      -1,   123,    -1,   101,    -1,   103,    -1,   129,   106,   107,
     108,   109,    -1,    -1,   112,    -1,   114,    -1,    -1,   117,
     118,    -1,    -1,    -1,   146,   123,    -1,    -1,    -1,   151,
      -1,   129,   154,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,    -1,   146,   171,
      -1,    -1,   174,   151,   176,    -1,   154,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
     168,    -1,    -1,   171,    -1,    -1,   174,    -1,   176,   201,
      -1,    -1,    -1,    -1,     1,   207,    -1,    -1,     5,    -1,
       7,    -1,    -1,     1,    -1,    -1,    -1,     5,    -1,     7,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,   206,   207,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   242,   243,   244,   245,    -1,    -1,
      -1,    -1,    -1,   251,   252,    -1,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,    -1,    -1,   281,    -1,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,   296,    -1,
      -1,    -1,   119,   301,    -1,   303,    -1,    -1,    -1,   117,
      -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,   316,   136,
     137,    -1,    -1,    -1,   322,    -1,    -1,    -1,    -1,   327,
      -1,    -1,    -1,    -1,    -1,   333,   334,   154,   336,    -1,
      -1,   339,   340,   341,   342,    -1,    -1,    -1,   346,    -1,
      -1,    -1,   350,   351,   352,   172,   173,    -1,   356,   357,
     168,    -1,     5,   361,     7,    -1,    -1,   365,    -1,    -1,
     368,   369,    -1,    -1,    -1,    -1,   374,    -1,   376,   377,
     378,    24,    -1,   381,    -1,   202,    -1,    -1,    31,   206,
      33,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
       5,    54,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    -1,    -1,    70,    -1,    -1,
      73,    -1,    27,    28,    29,    30,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,
     103,    -1,    -1,   106,   107,   108,   109,    -1,    -1,   112,
      -1,   114,    -1,   281,    -1,   118,    -1,    -1,    -1,    -1,
     123,    -1,    -1,    -1,    -1,    -1,   129,   304,   296,    -1,
      -1,    -1,    -1,   301,    -1,    -1,    -1,    -1,    93,    94,
      95,    -1,    97,   146,    -1,   100,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,   322,    -1,   159,    -1,    -1,   327,
     163,    -1,   117,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,   361,     5,    -1,     7,    -1,   201,   202,
     377,   204,   205,   206,   207,    -1,    -1,    -1,    -1,   377,
      -1,    -1,    -1,   168,    -1,    -1,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,   242,
     243,   244,   245,    54,    -1,    56,    -1,    58,   251,   252,
      -1,    -1,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    -1,    -1,   281,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   296,    -1,    -1,    -1,    -1,   301,    -1,
     303,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
     138,    -1,    -1,   316,    -1,    -1,    -1,    -1,    -1,   322,
      -1,    -1,    -1,    -1,   327,    -1,   281,   155,    -1,    -1,
     333,   334,    -1,   336,    -1,    -1,   339,   340,   341,   342,
      -1,   296,    -1,   346,    -1,    -1,   301,   350,   351,   352,
      -1,    -1,    -1,   356,   357,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   365,    -1,    -1,   368,   369,   322,    -1,    -1,
      -1,   374,   327,   376,   377,   378,    -1,    -1,   381,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     6,    -1,    -1,
      -1,    -1,    -1,   115,   116,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,   361,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    33,    -1,   138,    -1,    -1,    38,
      -1,    -1,   377,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
     281,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   296,    -1,   106,   107,   108,
     109,    -1,   303,   112,    -1,   114,    -1,    -1,    -1,   118,
      -1,   329,    -1,    -1,   123,   333,   334,    -1,    -1,    -1,
     129,   322,    -1,    -1,   133,   343,   327,    -1,   329,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,    -1,    -1,   154,   258,    -1,    -1,    -1,
     159,   369,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     361,    -1,   171,    -1,    -1,   174,    -1,   176,    -1,    -1,
     282,   283,   284,   285,   286,   287,   377,   289,   290,   291,
     292,   293,   294,   295,    -1,    -1,   298,   299,   300,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,   206,   207,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    -1,   329,    44,    -1,
      -1,    47,    48,    49,    50,    -1,    -1,    53,    54,    -1,
      56,    57,    58,   242,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
      -1,    -1,   281,     6,    -1,   101,    -1,   103,   104,   105,
      -1,    -1,   291,    -1,    -1,   294,   295,    -1,    -1,    -1,
      -1,    24,   301,    -1,   303,    -1,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    -1,    -1,   317,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   333,   334,    -1,    -1,    -1,    -1,
     339,   340,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,   352,    -1,    -1,    79,   356,   357,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,   366,   367,    -1,
     369,    -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,   378,
      -1,    -1,   381,   106,   107,   108,   109,    -1,    -1,   112,
      -1,   114,    -1,    -1,   146,   118,    -1,    -1,    -1,   151,
     123,    -1,   154,    -1,    -1,    -1,   129,   159,    -1,    -1,
     133,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,   171,
      -1,    -1,   174,   146,   176,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,    -1,    -1,   171,   201,
      -1,   174,    -1,   176,    -1,   207,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,   202,
     296,   204,   205,   206,   207,    -1,    -1,   303,    -1,    -1,
      -1,    -1,    -1,    -1,   310,   311,   312,   313,   314,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   322,    -1,    -1,    -1,
      -1,   327,    -1,   329,   330,    -1,    -1,    -1,    -1,   242,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    -1,    -1,   281,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   291,    -1,
      -1,   294,   295,    93,    -1,    -1,    -1,    24,   301,    -1,
     303,    -1,    -1,    -1,    31,    -1,    33,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,   115,   116,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     333,   334,    -1,    -1,    -1,    -1,   339,   340,   138,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,   352,
      -1,    -1,    79,   356,   357,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   366,   367,    -1,   369,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   378,    -1,    -1,   381,   106,
     107,   108,   109,    -1,    -1,   112,    -1,   114,    -1,    -1,
      -1,   118,    -1,    -1,    -1,   195,   123,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,   206,
     207,    -1,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,    -1,    -1,   298,   299,
     300,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   242,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   329,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,    -1,    -1,   281,     6,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   291,   186,    -1,   294,   295,    -1,
      -1,    -1,    -1,    24,   301,    -1,   303,    -1,    -1,    -1,
      31,    -1,    33,    -1,    -1,    -1,    -1,    38,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,    -1,   333,   334,    -1,    -1,
      -1,    -1,   339,   340,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,   352,    -1,    -1,    79,   356,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,
     367,    -1,   369,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   378,    -1,    -1,   381,   106,   107,   108,   109,    -1,
      -1,   112,    -1,   114,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,
     171,    -1,    -1,   174,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,   206,   207,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   242,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,    -1,    -1,
     281,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     291,    -1,    -1,   294,   295,    -1,    -1,    -1,    -1,    24,
     301,    -1,   303,    -1,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   333,   334,    -1,    -1,    -1,    -1,   339,   340,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,   352,    -1,    -1,    79,   356,   357,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   366,   367,    -1,   369,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   378,    -1,    -1,
     381,   106,   107,   108,   109,    -1,    -1,   112,    -1,   114,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,   123,    -1,
      -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,   206,   207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    -1,    -1,   281,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,   291,    -1,    -1,   294,
     295,    -1,    -1,    -1,    -1,    -1,   301,    -1,   303,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,    -1,    -1,    -1,   114,    -1,
      -1,    -1,   118,    -1,    -1,    -1,    -1,   123,   333,   334,
      22,    -1,    -1,   129,   339,   340,    -1,   133,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   352,    -1,    -1,
     146,   356,   357,    -1,    -1,   151,    -1,    -1,   154,    -1,
      -1,   366,   367,   159,   369,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   378,    -1,   171,   381,    -1,   174,    -1,
     176,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,   204,    -1,
      -1,   207,    -1,    -1,   106,   107,   108,   109,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,   123,    -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    22,    -1,    -1,   171,
      -1,    -1,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   291,    -1,    -1,   294,   295,
      -1,    -1,    -1,    -1,    -1,   301,    -1,    -1,    -1,   201,
      -1,    -1,   204,    -1,    -1,   207,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   339,   340,    -1,   342,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,    -1,    -1,    -1,   114,    -1,
     366,   367,   118,    -1,    -1,    -1,    -1,   123,    -1,    -1,
     376,    -1,   378,   129,    -1,   381,    -1,   133,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   291,
     146,    -1,   294,   295,    -1,   151,    -1,    -1,   154,   301,
      -1,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,    23,    24,   171,    -1,    -1,   174,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   339,   340,    -1,
     342,    -1,    -1,    -1,    -1,   201,    -1,    -1,   204,    -1,
      -1,   207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,   366,   367,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,   376,    -1,   378,    -1,    -1,   381,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
      -1,    -1,    -1,    -1,   114,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,   129,
      -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   291,   146,    -1,   294,   295,
      -1,   151,    -1,    -1,   154,   301,    -1,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,    -1,
      -1,   171,    -1,    -1,   174,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   339,   340,    -1,   342,    -1,    -1,    -1,
      -1,   201,    -1,    -1,   204,    -1,    -1,   207,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     366,   367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   378,    -1,    -1,   381,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    33,    -1,    -1,    -1,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      -1,   291,    -1,    70,   294,   295,    73,    -1,    -1,    -1,
      -1,   301,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,   103,    -1,    -1,   106,
     107,   108,   109,    -1,    -1,   112,    -1,   114,    -1,   339,
     340,   118,   342,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   366,   367,    -1,   146,
      -1,    -1,    -1,    -1,   151,    -1,   376,   154,   378,    -1,
      -1,   381,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,   206,
     207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   242,   243,   244,   245,    -1,
      -1,    -1,    -1,    -1,   251,   252,    -1,    -1,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   296,
      -1,    -1,    -1,    24,   301,    -1,   303,    -1,    -1,    -1,
      31,    -1,    33,    -1,    -1,    -1,    -1,    38,    -1,   316,
      -1,    -1,    -1,    -1,    -1,   322,    -1,    -1,    -1,    -1,
     327,    -1,    -1,    -1,    -1,    -1,   333,   334,    -1,   336,
      -1,    -1,   339,   340,   341,   342,    -1,    -1,    -1,   346,
      -1,    -1,    73,   350,   351,   352,    -1,    -1,    79,   356,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   365,    -1,
      -1,   368,   369,    -1,    -1,    -1,    -1,   374,    -1,   376,
      -1,   378,    -1,    -1,   381,   106,   107,   108,   109,    -1,
      -1,   112,    -1,   114,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,
     171,    -1,    -1,   174,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,   206,   207,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   242,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,    -1,    -1,
     281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     291,    -1,    -1,   294,   295,    -1,    -1,    -1,    -1,    24,
     301,    -1,   303,    -1,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   333,   334,    -1,    -1,    -1,    -1,   339,   340,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   352,    -1,    -1,    -1,   356,   357,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   366,   367,    -1,   369,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   378,    -1,    -1,
     381,   106,   107,   108,   109,    -1,    -1,   112,    -1,   114,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,   123,    -1,
      -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,   206,   207,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    33,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,   243,   244,
     245,    -1,    -1,    -1,    -1,    -1,   251,   252,    -1,    -1,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    -1,    -1,   281,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
      -1,   112,    -1,   114,    -1,    -1,    -1,   118,   303,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,   333,   334,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,   352,    -1,    -1,
     171,   356,   357,   174,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   368,   369,    -1,    -1,    -1,    -1,    -1,
      -1,   376,    -1,   378,    -1,    -1,   381,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,   206,   207,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,   242,   243,   244,   245,    -1,    -1,    -1,    -1,    -1,
     251,   252,    -1,    -1,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,    -1,    -1,
     281,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   303,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,    -1,    -1,    -1,    -1,   114,    -1,    -1,    -1,
     118,    -1,   333,   334,    -1,   123,    -1,    -1,    -1,    -1,
      -1,   129,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,
      -1,   352,    -1,    -1,    -1,   356,   357,    -1,   146,    -1,
      -1,    -1,    -1,   151,    -1,    -1,   154,   368,   369,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,   378,   166,   167,
     381,    -1,    24,   171,    -1,    -1,   174,    -1,   176,    31,
      -1,    33,    -1,    -1,    -1,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,   204,    -1,    -1,   207,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
     112,    -1,   114,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,   123,    -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   291,   146,    -1,   294,   295,    -1,   151,
      -1,    -1,   154,   301,    -1,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,   171,
      -1,    -1,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   339,   340,    -1,   342,    -1,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,   206,   207,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,   367,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   376,    -1,
     378,    -1,    -1,   381,    -1,    -1,    -1,    -1,    -1,    -1,
     242,   243,   244,   245,    -1,    -1,    -1,    -1,    -1,   251,
     252,    -1,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,    -1,    -1,   281,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,   296,    38,    -1,    -1,    -1,    -1,
      -1,   303,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     322,    -1,    -1,    -1,    -1,   327,    -1,   329,    -1,    -1,
      -1,   333,   334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   346,    -1,    -1,    -1,    -1,    -1,
     352,    -1,    -1,    -1,   356,   357,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   368,   369,    -1,   112,
      -1,   114,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,
     123,    -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,   206,   207,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,
     243,   244,   245,    -1,    -1,    -1,    -1,    -1,   251,   252,
      -1,    -1,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    -1,    -1,   281,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     303,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,     5,    -1,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     333,   334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,   352,
      -1,    -1,    39,   356,   357,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    51,   368,   369,    54,    55,    56,
      -1,    58,    -1,    -1,    -1,    -1,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,    -1,
     107,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   123,    -1,    -1,    -1,
     114,    -1,   129,    -1,   118,    -1,    -1,    -1,    -1,   123,
      -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    73,   159,    -1,    -1,    -1,   163,
      79,    -1,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,
     174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,    -1,    -1,    -1,    -1,   114,    -1,   201,    -1,   118,
     204,    -1,    -1,   207,   123,    -1,    -1,    -1,    -1,    -1,
     129,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,   174,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,   281,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,   296,
      -1,    -1,   201,    -1,   301,   204,   303,   291,   207,    -1,
     294,   295,    -1,    -1,    -1,    -1,    -1,   301,    -1,   106,
     107,   108,   109,    -1,    -1,   322,    -1,   114,    -1,    -1,
     327,   118,   329,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   339,   340,    -1,   342,   146,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   366,   367,   171,    -1,    -1,   174,    -1,   176,
      -1,    -1,   291,    -1,   378,   294,   295,   381,    -1,    -1,
      -1,    -1,   301,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   201,    -1,    -1,   204,    -1,    -1,
     207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     339,   340,    -1,   342,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,   367,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   378,
      -1,    -1,   381,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   291,    -1,    -1,   294,   295,    -1,
      -1,    -1,    -1,    -1,   301,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   339,   340,    -1,   342,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,
     367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   378,    -1,    -1,   381
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   385,     0,     1,     5,     7,    27,    28,    29,    30,
      39,    47,    49,    51,    54,    55,    56,    58,    63,    64,
      65,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,   113,   123,   129,   154,   281,
     296,   303,   322,   327,   329,   386,   443,   444,   445,   446,
     530,   531,   532,   534,   549,   386,   108,   107,   301,   526,
     526,   526,   532,   543,   532,   534,   549,   532,   537,   537,
     537,   532,   540,   446,    51,   447,    39,    47,    49,    54,
      55,    56,    58,   281,   296,   303,   322,   327,   329,   448,
      51,   449,    39,    47,    49,    51,    54,    55,    56,    58,
     281,   296,   303,   322,   327,   329,   454,    55,   456,    39,
      44,    47,    48,    49,    50,    53,    54,    56,    57,    58,
      60,   101,   103,   104,   105,   281,   296,   303,   310,   311,
     312,   313,   314,   322,   327,   329,   330,   459,    51,    52,
      54,    55,    56,   296,   310,   311,   327,   462,    47,    49,
      54,    56,    60,   101,   103,   463,    49,   464,    24,    31,
      33,    38,   106,   107,   108,   109,   112,   114,   118,   123,
     129,   146,   151,   154,   159,   163,   166,   167,   171,   174,
     176,   201,   202,   204,   205,   206,   207,   242,   243,   244,
     245,   251,   252,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   281,   303,   333,
     334,   352,   356,   357,   368,   369,   378,   381,   472,   527,
     664,   665,   668,   669,   670,   671,   675,   751,   754,   756,
     759,   762,   767,   768,   770,   772,   782,   784,   786,   788,
     790,   792,   796,   798,   800,   802,   804,   806,   808,   810,
     812,   814,   818,   820,   822,   824,   835,   843,   845,   847,
     848,   850,   852,   854,   856,   858,   860,   862,   864,    60,
     362,   363,   364,   465,   471,    60,   466,   471,    39,    47,
      49,    51,    54,    55,    56,    58,   281,   296,   303,   322,
     327,   329,   455,   107,   467,   468,   389,   408,   409,    93,
     288,   290,   543,   543,   543,   543,   543,     0,   386,   526,
     526,    59,   359,   360,   546,   547,   548,    37,    39,    54,
      65,    66,    70,    73,    82,    83,    84,    85,    86,    87,
     101,   103,   257,   281,   296,   301,   303,   316,   322,   327,
     336,   339,   340,   341,   342,   346,   350,   351,   365,   374,
     376,   553,   554,   555,   557,   558,   559,   560,   561,   562,
     563,   564,   565,   566,   567,   568,   569,   570,   574,   575,
     577,   578,   581,   582,   584,   585,   592,   593,   598,   599,
     608,   609,   612,   613,   614,   615,   616,   637,   638,   640,
     641,   643,   644,   647,   648,   649,   659,   660,   661,   662,
     663,   670,   677,   678,   679,   680,   681,   682,   686,   687,
     688,   735,   749,   754,   755,   780,   781,   782,   825,   386,
     375,   375,   386,   526,   620,   473,   478,   553,   526,   485,
     488,   664,   688,   491,   526,   499,   534,   550,   543,   532,
     534,   537,   537,   537,   540,    93,   288,   290,   543,   543,
     543,   543,   543,   549,   453,   532,   543,   544,   450,   530,
     532,   533,   451,   532,   534,   535,   550,   452,   532,   537,
     538,   537,   452,   532,   540,   541,    93,   288,   290,   712,
     453,   453,   453,   453,   453,   537,   543,   461,   531,   552,
     532,   552,   534,   552,    47,   457,   458,   531,   532,   552,
     537,   537,   458,   540,   552,    47,    48,   537,   552,   458,
      93,   288,   315,   712,   713,   543,   543,   458,   458,   458,
     458,   458,   543,   543,   543,   458,   415,   550,    49,   457,
     532,   534,   552,   537,   537,   537,   543,   458,   458,   543,
     427,   532,   534,   537,   537,   552,    47,   537,   534,   107,
     110,   111,   112,   785,   115,   116,   258,   259,   262,   673,
     674,    34,    35,    36,   258,   757,   136,   676,   172,   173,
     846,   115,   116,   117,   787,   117,   119,   120,   121,   122,
     789,   115,   116,   124,   125,   126,   127,   128,   791,   115,
     116,   119,   130,   131,   132,   133,   134,   135,   136,   138,
     139,   181,   793,   117,   119,   139,   147,   148,   149,   150,
     797,   117,   139,   152,   318,   799,   115,   116,   130,   132,
     133,   134,   156,   157,   158,   801,   116,   117,   119,   139,
     147,   148,   150,   160,   161,   162,   803,   131,   148,   157,
     164,   165,   805,   148,   165,   807,   157,   168,   169,   809,
     134,   139,   172,   173,   811,   139,   172,   173,   175,   813,
     139,   148,   164,   168,   172,   173,   177,   178,   179,   180,
     181,   826,   117,   172,   173,   181,   836,   168,   203,   786,
     788,   790,   792,   796,   798,   800,   802,   804,   806,   808,
     810,   812,   814,   815,   816,   817,   819,   835,   843,   845,
     130,   138,   168,   405,   823,   405,   117,   203,   817,   821,
     139,   172,   173,   208,   241,   844,   117,   129,   131,   150,
     154,   157,   246,   279,   280,   369,   769,   771,   851,   247,
     853,   247,   855,   168,   248,   249,   250,   857,   131,   157,
     849,   119,   135,   157,   164,   253,   254,   859,   131,   157,
     861,   117,   131,   139,   157,   164,   863,   107,   134,   138,
     155,   329,   343,   369,   752,   753,   754,   115,   116,   119,
     138,   258,   282,   283,   284,   285,   286,   287,   289,   290,
     291,   292,   293,   294,   295,   298,   299,   300,   329,   773,
     774,   777,    62,   119,   758,   343,   353,   761,   682,   687,
     354,   242,   251,   252,   255,   256,   865,   372,   373,   414,
     764,   681,   526,   433,   471,   363,   413,   471,   401,   453,
     450,   451,   534,   550,   452,   537,   537,   540,   541,   712,
     453,   453,   453,   453,   453,   396,   419,    48,    50,    52,
      53,    60,    61,    95,   469,   543,   543,   543,   393,   707,
     734,   722,   709,   711,   106,   106,   106,    88,   769,   297,
     660,   176,   526,   664,   750,   750,    65,   102,   526,   107,
     752,    93,   195,   288,   773,   774,   297,   297,   297,   323,
     297,    88,   168,    88,    88,   769,   107,     5,   387,   689,
     690,   361,   551,   565,   442,   478,   106,   325,   576,   392,
     393,   298,   299,   579,   580,   328,   583,   394,   439,   168,
     317,   318,   319,   320,   321,   586,   587,   325,   326,   594,
     601,   416,   325,   326,   595,   600,   424,   426,   337,   611,
     420,     6,    73,    79,    88,    90,   114,   118,   123,   129,
     133,   154,   242,   291,   294,   295,   301,   317,   339,   340,
     366,   367,   378,   623,   624,   625,   626,   627,   628,   629,
     631,   632,   633,   634,   635,   636,   665,   668,   671,   675,
     744,   745,   746,   751,   756,   759,   762,   768,   769,   770,
     772,   778,   779,   782,   434,   440,    40,    41,   191,   194,
     617,   618,   420,    88,   343,   344,   345,   639,   645,   646,
     420,    88,   642,   645,   398,   404,   425,   347,   348,   349,
     650,   651,   655,   656,    24,   664,   666,   667,    16,    17,
      18,    19,   380,     9,    25,    56,    10,    11,    12,    13,
      14,    15,    20,   114,   118,   123,   129,   146,   151,   154,
     159,   163,   166,   167,   171,   174,   176,   201,   204,   207,
     342,   378,   665,   667,   668,   683,   684,   685,   688,   736,
     737,   738,   739,   740,   741,   742,   743,   745,   746,   747,
     748,    54,    54,    23,   376,   705,   736,   737,   742,   705,
      40,   376,   619,   376,   376,   376,   376,   376,   546,   553,
     620,   473,   478,   485,   488,   491,   499,   543,   543,   543,
     393,   707,   734,   722,   709,   711,   553,   440,    59,    59,
      59,   478,    59,   488,    59,   499,   543,   393,   441,   416,
     424,   431,   488,   440,    45,   460,   532,   537,   552,   543,
      47,   393,   441,   416,   424,   431,   534,   488,   393,   424,
     537,   526,   436,     8,     9,    32,   117,   262,   263,   672,
     321,   432,   107,   130,   297,   436,   435,   400,   435,   410,
     114,   129,   114,   129,   389,   142,   143,   144,   145,   794,
     408,   435,   411,   435,   412,   409,   435,   411,   388,   399,
     391,   437,   438,    24,    40,   106,   179,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     827,   828,   829,   435,   106,   395,   433,   815,   405,   817,
     186,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   837,   842,
     429,   435,   408,   409,   414,   771,   428,   428,   382,   428,
     428,   382,   428,   407,   403,   397,   435,   418,   417,   431,
     417,   431,   115,   116,   129,   138,   154,   282,   283,   284,
     775,   776,   777,   392,   354,   354,   106,   428,   407,   403,
     397,   418,   371,   763,   379,   440,   488,   499,   543,   393,
     441,   416,   424,   431,   470,   471,   720,   720,   720,   298,
     376,   706,   376,   323,   376,   721,   376,   595,   708,   376,
     527,   710,     6,   129,   154,   635,    88,   635,   657,   658,
     682,   181,    24,    24,   100,   376,    54,    54,    54,    54,
     777,    54,   635,   635,   635,   635,   652,   653,   654,   664,
     668,   682,   686,   762,   768,   655,   635,   635,    88,   527,
      22,   688,   693,   694,   695,   703,   742,   743,     8,   377,
     527,   376,   106,   106,   106,   580,   107,   302,   528,    81,
     114,   129,   176,   265,   589,   527,   106,   106,   106,   527,
     588,   587,   106,   603,   605,   606,   106,   146,   159,   176,
     338,   635,   427,   396,     6,   635,    88,   400,   410,   389,
     408,   409,   394,    88,   420,   420,   628,   665,   746,    16,
      17,    18,    19,   380,    21,    23,     9,    56,     6,   645,
      88,    90,   247,   317,     8,     8,   106,   106,   618,     6,
       8,     6,   635,   653,   664,   668,   651,     8,   526,   376,
     747,   747,   738,   739,   740,   681,   376,   571,   666,   737,
     408,   411,   409,   411,   388,   399,   391,   437,   438,   433,
     395,   405,   429,   420,   742,     8,    21,    16,    17,    18,
      19,   380,     8,    21,    23,     9,   736,   737,   742,   635,
     635,   106,   377,   386,    21,   386,   106,   514,   440,   477,
     479,   487,   496,   500,   619,   376,   376,   376,   376,   376,
     720,   720,   720,   706,   376,   721,   708,   710,   106,   106,
     106,   376,   106,   106,   376,   720,   107,   392,   532,   106,
     674,   435,   402,   106,   422,   422,   400,   408,   400,   408,
     117,   134,   140,   141,   247,   408,   795,   390,   100,   833,
     195,   831,   200,   834,   198,   199,   832,   196,   197,   830,
     134,   395,   231,   235,   236,   237,   841,   226,   227,   228,
     229,   839,   230,   231,   232,   233,   234,   840,   840,   235,
     238,   238,   239,   240,   239,   117,   134,   168,   838,   430,
     428,   106,   106,   115,   116,   115,   116,   392,   392,   106,
     106,   355,   760,   106,   165,   370,   765,   769,   376,   720,
     376,   376,   376,   106,   507,   393,   733,   601,   512,   416,
     508,   106,   424,   513,   431,   635,     6,     6,   635,   440,
     666,    93,    96,   551,   696,   698,    40,   179,   184,   194,
     828,   829,   527,   527,   106,   682,   691,   692,   635,   635,
     635,   635,    54,   635,   393,   441,   416,   424,    23,   425,
      88,   347,    45,   635,   387,     6,   387,   281,   296,   322,
     327,   575,   577,   582,   593,   599,   700,   701,    93,    96,
     551,   699,   702,   387,   690,   480,   400,   153,   148,   153,
     590,   591,    21,   107,   299,   324,   602,   324,   604,    21,
     107,   299,   596,   107,   117,   610,   668,   117,   610,   433,
     117,   610,   635,     6,   635,   635,   379,   623,   623,   624,
     625,   626,   106,   628,   623,   630,   666,   688,   635,   635,
      88,     9,    88,   665,   746,   778,   778,   635,   646,   635,
     645,   656,   390,   657,   387,   572,   573,   379,   742,   736,
     742,   747,   747,   738,   739,   740,   742,   106,   736,   742,
     685,   742,    21,    21,   106,    41,   386,   377,   386,   443,
     551,   619,    39,    49,    54,    56,    58,   168,   281,   296,
     303,   322,   327,   329,   377,   386,   443,   474,   551,    46,
      97,   117,   168,   377,   386,   443,   516,   522,   523,   551,
     553,    42,    92,    93,    94,    95,    97,   100,   117,   168,
     377,   386,   443,   486,   497,   551,   556,   564,    42,    93,
      94,    95,   117,   168,   377,   386,   443,   497,   551,   556,
      43,    46,   168,   296,   377,   386,   443,   440,   477,   479,
     487,   496,   500,   376,   376,   376,   393,   733,   416,   424,
     431,   479,   500,   392,   392,     8,   432,   435,   408,   829,
     435,   429,   383,   383,   408,   408,   409,   409,   760,   358,
     760,   106,   406,   414,   115,   116,   766,   500,   392,   510,
     511,   509,   299,   377,   386,   443,   551,   706,   119,   129,
     136,   137,   154,   172,   173,   202,   206,   304,   377,   386,
     443,   551,   732,   603,   605,   377,   386,   443,   551,   721,
     377,   386,   443,   551,   708,   596,   377,   386,   443,   551,
     710,   635,   635,     6,   528,   528,   698,   433,   390,   390,
     376,   565,   696,   417,   417,   392,   392,   635,   392,   652,
     664,   668,   654,   653,   635,    45,    85,    86,   704,   743,
     749,   392,   393,   394,   416,   424,   701,   107,   697,   528,
     700,   702,   377,   386,   553,   408,     8,   433,   331,   332,
     333,   334,   335,   607,   106,   106,   606,   607,   328,   597,
     610,   610,    73,   610,   635,     6,   635,   170,   635,   645,
     645,     6,   377,   556,   693,     8,   377,   736,   736,   106,
      41,   440,   526,   545,   526,   536,   526,   539,   539,   526,
     542,   107,   475,   476,    93,   288,   290,   545,   545,   545,
     545,   545,   386,   375,    82,    83,   524,   525,   664,   435,
     102,   386,   386,   386,   386,   386,   483,   484,   669,   528,
     528,   375,    98,    99,   498,   106,   107,   132,   133,   258,
     278,   279,   504,   505,   515,    89,    90,    91,    93,   489,
     490,   386,   386,   386,   386,   564,   484,   528,   528,   375,
     505,   489,   386,   386,   386,   107,   375,   102,   393,   377,
     377,   377,   377,   377,   510,   511,   509,   377,   377,   377,
     106,   783,     8,   421,   106,   406,   414,   377,    97,   138,
     282,   377,   386,   443,   551,   718,    93,   100,   138,   173,
     282,   377,   386,   443,   551,   719,   117,   282,   377,   386,
     443,   551,   715,   106,   393,   106,   115,   116,   106,   106,
     115,   116,   106,   106,   730,   727,   724,   441,   386,   602,
     604,   416,   424,   597,   431,   635,   691,   377,   392,   425,
     425,   635,   387,   386,   591,   298,   107,   106,   435,   435,
     427,   435,   635,    88,   657,     6,   377,     6,   387,   573,
     194,   621,   106,   506,   478,   485,   491,   499,     8,   545,
     545,   545,   506,   506,   506,   506,   506,   376,   501,   664,
     423,   107,     9,   386,   386,   488,   423,     9,   435,     8,
     386,     6,   386,   386,   488,     6,   386,   155,   517,   501,
     386,   377,   377,   377,   441,   390,   106,   760,   375,   171,
     176,   714,   531,   392,   528,   106,   714,   106,   531,   392,
     108,   531,   392,   665,   665,   665,   665,   376,   376,   376,
     565,   596,   411,   411,   635,   377,   657,   749,   191,   622,
     386,   376,   376,   376,   376,   376,   476,   506,   506,   506,
     376,   376,   376,   376,   376,   502,   503,   527,   664,   386,
      43,   483,   504,   490,    39,    90,   107,   281,   296,   322,
     327,   329,   346,   481,   482,   484,   493,   494,   669,    90,
     493,   495,    23,   106,   107,   373,   518,   519,   520,   664,
     386,   386,   392,   392,   392,     8,   406,   376,   437,   433,
     386,   386,   386,   386,   386,   386,   386,   138,   386,   408,
     408,   409,   409,   729,   726,   723,   377,   377,   387,   621,
     514,   479,   487,   496,   500,   376,   376,   376,   507,   733,
     512,   508,   513,     8,   377,   107,   440,   484,    93,   288,
     492,   393,   416,   424,   431,   386,     9,   386,   386,   505,
     106,    23,    26,   783,   107,   716,   717,   714,   307,   377,
     386,   443,   731,   308,   377,   386,   443,   728,   119,   130,
     286,   360,   377,   386,   443,   725,   622,   377,   377,   377,
     377,   377,   510,   511,   509,   377,   377,   377,   377,   377,
     387,   503,    45,    46,   521,   392,   669,   435,   386,   106,
     106,     6,     8,   377,   386,   106,   386,   106,   386,   106,
     106,   305,   306,   106,   386,   386,   386,   386,   386,   386,
     377,   377,   377,   386,   386,   386,   386,   386,   527,   664,
     375,   517,   435,   106,   528,   529,   717,   386,   435,   309,
     440,   386,   386,   386,   393,   441,   416,   424,   431,   501,
     423,   106,   392,   392,   392,   209,   527
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   384,   385,   385,   386,   386,   387,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   443,   443,   443,   443,   444,   444,
     444,   444,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   446,   446,
     446,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   446,   447,   448,   448,   448,   448,   448,   448,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   448,   449,   450,   450,   451,   451,   452,   452,   453,
     453,   454,   454,   454,   454,   454,   454,   454,   454,   454,
     454,   454,   454,   454,   454,   454,   454,   454,   455,   455,
     455,   455,   455,   455,   455,   455,   455,   455,   455,   455,
     455,   455,   455,   455,   456,   457,   457,   458,   458,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   460,
     461,   461,   462,   462,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   462,   463,   463,   463,   463,   463,   463,
     463,   464,   465,   465,   466,   466,   467,   468,   468,   469,
     469,   469,   469,   469,   469,   469,   469,   470,   470,   471,
     471,   471,   472,   473,   474,   474,   475,   475,   476,   477,
     477,   477,   477,   477,   477,   477,   477,   477,   477,   477,
     477,   477,   477,   477,   477,   477,   478,   479,   479,   479,
     479,   479,   479,   479,   479,   479,   480,   480,   480,   481,
     481,   482,   482,   482,   482,   483,   484,   484,   485,   486,
     486,   487,   487,   487,   487,   487,   487,   487,   487,   487,
     487,   487,   487,   488,   488,   489,   489,   490,   490,   490,
     490,   491,   492,   492,   493,   493,   493,   493,   493,   494,
     494,   495,   495,   496,   496,   496,   496,   496,   496,   496,
     496,   496,   496,   496,   496,   496,   496,   497,   497,   498,
     498,   499,   500,   500,   500,   500,   500,   500,   500,   501,
     501,   502,   502,   502,   503,   503,   504,   504,   505,   505,
     506,   507,   507,   507,   507,   507,   508,   508,   508,   508,
     508,   509,   509,   509,   509,   509,   510,   510,   510,   510,
     510,   511,   511,   511,   511,   511,   512,   512,   512,   512,
     512,   513,   513,   513,   513,   513,   514,   514,   514,   514,
     514,   515,   515,   515,   515,   515,   516,   517,   518,   518,
     519,   519,   519,   519,   519,   520,   520,   521,   521,   521,
     521,   522,   523,   524,   524,   525,   525,   526,   526,   527,
     527,   527,   528,   529,   529,   530,   530,   531,   531,   531,
     531,   531,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   549,   549,   549,   550,   551,   552,   552,   553,   553,
     554,   555,   555,   556,   556,   557,   558,   559,   560,   561,
     562,   562,   563,   563,   563,   563,   563,   563,   564,   564,
     564,   564,   564,   565,   565,   565,   565,   565,   565,   565,
     565,   565,   565,   565,   565,   565,   565,   565,   565,   565,
     565,   565,   565,   565,   565,   565,   566,   567,   567,   568,
     569,   569,   570,   571,   571,   572,   572,   572,   573,   574,
     575,   576,   576,   577,   577,   578,   579,   579,   580,   580,
     581,   582,   582,   583,   583,   584,   584,   585,   586,   586,
     587,   587,   587,   587,   587,   587,   588,   589,   589,   589,
     589,   589,   590,   590,   591,   591,   592,   593,   594,   594,
     595,   595,   595,   596,   596,   597,   597,   598,   599,   600,
     601,   601,   601,   602,   602,   603,   604,   604,   605,   605,
     606,   606,   607,   607,   607,   607,   607,   608,   609,   610,
     610,   611,   611,   611,   611,   611,   611,   611,   611,   612,
     613,   613,   614,   614,   614,   614,   614,   614,   615,   615,
     616,   617,   617,   618,   618,   618,   618,   619,   619,   620,
     621,   621,   622,   622,   623,   623,   623,   623,   623,   623,
     623,   623,   623,   623,   623,   623,   623,   623,   623,   624,
     624,   624,   625,   625,   626,   626,   627,   627,   628,   629,
     629,   630,   630,   631,   631,   632,   633,   634,   634,   635,
     635,   635,   636,   636,   636,   636,   636,   636,   636,   636,
     636,   636,   636,   636,   636,   636,   637,   637,   638,   639,
     639,   639,   640,   640,   641,   642,   642,   642,   642,   642,
     643,   643,   644,   644,   645,   645,   646,   646,   646,   647,
     647,   647,   647,   648,   648,   649,   650,   650,   651,   651,
     652,   652,   653,   653,   653,   654,   654,   654,   654,   655,
     655,   656,   656,   657,   657,   658,   659,   659,   659,   660,
     660,   660,   661,   661,   662,   662,   663,   664,   665,   665,
     666,   666,   667,   668,   669,   669,   669,   669,   669,   669,
     669,   669,   669,   669,   669,   669,   669,   670,   670,   670,
     670,   671,   672,   672,   672,   672,   673,   673,   673,   673,
     673,   674,   674,   675,   675,   676,   676,   677,   677,   677,
     678,   678,   679,   679,   680,   680,   681,   682,   682,   683,
     684,   685,   685,   686,   687,   687,   687,   688,   689,   689,
     689,   690,   690,   690,   691,   691,   692,   693,   693,   693,
     694,   694,   695,   695,   696,   696,   697,   698,   698,   698,
     699,   699,   700,   700,   701,   701,   701,   701,   701,   702,
     702,   702,   703,   704,   704,   705,   705,   705,   705,   706,
     707,   708,   709,   710,   711,   712,   712,   712,   713,   713,
     713,   714,   714,   715,   715,   716,   716,   717,   718,   718,
     718,   719,   719,   719,   719,   719,   720,   721,   721,   722,
     723,   723,   723,   723,   724,   725,   725,   725,   725,   725,
     726,   726,   726,   726,   727,   728,   729,   729,   729,   729,
     730,   731,   732,   732,   732,   732,   732,   732,   732,   732,
     732,   732,   732,   732,   733,   733,   733,   733,   733,   734,
     735,   735,   735,   735,   735,   735,   735,   735,   736,   736,
     737,   737,   737,   738,   738,   738,   739,   739,   740,   740,
     741,   741,   742,   743,   743,   743,   743,   744,   744,   745,
     746,   746,   746,   746,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   748,   748,   748,   748,   748,   748,   748,   749,
     749,   749,   749,   749,   749,   750,   750,   751,   751,   751,
     752,   752,   753,   753,   753,   753,   753,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   755,   755,   755,   755,   755,   755,   756,   756,   757,
     757,   757,   758,   758,   759,   760,   760,   761,   761,   762,
     763,   763,   764,   764,   765,   765,   766,   766,   767,   767,
     768,   768,   768,   769,   769,   770,   770,   771,   771,   771,
     771,   772,   772,   772,   773,   773,   774,   774,   774,   774,
     774,   774,   774,   774,   774,   774,   774,   774,   774,   774,
     774,   774,   774,   775,   775,   775,   775,   775,   775,   775,
     776,   776,   776,   776,   777,   777,   777,   777,   778,   778,
     779,   779,   780,   780,   781,   782,   782,   782,   782,   782,
     782,   782,   782,   782,   782,   782,   782,   782,   782,   782,
     782,   782,   782,   782,   782,   782,   782,   783,   784,   785,
     785,   785,   785,   786,   787,   787,   787,   788,   789,   789,
     789,   789,   789,   790,   791,   791,   791,   791,   791,   791,
     791,   791,   791,   792,   792,   792,   793,   793,   793,   793,
     793,   793,   793,   793,   793,   793,   793,   793,   794,   794,
     794,   794,   795,   795,   795,   795,   795,   796,   797,   797,
     797,   797,   797,   797,   797,   798,   799,   799,   799,   799,
     800,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     802,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   804,   805,   805,   805,   805,   805,   806,   807,   807,
     808,   809,   809,   809,   810,   811,   811,   811,   811,   812,
     813,   813,   813,   813,   814,   814,   814,   814,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   816,   816,   816,   817,   817,   818,   818,   819,
     819,   820,   820,   821,   821,   822,   822,   823,   823,   823,
     824,   825,   826,   826,   826,   826,   826,   826,   826,   826,
     826,   826,   827,   827,   827,   827,   827,   827,   828,   828,
     828,   828,   828,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   830,   830,   831,   832,   832,
     833,   834,   835,   835,   836,   836,   836,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   838,   838,   838,   839,   839,
     839,   839,   840,   840,   840,   840,   840,   841,   841,   841,
     841,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   843,   843,   844,   844,   844,   844,   845,
     846,   846,   847,   847,   847,   847,   847,   847,   847,   847,
     848,   849,   849,   850,   851,   851,   851,   851,   852,   853,
     854,   855,   856,   857,   857,   857,   857,   858,   859,   859,
     859,   859,   859,   859,   860,   861,   861,   862,   863,   863,
     863,   863,   863,   864,   865,   865,   865,   865,   865
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
       0,     0,     0,     3,     5,     5,     3,     2,     1,     1,
       2,     2,     1,     2,     2,     2,     2,     2,     2,     3,
       3,     2,     2,     3,     3,     3,     2,     3,     2,     6,
       2,     6,     3,     2,     6,     6,     3,     6,     3,     5,
       7,     5,     7,     8,     8,     8,     5,     7,     5,     7,
       5,     7,     7,     3,     2,     6,     2,     6,     6,     6,
       3,     6,     3,     5,     5,     8,     8,     8,     5,     5,
       5,     7,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     6,     2,     2,     2,     3,     2,     2,
       6,     3,     3,     5,     3,     3,     3,     3,     2,     2,
       2,     2,     2,     3,     2,     2,     6,     3,     3,     5,
       3,     3,     3,     3,     3,     2,     1,     1,     1,     2,
       2,     2,     2,     2,     2,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     3,     2,     2,
       2,     2,     2,     2,     4,     5,     2,     2,     3,     2,
       1,     2,     2,     3,     2,     3,     2,     2,     2,     3,
       2,     3,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     3,     2,     1,     2,     1,     3,     0,     1,     0,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     2,     1,     0,     2,     1,     1,     3,     1,     0,
       2,     2,     3,     8,     8,     8,     8,     9,     9,    10,
      10,    10,     9,     9,     9,     9,     0,     0,     2,     2,
       3,     3,     3,     3,     5,     3,     0,     2,     3,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     0,     2,
       3,     0,     2,     2,     3,     4,     4,     4,     3,     4,
       2,     3,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     0,     1,     1,     2,     2,     2,     2,     2,     1,
       3,     1,     0,     0,     2,     2,     4,     4,     8,     6,
       7,     6,     4,     3,     4,     3,     3,     3,     2,     1,
       1,     0,     0,     2,     2,     5,     5,     3,     4,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     3,
       0,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     7,     2,     1,     1,
       1,     1,     1,     3,     3,     1,     2,     2,     2,     3,
       0,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     2,     2,     1,
       2,     1,     1,     2,     3,     2,     3,     1,     2,     3,
       1,     2,     3,     1,     2,     3,     1,     2,     2,     2,
       1,     2,     2,     2,     2,     2,     0,     1,     1,     2,
       1,     1,     2,     1,     2,     4,     4,     4,     4,     4,
       5,     5,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     1,     1,     2,     2,     2,     2,     1,
       1,     2,     1,     1,     2,     1,     3,     1,     1,     5,
       1,     1,     3,     3,     1,     1,     3,     3,     5,     2,
       2,     1,     2,     1,     2,     1,     1,     2,     2,     2,
       1,     1,     2,     2,     2,     1,     2,     1,     1,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     3,     1,
       2,     1,     3,     1,     1,     1,     2,     2,     3,     3,
       1,     1,     0,     1,     1,     0,     3,     1,     2,     4,
       1,     1,     0,     0,     3,     3,     0,     2,     2,     3,
       2,     2,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     0,     6,     3,     6,     3,     5,     3,     5,     2,
       1,     1,     3,     4,     4,     5,     6,     5,     1,     2,
       1,     1,     2,     2,     2,     1,     1,     6,     8,     0,
       0,     1,     0,     1,     1,     1,     1,     1,     1,     1,
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     4,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     3,     1,     4,     4,     0,     2,     1,     3,     3,
       1,     3,     1,     3,     1,     3,     1,     1,     3,     3,
       3,     1,     1,     3,     1,     1,     1,     3,     1,     3,
       3,     3,     3,     5,     1,     2,     1,     1,     2,     3,
       1,     1,     2,     1,     1,     2,     1,     2,     2,     1,
       1,     2,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       0,     4,     0,     1,     0,     1,     1,     1,     1,     1,
       1,     2,     2,     6,     3,     1,     3,     3,     3,     7,
       3,     3,     3,     3,     3,     3,     0,     4,     4,     0,
       0,     2,     2,     3,     0,     2,     2,     2,     2,     2,
       0,     2,     2,     3,     0,     6,     0,     2,     2,     3,
       0,     2,     2,     4,     4,     4,     4,     2,     2,     2,
       2,     5,     5,     5,     0,     2,     2,     3,     2,     0,
       2,     2,     4,     4,     5,     5,     3,     3,     3,     3,
       1,     1,     1,     1,     3,     3,     1,     3,     1,     3,
       1,     3,     1,     1,     1,     3,     3,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     2,
       1,     1,     1,     2,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     1,     2,     2,     2,     2,     2,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     5,     3,     5,     1,     5,     5,     3,     5,     1,
       1,     1,     1,     1,     2,     0,     2,     1,     1,     6,
       2,     0,     1,     1,     1,     1,     1,     1,     5,     6,
       8,     6,     5,     2,     2,     3,     4,     1,     1,     1,
       2,     3,     4,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       3,     3,     5,     6,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     7,     1,
       1,     2,     1,     3,     1,     1,     2,     3,     1,     1,
       1,     1,     2,     3,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     5,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     3,     1,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     2,     3,     3,     9,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     1,
       1,     2,     2,     1,     1,     3,     3,     1,     1,     1,
       3,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     4,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     5,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     3,     1,     1,     2,     1,     3,     4,
       3,     1,     3,     1,     1,     1,     4,     3,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     3,     1,     1,
       2,     1,     1,     2,     2,     2,     2,     2,     2
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
#line 395 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6553 "src/parser_bison.c"
        break;

    case YYSYMBOL_QUOTED_STRING: /* "quoted string"  */
#line 395 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6559 "src/parser_bison.c"
        break;

    case YYSYMBOL_ASTERISK_STRING: /* "string with a trailing asterisk"  */
#line 395 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6565 "src/parser_bison.c"
        break;

    case YYSYMBOL_line: /* line  */
#line 729 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6571 "src/parser_bison.c"
        break;

    case YYSYMBOL_base_cmd: /* base_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6577 "src/parser_bison.c"
        break;

    case YYSYMBOL_add_cmd: /* add_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6583 "src/parser_bison.c"
        break;

    case YYSYMBOL_replace_cmd: /* replace_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6589 "src/parser_bison.c"
        break;

    case YYSYMBOL_create_cmd: /* create_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6595 "src/parser_bison.c"
        break;

    case YYSYMBOL_insert_cmd: /* insert_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6601 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_or_id_spec: /* table_or_id_spec  */
#line 735 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6607 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_or_id_spec: /* chain_or_id_spec  */
#line 737 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6613 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_or_id_spec: /* set_or_id_spec  */
#line 742 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6619 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_or_id_spec: /* obj_or_id_spec  */
#line 744 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6625 "src/parser_bison.c"
        break;

    case YYSYMBOL_delete_cmd: /* delete_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6631 "src/parser_bison.c"
        break;

    case YYSYMBOL_destroy_cmd: /* destroy_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6637 "src/parser_bison.c"
        break;

    case YYSYMBOL_get_cmd: /* get_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6643 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_table: /* list_cmd_spec_table  */
#line 753 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6649 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_any: /* list_cmd_spec_any  */
#line 753 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6655 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd: /* list_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6661 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_device_name: /* basehook_device_name  */
#line 761 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6667 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_spec: /* basehook_spec  */
#line 750 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6673 "src/parser_bison.c"
        break;

    case YYSYMBOL_reset_cmd: /* reset_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6679 "src/parser_bison.c"
        break;

    case YYSYMBOL_flush_cmd: /* flush_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6685 "src/parser_bison.c"
        break;

    case YYSYMBOL_rename_cmd: /* rename_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6691 "src/parser_bison.c"
        break;

    case YYSYMBOL_import_cmd: /* import_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6697 "src/parser_bison.c"
        break;

    case YYSYMBOL_export_cmd: /* export_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6703 "src/parser_bison.c"
        break;

    case YYSYMBOL_monitor_cmd: /* monitor_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6709 "src/parser_bison.c"
        break;

    case YYSYMBOL_monitor_event: /* monitor_event  */
#line 989 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6715 "src/parser_bison.c"
        break;

    case YYSYMBOL_describe_cmd: /* describe_cmd  */
#line 732 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6721 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_block_alloc: /* table_block_alloc  */
#line 767 "src/parser_bison.y"
            { close_scope(state); table_free(((*yyvaluep).table)); }
#line 6727 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_block_alloc: /* chain_block_alloc  */
#line 769 "src/parser_bison.y"
            { close_scope(state); chain_free(((*yyvaluep).chain)); }
#line 6733 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_verdict_expr: /* typeof_verdict_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6739 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_data_expr: /* typeof_data_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6745 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_typeof_expr: /* primary_typeof_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6751 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_expr: /* typeof_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6757 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_alloc: /* set_block_alloc  */
#line 780 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6763 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_key_expr: /* typeof_key_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6769 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_expr: /* set_block_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6775 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_block_alloc: /* map_block_alloc  */
#line 783 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6781 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_block_alloc: /* flowtable_block_alloc  */
#line 787 "src/parser_bison.y"
            { flowtable_free(((*yyvaluep).flowtable)); }
#line 6787 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr: /* flowtable_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6793 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_list_expr: /* flowtable_list_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6799 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr_member: /* flowtable_expr_member  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6805 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_atom_expr: /* data_type_atom_expr  */
#line 726 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6811 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_expr: /* data_type_expr  */
#line 726 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6817 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_block_alloc: /* obj_block_alloc  */
#line 790 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 6823 "src/parser_bison.c"
        break;

    case YYSYMBOL_type_identifier: /* type_identifier  */
#line 721 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6829 "src/parser_bison.c"
        break;

    case YYSYMBOL_prio_spec: /* prio_spec  */
#line 758 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6835 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_name: /* extended_prio_name  */
#line 761 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6841 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_spec: /* extended_prio_spec  */
#line 758 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6847 "src/parser_bison.c"
        break;

    case YYSYMBOL_dev_spec: /* dev_spec  */
#line 764 "src/parser_bison.y"
            { free(((*yyvaluep).expr)); }
#line 6853 "src/parser_bison.c"
        break;

    case YYSYMBOL_policy_expr: /* policy_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6859 "src/parser_bison.c"
        break;

    case YYSYMBOL_identifier: /* identifier  */
#line 721 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6865 "src/parser_bison.c"
        break;

    case YYSYMBOL_string: /* string  */
#line 721 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6871 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_spec: /* table_spec  */
#line 735 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6877 "src/parser_bison.c"
        break;

    case YYSYMBOL_tableid_spec: /* tableid_spec  */
#line 735 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6883 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_spec: /* chain_spec  */
#line 737 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6889 "src/parser_bison.c"
        break;

    case YYSYMBOL_chainid_spec: /* chainid_spec  */
#line 737 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6895 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_identifier: /* chain_identifier  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6901 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_spec: /* set_spec  */
#line 742 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6907 "src/parser_bison.c"
        break;

    case YYSYMBOL_setid_spec: /* setid_spec  */
#line 742 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6913 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_identifier: /* set_identifier  */
#line 747 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6919 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_spec: /* flowtable_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6925 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtableid_spec: /* flowtableid_spec  */
#line 747 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6931 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_identifier: /* flowtable_identifier  */
#line 747 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6937 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_spec: /* obj_spec  */
#line 744 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6943 "src/parser_bison.c"
        break;

    case YYSYMBOL_objid_spec: /* objid_spec  */
#line 744 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6949 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_identifier: /* obj_identifier  */
#line 747 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6955 "src/parser_bison.c"
        break;

    case YYSYMBOL_handle_spec: /* handle_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6961 "src/parser_bison.c"
        break;

    case YYSYMBOL_position_spec: /* position_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6967 "src/parser_bison.c"
        break;

    case YYSYMBOL_index_spec: /* index_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6973 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule_position: /* rule_position  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6979 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleid_spec: /* ruleid_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6985 "src/parser_bison.c"
        break;

    case YYSYMBOL_comment_spec: /* comment_spec  */
#line 721 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6991 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleset_spec: /* ruleset_spec  */
#line 740 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6997 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule: /* rule  */
#line 771 "src/parser_bison.y"
            { rule_free(((*yyvaluep).rule)); }
#line 7003 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_list: /* stmt_list  */
#line 793 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7009 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt_list: /* stateful_stmt_list  */
#line 793 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7015 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_counter: /* objref_stmt_counter  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7021 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_limit: /* objref_stmt_limit  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7027 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_quota: /* objref_stmt_quota  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7033 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_synproxy: /* objref_stmt_synproxy  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7039 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_tunnel: /* objref_stmt_tunnel  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7045 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_ct: /* objref_stmt_ct  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7051 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt: /* objref_stmt  */
#line 801 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7057 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt: /* stateful_stmt  */
#line 797 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7063 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 795 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7069 "src/parser_bison.c"
        break;

    case YYSYMBOL_xt_stmt: /* xt_stmt  */
#line 1013 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7075 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_stmt: /* chain_stmt  */
#line 825 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7081 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_stmt: /* verdict_stmt  */
#line 795 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7087 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_stmt: /* verdict_map_stmt  */
#line 883 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7093 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_expr: /* verdict_map_expr  */
#line 886 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7099 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_expr: /* verdict_map_list_expr  */
#line 886 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7105 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_member_expr: /* verdict_map_list_member_expr  */
#line 886 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7111 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_limit_stmt_alloc: /* ct_limit_stmt_alloc  */
#line 799 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7117 "src/parser_bison.c"
        break;

    case YYSYMBOL_connlimit_stmt: /* connlimit_stmt  */
#line 813 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7123 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt: /* counter_stmt  */
#line 797 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7129 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt_alloc: /* counter_stmt_alloc  */
#line 797 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7135 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt_alloc: /* last_stmt_alloc  */
#line 799 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7141 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt: /* last_stmt  */
#line 797 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7147 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt: /* log_stmt  */
#line 810 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7153 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt_alloc: /* log_stmt_alloc  */
#line 810 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7159 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt_alloc: /* limit_stmt_alloc  */
#line 799 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7165 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt: /* limit_stmt  */
#line 813 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7171 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_unit: /* quota_unit  */
#line 761 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 7177 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt_alloc: /* quota_stmt_alloc  */
#line 799 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7183 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt: /* quota_stmt  */
#line 813 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7189 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt: /* reject_stmt  */
#line 816 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7195 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt_alloc: /* reject_stmt_alloc  */
#line 816 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7201 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_with_expr: /* reject_with_expr  */
#line 831 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7207 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt: /* nat_stmt  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7213 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt_alloc: /* nat_stmt_alloc  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7219 "src/parser_bison.c"
        break;

    case YYSYMBOL_tproxy_stmt: /* tproxy_stmt  */
#line 821 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7225 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt: /* synproxy_stmt  */
#line 823 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7231 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt_alloc: /* synproxy_stmt_alloc  */
#line 823 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7237 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_obj: /* synproxy_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7243 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_stmt_expr: /* primary_stmt_expr  */
#line 870 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7249 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_stmt_expr: /* shift_stmt_expr  */
#line 872 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7255 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_stmt_expr: /* and_stmt_expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7261 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_stmt_expr: /* exclusive_or_stmt_expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7267 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_stmt_expr: /* inclusive_or_stmt_expr  */
#line 874 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7273 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_stmt_expr: /* basic_stmt_expr  */
#line 870 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7279 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_stmt_expr: /* concat_stmt_expr  */
#line 862 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7285 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr_set: /* map_stmt_expr_set  */
#line 862 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7291 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr: /* map_stmt_expr  */
#line 862 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7297 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_stmt_expr: /* prefix_stmt_expr  */
#line 867 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7303 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_stmt_expr: /* range_stmt_expr  */
#line 867 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7309 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_stmt_expr: /* multiton_stmt_expr  */
#line 865 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7315 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_expr: /* stmt_expr  */
#line 862 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7321 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt: /* masq_stmt  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7327 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt_alloc: /* masq_stmt_alloc  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7333 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt: /* redir_stmt  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7339 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt_alloc: /* redir_stmt_alloc  */
#line 818 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7345 "src/parser_bison.c"
        break;

    case YYSYMBOL_dup_stmt: /* dup_stmt  */
#line 834 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7351 "src/parser_bison.c"
        break;

    case YYSYMBOL_fwd_stmt: /* fwd_stmt  */
#line 836 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7357 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt: /* queue_stmt  */
#line 829 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7363 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_compat: /* queue_stmt_compat  */
#line 829 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7369 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_alloc: /* queue_stmt_alloc  */
#line 829 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7375 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_expr: /* queue_expr  */
#line 831 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7381 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr_simple: /* queue_stmt_expr_simple  */
#line 831 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7387 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr: /* queue_stmt_expr  */
#line 831 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7393 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt: /* set_elem_expr_stmt  */
#line 893 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7399 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt_alloc: /* set_elem_expr_stmt_alloc  */
#line 893 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7405 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_stmt: /* set_stmt  */
#line 838 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7411 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt: /* map_stmt  */
#line 841 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7417 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_stmt: /* meter_stmt  */
#line 843 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7423 "src/parser_bison.c"
        break;

    case YYSYMBOL_match_stmt: /* match_stmt  */
#line 795 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7429 "src/parser_bison.c"
        break;

    case YYSYMBOL_variable_expr: /* variable_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7435 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_expr: /* symbol_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7441 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_expr: /* set_ref_expr  */
#line 854 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7447 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_symbol_expr: /* set_ref_symbol_expr  */
#line 854 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7453 "src/parser_bison.c"
        break;

    case YYSYMBOL_integer_expr: /* integer_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7459 "src/parser_bison.c"
        break;

    case YYSYMBOL_selector_expr: /* selector_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7465 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_expr: /* primary_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7471 "src/parser_bison.c"
        break;

    case YYSYMBOL_fib_expr: /* fib_expr  */
#line 980 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7477 "src/parser_bison.c"
        break;

    case YYSYMBOL_osf_expr: /* osf_expr  */
#line 985 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7483 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_expr: /* shift_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7489 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_expr: /* and_expr  */
#line 848 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7495 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_expr: /* exclusive_or_expr  */
#line 850 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7501 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_expr: /* inclusive_or_expr  */
#line 850 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7507 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_expr: /* basic_expr  */
#line 852 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7513 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_expr: /* concat_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7519 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_rhs_expr: /* prefix_rhs_expr  */
#line 859 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7525 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_rhs_expr: /* range_rhs_expr  */
#line 859 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7531 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_rhs_expr: /* multiton_rhs_expr  */
#line 857 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7537 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_expr: /* map_expr  */
#line 880 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7543 "src/parser_bison.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7549 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_expr: /* set_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7555 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_expr: /* set_list_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7561 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_member_expr: /* set_list_member_expr  */
#line 889 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7567 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr: /* meter_key_expr  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7573 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr_alloc: /* meter_key_expr_alloc  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7579 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr: /* set_elem_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7585 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_key_expr: /* set_elem_key_expr  */
#line 1036 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7591 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_alloc: /* set_elem_expr_alloc  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7597 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt_list: /* set_elem_stmt_list  */
#line 793 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7603 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt: /* set_elem_stmt  */
#line 795 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7609 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_lhs_expr: /* set_lhs_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7615 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_rhs_expr: /* set_rhs_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7621 "src/parser_bison.c"
        break;

    case YYSYMBOL_initializer_expr: /* initializer_expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7627 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_obj: /* counter_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7633 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_obj: /* quota_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7639 "src/parser_bison.c"
        break;

    case YYSYMBOL_secmark_obj: /* secmark_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7645 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_states: /* timeout_states  */
#line 1029 "src/parser_bison.y"
            { timeout_states_free(((*yyvaluep).list)); }
#line 7651 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_state: /* timeout_state  */
#line 1026 "src/parser_bison.y"
            { timeout_state_free(((*yyvaluep).timeout_state)); }
#line 7657 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_obj_alloc: /* ct_obj_alloc  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7663 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_obj: /* limit_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7669 "src/parser_bison.c"
        break;

    case YYSYMBOL_tunnel_obj: /* tunnel_obj  */
#line 909 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7675 "src/parser_bison.c"
        break;

    case YYSYMBOL_relational_expr: /* relational_expr  */
#line 912 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7681 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_rhs_expr: /* list_rhs_expr  */
#line 904 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7687 "src/parser_bison.c"
        break;

    case YYSYMBOL_rhs_expr: /* rhs_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7693 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_rhs_expr: /* shift_rhs_expr  */
#line 904 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7699 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_rhs_expr: /* and_rhs_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7705 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_rhs_expr: /* exclusive_or_rhs_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7711 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_rhs_expr: /* inclusive_or_rhs_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7717 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_rhs_expr: /* basic_rhs_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7723 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_rhs_expr: /* concat_rhs_expr  */
#line 902 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7729 "src/parser_bison.c"
        break;

    case YYSYMBOL_boolean_expr: /* boolean_expr  */
#line 1016 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7735 "src/parser_bison.c"
        break;

    case YYSYMBOL_keyword_expr: /* keyword_expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7741 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_rhs_expr: /* primary_rhs_expr  */
#line 904 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7747 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_expr: /* verdict_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7753 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_expr: /* chain_expr  */
#line 846 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7759 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_expr: /* meta_expr  */
#line 962 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7765 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_stmt: /* meta_stmt  */
#line 808 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7771 "src/parser_bison.c"
        break;

    case YYSYMBOL_socket_expr: /* socket_expr  */
#line 966 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7777 "src/parser_bison.c"
        break;

    case YYSYMBOL_tunnel_expr: /* tunnel_expr  */
#line 962 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7783 "src/parser_bison.c"
        break;

    case YYSYMBOL_numgen_expr: /* numgen_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7789 "src/parser_bison.c"
        break;

    case YYSYMBOL_xfrm_expr: /* xfrm_expr  */
#line 1033 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7795 "src/parser_bison.c"
        break;

    case YYSYMBOL_hash_expr: /* hash_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7801 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_expr: /* rt_expr  */
#line 972 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7807 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_expr: /* ct_expr  */
#line 976 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7813 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_stmt_expr: /* symbol_stmt_expr  */
#line 904 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7819 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_stmt_expr: /* list_stmt_expr  */
#line 872 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7825 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_stmt: /* ct_stmt  */
#line 806 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7831 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_stmt: /* payload_stmt  */
#line 804 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7837 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_expr: /* payload_expr  */
#line 916 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7843 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_raw_expr: /* payload_raw_expr  */
#line 916 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7849 "src/parser_bison.c"
        break;

    case YYSYMBOL_eth_hdr_expr: /* eth_hdr_expr  */
#line 921 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7855 "src/parser_bison.c"
        break;

    case YYSYMBOL_vlan_hdr_expr: /* vlan_hdr_expr  */
#line 921 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7861 "src/parser_bison.c"
        break;

    case YYSYMBOL_arp_hdr_expr: /* arp_hdr_expr  */
#line 924 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7867 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip_hdr_expr: /* ip_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7873 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp_hdr_expr: /* icmp_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7879 "src/parser_bison.c"
        break;

    case YYSYMBOL_igmp_hdr_expr: /* igmp_hdr_expr  */
#line 927 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7885 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip6_hdr_expr: /* ip6_hdr_expr  */
#line 931 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7891 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp6_hdr_expr: /* icmp6_hdr_expr  */
#line 931 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7897 "src/parser_bison.c"
        break;

    case YYSYMBOL_auth_hdr_expr: /* auth_hdr_expr  */
#line 934 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7903 "src/parser_bison.c"
        break;

    case YYSYMBOL_esp_hdr_expr: /* esp_hdr_expr  */
#line 934 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7909 "src/parser_bison.c"
        break;

    case YYSYMBOL_comp_hdr_expr: /* comp_hdr_expr  */
#line 934 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7915 "src/parser_bison.c"
        break;

    case YYSYMBOL_udp_hdr_expr: /* udp_hdr_expr  */
#line 937 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7921 "src/parser_bison.c"
        break;

    case YYSYMBOL_udplite_hdr_expr: /* udplite_hdr_expr  */
#line 937 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7927 "src/parser_bison.c"
        break;

    case YYSYMBOL_tcp_hdr_expr: /* tcp_hdr_expr  */
#line 995 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7933 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_inet_expr: /* inner_inet_expr  */
#line 1003 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7939 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_eth_expr: /* inner_eth_expr  */
#line 1003 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7945 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_expr: /* inner_expr  */
#line 1003 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7951 "src/parser_bison.c"
        break;

    case YYSYMBOL_vxlan_hdr_expr: /* vxlan_hdr_expr  */
#line 1006 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7957 "src/parser_bison.c"
        break;

    case YYSYMBOL_geneve_hdr_expr: /* geneve_hdr_expr  */
#line 1006 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7963 "src/parser_bison.c"
        break;

    case YYSYMBOL_gre_hdr_expr: /* gre_hdr_expr  */
#line 1006 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7969 "src/parser_bison.c"
        break;

    case YYSYMBOL_gretap_hdr_expr: /* gretap_hdr_expr  */
#line 1006 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7975 "src/parser_bison.c"
        break;

    case YYSYMBOL_optstrip_stmt: /* optstrip_stmt  */
#line 1010 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7981 "src/parser_bison.c"
        break;

    case YYSYMBOL_dccp_hdr_expr: /* dccp_hdr_expr  */
#line 940 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7987 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_chunk_alloc: /* sctp_chunk_alloc  */
#line 940 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7993 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_hdr_expr: /* sctp_hdr_expr  */
#line 940 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7999 "src/parser_bison.c"
        break;

    case YYSYMBOL_th_hdr_expr: /* th_hdr_expr  */
#line 946 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8005 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_expr: /* exthdr_expr  */
#line 950 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8011 "src/parser_bison.c"
        break;

    case YYSYMBOL_hbh_hdr_expr: /* hbh_hdr_expr  */
#line 952 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8017 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_hdr_expr: /* rt_hdr_expr  */
#line 955 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8023 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt0_hdr_expr: /* rt0_hdr_expr  */
#line 955 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8029 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt2_hdr_expr: /* rt2_hdr_expr  */
#line 955 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8035 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt4_hdr_expr: /* rt4_hdr_expr  */
#line 955 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8041 "src/parser_bison.c"
        break;

    case YYSYMBOL_frag_hdr_expr: /* frag_hdr_expr  */
#line 952 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8047 "src/parser_bison.c"
        break;

    case YYSYMBOL_dst_hdr_expr: /* dst_hdr_expr  */
#line 952 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8053 "src/parser_bison.c"
        break;

    case YYSYMBOL_mh_hdr_expr: /* mh_hdr_expr  */
#line 958 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8059 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_exists_expr: /* exthdr_exists_expr  */
#line 1020 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8065 "src/parser_bison.c"
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
#line 227 "src/parser_bison.y"
{
	location_init(scanner, state, &yylloc);
	if (nft->debug_mask & NFT_DEBUG_SCANNER)
		nft_set_debug(1, scanner);
	if (nft->debug_mask & NFT_DEBUG_PARSER)
		yydebug = 1;
}

#line 8171 "src/parser_bison.c"

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
#line 1042 "src/parser_bison.y"
                        {
				if ((yyvsp[0].cmd) != NULL) {
					(yyvsp[0].cmd)->location = (yylsp[0]);
					list_add_tail(&(yyvsp[0].cmd)->list, state->cmds);
				}
			}
#line 8389 "src/parser_bison.c"
    break;

  case 8: /* close_scope_ah: %empty  */
#line 1058 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_AH); }
#line 8395 "src/parser_bison.c"
    break;

  case 9: /* close_scope_arp: %empty  */
#line 1059 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ARP); }
#line 8401 "src/parser_bison.c"
    break;

  case 10: /* close_scope_at: %empty  */
#line 1060 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_AT); }
#line 8407 "src/parser_bison.c"
    break;

  case 11: /* close_scope_comp: %empty  */
#line 1061 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_COMP); }
#line 8413 "src/parser_bison.c"
    break;

  case 12: /* close_scope_ct: %empty  */
#line 1062 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CT); }
#line 8419 "src/parser_bison.c"
    break;

  case 13: /* close_scope_counter: %empty  */
#line 1063 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_COUNTER); }
#line 8425 "src/parser_bison.c"
    break;

  case 14: /* close_scope_last: %empty  */
#line 1064 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LAST); }
#line 8431 "src/parser_bison.c"
    break;

  case 15: /* close_scope_dccp: %empty  */
#line 1065 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DCCP); }
#line 8437 "src/parser_bison.c"
    break;

  case 16: /* close_scope_destroy: %empty  */
#line 1066 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_DESTROY); }
#line 8443 "src/parser_bison.c"
    break;

  case 17: /* close_scope_dst: %empty  */
#line 1067 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DST); }
#line 8449 "src/parser_bison.c"
    break;

  case 18: /* close_scope_dup: %empty  */
#line 1068 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_DUP); }
#line 8455 "src/parser_bison.c"
    break;

  case 19: /* close_scope_esp: %empty  */
#line 1069 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_ESP); }
#line 8461 "src/parser_bison.c"
    break;

  case 20: /* close_scope_eth: %empty  */
#line 1070 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ETH); }
#line 8467 "src/parser_bison.c"
    break;

  case 21: /* close_scope_export: %empty  */
#line 1071 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_EXPORT); }
#line 8473 "src/parser_bison.c"
    break;

  case 22: /* close_scope_fib: %empty  */
#line 1072 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FIB); }
#line 8479 "src/parser_bison.c"
    break;

  case 23: /* close_scope_frag: %empty  */
#line 1073 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FRAG); }
#line 8485 "src/parser_bison.c"
    break;

  case 24: /* close_scope_fwd: %empty  */
#line 1074 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_FWD); }
#line 8491 "src/parser_bison.c"
    break;

  case 25: /* close_scope_gre: %empty  */
#line 1075 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_GRE); }
#line 8497 "src/parser_bison.c"
    break;

  case 26: /* close_scope_hash: %empty  */
#line 1076 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HASH); }
#line 8503 "src/parser_bison.c"
    break;

  case 27: /* close_scope_hbh: %empty  */
#line 1077 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HBH); }
#line 8509 "src/parser_bison.c"
    break;

  case 28: /* close_scope_ip: %empty  */
#line 1078 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP); }
#line 8515 "src/parser_bison.c"
    break;

  case 29: /* close_scope_ip6: %empty  */
#line 1079 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP6); }
#line 8521 "src/parser_bison.c"
    break;

  case 30: /* close_scope_vlan: %empty  */
#line 1080 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_VLAN); }
#line 8527 "src/parser_bison.c"
    break;

  case 31: /* close_scope_icmp: %empty  */
#line 1081 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ICMP); }
#line 8533 "src/parser_bison.c"
    break;

  case 32: /* close_scope_igmp: %empty  */
#line 1082 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IGMP); }
#line 8539 "src/parser_bison.c"
    break;

  case 33: /* close_scope_import: %empty  */
#line 1083 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_IMPORT); }
#line 8545 "src/parser_bison.c"
    break;

  case 34: /* close_scope_ipsec: %empty  */
#line 1084 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_IPSEC); }
#line 8551 "src/parser_bison.c"
    break;

  case 35: /* close_scope_list: %empty  */
#line 1085 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_LIST); }
#line 8557 "src/parser_bison.c"
    break;

  case 36: /* close_scope_limit: %empty  */
#line 1086 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LIMIT); }
#line 8563 "src/parser_bison.c"
    break;

  case 37: /* close_scope_meta: %empty  */
#line 1087 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_META); }
#line 8569 "src/parser_bison.c"
    break;

  case 38: /* close_scope_mh: %empty  */
#line 1088 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_MH); }
#line 8575 "src/parser_bison.c"
    break;

  case 39: /* close_scope_monitor: %empty  */
#line 1089 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_MONITOR); }
#line 8581 "src/parser_bison.c"
    break;

  case 40: /* close_scope_nat: %empty  */
#line 1090 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_NAT); }
#line 8587 "src/parser_bison.c"
    break;

  case 41: /* close_scope_numgen: %empty  */
#line 1091 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_NUMGEN); }
#line 8593 "src/parser_bison.c"
    break;

  case 42: /* close_scope_osf: %empty  */
#line 1092 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_OSF); }
#line 8599 "src/parser_bison.c"
    break;

  case 43: /* close_scope_policy: %empty  */
#line 1093 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_POLICY); }
#line 8605 "src/parser_bison.c"
    break;

  case 44: /* close_scope_quota: %empty  */
#line 1094 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_QUOTA); }
#line 8611 "src/parser_bison.c"
    break;

  case 45: /* close_scope_queue: %empty  */
#line 1095 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_QUEUE); }
#line 8617 "src/parser_bison.c"
    break;

  case 46: /* close_scope_reject: %empty  */
#line 1096 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_REJECT); }
#line 8623 "src/parser_bison.c"
    break;

  case 47: /* close_scope_reset: %empty  */
#line 1097 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_RESET); }
#line 8629 "src/parser_bison.c"
    break;

  case 48: /* close_scope_rt: %empty  */
#line 1098 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_RT); }
#line 8635 "src/parser_bison.c"
    break;

  case 49: /* close_scope_sctp: %empty  */
#line 1099 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SCTP); }
#line 8641 "src/parser_bison.c"
    break;

  case 50: /* close_scope_sctp_chunk: %empty  */
#line 1100 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SCTP_CHUNK); }
#line 8647 "src/parser_bison.c"
    break;

  case 51: /* close_scope_secmark: %empty  */
#line 1101 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SECMARK); }
#line 8653 "src/parser_bison.c"
    break;

  case 52: /* close_scope_socket: %empty  */
#line 1102 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SOCKET); }
#line 8659 "src/parser_bison.c"
    break;

  case 53: /* close_scope_tcp: %empty  */
#line 1103 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TCP); }
#line 8665 "src/parser_bison.c"
    break;

  case 54: /* close_scope_tproxy: %empty  */
#line 1104 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_TPROXY); }
#line 8671 "src/parser_bison.c"
    break;

  case 55: /* close_scope_type: %empty  */
#line 1105 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TYPE); }
#line 8677 "src/parser_bison.c"
    break;

  case 56: /* close_scope_th: %empty  */
#line 1106 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_TH); }
#line 8683 "src/parser_bison.c"
    break;

  case 57: /* close_scope_udp: %empty  */
#line 1107 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDP); }
#line 8689 "src/parser_bison.c"
    break;

  case 58: /* close_scope_udplite: %empty  */
#line 1108 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDPLITE); }
#line 8695 "src/parser_bison.c"
    break;

  case 59: /* close_scope_log: %empty  */
#line 1110 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_LOG); }
#line 8701 "src/parser_bison.c"
    break;

  case 60: /* close_scope_synproxy: %empty  */
#line 1111 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_SYNPROXY); }
#line 8707 "src/parser_bison.c"
    break;

  case 61: /* close_scope_tunnel: %empty  */
#line 1112 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TUNNEL); }
#line 8713 "src/parser_bison.c"
    break;

  case 62: /* close_scope_xt: %empty  */
#line 1113 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_XT); }
#line 8719 "src/parser_bison.c"
    break;

  case 63: /* common_block: "include" "quoted string" stmt_separator  */
#line 1116 "src/parser_bison.y"
                        {
				if (scanner_include_file(nft, scanner, (yyvsp[-1].string), &(yyloc)) < 0) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				free_const((yyvsp[-1].string));
			}
#line 8731 "src/parser_bison.c"
    break;

  case 64: /* common_block: "define" identifier '=' initializer_expr stmt_separator  */
#line 1124 "src/parser_bison.y"
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
#line 8750 "src/parser_bison.c"
    break;

  case 65: /* common_block: "redefine" identifier '=' initializer_expr stmt_separator  */
#line 1139 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);

				symbol_bind(scope, (yyvsp[-3].string), (yyvsp[-1].expr));
				free_const((yyvsp[-3].string));
			}
#line 8761 "src/parser_bison.c"
    break;

  case 66: /* common_block: "undefine" identifier stmt_separator  */
#line 1146 "src/parser_bison.y"
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
#line 8777 "src/parser_bison.c"
    break;

  case 67: /* common_block: error stmt_separator  */
#line 1158 "src/parser_bison.y"
                        {
				if (++state->nerrs == nft->parser_max_errors)
					YYABORT;
				yyerrok;
			}
#line 8787 "src/parser_bison.c"
    break;

  case 68: /* line: common_block  */
#line 1165 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8793 "src/parser_bison.c"
    break;

  case 69: /* line: stmt_separator  */
#line 1166 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8799 "src/parser_bison.c"
    break;

  case 70: /* line: base_cmd stmt_separator  */
#line 1167 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8805 "src/parser_bison.c"
    break;

  case 71: /* line: base_cmd "end of file"  */
#line 1169 "src/parser_bison.y"
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
#line 8827 "src/parser_bison.c"
    break;

  case 72: /* base_cmd: add_cmd  */
#line 1188 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8833 "src/parser_bison.c"
    break;

  case 73: /* base_cmd: "add" add_cmd  */
#line 1189 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8839 "src/parser_bison.c"
    break;

  case 74: /* base_cmd: "replace" replace_cmd  */
#line 1190 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8845 "src/parser_bison.c"
    break;

  case 75: /* base_cmd: "create" create_cmd  */
#line 1191 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8851 "src/parser_bison.c"
    break;

  case 76: /* base_cmd: "insert" insert_cmd  */
#line 1192 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8857 "src/parser_bison.c"
    break;

  case 77: /* base_cmd: "delete" delete_cmd  */
#line 1193 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8863 "src/parser_bison.c"
    break;

  case 78: /* base_cmd: "get" get_cmd  */
#line 1194 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8869 "src/parser_bison.c"
    break;

  case 79: /* base_cmd: "list" list_cmd close_scope_list  */
#line 1195 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8875 "src/parser_bison.c"
    break;

  case 80: /* base_cmd: "reset" reset_cmd close_scope_reset  */
#line 1196 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8881 "src/parser_bison.c"
    break;

  case 81: /* base_cmd: "flush" flush_cmd  */
#line 1197 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8887 "src/parser_bison.c"
    break;

  case 82: /* base_cmd: "rename" rename_cmd  */
#line 1198 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8893 "src/parser_bison.c"
    break;

  case 83: /* base_cmd: "import" import_cmd close_scope_import  */
#line 1199 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8899 "src/parser_bison.c"
    break;

  case 84: /* base_cmd: "export" export_cmd close_scope_export  */
#line 1200 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8905 "src/parser_bison.c"
    break;

  case 85: /* base_cmd: "monitor" monitor_cmd close_scope_monitor  */
#line 1201 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8911 "src/parser_bison.c"
    break;

  case 86: /* base_cmd: "describe" describe_cmd  */
#line 1202 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8917 "src/parser_bison.c"
    break;

  case 87: /* base_cmd: "destroy" destroy_cmd close_scope_destroy  */
#line 1203 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8923 "src/parser_bison.c"
    break;

  case 88: /* add_cmd: "table" table_spec  */
#line 1207 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8931 "src/parser_bison.c"
    break;

  case 89: /* add_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1212 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 8941 "src/parser_bison.c"
    break;

  case 90: /* add_cmd: "chain" chain_spec  */
#line 1218 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 8949 "src/parser_bison.c"
    break;

  case 91: /* add_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1223 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 8960 "src/parser_bison.c"
    break;

  case 92: /* add_cmd: "rule" rule_position rule  */
#line 1230 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 8968 "src/parser_bison.c"
    break;

  case 93: /* add_cmd: rule_position rule  */
#line 1234 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 8976 "src/parser_bison.c"
    break;

  case 94: /* add_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1239 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8986 "src/parser_bison.c"
    break;

  case 95: /* add_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1246 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 8996 "src/parser_bison.c"
    break;

  case 96: /* add_cmd: "element" set_spec set_block_expr  */
#line 1252 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_ADD, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9010 "src/parser_bison.c"
    break;

  case 97: /* add_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1263 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9020 "src/parser_bison.c"
    break;

  case 98: /* add_cmd: "counter" obj_spec close_scope_counter  */
#line 1269 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 9033 "src/parser_bison.c"
    break;

  case 99: /* add_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1278 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9041 "src/parser_bison.c"
    break;

  case 100: /* add_cmd: "counter" obj_spec counter_obj '{' counter_block '}' close_scope_counter  */
#line 1282 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9049 "src/parser_bison.c"
    break;

  case 101: /* add_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1286 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9057 "src/parser_bison.c"
    break;

  case 102: /* add_cmd: "quota" obj_spec quota_obj '{' quota_block '}' close_scope_quota  */
#line 1290 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9065 "src/parser_bison.c"
    break;

  case 103: /* add_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1294 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9073 "src/parser_bison.c"
    break;

  case 104: /* add_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1298 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9081 "src/parser_bison.c"
    break;

  case 105: /* add_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1302 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9089 "src/parser_bison.c"
    break;

  case 106: /* add_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1306 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9097 "src/parser_bison.c"
    break;

  case 107: /* add_cmd: "limit" obj_spec limit_obj '{' limit_block '}' close_scope_limit  */
#line 1310 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9105 "src/parser_bison.c"
    break;

  case 108: /* add_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1314 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9113 "src/parser_bison.c"
    break;

  case 109: /* add_cmd: "secmark" obj_spec secmark_obj '{' secmark_block '}' close_scope_secmark  */
#line 1318 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9121 "src/parser_bison.c"
    break;

  case 110: /* add_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1322 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9129 "src/parser_bison.c"
    break;

  case 111: /* add_cmd: "synproxy" obj_spec synproxy_obj '{' synproxy_block '}' close_scope_synproxy  */
#line 1326 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9137 "src/parser_bison.c"
    break;

  case 112: /* add_cmd: "tunnel" obj_spec tunnel_obj '{' tunnel_block '}' close_scope_tunnel  */
#line 1330 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TUNNEL, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9145 "src/parser_bison.c"
    break;

  case 113: /* replace_cmd: "rule" ruleid_spec rule  */
#line 1336 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_REPLACE, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9153 "src/parser_bison.c"
    break;

  case 114: /* create_cmd: "table" table_spec  */
#line 1342 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9161 "src/parser_bison.c"
    break;

  case 115: /* create_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1347 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 9171 "src/parser_bison.c"
    break;

  case 116: /* create_cmd: "chain" chain_spec  */
#line 1353 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9179 "src/parser_bison.c"
    break;

  case 117: /* create_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1358 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9190 "src/parser_bison.c"
    break;

  case 118: /* create_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1366 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9200 "src/parser_bison.c"
    break;

  case 119: /* create_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1373 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9210 "src/parser_bison.c"
    break;

  case 120: /* create_cmd: "element" set_spec set_block_expr  */
#line 1379 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_CREATE, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9224 "src/parser_bison.c"
    break;

  case 121: /* create_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1390 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9234 "src/parser_bison.c"
    break;

  case 122: /* create_cmd: "counter" obj_spec close_scope_counter  */
#line 1396 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 9247 "src/parser_bison.c"
    break;

  case 123: /* create_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1405 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9255 "src/parser_bison.c"
    break;

  case 124: /* create_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1409 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9263 "src/parser_bison.c"
    break;

  case 125: /* create_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1413 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9271 "src/parser_bison.c"
    break;

  case 126: /* create_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1417 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9279 "src/parser_bison.c"
    break;

  case 127: /* create_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1421 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9287 "src/parser_bison.c"
    break;

  case 128: /* create_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1425 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9295 "src/parser_bison.c"
    break;

  case 129: /* create_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1429 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9303 "src/parser_bison.c"
    break;

  case 130: /* create_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1433 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9311 "src/parser_bison.c"
    break;

  case 131: /* create_cmd: "tunnel" obj_spec tunnel_obj '{' tunnel_block '}' close_scope_tunnel  */
#line 1437 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TUNNEL, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9319 "src/parser_bison.c"
    break;

  case 132: /* insert_cmd: "rule" rule_position rule  */
#line 1443 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_INSERT, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9327 "src/parser_bison.c"
    break;

  case 141: /* delete_cmd: "table" table_or_id_spec  */
#line 1465 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9335 "src/parser_bison.c"
    break;

  case 142: /* delete_cmd: "chain" chain_or_id_spec  */
#line 1469 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9343 "src/parser_bison.c"
    break;

  case 143: /* delete_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1474 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9354 "src/parser_bison.c"
    break;

  case 144: /* delete_cmd: "rule" ruleid_spec  */
#line 1481 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9362 "src/parser_bison.c"
    break;

  case 145: /* delete_cmd: "set" set_or_id_spec  */
#line 1485 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9370 "src/parser_bison.c"
    break;

  case 146: /* delete_cmd: "map" set_or_id_spec  */
#line 1489 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9378 "src/parser_bison.c"
    break;

  case 147: /* delete_cmd: "element" set_spec set_block_expr  */
#line 1493 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9386 "src/parser_bison.c"
    break;

  case 148: /* delete_cmd: "flowtable" flowtable_spec  */
#line 1497 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9394 "src/parser_bison.c"
    break;

  case 149: /* delete_cmd: "flowtable" flowtableid_spec  */
#line 1501 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9402 "src/parser_bison.c"
    break;

  case 150: /* delete_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1506 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9412 "src/parser_bison.c"
    break;

  case 151: /* delete_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1512 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9420 "src/parser_bison.c"
    break;

  case 152: /* delete_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1516 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9428 "src/parser_bison.c"
    break;

  case 153: /* delete_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1520 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DELETE, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9438 "src/parser_bison.c"
    break;

  case 154: /* delete_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1526 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9446 "src/parser_bison.c"
    break;

  case 155: /* delete_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1530 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9454 "src/parser_bison.c"
    break;

  case 156: /* delete_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1534 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9462 "src/parser_bison.c"
    break;

  case 157: /* delete_cmd: "tunnel" obj_or_id_spec close_scope_tunnel  */
#line 1538 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9470 "src/parser_bison.c"
    break;

  case 158: /* destroy_cmd: "table" table_or_id_spec  */
#line 1544 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9478 "src/parser_bison.c"
    break;

  case 159: /* destroy_cmd: "chain" chain_or_id_spec  */
#line 1548 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9486 "src/parser_bison.c"
    break;

  case 160: /* destroy_cmd: "rule" ruleid_spec  */
#line 1552 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9494 "src/parser_bison.c"
    break;

  case 161: /* destroy_cmd: "set" set_or_id_spec  */
#line 1556 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9502 "src/parser_bison.c"
    break;

  case 162: /* destroy_cmd: "map" set_spec  */
#line 1560 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9510 "src/parser_bison.c"
    break;

  case 163: /* destroy_cmd: "element" set_spec set_block_expr  */
#line 1564 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9518 "src/parser_bison.c"
    break;

  case 164: /* destroy_cmd: "flowtable" flowtable_spec  */
#line 1568 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9526 "src/parser_bison.c"
    break;

  case 165: /* destroy_cmd: "flowtable" flowtableid_spec  */
#line 1572 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9534 "src/parser_bison.c"
    break;

  case 166: /* destroy_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1577 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9544 "src/parser_bison.c"
    break;

  case 167: /* destroy_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1583 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9552 "src/parser_bison.c"
    break;

  case 168: /* destroy_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1587 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9560 "src/parser_bison.c"
    break;

  case 169: /* destroy_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1591 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DESTROY, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9570 "src/parser_bison.c"
    break;

  case 170: /* destroy_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1597 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9578 "src/parser_bison.c"
    break;

  case 171: /* destroy_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1601 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9586 "src/parser_bison.c"
    break;

  case 172: /* destroy_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1605 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9594 "src/parser_bison.c"
    break;

  case 173: /* destroy_cmd: "tunnel" obj_or_id_spec close_scope_tunnel  */
#line 1609 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9602 "src/parser_bison.c"
    break;

  case 174: /* get_cmd: "element" set_spec set_block_expr  */
#line 1616 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_GET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9610 "src/parser_bison.c"
    break;

  case 175: /* list_cmd_spec_table: "table" table_spec  */
#line 1621 "src/parser_bison.y"
                                                        { (yyval.handle) = (yyvsp[0].handle); }
#line 9616 "src/parser_bison.c"
    break;

  case 179: /* list_cmd: "table" table_spec  */
#line 1629 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9624 "src/parser_bison.c"
    break;

  case 180: /* list_cmd: "tables" ruleset_spec  */
#line 1633 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9632 "src/parser_bison.c"
    break;

  case 181: /* list_cmd: "chain" chain_spec  */
#line 1637 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9640 "src/parser_bison.c"
    break;

  case 182: /* list_cmd: "chains" ruleset_spec  */
#line 1641 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAINS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9648 "src/parser_bison.c"
    break;

  case 183: /* list_cmd: "sets" list_cmd_spec_any  */
#line 1645 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SETS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9656 "src/parser_bison.c"
    break;

  case 184: /* list_cmd: "set" set_spec  */
#line 1649 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9664 "src/parser_bison.c"
    break;

  case 185: /* list_cmd: "counters" list_cmd_spec_any  */
#line 1653 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9672 "src/parser_bison.c"
    break;

  case 186: /* list_cmd: "counter" obj_spec close_scope_counter  */
#line 1657 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9680 "src/parser_bison.c"
    break;

  case 187: /* list_cmd: "quotas" list_cmd_spec_any  */
#line 1661 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9688 "src/parser_bison.c"
    break;

  case 188: /* list_cmd: "quota" obj_spec close_scope_quota  */
#line 1665 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9696 "src/parser_bison.c"
    break;

  case 189: /* list_cmd: "limits" list_cmd_spec_any  */
#line 1669 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMITS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9704 "src/parser_bison.c"
    break;

  case 190: /* list_cmd: "limit" obj_spec close_scope_limit  */
#line 1673 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9712 "src/parser_bison.c"
    break;

  case 191: /* list_cmd: "secmarks" list_cmd_spec_any  */
#line 1677 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9720 "src/parser_bison.c"
    break;

  case 192: /* list_cmd: "secmark" obj_spec close_scope_secmark  */
#line 1681 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9728 "src/parser_bison.c"
    break;

  case 193: /* list_cmd: "synproxys" list_cmd_spec_any  */
#line 1685 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXYS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9736 "src/parser_bison.c"
    break;

  case 194: /* list_cmd: "synproxy" obj_spec close_scope_synproxy  */
#line 1689 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9744 "src/parser_bison.c"
    break;

  case 195: /* list_cmd: "ruleset" ruleset_spec  */
#line 1693 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9752 "src/parser_bison.c"
    break;

  case 196: /* list_cmd: "flow" "tables" ruleset_spec  */
#line 1697 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9760 "src/parser_bison.c"
    break;

  case 197: /* list_cmd: "flow" "table" set_spec  */
#line 1701 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9768 "src/parser_bison.c"
    break;

  case 198: /* list_cmd: "meters" ruleset_spec  */
#line 1705 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9776 "src/parser_bison.c"
    break;

  case 199: /* list_cmd: "meter" set_spec  */
#line 1709 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9784 "src/parser_bison.c"
    break;

  case 200: /* list_cmd: "flowtables" list_cmd_spec_any  */
#line 1713 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9792 "src/parser_bison.c"
    break;

  case 201: /* list_cmd: "flowtable" flowtable_spec  */
#line 1717 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9800 "src/parser_bison.c"
    break;

  case 202: /* list_cmd: "maps" list_cmd_spec_any  */
#line 1721 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAPS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9808 "src/parser_bison.c"
    break;

  case 203: /* list_cmd: "map" set_spec  */
#line 1725 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9816 "src/parser_bison.c"
    break;

  case 204: /* list_cmd: "ct" ct_obj_type obj_spec close_scope_ct  */
#line 1729 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_LIST, (yyvsp[-2].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9824 "src/parser_bison.c"
    break;

  case 205: /* list_cmd: "ct" ct_cmd_type "table" table_spec close_scope_ct  */
#line 1733 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, (yyvsp[-3].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9832 "src/parser_bison.c"
    break;

  case 206: /* list_cmd: "hooks" basehook_spec  */
#line 1737 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_HOOKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9840 "src/parser_bison.c"
    break;

  case 207: /* list_cmd: "tunnels" list_cmd_spec_any  */
#line 1741 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TUNNELS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9848 "src/parser_bison.c"
    break;

  case 208: /* list_cmd: "tunnel" obj_spec close_scope_tunnel  */
#line 1745 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9856 "src/parser_bison.c"
    break;

  case 209: /* basehook_device_name: "device" "string"  */
#line 1751 "src/parser_bison.y"
                        {
				(yyval.string) = (yyvsp[0].string);
			}
#line 9864 "src/parser_bison.c"
    break;

  case 210: /* basehook_spec: ruleset_spec  */
#line 1757 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 9872 "src/parser_bison.c"
    break;

  case 211: /* basehook_spec: ruleset_spec basehook_device_name  */
#line 1761 "src/parser_bison.y"
                        {
				if ((yyvsp[0].string)) {
					(yyvsp[-1].handle).obj.name = (yyvsp[0].string);
					(yyvsp[-1].handle).obj.location = (yylsp[0]);
				}
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 9884 "src/parser_bison.c"
    break;

  case 212: /* reset_cmd: "counters" list_cmd_spec_any  */
#line 1771 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9892 "src/parser_bison.c"
    break;

  case 213: /* reset_cmd: "counter" obj_spec close_scope_counter  */
#line 1775 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTER, &(yyvsp[-1].handle),&(yyloc), NULL);
			}
#line 9900 "src/parser_bison.c"
    break;

  case 214: /* reset_cmd: "quotas" list_cmd_spec_any  */
#line 1779 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9908 "src/parser_bison.c"
    break;

  case 215: /* reset_cmd: "quota" obj_spec close_scope_quota  */
#line 1783 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9916 "src/parser_bison.c"
    break;

  case 216: /* reset_cmd: "rules" ruleset_spec  */
#line 1787 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9924 "src/parser_bison.c"
    break;

  case 217: /* reset_cmd: "rules" list_cmd_spec_table  */
#line 1791 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9932 "src/parser_bison.c"
    break;

  case 218: /* reset_cmd: "rules" chain_spec  */
#line 1795 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9940 "src/parser_bison.c"
    break;

  case 219: /* reset_cmd: "rules" "chain" chain_spec  */
#line 1799 "src/parser_bison.y"
                        {
				/* alias of previous rule. */
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9949 "src/parser_bison.c"
    break;

  case 220: /* reset_cmd: "rule" ruleid_spec  */
#line 1804 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9957 "src/parser_bison.c"
    break;

  case 221: /* reset_cmd: "element" set_spec set_block_expr  */
#line 1808 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9965 "src/parser_bison.c"
    break;

  case 222: /* reset_cmd: "set" set_spec  */
#line 1812 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9973 "src/parser_bison.c"
    break;

  case 223: /* reset_cmd: "map" set_spec  */
#line 1816 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9981 "src/parser_bison.c"
    break;

  case 224: /* flush_cmd: "table" table_spec  */
#line 1822 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9989 "src/parser_bison.c"
    break;

  case 225: /* flush_cmd: "chain" chain_spec  */
#line 1826 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9997 "src/parser_bison.c"
    break;

  case 226: /* flush_cmd: "set" set_spec  */
#line 1830 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10005 "src/parser_bison.c"
    break;

  case 227: /* flush_cmd: "map" set_spec  */
#line 1834 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10013 "src/parser_bison.c"
    break;

  case 228: /* flush_cmd: "flow" "table" set_spec  */
#line 1838 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10021 "src/parser_bison.c"
    break;

  case 229: /* flush_cmd: "meter" set_spec  */
#line 1842 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10029 "src/parser_bison.c"
    break;

  case 230: /* flush_cmd: "ruleset" ruleset_spec  */
#line 1846 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10037 "src/parser_bison.c"
    break;

  case 231: /* rename_cmd: "chain" chain_spec identifier  */
#line 1852 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RENAME, CMD_OBJ_CHAIN, &(yyvsp[-1].handle), &(yyloc), NULL);
				(yyval.cmd)->arg = (yyvsp[0].string);
			}
#line 10046 "src/parser_bison.c"
    break;

  case 232: /* import_cmd: "ruleset" markup_format  */
#line 1859 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10056 "src/parser_bison.c"
    break;

  case 233: /* import_cmd: markup_format  */
#line 1865 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10066 "src/parser_bison.c"
    break;

  case 234: /* export_cmd: "ruleset" markup_format  */
#line 1873 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10076 "src/parser_bison.c"
    break;

  case 235: /* export_cmd: markup_format  */
#line 1879 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10086 "src/parser_bison.c"
    break;

  case 236: /* monitor_cmd: monitor_event monitor_object monitor_format  */
#line 1887 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct monitor *m = monitor_alloc((yyvsp[0].val), (yyvsp[-1].val), (yyvsp[-2].string));
				m->location = (yylsp[-2]);
				(yyval.cmd) = cmd_alloc(CMD_MONITOR, CMD_OBJ_MONITOR, &h, &(yyloc), m);
			}
#line 10097 "src/parser_bison.c"
    break;

  case 237: /* monitor_event: %empty  */
#line 1895 "src/parser_bison.y"
                                                { (yyval.string) = NULL; }
#line 10103 "src/parser_bison.c"
    break;

  case 238: /* monitor_event: "string"  */
#line 1896 "src/parser_bison.y"
                                                { (yyval.string) = (yyvsp[0].string); }
#line 10109 "src/parser_bison.c"
    break;

  case 239: /* monitor_object: %empty  */
#line 1899 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ANY; }
#line 10115 "src/parser_bison.c"
    break;

  case 240: /* monitor_object: "tables"  */
#line 1900 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TABLES; }
#line 10121 "src/parser_bison.c"
    break;

  case 241: /* monitor_object: "chains"  */
#line 1901 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_CHAINS; }
#line 10127 "src/parser_bison.c"
    break;

  case 242: /* monitor_object: "sets"  */
#line 1902 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_SETS; }
#line 10133 "src/parser_bison.c"
    break;

  case 243: /* monitor_object: "rules"  */
#line 1903 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULES; }
#line 10139 "src/parser_bison.c"
    break;

  case 244: /* monitor_object: "elements"  */
#line 1904 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ELEMS; }
#line 10145 "src/parser_bison.c"
    break;

  case 245: /* monitor_object: "ruleset"  */
#line 1905 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULESET; }
#line 10151 "src/parser_bison.c"
    break;

  case 246: /* monitor_object: "trace"  */
#line 1906 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TRACE; }
#line 10157 "src/parser_bison.c"
    break;

  case 247: /* monitor_format: %empty  */
#line 1909 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_DEFAULT; }
#line 10163 "src/parser_bison.c"
    break;

  case 249: /* markup_format: "xml"  */
#line 1913 "src/parser_bison.y"
                                                { (yyval.val) = __NFT_OUTPUT_NOTSUPP; }
#line 10169 "src/parser_bison.c"
    break;

  case 250: /* markup_format: "json"  */
#line 1914 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 10175 "src/parser_bison.c"
    break;

  case 251: /* markup_format: "vm" "json"  */
#line 1915 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 10181 "src/parser_bison.c"
    break;

  case 252: /* describe_cmd: primary_expr  */
#line 1919 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				(yyval.cmd) = cmd_alloc(CMD_DESCRIBE, CMD_OBJ_EXPR, &h, &(yyloc), NULL);
				(yyval.cmd)->expr = (yyvsp[0].expr);
			}
#line 10191 "src/parser_bison.c"
    break;

  case 253: /* table_block_alloc: %empty  */
#line 1927 "src/parser_bison.y"
                        {
				(yyval.table) = table_alloc();
				if (open_scope(state, &(yyval.table)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 10204 "src/parser_bison.c"
    break;

  case 254: /* table_options: "flags" table_flags  */
#line 1938 "src/parser_bison.y"
                        {
				(yyvsp[-2].table)->flags |= (yyvsp[0].val);
			}
#line 10212 "src/parser_bison.c"
    break;

  case 255: /* table_options: comment_spec  */
#line 1942 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].table)->comment, &(yyloc), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].table)->comment = (yyvsp[0].string);
			}
#line 10224 "src/parser_bison.c"
    break;

  case 257: /* table_flags: table_flags "comma" table_flag  */
#line 1953 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 10232 "src/parser_bison.c"
    break;

  case 258: /* table_flag: "string"  */
#line 1958 "src/parser_bison.y"
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
#line 10248 "src/parser_bison.c"
    break;

  case 259: /* table_block: %empty  */
#line 1971 "src/parser_bison.y"
                                                { (yyval.table) = (yyvsp[(-1) - (0)].table); }
#line 10254 "src/parser_bison.c"
    break;

  case 263: /* table_block: table_block "chain" chain_identifier chain_block_alloc '{' chain_block '}' stmt_separator  */
#line 1978 "src/parser_bison.y"
                        {
				(yyvsp[-4].chain)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].chain)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				close_scope(state);
				list_add_tail(&(yyvsp[-4].chain)->list, &(yyvsp[-7].table)->chains);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10267 "src/parser_bison.c"
    break;

  case 264: /* table_block: table_block "set" set_identifier set_block_alloc '{' set_block '}' stmt_separator  */
#line 1989 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10279 "src/parser_bison.c"
    break;

  case 265: /* table_block: table_block "map" set_identifier map_block_alloc '{' map_block '}' stmt_separator  */
#line 1999 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10291 "src/parser_bison.c"
    break;

  case 266: /* table_block: table_block "flowtable" flowtable_identifier flowtable_block_alloc '{' flowtable_block '}' stmt_separator  */
#line 2010 "src/parser_bison.y"
                        {
				(yyvsp[-4].flowtable)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].flowtable)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].flowtable)->list, &(yyvsp[-7].table)->flowtables);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10303 "src/parser_bison.c"
    break;

  case 267: /* table_block: table_block "counter" obj_identifier obj_block_alloc '{' counter_block '}' stmt_separator close_scope_counter  */
#line 2020 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_COUNTER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10316 "src/parser_bison.c"
    break;

  case 268: /* table_block: table_block "quota" obj_identifier obj_block_alloc '{' quota_block '}' stmt_separator close_scope_quota  */
#line 2031 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_QUOTA;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10329 "src/parser_bison.c"
    break;

  case 269: /* table_block: table_block "ct" "helper" obj_identifier obj_block_alloc '{' ct_helper_block '}' stmt_separator close_scope_ct  */
#line 2040 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_HELPER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10342 "src/parser_bison.c"
    break;

  case 270: /* table_block: table_block "ct" "timeout" obj_identifier obj_block_alloc '{' ct_timeout_block '}' stmt_separator close_scope_ct  */
#line 2049 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_TIMEOUT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10355 "src/parser_bison.c"
    break;

  case 271: /* table_block: table_block "ct" "expectation" obj_identifier obj_block_alloc '{' ct_expect_block '}' stmt_separator close_scope_ct  */
#line 2058 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_EXPECT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10368 "src/parser_bison.c"
    break;

  case 272: /* table_block: table_block "limit" obj_identifier obj_block_alloc '{' limit_block '}' stmt_separator close_scope_limit  */
#line 2069 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_LIMIT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10381 "src/parser_bison.c"
    break;

  case 273: /* table_block: table_block "secmark" obj_identifier obj_block_alloc '{' secmark_block '}' stmt_separator close_scope_secmark  */
#line 2080 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SECMARK;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10394 "src/parser_bison.c"
    break;

  case 274: /* table_block: table_block "synproxy" obj_identifier obj_block_alloc '{' synproxy_block '}' stmt_separator close_scope_synproxy  */
#line 2091 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SYNPROXY;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10407 "src/parser_bison.c"
    break;

  case 275: /* table_block: table_block "tunnel" obj_identifier obj_block_alloc '{' tunnel_block '}' stmt_separator close_scope_tunnel  */
#line 2102 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_TUNNEL;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10420 "src/parser_bison.c"
    break;

  case 276: /* chain_block_alloc: %empty  */
#line 2113 "src/parser_bison.y"
                        {
				(yyval.chain) = chain_alloc();
				if (open_scope(state, &(yyval.chain)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 10433 "src/parser_bison.c"
    break;

  case 277: /* chain_block: %empty  */
#line 2123 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10439 "src/parser_bison.c"
    break;

  case 283: /* chain_block: chain_block rule stmt_separator  */
#line 2130 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10448 "src/parser_bison.c"
    break;

  case 284: /* chain_block: chain_block "devices" '=' flowtable_expr stmt_separator  */
#line 2135 "src/parser_bison.y"
                        {
				if ((yyval.chain)->dev_expr) {
					list_splice_init(&expr_list((yyvsp[-1].expr))->expressions, &expr_list((yyval.chain)->dev_expr)->expressions);
					expr_free((yyvsp[-1].expr));
					break;
				}
				(yyval.chain)->dev_expr = (yyvsp[-1].expr);
			}
#line 10461 "src/parser_bison.c"
    break;

  case 285: /* chain_block: chain_block comment_spec stmt_separator  */
#line 2144 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].chain)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].chain)->comment = (yyvsp[-1].string);
			}
#line 10473 "src/parser_bison.c"
    break;

  case 286: /* subchain_block: %empty  */
#line 2153 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10479 "src/parser_bison.c"
    break;

  case 288: /* subchain_block: subchain_block rule stmt_separator  */
#line 2156 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10488 "src/parser_bison.c"
    break;

  case 289: /* typeof_verdict_expr: selector_expr  */
#line 2163 "src/parser_bison.y"
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
#line 10504 "src/parser_bison.c"
    break;

  case 290: /* typeof_verdict_expr: typeof_expr "." selector_expr  */
#line 2175 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10517 "src/parser_bison.c"
    break;

  case 291: /* typeof_data_expr: "interval" typeof_expr  */
#line 2186 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->flags |= EXPR_F_INTERVAL;
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10526 "src/parser_bison.c"
    break;

  case 292: /* typeof_data_expr: typeof_verdict_expr  */
#line 2191 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10534 "src/parser_bison.c"
    break;

  case 293: /* typeof_data_expr: "queue"  */
#line 2195 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &queue_type, BYTEORDER_HOST_ENDIAN, 16, NULL);
			}
#line 10542 "src/parser_bison.c"
    break;

  case 294: /* typeof_data_expr: "string"  */
#line 2199 "src/parser_bison.y"
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
#line 10561 "src/parser_bison.c"
    break;

  case 295: /* primary_typeof_expr: selector_expr  */
#line 2216 "src/parser_bison.y"
                        {
				if (expr_ops((yyvsp[0].expr))->build_udata == NULL) {
					erec_queue(error(&(yylsp[0]), "primary expression type '%s' lacks typeof serialization", expr_ops((yyvsp[0].expr))->name),
						   state->msgs);
					expr_free((yyvsp[0].expr));
					YYERROR;
				}

				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10576 "src/parser_bison.c"
    break;

  case 296: /* typeof_expr: primary_typeof_expr  */
#line 2229 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10584 "src/parser_bison.c"
    break;

  case 297: /* typeof_expr: typeof_expr "." primary_typeof_expr  */
#line 2233 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10597 "src/parser_bison.c"
    break;

  case 298: /* set_block_alloc: %empty  */
#line 2245 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10605 "src/parser_bison.c"
    break;

  case 299: /* typeof_key_expr: "typeof" typeof_expr  */
#line 2250 "src/parser_bison.y"
                                                    { (yyval.expr) = (yyvsp[0].expr); }
#line 10611 "src/parser_bison.c"
    break;

  case 300: /* typeof_key_expr: "type" data_type_expr close_scope_type  */
#line 2251 "src/parser_bison.y"
                                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 10617 "src/parser_bison.c"
    break;

  case 301: /* set_block: %empty  */
#line 2254 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 10623 "src/parser_bison.c"
    break;

  case 304: /* set_block: set_block typeof_key_expr stmt_separator  */
#line 2258 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->key, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-2].set)->key = (yyvsp[-1].expr);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10637 "src/parser_bison.c"
    break;

  case 305: /* set_block: set_block "flags" set_flag_list stmt_separator  */
#line 2268 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10646 "src/parser_bison.c"
    break;

  case 306: /* set_block: set_block "timeout" time_spec stmt_separator  */
#line 2273 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10655 "src/parser_bison.c"
    break;

  case 307: /* set_block: set_block "gc-interval" time_spec stmt_separator  */
#line 2278 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10664 "src/parser_bison.c"
    break;

  case 308: /* set_block: set_block stateful_stmt_list stmt_separator  */
#line 2283 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 10674 "src/parser_bison.c"
    break;

  case 309: /* set_block: set_block "elements" '=' set_block_expr  */
#line 2289 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-3].set)->init, &(yylsp[-2]), state)) {
					expr_free((yyvsp[0].expr));
					YYERROR;
				}
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10687 "src/parser_bison.c"
    break;

  case 310: /* set_block: set_block "auto-merge"  */
#line 2298 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->automerge = true;
				(yyval.set) = (yyvsp[-1].set);
			}
#line 10696 "src/parser_bison.c"
    break;

  case 312: /* set_block: set_block comment_spec stmt_separator  */
#line 2304 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10709 "src/parser_bison.c"
    break;

  case 315: /* set_flag_list: set_flag_list "comma" set_flag  */
#line 2319 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 10717 "src/parser_bison.c"
    break;

  case 317: /* set_flag: "constant"  */
#line 2325 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_CONSTANT; }
#line 10723 "src/parser_bison.c"
    break;

  case 318: /* set_flag: "interval"  */
#line 2326 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_INTERVAL; }
#line 10729 "src/parser_bison.c"
    break;

  case 319: /* set_flag: "timeout"  */
#line 2327 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_TIMEOUT; }
#line 10735 "src/parser_bison.c"
    break;

  case 320: /* set_flag: "dynamic"  */
#line 2328 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_EVAL; }
#line 10741 "src/parser_bison.c"
    break;

  case 321: /* map_block_alloc: %empty  */
#line 2332 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10749 "src/parser_bison.c"
    break;

  case 322: /* ct_obj_type_map: "timeout"  */
#line 2337 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 10755 "src/parser_bison.c"
    break;

  case 323: /* ct_obj_type_map: "expectation"  */
#line 2338 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 10761 "src/parser_bison.c"
    break;

  case 324: /* map_block_obj_type: "counter" close_scope_counter  */
#line 2341 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_COUNTER; }
#line 10767 "src/parser_bison.c"
    break;

  case 325: /* map_block_obj_type: "quota" close_scope_quota  */
#line 2342 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_QUOTA; }
#line 10773 "src/parser_bison.c"
    break;

  case 326: /* map_block_obj_type: "limit" close_scope_limit  */
#line 2343 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_LIMIT; }
#line 10779 "src/parser_bison.c"
    break;

  case 327: /* map_block_obj_type: "secmark" close_scope_secmark  */
#line 2344 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_SECMARK; }
#line 10785 "src/parser_bison.c"
    break;

  case 328: /* map_block_obj_type: "synproxy" close_scope_synproxy  */
#line 2345 "src/parser_bison.y"
                                                              { (yyval.val) = NFT_OBJECT_SYNPROXY; }
#line 10791 "src/parser_bison.c"
    break;

  case 330: /* map_block_obj_typeof: "ct" ct_obj_type_map close_scope_ct  */
#line 2349 "src/parser_bison.y"
                                                                        { (yyval.val) = (yyvsp[-1].val); }
#line 10797 "src/parser_bison.c"
    break;

  case 331: /* map_block_data_interval: "interval"  */
#line 2352 "src/parser_bison.y"
                                         { (yyval.val) = EXPR_F_INTERVAL; }
#line 10803 "src/parser_bison.c"
    break;

  case 332: /* map_block_data_interval: %empty  */
#line 2353 "src/parser_bison.y"
                                { (yyval.val) = 0; }
#line 10809 "src/parser_bison.c"
    break;

  case 333: /* map_block: %empty  */
#line 2356 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 10815 "src/parser_bison.c"
    break;

  case 336: /* map_block: map_block "timeout" time_spec stmt_separator  */
#line 2360 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10824 "src/parser_bison.c"
    break;

  case 337: /* map_block: map_block "gc-interval" time_spec stmt_separator  */
#line 2365 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10833 "src/parser_bison.c"
    break;

  case 338: /* map_block: map_block "type" data_type_expr "colon" map_block_data_interval data_type_expr stmt_separator close_scope_type  */
#line 2372 "src/parser_bison.y"
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
#line 10852 "src/parser_bison.c"
    break;

  case 339: /* map_block: map_block "typeof" typeof_expr "colon" typeof_data_expr stmt_separator  */
#line 2389 "src/parser_bison.y"
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
#line 10877 "src/parser_bison.c"
    break;

  case 340: /* map_block: map_block "type" data_type_expr "colon" map_block_obj_type stmt_separator close_scope_type  */
#line 2412 "src/parser_bison.y"
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
#line 10893 "src/parser_bison.c"
    break;

  case 341: /* map_block: map_block "typeof" typeof_expr "colon" map_block_obj_typeof stmt_separator  */
#line 2426 "src/parser_bison.y"
                        {
				(yyvsp[-5].set)->key = (yyvsp[-3].expr);
				(yyvsp[-5].set)->objtype = (yyvsp[-1].val);
				(yyvsp[-5].set)->flags  |= NFT_SET_OBJECT;
				(yyval.set) = (yyvsp[-5].set);
			}
#line 10904 "src/parser_bison.c"
    break;

  case 342: /* map_block: map_block "flags" set_flag_list stmt_separator  */
#line 2433 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags |= (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10913 "src/parser_bison.c"
    break;

  case 343: /* map_block: map_block stateful_stmt_list stmt_separator  */
#line 2438 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 10923 "src/parser_bison.c"
    break;

  case 344: /* map_block: map_block "elements" '=' set_block_expr  */
#line 2444 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10932 "src/parser_bison.c"
    break;

  case 345: /* map_block: map_block comment_spec stmt_separator  */
#line 2449 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10945 "src/parser_bison.c"
    break;

  case 347: /* set_mechanism: "policy" set_policy_spec close_scope_policy  */
#line 2461 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->policy = (yyvsp[-1].val);
			}
#line 10953 "src/parser_bison.c"
    break;

  case 348: /* set_mechanism: "size" "number"  */
#line 2465 "src/parser_bison.y"
                        {
				(yyvsp[-2].set)->desc.size = (yyvsp[0].val);
			}
#line 10961 "src/parser_bison.c"
    break;

  case 349: /* set_policy_spec: "performance"  */
#line 2470 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_PERFORMANCE; }
#line 10967 "src/parser_bison.c"
    break;

  case 350: /* set_policy_spec: "memory"  */
#line 2471 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_MEMORY; }
#line 10973 "src/parser_bison.c"
    break;

  case 351: /* flowtable_block_alloc: %empty  */
#line 2475 "src/parser_bison.y"
                        {
				(yyval.flowtable) = flowtable_alloc(&internal_location);
			}
#line 10981 "src/parser_bison.c"
    break;

  case 352: /* flowtable_block: %empty  */
#line 2480 "src/parser_bison.y"
                                                { (yyval.flowtable) = (yyvsp[(-1) - (0)].flowtable); }
#line 10987 "src/parser_bison.c"
    break;

  case 355: /* flowtable_block: flowtable_block "hook" "string" prio_spec stmt_separator  */
#line 2484 "src/parser_bison.y"
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
#line 11006 "src/parser_bison.c"
    break;

  case 356: /* flowtable_block: flowtable_block "devices" '=' flowtable_expr stmt_separator  */
#line 2499 "src/parser_bison.y"
                        {
				(yyval.flowtable)->dev_expr = (yyvsp[-1].expr);
			}
#line 11014 "src/parser_bison.c"
    break;

  case 357: /* flowtable_block: flowtable_block "counter" close_scope_counter  */
#line 2503 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= NFT_FLOWTABLE_COUNTER;
			}
#line 11022 "src/parser_bison.c"
    break;

  case 358: /* flowtable_block: flowtable_block "flags" "offload" stmt_separator  */
#line 2507 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= FLOWTABLE_F_HW_OFFLOAD;
			}
#line 11030 "src/parser_bison.c"
    break;

  case 359: /* flowtable_expr: '{' flowtable_list_expr '}'  */
#line 2513 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 11039 "src/parser_bison.c"
    break;

  case 360: /* flowtable_expr: variable_expr  */
#line 2518 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11048 "src/parser_bison.c"
    break;

  case 361: /* flowtable_list_expr: flowtable_expr_member  */
#line 2525 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11057 "src/parser_bison.c"
    break;

  case 362: /* flowtable_list_expr: flowtable_list_expr "comma" flowtable_expr_member  */
#line 2530 "src/parser_bison.y"
                        {
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 11066 "src/parser_bison.c"
    break;

  case 364: /* flowtable_expr_member: string  */
#line 2538 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = expr;
			}
#line 11079 "src/parser_bison.c"
    break;

  case 365: /* flowtable_expr_member: variable_expr  */
#line 2547 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11088 "src/parser_bison.c"
    break;

  case 366: /* data_type_atom_expr: type_identifier  */
#line 2554 "src/parser_bison.y"
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
#line 11105 "src/parser_bison.c"
    break;

  case 367: /* data_type_atom_expr: "time"  */
#line 2567 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yylsp[0]), &time_type, time_type.byteorder,
							 time_type.size, NULL);
			}
#line 11114 "src/parser_bison.c"
    break;

  case 369: /* data_type_expr: data_type_expr "." data_type_atom_expr  */
#line 2575 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 11127 "src/parser_bison.c"
    break;

  case 370: /* obj_block_alloc: %empty  */
#line 2586 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&internal_location);
			}
#line 11135 "src/parser_bison.c"
    break;

  case 371: /* counter_block: %empty  */
#line 2591 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11141 "src/parser_bison.c"
    break;

  case 374: /* counter_block: counter_block counter_config  */
#line 2595 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11149 "src/parser_bison.c"
    break;

  case 375: /* counter_block: counter_block comment_spec  */
#line 2599 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11161 "src/parser_bison.c"
    break;

  case 376: /* quota_block: %empty  */
#line 2608 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11167 "src/parser_bison.c"
    break;

  case 379: /* quota_block: quota_block quota_config  */
#line 2612 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11175 "src/parser_bison.c"
    break;

  case 380: /* quota_block: quota_block comment_spec  */
#line 2616 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11187 "src/parser_bison.c"
    break;

  case 381: /* ct_helper_block: %empty  */
#line 2625 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11193 "src/parser_bison.c"
    break;

  case 384: /* ct_helper_block: ct_helper_block ct_helper_config  */
#line 2629 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11201 "src/parser_bison.c"
    break;

  case 385: /* ct_helper_block: ct_helper_block comment_spec  */
#line 2633 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11213 "src/parser_bison.c"
    break;

  case 386: /* ct_timeout_block: %empty  */
#line 2643 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);
				init_list_head(&(yyval.obj)->ct_timeout.timeout_list);
				(yyval.obj)->type = NFT_OBJECT_CT_TIMEOUT;
			}
#line 11223 "src/parser_bison.c"
    break;

  case 389: /* ct_timeout_block: ct_timeout_block ct_timeout_config  */
#line 2651 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11231 "src/parser_bison.c"
    break;

  case 390: /* ct_timeout_block: ct_timeout_block comment_spec  */
#line 2655 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11243 "src/parser_bison.c"
    break;

  case 391: /* ct_expect_block: %empty  */
#line 2664 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11249 "src/parser_bison.c"
    break;

  case 394: /* ct_expect_block: ct_expect_block ct_expect_config  */
#line 2668 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11257 "src/parser_bison.c"
    break;

  case 395: /* ct_expect_block: ct_expect_block comment_spec  */
#line 2672 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11269 "src/parser_bison.c"
    break;

  case 396: /* limit_block: %empty  */
#line 2681 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11275 "src/parser_bison.c"
    break;

  case 399: /* limit_block: limit_block limit_config  */
#line 2685 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11283 "src/parser_bison.c"
    break;

  case 400: /* limit_block: limit_block comment_spec  */
#line 2689 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11295 "src/parser_bison.c"
    break;

  case 401: /* secmark_block: %empty  */
#line 2698 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11301 "src/parser_bison.c"
    break;

  case 404: /* secmark_block: secmark_block secmark_config  */
#line 2702 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11309 "src/parser_bison.c"
    break;

  case 405: /* secmark_block: secmark_block comment_spec  */
#line 2706 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11321 "src/parser_bison.c"
    break;

  case 406: /* synproxy_block: %empty  */
#line 2715 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11327 "src/parser_bison.c"
    break;

  case 409: /* synproxy_block: synproxy_block synproxy_config  */
#line 2719 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11335 "src/parser_bison.c"
    break;

  case 410: /* synproxy_block: synproxy_block comment_spec  */
#line 2723 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11347 "src/parser_bison.c"
    break;

  case 411: /* type_identifier: "string"  */
#line 2732 "src/parser_bison.y"
                                        { (yyval.string) = (yyvsp[0].string); }
#line 11353 "src/parser_bison.c"
    break;

  case 412: /* type_identifier: "mark"  */
#line 2733 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("mark"); }
#line 11359 "src/parser_bison.c"
    break;

  case 413: /* type_identifier: "dscp"  */
#line 2734 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("dscp"); }
#line 11365 "src/parser_bison.c"
    break;

  case 414: /* type_identifier: "ecn"  */
#line 2735 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("ecn"); }
#line 11371 "src/parser_bison.c"
    break;

  case 415: /* type_identifier: "classid"  */
#line 2736 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("classid"); }
#line 11377 "src/parser_bison.c"
    break;

  case 416: /* hook_spec: "type" close_scope_type "string" "hook" "string" dev_spec prio_spec  */
#line 2740 "src/parser_bison.y"
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
#line 11415 "src/parser_bison.c"
    break;

  case 417: /* prio_spec: "priority" extended_prio_spec  */
#line 2776 "src/parser_bison.y"
                        {
				(yyval.prio_spec) = (yyvsp[0].prio_spec);
				(yyval.prio_spec).loc = (yyloc);
			}
#line 11424 "src/parser_bison.c"
    break;

  case 418: /* extended_prio_name: "out"  */
#line 2783 "src/parser_bison.y"
                        {
				(yyval.string) = strdup("out");
			}
#line 11432 "src/parser_bison.c"
    break;

  case 420: /* extended_prio_spec: int_num  */
#line 2790 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &integer_type,
								BYTEORDER_HOST_ENDIAN,
								sizeof(int) *
								BITS_PER_BYTE, &(yyvsp[0].val32));
				(yyval.prio_spec) = spec;
			}
#line 11446 "src/parser_bison.c"
    break;

  case 421: /* extended_prio_spec: variable_expr  */
#line 2800 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = (yyvsp[0].expr);
				(yyval.prio_spec) = spec;
			}
#line 11457 "src/parser_bison.c"
    break;

  case 422: /* extended_prio_spec: extended_prio_name  */
#line 2807 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &string_type,
								BYTEORDER_HOST_ENDIAN,
								strlen((yyvsp[0].string)) * BITS_PER_BYTE,
								(yyvsp[0].string));
				free_const((yyvsp[0].string));
				(yyval.prio_spec) = spec;
			}
#line 11472 "src/parser_bison.c"
    break;

  case 423: /* extended_prio_spec: extended_prio_name "+" "number"  */
#line 2818 "src/parser_bison.y"
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
#line 11489 "src/parser_bison.c"
    break;

  case 424: /* extended_prio_spec: extended_prio_name "-" "number"  */
#line 2831 "src/parser_bison.y"
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
#line 11506 "src/parser_bison.c"
    break;

  case 425: /* int_num: "number"  */
#line 2845 "src/parser_bison.y"
                                                        { (yyval.val32) = (yyvsp[0].val); }
#line 11512 "src/parser_bison.c"
    break;

  case 426: /* int_num: "-" "number"  */
#line 2846 "src/parser_bison.y"
                                                        { (yyval.val32) = -(yyvsp[0].val); }
#line 11518 "src/parser_bison.c"
    break;

  case 427: /* dev_spec: "device" string  */
#line 2850 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), expr);

			}
#line 11533 "src/parser_bison.c"
    break;

  case 428: /* dev_spec: "device" variable_expr  */
#line 2861 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11543 "src/parser_bison.c"
    break;

  case 429: /* dev_spec: "devices" '=' flowtable_expr  */
#line 2867 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11551 "src/parser_bison.c"
    break;

  case 430: /* dev_spec: %empty  */
#line 2870 "src/parser_bison.y"
                                                        { (yyval.expr) = NULL; }
#line 11557 "src/parser_bison.c"
    break;

  case 431: /* flags_spec: "flags" "offload"  */
#line 2874 "src/parser_bison.y"
                        {
				(yyvsp[-2].chain)->flags |= CHAIN_F_HW_OFFLOAD;
			}
#line 11565 "src/parser_bison.c"
    break;

  case 432: /* policy_spec: "policy" policy_expr close_scope_policy  */
#line 2880 "src/parser_bison.y"
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
#line 11581 "src/parser_bison.c"
    break;

  case 433: /* policy_expr: variable_expr  */
#line 2894 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &policy_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11590 "src/parser_bison.c"
    break;

  case 434: /* policy_expr: chain_policy  */
#line 2899 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(int) *
							 BITS_PER_BYTE, &(yyvsp[0].val32));
			}
#line 11601 "src/parser_bison.c"
    break;

  case 435: /* chain_policy: "accept"  */
#line 2907 "src/parser_bison.y"
                                                { (yyval.val32) = NF_ACCEPT; }
#line 11607 "src/parser_bison.c"
    break;

  case 436: /* chain_policy: "drop"  */
#line 2908 "src/parser_bison.y"
                                                { (yyval.val32) = NF_DROP;   }
#line 11613 "src/parser_bison.c"
    break;

  case 438: /* identifier: "last"  */
#line 2912 "src/parser_bison.y"
                                                { (yyval.string) = xstrdup("last"); }
#line 11619 "src/parser_bison.c"
    break;

  case 442: /* time_spec: "string"  */
#line 2921 "src/parser_bison.y"
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
#line 11636 "src/parser_bison.c"
    break;

  case 444: /* time_spec_or_num_s: time_spec  */
#line 2937 "src/parser_bison.y"
                                          { (yyval.val) = (yyvsp[0].val) / 1000u; }
#line 11642 "src/parser_bison.c"
    break;

  case 445: /* family_spec: %empty  */
#line 2940 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11648 "src/parser_bison.c"
    break;

  case 447: /* family_spec_explicit: "ip" close_scope_ip  */
#line 2944 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11654 "src/parser_bison.c"
    break;

  case 448: /* family_spec_explicit: "ip6" close_scope_ip6  */
#line 2945 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 11660 "src/parser_bison.c"
    break;

  case 449: /* family_spec_explicit: "inet"  */
#line 2946 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_INET; }
#line 11666 "src/parser_bison.c"
    break;

  case 450: /* family_spec_explicit: "arp" close_scope_arp  */
#line 2947 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_ARP; }
#line 11672 "src/parser_bison.c"
    break;

  case 451: /* family_spec_explicit: "bridge"  */
#line 2948 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_BRIDGE; }
#line 11678 "src/parser_bison.c"
    break;

  case 452: /* family_spec_explicit: "netdev"  */
#line 2949 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_NETDEV; }
#line 11684 "src/parser_bison.c"
    break;

  case 453: /* table_spec: family_spec identifier  */
#line 2953 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[-1].val);
				(yyval.handle).table.location = (yylsp[0]);
				(yyval.handle).table.name	= (yyvsp[0].string);
			}
#line 11695 "src/parser_bison.c"
    break;

  case 454: /* tableid_spec: family_spec "handle" "number"  */
#line 2962 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family 		= (yyvsp[-2].val);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
				(yyval.handle).handle.location	= (yylsp[0]);
			}
#line 11706 "src/parser_bison.c"
    break;

  case 455: /* chain_spec: table_spec identifier  */
#line 2971 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).chain.name	= (yyvsp[0].string);
				(yyval.handle).chain.location = (yylsp[0]);
			}
#line 11716 "src/parser_bison.c"
    break;

  case 456: /* chainid_spec: table_spec "handle" "number"  */
#line 2979 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 11726 "src/parser_bison.c"
    break;

  case 457: /* chain_identifier: identifier  */
#line 2987 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).chain.name		= (yyvsp[0].string);
				(yyval.handle).chain.location	= (yylsp[0]);
			}
#line 11736 "src/parser_bison.c"
    break;

  case 458: /* set_spec: table_spec identifier  */
#line 2995 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 11746 "src/parser_bison.c"
    break;

  case 459: /* setid_spec: table_spec "handle" "number"  */
#line 3003 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 11756 "src/parser_bison.c"
    break;

  case 460: /* set_identifier: identifier  */
#line 3011 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 11766 "src/parser_bison.c"
    break;

  case 461: /* flowtable_spec: table_spec identifier  */
#line 3019 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-1].handle);
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 11776 "src/parser_bison.c"
    break;

  case 462: /* flowtableid_spec: table_spec "handle" "number"  */
#line 3027 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11786 "src/parser_bison.c"
    break;

  case 463: /* flowtable_identifier: identifier  */
#line 3035 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 11796 "src/parser_bison.c"
    break;

  case 464: /* obj_spec: table_spec identifier  */
#line 3043 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).obj.name	= (yyvsp[0].string);
				(yyval.handle).obj.location	= (yylsp[0]);
			}
#line 11806 "src/parser_bison.c"
    break;

  case 465: /* objid_spec: table_spec "handle" "number"  */
#line 3051 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11816 "src/parser_bison.c"
    break;

  case 466: /* obj_identifier: identifier  */
#line 3059 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).obj.name		= (yyvsp[0].string);
				(yyval.handle).obj.location		= (yylsp[0]);
			}
#line 11826 "src/parser_bison.c"
    break;

  case 467: /* handle_spec: "handle" "number"  */
#line 3067 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 11836 "src/parser_bison.c"
    break;

  case 468: /* position_spec: "position" "number"  */
#line 3075 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).position.location	= (yyloc);
				(yyval.handle).position.id		= (yyvsp[0].val);
			}
#line 11846 "src/parser_bison.c"
    break;

  case 469: /* index_spec: "index" "number"  */
#line 3083 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).index.location	= (yyloc);
				(yyval.handle).index.id		= (yyvsp[0].val) + 1;
			}
#line 11856 "src/parser_bison.c"
    break;

  case 470: /* rule_position: chain_spec  */
#line 3091 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 11864 "src/parser_bison.c"
    break;

  case 471: /* rule_position: chain_spec position_spec  */
#line 3095 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11873 "src/parser_bison.c"
    break;

  case 472: /* rule_position: chain_spec handle_spec  */
#line 3100 "src/parser_bison.y"
                        {
				(yyvsp[0].handle).position.location = (yyvsp[0].handle).handle.location;
				(yyvsp[0].handle).position.id = (yyvsp[0].handle).handle.id;
				(yyvsp[0].handle).handle.id = 0;
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11885 "src/parser_bison.c"
    break;

  case 473: /* rule_position: chain_spec index_spec  */
#line 3108 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11894 "src/parser_bison.c"
    break;

  case 474: /* ruleid_spec: chain_spec handle_spec  */
#line 3115 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 11903 "src/parser_bison.c"
    break;

  case 475: /* comment_spec: "comment" string  */
#line 3122 "src/parser_bison.y"
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
#line 11918 "src/parser_bison.c"
    break;

  case 476: /* ruleset_spec: %empty  */
#line 3135 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= NFPROTO_UNSPEC;
			}
#line 11927 "src/parser_bison.c"
    break;

  case 477: /* ruleset_spec: family_spec_explicit  */
#line 3140 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[0].val);
			}
#line 11936 "src/parser_bison.c"
    break;

  case 478: /* rule: rule_alloc  */
#line 3147 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = NULL;
			}
#line 11944 "src/parser_bison.c"
    break;

  case 479: /* rule: rule_alloc comment_spec  */
#line 3151 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = (yyvsp[0].string);
			}
#line 11952 "src/parser_bison.c"
    break;

  case 480: /* rule_alloc: stmt_list  */
#line 3157 "src/parser_bison.y"
                        {
				struct stmt *i;

				(yyval.rule) = rule_alloc(&(yyloc), NULL);
				list_for_each_entry(i, (yyvsp[0].list), list)
					(yyval.rule)->num_stmts++;
				list_splice_tail((yyvsp[0].list), &(yyval.rule)->stmts);
				free((yyvsp[0].list));
			}
#line 11966 "src/parser_bison.c"
    break;

  case 481: /* stmt_list: stmt  */
#line 3169 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 11976 "src/parser_bison.c"
    break;

  case 482: /* stmt_list: stmt_list stmt  */
#line 3175 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 11985 "src/parser_bison.c"
    break;

  case 483: /* stateful_stmt_list: stateful_stmt  */
#line 3182 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 11995 "src/parser_bison.c"
    break;

  case 484: /* stateful_stmt_list: stateful_stmt_list stateful_stmt  */
#line 3188 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 12004 "src/parser_bison.c"
    break;

  case 485: /* objref_stmt_counter: "counter" "name" stmt_expr close_scope_counter  */
#line 3195 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_COUNTER;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12014 "src/parser_bison.c"
    break;

  case 486: /* objref_stmt_limit: "limit" "name" stmt_expr close_scope_limit  */
#line 3203 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_LIMIT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12024 "src/parser_bison.c"
    break;

  case 487: /* objref_stmt_quota: "quota" "name" stmt_expr close_scope_quota  */
#line 3211 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_QUOTA;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12034 "src/parser_bison.c"
    break;

  case 488: /* objref_stmt_synproxy: "synproxy" "name" stmt_expr close_scope_synproxy  */
#line 3219 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_SYNPROXY;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12044 "src/parser_bison.c"
    break;

  case 489: /* objref_stmt_tunnel: "tunnel" "name" stmt_expr close_scope_tunnel  */
#line 3227 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_TUNNEL;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12054 "src/parser_bison.c"
    break;

  case 490: /* objref_stmt_ct: "ct" "timeout" "set" stmt_expr close_scope_ct  */
#line 3235 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_TIMEOUT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);

			}
#line 12065 "src/parser_bison.c"
    break;

  case 491: /* objref_stmt_ct: "ct" "expectation" "set" stmt_expr close_scope_ct  */
#line 3242 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_EXPECT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12075 "src/parser_bison.c"
    break;

  case 526: /* xt_stmt: "xt" "string" string  */
#line 3290 "src/parser_bison.y"
                        {
				(yyval.stmt) = NULL;
				free_const((yyvsp[-1].string));
				free_const((yyvsp[0].string));
				erec_queue(error(&(yyloc), "unsupported xtables compat expression, use iptables-nft with this ruleset"),
					   state->msgs);
				YYERROR;
			}
#line 12088 "src/parser_bison.c"
    break;

  case 527: /* chain_stmt_type: "jump"  */
#line 3300 "src/parser_bison.y"
                                        { (yyval.val) = NFT_JUMP; }
#line 12094 "src/parser_bison.c"
    break;

  case 528: /* chain_stmt_type: "goto"  */
#line 3301 "src/parser_bison.y"
                                        { (yyval.val) = NFT_GOTO; }
#line 12100 "src/parser_bison.c"
    break;

  case 529: /* chain_stmt: chain_stmt_type chain_block_alloc '{' subchain_block '}'  */
#line 3305 "src/parser_bison.y"
                        {
				(yyvsp[-3].chain)->location = (yylsp[-3]);
				close_scope(state);
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				(yyval.stmt) = chain_stmt_alloc(&(yyloc), (yyvsp[-1].chain), (yyvsp[-4].val));
			}
#line 12111 "src/parser_bison.c"
    break;

  case 530: /* verdict_stmt: verdict_expr  */
#line 3314 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 12119 "src/parser_bison.c"
    break;

  case 531: /* verdict_stmt: verdict_map_stmt  */
#line 3318 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 12127 "src/parser_bison.c"
    break;

  case 532: /* verdict_map_stmt: concat_expr "vmap" verdict_map_expr  */
#line 3324 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12135 "src/parser_bison.c"
    break;

  case 533: /* verdict_map_expr: '{' verdict_map_list_expr '}'  */
#line 3330 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 12144 "src/parser_bison.c"
    break;

  case 535: /* verdict_map_list_expr: verdict_map_list_member_expr  */
#line 3338 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				set_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 12153 "src/parser_bison.c"
    break;

  case 536: /* verdict_map_list_expr: verdict_map_list_expr "comma" verdict_map_list_member_expr  */
#line 3343 "src/parser_bison.y"
                        {
				set_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 12162 "src/parser_bison.c"
    break;

  case 538: /* verdict_map_list_member_expr: opt_newline set_elem_expr "colon" verdict_expr opt_newline  */
#line 3351 "src/parser_bison.y"
                        {
				(yyval.expr) = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr), (yyvsp[-1].expr));
			}
#line 12170 "src/parser_bison.c"
    break;

  case 539: /* ct_limit_stmt_alloc: "ct" "count"  */
#line 3357 "src/parser_bison.y"
                        {
				(yyval.stmt) = connlimit_stmt_alloc(&(yyloc));
			}
#line 12178 "src/parser_bison.c"
    break;

  case 541: /* ct_limit_args: "number"  */
#line 3366 "src/parser_bison.y"
                        {
				assert((yyvsp[-1].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-1].stmt)->connlimit.count	= (yyvsp[0].val);
			}
#line 12188 "src/parser_bison.c"
    break;

  case 542: /* ct_limit_args: "over" "number"  */
#line 3372 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-2].stmt)->connlimit.count = (yyvsp[0].val);
				(yyvsp[-2].stmt)->connlimit.flags = NFT_CONNLIMIT_F_INV;
			}
#line 12199 "src/parser_bison.c"
    break;

  case 545: /* counter_stmt_alloc: "counter"  */
#line 3384 "src/parser_bison.y"
                        {
				(yyval.stmt) = counter_stmt_alloc(&(yyloc));
			}
#line 12207 "src/parser_bison.c"
    break;

  case 546: /* counter_args: counter_arg  */
#line 3390 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12215 "src/parser_bison.c"
    break;

  case 548: /* counter_arg: "packets" "number"  */
#line 3397 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.packets = (yyvsp[0].val);
			}
#line 12224 "src/parser_bison.c"
    break;

  case 549: /* counter_arg: "bytes" "number"  */
#line 3402 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.bytes	 = (yyvsp[0].val);
			}
#line 12233 "src/parser_bison.c"
    break;

  case 550: /* last_stmt_alloc: "last"  */
#line 3409 "src/parser_bison.y"
                        {
				(yyval.stmt) = last_stmt_alloc(&(yyloc));
			}
#line 12241 "src/parser_bison.c"
    break;

  case 554: /* last_args: "used" time_spec  */
#line 3420 "src/parser_bison.y"
                        {
				struct last_stmt *last;

				assert((yyvsp[-2].stmt)->type == STMT_LAST);
				last = &(yyvsp[-2].stmt)->last;
				last->used = (yyvsp[0].val);
				last->set = true;
			}
#line 12254 "src/parser_bison.c"
    break;

  case 557: /* log_stmt_alloc: "log"  */
#line 3435 "src/parser_bison.y"
                        {
				(yyval.stmt) = log_stmt_alloc(&(yyloc));
			}
#line 12262 "src/parser_bison.c"
    break;

  case 558: /* log_args: log_arg  */
#line 3441 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12270 "src/parser_bison.c"
    break;

  case 560: /* log_arg: "prefix" string  */
#line 3448 "src/parser_bison.y"
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
#line 12291 "src/parser_bison.c"
    break;

  case 561: /* log_arg: "group" "number"  */
#line 3465 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.group	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_GROUP;
			}
#line 12300 "src/parser_bison.c"
    break;

  case 562: /* log_arg: "snaplen" "number"  */
#line 3470 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.snaplen	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_SNAPLEN;
			}
#line 12309 "src/parser_bison.c"
    break;

  case 563: /* log_arg: "queue-threshold" "number"  */
#line 3475 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.qthreshold = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_QTHRESHOLD;
			}
#line 12318 "src/parser_bison.c"
    break;

  case 564: /* log_arg: "level" level_type  */
#line 3480 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.level	= (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_LEVEL;
			}
#line 12327 "src/parser_bison.c"
    break;

  case 565: /* log_arg: "flags" log_flags  */
#line 3485 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.logflags	|= (yyvsp[0].val);
			}
#line 12335 "src/parser_bison.c"
    break;

  case 566: /* level_type: string  */
#line 3491 "src/parser_bison.y"
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
#line 12367 "src/parser_bison.c"
    break;

  case 567: /* log_flags: "tcp" log_flags_tcp close_scope_tcp  */
#line 3521 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-1].val);
			}
#line 12375 "src/parser_bison.c"
    break;

  case 568: /* log_flags: "ip" "options" close_scope_ip  */
#line 3525 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_IPOPT;
			}
#line 12383 "src/parser_bison.c"
    break;

  case 569: /* log_flags: "skuid"  */
#line 3529 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_UID;
			}
#line 12391 "src/parser_bison.c"
    break;

  case 570: /* log_flags: "ether" close_scope_eth  */
#line 3533 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MACDECODE;
			}
#line 12399 "src/parser_bison.c"
    break;

  case 571: /* log_flags: "all"  */
#line 3537 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MASK;
			}
#line 12407 "src/parser_bison.c"
    break;

  case 572: /* log_flags_tcp: log_flags_tcp "comma" log_flag_tcp  */
#line 3543 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 12415 "src/parser_bison.c"
    break;

  case 574: /* log_flag_tcp: "seq"  */
#line 3550 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPSEQ;
			}
#line 12423 "src/parser_bison.c"
    break;

  case 575: /* log_flag_tcp: "options"  */
#line 3554 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPOPT;
			}
#line 12431 "src/parser_bison.c"
    break;

  case 576: /* limit_stmt_alloc: "limit" "rate"  */
#line 3560 "src/parser_bison.y"
                        {
				(yyval.stmt) = limit_stmt_alloc(&(yyloc));
			}
#line 12439 "src/parser_bison.c"
    break;

  case 578: /* limit_args: limit_mode limit_rate_pkts limit_burst_pkts  */
#line 3569 "src/parser_bison.y"
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
#line 12461 "src/parser_bison.c"
    break;

  case 579: /* limit_args: limit_mode limit_rate_bytes limit_burst_bytes  */
#line 3587 "src/parser_bison.y"
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
#line 12478 "src/parser_bison.c"
    break;

  case 580: /* quota_mode: "over"  */
#line 3601 "src/parser_bison.y"
                                                { (yyval.val) = NFT_QUOTA_F_INV; }
#line 12484 "src/parser_bison.c"
    break;

  case 581: /* quota_mode: "until"  */
#line 3602 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12490 "src/parser_bison.c"
    break;

  case 582: /* quota_mode: %empty  */
#line 3603 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12496 "src/parser_bison.c"
    break;

  case 583: /* quota_unit: "bytes"  */
#line 3606 "src/parser_bison.y"
                                                { (yyval.string) = xstrdup("bytes"); }
#line 12502 "src/parser_bison.c"
    break;

  case 584: /* quota_unit: "string"  */
#line 3607 "src/parser_bison.y"
                                                { (yyval.string) = (yyvsp[0].string); }
#line 12508 "src/parser_bison.c"
    break;

  case 585: /* quota_used: %empty  */
#line 3610 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12514 "src/parser_bison.c"
    break;

  case 586: /* quota_used: "used" "number" quota_unit  */
#line 3612 "src/parser_bison.y"
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
#line 12531 "src/parser_bison.c"
    break;

  case 587: /* quota_stmt_alloc: "quota"  */
#line 3627 "src/parser_bison.y"
                        {
				(yyval.stmt) = quota_stmt_alloc(&(yyloc));
			}
#line 12539 "src/parser_bison.c"
    break;

  case 589: /* quota_args: quota_mode "number" quota_unit quota_used  */
#line 3636 "src/parser_bison.y"
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
#line 12562 "src/parser_bison.c"
    break;

  case 590: /* limit_mode: "over"  */
#line 3656 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_LIMIT_F_INV; }
#line 12568 "src/parser_bison.c"
    break;

  case 591: /* limit_mode: "until"  */
#line 3657 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12574 "src/parser_bison.c"
    break;

  case 592: /* limit_mode: %empty  */
#line 3658 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12580 "src/parser_bison.c"
    break;

  case 593: /* limit_burst_pkts: %empty  */
#line 3661 "src/parser_bison.y"
                                                                { (yyval.val) = 5; }
#line 12586 "src/parser_bison.c"
    break;

  case 594: /* limit_burst_pkts: "burst" "number" "packets"  */
#line 3662 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[-1].val); }
#line 12592 "src/parser_bison.c"
    break;

  case 595: /* limit_rate_pkts: "number" "/" time_unit  */
#line 3666 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12601 "src/parser_bison.c"
    break;

  case 596: /* limit_burst_bytes: %empty  */
#line 3672 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12607 "src/parser_bison.c"
    break;

  case 597: /* limit_burst_bytes: "burst" limit_bytes  */
#line 3673 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[0].val); }
#line 12613 "src/parser_bison.c"
    break;

  case 598: /* limit_rate_bytes: "number" "string"  */
#line 3677 "src/parser_bison.y"
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
#line 12631 "src/parser_bison.c"
    break;

  case 599: /* limit_rate_bytes: limit_bytes "/" time_unit  */
#line 3691 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12640 "src/parser_bison.c"
    break;

  case 600: /* limit_bytes: "number" "bytes"  */
#line 3697 "src/parser_bison.y"
                                                        { (yyval.val) = (yyvsp[-1].val); }
#line 12646 "src/parser_bison.c"
    break;

  case 601: /* limit_bytes: "number" "string"  */
#line 3699 "src/parser_bison.y"
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
#line 12663 "src/parser_bison.c"
    break;

  case 602: /* time_unit: "second"  */
#line 3713 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL; }
#line 12669 "src/parser_bison.c"
    break;

  case 603: /* time_unit: "minute"  */
#line 3714 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60; }
#line 12675 "src/parser_bison.c"
    break;

  case 604: /* time_unit: "hour"  */
#line 3715 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60; }
#line 12681 "src/parser_bison.c"
    break;

  case 605: /* time_unit: "day"  */
#line 3716 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24; }
#line 12687 "src/parser_bison.c"
    break;

  case 606: /* time_unit: "week"  */
#line 3717 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24 * 7; }
#line 12693 "src/parser_bison.c"
    break;

  case 608: /* reject_stmt_alloc: "reject"  */
#line 3724 "src/parser_bison.y"
                        {
				(yyval.stmt) = reject_stmt_alloc(&(yyloc));
			}
#line 12701 "src/parser_bison.c"
    break;

  case 609: /* reject_with_expr: "string"  */
#line 3730 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state), (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 12711 "src/parser_bison.c"
    break;

  case 610: /* reject_with_expr: integer_expr  */
#line 3735 "src/parser_bison.y"
                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12717 "src/parser_bison.c"
    break;

  case 611: /* reject_opts: %empty  */
#line 3739 "src/parser_bison.y"
                        {
				(yyvsp[0].stmt)->reject.type = -1;
				(yyvsp[0].stmt)->reject.icmp_code = -1;
			}
#line 12726 "src/parser_bison.c"
    break;

  case 612: /* reject_opts: "with" "icmp" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3744 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 12737 "src/parser_bison.c"
    break;

  case 613: /* reject_opts: "with" "icmp" reject_with_expr  */
#line 3751 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 12748 "src/parser_bison.c"
    break;

  case 614: /* reject_opts: "with" "icmpv6" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3758 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 12759 "src/parser_bison.c"
    break;

  case 615: /* reject_opts: "with" "icmpv6" reject_with_expr  */
#line 3765 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 12770 "src/parser_bison.c"
    break;

  case 616: /* reject_opts: "with" "icmpx" "type" reject_with_expr close_scope_type  */
#line 3772 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-5].stmt)->reject.expr = (yyvsp[-1].expr);
				datatype_set((yyvsp[-5].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 12780 "src/parser_bison.c"
    break;

  case 617: /* reject_opts: "with" "icmpx" reject_with_expr  */
#line 3778 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 12790 "src/parser_bison.c"
    break;

  case 618: /* reject_opts: "with" "tcp" close_scope_tcp "reset" close_scope_reset  */
#line 3784 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_TCP_RST;
			}
#line 12798 "src/parser_bison.c"
    break;

  case 620: /* nat_stmt_alloc: "snat"  */
#line 3792 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_SNAT); }
#line 12804 "src/parser_bison.c"
    break;

  case 621: /* nat_stmt_alloc: "dnat"  */
#line 3793 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_DNAT); }
#line 12810 "src/parser_bison.c"
    break;

  case 622: /* tproxy_stmt: "tproxy" "to" stmt_expr  */
#line 3797 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 12820 "src/parser_bison.c"
    break;

  case 623: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr  */
#line 3803 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-2].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 12830 "src/parser_bison.c"
    break;

  case 624: /* tproxy_stmt: "tproxy" "to" "colon" stmt_expr  */
#line 3809 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12840 "src/parser_bison.c"
    break;

  case 625: /* tproxy_stmt: "tproxy" "to" stmt_expr "colon" stmt_expr  */
#line 3815 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12851 "src/parser_bison.c"
    break;

  case 626: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 3822 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-4].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12862 "src/parser_bison.c"
    break;

  case 627: /* tproxy_stmt: "tproxy" nf_key_proto "to" "colon" stmt_expr  */
#line 3829 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-3].val);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 12872 "src/parser_bison.c"
    break;

  case 630: /* synproxy_stmt_alloc: "synproxy"  */
#line 3841 "src/parser_bison.y"
                        {
				(yyval.stmt) = synproxy_stmt_alloc(&(yyloc));
			}
#line 12880 "src/parser_bison.c"
    break;

  case 631: /* synproxy_args: synproxy_arg  */
#line 3847 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12888 "src/parser_bison.c"
    break;

  case 633: /* synproxy_arg: "mss" "number"  */
#line 3854 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->synproxy.mss = (yyvsp[0].val);
				(yyvsp[-2].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_MSS;
			}
#line 12897 "src/parser_bison.c"
    break;

  case 634: /* synproxy_arg: "wscale" "number"  */
#line 3859 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->synproxy.wscale = (yyvsp[0].val);
				(yyvsp[-2].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_WSCALE;
			}
#line 12906 "src/parser_bison.c"
    break;

  case 635: /* synproxy_arg: "timestamp"  */
#line 3864 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 12914 "src/parser_bison.c"
    break;

  case 636: /* synproxy_arg: "sack-permitted"  */
#line 3868 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 12922 "src/parser_bison.c"
    break;

  case 637: /* synproxy_config: "mss" "number" "wscale" "number" synproxy_ts synproxy_sack  */
#line 3874 "src/parser_bison.y"
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
#line 12942 "src/parser_bison.c"
    break;

  case 638: /* synproxy_config: "mss" "number" stmt_separator "wscale" "number" stmt_separator synproxy_ts synproxy_sack  */
#line 3890 "src/parser_bison.y"
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
#line 12962 "src/parser_bison.c"
    break;

  case 639: /* synproxy_obj: %empty  */
#line 3908 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SYNPROXY;
			}
#line 12971 "src/parser_bison.c"
    break;

  case 640: /* synproxy_ts: %empty  */
#line 3914 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12977 "src/parser_bison.c"
    break;

  case 641: /* synproxy_ts: "timestamp"  */
#line 3916 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 12985 "src/parser_bison.c"
    break;

  case 642: /* synproxy_sack: %empty  */
#line 3921 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12991 "src/parser_bison.c"
    break;

  case 643: /* synproxy_sack: "sack-permitted"  */
#line 3923 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 12999 "src/parser_bison.c"
    break;

  case 644: /* primary_stmt_expr: symbol_expr  */
#line 3928 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13005 "src/parser_bison.c"
    break;

  case 645: /* primary_stmt_expr: integer_expr  */
#line 3929 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13011 "src/parser_bison.c"
    break;

  case 646: /* primary_stmt_expr: boolean_expr  */
#line 3930 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13017 "src/parser_bison.c"
    break;

  case 647: /* primary_stmt_expr: meta_expr  */
#line 3931 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13023 "src/parser_bison.c"
    break;

  case 648: /* primary_stmt_expr: rt_expr  */
#line 3932 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13029 "src/parser_bison.c"
    break;

  case 649: /* primary_stmt_expr: tunnel_expr  */
#line 3933 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13035 "src/parser_bison.c"
    break;

  case 650: /* primary_stmt_expr: ct_expr  */
#line 3934 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13041 "src/parser_bison.c"
    break;

  case 651: /* primary_stmt_expr: numgen_expr  */
#line 3935 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13047 "src/parser_bison.c"
    break;

  case 652: /* primary_stmt_expr: hash_expr  */
#line 3936 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13053 "src/parser_bison.c"
    break;

  case 653: /* primary_stmt_expr: payload_expr  */
#line 3937 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13059 "src/parser_bison.c"
    break;

  case 654: /* primary_stmt_expr: keyword_expr  */
#line 3938 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13065 "src/parser_bison.c"
    break;

  case 655: /* primary_stmt_expr: socket_expr  */
#line 3939 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13071 "src/parser_bison.c"
    break;

  case 656: /* primary_stmt_expr: fib_expr  */
#line 3940 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13077 "src/parser_bison.c"
    break;

  case 657: /* primary_stmt_expr: osf_expr  */
#line 3941 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13083 "src/parser_bison.c"
    break;

  case 658: /* primary_stmt_expr: '(' basic_stmt_expr ')'  */
#line 3942 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 13089 "src/parser_bison.c"
    break;

  case 660: /* shift_stmt_expr: shift_stmt_expr "<<" primary_stmt_expr  */
#line 3947 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13097 "src/parser_bison.c"
    break;

  case 661: /* shift_stmt_expr: shift_stmt_expr ">>" primary_stmt_expr  */
#line 3951 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13105 "src/parser_bison.c"
    break;

  case 663: /* and_stmt_expr: and_stmt_expr "&" shift_stmt_expr  */
#line 3958 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13113 "src/parser_bison.c"
    break;

  case 665: /* exclusive_or_stmt_expr: exclusive_or_stmt_expr "^" and_stmt_expr  */
#line 3965 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13121 "src/parser_bison.c"
    break;

  case 667: /* inclusive_or_stmt_expr: inclusive_or_stmt_expr '|' exclusive_or_stmt_expr  */
#line 3972 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13129 "src/parser_bison.c"
    break;

  case 670: /* concat_stmt_expr: concat_stmt_expr "." primary_stmt_expr  */
#line 3982 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 13142 "src/parser_bison.c"
    break;

  case 673: /* map_stmt_expr: concat_stmt_expr "map" map_stmt_expr_set  */
#line 3997 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13150 "src/parser_bison.c"
    break;

  case 674: /* map_stmt_expr: concat_stmt_expr  */
#line 4000 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 13156 "src/parser_bison.c"
    break;

  case 675: /* prefix_stmt_expr: basic_stmt_expr "/" "number"  */
#line 4004 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 13164 "src/parser_bison.c"
    break;

  case 676: /* range_stmt_expr: basic_stmt_expr "-" basic_stmt_expr  */
#line 4010 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13172 "src/parser_bison.c"
    break;

  case 682: /* nat_stmt_args: stmt_expr  */
#line 4025 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13180 "src/parser_bison.c"
    break;

  case 683: /* nat_stmt_args: "to" stmt_expr  */
#line 4029 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13188 "src/parser_bison.c"
    break;

  case 684: /* nat_stmt_args: nf_key_proto "to" stmt_expr  */
#line 4033 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.family = (yyvsp[-2].val);
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13197 "src/parser_bison.c"
    break;

  case 685: /* nat_stmt_args: stmt_expr "colon" stmt_expr  */
#line 4038 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13206 "src/parser_bison.c"
    break;

  case 686: /* nat_stmt_args: "to" stmt_expr "colon" stmt_expr  */
#line 4043 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13215 "src/parser_bison.c"
    break;

  case 687: /* nat_stmt_args: nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 4048 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->nat.family = (yyvsp[-4].val);
				(yyvsp[-5].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-5].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13225 "src/parser_bison.c"
    break;

  case 688: /* nat_stmt_args: "colon" stmt_expr  */
#line 4054 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13233 "src/parser_bison.c"
    break;

  case 689: /* nat_stmt_args: "to" "colon" stmt_expr  */
#line 4058 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13241 "src/parser_bison.c"
    break;

  case 690: /* nat_stmt_args: nat_stmt_args nf_nat_flags  */
#line 4062 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13249 "src/parser_bison.c"
    break;

  case 691: /* nat_stmt_args: nf_key_proto "addr" "." "port" "to" stmt_expr  */
#line 4066 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->nat.family = (yyvsp[-5].val);
				(yyvsp[-6].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-6].stmt)->nat.type_flags = STMT_NAT_F_CONCAT;
			}
#line 13259 "src/parser_bison.c"
    break;

  case 692: /* nat_stmt_args: nf_key_proto "interval" "to" stmt_expr  */
#line 4072 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13268 "src/parser_bison.c"
    break;

  case 693: /* nat_stmt_args: "interval" "to" stmt_expr  */
#line 4077 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13276 "src/parser_bison.c"
    break;

  case 694: /* nat_stmt_args: nf_key_proto "prefix" "to" stmt_expr  */
#line 4081 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-4].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-4].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 13288 "src/parser_bison.c"
    break;

  case 695: /* nat_stmt_args: "prefix" "to" stmt_expr  */
#line 4089 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-3].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-3].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 13299 "src/parser_bison.c"
    break;

  case 698: /* masq_stmt_alloc: "masquerade"  */
#line 4101 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_MASQ); }
#line 13305 "src/parser_bison.c"
    break;

  case 699: /* masq_stmt_args: "to" "colon" stmt_expr  */
#line 4105 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13313 "src/parser_bison.c"
    break;

  case 700: /* masq_stmt_args: "to" "colon" stmt_expr nf_nat_flags  */
#line 4109 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13322 "src/parser_bison.c"
    break;

  case 701: /* masq_stmt_args: nf_nat_flags  */
#line 4114 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13330 "src/parser_bison.c"
    break;

  case 704: /* redir_stmt_alloc: "redirect"  */
#line 4123 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_REDIR); }
#line 13336 "src/parser_bison.c"
    break;

  case 705: /* redir_stmt_arg: "to" stmt_expr  */
#line 4127 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13344 "src/parser_bison.c"
    break;

  case 706: /* redir_stmt_arg: "to" "colon" stmt_expr  */
#line 4131 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13352 "src/parser_bison.c"
    break;

  case 707: /* redir_stmt_arg: nf_nat_flags  */
#line 4135 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13360 "src/parser_bison.c"
    break;

  case 708: /* redir_stmt_arg: "to" stmt_expr nf_nat_flags  */
#line 4139 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-3].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13369 "src/parser_bison.c"
    break;

  case 709: /* redir_stmt_arg: "to" "colon" stmt_expr nf_nat_flags  */
#line 4144 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13378 "src/parser_bison.c"
    break;

  case 710: /* dup_stmt: "dup" "to" stmt_expr  */
#line 4151 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[0].expr);
			}
#line 13387 "src/parser_bison.c"
    break;

  case 711: /* dup_stmt: "dup" "to" stmt_expr "device" stmt_expr  */
#line 4156 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[-2].expr);
				(yyval.stmt)->dup.dev = (yyvsp[0].expr);
			}
#line 13397 "src/parser_bison.c"
    break;

  case 712: /* fwd_stmt: "fwd" "to" stmt_expr  */
#line 4164 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13406 "src/parser_bison.c"
    break;

  case 713: /* fwd_stmt: "fwd" nf_key_proto "to" stmt_expr "device" stmt_expr  */
#line 4169 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.family = (yyvsp[-4].val);
				(yyval.stmt)->fwd.addr = (yyvsp[-2].expr);
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13417 "src/parser_bison.c"
    break;

  case 715: /* nf_nat_flags: nf_nat_flags "comma" nf_nat_flag  */
#line 4179 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13425 "src/parser_bison.c"
    break;

  case 716: /* nf_nat_flag: "random"  */
#line 4184 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM; }
#line 13431 "src/parser_bison.c"
    break;

  case 717: /* nf_nat_flag: "fully-random"  */
#line 4185 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM_FULLY; }
#line 13437 "src/parser_bison.c"
    break;

  case 718: /* nf_nat_flag: "persistent"  */
#line 4186 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PERSISTENT; }
#line 13443 "src/parser_bison.c"
    break;

  case 720: /* queue_stmt: "queue" "to" queue_stmt_expr close_scope_queue  */
#line 4191 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), 0);
			}
#line 13451 "src/parser_bison.c"
    break;

  case 721: /* queue_stmt: "queue" "flags" queue_stmt_flags "to" queue_stmt_expr close_scope_queue  */
#line 4195 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13459 "src/parser_bison.c"
    break;

  case 722: /* queue_stmt: "queue" "flags" queue_stmt_flags "num" queue_stmt_expr_simple close_scope_queue  */
#line 4199 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13467 "src/parser_bison.c"
    break;

  case 725: /* queue_stmt_alloc: "queue"  */
#line 4209 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), NULL, 0);
			}
#line 13475 "src/parser_bison.c"
    break;

  case 726: /* queue_stmt_args: queue_stmt_arg  */
#line 4215 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 13483 "src/parser_bison.c"
    break;

  case 728: /* queue_stmt_arg: "num" queue_stmt_expr_simple  */
#line 4222 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->queue.queue = (yyvsp[0].expr);
				(yyvsp[-2].stmt)->queue.queue->location = (yyloc);
			}
#line 13492 "src/parser_bison.c"
    break;

  case 729: /* queue_stmt_arg: queue_stmt_flags  */
#line 4227 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->queue.flags |= (yyvsp[0].val);
			}
#line 13500 "src/parser_bison.c"
    break;

  case 734: /* queue_stmt_expr_simple: queue_expr "-" queue_expr  */
#line 4239 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13508 "src/parser_bison.c"
    break;

  case 740: /* queue_stmt_flags: queue_stmt_flags "comma" queue_stmt_flag  */
#line 4252 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13516 "src/parser_bison.c"
    break;

  case 741: /* queue_stmt_flag: "bypass"  */
#line 4257 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_BYPASS; }
#line 13522 "src/parser_bison.c"
    break;

  case 742: /* queue_stmt_flag: "fanout"  */
#line 4258 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_CPU_FANOUT; }
#line 13528 "src/parser_bison.c"
    break;

  case 745: /* set_elem_expr_stmt_alloc: concat_expr  */
#line 4266 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 13536 "src/parser_bison.c"
    break;

  case 746: /* set_stmt: "set" set_stmt_op set_elem_expr_stmt set_ref_expr  */
#line 4272 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-2].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[0].expr);
			}
#line 13547 "src/parser_bison.c"
    break;

  case 747: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt '}'  */
#line 4279 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-4].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[-3].expr);
			}
#line 13558 "src/parser_bison.c"
    break;

  case 748: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list '}'  */
#line 4286 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-5].val);
				(yyval.stmt)->set.key = (yyvsp[-2].expr);
				(yyval.stmt)->set.set = (yyvsp[-4].expr);
				list_splice_tail((yyvsp[-1].list), &(yyval.stmt)->set.stmt_list);
				free((yyvsp[-1].list));
			}
#line 13571 "src/parser_bison.c"
    break;

  case 749: /* set_stmt_op: "add"  */
#line 4296 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_ADD; }
#line 13577 "src/parser_bison.c"
    break;

  case 750: /* set_stmt_op: "update"  */
#line 4297 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_UPDATE; }
#line 13583 "src/parser_bison.c"
    break;

  case 751: /* set_stmt_op: "delete"  */
#line 4298 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_DELETE; }
#line 13589 "src/parser_bison.c"
    break;

  case 752: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt "colon" set_elem_expr_stmt '}'  */
#line 4302 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-6].val);
				(yyval.stmt)->map.key = (yyvsp[-3].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-5].expr);
			}
#line 13601 "src/parser_bison.c"
    break;

  case 753: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list "colon" set_elem_expr_stmt '}'  */
#line 4310 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-7].val);
				(yyval.stmt)->map.key = (yyvsp[-4].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-6].expr);
				list_splice_tail((yyvsp[-3].list), &(yyval.stmt)->map.stmt_list);
				free((yyvsp[-3].list));
			}
#line 13615 "src/parser_bison.c"
    break;

  case 754: /* meter_stmt: "meter" identifier '{' meter_key_expr stmt '}'  */
#line 4322 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-4].string);
				(yyval.stmt)->meter.size = 0;
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13628 "src/parser_bison.c"
    break;

  case 755: /* meter_stmt: "meter" identifier "size" "number" '{' meter_key_expr stmt '}'  */
#line 4331 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-6].string);
				(yyval.stmt)->meter.size = (yyvsp[-4].val);
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13641 "src/parser_bison.c"
    break;

  case 756: /* match_stmt: relational_expr  */
#line 4342 "src/parser_bison.y"
                        {
				(yyval.stmt) = expr_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 13649 "src/parser_bison.c"
    break;

  case 757: /* variable_expr: '$' identifier  */
#line 4348 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);
				struct symbol *sym;

				sym = symbol_get(scope, (yyvsp[0].string));
				if (!sym) {
					sym = symbol_lookup_fuzzy(scope, (yyvsp[0].string));
					if (sym) {
						erec_queue(error(&(yylsp[0]), "unknown identifier '%s'; "
								      "did you mean identifier '%s'?",
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
#line 13677 "src/parser_bison.c"
    break;

  case 759: /* symbol_expr: string  */
#line 4375 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 13688 "src/parser_bison.c"
    break;

  case 762: /* set_ref_symbol_expr: "@" identifier close_scope_at  */
#line 4388 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_SET,
						       current_scope(state),
						       (yyvsp[-1].string));
				free_const((yyvsp[-1].string));
			}
#line 13699 "src/parser_bison.c"
    break;

  case 763: /* integer_expr: "number"  */
#line 4397 "src/parser_bison.y"
                        {
				char str[64];

				snprintf(str, sizeof(str), "%" PRIu64, (yyvsp[0].val));
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       str);
			}
#line 13712 "src/parser_bison.c"
    break;

  case 764: /* selector_expr: payload_expr  */
#line 4407 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13718 "src/parser_bison.c"
    break;

  case 765: /* selector_expr: exthdr_expr  */
#line 4408 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13724 "src/parser_bison.c"
    break;

  case 766: /* selector_expr: exthdr_exists_expr  */
#line 4409 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13730 "src/parser_bison.c"
    break;

  case 767: /* selector_expr: meta_expr  */
#line 4410 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13736 "src/parser_bison.c"
    break;

  case 768: /* selector_expr: tunnel_expr  */
#line 4411 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13742 "src/parser_bison.c"
    break;

  case 769: /* selector_expr: socket_expr  */
#line 4412 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13748 "src/parser_bison.c"
    break;

  case 770: /* selector_expr: rt_expr  */
#line 4413 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13754 "src/parser_bison.c"
    break;

  case 771: /* selector_expr: ct_expr  */
#line 4414 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13760 "src/parser_bison.c"
    break;

  case 772: /* selector_expr: numgen_expr  */
#line 4415 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13766 "src/parser_bison.c"
    break;

  case 773: /* selector_expr: hash_expr  */
#line 4416 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13772 "src/parser_bison.c"
    break;

  case 774: /* selector_expr: fib_expr  */
#line 4417 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13778 "src/parser_bison.c"
    break;

  case 775: /* selector_expr: osf_expr  */
#line 4418 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13784 "src/parser_bison.c"
    break;

  case 776: /* selector_expr: xfrm_expr  */
#line 4419 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13790 "src/parser_bison.c"
    break;

  case 777: /* primary_expr: symbol_expr  */
#line 4422 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13796 "src/parser_bison.c"
    break;

  case 778: /* primary_expr: integer_expr  */
#line 4423 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13802 "src/parser_bison.c"
    break;

  case 779: /* primary_expr: selector_expr  */
#line 4424 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13808 "src/parser_bison.c"
    break;

  case 780: /* primary_expr: '(' basic_expr ')'  */
#line 4425 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 13814 "src/parser_bison.c"
    break;

  case 781: /* fib_expr: "fib" fib_tuple fib_result close_scope_fib  */
#line 4429 "src/parser_bison.y"
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
#line 13846 "src/parser_bison.c"
    break;

  case 782: /* fib_result: "oif"  */
#line 4458 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIF; }
#line 13852 "src/parser_bison.c"
    break;

  case 783: /* fib_result: "oifname"  */
#line 4459 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIFNAME; }
#line 13858 "src/parser_bison.c"
    break;

  case 784: /* fib_result: "type" close_scope_type  */
#line 4460 "src/parser_bison.y"
                                                                { (yyval.val) =NFT_FIB_RESULT_ADDRTYPE; }
#line 13864 "src/parser_bison.c"
    break;

  case 785: /* fib_result: "check"  */
#line 4461 "src/parser_bison.y"
                                        { (yyval.val) = __NFT_FIB_RESULT_MAX; }
#line 13870 "src/parser_bison.c"
    break;

  case 786: /* fib_flag: "saddr"  */
#line 4464 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_SADDR; }
#line 13876 "src/parser_bison.c"
    break;

  case 787: /* fib_flag: "daddr"  */
#line 4465 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_DADDR; }
#line 13882 "src/parser_bison.c"
    break;

  case 788: /* fib_flag: "mark"  */
#line 4466 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_MARK; }
#line 13888 "src/parser_bison.c"
    break;

  case 789: /* fib_flag: "iif"  */
#line 4467 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_IIF; }
#line 13894 "src/parser_bison.c"
    break;

  case 790: /* fib_flag: "oif"  */
#line 4468 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_OIF; }
#line 13900 "src/parser_bison.c"
    break;

  case 791: /* fib_tuple: fib_flag "." fib_tuple  */
#line 4472 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13908 "src/parser_bison.c"
    break;

  case 793: /* osf_expr: "osf" osf_ttl "version" close_scope_osf  */
#line 4479 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), NFT_OSF_F_VERSION);
			}
#line 13916 "src/parser_bison.c"
    break;

  case 794: /* osf_expr: "osf" osf_ttl "name" close_scope_osf  */
#line 4483 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), 0);
			}
#line 13924 "src/parser_bison.c"
    break;

  case 795: /* osf_ttl: %empty  */
#line 4489 "src/parser_bison.y"
                        {
				(yyval.val) = NF_OSF_TTL_TRUE;
			}
#line 13932 "src/parser_bison.c"
    break;

  case 796: /* osf_ttl: "ttl" "string"  */
#line 4493 "src/parser_bison.y"
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
#line 13950 "src/parser_bison.c"
    break;

  case 798: /* shift_expr: shift_expr "<<" primary_rhs_expr  */
#line 4510 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13958 "src/parser_bison.c"
    break;

  case 799: /* shift_expr: shift_expr ">>" primary_rhs_expr  */
#line 4514 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13966 "src/parser_bison.c"
    break;

  case 801: /* and_expr: and_expr "&" shift_rhs_expr  */
#line 4521 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13974 "src/parser_bison.c"
    break;

  case 803: /* exclusive_or_expr: exclusive_or_expr "^" and_rhs_expr  */
#line 4528 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13982 "src/parser_bison.c"
    break;

  case 805: /* inclusive_or_expr: inclusive_or_expr '|' exclusive_or_rhs_expr  */
#line 4535 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13990 "src/parser_bison.c"
    break;

  case 808: /* concat_expr: concat_expr "." basic_expr  */
#line 4545 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 14003 "src/parser_bison.c"
    break;

  case 809: /* prefix_rhs_expr: basic_rhs_expr "/" "number"  */
#line 4556 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 14011 "src/parser_bison.c"
    break;

  case 810: /* range_rhs_expr: basic_rhs_expr "-" basic_rhs_expr  */
#line 4562 "src/parser_bison.y"
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
#line 14026 "src/parser_bison.c"
    break;

  case 813: /* map_expr: concat_expr "map" rhs_expr  */
#line 4579 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14034 "src/parser_bison.c"
    break;

  case 817: /* set_expr: '{' set_list_expr '}'  */
#line 4590 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14043 "src/parser_bison.c"
    break;

  case 818: /* set_list_expr: set_list_member_expr  */
#line 4597 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				set_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 14052 "src/parser_bison.c"
    break;

  case 819: /* set_list_expr: set_list_expr "comma" set_list_member_expr  */
#line 4602 "src/parser_bison.y"
                        {
				set_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 14061 "src/parser_bison.c"
    break;

  case 821: /* set_list_member_expr: opt_newline set_expr opt_newline  */
#line 4610 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14069 "src/parser_bison.c"
    break;

  case 822: /* set_list_member_expr: opt_newline set_elem_expr opt_newline  */
#line 4614 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14077 "src/parser_bison.c"
    break;

  case 823: /* set_list_member_expr: opt_newline set_elem_expr "colon" set_rhs_expr opt_newline  */
#line 4618 "src/parser_bison.y"
                        {
				(yyval.expr) = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr), (yyvsp[-1].expr));
			}
#line 14085 "src/parser_bison.c"
    break;

  case 825: /* meter_key_expr: meter_key_expr_alloc set_elem_options  */
#line 4625 "src/parser_bison.y"
                        {
				(yyval.expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14094 "src/parser_bison.c"
    break;

  case 826: /* meter_key_expr_alloc: concat_expr  */
#line 4632 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 14102 "src/parser_bison.c"
    break;

  case 829: /* set_elem_expr: set_elem_expr_alloc set_elem_expr_options set_elem_stmt_list  */
#line 4640 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 14112 "src/parser_bison.c"
    break;

  case 830: /* set_elem_key_expr: set_lhs_expr  */
#line 4647 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14118 "src/parser_bison.c"
    break;

  case 831: /* set_elem_key_expr: "*"  */
#line 4648 "src/parser_bison.y"
                                                        { (yyval.expr) = set_elem_catchall_expr_alloc(&(yylsp[0])); }
#line 14124 "src/parser_bison.c"
    break;

  case 832: /* set_elem_expr_alloc: set_elem_key_expr set_elem_stmt_list  */
#line 4652 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[-1]), (yyvsp[-1].expr));
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 14134 "src/parser_bison.c"
    break;

  case 833: /* set_elem_expr_alloc: set_elem_key_expr  */
#line 4658 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 14142 "src/parser_bison.c"
    break;

  case 834: /* set_elem_options: set_elem_option  */
#line 4664 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 14150 "src/parser_bison.c"
    break;

  case 836: /* set_elem_time_spec: "string"  */
#line 4671 "src/parser_bison.y"
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
#line 14173 "src/parser_bison.c"
    break;

  case 837: /* set_elem_option: "timeout" time_spec  */
#line 4692 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 14181 "src/parser_bison.c"
    break;

  case 838: /* set_elem_option: "expires" time_spec  */
#line 4696 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 14189 "src/parser_bison.c"
    break;

  case 839: /* set_elem_option: comment_spec  */
#line 4700 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 14201 "src/parser_bison.c"
    break;

  case 840: /* set_elem_expr_options: set_elem_expr_option  */
#line 4710 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 14209 "src/parser_bison.c"
    break;

  case 842: /* set_elem_stmt_list: set_elem_stmt  */
#line 4717 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 14219 "src/parser_bison.c"
    break;

  case 843: /* set_elem_stmt_list: set_elem_stmt_list set_elem_stmt  */
#line 4723 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 14228 "src/parser_bison.c"
    break;

  case 849: /* set_elem_expr_option: "timeout" set_elem_time_spec  */
#line 4737 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 14236 "src/parser_bison.c"
    break;

  case 850: /* set_elem_expr_option: "expires" time_spec  */
#line 4741 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 14244 "src/parser_bison.c"
    break;

  case 851: /* set_elem_expr_option: comment_spec  */
#line 4745 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 14256 "src/parser_bison.c"
    break;

  case 857: /* initializer_expr: '{' '}'  */
#line 4763 "src/parser_bison.y"
                                                { (yyval.expr) = set_expr_alloc(&(yyloc), NULL); }
#line 14262 "src/parser_bison.c"
    break;

  case 858: /* initializer_expr: "-" "number"  */
#line 4765 "src/parser_bison.y"
                        {
				int32_t num = -(yyvsp[0].val);

				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(num) * BITS_PER_BYTE,
							 &num);
			}
#line 14275 "src/parser_bison.c"
    break;

  case 859: /* counter_config: "packets" "number" "bytes" "number"  */
#line 4776 "src/parser_bison.y"
                        {
				struct counter *counter;

				counter = &(yyvsp[-4].obj)->counter;
				counter->packets = (yyvsp[-2].val);
				counter->bytes = (yyvsp[0].val);
			}
#line 14287 "src/parser_bison.c"
    break;

  case 860: /* counter_obj: %empty  */
#line 4786 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_COUNTER;
			}
#line 14296 "src/parser_bison.c"
    break;

  case 861: /* quota_config: quota_mode "number" quota_unit quota_used  */
#line 4793 "src/parser_bison.y"
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
#line 14318 "src/parser_bison.c"
    break;

  case 862: /* quota_obj: %empty  */
#line 4813 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_QUOTA;
			}
#line 14327 "src/parser_bison.c"
    break;

  case 863: /* secmark_config: string  */
#line 4820 "src/parser_bison.y"
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
#line 14345 "src/parser_bison.c"
    break;

  case 864: /* secmark_obj: %empty  */
#line 4836 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SECMARK;
			}
#line 14354 "src/parser_bison.c"
    break;

  case 865: /* ct_obj_type: "helper"  */
#line 4842 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_HELPER; }
#line 14360 "src/parser_bison.c"
    break;

  case 866: /* ct_obj_type: "timeout"  */
#line 4843 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 14366 "src/parser_bison.c"
    break;

  case 867: /* ct_obj_type: "expectation"  */
#line 4844 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 14372 "src/parser_bison.c"
    break;

  case 868: /* ct_cmd_type: "helpers"  */
#line 4847 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_HELPERS; }
#line 14378 "src/parser_bison.c"
    break;

  case 869: /* ct_cmd_type: "timeout"  */
#line 4848 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_TIMEOUTS; }
#line 14384 "src/parser_bison.c"
    break;

  case 870: /* ct_cmd_type: "expectation"  */
#line 4849 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_EXPECTATIONS; }
#line 14390 "src/parser_bison.c"
    break;

  case 871: /* ct_l4protoname: "tcp" close_scope_tcp  */
#line 4852 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_TCP; }
#line 14396 "src/parser_bison.c"
    break;

  case 872: /* ct_l4protoname: "udp" close_scope_udp  */
#line 4853 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_UDP; }
#line 14402 "src/parser_bison.c"
    break;

  case 873: /* ct_helper_config: "type" "quoted string" "protocol" ct_l4protoname stmt_separator close_scope_type  */
#line 4857 "src/parser_bison.y"
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
#line 14429 "src/parser_bison.c"
    break;

  case 874: /* ct_helper_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4880 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_helper.l3proto = (yyvsp[-1].val);
			}
#line 14437 "src/parser_bison.c"
    break;

  case 875: /* timeout_states: timeout_state  */
#line 4886 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyval.list));
			}
#line 14447 "src/parser_bison.c"
    break;

  case 876: /* timeout_states: timeout_states "comma" timeout_state  */
#line 4892 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyvsp[-2].list));
				(yyval.list) = (yyvsp[-2].list);
			}
#line 14456 "src/parser_bison.c"
    break;

  case 877: /* timeout_state: "string" "colon" time_spec_or_num_s  */
#line 4899 "src/parser_bison.y"
                        {
				struct timeout_state *ts;

				ts = xzalloc(sizeof(*ts));
				ts->timeout_str = (yyvsp[-2].string);
				ts->timeout_value = (yyvsp[0].val);
				ts->location = (yylsp[-2]);
				init_list_head(&ts->head);
				(yyval.timeout_state) = ts;
			}
#line 14471 "src/parser_bison.c"
    break;

  case 878: /* ct_timeout_config: "protocol" ct_l4protoname stmt_separator  */
#line 4912 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;
				int l4proto = (yyvsp[-1].val);

				ct = &(yyvsp[-3].obj)->ct_timeout;
				ct->l4proto = l4proto;
			}
#line 14483 "src/parser_bison.c"
    break;

  case 879: /* ct_timeout_config: "policy" '=' '{' timeout_states '}' stmt_separator close_scope_policy  */
#line 4920 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;

				ct = &(yyvsp[-7].obj)->ct_timeout;
				list_splice_tail((yyvsp[-3].list), &ct->timeout_list);
				free((yyvsp[-3].list));
			}
#line 14495 "src/parser_bison.c"
    break;

  case 880: /* ct_timeout_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4928 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_timeout.l3proto = (yyvsp[-1].val);
			}
#line 14503 "src/parser_bison.c"
    break;

  case 881: /* ct_expect_config: "protocol" ct_l4protoname stmt_separator  */
#line 4934 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l4proto = (yyvsp[-1].val);
			}
#line 14511 "src/parser_bison.c"
    break;

  case 882: /* ct_expect_config: "dport" "number" stmt_separator  */
#line 4938 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.dport = (yyvsp[-1].val);
			}
#line 14519 "src/parser_bison.c"
    break;

  case 883: /* ct_expect_config: "timeout" time_spec stmt_separator  */
#line 4942 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.timeout = (yyvsp[-1].val);
			}
#line 14527 "src/parser_bison.c"
    break;

  case 884: /* ct_expect_config: "size" "number" stmt_separator  */
#line 4946 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.size = (yyvsp[-1].val);
			}
#line 14535 "src/parser_bison.c"
    break;

  case 885: /* ct_expect_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4950 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l3proto = (yyvsp[-1].val);
			}
#line 14543 "src/parser_bison.c"
    break;

  case 886: /* ct_obj_alloc: %empty  */
#line 4956 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
			}
#line 14551 "src/parser_bison.c"
    break;

  case 887: /* limit_config: "rate" limit_mode limit_rate_pkts limit_burst_pkts  */
#line 4962 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-4].obj)->limit;
				limit->rate	= (yyvsp[-1].limit_rate).rate;
				limit->unit	= (yyvsp[-1].limit_rate).unit;
				limit->burst	= (yyvsp[0].val);
				limit->type	= NFT_LIMIT_PKTS;
				limit->flags	= (yyvsp[-2].val);
			}
#line 14566 "src/parser_bison.c"
    break;

  case 888: /* limit_config: "rate" limit_mode limit_rate_bytes limit_burst_bytes  */
#line 4973 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-4].obj)->limit;
				limit->rate	= (yyvsp[-1].limit_rate).rate;
				limit->unit	= (yyvsp[-1].limit_rate).unit;
				limit->burst	= (yyvsp[0].val);
				limit->type	= NFT_LIMIT_PKT_BYTES;
				limit->flags	= (yyvsp[-2].val);
			}
#line 14581 "src/parser_bison.c"
    break;

  case 889: /* limit_obj: %empty  */
#line 4986 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_LIMIT;
			}
#line 14590 "src/parser_bison.c"
    break;

  case 890: /* erspan_block: %empty  */
#line 4992 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14596 "src/parser_bison.c"
    break;

  case 893: /* erspan_block: erspan_block erspan_config stmt_separator  */
#line 4996 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14604 "src/parser_bison.c"
    break;

  case 894: /* erspan_block_alloc: %empty  */
#line 5002 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);

				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_ERSPAN, "erspan", state))
					YYERROR;
			}
#line 14615 "src/parser_bison.c"
    break;

  case 895: /* erspan_config: "version" "number"  */
#line 5011 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].obj)->tunnel.type == TUNNEL_ERSPAN);
				(yyvsp[-2].obj)->tunnel.erspan.version = (yyvsp[0].val);
			}
#line 14624 "src/parser_bison.c"
    break;

  case 896: /* erspan_config: "index" "number"  */
#line 5016 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v1.index = (yyvsp[0].val);
			}
#line 14632 "src/parser_bison.c"
    break;

  case 897: /* erspan_config: "direction" "ingress"  */
#line 5020 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.direction = 0;
			}
#line 14640 "src/parser_bison.c"
    break;

  case 898: /* erspan_config: "direction" "egress"  */
#line 5024 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.direction = 1;
			}
#line 14648 "src/parser_bison.c"
    break;

  case 899: /* erspan_config: "id" "number"  */
#line 5028 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.hwid = (yyvsp[0].val);
			}
#line 14656 "src/parser_bison.c"
    break;

  case 900: /* geneve_block: %empty  */
#line 5033 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14662 "src/parser_bison.c"
    break;

  case 903: /* geneve_block: geneve_block geneve_config stmt_separator  */
#line 5037 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14670 "src/parser_bison.c"
    break;

  case 904: /* geneve_block_alloc: %empty  */
#line 5043 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);
				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_GENEVE, "geneve", state))
					YYERROR;

				init_list_head(&(yyval.obj)->tunnel.geneve_opts);
			}
#line 14682 "src/parser_bison.c"
    break;

  case 905: /* geneve_config: "class" "number" "opt-type" "number" "data" string  */
#line 5053 "src/parser_bison.y"
                        {
				struct tunnel_geneve *geneve;

				assert((yyvsp[-6].obj)->tunnel.type == TUNNEL_GENEVE);

				geneve = xmalloc(sizeof(struct tunnel_geneve));
				geneve->geneve_class = (yyvsp[-4].val);
				geneve->type = (yyvsp[-2].val);
				if (tunnel_geneve_data_str2array((yyvsp[0].string), geneve->data, &geneve->data_len)) {
					erec_queue(error(&(yylsp[0]), "Invalid data array %s\n", (yyvsp[0].string)), state->msgs);
					free_const((yyvsp[0].string));
					free(geneve);
					YYERROR;
				}

				list_add_tail(&geneve->list, &(yyvsp[-6].obj)->tunnel.geneve_opts);
				free_const((yyvsp[0].string));
			}
#line 14705 "src/parser_bison.c"
    break;

  case 906: /* vxlan_block: %empty  */
#line 5073 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14711 "src/parser_bison.c"
    break;

  case 909: /* vxlan_block: vxlan_block vxlan_config stmt_separator  */
#line 5077 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14719 "src/parser_bison.c"
    break;

  case 910: /* vxlan_block_alloc: %empty  */
#line 5083 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);

				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_VXLAN, "vxlan", state))
					YYERROR;
			}
#line 14730 "src/parser_bison.c"
    break;

  case 911: /* vxlan_config: "gbp" "number"  */
#line 5092 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].obj)->tunnel.type == TUNNEL_VXLAN);
				(yyvsp[-2].obj)->tunnel.vxlan.gbp = (yyvsp[0].val);
			}
#line 14739 "src/parser_bison.c"
    break;

  case 912: /* tunnel_config: "id" "number"  */
#line 5099 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.id = (yyvsp[0].val);
			}
#line 14747 "src/parser_bison.c"
    break;

  case 913: /* tunnel_config: "ip" "saddr" symbol_expr close_scope_ip  */
#line 5103 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.src, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-4].obj)->tunnel.src = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ipaddr_type);
			}
#line 14761 "src/parser_bison.c"
    break;

  case 914: /* tunnel_config: "ip" "daddr" symbol_expr close_scope_ip  */
#line 5113 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.dst, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.dst = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ipaddr_type);
			}
#line 14774 "src/parser_bison.c"
    break;

  case 915: /* tunnel_config: "ip6" "saddr" symbol_expr close_scope_ip6  */
#line 5122 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.src, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.src = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ip6addr_type);
			}
#line 14787 "src/parser_bison.c"
    break;

  case 916: /* tunnel_config: "ip6" "daddr" symbol_expr close_scope_ip6  */
#line 5131 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.dst, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.dst = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ip6addr_type);
			}
#line 14800 "src/parser_bison.c"
    break;

  case 917: /* tunnel_config: "sport" "number"  */
#line 5140 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.sport = (yyvsp[0].val);
			}
#line 14808 "src/parser_bison.c"
    break;

  case 918: /* tunnel_config: "dport" "number"  */
#line 5144 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.dport = (yyvsp[0].val);
			}
#line 14816 "src/parser_bison.c"
    break;

  case 919: /* tunnel_config: "ttl" "number"  */
#line 5148 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.ttl = (yyvsp[0].val);
			}
#line 14824 "src/parser_bison.c"
    break;

  case 920: /* tunnel_config: "tos" "number"  */
#line 5152 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.tos = (yyvsp[0].val);
			}
#line 14832 "src/parser_bison.c"
    break;

  case 921: /* tunnel_config: "erspan" erspan_block_alloc '{' erspan_block '}'  */
#line 5156 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 14840 "src/parser_bison.c"
    break;

  case 922: /* tunnel_config: "vxlan" vxlan_block_alloc '{' vxlan_block '}'  */
#line 5160 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 14848 "src/parser_bison.c"
    break;

  case 923: /* tunnel_config: "geneve" geneve_block_alloc '{' geneve_block '}'  */
#line 5164 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 14856 "src/parser_bison.c"
    break;

  case 924: /* tunnel_block: %empty  */
#line 5169 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14862 "src/parser_bison.c"
    break;

  case 927: /* tunnel_block: tunnel_block tunnel_config stmt_separator  */
#line 5173 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14870 "src/parser_bison.c"
    break;

  case 928: /* tunnel_block: tunnel_block comment_spec  */
#line 5177 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 14882 "src/parser_bison.c"
    break;

  case 929: /* tunnel_obj: %empty  */
#line 5187 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_TUNNEL;
			}
#line 14891 "src/parser_bison.c"
    break;

  case 930: /* relational_expr: expr rhs_expr  */
#line 5194 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 14899 "src/parser_bison.c"
    break;

  case 931: /* relational_expr: expr list_rhs_expr  */
#line 5198 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 14907 "src/parser_bison.c"
    break;

  case 932: /* relational_expr: expr basic_rhs_expr "/" list_rhs_expr  */
#line 5202 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, (yyvsp[-2].expr));
			}
#line 14918 "src/parser_bison.c"
    break;

  case 933: /* relational_expr: expr list_rhs_expr "/" list_rhs_expr  */
#line 5209 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, value);
			}
#line 14930 "src/parser_bison.c"
    break;

  case 934: /* relational_expr: expr relational_op basic_rhs_expr "/" list_rhs_expr  */
#line 5217 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, (yyvsp[-2].expr));
			}
#line 14941 "src/parser_bison.c"
    break;

  case 935: /* relational_expr: expr relational_op list_rhs_expr "/" list_rhs_expr  */
#line 5224 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, value);
			}
#line 14953 "src/parser_bison.c"
    break;

  case 936: /* relational_expr: expr relational_op rhs_expr  */
#line 5232 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14961 "src/parser_bison.c"
    break;

  case 937: /* relational_expr: expr relational_op list_rhs_expr  */
#line 5236 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14969 "src/parser_bison.c"
    break;

  case 938: /* list_rhs_expr: basic_rhs_expr "comma" basic_rhs_expr  */
#line 5242 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[-2].expr));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 14979 "src/parser_bison.c"
    break;

  case 939: /* list_rhs_expr: list_rhs_expr "comma" basic_rhs_expr  */
#line 5248 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 14989 "src/parser_bison.c"
    break;

  case 940: /* rhs_expr: concat_rhs_expr  */
#line 5255 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14995 "src/parser_bison.c"
    break;

  case 941: /* rhs_expr: set_expr  */
#line 5256 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15001 "src/parser_bison.c"
    break;

  case 942: /* rhs_expr: set_ref_symbol_expr  */
#line 5257 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15007 "src/parser_bison.c"
    break;

  case 944: /* shift_rhs_expr: shift_rhs_expr "<<" primary_rhs_expr  */
#line 5262 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15015 "src/parser_bison.c"
    break;

  case 945: /* shift_rhs_expr: shift_rhs_expr ">>" primary_rhs_expr  */
#line 5266 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15023 "src/parser_bison.c"
    break;

  case 947: /* and_rhs_expr: and_rhs_expr "&" shift_rhs_expr  */
#line 5273 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15031 "src/parser_bison.c"
    break;

  case 949: /* exclusive_or_rhs_expr: exclusive_or_rhs_expr "^" and_rhs_expr  */
#line 5280 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15039 "src/parser_bison.c"
    break;

  case 951: /* inclusive_or_rhs_expr: inclusive_or_rhs_expr '|' exclusive_or_rhs_expr  */
#line 5287 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15047 "src/parser_bison.c"
    break;

  case 955: /* concat_rhs_expr: concat_rhs_expr "." multiton_rhs_expr  */
#line 5298 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 15060 "src/parser_bison.c"
    break;

  case 956: /* concat_rhs_expr: concat_rhs_expr "." basic_rhs_expr  */
#line 5307 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 15073 "src/parser_bison.c"
    break;

  case 957: /* boolean_keys: "exists"  */
#line 5317 "src/parser_bison.y"
                                                { (yyval.val8) = true; }
#line 15079 "src/parser_bison.c"
    break;

  case 958: /* boolean_keys: "missing"  */
#line 5318 "src/parser_bison.y"
                                                { (yyval.val8) = false; }
#line 15085 "src/parser_bison.c"
    break;

  case 959: /* boolean_expr: boolean_keys  */
#line 5322 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &boolean_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof((yyvsp[0].val8)) * BITS_PER_BYTE, &(yyvsp[0].val8));
			}
#line 15095 "src/parser_bison.c"
    break;

  case 960: /* keyword_expr: "ether" close_scope_eth  */
#line 5329 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ether"); }
#line 15101 "src/parser_bison.c"
    break;

  case 961: /* keyword_expr: "ip" close_scope_ip  */
#line 5330 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip"); }
#line 15107 "src/parser_bison.c"
    break;

  case 962: /* keyword_expr: "ip6" close_scope_ip6  */
#line 5331 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip6"); }
#line 15113 "src/parser_bison.c"
    break;

  case 963: /* keyword_expr: "vlan" close_scope_vlan  */
#line 5332 "src/parser_bison.y"
                                                         { (yyval.expr) = symbol_value(&(yyloc), "vlan"); }
#line 15119 "src/parser_bison.c"
    break;

  case 964: /* keyword_expr: "arp" close_scope_arp  */
#line 5333 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "arp"); }
#line 15125 "src/parser_bison.c"
    break;

  case 965: /* keyword_expr: "dnat" close_scope_nat  */
#line 5334 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "dnat"); }
#line 15131 "src/parser_bison.c"
    break;

  case 966: /* keyword_expr: "snat" close_scope_nat  */
#line 5335 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "snat"); }
#line 15137 "src/parser_bison.c"
    break;

  case 967: /* keyword_expr: "ecn"  */
#line 5336 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ecn"); }
#line 15143 "src/parser_bison.c"
    break;

  case 968: /* keyword_expr: "reset" close_scope_reset  */
#line 5337 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "reset"); }
#line 15149 "src/parser_bison.c"
    break;

  case 969: /* keyword_expr: "destroy" close_scope_destroy  */
#line 5338 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "destroy"); }
#line 15155 "src/parser_bison.c"
    break;

  case 970: /* keyword_expr: "original"  */
#line 5339 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "original"); }
#line 15161 "src/parser_bison.c"
    break;

  case 971: /* keyword_expr: "reply"  */
#line 5340 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "reply"); }
#line 15167 "src/parser_bison.c"
    break;

  case 972: /* keyword_expr: "label"  */
#line 5341 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "label"); }
#line 15173 "src/parser_bison.c"
    break;

  case 973: /* keyword_expr: "last" close_scope_last  */
#line 5342 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "last"); }
#line 15179 "src/parser_bison.c"
    break;

  case 974: /* primary_rhs_expr: symbol_expr  */
#line 5345 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15185 "src/parser_bison.c"
    break;

  case 975: /* primary_rhs_expr: integer_expr  */
#line 5346 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15191 "src/parser_bison.c"
    break;

  case 976: /* primary_rhs_expr: boolean_expr  */
#line 5347 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15197 "src/parser_bison.c"
    break;

  case 977: /* primary_rhs_expr: keyword_expr  */
#line 5348 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15203 "src/parser_bison.c"
    break;

  case 978: /* primary_rhs_expr: "tcp" close_scope_tcp  */
#line 5350 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_TCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15214 "src/parser_bison.c"
    break;

  case 979: /* primary_rhs_expr: "udp" close_scope_udp  */
#line 5357 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15225 "src/parser_bison.c"
    break;

  case 980: /* primary_rhs_expr: "udplite" close_scope_udplite  */
#line 5364 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDPLITE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15236 "src/parser_bison.c"
    break;

  case 981: /* primary_rhs_expr: "esp" close_scope_esp  */
#line 5371 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ESP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15247 "src/parser_bison.c"
    break;

  case 982: /* primary_rhs_expr: "ah" close_scope_ah  */
#line 5378 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_AH;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15258 "src/parser_bison.c"
    break;

  case 983: /* primary_rhs_expr: "icmp" close_scope_icmp  */
#line 5385 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15269 "src/parser_bison.c"
    break;

  case 984: /* primary_rhs_expr: "igmp"  */
#line 5392 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_IGMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15280 "src/parser_bison.c"
    break;

  case 985: /* primary_rhs_expr: "icmpv6" close_scope_icmp  */
#line 5399 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMPV6;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15291 "src/parser_bison.c"
    break;

  case 986: /* primary_rhs_expr: "gre" close_scope_gre  */
#line 5406 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_GRE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15302 "src/parser_bison.c"
    break;

  case 987: /* primary_rhs_expr: "comp" close_scope_comp  */
#line 5413 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_COMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15313 "src/parser_bison.c"
    break;

  case 988: /* primary_rhs_expr: "dccp" close_scope_dccp  */
#line 5420 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_DCCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15324 "src/parser_bison.c"
    break;

  case 989: /* primary_rhs_expr: "sctp" close_scope_sctp  */
#line 5427 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_SCTP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15335 "src/parser_bison.c"
    break;

  case 990: /* primary_rhs_expr: "redirect" close_scope_nat  */
#line 5434 "src/parser_bison.y"
                        {
				uint8_t data = ICMP_REDIRECT;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &icmp_type_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15346 "src/parser_bison.c"
    break;

  case 991: /* primary_rhs_expr: '(' basic_rhs_expr ')'  */
#line 5440 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 15352 "src/parser_bison.c"
    break;

  case 992: /* relational_op: "=="  */
#line 5443 "src/parser_bison.y"
                                                { (yyval.val) = OP_EQ; }
#line 15358 "src/parser_bison.c"
    break;

  case 993: /* relational_op: "!="  */
#line 5444 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEQ; }
#line 15364 "src/parser_bison.c"
    break;

  case 994: /* relational_op: "<"  */
#line 5445 "src/parser_bison.y"
                                                { (yyval.val) = OP_LT; }
#line 15370 "src/parser_bison.c"
    break;

  case 995: /* relational_op: ">"  */
#line 5446 "src/parser_bison.y"
                                                { (yyval.val) = OP_GT; }
#line 15376 "src/parser_bison.c"
    break;

  case 996: /* relational_op: ">="  */
#line 5447 "src/parser_bison.y"
                                                { (yyval.val) = OP_GTE; }
#line 15382 "src/parser_bison.c"
    break;

  case 997: /* relational_op: "<="  */
#line 5448 "src/parser_bison.y"
                                                { (yyval.val) = OP_LTE; }
#line 15388 "src/parser_bison.c"
    break;

  case 998: /* relational_op: "!"  */
#line 5449 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEG; }
#line 15394 "src/parser_bison.c"
    break;

  case 999: /* verdict_expr: "accept"  */
#line 5453 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_ACCEPT, NULL);
			}
#line 15402 "src/parser_bison.c"
    break;

  case 1000: /* verdict_expr: "drop"  */
#line 5457 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_DROP, NULL);
			}
#line 15410 "src/parser_bison.c"
    break;

  case 1001: /* verdict_expr: "continue"  */
#line 5461 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_CONTINUE, NULL);
			}
#line 15418 "src/parser_bison.c"
    break;

  case 1002: /* verdict_expr: "jump" chain_expr  */
#line 5465 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_JUMP, (yyvsp[0].expr));
			}
#line 15426 "src/parser_bison.c"
    break;

  case 1003: /* verdict_expr: "goto" chain_expr  */
#line 5469 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_GOTO, (yyvsp[0].expr));
			}
#line 15434 "src/parser_bison.c"
    break;

  case 1004: /* verdict_expr: "return"  */
#line 5473 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_RETURN, NULL);
			}
#line 15442 "src/parser_bison.c"
    break;

  case 1006: /* chain_expr: identifier  */
#line 5480 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &string_type,
							 BYTEORDER_HOST_ENDIAN,
							 strlen((yyvsp[0].string)) * BITS_PER_BYTE,
							 (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 15454 "src/parser_bison.c"
    break;

  case 1007: /* meta_expr: "meta" meta_key close_scope_meta  */
#line 5490 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 15462 "src/parser_bison.c"
    break;

  case 1008: /* meta_expr: meta_key_unqualified  */
#line 5494 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[0].val));
			}
#line 15470 "src/parser_bison.c"
    break;

  case 1009: /* meta_expr: "meta" "string" close_scope_meta  */
#line 5498 "src/parser_bison.y"
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
#line 15488 "src/parser_bison.c"
    break;

  case 1012: /* meta_key_qualified: "length"  */
#line 5517 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_LEN; }
#line 15494 "src/parser_bison.c"
    break;

  case 1013: /* meta_key_qualified: "protocol"  */
#line 5518 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PROTOCOL; }
#line 15500 "src/parser_bison.c"
    break;

  case 1014: /* meta_key_qualified: "priority"  */
#line 5519 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRIORITY; }
#line 15506 "src/parser_bison.c"
    break;

  case 1015: /* meta_key_qualified: "random"  */
#line 5520 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRANDOM; }
#line 15512 "src/parser_bison.c"
    break;

  case 1016: /* meta_key_qualified: "secmark" close_scope_secmark  */
#line 5521 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_META_SECMARK; }
#line 15518 "src/parser_bison.c"
    break;

  case 1017: /* meta_key_unqualified: "mark"  */
#line 5524 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_MARK; }
#line 15524 "src/parser_bison.c"
    break;

  case 1018: /* meta_key_unqualified: "iif"  */
#line 5525 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIF; }
#line 15530 "src/parser_bison.c"
    break;

  case 1019: /* meta_key_unqualified: "iifname"  */
#line 5526 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFNAME; }
#line 15536 "src/parser_bison.c"
    break;

  case 1020: /* meta_key_unqualified: "iiftype"  */
#line 5527 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFTYPE; }
#line 15542 "src/parser_bison.c"
    break;

  case 1021: /* meta_key_unqualified: "oif"  */
#line 5528 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIF; }
#line 15548 "src/parser_bison.c"
    break;

  case 1022: /* meta_key_unqualified: "oifname"  */
#line 5529 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFNAME; }
#line 15554 "src/parser_bison.c"
    break;

  case 1023: /* meta_key_unqualified: "oiftype"  */
#line 5530 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFTYPE; }
#line 15560 "src/parser_bison.c"
    break;

  case 1024: /* meta_key_unqualified: "skuid"  */
#line 5531 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKUID; }
#line 15566 "src/parser_bison.c"
    break;

  case 1025: /* meta_key_unqualified: "skgid"  */
#line 5532 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKGID; }
#line 15572 "src/parser_bison.c"
    break;

  case 1026: /* meta_key_unqualified: "nftrace"  */
#line 5533 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_NFTRACE; }
#line 15578 "src/parser_bison.c"
    break;

  case 1027: /* meta_key_unqualified: "rtclassid"  */
#line 5534 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_RTCLASSID; }
#line 15584 "src/parser_bison.c"
    break;

  case 1028: /* meta_key_unqualified: "ibriport"  */
#line 5535 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFNAME; }
#line 15590 "src/parser_bison.c"
    break;

  case 1029: /* meta_key_unqualified: "obriport"  */
#line 5536 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_OIFNAME; }
#line 15596 "src/parser_bison.c"
    break;

  case 1030: /* meta_key_unqualified: "ibrname"  */
#line 5537 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFNAME; }
#line 15602 "src/parser_bison.c"
    break;

  case 1031: /* meta_key_unqualified: "obrname"  */
#line 5538 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_OIFNAME; }
#line 15608 "src/parser_bison.c"
    break;

  case 1032: /* meta_key_unqualified: "pkttype"  */
#line 5539 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PKTTYPE; }
#line 15614 "src/parser_bison.c"
    break;

  case 1033: /* meta_key_unqualified: "cpu"  */
#line 5540 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CPU; }
#line 15620 "src/parser_bison.c"
    break;

  case 1034: /* meta_key_unqualified: "iifgroup"  */
#line 5541 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFGROUP; }
#line 15626 "src/parser_bison.c"
    break;

  case 1035: /* meta_key_unqualified: "oifgroup"  */
#line 5542 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFGROUP; }
#line 15632 "src/parser_bison.c"
    break;

  case 1036: /* meta_key_unqualified: "cgroup"  */
#line 5543 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CGROUP; }
#line 15638 "src/parser_bison.c"
    break;

  case 1037: /* meta_key_unqualified: "ipsec" close_scope_ipsec  */
#line 5544 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_META_SECPATH; }
#line 15644 "src/parser_bison.c"
    break;

  case 1038: /* meta_key_unqualified: "time"  */
#line 5545 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_NS; }
#line 15650 "src/parser_bison.c"
    break;

  case 1039: /* meta_key_unqualified: "day"  */
#line 5546 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_DAY; }
#line 15656 "src/parser_bison.c"
    break;

  case 1040: /* meta_key_unqualified: "hour"  */
#line 5547 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_HOUR; }
#line 15662 "src/parser_bison.c"
    break;

  case 1041: /* meta_stmt: "meta" meta_key "set" stmt_expr close_scope_meta  */
#line 5551 "src/parser_bison.y"
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
#line 15686 "src/parser_bison.c"
    break;

  case 1042: /* meta_stmt: meta_key_unqualified "set" stmt_expr  */
#line 5571 "src/parser_bison.y"
                        {
				(yyval.stmt) = meta_stmt_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[0].expr));
			}
#line 15694 "src/parser_bison.c"
    break;

  case 1043: /* meta_stmt: "meta" "string" "set" stmt_expr close_scope_meta  */
#line 5575 "src/parser_bison.y"
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
#line 15713 "src/parser_bison.c"
    break;

  case 1044: /* meta_stmt: "notrack"  */
#line 5590 "src/parser_bison.y"
                        {
				(yyval.stmt) = notrack_stmt_alloc(&(yyloc));
			}
#line 15721 "src/parser_bison.c"
    break;

  case 1045: /* meta_stmt: "flow" "offload" "@" string close_scope_at  */
#line 5594 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 15729 "src/parser_bison.c"
    break;

  case 1046: /* meta_stmt: "flow" "add" "@" string close_scope_at  */
#line 5598 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 15737 "src/parser_bison.c"
    break;

  case 1047: /* socket_expr: "socket" socket_key close_scope_socket  */
#line 5604 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), (yyvsp[-1].val), 0);
			}
#line 15745 "src/parser_bison.c"
    break;

  case 1048: /* socket_expr: "socket" "cgroupv2" "level" "number" close_scope_socket  */
#line 5608 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), NFT_SOCKET_CGROUPV2, (yyvsp[-1].val));
			}
#line 15753 "src/parser_bison.c"
    break;

  case 1049: /* socket_key: "transparent"  */
#line 5613 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_TRANSPARENT; }
#line 15759 "src/parser_bison.c"
    break;

  case 1050: /* socket_key: "mark"  */
#line 5614 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_MARK; }
#line 15765 "src/parser_bison.c"
    break;

  case 1051: /* socket_key: "wildcard"  */
#line 5615 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_WILDCARD; }
#line 15771 "src/parser_bison.c"
    break;

  case 1052: /* tunnel_key: "path"  */
#line 5618 "src/parser_bison.y"
                                                { (yyval.val) = NFT_TUNNEL_PATH; }
#line 15777 "src/parser_bison.c"
    break;

  case 1053: /* tunnel_key: "id"  */
#line 5619 "src/parser_bison.y"
                                                { (yyval.val) = NFT_TUNNEL_ID; }
#line 15783 "src/parser_bison.c"
    break;

  case 1054: /* tunnel_expr: "tunnel" tunnel_key  */
#line 5623 "src/parser_bison.y"
                        {
				(yyval.expr) = tunnel_expr_alloc(&(yyloc), (yyvsp[0].val));
			}
#line 15791 "src/parser_bison.c"
    break;

  case 1055: /* offset_opt: %empty  */
#line 5628 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 15797 "src/parser_bison.c"
    break;

  case 1056: /* offset_opt: "offset" "number"  */
#line 5629 "src/parser_bison.y"
                                                { (yyval.val) = (yyvsp[0].val); }
#line 15803 "src/parser_bison.c"
    break;

  case 1057: /* numgen_type: "inc"  */
#line 5632 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_INCREMENTAL; }
#line 15809 "src/parser_bison.c"
    break;

  case 1058: /* numgen_type: "random"  */
#line 5633 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_RANDOM; }
#line 15815 "src/parser_bison.c"
    break;

  case 1059: /* numgen_expr: "numgen" numgen_type "mod" "number" offset_opt close_scope_numgen  */
#line 5637 "src/parser_bison.y"
                        {
				(yyval.expr) = numgen_expr_alloc(&(yyloc), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 15823 "src/parser_bison.c"
    break;

  case 1060: /* xfrm_spnum: "spnum" "number"  */
#line 5642 "src/parser_bison.y"
                                            { (yyval.val) = (yyvsp[0].val); }
#line 15829 "src/parser_bison.c"
    break;

  case 1061: /* xfrm_spnum: %empty  */
#line 5643 "src/parser_bison.y"
                                            { (yyval.val) = 0; }
#line 15835 "src/parser_bison.c"
    break;

  case 1062: /* xfrm_dir: "in"  */
#line 5646 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_IN; }
#line 15841 "src/parser_bison.c"
    break;

  case 1063: /* xfrm_dir: "out"  */
#line 5647 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_OUT; }
#line 15847 "src/parser_bison.c"
    break;

  case 1064: /* xfrm_state_key: "spi"  */
#line 5650 "src/parser_bison.y"
                                    { (yyval.val) = NFT_XFRM_KEY_SPI; }
#line 15853 "src/parser_bison.c"
    break;

  case 1065: /* xfrm_state_key: "reqid"  */
#line 5651 "src/parser_bison.y"
                                      { (yyval.val) = NFT_XFRM_KEY_REQID; }
#line 15859 "src/parser_bison.c"
    break;

  case 1066: /* xfrm_state_proto_key: "daddr"  */
#line 5654 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_DADDR_IP4; }
#line 15865 "src/parser_bison.c"
    break;

  case 1067: /* xfrm_state_proto_key: "saddr"  */
#line 5655 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_SADDR_IP4; }
#line 15871 "src/parser_bison.c"
    break;

  case 1068: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum xfrm_state_key close_scope_ipsec  */
#line 5659 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].val) > 255) {
					erec_queue(error(&(yylsp[-2]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.expr) = xfrm_expr_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 15883 "src/parser_bison.c"
    break;

  case 1069: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum nf_key_proto xfrm_state_proto_key close_scope_ipsec  */
#line 5667 "src/parser_bison.y"
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
#line 15912 "src/parser_bison.c"
    break;

  case 1070: /* hash_expr: "jhash" expr "mod" "number" "seed" "number" offset_opt close_scope_hash  */
#line 5694 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-4].val), true, (yyvsp[-2].val), (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-6].expr);
			}
#line 15921 "src/parser_bison.c"
    break;

  case 1071: /* hash_expr: "jhash" expr "mod" "number" offset_opt close_scope_hash  */
#line 5699 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-4].expr);
			}
#line 15930 "src/parser_bison.c"
    break;

  case 1072: /* hash_expr: "symhash" "mod" "number" offset_opt close_scope_hash  */
#line 5704 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_SYM);
			}
#line 15938 "src/parser_bison.c"
    break;

  case 1073: /* nf_key_proto: "ip" close_scope_ip  */
#line 5709 "src/parser_bison.y"
                                                       { (yyval.val) = NFPROTO_IPV4; }
#line 15944 "src/parser_bison.c"
    break;

  case 1074: /* nf_key_proto: "ip6" close_scope_ip6  */
#line 5710 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 15950 "src/parser_bison.c"
    break;

  case 1075: /* rt_expr: "rt" rt_key close_scope_rt  */
#line 5714 "src/parser_bison.y"
                        {
				(yyval.expr) = rt_expr_alloc(&(yyloc), (yyvsp[-1].val), true);
			}
#line 15958 "src/parser_bison.c"
    break;

  case 1076: /* rt_expr: "rt" nf_key_proto rt_key close_scope_rt  */
#line 5718 "src/parser_bison.y"
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
#line 15980 "src/parser_bison.c"
    break;

  case 1077: /* rt_key: "classid"  */
#line 5737 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_CLASSID; }
#line 15986 "src/parser_bison.c"
    break;

  case 1078: /* rt_key: "nexthop"  */
#line 5738 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_NEXTHOP4; }
#line 15992 "src/parser_bison.c"
    break;

  case 1079: /* rt_key: "mtu"  */
#line 5739 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_TCPMSS; }
#line 15998 "src/parser_bison.c"
    break;

  case 1080: /* rt_key: "ipsec" close_scope_ipsec  */
#line 5740 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_RT_XFRM; }
#line 16004 "src/parser_bison.c"
    break;

  case 1081: /* ct_expr: "ct" ct_key close_scope_ct  */
#line 5744 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), -1);
			}
#line 16012 "src/parser_bison.c"
    break;

  case 1082: /* ct_expr: "ct" ct_dir ct_key_dir close_scope_ct  */
#line 5748 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 16020 "src/parser_bison.c"
    break;

  case 1083: /* ct_expr: "ct" ct_dir ct_key_proto_field close_scope_ct  */
#line 5752 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 16028 "src/parser_bison.c"
    break;

  case 1084: /* ct_dir: "original"  */
#line 5757 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_ORIGINAL; }
#line 16034 "src/parser_bison.c"
    break;

  case 1085: /* ct_dir: "reply"  */
#line 5758 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_REPLY; }
#line 16040 "src/parser_bison.c"
    break;

  case 1086: /* ct_key: "l3proto"  */
#line 5761 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 16046 "src/parser_bison.c"
    break;

  case 1087: /* ct_key: "protocol"  */
#line 5762 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 16052 "src/parser_bison.c"
    break;

  case 1088: /* ct_key: "mark"  */
#line 5763 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_MARK; }
#line 16058 "src/parser_bison.c"
    break;

  case 1089: /* ct_key: "state"  */
#line 5764 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATE; }
#line 16064 "src/parser_bison.c"
    break;

  case 1090: /* ct_key: "direction"  */
#line 5765 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DIRECTION; }
#line 16070 "src/parser_bison.c"
    break;

  case 1091: /* ct_key: "status"  */
#line 5766 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATUS; }
#line 16076 "src/parser_bison.c"
    break;

  case 1092: /* ct_key: "expiration"  */
#line 5767 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EXPIRATION; }
#line 16082 "src/parser_bison.c"
    break;

  case 1093: /* ct_key: "helper"  */
#line 5768 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_HELPER; }
#line 16088 "src/parser_bison.c"
    break;

  case 1094: /* ct_key: "saddr"  */
#line 5769 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 16094 "src/parser_bison.c"
    break;

  case 1095: /* ct_key: "daddr"  */
#line 5770 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 16100 "src/parser_bison.c"
    break;

  case 1096: /* ct_key: "proto-src"  */
#line 5771 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 16106 "src/parser_bison.c"
    break;

  case 1097: /* ct_key: "proto-dst"  */
#line 5772 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 16112 "src/parser_bison.c"
    break;

  case 1098: /* ct_key: "label"  */
#line 5773 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_LABELS; }
#line 16118 "src/parser_bison.c"
    break;

  case 1099: /* ct_key: "event"  */
#line 5774 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EVENTMASK; }
#line 16124 "src/parser_bison.c"
    break;

  case 1100: /* ct_key: "secmark" close_scope_secmark  */
#line 5775 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_CT_SECMARK; }
#line 16130 "src/parser_bison.c"
    break;

  case 1101: /* ct_key: "id"  */
#line 5776 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ID; }
#line 16136 "src/parser_bison.c"
    break;

  case 1103: /* ct_key_dir: "saddr"  */
#line 5780 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 16142 "src/parser_bison.c"
    break;

  case 1104: /* ct_key_dir: "daddr"  */
#line 5781 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 16148 "src/parser_bison.c"
    break;

  case 1105: /* ct_key_dir: "l3proto"  */
#line 5782 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 16154 "src/parser_bison.c"
    break;

  case 1106: /* ct_key_dir: "protocol"  */
#line 5783 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 16160 "src/parser_bison.c"
    break;

  case 1107: /* ct_key_dir: "proto-src"  */
#line 5784 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 16166 "src/parser_bison.c"
    break;

  case 1108: /* ct_key_dir: "proto-dst"  */
#line 5785 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 16172 "src/parser_bison.c"
    break;

  case 1110: /* ct_key_proto_field: "ip" "saddr" close_scope_ip  */
#line 5789 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_SRC_IP; }
#line 16178 "src/parser_bison.c"
    break;

  case 1111: /* ct_key_proto_field: "ip" "daddr" close_scope_ip  */
#line 5790 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_DST_IP; }
#line 16184 "src/parser_bison.c"
    break;

  case 1112: /* ct_key_proto_field: "ip6" "saddr" close_scope_ip6  */
#line 5791 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_SRC_IP6; }
#line 16190 "src/parser_bison.c"
    break;

  case 1113: /* ct_key_proto_field: "ip6" "daddr" close_scope_ip6  */
#line 5792 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_DST_IP6; }
#line 16196 "src/parser_bison.c"
    break;

  case 1114: /* ct_key_dir_optional: "bytes"  */
#line 5795 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_BYTES; }
#line 16202 "src/parser_bison.c"
    break;

  case 1115: /* ct_key_dir_optional: "packets"  */
#line 5796 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PKTS; }
#line 16208 "src/parser_bison.c"
    break;

  case 1116: /* ct_key_dir_optional: "avgpkt"  */
#line 5797 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_AVGPKT; }
#line 16214 "src/parser_bison.c"
    break;

  case 1117: /* ct_key_dir_optional: "zone"  */
#line 5798 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ZONE; }
#line 16220 "src/parser_bison.c"
    break;

  case 1120: /* list_stmt_expr: symbol_stmt_expr "comma" symbol_stmt_expr  */
#line 5806 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[-2].expr));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 16230 "src/parser_bison.c"
    break;

  case 1121: /* list_stmt_expr: list_stmt_expr "comma" symbol_stmt_expr  */
#line 5812 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 16240 "src/parser_bison.c"
    break;

  case 1122: /* ct_stmt: "ct" ct_key "set" stmt_expr close_scope_ct  */
#line 5820 "src/parser_bison.y"
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
#line 16257 "src/parser_bison.c"
    break;

  case 1123: /* ct_stmt: "ct" ct_dir ct_key_dir_optional "set" stmt_expr close_scope_ct  */
#line 5833 "src/parser_bison.y"
                        {
				(yyval.stmt) = ct_stmt_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-4].val), (yyvsp[-1].expr));
			}
#line 16265 "src/parser_bison.c"
    break;

  case 1124: /* payload_stmt: payload_expr "set" stmt_expr  */
#line 5839 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].expr)->etype == EXPR_EXTHDR)
					(yyval.stmt) = exthdr_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
				else
					(yyval.stmt) = payload_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 16276 "src/parser_bison.c"
    break;

  case 1147: /* payload_raw_len: "number"  */
#line 5872 "src/parser_bison.y"
                        {
				if ((yyvsp[0].val) > NFT_MAX_EXPR_LEN_BITS) {
					erec_queue(error(&(yylsp[0]), "raw payload length %lu exceeds upper limit of %lu",
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
#line 16296 "src/parser_bison.c"
    break;

  case 1148: /* payload_raw_expr: "@" payload_base_spec "comma" "number" "comma" payload_raw_len close_scope_at  */
#line 5890 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), NULL, 0);
				payload_init_raw((yyval.expr), (yyvsp[-5].val), (yyvsp[-3].val), (yyvsp[-1].val));
				(yyval.expr)->byteorder		= BYTEORDER_BIG_ENDIAN;
				(yyval.expr)->payload.is_raw	= true;
			}
#line 16307 "src/parser_bison.c"
    break;

  case 1149: /* payload_base_spec: "ll"  */
#line 5898 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_LL_HDR; }
#line 16313 "src/parser_bison.c"
    break;

  case 1150: /* payload_base_spec: "nh"  */
#line 5899 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_NETWORK_HDR; }
#line 16319 "src/parser_bison.c"
    break;

  case 1151: /* payload_base_spec: "th" close_scope_th  */
#line 5900 "src/parser_bison.y"
                                                                { (yyval.val) = PROTO_BASE_TRANSPORT_HDR; }
#line 16325 "src/parser_bison.c"
    break;

  case 1152: /* payload_base_spec: "string"  */
#line 5902 "src/parser_bison.y"
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
#line 16340 "src/parser_bison.c"
    break;

  case 1153: /* eth_hdr_expr: "ether" eth_hdr_field close_scope_eth  */
#line 5915 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_eth, (yyvsp[-1].val));
			}
#line 16348 "src/parser_bison.c"
    break;

  case 1154: /* eth_hdr_field: "saddr"  */
#line 5920 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_SADDR; }
#line 16354 "src/parser_bison.c"
    break;

  case 1155: /* eth_hdr_field: "daddr"  */
#line 5921 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_DADDR; }
#line 16360 "src/parser_bison.c"
    break;

  case 1156: /* eth_hdr_field: "type" close_scope_type  */
#line 5922 "src/parser_bison.y"
                                                                        { (yyval.val) = ETHHDR_TYPE; }
#line 16366 "src/parser_bison.c"
    break;

  case 1157: /* vlan_hdr_expr: "vlan" vlan_hdr_field close_scope_vlan  */
#line 5926 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_vlan, (yyvsp[-1].val));
			}
#line 16374 "src/parser_bison.c"
    break;

  case 1158: /* vlan_hdr_field: "id"  */
#line 5931 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_VID; }
#line 16380 "src/parser_bison.c"
    break;

  case 1159: /* vlan_hdr_field: "cfi"  */
#line 5932 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_CFI; }
#line 16386 "src/parser_bison.c"
    break;

  case 1160: /* vlan_hdr_field: "dei"  */
#line 5933 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_DEI; }
#line 16392 "src/parser_bison.c"
    break;

  case 1161: /* vlan_hdr_field: "pcp"  */
#line 5934 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_PCP; }
#line 16398 "src/parser_bison.c"
    break;

  case 1162: /* vlan_hdr_field: "type" close_scope_type  */
#line 5935 "src/parser_bison.y"
                                                                        { (yyval.val) = VLANHDR_TYPE; }
#line 16404 "src/parser_bison.c"
    break;

  case 1163: /* arp_hdr_expr: "arp" arp_hdr_field close_scope_arp  */
#line 5939 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_arp, (yyvsp[-1].val));
			}
#line 16412 "src/parser_bison.c"
    break;

  case 1164: /* arp_hdr_field: "htype"  */
#line 5944 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HRD; }
#line 16418 "src/parser_bison.c"
    break;

  case 1165: /* arp_hdr_field: "ptype"  */
#line 5945 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PRO; }
#line 16424 "src/parser_bison.c"
    break;

  case 1166: /* arp_hdr_field: "hlen"  */
#line 5946 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HLN; }
#line 16430 "src/parser_bison.c"
    break;

  case 1167: /* arp_hdr_field: "plen"  */
#line 5947 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PLN; }
#line 16436 "src/parser_bison.c"
    break;

  case 1168: /* arp_hdr_field: "operation"  */
#line 5948 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_OP; }
#line 16442 "src/parser_bison.c"
    break;

  case 1169: /* arp_hdr_field: "saddr" "ether" close_scope_eth  */
#line 5949 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_ETHER; }
#line 16448 "src/parser_bison.c"
    break;

  case 1170: /* arp_hdr_field: "daddr" "ether" close_scope_eth  */
#line 5950 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_ETHER; }
#line 16454 "src/parser_bison.c"
    break;

  case 1171: /* arp_hdr_field: "saddr" "ip" close_scope_ip  */
#line 5951 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_IP; }
#line 16460 "src/parser_bison.c"
    break;

  case 1172: /* arp_hdr_field: "daddr" "ip" close_scope_ip  */
#line 5952 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_IP; }
#line 16466 "src/parser_bison.c"
    break;

  case 1173: /* ip_hdr_expr: "ip" ip_hdr_field close_scope_ip  */
#line 5956 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip, (yyvsp[-1].val));
			}
#line 16474 "src/parser_bison.c"
    break;

  case 1174: /* ip_hdr_expr: "ip" "option" ip_option_type ip_option_field close_scope_ip  */
#line 5960 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[-1].val));
				if (!(yyval.expr)) {
					erec_queue(error(&(yylsp[-4]), "unknown ip option type/field"), state->msgs);
					YYERROR;
				}

				if ((yyvsp[-1].val) == IPOPT_FIELD_TYPE)
					(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 16489 "src/parser_bison.c"
    break;

  case 1175: /* ip_hdr_expr: "ip" "option" ip_option_type close_scope_ip  */
#line 5971 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-1].val), IPOPT_FIELD_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 16498 "src/parser_bison.c"
    break;

  case 1176: /* ip_hdr_field: "version"  */
#line 5977 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_VERSION; }
#line 16504 "src/parser_bison.c"
    break;

  case 1177: /* ip_hdr_field: "hdrlength"  */
#line 5978 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_HDRLENGTH; }
#line 16510 "src/parser_bison.c"
    break;

  case 1178: /* ip_hdr_field: "dscp"  */
#line 5979 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DSCP; }
#line 16516 "src/parser_bison.c"
    break;

  case 1179: /* ip_hdr_field: "ecn"  */
#line 5980 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ECN; }
#line 16522 "src/parser_bison.c"
    break;

  case 1180: /* ip_hdr_field: "length"  */
#line 5981 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_LENGTH; }
#line 16528 "src/parser_bison.c"
    break;

  case 1181: /* ip_hdr_field: "id"  */
#line 5982 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ID; }
#line 16534 "src/parser_bison.c"
    break;

  case 1182: /* ip_hdr_field: "frag-off"  */
#line 5983 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_FRAG_OFF; }
#line 16540 "src/parser_bison.c"
    break;

  case 1183: /* ip_hdr_field: "ttl"  */
#line 5984 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_TTL; }
#line 16546 "src/parser_bison.c"
    break;

  case 1184: /* ip_hdr_field: "protocol"  */
#line 5985 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_PROTOCOL; }
#line 16552 "src/parser_bison.c"
    break;

  case 1185: /* ip_hdr_field: "checksum"  */
#line 5986 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_CHECKSUM; }
#line 16558 "src/parser_bison.c"
    break;

  case 1186: /* ip_hdr_field: "saddr"  */
#line 5987 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_SADDR; }
#line 16564 "src/parser_bison.c"
    break;

  case 1187: /* ip_hdr_field: "daddr"  */
#line 5988 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DADDR; }
#line 16570 "src/parser_bison.c"
    break;

  case 1188: /* ip_option_type: "lsrr"  */
#line 5991 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_LSRR; }
#line 16576 "src/parser_bison.c"
    break;

  case 1189: /* ip_option_type: "rr"  */
#line 5992 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RR; }
#line 16582 "src/parser_bison.c"
    break;

  case 1190: /* ip_option_type: "ssrr"  */
#line 5993 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_SSRR; }
#line 16588 "src/parser_bison.c"
    break;

  case 1191: /* ip_option_type: "ra"  */
#line 5994 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RA; }
#line 16594 "src/parser_bison.c"
    break;

  case 1192: /* ip_option_field: "type" close_scope_type  */
#line 5997 "src/parser_bison.y"
                                                                        { (yyval.val) = IPOPT_FIELD_TYPE; }
#line 16600 "src/parser_bison.c"
    break;

  case 1193: /* ip_option_field: "length"  */
#line 5998 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_LENGTH; }
#line 16606 "src/parser_bison.c"
    break;

  case 1194: /* ip_option_field: "value"  */
#line 5999 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_VALUE; }
#line 16612 "src/parser_bison.c"
    break;

  case 1195: /* ip_option_field: "ptr"  */
#line 6000 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_PTR; }
#line 16618 "src/parser_bison.c"
    break;

  case 1196: /* ip_option_field: "addr"  */
#line 6001 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_ADDR_0; }
#line 16624 "src/parser_bison.c"
    break;

  case 1197: /* icmp_hdr_expr: "icmp" icmp_hdr_field close_scope_icmp  */
#line 6005 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp, (yyvsp[-1].val));
			}
#line 16632 "src/parser_bison.c"
    break;

  case 1198: /* icmp_hdr_field: "type" close_scope_type  */
#line 6010 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMPHDR_TYPE; }
#line 16638 "src/parser_bison.c"
    break;

  case 1199: /* icmp_hdr_field: "code"  */
#line 6011 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CODE; }
#line 16644 "src/parser_bison.c"
    break;

  case 1200: /* icmp_hdr_field: "checksum"  */
#line 6012 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CHECKSUM; }
#line 16650 "src/parser_bison.c"
    break;

  case 1201: /* icmp_hdr_field: "id"  */
#line 6013 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_ID; }
#line 16656 "src/parser_bison.c"
    break;

  case 1202: /* icmp_hdr_field: "seq"  */
#line 6014 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_SEQ; }
#line 16662 "src/parser_bison.c"
    break;

  case 1203: /* icmp_hdr_field: "gateway"  */
#line 6015 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_GATEWAY; }
#line 16668 "src/parser_bison.c"
    break;

  case 1204: /* icmp_hdr_field: "mtu"  */
#line 6016 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_MTU; }
#line 16674 "src/parser_bison.c"
    break;

  case 1205: /* igmp_hdr_expr: "igmp" igmp_hdr_field close_scope_igmp  */
#line 6020 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_igmp, (yyvsp[-1].val));
			}
#line 16682 "src/parser_bison.c"
    break;

  case 1206: /* igmp_hdr_field: "type" close_scope_type  */
#line 6025 "src/parser_bison.y"
                                                                        { (yyval.val) = IGMPHDR_TYPE; }
#line 16688 "src/parser_bison.c"
    break;

  case 1207: /* igmp_hdr_field: "checksum"  */
#line 6026 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_CHECKSUM; }
#line 16694 "src/parser_bison.c"
    break;

  case 1208: /* igmp_hdr_field: "mrt"  */
#line 6027 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_MRT; }
#line 16700 "src/parser_bison.c"
    break;

  case 1209: /* igmp_hdr_field: "group"  */
#line 6028 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_GROUP; }
#line 16706 "src/parser_bison.c"
    break;

  case 1210: /* ip6_hdr_expr: "ip6" ip6_hdr_field close_scope_ip6  */
#line 6032 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip6, (yyvsp[-1].val));
			}
#line 16714 "src/parser_bison.c"
    break;

  case 1211: /* ip6_hdr_field: "version"  */
#line 6037 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_VERSION; }
#line 16720 "src/parser_bison.c"
    break;

  case 1212: /* ip6_hdr_field: "dscp"  */
#line 6038 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DSCP; }
#line 16726 "src/parser_bison.c"
    break;

  case 1213: /* ip6_hdr_field: "ecn"  */
#line 6039 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_ECN; }
#line 16732 "src/parser_bison.c"
    break;

  case 1214: /* ip6_hdr_field: "flowlabel"  */
#line 6040 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_FLOWLABEL; }
#line 16738 "src/parser_bison.c"
    break;

  case 1215: /* ip6_hdr_field: "length"  */
#line 6041 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_LENGTH; }
#line 16744 "src/parser_bison.c"
    break;

  case 1216: /* ip6_hdr_field: "nexthdr"  */
#line 6042 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_NEXTHDR; }
#line 16750 "src/parser_bison.c"
    break;

  case 1217: /* ip6_hdr_field: "hoplimit"  */
#line 6043 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_HOPLIMIT; }
#line 16756 "src/parser_bison.c"
    break;

  case 1218: /* ip6_hdr_field: "saddr"  */
#line 6044 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_SADDR; }
#line 16762 "src/parser_bison.c"
    break;

  case 1219: /* ip6_hdr_field: "daddr"  */
#line 6045 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DADDR; }
#line 16768 "src/parser_bison.c"
    break;

  case 1220: /* icmp6_hdr_expr: "icmpv6" icmp6_hdr_field close_scope_icmp  */
#line 6048 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp6, (yyvsp[-1].val));
			}
#line 16776 "src/parser_bison.c"
    break;

  case 1221: /* icmp6_hdr_field: "type" close_scope_type  */
#line 6053 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMP6HDR_TYPE; }
#line 16782 "src/parser_bison.c"
    break;

  case 1222: /* icmp6_hdr_field: "code"  */
#line 6054 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CODE; }
#line 16788 "src/parser_bison.c"
    break;

  case 1223: /* icmp6_hdr_field: "checksum"  */
#line 6055 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CHECKSUM; }
#line 16794 "src/parser_bison.c"
    break;

  case 1224: /* icmp6_hdr_field: "param-problem"  */
#line 6056 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_PPTR; }
#line 16800 "src/parser_bison.c"
    break;

  case 1225: /* icmp6_hdr_field: "mtu"  */
#line 6057 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MTU; }
#line 16806 "src/parser_bison.c"
    break;

  case 1226: /* icmp6_hdr_field: "id"  */
#line 6058 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_ID; }
#line 16812 "src/parser_bison.c"
    break;

  case 1227: /* icmp6_hdr_field: "seq"  */
#line 6059 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_SEQ; }
#line 16818 "src/parser_bison.c"
    break;

  case 1228: /* icmp6_hdr_field: "max-delay"  */
#line 6060 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MAXDELAY; }
#line 16824 "src/parser_bison.c"
    break;

  case 1229: /* icmp6_hdr_field: "taddr"  */
#line 6061 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_TADDR; }
#line 16830 "src/parser_bison.c"
    break;

  case 1230: /* icmp6_hdr_field: "daddr"  */
#line 6062 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_DADDR; }
#line 16836 "src/parser_bison.c"
    break;

  case 1231: /* auth_hdr_expr: "ah" auth_hdr_field close_scope_ah  */
#line 6066 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ah, (yyvsp[-1].val));
			}
#line 16844 "src/parser_bison.c"
    break;

  case 1232: /* auth_hdr_field: "nexthdr"  */
#line 6071 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_NEXTHDR; }
#line 16850 "src/parser_bison.c"
    break;

  case 1233: /* auth_hdr_field: "hdrlength"  */
#line 6072 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_HDRLENGTH; }
#line 16856 "src/parser_bison.c"
    break;

  case 1234: /* auth_hdr_field: "reserved"  */
#line 6073 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_RESERVED; }
#line 16862 "src/parser_bison.c"
    break;

  case 1235: /* auth_hdr_field: "spi"  */
#line 6074 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SPI; }
#line 16868 "src/parser_bison.c"
    break;

  case 1236: /* auth_hdr_field: "seq"  */
#line 6075 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SEQUENCE; }
#line 16874 "src/parser_bison.c"
    break;

  case 1237: /* esp_hdr_expr: "esp" esp_hdr_field close_scope_esp  */
#line 6079 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_esp, (yyvsp[-1].val));
			}
#line 16882 "src/parser_bison.c"
    break;

  case 1238: /* esp_hdr_field: "spi"  */
#line 6084 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SPI; }
#line 16888 "src/parser_bison.c"
    break;

  case 1239: /* esp_hdr_field: "seq"  */
#line 6085 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SEQUENCE; }
#line 16894 "src/parser_bison.c"
    break;

  case 1240: /* comp_hdr_expr: "comp" comp_hdr_field close_scope_comp  */
#line 6089 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_comp, (yyvsp[-1].val));
			}
#line 16902 "src/parser_bison.c"
    break;

  case 1241: /* comp_hdr_field: "nexthdr"  */
#line 6094 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_NEXTHDR; }
#line 16908 "src/parser_bison.c"
    break;

  case 1242: /* comp_hdr_field: "flags"  */
#line 6095 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_FLAGS; }
#line 16914 "src/parser_bison.c"
    break;

  case 1243: /* comp_hdr_field: "cpi"  */
#line 6096 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_CPI; }
#line 16920 "src/parser_bison.c"
    break;

  case 1244: /* udp_hdr_expr: "udp" udp_hdr_field close_scope_udp  */
#line 6100 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udp, (yyvsp[-1].val));
			}
#line 16928 "src/parser_bison.c"
    break;

  case 1245: /* udp_hdr_field: "sport"  */
#line 6105 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 16934 "src/parser_bison.c"
    break;

  case 1246: /* udp_hdr_field: "dport"  */
#line 6106 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 16940 "src/parser_bison.c"
    break;

  case 1247: /* udp_hdr_field: "length"  */
#line 6107 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 16946 "src/parser_bison.c"
    break;

  case 1248: /* udp_hdr_field: "checksum"  */
#line 6108 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 16952 "src/parser_bison.c"
    break;

  case 1249: /* udplite_hdr_expr: "udplite" udplite_hdr_field close_scope_udplite  */
#line 6112 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udplite, (yyvsp[-1].val));
			}
#line 16960 "src/parser_bison.c"
    break;

  case 1250: /* udplite_hdr_field: "sport"  */
#line 6117 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 16966 "src/parser_bison.c"
    break;

  case 1251: /* udplite_hdr_field: "dport"  */
#line 6118 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 16972 "src/parser_bison.c"
    break;

  case 1252: /* udplite_hdr_field: "csumcov"  */
#line 6119 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 16978 "src/parser_bison.c"
    break;

  case 1253: /* udplite_hdr_field: "checksum"  */
#line 6120 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 16984 "src/parser_bison.c"
    break;

  case 1254: /* tcp_hdr_expr: "tcp" tcp_hdr_field  */
#line 6124 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_tcp, (yyvsp[0].val));
			}
#line 16992 "src/parser_bison.c"
    break;

  case 1255: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_type  */
#line 6128 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].val), TCPOPT_COMMON_KIND);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17001 "src/parser_bison.c"
    break;

  case 1256: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_kind_and_field  */
#line 6133 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].tcp_kind_field).kind, (yyvsp[0].tcp_kind_field).field);
				if ((yyval.expr) == NULL) {
					erec_queue(error(&(yylsp[-2]), "Could not find a tcp option template"), state->msgs);
					YYERROR;
				}
			}
#line 17013 "src/parser_bison.c"
    break;

  case 1257: /* tcp_hdr_expr: "tcp" "option" "@" close_scope_at tcp_hdr_option_type "comma" "number" "comma" payload_raw_len  */
#line 6141 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[-4].val), 0);
				tcpopt_init_raw((yyval.expr), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[0].val), 0);
			}
#line 17022 "src/parser_bison.c"
    break;

  case 1277: /* vxlan_hdr_expr: "vxlan" vxlan_hdr_field  */
#line 6173 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_vxlan, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_vxlan;
				(yyval.expr) = expr;
			}
#line 17034 "src/parser_bison.c"
    break;

  case 1278: /* vxlan_hdr_expr: "vxlan" inner_expr  */
#line 6181 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_vxlan;
			}
#line 17044 "src/parser_bison.c"
    break;

  case 1279: /* vxlan_hdr_field: "vni"  */
#line 6188 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_VNI; }
#line 17050 "src/parser_bison.c"
    break;

  case 1280: /* vxlan_hdr_field: "flags"  */
#line 6189 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_FLAGS; }
#line 17056 "src/parser_bison.c"
    break;

  case 1281: /* geneve_hdr_expr: "geneve" geneve_hdr_field  */
#line 6193 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_geneve, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_geneve;
				(yyval.expr) = expr;
			}
#line 17068 "src/parser_bison.c"
    break;

  case 1282: /* geneve_hdr_expr: "geneve" inner_expr  */
#line 6201 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_geneve;
			}
#line 17078 "src/parser_bison.c"
    break;

  case 1283: /* geneve_hdr_field: "vni"  */
#line 6208 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_VNI; }
#line 17084 "src/parser_bison.c"
    break;

  case 1284: /* geneve_hdr_field: "type"  */
#line 6209 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_TYPE; }
#line 17090 "src/parser_bison.c"
    break;

  case 1285: /* gre_hdr_expr: "gre" gre_hdr_field close_scope_gre  */
#line 6213 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_gre, (yyvsp[-1].val));
			}
#line 17098 "src/parser_bison.c"
    break;

  case 1286: /* gre_hdr_expr: "gre" close_scope_gre inner_inet_expr  */
#line 6217 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gre;
			}
#line 17107 "src/parser_bison.c"
    break;

  case 1287: /* gre_hdr_field: "version"  */
#line 6223 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_VERSION;	}
#line 17113 "src/parser_bison.c"
    break;

  case 1288: /* gre_hdr_field: "flags"  */
#line 6224 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_FLAGS; }
#line 17119 "src/parser_bison.c"
    break;

  case 1289: /* gre_hdr_field: "protocol"  */
#line 6225 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_PROTOCOL; }
#line 17125 "src/parser_bison.c"
    break;

  case 1290: /* gretap_hdr_expr: "gretap" close_scope_gre inner_expr  */
#line 6229 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gretap;
			}
#line 17134 "src/parser_bison.c"
    break;

  case 1291: /* optstrip_stmt: "reset" "tcp" "option" tcp_hdr_option_type close_scope_tcp  */
#line 6236 "src/parser_bison.y"
                        {
				(yyval.stmt) = optstrip_stmt_alloc(&(yyloc), tcpopt_expr_alloc(&(yyloc),
										(yyvsp[-1].val), TCPOPT_COMMON_KIND));
			}
#line 17143 "src/parser_bison.c"
    break;

  case 1292: /* tcp_hdr_field: "sport"  */
#line 6242 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SPORT; }
#line 17149 "src/parser_bison.c"
    break;

  case 1293: /* tcp_hdr_field: "dport"  */
#line 6243 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DPORT; }
#line 17155 "src/parser_bison.c"
    break;

  case 1294: /* tcp_hdr_field: "seq"  */
#line 6244 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SEQ; }
#line 17161 "src/parser_bison.c"
    break;

  case 1295: /* tcp_hdr_field: "ackseq"  */
#line 6245 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_ACKSEQ; }
#line 17167 "src/parser_bison.c"
    break;

  case 1296: /* tcp_hdr_field: "doff"  */
#line 6246 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DOFF; }
#line 17173 "src/parser_bison.c"
    break;

  case 1297: /* tcp_hdr_field: "reserved"  */
#line 6247 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_RESERVED; }
#line 17179 "src/parser_bison.c"
    break;

  case 1298: /* tcp_hdr_field: "flags"  */
#line 6248 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_FLAGS; }
#line 17185 "src/parser_bison.c"
    break;

  case 1299: /* tcp_hdr_field: "window"  */
#line 6249 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_WINDOW; }
#line 17191 "src/parser_bison.c"
    break;

  case 1300: /* tcp_hdr_field: "checksum"  */
#line 6250 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_CHECKSUM; }
#line 17197 "src/parser_bison.c"
    break;

  case 1301: /* tcp_hdr_field: "urgptr"  */
#line 6251 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_URGPTR; }
#line 17203 "src/parser_bison.c"
    break;

  case 1302: /* tcp_hdr_option_kind_and_field: "mss" tcpopt_field_maxseg  */
#line 6255 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MAXSEG, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17212 "src/parser_bison.c"
    break;

  case 1303: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_sack tcpopt_field_sack  */
#line 6260 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17221 "src/parser_bison.c"
    break;

  case 1304: /* tcp_hdr_option_kind_and_field: "window" tcpopt_field_window  */
#line 6265 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_WINDOW, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17230 "src/parser_bison.c"
    break;

  case 1305: /* tcp_hdr_option_kind_and_field: "timestamp" tcpopt_field_tsopt  */
#line 6270 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_TIMESTAMP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17239 "src/parser_bison.c"
    break;

  case 1306: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_type "length"  */
#line 6275 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = TCPOPT_COMMON_LENGTH };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17248 "src/parser_bison.c"
    break;

  case 1307: /* tcp_hdr_option_kind_and_field: "mptcp" tcpopt_field_mptcp  */
#line 6280 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MPTCP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17257 "src/parser_bison.c"
    break;

  case 1308: /* tcp_hdr_option_sack: "sack"  */
#line 6286 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 17263 "src/parser_bison.c"
    break;

  case 1309: /* tcp_hdr_option_sack: "sack0"  */
#line 6287 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 17269 "src/parser_bison.c"
    break;

  case 1310: /* tcp_hdr_option_sack: "sack1"  */
#line 6288 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK1; }
#line 17275 "src/parser_bison.c"
    break;

  case 1311: /* tcp_hdr_option_sack: "sack2"  */
#line 6289 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK2; }
#line 17281 "src/parser_bison.c"
    break;

  case 1312: /* tcp_hdr_option_sack: "sack3"  */
#line 6290 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK3; }
#line 17287 "src/parser_bison.c"
    break;

  case 1313: /* tcp_hdr_option_type: "echo"  */
#line 6293 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_ECHO; }
#line 17293 "src/parser_bison.c"
    break;

  case 1314: /* tcp_hdr_option_type: "eol"  */
#line 6294 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_EOL; }
#line 17299 "src/parser_bison.c"
    break;

  case 1315: /* tcp_hdr_option_type: "fastopen"  */
#line 6295 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_FASTOPEN; }
#line 17305 "src/parser_bison.c"
    break;

  case 1316: /* tcp_hdr_option_type: "md5sig"  */
#line 6296 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MD5SIG; }
#line 17311 "src/parser_bison.c"
    break;

  case 1317: /* tcp_hdr_option_type: "mptcp"  */
#line 6297 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MPTCP; }
#line 17317 "src/parser_bison.c"
    break;

  case 1318: /* tcp_hdr_option_type: "mss"  */
#line 6298 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MAXSEG; }
#line 17323 "src/parser_bison.c"
    break;

  case 1319: /* tcp_hdr_option_type: "nop"  */
#line 6299 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_NOP; }
#line 17329 "src/parser_bison.c"
    break;

  case 1320: /* tcp_hdr_option_type: "sack-permitted"  */
#line 6300 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_SACK_PERMITTED; }
#line 17335 "src/parser_bison.c"
    break;

  case 1321: /* tcp_hdr_option_type: "timestamp"  */
#line 6301 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_TIMESTAMP; }
#line 17341 "src/parser_bison.c"
    break;

  case 1322: /* tcp_hdr_option_type: "window"  */
#line 6302 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_WINDOW; }
#line 17347 "src/parser_bison.c"
    break;

  case 1323: /* tcp_hdr_option_type: tcp_hdr_option_sack  */
#line 6303 "src/parser_bison.y"
                                                        { (yyval.val) = (yyvsp[0].val); }
#line 17353 "src/parser_bison.c"
    break;

  case 1324: /* tcp_hdr_option_type: "number"  */
#line 6304 "src/parser_bison.y"
                                                        {
				if ((yyvsp[0].val) > 255) {
					erec_queue(error(&(yylsp[0]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.val) = (yyvsp[0].val);
			}
#line 17365 "src/parser_bison.c"
    break;

  case 1325: /* tcpopt_field_sack: "left"  */
#line 6313 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_LEFT; }
#line 17371 "src/parser_bison.c"
    break;

  case 1326: /* tcpopt_field_sack: "right"  */
#line 6314 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_RIGHT; }
#line 17377 "src/parser_bison.c"
    break;

  case 1327: /* tcpopt_field_window: "count"  */
#line 6317 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_WINDOW_COUNT; }
#line 17383 "src/parser_bison.c"
    break;

  case 1328: /* tcpopt_field_tsopt: "tsval"  */
#line 6320 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSVAL; }
#line 17389 "src/parser_bison.c"
    break;

  case 1329: /* tcpopt_field_tsopt: "tsecr"  */
#line 6321 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSECR; }
#line 17395 "src/parser_bison.c"
    break;

  case 1330: /* tcpopt_field_maxseg: "size"  */
#line 6324 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MAXSEG_SIZE; }
#line 17401 "src/parser_bison.c"
    break;

  case 1331: /* tcpopt_field_mptcp: "subtype"  */
#line 6327 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MPTCP_SUBTYPE; }
#line 17407 "src/parser_bison.c"
    break;

  case 1332: /* dccp_hdr_expr: "dccp" dccp_hdr_field close_scope_dccp  */
#line 6331 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_dccp, (yyvsp[-1].val));
			}
#line 17415 "src/parser_bison.c"
    break;

  case 1333: /* dccp_hdr_expr: "dccp" "option" "number" close_scope_dccp  */
#line 6335 "src/parser_bison.y"
                        {
				if ((yyvsp[-1].val) > DCCPOPT_TYPE_MAX) {
					erec_queue(error(&(yylsp[-3]), "value too large"),
						   state->msgs);
					YYERROR;
				}
				(yyval.expr) = dccpopt_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 17428 "src/parser_bison.c"
    break;

  case 1334: /* dccp_hdr_field: "sport"  */
#line 6345 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_SPORT; }
#line 17434 "src/parser_bison.c"
    break;

  case 1335: /* dccp_hdr_field: "dport"  */
#line 6346 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_DPORT; }
#line 17440 "src/parser_bison.c"
    break;

  case 1336: /* dccp_hdr_field: "type" close_scope_type  */
#line 6347 "src/parser_bison.y"
                                                                        { (yyval.val) = DCCPHDR_TYPE; }
#line 17446 "src/parser_bison.c"
    break;

  case 1337: /* sctp_chunk_type: "data"  */
#line 6350 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_DATA; }
#line 17452 "src/parser_bison.c"
    break;

  case 1338: /* sctp_chunk_type: "init"  */
#line 6351 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT; }
#line 17458 "src/parser_bison.c"
    break;

  case 1339: /* sctp_chunk_type: "init-ack"  */
#line 6352 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT_ACK; }
#line 17464 "src/parser_bison.c"
    break;

  case 1340: /* sctp_chunk_type: "sack"  */
#line 6353 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SACK; }
#line 17470 "src/parser_bison.c"
    break;

  case 1341: /* sctp_chunk_type: "heartbeat"  */
#line 6354 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT; }
#line 17476 "src/parser_bison.c"
    break;

  case 1342: /* sctp_chunk_type: "heartbeat-ack"  */
#line 6355 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT_ACK; }
#line 17482 "src/parser_bison.c"
    break;

  case 1343: /* sctp_chunk_type: "abort"  */
#line 6356 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ABORT; }
#line 17488 "src/parser_bison.c"
    break;

  case 1344: /* sctp_chunk_type: "shutdown"  */
#line 6357 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN; }
#line 17494 "src/parser_bison.c"
    break;

  case 1345: /* sctp_chunk_type: "shutdown-ack"  */
#line 6358 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_ACK; }
#line 17500 "src/parser_bison.c"
    break;

  case 1346: /* sctp_chunk_type: "error"  */
#line 6359 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ERROR; }
#line 17506 "src/parser_bison.c"
    break;

  case 1347: /* sctp_chunk_type: "cookie-echo"  */
#line 6360 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ECHO; }
#line 17512 "src/parser_bison.c"
    break;

  case 1348: /* sctp_chunk_type: "cookie-ack"  */
#line 6361 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ACK; }
#line 17518 "src/parser_bison.c"
    break;

  case 1349: /* sctp_chunk_type: "ecne"  */
#line 6362 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ECNE; }
#line 17524 "src/parser_bison.c"
    break;

  case 1350: /* sctp_chunk_type: "cwr"  */
#line 6363 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_CWR; }
#line 17530 "src/parser_bison.c"
    break;

  case 1351: /* sctp_chunk_type: "shutdown-complete"  */
#line 6364 "src/parser_bison.y"
                                                  { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_COMPLETE; }
#line 17536 "src/parser_bison.c"
    break;

  case 1352: /* sctp_chunk_type: "asconf-ack"  */
#line 6365 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF_ACK; }
#line 17542 "src/parser_bison.c"
    break;

  case 1353: /* sctp_chunk_type: "forward-tsn"  */
#line 6366 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_FORWARD_TSN; }
#line 17548 "src/parser_bison.c"
    break;

  case 1354: /* sctp_chunk_type: "asconf"  */
#line 6367 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF; }
#line 17554 "src/parser_bison.c"
    break;

  case 1355: /* sctp_chunk_common_field: "type" close_scope_type  */
#line 6370 "src/parser_bison.y"
                                                                { (yyval.val) = SCTP_CHUNK_COMMON_TYPE; }
#line 17560 "src/parser_bison.c"
    break;

  case 1356: /* sctp_chunk_common_field: "flags"  */
#line 6371 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_FLAGS; }
#line 17566 "src/parser_bison.c"
    break;

  case 1357: /* sctp_chunk_common_field: "length"  */
#line 6372 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_LENGTH; }
#line 17572 "src/parser_bison.c"
    break;

  case 1358: /* sctp_chunk_data_field: "tsn"  */
#line 6375 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_TSN; }
#line 17578 "src/parser_bison.c"
    break;

  case 1359: /* sctp_chunk_data_field: "stream"  */
#line 6376 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_STREAM; }
#line 17584 "src/parser_bison.c"
    break;

  case 1360: /* sctp_chunk_data_field: "ssn"  */
#line 6377 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_SSN; }
#line 17590 "src/parser_bison.c"
    break;

  case 1361: /* sctp_chunk_data_field: "ppid"  */
#line 6378 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_PPID; }
#line 17596 "src/parser_bison.c"
    break;

  case 1362: /* sctp_chunk_init_field: "init-tag"  */
#line 6381 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TAG; }
#line 17602 "src/parser_bison.c"
    break;

  case 1363: /* sctp_chunk_init_field: "a-rwnd"  */
#line 6382 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_RWND; }
#line 17608 "src/parser_bison.c"
    break;

  case 1364: /* sctp_chunk_init_field: "num-outbound-streams"  */
#line 6383 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_OSTREAMS; }
#line 17614 "src/parser_bison.c"
    break;

  case 1365: /* sctp_chunk_init_field: "num-inbound-streams"  */
#line 6384 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_ISTREAMS; }
#line 17620 "src/parser_bison.c"
    break;

  case 1366: /* sctp_chunk_init_field: "initial-tsn"  */
#line 6385 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TSN; }
#line 17626 "src/parser_bison.c"
    break;

  case 1367: /* sctp_chunk_sack_field: "cum-tsn-ack"  */
#line 6388 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_CTSN_ACK; }
#line 17632 "src/parser_bison.c"
    break;

  case 1368: /* sctp_chunk_sack_field: "a-rwnd"  */
#line 6389 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_RWND; }
#line 17638 "src/parser_bison.c"
    break;

  case 1369: /* sctp_chunk_sack_field: "num-gap-ack-blocks"  */
#line 6390 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_GACK_BLOCKS; }
#line 17644 "src/parser_bison.c"
    break;

  case 1370: /* sctp_chunk_sack_field: "num-dup-tsns"  */
#line 6391 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_DUP_TSNS; }
#line 17650 "src/parser_bison.c"
    break;

  case 1371: /* sctp_chunk_alloc: sctp_chunk_type  */
#line 6395 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[0].val), SCTP_CHUNK_COMMON_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17659 "src/parser_bison.c"
    break;

  case 1372: /* sctp_chunk_alloc: sctp_chunk_type sctp_chunk_common_field  */
#line 6400 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[0].val));
			}
#line 17667 "src/parser_bison.c"
    break;

  case 1373: /* sctp_chunk_alloc: "data" sctp_chunk_data_field  */
#line 6404 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_DATA, (yyvsp[0].val));
			}
#line 17675 "src/parser_bison.c"
    break;

  case 1374: /* sctp_chunk_alloc: "init" sctp_chunk_init_field  */
#line 6408 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT, (yyvsp[0].val));
			}
#line 17683 "src/parser_bison.c"
    break;

  case 1375: /* sctp_chunk_alloc: "init-ack" sctp_chunk_init_field  */
#line 6412 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT_ACK, (yyvsp[0].val));
			}
#line 17691 "src/parser_bison.c"
    break;

  case 1376: /* sctp_chunk_alloc: "sack" sctp_chunk_sack_field  */
#line 6416 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SACK, (yyvsp[0].val));
			}
#line 17699 "src/parser_bison.c"
    break;

  case 1377: /* sctp_chunk_alloc: "shutdown" "cum-tsn-ack"  */
#line 6420 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SHUTDOWN,
							   SCTP_CHUNK_SHUTDOWN_CTSN_ACK);
			}
#line 17708 "src/parser_bison.c"
    break;

  case 1378: /* sctp_chunk_alloc: "ecne" "lowest-tsn"  */
#line 6425 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ECNE,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 17717 "src/parser_bison.c"
    break;

  case 1379: /* sctp_chunk_alloc: "cwr" "lowest-tsn"  */
#line 6430 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_CWR,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 17726 "src/parser_bison.c"
    break;

  case 1380: /* sctp_chunk_alloc: "asconf-ack" "seqno"  */
#line 6435 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF_ACK,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 17735 "src/parser_bison.c"
    break;

  case 1381: /* sctp_chunk_alloc: "forward-tsn" "new-cum-tsn"  */
#line 6440 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_FORWARD_TSN,
							   SCTP_CHUNK_FORWARD_TSN_NCTSN);
			}
#line 17744 "src/parser_bison.c"
    break;

  case 1382: /* sctp_chunk_alloc: "asconf" "seqno"  */
#line 6445 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 17753 "src/parser_bison.c"
    break;

  case 1383: /* sctp_hdr_expr: "sctp" sctp_hdr_field close_scope_sctp  */
#line 6452 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_sctp, (yyvsp[-1].val));
			}
#line 17761 "src/parser_bison.c"
    break;

  case 1384: /* sctp_hdr_expr: "sctp" "chunk" sctp_chunk_alloc close_scope_sctp_chunk close_scope_sctp  */
#line 6456 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 17769 "src/parser_bison.c"
    break;

  case 1385: /* sctp_hdr_field: "sport"  */
#line 6461 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_SPORT; }
#line 17775 "src/parser_bison.c"
    break;

  case 1386: /* sctp_hdr_field: "dport"  */
#line 6462 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_DPORT; }
#line 17781 "src/parser_bison.c"
    break;

  case 1387: /* sctp_hdr_field: "vtag"  */
#line 6463 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_VTAG; }
#line 17787 "src/parser_bison.c"
    break;

  case 1388: /* sctp_hdr_field: "checksum"  */
#line 6464 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_CHECKSUM; }
#line 17793 "src/parser_bison.c"
    break;

  case 1389: /* th_hdr_expr: "th" th_hdr_field close_scope_th  */
#line 6468 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_th, (yyvsp[-1].val));
				if ((yyval.expr))
					(yyval.expr)->payload.is_raw = true;
			}
#line 17803 "src/parser_bison.c"
    break;

  case 1390: /* th_hdr_field: "sport"  */
#line 6475 "src/parser_bison.y"
                                                { (yyval.val) = THDR_SPORT; }
#line 17809 "src/parser_bison.c"
    break;

  case 1391: /* th_hdr_field: "dport"  */
#line 6476 "src/parser_bison.y"
                                                { (yyval.val) = THDR_DPORT; }
#line 17815 "src/parser_bison.c"
    break;

  case 1400: /* hbh_hdr_expr: "hbh" hbh_hdr_field close_scope_hbh  */
#line 6490 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_hbh, (yyvsp[-1].val));
			}
#line 17823 "src/parser_bison.c"
    break;

  case 1401: /* hbh_hdr_field: "nexthdr"  */
#line 6495 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_NEXTHDR; }
#line 17829 "src/parser_bison.c"
    break;

  case 1402: /* hbh_hdr_field: "hdrlength"  */
#line 6496 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_HDRLENGTH; }
#line 17835 "src/parser_bison.c"
    break;

  case 1403: /* rt_hdr_expr: "rt" rt_hdr_field close_scope_rt  */
#line 6500 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt, (yyvsp[-1].val));
			}
#line 17843 "src/parser_bison.c"
    break;

  case 1404: /* rt_hdr_field: "nexthdr"  */
#line 6505 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_NEXTHDR; }
#line 17849 "src/parser_bison.c"
    break;

  case 1405: /* rt_hdr_field: "hdrlength"  */
#line 6506 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_HDRLENGTH; }
#line 17855 "src/parser_bison.c"
    break;

  case 1406: /* rt_hdr_field: "type" close_scope_type  */
#line 6507 "src/parser_bison.y"
                                                                        { (yyval.val) = RTHDR_TYPE; }
#line 17861 "src/parser_bison.c"
    break;

  case 1407: /* rt_hdr_field: "seg-left"  */
#line 6508 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_SEG_LEFT; }
#line 17867 "src/parser_bison.c"
    break;

  case 1408: /* rt0_hdr_expr: "rt0" rt0_hdr_field close_scope_rt  */
#line 6512 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt0, (yyvsp[-1].val));
			}
#line 17875 "src/parser_bison.c"
    break;

  case 1409: /* rt0_hdr_field: "addr" '[' "number" ']'  */
#line 6518 "src/parser_bison.y"
                        {
				(yyval.val) = RT0HDR_ADDR_1 + (yyvsp[-1].val) - 1;
			}
#line 17883 "src/parser_bison.c"
    break;

  case 1410: /* rt2_hdr_expr: "rt2" rt2_hdr_field close_scope_rt  */
#line 6524 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt2, (yyvsp[-1].val));
			}
#line 17891 "src/parser_bison.c"
    break;

  case 1411: /* rt2_hdr_field: "addr"  */
#line 6529 "src/parser_bison.y"
                                                { (yyval.val) = RT2HDR_ADDR; }
#line 17897 "src/parser_bison.c"
    break;

  case 1412: /* rt4_hdr_expr: "srh" rt4_hdr_field close_scope_rt  */
#line 6533 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt4, (yyvsp[-1].val));
			}
#line 17905 "src/parser_bison.c"
    break;

  case 1413: /* rt4_hdr_field: "last-entry"  */
#line 6538 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_LASTENT; }
#line 17911 "src/parser_bison.c"
    break;

  case 1414: /* rt4_hdr_field: "flags"  */
#line 6539 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_FLAGS; }
#line 17917 "src/parser_bison.c"
    break;

  case 1415: /* rt4_hdr_field: "tag"  */
#line 6540 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_TAG; }
#line 17923 "src/parser_bison.c"
    break;

  case 1416: /* rt4_hdr_field: "sid" '[' "number" ']'  */
#line 6542 "src/parser_bison.y"
                        {
				(yyval.val) = RT4HDR_SID_1 + (yyvsp[-1].val) - 1;
			}
#line 17931 "src/parser_bison.c"
    break;

  case 1417: /* frag_hdr_expr: "frag" frag_hdr_field close_scope_frag  */
#line 6548 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_frag, (yyvsp[-1].val));
			}
#line 17939 "src/parser_bison.c"
    break;

  case 1418: /* frag_hdr_field: "nexthdr"  */
#line 6553 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_NEXTHDR; }
#line 17945 "src/parser_bison.c"
    break;

  case 1419: /* frag_hdr_field: "reserved"  */
#line 6554 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED; }
#line 17951 "src/parser_bison.c"
    break;

  case 1420: /* frag_hdr_field: "frag-off"  */
#line 6555 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_FRAG_OFF; }
#line 17957 "src/parser_bison.c"
    break;

  case 1421: /* frag_hdr_field: "reserved2"  */
#line 6556 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED2; }
#line 17963 "src/parser_bison.c"
    break;

  case 1422: /* frag_hdr_field: "more-fragments"  */
#line 6557 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_MFRAGS; }
#line 17969 "src/parser_bison.c"
    break;

  case 1423: /* frag_hdr_field: "id"  */
#line 6558 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_ID; }
#line 17975 "src/parser_bison.c"
    break;

  case 1424: /* dst_hdr_expr: "dst" dst_hdr_field close_scope_dst  */
#line 6562 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_dst, (yyvsp[-1].val));
			}
#line 17983 "src/parser_bison.c"
    break;

  case 1425: /* dst_hdr_field: "nexthdr"  */
#line 6567 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_NEXTHDR; }
#line 17989 "src/parser_bison.c"
    break;

  case 1426: /* dst_hdr_field: "hdrlength"  */
#line 6568 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_HDRLENGTH; }
#line 17995 "src/parser_bison.c"
    break;

  case 1427: /* mh_hdr_expr: "mh" mh_hdr_field close_scope_mh  */
#line 6572 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_mh, (yyvsp[-1].val));
			}
#line 18003 "src/parser_bison.c"
    break;

  case 1428: /* mh_hdr_field: "nexthdr"  */
#line 6577 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_NEXTHDR; }
#line 18009 "src/parser_bison.c"
    break;

  case 1429: /* mh_hdr_field: "hdrlength"  */
#line 6578 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_HDRLENGTH; }
#line 18015 "src/parser_bison.c"
    break;

  case 1430: /* mh_hdr_field: "type" close_scope_type  */
#line 6579 "src/parser_bison.y"
                                                                        { (yyval.val) = MHHDR_TYPE; }
#line 18021 "src/parser_bison.c"
    break;

  case 1431: /* mh_hdr_field: "reserved"  */
#line 6580 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_RESERVED; }
#line 18027 "src/parser_bison.c"
    break;

  case 1432: /* mh_hdr_field: "checksum"  */
#line 6581 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_CHECKSUM; }
#line 18033 "src/parser_bison.c"
    break;

  case 1433: /* exthdr_exists_expr: "exthdr" exthdr_key  */
#line 6585 "src/parser_bison.y"
                        {
				const struct exthdr_desc *desc;

				desc = exthdr_find_proto((yyvsp[0].val));

				/* Assume that NEXTHDR template is always
				 * the first one in list of templates.
				 */
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), desc, 1);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 18049 "src/parser_bison.c"
    break;

  case 1434: /* exthdr_key: "hbh" close_scope_hbh  */
#line 6598 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_HOPOPTS; }
#line 18055 "src/parser_bison.c"
    break;

  case 1435: /* exthdr_key: "rt" close_scope_rt  */
#line 6599 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_ROUTING; }
#line 18061 "src/parser_bison.c"
    break;

  case 1436: /* exthdr_key: "frag" close_scope_frag  */
#line 6600 "src/parser_bison.y"
                                                                { (yyval.val) = IPPROTO_FRAGMENT; }
#line 18067 "src/parser_bison.c"
    break;

  case 1437: /* exthdr_key: "dst" close_scope_dst  */
#line 6601 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_DSTOPTS; }
#line 18073 "src/parser_bison.c"
    break;

  case 1438: /* exthdr_key: "mh" close_scope_mh  */
#line 6602 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_MH; }
#line 18079 "src/parser_bison.c"
    break;


#line 18083 "src/parser_bison.c"

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

#line 6605 "src/parser_bison.y"


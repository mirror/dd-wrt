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

#line 1163 "src/parser_bison.c"

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
  YYSYMBOL_NEW = 79,                       /* "new"  */
  YYSYMBOL_DESTROY = 80,                   /* "destroy"  */
  YYSYMBOL_MONITOR = 81,                   /* "monitor"  */
  YYSYMBOL_ALL = 82,                       /* "all"  */
  YYSYMBOL_ACCEPT = 83,                    /* "accept"  */
  YYSYMBOL_DROP = 84,                      /* "drop"  */
  YYSYMBOL_CONTINUE = 85,                  /* "continue"  */
  YYSYMBOL_JUMP = 86,                      /* "jump"  */
  YYSYMBOL_GOTO = 87,                      /* "goto"  */
  YYSYMBOL_RETURN = 88,                    /* "return"  */
  YYSYMBOL_TO = 89,                        /* "to"  */
  YYSYMBOL_CONSTANT = 90,                  /* "constant"  */
  YYSYMBOL_INTERVAL = 91,                  /* "interval"  */
  YYSYMBOL_DYNAMIC = 92,                   /* "dynamic"  */
  YYSYMBOL_AUTOMERGE = 93,                 /* "auto-merge"  */
  YYSYMBOL_TIMEOUT = 94,                   /* "timeout"  */
  YYSYMBOL_GC_INTERVAL = 95,               /* "gc-interval"  */
  YYSYMBOL_ELEMENTS = 96,                  /* "elements"  */
  YYSYMBOL_EXPIRES = 97,                   /* "expires"  */
  YYSYMBOL_POLICY = 98,                    /* "policy"  */
  YYSYMBOL_MEMORY = 99,                    /* "memory"  */
  YYSYMBOL_PERFORMANCE = 100,              /* "performance"  */
  YYSYMBOL_SIZE = 101,                     /* "size"  */
  YYSYMBOL_FLOW = 102,                     /* "flow"  */
  YYSYMBOL_OFFLOAD = 103,                  /* "offload"  */
  YYSYMBOL_METER = 104,                    /* "meter"  */
  YYSYMBOL_METERS = 105,                   /* "meters"  */
  YYSYMBOL_FLOWTABLES = 106,               /* "flowtables"  */
  YYSYMBOL_NUM = 107,                      /* "number"  */
  YYSYMBOL_STRING = 108,                   /* "string"  */
  YYSYMBOL_QUOTED_STRING = 109,            /* "quoted string"  */
  YYSYMBOL_ASTERISK_STRING = 110,          /* "string with a trailing asterisk"  */
  YYSYMBOL_LL_HDR = 111,                   /* "ll"  */
  YYSYMBOL_NETWORK_HDR = 112,              /* "nh"  */
  YYSYMBOL_TRANSPORT_HDR = 113,            /* "th"  */
  YYSYMBOL_BRIDGE = 114,                   /* "bridge"  */
  YYSYMBOL_ETHER = 115,                    /* "ether"  */
  YYSYMBOL_SADDR = 116,                    /* "saddr"  */
  YYSYMBOL_DADDR = 117,                    /* "daddr"  */
  YYSYMBOL_TYPE = 118,                     /* "type"  */
  YYSYMBOL_VLAN = 119,                     /* "vlan"  */
  YYSYMBOL_ID = 120,                       /* "id"  */
  YYSYMBOL_CFI = 121,                      /* "cfi"  */
  YYSYMBOL_DEI = 122,                      /* "dei"  */
  YYSYMBOL_PCP = 123,                      /* "pcp"  */
  YYSYMBOL_ARP = 124,                      /* "arp"  */
  YYSYMBOL_HTYPE = 125,                    /* "htype"  */
  YYSYMBOL_PTYPE = 126,                    /* "ptype"  */
  YYSYMBOL_HLEN = 127,                     /* "hlen"  */
  YYSYMBOL_PLEN = 128,                     /* "plen"  */
  YYSYMBOL_OPERATION = 129,                /* "operation"  */
  YYSYMBOL_IP = 130,                       /* "ip"  */
  YYSYMBOL_HDRVERSION = 131,               /* "version"  */
  YYSYMBOL_HDRLENGTH = 132,                /* "hdrlength"  */
  YYSYMBOL_DSCP = 133,                     /* "dscp"  */
  YYSYMBOL_ECN = 134,                      /* "ecn"  */
  YYSYMBOL_LENGTH = 135,                   /* "length"  */
  YYSYMBOL_FRAG_OFF = 136,                 /* "frag-off"  */
  YYSYMBOL_TTL = 137,                      /* "ttl"  */
  YYSYMBOL_TOS = 138,                      /* "tos"  */
  YYSYMBOL_PROTOCOL = 139,                 /* "protocol"  */
  YYSYMBOL_CHECKSUM = 140,                 /* "checksum"  */
  YYSYMBOL_PTR = 141,                      /* "ptr"  */
  YYSYMBOL_VALUE = 142,                    /* "value"  */
  YYSYMBOL_LSRR = 143,                     /* "lsrr"  */
  YYSYMBOL_RR = 144,                       /* "rr"  */
  YYSYMBOL_SSRR = 145,                     /* "ssrr"  */
  YYSYMBOL_RA = 146,                       /* "ra"  */
  YYSYMBOL_ICMP = 147,                     /* "icmp"  */
  YYSYMBOL_CODE = 148,                     /* "code"  */
  YYSYMBOL_SEQUENCE = 149,                 /* "seq"  */
  YYSYMBOL_GATEWAY = 150,                  /* "gateway"  */
  YYSYMBOL_MTU = 151,                      /* "mtu"  */
  YYSYMBOL_IGMP = 152,                     /* "igmp"  */
  YYSYMBOL_MRT = 153,                      /* "mrt"  */
  YYSYMBOL_OPTIONS = 154,                  /* "options"  */
  YYSYMBOL_IP6 = 155,                      /* "ip6"  */
  YYSYMBOL_PRIORITY = 156,                 /* "priority"  */
  YYSYMBOL_FLOWLABEL = 157,                /* "flowlabel"  */
  YYSYMBOL_NEXTHDR = 158,                  /* "nexthdr"  */
  YYSYMBOL_HOPLIMIT = 159,                 /* "hoplimit"  */
  YYSYMBOL_ICMP6 = 160,                    /* "icmpv6"  */
  YYSYMBOL_PPTR = 161,                     /* "param-problem"  */
  YYSYMBOL_MAXDELAY = 162,                 /* "max-delay"  */
  YYSYMBOL_TADDR = 163,                    /* "taddr"  */
  YYSYMBOL_AH = 164,                       /* "ah"  */
  YYSYMBOL_RESERVED = 165,                 /* "reserved"  */
  YYSYMBOL_SPI = 166,                      /* "spi"  */
  YYSYMBOL_ESP = 167,                      /* "esp"  */
  YYSYMBOL_COMP = 168,                     /* "comp"  */
  YYSYMBOL_FLAGS = 169,                    /* "flags"  */
  YYSYMBOL_CPI = 170,                      /* "cpi"  */
  YYSYMBOL_PORT = 171,                     /* "port"  */
  YYSYMBOL_UDP = 172,                      /* "udp"  */
  YYSYMBOL_SPORT = 173,                    /* "sport"  */
  YYSYMBOL_DPORT = 174,                    /* "dport"  */
  YYSYMBOL_UDPLITE = 175,                  /* "udplite"  */
  YYSYMBOL_CSUMCOV = 176,                  /* "csumcov"  */
  YYSYMBOL_TCP = 177,                      /* "tcp"  */
  YYSYMBOL_ACKSEQ = 178,                   /* "ackseq"  */
  YYSYMBOL_DOFF = 179,                     /* "doff"  */
  YYSYMBOL_WINDOW = 180,                   /* "window"  */
  YYSYMBOL_URGPTR = 181,                   /* "urgptr"  */
  YYSYMBOL_OPTION = 182,                   /* "option"  */
  YYSYMBOL_ECHO = 183,                     /* "echo"  */
  YYSYMBOL_EOL = 184,                      /* "eol"  */
  YYSYMBOL_MPTCP = 185,                    /* "mptcp"  */
  YYSYMBOL_NOP = 186,                      /* "nop"  */
  YYSYMBOL_SACK = 187,                     /* "sack"  */
  YYSYMBOL_SACK0 = 188,                    /* "sack0"  */
  YYSYMBOL_SACK1 = 189,                    /* "sack1"  */
  YYSYMBOL_SACK2 = 190,                    /* "sack2"  */
  YYSYMBOL_SACK3 = 191,                    /* "sack3"  */
  YYSYMBOL_SACK_PERM = 192,                /* "sack-permitted"  */
  YYSYMBOL_FASTOPEN = 193,                 /* "fastopen"  */
  YYSYMBOL_MD5SIG = 194,                   /* "md5sig"  */
  YYSYMBOL_TIMESTAMP = 195,                /* "timestamp"  */
  YYSYMBOL_COUNT = 196,                    /* "count"  */
  YYSYMBOL_LEFT = 197,                     /* "left"  */
  YYSYMBOL_RIGHT = 198,                    /* "right"  */
  YYSYMBOL_TSVAL = 199,                    /* "tsval"  */
  YYSYMBOL_TSECR = 200,                    /* "tsecr"  */
  YYSYMBOL_SUBTYPE = 201,                  /* "subtype"  */
  YYSYMBOL_DCCP = 202,                     /* "dccp"  */
  YYSYMBOL_VXLAN = 203,                    /* "vxlan"  */
  YYSYMBOL_VNI = 204,                      /* "vni"  */
  YYSYMBOL_GRE = 205,                      /* "gre"  */
  YYSYMBOL_GRETAP = 206,                   /* "gretap"  */
  YYSYMBOL_GENEVE = 207,                   /* "geneve"  */
  YYSYMBOL_SCTP = 208,                     /* "sctp"  */
  YYSYMBOL_CHUNK = 209,                    /* "chunk"  */
  YYSYMBOL_DATA = 210,                     /* "data"  */
  YYSYMBOL_INIT = 211,                     /* "init"  */
  YYSYMBOL_INIT_ACK = 212,                 /* "init-ack"  */
  YYSYMBOL_HEARTBEAT = 213,                /* "heartbeat"  */
  YYSYMBOL_HEARTBEAT_ACK = 214,            /* "heartbeat-ack"  */
  YYSYMBOL_ABORT = 215,                    /* "abort"  */
  YYSYMBOL_SHUTDOWN = 216,                 /* "shutdown"  */
  YYSYMBOL_SHUTDOWN_ACK = 217,             /* "shutdown-ack"  */
  YYSYMBOL_ERROR = 218,                    /* "error"  */
  YYSYMBOL_COOKIE_ECHO = 219,              /* "cookie-echo"  */
  YYSYMBOL_COOKIE_ACK = 220,               /* "cookie-ack"  */
  YYSYMBOL_ECNE = 221,                     /* "ecne"  */
  YYSYMBOL_CWR = 222,                      /* "cwr"  */
  YYSYMBOL_SHUTDOWN_COMPLETE = 223,        /* "shutdown-complete"  */
  YYSYMBOL_ASCONF_ACK = 224,               /* "asconf-ack"  */
  YYSYMBOL_FORWARD_TSN = 225,              /* "forward-tsn"  */
  YYSYMBOL_ASCONF = 226,                   /* "asconf"  */
  YYSYMBOL_TSN = 227,                      /* "tsn"  */
  YYSYMBOL_STREAM = 228,                   /* "stream"  */
  YYSYMBOL_SSN = 229,                      /* "ssn"  */
  YYSYMBOL_PPID = 230,                     /* "ppid"  */
  YYSYMBOL_INIT_TAG = 231,                 /* "init-tag"  */
  YYSYMBOL_A_RWND = 232,                   /* "a-rwnd"  */
  YYSYMBOL_NUM_OSTREAMS = 233,             /* "num-outbound-streams"  */
  YYSYMBOL_NUM_ISTREAMS = 234,             /* "num-inbound-streams"  */
  YYSYMBOL_INIT_TSN = 235,                 /* "initial-tsn"  */
  YYSYMBOL_CUM_TSN_ACK = 236,              /* "cum-tsn-ack"  */
  YYSYMBOL_NUM_GACK_BLOCKS = 237,          /* "num-gap-ack-blocks"  */
  YYSYMBOL_NUM_DUP_TSNS = 238,             /* "num-dup-tsns"  */
  YYSYMBOL_LOWEST_TSN = 239,               /* "lowest-tsn"  */
  YYSYMBOL_SEQNO = 240,                    /* "seqno"  */
  YYSYMBOL_NEW_CUM_TSN = 241,              /* "new-cum-tsn"  */
  YYSYMBOL_VTAG = 242,                     /* "vtag"  */
  YYSYMBOL_RT = 243,                       /* "rt"  */
  YYSYMBOL_RT0 = 244,                      /* "rt0"  */
  YYSYMBOL_RT2 = 245,                      /* "rt2"  */
  YYSYMBOL_RT4 = 246,                      /* "srh"  */
  YYSYMBOL_SEG_LEFT = 247,                 /* "seg-left"  */
  YYSYMBOL_ADDR = 248,                     /* "addr"  */
  YYSYMBOL_LAST_ENT = 249,                 /* "last-entry"  */
  YYSYMBOL_TAG = 250,                      /* "tag"  */
  YYSYMBOL_SID = 251,                      /* "sid"  */
  YYSYMBOL_HBH = 252,                      /* "hbh"  */
  YYSYMBOL_FRAG = 253,                     /* "frag"  */
  YYSYMBOL_RESERVED2 = 254,                /* "reserved2"  */
  YYSYMBOL_MORE_FRAGMENTS = 255,           /* "more-fragments"  */
  YYSYMBOL_DST = 256,                      /* "dst"  */
  YYSYMBOL_MH = 257,                       /* "mh"  */
  YYSYMBOL_META = 258,                     /* "meta"  */
  YYSYMBOL_MARK = 259,                     /* "mark"  */
  YYSYMBOL_IIF = 260,                      /* "iif"  */
  YYSYMBOL_IIFNAME = 261,                  /* "iifname"  */
  YYSYMBOL_IIFTYPE = 262,                  /* "iiftype"  */
  YYSYMBOL_OIF = 263,                      /* "oif"  */
  YYSYMBOL_OIFNAME = 264,                  /* "oifname"  */
  YYSYMBOL_OIFTYPE = 265,                  /* "oiftype"  */
  YYSYMBOL_SKUID = 266,                    /* "skuid"  */
  YYSYMBOL_SKGID = 267,                    /* "skgid"  */
  YYSYMBOL_NFTRACE = 268,                  /* "nftrace"  */
  YYSYMBOL_RTCLASSID = 269,                /* "rtclassid"  */
  YYSYMBOL_IBRIDGENAME = 270,              /* "ibrname"  */
  YYSYMBOL_OBRIDGENAME = 271,              /* "obrname"  */
  YYSYMBOL_PKTTYPE = 272,                  /* "pkttype"  */
  YYSYMBOL_CPU = 273,                      /* "cpu"  */
  YYSYMBOL_IIFGROUP = 274,                 /* "iifgroup"  */
  YYSYMBOL_OIFGROUP = 275,                 /* "oifgroup"  */
  YYSYMBOL_CGROUP = 276,                   /* "cgroup"  */
  YYSYMBOL_TIME = 277,                     /* "time"  */
  YYSYMBOL_NFPROTO = 278,                  /* "nfproto"  */
  YYSYMBOL_L4PROTO = 279,                  /* "l4proto"  */
  YYSYMBOL_IIFKIND = 280,                  /* "iifkind"  */
  YYSYMBOL_OIFKIND = 281,                  /* "oifkind"  */
  YYSYMBOL_IBRPVID = 282,                  /* "ibrpvid"  */
  YYSYMBOL_IBRVPROTO = 283,                /* "ibrvproto"  */
  YYSYMBOL_SDIF = 284,                     /* "sdif"  */
  YYSYMBOL_SDIFNAME = 285,                 /* "sdifname"  */
  YYSYMBOL_BROUTE = 286,                   /* "broute"  */
  YYSYMBOL_BRIFHWADDR = 287,               /* "ibrhwaddr"  */
  YYSYMBOL_CLASSID = 288,                  /* "classid"  */
  YYSYMBOL_NEXTHOP = 289,                  /* "nexthop"  */
  YYSYMBOL_CT = 290,                       /* "ct"  */
  YYSYMBOL_L3PROTOCOL = 291,               /* "l3proto"  */
  YYSYMBOL_PROTO_SRC = 292,                /* "proto-src"  */
  YYSYMBOL_PROTO_DST = 293,                /* "proto-dst"  */
  YYSYMBOL_ZONE = 294,                     /* "zone"  */
  YYSYMBOL_DIRECTION = 295,                /* "direction"  */
  YYSYMBOL_EVENT = 296,                    /* "event"  */
  YYSYMBOL_EXPECTATION = 297,              /* "expectation"  */
  YYSYMBOL_EXPIRATION = 298,               /* "expiration"  */
  YYSYMBOL_HELPER = 299,                   /* "helper"  */
  YYSYMBOL_LABEL = 300,                    /* "label"  */
  YYSYMBOL_STATE = 301,                    /* "state"  */
  YYSYMBOL_STATUS = 302,                   /* "status"  */
  YYSYMBOL_ORIGINAL = 303,                 /* "original"  */
  YYSYMBOL_REPLY = 304,                    /* "reply"  */
  YYSYMBOL_COUNTER = 305,                  /* "counter"  */
  YYSYMBOL_NAME = 306,                     /* "name"  */
  YYSYMBOL_PACKETS = 307,                  /* "packets"  */
  YYSYMBOL_BYTES = 308,                    /* "bytes"  */
  YYSYMBOL_KBYTES = 309,                   /* "kbytes"  */
  YYSYMBOL_MBYTES = 310,                   /* "mbytes"  */
  YYSYMBOL_AVGPKT = 311,                   /* "avgpkt"  */
  YYSYMBOL_LAST = 312,                     /* "last"  */
  YYSYMBOL_NEVER = 313,                    /* "never"  */
  YYSYMBOL_TUNNEL = 314,                   /* "tunnel"  */
  YYSYMBOL_ERSPAN = 315,                   /* "erspan"  */
  YYSYMBOL_EGRESS = 316,                   /* "egress"  */
  YYSYMBOL_INGRESS = 317,                  /* "ingress"  */
  YYSYMBOL_GBP = 318,                      /* "gbp"  */
  YYSYMBOL_CLASS = 319,                    /* "class"  */
  YYSYMBOL_OPTTYPE = 320,                  /* "opt-type"  */
  YYSYMBOL_COUNTERS = 321,                 /* "counters"  */
  YYSYMBOL_QUOTAS = 322,                   /* "quotas"  */
  YYSYMBOL_LIMITS = 323,                   /* "limits"  */
  YYSYMBOL_TUNNELS = 324,                  /* "tunnels"  */
  YYSYMBOL_SYNPROXYS = 325,                /* "synproxys"  */
  YYSYMBOL_COUNTS = 326,                   /* "counts"  */
  YYSYMBOL_HELPERS = 327,                  /* "helpers"  */
  YYSYMBOL_LOG = 328,                      /* "log"  */
  YYSYMBOL_PREFIX = 329,                   /* "prefix"  */
  YYSYMBOL_GROUP = 330,                    /* "group"  */
  YYSYMBOL_SNAPLEN = 331,                  /* "snaplen"  */
  YYSYMBOL_QUEUE_THRESHOLD = 332,          /* "queue-threshold"  */
  YYSYMBOL_LEVEL = 333,                    /* "level"  */
  YYSYMBOL_EMERG = 334,                    /* "emerg"  */
  YYSYMBOL_ALERT = 335,                    /* "alert"  */
  YYSYMBOL_CRIT = 336,                     /* "crit"  */
  YYSYMBOL_ERR = 337,                      /* "err"  */
  YYSYMBOL_WARN = 338,                     /* "warn"  */
  YYSYMBOL_NOTICE = 339,                   /* "notice"  */
  YYSYMBOL_INFO = 340,                     /* "info"  */
  YYSYMBOL_DEBUG_TOKEN = 341,              /* "debug"  */
  YYSYMBOL_AUDIT = 342,                    /* "audit"  */
  YYSYMBOL_LIMIT = 343,                    /* "limit"  */
  YYSYMBOL_RATE = 344,                     /* "rate"  */
  YYSYMBOL_BURST = 345,                    /* "burst"  */
  YYSYMBOL_OVER = 346,                     /* "over"  */
  YYSYMBOL_UNTIL = 347,                    /* "until"  */
  YYSYMBOL_QUOTA = 348,                    /* "quota"  */
  YYSYMBOL_USED = 349,                     /* "used"  */
  YYSYMBOL_SECMARK = 350,                  /* "secmark"  */
  YYSYMBOL_SECMARKS = 351,                 /* "secmarks"  */
  YYSYMBOL_SECOND = 352,                   /* "second"  */
  YYSYMBOL_MINUTE = 353,                   /* "minute"  */
  YYSYMBOL_HOUR = 354,                     /* "hour"  */
  YYSYMBOL_DAY = 355,                      /* "day"  */
  YYSYMBOL_WEEK = 356,                     /* "week"  */
  YYSYMBOL__REJECT = 357,                  /* "reject"  */
  YYSYMBOL_WITH = 358,                     /* "with"  */
  YYSYMBOL_ICMPX = 359,                    /* "icmpx"  */
  YYSYMBOL_SNAT = 360,                     /* "snat"  */
  YYSYMBOL_DNAT = 361,                     /* "dnat"  */
  YYSYMBOL_MASQUERADE = 362,               /* "masquerade"  */
  YYSYMBOL_REDIRECT = 363,                 /* "redirect"  */
  YYSYMBOL_RANDOM = 364,                   /* "random"  */
  YYSYMBOL_FULLY_RANDOM = 365,             /* "fully-random"  */
  YYSYMBOL_PERSISTENT = 366,               /* "persistent"  */
  YYSYMBOL_QUEUE = 367,                    /* "queue"  */
  YYSYMBOL_QUEUENUM = 368,                 /* "num"  */
  YYSYMBOL_BYPASS = 369,                   /* "bypass"  */
  YYSYMBOL_FANOUT = 370,                   /* "fanout"  */
  YYSYMBOL_DUP = 371,                      /* "dup"  */
  YYSYMBOL_FWD = 372,                      /* "fwd"  */
  YYSYMBOL_NUMGEN = 373,                   /* "numgen"  */
  YYSYMBOL_INC = 374,                      /* "inc"  */
  YYSYMBOL_MOD = 375,                      /* "mod"  */
  YYSYMBOL_OFFSET = 376,                   /* "offset"  */
  YYSYMBOL_JHASH = 377,                    /* "jhash"  */
  YYSYMBOL_SYMHASH = 378,                  /* "symhash"  */
  YYSYMBOL_SEED = 379,                     /* "seed"  */
  YYSYMBOL_POSITION = 380,                 /* "position"  */
  YYSYMBOL_INDEX = 381,                    /* "index"  */
  YYSYMBOL_COMMENT = 382,                  /* "comment"  */
  YYSYMBOL_XML = 383,                      /* "xml"  */
  YYSYMBOL_JSON = 384,                     /* "json"  */
  YYSYMBOL_VM = 385,                       /* "vm"  */
  YYSYMBOL_NOTRACK = 386,                  /* "notrack"  */
  YYSYMBOL_EXISTS = 387,                   /* "exists"  */
  YYSYMBOL_MISSING = 388,                  /* "missing"  */
  YYSYMBOL_EXTHDR = 389,                   /* "exthdr"  */
  YYSYMBOL_IPSEC = 390,                    /* "ipsec"  */
  YYSYMBOL_REQID = 391,                    /* "reqid"  */
  YYSYMBOL_SPNUM = 392,                    /* "spnum"  */
  YYSYMBOL_IN = 393,                       /* "in"  */
  YYSYMBOL_OUT = 394,                      /* "out"  */
  YYSYMBOL_XT = 395,                       /* "xt"  */
  YYSYMBOL_FILTER = 396,                   /* "filter"  */
  YYSYMBOL_NAT = 397,                      /* "nat"  */
  YYSYMBOL_ROUTE = 398,                    /* "route"  */
  YYSYMBOL_LOOSE = 399,                    /* "loose"  */
  YYSYMBOL_SKIP = 400,                     /* "skip"  */
  YYSYMBOL_401_ = 401,                     /* '='  */
  YYSYMBOL_402_ = 402,                     /* '{'  */
  YYSYMBOL_403_ = 403,                     /* '}'  */
  YYSYMBOL_404_ = 404,                     /* '('  */
  YYSYMBOL_405_ = 405,                     /* ')'  */
  YYSYMBOL_406_ = 406,                     /* '|'  */
  YYSYMBOL_407_ = 407,                     /* '$'  */
  YYSYMBOL_408_ = 408,                     /* '['  */
  YYSYMBOL_409_ = 409,                     /* ']'  */
  YYSYMBOL_YYACCEPT = 410,                 /* $accept  */
  YYSYMBOL_input = 411,                    /* input  */
  YYSYMBOL_stmt_separator = 412,           /* stmt_separator  */
  YYSYMBOL_opt_newline = 413,              /* opt_newline  */
  YYSYMBOL_close_scope_ah = 414,           /* close_scope_ah  */
  YYSYMBOL_close_scope_arp = 415,          /* close_scope_arp  */
  YYSYMBOL_close_scope_at = 416,           /* close_scope_at  */
  YYSYMBOL_close_scope_comp = 417,         /* close_scope_comp  */
  YYSYMBOL_close_scope_ct = 418,           /* close_scope_ct  */
  YYSYMBOL_close_scope_counter = 419,      /* close_scope_counter  */
  YYSYMBOL_close_scope_last = 420,         /* close_scope_last  */
  YYSYMBOL_close_scope_dccp = 421,         /* close_scope_dccp  */
  YYSYMBOL_close_scope_destroy = 422,      /* close_scope_destroy  */
  YYSYMBOL_close_scope_dst = 423,          /* close_scope_dst  */
  YYSYMBOL_close_scope_dup = 424,          /* close_scope_dup  */
  YYSYMBOL_close_scope_esp = 425,          /* close_scope_esp  */
  YYSYMBOL_close_scope_eth = 426,          /* close_scope_eth  */
  YYSYMBOL_close_scope_export = 427,       /* close_scope_export  */
  YYSYMBOL_close_scope_fib = 428,          /* close_scope_fib  */
  YYSYMBOL_close_scope_frag = 429,         /* close_scope_frag  */
  YYSYMBOL_close_scope_fwd = 430,          /* close_scope_fwd  */
  YYSYMBOL_close_scope_gre = 431,          /* close_scope_gre  */
  YYSYMBOL_close_scope_hash = 432,         /* close_scope_hash  */
  YYSYMBOL_close_scope_hbh = 433,          /* close_scope_hbh  */
  YYSYMBOL_close_scope_ip = 434,           /* close_scope_ip  */
  YYSYMBOL_close_scope_ip6 = 435,          /* close_scope_ip6  */
  YYSYMBOL_close_scope_vlan = 436,         /* close_scope_vlan  */
  YYSYMBOL_close_scope_icmp = 437,         /* close_scope_icmp  */
  YYSYMBOL_close_scope_igmp = 438,         /* close_scope_igmp  */
  YYSYMBOL_close_scope_import = 439,       /* close_scope_import  */
  YYSYMBOL_close_scope_ipsec = 440,        /* close_scope_ipsec  */
  YYSYMBOL_close_scope_list = 441,         /* close_scope_list  */
  YYSYMBOL_close_scope_limit = 442,        /* close_scope_limit  */
  YYSYMBOL_close_scope_meta = 443,         /* close_scope_meta  */
  YYSYMBOL_close_scope_mh = 444,           /* close_scope_mh  */
  YYSYMBOL_close_scope_monitor = 445,      /* close_scope_monitor  */
  YYSYMBOL_close_scope_nat = 446,          /* close_scope_nat  */
  YYSYMBOL_close_scope_numgen = 447,       /* close_scope_numgen  */
  YYSYMBOL_close_scope_osf = 448,          /* close_scope_osf  */
  YYSYMBOL_close_scope_policy = 449,       /* close_scope_policy  */
  YYSYMBOL_close_scope_quota = 450,        /* close_scope_quota  */
  YYSYMBOL_close_scope_queue = 451,        /* close_scope_queue  */
  YYSYMBOL_close_scope_rate = 452,         /* close_scope_rate  */
  YYSYMBOL_close_scope_reject = 453,       /* close_scope_reject  */
  YYSYMBOL_close_scope_reset = 454,        /* close_scope_reset  */
  YYSYMBOL_close_scope_rt = 455,           /* close_scope_rt  */
  YYSYMBOL_close_scope_sctp = 456,         /* close_scope_sctp  */
  YYSYMBOL_close_scope_sctp_chunk = 457,   /* close_scope_sctp_chunk  */
  YYSYMBOL_close_scope_secmark = 458,      /* close_scope_secmark  */
  YYSYMBOL_close_scope_socket = 459,       /* close_scope_socket  */
  YYSYMBOL_close_scope_tcp = 460,          /* close_scope_tcp  */
  YYSYMBOL_close_scope_tproxy = 461,       /* close_scope_tproxy  */
  YYSYMBOL_close_scope_type = 462,         /* close_scope_type  */
  YYSYMBOL_close_scope_th = 463,           /* close_scope_th  */
  YYSYMBOL_close_scope_udp = 464,          /* close_scope_udp  */
  YYSYMBOL_close_scope_udplite = 465,      /* close_scope_udplite  */
  YYSYMBOL_close_scope_log = 466,          /* close_scope_log  */
  YYSYMBOL_close_scope_synproxy = 467,     /* close_scope_synproxy  */
  YYSYMBOL_close_scope_tunnel = 468,       /* close_scope_tunnel  */
  YYSYMBOL_close_scope_xt = 469,           /* close_scope_xt  */
  YYSYMBOL_common_block = 470,             /* common_block  */
  YYSYMBOL_line = 471,                     /* line  */
  YYSYMBOL_base_cmd = 472,                 /* base_cmd  */
  YYSYMBOL_add_cmd = 473,                  /* add_cmd  */
  YYSYMBOL_replace_cmd = 474,              /* replace_cmd  */
  YYSYMBOL_create_cmd = 475,               /* create_cmd  */
  YYSYMBOL_insert_cmd = 476,               /* insert_cmd  */
  YYSYMBOL_table_or_id_spec = 477,         /* table_or_id_spec  */
  YYSYMBOL_chain_or_id_spec = 478,         /* chain_or_id_spec  */
  YYSYMBOL_set_or_id_spec = 479,           /* set_or_id_spec  */
  YYSYMBOL_obj_or_id_spec = 480,           /* obj_or_id_spec  */
  YYSYMBOL_delete_cmd = 481,               /* delete_cmd  */
  YYSYMBOL_destroy_cmd = 482,              /* destroy_cmd  */
  YYSYMBOL_get_cmd = 483,                  /* get_cmd  */
  YYSYMBOL_list_cmd_spec_table = 484,      /* list_cmd_spec_table  */
  YYSYMBOL_list_cmd_spec_any = 485,        /* list_cmd_spec_any  */
  YYSYMBOL_list_cmd = 486,                 /* list_cmd  */
  YYSYMBOL_basehook_device_name = 487,     /* basehook_device_name  */
  YYSYMBOL_basehook_spec = 488,            /* basehook_spec  */
  YYSYMBOL_reset_cmd = 489,                /* reset_cmd  */
  YYSYMBOL_flush_cmd = 490,                /* flush_cmd  */
  YYSYMBOL_rename_cmd = 491,               /* rename_cmd  */
  YYSYMBOL_import_cmd = 492,               /* import_cmd  */
  YYSYMBOL_export_cmd = 493,               /* export_cmd  */
  YYSYMBOL_monitor_cmd = 494,              /* monitor_cmd  */
  YYSYMBOL_monitor_event = 495,            /* monitor_event  */
  YYSYMBOL_monitor_object = 496,           /* monitor_object  */
  YYSYMBOL_monitor_format = 497,           /* monitor_format  */
  YYSYMBOL_markup_format = 498,            /* markup_format  */
  YYSYMBOL_describe_cmd = 499,             /* describe_cmd  */
  YYSYMBOL_table_block_alloc = 500,        /* table_block_alloc  */
  YYSYMBOL_table_options = 501,            /* table_options  */
  YYSYMBOL_table_flags = 502,              /* table_flags  */
  YYSYMBOL_table_flag = 503,               /* table_flag  */
  YYSYMBOL_table_block = 504,              /* table_block  */
  YYSYMBOL_chain_block_alloc = 505,        /* chain_block_alloc  */
  YYSYMBOL_chain_block = 506,              /* chain_block  */
  YYSYMBOL_subchain_block = 507,           /* subchain_block  */
  YYSYMBOL_typeof_verdict_expr = 508,      /* typeof_verdict_expr  */
  YYSYMBOL_typeof_data_expr = 509,         /* typeof_data_expr  */
  YYSYMBOL_primary_typeof_expr = 510,      /* primary_typeof_expr  */
  YYSYMBOL_typeof_expr = 511,              /* typeof_expr  */
  YYSYMBOL_set_block_alloc = 512,          /* set_block_alloc  */
  YYSYMBOL_typeof_key_expr = 513,          /* typeof_key_expr  */
  YYSYMBOL_set_block = 514,                /* set_block  */
  YYSYMBOL_set_block_expr = 515,           /* set_block_expr  */
  YYSYMBOL_set_flag_list = 516,            /* set_flag_list  */
  YYSYMBOL_set_flag = 517,                 /* set_flag  */
  YYSYMBOL_map_block_alloc = 518,          /* map_block_alloc  */
  YYSYMBOL_ct_obj_type_map = 519,          /* ct_obj_type_map  */
  YYSYMBOL_map_block_obj_type = 520,       /* map_block_obj_type  */
  YYSYMBOL_map_block_obj_typeof = 521,     /* map_block_obj_typeof  */
  YYSYMBOL_map_block_data_interval = 522,  /* map_block_data_interval  */
  YYSYMBOL_map_block = 523,                /* map_block  */
  YYSYMBOL_set_mechanism = 524,            /* set_mechanism  */
  YYSYMBOL_set_policy_spec = 525,          /* set_policy_spec  */
  YYSYMBOL_flowtable_block_alloc = 526,    /* flowtable_block_alloc  */
  YYSYMBOL_flowtable_block = 527,          /* flowtable_block  */
  YYSYMBOL_flowtable_expr = 528,           /* flowtable_expr  */
  YYSYMBOL_flowtable_list_expr = 529,      /* flowtable_list_expr  */
  YYSYMBOL_flowtable_expr_member = 530,    /* flowtable_expr_member  */
  YYSYMBOL_data_type_atom_expr = 531,      /* data_type_atom_expr  */
  YYSYMBOL_data_type_expr = 532,           /* data_type_expr  */
  YYSYMBOL_obj_block_alloc = 533,          /* obj_block_alloc  */
  YYSYMBOL_counter_block = 534,            /* counter_block  */
  YYSYMBOL_quota_block = 535,              /* quota_block  */
  YYSYMBOL_ct_helper_block = 536,          /* ct_helper_block  */
  YYSYMBOL_ct_timeout_block = 537,         /* ct_timeout_block  */
  YYSYMBOL_ct_expect_block = 538,          /* ct_expect_block  */
  YYSYMBOL_connlimit_block = 539,          /* connlimit_block  */
  YYSYMBOL_limit_block = 540,              /* limit_block  */
  YYSYMBOL_secmark_block = 541,            /* secmark_block  */
  YYSYMBOL_synproxy_block = 542,           /* synproxy_block  */
  YYSYMBOL_type_identifier = 543,          /* type_identifier  */
  YYSYMBOL_hook_spec = 544,                /* hook_spec  */
  YYSYMBOL_chain_type = 545,               /* chain_type  */
  YYSYMBOL_prio_spec = 546,                /* prio_spec  */
  YYSYMBOL_extended_prio_name = 547,       /* extended_prio_name  */
  YYSYMBOL_extended_prio_spec = 548,       /* extended_prio_spec  */
  YYSYMBOL_int_num = 549,                  /* int_num  */
  YYSYMBOL_dev_spec = 550,                 /* dev_spec  */
  YYSYMBOL_flags_spec = 551,               /* flags_spec  */
  YYSYMBOL_policy_spec = 552,              /* policy_spec  */
  YYSYMBOL_policy_expr = 553,              /* policy_expr  */
  YYSYMBOL_chain_policy = 554,             /* chain_policy  */
  YYSYMBOL_identifier = 555,               /* identifier  */
  YYSYMBOL_string = 556,                   /* string  */
  YYSYMBOL_time_spec = 557,                /* time_spec  */
  YYSYMBOL_time_spec_or_num_s = 558,       /* time_spec_or_num_s  */
  YYSYMBOL_family_spec = 559,              /* family_spec  */
  YYSYMBOL_family_spec_explicit = 560,     /* family_spec_explicit  */
  YYSYMBOL_table_spec = 561,               /* table_spec  */
  YYSYMBOL_tableid_spec = 562,             /* tableid_spec  */
  YYSYMBOL_chain_spec = 563,               /* chain_spec  */
  YYSYMBOL_chainid_spec = 564,             /* chainid_spec  */
  YYSYMBOL_chain_identifier = 565,         /* chain_identifier  */
  YYSYMBOL_set_spec = 566,                 /* set_spec  */
  YYSYMBOL_setid_spec = 567,               /* setid_spec  */
  YYSYMBOL_set_identifier = 568,           /* set_identifier  */
  YYSYMBOL_flowtable_spec = 569,           /* flowtable_spec  */
  YYSYMBOL_flowtableid_spec = 570,         /* flowtableid_spec  */
  YYSYMBOL_flowtable_identifier = 571,     /* flowtable_identifier  */
  YYSYMBOL_obj_spec = 572,                 /* obj_spec  */
  YYSYMBOL_objid_spec = 573,               /* objid_spec  */
  YYSYMBOL_obj_identifier = 574,           /* obj_identifier  */
  YYSYMBOL_handle_spec = 575,              /* handle_spec  */
  YYSYMBOL_position_spec = 576,            /* position_spec  */
  YYSYMBOL_index_spec = 577,               /* index_spec  */
  YYSYMBOL_rule_position = 578,            /* rule_position  */
  YYSYMBOL_ruleid_spec = 579,              /* ruleid_spec  */
  YYSYMBOL_comment_spec = 580,             /* comment_spec  */
  YYSYMBOL_ruleset_spec = 581,             /* ruleset_spec  */
  YYSYMBOL_rule = 582,                     /* rule  */
  YYSYMBOL_rule_alloc = 583,               /* rule_alloc  */
  YYSYMBOL_stmt_list = 584,                /* stmt_list  */
  YYSYMBOL_stateful_stmt_list = 585,       /* stateful_stmt_list  */
  YYSYMBOL_objref_stmt_counter = 586,      /* objref_stmt_counter  */
  YYSYMBOL_objref_stmt_limit = 587,        /* objref_stmt_limit  */
  YYSYMBOL_objref_stmt_quota = 588,        /* objref_stmt_quota  */
  YYSYMBOL_objref_stmt_synproxy = 589,     /* objref_stmt_synproxy  */
  YYSYMBOL_objref_stmt_tunnel = 590,       /* objref_stmt_tunnel  */
  YYSYMBOL_objref_stmt_ct = 591,           /* objref_stmt_ct  */
  YYSYMBOL_objref_stmt = 592,              /* objref_stmt  */
  YYSYMBOL_stateful_stmt = 593,            /* stateful_stmt  */
  YYSYMBOL_stmt = 594,                     /* stmt  */
  YYSYMBOL_xt_stmt = 595,                  /* xt_stmt  */
  YYSYMBOL_chain_stmt_type = 596,          /* chain_stmt_type  */
  YYSYMBOL_chain_stmt = 597,               /* chain_stmt  */
  YYSYMBOL_verdict_stmt = 598,             /* verdict_stmt  */
  YYSYMBOL_verdict_map_stmt = 599,         /* verdict_map_stmt  */
  YYSYMBOL_verdict_map_expr = 600,         /* verdict_map_expr  */
  YYSYMBOL_verdict_map_list_expr = 601,    /* verdict_map_list_expr  */
  YYSYMBOL_verdict_map_list_member_expr = 602, /* verdict_map_list_member_expr  */
  YYSYMBOL_ct_limit_stmt_alloc = 603,      /* ct_limit_stmt_alloc  */
  YYSYMBOL_connlimit_obj = 604,            /* connlimit_obj  */
  YYSYMBOL_connlimit_config = 605,         /* connlimit_config  */
  YYSYMBOL_connlimit_stmt = 606,           /* connlimit_stmt  */
  YYSYMBOL_ct_limit_args = 607,            /* ct_limit_args  */
  YYSYMBOL_counter_stmt = 608,             /* counter_stmt  */
  YYSYMBOL_counter_stmt_alloc = 609,       /* counter_stmt_alloc  */
  YYSYMBOL_counter_args = 610,             /* counter_args  */
  YYSYMBOL_counter_arg = 611,              /* counter_arg  */
  YYSYMBOL_last_stmt_alloc = 612,          /* last_stmt_alloc  */
  YYSYMBOL_last_stmt = 613,                /* last_stmt  */
  YYSYMBOL_last_args = 614,                /* last_args  */
  YYSYMBOL_log_stmt = 615,                 /* log_stmt  */
  YYSYMBOL_log_stmt_alloc = 616,           /* log_stmt_alloc  */
  YYSYMBOL_log_args = 617,                 /* log_args  */
  YYSYMBOL_log_arg = 618,                  /* log_arg  */
  YYSYMBOL_level_type = 619,               /* level_type  */
  YYSYMBOL_log_flags = 620,                /* log_flags  */
  YYSYMBOL_log_flags_tcp = 621,            /* log_flags_tcp  */
  YYSYMBOL_log_flag_tcp = 622,             /* log_flag_tcp  */
  YYSYMBOL_limit_stmt_alloc = 623,         /* limit_stmt_alloc  */
  YYSYMBOL_limit_stmt = 624,               /* limit_stmt  */
  YYSYMBOL_limit_args = 625,               /* limit_args  */
  YYSYMBOL_quota_mode = 626,               /* quota_mode  */
  YYSYMBOL_bytes_unit = 627,               /* bytes_unit  */
  YYSYMBOL_quota_used = 628,               /* quota_used  */
  YYSYMBOL_quota_stmt_alloc = 629,         /* quota_stmt_alloc  */
  YYSYMBOL_quota_stmt = 630,               /* quota_stmt  */
  YYSYMBOL_quota_args = 631,               /* quota_args  */
  YYSYMBOL_limit_mode = 632,               /* limit_mode  */
  YYSYMBOL_limit_burst_pkts = 633,         /* limit_burst_pkts  */
  YYSYMBOL_limit_rate_pkts = 634,          /* limit_rate_pkts  */
  YYSYMBOL_limit_burst_bytes = 635,        /* limit_burst_bytes  */
  YYSYMBOL_limit_rate_bytes = 636,         /* limit_rate_bytes  */
  YYSYMBOL_time_unit = 637,                /* time_unit  */
  YYSYMBOL_reject_stmt = 638,              /* reject_stmt  */
  YYSYMBOL_reject_stmt_alloc = 639,        /* reject_stmt_alloc  */
  YYSYMBOL_reject_with_expr = 640,         /* reject_with_expr  */
  YYSYMBOL_reject_opts = 641,              /* reject_opts  */
  YYSYMBOL_nat_stmt = 642,                 /* nat_stmt  */
  YYSYMBOL_nat_stmt_alloc = 643,           /* nat_stmt_alloc  */
  YYSYMBOL_tproxy_stmt = 644,              /* tproxy_stmt  */
  YYSYMBOL_synproxy_stmt = 645,            /* synproxy_stmt  */
  YYSYMBOL_synproxy_stmt_alloc = 646,      /* synproxy_stmt_alloc  */
  YYSYMBOL_synproxy_args = 647,            /* synproxy_args  */
  YYSYMBOL_synproxy_wscale = 648,          /* synproxy_wscale  */
  YYSYMBOL_synproxy_arg = 649,             /* synproxy_arg  */
  YYSYMBOL_synproxy_config = 650,          /* synproxy_config  */
  YYSYMBOL_synproxy_obj = 651,             /* synproxy_obj  */
  YYSYMBOL_synproxy_ts = 652,              /* synproxy_ts  */
  YYSYMBOL_synproxy_sack = 653,            /* synproxy_sack  */
  YYSYMBOL_primary_stmt_expr = 654,        /* primary_stmt_expr  */
  YYSYMBOL_shift_stmt_expr = 655,          /* shift_stmt_expr  */
  YYSYMBOL_and_stmt_expr = 656,            /* and_stmt_expr  */
  YYSYMBOL_exclusive_or_stmt_expr = 657,   /* exclusive_or_stmt_expr  */
  YYSYMBOL_inclusive_or_stmt_expr = 658,   /* inclusive_or_stmt_expr  */
  YYSYMBOL_basic_stmt_expr = 659,          /* basic_stmt_expr  */
  YYSYMBOL_concat_stmt_expr = 660,         /* concat_stmt_expr  */
  YYSYMBOL_map_stmt_expr_set = 661,        /* map_stmt_expr_set  */
  YYSYMBOL_map_stmt_expr = 662,            /* map_stmt_expr  */
  YYSYMBOL_prefix_stmt_expr = 663,         /* prefix_stmt_expr  */
  YYSYMBOL_range_stmt_expr = 664,          /* range_stmt_expr  */
  YYSYMBOL_multiton_stmt_expr = 665,       /* multiton_stmt_expr  */
  YYSYMBOL_stmt_expr = 666,                /* stmt_expr  */
  YYSYMBOL_nat_stmt_args = 667,            /* nat_stmt_args  */
  YYSYMBOL_masq_stmt = 668,                /* masq_stmt  */
  YYSYMBOL_masq_stmt_alloc = 669,          /* masq_stmt_alloc  */
  YYSYMBOL_masq_stmt_args = 670,           /* masq_stmt_args  */
  YYSYMBOL_redir_stmt = 671,               /* redir_stmt  */
  YYSYMBOL_redir_stmt_alloc = 672,         /* redir_stmt_alloc  */
  YYSYMBOL_redir_stmt_arg = 673,           /* redir_stmt_arg  */
  YYSYMBOL_dup_stmt = 674,                 /* dup_stmt  */
  YYSYMBOL_fwd_stmt = 675,                 /* fwd_stmt  */
  YYSYMBOL_nf_nat_flags = 676,             /* nf_nat_flags  */
  YYSYMBOL_nf_nat_flag = 677,              /* nf_nat_flag  */
  YYSYMBOL_queue_stmt = 678,               /* queue_stmt  */
  YYSYMBOL_queue_stmt_compat = 679,        /* queue_stmt_compat  */
  YYSYMBOL_queue_stmt_alloc = 680,         /* queue_stmt_alloc  */
  YYSYMBOL_queue_stmt_args = 681,          /* queue_stmt_args  */
  YYSYMBOL_queue_stmt_arg = 682,           /* queue_stmt_arg  */
  YYSYMBOL_queue_expr = 683,               /* queue_expr  */
  YYSYMBOL_queue_stmt_expr_simple = 684,   /* queue_stmt_expr_simple  */
  YYSYMBOL_queue_stmt_expr = 685,          /* queue_stmt_expr  */
  YYSYMBOL_queue_stmt_flags = 686,         /* queue_stmt_flags  */
  YYSYMBOL_queue_stmt_flag = 687,          /* queue_stmt_flag  */
  YYSYMBOL_set_elem_expr_stmt = 688,       /* set_elem_expr_stmt  */
  YYSYMBOL_set_elem_expr_stmt_alloc = 689, /* set_elem_expr_stmt_alloc  */
  YYSYMBOL_set_stmt = 690,                 /* set_stmt  */
  YYSYMBOL_set_stmt_op = 691,              /* set_stmt_op  */
  YYSYMBOL_map_stmt = 692,                 /* map_stmt  */
  YYSYMBOL_meter_stmt = 693,               /* meter_stmt  */
  YYSYMBOL_match_stmt = 694,               /* match_stmt  */
  YYSYMBOL_variable_expr = 695,            /* variable_expr  */
  YYSYMBOL_symbol_expr = 696,              /* symbol_expr  */
  YYSYMBOL_set_ref_expr = 697,             /* set_ref_expr  */
  YYSYMBOL_set_ref_symbol_expr = 698,      /* set_ref_symbol_expr  */
  YYSYMBOL_integer_expr = 699,             /* integer_expr  */
  YYSYMBOL_selector_expr = 700,            /* selector_expr  */
  YYSYMBOL_primary_expr = 701,             /* primary_expr  */
  YYSYMBOL_fib_expr = 702,                 /* fib_expr  */
  YYSYMBOL_fib_result = 703,               /* fib_result  */
  YYSYMBOL_fib_flag = 704,                 /* fib_flag  */
  YYSYMBOL_fib_tuple = 705,                /* fib_tuple  */
  YYSYMBOL_osf_expr = 706,                 /* osf_expr  */
  YYSYMBOL_osf_ttl = 707,                  /* osf_ttl  */
  YYSYMBOL_shift_expr = 708,               /* shift_expr  */
  YYSYMBOL_and_expr = 709,                 /* and_expr  */
  YYSYMBOL_exclusive_or_expr = 710,        /* exclusive_or_expr  */
  YYSYMBOL_inclusive_or_expr = 711,        /* inclusive_or_expr  */
  YYSYMBOL_basic_expr = 712,               /* basic_expr  */
  YYSYMBOL_concat_expr = 713,              /* concat_expr  */
  YYSYMBOL_prefix_rhs_expr = 714,          /* prefix_rhs_expr  */
  YYSYMBOL_range_rhs_expr = 715,           /* range_rhs_expr  */
  YYSYMBOL_multiton_rhs_expr = 716,        /* multiton_rhs_expr  */
  YYSYMBOL_map_expr = 717,                 /* map_expr  */
  YYSYMBOL_expr = 718,                     /* expr  */
  YYSYMBOL_set_expr = 719,                 /* set_expr  */
  YYSYMBOL_set_list_expr = 720,            /* set_list_expr  */
  YYSYMBOL_set_list_member_expr = 721,     /* set_list_member_expr  */
  YYSYMBOL_meter_key_expr = 722,           /* meter_key_expr  */
  YYSYMBOL_meter_key_expr_alloc = 723,     /* meter_key_expr_alloc  */
  YYSYMBOL_set_elem_expr = 724,            /* set_elem_expr  */
  YYSYMBOL_set_elem_key_expr = 725,        /* set_elem_key_expr  */
  YYSYMBOL_set_elem_expr_alloc = 726,      /* set_elem_expr_alloc  */
  YYSYMBOL_set_elem_options = 727,         /* set_elem_options  */
  YYSYMBOL_set_elem_time_spec = 728,       /* set_elem_time_spec  */
  YYSYMBOL_set_elem_option = 729,          /* set_elem_option  */
  YYSYMBOL_set_elem_expr_options = 730,    /* set_elem_expr_options  */
  YYSYMBOL_set_elem_stmt_list = 731,       /* set_elem_stmt_list  */
  YYSYMBOL_set_elem_stmt = 732,            /* set_elem_stmt  */
  YYSYMBOL_set_elem_expr_option = 733,     /* set_elem_expr_option  */
  YYSYMBOL_set_lhs_expr = 734,             /* set_lhs_expr  */
  YYSYMBOL_set_rhs_expr = 735,             /* set_rhs_expr  */
  YYSYMBOL_initializer_expr = 736,         /* initializer_expr  */
  YYSYMBOL_counter_config = 737,           /* counter_config  */
  YYSYMBOL_counter_obj = 738,              /* counter_obj  */
  YYSYMBOL_quota_config = 739,             /* quota_config  */
  YYSYMBOL_quota_obj = 740,                /* quota_obj  */
  YYSYMBOL_secmark_config = 741,           /* secmark_config  */
  YYSYMBOL_secmark_obj = 742,              /* secmark_obj  */
  YYSYMBOL_ct_obj_type = 743,              /* ct_obj_type  */
  YYSYMBOL_ct_cmd_type = 744,              /* ct_cmd_type  */
  YYSYMBOL_ct_l4protoname = 745,           /* ct_l4protoname  */
  YYSYMBOL_ct_helper_config = 746,         /* ct_helper_config  */
  YYSYMBOL_timeout_states = 747,           /* timeout_states  */
  YYSYMBOL_timeout_state = 748,            /* timeout_state  */
  YYSYMBOL_ct_timeout_config = 749,        /* ct_timeout_config  */
  YYSYMBOL_ct_expect_config = 750,         /* ct_expect_config  */
  YYSYMBOL_ct_obj_alloc = 751,             /* ct_obj_alloc  */
  YYSYMBOL_limit_config = 752,             /* limit_config  */
  YYSYMBOL_limit_obj = 753,                /* limit_obj  */
  YYSYMBOL_erspan_block = 754,             /* erspan_block  */
  YYSYMBOL_erspan_block_alloc = 755,       /* erspan_block_alloc  */
  YYSYMBOL_erspan_config = 756,            /* erspan_config  */
  YYSYMBOL_geneve_block = 757,             /* geneve_block  */
  YYSYMBOL_geneve_block_alloc = 758,       /* geneve_block_alloc  */
  YYSYMBOL_geneve_config = 759,            /* geneve_config  */
  YYSYMBOL_vxlan_block = 760,              /* vxlan_block  */
  YYSYMBOL_vxlan_block_alloc = 761,        /* vxlan_block_alloc  */
  YYSYMBOL_vxlan_config = 762,             /* vxlan_config  */
  YYSYMBOL_tunnel_config = 763,            /* tunnel_config  */
  YYSYMBOL_tunnel_block = 764,             /* tunnel_block  */
  YYSYMBOL_tunnel_obj = 765,               /* tunnel_obj  */
  YYSYMBOL_relational_expr = 766,          /* relational_expr  */
  YYSYMBOL_list_rhs_expr = 767,            /* list_rhs_expr  */
  YYSYMBOL_rhs_expr = 768,                 /* rhs_expr  */
  YYSYMBOL_shift_rhs_expr = 769,           /* shift_rhs_expr  */
  YYSYMBOL_and_rhs_expr = 770,             /* and_rhs_expr  */
  YYSYMBOL_exclusive_or_rhs_expr = 771,    /* exclusive_or_rhs_expr  */
  YYSYMBOL_inclusive_or_rhs_expr = 772,    /* inclusive_or_rhs_expr  */
  YYSYMBOL_basic_rhs_expr = 773,           /* basic_rhs_expr  */
  YYSYMBOL_concat_rhs_expr = 774,          /* concat_rhs_expr  */
  YYSYMBOL_boolean_keys = 775,             /* boolean_keys  */
  YYSYMBOL_boolean_expr = 776,             /* boolean_expr  */
  YYSYMBOL_keyword_expr = 777,             /* keyword_expr  */
  YYSYMBOL_primary_rhs_expr = 778,         /* primary_rhs_expr  */
  YYSYMBOL_relational_op = 779,            /* relational_op  */
  YYSYMBOL_verdict_expr = 780,             /* verdict_expr  */
  YYSYMBOL_chain_expr = 781,               /* chain_expr  */
  YYSYMBOL_meta_expr = 782,                /* meta_expr  */
  YYSYMBOL_meta_key = 783,                 /* meta_key  */
  YYSYMBOL_meta_key_qualified = 784,       /* meta_key_qualified  */
  YYSYMBOL_meta_key_unqualified = 785,     /* meta_key_unqualified  */
  YYSYMBOL_meta_stmt = 786,                /* meta_stmt  */
  YYSYMBOL_socket_expr = 787,              /* socket_expr  */
  YYSYMBOL_socket_key = 788,               /* socket_key  */
  YYSYMBOL_tunnel_key = 789,               /* tunnel_key  */
  YYSYMBOL_tunnel_expr = 790,              /* tunnel_expr  */
  YYSYMBOL_offset_opt = 791,               /* offset_opt  */
  YYSYMBOL_numgen_type = 792,              /* numgen_type  */
  YYSYMBOL_numgen_expr = 793,              /* numgen_expr  */
  YYSYMBOL_xfrm_spnum = 794,               /* xfrm_spnum  */
  YYSYMBOL_xfrm_dir = 795,                 /* xfrm_dir  */
  YYSYMBOL_xfrm_state_key = 796,           /* xfrm_state_key  */
  YYSYMBOL_xfrm_state_proto_key = 797,     /* xfrm_state_proto_key  */
  YYSYMBOL_xfrm_expr = 798,                /* xfrm_expr  */
  YYSYMBOL_hash_expr = 799,                /* hash_expr  */
  YYSYMBOL_nf_key_proto = 800,             /* nf_key_proto  */
  YYSYMBOL_rt_expr = 801,                  /* rt_expr  */
  YYSYMBOL_rt_key = 802,                   /* rt_key  */
  YYSYMBOL_ct_expr = 803,                  /* ct_expr  */
  YYSYMBOL_ct_dir = 804,                   /* ct_dir  */
  YYSYMBOL_ct_key = 805,                   /* ct_key  */
  YYSYMBOL_ct_key_dir = 806,               /* ct_key_dir  */
  YYSYMBOL_ct_key_proto_field = 807,       /* ct_key_proto_field  */
  YYSYMBOL_ct_key_dir_optional = 808,      /* ct_key_dir_optional  */
  YYSYMBOL_symbol_stmt_expr = 809,         /* symbol_stmt_expr  */
  YYSYMBOL_list_stmt_expr = 810,           /* list_stmt_expr  */
  YYSYMBOL_ct_stmt = 811,                  /* ct_stmt  */
  YYSYMBOL_payload_stmt = 812,             /* payload_stmt  */
  YYSYMBOL_payload_expr = 813,             /* payload_expr  */
  YYSYMBOL_payload_raw_len = 814,          /* payload_raw_len  */
  YYSYMBOL_payload_raw_expr = 815,         /* payload_raw_expr  */
  YYSYMBOL_payload_base_spec = 816,        /* payload_base_spec  */
  YYSYMBOL_eth_hdr_expr = 817,             /* eth_hdr_expr  */
  YYSYMBOL_eth_hdr_field = 818,            /* eth_hdr_field  */
  YYSYMBOL_vlan_hdr_expr = 819,            /* vlan_hdr_expr  */
  YYSYMBOL_vlan_hdr_field = 820,           /* vlan_hdr_field  */
  YYSYMBOL_arp_hdr_expr = 821,             /* arp_hdr_expr  */
  YYSYMBOL_arp_hdr_field = 822,            /* arp_hdr_field  */
  YYSYMBOL_ip_hdr_expr = 823,              /* ip_hdr_expr  */
  YYSYMBOL_ip_hdr_field = 824,             /* ip_hdr_field  */
  YYSYMBOL_ip_option_type = 825,           /* ip_option_type  */
  YYSYMBOL_ip_option_field = 826,          /* ip_option_field  */
  YYSYMBOL_icmp_hdr_expr = 827,            /* icmp_hdr_expr  */
  YYSYMBOL_icmp_hdr_field = 828,           /* icmp_hdr_field  */
  YYSYMBOL_igmp_hdr_expr = 829,            /* igmp_hdr_expr  */
  YYSYMBOL_igmp_hdr_field = 830,           /* igmp_hdr_field  */
  YYSYMBOL_ip6_hdr_expr = 831,             /* ip6_hdr_expr  */
  YYSYMBOL_ip6_hdr_field = 832,            /* ip6_hdr_field  */
  YYSYMBOL_icmp6_hdr_expr = 833,           /* icmp6_hdr_expr  */
  YYSYMBOL_icmp6_hdr_field = 834,          /* icmp6_hdr_field  */
  YYSYMBOL_auth_hdr_expr = 835,            /* auth_hdr_expr  */
  YYSYMBOL_auth_hdr_field = 836,           /* auth_hdr_field  */
  YYSYMBOL_esp_hdr_expr = 837,             /* esp_hdr_expr  */
  YYSYMBOL_esp_hdr_field = 838,            /* esp_hdr_field  */
  YYSYMBOL_comp_hdr_expr = 839,            /* comp_hdr_expr  */
  YYSYMBOL_comp_hdr_field = 840,           /* comp_hdr_field  */
  YYSYMBOL_udp_hdr_expr = 841,             /* udp_hdr_expr  */
  YYSYMBOL_udp_hdr_field = 842,            /* udp_hdr_field  */
  YYSYMBOL_udplite_hdr_expr = 843,         /* udplite_hdr_expr  */
  YYSYMBOL_udplite_hdr_field = 844,        /* udplite_hdr_field  */
  YYSYMBOL_tcp_hdr_expr = 845,             /* tcp_hdr_expr  */
  YYSYMBOL_inner_inet_expr = 846,          /* inner_inet_expr  */
  YYSYMBOL_inner_eth_expr = 847,           /* inner_eth_expr  */
  YYSYMBOL_inner_expr = 848,               /* inner_expr  */
  YYSYMBOL_vxlan_hdr_expr = 849,           /* vxlan_hdr_expr  */
  YYSYMBOL_vxlan_hdr_field = 850,          /* vxlan_hdr_field  */
  YYSYMBOL_geneve_hdr_expr = 851,          /* geneve_hdr_expr  */
  YYSYMBOL_geneve_hdr_field = 852,         /* geneve_hdr_field  */
  YYSYMBOL_gre_hdr_expr = 853,             /* gre_hdr_expr  */
  YYSYMBOL_gre_hdr_field = 854,            /* gre_hdr_field  */
  YYSYMBOL_gretap_hdr_expr = 855,          /* gretap_hdr_expr  */
  YYSYMBOL_optstrip_stmt = 856,            /* optstrip_stmt  */
  YYSYMBOL_tcp_hdr_field = 857,            /* tcp_hdr_field  */
  YYSYMBOL_tcp_hdr_option_kind_and_field = 858, /* tcp_hdr_option_kind_and_field  */
  YYSYMBOL_tcp_hdr_option_sack = 859,      /* tcp_hdr_option_sack  */
  YYSYMBOL_tcp_hdr_option_type = 860,      /* tcp_hdr_option_type  */
  YYSYMBOL_tcpopt_field_sack = 861,        /* tcpopt_field_sack  */
  YYSYMBOL_tcpopt_field_window = 862,      /* tcpopt_field_window  */
  YYSYMBOL_tcpopt_field_tsopt = 863,       /* tcpopt_field_tsopt  */
  YYSYMBOL_tcpopt_field_maxseg = 864,      /* tcpopt_field_maxseg  */
  YYSYMBOL_tcpopt_field_mptcp = 865,       /* tcpopt_field_mptcp  */
  YYSYMBOL_dccp_hdr_expr = 866,            /* dccp_hdr_expr  */
  YYSYMBOL_dccp_hdr_field = 867,           /* dccp_hdr_field  */
  YYSYMBOL_sctp_chunk_type = 868,          /* sctp_chunk_type  */
  YYSYMBOL_sctp_chunk_common_field = 869,  /* sctp_chunk_common_field  */
  YYSYMBOL_sctp_chunk_data_field = 870,    /* sctp_chunk_data_field  */
  YYSYMBOL_sctp_chunk_init_field = 871,    /* sctp_chunk_init_field  */
  YYSYMBOL_sctp_chunk_sack_field = 872,    /* sctp_chunk_sack_field  */
  YYSYMBOL_sctp_chunk_alloc = 873,         /* sctp_chunk_alloc  */
  YYSYMBOL_sctp_hdr_expr = 874,            /* sctp_hdr_expr  */
  YYSYMBOL_sctp_hdr_field = 875,           /* sctp_hdr_field  */
  YYSYMBOL_th_hdr_expr = 876,              /* th_hdr_expr  */
  YYSYMBOL_th_hdr_field = 877,             /* th_hdr_field  */
  YYSYMBOL_exthdr_expr = 878,              /* exthdr_expr  */
  YYSYMBOL_hbh_hdr_expr = 879,             /* hbh_hdr_expr  */
  YYSYMBOL_hbh_hdr_field = 880,            /* hbh_hdr_field  */
  YYSYMBOL_rt_hdr_expr = 881,              /* rt_hdr_expr  */
  YYSYMBOL_rt_hdr_field = 882,             /* rt_hdr_field  */
  YYSYMBOL_rt0_hdr_expr = 883,             /* rt0_hdr_expr  */
  YYSYMBOL_rt0_hdr_field = 884,            /* rt0_hdr_field  */
  YYSYMBOL_rt2_hdr_expr = 885,             /* rt2_hdr_expr  */
  YYSYMBOL_rt2_hdr_field = 886,            /* rt2_hdr_field  */
  YYSYMBOL_rt4_hdr_expr = 887,             /* rt4_hdr_expr  */
  YYSYMBOL_rt4_hdr_field = 888,            /* rt4_hdr_field  */
  YYSYMBOL_frag_hdr_expr = 889,            /* frag_hdr_expr  */
  YYSYMBOL_frag_hdr_field = 890,           /* frag_hdr_field  */
  YYSYMBOL_dst_hdr_expr = 891,             /* dst_hdr_expr  */
  YYSYMBOL_dst_hdr_field = 892,            /* dst_hdr_field  */
  YYSYMBOL_mh_hdr_expr = 893,              /* mh_hdr_expr  */
  YYSYMBOL_mh_hdr_field = 894,             /* mh_hdr_field  */
  YYSYMBOL_exthdr_exists_expr = 895,       /* exthdr_exists_expr  */
  YYSYMBOL_exthdr_key = 896                /* exthdr_key  */
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
# define YYCOPY_NEEDED 1
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
#define YYLAST   10052

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  410
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  487
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1474
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2518

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   655


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
       2,     2,     2,     2,     2,     2,   407,     2,     2,     2,
     404,   405,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   401,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   408,     2,   409,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   402,   406,   403,     2,     2,     2,     2,
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,  1070,  1070,  1071,  1080,  1081,  1084,  1085,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,
    1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,
    1141,  1142,  1143,  1144,  1146,  1154,  1169,  1176,  1188,  1196,
    1197,  1198,  1199,  1219,  1220,  1221,  1222,  1223,  1224,  1225,
    1226,  1227,  1228,  1229,  1230,  1231,  1232,  1233,  1234,  1237,
    1241,  1248,  1252,  1260,  1264,  1268,  1275,  1282,  1292,  1299,
    1308,  1312,  1316,  1320,  1324,  1328,  1332,  1336,  1340,  1344,
    1348,  1352,  1356,  1360,  1364,  1370,  1376,  1380,  1387,  1391,
    1399,  1406,  1413,  1423,  1430,  1439,  1443,  1447,  1451,  1455,
    1459,  1463,  1467,  1471,  1475,  1481,  1487,  1488,  1491,  1492,
    1495,  1496,  1499,  1500,  1503,  1507,  1511,  1519,  1523,  1527,
    1531,  1535,  1539,  1543,  1550,  1554,  1558,  1564,  1568,  1572,
    1576,  1580,  1586,  1590,  1594,  1598,  1602,  1606,  1610,  1614,
    1618,  1625,  1629,  1633,  1639,  1643,  1647,  1651,  1655,  1662,
    1668,  1669,  1671,  1672,  1675,  1679,  1683,  1687,  1691,  1695,
    1699,  1703,  1707,  1711,  1715,  1719,  1723,  1727,  1731,  1735,
    1739,  1743,  1747,  1751,  1755,  1759,  1763,  1767,  1771,  1775,
    1779,  1783,  1787,  1791,  1795,  1799,  1806,  1812,  1816,  1826,
    1830,  1834,  1838,  1842,  1846,  1850,  1854,  1859,  1863,  1867,
    1871,  1877,  1881,  1885,  1889,  1893,  1897,  1901,  1907,  1914,
    1920,  1928,  1934,  1942,  1951,  1952,  1953,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1966,  1967,  1970,  1971,  1972,
    1975,  1984,  1994,  1998,  2008,  2009,  2014,  2028,  2029,  2030,
    2031,  2032,  2043,  2053,  2064,  2074,  2085,  2096,  2105,  2114,
    2123,  2134,  2145,  2156,  2167,  2181,  2191,  2192,  2193,  2194,
    2195,  2196,  2197,  2202,  2211,  2221,  2222,  2223,  2230,  2242,
    2253,  2258,  2262,  2266,  2283,  2296,  2300,  2313,  2318,  2319,
    2322,  2323,  2324,  2325,  2335,  2340,  2345,  2350,  2356,  2365,
    2370,  2371,  2382,  2383,  2386,  2390,  2393,  2394,  2395,  2396,
    2400,  2405,  2406,  2407,  2410,  2411,  2412,  2413,  2414,  2417,
    2418,  2421,  2422,  2425,  2426,  2427,  2428,  2433,  2438,  2455,
    2478,  2492,  2501,  2506,  2512,  2517,  2526,  2529,  2533,  2539,
    2540,  2544,  2549,  2550,  2551,  2552,  2567,  2571,  2575,  2581,
    2586,  2593,  2598,  2603,  2606,  2615,  2622,  2635,  2642,  2643,
    2655,  2660,  2661,  2662,  2663,  2667,  2677,  2678,  2679,  2680,
    2684,  2694,  2695,  2696,  2697,  2701,  2712,  2717,  2718,  2719,
    2723,  2733,  2734,  2735,  2736,  2740,  2750,  2751,  2752,  2753,
    2757,  2767,  2768,  2769,  2770,  2774,  2784,  2785,  2786,  2787,
    2791,  2801,  2802,  2803,  2804,  2808,  2818,  2819,  2820,  2821,
    2822,  2825,  2849,  2850,  2851,  2854,  2861,  2865,  2868,  2878,
    2885,  2896,  2909,  2924,  2925,  2928,  2939,  2945,  2949,  2952,
    2958,  2972,  2977,  2986,  2987,  2990,  2991,  2994,  2995,  2996,
    2999,  3015,  3016,  3019,  3020,  3023,  3024,  3025,  3026,  3027,
    3028,  3031,  3040,  3049,  3057,  3065,  3073,  3081,  3089,  3097,
    3105,  3113,  3121,  3129,  3137,  3145,  3153,  3161,  3169,  3173,
    3178,  3186,  3193,  3200,  3214,  3218,  3225,  3229,  3235,  3247,
    3253,  3260,  3266,  3273,  3281,  3289,  3297,  3305,  3313,  3320,
    3326,  3334,  3335,  3336,  3337,  3338,  3339,  3342,  3343,  3344,
    3345,  3346,  3349,  3350,  3351,  3352,  3353,  3354,  3355,  3356,
    3357,  3358,  3359,  3360,  3361,  3362,  3363,  3364,  3365,  3366,
    3367,  3368,  3369,  3370,  3371,  3374,  3385,  3386,  3389,  3398,
    3402,  3408,  3414,  3419,  3422,  3427,  3432,  3435,  3444,  3451,
    3457,  3466,  3476,  3479,  3485,  3494,  3495,  3497,  3503,  3507,
    3510,  3515,  3522,  3528,  3529,  3532,  3533,  3544,  3545,  3548,
    3554,  3558,  3561,  3578,  3583,  3588,  3593,  3598,  3604,  3605,
    3606,  3607,  3608,  3609,  3610,  3611,  3612,  3615,  3619,  3623,
    3627,  3631,  3637,  3641,  3644,  3648,  3654,  3660,  3663,  3681,
    3696,  3697,  3698,  3701,  3702,  3703,  3706,  3707,  3713,  3719,
    3722,  3735,  3736,  3737,  3740,  3741,  3744,  3751,  3752,  3755,
    3762,  3763,  3764,  3765,  3766,  3769,  3772,  3778,  3784,  3788,
    3792,  3799,  3806,  3813,  3820,  3826,  3832,  3838,  3841,  3842,
    3845,  3851,  3857,  3863,  3870,  3877,  3885,  3886,  3889,  3895,
    3899,  3902,  3913,  3918,  3923,  3927,  3933,  3949,  3968,  3974,
    3975,  3981,  3982,  3988,  3989,  3990,  3991,  3992,  3993,  3994,
    3995,  3996,  3997,  3998,  3999,  4000,  4001,  4002,  4005,  4006,
    4010,  4016,  4017,  4023,  4024,  4030,  4031,  4037,  4040,  4041,
    4052,  4053,  4056,  4060,  4063,  4069,  4075,  4076,  4079,  4080,
    4081,  4084,  4088,  4092,  4097,  4102,  4107,  4113,  4117,  4121,
    4125,  4131,  4136,  4140,  4148,  4157,  4158,  4161,  4164,  4168,
    4173,  4179,  4180,  4183,  4186,  4190,  4194,  4198,  4203,  4210,
    4215,  4223,  4228,  4237,  4238,  4244,  4245,  4246,  4249,  4250,
    4254,  4258,  4264,  4265,  4268,  4274,  4278,  4281,  4286,  4292,
    4293,  4296,  4297,  4298,  4304,  4305,  4306,  4307,  4310,  4311,
    4317,  4318,  4321,  4322,  4325,  4331,  4338,  4345,  4356,  4357,
    4358,  4361,  4369,  4381,  4390,  4401,  4407,  4433,  4434,  4443,
    4444,  4447,  4456,  4467,  4468,  4469,  4470,  4471,  4472,  4473,
    4474,  4475,  4476,  4477,  4478,  4479,  4482,  4483,  4484,  4485,
    4488,  4518,  4519,  4520,  4521,  4524,  4525,  4526,  4527,  4528,
    4531,  4535,  4538,  4542,  4548,  4549,  4550,  4553,  4554,  4558,
    4564,  4565,  4571,  4572,  4578,  4579,  4585,  4588,  4589,  4600,
    4606,  4619,  4620,  4623,  4629,  4630,  4631,  4634,  4641,  4646,
    4651,  4654,  4658,  4662,  4671,  4672,  4679,  4685,  4686,  4687,
    4695,  4696,  4699,  4705,  4711,  4715,  4718,  4739,  4743,  4747,
    4757,  4761,  4764,  4770,  4777,  4778,  4779,  4780,  4781,  4784,
    4788,  4792,  4802,  4805,  4806,  4809,  4810,  4811,  4812,  4823,
    4834,  4840,  4852,  4858,  4875,  4881,  4882,  4883,  4886,  4887,
    4888,  4891,  4892,  4895,  4918,  4924,  4930,  4937,  4950,  4958,
    4966,  4972,  4976,  4980,  4984,  4988,  4995,  5000,  5011,  5025,
    5031,  5032,  5033,  5034,  5041,  5049,  5054,  5058,  5062,  5066,
    5072,  5073,  5074,  5075,  5082,  5091,  5112,  5113,  5114,  5115,
    5122,  5130,  5137,  5141,  5151,  5160,  5169,  5178,  5182,  5186,
    5190,  5194,  5198,  5202,  5208,  5209,  5210,  5211,  5215,  5226,
    5232,  5236,  5240,  5247,  5255,  5262,  5270,  5274,  5280,  5286,
    5294,  5295,  5296,  5299,  5300,  5304,  5310,  5311,  5317,  5318,
    5324,  5325,  5331,  5334,  5335,  5336,  5345,  5356,  5357,  5360,
    5368,  5369,  5370,  5371,  5372,  5373,  5374,  5375,  5376,  5377,
    5378,  5379,  5380,  5381,  5384,  5385,  5386,  5387,  5388,  5395,
    5402,  5409,  5416,  5423,  5430,  5437,  5444,  5451,  5458,  5465,
    5472,  5479,  5482,  5483,  5484,  5485,  5486,  5487,  5488,  5491,
    5495,  5499,  5503,  5507,  5511,  5517,  5518,  5528,  5532,  5538,
    5539,  5542,  5543,  5544,  5545,  5546,  5547,  5548,  5549,  5550,
    5551,  5552,  5553,  5554,  5555,  5556,  5559,  5560,  5561,  5562,
    5563,  5564,  5565,  5566,  5567,  5568,  5569,  5570,  5571,  5572,
    5573,  5574,  5575,  5576,  5577,  5578,  5579,  5580,  5583,  5603,
    5607,  5611,  5615,  5621,  5625,  5631,  5632,  5633,  5636,  5637,
    5640,  5646,  5647,  5650,  5651,  5654,  5660,  5661,  5664,  5665,
    5668,  5669,  5672,  5673,  5676,  5684,  5711,  5716,  5721,  5727,
    5728,  5731,  5735,  5755,  5756,  5757,  5758,  5761,  5765,  5769,
    5775,  5776,  5779,  5780,  5781,  5782,  5783,  5784,  5785,  5786,
    5787,  5788,  5789,  5790,  5791,  5792,  5793,  5794,  5795,  5798,
    5799,  5800,  5801,  5802,  5803,  5804,  5807,  5808,  5809,  5810,
    5813,  5814,  5815,  5816,  5819,  5820,  5823,  5829,  5837,  5850,
    5856,  5865,  5866,  5867,  5868,  5869,  5870,  5871,  5872,  5873,
    5874,  5875,  5876,  5877,  5878,  5879,  5880,  5881,  5882,  5883,
    5884,  5885,  5886,  5889,  5907,  5916,  5917,  5918,  5919,  5932,
    5938,  5939,  5940,  5943,  5949,  5950,  5951,  5952,  5953,  5956,
    5962,  5963,  5964,  5965,  5966,  5967,  5968,  5969,  5970,  5973,
    5977,  5988,  5995,  5996,  5997,  5998,  5999,  6000,  6001,  6002,
    6003,  6004,  6005,  6006,  6009,  6010,  6011,  6012,  6015,  6016,
    6017,  6018,  6019,  6022,  6028,  6029,  6030,  6031,  6032,  6033,
    6034,  6037,  6043,  6044,  6045,  6046,  6049,  6055,  6056,  6057,
    6058,  6059,  6060,  6061,  6062,  6063,  6065,  6071,  6072,  6073,
    6074,  6075,  6076,  6077,  6078,  6079,  6080,  6083,  6089,  6090,
    6091,  6092,  6093,  6096,  6102,  6103,  6106,  6112,  6113,  6114,
    6117,  6123,  6124,  6125,  6126,  6129,  6135,  6136,  6137,  6138,
    6141,  6145,  6150,  6158,  6165,  6166,  6167,  6168,  6169,  6170,
    6171,  6172,  6173,  6174,  6175,  6176,  6177,  6178,  6181,  6182,
    6183,  6186,  6187,  6190,  6198,  6206,  6207,  6210,  6218,  6226,
    6227,  6230,  6234,  6241,  6242,  6243,  6246,  6253,  6260,  6261,
    6262,  6263,  6264,  6265,  6266,  6267,  6268,  6269,  6272,  6277,
    6282,  6287,  6292,  6297,  6304,  6305,  6306,  6307,  6308,  6311,
    6312,  6313,  6314,  6315,  6316,  6317,  6318,  6319,  6320,  6321,
    6322,  6331,  6332,  6335,  6338,  6339,  6342,  6345,  6348,  6352,
    6363,  6364,  6365,  6368,  6369,  6370,  6371,  6372,  6373,  6374,
    6375,  6376,  6377,  6378,  6379,  6380,  6381,  6382,  6383,  6384,
    6385,  6388,  6389,  6390,  6393,  6394,  6395,  6396,  6399,  6400,
    6401,  6402,  6403,  6406,  6407,  6408,  6409,  6412,  6417,  6421,
    6425,  6429,  6433,  6437,  6442,  6447,  6452,  6457,  6462,  6469,
    6473,  6479,  6480,  6481,  6482,  6485,  6493,  6494,  6497,  6498,
    6499,  6500,  6501,  6502,  6503,  6504,  6507,  6513,  6514,  6517,
    6523,  6524,  6525,  6526,  6529,  6535,  6541,  6547,  6550,  6556,
    6557,  6558,  6559,  6565,  6571,  6572,  6573,  6574,  6575,  6576,
    6579,  6585,  6586,  6589,  6595,  6596,  6597,  6598,  6599,  6602,
    6616,  6617,  6618,  6619,  6620
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "junk",
  "CRLF line terminators", "newline", "colon", "semicolon", "comma", ".",
  "==", "!=", "<", ">", ">=", "<=", "<<", ">>", "&", "^", "!", "/", "*",
  "-", "@", "vmap", "+", "include", "define", "redefine", "undefine",
  "fib", "check", "socket", "transparent", "wildcard", "cgroupv2",
  "tproxy", "osf", "synproxy", "mss", "wscale", "typeof", "hook", "hooks",
  "device", "devices", "table", "tables", "chain", "chains", "rule",
  "rules", "sets", "set", "element", "map", "maps", "flowtable", "handle",
  "ruleset", "trace", "path", "inet", "netdev", "add", "update", "replace",
  "create", "insert", "delete", "get", "list", "reset", "flush", "rename",
  "describe", "import", "export", "new", "destroy", "monitor", "all",
  "accept", "drop", "continue", "jump", "goto", "return", "to", "constant",
  "interval", "dynamic", "auto-merge", "timeout", "gc-interval",
  "elements", "expires", "policy", "memory", "performance", "size", "flow",
  "offload", "meter", "meters", "flowtables", "number", "string",
  "quoted string", "string with a trailing asterisk", "ll", "nh", "th",
  "bridge", "ether", "saddr", "daddr", "type", "vlan", "id", "cfi", "dei",
  "pcp", "arp", "htype", "ptype", "hlen", "plen", "operation", "ip",
  "version", "hdrlength", "dscp", "ecn", "length", "frag-off", "ttl",
  "tos", "protocol", "checksum", "ptr", "value", "lsrr", "rr", "ssrr",
  "ra", "icmp", "code", "seq", "gateway", "mtu", "igmp", "mrt", "options",
  "ip6", "priority", "flowlabel", "nexthdr", "hoplimit", "icmpv6",
  "param-problem", "max-delay", "taddr", "ah", "reserved", "spi", "esp",
  "comp", "flags", "cpi", "port", "udp", "sport", "dport", "udplite",
  "csumcov", "tcp", "ackseq", "doff", "window", "urgptr", "option", "echo",
  "eol", "mptcp", "nop", "sack", "sack0", "sack1", "sack2", "sack3",
  "sack-permitted", "fastopen", "md5sig", "timestamp", "count", "left",
  "right", "tsval", "tsecr", "subtype", "dccp", "vxlan", "vni", "gre",
  "gretap", "geneve", "sctp", "chunk", "data", "init", "init-ack",
  "heartbeat", "heartbeat-ack", "abort", "shutdown", "shutdown-ack",
  "error", "cookie-echo", "cookie-ack", "ecne", "cwr", "shutdown-complete",
  "asconf-ack", "forward-tsn", "asconf", "tsn", "stream", "ssn", "ppid",
  "init-tag", "a-rwnd", "num-outbound-streams", "num-inbound-streams",
  "initial-tsn", "cum-tsn-ack", "num-gap-ack-blocks", "num-dup-tsns",
  "lowest-tsn", "seqno", "new-cum-tsn", "vtag", "rt", "rt0", "rt2", "srh",
  "seg-left", "addr", "last-entry", "tag", "sid", "hbh", "frag",
  "reserved2", "more-fragments", "dst", "mh", "meta", "mark", "iif",
  "iifname", "iiftype", "oif", "oifname", "oiftype", "skuid", "skgid",
  "nftrace", "rtclassid", "ibrname", "obrname", "pkttype", "cpu",
  "iifgroup", "oifgroup", "cgroup", "time", "nfproto", "l4proto",
  "iifkind", "oifkind", "ibrpvid", "ibrvproto", "sdif", "sdifname",
  "broute", "ibrhwaddr", "classid", "nexthop", "ct", "l3proto",
  "proto-src", "proto-dst", "zone", "direction", "event", "expectation",
  "expiration", "helper", "label", "state", "status", "original", "reply",
  "counter", "name", "packets", "bytes", "kbytes", "mbytes", "avgpkt",
  "last", "never", "tunnel", "erspan", "egress", "ingress", "gbp", "class",
  "opt-type", "counters", "quotas", "limits", "tunnels", "synproxys",
  "counts", "helpers", "log", "prefix", "group", "snaplen",
  "queue-threshold", "level", "emerg", "alert", "crit", "err", "warn",
  "notice", "info", "debug", "audit", "limit", "rate", "burst", "over",
  "until", "quota", "used", "secmark", "secmarks", "second", "minute",
  "hour", "day", "week", "reject", "with", "icmpx", "snat", "dnat",
  "masquerade", "redirect", "random", "fully-random", "persistent",
  "queue", "num", "bypass", "fanout", "dup", "fwd", "numgen", "inc", "mod",
  "offset", "jhash", "symhash", "seed", "position", "index", "comment",
  "xml", "json", "vm", "notrack", "exists", "missing", "exthdr", "ipsec",
  "reqid", "spnum", "in", "out", "xt", "filter", "nat", "route", "loose",
  "skip", "'='", "'{'", "'}'", "'('", "')'", "'|'", "'$'", "'['", "']'",
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
  "close_scope_queue", "close_scope_rate", "close_scope_reject",
  "close_scope_reset", "close_scope_rt", "close_scope_sctp",
  "close_scope_sctp_chunk", "close_scope_secmark", "close_scope_socket",
  "close_scope_tcp", "close_scope_tproxy", "close_scope_type",
  "close_scope_th", "close_scope_udp", "close_scope_udplite",
  "close_scope_log", "close_scope_synproxy", "close_scope_tunnel",
  "close_scope_xt", "common_block", "line", "base_cmd", "add_cmd",
  "replace_cmd", "create_cmd", "insert_cmd", "table_or_id_spec",
  "chain_or_id_spec", "set_or_id_spec", "obj_or_id_spec", "delete_cmd",
  "destroy_cmd", "get_cmd", "list_cmd_spec_table", "list_cmd_spec_any",
  "list_cmd", "basehook_device_name", "basehook_spec", "reset_cmd",
  "flush_cmd", "rename_cmd", "import_cmd", "export_cmd", "monitor_cmd",
  "monitor_event", "monitor_object", "monitor_format", "markup_format",
  "describe_cmd", "table_block_alloc", "table_options", "table_flags",
  "table_flag", "table_block", "chain_block_alloc", "chain_block",
  "subchain_block", "typeof_verdict_expr", "typeof_data_expr",
  "primary_typeof_expr", "typeof_expr", "set_block_alloc",
  "typeof_key_expr", "set_block", "set_block_expr", "set_flag_list",
  "set_flag", "map_block_alloc", "ct_obj_type_map", "map_block_obj_type",
  "map_block_obj_typeof", "map_block_data_interval", "map_block",
  "set_mechanism", "set_policy_spec", "flowtable_block_alloc",
  "flowtable_block", "flowtable_expr", "flowtable_list_expr",
  "flowtable_expr_member", "data_type_atom_expr", "data_type_expr",
  "obj_block_alloc", "counter_block", "quota_block", "ct_helper_block",
  "ct_timeout_block", "ct_expect_block", "connlimit_block", "limit_block",
  "secmark_block", "synproxy_block", "type_identifier", "hook_spec",
  "chain_type", "prio_spec", "extended_prio_name", "extended_prio_spec",
  "int_num", "dev_spec", "flags_spec", "policy_spec", "policy_expr",
  "chain_policy", "identifier", "string", "time_spec",
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
  "ct_limit_stmt_alloc", "connlimit_obj", "connlimit_config",
  "connlimit_stmt", "ct_limit_args", "counter_stmt", "counter_stmt_alloc",
  "counter_args", "counter_arg", "last_stmt_alloc", "last_stmt",
  "last_args", "log_stmt", "log_stmt_alloc", "log_args", "log_arg",
  "level_type", "log_flags", "log_flags_tcp", "log_flag_tcp",
  "limit_stmt_alloc", "limit_stmt", "limit_args", "quota_mode",
  "bytes_unit", "quota_used", "quota_stmt_alloc", "quota_stmt",
  "quota_args", "limit_mode", "limit_burst_pkts", "limit_rate_pkts",
  "limit_burst_bytes", "limit_rate_bytes", "time_unit", "reject_stmt",
  "reject_stmt_alloc", "reject_with_expr", "reject_opts", "nat_stmt",
  "nat_stmt_alloc", "tproxy_stmt", "synproxy_stmt", "synproxy_stmt_alloc",
  "synproxy_args", "synproxy_wscale", "synproxy_arg", "synproxy_config",
  "synproxy_obj", "synproxy_ts", "synproxy_sack", "primary_stmt_expr",
  "shift_stmt_expr", "and_stmt_expr", "exclusive_or_stmt_expr",
  "inclusive_or_stmt_expr", "basic_stmt_expr", "concat_stmt_expr",
  "map_stmt_expr_set", "map_stmt_expr", "prefix_stmt_expr",
  "range_stmt_expr", "multiton_stmt_expr", "stmt_expr", "nat_stmt_args",
  "masq_stmt", "masq_stmt_alloc", "masq_stmt_args", "redir_stmt",
  "redir_stmt_alloc", "redir_stmt_arg", "dup_stmt", "fwd_stmt",
  "nf_nat_flags", "nf_nat_flag", "queue_stmt", "queue_stmt_compat",
  "queue_stmt_alloc", "queue_stmt_args", "queue_stmt_arg", "queue_expr",
  "queue_stmt_expr_simple", "queue_stmt_expr", "queue_stmt_flags",
  "queue_stmt_flag", "set_elem_expr_stmt", "set_elem_expr_stmt_alloc",
  "set_stmt", "set_stmt_op", "map_stmt", "meter_stmt", "match_stmt",
  "variable_expr", "symbol_expr", "set_ref_expr", "set_ref_symbol_expr",
  "integer_expr", "selector_expr", "primary_expr", "fib_expr",
  "fib_result", "fib_flag", "fib_tuple", "osf_expr", "osf_ttl",
  "shift_expr", "and_expr", "exclusive_or_expr", "inclusive_or_expr",
  "basic_expr", "concat_expr", "prefix_rhs_expr", "range_rhs_expr",
  "multiton_rhs_expr", "map_expr", "expr", "set_expr", "set_list_expr",
  "set_list_member_expr", "meter_key_expr", "meter_key_expr_alloc",
  "set_elem_expr", "set_elem_key_expr", "set_elem_expr_alloc",
  "set_elem_options", "set_elem_time_spec", "set_elem_option",
  "set_elem_expr_options", "set_elem_stmt_list", "set_elem_stmt",
  "set_elem_expr_option", "set_lhs_expr", "set_rhs_expr",
  "initializer_expr", "counter_config", "counter_obj", "quota_config",
  "quota_obj", "secmark_config", "secmark_obj", "ct_obj_type",
  "ct_cmd_type", "ct_l4protoname", "ct_helper_config", "timeout_states",
  "timeout_state", "ct_timeout_config", "ct_expect_config", "ct_obj_alloc",
  "limit_config", "limit_obj", "erspan_block", "erspan_block_alloc",
  "erspan_config", "geneve_block", "geneve_block_alloc", "geneve_config",
  "vxlan_block", "vxlan_block_alloc", "vxlan_config", "tunnel_config",
  "tunnel_block", "tunnel_obj", "relational_expr", "list_rhs_expr",
  "rhs_expr", "shift_rhs_expr", "and_rhs_expr", "exclusive_or_rhs_expr",
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
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-1989)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1156)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1989,  9481, -1989,   497, -1989, -1989,   179,   161,   161,   161,
    1717,  1717,  1717,  1717,  1717,  1717,  1717,  1717, -1989, -1989,
    3918,   280,  3975,   294,  3432,   262,  5372,  1149,  1623,   278,
    8617,   190,   207,  3874,   825, -1989, -1989, -1989, -1989,   536,
    1717,  1717,  1717,  1717,  1717, -1989, -1989, -1989,   786, -1989,
     161, -1989,   161,   153,  7871, -1989,   497, -1989, -1989,   -38,
      19,   497,   161, -1989,    79,   110,  7871,   161, -1989,   -96,
   -1989,   161, -1989, -1989,  1717, -1989,  1717,  1717,  1717,  1717,
    1717,  1717,  1717,   688,  1717,  1717,  1717,  1717,  1717, -1989,
    1717, -1989,  1717,  1717,  1717,  1717,  1717,  1717,  1717,  1717,
     779,  1717,  1717,  1717,  1717,  1717, -1989,  1717, -1989,  1717,
    1717,  1717,  1717,  1717,  2504,  2504,  1717,  1717,  2504,  1717,
    1717,   971,  1717,  1717,  2504,  1308,  1717,  1717,  2504,  2504,
    2504,  2504,  2504,  1717,  1717,  1717,  2504, -1989,  1717,  1502,
    1717,  1717,  1717,  1717,  2504,  2504,  1717, -1989,  1717,  1717,
    1717,  1717,  1717,   534,  1717, -1989,  1717, -1989,  1224,   689,
     320,   481, -1989, -1989, -1989, -1989,  1011,   558,  1628,  1821,
    2177,  1877,   858,  2423,  3233,  1082,   357,   940,  1072,  1080,
    2857,   230,  4786,   270, -1989,  5172,  1112,  1010,   477,   486,
     784,   301,  1151,   532,   767,  8779, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989,  4352,   218, -1989, -1989,    44,
    8428,   374,  1319,   708,  8617,   161, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,   996, -1989, -1989,
     376, -1989, -1989,   996, -1989, -1989,  1717,  1717,  1717,  1717,
    1717,  1717,  1717,  1717,   884,  1717,  1717,  1717,  1717,  1717,
   -1989, -1989, -1989, -1989,  1392, -1989, -1989, -1989,  1717,  1717,
    1717,  1717,   -11, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,   730,   757,   759, -1989, -1989, -1989,   576,   581,  1057,
   -1989, -1989, -1989,   714, -1989, -1989, -1989,   137,   137, -1989,
     263,   161,  8779,  5901,   622, -1989,   378, -1989,   108,   634,
   -1989, -1989, -1989, -1989, -1989,   183,   879,   635, -1989,   851,
     983, -1989,   608,  7871, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,   176, -1989,
   -1989,   914,   678, -1989, -1989,   951,   949, -1989,  1038, -1989,
   -1989,   674, -1989,  5859, -1989, -1989,   831, -1989,   438, -1989,
     450, -1989, -1989, -1989, -1989,   987, -1989,   146, -1989, -1989,
   -1989, -1989,  1380,  1029,  1037,   663, -1989,   273, -1989,  7309,
   -1989, -1989, -1989,  1021, -1989, -1989, -1989,  1035, -1989, -1989,
    7664,  7664, -1989, -1989,   124,   681,   719, -1989, -1989,   734,
   -1989, -1989, -1989,   737, -1989,   746,  1100,  7871, -1989,    79,
     110, -1989,   -96, -1989, -1989,  1717,  1717,  1717,  1717,   859,
   -1989, -1989, -1989, -1989,  7871, -1989,   238, -1989, -1989, -1989,
     254, -1989, -1989, -1989,   290,   110, -1989, -1989, -1989,   309,
   -1989, -1989,   -96, -1989,   315,   771, -1989, -1989,  1717, -1989,
   -1989,  1717, -1989, -1989, -1989, -1989, -1989,   -96, -1989, -1989,
   -1989,  1135, -1989, -1989, -1989,  1717, -1989, -1989,   168, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1717,  1717, -1989,
   -1989, -1989,  1140,  1717,  1147,  2504, -1989,  1717,  1150, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,  1717, -1989,   161, -1989, -1989, -1989,   -96, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
    1717, -1989,   161, -1989, -1989, -1989, -1989,  1191, -1989, -1989,
   -1989, -1989, -1989,  1201,   844, -1989, -1989,   892, -1989, -1989,
    1008,   193, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989,   266,   590, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989,  1662, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,  3325, -1989, -1989, -1989, -1989,  1110,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  3811, -1989,  4982,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  3679, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
     706, -1989, -1989,   821, -1989, -1989, -1989, -1989, -1989, -1989,
     824, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,  3009, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
     852,   303,   868,  1142, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,   869,   863, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,   -96, -1989,
     771, -1989,  1717,  1717, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,   996, -1989,
   -1989, -1989, -1989, -1989,     0,   871,   -22,    -5,   194, -1989,
   -1989, -1989,  6145,  1181,  8172,  8617,  1093, -1989, -1989, -1989,
   -1989,  1264,  1268,   105,  1248,  1250,  1002,  1258,  3009,  1260,
    8172,  8172,  8172, -1989,  8172,  8617,  1123,  8172,  8172,  1231,
    1317, -1989,  7382,   157, -1989,  1317, -1989, -1989, -1989,   916,
   -1989,  1223, -1989, -1989, -1989,  1233,  1236,   914, -1989,   115,
   -1989, -1989, -1989,  1331,  1317,  1239,  1241,  1246,  2514,   951,
   -1989, -1989, -1989, -1989,  1256, -1989, -1989, -1989,  1259, -1989,
   -1989, -1989,   753, -1989, -1989,  8172, -1989, -1989,  6431,  1283,
     558,  1628,  1821,  2177, -1989,  2423,   796, -1989, -1989, -1989,
   -1989,  1286, -1989, -1989, -1989, -1989,  8172, -1989,  1394,  1359,
    1373,   997,   605,   305, -1989, -1989, -1989, -1989,  1425,  1111,
    1435, -1989, -1989, -1989, -1989, -1989,  1452, -1989, -1989, -1989,
   -1989, -1989,   282, -1989, -1989,  1461,  1482, -1989, -1989, -1989,
    1326,  1332, -1989, -1989,   831, -1989, -1989, -1989,  1467, -1989,
   -1989, -1989, -1989,  1490, -1989, -1989,  6717, -1989,  1490, -1989,
   -1989, -1989,    87, -1989, -1989,   987, -1989,  1497, -1989,   161,
   -1989,  1105, -1989,  9572,  9572,  9572,  9572,  9572,  8617,   144,
    8791, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,  9572, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,   312, -1989,  1479,  1498,  1520,  1144,
     874,  1561, -1989, -1989, -1989,  8791,  8172,  8172,  1434,   152,
     497,  1565, -1989,  1041,   497,  1470, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989,  1552,  1204,  1206,  1209, -1989,
    1211,  1221, -1989, -1989, -1989, -1989, -1989,  1321,  1234,  1289,
    1038,  1317, -1989, -1989,  1531,  1535,  1538,  1254,  1556, -1989,
    1560,  1271, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,  1495, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1717,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,  1569,   689, -1989, -1989, -1989, -1989, -1989,  1571,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1187,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,  1583, -1989,  1499, -1989, -1989,  1501,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1310,
   -1989,  1315,  1577, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
    1040,  1715,  1734,  1734, -1989, -1989, -1989,  1468, -1989, -1989,
   -1989, -1989,  1475,  1476, -1989,  1480,  1485,  1483,   225, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1621, -1989,
   -1989,  1626, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1414, -1989,  1431, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1627,  1629,  1364, -1989, -1989, -1989, -1989,
   -1989,  1635,   210, -1989, -1989, -1989,  1342, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,  1350,  1352,  1354,  1355,
    1656, -1989, -1989, -1989,   949, -1989, -1989, -1989,  1659, -1989,
   -1989, -1989, -1989,  8172,  2177,  2423,  1765,  7003, -1989,   146,
     159,  1768,  3259,  1317,  1317,  1667,  8617,  8172,  8172,  8172,
    8172,  1724,  8172, -1989, -1989, -1989, -1989,  1759, -1989, -1989,
     927,  1049,   303, -1989,   276,  1116,   177,  1742, -1989,  8172,
   -1989, -1989,   983,  1555,   649,   224, -1989,   787,  1561,   983,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,  1631,   261, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,   475,  1444,  1447,  1244,   595,   636, -1989,  1023,
   -1989, -1989, -1989,  8172,  1787,  8172, -1989, -1989, -1989,   670,
     683, -1989,  8172, -1989, -1989,  1390, -1989, -1989,  8172,  8172,
    8172,  8172,  8172,  1690,  8172,  8172,   197,  8172,  1490,  8172,
    1709,  1791,  1712,  4443,  4443, -1989, -1989, -1989,  8172,  1111,
    8172,  1111, -1989,  1780,  1789, -1989,  1123, -1989,  8617, -1989,
   -1989,  1479,  1498,  1520, -1989,   983, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1405,  9572,  9572,  9572,  9572,  9572,  9572,
    9572,  9572,  9645,  9572,  9572,   658, -1989,  1156, -1989, -1989,
   -1989, -1989, -1989,  1708, -1989,   972,  1528, -1989,  1001,  4038,
    2659,  3535,   928, -1989, -1989, -1989, -1989, -1989, -1989,  1415,
    1422,  1424,  1427, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,  1819, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  3259,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1421,  1430, -1989, -1989, -1989, -1989, -1989,
   -1989,  1364,   420,  1726, -1989, -1989, -1989, -1989, -1989,  1465,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1526,  1702, -1989,
     887,  1256,  3185, -1989,  1429,  1244, -1989,  1009, -1989, -1989,
    8172,  8172,  1837, -1989, -1989,  1736,  1736, -1989,   159, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1446,  1768,
    7871,   159, -1989, -1989, -1989, -1989,  8172, -1989, -1989, -1989,
   -1989, -1989,    87, -1989,  8617,    87,  8172,  1801, -1989,  9499,
   -1989,  1658, -1989,  1512, -1989, -1989, -1989, -1989, -1989, -1989,
     649, -1989,  1750,  1736, -1989,  1216, -1989,  7382, -1989,  5557,
   -1989, -1989, -1989, -1989,  1851, -1989,  1095, -1989, -1989, -1989,
    1844,  1761, -1989,  1762, -1989,  1522, -1989,  1478, -1989, -1989,
    1478, -1989,  1800,  1478, -1989, -1989,  8172, -1989, -1989, -1989,
   -1989, -1989,  1394,  1359,  1373, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,  1869,  8172,  1707,  8172, -1989, -1989, -1989, -1989,
    1111, -1989,  1111,  1490, -1989, -1989,   173,  7496,   164, -1989,
   -1989, -1989,  1565,  1872, -1989, -1989,  1479,  1498,  1520, -1989,
     246,  1565, -1989, -1989,   787,  9572,  9645, -1989,  1841,  1688,
   -1989, -1989, -1989, -1989, -1989,   161,   161,   161,   161,   161,
    1776,   899,   161,   161,   161,   161,   161, -1989, -1989, -1989,
     497, -1989,  1491,   163,  1255,  1788, -1989, -1989, -1989,   497,
     497,   497,   497,   497,  9187, -1989,  1736,  1736,  1492,  1521,
    1790,  1496,  1372, -1989, -1989, -1989,   497,   497,   497,   870,
   -1989,  9187,  1736,  1736,  1500,  1496,  1372, -1989, -1989, -1989,
     497,   497,   870,  1798,  1506,  1793, -1989, -1989, -1989, -1989,
   -1989,  2677,  4424,  2894,  3644,  1296, -1989, -1989, -1989, -1989,
   -1989,  1086, -1989, -1989, -1989,  4810,  1740, -1989, -1989,  1806,
   -1989, -1989, -1989,  1906, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1817, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,  1955, -1989,  1340,  1237,  1473,  3140,  1856, -1989, -1989,
   -1989, -1989, -1989,  1864,  1532,  1865,  1866,  1542,  1870,  1879,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,   497,  1444,  1447,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
    1522, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  8172, -1989,
   -1989, -1989, -1989, -1989, -1989,  8617,  1548,   159, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
    8172,   137,   137,   983,  1561, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,   649, -1989, -1989, -1989,
     497, -1989,   261, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
    1095,  1672,  1244,  1884, -1989, -1989, -1989, -1989, -1989, -1989,
    8172, -1989,  1903, -1989,  1490,  1490,  8617, -1989,   220,  1988,
     983, -1989,  1565,  1565,   497, -1989,  1804, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1991, -1989,
     161,   161,   161,   161, -1989, -1989, -1989, -1989, -1989, -1989,
     335, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1993, -1989,   497,
     497,   -96, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,  1994, -1989, -1989, -1989, -1989, -1989,  1229,
   -1989, -1989, -1989, -1989, -1989, -1989,   690,   497,   497,   -96,
    1045,  1229, -1989, -1989, -1989,  1849,   335,   497, -1989, -1989,
   -1989, -1989, -1989, -1989,  2421,  1893,  2875,  3265, -1989, -1989,
   -1989, -1989, -1989,  1901, -1989,  1364, -1989, -1989, -1989,  1599,
     765,  1717, -1989, -1989, -1989, -1989, -1989,  1905,  1907, -1989,
   -1989, -1989, -1989, -1989,  1736,  1909,   765,  1915,  1717, -1989,
   -1989, -1989, -1989, -1989,  1921,  1717, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,   165,   165, -1989, -1989,   165,   165,
   -1989, -1989,  1632,  1640,  1642, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,  7871, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,  1244, -1989, -1989, -1989,
   -1989, -1989,  8172,  1630,  8617, -1989,  1398,  7496, -1989,  1688,
   -1989, -1989,  1645,  1646,  1649,  1651,  1653,  1776, -1989, -1989,
   -1989, -1989,  1654,  1660,  1663,  1664,  1665,   165,   497, -1989,
   -1989,  1995,  9187, -1989, -1989, -1989, -1989,  1496, -1989,  1372,
   -1989,  8998, -1989, -1989, -1989,   819, -1989,   346,   497,   497,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989,  2023, -1989,  1668,
   -1989, -1989,   497,   497, -1989, -1989, -1989, -1989,   497,   497,
     497,   497,   497, -1989,  1898,   497, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,  1655, -1989, -1989, -1989,
   -1989, -1989,  1657,   983,  1804, -1989, -1989, -1989, -1989, -1989,
   -1989,  1670,  1673,  1674,  1676, -1989, -1989, -1989, -1989, -1989,
     174, -1989, -1989, -1989, -1989,  1953, -1989, -1989, -1989, -1989,
    9187, -1989,  6179, -1989, -1989, -1989, -1989, -1989, -1989,   497,
    2071, -1989,   497,  2072, -1989,   497,  1496,  1975, -1989, -1989,
   -1989,  1056, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,  1806, -1989,  1977, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989,   765, -1989, -1989, -1989, -1989, -1989,   213,
     508,  2276, -1989, -1989, -1989, -1989,  2486,  5196,  3016,  4153,
    2612, -1989, -1989, -1989, -1989,  3308,  2777,  3362,  1689,  3094,
     150, -1989,  1636, -1989,  1993, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989,  9187, -1989, -1989,   977, -1989,  1976,
    1979, -1989,  2081,   175, -1989,   497,  1982, -1989, -1989, -1989,
     497,  1987, -1989, -1989, -1989,   497,  1992,  1996,  1376,  1997,
   -1989, -1989, -1989,   497,   497,   497,   497,   497,   497,  3193,
    2495,  3077,  3317,   497,   497,   497,   497,   497, -1989, -1989,
     165,  1700,  1849, -1989,  2072, -1989, -1989, -1989, -1989,  1590,
    1977,   497, -1989, -1989, -1989,  1782, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,   497,   497,
     497,   497, -1989, -1989, -1989, -1989, -1989, -1989, -1989,   335,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  2000, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989,  1899, -1989, -1989, -1989, -1989,  1317, -1989
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     1,     0,     4,     5,     0,     0,     0,     0,
     463,   463,   463,   463,   463,   463,   463,   463,   467,   470,
     463,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,   469,     9,    28,    29,     0,
     463,   463,   463,   463,   463,    70,    69,     3,     0,    73,
       0,   464,     0,   488,     0,    68,     0,   455,   456,     0,
       0,     0,     0,   668,    89,    91,     0,     0,   307,     0,
     330,     0,   361,    74,   463,    75,   463,   463,   463,   463,
     463,   463,   463,     0,   463,   463,   463,   463,   463,    76,
     463,    77,   463,   463,   463,   463,   463,   463,   463,   463,
       0,   463,   463,   463,   463,   463,    78,   463,    79,   463,
     494,   463,   494,   463,   494,   494,   463,   463,   494,   463,
     494,     0,   463,   494,   494,     0,   463,   463,   494,   494,
     494,   494,   494,   463,   463,   463,   494,    35,   463,   494,
     463,   463,   463,   463,   494,   494,   463,    48,   463,   463,
     463,   463,   494,     0,   463,    82,   463,    83,     0,     0,
       0,   824,   792,   457,   458,   459,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1075,     0,     0,  1077,  1076,     0,
       0,     0,     0,    34,     0,     0,    87,   788,   787,   806,
     807,   808,   260,   803,   804,   796,  1038,   798,   797,   801,
     805,   802,   799,   800,   793,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,    54,
    1179,  1180,  1181,  1182,  1176,  1177,  1178,   794,  1428,  1429,
    1430,  1431,  1432,  1433,  1434,  1435,   795,     0,   257,   258,
       0,    33,   240,     0,    21,   242,   463,   463,   463,   463,
     463,   463,   463,   463,     0,   463,   463,   463,   463,   463,
      16,   245,   246,    39,   247,   468,   465,   466,   463,   463,
     463,   463,    13,   959,   919,   892,   894,    72,    71,   471,
     473,     0,     0,     0,   490,   489,   491,     0,   658,     0,
     778,   779,   780,     0,  1029,  1030,  1031,   546,   547,  1034,
       0,     0,     0,     0,   567,   572,     0,   579,     0,   618,
     636,   648,   649,   727,   733,   754,     0,     0,  1080,     0,
       7,    94,   496,   498,   511,   512,   513,   514,   516,   515,
     544,   526,   499,    63,   285,   541,   522,   550,     0,    12,
      13,   565,   573,    14,    60,   577,   623,    36,   612,    44,
      47,   639,    40,     0,    55,    61,   656,    40,   726,    40,
     732,    18,    24,   532,    45,   752,   538,     0,   539,   524,
     523,   827,   830,   832,   834,   836,   837,   844,   846,     0,
     845,   785,   549,  1038,   527,   533,   525,   793,   542,    64,
       0,     0,    67,   482,     0,     0,     0,    93,   476,     0,
      97,   323,   322,     0,   479,     0,     0,     0,   668,   116,
     118,   307,     0,   330,   361,   463,   463,   463,   463,    13,
     959,   919,   892,   894,     0,    61,     0,   142,   143,   144,
       0,   136,   137,   145,     0,   138,   139,   147,   148,     0,
     140,   141,     0,   149,     0,   151,   152,   896,   463,   897,
     895,   463,    13,    62,    36,    44,    52,     0,    61,   211,
     495,   217,   184,   185,   186,   463,   182,   187,   495,   181,
     183,   188,   189,   208,   207,   206,   200,   463,   494,   204,
     203,   205,   896,   463,   897,   494,   898,   463,     0,    13,
      62,   190,   192,   194,   212,   198,    36,    44,    52,   196,
      80,   227,   463,   224,   181,   225,   223,   229,     0,   230,
      13,   219,   221,    44,    81,   231,   232,   233,   234,   237,
     463,   236,     0,  1188,  1185,  1186,    57,     0,   815,   816,
     817,   818,   819,   821,     0,  1085,  1087,     0,  1086,    53,
       0,     0,  1426,  1427,    57,  1190,  1191,    56,    20,    56,
    1194,  1195,  1196,  1197,    30,     0,     0,  1200,  1201,  1202,
    1203,  1204,     9,  1222,  1223,  1217,  1212,  1213,  1214,  1215,
    1216,  1218,  1219,  1220,  1221,     0,    28,    56,  1237,  1236,
    1235,  1238,  1239,  1240,    31,    56,  1243,  1244,  1245,    32,
    1254,  1255,  1247,  1248,  1249,  1251,  1250,  1252,  1253,    29,
    1266,    56,  1262,  1259,  1258,  1263,  1261,  1260,  1264,  1265,
      31,  1269,  1272,  1268,  1270,  1271,     8,  1275,  1274,    19,
    1277,  1278,  1279,    11,  1283,  1284,  1281,  1282,    58,  1289,
    1286,  1287,  1288,    59,  1336,  1330,  1333,  1334,  1328,  1329,
    1331,  1332,  1335,  1337,     0,  1290,    56,  1370,  1371,     0,
      15,  1316,  1315,  1308,  1309,  1310,  1294,  1295,  1296,  1297,
    1298,  1299,  1300,  1301,  1302,  1303,    54,  1312,  1311,  1314,
    1313,  1305,  1306,  1307,  1323,  1325,  1324,     0,    25,     0,
    1320,  1319,  1318,  1317,  1424,  1421,  1422,     0,  1423,    50,
      56,    28,  1441,  1115,    29,  1440,  1443,  1113,  1114,    34,
       0,    49,    49,     0,    49,  1447,    49,  1450,  1449,  1451,
       0,    49,  1438,  1437,    27,  1459,  1456,  1454,  1455,  1457,
    1458,    23,  1462,  1461,    17,    56,  1465,  1468,  1464,  1467,
      38,  1041,  1042,  1043,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,    52,  1044,    34,    37,  1039,  1040,
    1130,  1131,  1137,  1123,  1124,  1122,  1132,  1133,  1153,  1126,
    1135,  1128,  1129,  1134,  1125,  1127,  1120,  1121,  1151,  1150,
    1152,    52,     0,    12,  1138,  1088,  1089,  1090,  1094,  1093,
       0,   844,     0,     0,    49,    27,    23,    17,    38,  1469,
    1098,  1099,  1074,  1097,     0,   786,  1175,   239,   259,    84,
     241,    85,    61,   162,   163,   138,   164,   165,     0,   166,
     168,   169,   463,   463,    13,    62,    36,    44,    52,    88,
      86,   248,   249,   251,   250,   253,   254,   252,   255,   916,
     559,   916,   916,    99,     0,     0,     0,   612,     0,   485,
     486,   487,     0,     0,     0,     0,     0,  1036,  1035,  1032,
    1033,     0,     0,     0,    37,     0,   558,     0,     0,    12,
       0,     0,     0,   606,     0,     0,     0,     0,     0,     0,
       0,     6,     0,     0,   848,     0,   497,   500,   543,     0,
     563,     0,   562,   520,   517,     0,     0,   566,   568,     0,
     574,   521,   528,     0,     0,     0,     0,     0,     0,   578,
     580,   621,   622,    46,     0,   518,   610,   611,     0,   619,
     519,   529,     0,   635,   530,     0,    48,    16,     0,     0,
      20,    30,     9,    28,   997,    29,     0,  1002,  1000,  1001,
      14,     0,    40,    40,   987,   988,     0,   688,   691,   693,
     695,   697,   698,   703,   708,   706,   707,   709,   711,   647,
     673,   674,   685,   686,   989,   675,   683,   676,   684,   678,
     680,   681,     0,   677,   679,     0,   710,   682,   531,   540,
       0,     0,   665,   664,   657,   663,   659,   534,     0,   745,
     746,   747,   725,   730,   743,   535,     0,   731,   736,   536,
     537,   748,     0,   770,   771,   753,   755,   758,   768,     0,
     790,     0,   789,     0,     0,     0,     0,     0,     0,     0,
       0,  1022,  1023,  1024,  1025,  1026,  1027,  1028,    20,    30,
       9,    28,    31,  1014,    29,    31,     8,    19,    11,    58,
      59,    54,    15,    25,    50,    40,     0,  1004,   972,  1005,
     841,   842,   984,   971,   961,   960,   976,   978,   980,   982,
     983,   970,  1006,  1007,   973,     0,     0,     0,     0,     7,
       0,   886,   885,   983,     0,     0,   421,    61,   267,   286,
     310,   343,   362,   492,   115,     0,     0,     0,     0,   122,
       0,     0,   916,   559,   916,   916,   124,     0,     0,     0,
     612,     0,   135,   159,     0,     0,     0,     0,     0,   150,
       0,     0,    12,   916,   154,   160,   157,   155,   158,   179,
     199,     0,   218,   180,   202,   201,    12,   215,    12,   463,
     191,   213,   195,   193,   197,   226,   228,   220,   222,   235,
     238,  1187,     0,     0,   814,    56,   811,   812,    22,     0,
    1083,   825,   826,    42,    42,  1425,  1192,  1189,  1198,  1193,
      20,    28,    20,    28,  1199,  1224,  1225,  1226,  1227,    28,
    1209,  1234,  1233,  1242,  1241,  1246,  1257,  1256,  1267,  1273,
    1276,  1280,  1285,    10,  1354,  1360,  1358,  1349,  1350,  1353,
    1355,  1344,  1345,  1346,  1347,  1348,  1356,  1351,  1352,  1357,
    1292,  1359,  1291,  1372,    15,  1368,  1304,  1322,  1321,  1326,
    1376,  1373,  1374,  1375,  1377,  1378,  1379,  1380,  1381,  1382,
    1383,  1384,  1385,  1386,  1387,  1388,  1389,  1390,  1407,    51,
    1419,  1442,  1109,  1110,  1116,    49,  1111,  1439,     0,  1444,
    1446,     0,  1448,  1436,  1453,  1460,  1466,  1463,  1045,  1037,
    1136,  1139,  1140,     0,  1142,     0,  1141,  1143,  1144,    12,
      12,  1145,  1117,     0,     0,  1091,  1471,  1470,  1472,  1473,
    1474,     0,     0,   809,   176,   167,     0,    12,   916,   171,
     177,   174,   172,   175,   243,   256,     0,     0,     0,     0,
       0,   381,    13,   954,   623,   411,    36,   386,     0,    44,
     416,   893,    52,     0,    28,    29,   650,     0,    61,     0,
     772,   774,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1145,     0,    13,    62,    36,    44,     0,   767,    45,
     762,   761,     0,   766,   764,   765,     0,   739,   741,     0,
     545,   861,     7,     7,   863,   857,   860,   983,   882,     7,
     847,   493,   295,   564,   570,   571,   569,   460,   575,   576,
     601,    20,     0,     0,   599,   587,   582,   583,   584,   585,
     588,   589,   590,   591,   592,   593,   594,   595,   596,   586,
     581,   607,     0,   624,   627,     0,     0,     0,    54,     0,
     717,   998,   999,     0,   712,     0,   990,   993,   994,   991,
     992,  1003,     0,   996,   995,     0,   673,   683,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   719,     0,
       0,     0,     0,     0,     0,   662,   661,   660,     0,     0,
       0,   734,   757,   762,   761,   756,     0,    10,     0,   828,
     829,   831,   833,   835,   838,     7,   551,   553,   843,   991,
    1013,   992,  1015,  1012,  1011,  1017,  1009,  1010,  1008,  1018,
    1016,  1019,  1020,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   967,   966,   983,  1079,  1160,
     888,   887,    65,     0,    66,     0,     0,   111,     0,     0,
       0,     0,     0,    61,   267,   286,   310,   343,   362,     0,
       0,     0,     0,    13,   954,    36,    44,    52,   483,   472,
     474,   286,   477,   480,   362,   161,    12,   216,   214,   209,
      12,     0,   820,   813,   810,    53,   822,   823,  1205,  1207,
    1206,  1208,    56,  1229,  1231,  1230,  1232,  1211,    28,     0,
    1366,  1338,  1363,  1340,  1367,  1343,  1364,  1365,  1341,  1361,
    1362,  1339,  1342,  1369,  1404,  1403,  1405,  1406,  1412,  1394,
    1395,  1396,  1397,  1409,  1398,  1399,  1400,  1401,  1402,  1410,
    1411,  1413,  1414,  1415,  1416,  1417,  1418,    56,  1393,  1392,
    1408,    50,  1112,     0,     0,    28,    28,    29,    29,  1118,
    1119,  1091,  1091,     0,    26,  1096,  1100,  1101,    34,     0,
     362,   178,    12,   396,   406,   401,   391,     0,     0,   100,
       0,     0,     0,   107,     0,     0,   102,     0,   109,   652,
       0,     0,   651,   506,   775,     0,     0,   869,   773,   864,
    1354,  1358,  1353,  1357,  1359,    54,    10,    10,     0,   856,
       0,   854,    37,    12,    12,    12,     0,    12,   503,   507,
     504,   505,     0,   749,     0,     0,     0,     0,   851,     0,
     852,     0,   567,     0,   618,    12,    13,    14,    36,    44,
     862,   872,     0,     0,   881,   858,   870,   850,   849,     0,
     600,    28,   604,   605,    54,   603,     0,   613,   614,   615,
       0,     0,   608,     0,   609,   616,   637,     0,   641,   638,
       0,   643,     0,     0,   645,   718,     0,   722,   724,   687,
     689,   690,   692,   694,   696,   704,   705,   699,   702,   701,
     700,   714,   713,     0,     0,     0,  1154,  1155,  1156,  1157,
     728,   744,   735,   737,   769,   791,     0,     0,     0,   554,
    1021,   969,   963,     0,   974,   975,   977,   979,   981,   968,
     839,   962,   840,   985,   986,     0,     0,   839,     0,   669,
      61,   423,   422,   425,   424,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    90,   269,   268,
       0,   263,     0,     0,     0,     0,    92,   288,   287,     0,
       0,     0,     0,     0,     0,   319,     0,     0,     0,     0,
       0,     0,     0,    95,   312,   311,     0,     0,     0,     0,
     501,     0,     0,     0,     0,     0,     0,    96,   345,   344,
       0,     0,     0,     0,     0,     0,    13,    98,   364,   363,
     132,     0,     0,     0,     0,     0,   396,   406,   401,   391,
     125,     0,   130,   126,   131,     0,     0,   156,   210,     0,
    1084,  1228,  1210,     0,  1391,  1420,  1445,  1452,  1146,  1147,
    1148,  1149,    41,     0,    26,  1092,  1108,  1104,  1103,  1102,
      34,     0,   173,     0,     0,     0,     0,     0,    13,   383,
     382,   385,   384,     0,     0,     0,     0,     0,     0,     0,
     940,   934,   924,    62,   956,   955,   958,     0,   624,   627,
      36,   413,   412,   415,   414,    44,   388,   387,   390,   389,
     616,    52,   418,   417,   420,   419,   653,   655,     0,   867,
     868,   865,  1327,  1082,  1081,     0,     0,   855,  1078,   508,
     510,   509,    12,  1158,   763,   759,   760,    45,    45,   740,
       0,     0,     0,     7,   883,   884,   558,   876,   874,   878,
     875,   877,   873,   866,   879,   880,   859,   871,   548,   296,
       0,   598,     0,   597,   630,   631,   632,   633,   634,   626,
       0,     0,     0,     0,   620,    56,    56,    48,    56,   715,
       0,   721,     0,   723,   729,   738,     0,   776,     0,     0,
       7,   552,   965,   964,     0,   670,   671,   112,   484,   380,
     475,   285,   478,   307,   330,   481,   361,   266,   262,   264,
       0,     0,     0,     0,   380,   380,   380,   380,   380,   270,
       0,   453,   454,    43,   452,   451,   432,   433,   434,    56,
     449,   289,   291,   290,   294,   292,   305,   308,   304,     0,
       0,     0,   360,   359,    43,   358,   426,   428,   429,   427,
     377,   430,   378,    56,   376,   326,   327,   329,   328,     0,
     325,   313,   320,   321,   317,   502,     0,     0,     0,     0,
       0,     0,   356,   355,   353,     0,     0,     0,   367,   117,
     119,   120,   121,   123,     0,     0,     0,     0,    62,   146,
     153,  1183,    10,     0,  1095,  1091,  1107,  1105,   170,     0,
       0,     0,    12,   398,   397,   400,   399,     0,     0,    12,
     408,   407,   410,   409,     0,     0,     0,     0,     0,    12,
     403,   402,   405,   404,     0,     0,    12,   393,   392,   395,
     394,   889,   101,   942,     0,     0,   949,   950,     0,     0,
     947,   948,     0,     0,     0,   113,   957,    46,    46,   108,
     103,   891,   110,   654,     0,   783,  1159,   750,   751,   742,
     853,   297,   602,   629,   625,   628,     0,    31,    31,   646,
     644,   716,     0,     0,     0,   777,     0,   556,   555,   669,
     672,   666,     0,     0,     0,     0,     0,     0,   380,   380,
     380,   380,     0,     0,     0,     0,     0,     0,     0,   370,
     450,     0,     0,   315,   316,   318,   357,     0,   309,     0,
     314,     0,   346,   347,   354,   342,   352,     0,     0,     0,
     368,    12,    12,    12,    12,   133,  1184,     0,    26,     0,
      58,    54,     0,     0,   105,   561,   560,   114,     0,     0,
       0,     0,     0,   106,     0,     0,   104,    28,    28,    29,
      29,   936,   930,   920,   917,   918,     0,   617,   640,   642,
     720,   781,     0,     7,   671,   421,   286,   310,   343,   362,
     265,     0,     0,     0,     0,   381,   954,   411,   386,   416,
       0,   371,   374,   375,   293,     0,   306,   379,   324,    61,
       0,   303,     0,    13,    36,    44,    52,   302,   301,     0,
       0,   339,     0,   298,   341,     0,     0,     0,   443,   437,
     436,   440,   435,   438,   439,   365,   366,   128,   134,   129,
     127,     0,  1106,     0,   902,   901,   908,   910,   913,   914,
     911,   912,   915,     0,   904,   943,   944,   945,   946,     0,
       0,     0,   784,   782,   557,   667,     0,     0,     0,     0,
       0,   396,   406,   401,   391,     0,     0,     0,     0,     0,
       7,   369,   448,   338,   300,   331,   333,   332,    12,   334,
     336,   335,   337,   349,     0,   351,    56,     0,   444,     0,
       0,  1293,     0,     0,   905,     0,     0,   952,   938,   937,
       0,     0,   953,   932,   931,     0,     0,     0,     0,     0,
     951,   922,   921,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   372,
       0,     0,     0,   340,   299,   350,    56,   442,   441,     0,
       0,     0,    56,   941,   939,     0,   933,   929,   925,   928,
     927,   926,   923,    61,   271,   272,   273,   274,     0,     0,
       0,     0,    13,    62,    36,    44,    52,   445,   446,     0,
     431,   348,   461,   462,   907,   906,    43,   903,     0,   282,
      12,    12,    12,    12,   275,   283,   280,   276,   281,   447,
     909,     0,   278,   284,   279,   277,     0,   935
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1989, -1989,    -1, -1284,  1042,    49, -1376,  1050,   921,  -362,
    -884,  -804,  1160,  1292, -1989,  1054,  -531, -1989, -1989,  1297,
   -1989,  -154, -1718,  1301,   -36,   -27,  1541,  -598, -1989, -1989,
    -722, -1989,  -443,   457,  1294, -1989,  -295, -1989,   948, -1934,
    -436, -1253,  -477, -1989,  -921,  -543,  -976, -1989,  -532,   582,
    -692, -1989,  -481,  1558, -1049,  1066, -1989,  -452,  -525, -1989,
      24, -1989, -1989,  2128, -1989, -1989, -1989,  1868,  1871,   139,
     343, -1989, -1989, -1989,  2017,   710, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,    33, -1989,
    1711, -1989, -1989,   -56,   638,  -366, -1445, -1989, -1989, -1989,
   -1988, -1689,  -444, -1989, -1449,  -410,   317,   -74,  -438, -1989,
     -77, -1989, -1989, -1448, -1471, -1989,  -418, -1450, -1933, -1989,
    -220,   -66, -1722,  -286,  -133,  -135, -1762, -1748, -1746, -1709,
    -131,  -132,  -121, -1989, -1989, -1989,  -274, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989,   119,  -795, -1556, -1989,   138,
     -91,  3713, -1989,   785, -1989, -1989,   503, -1989,   382,   922,
    1888, -1989,   735, -1989,  -653,  1738, -1989, -1989,   295,   585,
    1225,    99,   -23, -1989, -1989, -1477, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1457,  -359, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989,   162, -1989,  1067, -1989, -1269, -1989, -1239, -1989,
   -1989,  1262, -1989, -1231, -1989, -1989, -1989, -1989,  1252, -1989,
   -1989, -1989,   200, -1989, -1229, -1989,  1805, -1340,   255, -1989,
   -1228, -1989,   873,   272,   553,   274,   560,   204, -1989, -1989,
   -1230, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1313,  1192,
    -352,  1764,     5,   -79,  -621,   778,   780,   777, -1989,  -857,
   -1989, -1989, -1989, -1989, -1989, -1989,  1813, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989,  -384,   763, -1989, -1989,
   -1989, -1989,  1188,   543,  -929,   545,  1325,   766, -1392, -1989,
   -1989,  1908, -1989, -1989, -1989,   -60,  1786,  -849,  -358,  1910,
   -1712,  2204,     4, -1989, -1989,  1062,    95, -1989, -1989, -1989,
   -1989, -1989,  -158,  -217, -1989, -1989,   739,  -837,  2018,   -47,
   -1989,   865,   291, -1989, -1636, -1989, -1989,   578, -1989, -1474,
   -1989,   549, -1475,   550, -1989, -1989,  1825,  -657,  1799,  -648,
    1795,  -623,  1802,   132, -1989, -1898, -1989, -1989,  -197, -1989,
   -1989,  -638,  -615,  1811, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1463,  1820, -1989,  -391,  -329,  -912,
    -892,  -885, -1989,  -357,  -900, -1989,  1107,   -80,  -902, -1989,
   -1557,  -328,   269,  1924, -1989,    15, -1989,   271, -1989, -1989,
     304, -1424, -1989,   323, -1989, -1989, -1989, -1989, -1989,   334,
    -301,   345,  1544,   389,  1939,  1943, -1989, -1989,  -512,   257,
   -1989, -1989, -1989,   606,   -63, -1989, -1989,   -72, -1989,   -32,
   -1989,   -26, -1989,   -71, -1989, -1989, -1989,   -61, -1989,   -48,
   -1989,   -34, -1989,   -19, -1989,    -8, -1989,    -7, -1989,    10,
   -1989,    11, -1989,    22, -1989,    31,  1574, -1989,   -69, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,  1608,
   -1127, -1989, -1989, -1989, -1989, -1989,    45, -1989, -1989, -1989,
   -1989,  1052, -1989, -1989,    50, -1989,    54, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989, -1989,
   -1989, -1989, -1989, -1989, -1989, -1989, -1989
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,  1858,   912,  1208,  1428,  1569,  1210,  1292,   873,
     931,  1235,   859,  1275,  1029,  1209,  1426,   841,  1554,  1274,
    1030,   717,  1896,  1273,  1479,  1481,  1427,  1202,  1204,   839,
     832,   540,   945,  1279,  1277,   860,   954,  2124,  1556,  2230,
     950,  1031,  1411,   951,   554,  1266,  1260,  1611,  1148,  1180,
     836,  1008,  1186,  1171,  1211,  1212,   932,  1009,  1145,   918,
    1859,    47,    48,    49,    75,    89,    91,   469,   473,   478,
     465,   106,   300,   108,   506,   507,   137,  1152,   499,   147,
     155,   157,   281,   284,   303,   304,   868,  1314,   282,   226,
     435,  1810,  2038,  2039,  1518,   436,  1519,  1709,  2328,  2329,
    2066,  2067,   439,  1836,  1520,   440,  2089,  2090,   443,  2398,
    2331,  2332,  2336,  1521,  1837,  2074,   445,  1522,  2228,  2310,
    2311,  2082,  2083,  2212,  1638,  1644,  1906,  1903,  1905,  1904,
    1642,  1647,  1516,  2084,  1819,  2059,  2248,  2341,  2342,  2343,
    2452,  1820,  1821,  2053,  2054,  2028,   227,  1389,  2494,    50,
      51,    62,   472,    53,   476,  2031,   480,   481,  2033,    72,
     486,  2036,   467,   468,  2029,   324,   325,   326,    54,   447,
    1657,   510,  1823,   362,   363,  1839,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
    1476,  1768,  1769,   378,  1317,  2143,   379,   922,   380,   381,
     927,   928,   382,   383,   930,   384,   385,   939,   940,  1409,
    1395,  1714,  1715,   386,   387,   943,  1328,  1720,  2004,   388,
     389,   949,   944,  1722,  1413,  1724,  1414,  1999,   390,   391,
    1728,   953,   392,   393,   394,   395,   396,  1014,  1015,  1016,
    1794,   434,  2026,  2211,   977,   978,   979,   980,   981,   982,
     983,  1748,   984,   985,   986,   987,   988,   989,   397,   398,
    1022,   399,   400,  1027,   401,   402,  1023,  1024,   403,   404,
     405,  1035,  1036,  1357,  1358,  1359,  1037,  1038,  1339,  1340,
     406,   407,   408,   409,   410,   228,   990,  1041,  1078,   991,
     231,   411,   992,  1178,   573,   574,   993,   581,   412,   413,
     414,   415,   416,   417,  1080,  1081,  1082,   418,   419,   420,
     913,   914,  1670,  1671,  1373,  1374,  1375,  1658,  1984,  1659,
    1705,  1700,  1701,  1706,  1376,  1973,  1100,  1912,   874,  1939,
     877,  1945,   878,   491,   528,  2262,  2160,  2413,  2414,  2136,
    2153,  1316,  1934,   876,  2371,  2174,  2433,  2370,  2173,  2425,
    2369,  2172,  2420,  1927,  1640,   875,   421,  1101,  1102,  1086,
    1087,  1088,  1089,  1377,  1091,   994,   995,   996,  1094,  1095,
     422,   889,   997,   787,   788,   236,   424,   998,   579,   817,
     999,  1624,   820,  1000,  1302,   833,  1628,  1900,   240,  1001,
     740,  1003,   741,  1004,   812,   813,  1289,  1290,   814,  1005,
    1006,   425,   426,  1007,  2122,   245,   567,   246,   588,   247,
     594,   248,   602,   249,   616,  1199,  1568,   250,   624,   251,
     629,   252,   639,   253,   650,   254,   656,   255,   659,   256,
     663,   257,   668,   258,   673,   259,   707,   708,   709,   260,
     710,   261,   723,   262,   718,   263,   428,   685,  1230,  1664,
    1232,  1581,  1573,  1578,  1571,  1575,   264,   690,  1258,  1610,
    1593,  1599,  1588,  1259,   265,   729,   266,   584,   267,   268,
     754,   269,   742,   270,   744,   271,   746,   272,   751,   273,
     761,   274,   764,   275,   770,   276,   829
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      45,   306,    55,   821,   917,  1161,  1164,  1118,   919,   441,
     890,   307,  1378,  1133,  1236,  1120,  1028,  1264,   924,   500,
    1486,   500,   442,   508,   508,    46,   883,   508,  1084,   500,
     719,   361,   500,   508,   233,  1421,  1121,   508,   508,   508,
     508,   508,  1119,   437,  1852,   508,  1150,   318,   508,  1042,
    1850,  1146,  1207,   508,   508,   429,   909,  1187,   233,  1147,
     432,   500,  1090,  1840,  1840,   285,   834,  1141,  1363,   423,
     233,  1871,  1139,  1103,  1103,  1725,  1766,  1863,  1865,  1864,
    1862,   423,  1107,  1331,  1117,   305,  1431,  1149,  1688,  1690,
    1085,  1765,  1002,  1162,  1876,  1707,  1875,  1126,  1491,  1949,
    1950,  1163,  1017,  1462,  1025,  1695,  1683,  2117,  1188,  1137,
     693,   696,  2068,   693,   696,  1370,   722,  1168,  2114,  1435,
    1381,   697,  2116,  2100,   697,   234,    59,    60,    61,  2068,
    1144,  2019,  1975,  1471,   698,  1696,  1201,   698,  1166,  1396,
    2236,  1469,  1470,  1697,  1203,  1698,  1699,  1985,   699,   234,
     694,   699,  2096,   694,  1472,   911,   695,   911,  2115,   695,
    1206,   234,  1473,   700,  1105,  1379,   700,  1160,  1039,   319,
    1039,   320,  2020,  2249,   701,   702,  2126,   701,   702,  2016,
    1901,   433,  2390,  2460,  1951,  1466,   438,  1731,  1167,  1734,
     444,  1767,   703,   704,   162,   703,   704,  1892,  1894,  1267,
    1477,  1269,  1789,  1270,   705,  1233,  1345,   705,  1272,   501,
     789,   503,   321,   706,     3,  1665,   706,  1322,     4,   516,
       5,  1039,   520,  1387,   233,  1982,  2204,   711,   233,  1329,
     711,   470,   712,  1318,  1319,   712,   713,   483,   546,   713,
       6,     7,     8,     9,  2316,    57,  2051,  2052,  2270,  1261,
     277,   559,  1278,  1655,  -792,  1332,  1656,   527,   163,   164,
     165,  1326,  -792,  -792,  -792,  -792,  1684,   283,  1489,    57,
    2069,  2070,   905,   163,   164,   165,  -464,   888,   888,  1280,
     815,  1296,  1048,   920,  1276,  -801,  2097,  2098,    56,  2018,
    1953,  1954,  -801,  -801,  -801,  -801,  -890,  1134,  1049,   235,
    1291,   237,   163,   164,   165,  1940,   360,  1320,    66,  1840,
     837,   225,  1048,  1135,  1445,   234,   840,   107,  1702,   234,
    1494,  1703,  1324,   235,  1183,   237,  1313,   156,   891,  1050,
    1310,    74,  -801,  1495,   238,   235,  1331,   237,   816,  1093,
     731,   946,   947,  1607,   835,    90,    57,  1040,   686,  1136,
    1093,  1093,   906,   239,   575,   576,   577,   789,   238,  1050,
    1608,  1446,    57,   430,   241,   734,   892,   233,  1138,  2337,
     238,  1449,  1083,  1450,  1140,   242,  1626,   239,   423,  1488,
    1325,  1190,  2095,  1083,  1083,   464,  1351,  1852,   241,   239,
    1304,  -890,   441,  1850,  1609,  2095,  1191,  1327,    57,   242,
     241,   714,  1321,   687,   688,   442,  1840,  1840,   818,   715,
    1712,   242,   689,  1311,   902,  1713,  2316,    57,   819,   243,
     431,  1312,   441,    57,  1114,   470,   853,   500,  1388,   847,
    1583,  1695,  1306,   752,   508,   442,  1695,   441,  1305,   716,
     815,  1132,  1883,   243,   492,   493,   494,   495,   496,    58,
     442,   233,   903,  2338,  2339,   243,   887,   887,   234,   753,
     893,  1696,   423,  1691,  1480,  2415,  1696,  1482,   233,  1697,
    1533,  1698,  1699,    58,  1697,  2024,  1698,  1699,  1692,   423,
    -464,  -261,  1536,  1951,  1529,   345,  1531,  1532,   441,   235,
    1654,   237,  1309,   235,   225,   237,  1716,  2005,   816,  1184,
    2006,   442,     4,  2008,     5,  1546,   657,  1346,  1537,     3,
    1691,  1982,  -285,     4,  1535,     5,  1693,    68,    69,    70,
    2068,  1694,   921,   658,   238,  1692,  1106,  1018,   238,  2333,
    1451,  2416,   345,   322,   323,     6,     7,     8,     9,  1026,
    2352,   915,   234,   239,   225,  1685,  1475,   239,  1666,  1667,
      58,   225,  2330,   225,   241,  1511,  2509,   225,   241,   234,
    1380,  2095,  2510,  1693,  1238,   242,    58,  2021,  1694,   242,
     225,  2019,   225,   278,   279,   280,  2017,  2391,  2461,   578,
    1200,   560,   451,   452,   453,   433,  1776,  1746,  2268,   319,
     278,   279,   280,   320,  1774,  1775,  1330,  1749,   438,   360,
     482,  1627,    58,   444,   225,  1448,   915,  1777,  2068,   243,
     497,  1452,  1205,   243,  2407,  1778,  2417,  1155,   580,   512,
     513,    58,  2442,  2205,  2203,   519,  1443,    58,  1444,   842,
     308,  2394,   235,  2439,   237,  1885,   244,  2441,   854,   855,
     856,   857,   858,   547,   548,   549,   696,   693,   696,  2293,
    1239,  1194,  -792,   557,   558,  1517,   697,   561,   697,  1558,
     427,  1560,  2195,   320,   762,   882,  1494,   238,  1341,   698,
    1632,   698,   427,  2440,   585,   586,   587,  1433,  1434,  1785,
     477,  1170,  -801,   699,   901,   699,   239,   694,  1362,  2190,
     763,  1042,  2454,   695,  1553,  1262,  2241,   241,   700,  2232,
     700,  2258,   162,  1726,  1505,  1192,   731,  1263,   242,   701,
     702,   701,   702,  1727,  2187,  2188,   235,  1695,   237,  1493,
    1193,  1478,  1612,   541,   908,   743,  1732,   703,   704,   703,
     704,   734,   309,   235,   745,   237,  2207,  2227,  1507,   705,
    2340,   705,   225,   162,  1726,    63,  2256,  1696,   706,   823,
     706,   238,   243,   225,  1730,  1697,  1968,  1698,  1699, -1109,
     838, -1109,   711,  1523,   711,   731,  1506,   712,   238,   712,
     239,   713, -1110,   713, -1110,   312,   313,   314,   315,   316,
    1492,   241,   455,  1717,  1718,  1719,   317,   239,   441,  1974,
     734,     4,   242,     5,   848,   849,  1623,    65,   241,  1893,
    1648,   442,  1019,  1020,  1021,   568,   569,  1378,  1513,   242,
    1503,   448,  2292,  1979,  1019,  1020,  1021,  1740,  1741,   459,
     460,   461,   462,   463,  1747,   511,   244,  2421,   514,  1679,
     244,  1142,  1093,   310,   521,   311,   243,   879,   531,   532,
     533,   534,   535,  2386,   498,  1360,   539,  1363,  2378,  2380,
    2379,  2377,  1331,   243,   551,   552,  2287,   733,  2319,   446,
    1710,   529,   530,   450,   880,  1372,   881,  1378,   536,   537,
     538,  1010,  1011,   487,   846,     4,  1174,     5,   550,   475,
     446,   553,  1501,  1643,   456,   765,  1653,   884,     3,   233,
    1474,   886,     4,  1646,     5,  1502,  1437,  1503,   504,   766,
    1416,  1315,  1852,  2493,   301,   302,  1897,   767,  1850,   233,
    2334,  2422,  1680,  1417,     6,     7,     8,     9, -1109,  1490,
    1681,  1840,  1840,   446,   545,   768,   731,  1429,   900,     3,
    1418, -1110,   769,     4,   556,     5,  -787,  2260,  1430,  1691,
     904,   562,  2261,  -787,  -787,  -787,  -787,   733,   570,   571,
    -759,   734,   572,   747,  1692,     6,     7,     8,     9,   910,
    1639,   345,  1175,  1093,  1093,  1093,  1093,  1093,   907,   427,
    1093,  1853,  1463,  1952,  1854,   488,   625,     4,   487,     5,
     234,  1042,     4,  -787,     5,   457,  2237,   458,   911,  1040,
     915,  1678,  1693,  2040,   737,   738,  1093,  1694,   626, -1109,
     234,  1629,     3,  1083,   454,  1874,     4,  1913,     5,  2374,
       3,   627, -1110,  1011,     4,  1093,     5,  1914,   517,   518,
    1154,   485,  1993,  1012,  1915,  1916,  1013,   929,     6,     7,
       8,     9,   952,   748,   749,   750,     6,     7,     8,     9,
    1795,   515,  1917,   869,   870,   871,   872,  1045,  1083,  1501,
    1796,  2245,   233,   427,  2237,  1797,  1046,  1798,  -807,  1799,
    1918,  1919,  1513,  1169,  1503,  -807,  -807,  -807,  -807,  1047,
     427,  1860,  -760,   845,   446,  1096,   489,  1763,   490,  2409,
     852,  1881,  2410,  1108,   737,   738,  2199,     3,  1042,  1097,
    1920,     4,  1872,     5,  1921,  2041,   739,  1855,   660,  1512,
    1873,   830,   831,  1514,  1772,  -807,  2448,  1176,  1177,   661,
     662,  1781,  1419,     6,     7,     8,     9,   163,   164,   165,
     933,  1109,   330,   331,  2323,  -802,  1884,   332,   730,  1669,
     162,  1726,  -802,  -802,  -802,  -802,  1110,  1771,  1773,  1111,
     731,  1733,   732,   234,  1779,  1773,  1782,  1784,  1112,  2044,
    2045,  2046,  2047,  2048,   235,  1559,   237,  1561,  1467,   321,
    1691,   733,  2324,  1567,  1501,   734,  -890,  2325,   735,  2326,
    1800,  1870,  -802,  -361,   235,  1692,   237,  1786,  2127,  1503,
    1151,   489,   345,   490,   582,   583,   739,  -899,   628,   238,
    1122,  1123,  1124,  1125,  -900,  1307,  2042,  1159,  2043,  1172,
     138,   139,  1922,   140,   141,   142,  1913,   664,   239,   238,
    1173,  2354,   665,  1693,   651,   850,  1914,  1234,  1694,   241,
     669,   925,   926,  1915,  1916,  1179,  1143,  1293,  1364,  1268,
     242,   652,  1271,  1856,     4,  1157,     5,  2239,     3,  1365,
     653,  1917,     4,  1294,     5,   666,   667,   654,   655,  1295,
     242,  1341,   724,   670,   671,  1980,   672,   736,  1156,  1918,
    1919,  1301,  1158,  1981,     6,     7,     8,     9,  1303,   915,
    1337,   755,  1584,  1323,   243,  1342,  1585,  1586,  1587,  1040,
     934,   935,   936,   937,   938,   725,   726,   756,  1343,  1920,
    1923,  1801,  1344,  1921,   243,   941,   942,     3,   737,   738,
     923,     4,  1347,     5,  1348,  1562,  1802,  1378,  1349,   757,
    1702,  1956,  1350,  1703,  1352,  1803,   758,   235,  1382,   237,
    1369,   727,  1563,     6,     7,     8,     9,  1165,  1564,  1565,
    1383,  1857,   563,  -787,  1978,   564,   565,   566,  2027,  1853,
    1384,     3,  1854,  1385,  1804,     4,  1397,     5,  1398,  1805,
     233,  1806,   238,  1399,   728,  1032,  1033,  1034,  1437,  1437,
    1437,  1437,  1437,  1412,  1437,  1437,  1415,     6,     7,     8,
       9,   239,  1425,  1757,  1757,  1432,  2014,  1440,  2015,   278,
     279,   280,   241,   915,   946,   947,  1040,  2218,  2219,  2220,
    2221,   915,  1441,   242,  2022,  2023,  1043,  1044,  2175,  1750,
     739,  1922,   522,  1442,  1807,   759,   760,  1181,  1182,  2182,
    1438,  1439,  1941,  1390,  1093,  1093,  1093,  1093,  1093,  1093,
    1093,  1093,  1093,  1093,  1093,   163,   164,   165,  1773,  1773,
       3,  1447,  2312,  1455,     4,  1566,     5,   243,  2129,  1456,
     861,   234,   862, -1154,   863,   864,  1391,  1994,  1995,  1996,
    1997,  1998,   865,   866,   143,  -807,     6,     7,     8,     9,
   -1155,  1392,  2085,  2086,  2087,  1855,  2088,  1362,   915,  1453,
     144,   145,   233,  1458,     3,  1019,  1020,  1021,     4,  2130,
       5,   334,   335,   336,  1971,  1972,   339,  2179,   867,  2118,
    1454,   244,  1033,  1034,  2108,  1496,  1497,   146,  1459,  2180,
       6,     7,     8,     9,   523,  1466,  1691,  1468,  1393,  1576,
    1577,   244,  1579,  1580,  1788,  1791,  1498,  1808,  1817,  1834,
    1848,  1692,  -802,   233,  2197,  2198,  1092,  2200,   345,     3,
    1615,  1616,  1882,     4,   423,     5,  -612,  1092,  1092,  1499,
    1792,  1510,  1809,  1818,  1835,  1849,  2162,  1617,  1618,   505,
    1500,   542,  1717,  1718,  1719,     6,     7,     8,     9,  1693,
     911,  1689,   824,   234,  1694,    18,    19,  2144,  1105,  2355,
    1504,   825,   826,  1494,  2145,   827,   828,  1515,  2231,  1888,
    1889,  1898,  1899,  2137,  2138,   162,  1726,   916,  1308,  2214,
    1890,  1891,  1105,  2255,  1331,  2312,  2215,  1394,   915,  2288,
    2289,  1856,  2238,  1547,  2076,   524,  1524,   490,  1525,  1093,
    -463,  1526,  2146,  1527,   234,   235,    35,   237,  2216,   915,
    2072,  2073,  1965,  1528,  1360,  1463,    36,  1093,  1320,  2077,
    2078,  2131,    37,  1324,   525,   526,  1534,  1909,  1538,  1924,
    2139,  1931,  1539,  1936,   890,  1540,  1942,  2147,  2164,  2165,
     238,  2056,  2057,  2058,   244,  2487,  1541,    38,  2168,  2169,
    1372,  2235,  1910,  1542,  1925,  2213,  1932,  1543,  1937,   239,
     148,  1943,   149,  1544,   233,  1991,  1551,   150,  1555,   151,
     241,  2450,  2451,   152,  1570,   423,  1990,  1093,   233,  2244,
       3,   242,  2469,  2470,     4,  1572,     5,  2492,  1387,  2113,
    2284,  2285,  1574,     3,  1601,  1093,  1093,     4,  1989,     5,
    1758,  1759,  1582,   233,  1602,  1603,     6,     7,     8,     9,
    1604,  2517,   915,  1606,   423,   153,  1605,   154,  1613,     6,
       7,     8,     9,  1614,  1621,   243,  1622,   235,  1669,   237,
    1623,     3,  1625,  2132,  1630,     4,   589,     5,   590,   591,
     592,   593,  1633,  2055,  1634,  2079,  1635,  1636,  2222,  2223,
    2224,  2225,  2226,  1637,  2148,   234,  1645,     6,     7,     8,
       9,  1650,   238,  2080,  1668,   946,   947,  1048,  1676,   234,
      18,    19,  1682,  1853,  2081,  1711,  1854,  1686,   235,  1721,
     237,   239,  1723,  1736,  2402,  1739,  -612,  1745,  1753,  1341,
    1754,  1755,   241,  -759,   234,  1195,  1196,  1197,  1198,  2049,
    1770,   915,  -760,   242,  -463,  1787,   229,  1866,  2061,  2062,
    2063,  2064,  2065,   238,  1867,  2286,  1868,  1879,   233,  1869,
    1886,    35,  1935,  1895,  1907,  2091,  2092,  2093,  2094,  1887,
     229,    36,   239,  1948,  1387,   233,  1970,    37,  1955,  2102,
    2103,  2104,   229,   241,  1976,   915,   903,   243,  1983,  1992,
    1808,  1817,  1834,  1848,   242,  2000,   233,  2393,  2001,  2002,
    1924,  2003,    38,  2007,  1817,  2010,  2149,   423,  2012,   233,
    1501,  2400,  1011,  2025,  2037,  1809,  1818,  1835,  1849,  2401,
     423,  2060,  2050,  2071,     3,  1925,  2107,  2075,     4,  1818,
       5,  2099,  2133,  2140,  2150,  2157,  2105,  2106,   243,  1855,
     915,   888,   888,  2121,  2123,  2030,  2032,  2032,  2035,   234,
       6,     7,     8,     9,  2125,  2455,  2176,  2134,  2141,  2151,
    2158,  1790,  2301,  2302,  2303,  2304,   234,   595,   596,   235,
     230,   237,  1589,  1590,  1591,  1592,   597,   598,   599,   600,
     601,  2185,   244,   235,  2508,   237,     3,   234,  2505,   233,
       4,  2399,     5,  2161,   230,  1594,  1595,  1596,  1597,  1598,
     234,  2163,  2166,  2167,   238,  2491,   230,  2170,   235,  2194,
     237,  2497,     6,     7,     8,     9,  2171,  1341,   238,  2191,
    2229,  2196,  2202,   239,  2206,   617,  2210,   618,  1853,  2217,
    2259,  1854,  2232,  2237,   241,  2247,   229,  1364,  2257,  1320,
     229,   441,  2265,   238,  2266,   242,  2269,   619,  1365,  1092,
     233,  2499,  2271,  2209,   442,   620,   621,   622,   623,   242,
    2274,  2351,   239,  2291,  2281,   946,   947,  2363,  2315,   441,
    2263,  2506,  2282,   241,  2283,  1856,  2229,  2295,  2296,  2507,
     234,  2297,   442,  2298,   242,  2299,  2305,  2272,  2372,   243,
    2373,  2392,  2306,  1545,  2275,  2307,  2308,  2309,  2233,  2234,
    2353,   915,  2381,   243,   244,  2382,  2383,  1548,  2384,  1549,
    2404,  -304,  2408,  2457,   915,  2412,  2458,  2459,  2240,  2463,
     887,   887,  2446,   235,  2465,   237,  2242,  2243,   243,  2467,
    2246,  2489,  2498,  2468,  2471,  1908,  2250,  2511,  1483,  2516,
     235,   234,   237,  2133,  2140,  2150,  2157,  1422,  1485,  1299,
    2504,  1484,  1300,  1298,  1855,   427,  1297,  1093,   238,  1958,
     230,   235,  1557,   237,   230,  1189,  1487,  1880,  2134,  2141,
    2151,  2158,  1185,  2120,   235,   238,   237,   239,    73,   229,
    1092,  1092,  1092,  1092,  1092,   843,   543,  1092,   241,   844,
    1116,  2300,  1861,  2101,   239,  2318,   238,  2313,  2335,   242,
    2449,  2317,  2385,  2388,  2376,   241,  2387,  2389,  2490,   238,
    2034,   851,  2208,  1092,  1113,   239,   242,  2344,   233,  1386,
    1530,  1410,  2192,   948,  1928,  2181,   241,  1641,   239,   423,
    2177,  1929,  1092,  2178,  2193,  1077,  1457,   242,   233,   241,
    1619,  1620,  1115,   243,  2294,  2375,  1077,  1077,  1742,  1744,
     242,  1743,  1761,  1465,   235,  1964,   237,  2314,  1631,  1967,
     243,  1366,  1764,   229,   232,  1552,   233,   885,   822,  2137,
    2138,  2365,  2366,  1783,  1708,   233,  2184,  2345,  2346,  1957,
     229,   243,  2367,  2368,  1986,  1987,  1104,  1130,  1127,   238,
    1856,  2356,  2357,  2495,   243,  1131,   894,  2358,  2359,  2360,
    2361,  2362,  1129,   230,  2364,   915,   427,     3,   239,   234,
    1128,     4,   898,     5,  1265,   235,   899,   237,  2411,   241,
     244,  1237,  1231,   603,   604,  1600,  2252,   605,     0,   234,
     242,     0,     0,     6,     7,     8,     9,     0,   606,   607,
     608,   609,   610,   611,   612,   427,   613,   614,     0,     0,
     238,     0,     0,     0,   233,     0,     0,   234,  2403,  1079,
    2313,  2405,     0,     0,  2406,     0,   234,     0,     0,   239,
    1079,  1079,     0,     0,   243,     0,     0,     0,     0,     0,
     241,     0,     0,     0,     0,     0,     0,   230,  2128,   615,
       0,   242,     0,     0,     0,     0,     0,     0,  2418,  2423,
    2431,     0,     0,     0,   230,  1791,  1817,  1834,  1848,     0,
       0,   233,     0,     0,  1909,  1924,  1931,  1936,  1942,     0,
    2488,     0,   423,  2419,  2424,  2432,  2426,     0,     0,     0,
    1792,  1818,  1835,  1849,     0,   243,  2456,  2427,   233,  1910,
    1925,  1932,  1937,  1943,  2462,   234,     0,     0,     0,  2464,
       0,     0,     3,     0,  2466,     0,     4,     0,     5,  2229,
     244,     0,  2472,  2473,  2474,  2475,  2476,  2477,  2133,  2140,
    2150,  2157,  2482,  2483,  2484,  2485,  2486,   244,     6,     7,
       8,     9,     0,   235,     0,   237,     0,     0,     0,     0,
    2496,     0,     0,  2134,  2141,  2151,  2158,  1877,   427,     0,
       0,  1878,   234,   235,     0,   237,     0,  2500,  2501,  2502,
    2503,   427,     0,     0,     0,     0,     0,     3,   238,     0,
       0,     4,     0,     5,     0,     0,     3,     0,     0,   234,
       4,   235,     5,   237,     0,     0,     0,   239,   238,     0,
     235,     0,   237,     6,     7,     8,     9,     0,   241,  2129,
       0,     0,     6,     7,     8,     9,  1105,   239,     0,   242,
       0,     0,     0,     0,     0,     0,   238,     0,   241,   630,
     631,     0,     0,     0,     0,   238,     0,     0,     0,   242,
       0,   505,     0,  1902,   632,   239,   633,   634,   635,     0,
    2130,   244,     0,     0,   239,     0,   241,    18,    19,     0,
       0,  2428,     0,   243,     0,   241,     0,   242,     0,     0,
     636,   637,   638,     0,     0,     0,   242,     0,     0,   235,
       0,   237,     0,   243,  1959,  1960,  1961,     0,  1963,     0,
    1704,  1092,  1092,  1092,  1092,  1092,  1092,  1092,  1092,  1092,
    1092,  1092,  -463,     3,     0,     0,  1977,     4,    35,     5,
       0,   243,   244,     0,   238,     0,     0,     0,    36,     0,
     243,     0,     0,     0,    37,     0,     0,     0,     0,     6,
       7,     8,     9,   239,     0,     0,   235,     0,   237,     0,
       0,     0,     0,     0,   241,  1853,     0,  2429,  1854,    38,
       3,     0,     0,     0,     4,   242,     5,     0,     0,     0,
       0,   229,     0,   235,     0,   237,     0,     0,     3,  2430,
       0,   238,     4,     0,     5,     0,     6,     7,     8,     9,
       0,   229,     0,     0,     0,  1336,     0,  1338,  1077,     0,
     239,  1824,     0,     0,     6,     7,     8,     9,   238,   243,
       0,   241,  2131,  1353,  1354,  1355,  1795,  1356,     0,     0,
    1367,  1368,   242,     0,     0,     0,  1796,   239,     0,     0,
       0,  1797,     0,  1798,     0,  1799,     0,     0,   241,     0,
       0,  1793,     0,  1811,  1822,  1838,  1851,     0,     0,   242,
       0,     0,  1825,  1826,  1827,  1828,     0,  1829,     0,     0,
    1830,     0,  1436,     0,     0,     0,   243,     0,  1420,     0,
       0,  1424,     0,     0,     0,     0,     0,  1831,     3,     0,
       0,  1855,     4,     0,     5,     0,     0,     0,     0,     0,
     427,     0,     0,   243,     0,   230,  1092,     0,     0,     0,
       0,     0,     0,   915,     6,     7,     8,     9,     0,     0,
     244,     0,     0,     0,  1092,  1361,  -463,     0,     0,     0,
       0,     0,  1079,     0,  2251,     0,     0,     0,  1832,  1077,
    1077,  1077,  1077,  1077,   229,     0,  1077,     0,   244,  1461,
       0,  2137,  2138,     0,     0,     0,  1800,   244,  1400,  1401,
    1402,  1403,  1404,  1405,  1406,  1407,  1408,     0,     0,     0,
       0,     0,  1077,  1911,     0,  1926,     0,  1933,   915,  1938,
       0,     0,  1944,     0,  1092,     0,     3,   915,     0,     0,
       4,  1077,     5,  2186,     0,     0,     0,     0,     0,  2434,
       0,     0,  1092,  1092,     0,     3,     0,  1913,  2479,     4,
       0,     5,     6,     7,     8,     9,     0,  1914,     0,  1508,
    1509,     0,     0,     0,  1915,  1916,     0,  1856,     0,     0,
       0,     6,     7,     8,     9,     0,   244,     0,     0,     0,
    1704,     0,  1917,     0,     0,     0,  1824,     0,     0,     0,
       0,     0,  1464,     0,     0,     0,     0,     0,     0,  1691,
    1918,  1919,     0,  1079,  1079,  1079,  1079,  1079,   230,     0,
    1079,     0,     0,     0,  1692,     0,     0,  1801,     0,  2144,
       0,   345,     0,     0,     0,     0,  2145,     0,     0,     0,
    1920,     0,  1802,   427,  1921,     0,  1079,  1825,  1826,  1827,
    1828,  1803,  1829,     0,     0,  1830,     0,   674,     0,     0,
       0,     0,  1693,     0,     0,  1079,   675,  1694,     0,     0,
     244,     0,  1831,     0,  2146,  2438,     0,     3,     0,     0,
    1804,     4,   676,     5,     0,  1805,   677,  1806,     0,     0,
     678,   679,     0,     0,     0,   680,   681,   682,   683,   684,
       0,   915,     0,     6,     7,     8,     9,     0,     0,  2147,
       0,     0,     0,  2264,     0,     0,     0,     0,  1824,   915,
    2267,     0,  1833,  1832,     0,     0,     0,     0,     0,     0,
    2273,     0,     0,     0,     0,     0,     0,  2276,     3,     0,
    2109,     0,     4,     0,     5,     0,  1811,  1822,  1838,  1851,
       0,     0,  1922,     0,     0,     3,  1926,     0,     0,     4,
    1822,     5,     0,     0,     6,     7,     8,     9,     0,  1825,
    1826,  1827,  1828,     0,  1829,     0,     0,  1830,     0,     0,
       0,     6,     7,     8,     9,  1281,  1282,     0,  2135,  2142,
    2152,  2159,   229,     0,  1831,     0,     0,     0,     0,  1283,
       0,     3,     0,     0,     0,     4,  1649,     5,  1284,     0,
    1652,     0,     0,     0,     0,     0,     0,     0,     0,   915,
    1672,  1673,  1674,  1675,  1285,  1677,  2148,     6,     7,     8,
       9,  2144,  2347,  2348,  2349,  2350,     0,     0,  2145,     0,
    2444,     0,  1687,     0,  1691,  1832,     3,     0,     0,     0,
       4,     0,     5,     0,     3,     0,     0,     0,     4,  1692,
       5,     0,   163,   164,   165,     0,   345,     0,     0,     0,
       0,     0,     6,     7,     8,     9,  2146,     0,     0,     0,
       6,     7,     8,     9,  1436,  1436,  1436,  1436,  1436,     0,
    1436,  1436,     0,     0,     0,     0,  1735,  1693,  1737,  1756,
    1756,     0,  1694,     0,     0,  1738,     0,     0,     0,     0,
       0,  2147,     0,     0,   229,     0,   230,   915,  2154,     0,
    1751,     0,  1752,     0,     0,     0,     3,     0,     0,     0,
       4,  1760,     5,  1762,     0,     0,   915,     0,  2253,     0,
    1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,  1077,
    1077,  2129,     6,     7,     8,     9,     0,  2111,     0,  1660,
    1286,  1287,  1288,   798,     0,   229,  1691,     0,     0,     3,
       0,     0,     0,     4,  1092,     5,   808,   809,     3,  2453,
     810,  1692,     4,     0,     5,     0,  1729,  1729,   345,  1729,
       0,     0,  2130,     0,     0,     6,     7,     8,     9,  2135,
    2142,  2152,  2159,     0,     6,     7,     8,     9,     0,  1213,
     640,   641,     0,   642,     0,     0,     0,     0,     0,  1693,
       0,     0,     0,     3,  1694,  1214,  1215,     4,  2148,     5,
       0,     0,     0,   643,     0,     0,     0,     0,   230,     0,
       0,   644,   645,  2154,   646,     0,     0,     0,     0,     6,
       7,     8,     9,     0,   647,   648,   649,     0,   915,     0,
       0,     0,     0,     0,  1079,  1079,  1079,  1079,  1079,  1079,
    1079,  1079,  1079,  1079,  1079,     0,     0,     0,     0,  2436,
       0,  2512,  2513,  2514,  2515,     0,     0,     0,     0,   230,
       0,  2155,  1215,     0,     0,  2154,     0,     0,     0,  1661,
       0,     0,  1217,  1218,  1662,  1220,  1221,  1222,  1223,  1224,
    1225,  1226,  1227,  1228,  1663,     0,   229,     0,     0,   915,
       0,     0,     0,  1946,  1947,     0,     0,     0,     0,     0,
     229,    92,     0,     0,     0,  1077,   915,     0,     0,    93,
    2480,    94,     0,    95,  2131,     0,    96,    97,    98,  1962,
      99,     0,     0,  1077,     0,   229,     0,  2447,     0,  1969,
       0,     0,     0,     0,     0,  1216,     0,     0,  1217,  1218,
    1219,  1220,  1221,  1222,  1223,  1224,  1225,  1226,  1227,  1228,
    1229,     0,   915,     0,     0,     0,     0,     0,     0,  1324,
       0,     0,     0,     0,     0,     0,     3,     0,     0,     0,
       4,     0,     5,  2156,     0,     0,     0,     0,     0,  2009,
       0,     0,     0,  1077,     0,     0,  2155,     0,     0,     0,
       0,     0,     6,     7,     8,     9,  2011,   915,  2013,     0,
       0,  1077,  1077,     0,     0,   915,     0,  1841,     0,     0,
     230,     0,     0,     0,     0,     0,     0,     0,  1930,     0,
       0,     0,  1966,     0,  1361,  1464,  2478,     0,     0,  1079,
       0,  1793,  1822,  1838,  1851,     0,     0,     0,  2155,     0,
    1911,  1926,  1933,  1938,  1944,  1320,     0,  1079,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1842,
    1843,  1844,     0,  1829,     0,     0,  1830,  1729,     0,     0,
    1729,     0,     0,  1729,     0,     3,     0,   915,   229,     4,
       0,     5,     0,  1845,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,  2135,  2142,  2152,  2159,  2254,     0,
       0,     6,     7,     8,     9,     0,     0,  1079,     0,     0,
       0,     0,     0,     0,     0,     0,  1841,     0,     0,     0,
     915,     0,     0,     0,     0,  1079,  1079,     0,     0,   915,
       0,     0,     0,     0,  1846,     0,  1324,     0,     0,     0,
       0,  2443,     0,     0,    52,     0,     0,     0,     0,     0,
    2481,     0,   100,     0,    64,    52,    52,    67,    67,    67,
      71,     0,     0,    52,     0,     0,     0,   101,  1842,  1843,
    1844,   229,  1829,     0,   915,  1830,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  2183,  1845,     0,     0,  2445,     0,     0,     0,     0,
       0,     0,   230,     0,     0,   103,     0,     0,     0,     0,
     104,     0,   105,  2189,     0,   230,     0,    52,     0,     0,
     449,    52,    67,    67,    67,    71,     0,     0,     0,     0,
       0,     0,   229,    52,     0,   466,   471,   474,    52,   479,
      67,   479,   484,  1846,   466,   466,   466,   466,   466,     0,
      67,     0,     0,  2201,   502,  1691,    52,   509,   509,    67,
      67,   509,    71,     0,     0,    67,     0,   509,     0,     0,
    1692,   509,   509,   509,   509,   509,     0,   345,     0,   509,
       0,    52,   544,    67,    67,    67,     0,   509,   509,     0,
       0,   555,    52,    67,    67,   230,  1240,    67,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,  1693,     0,
       0,     0,     0,  1694,     0,     0,     0,     0,     0,  1241,
    1242,  1243,  1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,
    1252,  1253,  1254,  1255,  1256,  1257,     0,     0,     0,     0,
       0,     0,     0,   286,     0,     0,     0,   915,     0,     0,
       0,   287,     0,   288,   166,   289,   230,     0,   290,   291,
     292,     0,   293,     0,  1691,     0,     0,     0,  1847,     0,
       0,   170,     0,     0,     0,     0,     0,     0,     0,  1692,
    2277,  2278,     0,     0,  2279,  2280,   345,    10,   171,     0,
       0,     0,     0,   172,     0,    11,   173,    12,     0,    13,
     229,   174,    14,    15,    16,   175,    17,     0,   176,   177,
       0,    18,    19,   178,     0,     0,   179,  1693,   180,     0,
     229,     0,  1694,  1077,     0,     0,     0,     0,     0,   466,
     471,   474,    52,   479,    67,    67,   484,     0,   466,   466,
     466,   466,   466,   181,    76,  2290,     0,     0,     0,   186,
       0,     0,    77,     0,    78,     0,   915,     0,     0,    79,
      80,    81,    35,    82,     0,     0,     0,     0,     0,     3,
       0,     0,    36,     4,     0,     5,     0,  2112,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   158,     0,     0,     6,     7,     8,     9,   159,
       0,   160,     0,    38,     0,   327,   161,   328,     0,     0,
       0,     0,     0,     0,  1812,     0,     0,     0,     0,     0,
       0,     0,   329,     0,   230,     0,     0,     0,     0,     0,
       0,     0,     0,   330,   331,     0,     0,     0,   332,     0,
       0,   333,     0,     0,   230,     0,     0,  1079,     0,     0,
       0,   334,   335,   336,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1813,     0,     0,     0,
     340,     0,   341,     0,     0,   162,   163,   164,   165,     0,
       0,   166,     0,   167,     3,     0,  1814,   168,     4,     0,
       5,     0,   169,   229,   294,     0,     0,     0,   170,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   295,
       6,     7,     8,     9,     0,   171,     0,     0,   296,     0,
     172,     0,     0,   173,     0,  1841,     0,     0,   174,     0,
       0,   466,   175,     0,     0,   176,   177,  1815,    39,     0,
     178,     0,     0,   179,     0,   180,     0,   297,  1153,     0,
       0,     0,   298,    40,   299,     0,     0,     0,     0,     0,
      67,     0,    41,     0,     0,     0,     0,     0,   509,     0,
     181,   182,     0,   183,   184,   185,   186,  1842,  1843,  1844,
       0,  1829,     0,     0,  1830,    52,     0,     0,     0,     0,
       0,    42,     0,     0,     0,    83,    43,     0,    44,     0,
       0,  1845,     0,    67,     0,     0,     0,     0,     0,     0,
      84,   187,   188,   189,   190,     0,     0,   230,     0,    85,
     191,   192,     0,     0,   193,   194,   342,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,     0,     0,    86,     0,
       0,     0,  1846,    87,     0,    88,     0,     0,   343,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   344,     0,     0,     0,     0,     0,     0,
     345,     0,   346,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   348,     0,     0,     0,     0,   349,     0,     0,     0,
       0,     0,   217,   218,     0,   350,     0,     0,   351,   352,
     353,   354,     0,     0,     0,   355,     0,     0,     0,   356,
     357,   219,     0,     0,     0,   220,   221,     0,     0,     0,
     915,     0,     0,     0,   358,     3,     0,   222,   223,     4,
       0,     5,     0,   359,     0,     0,     0,     0,     0,     0,
     360,  1816,   224,  1691,     0,   225,     0,     0,   158,     0,
       0,     6,     7,     8,     9,   159,     0,   160,  1692,     0,
       0,   327,   161,   328,     0,   345,     0,     0,   790,   791,
    1812,     0,   792,     0,     0,     0,     0,     0,   329,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   330,
     331,   793,     0,     0,   332,     0,  1693,   333,     0,     0,
       0,  1694,     0,     0,     0,     0,     0,   334,   335,   336,
     337,   338,   339,     0,     0,     0,   956,     0,     0,     0,
       0,     0,  1813,   957,     0,     0,   340,     0,   341,     0,
       0,   162,   163,   164,   165,   915,     0,   166,     0,   167,
       0,     0,  1814,   168,     0,     0,     0,     0,   169,     0,
       0,   163,   164,   165,   170,     0,  2437,     0,  1058,     0,
       0,     0,  1059,     0,     0,   466,     0,  1060,     0,     0,
       0,   171,     0,  1061,     0,     0,   172,   964,     0,   173,
       0,     0,     0,     0,   174,     0,     0,     0,   175,     0,
       0,   176,   177,  1815,     0,     0,   178,     0,  1064,   179,
       0,   180,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   794,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,   182,     0,   183,
     184,   185,   186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   795,   796,   797,   798,   799,   800,     0,
     801,   802,   803,   804,   805,   806,   807,     0,     0,   808,
     809,     0,     0,   810,     0,     0,     0,   187,   188,   189,
     190,     0,     0,     0,     0,     0,   191,   192,     0,     0,
     193,   194,   342,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   811,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   343,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,     0,     0,     0,     0,     0,   345,     0,   346,     0,
       0,     0,     0,   967,     0,     0,   968,   969,     0,     0,
       0,     0,   347,     0,     0,   970,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   348,     0,     0,
       0,     0,   349,     0,     0,     0,     0,     0,   217,   218,
       0,   350,     0,     0,   351,   352,   353,   354,     0,     0,
       0,   355,     0,     0,     0,   356,   357,   219,     0,     0,
       0,   220,   221,   972,   973,     0,   915,     0,     0,     0,
     358,     3,     0,   222,   223,     4,     0,     5,     0,   359,
       0,     0,     0,     0,     0,     0,   360,  2110,   224,     0,
       0,   225,     0,     0,   158,     0,     0,     6,     7,     8,
       9,   159,     0,   160,     0,     0,     0,   327,   161,   328,
     225,     0,     0,     0,     0,     0,  1812,     0,     0,     0,
       0,     0,     0,     0,   329,     0,     0,     0,     0,     0,
       0,     0,  1550,     0,     0,   330,   331,     0,     0,     0,
     332,     0,     0,   333,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   334,   335,   336,   337,   338,   339,   166,
       0,   167,     0,     0,     0,   168,     0,     0,  1813,     0,
     169,     0,   340,     0,   341,     0,   170,   162,   163,   164,
     165,     0,     0,   166,     0,   167,     0,     0,  1814,   168,
       0,     0,     0,   171,   169,     0,     0,     0,   172,     0,
     170,   173,     0,     0,     0,     0,   174,     0,     0,     0,
     175,     0,     0,   176,   177,   691,     0,   171,   178,     0,
       0,   179,   172,   180,     0,   173,     0,     0,     0,     0,
     174,     0,     0,     0,   175,     0,     0,   176,   177,  1815,
       0,     0,   178,     0,     0,   179,     0,   180,   181,     0,
     692,     0,     0,     0,   186,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   181,   182,     0,   183,   184,   185,   186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   187,   188,   189,   190,     0,     0,     0,
       0,     0,   191,   192,     0,     0,   193,   194,   342,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,     0,     0,
       0,     0,     0,     0,     0,   166,     0,   167,     0,     0,
     343,   168,     0,     0,     0,     0,   169,     0,     0,     0,
       0,     0,   170,     0,     0,   344,     0,     0,     0,     0,
       0,     0,   345,     0,   346,     0,     0,     0,     0,   171,
       0,     0,     0,     0,   172,     0,     0,   173,   347,     0,
       0,     0,   174,     0,     0,     0,   175,     0,     0,   176,
     177,     0,     0,   348,   178,     0,     0,   179,   349,   180,
       0,     0,     0,     0,   217,   218,     0,   350,     0,     0,
     351,   352,   353,   354,     0,     0,     0,   355,     0,     0,
       0,   356,   357,   219,   181,     0,     0,   220,   221,     0,
     186,     0,   915,     0,     0,     0,   358,     3,     0,   222,
     223,     4,     0,     5,     0,   359,     0,     0,     0,     0,
       0,     0,   360,  2119,   224,     0,     0,   225,     0,     0,
     158,     0,     0,     6,     7,     8,     9,   159,     0,   160,
       0,     0,     0,   327,   161,   328,     0,     0,     0,     0,
       0,     0,  1812,     0,     0,     0,     0,     0,     0,     0,
     329,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   330,   331,     0,     0,     0,   332,     0,     0,   333,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   334,
     335,   336,   337,   338,   339,   166,     0,   167,     0,     0,
     720,   168,     0,     0,  1813,     0,   169,     0,   340,     0,
     341,     0,   170,   162,   163,   164,   165,     0,     0,   166,
       0,   167,     0,     0,  1814,   168,     0,     0,     0,   171,
     169,     0,     0,     0,   172,     0,   170,   173,     0,     0,
       0,     0,   174,     0,     0,     0,   175,     0,     0,   176,
     177,     0,     0,   171,   178,     0,     0,   179,   172,   180,
       0,   173,     0,     0,     0,     0,   174,     0,     0,     0,
     175,     0,     0,   176,   177,  1815,     0,     0,   178,     0,
       0,   179,     0,   180,   181,     0,   721,     0,     0,     0,
     186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   181,   182,
       0,   183,   184,   185,   186,     0,     0,     0,     0,     0,
       0,   109,     0,     0,     0,     0,   110,     0,     0,   111,
     112,   113,   114,     0,     0,   115,   116,     0,   117,   118,
     119,     0,   120,     0,     0,     0,     0,     0,     0,   187,
     188,   189,   190,     0,     0,     0,     0,     0,   191,   192,
       0,     0,   193,   194,   342,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   121,     0,   122,   123,   124,     0,
       0,     0,     0,     0,     0,     0,   343,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,     0,     0,     0,     0,     0,     0,   345,     0,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   347,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   348,
       0,     0,     0,     0,   349,     0,     0,     0,     0,     0,
     217,   218,     0,   350,     0,     0,   351,   352,   353,   354,
       0,     0,     4,   355,     5,     0,     0,   356,   357,   219,
       0,     0,     0,   220,   221,     0,     0,     0,   915,     0,
       0,   158,   358,     0,     0,   222,   223,     0,   159,     0,
     160,   359,     0,     0,   327,   161,   328,     0,   360,  2435,
     224,     0,     0,   225,     0,     0,     0,     0,     0,     0,
       0,   329,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   330,   331,     0,     0,     0,   332,     0,     0,
     333,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     334,   335,   336,   337,   338,   339,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   340,
       0,   341,   125,     0,   162,   163,   164,   165,     0,     0,
     166,     0,   167,     0,     0,     0,   168,   126,     0,     0,
       0,   169,     0,     0,     0,     0,   127,   170,     0,     0,
       0,     0,     0,   128,   129,   130,   131,   132,     0,     0,
       0,     0,     0,     0,   171,     0,     0,     0,     0,   172,
       0,     0,   173,     0,     0,   133,     0,   174,     0,     0,
     134,   175,   135,   136,   176,   177,     0,     0,     0,   178,
       0,     0,   179,     0,   180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   181,
     182,     0,   183,   184,   185,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     187,   188,   189,   190,     0,     0,     0,     0,     0,   191,
     192,     0,     0,   193,   194,   342,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   343,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   344,     0,     0,   955,     0,     0,     0,   345,
       0,   346,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   158,     0,   347,     0,     0,     0,     0,
     159,     0,   160,     0,     0,     0,     0,   161,     0,     0,
     348,     0,     0,     0,     0,   349,     0,     0,     0,     0,
       0,   217,   218,     0,   350,     0,     0,   351,   352,   353,
     354,     0,     0,     0,   355,     0,     0,     0,   356,   357,
     219,     0,   956,     0,   220,   221,     0,     0,     0,   957,
       0,     0,     0,   358,     0,     0,   222,   223,   958,     0,
     959,     0,   359,     0,     0,     0,     0,     0,     0,   360,
    1988,   224,     0,     0,   225,     0,   162,   163,   164,   165,
       0,     0,   166,     0,   960,     0,     0,     0,   961,     0,
       0,     0,     0,   962,     0,     0,     0,     0,     0,   963,
       0,     0,     0,   964,     0,   895,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   171,     0,     0,     0,
       0,   172,     0,     0,   965,     0,     0,   790,   791,   174,
       0,   792,     0,   175,     0,     0,   176,   177,     0,     0,
       0,   178,     0,     0,   179,     0,   180,     0,     0,     0,
     793,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   181,   182,     0,   183,   184,   185,   186,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   896,     0,     0,
       0,     0,   966,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,  1333,     0,     0,     0,     0,     0,     0,     0,   967,
     794,     0,   968,   969,     0,     0,     0,     0,     0,   158,
       0,   970,     0,   216,     0,     0,   159,     0,   160,     0,
       0,     0,     0,   161,     0,     0,     0,     0,   971,     0,
       0,     0,   795,   796,   797,   798,   799,   800,   897,   801,
     802,   803,   804,   805,   806,   807,     0,     0,   808,   809,
       0,     0,   810,   217,   218,     0,     0,     0,   956,   972,
     973,     0,     0,     0,     0,   957,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,   220,   221,     0,     0,
       0,     0,     0,     0,     0,     0,   974,   975,     0,   786,
       0,   811,   162,   163,   164,   165,     0,     0,   166,     0,
     960,     0,     0,   976,   961,     0,   225,     0,     0,   962,
       0,     0,     0,  2395,     0,  1334,     0,     0,     0,   964,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   171,     0,     0,   790,   791,   172,     0,   792,
    1335,     0,     0,     0,     0,   174,     0,     0,     0,   175,
       0,     0,   176,   177,     0,     0,     0,   178,   793,     0,
     179,     0,   180,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   181,   182,     0,
     183,   184,   185,   186,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2396,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   966,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,  1423,   794,     0,
       0,     0,     0,     0,     0,   967,     0,     0,   968,   969,
       0,     0,     0,     0,     0,   158,     0,   970,     0,   216,
       0,     0,   159,     0,   160,     0,     0,     0,     0,   161,
     795,   796,   797,   798,   799,   800,  2397,   801,   802,   803,
     804,   805,   806,   807,     0,     0,   808,   809,     0,     0,
     810,     0,     0,     0,     0,     0,     0,     0,     0,   217,
     218,     0,     0,     0,   956,   972,   973,     0,     0,     0,
       0,   957,     0,     0,     0,     0,     0,     0,   219,     0,
       0,     0,   220,   221,     0,     0,     0,     0,     0,   811,
       0,     0,   974,   975,     0,   786,     0,     0,   162,   163,
     164,   165,     0,     0,   166,     0,   960,     0,     0,   976,
     961,     0,   225,     0,     0,   962,     0,     0,     0,     0,
       0,  1334,     0,     0,     0,   964,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   171,     0,
       0,     0,     0,   172,     0,     0,  1335,     0,     0,     0,
       0,   174,     0,     0,     0,   175,     0,     0,   176,   177,
       0,     0,     0,   178,     0,     0,   179,     0,   180,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   181,   182,     0,   183,   184,   185,   186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   966,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,  1460,     0,     0,     0,     0,     0,     0,
       0,   967,     0,     0,   968,   969,     0,     0,     0,     0,
       0,   158,     0,   970,     0,   216,     0,     0,   159,     0,
     160,     0,     0,     0,     0,   161,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,   218,     0,     0,     0,
     956,   972,   973,     0,     0,     0,     0,   957,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,   974,   975,
       0,   786,     0,     0,   162,   163,   164,   165,     0,     0,
     166,     0,   960,     0,     0,   976,   961,     0,   225,     0,
       0,   962,     0,     0,     0,     0,     0,  1334,     0,     0,
       0,   964,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   171,     0,     0,     0,     0,   172,
       0,     0,  1335,     0,     0,     0,     0,   174,     0,     0,
       0,   175,     0,     0,   176,   177,     0,     0,     0,   178,
       0,     0,   179,     0,   180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   181,
     182,     0,   183,   184,   185,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     966,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,  1651,
       0,     0,     0,     0,     0,     0,     0,   967,     0,     0,
     968,   969,     0,     0,     0,     0,     0,   158,     0,   970,
       0,   216,     0,     0,   159,     0,   160,     0,     0,     0,
       0,   161,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,   218,     0,     0,     0,   956,   972,   973,     0,
       0,     0,     0,   957,     0,     0,     0,     0,     0,     0,
     219,     0,     0,     0,   220,   221,     0,     0,     0,     0,
       0,     0,     0,     0,   974,   975,     0,   786,     0,     0,
     162,   163,   164,   165,     0,     0,   166,     0,   960,     0,
       0,   976,   961,     0,   225,     0,     0,   962,     0,     0,
       0,     0,     0,  1334,     0,     0,     0,   964,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     171,     0,     0,     0,     0,   172,     0,     0,  1335,     0,
       0,     0,     0,   174,     0,     0,     0,   175,     0,     0,
     176,   177,     0,     0,     0,   178,     0,     0,   179,     0,
     180,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   181,   182,     0,   183,   184,
     185,   186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   966,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   967,     0,     0,   968,   969,     0,     0,
       0,     0,     0,     0,     0,   970,     0,   216,     0,  1051,
    1052,  1053,  1054,  1055,  1056,     0,     0,     0,     0,  1057,
       0,     0,     0,  1039,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   218,     0,
       0,     0,     0,   972,   973,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
     220,   221,   956,     0,     0,     0,     0,     0,     0,   957,
     974,   975,     0,   786,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1371,     0,     0,   976,     0,     0,
     225,     0,     0,     0,     0,     0,   162,   163,   164,   165,
       0,     0,     0,     0,  1058,     0,     0,     0,  1059,     0,
       0,     0,     0,  1060,     0,     0,     0,     0,     0,  1061,
       0,     0,     0,   964,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   956,  1062,     0,     0,     0,
       0,  1063,   957,     0,  1064,     0,     0,     0,     0,  1065,
       0,     0,     0,  1066,     0,     0,  1067,  1068,     0,     0,
       0,  1069,     0,     0,  1070,     0,  1071,     0,     0,   162,
     163,   164,   165,     0,     0,     0,     0,  1058,     0,     0,
       0,  1059,     0,     0,     0,     0,  1060,     0,     0,     0,
       0,  1072,  1061,     0,  1073,     0,   964,  1074,  1371,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1062,
       0,     0,     0,     0,  1063,     0,     0,  1064,     0,     0,
       0,     0,  1065,     0,     0,     0,  1066,     0,     0,  1067,
    1068,     0,     0,     0,  1069,     0,     0,  1070,     0,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   956,
       0,     0,     0,     0,     0,     0,   957,     0,     0,     0,
       0,     0,     0,     0,  1072,     0,     0,  1073,     0,     0,
    1074,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   162,   163,   164,   165,     0,     0,   967,
       0,  1058,   968,   969,     0,  1059,     0,     0,     0,     0,
    1060,   970,     0,     0,     0,     0,  1061,     0,     0,     0,
     964,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1062,     0,     0,     0,     0,  1063,     0,
       0,  1064,     0,     0,     0,     0,  1065,     0,     0,     0,
    1066,     0,     0,  1067,  1068,     0,     0,     0,  1069,   972,
     973,  1070,  1075,  1071,     0,     0,     0,     0,     0,     0,
       0,     0,   967,     0,     0,   968,   969,  1098,  1039,     0,
       0,     0,     0,     0,   970,     0,   974,   975,  1072,     0,
       0,  1073,     0,     0,  1074,     0,     0,     0,     0,     0,
       0,   360,     0,  1076,     0,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   956,     0,     0,
       0,     0,   972,   973,   957,  1075,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   974,
     975,   162,   163,   164,   165,     0,     0,     0,     0,  1058,
       0,     0,     0,  1059,   360,     0,  1076,     0,  1060,   225,
       0,     0,     0,     0,  1061,     0,   967,     0,   964,   968,
     969,     0,     0,     0,     0,     0,     0,     0,   970,     0,
       0,  1062,     0,     0,     0,     0,  1063,     0,     0,  1064,
       0,     0,     0,     0,  1065,     0,     0,     0,  1066,     0,
       0,  1067,  1068,     0,     0,     0,  1069,     0,     0,  1070,
       0,  1071,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   972,   973,     0,  1075,
       0,     0,     0,     0,     0,     0,  1072,     0,     0,  1073,
       0,     0,  1074,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   974,   975,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   158,     0,     0,     0,     0,
    1076,     0,   159,   225,   160,     0,     0,     0,   327,   161,
     328,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   329,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   330,   331,     0,     0,
       0,   332,     0,     0,   333,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   334,   335,   336,   337,   338,   339,
       0,     0,     0,     0,   967,     0,     0,   968,   969,     0,
       0,     0,     0,   340,     0,   341,   970,     0,   162,   163,
     164,   165,     0,     0,   166,     0,   167,     0,     0,     0,
     168,     0,     0,     0,     0,   169,     0,     0,     0,     0,
       0,   170,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   171,     0,
       0,     0,     0,   172,   972,   973,   173,  1075,     0,     0,
       0,   174,     0,     0,     0,   175,     0,     0,   176,   177,
       0,     0,     0,   178,     0,     0,   179,     0,   180,     0,
       0,   974,   975,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1099,     0,  1076,     0,
       0,   225,     0,   181,   182,     0,   183,   184,   185,   186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   187,   188,   189,   190,     0,     0,
       0,     0,     0,   191,   192,     0,     0,   193,   194,   342,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   343,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,     0,
       0,     0,     0,   345,     0,   346,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   158,     0,     0,   347,
       0,     0,     0,   159,     0,   160,     0,     0,     0,     0,
     161,     0,     0,     0,   348,     0,     0,     0,     0,   349,
       0,     0,     0,     0,     0,   217,   218,     0,   350,     0,
       0,   351,   352,   353,   354,     0,     0,     0,   355,     0,
       0,     0,   356,   357,   219,   956,     0,     0,   220,   221,
       0,     0,   957,     0,     0,     0,     0,   358,     0,     0,
     222,   223,     0,     0,     0,     0,   359,     0,     0,     0,
       0,     0,     0,   360,     0,   224,     0,     0,   225,   162,
     163,   164,   165,     0,     0,   166,     0,   960,     0,     0,
       0,   961,     0,     0,     0,     0,   962,     0,     0,     0,
       0,     0,  1334,     0,     0,     0,   964,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,   172,     0,     0,  1335,     0,     0,
       0,     0,   174,     0,     0,     0,   175,     0,     0,   176,
     177,     0,     0,     0,   178,     0,     0,   179,     0,   180,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,   182,     0,   183,   184,   185,
     186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   966,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
       0,     0,   158,     0,     0,     0,     0,     0,     0,   159,
       0,   160,   215,     0,     0,     0,   161,     0,     0,     0,
       0,     0,   967,     0,     0,   968,   969,     0,     0,     0,
       0,     0,     0,     0,   970,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   218,     0,     0,
       0,     0,   972,   973,     0,   162,   163,   164,   165,     0,
       0,   166,     0,   167,     0,   219,     0,   168,     0,   220,
     221,     0,   169,     0,     0,     0,     0,     0,   170,   974,
     975,     0,   786,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,   976,     0,     0,   225,
     172,     0,     0,   173,     0,     0,     0,     0,   174,     0,
       0,     0,   175,     0,     0,   176,   177,     0,     0,     0,
     178,     0,     0,   179,     0,   180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     181,   182,     0,   183,   184,   185,   186,     0,     0,     0,
       0,   158,     0,     0,     0,     0,     0,     0,   159,     0,
     160,     0,     0,     0,     0,   161,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   187,   188,   189,   190,     0,     0,     0,     0,     0,
     191,   192,     0,     0,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,   162,   163,   164,   165,     0,     0,
     166,     0,   167,     0,     0,     0,   168,     0,     0,     0,
       0,   169,   216,     0,     0,     0,     0,   170,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   171,     0,     0,     0,     0,   172,
       0,     0,   173,     0,     0,     0,     0,   174,     0,     0,
       0,   175,   217,   218,   176,   177,     0,     0,     0,   178,
       0,     0,   179,     0,   180,     0,     0,     0,     0,     0,
       0,   219,     0,     0,     0,   220,   221,     0,     0,     0,
       0,     0,     0,     0,     0,  1039,     0,   222,   223,   181,
     182,     0,   183,   184,   185,   186,     0,     0,     0,     0,
     360,     0,   224,     0,     0,   225,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     187,   188,   189,   190,   956,     0,     0,     0,     0,   191,
     192,   957,     0,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,     0,     0,     0,   162,   163,
     164,   165,     0,     0,     0,     0,  1058,   215,     0,     0,
    1059,     0,     0,     0,   771,  1060,     0,     0,   772,     0,
       0,  1061,     0,     0,     0,   964,     0,     0,     0,     0,
       0,   216,     0,     0,     0,   773,     0,     0,  1062,     0,
       0,     0,     0,  1063,     0,     0,  1064,     0,     0,     0,
       0,  1065,     0,     0,     0,  1066,     0,     0,  1067,  1068,
       0,     0,     0,  1069,     0,     0,  1070,     0,  1071,     0,
       0,   217,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     219,     0,     0,  1072,   220,   221,  1073,     0,     0,  1074,
       0,     0,     0,     0,     0,     0,   222,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,   158,     0,   225,     0,     0,     0,     0,   159,
       0,   160,     0,     0,     0,     0,   161,  2319,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   774,   775,   776,
     777,   778,   779,   780,   781,   782,   783,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  2320,
       0,   967,     0,     0,   968,   969,     0,     0,     0,     0,
       0,     0,     0,   970,     0,     0,  2321,     0,     0,     0,
       0,   166,     0,   167,     0,     0,     0,   168,     0,     0,
       0,     0,   169,     0,     0,     0,     0,     0,   170,   784,
       0,     0,     0,   217,   218,     0,     0,     0,     0,     0,
       0,     0,     0,   785,     0,   171,     0,     0,     0,     0,
     172,   972,   973,   173,  1075,     0,     0,     0,   174,     0,
       0,     0,   175,     0,     0,   176,   177,     0,     0,   786,
     178,     0,     0,   179,     0,   180,     0,     0,   974,   975,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   360,     0,  1076,     0,     0,   225,     0,
     181,   182,     0,   183,   184,   185,   186,     0,     0,     0,
       0,   158,     0,     0,     0,     0,     0,     0,   159,     0,
     160,     0,     0,     0,     0,   161,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   187,   188,   189,   190,     0,     0,     0,     0,     0,
     191,   192,     0,     0,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  2322,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     166,     0,   167,  2323,     0,     0,   168,     0,     0,     0,
       0,   169,   216,     0,     0,     0,     0,   170,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   171,     0,     0,     0,     0,   172,
       0,  2324,   173,     0,     0,     0,  2325,   174,  2326,     0,
       0,   175,   217,   218,   176,   177,     0,     0,     0,   178,
       0,     0,   179,     0,   180,  2327,     0,     0,     0,     0,
       0,   219,     0,     0,     0,   220,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,   223,   181,
     182,     0,   183,   184,   185,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     187,   188,   189,   190,     0,     0,     0,     0,     0,   191,
     192,     0,     0,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     2,     3,     0,     0,     0,     4,     0,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     6,     7,
       8,     9,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,    11,     0,
      12,     0,    13,     0,     0,    14,    15,    16,     0,    17,
       0,   217,   218,     0,    18,    19,    20,     0,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
     219,    33,    34,     0,   220,   221,     0,     0,     0,     0,
       0,     0,   956,     0,     0,     0,   222,   223,     0,   957,
       0,     0,   334,   335,   336,  1971,  1972,   339,     0,  -463,
       0,     0,     0,     0,     0,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,   162,   163,   164,   165,
       0,    37,     0,     0,  1058,     0,     0,     0,  1059,     0,
       0,     0,     0,  1060,     0,     0,     0,     0,     0,  1061,
       0,     0,     0,   964,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,   956,  1062,     0,     0,     0,
       0,  1063,   957,     0,  1064,     0,     0,     0,     0,  1065,
       0,     0,     0,  1066,     0,     0,  1067,  1068,     0,     0,
       0,  1069,     0,     0,  1070,     0,  1071,     0,     0,   162,
     163,   164,   165,     0,     0,     0,     0,  1058,     0,     0,
       0,  1059,     0,     0,     0,     0,  1060,     0,     0,     0,
       0,  1072,  1061,     0,  1073,     0,   964,  1074,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   956,  1062,
       0,     0,     0,     0,  1063,   957,     0,  1064,     0,     0,
       0,     0,  1065,     0,     0,     0,  1066,     0,     0,  1067,
    1068,     0,     0,     0,  1069,     0,     0,  1070,     0,  1071,
       0,     0,  1780,   163,   164,   165,     0,     0,     0,     0,
    1058,     0,     0,     0,  1059,     0,     0,     0,     0,  1060,
       0,    39,     0,     0,  1072,  1061,     0,  1073,     0,   964,
    1074,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,  1062,  -463,     0,    41,     0,  1063,     0,   967,
    1064,     0,   968,   969,     0,  1065,     0,     0,     0,  1066,
       0,   970,  1067,  1068,     0,     0,     0,  1069,     0,     0,
    1070,     0,  1071,     0,    42,     0,     0,     0,     0,    43,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1072,     0,     0,
    1073,     0,     0,  1074,     0,     0,     0,     0,     0,   972,
     973,     0,  1075,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   967,     0,     0,   968,   969,     0,     0,     0,
       0,     0,     0,     0,   970,     0,   974,   975,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1076,     0,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   972,   973,     0,  1075,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   967,     0,     0,   968,   969,
       0,     0,     0,     0,     0,     0,     0,   970,     0,   974,
     975,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1076,     0,     0,   225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   972,   973,     0,  1075,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   974,   975,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1076,
       0,     0,   225
};

static const yytype_int16 yycheck[] =
{
       1,    37,     3,   220,   363,   530,   538,   451,   374,    69,
     338,    38,   912,   465,   706,   453,   400,   739,   380,   110,
    1069,   112,    69,   114,   115,     1,   327,   118,   419,   120,
     184,    54,   123,   124,    30,   956,   454,   128,   129,   130,
     131,   132,   452,    66,  1521,   136,   498,    48,   139,   407,
    1521,   494,   650,   144,   145,    56,   357,   588,    54,   495,
      61,   152,   419,  1520,  1521,    32,   224,   485,   905,    54,
      66,  1534,   482,   430,   431,  1415,  1468,  1526,  1528,  1527,
    1525,    66,   434,   878,   450,    36,   970,   497,  1372,  1373,
     419,  1467,   393,   536,  1544,  1379,  1541,   459,  1074,  1655,
    1656,   537,   397,  1032,   399,  1374,  1359,  1869,   589,   475,
     182,   182,  1824,   185,   185,   910,   185,   553,  1866,   976,
     915,   182,  1868,  1845,   185,    30,     7,     8,     9,  1841,
     492,  1767,  1689,  1045,   182,  1374,   617,   185,   548,   934,
    2074,  1043,  1044,  1374,   625,  1374,  1374,  1703,   182,    54,
     182,   185,  1841,   185,  1046,     5,   182,     5,  1867,   185,
     641,    66,  1047,   182,    40,     8,   185,   529,    24,    50,
      24,    52,     8,  2106,   182,   182,  1894,   185,   185,     6,
    1630,    62,     8,     8,  1658,     8,    67,  1417,   550,  1419,
      71,  1475,   182,   182,   107,   185,   185,  1621,  1622,   742,
    1049,   744,  1515,   746,   182,   686,   101,   185,   751,   110,
     195,   112,    59,   182,     1,  1342,   185,   874,     5,   120,
       7,    24,   123,   108,   220,  1700,     6,   182,   224,   877,
     185,    93,   182,   871,   872,   185,   182,    98,   139,   185,
      27,    28,    29,    30,  2232,   108,    83,    84,  2146,   730,
      60,   152,   784,    94,     8,   878,    97,   125,   108,   109,
     110,   876,    16,    17,    18,    19,    89,    60,  1072,   108,
    1826,  1827,    89,   108,   109,   110,   108,   337,   338,   811,
      62,   824,     9,   107,   765,     9,  1842,  1843,   109,  1766,
    1666,  1667,    16,    17,    18,    19,   307,    59,    25,    30,
     812,    30,   108,   109,   110,  1645,   402,   307,    13,  1766,
     277,   407,     9,    59,     9,   220,   283,    55,    94,   224,
       8,    97,   344,    54,   131,    54,   858,    49,    65,    56,
     855,    51,    56,    21,    30,    66,  1131,    66,   120,   419,
     130,   346,   347,   118,   225,    51,   108,   407,   118,    59,
     430,   431,   169,    30,    34,    35,    36,   342,    54,    56,
     135,    56,   108,   401,    30,   155,   103,   363,    59,    23,
      66,    89,   419,    91,    59,    30,   166,    54,   363,  1071,
     402,   115,  1839,   430,   431,    90,   898,  1864,    54,    66,
     842,   402,   452,  1864,   169,  1852,   130,   402,   108,    54,
      66,   131,   402,   173,   174,   452,  1863,  1864,   364,   139,
     149,    66,   182,   856,   306,   154,  2404,   108,   374,    30,
     401,   857,   482,   108,   447,   287,   294,   518,   313,   290,
    1234,  1700,   850,   132,   525,   482,  1705,   497,   848,   169,
      62,   464,  1569,    54,   101,   102,   103,   104,   105,   312,
     497,   447,   344,   107,   108,    66,   337,   338,   363,   158,
     341,  1700,   447,   290,  1062,  2363,  1705,  1065,   464,  1700,
    1127,  1700,  1700,   312,  1705,  1788,  1705,  1705,   305,   464,
     312,   402,  1130,  1957,  1122,   312,  1124,  1125,   548,   220,
    1339,   220,   854,   224,   407,   224,    21,  1727,   120,   306,
    1730,   548,     5,  1733,     7,  1143,   149,   402,  1131,     1,
     290,  1986,   402,     5,  1129,     7,   343,    14,    15,    16,
    2232,   348,   346,   166,   220,   305,   402,    89,   224,  2241,
     248,   318,   312,   380,   381,    27,    28,    29,    30,    89,
    2258,   382,   447,   220,   407,   368,   402,   224,  1343,  1344,
     312,   407,  2241,   407,   220,   403,  2489,   407,   224,   464,
     403,  2018,  2496,   343,   718,   220,   312,   403,   348,   224,
     407,  2207,   407,   383,   384,   385,   403,   403,   403,   259,
     616,    47,    79,    80,    81,   466,  1498,  1444,  2144,   470,
     383,   384,   385,   474,  1496,  1497,   402,  1446,   479,   402,
      97,   391,   312,   484,   407,   989,   382,  1499,  2320,   220,
     107,   329,   639,   224,  2336,  1500,   403,   518,   137,   116,
     117,   312,  2384,   403,  2016,   122,    21,   312,    23,   286,
      94,  2320,   363,  2381,   363,  1611,    30,  2383,   295,   296,
     297,   298,   299,   140,   141,   142,   717,   719,   719,  2206,
     719,   602,   406,   150,   151,  1107,   717,   154,   719,  1190,
      54,  1192,  2002,   544,   132,    89,     8,   363,   885,   717,
    1308,   719,    66,  2382,   116,   117,   118,   972,   973,    21,
      95,   562,   406,   717,   306,   719,   363,   719,   905,  1973,
     158,  1049,  2404,   719,  1175,   731,     6,   363,   717,     9,
     719,  2125,   107,   108,  1095,   115,   130,   734,   363,   717,
     717,   719,   719,   118,  1967,  1968,   447,  1986,   447,  1076,
     130,  1050,  1265,   138,    89,   248,  1418,   717,   717,   719,
     719,   155,   196,   464,   248,   464,  2020,   402,  1095,   717,
     394,   719,   407,   107,   108,    10,  2122,  1986,   717,   375,
     719,   447,   363,   407,   118,  1986,  1685,  1986,  1986,    89,
     384,    91,   717,  1115,   719,   130,  1095,   717,   464,   719,
     447,   717,    89,   719,    91,    40,    41,    42,    43,    44,
    1075,   447,    94,   308,   309,   310,     0,   464,   848,  1689,
     155,     5,   447,     7,   291,   292,   376,    12,   464,   379,
    1332,   848,   364,   365,   366,   116,   117,  1707,    21,   464,
      23,    76,  2204,  1697,   364,   365,   366,  1438,  1439,    84,
      85,    86,    87,    88,  1445,   115,   220,   319,   118,  1354,
     224,   488,   912,   297,   124,   299,   447,   107,   128,   129,
     130,   131,   132,  2306,   109,   905,   136,  1684,  2297,  2299,
    2298,  2296,  1647,   464,   144,   145,  2196,   151,    39,    74,
    1391,   126,   127,    78,   107,   912,   107,  1767,   133,   134,
     135,    40,    41,    94,   289,     5,    32,     7,   143,    94,
      95,   146,     8,  1326,   196,   118,  1338,   306,     1,   885,
    1048,   177,     5,  1329,     7,    21,   976,    23,   113,   132,
     147,   868,  2379,  2459,    79,    80,  1628,   140,  2379,   905,
      91,   403,  1355,   160,    27,    28,    29,    30,   248,  1073,
    1356,  2378,  2379,   138,   139,   158,   130,   963,   306,     1,
     177,   248,   165,     5,   149,     7,     9,   172,   965,   290,
     306,   156,   177,    16,    17,    18,    19,   151,   259,   260,
      23,   155,   263,   169,   305,    27,    28,    29,    30,   108,
    1322,   312,   118,  1043,  1044,  1045,  1046,  1047,    89,   363,
    1050,    43,  1032,  1665,    46,   196,   118,     5,    94,     7,
     885,  1339,     5,    56,     7,   297,     9,   299,     5,  1049,
     382,  1353,   343,    94,   288,   289,  1076,   348,   140,   329,
     905,  1302,     1,  1050,    82,  1537,     5,   120,     7,  2293,
       1,   153,   329,    41,     5,  1095,     7,   130,    47,    48,
     517,    99,  1714,   192,   137,   138,   195,   349,    27,    28,
      29,    30,   358,   249,   250,   251,    27,    28,    29,    30,
      39,   119,   155,   308,   309,   310,   311,    18,  1095,     8,
      49,     6,  1048,   447,     9,    54,    19,    56,     9,    58,
     173,   174,    21,   560,    23,    16,    17,    18,    19,   406,
     464,  1523,    23,   288,   289,    54,   297,  1461,   299,    23,
     196,  1562,    26,   402,   288,   289,  2007,     1,  1446,    54,
     203,     5,  1535,     7,   207,   196,   390,   169,   158,  1100,
    1536,   393,   394,  1104,  1495,    56,  2390,   263,   264,   169,
     170,  1502,   359,    27,    28,    29,    30,   108,   109,   110,
     169,   402,    65,    66,   305,     9,  1607,    70,   118,  1346,
     107,   108,    16,    17,    18,    19,   402,  1494,  1495,   402,
     130,   118,   132,  1048,  1501,  1502,  1503,  1504,   402,  1802,
    1803,  1804,  1805,  1806,   885,  1191,   885,  1193,  1039,    59,
     290,   151,   343,  1199,     8,   155,   307,   348,   158,   350,
     169,  1533,    56,   402,   905,   305,   905,    21,  1900,    23,
      45,   297,   312,   299,   173,   174,   390,    47,   330,   885,
     455,   456,   457,   458,    47,   852,   297,    47,   299,     8,
      51,    52,   315,    54,    55,    56,   120,   135,   885,   905,
       9,  2260,   140,   343,   132,   293,   130,   107,   348,   885,
     140,   307,   308,   137,   138,   333,   491,   375,   905,   408,
     885,   149,   408,   305,     5,   525,     7,     8,     1,   905,
     158,   155,     5,   375,     7,   173,   174,   165,   166,   107,
     905,  1468,   140,   173,   174,  1698,   176,   247,   523,   173,
     174,   392,   527,  1699,    27,    28,    29,    30,   405,   382,
      89,   120,   232,   402,   885,   182,   236,   237,   238,  1339,
     329,   330,   331,   332,   333,   173,   174,   136,    24,   203,
     403,   290,    24,   207,   905,   346,   347,     1,   288,   289,
     379,     5,    54,     7,    54,   118,   305,  2207,   306,   158,
      94,  1670,    54,    97,    54,   314,   165,  1048,   402,  1048,
      89,   209,   135,    27,    28,    29,    30,   542,   141,   142,
     107,   403,   108,   406,  1696,   111,   112,   113,  1790,    43,
     107,     1,    46,   107,   343,     5,   107,     7,   107,   348,
    1346,   350,  1048,   107,   242,   368,   369,   370,  1438,  1439,
    1440,  1441,  1442,   107,  1444,  1445,   107,    27,    28,    29,
      30,  1048,    89,  1453,  1454,    89,  1760,    18,  1762,   383,
     384,   385,  1048,   382,   346,   347,  1446,  2040,  2041,  2042,
    2043,   382,    19,  1048,  1785,  1786,    16,    17,  1923,  1446,
     390,   315,    94,   406,   403,   254,   255,   399,   400,  1941,
      16,    17,   403,    82,  1494,  1495,  1496,  1497,  1498,  1499,
    1500,  1501,  1502,  1503,  1504,   108,   109,   110,  1785,  1786,
       1,     6,  2227,   107,     5,   248,     7,  1048,    98,   107,
      48,  1346,    50,     8,    52,    53,   115,   352,   353,   354,
     355,   356,    60,    61,   305,   406,    27,    28,    29,    30,
       8,   130,    90,    91,    92,   169,    94,  1684,   382,     8,
     321,   322,  1468,     6,     1,   364,   365,   366,     5,   139,
       7,    83,    84,    85,    86,    87,    88,  1930,    96,   403,
       8,   885,   369,   370,  1856,    16,    17,   348,     8,  1935,
      27,    28,    29,    30,   196,     8,   290,   402,   177,   199,
     200,   905,   197,   198,  1515,  1516,    18,  1518,  1519,  1520,
    1521,   305,   406,  1519,  2005,  2006,   419,  2008,   312,     1,
     116,   117,  1568,     5,  1519,     7,   107,   430,   431,    19,
    1516,   107,  1518,  1519,  1520,  1521,  1908,   116,   117,    47,
     406,    49,   308,   309,   310,    27,    28,    29,    30,   343,
       5,     6,   243,  1468,   348,    63,    64,    94,    40,  2261,
       9,   252,   253,     8,   101,   256,   257,   107,  2059,  1615,
    1616,   116,   117,   346,   347,   107,   108,   362,   853,  2033,
    1617,  1618,    40,  2118,  2389,  2390,  2034,   266,   382,  2197,
    2198,   305,  2083,   108,   108,   297,   402,   299,   402,  1689,
     108,   402,   139,   402,  1519,  1346,   114,  1346,  2036,   382,
      99,   100,  1682,   402,  1684,  1685,   124,  1707,   307,   133,
     134,   291,   130,   344,   326,   327,   402,  1638,   107,  1640,
     403,  1642,   107,  1644,  1972,   107,  1647,   174,   116,   117,
    1346,   396,   397,   398,  1048,  2450,   402,   155,   116,   117,
    1707,  2071,  1638,   107,  1640,  2031,  1642,   107,  1644,  1346,
      47,  1647,    49,   402,  1670,  1711,   107,    54,   107,    56,
    1346,    45,    46,    60,   101,  1670,  1709,  1767,  1684,  2099,
       1,  1346,   316,   317,     5,   196,     7,   107,   108,   403,
    2177,  2178,   201,     1,   236,  1785,  1786,     5,  1709,     7,
    1453,  1454,   135,  1709,   239,   239,    27,    28,    29,    30,
     240,  2516,   382,   240,  1709,   102,   241,   104,   107,    27,
      28,    29,    30,   107,   107,  1346,   107,  1468,  1955,  1468,
     376,     1,   107,   403,   402,     5,   118,     7,   120,   121,
     122,   123,   402,  1813,   402,   259,   402,   402,  2044,  2045,
    2046,  2047,  2048,   107,   291,  1670,   107,    27,    28,    29,
      30,     6,  1468,   277,   107,   346,   347,     9,    54,  1684,
      63,    64,    23,    43,   288,   154,    46,    45,  1519,   345,
    1519,  1468,   345,     6,  2326,   405,   107,   107,    89,  2016,
       9,    89,  1468,    23,  1709,   143,   144,   145,   146,  1810,
     405,   382,    23,  1468,   312,   107,    30,   402,  1819,  1820,
    1821,  1822,  1823,  1519,   402,  2184,   402,     8,  1824,   402,
     409,   114,   403,   107,   308,  1836,  1837,  1838,  1839,   409,
      54,   124,  1519,     6,   108,  1841,    45,   130,   402,  1850,
    1851,  1852,    66,  1519,   196,   382,   344,  1468,   108,     8,
    1861,  1862,  1863,  1864,  1519,    21,  1862,  2319,   107,   107,
    1871,   349,   155,    73,  1875,     6,   403,  1862,   171,  1875,
       8,  2324,    41,   195,   108,  1861,  1862,  1863,  1864,  2325,
    1875,   103,   401,   401,     1,  1871,   103,   107,     5,  1875,
       7,   401,  1903,  1904,  1905,  1906,   108,   401,  1519,   169,
     382,  1971,  1972,   107,     8,  1796,  1797,  1798,  1799,  1824,
      27,    28,    29,    30,   107,  2406,  1927,  1903,  1904,  1905,
    1906,   403,  2218,  2219,  2220,  2221,  1841,   116,   117,  1670,
      30,  1670,   227,   228,   229,   230,   125,   126,   127,   128,
     129,   403,  1346,  1684,  2486,  1684,     1,  1862,  2483,  1955,
       5,  2323,     7,   107,    54,   231,   232,   233,   234,   235,
    1875,   107,   107,   107,  1670,  2456,    66,   107,  1709,   307,
    1709,  2462,    27,    28,    29,    30,   107,  2204,  1684,  1990,
    2050,   107,    89,  1670,     6,   118,   192,   120,    43,     8,
     401,    46,     9,     9,  1670,   156,   220,  1684,   107,   307,
     224,  2071,   107,  1709,   107,  1670,   107,   140,  1684,   912,
    2016,  2473,   107,  2024,  2071,   148,   149,   150,   151,  1684,
     109,     8,  1709,   403,   402,   346,   347,   139,    43,  2099,
    2131,  2484,   402,  1709,   402,   305,  2106,   402,   402,  2485,
    1955,   402,  2099,   402,  1709,   402,   402,  2148,   403,  1670,
     403,   108,   402,  1142,  2155,   402,   402,   402,  2069,  2070,
     402,   382,   402,  1684,  1468,   402,   402,  1156,   402,  1158,
       9,     9,   107,   107,   382,   108,   107,     6,  2089,   107,
    1971,  1972,   403,  1824,   107,  1824,  2097,  2098,  1709,   107,
    2101,   401,   320,   107,   107,   403,  2107,   107,  1066,   210,
    1841,  2016,  1841,  2114,  2115,  2116,  2117,   957,  1068,   827,
    2482,  1067,   828,   826,   169,  1519,   825,  2207,  1824,  1672,
     220,  1862,  1184,  1862,   224,   594,  1070,  1555,  2114,  2115,
    2116,  2117,   584,   403,  1875,  1841,  1875,  1824,    20,   363,
    1043,  1044,  1045,  1046,  1047,   287,   139,  1050,  1824,   288,
     449,  2217,  1524,  1846,  1841,  2239,  1862,  2227,  2245,  1824,
    2390,  2237,  2305,  2308,  2295,  1841,  2307,  2309,  2452,  1875,
    1798,   293,  2020,  1076,   446,  1862,  1841,  2247,  2184,   927,
    1123,   939,  1992,   388,  1641,  1940,  1862,  1324,  1875,  2184,
    1928,  1641,  1095,  1929,  2000,   419,  1014,  1862,  2204,  1875,
    1289,  1290,   448,  1824,  2209,  2294,   430,   431,  1440,  1442,
    1875,  1441,  1459,  1035,  1955,  1682,  1955,  2228,  1307,  1684,
    1841,   906,  1466,   447,    30,  1173,  2232,   329,   220,   346,
     347,  2277,  2278,  1504,  1379,  2241,  1955,  2248,  2249,  1671,
     464,  1862,  2279,  2280,  1705,  1705,   431,   462,   459,  1955,
     305,  2262,  2263,  2460,  1875,   463,   342,  2268,  2269,  2270,
    2271,  2272,   461,   363,  2275,   382,  1670,     1,  1955,  2184,
     460,     5,   343,     7,   740,  2016,   343,  2016,  2351,  1955,
    1684,   717,   684,   116,   117,  1243,   403,   120,    -1,  2204,
    1955,    -1,    -1,    27,    28,    29,    30,    -1,   131,   132,
     133,   134,   135,   136,   137,  1709,   139,   140,    -1,    -1,
    2016,    -1,    -1,    -1,  2320,    -1,    -1,  2232,  2329,   419,
    2390,  2332,    -1,    -1,  2335,    -1,  2241,    -1,    -1,  2016,
     430,   431,    -1,    -1,  1955,    -1,    -1,    -1,    -1,    -1,
    2016,    -1,    -1,    -1,    -1,    -1,    -1,   447,   403,   182,
      -1,  2016,    -1,    -1,    -1,    -1,    -1,    -1,  2369,  2370,
    2371,    -1,    -1,    -1,   464,  2376,  2377,  2378,  2379,    -1,
      -1,  2377,    -1,    -1,  2385,  2386,  2387,  2388,  2389,    -1,
    2450,    -1,  2377,  2369,  2370,  2371,   120,    -1,    -1,    -1,
    2376,  2377,  2378,  2379,    -1,  2016,  2407,   131,  2404,  2385,
    2386,  2387,  2388,  2389,  2415,  2320,    -1,    -1,    -1,  2420,
      -1,    -1,     1,    -1,  2425,    -1,     5,    -1,     7,  2489,
    1824,    -1,  2433,  2434,  2435,  2436,  2437,  2438,  2439,  2440,
    2441,  2442,  2443,  2444,  2445,  2446,  2447,  1841,    27,    28,
      29,    30,    -1,  2184,    -1,  2184,    -1,    -1,    -1,    -1,
    2461,    -1,    -1,  2439,  2440,  2441,  2442,  1546,  1862,    -1,
      -1,  1550,  2377,  2204,    -1,  2204,    -1,  2478,  2479,  2480,
    2481,  1875,    -1,    -1,    -1,    -1,    -1,     1,  2184,    -1,
      -1,     5,    -1,     7,    -1,    -1,     1,    -1,    -1,  2404,
       5,  2232,     7,  2232,    -1,    -1,    -1,  2184,  2204,    -1,
    2241,    -1,  2241,    27,    28,    29,    30,    -1,  2184,    98,
      -1,    -1,    27,    28,    29,    30,    40,  2204,    -1,  2184,
      -1,    -1,    -1,    -1,    -1,    -1,  2232,    -1,  2204,   116,
     117,    -1,    -1,    -1,    -1,  2241,    -1,    -1,    -1,  2204,
      -1,    47,    -1,  1632,   131,  2232,   133,   134,   135,    -1,
     139,  1955,    -1,    -1,  2241,    -1,  2232,    63,    64,    -1,
      -1,   295,    -1,  2184,    -1,  2241,    -1,  2232,    -1,    -1,
     157,   158,   159,    -1,    -1,    -1,  2241,    -1,    -1,  2320,
      -1,  2320,    -1,  2204,  1673,  1674,  1675,    -1,  1677,    -1,
    1375,  1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,
    1503,  1504,   108,     1,    -1,    -1,  1695,     5,   114,     7,
      -1,  2232,  2016,    -1,  2320,    -1,    -1,    -1,   124,    -1,
    2241,    -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,    27,
      28,    29,    30,  2320,    -1,    -1,  2377,    -1,  2377,    -1,
      -1,    -1,    -1,    -1,  2320,    43,    -1,   381,    46,   155,
       1,    -1,    -1,    -1,     5,  2320,     7,    -1,    -1,    -1,
      -1,   885,    -1,  2404,    -1,  2404,    -1,    -1,     1,   403,
      -1,  2377,     5,    -1,     7,    -1,    27,    28,    29,    30,
      -1,   905,    -1,    -1,    -1,   882,    -1,   884,   912,    -1,
    2377,    42,    -1,    -1,    27,    28,    29,    30,  2404,  2320,
      -1,  2377,   291,   900,   901,   902,    39,   904,    -1,    -1,
     907,   908,  2377,    -1,    -1,    -1,    49,  2404,    -1,    -1,
      -1,    54,    -1,    56,    -1,    58,    -1,    -1,  2404,    -1,
      -1,  1516,    -1,  1518,  1519,  1520,  1521,    -1,    -1,  2404,
      -1,    -1,    93,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,   976,    -1,    -1,    -1,  2377,    -1,   955,    -1,
      -1,   958,    -1,    -1,    -1,    -1,    -1,   118,     1,    -1,
      -1,   169,     5,    -1,     7,    -1,    -1,    -1,    -1,    -1,
    2184,    -1,    -1,  2404,    -1,   885,  1689,    -1,    -1,    -1,
      -1,    -1,    -1,   382,    27,    28,    29,    30,    -1,    -1,
    2204,    -1,    -1,    -1,  1707,   905,   312,    -1,    -1,    -1,
      -1,    -1,   912,    -1,   403,    -1,    -1,    -1,   169,  1043,
    1044,  1045,  1046,  1047,  1048,    -1,  1050,    -1,  2232,  1026,
      -1,   346,   347,    -1,    -1,    -1,   169,  2241,   334,   335,
     336,   337,   338,   339,   340,   341,   342,    -1,    -1,    -1,
      -1,    -1,  1076,  1638,    -1,  1640,    -1,  1642,   382,  1644,
      -1,    -1,  1647,    -1,  1767,    -1,     1,   382,    -1,    -1,
       5,  1095,     7,  1962,    -1,    -1,    -1,    -1,    -1,   403,
      -1,    -1,  1785,  1786,    -1,     1,    -1,   120,   403,     5,
      -1,     7,    27,    28,    29,    30,    -1,   130,    -1,  1096,
    1097,    -1,    -1,    -1,   137,   138,    -1,   305,    -1,    -1,
      -1,    27,    28,    29,    30,    -1,  2320,    -1,    -1,    -1,
    1705,    -1,   155,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,  1032,    -1,    -1,    -1,    -1,    -1,    -1,   290,
     173,   174,    -1,  1043,  1044,  1045,  1046,  1047,  1048,    -1,
    1050,    -1,    -1,    -1,   305,    -1,    -1,   290,    -1,    94,
      -1,   312,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,
     203,    -1,   305,  2377,   207,    -1,  1076,    93,    94,    95,
      96,   314,    98,    -1,    -1,   101,    -1,   140,    -1,    -1,
      -1,    -1,   343,    -1,    -1,  1095,   149,   348,    -1,    -1,
    2404,    -1,   118,    -1,   139,   403,    -1,     1,    -1,    -1,
     343,     5,   165,     7,    -1,   348,   169,   350,    -1,    -1,
     173,   174,    -1,    -1,    -1,   178,   179,   180,   181,   182,
      -1,   382,    -1,    27,    28,    29,    30,    -1,    -1,   174,
      -1,    -1,    -1,  2132,    -1,    -1,    -1,    -1,    42,   382,
    2139,    -1,   403,   169,    -1,    -1,    -1,    -1,    -1,    -1,
    2149,    -1,    -1,    -1,    -1,    -1,    -1,  2156,     1,    -1,
     403,    -1,     5,    -1,     7,    -1,  1861,  1862,  1863,  1864,
      -1,    -1,   315,    -1,    -1,     1,  1871,    -1,    -1,     5,
    1875,     7,    -1,    -1,    27,    28,    29,    30,    -1,    93,
      94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,
      -1,    27,    28,    29,    30,   116,   117,    -1,  1903,  1904,
    1905,  1906,  1346,    -1,   118,    -1,    -1,    -1,    -1,   130,
      -1,     1,    -1,    -1,    -1,     5,  1333,     7,   139,    -1,
    1337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   382,
    1347,  1348,  1349,  1350,   155,  1352,   291,    27,    28,    29,
      30,    94,  2251,  2252,  2253,  2254,    -1,    -1,   101,    -1,
     403,    -1,  1369,    -1,   290,   169,     1,    -1,    -1,    -1,
       5,    -1,     7,    -1,     1,    -1,    -1,    -1,     5,   305,
       7,    -1,   108,   109,   110,    -1,   312,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,   139,    -1,    -1,    -1,
      27,    28,    29,    30,  1438,  1439,  1440,  1441,  1442,    -1,
    1444,  1445,    -1,    -1,    -1,    -1,  1423,   343,  1425,  1453,
    1454,    -1,   348,    -1,    -1,  1432,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,  1468,    -1,  1346,   382,   118,    -1,
    1447,    -1,  1449,    -1,    -1,    -1,     1,    -1,    -1,    -1,
       5,  1458,     7,  1460,    -1,    -1,   382,    -1,   403,    -1,
    1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,
    1504,    98,    27,    28,    29,    30,    -1,   403,    -1,    40,
     291,   292,   293,   294,    -1,  1519,   290,    -1,    -1,     1,
      -1,    -1,    -1,     5,  2207,     7,   307,   308,     1,  2398,
     311,   305,     5,    -1,     7,    -1,  1416,  1417,   312,  1419,
      -1,    -1,   139,    -1,    -1,    27,    28,    29,    30,  2114,
    2115,  2116,  2117,    -1,    27,    28,    29,    30,    -1,    24,
     117,   118,    -1,   120,    -1,    -1,    -1,    -1,    -1,   343,
      -1,    -1,    -1,     1,   348,    40,   107,     5,   291,     7,
      -1,    -1,    -1,   140,    -1,    -1,    -1,    -1,  1468,    -1,
      -1,   148,   149,   118,   151,    -1,    -1,    -1,    -1,    27,
      28,    29,    30,    -1,   161,   162,   163,    -1,   382,    -1,
      -1,    -1,    -1,    -1,  1494,  1495,  1496,  1497,  1498,  1499,
    1500,  1501,  1502,  1503,  1504,    -1,    -1,    -1,    -1,   403,
      -1,  2500,  2501,  2502,  2503,    -1,    -1,    -1,    -1,  1519,
      -1,   291,   107,    -1,    -1,   118,    -1,    -1,    -1,   180,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,  1670,    -1,    -1,   382,
      -1,    -1,    -1,  1650,  1651,    -1,    -1,    -1,    -1,    -1,
    1684,    39,    -1,    -1,    -1,  1689,   382,    -1,    -1,    47,
     403,    49,    -1,    51,   291,    -1,    54,    55,    56,  1676,
      58,    -1,    -1,  1707,    -1,  1709,    -1,   403,    -1,  1686,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,   382,    -1,    -1,    -1,    -1,    -1,    -1,   344,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
       5,    -1,     7,   403,    -1,    -1,    -1,    -1,    -1,  1736,
      -1,    -1,    -1,  1767,    -1,    -1,   291,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,  1753,   382,  1755,    -1,
      -1,  1785,  1786,    -1,    -1,   382,    -1,    42,    -1,    -1,
    1670,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   403,    -1,
      -1,    -1,  1682,    -1,  1684,  1685,   403,    -1,    -1,  1689,
      -1,  2376,  2377,  2378,  2379,    -1,    -1,    -1,   291,    -1,
    2385,  2386,  2387,  2388,  2389,   307,    -1,  1707,    -1,  1709,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,  1727,    -1,    -1,
    1730,    -1,    -1,  1733,    -1,     1,    -1,   382,  1862,     5,
      -1,     7,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1875,    -1,    -1,  2439,  2440,  2441,  2442,   403,    -1,
      -1,    27,    28,    29,    30,    -1,    -1,  1767,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
     382,    -1,    -1,    -1,    -1,  1785,  1786,    -1,    -1,   382,
      -1,    -1,    -1,    -1,   169,    -1,   344,    -1,    -1,    -1,
      -1,   403,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
     403,    -1,   290,    -1,    11,    12,    13,    14,    15,    16,
      17,    -1,    -1,    20,    -1,    -1,    -1,   305,    94,    95,
      96,  1955,    98,    -1,   382,   101,   314,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1948,   118,    -1,    -1,   403,    -1,    -1,    -1,    -1,
      -1,    -1,  1862,    -1,    -1,   343,    -1,    -1,    -1,    -1,
     348,    -1,   350,  1970,    -1,  1875,    -1,    74,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,  2016,    90,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   169,   101,   102,   103,   104,   105,    -1,
     107,    -1,    -1,  2010,   111,   290,   113,   114,   115,   116,
     117,   118,   119,    -1,    -1,   122,    -1,   124,    -1,    -1,
     305,   128,   129,   130,   131,   132,    -1,   312,    -1,   136,
      -1,   138,   139,   140,   141,   142,    -1,   144,   145,    -1,
      -1,   148,   149,   150,   151,  1955,   187,   154,    -1,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,
      -1,    -1,    -1,   348,    -1,    -1,    -1,    -1,    -1,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    -1,   382,    -1,    -1,
      -1,    47,    -1,    49,   113,    51,  2016,    -1,    54,    55,
      56,    -1,    58,    -1,   290,    -1,    -1,    -1,   403,    -1,
      -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   305,
    2164,  2165,    -1,    -1,  2168,  2169,   312,    39,   147,    -1,
      -1,    -1,    -1,   152,    -1,    47,   155,    49,    -1,    51,
    2184,   160,    54,    55,    56,   164,    58,    -1,   167,   168,
      -1,    63,    64,   172,    -1,    -1,   175,   343,   177,    -1,
    2204,    -1,   348,  2207,    -1,    -1,    -1,    -1,    -1,   286,
     287,   288,   289,   290,   291,   292,   293,    -1,   295,   296,
     297,   298,   299,   202,    39,  2202,    -1,    -1,    -1,   208,
      -1,    -1,    47,    -1,    49,    -1,   382,    -1,    -1,    54,
      55,    56,   114,    58,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    -1,   124,     5,    -1,     7,    -1,   403,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    27,    28,    29,    30,    31,
      -1,    33,    -1,   155,    -1,    37,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    -1,  2184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    -1,    -1,    70,    -1,
      -1,    73,    -1,    -1,  2204,    -1,    -1,  2207,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
     102,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
      -1,   113,    -1,   115,     1,    -1,   118,   119,     5,    -1,
       7,    -1,   124,  2377,   290,    -1,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   305,
      27,    28,    29,    30,    -1,   147,    -1,    -1,   314,    -1,
     152,    -1,    -1,   155,    -1,    42,    -1,    -1,   160,    -1,
      -1,   488,   164,    -1,    -1,   167,   168,   169,   290,    -1,
     172,    -1,    -1,   175,    -1,   177,    -1,   343,   505,    -1,
      -1,    -1,   348,   305,   350,    -1,    -1,    -1,    -1,    -1,
     517,    -1,   314,    -1,    -1,    -1,    -1,    -1,   525,    -1,
     202,   203,    -1,   205,   206,   207,   208,    94,    95,    96,
      -1,    98,    -1,    -1,   101,   542,    -1,    -1,    -1,    -1,
      -1,   343,    -1,    -1,    -1,   290,   348,    -1,   350,    -1,
      -1,   118,    -1,   560,    -1,    -1,    -1,    -1,    -1,    -1,
     305,   243,   244,   245,   246,    -1,    -1,  2377,    -1,   314,
     252,   253,    -1,    -1,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,    -1,    -1,   343,    -1,
      -1,    -1,   169,   348,    -1,   350,    -1,    -1,   290,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   305,    -1,    -1,    -1,    -1,    -1,    -1,
     312,    -1,   314,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   328,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   343,    -1,    -1,    -1,    -1,   348,    -1,    -1,    -1,
      -1,    -1,   354,   355,    -1,   357,    -1,    -1,   360,   361,
     362,   363,    -1,    -1,    -1,   367,    -1,    -1,    -1,   371,
     372,   373,    -1,    -1,    -1,   377,   378,    -1,    -1,    -1,
     382,    -1,    -1,    -1,   386,     1,    -1,   389,   390,     5,
      -1,     7,    -1,   395,    -1,    -1,    -1,    -1,    -1,    -1,
     402,   403,   404,   290,    -1,   407,    -1,    -1,    24,    -1,
      -1,    27,    28,    29,    30,    31,    -1,    33,   305,    -1,
      -1,    37,    38,    39,    -1,   312,    -1,    -1,   116,   117,
      46,    -1,   120,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,   139,    -1,    -1,    70,    -1,   343,    73,    -1,    -1,
      -1,   348,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,
      86,    87,    88,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    98,    80,    -1,    -1,   102,    -1,   104,    -1,
      -1,   107,   108,   109,   110,   382,    -1,   113,    -1,   115,
      -1,    -1,   118,   119,    -1,    -1,    -1,    -1,   124,    -1,
      -1,   108,   109,   110,   130,    -1,   403,    -1,   115,    -1,
      -1,    -1,   119,    -1,    -1,   852,    -1,   124,    -1,    -1,
      -1,   147,    -1,   130,    -1,    -1,   152,   134,    -1,   155,
      -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,   164,    -1,
      -1,   167,   168,   169,    -1,    -1,   172,    -1,   155,   175,
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   259,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   291,   292,   293,   294,   295,   296,    -1,
     298,   299,   300,   301,   302,   303,   304,    -1,    -1,   307,
     308,    -1,    -1,   311,    -1,    -1,    -1,   243,   244,   245,
     246,    -1,    -1,    -1,    -1,    -1,   252,   253,    -1,    -1,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   350,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   290,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   305,
      -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,   314,    -1,
      -1,    -1,    -1,   300,    -1,    -1,   303,   304,    -1,    -1,
      -1,    -1,   328,    -1,    -1,   312,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,    -1,
      -1,    -1,   348,    -1,    -1,    -1,    -1,    -1,   354,   355,
      -1,   357,    -1,    -1,   360,   361,   362,   363,    -1,    -1,
      -1,   367,    -1,    -1,    -1,   371,   372,   373,    -1,    -1,
      -1,   377,   378,   360,   361,    -1,   382,    -1,    -1,    -1,
     386,     1,    -1,   389,   390,     5,    -1,     7,    -1,   395,
      -1,    -1,    -1,    -1,    -1,    -1,   402,   403,   404,    -1,
      -1,   407,    -1,    -1,    24,    -1,    -1,    27,    28,    29,
      30,    31,    -1,    33,    -1,    -1,    -1,    37,    38,    39,
     407,    -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1159,    -1,    -1,    65,    66,    -1,    -1,    -1,
      70,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    83,    84,    85,    86,    87,    88,   113,
      -1,   115,    -1,    -1,    -1,   119,    -1,    -1,    98,    -1,
     124,    -1,   102,    -1,   104,    -1,   130,   107,   108,   109,
     110,    -1,    -1,   113,    -1,   115,    -1,    -1,   118,   119,
      -1,    -1,    -1,   147,   124,    -1,    -1,    -1,   152,    -1,
     130,   155,    -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,   168,   169,    -1,   147,   172,    -1,
      -1,   175,   152,   177,    -1,   155,    -1,    -1,    -1,    -1,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,   177,   202,    -1,
     204,    -1,    -1,    -1,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,   203,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   243,   244,   245,   246,    -1,    -1,    -1,
      -1,    -1,   252,   253,    -1,    -1,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,   115,    -1,    -1,
     290,   119,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,   305,    -1,    -1,    -1,    -1,
      -1,    -1,   312,    -1,   314,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   328,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,    -1,   343,   172,    -1,    -1,   175,   348,   177,
      -1,    -1,    -1,    -1,   354,   355,    -1,   357,    -1,    -1,
     360,   361,   362,   363,    -1,    -1,    -1,   367,    -1,    -1,
      -1,   371,   372,   373,   202,    -1,    -1,   377,   378,    -1,
     208,    -1,   382,    -1,    -1,    -1,   386,     1,    -1,   389,
     390,     5,    -1,     7,    -1,   395,    -1,    -1,    -1,    -1,
      -1,    -1,   402,   403,   404,    -1,    -1,   407,    -1,    -1,
      24,    -1,    -1,    27,    28,    29,    30,    31,    -1,    33,
      -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    -1,    -1,    70,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      84,    85,    86,    87,    88,   113,    -1,   115,    -1,    -1,
     118,   119,    -1,    -1,    98,    -1,   124,    -1,   102,    -1,
     104,    -1,   130,   107,   108,   109,   110,    -1,    -1,   113,
      -1,   115,    -1,    -1,   118,   119,    -1,    -1,    -1,   147,
     124,    -1,    -1,    -1,   152,    -1,   130,   155,    -1,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,    -1,   147,   172,    -1,    -1,   175,   152,   177,
      -1,   155,    -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,   168,   169,    -1,    -1,   172,    -1,
      -1,   175,    -1,   177,   202,    -1,   204,    -1,    -1,    -1,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,   203,
      -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    -1,    47,
      48,    49,    50,    -1,    -1,    53,    54,    -1,    56,    57,
      58,    -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,   243,
     244,   245,   246,    -1,    -1,    -1,    -1,    -1,   252,   253,
      -1,    -1,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   102,    -1,   104,   105,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   305,    -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,
     314,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   328,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,
      -1,    -1,    -1,    -1,   348,    -1,    -1,    -1,    -1,    -1,
     354,   355,    -1,   357,    -1,    -1,   360,   361,   362,   363,
      -1,    -1,     5,   367,     7,    -1,    -1,   371,   372,   373,
      -1,    -1,    -1,   377,   378,    -1,    -1,    -1,   382,    -1,
      -1,    24,   386,    -1,    -1,   389,   390,    -1,    31,    -1,
      33,   395,    -1,    -1,    37,    38,    39,    -1,   402,   403,
     404,    -1,    -1,   407,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    -1,    -1,    70,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,   104,   290,    -1,   107,   108,   109,   110,    -1,    -1,
     113,    -1,   115,    -1,    -1,    -1,   119,   305,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,   314,   130,    -1,    -1,
      -1,    -1,    -1,   321,   322,   323,   324,   325,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
      -1,    -1,   155,    -1,    -1,   343,    -1,   160,    -1,    -1,
     348,   164,   350,   351,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
     203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     243,   244,   245,   246,    -1,    -1,    -1,    -1,    -1,   252,
     253,    -1,    -1,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   305,    -1,    -1,     6,    -1,    -1,    -1,   312,
      -1,   314,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,   328,    -1,    -1,    -1,    -1,
      31,    -1,    33,    -1,    -1,    -1,    -1,    38,    -1,    -1,
     343,    -1,    -1,    -1,    -1,   348,    -1,    -1,    -1,    -1,
      -1,   354,   355,    -1,   357,    -1,    -1,   360,   361,   362,
     363,    -1,    -1,    -1,   367,    -1,    -1,    -1,   371,   372,
     373,    -1,    73,    -1,   377,   378,    -1,    -1,    -1,    80,
      -1,    -1,    -1,   386,    -1,    -1,   389,   390,    89,    -1,
      91,    -1,   395,    -1,    -1,    -1,    -1,    -1,    -1,   402,
     403,   404,    -1,    -1,   407,    -1,   107,   108,   109,   110,
      -1,    -1,   113,    -1,   115,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,
      -1,    -1,    -1,   134,    -1,    94,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,    -1,    -1,   155,    -1,    -1,   116,   117,   160,
      -1,   120,    -1,   164,    -1,    -1,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,   177,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   202,   203,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,
      -1,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   300,
     259,    -1,   303,   304,    -1,    -1,    -1,    -1,    -1,    24,
      -1,   312,    -1,   314,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,   329,    -1,
      -1,    -1,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,    -1,    -1,   307,   308,
      -1,    -1,   311,   354,   355,    -1,    -1,    -1,    73,   360,
     361,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,   373,    -1,    -1,    -1,   377,   378,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   387,   388,    -1,   390,
      -1,   350,   107,   108,   109,   110,    -1,    -1,   113,    -1,
     115,    -1,    -1,   404,   119,    -1,   407,    -1,    -1,   124,
      -1,    -1,    -1,    94,    -1,   130,    -1,    -1,    -1,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,   116,   117,   152,    -1,   120,
     155,    -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,   168,    -1,    -1,    -1,   172,   139,    -1,
     175,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,   203,    -1,
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   243,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   290,    -1,     6,   259,    -1,
      -1,    -1,    -1,    -1,    -1,   300,    -1,    -1,   303,   304,
      -1,    -1,    -1,    -1,    -1,    24,    -1,   312,    -1,   314,
      -1,    -1,    31,    -1,    33,    -1,    -1,    -1,    -1,    38,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,    -1,    -1,   307,   308,    -1,    -1,
     311,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   354,
     355,    -1,    -1,    -1,    73,   360,   361,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,   373,    -1,
      -1,    -1,   377,   378,    -1,    -1,    -1,    -1,    -1,   350,
      -1,    -1,   387,   388,    -1,   390,    -1,    -1,   107,   108,
     109,   110,    -1,    -1,   113,    -1,   115,    -1,    -1,   404,
     119,    -1,   407,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,    -1,    -1,    -1,
      -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   202,   203,    -1,   205,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   290,    -1,     6,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   300,    -1,    -1,   303,   304,    -1,    -1,    -1,    -1,
      -1,    24,    -1,   312,    -1,   314,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   354,   355,    -1,    -1,    -1,
      73,   360,   361,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,   373,    -1,    -1,    -1,   377,   378,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   387,   388,
      -1,   390,    -1,    -1,   107,   108,   109,   110,    -1,    -1,
     113,    -1,   115,    -1,    -1,   404,   119,    -1,   407,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
      -1,    -1,   155,    -1,    -1,    -1,    -1,   160,    -1,    -1,
      -1,   164,    -1,    -1,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
     203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     243,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   300,    -1,    -1,
     303,   304,    -1,    -1,    -1,    -1,    -1,    24,    -1,   312,
      -1,   314,    -1,    -1,    31,    -1,    33,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   354,   355,    -1,    -1,    -1,    73,   360,   361,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
     373,    -1,    -1,    -1,   377,   378,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   387,   388,    -1,   390,    -1,    -1,
     107,   108,   109,   110,    -1,    -1,   113,    -1,   115,    -1,
      -1,   404,   119,    -1,   407,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    -1,    -1,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,    -1,
      -1,    -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   243,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   290,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   300,    -1,    -1,   303,   304,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   312,    -1,   314,    -1,    10,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    20,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   354,   355,    -1,
      -1,    -1,    -1,   360,   361,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   373,    -1,    -1,    -1,
     377,   378,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
     387,   388,    -1,   390,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    -1,   404,    -1,    -1,
     407,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,
      -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,   147,    -1,    -1,    -1,
      -1,   152,    80,    -1,   155,    -1,    -1,    -1,    -1,   160,
      -1,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,   177,    -1,    -1,   107,
     108,   109,   110,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,   202,   130,    -1,   205,    -1,   134,   208,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,    -1,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,    -1,    -1,   300,
      -1,   115,   303,   304,    -1,   119,    -1,    -1,    -1,    -1,
     124,   312,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,    -1,
      -1,   155,    -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,   168,    -1,    -1,    -1,   172,   360,
     361,   175,   363,   177,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   300,    -1,    -1,   303,   304,    23,    24,    -1,
      -1,    -1,    -1,    -1,   312,    -1,   387,   388,   202,    -1,
      -1,   205,    -1,    -1,   208,    -1,    -1,    -1,    -1,    -1,
      -1,   402,    -1,   404,    -1,    -1,   407,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,   360,   361,    80,   363,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   387,
     388,   107,   108,   109,   110,    -1,    -1,    -1,    -1,   115,
      -1,    -1,    -1,   119,   402,    -1,   404,    -1,   124,   407,
      -1,    -1,    -1,    -1,   130,    -1,   300,    -1,   134,   303,
     304,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,
      -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,   164,    -1,
      -1,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   360,   361,    -1,   363,
      -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,   205,
      -1,    -1,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   387,   388,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
     404,    -1,    31,   407,    33,    -1,    -1,    -1,    37,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    -1,
      -1,    70,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      -1,    -1,    -1,    -1,   300,    -1,    -1,   303,   304,    -1,
      -1,    -1,    -1,   102,    -1,   104,   312,    -1,   107,   108,
     109,   110,    -1,    -1,   113,    -1,   115,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   360,   361,   155,   363,    -1,    -1,
      -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,    -1,
      -1,   387,   388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   402,    -1,   404,    -1,
      -1,   407,    -1,   202,   203,    -1,   205,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   243,   244,   245,   246,    -1,    -1,
      -1,    -1,    -1,   252,   253,    -1,    -1,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   290,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   305,    -1,    -1,    -1,
      -1,    -1,    -1,   312,    -1,   314,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,   328,
      -1,    -1,    -1,    31,    -1,    33,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,   343,    -1,    -1,    -1,    -1,   348,
      -1,    -1,    -1,    -1,    -1,   354,   355,    -1,   357,    -1,
      -1,   360,   361,   362,   363,    -1,    -1,    -1,   367,    -1,
      -1,    -1,   371,   372,   373,    73,    -1,    -1,   377,   378,
      -1,    -1,    80,    -1,    -1,    -1,    -1,   386,    -1,    -1,
     389,   390,    -1,    -1,    -1,    -1,   395,    -1,    -1,    -1,
      -1,    -1,    -1,   402,    -1,   404,    -1,    -1,   407,   107,
     108,   109,   110,    -1,    -1,   113,    -1,   115,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,    -1,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    33,   290,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,   300,    -1,    -1,   303,   304,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   312,    -1,   314,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   354,   355,    -1,    -1,
      -1,    -1,   360,   361,    -1,   107,   108,   109,   110,    -1,
      -1,   113,    -1,   115,    -1,   373,    -1,   119,    -1,   377,
     378,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,   387,
     388,    -1,   390,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   404,    -1,    -1,   407,
     152,    -1,    -1,   155,    -1,    -1,    -1,    -1,   160,    -1,
      -1,    -1,   164,    -1,    -1,   167,   168,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,   177,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     202,   203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   243,   244,   245,   246,    -1,    -1,    -1,    -1,    -1,
     252,   253,    -1,    -1,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,
      -1,    -1,    -1,    -1,   107,   108,   109,   110,    -1,    -1,
     113,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,    -1,
      -1,   124,   314,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
      -1,    -1,   155,    -1,    -1,    -1,    -1,   160,    -1,    -1,
      -1,   164,   354,   355,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,   373,    -1,    -1,    -1,   377,   378,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,   389,   390,   202,
     203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
     402,    -1,   404,    -1,    -1,   407,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     243,   244,   245,   246,    73,    -1,    -1,    -1,    -1,   252,
     253,    80,    -1,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    -1,    -1,    -1,   107,   108,
     109,   110,    -1,    -1,    -1,    -1,   115,   290,    -1,    -1,
     119,    -1,    -1,    -1,   135,   124,    -1,    -1,   139,    -1,
      -1,   130,    -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,
      -1,   314,    -1,    -1,    -1,   156,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,    -1,    -1,    -1,
      -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,    -1,
      -1,   354,   355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     373,    -1,    -1,   202,   377,   378,   205,    -1,    -1,   208,
      -1,    -1,    -1,    -1,    -1,    -1,   389,   390,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   404,    24,    -1,   407,    -1,    -1,    -1,    -1,    31,
      -1,    33,    -1,    -1,    -1,    -1,    38,    39,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,   300,    -1,    -1,   303,   304,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   312,    -1,    -1,   108,    -1,    -1,    -1,
      -1,   113,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,   350,
      -1,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   364,    -1,   147,    -1,    -1,    -1,    -1,
     152,   360,   361,   155,   363,    -1,    -1,    -1,   160,    -1,
      -1,    -1,   164,    -1,    -1,   167,   168,    -1,    -1,   390,
     172,    -1,    -1,   175,    -1,   177,    -1,    -1,   387,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   402,    -1,   404,    -1,    -1,   407,    -1,
     202,   203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   243,   244,   245,   246,    -1,    -1,    -1,    -1,    -1,
     252,   253,    -1,    -1,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,   115,   305,    -1,    -1,   119,    -1,    -1,    -1,
      -1,   124,   314,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
      -1,   343,   155,    -1,    -1,    -1,   348,   160,   350,    -1,
      -1,   164,   354,   355,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,   177,   367,    -1,    -1,    -1,    -1,
      -1,   373,    -1,    -1,    -1,   377,   378,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   389,   390,   202,
     203,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     243,   244,   245,   246,    -1,    -1,    -1,    -1,    -1,   252,
     253,    -1,    -1,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,    -1,
      -1,     0,     1,    -1,    -1,    -1,     5,    -1,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   314,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,    58,
      -1,   354,   355,    -1,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
     373,    80,    81,    -1,   377,   378,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,   389,   390,    -1,    80,
      -1,    -1,    83,    84,    85,    86,    87,    88,    -1,   108,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,   107,   108,   109,   110,
      -1,   130,    -1,    -1,   115,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,
      -1,    -1,    -1,   134,    -1,    -1,   155,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,   147,    -1,    -1,    -1,
      -1,   152,    80,    -1,   155,    -1,    -1,    -1,    -1,   160,
      -1,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,   177,    -1,    -1,   107,
     108,   109,   110,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,   202,   130,    -1,   205,    -1,   134,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,   147,
      -1,    -1,    -1,    -1,   152,    80,    -1,   155,    -1,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,
      -1,    -1,   107,   108,   109,   110,    -1,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,   124,
      -1,   290,    -1,    -1,   202,   130,    -1,   205,    -1,   134,
     208,    -1,    -1,    -1,    -1,    -1,   305,    -1,    -1,    -1,
      -1,    -1,   147,   312,    -1,   314,    -1,   152,    -1,   300,
     155,    -1,   303,   304,    -1,   160,    -1,    -1,    -1,   164,
      -1,   312,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,
     175,    -1,   177,    -1,   343,    -1,    -1,    -1,    -1,   348,
      -1,   350,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,
     205,    -1,    -1,   208,    -1,    -1,    -1,    -1,    -1,   360,
     361,    -1,   363,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   300,    -1,    -1,   303,   304,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   312,    -1,   387,   388,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   404,    -1,    -1,   407,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   360,   361,    -1,   363,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   300,    -1,    -1,   303,   304,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,   387,
     388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   404,    -1,    -1,   407,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   360,   361,    -1,   363,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   387,   388,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   404,
      -1,    -1,   407
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   411,     0,     1,     5,     7,    27,    28,    29,    30,
      39,    47,    49,    51,    54,    55,    56,    58,    63,    64,
      65,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    80,    81,   114,   124,   130,   155,   290,
     305,   314,   343,   348,   350,   412,   470,   471,   472,   473,
     559,   560,   561,   563,   578,   412,   109,   108,   312,   555,
     555,   555,   561,   572,   561,   563,   578,   561,   566,   566,
     566,   561,   569,   473,    51,   474,    39,    47,    49,    54,
      55,    56,    58,   290,   305,   314,   343,   348,   350,   475,
      51,   476,    39,    47,    49,    51,    54,    55,    56,    58,
     290,   305,   314,   343,   348,   350,   481,    55,   483,    39,
      44,    47,    48,    49,    50,    53,    54,    56,    57,    58,
      60,   102,   104,   105,   106,   290,   305,   314,   321,   322,
     323,   324,   325,   343,   348,   350,   351,   486,    51,    52,
      54,    55,    56,   305,   321,   322,   348,   489,    47,    49,
      54,    56,    60,   102,   104,   490,    49,   491,    24,    31,
      33,    38,   107,   108,   109,   110,   113,   115,   119,   124,
     130,   147,   152,   155,   160,   164,   167,   168,   172,   175,
     177,   202,   203,   205,   206,   207,   208,   243,   244,   245,
     246,   252,   253,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   290,   314,   354,   355,   373,
     377,   378,   389,   390,   404,   407,   499,   556,   695,   696,
     699,   700,   701,   702,   706,   782,   785,   787,   790,   793,
     798,   799,   801,   803,   813,   815,   817,   819,   821,   823,
     827,   829,   831,   833,   835,   837,   839,   841,   843,   845,
     849,   851,   853,   855,   866,   874,   876,   878,   879,   881,
     883,   885,   887,   889,   891,   893,   895,    60,   383,   384,
     385,   492,   498,    60,   493,   498,    39,    47,    49,    51,
      54,    55,    56,    58,   290,   305,   314,   343,   348,   350,
     482,    79,    80,   494,   495,   415,   434,   435,    94,   196,
     297,   299,   572,   572,   572,   572,   572,     0,   412,   555,
     555,    59,   380,   381,   575,   576,   577,    37,    39,    54,
      65,    66,    70,    73,    83,    84,    85,    86,    87,    88,
     102,   104,   258,   290,   305,   312,   314,   328,   343,   348,
     357,   360,   361,   362,   363,   367,   371,   372,   386,   395,
     402,   582,   583,   584,   586,   587,   588,   589,   590,   591,
     592,   593,   594,   595,   596,   597,   598,   599,   603,   606,
     608,   609,   612,   613,   615,   616,   623,   624,   629,   630,
     638,   639,   642,   643,   644,   645,   646,   668,   669,   671,
     672,   674,   675,   678,   679,   680,   690,   691,   692,   693,
     694,   701,   708,   709,   710,   711,   712,   713,   717,   718,
     719,   766,   780,   785,   786,   811,   812,   813,   856,   412,
     401,   401,   412,   555,   651,   500,   505,   582,   555,   512,
     515,   695,   719,   518,   555,   526,   563,   579,   572,   561,
     563,   566,   566,   566,   569,    94,   196,   297,   299,   572,
     572,   572,   572,   572,   578,   480,   561,   572,   573,   477,
     559,   561,   562,   478,   561,   563,   564,   579,   479,   561,
     566,   567,   566,   479,   561,   569,   570,    94,   196,   297,
     299,   743,   480,   480,   480,   480,   480,   566,   572,   488,
     560,   581,   561,   581,   563,    47,   484,   485,   560,   561,
     581,   485,   566,   566,   485,   569,   581,    47,    48,   566,
     581,   485,    94,   196,   297,   326,   327,   743,   744,   572,
     572,   485,   485,   485,   485,   485,   572,   572,   572,   485,
     441,   579,    49,   484,   561,   563,   581,   566,   566,   566,
     572,   485,   485,   572,   454,   561,   563,   566,   566,   581,
      47,   566,   563,   108,   111,   112,   113,   816,   116,   117,
     259,   260,   263,   704,   705,    34,    35,    36,   259,   788,
     137,   707,   173,   174,   877,   116,   117,   118,   818,   118,
     120,   121,   122,   123,   820,   116,   117,   125,   126,   127,
     128,   129,   822,   116,   117,   120,   131,   132,   133,   134,
     135,   136,   137,   139,   140,   182,   824,   118,   120,   140,
     148,   149,   150,   151,   828,   118,   140,   153,   330,   830,
     116,   117,   131,   133,   134,   135,   157,   158,   159,   832,
     117,   118,   120,   140,   148,   149,   151,   161,   162,   163,
     834,   132,   149,   158,   165,   166,   836,   149,   166,   838,
     158,   169,   170,   840,   135,   140,   173,   174,   842,   140,
     173,   174,   176,   844,   140,   149,   165,   169,   173,   174,
     178,   179,   180,   181,   182,   857,   118,   173,   174,   182,
     867,   169,   204,   817,   819,   821,   823,   827,   829,   831,
     833,   835,   837,   839,   841,   843,   845,   846,   847,   848,
     850,   866,   874,   876,   131,   139,   169,   431,   854,   431,
     118,   204,   848,   852,   140,   173,   174,   209,   242,   875,
     118,   130,   132,   151,   155,   158,   247,   288,   289,   390,
     800,   802,   882,   248,   884,   248,   886,   169,   249,   250,
     251,   888,   132,   158,   880,   120,   136,   158,   165,   254,
     255,   890,   132,   158,   892,   118,   132,   140,   158,   165,
     894,   135,   139,   156,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   350,   364,   390,   783,   784,   785,
     116,   117,   120,   139,   259,   291,   292,   293,   294,   295,
     296,   298,   299,   300,   301,   302,   303,   304,   307,   308,
     311,   350,   804,   805,   808,    62,   120,   789,   364,   374,
     792,   713,   718,   375,   243,   252,   253,   256,   257,   896,
     393,   394,   440,   795,   712,   555,   460,   498,   384,   439,
     498,   427,   480,   477,   478,   563,   579,   479,   566,   566,
     569,   570,   196,   743,   480,   480,   480,   480,   480,   422,
     445,    48,    50,    52,    53,    60,    61,    96,   496,   572,
     572,   572,   572,   419,   738,   765,   753,   740,   742,   107,
     107,   107,    89,   800,   306,   691,   177,   555,   695,   781,
     781,    65,   103,   555,   783,    94,   196,   297,   804,   805,
     306,   306,   306,   344,   306,    89,   169,    89,    89,   800,
     108,     5,   413,   720,   721,   382,   580,   594,   469,   505,
     107,   346,   607,   418,   419,   307,   308,   610,   611,   349,
     614,   420,   466,   169,   329,   330,   331,   332,   333,   617,
     618,   346,   347,   625,   632,   442,   346,   347,   626,   631,
     450,   453,   358,   641,   446,     6,    73,    80,    89,    91,
     115,   119,   124,   130,   134,   155,   243,   300,   303,   304,
     312,   329,   360,   361,   387,   388,   404,   654,   655,   656,
     657,   658,   659,   660,   662,   663,   664,   665,   666,   667,
     696,   699,   702,   706,   775,   776,   777,   782,   787,   790,
     793,   799,   800,   801,   803,   809,   810,   813,   461,   467,
      40,    41,   192,   195,   647,   648,   649,   446,    89,   364,
     365,   366,   670,   676,   677,   446,    89,   673,   676,   424,
     430,   451,   368,   369,   370,   681,   682,   686,   687,    24,
     695,   697,   698,    16,    17,    18,    19,   406,     9,    25,
      56,    10,    11,    12,    13,    14,    15,    20,   115,   119,
     124,   130,   147,   152,   155,   160,   164,   167,   168,   172,
     175,   177,   202,   205,   208,   363,   404,   696,   698,   699,
     714,   715,   716,   719,   767,   768,   769,   770,   771,   772,
     773,   774,   776,   777,   778,   779,    54,    54,    23,   402,
     736,   767,   768,   773,   736,    40,   402,   650,   402,   402,
     402,   402,   402,   575,   582,   651,   500,   505,   512,   515,
     518,   526,   572,   572,   572,   572,   419,   738,   765,   753,
     740,   742,   582,   467,    59,    59,    59,   505,    59,   515,
      59,   526,   480,   572,   419,   468,   442,   450,   458,   515,
     467,    45,   487,   561,   566,   581,   572,   485,   572,    47,
     419,   468,   442,   450,   458,   563,   515,   419,   450,   566,
     555,   463,     8,     9,    32,   118,   263,   264,   703,   333,
     459,   399,   400,   131,   306,   463,   462,   426,   462,   436,
     115,   130,   115,   130,   415,   143,   144,   145,   146,   825,
     434,   462,   437,   462,   438,   435,   462,   437,   414,   425,
     417,   464,   465,    24,    40,   107,   180,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     858,   859,   860,   462,   107,   421,   460,   846,   431,   848,
     187,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   868,   873,
     456,   462,   434,   435,   440,   802,   455,   455,   408,   455,
     455,   408,   455,   433,   429,   423,   462,   444,   458,   443,
     458,   116,   117,   130,   139,   155,   291,   292,   293,   806,
     807,   808,   418,   375,   375,   107,   455,   433,   429,   423,
     444,   392,   794,   405,   467,   515,   526,   480,   572,   419,
     468,   442,   450,   458,   497,   498,   751,   604,   751,   751,
     307,   402,   737,   402,   344,   402,   752,   402,   626,   739,
     402,   556,   741,     6,   130,   155,   666,    89,   666,   688,
     689,   713,   182,    24,    24,   101,   402,    54,    54,   306,
      54,   808,    54,   666,   666,   666,   666,   683,   684,   685,
     695,   699,   713,   717,   793,   799,   686,   666,   666,    89,
     556,    22,   719,   724,   725,   726,   734,   773,   774,     8,
     403,   556,   402,   107,   107,   107,   611,   108,   313,   557,
      82,   115,   130,   177,   266,   620,   556,   107,   107,   107,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   619,
     618,   452,   107,   634,   636,   107,   147,   160,   177,   359,
     666,   454,   422,     6,   666,    89,   426,   436,   415,   434,
     435,   420,    89,   446,   446,   659,   696,   777,    16,    17,
      18,    19,   406,    21,    23,     9,    56,     6,   676,    89,
      91,   248,   329,     8,     8,   107,   107,   649,     6,     8,
       6,   666,   684,   695,   699,   682,     8,   555,   402,   778,
     778,   769,   770,   771,   712,   402,   600,   697,   768,   434,
     437,   435,   437,   414,   425,   417,   464,   465,   460,   421,
     431,   456,   446,   773,     8,    21,    16,    17,    18,    19,
     406,     8,    21,    23,     9,   767,   768,   773,   666,   666,
     107,   403,   412,    21,   412,   107,   542,   467,   504,   506,
     514,   523,   527,   650,   402,   402,   402,   402,   402,   751,
     604,   751,   751,   737,   402,   752,   739,   741,   107,   107,
     107,   402,   107,   107,   402,   418,   751,   108,   418,   418,
     561,   107,   705,   462,   428,   107,   448,   448,   426,   434,
     426,   434,   118,   135,   141,   142,   248,   434,   826,   416,
     101,   864,   196,   862,   201,   865,   199,   200,   863,   197,
     198,   861,   135,   421,   232,   236,   237,   238,   872,   227,
     228,   229,   230,   870,   231,   232,   233,   234,   235,   871,
     871,   236,   239,   239,   240,   241,   240,   118,   135,   169,
     869,   457,   455,   107,   107,   116,   117,   116,   117,   418,
     418,   107,   107,   376,   791,   107,   166,   391,   796,   800,
     402,   418,   751,   402,   402,   402,   402,   107,   534,   419,
     764,   632,   540,   442,   535,   107,   450,   541,   458,   666,
       6,     6,   666,   467,   697,    94,    97,   580,   727,   729,
      40,   180,   185,   195,   859,   860,   556,   556,   107,   713,
     722,   723,   666,   666,   666,   666,    54,   666,   419,   468,
     442,   450,    23,   451,    89,   368,    45,   666,   413,     6,
     413,   290,   305,   343,   348,   606,   608,   613,   624,   630,
     731,   732,    94,    97,   580,   730,   733,   413,   721,   507,
     426,   154,   149,   154,   621,   622,    21,   308,   309,   310,
     627,   345,   633,   345,   635,   627,   108,   118,   640,   699,
     118,   640,   460,   118,   640,   666,     6,   666,   666,   405,
     654,   654,   655,   656,   657,   107,   659,   654,   661,   697,
     719,   666,   666,    89,     9,    89,   696,   777,   809,   809,
     666,   677,   666,   676,   687,   416,   688,   413,   601,   602,
     405,   773,   767,   773,   778,   778,   769,   770,   771,   773,
     107,   767,   773,   716,   773,    21,    21,   107,   412,   648,
     403,   412,   470,   580,   650,    39,    49,    54,    56,    58,
     169,   290,   305,   314,   343,   348,   350,   403,   412,   470,
     501,   580,    46,    98,   118,   169,   403,   412,   470,   544,
     551,   552,   580,   582,    42,    93,    94,    95,    96,    98,
     101,   118,   169,   403,   412,   470,   513,   524,   580,   585,
     593,    42,    94,    95,    96,   118,   169,   403,   412,   470,
     524,   580,   585,    43,    46,   169,   305,   403,   412,   470,
     467,   504,   506,   514,   523,   527,   402,   402,   402,   402,
     419,   764,   442,   450,   458,   506,   527,   418,   418,     8,
     459,   462,   434,   860,   462,   456,   409,   409,   434,   434,
     435,   435,   791,   379,   791,   107,   432,   440,   116,   117,
     797,   527,   418,   537,   539,   538,   536,   308,   403,   412,
     470,   580,   737,   120,   130,   137,   138,   155,   173,   174,
     203,   207,   315,   403,   412,   470,   580,   763,   634,   636,
     403,   412,   470,   580,   752,   403,   412,   470,   580,   739,
     627,   403,   412,   470,   580,   741,   666,   666,     6,   557,
     557,   729,   460,   416,   416,   402,   594,   727,   443,   418,
     418,   418,   666,   418,   683,   695,   699,   685,   684,   666,
      45,    86,    87,   735,   774,   780,   196,   418,   419,   420,
     442,   450,   732,   108,   728,   557,   731,   733,   403,   412,
     582,   434,     8,   460,   352,   353,   354,   355,   356,   637,
      21,   107,   107,   349,   628,   640,   640,    73,   640,   666,
       6,   666,   171,   666,   676,   676,     6,   403,   585,   724,
       8,   403,   767,   767,   648,   195,   652,   467,   555,   574,
     555,   565,   555,   568,   568,   555,   571,   108,   502,   503,
      94,   196,   297,   299,   574,   574,   574,   574,   574,   412,
     401,    83,    84,   553,   554,   695,   396,   397,   398,   545,
     103,   412,   412,   412,   412,   412,   510,   511,   700,   557,
     557,   401,    99,   100,   525,   107,   108,   133,   134,   259,
     277,   288,   531,   532,   543,    90,    91,    92,    94,   516,
     517,   412,   412,   412,   412,   593,   511,   557,   557,   401,
     532,   516,   412,   412,   412,   108,   401,   103,   419,   403,
     403,   403,   403,   403,   537,   539,   538,   536,   403,   403,
     403,   107,   814,     8,   447,   107,   432,   440,   403,    98,
     139,   291,   403,   412,   470,   580,   749,   346,   347,   403,
     412,   470,   580,   605,    94,   101,   139,   174,   291,   403,
     412,   470,   580,   750,   118,   291,   403,   412,   470,   580,
     746,   107,   419,   107,   116,   117,   107,   107,   116,   117,
     107,   107,   761,   758,   755,   468,   412,   633,   635,   442,
     450,   628,   458,   666,   722,   403,   418,   451,   451,   666,
     413,   412,   622,   637,   307,   627,   107,   462,   462,   454,
     462,   666,    89,   688,     6,   403,     6,   413,   602,   412,
     192,   653,   533,   505,   512,   518,   526,     8,   574,   574,
     574,   574,   533,   533,   533,   533,   533,   402,   528,   695,
     449,   462,     9,   412,   412,   515,   449,     9,   462,     8,
     412,     6,   412,   412,   515,     6,   412,   156,   546,   528,
     412,   403,   403,   403,   403,   468,   416,   107,   791,   401,
     172,   177,   745,   560,   418,   107,   107,   418,   557,   107,
     745,   107,   560,   418,   109,   560,   418,   696,   696,   696,
     696,   402,   402,   402,   452,   452,   594,   627,   437,   437,
     666,   403,   688,   780,   652,   402,   402,   402,   402,   402,
     503,   533,   533,   533,   533,   402,   402,   402,   402,   402,
     529,   530,   556,   695,   412,    43,   510,   531,   517,    39,
      91,   108,   290,   305,   343,   348,   350,   367,   508,   509,
     511,   520,   521,   700,    91,   520,   522,    23,   107,   108,
     394,   547,   548,   549,   695,   412,   412,   418,   418,   418,
     418,     8,   432,   402,   464,   460,   412,   412,   412,   412,
     412,   412,   412,   139,   412,   434,   434,   435,   435,   760,
     757,   754,   403,   403,   413,   653,   542,   506,   514,   523,
     527,   402,   402,   402,   402,   534,   764,   540,   535,   541,
       8,   403,   108,   467,   511,    94,   196,   297,   519,   419,
     442,   450,   458,   412,     9,   412,   412,   532,   107,    23,
      26,   814,   108,   747,   748,   745,   318,   403,   412,   470,
     762,   319,   403,   412,   470,   759,   120,   131,   295,   381,
     403,   412,   470,   756,   403,   403,   403,   403,   403,   537,
     539,   538,   536,   403,   403,   403,   403,   403,   413,   530,
      45,    46,   550,   418,   700,   462,   412,   107,   107,     6,
       8,   403,   412,   107,   412,   107,   412,   107,   107,   316,
     317,   107,   412,   412,   412,   412,   412,   412,   403,   403,
     403,   403,   412,   412,   412,   412,   412,   556,   695,   401,
     546,   462,   107,   557,   558,   748,   412,   462,   320,   467,
     412,   412,   412,   412,   419,   468,   442,   450,   458,   528,
     449,   107,   418,   418,   418,   418,   210,   556
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   410,   411,   411,   412,   412,   413,   413,   414,   415,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   470,   470,   470,   470,   471,
     471,   471,   471,   472,   472,   472,   472,   472,   472,   472,
     472,   472,   472,   472,   472,   472,   472,   472,   472,   473,
     473,   473,   473,   473,   473,   473,   473,   473,   473,   473,
     473,   473,   473,   473,   473,   473,   473,   473,   473,   473,
     473,   473,   473,   473,   473,   474,   475,   475,   475,   475,
     475,   475,   475,   475,   475,   475,   475,   475,   475,   475,
     475,   475,   475,   475,   475,   476,   477,   477,   478,   478,
     479,   479,   480,   480,   481,   481,   481,   481,   481,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   482,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   482,   482,   483,
     484,   484,   485,   485,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   487,   488,   488,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   490,   490,   490,   490,   490,   490,   490,   491,   492,
     492,   493,   493,   494,   495,   495,   495,   496,   496,   496,
     496,   496,   496,   496,   496,   497,   497,   498,   498,   498,
     499,   500,   501,   501,   502,   502,   503,   504,   504,   504,
     504,   504,   504,   504,   504,   504,   504,   504,   504,   504,
     504,   504,   504,   504,   504,   505,   506,   506,   506,   506,
     506,   506,   506,   506,   506,   507,   507,   507,   508,   508,
     509,   509,   509,   509,   510,   511,   511,   512,   513,   513,
     514,   514,   514,   514,   514,   514,   514,   514,   514,   514,
     514,   514,   515,   515,   516,   516,   517,   517,   517,   517,
     518,   519,   519,   519,   520,   520,   520,   520,   520,   521,
     521,   522,   522,   523,   523,   523,   523,   523,   523,   523,
     523,   523,   523,   523,   523,   523,   523,   524,   524,   525,
     525,   526,   527,   527,   527,   527,   527,   527,   527,   528,
     528,   529,   529,   529,   530,   530,   531,   531,   532,   532,
     533,   534,   534,   534,   534,   534,   535,   535,   535,   535,
     535,   536,   536,   536,   536,   536,   537,   537,   537,   537,
     537,   538,   538,   538,   538,   538,   539,   539,   539,   539,
     539,   540,   540,   540,   540,   540,   541,   541,   541,   541,
     541,   542,   542,   542,   542,   542,   543,   543,   543,   543,
     543,   544,   545,   545,   545,   546,   547,   547,   548,   548,
     548,   548,   548,   549,   549,   550,   550,   550,   550,   551,
     552,   553,   553,   554,   554,   555,   555,   556,   556,   556,
     557,   558,   558,   559,   559,   560,   560,   560,   560,   560,
     560,   561,   562,   563,   564,   565,   566,   567,   568,   569,
     570,   571,   572,   573,   574,   575,   576,   577,   578,   578,
     578,   578,   579,   580,   581,   581,   582,   582,   583,   584,
     584,   585,   585,   586,   587,   588,   589,   590,   591,   591,
     591,   592,   592,   592,   592,   592,   592,   593,   593,   593,
     593,   593,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   595,   596,   596,   597,   598,
     598,   599,   600,   600,   601,   601,   601,   602,   603,   604,
     605,   605,   606,   607,   607,   608,   608,   609,   610,   610,
     611,   611,   612,   613,   613,   614,   614,   615,   615,   616,
     617,   617,   618,   618,   618,   618,   618,   618,   619,   619,
     619,   619,   619,   619,   619,   619,   619,   620,   620,   620,
     620,   620,   621,   621,   622,   622,   623,   624,   625,   625,
     626,   626,   626,   627,   627,   627,   628,   628,   629,   630,
     631,   632,   632,   632,   633,   633,   634,   635,   635,   636,
     637,   637,   637,   637,   637,   638,   639,   640,   640,   641,
     641,   641,   641,   641,   641,   641,   641,   642,   643,   643,
     644,   644,   644,   644,   644,   644,   645,   645,   646,   647,
     647,   648,   649,   649,   649,   649,   650,   650,   651,   652,
     652,   653,   653,   654,   654,   654,   654,   654,   654,   654,
     654,   654,   654,   654,   654,   654,   654,   654,   655,   655,
     655,   656,   656,   657,   657,   658,   658,   659,   660,   660,
     661,   661,   662,   662,   663,   664,   665,   665,   666,   666,
     666,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   668,   668,   669,   670,   670,
     670,   671,   671,   672,   673,   673,   673,   673,   673,   674,
     674,   675,   675,   676,   676,   677,   677,   677,   678,   678,
     678,   678,   679,   679,   680,   681,   681,   682,   682,   683,
     683,   684,   684,   684,   685,   685,   685,   685,   686,   686,
     687,   687,   688,   688,   689,   690,   690,   690,   691,   691,
     691,   692,   692,   693,   693,   694,   695,   696,   696,   697,
     697,   698,   699,   700,   700,   700,   700,   700,   700,   700,
     700,   700,   700,   700,   700,   700,   701,   701,   701,   701,
     702,   703,   703,   703,   703,   704,   704,   704,   704,   704,
     705,   705,   706,   706,   707,   707,   707,   708,   708,   708,
     709,   709,   710,   710,   711,   711,   712,   713,   713,   714,
     715,   716,   716,   717,   718,   718,   718,   719,   720,   720,
     720,   721,   721,   721,   722,   722,   723,   724,   724,   724,
     725,   725,   726,   726,   727,   727,   728,   729,   729,   729,
     730,   730,   731,   731,   732,   732,   732,   732,   732,   733,
     733,   733,   734,   735,   735,   736,   736,   736,   736,   737,
     738,   739,   740,   741,   742,   743,   743,   743,   744,   744,
     744,   745,   745,   746,   746,   747,   747,   748,   749,   749,
     749,   750,   750,   750,   750,   750,   751,   752,   752,   753,
     754,   754,   754,   754,   755,   756,   756,   756,   756,   756,
     757,   757,   757,   757,   758,   759,   760,   760,   760,   760,
     761,   762,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   764,   764,   764,   764,   764,   765,
     766,   766,   766,   766,   766,   766,   766,   766,   767,   767,
     768,   768,   768,   769,   769,   769,   770,   770,   771,   771,
     772,   772,   773,   774,   774,   774,   774,   775,   775,   776,
     777,   777,   777,   777,   777,   777,   777,   777,   777,   777,
     777,   777,   777,   777,   778,   778,   778,   778,   778,   778,
     778,   778,   778,   778,   778,   778,   778,   778,   778,   778,
     778,   778,   779,   779,   779,   779,   779,   779,   779,   780,
     780,   780,   780,   780,   780,   781,   781,   782,   782,   783,
     783,   784,   784,   784,   784,   784,   784,   784,   784,   784,
     784,   784,   784,   784,   784,   784,   785,   785,   785,   785,
     785,   785,   785,   785,   785,   785,   785,   785,   785,   785,
     785,   785,   785,   785,   785,   785,   785,   785,   786,   786,
     786,   786,   786,   787,   787,   788,   788,   788,   789,   789,
     790,   791,   791,   792,   792,   793,   794,   794,   795,   795,
     796,   796,   797,   797,   798,   798,   799,   799,   799,   800,
     800,   801,   801,   802,   802,   802,   802,   803,   803,   803,
     804,   804,   805,   805,   805,   805,   805,   805,   805,   805,
     805,   805,   805,   805,   805,   805,   805,   805,   805,   806,
     806,   806,   806,   806,   806,   806,   807,   807,   807,   807,
     808,   808,   808,   808,   809,   809,   810,   810,   811,   811,
     812,   813,   813,   813,   813,   813,   813,   813,   813,   813,
     813,   813,   813,   813,   813,   813,   813,   813,   813,   813,
     813,   813,   813,   814,   815,   816,   816,   816,   816,   817,
     818,   818,   818,   819,   820,   820,   820,   820,   820,   821,
     822,   822,   822,   822,   822,   822,   822,   822,   822,   823,
     823,   823,   824,   824,   824,   824,   824,   824,   824,   824,
     824,   824,   824,   824,   825,   825,   825,   825,   826,   826,
     826,   826,   826,   827,   828,   828,   828,   828,   828,   828,
     828,   829,   830,   830,   830,   830,   831,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   833,   834,   834,   834,
     834,   834,   834,   834,   834,   834,   834,   835,   836,   836,
     836,   836,   836,   837,   838,   838,   839,   840,   840,   840,
     841,   842,   842,   842,   842,   843,   844,   844,   844,   844,
     845,   845,   845,   845,   846,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   847,   847,
     847,   848,   848,   849,   849,   850,   850,   851,   851,   852,
     852,   853,   853,   854,   854,   854,   855,   856,   857,   857,
     857,   857,   857,   857,   857,   857,   857,   857,   858,   858,
     858,   858,   858,   858,   859,   859,   859,   859,   859,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   861,   861,   862,   863,   863,   864,   865,   866,   866,
     867,   867,   867,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   869,   869,   869,   870,   870,   870,   870,   871,   871,
     871,   871,   871,   872,   872,   872,   872,   873,   873,   873,
     873,   873,   873,   873,   873,   873,   873,   873,   873,   874,
     874,   875,   875,   875,   875,   876,   877,   877,   878,   878,
     878,   878,   878,   878,   878,   878,   879,   880,   880,   881,
     882,   882,   882,   882,   883,   884,   885,   886,   887,   888,
     888,   888,   888,   889,   890,   890,   890,   890,   890,   890,
     891,   892,   892,   893,   894,   894,   894,   894,   894,   895,
     896,   896,   896,   896,   896
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
       0,     0,     0,     0,     3,     5,     5,     3,     2,     1,
       1,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       3,     3,     2,     2,     3,     3,     3,     2,     3,     2,
       6,     2,     6,     3,     2,     6,     6,     3,     6,     3,
       5,     7,     5,     7,     8,     8,     8,     5,     7,     5,
       7,     5,     7,     7,     8,     3,     2,     6,     2,     6,
       6,     6,     3,     6,     3,     5,     5,     8,     8,     8,
       5,     5,     5,     7,     8,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     6,     2,     2,     2,
       3,     2,     2,     6,     3,     3,     5,     3,     3,     3,
       3,     4,     2,     2,     2,     2,     2,     3,     2,     2,
       6,     3,     3,     5,     3,     3,     3,     3,     4,     3,
       2,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     3,     2,     2,     2,     2,     2,     2,     4,
       5,     2,     2,     3,     4,     3,     2,     1,     2,     2,
       3,     2,     3,     2,     2,     2,     3,     2,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     3,     2,
       1,     2,     1,     3,     0,     1,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     2,
       1,     0,     2,     1,     1,     3,     1,     0,     2,     2,
       3,     8,     8,     8,     8,     9,     9,    10,    10,    10,
       9,     9,     9,     9,    10,     0,     0,     2,     2,     3,
       3,     3,     3,     5,     3,     0,     2,     3,     1,     3,
       2,     1,     1,     1,     1,     1,     3,     0,     2,     3,
       0,     2,     2,     3,     4,     4,     4,     3,     4,     2,
       3,     3,     1,     1,     3,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     2,     2,     2,     2,     2,     1,
       3,     1,     0,     0,     2,     2,     4,     4,     8,     6,
       7,     6,     4,     3,     4,     3,     3,     3,     2,     1,
       1,     0,     0,     2,     2,     5,     5,     3,     4,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     3,
       0,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     0,     2,     2,     2,
       2,     0,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     7,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     3,     3,     1,     2,     2,     2,     3,     0,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     2,     2,     1,     2,     1,
       1,     2,     3,     2,     3,     1,     2,     3,     1,     2,
       3,     1,     2,     3,     1,     2,     2,     2,     1,     2,
       2,     2,     2,     2,     0,     1,     1,     2,     1,     1,
       2,     1,     2,     4,     4,     4,     4,     4,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     2,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     1,     1,     2,     2,     2,     2,     1,     1,
       2,     1,     1,     2,     1,     3,     1,     1,     5,     1,
       1,     3,     3,     1,     1,     3,     3,     5,     2,     0,
       2,     2,     2,     1,     2,     1,     2,     1,     1,     2,
       2,     2,     1,     1,     2,     2,     2,     1,     2,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     1,
       2,     1,     3,     1,     1,     1,     2,     3,     3,     3,
       1,     1,     0,     1,     1,     1,     0,     3,     1,     2,
       4,     1,     1,     0,     0,     3,     3,     0,     3,     4,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     0,
       6,     3,     6,     3,     5,     3,     5,     2,     1,     1,
       3,     4,     4,     5,     6,     5,     1,     2,     1,     1,
       2,     2,     2,     1,     1,     1,     5,     7,     0,     0,
       1,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       3,     1,     3,     1,     3,     1,     3,     1,     1,     3,
       1,     1,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     2,     3,     3,     4,     5,     2,     3,     2,
       6,     4,     3,     4,     3,     2,     1,     1,     3,     4,
       1,     2,     1,     1,     2,     3,     1,     3,     4,     3,
       5,     3,     6,     1,     3,     1,     1,     1,     2,     4,
       6,     6,     1,     2,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     2,     1,     4,     5,     6,     1,     1,
       1,     7,     8,     6,     8,     1,     2,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       4,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       3,     1,     4,     4,     0,     2,     2,     1,     3,     3,
       1,     3,     1,     3,     1,     3,     1,     1,     3,     3,
       3,     1,     1,     3,     1,     1,     1,     3,     1,     3,
       3,     3,     3,     5,     1,     2,     1,     1,     2,     3,
       1,     1,     2,     1,     1,     2,     1,     2,     2,     1,
       1,     2,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       0,     4,     0,     1,     0,     1,     1,     1,     1,     1,
       1,     2,     2,     6,     3,     1,     3,     3,     3,     7,
       3,     3,     3,     3,     3,     3,     0,     5,     5,     0,
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
       1,     1,     2,     2,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     1,     5,     3,
       1,     5,     5,     3,     5,     1,     1,     1,     1,     1,
       2,     0,     2,     1,     1,     6,     2,     0,     1,     1,
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
        YY_LAC_DISCARD ("YYBACKUP");                              \
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


/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return YYENOMEM if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYPTRDIFF_T *yycapacity, YYPTRDIFF_T yyadd,
#if YYDEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yy_state_t **yybottom,
                      yy_state_t *yybottom_no_free,
                      yy_state_t **yytop, yy_state_t *yytop_empty)
{
  YYPTRDIFF_T yysize_old =
    *yytop == yytop_empty ? 0 : *yytop - *yybottom + 1;
  YYPTRDIFF_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYPTRDIFF_T yyalloc = 2 * yysize_new;
      yy_state_t *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        YY_CAST (yy_state_t *,
                 YYSTACK_ALLOC (YY_CAST (YYSIZE_T,
                                         yyalloc * YYSIZEOF (*yybottom_new))));
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                                \
do {                                                                    \
  if (!yy_lac_established)                                              \
    {                                                                   \
      YYDPRINTF ((stderr,                                               \
                  "LAC: initial context established for %s\n",          \
                  yysymbol_name (yytoken)));                            \
      yy_lac_established = 1;                                           \
      switch (yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken))    \
        {                                                               \
        case YYENOMEM:                                                  \
          YYNOMEM;                                                      \
        case 1:                                                         \
          goto yyerrlab;                                                \
        }                                                               \
    }                                                                   \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if YYDEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      YYDPRINTF ((stderr, "LAC: initial context discarded due to "       \
                  Event "\n"));                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return YYENOMEM if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yy_state_t *yyesa, yy_state_t **yyes,
        YYPTRDIFF_T *yyes_capacity, yy_state_t *yyssp, yysymbol_kind_t yytoken)
{
  yy_state_t *yyes_prev = yyssp;
  yy_state_t *yyesp = yyes_prev;
  /* Reduce until we encounter a shift and thereby accept the token.  */
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yysymbol_name (yytoken)));
  if (yytoken == YYSYMBOL_YYUNDEF)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[+*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          /* Use the default action.  */
          yyrule = yydefact[+*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          /* Use the action from yytable.  */
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      /* By now we know we have to simulate a reduce.  */
      YYDPRINTF ((stderr, " R%d", yyrule - 1));
      {
        /* Pop the corresponding number of values from the stack.  */
        YYPTRDIFF_T yylen = yyr2[yyrule];
        /* First pop from the LAC stack as many tokens as possible.  */
        if (yyesp != yyes_prev)
          {
            YYPTRDIFF_T yysize = yyesp - *yyes + 1;
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yyesp = yyes_prev;
                yylen -= yysize;
              }
          }
        /* Only afterwards look at the main stack.  */
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      /* Push the resulting state of the reduction.  */
      {
        yy_state_fast_t yystate;
        {
          const int yylhs = yyr1[yyrule] - YYNTOKENS;
          const int yyi = yypgoto[yylhs] + *yyesp;
          yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyesp
                     ? yytable[yyi]
                     : yydefgoto[yylhs]);
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            YY_IGNORE_USELESS_CAST_BEGIN
            *yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if YYDEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return YYENOMEM;
              }
            YY_IGNORE_USELESS_CAST_BEGIN
            *++yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        YYDPRINTF ((stderr, " G%d", yystate));
      }
    }
}

/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yy_state_t *yyesa;
  yy_state_t **yyes;
  YYPTRDIFF_T *yyes_capacity;
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

  int yyx;
  for (yyx = 0; yyx < YYNTOKENS; ++yyx)
    {
      yysymbol_kind_t yysym = YY_CAST (yysymbol_kind_t, yyx);
      if (yysym != YYSYMBOL_YYerror && yysym != YYSYMBOL_YYUNDEF)
        switch (yy_lac (yyctx->yyesa, yyctx->yyes, yyctx->yyes_capacity, yyctx->yyssp, yysym))
          {
          case YYENOMEM:
            return YYENOMEM;
          case 1:
            continue;
          default:
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}

/* The location of the lookahead of this context.  */
static YYLTYPE *
yypcontext_location (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static YYLTYPE *
yypcontext_location (const yypcontext_t *yyctx)
{
  return yyctx->yylloc;
}

/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx, struct nft_ctx *nft, void *scanner, struct parser_state *state);

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
#line 6691 "src/parser_bison.c"
        break;

    case YYSYMBOL_QUOTED_STRING: /* "quoted string"  */
#line 395 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6697 "src/parser_bison.c"
        break;

    case YYSYMBOL_ASTERISK_STRING: /* "string with a trailing asterisk"  */
#line 395 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6703 "src/parser_bison.c"
        break;

    case YYSYMBOL_line: /* line  */
#line 757 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6709 "src/parser_bison.c"
        break;

    case YYSYMBOL_base_cmd: /* base_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6715 "src/parser_bison.c"
        break;

    case YYSYMBOL_add_cmd: /* add_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6721 "src/parser_bison.c"
        break;

    case YYSYMBOL_replace_cmd: /* replace_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6727 "src/parser_bison.c"
        break;

    case YYSYMBOL_create_cmd: /* create_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6733 "src/parser_bison.c"
        break;

    case YYSYMBOL_insert_cmd: /* insert_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6739 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_or_id_spec: /* table_or_id_spec  */
#line 763 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6745 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_or_id_spec: /* chain_or_id_spec  */
#line 765 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6751 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_or_id_spec: /* set_or_id_spec  */
#line 770 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6757 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_or_id_spec: /* obj_or_id_spec  */
#line 772 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6763 "src/parser_bison.c"
        break;

    case YYSYMBOL_delete_cmd: /* delete_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6769 "src/parser_bison.c"
        break;

    case YYSYMBOL_destroy_cmd: /* destroy_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6775 "src/parser_bison.c"
        break;

    case YYSYMBOL_get_cmd: /* get_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6781 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_table: /* list_cmd_spec_table  */
#line 781 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6787 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd_spec_any: /* list_cmd_spec_any  */
#line 781 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6793 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_cmd: /* list_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6799 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_device_name: /* basehook_device_name  */
#line 789 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6805 "src/parser_bison.c"
        break;

    case YYSYMBOL_basehook_spec: /* basehook_spec  */
#line 778 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 6811 "src/parser_bison.c"
        break;

    case YYSYMBOL_reset_cmd: /* reset_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6817 "src/parser_bison.c"
        break;

    case YYSYMBOL_flush_cmd: /* flush_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6823 "src/parser_bison.c"
        break;

    case YYSYMBOL_rename_cmd: /* rename_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6829 "src/parser_bison.c"
        break;

    case YYSYMBOL_import_cmd: /* import_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6835 "src/parser_bison.c"
        break;

    case YYSYMBOL_export_cmd: /* export_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6841 "src/parser_bison.c"
        break;

    case YYSYMBOL_monitor_cmd: /* monitor_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6847 "src/parser_bison.c"
        break;

    case YYSYMBOL_describe_cmd: /* describe_cmd  */
#line 760 "src/parser_bison.y"
            { cmd_free(((*yyvaluep).cmd)); }
#line 6853 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_block_alloc: /* table_block_alloc  */
#line 795 "src/parser_bison.y"
            { close_scope(state); table_free(((*yyvaluep).table)); }
#line 6859 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_block_alloc: /* chain_block_alloc  */
#line 797 "src/parser_bison.y"
            { close_scope(state); chain_free(((*yyvaluep).chain)); }
#line 6865 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_verdict_expr: /* typeof_verdict_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6871 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_data_expr: /* typeof_data_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6877 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_typeof_expr: /* primary_typeof_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6883 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_expr: /* typeof_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6889 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_alloc: /* set_block_alloc  */
#line 808 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6895 "src/parser_bison.c"
        break;

    case YYSYMBOL_typeof_key_expr: /* typeof_key_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6901 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_block_expr: /* set_block_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6907 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_block_alloc: /* map_block_alloc  */
#line 811 "src/parser_bison.y"
            { set_free(((*yyvaluep).set)); }
#line 6913 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_block_alloc: /* flowtable_block_alloc  */
#line 815 "src/parser_bison.y"
            { flowtable_free(((*yyvaluep).flowtable)); }
#line 6919 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr: /* flowtable_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6925 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_list_expr: /* flowtable_list_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6931 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_expr_member: /* flowtable_expr_member  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6937 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_atom_expr: /* data_type_atom_expr  */
#line 754 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6943 "src/parser_bison.c"
        break;

    case YYSYMBOL_data_type_expr: /* data_type_expr  */
#line 754 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6949 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_block_alloc: /* obj_block_alloc  */
#line 819 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 6955 "src/parser_bison.c"
        break;

    case YYSYMBOL_type_identifier: /* type_identifier  */
#line 749 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6961 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_type: /* chain_type  */
#line 1066 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6967 "src/parser_bison.c"
        break;

    case YYSYMBOL_prio_spec: /* prio_spec  */
#line 786 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6973 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_name: /* extended_prio_name  */
#line 789 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 6979 "src/parser_bison.c"
        break;

    case YYSYMBOL_extended_prio_spec: /* extended_prio_spec  */
#line 786 "src/parser_bison.y"
            { expr_free(((*yyvaluep).prio_spec).expr); }
#line 6985 "src/parser_bison.c"
        break;

    case YYSYMBOL_dev_spec: /* dev_spec  */
#line 792 "src/parser_bison.y"
            { free(((*yyvaluep).expr)); }
#line 6991 "src/parser_bison.c"
        break;

    case YYSYMBOL_policy_expr: /* policy_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 6997 "src/parser_bison.c"
        break;

    case YYSYMBOL_identifier: /* identifier  */
#line 749 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 7003 "src/parser_bison.c"
        break;

    case YYSYMBOL_string: /* string  */
#line 749 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 7009 "src/parser_bison.c"
        break;

    case YYSYMBOL_table_spec: /* table_spec  */
#line 763 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7015 "src/parser_bison.c"
        break;

    case YYSYMBOL_tableid_spec: /* tableid_spec  */
#line 763 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7021 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_spec: /* chain_spec  */
#line 765 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7027 "src/parser_bison.c"
        break;

    case YYSYMBOL_chainid_spec: /* chainid_spec  */
#line 765 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7033 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_identifier: /* chain_identifier  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7039 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_spec: /* set_spec  */
#line 770 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7045 "src/parser_bison.c"
        break;

    case YYSYMBOL_setid_spec: /* setid_spec  */
#line 770 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7051 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_identifier: /* set_identifier  */
#line 775 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7057 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_spec: /* flowtable_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7063 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtableid_spec: /* flowtableid_spec  */
#line 775 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7069 "src/parser_bison.c"
        break;

    case YYSYMBOL_flowtable_identifier: /* flowtable_identifier  */
#line 775 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7075 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_spec: /* obj_spec  */
#line 772 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7081 "src/parser_bison.c"
        break;

    case YYSYMBOL_objid_spec: /* objid_spec  */
#line 772 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7087 "src/parser_bison.c"
        break;

    case YYSYMBOL_obj_identifier: /* obj_identifier  */
#line 775 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7093 "src/parser_bison.c"
        break;

    case YYSYMBOL_handle_spec: /* handle_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7099 "src/parser_bison.c"
        break;

    case YYSYMBOL_position_spec: /* position_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7105 "src/parser_bison.c"
        break;

    case YYSYMBOL_index_spec: /* index_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7111 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule_position: /* rule_position  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7117 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleid_spec: /* ruleid_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7123 "src/parser_bison.c"
        break;

    case YYSYMBOL_comment_spec: /* comment_spec  */
#line 749 "src/parser_bison.y"
            { free_const(((*yyvaluep).string)); }
#line 7129 "src/parser_bison.c"
        break;

    case YYSYMBOL_ruleset_spec: /* ruleset_spec  */
#line 768 "src/parser_bison.y"
            { handle_free(&((*yyvaluep).handle)); }
#line 7135 "src/parser_bison.c"
        break;

    case YYSYMBOL_rule: /* rule  */
#line 799 "src/parser_bison.y"
            { rule_free(((*yyvaluep).rule)); }
#line 7141 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_list: /* stmt_list  */
#line 822 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7147 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt_list: /* stateful_stmt_list  */
#line 822 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7153 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_counter: /* objref_stmt_counter  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7159 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_limit: /* objref_stmt_limit  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7165 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_quota: /* objref_stmt_quota  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7171 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_synproxy: /* objref_stmt_synproxy  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7177 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_tunnel: /* objref_stmt_tunnel  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7183 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt_ct: /* objref_stmt_ct  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7189 "src/parser_bison.c"
        break;

    case YYSYMBOL_objref_stmt: /* objref_stmt  */
#line 830 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7195 "src/parser_bison.c"
        break;

    case YYSYMBOL_stateful_stmt: /* stateful_stmt  */
#line 826 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7201 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 824 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7207 "src/parser_bison.c"
        break;

    case YYSYMBOL_xt_stmt: /* xt_stmt  */
#line 1040 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7213 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_stmt: /* chain_stmt  */
#line 854 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7219 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_stmt: /* verdict_stmt  */
#line 824 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7225 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_stmt: /* verdict_map_stmt  */
#line 912 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7231 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_expr: /* verdict_map_expr  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7237 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_expr: /* verdict_map_list_expr  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7243 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_map_list_member_expr: /* verdict_map_list_member_expr  */
#line 915 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7249 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_limit_stmt_alloc: /* ct_limit_stmt_alloc  */
#line 828 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7255 "src/parser_bison.c"
        break;

    case YYSYMBOL_connlimit_obj: /* connlimit_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7261 "src/parser_bison.c"
        break;

    case YYSYMBOL_connlimit_stmt: /* connlimit_stmt  */
#line 842 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7267 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt: /* counter_stmt  */
#line 826 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7273 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_stmt_alloc: /* counter_stmt_alloc  */
#line 826 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7279 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt_alloc: /* last_stmt_alloc  */
#line 828 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7285 "src/parser_bison.c"
        break;

    case YYSYMBOL_last_stmt: /* last_stmt  */
#line 826 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7291 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt: /* log_stmt  */
#line 839 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7297 "src/parser_bison.c"
        break;

    case YYSYMBOL_log_stmt_alloc: /* log_stmt_alloc  */
#line 839 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7303 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt_alloc: /* limit_stmt_alloc  */
#line 828 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7309 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_stmt: /* limit_stmt  */
#line 842 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7315 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt_alloc: /* quota_stmt_alloc  */
#line 828 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7321 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_stmt: /* quota_stmt  */
#line 842 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7327 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt: /* reject_stmt  */
#line 845 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7333 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_stmt_alloc: /* reject_stmt_alloc  */
#line 845 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7339 "src/parser_bison.c"
        break;

    case YYSYMBOL_reject_with_expr: /* reject_with_expr  */
#line 860 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7345 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt: /* nat_stmt  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7351 "src/parser_bison.c"
        break;

    case YYSYMBOL_nat_stmt_alloc: /* nat_stmt_alloc  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7357 "src/parser_bison.c"
        break;

    case YYSYMBOL_tproxy_stmt: /* tproxy_stmt  */
#line 850 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7363 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt: /* synproxy_stmt  */
#line 852 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7369 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_stmt_alloc: /* synproxy_stmt_alloc  */
#line 852 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7375 "src/parser_bison.c"
        break;

    case YYSYMBOL_synproxy_obj: /* synproxy_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7381 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_stmt_expr: /* primary_stmt_expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7387 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_stmt_expr: /* shift_stmt_expr  */
#line 901 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7393 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_stmt_expr: /* and_stmt_expr  */
#line 903 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7399 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_stmt_expr: /* exclusive_or_stmt_expr  */
#line 903 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7405 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_stmt_expr: /* inclusive_or_stmt_expr  */
#line 903 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7411 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_stmt_expr: /* basic_stmt_expr  */
#line 899 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7417 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_stmt_expr: /* concat_stmt_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7423 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr_set: /* map_stmt_expr_set  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7429 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt_expr: /* map_stmt_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7435 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_stmt_expr: /* prefix_stmt_expr  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7441 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_stmt_expr: /* range_stmt_expr  */
#line 896 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7447 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_stmt_expr: /* multiton_stmt_expr  */
#line 894 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7453 "src/parser_bison.c"
        break;

    case YYSYMBOL_stmt_expr: /* stmt_expr  */
#line 891 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7459 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt: /* masq_stmt  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7465 "src/parser_bison.c"
        break;

    case YYSYMBOL_masq_stmt_alloc: /* masq_stmt_alloc  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7471 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt: /* redir_stmt  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7477 "src/parser_bison.c"
        break;

    case YYSYMBOL_redir_stmt_alloc: /* redir_stmt_alloc  */
#line 847 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7483 "src/parser_bison.c"
        break;

    case YYSYMBOL_dup_stmt: /* dup_stmt  */
#line 863 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7489 "src/parser_bison.c"
        break;

    case YYSYMBOL_fwd_stmt: /* fwd_stmt  */
#line 865 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7495 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt: /* queue_stmt  */
#line 858 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7501 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_compat: /* queue_stmt_compat  */
#line 858 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7507 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_alloc: /* queue_stmt_alloc  */
#line 858 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7513 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_expr: /* queue_expr  */
#line 860 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7519 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr_simple: /* queue_stmt_expr_simple  */
#line 860 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7525 "src/parser_bison.c"
        break;

    case YYSYMBOL_queue_stmt_expr: /* queue_stmt_expr  */
#line 860 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7531 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt: /* set_elem_expr_stmt  */
#line 922 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7537 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_stmt_alloc: /* set_elem_expr_stmt_alloc  */
#line 922 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7543 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_stmt: /* set_stmt  */
#line 867 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7549 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_stmt: /* map_stmt  */
#line 870 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7555 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_stmt: /* meter_stmt  */
#line 872 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7561 "src/parser_bison.c"
        break;

    case YYSYMBOL_match_stmt: /* match_stmt  */
#line 824 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7567 "src/parser_bison.c"
        break;

    case YYSYMBOL_variable_expr: /* variable_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7573 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_expr: /* symbol_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7579 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_expr: /* set_ref_expr  */
#line 883 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7585 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_ref_symbol_expr: /* set_ref_symbol_expr  */
#line 883 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7591 "src/parser_bison.c"
        break;

    case YYSYMBOL_integer_expr: /* integer_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7597 "src/parser_bison.c"
        break;

    case YYSYMBOL_selector_expr: /* selector_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7603 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_expr: /* primary_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7609 "src/parser_bison.c"
        break;

    case YYSYMBOL_fib_expr: /* fib_expr  */
#line 1009 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7615 "src/parser_bison.c"
        break;

    case YYSYMBOL_osf_expr: /* osf_expr  */
#line 1014 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7621 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_expr: /* shift_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7627 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_expr: /* and_expr  */
#line 877 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7633 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_expr: /* exclusive_or_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7639 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_expr: /* inclusive_or_expr  */
#line 879 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7645 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_expr: /* basic_expr  */
#line 881 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7651 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_expr: /* concat_expr  */
#line 906 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7657 "src/parser_bison.c"
        break;

    case YYSYMBOL_prefix_rhs_expr: /* prefix_rhs_expr  */
#line 888 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7663 "src/parser_bison.c"
        break;

    case YYSYMBOL_range_rhs_expr: /* range_rhs_expr  */
#line 888 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7669 "src/parser_bison.c"
        break;

    case YYSYMBOL_multiton_rhs_expr: /* multiton_rhs_expr  */
#line 886 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7675 "src/parser_bison.c"
        break;

    case YYSYMBOL_map_expr: /* map_expr  */
#line 909 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7681 "src/parser_bison.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 928 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7687 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_expr: /* set_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7693 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_expr: /* set_list_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7699 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_list_member_expr: /* set_list_member_expr  */
#line 918 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7705 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr: /* meter_key_expr  */
#line 925 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7711 "src/parser_bison.c"
        break;

    case YYSYMBOL_meter_key_expr_alloc: /* meter_key_expr_alloc  */
#line 925 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7717 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr: /* set_elem_expr  */
#line 920 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7723 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_key_expr: /* set_elem_key_expr  */
#line 1063 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7729 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_expr_alloc: /* set_elem_expr_alloc  */
#line 920 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7735 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt_list: /* set_elem_stmt_list  */
#line 822 "src/parser_bison.y"
            { stmt_list_free(((*yyvaluep).list)); free(((*yyvaluep).list)); }
#line 7741 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_elem_stmt: /* set_elem_stmt  */
#line 824 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7747 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_lhs_expr: /* set_lhs_expr  */
#line 920 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7753 "src/parser_bison.c"
        break;

    case YYSYMBOL_set_rhs_expr: /* set_rhs_expr  */
#line 920 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7759 "src/parser_bison.c"
        break;

    case YYSYMBOL_initializer_expr: /* initializer_expr  */
#line 928 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7765 "src/parser_bison.c"
        break;

    case YYSYMBOL_counter_obj: /* counter_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7771 "src/parser_bison.c"
        break;

    case YYSYMBOL_quota_obj: /* quota_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7777 "src/parser_bison.c"
        break;

    case YYSYMBOL_secmark_obj: /* secmark_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7783 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_states: /* timeout_states  */
#line 1056 "src/parser_bison.y"
            { timeout_states_free(((*yyvaluep).list)); }
#line 7789 "src/parser_bison.c"
        break;

    case YYSYMBOL_timeout_state: /* timeout_state  */
#line 1053 "src/parser_bison.y"
            { timeout_state_free(((*yyvaluep).timeout_state)); }
#line 7795 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_obj_alloc: /* ct_obj_alloc  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7801 "src/parser_bison.c"
        break;

    case YYSYMBOL_limit_obj: /* limit_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7807 "src/parser_bison.c"
        break;

    case YYSYMBOL_tunnel_obj: /* tunnel_obj  */
#line 938 "src/parser_bison.y"
            { obj_free(((*yyvaluep).obj)); }
#line 7813 "src/parser_bison.c"
        break;

    case YYSYMBOL_relational_expr: /* relational_expr  */
#line 941 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7819 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_rhs_expr: /* list_rhs_expr  */
#line 933 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7825 "src/parser_bison.c"
        break;

    case YYSYMBOL_rhs_expr: /* rhs_expr  */
#line 931 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7831 "src/parser_bison.c"
        break;

    case YYSYMBOL_shift_rhs_expr: /* shift_rhs_expr  */
#line 933 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7837 "src/parser_bison.c"
        break;

    case YYSYMBOL_and_rhs_expr: /* and_rhs_expr  */
#line 935 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7843 "src/parser_bison.c"
        break;

    case YYSYMBOL_exclusive_or_rhs_expr: /* exclusive_or_rhs_expr  */
#line 935 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7849 "src/parser_bison.c"
        break;

    case YYSYMBOL_inclusive_or_rhs_expr: /* inclusive_or_rhs_expr  */
#line 935 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7855 "src/parser_bison.c"
        break;

    case YYSYMBOL_basic_rhs_expr: /* basic_rhs_expr  */
#line 931 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7861 "src/parser_bison.c"
        break;

    case YYSYMBOL_concat_rhs_expr: /* concat_rhs_expr  */
#line 931 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7867 "src/parser_bison.c"
        break;

    case YYSYMBOL_boolean_expr: /* boolean_expr  */
#line 1043 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7873 "src/parser_bison.c"
        break;

    case YYSYMBOL_keyword_expr: /* keyword_expr  */
#line 928 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7879 "src/parser_bison.c"
        break;

    case YYSYMBOL_primary_rhs_expr: /* primary_rhs_expr  */
#line 933 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7885 "src/parser_bison.c"
        break;

    case YYSYMBOL_verdict_expr: /* verdict_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7891 "src/parser_bison.c"
        break;

    case YYSYMBOL_chain_expr: /* chain_expr  */
#line 875 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7897 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_expr: /* meta_expr  */
#line 991 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7903 "src/parser_bison.c"
        break;

    case YYSYMBOL_meta_stmt: /* meta_stmt  */
#line 837 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7909 "src/parser_bison.c"
        break;

    case YYSYMBOL_socket_expr: /* socket_expr  */
#line 995 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7915 "src/parser_bison.c"
        break;

    case YYSYMBOL_tunnel_expr: /* tunnel_expr  */
#line 991 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7921 "src/parser_bison.c"
        break;

    case YYSYMBOL_numgen_expr: /* numgen_expr  */
#line 956 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7927 "src/parser_bison.c"
        break;

    case YYSYMBOL_xfrm_expr: /* xfrm_expr  */
#line 1060 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7933 "src/parser_bison.c"
        break;

    case YYSYMBOL_hash_expr: /* hash_expr  */
#line 956 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7939 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_expr: /* rt_expr  */
#line 1001 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7945 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_expr: /* ct_expr  */
#line 1005 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7951 "src/parser_bison.c"
        break;

    case YYSYMBOL_symbol_stmt_expr: /* symbol_stmt_expr  */
#line 933 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7957 "src/parser_bison.c"
        break;

    case YYSYMBOL_list_stmt_expr: /* list_stmt_expr  */
#line 901 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7963 "src/parser_bison.c"
        break;

    case YYSYMBOL_ct_stmt: /* ct_stmt  */
#line 835 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7969 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_stmt: /* payload_stmt  */
#line 833 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 7975 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_expr: /* payload_expr  */
#line 945 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7981 "src/parser_bison.c"
        break;

    case YYSYMBOL_payload_raw_expr: /* payload_raw_expr  */
#line 945 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7987 "src/parser_bison.c"
        break;

    case YYSYMBOL_eth_hdr_expr: /* eth_hdr_expr  */
#line 950 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7993 "src/parser_bison.c"
        break;

    case YYSYMBOL_vlan_hdr_expr: /* vlan_hdr_expr  */
#line 950 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 7999 "src/parser_bison.c"
        break;

    case YYSYMBOL_arp_hdr_expr: /* arp_hdr_expr  */
#line 953 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8005 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip_hdr_expr: /* ip_hdr_expr  */
#line 956 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8011 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp_hdr_expr: /* icmp_hdr_expr  */
#line 956 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8017 "src/parser_bison.c"
        break;

    case YYSYMBOL_igmp_hdr_expr: /* igmp_hdr_expr  */
#line 956 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8023 "src/parser_bison.c"
        break;

    case YYSYMBOL_ip6_hdr_expr: /* ip6_hdr_expr  */
#line 960 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8029 "src/parser_bison.c"
        break;

    case YYSYMBOL_icmp6_hdr_expr: /* icmp6_hdr_expr  */
#line 960 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8035 "src/parser_bison.c"
        break;

    case YYSYMBOL_auth_hdr_expr: /* auth_hdr_expr  */
#line 963 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8041 "src/parser_bison.c"
        break;

    case YYSYMBOL_esp_hdr_expr: /* esp_hdr_expr  */
#line 963 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8047 "src/parser_bison.c"
        break;

    case YYSYMBOL_comp_hdr_expr: /* comp_hdr_expr  */
#line 963 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8053 "src/parser_bison.c"
        break;

    case YYSYMBOL_udp_hdr_expr: /* udp_hdr_expr  */
#line 966 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8059 "src/parser_bison.c"
        break;

    case YYSYMBOL_udplite_hdr_expr: /* udplite_hdr_expr  */
#line 966 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8065 "src/parser_bison.c"
        break;

    case YYSYMBOL_tcp_hdr_expr: /* tcp_hdr_expr  */
#line 1022 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8071 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_inet_expr: /* inner_inet_expr  */
#line 1030 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8077 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_eth_expr: /* inner_eth_expr  */
#line 1030 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8083 "src/parser_bison.c"
        break;

    case YYSYMBOL_inner_expr: /* inner_expr  */
#line 1030 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8089 "src/parser_bison.c"
        break;

    case YYSYMBOL_vxlan_hdr_expr: /* vxlan_hdr_expr  */
#line 1033 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8095 "src/parser_bison.c"
        break;

    case YYSYMBOL_geneve_hdr_expr: /* geneve_hdr_expr  */
#line 1033 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8101 "src/parser_bison.c"
        break;

    case YYSYMBOL_gre_hdr_expr: /* gre_hdr_expr  */
#line 1033 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8107 "src/parser_bison.c"
        break;

    case YYSYMBOL_gretap_hdr_expr: /* gretap_hdr_expr  */
#line 1033 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8113 "src/parser_bison.c"
        break;

    case YYSYMBOL_optstrip_stmt: /* optstrip_stmt  */
#line 1037 "src/parser_bison.y"
            { stmt_free(((*yyvaluep).stmt)); }
#line 8119 "src/parser_bison.c"
        break;

    case YYSYMBOL_dccp_hdr_expr: /* dccp_hdr_expr  */
#line 969 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8125 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_chunk_alloc: /* sctp_chunk_alloc  */
#line 969 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8131 "src/parser_bison.c"
        break;

    case YYSYMBOL_sctp_hdr_expr: /* sctp_hdr_expr  */
#line 969 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8137 "src/parser_bison.c"
        break;

    case YYSYMBOL_th_hdr_expr: /* th_hdr_expr  */
#line 975 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8143 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_expr: /* exthdr_expr  */
#line 979 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8149 "src/parser_bison.c"
        break;

    case YYSYMBOL_hbh_hdr_expr: /* hbh_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8155 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt_hdr_expr: /* rt_hdr_expr  */
#line 984 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8161 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt0_hdr_expr: /* rt0_hdr_expr  */
#line 984 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8167 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt2_hdr_expr: /* rt2_hdr_expr  */
#line 984 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8173 "src/parser_bison.c"
        break;

    case YYSYMBOL_rt4_hdr_expr: /* rt4_hdr_expr  */
#line 984 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8179 "src/parser_bison.c"
        break;

    case YYSYMBOL_frag_hdr_expr: /* frag_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8185 "src/parser_bison.c"
        break;

    case YYSYMBOL_dst_hdr_expr: /* dst_hdr_expr  */
#line 981 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8191 "src/parser_bison.c"
        break;

    case YYSYMBOL_mh_hdr_expr: /* mh_hdr_expr  */
#line 987 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8197 "src/parser_bison.c"
        break;

    case YYSYMBOL_exthdr_exists_expr: /* exthdr_exists_expr  */
#line 1047 "src/parser_bison.y"
            { expr_free(((*yyvaluep).expr)); }
#line 8203 "src/parser_bison.c"
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

    yy_state_t yyesa[20];
    yy_state_t *yyes = yyesa;
    YYPTRDIFF_T yyes_capacity = 20 < YYMAXDEPTH ? 20 : YYMAXDEPTH;

  /* Whether LAC context is established.  A Boolean.  */
  int yy_lac_established = 0;
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



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */


/* User initialization code.  */
#line 226 "src/parser_bison.y"
{
	location_init(scanner, state, &yylloc);
	if (nft->debug_mask & NFT_DEBUG_SCANNER)
		nft_set_debug(1, scanner);
	if (nft->debug_mask & NFT_DEBUG_PARSER)
		yydebug = 1;
}

#line 8312 "src/parser_bison.c"

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
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      YY_LAC_ESTABLISH;
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
  YY_LAC_DISCARD ("shift");
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
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
  case 3: /* input: input line  */
#line 1072 "src/parser_bison.y"
                        {
				if ((yyvsp[0].cmd) != NULL) {
					(yyvsp[0].cmd)->location = (yylsp[0]);
					list_add_tail(&(yyvsp[0].cmd)->list, state->cmds);
				}
			}
#line 8537 "src/parser_bison.c"
    break;

  case 8: /* close_scope_ah: %empty  */
#line 1088 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_AH); }
#line 8543 "src/parser_bison.c"
    break;

  case 9: /* close_scope_arp: %empty  */
#line 1089 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ARP); }
#line 8549 "src/parser_bison.c"
    break;

  case 10: /* close_scope_at: %empty  */
#line 1090 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_AT); }
#line 8555 "src/parser_bison.c"
    break;

  case 11: /* close_scope_comp: %empty  */
#line 1091 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_COMP); }
#line 8561 "src/parser_bison.c"
    break;

  case 12: /* close_scope_ct: %empty  */
#line 1092 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CT); }
#line 8567 "src/parser_bison.c"
    break;

  case 13: /* close_scope_counter: %empty  */
#line 1093 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_COUNTER); }
#line 8573 "src/parser_bison.c"
    break;

  case 14: /* close_scope_last: %empty  */
#line 1094 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LAST); }
#line 8579 "src/parser_bison.c"
    break;

  case 15: /* close_scope_dccp: %empty  */
#line 1095 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DCCP); }
#line 8585 "src/parser_bison.c"
    break;

  case 16: /* close_scope_destroy: %empty  */
#line 1096 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_DESTROY); }
#line 8591 "src/parser_bison.c"
    break;

  case 17: /* close_scope_dst: %empty  */
#line 1097 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_DST); }
#line 8597 "src/parser_bison.c"
    break;

  case 18: /* close_scope_dup: %empty  */
#line 1098 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_DUP); }
#line 8603 "src/parser_bison.c"
    break;

  case 19: /* close_scope_esp: %empty  */
#line 1099 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_ESP); }
#line 8609 "src/parser_bison.c"
    break;

  case 20: /* close_scope_eth: %empty  */
#line 1100 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ETH); }
#line 8615 "src/parser_bison.c"
    break;

  case 21: /* close_scope_export: %empty  */
#line 1101 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_EXPORT); }
#line 8621 "src/parser_bison.c"
    break;

  case 22: /* close_scope_fib: %empty  */
#line 1102 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FIB); }
#line 8627 "src/parser_bison.c"
    break;

  case 23: /* close_scope_frag: %empty  */
#line 1103 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_FRAG); }
#line 8633 "src/parser_bison.c"
    break;

  case 24: /* close_scope_fwd: %empty  */
#line 1104 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_FWD); }
#line 8639 "src/parser_bison.c"
    break;

  case 25: /* close_scope_gre: %empty  */
#line 1105 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_GRE); }
#line 8645 "src/parser_bison.c"
    break;

  case 26: /* close_scope_hash: %empty  */
#line 1106 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HASH); }
#line 8651 "src/parser_bison.c"
    break;

  case 27: /* close_scope_hbh: %empty  */
#line 1107 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_HBH); }
#line 8657 "src/parser_bison.c"
    break;

  case 28: /* close_scope_ip: %empty  */
#line 1108 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP); }
#line 8663 "src/parser_bison.c"
    break;

  case 29: /* close_scope_ip6: %empty  */
#line 1109 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IP6); }
#line 8669 "src/parser_bison.c"
    break;

  case 30: /* close_scope_vlan: %empty  */
#line 1110 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_VLAN); }
#line 8675 "src/parser_bison.c"
    break;

  case 31: /* close_scope_icmp: %empty  */
#line 1111 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_ICMP); }
#line 8681 "src/parser_bison.c"
    break;

  case 32: /* close_scope_igmp: %empty  */
#line 1112 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_IGMP); }
#line 8687 "src/parser_bison.c"
    break;

  case 33: /* close_scope_import: %empty  */
#line 1113 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_IMPORT); }
#line 8693 "src/parser_bison.c"
    break;

  case 34: /* close_scope_ipsec: %empty  */
#line 1114 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_IPSEC); }
#line 8699 "src/parser_bison.c"
    break;

  case 35: /* close_scope_list: %empty  */
#line 1115 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_LIST); }
#line 8705 "src/parser_bison.c"
    break;

  case 36: /* close_scope_limit: %empty  */
#line 1116 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_LIMIT); }
#line 8711 "src/parser_bison.c"
    break;

  case 37: /* close_scope_meta: %empty  */
#line 1117 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_META); }
#line 8717 "src/parser_bison.c"
    break;

  case 38: /* close_scope_mh: %empty  */
#line 1118 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_MH); }
#line 8723 "src/parser_bison.c"
    break;

  case 39: /* close_scope_monitor: %empty  */
#line 1119 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_MONITOR); }
#line 8729 "src/parser_bison.c"
    break;

  case 40: /* close_scope_nat: %empty  */
#line 1120 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_NAT); }
#line 8735 "src/parser_bison.c"
    break;

  case 41: /* close_scope_numgen: %empty  */
#line 1121 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_NUMGEN); }
#line 8741 "src/parser_bison.c"
    break;

  case 42: /* close_scope_osf: %empty  */
#line 1122 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_OSF); }
#line 8747 "src/parser_bison.c"
    break;

  case 43: /* close_scope_policy: %empty  */
#line 1123 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_POLICY); }
#line 8753 "src/parser_bison.c"
    break;

  case 44: /* close_scope_quota: %empty  */
#line 1124 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_QUOTA); }
#line 8759 "src/parser_bison.c"
    break;

  case 45: /* close_scope_queue: %empty  */
#line 1125 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_QUEUE); }
#line 8765 "src/parser_bison.c"
    break;

  case 46: /* close_scope_rate: %empty  */
#line 1126 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_RATE); }
#line 8771 "src/parser_bison.c"
    break;

  case 47: /* close_scope_reject: %empty  */
#line 1127 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_REJECT); }
#line 8777 "src/parser_bison.c"
    break;

  case 48: /* close_scope_reset: %empty  */
#line 1128 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_CMD_RESET); }
#line 8783 "src/parser_bison.c"
    break;

  case 49: /* close_scope_rt: %empty  */
#line 1129 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_RT); }
#line 8789 "src/parser_bison.c"
    break;

  case 50: /* close_scope_sctp: %empty  */
#line 1130 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SCTP); }
#line 8795 "src/parser_bison.c"
    break;

  case 51: /* close_scope_sctp_chunk: %empty  */
#line 1131 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SCTP_CHUNK); }
#line 8801 "src/parser_bison.c"
    break;

  case 52: /* close_scope_secmark: %empty  */
#line 1132 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_SECMARK); }
#line 8807 "src/parser_bison.c"
    break;

  case 53: /* close_scope_socket: %empty  */
#line 1133 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_SOCKET); }
#line 8813 "src/parser_bison.c"
    break;

  case 54: /* close_scope_tcp: %empty  */
#line 1134 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TCP); }
#line 8819 "src/parser_bison.c"
    break;

  case 55: /* close_scope_tproxy: %empty  */
#line 1135 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_TPROXY); }
#line 8825 "src/parser_bison.c"
    break;

  case 56: /* close_scope_type: %empty  */
#line 1136 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TYPE); }
#line 8831 "src/parser_bison.c"
    break;

  case 57: /* close_scope_th: %empty  */
#line 1137 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_TH); }
#line 8837 "src/parser_bison.c"
    break;

  case 58: /* close_scope_udp: %empty  */
#line 1138 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDP); }
#line 8843 "src/parser_bison.c"
    break;

  case 59: /* close_scope_udplite: %empty  */
#line 1139 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_EXPR_UDPLITE); }
#line 8849 "src/parser_bison.c"
    break;

  case 60: /* close_scope_log: %empty  */
#line 1141 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_LOG); }
#line 8855 "src/parser_bison.c"
    break;

  case 61: /* close_scope_synproxy: %empty  */
#line 1142 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_STMT_SYNPROXY); }
#line 8861 "src/parser_bison.c"
    break;

  case 62: /* close_scope_tunnel: %empty  */
#line 1143 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_TUNNEL); }
#line 8867 "src/parser_bison.c"
    break;

  case 63: /* close_scope_xt: %empty  */
#line 1144 "src/parser_bison.y"
                          { scanner_pop_start_cond(nft->scanner, PARSER_SC_XT); }
#line 8873 "src/parser_bison.c"
    break;

  case 64: /* common_block: "include" "quoted string" stmt_separator  */
#line 1147 "src/parser_bison.y"
                        {
				if (scanner_include_file(nft, scanner, (yyvsp[-1].string), &(yyloc)) < 0) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				free_const((yyvsp[-1].string));
			}
#line 8885 "src/parser_bison.c"
    break;

  case 65: /* common_block: "define" identifier '=' initializer_expr stmt_separator  */
#line 1155 "src/parser_bison.y"
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
#line 8904 "src/parser_bison.c"
    break;

  case 66: /* common_block: "redefine" identifier '=' initializer_expr stmt_separator  */
#line 1170 "src/parser_bison.y"
                        {
				struct scope *scope = current_scope(state);

				symbol_bind(scope, (yyvsp[-3].string), (yyvsp[-1].expr));
				free_const((yyvsp[-3].string));
			}
#line 8915 "src/parser_bison.c"
    break;

  case 67: /* common_block: "undefine" identifier stmt_separator  */
#line 1177 "src/parser_bison.y"
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
#line 8931 "src/parser_bison.c"
    break;

  case 68: /* common_block: error stmt_separator  */
#line 1189 "src/parser_bison.y"
                        {
				if (++state->nerrs == nft->parser_max_errors)
					YYABORT;
				yyerrok;
			}
#line 8941 "src/parser_bison.c"
    break;

  case 69: /* line: common_block  */
#line 1196 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8947 "src/parser_bison.c"
    break;

  case 70: /* line: stmt_separator  */
#line 1197 "src/parser_bison.y"
                                                                { (yyval.cmd) = NULL; }
#line 8953 "src/parser_bison.c"
    break;

  case 71: /* line: base_cmd stmt_separator  */
#line 1198 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 8959 "src/parser_bison.c"
    break;

  case 72: /* line: base_cmd "end of file"  */
#line 1200 "src/parser_bison.y"
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
#line 8981 "src/parser_bison.c"
    break;

  case 73: /* base_cmd: add_cmd  */
#line 1219 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8987 "src/parser_bison.c"
    break;

  case 74: /* base_cmd: "add" add_cmd  */
#line 1220 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8993 "src/parser_bison.c"
    break;

  case 75: /* base_cmd: "replace" replace_cmd  */
#line 1221 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 8999 "src/parser_bison.c"
    break;

  case 76: /* base_cmd: "create" create_cmd  */
#line 1222 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9005 "src/parser_bison.c"
    break;

  case 77: /* base_cmd: "insert" insert_cmd  */
#line 1223 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9011 "src/parser_bison.c"
    break;

  case 78: /* base_cmd: "delete" delete_cmd  */
#line 1224 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9017 "src/parser_bison.c"
    break;

  case 79: /* base_cmd: "get" get_cmd  */
#line 1225 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9023 "src/parser_bison.c"
    break;

  case 80: /* base_cmd: "list" list_cmd close_scope_list  */
#line 1226 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9029 "src/parser_bison.c"
    break;

  case 81: /* base_cmd: "reset" reset_cmd close_scope_reset  */
#line 1227 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9035 "src/parser_bison.c"
    break;

  case 82: /* base_cmd: "flush" flush_cmd  */
#line 1228 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9041 "src/parser_bison.c"
    break;

  case 83: /* base_cmd: "rename" rename_cmd  */
#line 1229 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9047 "src/parser_bison.c"
    break;

  case 84: /* base_cmd: "import" import_cmd close_scope_import  */
#line 1230 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9053 "src/parser_bison.c"
    break;

  case 85: /* base_cmd: "export" export_cmd close_scope_export  */
#line 1231 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9059 "src/parser_bison.c"
    break;

  case 86: /* base_cmd: "monitor" monitor_cmd close_scope_monitor  */
#line 1232 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9065 "src/parser_bison.c"
    break;

  case 87: /* base_cmd: "describe" describe_cmd  */
#line 1233 "src/parser_bison.y"
                                                                { (yyval.cmd) = (yyvsp[0].cmd); }
#line 9071 "src/parser_bison.c"
    break;

  case 88: /* base_cmd: "destroy" destroy_cmd close_scope_destroy  */
#line 1234 "src/parser_bison.y"
                                                                                        { (yyval.cmd) = (yyvsp[-1].cmd); }
#line 9077 "src/parser_bison.c"
    break;

  case 89: /* add_cmd: "table" table_spec  */
#line 1238 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9085 "src/parser_bison.c"
    break;

  case 90: /* add_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1243 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 9095 "src/parser_bison.c"
    break;

  case 91: /* add_cmd: "chain" chain_spec  */
#line 1249 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9103 "src/parser_bison.c"
    break;

  case 92: /* add_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1254 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9114 "src/parser_bison.c"
    break;

  case 93: /* add_cmd: "rule" rule_position rule  */
#line 1261 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9122 "src/parser_bison.c"
    break;

  case 94: /* add_cmd: rule_position rule  */
#line 1265 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9130 "src/parser_bison.c"
    break;

  case 95: /* add_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1270 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9140 "src/parser_bison.c"
    break;

  case 96: /* add_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1277 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9150 "src/parser_bison.c"
    break;

  case 97: /* add_cmd: "element" set_spec set_block_expr  */
#line 1283 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_ADD, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9164 "src/parser_bison.c"
    break;

  case 98: /* add_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1294 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9174 "src/parser_bison.c"
    break;

  case 99: /* add_cmd: "counter" obj_spec close_scope_counter  */
#line 1300 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 9187 "src/parser_bison.c"
    break;

  case 100: /* add_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1309 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9195 "src/parser_bison.c"
    break;

  case 101: /* add_cmd: "counter" obj_spec counter_obj '{' counter_block '}' close_scope_counter  */
#line 1313 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_COUNTER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9203 "src/parser_bison.c"
    break;

  case 102: /* add_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1317 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9211 "src/parser_bison.c"
    break;

  case 103: /* add_cmd: "quota" obj_spec quota_obj '{' quota_block '}' close_scope_quota  */
#line 1321 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_QUOTA, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9219 "src/parser_bison.c"
    break;

  case 104: /* add_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1325 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9227 "src/parser_bison.c"
    break;

  case 105: /* add_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1329 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9235 "src/parser_bison.c"
    break;

  case 106: /* add_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1333 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_ADD, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9243 "src/parser_bison.c"
    break;

  case 107: /* add_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1337 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9251 "src/parser_bison.c"
    break;

  case 108: /* add_cmd: "limit" obj_spec limit_obj '{' limit_block '}' close_scope_limit  */
#line 1341 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_LIMIT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9259 "src/parser_bison.c"
    break;

  case 109: /* add_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1345 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9267 "src/parser_bison.c"
    break;

  case 110: /* add_cmd: "secmark" obj_spec secmark_obj '{' secmark_block '}' close_scope_secmark  */
#line 1349 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SECMARK, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9275 "src/parser_bison.c"
    break;

  case 111: /* add_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1353 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9283 "src/parser_bison.c"
    break;

  case 112: /* add_cmd: "synproxy" obj_spec synproxy_obj '{' synproxy_block '}' close_scope_synproxy  */
#line 1357 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_SYNPROXY, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9291 "src/parser_bison.c"
    break;

  case 113: /* add_cmd: "tunnel" obj_spec tunnel_obj '{' tunnel_block '}' close_scope_tunnel  */
#line 1361 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_TUNNEL, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9299 "src/parser_bison.c"
    break;

  case 114: /* add_cmd: "ct" "count" obj_spec connlimit_obj '{' connlimit_block '}' close_scope_ct  */
#line 1365 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_ADD, CMD_OBJ_CONNLIMIT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9307 "src/parser_bison.c"
    break;

  case 115: /* replace_cmd: "rule" ruleid_spec rule  */
#line 1371 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_REPLACE, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9315 "src/parser_bison.c"
    break;

  case 116: /* create_cmd: "table" table_spec  */
#line 1377 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9323 "src/parser_bison.c"
    break;

  case 117: /* create_cmd: "table" table_spec table_block_alloc '{' table_block '}'  */
#line 1382 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-3].table)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].table));
			}
#line 9333 "src/parser_bison.c"
    break;

  case 118: /* create_cmd: "chain" chain_spec  */
#line 1388 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9341 "src/parser_bison.c"
    break;

  case 119: /* create_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1393 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9352 "src/parser_bison.c"
    break;

  case 120: /* create_cmd: "set" set_spec set_block_alloc '{' set_block '}'  */
#line 1401 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9362 "src/parser_bison.c"
    break;

  case 121: /* create_cmd: "map" set_spec map_block_alloc '{' map_block '}'  */
#line 1408 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].set)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SET, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].set));
			}
#line 9372 "src/parser_bison.c"
    break;

  case 122: /* create_cmd: "element" set_spec set_block_expr  */
#line 1414 "src/parser_bison.y"
                        {
				if (nft_cmd_collapse_elems(CMD_CREATE, state->cmds, &(yyvsp[-1].handle), (yyvsp[0].expr))) {
					handle_free(&(yyvsp[-1].handle));
					expr_free((yyvsp[0].expr));
					(yyval.cmd) = NULL;
					break;
				}
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9386 "src/parser_bison.c"
    break;

  case 123: /* create_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1425 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9396 "src/parser_bison.c"
    break;

  case 124: /* create_cmd: "counter" obj_spec close_scope_counter  */
#line 1431 "src/parser_bison.y"
                        {
				struct obj *obj;

				obj = obj_alloc(&(yyloc));
				obj->type = NFT_OBJECT_COUNTER;
				handle_merge(&obj->handle, &(yyvsp[-1].handle));
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), obj);
			}
#line 9409 "src/parser_bison.c"
    break;

  case 125: /* create_cmd: "counter" obj_spec counter_obj counter_config close_scope_counter  */
#line 1440 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_COUNTER, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9417 "src/parser_bison.c"
    break;

  case 126: /* create_cmd: "quota" obj_spec quota_obj quota_config close_scope_quota  */
#line 1444 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_QUOTA, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9425 "src/parser_bison.c"
    break;

  case 127: /* create_cmd: "ct" "helper" obj_spec ct_obj_alloc '{' ct_helper_block '}' close_scope_ct  */
#line 1448 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_HELPER, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9433 "src/parser_bison.c"
    break;

  case 128: /* create_cmd: "ct" "timeout" obj_spec ct_obj_alloc '{' ct_timeout_block '}' close_scope_ct  */
#line 1452 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_TIMEOUT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9441 "src/parser_bison.c"
    break;

  case 129: /* create_cmd: "ct" "expectation" obj_spec ct_obj_alloc '{' ct_expect_block '}' close_scope_ct  */
#line 1456 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_CREATE, NFT_OBJECT_CT_EXPECT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9449 "src/parser_bison.c"
    break;

  case 130: /* create_cmd: "limit" obj_spec limit_obj limit_config close_scope_limit  */
#line 1460 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_LIMIT, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9457 "src/parser_bison.c"
    break;

  case 131: /* create_cmd: "secmark" obj_spec secmark_obj secmark_config close_scope_secmark  */
#line 1464 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SECMARK, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9465 "src/parser_bison.c"
    break;

  case 132: /* create_cmd: "synproxy" obj_spec synproxy_obj synproxy_config close_scope_synproxy  */
#line 1468 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_SYNPROXY, &(yyvsp[-3].handle), &(yyloc), (yyvsp[-2].obj));
			}
#line 9473 "src/parser_bison.c"
    break;

  case 133: /* create_cmd: "tunnel" obj_spec tunnel_obj '{' tunnel_block '}' close_scope_tunnel  */
#line 1472 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_TUNNEL, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9481 "src/parser_bison.c"
    break;

  case 134: /* create_cmd: "ct" "count" obj_spec connlimit_obj '{' connlimit_block '}' close_scope_ct  */
#line 1476 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_CREATE, CMD_OBJ_CONNLIMIT, &(yyvsp[-5].handle), &(yyloc), (yyvsp[-4].obj));
			}
#line 9489 "src/parser_bison.c"
    break;

  case 135: /* insert_cmd: "rule" rule_position rule  */
#line 1482 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_INSERT, CMD_OBJ_RULE, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].rule));
			}
#line 9497 "src/parser_bison.c"
    break;

  case 144: /* delete_cmd: "table" table_or_id_spec  */
#line 1504 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9505 "src/parser_bison.c"
    break;

  case 145: /* delete_cmd: "chain" chain_or_id_spec  */
#line 1508 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9513 "src/parser_bison.c"
    break;

  case 146: /* delete_cmd: "chain" chain_spec chain_block_alloc '{' chain_block '}'  */
#line 1513 "src/parser_bison.y"
                        {
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].chain)->handle, &(yyvsp[-4].handle));
				close_scope(state);
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CHAIN, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].chain));
			}
#line 9524 "src/parser_bison.c"
    break;

  case 147: /* delete_cmd: "rule" ruleid_spec  */
#line 1520 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9532 "src/parser_bison.c"
    break;

  case 148: /* delete_cmd: "set" set_or_id_spec  */
#line 1524 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9540 "src/parser_bison.c"
    break;

  case 149: /* delete_cmd: "map" set_or_id_spec  */
#line 1528 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9548 "src/parser_bison.c"
    break;

  case 150: /* delete_cmd: "element" set_spec set_block_expr  */
#line 1532 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9556 "src/parser_bison.c"
    break;

  case 151: /* delete_cmd: "flowtable" flowtable_spec  */
#line 1536 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9564 "src/parser_bison.c"
    break;

  case 152: /* delete_cmd: "flowtable" flowtableid_spec  */
#line 1540 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9572 "src/parser_bison.c"
    break;

  case 153: /* delete_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1545 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9582 "src/parser_bison.c"
    break;

  case 154: /* delete_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1551 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9590 "src/parser_bison.c"
    break;

  case 155: /* delete_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1555 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9598 "src/parser_bison.c"
    break;

  case 156: /* delete_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1559 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DELETE, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9608 "src/parser_bison.c"
    break;

  case 157: /* delete_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1565 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9616 "src/parser_bison.c"
    break;

  case 158: /* delete_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1569 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9624 "src/parser_bison.c"
    break;

  case 159: /* delete_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1573 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9632 "src/parser_bison.c"
    break;

  case 160: /* delete_cmd: "tunnel" obj_or_id_spec close_scope_tunnel  */
#line 1577 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9640 "src/parser_bison.c"
    break;

  case 161: /* delete_cmd: "ct" "count" obj_or_id_spec close_scope_ct  */
#line 1581 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DELETE, CMD_OBJ_CONNLIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9648 "src/parser_bison.c"
    break;

  case 162: /* destroy_cmd: "table" table_or_id_spec  */
#line 1587 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9656 "src/parser_bison.c"
    break;

  case 163: /* destroy_cmd: "chain" chain_or_id_spec  */
#line 1591 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9664 "src/parser_bison.c"
    break;

  case 164: /* destroy_cmd: "rule" ruleid_spec  */
#line 1595 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9672 "src/parser_bison.c"
    break;

  case 165: /* destroy_cmd: "set" set_or_id_spec  */
#line 1599 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9680 "src/parser_bison.c"
    break;

  case 166: /* destroy_cmd: "map" set_spec  */
#line 1603 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9688 "src/parser_bison.c"
    break;

  case 167: /* destroy_cmd: "element" set_spec set_block_expr  */
#line 1607 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9696 "src/parser_bison.c"
    break;

  case 168: /* destroy_cmd: "flowtable" flowtable_spec  */
#line 1611 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9704 "src/parser_bison.c"
    break;

  case 169: /* destroy_cmd: "flowtable" flowtableid_spec  */
#line 1615 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9712 "src/parser_bison.c"
    break;

  case 170: /* destroy_cmd: "flowtable" flowtable_spec flowtable_block_alloc '{' flowtable_block '}'  */
#line 1620 "src/parser_bison.y"
                        {
				(yyvsp[-1].flowtable)->location = (yylsp[-1]);
				handle_merge(&(yyvsp[-3].flowtable)->handle, &(yyvsp[-4].handle));
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_FLOWTABLE, &(yyvsp[-4].handle), &(yyloc), (yyvsp[-1].flowtable));
			}
#line 9722 "src/parser_bison.c"
    break;

  case 171: /* destroy_cmd: "counter" obj_or_id_spec close_scope_counter  */
#line 1626 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9730 "src/parser_bison.c"
    break;

  case 172: /* destroy_cmd: "quota" obj_or_id_spec close_scope_quota  */
#line 1630 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9738 "src/parser_bison.c"
    break;

  case 173: /* destroy_cmd: "ct" ct_obj_type obj_spec ct_obj_alloc close_scope_ct  */
#line 1634 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_DESTROY, (yyvsp[-3].val), &(yyvsp[-2].handle), &(yyloc), (yyvsp[-1].obj));
				if ((yyvsp[-3].val) == NFT_OBJECT_CT_TIMEOUT)
					init_list_head(&(yyvsp[-1].obj)->ct_timeout.timeout_list);
			}
#line 9748 "src/parser_bison.c"
    break;

  case 174: /* destroy_cmd: "limit" obj_or_id_spec close_scope_limit  */
#line 1640 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9756 "src/parser_bison.c"
    break;

  case 175: /* destroy_cmd: "secmark" obj_or_id_spec close_scope_secmark  */
#line 1644 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9764 "src/parser_bison.c"
    break;

  case 176: /* destroy_cmd: "synproxy" obj_or_id_spec close_scope_synproxy  */
#line 1648 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9772 "src/parser_bison.c"
    break;

  case 177: /* destroy_cmd: "tunnel" obj_or_id_spec close_scope_tunnel  */
#line 1652 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9780 "src/parser_bison.c"
    break;

  case 178: /* destroy_cmd: "ct" "count" obj_or_id_spec close_scope_ct  */
#line 1656 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_DESTROY, CMD_OBJ_CONNLIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9788 "src/parser_bison.c"
    break;

  case 179: /* get_cmd: "element" set_spec set_block_expr  */
#line 1663 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_GET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 9796 "src/parser_bison.c"
    break;

  case 180: /* list_cmd_spec_table: "table" table_spec  */
#line 1668 "src/parser_bison.y"
                                                        { (yyval.handle) = (yyvsp[0].handle); }
#line 9802 "src/parser_bison.c"
    break;

  case 184: /* list_cmd: "table" table_spec  */
#line 1676 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9810 "src/parser_bison.c"
    break;

  case 185: /* list_cmd: "tables" ruleset_spec  */
#line 1680 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9818 "src/parser_bison.c"
    break;

  case 186: /* list_cmd: "chain" chain_spec  */
#line 1684 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9826 "src/parser_bison.c"
    break;

  case 187: /* list_cmd: "chains" list_cmd_spec_any  */
#line 1688 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CHAINS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9834 "src/parser_bison.c"
    break;

  case 188: /* list_cmd: "sets" list_cmd_spec_any  */
#line 1692 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SETS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9842 "src/parser_bison.c"
    break;

  case 189: /* list_cmd: "set" set_spec  */
#line 1696 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9850 "src/parser_bison.c"
    break;

  case 190: /* list_cmd: "counters" list_cmd_spec_any  */
#line 1700 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9858 "src/parser_bison.c"
    break;

  case 191: /* list_cmd: "counter" obj_spec close_scope_counter  */
#line 1704 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_COUNTER, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9866 "src/parser_bison.c"
    break;

  case 192: /* list_cmd: "quotas" list_cmd_spec_any  */
#line 1708 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9874 "src/parser_bison.c"
    break;

  case 193: /* list_cmd: "quota" obj_spec close_scope_quota  */
#line 1712 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9882 "src/parser_bison.c"
    break;

  case 194: /* list_cmd: "limits" list_cmd_spec_any  */
#line 1716 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMITS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9890 "src/parser_bison.c"
    break;

  case 195: /* list_cmd: "limit" obj_spec close_scope_limit  */
#line 1720 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_LIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9898 "src/parser_bison.c"
    break;

  case 196: /* list_cmd: "secmarks" list_cmd_spec_any  */
#line 1724 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9906 "src/parser_bison.c"
    break;

  case 197: /* list_cmd: "secmark" obj_spec close_scope_secmark  */
#line 1728 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SECMARK, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9914 "src/parser_bison.c"
    break;

  case 198: /* list_cmd: "synproxys" list_cmd_spec_any  */
#line 1732 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXYS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9922 "src/parser_bison.c"
    break;

  case 199: /* list_cmd: "synproxy" obj_spec close_scope_synproxy  */
#line 1736 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_SYNPROXY, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 9930 "src/parser_bison.c"
    break;

  case 200: /* list_cmd: "ruleset" ruleset_spec  */
#line 1740 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9938 "src/parser_bison.c"
    break;

  case 201: /* list_cmd: "flow" "tables" ruleset_spec  */
#line 1744 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9946 "src/parser_bison.c"
    break;

  case 202: /* list_cmd: "flow" "table" set_spec  */
#line 1748 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9954 "src/parser_bison.c"
    break;

  case 203: /* list_cmd: "meters" ruleset_spec  */
#line 1752 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9962 "src/parser_bison.c"
    break;

  case 204: /* list_cmd: "meter" set_spec  */
#line 1756 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9970 "src/parser_bison.c"
    break;

  case 205: /* list_cmd: "flowtables" list_cmd_spec_any  */
#line 1760 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9978 "src/parser_bison.c"
    break;

  case 206: /* list_cmd: "flowtable" flowtable_spec  */
#line 1764 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_FLOWTABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9986 "src/parser_bison.c"
    break;

  case 207: /* list_cmd: "maps" list_cmd_spec_any  */
#line 1768 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAPS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 9994 "src/parser_bison.c"
    break;

  case 208: /* list_cmd: "map" set_spec  */
#line 1772 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10002 "src/parser_bison.c"
    break;

  case 209: /* list_cmd: "ct" ct_obj_type obj_spec close_scope_ct  */
#line 1776 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc_obj_ct(CMD_LIST, (yyvsp[-2].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 10010 "src/parser_bison.c"
    break;

  case 210: /* list_cmd: "ct" ct_cmd_type "table" table_spec close_scope_ct  */
#line 1780 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, (yyvsp[-3].val), &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 10018 "src/parser_bison.c"
    break;

  case 211: /* list_cmd: "hooks" basehook_spec  */
#line 1784 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_HOOKS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10026 "src/parser_bison.c"
    break;

  case 212: /* list_cmd: "tunnels" list_cmd_spec_any  */
#line 1788 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TUNNELS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10034 "src/parser_bison.c"
    break;

  case 213: /* list_cmd: "tunnel" obj_spec close_scope_tunnel  */
#line 1792 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_TUNNEL, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 10042 "src/parser_bison.c"
    break;

  case 214: /* list_cmd: "ct" "count" obj_spec close_scope_ct  */
#line 1796 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CONNLIMIT, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 10050 "src/parser_bison.c"
    break;

  case 215: /* list_cmd: "ct" "counts" list_cmd_spec_any  */
#line 1800 "src/parser_bison.y"
                        {

				(yyval.cmd) = cmd_alloc(CMD_LIST, CMD_OBJ_CONNLIMITS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10059 "src/parser_bison.c"
    break;

  case 216: /* basehook_device_name: "device" "string"  */
#line 1807 "src/parser_bison.y"
                        {
				(yyval.string) = (yyvsp[0].string);
			}
#line 10067 "src/parser_bison.c"
    break;

  case 217: /* basehook_spec: ruleset_spec  */
#line 1813 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 10075 "src/parser_bison.c"
    break;

  case 218: /* basehook_spec: ruleset_spec basehook_device_name  */
#line 1817 "src/parser_bison.y"
                        {
				if ((yyvsp[0].string)) {
					(yyvsp[-1].handle).obj.name = (yyvsp[0].string);
					(yyvsp[-1].handle).obj.location = (yylsp[0]);
				}
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 10087 "src/parser_bison.c"
    break;

  case 219: /* reset_cmd: "counters" list_cmd_spec_any  */
#line 1827 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTERS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10095 "src/parser_bison.c"
    break;

  case 220: /* reset_cmd: "counter" obj_spec close_scope_counter  */
#line 1831 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_COUNTER, &(yyvsp[-1].handle),&(yyloc), NULL);
			}
#line 10103 "src/parser_bison.c"
    break;

  case 221: /* reset_cmd: "quotas" list_cmd_spec_any  */
#line 1835 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTAS, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10111 "src/parser_bison.c"
    break;

  case 222: /* reset_cmd: "quota" obj_spec close_scope_quota  */
#line 1839 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_QUOTA, &(yyvsp[-1].handle), &(yyloc), NULL);
			}
#line 10119 "src/parser_bison.c"
    break;

  case 223: /* reset_cmd: "rules" ruleset_spec  */
#line 1843 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULES, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10127 "src/parser_bison.c"
    break;

  case 224: /* reset_cmd: "rules" list_cmd_spec_table  */
#line 1847 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10135 "src/parser_bison.c"
    break;

  case 225: /* reset_cmd: "rules" chain_spec  */
#line 1851 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10143 "src/parser_bison.c"
    break;

  case 226: /* reset_cmd: "rules" "chain" chain_spec  */
#line 1855 "src/parser_bison.y"
                        {
				/* alias of previous rule. */
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10152 "src/parser_bison.c"
    break;

  case 227: /* reset_cmd: "rule" ruleid_spec  */
#line 1860 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_RULE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10160 "src/parser_bison.c"
    break;

  case 228: /* reset_cmd: "element" set_spec set_block_expr  */
#line 1864 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_ELEMENTS, &(yyvsp[-1].handle), &(yyloc), (yyvsp[0].expr));
			}
#line 10168 "src/parser_bison.c"
    break;

  case 229: /* reset_cmd: "set" set_spec  */
#line 1868 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10176 "src/parser_bison.c"
    break;

  case 230: /* reset_cmd: "map" set_spec  */
#line 1872 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RESET, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10184 "src/parser_bison.c"
    break;

  case 231: /* flush_cmd: "table" table_spec  */
#line 1878 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_TABLE, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10192 "src/parser_bison.c"
    break;

  case 232: /* flush_cmd: "chain" chain_spec  */
#line 1882 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_CHAIN, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10200 "src/parser_bison.c"
    break;

  case 233: /* flush_cmd: "set" set_spec  */
#line 1886 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_SET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10208 "src/parser_bison.c"
    break;

  case 234: /* flush_cmd: "map" set_spec  */
#line 1890 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_MAP, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10216 "src/parser_bison.c"
    break;

  case 235: /* flush_cmd: "flow" "table" set_spec  */
#line 1894 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10224 "src/parser_bison.c"
    break;

  case 236: /* flush_cmd: "meter" set_spec  */
#line 1898 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_METER, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10232 "src/parser_bison.c"
    break;

  case 237: /* flush_cmd: "ruleset" ruleset_spec  */
#line 1902 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_FLUSH, CMD_OBJ_RULESET, &(yyvsp[0].handle), &(yyloc), NULL);
			}
#line 10240 "src/parser_bison.c"
    break;

  case 238: /* rename_cmd: "chain" chain_spec identifier  */
#line 1908 "src/parser_bison.y"
                        {
				(yyval.cmd) = cmd_alloc(CMD_RENAME, CMD_OBJ_CHAIN, &(yyvsp[-1].handle), &(yyloc), NULL);
				(yyval.cmd)->arg = (yyvsp[0].string);
			}
#line 10249 "src/parser_bison.c"
    break;

  case 239: /* import_cmd: "ruleset" markup_format  */
#line 1915 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10259 "src/parser_bison.c"
    break;

  case 240: /* import_cmd: markup_format  */
#line 1921 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_IMPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10269 "src/parser_bison.c"
    break;

  case 241: /* export_cmd: "ruleset" markup_format  */
#line 1929 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10279 "src/parser_bison.c"
    break;

  case 242: /* export_cmd: markup_format  */
#line 1935 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct markup *markup = markup_alloc((yyvsp[0].val));
				(yyval.cmd) = cmd_alloc(CMD_EXPORT, CMD_OBJ_MARKUP, &h, &(yyloc), markup);
			}
#line 10289 "src/parser_bison.c"
    break;

  case 243: /* monitor_cmd: monitor_event monitor_object monitor_format  */
#line 1943 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				struct monitor *m = monitor_alloc((yyvsp[0].val), (yyvsp[-1].val), (yyvsp[-2].val));
				m->location = (yylsp[-2]);
				(yyval.cmd) = cmd_alloc(CMD_MONITOR, CMD_OBJ_MONITOR, &h, &(yyloc), m);
			}
#line 10300 "src/parser_bison.c"
    break;

  case 244: /* monitor_event: %empty  */
#line 1951 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_EVENT_ANY; }
#line 10306 "src/parser_bison.c"
    break;

  case 245: /* monitor_event: "new"  */
#line 1952 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_EVENT_NEW; }
#line 10312 "src/parser_bison.c"
    break;

  case 246: /* monitor_event: "destroy"  */
#line 1953 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_EVENT_DEL; }
#line 10318 "src/parser_bison.c"
    break;

  case 247: /* monitor_object: %empty  */
#line 1956 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ANY; }
#line 10324 "src/parser_bison.c"
    break;

  case 248: /* monitor_object: "tables"  */
#line 1957 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TABLES; }
#line 10330 "src/parser_bison.c"
    break;

  case 249: /* monitor_object: "chains"  */
#line 1958 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_CHAINS; }
#line 10336 "src/parser_bison.c"
    break;

  case 250: /* monitor_object: "sets"  */
#line 1959 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_SETS; }
#line 10342 "src/parser_bison.c"
    break;

  case 251: /* monitor_object: "rules"  */
#line 1960 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULES; }
#line 10348 "src/parser_bison.c"
    break;

  case 252: /* monitor_object: "elements"  */
#line 1961 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_ELEMS; }
#line 10354 "src/parser_bison.c"
    break;

  case 253: /* monitor_object: "ruleset"  */
#line 1962 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_RULESET; }
#line 10360 "src/parser_bison.c"
    break;

  case 254: /* monitor_object: "trace"  */
#line 1963 "src/parser_bison.y"
                                                { (yyval.val) = CMD_MONITOR_OBJ_TRACE; }
#line 10366 "src/parser_bison.c"
    break;

  case 255: /* monitor_format: %empty  */
#line 1966 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_DEFAULT; }
#line 10372 "src/parser_bison.c"
    break;

  case 257: /* markup_format: "xml"  */
#line 1970 "src/parser_bison.y"
                                                { (yyval.val) = __NFT_OUTPUT_NOTSUPP; }
#line 10378 "src/parser_bison.c"
    break;

  case 258: /* markup_format: "json"  */
#line 1971 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 10384 "src/parser_bison.c"
    break;

  case 259: /* markup_format: "vm" "json"  */
#line 1972 "src/parser_bison.y"
                                                { (yyval.val) = NFTNL_OUTPUT_JSON; }
#line 10390 "src/parser_bison.c"
    break;

  case 260: /* describe_cmd: primary_expr  */
#line 1976 "src/parser_bison.y"
                        {
				struct handle h = { .family = NFPROTO_UNSPEC };
				(yyval.cmd) = cmd_alloc(CMD_DESCRIBE, CMD_OBJ_EXPR, &h, &(yyloc), NULL);
				(yyval.cmd)->expr = (yyvsp[0].expr);
			}
#line 10400 "src/parser_bison.c"
    break;

  case 261: /* table_block_alloc: %empty  */
#line 1984 "src/parser_bison.y"
                        {
				(yyval.table) = table_alloc();
				if (open_scope(state, &(yyval.table)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 10413 "src/parser_bison.c"
    break;

  case 262: /* table_options: "flags" table_flags  */
#line 1995 "src/parser_bison.y"
                        {
				(yyvsp[-2].table)->flags |= (yyvsp[0].val);
			}
#line 10421 "src/parser_bison.c"
    break;

  case 263: /* table_options: comment_spec  */
#line 1999 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].table)->comment, &(yyloc), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].table)->comment = (yyvsp[0].string);
			}
#line 10433 "src/parser_bison.c"
    break;

  case 265: /* table_flags: table_flags "comma" table_flag  */
#line 2010 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 10441 "src/parser_bison.c"
    break;

  case 266: /* table_flag: "string"  */
#line 2015 "src/parser_bison.y"
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
#line 10457 "src/parser_bison.c"
    break;

  case 267: /* table_block: %empty  */
#line 2028 "src/parser_bison.y"
                                                { (yyval.table) = (yyvsp[(-1) - (0)].table); }
#line 10463 "src/parser_bison.c"
    break;

  case 271: /* table_block: table_block "chain" chain_identifier chain_block_alloc '{' chain_block '}' stmt_separator  */
#line 2035 "src/parser_bison.y"
                        {
				(yyvsp[-4].chain)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].chain)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				close_scope(state);
				list_add_tail(&(yyvsp[-4].chain)->list, &(yyvsp[-7].table)->chains);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10476 "src/parser_bison.c"
    break;

  case 272: /* table_block: table_block "set" set_identifier set_block_alloc '{' set_block '}' stmt_separator  */
#line 2046 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10488 "src/parser_bison.c"
    break;

  case 273: /* table_block: table_block "map" set_identifier map_block_alloc '{' map_block '}' stmt_separator  */
#line 2056 "src/parser_bison.y"
                        {
				(yyvsp[-4].set)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].set)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].set)->list, &(yyvsp[-7].table)->sets);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10500 "src/parser_bison.c"
    break;

  case 274: /* table_block: table_block "flowtable" flowtable_identifier flowtable_block_alloc '{' flowtable_block '}' stmt_separator  */
#line 2067 "src/parser_bison.y"
                        {
				(yyvsp[-4].flowtable)->location = (yylsp[-5]);
				handle_merge(&(yyvsp[-4].flowtable)->handle, &(yyvsp[-5].handle));
				handle_free(&(yyvsp[-5].handle));
				list_add_tail(&(yyvsp[-4].flowtable)->list, &(yyvsp[-7].table)->flowtables);
				(yyval.table) = (yyvsp[-7].table);
			}
#line 10512 "src/parser_bison.c"
    break;

  case 275: /* table_block: table_block "counter" obj_identifier obj_block_alloc '{' counter_block '}' stmt_separator close_scope_counter  */
#line 2077 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_COUNTER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10525 "src/parser_bison.c"
    break;

  case 276: /* table_block: table_block "quota" obj_identifier obj_block_alloc '{' quota_block '}' stmt_separator close_scope_quota  */
#line 2088 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_QUOTA;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10538 "src/parser_bison.c"
    break;

  case 277: /* table_block: table_block "ct" "helper" obj_identifier obj_block_alloc '{' ct_helper_block '}' stmt_separator close_scope_ct  */
#line 2097 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_HELPER;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10551 "src/parser_bison.c"
    break;

  case 278: /* table_block: table_block "ct" "timeout" obj_identifier obj_block_alloc '{' ct_timeout_block '}' stmt_separator close_scope_ct  */
#line 2106 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_TIMEOUT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10564 "src/parser_bison.c"
    break;

  case 279: /* table_block: table_block "ct" "expectation" obj_identifier obj_block_alloc '{' ct_expect_block '}' stmt_separator close_scope_ct  */
#line 2115 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CT_EXPECT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10577 "src/parser_bison.c"
    break;

  case 280: /* table_block: table_block "limit" obj_identifier obj_block_alloc '{' limit_block '}' stmt_separator close_scope_limit  */
#line 2126 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_LIMIT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10590 "src/parser_bison.c"
    break;

  case 281: /* table_block: table_block "secmark" obj_identifier obj_block_alloc '{' secmark_block '}' stmt_separator close_scope_secmark  */
#line 2137 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SECMARK;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10603 "src/parser_bison.c"
    break;

  case 282: /* table_block: table_block "synproxy" obj_identifier obj_block_alloc '{' synproxy_block '}' stmt_separator close_scope_synproxy  */
#line 2148 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_SYNPROXY;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10616 "src/parser_bison.c"
    break;

  case 283: /* table_block: table_block "tunnel" obj_identifier obj_block_alloc '{' tunnel_block '}' stmt_separator close_scope_tunnel  */
#line 2159 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_TUNNEL;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-8].table)->objs);
				(yyval.table) = (yyvsp[-8].table);
			}
#line 10629 "src/parser_bison.c"
    break;

  case 284: /* table_block: table_block "ct" "count" obj_identifier obj_block_alloc '{' connlimit_block '}' stmt_separator close_scope_ct  */
#line 2170 "src/parser_bison.y"
                        {
				(yyvsp[-5].obj)->location = (yylsp[-6]);
				(yyvsp[-5].obj)->type = NFT_OBJECT_CONNLIMIT;
				handle_merge(&(yyvsp[-5].obj)->handle, &(yyvsp[-6].handle));
				handle_free(&(yyvsp[-6].handle));
				list_add_tail(&(yyvsp[-5].obj)->list, &(yyvsp[-9].table)->objs);
				(yyval.table) = (yyvsp[-9].table);
			}
#line 10642 "src/parser_bison.c"
    break;

  case 285: /* chain_block_alloc: %empty  */
#line 2181 "src/parser_bison.y"
                        {
				(yyval.chain) = chain_alloc();
				if (open_scope(state, &(yyval.chain)->scope) < 0) {
					erec_queue(error(&(yyloc), "too many levels of nesting"),
						   state->msgs);
					state->nerrs++;
				}
			}
#line 10655 "src/parser_bison.c"
    break;

  case 286: /* chain_block: %empty  */
#line 2191 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10661 "src/parser_bison.c"
    break;

  case 292: /* chain_block: chain_block rule stmt_separator  */
#line 2198 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10670 "src/parser_bison.c"
    break;

  case 293: /* chain_block: chain_block "devices" '=' flowtable_expr stmt_separator  */
#line 2203 "src/parser_bison.y"
                        {
				if ((yyval.chain)->dev_expr) {
					list_splice_init(&expr_list((yyvsp[-1].expr))->expressions, &expr_list((yyval.chain)->dev_expr)->expressions);
					expr_free((yyvsp[-1].expr));
					break;
				}
				(yyval.chain)->dev_expr = (yyvsp[-1].expr);
			}
#line 10683 "src/parser_bison.c"
    break;

  case 294: /* chain_block: chain_block comment_spec stmt_separator  */
#line 2212 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].chain)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].chain)->comment = (yyvsp[-1].string);
			}
#line 10695 "src/parser_bison.c"
    break;

  case 295: /* subchain_block: %empty  */
#line 2221 "src/parser_bison.y"
                                                { (yyval.chain) = (yyvsp[(-1) - (0)].chain); }
#line 10701 "src/parser_bison.c"
    break;

  case 297: /* subchain_block: subchain_block rule stmt_separator  */
#line 2224 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[-1].rule)->list, &(yyvsp[-2].chain)->rules);
				(yyval.chain) = (yyvsp[-2].chain);
			}
#line 10710 "src/parser_bison.c"
    break;

  case 298: /* typeof_verdict_expr: selector_expr  */
#line 2231 "src/parser_bison.y"
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
#line 10726 "src/parser_bison.c"
    break;

  case 299: /* typeof_verdict_expr: typeof_expr "." selector_expr  */
#line 2243 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10739 "src/parser_bison.c"
    break;

  case 300: /* typeof_data_expr: "interval" typeof_expr  */
#line 2254 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->flags |= EXPR_F_INTERVAL;
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10748 "src/parser_bison.c"
    break;

  case 301: /* typeof_data_expr: typeof_verdict_expr  */
#line 2259 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10756 "src/parser_bison.c"
    break;

  case 302: /* typeof_data_expr: "queue"  */
#line 2263 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &queue_type, BYTEORDER_HOST_ENDIAN, 16, NULL);
			}
#line 10764 "src/parser_bison.c"
    break;

  case 303: /* typeof_data_expr: "string"  */
#line 2267 "src/parser_bison.y"
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
#line 10783 "src/parser_bison.c"
    break;

  case 304: /* primary_typeof_expr: selector_expr  */
#line 2284 "src/parser_bison.y"
                        {
				if (expr_ops((yyvsp[0].expr))->build_udata == NULL) {
					erec_queue(error(&(yylsp[0]), "primary expression type '%s' lacks typeof serialization", expr_ops((yyvsp[0].expr))->name),
						   state->msgs);
					expr_free((yyvsp[0].expr));
					YYERROR;
				}

				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10798 "src/parser_bison.c"
    break;

  case 305: /* typeof_expr: primary_typeof_expr  */
#line 2297 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 10806 "src/parser_bison.c"
    break;

  case 306: /* typeof_expr: typeof_expr "." primary_typeof_expr  */
#line 2301 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 10819 "src/parser_bison.c"
    break;

  case 307: /* set_block_alloc: %empty  */
#line 2313 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10827 "src/parser_bison.c"
    break;

  case 308: /* typeof_key_expr: "typeof" typeof_expr  */
#line 2318 "src/parser_bison.y"
                                                    { (yyval.expr) = (yyvsp[0].expr); }
#line 10833 "src/parser_bison.c"
    break;

  case 309: /* typeof_key_expr: "type" data_type_expr close_scope_type  */
#line 2319 "src/parser_bison.y"
                                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 10839 "src/parser_bison.c"
    break;

  case 310: /* set_block: %empty  */
#line 2322 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 10845 "src/parser_bison.c"
    break;

  case 313: /* set_block: set_block typeof_key_expr stmt_separator  */
#line 2326 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->key, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-2].set)->key = (yyvsp[-1].expr);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10859 "src/parser_bison.c"
    break;

  case 314: /* set_block: set_block "flags" set_flag_list stmt_separator  */
#line 2336 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10868 "src/parser_bison.c"
    break;

  case 315: /* set_block: set_block "timeout" time_spec stmt_separator  */
#line 2341 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10877 "src/parser_bison.c"
    break;

  case 316: /* set_block: set_block "gc-interval" time_spec stmt_separator  */
#line 2346 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10886 "src/parser_bison.c"
    break;

  case 317: /* set_block: set_block stateful_stmt_list stmt_separator  */
#line 2351 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 10896 "src/parser_bison.c"
    break;

  case 318: /* set_block: set_block "elements" '=' set_block_expr  */
#line 2357 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-3].set)->init, &(yylsp[-2]), state)) {
					expr_free((yyvsp[0].expr));
					YYERROR;
				}
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 10909 "src/parser_bison.c"
    break;

  case 319: /* set_block: set_block "auto-merge"  */
#line 2366 "src/parser_bison.y"
                        {
				(yyvsp[-1].set)->automerge = true;
				(yyval.set) = (yyvsp[-1].set);
			}
#line 10918 "src/parser_bison.c"
    break;

  case 321: /* set_block: set_block comment_spec stmt_separator  */
#line 2372 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 10931 "src/parser_bison.c"
    break;

  case 324: /* set_flag_list: set_flag_list "comma" set_flag  */
#line 2387 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 10939 "src/parser_bison.c"
    break;

  case 326: /* set_flag: "constant"  */
#line 2393 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_CONSTANT; }
#line 10945 "src/parser_bison.c"
    break;

  case 327: /* set_flag: "interval"  */
#line 2394 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_INTERVAL; }
#line 10951 "src/parser_bison.c"
    break;

  case 328: /* set_flag: "timeout"  */
#line 2395 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_TIMEOUT; }
#line 10957 "src/parser_bison.c"
    break;

  case 329: /* set_flag: "dynamic"  */
#line 2396 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_EVAL; }
#line 10963 "src/parser_bison.c"
    break;

  case 330: /* map_block_alloc: %empty  */
#line 2400 "src/parser_bison.y"
                        {
				(yyval.set) = set_alloc(&internal_location);
			}
#line 10971 "src/parser_bison.c"
    break;

  case 331: /* ct_obj_type_map: "timeout"  */
#line 2405 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 10977 "src/parser_bison.c"
    break;

  case 332: /* ct_obj_type_map: "expectation"  */
#line 2406 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 10983 "src/parser_bison.c"
    break;

  case 333: /* ct_obj_type_map: "count"  */
#line 2407 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CONNLIMIT; }
#line 10989 "src/parser_bison.c"
    break;

  case 334: /* map_block_obj_type: "counter" close_scope_counter  */
#line 2410 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_COUNTER; }
#line 10995 "src/parser_bison.c"
    break;

  case 335: /* map_block_obj_type: "quota" close_scope_quota  */
#line 2411 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_QUOTA; }
#line 11001 "src/parser_bison.c"
    break;

  case 336: /* map_block_obj_type: "limit" close_scope_limit  */
#line 2412 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_OBJECT_LIMIT; }
#line 11007 "src/parser_bison.c"
    break;

  case 337: /* map_block_obj_type: "secmark" close_scope_secmark  */
#line 2413 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_OBJECT_SECMARK; }
#line 11013 "src/parser_bison.c"
    break;

  case 338: /* map_block_obj_type: "synproxy" close_scope_synproxy  */
#line 2414 "src/parser_bison.y"
                                                              { (yyval.val) = NFT_OBJECT_SYNPROXY; }
#line 11019 "src/parser_bison.c"
    break;

  case 340: /* map_block_obj_typeof: "ct" ct_obj_type_map close_scope_ct  */
#line 2418 "src/parser_bison.y"
                                                                        { (yyval.val) = (yyvsp[-1].val); }
#line 11025 "src/parser_bison.c"
    break;

  case 341: /* map_block_data_interval: "interval"  */
#line 2421 "src/parser_bison.y"
                                         { (yyval.val) = EXPR_F_INTERVAL; }
#line 11031 "src/parser_bison.c"
    break;

  case 342: /* map_block_data_interval: %empty  */
#line 2422 "src/parser_bison.y"
                                { (yyval.val) = 0; }
#line 11037 "src/parser_bison.c"
    break;

  case 343: /* map_block: %empty  */
#line 2425 "src/parser_bison.y"
                                                { (yyval.set) = (yyvsp[(-1) - (0)].set); }
#line 11043 "src/parser_bison.c"
    break;

  case 346: /* map_block: map_block "timeout" time_spec stmt_separator  */
#line 2429 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->timeout = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 11052 "src/parser_bison.c"
    break;

  case 347: /* map_block: map_block "gc-interval" time_spec stmt_separator  */
#line 2434 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->gc_int = (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 11061 "src/parser_bison.c"
    break;

  case 348: /* map_block: map_block "type" data_type_expr "colon" map_block_data_interval data_type_expr stmt_separator close_scope_type  */
#line 2441 "src/parser_bison.y"
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
#line 11080 "src/parser_bison.c"
    break;

  case 349: /* map_block: map_block "typeof" typeof_expr "colon" typeof_data_expr stmt_separator  */
#line 2458 "src/parser_bison.y"
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
#line 11105 "src/parser_bison.c"
    break;

  case 350: /* map_block: map_block "type" data_type_expr "colon" map_block_obj_type stmt_separator close_scope_type  */
#line 2481 "src/parser_bison.y"
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
#line 11121 "src/parser_bison.c"
    break;

  case 351: /* map_block: map_block "typeof" typeof_expr "colon" map_block_obj_typeof stmt_separator  */
#line 2495 "src/parser_bison.y"
                        {
				(yyvsp[-5].set)->key = (yyvsp[-3].expr);
				(yyvsp[-5].set)->objtype = (yyvsp[-1].val);
				(yyvsp[-5].set)->flags  |= NFT_SET_OBJECT;
				(yyval.set) = (yyvsp[-5].set);
			}
#line 11132 "src/parser_bison.c"
    break;

  case 352: /* map_block: map_block "flags" set_flag_list stmt_separator  */
#line 2502 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->flags |= (yyvsp[-1].val);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 11141 "src/parser_bison.c"
    break;

  case 353: /* map_block: map_block stateful_stmt_list stmt_separator  */
#line 2507 "src/parser_bison.y"
                        {
				list_splice_tail((yyvsp[-1].list), &(yyvsp[-2].set)->stmt_list);
				(yyval.set) = (yyvsp[-2].set);
				free((yyvsp[-1].list));
			}
#line 11151 "src/parser_bison.c"
    break;

  case 354: /* map_block: map_block "elements" '=' set_block_expr  */
#line 2513 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->init = (yyvsp[0].expr);
				(yyval.set) = (yyvsp[-3].set);
			}
#line 11160 "src/parser_bison.c"
    break;

  case 355: /* map_block: map_block comment_spec stmt_separator  */
#line 2518 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-2].set)->comment, &(yylsp[-1]), state)) {
					free_const((yyvsp[-1].string));
					YYERROR;
				}
				(yyvsp[-2].set)->comment = (yyvsp[-1].string);
				(yyval.set) = (yyvsp[-2].set);
			}
#line 11173 "src/parser_bison.c"
    break;

  case 357: /* set_mechanism: "policy" set_policy_spec close_scope_policy  */
#line 2530 "src/parser_bison.y"
                        {
				(yyvsp[-3].set)->policy = (yyvsp[-1].val);
			}
#line 11181 "src/parser_bison.c"
    break;

  case 358: /* set_mechanism: "size" "number"  */
#line 2534 "src/parser_bison.y"
                        {
				(yyvsp[-2].set)->desc.size = (yyvsp[0].val);
			}
#line 11189 "src/parser_bison.c"
    break;

  case 359: /* set_policy_spec: "performance"  */
#line 2539 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_PERFORMANCE; }
#line 11195 "src/parser_bison.c"
    break;

  case 360: /* set_policy_spec: "memory"  */
#line 2540 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SET_POL_MEMORY; }
#line 11201 "src/parser_bison.c"
    break;

  case 361: /* flowtable_block_alloc: %empty  */
#line 2544 "src/parser_bison.y"
                        {
				(yyval.flowtable) = flowtable_alloc(&internal_location);
			}
#line 11209 "src/parser_bison.c"
    break;

  case 362: /* flowtable_block: %empty  */
#line 2549 "src/parser_bison.y"
                                                { (yyval.flowtable) = (yyvsp[(-1) - (0)].flowtable); }
#line 11215 "src/parser_bison.c"
    break;

  case 365: /* flowtable_block: flowtable_block "hook" "string" prio_spec stmt_separator  */
#line 2553 "src/parser_bison.y"
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
#line 11234 "src/parser_bison.c"
    break;

  case 366: /* flowtable_block: flowtable_block "devices" '=' flowtable_expr stmt_separator  */
#line 2568 "src/parser_bison.y"
                        {
				(yyval.flowtable)->dev_expr = (yyvsp[-1].expr);
			}
#line 11242 "src/parser_bison.c"
    break;

  case 367: /* flowtable_block: flowtable_block "counter" close_scope_counter  */
#line 2572 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= NFT_FLOWTABLE_COUNTER;
			}
#line 11250 "src/parser_bison.c"
    break;

  case 368: /* flowtable_block: flowtable_block "flags" "offload" stmt_separator  */
#line 2576 "src/parser_bison.y"
                        {
				(yyval.flowtable)->flags |= FLOWTABLE_F_HW_OFFLOAD;
			}
#line 11258 "src/parser_bison.c"
    break;

  case 369: /* flowtable_expr: '{' flowtable_list_expr '}'  */
#line 2582 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 11267 "src/parser_bison.c"
    break;

  case 370: /* flowtable_expr: variable_expr  */
#line 2587 "src/parser_bison.y"
                        {
				(yyvsp[0].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11276 "src/parser_bison.c"
    break;

  case 371: /* flowtable_list_expr: flowtable_expr_member  */
#line 2594 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11285 "src/parser_bison.c"
    break;

  case 372: /* flowtable_list_expr: flowtable_list_expr "comma" flowtable_expr_member  */
#line 2599 "src/parser_bison.y"
                        {
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 11294 "src/parser_bison.c"
    break;

  case 374: /* flowtable_expr_member: string  */
#line 2607 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = expr;
			}
#line 11307 "src/parser_bison.c"
    break;

  case 375: /* flowtable_expr_member: variable_expr  */
#line 2616 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11316 "src/parser_bison.c"
    break;

  case 376: /* data_type_atom_expr: type_identifier  */
#line 2623 "src/parser_bison.y"
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
#line 11333 "src/parser_bison.c"
    break;

  case 377: /* data_type_atom_expr: "time"  */
#line 2636 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yylsp[0]), &time_type, time_type.byteorder,
							 time_type.size, NULL);
			}
#line 11342 "src/parser_bison.c"
    break;

  case 379: /* data_type_expr: data_type_expr "." data_type_atom_expr  */
#line 2644 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 11355 "src/parser_bison.c"
    break;

  case 380: /* obj_block_alloc: %empty  */
#line 2655 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&internal_location);
			}
#line 11363 "src/parser_bison.c"
    break;

  case 381: /* counter_block: %empty  */
#line 2660 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11369 "src/parser_bison.c"
    break;

  case 384: /* counter_block: counter_block counter_config  */
#line 2664 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11377 "src/parser_bison.c"
    break;

  case 385: /* counter_block: counter_block comment_spec  */
#line 2668 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11389 "src/parser_bison.c"
    break;

  case 386: /* quota_block: %empty  */
#line 2677 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11395 "src/parser_bison.c"
    break;

  case 389: /* quota_block: quota_block quota_config  */
#line 2681 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11403 "src/parser_bison.c"
    break;

  case 390: /* quota_block: quota_block comment_spec  */
#line 2685 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11415 "src/parser_bison.c"
    break;

  case 391: /* ct_helper_block: %empty  */
#line 2694 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11421 "src/parser_bison.c"
    break;

  case 394: /* ct_helper_block: ct_helper_block ct_helper_config  */
#line 2698 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11429 "src/parser_bison.c"
    break;

  case 395: /* ct_helper_block: ct_helper_block comment_spec  */
#line 2702 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11441 "src/parser_bison.c"
    break;

  case 396: /* ct_timeout_block: %empty  */
#line 2712 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);
				init_list_head(&(yyval.obj)->ct_timeout.timeout_list);
				(yyval.obj)->type = NFT_OBJECT_CT_TIMEOUT;
			}
#line 11451 "src/parser_bison.c"
    break;

  case 399: /* ct_timeout_block: ct_timeout_block ct_timeout_config  */
#line 2720 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11459 "src/parser_bison.c"
    break;

  case 400: /* ct_timeout_block: ct_timeout_block comment_spec  */
#line 2724 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11471 "src/parser_bison.c"
    break;

  case 401: /* ct_expect_block: %empty  */
#line 2733 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11477 "src/parser_bison.c"
    break;

  case 404: /* ct_expect_block: ct_expect_block ct_expect_config  */
#line 2737 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11485 "src/parser_bison.c"
    break;

  case 405: /* ct_expect_block: ct_expect_block comment_spec  */
#line 2741 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11497 "src/parser_bison.c"
    break;

  case 406: /* connlimit_block: %empty  */
#line 2750 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11503 "src/parser_bison.c"
    break;

  case 409: /* connlimit_block: connlimit_block connlimit_config  */
#line 2754 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11511 "src/parser_bison.c"
    break;

  case 410: /* connlimit_block: connlimit_block comment_spec  */
#line 2758 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11523 "src/parser_bison.c"
    break;

  case 411: /* limit_block: %empty  */
#line 2767 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11529 "src/parser_bison.c"
    break;

  case 414: /* limit_block: limit_block limit_config  */
#line 2771 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11537 "src/parser_bison.c"
    break;

  case 415: /* limit_block: limit_block comment_spec  */
#line 2775 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11549 "src/parser_bison.c"
    break;

  case 416: /* secmark_block: %empty  */
#line 2784 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11555 "src/parser_bison.c"
    break;

  case 419: /* secmark_block: secmark_block secmark_config  */
#line 2788 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11563 "src/parser_bison.c"
    break;

  case 420: /* secmark_block: secmark_block comment_spec  */
#line 2792 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11575 "src/parser_bison.c"
    break;

  case 421: /* synproxy_block: %empty  */
#line 2801 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 11581 "src/parser_bison.c"
    break;

  case 424: /* synproxy_block: synproxy_block synproxy_config  */
#line 2805 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-1].obj);
			}
#line 11589 "src/parser_bison.c"
    break;

  case 425: /* synproxy_block: synproxy_block comment_spec  */
#line 2809 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 11601 "src/parser_bison.c"
    break;

  case 426: /* type_identifier: "string"  */
#line 2818 "src/parser_bison.y"
                                        { (yyval.string) = (yyvsp[0].string); }
#line 11607 "src/parser_bison.c"
    break;

  case 427: /* type_identifier: "mark"  */
#line 2819 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("mark"); }
#line 11613 "src/parser_bison.c"
    break;

  case 428: /* type_identifier: "dscp"  */
#line 2820 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("dscp"); }
#line 11619 "src/parser_bison.c"
    break;

  case 429: /* type_identifier: "ecn"  */
#line 2821 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("ecn"); }
#line 11625 "src/parser_bison.c"
    break;

  case 430: /* type_identifier: "classid"  */
#line 2822 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("classid"); }
#line 11631 "src/parser_bison.c"
    break;

  case 431: /* hook_spec: "type" chain_type close_scope_type "hook" "string" dev_spec prio_spec  */
#line 2826 "src/parser_bison.y"
                        {
				(yyvsp[-7].chain)->type.loc = (yylsp[-4]);
				(yyvsp[-7].chain)->type.str = (yyvsp[-5].string);

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
#line 11657 "src/parser_bison.c"
    break;

  case 432: /* chain_type: "filter"  */
#line 2849 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("filter"); }
#line 11663 "src/parser_bison.c"
    break;

  case 433: /* chain_type: "nat"  */
#line 2850 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("nat"); }
#line 11669 "src/parser_bison.c"
    break;

  case 434: /* chain_type: "route"  */
#line 2851 "src/parser_bison.y"
                                        { (yyval.string) = xstrdup("route"); }
#line 11675 "src/parser_bison.c"
    break;

  case 435: /* prio_spec: "priority" extended_prio_spec  */
#line 2855 "src/parser_bison.y"
                        {
				(yyval.prio_spec) = (yyvsp[0].prio_spec);
				(yyval.prio_spec).loc = (yyloc);
			}
#line 11684 "src/parser_bison.c"
    break;

  case 436: /* extended_prio_name: "out"  */
#line 2862 "src/parser_bison.y"
                        {
				(yyval.string) = strdup("out");
			}
#line 11692 "src/parser_bison.c"
    break;

  case 438: /* extended_prio_spec: int_num  */
#line 2869 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &integer_type,
								BYTEORDER_HOST_ENDIAN,
								sizeof(int) *
								BITS_PER_BYTE, &(yyvsp[0].val32));
				(yyval.prio_spec) = spec;
			}
#line 11706 "src/parser_bison.c"
    break;

  case 439: /* extended_prio_spec: variable_expr  */
#line 2879 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = (yyvsp[0].expr);
				(yyval.prio_spec) = spec;
			}
#line 11717 "src/parser_bison.c"
    break;

  case 440: /* extended_prio_spec: extended_prio_name  */
#line 2886 "src/parser_bison.y"
                        {
				struct prio_spec spec = {0};

				spec.expr = constant_expr_alloc(&(yyloc), &string_type,
								BYTEORDER_HOST_ENDIAN,
								strlen((yyvsp[0].string)) * BITS_PER_BYTE,
								(yyvsp[0].string));
				free_const((yyvsp[0].string));
				(yyval.prio_spec) = spec;
			}
#line 11732 "src/parser_bison.c"
    break;

  case 441: /* extended_prio_spec: extended_prio_name "+" "number"  */
#line 2897 "src/parser_bison.y"
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
#line 11749 "src/parser_bison.c"
    break;

  case 442: /* extended_prio_spec: extended_prio_name "-" "number"  */
#line 2910 "src/parser_bison.y"
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
#line 11766 "src/parser_bison.c"
    break;

  case 443: /* int_num: "number"  */
#line 2924 "src/parser_bison.y"
                                                        { (yyval.val32) = (yyvsp[0].val); }
#line 11772 "src/parser_bison.c"
    break;

  case 444: /* int_num: "-" "number"  */
#line 2925 "src/parser_bison.y"
                                                        { (yyval.val32) = -(yyvsp[0].val); }
#line 11778 "src/parser_bison.c"
    break;

  case 445: /* dev_spec: "device" string  */
#line 2929 "src/parser_bison.y"
                        {
				struct expr *expr = ifname_expr_alloc(&(yyloc), state->msgs, (yyvsp[0].string));

				if (!expr)
					YYERROR;

				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), expr);

			}
#line 11793 "src/parser_bison.c"
    break;

  case 446: /* dev_spec: "device" variable_expr  */
#line 2940 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &ifname_type);
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 11803 "src/parser_bison.c"
    break;

  case 447: /* dev_spec: "devices" '=' flowtable_expr  */
#line 2946 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11811 "src/parser_bison.c"
    break;

  case 448: /* dev_spec: %empty  */
#line 2949 "src/parser_bison.y"
                                                        { (yyval.expr) = NULL; }
#line 11817 "src/parser_bison.c"
    break;

  case 449: /* flags_spec: "flags" "offload"  */
#line 2953 "src/parser_bison.y"
                        {
				(yyvsp[-2].chain)->flags |= CHAIN_F_HW_OFFLOAD;
			}
#line 11825 "src/parser_bison.c"
    break;

  case 450: /* policy_spec: "policy" policy_expr close_scope_policy  */
#line 2959 "src/parser_bison.y"
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
#line 11841 "src/parser_bison.c"
    break;

  case 451: /* policy_expr: variable_expr  */
#line 2973 "src/parser_bison.y"
                        {
				datatype_set((yyvsp[0].expr)->sym->expr, &policy_type);
				(yyval.expr) = (yyvsp[0].expr);
			}
#line 11850 "src/parser_bison.c"
    break;

  case 452: /* policy_expr: chain_policy  */
#line 2978 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(int) *
							 BITS_PER_BYTE, &(yyvsp[0].val32));
			}
#line 11861 "src/parser_bison.c"
    break;

  case 453: /* chain_policy: "accept"  */
#line 2986 "src/parser_bison.y"
                                                { (yyval.val32) = NF_ACCEPT; }
#line 11867 "src/parser_bison.c"
    break;

  case 454: /* chain_policy: "drop"  */
#line 2987 "src/parser_bison.y"
                                                { (yyval.val32) = NF_DROP;   }
#line 11873 "src/parser_bison.c"
    break;

  case 456: /* identifier: "last"  */
#line 2991 "src/parser_bison.y"
                                                { (yyval.string) = xstrdup("last"); }
#line 11879 "src/parser_bison.c"
    break;

  case 460: /* time_spec: "string"  */
#line 3000 "src/parser_bison.y"
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
#line 11896 "src/parser_bison.c"
    break;

  case 462: /* time_spec_or_num_s: time_spec  */
#line 3016 "src/parser_bison.y"
                                          { (yyval.val) = (yyvsp[0].val) / 1000u; }
#line 11902 "src/parser_bison.c"
    break;

  case 463: /* family_spec: %empty  */
#line 3019 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11908 "src/parser_bison.c"
    break;

  case 465: /* family_spec_explicit: "ip" close_scope_ip  */
#line 3023 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV4; }
#line 11914 "src/parser_bison.c"
    break;

  case 466: /* family_spec_explicit: "ip6" close_scope_ip6  */
#line 3024 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 11920 "src/parser_bison.c"
    break;

  case 467: /* family_spec_explicit: "inet"  */
#line 3025 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_INET; }
#line 11926 "src/parser_bison.c"
    break;

  case 468: /* family_spec_explicit: "arp" close_scope_arp  */
#line 3026 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_ARP; }
#line 11932 "src/parser_bison.c"
    break;

  case 469: /* family_spec_explicit: "bridge"  */
#line 3027 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_BRIDGE; }
#line 11938 "src/parser_bison.c"
    break;

  case 470: /* family_spec_explicit: "netdev"  */
#line 3028 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_NETDEV; }
#line 11944 "src/parser_bison.c"
    break;

  case 471: /* table_spec: family_spec identifier  */
#line 3032 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[-1].val);
				(yyval.handle).table.location = (yylsp[0]);
				(yyval.handle).table.name	= (yyvsp[0].string);
			}
#line 11955 "src/parser_bison.c"
    break;

  case 472: /* tableid_spec: family_spec "handle" "number"  */
#line 3041 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family 		= (yyvsp[-2].val);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
				(yyval.handle).handle.location	= (yylsp[0]);
			}
#line 11966 "src/parser_bison.c"
    break;

  case 473: /* chain_spec: table_spec identifier  */
#line 3050 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).chain.name	= (yyvsp[0].string);
				(yyval.handle).chain.location = (yylsp[0]);
			}
#line 11976 "src/parser_bison.c"
    break;

  case 474: /* chainid_spec: table_spec "handle" "number"  */
#line 3058 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 11986 "src/parser_bison.c"
    break;

  case 475: /* chain_identifier: identifier  */
#line 3066 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).chain.name		= (yyvsp[0].string);
				(yyval.handle).chain.location	= (yylsp[0]);
			}
#line 11996 "src/parser_bison.c"
    break;

  case 476: /* set_spec: table_spec identifier  */
#line 3074 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 12006 "src/parser_bison.c"
    break;

  case 477: /* setid_spec: table_spec "handle" "number"  */
#line 3082 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location 	= (yylsp[0]);
				(yyval.handle).handle.id 		= (yyvsp[0].val);
			}
#line 12016 "src/parser_bison.c"
    break;

  case 478: /* set_identifier: identifier  */
#line 3090 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).set.name	= (yyvsp[0].string);
				(yyval.handle).set.location	= (yylsp[0]);
			}
#line 12026 "src/parser_bison.c"
    break;

  case 479: /* flowtable_spec: table_spec identifier  */
#line 3098 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-1].handle);
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 12036 "src/parser_bison.c"
    break;

  case 480: /* flowtableid_spec: table_spec "handle" "number"  */
#line 3106 "src/parser_bison.y"
                        {
				(yyval.handle)			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 12046 "src/parser_bison.c"
    break;

  case 481: /* flowtable_identifier: identifier  */
#line 3114 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).flowtable.name	= (yyvsp[0].string);
				(yyval.handle).flowtable.location	= (yylsp[0]);
			}
#line 12056 "src/parser_bison.c"
    break;

  case 482: /* obj_spec: table_spec identifier  */
#line 3122 "src/parser_bison.y"
                        {
				(yyval.handle)		= (yyvsp[-1].handle);
				(yyval.handle).obj.name	= (yyvsp[0].string);
				(yyval.handle).obj.location	= (yylsp[0]);
			}
#line 12066 "src/parser_bison.c"
    break;

  case 483: /* objid_spec: table_spec "handle" "number"  */
#line 3130 "src/parser_bison.y"
                        {
				(yyval.handle) 			= (yyvsp[-2].handle);
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 12076 "src/parser_bison.c"
    break;

  case 484: /* obj_identifier: identifier  */
#line 3138 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).obj.name		= (yyvsp[0].string);
				(yyval.handle).obj.location		= (yylsp[0]);
			}
#line 12086 "src/parser_bison.c"
    break;

  case 485: /* handle_spec: "handle" "number"  */
#line 3146 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).handle.location	= (yylsp[0]);
				(yyval.handle).handle.id		= (yyvsp[0].val);
			}
#line 12096 "src/parser_bison.c"
    break;

  case 486: /* position_spec: "position" "number"  */
#line 3154 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).position.location	= (yyloc);
				(yyval.handle).position.id		= (yyvsp[0].val);
			}
#line 12106 "src/parser_bison.c"
    break;

  case 487: /* index_spec: "index" "number"  */
#line 3162 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).index.location	= (yyloc);
				(yyval.handle).index.id		= (yyvsp[0].val) + 1;
			}
#line 12116 "src/parser_bison.c"
    break;

  case 488: /* rule_position: chain_spec  */
#line 3170 "src/parser_bison.y"
                        {
				(yyval.handle) = (yyvsp[0].handle);
			}
#line 12124 "src/parser_bison.c"
    break;

  case 489: /* rule_position: chain_spec position_spec  */
#line 3174 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 12133 "src/parser_bison.c"
    break;

  case 490: /* rule_position: chain_spec handle_spec  */
#line 3179 "src/parser_bison.y"
                        {
				(yyvsp[0].handle).position.location = (yyvsp[0].handle).handle.location;
				(yyvsp[0].handle).position.id = (yyvsp[0].handle).handle.id;
				(yyvsp[0].handle).handle.id = 0;
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 12145 "src/parser_bison.c"
    break;

  case 491: /* rule_position: chain_spec index_spec  */
#line 3187 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 12154 "src/parser_bison.c"
    break;

  case 492: /* ruleid_spec: chain_spec handle_spec  */
#line 3194 "src/parser_bison.y"
                        {
				handle_merge(&(yyvsp[-1].handle), &(yyvsp[0].handle));
				(yyval.handle) = (yyvsp[-1].handle);
			}
#line 12163 "src/parser_bison.c"
    break;

  case 493: /* comment_spec: "comment" string  */
#line 3201 "src/parser_bison.y"
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
#line 12178 "src/parser_bison.c"
    break;

  case 494: /* ruleset_spec: %empty  */
#line 3214 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= NFPROTO_UNSPEC;
			}
#line 12187 "src/parser_bison.c"
    break;

  case 495: /* ruleset_spec: family_spec_explicit  */
#line 3219 "src/parser_bison.y"
                        {
				memset(&(yyval.handle), 0, sizeof((yyval.handle)));
				(yyval.handle).family	= (yyvsp[0].val);
			}
#line 12196 "src/parser_bison.c"
    break;

  case 496: /* rule: rule_alloc  */
#line 3226 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = NULL;
			}
#line 12204 "src/parser_bison.c"
    break;

  case 497: /* rule: rule_alloc comment_spec  */
#line 3230 "src/parser_bison.y"
                        {
				(yyval.rule)->comment = (yyvsp[0].string);
			}
#line 12212 "src/parser_bison.c"
    break;

  case 498: /* rule_alloc: stmt_list  */
#line 3236 "src/parser_bison.y"
                        {
				struct stmt *i;

				(yyval.rule) = rule_alloc(&(yyloc), NULL);
				list_for_each_entry(i, (yyvsp[0].list), list)
					(yyval.rule)->num_stmts++;
				list_splice_tail((yyvsp[0].list), &(yyval.rule)->stmts);
				free((yyvsp[0].list));
			}
#line 12226 "src/parser_bison.c"
    break;

  case 499: /* stmt_list: stmt  */
#line 3248 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 12236 "src/parser_bison.c"
    break;

  case 500: /* stmt_list: stmt_list stmt  */
#line 3254 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 12245 "src/parser_bison.c"
    break;

  case 501: /* stateful_stmt_list: stateful_stmt  */
#line 3261 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 12255 "src/parser_bison.c"
    break;

  case 502: /* stateful_stmt_list: stateful_stmt_list stateful_stmt  */
#line 3267 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 12264 "src/parser_bison.c"
    break;

  case 503: /* objref_stmt_counter: "counter" "name" stmt_expr close_scope_counter  */
#line 3274 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_COUNTER;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12274 "src/parser_bison.c"
    break;

  case 504: /* objref_stmt_limit: "limit" "name" stmt_expr close_scope_limit  */
#line 3282 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_LIMIT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12284 "src/parser_bison.c"
    break;

  case 505: /* objref_stmt_quota: "quota" "name" stmt_expr close_scope_quota  */
#line 3290 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_QUOTA;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12294 "src/parser_bison.c"
    break;

  case 506: /* objref_stmt_synproxy: "synproxy" "name" stmt_expr close_scope_synproxy  */
#line 3298 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_SYNPROXY;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12304 "src/parser_bison.c"
    break;

  case 507: /* objref_stmt_tunnel: "tunnel" "name" stmt_expr close_scope_tunnel  */
#line 3306 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_TUNNEL;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12314 "src/parser_bison.c"
    break;

  case 508: /* objref_stmt_ct: "ct" "timeout" "set" stmt_expr close_scope_ct  */
#line 3314 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_TIMEOUT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);

			}
#line 12325 "src/parser_bison.c"
    break;

  case 509: /* objref_stmt_ct: "ct" "expectation" "set" stmt_expr close_scope_ct  */
#line 3321 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CT_EXPECT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12335 "src/parser_bison.c"
    break;

  case 510: /* objref_stmt_ct: "ct" "count" "name" stmt_expr close_scope_ct  */
#line 3327 "src/parser_bison.y"
                        {
				(yyval.stmt) = objref_stmt_alloc(&(yyloc));
				(yyval.stmt)->objref.type = NFT_OBJECT_CONNLIMIT;
				(yyval.stmt)->objref.expr = (yyvsp[-1].expr);
			}
#line 12345 "src/parser_bison.c"
    break;

  case 545: /* xt_stmt: "xt" "string" string  */
#line 3375 "src/parser_bison.y"
                        {
				(yyval.stmt) = NULL;
				free_const((yyvsp[-1].string));
				free_const((yyvsp[0].string));
				erec_queue(error(&(yyloc), "unsupported xtables compat expression, use iptables-nft with this ruleset"),
					   state->msgs);
				YYERROR;
			}
#line 12358 "src/parser_bison.c"
    break;

  case 546: /* chain_stmt_type: "jump"  */
#line 3385 "src/parser_bison.y"
                                        { (yyval.val) = NFT_JUMP; }
#line 12364 "src/parser_bison.c"
    break;

  case 547: /* chain_stmt_type: "goto"  */
#line 3386 "src/parser_bison.y"
                                        { (yyval.val) = NFT_GOTO; }
#line 12370 "src/parser_bison.c"
    break;

  case 548: /* chain_stmt: chain_stmt_type chain_block_alloc '{' subchain_block '}'  */
#line 3390 "src/parser_bison.y"
                        {
				(yyvsp[-3].chain)->location = (yylsp[-3]);
				close_scope(state);
				(yyvsp[-1].chain)->location = (yylsp[-1]);
				(yyval.stmt) = chain_stmt_alloc(&(yyloc), (yyvsp[-1].chain), (yyvsp[-4].val));
			}
#line 12381 "src/parser_bison.c"
    break;

  case 549: /* verdict_stmt: verdict_expr  */
#line 3399 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 12389 "src/parser_bison.c"
    break;

  case 550: /* verdict_stmt: verdict_map_stmt  */
#line 3403 "src/parser_bison.y"
                        {
				(yyval.stmt) = verdict_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 12397 "src/parser_bison.c"
    break;

  case 551: /* verdict_map_stmt: concat_expr "vmap" verdict_map_expr  */
#line 3409 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 12405 "src/parser_bison.c"
    break;

  case 552: /* verdict_map_expr: '{' verdict_map_list_expr '}'  */
#line 3415 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 12414 "src/parser_bison.c"
    break;

  case 554: /* verdict_map_list_expr: verdict_map_list_member_expr  */
#line 3423 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				set_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 12423 "src/parser_bison.c"
    break;

  case 555: /* verdict_map_list_expr: verdict_map_list_expr "comma" verdict_map_list_member_expr  */
#line 3428 "src/parser_bison.y"
                        {
				set_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 12432 "src/parser_bison.c"
    break;

  case 557: /* verdict_map_list_member_expr: opt_newline set_elem_expr "colon" verdict_expr opt_newline  */
#line 3436 "src/parser_bison.y"
                        {
				struct expr *expr = (yyvsp[-3].expr);

				expr->key = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr)->key, (yyvsp[-1].expr));
				(yyval.expr) = expr;
			}
#line 12443 "src/parser_bison.c"
    break;

  case 558: /* ct_limit_stmt_alloc: "ct" "count"  */
#line 3445 "src/parser_bison.y"
                        {
				(yyval.stmt) = connlimit_stmt_alloc(&(yyloc));
			}
#line 12451 "src/parser_bison.c"
    break;

  case 559: /* connlimit_obj: %empty  */
#line 3451 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_CONNLIMIT;
			}
#line 12460 "src/parser_bison.c"
    break;

  case 560: /* connlimit_config: "until" "number"  */
#line 3458 "src/parser_bison.y"
                        {
				struct connlimit *connlimit;

				connlimit = &(yyvsp[-2].obj)->connlimit;
				connlimit->count = (yyvsp[0].val);
				connlimit->flags = 0;

			}
#line 12473 "src/parser_bison.c"
    break;

  case 561: /* connlimit_config: "over" "number"  */
#line 3467 "src/parser_bison.y"
                        {
				struct connlimit *connlimit;

				connlimit = &(yyvsp[-2].obj)->connlimit;
				connlimit->count = (yyvsp[0].val);
				connlimit->flags = NFT_CONNLIMIT_F_INV;
			}
#line 12485 "src/parser_bison.c"
    break;

  case 563: /* ct_limit_args: "number"  */
#line 3480 "src/parser_bison.y"
                        {
				assert((yyvsp[-1].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-1].stmt)->connlimit.count	= (yyvsp[0].val);
			}
#line 12495 "src/parser_bison.c"
    break;

  case 564: /* ct_limit_args: "over" "number"  */
#line 3486 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_CONNLIMIT);

				(yyvsp[-2].stmt)->connlimit.count = (yyvsp[0].val);
				(yyvsp[-2].stmt)->connlimit.flags = NFT_CONNLIMIT_F_INV;
			}
#line 12506 "src/parser_bison.c"
    break;

  case 567: /* counter_stmt_alloc: "counter"  */
#line 3498 "src/parser_bison.y"
                        {
				(yyval.stmt) = counter_stmt_alloc(&(yyloc));
			}
#line 12514 "src/parser_bison.c"
    break;

  case 568: /* counter_args: counter_arg  */
#line 3504 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12522 "src/parser_bison.c"
    break;

  case 570: /* counter_arg: "packets" "number"  */
#line 3511 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.packets = (yyvsp[0].val);
			}
#line 12531 "src/parser_bison.c"
    break;

  case 571: /* counter_arg: "bytes" "number"  */
#line 3516 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].stmt)->type == STMT_COUNTER);
				(yyvsp[-2].stmt)->counter.bytes	 = (yyvsp[0].val);
			}
#line 12540 "src/parser_bison.c"
    break;

  case 572: /* last_stmt_alloc: "last"  */
#line 3523 "src/parser_bison.y"
                        {
				(yyval.stmt) = last_stmt_alloc(&(yyloc));
			}
#line 12548 "src/parser_bison.c"
    break;

  case 576: /* last_args: "used" time_spec  */
#line 3534 "src/parser_bison.y"
                        {
				struct last_stmt *last;

				assert((yyvsp[-2].stmt)->type == STMT_LAST);
				last = &(yyvsp[-2].stmt)->last;
				last->used = (yyvsp[0].val);
				last->set = true;
			}
#line 12561 "src/parser_bison.c"
    break;

  case 579: /* log_stmt_alloc: "log"  */
#line 3549 "src/parser_bison.y"
                        {
				(yyval.stmt) = log_stmt_alloc(&(yyloc));
			}
#line 12569 "src/parser_bison.c"
    break;

  case 580: /* log_args: log_arg  */
#line 3555 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 12577 "src/parser_bison.c"
    break;

  case 582: /* log_arg: "prefix" string  */
#line 3562 "src/parser_bison.y"
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
#line 12598 "src/parser_bison.c"
    break;

  case 583: /* log_arg: "group" "number"  */
#line 3579 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.group	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_GROUP;
			}
#line 12607 "src/parser_bison.c"
    break;

  case 584: /* log_arg: "snaplen" "number"  */
#line 3584 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.snaplen	 = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_SNAPLEN;
			}
#line 12616 "src/parser_bison.c"
    break;

  case 585: /* log_arg: "queue-threshold" "number"  */
#line 3589 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.qthreshold = (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_QTHRESHOLD;
			}
#line 12625 "src/parser_bison.c"
    break;

  case 586: /* log_arg: "level" level_type  */
#line 3594 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.level	= (yyvsp[0].val);
				(yyvsp[-2].stmt)->log.flags 	|= STMT_LOG_LEVEL;
			}
#line 12634 "src/parser_bison.c"
    break;

  case 587: /* log_arg: "flags" log_flags  */
#line 3599 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->log.logflags	|= (yyvsp[0].val);
			}
#line 12642 "src/parser_bison.c"
    break;

  case 588: /* level_type: "emerg"  */
#line 3604 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_EMERG; }
#line 12648 "src/parser_bison.c"
    break;

  case 589: /* level_type: "alert"  */
#line 3605 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_ALERT; }
#line 12654 "src/parser_bison.c"
    break;

  case 590: /* level_type: "crit"  */
#line 3606 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_CRIT; }
#line 12660 "src/parser_bison.c"
    break;

  case 591: /* level_type: "err"  */
#line 3607 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_ERR; }
#line 12666 "src/parser_bison.c"
    break;

  case 592: /* level_type: "warn"  */
#line 3608 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_WARNING; }
#line 12672 "src/parser_bison.c"
    break;

  case 593: /* level_type: "notice"  */
#line 3609 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_NOTICE; }
#line 12678 "src/parser_bison.c"
    break;

  case 594: /* level_type: "info"  */
#line 3610 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_INFO; }
#line 12684 "src/parser_bison.c"
    break;

  case 595: /* level_type: "debug"  */
#line 3611 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_DEBUG; }
#line 12690 "src/parser_bison.c"
    break;

  case 596: /* level_type: "audit"  */
#line 3612 "src/parser_bison.y"
                                                { (yyval.val) = NFT_LOGLEVEL_AUDIT; }
#line 12696 "src/parser_bison.c"
    break;

  case 597: /* log_flags: "tcp" log_flags_tcp close_scope_tcp  */
#line 3616 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-1].val);
			}
#line 12704 "src/parser_bison.c"
    break;

  case 598: /* log_flags: "ip" "options" close_scope_ip  */
#line 3620 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_IPOPT;
			}
#line 12712 "src/parser_bison.c"
    break;

  case 599: /* log_flags: "skuid"  */
#line 3624 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_UID;
			}
#line 12720 "src/parser_bison.c"
    break;

  case 600: /* log_flags: "ether" close_scope_eth  */
#line 3628 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MACDECODE;
			}
#line 12728 "src/parser_bison.c"
    break;

  case 601: /* log_flags: "all"  */
#line 3632 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_MASK;
			}
#line 12736 "src/parser_bison.c"
    break;

  case 602: /* log_flags_tcp: log_flags_tcp "comma" log_flag_tcp  */
#line 3638 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 12744 "src/parser_bison.c"
    break;

  case 604: /* log_flag_tcp: "seq"  */
#line 3645 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPSEQ;
			}
#line 12752 "src/parser_bison.c"
    break;

  case 605: /* log_flag_tcp: "options"  */
#line 3649 "src/parser_bison.y"
                        {
				(yyval.val) = NF_LOG_TCPOPT;
			}
#line 12760 "src/parser_bison.c"
    break;

  case 606: /* limit_stmt_alloc: "limit" "rate"  */
#line 3655 "src/parser_bison.y"
                        {
				(yyval.stmt) = limit_stmt_alloc(&(yyloc));
			}
#line 12768 "src/parser_bison.c"
    break;

  case 608: /* limit_args: limit_mode limit_rate_pkts limit_burst_pkts  */
#line 3664 "src/parser_bison.y"
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
#line 12790 "src/parser_bison.c"
    break;

  case 609: /* limit_args: limit_mode limit_rate_bytes limit_burst_bytes  */
#line 3682 "src/parser_bison.y"
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
#line 12807 "src/parser_bison.c"
    break;

  case 610: /* quota_mode: "over"  */
#line 3696 "src/parser_bison.y"
                                                { (yyval.val) = NFT_QUOTA_F_INV; }
#line 12813 "src/parser_bison.c"
    break;

  case 611: /* quota_mode: "until"  */
#line 3697 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12819 "src/parser_bison.c"
    break;

  case 612: /* quota_mode: %empty  */
#line 3698 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12825 "src/parser_bison.c"
    break;

  case 613: /* bytes_unit: "bytes"  */
#line 3701 "src/parser_bison.y"
                                                { (yyval.val) = 1; }
#line 12831 "src/parser_bison.c"
    break;

  case 614: /* bytes_unit: "kbytes"  */
#line 3702 "src/parser_bison.y"
                                                { (yyval.val) = 1024; }
#line 12837 "src/parser_bison.c"
    break;

  case 615: /* bytes_unit: "mbytes"  */
#line 3703 "src/parser_bison.y"
                                                { (yyval.val) = 1024 * 1024; }
#line 12843 "src/parser_bison.c"
    break;

  case 616: /* quota_used: %empty  */
#line 3706 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 12849 "src/parser_bison.c"
    break;

  case 617: /* quota_used: "used" "number" bytes_unit  */
#line 3708 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-1].val) * (yyvsp[0].val);
			}
#line 12857 "src/parser_bison.c"
    break;

  case 618: /* quota_stmt_alloc: "quota"  */
#line 3714 "src/parser_bison.y"
                        {
				(yyval.stmt) = quota_stmt_alloc(&(yyloc));
			}
#line 12865 "src/parser_bison.c"
    break;

  case 620: /* quota_args: quota_mode "number" bytes_unit quota_used  */
#line 3723 "src/parser_bison.y"
                        {
				struct quota_stmt *quota;

				assert((yyvsp[-4].stmt)->type == STMT_QUOTA);

				quota = &(yyvsp[-4].stmt)->quota;
				quota->bytes = (yyvsp[-2].val) * (yyvsp[-1].val);
				quota->used = (yyvsp[0].val);
				quota->flags = (yyvsp[-3].val);
			}
#line 12880 "src/parser_bison.c"
    break;

  case 621: /* limit_mode: "over"  */
#line 3735 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_LIMIT_F_INV; }
#line 12886 "src/parser_bison.c"
    break;

  case 622: /* limit_mode: "until"  */
#line 3736 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12892 "src/parser_bison.c"
    break;

  case 623: /* limit_mode: %empty  */
#line 3737 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12898 "src/parser_bison.c"
    break;

  case 624: /* limit_burst_pkts: %empty  */
#line 3740 "src/parser_bison.y"
                                                                { (yyval.val) = 5; }
#line 12904 "src/parser_bison.c"
    break;

  case 625: /* limit_burst_pkts: "burst" "number" "packets"  */
#line 3741 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[-1].val); }
#line 12910 "src/parser_bison.c"
    break;

  case 626: /* limit_rate_pkts: "number" "/" time_unit  */
#line 3745 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12919 "src/parser_bison.c"
    break;

  case 627: /* limit_burst_bytes: %empty  */
#line 3751 "src/parser_bison.y"
                                                                { (yyval.val) = 0; }
#line 12925 "src/parser_bison.c"
    break;

  case 628: /* limit_burst_bytes: "burst" "number" bytes_unit  */
#line 3752 "src/parser_bison.y"
                                                                { (yyval.val) = (yyvsp[-1].val) * (yyvsp[0].val); }
#line 12931 "src/parser_bison.c"
    break;

  case 629: /* limit_rate_bytes: "number" bytes_unit "/" time_unit  */
#line 3756 "src/parser_bison.y"
                        {
				(yyval.limit_rate).rate = (yyvsp[-3].val) * (yyvsp[-2].val);
				(yyval.limit_rate).unit = (yyvsp[0].val);
			}
#line 12940 "src/parser_bison.c"
    break;

  case 630: /* time_unit: "second"  */
#line 3762 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL; }
#line 12946 "src/parser_bison.c"
    break;

  case 631: /* time_unit: "minute"  */
#line 3763 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60; }
#line 12952 "src/parser_bison.c"
    break;

  case 632: /* time_unit: "hour"  */
#line 3764 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60; }
#line 12958 "src/parser_bison.c"
    break;

  case 633: /* time_unit: "day"  */
#line 3765 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24; }
#line 12964 "src/parser_bison.c"
    break;

  case 634: /* time_unit: "week"  */
#line 3766 "src/parser_bison.y"
                                                { (yyval.val) = 1ULL * 60 * 60 * 24 * 7; }
#line 12970 "src/parser_bison.c"
    break;

  case 636: /* reject_stmt_alloc: "reject"  */
#line 3773 "src/parser_bison.y"
                        {
				(yyval.stmt) = reject_stmt_alloc(&(yyloc));
			}
#line 12978 "src/parser_bison.c"
    break;

  case 637: /* reject_with_expr: "string"  */
#line 3779 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state), (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 12988 "src/parser_bison.c"
    break;

  case 638: /* reject_with_expr: integer_expr  */
#line 3784 "src/parser_bison.y"
                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 12994 "src/parser_bison.c"
    break;

  case 639: /* reject_opts: %empty  */
#line 3788 "src/parser_bison.y"
                        {
				(yyvsp[0].stmt)->reject.type = -1;
				(yyvsp[0].stmt)->reject.icmp_code = -1;
			}
#line 13003 "src/parser_bison.c"
    break;

  case 640: /* reject_opts: "with" "icmp" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3793 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 13014 "src/parser_bison.c"
    break;

  case 641: /* reject_opts: "with" "icmp" reject_with_expr  */
#line 3800 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV4;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmp_code_type);
			}
#line 13025 "src/parser_bison.c"
    break;

  case 642: /* reject_opts: "with" "icmpv6" "type" reject_with_expr close_scope_type close_scope_icmp  */
#line 3807 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-6].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-6].stmt)->reject.expr = (yyvsp[-2].expr);
				datatype_set((yyvsp[-6].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 13036 "src/parser_bison.c"
    break;

  case 643: /* reject_opts: "with" "icmpv6" reject_with_expr  */
#line 3814 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.family = NFPROTO_IPV6;
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMP_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpv6_code_type);
			}
#line 13047 "src/parser_bison.c"
    break;

  case 644: /* reject_opts: "with" "icmpx" "type" reject_with_expr close_scope_type  */
#line 3821 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-5].stmt)->reject.expr = (yyvsp[-1].expr);
				datatype_set((yyvsp[-5].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 13057 "src/parser_bison.c"
    break;

  case 645: /* reject_opts: "with" "icmpx" reject_with_expr  */
#line 3827 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->reject.type = NFT_REJECT_ICMPX_UNREACH;
				(yyvsp[-3].stmt)->reject.expr = (yyvsp[0].expr);
				datatype_set((yyvsp[-3].stmt)->reject.expr, &reject_icmpx_code_type);
			}
#line 13067 "src/parser_bison.c"
    break;

  case 646: /* reject_opts: "with" "tcp" close_scope_tcp "reset" close_scope_reset  */
#line 3833 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->reject.type = NFT_REJECT_TCP_RST;
			}
#line 13075 "src/parser_bison.c"
    break;

  case 648: /* nat_stmt_alloc: "snat"  */
#line 3841 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_SNAT); }
#line 13081 "src/parser_bison.c"
    break;

  case 649: /* nat_stmt_alloc: "dnat"  */
#line 3842 "src/parser_bison.y"
                                        { (yyval.stmt) = nat_stmt_alloc(&(yyloc), __NFT_NAT_DNAT); }
#line 13087 "src/parser_bison.c"
    break;

  case 650: /* tproxy_stmt: "tproxy" "to" stmt_expr  */
#line 3846 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 13097 "src/parser_bison.c"
    break;

  case 651: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr  */
#line 3852 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-2].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[0].expr);
			}
#line 13107 "src/parser_bison.c"
    break;

  case 652: /* tproxy_stmt: "tproxy" "to" "colon" stmt_expr  */
#line 3858 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 13117 "src/parser_bison.c"
    break;

  case 653: /* tproxy_stmt: "tproxy" "to" stmt_expr "colon" stmt_expr  */
#line 3864 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = NFPROTO_UNSPEC;
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 13128 "src/parser_bison.c"
    break;

  case 654: /* tproxy_stmt: "tproxy" nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 3871 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-4].val);
				(yyval.stmt)->tproxy.addr = (yyvsp[-2].expr);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 13139 "src/parser_bison.c"
    break;

  case 655: /* tproxy_stmt: "tproxy" nf_key_proto "to" "colon" stmt_expr  */
#line 3878 "src/parser_bison.y"
                        {
				(yyval.stmt) = tproxy_stmt_alloc(&(yyloc));
				(yyval.stmt)->tproxy.family = (yyvsp[-3].val);
				(yyval.stmt)->tproxy.port = (yyvsp[0].expr);
			}
#line 13149 "src/parser_bison.c"
    break;

  case 658: /* synproxy_stmt_alloc: "synproxy"  */
#line 3890 "src/parser_bison.y"
                        {
				(yyval.stmt) = synproxy_stmt_alloc(&(yyloc));
			}
#line 13157 "src/parser_bison.c"
    break;

  case 659: /* synproxy_args: synproxy_arg  */
#line 3896 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 13165 "src/parser_bison.c"
    break;

  case 661: /* synproxy_wscale: "wscale" "number"  */
#line 3903 "src/parser_bison.y"
                        {
				if ((yyvsp[0].val) > 14) {
					erec_queue(error(&(yylsp[0]), "wscale must be in range 0-14"), state->msgs);
					YYERROR;
				}

				(yyval.val) = (yyvsp[0].val);
			}
#line 13178 "src/parser_bison.c"
    break;

  case 662: /* synproxy_arg: "mss" "number"  */
#line 3914 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->synproxy.mss = (yyvsp[0].val);
				(yyvsp[-2].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_MSS;
			}
#line 13187 "src/parser_bison.c"
    break;

  case 663: /* synproxy_arg: synproxy_wscale  */
#line 3919 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.wscale = (yyvsp[0].val);
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_WSCALE;
			}
#line 13196 "src/parser_bison.c"
    break;

  case 664: /* synproxy_arg: "timestamp"  */
#line 3924 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 13204 "src/parser_bison.c"
    break;

  case 665: /* synproxy_arg: "sack-permitted"  */
#line 3928 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->synproxy.flags |= NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 13212 "src/parser_bison.c"
    break;

  case 666: /* synproxy_config: "mss" "number" synproxy_wscale synproxy_ts synproxy_sack  */
#line 3934 "src/parser_bison.y"
                        {
				struct synproxy *synproxy;
				uint32_t flags = 0;

				synproxy = &(yyvsp[-5].obj)->synproxy;
				synproxy->mss = (yyvsp[-3].val);
				flags |= NF_SYNPROXY_OPT_MSS;
				synproxy->wscale = (yyvsp[-1].val);
				flags |= NF_SYNPROXY_OPT_WSCALE;
				if ((yyvsp[-1].val))
					flags |= (yyvsp[-1].val);
				if ((yyvsp[0].val))
					flags |= (yyvsp[0].val);
				synproxy->flags = flags;
			}
#line 13232 "src/parser_bison.c"
    break;

  case 667: /* synproxy_config: "mss" "number" stmt_separator synproxy_wscale stmt_separator synproxy_ts synproxy_sack  */
#line 3950 "src/parser_bison.y"
                        {
				struct synproxy *synproxy;
				uint32_t flags = 0;

				synproxy = &(yyvsp[-7].obj)->synproxy;
				synproxy->mss = (yyvsp[-5].val);
				flags |= NF_SYNPROXY_OPT_MSS;
				synproxy->wscale = (yyvsp[-3].val);
				flags |= NF_SYNPROXY_OPT_WSCALE;
				if ((yyvsp[-1].val))
					flags |= (yyvsp[-1].val);
				if ((yyvsp[0].val))
					flags |= (yyvsp[0].val);
				synproxy->flags = flags;
			}
#line 13252 "src/parser_bison.c"
    break;

  case 668: /* synproxy_obj: %empty  */
#line 3968 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SYNPROXY;
			}
#line 13261 "src/parser_bison.c"
    break;

  case 669: /* synproxy_ts: %empty  */
#line 3974 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 13267 "src/parser_bison.c"
    break;

  case 670: /* synproxy_ts: "timestamp"  */
#line 3976 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_TIMESTAMP;
			}
#line 13275 "src/parser_bison.c"
    break;

  case 671: /* synproxy_sack: %empty  */
#line 3981 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 13281 "src/parser_bison.c"
    break;

  case 672: /* synproxy_sack: "sack-permitted"  */
#line 3983 "src/parser_bison.y"
                        {
				(yyval.val) = NF_SYNPROXY_OPT_SACK_PERM;
			}
#line 13289 "src/parser_bison.c"
    break;

  case 673: /* primary_stmt_expr: symbol_expr  */
#line 3988 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13295 "src/parser_bison.c"
    break;

  case 674: /* primary_stmt_expr: integer_expr  */
#line 3989 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13301 "src/parser_bison.c"
    break;

  case 675: /* primary_stmt_expr: boolean_expr  */
#line 3990 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13307 "src/parser_bison.c"
    break;

  case 676: /* primary_stmt_expr: meta_expr  */
#line 3991 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13313 "src/parser_bison.c"
    break;

  case 677: /* primary_stmt_expr: rt_expr  */
#line 3992 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13319 "src/parser_bison.c"
    break;

  case 678: /* primary_stmt_expr: tunnel_expr  */
#line 3993 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13325 "src/parser_bison.c"
    break;

  case 679: /* primary_stmt_expr: ct_expr  */
#line 3994 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13331 "src/parser_bison.c"
    break;

  case 680: /* primary_stmt_expr: numgen_expr  */
#line 3995 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13337 "src/parser_bison.c"
    break;

  case 681: /* primary_stmt_expr: hash_expr  */
#line 3996 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13343 "src/parser_bison.c"
    break;

  case 682: /* primary_stmt_expr: payload_expr  */
#line 3997 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13349 "src/parser_bison.c"
    break;

  case 683: /* primary_stmt_expr: keyword_expr  */
#line 3998 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13355 "src/parser_bison.c"
    break;

  case 684: /* primary_stmt_expr: socket_expr  */
#line 3999 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13361 "src/parser_bison.c"
    break;

  case 685: /* primary_stmt_expr: fib_expr  */
#line 4000 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13367 "src/parser_bison.c"
    break;

  case 686: /* primary_stmt_expr: osf_expr  */
#line 4001 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 13373 "src/parser_bison.c"
    break;

  case 687: /* primary_stmt_expr: '(' basic_stmt_expr ')'  */
#line 4002 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 13379 "src/parser_bison.c"
    break;

  case 689: /* shift_stmt_expr: shift_stmt_expr "<<" primary_stmt_expr  */
#line 4007 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13387 "src/parser_bison.c"
    break;

  case 690: /* shift_stmt_expr: shift_stmt_expr ">>" primary_stmt_expr  */
#line 4011 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13395 "src/parser_bison.c"
    break;

  case 692: /* and_stmt_expr: and_stmt_expr "&" shift_stmt_expr  */
#line 4018 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13403 "src/parser_bison.c"
    break;

  case 694: /* exclusive_or_stmt_expr: exclusive_or_stmt_expr "^" and_stmt_expr  */
#line 4025 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13411 "src/parser_bison.c"
    break;

  case 696: /* inclusive_or_stmt_expr: inclusive_or_stmt_expr '|' exclusive_or_stmt_expr  */
#line 4032 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13419 "src/parser_bison.c"
    break;

  case 699: /* concat_stmt_expr: concat_stmt_expr "." primary_stmt_expr  */
#line 4042 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 13432 "src/parser_bison.c"
    break;

  case 702: /* map_stmt_expr: concat_stmt_expr "map" map_stmt_expr_set  */
#line 4057 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13440 "src/parser_bison.c"
    break;

  case 703: /* map_stmt_expr: concat_stmt_expr  */
#line 4060 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 13446 "src/parser_bison.c"
    break;

  case 704: /* prefix_stmt_expr: basic_stmt_expr "/" "number"  */
#line 4064 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 13454 "src/parser_bison.c"
    break;

  case 705: /* range_stmt_expr: basic_stmt_expr "-" basic_stmt_expr  */
#line 4070 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13462 "src/parser_bison.c"
    break;

  case 711: /* nat_stmt_args: stmt_expr  */
#line 4085 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13470 "src/parser_bison.c"
    break;

  case 712: /* nat_stmt_args: "to" stmt_expr  */
#line 4089 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13478 "src/parser_bison.c"
    break;

  case 713: /* nat_stmt_args: nf_key_proto "to" stmt_expr  */
#line 4093 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.family = (yyvsp[-2].val);
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13487 "src/parser_bison.c"
    break;

  case 714: /* nat_stmt_args: stmt_expr "colon" stmt_expr  */
#line 4098 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13496 "src/parser_bison.c"
    break;

  case 715: /* nat_stmt_args: "to" stmt_expr "colon" stmt_expr  */
#line 4103 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13505 "src/parser_bison.c"
    break;

  case 716: /* nat_stmt_args: nf_key_proto "to" stmt_expr "colon" stmt_expr  */
#line 4108 "src/parser_bison.y"
                        {
				(yyvsp[-5].stmt)->nat.family = (yyvsp[-4].val);
				(yyvsp[-5].stmt)->nat.addr = (yyvsp[-2].expr);
				(yyvsp[-5].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13515 "src/parser_bison.c"
    break;

  case 717: /* nat_stmt_args: "colon" stmt_expr  */
#line 4114 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13523 "src/parser_bison.c"
    break;

  case 718: /* nat_stmt_args: "to" "colon" stmt_expr  */
#line 4118 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13531 "src/parser_bison.c"
    break;

  case 719: /* nat_stmt_args: nat_stmt_args nf_nat_flags  */
#line 4122 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13539 "src/parser_bison.c"
    break;

  case 720: /* nat_stmt_args: nf_key_proto "addr" "." "port" "to" stmt_expr  */
#line 4126 "src/parser_bison.y"
                        {
				(yyvsp[-6].stmt)->nat.family = (yyvsp[-5].val);
				(yyvsp[-6].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-6].stmt)->nat.type_flags = STMT_NAT_F_CONCAT;
			}
#line 13549 "src/parser_bison.c"
    break;

  case 721: /* nat_stmt_args: nf_key_proto "interval" "to" stmt_expr  */
#line 4132 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13558 "src/parser_bison.c"
    break;

  case 722: /* nat_stmt_args: "interval" "to" stmt_expr  */
#line 4137 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
			}
#line 13566 "src/parser_bison.c"
    break;

  case 723: /* nat_stmt_args: nf_key_proto "prefix" "to" stmt_expr  */
#line 4141 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.family = (yyvsp[-3].val);
				(yyvsp[-4].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-4].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-4].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 13578 "src/parser_bison.c"
    break;

  case 724: /* nat_stmt_args: "prefix" "to" stmt_expr  */
#line 4149 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.addr = (yyvsp[0].expr);
				(yyvsp[-3].stmt)->nat.type_flags =
						STMT_NAT_F_PREFIX;
				(yyvsp[-3].stmt)->nat.flags |= NF_NAT_RANGE_NETMAP;
			}
#line 13589 "src/parser_bison.c"
    break;

  case 727: /* masq_stmt_alloc: "masquerade"  */
#line 4161 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_MASQ); }
#line 13595 "src/parser_bison.c"
    break;

  case 728: /* masq_stmt_args: "to" "colon" stmt_expr  */
#line 4165 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13603 "src/parser_bison.c"
    break;

  case 729: /* masq_stmt_args: "to" "colon" stmt_expr nf_nat_flags  */
#line 4169 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13612 "src/parser_bison.c"
    break;

  case 730: /* masq_stmt_args: nf_nat_flags  */
#line 4174 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13620 "src/parser_bison.c"
    break;

  case 733: /* redir_stmt_alloc: "redirect"  */
#line 4183 "src/parser_bison.y"
                                                { (yyval.stmt) = nat_stmt_alloc(&(yyloc), NFT_NAT_REDIR); }
#line 13626 "src/parser_bison.c"
    break;

  case 734: /* redir_stmt_arg: "to" stmt_expr  */
#line 4187 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13634 "src/parser_bison.c"
    break;

  case 735: /* redir_stmt_arg: "to" "colon" stmt_expr  */
#line 4191 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[0].expr);
			}
#line 13642 "src/parser_bison.c"
    break;

  case 736: /* redir_stmt_arg: nf_nat_flags  */
#line 4195 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13650 "src/parser_bison.c"
    break;

  case 737: /* redir_stmt_arg: "to" stmt_expr nf_nat_flags  */
#line 4199 "src/parser_bison.y"
                        {
				(yyvsp[-3].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-3].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13659 "src/parser_bison.c"
    break;

  case 738: /* redir_stmt_arg: "to" "colon" stmt_expr nf_nat_flags  */
#line 4204 "src/parser_bison.y"
                        {
				(yyvsp[-4].stmt)->nat.proto = (yyvsp[-1].expr);
				(yyvsp[-4].stmt)->nat.flags = (yyvsp[0].val);
			}
#line 13668 "src/parser_bison.c"
    break;

  case 739: /* dup_stmt: "dup" "to" stmt_expr  */
#line 4211 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[0].expr);
			}
#line 13677 "src/parser_bison.c"
    break;

  case 740: /* dup_stmt: "dup" "to" stmt_expr "device" stmt_expr  */
#line 4216 "src/parser_bison.y"
                        {
				(yyval.stmt) = dup_stmt_alloc(&(yyloc));
				(yyval.stmt)->dup.to = (yyvsp[-2].expr);
				(yyval.stmt)->dup.dev = (yyvsp[0].expr);
			}
#line 13687 "src/parser_bison.c"
    break;

  case 741: /* fwd_stmt: "fwd" "to" stmt_expr  */
#line 4224 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13696 "src/parser_bison.c"
    break;

  case 742: /* fwd_stmt: "fwd" nf_key_proto "to" stmt_expr "device" stmt_expr  */
#line 4229 "src/parser_bison.y"
                        {
				(yyval.stmt) = fwd_stmt_alloc(&(yyloc));
				(yyval.stmt)->fwd.family = (yyvsp[-4].val);
				(yyval.stmt)->fwd.addr = (yyvsp[-2].expr);
				(yyval.stmt)->fwd.dev = (yyvsp[0].expr);
			}
#line 13707 "src/parser_bison.c"
    break;

  case 744: /* nf_nat_flags: nf_nat_flags "comma" nf_nat_flag  */
#line 4239 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13715 "src/parser_bison.c"
    break;

  case 745: /* nf_nat_flag: "random"  */
#line 4244 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM; }
#line 13721 "src/parser_bison.c"
    break;

  case 746: /* nf_nat_flag: "fully-random"  */
#line 4245 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PROTO_RANDOM_FULLY; }
#line 13727 "src/parser_bison.c"
    break;

  case 747: /* nf_nat_flag: "persistent"  */
#line 4246 "src/parser_bison.y"
                                                { (yyval.val) = NF_NAT_RANGE_PERSISTENT; }
#line 13733 "src/parser_bison.c"
    break;

  case 749: /* queue_stmt: "queue" "to" queue_stmt_expr close_scope_queue  */
#line 4251 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), 0);
			}
#line 13741 "src/parser_bison.c"
    break;

  case 750: /* queue_stmt: "queue" "flags" queue_stmt_flags "to" queue_stmt_expr close_scope_queue  */
#line 4255 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13749 "src/parser_bison.c"
    break;

  case 751: /* queue_stmt: "queue" "flags" queue_stmt_flags "num" queue_stmt_expr_simple close_scope_queue  */
#line 4259 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), (yyvsp[-1].expr), (yyvsp[-3].val));
			}
#line 13757 "src/parser_bison.c"
    break;

  case 754: /* queue_stmt_alloc: "queue"  */
#line 4269 "src/parser_bison.y"
                        {
				(yyval.stmt) = queue_stmt_alloc(&(yyloc), NULL, 0);
			}
#line 13765 "src/parser_bison.c"
    break;

  case 755: /* queue_stmt_args: queue_stmt_arg  */
#line 4275 "src/parser_bison.y"
                        {
				(yyval.stmt)	= (yyvsp[-1].stmt);
			}
#line 13773 "src/parser_bison.c"
    break;

  case 757: /* queue_stmt_arg: "num" queue_stmt_expr_simple  */
#line 4282 "src/parser_bison.y"
                        {
				(yyvsp[-2].stmt)->queue.queue = (yyvsp[0].expr);
				(yyvsp[-2].stmt)->queue.queue->location = (yyloc);
			}
#line 13782 "src/parser_bison.c"
    break;

  case 758: /* queue_stmt_arg: queue_stmt_flags  */
#line 4287 "src/parser_bison.y"
                        {
				(yyvsp[-1].stmt)->queue.flags |= (yyvsp[0].val);
			}
#line 13790 "src/parser_bison.c"
    break;

  case 763: /* queue_stmt_expr_simple: queue_expr "-" queue_expr  */
#line 4299 "src/parser_bison.y"
                        {
				(yyval.expr) = range_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 13798 "src/parser_bison.c"
    break;

  case 769: /* queue_stmt_flags: queue_stmt_flags "comma" queue_stmt_flag  */
#line 4312 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 13806 "src/parser_bison.c"
    break;

  case 770: /* queue_stmt_flag: "bypass"  */
#line 4317 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_BYPASS; }
#line 13812 "src/parser_bison.c"
    break;

  case 771: /* queue_stmt_flag: "fanout"  */
#line 4318 "src/parser_bison.y"
                                        { (yyval.val) = NFT_QUEUE_FLAG_CPU_FANOUT; }
#line 13818 "src/parser_bison.c"
    break;

  case 774: /* set_elem_expr_stmt_alloc: concat_expr  */
#line 4326 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 13826 "src/parser_bison.c"
    break;

  case 775: /* set_stmt: "set" set_stmt_op set_elem_expr_stmt set_ref_expr  */
#line 4332 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-2].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[0].expr);
			}
#line 13837 "src/parser_bison.c"
    break;

  case 776: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt '}'  */
#line 4339 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-4].val);
				(yyval.stmt)->set.key = (yyvsp[-1].expr);
				(yyval.stmt)->set.set = (yyvsp[-3].expr);
			}
#line 13848 "src/parser_bison.c"
    break;

  case 777: /* set_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list '}'  */
#line 4346 "src/parser_bison.y"
                        {
				(yyval.stmt) = set_stmt_alloc(&(yyloc));
				(yyval.stmt)->set.op  = (yyvsp[-5].val);
				(yyval.stmt)->set.key = (yyvsp[-2].expr);
				(yyval.stmt)->set.set = (yyvsp[-4].expr);
				list_splice_tail((yyvsp[-1].list), &(yyval.stmt)->set.stmt_list);
				free((yyvsp[-1].list));
			}
#line 13861 "src/parser_bison.c"
    break;

  case 778: /* set_stmt_op: "add"  */
#line 4356 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_ADD; }
#line 13867 "src/parser_bison.c"
    break;

  case 779: /* set_stmt_op: "update"  */
#line 4357 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_UPDATE; }
#line 13873 "src/parser_bison.c"
    break;

  case 780: /* set_stmt_op: "delete"  */
#line 4358 "src/parser_bison.y"
                                        { (yyval.val) = NFT_DYNSET_OP_DELETE; }
#line 13879 "src/parser_bison.c"
    break;

  case 781: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt "colon" set_elem_expr_stmt '}'  */
#line 4362 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-6].val);
				(yyval.stmt)->map.key = (yyvsp[-3].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-5].expr);
			}
#line 13891 "src/parser_bison.c"
    break;

  case 782: /* map_stmt: set_stmt_op set_ref_expr '{' set_elem_expr_stmt stateful_stmt_list "colon" set_elem_expr_stmt '}'  */
#line 4370 "src/parser_bison.y"
                        {
				(yyval.stmt) = map_stmt_alloc(&(yyloc));
				(yyval.stmt)->map.op  = (yyvsp[-7].val);
				(yyval.stmt)->map.key = (yyvsp[-4].expr);
				(yyval.stmt)->map.data = (yyvsp[-1].expr);
				(yyval.stmt)->map.set = (yyvsp[-6].expr);
				list_splice_tail((yyvsp[-3].list), &(yyval.stmt)->map.stmt_list);
				free((yyvsp[-3].list));
			}
#line 13905 "src/parser_bison.c"
    break;

  case 783: /* meter_stmt: "meter" identifier '{' meter_key_expr stmt '}'  */
#line 4382 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-4].string);
				(yyval.stmt)->meter.size = 0;
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13918 "src/parser_bison.c"
    break;

  case 784: /* meter_stmt: "meter" identifier "size" "number" '{' meter_key_expr stmt '}'  */
#line 4391 "src/parser_bison.y"
                        {
				(yyval.stmt) = meter_stmt_alloc(&(yyloc));
				(yyval.stmt)->meter.name = (yyvsp[-6].string);
				(yyval.stmt)->meter.size = (yyvsp[-4].val);
				(yyval.stmt)->meter.key  = (yyvsp[-2].expr);
				(yyval.stmt)->meter.stmt = (yyvsp[-1].stmt);
				(yyval.stmt)->location  = (yyloc);
			}
#line 13931 "src/parser_bison.c"
    break;

  case 785: /* match_stmt: relational_expr  */
#line 4402 "src/parser_bison.y"
                        {
				(yyval.stmt) = expr_stmt_alloc(&(yyloc), (yyvsp[0].expr));
			}
#line 13939 "src/parser_bison.c"
    break;

  case 786: /* variable_expr: '$' identifier  */
#line 4408 "src/parser_bison.y"
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
#line 13967 "src/parser_bison.c"
    break;

  case 788: /* symbol_expr: string  */
#line 4435 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 13978 "src/parser_bison.c"
    break;

  case 791: /* set_ref_symbol_expr: "@" identifier close_scope_at  */
#line 4448 "src/parser_bison.y"
                        {
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_SET,
						       current_scope(state),
						       (yyvsp[-1].string));
				free_const((yyvsp[-1].string));
			}
#line 13989 "src/parser_bison.c"
    break;

  case 792: /* integer_expr: "number"  */
#line 4457 "src/parser_bison.y"
                        {
				char str[64];

				snprintf(str, sizeof(str), "%" PRIu64, (yyvsp[0].val));
				(yyval.expr) = symbol_expr_alloc(&(yyloc), SYMBOL_VALUE,
						       current_scope(state),
						       str);
			}
#line 14002 "src/parser_bison.c"
    break;

  case 793: /* selector_expr: payload_expr  */
#line 4467 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14008 "src/parser_bison.c"
    break;

  case 794: /* selector_expr: exthdr_expr  */
#line 4468 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14014 "src/parser_bison.c"
    break;

  case 795: /* selector_expr: exthdr_exists_expr  */
#line 4469 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14020 "src/parser_bison.c"
    break;

  case 796: /* selector_expr: meta_expr  */
#line 4470 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14026 "src/parser_bison.c"
    break;

  case 797: /* selector_expr: tunnel_expr  */
#line 4471 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14032 "src/parser_bison.c"
    break;

  case 798: /* selector_expr: socket_expr  */
#line 4472 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14038 "src/parser_bison.c"
    break;

  case 799: /* selector_expr: rt_expr  */
#line 4473 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14044 "src/parser_bison.c"
    break;

  case 800: /* selector_expr: ct_expr  */
#line 4474 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14050 "src/parser_bison.c"
    break;

  case 801: /* selector_expr: numgen_expr  */
#line 4475 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14056 "src/parser_bison.c"
    break;

  case 802: /* selector_expr: hash_expr  */
#line 4476 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14062 "src/parser_bison.c"
    break;

  case 803: /* selector_expr: fib_expr  */
#line 4477 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14068 "src/parser_bison.c"
    break;

  case 804: /* selector_expr: osf_expr  */
#line 4478 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14074 "src/parser_bison.c"
    break;

  case 805: /* selector_expr: xfrm_expr  */
#line 4479 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14080 "src/parser_bison.c"
    break;

  case 806: /* primary_expr: symbol_expr  */
#line 4482 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14086 "src/parser_bison.c"
    break;

  case 807: /* primary_expr: integer_expr  */
#line 4483 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14092 "src/parser_bison.c"
    break;

  case 808: /* primary_expr: selector_expr  */
#line 4484 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 14098 "src/parser_bison.c"
    break;

  case 809: /* primary_expr: '(' basic_expr ')'  */
#line 4485 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 14104 "src/parser_bison.c"
    break;

  case 810: /* fib_expr: "fib" fib_tuple fib_result close_scope_fib  */
#line 4489 "src/parser_bison.y"
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
#line 14136 "src/parser_bison.c"
    break;

  case 811: /* fib_result: "oif"  */
#line 4518 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIF; }
#line 14142 "src/parser_bison.c"
    break;

  case 812: /* fib_result: "oifname"  */
#line 4519 "src/parser_bison.y"
                                        { (yyval.val) =NFT_FIB_RESULT_OIFNAME; }
#line 14148 "src/parser_bison.c"
    break;

  case 813: /* fib_result: "type" close_scope_type  */
#line 4520 "src/parser_bison.y"
                                                                { (yyval.val) =NFT_FIB_RESULT_ADDRTYPE; }
#line 14154 "src/parser_bison.c"
    break;

  case 814: /* fib_result: "check"  */
#line 4521 "src/parser_bison.y"
                                        { (yyval.val) = __NFT_FIB_RESULT_MAX; }
#line 14160 "src/parser_bison.c"
    break;

  case 815: /* fib_flag: "saddr"  */
#line 4524 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_SADDR; }
#line 14166 "src/parser_bison.c"
    break;

  case 816: /* fib_flag: "daddr"  */
#line 4525 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_DADDR; }
#line 14172 "src/parser_bison.c"
    break;

  case 817: /* fib_flag: "mark"  */
#line 4526 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_MARK; }
#line 14178 "src/parser_bison.c"
    break;

  case 818: /* fib_flag: "iif"  */
#line 4527 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_IIF; }
#line 14184 "src/parser_bison.c"
    break;

  case 819: /* fib_flag: "oif"  */
#line 4528 "src/parser_bison.y"
                                        { (yyval.val) = NFTA_FIB_F_OIF; }
#line 14190 "src/parser_bison.c"
    break;

  case 820: /* fib_tuple: fib_flag "." fib_tuple  */
#line 4532 "src/parser_bison.y"
                        {
				(yyval.val) = (yyvsp[-2].val) | (yyvsp[0].val);
			}
#line 14198 "src/parser_bison.c"
    break;

  case 822: /* osf_expr: "osf" osf_ttl "version" close_scope_osf  */
#line 4539 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), NFT_OSF_F_VERSION);
			}
#line 14206 "src/parser_bison.c"
    break;

  case 823: /* osf_expr: "osf" osf_ttl "name" close_scope_osf  */
#line 4543 "src/parser_bison.y"
                        {
				(yyval.expr) = osf_expr_alloc(&(yyloc), (yyvsp[-2].val), 0);
			}
#line 14214 "src/parser_bison.c"
    break;

  case 824: /* osf_ttl: %empty  */
#line 4548 "src/parser_bison.y"
                                                { (yyval.val) = NF_OSF_TTL_TRUE; }
#line 14220 "src/parser_bison.c"
    break;

  case 825: /* osf_ttl: "ttl" "loose"  */
#line 4549 "src/parser_bison.y"
                                                { (yyval.val) = NF_OSF_TTL_LESS; }
#line 14226 "src/parser_bison.c"
    break;

  case 826: /* osf_ttl: "ttl" "skip"  */
#line 4550 "src/parser_bison.y"
                                                { (yyval.val) = NF_OSF_TTL_NOCHECK; }
#line 14232 "src/parser_bison.c"
    break;

  case 828: /* shift_expr: shift_expr "<<" primary_rhs_expr  */
#line 4555 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14240 "src/parser_bison.c"
    break;

  case 829: /* shift_expr: shift_expr ">>" primary_rhs_expr  */
#line 4559 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14248 "src/parser_bison.c"
    break;

  case 831: /* and_expr: and_expr "&" shift_rhs_expr  */
#line 4566 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14256 "src/parser_bison.c"
    break;

  case 833: /* exclusive_or_expr: exclusive_or_expr "^" and_rhs_expr  */
#line 4573 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14264 "src/parser_bison.c"
    break;

  case 835: /* inclusive_or_expr: inclusive_or_expr '|' exclusive_or_rhs_expr  */
#line 4580 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14272 "src/parser_bison.c"
    break;

  case 838: /* concat_expr: concat_expr "." basic_expr  */
#line 4590 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 14285 "src/parser_bison.c"
    break;

  case 839: /* prefix_rhs_expr: basic_rhs_expr "/" "number"  */
#line 4601 "src/parser_bison.y"
                        {
				(yyval.expr) = prefix_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].val));
			}
#line 14293 "src/parser_bison.c"
    break;

  case 840: /* range_rhs_expr: basic_rhs_expr "-" basic_rhs_expr  */
#line 4607 "src/parser_bison.y"
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
#line 14308 "src/parser_bison.c"
    break;

  case 843: /* map_expr: concat_expr "map" rhs_expr  */
#line 4624 "src/parser_bison.y"
                        {
				(yyval.expr) = map_expr_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 14316 "src/parser_bison.c"
    break;

  case 847: /* set_expr: '{' set_list_expr '}'  */
#line 4635 "src/parser_bison.y"
                        {
				(yyvsp[-1].expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14325 "src/parser_bison.c"
    break;

  case 848: /* set_list_expr: set_list_member_expr  */
#line 4642 "src/parser_bison.y"
                        {
				(yyval.expr) = set_expr_alloc(&(yyloc), NULL);
				set_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 14334 "src/parser_bison.c"
    break;

  case 849: /* set_list_expr: set_list_expr "comma" set_list_member_expr  */
#line 4647 "src/parser_bison.y"
                        {
				set_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 14343 "src/parser_bison.c"
    break;

  case 851: /* set_list_member_expr: opt_newline set_expr opt_newline  */
#line 4655 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yyloc), (yyvsp[-1].expr));
			}
#line 14351 "src/parser_bison.c"
    break;

  case 852: /* set_list_member_expr: opt_newline set_elem_expr opt_newline  */
#line 4659 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14359 "src/parser_bison.c"
    break;

  case 853: /* set_list_member_expr: opt_newline set_elem_expr "colon" set_rhs_expr opt_newline  */
#line 4663 "src/parser_bison.y"
                        {
				struct expr *expr = (yyvsp[-3].expr);

				expr->key = mapping_expr_alloc(&(yylsp[-3]), (yyvsp[-3].expr)->key, (yyvsp[-1].expr));
				(yyval.expr) = expr;
			}
#line 14370 "src/parser_bison.c"
    break;

  case 855: /* meter_key_expr: meter_key_expr_alloc set_elem_options  */
#line 4673 "src/parser_bison.y"
                        {
				(yyval.expr)->location = (yyloc);
				(yyval.expr) = (yyvsp[-1].expr);
			}
#line 14379 "src/parser_bison.c"
    break;

  case 856: /* meter_key_expr_alloc: concat_expr  */
#line 4680 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 14387 "src/parser_bison.c"
    break;

  case 859: /* set_elem_expr: set_elem_expr_alloc set_elem_expr_options set_elem_stmt_list  */
#line 4688 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 14397 "src/parser_bison.c"
    break;

  case 860: /* set_elem_key_expr: set_lhs_expr  */
#line 4695 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 14403 "src/parser_bison.c"
    break;

  case 861: /* set_elem_key_expr: "*"  */
#line 4696 "src/parser_bison.y"
                                                        { (yyval.expr) = set_elem_catchall_expr_alloc(&(yylsp[0])); }
#line 14409 "src/parser_bison.c"
    break;

  case 862: /* set_elem_expr_alloc: set_elem_key_expr set_elem_stmt_list  */
#line 4700 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[-1]), (yyvsp[-1].expr));
				list_splice_tail((yyvsp[0].list), &(yyval.expr)->stmt_list);
				free((yyvsp[0].list));
			}
#line 14419 "src/parser_bison.c"
    break;

  case 863: /* set_elem_expr_alloc: set_elem_key_expr  */
#line 4706 "src/parser_bison.y"
                        {
				(yyval.expr) = set_elem_expr_alloc(&(yylsp[0]), (yyvsp[0].expr));
			}
#line 14427 "src/parser_bison.c"
    break;

  case 864: /* set_elem_options: set_elem_option  */
#line 4712 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 14435 "src/parser_bison.c"
    break;

  case 866: /* set_elem_time_spec: "string"  */
#line 4719 "src/parser_bison.y"
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
#line 14458 "src/parser_bison.c"
    break;

  case 867: /* set_elem_option: "timeout" time_spec  */
#line 4740 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 14466 "src/parser_bison.c"
    break;

  case 868: /* set_elem_option: "expires" time_spec  */
#line 4744 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 14474 "src/parser_bison.c"
    break;

  case 869: /* set_elem_option: comment_spec  */
#line 4748 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 14486 "src/parser_bison.c"
    break;

  case 870: /* set_elem_expr_options: set_elem_expr_option  */
#line 4758 "src/parser_bison.y"
                        {
				(yyval.expr)	= (yyvsp[-1].expr);
			}
#line 14494 "src/parser_bison.c"
    break;

  case 872: /* set_elem_stmt_list: set_elem_stmt  */
#line 4765 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].stmt)->list, (yyval.list));
			}
#line 14504 "src/parser_bison.c"
    break;

  case 873: /* set_elem_stmt_list: set_elem_stmt_list set_elem_stmt  */
#line 4771 "src/parser_bison.y"
                        {
				(yyval.list) = (yyvsp[-1].list);
				list_add_tail(&(yyvsp[0].stmt)->list, (yyvsp[-1].list));
			}
#line 14513 "src/parser_bison.c"
    break;

  case 879: /* set_elem_expr_option: "timeout" set_elem_time_spec  */
#line 4785 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->timeout = (yyvsp[0].val);
			}
#line 14521 "src/parser_bison.c"
    break;

  case 880: /* set_elem_expr_option: "expires" time_spec  */
#line 4789 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->expiration = (yyvsp[0].val);
			}
#line 14529 "src/parser_bison.c"
    break;

  case 881: /* set_elem_expr_option: comment_spec  */
#line 4793 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].expr)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].expr)->comment = (yyvsp[0].string);
			}
#line 14541 "src/parser_bison.c"
    break;

  case 887: /* initializer_expr: '{' '}'  */
#line 4811 "src/parser_bison.y"
                                                { (yyval.expr) = set_expr_alloc(&(yyloc), NULL); }
#line 14547 "src/parser_bison.c"
    break;

  case 888: /* initializer_expr: "-" "number"  */
#line 4813 "src/parser_bison.y"
                        {
				int32_t num = -(yyvsp[0].val);

				(yyval.expr) = constant_expr_alloc(&(yyloc), &integer_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(num) * BITS_PER_BYTE,
							 &num);
			}
#line 14560 "src/parser_bison.c"
    break;

  case 889: /* counter_config: "packets" "number" "bytes" "number"  */
#line 4824 "src/parser_bison.y"
                        {
				struct counter *counter;

				counter = &(yyvsp[-4].obj)->counter;
				counter->packets = (yyvsp[-2].val);
				counter->bytes = (yyvsp[0].val);
			}
#line 14572 "src/parser_bison.c"
    break;

  case 890: /* counter_obj: %empty  */
#line 4834 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_COUNTER;
			}
#line 14581 "src/parser_bison.c"
    break;

  case 891: /* quota_config: quota_mode "number" bytes_unit quota_used  */
#line 4841 "src/parser_bison.y"
                        {
				struct quota *quota;

				quota = &(yyvsp[-4].obj)->quota;
				quota->bytes	= (yyvsp[-2].val) * (yyvsp[-1].val);
				quota->used	= (yyvsp[0].val);
				quota->flags	= (yyvsp[-3].val);
			}
#line 14594 "src/parser_bison.c"
    break;

  case 892: /* quota_obj: %empty  */
#line 4852 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_QUOTA;
			}
#line 14603 "src/parser_bison.c"
    break;

  case 893: /* secmark_config: string  */
#line 4859 "src/parser_bison.y"
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
#line 14621 "src/parser_bison.c"
    break;

  case 894: /* secmark_obj: %empty  */
#line 4875 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_SECMARK;
			}
#line 14630 "src/parser_bison.c"
    break;

  case 895: /* ct_obj_type: "helper"  */
#line 4881 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_HELPER; }
#line 14636 "src/parser_bison.c"
    break;

  case 896: /* ct_obj_type: "timeout"  */
#line 4882 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_TIMEOUT; }
#line 14642 "src/parser_bison.c"
    break;

  case 897: /* ct_obj_type: "expectation"  */
#line 4883 "src/parser_bison.y"
                                                { (yyval.val) = NFT_OBJECT_CT_EXPECT; }
#line 14648 "src/parser_bison.c"
    break;

  case 898: /* ct_cmd_type: "helpers"  */
#line 4886 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_HELPERS; }
#line 14654 "src/parser_bison.c"
    break;

  case 899: /* ct_cmd_type: "timeout"  */
#line 4887 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_TIMEOUTS; }
#line 14660 "src/parser_bison.c"
    break;

  case 900: /* ct_cmd_type: "expectation"  */
#line 4888 "src/parser_bison.y"
                                                { (yyval.val) = CMD_OBJ_CT_EXPECTATIONS; }
#line 14666 "src/parser_bison.c"
    break;

  case 901: /* ct_l4protoname: "tcp" close_scope_tcp  */
#line 4891 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_TCP; }
#line 14672 "src/parser_bison.c"
    break;

  case 902: /* ct_l4protoname: "udp" close_scope_udp  */
#line 4892 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_UDP; }
#line 14678 "src/parser_bison.c"
    break;

  case 903: /* ct_helper_config: "type" "quoted string" "protocol" ct_l4protoname stmt_separator close_scope_type  */
#line 4896 "src/parser_bison.y"
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
#line 14705 "src/parser_bison.c"
    break;

  case 904: /* ct_helper_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4919 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_helper.l3proto = (yyvsp[-1].val);
			}
#line 14713 "src/parser_bison.c"
    break;

  case 905: /* timeout_states: timeout_state  */
#line 4925 "src/parser_bison.y"
                        {
				(yyval.list) = xmalloc(sizeof(*(yyval.list)));
				init_list_head((yyval.list));
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyval.list));
			}
#line 14723 "src/parser_bison.c"
    break;

  case 906: /* timeout_states: timeout_states "comma" timeout_state  */
#line 4931 "src/parser_bison.y"
                        {
				list_add_tail(&(yyvsp[0].timeout_state)->head, (yyvsp[-2].list));
				(yyval.list) = (yyvsp[-2].list);
			}
#line 14732 "src/parser_bison.c"
    break;

  case 907: /* timeout_state: "string" "colon" time_spec_or_num_s  */
#line 4938 "src/parser_bison.y"
                        {
				struct timeout_state *ts;

				ts = xzalloc(sizeof(*ts));
				ts->timeout_str = (yyvsp[-2].string);
				ts->timeout_value = (yyvsp[0].val);
				ts->location = (yylsp[-2]);
				init_list_head(&ts->head);
				(yyval.timeout_state) = ts;
			}
#line 14747 "src/parser_bison.c"
    break;

  case 908: /* ct_timeout_config: "protocol" ct_l4protoname stmt_separator  */
#line 4951 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;
				int l4proto = (yyvsp[-1].val);

				ct = &(yyvsp[-3].obj)->ct_timeout;
				ct->l4proto = l4proto;
			}
#line 14759 "src/parser_bison.c"
    break;

  case 909: /* ct_timeout_config: "policy" '=' '{' timeout_states '}' stmt_separator close_scope_policy  */
#line 4959 "src/parser_bison.y"
                        {
				struct ct_timeout *ct;

				ct = &(yyvsp[-7].obj)->ct_timeout;
				list_splice_tail((yyvsp[-3].list), &ct->timeout_list);
				free((yyvsp[-3].list));
			}
#line 14771 "src/parser_bison.c"
    break;

  case 910: /* ct_timeout_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4967 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_timeout.l3proto = (yyvsp[-1].val);
			}
#line 14779 "src/parser_bison.c"
    break;

  case 911: /* ct_expect_config: "protocol" ct_l4protoname stmt_separator  */
#line 4973 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l4proto = (yyvsp[-1].val);
			}
#line 14787 "src/parser_bison.c"
    break;

  case 912: /* ct_expect_config: "dport" "number" stmt_separator  */
#line 4977 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.dport = (yyvsp[-1].val);
			}
#line 14795 "src/parser_bison.c"
    break;

  case 913: /* ct_expect_config: "timeout" time_spec stmt_separator  */
#line 4981 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.timeout = (yyvsp[-1].val);
			}
#line 14803 "src/parser_bison.c"
    break;

  case 914: /* ct_expect_config: "size" "number" stmt_separator  */
#line 4985 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.size = (yyvsp[-1].val);
			}
#line 14811 "src/parser_bison.c"
    break;

  case 915: /* ct_expect_config: "l3proto" family_spec_explicit stmt_separator  */
#line 4989 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->ct_expect.l3proto = (yyvsp[-1].val);
			}
#line 14819 "src/parser_bison.c"
    break;

  case 916: /* ct_obj_alloc: %empty  */
#line 4995 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
			}
#line 14827 "src/parser_bison.c"
    break;

  case 917: /* limit_config: "rate" limit_mode limit_rate_pkts limit_burst_pkts close_scope_rate  */
#line 5001 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-5].obj)->limit;
				limit->rate	= (yyvsp[-2].limit_rate).rate;
				limit->unit	= (yyvsp[-2].limit_rate).unit;
				limit->burst	= (yyvsp[-1].val);
				limit->type	= NFT_LIMIT_PKTS;
				limit->flags	= (yyvsp[-3].val);
			}
#line 14842 "src/parser_bison.c"
    break;

  case 918: /* limit_config: "rate" limit_mode limit_rate_bytes limit_burst_bytes close_scope_rate  */
#line 5012 "src/parser_bison.y"
                        {
				struct limit *limit;

				limit = &(yyvsp[-5].obj)->limit;
				limit->rate	= (yyvsp[-2].limit_rate).rate;
				limit->unit	= (yyvsp[-2].limit_rate).unit;
				limit->burst	= (yyvsp[-1].val);
				limit->type	= NFT_LIMIT_PKT_BYTES;
				limit->flags	= (yyvsp[-3].val);
			}
#line 14857 "src/parser_bison.c"
    break;

  case 919: /* limit_obj: %empty  */
#line 5025 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_LIMIT;
			}
#line 14866 "src/parser_bison.c"
    break;

  case 920: /* erspan_block: %empty  */
#line 5031 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14872 "src/parser_bison.c"
    break;

  case 923: /* erspan_block: erspan_block erspan_config stmt_separator  */
#line 5035 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14880 "src/parser_bison.c"
    break;

  case 924: /* erspan_block_alloc: %empty  */
#line 5041 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);

				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_ERSPAN, "erspan", state))
					YYERROR;
			}
#line 14891 "src/parser_bison.c"
    break;

  case 925: /* erspan_config: "version" "number"  */
#line 5050 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].obj)->tunnel.type == TUNNEL_ERSPAN);
				(yyvsp[-2].obj)->tunnel.erspan.version = (yyvsp[0].val);
			}
#line 14900 "src/parser_bison.c"
    break;

  case 926: /* erspan_config: "index" "number"  */
#line 5055 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v1.index = (yyvsp[0].val);
			}
#line 14908 "src/parser_bison.c"
    break;

  case 927: /* erspan_config: "direction" "ingress"  */
#line 5059 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.direction = 0;
			}
#line 14916 "src/parser_bison.c"
    break;

  case 928: /* erspan_config: "direction" "egress"  */
#line 5063 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.direction = 1;
			}
#line 14924 "src/parser_bison.c"
    break;

  case 929: /* erspan_config: "id" "number"  */
#line 5067 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.erspan.v2.hwid = (yyvsp[0].val);
			}
#line 14932 "src/parser_bison.c"
    break;

  case 930: /* geneve_block: %empty  */
#line 5072 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14938 "src/parser_bison.c"
    break;

  case 933: /* geneve_block: geneve_block geneve_config stmt_separator  */
#line 5076 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14946 "src/parser_bison.c"
    break;

  case 934: /* geneve_block_alloc: %empty  */
#line 5082 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);
				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_GENEVE, "geneve", state))
					YYERROR;

				init_list_head(&(yyval.obj)->tunnel.geneve_opts);
			}
#line 14958 "src/parser_bison.c"
    break;

  case 935: /* geneve_config: "class" "number" "opt-type" "number" "data" string  */
#line 5092 "src/parser_bison.y"
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
#line 14981 "src/parser_bison.c"
    break;

  case 936: /* vxlan_block: %empty  */
#line 5112 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 14987 "src/parser_bison.c"
    break;

  case 939: /* vxlan_block: vxlan_block vxlan_config stmt_separator  */
#line 5116 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 14995 "src/parser_bison.c"
    break;

  case 940: /* vxlan_block_alloc: %empty  */
#line 5122 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[(-1) - (0)].obj);

				if (!tunnel_set_type(&(yyval.obj)->location, (yyval.obj), TUNNEL_VXLAN, "vxlan", state))
					YYERROR;
			}
#line 15006 "src/parser_bison.c"
    break;

  case 941: /* vxlan_config: "gbp" "number"  */
#line 5131 "src/parser_bison.y"
                        {
				assert((yyvsp[-2].obj)->tunnel.type == TUNNEL_VXLAN);
				(yyvsp[-2].obj)->tunnel.vxlan.gbp = (yyvsp[0].val);
			}
#line 15015 "src/parser_bison.c"
    break;

  case 942: /* tunnel_config: "id" "number"  */
#line 5138 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.id = (yyvsp[0].val);
			}
#line 15023 "src/parser_bison.c"
    break;

  case 943: /* tunnel_config: "ip" "saddr" symbol_expr close_scope_ip  */
#line 5142 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.src, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}

				(yyvsp[-4].obj)->tunnel.src = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ipaddr_type);
			}
#line 15037 "src/parser_bison.c"
    break;

  case 944: /* tunnel_config: "ip" "daddr" symbol_expr close_scope_ip  */
#line 5152 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.dst, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.dst = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ipaddr_type);
			}
#line 15050 "src/parser_bison.c"
    break;

  case 945: /* tunnel_config: "ip6" "saddr" symbol_expr close_scope_ip6  */
#line 5161 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.src, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.src = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ip6addr_type);
			}
#line 15063 "src/parser_bison.c"
    break;

  case 946: /* tunnel_config: "ip6" "daddr" symbol_expr close_scope_ip6  */
#line 5170 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-4].obj)->tunnel.dst, &(yylsp[-1]), state)) {
					expr_free((yyvsp[-1].expr));
					YYERROR;
				}
				(yyvsp[-4].obj)->tunnel.dst = (yyvsp[-1].expr);
				datatype_set((yyvsp[-1].expr), &ip6addr_type);
			}
#line 15076 "src/parser_bison.c"
    break;

  case 947: /* tunnel_config: "sport" "number"  */
#line 5179 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.sport = (yyvsp[0].val);
			}
#line 15084 "src/parser_bison.c"
    break;

  case 948: /* tunnel_config: "dport" "number"  */
#line 5183 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.dport = (yyvsp[0].val);
			}
#line 15092 "src/parser_bison.c"
    break;

  case 949: /* tunnel_config: "ttl" "number"  */
#line 5187 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.ttl = (yyvsp[0].val);
			}
#line 15100 "src/parser_bison.c"
    break;

  case 950: /* tunnel_config: "tos" "number"  */
#line 5191 "src/parser_bison.y"
                        {
				(yyvsp[-2].obj)->tunnel.tos = (yyvsp[0].val);
			}
#line 15108 "src/parser_bison.c"
    break;

  case 951: /* tunnel_config: "erspan" erspan_block_alloc '{' erspan_block '}'  */
#line 5195 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 15116 "src/parser_bison.c"
    break;

  case 952: /* tunnel_config: "vxlan" vxlan_block_alloc '{' vxlan_block '}'  */
#line 5199 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 15124 "src/parser_bison.c"
    break;

  case 953: /* tunnel_config: "geneve" geneve_block_alloc '{' geneve_block '}'  */
#line 5203 "src/parser_bison.y"
                        {
				(yyvsp[-3].obj)->location = (yylsp[-3]);
			}
#line 15132 "src/parser_bison.c"
    break;

  case 954: /* tunnel_block: %empty  */
#line 5208 "src/parser_bison.y"
                                                { (yyval.obj) = (yyvsp[(-1) - (0)].obj); }
#line 15138 "src/parser_bison.c"
    break;

  case 957: /* tunnel_block: tunnel_block tunnel_config stmt_separator  */
#line 5212 "src/parser_bison.y"
                        {
				(yyval.obj) = (yyvsp[-2].obj);
			}
#line 15146 "src/parser_bison.c"
    break;

  case 958: /* tunnel_block: tunnel_block comment_spec  */
#line 5216 "src/parser_bison.y"
                        {
				if (already_set((yyvsp[-1].obj)->comment, &(yylsp[0]), state)) {
					free_const((yyvsp[0].string));
					YYERROR;
				}
				(yyvsp[-1].obj)->comment = (yyvsp[0].string);
			}
#line 15158 "src/parser_bison.c"
    break;

  case 959: /* tunnel_obj: %empty  */
#line 5226 "src/parser_bison.y"
                        {
				(yyval.obj) = obj_alloc(&(yyloc));
				(yyval.obj)->type = NFT_OBJECT_TUNNEL;
			}
#line 15167 "src/parser_bison.c"
    break;

  case 960: /* relational_expr: expr rhs_expr  */
#line 5233 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 15175 "src/parser_bison.c"
    break;

  case 961: /* relational_expr: expr list_rhs_expr  */
#line 5237 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, (yyvsp[-1].expr), (yyvsp[0].expr));
			}
#line 15183 "src/parser_bison.c"
    break;

  case 962: /* relational_expr: expr basic_rhs_expr "/" list_rhs_expr  */
#line 5241 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, (yyvsp[-2].expr));
			}
#line 15194 "src/parser_bison.c"
    break;

  case 963: /* relational_expr: expr list_rhs_expr "/" list_rhs_expr  */
#line 5248 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-3].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), OP_IMPLICIT, binop, value);
			}
#line 15206 "src/parser_bison.c"
    break;

  case 964: /* relational_expr: expr relational_op basic_rhs_expr "/" list_rhs_expr  */
#line 5256 "src/parser_bison.y"
                        {
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, (yyvsp[-2].expr));
			}
#line 15217 "src/parser_bison.c"
    break;

  case 965: /* relational_expr: expr relational_op list_rhs_expr "/" list_rhs_expr  */
#line 5263 "src/parser_bison.y"
                        {
				struct expr *value = list_expr_to_binop((yyvsp[-2].expr));
				struct expr *mask = list_expr_to_binop((yyvsp[0].expr));
				struct expr *binop = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-4].expr), mask);

				(yyval.expr) = relational_expr_alloc(&(yyloc), (yyvsp[-3].val), binop, value);
			}
#line 15229 "src/parser_bison.c"
    break;

  case 966: /* relational_expr: expr relational_op rhs_expr  */
#line 5271 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15237 "src/parser_bison.c"
    break;

  case 967: /* relational_expr: expr relational_op list_rhs_expr  */
#line 5275 "src/parser_bison.y"
                        {
				(yyval.expr) = relational_expr_alloc(&(yylsp[-1]), (yyvsp[-1].val), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15245 "src/parser_bison.c"
    break;

  case 968: /* list_rhs_expr: basic_rhs_expr "comma" basic_rhs_expr  */
#line 5281 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[-2].expr));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 15255 "src/parser_bison.c"
    break;

  case 969: /* list_rhs_expr: list_rhs_expr "comma" basic_rhs_expr  */
#line 5287 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 15265 "src/parser_bison.c"
    break;

  case 970: /* rhs_expr: concat_rhs_expr  */
#line 5294 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15271 "src/parser_bison.c"
    break;

  case 971: /* rhs_expr: set_expr  */
#line 5295 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15277 "src/parser_bison.c"
    break;

  case 972: /* rhs_expr: set_ref_symbol_expr  */
#line 5296 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15283 "src/parser_bison.c"
    break;

  case 974: /* shift_rhs_expr: shift_rhs_expr "<<" primary_rhs_expr  */
#line 5301 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_LSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15291 "src/parser_bison.c"
    break;

  case 975: /* shift_rhs_expr: shift_rhs_expr ">>" primary_rhs_expr  */
#line 5305 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_RSHIFT, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15299 "src/parser_bison.c"
    break;

  case 977: /* and_rhs_expr: and_rhs_expr "&" shift_rhs_expr  */
#line 5312 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_AND, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15307 "src/parser_bison.c"
    break;

  case 979: /* exclusive_or_rhs_expr: exclusive_or_rhs_expr "^" and_rhs_expr  */
#line 5319 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_XOR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15315 "src/parser_bison.c"
    break;

  case 981: /* inclusive_or_rhs_expr: inclusive_or_rhs_expr '|' exclusive_or_rhs_expr  */
#line 5326 "src/parser_bison.y"
                        {
				(yyval.expr) = binop_expr_alloc(&(yyloc), OP_OR, (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 15323 "src/parser_bison.c"
    break;

  case 985: /* concat_rhs_expr: concat_rhs_expr "." multiton_rhs_expr  */
#line 5337 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 15336 "src/parser_bison.c"
    break;

  case 986: /* concat_rhs_expr: concat_rhs_expr "." basic_rhs_expr  */
#line 5346 "src/parser_bison.y"
                        {
				struct location rhs[] = {
					[1]	= (yylsp[-1]),
					[2]	= (yylsp[0]),
				};

				(yyval.expr) = handle_concat_expr(&(yyloc), (yyval.expr), (yyvsp[-2].expr), (yyvsp[0].expr), rhs);
			}
#line 15349 "src/parser_bison.c"
    break;

  case 987: /* boolean_keys: "exists"  */
#line 5356 "src/parser_bison.y"
                                                { (yyval.val8) = true; }
#line 15355 "src/parser_bison.c"
    break;

  case 988: /* boolean_keys: "missing"  */
#line 5357 "src/parser_bison.y"
                                                { (yyval.val8) = false; }
#line 15361 "src/parser_bison.c"
    break;

  case 989: /* boolean_expr: boolean_keys  */
#line 5361 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &boolean_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof((yyvsp[0].val8)) * BITS_PER_BYTE, &(yyvsp[0].val8));
			}
#line 15371 "src/parser_bison.c"
    break;

  case 990: /* keyword_expr: "ether" close_scope_eth  */
#line 5368 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ether"); }
#line 15377 "src/parser_bison.c"
    break;

  case 991: /* keyword_expr: "ip" close_scope_ip  */
#line 5369 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip"); }
#line 15383 "src/parser_bison.c"
    break;

  case 992: /* keyword_expr: "ip6" close_scope_ip6  */
#line 5370 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ip6"); }
#line 15389 "src/parser_bison.c"
    break;

  case 993: /* keyword_expr: "vlan" close_scope_vlan  */
#line 5371 "src/parser_bison.y"
                                                         { (yyval.expr) = symbol_value(&(yyloc), "vlan"); }
#line 15395 "src/parser_bison.c"
    break;

  case 994: /* keyword_expr: "arp" close_scope_arp  */
#line 5372 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "arp"); }
#line 15401 "src/parser_bison.c"
    break;

  case 995: /* keyword_expr: "dnat" close_scope_nat  */
#line 5373 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "dnat"); }
#line 15407 "src/parser_bison.c"
    break;

  case 996: /* keyword_expr: "snat" close_scope_nat  */
#line 5374 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "snat"); }
#line 15413 "src/parser_bison.c"
    break;

  case 997: /* keyword_expr: "ecn"  */
#line 5375 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "ecn"); }
#line 15419 "src/parser_bison.c"
    break;

  case 998: /* keyword_expr: "reset" close_scope_reset  */
#line 5376 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "reset"); }
#line 15425 "src/parser_bison.c"
    break;

  case 999: /* keyword_expr: "destroy" close_scope_destroy  */
#line 5377 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "destroy"); }
#line 15431 "src/parser_bison.c"
    break;

  case 1000: /* keyword_expr: "original"  */
#line 5378 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "original"); }
#line 15437 "src/parser_bison.c"
    break;

  case 1001: /* keyword_expr: "reply"  */
#line 5379 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "reply"); }
#line 15443 "src/parser_bison.c"
    break;

  case 1002: /* keyword_expr: "label"  */
#line 5380 "src/parser_bison.y"
                                                        { (yyval.expr) = symbol_value(&(yyloc), "label"); }
#line 15449 "src/parser_bison.c"
    break;

  case 1003: /* keyword_expr: "last" close_scope_last  */
#line 5381 "src/parser_bison.y"
                                                                { (yyval.expr) = symbol_value(&(yyloc), "last"); }
#line 15455 "src/parser_bison.c"
    break;

  case 1004: /* primary_rhs_expr: symbol_expr  */
#line 5384 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15461 "src/parser_bison.c"
    break;

  case 1005: /* primary_rhs_expr: integer_expr  */
#line 5385 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15467 "src/parser_bison.c"
    break;

  case 1006: /* primary_rhs_expr: boolean_expr  */
#line 5386 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15473 "src/parser_bison.c"
    break;

  case 1007: /* primary_rhs_expr: keyword_expr  */
#line 5387 "src/parser_bison.y"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 15479 "src/parser_bison.c"
    break;

  case 1008: /* primary_rhs_expr: "tcp" close_scope_tcp  */
#line 5389 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_TCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15490 "src/parser_bison.c"
    break;

  case 1009: /* primary_rhs_expr: "udp" close_scope_udp  */
#line 5396 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15501 "src/parser_bison.c"
    break;

  case 1010: /* primary_rhs_expr: "udplite" close_scope_udplite  */
#line 5403 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_UDPLITE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15512 "src/parser_bison.c"
    break;

  case 1011: /* primary_rhs_expr: "esp" close_scope_esp  */
#line 5410 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ESP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15523 "src/parser_bison.c"
    break;

  case 1012: /* primary_rhs_expr: "ah" close_scope_ah  */
#line 5417 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_AH;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15534 "src/parser_bison.c"
    break;

  case 1013: /* primary_rhs_expr: "icmp" close_scope_icmp  */
#line 5424 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15545 "src/parser_bison.c"
    break;

  case 1014: /* primary_rhs_expr: "igmp"  */
#line 5431 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_IGMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15556 "src/parser_bison.c"
    break;

  case 1015: /* primary_rhs_expr: "icmpv6" close_scope_icmp  */
#line 5438 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_ICMPV6;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15567 "src/parser_bison.c"
    break;

  case 1016: /* primary_rhs_expr: "gre" close_scope_gre  */
#line 5445 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_GRE;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15578 "src/parser_bison.c"
    break;

  case 1017: /* primary_rhs_expr: "comp" close_scope_comp  */
#line 5452 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_COMP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15589 "src/parser_bison.c"
    break;

  case 1018: /* primary_rhs_expr: "dccp" close_scope_dccp  */
#line 5459 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_DCCP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15600 "src/parser_bison.c"
    break;

  case 1019: /* primary_rhs_expr: "sctp" close_scope_sctp  */
#line 5466 "src/parser_bison.y"
                        {
				uint8_t data = IPPROTO_SCTP;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &inet_protocol_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15611 "src/parser_bison.c"
    break;

  case 1020: /* primary_rhs_expr: "redirect" close_scope_nat  */
#line 5473 "src/parser_bison.y"
                        {
				uint8_t data = ICMP_REDIRECT;
				(yyval.expr) = constant_expr_alloc(&(yyloc), &icmp_type_type,
							 BYTEORDER_HOST_ENDIAN,
							 sizeof(data) * BITS_PER_BYTE, &data);
			}
#line 15622 "src/parser_bison.c"
    break;

  case 1021: /* primary_rhs_expr: '(' basic_rhs_expr ')'  */
#line 5479 "src/parser_bison.y"
                                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 15628 "src/parser_bison.c"
    break;

  case 1022: /* relational_op: "=="  */
#line 5482 "src/parser_bison.y"
                                                { (yyval.val) = OP_EQ; }
#line 15634 "src/parser_bison.c"
    break;

  case 1023: /* relational_op: "!="  */
#line 5483 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEQ; }
#line 15640 "src/parser_bison.c"
    break;

  case 1024: /* relational_op: "<"  */
#line 5484 "src/parser_bison.y"
                                                { (yyval.val) = OP_LT; }
#line 15646 "src/parser_bison.c"
    break;

  case 1025: /* relational_op: ">"  */
#line 5485 "src/parser_bison.y"
                                                { (yyval.val) = OP_GT; }
#line 15652 "src/parser_bison.c"
    break;

  case 1026: /* relational_op: ">="  */
#line 5486 "src/parser_bison.y"
                                                { (yyval.val) = OP_GTE; }
#line 15658 "src/parser_bison.c"
    break;

  case 1027: /* relational_op: "<="  */
#line 5487 "src/parser_bison.y"
                                                { (yyval.val) = OP_LTE; }
#line 15664 "src/parser_bison.c"
    break;

  case 1028: /* relational_op: "!"  */
#line 5488 "src/parser_bison.y"
                                                { (yyval.val) = OP_NEG; }
#line 15670 "src/parser_bison.c"
    break;

  case 1029: /* verdict_expr: "accept"  */
#line 5492 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_ACCEPT, NULL);
			}
#line 15678 "src/parser_bison.c"
    break;

  case 1030: /* verdict_expr: "drop"  */
#line 5496 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NF_DROP, NULL);
			}
#line 15686 "src/parser_bison.c"
    break;

  case 1031: /* verdict_expr: "continue"  */
#line 5500 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_CONTINUE, NULL);
			}
#line 15694 "src/parser_bison.c"
    break;

  case 1032: /* verdict_expr: "jump" chain_expr  */
#line 5504 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_JUMP, (yyvsp[0].expr));
			}
#line 15702 "src/parser_bison.c"
    break;

  case 1033: /* verdict_expr: "goto" chain_expr  */
#line 5508 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_GOTO, (yyvsp[0].expr));
			}
#line 15710 "src/parser_bison.c"
    break;

  case 1034: /* verdict_expr: "return"  */
#line 5512 "src/parser_bison.y"
                        {
				(yyval.expr) = verdict_expr_alloc(&(yyloc), NFT_RETURN, NULL);
			}
#line 15718 "src/parser_bison.c"
    break;

  case 1036: /* chain_expr: identifier  */
#line 5519 "src/parser_bison.y"
                        {
				(yyval.expr) = constant_expr_alloc(&(yyloc), &string_type,
							 BYTEORDER_HOST_ENDIAN,
							 strlen((yyvsp[0].string)) * BITS_PER_BYTE,
							 (yyvsp[0].string));
				free_const((yyvsp[0].string));
			}
#line 15730 "src/parser_bison.c"
    break;

  case 1037: /* meta_expr: "meta" meta_key close_scope_meta  */
#line 5529 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 15738 "src/parser_bison.c"
    break;

  case 1038: /* meta_expr: meta_key_unqualified  */
#line 5533 "src/parser_bison.y"
                        {
				(yyval.expr) = meta_expr_alloc(&(yyloc), (yyvsp[0].val));
			}
#line 15746 "src/parser_bison.c"
    break;

  case 1041: /* meta_key_qualified: "length"  */
#line 5542 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_LEN; }
#line 15752 "src/parser_bison.c"
    break;

  case 1042: /* meta_key_qualified: "protocol"  */
#line 5543 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PROTOCOL; }
#line 15758 "src/parser_bison.c"
    break;

  case 1043: /* meta_key_qualified: "priority"  */
#line 5544 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRIORITY; }
#line 15764 "src/parser_bison.c"
    break;

  case 1044: /* meta_key_qualified: "random"  */
#line 5545 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PRANDOM; }
#line 15770 "src/parser_bison.c"
    break;

  case 1045: /* meta_key_qualified: "secmark" close_scope_secmark  */
#line 5546 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_META_SECMARK; }
#line 15776 "src/parser_bison.c"
    break;

  case 1046: /* meta_key_qualified: "nfproto"  */
#line 5547 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_NFPROTO; }
#line 15782 "src/parser_bison.c"
    break;

  case 1047: /* meta_key_qualified: "l4proto"  */
#line 5548 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_L4PROTO; }
#line 15788 "src/parser_bison.c"
    break;

  case 1048: /* meta_key_qualified: "iifkind"  */
#line 5549 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFKIND; }
#line 15794 "src/parser_bison.c"
    break;

  case 1049: /* meta_key_qualified: "oifkind"  */
#line 5550 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFKIND; }
#line 15800 "src/parser_bison.c"
    break;

  case 1050: /* meta_key_qualified: "ibrpvid"  */
#line 5551 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFPVID; }
#line 15806 "src/parser_bison.c"
    break;

  case 1051: /* meta_key_qualified: "ibrvproto"  */
#line 5552 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFVPROTO; }
#line 15812 "src/parser_bison.c"
    break;

  case 1052: /* meta_key_qualified: "sdif"  */
#line 5553 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SDIF; }
#line 15818 "src/parser_bison.c"
    break;

  case 1053: /* meta_key_qualified: "sdifname"  */
#line 5554 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SDIFNAME; }
#line 15824 "src/parser_bison.c"
    break;

  case 1054: /* meta_key_qualified: "broute"  */
#line 5555 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_BROUTE; }
#line 15830 "src/parser_bison.c"
    break;

  case 1055: /* meta_key_qualified: "ibrhwaddr"  */
#line 5556 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFHWADDR; }
#line 15836 "src/parser_bison.c"
    break;

  case 1056: /* meta_key_unqualified: "mark"  */
#line 5559 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_MARK; }
#line 15842 "src/parser_bison.c"
    break;

  case 1057: /* meta_key_unqualified: "iif"  */
#line 5560 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIF; }
#line 15848 "src/parser_bison.c"
    break;

  case 1058: /* meta_key_unqualified: "iifname"  */
#line 5561 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFNAME; }
#line 15854 "src/parser_bison.c"
    break;

  case 1059: /* meta_key_unqualified: "iiftype"  */
#line 5562 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFTYPE; }
#line 15860 "src/parser_bison.c"
    break;

  case 1060: /* meta_key_unqualified: "oif"  */
#line 5563 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIF; }
#line 15866 "src/parser_bison.c"
    break;

  case 1061: /* meta_key_unqualified: "oifname"  */
#line 5564 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFNAME; }
#line 15872 "src/parser_bison.c"
    break;

  case 1062: /* meta_key_unqualified: "oiftype"  */
#line 5565 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFTYPE; }
#line 15878 "src/parser_bison.c"
    break;

  case 1063: /* meta_key_unqualified: "skuid"  */
#line 5566 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKUID; }
#line 15884 "src/parser_bison.c"
    break;

  case 1064: /* meta_key_unqualified: "skgid"  */
#line 5567 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_SKGID; }
#line 15890 "src/parser_bison.c"
    break;

  case 1065: /* meta_key_unqualified: "nftrace"  */
#line 5568 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_NFTRACE; }
#line 15896 "src/parser_bison.c"
    break;

  case 1066: /* meta_key_unqualified: "rtclassid"  */
#line 5569 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_RTCLASSID; }
#line 15902 "src/parser_bison.c"
    break;

  case 1067: /* meta_key_unqualified: "ibrname"  */
#line 5570 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_IIFNAME; }
#line 15908 "src/parser_bison.c"
    break;

  case 1068: /* meta_key_unqualified: "obrname"  */
#line 5571 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_BRI_OIFNAME; }
#line 15914 "src/parser_bison.c"
    break;

  case 1069: /* meta_key_unqualified: "pkttype"  */
#line 5572 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_PKTTYPE; }
#line 15920 "src/parser_bison.c"
    break;

  case 1070: /* meta_key_unqualified: "cpu"  */
#line 5573 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CPU; }
#line 15926 "src/parser_bison.c"
    break;

  case 1071: /* meta_key_unqualified: "iifgroup"  */
#line 5574 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_IIFGROUP; }
#line 15932 "src/parser_bison.c"
    break;

  case 1072: /* meta_key_unqualified: "oifgroup"  */
#line 5575 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_OIFGROUP; }
#line 15938 "src/parser_bison.c"
    break;

  case 1073: /* meta_key_unqualified: "cgroup"  */
#line 5576 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_CGROUP; }
#line 15944 "src/parser_bison.c"
    break;

  case 1074: /* meta_key_unqualified: "ipsec" close_scope_ipsec  */
#line 5577 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_META_SECPATH; }
#line 15950 "src/parser_bison.c"
    break;

  case 1075: /* meta_key_unqualified: "time"  */
#line 5578 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_NS; }
#line 15956 "src/parser_bison.c"
    break;

  case 1076: /* meta_key_unqualified: "day"  */
#line 5579 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_DAY; }
#line 15962 "src/parser_bison.c"
    break;

  case 1077: /* meta_key_unqualified: "hour"  */
#line 5580 "src/parser_bison.y"
                                                { (yyval.val) = NFT_META_TIME_HOUR; }
#line 15968 "src/parser_bison.c"
    break;

  case 1078: /* meta_stmt: "meta" meta_key "set" stmt_expr close_scope_meta  */
#line 5584 "src/parser_bison.y"
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
#line 15992 "src/parser_bison.c"
    break;

  case 1079: /* meta_stmt: meta_key_unqualified "set" stmt_expr  */
#line 5604 "src/parser_bison.y"
                        {
				(yyval.stmt) = meta_stmt_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[0].expr));
			}
#line 16000 "src/parser_bison.c"
    break;

  case 1080: /* meta_stmt: "notrack"  */
#line 5608 "src/parser_bison.y"
                        {
				(yyval.stmt) = notrack_stmt_alloc(&(yyloc));
			}
#line 16008 "src/parser_bison.c"
    break;

  case 1081: /* meta_stmt: "flow" "offload" "@" string close_scope_at  */
#line 5612 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 16016 "src/parser_bison.c"
    break;

  case 1082: /* meta_stmt: "flow" "add" "@" string close_scope_at  */
#line 5616 "src/parser_bison.y"
                        {
				(yyval.stmt) = flow_offload_stmt_alloc(&(yyloc), (yyvsp[-1].string));
			}
#line 16024 "src/parser_bison.c"
    break;

  case 1083: /* socket_expr: "socket" socket_key close_scope_socket  */
#line 5622 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), (yyvsp[-1].val), 0);
			}
#line 16032 "src/parser_bison.c"
    break;

  case 1084: /* socket_expr: "socket" "cgroupv2" "level" "number" close_scope_socket  */
#line 5626 "src/parser_bison.y"
                        {
				(yyval.expr) = socket_expr_alloc(&(yyloc), NFT_SOCKET_CGROUPV2, (yyvsp[-1].val));
			}
#line 16040 "src/parser_bison.c"
    break;

  case 1085: /* socket_key: "transparent"  */
#line 5631 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_TRANSPARENT; }
#line 16046 "src/parser_bison.c"
    break;

  case 1086: /* socket_key: "mark"  */
#line 5632 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_MARK; }
#line 16052 "src/parser_bison.c"
    break;

  case 1087: /* socket_key: "wildcard"  */
#line 5633 "src/parser_bison.y"
                                                { (yyval.val) = NFT_SOCKET_WILDCARD; }
#line 16058 "src/parser_bison.c"
    break;

  case 1088: /* tunnel_key: "path"  */
#line 5636 "src/parser_bison.y"
                                                { (yyval.val) = NFT_TUNNEL_PATH; }
#line 16064 "src/parser_bison.c"
    break;

  case 1089: /* tunnel_key: "id"  */
#line 5637 "src/parser_bison.y"
                                                { (yyval.val) = NFT_TUNNEL_ID; }
#line 16070 "src/parser_bison.c"
    break;

  case 1090: /* tunnel_expr: "tunnel" tunnel_key  */
#line 5641 "src/parser_bison.y"
                        {
				(yyval.expr) = tunnel_expr_alloc(&(yyloc), (yyvsp[0].val));
			}
#line 16078 "src/parser_bison.c"
    break;

  case 1091: /* offset_opt: %empty  */
#line 5646 "src/parser_bison.y"
                                                { (yyval.val) = 0; }
#line 16084 "src/parser_bison.c"
    break;

  case 1092: /* offset_opt: "offset" "number"  */
#line 5647 "src/parser_bison.y"
                                                { (yyval.val) = (yyvsp[0].val); }
#line 16090 "src/parser_bison.c"
    break;

  case 1093: /* numgen_type: "inc"  */
#line 5650 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_INCREMENTAL; }
#line 16096 "src/parser_bison.c"
    break;

  case 1094: /* numgen_type: "random"  */
#line 5651 "src/parser_bison.y"
                                                { (yyval.val) = NFT_NG_RANDOM; }
#line 16102 "src/parser_bison.c"
    break;

  case 1095: /* numgen_expr: "numgen" numgen_type "mod" "number" offset_opt close_scope_numgen  */
#line 5655 "src/parser_bison.y"
                        {
				(yyval.expr) = numgen_expr_alloc(&(yyloc), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 16110 "src/parser_bison.c"
    break;

  case 1096: /* xfrm_spnum: "spnum" "number"  */
#line 5660 "src/parser_bison.y"
                                            { (yyval.val) = (yyvsp[0].val); }
#line 16116 "src/parser_bison.c"
    break;

  case 1097: /* xfrm_spnum: %empty  */
#line 5661 "src/parser_bison.y"
                                            { (yyval.val) = 0; }
#line 16122 "src/parser_bison.c"
    break;

  case 1098: /* xfrm_dir: "in"  */
#line 5664 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_IN; }
#line 16128 "src/parser_bison.c"
    break;

  case 1099: /* xfrm_dir: "out"  */
#line 5665 "src/parser_bison.y"
                                        { (yyval.val) = XFRM_POLICY_OUT; }
#line 16134 "src/parser_bison.c"
    break;

  case 1100: /* xfrm_state_key: "spi"  */
#line 5668 "src/parser_bison.y"
                                    { (yyval.val) = NFT_XFRM_KEY_SPI; }
#line 16140 "src/parser_bison.c"
    break;

  case 1101: /* xfrm_state_key: "reqid"  */
#line 5669 "src/parser_bison.y"
                                      { (yyval.val) = NFT_XFRM_KEY_REQID; }
#line 16146 "src/parser_bison.c"
    break;

  case 1102: /* xfrm_state_proto_key: "daddr"  */
#line 5672 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_DADDR_IP4; }
#line 16152 "src/parser_bison.c"
    break;

  case 1103: /* xfrm_state_proto_key: "saddr"  */
#line 5673 "src/parser_bison.y"
                                                { (yyval.val) = NFT_XFRM_KEY_SADDR_IP4; }
#line 16158 "src/parser_bison.c"
    break;

  case 1104: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum xfrm_state_key close_scope_ipsec  */
#line 5677 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].val) > 255) {
					erec_queue(error(&(yylsp[-2]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.expr) = xfrm_expr_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-2].val), (yyvsp[-1].val));
			}
#line 16170 "src/parser_bison.c"
    break;

  case 1105: /* xfrm_expr: "ipsec" xfrm_dir xfrm_spnum nf_key_proto xfrm_state_proto_key close_scope_ipsec  */
#line 5685 "src/parser_bison.y"
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
#line 16199 "src/parser_bison.c"
    break;

  case 1106: /* hash_expr: "jhash" expr "mod" "number" "seed" "number" offset_opt close_scope_hash  */
#line 5712 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-4].val), true, (yyvsp[-2].val), (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-6].expr);
			}
#line 16208 "src/parser_bison.c"
    break;

  case 1107: /* hash_expr: "jhash" expr "mod" "number" offset_opt close_scope_hash  */
#line 5717 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_JENKINS);
				(yyval.expr)->hash.expr = (yyvsp[-4].expr);
			}
#line 16217 "src/parser_bison.c"
    break;

  case 1108: /* hash_expr: "symhash" "mod" "number" offset_opt close_scope_hash  */
#line 5722 "src/parser_bison.y"
                        {
				(yyval.expr) = hash_expr_alloc(&(yyloc), (yyvsp[-2].val), false, 0, (yyvsp[-1].val), NFT_HASH_SYM);
			}
#line 16225 "src/parser_bison.c"
    break;

  case 1109: /* nf_key_proto: "ip" close_scope_ip  */
#line 5727 "src/parser_bison.y"
                                                       { (yyval.val) = NFPROTO_IPV4; }
#line 16231 "src/parser_bison.c"
    break;

  case 1110: /* nf_key_proto: "ip6" close_scope_ip6  */
#line 5728 "src/parser_bison.y"
                                                        { (yyval.val) = NFPROTO_IPV6; }
#line 16237 "src/parser_bison.c"
    break;

  case 1111: /* rt_expr: "rt" rt_key close_scope_rt  */
#line 5732 "src/parser_bison.y"
                        {
				(yyval.expr) = rt_expr_alloc(&(yyloc), (yyvsp[-1].val), true);
			}
#line 16245 "src/parser_bison.c"
    break;

  case 1112: /* rt_expr: "rt" nf_key_proto rt_key close_scope_rt  */
#line 5736 "src/parser_bison.y"
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
#line 16267 "src/parser_bison.c"
    break;

  case 1113: /* rt_key: "classid"  */
#line 5755 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_CLASSID; }
#line 16273 "src/parser_bison.c"
    break;

  case 1114: /* rt_key: "nexthop"  */
#line 5756 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_NEXTHOP4; }
#line 16279 "src/parser_bison.c"
    break;

  case 1115: /* rt_key: "mtu"  */
#line 5757 "src/parser_bison.y"
                                                { (yyval.val) = NFT_RT_TCPMSS; }
#line 16285 "src/parser_bison.c"
    break;

  case 1116: /* rt_key: "ipsec" close_scope_ipsec  */
#line 5758 "src/parser_bison.y"
                                                          { (yyval.val) = NFT_RT_XFRM; }
#line 16291 "src/parser_bison.c"
    break;

  case 1117: /* ct_expr: "ct" ct_key close_scope_ct  */
#line 5762 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), -1);
			}
#line 16299 "src/parser_bison.c"
    break;

  case 1118: /* ct_expr: "ct" ct_dir ct_key_dir close_scope_ct  */
#line 5766 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 16307 "src/parser_bison.c"
    break;

  case 1119: /* ct_expr: "ct" ct_dir ct_key_proto_field close_scope_ct  */
#line 5770 "src/parser_bison.y"
                        {
				(yyval.expr) = ct_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[-2].val));
			}
#line 16315 "src/parser_bison.c"
    break;

  case 1120: /* ct_dir: "original"  */
#line 5775 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_ORIGINAL; }
#line 16321 "src/parser_bison.c"
    break;

  case 1121: /* ct_dir: "reply"  */
#line 5776 "src/parser_bison.y"
                                                { (yyval.val) = IP_CT_DIR_REPLY; }
#line 16327 "src/parser_bison.c"
    break;

  case 1122: /* ct_key: "l3proto"  */
#line 5779 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 16333 "src/parser_bison.c"
    break;

  case 1123: /* ct_key: "protocol"  */
#line 5780 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 16339 "src/parser_bison.c"
    break;

  case 1124: /* ct_key: "mark"  */
#line 5781 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_MARK; }
#line 16345 "src/parser_bison.c"
    break;

  case 1125: /* ct_key: "state"  */
#line 5782 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATE; }
#line 16351 "src/parser_bison.c"
    break;

  case 1126: /* ct_key: "direction"  */
#line 5783 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DIRECTION; }
#line 16357 "src/parser_bison.c"
    break;

  case 1127: /* ct_key: "status"  */
#line 5784 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_STATUS; }
#line 16363 "src/parser_bison.c"
    break;

  case 1128: /* ct_key: "expiration"  */
#line 5785 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EXPIRATION; }
#line 16369 "src/parser_bison.c"
    break;

  case 1129: /* ct_key: "helper"  */
#line 5786 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_HELPER; }
#line 16375 "src/parser_bison.c"
    break;

  case 1130: /* ct_key: "saddr"  */
#line 5787 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 16381 "src/parser_bison.c"
    break;

  case 1131: /* ct_key: "daddr"  */
#line 5788 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 16387 "src/parser_bison.c"
    break;

  case 1132: /* ct_key: "proto-src"  */
#line 5789 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 16393 "src/parser_bison.c"
    break;

  case 1133: /* ct_key: "proto-dst"  */
#line 5790 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 16399 "src/parser_bison.c"
    break;

  case 1134: /* ct_key: "label"  */
#line 5791 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_LABELS; }
#line 16405 "src/parser_bison.c"
    break;

  case 1135: /* ct_key: "event"  */
#line 5792 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_EVENTMASK; }
#line 16411 "src/parser_bison.c"
    break;

  case 1136: /* ct_key: "secmark" close_scope_secmark  */
#line 5793 "src/parser_bison.y"
                                                            { (yyval.val) = NFT_CT_SECMARK; }
#line 16417 "src/parser_bison.c"
    break;

  case 1137: /* ct_key: "id"  */
#line 5794 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ID; }
#line 16423 "src/parser_bison.c"
    break;

  case 1139: /* ct_key_dir: "saddr"  */
#line 5798 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_SRC; }
#line 16429 "src/parser_bison.c"
    break;

  case 1140: /* ct_key_dir: "daddr"  */
#line 5799 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_DST; }
#line 16435 "src/parser_bison.c"
    break;

  case 1141: /* ct_key_dir: "l3proto"  */
#line 5800 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_L3PROTOCOL; }
#line 16441 "src/parser_bison.c"
    break;

  case 1142: /* ct_key_dir: "protocol"  */
#line 5801 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTOCOL; }
#line 16447 "src/parser_bison.c"
    break;

  case 1143: /* ct_key_dir: "proto-src"  */
#line 5802 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_SRC; }
#line 16453 "src/parser_bison.c"
    break;

  case 1144: /* ct_key_dir: "proto-dst"  */
#line 5803 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PROTO_DST; }
#line 16459 "src/parser_bison.c"
    break;

  case 1146: /* ct_key_proto_field: "ip" "saddr" close_scope_ip  */
#line 5807 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_SRC_IP; }
#line 16465 "src/parser_bison.c"
    break;

  case 1147: /* ct_key_proto_field: "ip" "daddr" close_scope_ip  */
#line 5808 "src/parser_bison.y"
                                                               { (yyval.val) = NFT_CT_DST_IP; }
#line 16471 "src/parser_bison.c"
    break;

  case 1148: /* ct_key_proto_field: "ip6" "saddr" close_scope_ip6  */
#line 5809 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_SRC_IP6; }
#line 16477 "src/parser_bison.c"
    break;

  case 1149: /* ct_key_proto_field: "ip6" "daddr" close_scope_ip6  */
#line 5810 "src/parser_bison.y"
                                                                { (yyval.val) = NFT_CT_DST_IP6; }
#line 16483 "src/parser_bison.c"
    break;

  case 1150: /* ct_key_dir_optional: "bytes"  */
#line 5813 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_BYTES; }
#line 16489 "src/parser_bison.c"
    break;

  case 1151: /* ct_key_dir_optional: "packets"  */
#line 5814 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_PKTS; }
#line 16495 "src/parser_bison.c"
    break;

  case 1152: /* ct_key_dir_optional: "avgpkt"  */
#line 5815 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_AVGPKT; }
#line 16501 "src/parser_bison.c"
    break;

  case 1153: /* ct_key_dir_optional: "zone"  */
#line 5816 "src/parser_bison.y"
                                                { (yyval.val) = NFT_CT_ZONE; }
#line 16507 "src/parser_bison.c"
    break;

  case 1156: /* list_stmt_expr: symbol_stmt_expr "comma" symbol_stmt_expr  */
#line 5824 "src/parser_bison.y"
                        {
				(yyval.expr) = list_expr_alloc(&(yyloc));
				list_expr_add((yyval.expr), (yyvsp[-2].expr));
				list_expr_add((yyval.expr), (yyvsp[0].expr));
			}
#line 16517 "src/parser_bison.c"
    break;

  case 1157: /* list_stmt_expr: list_stmt_expr "comma" symbol_stmt_expr  */
#line 5830 "src/parser_bison.y"
                        {
				(yyvsp[-2].expr)->location = (yyloc);
				list_expr_add((yyvsp[-2].expr), (yyvsp[0].expr));
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 16527 "src/parser_bison.c"
    break;

  case 1158: /* ct_stmt: "ct" ct_key "set" stmt_expr close_scope_ct  */
#line 5838 "src/parser_bison.y"
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
#line 16544 "src/parser_bison.c"
    break;

  case 1159: /* ct_stmt: "ct" ct_dir ct_key_dir_optional "set" stmt_expr close_scope_ct  */
#line 5851 "src/parser_bison.y"
                        {
				(yyval.stmt) = ct_stmt_alloc(&(yyloc), (yyvsp[-3].val), (yyvsp[-4].val), (yyvsp[-1].expr));
			}
#line 16552 "src/parser_bison.c"
    break;

  case 1160: /* payload_stmt: payload_expr "set" stmt_expr  */
#line 5857 "src/parser_bison.y"
                        {
				if ((yyvsp[-2].expr)->etype == EXPR_EXTHDR)
					(yyval.stmt) = exthdr_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
				else
					(yyval.stmt) = payload_stmt_alloc(&(yyloc), (yyvsp[-2].expr), (yyvsp[0].expr));
			}
#line 16563 "src/parser_bison.c"
    break;

  case 1183: /* payload_raw_len: "number"  */
#line 5890 "src/parser_bison.y"
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
#line 16583 "src/parser_bison.c"
    break;

  case 1184: /* payload_raw_expr: "@" payload_base_spec "comma" "number" "comma" payload_raw_len close_scope_at  */
#line 5908 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), NULL, 0);
				payload_init_raw((yyval.expr), (yyvsp[-5].val), (yyvsp[-3].val), (yyvsp[-1].val));
				(yyval.expr)->byteorder		= BYTEORDER_BIG_ENDIAN;
				(yyval.expr)->payload.is_raw	= true;
			}
#line 16594 "src/parser_bison.c"
    break;

  case 1185: /* payload_base_spec: "ll"  */
#line 5916 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_LL_HDR; }
#line 16600 "src/parser_bison.c"
    break;

  case 1186: /* payload_base_spec: "nh"  */
#line 5917 "src/parser_bison.y"
                                                { (yyval.val) = PROTO_BASE_NETWORK_HDR; }
#line 16606 "src/parser_bison.c"
    break;

  case 1187: /* payload_base_spec: "th" close_scope_th  */
#line 5918 "src/parser_bison.y"
                                                                { (yyval.val) = PROTO_BASE_TRANSPORT_HDR; }
#line 16612 "src/parser_bison.c"
    break;

  case 1188: /* payload_base_spec: "string"  */
#line 5920 "src/parser_bison.y"
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
#line 16627 "src/parser_bison.c"
    break;

  case 1189: /* eth_hdr_expr: "ether" eth_hdr_field close_scope_eth  */
#line 5933 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_eth, (yyvsp[-1].val));
			}
#line 16635 "src/parser_bison.c"
    break;

  case 1190: /* eth_hdr_field: "saddr"  */
#line 5938 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_SADDR; }
#line 16641 "src/parser_bison.c"
    break;

  case 1191: /* eth_hdr_field: "daddr"  */
#line 5939 "src/parser_bison.y"
                                                { (yyval.val) = ETHHDR_DADDR; }
#line 16647 "src/parser_bison.c"
    break;

  case 1192: /* eth_hdr_field: "type" close_scope_type  */
#line 5940 "src/parser_bison.y"
                                                                        { (yyval.val) = ETHHDR_TYPE; }
#line 16653 "src/parser_bison.c"
    break;

  case 1193: /* vlan_hdr_expr: "vlan" vlan_hdr_field close_scope_vlan  */
#line 5944 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_vlan, (yyvsp[-1].val));
			}
#line 16661 "src/parser_bison.c"
    break;

  case 1194: /* vlan_hdr_field: "id"  */
#line 5949 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_VID; }
#line 16667 "src/parser_bison.c"
    break;

  case 1195: /* vlan_hdr_field: "cfi"  */
#line 5950 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_CFI; }
#line 16673 "src/parser_bison.c"
    break;

  case 1196: /* vlan_hdr_field: "dei"  */
#line 5951 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_DEI; }
#line 16679 "src/parser_bison.c"
    break;

  case 1197: /* vlan_hdr_field: "pcp"  */
#line 5952 "src/parser_bison.y"
                                                { (yyval.val) = VLANHDR_PCP; }
#line 16685 "src/parser_bison.c"
    break;

  case 1198: /* vlan_hdr_field: "type" close_scope_type  */
#line 5953 "src/parser_bison.y"
                                                                        { (yyval.val) = VLANHDR_TYPE; }
#line 16691 "src/parser_bison.c"
    break;

  case 1199: /* arp_hdr_expr: "arp" arp_hdr_field close_scope_arp  */
#line 5957 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_arp, (yyvsp[-1].val));
			}
#line 16699 "src/parser_bison.c"
    break;

  case 1200: /* arp_hdr_field: "htype"  */
#line 5962 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HRD; }
#line 16705 "src/parser_bison.c"
    break;

  case 1201: /* arp_hdr_field: "ptype"  */
#line 5963 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PRO; }
#line 16711 "src/parser_bison.c"
    break;

  case 1202: /* arp_hdr_field: "hlen"  */
#line 5964 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_HLN; }
#line 16717 "src/parser_bison.c"
    break;

  case 1203: /* arp_hdr_field: "plen"  */
#line 5965 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_PLN; }
#line 16723 "src/parser_bison.c"
    break;

  case 1204: /* arp_hdr_field: "operation"  */
#line 5966 "src/parser_bison.y"
                                                { (yyval.val) = ARPHDR_OP; }
#line 16729 "src/parser_bison.c"
    break;

  case 1205: /* arp_hdr_field: "saddr" "ether" close_scope_eth  */
#line 5967 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_ETHER; }
#line 16735 "src/parser_bison.c"
    break;

  case 1206: /* arp_hdr_field: "daddr" "ether" close_scope_eth  */
#line 5968 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_ETHER; }
#line 16741 "src/parser_bison.c"
    break;

  case 1207: /* arp_hdr_field: "saddr" "ip" close_scope_ip  */
#line 5969 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_SADDR_IP; }
#line 16747 "src/parser_bison.c"
    break;

  case 1208: /* arp_hdr_field: "daddr" "ip" close_scope_ip  */
#line 5970 "src/parser_bison.y"
                                                                { (yyval.val) = ARPHDR_DADDR_IP; }
#line 16753 "src/parser_bison.c"
    break;

  case 1209: /* ip_hdr_expr: "ip" ip_hdr_field close_scope_ip  */
#line 5974 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip, (yyvsp[-1].val));
			}
#line 16761 "src/parser_bison.c"
    break;

  case 1210: /* ip_hdr_expr: "ip" "option" ip_option_type ip_option_field close_scope_ip  */
#line 5978 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-2].val), (yyvsp[-1].val));
				if (!(yyval.expr)) {
					erec_queue(error(&(yylsp[-4]), "unknown ip option type/field"), state->msgs);
					YYERROR;
				}

				if ((yyvsp[-1].val) == IPOPT_FIELD_TYPE)
					(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 16776 "src/parser_bison.c"
    break;

  case 1211: /* ip_hdr_expr: "ip" "option" ip_option_type close_scope_ip  */
#line 5989 "src/parser_bison.y"
                        {
				(yyval.expr) = ipopt_expr_alloc(&(yyloc), (yyvsp[-1].val), IPOPT_FIELD_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 16785 "src/parser_bison.c"
    break;

  case 1212: /* ip_hdr_field: "version"  */
#line 5995 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_VERSION; }
#line 16791 "src/parser_bison.c"
    break;

  case 1213: /* ip_hdr_field: "hdrlength"  */
#line 5996 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_HDRLENGTH; }
#line 16797 "src/parser_bison.c"
    break;

  case 1214: /* ip_hdr_field: "dscp"  */
#line 5997 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DSCP; }
#line 16803 "src/parser_bison.c"
    break;

  case 1215: /* ip_hdr_field: "ecn"  */
#line 5998 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ECN; }
#line 16809 "src/parser_bison.c"
    break;

  case 1216: /* ip_hdr_field: "length"  */
#line 5999 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_LENGTH; }
#line 16815 "src/parser_bison.c"
    break;

  case 1217: /* ip_hdr_field: "id"  */
#line 6000 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_ID; }
#line 16821 "src/parser_bison.c"
    break;

  case 1218: /* ip_hdr_field: "frag-off"  */
#line 6001 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_FRAG_OFF; }
#line 16827 "src/parser_bison.c"
    break;

  case 1219: /* ip_hdr_field: "ttl"  */
#line 6002 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_TTL; }
#line 16833 "src/parser_bison.c"
    break;

  case 1220: /* ip_hdr_field: "protocol"  */
#line 6003 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_PROTOCOL; }
#line 16839 "src/parser_bison.c"
    break;

  case 1221: /* ip_hdr_field: "checksum"  */
#line 6004 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_CHECKSUM; }
#line 16845 "src/parser_bison.c"
    break;

  case 1222: /* ip_hdr_field: "saddr"  */
#line 6005 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_SADDR; }
#line 16851 "src/parser_bison.c"
    break;

  case 1223: /* ip_hdr_field: "daddr"  */
#line 6006 "src/parser_bison.y"
                                                { (yyval.val) = IPHDR_DADDR; }
#line 16857 "src/parser_bison.c"
    break;

  case 1224: /* ip_option_type: "lsrr"  */
#line 6009 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_LSRR; }
#line 16863 "src/parser_bison.c"
    break;

  case 1225: /* ip_option_type: "rr"  */
#line 6010 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RR; }
#line 16869 "src/parser_bison.c"
    break;

  case 1226: /* ip_option_type: "ssrr"  */
#line 6011 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_SSRR; }
#line 16875 "src/parser_bison.c"
    break;

  case 1227: /* ip_option_type: "ra"  */
#line 6012 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_RA; }
#line 16881 "src/parser_bison.c"
    break;

  case 1228: /* ip_option_field: "type" close_scope_type  */
#line 6015 "src/parser_bison.y"
                                                                        { (yyval.val) = IPOPT_FIELD_TYPE; }
#line 16887 "src/parser_bison.c"
    break;

  case 1229: /* ip_option_field: "length"  */
#line 6016 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_LENGTH; }
#line 16893 "src/parser_bison.c"
    break;

  case 1230: /* ip_option_field: "value"  */
#line 6017 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_VALUE; }
#line 16899 "src/parser_bison.c"
    break;

  case 1231: /* ip_option_field: "ptr"  */
#line 6018 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_PTR; }
#line 16905 "src/parser_bison.c"
    break;

  case 1232: /* ip_option_field: "addr"  */
#line 6019 "src/parser_bison.y"
                                                { (yyval.val) = IPOPT_FIELD_ADDR_0; }
#line 16911 "src/parser_bison.c"
    break;

  case 1233: /* icmp_hdr_expr: "icmp" icmp_hdr_field close_scope_icmp  */
#line 6023 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp, (yyvsp[-1].val));
			}
#line 16919 "src/parser_bison.c"
    break;

  case 1234: /* icmp_hdr_field: "type" close_scope_type  */
#line 6028 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMPHDR_TYPE; }
#line 16925 "src/parser_bison.c"
    break;

  case 1235: /* icmp_hdr_field: "code"  */
#line 6029 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CODE; }
#line 16931 "src/parser_bison.c"
    break;

  case 1236: /* icmp_hdr_field: "checksum"  */
#line 6030 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_CHECKSUM; }
#line 16937 "src/parser_bison.c"
    break;

  case 1237: /* icmp_hdr_field: "id"  */
#line 6031 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_ID; }
#line 16943 "src/parser_bison.c"
    break;

  case 1238: /* icmp_hdr_field: "seq"  */
#line 6032 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_SEQ; }
#line 16949 "src/parser_bison.c"
    break;

  case 1239: /* icmp_hdr_field: "gateway"  */
#line 6033 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_GATEWAY; }
#line 16955 "src/parser_bison.c"
    break;

  case 1240: /* icmp_hdr_field: "mtu"  */
#line 6034 "src/parser_bison.y"
                                                { (yyval.val) = ICMPHDR_MTU; }
#line 16961 "src/parser_bison.c"
    break;

  case 1241: /* igmp_hdr_expr: "igmp" igmp_hdr_field close_scope_igmp  */
#line 6038 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_igmp, (yyvsp[-1].val));
			}
#line 16969 "src/parser_bison.c"
    break;

  case 1242: /* igmp_hdr_field: "type" close_scope_type  */
#line 6043 "src/parser_bison.y"
                                                                        { (yyval.val) = IGMPHDR_TYPE; }
#line 16975 "src/parser_bison.c"
    break;

  case 1243: /* igmp_hdr_field: "checksum"  */
#line 6044 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_CHECKSUM; }
#line 16981 "src/parser_bison.c"
    break;

  case 1244: /* igmp_hdr_field: "mrt"  */
#line 6045 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_MRT; }
#line 16987 "src/parser_bison.c"
    break;

  case 1245: /* igmp_hdr_field: "group"  */
#line 6046 "src/parser_bison.y"
                                                { (yyval.val) = IGMPHDR_GROUP; }
#line 16993 "src/parser_bison.c"
    break;

  case 1246: /* ip6_hdr_expr: "ip6" ip6_hdr_field close_scope_ip6  */
#line 6050 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ip6, (yyvsp[-1].val));
			}
#line 17001 "src/parser_bison.c"
    break;

  case 1247: /* ip6_hdr_field: "version"  */
#line 6055 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_VERSION; }
#line 17007 "src/parser_bison.c"
    break;

  case 1248: /* ip6_hdr_field: "dscp"  */
#line 6056 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DSCP; }
#line 17013 "src/parser_bison.c"
    break;

  case 1249: /* ip6_hdr_field: "ecn"  */
#line 6057 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_ECN; }
#line 17019 "src/parser_bison.c"
    break;

  case 1250: /* ip6_hdr_field: "flowlabel"  */
#line 6058 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_FLOWLABEL; }
#line 17025 "src/parser_bison.c"
    break;

  case 1251: /* ip6_hdr_field: "length"  */
#line 6059 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_LENGTH; }
#line 17031 "src/parser_bison.c"
    break;

  case 1252: /* ip6_hdr_field: "nexthdr"  */
#line 6060 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_NEXTHDR; }
#line 17037 "src/parser_bison.c"
    break;

  case 1253: /* ip6_hdr_field: "hoplimit"  */
#line 6061 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_HOPLIMIT; }
#line 17043 "src/parser_bison.c"
    break;

  case 1254: /* ip6_hdr_field: "saddr"  */
#line 6062 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_SADDR; }
#line 17049 "src/parser_bison.c"
    break;

  case 1255: /* ip6_hdr_field: "daddr"  */
#line 6063 "src/parser_bison.y"
                                                { (yyval.val) = IP6HDR_DADDR; }
#line 17055 "src/parser_bison.c"
    break;

  case 1256: /* icmp6_hdr_expr: "icmpv6" icmp6_hdr_field close_scope_icmp  */
#line 6066 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_icmp6, (yyvsp[-1].val));
			}
#line 17063 "src/parser_bison.c"
    break;

  case 1257: /* icmp6_hdr_field: "type" close_scope_type  */
#line 6071 "src/parser_bison.y"
                                                                        { (yyval.val) = ICMP6HDR_TYPE; }
#line 17069 "src/parser_bison.c"
    break;

  case 1258: /* icmp6_hdr_field: "code"  */
#line 6072 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CODE; }
#line 17075 "src/parser_bison.c"
    break;

  case 1259: /* icmp6_hdr_field: "checksum"  */
#line 6073 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_CHECKSUM; }
#line 17081 "src/parser_bison.c"
    break;

  case 1260: /* icmp6_hdr_field: "param-problem"  */
#line 6074 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_PPTR; }
#line 17087 "src/parser_bison.c"
    break;

  case 1261: /* icmp6_hdr_field: "mtu"  */
#line 6075 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MTU; }
#line 17093 "src/parser_bison.c"
    break;

  case 1262: /* icmp6_hdr_field: "id"  */
#line 6076 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_ID; }
#line 17099 "src/parser_bison.c"
    break;

  case 1263: /* icmp6_hdr_field: "seq"  */
#line 6077 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_SEQ; }
#line 17105 "src/parser_bison.c"
    break;

  case 1264: /* icmp6_hdr_field: "max-delay"  */
#line 6078 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_MAXDELAY; }
#line 17111 "src/parser_bison.c"
    break;

  case 1265: /* icmp6_hdr_field: "taddr"  */
#line 6079 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_TADDR; }
#line 17117 "src/parser_bison.c"
    break;

  case 1266: /* icmp6_hdr_field: "daddr"  */
#line 6080 "src/parser_bison.y"
                                                { (yyval.val) = ICMP6HDR_DADDR; }
#line 17123 "src/parser_bison.c"
    break;

  case 1267: /* auth_hdr_expr: "ah" auth_hdr_field close_scope_ah  */
#line 6084 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_ah, (yyvsp[-1].val));
			}
#line 17131 "src/parser_bison.c"
    break;

  case 1268: /* auth_hdr_field: "nexthdr"  */
#line 6089 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_NEXTHDR; }
#line 17137 "src/parser_bison.c"
    break;

  case 1269: /* auth_hdr_field: "hdrlength"  */
#line 6090 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_HDRLENGTH; }
#line 17143 "src/parser_bison.c"
    break;

  case 1270: /* auth_hdr_field: "reserved"  */
#line 6091 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_RESERVED; }
#line 17149 "src/parser_bison.c"
    break;

  case 1271: /* auth_hdr_field: "spi"  */
#line 6092 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SPI; }
#line 17155 "src/parser_bison.c"
    break;

  case 1272: /* auth_hdr_field: "seq"  */
#line 6093 "src/parser_bison.y"
                                                { (yyval.val) = AHHDR_SEQUENCE; }
#line 17161 "src/parser_bison.c"
    break;

  case 1273: /* esp_hdr_expr: "esp" esp_hdr_field close_scope_esp  */
#line 6097 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_esp, (yyvsp[-1].val));
			}
#line 17169 "src/parser_bison.c"
    break;

  case 1274: /* esp_hdr_field: "spi"  */
#line 6102 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SPI; }
#line 17175 "src/parser_bison.c"
    break;

  case 1275: /* esp_hdr_field: "seq"  */
#line 6103 "src/parser_bison.y"
                                                { (yyval.val) = ESPHDR_SEQUENCE; }
#line 17181 "src/parser_bison.c"
    break;

  case 1276: /* comp_hdr_expr: "comp" comp_hdr_field close_scope_comp  */
#line 6107 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_comp, (yyvsp[-1].val));
			}
#line 17189 "src/parser_bison.c"
    break;

  case 1277: /* comp_hdr_field: "nexthdr"  */
#line 6112 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_NEXTHDR; }
#line 17195 "src/parser_bison.c"
    break;

  case 1278: /* comp_hdr_field: "flags"  */
#line 6113 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_FLAGS; }
#line 17201 "src/parser_bison.c"
    break;

  case 1279: /* comp_hdr_field: "cpi"  */
#line 6114 "src/parser_bison.y"
                                                { (yyval.val) = COMPHDR_CPI; }
#line 17207 "src/parser_bison.c"
    break;

  case 1280: /* udp_hdr_expr: "udp" udp_hdr_field close_scope_udp  */
#line 6118 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udp, (yyvsp[-1].val));
			}
#line 17215 "src/parser_bison.c"
    break;

  case 1281: /* udp_hdr_field: "sport"  */
#line 6123 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 17221 "src/parser_bison.c"
    break;

  case 1282: /* udp_hdr_field: "dport"  */
#line 6124 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 17227 "src/parser_bison.c"
    break;

  case 1283: /* udp_hdr_field: "length"  */
#line 6125 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 17233 "src/parser_bison.c"
    break;

  case 1284: /* udp_hdr_field: "checksum"  */
#line 6126 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 17239 "src/parser_bison.c"
    break;

  case 1285: /* udplite_hdr_expr: "udplite" udplite_hdr_field close_scope_udplite  */
#line 6130 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_udplite, (yyvsp[-1].val));
			}
#line 17247 "src/parser_bison.c"
    break;

  case 1286: /* udplite_hdr_field: "sport"  */
#line 6135 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_SPORT; }
#line 17253 "src/parser_bison.c"
    break;

  case 1287: /* udplite_hdr_field: "dport"  */
#line 6136 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_DPORT; }
#line 17259 "src/parser_bison.c"
    break;

  case 1288: /* udplite_hdr_field: "csumcov"  */
#line 6137 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_LENGTH; }
#line 17265 "src/parser_bison.c"
    break;

  case 1289: /* udplite_hdr_field: "checksum"  */
#line 6138 "src/parser_bison.y"
                                                { (yyval.val) = UDPHDR_CHECKSUM; }
#line 17271 "src/parser_bison.c"
    break;

  case 1290: /* tcp_hdr_expr: "tcp" tcp_hdr_field  */
#line 6142 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_tcp, (yyvsp[0].val));
			}
#line 17279 "src/parser_bison.c"
    break;

  case 1291: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_type  */
#line 6146 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].val), TCPOPT_COMMON_KIND);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17288 "src/parser_bison.c"
    break;

  case 1292: /* tcp_hdr_expr: "tcp" "option" tcp_hdr_option_kind_and_field  */
#line 6151 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[0].tcp_kind_field).kind, (yyvsp[0].tcp_kind_field).field);
				if ((yyval.expr) == NULL) {
					erec_queue(error(&(yylsp[-2]), "Could not find a tcp option template"), state->msgs);
					YYERROR;
				}
			}
#line 17300 "src/parser_bison.c"
    break;

  case 1293: /* tcp_hdr_expr: "tcp" "option" "@" close_scope_at tcp_hdr_option_type "comma" "number" "comma" payload_raw_len  */
#line 6159 "src/parser_bison.y"
                        {
				(yyval.expr) = tcpopt_expr_alloc(&(yyloc), (yyvsp[-4].val), 0);
				tcpopt_init_raw((yyval.expr), (yyvsp[-4].val), (yyvsp[-2].val), (yyvsp[0].val), 0);
			}
#line 17309 "src/parser_bison.c"
    break;

  case 1313: /* vxlan_hdr_expr: "vxlan" vxlan_hdr_field  */
#line 6191 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_vxlan, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_vxlan;
				(yyval.expr) = expr;
			}
#line 17321 "src/parser_bison.c"
    break;

  case 1314: /* vxlan_hdr_expr: "vxlan" inner_expr  */
#line 6199 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_vxlan;
			}
#line 17331 "src/parser_bison.c"
    break;

  case 1315: /* vxlan_hdr_field: "vni"  */
#line 6206 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_VNI; }
#line 17337 "src/parser_bison.c"
    break;

  case 1316: /* vxlan_hdr_field: "flags"  */
#line 6207 "src/parser_bison.y"
                                                        { (yyval.val) = VXLANHDR_FLAGS; }
#line 17343 "src/parser_bison.c"
    break;

  case 1317: /* geneve_hdr_expr: "geneve" geneve_hdr_field  */
#line 6211 "src/parser_bison.y"
                        {
				struct expr *expr;

				expr = payload_expr_alloc(&(yyloc), &proto_geneve, (yyvsp[0].val));
				expr->payload.inner_desc = &proto_geneve;
				(yyval.expr) = expr;
			}
#line 17355 "src/parser_bison.c"
    break;

  case 1318: /* geneve_hdr_expr: "geneve" inner_expr  */
#line 6219 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->location = (yyloc);
				(yyval.expr)->payload.inner_desc = &proto_geneve;
			}
#line 17365 "src/parser_bison.c"
    break;

  case 1319: /* geneve_hdr_field: "vni"  */
#line 6226 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_VNI; }
#line 17371 "src/parser_bison.c"
    break;

  case 1320: /* geneve_hdr_field: "type"  */
#line 6227 "src/parser_bison.y"
                                                        { (yyval.val) = GNVHDR_TYPE; }
#line 17377 "src/parser_bison.c"
    break;

  case 1321: /* gre_hdr_expr: "gre" gre_hdr_field close_scope_gre  */
#line 6231 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_gre, (yyvsp[-1].val));
			}
#line 17385 "src/parser_bison.c"
    break;

  case 1322: /* gre_hdr_expr: "gre" close_scope_gre inner_inet_expr  */
#line 6235 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gre;
			}
#line 17394 "src/parser_bison.c"
    break;

  case 1323: /* gre_hdr_field: "version"  */
#line 6241 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_VERSION;	}
#line 17400 "src/parser_bison.c"
    break;

  case 1324: /* gre_hdr_field: "flags"  */
#line 6242 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_FLAGS; }
#line 17406 "src/parser_bison.c"
    break;

  case 1325: /* gre_hdr_field: "protocol"  */
#line 6243 "src/parser_bison.y"
                                                        { (yyval.val) = GREHDR_PROTOCOL; }
#line 17412 "src/parser_bison.c"
    break;

  case 1326: /* gretap_hdr_expr: "gretap" close_scope_gre inner_expr  */
#line 6247 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[0].expr);
				(yyval.expr)->payload.inner_desc = &proto_gretap;
			}
#line 17421 "src/parser_bison.c"
    break;

  case 1327: /* optstrip_stmt: "reset" "tcp" "option" tcp_hdr_option_type close_scope_tcp  */
#line 6254 "src/parser_bison.y"
                        {
				(yyval.stmt) = optstrip_stmt_alloc(&(yyloc), tcpopt_expr_alloc(&(yyloc),
										(yyvsp[-1].val), TCPOPT_COMMON_KIND));
			}
#line 17430 "src/parser_bison.c"
    break;

  case 1328: /* tcp_hdr_field: "sport"  */
#line 6260 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SPORT; }
#line 17436 "src/parser_bison.c"
    break;

  case 1329: /* tcp_hdr_field: "dport"  */
#line 6261 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DPORT; }
#line 17442 "src/parser_bison.c"
    break;

  case 1330: /* tcp_hdr_field: "seq"  */
#line 6262 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_SEQ; }
#line 17448 "src/parser_bison.c"
    break;

  case 1331: /* tcp_hdr_field: "ackseq"  */
#line 6263 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_ACKSEQ; }
#line 17454 "src/parser_bison.c"
    break;

  case 1332: /* tcp_hdr_field: "doff"  */
#line 6264 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_DOFF; }
#line 17460 "src/parser_bison.c"
    break;

  case 1333: /* tcp_hdr_field: "reserved"  */
#line 6265 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_RESERVED; }
#line 17466 "src/parser_bison.c"
    break;

  case 1334: /* tcp_hdr_field: "flags"  */
#line 6266 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_FLAGS; }
#line 17472 "src/parser_bison.c"
    break;

  case 1335: /* tcp_hdr_field: "window"  */
#line 6267 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_WINDOW; }
#line 17478 "src/parser_bison.c"
    break;

  case 1336: /* tcp_hdr_field: "checksum"  */
#line 6268 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_CHECKSUM; }
#line 17484 "src/parser_bison.c"
    break;

  case 1337: /* tcp_hdr_field: "urgptr"  */
#line 6269 "src/parser_bison.y"
                                                { (yyval.val) = TCPHDR_URGPTR; }
#line 17490 "src/parser_bison.c"
    break;

  case 1338: /* tcp_hdr_option_kind_and_field: "mss" tcpopt_field_maxseg  */
#line 6273 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MAXSEG, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17499 "src/parser_bison.c"
    break;

  case 1339: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_sack tcpopt_field_sack  */
#line 6278 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17508 "src/parser_bison.c"
    break;

  case 1340: /* tcp_hdr_option_kind_and_field: "window" tcpopt_field_window  */
#line 6283 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_WINDOW, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17517 "src/parser_bison.c"
    break;

  case 1341: /* tcp_hdr_option_kind_and_field: "timestamp" tcpopt_field_tsopt  */
#line 6288 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_TIMESTAMP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17526 "src/parser_bison.c"
    break;

  case 1342: /* tcp_hdr_option_kind_and_field: tcp_hdr_option_type "length"  */
#line 6293 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = (yyvsp[-1].val), .field = TCPOPT_COMMON_LENGTH };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17535 "src/parser_bison.c"
    break;

  case 1343: /* tcp_hdr_option_kind_and_field: "mptcp" tcpopt_field_mptcp  */
#line 6298 "src/parser_bison.y"
                                {
					struct tcp_kind_field kind_field = { .kind = TCPOPT_KIND_MPTCP, .field = (yyvsp[0].val) };
					(yyval.tcp_kind_field) = kind_field;
				}
#line 17544 "src/parser_bison.c"
    break;

  case 1344: /* tcp_hdr_option_sack: "sack"  */
#line 6304 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 17550 "src/parser_bison.c"
    break;

  case 1345: /* tcp_hdr_option_sack: "sack0"  */
#line 6305 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK; }
#line 17556 "src/parser_bison.c"
    break;

  case 1346: /* tcp_hdr_option_sack: "sack1"  */
#line 6306 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK1; }
#line 17562 "src/parser_bison.c"
    break;

  case 1347: /* tcp_hdr_option_sack: "sack2"  */
#line 6307 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK2; }
#line 17568 "src/parser_bison.c"
    break;

  case 1348: /* tcp_hdr_option_sack: "sack3"  */
#line 6308 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_KIND_SACK3; }
#line 17574 "src/parser_bison.c"
    break;

  case 1349: /* tcp_hdr_option_type: "echo"  */
#line 6311 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_ECHO; }
#line 17580 "src/parser_bison.c"
    break;

  case 1350: /* tcp_hdr_option_type: "eol"  */
#line 6312 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_EOL; }
#line 17586 "src/parser_bison.c"
    break;

  case 1351: /* tcp_hdr_option_type: "fastopen"  */
#line 6313 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_FASTOPEN; }
#line 17592 "src/parser_bison.c"
    break;

  case 1352: /* tcp_hdr_option_type: "md5sig"  */
#line 6314 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MD5SIG; }
#line 17598 "src/parser_bison.c"
    break;

  case 1353: /* tcp_hdr_option_type: "mptcp"  */
#line 6315 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MPTCP; }
#line 17604 "src/parser_bison.c"
    break;

  case 1354: /* tcp_hdr_option_type: "mss"  */
#line 6316 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_MAXSEG; }
#line 17610 "src/parser_bison.c"
    break;

  case 1355: /* tcp_hdr_option_type: "nop"  */
#line 6317 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_NOP; }
#line 17616 "src/parser_bison.c"
    break;

  case 1356: /* tcp_hdr_option_type: "sack-permitted"  */
#line 6318 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_SACK_PERMITTED; }
#line 17622 "src/parser_bison.c"
    break;

  case 1357: /* tcp_hdr_option_type: "timestamp"  */
#line 6319 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_TIMESTAMP; }
#line 17628 "src/parser_bison.c"
    break;

  case 1358: /* tcp_hdr_option_type: "window"  */
#line 6320 "src/parser_bison.y"
                                                        { (yyval.val) = TCPOPT_KIND_WINDOW; }
#line 17634 "src/parser_bison.c"
    break;

  case 1359: /* tcp_hdr_option_type: tcp_hdr_option_sack  */
#line 6321 "src/parser_bison.y"
                                                        { (yyval.val) = (yyvsp[0].val); }
#line 17640 "src/parser_bison.c"
    break;

  case 1360: /* tcp_hdr_option_type: "number"  */
#line 6322 "src/parser_bison.y"
                                                        {
				if ((yyvsp[0].val) > 255) {
					erec_queue(error(&(yylsp[0]), "value too large"), state->msgs);
					YYERROR;
				}
				(yyval.val) = (yyvsp[0].val);
			}
#line 17652 "src/parser_bison.c"
    break;

  case 1361: /* tcpopt_field_sack: "left"  */
#line 6331 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_LEFT; }
#line 17658 "src/parser_bison.c"
    break;

  case 1362: /* tcpopt_field_sack: "right"  */
#line 6332 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_SACK_RIGHT; }
#line 17664 "src/parser_bison.c"
    break;

  case 1363: /* tcpopt_field_window: "count"  */
#line 6335 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_WINDOW_COUNT; }
#line 17670 "src/parser_bison.c"
    break;

  case 1364: /* tcpopt_field_tsopt: "tsval"  */
#line 6338 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSVAL; }
#line 17676 "src/parser_bison.c"
    break;

  case 1365: /* tcpopt_field_tsopt: "tsecr"  */
#line 6339 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_TS_TSECR; }
#line 17682 "src/parser_bison.c"
    break;

  case 1366: /* tcpopt_field_maxseg: "size"  */
#line 6342 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MAXSEG_SIZE; }
#line 17688 "src/parser_bison.c"
    break;

  case 1367: /* tcpopt_field_mptcp: "subtype"  */
#line 6345 "src/parser_bison.y"
                                                { (yyval.val) = TCPOPT_MPTCP_SUBTYPE; }
#line 17694 "src/parser_bison.c"
    break;

  case 1368: /* dccp_hdr_expr: "dccp" dccp_hdr_field close_scope_dccp  */
#line 6349 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_dccp, (yyvsp[-1].val));
			}
#line 17702 "src/parser_bison.c"
    break;

  case 1369: /* dccp_hdr_expr: "dccp" "option" "number" close_scope_dccp  */
#line 6353 "src/parser_bison.y"
                        {
				if ((yyvsp[-1].val) > DCCPOPT_TYPE_MAX) {
					erec_queue(error(&(yylsp[-3]), "value too large"),
						   state->msgs);
					YYERROR;
				}
				(yyval.expr) = dccpopt_expr_alloc(&(yyloc), (yyvsp[-1].val));
			}
#line 17715 "src/parser_bison.c"
    break;

  case 1370: /* dccp_hdr_field: "sport"  */
#line 6363 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_SPORT; }
#line 17721 "src/parser_bison.c"
    break;

  case 1371: /* dccp_hdr_field: "dport"  */
#line 6364 "src/parser_bison.y"
                                                { (yyval.val) = DCCPHDR_DPORT; }
#line 17727 "src/parser_bison.c"
    break;

  case 1372: /* dccp_hdr_field: "type" close_scope_type  */
#line 6365 "src/parser_bison.y"
                                                                        { (yyval.val) = DCCPHDR_TYPE; }
#line 17733 "src/parser_bison.c"
    break;

  case 1373: /* sctp_chunk_type: "data"  */
#line 6368 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_DATA; }
#line 17739 "src/parser_bison.c"
    break;

  case 1374: /* sctp_chunk_type: "init"  */
#line 6369 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT; }
#line 17745 "src/parser_bison.c"
    break;

  case 1375: /* sctp_chunk_type: "init-ack"  */
#line 6370 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_INIT_ACK; }
#line 17751 "src/parser_bison.c"
    break;

  case 1376: /* sctp_chunk_type: "sack"  */
#line 6371 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SACK; }
#line 17757 "src/parser_bison.c"
    break;

  case 1377: /* sctp_chunk_type: "heartbeat"  */
#line 6372 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT; }
#line 17763 "src/parser_bison.c"
    break;

  case 1378: /* sctp_chunk_type: "heartbeat-ack"  */
#line 6373 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_HEARTBEAT_ACK; }
#line 17769 "src/parser_bison.c"
    break;

  case 1379: /* sctp_chunk_type: "abort"  */
#line 6374 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ABORT; }
#line 17775 "src/parser_bison.c"
    break;

  case 1380: /* sctp_chunk_type: "shutdown"  */
#line 6375 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN; }
#line 17781 "src/parser_bison.c"
    break;

  case 1381: /* sctp_chunk_type: "shutdown-ack"  */
#line 6376 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_ACK; }
#line 17787 "src/parser_bison.c"
    break;

  case 1382: /* sctp_chunk_type: "error"  */
#line 6377 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ERROR; }
#line 17793 "src/parser_bison.c"
    break;

  case 1383: /* sctp_chunk_type: "cookie-echo"  */
#line 6378 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ECHO; }
#line 17799 "src/parser_bison.c"
    break;

  case 1384: /* sctp_chunk_type: "cookie-ack"  */
#line 6379 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_COOKIE_ACK; }
#line 17805 "src/parser_bison.c"
    break;

  case 1385: /* sctp_chunk_type: "ecne"  */
#line 6380 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ECNE; }
#line 17811 "src/parser_bison.c"
    break;

  case 1386: /* sctp_chunk_type: "cwr"  */
#line 6381 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_CWR; }
#line 17817 "src/parser_bison.c"
    break;

  case 1387: /* sctp_chunk_type: "shutdown-complete"  */
#line 6382 "src/parser_bison.y"
                                                  { (yyval.val) = SCTP_CHUNK_TYPE_SHUTDOWN_COMPLETE; }
#line 17823 "src/parser_bison.c"
    break;

  case 1388: /* sctp_chunk_type: "asconf-ack"  */
#line 6383 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF_ACK; }
#line 17829 "src/parser_bison.c"
    break;

  case 1389: /* sctp_chunk_type: "forward-tsn"  */
#line 6384 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_FORWARD_TSN; }
#line 17835 "src/parser_bison.c"
    break;

  case 1390: /* sctp_chunk_type: "asconf"  */
#line 6385 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_TYPE_ASCONF; }
#line 17841 "src/parser_bison.c"
    break;

  case 1391: /* sctp_chunk_common_field: "type" close_scope_type  */
#line 6388 "src/parser_bison.y"
                                                                { (yyval.val) = SCTP_CHUNK_COMMON_TYPE; }
#line 17847 "src/parser_bison.c"
    break;

  case 1392: /* sctp_chunk_common_field: "flags"  */
#line 6389 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_FLAGS; }
#line 17853 "src/parser_bison.c"
    break;

  case 1393: /* sctp_chunk_common_field: "length"  */
#line 6390 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_COMMON_LENGTH; }
#line 17859 "src/parser_bison.c"
    break;

  case 1394: /* sctp_chunk_data_field: "tsn"  */
#line 6393 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_TSN; }
#line 17865 "src/parser_bison.c"
    break;

  case 1395: /* sctp_chunk_data_field: "stream"  */
#line 6394 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_STREAM; }
#line 17871 "src/parser_bison.c"
    break;

  case 1396: /* sctp_chunk_data_field: "ssn"  */
#line 6395 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_SSN; }
#line 17877 "src/parser_bison.c"
    break;

  case 1397: /* sctp_chunk_data_field: "ppid"  */
#line 6396 "src/parser_bison.y"
                                        { (yyval.val) = SCTP_CHUNK_DATA_PPID; }
#line 17883 "src/parser_bison.c"
    break;

  case 1398: /* sctp_chunk_init_field: "init-tag"  */
#line 6399 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TAG; }
#line 17889 "src/parser_bison.c"
    break;

  case 1399: /* sctp_chunk_init_field: "a-rwnd"  */
#line 6400 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_RWND; }
#line 17895 "src/parser_bison.c"
    break;

  case 1400: /* sctp_chunk_init_field: "num-outbound-streams"  */
#line 6401 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_OSTREAMS; }
#line 17901 "src/parser_bison.c"
    break;

  case 1401: /* sctp_chunk_init_field: "num-inbound-streams"  */
#line 6402 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_ISTREAMS; }
#line 17907 "src/parser_bison.c"
    break;

  case 1402: /* sctp_chunk_init_field: "initial-tsn"  */
#line 6403 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_INIT_TSN; }
#line 17913 "src/parser_bison.c"
    break;

  case 1403: /* sctp_chunk_sack_field: "cum-tsn-ack"  */
#line 6406 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_CTSN_ACK; }
#line 17919 "src/parser_bison.c"
    break;

  case 1404: /* sctp_chunk_sack_field: "a-rwnd"  */
#line 6407 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_RWND; }
#line 17925 "src/parser_bison.c"
    break;

  case 1405: /* sctp_chunk_sack_field: "num-gap-ack-blocks"  */
#line 6408 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_GACK_BLOCKS; }
#line 17931 "src/parser_bison.c"
    break;

  case 1406: /* sctp_chunk_sack_field: "num-dup-tsns"  */
#line 6409 "src/parser_bison.y"
                                                { (yyval.val) = SCTP_CHUNK_SACK_DUP_TSNS; }
#line 17937 "src/parser_bison.c"
    break;

  case 1407: /* sctp_chunk_alloc: sctp_chunk_type  */
#line 6413 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[0].val), SCTP_CHUNK_COMMON_TYPE);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 17946 "src/parser_bison.c"
    break;

  case 1408: /* sctp_chunk_alloc: sctp_chunk_type sctp_chunk_common_field  */
#line 6418 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), (yyvsp[-1].val), (yyvsp[0].val));
			}
#line 17954 "src/parser_bison.c"
    break;

  case 1409: /* sctp_chunk_alloc: "data" sctp_chunk_data_field  */
#line 6422 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_DATA, (yyvsp[0].val));
			}
#line 17962 "src/parser_bison.c"
    break;

  case 1410: /* sctp_chunk_alloc: "init" sctp_chunk_init_field  */
#line 6426 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT, (yyvsp[0].val));
			}
#line 17970 "src/parser_bison.c"
    break;

  case 1411: /* sctp_chunk_alloc: "init-ack" sctp_chunk_init_field  */
#line 6430 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_INIT_ACK, (yyvsp[0].val));
			}
#line 17978 "src/parser_bison.c"
    break;

  case 1412: /* sctp_chunk_alloc: "sack" sctp_chunk_sack_field  */
#line 6434 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SACK, (yyvsp[0].val));
			}
#line 17986 "src/parser_bison.c"
    break;

  case 1413: /* sctp_chunk_alloc: "shutdown" "cum-tsn-ack"  */
#line 6438 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_SHUTDOWN,
							   SCTP_CHUNK_SHUTDOWN_CTSN_ACK);
			}
#line 17995 "src/parser_bison.c"
    break;

  case 1414: /* sctp_chunk_alloc: "ecne" "lowest-tsn"  */
#line 6443 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ECNE,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 18004 "src/parser_bison.c"
    break;

  case 1415: /* sctp_chunk_alloc: "cwr" "lowest-tsn"  */
#line 6448 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_CWR,
							   SCTP_CHUNK_ECNE_CWR_MIN_TSN);
			}
#line 18013 "src/parser_bison.c"
    break;

  case 1416: /* sctp_chunk_alloc: "asconf-ack" "seqno"  */
#line 6453 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF_ACK,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 18022 "src/parser_bison.c"
    break;

  case 1417: /* sctp_chunk_alloc: "forward-tsn" "new-cum-tsn"  */
#line 6458 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_FORWARD_TSN,
							   SCTP_CHUNK_FORWARD_TSN_NCTSN);
			}
#line 18031 "src/parser_bison.c"
    break;

  case 1418: /* sctp_chunk_alloc: "asconf" "seqno"  */
#line 6463 "src/parser_bison.y"
                        {
				(yyval.expr) = sctp_chunk_expr_alloc(&(yyloc), SCTP_CHUNK_TYPE_ASCONF,
							   SCTP_CHUNK_ASCONF_SEQNO);
			}
#line 18040 "src/parser_bison.c"
    break;

  case 1419: /* sctp_hdr_expr: "sctp" sctp_hdr_field close_scope_sctp  */
#line 6470 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_sctp, (yyvsp[-1].val));
			}
#line 18048 "src/parser_bison.c"
    break;

  case 1420: /* sctp_hdr_expr: "sctp" "chunk" sctp_chunk_alloc close_scope_sctp_chunk close_scope_sctp  */
#line 6474 "src/parser_bison.y"
                        {
				(yyval.expr) = (yyvsp[-2].expr);
			}
#line 18056 "src/parser_bison.c"
    break;

  case 1421: /* sctp_hdr_field: "sport"  */
#line 6479 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_SPORT; }
#line 18062 "src/parser_bison.c"
    break;

  case 1422: /* sctp_hdr_field: "dport"  */
#line 6480 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_DPORT; }
#line 18068 "src/parser_bison.c"
    break;

  case 1423: /* sctp_hdr_field: "vtag"  */
#line 6481 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_VTAG; }
#line 18074 "src/parser_bison.c"
    break;

  case 1424: /* sctp_hdr_field: "checksum"  */
#line 6482 "src/parser_bison.y"
                                                { (yyval.val) = SCTPHDR_CHECKSUM; }
#line 18080 "src/parser_bison.c"
    break;

  case 1425: /* th_hdr_expr: "th" th_hdr_field close_scope_th  */
#line 6486 "src/parser_bison.y"
                        {
				(yyval.expr) = payload_expr_alloc(&(yyloc), &proto_th, (yyvsp[-1].val));
				if ((yyval.expr))
					(yyval.expr)->payload.is_raw = true;
			}
#line 18090 "src/parser_bison.c"
    break;

  case 1426: /* th_hdr_field: "sport"  */
#line 6493 "src/parser_bison.y"
                                                { (yyval.val) = THDR_SPORT; }
#line 18096 "src/parser_bison.c"
    break;

  case 1427: /* th_hdr_field: "dport"  */
#line 6494 "src/parser_bison.y"
                                                { (yyval.val) = THDR_DPORT; }
#line 18102 "src/parser_bison.c"
    break;

  case 1436: /* hbh_hdr_expr: "hbh" hbh_hdr_field close_scope_hbh  */
#line 6508 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_hbh, (yyvsp[-1].val));
			}
#line 18110 "src/parser_bison.c"
    break;

  case 1437: /* hbh_hdr_field: "nexthdr"  */
#line 6513 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_NEXTHDR; }
#line 18116 "src/parser_bison.c"
    break;

  case 1438: /* hbh_hdr_field: "hdrlength"  */
#line 6514 "src/parser_bison.y"
                                                { (yyval.val) = HBHHDR_HDRLENGTH; }
#line 18122 "src/parser_bison.c"
    break;

  case 1439: /* rt_hdr_expr: "rt" rt_hdr_field close_scope_rt  */
#line 6518 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt, (yyvsp[-1].val));
			}
#line 18130 "src/parser_bison.c"
    break;

  case 1440: /* rt_hdr_field: "nexthdr"  */
#line 6523 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_NEXTHDR; }
#line 18136 "src/parser_bison.c"
    break;

  case 1441: /* rt_hdr_field: "hdrlength"  */
#line 6524 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_HDRLENGTH; }
#line 18142 "src/parser_bison.c"
    break;

  case 1442: /* rt_hdr_field: "type" close_scope_type  */
#line 6525 "src/parser_bison.y"
                                                                        { (yyval.val) = RTHDR_TYPE; }
#line 18148 "src/parser_bison.c"
    break;

  case 1443: /* rt_hdr_field: "seg-left"  */
#line 6526 "src/parser_bison.y"
                                                { (yyval.val) = RTHDR_SEG_LEFT; }
#line 18154 "src/parser_bison.c"
    break;

  case 1444: /* rt0_hdr_expr: "rt0" rt0_hdr_field close_scope_rt  */
#line 6530 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt0, (yyvsp[-1].val));
			}
#line 18162 "src/parser_bison.c"
    break;

  case 1445: /* rt0_hdr_field: "addr" '[' "number" ']'  */
#line 6536 "src/parser_bison.y"
                        {
				(yyval.val) = RT0HDR_ADDR_1 + (yyvsp[-1].val) - 1;
			}
#line 18170 "src/parser_bison.c"
    break;

  case 1446: /* rt2_hdr_expr: "rt2" rt2_hdr_field close_scope_rt  */
#line 6542 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt2, (yyvsp[-1].val));
			}
#line 18178 "src/parser_bison.c"
    break;

  case 1447: /* rt2_hdr_field: "addr"  */
#line 6547 "src/parser_bison.y"
                                                { (yyval.val) = RT2HDR_ADDR; }
#line 18184 "src/parser_bison.c"
    break;

  case 1448: /* rt4_hdr_expr: "srh" rt4_hdr_field close_scope_rt  */
#line 6551 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_rt4, (yyvsp[-1].val));
			}
#line 18192 "src/parser_bison.c"
    break;

  case 1449: /* rt4_hdr_field: "last-entry"  */
#line 6556 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_LASTENT; }
#line 18198 "src/parser_bison.c"
    break;

  case 1450: /* rt4_hdr_field: "flags"  */
#line 6557 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_FLAGS; }
#line 18204 "src/parser_bison.c"
    break;

  case 1451: /* rt4_hdr_field: "tag"  */
#line 6558 "src/parser_bison.y"
                                                { (yyval.val) = RT4HDR_TAG; }
#line 18210 "src/parser_bison.c"
    break;

  case 1452: /* rt4_hdr_field: "sid" '[' "number" ']'  */
#line 6560 "src/parser_bison.y"
                        {
				(yyval.val) = RT4HDR_SID_1 + (yyvsp[-1].val) - 1;
			}
#line 18218 "src/parser_bison.c"
    break;

  case 1453: /* frag_hdr_expr: "frag" frag_hdr_field close_scope_frag  */
#line 6566 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_frag, (yyvsp[-1].val));
			}
#line 18226 "src/parser_bison.c"
    break;

  case 1454: /* frag_hdr_field: "nexthdr"  */
#line 6571 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_NEXTHDR; }
#line 18232 "src/parser_bison.c"
    break;

  case 1455: /* frag_hdr_field: "reserved"  */
#line 6572 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED; }
#line 18238 "src/parser_bison.c"
    break;

  case 1456: /* frag_hdr_field: "frag-off"  */
#line 6573 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_FRAG_OFF; }
#line 18244 "src/parser_bison.c"
    break;

  case 1457: /* frag_hdr_field: "reserved2"  */
#line 6574 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_RESERVED2; }
#line 18250 "src/parser_bison.c"
    break;

  case 1458: /* frag_hdr_field: "more-fragments"  */
#line 6575 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_MFRAGS; }
#line 18256 "src/parser_bison.c"
    break;

  case 1459: /* frag_hdr_field: "id"  */
#line 6576 "src/parser_bison.y"
                                                { (yyval.val) = FRAGHDR_ID; }
#line 18262 "src/parser_bison.c"
    break;

  case 1460: /* dst_hdr_expr: "dst" dst_hdr_field close_scope_dst  */
#line 6580 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_dst, (yyvsp[-1].val));
			}
#line 18270 "src/parser_bison.c"
    break;

  case 1461: /* dst_hdr_field: "nexthdr"  */
#line 6585 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_NEXTHDR; }
#line 18276 "src/parser_bison.c"
    break;

  case 1462: /* dst_hdr_field: "hdrlength"  */
#line 6586 "src/parser_bison.y"
                                                { (yyval.val) = DSTHDR_HDRLENGTH; }
#line 18282 "src/parser_bison.c"
    break;

  case 1463: /* mh_hdr_expr: "mh" mh_hdr_field close_scope_mh  */
#line 6590 "src/parser_bison.y"
                        {
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), &exthdr_mh, (yyvsp[-1].val));
			}
#line 18290 "src/parser_bison.c"
    break;

  case 1464: /* mh_hdr_field: "nexthdr"  */
#line 6595 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_NEXTHDR; }
#line 18296 "src/parser_bison.c"
    break;

  case 1465: /* mh_hdr_field: "hdrlength"  */
#line 6596 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_HDRLENGTH; }
#line 18302 "src/parser_bison.c"
    break;

  case 1466: /* mh_hdr_field: "type" close_scope_type  */
#line 6597 "src/parser_bison.y"
                                                                        { (yyval.val) = MHHDR_TYPE; }
#line 18308 "src/parser_bison.c"
    break;

  case 1467: /* mh_hdr_field: "reserved"  */
#line 6598 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_RESERVED; }
#line 18314 "src/parser_bison.c"
    break;

  case 1468: /* mh_hdr_field: "checksum"  */
#line 6599 "src/parser_bison.y"
                                                { (yyval.val) = MHHDR_CHECKSUM; }
#line 18320 "src/parser_bison.c"
    break;

  case 1469: /* exthdr_exists_expr: "exthdr" exthdr_key  */
#line 6603 "src/parser_bison.y"
                        {
				const struct exthdr_desc *desc;

				desc = exthdr_find_proto((yyvsp[0].val));

				/* Assume that NEXTHDR template is always
				 * the first one in list of templates.
				 */
				(yyval.expr) = exthdr_expr_alloc(&(yyloc), desc, 1);
				(yyval.expr)->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			}
#line 18336 "src/parser_bison.c"
    break;

  case 1470: /* exthdr_key: "hbh" close_scope_hbh  */
#line 6616 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_HOPOPTS; }
#line 18342 "src/parser_bison.c"
    break;

  case 1471: /* exthdr_key: "rt" close_scope_rt  */
#line 6617 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_ROUTING; }
#line 18348 "src/parser_bison.c"
    break;

  case 1472: /* exthdr_key: "frag" close_scope_frag  */
#line 6618 "src/parser_bison.y"
                                                                { (yyval.val) = IPPROTO_FRAGMENT; }
#line 18354 "src/parser_bison.c"
    break;

  case 1473: /* exthdr_key: "dst" close_scope_dst  */
#line 6619 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_DSTOPTS; }
#line 18360 "src/parser_bison.c"
    break;

  case 1474: /* exthdr_key: "mh" close_scope_mh  */
#line 6620 "src/parser_bison.y"
                                                        { (yyval.val) = IPPROTO_MH; }
#line 18366 "src/parser_bison.c"
    break;


#line 18370 "src/parser_bison.c"

        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
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
          = {yyssp, yyesa, &yyes, &yyes_capacity, yytoken, &yylloc};
        if (yychar != YYEMPTY)
          YY_LAC_ESTABLISH;
        if (yyreport_syntax_error (&yyctx, nft, scanner, state) == 2)
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

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");

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
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);

  return yyresult;
}

#line 6623 "src/parser_bison.y"


#ifdef YY_LAC_ESTABLISH
static int
yyreport_syntax_error(const yypcontext_t *yyctx, struct nft_ctx *nft,
                      void *scanner, struct parser_state *state)
{
	const char *bad_token = yysymbol_name(yypcontext_token(yyctx));
	struct location *loc = yypcontext_location(yyctx);
	yysymbol_kind_t *exp_tokens;
	int exp_tokens_cnt;
	size_t errbufsz;
	FILE *errfp;
	char *msg;

	errfp = open_memstream(&msg, &errbufsz);
	if (!errfp)
		memory_allocation_error();

	exp_tokens_cnt = yypcontext_expected_tokens(yyctx, NULL, 0);
	exp_tokens = xmalloc_array(exp_tokens_cnt, sizeof(yysymbol_kind_t));
	yypcontext_expected_tokens(yyctx, exp_tokens, exp_tokens_cnt);

	fprintf(errfp, "syntax error, unexpected %s\nexpected any of: ", bad_token);

	for (int i = 0; i < exp_tokens_cnt; i++) {
		const char *token_name = yysymbol_name(exp_tokens[i]);
		bool is_keyword = true;

		/* tokens that name generic things shall be printed as <foo>; detect them */
		switch (exp_tokens[i]) {
		case YYSYMBOL_NUM:
		case YYSYMBOL_STRING:
		case YYSYMBOL_QUOTED_STRING:
		case YYSYMBOL_ASTERISK_STRING:
			is_keyword = false;
			break;
		default:
			break;
		}

		if (i > 0)
			fputs(", ", errfp);
		if (!is_keyword)
			fputc('<', errfp);
		fputs(token_name, errfp);
		if (!is_keyword)
			fputc('>', errfp);
	}

	free(exp_tokens);
	fclose(errfp);
	/* no newline on the end of the error message; this is intended */
	yyerror(loc, nft, scanner, state, msg);

	free(msg);
	return 0;
}

#endif /* YY_LAC_ESTABLISH */

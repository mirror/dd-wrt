/*
 * Copyright (c) 2007-2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

%{

#include <nft.h>

#include <limits.h>
#include <glob.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <sys/stat.h>

#include <nftables.h>
#include <erec.h>
#include <rule.h>
#include <parser.h>
#include "parser_bison.h"

#define YY_NO_INPUT

/*
 * Work around flex behaviour when reaching the end of buffer: normally, flex
 * regexes are greedy, when reaching the end of buffer however it tries to
 * match whatever is left in the buffer and only backs up in case it doesn't
 * match *any* pattern. Since we accept unquoted strings, this means any partial
 * token will be recognized as string.
 *
 * Make sure to only pass input to flex linewise to avoid this.
 */
#define YY_INPUT(buf,result,max_size)						\
{										\
	result = 0;								\
	errno = 0;								\
										\
	while (result < max_size) {						\
		int chr = fgetc(yyin);						\
										\
		if (chr != EOF) {						\
			buf[result++] = chr;					\
			if (chr == '\n' || chr == ' ')				\
				break;						\
			continue;						\
		}								\
										\
		if (ferror(yyin)) {						\
			if (errno != EINTR) {					\
				YY_FATAL_ERROR("input in flex scanner failed");	\
				break;						\
			}							\
			errno = 0;						\
			clearerr(yyin);						\
		}								\
		break;								\
	}									\
}

static void scanner_pop_buffer(yyscan_t scanner);


static void init_pos(struct input_descriptor *indesc)
{
	indesc->lineno		= 1;
	indesc->column		= 1;
	indesc->token_offset	= 0;
	indesc->line_offset 	= 0;
}

static void update_pos(struct parser_state *state, struct location *loc,
		       int len)
{
	loc->indesc			= state->indesc;
	loc->first_line			= state->indesc->lineno;
	loc->first_column		= state->indesc->column;
	loc->last_column		= state->indesc->column + len - 1;
	state->indesc->column		+= len;
}

static void update_offset(struct parser_state *state, struct location *loc,
			  unsigned int len)
{
	uint32_t line_offset;

	state->indesc->token_offset	+= len;
	if (state->indesc->line_offset > UINT32_MAX)
		line_offset = UINT32_MAX;
	else
		line_offset = state->indesc->line_offset;

	loc->line_offset		= line_offset;
}

static void reset_pos(struct parser_state *state, struct location *loc)
{
	state->indesc->line_offset	= state->indesc->token_offset;
	state->indesc->lineno		+= 1;
	state->indesc->column		= 1;
}

static void scanner_push_start_cond(void *scanner, enum startcond_type type);

#define YY_USER_ACTION {					\
	update_pos(yyget_extra(yyscanner), yylloc, yyleng);	\
	update_offset(yyget_extra(yyscanner), yylloc, yyleng);	\
}

/* avoid warnings with -Wmissing-prototypes */
extern int	yyget_column(yyscan_t);
extern void	yyset_column(int, yyscan_t);

%}

space		[ ]
tab		\t
newline_crlf	\r\n
newline		\n
digit		[0-9]
hexdigit	[0-9a-fA-F]
decstring	{digit}+
hexstring	0[xX]{hexdigit}+
letter		[a-zA-Z]
string		({letter}|[_.])({letter}|{digit}|[/\-_\.])*
quotedstring	\"[^"]*\"
asteriskstring	({string}\*|{string}\\\*|\\\*|{string}\\\*{string})
comment		#.*$
comment_line	^[ \t]*#.*\n
slash		\/

timestring	([0-9]+d)?([0-9]+h)?([0-9]+m)?([0-9]+s)?([0-9]+ms)?

hex4		([[:xdigit:]]{1,4})
rfc4291_broader	(((:{hex4}){2})|(:{ip4addr}))
v680		(({hex4}:){7}{hex4})
v670		((:)((:{hex4}){5}){rfc4291_broader})
v671		((({hex4}:){1})((:{hex4}){4}){rfc4291_broader})
v672		((({hex4}:){2})((:{hex4}){3}){rfc4291_broader})
v673		((({hex4}:){3})((:{hex4}){2}){rfc4291_broader})
v674		((({hex4}:){4})((:{hex4}){1}){rfc4291_broader})
v675		((({hex4}:){5}){rfc4291_broader})
v676		((({hex4}:){6})(:{hex4}{1}))
v677		((({hex4}:){7})(:))
v67		({v670}|{v671}|{v672}|{v673}|{v674}|{v675}|{v676}|{v677})
v660		((:)((:{hex4}){4}){rfc4291_broader})
v661		((({hex4}:){1})((:{hex4}){3}){rfc4291_broader})
v662		((({hex4}:){2})((:{hex4}){2}){rfc4291_broader})
v663		((({hex4}:){3})((:{hex4}){1}){rfc4291_broader})
v664		((({hex4}:){4}){rfc4291_broader})
v665		((({hex4}:){5})((:{hex4}){1}))
v666		((({hex4}:){6})(:))
v66		({v660}|{v661}|{v662}|{v663}|{v664}|{v665}|{v666})
v650		((:)((:{hex4}){3}){rfc4291_broader})
v651		((({hex4}:){1})((:{hex4}){2}){rfc4291_broader})
v652		((({hex4}:){2})((:{hex4}){1}){rfc4291_broader})
v653		((({hex4}:){3}){rfc4291_broader})
v654		((({hex4}:){4})(:{hex4}{1}))
v655		((({hex4}:){5})(:))
v65		({v650}|{v651}|{v652}|{v653}|{v654}|{v655})
v640		((:)((:{hex4}){2}){rfc4291_broader})
v641		((({hex4}:){1})((:{hex4}){1}){rfc4291_broader})
v642		((({hex4}:){2}){rfc4291_broader})
v643		((({hex4}:){3})((:{hex4}){1}))
v644		((({hex4}:){4})(:))
v64		({v640}|{v641}|{v642}|{v643}|{v644})
v630		((:)((:{hex4}){1}){rfc4291_broader})
v631		((({hex4}:){1}){rfc4291_broader})
v632		((({hex4}:){2})((:{hex4}){1}))
v633		((({hex4}:){3})(:))
v63		({v630}|{v631}|{v632}|{v633})
v620		((:){rfc4291_broader})
v621		((({hex4}:){1})((:{hex4}){1}))
v622		((({hex4}:){2})(:))
v62		({v620}|{v621}|{v622})
v610		((:)(:{hex4}{1}))
v611		((({hex4}:){1})(:))
v61		({v610}|{v611})
v60		(::)

macaddr		(([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2})
ip4addr		(([[:digit:]]{1,3}"."){3}([[:digit:]]{1,3}))
ip6addr		({v680}|{v67}|{v66}|{v65}|{v64}|{v63}|{v62}|{v61}|{v60})
ip6addr_rfc2732	(\[{ip6addr}\])

classid		({hexdigit}{1,4}:{hexdigit}{1,4})
addrstring	({macaddr}|{ip4addr}|{ip6addr})

%option prefix="nft_"
%option outfile="lex.yy.c"
%option reentrant
%option noyywrap
%option nounput
%option bison-bridge
%option bison-locations
%option debug
%option yylineno
%option nodefault
%option warn
%option stack
%s SCANSTATE_ARP
%s SCANSTATE_AT
%s SCANSTATE_CT
%s SCANSTATE_COUNTER
%s SCANSTATE_ETH
%s SCANSTATE_GRE
%s SCANSTATE_ICMP
%s SCANSTATE_IGMP
%s SCANSTATE_IP
%s SCANSTATE_IP6
%s SCANSTATE_LAST
%s SCANSTATE_LIMIT
%s SCANSTATE_META
%s SCANSTATE_POLICY
%s SCANSTATE_QUOTA
%s SCANSTATE_SCTP
%s SCANSTATE_SECMARK
%s SCANSTATE_TCP
%s SCANSTATE_TYPE
%s SCANSTATE_TUNNEL
%s SCANSTATE_VLAN
%s SCANSTATE_XT
%s SCANSTATE_CMD_DESTROY
%s SCANSTATE_CMD_EXPORT
%s SCANSTATE_CMD_IMPORT
%s SCANSTATE_CMD_LIST
%s SCANSTATE_CMD_MONITOR
%s SCANSTATE_CMD_RESET
%s SCANSTATE_EXPR_AH
%s SCANSTATE_EXPR_COMP
%s SCANSTATE_EXPR_DCCP
%s SCANSTATE_EXPR_DST
%s SCANSTATE_EXPR_ESP
%s SCANSTATE_EXPR_FIB
%s SCANSTATE_EXPR_FRAG
%s SCANSTATE_EXPR_HASH
%s SCANSTATE_EXPR_HBH
%s SCANSTATE_EXPR_IPSEC
%s SCANSTATE_EXPR_MH
%s SCANSTATE_EXPR_NUMGEN
%s SCANSTATE_EXPR_OSF
%s SCANSTATE_EXPR_QUEUE
%s SCANSTATE_EXPR_RT
%s SCANSTATE_EXPR_SCTP_CHUNK
%s SCANSTATE_EXPR_SOCKET
%s SCANSTATE_EXPR_TH
%s SCANSTATE_EXPR_UDP
%s SCANSTATE_EXPR_UDPLITE

%s SCANSTATE_STMT_DUP
%s SCANSTATE_STMT_FWD
%s SCANSTATE_STMT_LOG
%s SCANSTATE_STMT_NAT
%s SCANSTATE_STMT_REJECT
%s SCANSTATE_STMT_SYNPROXY
%s SCANSTATE_STMT_TPROXY

%%

"=="			{ return EQ; }
"eq"			{ return EQ; }
"!="			{ return NEQ; }
"ne"			{ return NEQ; }
"<="			{ return LTE; }
"le"			{ return LTE; }
"<"			{ return LT; }
"lt"			{ return LT; }
">="			{ return GTE; }
"ge"			{ return GTE; }
">"			{ return GT; }
"gt"			{ return GT; }
","			{ return COMMA; }
"."			{ return DOT; }
":"			{ return COLON; }
";"			{ return SEMICOLON; }
"{"			{ return '{'; }
"}"			{ return '}'; }
"["			{ return '['; }
"]"			{ return ']'; }
"("			{ return '('; }
")"			{ return ')'; }
"<<"			{ return LSHIFT; }
"lshift"		{ return LSHIFT; }
">>"			{ return RSHIFT; }
"rshift"		{ return RSHIFT; }
"^"			{ return CARET; }
"xor"			{ return CARET; }
"&"			{ return AMPERSAND; }
"and"			{ return AMPERSAND; }
"|"			{ return '|'; }
"or"			{ return '|'; }
"!"			{ return NOT; }
"not"			{ return NOT; }
"/"			{ return SLASH; }
"-"			{ return DASH; }
"*"			{ return ASTERISK; }
"@"			{ scanner_push_start_cond(yyscanner, SCANSTATE_AT); return AT; }
"$"			{ return '$'; }
"="			{ return '='; }
"vmap"			{ return VMAP; }

"+"			{ return PLUS; }

"include"		{ return INCLUDE; }
"define"		{ return DEFINE; }
"redefine"		{ return REDEFINE; }
"undefine"		{ return UNDEFINE; }

"describe"		{ return DESCRIBE; }

<SCANSTATE_CMD_LIST,SCANSTATE_CMD_MONITOR>{
	"chains"		{ return CHAINS; }
	"sets"			{ return SETS; }
	"tables"		{ return TABLES; }
}
<SCANSTATE_CMD_MONITOR>{
	"rules"			{ return RULES; }
	"trace"			{ return TRACE; }
}
"hook"			{ return HOOK; }
"device"		{ return DEVICE; }
"devices"		{ return DEVICES; }
"table"			{ return TABLE; }
"chain"			{ return CHAIN; }
"rule"			{ return RULE; }
"set"			{ return SET; }
"element"		{ return ELEMENT; }
"map"			{ return MAP; }
"flowtable"		{ return FLOWTABLE; }
"handle"		{ return HANDLE; }
"ruleset"		{ return RULESET; }

"socket"		{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_SOCKET); return SOCKET; }
<SCANSTATE_EXPR_SOCKET>{
	"transparent"		{ return TRANSPARENT; }
	"wildcard"		{ return WILDCARD; }
	"cgroupv2"		{ return CGROUPV2; }
	"level"			{ return LEVEL; }
}
"tproxy"		{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_TPROXY); return TPROXY; }

"accept"		{ return ACCEPT; }
"drop"			{ return DROP; }
"continue"		{ return CONTINUE; }
"jump"			{ return JUMP; }
"goto"			{ return GOTO; }
"return"		{ return RETURN; }
<SCANSTATE_EXPR_QUEUE,SCANSTATE_STMT_DUP,SCANSTATE_STMT_FWD,SCANSTATE_STMT_NAT,SCANSTATE_STMT_TPROXY,SCANSTATE_IP,SCANSTATE_IP6>"to"			{ return TO; } /* XXX: SCANSTATE_IP is a workaround */

"inet"			{ return INET; }
"netdev"		{ return NETDEV; }

"add"			{ return ADD; }
"replace"		{ return REPLACE; }
"update"		{ return UPDATE; }
"create"		{ return CREATE; }
"insert"		{ return INSERT; }
"delete"		{ return DELETE; }
"get"			{ return GET; }
"list"			{ scanner_push_start_cond(yyscanner, SCANSTATE_CMD_LIST); return LIST; }
"reset"			{ scanner_push_start_cond(yyscanner, SCANSTATE_CMD_RESET); return RESET; }
"flush"			{ return FLUSH; }
"rename"		{ return RENAME; }
"import"                { scanner_push_start_cond(yyscanner, SCANSTATE_CMD_IMPORT); return IMPORT; }
"export"		{ scanner_push_start_cond(yyscanner, SCANSTATE_CMD_EXPORT); return EXPORT; }
"monitor"		{ scanner_push_start_cond(yyscanner, SCANSTATE_CMD_MONITOR); return MONITOR; }
"destroy"		{ scanner_push_start_cond(yyscanner, SCANSTATE_CMD_DESTROY); return DESTROY; }


"position"		{ return POSITION; }
"index"			{ return INDEX; }
"comment"		{ return COMMENT; }

"constant"		{ return CONSTANT; }
"interval"		{ return INTERVAL; }
"dynamic"		{ return DYNAMIC; }
"auto-merge"		{ return AUTOMERGE; }
"timeout"		{ return TIMEOUT; }
"gc-interval"		{ return GC_INTERVAL; }
"elements"		{ return ELEMENTS; }
"expires"		{ return EXPIRES; }

"policy"		{ scanner_push_start_cond(yyscanner, SCANSTATE_POLICY); return POLICY; }
"size"			{ return SIZE; }
<SCANSTATE_POLICY>{
	"performance"		{ return PERFORMANCE; }
	"memory"		{ return MEMORY; }
}

"flow"			{ return FLOW; }
"offload"		{ return OFFLOAD; }
"meter"			{ return METER; }

<SCANSTATE_CMD_LIST>{
	"meters"		{ return METERS; }
	"flowtables"		{ return FLOWTABLES; }
	"limits"		{ return LIMITS; }
	"maps"			{ return MAPS; }
	"secmarks"		{ return SECMARKS; }
	"synproxys"		{ return SYNPROXYS; }
	"tunnels"		{ return TUNNELS; }
	"hooks"			{ return HOOKS; }
}

"counter"		{ scanner_push_start_cond(yyscanner, SCANSTATE_COUNTER); return COUNTER; }
<SCANSTATE_COUNTER,SCANSTATE_LIMIT,SCANSTATE_QUOTA,SCANSTATE_STMT_SYNPROXY,SCANSTATE_EXPR_OSF,SCANSTATE_TUNNEL>"name"			{ return NAME; }
<SCANSTATE_COUNTER,SCANSTATE_CT,SCANSTATE_LIMIT>"packets"		{ return PACKETS; }
<SCANSTATE_COUNTER,SCANSTATE_CT,SCANSTATE_LIMIT,SCANSTATE_QUOTA>"bytes"	{ return BYTES; }

"last"				{ scanner_push_start_cond(yyscanner, SCANSTATE_LAST); return LAST; }
<SCANSTATE_LAST>{
	"never"			{ return NEVER; }
}

<SCANSTATE_CMD_LIST,SCANSTATE_CMD_RESET>{
	"counters"		{ return COUNTERS; }
	"quotas"		{ return QUOTAS; }
	"rules"			{ return RULES; }
}

"log"			{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_LOG); return LOG; }
<SCANSTATE_STMT_LOG,SCANSTATE_STMT_NAT,SCANSTATE_IP,SCANSTATE_IP6>"prefix"		{ return PREFIX; }
<SCANSTATE_STMT_LOG>{
	"snaplen"		{ return SNAPLEN; }
	"queue-threshold"	{ return QUEUE_THRESHOLD; }
	"level"			{ return LEVEL; }
	"group"			{ return GROUP; }
}

"queue"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_QUEUE); return QUEUE;}
<SCANSTATE_EXPR_QUEUE>{
	"num"		{ return QUEUENUM;}
	"bypass"	{ return BYPASS;}
	"fanout"	{ return FANOUT;}
}
"limit"			{ scanner_push_start_cond(yyscanner, SCANSTATE_LIMIT); return LIMIT; }
<SCANSTATE_LIMIT>{
	"rate"			{ return RATE; }
	"burst"			{ return BURST; }

	/* time_unit */
	"second"		{ return SECOND; }
	"minute"		{ return MINUTE; }
	"week"			{ return WEEK; }
}
<SCANSTATE_CT,SCANSTATE_LIMIT,SCANSTATE_QUOTA>"over"		{ return OVER; }

"quota"			{ scanner_push_start_cond(yyscanner, SCANSTATE_QUOTA); return QUOTA; }
<SCANSTATE_QUOTA>{
	"until"		{ return UNTIL; }
}

<SCANSTATE_QUOTA,SCANSTATE_LAST>"used"		{ return USED; }

"hour"			{ return HOUR; }
"day"			{ return DAY; }

"reject"		{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_REJECT); return _REJECT; }
<SCANSTATE_STMT_REJECT>{
	"with"			{ return WITH; }
	"icmpx"			{ return ICMPX; }
}

"snat"			{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_NAT); return SNAT; }
"dnat"			{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_NAT); return DNAT; }
"masquerade"		{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_NAT); return MASQUERADE; }
"redirect"		{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_NAT); return REDIRECT; }
"random"		{ return RANDOM; }
<SCANSTATE_STMT_NAT>{
	"fully-random"		{ return FULLY_RANDOM; }
	"persistent"		{ return PERSISTENT; }
	"port"			{ return PORT; }
}

<SCANSTATE_AT>{
	"ll"			{ return LL_HDR; }
	"nh"			{ return NETWORK_HDR; }
}
"th"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_TH); return TRANSPORT_HDR; }

"bridge"		{ return BRIDGE; }

"ether"			{ scanner_push_start_cond(yyscanner, SCANSTATE_ETH); return ETHER; }
<SCANSTATE_ARP,SCANSTATE_CT,SCANSTATE_ETH,SCANSTATE_IP,SCANSTATE_IP6,SCANSTATE_EXPR_FIB,SCANSTATE_EXPR_IPSEC>{
	"saddr"			{ return SADDR; }
	"daddr"			{ return DADDR; }
}
"type"			{ scanner_push_start_cond(yyscanner, SCANSTATE_TYPE); return TYPE; }
"typeof"		{ return TYPEOF; }

"vlan"			{ scanner_push_start_cond(yyscanner, SCANSTATE_VLAN); return VLAN; }
<SCANSTATE_CT,SCANSTATE_EXPR_FRAG,SCANSTATE_VLAN,SCANSTATE_IP,SCANSTATE_ICMP>"id"			{ return ID; }
<SCANSTATE_VLAN>{
	"cfi"		{ return CFI; }
	"dei"		{ return DEI; }
	"pcp"		{ return PCP; }
}
"8021ad"		{ yylval->string = xstrdup(yytext); return STRING; }
"8021q"			{ yylval->string = xstrdup(yytext); return STRING; }

"arp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_ARP); return ARP; }
<SCANSTATE_ARP>{
	"htype"			{ return HTYPE; }
	"ptype"			{ return PTYPE; }
	"hlen"			{ return HLEN; }
	"plen"			{ return PLEN; }
	"operation"		{ return OPERATION; }
}

"ip"			{ scanner_push_start_cond(yyscanner, SCANSTATE_IP); return IP; }
<SCANSTATE_IP,SCANSTATE_IP6,SCANSTATE_EXPR_OSF,SCANSTATE_GRE>{
	"version"		{ return HDRVERSION; }
}
<SCANSTATE_EXPR_AH,SCANSTATE_EXPR_DST,SCANSTATE_EXPR_HBH,SCANSTATE_EXPR_MH,SCANSTATE_EXPR_RT,SCANSTATE_IP>{
	"hdrlength"		{ return HDRLENGTH; }
}
<SCANSTATE_IP,SCANSTATE_IP6,SCANSTATE_TYPE>{
	"dscp"			{ return DSCP; }
}
"ecn"			{ return ECN; }
<SCANSTATE_EXPR_UDP,SCANSTATE_IP,SCANSTATE_IP6,SCANSTATE_META,SCANSTATE_TCP,SCANSTATE_SCTP,SCANSTATE_EXPR_SCTP_CHUNK>"length"		{ return LENGTH; }
<SCANSTATE_EXPR_FRAG,SCANSTATE_IP>{
	"frag-off"		{ return FRAG_OFF; }
}
<SCANSTATE_EXPR_OSF,SCANSTATE_IP>{
	"ttl"			{ return TTL; }
}
<SCANSTATE_CT,SCANSTATE_IP,SCANSTATE_META,SCANSTATE_TYPE,SCANSTATE_GRE>"protocol"		{ return PROTOCOL; }
<SCANSTATE_EXPR_MH,SCANSTATE_EXPR_UDP,SCANSTATE_EXPR_UDPLITE,SCANSTATE_ICMP,SCANSTATE_IGMP,SCANSTATE_IP,SCANSTATE_SCTP,SCANSTATE_TCP>{
	"checksum"		{ return CHECKSUM; }
}

<SCANSTATE_IP>{
	"lsrr"			{ return LSRR; }
	"rr"			{ return RR; }
	"ssrr"			{ return SSRR; }
	"ra"			{ return RA; }

	"ptr"			{ return PTR; }
	"value"			{ return VALUE; }

	"option"		{ return OPTION; }
	"options"		{ return OPTIONS; }
}

<SCANSTATE_TCP>{
	/* tcp header fields */
	"ackseq"		{ return ACKSEQ; }
	"doff"			{ return DOFF; }
	"window"		{ return WINDOW; }
	"urgptr"		{ return URGPTR; }

	/* tcp option types */
	"echo"			{ return ECHO; }
	"eol"			{ return EOL; }
	"maxseg"		{ return MSS; }
	"mss"			{ return MSS; }
	"nop"			{ return NOP; }
	"noop"			{ return NOP; }
	"sack"			{ return SACK; }
	"sack0"			{ return SACK0; }
	"sack1"			{ return SACK1; }
	"sack2"			{ return SACK2; }
	"sack3"			{ return SACK3; }
	"sack-permitted"	{ return SACK_PERM; }
	"sack-perm"		{ return SACK_PERM; }
	"timestamp"		{ return TIMESTAMP; }
	"fastopen"		{ return FASTOPEN; }
	"mptcp"			{ return MPTCP; }
	"md5sig"		{ return MD5SIG; }

	/* tcp option fields */
	"left"			{ return LEFT; }
	"right"			{ return RIGHT; }
	"count"			{ return COUNT; }
	"tsval"			{ return TSVAL; }
	"tsecr"			{ return TSECR; }
	"subtype"		{ return SUBTYPE; }

	"options"		{ return OPTIONS; }
	"option"		{ return OPTION; }
}
"time"			{ return TIME; }

"icmp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_ICMP); return ICMP; }
"icmpv6"		{ scanner_push_start_cond(yyscanner, SCANSTATE_ICMP); return ICMP6; }
<SCANSTATE_ICMP>{
	"gateway"		{ return GATEWAY; }
	"code"			{ return CODE; }
	"param-problem"		{ return PPTR; }
	"max-delay"		{ return MAXDELAY; }
	"mtu"			{ return MTU; }
	"taddr"			{ return TADDR; }
	"daddr"			{ return DADDR; }
}
<SCANSTATE_EXPR_AH,SCANSTATE_EXPR_ESP,SCANSTATE_ICMP,SCANSTATE_TCP>{
	"sequence"		{ return SEQUENCE; }
}

"igmp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_IGMP); return IGMP; }
<SCANSTATE_IGMP>{
	"mrt"			{ return MRT; }
	"group"			{ return GROUP; }
}

"ip6"			{ scanner_push_start_cond(yyscanner, SCANSTATE_IP6); return IP6; }
"priority"		{ return PRIORITY; }
<SCANSTATE_IP6>{
	"flowlabel"		{ return FLOWLABEL; }
	"hoplimit"		{ return HOPLIMIT; }
}
<SCANSTATE_EXPR_AH,SCANSTATE_EXPR_COMP,SCANSTATE_EXPR_DST,SCANSTATE_EXPR_FRAG,SCANSTATE_EXPR_HBH,SCANSTATE_EXPR_MH,SCANSTATE_EXPR_RT,SCANSTATE_IP6>{
	"nexthdr"		{ return NEXTHDR; }
}

"ah"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_AH); return AH; }
<SCANSTATE_EXPR_AH,SCANSTATE_EXPR_FRAG,SCANSTATE_EXPR_MH,SCANSTATE_TCP>{
	"reserved"		{ return RESERVED; }
}
<SCANSTATE_EXPR_AH,SCANSTATE_EXPR_ESP,SCANSTATE_EXPR_IPSEC>"spi"			{ return SPI; }

"esp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_ESP); return ESP; }

"comp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_COMP); return COMP; }
<SCANSTATE_EXPR_COMP>{
	"cpi"			{ return CPI; }
}
"flags"			{ return FLAGS; }

"udp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_UDP); return UDP; }
"udplite"		{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_UDPLITE); return UDPLITE; }
<SCANSTATE_EXPR_UDPLITE>{
	"csumcov"	{ return CSUMCOV; }
}
<SCANSTATE_EXPR_DCCP,SCANSTATE_SCTP,SCANSTATE_TCP,SCANSTATE_EXPR_TH,SCANSTATE_EXPR_UDP,SCANSTATE_EXPR_UDPLITE>{
	"sport"			{ return SPORT; }
}
<SCANSTATE_CT,SCANSTATE_EXPR_DCCP,SCANSTATE_SCTP,SCANSTATE_TCP,SCANSTATE_EXPR_TH,SCANSTATE_EXPR_UDP,SCANSTATE_EXPR_UDPLITE>{
	"dport"			{ return DPORT; }
}
<SCANSTATE_EXPR_DCCP>{
	"option"		{ return OPTION; }
}

"vxlan"			{ return VXLAN; }
"vni"			{ return VNI; }

"geneve"		{ return GENEVE; }

"gre"			{ scanner_push_start_cond(yyscanner, SCANSTATE_GRE); return GRE; }
"gretap"		{ scanner_push_start_cond(yyscanner, SCANSTATE_GRE); return GRETAP; }

"tcp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_TCP); return TCP; }

"dccp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_DCCP); return DCCP; }

"sctp"			{ scanner_push_start_cond(yyscanner, SCANSTATE_SCTP); return SCTP; }

<SCANSTATE_SCTP>{
	"chunk"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_SCTP_CHUNK); return CHUNK; }
	"vtag"			{ return VTAG; }
}

<SCANSTATE_EXPR_SCTP_CHUNK>{
	"data"			{ return DATA; }
	"init"			{ return INIT; }
	"init-ack"		{ return INIT_ACK; }
	"heartbeat"		{ return HEARTBEAT; }
	"heartbeat-ack"		{ return HEARTBEAT_ACK; }
	"abort"			{ return ABORT; }
	"shutdown"		{ return SHUTDOWN; }
	"shutdown-ack"		{ return SHUTDOWN_ACK; }
	"error"			{ return ERROR; }
	"cookie-echo"		{ return COOKIE_ECHO; }
	"cookie-ack"		{ return COOKIE_ACK; }
	"ecne"			{ return ECNE; }
	"cwr"			{ return CWR; }
	"shutdown-complete"	{ return SHUTDOWN_COMPLETE; }
	"asconf-ack"		{ return ASCONF_ACK; }
	"forward-tsn"		{ return FORWARD_TSN; }
	"asconf"		{ return ASCONF; }

	"tsn"			{ return TSN; }
	"sack"			{ return SACK; }
	"stream"		{ return STREAM; }
	"ssn"			{ return SSN; }
	"ppid"			{ return PPID; }
	"init-tag"		{ return INIT_TAG; }
	"a-rwnd"		{ return A_RWND; }
	"num-outbound-streams"	{ return NUM_OSTREAMS; }
	"num-inbound-streams"	{ return NUM_ISTREAMS; }
	"initial-tsn"		{ return INIT_TSN; }
	"cum-tsn-ack"		{ return CUM_TSN_ACK; }
	"num-gap-ack-blocks"	{ return NUM_GACK_BLOCKS; }
	"num-dup-tsns"		{ return NUM_DUP_TSNS; }
	"lowest-tsn"		{ return LOWEST_TSN; }
	"seqno"			{ return SEQNO; }
	"new-cum-tsn"		{ return NEW_CUM_TSN; }
}

"rt"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_RT); return RT; }
"rt0"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_RT); return RT0; }
"rt2"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_RT); return RT2; }
"srh"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_RT); return RT4; }
<SCANSTATE_EXPR_RT,SCANSTATE_STMT_NAT,SCANSTATE_IP,SCANSTATE_IP6>"addr"			{ return ADDR; }

"hbh"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_HBH); return HBH; }

"frag"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_FRAG); return FRAG; }
<SCANSTATE_EXPR_FRAG>{
	"reserved2"		{ return RESERVED2; }
	"more-fragments"	{ return MORE_FRAGMENTS; }
}

"dst"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_DST); return DST; }

"mh"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_MH); return MH; }

"meta"			{ scanner_push_start_cond(yyscanner, SCANSTATE_META); return META; }
"mark"			{ return MARK; }
"iif"			{ return IIF; }
"iifname"		{ return IIFNAME; }
"iiftype"		{ return IIFTYPE; }
"oif"			{ return OIF; }
"oifname"		{ return OIFNAME; }
"oiftype"		{ return OIFTYPE; }
"skuid"			{ return SKUID; }
"skgid"			{ return SKGID; }
"nftrace"		{ return NFTRACE; }
"rtclassid"		{ return RTCLASSID; }
"ibriport"		{ return IBRIPORT; }
"ibrname"		{ return IBRIDGENAME; }
"obriport"		{ return OBRIPORT; }
"obrname"		{ return OBRIDGENAME; }
"pkttype"		{ return PKTTYPE; }
"cpu"			{ return CPU; }
"iifgroup"		{ return IIFGROUP; }
"oifgroup"		{ return OIFGROUP; }
"cgroup"		{ return CGROUP; }

<SCANSTATE_EXPR_RT>{
	"nexthop"		{ return NEXTHOP; }
	"seg-left"		{ return SEG_LEFT; }
	"mtu"			{ return MTU; }
	"last-entry"		{ return LAST_ENT; }
	"tag"			{ return TAG; }
	"sid"			{ return SID; }
}
<SCANSTATE_EXPR_RT,SCANSTATE_TYPE>{
	"classid"		{ return CLASSID; }
}

"ct"			{ scanner_push_start_cond(yyscanner, SCANSTATE_CT); return CT; }
<SCANSTATE_CT>{
	"avgpkt"		{ return AVGPKT; }
	"l3proto"		{ return L3PROTOCOL; }
	"proto-src"		{ return PROTO_SRC; }
	"proto-dst"		{ return PROTO_DST; }
	"zone"			{ return ZONE; }
	"original"		{ return ORIGINAL; }
	"reply"			{ return REPLY; }
	"direction"		{ return DIRECTION; }
	"event"			{ return EVENT; }
	"expectation"		{ return EXPECTATION; }
	"expiration"		{ return EXPIRATION; }
	"helper"		{ return HELPER; }
	"helpers"		{ return HELPERS; }
	"label"			{ return LABEL; }
	"state"			{ return STATE; }
	"status"		{ return STATUS; }
	"count"			{ return COUNT; }
}

"numgen"		{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_NUMGEN); return NUMGEN; }
<SCANSTATE_EXPR_NUMGEN>{
	"inc"		{ return INC; }
}

"jhash"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_HASH); return JHASH; }
"symhash"		{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_HASH); return SYMHASH; }

<SCANSTATE_EXPR_HASH>{
	"seed"		{ return SEED; }
}
<SCANSTATE_EXPR_HASH,SCANSTATE_EXPR_NUMGEN>{
	"mod"		{ return MOD; }
	"offset"	{ return OFFSET; }
}
"dup"			{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_DUP); return DUP; }
"fwd"			{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_FWD); return FWD; }

"fib"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_FIB); return FIB; }

<SCANSTATE_EXPR_FIB>{
	"check"		{ return CHECK; }
}

"osf"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_OSF); return OSF; }

"synproxy"		{ scanner_push_start_cond(yyscanner, SCANSTATE_STMT_SYNPROXY); return SYNPROXY; }
<SCANSTATE_STMT_SYNPROXY>{
	"wscale"		{ return WSCALE; }
	"maxseg"		{ return MSS; }
	"mss"			{ return MSS; }
	"timestamp"		{ return TIMESTAMP; }
	"sack-permitted"	{ return SACK_PERM; }
	"sack-perm"		{ return SACK_PERM; }
}

"tunnel"		{ scanner_push_start_cond(yyscanner, SCANSTATE_TUNNEL); return TUNNEL; }
<SCANSTATE_TUNNEL>{
	"id"			{ return ID; }
	"sport"			{ return SPORT; }
	"dport"			{ return DPORT; }
	"ttl"			{ return TTL; }
	"tos"			{ return TOS; }
	"version"		{ return HDRVERSION; }
	"direction"		{ return DIRECTION; }
	"erspan"		{ return ERSPAN; }
	"egress"		{ return EGRESS; }
	"ingress"		{ return INGRESS; }
	"path"			{ return PATH; }
	"gbp"			{ return GBP; }
	"class"			{ return CLASS; }
	"opt-type"		{ return OPTTYPE; }
	"data"			{ return DATA; }
}

"notrack"		{ return NOTRACK; }

"all"			{ return ALL; }

<SCANSTATE_CMD_EXPORT,SCANSTATE_CMD_IMPORT,SCANSTATE_CMD_MONITOR>{
	"xml"			{ return XML; }
	"json"			{ return JSON; }
	"vm"                    { return VM; }
}

"exists"		{ return EXISTS; }
"missing"		{ return MISSING; }

"exthdr"		{ return EXTHDR; }

"ipsec"			{ scanner_push_start_cond(yyscanner, SCANSTATE_EXPR_IPSEC); return IPSEC; }
<SCANSTATE_EXPR_IPSEC>{
	"reqid"			{ return REQID; }
	"spnum"			{ return SPNUM; }

	"in"			{ return IN; }
	"out"			{ return OUT; }
}

"secmark"		{ scanner_push_start_cond(yyscanner, SCANSTATE_SECMARK); return SECMARK; }

"xt"			{ scanner_push_start_cond(yyscanner, SCANSTATE_XT); return XT; }

{addrstring}		{
				yylval->string = xstrdup(yytext);
				return STRING;
			}

{ip6addr_rfc2732}	{
				yytext[yyleng - 1] = '\0';
				yylval->string = xstrdup(yytext + 1);
				return STRING;
			}

{timestring}		{
				yylval->string = xstrdup(yytext);
				return STRING;
			}

{hexstring}		{
				errno = 0;
				yylval->val = strtoull(yytext, NULL, 16);
				if (errno != 0) {
					yylval->string = xstrdup(yytext);
					return STRING;
				}
				return NUM;
			}

{decstring}		{
				int base = yytext[0] == '0' ? 8 : 10;
				char *end;

				errno = 0;
				yylval->val = strtoull(yytext, &end, base);
				if (errno != 0 || *end) {
					yylval->string = xstrdup(yytext);
					return STRING;
				}
				return NUM;
			}

{classid}/[ \t\n:\-},]	{
				yylval->string = xstrdup(yytext);
				return STRING;
			}

{quotedstring}		{
				yytext[yyleng - 1] = '\0';
				yylval->string = xstrdup(yytext + 1);
				return QUOTED_STRING;
			}

{asteriskstring}	{
				yylval->string = xstrdup(yytext);
				return ASTERISK_STRING;
			}

{string}		{
				yylval->string = xstrdup(yytext);
				return STRING;
			}

{newline_crlf}		{	return CRLF; }

\\{newline}		{
				reset_pos(yyget_extra(yyscanner), yylloc);
			}

{newline}		{
				reset_pos(yyget_extra(yyscanner), yylloc);
				return NEWLINE;
			}

{tab}+
{space}+
{comment_line}		{
				reset_pos(yyget_extra(yyscanner), yylloc);
			}
{comment}

<<EOF>> 		{
				update_pos(yyget_extra(yyscanner), yylloc, 1);
				scanner_pop_buffer(yyscanner);
				if (YY_CURRENT_BUFFER == NULL)
					return TOKEN_EOF;
			}

.			{ return JUNK; }

%%

static void scanner_push_indesc(struct parser_state *state,
				struct input_descriptor *indesc)
{
	if (!state->indesc)
		list_add_tail(&indesc->list, &state->indesc_list);
	else
		list_add(&indesc->list, &state->indesc->list);

	state->indesc = indesc;
}

static void scanner_pop_indesc(struct parser_state *state)
{
	if (!list_is_first(&state->indesc->list, &state->indesc_list)) {
		state->indesc = list_entry(state->indesc->list.prev,
					   struct input_descriptor, list);
	} else {
		state->indesc = NULL;
	}
}

static void scanner_pop_buffer(yyscan_t scanner)
{
	struct parser_state *state = yyget_extra(scanner);

	yypop_buffer_state(scanner);
	scanner_pop_indesc(state);
}

static void scanner_push_file(struct nft_ctx *nft, void *scanner,
			      FILE *f, const char *filename,
			      const struct location *loc,
			      const struct input_descriptor *parent_indesc)
{
	struct parser_state *state = yyget_extra(scanner);
	struct input_descriptor *indesc;
	YY_BUFFER_STATE b;

	b = yy_create_buffer(f, YY_BUF_SIZE, scanner);
	yypush_buffer_state(b, scanner);

	indesc = xzalloc(sizeof(struct input_descriptor));

	if (loc != NULL)
		indesc->location = *loc;
	indesc->type	= INDESC_FILE;
	indesc->name	= xstrdup(filename);
	indesc->f	= f;
	if (!parent_indesc) {
		indesc->depth = 1;
	} else {
		indesc->depth = parent_indesc->depth + 1;
	}
	init_pos(indesc);

	scanner_push_indesc(state, indesc);
}

enum nft_include_type {
	NFT_INCLUDE,
	NFT_CMDLINE,
};

static bool __is_useable(unsigned int type, enum nft_include_type t)
{
	type &= S_IFMT;
	switch (type) {
	case S_IFREG: return true;
	case S_IFIFO:
		 return t == NFT_CMDLINE; /* disallow include /path/to/fifo */
	default:
		break;
	}

	return false;
}

/* need to use stat() to, fopen() will block for named fifos */
static bool filename_is_useable(const char *name)
{
	struct stat sb;
	int err;

	err = stat(name, &sb);
	if (err)
		return false;

	return __is_useable(sb.st_mode, NFT_INCLUDE);
}

static bool fp_is_useable(FILE *fp, enum nft_include_type t)
{
	int fd = fileno(fp);
	struct stat sb;
	int err;

	if (fd < 0)
		return false;

	err = fstat(fd, &sb);
	if (err < 0)
		return false;

	return __is_useable(sb.st_mode, t);
}

static int include_file(struct nft_ctx *nft, void *scanner,
			const char *filename, const struct location *loc,
			const struct input_descriptor *parent_indesc,
			enum nft_include_type includetype)

{
	struct parser_state *state = yyget_extra(scanner);
	struct error_record *erec;
	FILE *f;

	if (parent_indesc && parent_indesc->depth == MAX_INCLUDE_DEPTH) {
		erec = error(loc, "Include nested too deeply, max %u levels",
			     MAX_INCLUDE_DEPTH);
		goto err;
	}

	if (includetype == NFT_INCLUDE && !filename_is_useable(filename)) {
		erec = error(loc, "Not a regular file: \"%s\"\n", filename);
		goto err;
	}

	f = fopen(filename, "r");
	if (f == NULL) {
		erec = error(loc, "Could not open file \"%s\": %s\n",
			     filename, strerror(errno));
		goto err;
	}

	if (!fp_is_useable(f, includetype)) {
		fclose(f);
		erec = error(loc, "Not a regular file: \"%s\"\n", filename);
		goto err;
	}

	scanner_push_file(nft, scanner, f, filename, loc, parent_indesc);
	return 0;
err:
	erec_queue(erec, state->msgs);
	return -1;
}

static int include_glob(struct nft_ctx *nft, void *scanner, const char *pattern,
			const struct location *loc)
{
	struct parser_state *state = yyget_extra(scanner);
	struct input_descriptor *indesc = state->indesc;
	struct error_record *erec = NULL;
	bool wildcard = false;
	glob_t glob_data;
	unsigned int i;
	int flags = 0;
	int ret;
	char *p;

	/* This function can return four meaningful values:
	 *
	 *  -1 means that there was an error.
	 *   0 means that a single non-wildcard match was done.
	 *   1 means that there are no wildcards in the pattern and the
	 *     search can continue.
	 *   2 means that there are wildcards in the pattern and the search
	 *     can continue.
	 *
	 * The diffrence is needed, because there is a semantic difference
	 * between patterns with wildcards and no wildcards. Not finding a
	 * non-wildcard file is an error but not finding any matches for a
	 * wildcard pattern is not.
	 */

	/* There shouldn't be a need to use escape characters in include paths.
	 */
	flags |= GLOB_NOESCAPE;

	/* Mark directories so we can filter them out (also links). */
	flags |= GLOB_MARK;

	/* If there is no match, glob() doesn't set GLOB_MAGCHAR even if there
	 * are wildcard characters in the pattern. We need to look for (luckily
	 * well-known and not likely to change) magic characters ourselves. In a
	 * perfect world, we could use glob() itself to figure out if there are
	 * wildcards in the pattern.
	 */
	p = (char *)pattern;
	while (*p) {
		if (*p == '*' || *p == '?' || *p == '[') {
			wildcard = true;
			break;
		}
		p++;
	}

	ret = glob(pattern, flags, NULL, &glob_data);
	if (ret == 0) {
		char *path;
		size_t len;

		/* reverse alphabetical order due to stack */
		for (i = glob_data.gl_pathc; i > 0; i--) {

			path = glob_data.gl_pathv[i-1];

			/* ignore directories */
			len = strlen(path);
			if (len == 0 || path[len - 1] == '/')
				continue;

			ret = include_file(nft, scanner, path, loc, indesc, NFT_INCLUDE);
			if (ret != 0)
				goto err;
		}

		globfree(&glob_data);

		/* If no wildcards and we found the file, stop the search (with
		 * 0). In case of wildcards we need to still continue the
		 * search, because other matches might be in other include
		 * directories. We handled the case with a non-wildcard pattern
		 * and no matches already before.
		 */
		 return wildcard ? 2 : 0;
	} else if (ret == GLOB_NOMATCH) {
		globfree(&glob_data);

		/* We need to tell the caller whether wildcards were used in
		 * case of no match, because the semantics for handling the
		 * cases are different.
		 */
		return wildcard ? 2 : 1;
	}

	erec = error(loc, "Failed to glob the pattern %s", pattern);

	/* intentional fall through */
err:
	if (erec)
		erec_queue(erec, state->msgs);
	globfree(&glob_data);
	return -1;
}

int scanner_read_file(struct nft_ctx *nft, const char *filename,
		      const struct location *loc)
{
	return include_file(nft, nft->scanner, filename, loc, NULL, NFT_CMDLINE);
}

static bool search_in_include_path(const char *filename)
{
	return (strncmp(filename, "./", strlen("./")) != 0 &&
		strncmp(filename, "../", strlen("../")) != 0 &&
		filename[0] != '/');
}

static int include_path_glob(struct nft_ctx *nft, void *scanner,
			     const char *include_path, const char *filename,
			     const struct location *loc)
{
	struct parser_state *state = yyget_extra(scanner);
	struct error_record *erec;
	char buf[PATH_MAX];
	int ret;

	ret = snprintf(buf, sizeof(buf), "%s/%s", include_path, filename);
	if (ret < 0 || ret >= PATH_MAX) {
		erec = error(loc, "Too long file path \"%s/%s\"\n",
			     include_path, filename);
		erec_queue(erec, state->msgs);
		return -1;
	}

	ret = include_glob(nft, scanner, buf, loc);

	/* error was already handled */
	if (ret == -1)
		return -1;
	/* no wildcards and file was processed: break early. */
	if (ret == 0)
		return 0;

	/* else 1 (no wildcards) or 2 (wildcards): keep
	 * searching.
	 */
	return ret;
}

int scanner_include_file(struct nft_ctx *nft, void *scanner,
			 const char *filename, const struct location *loc)
{
	struct parser_state *state = yyget_extra(scanner);
	struct error_record *erec;
	unsigned int i;
	int ret = -1;

	if (search_in_include_path(filename)) {
		for (i = 0; i < nft->num_include_paths; i++) {
			ret = include_path_glob(nft, scanner,
						nft->include_paths[i],
						filename, loc);
			if (ret <= 0)
				return ret;
		}
		ret = include_path_glob(nft, scanner, DEFAULT_INCLUDE_PATH,
					filename, loc);
		if (ret <= 0)
			return ret;
	} else {
		/* an absolute path (starts with '/') */
		ret = include_glob(nft, scanner, filename, loc);
	}

	/* handle the case where no file was found */
	if (ret == -1)
		return -1;
	else if (ret == 0 || ret == 2)
		return 0;

	/* 1 means an error, because there are no more include directories to
	 * search, and the pattern does not have wildcard characters.
	 */
	erec = error(loc, "File not found: %s", filename);
	erec_queue(erec, state->msgs);
	return -1;
}

void scanner_push_buffer(void *scanner, const struct input_descriptor *indesc,
			 const char *buffer)
{
	struct parser_state *state = yyget_extra(scanner);
	struct input_descriptor *new_indesc;
	YY_BUFFER_STATE b;

	new_indesc = xzalloc(sizeof(struct input_descriptor));
	memcpy(new_indesc, indesc, sizeof(*new_indesc));
	new_indesc->data = buffer;
	new_indesc->name = xstrdup(indesc->name);
	scanner_push_indesc(state, new_indesc);

	b = yy_scan_string(buffer, scanner);
	assert(b != NULL);
	init_pos(state->indesc);
}

void *scanner_init(struct parser_state *state)
{
	yyscan_t scanner;

	yylex_init_extra(state, &scanner);
	yyset_out(NULL, scanner);

	state->startcond_active = xzalloc_array(__SC_MAX,
						sizeof(*state->startcond_active));
	return scanner;
}

static void input_descriptor_destroy(const struct input_descriptor *indesc)
{
	if (indesc->name)
		free_const(indesc->name);
	free_const(indesc);
}

static void input_descriptor_list_destroy(struct parser_state *state)
{
	struct input_descriptor *indesc, *next;

	list_for_each_entry_safe(indesc, next, &state->indesc_list, list) {
		if (indesc->f) {
			fclose(indesc->f);
			indesc->f = NULL;
		}
		list_del(&indesc->list);
		input_descriptor_destroy(indesc);
	}
}

void scanner_destroy(struct nft_ctx *nft)
{
	struct parser_state *state = yyget_extra(nft->scanner);

	input_descriptor_list_destroy(state);
	free(state->startcond_active);

	yylex_destroy(nft->scanner);
}

static void scanner_push_start_cond(void *scanner, enum startcond_type type)
{
	struct parser_state *state = yyget_extra(scanner);

	state->startcond_type = type;
	state->startcond_active[type]++;

	yy_push_state((int)type, scanner);
}

void scanner_pop_start_cond(void *scanner, enum startcond_type t)
{
	struct parser_state *state = yyget_extra(scanner);

	state->startcond_active[t]--;

	if (state->startcond_type != t) {
		state->flex_state_pop++;
		return; /* Can't pop just yet! */
	}

	while (state->flex_state_pop) {
		state->flex_state_pop--;
		state->startcond_type = yy_top_state(scanner);
		yy_pop_state(scanner);

		t = state->startcond_type;
		if (state->startcond_active[t])
			return;
	}

	state->startcond_type = yy_top_state(scanner);

	yy_pop_state(scanner);
}

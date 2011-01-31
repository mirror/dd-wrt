%{
/* cfg.y - configuration language */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#include "atm.h"
#include "atmd.h"

#include "proto.h"
#include "io.h"
#include "trace.h"
#include "policy.h"


static RULE *rule;
static SIG_ENTITY *curr_sig = &_entity;

void yyerror(const char *s);
void yywarn(const char *s);
int yylex(void);

static int hex2num(char digit)
{
    if (isdigit(digit)) return digit-'0';
    if (islower(digit)) return toupper(digit)-'A'+10;
    return digit-'A'+10;
}


static void put_address(char *address)
{
    char *mask;

    mask = strchr(address,'/');
    if (mask) *mask++ = 0;
    if (text2atm(address,(struct sockaddr *) &rule->addr,sizeof(rule->addr),
      T2A_SVC | T2A_WILDCARD | T2A_NAME | T2A_LOCAL) < 0) {
	yyerror("invalid address");
	return;
    }
    if (!mask) rule->mask = -1;
    else rule->mask = strtol(mask,NULL,10);
    add_rule(rule);
}

%}

%union {
    int num;
    char *str;
    struct sockaddr_atmpvc pvc;
};


%token		TOK_LEVEL TOK_DEBUG TOK_INFO TOK_WARN TOK_ERROR TOK_FATAL
%token		TOK_SIG TOK_UNI30 TOK_UNI31 TOK_UNI40 TOK_Q2963_1 TOK_SAAL
%token		TOK_VC TOK_IO TOK_MODE TOK_USER TOK_NET TOK_SWITCH TOK_VPCI
%token		TOK_ITF TOK_PCR TOK_TRACE TOK_POLICY TOK_ALLOW TOK_REJECT
%token		TOK_ENTITY TOK_DEFAULT
%token <num>	TOK_NUMBER TOK_MAX_RATE
%token <str>	TOK_DUMP_DIR TOK_LOGFILE TOK_QOS TOK_FROM TOK_TO TOK_ROUTE
%token <pvc>	TOK_PVC

%type <num>	level opt_trace_size action

%%

all:
    global local
    ;

global:
    | item global
    ;

local:
    | entity local
	{
	    if (!curr_sig->uni)
		curr_sig->uni =
#if defined(UNI30) || defined(DYNAMIC_UNI)
		  S_UNI30
#endif
#ifdef UNI31
		  S_UNI31
#ifdef ALLOW_UNI30
		  | S_UNI30
#endif
#endif
#ifdef UNI40
		  S_UNI40
#ifdef Q2963_1
		  | S_Q2963_1
#endif
#endif
		  ;
	}
    ;

item:
    TOK_LEVEL level
	{
	    set_verbosity(NULL,$2);
	}
    | TOK_SIG sig
    | TOK_SAAL saal
    | TOK_IO io
    | TOK_DEBUG debug
    | TOK_POLICY policy
    ;

entity:
    TOK_ENTITY TOK_PVC
	{
	    SIG_ENTITY *sig,**walk;

	    if (atmpvc_addr_in_use(_entity.signaling_pvc))
		yyerror("can't use  io vc  and  entity ...  in the same "
		  "configuration");
	    if (entities == &_entity) entities = NULL;
	    for (sig = entities; sig; sig = sig->next)
		if (atm_equal((struct sockaddr *) &sig->signaling_pvc,
		  (struct sockaddr *) &$2,0,0))
		    yyerror("duplicate PVC address");
	    curr_sig = alloc_t(SIG_ENTITY);
	    *curr_sig = _entity;
	    curr_sig->signaling_pvc = $2;
	    curr_sig->next = NULL;
	    for (walk = &entities; *walk; walk = &(*walk)->next);
	    *walk = curr_sig;
	}
      opt_options
    ;

opt_options:
    | '{' options '}'
    ;

options:
    | option options
    ;

option:
    TOK_VPCI TOK_NUMBER TOK_ITF TOK_NUMBER
	{
	    enter_vpci(curr_sig,$2,$4);
	}
    | TOK_MODE mode
    | TOK_QOS
	{
	    curr_sig->sig_qos = $1;
	}
    | TOK_MAX_RATE
	{
	    curr_sig->max_rate = $1;
	}
    | TOK_ROUTE
	{
	     struct sockaddr_atmsvc addr;
	     char *mask;

	    mask = strchr($1,'/');
	    if (mask) *mask++ = 0;
	    if (text2atm($1,(struct sockaddr *) &addr,sizeof(addr),
	    T2A_SVC | T2A_WILDCARD | T2A_NAME | T2A_LOCAL) < 0) {
		yyerror("invalid address");
		YYABORT;
	    }
	    add_route(curr_sig,&addr,mask ? strtol(mask,NULL,10) : INT_MAX);
	}
    | TOK_DEFAULT
	{
	    add_route(curr_sig,NULL,0);
	}
    ;

sig:
    sig_item
    | '{' sig_items '}'
    ;

sig_items:
    | sig_item sig_items
    ;

saal:
    saal_item
    | '{' saal_items '}'
    ;

saal_items:
    | saal_item saal_items
    ;

io:
    io_item
    | '{' io_items '}'
    ;

io_items:
    | io_item io_items
    ;

debug:
    debug_item
    | '{' debug_items '}'
    ;

debug_items:
    | debug_item debug_items
    ;

policy:
    policy_item
    | '{' policy_items '}'
    ;

policy_items:
    | policy_item policy_items
    ;

sig_item:
    TOK_LEVEL level
	{
	    set_verbosity("UNI",$2);
	    set_verbosity("KERNEL",$2);
	    set_verbosity("SAP",$2);
	}
    | TOK_VPCI TOK_NUMBER TOK_ITF TOK_NUMBER
	{
	    enter_vpci(curr_sig,$2,$4);
	}
    | TOK_UNI30
	{
#if defined(UNI30) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni & ~S_UNI31) yyerror("UNI mode is already set");
	    curr_sig->uni |= S_UNI30;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    | TOK_UNI31
	{
#if defined(UNI31) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni & ~S_UNI30) yyerror("UNI mode is already set");
	    curr_sig->uni |= S_UNI31;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    | TOK_UNI40
	{
#if defined(UNI40) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni) yyerror("UNI mode is already set");
	    curr_sig->uni = S_UNI40;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    | TOK_Q2963_1
	{
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	    if (!(curr_sig->uni & S_UNI40)) yyerror("Incompatible UNI mode");
	    curr_sig->uni |= S_Q2963_1;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    | TOK_NET
	{
	    yywarn("sig net  is obsolete, please use  sig mode net  instead");
	    curr_sig->mode = sm_net;
	}
    | TOK_MODE mode
    ;

saal_item:
    TOK_LEVEL level
	{
	    set_verbosity("SSCF",$2);
	    set_verbosity("SSCOP",$2);
	}
    ;

io_item:
    TOK_LEVEL level
	{
	    set_verbosity("IO",$2);
	}
    | TOK_VC TOK_PVC
	{
	    curr_sig->signaling_pvc = $2;
	}
    | TOK_PCR TOK_NUMBER
	{
	    yywarn("io pcr  is obsolete, please use  io qos  instead");
	    curr_sig->sig_pcr = $2;
	}
    | TOK_QOS
	{
	    curr_sig->sig_qos = $1;
	}
    | TOK_MAX_RATE
	{
	    curr_sig->max_rate = $1;
	}
    ;

debug_item:
    TOK_LEVEL level
	{
	    set_verbosity(NULL,$2);
	}
    | TOK_DUMP_DIR
	{
	    dump_dir = $1;
	    if (!trace_size) trace_size = DEFAULT_TRACE_SIZE;
	}
    | TOK_LOGFILE
	{
	    set_logfile($1);
	}
    | TOK_TRACE opt_trace_size
	{
	    trace_size = $2;
	}
    ;

opt_trace_size:
 	{
	    $$ = DEFAULT_TRACE_SIZE;
	}
    | TOK_NUMBER
	{
	    $$ = $1;
	}
    ;

level:
    TOK_DEBUG
	{
	    $$ = DIAG_DEBUG;
	}
    | TOK_INFO
	{
	    $$ = DIAG_INFO;
	}
    | TOK_WARN
	{
	    $$ = DIAG_WARN;
	}
    | TOK_ERROR
	{
	    $$ = DIAG_ERROR;
	}
    | TOK_FATAL
	{
	    $$ = DIAG_FATAL;
	}
    ;

mode:
    TOK_USER
	{
	    curr_sig->mode = sm_user;
	}
    | TOK_NET
	{
	    curr_sig->mode = sm_net;
	}
    | TOK_SWITCH
	{
	    curr_sig->mode = sm_switch;
	}
    ;

policy_item:
    TOK_LEVEL level
	{
	    set_verbosity("POLICY",$2);
	}
    | action
	{
	    rule = alloc_t(RULE);
	    rule->type = $1;
	}
      direction
    ;

action:
    TOK_ALLOW
	{
	    $$ = ACL_ALLOW;
	}
    | TOK_REJECT
	{
	    $$ = ACL_REJECT;
	}
    ;

direction:
    TOK_FROM
	{
	    rule->type |= ACL_IN;
	    put_address($1);
	}
    | TOK_TO
	{
	    rule->type |= ACL_OUT;
	    put_address($1);
	}
    ;

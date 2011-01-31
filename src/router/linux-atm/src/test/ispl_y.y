%{
/* isp.y - Internal Signaling Protocol test generator language */

/* Written 1997,1998 by Werner Almesberger, EPFL-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <atm.h>
#include <linux/atmsvc.h>

#include "isp.h"


static struct atmsvc_msg msg;

int yylex (void);

%}

%union {
    char *str;
    int num;
    enum atmsvc_msg_type type;
    VAR *var;
};


%token		TOK_SEND TOK_WAIT TOK_RECEIVE TOK_HELP TOK_SET TOK_SHOW TOK_ECHO
%token		TOK_VCC TOK_LISTEN TOK_LISTEN_VCC TOK_REPLY TOK_PVC
%token		TOK_LOCAL TOK_QOS TOK_SVC TOK_BIND TOK_CONNECT TOK_ACCEPT
%token		TOK_REJECT TOK_LISTEN TOK_OKAY TOK_ERROR TOK_INDICATE
%token		TOK_CLOSE TOK_ITF_NOTIFY TOK_MODIFY TOK_SAP
%token		TOK_IDENTIFY TOK_TERMINATE TOK_EOL
%token	<str>	TOK_VALUE TOK_VARIABLE

%type	<type>	type
%type	<num>	field_type number
%type	<var>	new_var old_var

%%

all:
    | command all
    ;

command:
    TOK_SEND type
	{
	    memset(&msg,0,sizeof(msg));
	    msg.type = $2;
	}
      values
	{
	    send_msg(&msg);
	    if (verbose) dump_msg("SENT",&msg);
	}
    | TOK_RECEIVE
	{
	    recv_msg(&msg);
	    if (!quiet) dump_msg("RECV",&msg);
	}
      opt_recv
    | TOK_WAIT number
	{
	    sleep($2);
	}
    | TOK_SET new_var '=' TOK_VALUE
	{
	    assign($2,eval(vt_text,$4));
	    free($4);
	}
    | TOK_SHOW
	{
	    VAR *var;

	    for (var = variables; var; var = var->next) {
		printf("%s = ",var->name);
		print_value(var->value);
		putchar('\n');
	    }
	}
    | TOK_ECHO TOK_VALUE
	{
	    printf("%s\n",$2);
	    free($2);
	}
    | help
	{
	    fprintf(stderr,
"Commands:\n"
"  send msg_type [field=value|field=$var ...]\n"
"  receive [msg_type [field=value|field=$var|$var=field ...]]\n"
"  set $var=value\n"
"  show\n"
"  echo value\n"
"  help\n\n"
"msg_type: bind, connect, accept, reject, listen, okay, error, indicate,\n"
"          close, itf_notify, modify, identify, terminate\n"
"field: vcc, listen_vcc, reply, pvc, local, qos, svc, sap\n");
	}
    | TOK_EOL
    ;

type:
    TOK_BIND
	{
	    $$ = as_bind;
	}
    | TOK_CONNECT
	{
	    $$ = as_connect;
	}
    | TOK_ACCEPT
	{
	    $$ = as_accept;
	}
    | TOK_REJECT
	{
	    $$ = as_reject;
	}
    | TOK_LISTEN
	{
	    $$ = as_listen;
	}
    | TOK_OKAY
	{
	    $$ = as_okay;
	}
    | TOK_ERROR
	{
	    $$ = as_error;
	}
    | TOK_INDICATE
	{
	    $$ = as_indicate;
	}
    | TOK_CLOSE
	{
	    $$ = as_close;
	}
    | TOK_ITF_NOTIFY
	{
	    $$ = as_itf_notify;
	}
    | TOK_MODIFY
	{
	    $$ = as_modify;
	}
    | TOK_IDENTIFY
	{
	    $$ = as_identify;
	}
    | TOK_TERMINATE
	{
	    $$ = as_terminate;
	}
    ;

values:
    | value values
    ;

value:
    field_type '=' old_var
	{
	    cast($3,type_of($1));
	    store(&msg,$1,$3->value);
	}
    | field_type '=' TOK_VALUE
	{
	    store(&msg,$1,eval(type_of($1),$3));
	    free($3);
	}
    ;

number:
    TOK_VALUE
	{
	    char *end;

	    $$ = strtol($1,&end,10);
	    if (*end) yyerror("invalid number");
	    free($1);
	}
    ;

opt_recv:
    | type
	{
	    if (msg.type != $1) yyerror("wrong message type");
	}
      fields
   ;

fields:
   | field fields
   ;

field:
    new_var '=' field_type
	{
	    assign($1,pick(&msg,$3));
	}
    | field_type '=' old_var
	{
	    cast($3,type_of($1));
	    check(pick(&msg,$1),$3->value);
	}
    | field_type '=' TOK_VALUE
	{
	    check(pick(&msg,$1),eval(type_of($1),$3));
	    free($3);
	}
    ;

field_type:
    TOK_VCC
	{
	    $$ = F_VCC;
	}
    | TOK_LISTEN_VCC
	{
	    $$ = F_LISTEN_VCC;
	}
    | TOK_REPLY
	{
	    $$ = F_REPLY;
	}
    | TOK_PVC
	{
	    $$ = F_PVC;
	}
    | TOK_LOCAL
	{
	    $$ = F_LOCAL;
	}
    | TOK_QOS
	{
	    $$ = F_QOS;
	}
    | TOK_SVC
	{
	    $$ = F_SVC;
	}
    | TOK_SAP
	{
	    $$ = F_SAP;
	}
    ;

help:
    TOK_HELP
    | '?'
    ;

new_var:
    TOK_VARIABLE
	{
	    $$ = lookup($1);
	    if ($$) free($1);
	    else $$ = create_var($1);
	}
    ;

old_var:
    TOK_VARIABLE
	{
	    $$ = lookup($1);
	    if (!$$) yyerror("no such variable");
	    free($1);
	}
    ;

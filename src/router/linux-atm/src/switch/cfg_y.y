%{
/* cfg.y - switch configuration language */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "atm.h"

#include "fab.h"
#include "sig.h"
#include "route.h"
#include "swc.h"


static int itf;
static SIGNALING_ENTITY *sig;

void yyerror(const char *s);
int yylex(void);

%}

%union {
    int num;
    char *str;
    struct sockaddr_atmpvc pvc;
};


%token		TOK_COMMAND TOK_VPCI TOK_ITF TOK_DEFAULT
%token <str>	TOK_ROUTE TOK_STR TOK_SOCKET TOK_OPTION TOK_CONTROL
%token <num>	TOK_NUM
%token <pvc>	TOK_PVC

%type  <str>	opt_command

%%

all:
    | option all
    | sig all
    | TOK_CONTROL all
	{
	    control_init($1);
	}
    ;

option:
    TOK_OPTION TOK_STR
	{
	    fab_option($1,$2);
	}
    ;

sig:
    opt_command TOK_SOCKET '{'
        {
	    itf = 0;
	}
      opt_itf
	{
	    char *tmp;

	    tmp = strdup($2);
	    if (!tmp) yyerror(strerror(errno));
	    sig = sig_vc($1,tmp,itf);
	}
      opt_via
      routes '}'
    ;

opt_command:
	{
	    $$ = NULL;
	}
    | TOK_COMMAND TOK_STR
	{
	    $$ = strdup($2);
	    if (!$$) yyerror(strerror(errno));
	}
    ;

opt_itf:
    | TOK_ITF TOK_NUM
	{
	    itf = $2;
	}
    ;

opt_via:
    | TOK_PVC
	{
	    sig->pvc = $1;
	}
    ;

routes:
    | route routes
    | TOK_DEFAULT
	{
	    put_route(NULL,0,sig);
	}
      routes
    ;

route:
    TOK_ROUTE
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
	    put_route(&addr,mask ? strtol(mask,NULL,10) : INT_MAX,sig);
	}
    ;

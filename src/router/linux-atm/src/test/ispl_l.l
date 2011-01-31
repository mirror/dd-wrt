%{
/* isp.l - Internal Signaling Protocol test generator language */

/* Written 1997,1998 by Werner Almesberger, EPFL-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <linux/atmsvc.h>

#include "isp.h"
#include "ispl_y.h"


int yywrap(void)
{
       return 1;
}

static int lineno = 1;

%}

tail	([^a-zA-Z0-9]|$|[ \t\n]*=.*)

%%

send			return TOK_SEND;
wait			return TOK_WAIT;
receive			return TOK_RECEIVE;
help			return TOK_HELP;
set			return TOK_SET;
show			return TOK_SHOW;
echo			return TOK_ECHO;
vcc/{tail}		return TOK_VCC;
listen_vcc/{tail}	return TOK_LISTEN_VCC;
reply/{tail}		return TOK_REPLY;
pvc/{tail}		return TOK_PVC;
local/{tail}		return TOK_LOCAL;
qos/{tail}		return TOK_QOS;
svc/{tail}		return TOK_SVC;
sap/{tail}		return TOK_SAP;
listen			return TOK_LISTEN;
bind			return TOK_BIND;
connect			return TOK_CONNECT;
accept			return TOK_ACCEPT;
reject			return TOK_REJECT;
okay			return TOK_OKAY;
error			return TOK_ERROR;
indicate		return TOK_INDICATE;
close			return TOK_CLOSE;
itf_notify		return TOK_ITF_NOTIFY;
modify			return TOK_MODIFY;
identify		return TOK_IDENTIFY;
terminate		return TOK_TERMINATE;
\\[\t ]*\n		lineno++;
#[^\n]*\n		lineno++;
[^ \t\n\$=][^ \t\n]*	{
			    yylval.str = strdup(yytext);
			    if (!yylval.str) {
				perror("strdup");
				exit(1);
			    }
			    return TOK_VALUE;
			}
\$[0-9a-zA-Z_]+		{
			    yylval.str = strdup(yytext+1);
			    if (!yylval.str) {
				perror("strdup");
				exit(1);
			    }
			    return TOK_VARIABLE;
			}
\n			{
			    lineno++;
			    return TOK_EOL;
			}
[\t ]*			;
.			return *yytext;

%%

void yyerror(const char *s)
{
    fprintf(stderr,"line %d: %s near \"%s\"\n",lineno,s,yytext);
    exit(1);
}

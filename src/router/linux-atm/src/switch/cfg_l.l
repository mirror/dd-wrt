%{
/* cfg.l - switch configuration language */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "atm.h"

#include "cfg_y.h"


static int lineno = 1;
static int token; /* f@#%ing flex doesn't grok return after BEGIN */


void yyerror(const char *s);

%}

%s N
%s P

%%
		BEGIN(N);

<N>option	{ BEGIN(P);
		  token = TOK_OPTION; }
<N>control	{ BEGIN(P);
		  token = TOK_CONTROL; }
<N>command	return TOK_COMMAND;
<N>socket	{ BEGIN(P);
		  token = TOK_SOCKET; }
<N>vpci		return TOK_VPCI;
<N>itf		return TOK_ITF;
<N>route	{ BEGIN(P);
		  token = TOK_ROUTE; }
<N>default	return TOK_DEFAULT;
<N>\"[^"\t\n]*\" { yylval.str = strdup(yytext+1);
		  *strrchr(yylval.str,'"') = 0;
		  return TOK_STR; }
<N>[0-9]+	{ char *end;
		  yylval.num = strtoul(yytext,&end,10);
		  if (*end) yyerror("invalid number");
		  return TOK_NUM; }
<N>[0-9]+\.[0-9]+(\.[0-9]+)? {
		  if (text2atm(yytext,(struct sockaddr *) &yylval.pvc,
		    sizeof(yylval.pvc),T2A_PVC) < 0) yyerror("invalid VC");
		  return TOK_PVC; }
<P>[^\t\n ]+	{ BEGIN(N);
		  yylval.str = strdup(yytext);
		  if (!yylval.str) {
		      perror("strdup");
		      exit(1);
		  }
		  return token; }
\n?[\t ]*	lineno += *yytext == '\n';
#[^\n]*\n	lineno++;
<N>.		return *yytext;

%%

void yyerror(const char *s)
{
    fprintf(stderr,"line %d: %s near \"%s\"\n",lineno,s,yytext);
    exit(1);
}

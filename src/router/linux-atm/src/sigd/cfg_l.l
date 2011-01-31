%{
/* cfg.l - configuration language */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "atm.h"

#include "cfg_y.h"

int yywrap(void)
{
        return 1;
}

static int lineno = 1;
static int token; /* f@#%ing flex doesn't grok return after BEGIN */


void yyerror(const char *s);
void yywarn(const char *s);

%}

%s N
%s P
%s R

%%
			BEGIN(N);

<N>level		return TOK_LEVEL;
<N>debug		return TOK_DEBUG;
<N>info			return TOK_INFO;
<N>warn			return TOK_WARN;
<N>error		return TOK_ERROR;
<N>fatal		return TOK_FATAL;
<N>sig			return TOK_SIG;
<N>uni30		return TOK_UNI30;
<N>3.0			return TOK_UNI30;
<N>uni31		return TOK_UNI31;
<N>3.1			return TOK_UNI31;
<N>uni40		return TOK_UNI40;
<N>4.0			return TOK_UNI40;
<N>[qQ].2963.1		return TOK_Q2963_1;
<N>mode			return TOK_MODE;
<N>user			return TOK_USER;
<N>network		return TOK_NET;
<N>switch		return TOK_SWITCH;
<N>saal			return TOK_SAAL;
<N>vc			return TOK_VC;
<N>io			return TOK_IO;
<N>itf			return TOK_ITF;
<N>vpci			return TOK_VPCI;
<N>pcr			return TOK_PCR;
<N>policy		return TOK_POLICY;
<N>allow		return TOK_ALLOW;
<N>reject		return TOK_REJECT;
<N>entity		return TOK_ENTITY;
<N>default		return TOK_DEFAULT;
<N>dump			{ BEGIN(P);
			  token = TOK_DUMP_DIR; }
<N>log			{ BEGIN(P);
			  token = TOK_LOGFILE; }
<N>qos			{ BEGIN(P); /* syntacticly close to a path */
			  token = TOK_QOS; }
<N>from			{ BEGIN(P); /* syntacticly close to a path */
			  token = TOK_FROM; }
<N>to			{ BEGIN(P); /* syntacticly close to a path */
			  token = TOK_TO; }
<N>route		{ BEGIN(P); /* syntacticly close to a path */
			  token = TOK_ROUTE; }
<N>max_rate		{ BEGIN(R); /* rate */
			  token = TOK_MAX_RATE; }
<N>trace		return TOK_TRACE;
<N>[0-9]+		{ char *end;
			  yylval.num = strtoul(yytext,&end,10);
			  if (*end) yyerror("invalid number");
			  return TOK_NUMBER; }
<N>[0-9]+\.[0-9]+(\.[0-9]+)? {
			  if (text2atm(yytext,(struct sockaddr *) &yylval.pvc,
			    sizeof(yylval.pvc),T2A_PVC) < 0)
			      yyerror("invalid signaling channel");
			  return TOK_PVC;
			}
<P>[^\t\n ]+		{ BEGIN(N);
			  yylval.str = strdup(yytext); /* tiny leak ... */
			  if (!yylval.str) {
				perror("strdup");
				exit(1);
			  }
			  return token; }
<R>[0-9]+\.?[0-9]*[\t ]*([kKmMgG]?[bBcC][pP][sS])? { const char *text;
			  BEGIN(N);
			  text = yytext;
			  yylval.num = __t2q_get_rate(&text,1);
			  if (yylval.num < 0) yyerror("invalid rate");
			  return token; }
\n?[\t ]*		lineno += *yytext == '\n';
#[^\n]*\n		lineno++;
<N>.			return *yytext;

%%

void yywarn(const char *s)
{
    fprintf(stderr,"line %d: %s near \"%s\"\n",lineno,s,yytext);
}


void yyerror(const char *s)
{
    yywarn(s);
    exit(1);
}

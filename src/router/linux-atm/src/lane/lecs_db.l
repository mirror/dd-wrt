%{

#if HAVE_CONFIG_H
#include <config.h>
#endif

/*Standard includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local includes */
#include "lecs_load.h"
#include "lecs_db.h"

char *g_return;
unsigned int g_lineno=1;
%}
C [a-zA-Z]
H [0-9a-fA-F]

%%
[\t ]			{}

\n			{g_lineno++;}

\[{C}[a-zA-Z0-9\-_]*\] |
\[\]			{
                          g_return = (char*)
			    malloc(strlen(yytext));
			  strcpy(g_return, &yytext[1]);
			  g_return[strlen(yytext)-2] = '\0';
			  return ELAN_NAME;
			}

[Ll][Ee][Ss][\t ]?\:?\=?     	{ return LES_ADDR;}

[Tt][Yy][Pp][Ee][\t ]?\:?\=?  { return TYPE;}

802\_?3 |
[Ee][Tt][Hh][Ee][Rr][Nn][Ee][Tt] { return TYPE_ETHERNET;}

802\_?5 |
[Tt][Oo][Kk][Ee][Nn]\_?[Rr][Ii][Nn][Gg] { return TYPE_TR;}

[Mm][Aa][Xx]\_?[Ff][Rr][Aa][Mm][Ee][\t ]?\:?\=? {return MAX_FRAME;}

[Dd][Ee][Ff][Aa][Uu][Ll][Tt] { return DEFAULT;}

1516			{ return MF_1516; }
4544			{ return MF_4544; }
9234			{ return MF_9234; }
18190			{ return MF_18190;}

{H}[0-9a-fA-FxX\.\:\-]*	{
			  g_return = (char*)
			    malloc(strlen(yytext));
			  strcpy(g_return, yytext);
			  g_return[strlen(yytext)] = '\0';
			  return ADDRESS_ATM;
			}


\#.*			{}

.			{
				int i;
				while ((i = input())!='\n');
				unput(i);
				return ERROR;
			}

%%
int 
yywrap()
{
  return 1;
}

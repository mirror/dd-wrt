%{
/* ql.l - Q.2931 data structures description language */
 
/* Written 1995,1996 by Werner Almesberger, EPFL-LRC */

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "qgen.h"
#include "ql_y.h"


int yywrap(void)
{
        return 1;
}

typedef struct _tree {
    struct _tree *left,*right;
    const char str[0];
} TREE;


static TREE *tree = NULL;
static int lineno = 1;


static const char *ident(const char *str)
{
    TREE **walk;
    int diff;
 
    walk = &tree;
    while (*walk) {
        if (!(diff = strcmp(str,(*walk)->str))) return (*walk)->str;
        if (diff < 0) walk = &(*walk)->left;
        else walk = &(*walk)->right;
    }
    *walk = alloc(sizeof(TREE)+strlen(str)+1);
    (*walk)->left = (*walk)->right = NULL;
    strcpy((char *) (*walk)->str,str);
    return (*walk)->str;
}

%}

%%

break				return TOK_BREAK;
case				return TOK_CASE;
def				return TOK_DEF;
default				return TOK_DEFAULT;
length				return TOK_LENGTH;
multi				return TOK_MULTI;
recover				return TOK_RECOVER;
abort				return TOK_ABORT;
[_a-zA-Z0-9]+			{ yylval.str = ident(yytext);
				  return TOK_ID; }
\n?[\t ]*			lineno += *yytext == '\n';
include[\ \t]+\"[^\"\n\t]+\"	|
include[\ \t]+\<[^\<\>\n\t]+\>	{ yylval.str = ident(yytext);
				  return TOK_INCLUDE; }
\"[^\"\n\t]+\"			{ *strrchr(yytext,'"') = 0;
				  yylval.str = ident(yytext+1);
				  return TOK_STRING; }
[#;][^\n]*\n			lineno++;
.				return *yytext;

%%

void yyerror(char *s)
{
    fprintf(stderr,"line %d: %s near \"%s\"\n",lineno,s,yytext);
    exit(1);
}

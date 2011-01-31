%{

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "load_lex.h"
#include "mem.h"
#include "load.h"

Ret_t g_return;
char g_buf[256];
int g_buf_index;

char s_input(void);
int s_unput(char);

%}

D [0-9]
C [a-zA-Z]
H [0-9a-fA-F]

%%
[\t\n ]		{}
\"[^"]*\"      	{
                 g_return.stringgi = mem_alloc(&load_unit, strlen(yytext));
		 strcpy(g_return.stringgi,&yytext[1]);
		 g_return.stringgi[strlen(yytext)-2] = '\0';
		 return STRING;}
\:.*		{int i;
		 int hex;
		 g_return.atmaddress = (AtmAddr_t *)
		   mem_alloc(&load_unit, sizeof(AtmAddr_t));
		 for(i=0;i<20;i++) {
		   if(sscanf(&yytext[i*3],":%x",&hex) != 1)
		     return ERROR;
		   g_return.atmaddress->addr[i] = (unsigned char) hex;
		 }
		 return ATMADDRESS;
	        }
True	|
true		{g_return.bool = BL_TRUE;
		 return BOOLEAN;
		}	
False	|
false		{g_return.bool = BL_FALSE;
		 return BOOLEAN;
		}
\#.*		{}
{H}{H}\;{H}{H}\;{H}{H}\;{H}{H}\;{H}{H}\;{H}{H}\;	{
                 int i, hex;
		 g_return.destaddr = (LaneDestination_t *)
		   mem_alloc(&load_unit, sizeof(LaneDestination_t));
		 for(i=0;i<5;i++) {
		   if (sscanf(&yytext[i*3],"%x;",&hex) != 1)
		     return ERROR;
		   g_return.destaddr->a_r.mac_address[i] = hex;
		 }	
		 g_return.destaddr->tag = LANE_DEST_MAC;
		 return LANEDEST;
		}
{D}+\,{D}+\,{D}+	{
		 sscanf(yytext,"%ld,%ld,%ld",&g_return.vcc.port,
			&g_return.vcc.vpi, &g_return.vcc.vci);
		 return VCC;
	        }
{D}+		{int tmp;
		 sscanf(yytext,"%d",&tmp);
		 g_return.intti = tmp;
		 return INTEGER;}
\[{C}[a-zA-Z0-9]*\] {g_return.stringgi = mem_alloc(&load_unit, strlen(yytext));
		 strcpy(g_return.stringgi,&yytext[1]);
		 g_return.stringgi[strlen(yytext)-2] = '\0';
		 return UNIT;}
{C}[a-zA-Z0-9]*\=  {g_return.stringgi = mem_alloc(&load_unit, strlen(yytext));
		 strcpy(g_return.stringgi,yytext);
		 g_return.stringgi[strlen(yytext)-1] = '\0';
		 return VARNAME;}
\~              {return END;}
.               {return ERROR;}
%%
/* Flex != lex
#undef input
#define input s_input
#undef unput
#define unput(c) s_unput(c)

char s_input(void)
{
  int c;

  if (g_buf_index <=0)
    c = getc(g_fd);
  else
    c = g_buf[--g_buf_index];
  if (c==EOF) {return '~';}
  printf("%d :%c\n",c,c);
  return c;
}
*/
int yywrap()
{
  char c;

  return 1;
  c = input();
  if (c == '~')
    return 1;
  else {
    unput(c);
    return 0;
  }
}
/*
int s_unput(char c)
{
  g_buf[g_buf_index++] = c;
  return 1;
}
*/



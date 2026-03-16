/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2013, 2020
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: yacconfig.h,v 1.21.10.2 2020/11/11 16:11:51 karls Exp $ */

#ifndef _YACCONFIG_H_
#define _YACCONFIG_H_

/*
 * avoid symbolconflicts if socksifying programs that also use yacc/lex.
 */

#define yyact socks_yyact
#define yychar socks_yychar
#define yychk socks_yychk
#define yydebug socks_yydebug
#define yydef socks_yydef
#define yyerrflag socks_yyerrflag
#define yyerror socks_yyerror
#define yyerrorx socks_yyerrorx
#define yywarn socks_yywarn
#define yywarnx socks_yywarnx
#define yyexca socks_yyexca
#define yylex socks_yylex
#define yylval socks_yylval
#define yynerrs socks_yynerrs
#define yypact socks_yypact
#define yyparse socks_yyparse
#define yypgo socks_yypgo
#define yyps socks_yyps
#define yypv socks_yypv
#define yyr1 socks_yyr1
#define yyr2 socks_yyr2
#define yyreds socks_yyreds
#define yys socks_yys
#define yystate socks_yystate
#define yytmp socks_yytmp
#define yytoks socks_yytoks
#define yyv socks_yyv
#define yyval socks_yyval
#define yyback socks_yyback
#define yybgin socks_yybgin
#define yycrank socks_yycrank
#define yyestate socks_yyestate
#define yyextra socks_yyextra
#define yyfnd socks_yyfnd
#define yyin socks_yyin
#define yyinput socks_yyinput
#define yyleng socks_yyleng
#define yylex socks_yylex
#define yylineno socks_yylineno
#define yylook socks_yylook
#define yylsp socks_yylsp
#define yylstate socks_yylstate
#define yylval socks_yylval
#define yymatch socks_yymatch
#define yymorfg socks_yymorfg
#define yyolsp socks_yyolsp
#define yyout socks_yyout
#define yyoutout socks_yyoutout
#define yyprevious socks_yyprevious
#define yysbuf socks_yysbuf
#define yysptr socks_yysptr
#define yysvec socks_yysvec
#define yytchar socks_yytchar
#define yytext socks_yytext
#define yytop socks_yytop
#define yyunput socks_yyunput
#define yyvstop socks_yyvstop
/*#define yywrap socks_yywrap */
#define yy_create_buffer socks_yy_create_buffer
#define yy_delete_buffer socks_yy_delete_buffer
#define yy_init_buffer socks_yy_init_buffer
#define yy_load_buffer_state socks_yy_load_buffer_state
#define yy_switch_to_buffer socks_yy_switch_to_buffer
#define yyleng socks_yyleng
#define yyrestart socks_yyrestart



typedef enum { VALUETYPE_ERRNO = 1, VALUETYPE_GAIERR } valuetype_t;

void
yywarn(const char *fmt, ...)
   __ATTRIBUTE__((FORMAT(printf, 1, 2)));

void
yywarnx(const char *fmt, ...)
   __ATTRIBUTE__((FORMAT(printf, 1, 2)));

/*
 * Report an error related to (config file) parsing.
 */

void
yyerror(const char *fmt, ...)
   __ATTRIBUTE__((noreturn)) __ATTRIBUTE__((FORMAT(printf, 1, 2)));

void
yyerrorx(const char *fmt, ...)
   __ATTRIBUTE__((noreturn)) __ATTRIBUTE__((FORMAT(printf, 1, 2)));

/*
 * Report an error related to (config file) parsing and exit.
 */


void
yylog(const int loglevel, const char *fmt, ...)
   __ATTRIBUTE__((FORMAT(printf, 2, 3)));

/*
 * Log a parsing-related notice at loglevel "loglevel".
 */


void yyerrorx_nolib(const char *library);
void yywarnx_deprecated(const char *oldkeyword, const char *newkeyword);
void yyerrorx_nonetmask(void);
void yyerrorx_hasnetmask(void);
/*
 * misc. error functions related to specific parsing errorrs.
 */



#endif /* !_YACCONFIG_H_ */

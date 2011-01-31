/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_SEND = 258,
     TOK_WAIT = 259,
     TOK_RECEIVE = 260,
     TOK_HELP = 261,
     TOK_SET = 262,
     TOK_SHOW = 263,
     TOK_ECHO = 264,
     TOK_VCC = 265,
     TOK_LISTEN = 266,
     TOK_LISTEN_VCC = 267,
     TOK_REPLY = 268,
     TOK_PVC = 269,
     TOK_LOCAL = 270,
     TOK_QOS = 271,
     TOK_SVC = 272,
     TOK_BIND = 273,
     TOK_CONNECT = 274,
     TOK_ACCEPT = 275,
     TOK_REJECT = 276,
     TOK_OKAY = 277,
     TOK_ERROR = 278,
     TOK_INDICATE = 279,
     TOK_CLOSE = 280,
     TOK_ITF_NOTIFY = 281,
     TOK_MODIFY = 282,
     TOK_SAP = 283,
     TOK_IDENTIFY = 284,
     TOK_TERMINATE = 285,
     TOK_EOL = 286,
     TOK_VALUE = 287,
     TOK_VARIABLE = 288
   };
#endif
/* Tokens.  */
#define TOK_SEND 258
#define TOK_WAIT 259
#define TOK_RECEIVE 260
#define TOK_HELP 261
#define TOK_SET 262
#define TOK_SHOW 263
#define TOK_ECHO 264
#define TOK_VCC 265
#define TOK_LISTEN 266
#define TOK_LISTEN_VCC 267
#define TOK_REPLY 268
#define TOK_PVC 269
#define TOK_LOCAL 270
#define TOK_QOS 271
#define TOK_SVC 272
#define TOK_BIND 273
#define TOK_CONNECT 274
#define TOK_ACCEPT 275
#define TOK_REJECT 276
#define TOK_OKAY 277
#define TOK_ERROR 278
#define TOK_INDICATE 279
#define TOK_CLOSE 280
#define TOK_ITF_NOTIFY 281
#define TOK_MODIFY 282
#define TOK_SAP 283
#define TOK_IDENTIFY 284
#define TOK_TERMINATE 285
#define TOK_EOL 286
#define TOK_VALUE 287
#define TOK_VARIABLE 288




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 26 "ispl_y.y"

    char *str;
    int num;
    enum atmsvc_msg_type type;
    VAR *var;



/* Line 1685 of yacc.c  */
#line 126 "ispl_y.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;



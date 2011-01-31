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
     TOK_LEVEL = 258,
     TOK_DEBUG = 259,
     TOK_INFO = 260,
     TOK_WARN = 261,
     TOK_ERROR = 262,
     TOK_FATAL = 263,
     TOK_SIG = 264,
     TOK_UNI30 = 265,
     TOK_UNI31 = 266,
     TOK_UNI40 = 267,
     TOK_Q2963_1 = 268,
     TOK_SAAL = 269,
     TOK_VC = 270,
     TOK_IO = 271,
     TOK_MODE = 272,
     TOK_USER = 273,
     TOK_NET = 274,
     TOK_SWITCH = 275,
     TOK_VPCI = 276,
     TOK_ITF = 277,
     TOK_PCR = 278,
     TOK_TRACE = 279,
     TOK_POLICY = 280,
     TOK_ALLOW = 281,
     TOK_REJECT = 282,
     TOK_ENTITY = 283,
     TOK_DEFAULT = 284,
     TOK_NUMBER = 285,
     TOK_MAX_RATE = 286,
     TOK_DUMP_DIR = 287,
     TOK_LOGFILE = 288,
     TOK_QOS = 289,
     TOK_FROM = 290,
     TOK_TO = 291,
     TOK_ROUTE = 292,
     TOK_PVC = 293
   };
#endif
/* Tokens.  */
#define TOK_LEVEL 258
#define TOK_DEBUG 259
#define TOK_INFO 260
#define TOK_WARN 261
#define TOK_ERROR 262
#define TOK_FATAL 263
#define TOK_SIG 264
#define TOK_UNI30 265
#define TOK_UNI31 266
#define TOK_UNI40 267
#define TOK_Q2963_1 268
#define TOK_SAAL 269
#define TOK_VC 270
#define TOK_IO 271
#define TOK_MODE 272
#define TOK_USER 273
#define TOK_NET 274
#define TOK_SWITCH 275
#define TOK_VPCI 276
#define TOK_ITF 277
#define TOK_PCR 278
#define TOK_TRACE 279
#define TOK_POLICY 280
#define TOK_ALLOW 281
#define TOK_REJECT 282
#define TOK_ENTITY 283
#define TOK_DEFAULT 284
#define TOK_NUMBER 285
#define TOK_MAX_RATE 286
#define TOK_DUMP_DIR 287
#define TOK_LOGFILE 288
#define TOK_QOS 289
#define TOK_FROM 290
#define TOK_TO 291
#define TOK_ROUTE 292
#define TOK_PVC 293




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 58 "cfg_y.y"

    int num;
    char *str;
    struct sockaddr_atmpvc pvc;



/* Line 1685 of yacc.c  */
#line 135 "cfg_y.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;



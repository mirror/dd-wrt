/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DIR = 258,
     ACTION = 259,
     PROTOCOL = 260,
     MODE = 261,
     LEVEL = 262,
     LEVEL_SPECIFY = 263,
     IPADDRESS = 264,
     ME = 265,
     ANY = 266,
     SLASH = 267,
     HYPHEN = 268
   };
#endif
#define DIR 258
#define ACTION 259
#define PROTOCOL 260
#define MODE 261
#define LEVEL 262
#define LEVEL_SPECIFY 263
#define IPADDRESS 264
#define ME 265
#define ANY 266
#define SLASH 267
#define HYPHEN 268




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 99 "policy_parse.y"
typedef union YYSTYPE {
	u_int num;
	struct _val {
		int len;
		char *buf;
	} val;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 71 "policy_parse.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;




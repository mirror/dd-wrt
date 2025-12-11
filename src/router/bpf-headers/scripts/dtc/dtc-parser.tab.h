/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SCRIPTS_DTC_DTC_PARSER_TAB_H_INCLUDED
# define YY_YY_SCRIPTS_DTC_DTC_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    DT_V1 = 258,                   /* DT_V1  */
    DT_PLUGIN = 259,               /* DT_PLUGIN  */
    DT_MEMRESERVE = 260,           /* DT_MEMRESERVE  */
    DT_LSHIFT = 261,               /* DT_LSHIFT  */
    DT_RSHIFT = 262,               /* DT_RSHIFT  */
    DT_LE = 263,                   /* DT_LE  */
    DT_GE = 264,                   /* DT_GE  */
    DT_EQ = 265,                   /* DT_EQ  */
    DT_NE = 266,                   /* DT_NE  */
    DT_AND = 267,                  /* DT_AND  */
    DT_OR = 268,                   /* DT_OR  */
    DT_BITS = 269,                 /* DT_BITS  */
    DT_DEL_PROP = 270,             /* DT_DEL_PROP  */
    DT_DEL_NODE = 271,             /* DT_DEL_NODE  */
    DT_OMIT_NO_REF = 272,          /* DT_OMIT_NO_REF  */
    DT_PROPNODENAME = 273,         /* DT_PROPNODENAME  */
    DT_LITERAL = 274,              /* DT_LITERAL  */
    DT_CHAR_LITERAL = 275,         /* DT_CHAR_LITERAL  */
    DT_BYTE = 276,                 /* DT_BYTE  */
    DT_STRING = 277,               /* DT_STRING  */
    DT_LABEL = 278,                /* DT_LABEL  */
    DT_LABEL_REF = 279,            /* DT_LABEL_REF  */
    DT_PATH_REF = 280,             /* DT_PATH_REF  */
    DT_INCBIN = 281                /* DT_INCBIN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

	char *propnodename;
	char *labelref;
	uint8_t byte;
	struct data data;

	struct {
		struct data	data;
		int		bits;
	} array;

	struct property *prop;
	struct property *proplist;
	struct node *node;
	struct node *nodelist;
	struct reserve_info *re;
	uint64_t integer;
	unsigned int flags;


};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_SCRIPTS_DTC_DTC_PARSER_TAB_H_INCLUDED  */

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

#ifndef YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
# define YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef ZENDDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ZENDDEBUG 1
#  else
#   define ZENDDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ZENDDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ZENDDEBUG */
#if ZENDDEBUG
extern int zenddebug;
#endif
/* "%code requires" blocks.  */

#include "zend_compile.h"


/* Token kinds.  */
#ifndef ZENDTOKENTYPE
# define ZENDTOKENTYPE
  enum zendtokentype
  {
    ZENDEMPTY = -2,
    END = 0,                       /* "end of file"  */
    ZENDerror = 256,               /* error  */
    ZENDUNDEF = 257,               /* "invalid token"  */
    PREC_ARROW_FUNCTION = 258,     /* PREC_ARROW_FUNCTION  */
    T_NOELSE = 259,                /* T_NOELSE  */
    T_LNUMBER = 260,               /* "integer"  */
    T_DNUMBER = 261,               /* "floating-point number"  */
    T_STRING = 262,                /* "identifier"  */
    T_NAME_FULLY_QUALIFIED = 263,  /* "fully qualified name"  */
    T_NAME_RELATIVE = 264,         /* "namespace-relative name"  */
    T_NAME_QUALIFIED = 265,        /* "namespaced name"  */
    T_VARIABLE = 266,              /* "variable"  */
    T_INLINE_HTML = 267,           /* T_INLINE_HTML  */
    T_ENCAPSED_AND_WHITESPACE = 268, /* "string content"  */
    T_CONSTANT_ENCAPSED_STRING = 269, /* "quoted string"  */
    T_STRING_VARNAME = 270,        /* "variable name"  */
    T_NUM_STRING = 271,            /* "number"  */
    T_INCLUDE = 272,               /* "'include'"  */
    T_INCLUDE_ONCE = 273,          /* "'include_once'"  */
    T_EVAL = 274,                  /* "'eval'"  */
    T_REQUIRE = 275,               /* "'require'"  */
    T_REQUIRE_ONCE = 276,          /* "'require_once'"  */
    T_LOGICAL_OR = 277,            /* "'or'"  */
    T_LOGICAL_XOR = 278,           /* "'xor'"  */
    T_LOGICAL_AND = 279,           /* "'and'"  */
    T_PRINT = 280,                 /* "'print'"  */
    T_YIELD = 281,                 /* "'yield'"  */
    T_YIELD_FROM = 282,            /* "'yield from'"  */
    T_INSTANCEOF = 283,            /* "'instanceof'"  */
    T_NEW = 284,                   /* "'new'"  */
    T_CLONE = 285,                 /* "'clone'"  */
    T_EXIT = 286,                  /* "'exit'"  */
    T_IF = 287,                    /* "'if'"  */
    T_ELSEIF = 288,                /* "'elseif'"  */
    T_ELSE = 289,                  /* "'else'"  */
    T_ENDIF = 290,                 /* "'endif'"  */
    T_ECHO = 291,                  /* "'echo'"  */
    T_DO = 292,                    /* "'do'"  */
    T_WHILE = 293,                 /* "'while'"  */
    T_ENDWHILE = 294,              /* "'endwhile'"  */
    T_FOR = 295,                   /* "'for'"  */
    T_ENDFOR = 296,                /* "'endfor'"  */
    T_FOREACH = 297,               /* "'foreach'"  */
    T_ENDFOREACH = 298,            /* "'endforeach'"  */
    T_DECLARE = 299,               /* "'declare'"  */
    T_ENDDECLARE = 300,            /* "'enddeclare'"  */
    T_AS = 301,                    /* "'as'"  */
    T_SWITCH = 302,                /* "'switch'"  */
    T_ENDSWITCH = 303,             /* "'endswitch'"  */
    T_CASE = 304,                  /* "'case'"  */
    T_DEFAULT = 305,               /* "'default'"  */
    T_MATCH = 306,                 /* "'match'"  */
    T_BREAK = 307,                 /* "'break'"  */
    T_CONTINUE = 308,              /* "'continue'"  */
    T_GOTO = 309,                  /* "'goto'"  */
    T_FUNCTION = 310,              /* "'function'"  */
    T_FN = 311,                    /* "'fn'"  */
    T_CONST = 312,                 /* "'const'"  */
    T_RETURN = 313,                /* "'return'"  */
    T_TRY = 314,                   /* "'try'"  */
    T_CATCH = 315,                 /* "'catch'"  */
    T_FINALLY = 316,               /* "'finally'"  */
    T_THROW = 317,                 /* "'throw'"  */
    T_USE = 318,                   /* "'use'"  */
    T_INSTEADOF = 319,             /* "'insteadof'"  */
    T_GLOBAL = 320,                /* "'global'"  */
    T_STATIC = 321,                /* "'static'"  */
    T_ABSTRACT = 322,              /* "'abstract'"  */
    T_FINAL = 323,                 /* "'final'"  */
    T_PRIVATE = 324,               /* "'private'"  */
    T_PROTECTED = 325,             /* "'protected'"  */
    T_PUBLIC = 326,                /* "'public'"  */
    T_PRIVATE_SET = 327,           /* "'private(set)'"  */
    T_PROTECTED_SET = 328,         /* "'protected(set)'"  */
    T_PUBLIC_SET = 329,            /* "'public(set)'"  */
    T_READONLY = 330,              /* "'readonly'"  */
    T_VAR = 331,                   /* "'var'"  */
    T_UNSET = 332,                 /* "'unset'"  */
    T_ISSET = 333,                 /* "'isset'"  */
    T_EMPTY = 334,                 /* "'empty'"  */
    T_HALT_COMPILER = 335,         /* "'__halt_compiler'"  */
    T_CLASS = 336,                 /* "'class'"  */
    T_TRAIT = 337,                 /* "'trait'"  */
    T_INTERFACE = 338,             /* "'interface'"  */
    T_ENUM = 339,                  /* "'enum'"  */
    T_EXTENDS = 340,               /* "'extends'"  */
    T_IMPLEMENTS = 341,            /* "'implements'"  */
    T_NAMESPACE = 342,             /* "'namespace'"  */
    T_LIST = 343,                  /* "'list'"  */
    T_ARRAY = 344,                 /* "'array'"  */
    T_CALLABLE = 345,              /* "'callable'"  */
    T_LINE = 346,                  /* "'__LINE__'"  */
    T_FILE = 347,                  /* "'__FILE__'"  */
    T_DIR = 348,                   /* "'__DIR__'"  */
    T_CLASS_C = 349,               /* "'__CLASS__'"  */
    T_TRAIT_C = 350,               /* "'__TRAIT__'"  */
    T_METHOD_C = 351,              /* "'__METHOD__'"  */
    T_FUNC_C = 352,                /* "'__FUNCTION__'"  */
    T_PROPERTY_C = 353,            /* "'__PROPERTY__'"  */
    T_NS_C = 354,                  /* "'__NAMESPACE__'"  */
    T_ATTRIBUTE = 355,             /* "'#['"  */
    T_PLUS_EQUAL = 356,            /* "'+='"  */
    T_MINUS_EQUAL = 357,           /* "'-='"  */
    T_MUL_EQUAL = 358,             /* "'*='"  */
    T_DIV_EQUAL = 359,             /* "'/='"  */
    T_CONCAT_EQUAL = 360,          /* "'.='"  */
    T_MOD_EQUAL = 361,             /* "'%='"  */
    T_AND_EQUAL = 362,             /* "'&='"  */
    T_OR_EQUAL = 363,              /* "'|='"  */
    T_XOR_EQUAL = 364,             /* "'^='"  */
    T_SL_EQUAL = 365,              /* "'<<='"  */
    T_SR_EQUAL = 366,              /* "'>>='"  */
    T_COALESCE_EQUAL = 367,        /* "'??='"  */
    T_BOOLEAN_OR = 368,            /* "'||'"  */
    T_BOOLEAN_AND = 369,           /* "'&&'"  */
    T_IS_EQUAL = 370,              /* "'=='"  */
    T_IS_NOT_EQUAL = 371,          /* "'!='"  */
    T_IS_IDENTICAL = 372,          /* "'==='"  */
    T_IS_NOT_IDENTICAL = 373,      /* "'!=='"  */
    T_IS_SMALLER_OR_EQUAL = 374,   /* "'<='"  */
    T_IS_GREATER_OR_EQUAL = 375,   /* "'>='"  */
    T_SPACESHIP = 376,             /* "'<=>'"  */
    T_SL = 377,                    /* "'<<'"  */
    T_SR = 378,                    /* "'>>'"  */
    T_INC = 379,                   /* "'++'"  */
    T_DEC = 380,                   /* "'--'"  */
    T_INT_CAST = 381,              /* "'(int)'"  */
    T_DOUBLE_CAST = 382,           /* "'(double)'"  */
    T_STRING_CAST = 383,           /* "'(string)'"  */
    T_ARRAY_CAST = 384,            /* "'(array)'"  */
    T_OBJECT_CAST = 385,           /* "'(object)'"  */
    T_BOOL_CAST = 386,             /* "'(bool)'"  */
    T_UNSET_CAST = 387,            /* "'(unset)'"  */
    T_OBJECT_OPERATOR = 388,       /* "'->'"  */
    T_NULLSAFE_OBJECT_OPERATOR = 389, /* "'?->'"  */
    T_DOUBLE_ARROW = 390,          /* "'=>'"  */
    T_COMMENT = 391,               /* "comment"  */
    T_DOC_COMMENT = 392,           /* "doc comment"  */
    T_OPEN_TAG = 393,              /* "open tag"  */
    T_OPEN_TAG_WITH_ECHO = 394,    /* "'<?='"  */
    T_CLOSE_TAG = 395,             /* "'?>'"  */
    T_WHITESPACE = 396,            /* "whitespace"  */
    T_START_HEREDOC = 397,         /* "heredoc start"  */
    T_END_HEREDOC = 398,           /* "heredoc end"  */
    T_DOLLAR_OPEN_CURLY_BRACES = 399, /* "'${'"  */
    T_CURLY_OPEN = 400,            /* "'{$'"  */
    T_PAAMAYIM_NEKUDOTAYIM = 401,  /* "'::'"  */
    T_NS_SEPARATOR = 402,          /* "'\\'"  */
    T_ELLIPSIS = 403,              /* "'...'"  */
    T_COALESCE = 404,              /* "'??'"  */
    T_POW = 405,                   /* "'**'"  */
    T_POW_EQUAL = 406,             /* "'**='"  */
    T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 407, /* "'&'"  */
    T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 408, /* "amp"  */
    T_BAD_CHARACTER = 409,         /* "invalid character"  */
    T_ERROR = 410                  /* T_ERROR  */
  };
  typedef enum zendtokentype zendtoken_kind_t;
#endif

/* Value type.  */
#if ! defined ZENDSTYPE && ! defined ZENDSTYPE_IS_DECLARED
typedef zend_parser_stack_elem ZENDSTYPE;
# define ZENDSTYPE_IS_TRIVIAL 1
# define ZENDSTYPE_IS_DECLARED 1
#endif




ZEND_API int zendparse (void);


#endif /* !YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED  */

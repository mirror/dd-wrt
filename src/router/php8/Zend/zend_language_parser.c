/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */

#include "zend.h"
#include "zend_list.h"
#include "zend_globals.h"
#include "zend_API.h"
#include "zend_constants.h"
#include "zend_language_scanner.h"
#include "zend_exceptions.h"

#define YYSIZE_T size_t
#define yytnamerr zend_yytnamerr
static YYSIZE_T zend_yytnamerr(char*, const char*);

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

/* Substitute the type names.  */
#define YYSTYPE         ZENDSTYPE
/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yydebug         zenddebug
#define yynerrs         zendnerrs


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "zend_language_parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PREC_ARROW_FUNCTION = 3,        /* PREC_ARROW_FUNCTION  */
  YYSYMBOL_4_ = 4,                         /* '='  */
  YYSYMBOL_5_ = 5,                         /* '?'  */
  YYSYMBOL_6_ = 6,                         /* ':'  */
  YYSYMBOL_7_ = 7,                         /* '|'  */
  YYSYMBOL_8_ = 8,                         /* '^'  */
  YYSYMBOL_9_ = 9,                         /* '<'  */
  YYSYMBOL_10_ = 10,                       /* '>'  */
  YYSYMBOL_11_ = 11,                       /* '.'  */
  YYSYMBOL_12_ = 12,                       /* '+'  */
  YYSYMBOL_13_ = 13,                       /* '-'  */
  YYSYMBOL_14_ = 14,                       /* '*'  */
  YYSYMBOL_15_ = 15,                       /* '/'  */
  YYSYMBOL_16_ = 16,                       /* '%'  */
  YYSYMBOL_17_ = 17,                       /* '!'  */
  YYSYMBOL_18_ = 18,                       /* '~'  */
  YYSYMBOL_19_ = 19,                       /* '@'  */
  YYSYMBOL_T_NOELSE = 20,                  /* T_NOELSE  */
  YYSYMBOL_T_LNUMBER = 21,                 /* "integer"  */
  YYSYMBOL_T_DNUMBER = 22,                 /* "floating-point number"  */
  YYSYMBOL_T_STRING = 23,                  /* "identifier"  */
  YYSYMBOL_T_NAME_FULLY_QUALIFIED = 24,    /* "fully qualified name"  */
  YYSYMBOL_T_NAME_RELATIVE = 25,           /* "namespace-relative name"  */
  YYSYMBOL_T_NAME_QUALIFIED = 26,          /* "namespaced name"  */
  YYSYMBOL_T_VARIABLE = 27,                /* "variable"  */
  YYSYMBOL_T_INLINE_HTML = 28,             /* T_INLINE_HTML  */
  YYSYMBOL_T_ENCAPSED_AND_WHITESPACE = 29, /* "string content"  */
  YYSYMBOL_T_CONSTANT_ENCAPSED_STRING = 30, /* "quoted string"  */
  YYSYMBOL_T_STRING_VARNAME = 31,          /* "variable name"  */
  YYSYMBOL_T_NUM_STRING = 32,              /* "number"  */
  YYSYMBOL_T_INCLUDE = 33,                 /* "'include'"  */
  YYSYMBOL_T_INCLUDE_ONCE = 34,            /* "'include_once'"  */
  YYSYMBOL_T_EVAL = 35,                    /* "'eval'"  */
  YYSYMBOL_T_REQUIRE = 36,                 /* "'require'"  */
  YYSYMBOL_T_REQUIRE_ONCE = 37,            /* "'require_once'"  */
  YYSYMBOL_T_LOGICAL_OR = 38,              /* "'or'"  */
  YYSYMBOL_T_LOGICAL_XOR = 39,             /* "'xor'"  */
  YYSYMBOL_T_LOGICAL_AND = 40,             /* "'and'"  */
  YYSYMBOL_T_PRINT = 41,                   /* "'print'"  */
  YYSYMBOL_T_YIELD = 42,                   /* "'yield'"  */
  YYSYMBOL_T_YIELD_FROM = 43,              /* "'yield from'"  */
  YYSYMBOL_T_INSTANCEOF = 44,              /* "'instanceof'"  */
  YYSYMBOL_T_NEW = 45,                     /* "'new'"  */
  YYSYMBOL_T_CLONE = 46,                   /* "'clone'"  */
  YYSYMBOL_T_EXIT = 47,                    /* "'exit'"  */
  YYSYMBOL_T_IF = 48,                      /* "'if'"  */
  YYSYMBOL_T_ELSEIF = 49,                  /* "'elseif'"  */
  YYSYMBOL_T_ELSE = 50,                    /* "'else'"  */
  YYSYMBOL_T_ENDIF = 51,                   /* "'endif'"  */
  YYSYMBOL_T_ECHO = 52,                    /* "'echo'"  */
  YYSYMBOL_T_DO = 53,                      /* "'do'"  */
  YYSYMBOL_T_WHILE = 54,                   /* "'while'"  */
  YYSYMBOL_T_ENDWHILE = 55,                /* "'endwhile'"  */
  YYSYMBOL_T_FOR = 56,                     /* "'for'"  */
  YYSYMBOL_T_ENDFOR = 57,                  /* "'endfor'"  */
  YYSYMBOL_T_FOREACH = 58,                 /* "'foreach'"  */
  YYSYMBOL_T_ENDFOREACH = 59,              /* "'endforeach'"  */
  YYSYMBOL_T_DECLARE = 60,                 /* "'declare'"  */
  YYSYMBOL_T_ENDDECLARE = 61,              /* "'enddeclare'"  */
  YYSYMBOL_T_AS = 62,                      /* "'as'"  */
  YYSYMBOL_T_SWITCH = 63,                  /* "'switch'"  */
  YYSYMBOL_T_ENDSWITCH = 64,               /* "'endswitch'"  */
  YYSYMBOL_T_CASE = 65,                    /* "'case'"  */
  YYSYMBOL_T_DEFAULT = 66,                 /* "'default'"  */
  YYSYMBOL_T_MATCH = 67,                   /* "'match'"  */
  YYSYMBOL_T_BREAK = 68,                   /* "'break'"  */
  YYSYMBOL_T_CONTINUE = 69,                /* "'continue'"  */
  YYSYMBOL_T_GOTO = 70,                    /* "'goto'"  */
  YYSYMBOL_T_FUNCTION = 71,                /* "'function'"  */
  YYSYMBOL_T_FN = 72,                      /* "'fn'"  */
  YYSYMBOL_T_CONST = 73,                   /* "'const'"  */
  YYSYMBOL_T_RETURN = 74,                  /* "'return'"  */
  YYSYMBOL_T_TRY = 75,                     /* "'try'"  */
  YYSYMBOL_T_CATCH = 76,                   /* "'catch'"  */
  YYSYMBOL_T_FINALLY = 77,                 /* "'finally'"  */
  YYSYMBOL_T_THROW = 78,                   /* "'throw'"  */
  YYSYMBOL_T_USE = 79,                     /* "'use'"  */
  YYSYMBOL_T_INSTEADOF = 80,               /* "'insteadof'"  */
  YYSYMBOL_T_GLOBAL = 81,                  /* "'global'"  */
  YYSYMBOL_T_STATIC = 82,                  /* "'static'"  */
  YYSYMBOL_T_ABSTRACT = 83,                /* "'abstract'"  */
  YYSYMBOL_T_FINAL = 84,                   /* "'final'"  */
  YYSYMBOL_T_PRIVATE = 85,                 /* "'private'"  */
  YYSYMBOL_T_PROTECTED = 86,               /* "'protected'"  */
  YYSYMBOL_T_PUBLIC = 87,                  /* "'public'"  */
  YYSYMBOL_T_PRIVATE_SET = 88,             /* "'private(set)'"  */
  YYSYMBOL_T_PROTECTED_SET = 89,           /* "'protected(set)'"  */
  YYSYMBOL_T_PUBLIC_SET = 90,              /* "'public(set)'"  */
  YYSYMBOL_T_READONLY = 91,                /* "'readonly'"  */
  YYSYMBOL_T_VAR = 92,                     /* "'var'"  */
  YYSYMBOL_T_UNSET = 93,                   /* "'unset'"  */
  YYSYMBOL_T_ISSET = 94,                   /* "'isset'"  */
  YYSYMBOL_T_EMPTY = 95,                   /* "'empty'"  */
  YYSYMBOL_T_HALT_COMPILER = 96,           /* "'__halt_compiler'"  */
  YYSYMBOL_T_CLASS = 97,                   /* "'class'"  */
  YYSYMBOL_T_TRAIT = 98,                   /* "'trait'"  */
  YYSYMBOL_T_INTERFACE = 99,               /* "'interface'"  */
  YYSYMBOL_T_ENUM = 100,                   /* "'enum'"  */
  YYSYMBOL_T_EXTENDS = 101,                /* "'extends'"  */
  YYSYMBOL_T_IMPLEMENTS = 102,             /* "'implements'"  */
  YYSYMBOL_T_NAMESPACE = 103,              /* "'namespace'"  */
  YYSYMBOL_T_LIST = 104,                   /* "'list'"  */
  YYSYMBOL_T_ARRAY = 105,                  /* "'array'"  */
  YYSYMBOL_T_CALLABLE = 106,               /* "'callable'"  */
  YYSYMBOL_T_LINE = 107,                   /* "'__LINE__'"  */
  YYSYMBOL_T_FILE = 108,                   /* "'__FILE__'"  */
  YYSYMBOL_T_DIR = 109,                    /* "'__DIR__'"  */
  YYSYMBOL_T_CLASS_C = 110,                /* "'__CLASS__'"  */
  YYSYMBOL_T_TRAIT_C = 111,                /* "'__TRAIT__'"  */
  YYSYMBOL_T_METHOD_C = 112,               /* "'__METHOD__'"  */
  YYSYMBOL_T_FUNC_C = 113,                 /* "'__FUNCTION__'"  */
  YYSYMBOL_T_PROPERTY_C = 114,             /* "'__PROPERTY__'"  */
  YYSYMBOL_T_NS_C = 115,                   /* "'__NAMESPACE__'"  */
  YYSYMBOL_T_ATTRIBUTE = 116,              /* "'#['"  */
  YYSYMBOL_T_PLUS_EQUAL = 117,             /* "'+='"  */
  YYSYMBOL_T_MINUS_EQUAL = 118,            /* "'-='"  */
  YYSYMBOL_T_MUL_EQUAL = 119,              /* "'*='"  */
  YYSYMBOL_T_DIV_EQUAL = 120,              /* "'/='"  */
  YYSYMBOL_T_CONCAT_EQUAL = 121,           /* "'.='"  */
  YYSYMBOL_T_MOD_EQUAL = 122,              /* "'%='"  */
  YYSYMBOL_T_AND_EQUAL = 123,              /* "'&='"  */
  YYSYMBOL_T_OR_EQUAL = 124,               /* "'|='"  */
  YYSYMBOL_T_XOR_EQUAL = 125,              /* "'^='"  */
  YYSYMBOL_T_SL_EQUAL = 126,               /* "'<<='"  */
  YYSYMBOL_T_SR_EQUAL = 127,               /* "'>>='"  */
  YYSYMBOL_T_COALESCE_EQUAL = 128,         /* "'??='"  */
  YYSYMBOL_T_BOOLEAN_OR = 129,             /* "'||'"  */
  YYSYMBOL_T_BOOLEAN_AND = 130,            /* "'&&'"  */
  YYSYMBOL_T_IS_EQUAL = 131,               /* "'=='"  */
  YYSYMBOL_T_IS_NOT_EQUAL = 132,           /* "'!='"  */
  YYSYMBOL_T_IS_IDENTICAL = 133,           /* "'==='"  */
  YYSYMBOL_T_IS_NOT_IDENTICAL = 134,       /* "'!=='"  */
  YYSYMBOL_T_IS_SMALLER_OR_EQUAL = 135,    /* "'<='"  */
  YYSYMBOL_T_IS_GREATER_OR_EQUAL = 136,    /* "'>='"  */
  YYSYMBOL_T_SPACESHIP = 137,              /* "'<=>'"  */
  YYSYMBOL_T_SL = 138,                     /* "'<<'"  */
  YYSYMBOL_T_SR = 139,                     /* "'>>'"  */
  YYSYMBOL_T_INC = 140,                    /* "'++'"  */
  YYSYMBOL_T_DEC = 141,                    /* "'--'"  */
  YYSYMBOL_T_INT_CAST = 142,               /* "'(int)'"  */
  YYSYMBOL_T_DOUBLE_CAST = 143,            /* "'(double)'"  */
  YYSYMBOL_T_STRING_CAST = 144,            /* "'(string)'"  */
  YYSYMBOL_T_ARRAY_CAST = 145,             /* "'(array)'"  */
  YYSYMBOL_T_OBJECT_CAST = 146,            /* "'(object)'"  */
  YYSYMBOL_T_BOOL_CAST = 147,              /* "'(bool)'"  */
  YYSYMBOL_T_UNSET_CAST = 148,             /* "'(unset)'"  */
  YYSYMBOL_T_OBJECT_OPERATOR = 149,        /* "'->'"  */
  YYSYMBOL_T_NULLSAFE_OBJECT_OPERATOR = 150, /* "'?->'"  */
  YYSYMBOL_T_DOUBLE_ARROW = 151,           /* "'=>'"  */
  YYSYMBOL_T_COMMENT = 152,                /* "comment"  */
  YYSYMBOL_T_DOC_COMMENT = 153,            /* "doc comment"  */
  YYSYMBOL_T_OPEN_TAG = 154,               /* "open tag"  */
  YYSYMBOL_T_OPEN_TAG_WITH_ECHO = 155,     /* "'<?='"  */
  YYSYMBOL_T_CLOSE_TAG = 156,              /* "'?>'"  */
  YYSYMBOL_T_WHITESPACE = 157,             /* "whitespace"  */
  YYSYMBOL_T_START_HEREDOC = 158,          /* "heredoc start"  */
  YYSYMBOL_T_END_HEREDOC = 159,            /* "heredoc end"  */
  YYSYMBOL_T_DOLLAR_OPEN_CURLY_BRACES = 160, /* "'${'"  */
  YYSYMBOL_T_CURLY_OPEN = 161,             /* "'{$'"  */
  YYSYMBOL_T_PAAMAYIM_NEKUDOTAYIM = 162,   /* "'::'"  */
  YYSYMBOL_T_NS_SEPARATOR = 163,           /* "'\\'"  */
  YYSYMBOL_T_ELLIPSIS = 164,               /* "'...'"  */
  YYSYMBOL_T_COALESCE = 165,               /* "'??'"  */
  YYSYMBOL_T_POW = 166,                    /* "'**'"  */
  YYSYMBOL_T_POW_EQUAL = 167,              /* "'**='"  */
  YYSYMBOL_T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 168, /* "'&'"  */
  YYSYMBOL_T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 169, /* "amp"  */
  YYSYMBOL_T_BAD_CHARACTER = 170,          /* "invalid character"  */
  YYSYMBOL_T_ERROR = 171,                  /* T_ERROR  */
  YYSYMBOL_172_ = 172,                     /* ','  */
  YYSYMBOL_173_ = 173,                     /* ']'  */
  YYSYMBOL_174_ = 174,                     /* '('  */
  YYSYMBOL_175_ = 175,                     /* ')'  */
  YYSYMBOL_176_ = 176,                     /* ';'  */
  YYSYMBOL_177_ = 177,                     /* '{'  */
  YYSYMBOL_178_ = 178,                     /* '}'  */
  YYSYMBOL_179_ = 179,                     /* '['  */
  YYSYMBOL_180_ = 180,                     /* '`'  */
  YYSYMBOL_181_ = 181,                     /* '"'  */
  YYSYMBOL_182_ = 182,                     /* '$'  */
  YYSYMBOL_YYACCEPT = 183,                 /* $accept  */
  YYSYMBOL_start = 184,                    /* start  */
  YYSYMBOL_reserved_non_modifiers = 185,   /* reserved_non_modifiers  */
  YYSYMBOL_semi_reserved = 186,            /* semi_reserved  */
  YYSYMBOL_ampersand = 187,                /* ampersand  */
  YYSYMBOL_identifier = 188,               /* identifier  */
  YYSYMBOL_top_statement_list = 189,       /* top_statement_list  */
  YYSYMBOL_namespace_declaration_name = 190, /* namespace_declaration_name  */
  YYSYMBOL_namespace_name = 191,           /* namespace_name  */
  YYSYMBOL_legacy_namespace_name = 192,    /* legacy_namespace_name  */
  YYSYMBOL_name = 193,                     /* name  */
  YYSYMBOL_attribute_decl = 194,           /* attribute_decl  */
  YYSYMBOL_attribute_group = 195,          /* attribute_group  */
  YYSYMBOL_attribute = 196,                /* attribute  */
  YYSYMBOL_attributes = 197,               /* attributes  */
  YYSYMBOL_attributed_statement = 198,     /* attributed_statement  */
  YYSYMBOL_top_statement = 199,            /* top_statement  */
  YYSYMBOL_200_1 = 200,                    /* $@1  */
  YYSYMBOL_201_2 = 201,                    /* $@2  */
  YYSYMBOL_use_type = 202,                 /* use_type  */
  YYSYMBOL_group_use_declaration = 203,    /* group_use_declaration  */
  YYSYMBOL_mixed_group_use_declaration = 204, /* mixed_group_use_declaration  */
  YYSYMBOL_possible_comma = 205,           /* possible_comma  */
  YYSYMBOL_inline_use_declarations = 206,  /* inline_use_declarations  */
  YYSYMBOL_unprefixed_use_declarations = 207, /* unprefixed_use_declarations  */
  YYSYMBOL_use_declarations = 208,         /* use_declarations  */
  YYSYMBOL_inline_use_declaration = 209,   /* inline_use_declaration  */
  YYSYMBOL_unprefixed_use_declaration = 210, /* unprefixed_use_declaration  */
  YYSYMBOL_use_declaration = 211,          /* use_declaration  */
  YYSYMBOL_const_list = 212,               /* const_list  */
  YYSYMBOL_inner_statement_list = 213,     /* inner_statement_list  */
  YYSYMBOL_inner_statement = 214,          /* inner_statement  */
  YYSYMBOL_statement = 215,                /* statement  */
  YYSYMBOL_216_3 = 216,                    /* $@3  */
  YYSYMBOL_catch_list = 217,               /* catch_list  */
  YYSYMBOL_catch_name_list = 218,          /* catch_name_list  */
  YYSYMBOL_optional_variable = 219,        /* optional_variable  */
  YYSYMBOL_finally_statement = 220,        /* finally_statement  */
  YYSYMBOL_unset_variables = 221,          /* unset_variables  */
  YYSYMBOL_unset_variable = 222,           /* unset_variable  */
  YYSYMBOL_function_name = 223,            /* function_name  */
  YYSYMBOL_function_declaration_statement = 224, /* function_declaration_statement  */
  YYSYMBOL_is_reference = 225,             /* is_reference  */
  YYSYMBOL_is_variadic = 226,              /* is_variadic  */
  YYSYMBOL_class_declaration_statement = 227, /* class_declaration_statement  */
  YYSYMBOL_228_4 = 228,                    /* @4  */
  YYSYMBOL_229_5 = 229,                    /* @5  */
  YYSYMBOL_class_modifiers = 230,          /* class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers = 231, /* anonymous_class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers_optional = 232, /* anonymous_class_modifiers_optional  */
  YYSYMBOL_class_modifier = 233,           /* class_modifier  */
  YYSYMBOL_trait_declaration_statement = 234, /* trait_declaration_statement  */
  YYSYMBOL_235_6 = 235,                    /* @6  */
  YYSYMBOL_interface_declaration_statement = 236, /* interface_declaration_statement  */
  YYSYMBOL_237_7 = 237,                    /* @7  */
  YYSYMBOL_enum_declaration_statement = 238, /* enum_declaration_statement  */
  YYSYMBOL_239_8 = 239,                    /* @8  */
  YYSYMBOL_enum_backing_type = 240,        /* enum_backing_type  */
  YYSYMBOL_enum_case = 241,                /* enum_case  */
  YYSYMBOL_enum_case_expr = 242,           /* enum_case_expr  */
  YYSYMBOL_extends_from = 243,             /* extends_from  */
  YYSYMBOL_interface_extends_list = 244,   /* interface_extends_list  */
  YYSYMBOL_implements_list = 245,          /* implements_list  */
  YYSYMBOL_foreach_variable = 246,         /* foreach_variable  */
  YYSYMBOL_for_statement = 247,            /* for_statement  */
  YYSYMBOL_foreach_statement = 248,        /* foreach_statement  */
  YYSYMBOL_declare_statement = 249,        /* declare_statement  */
  YYSYMBOL_switch_case_list = 250,         /* switch_case_list  */
  YYSYMBOL_case_list = 251,                /* case_list  */
  YYSYMBOL_case_separator = 252,           /* case_separator  */
  YYSYMBOL_match = 253,                    /* match  */
  YYSYMBOL_match_arm_list = 254,           /* match_arm_list  */
  YYSYMBOL_non_empty_match_arm_list = 255, /* non_empty_match_arm_list  */
  YYSYMBOL_match_arm = 256,                /* match_arm  */
  YYSYMBOL_match_arm_cond_list = 257,      /* match_arm_cond_list  */
  YYSYMBOL_while_statement = 258,          /* while_statement  */
  YYSYMBOL_if_stmt_without_else = 259,     /* if_stmt_without_else  */
  YYSYMBOL_if_stmt = 260,                  /* if_stmt  */
  YYSYMBOL_alt_if_stmt_without_else = 261, /* alt_if_stmt_without_else  */
  YYSYMBOL_alt_if_stmt = 262,              /* alt_if_stmt  */
  YYSYMBOL_parameter_list = 263,           /* parameter_list  */
  YYSYMBOL_non_empty_parameter_list = 264, /* non_empty_parameter_list  */
  YYSYMBOL_attributed_parameter = 265,     /* attributed_parameter  */
  YYSYMBOL_optional_cpp_modifiers = 266,   /* optional_cpp_modifiers  */
  YYSYMBOL_parameter = 267,                /* parameter  */
  YYSYMBOL_optional_type_without_static = 268, /* optional_type_without_static  */
  YYSYMBOL_type_expr = 269,                /* type_expr  */
  YYSYMBOL_type = 270,                     /* type  */
  YYSYMBOL_union_type_element = 271,       /* union_type_element  */
  YYSYMBOL_union_type = 272,               /* union_type  */
  YYSYMBOL_intersection_type = 273,        /* intersection_type  */
  YYSYMBOL_type_expr_without_static = 274, /* type_expr_without_static  */
  YYSYMBOL_type_without_static = 275,      /* type_without_static  */
  YYSYMBOL_union_type_without_static_element = 276, /* union_type_without_static_element  */
  YYSYMBOL_union_type_without_static = 277, /* union_type_without_static  */
  YYSYMBOL_intersection_type_without_static = 278, /* intersection_type_without_static  */
  YYSYMBOL_return_type = 279,              /* return_type  */
  YYSYMBOL_argument_list = 280,            /* argument_list  */
  YYSYMBOL_non_empty_argument_list = 281,  /* non_empty_argument_list  */
  YYSYMBOL_argument = 282,                 /* argument  */
  YYSYMBOL_global_var_list = 283,          /* global_var_list  */
  YYSYMBOL_global_var = 284,               /* global_var  */
  YYSYMBOL_static_var_list = 285,          /* static_var_list  */
  YYSYMBOL_static_var = 286,               /* static_var  */
  YYSYMBOL_class_statement_list = 287,     /* class_statement_list  */
  YYSYMBOL_attributed_class_statement = 288, /* attributed_class_statement  */
  YYSYMBOL_class_statement = 289,          /* class_statement  */
  YYSYMBOL_class_name_list = 290,          /* class_name_list  */
  YYSYMBOL_trait_adaptations = 291,        /* trait_adaptations  */
  YYSYMBOL_trait_adaptation_list = 292,    /* trait_adaptation_list  */
  YYSYMBOL_trait_adaptation = 293,         /* trait_adaptation  */
  YYSYMBOL_trait_precedence = 294,         /* trait_precedence  */
  YYSYMBOL_trait_alias = 295,              /* trait_alias  */
  YYSYMBOL_trait_method_reference = 296,   /* trait_method_reference  */
  YYSYMBOL_absolute_trait_method_reference = 297, /* absolute_trait_method_reference  */
  YYSYMBOL_method_body = 298,              /* method_body  */
  YYSYMBOL_property_modifiers = 299,       /* property_modifiers  */
  YYSYMBOL_method_modifiers = 300,         /* method_modifiers  */
  YYSYMBOL_class_const_modifiers = 301,    /* class_const_modifiers  */
  YYSYMBOL_non_empty_member_modifiers = 302, /* non_empty_member_modifiers  */
  YYSYMBOL_member_modifier = 303,          /* member_modifier  */
  YYSYMBOL_property_list = 304,            /* property_list  */
  YYSYMBOL_property = 305,                 /* property  */
  YYSYMBOL_hooked_property = 306,          /* hooked_property  */
  YYSYMBOL_property_hook_list = 307,       /* property_hook_list  */
  YYSYMBOL_optional_property_hook_list = 308, /* optional_property_hook_list  */
  YYSYMBOL_property_hook_modifiers = 309,  /* property_hook_modifiers  */
  YYSYMBOL_property_hook = 310,            /* property_hook  */
  YYSYMBOL_311_9 = 311,                    /* @9  */
  YYSYMBOL_property_hook_body = 312,       /* property_hook_body  */
  YYSYMBOL_optional_parameter_list = 313,  /* optional_parameter_list  */
  YYSYMBOL_class_const_list = 314,         /* class_const_list  */
  YYSYMBOL_class_const_decl = 315,         /* class_const_decl  */
  YYSYMBOL_const_decl = 316,               /* const_decl  */
  YYSYMBOL_echo_expr_list = 317,           /* echo_expr_list  */
  YYSYMBOL_echo_expr = 318,                /* echo_expr  */
  YYSYMBOL_for_exprs = 319,                /* for_exprs  */
  YYSYMBOL_non_empty_for_exprs = 320,      /* non_empty_for_exprs  */
  YYSYMBOL_anonymous_class = 321,          /* anonymous_class  */
  YYSYMBOL_322_10 = 322,                   /* @10  */
  YYSYMBOL_new_dereferenceable = 323,      /* new_dereferenceable  */
  YYSYMBOL_new_non_dereferenceable = 324,  /* new_non_dereferenceable  */
  YYSYMBOL_expr = 325,                     /* expr  */
  YYSYMBOL_inline_function = 326,          /* inline_function  */
  YYSYMBOL_fn = 327,                       /* fn  */
  YYSYMBOL_function = 328,                 /* function  */
  YYSYMBOL_backup_doc_comment = 329,       /* backup_doc_comment  */
  YYSYMBOL_backup_fn_flags = 330,          /* backup_fn_flags  */
  YYSYMBOL_backup_lex_pos = 331,           /* backup_lex_pos  */
  YYSYMBOL_returns_ref = 332,              /* returns_ref  */
  YYSYMBOL_lexical_vars = 333,             /* lexical_vars  */
  YYSYMBOL_lexical_var_list = 334,         /* lexical_var_list  */
  YYSYMBOL_lexical_var = 335,              /* lexical_var  */
  YYSYMBOL_function_call = 336,            /* function_call  */
  YYSYMBOL_337_11 = 337,                   /* @11  */
  YYSYMBOL_class_name = 338,               /* class_name  */
  YYSYMBOL_class_name_reference = 339,     /* class_name_reference  */
  YYSYMBOL_backticks_expr = 340,           /* backticks_expr  */
  YYSYMBOL_ctor_arguments = 341,           /* ctor_arguments  */
  YYSYMBOL_dereferenceable_scalar = 342,   /* dereferenceable_scalar  */
  YYSYMBOL_scalar = 343,                   /* scalar  */
  YYSYMBOL_constant = 344,                 /* constant  */
  YYSYMBOL_class_constant = 345,           /* class_constant  */
  YYSYMBOL_optional_expr = 346,            /* optional_expr  */
  YYSYMBOL_variable_class_name = 347,      /* variable_class_name  */
  YYSYMBOL_fully_dereferenceable = 348,    /* fully_dereferenceable  */
  YYSYMBOL_array_object_dereferenceable = 349, /* array_object_dereferenceable  */
  YYSYMBOL_callable_expr = 350,            /* callable_expr  */
  YYSYMBOL_callable_variable = 351,        /* callable_variable  */
  YYSYMBOL_variable = 352,                 /* variable  */
  YYSYMBOL_simple_variable = 353,          /* simple_variable  */
  YYSYMBOL_static_member = 354,            /* static_member  */
  YYSYMBOL_new_variable = 355,             /* new_variable  */
  YYSYMBOL_member_name = 356,              /* member_name  */
  YYSYMBOL_property_name = 357,            /* property_name  */
  YYSYMBOL_array_pair_list = 358,          /* array_pair_list  */
  YYSYMBOL_possible_array_pair = 359,      /* possible_array_pair  */
  YYSYMBOL_non_empty_array_pair_list = 360, /* non_empty_array_pair_list  */
  YYSYMBOL_array_pair = 361,               /* array_pair  */
  YYSYMBOL_encaps_list = 362,              /* encaps_list  */
  YYSYMBOL_encaps_var = 363,               /* encaps_var  */
  YYSYMBOL_encaps_var_offset = 364,        /* encaps_var_offset  */
  YYSYMBOL_internal_functions_in_yacc = 365, /* internal_functions_in_yacc  */
  YYSYMBOL_isset_variables = 366,          /* isset_variables  */
  YYSYMBOL_isset_variable = 367            /* isset_variable  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ZENDSTYPE_IS_TRIVIAL && ZENDSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   10316

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  183
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  185
/* YYNRULES -- Number of rules.  */
#define YYNRULES  618
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1172

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   410


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    17,   181,     2,   182,    16,     2,     2,
     174,   175,    14,    12,   172,    13,    11,    15,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,   176,
       9,     4,    10,     5,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   179,     2,   173,     8,     2,   180,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   177,     7,   178,    18,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171
};

#if ZENDDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   301,   301,   305,   305,   305,   305,   305,   305,   305,
     305,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   308,   308,   308,   308,   308,   308,   308,
     308,   308,   308,   309,   309,   309,   309,   309,   309,   309,
     309,   309,   309,   309,   310,   310,   310,   310,   310,   310,
     310,   310,   311,   311,   311,   311,   311,   311,   311,   311,
     311,   311,   311,   312,   316,   317,   317,   317,   317,   317,
     317,   317,   321,   322,   326,   327,   335,   336,   341,   342,
     347,   348,   353,   354,   358,   359,   360,   361,   365,   367,
     372,   374,   379,   383,   384,   388,   389,   390,   391,   392,
     396,   397,   398,   399,   403,   406,   406,   409,   409,   412,
     413,   414,   415,   416,   420,   421,   425,   430,   435,   436,
     440,   442,   447,   449,   454,   456,   461,   462,   466,   468,
     473,   475,   480,   481,   485,   487,   493,   494,   495,   496,
     503,   504,   505,   506,   508,   510,   512,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   525,   529,   528,
     532,   533,   535,   536,   540,   542,   547,   548,   552,   553,
     557,   558,   562,   563,   567,   571,   572,   580,   587,   588,
     592,   593,   597,   597,   600,   600,   606,   607,   612,   614,
     619,   620,   624,   625,   626,   630,   630,   636,   636,   642,
     642,   648,   649,   653,   658,   659,   663,   664,   668,   669,
     673,   674,   678,   679,   680,   681,   685,   686,   690,   691,
     695,   696,   700,   701,   702,   703,   707,   708,   710,   715,
     716,   721,   726,   727,   731,   732,   736,   738,   743,   744,
     749,   750,   755,   758,   764,   765,   770,   773,   779,   780,
     786,   787,   792,   794,   799,   800,   804,   806,   812,   816,
     823,   824,   828,   829,   830,   831,   835,   836,   840,   841,
     845,   847,   852,   853,   860,   861,   862,   863,   867,   868,
     869,   873,   874,   878,   880,   885,   887,   892,   893,   897,
     898,   899,   903,   905,   910,   911,   913,   917,   918,   922,
     928,   929,   933,   934,   938,   940,   946,   949,   952,   955,
     958,   962,   966,   967,   968,   973,   974,   978,   979,   980,
     984,   986,   991,   992,   996,  1001,  1003,  1007,  1012,  1020,
    1022,  1026,  1031,  1032,  1036,  1039,  1044,  1046,  1053,  1055,
    1062,  1064,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1082,  1083,  1087,  1089,  1094,  1096,  1101,  1102,
    1103,  1109,  1110,  1114,  1115,  1123,  1122,  1133,  1134,  1135,
    1140,  1141,  1145,  1146,  1150,  1151,  1159,  1163,  1164,  1167,
    1171,  1172,  1176,  1177,  1181,  1181,  1191,  1193,  1195,  1200,
    1205,  1207,  1209,  1211,  1213,  1215,  1216,  1218,  1220,  1222,
    1224,  1226,  1228,  1230,  1232,  1234,  1236,  1238,  1240,  1242,
    1243,  1244,  1245,  1246,  1248,  1250,  1252,  1254,  1256,  1257,
    1258,  1259,  1260,  1261,  1262,  1263,  1264,  1265,  1266,  1267,
    1268,  1269,  1270,  1271,  1272,  1273,  1275,  1277,  1279,  1281,
    1283,  1285,  1287,  1289,  1291,  1293,  1297,  1298,  1299,  1301,
    1303,  1305,  1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,
    1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,  1327,
    1328,  1329,  1330,  1332,  1337,  1342,  1350,  1354,  1358,  1362,
    1366,  1370,  1371,  1375,  1376,  1380,  1381,  1385,  1386,  1390,
    1392,  1397,  1399,  1401,  1401,  1408,  1411,  1415,  1416,  1417,
    1421,  1423,  1424,  1429,  1430,  1435,  1436,  1437,  1438,  1442,
    1443,  1444,  1445,  1447,  1448,  1449,  1450,  1454,  1455,  1456,
    1457,  1458,  1459,  1460,  1461,  1462,  1463,  1467,  1469,  1471,
    1473,  1478,  1479,  1483,  1487,  1488,  1492,  1493,  1494,  1498,
    1499,  1503,  1504,  1505,  1506,  1510,  1512,  1514,  1516,  1518,
    1522,  1524,  1526,  1528,  1533,  1534,  1535,  1539,  1541,  1546,
    1548,  1550,  1552,  1554,  1556,  1561,  1562,  1563,  1567,  1568,
    1569,  1573,  1578,  1579,  1583,  1585,  1590,  1592,  1594,  1596,
    1598,  1600,  1603,  1609,  1611,  1613,  1615,  1620,  1622,  1625,
    1628,  1631,  1633,  1635,  1638,  1642,  1643,  1644,  1645,  1650,
    1651,  1652,  1654,  1656,  1658,  1660,  1665,  1666,  1671
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PREC_ARROW_FUNCTION",
  "'='", "'?'", "':'", "'|'", "'^'", "'<'", "'>'", "'.'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "'@'", "T_NOELSE", "\"integer\"",
  "\"floating-point number\"", "\"identifier\"",
  "\"fully qualified name\"", "\"namespace-relative name\"",
  "\"namespaced name\"", "\"variable\"", "T_INLINE_HTML",
  "\"string content\"", "\"quoted string\"", "\"variable name\"",
  "\"number\"", "\"'include'\"", "\"'include_once'\"", "\"'eval'\"",
  "\"'require'\"", "\"'require_once'\"", "\"'or'\"", "\"'xor'\"",
  "\"'and'\"", "\"'print'\"", "\"'yield'\"", "\"'yield from'\"",
  "\"'instanceof'\"", "\"'new'\"", "\"'clone'\"", "\"'exit'\"", "\"'if'\"",
  "\"'elseif'\"", "\"'else'\"", "\"'endif'\"", "\"'echo'\"", "\"'do'\"",
  "\"'while'\"", "\"'endwhile'\"", "\"'for'\"", "\"'endfor'\"",
  "\"'foreach'\"", "\"'endforeach'\"", "\"'declare'\"", "\"'enddeclare'\"",
  "\"'as'\"", "\"'switch'\"", "\"'endswitch'\"", "\"'case'\"",
  "\"'default'\"", "\"'match'\"", "\"'break'\"", "\"'continue'\"",
  "\"'goto'\"", "\"'function'\"", "\"'fn'\"", "\"'const'\"",
  "\"'return'\"", "\"'try'\"", "\"'catch'\"", "\"'finally'\"",
  "\"'throw'\"", "\"'use'\"", "\"'insteadof'\"", "\"'global'\"",
  "\"'static'\"", "\"'abstract'\"", "\"'final'\"", "\"'private'\"",
  "\"'protected'\"", "\"'public'\"", "\"'private(set)'\"",
  "\"'protected(set)'\"", "\"'public(set)'\"", "\"'readonly'\"",
  "\"'var'\"", "\"'unset'\"", "\"'isset'\"", "\"'empty'\"",
  "\"'__halt_compiler'\"", "\"'class'\"", "\"'trait'\"", "\"'interface'\"",
  "\"'enum'\"", "\"'extends'\"", "\"'implements'\"", "\"'namespace'\"",
  "\"'list'\"", "\"'array'\"", "\"'callable'\"", "\"'__LINE__'\"",
  "\"'__FILE__'\"", "\"'__DIR__'\"", "\"'__CLASS__'\"", "\"'__TRAIT__'\"",
  "\"'__METHOD__'\"", "\"'__FUNCTION__'\"", "\"'__PROPERTY__'\"",
  "\"'__NAMESPACE__'\"", "\"'#['\"", "\"'+='\"", "\"'-='\"", "\"'*='\"",
  "\"'/='\"", "\"'.='\"", "\"'%='\"", "\"'&='\"", "\"'|='\"", "\"'^='\"",
  "\"'<<='\"", "\"'>>='\"", "\"'?""?='\"", "\"'||'\"", "\"'&&'\"",
  "\"'=='\"", "\"'!='\"", "\"'==='\"", "\"'!=='\"", "\"'<='\"", "\"'>='\"",
  "\"'<=>'\"", "\"'<<'\"", "\"'>>'\"", "\"'++'\"", "\"'--'\"",
  "\"'(int)'\"", "\"'(double)'\"", "\"'(string)'\"", "\"'(array)'\"",
  "\"'(object)'\"", "\"'(bool)'\"", "\"'(unset)'\"", "\"'->'\"",
  "\"'?->'\"", "\"'=>'\"", "\"comment\"", "\"doc comment\"",
  "\"open tag\"", "\"'<?='\"", "\"'?>'\"", "\"whitespace\"",
  "\"heredoc start\"", "\"heredoc end\"", "\"'${'\"", "\"'{$'\"",
  "\"'::'\"", "\"'\\\\'\"", "\"'...'\"", "\"'?""?'\"", "\"'**'\"",
  "\"'**='\"", "\"'&'\"", "\"amp\"", "\"invalid character\"", "T_ERROR",
  "','", "']'", "'('", "')'", "';'", "'{'", "'}'", "'['", "'`'", "'\"'",
  "'$'", "$accept", "start", "reserved_non_modifiers", "semi_reserved",
  "ampersand", "identifier", "top_statement_list",
  "namespace_declaration_name", "namespace_name", "legacy_namespace_name",
  "name", "attribute_decl", "attribute_group", "attribute", "attributes",
  "attributed_statement", "top_statement", "$@1", "$@2", "use_type",
  "group_use_declaration", "mixed_group_use_declaration", "possible_comma",
  "inline_use_declarations", "unprefixed_use_declarations",
  "use_declarations", "inline_use_declaration",
  "unprefixed_use_declaration", "use_declaration", "const_list",
  "inner_statement_list", "inner_statement", "statement", "$@3",
  "catch_list", "catch_name_list", "optional_variable",
  "finally_statement", "unset_variables", "unset_variable",
  "function_name", "function_declaration_statement", "is_reference",
  "is_variadic", "class_declaration_statement", "@4", "@5",
  "class_modifiers", "anonymous_class_modifiers",
  "anonymous_class_modifiers_optional", "class_modifier",
  "trait_declaration_statement", "@6", "interface_declaration_statement",
  "@7", "enum_declaration_statement", "@8", "enum_backing_type",
  "enum_case", "enum_case_expr", "extends_from", "interface_extends_list",
  "implements_list", "foreach_variable", "for_statement",
  "foreach_statement", "declare_statement", "switch_case_list",
  "case_list", "case_separator", "match", "match_arm_list",
  "non_empty_match_arm_list", "match_arm", "match_arm_cond_list",
  "while_statement", "if_stmt_without_else", "if_stmt",
  "alt_if_stmt_without_else", "alt_if_stmt", "parameter_list",
  "non_empty_parameter_list", "attributed_parameter",
  "optional_cpp_modifiers", "parameter", "optional_type_without_static",
  "type_expr", "type", "union_type_element", "union_type",
  "intersection_type", "type_expr_without_static", "type_without_static",
  "union_type_without_static_element", "union_type_without_static",
  "intersection_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "attributed_class_statement", "class_statement", "class_name_list",
  "trait_adaptations", "trait_adaptation_list", "trait_adaptation",
  "trait_precedence", "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "property_modifiers",
  "method_modifiers", "class_const_modifiers",
  "non_empty_member_modifiers", "member_modifier", "property_list",
  "property", "hooked_property", "property_hook_list",
  "optional_property_hook_list", "property_hook_modifiers",
  "property_hook", "@9", "property_hook_body", "optional_parameter_list",
  "class_const_list", "class_const_decl", "const_decl", "echo_expr_list",
  "echo_expr", "for_exprs", "non_empty_for_exprs", "anonymous_class",
  "@10", "new_dereferenceable", "new_non_dereferenceable", "expr",
  "inline_function", "fn", "function", "backup_doc_comment",
  "backup_fn_flags", "backup_lex_pos", "returns_ref", "lexical_vars",
  "lexical_var_list", "lexical_var", "function_call", "@11", "class_name",
  "class_name_reference", "backticks_expr", "ctor_arguments",
  "dereferenceable_scalar", "scalar", "constant", "class_constant",
  "optional_expr", "variable_class_name", "fully_dereferenceable",
  "array_object_dereferenceable", "callable_expr", "callable_variable",
  "variable", "simple_variable", "static_member", "new_variable",
  "member_name", "property_name", "array_pair_list", "possible_array_pair",
  "non_empty_array_pair_list", "array_pair", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions_in_yacc", "isset_variables",
  "isset_variable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-1034)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-590)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1034,    75,  3873, -1034,  7635,  7635,  7635,  7635,  7635, -1034,
   -1034,    84, -1034, -1034, -1034, -1034, -1034, -1034,  7635,  7635,
     -23,  7635,  7635,  7635,  7635,  7635,  1261,  7635,   -21,     8,
    7635,  6438,    20,    36,    38,    65,    70,    93,  7635,  7635,
     138, -1034, -1034,   151,  7635,   122,  7635,   242,     6,    55,
   -1034, -1034,   -21,   108,   131,   154,   157, -1034, -1034, -1034,
   -1034, 10015,   159,   164, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034,   331,  9333,  9333,  7635,  7635,  7635,  7635,
    7635,  7635,  7635,   225,  7635, -1034, -1034,  6609,    72,   207,
      37,   -34, -1034,   980, -1034, -1034, -1034, -1034, -1034,   554,
   -1034, -1034, -1034, -1034, -1034,   429, -1034,   505, -1034,   492,
   -1034,  2894, -1034,   339,   339, -1034,    85,   508, -1034,   -63,
     327,   163,   178,   173, -1034,   189,  1070, -1034, -1034, -1034,
   -1034,   424,   -21,   368,   229,   339,   229,     3,   229,   229,
   -1034,  8835,  8835,  7635,  8835,  8835,  8888,  1358,  8888, -1034,
   -1034,  7635, -1034,   319,   391,   301, -1034, -1034,   255,   -21,
   -1034,   511, -1034,  3189, -1034, -1034,  7635,   171, -1034,  8835,
     366,  7635,  7635,  7635,   151,  7635,  7635,  8835,   254,   265,
     273,   449,   218, -1034,   286, -1034,  8835, -1034, -1034, -1034,
   -1034, -1034, -1034,   -32,   516,   293,   221, -1034,   228, -1034,
   -1034,   488,   236, -1034, -1034, -1034,  9333,  7635,  7635,   328,
     498,   510,   528,   548, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034,   352,   354,  6609,
    6609, -1034,   439,   -21,  1261,  7635,  6609,   422,   443, -1034,
   -1034,   601,   601,   229,   229,   229,   229,   229,   229,   229,
     309,   200, -1034,  7122,  9333,   277, -1034,  7747,  4044,   446,
    7635, -1034, -1034,  9333,  8643,   440, -1034,   457, -1034,   149,
     454,   264,   149,   127,  7635, -1034, -1034,   424, -1034, -1034,
   -1034, -1034, -1034,   473,  6438,   481,   653,   486,  2230,  7635,
    7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,
    7635,  7635,   237,  7635,  7635,  7635,  7635,  7635,  7635,  7635,
    7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635, -1034, -1034,
   -1034,    50,  9078,  9229,    58,    58,  7635,   -21,  6780,  7635,
    7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,  7635,
    7635, -1034, -1034,  7635, -1034,  7813,  7635,  7825, -1034, -1034,
   -1034,     6, -1034,    58,    58,     6,  7635,    46,  7635,  7635,
     -23,  7635,  7635,  7635,  2401,   828,  7635,    35,    93,   659,
     670,  7635,     0,   -21,   131,   154,   159,   164,   672,   682,
     683,   685,   689,   704,   707,   711,   715,  7293, -1034,   717,
     555, -1034,  8835,  7885,  7635, -1034,   557,  7994,   552,   565,
    8835,  8688,   -43,  8054,  8066, -1034, -1034, -1034,  7635,   151,
   -1034, -1034,  4215,   721,   561,   -24,   564,   250, -1034,   516,
   -1034,     6, -1034,  7635,   718, -1034,   576, -1034,    15,  8835,
     581, -1034,  8132,   578,   661, -1034,   667,   755, -1034,   604,
   -1034,   608,   609,   331,   612, -1034,   -21,  8235,   613,   764,
     766,   356, -1034, -1034,   345,  2513,   615, -1034, -1034, -1034,
     556,   616, -1034,   980, -1034, -1034, -1034,  6609,  8835,   377,
    6951,   787,  6609, -1034, -1034,  2579, -1034,   772,  7635, -1034,
    7635, -1034, -1034,  7635,  8700,  2915,  1160,  1115,  1115,   577,
      47,    47,     3,     3,     3,  8876,  3177,  8888, -1034,  8383,
    8941,  2977,  2977,  2977,  2977,  1115,  1115,  2977,   105,   105,
    8931,   229,  9077,  9077,   628, -1034, -1034, -1034,   630,  7635,
     631,   632,   -21,  7635,   631,   632,   -21, -1034,  7635, -1034,
     -21,   -21,   635, -1034,  9333,  8888,  8888,  8888,  8888,  8888,
    8888,  8888,  8888,  8888,  8888,  8888,  8888,  8888,  8888, -1034,
    8888, -1034,   -21, -1034, -1034, -1034, -1034,   640, -1034,  8835,
    7635,  3360,   644,  1370, -1034,  7635,  1542,  7635,  7635,  9322,
   -1034,    19,   638,  8835, -1034, -1034, -1034,   247,   643, -1034,
   -1034,   760, -1034, -1034,  8835, -1034,  9333,   648,  7635,   649,
   -1034, -1034,   331,   723,   651,   331, -1034,    54,   723, -1034,
    3531,   822, -1034, -1034, -1034,   658, -1034, -1034, -1034,   801,
   -1034, -1034, -1034,   668, -1034,  7635, -1034, -1034,   665, -1034,
     669,   674,  9333,  8835,  7635, -1034, -1034,   661,  8301,  8313,
    4386,  8931,  7635,  1416,   675,  1416,  2593, -1034,  2648, -1034,
    2783, -1034, -1034, -1034,   601,   661, -1034,  8835,  7635, -1034,
   -1034, -1034, -1034,  8373, -1034, -1034, -1034,   666,  8835,   686,
    6609,  9333,   -86,   -68,  1714,   680,   687, -1034,  7464, -1034,
     459,   784,   271,   690, -1034, -1034,   271, -1034,   688, -1034,
   -1034, -1034,   331, -1034, -1034,   694, -1034,   684,   660, -1034,
   -1034, -1034,   660, -1034, -1034,    28,   852,   860,   700, -1034,
   -1034,  3702, -1034,  7635, -1034, -1034,  8631,   696,   822,  6609,
     451,  8888,   723,  6438,   864,   697,  8931, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034,  1749,   701,   703,
   -1034,    79, -1034,  1059, -1034,  1416,   702,   705,   705, -1034,
     723,  6267,   706,  4557,  7635,  6609,   722,   -45,  9322,  1886,
   -1034, -1034, -1034, -1034,   708, -1034,   -15,   719,   712,   724,
   -1034,   725,  8835,   726,   727, -1034,   855, -1034,   247,   720,
     730, -1034, -1034,   694,   728,  9300,   331, -1034, -1034,   734,
      73,   660,   420,   420,   660,   729, -1034,  8888,   731, -1034,
     732, -1034, -1034, -1034, -1034, -1034,   874,  1590, -1034,   733,
     733,   745, -1034,    39,   892,   907,   746, -1034,   741,   842,
   -1034, -1034,   748,   747,   751,   -29,   752, -1034, -1034, -1034,
    4728,   713,   754,  7635,    16,   -10, -1034, -1034,   777, -1034,
    7464, -1034,  7635,   780,   331, -1034, -1034, -1034, -1034,   271,
     758, -1034, -1034,   331, -1034, -1034,   428, -1034, -1034, -1034,
      79,   861,   865,  1793, -1034,  9397, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034,   822,   756,  6267,    54,   788, -1034,
   -1034,   771,   115, -1034,   778,   733,   442,   442,   733,   874,
     767,   874,   768, -1034,  2058, -1034,  1886,  4899,   770,   773,
   -1034,  2846, -1034, -1034, -1034, -1034,  7635, -1034,  8835,  7635,
      67, -1034,  5070, -1034, -1034,  9671, 10108,   169, -1034,   916,
     339,  8520, -1034,  9717, -1034, -1034, -1034, -1034, -1034,   920,
   -1034, -1034, -1034, -1034, -1034, -1034,    10, -1034, -1034, -1034,
   -1034, -1034, -1034,   774, -1034, -1034, -1034,  6267,  8835,  8835,
     331, -1034,   779, -1034, -1034,   944, -1034,  9485, -1034,   948,
     256, -1034, -1034, 10108,   949,   953,   954,   956,   957, 10201,
     275, -1034, -1034,  9752, -1034, -1034,   785, -1034,   936,   792,
   -1034,   789,  9787,  5241, -1034,  6267, -1034,   791,  7635,   793,
     812, -1034, -1034,  9620, -1034,   799,   800,   915,   899,   818,
    7635,   805,   958, -1034, -1034,  7635,  7635,   949,   288, 10201,
   -1034, -1034,  7635,    23, -1034, -1034,    10,   808, -1034, -1034,
     811, -1034,  8835, -1034, -1034, -1034, -1034, -1034,  9922,   331,
   10108,  8835, -1034,   984, -1034,   810,  8835,  8835, -1034, -1034,
    8835,  7635, -1034, -1034,  5412, -1034, -1034,  5583, -1034,  5754,
   -1034, -1034, 10108,   694, -1034,   813,  1139,  7635, -1034,  1416,
   -1034, -1034, -1034,  2833,  1221, -1034, -1034, -1034, -1034, -1034,
   -1034,  1749,  1059,   339, -1034,  8835,   814, -1034, -1034, -1034,
   -1034,  1580, -1034,   969, -1034,   874, -1034, -1034, -1034, -1034,
     369,   826, -1034, -1034, -1034,  1416, -1034,  5925, -1034,   821,
     160, -1034, -1034,  7635, -1034, -1034, -1034,  3029,  6096, -1034,
   -1034, -1034
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      87,     0,     2,     1,     0,     0,     0,     0,     0,   519,
     520,    94,    96,    97,    95,   564,   163,   517,     0,     0,
       0,     0,     0,     0,   474,     0,   200,     0,   513,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   541,   541,
       0,   487,   486,     0,   541,     0,     0,     0,     0,   505,
     202,   203,   204,     0,     0,     0,     0,   194,   205,   207,
     209,   117,     0,     0,   528,   529,   530,   536,   531,   532,
     533,   534,   535,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   170,   145,   582,   510,     0,
       0,   527,   103,     0,   111,    86,   110,   105,   106,     0,
     196,   107,   108,   109,   483,   254,   151,     0,   152,   456,
     457,     0,   479,   491,   491,   559,     0,   524,   471,   525,
     526,     0,   549,     0,   503,   560,   400,   555,   561,   461,
      94,   505,     0,     0,   441,   491,   442,   443,   444,   470,
     173,   611,   612,     0,   614,   615,   473,   475,   477,   505,
     204,     0,   506,   200,   201,     0,   198,   397,   507,   399,
     569,   508,   405,     0,   514,   469,     0,     0,   388,   389,
       0,     0,   390,     0,     0,     0,     0,   542,     0,     0,
       0,     0,     0,   143,     0,   145,   478,    90,    93,    91,
     124,   125,    92,   140,     0,     0,     0,   135,     0,   308,
     309,   312,     0,   311,   481,   500,     0,     0,     0,     0,
       0,     0,     0,     0,    84,    89,     3,     4,     5,     6,
       7,     8,     9,    10,    46,    47,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    49,    50,    51,    52,    71,
      53,    41,    42,    43,    70,    44,    45,    30,    31,    32,
      33,    34,    35,    36,    75,    76,    77,    78,    79,    80,
      81,    37,    38,    39,    40,    61,    59,    60,    72,    56,
      57,    58,    48,    54,    55,    66,    67,    68,    62,    63,
      65,    64,    73,    69,    74,    85,    88,   115,     0,   582,
     582,   100,   128,    98,   200,     0,   582,   548,   546,   550,
     547,   420,   422,   462,   463,   464,   465,   466,   467,   468,
     597,     0,   522,     0,     0,     0,   595,     0,     0,     0,
       0,    82,    83,     0,   587,     0,   585,   581,   583,   511,
       0,   512,     0,     0,     0,   566,   499,     0,   104,   112,
     480,   192,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   164,   492,
     488,   488,     0,     0,     0,     0,   541,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   419,   421,     0,   488,     0,     0,     0,   398,   199,
     394,     0,   396,     0,     0,     0,   541,    94,     3,     4,
       5,     6,     7,    46,   474,    12,    13,   513,    71,   487,
     486,    33,    75,    81,    39,    40,    48,    54,   528,   529,
     530,   536,   531,   532,   533,   534,   535,     0,   299,     0,
     128,   302,   304,     0,     0,   162,     0,     0,     0,   391,
     393,     0,     0,     0,     0,   157,   158,   172,     0,     0,
     123,   159,     0,     0,     0,   140,     0,     0,   119,     0,
     121,     0,   160,     0,     0,   161,   128,   182,   544,   618,
     128,   616,     0,     0,   216,   488,   218,   211,   114,     0,
      87,     0,     0,   129,     0,    99,     0,     0,     0,     0,
       0,     0,   521,   596,     0,     0,   544,   594,   523,   593,
     455,     0,   150,     0,   147,   144,   146,   582,   590,   544,
       0,   516,   582,   472,   518,     0,   482,     0,     0,   255,
       0,   145,   258,     0,     0,   428,   431,   449,   451,   432,
     433,   434,   435,   437,   438,   425,   427,   426,   454,   423,
     424,   447,   448,   445,   446,   450,   452,   453,   439,   440,
     460,   436,   430,   429,     0,   185,   186,   488,     0,     0,
     537,   567,     0,     0,   538,   568,     0,   578,     0,   580,
     562,   563,     0,   504,     0,   403,   406,   407,   408,   410,
     411,   412,   413,   414,   415,   416,   417,   418,   409,   613,
     476,   509,   513,   573,   571,   572,   574,     0,   301,   306,
       0,   129,     0,     0,   387,     0,     0,   390,     0,     0,
     168,     0,     0,   488,   142,   174,   141,     0,     0,   120,
     122,   140,   134,   307,   313,   310,   129,     0,   129,     0,
     610,   113,     0,   220,     0,     0,   488,     0,   220,    87,
       0,     0,   515,   101,   102,   545,   516,   599,   600,     0,
     605,   608,   606,     0,   602,     0,   601,   604,     0,   148,
       0,     0,     0,   586,     0,   584,   565,   216,     0,     0,
       0,   459,     0,   266,     0,   266,     0,   501,     0,   502,
       0,   557,   558,   556,   404,   216,   570,   305,     0,   303,
     300,   145,   252,     0,   145,   250,   153,     0,   392,     0,
     582,     0,     0,   544,     0,   236,   236,   156,   242,   386,
     180,   138,     0,   128,   131,   136,     0,   183,     0,   617,
     609,   217,     0,   488,   315,   219,   325,     0,     0,   277,
     288,   289,     0,   290,   212,   272,     0,   274,   275,   276,
     488,     0,   118,     0,   607,   598,     0,     0,   592,   582,
     544,   402,   220,     0,     0,     0,   458,   358,   359,   360,
     354,   353,   352,   357,   356,   355,   361,   266,     0,   128,
     262,   270,   265,   267,   350,   266,     0,   539,   540,   579,
     220,   256,     0,     0,   390,   582,     0,   544,     0,     0,
     145,   230,   169,   236,     0,   236,     0,   128,     0,   128,
     244,   128,   248,     0,     0,   171,     0,   137,   129,     0,
     128,   133,   165,   221,     0,   346,     0,   315,   273,     0,
       0,     0,     0,     0,     0,     0,   116,   401,     0,   149,
       0,   488,   253,   145,   259,   264,   297,   266,   260,     0,
       0,   188,   271,   284,     0,   286,   287,   351,     0,   493,
     488,   154,     0,     0,     0,   516,     0,   145,   228,   166,
       0,     0,     0,     0,     0,     0,   232,   129,     0,   241,
     129,   243,   129,     0,     0,   145,   139,   130,   127,   129,
       0,   315,   488,     0,   345,   206,   346,   321,   322,   314,
     270,     0,     0,   344,   326,   346,   279,   282,   278,   280,
     281,   283,   315,   603,   591,     0,   257,     0,     0,   263,
     285,     0,     0,   189,   190,     0,     0,     0,     0,   297,
       0,   297,     0,   251,     0,   224,     0,     0,     0,     0,
     234,     0,   239,   240,   145,   233,     0,   245,   249,     0,
     178,   176,     0,   132,   126,   346,     0,     0,   323,     0,
     491,     0,   208,   346,   315,   298,   489,   292,   191,     0,
     295,   291,   293,   294,   296,   489,     0,   489,   315,   145,
     226,   155,   167,     0,   231,   235,   145,   238,   247,   246,
       0,   179,     0,   181,   195,   214,   327,     0,   324,   488,
       0,   363,   317,     0,    94,   277,   288,   289,     0,     0,
       0,   383,   210,   346,   490,   488,     0,   497,     0,   128,
     496,     0,   346,     0,   229,   237,   177,     0,     0,     0,
      75,   328,   339,     0,   330,     0,     0,     0,   340,     0,
       0,   364,     0,   316,   488,     0,     0,     0,     0,     0,
     318,   193,     0,   371,   145,   498,   129,     0,   145,   395,
       0,   145,   215,   213,   329,   331,   332,   333,     0,     0,
       0,   488,   368,   488,   362,     0,   488,   488,   319,   382,
     489,     0,   368,   268,     0,   495,   494,     0,   227,     0,
     335,   336,   338,   334,   341,   365,   373,     0,   364,   266,
     384,   385,   485,   371,   373,   489,   489,   175,   337,   368,
     366,   373,   374,   491,   369,   488,     0,   269,   372,   187,
     484,   373,   370,     0,   365,   297,   367,   488,   489,   375,
       0,   380,   342,   145,   489,   266,   489,     0,   320,     0,
       0,   343,   381,     0,   377,   145,   489,     0,     0,   376,
     379,   378
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1034, -1034,   -87,  -814,  -109,   -58,  -467, -1034,   -37,  -177,
     332,   490, -1034,   -85,    -2,     4, -1034, -1034, -1034,   960,
   -1034, -1034,  -451, -1034, -1034,   815,   166,  -714,   519,   837,
     -13, -1034,    64, -1034, -1034, -1034, -1034, -1034, -1034,   359,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
      -6, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
    -542, -1034,  -624,   194, -1034,    60, -1034, -1034,  -700,    56,
   -1034, -1034, -1034,   119, -1034, -1034, -1034, -1034, -1034, -1034,
    -692, -1034,   155, -1034,   226,   106,  -870,  -381,  -203, -1034,
     263, -1034,  -689,  -294, -1034,   158,  -920,    18, -1034,   396,
   -1034,   538, -1034,   536,  -681,   117, -1034,  -738, -1034, -1034,
     -18, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,  -792,
    -788, -1034,   -26, -1034, -1033,   -84, -1034,   -93, -1034, -1034,
   -1034,    21,   -22,   558, -1034,   584,  -616, -1034,   896, -1034,
     -17, -1034,   199,   -33, -1034,    -1,  -371,  -955, -1034,  -112,
   -1034, -1034,   -31, -1034, -1034,   -19,  -228, -1034,   431,    40,
   -1034,    63,    68,    -5, -1034, -1034, -1034, -1034, -1034,    90,
      62, -1034, -1034,   662,   -94,  -288,   514, -1034, -1034,   592,
     475, -1034, -1034, -1034,   399
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   294,   295,   333,   459,     2,   297,   741,   193,
      91,   301,   302,    92,   133,   534,    95,   509,   298,   742,
     486,   195,   514,   743,   840,   196,   744,   745,   197,   182,
     328,   535,   536,   734,   740,   970,  1012,   835,   496,   497,
     587,    97,   944,   989,    98,   547,   210,    99,   154,   155,
     100,   101,   211,   102,   212,   103,   213,   668,   917,  1049,
     663,   666,   753,   732,  1001,   889,   822,   737,   824,   964,
     104,   828,   829,   830,   831,   726,   105,   106,   107,   108,
     798,   799,   800,   801,   802,   871,   764,   765,   766,   767,
     768,   872,   769,   874,   875,   876,   938,   164,   460,   461,
     198,   199,   202,   203,   845,   918,   919,   755,  1018,  1053,
    1054,  1055,  1056,  1057,  1058,  1154,   920,   921,   922,   803,
     804,  1020,  1021,  1022,  1116,  1103,  1133,  1134,  1151,  1166,
    1156,  1030,  1031,   183,   167,   168,   468,   469,   157,   622,
     109,   110,   111,   112,   113,   135,   588,  1034,  1072,   390,
     951,  1039,  1040,   115,   397,   116,   159,   340,   165,   117,
     118,   119,   120,   178,   121,   122,   123,   124,   125,   126,
     127,   128,   161,   592,   600,   335,   336,   337,   338,   325,
     326,   683,   129,   500,   501
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      93,   114,   391,   296,   389,   389,    94,   158,   348,   632,
     192,   511,   512,   806,   843,   877,   204,   485,   518,   584,
     156,   727,   962,   414,   153,   735,   389,  1101,   837,   995,
     483,   997,   841,    15,   179,  -278,   826,  1037,   483,   184,
    1036,   -14,  1041,   670,   770,   657,  -291,   372,   348,   659,
     893,   894,   -84,   923,   303,   893,   894,   307,   307,   758,
     350,   366,   367,   368,    15,   818,    96,   985,   348,  1124,
     205,    41,    42,   585,  1010,     3,   516,   130,    12,    13,
      14,   597,   201,  -222,   869,    15,  -550,  -550,   160,   819,
     140,   372,   114,   352,  1011,   170,  1141,   349,   204,   320,
     350,   339,   130,    12,    13,    14,  -223,  -222,   -84,   346,
     200,  1029,   873,   878,   308,   308,  -550,   364,   365,   366,
     367,   368,  -225,   891,   923,   895,    41,    42,  -506,   479,
    -223,   484,   640,   923,   664,   877,   759,   309,   309,   648,
     163,   586,   310,   310,   568,  1122,  -225,   156,   419,   372,
     205,   143,   345,   163,   320,   782,   527,   192,   861,   760,
     761,   180,  -505,   896,   311,   312,   925,  1028,   965,   385,
    1139,  1140,   482,   810,   181,   977,   320,   422,   331,   332,
     940,   941,   166,   923,   760,   761,   880,  -184,    90,   307,
    -184,   923,   963,  1150,   171,   973,   736,   851,   883,  1158,
    1102,  1160,   771,   134,   136,   137,   138,   139,   945,   163,
     172,  1169,   173,   385,   344,  1028,   704,   141,   142,    90,
     144,   145,   146,   147,   148,  1148,   162,   320,   762,   169,
     975,   873,   323,   324,   320,   598,   342,   177,   177,   174,
      90,   923,   854,   177,   175,   186,   308,   392,   926,   690,
     923,   983,   320,   870,   321,  1028,   990,   991,   991,   994,
     130,    12,    13,    14,    15,   187,   188,   176,   189,   309,
     187,   385,   739,   189,   310,   313,   314,   315,   316,   317,
     318,   319,   206,   327,   948,   158,   334,   323,   324,   604,
     987,   320,   839,   527,   187,   757,   498,   189,   156,   185,
    1112,   601,   153,  1033,   320,   207,   527,   307,   544,   323,
     324,  1163,   651,   190,   546,   191,   307,  1042,   190,   149,
     191,   515,   394,   395,  1132,   393,   533,   114,   208,   624,
     625,   209,  1132,   299,   590,   594,  1164,  1165,   300,  1132,
    -543,   846,   415,   464,   877,  1016,  1017,   465,   868,  1132,
     417,  1113,   396,   158,   130,    12,    13,    14,   152,   522,
     323,   324,   462,  -551,   308,   463,   160,   323,   324,   679,
     467,   470,   471,   308,   473,   474,   898,   848,   901,   680,
     903,   849,   844,   681,   322,   323,   324,   309,   682,   910,
     479,   602,   310,   489,   480,   385,   309,   490,   420,   855,
     491,   310,    50,    51,   492,   152,   499,   502,   494,   204,
     150,   151,   495,   149,   526,   603,   158,   421,   549,    90,
     466,   627,   489,   539,   323,   324,   650,  1136,  1062,   156,
     475,   692,  1063,   153,   160,    73,   528,   323,   324,    41,
      42,   476,   816,   130,    12,    13,    14,  1069,   348,   477,
     347,  1070,   192,   478,   591,   595,   599,   599,   519,   520,
    1069,   205,   481,  1159,  1098,   130,    12,    13,    14,   488,
     927,   928,   928,   931,    50,    51,  -547,  -547,   353,   354,
     533,   114,   150,   623,    73,   599,   599,   626,   521,  -547,
     935,   860,   493,   912,   303,    41,    42,   160,   334,   334,
     350,  -348,   759,   503,   517,   334,  -547,   331,   332,   952,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   796,
     914,   504,   525,   684,   685,   760,   761,   884,   508,   538,
     731,   510,   114,   505,   422,   833,   834,   689,   700,   187,
     188,   976,   189,   545,    73,  1152,  1153,   760,   761,  -589,
    -589,   506,  -589,   200,   355,   356,   357,   554,   555,   556,
     557,   558,   559,   560,   561,   562,   563,   564,   565,   566,
     567,   507,   569,   570,   571,   572,   573,   574,   575,   576,
     577,   578,   579,   580,   581,   582,   583,   307,  1077,   364,
     365,   366,   367,   368,   762,   177,  -554,   605,   606,   607,
     608,   609,   610,   611,   612,   613,   614,   615,   616,   617,
     707,   513,   618,   541,   709,   620,   870,  -553,   711,   712,
     537,   372,   307,  -588,  -588,   177,  -588,   141,   142,   542,
     144,   145,   146,   147,   543,   162,   152,    50,    51,   307,
     186,  -548,  -548,   751,   308,   150,   756,   548,  1061,   929,
     930,   351,   992,   993,  -548,   550,   629,  -546,  -546,   551,
     423,   424,   552,   169,  1073,   -43,  -554,   309,    93,   114,
    -546,  -548,   310,   425,    94,   307,   -70,   643,   -66,   308,
     341,   343,  -553,   130,    12,    13,    14,  -546,   -67,   -68,
     426,   -62,   654,  1095,   714,   -63,   308,   722,   533,   114,
     725,   797,   309,   797,   152,  -545,  -545,   310,   811,   731,
     -65,   813,   348,   -64,   307,   382,   383,   -73,  -545,   309,
    1115,   -69,  1118,   630,   310,  1120,  1121,   631,   637,   733,
    -552,   635,   308,   756,    96,  -545,   334,   638,   647,   693,
     649,   334,   759,   385,   646,   201,   498,   698,   656,   699,
    -544,  -544,   701,   658,   661,   309,   130,    12,    13,    14,
     310,   667,   662,  -544,  1144,   760,   761,   152,   665,    93,
     114,   308,   892,   893,   894,    94,  1149,   959,   893,   894,
    -544,   669,   780,   671,   672,   674,   676,   677,   706,   678,
     688,   694,   708,   687,   309,   697,   523,   710,   821,   310,
     529,   307,   703,   797,   705,  -575,  -577,   890,   713,   533,
     114,   533,   114,   716,   523,   738,   529,   523,   529,   720,
     746,   817,   483,   748,   750,   752,   773,   924,   754,   717,
     462,   348,  -552,   774,   723,    96,   470,   728,   760,   761,
     777,   775,   814,   916,   778,   152,   836,   862,   779,   805,
     936,   130,    12,    13,    14,    15,   823,   499,   308,   852,
     815,   847,   838,   825,   842,   797,   846,   853,  1023,   854,
     863,   389,   859,   864,   957,   867,   866,   879,   906,  -576,
     937,   309,   881,   888,   776,   971,   310,  1038,   533,   114,
     899,   897,   972,   781,   756,   885,   900,   902,   908,   946,
     904,   786,   909,   851,   905,   911,   932,   934,   733,   933,
     149,    50,    51,   943,   947,   948,   949,   629,  1015,   150,
     980,   950,   954,   916,   953,  -200,   955,   956,   966,   334,
     960,   969,    41,   984,   533,   114,   974,   832,   981,   986,
     945,   996,   988,  1019,    73,   998,  1004,  1035,  1048,  1005,
    1044,  1007,  1060,  1065,  1047,   533,   114,   -75,   -54,  1052,
     -55,  1066,  1074,  1075,  1076,  1064,  1078,  1038,  1081,  1083,
     533,   114,   857,   916,  -505,  1086,  1087,  1088,   334,  1089,
    1090,   916,  1092,  1106,  1119,  1093,  1043,  1108,  1117,  1145,
    1129,  1046,  1147,  1045,   152,  1052,  1162,   152,  1059,   763,
    1155,  1111,   151,   673,   907,   533,   114,   194,   652,   487,
      90,   472,   886,   470,   334,   747,  1002,  1006,  1000,   967,
     888,  1143,   939,   865,   389,   850,   979,   719,   942,   653,
     655,   916,  1114,   978,  1059,  1085,  1094,   644,  1142,  1137,
     916,   533,   114,   533,   114,  1105,   348,  1099,   634,   418,
    1068,    41,    42,   715,  1128,   596,   695,   749,     0,     0,
       0,  1104,   347,    50,    51,  1107,     0,     0,  1109,     0,
     756,   150,     0,     0,   398,     0,     0,    57,    58,    59,
      60,     0,     0,     0,   152,     0,     0,     0,     0,     0,
     763,     0,   961,     0,   763,     0,    73,     0,     0,   832,
       0,   968,   533,   114,     0,   533,   114,   533,   114,     0,
       0,     0,     0,     0,  1131,     0,     0,   797,     0,     0,
       0,     0,  1131,     0,  -590,  -590,   363,   364,   365,   366,
     367,   368,     0,   763,     0,     0,     0,     0,     0,  1131,
    1157,   787,   788,   789,   790,   791,   792,   793,   794,   795,
     796,     0,  1168,   797,     0,   533,   114,     0,     0,   372,
       0,     0,     0,     0,     0,  1008,   533,   114,  1009,   361,
     362,   363,   364,   365,   366,   367,   368,     0,   152,     0,
       0,     0,     0,   763,   763,   763,   763,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   409,   410,     0,
       0,   763,   763,     0,   372,     0,     0,     0,     0,     0,
     411,   412,     0,     0,     0,     0,     0,     0,     0,  -544,
    -544,   787,   788,   789,   790,   791,   792,   793,   794,   795,
     796,     0,  -544,     0,     0,     0,   152,   413,     0,     0,
       0,     0,     0,     0,     0,   152,     0,  1082,     0,  -544,
    -590,  -590,   763,   382,   383,    73,     0,     0,     0,  1091,
       0,     0,     0,     0,  1096,  1097,     0,     0,     0,   763,
       0,  1100,     0,     0,     0,     0,     0,   763,   763,   763,
     763,   385,     0,     0,   130,    12,    13,    14,    15,     0,
       0,   375,   376,   377,   378,   379,   380,   381,   382,   383,
    1123,     0,     0,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   763,     0,     0,  1135,  1130,     0,     0,
       0,     0,     0,     0,     0,     0,   385,     0,   386,   387,
       0,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   152,   149,    50,    51,     0,     0,     0,   152,
       0,     0,   150,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1167,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,     0,   721,    73,     0,     0,
       0,     0,     4,     5,     0,   152,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,  1138,
      17,     0,   372,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,   152,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,   151,     0,    37,    38,    39,
      40,    41,    42,    90,    44,    45,     0,     0,    46,     0,
       0,    48,    49,     0,     0,     0,     0,     0,     0,     0,
       0,   132,     0,    53,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   787,   788,
     789,   790,   791,   792,   793,   794,   795,   796,     0,   416,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,   384,   385,     0,   386,   387,    83,     0,
       0,     0,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,    85,    86,   724,    87,
      88,    89,    90,     0,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,  -261,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
       0,     0,     0,   132,     0,    53,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,    73,     0,     0,     0,
      83,     0,     0,     0,     0,     0,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,    85,    86,
     820,    87,    88,    89,    90,     0,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,  1146,    26,
      27,    28,    29,     0,     0,  -129,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,     0,     0,     0,
       0,     0,     0,     0,     0,   132,     0,    53,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,   787,   788,   789,   790,   791,   792,   793,   794,   795,
     796,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,  -347,    73,  -349,     0,     0,     0,
       0,     0,    83,     0,     0,   787,   788,   789,   790,   791,
     792,   793,   794,   795,   796,     0,     0,     0,    84,     0,
      85,    86,   887,    87,    88,    89,    90,     0,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,     0,
       0,     0,     0,     0,     0,     0,     0,   132,     0,    53,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,    85,    86,   999,    87,    88,    89,    90,     0,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,     0,     0,     0,   132,
       0,    53,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,    85,    86,   553,    87,    88,    89,
      90,     0,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   130,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   131,     0,     0,     0,     0,     0,     0,     0,
       0,   132,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,   -47,     0,    87,
      88,    89,    90,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   130,    12,    13,    14,    15,     0,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    41,    42,     0,     0,     0,     0,     0,    46,
       0,     0,     0,   131,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,   369,   370,   371,     0,     0,     0,   372,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
      87,    88,    89,    90,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,   369,   370,   371,
       0,     0,     0,   372,     0,     0,     0,     0,     0,     0,
       0,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   384,   385,
       0,   386,   387,     0,     0,     0,   369,   370,   371,     0,
       0,   686,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
       0,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   384,   385,     0,   386,   387,     0,
       0,     0,     0,     0,     0,     0,     0,   696,   384,   385,
       0,   386,   387,     0,     0,     0,     0,     0,     0,     0,
       0,   807,     0,     0,     0,     0,     0,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   384,   385,     0,   386,   387,     0,     0,
       0,   369,   370,   371,     0,     0,   808,   372,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   358,   962,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,     0,     0,   369,   370,   371,     0,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,   358,
       0,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,     0,     0,   372,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   384,   385,
       0,   386,   387,     0,     0,     0,     0,     0,     0,   372,
       0,   809,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,     0,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   361,   362,   363,   364,
     365,   366,   367,   368,     0,     0,     0,     0,   384,   385,
       0,   386,   387,     0,     0,     0,     0,     0,     0,     0,
    1102,   384,   385,     0,   386,   387,     0,     0,     0,     0,
       0,   372,   963,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   375,   376,   377,   378,
     379,   380,   381,   382,   383,     0,     0,     0,     0,   384,
     385,     0,   386,   387,     0,     0,     0,   369,   370,   371,
     388,     0,     0,   372,     0,     0,     0,     0,     0,     0,
       0,   385,     0,   386,   387,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -590,  -590,
    -590,  -590,   379,   380,  -590,   382,   383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   384,   385,     0,   386,   387,     0,
       0,     4,     5,     0,     0,  1170,     6,     7,     8,     0,
       9,    10,   427,    12,    13,    14,    15,   371,     0,    17,
       0,   372,   428,   429,   430,   431,   432,   221,   222,   223,
     433,   434,    25,   226,   435,   436,   437,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   438,   250,   251,   252,
     439,   440,   255,   256,   257,   258,   259,   441,   261,   262,
     263,   442,   265,   266,   267,   268,   269,     0,     0,     0,
     443,   271,   272,   444,   445,     0,   275,   276,   277,   278,
     279,   280,   281,   446,   447,   284,   448,   449,   450,   451,
     452,   453,   454,   455,   456,    73,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,     0,
       0,     0,   384,   385,     0,   386,   387,    83,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,   458,     0,     0,     0,    87,    88,
      89,    90,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   427,    12,    13,    14,    15,     0,     0,
      17,     0,     0,   428,   429,   430,   431,   432,   221,   222,
     223,   433,   434,    25,   226,   435,   436,   437,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   438,   250,   251,
     252,   439,   440,   255,   256,   257,   258,   259,   441,   261,
     262,   263,   442,   265,   266,   267,   268,   269,     0,     0,
       0,   443,   271,   272,   444,   445,     0,   275,   276,   277,
     278,   279,   280,   281,   446,   447,   284,   448,   449,   450,
     451,   452,   453,   454,   455,   456,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,   718,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,    87,
      88,    89,    90,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,    46,
      47,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,     0,     0,    61,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,    85,    86,   772,
      87,    88,    89,    90,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,     0,     0,
      46,    47,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,     0,     0,    61,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,    85,    86,
     856,    87,    88,    89,    90,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,    46,    47,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,     0,     0,    61,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,    85,
      86,     0,    87,    88,    89,    90,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,    52,     0,    53,    54,    55,
     531,    57,    58,    59,    60,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
      85,    86,   532,    87,    88,    89,    90,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,    52,     0,    53,    54,
      55,   531,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    85,    86,   645,    87,    88,    89,    90,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,   785,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,   531,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,    85,    86,     0,    87,    88,    89,    90,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,   882,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
      53,    54,    55,   531,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,    85,    86,     0,    87,    88,    89,    90,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,   958,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,    52,
       0,    53,    54,    55,   531,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,    85,    86,     0,    87,    88,    89,
      90,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,  1003,    35,
       0,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,    44,    45,     0,     0,    46,     0,     0,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
      52,     0,    53,    54,    55,   531,    57,    58,    59,    60,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,    85,    86,     0,    87,    88,
      89,    90,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,    52,     0,    53,    54,    55,   531,    57,    58,    59,
      60,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,    85,    86,  1013,    87,
      88,    89,    90,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,  1080,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,   531,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,    85,    86,     0,
      87,    88,    89,    90,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,    52,     0,    53,    54,    55,   531,    57,
      58,    59,    60,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,    85,    86,
    1125,    87,    88,    89,    90,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,   531,
      57,    58,    59,    60,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,    85,
      86,  1126,    87,    88,    89,    90,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,    52,     0,    53,    54,    55,
     531,    57,    58,    59,    60,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
      85,    86,  1127,    87,    88,    89,    90,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,    52,     0,    53,    54,
      55,   531,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    85,    86,  1161,    87,    88,    89,    90,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,   531,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,    85,    86,  1171,    87,    88,    89,    90,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
      53,    54,    55,   531,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,    85,    86,     0,    87,    88,    89,    90,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,     0,     0,     0,   132,
       0,    53,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,    85,    86,     0,    87,    88,    89,
      90,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,   130,    12,    13,    14,    15,     0,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
      41,    42,     0,     0,     0,     0,     0,    46,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,     0,     0,
     132,     0,     0,    54,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   329,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,   330,     0,     0,     0,   331,   332,     0,
       0,     0,     0,    84,     0,     0,     0,     0,    87,    88,
      89,    90,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   130,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   131,     0,     0,     0,     0,     0,     0,     0,
       0,   132,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   331,   332,
       0,     0,     0,     0,    84,     0,     0,     0,     0,    87,
      88,    89,    90,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   130,    12,    13,    14,    15,     0,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    41,    42,     0,     0,     0,     0,     0,    46,
       0,     0,     0,   131,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   691,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   331,
     332,     0,     0,     0,     0,    84,     0,     0,     0,     0,
      87,    88,    89,    90,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,   130,    12,    13,    14,    15,
       0,     0,    17,   524,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
      46,     0,     0,     0,   131,     0,     0,     0,     0,     0,
       0,     0,     0,   132,     0,     0,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,    87,    88,    89,    90,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   130,    12,    13,    14,
      15,     0,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   131,     0,     0,     0,     0,
       0,     0,     0,     0,   132,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,   628,     0,
       0,     0,    87,    88,    89,    90,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   130,    12,    13,
      14,    15,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     827,    37,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,    46,     0,     0,     0,   131,     0,     0,     0,
       0,     0,     0,     0,     0,   132,     0,     0,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,    87,    88,    89,    90,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,   130,    12,
      13,    14,    15,     0,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,     0,     0,     0,    41,    42,     0,     0,
       0,     0,     0,    46,     0,     0,     0,   131,     0,     0,
       0,     0,     0,     0,     0,     0,   132,     0,     0,    54,
      55,     0,     0,     0,     0,     0,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,   369,   370,   371,     0,     0,
       0,   372,     0,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,    87,    88,    89,    90,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,     0,   369,   370,   371,     0,     0,     0,   372,
       0,     0,     0,     0,     0,     0,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,     0,     0,     0,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   384,   385,     0,   386,   387,     0,     0,     0,
       0,     0,   530,   369,   370,   371,     0,     0,     0,   372,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   384,   385,
       0,   386,   387,     0,     0,     0,     0,     0,   619,     0,
     384,   385,     0,   386,   387,     0,     0,     0,     0,   358,
     621,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,     0,     0,     0,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,     0,     0,     0,     0,     0,
       0,     0,   369,   370,   371,     0,     0,     0,   372,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     384,   385,     0,   386,   387,     0,     0,     0,     0,   358,
     633,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   369,   370,   371,     0,     0,     0,   372,     0,
       0,     0,     0,     0,   369,   370,   371,     0,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,     0,     0,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   384,
     385,     0,   386,   387,     0,     0,     0,     0,     0,   636,
     369,   370,   371,     0,     0,     0,   372,     0,     0,     0,
       0,     0,     0,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   384,
     385,     0,   386,   387,     0,     0,     0,     0,     0,   641,
       0,   384,   385,     0,   386,   387,     0,     0,     0,     0,
     358,   642,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,     0,   369,   370,   371,     0,     0,     0,   372,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   384,   385,     0,
     386,   387,     0,     0,     0,     0,   358,   660,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   369,
     370,   371,     0,     0,     0,   372,     0,     0,     0,     0,
       0,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,     0,     0,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,     0,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     384,   385,     0,   386,   387,     0,     0,     0,     0,     0,
     675,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   372,     0,     0,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   384,   385,     0,   386,
     387,     0,     0,     0,     0,     0,   783,     0,   384,   385,
       0,   386,   387,     0,     0,     0,     0,     0,   784,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,     0,   758,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   384,   385,
       0,   386,   387,  1024,    12,    13,    14,     0,   812,   385,
       0,   386,   387,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,     0,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,  1025,   265,   266,   267,   268,   269,     0,     0,
       0,   270,   271,   272,   273,   274,     0,   275,   276,   277,
     278,   279,   280,   281,   282,  1026,  1027,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   358,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   369,
     370,   371,     0,     0,     0,   372,     0,     0,     0,     0,
       0,   369,   370,   371,     0,     0,     0,   372,     0,     0,
       0,     0,     0,   358,   762,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   358,   702,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   369,   370,   371,     0,
       0,     0,   372,     0,     0,     0,     0,     0,   369,   370,
     371,     0,     0,     0,   372,     0,     0,     0,     0,     0,
     639,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   540,     0,   384,   385,     0,   386,
     387,     0,     0,     0,   858,     0,     0,     0,   384,   385,
       0,   386,   387,     0,     0,     0,     0,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,     0,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,   384,   385,     0,   386,   387,     0,     0,
       0,     0,     0,     0,     0,   384,   385,     0,   386,   387,
       0,     0,     0,   369,   370,   371,     0,     0,     0,   372,
       0,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   370,   371,     0,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   372,     0,     0,     0,     0,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,     0,
       0,     0,     0,     0,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     384,   385,     0,   386,   387,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   384,   385,     0,   386,   387,     0,     0,     0,     0,
       0,     0,     0,   384,   385,     0,   386,   387,     0,     0,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,     0,     0,     0,     0,   361,   362,   363,   364,
     365,   366,   367,   368,     0,     0,   384,   385,     0,   386,
     387,   214,     0,     0,     0,    15,     0,   385,     0,   386,
     387,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   372,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,     0,     0,     0,   270,
     271,   272,   273,   274,     0,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   375,   376,
     377,   378,   379,   380,   381,   382,   383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,   589,    15,     0,     0,     0,
      90,     0,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,     0,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
     270,   271,   272,   273,   274,     0,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   130,    12,    13,    14,    15,
       0,     0,    17,     0,     0,     0,   130,    12,    13,    14,
      15,     0,     0,    17,     0,   912,     0,   304,     0,     0,
       0,     0,     0,  -348,     0,     0,     0,     0,   304,   913,
       0,     0,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   914,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   149,     0,   593,     0,     0,     0,
       0,    90,     0,   132,     0,   149,    73,     0,     0,     0,
       0,     0,     0,     0,   132,     0,   729,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   912,     0,     0,     0,     0,     0,     0,     0,
    -348,     0,     0,     0,     0,     0,   913,     0,   915,   787,
     788,   789,   790,   791,   792,   793,   794,   795,   796,   914,
     331,   332,     0,     0,     0,     0,   305,     0,     0,     0,
       0,   730,     0,    89,    90,     0,     0,   305,   427,    12,
      13,    14,   306,    73,    89,    90,     0,     0,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,     0,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,  1050,   265,   266,
     267,   268,   269,     0,     0,   982,   270,   271,   272,   273,
     274,     0,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   427,    12,    13,    14,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,  1051,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,  1050,   265,   266,   267,   268,   269,     0,     0,
       0,   270,   271,   272,   273,   274,     0,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   912,     0,     0,     0,
       0,     0,     0,     0,  -348,     0,     0,     0,     0,     0,
     913,     0,     0,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   914,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   912,     0,     0,     0,     0,    73,     0,     0,
    -348,     0,     0,     0,     0,     0,   913,     0,  1084,   787,
     788,   789,   790,   791,   792,   793,   794,   795,   796,   914,
       0,     0,     0,     0,     0,     0,     0,   912,     0,     0,
       0,     0,     0,     0,     0,  -348,     0,     0,     0,     0,
       0,   913,     0,    73,   787,   788,   789,   790,   791,   792,
     793,   794,   795,   796,   914,     0,     0,     0,     0,  1014,
       0,     0,   912,     0,     0,     0,     0,     0,     0,     0,
    -348,     0,     0,     0,     0,     0,   913,     0,    73,   787,
     788,   789,   790,   791,   792,   793,   794,   795,   796,   914,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1032,     0,     0,     0,     0,
       0,     0,     0,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1071,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,  1079,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   787,   788,   789,   790,   791,   792,
     793,   794,   795,   796,   271,   272,   273,   274,     0,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   214,     0,
       0,   215,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,     0,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,     0,     0,     0,   270,   271,   272,   273,
     274,     0,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,     0,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,     0,     0,     0,   270,
     271,   272,   273,   274,     0,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,  1067,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,     0,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,   270,   271,   272,   273,   274,     0,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293
};

static const yytype_int16 yycheck[] =
{
       2,     2,   114,    61,   113,   114,     2,    26,    93,   460,
      47,   299,   300,   705,   752,   803,    49,   194,   306,   390,
      26,   637,     6,   135,    26,     6,   135,     4,   742,   949,
      62,   951,   746,    27,    39,     7,   736,    27,    62,    44,
     995,     6,   997,   510,   668,   496,     7,    44,   133,   500,
      65,    66,     6,   845,    73,    65,    66,    74,    75,     5,
      93,    14,    15,    16,    27,   151,     2,   937,   153,  1102,
      52,    71,    72,    23,     7,     0,   304,    23,    24,    25,
      26,    23,    27,   151,     5,    27,   149,   150,    26,   175,
       6,    44,    93,    99,    27,    31,  1129,    93,   131,    27,
     133,    29,    23,    24,    25,    26,   151,   175,    62,    91,
      48,   981,   801,   805,    74,    75,   179,    12,    13,    14,
      15,    16,   151,   823,   916,   825,    71,    72,   162,   172,
     175,   163,   175,   925,   505,   923,    82,    74,    75,   163,
     174,    91,    74,    75,   372,  1100,   175,   153,   154,    44,
     132,   174,    90,   174,    27,   697,    29,   194,   782,   105,
     106,    23,   162,   178,    74,    75,   847,   981,   178,   166,
    1125,  1126,   185,   715,    23,   913,    27,   159,   168,   169,
     869,   870,   174,   975,   105,   106,   810,   172,   182,   206,
     175,   983,   176,  1148,   174,   909,   177,   169,   814,  1154,
     177,  1156,   669,     4,     5,     6,     7,     8,   169,   174,
     174,  1166,   174,   166,   177,  1029,   587,    18,    19,   182,
      21,    22,    23,    24,    25,  1145,    27,    27,   174,    30,
     911,   920,   160,   161,    27,   177,    29,    38,    39,   174,
     182,  1033,   169,    44,   174,    46,   206,   162,   175,   537,
    1042,   932,    27,   174,    29,  1069,   945,   946,   947,   948,
      23,    24,    25,    26,    27,    23,    24,   174,    26,   206,
      23,   166,   643,    26,   206,    76,    77,    78,    79,    80,
      81,    82,   174,    84,   169,   304,    87,   160,   161,   398,
     175,    27,   743,    29,    23,   666,   206,    26,   304,   177,
    1088,   395,   304,   984,    27,   174,    29,   324,   181,   160,
     161,   151,   489,    71,   347,    73,   333,   998,    71,    82,
      73,   303,   149,   150,  1116,   162,   328,   328,   174,   423,
     424,   174,  1124,   174,   392,   393,   176,   177,   174,  1131,
     162,   172,   143,   172,  1132,   176,   177,   176,   799,  1141,
     151,  1089,   179,   372,    23,    24,    25,    26,    26,   159,
     160,   161,   163,   174,   324,   166,   304,   160,   161,    13,
     171,   172,   173,   333,   175,   176,   827,   758,   829,    23,
     831,   762,   753,    27,   159,   160,   161,   324,    32,   840,
     172,   396,   324,   172,   176,   166,   333,   176,    97,   770,
     172,   333,    83,    84,   176,    73,   207,   208,   172,   442,
      91,   174,   176,    82,   324,   397,   435,   162,   354,   182,
      54,   426,   172,   333,   160,   161,   176,  1119,   172,   435,
     176,   540,   176,   435,   372,   116,   159,   160,   161,    71,
      72,   176,   730,    23,    24,    25,    26,   172,   533,   176,
      82,   176,   489,     4,   392,   393,   394,   395,   149,   150,
     172,   443,   176,  1155,   176,    23,    24,    25,    26,   176,
     851,   852,   853,   854,    83,    84,   149,   150,    49,    50,
     482,   482,    91,   421,   116,   423,   424,   425,   179,   162,
     861,   779,     4,    65,   513,    71,    72,   435,   299,   300,
     533,    73,    82,   175,   305,   306,   179,   168,   169,   880,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    23,   323,   178,   179,   105,   106,   815,   176,   330,
     639,   177,   533,    23,   516,    76,    77,   533,   551,    23,
      24,   912,    26,   344,   116,   176,   177,   105,   106,   172,
     173,    23,   175,   491,    49,    50,    51,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,    23,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   604,  1039,    12,
      13,    14,    15,    16,   174,   396,   174,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,   410,
     592,   172,   413,   173,   596,   416,   174,   174,   600,   601,
     174,    44,   639,   172,   173,   426,   175,   428,   429,   172,
     431,   432,   433,   434,   180,   436,   304,    83,    84,   656,
     441,   149,   150,   662,   604,    91,   665,   174,  1019,   852,
     853,    97,   946,   947,   162,   174,   457,   149,   150,     6,
     149,   150,   176,   464,  1035,     6,   174,   604,   670,   670,
     162,   179,   604,   162,   670,   692,     6,   478,     6,   639,
      88,    89,   174,    23,    24,    25,    26,   179,     6,     6,
     179,     6,   493,  1064,   604,     6,   656,   633,   700,   700,
     636,   703,   639,   705,   372,   149,   150,   639,   721,   818,
       6,   724,   797,     6,   731,   138,   139,     6,   162,   656,
    1091,     6,  1093,     6,   656,  1096,  1097,   172,   176,   639,
     174,   174,   692,   752,   670,   179,   537,   172,   177,   540,
     176,   542,    82,   166,    23,    27,   656,   548,   172,   550,
     149,   150,   553,   172,   176,   692,    23,    24,    25,    26,
     692,     6,   101,   162,  1135,   105,   106,   435,   101,   771,
     771,   731,    64,    65,    66,   771,  1147,    64,    65,    66,
     179,   177,   692,   175,   175,   173,   173,    23,   589,    23,
     174,     4,   593,   178,   731,    23,   321,   598,   734,   731,
     325,   818,   174,   805,   174,   174,   174,   820,   173,   811,
     811,   813,   813,   173,   339,   177,   341,   342,   343,   175,
     177,   731,    62,   175,   175,   102,     4,   846,   177,   630,
     631,   916,   174,    32,   635,   771,   637,   638,   105,   106,
     175,   173,   176,   845,   175,   513,    62,   783,   174,   174,
     863,    23,    24,    25,    26,    27,   176,   658,   818,     7,
     174,   177,   172,   176,   176,   867,   172,     7,   980,   169,
       6,   980,   176,   176,   887,   172,   175,   175,    23,   174,
       6,   818,   176,   819,   685,   904,   818,   996,   890,   890,
     178,   172,   905,   694,   913,   173,   172,   172,   178,     7,
     174,   702,   172,   169,   177,   177,   177,   175,   818,   178,
      82,    83,    84,   168,     7,   169,   175,   718,   976,    91,
     921,    79,   175,   925,   176,    97,   175,   175,   151,   730,
     176,   151,    71,   177,   936,   936,   178,   738,    73,   151,
     169,   174,   164,    27,   116,   177,   176,    27,     4,   176,
     176,   964,     4,     4,   175,   957,   957,     4,     4,  1017,
       4,     4,   177,    27,   172,  1023,   177,  1076,   177,   176,
     972,   972,   773,   975,   162,   176,   176,    62,   779,    80,
     162,   983,   177,   175,   174,    27,   999,   176,     4,   175,
     177,  1010,    23,  1006,   662,  1053,   175,   665,  1017,   667,
     174,  1088,   174,   513,   838,  1007,  1007,    47,   489,   194,
     182,   174,   818,   814,   815,   656,   956,   961,   954,   900,
     956,  1133,   867,   797,  1133,   762,   920,   631,   870,   491,
     494,  1033,  1090,   916,  1053,  1053,  1062,   479,  1131,  1123,
    1042,  1043,  1043,  1045,  1045,  1076,  1131,  1069,   464,   153,
    1029,    71,    72,   622,  1112,   393,   542,   658,    -1,    -1,
      -1,  1074,    82,    83,    84,  1078,    -1,    -1,  1081,    -1,
    1089,    91,    -1,    -1,     4,    -1,    -1,    97,    98,    99,
     100,    -1,    -1,    -1,   752,    -1,    -1,    -1,    -1,    -1,
     758,    -1,   893,    -1,   762,    -1,   116,    -1,    -1,   900,
      -1,   902,  1104,  1104,    -1,  1107,  1107,  1109,  1109,    -1,
      -1,    -1,    -1,    -1,  1116,    -1,    -1,  1119,    -1,    -1,
      -1,    -1,  1124,    -1,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,   801,    -1,    -1,    -1,    -1,    -1,  1141,
    1153,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    -1,  1165,  1155,    -1,  1157,  1157,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,   966,  1168,  1168,   969,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,   846,    -1,
      -1,    -1,    -1,   851,   852,   853,   854,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,   869,   870,    -1,    44,    -1,    -1,    -1,    -1,    -1,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,
     150,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    -1,   162,    -1,    -1,    -1,   904,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   913,    -1,  1048,    -1,   179,
     135,   136,   920,   138,   139,   116,    -1,    -1,    -1,  1060,
      -1,    -1,    -1,    -1,  1065,  1066,    -1,    -1,    -1,   937,
      -1,  1072,    -1,    -1,    -1,    -1,    -1,   945,   946,   947,
     948,   166,    -1,    -1,    23,    24,    25,    26,    27,    -1,
      -1,   131,   132,   133,   134,   135,   136,   137,   138,   139,
    1101,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,   981,    -1,    -1,  1117,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,
      -1,    -1,  1010,    82,    83,    84,    -1,    -1,    -1,  1017,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1163,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,     6,   116,    -1,    -1,
      -1,    -1,    12,    13,    -1,  1053,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,   178,
      30,    -1,    44,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,  1089,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,   174,    -1,    67,    68,    69,
      70,    71,    72,   182,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    -1,   151,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,   158,    -1,
      -1,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,     6,   179,
     180,   181,   182,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,   175,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,   140,   141,   142,   143,   144,   145,   146,   147,
     148,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
       6,   179,   180,   181,   182,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,   178,    45,
      46,    47,    48,    -1,    -1,   175,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    71,   116,    73,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    -1,    -1,    -1,   174,    -1,
     176,   177,     6,   179,   180,   181,   182,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,     6,   179,   180,   181,   182,    -1,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,     6,   179,   180,   181,
     182,    -1,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,     6,    -1,   179,
     180,   181,   182,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,   158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
     179,   180,   181,   182,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    38,    39,    40,    -1,
      -1,   178,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,   178,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    38,    39,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,   178,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     177,   165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,
      -1,    44,   176,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,    -1,    -1,    -1,    38,    39,    40,
     176,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   165,   166,    -1,   168,   169,    -1,
      -1,    12,    13,    -1,    -1,   176,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    40,    -1,    30,
      -1,    44,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   158,    -1,    -1,
      -1,    -1,    -1,   164,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   175,    -1,    -1,    -1,   179,   180,
     181,   182,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,    -1,   164,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,   179,
     180,   181,   182,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    78,
      79,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,   103,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      78,    79,    -1,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,   103,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,    -1,   179,   180,   181,   182,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    74,
      75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,   104,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    51,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,    -1,   179,   180,   181,   182,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    55,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,   181,
     182,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    58,    59,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,
     180,   181,   182,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    57,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    74,    75,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    74,
      75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,   104,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,   181,
     182,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,    -1,   164,    -1,    -1,    -1,   168,   169,    -1,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,
     181,   182,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   168,   169,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,   179,
     180,   181,   182,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   168,
     169,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
     179,   180,   181,   182,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,
      -1,    -1,   179,   180,   181,   182,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
      -1,    -1,    -1,   179,   180,   181,   182,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    -1,    -1,   179,   180,   181,   182,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,    -1,    -1,    -1,
      -1,    -1,   175,    38,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
     165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,     5,
     175,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,     5,
     175,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,    -1,    -1,     5,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,    -1,    -1,    -1,    -1,    -1,   175,
      38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,   165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,
       5,   175,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,    38,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,    -1,    -1,    -1,    -1,     5,   175,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,    -1,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
     165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,    -1,
     175,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,     5,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    23,    24,    25,    26,    -1,   175,   166,
      -1,   168,   169,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,     5,   174,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,   165,   166,    -1,   168,
     169,    -1,    -1,    -1,   173,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,   165,   166,    -1,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
      -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,    44,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,   165,   166,    -1,   168,
     169,    23,    -1,    -1,    -1,    27,    -1,   166,    -1,   168,
     169,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    44,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    -1,   177,    27,    -1,    -1,    -1,
     182,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    -1,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    23,    24,    25,    26,    27,
      -1,    -1,    30,    -1,    -1,    -1,    23,    24,    25,    26,
      27,    -1,    -1,    30,    -1,    65,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    45,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    -1,   177,    -1,    -1,    -1,
      -1,   182,    -1,    91,    -1,    82,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,   178,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
     168,   169,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,    -1,   181,   182,    -1,    -1,   174,    23,    24,
      25,    26,   179,   116,   181,   182,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    -1,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    -1,   178,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,   178,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,   116,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,   178,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,   116,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,   178,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,   116,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,   178,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    -1,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    23,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   184,   189,     0,    12,    13,    17,    18,    19,    21,
      22,    23,    24,    25,    26,    27,    28,    30,    33,    34,
      35,    36,    37,    41,    42,    43,    45,    46,    47,    48,
      52,    53,    54,    56,    58,    60,    63,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    78,    79,    81,    82,
      83,    84,    91,    93,    94,    95,    96,    97,    98,    99,
     100,   103,   104,   105,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   158,   174,   176,   177,   179,   180,   181,
     182,   193,   196,   197,   198,   199,   215,   224,   227,   230,
     233,   234,   236,   238,   253,   259,   260,   261,   262,   323,
     324,   325,   326,   327,   328,   336,   338,   342,   343,   344,
     345,   347,   348,   349,   350,   351,   352,   353,   354,   365,
      23,    82,    91,   197,   325,   328,   325,   325,   325,   325,
       6,   325,   325,   174,   325,   325,   325,   325,   325,    82,
      91,   174,   193,   197,   231,   232,   233,   321,   338,   339,
     353,   355,   325,   174,   280,   341,   174,   317,   318,   325,
     215,   174,   174,   174,   174,   174,   174,   325,   346,   346,
      23,    23,   212,   316,   346,   177,   325,    23,    24,    26,
      71,    73,   191,   192,   202,   204,   208,   211,   283,   284,
     353,    27,   285,   286,   326,   280,   174,   174,   174,   174,
     229,   235,   237,   239,    23,    26,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      91,    92,    93,    94,    95,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   185,   186,   188,   190,   201,   174,
     174,   194,   195,   338,    45,   174,   179,   323,   342,   344,
     345,   352,   352,   325,   325,   325,   325,   325,   325,   325,
      27,    29,   159,   160,   161,   362,   363,   325,   213,   104,
     164,   168,   169,   187,   325,   358,   359,   360,   361,    29,
     340,   362,    29,   362,   177,   353,   280,    82,   196,   198,
     326,    97,   233,    49,    50,    49,    50,    51,     5,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    38,
      39,    40,    44,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   165,   166,   168,   169,   176,   187,
     332,   332,   162,   162,   149,   150,   179,   337,     4,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   140,   141,   167,   332,   325,   151,   325,   321,   233,
      97,   162,   280,   149,   150,   162,   179,    23,    33,    34,
      35,    36,    37,    41,    42,    45,    46,    47,    67,    71,
      72,    78,    82,    91,    94,    95,   104,   105,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   164,   175,   188,
     281,   282,   325,   325,   172,   176,    54,   325,   319,   320,
     325,   325,   212,   325,   325,   176,   176,   176,     4,   172,
     176,   176,   213,    62,   163,   192,   203,   208,   176,   172,
     176,   172,   176,     4,   172,   176,   221,   222,   352,   325,
     366,   367,   325,   175,    23,    23,    23,    23,   176,   200,
     177,   358,   358,   172,   205,   280,   339,   325,   358,   149,
     150,   179,   159,   363,    31,   325,   352,    29,   159,   363,
     175,    96,   178,   197,   198,   214,   215,   174,   325,   352,
     151,   173,   172,   180,   181,   325,   326,   228,   174,   215,
     174,     6,   176,     6,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   339,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   329,    23,    91,   223,   329,   177,
     188,   353,   356,   177,   188,   353,   356,    23,   177,   353,
     357,   357,   346,   280,   187,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   175,
     325,   175,   322,   353,   357,   357,   353,   346,   175,   325,
       6,   172,   205,   175,   318,   174,   175,   176,   172,    62,
     175,   175,   175,   325,   316,   178,    23,   177,   163,   176,
     176,   192,   211,   284,   325,   286,   172,   205,   172,   205,
     175,   176,   101,   243,   329,   101,   244,     6,   240,   177,
     189,   175,   175,   194,   173,   175,   173,    23,    23,    13,
      23,    27,    32,   364,   178,   179,   178,   178,   174,   198,
     358,   104,   187,   325,     4,   359,   178,    23,   325,   325,
     213,   325,     6,   174,   329,   174,   325,   280,   325,   280,
     325,   280,   280,   173,   352,   341,   173,   325,   164,   282,
     175,     6,   215,   325,     6,   215,   258,   319,   325,   104,
     179,   187,   246,   352,   216,     6,   177,   250,   177,   329,
     217,   191,   202,   206,   209,   210,   177,   222,   175,   367,
     175,   338,   102,   245,   177,   290,   338,   329,     5,    82,
     105,   106,   174,   193,   269,   270,   271,   272,   273,   275,
     245,   189,   178,     4,    32,   173,   325,   175,   175,   174,
     352,   325,   243,   175,   175,    51,   325,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,   197,   263,   264,
     265,   266,   267,   302,   303,   174,   263,   178,   178,   178,
     243,   213,   175,   213,   176,   174,   358,   352,   151,   175,
       6,   215,   249,   176,   251,   176,   251,    66,   254,   255,
     256,   257,   325,    76,    77,   220,    62,   210,   172,   205,
     207,   210,   176,   290,   329,   287,   172,   177,   270,   270,
     273,   169,     7,     7,   169,   329,   178,   325,   173,   176,
     358,   245,   215,     6,   176,   267,   175,   172,   205,     5,
     174,   268,   274,   275,   276,   277,   278,   303,   263,   175,
     245,   176,    55,   319,   358,   173,   246,     6,   215,   248,
     213,   251,    64,    65,    66,   251,   178,   172,   205,   178,
     172,   205,   172,   205,   174,   177,    23,   209,   178,   172,
     205,   177,    65,    79,    92,   178,   197,   241,   288,   289,
     299,   300,   301,   302,   338,   287,   175,   270,   270,   271,
     271,   270,   177,   178,   175,   329,   213,     6,   279,   265,
     275,   275,   278,   168,   225,   169,     7,     7,   169,   175,
      79,   333,   329,   176,   175,   175,   175,   213,    61,    64,
     176,   325,     6,   176,   252,   178,   151,   256,   325,   151,
     218,   338,   213,   210,   178,   287,   329,   290,   288,   268,
     328,    73,   178,   287,   177,   269,   151,   175,   164,   226,
     275,   275,   276,   276,   275,   279,   174,   279,   177,     6,
     215,   247,   248,    59,   176,   176,   252,   213,   325,   325,
       7,    27,   219,   178,   178,   188,   176,   177,   291,    27,
     304,   305,   306,   332,    23,    82,   105,   106,   186,   269,
     314,   315,   178,   287,   330,    27,   330,    27,   187,   334,
     335,   330,   287,   213,   176,   213,   338,   175,     4,   242,
      82,   178,   188,   292,   293,   294,   295,   296,   297,   338,
       4,   329,   172,   176,   188,     4,     4,    23,   314,   172,
     176,   178,   331,   329,   177,    27,   172,   205,   177,   178,
      57,   177,   325,   176,   178,   293,   176,   176,    62,    80,
     162,   325,   177,    27,   305,   329,   325,   325,   176,   315,
     325,     4,   177,   308,   213,   335,   175,   213,   176,   213,
      23,   185,   303,   290,   188,   329,   307,     4,   329,   174,
     329,   329,   330,   325,   307,   178,   178,   178,   188,   177,
     178,   197,   302,   309,   310,   325,   263,   308,   178,   330,
     330,   307,   310,   332,   329,   175,   178,    23,   279,   329,
     330,   311,   176,   177,   298,   174,   313,   213,   330,   263,
     330,   178,   175,   151,   176,   177,   312,   325,   213,   330,
     176,   178
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   183,   184,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   186,   186,   186,   186,   186,   186,
     186,   186,   187,   187,   188,   188,   189,   189,   190,   190,
     191,   191,   192,   192,   193,   193,   193,   193,   194,   194,
     195,   195,   196,   197,   197,   198,   198,   198,   198,   198,
     199,   199,   199,   199,   199,   200,   199,   201,   199,   199,
     199,   199,   199,   199,   202,   202,   203,   204,   205,   205,
     206,   206,   207,   207,   208,   208,   209,   209,   210,   210,
     211,   211,   212,   212,   213,   213,   214,   214,   214,   214,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   216,   215,
     215,   215,   215,   215,   217,   217,   218,   218,   219,   219,
     220,   220,   221,   221,   222,   223,   223,   224,   225,   225,
     226,   226,   228,   227,   229,   227,   230,   230,   231,   231,
     232,   232,   233,   233,   233,   235,   234,   237,   236,   239,
     238,   240,   240,   241,   242,   242,   243,   243,   244,   244,
     245,   245,   246,   246,   246,   246,   247,   247,   248,   248,
     249,   249,   250,   250,   250,   250,   251,   251,   251,   252,
     252,   253,   254,   254,   255,   255,   256,   256,   257,   257,
     258,   258,   259,   259,   260,   260,   261,   261,   262,   262,
     263,   263,   264,   264,   265,   265,   266,   266,   267,   267,
     268,   268,   269,   269,   269,   269,   270,   270,   271,   271,
     272,   272,   273,   273,   274,   274,   274,   274,   275,   275,
     275,   276,   276,   277,   277,   278,   278,   279,   279,   280,
     280,   280,   281,   281,   282,   282,   282,   283,   283,   284,
     285,   285,   286,   286,   287,   287,   288,   288,   288,   288,
     288,   288,   289,   289,   289,   290,   290,   291,   291,   291,
     292,   292,   293,   293,   294,   295,   295,   295,   295,   296,
     296,   297,   298,   298,   299,   299,   300,   300,   301,   301,
     302,   302,   303,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   304,   304,   305,   305,   306,   306,   307,   307,
     307,   308,   308,   309,   309,   311,   310,   312,   312,   312,
     313,   313,   314,   314,   315,   315,   316,   317,   317,   318,
     319,   319,   320,   320,   322,   321,   323,   323,   323,   324,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   326,   326,   327,   328,   329,   330,
     331,   332,   332,   333,   333,   334,   334,   335,   335,   336,
     336,   336,   336,   337,   336,   338,   338,   339,   339,   339,
     340,   340,   340,   341,   341,   342,   342,   342,   342,   343,
     343,   343,   343,   343,   343,   343,   343,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   345,   345,   345,
     345,   346,   346,   347,   348,   348,   348,   348,   348,   349,
     349,   350,   350,   350,   350,   351,   351,   351,   351,   351,
     352,   352,   352,   352,   353,   353,   353,   354,   354,   355,
     355,   355,   355,   355,   355,   356,   356,   356,   357,   357,
     357,   358,   359,   359,   360,   360,   361,   361,   361,   361,
     361,   361,   361,   362,   362,   362,   362,   363,   363,   363,
     363,   363,   363,   363,   363,   364,   364,   364,   364,   365,
     365,   365,   365,   365,   365,   365,   366,   366,   367
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     3,     4,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     2,     4,     3,     0,     6,     0,     5,     3,
       4,     3,     4,     3,     1,     1,     6,     6,     0,     1,
       3,     1,     3,     1,     3,     1,     1,     2,     1,     3,
       1,     3,     3,     1,     2,     0,     1,     1,     2,     4,
       3,     1,     1,     5,     7,     9,     5,     3,     3,     3,
       3,     3,     3,     1,     2,     6,     7,     9,     0,     6,
       1,     6,     3,     2,     0,     9,     1,     3,     0,     1,
       0,     4,     1,     3,     1,     1,     1,    13,     0,     1,
       0,     1,     0,    10,     0,     9,     1,     2,     1,     2,
       0,     1,     1,     1,     1,     0,     7,     0,     8,     0,
       9,     0,     2,     5,     0,     2,     0,     2,     0,     2,
       0,     2,     1,     2,     4,     3,     1,     4,     1,     4,
       1,     4,     3,     4,     4,     5,     0,     5,     4,     1,
       1,     7,     0,     2,     1,     3,     4,     4,     1,     3,
       1,     4,     5,     6,     1,     3,     6,     7,     3,     6,
       2,     0,     1,     3,     2,     1,     0,     1,     7,     9,
       0,     1,     1,     2,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     1,     2,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     0,     2,     2,
       4,     3,     1,     3,     1,     3,     2,     3,     1,     1,
       3,     1,     1,     3,     2,     0,     4,     3,     4,     5,
      12,     1,     1,     2,     3,     1,     3,     1,     2,     3,
       1,     2,     2,     2,     3,     3,     3,     4,     3,     1,
       1,     3,     1,     3,     1,     1,     0,     1,     0,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     4,     5,     7,     0,     2,
       3,     0,     3,     0,     1,     0,     9,     1,     3,     3,
       0,     3,     3,     1,     4,     4,     4,     3,     1,     1,
       0,     1,     3,     1,     0,    10,     3,     2,     3,     2,
       1,     6,     5,     3,     4,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     5,     4,
       3,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     3,     2,     1,     2,     4,     2,     2,     1,
       2,     2,     3,     1,    13,    12,     1,     1,     0,     0,
       0,     0,     1,     0,     5,     3,     1,     1,     2,     2,
       2,     4,     4,     0,     3,     1,     1,     1,     1,     3,
       0,     1,     1,     0,     1,     4,     3,     1,     3,     1,
       1,     3,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     5,
       5,     0,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     4,     4,     4,     1,
       1,     1,     3,     3,     1,     4,     2,     3,     3,     1,
       4,     3,     3,     3,     3,     1,     3,     1,     1,     3,
       1,     1,     0,     1,     3,     1,     3,     1,     4,     2,
       2,     6,     4,     2,     2,     1,     2,     1,     4,     3,
       3,     3,     3,     6,     3,     1,     1,     2,     1,     5,
       4,     2,     2,     4,     2,     2,     1,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ZENDEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ZENDEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ZENDerror or ZENDUNDEF. */
#define YYERRCODE ZENDUNDEF


/* Enable debugging if requested.  */
#if ZENDDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ZENDDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ZENDDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_T_LNUMBER: /* "integer"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_DNUMBER: /* "floating-point number"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_STRING: /* "identifier"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_FULLY_QUALIFIED: /* "fully qualified name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_RELATIVE: /* "namespace-relative name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_QUALIFIED: /* "namespaced name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_VARIABLE: /* "variable"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_INLINE_HTML: /* T_INLINE_HTML  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_ENCAPSED_AND_WHITESPACE: /* "string content"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_CONSTANT_ENCAPSED_STRING: /* "quoted string"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_STRING_VARNAME: /* "variable name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NUM_STRING: /* "number"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_identifier: /* identifier  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_top_statement_list: /* top_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_namespace_declaration_name: /* namespace_declaration_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_namespace_name: /* namespace_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_legacy_namespace_name: /* legacy_namespace_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_name: /* name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute_decl: /* attribute_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute_group: /* attribute_group  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute: /* attribute  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributes: /* attributes  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_statement: /* attributed_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_top_statement: /* top_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_group_use_declaration: /* group_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_mixed_group_use_declaration: /* mixed_group_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_use_declarations: /* inline_use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unprefixed_use_declarations: /* unprefixed_use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_use_declarations: /* use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_use_declaration: /* inline_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unprefixed_use_declaration: /* unprefixed_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_use_declaration: /* use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_const_list: /* const_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inner_statement_list: /* inner_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inner_statement: /* inner_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_statement: /* statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_catch_list: /* catch_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_catch_name_list: /* catch_name_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_variable: /* optional_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_finally_statement: /* finally_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unset_variables: /* unset_variables  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unset_variable: /* unset_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_function_name: /* function_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_function_declaration_statement: /* function_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_declaration_statement: /* class_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_declaration_statement: /* trait_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_interface_declaration_statement: /* interface_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_declaration_statement: /* enum_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_backing_type: /* enum_backing_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_case: /* enum_case  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_case_expr: /* enum_case_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_extends_from: /* extends_from  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_interface_extends_list: /* interface_extends_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_implements_list: /* implements_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_foreach_variable: /* foreach_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_for_statement: /* for_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_foreach_statement: /* foreach_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_declare_statement: /* declare_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_switch_case_list: /* switch_case_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_case_list: /* case_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match: /* match  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm_list: /* match_arm_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_match_arm_list: /* non_empty_match_arm_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm: /* match_arm  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm_cond_list: /* match_arm_cond_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_while_statement: /* while_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_if_stmt_without_else: /* if_stmt_without_else  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_if_stmt: /* if_stmt  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_alt_if_stmt_without_else: /* alt_if_stmt_without_else  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_alt_if_stmt: /* alt_if_stmt  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_parameter_list: /* parameter_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_parameter_list: /* non_empty_parameter_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_parameter: /* attributed_parameter  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_parameter: /* parameter  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_type_without_static: /* optional_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_expr: /* type_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type: /* type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type_element: /* union_type_element  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type: /* union_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_intersection_type: /* intersection_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_expr_without_static: /* type_expr_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_without_static: /* type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type_without_static_element: /* union_type_without_static_element  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type_without_static: /* union_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_intersection_type_without_static: /* intersection_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_return_type: /* return_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument_list: /* argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_argument_list: /* non_empty_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument: /* argument  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_global_var_list: /* global_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_global_var: /* global_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_var_list: /* static_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_var: /* static_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_statement_list: /* class_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_class_statement: /* attributed_class_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_statement: /* class_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name_list: /* class_name_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptations: /* trait_adaptations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptation_list: /* trait_adaptation_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptation: /* trait_adaptation  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_precedence: /* trait_precedence  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_alias: /* trait_alias  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_method_reference: /* trait_method_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_absolute_trait_method_reference: /* absolute_trait_method_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_method_body: /* method_body  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_member_modifiers: /* non_empty_member_modifiers  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_list: /* property_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property: /* property  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_hooked_property: /* hooked_property  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook_list: /* property_hook_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_property_hook_list: /* optional_property_hook_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook: /* property_hook  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook_body: /* property_hook_body  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_parameter_list: /* optional_parameter_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_const_list: /* class_const_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_const_decl: /* class_const_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_const_decl: /* const_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_echo_expr_list: /* echo_expr_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_echo_expr: /* echo_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_for_exprs: /* for_exprs  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_for_exprs: /* non_empty_for_exprs  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_anonymous_class: /* anonymous_class  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_dereferenceable: /* new_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_non_dereferenceable: /* new_non_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_expr: /* expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_function: /* inline_function  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_backup_doc_comment: /* backup_doc_comment  */
            { if (((*yyvaluep).str)) zend_string_release_ex(((*yyvaluep).str), 0); }
        break;

    case YYSYMBOL_lexical_vars: /* lexical_vars  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_lexical_var_list: /* lexical_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_lexical_var: /* lexical_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_function_call: /* function_call  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name: /* class_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name_reference: /* class_name_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_backticks_expr: /* backticks_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_ctor_arguments: /* ctor_arguments  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_dereferenceable_scalar: /* dereferenceable_scalar  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_scalar: /* scalar  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_constant: /* constant  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_constant: /* class_constant  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_expr: /* optional_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_variable_class_name: /* variable_class_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_fully_dereferenceable: /* fully_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_object_dereferenceable: /* array_object_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_callable_expr: /* callable_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_callable_variable: /* callable_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_variable: /* variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_simple_variable: /* simple_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_member: /* static_member  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_variable: /* new_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_member_name: /* member_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_name: /* property_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_pair_list: /* array_pair_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_possible_array_pair: /* possible_array_pair  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_array_pair_list: /* non_empty_array_pair_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_pair: /* array_pair  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_list: /* encaps_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_var: /* encaps_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_var_offset: /* encaps_var_offset  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_internal_functions_in_yacc: /* internal_functions_in_yacc  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_isset_variables: /* isset_variables  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_isset_variable: /* isset_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ZENDEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ZENDEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= END)
    {
      yychar = END;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ZENDerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ZENDUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = ZENDEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: top_statement_list  */
                                { CG(ast) = (yyvsp[0].ast); (void) zendnerrs; }
    break;

  case 84: /* identifier: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 85: /* identifier: semi_reserved  */
                               {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 86: /* top_statement_list: top_statement_list top_statement  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 87: /* top_statement_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 88: /* namespace_declaration_name: identifier  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 89: /* namespace_declaration_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 90: /* namespace_name: "identifier"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 91: /* namespace_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 92: /* legacy_namespace_name: namespace_name  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 93: /* legacy_namespace_name: "fully qualified name"  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 94: /* name: "identifier"  */
                                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 95: /* name: "namespaced name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 96: /* name: "fully qualified name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }
    break;

  case 97: /* name: "namespace-relative name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }
    break;

  case 98: /* attribute_decl: class_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[0].ast), NULL); }
    break;

  case 99: /* attribute_decl: class_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 100: /* attribute_group: attribute_decl  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_GROUP, (yyvsp[0].ast)); }
    break;

  case 101: /* attribute_group: attribute_group ',' attribute_decl  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 102: /* attribute: "'#['" attribute_group possible_comma ']'  */
                                                                { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 103: /* attributes: attribute  */
                                                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_LIST, (yyvsp[0].ast)); }
    break;

  case 104: /* attributes: attributes attribute  */
                                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 105: /* attributed_statement: function_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 106: /* attributed_statement: class_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 107: /* attributed_statement: trait_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 108: /* attributed_statement: interface_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 109: /* attributed_statement: enum_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 110: /* top_statement: statement  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 111: /* top_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 112: /* top_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 113: /* top_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }
    break;

  case 114: /* top_statement: "'namespace'" namespace_declaration_name ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }
    break;

  case 115: /* $@1: %empty  */
                                                       { RESET_DOC_COMMENT(); }
    break;

  case 116: /* top_statement: "'namespace'" namespace_declaration_name $@1 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 117: /* $@2: %empty  */
                            { RESET_DOC_COMMENT(); }
    break;

  case 118: /* top_statement: "'namespace'" $@2 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }
    break;

  case 119: /* top_statement: "'use'" mixed_group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 120: /* top_statement: "'use'" use_type group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 121: /* top_statement: "'use'" use_declarations ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 122: /* top_statement: "'use'" use_type use_declarations ';'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 123: /* top_statement: "'const'" const_list ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 124: /* use_type: "'function'"  */
                                        { (yyval.num) = ZEND_SYMBOL_FUNCTION; }
    break;

  case 125: /* use_type: "'const'"  */
                                        { (yyval.num) = ZEND_SYMBOL_CONST; }
    break;

  case 126: /* group_use_declaration: legacy_namespace_name "'\\'" '{' unprefixed_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 127: /* mixed_group_use_declaration: legacy_namespace_name "'\\'" '{' inline_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}
    break;

  case 130: /* inline_use_declarations: inline_use_declarations ',' inline_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 131: /* inline_use_declarations: inline_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 132: /* unprefixed_use_declarations: unprefixed_use_declarations ',' unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 133: /* unprefixed_use_declarations: unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 134: /* use_declarations: use_declarations ',' use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 135: /* use_declarations: use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 136: /* inline_use_declaration: unprefixed_use_declaration  */
                                           { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 137: /* inline_use_declaration: use_type unprefixed_use_declaration  */
                                                    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }
    break;

  case 138: /* unprefixed_use_declaration: namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 139: /* unprefixed_use_declaration: namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 140: /* use_declaration: legacy_namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 141: /* use_declaration: legacy_namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 142: /* const_list: const_list ',' const_decl  */
                                          { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 143: /* const_list: const_decl  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 144: /* inner_statement_list: inner_statement_list inner_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 145: /* inner_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 146: /* inner_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 147: /* inner_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 148: /* inner_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 149: /* inner_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }
    break;

  case 150: /* statement: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 151: /* statement: if_stmt  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 152: /* statement: alt_if_stmt  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 153: /* statement: "'while'" '(' expr ')' while_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 154: /* statement: "'do'" statement "'while'" '(' expr ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 155: /* statement: "'for'" '(' for_exprs ';' for_exprs ';' for_exprs ')' for_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 156: /* statement: "'switch'" '(' expr ')' switch_case_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 157: /* statement: "'break'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }
    break;

  case 158: /* statement: "'continue'" optional_expr ';'  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }
    break;

  case 159: /* statement: "'return'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }
    break;

  case 160: /* statement: "'global'" global_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 161: /* statement: "'static'" static_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 162: /* statement: "'echo'" echo_expr_list ';'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 163: /* statement: T_INLINE_HTML  */
                              { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 164: /* statement: expr ';'  */
                         { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 165: /* statement: "'unset'" '(' unset_variables possible_comma ')' ';'  */
                                                                   { (yyval.ast) = (yyvsp[-3].ast); }
    break;

  case 166: /* statement: "'foreach'" '(' expr "'as'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 167: /* statement: "'foreach'" '(' expr "'as'" foreach_variable "'=>'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }
    break;

  case 168: /* $@3: %empty  */
                        { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }
    break;

  case 169: /* statement: "'declare'" '(' const_list ')' $@3 declare_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 170: /* statement: ';'  */
                                              { (yyval.ast) = NULL; }
    break;

  case 171: /* statement: "'try'" '{' inner_statement_list '}' catch_list finally_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 172: /* statement: "'goto'" "identifier" ';'  */
                                    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }
    break;

  case 173: /* statement: "identifier" ':'  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }
    break;

  case 174: /* catch_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }
    break;

  case 175: /* catch_list: catch_list "'catch'" '(' catch_name_list optional_variable ')' '{' inner_statement_list '}'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }
    break;

  case 176: /* catch_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 177: /* catch_name_list: catch_name_list '|' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 178: /* optional_variable: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 179: /* optional_variable: "variable"  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 180: /* finally_statement: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 181: /* finally_statement: "'finally'" '{' inner_statement_list '}'  */
                                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 182: /* unset_variables: unset_variable  */
                               { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 183: /* unset_variables: unset_variables ',' unset_variable  */
                                                   { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 184: /* unset_variable: variable  */
                         { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }
    break;

  case 185: /* function_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 186: /* function_name: "'readonly'"  */
                           {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 187: /* function_declaration_statement: function returns_ref function_name backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 188: /* is_reference: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 189: /* is_reference: "'&'"  */
                                                        { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 190: /* is_variadic: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 191: /* is_variadic: "'...'"  */
                            { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 192: /* @4: %empty  */
                                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 193: /* class_declaration_statement: class_modifiers "'class'" @4 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 194: /* @5: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 195: /* class_declaration_statement: "'class'" @5 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 196: /* class_modifiers: class_modifier  */
                                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 197: /* class_modifiers: class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 198: /* anonymous_class_modifiers: class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier(0, (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 199: /* anonymous_class_modifiers: anonymous_class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 200: /* anonymous_class_modifiers_optional: %empty  */
                                                { (yyval.num) = 0; }
    break;

  case 201: /* anonymous_class_modifiers_optional: anonymous_class_modifiers  */
                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 202: /* class_modifier: "'abstract'"  */
                                        { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 203: /* class_modifier: "'final'"  */
                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 204: /* class_modifier: "'readonly'"  */
                                        { (yyval.num) = ZEND_ACC_READONLY_CLASS|ZEND_ACC_NO_DYNAMIC_PROPERTIES; }
    break;

  case 205: /* @6: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 206: /* trait_declaration_statement: "'trait'" @6 "identifier" backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 207: /* @7: %empty  */
                            { (yyval.num) = CG(zend_lineno); }
    break;

  case 208: /* interface_declaration_statement: "'interface'" @7 "identifier" interface_extends_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 209: /* @8: %empty  */
                       { (yyval.num) = CG(zend_lineno); }
    break;

  case 210: /* enum_declaration_statement: "'enum'" @8 "identifier" enum_backing_type implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }
    break;

  case 211: /* enum_backing_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 212: /* enum_backing_type: ':' type_expr  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 213: /* enum_case: "'case'" backup_doc_comment identifier enum_case_expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }
    break;

  case 214: /* enum_case_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 215: /* enum_case_expr: '=' expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 216: /* extends_from: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 217: /* extends_from: "'extends'" class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 218: /* interface_extends_list: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 219: /* interface_extends_list: "'extends'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 220: /* implements_list: %empty  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 221: /* implements_list: "'implements'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 222: /* foreach_variable: variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 223: /* foreach_variable: ampersand variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }
    break;

  case 224: /* foreach_variable: "'list'" '(' array_pair_list ')'  */
                                               { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 225: /* foreach_variable: '[' array_pair_list ']'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 226: /* for_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 227: /* for_statement: ':' inner_statement_list "'endfor'" ';'  */
                                                      { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 228: /* foreach_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 229: /* foreach_statement: ':' inner_statement_list "'endforeach'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 230: /* declare_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 231: /* declare_statement: ':' inner_statement_list "'enddeclare'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 232: /* switch_case_list: '{' case_list '}'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 233: /* switch_case_list: '{' ';' case_list '}'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 234: /* switch_case_list: ':' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 235: /* switch_case_list: ':' ';' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 236: /* case_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 237: /* case_list: case_list "'case'" expr case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 238: /* case_list: case_list "'default'" case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }
    break;

  case 241: /* match: "'match'" '(' expr ')' '{' match_arm_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 242: /* match_arm_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }
    break;

  case 243: /* match_arm_list: non_empty_match_arm_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 244: /* non_empty_match_arm_list: match_arm  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }
    break;

  case 245: /* non_empty_match_arm_list: non_empty_match_arm_list ',' match_arm  */
                                                       { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 246: /* match_arm: match_arm_cond_list possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 247: /* match_arm: "'default'" possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }
    break;

  case 248: /* match_arm_cond_list: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 249: /* match_arm_cond_list: match_arm_cond_list ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 250: /* while_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 251: /* while_statement: ':' inner_statement_list "'endwhile'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 252: /* if_stmt_without_else: "'if'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 253: /* if_stmt_without_else: if_stmt_without_else "'elseif'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 254: /* if_stmt: if_stmt_without_else  */
                                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 255: /* if_stmt: if_stmt_without_else "'else'" statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }
    break;

  case 256: /* alt_if_stmt_without_else: "'if'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 257: /* alt_if_stmt_without_else: alt_if_stmt_without_else "'elseif'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 258: /* alt_if_stmt: alt_if_stmt_without_else "'endif'" ';'  */
                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 259: /* alt_if_stmt: alt_if_stmt_without_else "'else'" ':' inner_statement_list "'endif'" ';'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }
    break;

  case 260: /* parameter_list: non_empty_parameter_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 261: /* parameter_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 262: /* non_empty_parameter_list: attributed_parameter  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }
    break;

  case 263: /* non_empty_parameter_list: non_empty_parameter_list ',' attributed_parameter  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 264: /* attributed_parameter: attributes parameter  */
                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 265: /* attributed_parameter: parameter  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 266: /* optional_cpp_modifiers: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 267: /* optional_cpp_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CPP, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 268: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-6].num) | (yyvsp[-4].num) | (yyvsp[-3].num), (yyvsp[-5].ast), (yyvsp[-2].ast), NULL,
					NULL, (yyvsp[-1].str) ? zend_ast_create_zval_from_str((yyvsp[-1].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 269: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment '=' expr optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-8].num) | (yyvsp[-6].num) | (yyvsp[-5].num), (yyvsp[-7].ast), (yyvsp[-4].ast), (yyvsp[-1].ast),
					NULL, (yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 270: /* optional_type_without_static: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 271: /* optional_type_without_static: type_expr_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 272: /* type_expr: type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 273: /* type_expr: '?' type  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 274: /* type_expr: union_type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 275: /* type_expr: intersection_type  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 276: /* type: type_without_static  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 277: /* type: "'static'"  */
                                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }
    break;

  case 278: /* union_type_element: type  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 279: /* union_type_element: '(' intersection_type ')'  */
                                           { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 280: /* union_type: union_type_element '|' union_type_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 281: /* union_type: union_type '|' union_type_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 282: /* intersection_type: type "amp" type  */
                                                                          { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 283: /* intersection_type: intersection_type "amp" type  */
                                                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 284: /* type_expr_without_static: type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 285: /* type_expr_without_static: '?' type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 286: /* type_expr_without_static: union_type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 287: /* type_expr_without_static: intersection_type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 288: /* type_without_static: "'array'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 289: /* type_without_static: "'callable'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 290: /* type_without_static: name  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 291: /* union_type_without_static_element: type_without_static  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 292: /* union_type_without_static_element: '(' intersection_type_without_static ')'  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 293: /* union_type_without_static: union_type_without_static_element '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 294: /* union_type_without_static: union_type_without_static '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 295: /* intersection_type_without_static: type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 296: /* intersection_type_without_static: intersection_type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 297: /* return_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 298: /* return_type: ':' type_expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 299: /* argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 300: /* argument_list: '(' non_empty_argument_list possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 301: /* argument_list: '(' "'...'" ')'  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_CALLABLE_CONVERT); }
    break;

  case 302: /* non_empty_argument_list: argument  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 303: /* non_empty_argument_list: non_empty_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 304: /* argument: expr  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 305: /* argument: identifier ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 306: /* argument: "'...'" expr  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 307: /* global_var_list: global_var_list ',' global_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 308: /* global_var_list: global_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 309: /* global_var: simple_variable  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }
    break;

  case 310: /* static_var_list: static_var_list ',' static_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 311: /* static_var_list: static_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 312: /* static_var: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }
    break;

  case 313: /* static_var: "variable" '=' expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 314: /* class_statement_list: class_statement_list class_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 315: /* class_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 316: /* attributed_class_statement: property_modifiers optional_type_without_static property_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 317: /* attributed_class_statement: property_modifiers optional_type_without_static hooked_property  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-1].ast), zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)), NULL);
			  (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 318: /* attributed_class_statement: class_const_modifiers "'const'" class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 319: /* attributed_class_statement: class_const_modifiers "'const'" type_expr class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, (yyvsp[-2].ast));
			  (yyval.ast)->attr = (yyvsp[-4].num); }
    break;

  case 320: /* attributed_class_statement: method_modifiers function returns_ref identifier backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags method_body backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 321: /* attributed_class_statement: enum_case  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 322: /* class_statement: attributed_class_statement  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 323: /* class_statement: attributes attributed_class_statement  */
                                                      { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 324: /* class_statement: "'use'" class_name_list trait_adaptations  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 325: /* class_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 326: /* class_name_list: class_name_list ',' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 327: /* trait_adaptations: ';'  */
                                                                                { (yyval.ast) = NULL; }
    break;

  case 328: /* trait_adaptations: '{' '}'  */
                                                                        { (yyval.ast) = NULL; }
    break;

  case 329: /* trait_adaptations: '{' trait_adaptation_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 330: /* trait_adaptation_list: trait_adaptation  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }
    break;

  case 331: /* trait_adaptation_list: trait_adaptation_list trait_adaptation  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 332: /* trait_adaptation: trait_precedence ';'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 333: /* trait_adaptation: trait_alias ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 334: /* trait_precedence: absolute_trait_method_reference "'insteadof'" class_name_list  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 335: /* trait_alias: trait_method_reference "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 336: /* trait_alias: trait_method_reference "'as'" reserved_non_modifiers  */
                        { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }
    break;

  case 337: /* trait_alias: trait_method_reference "'as'" member_modifier identifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[-1].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-3].ast), (yyvsp[0].ast));
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 338: /* trait_alias: trait_method_reference "'as'" member_modifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-2].ast), NULL);
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 339: /* trait_method_reference: identifier  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }
    break;

  case 340: /* trait_method_reference: absolute_trait_method_reference  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 341: /* absolute_trait_method_reference: class_name "'::'" identifier  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 342: /* method_body: ';'  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 343: /* method_body: '{' inner_statement_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 344: /* property_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 345: /* property_modifiers: "'var'"  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 346: /* method_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 347: /* method_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 348: /* class_const_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 349: /* class_const_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CONSTANT, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 350: /* non_empty_member_modifiers: member_modifier  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MODIFIER_LIST, zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 351: /* non_empty_member_modifiers: non_empty_member_modifiers member_modifier  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 352: /* member_modifier: "'public'"  */
                                                        { (yyval.num) = T_PUBLIC; }
    break;

  case 353: /* member_modifier: "'protected'"  */
                                                        { (yyval.num) = T_PROTECTED; }
    break;

  case 354: /* member_modifier: "'private'"  */
                                                        { (yyval.num) = T_PRIVATE; }
    break;

  case 355: /* member_modifier: "'public(set)'"  */
                                                { (yyval.num) = T_PUBLIC_SET; }
    break;

  case 356: /* member_modifier: "'protected(set)'"  */
                                                { (yyval.num) = T_PROTECTED_SET; }
    break;

  case 357: /* member_modifier: "'private(set)'"  */
                                                { (yyval.num) = T_PRIVATE_SET; }
    break;

  case 358: /* member_modifier: "'static'"  */
                                                        { (yyval.num) = T_STATIC; }
    break;

  case 359: /* member_modifier: "'abstract'"  */
                                                        { (yyval.num) = T_ABSTRACT; }
    break;

  case 360: /* member_modifier: "'final'"  */
                                                        { (yyval.num) = T_FINAL; }
    break;

  case 361: /* member_modifier: "'readonly'"  */
                                                        { (yyval.num) = T_READONLY; }
    break;

  case 362: /* property_list: property_list ',' property  */
                                           { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 363: /* property_list: property  */
                         { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }
    break;

  case 364: /* property: "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 365: /* property: "variable" '=' expr backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 366: /* hooked_property: "variable" backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-4].ast), NULL, ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 367: /* hooked_property: "variable" '=' expr backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-6].ast), (yyvsp[-4].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 368: /* property_hook_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 369: /* property_hook_list: property_hook_list property_hook  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 370: /* property_hook_list: property_hook_list attributes property_hook  */
                                                            {
			(yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)));
		}
    break;

  case 371: /* optional_property_hook_list: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 372: /* optional_property_hook_list: '{' property_hook_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 373: /* property_hook_modifiers: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 374: /* property_hook_modifiers: non_empty_member_modifiers  */
                                           {
			(yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY_HOOK, (yyvsp[0].ast));
			if (!(yyval.num)) { YYERROR; }
		}
    break;

  case 375: /* @9: %empty  */
                                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 376: /* property_hook: property_hook_modifiers returns_ref "identifier" backup_doc_comment @9 optional_parameter_list backup_fn_flags property_hook_body backup_fn_flags  */
                                                                                           {
			(yyval.ast) = zend_ast_create_decl(
				ZEND_AST_PROPERTY_HOOK, (yyvsp[-8].num) | (yyvsp[-7].num) | (yyvsp[0].num), (yyvsp[-4].num), (yyvsp[-5].str), zend_ast_get_str((yyvsp[-6].ast)),
				(yyvsp[-3].ast), NULL, (yyvsp[-1].ast), NULL, NULL);
			CG(extra_fn_flags) = (yyvsp[-2].num);
		}
    break;

  case 377: /* property_hook_body: ';'  */
                    { (yyval.ast) = NULL; }
    break;

  case 378: /* property_hook_body: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 379: /* property_hook_body: "'=>'" expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROPERTY_HOOK_SHORT_BODY, (yyvsp[-1].ast)); }
    break;

  case 380: /* optional_parameter_list: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 381: /* optional_parameter_list: '(' parameter_list ')'  */
                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 382: /* class_const_list: class_const_list ',' class_const_decl  */
                                                      { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 383: /* class_const_list: class_const_decl  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 384: /* class_const_decl: "identifier" '=' expr backup_doc_comment  */
                                                     { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 385: /* class_const_decl: semi_reserved '=' expr backup_doc_comment  */
                                                          {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-3].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, zend_ast_create_zval(&zv), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL));
		}
    break;

  case 386: /* const_decl: "identifier" '=' expr backup_doc_comment  */
                                             { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 387: /* echo_expr_list: echo_expr_list ',' echo_expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 388: /* echo_expr_list: echo_expr  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 389: /* echo_expr: expr  */
             { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 390: /* for_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 391: /* for_exprs: non_empty_for_exprs  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 392: /* non_empty_for_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 393: /* non_empty_for_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 394: /* @10: %empty  */
                                                           { (yyval.num) = CG(zend_lineno); }
    break;

  case 395: /* anonymous_class: anonymous_class_modifiers_optional "'class'" @10 ctor_arguments extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                                                                                             {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS | (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}
    break;

  case 396: /* new_dereferenceable: "'new'" class_name_reference argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 397: /* new_dereferenceable: "'new'" anonymous_class  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 398: /* new_dereferenceable: "'new'" attributes anonymous_class  */
                        { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 399: /* new_non_dereferenceable: "'new'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[0].ast), zend_ast_create_list(0, ZEND_AST_ARG_LIST)); }
    break;

  case 400: /* expr: variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 401: /* expr: "'list'" '(' array_pair_list ')' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 402: /* expr: '[' array_pair_list ']' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 403: /* expr: variable '=' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 404: /* expr: variable '=' ampersand variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 405: /* expr: "'clone'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }
    break;

  case 406: /* expr: variable "'+='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 407: /* expr: variable "'-='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 408: /* expr: variable "'*='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 409: /* expr: variable "'**='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 410: /* expr: variable "'/='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 411: /* expr: variable "'.='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 412: /* expr: variable "'%='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 413: /* expr: variable "'&='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 414: /* expr: variable "'|='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 415: /* expr: variable "'^='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 416: /* expr: variable "'<<='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 417: /* expr: variable "'>>='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 418: /* expr: variable "'??='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 419: /* expr: variable "'++'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }
    break;

  case 420: /* expr: "'++'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }
    break;

  case 421: /* expr: variable "'--'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }
    break;

  case 422: /* expr: "'--'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }
    break;

  case 423: /* expr: expr "'||'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 424: /* expr: expr "'&&'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 425: /* expr: expr "'or'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 426: /* expr: expr "'and'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 427: /* expr: expr "'xor'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 428: /* expr: expr '|' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 429: /* expr: expr "amp" expr  */
                                                                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 430: /* expr: expr "'&'" expr  */
                                                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 431: /* expr: expr '^' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 432: /* expr: expr '.' expr  */
                                { (yyval.ast) = zend_ast_create_concat_op((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 433: /* expr: expr '+' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 434: /* expr: expr '-' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 435: /* expr: expr '*' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 436: /* expr: expr "'**'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 437: /* expr: expr '/' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 438: /* expr: expr '%' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 439: /* expr: expr "'<<'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 440: /* expr: expr "'>>'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 441: /* expr: '+' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }
    break;

  case 442: /* expr: '-' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }
    break;

  case 443: /* expr: '!' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }
    break;

  case 444: /* expr: '~' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }
    break;

  case 445: /* expr: expr "'==='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 446: /* expr: expr "'!=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 447: /* expr: expr "'=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 448: /* expr: expr "'!='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 449: /* expr: expr '<' expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 450: /* expr: expr "'<='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 451: /* expr: expr '>' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 452: /* expr: expr "'>='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 453: /* expr: expr "'<=>'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 454: /* expr: expr "'instanceof'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 455: /* expr: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
		}
    break;

  case 456: /* expr: new_dereferenceable  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 457: /* expr: new_non_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 458: /* expr: expr '?' expr ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 459: /* expr: expr '?' ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 460: /* expr: expr "'??'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 461: /* expr: internal_functions_in_yacc  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 462: /* expr: "'(int)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }
    break;

  case 463: /* expr: "'(double)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }
    break;

  case 464: /* expr: "'(string)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }
    break;

  case 465: /* expr: "'(array)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }
    break;

  case 466: /* expr: "'(object)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }
    break;

  case 467: /* expr: "'(bool)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }
    break;

  case 468: /* expr: "'(unset)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }
    break;

  case 469: /* expr: "'exit'" ctor_arguments  */
                                      {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_EXIT));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, (yyvsp[0].ast));
		}
    break;

  case 470: /* expr: '@' expr  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }
    break;

  case 471: /* expr: scalar  */
                       { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 472: /* expr: '`' backticks_expr '`'  */
                                       { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }
    break;

  case 473: /* expr: "'print'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }
    break;

  case 474: /* expr: "'yield'"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 475: /* expr: "'yield'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 476: /* expr: "'yield'" expr "'=>'" expr  */
                                                 { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 477: /* expr: "'yield from'" expr  */
                                  { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 478: /* expr: "'throw'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }
    break;

  case 479: /* expr: inline_function  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 480: /* expr: attributes inline_function  */
                                           { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 481: /* expr: "'static'" inline_function  */
                                         { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 482: /* expr: attributes "'static'" inline_function  */
                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 483: /* expr: match  */
                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 484: /* inline_function: function returns_ref backup_doc_comment '(' parameter_list ')' lexical_vars return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  NULL,
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 485: /* inline_function: fn returns_ref backup_doc_comment '(' parameter_list ')' return_type "'=>'" backup_fn_flags backup_lex_pos expr backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  NULL, (yyvsp[-7].ast), NULL, (yyvsp[-1].ast), (yyvsp[-5].ast), NULL);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }
    break;

  case 486: /* fn: "'fn'"  */
             { (yyval.num) = CG(zend_lineno); }
    break;

  case 487: /* function: "'function'"  */
                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 488: /* backup_doc_comment: %empty  */
               { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 489: /* backup_fn_flags: %empty  */
                                         { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 490: /* backup_lex_pos: %empty  */
               { (yyval.ptr) = LANG_SCNG(yy_text); }
    break;

  case 491: /* returns_ref: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 492: /* returns_ref: ampersand  */
                                { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 493: /* lexical_vars: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 494: /* lexical_vars: "'use'" '(' lexical_var_list possible_comma ')'  */
                                                              { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 495: /* lexical_var_list: lexical_var_list ',' lexical_var  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 496: /* lexical_var_list: lexical_var  */
                            { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }
    break;

  case 497: /* lexical_var: "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 498: /* lexical_var: ampersand "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }
    break;

  case 499: /* function_call: name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 500: /* function_call: "'readonly'" argument_list  */
                                         {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-1].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, zend_ast_create_zval(&zv), (yyvsp[0].ast));
		}
    break;

  case 501: /* function_call: class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 502: /* function_call: variable_class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 503: /* @11: %empty  */
                              { (yyval.num) = CG(zend_lineno); }
    break;

  case 504: /* function_call: callable_expr @11 argument_list  */
                                                                           {
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-2].ast), (yyvsp[0].ast));
			(yyval.ast)->lineno = (yyvsp[-1].num);
		}
    break;

  case 505: /* class_name: "'static'"  */
                        { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 506: /* class_name: name  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 507: /* class_name_reference: class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 508: /* class_name_reference: new_variable  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 509: /* class_name_reference: '(' expr ')'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 510: /* backticks_expr: %empty  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 511: /* backticks_expr: "string content"  */
                                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 512: /* backticks_expr: encaps_list  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 513: /* ctor_arguments: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 514: /* ctor_arguments: argument_list  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 515: /* dereferenceable_scalar: "'array'" '(' array_pair_list ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 516: /* dereferenceable_scalar: '[' array_pair_list ']'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 517: /* dereferenceable_scalar: "quoted string"  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 518: /* dereferenceable_scalar: '"' encaps_list '"'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 519: /* scalar: "integer"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 520: /* scalar: "floating-point number"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 521: /* scalar: "heredoc start" "string content" "heredoc end"  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 522: /* scalar: "heredoc start" "heredoc end"  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 523: /* scalar: "heredoc start" encaps_list "heredoc end"  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 524: /* scalar: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 525: /* scalar: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 526: /* scalar: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 527: /* constant: name  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }
    break;

  case 528: /* constant: "'__LINE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 529: /* constant: "'__FILE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 530: /* constant: "'__DIR__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 531: /* constant: "'__TRAIT__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 532: /* constant: "'__METHOD__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 533: /* constant: "'__FUNCTION__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 534: /* constant: "'__PROPERTY__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_PROPERTY_C); }
    break;

  case 535: /* constant: "'__NAMESPACE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 536: /* constant: "'__CLASS__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 537: /* class_constant: class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 538: /* class_constant: variable_class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 539: /* class_constant: class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 540: /* class_constant: variable_class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 541: /* optional_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 542: /* optional_expr: expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 543: /* variable_class_name: fully_dereferenceable  */
                                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 544: /* fully_dereferenceable: variable  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 545: /* fully_dereferenceable: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_STATIC_PROP) (yyval.ast)->attr = ZEND_PARENTHESIZED_STATIC_PROP;
		}
    break;

  case 546: /* fully_dereferenceable: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 547: /* fully_dereferenceable: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 548: /* fully_dereferenceable: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 549: /* array_object_dereferenceable: fully_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 550: /* array_object_dereferenceable: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 551: /* callable_expr: callable_variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 552: /* callable_expr: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 553: /* callable_expr: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 554: /* callable_expr: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 555: /* callable_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 556: /* callable_variable: array_object_dereferenceable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 557: /* callable_variable: array_object_dereferenceable "'->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 558: /* callable_variable: array_object_dereferenceable "'?->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 559: /* callable_variable: function_call  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 560: /* variable: callable_variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 561: /* variable: static_member  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 562: /* variable: array_object_dereferenceable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 563: /* variable: array_object_dereferenceable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 564: /* simple_variable: "variable"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 565: /* simple_variable: '$' '{' expr '}'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 566: /* simple_variable: '$' simple_variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 567: /* static_member: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 568: /* static_member: variable_class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 569: /* new_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 570: /* new_variable: new_variable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 571: /* new_variable: new_variable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 572: /* new_variable: new_variable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 573: /* new_variable: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 574: /* new_variable: new_variable "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 575: /* member_name: identifier  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 576: /* member_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 577: /* member_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 578: /* property_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 579: /* property_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 580: /* property_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 581: /* array_pair_list: non_empty_array_pair_list  */
                        { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }
    break;

  case 582: /* possible_array_pair: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 583: /* possible_array_pair: array_pair  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 584: /* non_empty_array_pair_list: non_empty_array_pair_list ',' possible_array_pair  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 585: /* non_empty_array_pair_list: possible_array_pair  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }
    break;

  case 586: /* array_pair: expr "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }
    break;

  case 587: /* array_pair: expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 588: /* array_pair: expr "'=>'" ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }
    break;

  case 589: /* array_pair: ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }
    break;

  case 590: /* array_pair: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 591: /* array_pair: expr "'=>'" "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }
    break;

  case 592: /* array_pair: "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }
    break;

  case 593: /* encaps_list: encaps_list encaps_var  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 594: /* encaps_list: encaps_list "string content"  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 595: /* encaps_list: encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }
    break;

  case 596: /* encaps_list: "string content" encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 597: /* encaps_var: "variable"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 598: /* encaps_var: "variable" '[' encaps_var_offset ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }
    break;

  case 599: /* encaps_var: "variable" "'->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 600: /* encaps_var: "variable" "'?->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 601: /* encaps_var: "'${'" expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR, (yyvsp[-1].ast)); }
    break;

  case 602: /* encaps_var: "'${'" "variable name" '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY, (yyvsp[-1].ast)); }
    break;

  case 603: /* encaps_var: "'${'" "variable name" '[' expr ']' '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_ENCAPS_VAR_DOLLAR_CURLY,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }
    break;

  case 604: /* encaps_var: "'{$'" variable '}'  */
                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 605: /* encaps_var_offset: "identifier"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 606: /* encaps_var_offset: "number"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 607: /* encaps_var_offset: '-' "number"  */
                                        { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }
    break;

  case 608: /* encaps_var_offset: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 609: /* internal_functions_in_yacc: "'isset'" '(' isset_variables possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 610: /* internal_functions_in_yacc: "'empty'" '(' expr ')'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }
    break;

  case 611: /* internal_functions_in_yacc: "'include'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }
    break;

  case 612: /* internal_functions_in_yacc: "'include_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }
    break;

  case 613: /* internal_functions_in_yacc: "'eval'" '(' expr ')'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }
    break;

  case 614: /* internal_functions_in_yacc: "'require'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }
    break;

  case 615: /* internal_functions_in_yacc: "'require_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }
    break;

  case 616: /* isset_variables: isset_variable  */
                               { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 617: /* isset_variables: isset_variables ',' isset_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 618: /* isset_variable: expr  */
                     { (yyval.ast) = zend_ast_create(ZEND_AST_ISSET, (yyvsp[0].ast)); }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ZENDEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END)
        {
          /* Return failure if at end of input.  */
          if (yychar == END)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = ZENDEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ZENDEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}



/* Over-ride Bison formatting routine to give better token descriptions.
   Copy to YYRES the contents of YYSTR for use in yyerror.
   YYSTR is taken from yytname, from the %token declaration.
   If YYRES is null, do not copy; instead, return the length of what
   the result would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	const char *toktype = yystr;
	size_t toktype_len = strlen(toktype);

	/* CG(parse_error) states:
	 * 0 => yyres = NULL, yystr is the unexpected token
	 * 1 => yyres = NULL, yystr is one of the expected tokens
	 * 2 => yyres != NULL, yystr is the unexpected token
	 * 3 => yyres != NULL, yystr is one of the expected tokens
	 */
	if (yyres && CG(parse_error) < 2) {
		CG(parse_error) = 2;
	}

	if (CG(parse_error) % 2 == 0) {
		/* The unexpected token */
		char buffer[120];
		const unsigned char *tokcontent, *tokcontent_end;
		size_t tokcontent_len;

		CG(parse_error)++;

		if (LANG_SCNG(yy_text)[0] == 0 &&
			LANG_SCNG(yy_leng) == 1 &&
			strcmp(toktype, "\"end of file\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "end of file");
			}
			return sizeof("end of file")-1;
		}

		/* Prevent the backslash getting doubled in the output (eugh) */
		if (strcmp(toktype, "\"'\\\\'\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"\\\"");
			}
			return sizeof("token \"\\\"")-1;
		}

		/* We used "amp" as a dummy label to avoid a duplicate token literal warning. */
		if (strcmp(toktype, "\"amp\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"&\"");
			}
			return sizeof("token \"&\"")-1;
		}

		/* Avoid unreadable """ */
		/* "'" would theoretically be just as bad, but is never currently parsed as a separate token */
		if (strcmp(toktype, "'\"'") == 0) {
			if (yyres) {
				yystpcpy(yyres, "double-quote mark");
			}
			return sizeof("double-quote mark")-1;
		}

		/* Strip off the outer quote marks */
		if (toktype_len >= 2 && *toktype == '"') {
			toktype++;
			toktype_len -= 2;
		}

		/* If the token always has one form, the %token line should have a single-quoted name */
		/* The parser rules also include single-character un-named tokens which will be single-quoted here */
		/* We re-format this with double quotes here to ensure everything's consistent */
		if (toktype_len > 0 && *toktype == '\'') {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "token \"%.*s\"", (int)toktype_len-2, toktype+1);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + sizeof("token ")-1;
		}

		/* Fetch the content of the last seen token from global lexer state */
		tokcontent = LANG_SCNG(yy_text);
		tokcontent_len = LANG_SCNG(yy_leng);

		/* For T_BAD_CHARACTER, the content probably won't be a printable char */
		/* Also, "unexpected invalid character" sounds a bit redundant */
		if (tokcontent_len == 1 && strcmp(yystr, "\"invalid character\"") == 0) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "character 0x%02hhX", *tokcontent);
				yystpcpy(yyres, buffer);
			}
			return sizeof("character 0x00")-1;
		}

		/* Truncate at line end to avoid messing up log formats */
		tokcontent_end = memchr(tokcontent, '\n', tokcontent_len);
		if (tokcontent_end != NULL) {
			tokcontent_len = (tokcontent_end - tokcontent);
		}

		/* Try to be helpful about what kind of string was found, before stripping the quotes */
		if (tokcontent_len > 0 && strcmp(yystr, "\"quoted string\"") == 0) {
			if (*tokcontent == '"') {
				toktype = "double-quoted string";
				toktype_len = sizeof("double-quoted string")-1;
			}
			else if (*tokcontent == '\'') {
				toktype = "single-quoted string";
				toktype_len = sizeof("single-quoted string")-1;
			}
		}

		/* For quoted strings, strip off another layer of quotes to avoid putting quotes inside quotes */
		if (tokcontent_len > 0 && (*tokcontent == '\'' || *tokcontent=='"'))  {
			tokcontent++;
			tokcontent_len--;
		}
		if (tokcontent_len > 0 && (tokcontent[tokcontent_len-1] == '\'' || tokcontent[tokcontent_len-1] == '"'))  {
			tokcontent_len--;
		}

		/* Truncate to 30 characters and add a ... */
		if (tokcontent_len > 30 + sizeof("...")-1) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "%.*s \"%.*s...\"", (int)toktype_len, toktype, 30, tokcontent);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + 30 + sizeof(" \"...\"")-1;
		}

		if (yyres) {
			snprintf(buffer, sizeof(buffer), "%.*s \"%.*s\"", (int)toktype_len, toktype, (int)tokcontent_len, tokcontent);
			yystpcpy(yyres, buffer);
		}
		return toktype_len + tokcontent_len + sizeof(" \"\"")-1;
	}

	/* One of the expected tokens */

	/* Prevent the backslash getting doubled in the output (eugh) */
	if (strcmp(toktype, "\"'\\\\'\"") == 0) {
		if (yyres) {
			yystpcpy(yyres, "\"\\\"");
		}
		return sizeof("\"\\\"")-1;
	}

	/* Strip off the outer quote marks */
	if (toktype_len >= 2 && *toktype == '"') {
		toktype++;
		toktype_len -= 2;
	}

	if (yyres) {
		YYSIZE_T yyn = 0;

		for (; yyn < toktype_len; ++yyn) {
			/* Replace single quotes with double for consistency */
			if (toktype[yyn] == '\'') {
				yyres[yyn] = '"';
			}
			else {
				yyres[yyn] = toktype[yyn];
			}
		}
		yyres[toktype_len] = '\0';
	}

	return toktype_len;
}

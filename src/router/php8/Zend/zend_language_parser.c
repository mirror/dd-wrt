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
  YYSYMBOL_T_DOUBLE_CAST = 143,            /* "'(float)'"  */
  YYSYMBOL_T_STRING_CAST = 144,            /* "'(string)'"  */
  YYSYMBOL_T_ARRAY_CAST = 145,             /* "'(array)'"  */
  YYSYMBOL_T_OBJECT_CAST = 146,            /* "'(object)'"  */
  YYSYMBOL_T_BOOL_CAST = 147,              /* "'(bool)'"  */
  YYSYMBOL_T_UNSET_CAST = 148,             /* "'(unset)'"  */
  YYSYMBOL_T_VOID_CAST = 149,              /* "'(void)'"  */
  YYSYMBOL_T_OBJECT_OPERATOR = 150,        /* "'->'"  */
  YYSYMBOL_T_NULLSAFE_OBJECT_OPERATOR = 151, /* "'?->'"  */
  YYSYMBOL_T_DOUBLE_ARROW = 152,           /* "'=>'"  */
  YYSYMBOL_T_COMMENT = 153,                /* "comment"  */
  YYSYMBOL_T_DOC_COMMENT = 154,            /* "doc comment"  */
  YYSYMBOL_T_OPEN_TAG = 155,               /* "open tag"  */
  YYSYMBOL_T_OPEN_TAG_WITH_ECHO = 156,     /* "'<?='"  */
  YYSYMBOL_T_CLOSE_TAG = 157,              /* "'?>'"  */
  YYSYMBOL_T_WHITESPACE = 158,             /* "whitespace"  */
  YYSYMBOL_T_START_HEREDOC = 159,          /* "heredoc start"  */
  YYSYMBOL_T_END_HEREDOC = 160,            /* "heredoc end"  */
  YYSYMBOL_T_DOLLAR_OPEN_CURLY_BRACES = 161, /* "'${'"  */
  YYSYMBOL_T_CURLY_OPEN = 162,             /* "'{$'"  */
  YYSYMBOL_T_PAAMAYIM_NEKUDOTAYIM = 163,   /* "'::'"  */
  YYSYMBOL_T_NS_SEPARATOR = 164,           /* "'\\'"  */
  YYSYMBOL_T_ELLIPSIS = 165,               /* "'...'"  */
  YYSYMBOL_T_COALESCE = 166,               /* "'??'"  */
  YYSYMBOL_T_POW = 167,                    /* "'**'"  */
  YYSYMBOL_T_POW_EQUAL = 168,              /* "'**='"  */
  YYSYMBOL_T_PIPE = 169,                   /* "'|>'"  */
  YYSYMBOL_T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 170, /* "'&'"  */
  YYSYMBOL_T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 171, /* "amp"  */
  YYSYMBOL_T_BAD_CHARACTER = 172,          /* "invalid character"  */
  YYSYMBOL_T_ERROR = 173,                  /* T_ERROR  */
  YYSYMBOL_174_ = 174,                     /* ','  */
  YYSYMBOL_175_ = 175,                     /* ']'  */
  YYSYMBOL_176_ = 176,                     /* ';'  */
  YYSYMBOL_177_ = 177,                     /* '('  */
  YYSYMBOL_178_ = 178,                     /* ')'  */
  YYSYMBOL_179_ = 179,                     /* '{'  */
  YYSYMBOL_180_ = 180,                     /* '}'  */
  YYSYMBOL_181_ = 181,                     /* '['  */
  YYSYMBOL_182_ = 182,                     /* '`'  */
  YYSYMBOL_183_ = 183,                     /* '"'  */
  YYSYMBOL_184_ = 184,                     /* '$'  */
  YYSYMBOL_YYACCEPT = 185,                 /* $accept  */
  YYSYMBOL_start = 186,                    /* start  */
  YYSYMBOL_reserved_non_modifiers = 187,   /* reserved_non_modifiers  */
  YYSYMBOL_semi_reserved = 188,            /* semi_reserved  */
  YYSYMBOL_ampersand = 189,                /* ampersand  */
  YYSYMBOL_identifier = 190,               /* identifier  */
  YYSYMBOL_top_statement_list = 191,       /* top_statement_list  */
  YYSYMBOL_namespace_declaration_name = 192, /* namespace_declaration_name  */
  YYSYMBOL_namespace_name = 193,           /* namespace_name  */
  YYSYMBOL_legacy_namespace_name = 194,    /* legacy_namespace_name  */
  YYSYMBOL_name = 195,                     /* name  */
  YYSYMBOL_attribute_decl = 196,           /* attribute_decl  */
  YYSYMBOL_attribute_group = 197,          /* attribute_group  */
  YYSYMBOL_attribute = 198,                /* attribute  */
  YYSYMBOL_attributes = 199,               /* attributes  */
  YYSYMBOL_attributed_statement = 200,     /* attributed_statement  */
  YYSYMBOL_attributed_top_statement = 201, /* attributed_top_statement  */
  YYSYMBOL_top_statement = 202,            /* top_statement  */
  YYSYMBOL_203_1 = 203,                    /* $@1  */
  YYSYMBOL_204_2 = 204,                    /* $@2  */
  YYSYMBOL_use_type = 205,                 /* use_type  */
  YYSYMBOL_group_use_declaration = 206,    /* group_use_declaration  */
  YYSYMBOL_mixed_group_use_declaration = 207, /* mixed_group_use_declaration  */
  YYSYMBOL_possible_comma = 208,           /* possible_comma  */
  YYSYMBOL_inline_use_declarations = 209,  /* inline_use_declarations  */
  YYSYMBOL_unprefixed_use_declarations = 210, /* unprefixed_use_declarations  */
  YYSYMBOL_use_declarations = 211,         /* use_declarations  */
  YYSYMBOL_inline_use_declaration = 212,   /* inline_use_declaration  */
  YYSYMBOL_unprefixed_use_declaration = 213, /* unprefixed_use_declaration  */
  YYSYMBOL_use_declaration = 214,          /* use_declaration  */
  YYSYMBOL_const_list = 215,               /* const_list  */
  YYSYMBOL_inner_statement_list = 216,     /* inner_statement_list  */
  YYSYMBOL_inner_statement = 217,          /* inner_statement  */
  YYSYMBOL_statement = 218,                /* statement  */
  YYSYMBOL_219_3 = 219,                    /* $@3  */
  YYSYMBOL_catch_list = 220,               /* catch_list  */
  YYSYMBOL_catch_name_list = 221,          /* catch_name_list  */
  YYSYMBOL_optional_variable = 222,        /* optional_variable  */
  YYSYMBOL_finally_statement = 223,        /* finally_statement  */
  YYSYMBOL_unset_variables = 224,          /* unset_variables  */
  YYSYMBOL_unset_variable = 225,           /* unset_variable  */
  YYSYMBOL_function_name = 226,            /* function_name  */
  YYSYMBOL_function_declaration_statement = 227, /* function_declaration_statement  */
  YYSYMBOL_is_reference = 228,             /* is_reference  */
  YYSYMBOL_is_variadic = 229,              /* is_variadic  */
  YYSYMBOL_class_declaration_statement = 230, /* class_declaration_statement  */
  YYSYMBOL_231_4 = 231,                    /* @4  */
  YYSYMBOL_232_5 = 232,                    /* @5  */
  YYSYMBOL_class_modifiers = 233,          /* class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers = 234, /* anonymous_class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers_optional = 235, /* anonymous_class_modifiers_optional  */
  YYSYMBOL_class_modifier = 236,           /* class_modifier  */
  YYSYMBOL_trait_declaration_statement = 237, /* trait_declaration_statement  */
  YYSYMBOL_238_6 = 238,                    /* @6  */
  YYSYMBOL_interface_declaration_statement = 239, /* interface_declaration_statement  */
  YYSYMBOL_240_7 = 240,                    /* @7  */
  YYSYMBOL_enum_declaration_statement = 241, /* enum_declaration_statement  */
  YYSYMBOL_242_8 = 242,                    /* @8  */
  YYSYMBOL_enum_backing_type = 243,        /* enum_backing_type  */
  YYSYMBOL_enum_case = 244,                /* enum_case  */
  YYSYMBOL_enum_case_expr = 245,           /* enum_case_expr  */
  YYSYMBOL_extends_from = 246,             /* extends_from  */
  YYSYMBOL_interface_extends_list = 247,   /* interface_extends_list  */
  YYSYMBOL_implements_list = 248,          /* implements_list  */
  YYSYMBOL_foreach_variable = 249,         /* foreach_variable  */
  YYSYMBOL_for_statement = 250,            /* for_statement  */
  YYSYMBOL_foreach_statement = 251,        /* foreach_statement  */
  YYSYMBOL_declare_statement = 252,        /* declare_statement  */
  YYSYMBOL_switch_case_list = 253,         /* switch_case_list  */
  YYSYMBOL_case_list = 254,                /* case_list  */
  YYSYMBOL_match = 255,                    /* match  */
  YYSYMBOL_match_arm_list = 256,           /* match_arm_list  */
  YYSYMBOL_non_empty_match_arm_list = 257, /* non_empty_match_arm_list  */
  YYSYMBOL_match_arm = 258,                /* match_arm  */
  YYSYMBOL_match_arm_cond_list = 259,      /* match_arm_cond_list  */
  YYSYMBOL_while_statement = 260,          /* while_statement  */
  YYSYMBOL_if_stmt_without_else = 261,     /* if_stmt_without_else  */
  YYSYMBOL_if_stmt = 262,                  /* if_stmt  */
  YYSYMBOL_alt_if_stmt_without_else = 263, /* alt_if_stmt_without_else  */
  YYSYMBOL_alt_if_stmt = 264,              /* alt_if_stmt  */
  YYSYMBOL_parameter_list = 265,           /* parameter_list  */
  YYSYMBOL_non_empty_parameter_list = 266, /* non_empty_parameter_list  */
  YYSYMBOL_attributed_parameter = 267,     /* attributed_parameter  */
  YYSYMBOL_optional_cpp_modifiers = 268,   /* optional_cpp_modifiers  */
  YYSYMBOL_parameter = 269,                /* parameter  */
  YYSYMBOL_optional_type_without_static = 270, /* optional_type_without_static  */
  YYSYMBOL_type_expr = 271,                /* type_expr  */
  YYSYMBOL_type = 272,                     /* type  */
  YYSYMBOL_union_type_element = 273,       /* union_type_element  */
  YYSYMBOL_union_type = 274,               /* union_type  */
  YYSYMBOL_intersection_type = 275,        /* intersection_type  */
  YYSYMBOL_type_expr_without_static = 276, /* type_expr_without_static  */
  YYSYMBOL_type_without_static = 277,      /* type_without_static  */
  YYSYMBOL_union_type_without_static_element = 278, /* union_type_without_static_element  */
  YYSYMBOL_union_type_without_static = 279, /* union_type_without_static  */
  YYSYMBOL_intersection_type_without_static = 280, /* intersection_type_without_static  */
  YYSYMBOL_return_type = 281,              /* return_type  */
  YYSYMBOL_argument_list = 282,            /* argument_list  */
  YYSYMBOL_non_empty_argument_list = 283,  /* non_empty_argument_list  */
  YYSYMBOL_clone_argument_list = 284,      /* clone_argument_list  */
  YYSYMBOL_non_empty_clone_argument_list = 285, /* non_empty_clone_argument_list  */
  YYSYMBOL_argument_no_expr = 286,         /* argument_no_expr  */
  YYSYMBOL_argument = 287,                 /* argument  */
  YYSYMBOL_global_var_list = 288,          /* global_var_list  */
  YYSYMBOL_global_var = 289,               /* global_var  */
  YYSYMBOL_static_var_list = 290,          /* static_var_list  */
  YYSYMBOL_static_var = 291,               /* static_var  */
  YYSYMBOL_class_statement_list = 292,     /* class_statement_list  */
  YYSYMBOL_attributed_class_statement = 293, /* attributed_class_statement  */
  YYSYMBOL_class_statement = 294,          /* class_statement  */
  YYSYMBOL_class_name_list = 295,          /* class_name_list  */
  YYSYMBOL_trait_adaptations = 296,        /* trait_adaptations  */
  YYSYMBOL_trait_adaptation_list = 297,    /* trait_adaptation_list  */
  YYSYMBOL_trait_adaptation = 298,         /* trait_adaptation  */
  YYSYMBOL_trait_precedence = 299,         /* trait_precedence  */
  YYSYMBOL_trait_alias = 300,              /* trait_alias  */
  YYSYMBOL_trait_method_reference = 301,   /* trait_method_reference  */
  YYSYMBOL_absolute_trait_method_reference = 302, /* absolute_trait_method_reference  */
  YYSYMBOL_method_body = 303,              /* method_body  */
  YYSYMBOL_property_modifiers = 304,       /* property_modifiers  */
  YYSYMBOL_method_modifiers = 305,         /* method_modifiers  */
  YYSYMBOL_class_const_modifiers = 306,    /* class_const_modifiers  */
  YYSYMBOL_non_empty_member_modifiers = 307, /* non_empty_member_modifiers  */
  YYSYMBOL_member_modifier = 308,          /* member_modifier  */
  YYSYMBOL_property_list = 309,            /* property_list  */
  YYSYMBOL_property = 310,                 /* property  */
  YYSYMBOL_hooked_property = 311,          /* hooked_property  */
  YYSYMBOL_property_hook_list = 312,       /* property_hook_list  */
  YYSYMBOL_optional_property_hook_list = 313, /* optional_property_hook_list  */
  YYSYMBOL_property_hook_modifiers = 314,  /* property_hook_modifiers  */
  YYSYMBOL_property_hook = 315,            /* property_hook  */
  YYSYMBOL_316_9 = 316,                    /* @9  */
  YYSYMBOL_property_hook_body = 317,       /* property_hook_body  */
  YYSYMBOL_optional_parameter_list = 318,  /* optional_parameter_list  */
  YYSYMBOL_class_const_list = 319,         /* class_const_list  */
  YYSYMBOL_class_const_decl = 320,         /* class_const_decl  */
  YYSYMBOL_const_decl = 321,               /* const_decl  */
  YYSYMBOL_echo_expr_list = 322,           /* echo_expr_list  */
  YYSYMBOL_echo_expr = 323,                /* echo_expr  */
  YYSYMBOL_for_cond_exprs = 324,           /* for_cond_exprs  */
  YYSYMBOL_for_exprs = 325,                /* for_exprs  */
  YYSYMBOL_non_empty_for_exprs = 326,      /* non_empty_for_exprs  */
  YYSYMBOL_anonymous_class = 327,          /* anonymous_class  */
  YYSYMBOL_328_10 = 328,                   /* @10  */
  YYSYMBOL_new_dereferenceable = 329,      /* new_dereferenceable  */
  YYSYMBOL_new_non_dereferenceable = 330,  /* new_non_dereferenceable  */
  YYSYMBOL_expr = 331,                     /* expr  */
  YYSYMBOL_inline_function = 332,          /* inline_function  */
  YYSYMBOL_fn = 333,                       /* fn  */
  YYSYMBOL_function = 334,                 /* function  */
  YYSYMBOL_backup_doc_comment = 335,       /* backup_doc_comment  */
  YYSYMBOL_backup_fn_flags = 336,          /* backup_fn_flags  */
  YYSYMBOL_backup_lex_pos = 337,           /* backup_lex_pos  */
  YYSYMBOL_returns_ref = 338,              /* returns_ref  */
  YYSYMBOL_lexical_vars = 339,             /* lexical_vars  */
  YYSYMBOL_lexical_var_list = 340,         /* lexical_var_list  */
  YYSYMBOL_lexical_var = 341,              /* lexical_var  */
  YYSYMBOL_function_call = 342,            /* function_call  */
  YYSYMBOL_343_11 = 343,                   /* @11  */
  YYSYMBOL_class_name = 344,               /* class_name  */
  YYSYMBOL_class_name_reference = 345,     /* class_name_reference  */
  YYSYMBOL_backticks_expr = 346,           /* backticks_expr  */
  YYSYMBOL_ctor_arguments = 347,           /* ctor_arguments  */
  YYSYMBOL_dereferenceable_scalar = 348,   /* dereferenceable_scalar  */
  YYSYMBOL_scalar = 349,                   /* scalar  */
  YYSYMBOL_constant = 350,                 /* constant  */
  YYSYMBOL_class_constant = 351,           /* class_constant  */
  YYSYMBOL_optional_expr = 352,            /* optional_expr  */
  YYSYMBOL_variable_class_name = 353,      /* variable_class_name  */
  YYSYMBOL_fully_dereferenceable = 354,    /* fully_dereferenceable  */
  YYSYMBOL_array_object_dereferenceable = 355, /* array_object_dereferenceable  */
  YYSYMBOL_callable_expr = 356,            /* callable_expr  */
  YYSYMBOL_callable_variable = 357,        /* callable_variable  */
  YYSYMBOL_variable = 358,                 /* variable  */
  YYSYMBOL_simple_variable = 359,          /* simple_variable  */
  YYSYMBOL_static_member = 360,            /* static_member  */
  YYSYMBOL_new_variable = 361,             /* new_variable  */
  YYSYMBOL_member_name = 362,              /* member_name  */
  YYSYMBOL_property_name = 363,            /* property_name  */
  YYSYMBOL_array_pair_list = 364,          /* array_pair_list  */
  YYSYMBOL_possible_array_pair = 365,      /* possible_array_pair  */
  YYSYMBOL_non_empty_array_pair_list = 366, /* non_empty_array_pair_list  */
  YYSYMBOL_array_pair = 367,               /* array_pair  */
  YYSYMBOL_encaps_list = 368,              /* encaps_list  */
  YYSYMBOL_encaps_var = 369,               /* encaps_var  */
  YYSYMBOL_encaps_var_offset = 370,        /* encaps_var_offset  */
  YYSYMBOL_internal_functions_in_yacc = 371, /* internal_functions_in_yacc  */
  YYSYMBOL_isset_variables = 372,          /* isset_variables  */
  YYSYMBOL_isset_variable = 373            /* isset_variable  */
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
#define YYLAST   11223

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  185
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  189
/* YYNRULES -- Number of rules.  */
#define YYNRULES  635
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1204

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   412


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
       2,     2,     2,    17,   183,     2,   184,    16,     2,     2,
     177,   178,    14,    12,   174,    13,    11,    15,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,   176,
       9,     4,    10,     5,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   181,     2,   175,     8,     2,   182,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   179,     7,   180,    18,     2,     2,     2,
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
     171,   172,   173
};

#if ZENDDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   304,   304,   308,   308,   308,   308,   308,   308,   308,
     308,   309,   309,   309,   309,   309,   309,   309,   309,   309,
     309,   309,   309,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   311,   311,   311,   311,   311,   311,   311,
     311,   311,   311,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   313,   313,   313,   313,   313,   313,
     313,   313,   314,   314,   314,   314,   314,   314,   314,   314,
     314,   314,   314,   315,   319,   320,   320,   320,   320,   320,
     320,   320,   324,   325,   329,   330,   338,   339,   344,   345,
     350,   351,   356,   357,   361,   362,   363,   364,   368,   370,
     375,   377,   382,   386,   387,   391,   392,   393,   394,   395,
     399,   400,   404,   405,   406,   407,   411,   414,   414,   417,
     417,   420,   421,   422,   423,   427,   428,   432,   437,   442,
     443,   447,   449,   454,   456,   461,   463,   468,   469,   473,
     475,   480,   482,   487,   488,   492,   494,   500,   501,   502,
     503,   510,   511,   512,   513,   515,   517,   519,   521,   522,
     523,   524,   525,   526,   527,   528,   529,   530,   532,   536,
     535,   539,   540,   542,   543,   544,   548,   550,   555,   556,
     560,   561,   565,   566,   570,   571,   575,   579,   580,   588,
     595,   596,   600,   601,   605,   605,   608,   608,   614,   615,
     620,   622,   627,   628,   632,   633,   634,   638,   638,   644,
     644,   650,   650,   656,   657,   661,   666,   667,   671,   672,
     676,   677,   681,   682,   686,   687,   688,   689,   693,   694,
     698,   699,   703,   704,   708,   709,   710,   711,   715,   716,
     718,   720,   722,   728,   733,   734,   738,   739,   743,   745,
     750,   751,   756,   757,   762,   765,   771,   772,   777,   780,
     786,   787,   793,   794,   799,   801,   806,   807,   811,   813,
     819,   823,   830,   831,   835,   836,   837,   838,   842,   843,
     847,   848,   852,   854,   859,   860,   867,   868,   869,   870,
     874,   875,   876,   880,   881,   885,   887,   892,   894,   899,
     900,   904,   905,   906,   910,   912,   926,   927,   928,   929,
     933,   935,   937,   942,   944,   948,   949,   953,   954,   958,
     964,   965,   969,   970,   974,   976,   982,   985,   988,   991,
     994,   998,  1002,  1003,  1004,  1009,  1010,  1014,  1015,  1016,
    1020,  1022,  1027,  1028,  1032,  1037,  1039,  1043,  1048,  1056,
    1058,  1062,  1067,  1068,  1072,  1075,  1080,  1082,  1089,  1091,
    1098,  1100,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1118,  1119,  1123,  1125,  1130,  1132,  1137,  1138,
    1139,  1145,  1146,  1150,  1151,  1159,  1158,  1169,  1170,  1171,
    1176,  1177,  1181,  1182,  1186,  1187,  1195,  1199,  1200,  1203,
    1207,  1208,  1209,  1213,  1214,  1218,  1219,  1220,  1221,  1225,
    1225,  1235,  1237,  1239,  1244,  1249,  1251,  1253,  1255,  1257,
    1259,  1264,  1269,  1271,  1273,  1275,  1277,  1279,  1281,  1283,
    1285,  1287,  1289,  1291,  1293,  1295,  1296,  1297,  1298,  1299,
    1301,  1303,  1305,  1307,  1309,  1310,  1311,  1312,  1313,  1314,
    1315,  1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,
    1325,  1326,  1328,  1330,  1332,  1334,  1336,  1338,  1340,  1342,
    1344,  1346,  1348,  1353,  1354,  1355,  1357,  1359,  1361,  1362,
    1363,  1364,  1365,  1366,  1367,  1368,  1369,  1374,  1375,  1376,
    1377,  1378,  1379,  1380,  1381,  1382,  1383,  1384,  1385,  1386,
    1388,  1393,  1398,  1406,  1410,  1414,  1418,  1422,  1426,  1427,
    1431,  1432,  1436,  1437,  1441,  1442,  1446,  1448,  1453,  1455,
    1457,  1457,  1464,  1467,  1471,  1472,  1473,  1477,  1479,  1480,
    1485,  1486,  1491,  1492,  1493,  1494,  1498,  1499,  1500,  1501,
    1503,  1504,  1505,  1506,  1510,  1511,  1512,  1513,  1514,  1515,
    1516,  1517,  1518,  1519,  1523,  1525,  1527,  1529,  1534,  1535,
    1539,  1543,  1544,  1548,  1549,  1550,  1554,  1555,  1559,  1560,
    1561,  1562,  1566,  1568,  1570,  1572,  1574,  1578,  1580,  1582,
    1584,  1589,  1590,  1591,  1595,  1597,  1602,  1604,  1606,  1608,
    1610,  1612,  1617,  1618,  1619,  1623,  1624,  1625,  1629,  1634,
    1635,  1639,  1641,  1646,  1648,  1650,  1652,  1654,  1656,  1659,
    1665,  1667,  1669,  1671,  1676,  1678,  1681,  1684,  1687,  1689,
    1691,  1694,  1698,  1699,  1700,  1701,  1706,  1707,  1708,  1710,
    1712,  1714,  1716,  1721,  1722,  1727
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
  "\"'(int)'\"", "\"'(float)'\"", "\"'(string)'\"", "\"'(array)'\"",
  "\"'(object)'\"", "\"'(bool)'\"", "\"'(unset)'\"", "\"'(void)'\"",
  "\"'->'\"", "\"'?->'\"", "\"'=>'\"", "\"comment\"", "\"doc comment\"",
  "\"open tag\"", "\"'<?='\"", "\"'?>'\"", "\"whitespace\"",
  "\"heredoc start\"", "\"heredoc end\"", "\"'${'\"", "\"'{$'\"",
  "\"'::'\"", "\"'\\\\'\"", "\"'...'\"", "\"'?""?'\"", "\"'**'\"",
  "\"'**='\"", "\"'|>'\"", "\"'&'\"", "\"amp\"", "\"invalid character\"",
  "T_ERROR", "','", "']'", "';'", "'('", "')'", "'{'", "'}'", "'['", "'`'",
  "'\"'", "'$'", "$accept", "start", "reserved_non_modifiers",
  "semi_reserved", "ampersand", "identifier", "top_statement_list",
  "namespace_declaration_name", "namespace_name", "legacy_namespace_name",
  "name", "attribute_decl", "attribute_group", "attribute", "attributes",
  "attributed_statement", "attributed_top_statement", "top_statement",
  "$@1", "$@2", "use_type", "group_use_declaration",
  "mixed_group_use_declaration", "possible_comma",
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
  "case_list", "match", "match_arm_list", "non_empty_match_arm_list",
  "match_arm", "match_arm_cond_list", "while_statement",
  "if_stmt_without_else", "if_stmt", "alt_if_stmt_without_else",
  "alt_if_stmt", "parameter_list", "non_empty_parameter_list",
  "attributed_parameter", "optional_cpp_modifiers", "parameter",
  "optional_type_without_static", "type_expr", "type",
  "union_type_element", "union_type", "intersection_type",
  "type_expr_without_static", "type_without_static",
  "union_type_without_static_element", "union_type_without_static",
  "intersection_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "clone_argument_list",
  "non_empty_clone_argument_list", "argument_no_expr", "argument",
  "global_var_list", "global_var", "static_var_list", "static_var",
  "class_statement_list", "attributed_class_statement", "class_statement",
  "class_name_list", "trait_adaptations", "trait_adaptation_list",
  "trait_adaptation", "trait_precedence", "trait_alias",
  "trait_method_reference", "absolute_trait_method_reference",
  "method_body", "property_modifiers", "method_modifiers",
  "class_const_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "hooked_property", "property_hook_list",
  "optional_property_hook_list", "property_hook_modifiers",
  "property_hook", "@9", "property_hook_body", "optional_parameter_list",
  "class_const_list", "class_const_decl", "const_decl", "echo_expr_list",
  "echo_expr", "for_cond_exprs", "for_exprs", "non_empty_for_exprs",
  "anonymous_class", "@10", "new_dereferenceable",
  "new_non_dereferenceable", "expr", "inline_function", "fn", "function",
  "backup_doc_comment", "backup_fn_flags", "backup_lex_pos", "returns_ref",
  "lexical_vars", "lexical_var_list", "lexical_var", "function_call",
  "@11", "class_name", "class_name_reference", "backticks_expr",
  "ctor_arguments", "dereferenceable_scalar", "scalar", "constant",
  "class_constant", "optional_expr", "variable_class_name",
  "fully_dereferenceable", "array_object_dereferenceable", "callable_expr",
  "callable_variable", "variable", "simple_variable", "static_member",
  "new_variable", "member_name", "property_name", "array_pair_list",
  "possible_array_pair", "non_empty_array_pair_list", "array_pair",
  "encaps_list", "encaps_var", "encaps_var_offset",
  "internal_functions_in_yacc", "isset_variables", "isset_variable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-1077)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-607)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1077,    56,  4479, -1077,  8897,  8897,  8897,  8897,  8897, -1077,
   -1077,    80, -1077, -1077, -1077, -1077, -1077, -1077,  8897,  8897,
     -55,  8897,  8897,  8897,  8897,  8897,  1074,  9070,   -43,    -9,
    8897,  7167,     0,     5,    48,    53,    58,    88,  8897,  8897,
     253, -1077, -1077,   272,  8897,   123,  8897,   524,    37,   443,
   -1077, -1077,   -43,   139,   148,   161,   168, -1077, -1077, -1077,
   -1077, 10922,   171,   260, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077,   329, 10342, 10342,  8897,  8897,  8897,  8897,
    8897,  8897,  8897,  8897,   101, -1077,  8897, -1077,  7340,    77,
     124,     1,  -115, -1077,  2146, -1077, -1077, -1077, -1077, -1077,
   -1077,   275, -1077, -1077, -1077, -1077, -1077,   405, -1077,   550,
   -1077,   616, -1077,  9230, -1077,   368,   368, -1077,   142,   656,
   -1077,   -67,   338,   265,   284,   317, -1077,   296,   129, -1077,
   -1077, -1077, -1077,   514,   -43,   370,   337,   368,   337,    -1,
     337,   337, -1077,  9730,  9730,  8897,  9730,  9730,  9932,  9877,
    9932, -1077, -1077,  8897, -1077,   507,   554,   428, -1077, -1077,
     397,   -43, -1077,   421,  3441, -1077, -1077,  3614, -1077, -1077,
    8897,   -30, -1077,  9730,   516,  8897,  7859,  8897,   272,  8897,
    8897,  9730,   321,   442,   446,   577,   238, -1077,   482, -1077,
    9730, -1077, -1077, -1077, -1077, -1077, -1077,    69,   675,   495,
     303, -1077,   316, -1077, -1077,   630,   392, -1077, -1077, -1077,
   10342,  8897,  8897,   505,   664,   677,   679,   681, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077,   532,   538,  7340,  7340, -1077,   546,   -43,  1074,  8897,
    7340,   545,   555, -1077, -1077,   470,   470,   337,   337,   337,
     337,   337,   337,   337,  9250,   360,   147, -1077,  8032, 10342,
     138, -1077,  2780,  4652,   564,  8897, -1077, -1077, 10342,  9628,
     567, -1077,   569, -1077,    83,   562,   334,    83,    43,  8897,
   -1077, -1077,   514, -1077, -1077, -1077, -1077, -1077,   570,  7167,
     571,   740,   574,  2144,  8897,  8897,  8897,  8897,  8897,  8897,
    8897,  8897,  8897,  8897,  8897,  8897,  8897,   458,  8897,  8897,
    8897,  8897,  8897,  8897,  8897,  8897,  8897,  8897,  8897,  8897,
    8897,  8897,  8897,  8897, -1077, -1077, -1077,    50, 10084, 10237,
      52,    52,  8897,   -43,  7513,  8897,  8897,  8897,  8897,  8897,
    8897,  8897,  8897,  8897,  8897,  8897,  8897, -1077, -1077,  8897,
   -1077,  2793,  8897,  2848, -1077, -1077, -1077,    37, -1077,    52,
      52,    37,  8897,   210,  8897,  8897,   -55,  8897,  8897,  8897,
    2317,   704,  9070,    16,    88,   745,   747,  8897,    10,   -43,
     148,   161,   171,   260,   748,   763,   764,   766,   767,   768,
     769,   771,   772,  8205, -1077,   774,   607, -1077,  2711,  8378,
   -1077,   625, -1077, -1077,  9730,  2928,  8897, -1077,   623,  2983,
    8897,   627,   631,  9730,  9671,   -69,  2996,  3063, -1077, -1077,
   -1077,  8897,   272, -1077, -1077,  4825,   787,   634,    70,   646,
     438, -1077,   675, -1077,    37, -1077,  8897,   796, -1077,   650,
   -1077,   190,  9730,   651, -1077,  3131,   652,   728, -1077,   730,
     826, -1077,   655, -1077,   657,   662,   329,   666, -1077,   -43,
    3198,   668, -1077,   824,   827,   640, -1077, -1077,   425,  2429,
     669, -1077, -1077, -1077,   779,   680, -1077,  2294, -1077, -1077,
   -1077,  7340,  9730,   155,  7686,   842,  7340, -1077, -1077,  2497,
   -1077,   831,  8897, -1077,  8897, -1077, -1077,  8897,  9683,  3774,
     887,  1607,  1607,   145,   104,   104,    -1,    -1,    -1,  9865,
    9920,  9932, -1077,  1435,  3601,  3947,  3947,  3947,  3947,  1607,
    1607,  3947,   530,   530,  9943,   337,   394,  4120,  4120,   682,
   -1077, -1077, -1077,   683,  8897,   684,   685,   -43,  8897,   684,
     685,   -43, -1077,  8897, -1077,   -43,   -43,   692, -1077, 10342,
    9932,  9932,  9932,  9932,  9932,  9932,  9932,  9932,  9932,  9932,
    9932,  9932,  9932,  9932, -1077,  9932, -1077,   -43, -1077, -1077,
   -1077, -1077,   694, -1077,  9730,  8897,  3960,   697,  3787, -1077,
    3960,   698,  1257, -1077,  8897,  1448,  9730,  7859,  8551, 10331,
   -1077,    15,   691,  9730, -1077, -1077, -1077,   251,   699, -1077,
   -1077,   817, -1077, -1077,  9730, -1077, 10342,   707,  8897,   708,
   -1077, -1077,   329,   756,   703,   329, -1077,   498,   756, -1077,
    4133,   883, -1077, -1077, -1077,   712, -1077, -1077, -1077,   858,
   -1077, -1077, -1077,   717, -1077,  8897, -1077, -1077,   715, -1077,
     726,   739, 10342,  9730,  8897, -1077, -1077,   728,  3211,  3266,
    4998,  9943,  8897,   673,   743,   673,  2509, -1077,  2564, -1077,
    2576, -1077, -1077, -1077,   470,   728, -1077,  9730,  8897, -1077,
   -1077, -1077, -1077, -1077, -1077, -1077, -1077,  3279, -1077, -1077,
   -1077,   741,   749,  9479,  8897,  9730,   744,  7340, 10342,   -39,
      90,  1622,   752,   759, -1077,  8724, -1077,   549,   863,   169,
     762, -1077, -1077,   169, -1077,   761, -1077, -1077, -1077,   329,
   -1077, -1077,   770, -1077,   773,   511, -1077, -1077, -1077,   511,
   -1077, -1077,    30,   931,   933,   777, -1077, -1077,  4306, -1077,
    8897, -1077, -1077,  9432,   775,   883,  7340,   257,  9932,   756,
    7167,   944,   778,  9943, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077,  1831,   780,   783, -1077,   112, -1077,
    1841, -1077,   673,   781,   784,   784, -1077,   756,  6901,   786,
    5171,  7859,  8551,  9730,  7340,   788,   135, 10331,  1796, -1077,
   -1077, -1077, -1077,   641, -1077,    61,   792,   789,   793, -1077,
     794,  9730,   795,   797, -1077,   950, -1077,   251,   798,   800,
   -1077, -1077,   770,   802,  1126,   329, -1077, -1077,   806,    42,
     511,   840,   840,   511,   803, -1077,  9932,   799, -1077,   805,
   -1077, -1077, -1077, -1077, -1077,   978,  1299, -1077,   605,   605,
     816, -1077,    44,   980,   981,   818, -1077,   814,   915, -1077,
   -1077,   819,   820,  9491,   822,   149,   829, -1077, -1077, -1077,
    5344,   649,   832,  8897,    20,   137, -1077, -1077,   860, -1077,
    8724, -1077,  8897,   876,   329, -1077, -1077, -1077, -1077,   169,
     849, -1077, -1077,   329, -1077, -1077,  2111, -1077, -1077, -1077,
     112,   959,   958,  1702, -1077,  9213, -1077, -1077, -1077, -1077,
   -1077, -1077, -1077, -1077,   883,   854,  6901,   498,   882, -1077,
   -1077,   866,   110, -1077,   873,   605,   483,   483,   605,   978,
     862,   978,   861, -1077,  1970, -1077,  1796,  5517,   867,   868,
   -1077,  9182, -1077, -1077, -1077,  8897, -1077,  9730,  8897,    96,
   -1077,  5690, -1077, -1077, 10145, 11015,   506, -1077,  1018,   368,
    7063, -1077, 10309, -1077, -1077, -1077, -1077, -1077,  1019, -1077,
   -1077, -1077, -1077, -1077, -1077,    40, -1077, -1077, -1077, -1077,
   -1077, -1077,   871, -1077, -1077, -1077, -1077,  6901,  6901,  9730,
    9730,   329, -1077,   870, -1077, -1077,  1045, -1077, 10504, -1077,
    1046,   467, -1077, -1077, 11015,  1048,  1049,  1051,  1055,  1056,
   11108,   474, -1077, -1077, 10557, -1077, -1077,   884, -1077,  1038,
     903, -1077,   904, 10692,  5863, -1077,  6901,  6901, -1077,   908,
    8897,   906,   927, -1077, -1077, 10641, -1077,   916,   917,  1032,
    1015,   939,  8897,   925,  1069, -1077, -1077,  8897,  8897,  1048,
     490, 11108, -1077, -1077,  8897,    21, -1077, -1077,    40,   928,
   -1077, -1077,   929, -1077,  9730, -1077, -1077, -1077, -1077, -1077,
   10829,   329, 11015,  9730, -1077,  1103, -1077,   934,  9730,  9730,
   -1077, -1077,  9730,  8897, -1077, -1077,  6036, -1077, -1077,  6209,
   -1077,  6382, -1077, -1077, 11015,   770, -1077,   935,   823,  8897,
   -1077,   673, -1077, -1077, -1077,  2644,  1493, -1077, -1077, -1077,
   -1077, -1077, -1077,  1831,  1841,   368, -1077,  9730,   938, -1077,
   -1077, -1077, -1077,  1666, -1077,  1089, -1077,   978, -1077, -1077,
   -1077, -1077,   117,   936, -1077, -1077, -1077,   673, -1077,  6555,
   -1077,   940,   400, -1077, -1077,  8897, -1077, -1077, -1077,  9420,
    6728, -1077, -1077, -1077
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      87,     0,     2,     1,     0,     0,     0,     0,     0,   536,
     537,    94,    96,    97,    95,   581,   164,   534,     0,     0,
       0,     0,     0,     0,   491,     0,   202,     0,   530,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   558,   558,
       0,   504,   503,     0,   558,     0,     0,     0,     0,   522,
     204,   205,   206,     0,     0,     0,     0,   196,   207,   209,
     211,   119,     0,     0,   545,   546,   547,   553,   548,   549,
     550,   551,   552,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,   146,   599,   527,
       0,     0,   544,   103,     0,   110,   113,    86,   112,   105,
     106,     0,   198,   107,   108,   109,   500,   256,   152,     0,
     153,   473,   474,     0,   496,   508,   508,   576,     0,   541,
     488,   542,   543,     0,   566,     0,   520,   577,   415,   572,
     578,   478,    94,   522,     0,     0,   457,   508,   458,   459,
     460,   487,   174,   628,   629,     0,   631,   632,   490,   492,
     494,   522,   206,     0,   523,   202,   203,     0,   200,   412,
     524,   414,   586,   525,     0,   420,   421,     0,   531,   486,
       0,     0,   398,   399,     0,     0,   403,     0,     0,     0,
       0,   559,     0,     0,     0,     0,     0,   144,     0,   146,
     495,    90,    93,    91,   125,   126,    92,   141,     0,     0,
       0,   136,     0,   318,   319,   322,     0,   321,   498,   517,
       0,     0,     0,     0,     0,     0,     0,     0,    84,    89,
       3,     4,     5,     6,     7,     8,     9,    10,    46,    47,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    49,
      50,    51,    52,    71,    53,    41,    42,    43,    70,    44,
      45,    30,    31,    32,    33,    34,    35,    36,    75,    76,
      77,    78,    79,    80,    81,    37,    38,    39,    40,    61,
      59,    60,    72,    56,    57,    58,    48,    54,    55,    66,
      67,    68,    62,    63,    65,    64,    73,    69,    74,    85,
      88,   117,     0,   599,   599,   100,   129,    98,   202,     0,
     599,   565,   563,   567,   564,   436,   438,   479,   480,   481,
     482,   483,   484,   485,     0,   614,     0,   539,     0,     0,
       0,   612,     0,     0,     0,     0,    82,    83,     0,   604,
       0,   602,   598,   600,   528,     0,   529,     0,     0,     0,
     583,   516,     0,   104,   114,   497,   194,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,   509,   505,   505,     0,     0,
       0,     0,   558,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   435,   437,     0,
     505,     0,     0,     0,   413,   201,   409,     0,   411,     0,
       0,     0,   558,    94,     3,     4,     5,     6,     7,    46,
     491,    12,    13,   530,    71,   504,   503,    33,    75,    81,
      39,    40,    48,    54,   545,   546,   547,   553,   548,   549,
     550,   551,   552,     0,   306,     0,   129,   311,     0,     0,
     301,   129,   316,   304,   315,     0,     0,   163,     0,     0,
       0,     0,   404,   408,     0,     0,     0,     0,   158,   159,
     173,     0,     0,   111,   160,     0,     0,     0,   141,     0,
       0,   121,     0,   123,     0,   161,     0,     0,   162,   129,
     184,   561,   635,   129,   633,     0,     0,   218,   505,   220,
     213,   116,     0,    87,     0,     0,   130,     0,    99,     0,
       0,     0,   175,     0,     0,     0,   538,   613,     0,     0,
     561,   611,   540,   610,   472,     0,   151,     0,   148,   145,
     147,   599,   607,   561,     0,   533,   599,   489,   535,     0,
     499,     0,     0,   257,     0,   146,   260,     0,     0,   444,
     447,   466,   468,   448,   449,   450,   451,   453,   454,   441,
     443,   442,   471,   439,   440,   463,   464,   461,   462,   467,
     469,   470,   455,   456,   477,   452,   465,   446,   445,     0,
     187,   188,   505,     0,     0,   554,   584,     0,     0,   555,
     585,     0,   595,     0,   597,   579,   580,     0,   521,     0,
     418,   422,   423,   424,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   425,   630,   493,   526,   530,   590,   588,
     589,   591,     0,   309,   314,     0,   130,     0,     0,   303,
     130,     0,     0,   397,     0,     0,   407,   400,     0,     0,
     169,     0,     0,   505,   143,   176,   142,     0,     0,   122,
     124,   141,   135,   317,   323,   320,   130,     0,   130,     0,
     627,   115,     0,   222,     0,     0,   505,     0,   222,    87,
       0,     0,   532,   101,   102,   562,   533,   616,   617,     0,
     622,   625,   623,     0,   619,     0,   618,   621,     0,   149,
       0,     0,     0,   603,     0,   601,   582,   218,     0,     0,
       0,   476,     0,   268,     0,   268,     0,   518,     0,   519,
       0,   574,   575,   573,   419,   218,   587,   313,     0,   312,
     307,   308,   310,   305,   302,   146,   254,     0,   146,   252,
     154,     0,     0,   402,     0,   405,     0,   599,     0,     0,
     561,     0,   238,   238,   157,   244,   396,   182,   139,     0,
     129,   132,   137,     0,   185,     0,   634,   626,   219,     0,
     505,   325,   221,   335,     0,     0,   279,   290,   291,     0,
     292,   214,   274,     0,   276,   277,   278,   505,     0,   120,
       0,   624,   615,     0,     0,   609,   599,   561,   417,   222,
       0,     0,     0,   475,   368,   369,   370,   364,   363,   362,
     367,   366,   365,   371,   268,     0,   129,   264,   272,   267,
     269,   360,   268,     0,   556,   557,   596,   222,   258,     0,
       0,   403,     0,   406,   599,     0,   561,     0,     0,   146,
     232,   170,   238,     0,   238,     0,   129,     0,   129,   246,
     129,   250,     0,     0,   172,     0,   138,   130,     0,   129,
     134,   166,   223,     0,   356,     0,   325,   275,     0,     0,
       0,     0,     0,     0,     0,   118,   416,     0,   150,     0,
     505,   255,   146,   261,   266,   299,   268,   262,     0,     0,
     190,   273,   286,     0,   288,   289,   361,     0,   510,   505,
     155,     0,     0,   401,     0,   533,     0,   146,   230,   167,
       0,     0,     0,     0,     0,     0,   234,   130,     0,   243,
     130,   245,   130,     0,     0,   146,   140,   131,   128,   130,
       0,   325,   505,     0,   355,   208,   356,   331,   332,   324,
     272,     0,     0,   354,   336,   356,   281,   284,   280,   282,
     283,   285,   325,   620,   608,     0,   259,     0,     0,   265,
     287,     0,     0,   191,   192,     0,     0,     0,     0,   299,
       0,   299,     0,   253,     0,   226,     0,     0,     0,     0,
     236,     0,   146,   146,   235,     0,   247,   251,     0,   180,
     178,     0,   133,   127,   356,     0,     0,   333,     0,   508,
       0,   210,   356,   325,   300,   506,   294,   193,     0,   297,
     293,   295,   296,   298,   506,     0,   506,   325,   146,   228,
     156,   168,     0,   233,   237,   146,   146,   241,   242,   249,
     248,     0,   181,     0,   183,   197,   216,   337,     0,   334,
     505,     0,   373,   327,     0,    94,   279,   290,   291,     0,
       0,     0,   393,   212,   356,   507,   505,     0,   514,     0,
     129,   513,     0,   356,     0,   231,   239,   240,   179,     0,
       0,     0,    75,   338,   349,     0,   340,     0,     0,     0,
     350,     0,     0,   374,     0,   326,   505,     0,     0,     0,
       0,     0,   328,   195,     0,   381,   146,   515,   130,     0,
     146,   410,     0,   146,   217,   215,   339,   341,   342,   343,
       0,     0,     0,   505,   378,   505,   372,     0,   505,   505,
     329,   392,   506,     0,   378,   270,     0,   512,   511,     0,
     229,     0,   345,   346,   348,   344,   351,   375,   383,     0,
     374,   268,   394,   395,   502,   381,   383,   506,   506,   177,
     347,   378,   376,   383,   384,   508,   379,   505,     0,   271,
     382,   189,   501,   383,   380,     0,   375,   299,   377,   505,
     506,   385,     0,   390,   352,   146,   506,   268,   506,     0,
     330,     0,     0,   353,   391,     0,   387,   146,   506,     0,
       0,   386,   389,   388
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1077, -1077,    -3,  -778,   -76,   -58,  -510, -1077,   -27,  -191,
     186,   593, -1077,   -90,    -2,     6,  1027, -1077, -1077, -1077,
    1075, -1077, -1077,  -397, -1077, -1077,   926,   256,  -710,   624,
     947,  -109, -1077,    29, -1077, -1077, -1077, -1077, -1077, -1077,
     452, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077,    28, -1077, -1077, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077,  -592, -1077,  -610,   283, -1077,   146, -1077, -1077,  -503,
   -1077, -1077, -1077,   201, -1077, -1077, -1077, -1077, -1077, -1077,
    -711, -1077,   237, -1077,   311,   188,  -915,  -145,  -227, -1077,
     352, -1077,  -368,  -300, -1077,   243,  -937,    63, -1077, -1077,
   -1077,   983,  -130, -1077,   644, -1077,   643,  -799,   205, -1077,
    -774, -1077, -1077,    67, -1077, -1077, -1077, -1077, -1077, -1077,
   -1077, -1077,  -806,  -829, -1077,    65, -1077, -1076,    12, -1077,
      -8, -1077, -1077, -1077,   106,    68,   676, -1077,   696, -1077,
     332,   513,  1020, -1077,   -65, -1077,    11,    17, -1077,     4,
     330,  -854, -1077,  -114, -1077, -1077,    66, -1077, -1077,   136,
    -281, -1077,   539,   -63, -1077,    27,    33,    32, -1077, -1077,
   -1077, -1077, -1077,   133,   128, -1077, -1077,   782,  -111,  -257,
     622, -1077, -1077,   601,   468, -1077, -1077, -1077,   501
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   298,   299,   338,   465,     2,   301,   768,   197,
      92,   305,   306,    93,   135,   548,    96,    97,   522,   302,
     769,   499,   199,   527,   770,   869,   200,   771,   772,   201,
     186,   333,   549,   550,   761,   767,   999,  1043,   864,   509,
     510,   602,    99,   974,  1018,   100,   561,   214,   101,   156,
     157,   102,   103,   215,   104,   216,   105,   217,   688,   947,
    1081,   683,   686,   780,   759,  1030,   919,   851,   764,   853,
     106,   857,   858,   859,   860,   750,   107,   108,   109,   110,
     825,   826,   827,   828,   829,   900,   791,   792,   793,   794,
     795,   901,   796,   903,   904,   905,   968,   168,   471,   165,
     466,   472,   473,   202,   203,   206,   207,   874,   948,   949,
     782,  1049,  1085,  1086,  1087,  1088,  1089,  1090,  1186,   950,
     951,   952,   830,   831,  1051,  1052,  1053,  1148,  1135,  1165,
    1166,  1183,  1198,  1188,  1061,  1062,   187,   171,   172,   751,
     481,   482,   159,   637,   111,   112,   113,   114,   115,   137,
     603,  1065,  1104,   396,   981,  1070,  1071,   117,   403,   118,
     161,   345,   169,   119,   120,   121,   122,   182,   123,   124,
     125,   126,   127,   128,   129,   130,   163,   607,   615,   340,
     341,   342,   343,   330,   331,   703,   131,   513,   514
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,   906,   397,   300,   353,   872,   116,   498,    95,   311,
     311,   312,   312,   690,   833,   136,   138,   139,   140,   141,
     196,   762,   -14,   420,   155,  1133,   992,   529,    15,   143,
     144,    98,   146,   147,   148,   149,   150,  -280,   166,   395,
     395,   173,  1024,   377,  1026,   353,   524,   525,  -523,   181,
     181,  -293,  1014,   531,   158,   181,     3,   190,  1156,   866,
     174,   395,   167,   870,    15,   353,   208,  1068,   953,   647,
     325,   183,   541,   600,   651,   612,   188,   955,   797,    15,
     495,    41,    42,  -567,  -567,  1173,   142,   317,   318,   319,
     320,   321,   322,   323,   324,  1060,   582,   332,   116,   339,
      95,   313,   313,  1041,   325,   492,   344,   314,   314,   660,
     325,   355,   677,   847,  -567,   209,   679,   898,   371,   372,
     373,   907,   145,  1042,   906,   809,   923,   924,   325,   357,
     326,   496,   496,   404,   167,   132,    12,    13,    14,   848,
     953,   601,  1004,   837,   476,   311,   477,   312,   377,   953,
     208,   325,   355,   347,   162,   351,   421,   369,   370,   371,
     372,   373,   160,  1012,   423,   325,   390,   541,   170,  1006,
    1067,   196,  1072,  -522,   325,   468,   204,   175,   474,   798,
     349,   475,   176,   158,   425,    91,   479,   483,   484,   377,
     486,   487,   191,   167,   763,   193,   993,   209,   953,   890,
    1134,   880,   923,   924,   328,   329,   953,   315,   316,   307,
     336,   337,   154,   883,  1064,   975,   -84,   787,   788,   350,
     956,    91,   512,   515,   428,   177,   558,   909,  1073,  1002,
     178,   613,  1059,   497,   668,   179,    91,   313,   328,   329,
    1180,   926,  -224,   314,   328,   329,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   953,   154,
     855,   327,   328,   329,   311,   180,   312,   953,  -224,   417,
     418,   390,   -84,   311,   191,   312,   184,   193,  1154,  -561,
    -561,   978,  1059,   387,   388,   328,   329,  -225,  1016,   899,
     616,  1144,  -561,  1184,   710,   185,  1185,   419,   542,   328,
     329,  -227,   189,  1171,  1172,   398,   155,   536,   328,   329,
    -561,   671,   390,  -225,   339,   339,   210,   994,   639,   640,
     530,   339,   194,  1059,   195,   211,  1182,  -227,   619,  -606,
    -606,   547,  1190,  -606,  1192,   906,   158,   116,   212,   539,
     605,   609,  1164,   511,  1201,   213,   552,  1145,   303,   921,
    1164,   925,   132,    12,    13,    14,   313,  1164,    50,    51,
     559,   325,   314,   541,  -186,   313,   152,  1164,  -186,   560,
     528,   314,   356,   868,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,   581,   563,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   368,   369,   370,   371,   372,
     373,   151,   492,   181,   493,   620,   621,   622,   623,   624,
     625,   626,   627,   628,   629,   630,   631,   632,   399,   897,
     633,  -605,  -605,   635,   617,  -605,   162,   304,   377,   155,
    1168,    41,    42,   181,   160,   143,   144,  -560,   146,   147,
     148,   149,   352,   166,   358,   359,   720,   353,   190,   928,
     902,   931,   540,   933,   642,   208,   618,   400,   401,   158,
     205,   553,   940,  -568,   644,   196,  1191,   502,   712,   503,
     644,   132,    12,    13,    14,    15,    73,   173,  -564,  -564,
     504,   656,   505,   547,   154,   328,   329,   488,   402,   116,
     845,  -564,   663,   785,   390,   162,   132,    12,    13,    14,
     533,   534,   209,   160,    41,    42,   739,   674,   742,  -564,
     743,   132,    12,    13,    14,   426,   606,   610,   614,   614,
     970,   971,   387,   388,   132,    12,    13,    14,   336,   337,
     151,   535,   369,   370,   371,   372,   373,   191,   192,   889,
     193,   116,  1195,   709,   311,   638,   312,   614,   614,   641,
     427,   390,   339,   154,   355,   713,   507,   339,   508,   162,
     478,   429,   430,   718,   377,   719,  1196,   160,   721,  1197,
     786,   491,   902,   758,   431,    41,    42,   914,   787,   788,
      50,    51,   428,   786,   311,   194,   312,   195,   152,   360,
     361,   362,   432,   787,   788,   704,   705,  1019,  1020,  1020,
    1023,   311,   502,   312,   670,   726,   787,   788,   489,   728,
    -561,  -561,   490,    73,   730,   862,   863,   154,   132,    12,
      13,    14,   204,  -561,   506,   153,   838,    50,    51,   840,
     877,  1094,    91,  1095,   878,   152,   313,   311,  1101,   312,
    1102,  -561,   314,   699,   959,   960,   737,   474,   494,   474,
     899,   474,   307,   700,  1101,   747,  1130,   701,   753,   755,
     727,   501,   702,  1109,   729,   789,  1021,  1022,   731,   732,
     875,   746,  1047,   516,   749,  1048,   313,   517,    94,   512,
     346,   348,   314,   311,   116,   312,    95,   390,   191,   192,
     518,   193,   519,   313,   520,   922,   923,   924,   521,   314,
     787,   788,   154,   989,   923,   924,   803,   523,   547,    98,
     526,   824,  -571,   824,   116,   808,   599,   132,    12,    13,
      14,    15,  -570,   813,   353,   957,   958,   958,   961,   313,
     920,   551,   555,   556,   557,   314,   565,   562,   564,   644,
     566,   -43,   734,   -70,   -66,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   843,  -565,  -565,   339,   -67,
     -68,   758,   -62,   -63,   -65,   -64,   861,   -73,   -69,  -565,
     645,   646,   311,   966,   312,   313,   151,    50,    51,    73,
     850,   314,   760,  -571,   537,   152,    94,  -565,   543,   650,
     654,  -202,   116,   657,    95,   658,  -563,  -563,   987,   511,
     666,   886,   537,   667,   543,   537,   543,   339,   778,  -563,
      73,   783,   669,   205,   676,   678,  1001,    98,   681,   682,
     824,   685,   687,  -570,   689,   691,   547,  -563,   547,   891,
     692,   694,   116,   696,   116,   807,   714,   697,   684,   707,
     698,  -263,   483,   913,   717,   339,   353,   708,   779,   723,
     725,  -592,  -594,   132,    12,    13,    14,   733,   154,   736,
     765,   154,   946,   790,   313,   740,   744,   918,   773,   496,
     314,   153,   781,  1037,  1038,   775,   777,   800,    91,  -569,
     801,   846,   802,   804,   824,  1054,   366,   367,   368,   369,
     370,   371,   372,   373,   805,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   783,   806,   841,   547,  1074,
     832,   844,   786,   842,   116,   865,  1076,  1077,   852,  -562,
    -562,   377,   724,   395,   991,   854,   867,   871,   881,    73,
     882,   861,  -562,   997,   875,   787,   788,  1046,   883,  1069,
     892,   888,   876,   946,   893,  1009,  -569,   896,   895,   908,
    -562,  -593,   910,   915,   547,   154,   927,   930,   932,   929,
     116,   790,   934,   936,   939,   790,   935,   880,   938,   963,
     760,   941,   962,   964,   967,   547,   973,   976,   977,   978,
    1084,   116,   979,   766,   980,   983,  1096,  1136,   984,   547,
     985,  1139,   946,  1162,  1141,   116,  1039,   986,   990,  1040,
     946,   954,   995,  1029,   790,   918,   784,   789,   380,   381,
     382,   383,   384,   385,   386,   387,   388,  1084,   998,  1003,
      41,  1010,  1069,  1013,  1015,   547,   547,   975,  1017,  1025,
    1027,   116,   116,  1033,  1034,  1050,  1066,  1075,  1079,  1080,
    1092,  1175,  1097,   -75,   390,   -54,   391,   392,   393,   -55,
    1098,   154,   946,  1106,  1146,  1107,   790,   790,   790,   790,
    1000,   946,   547,   353,   547,   547,  1189,  1108,   116,   783,
     116,   116,  1115,  1110,   790,   790,  1160,  1113,  1200,   395,
    -522,  1114,  1118,  1119,  1120,  1121,  1125,   132,    12,    13,
      14,    15,  1122,  1123,  1124,  1140,  1138,  1149,  1128,  1129,
     873,  1151,  1179,  1187,  1161,  1132,  1177,  1143,  1194,   693,
     154,   354,   198,   937,   500,   485,   672,   884,   774,   154,
     916,   996,  1031,   969,   547,   894,   790,   547,  1008,   547,
     116,   879,   972,   116,  1155,   116,  1163,   467,   673,   824,
     675,  1007,  1117,   790,  1163,  1174,   151,    50,    51,  1126,
    1167,   790,   790,   790,   790,   152,  1100,  1169,   664,  1131,
     752,  1163,   653,   912,  1137,   424,   735,  1078,   715,   776,
       0,   611,     0,     0,  1091,   824,     0,   547,     0,     0,
      73,   942,     0,   116,     0,     0,   790,     0,   547,  -358,
       0,     0,     0,     0,   116,   943,  1199,     0,   814,   815,
     816,   817,   818,   819,   820,   821,   822,   823,   944,     0,
     965,  1091,     0,     0,     0,     0,     0,   154,     0,     0,
       0,     0,     0,     0,   154,     0,     0,     0,     0,   982,
       0,     0,    73,     0,     0,     0,     0,     0,     0,     0,
       0,   153,     0,     0,     0,     0,     0,   783,    91,     0,
       0,     0,     0,   745,     0,     0,     0,     0,     0,     4,
       5,   154,  1005,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,   945,   154,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
       0,     0,     0,     0,     0,     0,     0,     0,   134,     0,
      53,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
    1093,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     823,     0,     0,     0,     0,     0,  1105,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,    73,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1127,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,     0,    88,    89,
      90,    91,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,     0,  1147,   748,  1150,     0,     0,  1152,  1153,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,  -130,    17,   377,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,  1176,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,  1181,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,    53,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,   390,     0,   391,   392,   393,    84,     0,    73,
       0,     0,     0,     0,     0,     0,  -607,  -607,   368,   369,
     370,   371,   372,   373,    85,    86,     0,    87,   849,    88,
      89,    90,    91,     0,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,   377,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,  1170,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
       0,     0,     0,   134,     0,    53,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,  -607,  -607,     0,   387,   388,     0,   814,   815,
     816,   817,   818,   819,   820,   821,   822,   823,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,  -357,   390,  -359,   391,     0,     0,     0,
       0,    84,    73,     0,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,     0,     0,     0,     0,    85,    86,
       0,    87,   917,    88,    89,    90,    91,     0,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,  1178,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,    53,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,   814,   815,   816,   817,   818,   819,   820,
     821,   822,   823,   814,   815,   816,   817,   818,   819,   820,
     821,   822,   823,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,    73,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,  1028,    88,    89,    90,
      91,     0,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,     0,     0,     0,     0,     0,     0,     0,
       0,   134,     0,    53,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
     567,    88,    89,    90,    91,     0,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   132,    12,    13,
      14,    15,     0,     0,    17,     0,   942,    18,    19,    20,
      21,    22,     0,     0,  -358,    23,    24,    25,     0,    26,
      27,    28,     0,   814,   815,   816,   817,   818,   819,   820,
     821,   822,   823,   944,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,    41,    42,    41,    42,    43,
       0,     0,    46,     0,     0,     0,   133,    73,   352,    50,
      51,     0,     0,     0,     0,   134,     0,   152,    54,    55,
       0,     0,     0,    57,    58,    59,    60,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,     0,   -47,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     132,    12,    13,    14,    15,     0,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   352,    50,    51,     0,
       0,     0,     0,     0,    37,   152,     0,     0,    41,    42,
       0,    57,    58,    59,    60,    46,     0,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,     0,   134,     0,
      73,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,   374,   375,   376,
       0,     0,     0,   377,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,    88,    89,
      90,    91,   363,     0,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   374,   375,   376,     0,     0,
       0,   377,     0,     0,     0,     0,     0,   374,   375,   376,
       0,     0,     0,   377,     0,     0,     0,     0,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   363,
       0,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,     0,     0,   389,   390,     0,   391,   392,
     393,     0,   374,   375,   376,     0,     0,     0,   377,   706,
       0,     0,     0,     0,   374,   375,   376,     0,     0,     0,
     377,     0,     0,     0,     0,     0,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,     0,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   363,
       0,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,     0,     0,   389,   390,     0,   391,   392,   393,     0,
       0,     0,     0,     0,     0,   389,   390,   716,   391,   392,
     393,     0,   374,   375,   376,     0,     0,     0,   377,   834,
       0,     0,     0,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,     0,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,     0,     0,
     389,   390,     0,   391,   392,   393,     0,     0,     0,     0,
       0,     0,   389,   390,   835,   391,   392,   393,     0,   374,
     375,   376,     0,     0,     0,   377,   836,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,     0,   363,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     389,   390,     0,   391,   392,   393,     0,     0,   374,   375,
     376,     0,     0,  1134,   377,     0,     0,     0,     0,     0,
       0,   374,   375,   376,     0,     0,     0,   377,     0,     0,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,     0,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   389,   390,     0,
     391,   392,   393,     0,     0,   648,   374,   375,   376,   544,
       0,     0,   377,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,     0,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,     0,   389,   390,     0,   391,
     392,   393,     0,     0,     0,     0,     0,     0,   544,   389,
     390,     0,   391,   392,   393,     0,   374,   375,   376,     0,
       0,   634,   377,     0,     0,     0,     0,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   363,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,     0,   389,   390,     0,   391,   392,   393,
       0,   374,   375,   376,     0,     0,   636,   377,     0,     0,
       0,     0,     0,     0,   374,   375,   376,     0,     0,     0,
     377,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   363,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   389,   390,     0,   391,   392,   393,
       0,   374,   375,   376,     0,     0,   652,   377,     0,     0,
       0,     0,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,     0,     0,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,     0,   389,
     390,     0,   391,   392,   393,     0,     0,     0,     0,     0,
       0,   655,   389,   390,     0,   391,   392,   393,     0,   374,
     375,   376,     0,     0,   661,   377,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,     0,   389,
     390,     0,   391,   392,   393,     0,   374,   375,   376,     0,
       0,   662,   377,     0,     0,     0,     0,     0,     0,   374,
     375,   376,     0,     0,     0,   377,     0,     0,     0,     0,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,     0,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,     0,   389,   390,     0,
     391,   392,   393,     0,   374,   375,   376,     0,     0,   680,
     377,     0,     0,     0,     0,     0,     0,   374,   375,   376,
       0,     0,     0,   377,     0,     0,     0,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,     0,     0,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   389,   390,     0,   391,   392,   393,
       0,     0,     0,     0,     0,     0,   695,   389,   390,     0,
     391,   392,   393,     0,     0,     0,     0,     0,     0,   810,
       0,     0,     0,     0,     0,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,     0,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   389,   390,     0,   391,   392,   393,     0,     0,
       0,     0,     0,     0,   811,   389,   390,     0,   391,   392,
     393,     0,     0,     4,     5,     0,     0,   839,     6,     7,
       8,     0,     9,    10,   433,    12,    13,    14,    15,     0,
       0,    17,     0,     0,   434,   435,   436,   437,   438,   225,
     226,   227,   439,   440,    25,   230,   441,   442,   443,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   444,   254,
     255,   256,   445,   446,   259,   260,   261,   262,   263,   447,
     265,   266,   267,   448,   269,   270,   271,   272,   273,     0,
       0,     0,   449,   275,   276,   450,   451,     0,   279,   280,
     281,   282,   283,   284,   285,   452,   453,   288,   454,   455,
     456,   457,   458,   459,   460,   461,   462,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,   463,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,    86,   464,
       0,     0,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   433,    12,    13,
      14,    15,     0,     0,    17,   377,     0,   434,   435,   436,
     437,   438,   225,   226,   227,   439,   440,    25,   230,   441,
     442,   443,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   444,   254,   255,   256,   445,   446,   259,   260,   261,
     262,   263,   447,   265,   266,   267,   448,   269,   270,   271,
     272,   273,     0,     0,     0,   449,   275,   276,   450,   451,
       0,   279,   280,   281,   282,   283,   284,   285,   452,   453,
     288,   454,   455,   456,   457,   458,   459,   460,   461,   462,
      73,     0,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,   390,     0,
     391,   392,   393,    84,     0,     0,     0,     0,     0,   469,
       0,     0,   365,   366,   367,   368,   369,   370,   371,   372,
     373,    86,   470,     0,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     433,    12,    13,    14,    15,     0,     0,    17,   377,     0,
     434,   435,   436,   437,   438,   225,   226,   227,   439,   440,
      25,   230,   441,   442,   443,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   444,   254,   255,   256,   445,   446,
     259,   260,   261,   262,   263,   447,   265,   266,   267,   448,
     269,   270,   271,   272,   273,     0,     0,     0,   449,   275,
     276,   450,   451,     0,   279,   280,   281,   282,   283,   284,
     285,   452,   453,   288,   454,   455,   456,   457,   458,   459,
     460,   461,   462,    73,     0,   380,   381,   382,   383,   384,
     385,   386,   387,   388,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,   390,     0,   391,   392,   393,    84,     0,     0,     0,
       0,     0,   738,     0,     0,     0,   366,   367,   368,   369,
     370,   371,   372,   373,    86,   741,     0,     0,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   433,    12,    13,    14,    15,     0,     0,
      17,   377,     0,   434,   435,   436,   437,   438,   225,   226,
     227,   439,   440,    25,   230,   441,   442,   443,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   444,   254,   255,
     256,   445,   446,   259,   260,   261,   262,   263,   447,   265,
     266,   267,   448,   269,   270,   271,   272,   273,     0,     0,
       0,   449,   275,   276,   450,   451,     0,   279,   280,   281,
     282,   283,   284,   285,   452,   453,   288,   454,   455,   456,
     457,   458,   459,   460,   461,   462,    73,     0,  -607,  -607,
    -607,  -607,   384,   385,  -607,   387,   388,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,   390,     0,   391,     0,     0,    84,
       0,     0,     0,     0,     0,   738,     0,     0,     0,   366,
     367,   368,   369,   370,   371,   372,   373,    86,     0,     0,
       0,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,   377,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,    46,    47,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,     0,     0,    61,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,     0,     0,     0,     0,   390,     0,   391,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,   799,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,    46,    47,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,    56,    57,    58,    59,    60,     0,     0,    61,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,   885,    88,    89,    90,
      91,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,     0,    35,
       0,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,     0,    46,    47,     0,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
       0,     0,    61,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,    86,     0,    87,     0,
      88,    89,    90,    91,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,    52,     0,    53,    54,    55,   545,    57,
      58,    59,    60,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,   546,    88,    89,    90,    91,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,    52,     0,    53,    54,
      55,   545,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,   665,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,   812,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,    52,
       0,    53,    54,    55,   545,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,     0,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,   911,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,   545,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,     0,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,   988,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,    52,     0,    53,    54,    55,
     545,    57,    58,    59,    60,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,  1032,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
      53,    54,    55,   545,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,     0,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,    52,     0,    53,    54,    55,   545,    57,    58,    59,
      60,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
    1044,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
    1112,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,   545,
      57,    58,    59,    60,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,     0,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,   545,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,  1157,    88,    89,    90,
      91,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,     0,    35,
       0,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,    44,    45,     0,     0,    46,     0,     0,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
      52,     0,    53,    54,    55,   545,    57,    58,    59,    60,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,    86,     0,    87,  1158,
      88,    89,    90,    91,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,    52,     0,    53,    54,    55,   545,    57,
      58,    59,    60,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,  1159,    88,    89,    90,    91,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,    52,     0,    53,    54,
      55,   545,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,  1193,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,    52,
       0,    53,    54,    55,   545,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,  1203,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,   545,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,   785,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,     0,    88,    89,    90,    91,  1055,    12,    13,    14,
       0,     0,     0,     0,     0,     0,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,     0,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,  1056,   269,   270,   271,   272,
     273,     0,     0,     0,   274,   275,   276,   277,   278,     0,
     279,   280,   281,   282,   283,   284,   285,   286,  1057,  1058,
     289,   290,   291,   292,   293,   294,   295,   296,   297,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
     789,    44,    45,     0,     0,    46,     0,     0,    48,    49,
       0,     0,     0,     0,     0,     0,     0,     0,   134,     0,
      53,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,     0,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   132,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   133,     0,     0,     0,     0,     0,     0,     0,
       0,   134,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   334,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,   335,     0,     0,     0,     0,
     336,   337,     0,     0,     0,     0,     0,    86,     0,     0,
       0,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   132,    12,    13,    14,
      15,     0,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   133,     0,     0,     0,     0,
       0,     0,     0,     0,   134,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   336,   337,     0,     0,     0,     0,     0,
      86,     0,     0,     0,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   132,
      12,    13,    14,    15,     0,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,     0,     0,     0,   133,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,     0,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     711,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   336,   337,     0,     0,
       0,     0,     0,    86,     0,     0,     0,    88,    89,    90,
      91,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,   132,    12,    13,    14,    15,     0,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
      41,    42,     0,     0,     0,     0,     0,    46,     0,     0,
       0,   133,     0,     0,     0,     0,     0,     0,     0,     0,
     134,     0,     0,    54,    55,     0,     0,     0,     0,     0,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
      88,    89,    90,    91,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,   132,    12,    13,    14,    15,
       0,     0,    17,   538,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
      46,     0,     0,     0,   133,     0,     0,     0,     0,     0,
       0,     0,     0,   134,     0,     0,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    86,
       0,     0,     0,    88,    89,    90,    91,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,   132,    12,
      13,    14,    15,     0,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,     0,     0,     0,    41,    42,     0,     0,
       0,     0,     0,    46,     0,     0,     0,   133,     0,     0,
       0,     0,     0,     0,     0,     0,   134,     0,     0,    54,
      55,     0,     0,     0,     0,     0,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,   643,     0,     0,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,   132,    12,    13,    14,    15,     0,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,    46,     0,     0,     0,
     133,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,     0,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,   649,     0,     0,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   132,    12,    13,    14,    15,     0,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    41,    42,     0,     0,     0,     0,     0,    46,
       0,     0,     0,   133,     0,     0,     0,     0,     0,     0,
       0,     0,   134,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
     754,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   132,    12,    13,
      14,    15,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     856,    37,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,    46,     0,     0,     0,   133,     0,     0,     0,
       0,     0,     0,     0,     0,   134,     0,     0,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,     0,     0,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     132,    12,    13,    14,    15,     0,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    37,     0,     0,     0,    41,    42,
       0,     0,     0,     0,     0,    46,     0,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,     0,   134,     0,
       0,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   132,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   133,     0,     0,     0,     0,     0,     0,     0,
       0,   134,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,   363,  1035,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
     374,   375,   376,     0,     0,     0,   377,     0,     0,    84,
       0,     0,     0,     0,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   164,     0,     0,
       0,    88,    89,    90,    91,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,     0,   374,   375,
     376,     0,     0,     0,   377,     0,     0,     0,   942,     0,
       0,     0,     0,     0,     0,     0,  -358,     0,   374,   375,
     376,     0,   943,     0,   377,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   944,     0,     0,     0,     0,
       0,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,     0,     0,     0,     0,     0,     0,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   389,   390,
       0,   391,   392,   393,     0,     0,     0,     0,  1036,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,     0,     0,  1011,     0,     0,   389,   390,     0,   391,
     392,   393,     0,     0,     0,     0,   394,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   389,   390,     0,   391,
     392,   393,     0,     0,     0,   363,   532,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   363,     0,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   374,   375,
     376,     0,     0,     0,   377,     0,     0,     0,     0,     0,
     374,   375,   376,     0,     0,     0,   377,     0,     0,     0,
       0,     0,     0,     0,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   374,   375,   376,
       0,     0,     0,   377,     0,     0,     0,     0,     0,   374,
     375,   376,     0,     0,     0,   377,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   389,   390,     0,   391,
     392,   393,     0,     0,     0,     0,  1202,     0,   389,   390,
       0,   391,   392,   393,     0,     0,     0,   887,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,     0,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,     0,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   389,   390,     0,   391,   392,
     393,     0,     0,  -408,     0,     0,     0,   389,   390,     0,
     391,   392,   393,     0,     0,  -405,   374,   375,   376,     0,
       0,     0,   377,     0,     0,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   363,   722,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   374,
     375,   376,     0,     0,     0,   377,     0,     0,     0,     0,
       0,   374,   375,   376,     0,     0,     0,   377,     0,     0,
       0,     0,     0,   659,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   374,   375,
     376,     0,     0,     0,   377,     0,     0,     0,     0,     0,
     554,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   389,   390,     0,   391,   392,   393,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   389,   390,     0,
     391,   392,   393,     0,     0,     0,     0,     0,     0,   389,
     390,     0,   391,   392,   393,     0,     0,     0,     0,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     363,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   363,     0,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,     0,     0,   389,   390,     0,   391,
     392,   393,     0,     0,   375,   376,     0,     0,     0,   377,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   377,     0,     0,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   363,     0,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     376,     0,     0,     0,   377,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   377,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   377,     0,     0,
       0,     0,     0,     0,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   422,
       0,   389,   390,     0,   391,   392,   393,     0,     0,     0,
       0,     0,     0,   389,   390,     0,   391,   392,   393,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,     0,     0,     0,   389,   390,     0,   391,
     392,   393,     0,     0,     0,     0,     0,     0,   389,   390,
       0,   391,   392,   393,     0,     0,     0,   218,     0,   389,
     390,    15,   391,   392,   393,     0,     0,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,     0,     0,     0,   274,   275,   276,   277,   278,
       0,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     942,     0,     0,     0,     0,     0,     0,     0,  -358,     0,
       0,     0,     0,     0,   943,     0,     0,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   944,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     218,    73,     0,   604,    15,     0,     0,     0,    91,     0,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
       0,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,  1045,     0,     0,   274,   275,
     276,   277,   278,     0,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,     0,   132,    12,    13,    14,    15,     0,
       0,    17,     0,     0,     0,   132,    12,    13,    14,    15,
       0,     0,    17,     0,   942,     0,   308,     0,     0,     0,
       0,     0,  -358,     0,     0,     0,     0,   308,   943,     0,
       0,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     823,   944,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   151,     0,     0,   608,     0,     0,     0,
       0,    91,   134,     0,   151,    73,     0,     0,     0,     0,
       0,     0,     0,   134,     0,   756,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1063,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   336,   337,     0,     0,     0,     0,     0,   309,     0,
       0,     0,   757,     0,    90,    91,     0,     0,     0,   309,
       0,     0,     0,   310,     0,    90,    91,   433,    12,    13,
      14,     0,     0,     0,     0,     0,     0,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,  1082,   269,   270,   271,
     272,   273,     0,     0,     0,   274,   275,   276,   277,   278,
       0,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
       0,     0,   942,     0,     0,     0,     0,     0,     0,     0,
    -358,     0,     0,     0,     0,     0,   943,     0,     0,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   823,   944,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   433,    12,    13,    14,     0,     0,
       0,     0,     0,    73,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,  1083,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,  1082,   269,   270,   271,   272,   273,     0,
       0,     0,   274,   275,   276,   277,   278,  1103,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   942,     0,     0,
       0,     0,     0,     0,     0,  -358,     0,     0,     0,     0,
       0,   943,     0,     0,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   944,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,  1111,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     823,   275,   276,   277,   278,     0,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   218,     0,     0,   219,     0,
       0,     0,     0,     0,     0,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
       0,     0,     0,   274,   275,   276,   277,   278,     0,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,     0,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,     0,     0,     0,   274,   275,   276,   277,
     278,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,  1099,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,     0,     0,     0,   274,
     275,   276,   277,   278,     0,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297
};

static const yytype_int16 yycheck[] =
{
       2,   830,   116,    61,    94,   779,     2,   198,     2,    74,
      75,    74,    75,   523,   725,     4,     5,     6,     7,     8,
      47,     6,     6,   137,    26,     4,     6,   308,    27,    18,
      19,     2,    21,    22,    23,    24,    25,     7,    27,   115,
     116,    30,   979,    44,   981,   135,   303,   304,   163,    38,
      39,     7,   967,   310,    26,    44,     0,    46,  1134,   769,
      31,   137,   177,   773,    27,   155,    49,    27,   874,   466,
      27,    39,    29,    23,   471,    23,    44,   876,   688,    27,
     189,    71,    72,   150,   151,  1161,     6,    76,    77,    78,
      79,    80,    81,    82,    83,  1010,   377,    86,    94,    88,
      94,    74,    75,     7,    27,   174,    29,    74,    75,   178,
      27,    94,   509,   152,   181,    52,   513,     5,    14,    15,
      16,   832,   177,    27,   953,   717,    65,    66,    27,   101,
      29,    62,    62,     4,   177,    23,    24,    25,    26,   178,
     946,    91,   941,   735,   174,   210,   176,   210,    44,   955,
     133,    27,   135,    29,    26,    92,   145,    12,    13,    14,
      15,    16,    26,   962,   153,    27,   167,    29,   177,   943,
    1024,   198,  1026,   163,    27,   164,    48,   177,   167,   689,
     179,   170,   177,   155,   156,   184,   175,   176,   177,    44,
     179,   180,    23,   177,   179,    26,   176,   134,  1004,   809,
     179,   171,    65,    66,   161,   162,  1012,    74,    75,    73,
     170,   171,    26,   171,  1013,   171,     6,   105,   106,    91,
     178,   184,   211,   212,   161,   177,   183,   837,  1027,   939,
     177,   179,  1010,   164,   164,   177,   184,   210,   161,   162,
    1177,   180,   152,   210,   161,   162,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,  1064,    73,
     763,   160,   161,   162,   329,   177,   329,  1073,   178,   140,
     141,   167,    62,   338,    23,   338,    23,    26,  1132,   150,
     151,   171,  1060,   138,   139,   161,   162,   152,   178,   177,
     401,  1120,   163,   176,   551,    23,   179,   168,   160,   161,
     162,   152,   179,  1157,  1158,   163,   308,   160,   161,   162,
     181,   502,   167,   178,   303,   304,   177,   180,   429,   430,
     309,   310,    71,  1101,    73,   177,  1180,   178,   404,   174,
     175,   333,  1186,   178,  1188,  1164,   308,   333,   177,   328,
     398,   399,  1148,   210,  1198,   177,   335,  1121,   177,   852,
    1156,   854,    23,    24,    25,    26,   329,  1163,    83,    84,
     349,    27,   329,    29,   174,   338,    91,  1173,   178,   352,
     307,   338,    97,   770,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   359,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,    11,    12,    13,    14,    15,
      16,    82,   174,   402,   176,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   163,   826,
     419,   174,   175,   422,   402,   178,   308,   177,    44,   441,
    1151,    71,    72,   432,   308,   434,   435,   163,   437,   438,
     439,   440,    82,   442,    49,    50,   565,   547,   447,   856,
     828,   858,   329,   860,   432,   448,   403,   150,   151,   441,
      27,   338,   869,   177,   463,   502,  1187,   174,   554,   176,
     469,    23,    24,    25,    26,    27,   116,   476,   150,   151,
     174,   480,   176,   495,   308,   161,   162,   176,   181,   495,
     757,   163,   491,     5,   167,   377,    23,    24,    25,    26,
     150,   151,   449,   377,    71,    72,   646,   506,   648,   181,
     650,    23,    24,    25,    26,    97,   398,   399,   400,   401,
     898,   899,   138,   139,    23,    24,    25,    26,   170,   171,
      82,   181,    12,    13,    14,    15,    16,    23,    24,   806,
      26,   547,   152,   547,   619,   427,   619,   429,   430,   431,
     163,   167,   551,   377,   547,   554,   174,   556,   176,   441,
      54,   150,   151,   562,    44,   564,   176,   441,   567,   179,
      82,     4,   950,   659,   163,    71,    72,   844,   105,   106,
      83,    84,   529,    82,   659,    71,   659,    73,    91,    49,
      50,    51,   181,   105,   106,   180,   181,   975,   976,   977,
     978,   676,   174,   676,   176,   604,   105,   106,   176,   608,
     150,   151,   176,   116,   613,    76,    77,   441,    23,    24,
      25,    26,   504,   163,     4,   177,   745,    83,    84,   748,
     785,   174,   184,   176,   789,    91,   619,   712,   174,   712,
     176,   181,   619,    13,   881,   882,   645,   646,   176,   648,
     177,   650,   526,    23,   174,   654,   176,    27,   657,   658,
     607,   176,    32,  1070,   611,   177,   976,   977,   615,   616,
     174,   652,   176,   178,   655,   179,   659,    23,   690,   678,
      89,    90,   659,   758,   690,   758,   690,   167,    23,    24,
      23,    26,    23,   676,    23,    64,    65,    66,   176,   676,
     105,   106,   526,    64,    65,    66,   705,   179,   720,   690,
     174,   723,   177,   725,   720,   714,   396,    23,    24,    25,
      26,    27,   177,   722,   824,   880,   881,   882,   883,   712,
     849,   177,   175,   174,   182,   712,     6,   177,   177,   738,
     176,     6,   619,     6,     6,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,   754,   150,   151,   757,     6,
       6,   847,     6,     6,     6,     6,   765,     6,     6,   163,
       6,   174,   847,   892,   847,   758,    82,    83,    84,   116,
     761,   758,   659,   177,   326,    91,   798,   181,   330,   174,
     177,    97,   798,   176,   798,   174,   150,   151,   917,   676,
      23,   800,   344,   179,   346,   347,   348,   806,   682,   163,
     116,   685,   176,    27,   174,   174,   935,   798,   176,   101,
     832,   101,     6,   177,   179,   178,   838,   181,   840,   810,
     178,   175,   838,   175,   840,   712,     4,    23,   518,   180,
      23,   178,   841,   842,    23,   844,   946,   177,   102,   177,
     177,   177,   177,    23,    24,    25,    26,   175,   682,   175,
     179,   685,   874,   687,   847,   178,   178,   848,   179,    62,
     847,   177,   179,   992,   993,   178,   178,     4,   184,   177,
      32,   758,   175,   178,   896,  1009,     9,    10,    11,    12,
      13,    14,    15,    16,   178,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,   779,   177,   176,   920,  1028,
     177,   177,    82,   174,   920,    62,  1035,  1036,   176,   150,
     151,    44,   602,  1009,   923,   176,   174,   176,     7,   116,
       7,   930,   163,   932,   174,   105,   106,  1005,   171,  1025,
       6,   176,   179,   955,   176,   951,   177,   174,   178,   178,
     181,   177,   176,   175,   966,   779,   174,   174,   174,   180,
     966,   785,   177,    23,   174,   789,   179,   171,   180,   180,
     847,   179,   179,   178,     6,   987,   170,     7,     7,   171,
    1048,   987,   178,   663,    79,   176,  1054,  1106,   178,  1001,
     178,  1110,  1004,   180,  1113,  1001,   995,   178,   176,   998,
    1012,   875,   152,   984,   828,   986,   686,   177,   131,   132,
     133,   134,   135,   136,   137,   138,   139,  1085,   152,   180,
      71,    73,  1108,   179,   152,  1037,  1038,   171,   165,   177,
     179,  1037,  1038,   176,   176,    27,    27,   176,   178,     4,
       4,  1165,     4,     4,   167,     4,   169,   170,   171,     4,
       4,   875,  1064,   179,  1122,    27,   880,   881,   882,   883,
     934,  1073,  1074,  1163,  1076,  1077,  1185,   174,  1074,   943,
    1076,  1077,   176,   179,   898,   899,  1144,   179,  1197,  1165,
     163,  1080,   176,   176,    62,    80,    27,    23,    24,    25,
      26,    27,   163,  1092,   179,   176,   178,     4,  1097,  1098,
     780,   177,    23,   177,   179,  1104,   178,  1120,   178,   526,
     934,    94,    47,   867,   198,   178,   502,   797,   676,   943,
     847,   930,   986,   896,  1136,   824,   950,  1139,   950,  1141,
    1136,   789,   899,  1139,  1133,  1141,  1148,   164,   504,  1151,
     507,   946,  1085,   967,  1156,  1163,    82,    83,    84,  1094,
    1149,   975,   976,   977,   978,    91,  1060,  1155,   492,  1101,
     657,  1173,   476,   841,  1108,   155,   637,  1041,   556,   678,
      -1,   399,    -1,    -1,  1048,  1187,    -1,  1189,    -1,    -1,
     116,    65,    -1,  1189,    -1,    -1,  1010,    -1,  1200,    73,
      -1,    -1,    -1,    -1,  1200,    79,  1195,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
     890,  1085,    -1,    -1,    -1,    -1,    -1,  1041,    -1,    -1,
      -1,    -1,    -1,    -1,  1048,    -1,    -1,    -1,    -1,   909,
      -1,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,  1121,   184,    -1,
      -1,    -1,    -1,     6,    -1,    -1,    -1,    -1,    -1,    12,
      13,  1085,   942,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,   180,  1121,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
    1050,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    -1,    -1,    -1,    -1,    -1,  1066,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1096,    -1,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
     183,   184,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,  1123,     6,  1125,    -1,    -1,  1128,  1129,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,   178,    30,    44,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,  1167,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,  1179,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,   167,    -1,   169,   170,   171,   159,    -1,   116,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,   176,   177,    -1,   179,     6,   181,
     182,   183,   184,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    44,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,   180,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,   135,   136,    -1,   138,   139,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,    -1,    71,   167,    73,   169,    -1,    -1,    -1,
      -1,   159,   116,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,     6,   181,   182,   183,   184,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,   180,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    -1,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,     6,   181,   182,   183,
     184,    -1,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
       6,   181,   182,   183,   184,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    65,    33,    34,    35,
      36,    37,    -1,    -1,    73,    41,    42,    43,    -1,    45,
      46,    47,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    71,    72,    71,    72,    73,
      -1,    -1,    78,    -1,    -1,    -1,    82,   116,    82,    83,
      84,    -1,    -1,    -1,    -1,    91,    -1,    91,    94,    95,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,     6,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    67,    91,    -1,    -1,    71,    72,
      -1,    97,    98,    99,   100,    78,    -1,    -1,    -1,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
     116,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,   181,   182,
     183,   184,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    38,    39,    40,    -1,    -1,    -1,    44,   180,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    -1,   166,   167,    -1,   169,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,   166,   167,   180,   169,   170,
     171,    -1,    38,    39,    40,    -1,    -1,    -1,    44,   180,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
     166,   167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   166,   167,   180,   169,   170,   171,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,   180,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
     166,   167,    -1,   169,   170,   171,    -1,    -1,    38,    39,
      40,    -1,    -1,   179,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,   174,    38,    39,    40,   178,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   178,   166,
     167,    -1,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,   178,    44,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    38,    39,    40,    -1,    -1,   178,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    38,    39,    40,    -1,    -1,   178,    44,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,   166,   167,    -1,   169,   170,   171,    -1,    38,
      39,    40,    -1,    -1,   178,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,   178,    44,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    38,    39,    40,    -1,    -1,   178,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   178,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   166,   167,    -1,   169,   170,   171,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    12,    13,    -1,    -1,   178,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,   165,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,   177,   178,
      -1,    -1,   181,   182,   183,   184,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    30,    44,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,   171,   159,    -1,    -1,    -1,    -1,    -1,   165,
      -1,    -1,     8,     9,    10,    11,    12,    13,    14,    15,
      16,   177,   178,    -1,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    44,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,   167,    -1,   169,   170,   171,   159,    -1,    -1,    -1,
      -1,    -1,   165,    -1,    -1,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,   177,   178,    -1,    -1,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    44,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      -1,    -1,    -1,    -1,   167,    -1,   169,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,   165,    -1,    -1,    -1,     9,
      10,    11,    12,    13,    14,    15,    16,   177,    -1,    -1,
      -1,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    44,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,   167,    -1,   169,
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    78,    79,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,   103,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    -1,    -1,    78,    79,    -1,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
     181,   182,   183,   184,    12,    13,    -1,    -1,    -1,    17,
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
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,    12,    13,    -1,
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
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    51,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,    -1,   181,   182,   183,   184,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    59,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
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
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      57,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    74,    75,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,    -1,   181,   182,   183,   184,    12,    13,
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
     144,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,    12,    13,    -1,    -1,    -1,    17,
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
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,    12,    13,    -1,
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
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,    -1,   181,   182,   183,   184,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
     177,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,   165,    -1,    -1,    -1,    -1,
     170,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
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
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,   181,   182,   183,   184,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,   181,   182,   183,
     184,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
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
     141,   142,   143,   144,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
     181,   182,   183,   184,    12,    13,    -1,    -1,    -1,    17,
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
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,   181,   182,   183,   184,    12,    13,    -1,
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
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,   178,    -1,    -1,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,
      72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   177,   178,    -1,    -1,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
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
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181,   182,   183,   184,    12,    13,    -1,    -1,
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
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,    -1,
      38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,   177,    -1,    -1,
      -1,   181,   182,   183,   184,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    38,    39,
      40,    -1,    79,    -1,    44,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,
      -1,   169,   170,   171,    -1,    -1,    -1,    -1,   176,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   180,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,     5,   176,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     5,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,   176,    -1,   166,   167,
      -1,   169,   170,   171,    -1,    -1,    -1,   175,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,   166,   167,    -1,   169,   170,
     171,    -1,    -1,   174,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,   174,    38,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    62,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     5,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,   166,   167,    -1,   169,   170,   171,    -1,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,
      -1,   169,   170,   171,    -1,    -1,    -1,    23,    -1,   166,
     167,    27,   169,   170,   171,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    -1,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      23,   116,    -1,   179,    27,    -1,    -1,    -1,   184,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      -1,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,   180,    -1,    -1,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    -1,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    -1,    23,    24,    25,    26,    27,
      -1,    -1,    30,    -1,    65,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    45,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    -1,    -1,   179,    -1,    -1,    -1,
      -1,   184,    91,    -1,    82,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181,    -1,   183,   184,    -1,    -1,    -1,   177,
      -1,    -1,    -1,   181,    -1,   183,   184,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    -1,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,   116,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,   180,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    95,   180,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,   180,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    -1,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    23,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,
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
     112,   113,   114,   115
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   186,   191,     0,    12,    13,    17,    18,    19,    21,
      22,    23,    24,    25,    26,    27,    28,    30,    33,    34,
      35,    36,    37,    41,    42,    43,    45,    46,    47,    48,
      52,    53,    54,    56,    58,    60,    63,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    78,    79,    81,    82,
      83,    84,    91,    93,    94,    95,    96,    97,    98,    99,
     100,   103,   104,   105,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   159,   176,   177,   179,   181,   182,
     183,   184,   195,   198,   199,   200,   201,   202,   218,   227,
     230,   233,   236,   237,   239,   241,   255,   261,   262,   263,
     264,   329,   330,   331,   332,   333,   334,   342,   344,   348,
     349,   350,   351,   353,   354,   355,   356,   357,   358,   359,
     360,   371,    23,    82,    91,   199,   331,   334,   331,   331,
     331,   331,     6,   331,   331,   177,   331,   331,   331,   331,
     331,    82,    91,   177,   195,   199,   234,   235,   236,   327,
     344,   345,   359,   361,   177,   284,   331,   177,   282,   347,
     177,   322,   323,   331,   218,   177,   177,   177,   177,   177,
     177,   331,   352,   352,    23,    23,   215,   321,   352,   179,
     331,    23,    24,    26,    71,    73,   193,   194,   205,   207,
     211,   214,   288,   289,   359,    27,   290,   291,   332,   282,
     177,   177,   177,   177,   232,   238,   240,   242,    23,    26,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    91,    92,    93,    94,    95,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   187,   188,
     190,   192,   204,   177,   177,   196,   197,   344,    45,   177,
     181,   329,   348,   350,   351,   358,   358,   331,   331,   331,
     331,   331,   331,   331,   331,    27,    29,   160,   161,   162,
     368,   369,   331,   216,   104,   165,   170,   171,   189,   331,
     364,   365,   366,   367,    29,   346,   368,    29,   368,   179,
     359,   282,    82,   198,   201,   332,    97,   236,    49,    50,
      49,    50,    51,     5,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    38,    39,    40,    44,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   166,
     167,   169,   170,   171,   176,   189,   338,   338,   163,   163,
     150,   151,   181,   343,     4,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   140,   141,   168,
     338,   331,   152,   331,   327,   236,    97,   163,   282,   150,
     151,   163,   181,    23,    33,    34,    35,    36,    37,    41,
      42,    45,    46,    47,    67,    71,    72,    78,    82,    91,
      94,    95,   104,   105,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   165,   178,   190,   285,   286,   331,   165,
     178,   283,   286,   287,   331,   331,   174,   176,    54,   331,
     149,   325,   326,   331,   331,   215,   331,   331,   176,   176,
     176,     4,   174,   176,   176,   216,    62,   164,   194,   206,
     211,   176,   174,   176,   174,   176,     4,   174,   176,   224,
     225,   358,   331,   372,   373,   331,   178,    23,    23,    23,
      23,   176,   203,   179,   364,   364,   174,   208,   282,   345,
     331,   364,   176,   150,   151,   181,   160,   369,    31,   331,
     358,    29,   160,   369,   178,    96,   180,   199,   200,   217,
     218,   177,   331,   358,   152,   175,   174,   182,   183,   331,
     332,   231,   177,   218,   177,     6,   176,     6,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   345,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   335,
      23,    91,   226,   335,   179,   190,   359,   362,   179,   190,
     359,   362,    23,   179,   359,   363,   363,   352,   282,   189,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   178,   331,   178,   328,   359,   363,
     363,   359,   352,   178,   331,     6,   174,   208,   174,   178,
     174,   208,   178,   323,   177,   178,   331,   176,   174,    62,
     178,   178,   178,   331,   321,   180,    23,   179,   164,   176,
     176,   194,   214,   289,   331,   291,   174,   208,   174,   208,
     178,   176,   101,   246,   335,   101,   247,     6,   243,   179,
     191,   178,   178,   196,   175,   178,   175,    23,    23,    13,
      23,    27,    32,   370,   180,   181,   180,   180,   177,   200,
     364,   104,   189,   331,     4,   365,   180,    23,   331,   331,
     216,   331,     6,   177,   335,   177,   331,   282,   331,   282,
     331,   282,   282,   175,   358,   347,   175,   331,   165,   287,
     178,   178,   287,   287,   178,     6,   218,   331,     6,   218,
     260,   324,   326,   331,   149,   331,   104,   181,   189,   249,
     358,   219,     6,   179,   253,   179,   335,   220,   193,   205,
     209,   212,   213,   179,   225,   178,   373,   178,   344,   102,
     248,   179,   295,   344,   335,     5,    82,   105,   106,   177,
     195,   271,   272,   273,   274,   275,   277,   248,   191,   180,
       4,    32,   175,   331,   178,   178,   177,   358,   331,   246,
     178,   178,    51,   331,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,   199,   265,   266,   267,   268,   269,
     307,   308,   177,   265,   180,   180,   180,   246,   216,   178,
     216,   176,   174,   331,   177,   364,   358,   152,   178,     6,
     218,   252,   176,   254,   176,   254,    66,   256,   257,   258,
     259,   331,    76,    77,   223,    62,   213,   174,   208,   210,
     213,   176,   295,   335,   292,   174,   179,   272,   272,   275,
     171,     7,     7,   171,   335,   180,   331,   175,   176,   364,
     248,   218,     6,   176,   269,   178,   174,   208,     5,   177,
     270,   276,   277,   278,   279,   280,   308,   265,   178,   248,
     176,    55,   325,   331,   364,   175,   249,     6,   218,   251,
     216,   254,    64,    65,    66,   254,   180,   174,   208,   180,
     174,   208,   174,   208,   177,   179,    23,   212,   180,   174,
     208,   179,    65,    79,    92,   180,   199,   244,   293,   294,
     304,   305,   306,   307,   344,   292,   178,   272,   272,   273,
     273,   272,   179,   180,   178,   335,   216,     6,   281,   267,
     277,   277,   280,   170,   228,   171,     7,     7,   171,   178,
      79,   339,   335,   176,   178,   178,   178,   216,    61,    64,
     176,   331,     6,   176,   180,   152,   258,   331,   152,   221,
     344,   216,   213,   180,   292,   335,   295,   293,   270,   334,
      73,   180,   292,   179,   271,   152,   178,   165,   229,   277,
     277,   278,   278,   277,   281,   177,   281,   179,     6,   218,
     250,   251,    59,   176,   176,     6,   176,   216,   216,   331,
     331,     7,    27,   222,   180,   180,   190,   176,   179,   296,
      27,   309,   310,   311,   338,    23,    82,   105,   106,   188,
     271,   319,   320,   180,   292,   336,    27,   336,    27,   189,
     340,   341,   336,   292,   216,   176,   216,   216,   344,   178,
       4,   245,    82,   180,   190,   297,   298,   299,   300,   301,
     302,   344,     4,   335,   174,   176,   190,     4,     4,    23,
     319,   174,   176,   180,   337,   335,   179,    27,   174,   208,
     179,   180,    57,   179,   331,   176,   180,   298,   176,   176,
      62,    80,   163,   331,   179,    27,   310,   335,   331,   331,
     176,   320,   331,     4,   179,   313,   216,   341,   178,   216,
     176,   216,    23,   187,   308,   295,   190,   335,   312,     4,
     335,   177,   335,   335,   336,   331,   312,   180,   180,   180,
     190,   179,   180,   199,   307,   314,   315,   331,   265,   313,
     180,   336,   336,   312,   315,   338,   335,   178,   180,    23,
     281,   335,   336,   316,   176,   179,   303,   177,   318,   216,
     336,   265,   336,   180,   178,   152,   176,   179,   317,   331,
     216,   336,   176,   180
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   185,   186,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   188,   188,   188,   188,   188,   188,
     188,   188,   189,   189,   190,   190,   191,   191,   192,   192,
     193,   193,   194,   194,   195,   195,   195,   195,   196,   196,
     197,   197,   198,   199,   199,   200,   200,   200,   200,   200,
     201,   201,   202,   202,   202,   202,   202,   203,   202,   204,
     202,   202,   202,   202,   202,   205,   205,   206,   207,   208,
     208,   209,   209,   210,   210,   211,   211,   212,   212,   213,
     213,   214,   214,   215,   215,   216,   216,   217,   217,   217,
     217,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   219,
     218,   218,   218,   218,   218,   218,   220,   220,   221,   221,
     222,   222,   223,   223,   224,   224,   225,   226,   226,   227,
     228,   228,   229,   229,   231,   230,   232,   230,   233,   233,
     234,   234,   235,   235,   236,   236,   236,   238,   237,   240,
     239,   242,   241,   243,   243,   244,   245,   245,   246,   246,
     247,   247,   248,   248,   249,   249,   249,   249,   250,   250,
     251,   251,   252,   252,   253,   253,   253,   253,   254,   254,
     254,   254,   254,   255,   256,   256,   257,   257,   258,   258,
     259,   259,   260,   260,   261,   261,   262,   262,   263,   263,
     264,   264,   265,   265,   266,   266,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   276,   276,
     277,   277,   277,   278,   278,   279,   279,   280,   280,   281,
     281,   282,   282,   282,   283,   283,   284,   284,   284,   284,
     285,   285,   285,   286,   286,   287,   287,   288,   288,   289,
     290,   290,   291,   291,   292,   292,   293,   293,   293,   293,
     293,   293,   294,   294,   294,   295,   295,   296,   296,   296,
     297,   297,   298,   298,   299,   300,   300,   300,   300,   301,
     301,   302,   303,   303,   304,   304,   305,   305,   306,   306,
     307,   307,   308,   308,   308,   308,   308,   308,   308,   308,
     308,   308,   309,   309,   310,   310,   311,   311,   312,   312,
     312,   313,   313,   314,   314,   316,   315,   317,   317,   317,
     318,   318,   319,   319,   320,   320,   321,   322,   322,   323,
     324,   324,   324,   325,   325,   326,   326,   326,   326,   328,
     327,   329,   329,   329,   330,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   332,   332,   333,   334,   335,   336,   337,   338,   338,
     339,   339,   340,   340,   341,   341,   342,   342,   342,   342,
     343,   342,   344,   344,   345,   345,   345,   346,   346,   346,
     347,   347,   348,   348,   348,   348,   349,   349,   349,   349,
     349,   349,   349,   349,   350,   350,   350,   350,   350,   350,
     350,   350,   350,   350,   351,   351,   351,   351,   352,   352,
     353,   354,   354,   354,   354,   354,   355,   355,   356,   356,
     356,   356,   357,   357,   357,   357,   357,   358,   358,   358,
     358,   359,   359,   359,   360,   360,   361,   361,   361,   361,
     361,   361,   362,   362,   362,   363,   363,   363,   364,   365,
     365,   366,   366,   367,   367,   367,   367,   367,   367,   367,
     368,   368,   368,   368,   369,   369,   369,   369,   369,   369,
     369,   369,   370,   370,   370,   370,   371,   371,   371,   371,
     371,   371,   371,   372,   372,   373
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
       1,     3,     1,     1,     2,     4,     3,     0,     6,     0,
       5,     3,     4,     3,     4,     1,     1,     6,     6,     0,
       1,     3,     1,     3,     1,     3,     1,     1,     2,     1,
       3,     1,     3,     3,     1,     2,     0,     1,     1,     2,
       4,     3,     1,     1,     5,     7,     9,     5,     3,     3,
       3,     3,     3,     3,     1,     2,     6,     7,     9,     0,
       6,     1,     6,     3,     2,     3,     0,     9,     1,     3,
       0,     1,     0,     4,     1,     3,     1,     1,     1,    13,
       0,     1,     0,     1,     0,    10,     0,     9,     1,     2,
       1,     2,     0,     1,     1,     1,     1,     0,     7,     0,
       8,     0,     9,     0,     2,     5,     0,     2,     0,     2,
       0,     2,     0,     2,     1,     2,     4,     3,     1,     4,
       1,     4,     1,     4,     3,     4,     4,     5,     0,     5,
       5,     4,     4,     7,     0,     2,     1,     3,     4,     4,
       1,     3,     1,     4,     5,     6,     1,     3,     6,     7,
       3,     6,     2,     0,     1,     3,     2,     1,     0,     1,
       7,     9,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     0,
       2,     2,     4,     3,     1,     3,     2,     4,     4,     3,
       3,     1,     3,     3,     2,     1,     1,     3,     1,     1,
       3,     1,     1,     3,     2,     0,     4,     3,     4,     5,
      12,     1,     1,     2,     3,     1,     3,     1,     2,     3,
       1,     2,     2,     2,     3,     3,     3,     4,     3,     1,
       1,     3,     1,     3,     1,     1,     0,     1,     0,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     4,     5,     7,     0,     2,
       3,     0,     3,     0,     1,     0,     9,     1,     3,     3,
       0,     3,     3,     1,     4,     4,     4,     3,     1,     1,
       0,     3,     1,     0,     1,     3,     4,     2,     1,     0,
      10,     3,     2,     3,     2,     1,     6,     5,     3,     4,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     3,
       2,     1,     2,     4,     2,     2,     1,     2,     2,     3,
       1,    13,    12,     1,     1,     0,     0,     0,     0,     1,
       0,     5,     3,     1,     1,     2,     2,     2,     4,     4,
       0,     3,     1,     1,     1,     1,     3,     0,     1,     1,
       0,     1,     4,     3,     1,     3,     1,     1,     3,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     5,     5,     0,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     4,     4,     4,     1,     1,     1,     3,
       3,     1,     4,     2,     3,     3,     1,     4,     3,     3,
       3,     3,     1,     3,     1,     1,     3,     1,     1,     0,
       1,     3,     1,     3,     1,     4,     2,     2,     6,     4,
       2,     2,     1,     2,     1,     4,     3,     3,     3,     3,
       6,     3,     1,     1,     2,     1,     5,     4,     2,     2,
       4,     2,     2,     1,     3,     1
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

    case YYSYMBOL_attributed_top_statement: /* attributed_top_statement  */
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

    case YYSYMBOL_clone_argument_list: /* clone_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_clone_argument_list: /* non_empty_clone_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument_no_expr: /* argument_no_expr  */
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

    case YYSYMBOL_for_cond_exprs: /* for_cond_exprs  */
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

  case 110: /* attributed_top_statement: attributed_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 111: /* attributed_top_statement: "'const'" const_list ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 112: /* top_statement: statement  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 113: /* top_statement: attributed_top_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 114: /* top_statement: attributes attributed_top_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 115: /* top_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }
    break;

  case 116: /* top_statement: "'namespace'" namespace_declaration_name ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }
    break;

  case 117: /* $@1: %empty  */
                                                       { RESET_DOC_COMMENT(); }
    break;

  case 118: /* top_statement: "'namespace'" namespace_declaration_name $@1 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 119: /* $@2: %empty  */
                            { RESET_DOC_COMMENT(); }
    break;

  case 120: /* top_statement: "'namespace'" $@2 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }
    break;

  case 121: /* top_statement: "'use'" mixed_group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 122: /* top_statement: "'use'" use_type group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 123: /* top_statement: "'use'" use_declarations ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 124: /* top_statement: "'use'" use_type use_declarations ';'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 125: /* use_type: "'function'"  */
                                        { (yyval.num) = ZEND_SYMBOL_FUNCTION; }
    break;

  case 126: /* use_type: "'const'"  */
                                        { (yyval.num) = ZEND_SYMBOL_CONST; }
    break;

  case 127: /* group_use_declaration: legacy_namespace_name "'\\'" '{' unprefixed_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 128: /* mixed_group_use_declaration: legacy_namespace_name "'\\'" '{' inline_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}
    break;

  case 131: /* inline_use_declarations: inline_use_declarations ',' inline_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 132: /* inline_use_declarations: inline_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 133: /* unprefixed_use_declarations: unprefixed_use_declarations ',' unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 134: /* unprefixed_use_declarations: unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 135: /* use_declarations: use_declarations ',' use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 136: /* use_declarations: use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 137: /* inline_use_declaration: unprefixed_use_declaration  */
                                           { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 138: /* inline_use_declaration: use_type unprefixed_use_declaration  */
                                                    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }
    break;

  case 139: /* unprefixed_use_declaration: namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 140: /* unprefixed_use_declaration: namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 141: /* use_declaration: legacy_namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 142: /* use_declaration: legacy_namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 143: /* const_list: const_list ',' const_decl  */
                                          { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 144: /* const_list: const_decl  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 145: /* inner_statement_list: inner_statement_list inner_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 146: /* inner_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 147: /* inner_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 148: /* inner_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 149: /* inner_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 150: /* inner_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }
    break;

  case 151: /* statement: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 152: /* statement: if_stmt  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 153: /* statement: alt_if_stmt  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 154: /* statement: "'while'" '(' expr ')' while_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 155: /* statement: "'do'" statement "'while'" '(' expr ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 156: /* statement: "'for'" '(' for_exprs ';' for_cond_exprs ';' for_exprs ')' for_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 157: /* statement: "'switch'" '(' expr ')' switch_case_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 158: /* statement: "'break'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }
    break;

  case 159: /* statement: "'continue'" optional_expr ';'  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }
    break;

  case 160: /* statement: "'return'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }
    break;

  case 161: /* statement: "'global'" global_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 162: /* statement: "'static'" static_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 163: /* statement: "'echo'" echo_expr_list ';'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 164: /* statement: T_INLINE_HTML  */
                              { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 165: /* statement: expr ';'  */
                         { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 166: /* statement: "'unset'" '(' unset_variables possible_comma ')' ';'  */
                                                                   { (yyval.ast) = (yyvsp[-3].ast); }
    break;

  case 167: /* statement: "'foreach'" '(' expr "'as'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 168: /* statement: "'foreach'" '(' expr "'as'" foreach_variable "'=>'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }
    break;

  case 169: /* $@3: %empty  */
                        { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }
    break;

  case 170: /* statement: "'declare'" '(' const_list ')' $@3 declare_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 171: /* statement: ';'  */
                                              { (yyval.ast) = NULL; }
    break;

  case 172: /* statement: "'try'" '{' inner_statement_list '}' catch_list finally_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 173: /* statement: "'goto'" "identifier" ';'  */
                                    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }
    break;

  case 174: /* statement: "identifier" ':'  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }
    break;

  case 175: /* statement: "'(void)'" expr ';'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[-1].ast)); }
    break;

  case 176: /* catch_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }
    break;

  case 177: /* catch_list: catch_list "'catch'" '(' catch_name_list optional_variable ')' '{' inner_statement_list '}'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }
    break;

  case 178: /* catch_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 179: /* catch_name_list: catch_name_list '|' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 180: /* optional_variable: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 181: /* optional_variable: "variable"  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 182: /* finally_statement: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 183: /* finally_statement: "'finally'" '{' inner_statement_list '}'  */
                                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 184: /* unset_variables: unset_variable  */
                               { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 185: /* unset_variables: unset_variables ',' unset_variable  */
                                                   { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 186: /* unset_variable: variable  */
                         { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }
    break;

  case 187: /* function_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 188: /* function_name: "'readonly'"  */
                           {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 189: /* function_declaration_statement: function returns_ref function_name backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 190: /* is_reference: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 191: /* is_reference: "'&'"  */
                                                        { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 192: /* is_variadic: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 193: /* is_variadic: "'...'"  */
                            { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 194: /* @4: %empty  */
                                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 195: /* class_declaration_statement: class_modifiers "'class'" @4 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 196: /* @5: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 197: /* class_declaration_statement: "'class'" @5 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 198: /* class_modifiers: class_modifier  */
                                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 199: /* class_modifiers: class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 200: /* anonymous_class_modifiers: class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier(0, (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 201: /* anonymous_class_modifiers: anonymous_class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 202: /* anonymous_class_modifiers_optional: %empty  */
                                                { (yyval.num) = 0; }
    break;

  case 203: /* anonymous_class_modifiers_optional: anonymous_class_modifiers  */
                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 204: /* class_modifier: "'abstract'"  */
                                        { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 205: /* class_modifier: "'final'"  */
                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 206: /* class_modifier: "'readonly'"  */
                                        { (yyval.num) = ZEND_ACC_READONLY_CLASS|ZEND_ACC_NO_DYNAMIC_PROPERTIES; }
    break;

  case 207: /* @6: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 208: /* trait_declaration_statement: "'trait'" @6 "identifier" backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 209: /* @7: %empty  */
                            { (yyval.num) = CG(zend_lineno); }
    break;

  case 210: /* interface_declaration_statement: "'interface'" @7 "identifier" interface_extends_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 211: /* @8: %empty  */
                       { (yyval.num) = CG(zend_lineno); }
    break;

  case 212: /* enum_declaration_statement: "'enum'" @8 "identifier" enum_backing_type implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }
    break;

  case 213: /* enum_backing_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 214: /* enum_backing_type: ':' type_expr  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 215: /* enum_case: "'case'" backup_doc_comment identifier enum_case_expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }
    break;

  case 216: /* enum_case_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 217: /* enum_case_expr: '=' expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 218: /* extends_from: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 219: /* extends_from: "'extends'" class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 220: /* interface_extends_list: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 221: /* interface_extends_list: "'extends'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 222: /* implements_list: %empty  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 223: /* implements_list: "'implements'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 224: /* foreach_variable: variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 225: /* foreach_variable: ampersand variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }
    break;

  case 226: /* foreach_variable: "'list'" '(' array_pair_list ')'  */
                                               { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 227: /* foreach_variable: '[' array_pair_list ']'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 228: /* for_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 229: /* for_statement: ':' inner_statement_list "'endfor'" ';'  */
                                                      { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 230: /* foreach_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 231: /* foreach_statement: ':' inner_statement_list "'endforeach'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 232: /* declare_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 233: /* declare_statement: ':' inner_statement_list "'enddeclare'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 234: /* switch_case_list: '{' case_list '}'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 235: /* switch_case_list: '{' ';' case_list '}'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 236: /* switch_case_list: ':' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 237: /* switch_case_list: ':' ';' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 238: /* case_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 239: /* case_list: case_list "'case'" expr ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 240: /* case_list: case_list "'case'" expr ';' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create_ex(ZEND_AST_SWITCH_CASE, ZEND_ALT_CASE_SYNTAX, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 241: /* case_list: case_list "'default'" ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }
    break;

  case 242: /* case_list: case_list "'default'" ';' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create_ex(ZEND_AST_SWITCH_CASE, ZEND_ALT_CASE_SYNTAX, NULL, (yyvsp[0].ast))); }
    break;

  case 243: /* match: "'match'" '(' expr ')' '{' match_arm_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 244: /* match_arm_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }
    break;

  case 245: /* match_arm_list: non_empty_match_arm_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 246: /* non_empty_match_arm_list: match_arm  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }
    break;

  case 247: /* non_empty_match_arm_list: non_empty_match_arm_list ',' match_arm  */
                                                       { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 248: /* match_arm: match_arm_cond_list possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 249: /* match_arm: "'default'" possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }
    break;

  case 250: /* match_arm_cond_list: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 251: /* match_arm_cond_list: match_arm_cond_list ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 252: /* while_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 253: /* while_statement: ':' inner_statement_list "'endwhile'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 254: /* if_stmt_without_else: "'if'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 255: /* if_stmt_without_else: if_stmt_without_else "'elseif'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 256: /* if_stmt: if_stmt_without_else  */
                                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 257: /* if_stmt: if_stmt_without_else "'else'" statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }
    break;

  case 258: /* alt_if_stmt_without_else: "'if'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 259: /* alt_if_stmt_without_else: alt_if_stmt_without_else "'elseif'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 260: /* alt_if_stmt: alt_if_stmt_without_else "'endif'" ';'  */
                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 261: /* alt_if_stmt: alt_if_stmt_without_else "'else'" ':' inner_statement_list "'endif'" ';'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }
    break;

  case 262: /* parameter_list: non_empty_parameter_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 263: /* parameter_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 264: /* non_empty_parameter_list: attributed_parameter  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }
    break;

  case 265: /* non_empty_parameter_list: non_empty_parameter_list ',' attributed_parameter  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 266: /* attributed_parameter: attributes parameter  */
                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 267: /* attributed_parameter: parameter  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 268: /* optional_cpp_modifiers: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 269: /* optional_cpp_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CPP, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 270: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-6].num) | (yyvsp[-4].num) | (yyvsp[-3].num), (yyvsp[-5].ast), (yyvsp[-2].ast), NULL,
					NULL, (yyvsp[-1].str) ? zend_ast_create_zval_from_str((yyvsp[-1].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 271: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment '=' expr optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-8].num) | (yyvsp[-6].num) | (yyvsp[-5].num), (yyvsp[-7].ast), (yyvsp[-4].ast), (yyvsp[-1].ast),
					NULL, (yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 272: /* optional_type_without_static: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 273: /* optional_type_without_static: type_expr_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 274: /* type_expr: type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 275: /* type_expr: '?' type  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 276: /* type_expr: union_type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 277: /* type_expr: intersection_type  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 278: /* type: type_without_static  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 279: /* type: "'static'"  */
                                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }
    break;

  case 280: /* union_type_element: type  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 281: /* union_type_element: '(' intersection_type ')'  */
                                           { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 282: /* union_type: union_type_element '|' union_type_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 283: /* union_type: union_type '|' union_type_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 284: /* intersection_type: type "amp" type  */
                                                                          { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 285: /* intersection_type: intersection_type "amp" type  */
                                                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 286: /* type_expr_without_static: type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 287: /* type_expr_without_static: '?' type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 288: /* type_expr_without_static: union_type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 289: /* type_expr_without_static: intersection_type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 290: /* type_without_static: "'array'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 291: /* type_without_static: "'callable'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 292: /* type_without_static: name  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 293: /* union_type_without_static_element: type_without_static  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 294: /* union_type_without_static_element: '(' intersection_type_without_static ')'  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 295: /* union_type_without_static: union_type_without_static_element '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 296: /* union_type_without_static: union_type_without_static '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 297: /* intersection_type_without_static: type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 298: /* intersection_type_without_static: intersection_type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 299: /* return_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 300: /* return_type: ':' type_expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 301: /* argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 302: /* argument_list: '(' non_empty_argument_list possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 303: /* argument_list: '(' "'...'" ')'  */
                                   { (yyval.ast) = zend_ast_create_fcc(); }
    break;

  case 304: /* non_empty_argument_list: argument  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 305: /* non_empty_argument_list: non_empty_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 306: /* clone_argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 307: /* clone_argument_list: '(' non_empty_clone_argument_list possible_comma ')'  */
                                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 308: /* clone_argument_list: '(' expr ',' ')'  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[-2].ast)); }
    break;

  case 309: /* clone_argument_list: '(' "'...'" ')'  */
                                   { (yyval.ast) = zend_ast_create_fcc(); }
    break;

  case 310: /* non_empty_clone_argument_list: expr ',' argument  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ARG_LIST, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 311: /* non_empty_clone_argument_list: argument_no_expr  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 312: /* non_empty_clone_argument_list: non_empty_clone_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 313: /* argument_no_expr: identifier ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 314: /* argument_no_expr: "'...'" expr  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 315: /* argument: expr  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 316: /* argument: argument_no_expr  */
                                 { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 317: /* global_var_list: global_var_list ',' global_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 318: /* global_var_list: global_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 319: /* global_var: simple_variable  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }
    break;

  case 320: /* static_var_list: static_var_list ',' static_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 321: /* static_var_list: static_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 322: /* static_var: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }
    break;

  case 323: /* static_var: "variable" '=' expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 324: /* class_statement_list: class_statement_list class_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 325: /* class_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 326: /* attributed_class_statement: property_modifiers optional_type_without_static property_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 327: /* attributed_class_statement: property_modifiers optional_type_without_static hooked_property  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-1].ast), zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)), NULL);
			  (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 328: /* attributed_class_statement: class_const_modifiers "'const'" class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 329: /* attributed_class_statement: class_const_modifiers "'const'" type_expr class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, (yyvsp[-2].ast));
			  (yyval.ast)->attr = (yyvsp[-4].num); }
    break;

  case 330: /* attributed_class_statement: method_modifiers function returns_ref identifier backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags method_body backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 331: /* attributed_class_statement: enum_case  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 332: /* class_statement: attributed_class_statement  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 333: /* class_statement: attributes attributed_class_statement  */
                                                      { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 334: /* class_statement: "'use'" class_name_list trait_adaptations  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 335: /* class_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 336: /* class_name_list: class_name_list ',' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 337: /* trait_adaptations: ';'  */
                                                                                { (yyval.ast) = NULL; }
    break;

  case 338: /* trait_adaptations: '{' '}'  */
                                                                        { (yyval.ast) = NULL; }
    break;

  case 339: /* trait_adaptations: '{' trait_adaptation_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 340: /* trait_adaptation_list: trait_adaptation  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }
    break;

  case 341: /* trait_adaptation_list: trait_adaptation_list trait_adaptation  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 342: /* trait_adaptation: trait_precedence ';'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 343: /* trait_adaptation: trait_alias ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 344: /* trait_precedence: absolute_trait_method_reference "'insteadof'" class_name_list  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 345: /* trait_alias: trait_method_reference "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 346: /* trait_alias: trait_method_reference "'as'" reserved_non_modifiers  */
                        { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }
    break;

  case 347: /* trait_alias: trait_method_reference "'as'" member_modifier identifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[-1].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-3].ast), (yyvsp[0].ast));
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 348: /* trait_alias: trait_method_reference "'as'" member_modifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-2].ast), NULL);
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 349: /* trait_method_reference: identifier  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }
    break;

  case 350: /* trait_method_reference: absolute_trait_method_reference  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 351: /* absolute_trait_method_reference: class_name "'::'" identifier  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 352: /* method_body: ';'  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 353: /* method_body: '{' inner_statement_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 354: /* property_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 355: /* property_modifiers: "'var'"  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 356: /* method_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 357: /* method_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 358: /* class_const_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 359: /* class_const_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CONSTANT, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 360: /* non_empty_member_modifiers: member_modifier  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MODIFIER_LIST, zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 361: /* non_empty_member_modifiers: non_empty_member_modifiers member_modifier  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 362: /* member_modifier: "'public'"  */
                                                        { (yyval.num) = T_PUBLIC; }
    break;

  case 363: /* member_modifier: "'protected'"  */
                                                        { (yyval.num) = T_PROTECTED; }
    break;

  case 364: /* member_modifier: "'private'"  */
                                                        { (yyval.num) = T_PRIVATE; }
    break;

  case 365: /* member_modifier: "'public(set)'"  */
                                                { (yyval.num) = T_PUBLIC_SET; }
    break;

  case 366: /* member_modifier: "'protected(set)'"  */
                                                { (yyval.num) = T_PROTECTED_SET; }
    break;

  case 367: /* member_modifier: "'private(set)'"  */
                                                { (yyval.num) = T_PRIVATE_SET; }
    break;

  case 368: /* member_modifier: "'static'"  */
                                                        { (yyval.num) = T_STATIC; }
    break;

  case 369: /* member_modifier: "'abstract'"  */
                                                        { (yyval.num) = T_ABSTRACT; }
    break;

  case 370: /* member_modifier: "'final'"  */
                                                        { (yyval.num) = T_FINAL; }
    break;

  case 371: /* member_modifier: "'readonly'"  */
                                                        { (yyval.num) = T_READONLY; }
    break;

  case 372: /* property_list: property_list ',' property  */
                                           { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 373: /* property_list: property  */
                         { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }
    break;

  case 374: /* property: "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 375: /* property: "variable" '=' expr backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 376: /* hooked_property: "variable" backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-4].ast), NULL, ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 377: /* hooked_property: "variable" '=' expr backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-6].ast), (yyvsp[-4].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 378: /* property_hook_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 379: /* property_hook_list: property_hook_list property_hook  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 380: /* property_hook_list: property_hook_list attributes property_hook  */
                                                            {
			(yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)));
		}
    break;

  case 381: /* optional_property_hook_list: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 382: /* optional_property_hook_list: '{' property_hook_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 383: /* property_hook_modifiers: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 384: /* property_hook_modifiers: non_empty_member_modifiers  */
                                           {
			(yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY_HOOK, (yyvsp[0].ast));
			if (!(yyval.num)) { YYERROR; }
		}
    break;

  case 385: /* @9: %empty  */
                                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 386: /* property_hook: property_hook_modifiers returns_ref "identifier" backup_doc_comment @9 optional_parameter_list backup_fn_flags property_hook_body backup_fn_flags  */
                                                                                           {
			(yyval.ast) = zend_ast_create_decl(
				ZEND_AST_PROPERTY_HOOK, (yyvsp[-8].num) | (yyvsp[-7].num) | (yyvsp[0].num), (yyvsp[-4].num), (yyvsp[-5].str), zend_ast_get_str((yyvsp[-6].ast)),
				(yyvsp[-3].ast), NULL, (yyvsp[-1].ast), NULL, NULL);
			CG(extra_fn_flags) = (yyvsp[-2].num);
		}
    break;

  case 387: /* property_hook_body: ';'  */
                    { (yyval.ast) = NULL; }
    break;

  case 388: /* property_hook_body: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 389: /* property_hook_body: "'=>'" expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROPERTY_HOOK_SHORT_BODY, (yyvsp[-1].ast)); }
    break;

  case 390: /* optional_parameter_list: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 391: /* optional_parameter_list: '(' parameter_list ')'  */
                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 392: /* class_const_list: class_const_list ',' class_const_decl  */
                                                      { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 393: /* class_const_list: class_const_decl  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 394: /* class_const_decl: "identifier" '=' expr backup_doc_comment  */
                                                     { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 395: /* class_const_decl: semi_reserved '=' expr backup_doc_comment  */
                                                          {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-3].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, zend_ast_create_zval(&zv), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL));
		}
    break;

  case 396: /* const_decl: "identifier" '=' expr backup_doc_comment  */
                                             { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 397: /* echo_expr_list: echo_expr_list ',' echo_expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 398: /* echo_expr_list: echo_expr  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 399: /* echo_expr: expr  */
             { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 400: /* for_cond_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 401: /* for_cond_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 402: /* for_cond_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 403: /* for_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 404: /* for_exprs: non_empty_for_exprs  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 405: /* non_empty_for_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 406: /* non_empty_for_exprs: non_empty_for_exprs ',' "'(void)'" expr  */
                                                         { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[0].ast))); }
    break;

  case 407: /* non_empty_for_exprs: "'(void)'" expr  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[0].ast))); }
    break;

  case 408: /* non_empty_for_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 409: /* @10: %empty  */
                                                           { (yyval.num) = CG(zend_lineno); }
    break;

  case 410: /* anonymous_class: anonymous_class_modifiers_optional "'class'" @10 ctor_arguments extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                                                                                             {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS | (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}
    break;

  case 411: /* new_dereferenceable: "'new'" class_name_reference argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 412: /* new_dereferenceable: "'new'" anonymous_class  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 413: /* new_dereferenceable: "'new'" attributes anonymous_class  */
                        { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 414: /* new_non_dereferenceable: "'new'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[0].ast), zend_ast_create_list(0, ZEND_AST_ARG_LIST)); }
    break;

  case 415: /* expr: variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 416: /* expr: "'list'" '(' array_pair_list ')' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 417: /* expr: '[' array_pair_list ']' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 418: /* expr: variable '=' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 419: /* expr: variable '=' ampersand variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 420: /* expr: "'clone'" clone_argument_list  */
                                            {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_CLONE));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, (yyvsp[0].ast));
		}
    break;

  case 421: /* expr: "'clone'" expr  */
                             {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_CLONE));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)));
		}
    break;

  case 422: /* expr: variable "'+='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 423: /* expr: variable "'-='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 424: /* expr: variable "'*='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 425: /* expr: variable "'**='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 426: /* expr: variable "'/='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 427: /* expr: variable "'.='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 428: /* expr: variable "'%='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 429: /* expr: variable "'&='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 430: /* expr: variable "'|='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 431: /* expr: variable "'^='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 432: /* expr: variable "'<<='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 433: /* expr: variable "'>>='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 434: /* expr: variable "'??='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 435: /* expr: variable "'++'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }
    break;

  case 436: /* expr: "'++'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }
    break;

  case 437: /* expr: variable "'--'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }
    break;

  case 438: /* expr: "'--'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }
    break;

  case 439: /* expr: expr "'||'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 440: /* expr: expr "'&&'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 441: /* expr: expr "'or'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 442: /* expr: expr "'and'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 443: /* expr: expr "'xor'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 444: /* expr: expr '|' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 445: /* expr: expr "amp" expr  */
                                                                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 446: /* expr: expr "'&'" expr  */
                                                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 447: /* expr: expr '^' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 448: /* expr: expr '.' expr  */
                                { (yyval.ast) = zend_ast_create_concat_op((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 449: /* expr: expr '+' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 450: /* expr: expr '-' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 451: /* expr: expr '*' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 452: /* expr: expr "'**'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 453: /* expr: expr '/' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 454: /* expr: expr '%' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 455: /* expr: expr "'<<'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 456: /* expr: expr "'>>'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 457: /* expr: '+' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }
    break;

  case 458: /* expr: '-' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }
    break;

  case 459: /* expr: '!' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }
    break;

  case 460: /* expr: '~' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }
    break;

  case 461: /* expr: expr "'==='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 462: /* expr: expr "'!=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 463: /* expr: expr "'=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 464: /* expr: expr "'!='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 465: /* expr: expr "'|>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PIPE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 466: /* expr: expr '<' expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 467: /* expr: expr "'<='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 468: /* expr: expr '>' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 469: /* expr: expr "'>='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 470: /* expr: expr "'<=>'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 471: /* expr: expr "'instanceof'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 472: /* expr: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
			if ((yyval.ast)->kind == ZEND_AST_ARROW_FUNC) (yyval.ast)->attr = ZEND_PARENTHESIZED_ARROW_FUNC;
		}
    break;

  case 473: /* expr: new_dereferenceable  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 474: /* expr: new_non_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 475: /* expr: expr '?' expr ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 476: /* expr: expr '?' ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 477: /* expr: expr "'??'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 478: /* expr: internal_functions_in_yacc  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 479: /* expr: "'(int)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }
    break;

  case 480: /* expr: "'(float)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }
    break;

  case 481: /* expr: "'(string)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }
    break;

  case 482: /* expr: "'(array)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }
    break;

  case 483: /* expr: "'(object)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }
    break;

  case 484: /* expr: "'(bool)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }
    break;

  case 485: /* expr: "'(unset)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }
    break;

  case 486: /* expr: "'exit'" ctor_arguments  */
                                      {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_EXIT));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, (yyvsp[0].ast));
		}
    break;

  case 487: /* expr: '@' expr  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }
    break;

  case 488: /* expr: scalar  */
                       { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 489: /* expr: '`' backticks_expr '`'  */
                                       { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }
    break;

  case 490: /* expr: "'print'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }
    break;

  case 491: /* expr: "'yield'"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 492: /* expr: "'yield'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 493: /* expr: "'yield'" expr "'=>'" expr  */
                                                 { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 494: /* expr: "'yield from'" expr  */
                                  { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 495: /* expr: "'throw'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }
    break;

  case 496: /* expr: inline_function  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 497: /* expr: attributes inline_function  */
                                           { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 498: /* expr: "'static'" inline_function  */
                                         { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 499: /* expr: attributes "'static'" inline_function  */
                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 500: /* expr: match  */
                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 501: /* inline_function: function returns_ref backup_doc_comment '(' parameter_list ')' lexical_vars return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  NULL,
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 502: /* inline_function: fn returns_ref backup_doc_comment '(' parameter_list ')' return_type "'=>'" backup_fn_flags backup_lex_pos expr backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  NULL, (yyvsp[-7].ast), NULL, (yyvsp[-1].ast), (yyvsp[-5].ast), NULL);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }
    break;

  case 503: /* fn: "'fn'"  */
             { (yyval.num) = CG(zend_lineno); }
    break;

  case 504: /* function: "'function'"  */
                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 505: /* backup_doc_comment: %empty  */
               { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 506: /* backup_fn_flags: %empty  */
                                         { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 507: /* backup_lex_pos: %empty  */
               { (yyval.ptr) = LANG_SCNG(yy_text); }
    break;

  case 508: /* returns_ref: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 509: /* returns_ref: ampersand  */
                                { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 510: /* lexical_vars: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 511: /* lexical_vars: "'use'" '(' lexical_var_list possible_comma ')'  */
                                                              { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 512: /* lexical_var_list: lexical_var_list ',' lexical_var  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 513: /* lexical_var_list: lexical_var  */
                            { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }
    break;

  case 514: /* lexical_var: "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 515: /* lexical_var: ampersand "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }
    break;

  case 516: /* function_call: name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 517: /* function_call: "'readonly'" argument_list  */
                                         {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-1].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, zend_ast_create_zval(&zv), (yyvsp[0].ast));
		}
    break;

  case 518: /* function_call: class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 519: /* function_call: variable_class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 520: /* @11: %empty  */
                              { (yyval.num) = CG(zend_lineno); }
    break;

  case 521: /* function_call: callable_expr @11 argument_list  */
                                                                           {
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-2].ast), (yyvsp[0].ast));
			(yyval.ast)->lineno = (yyvsp[-1].num);
		}
    break;

  case 522: /* class_name: "'static'"  */
                        { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 523: /* class_name: name  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 524: /* class_name_reference: class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 525: /* class_name_reference: new_variable  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 526: /* class_name_reference: '(' expr ')'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 527: /* backticks_expr: %empty  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 528: /* backticks_expr: "string content"  */
                                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 529: /* backticks_expr: encaps_list  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 530: /* ctor_arguments: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 531: /* ctor_arguments: argument_list  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 532: /* dereferenceable_scalar: "'array'" '(' array_pair_list ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 533: /* dereferenceable_scalar: '[' array_pair_list ']'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 534: /* dereferenceable_scalar: "quoted string"  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 535: /* dereferenceable_scalar: '"' encaps_list '"'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 536: /* scalar: "integer"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 537: /* scalar: "floating-point number"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 538: /* scalar: "heredoc start" "string content" "heredoc end"  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 539: /* scalar: "heredoc start" "heredoc end"  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 540: /* scalar: "heredoc start" encaps_list "heredoc end"  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 541: /* scalar: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 542: /* scalar: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 543: /* scalar: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 544: /* constant: name  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }
    break;

  case 545: /* constant: "'__LINE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 546: /* constant: "'__FILE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 547: /* constant: "'__DIR__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 548: /* constant: "'__TRAIT__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 549: /* constant: "'__METHOD__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 550: /* constant: "'__FUNCTION__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 551: /* constant: "'__PROPERTY__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_PROPERTY_C); }
    break;

  case 552: /* constant: "'__NAMESPACE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 553: /* constant: "'__CLASS__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 554: /* class_constant: class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 555: /* class_constant: variable_class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 556: /* class_constant: class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 557: /* class_constant: variable_class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 558: /* optional_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 559: /* optional_expr: expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 560: /* variable_class_name: fully_dereferenceable  */
                                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 561: /* fully_dereferenceable: variable  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 562: /* fully_dereferenceable: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_STATIC_PROP) (yyval.ast)->attr = ZEND_PARENTHESIZED_STATIC_PROP;
		}
    break;

  case 563: /* fully_dereferenceable: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 564: /* fully_dereferenceable: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 565: /* fully_dereferenceable: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 566: /* array_object_dereferenceable: fully_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 567: /* array_object_dereferenceable: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 568: /* callable_expr: callable_variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 569: /* callable_expr: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 570: /* callable_expr: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 571: /* callable_expr: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 572: /* callable_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 573: /* callable_variable: array_object_dereferenceable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 574: /* callable_variable: array_object_dereferenceable "'->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 575: /* callable_variable: array_object_dereferenceable "'?->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 576: /* callable_variable: function_call  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 577: /* variable: callable_variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 578: /* variable: static_member  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 579: /* variable: array_object_dereferenceable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 580: /* variable: array_object_dereferenceable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 581: /* simple_variable: "variable"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 582: /* simple_variable: '$' '{' expr '}'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 583: /* simple_variable: '$' simple_variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 584: /* static_member: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 585: /* static_member: variable_class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 586: /* new_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 587: /* new_variable: new_variable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 588: /* new_variable: new_variable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 589: /* new_variable: new_variable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 590: /* new_variable: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 591: /* new_variable: new_variable "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 592: /* member_name: identifier  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 593: /* member_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 594: /* member_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 595: /* property_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 596: /* property_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 597: /* property_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 598: /* array_pair_list: non_empty_array_pair_list  */
                        { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }
    break;

  case 599: /* possible_array_pair: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 600: /* possible_array_pair: array_pair  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 601: /* non_empty_array_pair_list: non_empty_array_pair_list ',' possible_array_pair  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 602: /* non_empty_array_pair_list: possible_array_pair  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }
    break;

  case 603: /* array_pair: expr "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }
    break;

  case 604: /* array_pair: expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 605: /* array_pair: expr "'=>'" ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }
    break;

  case 606: /* array_pair: ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }
    break;

  case 607: /* array_pair: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 608: /* array_pair: expr "'=>'" "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }
    break;

  case 609: /* array_pair: "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }
    break;

  case 610: /* encaps_list: encaps_list encaps_var  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 611: /* encaps_list: encaps_list "string content"  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 612: /* encaps_list: encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }
    break;

  case 613: /* encaps_list: "string content" encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 614: /* encaps_var: "variable"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 615: /* encaps_var: "variable" '[' encaps_var_offset ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }
    break;

  case 616: /* encaps_var: "variable" "'->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 617: /* encaps_var: "variable" "'?->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 618: /* encaps_var: "'${'" expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR, (yyvsp[-1].ast)); }
    break;

  case 619: /* encaps_var: "'${'" "variable name" '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY, (yyvsp[-1].ast)); }
    break;

  case 620: /* encaps_var: "'${'" "variable name" '[' expr ']' '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_ENCAPS_VAR_DOLLAR_CURLY,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }
    break;

  case 621: /* encaps_var: "'{$'" variable '}'  */
                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 622: /* encaps_var_offset: "identifier"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 623: /* encaps_var_offset: "number"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 624: /* encaps_var_offset: '-' "number"  */
                                        { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }
    break;

  case 625: /* encaps_var_offset: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 626: /* internal_functions_in_yacc: "'isset'" '(' isset_variables possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 627: /* internal_functions_in_yacc: "'empty'" '(' expr ')'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }
    break;

  case 628: /* internal_functions_in_yacc: "'include'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }
    break;

  case 629: /* internal_functions_in_yacc: "'include_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }
    break;

  case 630: /* internal_functions_in_yacc: "'eval'" '(' expr ')'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }
    break;

  case 631: /* internal_functions_in_yacc: "'require'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }
    break;

  case 632: /* internal_functions_in_yacc: "'require_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }
    break;

  case 633: /* isset_variables: isset_variable  */
                               { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 634: /* isset_variables: isset_variables ',' isset_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 635: /* isset_variable: expr  */
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

	/* We used "amp" as a dummy label to avoid a duplicate token literal warning. */
	if (strcmp(toktype, "\"amp\"") == 0) {
		if (yyres) {
			yystpcpy(yyres, "token \"&\"");
		}
		return sizeof("token \"&\"")-1;
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

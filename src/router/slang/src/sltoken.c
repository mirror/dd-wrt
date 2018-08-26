/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#define MAX_FILE_LINE_LEN (SL_MAX_TOKEN_LEN + 2)

/* int _pSLang_Compile_Line_Num_Info; */
#if SLANG_HAS_BOSEOS
int _pSLang_Compile_BOSEOS;
int _pSLang_Compile_BOFEOF;
#endif
#if SLANG_HAS_DEBUG_CODE
/* static int Default_Compile_Line_Num_Info; */
#if 0
static int Default_Compile_BOSEOS;
#endif
#endif

static char Empty_Line[1] = {0};

static char *Input_Line = Empty_Line;
static char *Input_Line_Pointer;

static SLprep_Type *This_SLpp;

static SLang_Load_Type *LLT;

static int next_input_line (void)
{
   LLT->line_num++;
   Input_Line_Pointer = Input_Line = LLT->read(LLT);
   if ((NULL == Input_Line) || _pSLang_Error)
     {
	Input_Line_Pointer = Input_Line = NULL;
	return -1;
     }
   return 0;
}

static void free_slstring_token_val (_pSLang_Token_Type *tok)
{
   char *s = tok->v.s_val;
   if (s != NULL)
     {
	_pSLfree_hashed_string (s, strlen(s), tok->hash);
	tok->v.s_val = NULL;
     }
}
static void free_static_sval_token (_pSLang_Token_Type *tok)
{
   tok->v.s_val = NULL;
}

_pSLtok_Type _pSLtoken_init_slstring_token (_pSLang_Token_Type *tok, _pSLtok_Type type,
					    char *s, unsigned int len)
{
   if (NULL == (tok->v.s_val = _pSLstring_make_hashed_string (s, len, &tok->hash)))
     return tok->type = EOF_TOKEN;

   tok->free_val_func = free_slstring_token_val;
   return tok->type = type;
}

static void free_bstring_token_val (_pSLang_Token_Type *tok)
{
   if (tok->v.b_val != NULL)
     {
	SLbstring_free (tok->v.b_val);
	tok->v.b_val = NULL;
     }
}

static _pSLtok_Type init_bstring_token (_pSLang_Token_Type *tok,
					unsigned char *s, unsigned int len)
{
   if (NULL == (tok->v.b_val = SLbstring_create (s, len)))
     return tok->type = EOF_TOKEN;

   tok->free_val_func = free_bstring_token_val;
   return tok->type = BSTRING_TOKEN;
}

/* In this table, if a single character can represent an operator, e.g.,
 * '&' (BAND_TOKEN), then it must be placed before multiple-character
 * operators that begin with the same character, e.g., "&=".  See
 * get_op_token to see how this is exploited.
 */
#define NUM_OPERATOR_TABLE_ROWS 31
typedef struct
{
   char opstring[4];
   _pSLtok_Type type;
}
Operator_Table_Entry_Type;
static SLCONST Operator_Table_Entry_Type Operators[NUM_OPERATOR_TABLE_ROWS] =
{
#define OFS_EXCL	0
   {"!=",	NE_TOKEN},
#define OFS_POUND	1
   {"#",	POUND_TOKEN},
#define OFS_BAND	2
   {"&",	BAND_TOKEN},
   {"&&",	SC_AND_TOKEN},
   {"&=",	BANDEQS_TOKEN},
#define OFS_STAR	5
   {"*",	TIMES_TOKEN},
   {"*=",	TIMESEQS_TOKEN},
#define OFS_PLUS	7
   {"+",	ADD_TOKEN},
   {"++",	PLUSPLUS_TOKEN},
   {"+=",	PLUSEQS_TOKEN},
#define OFS_MINUS	10
   {"-",	SUB_TOKEN},
   {"--",	MINUSMINUS_TOKEN},
   {"-=",	MINUSEQS_TOKEN},
   {"->",	NAMESPACE_TOKEN},
#define OFS_DIV		14
   {"/",	DIV_TOKEN},
   {"/=",	DIVEQS_TOKEN},
#define OFS_LT		16
   {"<",	LT_TOKEN},
   {"<<",	SHL_TOKEN},
   {"<=",	LE_TOKEN},
#define OFS_EQS		19
   {"=",	ASSIGN_TOKEN},
   {"==",	EQ_TOKEN},
#define OFS_GT		21
   {">",	GT_TOKEN},
   {">=",	GE_TOKEN},
   {">>",	SHR_TOKEN},
#define OFS_AT		24
   {"@",	DEREF_TOKEN},
#define OFS_POW		25
   {"^",	POW_TOKEN},
#define OFS_BOR		26
   {"|",	BOR_TOKEN},
   {"||",	SC_OR_TOKEN},
   {"|=",	BOREQS_TOKEN},
#define OFS_BNOT	29
   {"~",	BNOT_TOKEN},
   {"",	EOF_TOKEN}
};

static SLCONST char *lookup_op_token_string (_pSLtok_Type type)
{
   SLCONST Operator_Table_Entry_Type *op, *opmax;

   op = Operators;
   opmax = op + NUM_OPERATOR_TABLE_ROWS;

   while (op < opmax)
     {
	if (op->type == type)
	  return op->opstring;
	op++;
     }
   return NULL;
}

static SLCONST char *map_token_to_string (_pSLang_Token_Type *tok)
{
   SLCONST char *s;
   static char numbuf [32];
   _pSLtok_Type type;
   s = NULL;

   if (tok != NULL) type = tok->type;
   else type = 0;

   switch (type)
     {
      case 0:
	s = "??";
	break;

      case EOF_TOKEN:
	s = "End of input";
	break;

      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	sprintf (numbuf, "%ld", tok->v.long_val);
	s = numbuf;
	break;

      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
	sprintf (numbuf, "%lu", (unsigned long)tok->v.long_val);
	s = numbuf;
	break;

#ifdef HAVE_LONG_LONG
      case LLONG_TOKEN:
	sprintf (numbuf, "%lld", tok->v.llong_val);
	s = numbuf;
	break;

      case ULLONG_TOKEN:
	sprintf (numbuf, "%llu", tok->v.ullong_val);
	s = numbuf;
	break;
#endif

      case OBRACKET_TOKEN: s = "["; break;
      case CBRACKET_TOKEN: s = "]"; break;
      case OPAREN_TOKEN: s = "("; break;
      case CPAREN_TOKEN: s = ")"; break;
      case OBRACE_TOKEN: s = "{"; break;
      case CBRACE_TOKEN: s = "}"; break;
      case AND_TOKEN: s = "and"; break;
      case OR_TOKEN: s = "or"; break;
      case MOD_TOKEN: s = "mod"; break;
      case SHL_TOKEN: s = "shl"; break;
      case SHR_TOKEN: s = "shr"; break;
      case BXOR_TOKEN: s = "xor"; break;
      case COMMA_TOKEN: s = ","; break;
      case SEMICOLON_TOKEN: s = ";"; break;
      case COLON_TOKEN: s = ":"; break;
      case QUESTION_TOKEN: s = "?"; break;

      case ARRAY_TOKEN: s = "["; break;
      case DOT_TOKEN: s = "."; break;

      case MULTI_STRING_TOKEN:
	if (tok->v.multistring_val != NULL)
	  {
	     _pSLang_Multiline_String_Type *m = tok->v.multistring_val;
	     if ((m->type == STRING_TOKEN) || (m->type == STRING_DOLLAR_TOKEN))
	       s = m->v.s_val;
	     else
	       s = "<binary string>";
	  }
	break;

      case _BSTRING_TOKEN:
      case BSTRING_TOKEN:
      case ESC_BSTRING_TOKEN:
	s = "<binary string>";
	break;

#if SLANG_HAS_FLOAT
      case FLOAT_TOKEN:
      case DOUBLE_TOKEN:
      case COMPLEX_TOKEN:
	/* drop */
#endif
      default:
	  if (NULL != (s = lookup_op_token_string (type)))
	    break;
	if (((tok->free_val_func == free_slstring_token_val)
	     || (tok->free_val_func == free_static_sval_token))
	    && (tok->num_refs != 0))
	  s = tok->v.s_val;
	break;
     }

   if (s == NULL)
     {
	sprintf (numbuf, "(0x%02X)", type);
	s = numbuf;
     }

   return s;
}

void _pSLparse_error (int errcode, SLCONST char *str, _pSLang_Token_Type *tok, int flag)
{
   int line = LLT->line_num;
   SLFUTURE_CONST char *file = (char *) LLT->name;

   if (str == NULL)
     str = "Parse Error";

#if SLANG_HAS_DEBUG_CODE
   if ((tok != NULL) && (tok->line_number != -1))
     line = tok->line_number;
#endif
   if (file == NULL) file = "??";

   if (flag || (_pSLang_Error == 0))
     _pSLang_verror (errcode, "%s:%d: %s: found '%s'",
		   file, line, str, map_token_to_string (tok));

   (void) _pSLerr_set_line_info (file, line, NULL);
}

#define ALPHA_CHAR 	1
#define DIGIT_CHAR	2
#define EXCL_CHAR 	3
#define SEP_CHAR	4
#define OP_CHAR		5
#define DOT_CHAR	6
#define BOLDOT_CHAR	7
#define DQUOTE_CHAR	8
#define QUOTE_CHAR	9
#define COMMENT_CHAR	10
#define NL_CHAR		11
#define BAD_CHAR	12
#define WHITE_CHAR	13
#define BQUOTE_CHAR	15

#define CHAR_EOF	255

#define CHAR_CLASS(c)	(Char_Type_Table[(c)][0])
#define CHAR_DATA(c)	(Char_Type_Table[(c)][1])

static SLCONST unsigned char Char_Type_Table[256][2] =
{
 { NL_CHAR, 0 },	/* 0x0 */   { BAD_CHAR, 0 },	/* 0x1 */
 { BAD_CHAR, 0 },	/* 0x2 */   { BAD_CHAR, 0 },	/* 0x3 */
 { BAD_CHAR, 0 },	/* 0x4 */   { BAD_CHAR, 0 },	/* 0x5 */
 { BAD_CHAR, 0 },	/* 0x6 */   { BAD_CHAR, 0 },	/* 0x7 */
 { WHITE_CHAR, 0 },	/* 0x8 */   { WHITE_CHAR, 0 },	/* 0x9 */
 { NL_CHAR, 0 },	/* \n */   { WHITE_CHAR, 0 },	/* 0xb */
 { WHITE_CHAR, 0 },	/* 0xc */   { WHITE_CHAR, 0 },	/* \r */
 { BAD_CHAR, 0 },	/* 0xe */   { BAD_CHAR, 0 },	/* 0xf */
 { BAD_CHAR, 0 },	/* 0x10 */  { BAD_CHAR, 0 },	/* 0x11 */
 { BAD_CHAR, 0 },	/* 0x12 */  { BAD_CHAR, 0 },	/* 0x13 */
 { BAD_CHAR, 0 },	/* 0x14 */  { BAD_CHAR, 0 },	/* 0x15 */
 { BAD_CHAR, 0 },	/* 0x16 */  { BAD_CHAR, 0 },	/* 0x17 */
 { BAD_CHAR, 0 },	/* 0x18 */  { BAD_CHAR, 0 },	/* 0x19 */
 { BAD_CHAR, 0 },	/* 0x1a */  { BAD_CHAR, 0 },	/* 0x1b */
 { BAD_CHAR, 0 },	/* 0x1c */  { BAD_CHAR, 0 },	/* 0x1d */
 { BAD_CHAR, 0 },	/* 0x1e */  { BAD_CHAR, 0 },	/* 0x1f */
 { WHITE_CHAR, 0 },	/* 0x20 */  { EXCL_CHAR, OFS_EXCL },	/* ! */
 { DQUOTE_CHAR, 0 },	/* " */	    { OP_CHAR, OFS_POUND },	/* # */
 { ALPHA_CHAR, 0 },	/* $ */	    { NL_CHAR, 0 },/* % */
 { OP_CHAR, OFS_BAND },	/* & */	    { QUOTE_CHAR, 0 },	/* ' */
 { SEP_CHAR, OPAREN_TOKEN },	/* ( */	    { SEP_CHAR, CPAREN_TOKEN },	/* ) */
 { OP_CHAR, OFS_STAR },	/* * */	    { OP_CHAR, OFS_PLUS},	/* + */
 { SEP_CHAR, COMMA_TOKEN },	/* , */	    { OP_CHAR, OFS_MINUS },	/* - */
 { DOT_CHAR, 0 },	/* . */	    { OP_CHAR, OFS_DIV },	/* / */
 { DIGIT_CHAR, 0 },	/* 0 */	    { DIGIT_CHAR, 0 },	/* 1 */
 { DIGIT_CHAR, 0 },	/* 2 */	    { DIGIT_CHAR, 0 },	/* 3 */
 { DIGIT_CHAR, 0 },	/* 4 */	    { DIGIT_CHAR, 0 },	/* 5 */
 { DIGIT_CHAR, 0 },	/* 6 */	    { DIGIT_CHAR, 0 },	/* 7 */
 { DIGIT_CHAR, 0 },	/* 8 */	    { DIGIT_CHAR, 0 },	/* 9 */
 { SEP_CHAR, COLON_TOKEN },	/* : */	    { SEP_CHAR, SEMICOLON_TOKEN },	/* ; */
 { OP_CHAR, OFS_LT },	/* < */	    { OP_CHAR, OFS_EQS },	/* = */
 { OP_CHAR, OFS_GT },	/* > */	    { SEP_CHAR, QUESTION_TOKEN},	/* ? */
 { OP_CHAR, OFS_AT},	/* @ */	    { ALPHA_CHAR, 0 },	/* A */
 { ALPHA_CHAR, 0 },	/* B */	    { ALPHA_CHAR, 0 },	/* C */
 { ALPHA_CHAR, 0 },	/* D */	    { ALPHA_CHAR, 0 },	/* E */
 { ALPHA_CHAR, 0 },	/* F */	    { ALPHA_CHAR, 0 },	/* G */
 { ALPHA_CHAR, 0 },	/* H */	    { ALPHA_CHAR, 0 },	/* I */
 { ALPHA_CHAR, 0 },	/* J */	    { ALPHA_CHAR, 0 },	/* K */
 { ALPHA_CHAR, 0 },	/* L */	    { ALPHA_CHAR, 0 },	/* M */
 { ALPHA_CHAR, 0 },	/* N */	    { ALPHA_CHAR, 0 },	/* O */
 { ALPHA_CHAR, 0 },	/* P */	    { ALPHA_CHAR, 0 },	/* Q */
 { ALPHA_CHAR, 0 },	/* R */	    { ALPHA_CHAR, 0 },	/* S */
 { ALPHA_CHAR, 0 },	/* T */	    { ALPHA_CHAR, 0 },	/* U */
 { ALPHA_CHAR, 0 },	/* V */	    { ALPHA_CHAR, 0 },	/* W */
 { ALPHA_CHAR, 0 },	/* X */	    { ALPHA_CHAR, 0 },	/* Y */
 { ALPHA_CHAR, 0 },	/* Z */	    { SEP_CHAR, OBRACKET_TOKEN },	/* [ */
 { BAD_CHAR, 0 },	/* \ */	    { SEP_CHAR, CBRACKET_TOKEN },	/* ] */
 { OP_CHAR, OFS_POW },	/* ^ */	    { ALPHA_CHAR, 0 },	/* _ */
 { BQUOTE_CHAR, 0 },	/* ` */	    { ALPHA_CHAR, 0 },	/* a */
 { ALPHA_CHAR, 0 },	/* b */	    { ALPHA_CHAR, 0 },	/* c */
 { ALPHA_CHAR, 0 },	/* d */	    { ALPHA_CHAR, 0 },	/* e */
 { ALPHA_CHAR, 0 },	/* f */	    { ALPHA_CHAR, 0 },	/* g */
 { ALPHA_CHAR, 0 },	/* h */	    { ALPHA_CHAR, 0 },	/* i */
 { ALPHA_CHAR, 0 },	/* j */	    { ALPHA_CHAR, 0 },	/* k */
 { ALPHA_CHAR, 0 },	/* l */	    { ALPHA_CHAR, 0 },	/* m */
 { ALPHA_CHAR, 0 },	/* n */	    { ALPHA_CHAR, 0 },	/* o */
 { ALPHA_CHAR, 0 },	/* p */	    { ALPHA_CHAR, 0 },	/* q */
 { ALPHA_CHAR, 0 },	/* r */	    { ALPHA_CHAR, 0 },	/* s */
 { ALPHA_CHAR, 0 },	/* t */	    { ALPHA_CHAR, 0 },	/* u */
 { ALPHA_CHAR, 0 },	/* v */	    { ALPHA_CHAR, 0 },	/* w */
 { ALPHA_CHAR, 0 },	/* x */	    { ALPHA_CHAR, 0 },	/* y */
 { ALPHA_CHAR, 0 },	/* z */	    { SEP_CHAR, OBRACE_TOKEN },	/* { */
 { OP_CHAR, OFS_BOR },	/* | */	    { SEP_CHAR, CBRACE_TOKEN },	/* } */
 { OP_CHAR, OFS_BNOT },	/* ~ */	    { BAD_CHAR, 0 },	/* 0x7f */

 { ALPHA_CHAR, 0 },	/* € */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* ‚ */	    { ALPHA_CHAR, 0 },	/* ƒ */
 { ALPHA_CHAR, 0 },	/* „ */	    { ALPHA_CHAR, 0 },	/* … */
 { ALPHA_CHAR, 0 },	/* † */	    { ALPHA_CHAR, 0 },	/* ‡ */
 { ALPHA_CHAR, 0 },	/* ˆ */	    { ALPHA_CHAR, 0 },	/* ‰ */
 { ALPHA_CHAR, 0 },	/* Š */	    { ALPHA_CHAR, 0 },	/* ‹ */
 { ALPHA_CHAR, 0 },	/* Œ */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* Ž */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/*  */	    { ALPHA_CHAR, 0 },	/* ‘ */
 { ALPHA_CHAR, 0 },	/* ’ */	    { ALPHA_CHAR, 0 },	/* “ */
 { ALPHA_CHAR, 0 },	/* ” */	    { ALPHA_CHAR, 0 },	/* • */
 { ALPHA_CHAR, 0 },	/* – */	    { ALPHA_CHAR, 0 },	/* — */
 { ALPHA_CHAR, 0 },	/* ˜ */	    { ALPHA_CHAR, 0 },	/* ™ */
 { ALPHA_CHAR, 0 },	/* š */	    { ALPHA_CHAR, 0 },	/* › */
 { ALPHA_CHAR, 0 },	/* œ */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* ž */	    { ALPHA_CHAR, 0 },	/* Ÿ */
 { ALPHA_CHAR, 0 },	/*   */	    { ALPHA_CHAR, 0 },	/* ¡ */
 { ALPHA_CHAR, 0 },	/* ¢ */	    { ALPHA_CHAR, 0 },	/* £ */
 { ALPHA_CHAR, 0 },	/* ¤ */	    { ALPHA_CHAR, 0 },	/* ¥ */
 { ALPHA_CHAR, 0 },	/* ¦ */	    { ALPHA_CHAR, 0 },	/* § */
 { ALPHA_CHAR, 0 },	/* ¨ */	    { ALPHA_CHAR, 0 },	/* © */
 { ALPHA_CHAR, 0 },	/* ª */	    { ALPHA_CHAR, 0 },	/* « */
 { ALPHA_CHAR, 0 },	/* ¬ */	    { ALPHA_CHAR, 0 },	/* ­ */
 { ALPHA_CHAR, 0 },	/* ® */	    { ALPHA_CHAR, 0 },	/* ¯ */
 { ALPHA_CHAR, 0 },	/* ° */	    { ALPHA_CHAR, 0 },	/* ± */
 { ALPHA_CHAR, 0 },	/* ² */	    { ALPHA_CHAR, 0 },	/* ³ */
 { ALPHA_CHAR, 0 },	/* ´ */	    { ALPHA_CHAR, 0 },	/* µ */
 { ALPHA_CHAR, 0 },	/* ¶ */	    { ALPHA_CHAR, 0 },	/* · */
 { ALPHA_CHAR, 0 },	/* ¸ */	    { ALPHA_CHAR, 0 },	/* ¹ */
 { ALPHA_CHAR, 0 },	/* º */	    { ALPHA_CHAR, 0 },	/* » */
 { ALPHA_CHAR, 0 },	/* ¼ */	    { ALPHA_CHAR, 0 },	/* ½ */
 { ALPHA_CHAR, 0 },	/* ¾ */	    { ALPHA_CHAR, 0 },	/* ¿ */
 { ALPHA_CHAR, 0 },	/* À */	    { ALPHA_CHAR, 0 },	/* Á */
 { ALPHA_CHAR, 0 },	/* Â */	    { ALPHA_CHAR, 0 },	/* Ã */
 { ALPHA_CHAR, 0 },	/* Ä */	    { ALPHA_CHAR, 0 },	/* Å */
 { ALPHA_CHAR, 0 },	/* Æ */	    { ALPHA_CHAR, 0 },	/* Ç */
 { ALPHA_CHAR, 0 },	/* È */	    { ALPHA_CHAR, 0 },	/* É */
 { ALPHA_CHAR, 0 },	/* Ê */	    { ALPHA_CHAR, 0 },	/* Ë */
 { ALPHA_CHAR, 0 },	/* Ì */	    { ALPHA_CHAR, 0 },	/* Í */
 { ALPHA_CHAR, 0 },	/* Î */	    { ALPHA_CHAR, 0 },	/* Ï */
 { ALPHA_CHAR, 0 },	/* Ð */	    { ALPHA_CHAR, 0 },	/* Ñ */
 { ALPHA_CHAR, 0 },	/* Ò */	    { ALPHA_CHAR, 0 },	/* Ó */
 { ALPHA_CHAR, 0 },	/* Ô */	    { ALPHA_CHAR, 0 },	/* Õ */
 { ALPHA_CHAR, 0 },	/* Ö */	    { ALPHA_CHAR, 0 },	/* × */
 { ALPHA_CHAR, 0 },	/* Ø */	    { ALPHA_CHAR, 0 },	/* Ù */
 { ALPHA_CHAR, 0 },	/* Ú */	    { ALPHA_CHAR, 0 },	/* Û */
 { ALPHA_CHAR, 0 },	/* Ü */	    { ALPHA_CHAR, 0 },	/* Ý */
 { ALPHA_CHAR, 0 },	/* Þ */	    { ALPHA_CHAR, 0 },	/* ß */
 { ALPHA_CHAR, 0 },	/* à */	    { ALPHA_CHAR, 0 },	/* á */
 { ALPHA_CHAR, 0 },	/* â */	    { ALPHA_CHAR, 0 },	/* ã */
 { ALPHA_CHAR, 0 },	/* ä */	    { ALPHA_CHAR, 0 },	/* å */
 { ALPHA_CHAR, 0 },	/* æ */	    { ALPHA_CHAR, 0 },	/* ç */
 { ALPHA_CHAR, 0 },	/* è */	    { ALPHA_CHAR, 0 },	/* é */
 { ALPHA_CHAR, 0 },	/* ê */	    { ALPHA_CHAR, 0 },	/* ë */
 { ALPHA_CHAR, 0 },	/* ì */	    { ALPHA_CHAR, 0 },	/* í */
 { ALPHA_CHAR, 0 },	/* î */	    { ALPHA_CHAR, 0 },	/* ï */
 { ALPHA_CHAR, 0 },	/* ð */	    { ALPHA_CHAR, 0 },	/* ñ */
 { ALPHA_CHAR, 0 },	/* ò */	    { ALPHA_CHAR, 0 },	/* ó */
 { ALPHA_CHAR, 0 },	/* ô */	    { ALPHA_CHAR, 0 },	/* õ */
 { ALPHA_CHAR, 0 },	/* ö */	    { ALPHA_CHAR, 0 },	/* ÷ */
 { ALPHA_CHAR, 0 },	/* ø */	    { ALPHA_CHAR, 0 },	/* ù */
 { ALPHA_CHAR, 0 },	/* ú */	    { ALPHA_CHAR, 0 },	/* û */
 { ALPHA_CHAR, 0 },	/* ü */	    { ALPHA_CHAR, 0 },	/* ý */
 { ALPHA_CHAR, 0 },	/* þ */	    { ALPHA_CHAR, 0 },	/* ÿ */
};

int _pSLcheck_identifier_syntax (SLCONST char *name)
{
   unsigned char *p;

   p = (unsigned char *) name;
   if (ALPHA_CHAR == Char_Type_Table[*p][0]) while (1)
     {
	unsigned ch;
	unsigned char type;

	ch = *++p;

	type = Char_Type_Table [ch][0];
	if ((type != ALPHA_CHAR) && (type != DIGIT_CHAR))
	  {
	     if (ch == 0)
	       return 0;
	     break;
	  }
     }

   _pSLang_verror (SL_SYNTAX_ERROR,
		 "Identifier or structure field name '%s' contains an illegal character", name);
   return -1;
}

static unsigned char prep_get_char (void)
{
   register unsigned char ch;

   if (0 != (ch = *Input_Line_Pointer++))
     return ch;

   Input_Line_Pointer--;
   return 0;
}

static void unget_prep_char (unsigned char ch)
{
   if ((Input_Line_Pointer != Input_Line)
       && (ch != 0))
     Input_Line_Pointer--;
   /* *Input_Line_Pointer = ch; -- Do not modify the Input_Line */
}

#include "keywhash.c"

static int get_ident_token (_pSLang_Token_Type *tok, unsigned char *s, unsigned int len)
{
   unsigned char ch;
   unsigned char type;
   Keyword_Table_Type *table;

   while (1)
     {
	ch = prep_get_char ();
	type = CHAR_CLASS (ch);
	if ((type != ALPHA_CHAR) && (type != DIGIT_CHAR))
	  {
	     unget_prep_char (ch);
	     break;
	  }
	if (len == (SL_MAX_TOKEN_LEN - 1))
	  {
	     _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Identifier length exceeded maximum supported value", NULL, 0);
	     return tok->type = EOF_TOKEN;
	  }
	s [len++] = ch;
     }

   s[len] = 0;

   /* check if keyword */
   table = is_keyword ((char *) s, len);
   if (table != NULL)
     {
	tok->v.s_val = table->name;
	tok->free_val_func = free_static_sval_token;
	return (tok->type = table->type);
     }

   return _pSLtoken_init_slstring_token (tok, IDENT_TOKEN, (char *) s, len);
}

#define LONG_MASK (~(long)(unsigned long)-1)   /* 0 */
#define INT_MASK (~(long)(unsigned int)-1)
#define SHORT_MASK (~(long)(unsigned short)-1)
#define CHAR_MASK (~(long)(unsigned char)-1)

static int str_to_signed_constant (unsigned char *s, SLtype stype, _pSLtok_Type ttype, long mask,
				   _pSLang_Token_Type *tok)
{
   long lval = SLatol (s);

   if (lval & mask)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Literal integer constant is too large for %s", SLclass_get_datatype_name(stype));
	return tok->type = EOF_TOKEN;
     }

   (void) stype;
   tok->flags |= SLTOKEN_TYPE_INTEGER;
   tok->v.long_val = lval;
   return tok->type = ttype;
}

static int str_to_unsigned_constant (unsigned char *s, SLtype stype, _pSLtok_Type ttype, long mask,
				     _pSLang_Token_Type *tok)
{
   unsigned long lval = SLatoul (s);

   if (lval & mask)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Literal integer constant is too large for %s", SLclass_get_datatype_name(stype));
	return tok->type = EOF_TOKEN;
     }

   (void) stype;
   tok->flags |= SLTOKEN_TYPE_INTEGER;
   tok->v.long_val = (long) lval;
   return tok->type = ttype;
}

static int get_number_token (_pSLang_Token_Type *tok, unsigned char *s, unsigned int len)
{
   unsigned char ch;
   unsigned char type;
   int tok_flags = 0;
   int status;

   /* Look for pattern  [0-9.xXb]*([eE][-+]?[digits])?[ijfhul]? */
   while (1)
     {
	ch = prep_get_char ();

	type = CHAR_CLASS (ch);
	if ((type != DIGIT_CHAR) && (type != DOT_CHAR))
	  {
	     if ((ch == 'x') || (ch == 'X'))
	       tok_flags = SLTOKEN_IS_HEX;
	     else if ((ch == 'b') || (ch == 'B'))
	       tok_flags = SLTOKEN_IS_BINARY;
	     else
	       break;

	     do
	       {
		  if (len == (SL_MAX_TOKEN_LEN - 1))
		    goto too_long_return_error;

		  s[len++] = ch;
		  ch = prep_get_char ();
		  type = CHAR_CLASS (ch);
	       }
	     while ((type == DIGIT_CHAR) || (type == ALPHA_CHAR));
	     break;
	  }
	if (len == (SL_MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s[len++] = ch;
     }

   /* At this point, type and ch are synchronized */

   if ((ch == 'e') || (ch == 'E'))
     {
	if (len == (SL_MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s[len++] = ch;
	ch = prep_get_char ();
	if ((ch == '+') || (ch == '-'))
	  {
	     if (len == (SL_MAX_TOKEN_LEN - 1))
	       goto too_long_return_error;
	     s[len++] = ch;
	     ch = prep_get_char ();
	  }

	while (DIGIT_CHAR == (type = CHAR_CLASS(ch)))
	  {
	     if (len == (SL_MAX_TOKEN_LEN - 1))
	       goto too_long_return_error;
	     s[len++] = ch;
	     ch = prep_get_char ();
	  }
     }
   tok->flags |= tok_flags;

   while (ALPHA_CHAR == type)
     {
	if (len == (SL_MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s[len++] = ch;
	ch = prep_get_char ();
	type = CHAR_CLASS(ch);
     }

   unget_prep_char (ch);
   s[len] = 0;

   switch (SLang_guess_type ((char *) s))
     {
      default:
	tok->v.s_val = (char *) s;
	_pSLparse_error (SL_TYPE_MISMATCH, "Not a number", tok, 0);
	return (tok->type = EOF_TOKEN);

#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	status = _pSLtoken_init_slstring_token (tok, FLOAT_TOKEN, (char *)s, len);
	if (status == FLOAT_TOKEN)
	  tok->flags |= SLTOKEN_TYPE_FLOAT;
	return status;

      case SLANG_DOUBLE_TYPE:
	status = _pSLtoken_init_slstring_token (tok, DOUBLE_TOKEN, (char *)s, len);
	if (status == DOUBLE_TOKEN)
	  tok->flags |= SLTOKEN_TYPE_FLOAT;
	return status;
#endif
#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	status = _pSLtoken_init_slstring_token (tok, COMPLEX_TOKEN, (char *)s, len);
	if (status == COMPLEX_TOKEN)
	  tok->flags |= SLTOKEN_TYPE_FLOAT;
	return status;
#endif
      case SLANG_CHAR_TYPE:
	return str_to_signed_constant (s, SLANG_CHAR_TYPE, CHAR_TOKEN, CHAR_MASK, tok);

      case SLANG_UCHAR_TYPE:
	return str_to_unsigned_constant (s, SLANG_UCHAR_TYPE, UCHAR_TOKEN, CHAR_MASK, tok);

      case SLANG_SHORT_TYPE:
	return str_to_signed_constant (s, SLANG_SHORT_TYPE, SHORT_TOKEN, SHORT_MASK, tok);

      case SLANG_USHORT_TYPE:
	return str_to_unsigned_constant (s, SLANG_USHORT_TYPE, USHORT_TOKEN, SHORT_MASK, tok);

      case SLANG_INT_TYPE:
	return str_to_signed_constant (s, SLANG_INT_TYPE, INT_TOKEN, INT_MASK, tok);

      case SLANG_UINT_TYPE:
	return str_to_unsigned_constant (s, SLANG_UINT_TYPE, UINT_TOKEN, INT_MASK, tok);

      case SLANG_LONG_TYPE:
	return str_to_signed_constant (s, SLANG_LONG_TYPE, LONG_TOKEN, LONG_MASK, tok);

      case SLANG_ULONG_TYPE:
	return str_to_unsigned_constant (s, SLANG_ULONG_TYPE, ULONG_TOKEN, LONG_MASK, tok);

#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE:
	tok->v.llong_val = SLatoll (s);
	tok->flags |= SLTOKEN_TYPE_INTEGER;
	return tok->type = LLONG_TOKEN;
      case SLANG_ULLONG_TYPE:
	tok->flags |= SLTOKEN_TYPE_INTEGER;
	tok->v.ullong_val = SLatoull (s);
	return tok->type = ULLONG_TOKEN;
#endif
     }

   too_long_return_error:
   _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Number too long for buffer", NULL, 0);
   return (tok->type = EOF_TOKEN);
}

static int get_op_token (_pSLang_Token_Type *tok, char ch)
{
   unsigned int offset;
   char second_char;
   _pSLtok_Type type;
   SLCONST char *name;
   SLCONST Operator_Table_Entry_Type *op;

   /* operators are: + - / * ++ -- += -= = == != > < >= <= | etc..
    * These lex to the longest valid operator token.
    */

   offset = CHAR_DATA((unsigned char) ch);
   op = Operators + offset;
   if (0 == op->opstring[1])
     {
	name = op->opstring;
	type = op->type;
     }
   else
     {
	type = EOF_TOKEN;
	name = NULL;
     }

   second_char = prep_get_char ();
   do
     {
	if (second_char == op->opstring[1])
	  {
	     name = op->opstring;
	     type = op->type;
	     break;
	  }
	op++;
     }
   while (ch == op->opstring[0]);

   tok->type = type;

   if (type == EOF_TOKEN)
     {
	_pSLparse_error (SL_NOT_IMPLEMENTED, "Operator not supported", NULL, 0);
	return type;
     }

   tok->v.s_val = (char *)name;

   if (name[1] == 0)
     unget_prep_char (second_char);

   return type;
}

/* s and t may point to the same buffer --- even for unicode.  This
 * is because a wchar is denoted by (greater than) 4 characters \x{...}, which
 * will expand to at most 6 bytes when UTF-8 encoded.  That is:
 * \x{F} expands to 1 byte
 * \x{FF} expands to 2 bytes
 * \x{FFF} expands to 3 bytes
 * \x{FFFF} expands to 3 bytes
 * \x{FFFFF} expands to 4 bytes
 * \x{FFFFFF} expands to 5 bytes
 * \x{7FFFFFF} expands to 6 bytes
 *
 * Also, consider octal, decimal, and hex forms:
 *
 *    \200   (0x80)
 *    \d128
 *    \x80
 *
 * In all these cases, the escaped form uses 4 bytes.  Hence, these forms also
 * may be converted to UTF-8.
 */
/* If this returns non-zero, then it is a binary string */
static int expand_escaped_string (register char *s,
				  register char *t, register char *tmax,
				  unsigned int *lenp, int is_binary)
{
   char *s0;
   char ch;
#if 0
   int utf8_encode;

   utf8_encode = (is_binary == 0) && _pSLinterp_UTF8_Mode;
#endif
   s0 = s;
   while (t < tmax)
     {
	int isunicode;
	SLwchar_Type wch;
	char *s1;
	ch = *t++;

	if (ch != '\\')
	  {
	     if (ch == 0) is_binary = 1;
	     *s++ = ch;
	     continue;
	  }

	if ((t == tmax)		       /* \ at EOL */
	    || (((t + 1) == tmax) && (*t == '\n')))    /* \ \n at EOL */
	  break;

	if (NULL == (t = _pSLexpand_escaped_char (t, tmax, &wch, &isunicode)))
	  {
	     is_binary = -1;
	     break;
	  }
	if ((isunicode == 0)
#if 0
	    && ((wch < 127)
		|| (utf8_encode == 0))
#endif
	    )
	  {
	     if (wch == 0)
	       is_binary = 1;

	     *s++ = (char) wch;
	     continue;
	  }
	/* Escaped representation is always greater than encoded form.
	 * So, 6 below is ok (although ugly).
	 */
	s1 = (char *) SLutf8_encode (wch, (SLuchar_Type *)s, 6);
	if (s1 == NULL)
	  {
	     _pSLang_verror (SL_INVALID_UTF8, "Unable to UTF-8 encode 0x%lX\n", (unsigned long)wch);
	     is_binary = -1;
	     break;
	  }
	s = s1;
     }
   *s = 0;

   *lenp = (unsigned char) (s - s0);
   return is_binary;
}

#define STRING_SUFFIX_B		1
#define STRING_SUFFIX_Q		2
#define STRING_SUFFIX_R		4
#define STRING_SUFFIX_S		8
static int get_string_suffix (int *suffixp)
{
   int suffix = 0;

   while (1)
     {
	unsigned char ch = prep_get_char ();
	if (ch == 'B')
	  {
	     suffix |= STRING_SUFFIX_B;
	     continue;
	  }

	if (ch == 'R')
	  {
	     suffix |= STRING_SUFFIX_R;
	     continue;
	  }

	if (ch == 'Q')
	  {
	     suffix |= STRING_SUFFIX_Q;
	     continue;
	  }
	if (ch == '$')
	  {
	     suffix |= STRING_SUFFIX_S;
	     continue;
	  }
	unget_prep_char (ch);
	break;
     }

   if ((suffix & STRING_SUFFIX_R) && (suffix & STRING_SUFFIX_Q))
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Conflicting suffix for string literal", NULL, 0);
	return -1;
     }

   *suffixp = suffix;
   return 0;
}

static int process_string_token (_pSLang_Token_Type *tok, unsigned char quote_char,
				 unsigned char *s, unsigned int len,
				 int has_backslash)
{
   SLwchar_Type wch;

   if (('"' == quote_char) || (quote_char == '`'))
     {
	int suffix;
	int is_binary;

	if (-1 == get_string_suffix (&suffix))
	  return tok->type = EOF_TOKEN;

	if ((quote_char == '`') && (0 == (suffix & STRING_SUFFIX_Q)))
	  suffix |= STRING_SUFFIX_R;

	is_binary = (suffix & STRING_SUFFIX_B);
	if (suffix & STRING_SUFFIX_R)
	  has_backslash = 0;

	if (has_backslash)
	  is_binary = expand_escaped_string ((char *) s, (char *)s, (char *)s + len, &len, is_binary);

	if (is_binary && (suffix & STRING_SUFFIX_S))
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "A binary string is not permitted to have the $ suffix", NULL, 0);
	     return tok->type = EOF_TOKEN;
	  }

	if (is_binary)
	  return init_bstring_token (tok, s, len);
	else
	  {
	     _pSLtok_Type t = (suffix & STRING_SUFFIX_S) ? STRING_DOLLAR_TOKEN : STRING_TOKEN;
	     return _pSLtoken_init_slstring_token (tok, t, (char *) s, len);
	  }
     }

   /* else single character */

   if (has_backslash)
     {
	if ((s[0] != '\\')
	    || (NULL == (s = (unsigned char *)_pSLexpand_escaped_char ((char *)s+1, (char *)s+len, &wch, NULL)))
	    || (*s != 0))
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Unable to parse character", NULL, 0);
	     return (tok->type = EOF_TOKEN);
	  }
     }
   else if (len == 1)
     wch = s[0];
   else /* Assume unicode */
     {
	unsigned char *ss = SLutf8_decode (s, s+len, &wch, NULL);
	if ((ss == NULL) || (*ss != 0))
	  {
	     _pSLparse_error(SL_SYNTAX_ERROR, "Single char expected", NULL, 0);
	     return (tok->type = EOF_TOKEN);
	  }
     }
   tok->v.long_val = wch;

   if (wch > 256)
     return tok->type = ULONG_TOKEN;

   return (tok->type = UCHAR_TOKEN);
}

static int read_string_token (unsigned char quote_char,
			      unsigned char *s, unsigned int maxlen,
			      int is_multiline_raw,
			      int *has_backslashp,
			      unsigned int *lenp)
{
   unsigned int len = 0;
   int has_bs = 0;
   int is_continued = 0;

   while (len < maxlen)
     {
	unsigned char ch = prep_get_char ();

	if (ch == 0)
	  ch = '\n';

	if (ch == '\n')
	  {
	     if (is_multiline_raw)
	       {
		  s[len++] = ch;
		  is_continued = 1;
		  break;
	       }
	     _pSLparse_error(SL_SYNTAX_ERROR, "Expecting a quote-character", NULL, 0);
	     return -1;
	  }

	if (ch == quote_char)
	  {
	     if (is_multiline_raw == 0)
	       break;

	     /* For multi-line raw, double quotes make a single one */
	     ch = prep_get_char ();
	     if (ch == quote_char)
	       {
		  s[len++] = ch;
		  continue;
	       }

	     unget_prep_char (ch);
	     break;
	  }

	if (ch == '\\')
	  {
	     if (is_multiline_raw)
	       {
		  s[len++] = ch;
		  has_bs = 1;
		  continue;
	       }

	     ch = prep_get_char ();
	     if ((ch == '\n') || (ch == 0))
	       {
		  is_continued = 1;
		  break;
	       }
	     s[len++] = '\\';
	     if (len < maxlen)
	       {
		  s[len++] = ch;
		  has_bs = 1;
	       }
	     continue;
	  }

	s[len++] = ch;
     }

   if (len == maxlen)
     {
	_pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Literal string exceeds the maximum allowable size--- use concatenation", NULL, 0);
	return -1;
     }

   s[len] = 0;
   *lenp = len;
   *has_backslashp = has_bs;
   return is_continued;
}

static _pSLtoken_String_List_Type *alloc_string_list_type (unsigned char *buf, unsigned int len)
{
   _pSLtoken_String_List_Type *l;

   if (NULL == (l = (_pSLtoken_String_List_Type *) SLmalloc (sizeof (_pSLtoken_String_List_Type) + len)))
     return NULL;

   l->next = NULL;
   l->len = len;
   memcpy ((char *) l->buf, (char *)buf, len);
   return l;
}

static void free_string_list (_pSLtoken_String_List_Type *l)
{
   while (l != NULL)
     {
	_pSLtoken_String_List_Type *next = l->next;
	SLfree ((char *) l);
	l = next;
     }
}

static void free_multistring_token_val (_pSLang_Token_Type *tok)
{
   _pSLang_Multiline_String_Type *m;

   m = tok->v.multistring_val;
   if (m == NULL)
     return;

   if ((m->type == STRING_TOKEN) || (m->type == STRING_DOLLAR_TOKEN))
     {
	_pSLfree_hashed_string (m->v.s_val, m->len, m->hash);
     }
   else if (m->type == BSTRING_TOKEN)
     {
	SLbstring_free (m->v.b_val);
     }

   free_string_list (m->list);
   SLfree ((char *) m);

   tok->v.multistring_val = NULL;
}

static _pSLang_Multiline_String_Type *
  create_multistring (_pSLtoken_String_List_Type **rootp, _pSLtok_Type type)
{
   _pSLtoken_String_List_Type *root, *tail;
   _pSLang_Multiline_String_Type *m;
   char *buf;
   unsigned int len, num;

   m = (_pSLang_Multiline_String_Type *) SLmalloc (sizeof (_pSLang_Multiline_String_Type));
   if (m == NULL)
     return NULL;

   len = 0;
   num = 0;
   tail = root = *rootp;
   while (tail != NULL)
     {
	len += tail->len;
	tail = tail->next;
	num++;
     }

   if (NULL == (buf = SLmalloc (len+1)))
     {
	SLfree ((char *) m);
	return NULL;
     }

   tail = root;
   len = 0;
   while (tail != NULL)
     {
	memcpy (buf+len, tail->buf, tail->len);
	len += tail->len;
	tail = tail->next;
     }

   m->num = num;
   m->type = type;
   if (type == BSTRING_TOKEN)
     {
	if (NULL == (m->v.b_val = SLbstring_create_malloced ((unsigned char *)buf, len, 0)))
	  goto return_error;
	buf = NULL;
     }
   else
     {
	m->v.s_val = _pSLstring_make_hashed_string (buf, len, &m->hash);
	if (m->v.s_val == NULL)
	  goto return_error;
	SLfree (buf);
     }
   m->num = num;
   m->list = root;
   m->len = len;
   *rootp = NULL;
   return m;

return_error:
   if (buf != NULL) SLfree (buf);
   SLfree ((char *) m);
   return NULL;
}

static int get_string_token (_pSLang_Token_Type *tok, unsigned char quote_char,
			     unsigned char *s, int is_multiline_raw)
{
   _pSLtoken_String_List_Type *root, *tail;
   _pSLang_Multiline_String_Type *m;
   unsigned int len;
   int has_backslash;
   int status;
   unsigned int num_lines;
   int suffix;
   int is_binary;
   _pSLtok_Type type;

   status = read_string_token (quote_char, s, SL_MAX_TOKEN_LEN-1,
			       is_multiline_raw, &has_backslash, &len);

   if (status == -1)
     return tok->type = EOF_TOKEN;

   if (status == 0)
     return process_string_token (tok, quote_char, s, len, has_backslash);

   tail = root = alloc_string_list_type (s, len);
   if (root == NULL)
     return tok->type = EOF_TOKEN;

   LLT->parse_level += 1;

   num_lines = 1;
   do
     {
	int has_bs;

	if (-1 == next_input_line ())
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Multiline string literal is unterminated", NULL, 0);
	     goto return_error;
	  }

	status = read_string_token (quote_char, s, SL_MAX_TOKEN_LEN-1,
				    is_multiline_raw, &has_bs, &len);

	if ((status == -1)
	    || (NULL == (tail->next = alloc_string_list_type (s, len))))
	  goto return_error;

	has_backslash = has_backslash || has_bs;
	tail = tail->next;
	num_lines++;
     }
   while (status == 1);

   /* At this point, root contains the list of strings */
   if (-1 == get_string_suffix (&suffix))
     goto return_error;

   /* A multiline raw string is raw unless an explicit Q suffix was given */
   if (is_multiline_raw
       && (0 == (suffix & STRING_SUFFIX_Q)))
     suffix |= STRING_SUFFIX_R;

   is_binary = (suffix & STRING_SUFFIX_B);
   if (suffix & STRING_SUFFIX_R)
     has_backslash = 0;

   if (has_backslash)
     {
	tail = root;
	while (tail != NULL)
	  {
	     int ib;

	     ib = expand_escaped_string (tail->buf, tail->buf,
					 tail->buf+tail->len, &tail->len, is_binary);

	     is_binary = is_binary || ib;

	     if (is_binary && (suffix & STRING_SUFFIX_S))
	       {
		  _pSLparse_error (SL_SYNTAX_ERROR, "A binary string is not permitted to have the $ suffix", NULL, 0);
		  goto return_error;
	       }
	     tail = tail->next;
	  }
     }

   if (is_binary)
     type = BSTRING_TOKEN;
   else if (suffix & STRING_SUFFIX_S)
     type = STRING_DOLLAR_TOKEN;
   else
     type = STRING_TOKEN;

   if (NULL == (m = create_multistring (&root, type)))
     goto return_error;

   tok->v.multistring_val = m;
   tok->free_val_func = free_multistring_token_val;

   LLT->parse_level -= 1;
   return tok->type = MULTI_STRING_TOKEN;

return_error:

   if (root != NULL)
     free_string_list (root);

   LLT->parse_level -= 1;
   return tok->type = EOF_TOKEN;
}

static int extract_token (_pSLang_Token_Type *tok, unsigned char ch, unsigned char t)
{
   unsigned char s [SL_MAX_TOKEN_LEN];
   unsigned int slen;

   s[0] = (char) ch;
   slen = 1;

   switch (t)
     {
      case ALPHA_CHAR:
	return get_ident_token (tok, s, slen);

      case OP_CHAR:
	return get_op_token (tok, ch);

      case DIGIT_CHAR:
	return get_number_token (tok, s, slen);

      case EXCL_CHAR:
	ch = prep_get_char ();
	s [slen++] = ch;
	t = CHAR_CLASS(ch);
	if (t == ALPHA_CHAR) return get_ident_token (tok, s, slen);
	if (t == OP_CHAR)
	  {
	     unget_prep_char (ch);
	     return get_op_token (tok, '!');
	  }
	_pSLparse_error(SL_SYNTAX_ERROR, "Misplaced !", NULL, 0);
	return -1;

      case DOT_CHAR:
	ch = prep_get_char ();
	if (DIGIT_CHAR == CHAR_CLASS(ch))
	  {
	     s [slen++] = ch;
	     return get_number_token (tok, s, slen);
	  }
	unget_prep_char (ch);
	return (tok->type = DOT_TOKEN);

      case SEP_CHAR:
	return (tok->type = CHAR_DATA(ch));

      case DQUOTE_CHAR:
      case QUOTE_CHAR:
	return get_string_token (tok, ch, s, 0);

      case BQUOTE_CHAR:
	return get_string_token (tok, ch, s, 1);

      default:
	_pSLparse_error(SL_SYNTAX_ERROR, "Invalid character", NULL, 0);
	return (tok->type = EOF_TOKEN);
     }
}

int _pSLget_rpn_token (_pSLang_Token_Type *tok)
{
   unsigned char ch;

   tok->v.s_val = "??";
   while ((ch = *Input_Line_Pointer) != 0)
     {
	unsigned char t;

	Input_Line_Pointer++;
	if (WHITE_CHAR == (t = CHAR_CLASS(ch)))
	  continue;

	if (NL_CHAR == t)
	  break;

	return extract_token (tok, ch, t);
     }
   Input_Line_Pointer = Empty_Line;
   return EOF_TOKEN;
}

int _pSLget_token (_pSLang_Token_Type *tok)
{
   unsigned char ch;
   unsigned char t;

   tok->num_refs = 1;
   tok->free_val_func = NULL;
   tok->v.s_val = "??";
   tok->flags = 0;
#if SLANG_HAS_DEBUG_CODE
   tok->line_number = LLT->line_num;
#endif
   if (_pSLang_Error || (Input_Line == NULL))
     return (tok->type = EOF_TOKEN);

   while (1)
     {
	ch = *Input_Line_Pointer++;
	if (WHITE_CHAR == (t = CHAR_CLASS (ch)))
	  continue;

	if (t != NL_CHAR)
	  return extract_token (tok, ch, t);

	do
	  {
#if SLANG_HAS_DEBUG_CODE
	     tok->line_number++;
#endif
	     if (-1 == next_input_line ())
	       return tok->type = EOF_TOKEN;
	  }
	while (0 == SLprep_line_ok(Input_Line, This_SLpp));

	if (*Input_Line_Pointer == '.')
	  {
	     Input_Line_Pointer++;
	     return tok->type = RPN_TOKEN;
	  }
     }
}

static int prep_exists_function (SLprep_Type *pt, SLFUTURE_CONST char *line)
{
   char buf[MAX_FILE_LINE_LEN+1], *b, *bmax;
   unsigned char ch;
   unsigned char comment;

   (void) pt;
   bmax = buf + (sizeof (buf) - 1);

   comment = (unsigned char)'%';
   while (1)
     {
	/* skip whitespace */
	while ((ch = (unsigned char) *line),
	       ch && (ch != '\n') && (ch <= ' '))
	  line++;

	if ((ch <= '\n')
	    || (ch == comment)) break;

	b = buf;
	while ((ch = (unsigned char) *line) > ' ')
	  {
	     if (b < bmax) *b++ = (char) ch;
	     line++;
	  }
	*b = 0;
#if 0
	if (SLang_is_defined (buf))
	  return 1;
#else
	if (NULL != _pSLlocate_name (buf))
	  return 1;
#endif
     }

   return 0;
}

static int prep_eval_expr (SLprep_Type *pt, SLFUTURE_CONST char *expr)
{
   int ret;
   SLCONST char *end;
   void (*compile)(_pSLang_Token_Type *);
   char *expr1;
#if SLANG_HAS_BOSEOS
   int boseos;
#endif

   (void) pt;
   end = strchr (expr, '\n');
   if (end == NULL)
     end = expr + strlen (expr);
   expr1 = SLmake_nstring (expr, (unsigned int) (end - expr));
   if (expr1 == NULL)
     return -1;

   compile = _pSLcompile_ptr;
   _pSLcompile_ptr = _pSLcompile;
#if SLANG_HAS_BOSEOS
   boseos = _pSLang_Compile_BOSEOS;
   if (0 == (boseos & SLANG_BOSEOS_PREPROC))
     _pSLang_Compile_BOSEOS = 0;
#endif
   if ((0 != SLns_load_string (expr1, _pSLang_cur_namespace_intrinsic ()))
       || (-1 == SLang_pop_integer (&ret)))
     ret = -1;
   else
     ret = (ret != 0);
#if SLANG_HAS_BOSEOS
   _pSLang_Compile_BOSEOS = boseos;
#endif
   _pSLcompile_ptr = compile;

   SLfree (expr1);
   return ret;
}

int SLang_load_object (SLang_Load_Type *x)
{
   SLprep_Type *this_pp;
   SLprep_Type *save_this_pp;
   SLang_Load_Type *save_llt;
   char *save_input_line, *save_input_line_ptr;
#if SLANG_HAS_DEBUG_CODE
   /* int save_compile_line_num_info; */
#endif
#if SLANG_HAS_BOSEOS
   int save_compile_boseos;
   int save_compile_bofeof;
#endif
   int save_auto_declare_variables;

   if (NULL == (this_pp = SLprep_new ()))
     return -1;
   (void) SLprep_set_exists_hook (this_pp, prep_exists_function);
   (void) SLprep_set_eval_hook (this_pp, prep_eval_expr);

   if (-1 == _pSLcompile_push_context (x))
     {
	SLprep_delete (this_pp);
	return -1;
     }

#if SLANG_HAS_DEBUG_CODE
   /* save_compile_line_num_info = _pSLang_Compile_Line_Num_Info; */
#endif
#if SLANG_HAS_BOSEOS
   save_compile_boseos = _pSLang_Compile_BOSEOS;
   save_compile_bofeof = _pSLang_Compile_BOFEOF;
#endif
   save_this_pp = This_SLpp;
   save_input_line = Input_Line;
   save_input_line_ptr = Input_Line_Pointer;
   save_llt = LLT;
   save_auto_declare_variables = _pSLang_Auto_Declare_Globals;

   This_SLpp = this_pp;
   Input_Line_Pointer = Input_Line = Empty_Line;
   LLT = x;

   /* x->line_num = 0; */  /* already set to 0 when allocated. */
   x->parse_level = 0;
   _pSLang_Auto_Declare_Globals = x->auto_declare_globals;

#if SLANG_HAS_DEBUG_CODE
   /* _pSLang_Compile_Line_Num_Info = Default_Compile_Line_Num_Info; */
#endif
#if SLANG_HAS_BOSEOS
#if 0
   /* Instead of setting this variable to 0, let it keep its current value.
    * Suppose that the following evalfiles take place:
    *
    *  A -> B1  --> C1  --> D1
    *    -> B2  --> C1  --> D2
    *
    * and that B1 sets _boseos_info to 1.  Then C1 and D1 will get this value
    * but B2 will not, since it will get rest to 0 when the routine has finished
    * loading B1.
    */
     {
	char *env = getenv ("SLANG_BOSEOS");
	if (env != NULL)
	  _pSLang_Compile_BOSEOS = atoi (env);
	else
	  _pSLang_Compile_BOSEOS = 0;
     }
#endif
#endif
   _pSLparse_start (x);
   if (_pSLang_Error)
     {
       if (_pSLang_Error != SL_Usage_Error)
	  (void) _pSLerr_set_line_info (x->name, x->line_num, NULL);
	/* Doing this resets the state of the line_info object */
	(void) _pSLerr_set_line_info (x->name, x->line_num, "");
     }

   _pSLang_Auto_Declare_Globals = save_auto_declare_variables;

   (void) _pSLcompile_pop_context ();

   Input_Line = save_input_line;
   Input_Line_Pointer = save_input_line_ptr;
   LLT = save_llt;
   SLprep_delete (this_pp);
   This_SLpp = save_this_pp;

#if SLANG_HAS_DEBUG_CODE
   /* _pSLang_Compile_Line_Num_Info = save_compile_line_num_info; */
#endif
#if SLANG_HAS_BOSEOS
   _pSLang_Compile_BOSEOS = save_compile_boseos;
   _pSLang_Compile_BOFEOF = save_compile_bofeof;
#endif
   if (_pSLang_Error) return -1;
   return 0;
}

SLang_Load_Type *SLns_allocate_load_type (SLFUTURE_CONST char *name, SLFUTURE_CONST char *namespace_name)
{
   SLang_Load_Type *x;

   if (NULL == (x = (SLang_Load_Type *)SLmalloc (sizeof (SLang_Load_Type))))
     return NULL;
   memset ((char *) x, 0, sizeof (SLang_Load_Type));

   if (name == NULL) name = "";

   if (NULL == (x->name = SLang_create_slstring (name)))
     {
	SLfree ((char *) x);
	return NULL;
     }

   if (namespace_name != NULL)
     {
	if (NULL == (x->namespace_name = SLang_create_slstring (namespace_name)))
	  {
	     SLang_free_slstring ((char *) x->name);
	     SLfree ((char *) x);
	     return NULL;
	  }
     }

   return x;
}

SLang_Load_Type *SLallocate_load_type (SLFUTURE_CONST char *name)
{
   return SLns_allocate_load_type (name, NULL);
}

void SLdeallocate_load_type (SLang_Load_Type *x)
{
   if (x != NULL)
     {
	SLang_free_slstring ((char *) x->name);
	SLang_free_slstring ((char *) x->namespace_name);
	SLfree ((char *) x);
     }
}

typedef struct
{
   SLCONST char *string;
   SLCONST char *ptr;
}
String_Client_Data_Type;

static char *read_from_string (SLang_Load_Type *x)
{
   String_Client_Data_Type *data;
   SLCONST char *s, *s1;
   char ch;

   data = (String_Client_Data_Type *)x->client_data;
   s1 = s = data->ptr;

   if (*s == 0)
     return NULL;

   while ((ch = *s) != 0)
     {
	s++;
	if (ch == '\n')
	  break;
     }

   data->ptr = s;
   return (char *) s1;
}

int SLang_load_string (SLFUTURE_CONST char *string)
{
   return SLns_load_string (string, NULL);
}

int SLns_load_string (SLFUTURE_CONST char *string, SLFUTURE_CONST char *ns_name)
{
   SLang_Load_Type *x;
   String_Client_Data_Type data;
   int ret;

   if (string == NULL)
     return -1;

   /* Grab a private copy in case loading modifies string */
   if (NULL == (string = SLang_create_slstring (string)))
     return -1;

   /* To avoid creating a static data space for every string loaded,
    * all string objects will be regarded as identical.  So, identify
    * all of them by ***string***
    */
   if (NULL == (x = SLns_allocate_load_type ("***string***", ns_name)))
     {
	SLang_free_slstring ((char *) string);
	return -1;
     }

   x->client_data = (VOID_STAR) &data;
   x->read = read_from_string;

   data.ptr = data.string = string;
   if ((-1 == (ret = SLang_load_object (x)))
       && (SLang_Traceback & SL_TB_FULL))
     _pSLerr_traceback_msg ("Traceback: called from eval: %s\n", string);

   SLang_free_slstring ((char *)string);
   SLdeallocate_load_type (x);
   return ret;
}

typedef struct
{
   char *buf;
   FILE *fp;
}
File_Client_Data_Type;

char *SLang_User_Prompt = NULL;
static char *read_from_file (SLang_Load_Type *x)
{
   FILE *fp;
   File_Client_Data_Type *c;
   char *buf;

   c = (File_Client_Data_Type *)x->client_data;
   fp = c->fp;

   if ((fp == stdin) && (SLang_User_Prompt != NULL))
     {
	fputs (SLang_User_Prompt, stdout);
	fflush (stdout);
     }

   buf = fgets (c->buf, MAX_FILE_LINE_LEN+1, c->fp);
   if (buf != NULL)
     {
	unsigned int num;

	num = strlen (buf);
	if ((num == MAX_FILE_LINE_LEN)
	    && (buf[num-1] != '\n'))
	  {
	     SLang_verror (SL_LimitExceeded_Error, "Line %u is too long or lacks a newline character", x->line_num);
	     return NULL;
	  }
     }
   return buf;
}

int _pSLang_Load_File_Verbose = 0;
int SLang_load_file_verbose (int v)
{
   int v1 = _pSLang_Load_File_Verbose;
   _pSLang_Load_File_Verbose = v;
   return v1;
}

/* Note that file could be freed from Slang during run of this routine
 * so get it and store it !! (e.g., autoloading)
 */
int (*SLang_Load_File_Hook) (SLFUTURE_CONST char *) = NULL;
int (*SLns_Load_File_Hook) (SLFUTURE_CONST char *, SLFUTURE_CONST char *) = NULL;
int SLang_load_file (SLFUTURE_CONST char *f)
{
   return SLns_load_file (f, NULL);
}

int SLns_load_file (SLFUTURE_CONST char *f, SLFUTURE_CONST char *ns_name)
{
   File_Client_Data_Type client_data;
   SLang_Load_Type *x;
   char *name, *buf;
   FILE *fp;

   if ((ns_name == NULL) && (NULL != SLang_Load_File_Hook))
     return (*SLang_Load_File_Hook) (f);

   if (SLns_Load_File_Hook != NULL)
     return (*SLns_Load_File_Hook) (f, ns_name);

   if (f == NULL)
     name = SLang_create_slstring ("<stdin>");
   else
     name = _pSLpath_find_file (f, 1);

   if (name == NULL)
     return -1;

   if (NULL == (x = SLns_allocate_load_type (name, ns_name)))
     {
	SLang_free_slstring (name);
	return -1;
     }

   buf = NULL;

   if (f != NULL)
     {
	fp = fopen (name, "r");
	if (_pSLang_Load_File_Verbose & SLANG_LOAD_FILE_VERBOSE)
	  {
	     if ((ns_name != NULL)
		 && (*ns_name != 0) && (0 != strcmp (ns_name, "Global")))
	       SLang_vmessage ("Loading %s [ns:%s]", name, ns_name);
	     else
	       SLang_vmessage ("Loading %s", name);
	  }
     }
   else
     fp = stdin;

   if (fp == NULL)
     _pSLang_verror (SL_OBJ_NOPEN, "Unable to open %s", name);
   else if (NULL != (buf = SLmalloc (MAX_FILE_LINE_LEN + 1)))
     {
	client_data.fp = fp;
	client_data.buf = buf;
	x->client_data = (VOID_STAR) &client_data;
	x->read = read_from_file;

	(void) SLang_load_object (x);
     }

   if ((fp != NULL) && (fp != stdin))
     fclose (fp);

   SLfree (buf);
   SLang_free_slstring (name);
   SLdeallocate_load_type (x);

   if (_pSLang_Error)
     return -1;

   return 0;
}

/* In the byte-compiled file, a token is represented by N+3 bytes, where
 *   byte[0] = TOKEN_TYPE;
 *   byte[1] = len_lo
 *   byte[2] = len_hi
 *   bytes[3:N+3] = value
 * and N = (len_lo - 32) | (len_hi-32) << 7.
 * The maximumn value of N for this encoding is 28639.
 */
static char *check_byte_compiled_token (char *buf)
{
   unsigned int len_lo, len_hi, len;
   char *input_buffer;
   char *b;

   input_buffer = Input_Line_Pointer;

   len_lo = (unsigned char) *input_buffer++;
   while ((len_lo == 0) || (len_lo == '\n'))
     {
	if (-1 == next_input_line ())
	  goto return_error;
	input_buffer = Input_Line_Pointer;
	len_lo = (unsigned char) *input_buffer++;
     }

   len_hi = (unsigned char) *input_buffer++;
   while ((len_hi == 0) || (len_hi == '\n'))
     {
	if (-1 == next_input_line ())
	  goto return_error;
	input_buffer = Input_Line_Pointer;
	len_hi = (unsigned char) *input_buffer++;
     }

   if ((len_lo < 32) || (len_hi < 32)
       || ((len = (len_lo - 32) | ((len_hi - 32) << 7)) >= SL_MAX_TOKEN_LEN))
     goto return_error;

   b = buf;
   while (len)
     {
	char ch = *input_buffer++;
	if ((ch == 0) || (ch == '\n'))
	  {
	     if (-1 == next_input_line ())
	       goto return_error;

	     input_buffer = Input_Line_Pointer;
	     continue;
	  }
	*b++ = ch;
	len--;
     }
   *b = 0;
   Input_Line_Pointer = input_buffer;
   return b;

return_error:
   _pSLang_verror (SL_INVALID_DATA_ERROR, "Byte compiled file appears corrupt");
   return NULL;
}

static int compile_byte_compiled_multistring (char *buf)
{
   _pSLtok_Type type;
   _pSLtoken_String_List_Type *root, *tail;
   _pSLang_Multiline_String_Type *m;
   _pSLang_Token_Type tok;

   /* The caller read MULTI_STRING_TOKEN from the input stream.  Concat the
    * next N objects in the stream until another MULTI_STRING_TOKEN is seen.
    */
   type = 0;
   tail = root = NULL;
   while (1)
     {
	char *ebuf;
	unsigned int len;
	_pSLtok_Type this_type, last_type;
	_pSLtoken_String_List_Type *next;
	unsigned char ch = *Input_Line_Pointer++;

	if ((ch == 0) || (ch == '\n'))
	  {
	     if (-1 == next_input_line ())
	       return -1;
	     continue;
	  }
	this_type = ch;

	if (this_type == MULTI_STRING_TOKEN)
	  break;

	switch (this_type)
	  {
	   case STRING_TOKEN:
	   case STRING_DOLLAR_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       goto return_error;
	     len = (unsigned int) (ebuf - buf);
	     last_type = this_type;
	     break;

	   case ESC_STRING_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       goto return_error;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 0);
	     last_type = STRING_TOKEN;
	     break;

	   case ESC_STRING_DOLLAR_TOKEN:
	     last_type = STRING_DOLLAR_TOKEN;
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       goto return_error;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 0);
	     last_type = STRING_DOLLAR_TOKEN;
	     break;

	   case ESC_BSTRING_TOKEN:
	     type = BSTRING_TOKEN;
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       goto return_error;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 1);
	     last_type = BSTRING_TOKEN;
	     break;

	   default:
	     SLang_verror (SL_INVALID_DATA_ERROR, "Unexpected object (0x%X) encountered in stream", (int)this_type);
	     goto return_error;
	  }

	if ((last_type != type) && (type != 0))
	  {
	     SLang_verror (SL_INVALID_DATA_ERROR, "Unexpected object (0x%X) encountered in stream", (int)this_type);
	     return -1;
	  }
	type = last_type;

	if (NULL == (next = alloc_string_list_type ((unsigned char *)buf, len)))
	  goto return_error;
	if (root == NULL)
	  root = next;
	else
	  tail->next = next;
	tail = next;
     }

   if (NULL == (m = create_multistring (&root, type)))
     goto return_error;

   tok.v.multistring_val = m;
   tok.free_val_func = free_multistring_token_val;
   tok.type = MULTI_STRING_TOKEN;
   (*_pSLcompile_ptr)(&tok);
   (*tok.free_val_func) (&tok);
   if (_pSLang_Error)
     return -1;
   return 0;

return_error:
   if (root != NULL) free_string_list (root);
   return -1;
}

void _pSLcompile_byte_compiled (void)
{
   _pSLtok_Type type;
   _pSLang_Token_Type tok;
   char buf[SL_MAX_TOKEN_LEN+1];
   char *ebuf;
   unsigned int len;

   memset ((char *) &tok, 0, sizeof (_pSLang_Token_Type));

   while (_pSLang_Error == 0)
     {
	top_of_switch:
	type = (unsigned char) *Input_Line_Pointer++;
	switch (type)
	  {
	   case '\n':
	   case 0:
	     if (-1 == next_input_line ())
	       return;

	     goto top_of_switch;

	   case CONT_N_TOKEN:
	   case BREAK_N_TOKEN:
	   case LINE_NUM_TOKEN:
	   case CHAR_TOKEN:
	   case UCHAR_TOKEN:
	   case SHORT_TOKEN:
	   case USHORT_TOKEN:
	   case INT_TOKEN:
	   case UINT_TOKEN:
	   case LONG_TOKEN:
	   case ULONG_TOKEN:
	     if (NULL == check_byte_compiled_token (buf))
	       return;
	     tok.v.long_val = atol (buf);
	     break;
#ifdef HAVE_LONG_LONG
	   case LLONG_TOKEN:
	   case ULLONG_TOKEN:
	     if (NULL == check_byte_compiled_token (buf))
	       return;
	     tok.v.llong_val = SLatoll ((unsigned char *)buf);
	     break;
#endif
	   case COMPLEX_TOKEN:
	   case FLOAT_TOKEN:
	   case DOUBLE_TOKEN:
	     if (NULL == check_byte_compiled_token (buf))
	       return;
	     tok.v.s_val = buf;
	     break;

	   case ESC_STRING_DOLLAR_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 0);
	     tok.hash = _pSLstring_hash ((unsigned char *)buf, (unsigned char *)buf + len);
	     type = STRING_DOLLAR_TOKEN;
	     break;

	   case ESC_STRING_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 0);
	     tok.hash = _pSLstring_hash ((unsigned char *)buf, (unsigned char *)buf + len);
	     type = STRING_TOKEN;
	     break;

	   case ESC_BSTRING_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     (void) expand_escaped_string (buf, buf, ebuf, &len, 1);
	     tok.hash = len;
	     type = _BSTRING_TOKEN;
	     break;

	   case TMP_TOKEN:
	   case DEFINE_TOKEN:
	   case DEFINE_STATIC_TOKEN:
	   case DEFINE_PRIVATE_TOKEN:
	   case DEFINE_PUBLIC_TOKEN:
	   case DOT_TOKEN:
	   case DOT_METHOD_CALL_TOKEN:
	   case STRING_DOLLAR_TOKEN:
	   case STRING_TOKEN:
	   case IDENT_TOKEN:
	   case _REF_TOKEN:
	   /* case _DEREF_ASSIGN_TOKEN: */
	   case _SCALAR_ASSIGN_TOKEN:
	   case _SCALAR_PLUSEQS_TOKEN:
	   case _SCALAR_MINUSEQS_TOKEN:
	   case _SCALAR_TIMESEQS_TOKEN:
	   case _SCALAR_DIVEQS_TOKEN:
	   case _SCALAR_BOREQS_TOKEN:
	   case _SCALAR_BANDEQS_TOKEN:
	   case _SCALAR_PLUSPLUS_TOKEN:
	   case _SCALAR_POST_PLUSPLUS_TOKEN:
	   case _SCALAR_MINUSMINUS_TOKEN:
	   case _SCALAR_POST_MINUSMINUS_TOKEN:
	   case _STRUCT_ASSIGN_TOKEN:
	   case _STRUCT_PLUSEQS_TOKEN:
	   case _STRUCT_MINUSEQS_TOKEN:
	   case _STRUCT_TIMESEQS_TOKEN:
	   case _STRUCT_DIVEQS_TOKEN:
	   case _STRUCT_BOREQS_TOKEN:
	   case _STRUCT_BANDEQS_TOKEN:
	   case _STRUCT_POST_MINUSMINUS_TOKEN:
	   case _STRUCT_MINUSMINUS_TOKEN:
	   case _STRUCT_POST_PLUSPLUS_TOKEN:
	   case _STRUCT_PLUSPLUS_TOKEN:
	   case _STRUCT_FIELD_REF_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     tok.hash = _pSLstring_hash ((unsigned char *)buf, (unsigned char *)ebuf);
	     break;

	   case MULTI_STRING_TOKEN:
	     if (-1 == compile_byte_compiled_multistring (buf))
	       return;
	     type = 0;
	     break;

	   default:
	     break;
	  }

	if (type == 0)
	  continue;

	tok.type = type;
	(*_pSLcompile_ptr) (&tok);
     }
}

static int escape_string (unsigned char *s, unsigned char *smax,
			  unsigned char *buf, unsigned char *buf_max,
			  int *is_escaped)
{
   unsigned char ch;

   *is_escaped = 0;
   while (buf < buf_max)
     {
	if (s == smax)
	  {
	     *buf = 0;
	     return 0;
	  }

	ch = *s++;
	switch (ch)
	  {
	   default:
	     *buf++ = ch;
	     break;

	   case 0:
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'x';
	     if (buf < buf_max) *buf++ = '0';
	     if (buf < buf_max) *buf++ = '0';
	     *is_escaped = 1;
	     break; /* return 0; */

	   case '\n':
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'n';
	     *is_escaped = 1;
	     break;

	   case '\r':
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'r';
	     *is_escaped = 1;
	     break;

	   case 0x1A:		       /* ^Z */
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'x';
	     if (buf < buf_max) *buf++ = '1';
	     if (buf < buf_max) *buf++ = 'A';
	     *is_escaped = 1;
	     break;

	   case '\\':
	     *buf++ = ch;
	     if (buf < buf_max) *buf++ = ch;
	     *is_escaped = 1;
	     break;
	  }
     }
   _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "String too long to byte-compile", NULL, 0);
   return -1;
}

static FILE *Byte_Compile_Fp;
static unsigned int Byte_Compile_Line_Len;

static int bytecomp_write_data (SLCONST char *buf, unsigned int len)
{
   unsigned int clen = Byte_Compile_Line_Len;
   unsigned int clen_max = MAX_FILE_LINE_LEN - 1;
   SLCONST char *bufmax = buf + len;
   FILE *fp = Byte_Compile_Fp;

   while (buf < bufmax)
     {
	if (clen == clen_max)
	  {
	     if (EOF == putc ('\n', fp))
	       {
		  SLang_set_error (SL_IO_WRITE_ERROR);
		  return -1;
	       }
	     clen = 0;
	  }

	if (EOF == putc (*buf, fp))
	  {
	     SLang_set_error (SL_IO_WRITE_ERROR);
	     return -1;
	  }
	buf++;
	clen++;
     }
   Byte_Compile_Line_Len = clen;
   return 0;
}

static int byte_compile_multiline_token (_pSLang_Token_Type *tok,
					 unsigned char *buf, unsigned char *buf_max)
{
   _pSLtoken_String_List_Type *root;
   _pSLang_Multiline_String_Type *m;
   _pSLtok_Type type, esc_type;
   char *b3;

   /* The token consists of N elements.  It gets encoded as N+2 tokens:
    *   TYPE STRING_1 .... STRING_N TYPE
    */
   m = tok->v.multistring_val;
   switch (m->type)
     {
      case STRING_TOKEN:
	type = STRING_TOKEN;
	esc_type = ESC_STRING_TOKEN;
	break;

      case BSTRING_TOKEN:
	type = esc_type = ESC_BSTRING_TOKEN;
	break;

      case STRING_DOLLAR_TOKEN:
	type = STRING_DOLLAR_TOKEN;
	esc_type = ESC_STRING_DOLLAR_TOKEN;
	break;

      default:
	SLang_verror (SL_Internal_Error, "Unsupported multline token: 0x%X", tok->type);
	return -1;
     }

   buf[0] = tok->type;
   buf[1] = 0;
   if (-1 == bytecomp_write_data ((char *)buf, 1))
     return -1;

   root = m->list;
   b3 = (char *)buf + 3;
   while (root != NULL)
     {
	unsigned int len;
	int is_escaped;

	if (-1 == escape_string ((unsigned char *)root->buf, (unsigned char *)root->buf + root->len,
				 (unsigned char *)b3, buf_max, &is_escaped))
	  return -1;

	if (is_escaped)
	  buf[0] = esc_type;
	else
	  buf[0] = type;

	len = strlen (b3);
	buf[1] = (unsigned char) ((len & 0x7F) + 32);
	buf[2] = (unsigned char) (((len >> 7) & 0x7F) + 32);
	len += 3;

	if (-1 == bytecomp_write_data ((char *) buf, len))
	  return -1;

	root = root->next;
     }

   buf[0] = tok->type;
   buf[1] = 0;
   if (-1 == bytecomp_write_data ((char *)buf, 1))
     return -1;

   return 0;
}

static void byte_compile_token (_pSLang_Token_Type *tok)
{
   unsigned char buf [SL_MAX_TOKEN_LEN + 4], *buf_max;
   unsigned int len;
   char *b3;
   int is_escaped;
   unsigned char *s;

   if (_pSLang_Error) return;

   buf [0] = (unsigned char) tok->type;
   buf [1] = 0;

   buf_max = buf + sizeof(buf);
   b3 = (char *) buf + 3;

   switch (tok->type)
     {
      case BOS_TOKEN:
      case LINE_NUM_TOKEN:
      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	sprintf (b3, "%ld", tok->v.long_val);
	break;

      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
	sprintf (b3, "%lu", tok->v.long_val);
	break;

#ifdef HAVE_LONG_LONG
      case LLONG_TOKEN:
	sprintf (b3, "%lld", tok->v.llong_val);
	break;

      case ULLONG_TOKEN:
	sprintf (b3, "%llu", tok->v.ullong_val);
	break;
#endif
      case _BSTRING_TOKEN:
	s = (unsigned char *) tok->v.s_val;
	len = (unsigned int) tok->hash;

	if (-1 == escape_string (s, s + len,
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;

	buf[0] = ESC_BSTRING_TOKEN;
	break;

      case BSTRING_TOKEN:
	if (NULL == (s = SLbstring_get_pointer (tok->v.b_val, &len)))
	  return;

	if (-1 == escape_string (s, s + len,
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;
	buf[0] = ESC_BSTRING_TOKEN;
	break;

      case STRING_DOLLAR_TOKEN:
      case STRING_TOKEN:
	s = (unsigned char *)tok->v.s_val;

	if (-1 == escape_string (s, s + strlen ((char *)s),
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;

	if (is_escaped)
	  buf[0] = ((tok->type == STRING_TOKEN)
		    ? ESC_STRING_TOKEN : ESC_STRING_DOLLAR_TOKEN);
	break;

      /* case _DEREF_ASSIGN_TOKEN: */
	/* a _SCALAR_* token is attached to an identifier. */
      case _SCALAR_ASSIGN_TOKEN:
      case _SCALAR_PLUSEQS_TOKEN:
      case _SCALAR_MINUSEQS_TOKEN:
      case _SCALAR_TIMESEQS_TOKEN:
      case _SCALAR_DIVEQS_TOKEN:
      case _SCALAR_BOREQS_TOKEN:
      case _SCALAR_BANDEQS_TOKEN:
      case _SCALAR_PLUSPLUS_TOKEN:
      case _SCALAR_POST_PLUSPLUS_TOKEN:
      case _SCALAR_MINUSMINUS_TOKEN:
      case _SCALAR_POST_MINUSMINUS_TOKEN:
      case DOT_TOKEN:
      case DOT_METHOD_CALL_TOKEN:
      case TMP_TOKEN:
      case DEFINE_TOKEN:
      case DEFINE_STATIC_TOKEN:
      case DEFINE_PRIVATE_TOKEN:
      case DEFINE_PUBLIC_TOKEN:
      case FLOAT_TOKEN:
      case DOUBLE_TOKEN:
      case COMPLEX_TOKEN:
      case IDENT_TOKEN:
      case _REF_TOKEN:
      case _STRUCT_ASSIGN_TOKEN:
      case _STRUCT_PLUSEQS_TOKEN:
      case _STRUCT_MINUSEQS_TOKEN:
      case _STRUCT_TIMESEQS_TOKEN:
      case _STRUCT_DIVEQS_TOKEN:
      case _STRUCT_BOREQS_TOKEN:
      case _STRUCT_BANDEQS_TOKEN:
      case _STRUCT_POST_MINUSMINUS_TOKEN:
      case _STRUCT_MINUSMINUS_TOKEN:
      case _STRUCT_POST_PLUSPLUS_TOKEN:
      case _STRUCT_PLUSPLUS_TOKEN:
      case _STRUCT_FIELD_REF_TOKEN:
	strcpy (b3, tok->v.s_val);
	break;

      case BREAK_N_TOKEN:
      case CONT_N_TOKEN:
	sprintf (b3, "%ld", tok->v.long_val);
	break;

      case MULTI_STRING_TOKEN:
	(void) byte_compile_multiline_token (tok, buf, buf_max);
	return;

      default:
	b3 = NULL;
     }

   if (b3 != NULL)
     {
	len = strlen (b3);
	buf[1] = (unsigned char) ((len & 0x7F) + 32);
	buf[2] = (unsigned char) (((len >> 7) & 0x7F) + 32);
	len += 3;
     }
   else len = 1;

   (void) bytecomp_write_data ((char *)buf, len);
}

int SLang_byte_compile_file (SLFUTURE_CONST char *name, int method)
{
   char file [1024];

   (void) method;
   if (strlen (name) + 2 >= sizeof (file))
     {
	_pSLang_verror (SL_INVALID_PARM, "Filename too long");
	return -1;
     }
   sprintf (file, "%sc", name);
   if (NULL == (Byte_Compile_Fp = fopen (file, "w")))
     {
	_pSLang_verror(SL_OBJ_NOPEN, "%s: unable to open", file);
	return -1;
     }

   Byte_Compile_Line_Len = 0;
   if (-1 != bytecomp_write_data (".#", 2))
     {
	_pSLcompile_ptr = byte_compile_token;
	(void) SLang_load_file (name);
	_pSLcompile_ptr = _pSLcompile;

	(void) bytecomp_write_data ("\n", 1);
     }

   if (EOF == fclose (Byte_Compile_Fp))
     SLang_set_error (SL_IO_WRITE_ERROR);

   if (_pSLang_Error)
     {
	_pSLang_verror (0, "Error processing %s", name);
	return -1;
     }
   return 0;
}

int SLang_generate_debug_info (int x)
{
#if SLANG_HAS_DEBUG_CODE
   /* int y = Default_Compile_Line_Num_Info; */
   /* Default_Compile_Line_Num_Info = x; */
   int y = 0;
   (void)x;
#if 0
   if (x == 0)
     Default_Compile_BOSEOS = 0;
   else
     Default_Compile_BOSEOS = 3;
#endif
   return y;
#else
   (void) x;
   return 0;
#endif
}

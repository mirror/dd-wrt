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

static SLang_Load_Type *LLT;

static void free_token (_pSLang_Token_Type *t)
{
   register unsigned int nrefs = t->num_refs;

   if (nrefs == 0)
     return;

   if (nrefs == 1)
     {
	if (t->free_val_func != NULL)
	  {
	     (*t->free_val_func)(t);
	     t->free_val_func = NULL;
	     t->v.s_val = NULL;
	  }
     }

   t->num_refs = nrefs - 1;
}

static void init_token (_pSLang_Token_Type *t)
{
   memset ((char *) t, 0, sizeof (_pSLang_Token_Type));
#if SLANG_HAS_DEBUG_CODE
   t->line_number = -1;
#endif
}

/* Allow room for one push back of a token.  This is necessary for
 * multiple assignment.
 */
static unsigned int Use_Next_Token;
static _pSLang_Token_Type Next_Token;
#if SLANG_HAS_DEBUG_CODE
static int Last_Line_Number = -1;
#endif

static int In_Looping_Context = 0;

static int unget_token (_pSLang_Token_Type *ctok)
{
   if (_pSLang_Error)
     return -1;
   if (Use_Next_Token != 0)
     {
	_pSLparse_error (SL_INTERNAL_ERROR, "unget_token failed", ctok, 0);
	return -1;
     }

   Use_Next_Token++;
   Next_Token = *ctok;
   init_token (ctok);
   return 0;
}

static int get_token (_pSLang_Token_Type *ctok)
{
   if (ctok->num_refs)
     free_token (ctok);

   if (Use_Next_Token)
     {
	Use_Next_Token--;
	*ctok = Next_Token;
	return ctok->type;
     }

   return _pSLget_token (ctok);
}

static int check_int_token_overflow (_pSLang_Token_Type *ctok, int sign)
{
   long ival, lval;
   SLtype stype;

   ctok->v.long_val = lval = sign * ctok->v.long_val;

   if (ctok->flags & (SLTOKEN_IS_HEX|SLTOKEN_IS_BINARY))
     return 0;

   switch (ctok->type)
     {
      case CHAR_TOKEN:
	stype = SLANG_CHAR_TYPE;
	ival = (long)(char) lval;
	break;
      case SHORT_TOKEN:
	stype = SLANG_SHORT_TYPE;
	ival = (long)(short) lval;
	break;
      case INT_TOKEN:
	stype = SLANG_INT_TYPE;
	ival = (long)(int) lval;
	break;
      case LONG_TOKEN:
	stype = LONG_TOKEN;
        ival = lval;
	break;

      default:
	return 0;
     }

   if (ival == lval)
     {
	if (((lval >= 0) && (sign > 0))
	    || ((lval <= 0) && (sign < 0)))
	  return 0;
     }
   SLang_verror (SL_SYNTAX_ERROR, "Literal integer constant is too large for %s", SLclass_get_datatype_name(stype));
   return -1;
}

static int check_uint_token_overflow (_pSLang_Token_Type *ctok, int sign)
{
   unsigned long ival, lval;
   SLtype stype;

   ctok->v.long_val = sign * ctok->v.long_val;
   lval = (unsigned long) ctok->v.long_val;

   switch (ctok->type)
     {
      case UCHAR_TOKEN:
	ival = (unsigned long)(unsigned char) lval;
	stype = SLANG_UCHAR_TYPE;
	break;
      case USHORT_TOKEN:
	stype = SLANG_USHORT_TYPE;
	ival = (unsigned long)(unsigned short) lval;
	break;
      case UINT_TOKEN:
	stype = SLANG_UINT_TYPE;
	ival = (unsigned long)(unsigned int) lval;
	break;
      case ULONG_TOKEN:
	stype = SLANG_ULONG_TYPE;
	ival = lval;
	break;
      default:
	return 0;
     }

   if (ival == lval)
     return 0;

   SLang_verror (SL_SYNTAX_ERROR, "Literal integer constant is too large for %s", SLclass_get_datatype_name(stype));
   return -1;
}

#ifdef HAVE_LONG_LONG
static int check_llong_token_overflow (_pSLang_Token_Type *ctok, int sign)
{
   long long lval = sign * ctok->v.llong_val;
   ctok->v.llong_val = lval;

   if ((ctok->flags & (SLTOKEN_IS_HEX|SLTOKEN_IS_BINARY))
       || ((lval >= 0) && (sign > 0))
       || ((lval <= 0) && (sign < 0)))
     return 0;

   SLang_verror (SL_SYNTAX_ERROR, "Literal integer constant is too large for %s", SLclass_get_datatype_name(SLANG_LLONG_TYPE));
   return -1;
}

static int check_ullong_token_overflow (_pSLang_Token_Type *ctok, int sign)
{
   ctok->v.ullong_val *= sign;
   return 0;
}
#endif

static int check_number_token_overflow (_pSLang_Token_Type *tok, int sign)
{
   tok->flags |= SLTOKEN_OVERFLOW_CHECKED;

   switch (tok->type)
     {
      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	return check_int_token_overflow (tok, sign);

      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
	return check_uint_token_overflow (tok, sign);

#ifdef HAVE_LONG_LONG
      case LLONG_TOKEN:
	return check_llong_token_overflow (tok, sign);

      case ULLONG_TOKEN:
	return check_ullong_token_overflow (tok, sign);
#endif
     }
   return 0;
}

static int compile_token (_pSLang_Token_Type *t)
{
#if SLANG_HAS_DEBUG_CODE
   if ((t->line_number != Last_Line_Number)
       && (t->line_number != -1))
     {
	_pSLang_Token_Type tok;
	tok.type = LINE_NUM_TOKEN;
	tok.v.long_val = Last_Line_Number = t->line_number;
	(*_pSLcompile_ptr) (&tok);
     }
#endif
   if ((t->flags & (SLTOKEN_TYPE_INTEGER|SLTOKEN_OVERFLOW_CHECKED)) == SLTOKEN_TYPE_INTEGER)
     {
	if (-1 == check_number_token_overflow (t, 1))
	  return -1;
     }
   (*_pSLcompile_ptr) (t);
   return 0;
}

typedef struct
{
#define USE_PARANOID_MAGIC	0
#if USE_PARANOID_MAGIC
   unsigned long magic;
#endif
   _pSLang_Token_Type *stack;
   unsigned int len;
   unsigned int size;
}
Token_List_Type;

#define MAX_TOKEN_LISTS 256
static Token_List_Type Token_List_Stack [MAX_TOKEN_LISTS];
static unsigned int Token_List_Stack_Depth = 0;
static Token_List_Type *Token_List = NULL;

static void init_token_list (Token_List_Type *t)
{
   t->size = 0;
   t->len = 0;
   t->stack = NULL;
#if USE_PARANOID_MAGIC
   t->magic = 0xABCDEF12;
#endif
}

static void free_token_list (Token_List_Type *t)
{
   _pSLang_Token_Type *s;

   if (t == NULL)
     return;
#if USE_PARANOID_MAGIC
   if (t->magic != 0xABCDEF12)
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "Token Magic number error.");
	return;
     }
#endif
   s = t->stack;
   if (s != NULL)
     {
	_pSLang_Token_Type *smax = s + t->len;
	while (s != smax)
	  {
	     if (s->num_refs) free_token (s);
	     s++;
	  }

	SLfree ((char *) t->stack);
     }

   memset ((char *) t, 0, sizeof (Token_List_Type));
}

static Token_List_Type *push_token_list (void)
{
   if (Token_List_Stack_Depth == MAX_TOKEN_LISTS)
     {
	_pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED,
			"Token list stack size exceeded", NULL, 0);
	return NULL;
     }

   Token_List = Token_List_Stack + Token_List_Stack_Depth;
   Token_List_Stack_Depth++;
   init_token_list (Token_List);
   return Token_List;
}

static int pop_token_list (int do_free)
{
   if (Token_List_Stack_Depth == 0)
     {
	if (_pSLang_Error == 0)
	  _pSLparse_error (SL_INTERNAL_ERROR, "Token list stack underflow", NULL, 0);
	return -1;
     }
   Token_List_Stack_Depth--;

   if (do_free) free_token_list (Token_List);

   if (Token_List_Stack_Depth != 0)
     Token_List = Token_List_Stack + (Token_List_Stack_Depth - 1);
   else
     Token_List = NULL;

   return 0;
}

static int check_token_list_space (Token_List_Type *t, unsigned int delta_size)
{
   _pSLang_Token_Type *st;
   unsigned int len;
#if USE_PARANOID_MAGIC
   if (t->magic != 0xABCDEF12)
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "Token Magic number error.");
	return -1;
     }
#endif
   len = t->len + delta_size;
   if (len <= t->size) return 0;

   if (delta_size < 4)
     {
	delta_size = 4;
	len = t->len + delta_size;
     }

   st = (_pSLang_Token_Type *) SLrealloc((char *) t->stack,
					len * sizeof(_pSLang_Token_Type));
   if (st == NULL)
     {
	_pSLparse_error (SL_MALLOC_ERROR, "Malloc error", NULL, 0);
	return -1;
     }

   memset ((char *) (st + t->len), 0, delta_size);

   t->stack = st;
   t->size = len;
   return 0;
}

static int append_token (_pSLang_Token_Type *t)
{
   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   Token_List->stack [Token_List->len] = *t;
   Token_List->len += 1;
   t->num_refs = 0;		       /* stealing it */
   return 0;
}

static int append_copy_of_string_token (_pSLang_Token_Type *t)
{
   _pSLang_Token_Type *t1;

   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   t1 = Token_List->stack + Token_List->len;
   *t1 = *t;

   if (t->v.s_val == NULL)
     return -1;

   if (EOF_TOKEN == _pSLtoken_init_slstring_token (t1, t->type, t->v.s_val, strlen (t->v.s_val)))
     return -1;

   t1->num_refs = 1;

   Token_List->len += 1;
   return 0;
}

static int append_int_as_token (int n)
{
   _pSLang_Token_Type num_tok;

   init_token (&num_tok);
   num_tok.type = INT_TOKEN;
   num_tok.flags |= SLTOKEN_TYPE_INTEGER|SLTOKEN_OVERFLOW_CHECKED;
   num_tok.v.long_val = n;
   return append_token (&num_tok);
}

#if 0
static int append_string_as_token (char *str)
{
   _pSLang_Token_Type *t;

   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   t = Token_List->stack + Token_List->len;
   init_token (t);

   if (EOF_TOKEN == _pSLtoken_init_slstring_token (t, STRING_TOKEN, str, strlen (str)))
     return -1;

   t->num_refs = 1;

   Token_List->len += 1;
   return 0;
}
#endif

static int append_token_of_type (unsigned char t)
{
   _pSLang_Token_Type *tok;

   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   /* The memset when the list was created ensures that the other fields
    * are properly initialized.
    */
#if 0
   if ((t == CHS_TOKEN) && Token_List->len)
     {
	tok = Token_List->stack + (Token_List->len-1);
	if (IS_INTEGER_TOKEN(tok->type))
	  {
	     tok->v.long_val = -tok->v.long_val;
	     return 0;
	  }
#ifdef HAVE_LONG_LONG
	if ((tok->type == LLONG_TOKEN) || (tok->type == ULLONG_TOKEN))
	  {
	     tok->v.llong_val = -tok->v.llong_val;
	     return 0;
	  }
#endif
     }
#endif

   tok = Token_List->stack + Token_List->len;
   init_token (tok);
   tok->type = t;
   Token_List->len += 1;
   return 0;
}

static _pSLang_Token_Type *get_last_token (void)
{
   unsigned int len;

   if ((Token_List == NULL)
       || (0 == (len = Token_List->len)))
     return NULL;

   len--;
   return Token_List->stack + len;
}

/* This function does NOT free the list. */
static int compile_token_list_with_fun (int dir, Token_List_Type *list,
					int (*f)(_pSLang_Token_Type *))
{
   _pSLang_Token_Type *t0, *t1;

   if (list == NULL)
     return -1;

   if (f == NULL)
     f = compile_token;

   t0 = list->stack;
   t1 = t0 + list->len;

   if (dir < 0)
     {
	/* backwards */

	while ((_pSLang_Error == 0) && (t1 > t0))
	  {
	     t1--;
	     (*f) (t1);
	  }
	return 0;
     }

   /* forward */
   while ((_pSLang_Error == 0) && (t0 < t1))
     {
	(*f) (t0);
	t0++;
     }
   return 0;
}

static int compile_token_list (void)
{
   if (Token_List == NULL)
     return -1;

   compile_token_list_with_fun (1, Token_List, NULL);
   pop_token_list (1);
   return 0;
}

/* Take all elements in the list from pos2 to the end and exchange them
 * with the elements at pos1, e.g.,
 * ...ABCDEabc ==> ...abcABCDE
 * where pos1 denotes A and pos2 denotes a.
 *
 * NOTE: The caller must make special provisions for NO_OP_LITERAL tokens.
 */
static int token_list_element_exchange (unsigned int pos1, unsigned int pos2)
{
   _pSLang_Token_Type *s, *s1, *s2;
   unsigned int len, nloops;

   if (Token_List == NULL)
     return -1;

   s = Token_List->stack;
   len = Token_List->len;

   if ((s == NULL) || (len == 0)
       || (pos2 >= len))
     return -1;

   if (pos1 > pos2)
     {
	SLang_verror (SL_INTERNAL_ERROR, "pos1<pos2 in token_list_element_exchange");
	return -1;
     }

   /* This may not be the most efficient algorithm but the number to swap
    * is most-likely going to be small, e.g, 3
    * The algorithm is to rotate the list.  The particular rotation
    * direction was chosen to make insert_token fast.
    * It works like:
    * @ ABCabcde --> BCabcdeA --> CabcdeAB -->  abcdefAB
    * which is optimal for Abcdef sequence produced by function calls.
    *
    * Profiling indicates that nloops is almost always 1, whereas the inner
    * loop can loop many times (e.g., 9 times).
    */

   s2 = s + (len - 1);
   s1 = s + pos1;
   nloops = pos2 - pos1;

   while (nloops)
     {
	_pSLang_Token_Type save;

	s = s1;
	save = *s;

	while (s < s2)
	  {
	     *s = *(s + 1);
	     s++;
	  }
	*s = save;

	nloops--;
     }
   return 0;
}

#if 0
static int insert_token (_pSLang_Token_Type *t, unsigned int pos)
{
   if (-1 == append_token (t))
     return -1;

   return token_list_element_exchange (pos, Token_List->len - 1);
}
#endif
static void compile_token_of_type (unsigned char t)
{
   _pSLang_Token_Type tok;

#if SLANG_HAS_DEBUG_CODE
   tok.line_number = -1;
#endif
   tok.flags = 0;
   tok.type = t;
   compile_token(&tok);
}

#if SLANG_HAS_BOSEOS
static void compile_eos (void)
{
   /* This is commented out to ensure that if bos was compiled, the eos will be.
    *
    * if (0 == (_pSLang_Compile_Line_Num_Info & SLANG_BOSEOS_MASK))
    * return;
    */
   compile_token_of_type (EOS_TOKEN);
}

static int compile_bos (_pSLang_Token_Type *t, int level)
{
   _pSLang_Token_Type tok;

   if (level > (_pSLang_Compile_BOSEOS & SLANG_BOSEOS_VALUE_BITS))
     return 0;

   tok.type = BOS_TOKEN;
   tok.v.long_val = t->line_number;
   (*_pSLcompile_ptr) (&tok);
   return 1;
}
static void append_eos (void)
{
   /* This is commented out to ensure that if bos was compiled, the eos will be.
    *
    * if (0 == (_pSLang_Compile_Line_Num_Info & SLANG_BOSEOS_MASK))
    * return;
    */
   append_token_of_type (EOS_TOKEN);
}

static int append_bos (_pSLang_Token_Type *t, int level)
{
   _pSLang_Token_Type tok;

   if (level > (_pSLang_Compile_BOSEOS & SLANG_BOSEOS_VALUE_BITS))
     return 0;

   init_token (&tok);
   tok.type = BOS_TOKEN;
   tok.v.long_val = t->line_number;

   append_token (&tok);
   free_token (&tok);
   return 1;
}
#endif

static void statement (_pSLang_Token_Type *);
static void loop_statement (_pSLang_Token_Type *);
static void compound_statement (_pSLang_Token_Type *);
static void expression_with_parenthesis (_pSLang_Token_Type *);
static void handle_semicolon (_pSLang_Token_Type *);
static void handle_for_statement (_pSLang_Token_Type *);
static void handle_foreach_statement (_pSLang_Token_Type *);
static void statement_list (_pSLang_Token_Type *);
static void variable_list (_pSLang_Token_Type *, unsigned char);
static void struct_declaration (_pSLang_Token_Type *, int);
static void define_function_args (_pSLang_Token_Type *);
static void typedef_definition (_pSLang_Token_Type *);
static void function_args_expression (_pSLang_Token_Type *, int, int, int, unsigned int *);
static void expression (_pSLang_Token_Type *);
static void expression_with_commas (_pSLang_Token_Type *, int);
static void simple_expression (_pSLang_Token_Type *);
static void unary_expression (_pSLang_Token_Type *);
static void postfix_expression (_pSLang_Token_Type *);
static int check_for_lvalue (unsigned char, _pSLang_Token_Type *);
/* static void primary_expression (_pSLang_Token_Type *); */
static void block (_pSLang_Token_Type *);
static void loop_block (_pSLang_Token_Type *);
static void inline_array_expression (_pSLang_Token_Type *);
static void inline_list_expression (_pSLang_Token_Type *);
static void array_index_expression (_pSLang_Token_Type *);
static void do_multiple_assignment (_pSLang_Token_Type *);
static void try_multiple_assignment (_pSLang_Token_Type *);
static void handle_try_statement (_pSLang_Token_Type *);
static void handle_throw_statement (_pSLang_Token_Type *);

#if 0
static void not_implemented (char *what)
{
   char err [256];
   sprintf (err, "Expression not implemented: %s", what);
   _pSLparse_error (SL_NOT_IMPLEMENTED, err, NULL, 0);
}
#endif
static void rpn_parse_line (_pSLang_Token_Type *tok)
{
   do
     {
	  /* multiple RPN tokens possible when the file looks like:
	   * . <end of line>
	   * . <end of line>
	   */
	if (tok->type != RPN_TOKEN)
	  compile_token (tok);
	free_token (tok);
     }
   while (EOF_TOKEN != _pSLget_rpn_token (tok));
}

static int get_identifier_token (_pSLang_Token_Type *tok)
{
   if (IDENT_TOKEN == get_token (tok))
     return IDENT_TOKEN;

   _pSLparse_error (SL_SYNTAX_ERROR, "Expecting identifier", tok, 0);
   return tok->type;
}

static void define_function (_pSLang_Token_Type *ctok, unsigned char type)
{
   _pSLang_Token_Type fname;

   switch (type)
     {
      case STATIC_TOKEN:
	type = DEFINE_STATIC_TOKEN;
	break;

      case PUBLIC_TOKEN:
	type = DEFINE_PUBLIC_TOKEN;
	break;

      case PRIVATE_TOKEN:
	type = DEFINE_PRIVATE_TOKEN;
     }

   init_token (&fname);
   if (IDENT_TOKEN != get_identifier_token (&fname))
     {
	free_token (&fname);
	return;
     }

   compile_token_of_type(OPAREN_TOKEN);
   get_token (ctok);
   define_function_args (ctok);
   compile_token_of_type(FARG_TOKEN);

   if (ctok->type == OBRACE_TOKEN)
     {
	int loop_context = In_Looping_Context;
	In_Looping_Context = 0;
	compound_statement(ctok);
	In_Looping_Context = loop_context;
     }

   else if (ctok->type != SEMICOLON_TOKEN)
     {
	_pSLparse_error(SL_SYNTAX_ERROR, "Expecting {", ctok, 0);
	free_token (&fname);
	return;
     }

   fname.type = type;
   compile_token (&fname);
   free_token (&fname);
}

/* This is called from "statement", which is expected to return no
 * new token.
 */
static int check_for_loop_then_else (_pSLang_Token_Type *ctok)
{
   int b = 0;

   get_token (ctok);

   while (1)
     {
	_pSLtok_Type type = ctok->type;
#ifdef LOOP_ELSE_TOKEN
	if ((type == ELSE_TOKEN) && ((b & 1) == 0))
	  {
	     get_token (ctok);
	     block (ctok);
	     compile_token_of_type (LOOP_ELSE_TOKEN);
	     get_token (ctok);
	     b |= 1;
	     continue;
	  }
#endif
	if ((type == THEN_TOKEN) && ((b & 2) == 0))
	  {
	     get_token (ctok);
	     block (ctok);
	     compile_token_of_type (LOOP_THEN_TOKEN);
	     b |= 2;
#ifdef LOOP_ELSE_TOKEN
	     get_token (ctok);
	     continue;
#else
	     return b;
#endif
	  }
	break;
     }
   unget_token (ctok);
   return b;
}

/* statement:
 *	 compound-statement
 *	 if ( expression ) statement
 *	 if ( expression ) statement else statement
 *	 !if ( expression ) statement
 *	 loop ( expression ) statement
 *	 _for ( expression ) statement
 *       foreach ( expression ) statement
 *       foreach (expression ) using (expression-list) statement
 *	 while ( expression ) statement
 *	 do statement while (expression) ;
 *	 for ( expressionopt ; expressionopt ; expressionopt ) statement
 *	 ERROR_BLOCK statement
 *       try statement rest-of-try-statement
 *       throw exception, object
 *	 EXIT_BLOCK statement
 *	 USER_BLOCK0 statement
 *	 USER_BLOCK1 statement
 *	 USER_BLOCK2 statement
 *	 USER_BLOCK3 statement
 *	 USER_BLOCK4 statement
 *	 forever statement
 *	 break ;
 *	 continue ;
 *	 return expressionopt ;
 *	 variable variable-list ;
 *	 struct struct-decl ;
 *	 define identifier function-args ;
 *	 define identifier function-args compound-statement
 *	 switch ( expression ) statement
 *	 rpn-line
 *	 at-line
 *	 push ( expression )
 *	 ( expression ) = expression ;
 *	 expression ;
 *       expression :
 */

/* Note: This function does not return with a new token.  It is up to the
 * calling routine to handle that.
 */
static void statement (_pSLang_Token_Type *ctok)
{
   unsigned char type;
#if SLANG_HAS_BOSEOS
   int eos;
#endif
   if (_pSLang_Error)
     return;

   LLT->parse_level += 1;

   switch (ctok->type)
     {
      case OBRACE_TOKEN:
	compound_statement (ctok);
	break;

      case IF_TOKEN:
      case IFNOT_TOKEN:
	type = ctok->type;
	get_token (ctok);
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 2);
#endif
	expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif
	block (ctok);

	if (ELSE_TOKEN != get_token (ctok))
	  {
	     compile_token_of_type (type);
	     unget_token (ctok);
	     break;
	  }
	get_token (ctok);
	block (ctok);
	if (type == IF_TOKEN) type = ELSE_TOKEN; else type = NOTELSE_TOKEN;
	compile_token_of_type (type);
	break;

      /* case IFNOT_TOKEN: */
      case _FOR_TOKEN:
	get_token (ctok);
	handle_for_statement (ctok);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case LOOP_TOKEN:
	get_token (ctok);
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 2);
#endif
	expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif
	loop_block (ctok);
	compile_token_of_type (LOOP_TOKEN);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case FOREACH_TOKEN:
	get_token (ctok);
	handle_foreach_statement (ctok);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case WHILE_TOKEN:
	get_token (ctok);
	compile_token_of_type (OBRACE_TOKEN);
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 2);
#endif
	expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif
	compile_token_of_type (CBRACE_TOKEN);
	loop_block (ctok);
	compile_token_of_type (WHILE_TOKEN);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case DO_TOKEN:
	get_token (ctok);
	loop_block (ctok);

	if (WHILE_TOKEN != get_token (ctok))
	  {
	     _pSLparse_error(SL_SYNTAX_ERROR, "Expecting while", ctok, 0);
	     break;
	  }

	get_token (ctok);

	compile_token_of_type (OBRACE_TOKEN);
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 2);
#endif
	expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif
	compile_token_of_type (CBRACE_TOKEN);
	compile_token_of_type (DOWHILE_TOKEN);
	handle_semicolon (ctok);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case FOR_TOKEN:
	/* Look for (exp_opt ; exp_opt ; exp_opt ) */

	if (OPAREN_TOKEN != get_token (ctok))
	  {
	     _pSLparse_error(SL_SYNTAX_ERROR, "Expecting (.", ctok, 0);
	     break;
	  }

	if (NULL == push_token_list ())
	  break;

	append_token_of_type (OBRACE_TOKEN);
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
#if SLANG_HAS_BOSEOS
	     eos = append_bos (ctok, 2);
#endif
	     expression (ctok);
#if SLANG_HAS_BOSEOS
	     if (eos) append_eos ();
#endif
	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _pSLparse_error(SL_SYNTAX_ERROR, "Expecting ;", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	append_token_of_type (OBRACE_TOKEN);
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
#if SLANG_HAS_BOSEOS
	     eos = append_bos (ctok, 2);
#endif
	     expression (ctok);
#if SLANG_HAS_BOSEOS
	     if (eos) append_eos ();
#endif
	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _pSLparse_error(SL_SYNTAX_ERROR, "Expecting ;", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	append_token_of_type (OBRACE_TOKEN);
	if (CPAREN_TOKEN != get_token (ctok))
	  {
#if SLANG_HAS_BOSEOS
	     eos = append_bos (ctok, 2);
#endif
	     expression (ctok);
#if SLANG_HAS_BOSEOS
	     if (eos) append_eos ();
#endif
	     if (ctok->type != CPAREN_TOKEN)
	       {
		  _pSLparse_error(SL_SYNTAX_ERROR, "Expecting ).", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	compile_token_list ();

	get_token (ctok);
	loop_block (ctok);
	compile_token_of_type (FOR_TOKEN);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case FOREVER_TOKEN:
	get_token (ctok);
	loop_block (ctok);
	compile_token_of_type (FOREVER_TOKEN);
	if (0 == check_for_loop_then_else (ctok))
	  compile_token_of_type (NOP_TOKEN);
	break;

      case ERRBLK_TOKEN:
      case EXITBLK_TOKEN:
      case USRBLK0_TOKEN:
      case USRBLK1_TOKEN:
      case USRBLK2_TOKEN:
      case USRBLK3_TOKEN:
      case USRBLK4_TOKEN:
	type = ctok->type;
	get_token (ctok);
	block (ctok);
	compile_token_of_type (type);
	break;

      case BREAK_TOKEN:
      case CONT_TOKEN:
	if (In_Looping_Context == 0)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "`break' or `continue' requires a looping context", ctok, 0);
	     break;
	  }

	type = ctok->type;
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 3);
#endif
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
	     if ((ctok->type != INT_TOKEN)
		 || (ctok->v.long_val <= 0))
	       {
		  _pSLparse_error (SL_SYNTAX_ERROR, "Expecting a positive non-zero integer or a semi-colon.", ctok, 0);
		  break;
	       }
	     ctok->type = (type == BREAK_TOKEN) ? BREAK_N_TOKEN : CONT_N_TOKEN;
	     compile_token (ctok);
	     get_token (ctok);
	     handle_semicolon (ctok);
	  }
	else compile_token_of_type (type);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif
	break;

      case RETURN_TOKEN:
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
	     if (NULL == push_token_list ())
	       break;

#if SLANG_HAS_BOSEOS
	     eos = append_bos (ctok, 3);
#endif
	     expression (ctok);
#if SLANG_HAS_BOSEOS
	     if (eos) append_eos ();
#endif

	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _pSLparse_error (SL_SYNTAX_ERROR, "Expecting ;", ctok, 0);
		  break;
	       }
	     compile_token_list ();
	  }
	compile_token_of_type (RETURN_TOKEN);
	handle_semicolon (ctok);
	break;

      case STATIC_TOKEN:
      case PRIVATE_TOKEN:
      case PUBLIC_TOKEN:
	type = ctok->type;
	get_token (ctok);
	if (ctok->type == VARIABLE_TOKEN)
	  {
	     get_token (ctok);
	     variable_list (ctok, type);
	     handle_semicolon (ctok);
	     break;
	  }
	if (ctok->type == DEFINE_TOKEN)
	  {
	     define_function (ctok, type);
	     break;
	  }
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting 'variable' or 'define'", ctok, 0);
	break;

      case VARIABLE_TOKEN:
	get_token (ctok);
	variable_list (ctok, OBRACKET_TOKEN);
	handle_semicolon (ctok);
	break;

      case TYPEDEF_TOKEN:
	get_token (ctok);
	if (NULL == push_token_list ())
	  break;
	typedef_definition (ctok);
	compile_token_list ();

	handle_semicolon (ctok);
	break;

      case DEFINE_TOKEN:
	define_function (ctok, DEFINE_TOKEN);
	break;

      case TRY_TOKEN:
	get_token (ctok);
	handle_try_statement (ctok);
	break;

      case THROW_TOKEN:
	get_token (ctok);
	handle_throw_statement (ctok);
	break;

      case SWITCH_TOKEN:
	get_token (ctok);
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 2);
#endif
	expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif

	while ((_pSLang_Error == 0)
	       && (OBRACE_TOKEN == ctok->type))
	  {
	     compile_token_of_type (OBRACE_TOKEN);
	     compound_statement (ctok);
	     compile_token_of_type (CBRACE_TOKEN);
	     get_token (ctok);
	  }
	compile_token_of_type (SWITCH_TOKEN);
	unget_token (ctok);
	break;

      case EOF_TOKEN:
	break;
#if 0
      case PUSH_TOKEN:
	get_token (ctok);
	expression_list_with_parenthesis (ctok);
	handle_semicolon (ctok);
	break;
#endif

      case SEMICOLON_TOKEN:
#if SLANG_HAS_BOSEOS
	eos = compile_bos (ctok, 3);
	if (eos) compile_eos ();
#endif
	handle_semicolon (ctok);
	break;

      case RPN_TOKEN:
	if (POUND_TOKEN == get_token (ctok))
	  _pSLcompile_byte_compiled ();
	else if (ctok->type != EOF_TOKEN)
	  rpn_parse_line (ctok);
	break;

      case OPAREN_TOKEN:	       /* multiple assignment */
#if SLANG_HAS_BOSEOS
	eos = compile_bos(ctok, 1);
#endif
	try_multiple_assignment (ctok);
	if (ctok->type == COLON_TOKEN)
	  compile_token_of_type (COLON_TOKEN);
	else handle_semicolon (ctok);
#if SLANG_HAS_BOSEOS
	if (eos) compile_eos();
#endif
	break;

      default:

#if SLANG_HAS_BOSEOS
	eos = compile_bos(ctok, ctok->type != CASE_TOKEN ? 1 : 2);
#endif
	if (NULL == push_token_list ())
	  break;

	expression (ctok);
	compile_token_list ();

#if SLANG_HAS_BOSEOS
	if (eos) compile_eos ();
#endif

	if (ctok->type == COLON_TOKEN)
	  compile_token_of_type (COLON_TOKEN);
	else handle_semicolon (ctok);
	break;
     }

   LLT->parse_level -= 1;
}

static void loop_statement (_pSLang_Token_Type *ctok)
{
   In_Looping_Context++;
   statement (ctok);
   In_Looping_Context--;
}

/* This function does not return a new token.  Calling routine must do that */
static void block (_pSLang_Token_Type *ctok)
{
   compile_token_of_type (OBRACE_TOKEN);
   statement (ctok);
   compile_token_of_type (CBRACE_TOKEN);
}

static void loop_block (_pSLang_Token_Type *ctok)
{
   compile_token_of_type (OBRACE_TOKEN);
   loop_statement (ctok);
   compile_token_of_type (CBRACE_TOKEN);
}

static void handle_for_statement (_pSLang_Token_Type *ctok)
{
   _pSLang_Token_Type tok_buf;
   _pSLang_Token_Type *ident_token = NULL;
#if SLANG_HAS_BOSEOS
   int eos;
#endif
   if (ctok->type == IDENT_TOKEN)
     {
	tok_buf = *ctok;
	ident_token = &tok_buf;
	init_token (ctok);
	get_token (ctok);
     }
#if SLANG_HAS_BOSEOS
   eos = compile_bos(ctok, 2);
#endif
   expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
   if (eos) compile_eos ();
#endif

   compile_token_of_type (OBRACE_TOKEN);
   if (ident_token != NULL)
     {
	ident_token->type = _SCALAR_ASSIGN_TOKEN;
	compile_token (ident_token);
	free_token (ident_token);
     }
   loop_statement (ctok);
   compile_token_of_type (CBRACE_TOKEN);

   compile_token_of_type (_FOR_TOKEN);
}

static int init_identifier_token (_pSLang_Token_Type *t, SLFUTURE_CONST char *name)
{
   init_token (t);

   if (EOF_TOKEN == _pSLtoken_init_slstring_token (t, IDENT_TOKEN, name, strlen(name)))
     return -1;

   return 0;
}

/* The try-statement looks like:
 *
 *    TRY ev_optional catch_blocks_opt finally_block_opt
 *
 *    ev_optional:
 *       nil
 *       ( e , v)
 *
 *    catch_blocks_opt:
 *       catch_blocks_opt catch_block
 *    catch_block:
 *       CATCH exception-list : block
 *       CATCH exception-list ;
 *    exception-list:
 *       expression
 *       exception-list , exception
 *
 *    finally_block_opt:
 *       FINALLY[:] block
 *
 * The above gets compiled into the form:
 *
 *    {ev_block} {try-statements}
 *       {exception-list}{catch-block}
 *               .
 *               .
 *       {exception-list}{catch-block}
 *    {finally_block} try
 */

static void handle_try_statement (_pSLang_Token_Type *ctok)
{
   int num_catches;

   /* start ev_block */
   if (NULL == push_token_list ())
     return;

   append_token_of_type (OBRACE_TOKEN);
   if (ctok->type == OPAREN_TOKEN)
     {
	_pSLang_Token_Type e;
	if (-1 == init_identifier_token (&e, "__get_exception_info"))
	  return;

	append_token (&e);
	free_token (&e);

	get_token (ctok);
	postfix_expression (ctok);
	check_for_lvalue (ASSIGN_TOKEN, NULL);

	if (ctok->type != CPAREN_TOKEN)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting )", ctok, 0);
	     return;
	  }
	get_token (ctok);
     }
   append_token_of_type (CBRACE_TOKEN);
   compile_token_list ();

   /* Now the try block itself */
   block (ctok);

   /* Now the various catch blocks */
   num_catches = 0;
   while (CATCH_TOKEN == get_token (ctok))
     {
	/* Expecting catch expression-list: */
	compile_token_of_type (OBRACE_TOKEN);
	get_token (ctok);

	push_token_list ();

	while (_pSLang_Error == 0)
	  {
	     if (ctok->type == COLON_TOKEN)
	       break;

	     simple_expression (ctok);
	     if (ctok->type != COMMA_TOKEN)
	       break;
	     get_token (ctok);
	  }

	if (ctok->type == COLON_TOKEN)
	  get_token (ctok);
	else if (ctok->type != SEMICOLON_TOKEN)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting a colon to end the exception list", ctok, 0);
	     return;
	  }
	compile_token_list ();

	compile_token_of_type (CBRACE_TOKEN);

	/* catch block */
	block (ctok);
	num_catches++;
     }

   if ((num_catches == 0)
       && (ctok->type != FINALLY_TOKEN))
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting \"catch\" or \"finally\"", ctok, 0);
	return;
     }

   /* finally */
   if (ctok->type == FINALLY_TOKEN)
     {
	get_token (ctok);
	if (ctok->type == COLON_TOKEN)
	  get_token (ctok);
	block (ctok);
     }
   else
     {
	/* since this is called directly from statement, we need to get only
	 * tokens that were used.  So, since we are not using this, put it back.
	 */
	unget_token (ctok);
	compile_token_of_type (OBRACE_TOKEN);
	compile_token_of_type (CBRACE_TOKEN);
     }
   compile_token_of_type (TRY_TOKEN);
}

/*
 * throw-statement:
 *   throw ;
 *   throw exception_expr ;
 *   throw exception_expr , simple_expression
 *   throw exception_expr , simple_expression ;
 *   throw exception_expr , simple_expression, simple_expression ;
 */
static void handle_throw_statement (_pSLang_Token_Type *ctok)
{
#if SLANG_HAS_BOSEOS
   int eos;
#endif

   push_token_list ();

   if (ctok->type == SEMICOLON_TOKEN)
     append_token_of_type (ARG_TOKEN);
   else
     {
#if SLANG_HAS_BOSEOS
	eos = append_bos (ctok, 2);
#endif
	append_token_of_type (ARG_TOKEN);
	simple_expression (ctok);
	if (ctok->type == COMMA_TOKEN)
	  {
	     get_token (ctok);
	     simple_expression (ctok);
	  }
	if (ctok->type == COMMA_TOKEN)
	  {
	     get_token (ctok);
	     simple_expression (ctok);
	  }
	handle_semicolon (ctok);
   /* not-necessary -- append_token_of_type (EARG_TOKEN); */
#if SLANG_HAS_BOSEOS
	if (eos) append_eos ();
#endif
     }
   compile_token_list ();
   compile_token_of_type (THROW_TOKEN);
}

static _pSLang_Token_Type *allocate_token (void)
{
   _pSLang_Token_Type *v;

   v = (_pSLang_Token_Type *)SLmalloc (sizeof (_pSLang_Token_Type));
   if (v == NULL)
     return NULL;

   init_token (v);
   return v;
}

static void handle_foreach_statement (_pSLang_Token_Type *ctok)
{
   _pSLang_Token_Type *var_tokens = NULL;
   _pSLang_Token_Type *v;
#if SLANG_HAS_BOSEOS
   int eos;
#endif

#if SLANG_HAS_BOSEOS
   eos = compile_bos (ctok, 2);
#endif
   /* I may want to make this a separate routine */
   while (ctok->type == IDENT_TOKEN)
     {
	v = (_pSLang_Token_Type *)SLmalloc (sizeof (_pSLang_Token_Type));
	if (v == NULL)
	  goto free_return;
	init_token (v);
	*v = *ctok;
	init_token (ctok);
	v->next = var_tokens;
	var_tokens = v;
	get_token (ctok);
	if (ctok->type != COMMA_TOKEN)
	  break;
	get_token (ctok);
     }

   expression_with_parenthesis (ctok);
#if SLANG_HAS_BOSEOS
   if (eos) compile_eos ();
#endif

   if (NULL == push_token_list ())
     goto free_return;

   append_token_of_type (ARG_TOKEN);
   if (ctok->type == USING_TOKEN)
     {
	if (OPAREN_TOKEN != get_token (ctok))
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expected 'using ('", ctok, 0);
	     goto free_return;
	  }
	get_token (ctok);
	function_args_expression (ctok, 0, 0, 0, NULL);
     }
   /* append_token_of_type (EARG_TOKEN); // This is now handled by the
    * FOREACH_TOKEN itself.  Doing it presents a problem if a hook gets
    * called when the loop_statements are being parsed.  This can and will
    * happen when slsh calls its massage_input hook.  The would result in
    * bytecode such as:
    *  __args <using-expression> __eargs __args <hookargs> __eargs hook
    * As a result, the first __args/__eargs info will be lost.  By allowing
    * __foreach__ to handle it, we effectively get the proper nested
    * __args/__eargs form:
    *  __args <using-expression> __args <hookargs> __eargs hook __eargs __foreach
    *
    * Yes, this is subtle.  Always follow the rule:  Avoid __args/__eargs
    * outside a token list and make sure the code that uses it is either also
    * in the same token list, or implicitely calls __eargs.
    */

   compile_token_list ();

   compile_token_of_type (OBRACE_TOKEN);

   v = var_tokens;
   while (v != NULL)
     {
	v->type = _SCALAR_ASSIGN_TOKEN;
	compile_token (v);
	v = v->next;
     }

   loop_statement (ctok);

   compile_token_of_type (CBRACE_TOKEN);
   compile_token_of_type (FOREACH_EARGS_TOKEN);

   free_return:
   while (var_tokens != NULL)
     {
	v = var_tokens->next;
	free_token (var_tokens);
	SLfree ((char *) var_tokens);
	var_tokens = v;
     }
}

/*
 * statement-list:
 *	 statement
 *	 statement-list statement
 */
static void statement_list (_pSLang_Token_Type *ctok)
{
   while ((_pSLang_Error == 0)
	  && (ctok->type != CBRACE_TOKEN)
	  && (ctok->type != EOF_TOKEN))
     {
	statement(ctok);
	get_token (ctok);
     }
}

/* compound-statement:
 *	 { statement-list }
 */
static void compound_statement (_pSLang_Token_Type *ctok)
{
   /* ctok->type is OBRACE_TOKEN here */
   get_token (ctok);
   statement_list(ctok);
   if (CBRACE_TOKEN != ctok->type)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting '}'", ctok, 0);
	return;
     }
}

/* This function is only called from statement. */
static void expression_with_parenthesis (_pSLang_Token_Type *ctok)
{
   if (ctok->type != OPAREN_TOKEN)
     {
	_pSLparse_error(SL_SYNTAX_ERROR, "Expecting (", ctok, 0);
	return;
     }

   if (NULL == push_token_list ())
     return;

   get_token (ctok);
   expression (ctok);

   if (ctok->type != CPAREN_TOKEN)
     _pSLparse_error(SL_SYNTAX_ERROR, "Expecting )", ctok, 0);

   compile_token_list ();

   get_token (ctok);
}

static void handle_semicolon (_pSLang_Token_Type *ctok)
{
   if ((ctok->type == SEMICOLON_TOKEN)
       || (ctok->type == EOF_TOKEN))
     return;

   _pSLparse_error (SL_SYNTAX_ERROR, "Expecting ;", ctok, 0);
}

void _pSLparse_start (SLang_Load_Type *llt)
{
   _pSLang_Token_Type ctok;
   SLang_Load_Type *save_llt;
   unsigned int save_use_next_token;
   _pSLang_Token_Type save_next_token;
   Token_List_Type *save_list;
   int save_looping_context = In_Looping_Context;
#if SLANG_HAS_DEBUG_CODE
   int save_last_line_number = Last_Line_Number;

   Last_Line_Number = -1;
#endif
   save_use_next_token = Use_Next_Token;
   save_next_token = Next_Token;
   save_list = Token_List;
   save_llt = LLT;
   LLT = llt;

   init_token (&Next_Token);
   Use_Next_Token = 0;
   In_Looping_Context = 0;
   init_token (&ctok);
   get_token (&ctok);

   llt->parse_level = 0;
   statement_list (&ctok);

   if (_pSLang_Error == 0)
     {
	if (ctok.type != EOF_TOKEN)
	  _pSLparse_error (SL_SYNTAX_ERROR, "Parse ended prematurely", &ctok, 0);
	else
	  compile_token_of_type (EOF_TOKEN);
     }

   if (_pSLang_Error)
     {
	if (_pSLang_Error < 0)	       /* severe error */
	  save_list = NULL;

	while (Token_List != save_list)
	  {
	     if (-1 == pop_token_list (1))
	       break;		       /* ??? when would this happen? */
	  }
     }

   free_token (&ctok);
   LLT = save_llt;
   if (Use_Next_Token)
     free_token (&Next_Token);
   Use_Next_Token = save_use_next_token;
   Next_Token = save_next_token;
   In_Looping_Context = save_looping_context;

#if SLANG_HAS_DEBUG_CODE
   Last_Line_Number = save_last_line_number;
#endif
}

/* variable-list:
 * 	variable-decl
 * 	variable-decl variable-list
 *
 * variable-decl:
 * 	identifier
 * 	identifier = simple-expression
 */
static void variable_list (_pSLang_Token_Type *name_token, unsigned char variable_type)
{
   int declaring;
   _pSLang_Token_Type tok;

   if (name_token->type != IDENT_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting a variable name", name_token, 0);
	return;
     }

   declaring = 0;
   do
     {
	if (declaring == 0)
	  {
	     declaring = 1;
	     compile_token_of_type (variable_type);
	  }

	compile_token (name_token);

	init_token (&tok);
	if (ASSIGN_TOKEN == get_token (&tok))
	  {
#if SLANG_HAS_BOSEOS
	     int eos;
#endif
	     compile_token_of_type (CBRACKET_TOKEN);
	     declaring = 0;

	     get_token (&tok);
#if SLANG_HAS_BOSEOS
	     eos = compile_bos (&tok, 1);
#endif
	     push_token_list ();
	     simple_expression (&tok);
	     compile_token_list ();

	     name_token->type = _SCALAR_ASSIGN_TOKEN;
	     compile_token (name_token);
#if SLANG_HAS_BOSEOS
	     if (eos) compile_eos ();
#endif
	  }

	free_token (name_token);
	*name_token = tok;
     }
   while ((name_token->type == COMMA_TOKEN)
	  && (IDENT_TOKEN == get_token (name_token)));

   if (declaring) compile_token_of_type (CBRACKET_TOKEN);
}

static void free_token_linked_list (_pSLang_Token_Type *tok)
{
   while (tok != NULL)
     {
	_pSLang_Token_Type *next = tok->next;
	free_token (tok);
	if (tok->num_refs != 0)
	  {
	     _pSLang_verror (SL_INTERNAL_ERROR, "Cannot free token in linked list");
	  }
	else
	  SLfree ((char *) tok);

	tok = next;
     }
}

/* This works with any string-like token */
static int prefix_token_sval_field (_pSLang_Token_Type *tok, char *prefix)
{
   char buf[2*SL_MAX_TOKEN_LEN];
   unsigned int len, prefix_len;

   prefix_len = strlen (prefix);
   len = _pSLstring_bytelen (tok->v.s_val);   /* sign */
   if (len + prefix_len >= sizeof(buf))
     {
	_pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Number too long for buffer", tok, 1);
	return -1;
     }
   memcpy (buf, prefix, prefix_len);
   memcpy (buf+prefix_len, tok->v.s_val, len);  /* copys \0 */
   (*tok->free_val_func)(tok);
   if (EOF_TOKEN == _pSLtoken_init_slstring_token (tok, tok->type, buf, len+prefix_len))
     return -1;

   return 0;
}

/*
 * This function parses a structure definition block.  It returns the names
 * of the structure fields in the form of a linked list of tokens.
 *
 * If the structure contains assignments, the function parses the expressions and returns the
 * number of such assignments.  So:
 *
 *  foo = foo_expr, bar = bar_expr, ...
 *
 * generates:  foo_expr_tokens "foo" bar_expr_tokens "bar" ...
 */
static _pSLang_Token_Type *
  handle_struct_assign_list (_pSLang_Token_Type *ctok, int assign_ok, unsigned int *nassignp)
{
   _pSLang_Token_Type *name_list_root = NULL;
   _pSLang_Token_Type *name_list_tail = NULL;
   unsigned int n, m;
   char buf[64];

   n = m = 0;
   while (_pSLang_Error == 0)
     {
	_pSLang_Token_Type *new_tok;
	int is_deref = 0;

	if (assign_ok && (ctok->type == DEREF_TOKEN))
	  {
	     /* struct { @ expr, ... } */

	     (void) SLsnprintf (buf, sizeof(buf), "@%d", n);
	     free_token (ctok);
	     if (EOF_TOKEN == _pSLtoken_init_slstring_token (ctok, STRING_TOKEN, buf, strlen (buf)))
	       break;

	     is_deref = 1;
	  }
	else
	  {
	     if (IDENT_TOKEN != ctok->type)
	       break;
	  }

	new_tok = allocate_token ();
	if (new_tok == NULL)
	  break;

	*new_tok = *ctok;
	new_tok->type = STRING_TOKEN;

	init_token (ctok);

	if (name_list_root == NULL)
	  name_list_tail = name_list_root = new_tok;
	else
	  name_list_tail->next = new_tok;
	name_list_tail = new_tok;

	n++;

	if ((COMMA_TOKEN == get_token (ctok))
	    && (is_deref == 0))
	  {
	     get_token (ctok);
	     continue;
	  }

	if (assign_ok == 0)
	  break;

	if ((ASSIGN_TOKEN == ctok->type) || is_deref)
	  {
	     /* name = ... */
#if SLANG_HAS_BOSEOS
	     int eos = append_bos (ctok, 2);
#endif
	     if (is_deref == 0)
	       get_token (ctok);

	     simple_expression (ctok);
#if SLANG_HAS_BOSEOS
	     if (eos) append_eos ();
#endif

	     if (-1 == append_copy_of_string_token (new_tok))
	       break;

	     m++;

	     if (ctok->type != COMMA_TOKEN)
	       break;

	     get_token (ctok);
	     continue;
	  }

	break;
     }

   if (_pSLang_Error)
     {
	free_token_linked_list (name_list_root);
	return NULL;
     }

   if (n == 0)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting a qualifier", ctok, 0);
	return NULL;
     }

   *nassignp = m;
   return name_list_root;
}

static int handle_struct_fields (_pSLang_Token_Type *ctok, int assign_ok)
{
   _pSLang_Token_Type *name_list, *next;
   unsigned int n, m;

   if (NULL == (name_list = handle_struct_assign_list (ctok, assign_ok, &m)))
     return -1;

   n = 0;
   next = name_list;
   while (next != NULL)
     {
	if (-1 == append_token (next))
	  break;
	next = next->next;
	n++;
     }
   free_token_linked_list (name_list);

   if (_pSLang_Error)
     return -1;

   append_int_as_token (n);
   if (m == 0)
     append_token_of_type (STRUCT_TOKEN);
   else
     {
	append_int_as_token (m);
	append_token_of_type (STRUCT_WITH_ASSIGN_TOKEN);
     }

   if (_pSLang_Error)
     return -1;

   return 0;
}

/* struct-declaration:
 * 	struct { struct-field-list };
 *
 * struct-field-list:
 * 	struct-field-name [= simple_expr], struct-field-list
 * 	struct-field-name [= simple_expr]
 *
 * Generates code:
 *    "field-name-1" ... "field-name-N" N STRUCT_TOKEN
 * - OR -
 *    expr-k1 "field-name-k1" ... expr-kM "field-name-kM"
 *        "name-1" ... "field-name-N" N M STRUCT_DEF_ASSIGN_TOKEN
 */
static void struct_declaration (_pSLang_Token_Type *ctok, int assign_ok)
{
   if (ctok->type != OBRACE_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting {", ctok, 0);
	return;
     }
   get_token (ctok);

   if (-1 == handle_struct_fields (ctok, assign_ok))
     return;

   if (ctok->type != CBRACE_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting }", ctok, 0);
	return;
     }
   get_token (ctok);
}

/* struct-declaration:
 * 	typedef struct { struct-field-list } Type_Name;
 *
 * struct-field-list:
 * 	struct-field-name , struct-field-list
 * 	struct-field-name
 *
 * Generates code: "field-name-1" ... "field-name-N" N STRUCT_TOKEN typedef
 */
static void typedef_definition (_pSLang_Token_Type *t)
{

   if (t->type != STRUCT_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting `struct'", t, 0);
	return;
     }
   get_token (t);

   struct_declaration (t, 0);
   if (t->type != IDENT_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting identifier", t, 0);
	return;
     }

   t->type = STRING_TOKEN;
   append_token (t);
   append_token_of_type (TYPEDEF_TOKEN);

   get_token (t);
}

/* function-args:
 * 	( args-dec-opt )
 *
 * args-decl-opt:
 * 	identifier
 * 	args-decl , identifier
 */
static void define_function_args (_pSLang_Token_Type *ctok)
{
   if (CPAREN_TOKEN == get_token (ctok))
     {
	get_token (ctok);
	return;
     }

   compile_token_of_type(OBRACKET_TOKEN);

   while ((_pSLang_Error == 0)
	  && (ctok->type == IDENT_TOKEN))
     {
	compile_token (ctok);
	if (COMMA_TOKEN != get_token (ctok))
	  break;

	get_token (ctok);
     }

   if (CPAREN_TOKEN != ctok->type)
     {
	_pSLparse_error(SL_SYNTAX_ERROR, "Expecting )", ctok, 0);
	return;
     }
   compile_token_of_type(CBRACKET_TOKEN);

   get_token (ctok);
}

void try_multiple_assignment (_pSLang_Token_Type *ctok)
{
   /* This is called with ctok->type == OPAREN_TOKEN.  We have no idea
    * what follows this.  There are various possibilities such as:
    * @  () = x;
    * @  ( expression ) = x;
    * @  ( expression ) ;
    * @  ( expression ) OP expression;
    * @  ( expression ) [expression] = expression;
    * and only the first two constitute a multiple assignment.  The last
    * two forms create the difficulty.
    *
    * Here is the plan.  First parse (expression) and then check next token.
    * If it is an equal operator, then it will be parsed as a multiple
    * assignment.  In fact, that is the easy part.
    *
    * The hard part stems from the fact that by parsing (expression), we
    * have effectly truncated the parse if (expression) is part of a binary
    * or unary expression.  Somehow, the parsing must be resumed.  The trick
    * here is to use a dummy literal that generates no code: NO_OP_LITERAL
    * Using it, we just call 'expression' and proceed.
    */

   if (NULL == push_token_list ())
     return;

   get_token (ctok);

   if (ctok->type != CPAREN_TOKEN)
     {
	expression_with_commas (ctok, 1);
	if (ctok->type != CPAREN_TOKEN)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting )", ctok, 0);
	     return;
	  }
     }

   switch (get_token (ctok))
     {
      case ASSIGN_TOKEN:
      case PLUSEQS_TOKEN:
      case MINUSEQS_TOKEN:
      case TIMESEQS_TOKEN:
      case DIVEQS_TOKEN:
      case BOREQS_TOKEN:
      case BANDEQS_TOKEN:
	do_multiple_assignment (ctok);
	pop_token_list (1);
	break;

      default:
	unget_token (ctok);
	ctok->type = NO_OP_LITERAL;
	expression (ctok);
	compile_token_list ();
	break;
     }
}

/* assignment-expression:
 *   simple_expression
 *   simple_expression assign-op simple_expression
 *   simple_expression ++
 *   simple_expression --
 *   ++ simple_expression
 *   -- simple_expression
 */
static void assignment_expression (_pSLang_Token_Type *ctok)
{
   unsigned int start_pos, end_pos;
   unsigned char type;

   if (Token_List == NULL)
     return;

   type = ctok->type;
   if ((type == PLUSPLUS_TOKEN) || (type == MINUSMINUS_TOKEN))
     {
	get_token (ctok);
	simple_expression (ctok);
	check_for_lvalue (type, NULL);
	return;
     }
   start_pos = Token_List->len;

   if (ctok->type == NO_OP_LITERAL)
     {
	/* This is called from try_multiple_assignment with a new token list.
	 * The tokens added to that list collectively make up a object that
	 * is treated as a literal.  Reset the list start position to
	 * those start of those elements.
	 */
	start_pos = 0;
     }
   simple_expression (ctok);
   switch (ctok->type)
     {
      case PLUSPLUS_TOKEN:
      case MINUSMINUS_TOKEN:
	check_for_lvalue (ctok->type, NULL);
	get_token (ctok);
	break;

      case ASSIGN_TOKEN:
      case PLUSEQS_TOKEN:
      case MINUSEQS_TOKEN:
      case TIMESEQS_TOKEN:
      case DIVEQS_TOKEN:
      case BOREQS_TOKEN:
      case BANDEQS_TOKEN:
	end_pos = Token_List->len;
	check_for_lvalue (ctok->type, NULL);
	get_token (ctok);
	simple_expression (ctok);
	token_list_element_exchange (start_pos, end_pos);
	break;
     }
}

/* Note:  expression never gets compiled directly.  Rather, it gets
 *        appended to the token list and then compiled by a calling
 *        routine.
 */

/* expression:
 *	 assignment_expression
 *	 assignment_expression, expression
 *       <none>
 */
static void expression_with_commas (_pSLang_Token_Type *ctok, int save_comma)
{
   while (_pSLang_Error == 0)
     {
	if (ctok->type != COMMA_TOKEN)
	  {
	     if (ctok->type == CPAREN_TOKEN)
	       return;

	     assignment_expression (ctok);

	     if (ctok->type != COMMA_TOKEN)
	       break;
	  }
	if (save_comma) append_token (ctok);
	get_token (ctok);
     }
}

static void expression (_pSLang_Token_Type *ctok)
{
   expression_with_commas (ctok, 0);
}

/* priority levels of binary operations */
static unsigned char Binop_Level[] =
{
/* SC_AND_TOKEN */	10,
/* SC_OR_TOKEN */	12,
/* POW_TOKEN */		0,  /* This case is handled in unary expression so that -x^2 == -(x^2) */
/* ADD_TOKEN */		2,
/* SUB_TOKEN */		2,
/* MUL_TOKEN */		1,
/* DIV_TOKEN */		1,
/* LT_TOKEN */		4,
/* LE_TOKEN */		4,
/* GT_TOKEN */		4,
/* GE_TOKEN */		4,
/* EQ_TOKEN */		4,
/* NE_TOKEN */		4,
/* AND_TOKEN */		9,
/* OR_TOKEN */		11,
/* MOD_TOKEN */		1,
/* BAND_TOKEN */	6,
/* SHL_TOKEN */		3,
/* SHR_TOKEN */		3,
/* BXOR_TOKEN */	7,
/* BOR_TOKEN */		8,
/* POUND_TOKEN */	1  /* Matrix Multiplication */
};

static void handle_binary_sequence (_pSLang_Token_Type *, unsigned char);
static void handle_sc_sequence (_pSLang_Token_Type *ctok, unsigned char level)
{
   unsigned char type = ctok->type;

   while ((ctok->type == type) && (_pSLang_Error == 0))
     {
	append_token_of_type (OBRACE_TOKEN);
	get_token (ctok);
	unary_expression (ctok);
	handle_binary_sequence (ctok, level);
	append_token_of_type (CBRACE_TOKEN);
     }
   append_token_of_type (type);
}

/* unary0 OP1 unary1 OP2 unary2 ... OPN unaryN
 * ==> unary0 unary1 ... unaryN {OP1 OP2 ... OPN} COMPARE_BLOCK
 */
static void handle_compare_sequence (_pSLang_Token_Type *ctok, unsigned char level)
{
   unsigned char op_stack [64];
   unsigned int op_num = 0;
   unsigned int i;

   do
     {
	if (op_num >= sizeof(op_stack))
	  {
	     _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Too many comparison operators", ctok, 0);
	     return;
	  }
	op_stack[op_num++] = ctok->type;
	get_token (ctok);
	unary_expression (ctok);
	handle_binary_sequence (ctok, level);
     }
   while (IS_COMPARE_OP(ctok->type) && (_pSLang_Error == 0));

   if (op_num == 1)
     {
	append_token_of_type (op_stack[0]);
	return;
     }

   append_token_of_type (OBRACE_TOKEN);
   for (i = 0; i < op_num; i++)
     append_token_of_type (op_stack[i]);
   append_token_of_type (CBRACE_TOKEN);
   append_token_of_type (_COMPARE_TOKEN);
}

static void handle_binary_sequence (_pSLang_Token_Type *ctok, unsigned char max_level)
{
   unsigned char op_stack [64];
   unsigned char level_stack [64];
   unsigned char level;
   unsigned int op_num;
   unsigned char type;

   op_num = 0;
   type = ctok->type;

   while ((_pSLang_Error == 0)
	  && (IS_BINARY_OP(type)))
     {
	level = Binop_Level[type - FIRST_BINARY_OP];
	if (level >= max_level)
	  break;

	while ((op_num > 0) && (level_stack [op_num - 1] <= level))
	  append_token_of_type (op_stack [--op_num]);

	if ((type == SC_AND_TOKEN) || (type == SC_OR_TOKEN))
	  {
	     handle_sc_sequence (ctok, level);
	     type = ctok->type;
	     continue;
	  }

	if (IS_COMPARE_OP(type))
	  {
	     handle_compare_sequence (ctok, level);
	     type = ctok->type;
	     continue;
	  }
	if (op_num >= sizeof (op_stack) - 1)
	  {
	     _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Binary op stack overflow", ctok, 0);
	     return;
	  }

	op_stack [op_num] = type;
	level_stack [op_num] = level;
	op_num++;

	get_token (ctok);
	unary_expression (ctok);
	type = ctok->type;
     }

   while (op_num > 0)
     append_token_of_type(op_stack[--op_num]);
}

/* % Note: simple-expression groups operators OP1 at same level.  The
 * % actual implementation will not do this.
 * simple-expression:
 *       simple-expression ? simple-expression : simple-expression
 *	 unary-expression
 *	 binary-expression BINARY-OP unary-expression
 *       andelse xxelse-expression-list
 *       orelse xxelse-expression-list
 *
 * xxelse-expression-list:
 * 	{ expression }
 * 	xxelse-expression-list { expression }
 * binary-expression:
 *      unary-expression
 *      unary-expression BINARY-OP binary-expression
 */
static void simple_expression (_pSLang_Token_Type *ctok)
{
   unsigned char type;

   switch (ctok->type)
     {
      case ANDELSE_TOKEN:
      case ORELSE_TOKEN:
	type = ctok->type;
	if (OBRACE_TOKEN != get_token (ctok))
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting '{'", ctok, 0);
	     return;
	  }

	while (ctok->type == OBRACE_TOKEN)
	  {
	     append_token (ctok);
	     get_token (ctok);
	     expression (ctok);
	     if (CBRACE_TOKEN != ctok->type)
	       {
		  _pSLparse_error(SL_SYNTAX_ERROR, "Expecting }", ctok, 0);
		  return;
	       }
	     append_token (ctok);
	     get_token (ctok);
	  }
	append_token_of_type (type);
	return;

	/* avoid unary-expression if possible */
      case STRING_TOKEN:
	append_token (ctok);
	get_token (ctok);
	break;

      default:
	unary_expression (ctok);
	break;
     }

   if (SEMICOLON_TOKEN == (type = ctok->type))
     return;

   handle_binary_sequence (ctok, 0xFF);

   if (ctok->type == QUESTION_TOKEN)
     {
	append_token_of_type (OBRACE_TOKEN);
	get_token (ctok);
	simple_expression (ctok);
	if (ctok->type != COLON_TOKEN)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting a colon in the ternary expression", ctok, 0);
	     return;
	  }
	append_token_of_type (CBRACE_TOKEN);
	get_token (ctok);
	append_token_of_type (OBRACE_TOKEN);
	simple_expression (ctok);
	append_token_of_type (CBRACE_TOKEN);
	append_token_of_type (ELSE_TOKEN);
     }
}

static int negate_float_type_token (_pSLang_Token_Type *tok)
{
   return prefix_token_sval_field (tok, "-");
}

/* unary-expression:
 *	 postfix-expression
 *	 case unary-expression
 *	 OP3 unary-expression
 *	 (OP3: + - ~ & not)
 *
 * Note:  This grammar permits: case case case WHATEVER
 */
static void unary_expression (_pSLang_Token_Type *ctok)
{
   unsigned char save_unary_ops [16];
   unsigned int num_unary_ops;
   unsigned char type;
#if 0
   _pSLang_Token_Type *last_token;
#endif
   num_unary_ops = 0;
   while (_pSLang_Error == 0)
     {
	type = ctok->type;

	switch (type)
	  {
#if 0
	   case PLUSPLUS_TOKEN:
	   case MINUSMINUS_TOKEN:
	     get_token (ctok);
	     postfix_expression (ctok);
	     check_for_lvalue (type, NULL);
	     goto out_of_switch;
#endif
	   case ADD_TOKEN:
	     get_token (ctok);	       /* skip it-- it's unary here */
	     break;

	   case SUB_TOKEN:
	     (void) get_token (ctok);
	     if (ctok->flags & SLTOKEN_TYPE_NUMBER)
	       {
		  _pSLang_Token_Type *last_token;
		  postfix_expression (ctok);
		  if ((NULL != (last_token = get_last_token ()))
		      && (last_token->flags & SLTOKEN_TYPE_NUMBER))
		    {
		       if (last_token->flags & SLTOKEN_TYPE_FLOAT)
			 {
			    if (-1 == negate_float_type_token (last_token))
			      return;
			 }
		       else if (-1 == check_number_token_overflow (last_token, -1))
			 return;
		    }
		  else
		    {
		       if (num_unary_ops == 16)
			 goto stack_overflow_error;
		       save_unary_ops [num_unary_ops++] = CHS_TOKEN;
		    }
		  goto out_of_switch;
	       }
	     if (num_unary_ops == 16)
	       goto stack_overflow_error;
	     save_unary_ops [num_unary_ops++] = CHS_TOKEN;
	     break;
#if 0
	   case DEREF_TOKEN:
#endif
	   case BNOT_TOKEN:
	   case NOT_TOKEN:
	   case CASE_TOKEN:
	     if (num_unary_ops == 16)
	       goto stack_overflow_error;

	     save_unary_ops [num_unary_ops++] = type;
	     get_token (ctok);
	     break;

	     /* Try to avoid ->postfix_expression->primary_expression
	      * subroutine calls.
	      */
	   case STRING_TOKEN:
	     append_token (ctok);
	     get_token (ctok);
	     goto out_of_switch;

	   default:
	     postfix_expression (ctok);
	     goto out_of_switch;
	  }
     }

   out_of_switch:
   while (num_unary_ops)
     {
	num_unary_ops--;
	append_token_of_type (save_unary_ops [num_unary_ops]);
     }
   return;

   stack_overflow_error:
   _pSLparse_error (SL_BUILTIN_LIMIT_EXCEEDED, "Too many unary operators.", ctok, 0);
}

static int combine_namespace_tokens (_pSLang_Token_Type *a, _pSLang_Token_Type *b)
{
   SLFUTURE_CONST char *sa, *sb;
   char *sc;
   unsigned int lena, lenb;
   unsigned long hash;

   /* This is somewhat of a hack.  Combine the TWO identifier names
    * (NAMESPACE) and (name) into the form NAMESPACE->name.  Then when the
    * byte compiler compiles the object it will not be found.  It will then
    * check for this hack and make the appropriate namespace lookup.
    */

   sa = a->v.s_val;
   sb = b->v.s_val;

   lena = strlen (sa);
   lenb = strlen (sb);

   sc = SLmalloc (lena + lenb + 3);
   if (sc == NULL)
     return -1;

   strcpy (sc, sa);
   strcpy (sc + lena, "->");
   strcpy (sc + lena + 2, sb);

   sb = _pSLstring_make_hashed_string (sc, lena + lenb + 2, &hash);
   SLfree (sc);
   if (sb == NULL)
     return -1;

   /* I can free this string because no other token should be referencing it.
    * (num_refs == 1).
    */
   _pSLfree_hashed_string ((char *) sa, lena, a->hash);
   a->v.s_val = sb;
   a->hash = hash;

   return 0;
}

static void append_identifier_token (_pSLang_Token_Type *ctok)
{
   _pSLang_Token_Type *last_token;

   append_token (ctok);

   if (NAMESPACE_TOKEN != get_token (ctok))
     return;

   if (IDENT_TOKEN != get_token (ctok))
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting name-space identifier", ctok, 0);
	return;
     }

   if (NULL == (last_token = get_last_token ()))
     {
	if (_pSLang_Error == 0)
	  _pSLang_verror (SL_INTERNAL_ERROR, "get_last_token returned NULL in append_identifier_token");
	return;
     }
   if (-1 == combine_namespace_tokens (last_token, ctok))
     return;

   (void) get_token (ctok);
}

static int get_identifier_expr_token (_pSLang_Token_Type *ctok)
{
   _pSLang_Token_Type next_token;

   if (IDENT_TOKEN != get_identifier_token (ctok))
     return -1;

   init_token (&next_token);
   if (NAMESPACE_TOKEN != get_token (&next_token))
     {
	unget_token (&next_token);
	return IDENT_TOKEN;
     }

   if (IDENT_TOKEN != get_identifier_token (&next_token))
     {
	free_token (&next_token);
	return -1;
     }

   if (-1 == combine_namespace_tokens (ctok, &next_token))
     {
	free_token (&next_token);
	return -1;
     }
   free_token (&next_token);
   return IDENT_TOKEN;
}

/* postfix-expression:
 *       @ postfix-expression
 *	 primary-expression
 *	 postfix-expression [ expression ]
 *	 postfix-expression ( function-args-expression )
 *	 postfix-expression . identifier
 *       postfix-expression ^ unary-expression
 *
 * Not yet supported:
 *	 postfix-expression ++
 *	 postfix-expression --
 *	 postfix-expression = simple-expression
 *	 postfix-expression += simple-expression
 *	 postfix-expression -= simple-expression
 *
 * primary-expression:
 *	literal
 *	identifier-expr
 *	( expression_opt )
 * 	[ inline-array-expression ]
 *      { inline-list-expression }
 * 	&identifier-expr
 *      struct-definition
 *      __tmp(identifier-expr)
 *
 * identifier-expr:
 *      identifier
 *      identifier->identifier
 */
static void postfix_expression (_pSLang_Token_Type *ctok)
{
   unsigned int start_pos;
   unsigned char type;
   _pSLang_Token_Type *last_token;

   if (Token_List == NULL)
     return;

   start_pos = Token_List->len;

   switch (ctok->type)
     {
#if 1
      case DEREF_TOKEN:
	get_token (ctok);
	postfix_expression (ctok);
	append_token_of_type (DEREF_TOKEN);
	break;
#endif
      case IDENT_TOKEN:
	append_identifier_token (ctok);
	break;

      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
#ifdef HAVE_LONG_LONG
      case LLONG_TOKEN:
      case ULLONG_TOKEN:
#endif
      case STRING_TOKEN:
      case BSTRING_TOKEN:
#ifdef SLANG_HAS_FLOAT
      case DOUBLE_TOKEN:
      case FLOAT_TOKEN:
      case LDOUBLE_TOKEN:
#endif
#ifdef SLANG_HAS_COMPLEX
      case COMPLEX_TOKEN:
#endif
      case STRING_DOLLAR_TOKEN:
      case MULTI_STRING_TOKEN:
	append_token (ctok);
	get_token (ctok);
	if (ctok->type == OPAREN_TOKEN)
	  _pSLparse_error(SL_SYNTAX_ERROR, "Literal constant is not callable", ctok, 1);
	break;

      case OPAREN_TOKEN:
	if (CPAREN_TOKEN != get_token (ctok))
	  {
	     expression (ctok);
	     if (ctok->type != CPAREN_TOKEN)
	       _pSLparse_error(SL_SYNTAX_ERROR, "Expecting )", ctok, 0);
	  }
	get_token (ctok);
	break;

      case BAND_TOKEN:
#if 0
	if (IDENT_TOKEN != get_identifier_expr_token (ctok))
	  break;

	ctok->type = _REF_TOKEN;
	append_token (ctok);
	get_token (ctok);
	if ((ctok->type == OBRACKET_TOKEN) || (ctok->type == DOT_TOKEN))
	  _pSLparse_error (SL_NOT_IMPLEMENTED, "& of an array or structure element is not currently supported", ctok, 0);
#else
	get_token (ctok);
	postfix_expression (ctok);
	last_token = get_last_token ();
	if (last_token == NULL)
	  {
	     if (_pSLang_Error == 0)
	       _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced &");
	     return;
	  }
	switch (last_token->type)
	  {
	   case IDENT_TOKEN:
	     last_token->type = _REF_TOKEN;
	     break;
	   case ARRAY_TOKEN:
	     last_token->type = _ARRAY_ELEM_REF_TOKEN;
	     break;
	   case DOT_TOKEN:
	     last_token->type = _STRUCT_FIELD_REF_TOKEN;
	     break;
	   default:
	     _pSLparse_error (SL_NOT_IMPLEMENTED, "& not supported in this context", last_token, 0);
	  }
#endif
	break;

      case OBRACKET_TOKEN:
	get_token (ctok);
	inline_array_expression (ctok);
	break;

      case OBRACE_TOKEN:
	get_token (ctok);
	inline_list_expression (ctok);
	break;

      case NO_OP_LITERAL:
	/* This token was introduced by try_multiple_assignment.  There,
	 * a new token_list was pushed and (expression) was evaluated.
	 * NO_OP_LITERAL represents the result of expression.  However,
	 * we need to tweak the start_pos variable to point to the beginning
	 * of the token list to complete the equivalence.
	 */
	start_pos = 0;
	get_token (ctok);
	break;

      case STRUCT_TOKEN:
	get_token (ctok);
	struct_declaration (ctok, 1);
	break;

      case TMP_TOKEN:
	get_token (ctok);
	if (ctok->type == OPAREN_TOKEN)
	  {
	     if (IDENT_TOKEN == get_identifier_expr_token (ctok))
	       {
		  ctok->type = TMP_TOKEN;
		  append_token (ctok);
		  get_token (ctok);
		  if (ctok->type == CPAREN_TOKEN)
		    {
		       get_token (ctok);
		       break;
		    }
	       }
	  }
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting form __tmp(NAME)", ctok, 0);
	break;

      default:
#ifdef IS_INTERNAL_FUNC
	if (IS_INTERNAL_FUNC(ctok->type))
	  {
	     append_token (ctok);
	     get_token (ctok);
	  }
	else
#endif
	  _pSLparse_error(SL_SYNTAX_ERROR, "Expecting a PRIMARY", ctok, 0);
     }

   while (_pSLang_Error == 0)
     {
	unsigned int end_pos = Token_List->len;
	type = ctok->type;
	switch (type)
	  {
	   case OBRACKET_TOKEN:	       /* X[args] ==> [args] X ARRAY */
	     get_token (ctok);
	     append_token_of_type (ARG_TOKEN);
	     if (ctok->type != CBRACKET_TOKEN)
	       array_index_expression (ctok);

	     if (ctok->type != CBRACKET_TOKEN)
	       {
		  _pSLparse_error (SL_SYNTAX_ERROR, "Expecting ']'", ctok, 0);
		  return;
	       }
	     get_token (ctok);
	     /* append_token_of_type (EARG_TOKEN); -- ARRAY_TOKEN implicitely does this */
	     token_list_element_exchange (start_pos, end_pos);
	     append_token_of_type (ARRAY_TOKEN);
	     break;

	   case OPAREN_TOKEN:
	     /* f(args) ==> args f */
	     if ((NULL != (last_token = get_last_token ()))
		 && (last_token->type == DEREF_TOKEN))
	       {
		  /* Expressions such as (@A[i])(args...) */
		  last_token->type = _DEREF_FUNCALL_TOKEN;
		  append_token_of_type (ARG_TOKEN);
		  (void) get_token (ctok);
		  function_args_expression (ctok, 0, 1, 1, NULL);
		  /* Now we have: ... @ __args ...
		   * and we want: ... __args ... @
		   */
		  /* token_list_element_exchange (start_pos, end_pos); */
		  token_list_element_exchange (end_pos-1, end_pos);
		  break;
	       }

	     if (CPAREN_TOKEN != get_token (ctok))
	       {
		  function_args_expression (ctok, 1, 1, 1, NULL);
		  token_list_element_exchange (start_pos, end_pos);
	       }
	     else get_token (ctok);
	     break;

	   case DOT_TOKEN:
	     /* S.a ==> "a" S DOT
	      * This means that if S is X[b], then X[b].a ==> a b X ARRAY DOT
	      * and f(a).X[b].c ==> "c" b "X" a f . ARRAY .
	      * Also, f(a).X[b] = g(x); ==> x g b "X" a f .
	      */
	     if (IDENT_TOKEN != get_identifier_token (ctok))
	       return;

	     ctok->type = DOT_TOKEN;
	     append_token (ctok);
	     get_token (ctok);
#ifdef DOT_METHOD_CALL_TOKEN
	     if (ctok->type == OPAREN_TOKEN)
	       {
		  unsigned int qual_pos, meth_pos, x_pos, y_pos;
		  /* This case is a bit tricky for expressions such as:
		   *   foo(x;q1).bar(y;q2)
		   *                ^
		   * Here, '^' denotes the parse point.  The token list looks
		   * roughly like:
		   *    __arg x q1 __earg foo bar .
		   */
		  if (NULL == (last_token = get_last_token ()))
		    return;
		  last_token->type = DOT_METHOD_CALL_TOKEN;
		  x_pos = start_pos;
		  y_pos = Token_List->len;
		  meth_pos = y_pos-1;
		  append_token_of_type (ARG_TOKEN);
		  get_token (ctok);
		  function_args_expression (ctok, 0, 1, 1, &qual_pos);
		  if (_pSLang_Error)
		    break;
		  end_pos = Token_List->len;
		  /* At this point, the token list looks like:
		   *   __arg x q1 __earg foo methcall(bar) __arg y q2
		   * x^                    m^            y^      q^
		   * where ^ denotes the meth, x, y, and qual positions.
		   * We want to rearrange this to be:
		   *   __arg y __arg x q1 __earg foo q2 methcall(bar)
		   * Do it in 3 stages:
		   */
		  token_list_element_exchange (x_pos, y_pos);
		  /*   __arg y q2 __arg x q1 __earg foo methcall(bar)
		   * y^      q^ x^                    m^
		   */
		  qual_pos = start_pos + (qual_pos-y_pos);
		  meth_pos = meth_pos + (end_pos-y_pos);
		  x_pos = start_pos + (end_pos-y_pos);
		  token_list_element_exchange (qual_pos, x_pos);
		  /*   __arg y __arg x q1 __earg foo methcall(bar) q2
		   * y^      x^                    m^            q^
		   */
		  meth_pos = meth_pos - (x_pos - qual_pos);
		  qual_pos = qual_pos + (end_pos-x_pos);
		  token_list_element_exchange (meth_pos, qual_pos);
	       }
#endif
	     break;

#if 0
	   case PLUSPLUS_TOKEN:
	   case MINUSMINUS_TOKEN:
	     check_for_lvalue (type, NULL);
	     get_token (ctok);
	     break;
	   case ASSIGN_TOKEN:
	   case PLUSEQS_TOKEN:
	   case MINUSEQS_TOKEN:
	   case TIMESEQS_TOKEN:
	   case DIVEQS_TOKEN:
	   case BOREQS_TOKEN:
	   case BANDEQS_TOKEN:
	     check_for_lvalue (type, NULL);
	     get_token (ctok);
	     simple_expression (ctok);
	     token_list_element_exchange (start_pos, end_pos);
	     break;
#endif
#if 1
	   case POW_TOKEN:
	     get_token (ctok);
	     unary_expression (ctok);
	     append_token_of_type (POW_TOKEN);
	     break;
#endif
	   default:
	     return;
	  }
     }
}

/* This function is used for more than function arguments */
static void function_args_expression (_pSLang_Token_Type *ctok, int handle_num_args, int handle_qualifiers,
				      int is_function,
				      unsigned int *qual_posp)
{
   unsigned char last_type, this_type;
   int has_qualifiers = 0;

   if (handle_num_args) append_token_of_type (ARG_TOKEN);

   last_type = (ctok->type == COMMA_TOKEN) ? COMMA_TOKEN : 0;

   while (_pSLang_Error == 0)
     {
	this_type = ctok->type;

	switch (this_type)
	  {
	   case COMMA_TOKEN:
	     if (last_type == COMMA_TOKEN)
	       append_token_of_type (_NULL_TOKEN);
	     get_token (ctok);
	     break;

	   case CPAREN_TOKEN:
	     if (last_type == COMMA_TOKEN)
	       append_token_of_type (_NULL_TOKEN);

	     if (handle_num_args) append_token_of_type (EARG_TOKEN);

	     if ((qual_posp != NULL) && (has_qualifiers == 0))
	       *qual_posp = Token_List->len;

	     get_token (ctok);
	     if (is_function && (ctok->type == OPAREN_TOKEN))
	       _pSLparse_error (SL_SYNTAX_ERROR, "A '(' is not permitted here", ctok, 0);
	     return;

	   case SEMICOLON_TOKEN:
	     if (handle_qualifiers)
	       {
		  if (last_type == COMMA_TOKEN)
		    append_token_of_type (_NULL_TOKEN);

		  if (qual_posp != NULL)
		    *qual_posp = Token_List->len;

		  has_qualifiers = 1;

		  if (SEMICOLON_TOKEN == get_token (ctok))
		    {
		       /* foo (args... ;; q) form */
		       if (CPAREN_TOKEN == get_token (ctok))
			 break;  /* foo (args ;;) */
		       simple_expression (ctok);
		    }
		  else if (ctok->type == CPAREN_TOKEN)
		    break;	       /* foo (args;) */
		  else if (-1 == handle_struct_fields (ctok, 1))
		    return;

		  append_token_of_type (QUALIFIER_TOKEN);
		  if (ctok->type != CPAREN_TOKEN)
		    _pSLparse_error (SL_SYNTAX_ERROR, "Expecting ')'", ctok, 0);
		  break;
	       }
	     /* drop */

	   default:
	     simple_expression (ctok);
	     if ((ctok->type != COMMA_TOKEN)
		 && (ctok->type != CPAREN_TOKEN)
		 && ((handle_qualifiers == 0)
		     || (ctok->type != SEMICOLON_TOKEN)))
	       {
		  _pSLparse_error (SL_SYNTAX_ERROR, "Expecting ')'", ctok, 0);
		  break;
	       }
	  }
	last_type = this_type;
     }
}

static int check_for_lvalue (unsigned char eqs_type, _pSLang_Token_Type *ctok)
{
   unsigned char type;

   if ((ctok == NULL)
       && (NULL == (ctok = get_last_token ())))
     type = ILLEGAL_TOKEN;
   else
     type = ctok->type;

   eqs_type -= ASSIGN_TOKEN;
   switch (type)
     {
      case IDENT_TOKEN:
	eqs_type += _SCALAR_ASSIGN_TOKEN;
	break;
      case ARRAY_TOKEN:
	eqs_type += _ARRAY_ASSIGN_TOKEN;
	break;
      case DOT_TOKEN:
	eqs_type += _STRUCT_ASSIGN_TOKEN;
	break;
      case DEREF_TOKEN:
	eqs_type += _DEREF_ASSIGN_TOKEN;
	break;
      default:
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting LVALUE", ctok, 0);
	return -1;
     }

   ctok->type = eqs_type;
   return 0;
}

/* inline_list_expression:
 *     (nil)
 *     simple_expression
 *     inline_list_expression, simple_expression
 */
static void inline_list_expression (_pSLang_Token_Type *ctok)
{
   append_token_of_type (ARG_TOKEN);

   if (ctok->type != CBRACE_TOKEN)
     {
	while (1)
	  {
	     simple_expression (ctok);
	     if (ctok->type != COMMA_TOKEN)
	       break;
	     get_token (ctok);
	     if (ctok->type == CBRACE_TOKEN)   /* trailing comma: {a,b,} */
	       break;
	  }
	if (ctok->type != CBRACE_TOKEN)
	  {
	     _pSLparse_error (SL_SYNTAX_ERROR, "Expecting '}' to denote list end", ctok, 0);
	     return;
	  }
     }

   append_token_of_type (_INLINE_LIST_TOKEN);
   get_token (ctok);
}

static void array_index_expression (_pSLang_Token_Type *ctok)
{
   unsigned int num_commas;

   num_commas = 0;
   while (1)
     {
	switch (ctok->type)
	  {
	   case COLON_TOKEN:
	     if (num_commas)
	       _pSLparse_error (SL_SYNTAX_ERROR, "Misplaced ':'", ctok, 0);
	     return;

	   case TIMES_TOKEN:
	     append_token_of_type (_INLINE_WILDCARD_ARRAY_TOKEN);
	     get_token (ctok);
	     break;

	   case COMMA_TOKEN:
	     _pSLparse_error (SL_SYNTAX_ERROR, "Misplaced ','", ctok, 0);
	     return;

	   default:
	     simple_expression (ctok);
	  }

	if (ctok->type != COMMA_TOKEN)
	  return;
	num_commas++;
	get_token (ctok);
	if (ctok->type == CBRACKET_TOKEN)   /* allow trailing comma */
	  return;
     }
}

/* inline-array-expression:
 *    array_index_expression
 *    simple_expression : simple_expression
 *    simple_expression : simple_expression : simple_expression
 *    simple_expression : simple_expression : # simple_expression
 */
static void inline_array_expression (_pSLang_Token_Type *ctok)
{
   int num_colons = 0;
   int has_pound = 0;

   append_token_of_type (ARG_TOKEN);

   if (ctok->type == COLON_TOKEN)	       /* [:...] */
     append_token_of_type (_NULL_TOKEN);
   else if (ctok->type != CBRACKET_TOKEN)
     array_index_expression (ctok);

   if (ctok->type == COLON_TOKEN)
     {
	num_colons++;
	if ((COLON_TOKEN == get_token (ctok))
	    || (ctok->type == CBRACKET_TOKEN))
	  append_token_of_type (_NULL_TOKEN);
	else
	  simple_expression (ctok);

	if (ctok->type == COLON_TOKEN)
	  {
	     num_colons++;
	     if (POUND_TOKEN == get_token (ctok))
	       {
		  has_pound = 1;
		  get_token(ctok);
	       }
	     simple_expression (ctok);
	  }
     }

   if (ctok->type != CBRACKET_TOKEN)
     {
	_pSLparse_error (SL_SYNTAX_ERROR, "Expecting ']'", ctok, 0);
	return;
     }

   /* append_token_of_type (EARG_TOKEN); */
   if (num_colons)
     {
	if (has_pound)
	  append_token_of_type (_INLINE_IMPLICIT_ARRAYN_TOKEN);
	else
	  append_token_of_type (_INLINE_IMPLICIT_ARRAY_TOKEN);
     }
   else
     append_token_of_type (_INLINE_ARRAY_TOKEN);
   get_token (ctok);
}

static void do_multiple_assignment (_pSLang_Token_Type *ctok)
{
   _pSLang_Token_Type *s;
   unsigned int i, k, len;
   unsigned char assign_type;

   assign_type = ctok->type;

   /* The LHS token list has already been pushed.  Here we do the RHS
    * so push to another token list, process it, then come back to
    * LHS for assignment.
    */
   if (NULL == push_token_list ())
     return;

   get_token (ctok);
   expression (ctok);
   compile_token_list ();

   if (_pSLang_Error)
     return;

   /* Finally compile the LHS of the assignment expression
    * that has been saved.
    */
   s = Token_List->stack;
   len = Token_List->len;

   if (len == 0)
     {
	compile_token_of_type (POP_TOKEN);
	return;
     }

   while (len > 0)
     {
	/* List is of form:
	 *    a , b, c d e, f , g , , , h ,
	 * The missing expressions will be replaced by a POP
	 * ,,a
	 */

	/* Start from back looking for a COMMA */
	k = len - 1;
	if (s[k].type == COMMA_TOKEN)
	  {
	     compile_token_of_type (POP_TOKEN);
	     len = k;
	     continue;
	  }

	if (-1 == check_for_lvalue (assign_type, s + k))
	  return;

	i = 0;
	while (1)
	  {
	     if (s[k].type == COMMA_TOKEN)
	       {
		  i = k + 1;
		  break;
	       }

	     if (k == 0)
	       break;

	     k--;
	  }

	while (i < len)
	  {
	     compile_token (s + i);
	     i++;
	  }

	len = k;
     }

   if (s[0].type == COMMA_TOKEN)
     compile_token_of_type (POP_TOKEN);
}


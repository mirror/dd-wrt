#include <stdio.h>
#include <stdio.h>
#include "slinclud.h"
#include "slang.h"
#include "_slang.h"

static void dump_token (_pSLang_Token_Type *t)
{
   char buf [256], *b;

   b = buf;

   if (_pSLang_Error)
     return;

   switch (t->type)
     {
      case TIMES_TOKEN:
	b = "*";
	break;
      case IDENT_TOKEN:
	b = t->v.s_val;
	break;

      case TMP_TOKEN:
	b = "__tmp"; break;

      case CHAR_TOKEN:
      case UCHAR_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	sprintf (buf, "%ld", t->v.long_val);
	break;

      case DOUBLE_TOKEN:
        b = t->v.s_val;
	break;

      case STRING_TOKEN:
	sprintf (buf, "\"%s\"", t->v.s_val);
	break;

      case STRING_DOLLAR_TOKEN:
	sprintf (buf, "\"%s\"$", t->v.s_val);
	break;

      case PLUSPLUS_TOKEN:
	sprintf (buf, "++ (RPN)");
	break;

      case POST_PLUSPLUS_TOKEN:
	sprintf (buf, "%s++", t->v.s_val);
	break;

      case MINUSMINUS_TOKEN:
	sprintf (buf, "-- (RPN)");
	break;

      case POST_MINUSMINUS_TOKEN:
	sprintf (buf, "%s--", t->v.s_val);
	break;

      case MINUSEQS_TOKEN:
	sprintf (buf, "-=%s", t->v.s_val);
	break;

      case PLUSEQS_TOKEN:
	sprintf (buf, "+=%s", t->v.s_val);
	break;

      case ASSIGN_TOKEN:
	sprintf (buf, "= (RPN assign)");
	break;

      case EOF_TOKEN:
	b = "EOF_TOKEN";
	break;

      case NOP_TOKEN:
	b = "NOP_TOKEN";
	break;

      case FOREVER_TOKEN:
	b = "forever";
	break;

      case ARG_TOKEN:
	b = "__args";
	break;

      case EARG_TOKEN:
	b = "__eargs";
	break;

      case FARG_TOKEN:
	b = "__farg";
	break;

      case _INLINE_ARRAY_TOKEN:
	b = "__inline_array";
	break;

      case _INLINE_IMPLICIT_ARRAY_TOKEN:
	b = "__inline_implicit_array";
	break;

      case IFNOT_TOKEN:
	b = "!if";
	break;
#ifdef ABS_TOKEN
      case ABS_TOKEN:
	b = "abs";
	break;
#endif
      case LT_TOKEN:
	b = "<";
	break;

      case LE_TOKEN:
	b = "<=";
	break;

      case GT_TOKEN:
	b = ">";
	break;

      case GE_TOKEN:
	b = ">=";
	break;

      case EQ_TOKEN:
	b = "==";
	break;

      case NE_TOKEN:
	b = "!=";
	break;

      case AND_TOKEN:
	b = "and";
	break;

      case IF_TOKEN:
	b = "if";
	break;

      case POP_TOKEN:
	b = "pop";
	break;

      case ANDELSE_TOKEN:
	b = "andelse";
	break;

      case BXOR_TOKEN:
	b = "xor";
	break;

      case BAND_TOKEN:
	b = "&";
	break;

      case BOR_TOKEN:
	b = "|";
	break;

      case BNOT_TOKEN:
	b = "~";
	break;

      case SHR_TOKEN:
	b = "shr";
	break;

      case CHS_TOKEN:
	b = "chs";
	break;

      case SHL_TOKEN:
	b = "shl";
	break;
#ifdef SQR_TOKEN
      case SQR_TOKEN:
	b = "sqr";
	break;
#endif
      case CASE_TOKEN:
	b = "case";
	break;
#ifdef SIGN_TOKEN
      case SIGN_TOKEN:
	b = "sign";
	break;
#endif
      case BREAK_TOKEN:
	b = "break";
	break;

      case STATIC_TOKEN:
	b = "static";
	break;

      case PRIVATE_TOKEN:
	b = "private";
	break;

      case PUBLIC_TOKEN:
	b = "public";
	break;

      case STRUCT_TOKEN:
	b = "struct";
	break;

      case STRUCT_WITH_ASSIGN_TOKEN:
	b = "__struct_with_assign";
	break;

      case QUALIFIER_TOKEN:
	b = "set __qualifiers";
	break;

      case RETURN_TOKEN:
	b = "return";
	break;

      case SWITCH_TOKEN:
	b = "switch";
	break;

      case EXCH_TOKEN:
	b = "exch";
	break;

      case CONT_TOKEN:
	b = "continue";
	break;

      case EXITBLK_TOKEN:
	b = "EXIT_BLOCK";
	break;

      case ERRBLK_TOKEN:
	b = "ERROR_BLOCK";
	break;

      case USRBLK0_TOKEN:
	b = "USER_BLOCK0";
	break;

      case USRBLK1_TOKEN:
	b = "USER_BLOCK1";
	break;

      case USRBLK2_TOKEN:
	b = "USER_BLOCK2";
	break;

      case USRBLK3_TOKEN:
	b = "USER_BLOCK3";
	break;

      case USRBLK4_TOKEN:
	b = "USER_BLOCK4";
	break;

      case ELSE_TOKEN:
	b = "else";
	break;
#ifdef MUL2_TOKEN
      case MUL2_TOKEN:
	b = "mul2";
	break;
#endif
      case DEFINE_TOKEN:
	sprintf (buf, ") %s", t->v.s_val);
	break;

      case DEFINE_STATIC_TOKEN:
	sprintf (buf, ") static %s", t->v.s_val);
	break;

      case DEFINE_PRIVATE_TOKEN:
	sprintf (buf, ") private %s", t->v.s_val);
	break;

      case LOOP_TOKEN:
	b = "loop";
	break;

      case MOD_TOKEN:
	b = "mod";
	break;

      case DO_TOKEN:
	b = "do";
	break;

      case DOWHILE_TOKEN:
	b = "do_while";
	break;

      case WHILE_TOKEN:
	b = "while";
	break;

      case OR_TOKEN:
	b = "or";
	break;

      case VARIABLE_TOKEN:
	b = "variable";
	break;

      case _SCALAR_ASSIGN_TOKEN:
	sprintf (buf, "=%s", t->v.s_val);
	break;

      case _SCALAR_PLUSEQS_TOKEN:
	sprintf (buf, "+=%s", t->v.s_val);
	break;

      case _SCALAR_MINUSEQS_TOKEN:
	sprintf (buf, "-=%s", t->v.s_val);
	break;

      case _SCALAR_PLUSPLUS_TOKEN:
	sprintf (buf, "++%s", t->v.s_val);
	break;

      case _SCALAR_POST_PLUSPLUS_TOKEN:
	sprintf (buf, "%s++", t->v.s_val);
	break;

      case _SCALAR_MINUSMINUS_TOKEN:
	sprintf (buf, "--%s", t->v.s_val);
	break;

      case _SCALAR_POST_MINUSMINUS_TOKEN:
	sprintf (buf, "%s--", t->v.s_val);
	break;

      case _ARRAY_DIVEQS_TOKEN:
	sprintf (buf, "/= (array)"); break;

#if 0
      case _DEREF_ASSIGN_TOKEN:
	sprintf (buf, "=@%s", t->v.s_val);
	break;
#endif
      case _REF_TOKEN:
	sprintf (buf, "%s __ref", t->v.s_val);
	break;

      case _ARRAY_ELEM_REF_TOKEN:
	b = "__array_elem_ref";
	break;

      case _STRUCT_FIELD_REF_TOKEN:
	sprintf (buf, "%s __struct_field_ref", t->v.s_val);
	break;

      case ORELSE_TOKEN:
	b = "orelse";
	break;

      case _FOR_TOKEN:
	b = "_for";
	break;

      case FOR_TOKEN:
	b = "for";
	break;

      case NOT_TOKEN:
	b = "not";
	break;

      case OBRACKET_TOKEN:
	b = "[";
	break;

      case CBRACKET_TOKEN:
	b = "]";
	break;

      case OPAREN_TOKEN:
	b = "(";
	break;

      case CPAREN_TOKEN:
	b = ")";
	break;

      case OBRACE_TOKEN:
	b = "{";
	break;

      case CBRACE_TOKEN:
	b = "}";
	break;

      case DEREF_TOKEN:
	b = "@";
	break;

      case COMMA_TOKEN:
	b = ",";
	break;

      case SEMICOLON_TOKEN:
	b = ";";
	break;

      case COLON_TOKEN:
	b = ":";
	break;

      case SC_AND_TOKEN: b = "&&"; break;
      case SC_OR_TOKEN: b = "||"; break;

      case POW_TOKEN:
	b = "^";
	break;

      case ADD_TOKEN:
	b = "+";
	break;

      case SUB_TOKEN:
	b = "-";
	break;

      /* case MUL_TOKEN: */
      /* 	b = "*"; */
      /* 	break; */

      case DIV_TOKEN:
	b = "/";
	break;

      case ARRAY_TOKEN:
	b = "__aget";
	break;

      case DOT_TOKEN:
	sprintf (buf, "%s .", t->v.s_val);
	break;

      case DOT_METHOD_CALL_TOKEN:
	sprintf (buf, "%s __eargs __method_call", t->v.s_val);
	break;

      case _STRUCT_ASSIGN_TOKEN:
	b = "__struct_eqs"; break;
      case _STRUCT_PLUSEQS_TOKEN:
	b = "__struct_pluseqs"; break;
      case _STRUCT_MINUSEQS_TOKEN:
	b = "__struct_minuseqs"; break;
      case _STRUCT_PLUSPLUS_TOKEN:
	b = "__struct_plusplus"; break;
      case _STRUCT_POST_PLUSPLUS_TOKEN:
	b = "__struct_pplusplus"; break;
      case _STRUCT_MINUSMINUS_TOKEN:
	b = "__struct_minusminus"; break;
      case _STRUCT_POST_MINUSMINUS_TOKEN:
	b = "__struct_pminusminus"; break;

      case _ARRAY_ASSIGN_TOKEN:
	b = "_aput"; break;
      case _ARRAY_PLUSEQS_TOKEN:
	b = "__array_pluseqs"; break;
      case _ARRAY_MINUSEQS_TOKEN:
	b = "__array_minuseqs"; break;
      case _ARRAY_PLUSPLUS_TOKEN:
	b = "__array_plusplus"; break;
      case _ARRAY_POST_PLUSPLUS_TOKEN:
	b = "__array_pplusplus"; break;
      case _ARRAY_MINUSMINUS_TOKEN:
	b = "__array_minusminus"; break;
      case _ARRAY_POST_MINUSMINUS_TOKEN:
	b = "__array_pminusminus"; break;

      case _DEREF_ASSIGN_TOKEN:
	b = "__deref_eqs"; break;
      case _DEREF_PLUSEQS_TOKEN:
	b = "__deref_pluseqs"; break;
      case _DEREF_MINUSEQS_TOKEN:
	b = "__deref_minuseqs"; break;
      case _DEREF_PLUSPLUS_TOKEN:
	b = "__deref_plusplus"; break;
      case _DEREF_POST_PLUSPLUS_TOKEN:
	b = "__deref_pplusplus"; break;
      case _DEREF_MINUSMINUS_TOKEN:
	b = "__deref_minusminus"; break;
      case _DEREF_POST_MINUSMINUS_TOKEN:
	b = "__deref_pminusminus"; break;

      case _DEREF_OBSOLETE_FUNCALL_TOKEN:
	b = "__deref_funcall ***WARNING: OBSOLETE form"; break;

      case _DEREF_FUNCALL_TOKEN:
	b = "__deref_funcall"; break;

      case _NULL_TOKEN: b = "NULL"; break;

      case USING_TOKEN:
	b = "__using__"; break;
      case FOREACH_TOKEN:
	b = "__foreach__"; break;

      case LOOP_THEN_TOKEN:
	b = "__loopthen__"; break;
#if 0
      case LOOP_ELSE_TOKEN:
	b = "__loopelse__"; break;
#endif
      case TRY_TOKEN:
	b = "try"; break;

      case LINE_NUM_TOKEN:
        sprintf (buf, "__line_num__ %ld", t->v.long_val); break;

      case _INLINE_WILDCARD_ARRAY_TOKEN:
	sprintf (buf, "__inline_wildcard_array__"); break;

      case BOS_TOKEN:
	b = "__bos__"; break;
      case EOS_TOKEN:
	b = "__eos__"; break;
      case THROW_TOKEN:
	b = "__throw__"; break;

      default:
	sprintf (buf, "____UNKNOWN___0x%X", t->type);
	break;
     }

   fprintf (stdout, "0x%2X: %s\n", t->type, b);
}

int main (int argc, char **argv)
{
   char *file;

   if (argc == 2)
     {
	/* fprintf (stderr, "Usage: %s <filename>\n", argv[0]); */
	file = argv[1];
	/* return 1; */
     }
   else file = NULL;

   if (-1 == SLang_init_slang ())
     return 1;

   _pSLcompile_ptr = dump_token;
   _pSLang_Compile_BOSEOS = 3;
   _pSLang_Compile_BOFEOF = 1;
   SLang_load_file (file);

   return _pSLang_Error;
}

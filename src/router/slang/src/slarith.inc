/* -*- c -*- */

/* This include file is a template for defining arithmetic binary operations
 * on arithmetic types.  I realize that doing it this way is not very
 * elegant but it minimizes the number of lines of code and I believe it
 * promotes clarity.
 */

/* The following macros should be properly defined before including this file:
 *
 *   GENERIC_BINARY_FUNCTION:   The name of the binary function
 *   GENERIC_TYPE:              The class data type
 *   MOD_FUNCTION:	        The function to use for mod
 *   ABS_FUNCTION:              Name of the abs function
 *   SIGN_FUNCTION:             Name of the sign function
 *   GENERIC_UNARY_FUNCTION     Name of the unary function
 *
 * If GENERIC_BIT_OPERATIONS is defined, the bit-level binary operators
 * will get included.  If the data type has a power operation (SLANG_POW),
 * then POW_FUNCTION should be defined to return POW_RESULT_TYPE.  The code
 * below assumes that POW_RESULT_TYPE is a floating point type,
 * with intermediate computations performed in double precision.
 *
 * If division by zero errors should be trapped, the define TRAP_DIV_ZERO to 1
 */
#ifdef GENERIC_BINARY_FUNCTION

static int GENERIC_BINARY_FUNCTION
  (int op,
   SLtype a_type, VOID_STAR ap, SLuindex_Type na,
   SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
   VOID_STAR cp)
{
   GENERIC_TYPE *c, *a, *b;
# ifdef POW_FUNCTION
   POW_RESULT_TYPE *d;
# endif
   SLuindex_Type n;
# if SLANG_OPTIMIZE_FOR_SPEED < 2
   SLuindex_Type n_max, da, db;
# endif
   char *cc;

   (void) a_type;		       /* Both SLANG_INT_TYPE */
   (void) b_type;

   a = (GENERIC_TYPE *) ap;
   b = (GENERIC_TYPE *) bp;
   c = (GENERIC_TYPE *) cp;
   cc = (char *) cp;

# if SLANG_OPTIMIZE_FOR_SPEED < 2
   if (na == 1) da = 0; else da = 1;
   if (nb == 1) db = 0; else db = 1;

   if (na > nb) n_max = na; else n_max = nb;
# endif

   switch (op)
     {
      default:
	return 0;
# if !defined(JUST_BOOLEAN_BINARY_OPS)
#  ifdef POW_FUNCTION
      case SLANG_POW:
	d = (POW_RESULT_TYPE *) cp;
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     d[n] = POW_FUNCTION(*a, *b);
	     a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       d[n] = POW_FUNCTION(a[n],b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     if (xb == 2)
	       for (n = 0; n < na; n++)
		 d[n] = (double)a[n] * (double)a[n];
	     else
	       for (n = 0; n < na; n++)
		 d[n] = POW_FUNCTION(a[n], xb);
	  }
	else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       d[n] = POW_FUNCTION(xa, b[n]);
	  }
#   endif
	break;
#  endif
      case SLANG_PLUS:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a + *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] + b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] + xb;
	  }
	else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa + b[n];
	  }
#  endif
	break;

      case SLANG_MINUS:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a - *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] - b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] - xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa - b[n];
	  }
#  endif
	break;

      case SLANG_TIMES:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a * *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] * b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] * xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa * b[n];
	  }
#  endif
	break;

      case SLANG_DIVIDE:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
#   if TRAP_DIV_ZERO
	     if (*b == 0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#   endif
	     c[n] = (*a / *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       {
#   if TRAP_DIV_ZERO
		  if (b[n] == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
#   endif
		  c[n] = a[n] / b[n];
	       }
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
#   if TRAP_DIV_ZERO
	     if (xb == 0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#   endif
	     for (n = 0; n < na; n++)
	       c[n] = a[n] / xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       {
#   if TRAP_DIV_ZERO
		  if (b[n] == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
#   endif
		  c[n] = xa / b[n];
	       }
	  }
#  endif
	break;

      case SLANG_MOD:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
#   if TRAP_DIV_ZERO
	     if (*b == 0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#   endif
	     c[n] = MOD_FUNCTION(*a, *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       {
#   if TRAP_DIV_ZERO
		  if (b[n] == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
#   endif
		  c[n] = MOD_FUNCTION(a[n],b[n]);
	       }
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
#   if TRAP_DIV_ZERO
	     if (xb == 0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#   endif
	     for (n = 0; n < na; n++)
	       c[n] = MOD_FUNCTION(a[n],xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       {
#   if TRAP_DIV_ZERO
		  if (b[n] == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
#   endif
		  c[n] = MOD_FUNCTION(xa,b[n]);
	       }
	  }
#  endif
	break;

#  ifdef GENERIC_BIT_OPERATIONS
      case SLANG_BAND:
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a & *b); a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] & b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] & xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa & b[n];
	  }
#   endif
	break;

      case SLANG_BXOR:
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a ^ *b); a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] ^ b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] ^ xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa ^ b[n];
	  }
#   endif
	break;

      case SLANG_BOR:
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a | *b); a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] | b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] | xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa | b[n];
	  }
#   endif
	break;

      case SLANG_SHL:
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a << *b); a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] << b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] << xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa << b[n];
	  }
#   endif
	break;

      case SLANG_SHR:
#   if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (*a >> *b); a += da; b += db;
	  }
#   else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       c[n] = a[n] >> b[n];
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       c[n] = a[n] >> xb;
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       c[n] = xa >> b[n];
	  }
#   endif
	break;
#  endif 			       /* GENERIC_BIT_OPERATIONS */
      case SLANG_GT:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a > *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] > b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] > xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa > b[n]);
	  }
#  endif
	break;

      case SLANG_GE:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a >= *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] >= b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] >= xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa >= b[n]);
	  }
#  endif
	break;

      case SLANG_LT:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a < *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] < b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] < xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa < b[n]);
	  }
#  endif
	break;

      case SLANG_LE:
#  if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a <= *b); a += da; b += db;
	  }
#  else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] <= b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] <= xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa <= b[n]);
	  }
#  endif
	break;
# endif			       /* !defined(JUST_BOOLEAN_BINARY_OPS) */

	/* Boolean operations */
      case SLANG_EQ:
# if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a == *b); a += da; b += db;
	  }
# else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] == b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] == xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa == b[n]);
	  }
# endif
	break;

      case SLANG_NE:
# if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a != *b); a += da; b += db;
	  }
# else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] != b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] != xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa != b[n]);
	  }
# endif
	break;

      case SLANG_OR:
# if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a || *b); a += da; b += db;
	  }
# else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] || b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] || xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa || b[n]);
	  }
# endif
	break;

      case SLANG_AND:
# if SLANG_OPTIMIZE_FOR_SPEED < 2
	for (n = 0; n < n_max; n++)
	  {
	     cc[n] = (*a && *b); a += da; b += db;
	  }
# else
	if (na == nb)
	  {
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] && b[n]);
	  }
	else if (nb == 1)
	  {
	     GENERIC_TYPE xb = *b;
	     for (n = 0; n < na; n++)
	       cc[n] = (a[n] && xb);
	  }
        else /* if (na == 1) */
	  {
	     GENERIC_TYPE xa = *a;
	     for (n = 0; n < nb; n++)
	       cc[n] = (xa && b[n]);
	  }
# endif
	break;
     }
   return 1;
}

#endif				       /* GENERIC_BINARY_FUNCTION */

#ifdef GENERIC_UNARY_FUNCTION

static int GENERIC_UNARY_FUNCTION
  (int op,
   SLtype a_type, VOID_STAR ap, SLuindex_Type na,
   VOID_STAR bp
   )
{
   GENERIC_TYPE *a, *b;
   SLuindex_Type n;
   int *ib;
   char *cb;

   (void) a_type;

   a = (GENERIC_TYPE *) ap;
   b = (GENERIC_TYPE *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUSPLUS:
	for (n = 0; n < na; n++) b[n] = (a[n] + 1);
	break;
      case SLANG_MINUSMINUS:
	for (n = 0; n < na; n++) b[n] = (a[n] - 1);
	break;
      case SLANG_ABS:
	for (n = 0; n < na; n++) b[n] = ABS_FUNCTION (a[n]);
	break;
      case SLANG_SIGN:
	ib = (int *) bp;
	for (n = 0; n < na; n++)
	  ib[n] = SIGN_FUNCTION(a[n]);
	break;
      case SLANG_SQR:
	for (n = 0; n < na; n++) b[n] = (a[n] * a[n]);
	break;
      case SLANG_MUL2:
	for (n = 0; n < na; n++) b[n] = (2 * a[n]);
	break;
      case SLANG_CHS:
	for (n = 0; n < na; n++) b[n] = (GENERIC_TYPE) -(a[n]);
	break;

      case SLANG_NOT:
	cb = (char *) bp;
	for (n = 0; n < na; n++) cb[n] = (a[n] == 0);
	break;

# ifdef GENERIC_BIT_OPERATIONS
      case SLANG_BNOT:
	for (n = 0; n < na; n++) b[n] = ~(a[n]);
	break;
# endif
      case SLANG_ISPOS:
	cb = (char *) bp;
	for (n = 0; n < na; n++) cb[n] = (a[n] > 0);
	break;
      case SLANG_ISNEG:
	cb = (char *) bp;
# ifdef GENERIC_TYPE_IS_UNSIGNED
	for (n = 0; n < na; n++) cb[n] = 0;
# else
	for (n = 0; n < na; n++) cb[n] = (a[n] < 0);
# endif
	break;
      case SLANG_ISNONNEG:
	cb = (char *) bp;
# ifdef GENERIC_TYPE_IS_UNSIGNED
	for (n = 0; n < na; n++) cb[n] = 1;
# else
	for (n = 0; n < na; n++) cb[n] = (a[n] >= 0);
# endif
	break;
     }

   return 1;
}
#endif				       /* GENERIC_UNARY_FUNCTION */

#ifdef SCALAR_BINARY_FUNCTION

static int SCALAR_BINARY_FUNCTION (GENERIC_TYPE a, GENERIC_TYPE b, int op)
{
   switch (op)
     {
      default:
	return 1;
# if SLANG_HAS_FLOAT
#  ifdef POW_FUNCTION
      case SLANG_POW:
	return PUSH_POW_OBJ_FUN(POW_FUNCTION(a, b));
#  endif
# endif
      case SLANG_PLUS:
	return PUSH_SCALAR_OBJ_FUN (a + b);
      case SLANG_MINUS:
	return PUSH_SCALAR_OBJ_FUN (a - b);
      case SLANG_TIMES:
	return PUSH_SCALAR_OBJ_FUN (a * b);
      case SLANG_DIVIDE:
# if TRAP_DIV_ZERO
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
# endif
	return PUSH_SCALAR_OBJ_FUN (a / b);
      case SLANG_MOD:
# if TRAP_DIV_ZERO
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
# endif
	return PUSH_SCALAR_OBJ_FUN (MOD_FUNCTION(a,b));
# ifdef GENERIC_BIT_OPERATIONS
      case SLANG_BAND:
	return PUSH_SCALAR_OBJ_FUN (a & b);
      case SLANG_BXOR:
	return PUSH_SCALAR_OBJ_FUN (a ^ b);
      case SLANG_BOR:
	return PUSH_SCALAR_OBJ_FUN (a | b);
      case SLANG_SHL:
	return PUSH_SCALAR_OBJ_FUN (a << b);
      case SLANG_SHR:
	return PUSH_SCALAR_OBJ_FUN (a >> b);
# endif
      case SLANG_GT: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a > b));
      case SLANG_LT: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a < b));
      case SLANG_GE: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a >= b));
      case SLANG_LE: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a <= b));
      case SLANG_EQ: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a == b));
      case SLANG_NE: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a != b));
      case SLANG_OR: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a || b));
      case SLANG_AND: return SLclass_push_char_obj (SLANG_CHAR_TYPE, (char)(a && b));
     }
}

#endif				       /* SCALAR_BINARY_FUNCTION */

#ifdef CMP_FUNCTION
static int CMP_FUNCTION (SLtype unused, VOID_STAR a, VOID_STAR b, int *c)
{
   GENERIC_TYPE x, y;

   (void) unused;
   x = *(GENERIC_TYPE *) a;
   y = *(GENERIC_TYPE *) b;

   if (x > y) *c = 1;
   else if (x == y) *c = 0;
   else *c = -1;

   return 0;
}
#endif

#ifdef TO_BINARY_FUNCTION
static int TO_BINARY_FUNCTION (GENERIC_TYPE x, char *buf, unsigned int buflen,
			       unsigned int min_num_bits)
{
   GENERIC_TYPE x1;
   unsigned int nbits;
   char *s;

   nbits = 1;
   x1 = x;
   while (x1 >> 8) { x1 = x1 >> 8; nbits += 8; }
   while (x1 >> 4) { x1 = x1 >> 4; nbits += 4; }
   while (x1 >> 1) { x1 = x1 >> 1; nbits += 1; }

   if (nbits >= buflen)
     return -1;			       /* calling routine must issue error */

   if (nbits < min_num_bits)
     {
	unsigned int dnbits = min_num_bits - nbits;
	if (nbits + dnbits + 1 > buflen)
	  dnbits = buflen - (nbits + 1);
	nbits += dnbits;
     }
   s = buf + nbits;
   *s = 0;
   while (x)
     {
	*(--s) = '0' + (x & 1);
	x = x >> 1;
     }
   while (s > buf)
     *(--s) = '0';

   return 0;
}
#endif

#undef TO_BINARY_FUNCTION
#undef CMP_FUNCTION
#undef SCALAR_BINARY_FUNCTION
#undef PUSH_POW_OBJ_FUN
#undef PUSH_SCALAR_OBJ_FUN
#undef GENERIC_BINARY_FUNCTION
#undef GENERIC_UNARY_FUNCTION
#undef GENERIC_BIT_OPERATIONS
#undef GENERIC_TYPE
#undef POW_FUNCTION
#undef POW_RESULT_TYPE
#undef MOD_FUNCTION
#undef ABS_FUNCTION
#undef SIGN_FUNCTION
#undef GENERIC_TYPE_IS_UNSIGNED
#undef GENERIC_ARITH_UNARY_FUNCTION
#undef JUST_BOOLEAN_BINARY_OPS
#undef TRAP_DIV_ZERO

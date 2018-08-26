/* sscanf function for S-Lang */
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
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "slang.h"
#include "_slang.h"

static void init_map (unsigned char map[256], int base)
{
   memset ((char *) map, 0xFF, 256);

   map['0'] = 0;   map['1'] = 1;   map['2'] = 2;   map['3'] = 3;
   map['4'] = 4;   map['5'] = 5;   map['6'] = 6;   map['7'] = 7;
   if (base == 8)
     return;

   map['8'] = 8;   map['9'] = 9;
   if (base == 10)
     return;

   map['A'] = 10;   map['B'] = 11;   map['C'] = 12;   map['D'] = 13;
   map['E'] = 14;   map['F'] = 15;   map['a'] = 10;   map['b'] = 11;
   map['c'] = 12;   map['d'] = 13;   map['e'] = 14;   map['f'] = 15;
}

static SLFUTURE_CONST char *get_sign (SLFUTURE_CONST char *s, SLFUTURE_CONST char *smax, int *sign)
{
   *sign = 1;
   if (s + 1 < smax)
     {
	if (*s == '+') s++;
	else if (*s == '-')
	  {
	     s++;
	     *sign = -1;
	  }
     }
   return s;
}

static int parse_long (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, long *np,
		       long base, unsigned char map[256])
{
   SLFUTURE_CONST char *s, *s0;
   long n;
   int sign;

   s = s0 = get_sign (*sp, smax, &sign);

   n = 0;
   while (s < smax)
     {
	unsigned char value;

	value = map [(unsigned char) *s];
	if (value == 0xFF)
	  break;

	n = base * n + value;
	s++;
     }

   *sp = s;
   if (s == s0)
     return 0;

   *np = n * sign;

   return 1;
}

static int parse_int (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, int *np,
		      long base, unsigned char map[256])
{
   long n;
   int status;

   if (1 == (status = parse_long (sp, smax, &n, base, map)))
     *np = (int) n;
   return status;
}

static int parse_short (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, short *np,
			long base, unsigned char map[256])
{
   long n;
   int status;

   if (1 == (status = parse_long (sp, smax, &n, base, map)))
     *np = (short) n;
   return status;
}

static int parse_ulong (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, unsigned long *np,
			long base, unsigned char map[256])
{
   return parse_long (sp, smax, (long *) np, base, map);
}

static int parse_uint (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, unsigned int *np,
		       long base, unsigned char map[256])
{
   return parse_int (sp, smax, (int *) np, base, map);
}

static int parse_ushort (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, unsigned short *np,
			 long base, unsigned char map[256])
{
   return parse_short (sp, smax, (short *) np, base, map);
}

#if SLANG_HAS_FLOAT
/*
 * In an ideal world, strtod would be the correct function to use.  However,
 * there may be problems relying on this function because some systems do
 * not support and some that do get it wrong.  So, I will handle the parsing
 * of the string and let atof or strtod handle the arithmetic.
 */
static int parse_double (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, double *d)
{
   SLFUTURE_CONST char *s, *s0;
   int sign;
   int expon;
   unsigned char map[256];
   char buf[128];
   int has_leading_zeros;
   SLFUTURE_CONST char *start_pos, *sign_pos;
   char *b, *bmax;
   char ch;

   start_pos = *sp;
   s = get_sign (start_pos, smax, &sign);
   if (s >= smax)
     {
	errno = _pSLerrno_errno = EINVAL;
	return 0;
     }

   ch = *s|0x20;
   if ((ch == 'n') || (ch == 'i'))
     {
	if (s + 3 <= smax)
	  {
	     if (ch == 'n')
	       {
		  if (((s[1]|0x20) == 'a') && ((s[2]|0x20) == 'n'))
		    {
		       s += 3;
		       s0 = s;
		       /* Now parse the nan(chars) form.  Here we allow
			*   ([a-zA-Z_0-9]*)
			*/
		       if ((s < smax) && (*s == '('))
			 {
			    s++;
			    while (s < smax)
			      {
				 ch = *s++;
				 if (isdigit(ch)
				     || ((ch >= 'a') && (ch <= 'z'))
				     || ((ch >= 'A') && (ch <= 'Z'))
				     || (ch == '_'))
				   continue;

				 if (ch == ')')
				   s0 = s;

				 break;
			      }
			 }
		       *sp = s0;
		       *d = _pSLang_NaN;
		       return 1;
		    }
		  *sp = start_pos;
		  errno = _pSLerrno_errno = EINVAL;
		  return 0;
	       }
	     if (((s[1] | 0x20) == 'n') && ((s[2] | 0x20) == 'f'))
	       {
		  /* check for infinity */
		  if ((s + 8 <= smax)
		      && (((s[3]|0x20)=='i')&&((s[4]|0x20)=='n')&&((s[5]|0x20)=='i')
		     && ((s[6]|0x20)=='t')&&((s[7]|0x20)=='y')))
		    *sp = s + 8;
		  else
		    *sp = s + 3;
		  *d = _pSLang_Inf * sign;
		  return 1;
	       }
	  }
	*sp = start_pos;
	errno = _pSLerrno_errno = EINVAL;
	return 0;
     }

   /* Prepare the buffer that will be passed to strtod */
   /* Allow the exponent to be 5 significant digits: E+xxxxx\0 */
   bmax = buf + (sizeof (buf) - 8);
   buf[0] = '0'; buf[1] = '.';
   b = buf + 2;

   init_map (map, 10);

   /* Skip leading 0s */
   s0 = s;
   while ((s < smax) && (*s == '0'))
     s++;
   has_leading_zeros = (s != s0);

   expon = 0;
   while (s < smax)
     {
	unsigned char value = map [(unsigned char) *s];

	if (value == 0xFF)
	  break;

	if (b < bmax)
	  *b++ = *s;

	expon++;
	s++;
     }

   if ((s < smax) && (*s == '.'))
     {
	s++;
	if (b == buf + 2)	       /* nothing added yet */
	  {
	     while ((s < smax) && (*s == '0'))
	       {
		  expon--;
		  s++;
	       }
	  }

	while (s < smax)
	  {
	     unsigned char value = map [(unsigned char) *s];

	     if (value == 0xFF)
	       break;

	     if (b < bmax)
	       *b++ = *s;
	     s++;
	  }
     }

   if ((b == buf + 2)
       && (has_leading_zeros == 0))
     {
	*sp = start_pos;
	errno = EINVAL;
	return 0;
     }

   if ((s + 1 < smax) && ((*s == 'E') || (*s == 'e')))
     {
	int e;
	int esign;

	s0 = s;
	s = get_sign (s + 1, smax, &esign);
	sign_pos = s;
	e = 0;
	while (s < smax)
	  {
	     unsigned char value = map [(unsigned char) *s];
	     if (value == 0xFF)
	       break;
	     if (e < 25000)	       /* avoid overflow if 16 bit */
	       e = 10 * e + value;
	     s++;
	  }
#ifdef ERANGE
	if (e >= 25000)
	  errno = ERANGE;
#endif
	if (s == sign_pos)
	  s = s0;		       /* ...E-X */
	else
	  {
	     e = esign * e;
	     expon += e;
	  }
     }

   if (expon != 0)
     sprintf (b, "e%d", expon);
   else
     *b = 0;

   *sp = s;

   /* fprintf (stdout, "buf='%s'\n", buf); */
#ifdef HAVE_STRTOD
   *d = sign * strtod (buf, NULL);
#else
   *d = sign * atof (buf);
#endif
   return 1;
}

static int parse_float (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, float *d)
{
   double x;
   if (1 == parse_double (sp, smax, &x))
     {
	*d = (float) x;
	return 1;
     }
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */

static int parse_string (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, char **str)
{
   SLFUTURE_CONST char *s, *s0;

   s0 = s = *sp;
   while (s < smax)
     {
	if (isspace (*s))
	  break;
	s++;
     }
   if (NULL == (*str = SLang_create_nslstring (s0, (unsigned int) (s - s0))))
     return -1;

   *sp = s;
   return 1;
}

static int parse_bstring (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, char **str)
{
   SLFUTURE_CONST char *s;

   s = *sp;
   if (NULL == (*str = SLang_create_nslstring (s, (unsigned int) (smax - s))))
     return -1;

   *sp = smax;
   return 1;
}

static int parse_range (SLFUTURE_CONST char **sp, SLFUTURE_CONST char *smax, SLFUTURE_CONST char **fp, char **str)
{
   SLFUTURE_CONST char *s, *s0;
   char *range;
   SLFUTURE_CONST char *f;
   unsigned char map[256];
   unsigned char reverse;

   /* How can one represent a range with just '^'?  The naive answer is
    * is [^].  However, this may be interpreted as meaning any character
    * but ']' and others.  Let's assume that the user will not use a range
    * to match '^'.
    */
   f = *fp;
   /* f is a pointer to (one char after) [...]. */
   if (*f == '^')
     {
	f++;
	reverse = 1;
     }
   else reverse = 0;

   s0 = f;
   if (*f == ']')
     f++;

   while (1)
     {
	char ch = *f;

	if (ch == 0)
	  {
	     _pSLang_verror (SL_INVALID_PARM, "Unexpected end of range in format");
	     return -1;
	  }
	if (ch == ']')
	  break;
	f++;
     }
   if (NULL == (range = SLmake_nstring (s0, (unsigned int) (f - s0))))
     return -1;
   *fp = f + 1;			       /* skip ] */

   SLmake_lut (map, (unsigned char *) range, reverse);
   SLfree (range);

   s0 = s = *sp;
   while ((s < smax) && map [(unsigned char) *s])
     s++;

   if (NULL == (*str = SLang_create_nslstring (s0, (unsigned int) (s - s0))))
     return -1;

   *sp = s;
   return 1;
}

/* FIXME: This function does not handle LONG_LONG */
int _pSLang_sscanf (void)
{
   int num;
   unsigned int num_refs;
   char *format;
   char *input_string, *input_string_max;
   SLFUTURE_CONST char *f, *s;
   unsigned char map8[256], map10[256], map16[256];

   if (SLang_Num_Function_Args < 2)
     {
	_pSLang_verror (SL_INVALID_PARM, "Int_Type sscanf (str, format, ...)");
	return -1;
     }

   num_refs = (unsigned int) SLang_Num_Function_Args;
   if (-1 == SLreverse_stack (num_refs))
     return -1;
   num_refs -= 2;

   if (-1 == SLang_pop_slstring (&input_string))
     return -1;

   if (-1 == SLang_pop_slstring (&format))
     {
	SLang_free_slstring (input_string);
	return -1;
     }

   f = format;
   s = input_string;
   input_string_max = input_string + strlen (input_string);

   init_map (map8, 8);
   init_map (map10, 10);
   init_map (map16, 16);

   num = 0;

   while (num_refs != 0)
     {
	SLang_Object_Type obj;
	SLang_Ref_Type *ref;
	SLFUTURE_CONST char *smax;
	unsigned char *map;
	int base;
	int no_assign;
	int is_short;
	int is_long;
	int status;
	char chf;
	unsigned int width;
	int has_width;

	chf = *f++;

	if (chf == 0)
	  {
	     /* Hmmm....  what is the most useful thing to do?? */
#if 1
	     break;
#else
	     _pSLang_verror (SL_INVALID_PARM, "sscanf: format not big enough for output list");
	     goto return_error;
#endif
	  }

	if (isspace (chf))
	  {
	     char *s1 = _pSLskip_whitespace (s);
	     if (s1 == s)
	       break;
	     s = s1;
	     continue;
	  }

	if ((chf != '%')
	    || ((chf = *f++) == '%'))
	  {
	     if (*s != chf)
	       break;
	     s++;
	     continue;
	  }

	no_assign = 0;
	is_short = 0;
	is_long = 0;
	width = 0;
	smax = input_string_max;

	/* Look for the flag character */
	if (chf == '*')
	  {
	     no_assign = 1;
	     chf = *f++;
	  }

	/* Width */
	has_width = isdigit (chf);
	if (has_width)
	  {
	     f--;
	     (void) parse_uint (&f, f + strlen(f), &width, 10, map10);
	     chf = *f++;
	  }

	/* Now the type modifier */
	switch (chf)
	  {
	   case 'h':
	     is_short = 1;
	     chf = *f++;
	     break;

	   case 'L':		       /* not implemented */
	   case 'l':
	     is_long = 1;
	     chf = *f++;
	     break;
	  }

	status = -1;

	if ((chf != 'c') && (chf != '['))
	  {
	     s = _pSLskip_whitespace (s);
	     if (*s == 0)
	       break;
	  }

	if (has_width)
	  {
	     if (width > (unsigned int) (input_string_max - s))
	       width = (unsigned int) (input_string_max - s);
	     smax = s + width;
	  }

	/* Now the format descriptor */

	map = map10;
	base = 10;

	try_again:		       /* used by i, x, and o, conversions */
	switch (chf)
	  {
	   case 0:
	     _pSLang_verror (SL_INVALID_PARM, "sscanf: Unexpected end of format");
	     goto return_error;
	   case 'D':
	     is_long = 1;
	   case 'd':
	     if (is_short)
	       {
		  obj.o_data_type = SLANG_SHORT_TYPE;
		  status = parse_short (&s, smax, &obj.v.short_val, base, map);
	       }
	     else if (is_long)
	       {
		  obj.o_data_type = SLANG_LONG_TYPE;
		  status = parse_long (&s, smax, &obj.v.long_val, base, map);
	       }
	     else
	       {
		  obj.o_data_type = SLANG_INT_TYPE;
		  status = parse_int (&s, smax, &obj.v.int_val, base, map);
	       }
	     break;

	   case 'U':
	     is_long = 1;
	   case 'u':
	     if (is_short)
	       {
		  obj.o_data_type = SLANG_USHORT_TYPE;
		  status = parse_ushort (&s, smax, &obj.v.ushort_val, base, map);
	       }
	     else if (is_long)
	       {
		  obj.o_data_type = SLANG_ULONG_TYPE;
		  status = parse_ulong (&s, smax, &obj.v.ulong_val, base, map);
	       }
	     else
	       {
		  obj.o_data_type = SLANG_INT_TYPE;
		  status = parse_uint (&s, smax, &obj.v.uint_val, base, map);
	       }
	     break;

	   case 'I':
	     is_long = 1;
	   case 'i':
	     if ((s + 1 >= smax)
		 || (*s != 0))
	       chf = 'd';
	     else if (((s[1] == 'x') || (s[1] == 'X'))
		      && (s + 2 < smax))
	       {
		  s += 2;
		  chf = 'x';
	       }
	     else chf = 'o';
	     goto try_again;

	   case 'O':
	     is_long = 1;
	   case 'o':
	     map = map8;
	     base = 8;
	     chf = 'd';
	     goto try_again;

	   case 'X':
	     is_long = 1;
	   case 'x':
	     base = 16;
	     map = map16;
	     chf = 'd';
	     goto try_again;

	   case 'E':
	   case 'F':
	     is_long = 1;
	   case 'e':
	   case 'f':
	   case 'g':
#if SLANG_HAS_FLOAT
	     if (is_long)
	       {
		  obj.o_data_type = SLANG_DOUBLE_TYPE;
		  status = parse_double (&s, smax, &obj.v.double_val);
	       }
	     else
	       {
		  obj.o_data_type = SLANG_FLOAT_TYPE;
		  status = parse_float (&s, smax, &obj.v.float_val);
	       }
#else
	     _pSLang_verror (SL_NOT_IMPLEMENTED,
			   "This version of the S-Lang does not support floating point");
	     status = -1;
#endif
	     break;

	   case 's':
	     obj.o_data_type = SLANG_STRING_TYPE;
	     status = parse_string (&s, smax, &obj.v.s_val);
	     break;

	   case 'c':
	     if (has_width == 0)
	       {
		  obj.o_data_type = SLANG_UCHAR_TYPE;
		  obj.v.uchar_val = *s++;
		  status = 1;
		  break;
	       }
	     obj.o_data_type = SLANG_STRING_TYPE;
	     status = parse_bstring (&s, smax, &obj.v.s_val);
	     break;

	   case '[':
	     obj.o_data_type = SLANG_STRING_TYPE;
	     status = parse_range (&s, smax, &f, &obj.v.s_val);
	     break;

	   case 'n':
	     obj.o_data_type = SLANG_UINT_TYPE;
	     obj.v.uint_val = (unsigned int) (s - input_string);
	     status = 1;
	     break;

	   default:
	     status = -1;
	     _pSLang_verror (SL_NOT_IMPLEMENTED, "format specifier '%c' is not supported", chf);
	     break;
	  }

	if (status == 0)
	  break;

	if (status == -1)
	  goto return_error;

	if (no_assign)
	  {
	     SLang_free_object (&obj);
	     continue;
	  }

	if (-1 == SLang_pop_ref (&ref))
	  {
	     SLang_free_object (&obj);
	     goto return_error;
	  }

	if (-1 == SLang_push (&obj))
	  {
	     SLang_free_object (&obj);
	     SLang_free_ref (ref);
	     goto return_error;
	  }

	if (-1 == _pSLang_deref_assign (ref))
	  {
	     SLang_free_ref (ref);
	     goto return_error;
	  }
	SLang_free_ref (ref);

	num++;
	num_refs--;
     }

   if (-1 == SLdo_pop_n (num_refs))
     goto return_error;

   SLang_free_slstring (format);
   SLang_free_slstring (input_string);
   return num;

   return_error:
   /* NULLS ok */
   SLang_free_slstring (format);
   SLang_free_slstring (input_string);
   return -1;
}

# if SLANG_HAS_FLOAT

#ifndef HAVE_STDLIB_H
/* Oh dear.  Where is the prototype for atof?  If not in stdlib, then
 * I do not know where.  Not in math.h on some systems either.
 */
extern double atof ();
#endif

double _pSLang_atof (SLFUTURE_CONST char *s)
{
   double x;

   s = _pSLskip_whitespace (s);
   errno = 0;

   if (1 != parse_double (&s, s + strlen (s), &x))
     {
	if ((0 == strcmp ("NaN", s))
	    || (0 == strcmp ("-Inf", s))
	    || (0 == strcmp ("Inf", s)))
	  return atof (s);	       /* let this deal with it */
#ifdef EINVAL
	errno = _pSLerrno_errno = EINVAL;
#endif
	return 0.0;
     }
   if (errno)
     _pSLerrno_errno = errno;
   return x;
}
#endif

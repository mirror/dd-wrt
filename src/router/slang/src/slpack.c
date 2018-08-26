/* Pack objects as a binary string */
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

#include "slang.h"
#include "_slang.h"

#ifndef isdigit
# define isdigit(c) (((c)>='0')&&((c)<= '9'))
#endif
#ifndef isspace
# define isspace(c) (((c)==' ') || ((c)=='\t') || ((c)=='\n'))
#endif

/* format description:
 *
 *    s = string (null padded, except when unpacking)
 *    S = string (space padded)
 *    z = string (null padded)
 *    c = signed char
 *    C = unsigned char
 *    h = short
 *    H = unsigned short
 *    i = int
 *    I = unsigned int
 *    l = long
 *    L = unsigned long
 *    j = 16 bit signed integer (short)
 *    J = 16 bit unsigned integer (short)
 *    k = 32 bit signed integer (long)
 *    K = 32 bit unsigned integer (long)
 *    q = 64 bit unsigned integer (long)
 *    Q = 64 bit unsigned integer (long)
 *    f = float (native format)
 *    F = 32 bit double
 *    d = double (native format)
 *    D = 64 bit double
 *    x = null pad byte
 *    > = big-endian mode
 *    < = little-endian mode
 *    = = native mode
 */

#define NATIVE_ORDER		0
#define BIGENDIAN_ORDER		1
#define LILENDIAN_ORDER		2
static int Native_Byte_Order = NATIVE_ORDER;

typedef struct
{
   char format_type;
   SLtype data_type;
   unsigned int repeat;
   unsigned int sizeof_type;
   char pad;
   int byteorder;
   int is_scalar;
}
Format_Type;

static int get_int_type_for_size (unsigned int size, SLtype *s, SLtype *u)
{
   if (sizeof (int) == size)
     {
	if (s != NULL) *s = SLANG_INT_TYPE;
	if (u != NULL) *u = SLANG_UINT_TYPE;
	return 0;
     }

   if (sizeof (short) == size)
     {
	if (s != NULL) *s = SLANG_SHORT_TYPE;
	if (u != NULL) *u = SLANG_USHORT_TYPE;
	return 1;
     }

   if (sizeof (long) == size)
     {
	if (s != NULL) *s = SLANG_LONG_TYPE;
	if (u != NULL) *u = SLANG_ULONG_TYPE;
	return 1;
     }
#ifdef HAVE_LONG_LONG
   if (sizeof (long long) == size)
     {
	if (s != NULL) *s = SLANG_LLONG_TYPE;
	if (u != NULL) *u = SLANG_ULLONG_TYPE;
	return 1;
     }
#endif
   if (s != NULL) *s = 0;
   if (u != NULL) *u = 0;
   _pSLang_verror (SL_NOT_IMPLEMENTED,
		 "This OS does not support a %u byte int", size);
   return -1;
}

static int get_float_type_for_size (unsigned int size, SLtype *s)
{
   if (sizeof (float) == size)
     {
	*s = SLANG_FLOAT_TYPE;
	return 0;
     }

   if (sizeof (double) == size)
     {
	*s = SLANG_DOUBLE_TYPE;
	return 0;
     }

   _pSLang_verror (SL_NOT_IMPLEMENTED,
		 "This OS does not support a %u byte float", size);
   return -1;
}

static int parse_a_format (char **format, Format_Type *ft)
{
   char *f;
   char ch;
   unsigned int repeat;

   f = *format;

   while (((ch = *f++) != 0)
	  && isspace (ch))
     ;

   switch (ch)
     {
      default:
	ft->byteorder = NATIVE_ORDER;
	break;

      case '=':
	ft->byteorder = NATIVE_ORDER;
	ch = *f++;
	break;

      case '>':
	ft->byteorder = BIGENDIAN_ORDER;
	ch = *f++;
	break;

      case '<':
	ft->byteorder = LILENDIAN_ORDER;
	ch = *f++;
	break;
     }

   if (ch == 0)
     {
	f--;
	*format = f;
	return 0;
     }

   ft->format_type = ch;
   ft->repeat = 1;

   if (isdigit (*f))
     {
	repeat = (unsigned int) (*f - '0');
	f++;

	while (isdigit (*f))
	  {
	     unsigned int repeat10 = 10 * repeat + (unsigned int)(*f - '0');

	     /* Check overflow */
	     if (repeat != repeat10 / 10)
	       {
		  _pSLang_verror (SL_ARITH_OVERFLOW_ERROR,
				"Repeat count too large in [un]pack format");
		  return -1;
	       }
	     repeat = repeat10;
	     f++;
	  }
	ft->repeat = repeat;
     }

   *format = f;

   ft->is_scalar = 1;
   ft->pad = 0;

   switch (ft->format_type)
     {
      default:
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "[un]pack format character '%c' not supported", ft->format_type);
	return -1;

      case 'D':
	ft->sizeof_type = 8;
	if (-1 == get_float_type_for_size (8, &ft->data_type))
	  return -1;
	break;

      case 'd':
	ft->data_type = SLANG_DOUBLE_TYPE;
	ft->sizeof_type = sizeof (double);
	break;

      case 'F':
	ft->sizeof_type = 4;
	if (-1 == get_float_type_for_size (4, &ft->data_type))
	  return -1;
	break;
      case 'f':
	ft->data_type = SLANG_FLOAT_TYPE;
	ft->sizeof_type = sizeof (float);
	break;

      case 'h':
	ft->data_type = SLANG_SHORT_TYPE;
	ft->sizeof_type = sizeof (short);
	break;
      case 'H':
	ft->data_type = SLANG_USHORT_TYPE;
	ft->sizeof_type = sizeof (unsigned short);
	break;
      case 'i':
	ft->data_type = SLANG_INT_TYPE;
	ft->sizeof_type = sizeof (int);
	break;
      case 'I':
	ft->data_type = SLANG_UINT_TYPE;
	ft->sizeof_type = sizeof (unsigned int);
	break;
      case 'l':
	ft->data_type = SLANG_LONG_TYPE;
	ft->sizeof_type = sizeof (long);
	break;
      case 'L':
	ft->data_type = SLANG_ULONG_TYPE;
	ft->sizeof_type = sizeof (unsigned long);
	break;
#ifdef HAVE_LONG_LONG
      case 'm':
	ft->data_type = SLANG_LLONG_TYPE;
	ft->sizeof_type = sizeof (long long);
	break;
      case 'M':
	ft->data_type = SLANG_ULLONG_TYPE;
	ft->sizeof_type = sizeof (unsigned long long);
	break;
#endif

	/* 16 bit ints */
      case 'j':
	ft->sizeof_type = 2;
	if (-1 == get_int_type_for_size (2, &ft->data_type, NULL))
	  return -1;
	break;
      case 'J':
	ft->sizeof_type = 2;
	if (-1 == get_int_type_for_size (2, NULL, &ft->data_type))
	  return -1;
	break;

	/* 32 bit ints */
      case 'k':
	ft->sizeof_type = 4;
	if (-1 == get_int_type_for_size (4, &ft->data_type, NULL))
	  return -1;
	break;
      case 'K':
	ft->sizeof_type = 4;
	if (-1 == get_int_type_for_size (4, NULL, &ft->data_type))
	  return -1;
	break;

	/* 64 bit ints */
      case 'q':
	ft->sizeof_type = 8;
	if (-1 == get_int_type_for_size (8, &ft->data_type, NULL))
	  return -1;
	break;
      case 'Q':
	ft->sizeof_type = 8;
	if (-1 == get_int_type_for_size (8, NULL, &ft->data_type))
	  return -1;
	break;

      case 'x':
	ft->sizeof_type = 1;
	ft->data_type = 0;
	break;

      case 'c':
	ft->sizeof_type = 1;
	ft->data_type = SLANG_CHAR_TYPE;
	break;

      case 'C':
	ft->data_type = SLANG_UCHAR_TYPE;
	ft->sizeof_type = 1;
	break;

      case 'S':
      case 'A':
	ft->pad = ' ';
      case 'a':
      case 's':
      case 'z':
	ft->data_type = SLANG_BSTRING_TYPE;
	ft->sizeof_type = 1;
	ft->is_scalar = 0;
	break;
     }
   return 1;
}

static int compute_size_for_format (char *format, unsigned int *num_bytes)
{
   unsigned int size;
   Format_Type ft;
   int status;

   *num_bytes = size = 0;

   while (1 == (status = parse_a_format (&format, &ft)))
     size += ft.repeat * ft.sizeof_type;

   *num_bytes = size;
   return status;
}

static void byte_swap64 (unsigned char *ss, unsigned int n) /*{{{*/
{
   unsigned char *p, *pmax, ch;

   if (n == 0) return;
   p = (unsigned char *) ss;
   pmax = p + 8 * n;
   while (p < pmax)
     {
	ch = *p;
	*p = *(p + 7);
	*(p + 7) = ch;

	ch = *(p + 6);
	*(p + 6) = *(p + 1);
	*(p + 1) = ch;

	ch = *(p + 5);
	*(p + 5) = *(p + 2);
	*(p + 2) = ch;

	ch = *(p + 4);
	*(p + 4) = *(p + 3);
	*(p + 3) = ch;

	p += 8;
     }
}

/*}}}*/
static void byte_swap32 (unsigned char *ss, unsigned int n) /*{{{*/
{
   unsigned char *p, *pmax, ch;

   p = (unsigned char *) ss;
   pmax = p + 4 * n;
   while (p < pmax)
     {
	ch = *p;
	*p = *(p + 3);
	*(p + 3) = ch;

	ch = *(p + 1);
	*(p + 1) = *(p + 2);
	*(p + 2) = ch;
	p += 4;
     }
}

/*}}}*/
static void byte_swap16 (unsigned char *p, unsigned int nread) /*{{{*/
{
   unsigned char *pmax, ch;

   pmax = p + 2 * nread;
   while (p < pmax)
     {
	ch = *p;
	*p = *(p + 1);
	*(p + 1) = ch;
	p += 2;
     }
}

/*}}}*/

static int byteswap (int order, unsigned char *b,  unsigned int size, unsigned int num)
{
   if (Native_Byte_Order == order)
     return 0;

   switch (size)
     {
      case 2:
	byte_swap16 (b, num);
	break;
      case 4:
	byte_swap32 (b, num);
	break;
      case 8:
	byte_swap64 (b, num);
	break;
      default:
	return -1;
     }

   return 0;
}

static void check_native_byte_order (void)
{
   unsigned short x;

   if (Native_Byte_Order != NATIVE_ORDER)
     return;

   x = 0xFF;
   if (*(unsigned char *)&x == 0xFF)
     Native_Byte_Order = LILENDIAN_ORDER;
   else
     Native_Byte_Order = BIGENDIAN_ORDER;
}

static SLang_BString_Type *
pack_according_to_format (char *format, unsigned int nitems)
{
   unsigned int size, num;
   unsigned char *buf, *b;
   SLang_BString_Type *bs;
   Format_Type ft;

   buf = NULL;

   if (-1 == compute_size_for_format (format, &size))
     goto return_error;

   if (NULL == (buf = (unsigned char *) SLmalloc (size + 1)))
     goto return_error;

   b = buf;

   while (1 == parse_a_format (&format, &ft))
     {
	unsigned char *ptr;
	unsigned int repeat;

	repeat = ft.repeat;
	if (ft.data_type == 0)
	  {
	     memset ((char *) b, ft.pad, repeat);
	     b += repeat;
	     continue;
	  }

	if (ft.is_scalar)
	  {
	     unsigned char *bstart;
	     num = repeat;

	     bstart = b;
	     while (repeat != 0)
	       {
		  unsigned int nelements;
		  SLang_Array_Type *at;

		  if (nitems == 0)
		    {
		       _pSLang_verror (SL_INVALID_PARM,
				     "Not enough items for pack format");
		       goto return_error;
		    }

		  if (-1 == SLang_pop_array_of_type (&at, ft.data_type))
		    goto return_error;

		  nelements = at->num_elements;
		  if (repeat < nelements)
		    nelements = repeat;
		  repeat -= nelements;

		  nelements = nelements * ft.sizeof_type;
		  memcpy ((char *)b, (char *)at->data, nelements);

		  b += nelements;
		  SLang_free_array (at);
		  nitems--;
	       }

	     if (ft.byteorder != NATIVE_ORDER)
	       byteswap (ft.byteorder, bstart, ft.sizeof_type, num);

	     continue;
	  }

	/* Otherwise we have a string */
	if (-1 == SLang_pop_bstring (&bs))
	  goto return_error;

	ptr = SLbstring_get_pointer (bs, &num);
	if (repeat < num) num = repeat;
	memcpy ((char *)b, (char *)ptr, num);
	b += num;
	repeat -= num;
	if ((repeat == 0) && (ft.format_type == 'z'))
	  {
	     if (num) *(b-1) = 0;
	  }
	else memset ((char *)b, ft.pad, repeat);
	SLbstring_free (bs);
	b += repeat;
	nitems--;
     }

   *b = 0;
   bs = SLbstring_create_malloced (buf, size, 0);
   if (bs == NULL)
     goto return_error;

   SLdo_pop_n (nitems);
   return bs;

   return_error:
   SLdo_pop_n (nitems);
   if (buf != NULL)
     SLfree ((char *) buf);

   return NULL;
}

void _pSLpack (void)
{
   SLang_BString_Type *bs;
   char *fmt;
   int nitems;

   check_native_byte_order ();

   nitems = SLang_Num_Function_Args;
   if (nitems <= 0)
     {
	_pSLang_verror (SL_SYNTAX_ERROR,
		      "pack: not enough arguments");
	return;
     }

   if ((-1 == SLreverse_stack (nitems))
       || (-1 == SLang_pop_slstring (&fmt)))
     bs = NULL;
   else
     {
	bs = pack_according_to_format (fmt, (unsigned int)nitems - 1);
	SLang_free_slstring (fmt);
     }

   SLang_push_bstring (bs);
   SLbstring_free (bs);
}

static unsigned int get_unpadded_strlen (char *str, char pad, unsigned int len)
{
   char *s;

   s = str + len;
   while (s > str)
     {
	s--;
	if ((*s != pad) && (*s != 0))
	  {
	     s++;
	     break;
	  }
     }
   return (unsigned int) (s - str);
}

void _pSLunpack (char *format, SLang_BString_Type *bs)
{
   Format_Type ft;
   unsigned char *b;
   unsigned int len;
   unsigned int num_bytes;

   check_native_byte_order ();

   if (-1 == compute_size_for_format (format, &num_bytes))
     return;

   b = SLbstring_get_pointer (bs, &len);
   if (b == NULL)
     return;

   if (len < num_bytes)
     {
	_pSLang_verror (SL_INVALID_PARM,
		      "unpack format %s is too large for input string",
		      format);
	return;
     }

   while (1 == parse_a_format (&format, &ft))
     {
	char *str, *s;

	if (ft.repeat == 0)
	  continue;

	if (ft.data_type == 0)
	  {			       /* skip padding */
	     b += ft.repeat;
	     continue;
	  }

	if (ft.is_scalar)
	  {
	     SLang_Array_Type *at;
	     SLindex_Type dims;

	     if (ft.repeat == 1)
	       {
		  SLang_Class_Type *cl;

		  cl = _pSLclass_get_class (ft.data_type);
		  memcpy ((char *)cl->cl_transfer_buf, (char *)b, ft.sizeof_type);
		  if (ft.byteorder != NATIVE_ORDER)
		    byteswap (ft.byteorder, (unsigned char *)cl->cl_transfer_buf, ft.sizeof_type, 1);

		  if (-1 == (cl->cl_apush (ft.data_type, cl->cl_transfer_buf)))
		    return;
		  b += ft.sizeof_type;
		  continue;
	       }

	     dims = (SLindex_Type) ft.repeat;
	     at = SLang_create_array (ft.data_type, 0, NULL, &dims, 1);
	     if (at == NULL)
	       return;

	     num_bytes = ft.repeat * ft.sizeof_type;
	     memcpy ((char *)at->data, (char *)b, num_bytes);
	     if (ft.byteorder != NATIVE_ORDER)
	       byteswap (ft.byteorder, (unsigned char *)at->data, ft.sizeof_type, ft.repeat);

	     if (-1 == SLang_push_array (at, 1))
	       return;

	     b += num_bytes;
	     continue;
	  }

	/* string type: s, S, or Z */
	if (ft.format_type == 's')
	  len = ft.repeat;
	else
	  len = get_unpadded_strlen ((char *)b, ft.pad, ft.repeat);

	str = SLmalloc (len + 1);
	if (str == NULL)
	  return;
	memcpy ((char *) str, (char *)b, len);
	str [len] = 0;

	/* Avoid a bstring if possible */
	s = SLmemchr (str, 0, len);
	if (s == NULL)
	  {
	     if (-1 == SLang_push_malloced_string (str))
	       return;
	  }
	else
	  {
	     SLang_BString_Type *new_bs;

	     new_bs = SLbstring_create_malloced ((unsigned char *)str, len, 1);
	     if (new_bs == NULL)
	       return;

	     if (-1 == SLang_push_bstring (new_bs))
	       {
		  SLfree (str);
		  return;
	       }
	     SLbstring_free (new_bs);
	  }

	b += ft.repeat;
     }
}

unsigned int _pSLpack_compute_size (char *format)
{
   unsigned int n;

   n = 0;
   (void) compute_size_for_format (format, &n);
   return n;
}

void _pSLpack_pad_format (char *format)
{
   unsigned int len, max_len;
   Format_Type ft;
   char *buf, *b;

   check_native_byte_order ();

   /* Just check the syntax */
   if (-1 == compute_size_for_format (format, &max_len))
     return;

   /* This should be sufficient to handle any needed xyy padding characters.
    * I cannot see how this will be overrun
    */
   max_len = 4 * (strlen (format) + 1);
   if (NULL == (buf = SLmalloc (max_len + 1)))
     return;

   b = buf;
   len = 0;
   while (1 == parse_a_format (&format, &ft))
     {
	struct { char a; short b; } s_h;
	struct { char a; int b; } s_i;
	struct { char a; long b; } s_l;
	struct { char a; float b; } s_f;
	struct { char a; double b; } s_d;
	unsigned int pad;

	if (ft.repeat == 0)
	  continue;

	if (ft.data_type == 0)
	  {			       /* pad */
	     sprintf (b, "x%u", ft.repeat);
	     b += strlen (b);
	     len += ft.repeat;
	     continue;
	  }

	switch (ft.data_type)
	  {
	   default:
	   case SLANG_STRING_TYPE:
	   case SLANG_BSTRING_TYPE:
	   case SLANG_CHAR_TYPE:
	   case SLANG_UCHAR_TYPE:
	     pad = 0;
	     break;

	   case SLANG_SHORT_TYPE:
	   case SLANG_USHORT_TYPE:
	     pad = ((unsigned int) ((char *)&s_h.b - (char *)&s_h.a));
	     break;

	   case SLANG_INT_TYPE:
	   case SLANG_UINT_TYPE:
	     pad = ((unsigned int) ((char *)&s_i.b - (char *)&s_i.a));
	     break;

	   case SLANG_LONG_TYPE:
	   case SLANG_ULONG_TYPE:
	     pad = ((unsigned int) ((char *)&s_l.b - (char *)&s_l.a));
	     break;

	   case SLANG_FLOAT_TYPE:
	     pad = ((unsigned int) ((char *)&s_f.b - (char *)&s_f.a));
	     break;

	   case SLANG_DOUBLE_TYPE:
	     pad = ((unsigned int) ((char *)&s_d.b - (char *)&s_d.a));
	     break;
	  }

	/* Pad to a length that is an integer multiple of pad. */
	if (pad)
	  pad = pad * ((len + pad - 1)/pad) - len;

	if (pad)
	  {
	     sprintf (b, "x%u", pad);
	     b += strlen (b);
	     len += pad;
	  }

	*b++ = ft.format_type;
	if (ft.repeat > 1)
	  {
	     sprintf (b, "%u", ft.repeat);
	     b += strlen (b);
	  }

	len += ft.repeat * ft.sizeof_type;
     }
   *b = 0;

   (void) SLang_push_malloced_string (buf);
}

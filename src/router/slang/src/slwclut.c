/* slwclut.c: wide character lookup tables */
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
#include <string.h>

#include "slang.h"
#include "_slang.h"
#include "slischar.h"

#define IS_ASCII256(x) ((x) < 256)

struct SLwchar_Lut_Type
{
   unsigned char lut[256];             /* for chars < 256 */
   int utf8_mode;

   SLwchar_Type *chmin, *chmax;
   unsigned int table_len;
   unsigned int malloced_len;
   unsigned int char_class;
};

void SLwchar_free_lut (SLwchar_Lut_Type *r)
{
   if (r == NULL)
     return;

   SLfree ((char *) r->chmin);
   SLfree ((char *) r->chmax);

   SLfree ((char *) r);
}

SLwchar_Lut_Type *SLwchar_create_lut (unsigned int num_entries)
{
   SLwchar_Lut_Type *r;

   r = (SLwchar_Lut_Type *)SLcalloc (sizeof (SLwchar_Lut_Type), 1);
   if (r == NULL)
     return NULL;

   r->chmin = (SLwchar_Type *) _SLcalloc (num_entries, sizeof(SLwchar_Type));
   r->chmax = (SLwchar_Type *) _SLcalloc (num_entries, sizeof(SLwchar_Type));
   if ((r->chmin == NULL) || (r->chmax == NULL))
     {
        SLwchar_free_lut (r);
        return NULL;
     }

   r->malloced_len = num_entries;
   r->utf8_mode = _pSLinterp_UTF8_Mode;
   return r;
}

int SLwchar_add_range_to_lut (SLwchar_Lut_Type *r, SLwchar_Type a, SLwchar_Type b)
{
   if (b < a)
     {
        SLwchar_Type tmp = a;
        a = b;
        b = tmp;
     }

   if (b < 256)
     {
        unsigned char *lut = r->lut;
        while (a <= b)
          {
             lut[a] = 1;
             a++;
          }
        return 0;
     }

   if (a < 256)
     {
        if (-1 == SLwchar_add_range_to_lut (r, a, 255))
          return -1;

        a = 256;
     }

   if (r->table_len == r->malloced_len)
     {
        SLwchar_Type *chmin, *chmax;
        unsigned int malloced_len = r->malloced_len + 5;

        chmin = (SLwchar_Type *) _SLrecalloc ((char *)r->chmin, malloced_len, sizeof (SLwchar_Type));
        if (chmin == NULL)
          return -1;
        r->chmin = chmin;

        chmax = (SLwchar_Type *) _SLrecalloc ((char *)r->chmax, malloced_len, sizeof (SLwchar_Type));
        if (chmax == NULL)
          return -1;

        r->chmax = chmax;
        r->malloced_len = malloced_len;
     }

   r->chmin[r->table_len] = a;
   r->chmax[r->table_len] = b;

   r->table_len += 1;

   return 0;
}

static void add_char_class (SLwchar_Lut_Type *r, unsigned int char_class)
{
   unsigned int i;
   unsigned char *lut;

   r->char_class |= char_class;
   lut = r->lut;

   for (i = 0; i < 256; i++)
     {
	if (SL_CLASSIFICATION_LOOKUP(i) & char_class)
	  lut[i] = 1;
     }
}

static int wch_in_lut (SLwchar_Lut_Type *r, SLwchar_Type wch)
{
   unsigned int i, table_len;
   SLwchar_Type *chmin, *chmax;

   if (wch < 256)
     return r->lut[wch];

   if (r->char_class
       && (SL_CLASSIFICATION_LOOKUP(wch) & r->char_class))
     return 1;

   /* FIXME.  I should use a binary search for this... */
   table_len = r->table_len;
   chmin = r->chmin;
   chmax = r->chmax;

   for (i = 0; i < table_len; i++)
     {
	if ((wch <= chmax[i])
	    && (wch >= chmin[i]))
	  return 1;
     }
   return 0;
}

int SLwchar_in_lut (SLwchar_Lut_Type *r, SLwchar_Type wch)
{
   if (r == NULL)
     return -1;

   return wch_in_lut (r, wch);
}

SLuchar_Type *SLwchar_skip_range (SLwchar_Lut_Type *r, SLuchar_Type *p,
                                  SLuchar_Type *pmax, int ignore_combining,
                                  int invert)
{
   unsigned char *lut;
   int utf8_mode;

   if ((r == NULL) || (p == NULL) || (pmax == NULL))
     return NULL;

   lut = r->lut;
   invert = (invert != 0);
   utf8_mode = r->utf8_mode;

   while (p < pmax)
     {
        SLwchar_Type wch;
        SLstrlen_Type dn;

        if ((*p < 0x80)
	    || (utf8_mode == 0))
          {
             if ((int)lut[*p] == invert)
	       return p;

             p++;
             continue;
          }

        if (NULL == SLutf8_decode (p, pmax, &wch, &dn))
          {
             if (invert == 0)
               return p;

             p++;
             continue;
          }

	if ((ignore_combining)
	    && (0 == SLwchar_wcwidth (wch)))
	  {
	     p += dn;
	     continue;
	  }

	if (invert == wch_in_lut (r, wch))
	  return p;

        p += dn;
     }

   return p;
}

SLuchar_Type *SLwchar_bskip_range (SLwchar_Lut_Type *r, SLuchar_Type *pmin,
				   SLuchar_Type *p,
				   int ignore_combining,
				   int invert)
{
   unsigned char *lut;
   SLuchar_Type *pmax;
   int utf8_mode;

   if ((r == NULL) || (p == NULL) || (pmin == NULL))
     return NULL;

   lut = r->lut;
   pmax = p;

   invert = (invert != 0);
   utf8_mode = r->utf8_mode;

   while (p > pmin)
     {
	SLuchar_Type *p0;
        SLwchar_Type wch;
        SLstrlen_Type dn;

	p0 = p - 1;
        if ((*p0 < 0x80) || (utf8_mode == 0))
          {
             if ((int)lut[*p0] == invert)
	       return p;

	     p = p0;
	     continue;
	  }

	p0 = SLutf8_bskip_char (pmin, p);

        if (NULL == SLutf8_decode (p0, pmax, &wch, &dn))
          {
             if (invert)
               return p;

             p = p0;
             continue;
          }

	if ((ignore_combining)
	    && (0 == SLwchar_wcwidth (wch)))
	  {
	     p = p0;
	     continue;
	  }

	if (invert == wch_in_lut (r, wch))
	  return p;

        p = p0;
     }

   return p;
}

/*
 * Special Range characters:
 *
 * \w matches a unicode "word" character, taken to be alphanumeric.
 * \a alpha character, excluding digits
 * \s matches whitespace
 * \l matches lowercase
 * \u matches uppercase
 * \d matches a digit
 */

/* QUESTION: What is the encoding of the range?  Is it utf-8?  I suspect
 * it ought to be.  For example, a jed .sl file may use:
 *
 *    skip_chars ("\\w\u{ADFF}-\u{AFFF}");
 *
 * to skip words chars and chars in the range 0xADFF-0xAFFF.  By the time it
 * gets here, the parser will have converted the wchars \u{ADFF} and \u{AFFF}
 * to their UTF-8 equivalents.  Hence the function needs to use SLutf8_decode
 * to get characters.
 */

typedef struct
{
   SLCONST char *name;
   char escaped_form;
}
Posix_Char_Class_Type;

static Posix_Char_Class_Type Posix_Char_Class_Table [] =
{
     {"alnum", 'w'},
     {"alpha", 'a'},
     {"blank", 'b'},
     {"cntrl", 'c'},
     {"digit", 'd'},
     {"graph", 'g'},
     {"lower", 'l'},
     {"print", 'p'},
     {"punct", ','},
     {"space", 's'},
     {"upper", 'u'},
     {"xdigit", 'x'},
     {NULL, 0}
};

static int is_posix_charclass (SLuchar_Type **up, SLuchar_Type *umax, SLwchar_Type *char_classp)
{
   SLuchar_Type *u, *u1;
   unsigned int len;
   Posix_Char_Class_Type *p;

   u = *up;
   if (*u != ':')
     return 0;
   u++;

   u1 = u;
   while ((u1 < umax)
	  && (*u1 >= 'a')
	  && (*u1 <= 'z'))
     u1++;

   if (((u1+1) >= umax) || (u1[0] != ':') || (u1[1] != ']'))
     return 0;

   len = u1 - u;
   p = Posix_Char_Class_Table;
   while (p->name != NULL)
     {
	if ((0 == strncmp (p->name, (char *) u, len))
	    && (p->name[len] == 0))
	  {
	     *char_classp = p->escaped_form;
	     *up = u1 + 2;
	     return 1;
	  }
	p++;
     }
   _pSLang_verror (SL_NotImplemented_Error, "Character class in range specification is unknown or unsupported");
   return -1;
}

static int get_lex_char (SLuchar_Type **up, SLuchar_Type *umax,
			 int allow_charclass,
			 SLwchar_Type *chp, SLwchar_Type *char_classp)
{
   SLuchar_Type *u;
   SLwchar_Type ch;

   u = *up;
   if (u == umax)
     {
	*chp = 0;
	*char_classp = 0;
	return 0;
     }

   if (NULL == (u = _pSLinterp_decode_wchar (u, umax, &ch)))
     return -1;

   if ((ch == '[') && allow_charclass)
     {
	int status = is_posix_charclass (&u, umax, &ch);
	if (status != 0)
	  {
	     if (status == 1)
	       {
		  *chp = *char_classp = ch;
		  *up = u;
	       }
	     return status;
	  }
     }

   if ((ch != '\\') || (allow_charclass == 0)
       || (u == umax)) /* Permit a single backslash as the last character */
     {
	*char_classp = 0;
	*chp = ch;
	*up = u;
	return 0;
     }

   /* Here, ch=='\\' and *u represents the next character. */

   /* Allow \\ and \^ to represent \ and ^, resp.  Supporting \^ is useful
    * in constructs such as "\\^x" since "^x" may mean anything but x, and not
    * '^' or 'x'.
    */
   ch = *u;
   if ((ch == '\\') || (ch == '^'))
     {
	*char_classp = 0;
	*chp = ch;
	*up = u+1;
	return 0;
     }

   if (NULL == (u = _pSLinterp_decode_wchar (u, umax, &ch)))
     return -1;

   *chp = *char_classp = ch;
   *up = u;
   return 0;
}

typedef struct
{
#define LEXICAL_CHAR_TYPE	1
#define LEXICAL_RANGE_TYPE	2
#define LEXICAL_CLASS_TYPE	3
   int lexical_type;
   union
     {
	SLwchar_Type range[2];
	SLwchar_Type wch;
	int char_class;
     }
   e;
}
Lexical_Element_Type;

static SLuchar_Type *get_lexical_element (SLuchar_Type *u, SLuchar_Type *umax,
					  int allow_range,
					  int allow_charclass,
					  Lexical_Element_Type *lex)
{
   SLwchar_Type r0, r1;
   SLwchar_Type char_class;

   if (u == umax)
     return NULL;

   if (-1 == get_lex_char (&u, umax, allow_charclass, &r0, &char_class))
     return NULL;

   if (char_class)
     {
	lex->lexical_type = LEXICAL_CLASS_TYPE;
	switch (char_class)
	  {
	   case '7':
	     lex->e.char_class = SLCHARCLASS_ASCII;
	     break;

	   case 'a':	       /* alpha */
	     lex->e.char_class = SLCHARCLASS_ALPHA;
	     break;

	   case 'b':
	     lex->e.char_class = SLCHARCLASS_BLANK;
	     break;

	   case 'c':
	     lex->e.char_class = SLCHARCLASS_CNTRL;
	     break;

	   case 'd':	       /* digit */
	     lex->lexical_type = LEXICAL_RANGE_TYPE;
	     lex->e.range[0] = '0';
	     lex->e.range[1] = '9';
	     break;

	   case 'g':
	     lex->e.char_class = SLCHARCLASS_GRAPH;
	     break;

	   case 'l':	       /* lowercase */
	     lex->e.char_class = SLCHARCLASS_LOWER;
	     break;

	   case 'p':	       /* printable */
	     lex->e.char_class = SLCHARCLASS_PRINT;
	     break;

	   case ',':	       /* punctuation */
	     lex->e.char_class = SLCHARCLASS_PUNCT;
	     break;

	   case 's':	       /* whitespace */
	     lex->e.char_class = SLCHARCLASS_SPACE;
	     break;

	   case 'u':	       /* uppercase */
	     lex->e.char_class = SLCHARCLASS_UPPER;
	     break;

	   case 'x':
	     lex->e.char_class = SLCHARCLASS_XDIGIT;
	     break;

	   case 'w':	       /* alphanumeric */
	     lex->e.char_class = SLCHARCLASS_ALPHA|SLCHARCLASS_XDIGIT;
	     break;

	   default:
	     _pSLang_verror (SL_INVALID_PARM, "Invalid character class '%c'.", char_class);
	     return NULL;
	  }
	return u;
     }

   if ((*u != '-') || (allow_range == 0))
     {
	lex->lexical_type = LEXICAL_CHAR_TYPE;
	lex->e.wch = r0;
	return u;
     }

   u++;
   if (u == umax)
     {
	lex->lexical_type = LEXICAL_CHAR_TYPE;
	lex->e.wch = '-';
	return u;
	/* _pSLang_verror (SL_INVALID_PARM, "Unfinished range specification"); */
	/* return NULL; */
     }

   if (-1 == get_lex_char (&u, umax, allow_charclass, &r1, &char_class))
     return NULL;

   if (char_class)
     {
	_pSLang_verror (SL_INVALID_PARM, "Character class not allowed in a range");
	return NULL;
     }

   if (r1 == 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "Unfinished range specification");
	return NULL;
     }

   lex->lexical_type = LEXICAL_RANGE_TYPE;
   lex->e.range[0] = r0;
   lex->e.range[1] = r1;
   return u;
}

SLwchar_Lut_Type *SLwchar_strtolut (SLuchar_Type *u,
				    int allow_range, int allow_charclass)
{
   SLuchar_Type *umax;
   SLwchar_Lut_Type *r;
   Lexical_Element_Type lex;

   r = SLwchar_create_lut (32);
   if (r == NULL)
     return NULL;

   umax = u + strlen ((char *) u);

   while (u < umax)
     {
	if (NULL == (u = get_lexical_element (u, umax, allow_range, allow_charclass, &lex)))
	  goto return_error;

	switch (lex.lexical_type)
	  {
	   case LEXICAL_CHAR_TYPE:
	     if (-1 == SLwchar_add_range_to_lut (r, lex.e.wch, lex.e.wch))
	       goto return_error;
	     break;

	   case LEXICAL_RANGE_TYPE:
	     if (-1 == SLwchar_add_range_to_lut (r, lex.e.range[0], lex.e.range[1]))
	       goto return_error;
	     break;

	   case LEXICAL_CLASS_TYPE:
	     add_char_class (r, lex.e.char_class);
	     break;
	  }
     }
   return r;

   return_error:
   SLwchar_free_lut (r);
   return NULL;
}

/* This structure is used for mapping 1 character to another, and is used
 * by, e.g., strtrans.
 *
 * The most efficient implementation that I have come up with requires a
 * many-1 mapping between _constructs_ in the "from" list and the "to" list.
 * Here a _construct_ is a single character, range, or a character class.
 * The following mappings are legal:
 *
 *    Character --> Character
 *    Range     --> Character
 *    Range     --> Equal length range
 *    Range	--> Class (upper or lower)
 *    Class     --> Character
 *    Class     --> Compatible Class
 *
 * For inversion, the only mapping that makes sense is a many to one mapping.
 * For example, strtrans(str, "^A-Za-z", "x"), should replace any character
 * that is not one of the ranges A-Z and a-z by x.
 */
typedef struct Char_Map_Type
{
   int (*map_function)(Lexical_Element_Type *, Lexical_Element_Type *, int,
		       SLwchar_Type, SLwchar_Type *);

   Lexical_Element_Type from;
   Lexical_Element_Type to;

   struct Char_Map_Type *next;
}
Char_Map_Type;

struct SLwchar_Map_Type
{
   /* for chars < 256. */
   SLwchar_Type chmap[256];

   int invert;
   Char_Map_Type *list;
};

static int map_char_to_char_method (Lexical_Element_Type *from,
				    Lexical_Element_Type *to, int invert,
				    SLwchar_Type in, SLwchar_Type *out)
{
   int ok = (in == from->e.wch);
   if (0 == (ok ^ invert))
     return 0;

   *out = to->e.wch;
   return 1;
}

static int map_range_to_char_method (Lexical_Element_Type *from,
				     Lexical_Element_Type *to, int invert,
				     SLwchar_Type in, SLwchar_Type *out)
{
   int ok = ((in >= from->e.range[0]) && (in <= from->e.range[1]));
   if (0 == (ok ^ invert))
     return 0;

   *out = to->e.wch;
   return 1;
}

static int map_range_to_range_method (Lexical_Element_Type *from,
				      Lexical_Element_Type *to, int invert,
				      SLwchar_Type in, SLwchar_Type *out)
{
   int ok = ((in >= from->e.range[0]) && (in <= from->e.range[1]));
   if (0 == (ok ^ invert))
     return 0;

   *out = to->e.range[0] + (in - from->e.range[0]);
   return 1;
}

static int map_range_to_class_method (Lexical_Element_Type *from,
				      Lexical_Element_Type *to, int invert,
				      SLwchar_Type in, SLwchar_Type *out)
{
   int ok = ((in >= from->e.range[0]) && (in <= from->e.range[1]));
   if (0 == (ok ^ invert))
     return 0;

   if (to->e.char_class == SLCHARCLASS_UPPER)
     *out = SLwchar_toupper (in);
   else if (to->e.char_class == SLCHARCLASS_LOWER)
     *out = SLwchar_tolower (in);
   else
     return 0;

   return 1;
}

static int is_of_class (int char_class, SLwchar_Type w)
{
   switch (char_class)
     {
      case SLCHARCLASS_ALPHA:
	return SLwchar_isalpha (w);

      case SLCHARCLASS_ALPHA|SLCHARCLASS_XDIGIT:
	return SLwchar_isalnum (w);

      case SLCHARCLASS_UPPER:
	return SLwchar_isupper (w);

      case SLCHARCLASS_LOWER:
	return SLwchar_islower (w);

      case SLCHARCLASS_SPACE:
	return SLwchar_isspace (w);

      case SLCHARCLASS_ASCII:
	return w < (SLwchar_Type)0x80;
     }

   return 0;
}

static int map_class_to_char_method (Lexical_Element_Type *from,
				     Lexical_Element_Type *to, int invert,
				     SLwchar_Type in, SLwchar_Type *out)
{
   int ok = is_of_class (from->e.char_class, in);
   if (0 == (ok ^ invert))
     return 0;

   *out = to->e.wch;
   return 1;
}

static int map_class_to_class_method (Lexical_Element_Type *from,
				      Lexical_Element_Type *to, int invert,
				      SLwchar_Type in, SLwchar_Type *out)
{
   int ok = is_of_class (from->e.char_class, in);
   if (0 == (ok ^ invert))
     return 0;

   if (to->e.char_class == SLCHARCLASS_UPPER)
     *out = SLwchar_toupper (in);
   else if (to->e.char_class == SLCHARCLASS_LOWER)
     *out = SLwchar_tolower (in);
   else
     return 0;

   return 1;
}

static void init_chmap (SLwchar_Type *chmap, SLwchar_Type wch,
			SLwchar_Type (*to_func)(SLwchar_Type))
{
   unsigned int i;

   chmap[0] = 0;
   if (to_func == NULL)
     {
	for (i = 1; i < 256; i++)
	  chmap[i] = wch;
     }
   else
     {
	for (i = 1; i < 256; i++)
	  chmap[i] = (*to_func) (i);
     }
}

static void get_range_values (Lexical_Element_Type *lex,
			      SLwchar_Type *chminp, SLwchar_Type *chmaxp,
			      int *range_dirp)
{
   SLwchar_Type chmin = lex->e.range[0];
   SLwchar_Type chmax = lex->e.range[1];

   *range_dirp = 1;
   if (chmin > chmax)
     {
	SLwchar_Type tmp = chmin;
	chmin = chmax;
	chmax = tmp;

	lex->e.range[0] = chmax;
	lex->e.range[1] = chmin;
	*range_dirp = -1;
     }
   *chminp = chmin;
   *chmaxp = chmax;
}

static int is_ascii (SLwchar_Type wch)
{
   return wch < (SLwchar_Type) 0x80;
}

static int check_char_mapping (SLwchar_Map_Type *map, Char_Map_Type *list, int first_time)
{
   Lexical_Element_Type *lex_from, *lex_to;
   SLwchar_Type chmin, chmax, wch, wch1;
   SLwchar_Type (*to_func) (SLwchar_Type);
   int (*is_func) (SLwchar_Type);
   SLwchar_Type *chmap;
   int invert, from_range_dir, to_range_dir;

   lex_to = &list->to;
   lex_from = &list->from;
   chmap = map->chmap;
   invert = map->invert;

   switch (lex_from->lexical_type)
     {
      default:
	return -1;

      case LEXICAL_CHAR_TYPE:
	if (lex_to->lexical_type != LEXICAL_CHAR_TYPE)
	  return -1;

	wch = lex_to->e.wch;
	if (invert && first_time)
	  init_chmap (chmap, wch, NULL);

	list->map_function = map_char_to_char_method;

	if (0 == IS_ASCII256(lex_from->e.wch))
	  break;

	if (invert)
	  map->chmap[lex_from->e.wch] = lex_from->e.wch;
	else
	  {
	     map->chmap[lex_from->e.wch] = wch;
	     list->map_function = NULL;
	  }
	break;

      case LEXICAL_RANGE_TYPE:
	get_range_values (lex_from, &chmin, &chmax, &from_range_dir);

	switch (lex_to->lexical_type)
	  {
	   case LEXICAL_CHAR_TYPE:
	     wch = lex_to->e.wch;
	     if (invert && first_time)
	       init_chmap (chmap, wch, NULL);

	     while ((chmin < 256) && (chmin <= chmax))
	       {
		  chmap[chmin] = (invert ? chmin : wch);
		  chmin++;
	       }
	     list->map_function = map_range_to_char_method;
	     break;

	   case LEXICAL_CLASS_TYPE:
	     if (lex_to->e.char_class == SLCHARCLASS_UPPER)
	       to_func = SLwchar_toupper;
	     else if (lex_to->e.char_class == SLCHARCLASS_LOWER)
	       to_func = SLwchar_tolower;
	     else return -1;

	     if (invert && first_time)
	       init_chmap (chmap, 0, to_func);

	     while ((chmin < 256) && (chmin <= chmax))
	       {
		  chmap[chmin] = (invert ? chmin : (*to_func) (chmin));
		  chmin++;
	       }
	     list->map_function = map_range_to_class_method;
	     break;

	   case LEXICAL_RANGE_TYPE:
	     if (invert)
	       {
		  _pSLang_verror (SL_INVALID_PARM, "Inversion from a range to a range not permitted");
		  return -1;
	       }

	     get_range_values (lex_to, &wch, &wch1, &to_range_dir);

	     if ((chmax - chmin) != (wch1 - wch))
	       {
		  _pSLang_verror (SL_INVALID_PARM, "Character mapping of unequal ranges is forbidden");
		  return -1;
	       }
	     if (from_range_dir != to_range_dir)
	       {
		  wch = wch1;
		  to_range_dir = -1;
	       }
	     else to_range_dir = 1;

	     while ((chmin < 256) && (chmin <= chmax))
	       {
		  chmap[chmin] = wch;
		  chmin++;
		  wch += to_range_dir;
	       }
	     list->map_function = map_range_to_range_method;
	     break;

	   default:
	     return -1;
	  }
	if ((chmax < 256) && (invert == 0))
	  list->map_function = NULL;
	break;

      case LEXICAL_CLASS_TYPE:
	switch (lex_from->e.char_class)
	  {
	   case SLCHARCLASS_ALPHA:
	     is_func = SLwchar_isalpha;
	     break;

	   case SLCHARCLASS_ALPHA|SLCHARCLASS_XDIGIT:
	     is_func = SLwchar_isalnum;
	     break;

	   case SLCHARCLASS_UPPER:
	     is_func = SLwchar_isupper;
	     break;

	   case SLCHARCLASS_LOWER:
	     is_func = SLwchar_islower;
	     break;

	   case SLCHARCLASS_SPACE:
	     is_func = SLwchar_isspace;
	     break;

	   case SLCHARCLASS_ASCII:
	     is_func = is_ascii;
	     break;

	   case SLCHARCLASS_BLANK:
	     is_func = SLwchar_isblank;
	     break;

	   case SLCHARCLASS_CNTRL:
	     is_func = SLwchar_iscntrl;
	     break;

	   case SLCHARCLASS_GRAPH:
	     is_func = SLwchar_isgraph;
	     break;

	   case SLCHARCLASS_PRINT:
	     is_func = SLwchar_isprint;
	     break;

	   case SLCHARCLASS_PUNCT:
	     is_func = SLwchar_ispunct;
	     break;

	   case SLCHARCLASS_XDIGIT:
	     is_func = SLwchar_isxdigit;
	     break;

	   default:
	     _pSLang_verror (SL_INVALID_PARM, "Invalid character class in character map");
	     return -1;
	  }
	switch (lex_to->lexical_type)
	  {
	   case LEXICAL_CHAR_TYPE:
	     wch = lex_to->e.wch;

	     if (first_time && invert)
	       init_chmap (chmap, wch, NULL);

	     for (chmin = 0; chmin < 256; chmin++)
	       {
		  if ((*is_func)(chmin))
		    chmap[chmin] = (invert ? chmin : wch);
	       }
	     list->map_function = map_class_to_char_method;
	     break;

	   case LEXICAL_CLASS_TYPE:
	     switch (lex_to->e.char_class)
	       {
		case SLCHARCLASS_LOWER:
		  to_func = SLwchar_tolower;
		  break;
		case SLCHARCLASS_UPPER:
		  to_func = SLwchar_toupper;
		  break;

		default:
		  return -1;
	       }

	     if (invert && first_time)
	       init_chmap (chmap, 0, to_func);

	     for (chmin = 0; chmin < 256; chmin++)
	       {
		  if ((*is_func)(chmin))
		    chmap[chmin] = (invert ? chmin : (*to_func)(chmin));
	       }
	     break;

	   default:
	     return -1;
	  }
	list->map_function = map_class_to_class_method;
	break;
     }
   return 0;
}

static void free_char_map_type (Char_Map_Type *m)
{
   SLfree ((char *) m);
}

void SLwchar_free_char_map (SLwchar_Map_Type *map)
{
   Char_Map_Type *list;

   if (map == NULL)
     return;

   list = map->list;
   while (list != NULL)
     {
	Char_Map_Type *next = list->next;
	free_char_map_type (list);
	list = next;
     }
   SLfree ((char *) map);
}

SLwchar_Map_Type *SLwchar_allocate_char_map (SLuchar_Type *from, SLuchar_Type *to)
{
   SLwchar_Map_Type *map;
   Char_Map_Type *list, *prev;
   SLuchar_Type *from_max, *to_max;
   unsigned int i;
   int invert = 0, first_time;

   if (*from == '^')
     {
	invert = 1;
	from++;
     }

#if 0
   if (*from == 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "Illegal empty string in character map specification");
	return NULL;
     }
#endif
   map = (SLwchar_Map_Type *)SLcalloc (1, sizeof (SLwchar_Map_Type));
   if (map == NULL)
     return NULL;

   map->invert = invert;

   for (i = 0; i < 256; i++)
     map->chmap[i] = i;

   from_max = from + strlen ((char *) from);
   to_max = to + strlen ((char *) to);

   list = NULL;

   while (from < from_max)
     {
	Char_Map_Type *next;
	SLuchar_Type *next_to;

	if (NULL == (next = (Char_Map_Type *) SLcalloc (1, sizeof (Char_Map_Type))))
	  goto return_error;

	if (list == NULL)
	  map->list = next;
	else
	  list->next = next;
	list = next;

	if (NULL == (from = get_lexical_element (from, from_max, 1, 1, &list->from)))
	  goto return_error;

	if (NULL == (next_to = get_lexical_element (to, to_max, 1, 1, &list->to)))
	  goto return_error;

	/* If the mapping is not 1-1, then the last "to" object applies to the
	 * remaining "from" objects.  This will permit, e.g.,
	 *  A-Za-z --> X
	 */
	if (next_to != to_max)
	  {
	     if (invert)
	       {
		  _pSLang_verror (SL_INVALID_PARM, "Character map inversion must specify a many-to-one mapping");
		  goto return_error;
	       }
	     to = next_to;
	  }
     }

   list = map->list;
   prev = NULL;
   first_time = 1;
   while (list != NULL)
     {
	Char_Map_Type *next = list->next;

	if (-1 == check_char_mapping (map, list, first_time))
	  {
	     _pSLang_verror (SL_INVALID_PARM, "Specified character mapping is invalid");
	     goto return_error;
	  }
	first_time = 0;

	if (list->map_function == NULL)
	  {
	     if (prev == NULL)
	       map->list = next;
	     else
	       prev->next = next;

	     free_char_map_type (list);
	  }
	else prev = list;
	list = next;
     }
   return map;

   return_error:
   SLwchar_free_char_map (map);
   return NULL;
}

static int apply_lexical_map (SLwchar_Map_Type *map, SLwchar_Type wc_in, SLwchar_Type *wc_out)
{
   Char_Map_Type *list = map->list;
   int invert = map->invert;

   while (list != NULL)
     {
	if (list->map_function != NULL)
	  {
	     int status = (*list->map_function)(&list->from, &list->to, invert, wc_in, wc_out);
	     if (invert ^ status)
	       return status;
	  }
	list = list->next;
     }
   return 0;
}

int SLwchar_apply_char_map (SLwchar_Map_Type *map, SLwchar_Type *input, SLwchar_Type *output, unsigned int num)
{
   unsigned int i;
   SLwchar_Type *chmap;

   if ((map == NULL) || (input == NULL) || (output == NULL))
     return -1;

   chmap = map->chmap;

   for (i = 0; i < num; i++)
     {
	SLwchar_Type wc_in;

	if ((wc_in = input[i]) < 0x100)
	  {
	     output[i] = chmap[wc_in];
	     continue;
	  }

	if (0 == apply_lexical_map (map, wc_in, output + i))
	  output[i] = wc_in;
     }

   return 0;
}

/* This function returns a malloced string */
SLuchar_Type *SLuchar_apply_char_map (SLwchar_Map_Type *map, SLuchar_Type *str)
{
   SLuchar_Type *str_max;
   SLuchar_Type *output, *output_max, *outptr;
   int use_chmap;
   unsigned int len;
   SLwchar_Type *chmap;

   if ((map == NULL) || (str == NULL))
     return NULL;

   use_chmap = 1;
   if (_pSLinterp_UTF8_Mode == 0)
     str_max = str + strlen ((char *)str);
   else
     {
	str_max = str;
	while (*str_max)
	  {
	     if (*str_max & 0x80)
	       use_chmap = 0;
	     str_max++;
	  }
     }

   len = str_max - str;
   chmap = map->chmap;

   if (use_chmap)
     {
	unsigned int i;

	output = (SLuchar_Type *)SLmalloc (len+1);
	if (output == NULL)
	  return NULL;

	for (i = 0; i < len; i++)
	  output[i] = chmap[str[i]];

	output[len] = 0;
	return output;
     }

   /* Hard way */
   len += SLUTF8_MAX_MBLEN;
   if (NULL == (output = (SLuchar_Type *)SLmalloc (len + 1)))
     return NULL;
   output_max = output + len;
   outptr = output;

   while (str < str_max)
     {
	SLwchar_Type w_out, w_in;
	unsigned int encoded_len;

	w_in = (SLwchar_Type) *str;
	if (w_in < 0x100)
	  {
	     str++;
	     w_out = chmap[w_in];
	     if ((w_out < 0x80) && (outptr < output_max))
	       {
		  *outptr++ = (SLuchar_Type) w_out;
		  continue;
	       }
	  }
	else
	  {
	     if (NULL == (str = _pSLinterp_decode_wchar (str, str_max, &w_in)))
	       goto return_error;

	     if (-1 == SLwchar_apply_char_map (map, &w_in, &w_out, 1))
	       goto return_error;
	  }

	if (outptr + SLUTF8_MAX_MBLEN >= output_max)
	  {
	     SLuchar_Type *tmp;

	     len += 32 * SLUTF8_MAX_MBLEN;
	     if (NULL == (tmp = (SLuchar_Type *)SLrealloc ((char *)output, len)))
	       goto return_error;

	     outptr = tmp + (outptr - output);
	     output = tmp;
	     output_max = output + len;
	  }

	if (NULL == (outptr = _pSLinterp_encode_wchar (w_out, outptr, &encoded_len)))
	  goto return_error;
     }

   *outptr = 0;

   return output;

   return_error:
   SLfree ((char *) output);
   return NULL;
}

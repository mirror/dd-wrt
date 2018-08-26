/* ed style regular expressions */
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

struct _pSLRegexp_Type
{
   /* These must be set by calling routine. */
   unsigned char *pat;		       /* regular expression pattern */
   unsigned char *buf;		       /* buffer for compiled regexp */
   unsigned int buf_len;	       /* length of buffer */
   int case_sensitive;		       /* 1 if match is case sensitive  */

   /* The rest are set by SLang_regexp_compile */

   int must_match;		       /* 1 if line must contain substring */
   int must_match_bol;		       /* true if it must match beginning of line */
   unsigned char must_match_str[16];   /* 15 char null term substring */
   int osearch;			       /* 1 if ordinary search suffices */
   unsigned int min_length;	       /* minimum length the match must be */
   int beg_matches[10];		       /* offset of start of \( */
   unsigned int end_matches[10];       /* length of nth submatch
					* Note that the entire match corresponds
					* to \0
					*/
   int offset;			       /* offset to be added to beg_matches */
};

#define SET_BIT(b, n) b[(unsigned int) (n) >> 3] |= 1 << ((unsigned int) (n) % 8)
#define TEST_BIT(b, n) (b[(unsigned int)(n) >> 3] & (1 << ((unsigned int) (n) % 8)))
#define LITERAL 1
#define RANGE 2			       /* [...] */
#define ANY 3			       /* . */
#define BOL 4			       /* ^ */
#define EOL 5			       /* $ */
#define NTH_MATCH 6		       /* \1 \2 ... \9 */
#define OPAREN 7		       /* \( */
#define CPAREN 0x8		       /* \) */
#define ANY_DIGIT 0x9		       /* \d */
#define BOW	0xA		       /* \< */
#define EOW	0xB		       /* \> */
#if 0
#define NOT_LITERAL		0xC	       /* \~ */
#endif
#define STAR 0x80		       /* * */
#define LEAST_ONCE 0x40		       /* + */
#define MAYBE_ONCE 0x20		       /* ? */
#define MANY 0x10		       /* {n,m} */
/* The rest are additions */
#define YES_CASE (STAR | BOL)
#define NO_CASE  (STAR | EOL)

#define UPPERCASE(x)  (cs ? (x) : UPPER_CASE(x))
#define LOWERCASE(x)  (cs ? (x) : LOWER_CASE(x))

/* FIXME: UTF8 */
static unsigned char Word_Chars[256];
#define IS_WORD_CHAR(x) Word_Chars[(unsigned int) (x)]

#if 0
static int ctx->open_paren_number;
static char Closed_Paren_Matches[10];

static SLRegexp_Type *This_Reg;
static unsigned char *This_Str;
#endif

typedef struct
{
   SLRegexp_Type *reg;
   SLCONST unsigned char *str;
   unsigned int len;
   char closed_paren_matches[10];
   int open_paren_number;
}
Re_Context_Type;

static SLCONST unsigned char *do_nth_match (Re_Context_Type *ctx, int n, SLCONST unsigned char *str, SLCONST unsigned char *estr)
{
   SLCONST unsigned char *bpos;

   if (ctx->closed_paren_matches[n] == 0)
     return NULL;

   bpos = ctx->reg->beg_matches[n] + ctx->str;
   n = ctx->reg->end_matches[n];
   if (n == 0) return(str);
   if (n > (int) (estr - str)) return (NULL);

   /* This needs fixed for case sensitive match */
   if (0 != strncmp((char *) str, (char *) bpos, (unsigned int) n)) return (NULL);
   str += n;
   return (str);
}

/* returns pointer to the end of regexp or NULL */
static SLCONST unsigned char *regexp_looking_at (Re_Context_Type *ctx,
						 SLCONST unsigned char *str, SLCONST unsigned char *estr,
						 unsigned char *regexp,
						 int cs)
{
   register unsigned char p, p1;
   SLCONST unsigned char *save_str, *tmpstr;
   int n, n0, n1;
   int save_num_open;
   char save_closed_matches[10];

   p = *regexp++;

   while (p != 0)
     {
	/* p1 = UPPERCASE(*regexp); */
	/* if (str < estr) c = UPPERCASE(*str); */

	switch((unsigned char) p)
	  {
	   case BOW:
	     if ((str != ctx->str)
		 && ((str >= estr)
		     || IS_WORD_CHAR(*(str - 1))
		     || (0 == IS_WORD_CHAR(*str)))) return NULL;
	     break;

	   case EOW:
	     if ((str < estr)
		 && IS_WORD_CHAR (*str)) return NULL;
	     break;

	   case YES_CASE: cs = 1; break;
	   case NO_CASE: cs = 0; break;

	   case OPAREN:
	     ctx->open_paren_number++;
	     ctx->reg->beg_matches[ctx->open_paren_number] = (int) (str - ctx->str);
	     break;
	   case CPAREN:
	     n = ctx->open_paren_number;
	     while (n > 0)
	       {
		  if (ctx->closed_paren_matches[n] != 0)
		    {
		       n--;
		       continue;
		    }
		  ctx->closed_paren_matches[n] = 1;
		  ctx->reg->end_matches[n] = (unsigned int) (str - (ctx->str + ctx->reg->beg_matches[n]));
		  break;
	       }
	     break;
#ifdef NOT_LITERAL
	   case NOT_LITERAL:
	     if ((str >= estr) || (*regexp == UPPERCASE(*str))) return (NULL);
	     str++; regexp++;
	     break;

	   case MAYBE_ONCE | NOT_LITERAL:
	     save_str = str;
	     if ((str < estr) && (*regexp != UPPERCASE(*str))) str++;
	     regexp++;
	     goto match_rest;

	   case NOT_LITERAL | LEAST_ONCE:   /* match at least once */
	     if ((str >= estr) || (UPPERCASE(*str) == UPPERCASE(*regexp))) return (NULL);
	     str++;
	     /* drop */
	   case STAR | NOT_LITERAL:
	     save_str = str;  p1 = *regexp;
	     while ((str < estr) && (UPPERCASE(*str) != p1)) str++;
	     regexp++;
	     goto match_rest;

	     /* this type consists of the expression + two bytes that
	        determine number of matches to perform */
	   case MANY | NOT_LITERAL:
	     p1 = *regexp; regexp++;
	     n = n0 = (int) (unsigned char) *regexp++;
	     /* minimum number to match--- could be 0 */
	     n1 = (int) (unsigned char) *regexp++;
	     /* maximum number to match */

	     while (n && (str < estr) && (p1 != UPPERCASE(*str)))
	       {
		  n--;
		  str++;
	       }
	     if (n) return (NULL);

	     save_str = str;
	     n = n1 - n0;
	     while (n && (str < estr) && (p1 != UPPERCASE(*str)))
	       {
		  n--;
		  str++;
	       }
	     goto match_rest;
#endif				       /* NOT_LITERAL */
	   case LITERAL:
	     if ((str >= estr) || (*regexp != UPPERCASE(*str))) return (NULL);
	     str++; regexp++;
	     break;

	   case MAYBE_ONCE | LITERAL:
	     save_str = str;
	     if ((str < estr) && (*regexp == UPPERCASE(*str))) str++;
	     regexp++;
	     goto match_rest;

	   case LITERAL | LEAST_ONCE:   /* match at least once */
	     if ((str >= estr) || (UPPERCASE(*str) != UPPERCASE(*regexp))) return (NULL);
	     str++;
	     /* drop */
	   case STAR | LITERAL:
	     save_str = str;  p1 = *regexp;
	     while ((str < estr) && (UPPERCASE(*str) == p1)) str++;
	     regexp++;
	     goto match_rest;

	     /* this type consists of the expression + two bytes that
	        determine number of matches to perform */
	   case MANY | LITERAL:
	     p1 = *regexp; regexp++;
	     n = n0 = (int) (unsigned char) *regexp++;
	     /* minimum number to match--- could be 0 */
	     n1 = (int) (unsigned char) *regexp++;
	     /* maximum number to match */

	     while (n && (str < estr) && (p1 == UPPERCASE(*str)))
	       {
		  n--;
		  str++;
	       }
	     if (n) return (NULL);

	     save_str = str;
	     n = n1 - n0;
	     while (n && (str < estr) && (p1 == UPPERCASE(*str)))
	       {
		  n--;
		  str++;
	       }
	     goto match_rest;

	   case NTH_MATCH:
	     if ((str = do_nth_match(ctx, (int) (unsigned char) *regexp, str, estr)) == NULL) return(NULL);
	     regexp++;
	     break;

	   case MAYBE_ONCE | NTH_MATCH:
	     save_str = str;
	     tmpstr = do_nth_match (ctx, (int) (unsigned char) *regexp, str, estr);
	     regexp++;
	     if (tmpstr != NULL)
	       {
		  str = tmpstr;
		  goto match_rest;
	       }
	     continue;

	   case LEAST_ONCE | NTH_MATCH:
	     if ((str = do_nth_match(ctx, (int) (unsigned char) *regexp, str, estr)) == NULL) return(NULL);
	     /* drop */
	   case STAR | NTH_MATCH:
	     save_str = str;
	     while (NULL != (tmpstr = do_nth_match(ctx, (int) (unsigned char) *regexp, str, estr)))
	       {
		  str = tmpstr;
	       }
	     regexp++;
	     goto match_rest;

	   case MANY | NTH_MATCH: return(NULL);
	     /* needs done */

	   case RANGE:
	     if (str >= estr) return (NULL);
	     if (TEST_BIT(regexp, UPPERCASE(*str)) == 0) return (NULL);
	     regexp += 32; str++;
	     break;

	   case MAYBE_ONCE | RANGE:
	     save_str = str;
	     if ((str < estr) && TEST_BIT(regexp, UPPERCASE(*str))) str++;
	     regexp += 32;
	     goto match_rest;

	   case LEAST_ONCE | RANGE:
	     if ((str >= estr) || (0 == TEST_BIT(regexp, UPPERCASE(*str)))) return NULL;
	     str++;
	     /* drop */
	   case STAR | RANGE:
	     save_str = str;
	     while ((str < estr) && TEST_BIT(regexp, UPPERCASE(*str))) str++;
	     regexp += 32;
	     goto match_rest;

	     /* The first 32 bytes correspond to the range and the two
	      * following bytes indicate the min and max number of matches.
	      */
	   case MANY | RANGE:
	     /* minimum number to match--- could be 0 */
	     n = n0 = (int) (unsigned char) *(regexp + 32);
	     /* maximum number to match */
	     n1 = (int) (unsigned char) *(regexp + 33);

	     while (n && (str < estr) && (TEST_BIT(regexp, UPPERCASE(*str))))
	       {
		  n--;
		  str++;
	       }
	     if (n) return (NULL);
	     save_str = str;
	     n = n1 - n0;
	     while (n && (str < estr) && (TEST_BIT(regexp, UPPERCASE(*str))))
	       {
		  n--;
		  str++;
	       }
	     regexp += 34;		       /* 32 + 2 */
	     goto match_rest;

	   case ANY_DIGIT:
	     if ((str >= estr) || (*str > '9') || (*str < '0')) return (NULL);
	     str++;
	     break;

	   case MAYBE_ONCE | ANY_DIGIT:
	     save_str = str;
	     if ((str < estr) && ((*str > '9') || (*str < '0'))) str++;
	     goto match_rest;

	   case LEAST_ONCE | ANY_DIGIT:
	     if ((str >= estr) || ((*str > '9') || (*str < '0'))) return NULL;
	     str++;
	     /* drop */
	   case STAR | ANY_DIGIT:
	     save_str = str;
	     while ((str < estr) && ((*str <= '9') && (*str >= '0'))) str++;
	     goto match_rest;

	   case MANY | ANY_DIGIT:
	     /* needs finished */
	     return (NULL);

	   case ANY:		       /* . */
	     /* FIXME: UTF8 */
	     if ((str >= estr) || (*str == '\n')) return (NULL);
	     str++;
	     break;

	   case MAYBE_ONCE | ANY:      /* .? */
	     /* FIXME: UTF8 */
	     save_str = str;
	     if ((str < estr) && (*str != '\n')) str++;
	     goto match_rest;

	   case LEAST_ONCE | ANY:      /* .+ */
	     /* FIXME: UTF8 */
	     if ((str >= estr) || (*str == '\n')) return (NULL);
	     str++;
	     /* drop */
	   case STAR | ANY:	       /* .* */
	     /* FIXME: UTF8 */
	     save_str = str;
	     while ((str < estr) && (*str != '\n')) str++;
	     goto match_rest;

	   case MANY | ANY:
	     /* FIXME: Not implemented */
	     return (NULL);
	     /* needs finished */

	   case EOL:
	     if ((str >= estr) || (*str == '\n')) return (str);
	     return(NULL);

	   default: return (NULL);
	  }
	p = *regexp++;
	continue;

	match_rest:
	if (save_str == str)
	  {
	     p = *regexp++;
	     continue;
	  }

	/* if (p == EOL)
	 * {
	 * if (str < estr) return (NULL); else return (str);
	 * }
	 */

	SLMEMCPY(save_closed_matches, ctx->closed_paren_matches, sizeof(save_closed_matches));
	save_num_open = ctx->open_paren_number;
	while (str >= save_str)
	  {
	     tmpstr = regexp_looking_at (ctx, str, estr, regexp, cs);
	     if (tmpstr != NULL) return(tmpstr);
	     SLMEMCPY(ctx->closed_paren_matches, save_closed_matches, sizeof(ctx->closed_paren_matches));
	     ctx->open_paren_number = save_num_open;
	     str--;
	  }
	return NULL;
     }
   if ((p != 0) && (p != EOL)) return (NULL); else return (str);
}

static void
fixup_beg_end_matches (Re_Context_Type *ctx, SLRegexp_Type *r,
		       SLCONST unsigned char *str, SLCONST unsigned char *epos)
{
   int i;

   if (str == NULL)
     {
	r->beg_matches[0] = -1;
	r->end_matches[0] = 0;
	SLMEMSET(ctx->closed_paren_matches, 0, sizeof(ctx->closed_paren_matches));
     }
   else
     {
	r->beg_matches[0] = (int) (str - ctx->str);
	r->end_matches[0] = (unsigned int) (epos - str);
     }

   for (i = 1; i < 10; i++)
     {
	if (ctx->closed_paren_matches [i] == 0)
	  {
	     r->beg_matches[i] = -1;
	     r->end_matches[i] = 0;
	  }
     }
}

static void init_re_context (Re_Context_Type *ctx, SLRegexp_Type *reg,
			     SLCONST unsigned char *str, unsigned int len)
{
   memset ((char *) ctx, 0, sizeof (Re_Context_Type));
   ctx->reg = reg;
   ctx->str = str;
   ctx->len = len;
}

static SLCONST unsigned char *regexp_match(SLCONST unsigned char *str,
					   unsigned int len, SLRegexp_Type *reg)
{
   unsigned char c = 0;
   SLCONST unsigned char *estr = str + len;
   int cs = reg->case_sensitive, lit = 0;
   unsigned char *buf = reg->buf;
   SLCONST unsigned char *epos = NULL;
   Re_Context_Type ctx_buf;

   if (reg->min_length > len) return NULL;

   init_re_context (&ctx_buf, reg, str, len);

   if (*buf == BOL)
     {
	if (NULL == (epos = regexp_looking_at (&ctx_buf, str, estr, buf + 1, cs)))
	  str = NULL;

	fixup_beg_end_matches (&ctx_buf, reg, str, epos);
	return str;
     }

   if (*buf == NO_CASE)
     {
	buf++;  cs = 0;
     }

   if (*buf == YES_CASE)
     {
	buf++;  cs = 1;
     }

   if (*buf == LITERAL)
     {
	lit = 1;
	c = *(buf + 1);
     }
   else if ((*buf == OPAREN) && (*(buf + 1) == LITERAL))
     {
	lit = 1;
	c = *(buf + 2);
     }

   while (1)
     {
	ctx_buf.open_paren_number = 0;
	memset (ctx_buf.closed_paren_matches, 0, sizeof(ctx_buf.closed_paren_matches));
	/* take care of leading chars */
	if (lit)
	  {
	     while ((str < estr) && (c != UPPERCASE(*str))) str++;
	     if (str >= estr)
	       break;		       /* failed */
	  }

	if (NULL != (epos = regexp_looking_at(&ctx_buf, str, estr, buf, cs)))
	  {
	     fixup_beg_end_matches (&ctx_buf, reg, str, epos);
	     return str;
	  }
	if (str >= estr)
	  break;
	str++;
     }
   fixup_beg_end_matches (&ctx_buf, reg, NULL, epos);
   return NULL;
}

char *SLregexp_match (SLRegexp_Type *reg, SLFUTURE_CONST char *str, unsigned int len)
{
   return (char *) regexp_match ((SLCONST unsigned char *)str, len, reg);
}

static unsigned char *convert_digit(unsigned char *pat, int *nn)
{
   int n = 0, m = 0;
   unsigned char c;
   while (c = (unsigned char) *pat, (c <= '9') && (c >= '0'))
     {
	pat++;
	n = 10 * n + (c - '0');
	m++;
     }
   if (m == 0)
     {
	return (NULL);
     }
   *nn = n;
   return pat;
}

#define ERROR  return (int) (pat - reg->pat)

/* Returns 0 if successful or offset in pattern of error */
static int regexp_compile (SLRegexp_Type *reg)
{
   register unsigned char *buf, *ebuf, *pat;
   unsigned char *last = NULL, *tmppat;
   register unsigned char c;
   int i, reverse = 0, n, cs;
   int oparen = 0, nparen = 0;
   /* substring stuff */
   int count, last_count, this_max_mm = 0, max_mm = 0, ordinary_search,
     no_osearch = 0, min_length = 0;
   unsigned char *mm_p = NULL, *this_mm_p = NULL;
   static int already_initialized;

   reg->beg_matches[0] = reg->end_matches[0] = 0;
   buf = reg->buf;
   ebuf = (reg->buf + reg->buf_len) - 2; /* make some room */
   pat = reg->pat;
   cs = reg->case_sensitive;

   if (already_initialized == 0)
     {
	SLang_init_case_tables ();
#ifdef IBMPC_SYSTEM
	SLmake_lut (Word_Chars, (unsigned char *) "_0-9a-zA-Z\200-\232\240-\245\341-\353", 0);
#else
	SLmake_lut (Word_Chars, (unsigned char *) "_0-9a-zA-Z\277-\326\330-\336\340-\366\370-\376", 0);
#endif
	already_initialized = 1;
     }

   i = 1; while (i < 10)
     {
	reg->beg_matches[i] = -1;
	reg->end_matches[i] = 0;
	i++;
     }

   if (*pat == '\\')
     {
	if (pat[1] == 'c')
	  {
	     cs = 1;
	     pat += 2;
	     no_osearch = 1;
	  }
	else if (pat[1] == 'C')
	  {
	     cs = 0;
	     pat += 2;
	     no_osearch = 1;
	  }
     }

   if (*pat == '^')
     {
	pat++;
	*buf++ = BOL;
	reg->must_match_bol = 1;
     }
   else reg->must_match_bol = 0;

   if (cs != reg->case_sensitive)
     {
	if (cs) *buf++ = YES_CASE;
	else *buf++ = NO_CASE;
     }

   *buf = 0;

   last_count = count = 0;
   while ((c = *pat++) != 0)
     {
	if (buf >= ebuf - 3)
	  {
	     _pSLang_verror (SL_BUILTIN_LIMIT_EXCEEDED, "Pattern too large to be compiled.");
	     ERROR;
	  }

	count++;
	switch (c)
	  {
	   case '$':
	     if (*pat != 0) goto literal_char;
	     *buf++ = EOL;
	     break;

	   case '\\':
	     c = *pat++;
	     no_osearch = 1;
	     switch(c)
	       {
		case 'e': c = 033; goto literal_char;
		case 'n': c = '\n'; goto literal_char;
		case 't': c = '\t'; goto literal_char;
		case 'C': cs = 0; *buf++ = NO_CASE; break;
		case 'c': cs = 1; *buf++ = YES_CASE; break;
		case '1': case '2': case '3':  case '4':  case '5':
		case '6': case '7': case '8':  case '9':
		  c = c - '0';
		  if ((int) c > nparen) ERROR;
		  last = buf;
		  *buf++ = NTH_MATCH; *buf++ = c;
		  break;
#ifdef NOT_LITERAL
		case '~':	       /* slang extension */
		  if ((c = *pat) == 0) ERROR;
		  pat++;
		  last = buf;
		  *buf++ = NOT_LITERAL;
		  *buf++ = c;
		  min_length++;
		  break;
#endif
		case 'd':	       /* slang extension */
		  last = buf;
		  *buf++ = ANY_DIGIT;
		  min_length++;
		  break;

		case '<':
		  last = NULL;
		  *buf++ = BOW;
		  break;

		case '>':
		  last = NULL;
		  *buf++ = EOW;
		  break;

		case '{':
		  if (last == NULL) goto literal_char;
		  *last |= MANY;
		  tmppat = convert_digit(pat, &n);
		  if (tmppat == NULL) ERROR;
		  pat = tmppat;
		  *buf++ = n;

		  min_length += (n - 1);

		  if (*pat == '\\')
		    {
		       *buf++ = n;
		    }
		  else if (*pat == ',')
		    {
		       pat++;
		       if (*pat == '\\')
			 {
			    n = 255;
			 }
		       else
			 {
			    tmppat = convert_digit(pat, &n);
			    if (tmppat == NULL) ERROR;
			    pat = tmppat;
			    if (*pat != '\\') ERROR;
			 }
		       *buf++ = n;
		    }
		  else ERROR;
		  last = NULL;
		  pat++;
		  if (*pat != '}') ERROR;
		  pat++;
		  break;   /* case '{' */

		case '(':
		  oparen++;
		  if (oparen > 9) ERROR;
		  *buf++ = OPAREN;
		  break;
		case ')':
		  if (oparen == 0) ERROR;
		  oparen--;
		  nparen++;
		  *buf++ = CPAREN;
		  break;

		case 0: ERROR;
		default:
		  goto literal_char;
	       }
	     break;

	   case '[':

	     *buf = RANGE;
	     last = buf++;

	     if (buf + 32 >= ebuf) ERROR;

	     for (i = 0; i < 32; i++) buf[i] = 0;
	     c = *pat++;
	     if (c == '^')
	       {
		  reverse = 1;
		  SET_BIT(buf, '\n');
		  c = *pat++;
	       }

	     if (c == ']')
	       {
		  SET_BIT(buf, c);
		  c = *pat++;
	       }
	     while (c && (c != ']'))
	       {
		  if (c == '\\')
		    {
		       c = *pat++;
		       switch(c)
			 {
			    case 'n': c = '\n'; break;
			    case 't': c = '\t'; break;
			    case 0: ERROR;
			 }
		    }

		  if (*pat == '-')
		    {
		       pat++;
		       while (c < *pat)
			 {
			    if (cs == 0)
			      {
				 SET_BIT(buf, UPPERCASE(c));
				 SET_BIT(buf, LOWERCASE(c));
			      }
			    else SET_BIT(buf, c);
			    c++;
			 }
		    }
		  if (cs == 0)
		    {
		       SET_BIT(buf, UPPERCASE(c));
		       SET_BIT(buf, LOWERCASE(c));
		    }
		  else SET_BIT(buf, c);
		  c = *pat++;
	       }
	     if (c != ']') ERROR;
	     if (reverse) for (i = 0; i < 32; i++) buf[i] = buf[i] ^ 0xFF;
	     reverse = 0;
	     buf += 32;
	     min_length++;
	     break;

	   case '.':
	     last = buf;
	     *buf++ = ANY;
	     min_length++;
	     break;

	   case '*':
	     if (last == NULL) goto literal_char;
	     *last |= STAR;
	     min_length--;
	     last = NULL;
	     break;

	   case '+':
	     if (last == NULL) goto literal_char;
	     *last |= LEAST_ONCE;
	     last = NULL;
	     break;

	   case '?':
	     if (last == NULL) goto literal_char;
	     *last |= MAYBE_ONCE;
	     last = NULL;
	     min_length--;
	     break;

	   literal_char:
	   default:
	     /* This is to keep track of longest substring */
	     min_length++;
	     this_max_mm++;
	     if (last_count + 1 == count)
	       {
		  if (this_max_mm == 1)
		    {
		       this_mm_p = buf;
		    }
		  else if (max_mm < this_max_mm)
		    {
		       mm_p = this_mm_p;
		       max_mm = this_max_mm;
		    }
	       }
	     else
	       {
		  this_mm_p = buf;
		  this_max_mm = 1;
	       }

	     last_count = count;

	     last = buf;
	     *buf++ = LITERAL;
	     *buf++ = UPPERCASE(c);
	  }
     }
   *buf = 0;
   /* Check for ordinary search */
   ebuf = buf;
   buf = reg->buf;

   if (no_osearch) ordinary_search = 0;
   else
     {
	ordinary_search = 1;
	while (buf < ebuf)
	  {
	     if (*buf != LITERAL)
	       {
		  ordinary_search = 0;
		  break;
	       }
	     buf += 2;
	  }
     }

   reg->osearch = ordinary_search;
   reg->must_match_str[15] = 0;
   reg->min_length = (min_length > 0) ? (unsigned int) min_length : 0;
   if (ordinary_search)
     {
	strncpy((char *) reg->must_match_str, (char *) reg->pat, 15);
	reg->must_match = 1;
	return(0);
     }
   /* check for longest substring of pattern */
   reg->must_match = 0;
   if ((mm_p == NULL) && (this_mm_p != NULL)) mm_p = this_mm_p;
   if (mm_p == NULL)
     {
	return (0);
     }
   n = 15;
   pat = reg->must_match_str;
   buf = mm_p;
   while (n--)
     {
	if (*buf++ != LITERAL) break;
	*pat++ = *buf++;
     }
   *pat = 0;
   if (pat != reg->must_match_str) reg->must_match = 1;
   return(0);
}

void SLregexp_free (SLRegexp_Type *reg)
{
   if (reg == NULL)
     return;
   if (reg->buf != NULL)
     SLfree ((char *) reg->buf);
   SLfree ((char *) reg);
}

SLRegexp_Type *SLregexp_compile (SLFUTURE_CONST char *pattern, unsigned int flags)
{
   SLRegexp_Type *reg;

   reg = (SLRegexp_Type *)SLcalloc (1, sizeof (SLRegexp_Type));
   if (reg == NULL)
     return NULL;

   if (NULL == (reg->buf = (unsigned char *)SLmalloc (1024)))
     {
	SLfree ((char *) reg);
	return NULL;
     }
   reg->buf_len = 1024;
   reg->case_sensitive = (0 == (flags & SLREGEXP_CASELESS));
   reg->pat = (unsigned char *)pattern;

   if (-1 == regexp_compile (reg))
     {
	SLregexp_free (reg);
	return NULL;
     }

   return reg;
}

char *SLregexp_quote_string (SLFUTURE_CONST char *re, char *buf, unsigned int buflen)
{
   char ch;
   char *b, *bmax;

   if (re == NULL) return NULL;

   b = buf;
   bmax = buf + buflen;

   while (b < bmax)
     {
	switch (ch = *re++)
	  {
	   case 0:
	     *b = 0;
	     return buf;

	   case '$':
	   case '\\':
	   case '[':
	   case ']':
	   case '.':
	   case '^':
	   case '*':
	   case '+':
	   case '?':
	     *b++ = '\\';
	    if (b == bmax) break;
	     /* drop */

	   default:
	     *b++ = ch;
	  }
     }
   return NULL;
}

int SLregexp_nth_match (SLRegexp_Type *reg, unsigned int nth,
			unsigned int *ofsp, unsigned int *lenp)
{
   if (nth >= 10)
     {
	SLang_set_error (SL_InvalidParm_Error);
	return -1;
     }
   if (reg->beg_matches[nth] < 0)
     return -1;

   if (ofsp != NULL)
     *ofsp = (unsigned int) reg->beg_matches[nth];
   if (lenp != NULL)
     *lenp = (unsigned int) reg->end_matches[nth];

   return 0;
}

int SLregexp_get_hints (SLRegexp_Type *reg, unsigned int *hintsp)
{
   unsigned int hints = 0;

   if (reg == NULL)
     return -1;

   if (reg->osearch) hints |= SLREGEXP_HINT_OSEARCH;
   if (reg->must_match_bol) hints |= SLREGEXP_HINT_BOL;

   *hintsp = hints;
   return 0;
}

#if 0
#define MAX_EXP 4096
int main(int argc, char **argv)
{
   FILE *fp;
   char *regexp, *file;
   char expbuf[MAX_EXP], buf[512];
   SLRegexp_Type reg;

   file = argv[2];
   regexp = argv[1];

   if (NULL == (fp = fopen(file, "r")))
     {
	fprintf(stderr, "File not open\n");
	return(1);
     }

   reg.buf = expbuf;
   reg.buf_len = MAX_EXP;
   reg.pat = regexp;
   reg.case_sensitive = 1;

   if (!regexp_compile(&reg)) while (NULL != fgets(buf, 511, fp))
     {
	if (reg.osearch)
	  {
	     if (NULL == strstr(buf, reg.pat)) continue;
	  }
	else
	  {
	     if (reg.must_match && (NULL == strstr(buf, reg.must_match_str))) continue;
	     if (0 == regexp_match(buf, buf + strlen(buf), &reg)) continue;
	  }

	fputs(buf, stdout);
     }
   return (0);
}
#endif

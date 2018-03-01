/* Copyright (c) 1993-2003
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program (see the file COPYING); if not, see
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include <sys/types.h>

#include "config.h"
#include "screen.h"
#include "extern.h"

#ifdef ENCODINGS

extern unsigned char *null;
extern struct display *display, *displays;
extern struct layer *flayer;

extern char *screenencodings;

#ifdef DW_CHARS
extern int cjkwidth;
#endif

static int  encmatch __P((char *, char *));
# ifdef UTF8
static int   recode_char __P((int, int, int));
static int   recode_char_to_encoding __P((int, int));
static void  comb_tofront __P((int, int));
#  ifdef DW_CHARS
static int   recode_char_dw __P((int, int *, int, int));
static int   recode_char_dw_to_encoding __P((int, int *, int));
#  endif
# endif

struct encoding {
  char *name;
  char *charsets;
  int  deffont;
  int  usegr;
  int  noc1;
  char *fontlist; 
};

/* big5 font:   ^X */
/* KOI8-R font: 96 ! */
/* CP1251 font: 96 ? */

struct encoding encodings[] = {
  { "C",		0,		0,		0, 0, 0 },
  { "eucJP",		"B\002I\00401",	0,		1, 0, "\002\004I" },
  { "SJIS",		"BIBB01",	0,		1, 1, "\002I" },
  { "eucKR",		"B\003BB01",	0,		1, 0, "\003" },
  { "eucCN",		"B\001BB01",	0,		1, 0, "\001" },
  { "Big5",		"B\030BB01",	0,		1, 0, "\030" },
  { "KOI8-R",		0,		0x80|'!',	0, 1, 0 },
  { "CP1251",		0,		0x80|'?',	0, 1, 0 },
  { "UTF-8",		0,		-1,		0, 0, 0 },
  { "ISO8859-2",	0,		0x80|'B',	0, 0, 0 },
  { "ISO8859-3",	0,		0x80|'C',	0, 0, 0 },
  { "ISO8859-4",	0,		0x80|'D',	0, 0, 0 },
  { "ISO8859-5",	0,		0x80|'L',	0, 0, 0 },
  { "ISO8859-6",	0,		0x80|'G',	0, 0, 0 },
  { "ISO8859-7",	0,		0x80|'F',	0, 0, 0 },
  { "ISO8859-8",	0,		0x80|'H',	0, 0, 0 },
  { "ISO8859-9",	0,		0x80|'M',	0, 0, 0 },
  { "ISO8859-10",	0,		0x80|'V',	0, 0, 0 },
  { "ISO8859-15",	0,		0x80|'b',	0, 0, 0 },
  { "jis",		0,		0,		0, 0, "\002\004I" },
  { "GBK",		"B\031BB01",	0x80|'b',	1, 1, "\031" }
};

#ifdef UTF8

static unsigned short builtin_tabs[][2] = {
  { 0x30, 0 },		/* 0: special graphics (line drawing) */
  { 0x005f, 0x25AE },
  { 0x0060, 0x25C6 },
  { 0x0061, 0x2592 },
  { 0x0062, 0x2409 },
  { 0x0063, 0x240C },
  { 0x0064, 0x240D },
  { 0x0065, 0x240A },
  { 0x0066, 0x00B0 },
  { 0x0067, 0x00B1 },
  { 0x0068, 0x2424 },
  { 0x0069, 0x240B },
  { 0x006a, 0x2518 },
  { 0x006b, 0x2510 },
  { 0x006c, 0x250C },
  { 0x006d, 0x2514 },
  { 0x006e, 0x253C },
  { 0x006f, 0x23BA },
  { 0x0070, 0x23BB },
  { 0x0071, 0x2500 },
  { 0x0072, 0x23BC },
  { 0x0073, 0x23BD },
  { 0x0074, 0x251C },
  { 0x0075, 0x2524 },
  { 0x0076, 0x2534 },
  { 0x0077, 0x252C },
  { 0x0078, 0x2502 },
  { 0x0079, 0x2264 },
  { 0x007a, 0x2265 },
  { 0x007b, 0x03C0 },
  { 0x007c, 0x2260 },
  { 0x007d, 0x00A3 },
  { 0x007e, 0x00B7 },
  { 0, 0},

  { 0x34, 0 },		/* 4: Dutch */
  { 0x0023, 0x00a3 },
  { 0x0040, 0x00be },
  { 0x005b, 0x00ff },
  { 0x005c, 0x00bd },
  { 0x005d, 0x007c },
  { 0x007b, 0x00a8 },
  { 0x007c, 0x0066 },
  { 0x007d, 0x00bc },
  { 0x007e, 0x00b4 },
  { 0, 0},

  { 0x35, 0 },		/* 5: Finnish */
  { 0x005b, 0x00c4 },
  { 0x005c, 0x00d6 },
  { 0x005d, 0x00c5 },
  { 0x005e, 0x00dc },
  { 0x0060, 0x00e9 },
  { 0x007b, 0x00e4 },
  { 0x007c, 0x00f6 },
  { 0x007d, 0x00e5 },
  { 0x007e, 0x00fc },
  { 0, 0},

  { 0x36, 0 },		/* 6: Norwegian/Danish */
  { 0x0040, 0x00c4 },
  { 0x005b, 0x00c6 },
  { 0x005c, 0x00d8 },
  { 0x005d, 0x00c5 },
  { 0x005e, 0x00dc },
  { 0x0060, 0x00e4 },
  { 0x007b, 0x00e6 },
  { 0x007c, 0x00f8 },
  { 0x007d, 0x00e5 },
  { 0x007e, 0x00fc },
  { 0, 0},

  { 0x37, 0 },		/* 7: Swedish */
  { 0x0040, 0x00c9 },
  { 0x005b, 0x00c4 },
  { 0x005c, 0x00d6 },
  { 0x005d, 0x00c5 },
  { 0x005e, 0x00dc },
  { 0x0060, 0x00e9 },
  { 0x007b, 0x00e4 },
  { 0x007c, 0x00f6 },
  { 0x007d, 0x00e5 },
  { 0x007e, 0x00fc },
  { 0, 0},

  { 0x3d, 0},		/* =: Swiss */
  { 0x0023, 0x00f9 },
  { 0x0040, 0x00e0 },
  { 0x005b, 0x00e9 },
  { 0x005c, 0x00e7 },
  { 0x005d, 0x00ea },
  { 0x005e, 0x00ee },
  { 0x005f, 0x00e8 },
  { 0x0060, 0x00f4 },
  { 0x007b, 0x00e4 },
  { 0x007c, 0x00f6 },
  { 0x007d, 0x00fc },
  { 0x007e, 0x00fb },
  { 0, 0},

  { 0x41, 0},		/* A: UK */
  { 0x0023, 0x00a3 },
  { 0, 0},

  { 0x4b, 0},		/* K: German */
  { 0x0040, 0x00a7 },
  { 0x005b, 0x00c4 },
  { 0x005c, 0x00d6 },
  { 0x005d, 0x00dc },
  { 0x007b, 0x00e4 },
  { 0x007c, 0x00f6 },
  { 0x007d, 0x00fc },
  { 0x007e, 0x00df },
  { 0, 0},

  { 0x51, 0},		/* Q: French Canadian */
  { 0x0040, 0x00e0 },
  { 0x005b, 0x00e2 },
  { 0x005c, 0x00e7 },
  { 0x005d, 0x00ea },
  { 0x005e, 0x00ee },
  { 0x0060, 0x00f4 },
  { 0x007b, 0x00e9 },
  { 0x007c, 0x00f9 },
  { 0x007d, 0x00e8 },
  { 0x007e, 0x00fb },
  { 0, 0},

  { 0x52, 0},		/* R: French */
  { 0x0023, 0x00a3 },
  { 0x0040, 0x00e0 },
  { 0x005b, 0x00b0 },
  { 0x005c, 0x00e7 },
  { 0x005d, 0x00a7 },
  { 0x007b, 0x00e9 },
  { 0x007c, 0x00f9 },
  { 0x007d, 0x00e8 },
  { 0x007e, 0x00a8 },
  { 0, 0},

  { 0x59, 0},		/* Y: Italian */
  { 0x0023, 0x00a3 },
  { 0x0040, 0x00a7 },
  { 0x005b, 0x00b0 },
  { 0x005c, 0x00e7 },
  { 0x005d, 0x00e9 },
  { 0x0060, 0x00f9 },
  { 0x007b, 0x00e0 },
  { 0x007c, 0x00f2 },
  { 0x007d, 0x00e8 },
  { 0x007e, 0x00ec },
  { 0, 0},

  { 0x5a, 0},		/* Z: Spanish */
  { 0x0023, 0x00a3 },
  { 0x0040, 0x00a7 },
  { 0x005b, 0x00a1 },
  { 0x005c, 0x00d1 },
  { 0x005d, 0x00bf },
  { 0x007b, 0x00b0 },
  { 0x007c, 0x00f1 },
  { 0x007d, 0x00e7 },
  { 0, 0},

  { 0xe2, 0},		/* 96-b: ISO-8859-15 */
  { 0x00a4, 0x20ac },
  { 0x00a6, 0x0160 },
  { 0x00a8, 0x0161 },
  { 0x00b4, 0x017D },
  { 0x00b8, 0x017E },
  { 0x00bc, 0x0152 },
  { 0x00bd, 0x0153 },
  { 0x00be, 0x0178 },
  { 0, 0},

  { 0x4a, 0},		/* J: JIS 0201 Roman */
  { 0x005c, 0x00a5 },
  { 0x007e, 0x203e },
  { 0, 0},

  { 0x49, 0},		/* I: halfwidth katakana */
  { 0x0021, 0xff61 },
  { 0x005f|0x8000, 0xff9f },
  { 0, 0},

  { 0, 0}
};

struct recodetab
{
  unsigned short (*tab)[2];
  int flags;
};

#define RECODETAB_ALLOCED	1
#define RECODETAB_BUILTIN	2
#define RECODETAB_TRIED		4

static struct recodetab recodetabs[256];

void
InitBuiltinTabs()
{
  unsigned short (*p)[2];
  for (p = builtin_tabs; (*p)[0]; p++)
    {
      recodetabs[(*p)[0]].flags = RECODETAB_BUILTIN;
      recodetabs[(*p)[0]].tab = p + 1;
      p++;
      while((*p)[0])
	p++;
    }
}

static int
recode_char(c, to_utf, font)
int c, to_utf, font;
{
  int f;
  unsigned short (*p)[2];

  if (to_utf)
    {
      if (c < 256)
	return c;
      f = (c >> 8) & 0xff;
      c &= 0xff;
      /* map aliases to keep the table small */
      switch (f)
	{
	  case 'C':
	    f ^= ('C' ^ '5');
	    break;
	  case 'E':
	    f ^= ('E' ^ '6');
	    break;
	  case 'H':
	    f ^= ('H' ^ '7');
	    break;
	  default:
	    break;
	}
      p = recodetabs[f].tab;
      if (p == 0 && recodetabs[f].flags == 0)
	{
	  LoadFontTranslation(f, 0);
          p = recodetabs[f].tab;
	}
      if (p)
        for (; (*p)[0]; p++)
	  {
	    if ((p[0][0] & 0x8000) && (c <= (p[0][0] & 0x7fff)) && c >= p[-1][0])
	      return c - p[-1][0] + p[-1][1];
	    if ((*p)[0] == c)
	      return (*p)[1];
	  }
      return c & 0xff;	/* map to latin1 */
    }
  if (font == -1)
    {
      if (c < 256)
	return c;	/* latin1 */
      for (font = 32; font < 128; font++)
	{
	  p = recodetabs[font].tab;
	  if (p)
	    for (; (*p)[1]; p++)
	      {
		if ((p[0][0] & 0x8000) && c <= p[0][1] && c >= p[-1][1])
		  return (c - p[-1][1] + p[-1][0]) | (font << 8);
	        if ((*p)[1] == c)
		  return (*p)[0] | (font << 8);
	      }
	}
      return '?';
    }
  if (c < 128 && (font & 128) != 0)
    return c;
  if (font >= 32)
    {
      p = recodetabs[font].tab;
      if (p == 0 && recodetabs[font].flags == 0)
	{
	  LoadFontTranslation(font, 0);
          p = recodetabs[font].tab;
	}
      if (p)
	for (; (*p)[1]; p++)
	  {
	    if ((p[0][0] & 0x8000) && c <= p[0][1] && c >= p[-1][1])
	      return (c - p[-1][1] + p[-1][0]) | (font & 128 ? 0 : font << 8);
	    if ((*p)[1] == c)
	      return (*p)[0] | (font & 128 ? 0 : font << 8);
	  }
    }
  return -1;
}


#ifdef DW_CHARS
static int
recode_char_dw(c, c2p, to_utf, font)
int c, *c2p, to_utf, font;
{
  int f;
  unsigned short (*p)[2];

  if (to_utf)
    {
      f = (c >> 8) & 0xff;
      c = (c & 255) << 8 | (*c2p & 255);
      *c2p = 0xffff;
      p = recodetabs[f].tab;
      if (p == 0 && recodetabs[f].flags == 0)
	{
	  LoadFontTranslation(f, 0);
          p = recodetabs[f].tab;
	}
      if (p)
        for (; (*p)[0]; p++)
	  if ((*p)[0] == c)
	    {
#ifdef DW_CHARS
	      if (!utf8_isdouble((*p)[1]))
		*c2p = ' ';
#endif
	      return (*p)[1];
	    }
      return UCS_REPL_DW;
    }
  if (font == -1)
    {
      for (font = 0; font < 030; font++)
	{
	  p = recodetabs[font].tab;
	  if (p)
	    for (; (*p)[1]; p++)
	      if ((*p)[1] == c)
		{
		  *c2p = ((*p)[0] & 255) | font << 8 | 0x8000;
		  return ((*p)[0] >> 8) | font << 8;
		}
	}
      *c2p = '?';
      return '?';
    }
  if (font < 32)
    {
      p = recodetabs[font].tab;
      if (p == 0 && recodetabs[font].flags == 0)
	{
	  LoadFontTranslation(font, 0);
          p = recodetabs[font].tab;
	}
      if (p)
	for (; (*p)[1]; p++)
	  if ((*p)[1] == c)
	    {
	      *c2p = ((*p)[0] & 255) | font << 8 | 0x8000;
	      return ((*p)[0] >> 8) | font << 8;
	    }
    }
  return -1;
}
#endif

static int
recode_char_to_encoding(c, encoding)
int c, encoding;
{
  char *fp;
  int x;

  if (encoding == UTF8)
    return recode_char(c, 1, -1);
  if ((fp = encodings[encoding].fontlist) != 0)
    while(*fp)
      if ((x = recode_char(c, 0, (unsigned char)*fp++)) != -1)
        return x;
  if (encodings[encoding].deffont)
    if ((x = recode_char(c, 0, encodings[encoding].deffont)) != -1)
      return x;
  return recode_char(c, 0, -1);
}

#ifdef DW_CHARS
static int
recode_char_dw_to_encoding(c, c2p, encoding)
int c, *c2p, encoding;
{
  char *fp;
  int x;

  if (encoding == UTF8)
    return recode_char_dw(c, c2p, 1, -1);
  if ((fp = encodings[encoding].fontlist) != 0)
    while(*fp)
      if ((x = recode_char_dw(c, c2p, 0, (unsigned char)*fp++)) != -1)
        return x;
  if (encodings[encoding].deffont)
    if ((x = recode_char_dw(c, c2p, 0, encodings[encoding].deffont)) != -1)
      return x;
  return recode_char_dw(c, c2p, 0, -1);
}
#endif


struct mchar *
recode_mchar(mc, from, to)
struct mchar *mc;
int from, to;
{
  static struct mchar rmc;
  int c;

  debug3("recode_mchar %02x from %d to %d\n", mc->image, from, to);
  if (from == to || (from != UTF8 && to != UTF8))
    return mc;
  rmc = *mc;
  if (rmc.font == 0 && from != UTF8)
    rmc.font = encodings[from].deffont;
  if (rmc.font == 0)	/* latin1 is the same in unicode */
    return mc;
  c = rmc.image | (rmc.font << 8);
  if (from == UTF8)
    c |= rmc.fontx << 16;
#ifdef DW_CHARS
  if (rmc.mbcs)
    {
      int c2 = rmc.mbcs;
      c = recode_char_dw_to_encoding(c, &c2, to);
      rmc.mbcs = c2;
    }
  else
#endif
    c = recode_char_to_encoding(c, to);
  rmc.image = c & 255;
  rmc.font = c >> 8 & 255;
  if (to == UTF8)
    rmc.fontx = c >> 16 & 255;
  return &rmc;
}

struct mline *
recode_mline(ml, w, from, to)
struct mline *ml;
int w;
int from, to;
{
  static int maxlen;
  static int last;
  static struct mline rml[2], *rl;
  int i, c;

  if (from == to || (from != UTF8 && to != UTF8) || w == 0)
    return ml;
  if (ml->font == null && ml->fontx == null && encodings[from].deffont == 0)
    return ml;
  if (w > maxlen)
    {
      for (i = 0; i < 2; i++)
	{
	  if (rml[i].image == 0)
	    rml[i].image = malloc(w);
	  else
	    rml[i].image = realloc(rml[i].image, w);
	  if (rml[i].font == 0)
	    rml[i].font = malloc(w);
	  else
	    rml[i].font = realloc(rml[i].font, w);
	  if (rml[i].fontx == 0)
	    rml[i].fontx = malloc(w);
	  else
	    rml[i].fontx = realloc(rml[i].fontx, w);
	  if (rml[i].image == 0 || rml[i].font == 0 || rml[i].fontx == 0)
	    {
	      maxlen = 0;
	      return ml;	/* sorry */
	    }
	}
      maxlen = w;
    }

  debug("recode_mline: from\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->image[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->image[i]     ) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->font[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->font[i]     ) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->fontx[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(ml->fontx[i]     ) & 15]);
  debug("\n");

  rl = rml + last;
  rl->attr = ml->attr;
#ifdef COLOR
  rl->color = ml->color;
# ifdef COLORS256
  rl->colorx = ml->colorx;
# endif
#endif
  for (i = 0; i < w; i++)
    {
      c = ml->image[i] | (ml->font[i] << 8);
      if (from == UTF8)
	c |= ml->fontx[i] << 16;
      if (from != UTF8 && c < 256)
	c |= encodings[from].deffont << 8;
#ifdef DW_CHARS
      if ((from != UTF8 && (c & 0x1f00) != 0 && (c & 0xe000) == 0) || (from == UTF8 && utf8_isdouble(c)))
	{
	  if (i + 1 == w)
	    c = '?';
	  else
	    {
	      int c2;
	      i++;
	      c2 = ml->image[i] | (ml->font[i] << 8);
	      c = recode_char_dw_to_encoding(c, &c2, to);
	      if (to == UTF8)
	        rl->fontx[i - 1]  = c >> 16 & 255;
	      rl->font[i - 1]  = c >> 8 & 255;
	      rl->image[i - 1] = c      & 255;
	      c = c2;
	    }
	}
      else
#endif
        c = recode_char_to_encoding(c, to);
      rl->image[i] = c & 255;
      rl->font[i] = c >> 8 & 255;
      if (to == UTF8)
        rl->fontx[i] = c >> 16 & 255;
    }
  last ^= 1;
  debug("recode_mline: to\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->image[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->image[i]     ) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->font[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->font[i]     ) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->fontx[i] >> 4) & 15]);
  debug("\n");
  for (i = 0; i < w; i++)
    debug1("%c", "0123456789abcdef"[(rl->fontx[i]     ) & 15]);
  debug("\n");
  return rl;
}

struct combchar {
  unsigned int c1;
  unsigned int c2;
  unsigned int next;
  unsigned int prev;
};
struct combchar **combchars;

void
AddUtf8(c)
int c;
{
  ASSERT(D_encoding == UTF8);
  if (c >= 0xd800 && c < 0xe000 && combchars && combchars[c - 0xd800])
    {
      AddUtf8(combchars[c - 0xd800]->c1);
      c = combchars[c - 0xd800]->c2;
    }
  if (c >= 0x10000)
    {
      if (c >= 0x200000)
	{
	  AddChar((c & 0x3000000) >> 12 ^ 0xf8);
	  c = (c & 0xffffff) ^ ((0xf0 ^ 0x80) << 18);
	}
      AddChar((c & 0x1fc0000) >> 18 ^ 0xf0);
      c = (c & 0x3ffff) ^ ((0xe0 ^ 0x80) << 12);
    }
  if (c >= 0x800)
    {
      AddChar((c & 0x7f000) >> 12 ^ 0xe0);
      c = (c & 0x0fff) ^ ((0xc0 ^ 0x80) << 6);
    }
  if (c >= 0x80)
    {
      AddChar((c & 0x1fc0) >> 6 ^ 0xc0);
      c = (c & 0x3f) | 0x80; 
    }
  AddChar(c);
}

int
ToUtf8_comb(p, c)
char *p;
int c;
{
  int l;

  if (c >= 0xd800 && c < 0xe000 && combchars && combchars[c - 0xd800])
    {
      l = ToUtf8_comb(p, combchars[c - 0xd800]->c1);
      return l + ToUtf8(p ? p + l : 0, combchars[c - 0xd800]->c2);
    }
  return ToUtf8(p, c);
}

int
ToUtf8(p, c)
char *p;
int c;
{
  int l = 1;
  if (c >= 0x10000)
    {
      if (c >= 0x200000)
	{
	  if (p)
	    *p++ = (c & 0x3000000) >> 12 ^ 0xf8;
	  l++;
	  c = (c & 0xffffff) ^ ((0xf0 ^ 0x80) << 18);
	}
      if (p)
        *p++ = (c & 0x1fc0000) >> 18 ^ 0xf0;
      l++;
      c = (c & 0x3ffff) ^ ((0xe0 ^ 0x80) << 12);
    }
  if (c >= 0x800)
    {
      if (p)
	*p++ = (c & 0x7f000) >> 12 ^ 0xe0; 
      l++;
      c = (c & 0x0fff) | 0x1000; 
    }
  if (c >= 0x80)
    {
      if (p)
	*p++ = (c & 0x1fc0) >> 6 ^ 0xc0; 
      l++;
      c = (c & 0x3f) | 0x80; 
    }
  if (p)
    *p++ = c;
  return l;
}

/*
 * returns:
 * -1: need more bytes, sequence not finished
 * -2: corrupt sequence found, redo last char
 * >= 0: decoded character
 */
int
FromUtf8(c, utf8charp)
int c, *utf8charp;
{
  int utf8char = *utf8charp;
  if (utf8char)
    {
      if ((c & 0xc0) != 0x80)
	{
	  *utf8charp = 0;
	  return -2; /* corrupt sequence! */
	}
      else
	c = (c & 0x3f) | (utf8char << 6);
      if (!(utf8char & 0x40000000))
	{
	  /* check for overlong sequences */
	  if ((c & 0x820823e0) == 0x80000000)
	    c = 0xfdffffff;
	  else if ((c & 0x020821f0) == 0x02000000)
	    c = 0xfff7ffff;
	  else if ((c & 0x000820f8) == 0x00080000)
	    c = 0xffffd000;
	  else if ((c & 0x0000207c) == 0x00002000)
	    c = 0xffffff70;
	}
    }
  else
    {
      /* new sequence */
      if (c >= 0xfe)
	c = UCS_REPL;
      else if (c >= 0xfc)
	c = (c & 0x01) | 0xbffffffc;	/* 5 bytes to follow */
      else if (c >= 0xf8)
	c = (c & 0x03) | 0xbfffff00;	/* 4 */
      else if (c >= 0xf0)
	c = (c & 0x07) | 0xbfffc000;	/* 3 */
      else if (c >= 0xe0)
	c = (c & 0x0f) | 0xbff00000;	/* 2 */
      else if (c >= 0xc2)
	c = (c & 0x1f) | 0xfc000000;	/* 1 */
      else if (c >= 0xc0)
	c = 0xfdffffff;		/* overlong */
      else if (c >= 0x80)
	c = UCS_REPL;
    }
  *utf8charp = utf8char = (c & 0x80000000) ? c : 0;
  if (utf8char)
    return -1;
#if 0
  if (c & 0xffff0000)
    c = UCS_REPL;	/* sorry, only know 16bit Unicode */
#else
  if (c & 0xff800000)
    c = UCS_REPL;	/* sorry, only know 23bit Unicode */
#endif
  if (c >= 0xd800 && (c <= 0xdfff || c == 0xfffe || c == 0xffff))
    c = UCS_REPL;	/* illegal code */
  return c;
}


void
WinSwitchEncoding(p, encoding)
struct win *p;
int encoding;
{
  int i, j, c;
  struct mline *ml;
  struct display *d;
  struct canvas *cv;
  struct layer *oldflayer;

  if ((p->w_encoding == UTF8) == (encoding == UTF8))
    {
      p->w_encoding = encoding;
      return;
    }
  oldflayer = flayer;
  for (d = displays; d; d = d->d_next)
    for (cv = d->d_cvlist; cv; cv = cv->c_next)
      if (p == Layer2Window(cv->c_layer))
	{
	  flayer = cv->c_layer;
	  while(flayer->l_next)
	    {
	      if (oldflayer == flayer)
		oldflayer = flayer->l_next;
	      ExitOverlayPage();
	    }
	}
  flayer = oldflayer;
  for (j = 0; j < p->w_height + p->w_histheight; j++)
    {
#ifdef COPY_PASTE
      ml = j < p->w_height ? &p->w_mlines[j] : &p->w_hlines[j - p->w_height];
#else
      ml = &p->w_mlines[j];
#endif
      if (ml->font == null && ml->fontx == 0 && encodings[p->w_encoding].deffont == 0)
	continue;
      for (i = 0; i < p->w_width; i++)
	{
	  c = ml->image[i] | (ml->font[i] << 8);
	  if (p->w_encoding == UTF8)
	    c |= ml->fontx[i] << 16;
	  if (p->w_encoding != UTF8 && c < 256)
	    c |= encodings[p->w_encoding].deffont << 8;
	  if (c < 256)
	    continue;
	  if (ml->font == null)
	    {
	      if ((ml->font = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
		{
		  ml->font = null;
		  break;
		}
	    }
#ifdef DW_CHARS
	  if ((p->w_encoding != UTF8 && (c & 0x1f00) != 0 && (c & 0xe000) == 0) || (p->w_encoding == UTF8 && utf8_isdouble(c)))
	    {
	      if (i + 1 == p->w_width)
		c = '?';
	      else
		{
		  int c2;
		  i++;
		  c2 = ml->image[i] | (ml->font[i] << 8) | (ml->fontx[i] << 16);
		  c = recode_char_dw_to_encoding(c, &c2, encoding);
		  if (encoding == UTF8)
		    {
		      if (c > 0x10000 && ml->fontx == null)
			{
			  if ((ml->fontx = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
			    {
			      ml->fontx = null;
			      break;
			    }
			}
		      ml->fontx[i - 1]  = c >> 16 & 255;
		    }
		  else
		    ml->fontx = null;
		  ml->font[i - 1]  = c >> 8 & 255;
		  ml->image[i - 1] = c      & 255;
		  c = c2;
		}
	    }
	  else
#endif
	    c = recode_char_to_encoding(c, encoding);
	  ml->image[i] = c & 255;
	  ml->font[i] = c >> 8 & 255;
	  if (encoding == UTF8)
	    {
	      if (c > 0x10000 && ml->fontx == null)
		{
		  if ((ml->fontx = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
		    {
		      ml->fontx = null;
		      break;
		    }
		}
	      ml->fontx[i]  = c >> 16 & 255;
	    }
	  else
	    ml->fontx = null;
	}
    }
  p->w_encoding = encoding;
  return;
}

#ifdef DW_CHARS
struct interval {
  int first;
  int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(int ucs, const struct interval *table, int max) {
  int min = 0;
  int mid;

  if (ucs < table[0].first || ucs > table[max].last)
    return 0;
  while (max >= min) {
    mid = (min + max) / 2;
    if (ucs > table[mid].last)
      min = mid + 1;
    else if (ucs < table[mid].first)
      max = mid - 1;
    else
      return 1;
  }

  return 0;
}

int
utf8_isdouble(c)
int c;
{
  /* A sorted list of intervals of ambiguous width characters generated by
   * https://github.com/GNOME/glib/blob/glib-2-50/glib/gen-unicode-tables.pl */
  static const struct interval ambiguous[] = {
    {0x00A1, 0x00A1},
    {0x00A4, 0x00A4},
    {0x00A7, 0x00A8},
    {0x00AA, 0x00AA},
    {0x00AD, 0x00AE},
    {0x00B0, 0x00B4},
    {0x00B6, 0x00BA},
    {0x00BC, 0x00BF},
    {0x00C6, 0x00C6},
    {0x00D0, 0x00D0},
    {0x00D7, 0x00D8},
    {0x00DE, 0x00E1},
    {0x00E6, 0x00E6},
    {0x00E8, 0x00EA},
    {0x00EC, 0x00ED},
    {0x00F0, 0x00F0},
    {0x00F2, 0x00F3},
    {0x00F7, 0x00FA},
    {0x00FC, 0x00FC},
    {0x00FE, 0x00FE},
    {0x0101, 0x0101},
    {0x0111, 0x0111},
    {0x0113, 0x0113},
    {0x011B, 0x011B},
    {0x0126, 0x0127},
    {0x012B, 0x012B},
    {0x0131, 0x0133},
    {0x0138, 0x0138},
    {0x013F, 0x0142},
    {0x0144, 0x0144},
    {0x0148, 0x014B},
    {0x014D, 0x014D},
    {0x0152, 0x0153},
    {0x0166, 0x0167},
    {0x016B, 0x016B},
    {0x01CE, 0x01CE},
    {0x01D0, 0x01D0},
    {0x01D2, 0x01D2},
    {0x01D4, 0x01D4},
    {0x01D6, 0x01D6},
    {0x01D8, 0x01D8},
    {0x01DA, 0x01DA},
    {0x01DC, 0x01DC},
    {0x0251, 0x0251},
    {0x0261, 0x0261},
    {0x02C4, 0x02C4},
    {0x02C7, 0x02C7},
    {0x02C9, 0x02CB},
    {0x02CD, 0x02CD},
    {0x02D0, 0x02D0},
    {0x02D8, 0x02DB},
    {0x02DD, 0x02DD},
    {0x02DF, 0x02DF},
    {0x0300, 0x036F},
    {0x0391, 0x03A1},
    {0x03A3, 0x03A9},
    {0x03B1, 0x03C1},
    {0x03C3, 0x03C9},
    {0x0401, 0x0401},
    {0x0410, 0x044F},
    {0x0451, 0x0451},
    {0x2010, 0x2010},
    {0x2013, 0x2016},
    {0x2018, 0x2019},
    {0x201C, 0x201D},
    {0x2020, 0x2022},
    {0x2024, 0x2027},
    {0x2030, 0x2030},
    {0x2032, 0x2033},
    {0x2035, 0x2035},
    {0x203B, 0x203B},
    {0x203E, 0x203E},
    {0x2074, 0x2074},
    {0x207F, 0x207F},
    {0x2081, 0x2084},
    {0x20AC, 0x20AC},
    {0x2103, 0x2103},
    {0x2105, 0x2105},
    {0x2109, 0x2109},
    {0x2113, 0x2113},
    {0x2116, 0x2116},
    {0x2121, 0x2122},
    {0x2126, 0x2126},
    {0x212B, 0x212B},
    {0x2153, 0x2154},
    {0x215B, 0x215E},
    {0x2160, 0x216B},
    {0x2170, 0x2179},
    {0x2189, 0x2189},
    {0x2190, 0x2199},
    {0x21B8, 0x21B9},
    {0x21D2, 0x21D2},
    {0x21D4, 0x21D4},
    {0x21E7, 0x21E7},
    {0x2200, 0x2200},
    {0x2202, 0x2203},
    {0x2207, 0x2208},
    {0x220B, 0x220B},
    {0x220F, 0x220F},
    {0x2211, 0x2211},
    {0x2215, 0x2215},
    {0x221A, 0x221A},
    {0x221D, 0x2220},
    {0x2223, 0x2223},
    {0x2225, 0x2225},
    {0x2227, 0x222C},
    {0x222E, 0x222E},
    {0x2234, 0x2237},
    {0x223C, 0x223D},
    {0x2248, 0x2248},
    {0x224C, 0x224C},
    {0x2252, 0x2252},
    {0x2260, 0x2261},
    {0x2264, 0x2267},
    {0x226A, 0x226B},
    {0x226E, 0x226F},
    {0x2282, 0x2283},
    {0x2286, 0x2287},
    {0x2295, 0x2295},
    {0x2299, 0x2299},
    {0x22A5, 0x22A5},
    {0x22BF, 0x22BF},
    {0x2312, 0x2312},
    {0x2460, 0x24E9},
    {0x24EB, 0x254B},
    {0x2550, 0x2573},
    {0x2580, 0x258F},
    {0x2592, 0x2595},
    {0x25A0, 0x25A1},
    {0x25A3, 0x25A9},
    {0x25B2, 0x25B3},
    {0x25B6, 0x25B7},
    {0x25BC, 0x25BD},
    {0x25C0, 0x25C1},
    {0x25C6, 0x25C8},
    {0x25CB, 0x25CB},
    {0x25CE, 0x25D1},
    {0x25E2, 0x25E5},
    {0x25EF, 0x25EF},
    {0x2605, 0x2606},
    {0x2609, 0x2609},
    {0x260E, 0x260F},
    {0x261C, 0x261C},
    {0x261E, 0x261E},
    {0x2640, 0x2640},
    {0x2642, 0x2642},
    {0x2660, 0x2661},
    {0x2663, 0x2665},
    {0x2667, 0x266A},
    {0x266C, 0x266D},
    {0x266F, 0x266F},
    {0x269E, 0x269F},
    {0x26BF, 0x26BF},
    {0x26C6, 0x26CD},
    {0x26CF, 0x26D3},
    {0x26D5, 0x26E1},
    {0x26E3, 0x26E3},
    {0x26E8, 0x26E9},
    {0x26EB, 0x26F1},
    {0x26F4, 0x26F4},
    {0x26F6, 0x26F9},
    {0x26FB, 0x26FC},
    {0x26FE, 0x26FF},
    {0x273D, 0x273D},
    {0x2776, 0x277F},
    {0x2B56, 0x2B59},
    {0x3248, 0x324F},
    {0xE000, 0xF8FF},
    {0xFE00, 0xFE0F},
    {0xFFFD, 0xFFFD},
    {0x1F100, 0x1F10A},
    {0x1F110, 0x1F12D},
    {0x1F130, 0x1F169},
    {0x1F170, 0x1F18D},
    {0x1F18F, 0x1F190},
    {0x1F19B, 0x1F1AC},
    {0xE0100, 0xE01EF},
    {0xF0000, 0xFFFFD},
    {0x100000, 0x10FFFD},
  };
  /* A sorted list of intervals of double width characters generated by
   * https://github.com/GNOME/glib/blob/glib-2-50/glib/gen-unicode-tables.pl */
  static const struct interval wide[] = {
    {0x1100, 0x115F},
    {0x231A, 0x231B},
    {0x2329, 0x232A},
    {0x23E9, 0x23EC},
    {0x23F0, 0x23F0},
    {0x23F3, 0x23F3},
    {0x25FD, 0x25FE},
    {0x2614, 0x2615},
    {0x2648, 0x2653},
    {0x267F, 0x267F},
    {0x2693, 0x2693},
    {0x26A1, 0x26A1},
    {0x26AA, 0x26AB},
    {0x26BD, 0x26BE},
    {0x26C4, 0x26C5},
    {0x26CE, 0x26CE},
    {0x26D4, 0x26D4},
    {0x26EA, 0x26EA},
    {0x26F2, 0x26F3},
    {0x26F5, 0x26F5},
    {0x26FA, 0x26FA},
    {0x26FD, 0x26FD},
    {0x2705, 0x2705},
    {0x270A, 0x270B},
    {0x2728, 0x2728},
    {0x274C, 0x274C},
    {0x274E, 0x274E},
    {0x2753, 0x2755},
    {0x2757, 0x2757},
    {0x2795, 0x2797},
    {0x27B0, 0x27B0},
    {0x27BF, 0x27BF},
    {0x2B1B, 0x2B1C},
    {0x2B50, 0x2B50},
    {0x2B55, 0x2B55},
    {0x2E80, 0x2E99},
    {0x2E9B, 0x2EF3},
    {0x2F00, 0x2FD5},
    {0x2FF0, 0x2FFB},
    {0x3000, 0x303E},
    {0x3041, 0x3096},
    {0x3099, 0x30FF},
    {0x3105, 0x312D},
    {0x3131, 0x318E},
    {0x3190, 0x31BA},
    {0x31C0, 0x31E3},
    {0x31F0, 0x321E},
    {0x3220, 0x3247},
    {0x3250, 0x32FE},
    {0x3300, 0x4DBF},
    {0x4E00, 0xA48C},
    {0xA490, 0xA4C6},
    {0xA960, 0xA97C},
    {0xAC00, 0xD7A3},
    {0xF900, 0xFAFF},
    {0xFE10, 0xFE19},
    {0xFE30, 0xFE52},
    {0xFE54, 0xFE66},
    {0xFE68, 0xFE6B},
    {0xFF01, 0xFF60},
    {0xFFE0, 0xFFE6},
    {0x16FE0, 0x16FE0},
    {0x17000, 0x187EC},
    {0x18800, 0x18AF2},
    {0x1B000, 0x1B001},
    {0x1F004, 0x1F004},
    {0x1F0CF, 0x1F0CF},
    {0x1F18E, 0x1F18E},
    {0x1F191, 0x1F19A},
    {0x1F200, 0x1F202},
    {0x1F210, 0x1F23B},
    {0x1F240, 0x1F248},
    {0x1F250, 0x1F251},
    {0x1F300, 0x1F320},
    {0x1F32D, 0x1F335},
    {0x1F337, 0x1F37C},
    {0x1F37E, 0x1F393},
    {0x1F3A0, 0x1F3CA},
    {0x1F3CF, 0x1F3D3},
    {0x1F3E0, 0x1F3F0},
    {0x1F3F4, 0x1F3F4},
    {0x1F3F8, 0x1F43E},
    {0x1F440, 0x1F440},
    {0x1F442, 0x1F4FC},
    {0x1F4FF, 0x1F53D},
    {0x1F54B, 0x1F54E},
    {0x1F550, 0x1F567},
    {0x1F57A, 0x1F57A},
    {0x1F595, 0x1F596},
    {0x1F5A4, 0x1F5A4},
    {0x1F5FB, 0x1F64F},
    {0x1F680, 0x1F6C5},
    {0x1F6CC, 0x1F6CC},
    {0x1F6D0, 0x1F6D2},
    {0x1F6EB, 0x1F6EC},
    {0x1F6F4, 0x1F6F6},
    {0x1F910, 0x1F91E},
    {0x1F920, 0x1F927},
    {0x1F930, 0x1F930},
    {0x1F933, 0x1F93E},
    {0x1F940, 0x1F94B},
    {0x1F950, 0x1F95E},
    {0x1F980, 0x1F991},
    {0x1F9C0, 0x1F9C0},
    {0x20000, 0x2FFFD},
    {0x30000, 0x3FFFD},
  };

  return ((bisearch(c, wide, sizeof(wide) / sizeof(struct interval) - 1)) ||
          (cjkwidth &&
           bisearch(c, ambiguous,
	            sizeof(ambiguous) / sizeof(struct interval) - 1)));
}
#endif

int
utf8_iscomb(c)
int c;
{
  /* taken from Markus Kuhn's wcwidth */
  static const struct interval combining[] = {
    { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
    { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
    { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
    { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
    { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
    { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
    { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
    { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
    { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
    { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
    { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
    { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
    { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
    { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
    { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
    { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
    { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
    { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
    { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
    { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
    { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
    { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
    { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
    { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
    { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
    { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
    { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
    { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
    { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
    { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
    { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
    { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
    { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
    { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
    { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
    { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
    { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
    { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
    { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
    { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
    { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
    { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
    { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
    { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
    { 0xE0100, 0xE01EF }
  };

  return bisearch(c, combining, sizeof(combining) / sizeof(struct interval) - 1);
}

static void
comb_tofront(root, i)
int root, i;
{
  for (;;)
    {
      debug1("bring to front: %x\n", i);
      combchars[combchars[i]->prev]->next = combchars[i]->next;
      combchars[combchars[i]->next]->prev = combchars[i]->prev;
      combchars[i]->next = combchars[root]->next;
      combchars[i]->prev = root;
      combchars[combchars[root]->next]->prev = i;
      combchars[root]->next = i;
      i = combchars[i]->c1;
      if (i < 0xd800 || i >= 0xe000)
	return;
      i -= 0xd800;
    }
}

void
utf8_handle_comb(c, mc)
int c;
struct mchar *mc;
{
  int root, i, c1;
  int isdouble;

  c1 = mc->image | (mc->font << 8) | mc->fontx << 16;
  isdouble = c1 >= 0x1100 && utf8_isdouble(c1);
  if (!combchars)
    {
      combchars = (struct combchar **)calloc(0x802, sizeof(struct combchar *));
      if (!combchars)
	return;
      combchars[0x800] = (struct combchar *)malloc(sizeof(struct combchar));
      combchars[0x801] = (struct combchar *)malloc(sizeof(struct combchar));
      if (!combchars[0x800] || !combchars[0x801])
	{
	  if (combchars[0x800])
	    free(combchars[0x800]);
	  if (combchars[0x801])
	    free(combchars[0x801]);
	  free(combchars);
	  return;
	}
      combchars[0x800]->c1 = 0x000;
      combchars[0x800]->c2 = 0x700;
      combchars[0x800]->next = 0x800;
      combchars[0x800]->prev = 0x800;
      combchars[0x801]->c1 = 0x700;
      combchars[0x801]->c2 = 0x800;
      combchars[0x801]->next = 0x801;
      combchars[0x801]->prev = 0x801;
    }
  root = isdouble ? 0x801 : 0x800;
  for (i = combchars[root]->c1; i < combchars[root]->c2; i++)
    {
      if (!combchars[i])
	break;
      if (combchars[i]->c1 == c1 && combchars[i]->c2 == c)
	break;
    }
  if (i == combchars[root]->c2)
    {
      /* full, recycle old entry */
      if (c1 >= 0xd800 && c1 < 0xe000)
        comb_tofront(root, c1 - 0xd800);
      i = combchars[root]->prev;
      if (c1 == i + 0xd800)
	{
	  /* completely full, can't recycle */
	  debug("utf8_handle_comp: completely full!\n");
	  mc->image = '?';
	  mc->font  = 0;
	  return;
	}
      /* FIXME: delete old char from all buffers */
    }
  else if (!combchars[i])
    {
      combchars[i] = (struct combchar *)malloc(sizeof(struct combchar));
      if (!combchars[i])
	return;
      combchars[i]->prev = i;
      combchars[i]->next = i;
    }
  combchars[i]->c1 = c1;
  combchars[i]->c2 = c;
  mc->image = i & 0xff;
  mc->font  = (i >> 8) + 0xd8;
  mc->fontx = 0;
  debug3("combinig char %x %x -> %x\n", c1, c, i + 0xd800);
  comb_tofront(root, i);
}

#else /* !UTF8 */

void
WinSwitchEncoding(p, encoding)
struct win *p;
int encoding;
{
  p->w_encoding = encoding;
  return;
}

#endif /* UTF8 */

static int
encmatch(s1, s2)
char *s1;
char *s2;
{
  int c1, c2;
  do
    {
      c1 = (unsigned char)*s1;
      if (c1 >= 'A' && c1 <= 'Z')
	c1 += 'a' - 'A';
      if (!(c1 >= 'a' && c1 <= 'z') && !(c1 >= '0' && c1 <= '9'))
	{
	  s1++;
	  continue;
	}
      c2 = (unsigned char)*s2;
      if (c2 >= 'A' && c2 <= 'Z')
	c2 += 'a' - 'A';
      if (!(c2 >= 'a' && c2 <= 'z') && !(c2 >= '0' && c2 <= '9'))
	{
	  s2++;
	  continue;
	}
      if (c1 != c2)
	return 0;
      s1++;
      s2++;
    }
  while(c1);
  return 1;
}

int
FindEncoding(name)
char *name;
{
  int encoding;

  debug1("FindEncoding %s\n", name);
  if (name == 0 || *name == 0)
    return 0;
  if (encmatch(name, "euc"))
    name = "eucJP";
  if (encmatch(name, "off") || encmatch(name, "iso8859-1"))
    return 0;
#ifndef UTF8
  if (encmatch(name, "UTF-8"))
    return -1;
#endif
  for (encoding = 0; encoding < (int)(sizeof(encodings)/sizeof(*encodings)); encoding++)
    if (encmatch(name, encodings[encoding].name))
      {
#ifdef UTF8
	LoadFontTranslationsForEncoding(encoding);
#endif
        return encoding;
      }
  return -1;
}

char *
EncodingName(encoding)
int encoding;
{
  if (encoding >= (int)(sizeof(encodings)/sizeof(*encodings)))
    return 0;
  return encodings[encoding].name;
}

int
EncodingDefFont(encoding)
int encoding;
{
  return encodings[encoding].deffont;
}

void
ResetEncoding(p)
struct win *p;
{
  char *c;
  int encoding = p->w_encoding;

  c = encodings[encoding].charsets;
  if (c)
    SetCharsets(p, c);
#ifdef UTF8
  LoadFontTranslationsForEncoding(encoding);
#endif
  if (encodings[encoding].usegr)
    {
      p->w_gr = 2;
      p->w_FontE = encodings[encoding].charsets[1];
    }
  else
    p->w_FontE = 0;
  if (encodings[encoding].noc1)
    p->w_c1 = 0;
}

/* decoded char: 32-bit <fontx><font><c2><c>
 * fontx: non-bmp utf8
 * c2: multi-byte character
 * font is always zero for utf8
 * returns: -1 need more bytes
 *          -2 decode error
 */


int
DecodeChar(c, encoding, statep)
int c;
int encoding;
int *statep;
{
  int t;

  debug2("Decoding char %02x for encoding %d\n", c, encoding);
#ifdef UTF8
  if (encoding == UTF8)
    {
      c = FromUtf8(c, statep);
      if (c >= 0x10000)
	c = (c & 0x7f0000) << 8 | (c & 0xffff);
      return c;
    }
#endif
  if (encoding == SJIS)
    {
      if (!*statep)
	{
	  if ((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xef))
	    {
	      *statep = c;
	      return -1;
	    }
	  if (c < 0x80)
	    return c;
	  return c | (KANA << 16);
	}
      t = c;
      c = *statep;
      *statep = 0;
      if (0x40 <= t && t <= 0xfc && t != 0x7f)
	{
	  if (c <= 0x9f)
	    c = (c - 0x81) * 2 + 0x21; 
	  else
	    c = (c - 0xc1) * 2 + 0x21; 
	  if (t <= 0x7e)
	    t -= 0x1f;
	  else if (t <= 0x9e)
	    t -= 0x20;
	  else
	     t -= 0x7e, c++;
	  return (c << 8) | t | (KANJI << 16);
	}
      return t;
    }
  if (encoding == EUC_JP || encoding == EUC_KR || encoding == EUC_CN)
    {
      if (!*statep)
	{
	  if (c & 0x80)
	    {
	      *statep = c;
	      return -1;
	    }
	  return c;
	}
      t = c;
      c = *statep;
      *statep = 0;
      if (encoding == EUC_JP)
	{
	  if (c == 0x8e)
	    return t | (KANA << 16);
	  if (c == 0x8f)
	    {
	      *statep = t | (KANJI0212 << 8);
	      return -1;
	    }
	}
      c &= 0xff7f;
      t &= 0x7f;
      c = c << 8 | t;
      if (encoding == EUC_KR)
	return c | (3 << 16);
      if (encoding == EUC_CN)
	return c | (1 << 16);
      if (c & (KANJI0212 << 16))
        return c;
      else
        return c | (KANJI << 16);
    }
  if (encoding == BIG5 || encoding == GBK)
    {
      if (!*statep)
	{
	  if (c & 0x80)
	    {
	      if (encoding == GBK && c == 0x80)
		return 0xa4 | (('b'|0x80) << 16);
	      *statep = c;
	      return -1;
	    }
	  return c;
	}
      t = c;
      c = *statep;
      *statep = 0;
      c &= 0x7f;
      return c << 8 | t | (encoding == BIG5  ? 030 << 16 : 031 << 16);
    }
  return c | (encodings[encoding].deffont << 16);
}

int
EncodeChar(bp, c, encoding, fontp)
char *bp;
int c;
int encoding;
int *fontp;
{
  int t, f, l;

  debug2("Encoding char %02x for encoding %d\n", c, encoding);
  if (c == -1 && fontp)
    {
      if (*fontp == 0)
	return 0;
      if (bp)
	{
	  *bp++ = 033;
	  *bp++ = '(';
	  *bp++ = 'B';
	}
      return 3;
    }
  f = (c >> 16) & 0xff;

#ifdef UTF8
  if (encoding == UTF8)
    {
      if (f)
	{
# ifdef DW_CHARS
	  if (is_dw_font(f))
	    {
	      int c2 = c & 0xff;
	      c = (c >> 8 & 0xff) | (f << 8);
	      c = recode_char_dw_to_encoding(c, &c2, encoding);
	    }
	  else
# endif
	    {
	      c = (c & 0xff) | (f << 8);
	      c = recode_char_to_encoding(c, encoding);
	    }
        }
      return ToUtf8(bp, c);
    }
  if (f == 0 && (c & 0x7f00ff00) != 0)	/* is_utf8? */
    {
      if (c >= 0x10000)
	c = (c & 0x7f0000) >> 8 | (c & 0xffff);
# ifdef DW_CHARS
      if (utf8_isdouble(c))
	{
	  int c2 = 0xffff;
	  c = recode_char_dw_to_encoding(c, &c2, encoding);
	  c = (c << 8) | (c2 & 0xff);
	}
      else
# endif
	{
	  c = recode_char_to_encoding(c, encoding);
	  c = ((c & 0xff00) << 8) | (c & 0xff);
	}
      debug1("Encode: char mapped from utf8 to %x\n", c);
      f = c >> 16;
    }
#endif
  if (f & 0x80)		/* map special 96-fonts to latin1 */
    f = 0;

  if (encoding == SJIS)
    {
      if (f == KANA)
        c = (c & 0xff) | 0x80;
      else if (f == KANJI)
	{
	  if (!bp)
	    return 2;
	  t = c & 0xff;
	  c = (c >> 8) & 0xff;
	  t += (c & 1) ? ((t <= 0x5f) ? 0x1f : 0x20) : 0x7e; 
	  c = (c - 0x21) / 2 + ((c < 0x5f) ? 0x81 : 0xc1);
	  *bp++ = c;
	  *bp++ = t;
	  return 2;
	}
    }
  if (encoding == EUC)
    {
      if (f == KANA)
	{
	  if (bp)
	    {
	      *bp++ = 0x8e;
	      *bp++ = c;
	    }
	  return 2;
	}
      if (f == KANJI)
	{
	  if (bp)
	    {
	      *bp++ = (c >> 8) | 0x80;
	      *bp++ = c | 0x80;
	    }
	  return 2;
	}
      if (f == KANJI0212)
	{
	  if (bp)
	    {
	      *bp++ = 0x8f;
	      *bp++ = c >> 8;
	      *bp++ = c;
	    }
	  return 3;
	}
    }
  if ((encoding == EUC_KR && f == 3) || (encoding == EUC_CN && f == 1))
    {
      if (bp)
	{
	  *bp++ = (c >> 8) | 0x80;
	  *bp++ = c | 0x80;
	}
      return 2;
    }
  if ((encoding == BIG5 && f == 030) || (encoding == GBK && f == 031))
    {
      if (bp)
	{
	  *bp++ = (c >> 8) | 0x80;
	  *bp++ = c;
	}
      return 2;
    }
  if (encoding == GBK && f == 0 && c == 0xa4)
    c = 0x80;

  l = 0;
  if (fontp && f != *fontp)
    {
      *fontp = f;
      if (f && f < ' ')
	{
	  if (bp)
	   {
	     *bp++ = 033;
	     *bp++ = '$';
	     if (f > 2)
	       *bp++ = '(';
	     *bp++ = '@' + f;
	   }
	  l += f > 2 ? 4 : 3;
	}
      else if (f < 128)
	{
	  if (f == 0)
	    f = 'B';
	  if (bp)
	    {
	      *bp++ = 033;
	      *bp++ = '(';
	      *bp++ = f;
	    }
	  l += 3;
	}
    }
  if (c & 0xff00)
    {
      if (bp)
	*bp++ = c >> 8;
      l++;
    }
  if (bp)
    *bp++ = c;
  return l + 1;
}

int
CanEncodeFont(encoding, f)
int encoding, f;
{
  switch(encoding)
    {
#ifdef UTF8
    case UTF8:
      return 1;
#endif
    case SJIS:
      return f == KANJI || f == KANA;
    case EUC:
      return f == KANJI || f == KANA || f == KANJI0212;
    case EUC_KR:
      return f == 3;
    case EUC_CN:
      return f == 1;
    case BIG5:
      return f == 030;
    case GBK:
      return f == 031;
    default:
      break;
    }
  return 0;
}

#ifdef DW_CHARS
int
PrepareEncodedChar(c)
int c;
{
  int encoding;
  int t = 0;
  int f;

  encoding = D_encoding;
  f = D_rend.font;
  t = D_mbcs;
  if (encoding == SJIS)
    {
      if (f == KANA)
        return c | 0x80;
      else if (f == KANJI)
	{
	  t += (c & 1) ? ((t <= 0x5f) ? 0x1f : 0x20) : 0x7e; 
	  c = (c - 0x21) / 2 + ((c < 0x5f) ? 0x81 : 0xc1);
	  D_mbcs = t;
	}
      return c;
    }
  if (encoding == EUC)
    {
      if (f == KANA)
	{
	  AddChar(0x8e);
	  return c | 0x80;
	}
      if (f == KANJI)
	{
	  D_mbcs = t | 0x80;
	  return c | 0x80;
	}
      if (f == KANJI0212)
	{
	  AddChar(0x8f);
	  D_mbcs = t | 0x80;
	  return c | 0x80;
	}
    }
  if ((encoding == EUC_KR && f == 3) || (encoding == EUC_CN && f == 1))
    {
      D_mbcs = t | 0x80;
      return c | 0x80;
    }
  if ((encoding == BIG5 && f == 030) || (encoding == GBK && f == 031))
    return c | 0x80;
  return c;
}
#endif

int
RecodeBuf(fbuf, flen, fenc, tenc, tbuf)
unsigned char *fbuf;
int flen;
int fenc, tenc;
unsigned char *tbuf;
{
  int c, i, j;
  int decstate = 0, font = 0;

  for (i = j = 0; i < flen; i++)
    {
      c = fbuf[i];
      c = DecodeChar(c, fenc, &decstate);
      if (c == -2)
	i--;
      if (c < 0)
	continue;
      j += EncodeChar(tbuf ? (char *)tbuf + j : 0, c, tenc, &font);
    }
  j += EncodeChar(tbuf ? (char *)tbuf + j : 0, -1, tenc, &font);
  return j;
}

#ifdef UTF8
int
ContainsSpecialDeffont(ml, xs, xe, encoding)
struct mline *ml;
int xs, xe;
int encoding;
{
  unsigned char *f, *i;
  int c, x, dx;

  if (encoding == UTF8 || encodings[encoding].deffont == 0)
    return 0;
  i = ml->image + xs;
  f = ml->font + xs;
  dx = xe - xs + 1;
  while (dx-- > 0)
    {
      if (*f++)
	continue;
      c = *i++;
      x = recode_char_to_encoding(c | (encodings[encoding].deffont << 8), UTF8);
      if (c != x)
	{
	  debug2("ContainsSpecialDeffont: yes %02x != %02x\n", c, x);
	  return 1;
	}
    }
  debug("ContainsSpecialDeffont: no\n");
  return 0;
}


int
LoadFontTranslation(font, file)
int font;
char *file;
{
  char buf[1024], *myfile;
  FILE *f;
  int i;
  int fo;
  int x, u, c, ok;
  unsigned short (*p)[2], (*tab)[2];

  myfile = file;
  if (myfile == 0)
    {
      if (font == 0 || screenencodings == 0)
	return -1;
      if (strlen(screenencodings) > sizeof(buf) - 10)
	return -1;
      sprintf(buf, "%s/%02x", screenencodings, font & 0xff);
      myfile = buf;
    }
  debug1("LoadFontTranslation: trying %s\n", myfile);
  if ((f = secfopen(myfile, "r")) == 0)
    return -1;
  i = ok = 0;
  for (;;)
    {
      for(; i < 12; i++)
	if (getc(f) != "ScreenI2UTF8"[i])
	  break;
      if (getc(f) != 0)		/* format */
	break;
      fo = getc(f);		/* id */
      if (fo == EOF)
	break;
      if (font != -1 && font != fo)
	break;
      i = getc(f);
      x = getc(f);
      if (x == EOF)
	break;
      i = i << 8 | x;
      getc(f);
      while ((x = getc(f)) && x != EOF)
	getc(f); 	/* skip font name (padded to 2 bytes) */
      if ((p = malloc(sizeof(*p) * (i + 1))) == 0)
	break;
      tab = p;
      while(i > 0)
	{
	  x = getc(f);
	  x = x << 8 | getc(f);
	  u = getc(f);
	  c = getc(f);
	  u = u << 8 | c;
	  if (c == EOF)
	    break;
	  (*p)[0] = x;
	  (*p)[1] = u;
	  p++;
	  i--;
	}
      (*p)[0] = 0;
      (*p)[1] = 0;
      if (i || (tab[0][0] & 0x8000))
	{
	  free(tab);
	  break;
	}
      if (recodetabs[fo].tab && (recodetabs[fo].flags & RECODETAB_ALLOCED) != 0)
	free(recodetabs[fo].tab);
      recodetabs[fo].tab = tab;
      recodetabs[fo].flags = RECODETAB_ALLOCED;
      debug1("Successful load of recodetab %02x\n", fo);
      c = getc(f);
      if (c == EOF)
	{
	  ok = 1;
	  break;
	}
      if (c != 'S')
	break;
      i = 1;
    }
  fclose(f);
  if (font != -1 && file == 0 && recodetabs[font].flags == 0)
    recodetabs[font].flags = RECODETAB_TRIED;
  return ok ? 0 : -1;
}

void
LoadFontTranslationsForEncoding(encoding)
int encoding;
{
  char *c;
  int f;

  debug1("LoadFontTranslationsForEncoding: encoding %d\n", encoding);
  if ((c = encodings[encoding].fontlist) != 0)
    while ((f = (unsigned char)*c++) != 0)
      if (recodetabs[f].flags == 0)
	  LoadFontTranslation(f, 0);
  f = encodings[encoding].deffont;
  if (f > 0 && recodetabs[f].flags == 0)
    LoadFontTranslation(f, 0);
}

#endif /* UTF8 */

#else /* !ENCODINGS */

/* Simple version of EncodeChar to encode font changes for
 * copy/paste mode
 */
int
EncodeChar(bp, c, encoding, fontp)
char *bp;
int c;
int encoding;
int *fontp;
{
  int f, l;
  f = (c == -1) ? 0 : c >> 16;
  l = 0;
  if (fontp && f != *fontp)
    {
      *fontp = f;
      if (f && f < ' ')
	{
	  if (bp)
	   {
	     *bp++ = 033;
	     *bp++ = '$';
	     if (f > 2)
	       *bp++ = '(';
	     *bp++ = '@' + f;
	   }
	  l += f > 2 ? 4 : 3;
	}
      else if (f < 128)
	{
	  if (f == 0)
	    f = 'B';
	  if (bp)
	    {
	      *bp++ = 033;
	      *bp++ = '(';
	      *bp++ = f;
	    }
	  l += 3;
	}
    }
  if (c == -1)
    return l;
  if (c & 0xff00)
    {
      if (bp)
	*bp++ = c >> 8;
      l++;
    }
  if (bp)
    *bp++ = c;
  return l + 1;
}

#endif /* ENCODINGS */
